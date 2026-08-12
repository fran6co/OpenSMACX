#!/usr/bin/env python3
"""The state of the decompilation, asked of the compiler and read off src/.

One command answers, for every piece of the binary the source tree claims to
map: what does VC6 say the code compiles to, against what the shipped image
holds at that address?

    tools/decomp_status.py                     full map: states, drift, verdicts
    tools/decomp_status.py --state-only        the map alone; no VC6 needed
    tools/decomp_status.py src/stringstruct.cpp
    tools/decomp_status.py --addresses 0x401640,0x402530
    tools/decomp_status.py --placeholders-only
    tools/decomp_status.py --no-cache --verbose
    tools/decomp_status.py --json

HOW IT MEASURES. Implemented annotations become translation units and go
through the SAME machinery the ratchet trusts - no forked extractor, no
second comparator:

  * body mode, census recipe   `byte_match_census.build_unit` VERBATIM: the
                               brace-counted extract, the emitter scaffolding,
                               the seam and compat preambles;
  * body mode, writeback       `mizuchi_writeback.build_unit` - the proved
                               store's recipe, declfix included, exactly as
                               `byte_match_fanout --collect` re-verifies it;
  * FILE mode                  the file text, compiled as-is.

Units then go through `byte_match.match_functions`: batched response-file
compiles under Wine (~2 ms a file), four flag sets, best verdict kept, early
exit on BYTE_EXACT. States PLACEHOLDER and EXCLUDED are never compiled: a
placeholder has nothing to compile, and exclusion is a decision, not a
measurement.

THE LEDGER IS MERGED, NEVER OVERWRITTEN. Verdicts land in
.opensmacx/byte-match.csv keyed by address, rows this run did not measure
preserved verbatim, written atomically. The ratchet
(`tools/byte_match_fanout.py --check`) reads that file and stays where it is;
this tool adds no floor of its own.

CACHING. A content hash of each BUILT unit (after scaffolding, so an emitter
change invalidates correctly) keys a verdict cache in the ignored
`.opensmacx/decomp-status-cache.json`. A warm rerun recompiles only the
pieces whose text changed - seconds, not minutes. `--no-cache` measures
everything from scratch, which is also the way to audit the cache itself.

SKIP SAFETY. Without VC6, Wine, or the pinned executable, measurement prints
the reason and exits 0; the map, drift and placeholder reports still run,
because they need no compiler. Output holds addresses, counts, tiers and
mnemonic NAMES. No opcode byte reaches it.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import os
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import annotation_scan  # noqa: E402
import byte_match  # noqa: E402
import byte_match_census as census  # noqa: E402
import emit_translation_unit as emit  # noqa: E402
import mizuchi_writeback as writeback  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
CACHE_PATH = REPO_ROOT / ".opensmacx" / "decomp-status-cache.json"
UNRECOVERED_DIR = REPO_ROOT / "src" / "unrecovered"
# Bump when the comparison arithmetic changes under a unit text; a verdict
# cached under another version of the comparator is not this tool's verdict.
CACHE_VERSION = "v1"

GENERATED_FILES = census.GENERATED_FILES
FIELDS = census.FIELDS
LEDGER = census.LEDGER

# Exclusion grounds, keyed to docs/EXCLUSIONS.md. Same ranges the coverage
# tool uses - the map follows the exclusions already decided rather than
# re-asking them (and the scanner's grammar carries the citation).
EH_LOW, EH_HIGH = 0x0065_0000, 0x0066_FFFF
PORT_IO_ADDRESS = 0x005D4240


# ------------------------------------------------------------------ loading


def load_cache(no_cache: bool) -> dict:
    if no_cache or not CACHE_PATH.is_file():
        return {}
    try:
        data = json.loads(CACHE_PATH.read_text())
    except (OSError, ValueError):
        return {}
    return data.get("entries", {}) if data.get("version") == CACHE_VERSION \
        else {}


def save_cache(entries: dict) -> None:
    CACHE_PATH.parent.mkdir(parents=True, exist_ok=True)
    tmp = CACHE_PATH.with_suffix(".tmp")
    tmp.write_text(json.dumps({"version": CACHE_VERSION,
                               "entries": entries}, sort_keys=True))
    os.replace(tmp, CACHE_PATH)


def unit_hash(text: str) -> str:
    return hashlib.sha256(text.encode("utf-8", "replace")).hexdigest()


# ----------------------------------------------------------------- building


def build_units(annotations: list, matched: dict, functions: dict,
                derived: dict, callees: dict, pe_fast):
    """{address: unit text} and {address: refusal} for implemented pieces."""
    units, refusals = {}, {}
    for annotation in annotations:
        if annotation.state != annotation_scan.STATE_IMPLEMENTED:
            continue
        address = annotation.address
        row = matched.get(address)
        if row is None:
            continue  # uncatalogued: reported by the cross-reference, not here
        if annotation.recipe == "verbatim":
            units[address] = annotation.region
            continue
        if annotation.recipe == "writeback":
            try:
                units[address] = writeback.build_unit(
                    address, annotation.region, functions, callees, derived,
                    pe_fast)
            except emit.Unsettled as error:
                refusals[address] = f"no scaffolding: {error}"
            continue
        # Census recipe: reuse the census extractor and scaffolder VERBATIM.
        # The marker line plays the role source_locations plays today, so a
        # migrated annotation and the ledger row it came from are measured
        # by the same bytes of code.
        unit, refusal = census.build_unit(
            address, row, annotation.location, functions, derived, callees,
            pe_fast)
        if refusal:
            refusals[address] = refusal
        else:
            units[address] = unit
    return units, refusals


# ---------------------------------------------------------------- measuring


def measure(units: dict, cache: dict, no_cache: bool, jobs: int):
    """Compile + compare every unit; returns ({address: outcome}, new cache).

    `byte_match.match_functions` is the ratchet's own arithmetic, so the
    batch is handed to it whole: layout classification, SHARED_TAIL and
    selfmod refusals, the four flag sets, the best-of rule and the early
    exit all stay where they already live.
    """
    import pefile

    entries = dict(cache)
    hashed = {address: unit_hash(text) for address, text in units.items()}
    pending = {}
    outcomes = {}
    for address, text in units.items():
        cached = entries.get(hashed[address]) if not no_cache else None
        if cached is not None:
            outcomes[address] = cached
        else:
            pending[address] = text

    if pending:
        if byte_match.available():
            raise RuntimeError("VC6 unavailable")  # guarded by the caller
        pe = pefile.PE(str(byte_match.DEFAULT_EXE))
        rows = byte_match.load_rows()
        shared = byte_match.shared_span_index(rows)
        subjects = [(address, pending[address], "") for address in
                    sorted(pending)]
        results = byte_match.match_functions(pe, rows, shared, subjects, "",
                                             chunk=jobs)
        for address, outcome in results.items():
            outcomes[address] = outcome
            entries[hashed[address]] = outcome
    return outcomes, entries


# ------------------------------------------------------------------- ledger


def is_generated(annotation) -> bool:
    return any(annotation.path.endswith(name) for name in GENERATED_FILES)


def span_class_string(outcome: dict) -> str:
    classes = outcome.get("span_classes")
    if not classes:
        return ""
    if isinstance(classes, dict):
        return (f"primary={classes.get('primary', 0)};"
                f"eh={classes.get('eh', 0)};"
                f"shared={classes.get('shared', 0)};"
                f"selfmod={classes.get('selfmod', 0)}")
    return str(classes)


def ledger_row(address: int, annotation, outcome: dict, row: dict) -> dict:
    return {
        "address": f"0x{address:08X}",
        "name": row.get("name", ""),
        "source_location": annotation.location,
        "generated": int(is_generated(annotation)),
        "size": row.get("size", ""),
        "span_classes": span_class_string(outcome),
        "tier": outcome.get("tier", ""),
        "original_bytes": outcome.get("original_bytes", ""),
        "rebuilt_bytes": outcome.get("rebuilt_bytes", ""),
        "original_mnemonics": outcome.get("original_mnemonics", ""),
        "rebuilt_mnemonics": outcome.get("rebuilt_mnemonics", ""),
        "first_divergence": outcome.get("first_divergence", ""),
        "note": outcome.get("note", ""),
        "refusal_reason": outcome.get("refusal_reason", ""),
    }


def merge_ledger(rows_to_write: dict) -> tuple:
    """Address-keyed merge, atomic write, foreign rows preserved verbatim.

    A BYTE_EXACT ROW IS NEVER DOWNGRADED. The ratchet's own rule
    (byte_match_fanout.py): a body that used to match and now does not is
    either a tooling change or a lost scaffolding, and both need a human -
    so the old row is kept and the disagreement is returned for reporting,
    not silently rewritten into the floor.
    """
    existing = {}
    if LEDGER.is_file():
        with LEDGER.open(newline="", encoding="utf-8-sig") as handle:
            existing = {row["address"]: row for row in csv.DictReader(handle)}
    protected = {}
    for address, row in rows_to_write.items():
        incumbent = existing.get(address)
        if incumbent and incumbent.get("tier") == "BYTE_EXACT" \
                and row.get("tier") != "BYTE_EXACT":
            protected[address] = (incumbent, row)
            continue
        existing[address] = row
    tmp = LEDGER.with_suffix(".tmp")
    with tmp.open("w", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=FIELDS)
        writer.writeheader()
        for key in sorted(existing):
            writer.writerow({field: existing[key].get(field, "")
                             for field in FIELDS})
    os.replace(tmp, LEDGER)
    return len(existing), protected


# ------------------------------------------------------------------ drift


def drift_report(annotations: list, matched: dict, functions: dict) -> dict:
    """Where the map and the catalogue disagree. Drift, not failure."""
    by_address = {}
    for annotation in annotations:
        by_address.setdefault(annotation.address, []).append(annotation)

    annotated_not_recovered = []
    for address, annotation in matched.items():
        # A placeholder or exclusion claims nothing about the catalogue's
        # recovery_state, so only implemented pieces can drift against it.
        if annotation.state != annotation_scan.STATE_IMPLEMENTED:
            continue
        state = functions[address].get("recovery_state", "")
        if state in ("unrecovered", "original_dependency"):
            annotated_not_recovered.append((address, annotation, state))

    unannotated_complete = []
    locatable_unlocated = []
    for address, row in functions.items():
        if row.get("recovery_state") != "source_complete":
            continue
        if address in by_address:
            location = (row.get("source_locations") or "").strip()
            if not location:
                locatable_unlocated.append(address)
            continue
        unannotated_complete.append(address)

    stale_locations = []
    for address, annotation in matched.items():
        location = (functions[address].get("source_locations") or "").strip()
        if location and location.split(";")[0].strip() != annotation.location:
            stale_locations.append((address, location, annotation.location))

    duplicates = {address: anns for address, anns in by_address.items()
                  if len(anns) > 1}
    return {
        "annotated_not_recovered": annotated_not_recovered,
        "unannotated_complete": unannotated_complete,
        "locatable_unlocated": locatable_unlocated,
        "stale_locations": stale_locations,
        "duplicates": duplicates,
    }


# ----------------------------------------------------------------- report


def summarise(annotations: list, matched: dict, outcomes: dict,
              refusals: dict, functions: dict) -> None:
    """Byte-weighted state. Counts flatter; bytes do not."""
    def size_of(address: int) -> int:
        return int(functions.get(address, {}).get("size") or 0)

    states: dict = {}
    for annotation in annotations:
        key = annotation.state
        count, total = states.get(key, (0, 0))
        states[key] = (count + 1, total + size_of(annotation.address))

    print("the map (src/ annotations)")
    for key in sorted(states):
        count, total = states[key]
        print(f"  {key:14s} {count:6d} piece(s)  {total:9d} bytes")

    # Two populations, never totalled - the census rule, carried over.
    for label, want in (("hand-written", False), ("generated", True)):
        measured = [(a, outcomes[a.address]) for a in annotations
                    if a.address in outcomes
                    and a.state == annotation_scan.STATE_IMPLEMENTED
                    and is_generated(a) is want]
        if not measured:
            continue
        tiers: dict = {}
        for annotation, outcome in measured:
            tier = outcome.get("tier", "?")
            count, total = tiers.get(tier, (0, 0))
            tiers[tier] = (count + 1, total + size_of(annotation.address))
        exact = tiers.get("BYTE_EXACT", (0, 0))
        print(f"\nmeasured, {label}: {exact[0]}/{len(measured)} BYTE_EXACT "
              f"({exact[1]}/{sum(t for _, t in tiers.values())} bytes)")
        for tier in byte_match.TIER_ORDER:
            if tier in tiers:
                count, total = tiers[tier]
                print(f"  {tier:14s} {count:6d}  {total:9d} bytes")

    if refusals:
        print(f"\nrefused before compile: {len(refusals)}")
        reasons: dict = {}
        for reason in refusals.values():
            key = reason[:70]
            reasons[key] = reasons.get(key, 0) + 1
        for reason, count in sorted(reasons.items(),
                                    key=lambda item: -item[1])[:8]:
            print(f"  x{count:4d}  {reason}")


def print_drift(drift: dict, limit: int = 15) -> None:
    print("\ndrift between the map and the catalogue")
    for address, annotation, state in drift["annotated_not_recovered"][:limit]:
        print(f"  annotated but catalogue says {state}: "
              f"0x{address:08X} at {annotation.location}")
    extra = len(drift["annotated_not_recovered"]) - limit
    if extra > 0:
        print(f"  ... and {extra} more")
    if drift["unannotated_complete"]:
        print(f"  source_complete with no annotation in src/: "
              f"{len(drift['unannotated_complete'])}")
    if drift["locatable_unlocated"]:
        print(f"  unlocated source_complete rows the scan CAN locate: "
              f"{len(drift['locatable_unlocated'])}")
    for address, was, now in drift["stale_locations"][:limit]:
        print(f"  stale source_locations: 0x{address:08X} {was} "
              f"-> scan says {now}")
    extra = len(drift["stale_locations"]) - limit
    if extra > 0:
        print(f"  ... and {extra} more stale locations")
    for address, anns in sorted(drift["duplicates"].items())[:limit]:
        places = ", ".join(a.location for a in anns)
        print(f"  duplicate address 0x{address:08X}: {places}")


def print_pieces(annotations: list, outcomes: dict, verbose: bool,
                 placeholders_only: bool) -> None:
    for annotation in annotations:
        if placeholders_only and \
                annotation.state != annotation_scan.STATE_PLACEHOLDER:
            continue
        outcome = outcomes.get(annotation.address, {})
        tier = outcome.get("tier", "")
        if not verbose and annotation.state == \
                annotation_scan.STATE_IMPLEMENTED and tier == "BYTE_EXACT":
            continue  # the quiet majority; --verbose lists them too
        note = outcome.get("note") or outcome.get("refusal_reason") or ""
        flags = " ".join(filter(None, [
            annotation.mode, annotation.state,
            "deprecated" if annotation.deprecated else "",
            annotation.exclusion]))
        line = (f"0x{annotation.address:08X} {tier:14s} "
                f"{annotation.location:56s} {flags}")
        if note and verbose:
            line += f"  {note[:100]}"
        print(line)


# ---------------------------------------------------------- placeholders


def exclusion_for(address: int, row: dict) -> tuple:
    """(token, prose) when the catalogue row is excluded, else ("", "")."""
    state = row.get("recovery_state", "")
    if state == "external_library":
        return ("S1", "availability - MSVC 6 CRT and Windows library code; "
                "linked, not rewritten (docs/EXCLUSIONS.md section 1)")
    if EH_LOW <= address <= EH_HIGH:
        return ("S2a", "expressibility - C++ EH unwind funclet; borrows the "
                "enclosing function's frame (docs/EXCLUSIONS.md section 2a)")
    if address == PORT_IO_ADDRESS:
        return ("S3", "expressibility - VGA DAC palette loop of out/cli/sti; "
                "no C++ for the body (docs/EXCLUSIONS.md section 3)")
    return ("", "")


def _catalog_facts(address: int, row: dict) -> str:
    """The catalogue's facts as comment lines, so the file speaks for itself."""
    spans = row.get("body_ranges") or \
        f"0x{address:08X}-0x{address + int(row.get('size') or 0):08X}"
    return (f"// name      {row.get('name', '')}\n"
            f"// size      {row.get('size', '')} bytes\n"
            f"// spans     {spans}\n"
            f"// prototype {row.get('prototype', '')}\n"
            f"// callers   {row.get('caller_count', '')}   "
            f"call targets   {row.get('call_target_count', '')}\n")


