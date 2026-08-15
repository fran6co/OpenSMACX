#!/usr/bin/env python3
"""Which C++ objects does this function's stack frame hold, and where?

THE SINGLE BIGGEST SOURCE OF WASTED EFFORT in the last three batches was
agents guessing which locals a large RAII function declares and how they nest.
A `Popup` local carries a `Scroll`, two `FlatButton`, a `Spot`, a `Sprite`, a
`Heap` and a `Dialogs` region with seven more objects inside it - and from the
disassembly alone every one of those looks like an independent local. Declare
them separately and the frame is wrong, the teardown is wrong, and the body
diverges at instruction zero.

The image answers the question exactly. Every destructor call names its
receiver, so `lea ecx, [ebp - 0x5488]` before `call ??1BasePop@@QAE@XZ` says
there is a `BasePop`-rooted object at that frame offset - and the compiler
emitted that instruction from the real layout, so it cannot be wrong about it.

WHAT IT PROVES, on 0x004D61A0, mechanically and in one pass:

    [ebp-0x5488] -> BasePop        the Popup's own base
    [ebp-0x2258] -> Scroll         0x5488 - 0x2258 = 0x3230 = sizeof(BasePop)
    [ebp-0x2258] -> GraphicWin     the Scroll's own base, same address
    [ebp-0xc60]  -> BaseButton     inside it
    [ebp-0x23f0] -> Spot

Two receivers at ONE address are a base and its derived class, not two
objects. Two receivers whose difference is a known class size are a container
and its member. An agent reading that table writes `Popup popup;` and stops,
where before it wrote a dozen locals and got the frame wrong.

READ FROM THE ORDINARY PATH, NOT THE EH FUNCLETS. The unwind thunks carry the
same information and an agent used them to great effect, but they live in the
excluded 0x0065xxxx region and associating a thunk with its function needs the
FuncInfo tables decoded. The destructor calls on the normal path are inside the
span this tool is already handed, say the same thing, and need nothing decoded.

A RECEIVER THROUGH A REGISTER IS REPORTED SEPARATELY. `lea ecx, [eax + 0x8c]`
names an offset inside whatever `eax` points at, which is a SUB-OBJECT
relationship rather than a frame slot, and conflating the two would invent
locals that do not exist.
"""
from __future__ import annotations

import argparse
import collections
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import emit_translation_unit as emit                          # noqa: E402

# `lea ecx, [ebp - 0x5488]` / `lea ecx, [esp + 0x20]`
FRAME_RECEIVER = re.compile(
    r"^ecx, \[(?P<base>ebp|esp) (?P<sign>[-+]) (?P<offset>0x[0-9a-f]+)\]$")
# `lea ecx, [eax + 0x8c]` - inside another object, not a frame slot.
FIELD_RECEIVER = re.compile(
    r"^ecx, \[(?P<reg>e[abcds][xip]) \+ (?P<offset>0x[0-9a-f]+)\]$")
# `??1Popup@@QAE@XZ` -> Popup
DESTRUCTOR = re.compile(r"^\?\?1(\w+)@@")
# How far back to look for the receiver that a `call` consumes.
WINDOW = 6


def destructors(functions: dict) -> dict:
    """{address: class name} for every catalogued destructor."""
    out = {}
    for address, row in functions.items():
        matched = DESTRUCTOR.match(row.get("name") or "")
        if matched:
            out[address] = matched.group(1)
    return out


