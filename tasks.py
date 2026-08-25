"""Invoke tasks for configuring, building, and testing cw-api3d.

Run `uv run invoke --list` to see available tasks. CMake / CTest tasks load
MSVC toolset 14.44 automatically so a Developer shell is not required.
"""

from __future__ import annotations

from functools import cache
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile

from invoke import Context, task

PROJECT_ROOT = Path(__file__).resolve().parent
DEFAULT_PRESET = "local-relwithdebinfo"
MSVC_TOOLSET = "14.44"
HOST_ONLY_K = "not test_plugin_initialization_and_execution"

_PRESET_NAME = re.compile(r"^[A-Za-z0-9][A-Za-z0-9_.-]*$")
_ENV_KEY = re.compile(r"^[A-Za-z_][A-Za-z0-9_()]*$")

_VSWHERE = (
    Path(os.environ.get("ProgramFiles(x86)", r"C:\Program Files (x86)"))
    / "Microsoft Visual Studio"
    / "Installer"
    / "vswhere.exe"
)
_FALLBACK_VCVARS = Path(
    r"C:\Program Files (x86)\Microsoft Visual Studio\2022"
    r"\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
)


def _require_windows() -> None:
    if sys.platform != "win32":
        raise RuntimeError("cw-api3d invoke tasks require Windows (MSVC + cadwork).")


def _validated_preset(preset: str) -> str:
    if not _PRESET_NAME.fullmatch(preset):
        raise ValueError(f"invalid CMake preset name: {preset!r}")
    return preset


def _build_dir(preset: str) -> Path:
    return PROJECT_ROOT / "out" / "build" / _validated_preset(preset)


def _vcvars_candidates() -> list[Path]:
    found: list[Path] = []
    if _FALLBACK_VCVARS.is_file():
        found.append(_FALLBACK_VCVARS)
    if _VSWHERE.is_file():
        result = subprocess.run(
            [
                str(_VSWHERE),
                "-products",
                "*",
                "-find",
                r"VC\Auxiliary\Build\vcvars64.bat",
            ],
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            check=False,
        )
        for line in result.stdout.splitlines():
            candidate = Path(line.strip())
            if candidate.is_file() and candidate not in found:
                found.append(candidate)
    return found


def _parse_cmd_env(dump: str) -> dict[str, str]:
    env: dict[str, str] = {}
    for line in dump.splitlines():
        if "=" not in line:
            continue
        key, _, value = line.partition("=")
        if _ENV_KEY.fullmatch(key):
            env[key] = value
    return env


def _env_from_vcvars(vcvars: Path) -> dict[str, str]:
    # A helper .bat avoids Windows list2cmdline extra-escaping the spaced vcvars
    # path. cmd /d skips Command Processor AutoRun (conda/clink hooks).
    script = (
        "@echo off\r\n"
        f'call "{vcvars}" -vcvars_ver={MSVC_TOOLSET}\r\n'
        "if errorlevel 1 exit /b 1\r\n"
        "set\r\n"
    )
    fd, bat_path = tempfile.mkstemp(suffix=".bat")
    try:
        with os.fdopen(fd, "w", encoding="utf-8") as handle:
            handle.write(script)
        try:
            result = subprocess.run(
                ["cmd.exe", "/d", "/c", bat_path],
                capture_output=True,
                text=True,
                encoding="oem",
                errors="replace",
                check=False,
                cwd=str(PROJECT_ROOT),
                timeout=30,
            )
        except subprocess.TimeoutExpired as exc:
            raise RuntimeError(
                f"Timed out loading MSVC toolset {MSVC_TOOLSET} from {vcvars}"
            ) from exc
    finally:
        Path(bat_path).unlink(missing_ok=True)
    if result.returncode != 0:
        details = (result.stderr or result.stdout).strip()
        raise RuntimeError(
            f"Failed to load MSVC toolset {MSVC_TOOLSET} from {vcvars}. "
            f"{details or f'exit {result.returncode}'}"
        )
    return _parse_cmd_env(result.stdout)


@cache
def _msvc_env() -> dict[str, str]:
    """Return the environment produced by vcvars64.bat -vcvars_ver=14.44."""
    _require_windows()
    existing = os.environ.get("VCToolsVersion", "")
    if existing.startswith(MSVC_TOOLSET):
        return dict(os.environ)
    candidates = _vcvars_candidates()
    if not candidates:
        raise RuntimeError(
            "vcvars64.bat not found. Install Visual Studio 2022 Build Tools "
            f"with MSVC toolset {MSVC_TOOLSET}+, or add vswhere.exe to the default path."
        )

    errors: list[str] = []
    for vcvars in candidates:
        try:
            env = _env_from_vcvars(vcvars)
        except RuntimeError as exc:
            errors.append(str(exc))
            continue
        tools_version = env.get("VCToolsVersion", "")
        if tools_version.startswith(MSVC_TOOLSET):
            print(f"MSVC {tools_version} ({vcvars})")
            return env
        errors.append(
            f"{vcvars}: VCToolsVersion={tools_version!r} (need {MSVC_TOOLSET}+)"
        )

    details = " ".join(errors)
    raise RuntimeError(
        f"MSVC toolset {MSVC_TOOLSET} was not found. The installer default 14.42 "
        f"does not support C++23. Install toolset {MSVC_TOOLSET}+ and retry. {details}"
    )


