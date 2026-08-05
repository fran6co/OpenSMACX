#!/usr/bin/env python3
"""Land a byte-exact Mizuchi match in a tracked file, or refuse and change nothing.

The Mizuchi pipeline ends at a verdict: `mizuchi-integrator.mjs` gates a match
through `byte_match.py` and ratchets the ledger row. That records that a
BYTE_EXACT body EXISTS, but leaves it in `build/byte-match/<hex>/unit.cpp` -
a build artefact, gitignored, wiped by a clean. The recovery only lands when
the body is committed, and doing that by hand for every match is the step that
does not scale.

TWO DESTINATIONS, because the catalogue holds two populations.

A function with a `src/` `source_locations` has a file that owns it and a
catalogued span to replace. That is the splice below, and it is the whole
original purpose of this tool.

A function WITHOUT one has never been placed in the tree, and every one of the
2,783 unrecovered rows is in that state - which is to say the entire population
the agent loop works on. Choosing a file, a position and an include set for a
recovery is a decision this tool does not make; that has not changed. What DID
change is the consequence of refusing: the match was simply lost, so a run
could spend a day proving thousands of bodies byte-exact and commit nothing but
ledger rows. So those bodies are now written to

    src/recovered/<address>.cpp

one file per function, named for the address. It sits under `src/` because it
is source rather than documentation, and in its own directory because it is
not yet PRODUCT source: `OPENSMACX_SOURCES` lists every compiled file by hand
and does not list these, so nothing here reaches the build until someone
rewrites it in the tree's own style. `src/recovered/README.md` says the rest.

`source_locations` is NOT updated to point at the store. That field means
"placed in the tree as product source", and `export_recovery_inventory` reads
`Status: Complete` through it to set `recovery_state` while
`audit_recovered_signatures` reads it to compare signatures. Pointing it at a
verification artefact would make both of them report something that is not
true. The filename is the index instead.

WHAT IT REPLACES, AND WHAT IT KEEPS. `functions.csv` points at a line inside
the doc comment that precedes the definition, so the catalogued span covers
BOTH the comment and the body. This tool replaces only the definition and
keeps the comment verbatim, because the comment is not decoration: the
`Status: Complete` line in it is what `export_recovery_inventory` reads to set
`recovery_state`, and regenerating it from a mangled name would lose the
Purpose and Return Value prose a human wrote. The span comes from
`byte_match_census.body_span`, the same parser the census compiles through, so
a writer and a reader cannot disagree about where a body ends.

IT VERIFIES FROM `src/`, NOT FROM THE UNIT IT WAS HANDED. The integrator has
already proved that some text is BYTE_EXACT; what is unproved after an edit is
that the text now IN `src/` is that text. Those differ whenever the splice is
wrong - a doc comment absorbed into the body, a trailing `}` left behind, a
brace counter that stopped early. So the check re-extracts the body out of the
edited file, rebuilds the unit through the census scaffolding, and demands
BYTE_EXACT again. Anything less restores every file this tool touched. A
writeback that cannot prove itself leaves no trace.

WHAT IT DOES NOT PROVE. `src/` compiles under CMake with the project's real
headers; the verification here compiles the census scaffolding, which declares
what the body needs rather than including it. A body can be byte-exact and
still not build in the tree - a type spelled the way the scaffold spells it, a
header the file does not include. The project build is the arbiter of that,
and this tool does not run it.

LINE NUMBERS MOVE. A replacement of a different length shifts every catalogued
location BELOW it in the same file. Three committed catalogues carry those
pointers and are updated here: `docs/recovery/functions.csv`,
`docs/recovery/byte-match.csv` and `mizuchi/source-map.json`. Two more are
derived and are NOT patched - `docs/recovery/summary.json` and the metadata
caches - because they are regenerated from the IDB by
`tools/verify_recovery_metadata.py`, and hand-editing a generated catalogue is
exactly what that tool is built to catch. After a batch of writebacks, run it.

Usage:
    tools/mizuchi_writeback.py 0x00601B80 path/to/matched.cpp
    tools/mizuchi_writeback.py '?set_loc@BasePop@@QAEXHH@Z' matched.cpp --json
"""

from __future__ import annotations