def placeholder_text(address: int, row: dict) -> str:
    # FILE mode: the whole file is the unit. The sentinel makes the state
    # PLACEHOLDER by measurement; --work replaces the file with scaffolding
    # that carries the same sentinel until a real body lands.
    return (f"// ORIGINAL: 0x{address:08X} FILE\n"
            f"// placeholder - not yet decompiled\n"
            f"{_catalog_facts(address, row)}"
            f"// To start: tools/decomp_status.py --work 0x{address:08X}\n"
            f"\n"
            f"// BODY GOES HERE.\n")


def excluded_text(address: int, row: dict, token: str, prose: str) -> str:
    return (f"// ORIGINAL: 0x{address:08X} EXCLUDED {token}\n"
            f"// excluded - {prose}\n"
            f"{_catalog_facts(address, row)}")


def generate_placeholders(functions: dict, annotated: set, force: bool) -> int:
    """One file per catalogue row nothing in src/ maps yet. 100% of the map."""
    created, skipped_excluded, skipped_state = 0, 0, []
    for address in sorted(set(functions) - annotated):
        row = functions[address]
        if row.get("recovery_state") == "source_complete":
            # The catalogue says implemented but no annotation exists: a
            # placeholder would claim the opposite. These need a human (or
            # tools/repair_source_locations.py), not a generated file.
            skipped_state.append(address)
            continue
        path = UNRECOVERED_DIR / f"{address:08x}.cpp"
        if path.is_file() and not force:
            continue
        token, prose = exclusion_for(address, row)
        text = excluded_text(address, row, token, prose) if token \
            else placeholder_text(address, row)
        if token:
            skipped_excluded += 1
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text)
        created += 1
    print(f"placeholders written: {created} "
          f"({skipped_excluded} of them EXCLUDED with grounds)")
    if skipped_state:
        print(f"NOT written for {len(skipped_state)} source_complete rows "
              f"with no annotation - the catalogue says implemented; they "
              f"need repair_source_locations.py or a human, not a "
              f"placeholder")
        for address in skipped_state[:10]:
            print(f"  0x{address:08X} {functions[address].get('name', '')}")
    return created


