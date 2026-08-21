#!/usr/bin/env -S uv run python
"""Byte-exact bodies that live in an artifact the build never compiles.

`src/recovered/`, `src/recovered/units/` and `src/unrecovered/` hold bodies from
earlier bulk passes. A BYTE_EXACT claim on one of those proves THE ARTIFACT and
not this tree: the file is not in `OPENSMACX_SOURCES`, nothing links it, and the
product source often does not even declare the function. `Caviar::close` was
exactly that - byte-exact in `src/recovered/00617020.cpp`, absent from `class
Caviar`, and its atexit teardown was reaching the real body through a pointer
seam because there was nothing to call.

    uv run tools/promotable.py            # only those the compiled tree CALLS
    uv run tools/promotable.py --all      # every artifact-only byte-exact body
    uv run tools/promotable.py --unmarked # bodies already IN the tree, unclaimed

THREE SHAPES, and `--unmarked` is much the cheapest. Some of these bodies are
already DEFINED in product source and simply carry no `// ORIGINAL:` marker, so
the catalogue knows them only through the artifact - `Time::init_class` had been
correct in `time.cpp` all along. Those are not promotions, they are missing
claims on finished work: read the true span with `osmx show`, get the emitted
symbol (the catalogued name is often `QAA`, a non-static member, where the tree
emits `SA`), write the marker, and `osmx record` it.

RANKED BY WHETHER IT MATTERS. An artifact body some compiled body CALLS is worth
promoting now: it removes a `pending_bodies.cpp` forwarder, turns an indirect
call into the image's `E8`, and extends the byte-exact graph by one edge. One
nobody calls is a body without a caller, and can wait.

PROMOTING IS NOT COPYING. The artifact was written in the emitter's style and
usually reaches its fields through `reinterpret_cast<char *>(this) + 0x...`;
promoting it means declaring the member, modelling the layout, and DELETING the
artifact so the tree has one copy. `osmx measure` reports two pieces for an
address carried in both places, which is the reminder to finish the job.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from decomp import read

REPO_ROOT = Path(__file__).resolve().parent.parent
ARTIFACT = ("/recovered/", "/unrecovered/")


if __name__ == "__main__":
    product, artifact = {}, {}
    for record in read(REPO_ROOT / "src"):
        where = str(record.path)
        bucket = artifact if any(p in where for p in ARTIFACT) else product
        bucket.setdefault(record.address, record)

    orphan = {a: r for a, r in artifact.items()
              if a not in product and r.byte_exact}

    # Every address a product body's `calls` fact names. That fact is derived
    # from the shipped bytes, so it says who really calls this - not who
    # happens to mention the name.
    called = set()
    for path in sorted((REPO_ROOT / "src").glob("*.cpp")):
        for line in path.read_text(errors="replace").splitlines():
            if line.startswith("// calls "):
                called.update(int(x, 16)
                              for x in re.findall(r"0x[0-9A-Fa-f]{8}", line))

    # Bodies whose function is already DEFINED in product source. Comments are
    # blanked first so a mention in prose does not read as a definition.
    def blanked(text: str) -> str:
        text = re.sub(r"/\*.*?\*/", lambda m: " " * len(m.group(0)), text,
                      flags=re.S)
        return re.sub(r"//[^\n]*", "", text)

    # `pending_bodies.cpp` IS EXCLUDED BY CONSTRUCTION. Every body in it is a
    # forwarder aimed at an address this tree has NOT promoted - the opposite
    # of "already defined" - and matching on it attributed fourteen bodies to
    # the one file that proves they are missing.
    product_text = "\n".join(
        blanked(p.read_text(errors="replace"))
        for p in sorted((REPO_ROOT / "src").glob("*.cpp"))
        if p.name != "pending_bodies.cpp")

    def cpp_name(mangled: str | None) -> str | None:
        found = re.match(r"\?(\w+)@(\w+)@@", mangled or "")
        if found:
            return f"{found.group(2)}::{found.group(1)}"
        found = re.match(r"\?(\w+)@@", mangled or "")
        return found.group(1) if found else None

    # A DEFINITION, NOT A MENTION. `\bName\s*\(` also matches every CALL, which
    # attributed nineteen bodies to `adjustor_thunks.cpp` - a file that calls
    # those methods and defines none of them.
    #
    # Checked by LOOKING AROUND each occurrence rather than with one anchored
    # pattern: a `^[\w\s:*&<>,~]*` prefix over a few megabytes backtracks
    # catastrophically and never returns. A definition has a `{` before the
    # next `;`, and is not preceded on its line by `return`, `=`, `.` or `->`.
    unmarked = {}
    for address, record in orphan.items():
        name = cpp_name(record.name)
        if not name:
            continue
        for hit in re.finditer(rf"\b{re.escape(name)}\s*\(", product_text):
            line_start = product_text.rfind("\n", 0, hit.start()) + 1
            before = product_text[line_start:hit.start()]
            if re.search(r"(return|=|\.|->|\bnew\b)\s*$", before):
                continue
            after = product_text[hit.end():hit.end() + 400]
            close = after.find(")")
            if close < 0:
                continue
            tail = after[close + 1:close + 40]
            if re.match(r"\s*(const\s*)?\{", tail):
                unmarked[address] = name
                break

    if "--unmarked" in sys.argv:
        for address in sorted(unmarked):
            record = orphan[address]
            print(f"  0x{address:08X}  {record.size or 0:6,}b  "
                  f"{unmarked[address]}")
        print(f"\n{len(unmarked):,} byte-exact bodies are ALREADY DEFINED in "
              f"product source and merely unmarked")
        sys.exit(0)

    wanted = sorted(a for a in orphan if a in called)
    rest = sorted(a for a in orphan if a not in called)
    show = wanted + (rest if "--all" in sys.argv else [])

    for address in show:
        record = orphan[address]
        mark = "CALLED" if address in called else "      "
        print(f"  0x{address:08X}  {mark}  {record.size or 0:6,}b  "
              f"{record.path.parent.name}/{record.path.name:20s} {record.name}")
    print(f"\n{len(orphan):,} byte-exact bodies exist only in an artifact; "
          f"{len(wanted):,} of them are CALLED by code the build compiles")
    print(f"{len(unmarked):,} of the {len(orphan):,} are already DEFINED in "
          f"product source and merely unmarked (--unmarked lists them)")
    if rest and "--all" not in sys.argv:
        print(f"{len(rest):,} more have no caller in the compiled tree "
              f"(--all lists them)")
