#!/usr/bin/env python3
"""How much of the executable has a body in the tree at all.

COVERAGE IS NOT THE RATCHET, and conflating them is the mistake this exists
to prevent. `byte_match_fanout.py --check` answers "how much is proved
byte-identical", may only ever rise, and is the gate. This answers "how much
has anybody written a body for", counts a measured MISMATCH exactly the same
as a BYTE_EXACT, and gates nothing.

Both numbers are needed and they move for different reasons. The plan is
coverage first - a body in the tree for every function - and then passes that
turn those bodies byte-exact. During the first phase the ratchet barely moves
while coverage climbs; during the second, coverage is flat while the ratchet
climbs. A single number would hide whichever phase is running.

A function counts as covered when a body for it exists somewhere committed:

  owned      `functions.csv` gives it `source_locations`, so a `src/` file
             holds it as product source
  proved     `src/recovered/<address>.cpp`, byte-exact and re-verified by
             every `--collect`
  preserved  `src/recovered/units/<address>.cpp`, a whole unit kept for
             coverage and stamped with the tier it measured, proved or not

Two populations are reported apart rather than counted as debt, and both are
`docs/EXCLUSIONS.md` decisions this tool follows rather than re-litigates:

  EH unwind funclets (section 2a) borrow the enclosing function's frame, so a
  standalone compile cannot reproduce one. Measured three times over.

  The MSVC 6 CRT (section 1), `recovery_state == external_library`. It is in
  the executable and it is not Alpha Centauri: 278 of the 327 carry the
  leading-underscore C convention. The section's own conclusion is that
  linking the ISO's `LIBC.LIB` makes these identical rather than
  assumed-equivalent, which is a linking job, not a recovery one.

A third population is reported apart for a different reason. 59 rows are
`recovery_state == source_complete` with an EMPTY `source_locations`: the
catalogue says a `src/` file implements them and does not say which one.
Spot-checked, they are real - `Spot::clear` is `src/spot_recovery.cpp:12`,
`text_get` is `src/text.cpp:241` - so counting them as work still to do
overstates the debt by about 1%. They are not counted as covered either,
because nothing here can point at the body.

Repairing them is NOT free and is deliberately not done automatically. The
census reads a body from `source_locations` and scores whatever it extracts
against the address it MEANT to read, so a location pointed at the wrong
body produces a confident wrong verdict - the exact failure
`tools/repair_source_locations.py` documents at length. 31 of the 59 have
their address appearing exactly once anywhere in `src/` and could be placed
mechanically; the other 28 never appear at all and would have to be matched
by name, which is a guess. Until a row can be placed by evidence, it is
reported and not counted.

`--include-crt` counts the CRT anyway, for the question "how much of the
executable", as distinct from "how much of the game".

    tools/recovery_coverage.py           # the numbers
    tools/recovery_coverage.py --left    # what is still uncovered, by size
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import emit_translation_unit as emit  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
PROVED = REPO_ROOT / "src" / "recovered"
PRESERVED = PROVED / "units"

# Compiler-generated EH unwind funclets. See docs/EXCLUSIONS.md section 2a.
EH_LOW, EH_HIGH = 0x0065_0000, 0x0066_FFFF

BANDS = ((64, "<=64"), (128, "65-128"), (256, "129-256"), (1024, "257-1k"))


def addresses(directory: Path) -> set:
    if not directory.is_dir():
        return set()
    found = set()
    for path in directory.glob("*.cpp"):
        try:
            found.add(int(path.stem, 16))
        except ValueError:
            continue
    return found


def survey(functions: dict, include_crt: bool = False) -> dict:
    owned = {address for address, row in functions.items()
             if (row.get("source_locations") or "").strip()}
    proved = addresses(PROVED) - owned
    preserved = addresses(PRESERVED) - owned - proved
    covered = owned | proved | preserved
    rest = [a for a in functions if a not in covered]
    funclets = [a for a in rest if EH_LOW <= a <= EH_HIGH]
    crt = [] if include_crt else [
        a for a in rest if not EH_LOW <= a <= EH_HIGH
        and functions[a].get("recovery_state") == "external_library"]
    unlocated = [a for a in rest if not EH_LOW <= a <= EH_HIGH
                 and functions[a].get("recovery_state") == "source_complete"]
    excluded = set(funclets) | set(crt) | set(unlocated)
    return {"functions": functions, "owned": owned, "proved": proved,
            "preserved": preserved, "covered": covered,
            "uncovered": [a for a in rest if a not in excluded],
            "funclets": funclets, "crt": crt, "unlocated": unlocated}


def band(size: int) -> str:
    for limit, name in BANDS:
        if size <= limit:
            return name
    return ">1k"


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--left", action="store_true",
                        help="list what is still uncovered, smallest first")
    parser.add_argument("--limit", type=int, default=40)
    parser.add_argument("--include-crt", action="store_true",
                        help="count the MSVC 6 CRT as debt (EXCLUSIONS.md #1)")
    arguments = parser.parse_args(argv)

    functions = emit.load_functions()
    state = survey(functions, include_crt=arguments.include_crt)
    total = (len(functions) - len(state["funclets"]) - len(state["crt"])
             - len(state["unlocated"]))
    covered = len(state["covered"])

    print(f"in scope               {total}")
    print(f"  owned by src/        {len(state['owned']):>6}")
    print(f"  proved store         {len(state['proved']):>6}")
    print(f"  preserved units      {len(state['preserved']):>6}")
    print(f"  COVERED              {covered:>6}  "
          f"({100 * covered / total:.1f}%)")
    print(f"  uncovered            {len(state['uncovered']):>6}")
    print(f"  EH funclets          {len(state['funclets']):>6}  "
          f"(excluded, EXCLUSIONS.md 2a)")
    if state["crt"]:
        print(f"  MSVC 6 CRT           {len(state['crt']):>6}  "
              f"(excluded, EXCLUSIONS.md 1; --include-crt to count)")
    if state["unlocated"]:
        print(f"  source_complete      {len(state['unlocated']):>6}  "
              f"(implemented in src/, catalogue does not say where)")

    sizes = {}
    for address in state["uncovered"]:
        size = int(functions[address].get("size") or 0)
        sizes.setdefault(band(size), []).append(size)
    if sizes:
        print("\nuncovered by size:")
        for _limit, name in BANDS + ((0, ">1k"),):
            if name in sizes:
                group = sizes[name]
                print(f"  {name:>8}  {len(group):>5} function(s), "
                      f"{sum(group):>8} bytes")

    if arguments.left:
        print("\nnext up, smallest first:")
        ordered = sorted(state["uncovered"],
                         key=lambda a: int(functions[a].get("size") or 0))
        for address in ordered[:arguments.limit]:
            row = functions[address]
            print(f"  0x{address:08X}  {str(row.get('size') or 0):>6} B  "
                  f"{(row.get('name') or '')[:58]}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
