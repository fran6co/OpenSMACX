#!/usr/bin/env -S uv run python
r"""Semantic debt: what a body can carry while measuring BYTE_EXACT.

The byte ratchet proves the COMPILED BYTES reproduce the image. It is
structurally blind to everything that lowers identically: a method named
UNK2, an `operator=` spelled `__as` returning `int`, a dead `*_redirect`
wrapper nothing calls, a function address smuggled through a `void *` data
global. All of it measured exact in palette.cpp on 2026-08-24, because the
bytes WERE exact - the lies live in the source model, not the object file.

Five shapes, counted like compiler_work.py: line-wise over the product tree
(src/*.cpp + *.h; the artifact archives are excluded - they die by homing,
not by cleanup), comment lines skipped, a ceiling per shape that only falls.
`--check` is wired into the gate; the class pass (recover-class.md pass 6)
is what lowers the numbers, and a class is DONE only at debt 0.

    uv run tools/class_debt.py            # the census, with what to do
    uv run tools/class_debt.py --check    # exit 1 if any shape grew
    uv run tools/class_debt.py --json     # per-file counts, for --by-class
"""
from __future__ import annotations

import collections
import json
import re
import sys
from bisect import bisect_right
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO))

from decomp import read  # noqa: E402

# Measured 2026-08-24 on a clean tree at 09c18640. Lower a ceiling when its
# count falls - the gate fails on slack, same as compiler_work. The zero is
# not decorative: 284 trivial BYTE_EXACT claims were scanned and every one
# carries Purpose/Verification prose (thunk families document per-file);
# the ceiling holds that at zero. All 66 function-address bindings were
# eyeballed before pinning - vector-dtor iterators, atexit callbacks, and
# `g_00406850`-style disguises, every one a real function reference.
CEILINGS = {
    "unk-method": 286,
    "function-address binding": 65,
    "orphan redirect": 891,
    "pointer-as-int": 2,
    "undocumented trivial body": 0,
}

WHY = {
    "unk-method":
        "a method named UNKn is a body whose behaviour nobody wrote down. "
        "Name it from evidence: callers, the image's data, "
        "docs/recovery/behaviour-member-names.csv.",
    "function-address binding":
        "a cast of an address that lands INSIDE a catalogued function - a "
        "function reference wearing a data global's clothes. Declare the "
        "callee (forward it in pending_bodies.cpp if unpromoted) and delete "
        "the global.",
    "orphan redirect":
        "a *_redirect wrapper referenced only by its own declaration and "
        "definition. Delete it; the gate's link step is the proof nothing "
        "non-textual needed it.",
    "pointer-as-int":
        "a pointer travelling as int - `return reinterpret_cast<int>(this)`, "
        "`return (int)new`, or an invented operator name (__as is operator=). "
        "Same bytes, wrong model; fix the type and re-measure.",
    "undocumented trivial body":
        "a BYTE_EXACT claim on a <=4-byte body with no Purpose:/Verification "
        "note prose. The claim is honest - the image really is that trivial - "
        "but nothing SAYS so, and it reads as a stub.",
}

UNK = re.compile(r"\bUNK\d+\b")
ADDRESS_CAST = re.compile(
    r"(?:\(\s*[A-Za-z_][\w :]*?\*\s*\)|reinterpret_cast<[^>]*\*\s*>\s*\()"
    r"\s*0x(00[0-9A-Fa-f]{6})\b")
REDIRECT = re.compile(r"\b\w+_redirect\b")
POINTER_AS_INT = re.compile(
    r"return\s+reinterpret_cast<\s*int\s*>\s*\(\s*this\s*\)"
    r"|return\s*\(\s*int\s*\)\s*new\b"
    r"|\b__(?:as|ct|dt)\b")
DOCUMENTED = re.compile(r"Purpose:|Verification note", re.I)


def product_files() -> list[Path]:
    return sorted(REPO.glob("src/*.cpp")) + sorted(REPO.glob("src/*.h"))


def code_lines(path: Path):
    """Non-comment lines - and that includes /* */ INTERIORS. The first
    version skipped only `//`-prefixed lines, so prose inside a Purpose
    block ("the catalogue's UNK3 stays on the marker") counted as an
    unnamed method. History must not be renamed to appease a census; the
    census must know history when it sees it."""
    in_block = False
    for line in path.read_text(errors="replace").splitlines():
        s = line.lstrip()
        if in_block:
            if "*/" in s:
                in_block = False
            continue
        if s.startswith(("//", "*")):
            continue
        if s.startswith("/*"):
            if "*/" not in s:
                in_block = True
            continue
        yield line


