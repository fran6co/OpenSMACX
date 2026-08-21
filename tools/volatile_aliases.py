#!/usr/bin/env -S uv run python
"""Bodies reading an object through a `volatile` pointer alias.

`volatile uint32_t *ordered = reinterpret_cast<volatile uint32_t *>(this)`, read
at raw offsets, BLOCKS VC6's memory-operand folding. The image emits
`cmp dword ptr [esi + 0x28], edi` and `imul eax, dword ptr [esi + 0x10]`; an
aliased read forces a `mov` into a register first, one extra instruction at
every site.

It cost seven bodies on 2026-08-21. Five in `basebutton.cpp` - three
`set_text_color` overloads, `set_def_font` and `init` - went byte-exact the
moment `volatile Font **const fonts` came out in favour of independent absolute
lvalues. `Sprite::close` went 4 of 38 agreeing to 35 of 38 the same way, and
every offset it aliased already had a named member.

    uv run tools/volatile_aliases.py

Reports each alias with the body that holds it and whether that body is already
byte-exact, because a claimed body must NOT be touched on this evidence alone -
it reproduces, so whatever it does is what the image does.

WHY THE ALIAS IS USUALLY THERE: it came from a bulk emitter that had no member
names to use. Where the class now names those offsets, the fix is to use them;
where it does not, the fix is to name them first.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from decomp import read

REPO_ROOT = Path(__file__).resolve().parent.parent
ALIAS = re.compile(
    r"^\s*(?:const\s+)?volatile\s+[\w:]+\s*\*+\s*(?:const\s+)?(\w+)\s*=", re.M)


if __name__ == "__main__":
    records = sorted(read(REPO_ROOT / "src"), key=lambda r: (str(r.path), r.line))
    by_path: dict = {}
    for record in records:
        by_path.setdefault(record.path, []).append(record)

    total = claimed = 0
    for path, group in sorted(by_path.items(), key=lambda kv: str(kv[0])):
        if "/recovered/" in str(path) or "/unrecovered/" in str(path):
            continue
        try:
            lines = path.read_text(errors="replace").splitlines()
        except OSError:
            continue
        for match in ALIAS.finditer("\n".join(lines)):
            line_no = "\n".join(lines).count("\n", 0, match.start()) + 1
            # The record whose marker most closely precedes this line.
            owner = None
            for record in group:
                if record.line and record.line <= line_no:
                    owner = record
                else:
                    break
            total += 1
            flag = ""
            if owner is not None and owner.byte_exact:
                claimed += 1
                flag = "  [BYTE_EXACT - leave it alone]"
            name = owner.name if owner else "?"
            print(f"  {path.name}:{line_no}  {match.group(1):22s} "
                  f"{name}{flag}")
    print(f"\n{total} volatile alias(es); {claimed} sit in bodies that already "
          f"reproduce and must not be changed on this evidence")
