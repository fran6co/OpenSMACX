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

WHAT IT REPLACES, AND WHAT IT KEEPS. A row's `source_locations` points at the
`ORIGINAL:`/`Original Offset:` marker line, which sits inside the doc comment
that precedes the definition, so the catalogued span covers BOTH the comment
and the body. This tool replaces only the definition and keeps the comment
verbatim, because the comment is not decoration: the
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
location BELOW it in the same file. Two committed catalogues carry those
pointers as text and are updated here: `.opensmacx/byte-match.csv` and
`mizuchi/source-map.json`. The FUNCTION CATALOGUE is no longer among them.
`docs/recovery/functions.csv` was deleted (185dd977) and the catalogue is now
read out of the `src/` annotations themselves by `project_catalogue.from_source`
- a row's `source_locations` IS the line the `ORIGINAL:` marker currently sits
on, re-derived on every load, so an edit that moves a marker has already moved
the catalogue with it and there is nothing to patch. Measured today: 6,000 rows
from `src/`, 4,112 of them with a `src/` location. Two more catalogues are
derived and are NOT patched - `docs/recovery/summary.json` and the metadata
caches - because they are regenerated from the IDB by
`tools/verify_recovery_metadata.py`, and hand-editing a generated catalogue is
exactly what that tool is built to catch. After a batch of writebacks, run it.

Usage:
    tools/writeback.py 0x00601B80 path/to/matched.cpp
    tools/writeback.py '?set_loc@BasePop@@QAEXHH@Z' matched.cpp --json
"""

from __future__ import annotations

import argparse
import csv
import io
import json
import os
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import byte_match  # noqa: E402
import byte_match_census as census  # noqa: E402
import emit_translation_unit as emit  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
LEDGER_CSV = REPO_ROOT / ".opensmacx" / "byte-match.csv"
SOURCE_MAP = REPO_ROOT / "mizuchi" / "source-map.json"
MATCHED_DIR = REPO_ROOT / "src" / "recovered"

MATCHED_HEADER = """\
// ORIGINAL: 0x{address:08X}
// 0x{address:08X}  {name}  ->  {symbol}
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. The ORIGINAL marker makes this file part of the source
// map (docs/DECOMP_MAP.md); tools/decomp_status.py re-measures it.
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


def leading_comment(lines: list) -> int:
    """How many of `lines` form a leading `/* ... */` block, or 0.

    Only a block that STARTS the text counts, and only before the first `{`:
    a `/*` further down belongs to the body.
    """
    if not lines or not lines[0].lstrip().startswith("/*"):
        return 0
    for index, line in enumerate(lines):
        if "*/" in line:
            return index + 1
        if "{" in line:
            return 0
    return 0


def comment_start(lines: list, span_start: int) -> int:
    """Index of the `/*` opening the doc comment the span begins inside.

    A catalogued location names a line WITHIN the comment - the `ORIGINAL:` or
    `Original Offset:` marker the annotation scanner keys on - so the opening
    `/*` sits ABOVE the span and is not part of the region the splice
    replaces. That was harmless while the old comment was kept. It is not
    harmless once a submitted comment replaces it:
    the orphaned `/*` stays behind and the file gets `/*` ... `/*` with no
    close between them, which is a comment swallowing the next function.

    Returns `span_start` unchanged when no open comment is found above it.
    """
    index = span_start
    while index >= 0:
        line = lines[index].strip()
        if index != span_start and "*/" in line:
            return span_start          # the span is not inside a comment
        if line.startswith("/*"):
            return index
        index -= 1
    return span_start


def merge_doc_comment(existing: list, replacement: list) -> tuple:
    """(comment lines to keep, definition lines to write).

    The catalogued span opens inside the doc comment, and this tool has always
    replaced only the DEFINITION and kept the comment verbatim, because the
    comment carries `Purpose:` and `Original Offset:` that the code does not.

    That was right while bodies were submitted bare. They are not any more:
    the brief shows an agent the committed body INCLUDING its comment and asks
    for the complete definition back, so the submission carries a comment of
    its own and the file ends up with both. 22 duplicated headers landed in
    product source in one day - which is precisely the readability this loop
    is supposed to be protecting.

    So a submitted comment REPLACES the existing one. It is written by whoever
    just read the disassembly, and the brief now asks them to say what the
    function does. The one thing that is not theirs to drop is a `Purpose:`
    line: if the old comment had one and the new one does not, it is carried
    over rather than lost.
    """
    taken = leading_comment(replacement)
    if not taken:
        return existing, replacement
    incoming, body = replacement[:taken], replacement[taken:]
    purpose = [line for line in existing if line.lstrip().startswith("Purpose:")]
    if purpose and not any(l.lstrip().startswith("Purpose:") for l in incoming):
        incoming = incoming[:1] + purpose + incoming[1:]
    return incoming, body


