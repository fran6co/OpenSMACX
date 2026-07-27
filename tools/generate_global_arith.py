#!/usr/bin/env python3
"""Generate complete recoveries for absolute-operand arithmetic leaves.

Twenty-odd functions in the image touch nothing but fixed globals: they load a
few, add or subtract, store the result, and return. No register is live on
entry, no memory operand is register-relative, there is no branch, and the
body ends in a bare `ret`.

Rather than a template per shape - there are about eleven shapes for
twenty-three functions, with a tail of one-offs - this INTERPRETS the
instruction list over a symbolic register file and emits ONE C statement per
instruction, in order. That is both more faithful (the original's store order
is preserved exactly, including a load that must see an earlier store) and
better for the mutation sweep, which gets a perturbable line per instruction
instead of one folded expression.

The opcode set is deliberately tiny and everything outside it is REFUSED and
printed, the same discipline the delegation and nullsub generators follow.

THE ADDRESSES ARE BOUND, NEVER INLINED. Every global these functions touch
lives in the original image and is unmapped in the standalone leaf-test
process. A body dereferencing 0x008577EC directly compiles, passes the hybrid
gate, and page-faults the moment the leaf suite calls it - the same latent
shape as the BufferResetValue520 and BufferPalette faults already fixed
elsewhere in this tree. So each distinct address becomes a named rebindable
pointer that a fixture can repoint at local storage.

Calling convention is undecidable here and, as with the do-nothing leaves,
irrelevant: every body ends in a bare `ret` (callee-pop 0) and none reads ECX
or EDX, so a __fastcall redirect taking (self, unused) is byte-identical to
any zero-argument convention.

Output is derived from proprietary bytes: local analysis only, never
committed.
"""
from __future__ import annotations

import argparse
import csv
import re
from pathlib import Path

import capstone
import pefile

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"
FUNCTIONS_CSV = REPO_ROOT / "docs" / "recovery" / "functions.csv"
JUMP_PATCH_BYTES = 5
OWN_SOURCE = "src/global_arith.cpp"
REGISTERS = ("eax", "ebx", "ecx", "edx", "esi", "edi")


class Unsettled(Exception):
    """An instruction this generator refuses to guess at."""


def read_bytes(pe: pefile.PE, address: int, length: int) -> bytes:
    base = pe.OPTIONAL_HEADER.ImageBase
    for section in pe.sections:
        low = section.VirtualAddress + base
        high = low + max(section.Misc_VirtualSize, section.SizeOfRawData)
        if low <= address < high:
            return section.get_data()[address - low:address - low + length]
    return b""


def _imm(text: str) -> int:
    text = text.strip()
    negative = text.startswith("-")
    if negative:
        text = text[1:]
    value = int(text, 16) if text.startswith("0x") else int(text)
    return -value if negative else value


ABSOLUTE = re.compile(r"^dword ptr \[(0x[0-9a-f]+)\]$")


