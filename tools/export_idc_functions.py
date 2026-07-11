#!/usr/bin/env python3

import argparse
from pathlib import Path

from correlate_recovery_analyses import format_ranges, load_idc, sha256, write_csv


REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_IDC = REPO_ROOT / "terranx.exe.idc"
DEFAULT_OUTPUT = REPO_ROOT / "docs" / "recovery" / "ida9-functions.csv"


def main():
    parser = argparse.ArgumentParser(
        description="Normalize IDA IDC function metadata for recovery correlation")
    parser.add_argument("--idc", type=Path, default=DEFAULT_IDC)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    args = parser.parse_args()
    if not args.idc.is_file():
        parser.error(f"IDC input not found: {args.idc}")

    functions = load_idc(args.idc)
    input_hash = sha256(args.idc)
    rows = []
    for function in functions:
        flags = function["flags"]
        kind = "thunk" if flags & 0x80 else "library" if flags & 0x4 else "game"
        rows.append({
            "source_idc_sha256": input_hash,
            "address": f"0x{function['start']:08X}",
            "end_address": f"0x{function['end']:08X}",
            "size": function["size"],
            "body_ranges": format_ranges(function["body_ranges_value"]),
            "name": function["name"],
            "prototype": function["prototype"],
            "flags": f"0x{flags:X}",
            "binary_kind": kind,
        })

    args.output.parent.mkdir(parents=True, exist_ok=True)
    write_csv(args.output, list(rows[0]), rows)
    print(f"Exported {len(rows)} IDA functions to {args.output}")


if __name__ == "__main__":
    main()
