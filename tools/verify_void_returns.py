#!/usr/bin/env python3
"""Which functions the catalogue calls `void` while their bytes return a status?

A RETURN TYPE IS INHERITED. The emitter declares every callee from its
catalogued name, so one function spelled `void` that really returns a failure
code teaches that lie to every caller's scaffolding at once - measured at 47
callers for `?init@GraphicWin@@QAEX...`, which is what made this worth a tool
rather than a correction. The body compiles either way; what breaks is the
caller that cannot see the value it is supposed to test.

That one was found by reading disassembly, fixed by hand, and the check added
alongside it did not generalise: an agent recovering `Cursor::init`
(0x0063B2E0) found the identical defect two batches later, again by reading
disassembly. So the question is asked here of every catalogued function at
once.

THE SIGNAL: a constant loaded into EAX immediately before a JUMP or a `ret`.

The first version of this looked for the last write to EAX before each `ret`,
and reported ZERO on `?init@Cursor@@QAEX...` - a function an agent had already
proved returns five distinct codes. The model was wrong: cl 12.00.8168 at /O2
emits ONE shared epilogue, so `return 3` is `mov eax, 3; jmp <epilogue>` and
the whole function has a single `ret`. A walk that only looks at `ret` sees one
constant no matter how many the source returned.

So the shape is `mov eax, <imm>` (or `xor eax, eax`) whose very next
instruction transfers control AND whose destination reaches a `ret` without
touching EAX again. Two or more DISTINCT constants like that is a status
return. One is not evidence: EAX is the scratch register and holds the last
computation whether or not anybody reads it.

FOLLOWING THE JUMP IS WHAT MAKES THIS USABLE. Counting every `mov eax, N; jmp`
without checking where it lands reported `?base_production@@YAXXZ` as
returning sixteen different values, four of them addresses - because that is
also how a switch arm hands a value to shared code that STORES it. With the
destination followed, that function drops to zero and `Cursor::init` keeps
exactly the five codes an agent had already read by hand.

Still A REVIEW AID, NOT A GATE, and deliberately not registered as one. One
straight-line run from the jump target is not real dataflow: a constant that
survives to a `ret` down one path may be overwritten down another this does
not walk. Read the disassembly before changing a name. A gate nobody can make
green is worse than no gate - tools/derive_array_strides.py carries the same
warning for the same reason.

    tools/verify_void_returns.py            # ranked, most exit paths first
    tools/verify_void_returns.py --min 3    # only stronger candidates
"""
from __future__ import annotations

import argparse
import collections
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import pefile                                                 # noqa: E402
from capstone import CS_ARCH_X86, CS_MODE_32, Cs              # noqa: E402

import byte_match                                             # noqa: E402
import declfix                                                # noqa: E402
import emit_translation_unit as emit                          # noqa: E402

SPAN = re.compile(r"0x([0-9A-Fa-f]{8})-0x([0-9A-Fa-f]{8})")
# `mov eax, 3` and `xor eax, eax` are the two spellings of a constant return.
MOV_CONST = re.compile(r"^eax, (?:0x([0-9a-f]+)|(\d+))$")
# Anything that writes EAX without making it a known constant.
EAX_WRITER = re.compile(r"^(?:eax|ax|al|ah)\b")


def image():
    pe = pefile.PE(str(byte_match.DEFAULT_EXE), fast_load=True)
    section = [s for s in pe.sections if s.Name.startswith(b".text")][0]
    return (section.get_data(),
            pe.OPTIONAL_HEADER.ImageBase + section.VirtualAddress)


def returns_void(name: str) -> bool:
    """Does the catalogued NAME encode a void return?

    Read through `declfix.decode_signature`, the same decoder the emitter uses,
    so this cannot disagree with what the scaffolding actually declares. A name
    it refuses is skipped rather than guessed at.
    """
    if not name or not name.startswith("?"):
        return False
    decoded = declfix.decode_signature(name)
    return bool(decoded) and decoded[0] == "void"


def survives_to_ret(blocks, target, budget=40) -> bool:
    """From `target`, is a `ret` reached without EAX being rewritten?

    THE DISCRIMINATOR, and without it this tool is not usable. `mov eax, N;
    jmp X` is ALSO how the compiler passes a value to shared code that stores
    it somewhere - a switch arm loading a table index, a common tail that
    writes a field. Counting those made `?base_production@@YAXXZ` look like it
    returned 16 different values, four of them addresses.

    A RETURN is the case where the jumped-to code runs to `ret` and leaves EAX
    alone on the way. Following one straight-line run from the target is
    enough: the shared epilogue this is looking for is a handful of pops.
    """
    seen = 0
    while target in blocks and seen < budget:
        mnemonic, operands, nxt = blocks[target]
        seen += 1
        if mnemonic == "ret":
            return True
        if mnemonic == "call" or EAX_WRITER.match(operands):
            return False
        if mnemonic == "jmp":
            try:
                target = int(operands, 16)
            except ValueError:
                return False
            continue
        target = nxt
    return False