import argparse
import csv
import json
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import byte_match  # noqa: E402
import byte_match_census as census  # noqa: E402
import emit_translation_unit as emit  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
FUNCTIONS_CSV = REPO_ROOT / "docs" / "recovery" / "functions.csv"
LEDGER_CSV = REPO_ROOT / "docs" / "recovery" / "byte-match.csv"
SOURCE_MAP = REPO_ROOT / "mizuchi" / "source-map.json"
MATCHED_DIR = REPO_ROOT / "src" / "recovered"

MATCHED_HEADER = """\
// 0x{address:08X}  {name}  ->  {symbol}
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.
"""


class Refused(Exception):
    """The writeback did not happen and nothing was changed."""


def resolve_address(target: str, functions: dict) -> int:
    """An address from either `0x00601B80` or a mangled name."""
    try:
        return int(target, 16)
    except ValueError:
        pass
    for address, row in functions.items():
        if row.get("name") == target:
            return address
    raise Refused(f"{target} is neither a hex address nor a catalogued name")


def split_definition(lines: list) -> int:
    """Index into `lines` where the definition starts, past any doc comment.

    The catalogued span opens mid-comment (see `body_span`), so the definition
    begins after the comment closes. `*/` is looked for only BEFORE the first
    `{`: a `*/` inside the body would otherwise move the boundary past real
    code and the splice would eat it.
    """
    for index, line in enumerate(lines):
        if "{" in line:
            return 0
        if "*/" in line:
            return index + 1
    raise Refused("catalogued span has no opening brace")


def shift_csv(path: Path, field: str, source_file: str, after: int, delta: int) -> bool:
    """Shift `file:line` pointers below an edit. True when the file changed."""
    if delta == 0 or not path.is_file():
        return False
    with path.open(newline="", encoding="utf-8-sig") as handle:
        reader = csv.DictReader(handle)
        fieldnames = reader.fieldnames
        rows = list(reader)
    if field not in (fieldnames or ()):
        return False
    changed = False
    for row in rows:
        parts, moved = [], False
        for location in (row.get(field) or "").split(";"):
            stripped = location.strip()
            head, _, tail = stripped.rpartition(":")
            if head == source_file and tail.isdigit() and int(tail) > after:
                stripped = f"{head}:{int(tail) + delta}"
                moved = True
            parts.append(stripped)
        if moved:
            row[field] = ";".join(parts)
            changed = True
    if changed:
        with path.open("w", newline="", encoding="utf-8") as handle:
            writer = csv.DictWriter(handle, fieldnames=fieldnames)
            writer.writeheader()
            writer.writerows(rows)
    return changed


def shift_source_map(path: Path, source_file: str, after: int, delta: int) -> bool:
    """The same shift over `mizuchi/source-map.json`, which Mizuchi reads."""
    if delta == 0 or not path.is_file():
        return False
    entries = json.loads(path.read_text())
    changed = False
    for entry in entries:
        if entry.get("file") == source_file and int(entry.get("line", 0)) > after:
            entry["line"] = int(entry["line"]) + delta
            changed = True
    if changed:
        path.write_text(json.dumps(entries, indent=2) + "\n")
    return changed


def matched_path(address: int) -> Path:
    """Where a match with no `src/` home is kept. Named for the address.

    One file per function is deliberate: `docs/recovery/` is `merge=binary` in
    `.gitattributes` precisely because every recovery rewrites the same shared
    catalogues and two branches always collide there. Two branches landing two
    different functions here touch no common file at all.
    """
    return MATCHED_DIR / f"{address:08x}.cpp"


def read_matched_body(path: Path) -> str:
    """The definition out of a stored file, without its provenance header.

    The header is the leading `//` run and nothing else, so a body that opens
    with its own comment is not confused for one: the split is at the first
    line that is not a `//` comment, and everything from there is the body.
    """
    lines = path.read_text().splitlines()
    index = 0
    while index < len(lines) and (not lines[index].strip()
                                  or lines[index].lstrip().startswith("//")):
        index += 1
    return "\n".join(lines[index:]).strip() + "\n"


