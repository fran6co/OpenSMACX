#!/usr/bin/env -S uv run python
"""Bodies widening a value the image keeps narrow, or the reverse.

A local declared wider than the field it is loaded from makes VC6 emit a
`movsx`/`movzx` the image does not have. `guard_check` was byte-exact the moment
`plan_region` became `uint8_t` - the element type of the `region_base_plan[128]`
it reads - instead of `uint32_t`, which had been forcing an `xor edx, edx`
zero-extend and word-form compares against the image's `cmp dl, N`. A
stack-climb induction variable needs `int16_t` where the field it walks is
`int16_t`: `int` forces an early `movsx` the image DEFERS, keeping the 16-bit
value in a register and widening only at the indexing.

Four bodies went byte-exact on this alone. This finds the rest by comparing how
many widening instructions each side emits.

    uv run tools/width_mismatch.py [<addr> ...]

MORE on our side means a local is too WIDE for what it reads - narrow it to the
field's own type. FEWER means the opposite: we are keeping something narrow that
the image widens, usually because a field is declared too small.

The count is a signal, not a proof: `movzx` also appears in ordinary byte
handling that both sides do. Read `tools/listing_diff.py <addr>` before changing
anything.
"""

from __future__ import annotations

import concurrent.futures
import os
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from decomp import read
from decomp.asm import (build_command, build_inputs, compiled_asm,
                        original_asm, shared_spans, span_refusal)

REPO_ROOT = Path(__file__).resolve().parent.parent
IMAGE = Path(os.environ.get(
    "OPENSMACX_IMAGE",
    REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"))
COMPILE_COMMANDS = Path(os.environ.get(
    "OPENSMACX_COMPILE_COMMANDS", REPO_ROOT / "build" / "compile_commands.json"))
BORROW = REPO_ROOT / "src" / "buffer.cpp"
FLAGS = "/c /O2 /Gy /GR- /GX"
WIDENING = ("movsx", "movzx")


def widenings(listing) -> int:
    return sum(1 for m in listing.mnemonics if m in WIDENING)


def one(job):
    record, command = job
    try:
        theirs = widenings(original_asm(record, IMAGE))
        mine = widenings(compiled_asm(record, command, FLAGS))
    except Exception:                                     # noqa: BLE001
        return None
    if theirs == mine:
        return None
    return (record, theirs, mine)


if __name__ == "__main__":
    wanted = {int(a, 16) for a in sys.argv[1:] if not a.startswith("--")}
    records = read(REPO_ROOT / "src")
    built = build_inputs(COMPILE_COMMANDS)
    shared = shared_spans(records)

    work = []
    for record in records:
        if wanted and record.address not in wanted:
            continue
        if record.byte_exact or record.path not in built or not record.name:
            continue
        if span_refusal(record, IMAGE, shared):
            continue
        try:
            command = build_command(COMPILE_COMMANDS, record.path)
        except ValueError:
            command = build_command(COMPILE_COMMANDS, BORROW)
        work.append((record, command))

    print(f"scoring {len(work):,} bodies", flush=True)
    jobs = max(1, min(os.cpu_count() or 1, 8))
    with concurrent.futures.ProcessPoolExecutor(max_workers=jobs) as pool:
        rows = [r for r in pool.map(one, work, chunksize=4) if r]

    rows.sort(key=lambda r: -(abs(r[2] - r[1])))
    too_wide = [r for r in rows if r[2] > r[1]]
    for record, theirs, mine in rows[:40]:
        where = "TOO WIDE" if mine > theirs else "too narrow"
        print(f"  {record.address_hex}  image {theirs:2d}, ours {mine:2d}  "
              f"{where:10s} {record.path.name:20s} {record.name}")
    if len(rows) > 40:
        print(f"  ... and {len(rows) - 40} more")
    print(f"\n{len(rows):,} body(s) disagree about widening; {len(too_wide):,} "
          f"emit MORE than the image (a local wider than what it reads)")
