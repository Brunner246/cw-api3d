"""In-cadwork E2E driver for cw_api3d plugin.

Runs inside cadwork 3D's embedded Python interpreter when invoked via
ci_start.exe <model.3d> /EXE=<exe_dir> /RUNPROGRAM=<path_to_driver.py>.

Discovers the deployed or configured charts plugin DLL, executes it via
utility_controller.run_external_program_from_custom_directory, and writes
a results.json sentinel file for the host test harness.
"""

from __future__ import annotations

import json
import os
from pathlib import Path
import sys
import time
import traceback


def _get_log_path(results_path: Path) -> Path:
    """Determine log file path from env or beside results.json."""
    custom_log = os.environ.get("CW_E2E_LOG")
    if custom_log:
        return Path(custom_log)
    return results_path.parent / "driver.log"


def _log(msg: str, log_path: Path | None = None) -> None:
    """Append a timestamped message to the driver log."""
    formatted = f"[{time.strftime('%Y-%m-%d %H:%M:%S')}] {msg}\n"
    if log_path is not None:
        try:
            log_path.parent.mkdir(parents=True, exist_ok=True)
            with open(log_path, "a", encoding="utf-8") as f:
                f.write(formatted)
        except OSError:
            pass


def _write_results(data: dict, results_path: Path, log_path: Path | None = None) -> None:
    """Safely write results.json sentinel."""
    try:
        results_path.parent.mkdir(parents=True, exist_ok=True)
        temp_path = results_path.with_suffix(".tmp")
        with open(temp_path, "w", encoding="utf-8") as f:
            json.dump(data, f, indent=2)
        # Atomic rename
        if temp_path.exists():
            temp_path.replace(results_path)
        _log(f"Wrote results to {results_path}", log_path)
    except Exception as exc:  # noqa: BLE001
        _log(f"Failed to write results.json: {exc}", log_path)


def resolve_plugin_dll() -> Path | None:
    """Resolve the charts plugin DLL from env or the cadwork userprofil."""
    override = os.environ.get("CW_API3D_PLUGIN_DLL")
    if override:
        path = Path(override)
        if path.exists():
            return path

    name = os.environ.get("CW_API3D_PLUGIN_NAME", "cw_api3d_charts").strip() or "cw_api3d_charts"
    usp = os.environ.get("CADWORK_USP")
    if usp:
        candidate = Path(usp) / "3d" / "API.x64" / name / f"{name}.dll"
        if candidate.exists():
            return candidate

    return None


def main() -> None:
    """Main in-cadwork driver entry point."""
    raw_results = os.environ.get("CW_E2E_RESULTS")
    results_path = Path(raw_results) if raw_results else (Path.cwd() / "results.json")
    log_path = _get_log_path(results_path)

    _log("--- cw_e2e_driver started ---", log_path)
    _log(f"Python executable: {sys.executable}", log_path)
    _log(f"Python version: {sys.version}", log_path)
    _log(f"Working directory: {os.getcwd()}", log_path)

    plugin_dll: Path | None = None
    try:
        # Import cadwork controllers (available only inside cadwork process)
        try:
            import utility_controller as uc  # type: ignore[import-not-found]
        except ImportError as ie:
            raise RuntimeError(
                f"Failed to import utility_controller: {ie}. Driver must run inside cadwork 3D."
            ) from ie

        plugin_dll = resolve_plugin_dll()
        if plugin_dll is None or not plugin_dll.exists():
            raise FileNotFoundError(
                f"plugin DLL not found at '{plugin_dll}'. Ensure the plugin is built and deployed."
            )

        _log(f"Executing plugin DLL via utility_controller: {plugin_dll}", log_path)

        # Execute the plugin DLL inside cadwork
        uc.run_external_program_from_custom_directory(str(plugin_dll))

        _log("Plugin execution completed successfully.", log_path)

        _write_results(
            {
                "completed": True,
                "pluginExecuted": True,
                "pluginPath": str(plugin_dll),
                "error": None,
                "timestamp": time.time(),
            },
            results_path,
            log_path,
        )

    except Exception as exc:  # noqa: BLE001
        error_msg = str(exc)
        tb = traceback.format_exc()
        _log(f"ERROR: {error_msg}\n{tb}", log_path)

        _write_results(
            {
                "completed": False,
                "pluginExecuted": False,
                "pluginPath": str(plugin_dll) if plugin_dll else None,
                "error": error_msg,
                "traceback": tb,
                "timestamp": time.time(),
            },
            results_path,
            log_path,
        )
    finally:
        _log("--- cw_e2e_driver finished ---", log_path)


# Unconditionally invoke main() when loaded by cadwork /RUNPROGRAM
main()