def exit_constants(data, base, engine, spans) -> tuple:
    """({constant: times it precedes a transfer}, rets seen, exits seen).

    Linear, deliberately: a real dataflow pass over this image is a much larger
    tool, and the shape wanted here survives a linear read because the constant
    sits immediately before the control transfer that carries it away.
    """
    found, rets, exits = collections.Counter(), 0, 0
    for low, high in spans:
        if not base <= low < base + len(data):
            continue
        window = data[low - base:min(high, base + len(data)) - base]
        # Indexed first so a `jmp` can be followed to see whether the constant
        # it carries actually survives to a `ret`.
        decoded = list(engine.disasm(window, low))
        blocks = {}
        for index, one in enumerate(decoded):
            nxt = decoded[index + 1].address if index + 1 < len(decoded) else None
            blocks[one.address] = (one.mnemonic, one.op_str, nxt)
        pending = None                    # constant loaded into EAX just now
        for one in decoded:
            if one.mnemonic == "ret":
                rets += 1
                if pending is not None:
                    found[pending] += 1
                    exits += 1
                pending = None
                continue
            # AN UNCONDITIONAL JUMP IS AN EXIT ONLY IF IT REACHES A `ret`
            # with EAX untouched. With one shared epilogue that is how every
            # `return <constant>` but the last one leaves - but it is also how
            # a switch arm hands a value to shared code that stores it, which
            # is why the target has to be followed rather than assumed.
            if one.mnemonic == "jmp":
                if pending is not None:
                    try:
                        target = int(one.op_str, 16)
                    except ValueError:
                        target = None
                    if target is not None and survives_to_ret(blocks, target):
                        found[pending] += 1
                        exits += 1
                pending = None
                continue
            if one.mnemonic == "xor" and one.op_str == "eax, eax":
                pending = 0
                continue
            if one.mnemonic == "mov":
                hit = MOV_CONST.match(one.op_str)
                if hit:
                    pending = int(hit.group(1), 16) if hit.group(1) \
                        else int(hit.group(2))
                    continue
            # A conditional jump does not consume the constant - `mov eax,1`
            # then `jne` keeps it live into the fallthrough - but anything
            # that writes EAX destroys it, and so does a call.
            if one.mnemonic.startswith("j"):
                continue
            if one.mnemonic == "call" or EAX_WRITER.match(one.op_str):
                pending = None
    return found, rets, exits


def candidates(minimum: int = 2) -> list:
    data, base = image()
    engine = Cs(CS_ARCH_X86, CS_MODE_32)
    out = []
    for address, row in sorted(emit.load_functions().items()):
        name = row.get("name") or ""
        if not returns_void(name):
            continue
        spans = [(int(a, 16), int(b, 16))
                 for a, b in SPAN.findall(row.get("body_ranges") or "")]
        if not spans:
            continue
        found, rets, exits = exit_constants(data, base, engine, spans)
        if len(found) >= minimum and sum(found.values()) >= minimum:
            out.append({
                "address": address,
                "name": name,
                "constants": dict(found),
                "rets": rets,
                "constant_exits": exits,
            })
    out.sort(key=lambda r: (-len(r["constants"]), -sum(r["constants"].values())))
    return out


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--min", type=int, default=2,
                        help="distinct constants required (default 2)")
    parser.add_argument("--limit", type=int, default=40)
    arguments = parser.parse_args()

    if not byte_match.DEFAULT_EXE.is_file():
        print("SKIP: the pinned executable is absent")
        return 0

    found = candidates(arguments.min)
    for row in found[:arguments.limit]:
        values = ", ".join(
            f"{value}" + (f" x{count}" if count > 1 else "")
            for value, count in sorted(row["constants"].items()))
        print(f"0x{row['address']:08X}  {row['name'][:52]}")
        print(f"    name says void; EAX set to {values} immediately before a "
              f"jump or ret   ({row['constant_exits']} such exit(s), "
              f"{row['rets']} ret(s))")
    print(f"\n{len(found)} function(s) the catalogue calls void whose bytes "
          f"set EAX to {arguments.min}+ distinct constants on exit.")
    print("A REVIEW AID, not a gate. The jump target IS followed - that is "
          "what took base_production from 16 bogus constants to zero - but "
          "one straight-line run is not real dataflow. Read the disassembly "
          "before changing a name.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
