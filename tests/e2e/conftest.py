"""Pytest fixtures for cw-api3d E2E test harness."""

from __future__ import annotations

import os
from pathlib import Path
import shutil
import pytest

from . import cadwork_launch
from . import cadwork_paths

E2E_ROOT = Path(__file__).resolve().parent
FIXTURES_DIR = E2E_ROOT / "fixtures"
DRIVER_DIR = E2E_ROOT / "driver"


@pytest.fixture(scope="session")
def cadwork_installed() -> bool:
    """Check if cadwork is installed and accessible on this machine."""
    exe = cadwork_paths.exe_dir()
    ci_start = cadwork_paths.ci_start_path()
    return exe is not None and ci_start.exists()


@pytest.fixture
def require_cadwork(cadwork_installed: bool) -> None:
    """Skip test if cadwork 3D is not installed on this machine."""
    if not cadwork_installed:
        pytest.skip("Cadwork 3D is not installed or ci_start.exe is unavailable on this machine.")


@pytest.fixture(scope="session")
def model_fixture_path() -> Path:
    """Return path to the committed CAD model fixture."""
    fixture_path = FIXTURES_DIR / "test_model.3d"
    if not fixture_path.exists():
        pytest.fail(f"Model fixture not found at {fixture_path}. Ensure Git LFS assets are pulled.")
    return fixture_path


@pytest.fixture(scope="session")
def driver_script_path() -> Path:
    """Return path to the in-cadwork driver script."""
    driver_path = DRIVER_DIR / "cw_e2e_driver.py"
    if not driver_path.exists():
        pytest.fail(f"Driver script not found at {driver_path}.")
    return driver_path


@pytest.fixture
def e2e_run_dir(tmp_path: Path, model_fixture_path: Path) -> dict[str, Path]:
    """Prepare an isolated temporary directory for an E2E test run."""
    run_dir = tmp_path / "e2e_run"
    run_dir.mkdir(parents=True, exist_ok=True)

    # Copy fixture model so cadwork opens an isolated copy
    run_model = run_dir / "test_model.3d"
    shutil.copyfile(model_fixture_path, run_model)

    return {
        "work_dir": run_dir,
        "model_path": run_model,
        "results_path": run_dir / "results.json",
        "stdout_path": run_dir / "stdout.log",
        "driver_log_path": run_dir / "driver.log",
    }
