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
    xor eax,eax / ret                return zero
    ret                              a body that does nothing at all
    a SEQUENCE of stores of constants to [this+N], optionally preceded by
    `mov eax,ecx` and register loads of constants, ending in `ret` - the shape
    every one of these small constructors has

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

    if not body:
        # A body that is only its `ret`. It does nothing and returns nothing,
        # and the ONLY thing there is to get wrong is the cleanup - which is
        # why it is emitted rather than dismissed as trivial.
        return "nothing", {"cleanup": cleanup}

    if (len(body) == 1 and body[0].mnemonic == "xor"
            and len(body[0].operands) == 2
            and all(operand.type == X86_OP_REG for operand in body[0].operands)
            and body[0].operands[0].reg == body[0].operands[1].reg
            and body[0].operands[0].reg == X86_REG_EAX):
        return "constant", {"value": 0, "cleanup": cleanup}

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
        # Deliberately NOT `return None` here: a single dword store to
        # [this+N] is a one-element store sequence, and returning early meant
        # the sequence check below never saw it. `mov dword [ecx+0x200],0 /
        # ret` was refused for that reason alone.

    if len(body) == 1 and body[0].mnemonic == "inc":
        offset = this_offset(body[0].operands[0])
        if offset is not None:
            return "increment", {"offset": offset, "cleanup": cleanup}

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

    sequence = store_sequence(instructions)
    if sequence is not None:
        sequence["cleanup"] = cleanup
        sequence["count"] = len(sequence["stores"])
        return "stores", sequence
    return None


def store_sequence(instructions) -> dict | None:
    """A run of stores of CONSTANTS to `[this+N]`, and nothing else.

    Tracked symbolically rather than pattern matched, because the register a
    store reads is set up earlier and by no fixed instruction: these bodies
    open `mov eax,ecx` to alias `this` (which is also the EAX = this residue),
    then `xor ecx,ecx` or `mov edx,IMM` to hold the value, then store it to a
    series of offsets. Only three facts are ever needed - which registers alias
    `this`, which hold a known constant, and what EAX ends up as - so anything
    that would make one of them unknown is refused rather than guessed.
    """
    aliases = {X86_REG_ECX}          # `this` on entry
    constants: dict[int, int] = {}
    stores: list[tuple[int, int]] = []

    for one in instructions[:-1]:
        operands = one.operands
        if one.mnemonic == "xor" and len(operands) == 2:
            if (operands[0].type != X86_OP_REG or operands[1].type != X86_OP_REG
                    or operands[0].reg != operands[1].reg):
                return None
            constants[operands[0].reg] = 0
            aliases.discard(operands[0].reg)
            continue
        if one.mnemonic != "mov" or len(operands) != 2:
            return None
        destination, source = operands
        if destination.type == X86_OP_REG:
            if source.type == X86_OP_IMM:
                constants[destination.reg] = source.imm
                aliases.discard(destination.reg)
            elif source.type == X86_OP_REG and source.reg in aliases:
                aliases.add(destination.reg)
                constants.pop(destination.reg, None)
            else:
                return None
            continue
        if destination.type != X86_OP_MEM or destination.mem.index != 0:
            return None
        if destination.mem.base not in aliases:
            return None
        width = 4 if one.op_str.startswith("dword") else None
        if width is None:
            return None                      # only dword stores, for now
        if source.type == X86_OP_IMM:
            stores.append((destination.mem.disp, source.imm))
        elif source.type == X86_OP_REG and source.reg in constants:
            stores.append((destination.mem.disp, constants[source.reg]))
        else:
            return None                      # storing something unknown
    if not stores:
        return None
    # No separate "EAX holds something undescribable" guard, and that is on
    # purpose. Every write to EAX above is already either an immediate, a
    # this-alias, or `xor eax,eax`; anything else returned None at the point it
    # was read. So such a guard could never fire, and disabling it failed no
    # test - the same dead-mechanism the stack-read check turned out to be.
    # One mechanism, one test.
    returns_this = X86_REG_EAX in aliases
    return {"stores": stores, "returns_this": returns_this,
            "eax": None if returns_this else constants.get(X86_REG_EAX)}


