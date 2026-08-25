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
import collections
import re
import sys
from pathlib import Path

REDIRECT = re.compile(r"\b[A-Za-z_][\w]*_redirect\b")


_COUNTS: collections.Counter = collections.Counter()


def _load_counts(root: Path) -> None:
    """Count EVERY `*_redirect` occurrence in the tree in ONE pass.

    The first version ran one regex per name over every file: ~400 names by
    ~900 files is 360,000 scans of large texts, and the sweep never
    finished. One `findall` per file with the shared REDIRECT pattern gives
    the same answer in a single read.
    """
    if _COUNTS:
        return
    for path in list(root.rglob("*.cpp")) + list(root.rglob("*.h")):
        _COUNTS.update(REDIRECT.findall(path.read_text(errors="replace")))


def references(name: str, root: Path) -> int:
    _load_counts(root)
    return _COUNTS[name]


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


def cut(path: Path, name: str, _unused: str, apply: bool) -> int:
    """Remove every declaration and definition of `name` from `path`.

    DECLARATION AND DEFINITION ARE NOT THE SAME CUT, and conflating them is
    how the first sweep corrupted stringstruct.cpp: a forward DECLARATION
    inside the .cpp was matched and then cut with a `}` terminator, which
    scanned past its own `;` and swallowed the function after it. So: read
    forward to whichever comes first, `;` or `{`. A `;` ends a declaration
    there; a `{` starts a body, and the body ends when brace depth returns
    to zero - not at the first line that happens to end in `}`, which any
    nested block satisfies.
    """
    lines = path.read_text(errors="replace").split("\n")
    start = re.compile(r"^[A-Za-z_].*\b" + re.escape(name) + r"\b")
    out, i, removed = [], 0, 0
    while i < len(lines):
        if not start.match(lines[i]):
            out.append(lines[i])
            i += 1
            continue
        j, kind = i, None
        while j < len(lines):
            line = lines[j]
            semi, brace = line.find(";"), line.find("{")
            if brace >= 0 and (semi < 0 or brace < semi):
                kind = "body"
                break
            if semi >= 0:
                kind = "decl"
                break
            j += 1
        if kind is None:
            out.append(lines[i])
            i += 1
            continue
        if kind == "body":
            depth = 0
            while j < len(lines):
                depth += lines[j].count("{") - lines[j].count("}")
                if depth <= 0:
                    break
                j += 1
        while out and out[-1].lstrip().startswith("//"):
            out.pop()
        i, removed = j + 1, removed + 1
    if apply and removed:
        path.write_text("\n".join(out))
    return removed


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("header", type=Path, nargs="?")
    ap.add_argument("source", type=Path, nargs="?")
    ap.add_argument("--all", action="store_true",
                    help="every header/source pair under --root that declares one")
    ap.add_argument("--apply", action="store_true")
    ap.add_argument("--root", type=Path, default=Path("src"))
    args = ap.parse_args()

    if args.all:
        pairs = []
        for h in sorted(args.root.glob("*.h")):
            c = h.with_suffix(".cpp")
            if c.exists() and "_redirect" in h.read_text(errors="replace"):
                pairs.append((h, c))
        total_d = total_k = 0
        for h, c in pairs:
            args.header, args.source = h, c
            names = sorted(set(REDIRECT.findall(h.read_text(errors="replace"))))
            d = k = 0
            for name in names:
                if references(name, args.root) > 2 or is_claimed(name, c):
                    k += 1
                    continue
                cut(h, name, ";", args.apply)
                cut(c, name, "}", args.apply)
                # NO cache invalidation: deleting a redirect removes only
                # its OWN two references, so every other name's count is
                # still right. Clearing here re-read ~900 files per drop and
                # made the sweep quadratic.
                d += 1
            if d or k:
                print(f"  {h.stem:<24} {d} dropped, {k} kept")
            total_d += d; total_k += k
        print(f"{total_d} orphan(s) dropped, {total_k} kept, across {len(pairs)} pair(s)"
              f"{'' if args.apply else '  (dry run; pass --apply)'}")
        return 0

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