def build_unit(address: int, body: str, functions: dict, callees: dict,
               derived: dict, pe_fast) -> str:
    """Scaffolding + declfix + body: the unit the integrator's gate compiled.

    Split out so `byte_match_fanout --collect` can rebuild a stored body's unit
    with ITS hoisted catalogue and PE reader rather than re-parsing both per
    file. Raises `emit.Unsettled` when there is no scaffolding to build on.
    """
    from mizuchi_declfix import fix_declarations

    scaffolding = emit.emit(address, functions, derived, callees, pe_fast,
                            scaffolding_only=True)
    callee_rows = [functions[target] for target in callees.get(address, [])
                   if target in functions]
    return fix_declarations(scaffolding, callee_rows) + "\n" + body


def verify(address: int, body: str) -> dict:
    """Rebuild the unit around a body READ BACK FROM DISK and re-measure it.

    The unit is assembled the way `mizuchi_context.py` assembles the one the
    integrator gated - scaffolding, then `mizuchi_declfix`, then the body -
    rather than the way the census assembles its own. The census omits declfix,
    which respells callee declarations so VC6 re-mangles them to the catalogued
    names; without it a body calling a CRT function reads as NO_COMPILE. Using
    the census recipe here would refuse writebacks whose gate had just passed,
    and the refusal would be an artefact of the check rather than of the code.
    The ONLY difference from the gate is where the body came from, which is
    exactly the thing under test - so the caller reads it off disk AFTER the
    write rather than passing the text it was handed. Passing the input back in
    would make this check assert that the tool was given what it was given.
    """
    import pefile  # imported here so a --no-verify run needs no PE reader

    functions = emit.load_functions()
    callees = emit.load_callees()

    # No content policy here. This used to run the census's REFUSE_SUBSTRINGS
    # over the body, which rejected `__asm` and `std::` before compiling. Both
    # are the census's business, not this tool's: the census is a measurement
    # that must be comparable across thousands of bodies, so it excludes shapes
    # it cannot compile under BOTH toolchains. A writeback has exactly one
    # question — is what is now on disk still byte-exact — and the compiler
    # answers it. A body that cannot compile comes back NO_COMPILE and gets
    # reverted for that reason, which is a fact about the code rather than a
    # substring match on it.
    pe_fast = pefile.PE(str(byte_match.DEFAULT_EXE), fast_load=True)
    try:
        unit = build_unit(address, body, functions, callees,
                          emit.load_derived(), pe_fast)
    except emit.Unsettled as error:
        return {"tier": "REFUSED", "refusal_reason": f"no scaffolding: {error}"}

    pe = pefile.PE(str(byte_match.DEFAULT_EXE))
    catalogue = byte_match.load_rows()
    shared = byte_match.shared_span_index(catalogue)
    with tempfile.TemporaryDirectory() as tmp:
        return byte_match.match_function(pe, catalogue, shared, address, unit,
                                         Path(tmp), "", f"w{address:08x}")


def land(address: int, row: dict, code: str, check: bool = True) -> dict:
    """Commit a match that has no `src/` home, under `src/recovered/`.

    Choosing a file, a position and an include set in `src/` is still not this
    tool's decision - that is why this is a separate directory rather than a
    guess at one. What it stops being is a reason to throw the body away.
    """
    replacement = code.strip()
    if not replacement:
        raise Refused("matched code is empty")

    import recovery_symbols  # noqa: E402 - only needed for the header line

    path = matched_path(address)
    existed = path.read_text() if path.is_file() else None
    MATCHED_DIR.mkdir(parents=True, exist_ok=True)
    name = row.get("name", "")
    path.write_text(MATCHED_HEADER.format(
        address=address, name=name,
        symbol=recovery_symbols.symbol_for(name, address)) + "\n"
        + replacement + "\n")

    relative = str(path.relative_to(REPO_ROOT))
    result = {"address": f"0x{address:08X}", "name": name,
              "source_location": relative, "lines_replaced": 0,
              "lines_written": len(replacement.splitlines()), "line_delta": 0,
              "files_modified": [relative], "verified": False}
    if not check:
        return result

    verdict = verify(address, read_matched_body(path))
    if verdict.get("tier") != "BYTE_EXACT":
        # Same rule as the splice: a store holding a body that does not verify
        # is worse than an empty one, because the ledger says it matched.
        if existed is None:
            path.unlink()
        else:
            path.write_text(existed)
        raise Refused(
            f"body in {relative} verifies as {verdict.get('tier')} rather than "
            f"BYTE_EXACT "
            f"({verdict.get('note') or verdict.get('refusal_reason') or ''}"
            f" first divergence #{verdict.get('first_divergence', '?')}); "
            f"reverted, nothing changed")
    result["verified"] = True
    return result


