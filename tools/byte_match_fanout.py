#!/usr/bin/env python3
"""Prepare work for the byte-match fan-out, and score what comes back.

Two jobs, deliberately in one tool because they share the ledger and would
otherwise disagree about it:

  --prepare N   choose the next N targets, write a scaffolding unit for each
                into build/byte-match/<address>/unit.cpp
  --collect     RE-VERIFY every unit on disk and update the ledger

THE COLLECT STEP DOES NOT TRUST THE AGENTS. Each fan-out agent reports the tier
it believes it reached, and that report is not evidence: it is a claim about a
run this tool did not observe, produced by the same process that had an
incentive to stop. So `--collect` recompiles every `unit.cpp` itself and writes
the tier IT measured. An agent that reports BYTE_EXACT and whose unit scores
MISMATCH here is recorded as MISMATCH, and the disagreement is printed - a
divergence between claim and measurement is a finding about the harness, not a
rounding error.

THE LEDGER IS MERGED, NEVER OVERWRITTEN. `byte_match_census.py` fills in the
`source_complete` rows and this fills in the unrecovered ones; whichever runs
second must not erase the other's work. Rows are keyed on address and only the
keys a run actually measured are touched.

RATCHET. A row already at BYTE_EXACT is never re-queued and never downgraded by
a later run that failed to reproduce it - it is reported as a REGRESSION
instead, loudly, because a body that used to match and now does not is either a
tooling change or a lost file, and both need a human.

Output holds addresses, counts and tier names. No opcode byte reaches it.
"""

from __future__ import annotations

import argparse
import collections
import csv
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import pefile  # noqa: E402

import byte_match  # noqa: E402
import emit_translation_unit as emit  # noqa: E402
import mizuchi_writeback as writeback  # noqa: E402
from byte_match_census import FIELDS, LEDGER  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
WORK_ROOT = REPO_ROOT / "build" / "byte-match"

TARGET_STATES = ("unrecovered", "original_dependency")
# Only byte equality retires a row. SHAPE_EXACT deliberately means the
# instruction/register shape agrees while at least one non-relocated operand
# value differs; the comparator's controls use a wrong field offset and wrong
# vtable slot to prove that tier fires. Counting it here would turn the first
# localized wrong constant into recovery progress.
MATCHED = ("BYTE_EXACT",)


def read_ledger() -> dict:
    if not LEDGER.is_file():
        return {}
    with LEDGER.open() as handle:
        return {row["address"]: row for row in csv.DictReader(handle)}


def write_ledger(rows: dict) -> None:
    LEDGER.parent.mkdir(parents=True, exist_ok=True)
    with LEDGER.open("w", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=FIELDS)
        writer.writeheader()
        for key in sorted(rows):
            writer.writerow({field: rows[key].get(field, "")
                             for field in FIELDS})


def size_band(size: int) -> str:
    for limit, label in ((64, "<=64 B"), (256, "<=256 B"), (1024, "<=1 kB"),
                         (4096, "<=4 kB")):
        if size <= limit:
            return label
    return ">4 kB"


