#!/usr/bin/env python3
"""Publish which recovered functions have been RUN against the original.

`recovery_state == source_complete` records a declaration. A source annotation
says `Status: Complete` and `tools/export_recovery_inventory.py` promotes the
row; nothing anywhere checks that the body behaves like the function it
replaces. So a recovery that is complete and WRONG improves every published
figure, `machine_carried` included, and no number in the repository falls when
that happens.

This file is the input to the number that does: `unproven_recovered` in
`docs/recovery/summary.json`. It lists, per function, the oracle that executes
the ORIGINAL and compares it against the recovered source.

TWO MECHANISMS COUNT, AND THEY ARE NOT INTERCHANGEABLE.

`legacy_leaf_island` copies the original's bytes into a local object and calls
them beside the recovered body. That is only possible for a body that is
position-independent, relocation-free and call-free - the criteria in
`tools/extract_legacy_leaves.py` - which is why it reaches so few functions.

`hybrid_runtime` calls the original at its canonical address inside the hybrid
process, where the image is mapped and its imports resolve. It has no such
restriction, and it is where the coverage has to come from.

WHAT DOES NOT COUNT, and each of these was checked for rather than assumed:

  * an oracle that calls the ORIGINAL but compares nothing against recovered
    source - it proves the harness runs, not that the recovery is right;
  * an oracle that calls only the RECOVERED body against hand-written
    expectations - that is a unit test, and a wrong transcription passes it
    whenever the expectations were written from the same misreading;
  * an address used as DATA - a vtable, a global, a table base. Two of the
    addresses in `src/*_oracle.cpp` that look like functions are exactly this,
    which is why the markers are declared rather than scraped.

The proof set is therefore DECLARED, not inferred. Each oracle names what it
proves in a `PROVEN-AGAINST-ORIGINAL:` marker, and `verify` below refuses a
marker whose address is not a catalogued `source_complete` function start.
Inference from syntax was tried first: `reinterpret_cast<...>(0x00XXXXXX)`
picked up two data addresses, and a plain hex scan picked up twenty-six more.
"""

from __future__ import annotations

import argparse
import csv
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_FUNCTIONS = REPO_ROOT / "docs" / "recovery" / "functions.csv"
DEFAULT_ORACLE_SOURCES = REPO_ROOT / "src"
DEFAULT_CMAKE = REPO_ROOT / "CMakeLists.txt"
DEFAULT_OUTPUT = REPO_ROOT / "docs" / "recovery" / "proven.csv"

FIELDS = ("address", "name", "size", "mechanism", "evidence")

MARKER = re.compile(
    r"PROVEN-AGAINST-ORIGINAL:\s*(0x[0-9A-Fa-f]{6,8})")
ISLAND_ADDRESS = re.compile(r"--address\s+(0x[0-9A-Fa-f]{6,8})")
# Which invocation of the extractor is the recovery oracle's. FOUR custom
# commands run extract_legacy_leaves.py and three are not the oracle.
ISLAND_OUTPUT_MARKER = "OPENSMACX_RECOVERY_ORACLE_DIR"


def load_catalogue(path: Path) -> dict[int, dict]:
    with path.open(newline="") as handle:
        return {int(row["address"], 16): row for row in csv.DictReader(handle)}


def island_addresses(cmake_text: str) -> set[int]:
    """Addresses the `recovery-oracles` target asks the extractor for.

    Selected by the invocation's --output, NOT by the tool name. Four custom
    commands run extract_legacy_leaves.py; matching on the tool swept up
    0x004C5460 from the static-recompile target and published a proof for a
    function the recovery oracle never builds. That address is source_complete
    and a plausible-looking leaf, so nothing downstream would have objected -
    it surfaced only because the extractor's manifest held 32 entries against
    the 33 this function reported.
    """
    found: set[int] = set()
    for chunk in cmake_text.split("extract_legacy_leaves.py")[1:]:
        head = chunk.split("COMMAND")[0]
        if ISLAND_OUTPUT_MARKER not in head:
            continue
        for address in ISLAND_ADDRESS.findall(head):
            found.add(int(address, 16))
    return found


def marker_addresses(sources: Path) -> dict[int, str]:
    """{address: file} for every declared hybrid-runtime proof."""
    found: dict[int, str] = {}
    for path in sorted(sources.glob("*_oracle.cpp")):
        for address in MARKER.findall(path.read_text()):
            found[int(address, 16)] = path.name
    return found


def build_rows(catalogue, islands, markers) -> list[dict]:
    rows = []
    for address in sorted(set(islands) | set(markers)):
        entry = catalogue.get(address)
        if entry is None or entry.get("recovery_state") != "source_complete":
            continue        # verify() reports these; they are never published
        mechanisms = []
        evidence = []
        if address in islands:
            mechanisms.append("legacy_leaf_island")
            evidence.append("CMakeLists.txt recovery-oracles --address")
        if address in markers:
            mechanisms.append("hybrid_runtime")
            evidence.append(f"src/{markers[address]}")
        rows.append({
            "address": f"0x{address:08X}",
            "name": entry.get("name", ""),
            "size": entry.get("size", "0"),
            "mechanism": "+".join(mechanisms),
            "evidence": "; ".join(evidence),
        })
    return rows


def verify(catalogue, islands, markers) -> list[str]:
    """Every reason a declared proof is refused, so none is refused silently."""
    problems = []
    for address in sorted(set(islands) | set(markers)):
        entry = catalogue.get(address)
        where = markers.get(address, "CMakeLists.txt")
        if entry is None:
            problems.append(
                f"0x{address:08X} ({where}) is not a catalogued function start")
            continue
        state = entry.get("recovery_state")
        if state != "source_complete":
            problems.append(
                f"0x{address:08X} ({where}) is {state}, not source_complete: "
                f"an oracle over it proves nothing about recovered source")
    return problems


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--functions", type=Path, default=DEFAULT_FUNCTIONS)
    parser.add_argument("--oracle-sources", type=Path,
                        default=DEFAULT_ORACLE_SOURCES)
    parser.add_argument("--cmake", type=Path, default=DEFAULT_CMAKE)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--check", action="store_true",
                        help="fail if the output would differ; write nothing")
    options = parser.parse_args(argv)

    catalogue = load_catalogue(options.functions)
    islands = island_addresses(options.cmake.read_text())
    markers = marker_addresses(options.oracle_sources)

    problems = verify(catalogue, islands, markers)
    for problem in problems:
        print(f"error: {problem}", file=sys.stderr)
    if problems:
        return 2

    rows = build_rows(catalogue, islands, markers)
    lines = [",".join(FIELDS)]
    lines += [",".join(row[field] for field in FIELDS) for row in rows]
    text = "\n".join(lines) + "\n"

    if options.check:
        current = options.output.read_text() if options.output.exists() else ""
        if current != text:
            print(f"error: {options.output} is stale; regenerate it",
                  file=sys.stderr)
            return 1
        print(f"proven functions: {len(rows)} (up to date)")
        return 0

    options.output.write_text(text)
    total = sum(int(row["size"]) for row in rows)
    print(f"proven functions: {len(rows)} / {total} bytes -> {options.output}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