def _run(c: Context, command: str, env: dict[str, str] | None = None) -> None:
    with c.cd(str(PROJECT_ROOT)):
        c.run(command, env=env, echo=True)


def _configure(c: Context, preset: str) -> None:
    _run(c, f"cmake --preset {_validated_preset(preset)}", env=_msvc_env())


def _ninja_graph_complete(preset: str) -> bool:
    build_dir = _build_dir(preset)
    return (
        (build_dir / "CMakeCache.txt").is_file()
        and (build_dir / "build.ninja").is_file()
        and (build_dir / "CMakeFiles" / "rules.ninja").is_file()
    )


def _ensure_configured(c: Context, preset: str) -> None:
    # A CMakeCache.txt can survive a failed CLion/CMake reconfigure that
    # deleted CMakeFiles/rules.ninja. Ninja then dies on `include rules.ninja`.
    if not _ninja_graph_complete(preset):
        _configure(c, preset)


def _build(c: Context, preset: str) -> None:
    _ensure_configured(c, preset)
    _run(c, f'cmake --build "{_build_dir(preset)}"', env=_msvc_env())


@task
def configure(c: Context, preset: str = DEFAULT_PRESET) -> None:
    """Configure the CMake preset under MSVC 14.44.

    Args:
        preset: CMake configure preset (default: local-relwithdebinfo).
    """
    _configure(c, preset)


@task
def build(c: Context, preset: str = DEFAULT_PRESET) -> None:
    """Configure if needed, then build the preset directory.

    Args:
        preset: CMake configure preset (default: local-relwithdebinfo).
    """
    _build(c, preset)


@task
def test(
    c: Context,
    preset: str = DEFAULT_PRESET,
    filter: str = "",
    build: bool = True,
) -> None:
    """Run C++ GoogleTest suites via CTest.

    Args:
        preset: CMake configure preset (default: local-relwithdebinfo).
        filter: Optional CTest -R regex (e.g. CompositionRoot).
        build: Build first (default true; pass --no-build to skip).
    """
    if build:
        _build(c, preset)
    jobs = os.cpu_count() or 1
    command = (
        f'ctest --test-dir "{_build_dir(preset)}" '
        f"--output-on-failure --parallel {jobs}"
    )
    if filter:
        command += f' -R "{filter}"'
    _run(c, command, env=_msvc_env())


@task(auto_shortflags=False)
def e2e(
    c: Context,
    args: str = "",
    host_only: bool = False,
    build: bool = True,
    preset: str = DEFAULT_PRESET,
) -> None:
    """Run the Python E2E harness (pytest tests/e2e).

    Args:
        args: Extra arguments forwarded to pytest verbatim.
        host_only: Skip the live cadwork launch test.
        build: Build first so cw_api3d.dll is deployed (default true).
        preset: CMake configure preset used when building.
    """
    if build:
        _build(c, preset)
    if shutil.which("uv") is None:
        raise RuntimeError("uv was not found on PATH. Install it (winget install astral-sh.uv) and retry.")
    command = "uv run pytest -v"
    if host_only:
        command += f' -k "{HOST_ONLY_K}"'
    if args:
        command += f" {args}"
    _run(c, command)


@task
def clean(c: Context, preset: str = DEFAULT_PRESET) -> None:
    """Remove out/build/<preset> only.

    Args:
        preset: CMake configure preset whose build directory is deleted.
    """
    target = _build_dir(preset)
    if not target.exists():
        print(f"Nothing to clean: {target} does not exist")
        return
    print(f"Removing {target}")
    shutil.rmtree(target)


@task
def rebuild(c: Context, preset: str = DEFAULT_PRESET) -> None:
    """Delete the preset build directory, then configure and build.

    Args:
        preset: CMake configure preset (default: local-relwithdebinfo).
    """
    clean(c, preset=preset)
    _configure(c, preset)
    _build(c, preset)


@task
def dev(c: Context, preset: str = DEFAULT_PRESET) -> None:
    """Build and run C++ tests. Does not launch cadwork (use invoke e2e).

    Args:
        preset: CMake configure preset (default: local-relwithdebinfo).
    """
    _build(c, preset)
    test(c, preset=preset, build=False)