def write_atomically(path: Path, text: str) -> None:
    """Replace a file's contents in one step, from a reader's point of view.

    The WRITER is serialised - the integrator holds a process-wide lock - but
    the READERS are not. `getContextScript` runs `mizuchi_context.py` as every
    prompt starts, and that reads `functions.csv` and the `src/` headers. A
    plain `write_text` truncates and then fills, so a reader landing in that
    window sees a half-written file and the prompt is refused for a reason
    that has nothing to do with it. `os.replace` is atomic on one filesystem,
    so a reader sees either the old file or the new one.
    """
    temporary = path.with_name(f"{path.name}.{os.getpid()}.tmp")
    temporary.write_text(text)
    os.replace(temporary, path)


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
        buffer = io.StringIO()
        writer = csv.DictWriter(buffer, fieldnames=fieldnames, lineterminator="\r\n")
        writer.writeheader()
        writer.writerows(rows)
        write_atomically(path, buffer.getvalue())
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
        write_atomically(path, json.dumps(entries, indent=2) + "\n")
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
    from declfix import fix_declarations

    scaffolding = emit.emit(address, functions, derived, callees, pe_fast,
                            scaffolding_only=True)
    callee_rows = [functions[target] for target in callees.get(address, [])
                   if target in functions]
    # A GLOBAL THE BODY DECLARES ITSELF IS NOT THE SCAFFOLD'S TO DECLARE.
    # See `emit.without_globals_the_body_declares`: the body's spelling is the
    # one that was measured, and two spellings of one name is C2373.
    scaffolding = emit.without_globals_the_body_declares(scaffolding, body)
    # A CLASS THE BODY DEFINES IS NOT THE SCAFFOLD'S TO DEFINE either, and for
    # the same reason: two declarations of one name is C2011 and kills the
    # unit. See `emit.without_classes_the_body_defines`.
    scaffolding = emit.without_classes_the_body_defines(scaffolding, body)
    return fix_declarations(scaffolding, callee_rows) + "\n" + body


