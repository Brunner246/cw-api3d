"""End-to-end tests for cw_api3d plugin execution inside cadwork 3D."""

from __future__ import annotations

import json
import os
from pathlib import Path
import pytest

from . import cadwork_launch
from . import cadwork_paths


def test_cadwork_paths_resolution() -> None:
    """Verify cadwork_paths functions return valid Path objects or None without error."""
    install = cadwork_paths.install_dir()
    assert install is None or isinstance(install, Path)

    usp = cadwork_paths.userprofil_dir()
    assert usp is None or isinstance(usp, Path)

    year = cadwork_paths.profile_year()
    assert isinstance(year, str) and len(year) == 4

    exe = cadwork_paths.exe_dir()
    assert exe is None or isinstance(exe, Path)

    ci_start = cadwork_paths.ci_start_path()
    assert isinstance(ci_start, Path)

    plugin_dll = cadwork_paths.plugin_dll_path()
    assert isinstance(plugin_dll, Path)


def test_stage_script_short_path(tmp_path: Path) -> None:
    """Verify that a script path whose switch token fits in 127 chars is not staged."""
    short_script = tmp_path / "driver.py"
    short_script.write_text("print('test')", encoding="utf-8")

    staged = cadwork_launch.stage_script(short_script)
    if len(f"/RUNPROGRAM={short_script.resolve()}") <= cadwork_launch.CI_START_MAX_SWITCH_LENGTH:
        assert staged == short_script.resolve()


def test_stage_script_long_path(tmp_path: Path) -> None:
    """Verify that a deeply nested script path is staged to a short directory."""
    deep_dir = tmp_path
    for i in range(10):
        deep_dir = deep_dir / f"nested_directory_level_{i}"
    deep_dir.mkdir(parents=True, exist_ok=True)
    deep_script = deep_dir / "long_driver_script.py"
    deep_script.write_text("print('staged')", encoding="utf-8")

    staged = cadwork_launch.stage_script(deep_script)
    assert len(f"/RUNPROGRAM={staged}") <= cadwork_launch.CI_START_MAX_SWITCH_LENGTH
    assert staged.exists()
    assert staged.read_text(encoding="utf-8") == "print('staged')"


def test_build_launch_command_structure(
    cadwork_installed: bool,
    model_fixture_path: Path,
    driver_script_path: Path,
) -> None:
    """Verify the command arguments constructed by build_launch_command."""
    if not cadwork_installed:
        with pytest.raises(cadwork_launch.LaunchUnavailable):
            cadwork_launch.build_launch_command(model_fixture_path, driver_script_path)
        return

    command = cadwork_launch.build_launch_command(model_fixture_path, driver_script_path)
    assert len(command) == 5
    assert command[0].endswith("ci_start.exe")
    assert command[1] == str(model_fixture_path.resolve())
    assert command[2].startswith("/EXE=")
    assert command[3] == "/AlwaysIgnoreMultiOpenProtectDlg"
    assert command[4].startswith("/RUNPROGRAM=")

# DI injection via @pytest.fixture
def test_plugin_initialization_and_execution(
    require_cadwork: None,
    driver_script_path: Path,
    e2e_run_dir: dict[str, Path],
) -> None:
    """Launch cadwork with test_model.3d, run cw_e2e_driver.py, and verify cw_api3d.dll execution."""
    work_dir = e2e_run_dir["work_dir"]
    model_path = e2e_run_dir["model_path"]
    results_path = e2e_run_dir["results_path"]
    stdout_path = e2e_run_dir["stdout_path"]
    driver_log_path = e2e_run_dir["driver_log_path"]

    # Clear any leftover locks
    cadwork_launch.sweep_cadwork_transients(work_dir)

    # Prepare environment variables
    env = os.environ.copy()
    for key, val in cadwork_paths.resolved_env().items():
        env[key] = val
    env["CW_E2E_RESULTS"] = str(results_path)
    env["CW_E2E_LOG"] = str(driver_log_path)
    env["CW_API3D_PLUGIN_DLL"] = str(cadwork_paths.plugin_dll_path())

    command = cadwork_launch.build_launch_command(model_path, driver_script_path)

    # Run cadwork and poll for results.json
    launch_result = cadwork_launch.run_until_sentinel(
        command=command,
        sentinel_path=results_path,
        timeout_seconds=60.0,
        env=env,
        stdout_path=stdout_path,
    )

    # If timeout or failure, read driver log for diagnostics
    driver_log_content = ""
    if driver_log_path.exists():
        driver_log_content = driver_log_path.read_text(encoding="utf-8", errors="replace")

    assert launch_result.status == cadwork_launch.WAIT_READY, (
        f"Launch failed with status '{launch_result.status}' (returncode={launch_result.returncode}).\n"
        f"Driver log:\n{driver_log_content}"
    )

    assert results_path.exists(), f"Sentinel results.json was not created at {results_path}"

    with open(results_path, encoding="utf-8") as f:
        results_data = json.load(f)

    assert results_data.get("completed") is True, f"Driver reported incomplete run: {results_data}"
    assert results_data.get("pluginExecuted") is True, f"Plugin was not executed: {results_data}"
    assert results_data.get("error") is None, f"Driver encountered error: {results_data.get('error')}"
