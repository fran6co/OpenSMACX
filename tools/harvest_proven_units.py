#!/usr/bin/env python3
"""Commit the recoveries that are already proved and are about to be lost.

The ledger records 186 unrecovered rows as BYTE_EXACT. Every one of them was
proved by a mizuchi run, and every one of them exists ONLY as
`build/byte-match/<address>/unit.cpp` - a gitignored build artefact. A
`git clean` erases the lot. The ledger would still say BYTE_EXACT, because the
ledger records that a matching body EXISTED, not that anyone kept it.

`mizuchi_writeback.py` is the tool that keeps one, and it takes a BODY. A unit
is scaffolding plus a body, and pasting the whole thing into the store would
duplicate every declaration the writeback is about to regenerate. So the body
has to come back out first, and it is found by its own DEFINITION HEAD - the
line the emitter still computes for every address.

Stripping the current scaffolding as a prefix was tried first and rejects all
186: the emitter's header COMMENT has been rewritten since these units were
generated - the VC6 dialect rules moved out into the agent system prompt - so
every unit diverges after 325 bytes, in prose, while the declarations below it
are fine. The head is load-bearing where the comment never was.

103 units yield a body. The other 83 no longer contain their own head, which
means the catalogued SIGNATURE has changed since the proof was recorded - and
a body proved against a different signature is not a body for this function.
Those are reported, not trimmed to fit.

    tools/harvest_proven_units.py            # report
    tools/harvest_proven_units.py --apply    # keep them
"""

from __future__ import annotations

import argparse
import csv
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import byte_match  # noqa: E402
import emit_translation_unit as emit  # noqa: E402
import emit_mizuchi_prompts as prompts  # noqa: E402
import mizuchi_writeback as writeback  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
WORK_ROOT = REPO_ROOT / "build" / "byte-match"
LEDGER = REPO_ROOT / ".opensmacx" / "byte-match.csv"

TARGET_STATES = ("unrecovered", "original_dependency")


def ledger_tiers() -> dict:
    if not LEDGER.is_file():
        return {}
    with LEDGER.open(newline="", encoding="utf-8-sig") as handle:
        return {row["address"].upper(): row.get("tier", "")
                for row in csv.DictReader(handle)}


def candidates(functions: dict, tiers: dict) -> list:
    """Addresses proved BYTE_EXACT whose only copy is a build artefact."""
    found = []
    for unit in sorted(WORK_ROOT.glob("*/unit.cpp")):
        try:
            address = int(unit.parent.name, 16)
        except ValueError:
            continue
        if (writeback.MATCHED_DIR / f"{address:08x}.cpp").is_file():
            continue                       # already kept
        row = functions.get(address)
        if row is None or row.get("recovery_state") not in TARGET_STATES:
            continue
        # A row with a `src/` home already has an authoritative body, and a
        # store copy alongside it is not a spare - it is a SECOND ANSWER for
        # one address. The census scores the `src/` one and `--collect` scores
        # the store one, both write the same ledger row, and the tier then
        # depends on which tool ran last. Three rows reached that state and
        # made the ratchet read 702, then 699, then 702 again in one session;
        # the swing was read first as a branch regression and then as a stale
        # row, and it was neither.
        #
        # `recovery_state` alone does not catch it: these were catalogued
        # `unrecovered` while carrying a `source_locations`.
        if (row.get("source_locations") or "").strip():
            continue
        if tiers.get(f"0X{address:08X}") != "BYTE_EXACT":
            continue
        found.append((address, unit))
    return found


