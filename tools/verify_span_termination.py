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


def second_entry(pe, engine, low: int, high: int):   # noqa: C901
    """Where a SECOND function starts inside this span, or None.

    A catalogued span is supposed to be one function. 0x0061F800's is two,
    back to back, each with its own full prologue and epilogue and no jump
    between them - found by an agent who reproduced the first and had no way
    to know the second was even in scope. Nothing checked for it.

    The signature is a `ret` (the first function's epilogue) followed, after
    at most a run of alignment padding, by `push ebp; mov ebp, esp` - a fresh
    frame set up by code that nothing jumped to. A `ret` mid-body is ordinary
    - an early return - so the `ret` alone proves nothing; it is the PROLOGUE
    after it that does, because a function does not build its frame twice.
    """
    window = emit.read_bytes(pe, low, high - low)
    if not window:
        return None
    seen_return, padding = False, 0
    previous = None
    for one in engine.disasm(window, low):
        if one.address == low:
            previous = one
            continue
        if seen_return:
            if one.mnemonic in ("int3", "nop") or \
                    (one.mnemonic == "lea" and one.op_str.startswith("esp")):
                padding += 1
                continue
            if one.mnemonic == "push" and one.op_str == "ebp":
                # NOT `push ebp` ALONE. That is also how a function saves ebp
                # as a callee-saved register partway through, and three of the
                # four rows this first flagged were exactly that. A new
                # function is the PAIR - `push ebp; mov ebp, esp` - because
                # only an entry point establishes a frame.
                nxt = next(engine.disasm(
                    emit.read_bytes(pe, one.address + one.size, 4) or b"",
                    one.address + one.size), None)
                if nxt is not None and nxt.mnemonic == "mov" \
                        and nxt.op_str.replace(" ", "") == "ebp,esp":
                    return one.address, padding
            seen_return, padding = False, 0
        if one.mnemonic == "ret":
            seen_return, padding = True, 0
        previous = one
    return None


def split_spans(rows: dict = None) -> list:
    """Rows whose span holds more than one function."""
    from capstone import CS_ARCH_X86, CS_MODE_32, Cs
    import pefile

    rows = emit.load_functions() if rows is None else rows
    pe = pefile.PE(str(byte_match.DEFAULT_EXE), fast_load=True)
    engine = Cs(CS_ARCH_X86, CS_MODE_32)
    found = []
    for address, row in sorted(rows.items()):
        for low, high in emit.parse_body_ranges(row.get("body_ranges") or ""):
            hit = second_entry(pe, engine, low, high)
            if hit is None:
                continue
            where, padding = hit
            found.append(
                f"0x{address:08X} {row.get('name', '')}: span "
                f"0x{low:08X}-0x{high:08X} contains a SECOND prologue at "
                f"0x{where:08X} ({padding} pad instruction(s) after a ret) - "
                f"no single body can reproduce two functions")
    return found


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


def mismeasured(rows: dict = None) -> list:
    """Rows whose `// size` is not what their own `// spans` cover.

    THE TWO FIELDS ARE ONE FACT WRITTEN TWICE, and the spans are the half
    every consumer reads: the comparison takes its bytes from them, and
    `size` is only what the catalogue says about their length. So a
    disagreement is always a fact having moved, and which half moved is not
    in doubt.

    Held to zero because it WAS zero. All 6,000 rows agreed at 185dd977, the
    commit where `src/` became the store; 60 disagree now. 39 of them are in
    `src/field_accessors.cpp` and 20 in `src/scroll.cpp`, each file carrying
    one size repeated over every annotation in it - `?on_left_click@Scroll@@`
    is three bytes and says 75 because the row above it does.

    It is also the check that catches a span being REMOVED. `0x00589B60` lost
    the `0x0066026A-0x0066028A` half of its span in batch 10 - an SEH funclet
    the body had dropped - and taking a span out lowers the bar the body is
    measured against, which is exactly what this file exists to refuse. The
    truncation test above cannot see it: what is left ends on a `ret` and
    decodes perfectly.
    """
    rows = emit.load_functions() if rows is None else rows
    found = []
    for address, row in sorted(rows.items()):
        size = row.get("size")
        spans = emit.parse_body_ranges(row.get("body_ranges") or "")
        if not spans or not str(size).isdigit():
            continue
        covered = sum(high - low for low, high in spans)
        if covered != int(size):
            found.append(
                f"0x{address:08X} {row.get('name', '')}: `// size "
                f"{size} bytes` beside spans covering {covered}")
    return found


