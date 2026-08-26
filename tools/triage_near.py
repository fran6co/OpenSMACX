#!/usr/bin/env python3
"""Separate a TRACTABLE near-miss from a SCHEDULING wall, mechanically.

`osmx measure` scores agreement POSITIONALLY, so one extra instruction early
in a body drops the score for everything after it. A body reported as "4 of
14" can be one `xor edx, edx` from BYTE_EXACT. That misreading cost an
evening on 2026-08-26: seventeen ??_G bodies read as structurally blocked,
acquired an elaborate vtable theory, and turned out to be five copies of a
defect already fixed that same session.

The two cases look IDENTICAL to `measure` and are told apart only by the
diff:

  ALIGNMENT SLIP - tractable.  Counts differ by one or two, and listing_diff
      shows ONE differing run. Something extra is emitted early and
      everything after it shifts. Eight bodies fell to this in one evening.

  SCHEDULING - a wall.  Counts MATCH, the instructions match, and one is
      displaced by a position or two. The allocator chose the order and no
      source spelling reaches it. add_lf was measured at four spellings, all
      8 of 10 or worse.

Reads listing_diff's own output rather than re-deriving it, so the two tools
cannot drift apart about what a run is.
"""
from __future__ import annotations
import argparse, re, subprocess, sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
COUNTS = re.compile(r"image (\d+) instruction\(s\), this tree (\d+)")
RUNS = re.compile(r"(\d+) differing run\(s\)")


def triage(addr: str) -> tuple[str, str]:
    r = subprocess.run([sys.executable, str(REPO / "tools" / "listing_diff.py"), addr],
                       capture_output=True, text=True, cwd=REPO)
    out = r.stdout
    c, runs = COUNTS.search(out), RUNS.search(out)
    if not c or not runs:
        return "UNKNOWN", "listing_diff said nothing this can read"
    image, ours, n = int(c.group(1)), int(c.group(2)), int(runs.group(1))
    delta = ours - image
    if delta == 0 and n <= 3:
        return "SCHEDULING", (f"counts match at {image}; {n} run(s) displaced. "
                              f"The allocator chose the order - measure a spelling "
                              f"or two, then record the wall.")
    if 0 < delta <= 2 and n == 1:
        return "SLIP", (f"we emit {delta} instruction(s) too many in ONE run. "
                        f"Read that run: an extra `xor edx, edx` from a nullptr "
                        f"argument, or a frame the image does not build.")
    if delta < 0:
        return "MISSING", (f"we emit {-delta} FEWER instruction(s) than the image "
                           f"in {n} run(s) - a call or a store the body never makes.")
    # THE THUNK SHAPE, measured across adjustor_thunks.cpp and
    # deleting_thunks.cpp on 2026-08-26: the image TAIL-JUMPS where we push
    # the argument and call. The receiver adjustment already matches; what
    # differs is the forwarder's OWN signature, so no rewriting of the body
    # reaches it. Calling these "not a near miss" was wrong - they are two
    # instructions away and still unreachable, which is worth saying exactly.
    if "jmp " in out and "call " in out and 0 < delta <= 4:
        return "THUNK", (f"{delta:+d} instruction(s): the image TAIL-JUMPS where we "
                         f"push and call. That is the forwarder's own signature, not "
                         f"its statements - declaration work, not a byte grind.")
    return "WIDE", f"{n} differing run(s), {delta:+d} instruction(s): not a near miss."


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("addresses", nargs="+")
    args = ap.parse_args()
    for a in args.addresses:
        verdict, why = triage(a)
        print(f"  {a}  {verdict:<10} {why}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
