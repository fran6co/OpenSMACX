#!/usr/bin/env python3
"""Put the catalogue's facts beside every annotation, so `src/` carries them.

`docs/DECOMP_MAP.md` has said since it was written that "the decompilation is
tracked in exactly one place: the source tree… The CSVs under `docs/recovery/`
are derived views with a retirement path, not the record." That was true of the
MAP - which addresses exist and what state they are in - and false of everything
else about a function. Its name, its size, the spans it occupies and its
prototype lived only in `docs/recovery/functions.csv`, an IDA export, and every
tool that wanted them read the CSV.

Except for placeholders. All 1,863 of those already carry the facts, stamped by
`decomp_status.py --generate-placeholders`:

    // ORIGINAL: 0x00401100 FILE
    // name      ?add@StringStruct@@QAEHH@Z
    // size      1043 bytes
    // spans     0x00401100-0x00401513
    // prototype int (__thiscall ?add@StringStruct@@QAEHH@Z)(StringStruct*, int)
    // callers   21   call targets   3

Measured 2026-08-12: 1,863 of 3,478 annotated files carry that block, and it is
exactly the placeholder third. Product source, `src/recovered/` and
`src/recovered/units/` carry none of it. The projection was built for the
population that needed it and never finished, so the catalogue stayed load
bearing for the 4,108 pieces already recovered.

This finishes it, and then CHECKS it. `--check` compares what `src/` says
against what the export says, in both directions: a stamped block that
disagrees, and an annotation with no block. Once every annotation carries its
own facts, a reader wanting the name of a function can ask `src/`, and the CSV
becomes what the map already is - a derived view.

WHAT IS NOT PROJECTED, and why. `segment`, `comments` and `priority` have no
reader at all (measured: 0 tool references between them, and `priority` is empty
in every one of the 6,000 rows), so copying them into `src/` would carry dead
weight across. `body_ranges` IS projected, because a function's spans are how
the ratchet knows which bytes to compare and 402 functions carry more than one.
"""

from __future__ import annotations

import argparse
import csv
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import annotation_scan  # noqa: E402
import emit_translation_unit as emit  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
FUNCTIONS_CSV = REPO_ROOT / "docs" / "recovery" / "functions.csv"

FACT_LINE = re.compile(
    r"^\s*(?://|\*)?\s*(name|size|spans|prototype|callers|kind|flags|calls|notes"
    r"|indirect)"
    r"\s+(.*?)\s*$")


def catalogue(path: Path = None) -> dict:
    """{address: row} for the CORRECTED catalogue, not the raw export.

    THROUGH `emit.load_functions()`, WHICH APPLIES `catalogue_corrections`. The
    raw CSV is IDA's reconstruction and the tree keeps a hand-curated overlay of
    names the bytes contradict - `?vx_malloc@Caviar@@QAAXK@Z` in the export is
    `SAXK` in `src/`, because the body takes no receiver. Reading the CSV
    directly made the first run of this tool report those corrections as
    disagreements, i.e. it accused `src/` of being wrong about the one thing
    `src/` is right about. The corrected catalogue is what every other tool
    consumes and is what a projection must be compared against.
    """
    if path is not None:
        rows = {}
        with path.open(newline="", encoding="utf-8-sig") as handle:
            for row in csv.DictReader(handle):
                rows[int(row["address"], 16)] = row
        return rows
    rows = emit.load_functions()
    for address, targets in emit.load_callees().items():
        if address in rows:
            rows[address]["_calls"] = set(targets)
    for address, row in rows.items():
        row.setdefault("_calls", set())
    # Attribute each indirect call site to the function whose spans contain it.
    import json
    graph = REPO_ROOT / "docs" / "recovery" / "callgraph.json"
    if graph.is_file():
        spans = []
        for address, row in rows.items():
            for part in (row.get("body_ranges") or "").split(";"):
                if "-" in part:
                    low, high = part.split("-")
                    spans.append((int(low, 16), int(high, 16), address))
        spans.sort()
        import bisect
        starts = [s for s, _, _ in spans]
        for site in json.loads(graph.read_text()).get("indirect_call_sites", []):
            value = int(site, 16)
            index = bisect.bisect_right(starts, value) - 1
            if index >= 0 and spans[index][0] <= value < spans[index][1]:
                rows[spans[index][2]].setdefault("_indirect", set()).add(value)
    return rows