def choose(functions: dict, ledger: dict, count: int, stratified: bool) -> list:
    """The next targets. Ordered by BYTES, because bytes are the published debt.

    `stratified` spreads the pick across size bands instead, which is what a
    calibration wave wants: a wave drawn purely by size answers nothing about
    the small end, and a wave drawn purely by count answers nothing about the
    large end where 36% of the remaining mass sits in 86 functions.
    """
    done = {address for address, row in ledger.items()
            if row.get("tier") in MATCHED}
    pool = []
    for address, row in functions.items():
        if row.get("recovery_state") not in TARGET_STATES:
            continue
        if f"0x{address:08X}" in done:
            continue
        pool.append((address, int(row.get("size") or 0)))

    if not stratified:
        pool.sort(key=lambda item: -item[1])
        return [address for address, _ in pool[:count]]

    bands = collections.defaultdict(list)
    for address, size in pool:
        bands[size_band(size)].append((address, size))
    order = ("<=64 B", "<=256 B", "<=1 kB", "<=4 kB", ">4 kB")
    per = max(1, count // len(order))
    picked = []
    for label in order:
        entries = sorted(bands.get(label, []), key=lambda item: item[1])
        picked.extend(address for address, _ in entries[:per])
    return picked[:count]


def groups(count: int, per_agent: int, stratified: bool) -> int:
    """Print agent batches built from what was ACTUALLY staged.

    A batch list must be derived from the units on disk, never from the
    candidate list that fed staging. Building it from candidates handed six
    agents twelve addresses each of which 28 had no directory, because staging
    had refused or capped them - the agents dutifully reported NO_DIR and the
    work silently covered less than the queue claimed. Same shape as a
    generator reporting success while returning zero candidates.
    """
    ledger = read_ledger()
    done = {address for address, row in ledger.items()
            if row.get("tier") in MATCHED}
    functions = emit.load_functions()
    staged = []
    for unit in sorted(WORK_ROOT.glob("*/unit.cpp")):
        address = int(unit.parent.name, 16)
        if f"0x{address:08X}" in done:
            continue
        if "// BODY GOES HERE." not in unit.read_text():
            continue                      # already worked on
        staged.append((int(functions.get(address, {}).get("size") or 0),
                       address))
    staged.sort()
    if count:
        staged = staged[:count]
    if not staged:
        print("nothing staged and unworked; run --prepare first")
        return 0
    batches = [staged[i::max(1, len(staged) // per_agent or 1)]
               for i in range(max(1, len(staged) // per_agent or 1))]
    print(f"{len(staged)} staged and unworked units, "
          f"{len(batches)} batch(es) of ~{per_agent}")
    for index, batch in enumerate(batches):
        print(f"GROUP{index}: " + " ".join(f"0x{a:08X}({s})" for s, a in batch))
    return 0


def prepare(count: int, stratified: bool) -> int:
    functions = emit.load_functions()
    derived = emit.load_derived()
    callees = emit.load_callees()
    pe = pefile.PE(str(emit.DEFAULT_EXE), fast_load=True)
    ledger = read_ledger()

    chosen = choose(functions, ledger, count, stratified)
    written, refused = 0, collections.Counter()
    for address in chosen:
        directory = WORK_ROOT / f"{address:08x}"
        try:
            text = emit.emit(address, functions, derived, callees, pe)
        except emit.Unsettled as error:
            refused[str(error)[:60]] += 1
            continue
        directory.mkdir(parents=True, exist_ok=True)
        unit = directory / "unit.cpp"
        if not unit.is_file():
            unit.write_text(text)
        written += 1
        print(f"0x{address:08X}  {functions[address].get('size', '?'):>6} B  "
              f"{functions[address].get('name', '')[:56]}")

    print(f"\nprepared {written} of {len(chosen)} chosen")
    if refused:
        # Never let a target vanish silently. A queue that covers less without
        # saying so is how an exhausted generator kept reporting success.
        print(f"REFUSED {sum(refused.values())}, and they are NOT in the queue:")
        for reason, number in refused.most_common(8):
            print(f"  x{number:4d}  {reason}")
    return 0


def collect(reverify: bool = False, stored_only: bool = False) -> int:
    functions = emit.load_functions()
    pe = pefile.PE(str(byte_match.DEFAULT_EXE))
    shared = byte_match.shared_span_index(byte_match.load_rows())
    ledger = read_ledger()

    # Two sources, and the second is the one that must not be skipped. A body
    # under src/recovered/ is the ONLY committed copy of that recovery,
    # and its unit is rebuilt from the current emitter every time - so a
    # scaffolding change that stops it verifying has to surface here as a
    # ledger regression rather than as a tracked file nobody re-reads.
    # `--stored-only` skips the build artefacts. A routine collect re-verifies
    # 1,198 units under `build/byte-match/` as well as the store, and each one
    # is a real compile under wine - the full pass is over an hour. After a
    # batch of writebacks the only rows that CHANGED are the stored ones, and
    # waiting an hour to see them is how a ratchet stops being checked.
    # A row that a file in `src/` OWNS is the census's to score, and a build
    # artefact must never speak for it. `build/byte-match/<addr>/unit.cpp` is
    # whatever was written the last time that address was worked on, read back
    # VERBATIM rather than regenerated - so a stale unit can compile and score
    # BYTE_EXACT while the committed body it stands for no longer builds.
    #
    # Not hypothetical. `StringStruct::close`, `remove_all` and the two
    # ALPHAMENU_WAVE thunks were NO_COMPILE in a census; a later collect found
    # day-old units for them and wrote BYTE_EXACT back over the top, taking
    # the ledger from 867 to 884 and carrying four broken rows into the
    # ratchet floor. The next census failed, which is the only reason it
    # surfaced.
    #
    # The store is NOT skipped for the same reason: a body under
    # `src/recovered/` is the only committed copy of that recovery, so if this
    # does not score it, nothing does.
    owned = {address for address, row in functions.items()
             if (row.get("source_locations") or "").strip()}
    units = [] if stored_only else [
        (int(unit.parent.name, 16), unit.read_text, "unit")
        for unit in sorted(WORK_ROOT.glob("*/unit.cpp"))
        if int(unit.parent.name, 16) not in owned]
    stored = [(int(body.stem, 16), body, "stored")
              for body in sorted(writeback.MATCHED_DIR.glob("*.cpp"))]
    if not units and not stored:
        print(f"no units under {WORK_ROOT} and nothing under "
              f"{writeback.MATCHED_DIR}")
        return 0
    print(f"re-verifying {len(units)} units and {len(stored)} stored bodies "
          f"(agent reports are not evidence)")

    # Hoisted: this used to re-read the 1.2 MB catalogue once PER UNIT, which
    # is 194 parses of 6,000 rows to answer 194 questions.
    catalogue = byte_match.load_rows()
    # A row already at BYTE_EXACT is SETTLED and is not re-measured. Two
    # reasons, and the second is the one that bit:
    #
    #  * cost - re-verifying hundreds of finished rows on every collect is
    #    most of the run, and the answer never changes;
    #  * a LONG collect finishes AFTER later runs have written newer results,
    #    and then overwrites them with what it measured before the tool
    #    changed. That happened twice here. The ratchet caught it both times,
    #    but a guard that makes the overwrite impossible beats one that
    #    reports it afterwards.
    #
    # Use --reverify to re-measure everything, which is what a tooling change
    # wants and a routine collect does not.
    settled = {address for address, row in ledger.items()
               if row.get("tier") == "BYTE_EXACT"} if not reverify else set()
    derived = emit.load_derived()
    callees = emit.load_callees()
    pe_fast = pefile.PE(str(byte_match.DEFAULT_EXE), fast_load=True)
    subjects = units + stored
    tiers, regressions = collections.Counter(), []
    with tempfile.TemporaryDirectory() as tmp:
        work = Path(tmp)
        skipped = 0
        for index, (address, source, kind) in enumerate(subjects, start=1):
            key = f"0x{address:08X}"
            if key in settled:
                skipped += 1
                continue
            row = functions.get(address, {})
            if kind == "unit":
                text = source()
            else:
                # A stored body is a DEFINITION, not a unit: the scaffolding
                # around it is regenerated rather than committed, because it is
                # derived from the pinned executable.
                try:
                    text = writeback.build_unit(
                        address, writeback.read_matched_body(source),
                        functions, callees, derived, pe_fast)
                except emit.Unsettled as error:
                    tiers["REFUSED"] += 1
                    if ledger.get(key, {}).get("tier", "") in MATCHED:
                        regressions.append((key, "BYTE_EXACT",
                                            f"no scaffolding: {error}"))
                    continue
            outcome = byte_match.match_function(
                pe, catalogue, shared, address,
                text, work, byte_match.MEASURED_FLAGS,
                f"v{address:08x}")
            if index % 25 == 0:
                print(f"  {index}/{len(subjects)}", flush=True)
            tier = outcome.get("tier", "NO_COMPILE")
            was = ledger.get(key, {}).get("tier", "")
            if was in MATCHED and tier not in MATCHED:
                regressions.append((key, was, tier))
            tiers[tier] += 1
            entry = ledger.setdefault(key, {})
            entry.update(
                address=key, name=row.get("name", ""),
                source_location="", generated=0,
                size=row.get("size", ""), tier=tier,
                span_classes=";".join(
                    f"{k}={v}" for k, v in
                    (outcome.get("span_classes") or {}).items()),
                original_bytes=outcome.get("original_bytes", ""),
                rebuilt_bytes=outcome.get("rebuilt_bytes", ""),
                original_mnemonics=outcome.get("original_mnemonics", ""),
                rebuilt_mnemonics=outcome.get("rebuilt_mnemonics", ""),
                first_divergence=outcome.get("first_divergence", ""),
                note=outcome.get("note", ""),
                refusal_reason=outcome.get("refusal_reason", ""))

    write_ledger(ledger)
    print(f"\nmeasured tiers: {dict(tiers)}")
    matched = sum(tiers[t] for t in MATCHED if t in tiers)
    print(f"matched {matched}/{len(subjects) - skipped} measured "
          f"({skipped} already settled)")
    if regressions:
        print("\nREGRESSIONS - these matched before and do not now:")
        for key, was, now in regressions:
            print(f"  {key}  {was} -> {now}")
    print(f"wrote {LEDGER}")
    return 0


# The ledger's own floor. Matched functions and matched bytes may RISE and may
# never fall - the same ratchet shape as `verify_observability_ratchet.py` and
# `audit_export_signedness.py`'s BASELINE. Raise these as waves land; a fall is
# either a tooling regression or a lost unit, and both need a human.
#
# Read these together. The exact-match population is deliberately ordered from
# the smallest bodies upward, so its function count is dominated by trivial
# stubs and can rise much faster than the number of machine-carried bytes it
# retires. The byte figure is the one that means anything, which is why both
# are ratcheted and why the function count is never quoted alone.
# LOWERED, deliberately, from 702 / 11199 - the only downward move this file
# has ever made, and it is a measurement correction rather than a regression.
# A full census found three rows sitting at BYTE_EXACT for bodies that no
# longer compile at all:
#
#   0x00401060  ?close@StringStruct@@QAEXXZ    153 B
#   0x00402F10  ??__Eg_ALPHAMENU_WAVE@@YAXXZ    22 B
#   0x00402F30  ??__Fg_ALPHAMENU_WAVE@@YAXXZ    10 B
#
# None is a body defect. Each calls something the scaffolding cannot declare -
# `close_with_tables` is a hand-written helper on StringStruct that appears in
# NO catalogued row, and `g_ALPHAMENU_WAVE` is an uncatalogued global - so the
# verification unit cannot compile them however good the body is.
#
# They went green long ago and stayed green because nothing re-measured them:
# `--collect` skips settled rows by design, since re-verifying finished work is
# most of a run. That is the right default and it has this cost, so the number
# above is only as true as the last full census.
#
# 883 / 18598, from a full census after the VC6 build was made to compile.
#
# It went 880 -> 866 first, and the drop is worth recording because the cause
# was not the code. Fixing the build inserted lines near the top of
# src/datalink.cpp and src/vector.cpp - an #include and a rewritten
# constructor - and every recovered body BELOW an insertion moved. The census
# extracts a body by the line in `source_locations`, so it began reading from
# the wrong offset and 17 rows failed with "extract does not end in a closing
# brace". Not one of them had changed.
#
# `tools/repair_source_locations.py --apply` re-points each row from its own
# `Original Offset:` marker rather than by counting lines, so it cannot drift
# in turn. It found 242 stale locations across 10 files, of which the 17
# byte-exact ones were the only visible symptom.
#
# THE LESSON IS THE ORDERING, not the tool: any edit to a file holding
# recovered bodies must be followed by the repair before the census, or the
# census measures the wrong bytes and blames the body.
#
# 888 / 18738 after repairing tools/class_layouts.py's type model. The rise is
# the point of that repair and is worth recording as evidence for it: nothing
# in src/ changed, only what the emitter could SAY about a class. Five bodies
# went byte-exact because their scaffolding stopped being an opaque shell.
#
# What had been refusing them was not unknown layout. `RECT` is a Win32struct
# held by value and could not be spelled as a primitive, so four classes
# refused outright and the cascade through Buffer and GraphicWin took
# seventeen; `Dib` is `typedef int32_t Dib;` nested inside Buffer and the
# parser never learned nested aliases; and function-pointer members were
# refused by a branch whose comment said "no pinned class has one today",
# which `Time` had since made false.
BASELINE_MATCHED_FUNCTIONS = 888
BASELINE_MATCHED_BYTES = 18738


def summarise(ledger: dict) -> tuple:
    matched = [row for row in ledger.values() if row.get("tier") in MATCHED]
    return len(matched), sum(int(row.get("size") or 0) for row in matched)


def check() -> int:
    """Refuse a ledger that has gone backwards."""
    ledger = read_ledger()
    if not ledger:
        print(f"byte-match-ratchet: {LEDGER} is absent; nothing to check")
        return 0
    functions, size = summarise(ledger)

    # A body may be traded down so the tree compiles - see
    # `tools/build_regressions.py`. That is allowed and it is NOT free: the
    # debt is recorded per address with the tier it held and why it was given
    # up, and forgiven here against the floor. The alternative is lowering the
    # floor, which is a number moving down with no record of which functions
    # regressed or whether anyone meant it.
    import build_regressions
    forgiven, regressions = build_regressions.owed()
    forgiven_bytes = sum(int(functions_row.get("size") or 0)
                         for functions_row in (
                             ledger.get(row["address"].upper(), {})
                             for row in regressions))

    print(f"byte-match-ratchet: {functions} functions / {size} bytes matched "
          f"(floor {BASELINE_MATCHED_FUNCTIONS} / {BASELINE_MATCHED_BYTES})")
    if forgiven:
        print(f"  {forgiven} body(ies) traded down for the build, forgiven "
              f"against the floor - tools/build_regressions.py --list")
    failed = False
    if functions + forgiven < BASELINE_MATCHED_FUNCTIONS:
        print(f"  FAIL: matched functions fell {BASELINE_MATCHED_FUNCTIONS} "
              f"-> {functions}")
        failed = True
    if size + forgiven_bytes < BASELINE_MATCHED_BYTES:
        print(f"  FAIL: matched bytes fell {BASELINE_MATCHED_BYTES} -> {size}")
        failed = True
    if functions > BASELINE_MATCHED_FUNCTIONS or size > BASELINE_MATCHED_BYTES:
        print(f"  the floor is stale and should be raised to "
              f"{functions} / {size}")
    return 1 if failed else 0


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--prepare", type=int, metavar="N")
    parser.add_argument("--stratified", action="store_true",
                        help="spread the pick across size bands (a calibration "
                             "wave); default is largest-bytes-first")
    parser.add_argument("--collect", action="store_true")
    parser.add_argument(
        "--stored-only", action="store_true",
        help="with --collect: re-verify only src/recovered/, not the "
             "build artefacts. Minutes instead of an hour.")
    parser.add_argument("--reverify", action="store_true",
                        help="re-measure rows already at BYTE_EXACT too; use "
                             "after a tooling change, not for a routine "
                             "collect")
    parser.add_argument("--groups", type=int, metavar="N", nargs="?", const=0,
                        help="print agent batches built from the units ACTUALLY "
                             "on disk, optionally limited to the N smallest")
    parser.add_argument("--per-agent", type=int, default=12,
                        help="addresses per batch (default 12)")
    parser.add_argument("--check", action="store_true",
                        help="refuse a ledger whose matched count or matched "
                             "bytes have fallen")
    arguments = parser.parse_args()

    if arguments.check:
        return check()
    if arguments.groups is not None:
        return groups(arguments.groups, arguments.per_agent,
                      arguments.stratified)
    if arguments.prepare:
        return prepare(arguments.prepare, arguments.stratified)
    if arguments.collect:
        reason = byte_match.available()
        if reason:
            print(f"SKIP: {reason}")
            return 0
        return collect(arguments.reverify, arguments.stored_only)
    parser.error("one of --prepare N, --collect or --check is required")


if __name__ == "__main__":
    sys.exit(main())
