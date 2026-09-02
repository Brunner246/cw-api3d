#!/usr/bin/env python3
"""
cadwork_batch_run.py

Batch-launches the cadwork 3D executable in console mode for a set of .3d
files, running a given RUNPROGRAM script (e.g. a cwapi3dpython script) on
each one.

Each file is opened via:
    "<exe>" "<file>.3d" /Console /RUNPROGRAM=<script.py>

Usage examples
--------------
Run one script on every .3d file in a folder (sequential):
    python cadwork_batch_run.py --exe "D:\\cadwork.dir\\ci_start.exe" ^
        --script "C:\\scripts\\export.py" --dir "C:\\Projects\\Batch"

Run on an explicit file list, 4 in parallel:
    python cadwork_batch_run.py --exe "D:\\cadwork.dir\\ci_start.exe" ^
        --script "C:\\scripts\\export.py" --workers 4 ^
        --files "C:\\P\\a.3d" "C:\\P\\b.3d" "C:\\P\\c.3d"

Dry run (just print the commands that would be executed):
    python cadwork_batch_run.py --exe ... --script ... --dir ... --dry-run
"""

from __future__ import annotations

import argparse
import concurrent.futures
import csv
import datetime as dt
import logging
import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path


# --------------------------------------------------------------------------- #
# Data model
# --------------------------------------------------------------------------- #

@dataclass
class JobResult:
    file: Path
    command: list[str]
    returncode: int | None = None
    stdout: str = ""
    stderr: str = ""
    duration_s: float = 0.0
    error: str | None = None

    @property
    def success(self) -> bool:
        return self.error is None and self.returncode == 0


@dataclass
class BatchConfig:
    exe: Path
    script: Path
    files: list[Path]
    workers: int = 1
    timeout_s: float | None = None
    extra_args: list[str] = field(default_factory=list)
    dry_run: bool = False


# --------------------------------------------------------------------------- #
# Core execution
# --------------------------------------------------------------------------- #

def build_command(cfg: BatchConfig, file: Path) -> list[str]:
    return [
        str(cfg.exe),
        str(file),
        "/Console",
        f"/RUNPROGRAM={cfg.script}",
        *cfg.extra_args,
    ]


def _as_str(value: bytes | str | None) -> str:
    """Normalize subprocess output to str regardless of typeshed's Any/bytes typing
    for TimeoutExpired.stdout/stderr (they are always str here since we pass text=True)."""
    if value is None:
        return ""
    if isinstance(value, bytes):
        return value.decode(errors="replace")
    return value


def run_one(cfg: BatchConfig, file: Path) -> JobResult:
    command = build_command(cfg, file)
    result = JobResult(file=file, command=command)

    if cfg.dry_run:
        result.returncode = 0
        result.stdout = "(dry-run, not executed)"
        return result

    start = dt.datetime.now()
    try:
        proc = subprocess.run(
            command,
            capture_output=True,
            text=True,
            timeout=cfg.timeout_s,
        )
        result.returncode = proc.returncode
        result.stdout = proc.stdout
        result.stderr = proc.stderr
    except subprocess.TimeoutExpired as exc:
        result.error = f"timeout after {cfg.timeout_s}s"
        result.stdout = _as_str(exc.stdout)
        result.stderr = _as_str(exc.stderr)
    except OSError as exc:
        result.error = f"failed to launch: {exc}"
    finally:
        result.duration_s = (dt.datetime.now() - start).total_seconds()

    return result


def run_batch(cfg: BatchConfig, log: logging.Logger) -> list[JobResult]:
    results: list[JobResult] = []

    if cfg.workers <= 1:
        for f in cfg.files:
            log.info("Running: %s", f.name)
            r = run_one(cfg, f)
            _log_result(log, r)
            results.append(r)
        return results

    log.info("Running %d files with %d parallel workers", len(cfg.files), cfg.workers)
    with concurrent.futures.ThreadPoolExecutor(max_workers=cfg.workers) as pool:
        future_to_file = {pool.submit(run_one, cfg, f): f for f in cfg.files}
        for future in concurrent.futures.as_completed(future_to_file):
            r = future.result()
            _log_result(log, r)
            results.append(r)

    # keep output order stable regardless of completion order
    order = {f: i for i, f in enumerate(cfg.files)}
    results.sort(key=lambda r: order[r.file])
    return results


def _log_result(log: logging.Logger, r: JobResult) -> None:
    if r.success:
        log.info("OK   (%.1fs) %s", r.duration_s, r.file.name)
    else:
        reason = r.error or f"exit code {r.returncode}"
        log.error("FAIL (%.1fs) %s -- %s", r.duration_s, r.file.name, reason)
        if r.stderr:
            log.debug("stderr for %s:\n%s", r.file.name, r.stderr.strip())


# --------------------------------------------------------------------------- #
# Reporting
# --------------------------------------------------------------------------- #