def body_of(address: int, unit_text: str, functions: dict, derived: dict,
            callees: dict, pe_fast) -> str:
    """The definition, found by its own head rather than by a prefix.

    Stripping the current scaffolding as a prefix does not work: the emitter's
    HEADER COMMENT has been rewritten since these units were generated - the
    VC6 dialect rules moved out into the agent system prompt - so all 186 units
    diverge from it after 325 bytes, in prose, while the declarations below are
    fine. Comparing text that was never load-bearing rejected every one.

    Anchoring on the definition HEAD does not work either, and the way it
    failed is worth keeping. It rescued 65 of 186 and refused the rest - and
    re-measuring all 122 refusals by compiling their units WHOLE showed every
    single one still BYTE_EXACT. The proofs were fine; the extraction was
    wrong, twice over. 83 units no longer contain today's head because the
    catalogued name has since been corrected, and 38 more extracted a body
    that would not compile because agents declare shim classes and typedefs
    ABOVE the definition, and cutting at the head throws them away.

    So the cut is made where the SCAFFOLDING ends, which is what "the body"
    always meant. The scaffolding's declarations are stable even though its
    header comment is not, so the anchor is its last real line of code - not
    its prose, and not the subject's own signature.

    THE EMITTER GROWS, AND THE CUT HAS TO SURVIVE THAT. Searching for the last
    line and only the last line assumed the scaffolding a unit was written
    against still ends where today's ends. It does not: the emitter has since
    gained a second typedef block (`int32`..`uint8` alongside `int32_t`..
    `uint8_t`), and every unit predating that gain stopped containing today's
    final declaration. 49 proved recoveries were refused over it while nothing
    about the proof had changed - each unit's scaffolding was a strict PREFIX
    of the current one, which is the case that most obviously should succeed.

    So the cut is the longest agreeing PREFIX of code lines, and the position
    is counted rather than searched. Prefix comparison was tried before and
    rejected, correctly, for comparing the whole text: the header COMMENT is
    rewritten constantly and diverges within 325 bytes while the declarations
    below it are fine. Comparing only code lines keeps the part that was ever
    load-bearing. Counting instead of searching is what a `rfind` cannot do -
    `};` closes the emitted shell AND every shim struct an agent wrote beside
    its body, so any search for it cuts at the wrong brace, while the sixth
    code line is unambiguously the sixth code line.

    A cut that is wrong anyway is not silent: `mizuchi_writeback` recompiles
    the body and refuses it unless it still scores BYTE_EXACT. If the emitter
    ever changes a line in the MIDDLE, agreement stops early, stale
    declarations ride along into the body, and the compiler rejects the
    duplicate - which is the refusal we already had, not a new failure mode.
    """
    scaffolding = emit.emit(address, functions, derived, callees, pe_fast,
                            scaffolding_only=True)
    wanted = [line.strip() for line in scaffolding.splitlines()
              if line.strip() and not line.lstrip().startswith("//")]
    if not wanted:
        raise ValueError("scaffolding has no code to anchor on")

    agreed, cut, seen = 0, 0, 0
    for line in unit_text.splitlines(keepends=True):
        stripped = line.strip()
        if not stripped or stripped.startswith("//"):
            continue
        if seen >= len(wanted) or stripped != wanted[seen]:
            break
        seen += 1
        agreed += 1
        cut = unit_text.index(line, cut) + len(line)
    if not agreed:
        raise ValueError(f"unit shares no scaffolding with the emitter; its "
                         f"first declaration should be {wanted[0]!r}")
    body = unit_text[cut:]
    if not body.strip():
        raise ValueError("nothing follows the scaffolding")
    return body.strip() + "\n"


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--apply", action="store_true",
                        help="write the bodies; without it, only report")
    parser.add_argument("--limit", type=int)
    arguments = parser.parse_args(argv)

    import pefile
    functions = emit.load_functions()
    tiers = ledger_tiers()
    found = candidates(functions, tiers)
    if arguments.limit:
        found = found[:arguments.limit]
    print(f"{len(found)} proved recoveries exist only as build artefacts")
    if not found:
        return 0

    derived = emit.load_derived()
    callees = emit.load_callees()
    pe_fast = pefile.PE(str(byte_match.DEFAULT_EXE), fast_load=True)

    kept, refused = 0, []
    for address, unit in found:
        try:
            body = body_of(address, unit.read_text(), functions, derived,
                           callees, pe_fast)
        except (ValueError, emit.Unsettled) as error:
            refused.append((address, str(error)[:70]))
            continue
        if not arguments.apply:
            kept += 1
            continue
        try:
            writeback.writeback(f"0x{address:08X}", body)
            kept += 1
            print(f"  kept 0x{address:08X}")
        except writeback.Refused as error:
            refused.append((address, str(error)[:70]))

    verb = "kept" if arguments.apply else "would keep"
    print(f"\n{verb} {kept}, refused {len(refused)}")
    for address, why in refused[:20]:
        print(f"  0x{address:08X}  {why}")
    if len(refused) > 20:
        print(f"  ... and {len(refused) - 20} more")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