def census():
    """counts per shape, files per shape, per-file detail for --json."""
    records = read(REPO / "src")

    # The catalogued function spans, as a bisectable interval list. An
    # address cast that lands inside one is a FUNCTION reference, whatever
    # the cast says.
    spans = sorted((start, end) for r in records if r.address
                   for start, end in (r.image_spans or ()))
    starts = [s for s, _ in spans]

    def in_function(address: int) -> bool:
        i = bisect_right(starts, address) - 1
        return i >= 0 and spans[i][0] <= address < spans[i][1]

    counts = collections.Counter()
    files = collections.defaultdict(collections.Counter)
    redirect_refs = collections.Counter()

    for path in product_files():
        for line in code_lines(path):
            for name in UNK.findall(line):
                counts["unk-method"] += 1
                files["unk-method"][path.name] += 1
            for hexpart in ADDRESS_CAST.findall(line):
                if in_function(int("0x" + hexpart, 16)):
                    counts["function-address binding"] += 1
                    files["function-address binding"][path.name] += 1
            for name in REDIRECT.findall(line):
                redirect_refs[name] += 1
            if POINTER_AS_INT.search(line):
                counts["pointer-as-int"] += 1
                files["pointer-as-int"][path.name] += 1

    # Orphans need the whole tree's reference counts first; attribute each to
    # the file that DEFINES it (the one place `name(` appears at line start
    # is unknowable line-wise, so attribute to every file that mentions it -
    # the per-file numbers guide the class pass, the total is the ratchet).
    orphans = {name for name, n in redirect_refs.items() if n <= 2}
    if orphans:
        for path in product_files():
            for line in code_lines(path):
                for name in REDIRECT.findall(line):
                    if name in orphans:
                        counts["orphan redirect"] += 1
                        files["orphan redirect"][path.name] += 1
    # Each orphan was counted once per mention (<=2); normalise to one per
    # redirect so the ceiling is "orphan redirects", not "mentions".
    counts["orphan redirect"] = len(orphans)

    # Trivial bodies come from the records, not from lines.
    for r in records:
        if not r.byte_exact or (r.size or 99) > 4:
            continue
        rel = str(r.path)
        if "/recovered/" in rel or "/unrecovered/" in rel:
            continue
        text = r.path.read_text(errors="replace").splitlines()
        window = "\n".join(text[max(0, r.line - 30):r.line + 3])
        preamble = "\n".join(text[:40])
        if not DOCUMENTED.search(window) and not DOCUMENTED.search(preamble):
            counts["undocumented trivial body"] += 1
            files["undocumented trivial body"][r.path.name] += 1

    return counts, files


def main() -> int:
    check = "--check" in sys.argv
    as_json = "--json" in sys.argv
    counts, files = census()

    if as_json:
        print(json.dumps({shape: dict(files[shape]) for shape in CEILINGS},
                         indent=2))
        return 0

    grew, shrank = [], []
    for shape, ceiling in CEILINGS.items():
        n = counts[shape]
        if n > ceiling:
            grew.append((shape, n, ceiling))
        elif n < ceiling:
            shrank.append((shape, n, ceiling))
        if not check:
            flag = "GREW" if n > ceiling else "down" if n < ceiling else "    "
            print(f"  {flag} {n:4d}/{ceiling:<4d} {shape}")
            print(f"           {WHY[shape]}")
            print(f"           {len(files[shape])} file(s)")

    total = sum(counts[s] for s in CEILINGS)
    scanned = len(product_files())
    if grew:
        for shape, n, ceiling in grew:
            print(f"SEMANTIC DEBT GREW: {shape} is {n}, above its ceiling "
                  f"of {ceiling}")
        return 1
    if shrank:
        for shape, n, ceiling in shrank:
            print(f"semantic debt down: {shape} is {n}, below its ceiling of "
                  f"{ceiling} - lower it in this same commit")
        return 1 if check else 0
    print(f"{'semantic debt: ' if check else ''}{total} site(s) across "
          f"{len(CEILINGS)} shapes, {scanned} product file(s) scanned"
          f"{', every ceiling exact' if check else ''}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