def write_csv_report(results: list[JobResult], path: Path) -> None:
    with path.open("w", newline="", encoding="utf-8") as fh:
        writer = csv.writer(fh)
        writer.writerow(["file", "success", "returncode", "duration_s", "error"])
        for r in results:
            writer.writerow([str(r.file), r.success, r.returncode, f"{r.duration_s:.2f}", r.error or ""])


def print_summary(results: list[JobResult], log: logging.Logger) -> None:
    ok = sum(1 for r in results if r.success)
    fail = len(results) - ok
    log.info("=" * 60)
    log.info("Batch finished: %d ok, %d failed, %d total", ok, fail, len(results))
    if fail:
        log.info("Failed files:")
        for r in results:
            if not r.success:
                log.info("  - %s (%s)", r.file, r.error or f"exit {r.returncode}")


# --------------------------------------------------------------------------- #
# CLI / setup
# --------------------------------------------------------------------------- #

def collect_files(args: argparse.Namespace) -> list[Path]:
    files: list[Path] = []

    if args.files:
        files.extend(Path(f) for f in args.files)

    if args.dir:
        base = Path(args.dir)
        pattern = "**/*.3d" if args.recursive else "*.3d"
        files.extend(sorted(base.glob(pattern)))

    if args.list_file:
        with open(args.list_file, "r", encoding="utf-8") as fh:
            files.extend(Path(line.strip()) for line in fh if line.strip() and not line.startswith("#"))

    # de-duplicate while preserving order
    seen: set[Path] = set()
    unique: list[Path] = []
    for f in files:
        rf = f.resolve()
        if rf not in seen:
            seen.add(rf)
            unique.append(f)
    return unique


def setup_logging(log_file: Path | None, verbose: bool) -> logging.Logger:
    log = logging.getLogger("cadwork_batch")
    log.setLevel(logging.DEBUG if verbose else logging.INFO)
    log.handlers.clear()

    fmt = logging.Formatter("%(asctime)s [%(levelname)s] %(message)s", "%H:%M:%S")

    console = logging.StreamHandler(sys.stdout)
    console.setFormatter(fmt)
    log.addHandler(console)

    if log_file:
        fh = logging.FileHandler(log_file, encoding="utf-8")
        fh.setFormatter(fmt)
        log.addHandler(fh)

    return log


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    p = argparse.ArgumentParser(
        description="Batch-run cadwork 3d.exe with /RUNPROGRAM over multiple .3d files."
    )
    p.add_argument("--exe", required=True, help="Path to 3d.exe")
    p.add_argument("--script", required=True, help="Path to the RUNPROGRAM .py script")

    src = p.add_argument_group("file selection (combine freely)")
    src.add_argument("--files", nargs="+", help="Explicit list of .3d files")
    src.add_argument("--dir", help="Folder to scan for .3d files")
    src.add_argument("--recursive", action="store_true", help="Scan --dir recursively")
    src.add_argument("--list-file", help="Text file with one .3d path per line (# comments allowed)")

    p.add_argument("--workers", type=int, default=1, help="Parallel cadwork instances (default: 1 = sequential)")
    p.add_argument("--timeout", type=float, default=None, help="Per-file timeout in seconds")
    p.add_argument("--extra-arg", action="append", default=[], help="Extra CLI arg to pass to 3d.exe (repeatable)")
    p.add_argument("--dry-run", action="store_true", help="Print commands without executing them")
    p.add_argument("--log-file", help="Also write logs to this file")
    p.add_argument("--report-csv", help="Write a CSV summary report to this path")
    p.add_argument("--verbose", action="store_true", help="Debug-level logging")

    return p.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    log = setup_logging(Path(args.log_file) if args.log_file else None, args.verbose)

    exe = Path(args.exe)
    script = Path(args.script)
    files = collect_files(args)

    if not files:
        log.error("No .3d files found (use --files / --dir / --list-file)")
        return 2

    if not args.dry_run:
        if not exe.exists():
            log.error("3d.exe not found: %s", exe)
            return 2
        if not script.exists():
            log.error("RUNPROGRAM script not found: %s", script)
            return 2
        missing = [f for f in files if not f.exists()]
        if missing:
            for f in missing:
                log.warning("Skipping missing file: %s", f)
            files = [f for f in files if f.exists()]
        if not files:
            log.error("No existing .3d files left to run")
            return 2

    cfg = BatchConfig(
        exe=exe,
        script=script,
        files=files,
        workers=max(1, args.workers),
        timeout_s=args.timeout,
        extra_args=args.extra_arg,
        dry_run=args.dry_run,
    )

    log.info("Batch: %d file(s), %d worker(s), script=%s", len(files), cfg.workers, script.name)
    results = run_batch(cfg, log)
    print_summary(results, log)

    if args.report_csv:
        write_csv_report(results, Path(args.report_csv))
        log.info("CSV report written to %s", args.report_csv)

    return 0 if all(r.success for r in results) else 1


if __name__ == "__main__":
    sys.exit(main())