def verify(address: int, body: str) -> dict:
    """Rebuild the unit around a body READ BACK FROM DISK and re-measure it.

    The unit is assembled the way `mizuchi_context.py` assembles the one the
    integrator gated - scaffolding, then `declfix`, then the body -
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
    write_atomically(path, MATCHED_HEADER.format(
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
            write_atomically(path, existed)
        raise Refused(
            f"body in {relative} verifies as {verdict.get('tier')} rather than "
            f"BYTE_EXACT "
            f"({verdict.get('note') or verdict.get('refusal_reason') or ''}"
            f" first divergence #{verdict.get('first_divergence', '?')}); "
            f"reverted, nothing changed")
    result["verified"] = True
    return result


def splice(address: int, row: dict, location: str, code: str,
           check: bool = True, improve: bool = False) -> dict:
    path, lines, start, end = census.body_span(location)
    source_file = location.rpartition(":")[0]
    offset = split_definition(lines[start:end + 1])
    replacement = code.strip().splitlines()
    if not replacement:
        raise Refused("matched code is empty")

    # What is there now, measured BEFORE anything moves. Only needed to judge
    # an improvement, and it costs a compile, so it is not paid otherwise.
    incumbent = verify(address, census.extract_body(location)) if (
        check and improve) else None

    # A submitted comment replaces the whole doc comment, which begins ABOVE
    # the catalogued span - so the region being rewritten starts there, not at
    # `start`. Anchoring at `start` leaves the opening `/*` orphaned above a
    # second `/*`, and the resulting unterminated comment eats the function
    # after it.
    head = comment_start(lines, start) if leading_comment(replacement) else start
    comment, replacement = merge_doc_comment(
        lines[head:start + offset], replacement)
    if head < start and len(comment) <= start - head:
        # This row's OWN catalogue entry IS a line inside the comment: the
        # `ORIGINAL:`/`Original Offset:` marker `annotation_scan` keys on, now
        # that the catalogue is read out of `src/` rather than out of a CSV.
        # A replacement comment too short to reach that line cannot be carrying
        # the marker, and a body with no marker is not a mislocated row any
        # more - it is not a row at all, so the function silently leaves the
        # 6,000. Keeping the existing comment is still the lesser evil: a
        # duplicate reads badly, a lost annotation reads as lost work, and the
        # mislocation this guard was first written for cost a day.
        head, comment = start, lines[start:start + offset]
        replacement = code.strip().splitlines()
    before = lines[:head] + comment
    after = lines[end + 1:]
    original_text = path.read_text()
    write_atomically(path, "\n".join(before + replacement + after) + "\n")

    # Measured over the WHOLE rewritten region, because both the comment and
    # its starting line can move now. Getting this wrong silently renumbers
    # every `source_location` below it in the file.
    #
    # The FUNCTION catalogue is not shifted, because it is no longer a file:
    # `project_catalogue.from_source` re-derives every `source_locations` from
    # the `ORIGINAL:` marker's current line each time it is loaded, so writing
    # `src/` IS updating it. Only the two stores that keep line numbers as text
    # of their own are patched here.
    delta = (len(comment) + len(replacement)) - (end + 1 - head)
    touched = [source_file]
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
    tier = verdict.get("tier")
    result["tier"] = tier

    # BYTE_EXACT is the only unconditional pass. `--improve` adds a second,
    # narrower one: a body that is STRICTLY BETTER than the one it replaces,
    # by `byte_match._better` - the same ordering the ledger ratchet and
    # `verify_recovered_function.py --against-committed` already use, over
    # tier first and then mnemonic similarity.
    #
    # This exists because refusing everything but BYTE_EXACT threw away real
    # work. Two committed bodies were NO_COMPILE under this very recipe
    # (`BOOL` and `LPSTR` are never forward-declared for a type that appears
    # only on a local), and the one-word respelling that fixes them is not
    # byte-exact and could not land - so the row kept reading as a mismatch
    # for a reason that had nothing to do with its code.
    accepted = tier == "BYTE_EXACT" or (
        improve and incumbent is not None
        and byte_match._better(verdict, incumbent))

    if not accepted:
        # Restore everything. A half-applied writeback is worse than none: the
        # body would read as recovered while the pointers around it moved.
        write_atomically(path, original_text)
        shift_csv(LEDGER_CSV, "source_location", source_file, end + 1, -delta)
        shift_source_map(SOURCE_MAP, source_file, end + 1, -delta)
        against = (f" and no better than the committed "
                   f"{incumbent.get('tier')}" if incumbent is not None else "")
        raise Refused(
            f"body in src/ verifies as {tier} rather than BYTE_EXACT"
            f"{against} after the splice "
            f"({verdict.get('note') or verdict.get('refusal_reason') or ''}"
            f" first divergence #{verdict.get('first_divergence', '?')}); "
            f"reverted, nothing changed")
    result["verified"] = tier == "BYTE_EXACT"
    if incumbent is not None:
        result["improved_from"] = incumbent.get("tier")
    return result


def writeback(target: str, code: str, check: bool = True,
              improve: bool = False) -> dict:
    """Land a match wherever it belongs: the `src/` span, or the store.

    A `src/` span is preferred and is NOT the only place a match may go. The
    emitted unit declares the subject's class as an opaque shell with no
    members, so an agent that needs a field reaches it by offset - and the
    ones that reach several declare a shadow struct next to the function:

        struct SS_Impl { int pad0; int pad1; int head; SS_Node *current; };
        int StringStruct::current_id() {
            SS_Impl *p = reinterpret_cast<SS_Impl *>(this);
            ...

    `splice` replaces the DEFINITION inside `src/`, so that struct does not
    come with it and the spliced file stops compiling. Measured on the first
    run over the real corpus: 4 of 10 writebacks, and every one of them a
    body that had already been verified BYTE_EXACT on its own.

    Discarding a proven match because it cannot be phrased in `src/`'s own
    style is the wrong trade. The store exists for exactly this - a body with
    nowhere in `src/` to go - so a splice that will not verify falls back to
    it, and the reason is carried along rather than dropped.
    """
    functions = emit.load_functions()
    address = resolve_address(target, functions)
    row = functions.get(address)
    if row is None:
        raise Refused(f"0x{address:08X} is not a catalogued function")
    location = (row.get("source_locations") or "").split(";")[0].strip()
    if not location.startswith("src/"):
        return land(address, row, code, check)
    try:
        return splice(address, row, location, code, check, improve)
    except Refused as refusal:
        landed = land(address, row, code, check)
        landed["spliced"] = False
        landed["splice_refusal"] = str(refusal)
        return landed


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("target", help="hex address or mangled function name")
    parser.add_argument("code", help="file holding the matched body; - for stdin")
    parser.add_argument("--no-verify", action="store_true",
                        help="skip the BYTE_EXACT re-check (needs no Wine/VC6)")
    parser.add_argument(
        "--improve", action="store_true",
        help="also accept a body that is not byte-exact but is STRICTLY "
             "BETTER than the one it replaces, by the same ordering the "
             "ledger ratchet uses. Refuses a tie.")
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
        result = writeback(arguments.target, code, check=check,
                           improve=arguments.improve)
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
        # A STORED body is invisible to the ledger until `--collect` runs.
        # `byte_match_census.py` scores rows through their `source_locations`,
        # and a stored row deliberately has none, so the census measures it
        # against an empty scaffold and the ratchet never moves. That is easy
        # to mistake for the recovery not counting: two full censuses were
        # spent here before anyone noticed they structurally could not see
        # this work.
        if result["source_location"].startswith("src/recovered/"):
            print("  NOTE: stored bodies are invisible to the census. Run "
                  "`tools/byte_match_fanout.py --collect`\n"
                  "        to re-verify them and move the ledger.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
