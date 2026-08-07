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

import byte_match_census as census  # noqa: E402
import emit_translation_unit as emit  # noqa: E402
import mizuchi_writeback as writeback  # noqa: E402

MATCHED = "BYTE_EXACT"


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

    verdict = writeback.verify(address, body)
    tier = verdict.get("tier", "?")

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
