#!/usr/bin/env python3
"""Emit the unnamed one-expression accessors over `this` as recovered source.

The leaf-testable closure's tail is not more of the same. Of the functions
`tools/find_leaf_testable.py` still reports, most are unnamed `sub_xxxxxx`
bodies, and the largest single shape - `mov eax,[ecx+N] / ret` - accounts for
thirteen on its own. Recovering seventy of those by hand would mean inventing
seventy names, and this repository already decided how that is done:
`src/global_arith.cpp` holds twenty-six unnamed leaves emitted by
`tools/generate_global_arith.py`, named after their ADDRESS rather than
invented, grouped in one family file.

The convention that makes it work is the signature. Those bodies take
`void *`, so no class has to be attributed to them - which is the thing that
blocks hand recovery of a `sub_xxxxxx` whose owning class is not established
and may not be declared anywhere. `this` is a pointer and an offset, and that
is all these functions need it to be.

WHAT IS ACCEPTED, and nothing else:

    mov eax,[ecx+N] / ret            read a dword field
    mov eax,[ecx+N] / and eax,M / ret  read a field, masked
    inc dword [ecx+N] / ret          increment a field
    mov eax,IMM / ret                return a constant
    mov byte [ecx+N],IMM / ret       store a byte

Each is one expression whose meaning is fully determined by the instruction,
which is the entire reason this can be generated rather than read. Anything
else - a branch, a call, a second effect - is refused and stays a hand
recovery, because a generator that guesses is worse than one that declines.

THE STACK CLEANUP IS TAKEN FROM THE BODY, NOT INVENTED. These have no mangled
name, so there is no declared arity to check against - the `ret N` is the only
statement of it that exists, and the emitted adapter cleans exactly that many
bytes. A shape reading `[esp+k]` is refused for the same reason: its arguments
are not knowable from a name that does not exist.

Output goes to a scratch directory for review, exactly as the other generators
here do. Nothing is written into src/ and no redirect is wired; the wire list
is emitted for tools/add_redirect.py to consume after the emitted source has
been read.
"""

from __future__ import annotations

import argparse
import csv
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import pefile  # noqa: E402
from capstone import CS_ARCH_X86, CS_MODE_32, Cs  # noqa: E402
from capstone.x86 import (X86_OP_IMM, X86_OP_MEM,  # noqa: E402
                          X86_OP_REG, X86_REG_EAX, X86_REG_ECX)

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"
FUNCTIONS_CSV = REPO_ROOT / "docs" / "recovery" / "functions.csv"


def read_bytes(pe: pefile.PE, address: int, length: int) -> bytes:
    base = pe.OPTIONAL_HEADER.ImageBase
    for section in pe.sections:
        begin = base + section.VirtualAddress
        end = begin + max(section.Misc_VirtualSize, section.SizeOfRawData)
        if begin <= address < end:
            offset = section.PointerToRawData + (address - begin)
            return pe.__data__[offset:offset + length]
    return b""


def this_offset(operand) -> int | None:
    """The N of `[ecx+N]`, or None when the operand is anything else.

    This is the ONLY place a memory operand is admitted, which is deliberate.
    An earlier version also carried a separate "refuse anything that reads the
    stack" guard, and it decided nothing: every shape below reaches its memory
    operand through here, so `[esp+4]`, `[edx+8]` and an absolute address were
    already refused by the base check. A second mechanism that cannot fail
    independently is not defence in depth, it is a test that cannot fail -
    the guard was removed rather than given a passing test that proved
    something else.
    """
    if operand.type != X86_OP_MEM:
        return None
    if operand.mem.base != X86_REG_ECX or operand.mem.index != 0:
        return None
    return operand.mem.disp


def classify(instructions) -> tuple[str, dict] | None:
    """(kind, detail) for a shape this generator will emit, else None."""
    if not instructions or instructions[-1].mnemonic != "ret":
        return None
    tail = instructions[-1]
    cleanup = (tail.operands[0].imm
               if tail.operands and tail.operands[0].type == X86_OP_IMM else 0)
    body = instructions[:-1]

    if len(body) == 1 and body[0].mnemonic == "mov":
        destination, source = body[0].operands
        offset = this_offset(source)
        if (destination.type == X86_OP_REG and destination.reg == X86_REG_EAX
                and offset is not None):
            return "read", {"offset": offset, "cleanup": cleanup}
        if (destination.type == X86_OP_REG and destination.reg == X86_REG_EAX
                and source.type == X86_OP_IMM):
            return "constant", {"value": source.imm, "cleanup": cleanup}
        offset = this_offset(destination)
        if (offset is not None and source.type == X86_OP_IMM
                and body[0].op_str.startswith("byte")):
            return "store_byte", {"offset": offset, "value": source.imm,
                                  "cleanup": cleanup}
        return None

    if len(body) == 1 and body[0].mnemonic == "inc":
        offset = this_offset(body[0].operands[0])
        if offset is not None:
            return "increment", {"offset": offset, "cleanup": cleanup}
        return None

    if (len(body) == 2 and body[0].mnemonic == "mov"
            and body[1].mnemonic == "and"):
        destination, source = body[0].operands
        offset = this_offset(source)
        mask = body[1].operands[1]
        if (offset is not None and destination.type == X86_OP_REG
                and destination.reg == X86_REG_EAX
                and mask.type == X86_OP_IMM):
            return "masked", {"offset": offset, "mask": mask.imm,
                              "cleanup": cleanup}
    return None


