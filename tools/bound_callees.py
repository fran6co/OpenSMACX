#!/usr/bin/env -S uv run python
"""Callees bound as pointers, which cost every call site the image's `E8`.

A binding of the shape

    typedef int func13(int, int, BOOL);
    func13 *const tech_val_OG = (func13 *)0x005BCBE0;

compiles `call dword ptr [tech_val_OG]` at every call site, where the image
emits `call rel32`. The fix is not to name the image's symbol - declare the
callee as an ordinary function and forward it in `src/pending_bodies.cpp`, and
the `E8`'s target is a relocation on both sides and is discounted.

This finds them: function-TYPED `*const` bindings to an address, cross-checked
against whether anything actually calls the name. Worth 202 claims in one
change on the vector iterators, and the whole message-pump family after.

    uv run tools/bound_callees.py

A binding nothing calls is NOT reported - `tech_val_OG` is deliberate, an A/B
harness comparing the recovered `tech_val` against the original, and a tool
that flags it teaches the reader to ignore this tool.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
SRC = REPO_ROOT / "src"

# `typedef <ret> <name>(<params>);` - a function TYPE, not a pointer typedef.
TYPEDEF = re.compile(
    r"^\s*typedef\s+[\w \*&:]+?\b(\w+)\s*\((?!\s*\*)[^;]*\)\s*;", re.M)
# `<type> *const <name> = (<type> *)0xADDR;`
BINDING = re.compile(
    r"^\s*(\w+)\s*\*\s*const\s+(\w+)\s*=\s*\(\s*\1\s*\*\s*\)\s*(0x[0-9A-Fa-f]+)\s*;",
    re.M)


def code(text: str) -> str:
    """The file with comments blanked, so a mention in prose is not a call."""
    text = re.sub(r"/\*.*?\*/", lambda m: " " * len(m.group(0)), text, flags=re.S)
    return re.sub(r"//[^\n]*", "", text)


if __name__ == "__main__":
    sources = sorted(SRC.glob("*.h")) + sorted(SRC.glob("*.cpp"))
    bodies = {path: code(path.read_text(errors="replace")) for path in sources}

    function_types: set[str] = set()
    for text in bodies.values():
        function_types.update(TYPEDEF.findall(text))

    found = []
    for path, text in bodies.items():
        for typename, name, address in BINDING.findall(text):
            if typename not in function_types:
                continue
            # A USE, NOT ONLY A CALL. `\b{name}\s*\(` finds `foo(...)` and
            # misses `start(foo, 1, 150, 150)` - a bound pointer PASSED AS AN
            # ARGUMENT, which costs the identical `mov reg,[mem]; push reg`
            # where the image has `push imm`. Four such bindings sat in temp.h
            # with a live use in time.cpp and this reported "0 bound callee(s)
            # with call sites" both before and after they were fixed, because
            # the only use was an argument. A checker that cannot see the
            # cheapest form of the defect reads as a clean tree.
            use = rf"\b{name}\b(?!\s*[=;])"
            callers = sorted(
                other.name for other, body in bodies.items()
                if other != path and re.search(use, body))
            if re.search(use, text.replace(
                    f"{typename} *const {name}", "")):
                callers.append(path.name)
            if callers:
                found.append((path.name, name, address, sorted(set(callers))))

    for where, name, address, callers in sorted(found):
        print(f"  {address}  {name}")
        print(f"      bound in {where}, called from {', '.join(callers)}")
    print(f"{len(found)} bound callee(s) with call sites")
    sys.exit(0)
