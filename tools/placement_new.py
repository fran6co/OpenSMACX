#!/usr/bin/env -S uv run python
"""Bodies compiling an SEH frame the image has no trace of.

A placement-new constructor call - `new (obj) Class(...)` - pulls in VC6's
unwind-protection scaffolding whenever the class has a NON-TRIVIAL DESTRUCTOR
and the inlined constructor calls a non-intrinsic function. The image's body
has no such frame, and the divergence starts at instruction 0, so it looks like
a total mismatch rather than one idiom.

`log_logging` was exactly this: `new (Logging) Log(...)` against an image body
with no frame at all. Replacing it with an ordinary `construct(LPCSTR)` method -
no `new`-expression - dropped the frame and fixed the call count with it. That is
the `Win::construct` idiom this tree already uses elsewhere.

    uv run tools/placement_new.py

Reports the placement-new sites, the body each sits in, and whether that body
already reproduces. A BYTE_EXACT body is doing what the image does and must be
left alone - the frame is evidently correct there, which is exactly the check
that stops this being applied blindly.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from decomp import read

REPO_ROOT = Path(__file__).resolve().parent.parent
SITE = re.compile(r"\bnew\s*\(\s*[^)]+\)\s*[A-Z]\w*")


if __name__ == "__main__":
    records = [r for r in read(REPO_ROOT / "src")
               if "/recovered/" not in str(r.path)
               and "/unrecovered/" not in str(r.path)]
    by_file: dict = {}
    for record in records:
        by_file.setdefault(record.path.name, []).append(record)
    for group in by_file.values():
        group.sort(key=lambda r: r.line or 0)

    total = claimed = 0
    for path in sorted((REPO_ROOT / "src").glob("*.cpp")):
        try:
            lines = path.read_text(errors="replace").splitlines()
        except OSError:
            continue
        for number, line in enumerate(lines, 1):
            if line.lstrip().startswith("//") or not SITE.search(line):
                continue
            owner = None
            for record in by_file.get(path.name, []):
                if record.line and record.line <= number:
                    owner = record
            total += 1
            flag = ""
            if owner is not None and owner.byte_exact:
                claimed += 1
                flag = "   [BYTE_EXACT - the frame is correct here]"
            print(f"  {path.name}:{number}  {(owner.name if owner else '?')}{flag}")
            print(f"      {line.strip()[:90]}")
    print(f"\n{total} placement-new site(s); {claimed} sit in bodies that "
          f"already reproduce")