def transcribe(code: bytes, address: int) -> tuple[list[str], set[int]]:
    """(C statements, the addresses touched). Raises Unsettled on anything
    outside the supported opcode set."""
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    text = [f"{i.mnemonic} {i.op_str}".rstrip()
            for i in md.disasm(code, address)]
    if not text or text[-1] != "ret":
        raise Unsettled("does not end in a bare ret")
    statements: list[str] = []
    touched: set[int] = set()
    live: set[str] = set()

    def slot(target: int) -> str:
        touched.add(target)
        return f"*GlobalSlot{target:08X}"

    for entry in text[:-1]:
        found = re.match(r"^(\w+) (.+)$", entry)
        if not found:
            raise Unsettled(f"unreadable: {entry}")
        opcode, operands = found.group(1), found.group(2)
        parts = [part.strip() for part in operands.split(",", 1)]
        if len(parts) != 2:
            raise Unsettled(f"unsupported operand count: {entry}")
        left, right = parts

        def value(operand: str) -> str:
            memory = ABSOLUTE.match(operand)
            if memory:
                return slot(int(memory.group(1), 16))
            if operand in REGISTERS:
                if operand not in live:
                    raise Unsettled(f"reads a register live on entry: {entry}")
                return operand
            # Anything that is neither an absolute slot, a register we have
            # already defined, nor a plain immediate is refused rather than
            # coerced - a register-relative operand reaching the immediate
            # parser is exactly how a body with a real memory access would
            # otherwise slip through.
            if not re.fullmatch(r"-?(0x[0-9a-f]+|\d+)", operand):
                raise Unsettled(f"unsupported operand: {entry}")
            return str(_imm(operand))

        if opcode == "mov":
            memory = ABSOLUTE.match(left)
            if memory:
                statements.append(
                    f"{slot(int(memory.group(1), 16))} = {value(right)};")
            elif left in REGISTERS:
                statements.append(
                    ("int32_t " if left not in live else "")
                    + f"{left} = {value(right)};")
                live.add(left)
            else:
                raise Unsettled(f"unsupported destination: {entry}")
        elif opcode in ("add", "sub", "xor", "and", "or"):
            symbol = {"add": "+", "sub": "-", "xor": "^", "and": "&",
                      "or": "|"}[opcode]
            memory = ABSOLUTE.match(left)
            if memory:
                target = slot(int(memory.group(1), 16))
                statements.append(f"{target} = {target} {symbol} "
                                  f"{value(right)};")
            elif left in REGISTERS:
                if left not in live:
                    raise Unsettled(f"reads a register live on entry: {entry}")
                statements.append(f"{left} = {left} {symbol} {value(right)};")
            else:
                raise Unsettled(f"unsupported destination: {entry}")
        elif opcode == "lea":
            doubling = re.match(r"^\[(\w+) \+ (\w+)\]$", right)
            if not (left in REGISTERS and doubling
                    and doubling.group(1) == doubling.group(2)
                    and doubling.group(1) in live):
                raise Unsettled(f"unsupported lea: {entry}")
            statements.append(
                ("int32_t " if left not in live else "")
                + f"{left} = {doubling.group(1)} + {doubling.group(1)};")
            live.add(left)
        else:
            raise Unsettled(f"unsupported opcode: {entry}")
    if not statements:
        raise Unsettled("no statements")
    if not touched:
        raise Unsettled("touches no global")
    return statements, touched


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--scratch-dir", type=Path)
    parser.add_argument("--source-dir", type=Path)
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()

    pe = pefile.PE(str(args.exe), fast_load=True)
    with FUNCTIONS_CSV.open() as handle:
        rows = list(csv.DictReader(handle))
    ordered = sorted(int(row["address"], 16) for row in rows)
    following = {address: ordered[index + 1]
                 for index, address in enumerate(ordered[:-1])}

    accepted, skipped, addresses = [], [], set()
    for row in rows:
        owned = OWN_SOURCE in row["source_locations"]
        if row["recovery_state"] != "unrecovered" and not owned:
            continue
        if row["call_target_count"] != "0":
            continue
        size = int(row["size"] or 0)
        if not 4 <= size <= 48:
            continue
        address = int(row["address"], 16)
        body = read_bytes(pe, address, size)
        if b"\xc3" not in body:
            continue
        try:
            statements, touched = transcribe(body, address)
        except Unsettled as reason:
            # Only report candidates that at least look like this family;
            # everything else in the image is simply not our business.
            if b"\xa1" in body[:1] or b"\x8b\x0d" in body[:2]:
                skipped.append((row, str(reason)))
            continue
        if row["name"].startswith("?"):
            skipped.append((row, "mangled name - belongs to its class"))
            continue
        slack = following.get(address, address + size) - address
        if slack < JUMP_PATCH_BYTES:
            skipped.append((row, f"only {slack}B of slack"))
            continue
        accepted.append((row, statements, touched))
        addresses |= touched

    accepted.sort(key=lambda entry: int(entry[0]["address"], 16))
    print(f"{len(accepted)} absolute-operand arithmetic leaves, "
          f"{len(addresses)} distinct globals")
    for row, statements, touched in accepted:
        print(f"  {row['address']}  {len(statements)} stmt  "
              f"{len(touched)} globals  {row['name']}")
    if skipped:
        print(f"\n{len(skipped)} skipped - never guessed, always named:")
        for row, reason in sorted(skipped, key=lambda e: e[0]["address"]):
            print(f"  {row['address']}  {row['name'][:34]:<34} {reason}")

    if args.dry_run:
        return 0
    header, source, fragment, wires = emit(accepted, sorted(addresses))
    if args.source_dir:
        args.source_dir.mkdir(parents=True, exist_ok=True)
        (args.source_dir / "global_arith.h").write_text(header)
        (args.source_dir / "global_arith.cpp").write_text(source)
    if args.scratch_dir:
        args.scratch_dir.mkdir(parents=True, exist_ok=True)
        (args.scratch_dir / "global-arith-tests.cpp").write_text(fragment)
        (args.scratch_dir / "global-arith-wire.txt").write_text(wires)
    print("\nemitted global_arith.{h,cpp}, test fragment, wire list")
    return 0


LICENCE = (REPO_ROOT / "src" / "vector_teardown.h").read_text().split(
    "#pragma once")[0]

BANNER = """/*
 * Absolute-operand arithmetic leaves - GENERATED by
 * tools/generate_global_arith.py. Do not edit by hand; edit the generator and
 * regenerate.
 *
 * %d functions that read fixed globals, compute, store, and return. No
 * register is live on entry and no memory operand is register-relative, so
 * one C statement per original instruction reproduces them exactly - store
 * order included, which matters wherever a load must see an earlier store.
 *
 * The %d globals below are BOUND rather than inlined. Every one lives in the
 * original image and is unmapped in a standalone test process, so a body that
 * dereferenced its address directly would compile, pass the hybrid gate, and
 * fault the moment a leaf test called it.
 */
"""