def from_source(src: Path = None) -> dict:
    """The catalogue, read back out of `src/` - the same shape `emit` returns.

    This is the direction that makes the export deletable. Every annotation
    carries its own facts and `--check` holds them to the export, so reading
    them back is not a second opinion: it is the same data with `src/` as the
    store instead of a CSV.
    """
    root = src or (REPO_ROOT / "src")
    rows = {}
    for annotation in annotation_scan.scan_tree(root):
        path = REPO_ROOT / annotation.path
        try:
            lines = path.read_text(errors="ignore").splitlines()
        except OSError:
            continue
        if not annotation.line:
            continue
        present = stamped(lines, annotation.line - 1)
        if not present:
            continue
        size = present.get("size", "").replace(" bytes", "").strip()
        calls = present.get("calls", "")
        rows[annotation.address] = {
            "address": f"0x{annotation.address:08X}",
            "name": present.get("name", ""),
            "size": size,
            "body_ranges": present.get("spans", ""),
            "prototype": present.get("prototype", ""),
            "binary_kind": present.get("kind", ""),
            "flags": present.get("flags", ""),
            "notes": present.get("notes", ""),
            "caller_count": present.get("callers", "").split()[0]
                            if present.get("callers") else "0",
            "call_target_count": present.get("callers", "").split()[-1]
                                 if present.get("callers") else "0",
            "_calls": {int(part, 16) for part in calls.split()
                       if part.startswith("0x")},
            "_indirect": {int(part, 16) for part in
                          present.get("indirect", "").split()
                          if part.startswith("0x")},
            # THE FIRST SPAN'S END, not the last. 402 functions carry a second
            # span, and MSVC outlined those cold blocks to 0x0065xxxx - so
            # taking the last one puts `end_address` in the funclet range and
            # `byte_match` reads the wrong bytes. Measured: 416 rows differed
            # that way and 241 BYTE_EXACT claims stopped reproducing, which is
            # how this was found rather than reasoned.
            "end_address": (present.get("spans", "").split(";")[0].split("-")[-1]
                            if "-" in present.get("spans", "") else ""),
        }
    return rows


def facts(address: int, row: dict) -> list:
    """The comment lines a piece carries about itself.

    Deliberately the same spelling `decomp_status._catalog_facts` already uses
    on placeholders - one projection, not two, or the check below would be
    comparing this tool against itself rather than against the export.
    """
    spans = row.get("body_ranges") or \
        f"0x{address:08X}-0x{address + int(row.get('size') or 0):08X}"
    lines = [f"// name      {row.get('name', '')}",
             f"// size      {row.get('size', '')} bytes",
             f"// spans     {spans}",
             f"// prototype {row.get('prototype', '')}",
             f"// callers   {row.get('caller_count', '')}   "
             f"call targets   {row.get('call_target_count', '')}",
             f"// kind      {row.get('binary_kind', '')}",
             f"// flags     {row.get('flags', '')}"]
    # THE CALL EDGES, which is the whole of what `callgraph.json` says about a
    # function that `src/` could not. Written as addresses rather than as real
    # calls in a body: a structural skeleton would be checked by the compiler and
    # is the better form, but it also rewrites 1,488 placeholder bodies, and the
    # question this answers - "can the frontier be computed without the JSON" -
    # is answered either way. `callgraph.json` records only direct `call rel32`,
    # so this is a FLOOR: AGENTS.md measures 43.0% of functions carrying a call
    # site it never counted, and the count above says how many are indirect.
    targets = row.get("_calls")
    if targets is not None:
        spelled = " ".join(f"0x{t:08X}" for t in sorted(targets)) or "(none)"
        lines.append(f"// calls     {spelled}")
    # THE INDIRECT CALL SITES, which is the rest of what `callgraph.json` holds:
    # 5,159 addresses whose target no static analysis can name. They are why
    # every seam count in this repository is a floor rather than an estimate, so
    # a function that has them must say so beside the `calls` line that does not
    # include them.
    indirect = row.get("_indirect")
    if indirect:
        spelled = " ".join(f"0x{site:08X}" for site in sorted(indirect))
        lines.append(f"// indirect  {spelled}")
    note = (row.get("notes") or "").strip()
    if note:
        lines.append(f"// notes     {note}")
    return lines


