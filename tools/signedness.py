#!/usr/bin/env -S uv run python
"""Claims whose only fault is a SIGNED/UNSIGNED branch or divide.

WHY. `jl` and `jb` test the same comparison and differ only in whether the
operands are signed. A `uint32_t` where the image had an `int` costs exactly
one opcode, so the body reads as MISMATCH with a similarity of 0.95 and
nothing points at the keyword. veh_who, base_who and anything_at were all this,
and all three turned on one word.

The same holds for `idiv`/`div`, `imul`/`mul`, `sar`/`shr`, `movsx`/`movzx`
and the whole `setcc`/`jcc` family.

This scores every non-exact claim in the build's own files and reports the
ones where every positional difference is one of those pairs - so the fix is a
type, and which type is stated.

    uv run tools/signedness.py [--all]

Without `--all` only claims where EVERY difference is a signedness pair are
listed; those are the ones a keyword finishes.
"""

from __future__ import annotations

import concurrent.futures
import os
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from decomp import read
from decomp.asm import (build_command, build_inputs, compile_unit,
                        original_asm, shared_spans, span_refusal, subject_asm)

REPO_ROOT = Path(__file__).resolve().parent.parent
# HONOURS `OPENSMACX_IMAGE`, because `.opensmacx/` is gitignored and a
# worktree does not have it - an agent working in one must be able to
# point every tool back at the root copy. osmx.py has always taken it;
# these did not, and an agent had to symlink the image to work around it.
IMAGE = Path(os.environ.get(
    "OPENSMACX_IMAGE",
    REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"))
COMPILE_COMMANDS = Path(os.environ.get(
    "OPENSMACX_COMPILE_COMMANDS", REPO_ROOT / "build" / "compile_commands.json"))
BORROW = REPO_ROOT / "src" / "buffer.cpp"
FLAGS = ("/c /O2 /Gy /GR- /Oy- /GX", "/c /O2 /Gy /GR- /GX",
         "/c /O1 /Gy /GR- /Oy- /GX", "/c /O1 /Gy /GR- /GX")

# Each pair is (SIGNED, UNSIGNED). The image's side names the type the source
# should have.
PAIRS = [("jl", "jb"), ("jle", "jbe"), ("jg", "ja"), ("jge", "jae"),
         ("setl", "setb"), ("setle", "setbe"), ("setg", "seta"),
         ("setge", "setae"), ("idiv", "div"), ("imul", "mul"),
         ("sar", "shr"), ("movsx", "movzx"), ("cdq", "xor")]
SIGNED = {signed: unsigned for signed, unsigned in PAIRS}
UNSIGNED = {unsigned: signed for signed, unsigned in PAIRS}


def _one(job: tuple) -> list[tuple]:
    path, records, command = job
    out = []
    for flags in FLAGS:
        try:
            obj = compile_unit(path, command, flags)
        except Exception:                       # noqa: BLE001 - next flag set
            continue
        for record in records:
            try:
                here = subject_asm(obj, record, flags)
                image = original_asm(record, IMAGE)
            except Exception:                   # noqa: BLE001 - not this one
                continue
            left = [i.mnemonic for i in image.instructions]
            right = [i.mnemonic for i in here.instructions]
            if len(left) != len(right):
                continue
            differences = [(a, b) for a, b in zip(left, right) if a != b]
            if not differences:
                continue
            signedness = [(a, b) for a, b in differences
                          if SIGNED.get(a) == b or UNSIGNED.get(a) == b]
            out.append((record, len(differences), signedness))
        break
    return out


if __name__ == "__main__":
    records = read(REPO_ROOT / "src")
    shared = shared_spans(records)
    built = build_inputs(COMPILE_COMMANDS)
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

    rows = [row for batch in batches for row in batch if row[2]]
    only = [r for r in rows if r[1] == len(r[2])]
    for record, total, signedness in sorted(
            only if "--all" not in sys.argv else rows,
            key=lambda r: (r[1] - len(r[2]), r[1])):
        kinds = ", ".join(sorted({f"{a}/{b}" for a, b in signedness}))
        rest = "" if total == len(signedness) else \
            f", plus {total - len(signedness)} other difference(s)"
        print(f"  {record.address_hex}  {kinds:24s} "
              f"{record.path.name:22s} {record.name}{rest}")
    print(f"{len(only):,} claim(s) differ ONLY by signedness, of {len(rows):,} "
          f"that differ by it at all")