def simulate(statements, addresses, seed):
    """Final slot image after running one body over a seeded image.

    This exists so the leaf test can compare against a value rather than only
    against itself. The first fixture checked that the batch was a
    deterministic function of its seed, which every mutant also satisfies -
    dropping an `add` still stores something, still deterministically. A
    golden checksum kills those. Its authority comes from the
    statement-by-statement verification of the emitted bodies against the
    disassembly, not from the bodies themselves.
    """
    index = {target: position for position, target in enumerate(addresses)}
    image = [(seed + position * 7) & 0xFFFFFFFF
             for position in range(len(addresses))]
    registers = {}

    def evaluate(token):
        token = token.strip().rstrip(";")
        if token.startswith("*GlobalSlot"):
            return image[index[int(token[len("*GlobalSlot"):], 16)]]
        if token in REGISTERS:
            return registers[token]
        return int(token) & 0xFFFFFFFF

    for statement in statements:
        left, right = statement.split(" = ", 1)
        right = right.rstrip(";")
        pieces = right.split(" ")
        if len(pieces) == 1:
            result = evaluate(pieces[0])
        else:
            first, operator, second = pieces[0], pieces[1], pieces[2]
            a, b = evaluate(first), evaluate(second)
            result = {"+": a + b, "-": a - b, "^": a ^ b, "&": a & b,
                      "|": a | b}[operator]
        result &= 0xFFFFFFFF
        left = left.replace("int32_t ", "").strip()
        if left.startswith("*GlobalSlot"):
            image[index[int(left[len("*GlobalSlot"):], 16)]] = result
        else:
            registers[left] = result
    return image


def checksum(image):
    """FNV-1a over the slot image, so one number stands for the whole state."""
    value = 0x811C9DC5
    for entry in image:
        for shift in (0, 8, 16, 24):
            value ^= (entry >> shift) & 0xFF
            value = (value * 0x01000193) & 0xFFFFFFFF
    return value


def symbol_for(row) -> str:
    name = row["name"]
    if re.fullmatch(r"(nullsub|sub)_[0-9a-fA-F]+", name):
        return f"global_arith_{int(row['address'], 16):08x}"
    return re.sub(r"\W", "_", name).lower()


def emit(accepted, addresses):
    banner = BANNER % (len(accepted), len(addresses))
    lines = [LICENCE, "#pragma once", "", banner]
    for target in addresses:
        lines.append(f"extern int32_t *GlobalSlot{target:08X};")
    lines.append("")
    for row, _, _ in accepted:
        lines.append(f"void __fastcall {symbol_for(row)}_redirect("
                     "void *self, void *);")

    body = [LICENCE, '#include "stdafx.h"', '#include "global_arith.h"',
            "", banner]
    for target in addresses:
        body.append(f"int32_t *GlobalSlot{target:08X} = "
                    f"reinterpret_cast<int32_t *>(0x{target:08X});")
    body.append("")
    wire, cases = [], []
    for row, statements, touched in accepted:
        symbol = symbol_for(row)
        address = int(row["address"], 16)
        body.append("/*")
        body.append(f"Purpose: {row['name']} - arithmetic over "
                    f"{len(touched)} fixed global(s), one")
        body.append("         statement per original instruction.")
        body.append(f"Original Offset: {address:08X}")
        body.append("Return Value: n/a")
        body.append("Status: Complete")
        body.append("*/")
        body.append(f"void __fastcall {symbol}_redirect(void *, void *) {{")
        for statement in statements:
            body.append(f"    {statement}")
        body.append("}")
        body.append("")
        wire.append(f"0x{address:08X} {symbol}_redirect")
        golden = checksum(simulate(statements, addresses, 0x3000))
        cases.append(f"    {{reinterpret_cast<void *>(&{symbol}_redirect), "
                     f"0x{golden:08X}U}},")

    fragment = ["// GENERATED by tools/generate_global_arith.py.",
                "// `golden` is FNV-1a over the whole slot image after this",
                "// thunk runs from the seed the fixture uses. It is computed",
                "// by an independent simulator in the generator, so the test",
                "// compares against a value rather than against itself.",
                "struct GlobalArithCase { void *thunk; uint32_t golden; };",
                "const GlobalArithCase g_global_arith_cases[] = {"]
    fragment += cases
    fragment += ["};", "",
                 "int32_t **const g_global_arith_slots[] = {"]
    for target in addresses:
        fragment.append(f"    &GlobalSlot{target:08X},")
    fragment += ["};"]
    return ("\n".join(lines) + "\n", "\n".join(body),
            "\n".join(fragment) + "\n", "\n".join(wire) + "\n")


if __name__ == "__main__":
    raise SystemExit(main())
