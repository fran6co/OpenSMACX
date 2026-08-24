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
    ("ORIGINAL() vtable-slot dispatch", 38,
     re.compile(r"\bORIGINAL\((?=.*vtable\[)"),
     "reads the target out of a vtable slot by hand because the method is not "
     "declared virtual. Declare it - the compiler writes this dispatch."),

    ("ORIGINAL() named-pointer seam", 121,
     re.compile(r"\bORIGINAL\((?!.*vtable\[)"),
     "reaches a member through a function-pointer slot, which compiles "
     "`FF 15` where the image has `E8`. Call it by name once its body lands."),
]

# SCANNED OVER .cpp AND .h TOGETHER, which is why they are a second list: the
# members live in HEADERS, and turning the whole census loose on headers
# inflates three existing shapes with DECLARATIONS (free-function destructor
# 10 -> 20, VCall 21 -> 23, named-pointer seam 121 -> 123 - measured
# 2026-08-23). Same ratchet, narrower lens.
HEADER_SHAPES = [
    ("vtable-as-member", 11,
     re.compile(r"\b[A-Za-z_]\w*\s+vtable\w*\s*;"),
     "a vtable pointer spelled as a data member is a base class that has "
     "not been declared. Declare the base whose vfptr lives at that offset "
     "- or the class's own virtuals - and let the compiler own slot 0."),
    ("vtable-initialiser store", 1,
     re.compile(r"\w+\s*\(\s*[A-Za-z_]\w*Vtable\w*\s*\)"),
     "a base installed by an initialiser-list store is a base that has not "
     "been declared (Win's second base at 0xC8). Declare it."),
    ("vtable address constant", 78,
     re.compile(r"\b[A-Za-z_]\w*Vtable\w*\s*=\s*\(?\s*0x"),
     "the raw material every hand-installed vtable is built from. When the "
     "classes are real, these constants have nothing left to point at."),
]

# THE SCAFFOLD RATCHET. Twelve files exist because a body had no real home or
# the tree did by hand what a faithful class makes the compiler do; the
# recovery includes their retirement, so their sizes are ceilings like any
# shape's. A scaffold at zero is deleted in its own commit. Measured
# 2026-08-23 on a clean tree at 2bbecdbd.
SCAFFOLD_MARKERS = ("init_thunks.cpp", "atexit_thunks.cpp",
                    "adjustor_thunks.cpp", "deleting_thunks.cpp",
                    "delegation_thunks.cpp", "field_accessors.cpp",
                    "leaf_recoveries.cpp", "nullsub_thunks.cpp",
                    "guarded_teardowns.cpp")
SCAFFOLD_CEILINGS = {
    "init_thunks.cpp markers": 395,
    "atexit_thunks.cpp markers": 373,
    "adjustor_thunks.cpp markers": 116,
    "deleting_thunks.cpp markers": 74,
    "delegation_thunks.cpp markers": 27,
    "field_accessors.cpp markers": 42,
    "leaf_recoveries.cpp markers": 53,
    "nullsub_thunks.cpp markers": 56,
    "guarded_teardowns.cpp markers": 25,
    "PENDING_BODY forwarders": 235,
    "artifact files (recovered/)": 1391,
    "unrecovered files": 1753,
    "hypothesis_layouts.h lines": 2709,
}


def scaffold_census():
    """The scaffold sizes, counted live against the ceilings above."""
    counts = {}
    for name in SCAFFOLD_MARKERS:
        text = (REPO / "src" / name).read_text(errors="replace")
        counts[f"{name} markers"] = len(re.findall(r"ORIGINAL: 0x", text))
    pending = (REPO / "src" / "pending_bodies.cpp").read_text(errors="replace")
    counts["PENDING_BODY forwarders"] = len(
        re.findall(r"PENDING_BODY\(", pending))
    counts["artifact files (recovered/)"] = sum(
        1 for _ in (REPO / "src" / "recovered").rglob("*.cpp"))
    counts["unrecovered files"] = sum(
        1 for _ in (REPO / "src" / "unrecovered").glob("*.cpp"))
    counts["hypothesis_layouts.h lines"] = sum(
        1 for _ in (REPO / "src" / "hypothesis_layouts.h").open())
    return counts


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
            # SHAPES are .cpp-only BY MEASUREMENT: loosing them on headers
            # counts DECLARATIONS as sites (free-function destructor alone
            # went 10 -> 20). HEADER_SHAPES were measured over BOTH.
            for name, _ceiling, rx, _why in SHAPES + HEADER_SHAPES:
                n = len(rx.findall(line))
                if n:
                    counts[name] += n
                    files[name].add(path.name)
    for path in sorted(root.glob("*.h")):
        if "recovered" in path.parts or "unrecovered" in path.parts:
            continue
        for line in path.read_text(errors="replace").splitlines():
            if line.lstrip().startswith("//"):
                continue
            for name, _ceiling, rx, _why in HEADER_SHAPES:
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
    for name, ceiling, _rx, why in SHAPES + HEADER_SHAPES:
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

    scaffold_grew, scaffold_shrank = [], []
    scaffolds = scaffold_census()
    for name, ceiling in SCAFFOLD_CEILINGS.items():
        n = scaffolds.get(name, 0)
        if n > ceiling:
            scaffold_grew.append((name, n, ceiling))
        elif n < ceiling:
            scaffold_shrank.append((name, n, ceiling))
        if not args.check:
            flag = ("GREW" if n > ceiling
                    else "down" if n < ceiling else "    ")
            print(f"  {flag} {n:4d}/{ceiling:<4d} {name}  [scaffold]")
            print(f"           retired by homing and the class passes; at zero"
                  f" it is deleted in its own commit")

    total = sum(counts.values())
    if grew or scaffold_grew:
        for name, n, ceiling in grew + scaffold_grew:
            print(f"COMPILER WORK GREW: {name} is {n}, above its ceiling of {ceiling}")
        return 1
    # SLACK IS A REGRESSION TOO, not advice. A ceiling above its count is
    # re-fillable space: the next regression hides inside it exactly as it
    # would above a stale total, which is how one unit of drift went
    # unnoticed for the whole day a ratchet existed (finding 4, 2026-08-23:
    # artifact files sat at 1395 under a 1396 ceiling from birth). A count
    # below its ceiling means an improvement landed WITHOUT claiming its
    # ratchet - so every improvement commit must set the ceiling from its
    # own measured number, and anything else fails here.
    if shrank or scaffold_shrank:
        for name, n, ceiling in shrank + scaffold_shrank:
            kind = "scaffold" if name in SCAFFOLD_CEILINGS else "compiler work"
            print(f"{kind} SLACK: {name} is {n}, below its ceiling of "
                  f"{ceiling} - set the ceiling to {n} in this same commit")
        return 1
    if not args.check:
        print(f"\n{total} site(s) doing the compiler's work, across "
              f"{len(SHAPES) + len(HEADER_SHAPES)} shapes; "
              f"{len(SCAFFOLD_CEILINGS)} scaffold ceilings watched")
    else:
        print(f"compiler work: {total} site(s), every ceiling exact")
    return 0


if __name__ == "__main__":
    sys.exit(main())