MALFORMED_LEVER = re.compile(r"^\s*(?://|\*)?\s*LEVER:\s*(?:\S+\s*)?$")


def lesson_report(annotations: list) -> list:
    """[(location, why)] for every lesson marker that contradicts its state.

    THE TOKENS ARE SELF-REFUTING, WHICH IS WHAT EARNS THEM A PLACE under the
    rule that state is measured and not claimed (docs/DECOMP_MAP.md). A `LEVER`
    sits on a body that matched, so the existing ratchet already fails if it
    stops. A `RULED-OUT` sits on a body that has NOT matched - so if one ever
    does match, the list is refuted and this fails until somebody promotes it to
    a LEVER or deletes it. No wall can quietly outlive its reason.

    Three checks, all offline - no VC6, no image:

      state      RULED-OUT on a BYTE_EXACT piece, or LEVER on one without.
      placement  RULED-OUT on a PLACEHOLDER. You cannot rule a spelling out of
                 a body that does not exist; the agent has to land its best
                 attempt first, which is the point - a MISMATCH run currently
                 leaves nothing behind at all.
      syntax     `LEVER:` with no fingerprint after it. The fingerprint is the
                 key the aggregation groups on, so a LEVER without one is a
                 lesson that can never be filed and reads exactly like one that
                 can.
    """
    faults = []
    for annotation in annotations:
        if annotation.ruled_out and annotation.matched:
            faults.append((annotation.location,
                           f"{len(annotation.ruled_out)} RULED-OUT line(s) on a "
                           f"BYTE_EXACT body - promote one to `LEVER: "
                           f"<fingerprint> ...` or delete them"))
        if annotation.levers and not annotation.matched:
            faults.append((annotation.location,
                           "LEVER on a body carrying no BYTE_EXACT claim - a "
                           "lever names what MADE it match"))
        if annotation.ruled_out and annotation.state == annotation_scan.STATE_PLACEHOLDER:
            faults.append((annotation.location,
                           "RULED-OUT on a placeholder - land the attempt that "
                           "ruled them out, then record it"))
        for line in (annotation.region or "").splitlines():
            if MALFORMED_LEVER.match(line):
                faults.append((annotation.location,
                               "LEVER with no fingerprint: write "
                               "`LEVER: <original>/<rebuilt> <what worked>`"))
                break
    return faults


