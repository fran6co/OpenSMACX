#!/usr/bin/env -S uv run python
"""Comparisons that can never be true, given the loop they sit in.

`map.cpp`'s 0x005C55C0 carried `if (i >= 32)` and `i == 32` inside a loop
bounded `i < 20`. Neither could ever be taken. The cause was a misread
constant: the image's `cmp esi, 0x20` is the loop counter STRENGTH-REDUCED by
four for `RadiusOffsetX/Y` addressing, so the real threshold is 8, and the
recovery had transcribed the scaled value as the unscaled one.

That is a LOGIC defect, not a matching one - the recovered program contained a
branch the game does not have - and no similarity score points at it. This
finds the shape statically.

    uv run tools/dead_branches.py

Reports `for (T i = 0; i < BOUND; ...)` loops whose body compares `i` against a
constant at or beyond BOUND. Both must be literals; anything computed is left
alone, because a bound that is a named constant may legitimately exceed the
comparison at some other call.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
LOOP = re.compile(
    r"for\s*\(\s*(?:int|unsigned|uint32_t|size_t|long)\s+(\w+)\s*=\s*0\s*;"
    r"\s*\1\s*<\s*(\d+)\s*;")


def blanked(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", lambda m: " " * len(m.group(0)), text, flags=re.S)
    return re.sub(r"//[^\n]*", lambda m: " " * len(m.group(0)), text)


def body_of(text: str, start: int) -> tuple[int, int] | None:
    """Span of the braced block following the `for` header at `start`."""
    open_brace = text.find("{", start)
    if open_brace < 0 or open_brace - start > 200:
        return None
    depth = 0
    for i in range(open_brace, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return (open_brace, i)
    return None


if __name__ == "__main__":
    found = 0
    for path in sorted((REPO_ROOT / "src").glob("*.cpp")):
        raw = path.read_text(errors="replace")
        text = blanked(raw)
        for loop in LOOP.finditer(text):
            name, bound = loop.group(1), int(loop.group(2))
            span = body_of(text, loop.end())
            if not span:
                continue
            body = text[span[0]:span[1]]
            # A nested `for` over the same name rebinds it; skip those.
            if re.search(rf"for\s*\([^;]*\b{name}\s*=", body):
                continue
            for cmp in re.finditer(
                    rf"\b{re.escape(name)}\s*(>=|>|==)\s*(\d+)\b", body):
                op, value = cmp.group(1), int(cmp.group(2))
                dead = (op in (">=", "==") and value >= bound) or \
                       (op == ">" and value >= bound - 1)
                if not dead:
                    continue
                line = raw.count("\n", 0, span[0] + cmp.start()) + 1
                found += 1
                print(f"  {path.name}:{line}  `{name} {op} {value}` inside "
                      f"`for (... {name} < {bound}; ...)` - never true")
    print(f"\n{found} comparison(s) that cannot be taken")
