#!/usr/bin/env -S uv run python
"""Vtables installed by hand, which this tree does not want and keeps growing.

A class here sometimes models its vtable as a raw dword written through the
object:

    uint32_t *const object = reinterpret_cast<uint32_t *>(this);
    object[0x000 / 4] = PopupPrimaryVtable;
    object[0x444 / 4] = PopupBufferVtable;

THE OBJECTION IS NOT STYLE. A hand-stored vtable dword means the compiler
emits no vtable for that class - and with no vtable there is nowhere to put
the adjustor thunks that belong in it. That is measured: all 46
`??_G<Class>@@WEEE@AEPAXI@Z` deleting thunks are unrecovered, and converting
GraphicWin's `Buffer` member into a base produced ZERO of them, because the
hand-installed vtable was the blocker all along. Installing the vtable by hand
suppresses the very code the recovery is trying to reach.

IT IS A CHECK BECAUSE SAYING IT DID NOT WORK. This rule has been given more
than once in prose and the population still grew. A rule a person has to
remember is not a rule this tree enforces; the gate is.

A RATCHET, NOT A CLIFF. 54 sites exist today, so failing on any occurrence
would just be a permanently red gate that everyone learns to ignore. The
number may not go UP. When it goes down, the tool says so and CEILING should
be lowered in the same commit - a ratchet with unclaimed slack is how a real
regression hides inside a number that still passes.

    uv run tools/handwritten_vtables.py           # list the sites
    uv run tools/handwritten_vtables.py --check   # exit 1 if the count grew
"""
import argparse, pathlib, re, sys

REPO = pathlib.Path(__file__).resolve().parent.parent
# Measured 2026-08-22 on a clean tree. Lower it when the count falls.
CEILING = 72

# A store of a `*Vtable` constant through an object pointer, in any of the
# spellings this tree has used.
STORE = re.compile(
    r"""(?x)
    (?: \w+ \s* \[ \s* 0x[0-9A-Fa-f]+ \s* /  \s* 4 \s* \]      # object[0x444 / 4]
      | \* \s* reinterpret_cast \s* < [^>]*? \* \s* > \s* \([^)]*\)  # *(T *)(...)
      | \w+ \s* \[ \s* \d+ \s* \]                              # object[0]
    )
    \s* = \s* [A-Za-z_]\w*Vtable\w* \s* ;""")


def sites():
    found = []
    for path in sorted(REPO.joinpath("src").rglob("*.cpp")):
        if "recovered" in path.parts or "unrecovered" in path.parts:
            continue
        for number, line in enumerate(path.read_text(errors="replace")
                                      .splitlines(), 1):
            if STORE.search(line):
                found.append((str(path.relative_to(REPO)), number, line.strip()))
    return found


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--check", action="store_true")
    ap.add_argument("--root", type=pathlib.Path, default=None,
                    help="scan somewhere else; used to positive-control --check")
    args = ap.parse_args()

    global REPO
    if args.root:
        REPO = args.root.resolve()
    found = sites()

    if not args.check:
        by_file = {}
        for f, n, line in found:
            by_file.setdefault(f, []).append((n, line))
        for f in sorted(by_file):
            print(f"  {f}")
            for n, line in by_file[f]:
                print(f"      {n}: {line[:96]}")
        print()

    n = len(found)
    if n > CEILING:
        print(f"HAND-INSTALLED VTABLES: {n}, above the ceiling of {CEILING}. "
              f"A class that stores its own vtable dword gets no compiler "
              f"vtable, and no adjustor thunks in it.")
        return 1
    if n < CEILING:
        print(f"hand-installed vtables: {n}, BELOW the ceiling of {CEILING} - "
              f"lower CEILING in tools/handwritten_vtables.py to {n} in this "
              f"same commit, or the slack will hide the next regression")
        return 0
    print(f"hand-installed vtables: {n}, at the ceiling of {CEILING}; not growing")
    return 0


if __name__ == "__main__":
    sys.exit(main())
