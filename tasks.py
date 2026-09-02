"""Invoke tasks for configuring, building, and testing cw-api3d.

Run `uv run invoke --list` to see available tasks. CMake / CTest tasks load
the newest installed MSVC toolset automatically so a Developer shell is not
required, and they resolve the CMake install themselves so a shadowed
cmake.exe on PATH (Strawberry Perl, an IDE-bundled copy) can never
configure the build.
"""

from __future__ import annotations

from collections.abc import Callable
from functools import cache
import os
from pathlib import Path
import re
import shutil
import stat
import subprocess
import sys
import tempfile

from invoke import Context, task

PROJECT_ROOT = Path(__file__).resolve().parent
DEFAULT_PRESET = "local-relwithdebinfo"
MSVC_MIN_TOOLSET = (14, 44)  # a floor, not a pin: C++23 <generator> needs 14.44
MIN_CMAKE = (3, 28)
HOST_ONLY_K = "not test_plugin_initialization_and_execution"

_PRESET_NAME = re.compile(r"^[A-Za-z0-9][A-Za-z0-9_.-]*$")
_ENV_KEY = re.compile(r"^[A-Za-z_][A-Za-z0-9_()]*$")
_CMAKE_VERSION = re.compile(r"cmake version (\d+(?:\.\d+)*)")
_CACHED_CMAKE_COMMAND = re.compile(r"^CMAKE_COMMAND:INTERNAL=(.+)$", re.MULTILINE)

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
_OFFICIAL_CMAKE_BIN = (
    Path(os.environ.get("ProgramFiles", r"C:\Program Files")) / "CMake" / "bin"
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


def _clear_readonly_and_retry(
    func: Callable[[str], None], path: str, _exc: BaseException
) -> None:
    """rmtree handler for FetchContent trees.

    Git marks pack files read-only, which makes os.unlink fail with WinError 5;
    dropping the flag and retrying is the only way to remove a _deps checkout.
    """
    os.chmod(path, stat.S_IWRITE)
    func(path)


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


def _with_path_prefix(env: dict[str, str], directory: Path) -> dict[str, str]:
    """Put `directory` first on PATH, normalising the key to the os.environ spelling.

    cmd's `set` emits `Path`, os.environ uses `PATH`; leaving both in the dict
    would hand the child two case-variant PATH entries and let the stale one win.
    """
    normalized = {key: value for key, value in env.items() if key.upper() != "PATH"}
    inherited = next((value for key, value in env.items() if key.upper() == "PATH"), "")
    normalized["PATH"] = f"{directory}{os.pathsep}{inherited}"
    return normalized


def _env_from_vcvars(vcvars: Path) -> dict[str, str]:
    # A helper .bat avoids Windows list2cmdline extra-escaping the spaced vcvars
    # path. cmd /d skips Command Processor AutoRun (conda/clink hooks).
    script = (
        "@echo off\r\n"
        f'call "{vcvars}"\r\n'
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
                f"Timed out loading the MSVC environment from {vcvars}"
            ) from exc
    finally:
        Path(bat_path).unlink(missing_ok=True)
    if result.returncode != 0:
        details = (result.stderr or result.stdout).strip()
        raise RuntimeError(
            f"Failed to load the MSVC environment from {vcvars}. "
            f"{details or f'exit {result.returncode}'}"
        )
    return _parse_cmd_env(result.stdout)


def _toolset_version(reported: str) -> tuple[int, ...] | None:
    """Parse a VCToolsVersion such as "14.51.36231" into a comparable tuple."""
    parts = reported.split(".")
    if not reported or not all(part.isdigit() for part in parts):
        return None
    return tuple(int(part) for part in parts)


def _vcvars_env() -> dict[str, str]:
    """Return the environment of the newest installed MSVC toolset.

    Each vcvars64.bat is called without -vcvars_ver, so every install offers its
    own default (newest) toolset and the highest version across installs wins.
    Pinning an exact version instead would strand the build on a toolset the
    machine has already moved past.
    """
    _require_windows()
    floor = ".".join(str(part) for part in MSVC_MIN_TOOLSET)
    # An already-loaded Developer shell wins outright: it is the toolset the
    # caller deliberately entered, and reloading could silently switch it.
    active = _toolset_version(os.environ.get("VCToolsVersion", ""))
    if active is not None and active >= MSVC_MIN_TOOLSET:
        return dict(os.environ)
    candidates = _vcvars_candidates()
    if not candidates:
        raise RuntimeError(
            "vcvars64.bat not found. Install Visual Studio Build Tools with MSVC "
            f"toolset {floor} or newer, or add vswhere.exe to the default path."
        )

    errors: list[str] = []
    newest: tuple[tuple[int, ...], Path, dict[str, str]] | None = None
    for vcvars in candidates:
        try:
            env = _env_from_vcvars(vcvars)
        except RuntimeError as exc:
            errors.append(str(exc))
            continue
        reported = env.get("VCToolsVersion", "")
        version = _toolset_version(reported)
        if version is None or version < MSVC_MIN_TOOLSET:
            errors.append(f"{vcvars}: VCToolsVersion={reported!r} (need {floor}+)")
            continue
        if newest is None or version > newest[0]:
            newest = (version, vcvars, env)

    if newest is None:
        raise RuntimeError(
            f"No MSVC toolset {floor} or newer was found; C++23 support starts "
            f"there. Install a newer toolset and retry. {' '.join(errors)}"
        )
    _, vcvars, env = newest
    print(f"MSVC {env['VCToolsVersion']} ({vcvars})")
    return env


def _cmake_version(exe: Path) -> str | None:
    result = subprocess.run(
        [str(exe), "--version"],
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
        check=False,
        timeout=30,
    )
    if result.returncode != 0:
        return None
    match = _CMAKE_VERSION.search(result.stdout)
    return match.group(1) if match else None


def _version_key(version: str) -> tuple[int, ...]:
    return tuple(int(part) for part in version.split("."))


def _cmake_candidates() -> list[Path]:
    """Bin directories to try, official install ahead of whatever PATH offers."""
    candidates: list[Path] = []
    override = os.environ.get("CW_CMAKE_BIN")
    if override:
        candidates.append(Path(override))
    candidates.append(_OFFICIAL_CMAKE_BIN)
    discovered = shutil.which("cmake")
    if discovered:
        candidates.append(Path(discovered).parent)

    unique: list[Path] = []
    for candidate in candidates:
        if candidate not in unique:
            unique.append(candidate)
    return unique


@cache
def _cmake_bin_dir() -> Path:
    """Return the bin directory of the CMake this project builds with.

    Candidate order is what excludes a shadowing install: Strawberry Perl ships
    cmake 3.29, which clears MIN_CMAKE, so the version gate alone would accept it.
    """
    _require_windows()
    required = ".".join(str(part) for part in MIN_CMAKE)
    rejected: list[str] = []
    for candidate in _cmake_candidates():
        exe = candidate / "cmake.exe"
        if not exe.is_file():
            rejected.append(f"{candidate}: no cmake.exe")
            continue
        version = _cmake_version(exe)
        if version is None:
            rejected.append(f"{exe}: could not read --version")
            continue
        if _version_key(version) < MIN_CMAKE:
            rejected.append(f"{exe}: {version} (need {required}+)")
            continue
        print(f"CMake {version} ({exe})")
        return candidate

    details = " ".join(rejected)
    raise RuntimeError(
        f"No CMake {required}+ was found. Install it to {_OFFICIAL_CMAKE_BIN.parent}, "
        f"or point CW_CMAKE_BIN at the bin directory of another install. {details}"
    )


def _cmake_exe() -> Path:
    return _cmake_bin_dir() / "cmake.exe"


def _ctest_exe() -> Path:
    return _cmake_bin_dir() / "ctest.exe"


@cache
def _msvc_env() -> dict[str, str]:
    """Newest MSVC environment with the resolved CMake install first on PATH.

    Nested lookups (ExternalProject sub-builds, CTest re-invoking cmake) resolve
    by name, so pinning the parent process alone would not be enough.
    """
    return _with_path_prefix(_vcvars_env(), _cmake_bin_dir())


def _run(c: Context, command: str, env: dict[str, str] | None = None) -> None:
    with c.cd(str(PROJECT_ROOT)):
        c.run(command, env=env, echo=True)


def _configure(c: Context, preset: str) -> None:
    _run(
        c,
        f'"{_cmake_exe()}" --preset {_validated_preset(preset)}',
        env=_msvc_env(),
    )


def _cached_cmake_command(build_dir: Path) -> Path | None:
    """The cmake.exe recorded in an existing build tree, if there is one."""
    cache_file = build_dir / "CMakeCache.txt"
    if not cache_file.is_file():
        return None
    match = _CACHED_CMAKE_COMMAND.search(
        cache_file.read_text(encoding="utf-8", errors="replace")
    )
    return Path(match.group(1).strip()) if match else None


def _same_executable(left: Path, right: Path) -> bool:
    return os.path.normcase(str(left.resolve())) == os.path.normcase(str(right.resolve()))


def _ensure_configured(c: Context, preset: str) -> None:
    configured = _cached_cmake_command(_build_dir(preset))
    if configured is None:
        _configure(c, preset)
        return
    expected = _cmake_exe()
    if not _same_executable(configured, expected):
        raise RuntimeError(
            f"{_build_dir(preset)} was configured by {configured}, not {expected}. "
            f"A build tree is bound to the CMake that generated it, so reconfiguring "
            f"cannot rebind it. Run: uv run invoke rebuild --preset {preset}"
        )


def _build(c: Context, preset: str) -> None:
    _ensure_configured(c, preset)
    _run(c, f'"{_cmake_exe()}" --build "{_build_dir(preset)}"', env=_msvc_env())


@task
def configure(c: Context, preset: str = DEFAULT_PRESET) -> None:
    """Configure the CMake preset under the newest installed MSVC toolset.

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
        f'"{_ctest_exe()}" --test-dir "{_build_dir(preset)}" '
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
    shutil.rmtree(target, onexc=_clear_readonly_and_retry)


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
