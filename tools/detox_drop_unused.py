#!/usr/bin/env -S uv run python
"""Scratch: drop the binding declarations nothing in this file uses."""
import pathlib
import re

P = pathlib.Path("src/netdaemon.cpp")
lines = P.read_text().splitlines(keepends=True)

decl = re.compile(r"^static int \*const (g_[0-9a-f]+) = \(int \*\)0x[0-9A-Fa-f]+;\n")
dropped = 0
for i, line in enumerate(lines):
    m = decl.match(line)
    if not m:
        continue
    name = m.group(1)
    uses = sum(1 for j, other in enumerate(lines)
               if j != i and re.search(rf"\b{name}\b", other))
    if uses == 0:
        lines[i] = ""
        dropped += 1

P.write_text("".join(lines))
print(f"dropped {dropped} unused binding(s)")
left = sorted({m for m in re.findall(r"\bg_[0-9a-f]{6,8}\b", "".join(lines))})
print(f"{len(left)} anonymous identifier(s) left")
