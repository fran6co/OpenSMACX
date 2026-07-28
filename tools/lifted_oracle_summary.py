#!/usr/bin/env python3
"""Summarise build/oracle/report.tsv over the WHOLE image.

The oracle's own end-of-run summary covers only the last segment of a sweep:
every tally resets when lifted_oracle_sweep.sh restarts the process after a
hang, so the printed figures describe a suffix of the plan, not the plan. The
report is the only whole-image record, so the whole-image numbers come from
here.

Two things this reports that the oracle's summary cannot:

  * BYTE WEIGHTING. This project's own measure - "40% of functions is 8% of
    bytes". A count of functions flatters any coverage claim, because the
    passing functions are small: the median is tens of bytes.
  * HOW MANY SEEDS ACTUALLY COMPARED. A PASS may rest on one seed of four,
    with the other three unable to run the original at all. Cases attempted
    and cases compared are different numbers and only the second is evidence.
"""

from __future__ import annotations

import csv
import sys
from collections import Counter, defaultdict
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
REPORT = REPO_ROOT / "build" / "oracle" / "report.tsv"
FUNCTIONS = REPO_ROOT / "docs" / "recovery" / "functions.csv"


def function_sizes() -> dict[int, int]:
    sizes: dict[int, int] = {}
    with FUNCTIONS.open() as handle:
        for row in csv.DictReader(handle):
            if row["recovery_state"] == "external_library":
                continue
            try:
                sizes[int(row["address"], 16)] = int(row["size"] or 0)
            except ValueError:
                continue
    return sizes


def main() -> int:
    path = Path(sys.argv[1]) if len(sys.argv) > 1 else REPORT
    if not path.exists():
        print(f"no {path}", file=sys.stderr)
        return 2
    sizes = function_sizes()

    rows = []
    with path.open() as handle:
        header = handle.readline().rstrip("\n").split("\t")
        has_compared = "compared" in header
        for line in handle:
            parts = line.rstrip("\n").split("\t")
            if len(parts) < 4 or not parts[0].startswith("0x"):
                continue
            rows.append(parts)

    counts: Counter[str] = Counter()
    byte_counts: Counter[str] = Counter()
    compared_histogram: Counter[int] = Counter()
    failures = []
    seen = set()
    for parts in rows:
        address = int(parts[0], 16)
        if address in seen:      # a resumed sweep can repeat a boundary row
            continue
        seen.add(address)
        verdict = parts[1]
        size = sizes.get(address, 0)
        counts[verdict] += 1
        byte_counts[verdict] += size
        if has_compared and verdict == "PASS":
            compared_histogram[int(parts[3])] += 1
        if verdict.startswith("FAIL"):
            detail = parts[4] if has_compared else parts[3]
            name = parts[-1]
            failures.append((address, size, verdict, detail, name))

    total_functions = sum(counts.values())
    total_bytes = sum(byte_counts.values()) or 1
    print(f"{path}: {total_functions} functions, {total_bytes} bytes\n")
    print(f"{'verdict':30s} {'functions':>9s} {'bytes':>10s} {'% bytes':>8s}")
    for verdict, count in counts.most_common():
        print(f"{verdict:30s} {count:9d} {byte_counts[verdict]:10d} "
              f"{100.0 * byte_counts[verdict] / total_bytes:7.2f}%")

    executed = sum(c for v, c in counts.items()
                   if v in ("PASS", "FAIL", "FAIL-lifted-fault")
                   or v.startswith("INCONCLUSIVE-lifted"))
    executed_bytes = sum(b for v, b in byte_counts.items()
                         if v in ("PASS", "FAIL", "FAIL-lifted-fault")
                         or v.startswith("INCONCLUSIVE-lifted"))
    print(f"\nlifted body executed at least once: {executed} functions, "
          f"{executed_bytes} bytes ({100.0 * executed_bytes / total_bytes:.2f}% "
          f"of image bytes)")
    print(f"compared and agreed:                {counts['PASS']} functions, "
          f"{byte_counts['PASS']} bytes "
          f"({100.0 * byte_counts['PASS'] / total_bytes:.2f}% of image bytes)")

    if compared_histogram:
        print("\nseeds that actually compared, among the PASSes:")
        for seeds in sorted(compared_histogram):
            count = compared_histogram[seeds]
            print(f"  {seeds} of 4 seeds: {count:5d} "
                  f"({100.0 * count / max(counts['PASS'], 1):.1f}%)")
        one = compared_histogram.get(1, 0)
        print(f"  -> {one} PASSes rest on a single seed; the original side "
              f"could not be run on the other three.")

    if failures:
        print(f"\n{len(failures)} FAILING functions "
              f"({sum(f[1] for f in failures)} bytes):")
        for address, size, verdict, detail, name in sorted(
                failures, key=lambda f: -f[1]):
            print(f"\n  {address:#010x}  {size:6d} B  {verdict}  {name}")
            for piece in detail.split("; "):
                if piece.strip():
                    print(f"      {piece.strip()}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
