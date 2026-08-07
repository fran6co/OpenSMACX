#!/usr/bin/env python3
"""Score ONE already-recovered function, without touching the tree.

`byte_match_census.py` scores all 2,518 committed bodies and takes minutes;
`byte_match_fanout.py --prepare` only ever queues functions that are NOT
recovered yet. Between them there was no way to ask "is this one body byte
exact, and if not where does it diverge" - which is the loop anyone improving
an existing recovery needs, and the loop an agent proposing a replacement needs
most of all.

The machinery already existed. `mizuchi_writeback.verify(address, body)`
rebuilds the unit and re-measures it; it simply had no command line. This is
that command line, plus the one thing it was missing: the ability to score a
CANDIDATE body instead of the committed one, so a proposed fix can be checked
before anything is written.

    tools/verify_recovered_function.py 0x006077F0
    tools/verify_recovered_function.py 0x006077F0 --body candidate.cpp
    cat candidate.cpp | tools/verify_recovered_function.py 0x006077F0 --body -

Exit code is 0 only for BYTE_EXACT, so it works directly as a loop condition -
the same contract `byte_match.py` uses, and the reason agent reports are never
taken as evidence: the claim and the measurement are different things, and this
is the measurement.

IT USES THE WRITEBACK RECIPE, NOT THE CENSUS RECIPE. The census omits
`mizuchi_declfix`, which respells callee declarations so VC6 re-mangles them to
the catalogued names; without it a body calling a CRT function reads as
NO_COMPILE. So a function can score NO_COMPILE in the census and BYTE_EXACT
here, and here is the one that matches what the integrator gates on.
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import byte_match  # noqa: E402
import byte_match_census as census  # noqa: E402
import emit_translation_unit as emit  # noqa: E402
import mizuchi_writeback as writeback  # noqa: E402

MATCHED = "BYTE_EXACT"

# BYTE EXACTNESS IS NOT THE ONLY THING THAT MATTERS. A body can reproduce the
# instruction stream and still be the wrong thing to have in the tree, and the
# compiler cannot tell the difference - so the form is checked separately.
#
# The first entry is a REFUSAL, not a warning: `__asm` pastes the original's
# instructions instead of deriving them, so it proves nothing about the source,
# and `AGENTS.md:5` bars copied machine code from distributable builds.
# `mizuchi_writeback.py` deliberately carries no content policy, so without a
# check here an `__asm` body that happens to be byte-exact lands in `src/`
# unchallenged. That has happened once already.
FORBIDDEN = (
    (r"\b__asm\b|\b_emit\b",
     "inline assembly: pastes the original's instructions instead of deriving "
     "them (AGENTS.md:5)"),
)
QUESTIONABLE = (
    (r"\(\s*(?:[^)]*,)?\s*uint32_t\s+\w+\s*[,)]",
     "uint32_t parameter: AGENTS.md:87 says parameters are int, and the "
     "signedness is visible in the branch the compiler picks"),
    (r"0x00[45678][0-9A-Fa-f]{5}\b",
     "a fixed image address written into the body: the tree reaches those "
     "through a rebindable seam"),
    (r"\bgoto\b", "goto"),
)


def form_report(body: str) -> tuple:
    """(hard refusals, things worth a second look)."""
    import re
    refusals = [why for pattern, why in FORBIDDEN if re.search(pattern, body)]
    notes = [why for pattern, why in QUESTIONABLE if re.search(pattern, body)]
    return refusals, notes


def committed_body(address: int) -> tuple:
    """(body text, source location) for the recovery on disk, or (None, why)."""
    functions = emit.load_functions()
    row = functions.get(address)
    if row is None:
        return None, f"0x{address:08X} is not in the catalogue"
    location = (row.get("source_locations") or "").split(";")[0].strip()
    if not location:
        return None, (f"0x{address:08X} has no source_locations, so nothing is "
                      f"recovered for it yet - use byte_match_fanout.py "
                      f"--prepare instead")
    try:
        return census.extract_body(location), location
    except (ValueError, OSError) as error:
        return None, f"{location}: {error}"


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("address", help="hex address, e.g. 0x006077F0")
    parser.add_argument("--body", type=str,
                        help="score this candidate instead of what is "
                             "committed; '-' reads stdin")
    parser.add_argument("--against-committed", action="store_true",
                        help="with --body: also score what is committed and "
                             "refuse a candidate that is WORSE")
    parser.add_argument("--json", action="store_true")
    arguments = parser.parse_args(argv)

    try:
        address = int(arguments.address, 16)
    except ValueError:
        print(f"error: {arguments.address} is not a hex address",
              file=sys.stderr)
        return 2

    if arguments.body:
        body = (sys.stdin.read() if arguments.body == "-"
                else Path(arguments.body).read_text())
        source = "candidate"
    else:
        body, source = committed_body(address)
        if body is None:
            print(f"SKIP: {source}")
            return 0

    refusals, notes = form_report(body)
    for why in refusals:
        print(f"    REFUSED on form: {why}", file=sys.stderr)
    for why in notes:
        print(f"    form: {why}")
    if refusals:
        # Refused before compiling. A byte-exact answer would only make this
        # harder to argue with.
        return 1

    verdict = writeback.verify(address, body)
    tier = verdict.get("tier", "?")

    # A CANDIDATE THAT IS WORSE IS NOT AN IMPROVEMENT, and "byte exact or not"
    # cannot see the difference between a candidate that stayed MNEMONIC_ONLY
    # and one that fell to MISMATCH. Both are non-zero exits. This is the
    # per-function form of the ratchet `byte_match_fanout.py --check` applies
    # to the whole ledger: a tier may rise and may not fall.
    if arguments.against_committed and arguments.body:
        incumbent_body, incumbent_source = committed_body(address)
        if incumbent_body is not None:
            incumbent = writeback.verify(address, incumbent_body)
            print(f"    committed ({incumbent_source}): "
                  f"{incumbent.get('tier', '?')}")
            if byte_match._better(incumbent, verdict):
                print(f"    REGRESSION: the candidate is worse than what is "
                      f"already committed; refusing it", file=sys.stderr)
                return 1

    if arguments.json:
        import json
        print(json.dumps({**verdict, "address": f"0x{address:08X}",
                          "source": source}, indent=2, default=str))
    else:
        print(f"0x{address:08X}  {tier}   ({source})")
        for key, label in (("note", "divergence"),
                           ("refusal_reason", "reason"),
                           ("first_divergence", "first differing mnemonic"),
                           ("original_bytes", "original bytes"),
                           ("rebuilt_bytes", "rebuilt bytes")):
            value = verdict.get(key)
            if value not in (None, ""):
                print(f"    {label}: {value}")
    return 0 if tier == MATCHED else 1


if __name__ == "__main__":
    raise SystemExit(main())