def work_address(address: int, annotations: list, functions: dict,
                 derived: dict, callees: dict, pe_fast) -> int:
    """Replace a placeholder in place with its emitted scaffold.

    The decompile loop: claim -> edit body -> decomp_status -> BYTE_EXACT.
    The scaffold keeps the FILE marker and the sentinel, so the state stays
    PLACEHOLDER (measured) until a real body exists - this tool does not
    trust the claim that work has started.

    THE SCAFFOLD GOES THROUGH `mizuchi_declfix` BEFORE IT IS WRITTEN, so that
    the unit an agent iterates on and the unit that banks the result are the
    same bytes. They were not. A `FILE`-marked scaffold is compiled VERBATIM by
    `build_units`, while `verify_recovered_function` scores through
    `mizuchi_writeback.build_unit`, which respells callee declarations so VC6
    re-mangles them to the catalogued names. Without declfix a body calling a
    CRT function reads NO_COMPILE in one and BYTE_EXACT in the other - which
    means an agent could prove a body and `--record-matches` would stamp
    nothing, banking zero for the batch and looking like an agent-honesty
    problem rather than a recipe mismatch.

    The definition is split off the full unit by prefix rather than rebuilt:
    `emit` returns the scaffolding at exactly the point the definition begins
    (`emit_translation_unit.py:1430,1441`), so this produces byte-for-byte what
    `build_unit` would, which is the property that matters.
    """
    annotation = next((a for a in annotations if a.address == address), None)
    if annotation is None:
        print(f"no annotation for 0x{address:08X}; nothing to work")
        return 1
    if annotation.state != annotation_scan.STATE_PLACEHOLDER:
        print(f"0x{address:08X} is {annotation.state} at "
              f"{annotation.location}; --work only claims placeholders")
        return 1
    try:
        whole = emit.emit(address, functions, derived, callees, pe_fast)
        scaffolding = emit.emit(address, functions, derived, callees, pe_fast,
                                scaffolding_only=True)
        unit = writeback.build_unit(address, whole[len(scaffolding):].lstrip("\n"),
                                    functions, callees, derived, pe_fast)
    except emit.Unsettled as error:
        print(f"no scaffolding for 0x{address:08X}: {error}")
        return 1
    header = (f"// ORIGINAL: 0x{address:08X} FILE\n"
              f"// working copy - scaffold materialised by --work\n"
              f"{_catalog_facts(address, functions[address])}\n")
    path = REPO_ROOT / annotation.path
    path.write_text(header + unit)
    print(f"materialised scaffold for 0x{address:08X} at {annotation.path}; "
          f"replace the BODY GOES HERE sentinel with the body, then "
          f"tools/decomp_status.py {annotation.path}")
    return 0


