#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# dependencies = ["capstone==5.0.6"]
# ///
r"""How far is the jackal engine from being a separable static library?

`jackal` is the engine layer this game sits on: WinMain calls
`jackal_version_check`, then `jackal_init_real` to bring every subsystem up, and
`jackal_close` to tear them down. Pulling it into src/jackal/ as its own library
is a stated goal, and the question that decides WHEN is not "where is the seam"
- one call-graph walk answers that - but "do the seam and the FILE boundaries
agree", because a static library is a set of translation units.

Measured 2026-08-22, the answer was no, and by a wide margin: of 92
product-source modules in the WinMain graph, 4 were purely jackal, 54 purely
game, and 34 MIXED - holding 160 jackal bodies and 160 game bodies, an even
split. Every mixed module has to be cut in two, and each cut moves bodies
between translation units while thousands of verified claims depend on COMDAT
identity inside those units.

So this exists to make the WAITING measurable rather than re-argued. Run it
whenever you want to know whether the split has become cheap: the number to
watch is MIXED, and it should fall on its own as bodies are promoted out of
catch-all modules like leaf_recoveries.cpp and general.cpp.

    uv run tools/jackal_seam.py
    uv run tools/jackal_seam.py --list      # name the mixed modules

Exit 0 always: this reports a distance, not a fault.
"""

from __future__ import annotations

import collections
import importlib.util
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO_ROOT))

from decomp.reader import read  # noqa: E402

_spec = importlib.util.spec_from_file_location(
    "frontier", REPO_ROOT / "tools" / "frontier.py")
_frontier = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(_frontier)

EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"
# THE THREE ENTRY POINTS, found by NAME rather than pinned by address: the
# addresses are in the annotations already and hard-coding them here would be a
# second copy to drift.
ROOTS = ("jackal_version_check", "jackal_init_real", "jackal_close")


def address_of(name: str, records) -> int | None:
    for record in records:
        if record.name and name in record.name:
            return record.address
    return None


if __name__ == "__main__":
    if not EXE.exists():
        print(f"  no image at {EXE} - nothing to walk")
        raise SystemExit(0)
    records = list(read(REPO_ROOT / "src"))

    jackal: set[int] = set()
    for name in ROOTS:
        address = address_of(name, records)
        if address is None:
            print(f"  {name}: not annotated, skipped")
            continue
        order, _edges, _unnamed = _frontier.walk(records, EXE, address)
        jackal |= {r.address for r in order}

    main = (REPO_ROOT / "src" / "main.cpp").read_text(errors="replace")
    found = re.search(r"^// ORIGINAL: (0x[0-9A-Fa-f]{8}) \S*WinMain\S*",
                      main, re.M)
    if not found:
        print("  WinMain is not annotated; cannot compare")
        raise SystemExit(0)
    order, _e, _u = _frontier.walk(records, EXE, int(found.group(1), 16))
    winmain = {r.address for r in order}

    # PRODUCT SOURCE ONLY. src/recovered/ and src/unrecovered/ are verification
    # artifacts the build never compiles, and counting them made the first run
    # of this report claim 210 "purely jackal" modules - almost all of them
    # one-function artifact files, which no library would ever contain.
    tally: dict[str, list[int]] = collections.defaultdict(lambda: [0, 0])
    for record in records:
        if record.path.parent.name != "src":
            continue
        if record.address in jackal:
            tally[record.path.name][0] += 1
        elif record.address in winmain:
            tally[record.path.name][1] += 1

    pure_jackal = sorted(f for f, (j, g) in tally.items() if j and not g)
    pure_game = [f for f, (j, g) in tally.items() if not j]
    mixed = sorted(((f, j, g) for f, (j, g) in tally.items() if j and g),
                   key=lambda row: -(row[1] + row[2]))

    print(f"  {len(jackal):,} bodies reachable from the jackal roots, "
          f"{len(jackal - winmain):,} of them outside the WinMain graph")
    print(f"\n  {len(tally)} product-source module(s) in the graph:")
    print(f"    purely jackal : {len(pure_jackal)}  "
          f"{', '.join(pure_jackal)}")
    print(f"    purely game   : {len(pure_game)}")
    print(f"    MIXED         : {len(mixed)}   <- each must be split in two")

    if "--list" in sys.argv:
        print("\n  mixed modules (jackal bodies / game bodies):")
        for name, j, g in mixed:
            print(f"    {name:24} {j:4} / {g:4}")

    j_total = sum(j for _f, j, _g in mixed)
    g_total = sum(g for _f, _j, g in mixed)
    print(f"\n  the mixed modules hold {j_total} jackal and {g_total} game "
          f"bodies.")
    print("  WATCH `MIXED`: when it is small, the split is cheap. It falls on "
          "its own as\n  bodies are promoted out of catch-all modules; it does "
          "not need to be forced.")
