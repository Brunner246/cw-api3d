"""Cadwork install and profile path resolution for cw-api3d E2E test harness.

Provides dynamic discovery of cadwork directories, executables, user profiles,
and plugin binaries from the Windows registry with environment variable overrides.
"""

from __future__ import annotations

import os
from pathlib import Path
import sys

# Default fallback values when registry is unavailable
DEFAULT_PROFILE_YEAR = "2026"
DEFAULT_CI_START = r"D:\cadwork.dir\ci_start.exe"
CADWORK_EXE_RELPATH = Path("3d.x64") / "3d.exe"

# Checkout-local userprofile marker written by build-scripts/new-local-profile.ps1, so a git
# worktree deploys and tests against its own profile.
# Keep in sync with cmake/cadwork_deploy.cmake and build-scripts/new-local-profile.ps1.
USERPROFILE_MARKER_NAME = ".cw-userprofile"
REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_PLUGIN_NAME = "cw_api3d_hello"


def _registry_value(value_name: str) -> Path | None:
    """Read a string value from HKCU\\Software\\cadwork Informatik\\ENV.

    Args:
        value_name: The registry value name to look up.

    Returns:
        Path object if found and non-empty, otherwise None.
    """
    if sys.platform != "win32":
        return None

    try:
        import winreg

        with winreg.OpenKey(winreg.HKEY_CURRENT_USER, r"Software\cadwork Informatik\ENV") as key:
            value, _ = winreg.QueryValueEx(key, value_name)
    except (OSError, ImportError, ValueError):
        return None

    if not isinstance(value, str):
        return None

    cleaned = value.strip().strip('"')
    return Path(cleaned) if cleaned else None


def _marker_userprofil() -> Path | None:
    """Read the checkout-local userprofile marker at the repository root.

    Returns:
        Path from the first non-blank, non-comment line, otherwise None.
    """
    marker = REPO_ROOT / USERPROFILE_MARKER_NAME
    try:
        lines = marker.read_text(encoding="utf-8").splitlines()
    except OSError:
        return None

    for line in lines:
        cleaned = line.strip().strip('"')
        if cleaned and not cleaned.startswith("#"):
            return Path(cleaned)
    return None


def install_dir() -> Path | None:
    """Resolve the cadwork install root directory (CADWORK.DIR).

    Returns:
        Path to install directory if found, otherwise None.
    """
    override = os.environ.get("CADWORK_DIR")
    if override:
        return Path(override)
    return _registry_value("CADWORK.DIR")


def userprofil_dir() -> Path | None:
    """Resolve the cadwork userprofile directory.

    Precedence: CADWORK_USP env override, checkout-local marker, registry
    (CADWORK_USP / CISTART_USP).

    Returns:
        Path to userprofile directory if found, otherwise None.
    """
    override = os.environ.get("CADWORK_USP")
    if override:
        return Path(override)

    marker = _marker_userprofil()
    if marker is not None:
        return marker

    usp = _registry_value("CADWORK_USP")
    if usp is not None:
        return usp
    return _registry_value("CISTART_USP")


def profile_year() -> str:
    """Resolve the cadwork profile year (e.g. '2026').

    Returns:
        String representing the cadwork product year.
    """
    override = os.environ.get("CW_PROFILE_YEAR")
    if override:
        return override.strip()

    usp = userprofil_dir()
    if usp is not None:
        # Check if folder name contains a year (e.g. USERPROFIL_2026)
        name = usp.name.upper()
        if "USERPROFIL_" in name:
            year = name.replace("USERPROFIL_", "").strip()
            if year.isdigit():
                return year

    exe = _registry_value("CADWORK_EXE")
    if exe is not None:
        name = exe.name.upper()
        if "EXE_" in name:
            year = name.replace("EXE_", "").strip()
            if year.isdigit():
                return year

    return DEFAULT_PROFILE_YEAR


def exe_dir() -> Path | None:
    """Resolve the cadwork executable directory (e.g. D:\\cadwork.dir\\EXE_2026).

    Returns:
        Path to the exe directory if 3d.exe is found, otherwise None.
    """
    override = os.environ.get("CADWORK_EXE_DIR")
    if override:
        return Path(override)

    reg_exe = _registry_value("CADWORK_EXE")
    if reg_exe is not None and reg_exe.exists():
        if (reg_exe / CADWORK_EXE_RELPATH).exists() or (reg_exe / "3d.exe").exists():
            return reg_exe

    install = install_dir()
    if install is not None:
        year = profile_year()
        for folder_name in (f"EXE_{year}", f"exe_{year}"):
            candidate = install / folder_name
            if (candidate / CADWORK_EXE_RELPATH).exists() or (candidate / "3d.exe").exists():
                return candidate

    return None


def ci_start_path() -> Path:
    """Resolve the path to ci_start.exe.

    Returns:
        Path to ci_start.exe.
    """
    override = os.environ.get("CADWORK_CI_START")
    if override:
        return Path(override)

    install = install_dir()
    if install is not None:
        candidate = install / "ci_start.exe"
        if candidate.exists():
            return candidate

    return Path(DEFAULT_CI_START)


def plugin_name() -> str:
    """Return the plugin target name used for deploy folders and DLL filenames."""
    override = os.environ.get("CW_API3D_PLUGIN_NAME", "").strip()
    return override or DEFAULT_PLUGIN_NAME


def plugin_dll_path() -> Path:
    """Resolve the path to the selected example plugin DLL.

    ``CW_API3D_PLUGIN_NAME`` selects the example (default ``cw_api3d_hello``).
    ``CW_API3D_PLUGIN_DLL`` overrides the resolved path entirely.

    Returns:
        Path to the plugin DLL (deployed or build artifact).
    """
    override = os.environ.get("CW_API3D_PLUGIN_DLL")
    if override:
        return Path(override)

    name = plugin_name()
    dll_name = f"{name}.dll"

    usp = userprofil_dir()
    if usp is not None:
        deployed = usp / "3d" / "API.x64" / name / dll_name
        if deployed.exists():
            return deployed

    build_candidates = [
        REPO_ROOT / "out" / "build" / "local-debug" / "bin" / "debug" / dll_name,
        REPO_ROOT / "out" / "build" / "local-release" / "bin" / "release" / dll_name,
        REPO_ROOT / "out" / "build" / "local-relwithdebinfo" / "bin" / "relwithdebinfo" / dll_name,
        REPO_ROOT / "build" / "bin" / dll_name,
    ]
    for candidate in build_candidates:
        if candidate.exists():
            return candidate

    if usp is not None:
        return usp / "3d" / "API.x64" / name / dll_name

    return build_candidates[0]


def resolved_env() -> dict[str, str]:
    """Return dictionary of all resolved environment variables.

    Returns:
        Mapping of environment variable names to resolved path strings.
    """
    resolved: dict[str, str] = {
        "CW_PROFILE_YEAR": profile_year(),
        "CADWORK_CI_START": str(ci_start_path()),
        "CW_API3D_PLUGIN_NAME": plugin_name(),
        "CW_API3D_PLUGIN_DLL": str(plugin_dll_path()),
    }
    install = install_dir()
    if install is not None:
        resolved["CADWORK_DIR"] = str(install)
    usp = userprofil_dir()
    if usp is not None:
        resolved["CADWORK_USP"] = str(usp)
    exe = exe_dir()
    if exe is not None:
        resolved["CADWORK_EXE_DIR"] = str(exe)
    return resolved


def main() -> None:
    """CLI entry point printing resolved environment variables."""
    for key, val in resolved_env().items():
        print(f"{key}={val}")


if __name__ == "__main__":
    main()
