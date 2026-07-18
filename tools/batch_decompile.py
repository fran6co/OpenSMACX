#!/usr/bin/env python3
"""Batch-decompile canonical functions into the local decompile cache.

Wraps the persistent Ghidra recovery project so a whole recovery lane's worth
of decompiled C is produced by one headless invocation instead of one manual
run per function. Addresses come from an explicit list or from filters over
the committed priorities catalog, already-cached results are skipped, and a
merged manifest records every outcome. All outputs stay inside the ignored
build directory: decompiler output is derived proprietary expression and is
never committed.
"""

import argparse
import csv
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile

from local_artifact import require_local_artifact_path
from run_ghidra_analysis import configure_java, locate_analyze_headless


REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_PRIORITIES = REPO_ROOT / "docs" / "recovery" / "priorities.csv"
DEFAULT_PROJECT_DIR = REPO_ROOT / "build" / "ghidra-projects"
DEFAULT_OUTPUT_DIR = REPO_ROOT / "build" / "ghidra-decompile"
PROJECT_NAME = "live-recovery"
PROGRAM_NAME = "terranx_original.exe"
MANIFEST_NAME = "manifest.json"
RUN_MANIFEST_NAME = "run-manifest.json"


def normalize_address(value):
    return f"0x{int(value, 16):08X}"


def select_from_priorities(path, arguments):
    selected = []
    with path.open(newline="", encoding="utf-8") as file:
        for row in csv.DictReader(file):
            if arguments.recovery_state and \
                    row["recovery_state"] != arguments.recovery_state:
                continue
            if arguments.name_prefix and \
                    not row["name"].startswith(arguments.name_prefix):
                continue
            if arguments.max_size is not None and \
                    int(row["size"]) > arguments.max_size:
                continue
            if arguments.min_callers is not None and \
                    int(row["caller_count"]) < arguments.min_callers:
                continue
            if arguments.max_call_targets is not None and \
                    int(row["call_target_count"]) > arguments.max_call_targets:
                continue
            selected.append(normalize_address(row["address"]))
            if arguments.limit and len(selected) >= arguments.limit:
                break
    return selected


def load_manifest(output_dir):
    manifest_path = output_dir / MANIFEST_NAME
    if not manifest_path.is_file():
        return {}
    try:
        return json.loads(manifest_path.read_text(encoding="utf-8"))
    except json.JSONDecodeError:
        return {}


def cached_addresses(manifest, output_dir):
    cached = set()
    for address, entry in manifest.items():
        if entry.get("status") == "ok" and \
                (output_dir / entry.get("file", "")).is_file():
            cached.add(address)
    return cached


def run_headless(addresses, output_dir, arguments):
    script_dir = Path(__file__).resolve().parent / "ghidra"
    analyze_headless = locate_analyze_headless(arguments.ghidra_home)
    environment = os.environ.copy()
    configure_java(environment)
    with tempfile.NamedTemporaryFile(
            "w", encoding="utf-8", dir=output_dir, suffix=".addresses",
            delete=False) as handle:
        handle.write("\n".join(addresses) + "\n")
        address_list = Path(handle.name)
    try:
        command = [
            str(analyze_headless),
            str(arguments.project_dir),
            PROJECT_NAME,
            "-process", PROGRAM_NAME,
            "-noanalysis",
            "-scriptPath", str(script_dir),
            "-postScript", "DecompileBatch.java",
            str(address_list), str(output_dir),
            str(arguments.timeout),
        ]
        subprocess.run(command, check=True, env=environment)
    finally:
        address_list.unlink(missing_ok=True)
    run_manifest_path = output_dir / RUN_MANIFEST_NAME
    if not run_manifest_path.is_file():
        raise RuntimeError("headless decompilation produced no run manifest")
    results = json.loads(run_manifest_path.read_text(encoding="utf-8"))
    run_manifest_path.unlink()
    return results


def main():
    parser = argparse.ArgumentParser(
        description="Batch-decompile canonical functions into the local cache")
    parser.add_argument("--addresses", type=Path,
                        help="File listing one hexadecimal address per line")
    parser.add_argument("--from-priorities", type=Path, nargs="?",
                        const=DEFAULT_PRIORITIES, default=None,
                        help="Select addresses from the priorities catalog")
    parser.add_argument("--recovery-state", default="unrecovered")
    parser.add_argument("--name-prefix")
    parser.add_argument("--max-size", type=int)
    parser.add_argument("--min-callers", type=int)
    parser.add_argument("--max-call-targets", type=int)
    parser.add_argument("--limit", type=int)
    parser.add_argument("--output-dir", type=Path, default=DEFAULT_OUTPUT_DIR)
    parser.add_argument("--project-dir", type=Path, default=DEFAULT_PROJECT_DIR)
    parser.add_argument("--ghidra-home")
    parser.add_argument("--timeout", type=int, default=120,
                        help="Per-function decompilation timeout in seconds")
    parser.add_argument("--force", action="store_true",
                        help="Re-decompile addresses already in the cache")
    parser.add_argument("--dry-run", action="store_true",
                        help="Only report the addresses that would run")
    args = parser.parse_args()

    output_dir = require_local_artifact_path(
        args.output_dir, "decompile output directory")
    args.project_dir = require_local_artifact_path(
        args.project_dir, "Ghidra project directory")

    if args.addresses:
        requested = [
            normalize_address(line.strip())
            for line in args.addresses.read_text(encoding="utf-8").splitlines()
            if line.strip()
        ]
    elif args.from_priorities:
        requested = select_from_priorities(args.from_priorities, args)
    else:
        parser.error("pass --addresses or --from-priorities")
    if not requested:
        print("batch-decompile: no addresses selected")
        return 0

    output_dir.mkdir(parents=True, exist_ok=True)
    manifest = load_manifest(output_dir)
    cached = cached_addresses(manifest, output_dir)
    pending = [address for address in requested
               if args.force or address not in cached]
    print(
        f"batch-decompile: {len(requested)} requested, "
        f"{len(requested) - len(pending)} cached, {len(pending)} to run")
    if args.dry_run or not pending:
        return 0

    results = run_headless(pending, output_dir, args)
    manifest.update(results)
    (output_dir / MANIFEST_NAME).write_text(
        json.dumps(manifest, indent=1, sort_keys=True) + "\n",
        encoding="utf-8")
    failures = [address for address, entry in results.items()
                if entry.get("status") != "ok"]
    succeeded = len(results) - len(failures)
    print(f"batch-decompile: {succeeded} decompiled, {len(failures)} failed")
    if failures:
        for address in failures:
            print(f"  failed: {address}: {results[address].get('error')}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
