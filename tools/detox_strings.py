#!/usr/bin/env -S uv run python
"""Scratch: turn the daemon's string-table bindings into literals.

The 0x0068xxxx / 0x0073xxxx slots are .rdata string literals - log_say
captions and NetMsg::pop labels. The image pushes their address; a literal in
this build is the same kind of relocated immediate, so the honest spelling is
the text, not a named global. Binding DECLARATION lines are dropped, never
rewritten.
"""
import pathlib
import re
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import image_data

P = pathlib.Path("src/netdaemon.cpp")
text = P.read_text()
lines = text.splitlines(keepends=True)

addrs = sorted({int(m, 16) for m in re.findall(r"\bg_([0-9a-f]{6,8})\b", text)})
literal = {}
for va in addrs:
    if not (0x680000 <= va <= 0x760000):
        continue
    try:
        raw = image_data.read(image_data.DEFAULT_IMAGE, va, 160)
    except Exception:
        continue
    if not raw:
        continue
    end = raw.find(b"\0")
    if end <= 0:
        continue
    body = raw[:end]
    if not all(0x20 <= b < 0x7F or b in (9, 10, 13) for b in body):
        continue
    literal[va] = body.decode("latin-1")

changed = 0
for i, line in enumerate(lines):
    if line.lstrip().startswith("static int *const"):
        continue
    for va, s in sorted(literal.items()):
        esc = s.replace("\\", "\\\\").replace('"', '\\"')
        for gname in (f"g_{va:08x}", f"g_{va:06x}"):
            if gname not in line:
                continue
            new = re.sub(rf"\b{gname}\b", f'"{esc}"', line)
            if new != line:
                changed += 1
                line = new
    lines[i] = line

text = "".join(lines)
for va in sorted(literal):
    text = re.sub(rf"^static int \*const g_{va:x} = \(int \*\)0x[0-9A-Fa-f]+;\n",
                  "", text, flags=re.M)

P.write_text(text)
print(f"{changed} line(s) rewritten; {len(literal)} string address(es)")
left = sorted({m for m in re.findall(r"\bg_[0-9a-f]{6,8}\b", text)})
print(f"{len(left)} anonymous identifier(s) left")