def splice(address: int, row: dict, location: str, code: str,
           check: bool = True) -> dict:
    path, lines, start, end = census.body_span(location)
    source_file = location.rpartition(":")[0]
    offset = split_definition(lines[start:end + 1])
    replacement = code.strip().splitlines()
    if not replacement:
        raise Refused("matched code is empty")

    before = lines[:start + offset]
    after = lines[end + 1:]
    original_text = path.read_text()
    path.write_text("\n".join(before + replacement + after) + "\n")

    delta = len(replacement) - (end + 1 - start - offset)
    touched = [source_file]
    if shift_csv(FUNCTIONS_CSV, "source_locations", source_file, end + 1, delta):
        touched.append(str(FUNCTIONS_CSV.relative_to(REPO_ROOT)))
    if shift_csv(LEDGER_CSV, "source_location", source_file, end + 1, delta):
        touched.append(str(LEDGER_CSV.relative_to(REPO_ROOT)))
    if shift_source_map(SOURCE_MAP, source_file, end + 1, delta):
        touched.append(str(SOURCE_MAP.relative_to(REPO_ROOT)))

    result = {"address": f"0x{address:08X}", "name": row.get("name", ""),
              "source_location": location, "lines_replaced": end + 1 - start - offset,
              "lines_written": len(replacement), "line_delta": delta,
              "files_modified": touched, "verified": False}
    if not check:
        return result

    verdict = verify(address, census.extract_body(location))
    if verdict.get("tier") != "BYTE_EXACT":
        # Restore everything. A half-applied writeback is worse than none: the
        # body would read as recovered while the pointers around it moved.
        path.write_text(original_text)
        shift_csv(FUNCTIONS_CSV, "source_locations", source_file, end + 1, -delta)
        shift_csv(LEDGER_CSV, "source_location", source_file, end + 1, -delta)
        shift_source_map(SOURCE_MAP, source_file, end + 1, -delta)
        raise Refused(
            f"body in src/ verifies as {verdict.get('tier')} rather than "
            f"BYTE_EXACT after the splice "
            f"({verdict.get('note') or verdict.get('refusal_reason') or ''}"
            f" first divergence #{verdict.get('first_divergence', '?')}); "
            f"reverted, nothing changed")
    result["verified"] = True
    return result


def writeback(target: str, code: str, check: bool = True) -> dict:
    """Land a match wherever it belongs: the `src/` span, or the store."""
    functions = emit.load_functions()
    address = resolve_address(target, functions)
    row = functions.get(address)
    if row is None:
        raise Refused(f"0x{address:08X} is not a catalogued function")
    location = (row.get("source_locations") or "").split(";")[0].strip()
    if location.startswith("src/"):
        return splice(address, row, location, code, check)
    return land(address, row, code, check)


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("target", help="hex address or mangled function name")
    parser.add_argument("code", help="file holding the matched body; - for stdin")
    parser.add_argument("--no-verify", action="store_true",
                        help="skip the BYTE_EXACT re-check (needs no Wine/VC6)")
    parser.add_argument("--json", action="store_true", help="machine-readable result")
    arguments = parser.parse_args()

    code = sys.stdin.read() if arguments.code == "-" else Path(arguments.code).read_text()
    check = not arguments.no_verify
    if check:
        reason = byte_match.available()
        if reason:
            print(f"SKIP: {reason}. Pass --no-verify to splice without the "
                  f"BYTE_EXACT re-check.", file=sys.stderr)
            return 3

    try:
        result = writeback(arguments.target, code, check=check)
    except Refused as error:
        if arguments.json:
            print(json.dumps({"status": "refused", "reason": str(error)}))
        else:
            print(f"refused: {error}", file=sys.stderr)
        return 1

    if arguments.json:
        print(json.dumps({"status": "written", **result}))
    else:
        print(f"wrote {result['name']} into {result['source_location']} "
              f"({result['lines_replaced']} -> {result['lines_written']} lines, "
              f"{'verified BYTE_EXACT' if result['verified'] else 'UNVERIFIED'})")
        for name in result["files_modified"]:
            print(f"  modified {name}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