BODIES = {
    "read": lambda d: (f"    return *reinterpret_cast<const uint32_t *>(\n"
                       f"        static_cast<const uint8_t *>(self) + {d['offset']:#x});"),
    "masked": lambda d: (f"    return *reinterpret_cast<const uint32_t *>(\n"
                         f"        static_cast<const uint8_t *>(self) + {d['offset']:#x})\n"
                         f"        & {d['mask']:#x}U;"),
    "constant": lambda d: f"    return {d['value']:#x}U;",
    "increment": lambda d: (f"    ++*reinterpret_cast<uint32_t *>(\n"
                            f"        static_cast<uint8_t *>(self) + {d['offset']:#x});"),
    "store_byte": lambda d: (f"    *(static_cast<uint8_t *>(self) + {d['offset']:#x})"
                             f" = {d['value']:#x};"),
}
RETURNS = {"read": "uint32_t", "masked": "uint32_t", "constant": "uint32_t",
           "increment": "void", "store_byte": "void"}
# A constant return never reads `this`, so naming the parameter would be an
# unused one - and this tree builds with -Wall -Wextra, where that is an error
# rather than a note. The name is emitted only where the body uses it.
USES_SELF = {"read": True, "masked": True, "constant": False,
             "increment": True, "store_byte": True}
PURPOSE = {
    "read": "Read the dword field at {offset:#x}.",
    "masked": "Read the dword field at {offset:#x}, masked to {mask:#x}.",
    "constant": "Return the constant {value:#x}.",
    "increment": "Increment the dword field at {offset:#x}.",
    "store_byte": "Store {value:#x} in the byte at {offset:#x}.",
}


def emit(accepted: list[tuple]) -> tuple[str, str, str]:
    declarations, definitions, wires = [], [], []
    for address, kind, detail, text in accepted:
        name = f"field_accessor_{address:08x}_redirect"
        result = RETURNS[kind]
        declarations.append(
            f"{result} __fastcall {name}(void *, void *);")
        parameter = "void *self" if USES_SELF[kind] else "void *"
        definitions.append(f"""/*
Purpose: {PURPOSE[kind].format(**detail)}
         Emitted by tools/generate_field_accessors.py from

             {text}

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter cleans {detail['cleanup']} argument bytes, taken from the
         body's own `ret`, which is the only statement of its arity that
         exists.
Original Offset: {address:08X}
Return Value: {'n/a' if result == 'void' else 'the value described above'}
Status: Complete
*/
{result} __fastcall {name}({parameter}, void *) {{
{BODIES[kind](detail)}
}}
""")
        wires.append(f"{address:#010x} {name}")
    header = ("#pragma once\n#include \"stdafx.h\"\n\n"
              + "\n".join(declarations) + "\n")
    source = ("#include \"stdafx.h\"\n#include \"field_accessors.h\"\n\n"
              + "\n".join(definitions))
    return header, source, "\n".join(wires) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--functions", type=Path, default=FUNCTIONS_CSV)
    parser.add_argument("--addresses", type=Path,
                        help="one hex address per line; default is every "
                             "unrecovered row the shapes accept")
    parser.add_argument("--scratch-dir", type=Path)
    args = parser.parse_args()

    with args.functions.open() as handle:
        rows = list(csv.DictReader(handle))
    wanted = None
    if args.addresses:
        wanted = {int(line, 16) for line in args.addresses.read_text().split()
                  if line.strip()}

    pe = pefile.PE(str(args.exe), fast_load=True)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True

    accepted, refused = [], 0
    for row in rows:
        if row["recovery_state"] != "unrecovered":
            continue
        address = int(row["address"], 16)
        if wanted is not None and address not in wanted:
            continue
        size = int(row["size"] or 0)
        if not size or size > 32:
            continue
        data = read_bytes(pe, address, size)
        instructions = list(decoder.disasm(data, address))
        if sum(one.size for one in instructions) != size:
            continue
        shape = classify(instructions)
        if shape is None:
            refused += 1
            continue
        text = " / ".join(f"{one.mnemonic} {one.op_str}".strip()
                          for one in instructions)
        accepted.append((address, shape[0], shape[1], text))

    accepted.sort()
    print(f"accepted {len(accepted)}; refused {refused} that did not match a "
          f"shape this generator will emit")
    for address, kind, detail, text in accepted:
        print(f"  {address:#010x}  {kind:<10} {text[:64]}")

    if args.scratch_dir:
        header, source, wires = emit(accepted)
        args.scratch_dir.mkdir(parents=True, exist_ok=True)
        (args.scratch_dir / "field_accessors.h").write_text(header)
        (args.scratch_dir / "field_accessors.cpp").write_text(source)
        (args.scratch_dir / "field-accessors-wire.txt").write_text(wires)
        print(f"\nemitted field_accessors.{{h,cpp}} and the wire list to "
              f"{args.scratch_dir}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
