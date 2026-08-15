#!/usr/bin/env python3
"""Which compiler invocation built the shipped image, derived from the corpus.

`tools/byte_match.py` tries several flag sets per body and keeps the best
verdict. That is a search, and a search is what you do while you do not know
the answer - but the project now holds over 1,500 bodies that reproduce the
shipped bytes EXACTLY, which is a large labelled sample of "this invocation
produced these bytes". Asking each of them which invocations reproduce it
turns the search into a measurement.

WHAT THE ANSWER LOOKS LIKE. If one flag set reproduces every byte-exact body,
the image had one build configuration and the other sets are dead search
space - they can only cost time and admit false matches. If the corpus
partitions, the image really is mixed and each set has to earn its place with
the bodies only it explains.

THE BIAS, STATED UP FRONT. Every body here was AUTHORED against the current
search: an agent wrote a spelling, measured it across all the sets, and kept
what matched. So a body counted under `/O1` may be a body whose source form
was shaped until `/O1` matched, not evidence that its translation unit was
compiled `/O1`. That makes a set's exclusive count an UPPER bound on its
necessity, and it makes the headline number - how much of the corpus one set
alone explains - the honest one, because no amount of source-shaping inflates
that.

    tools/derive_build_flags.py                  # the whole byte-exact corpus
    tools/derive_build_flags.py --limit 150      # a random sample, same shape
    tools/derive_build_flags.py --candidates ... # try an invocation not listed

Everything here is OFFLINE except the compiler: it needs VC6 under Wine and
the pinned executable, the same as the gate.
"""

from __future__ import annotations

import argparse
import random
import sys
from collections import Counter
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import annotation_scan  # noqa: E402
import byte_match  # noqa: E402
import decomp_status as ds  # noqa: E402
import emit_translation_unit as emit  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent

# The invocations to ask about. Named rather than generated because each one
# is a HYPOTHESIS someone can argue with, and an unnamed cross product is a
# table nobody reads. A trailing space keeps each distinct from the members of
# `byte_match.FLAG_SETS`, which `match_functions` expands to the whole search
# rather than honouring as a single set.
CANDIDATES = (
    ("/O2",            "/c /O2 /Gy /GR- /Oy- /GX "),
    ("/O2 frameless",  "/c /O2 /Gy /GR- /GX "),
    ("/O2 /Oi-",       "/c /O2 /Oi- /Gy /GR- /Oy- /GX "),
    ("/O2 /Oi- frameless", "/c /O2 /Oi- /Gy /GR- /GX "),
    ("/O1",            "/c /O1 /Gy /GR- /Oy- /GX "),
    ("/O1 frameless",  "/c /O1 /Gy /GR- /GX "),
    ("/O2 /Os",        "/c /O2 /Os /Gy /GR- /Oy- /GX "),
    ("/Ox",            "/c /Ox /Gy /GR- /Oy- /GX "),
)


def byte_exact_units(limit: int, seed: int):
    """[(address, unit)] for every body carrying a BYTE_EXACT claim."""
    import pefile
    annotations = [a for a in annotation_scan.scan_tree(REPO_ROOT / "src")
                   if a.matched]
    if limit and limit < len(annotations):
        random.Random(seed).shuffle(annotations)
        annotations = annotations[:limit]
    functions = emit.load_functions()
    pe_fast = pefile.PE(str(byte_match.DEFAULT_EXE), fast_load=True)
    units, refusals = ds.build_units(
        annotations, {a.address: functions.get(a.address) for a in annotations
                      if functions.get(a.address)},
        functions, emit.load_derived(), emit.load_callees(), pe_fast)
    return units, refusals, functions, pe_fast


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--limit", type=int, default=0,
                        help="score a random sample of this many bodies")
    parser.add_argument("--seed", type=int, default=11)
    parser.add_argument("--candidates", action="append", default=[],
                        help="an extra invocation to test, e.g. '/c /O2 /Gy'")
    arguments = parser.parse_args()

    reason = byte_match.available()
    if reason:
        print(f"SKIP: {reason}")
        return 0

    units, refusals, functions, pe = byte_exact_units(arguments.limit,
                                                      arguments.seed)
    if not units:
        print("derive-build-flags: no byte-exact bodies could be built, so "
              "this measured NOTHING", file=sys.stderr)
        return 1

    shared = byte_match.shared_spans(functions) \
        if hasattr(byte_match, "shared_spans") else set()
    candidates = list(CANDIDATES) + [(flags.strip(), flags.rstrip() + " ")
                                     for flags in arguments.candidates]

    subjects = [(address, unit, f"d{address:08x}")
                for address, unit in sorted(units.items())]
    matched_by = {address: set() for address, _, _ in subjects}
    for label, flags in candidates:
        outcomes = byte_match.match_functions(pe, functions, shared, subjects,
                                              flags)
        hit = 0
        for address, outcome in outcomes.items():
            if outcome.get("tier") == "BYTE_EXACT":
                matched_by[address].add(label)
                hit += 1
        print(f"  {label:22} reproduces {hit:5} / {len(subjects)}")

    print(f"\n{len(subjects)} byte-exact bodies scored"
          f"{f', {len(refusals)} could not be built' if refusals else ''}")
    combos = Counter(tuple(sorted(v)) for v in matched_by.values())
    print("\nwhich invocations reproduce each body:")
    for combo, count in combos.most_common(12):
        print(f"  {count:5}  {', '.join(combo) if combo else '(none)'}")

    print("\nbodies only ONE invocation explains:")
    exclusive = Counter(next(iter(v)) for v in matched_by.values()
                        if len(v) == 1)
    for label, count in exclusive.most_common():
        print(f"  {count:5}  {label}")
    unexplained = sum(1 for v in matched_by.values() if not v)
    if unexplained:
        print(f"\n{unexplained} body reproduce under NO candidate, which means "
              f"the search that banked them is not in this table")
    return 0


if __name__ == "__main__":
    sys.exit(main())