def stamped(lines: list, index: int) -> dict:
    """{key: value} already recorded in the comment run after a marker."""
    found = {}
    for line in lines[index + 1:]:
        stripped = line.strip()
        if not (stripped.startswith("//") or stripped.startswith("*")):
            break
        match = FACT_LINE.match(line)
        if match:
            found[match.group(1)] = match.group(2)
    return found


def survey(src: Path, rows: dict) -> tuple:
    """(missing, disagreeing, uncatalogued) over every annotation in `src`."""
    missing, disagreeing, uncatalogued = [], [], []
    for annotation in annotation_scan.scan_tree(src):
        row = rows.get(annotation.address)
        if row is None:
            uncatalogued.append(annotation)
            continue
        path = REPO_ROOT / annotation.path
        try:
            lines = path.read_text(errors="ignore").splitlines()
        except OSError:
            continue
        if not annotation.line:
            continue
        present = stamped(lines, annotation.line - 1)
        want = {}
        for line in facts(annotation.address, row):
            match = FACT_LINE.match(line)
            want[match.group(1)] = match.group(2)
        # INCOMPLETE COUNTS AS MISSING. Checking only for an absent block let a
        # block that predates a new fact pass forever - which it did the first
        # time `kind`, `flags` and `calls` were added, reporting 6,003 of 6,003
        # while not one annotation carried any of them.
        if any(key not in present for key in want):
            missing.append(annotation)
            continue
        for key, value in want.items():
            if present[key] != value:
                disagreeing.append((annotation, key, present[key], value))
    return missing, disagreeing, uncatalogued