def restate(rows: dict) -> int:
    """Rewrite every `// size` that disagrees with its own spans.

    Only ever the size: the spans are what the comparison reads, so moving
    them to agree with a stated length would change what a body is measured
    against - the defect, not the repair.
    """
    import annotation_scan
    import project_catalogue
    resolved, _ = annotation_scan.resolve(annotation_scan.scan_tree())
    where, by_file = {}, {}
    for one in resolved:
        where[one.address] = (one.path, one.line - 1)
    for address, row in rows.items():
        spans = emit.parse_body_ranges(row.get("body_ranges") or "")
        if not spans or not str(row.get("size")).isdigit():
            continue
        covered = sum(high - low for low, high in spans)
        if covered == int(row["size"]) or address not in where:
            continue
        path, index = where[address]
        by_file.setdefault(path, []).append((index, covered))

    changed = 0
    for relative, entries in by_file.items():
        path = project_catalogue.REPO_ROOT / relative
        lines = path.read_text().splitlines()
        for index, covered in entries:
            for offset in range(index + 1, len(lines)):
                stripped = lines[offset].strip()
                if not (stripped.startswith("//") or stripped.startswith("*")):
                    break
                if lines[offset].startswith("// size "):
                    lines[offset] = f"// size      {covered} bytes"
                    changed += 1
                    break
        path.write_text("\n".join(lines) + "\n")
    return changed


def main(argv=None) -> int:
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repair", action="store_true",
                        help="restate `// size` from the spans it disagrees with")
    # `--src` EXISTS SO THE DAMAGE CASE CAN RUN THIS EXACT TOOL. A proof that
    # a check can fail is worth nothing if it fails a different invocation
    # than the gate makes; this repository has published one of those. With
    # no flag the tool reads `src/`, which is what the gate runs, and the
    # damage case points it at a one-file tree through the same reader.
    parser.add_argument("--src", help="annotation tree to read instead of src/")
    arguments = parser.parse_args(argv)
    rows = None
    if arguments.src:
        import project_catalogue
        rows = project_catalogue.from_source(Path(arguments.src))

    # THE SIZE COMPARISON NEEDS NO IMAGE. It reads one annotation against
    # itself, so it still runs on a checkout without the game - and putting it
    # after the skip would have made "the exe is absent" silently cover both.
    if arguments.repair:
        print(f"restated {restate(rows or emit.load_functions())} size(s)")
        return 0

    wrong = mismeasured(rows)
    for line in wrong:
        print(f"  {line}")
    print(f"span-termination: {len(wrong)} row(s) whose size and spans disagree")

    if not byte_match.DEFAULT_EXE.is_file():
        print("SKIP: the pinned executable is absent; truncation not measured")
        return 1 if wrong else 0
    found = faults(rows)
    for line in found:
        print(f"  {line}")
    print(f"span-termination: {len(found)} truncated span(s)")

    # ONE SPAN, ONE FUNCTION. Not checked until an agent reproducing
    # 0x0061F800 noticed the span held two, each with its own prologue and
    # epilogue - it wrote the first and had no way to know the second was
    # supposed to be in scope. A row like that can never match, and it reads
    # as a hard recovery rather than a catalogue defect.
    split = split_spans(rows)
    for line in split:
        print(f"  {line}")
    print(f"span-termination: {len(split)} span(s) holding two functions")
    return 1 if found or wrong or split else 0


if __name__ == "__main__":
    raise SystemExit(main())