def scan(pe, spans, functions: dict) -> tuple:
    """(frame objects, sub-objects) as {offset or (reg, offset): [classes]}."""
    from capstone import CS_ARCH_X86, CS_MODE_32, Cs
    import jump_tables

    engine = Cs(CS_ARCH_X86, CS_MODE_32)
    known = destructors(functions)
    frame, fields = collections.defaultdict(list), collections.defaultdict(list)
    for low, high in spans:
        data = jump_tables._read(pe, low, high - low)
        if not data:
            continue
        window = list(engine.disasm(data, low))
        for index, one in enumerate(window):
            if one.mnemonic != "call":
                continue
            try:
                target = int(one.op_str, 16)
            except ValueError:
                continue
            klass = known.get(target)
            if klass is None:
                continue
            for back in range(index - 1, max(index - WINDOW, -1), -1):
                previous = window[back]
                if previous.mnemonic not in ("lea", "mov"):
                    continue
                hit = FRAME_RECEIVER.match(previous.op_str)
                if hit:
                    sign = -1 if hit.group("sign") == "-" else 1
                    offset = sign * int(hit.group("offset"), 16)
                    if klass not in frame[(hit.group("base"), offset)]:
                        frame[(hit.group("base"), offset)].append(klass)
                    break
                hit = FIELD_RECEIVER.match(previous.op_str)
                if hit:
                    key = (hit.group("reg"), int(hit.group("offset"), 16))
                    if klass not in fields[key]:
                        fields[key].append(klass)
                    break
                if previous.op_str.startswith("ecx"):
                    break
    return frame, fields


def render(pe, spans, functions: dict) -> str:
    """The frame map for the brief, or "" when the function holds no objects."""
    frame, fields = scan(pe, spans, functions)
    if not frame and not fields:
        return ""
    # THE SIZES THE TREE HAS PINNED, from the `static_assert`s themselves -
    # the same source `derive_array_strides --check` reads. A gap that equals
    # one of these is the whole point of printing gaps at all.
    try:
        import derive_array_strides
        sizes = derive_array_strides.asserted()
    except Exception:
        sizes = {}
    lines = ["", "# C++ objects this function's frame holds",
             "",
             "Read from the image: every destructor call names its receiver,",
             "and the compiler emitted that instruction from the real layout.",
             "TWO CLASSES AT ONE OFFSET are a base and its derived class, not",
             "two objects. A gap between two offsets that equals a known class",
             "size is a container and its member - declare the OUTER one and",
             "let the compiler generate the rest, rather than declaring each",
             "separately and getting the frame wrong.",
             ""]
    for (base, offset), classes in sorted(frame.items(), key=lambda kv: kv[0][1]):
        spelled = f"[{base} {'-' if offset < 0 else '+'} 0x{abs(offset):X}]"
        lines.append(f"    {spelled:22} {', '.join(classes)}")
    if len(frame) > 1:
        ordered = sorted(frame, key=lambda key: key[1])
        lines.append("")
        lines.append("  gaps between frame slots (a gap equal to a class size")
        lines.append("  means the lower object is EMBEDDED in the higher one):")
        for first, second in zip(ordered, ordered[1:]):
            gap = second[1] - first[1]
            matches = sorted(name for name, size in sizes.items()
                             if size == gap)
            note = f"   = sizeof({matches[0]})" if matches else ""
            lines.append(f"    0x{abs(first[1]):X} -> 0x{abs(second[1]):X}"
                         f"  gap 0x{gap:X}{note}")
    if fields:
        lines.append("")
        lines.append("  reached through a register, so these are SUB-OBJECTS")
        lines.append("  of something else rather than frame slots:")
        for (reg, offset), classes in sorted(fields.items()):
            lines.append(f"    [{reg} + 0x{offset:X}]        {', '.join(classes)}")
    return "\n".join(lines)


def main() -> int:
    import pefile
    import byte_match

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("address")
    arguments = parser.parse_args()
    address = int(arguments.address, 16)

    functions = emit.load_functions()
    row = functions.get(address)
    if row is None:
        print(f"0x{address:08X} is not catalogued", file=sys.stderr)
        return 2
    pe = pefile.PE(str(byte_match.DEFAULT_EXE), fast_load=True)
    text = render(pe, emit.parse_body_ranges(row.get("body_ranges") or ""),
                  functions)
    print(text or f"0x{address:08X}'s frame holds no catalogued C++ objects")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