def apply(src: Path, rows: dict, missing: list) -> int:
    """Insert the facts under each marker that has none. Returns how many."""
    by_file = {}
    for annotation in missing:
        by_file.setdefault(annotation.path, []).append(annotation)
    changed = 0
    for relative, entries in by_file.items():
        path = REPO_ROOT / relative
        lines = path.read_text(errors="ignore").splitlines(keepends=True)
        # Deepest marker first, so an insertion never moves a later one.
        for annotation in sorted(entries, key=lambda a: -a.line):
            row = rows.get(annotation.address)
            if row is None or not annotation.line:
                continue
            # Re-stamping must be IDEMPOTENT. The whole contiguous comment run
            # after the marker is rewritten: fact lines are dropped wherever
            # they sit in it and prose is kept in order, then the fresh block
            # goes in at the top. Stopping the strip at the first non-fact line
            # is not enough - a placeholder carries `// placeholder - not yet
            # decompiled` between the marker and its facts, so the old block
            # survived below the new one and every file grew a duplicate.
            end = annotation.line
            while end < len(lines):
                stripped = lines[end].strip()
                if not (stripped.startswith("//") or stripped.startswith("*")):
                    break
                end += 1
            prose = [line for line in lines[annotation.line:end]
                     if not FACT_LINE.match(line)]
            block = [line + "\n" for line in facts(annotation.address, row)]
            lines[annotation.line:end] = block + prose
            changed += 1
        path.write_text("".join(lines))
    return changed


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--src", type=Path, default=REPO_ROOT / "src")
    parser.add_argument("--functions", type=Path, default=None,
                        help="an export to compare against; required by --check")
    parser.add_argument("--apply", action="store_true",
                        help="stamp the facts onto annotations that lack them")
    parser.add_argument("--place-orphans", action="store_true",
                        help="write a placeholder for every catalogue row that "
                             "no annotation in src/ claims")
    parser.add_argument("--check", action="store_true",
                        help="exit 1 if src/ and the export disagree")
    arguments = parser.parse_args(argv)

    # A CHECK WITH NOTHING TO CHECK AGAINST MUST NOT PASS. Once the export is
    # deleted, `catalogue()` reads `src/` - so comparing them would be comparing
    # src/ with itself and printing OK forever, which is the vacuous-gate shape
    # this tree has caught in itself three times. `--check` therefore REQUIRES an
    # explicit export to compare against: regenerate one from the IDB with
    # `export_recovery_inventory.py --output <path>` and point this at it.
    if arguments.check and not arguments.functions:
        print("FAIL: --check needs an export to compare against. `src/` is the "
              "store now, so with no --functions this would compare src/ with "
              "itself and pass on anything. Regenerate one from the IDB with "
              "tools/export_recovery_inventory.py and pass --functions.",
              file=sys.stderr)
        return 1

    rows = catalogue(arguments.functions)
    missing, disagreeing, uncatalogued = survey(arguments.src, rows)
    total = len(annotation_scan.scan_tree(arguments.src))

    print(f"catalogue projection: {total - len(missing)} of {total} "
          f"annotation(s) carry their own facts")

    if disagreeing:
        print(f"  {len(disagreeing)} DISAGREE with the export:", file=sys.stderr)
        for annotation, key, mine, theirs in disagreeing[:10]:
            print(f"    {annotation.location} {key}: src says {mine!r}, "
                  f"the export says {theirs!r}", file=sys.stderr)
    if uncatalogued:
        print(f"  {len(uncatalogued)} annotation(s) name an address the export "
              f"does not have")

    if arguments.place_orphans:
        # THE CATALOGUE'S `source_complete` IS THE STALE HALF HERE, and saying
        # so is the whole point of `src/` being the store. These rows carry that
        # state because a `Status: Complete` annotation once existed for them;
        # none does now, and the bodies are not in the tree under any spelling -
        # `??0Heap@@QAE@XZ` at 0x005D4560 has no `Heap::Heap` anywhere in src/.
        # `--generate-placeholders` refuses them for the right reason under the
        # old ordering (never contradict the catalogue) and the wrong one under
        # this one: src/ measures state, the export only remembers it.
        placed = 0
        directory = REPO_ROOT / "src" / "unrecovered"
        directory.mkdir(parents=True, exist_ok=True)
        known = {a.address for a in annotation_scan.scan_tree(arguments.src)}
        for address, row in sorted(rows.items()):
            if address in known:
                continue
            path = directory / f"{address:08x}.cpp"
            if path.exists():
                continue
            body = [f"// ORIGINAL: 0x{address:08X} FILE"]
            body += facts(address, row)
            body += ["// unlocated - the catalogue records this address as",
                     "// source_complete, but no body for it exists in src/ under",
                     "// any spelling. src/ measures state; the export remembers",
                     "// it, and here they disagree. Treated as unrecovered.",
                     "",
                     "// BODY GOES HERE.",
                     ""]
            path.write_text("\n".join(body))
            placed += 1
        print(f"placed {placed} orphan row(s) as placeholders")
        return 0

    if arguments.apply:
        changed = apply(arguments.src, rows, missing)
        print(f"stamped {changed} annotation(s)")
        return 0

    if arguments.check:
        # A disagreement is always a defect. A MISSING block is one too, but
        # only once the projection is complete - reported either way, and the
        # exit code says which of the two states this tree is in.
        if disagreeing:
            print(f"FAIL: src/ and the export disagree on {len(disagreeing)} "
                  f"fact(s)", file=sys.stderr)
            return 1
        if missing:
            print(f"FAIL: {len(missing)} annotation(s) carry no catalogue "
                  f"facts; run --apply", file=sys.stderr)
            return 1
        if not total:
            print("FAIL: no annotations were scanned, so this proves nothing",
                  file=sys.stderr)
            return 1
        print("OK: every annotation carries its own catalogue facts, and none "
              "disagrees with the export")
        return 0

    print(f"  {len(missing)} carry none; run --apply to stamp them")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