# ------------------------------------------------------------------- gaps


def gap_report(functions: dict) -> None:
    """Bytes of .text and _SELFMOD no catalogued span covers.

    The map claims 100% of the mappable binary; this measures the claim.
    What is left is padding, switch tables and funclet edges - named here so
    it is visible rather than assumed away.
    """
    import pefile
    from generator_support import parse_body_ranges

    pe = pefile.PE(str(byte_match.DEFAULT_EXE), fast_load=True)
    sections = {}
    for section in pe.sections:
        name = section.Name.rstrip(b"\x00").decode(errors="replace")
        base = pe.OPTIONAL_HEADER.ImageBase + section.VirtualAddress
        sections[name] = (base, base + section.Misc_VirtualSize)

    spans: list = []
    for address, row in functions.items():
        parsed = parse_body_ranges(row.get("body_ranges") or "")
        if not parsed:
            size = int(row.get("size") or 0)
            parsed = [(address, address + size)] if size else []
        spans.extend(parsed)

    for section_name in (".text", "_SELFMOD"):
        if section_name not in sections:
            continue
        low, high = sections[section_name]
        covered = sorted((max(a, low), min(b, high)) for a, b in spans
                         if a < high and b > low)
        gaps, cursor = [], low
        for start, end in covered:
            if start > cursor:
                gaps.append((cursor, start))
            cursor = max(cursor, end)
        if cursor < high:
            gaps.append((cursor, high))
        total_gap = sum(b - a for a, b in gaps)
        section_size = high - low
        print(f"\n{section_name}: {section_size} bytes, "
              f"{total_gap} in no catalogued span "
              f"({100 * total_gap / max(section_size, 1):.2f}%), "
              f"{len(gaps)} gap(s)")
        for start, end in sorted(gaps, key=lambda g: g[0] - g[1])[:12]:
            print(f"  0x{start:08X}-0x{end:08X}  {end - start:6d} bytes")


# ------------------------------------------------------------- migration


def _format_marker(address: int, keyword: str = "") -> str:
    suffix = f" {keyword}" if keyword else ""
    return f"ORIGINAL: 0x{address:08X}{suffix}"


def plan_file_migration(path: Path, text: str, store: str) -> str:
    """The migrated text for one file. Comment-only by construction.

    Block markers are rewritten in place; the two inline forms MOVE their
    marker onto its own line above the definition, because the new grammar
    extracts forward from the marker and the census forward count never
    applied to them anyway; store files gain an explicit marker line so the
    adapter that keys on their directory can be retired.
    """
    kind = annotation_scan.store_kind(path)
    if store == "unit" or kind == "units":
        address = int(Path(path).resolve().stem, 16)
        return f"// {_format_marker(address, 'FILE')}\n{text}"
    if kind == "proved":
        address = int(Path(path).resolve().stem, 16)
        return f"// {_format_marker(address)}\n{text}"

    lines = text.splitlines()

    def block_replacement(match):
        address = int(match.group("addr"), 16)
        return _format_marker(address)

    lines = [annotation_scan.LEGACY_BLOCK.sub(block_replacement, line)
             for line in lines]

    # Opening-brace form: strip the address comment off the brace line and
    # insert the marker above it.
    insertions: list = []
    for index, line in enumerate(lines):
        match = annotation_scan.LEGACY_OPENING.search(line)
        if match:
            address = int(match.group("addr"), 16)
            lines[index] = annotation_scan.LEGACY_OPENING.sub("{", line)
            insertions.append((index, f"// {_format_marker(address)}"))

    # Trailing form: strip the address comment off the closing line and
    # insert the marker above the body's opening line. Bottom-up within a
    # pass so earlier indices stay valid; collected first, then applied.
    trailing: list = []
    for index, line in enumerate(lines):
        match = annotation_scan.LEGACY_TRAILING.search(line)
        if match:
            address = int(match.group("addr"), 16)
            start = annotation_scan.backward_start(lines, index)
            trailing.append((start, f"// {_format_marker(address)}"))
            lines[index] = annotation_scan.LEGACY_TRAILING.sub("}", line)

    for position, marker in sorted(insertions + trailing, reverse=True):
        lines.insert(position, marker)
    return "\n".join(lines) + ("\n" if text.endswith("\n") else "")


