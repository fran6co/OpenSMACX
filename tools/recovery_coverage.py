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

EH unwind funclets are reported separately rather than counted as debt.
`docs/EXCLUSIONS.md` records why a standalone compile cannot reproduce one:
they borrow the enclosing function's frame, and it has been measured three
times over.

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


def survey(functions: dict) -> dict:
    owned = {address for address, row in functions.items()
             if (row.get("source_locations") or "").strip()}
    proved = addresses(PROVED) - owned
    preserved = addresses(PRESERVED) - owned - proved
    covered = owned | proved | preserved
    rest = [a for a in functions if a not in covered]
    funclets = [a for a in rest if EH_LOW <= a <= EH_HIGH]
    return {"functions": functions, "owned": owned, "proved": proved,
            "preserved": preserved, "covered": covered,
            "uncovered": [a for a in rest if not EH_LOW <= a <= EH_HIGH],
            "funclets": funclets}


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
    arguments = parser.parse_args(argv)

    functions = emit.load_functions()
    state = survey(functions)
    total = len(functions)
    covered = len(state["covered"])

    print(f"catalogued functions   {total}")
    print(f"  owned by src/        {len(state['owned']):>6}")
    print(f"  proved store         {len(state['proved']):>6}")
    print(f"  preserved units      {len(state['preserved']):>6}")
    print(f"  COVERED              {covered:>6}  "
          f"({100 * covered / total:.1f}%)")
    print(f"  uncovered            {len(state['uncovered']):>6}")
    print(f"  EH funclets          {len(state['funclets']):>6}  "
          f"(excluded, see docs/EXCLUSIONS.md)")

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
