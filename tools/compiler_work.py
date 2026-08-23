#!/usr/bin/env -S uv run python
r"""Work this tree does by hand that the COMPILER should be doing.

Every large gain on 2026-08-22 came from deleting something, not writing
something: a hand-installed vtable, a hand-written adjustor thunk, an explicit
base-destructor call, a `construct()` method standing in for a constructor.
None of them was a near-miss to grind. Each was the tree doing a job VC6 does
better, and doing it in a place VC6 would not have put it.

WHY IT IS A CORRECTNESS PROBLEM AND NOT A STYLE ONE. The compiler emits these
constructs in a fixed ORDER relative to everything else it generates, and that
order is part of the bytes:

  * A class that stores its own vtable dword gets NO compiler vtable, and
    therefore none of the adjustor thunks that live in one. That single fact
    made 46 deleting thunks look blocked.
  * MSVC assigns the vfptr AFTER the member-initialiser list. Stores written in
    the constructor BODY land after it instead of before, and Win::Win() sat at
    99 of 107 until they moved into the list.
  * In a destructor the vptr restores come BEFORE the EH-state store.
    FlatButton's hand-written ones landed between them, and its own lesson
    blamed "VC6's own EH-state scheduling" rather than the hand-writing.
  * An explicit `Base::destroy()` duplicates the base destructor the compiler
    already calls, so the body has one call too many.

A RATCHET PER SHAPE. These cannot all be fixed at once, and a permanently red
gate is one nobody reads. No shape may GROW. When one shrinks the tool says so
and asks for its ceiling to be lowered in the same commit, because a ratchet
with unclaimed slack is where the next regression hides.

    uv run tools/compiler_work.py            # the census, with what to do
    uv run tools/compiler_work.py --check    # exit 1 if any shape grew
"""
import argparse, collections, pathlib, re, sys

REPO = pathlib.Path(__file__).resolve().parent.parent

# Measured 2026-08-22 on a clean tree. Lower a ceiling when its count falls.
SHAPES = [
    ("vtable", 25,
     re.compile(r"""(?x)
        (?: \w+ \s* \[ \s* 0x[0-9A-Fa-f]+ \s* / \s* 4 \s* \]
          | \* \s* reinterpret_cast \s* < [^>]*? \* \s* > \s* \([^)]*\)
          | \w+ \s* \[ \s* \d+ \s* \] )
        \s* = \s* [A-Za-z_]\w*Vtable\w* \s* ;"""),
     "a class that stores its own vtable dword gets no compiler vtable, and no "
     "adjustor thunks in it. Give it a real constructor and delete the store."),

    ("construct", 11,
     re.compile(r"^\w[\w :*&]*\b\w+::construct\(\)\s*\{", re.M),
     "a `construct()` method standing in for a constructor. Convert it; put "
     "anything the image writes BEFORE the vfptr store in the member-"
     "initialiser list, and drop the trailing `return this;`."),

    ("free-function destructor", 10,
     re.compile(r"^\w[\w :*&]*\b\w*destructor_redirect\w*\s*\(", re.M),
     "a destructor modelled as a free function. Make it `X::~X()`. A free "
     "function is only needed where C++ cannot take a destructor's address."),

    ("explicit base construct/destroy", 12,
     re.compile(r"\b\w+::(?:destroy|construct)\(\)\s*;"),
     "duplicates the base constructor or destructor the compiler already "
     "calls, so the body carries one call too many."),

    ("placement new on a subobject", 19,
     re.compile(r"new\s*\(\s*(?:&|static_cast|reinterpret_cast)"),
     "costs a null guard (`test ecx, ecx; je`) the image does not have. A real "
     "base or member is constructed unconditionally."),

    ("hand-walked vbtable", 14,
     re.compile(r"\*reinterpret_cast<(?:const )?int32_t \*(?:const )?\*>\([^)]*\)\)\[[12]\]"),
     "reads a virtual-base displacement out of the vbtable by hand. That is "
     "what `public virtual` makes the compiler do."),

    ("VCall shim", 21,
     re.compile(r"\bVCall\b"),
     "dispatches through a fake class because the real function is not "
     "declared virtual. Declare it."),

    # ONE SHAPE UNTIL 2026-08-23, and one shape was the mistake: the remedy
    # decides the split, not the syntax. Both spell `ORIGINAL(`, but a
    # vtable-slot dispatch is cured by DECLARING THE METHOD VIRTUAL (it is the
    # VCall shim's defect wearing different clothes), while a named-pointer
    # seam is cured by declaring the method and forwarding it from
    # pending_bodies.cpp. A single ratchet let one grow while the other
    # shrank, invisibly. The two regexes classify by what follows on the
    # LINE - the census is line-wise, so a lookahead cannot leak across
    # statements.
    ("ORIGINAL() vtable-slot dispatch", 70,
     re.compile(r"\bORIGINAL\((?=.*vtable\[)"),
     "reads the target out of a vtable slot by hand because the method is not "
     "declared virtual. Declare it - the compiler writes this dispatch."),

    ("ORIGINAL() named-pointer seam", 121,
     re.compile(r"\bORIGINAL\((?!.*vtable\[)"),
     "reaches a member through a function-pointer slot, which compiles "
     "`FF 15` where the image has `E8`. Call it by name once its body lands."),
]


def census(root):
    r"""Counted LINE BY LINE, and comment lines are skipped.

    Both halves matter. Prose describing a shape is not an instance of it - the
    explanations in this file would otherwise count themselves. And joining the
    file into one string lets `\s*` inside a pattern span newlines, which
    silently found five matches that do not exist: 66 against the 61 the
    line-wise count agrees on.
    """
    counts = collections.Counter()
    files = collections.defaultdict(set)
    for path in sorted(root.rglob("*.cpp")):
        if "recovered" in path.parts or "unrecovered" in path.parts:
            continue
        for line in path.read_text(errors="replace").splitlines():
            if line.lstrip().startswith("//"):
                continue
            for name, _ceiling, rx, _why in SHAPES:
                n = len(rx.findall(line))
                if n:
                    counts[name] += n
                    files[name].add(path.name)
    return counts, files


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--check", action="store_true")
    ap.add_argument("--root", type=pathlib.Path, default=REPO / "src",
                    help="scan elsewhere; used to positive-control --check")
    args = ap.parse_args()

    counts, files = census(args.root)
    grew, shrank = [], []
    for name, ceiling, _rx, why in SHAPES:
        n = counts[name]
        if n > ceiling:
            grew.append((name, n, ceiling))
        elif n < ceiling:
            shrank.append((name, n, ceiling))
        if not args.check:
            flag = "GREW" if n > ceiling else "down" if n < ceiling else "    "
            print(f"  {flag} {n:4d}/{ceiling:<4d} {name}")
            print(f"           {why}")
            print(f"           {len(files[name])} file(s)")

    total = sum(counts.values())
    if grew:
        for name, n, ceiling in grew:
            print(f"COMPILER WORK GREW: {name} is {n}, above its ceiling of {ceiling}")
        return 1
    for name, n, ceiling in shrank:
        print(f"compiler work down: {name} is {n}, below its ceiling of "
              f"{ceiling} - lower it in this same commit")
    if not args.check:
        print(f"\n{total} site(s) doing the compiler's work, across "
              f"{len(SHAPES)} shapes")
    elif not shrank:
        print(f"compiler work: {total} site(s), no shape growing")
    return 0


if __name__ == "__main__":
    sys.exit(main())
