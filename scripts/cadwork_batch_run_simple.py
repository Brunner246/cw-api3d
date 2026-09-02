#!/usr/bin/env python3
"""Run a cadwork RUNPROGRAM script over a set of .3d files, one model at a time.

A stripped-down companion to cadwork_batch_run.py: sequential only, no CSV
report, no logging configuration, stdlib only. Each model is opened with

    "<exe>" "<model>.3d" /Console /RUNPROGRAM=<script.py>

Examples
--------
Every .3d file in a folder:
    python cadwork_batch_run_simple.py --exe "D:\\cadwork.dir\\ci_start.exe" \
        --script "C:\\scripts\\export.py" "C:\\Projects\\Batch"

An explicit list, printing the commands instead of running them:
    python cadwork_batch_run_simple.py --exe ... --script ... --dry-run a.3d b.3d
"""

from __future__ import annotations

import argparse
from pathlib import Path
import subprocess
import sys

# A headless cadwork blocked on a modal dialog never exits on its own, so the
# batch always runs under a deadline rather than defaulting to "wait forever".
DEFAULT_TIMEOUT_S = 600.0


def collect_models(paths: list[Path], recursive: bool) -> list[Path]:
    """Expand a mix of .3d files and folders into a de-duplicated model list."""
    models: list[Path] = []
    for path in paths:
        if path.is_dir():
            models.extend(sorted(path.glob("**/*.3d" if recursive else "*.3d")))
        else:
            models.append(path)

    seen: set[Path] = set()
    unique: list[Path] = []
    for model in models:
        resolved = model.resolve()
        if resolved not in seen:
            seen.add(resolved)
            unique.append(model)
    return unique


def build_command(exe: Path, script: Path, model: Path) -> list[str]:
    """Build the cadwork console invocation for one model."""
    return [str(exe), str(model), "/Console", f"/RUNPROGRAM={script}"]


def run_one(exe: Path, script: Path, model: Path, timeout_s: float) -> str | None:
    """Run one model, returning None on success or a short failure reason."""
    try:
        completed = subprocess.run(
            build_command(exe, script, model),
            capture_output=True,
            text=True,
            timeout=timeout_s,
            check=False,
        )
    except subprocess.TimeoutExpired:
        return f"timed out after {timeout_s:.0f}s"
    except OSError as exc:
        return f"failed to launch: {exc}"

    if completed.returncode != 0:
        stderr_tail = completed.stderr.strip().splitlines()
        detail = f": {stderr_tail[-1]}" if stderr_tail else ""
        return f"exit code {completed.returncode}{detail}"
    return None


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Run a cadwork /RUNPROGRAM script over several .3d files, sequentially."
    )
    parser.add_argument(
        "paths",
        nargs="+",
        type=Path,
        help=".3d files, folders containing them, or a mix of both",
    )
    parser.add_argument("--exe", required=True, type=Path, help="Path to cadwork 3d.exe")
    parser.add_argument(
        "--script", required=True, type=Path, help="Path to the RUNPROGRAM .py script"
    )
    parser.add_argument(
        "-r", "--recursive", action="store_true", help="Search given folders recursively"
    )
    parser.add_argument(
        "--timeout",
        type=float,
        default=DEFAULT_TIMEOUT_S,
        metavar="SECONDS",
        help=f"Per-model timeout (default: {DEFAULT_TIMEOUT_S:.0f})",
    )
    parser.add_argument(
        "--dry-run", action="store_true", help="Print the commands instead of running them"
    )
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)

    models = collect_models(args.paths, args.recursive)
    if not models:
        print("No .3d files found.", file=sys.stderr)
        return 2

    if args.dry_run:
        for model in models:
            print(subprocess.list2cmdline(build_command(args.exe, args.script, model)))
        return 0

    for required in (args.exe, args.script):
        if not required.is_file():
            print(f"Not found: {required}", file=sys.stderr)
            return 2

    missing = [model for model in models if not model.is_file()]
    for model in missing:
        print(f"Skipping missing path: {model}", file=sys.stderr)
    models = [model for model in models if model.is_file()]
    if not models:
        print("No existing .3d files left to run.", file=sys.stderr)
        return 2

    failures: list[tuple[Path, str]] = []
    for index, model in enumerate(models, start=1):
        print(f"[{index}/{len(models)}] {model.name} ... ", end="", flush=True)
        reason = run_one(args.exe, args.script, model, args.timeout)
        print("ok" if reason is None else f"FAILED ({reason})")
        if reason is not None:
            failures.append((model, reason))

    print(f"\n{len(models) - len(failures)} ok, {len(failures)} failed, {len(models)} total")
    for model, reason in failures:
        print(f"  - {model}: {reason}")

    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