def plan_migration() -> list:
    """(path, old text, new text) for every file holding deprecated forms."""
    plan = []
    seen: set = set()
    for annotation in annotation_scan.scan_tree():
        if not annotation.deprecated or annotation.path in seen:
            continue
        seen.add(annotation.path)
        path = REPO_ROOT / annotation.path
        text = path.read_text()
        store = "unit" if annotation.mode == annotation_scan.MODE_FILE else ""
        plan.append((path, text,
                     plan_file_migration(path, text, store)))
    return plan


def _code_identity(old: str, new: str) -> bool:
    return annotation_scan._code_only(old) == annotation_scan._code_only(new)


def check_migration(plan: list) -> bool:
    """The guards every rewrite must pass before a file is touched.

    G0: the rewrite is comment-only - the code content is identical, so the
        compiler's answer cannot change;
    G1: the address set is identical per file - nothing lost, nothing added.
    A rewrite is a change of SPELLING; either guard failing means it is not.
    """
    ok = True
    for path, old, new in plan:
        if not _code_identity(old, new):
            print(f"  GUARD FAIL (code changed): {path}")
            ok = False
            continue
        before = {(a.address, a.mode) for a in
                  annotation_scan.resolve(annotation_scan.scan_text(
                      old, path))[0]}
        after = {(a.address, a.mode) for a in
                 annotation_scan.resolve(annotation_scan.scan_text(
                     new, path))[0]}
        if before != after:
            print(f"  GUARD FAIL (address set): {path}\n"
                  f"    lost {sorted(before - after)}\n"
                  f"    gained {sorted(after - before)}")
            ok = False
    return ok


