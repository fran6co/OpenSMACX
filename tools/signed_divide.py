#!/usr/bin/env -S uv run python
"""Bodies paying for a signed divide the image does not pay for.

`x / 2` on a signed int is NOT `x >> 1`: C rounds toward zero, so VC6 emits a
fixup before the shift - `cdq; sub eax, edx; sar eax, 1` for /2, or
`cdq; and edx, 1; add eax, edx; sar eax, 1` shapes for larger powers. `x >> 1`
is one bare `sar`.

When a value cannot be negative - a sum of `abs()` results, a count, a length -
the two are identical in meaning and differ by two to four instructions per
site. `base_find` had two such divides and was +4 instructions on exactly them:
31/79 agreeing became 53/79, with the instruction count then EXACT.

    uv run tools/signed_divide.py [<addr> ...]

Reports a body when OUR listing contains the fixup and the image's does not, at
a comparable point. That asymmetry is the whole signal - a fixup both sides
emit is correct and must be left alone, because the value really can be
negative there and `>>` would change the result.
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
FLAGS = "/c /O2 /Oi- /Gy /GR- /Oy- /GX"


def fixups(listing) -> int:
    """How many signed-division fixups this listing contains.

    The tell is `cdq` (or `cwd`) followed IMMEDIATELY by `sub` - that pair is
    the round-toward-zero correction and nothing else uses it - and then a `sar`
    at some point before the next `cdq` or any `idiv`.

    THE `sar` CAN BE FAR AWAY. A first version required it within three
    instructions, and `Buffer::write_cent_l` reported a fixup we had and the
    image did not: the image emits `cdq; sub; push; mov; mov; sar`, scheduling
    five instructions between the pair and the shift. Its source comment says
    plainly that the image does a signed halving there, so the tool contradicted
    a note that was right, on a body whose value really can be negative. `cdq`
    alone is never enough either: it also precedes `idiv`, a real division.
    """
    mnemonics = list(listing.mnemonics)
    found = 0
    for i, mnemonic in enumerate(mnemonics):
        if mnemonic not in ("cdq", "cwd"):
            continue
        if i + 1 >= len(mnemonics) or mnemonics[i + 1] != "sub":
            continue
        for later in mnemonics[i + 2:]:
            if later in ("cdq", "cwd", "idiv", "div"):
                break
            if later == "sar":
                found += 1
                break
    return found


def one(job):
    record, command = job
    try:
        theirs = fixups(original_asm(record, IMAGE))
        mine = fixups(compiled_asm(record, command, FLAGS))
    except Exception as error:                            # noqa: BLE001
        return None
    if mine > theirs:
        return (record, theirs, mine)
    return None


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

    rows.sort(key=lambda r: -(r[2] - r[1]))
    for record, theirs, mine in rows:
        print(f"  {record.address_hex}  image {theirs}, ours {mine}   "
              f"{record.path.name:20s} {record.name}")
    print(f"\n{len(rows):,} body(s) emit a signed-division fixup the image does not")
