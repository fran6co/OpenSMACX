#!/usr/bin/env -S uv run python
"""Bodies that call something the image does not, or miss one it makes.

WHY THIS IS NOT A BYTE QUESTION. A body that reaches a helper the shipped code
writes out inline is not merely encoded differently - it executes a `call` the
original never executes, and its callee's own claim is being satisfied by a
different program. Three faction.cpp bodies were found this way in one pass:
`great_satan` called `great_beelzebub`, `climactic_battle` called `ascending`,
`del_site` called `vector_dist`, and the image open-codes all three.

The reverse is worse: a call the IMAGE makes and this tree does not is a body
that skips work.

Both are visible without any byte comparison - disassemble each side, collect
the direct call targets, and subtract. This reports them, ranked by how many
edges disagree, and names the callee where the catalogue can.

    uv run tools/call_diff.py [--all]

Only DIRECT calls, on both sides. An indirect call through a vtable or a bound
slot is a runtime fact neither side can resolve, so it is counted and excluded
rather than guessed at.
"""

from __future__ import annotations

import collections
import concurrent.futures
import os
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from decomp import read
from decomp.asm import (build_command, build_inputs, call_symbols,
                        compile_unit, original_asm, shared_spans,
                        span_refusal, subject_asm)
from decomp.calls import call_sites

REPO_ROOT = Path(__file__).resolve().parent.parent
IMAGE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"
COMPILE_COMMANDS = REPO_ROOT / "build" / "compile_commands.json"
BORROW = REPO_ROOT / "src" / "buffer.cpp"
# EVERY SET, NOT ONE. `memset` and `strcat` are INTRINSICS under `/Oi`, so a
# body the image calls them from reads as "makes no calls" under the default
# set and as a false positive here. A disagreement is only real when NO
# invocation reproduces the image's count.
FLAG_SETS = ("/c /O2 /Gy /GR- /Oy- /GX", "/c /O2 /Gy /GR- /GX",
             "/c /O2 /Oi- /Gy /GR- /Oy- /GX", "/c /O2 /Oi- /Gy /GR- /GX",
             "/c /O2 /Ob0 /Gy /GR- /Oy- /GX", "/c /O1 /Gy /GR- /Oy- /GX")


def _targets(listing) -> collections.Counter:
    out: collections.Counter = collections.Counter()
    for site in call_sites(listing):
        if site.form in ("direct", "tail") and site.target is not None:
            out[site.target] += 1
    return out


def _one(job: tuple) -> list[tuple]:
    path, records, command = job
    objects = []
    for flags in FLAG_SETS:
        try:
            objects.append((flags, compile_unit(path, command, flags)))
        except Exception:                       # noqa: BLE001 - try the next
            continue
    if not objects:
        return []
    out = []
    for record in records:
        try:
            image = original_asm(record, IMAGE)
        except Exception:                       # noqa: BLE001 - skip this one
            continue
        best = None
        for flags, obj in objects:
            try:
                candidate = subject_asm(obj, record, flags)
            except Exception:                   # noqa: BLE001 - next set
                continue
            gap = abs(sum(_targets(candidate).values())
                      - sum(_targets(image).values()))
            if best is None or gap < best[0]:
                best = (gap, candidate, obj)
        if best is None:
            continue
        _gap, here, obj = best
        # RELOCATED TARGETS ARE UNKNOWABLE on the object side: the linker
        # writes them. So the tree's call COUNT is comparable but not its
        # addresses, and the useful statement is about how many.
        theirs, mine = _targets(image), _targets(here)
        if sum(theirs.values()) != sum(mine.values()):
            # WHICH ONES, on the tree's side. The object's call target is a
            # relocation, so the SYMBOL is the only thing that names it - and
            # "you call `vector_dist` and the image does not" is the sentence
            # worth printing.
            try:
                calling = call_symbols(obj, record.name, record.symbol)
            except Exception:               # noqa: BLE001 - names are a bonus
                calling = []
            out.append((record, sum(theirs.values()), sum(mine.values()),
                        sorted(theirs), calling))
    return out


if __name__ == "__main__":
    records = read(REPO_ROOT / "src")
    shared = shared_spans(records)
    built = build_inputs(COMPILE_COMMANDS)
    named = {}
    for record in records:
        named.setdefault(record.address, record.name)

    grouped: dict = {}
    for record in records:
        if record.byte_exact or record.path not in built or not record.name:
            continue
        if span_refusal(record, IMAGE, shared):
            continue
        grouped.setdefault(record.path, []).append(record)

    work = []
    for path, mine in sorted(grouped.items()):
        try:
            command = build_command(COMPILE_COMMANDS, path)
        except ValueError:
            command = build_command(COMPILE_COMMANDS, BORROW)
        work.append((path, mine, command))

    jobs = max(1, min(os.cpu_count() or 1, 8))
    print(f"{sum(len(v) for v in grouped.values()):,} unclaimed bodies in "
          f"{len(grouped):,} files, {jobs} at a time", flush=True)
    pool = concurrent.futures.ProcessPoolExecutor(max_workers=jobs)
    with pool:
        batches = list(pool.map(_one, work))

    rows = [row for batch in batches for row in batch]
    rows.sort(key=lambda row: (-abs(row[1] - row[2]), row[0].address))
    for record, theirs, mine, targets, calling in (rows if "--all" in sys.argv
                                                   else rows[:40]):
        where = "MORE" if mine > theirs else "FEWER"
        names = ", ".join(named.get(t, f"0x{t:08X}") or f"0x{t:08X}"
                          for t in targets[:4])
        print(f"  {record.address_hex}  this tree makes {mine} call(s), the "
              f"image {theirs} - {where}")
        print(f"      {record.path.name:22s} {record.name}")
        if names:
            print(f"      image calls: {names}")
        if calling and mine > theirs:
            extra = ", ".join(sorted(set(calling))[:5])
            print(f"      this tree calls: {extra}")
    print(f"{len(rows):,} body(s) disagree with the image about how many "
          f"calls they make")
