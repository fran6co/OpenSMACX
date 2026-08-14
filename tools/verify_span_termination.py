#!/usr/bin/env python3
"""Fail when a catalogued span stops in the middle of an instruction.

A SPAN THAT IS TWO BYTES SHORT CANNOT BE MATCHED BY ANY BODY. The comparison
reads `end_address` off the catalogue and asks the compiler to reproduce
exactly those bytes; if the span cuts the closing `ret 0x14` in half, the
rebuild is right and the verdict is MISMATCH, forever, on a divergence at the
very last instruction. Nothing in the loop points back here - the agent sees
`#75: original '<end>' vs rebuilt 'ret 0x14'` and reasons about its own body.

Measured 2026-08-14. An agent recovering 0x005D91D0 reported "every mnemonic
matches" and traced it to the span rather than believing the verdict. Nine
catalogued rows were truncated the same way, six of them by exactly two bytes
- the width of a `ret imm16` minus its opcode - and six of the nine were in
the batch being worked at the time. Correcting 0x005D91D0's span turned it
BYTE_EXACT with no change to the body at all.

WHAT THIS DOES NOT FLAG, deliberately, because each is a legitimate shape and
failing on it would make the check something to switch off rather than fix:

  * a span ending on a `call`, 8 rows - a tail call into a noreturn CRT
    routine (`_amsg_exit`, `__threadstart`), which really is the last
    instruction;
  * a span ending on `nop`, 18 rows - trailing alignment padding, which VC6
    emits too: `sub_5ad450` is BYTE_EXACT with its padding inside the span;
  * a region this disassembler cannot decode at all, such as `_memcpy`, which
    carries jump-table data inside its own body. "I could not read it" is not
    evidence of truncation.

The test is narrow and positive: decoding from the span's start, does the
last instruction OVERLAP the end rather than finish at it, and does extending
by a few bytes land on a terminator? That is truncation and nothing else is.
"""

from __future__ import annotations

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import byte_match  # noqa: E402
import emit_translation_unit as emit  # noqa: E402

# `int3` and `ud2` end a body as surely as `ret` does; a tail `jmp` is how
# every EH funclet and every `/Gy` fold leaves.
TERMINATORS = frozenset({"ret", "retf", "jmp", "int3", "ud2"})

# How far past a truncated end to look for the terminator. The longest x86
# instruction is 15 bytes, so nothing legitimate needs more.
LOOKAHEAD = 16


def truncation(pe, engine, low: int, high: int):
    """(bytes short, terminator text) when the span cuts an instruction."""
    window = emit.read_bytes(pe, low, high - low + LOOKAHEAD)
    if not window:
        return None
    last = None
    for one in engine.disasm(window, low):
        if one.address >= high:
            break
        last = one
    if last is None:
        return None
    finish = last.address + last.size
    if finish <= high:
        return None                     # ends cleanly, or undecodable: not ours
    if last.mnemonic not in TERMINATORS:
        # The instruction straddling the end is not the body's last one, so
        # this is a disassembly disagreement rather than a truncation. Saying
        # so would be a guess about where the function really ends.
        return None
    return finish - high, f"{last.mnemonic} {last.op_str}".strip()


def faults(rows: dict = None) -> list:
    from capstone import CS_ARCH_X86, CS_MODE_32, Cs
    import pefile

    rows = emit.load_functions() if rows is None else rows
    pe = pefile.PE(str(byte_match.DEFAULT_EXE), fast_load=True)
    engine = Cs(CS_ARCH_X86, CS_MODE_32)
    found = []
    for address, row in sorted(rows.items()):
        for low, high in emit.parse_body_ranges(row.get("body_ranges") or ""):
            hit = truncation(pe, engine, low, high)
            if hit is None:
                continue
            short, text = hit
            found.append(
                f"0x{address:08X} {row.get('name', '')}: span "
                f"0x{low:08X}-0x{high:08X} stops {short} byte(s) into "
                f"`{text}`; no body can reproduce a partial instruction")
    return found


def main(argv=None) -> int:
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    # `--src` EXISTS SO THE DAMAGE CASE CAN RUN THIS EXACT TOOL. A proof that
    # a check can fail is worth nothing if it fails a different invocation
    # than the gate makes; this repository has published one of those. With
    # no flag the tool reads `src/`, which is what the gate runs, and the
    # damage case points it at a one-file tree through the same reader.
    parser.add_argument("--src", help="annotation tree to read instead of src/")
    arguments = parser.parse_args(argv)
    if not byte_match.DEFAULT_EXE.is_file():
        print("SKIP: the pinned executable is absent")
        return 0
    rows = None
    if arguments.src:
        import project_catalogue
        rows = project_catalogue.from_source(Path(arguments.src))
    found = faults(rows)
    for line in found:
        print(f"  {line}")
    print(f"span-termination: {len(found)} truncated span(s)")
    return 1 if found else 0


if __name__ == "__main__":
    raise SystemExit(main())