def render_stores(detail: dict) -> str:
    lines = ["    uint8_t *const bytes = static_cast<uint8_t *>(self);"]
    for offset, value in detail["stores"]:
        lines.append(f"    *reinterpret_cast<uint32_t *>(bytes + {offset:#x})"
                     f" = {value:#x}U;")
    if detail["returns_this"]:
        lines.append("    return self;")
    elif detail["eax"]:
        lines.append(f"    return {detail['eax']:#x}U;")
    return "\n".join(lines)


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
    "stores": render_stores,
    "nothing": lambda d: "",
}
RETURNS = {"read": "uint32_t", "masked": "uint32_t", "constant": "uint32_t",
           "increment": "void", "store_byte": "void", "nothing": "void"}
# A constant return never reads `this`, so naming the parameter would be an
# unused one - and this tree builds with -Wall -Wextra, where that is an error
# rather than a note. The name is emitted only where the body uses it.
USES_SELF = {"read": True, "masked": True, "constant": False,
             "increment": True, "store_byte": True, "stores": True,
             "nothing": False}
PURPOSE = {
    "read": "Read the dword field at {offset:#x}.",
    "masked": "Read the dword field at {offset:#x}, masked to {mask:#x}.",
    "constant": "Return the constant {value:#x}.",
    "increment": "Increment the dword field at {offset:#x}.",
    "store_byte": "Store {value:#x} in the byte at {offset:#x}.",
    "stores": "Set {count} field(s) to constants.",
    "nothing": "Do nothing; the original body is only its `ret`.",
}


def emit(accepted: list[tuple]) -> tuple[str, str, str]:
    declarations, definitions, wires = [], [], []
    for address, kind, detail, text in accepted:
        name = f"field_accessor_{address:08x}_redirect"
        result = RETURNS.get(kind)
        if result is None:      # a store sequence: void, or the residue
            result = "void *" if detail["returns_this"] else (
                "uint32_t" if detail.get("eax") else "void")
        declarations.append(
            f"{result} __fastcall {name}(void *, void *"
            + "".join(", int" for _ in range(detail["cleanup"] // 4)) + ");")
        parameter = "void *self" if USES_SELF[kind] else "void *"
        # THE STACK ARGUMENTS MUST BE DECLARED, or the adapter cleans nothing.
        #
        # __fastcall passes the first two integer parameters in ECX and EDX and
        # leaves the CALLEE to clean the rest off the stack. An adapter
        # declaring only (void *, void *) therefore compiles to a bare `ret`,
        # and replacing a body whose original ends `ret 0x14` then leaves
        # twenty bytes on the caller's stack every call. That is the failure
        # the working rules call the worst in this project, because the crash
        # lands in an unrelated function later.
        #
        # This was shipped once: five of the first sixteen accessors carried
        # `ret 4`, `ret 12`, `ret 16` and `ret 20`, and the comment below
        # claimed the cleanup was "taken from the body's own ret" while the
        # signature ignored it. The fix is to declare that many dwords, which
        # is how every hand-written redirect in this tree spells the same
        # thing.
        extra = "".join(f", int stack{index}"
                        for index in range(detail["cleanup"] // 4))
        if detail["cleanup"] % 4:
            raise SystemExit(
                f"{address:#010x} cleans {detail['cleanup']} bytes, which is "
                f"not a whole number of dwords; refusing to guess a signature")
        definitions.append(f"""/*
Purpose: {PURPOSE[kind].format(**detail)}
         Emitted by tools/generate_field_accessors.py from

             {text}

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares {detail['cleanup'] // 4} stack dword(s) so it
         cleans {detail['cleanup']} bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
Original Offset: {address:08X}
Return Value: {'n/a' if result == 'void' else 'the value described above'}
Status: Complete
*/
{result} __fastcall {name}({parameter}, void *{extra}) {{
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
        if not size or size > 64:
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
