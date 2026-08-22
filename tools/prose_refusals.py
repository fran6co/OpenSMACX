#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# dependencies = []
# ///
"""Bodies whose refusal is written as prose the reader cannot see.

A refusal only counts if it is a `// RULED-OUT:` LESSON LINE. Written as
ordinary prose inside the comment block - "cannot reach byte-exact", "not
chased further", "gives up" - it is invisible to `decomp.reader`, so
`frontier.py --untouched` keeps offering the body as unworked and each new
pass re-derives the same wall.

`Buffer::fill` is the example that motivated this: its comment says CANNOT
REACH BYTE-EXACT UNDER THIS PROJECT'S RULES and explains exactly why (the
image body is hand-written `__asm`), and it still reads as untouched.

    uv run tools/prose_refusals.py

Reports every marker whose comment block contains refusal language but
carries no RULED-OUT line of its own.
"""
import pathlib
import re
import sys

REPO_ROOT = pathlib.Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO_ROOT))

from decomp.reader import read  # noqa: E402

# Phrases this tree actually uses when it is giving up on a body.
REFUSAL = re.compile(
    r"cannot reach|can't reach|not chased further|gives up|give up on|"
    r"unreachable from|no tier is reachable|not reproducible|"
    r"cannot be reproduced|not attempted further|out of scope for",
    re.I)
LESSON = re.compile(r"^\s*//\s*(RULED-OUT|UNRECOVERABLE|DEFERRED)\b", re.M)


def block(path: pathlib.Path, line: int) -> str:
    """The comment block a marker sits in - back to `/*`, on to `*/`."""
    try:
        lines = path.read_text(errors="replace").splitlines()
    except OSError:
        return ""
    start = line - 1
    while start > 0 and not lines[start].lstrip().startswith("/*"):
        start -= 1
        if line - start > 80:
            break
    end = line
    while end < len(lines) and "*/" not in lines[end]:
        end += 1
        if end - line > 120:
            break
    return "\n".join(lines[start:end + 1])


if __name__ == "__main__":
    rows = []
    for record in read(REPO_ROOT / "src"):
        if record.byte_exact or record.ruled_out:
            continue
        text = block(record.path, record.line)
        if not text:
            continue
        found = REFUSAL.search(text)
        if found and not LESSON.search(text):
            where = record.path.relative_to(REPO_ROOT / "src")
            rows.append((record, where, found.group(0)))

    for record, where, phrase in rows:
        print(f"  {record.address_hex}  {where}:{record.line}")
        print(f"      says \"{phrase}\" in prose, carries no RULED-OUT line - "
              f"{record.name}")
    print(f"\n{len(rows)} body(s) refuse in prose but not where the reader "
          f"looks, so `frontier.py --untouched` still offers them")
    sys.exit(0)
