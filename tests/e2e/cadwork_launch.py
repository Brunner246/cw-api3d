"""Cadwork ci_start.exe process launcher and sentinel poller for E2E testing.

Handles building command lines for ci_start.exe, staging scripts exceeding
the 127-character switch-token buffer limit, sweeping cadwork lock transients,
and polling for sentinel results.json with timeout and process reaping.
"""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import time

try:
    from . import cadwork_paths
except ImportError:
    import cadwork_paths  # type: ignore[no-redef]

# ci_start.exe forwards switch tokens through a 128-byte buffer,
# silently truncating tokens longer than 127 characters.
CI_START_MAX_SWITCH_LENGTH = 127

# run_until_sentinel outcomes
WAIT_READY = "ready"
WAIT_TIMEOUT = "timeout"
WAIT_LAUNCHER_FAILED = "launcher-failed"


class LaunchUnavailable(Exception):
    """Raised when a launch prerequisite is missing (e.g. cadwork not installed)."""


class LaunchError(Exception):
    """Raised when the launch command cannot be safely formed (e.g. token length overflow)."""


@dataclass(frozen=True)
class LaunchResult:
    """Outcome of an observed cadwork launch run.

    Attributes:
        status: One of WAIT_READY, WAIT_TIMEOUT, or WAIT_LAUNCHER_FAILED.
        returncode: Exit code of the launcher process if exited, otherwise None.
    """

    status: str
    returncode: int | None


def _noop_log(_message: str) -> None:
    """Default no-op logger."""


def is_cadwork_lock_file(path: Path) -> bool:
    """Check if a path represents a cadwork model lock file (.~*.3d or ~*.3d)."""
    return path.name.startswith(".~") or path.name.startswith("~")


def is_cadwork_transient(path: Path) -> bool:
    """Check if a path is a transient cadwork lock or backup file."""
    return is_cadwork_lock_file(path) or path.name.lower().endswith((".bak", ".dbak"))


def sweep_cadwork_transients(directory: Path | str) -> None:
    """Delete cadwork lock and backup files beside the model (best effort)."""
    try:
        entries = list(Path(directory).iterdir())
    except OSError:
        return

    for entry in entries:
        if entry.is_file() and is_cadwork_transient(entry):
            try:
                entry.unlink()
            except OSError:
                pass


def stage_script(source: Path | str) -> Path:
    """Stage a Python script if its /RUNPROGRAM= token exceeds 127 characters.

    Args:
        source: Absolute or relative path to the Python driver script.

    Returns:
        Path to the script (original if within limits, or copied to a short staging dir).

    Raises:
        LaunchError: If the token exceeds 127 chars and cannot be staged to a shorter path.
    """
    source_path = Path(source).resolve()
    if len(f"/RUNPROGRAM={source_path}") <= CI_START_MAX_SWITCH_LENGTH:
        return source_path

    # Attempt staging in %TEMP% or %LOCALAPPDATA%
    candidates = [
        tempfile.gettempdir(),
        os.environ.get("LOCALAPPDATA", ""),
    ]

    for base in candidates:
        if not base:
            continue
        staged = Path(base) / "cw_e2e" / source_path.name
        if len(f"/RUNPROGRAM={staged}") <= CI_START_MAX_SWITCH_LENGTH:
            staged.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(source_path, staged)
            return staged

    raise LaunchError(
        f"/RUNPROGRAM={source_path} exceeds ci_start.exe switch token limit of "
        f"{CI_START_MAX_SWITCH_LENGTH} characters and cannot be staged to a shorter path."
    )


def build_launch_command(model_path: Path | str, script_path: Path | str) -> list[str]:
    """Build the command line for launching cadwork via ci_start.exe.

    Args:
        model_path: Path to the .3d CAD model file.
        script_path: Path to the Python script to run via /RUNPROGRAM.

    Returns:
        List of command-line arguments for ci_start.exe.

    Raises:
        LaunchUnavailable: If cadwork exe directory or ci_start.exe is missing.
        LaunchError: If any switch token exceeds the 127 character limit.
    """
    exe = cadwork_paths.exe_dir()
    if exe is None:
        raise LaunchUnavailable(
            "Cadwork exe directory (exe_<year>) not found. Set CADWORK_EXE_DIR to specify."
        )

    ci_start = cadwork_paths.ci_start_path()
    if not ci_start.exists():
        raise LaunchUnavailable(
            f"Cadwork launcher ci_start.exe not found at {ci_start}. Set CADWORK_CI_START to specify."
        )

    model_resolved = Path(model_path).resolve()
    staged_script = stage_script(script_path)

    command = [
        str(ci_start),
        str(model_resolved),
        f"/EXE={exe}",
        "/AlwaysIgnoreMultiOpenProtectDlg",
        f"/RUNPROGRAM={staged_script}",
    ]

    # Verify switch token lengths (tokens start from index 2)
    oversized = [token for token in command[2:] if len(token) > CI_START_MAX_SWITCH_LENGTH]
    if oversized:
        raise LaunchError(
            f"Switch token(s) exceed ci_start.exe {CI_START_MAX_SWITCH_LENGTH}-char limit: {oversized}"
        )

    return command


def run_until_sentinel(
    command: list[str],
    sentinel_path: Path | str,
    timeout_seconds: float = 60.0,
    env: dict[str, str] | None = None,
    stdout_path: Path | str | None = None,
    log: Callable[[str], None] | None = None,
) -> LaunchResult:
    """Run cadwork launch command and poll for the sentinel file until timeout.

    Args:
        command: Launch command list.
        sentinel_path: Path to the expected sentinel file (e.g. results.json).
        timeout_seconds: Maximum time in seconds to wait for sentinel file.
        env: Optional environment dictionary.
        stdout_path: Optional path to write launcher stdout/stderr.
        log: Optional logger function.

    Returns:
        LaunchResult indicating status (WAIT_READY, WAIT_TIMEOUT, WAIT_LAUNCHER_FAILED).
    """
    logger = log or _noop_log
    sentinel = Path(sentinel_path)
    deadline = time.monotonic() + timeout_seconds
    next_log = time.monotonic() + 10.0

    stdout_handle = None
    if stdout_path is not None:
        out_file = Path(stdout_path)
        out_file.parent.mkdir(parents=True, exist_ok=True)
        stdout_handle = open(out_file, "w", encoding="utf-8")  # noqa: SIM115

    try:
        process = subprocess.Popen(
            command,
            env=env,
            stdout=stdout_handle or subprocess.DEVNULL,
            stderr=subprocess.STDOUT if stdout_handle else subprocess.DEVNULL,
        )

        try:
            while time.monotonic() < deadline:
                if sentinel.exists():
                    logger(f"Sentinel file {sentinel.name} detected.")
                    return LaunchResult(status=WAIT_READY, returncode=process.poll())

                returncode = process.poll()
                if returncode is not None and returncode != 0:
                    logger(f"Launcher process exited with non-zero exit code: {returncode}")
                    return LaunchResult(status=WAIT_LAUNCHER_FAILED, returncode=returncode)

                if time.monotonic() >= next_log:
                    remaining = int(deadline - time.monotonic())
                    logger(f"Waiting for {sentinel.name}... {remaining}s remaining.")
                    next_log += 10.0

                time.sleep(0.5)

            logger(f"Timed out after {timeout_seconds}s waiting for {sentinel.name}.")
            return LaunchResult(status=WAIT_TIMEOUT, returncode=process.poll())
        finally:
            if process.poll() is None:
                process.terminate()
                try:
                    process.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    process.kill()
    finally:
        if stdout_handle is not None:
            stdout_handle.close()
