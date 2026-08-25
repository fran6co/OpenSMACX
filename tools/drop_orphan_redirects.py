#!/usr/bin/env python3
"""Delete `*_redirect` wrappers nothing references, for one class pair.

`class_debt`'s "orphan redirect" shape counts a wrapper referenced only by
its own declaration and definition. The remedy is to delete it, and the
gate's LINK STEP is the proof that nothing non-textual needed it.

    uv run tools/drop_orphan_redirects.py src/buffer.h src/buffer.cpp
    uv run tools/drop_orphan_redirects.py src/buffer.h src/buffer.cpp --apply

TWO REFUSALS, both learned by doing this by hand:

  - a redirect that carries an ORIGINAL marker is a CLAIMED BODY, not a
    wrapper. `wave_group_insert_redirect` was the BYTE_EXACT body for
    0x004C5BF0 while a PENDING_BODY forwarder jumped to the raw address
    beside it; deleting it would have dropped a claim and left the fault.
  - a redirect with more than its own two references is live. Buffer's
    close/close_class/construct/destructor each had three or six.

Counts references across the WHOLE tree, not the pair, because a redirect
can be called from another class's file.
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

REDIRECT = re.compile(r"\b[A-Za-z_][\w]*_redirect\b")


def references(name: str, root: Path) -> int:
    n = 0
    word = re.compile(r"\b" + re.escape(name) + r"\b")
    for path in list(root.rglob("*.cpp")) + list(root.rglob("*.h")):
        n += len(word.findall(path.read_text(errors="replace")))
    return n


def is_claimed(name: str, source: Path) -> bool:
    lines = source.read_text(errors="replace").split("\n")
    start = re.compile(r"^[A-Za-z_].*\b" + re.escape(name) + r"\b")
    for i, line in enumerate(lines):
        if not start.match(line):
            continue
        k = i - 1
        while k >= 0 and lines[k].lstrip().startswith("//"):
            if lines[k].lstrip().startswith("// ORIGINAL:"):
                return True
            k -= 1
    return False


def cut(path: Path, name: str, terminator: str, apply: bool) -> int:
    lines = path.read_text(errors="replace").split("\n")
    start = re.compile(r"^[A-Za-z_].*\b" + re.escape(name) + r"\b")
    out, i, removed = [], 0, 0
    while i < len(lines):
        if start.match(lines[i]):
            j = i
            while j < len(lines) and not lines[j].rstrip().endswith(terminator):
                j += 1
            while out and out[-1].lstrip().startswith("//"):
                out.pop()
            i, removed = j + 1, removed + 1
            continue
        out.append(lines[i])
        i += 1
    if apply and removed:
        path.write_text("\n".join(out))
    return removed


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("header", type=Path)
    ap.add_argument("source", type=Path)
    ap.add_argument("--apply", action="store_true")
    ap.add_argument("--root", type=Path, default=Path("src"))
    args = ap.parse_args()

    names = sorted(set(REDIRECT.findall(args.header.read_text(errors="replace"))))
    if not names:
        print(f"{args.header.name} declares no *_redirect wrapper", file=sys.stderr)
        return 2

    dropped = kept = 0
    for name in names:
        refs = references(name, args.root)
        if refs > 2:
            print(f"  keep {name}: {refs} reference(s), not an orphan")
            kept += 1
            continue
        if is_claimed(name, args.source):
            print(f"  KEEP {name}: carries an ORIGINAL marker - it IS the body")
            kept += 1
            continue
        print(f"  drop {name}")
        cut(args.header, name, ";", args.apply)
        cut(args.source, name, "}", args.apply)
        dropped += 1
    print(f"{dropped} orphan(s) dropped, {kept} kept, of {len(names)} redirect(s) "
          f"declared in {args.header.name}"
          f"{'' if args.apply else '  (dry run; pass --apply)'}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