def rewrite_source_locations(catalog: dict) -> int:
    """Invert the truth direction: locations derive from the scan.

    But the catalogue column is not the map - it is the SCORING ROUTE. A row
    with a location belongs to the census; a body under src/recovered/ stays
    scoreable only while its row is unowned (collect-ownership pins exactly
    this), and a placeholder is a promise, not an implementation, so it must
    not make its row owned either. Product source owned by an IMPLEMENTED
    annotation is the only thing that may occupy the column; everything else
    is cleared, and the map (the annotations) stays complete regardless.
    """
    annotations, _ = annotation_scan.resolve(annotation_scan.scan_tree())
    locations = {}
    for annotation in annotations:
        if annotation.state != annotation_scan.STATE_IMPLEMENTED:
            continue
        if "src/recovered/" in annotation.path:
            continue
        locations[annotation.address] = annotation.location
    path = byte_match.FUNCTIONS_CSV
    with path.open(newline="") as handle:
        rows = list(csv.DictReader(handle))
        fieldnames = list(rows[0].keys()) if rows else []
    changed = 0
    for row in rows:
        address = int(row["address"], 16)
        want = locations.get(address, "")
        if row.get("source_locations", "") != want:
            row["source_locations"] = want
            changed += 1
    tmp = path.with_suffix(".tmp")
    with tmp.open("w", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(rows)
    os.replace(tmp, path)
    return changed


def run_migration(apply: bool, rewrite_locations: bool) -> int:
    plan = plan_migration()
    print(f"migration plan: {len(plan)} file(s) carry deprecated spellings")
    if not plan:
        if apply and rewrite_locations:
            changed = rewrite_source_locations(emit.load_functions())
            print(f"functions.csv source_locations rewritten from the scan: "
                  f"{changed} row(s); run promote -> classify -> promote, "
                  f"then tools/run_gate.py")
        return 0
    if not check_migration(plan):
        print("guards FAILED; nothing written")
        return 1
    print("guards pass: every rewrite is comment-only and preserves the "
          "address set")
    if not apply:
        shown = 0
        for path, old, new in plan:
            if shown >= 5:
                break
            old_lines = old.splitlines()
            new_lines = new.splitlines()
            changed = [line for line in new_lines if line not in old_lines]
            if not changed:
                continue
            shown += 1
            print(f"\n--- {path}")
            for line in changed[:4]:
                print(f"    {line}")
        if len(plan) > shown:
            print(f"\n... {len(plan) - shown} more file(s)")
        print("\ndry run only; --migrate --apply writes the rewrites")
        return 0
    for path, _old, new in plan:
        tmp = path.with_suffix(".tmp")
        tmp.write_text(new)
        os.replace(tmp, path)
    print(f"wrote {len(plan)} migrated file(s)")
    if rewrite_locations:
        changed = rewrite_source_locations(emit.load_functions())
        print(f"functions.csv source_locations rewritten from the scan: "
              f"{changed} row(s); run promote -> classify -> promote, then "
              f"tools/run_gate.py")
    return 0


# ------------------------------------------------------------------- main


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("paths", nargs="*", type=Path,
                        help="files to scan (default: all of src/)")
    parser.add_argument("--addresses", default="",
                        help="comma-separated hex addresses to restrict to")
    parser.add_argument("--state-only", action="store_true",
                        help="the map and drift only; do not compile")
    parser.add_argument("--placeholders-only", action="store_true",
                        help="list placeholder pieces only")
    parser.add_argument("--verbose", action="store_true",
                        help="list every piece, BYTE_EXACT included")
    parser.add_argument("--no-cache", action="store_true",
                        help="measure from scratch, ignoring the cache")
    parser.add_argument("--no-ledger", action="store_true",
                        help="do not write the .opensmacx/byte-match.csv cache")
    parser.add_argument("--jobs", type=int, default=120,
                        help="units per CL response-file batch")
    parser.add_argument("--json", action="store_true",
                        help="machine-readable result instead of prose")
    parser.add_argument("--generate-placeholders", action="store_true",
                        help="write src/unrecovered/<addr>.cpp for every "
                             "catalogued row nothing in src/ maps yet")
    parser.add_argument("--force", action="store_true",
                        help="overwrite existing placeholder files")
    parser.add_argument("--work", default="",
                        help="materialise the emitted scaffold over the "
                             "placeholder at this address")
    parser.add_argument("--gaps", action="store_true",
                        help="report image bytes in no catalogued span")
    parser.add_argument("--migrate", action="store_true",
                        help="rewrite deprecated spellings to the ORIGINAL "
                             "grammar (dry run unless --apply)")
    parser.add_argument("--apply", action="store_true",
                        help="with --migrate: actually write the rewrites")
    parser.add_argument("--src", type=Path, default=None,
                        help="the source tree to scan (default: src/). Named "
                             "so verify_checks_can_fail.py can point --check "
                             "at a deliberately damaged copy: a ratchet whose "
                             "failure nobody has seen is not a ratchet.")
    parser.add_argument("--check", action="store_true",
                        help="fail when a BYTE_EXACT claim in src/ no longer "
                             "reproduces; this is the ratchet")
    parser.add_argument("--record-matches", action="store_true",
                        help="write BYTE_EXACT onto every annotation this run "
                             "proved (adds only, never removes)")
    parser.add_argument("--rewrite-locations", action="store_true",
                        help="with --migrate --apply: rewrite functions.csv "
                             "source_locations from the scan")
    arguments = parser.parse_args(argv)

    if arguments.paths:
        annotations = []
        for path in arguments.paths:
            annotations.extend(annotation_scan.scan_file(path))
    elif arguments.src is not None:
        annotations = annotation_scan.scan_tree(arguments.src)
    else:
        annotations = annotation_scan.scan_tree()

    wanted = set()
    if arguments.addresses:
        wanted = {int(token, 16) for token in
                  arguments.addresses.replace(" ", "").split(",") if token}
        annotations = [a for a in annotations if a.address in wanted]

    functions = emit.load_functions()

    if arguments.migrate:
        return run_migration(arguments.apply, arguments.rewrite_locations)

    # Map-changing actions run against the FULL tree regardless of any path
    # filter: a partial scan would undercount what is already annotated and
    # generate placeholders over owned addresses.
    if arguments.generate_placeholders or arguments.work or arguments.gaps:
        full_scan, _ = annotation_scan.resolve(annotation_scan.scan_tree())
        if arguments.generate_placeholders:
            generate_placeholders(
                functions, {a.address for a in full_scan}, arguments.force)
            return 0
        if arguments.work:
            import pefile
            address = int(arguments.work, 16)
            pe_fast = pefile.PE(str(byte_match.DEFAULT_EXE), fast_load=True)
            return work_address(address, full_scan, functions,
                                emit.load_derived(), emit.load_callees(),
                                pe_fast)
        gap_report(functions)
        return 0

    annotations, conflicts = annotation_scan.resolve(annotations)
    cross = annotation_scan.cross_reference(annotations, functions)
    drift = drift_report(annotations, cross.matched, functions)
    drift["duplicates"].update(conflicts)

    # OFFLINE, AND BEFORE EVERY EARLY RETURN. These three need no compiler and
    # no image, so a checkout without VC6 - which takes the SKIP path below and
    # exits 0 - must still enforce them. Put after the skip, the grammar would
    # be checked only on machines that could already measure everything.
    lesson_faults = lesson_report(annotations)
    if lesson_faults:
        print(f"\nLESSON GRAMMAR: {len(lesson_faults)} annotation(s) carry a "
              f"LEVER or RULED-OUT that contradicts their state.",
              file=sys.stderr)
        for location, why in lesson_faults:
            print(f"  {location}: {why}", file=sys.stderr)
        if arguments.check:
            return 1

    if arguments.state_only:
        if arguments.json:
            print(json.dumps({
                "states": _state_counts(annotations, functions),
                "catalog_only": len(cross.catalog_only),
                "uncatalogued": [a.address for a in cross.uncatalogued],
            }, indent=2))
        else:
            summarise(annotations, cross.matched, {}, {}, functions)
            print(f"\ncatalogue rows with no annotation anywhere: "
                  f"{len(cross.catalog_only)}")
            print_drift(drift)
        return 0

    reason = byte_match.available()
    if reason:
        print(f"SKIP: {reason}. This needs Visual C++ 6.0 (12.00.8168) under "
              f"Wine and the pinned executable; set VC6_ROOT to relocate it. "
              f"Reporting the map only.")
        summarise(annotations, cross.matched, {}, {}, functions)
        print(f"\ncatalogue rows with no annotation anywhere: "
              f"{len(cross.catalog_only)}")
        print_drift(drift)
        return 0

    import pefile
    pe_fast = pefile.PE(str(byte_match.DEFAULT_EXE), fast_load=True)
    derived = emit.load_derived()
    callees = emit.load_callees()

    duplicates = set(drift["duplicates"])
    measurable = [a for a in annotations if a.address not in duplicates]
    units, refusals = build_units(measurable, cross.matched, functions,
                                  derived, callees, pe_fast)

    cache = load_cache(arguments.no_cache)
    outcomes, entries = measure(units, cache, arguments.no_cache,
                                arguments.jobs)
    save_cache(entries)

    for address, reason_text in refusals.items():
        outcomes[address] = {"tier": "REFUSED",
                             "refusal_reason": reason_text}

    rows_to_write = {}
    for address, outcome in outcomes.items():
        annotation = cross.matched.get(address)
        if annotation is None or address not in functions:
            continue
        rows_to_write[f"0x{address:08X}"] = ledger_row(
            address, annotation, outcome, functions[address])
    ledger_total, protected = None, {}
    if rows_to_write and not arguments.no_ledger:
        ledger_total, protected = merge_ledger(rows_to_write)

    if arguments.json:
        printable = {
            "outcomes": {
                f"0x{address:08X}": {
                    key: value for key, value in outcome.items()
                    if key in ("tier", "original_bytes", "rebuilt_bytes",
                               "first_divergence", "note", "refusal_reason")}
                for address, outcome in sorted(outcomes.items())},
            "states": _state_counts(annotations, functions),
            "refusals": {f"0x{address:08X}": text
                         for address, text in sorted(refusals.items())},
            "cache_hits": len(units) - _miss_count(units, cache),
        }
        print(json.dumps(printable, indent=2))
        return 0

    summarise(annotations, cross.matched, outcomes, refusals, functions)
    print(f"\ncatalogue rows with no annotation anywhere: "
          f"{len(cross.catalog_only)}")
    print_pieces(measurable, outcomes, arguments.verbose,
                 arguments.placeholders_only)
    print_drift(drift)
    if protected:
        print(f"\nUNREPRODUCED BYTE_EXACT rows kept, not downgraded: "
              f"{len(protected)} (needs a human, not a rewrite)")
        for address in sorted(protected):
            incumbent, fresh = protected[address]
            print(f"  {address}: ledger BYTE_EXACT "
                  f"({incumbent.get('source_location') or 'no location'}) "
                  f"vs now {fresh.get('tier')} at "
                  f"{fresh.get('source_location')}")
    if ledger_total is not None:
        print(f"\nwrote {LEDGER} ({ledger_total} rows, "
              f"{len(rows_to_write)} measured by this run)")

    claimed = [a for a in measurable if a.matched]
    lost = claim_regressions(measurable, outcomes)
    if arguments.record_matches:
        added = record_claims(measurable, outcomes)
        print(f"\nrecorded {added} new BYTE_EXACT claim(s) in src/; "
              f"{len(claimed) + added} carried in total")
    else:
        print(f"\nBYTE_EXACT claims in src/: {len(claimed)} "
              f"({len(lost)} not reproduced)")
    if lost:
        print("\nRATCHET: a BYTE_EXACT claim in src/ no longer reproduces.",
              file=sys.stderr)
        for annotation, tier in sorted(lost, key=lambda e: e[0].address):
            print(f"  {annotation.address_hex} {annotation.location} "
                  f"claims BYTE_EXACT, measured {tier}", file=sys.stderr)
        if arguments.check:
            return 1
    return 0


# ------------------------------------------------------------------ ratchet


def claim_regressions(measurable: list, outcomes: dict) -> list:
    """[(annotation, tier)] for BYTE_EXACT claims this run did not reproduce.

    THE CLAIM IS THE RATCHET, and it lives beside the body rather than in a
    committed CSV and a pair of constants. `// ORIGINAL: 0x... BYTE_EXACT`
    says "this was proved to recompile to the shipped bytes"; if it stops, the
    check fails and names the address. Deleting the body deletes the claim,
    which is the property a separate ledger could never have - one row claimed
    BYTE_EXACT for a body that had been reset to `// BODY GOES HERE.` and the
    floor counted it for months.
    """
    out = []
    for annotation in measurable:
        if not annotation.matched:
            continue
        tier = (outcomes.get(annotation.address) or {}).get("tier")
        if tier != "BYTE_EXACT":
            out.append((annotation, tier or "not measured"))
    return out


def record_claims(measurable: list, outcomes: dict) -> int:
    """Write `BYTE_EXACT` onto every annotation this run proved. Returns the count.

    Only ADDS. A claim is never removed here: losing one is a regression and
    has to be a deliberate source edit, visible in the diff.
    """
    by_path: dict = {}
    for annotation in measurable:
        if annotation.matched or not annotation.line:
            continue
        if (outcomes.get(annotation.address) or {}).get("tier") != "BYTE_EXACT":
            continue
        by_path.setdefault(annotation.path, []).append(annotation)
    written = 0
    for path, entries in by_path.items():
        target = REPO_ROOT / path
        lines = target.read_text().splitlines(keepends=True)
        for annotation in entries:
            index = annotation.line - 1
            if index >= len(lines):
                continue
            hit = annotation_scan.MARKER.search(lines[index])
            if not hit or annotation_scan.MARKER_MATCHED.search(lines[index]):
                continue
            cut = hit.end("addr")
            lines[index] = (lines[index][:cut] + " BYTE_EXACT"
                            + lines[index][cut:])
            written += 1
        target.write_text("".join(lines))
    return written


def _state_counts(annotations: list, functions: dict) -> dict:
    states: dict = {}
    for annotation in annotations:
        entry = states.setdefault(annotation.state,
                                  {"count": 0, "bytes": 0})
        entry["count"] += 1
        entry["bytes"] += int(
            functions.get(annotation.address, {}).get("size") or 0)
    return states


def _miss_count(units: dict, cache: dict) -> int:
    return sum(1 for text in units.values()
               if unit_hash(text) not in cache)


if __name__ == "__main__":
    raise SystemExit(main())
