#!/usr/bin/env -S uv run python
"""Which catalogued bodies NOTHING in the image calls - so it inlines them.

WHY. `bit_at` has a real body at 0x005001B0 and is also written out inside
`goody_at`, which makes no calls at all. A helper like that must be `inline` in
a header AND `MEASURED`, or its callers can never be byte exact - but defining
it in a .cpp makes it exactly one of those, and nothing said which ones are
like this.

THIS ASKS THE IMAGE. Every catalogued body is disassembled and its direct call
targets collected; a body that is never anyone's target, yet is named at many
call sites in the tree, is one the image inlines everywhere. That is a
measurement, not a guess about what VC6 might do with a small function.

A LOWER BOUND on the callers, deliberately: a target reached only through a
vtable or a bound slot is not counted, so a body listed here might still be
called somewhere this cannot see. The count of unresolved edges is printed.
"""

from __future__ import annotations

import re
import os
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from decomp import read
from decomp.asm import build_inputs, original_asm, shared_spans, span_refusal
from decomp.calls import call_sites
from decomp.mangled import identifier

REPO_ROOT = Path(__file__).resolve().parent.parent
# HONOURS `OPENSMACX_IMAGE`, because `.opensmacx/` is gitignored and a
# worktree does not have it - an agent working in one must be able to
# point every tool back at the root copy. osmx.py has always taken it;
# these did not, and an agent had to symlink the image to work around it.
IMAGE = Path(os.environ.get(
    "OPENSMACX_IMAGE",
    REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"))


if __name__ == "__main__":
    records = read(REPO_ROOT / "src")
    shared = shared_spans(records)
    built = build_inputs(REPO_ROOT / "build" / "compile_commands.json")

    called, indirect, read_bodies = set(), 0, 0
    for record in records:
        if not record.image_spans or span_refusal(record, IMAGE, shared):
            continue
        try:
            listing = original_asm(record, IMAGE)
        except (ValueError, KeyError):
            continue
        read_bodies += 1
        for site in call_sites(listing):
            if site.form in ("direct", "tail") and site.target is not None:
                called.add(site.target)
            else:
                indirect += 1

    whole = "\n".join(p.read_text() for p in sorted(REPO_ROOT.glob("src/*.[ch]*")))
    rows = []
    for record in records:
        if record.path not in built or record.address in called:
            continue
        if not record.size or record.size > 120:
            continue
        # FREE FUNCTIONS ONLY - `?name@@Y...`. A member is reached through a
        # vtable as often as not, and 5,531 of the edges read here are
        # indirect, so "nothing calls it" is not evidence for one. It is for a
        # free function, which is called directly or not at all. It also keeps
        # the use count honest: a dozen classes have a `close`.
        if not record.name.startswith("?") or "@@Y" not in record.name:
            continue
        name = identifier(record.name) if record.name else ""
        if not name or not name.isidentifier():
            continue
        uses = len(re.findall(rf"(?<![\w:])\b{re.escape(name)}\s*\(", whole))
        if uses >= 4:
            rows.append((uses, record.size, record.address_hex, name,
                         record.path.name, record.byte_exact))
    for uses, size, address, name, where, exact in sorted(rows, reverse=True):
        mark = "exact" if exact else "     "
        print(f"{uses:5d} uses  {size:4d}B  {address}  {mark}  {name:26s} "
              f"{where}")
    print(f"{len(rows):,} catalogued bodies nothing calls directly, of "
          f"{read_bodies:,} read ({indirect:,} indirect edges not resolved)")
