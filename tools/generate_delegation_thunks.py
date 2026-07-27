#!/usr/bin/env python3
"""Generate complete recoveries for guarded delegation thunks, in bulk.

One shape recurs 44 times across Midi, VoiceRx, VoiceTx and Wave_In_Device:

    member = this->at(OFFSET);
    if (!member) return CONSTANT;
    return member->vtable[SLOT](args...);

Every one of those is mechanical up to four numbers - the member offset, the
vtable slot, the forwarded argument count, and the constant the absent-member
path answers - so this emits the bodies, the redirects, the canary tests and
the wire list for the whole family at once, the same way the adjustor, atexit,
init and deleting-thunk generators do for theirs.

What it does NOT do is decide anything a wrong guess would make plausible.
Three checks reject rather than assume, and each rejection is printed:

  * The mangled name's parameter bytes must equal the terminating `ret N`.
    This is the check that rejects Win::on_redraw image-wide, and it is
    borrowed from find_constant_returns rather than reimplemented.
  * The absent-member path must load a constant into EAX. Several of these
    functions instead branch straight to the epilogue - ?switch_state@Midi@@
    and ?set_base_path@Midi@@ among them - so the original returns whatever
    EAX happened to hold on entry. No C++ body can reproduce a caller's
    register residue, and answering 0 there would be a guess a caller could
    branch on, so those are skipped and named.
  * A jump redirect writes five bytes, so a body with less slack than that
    before the next function is dropped.

The emitted bodies are FREE `__fastcall` redirects over opaque storage, not
methods. Nothing here adds or edits a class header: the classes involved are
largely unmodelled, the redirect is what gets installed at the original
address either way, and AGENTS.md records what happened the last time a
scripted batch wrote a class header.

Like the other bulk generators this compresses authoring, not verification.
The emitted bodies still go through the leaf suite, the mutation sweep and the
batch gate exactly as hand-written recoveries do.

Output is derived from proprietary bytes: local analysis only, never
committed.
"""
from __future__ import annotations

import argparse
import csv
import re
import sys
from pathlib import Path

import capstone
import pefile

REPO_ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(Path(__file__).resolve().parent))
from find_constant_returns import declared_arity  # noqa: E402

DEFAULT_EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"
FUNCTIONS_CSV = REPO_ROOT / "docs" / "recovery" / "functions.csv"
JUMP_PATCH_BYTES = 5
MAX_BODY_BYTES = 48


def read_bytes(pe: pefile.PE, address: int, length: int) -> bytes:
    base = pe.OPTIONAL_HEADER.ImageBase
    for section in pe.sections:
        low = section.VirtualAddress + base
        high = low + max(section.Misc_VirtualSize, section.SizeOfRawData)
        if low <= address < high:
            return section.get_data()[address - low:address - low + length]
    return b""


def _number(text: str) -> int:
    return int(text, 16) if text.startswith("0x") else int(text)


def _displacement(operand: str) -> int | None:
    found = re.search(r"\[e\w\w \+ (0x[0-9a-f]+|\d+)\]", operand)
    return _number(found.group(1)) if found else None


class Unsettled(Exception):
    """The bytes do not pin something this generator refuses to guess."""


def decode(code: bytes, address: int) -> tuple[int, int, int, int, int]:
    """(member offset, vtable slot, forwarded args, ret N, null constant).

    Raises Unsettled for anything outside the family or short of pinned.
    """
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    instructions = list(md.disasm(code, address))
    if not instructions:
        raise Unsettled("undecodable")
    text = [f"{i.mnemonic} {i.op_str}".rstrip() for i in instructions]

    popped = None
    for instruction in reversed(instructions):
        if instruction.mnemonic == "ret":
            operand = instruction.op_str.strip()
            popped = _number(operand) if operand else 0
            break
    if popped is None:
        raise Unsettled("no ret")

    # The frame-pointer form: the delegate is reached with `call`, and the
    # absent-member path is the code after it.
    if (len(text) >= 8 and text[0] == "push ebp" and text[1] == "mov ebp, esp"
            and text[2].startswith("mov ecx, dword ptr [ecx +")
            and text[3] == "test ecx, ecx" and text[4].startswith("je ")):
        member = _displacement(text[2])
        call = next((t for t in text if t.startswith("call dword ptr [eax +")),
                    None)
        if call is None:
            raise Unsettled("no vtable call")
        slot = _displacement(call)
        forwarded = sum(1 for t in text
                        if re.match(r"mov e\w\w, dword ptr \[ebp \+", t))
        # The `je` target decides the absent-member answer. When it lands on
        # the epilogue the original returns the caller's EAX; see the module
        # docstring for why that is a refusal rather than a zero.
        target = _number(text[4].split()[1])
        tail = [t for i, t in enumerate(text)
                if instructions[i].address >= target]
        constant = _constant_from(tail)
        if constant is None:
            raise Unsettled("absent-member path returns caller EAX residue")
        return member, slot, forwarded, popped, constant

    # The tail-call form: no frame, `jmp` through the slot, and the
    # absent-member path is whatever follows the jump.
    if (len(text) >= 6 and text[0].startswith("mov ecx, dword ptr [ecx +")
            and text[1] == "test ecx, ecx" and text[2].startswith("je ")
            and text[3] == "mov eax, dword ptr [ecx]"
            and text[4].startswith("jmp dword ptr [eax +")):
        constant = _constant_from(text[5:])
        if constant is None:
            raise Unsettled("absent-member path returns caller EAX residue")
        return (_displacement(text[0]), _displacement(text[4]), 0, popped,
                constant)

    raise Unsettled("not a guarded delegation")


def _constant_from(tail: list[str]) -> int | None:
    """The value the absent-member path loads into EAX, if it loads one."""
    for entry in tail:
        if entry == "xor eax, eax":
            return 0
        found = re.match(r"mov eax, (0x[0-9a-f]+|\d+)$", entry)
        if found:
            return _number(found.group(1))
        if entry.startswith(("pop ", "ret")):
            continue
        # Anything else in the tail means this is a shape we have not read.
        return None
    return None


def symbol_for(mangled: str) -> tuple[str, str]:
    """(class, method) in snake_case, for naming the emitted redirect."""
    found = re.match(r"^\?(\w+)@(\w+)@@", mangled)
    if not found:
        raise Unsettled("unreadable name")
    method, class_name = found.group(1), found.group(2)
    snake = re.sub(r"(?<!^)(?=[A-Z])", "_", class_name).lower()
    return snake.replace("__", "_"), method.lower()


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--scratch-dir", type=Path, required=False)
    parser.add_argument("--source-dir", type=Path,
                        help="where the committed source pair is written")
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()

    pe = pefile.PE(str(args.exe), fast_load=True)
    with FUNCTIONS_CSV.open() as handle:
        rows = list(csv.DictReader(handle))
    ordered = sorted(int(row["address"], 16) for row in rows)
    following = {address: ordered[index + 1]
                 for index, address in enumerate(ordered[:-1])}

    accepted, skipped = [], []
    for row in rows:
        if row["recovery_state"] != "unrecovered":
            continue
        if not row["name"].startswith("?"):
            continue
        size = int(row["size"] or 0)
        if not 2 <= size <= MAX_BODY_BYTES:
            continue
        address = int(row["address"], 16)
        try:
            member, slot, forwarded, popped, constant = decode(
                read_bytes(pe, address, size), address)
        except Unsettled as reason:
            if "not a guarded delegation" not in str(reason):
                skipped.append((row, str(reason)))
            continue
        arity = declared_arity(row["name"])
        if arity is None:
            skipped.append((row, "name too complex to read"))
            continue
        convention, declared = arity
        if convention != "thiscall" or declared != popped:
            skipped.append(
                (row, f"name says {declared}B, body pops {popped}B"))
            continue
        if declared != forwarded * 4:
            skipped.append(
                (row, f"name says {declared}B, body forwards {forwarded}"))
            continue
        slack = following.get(address, address + size) - address
        if slack < JUMP_PATCH_BYTES:
            skipped.append((row, f"only {slack}B of slack"))
            continue
        try:
            class_name, method = symbol_for(row["name"])
        except Unsettled as reason:
            skipped.append((row, str(reason)))
            continue
        accepted.append((row, class_name, method, member, slot, forwarded,
                         popped, constant))

    accepted.sort(key=lambda entry: int(entry[0]["address"], 16))
    # Midi::map_patch and Midi::set_active_tracks are each C++ OVERLOADS: two
    # addresses, one name, different slots and arities. Disambiguating on the
    # argument count rather than the address keeps the emitted symbol readable
    # and still unique, and a third overload with the same arity would be a
    # duplicate symbol at link time rather than a silent overwrite.
    seen = {}
    for entry in accepted:
        seen[(entry[1], entry[2])] = seen.get((entry[1], entry[2]), 0) + 1
    symbols = []
    for row, class_name, method, member, slot, forwarded, *rest in accepted:
        base = f"{class_name}_{method}"
        if seen[(class_name, method)] > 1:
            base = f"{base}_{forwarded}"
        symbols.append(base)
    if len(set(symbols)) != len(symbols):
        raise SystemExit(f"symbol collision after disambiguation: "
                         f"{sorted(s for s in symbols if symbols.count(s) > 1)}")

    print(f"{len(accepted)} guarded delegation thunks")
    for symbol, (row, _, _, member, slot, forwarded, popped, constant) \
            in zip(symbols, accepted):
        print(f"  {row['address']}  {symbol:<34} "
              f"member +0x{member:<4X} slot 0x{slot:<4X} args {forwarded} "
              f"absent {constant:#x}")
    if skipped:
        print(f"\n{len(skipped)} skipped - never guessed, always named:")
        for row, reason in sorted(skipped,
                                  key=lambda e: e[0]["address"]):
            print(f"  {row['address']}  {row['name'][:52]:<52} {reason}")

    if args.dry_run:
        return 0

    header, source, fragment, wires = emit(symbols, accepted)
    if args.source_dir:
        args.source_dir.mkdir(parents=True, exist_ok=True)
        (args.source_dir / "delegation_thunks.h").write_text(header)
        (args.source_dir / "delegation_thunks.cpp").write_text(source)
    if args.scratch_dir:
        args.scratch_dir.mkdir(parents=True, exist_ok=True)
        (args.scratch_dir / "delegation-thunk-tests.cpp").write_text(fragment)
        (args.scratch_dir / "delegation-wire.txt").write_text(wires)
    print("\nemitted delegation_thunks.{h,cpp}, test fragment, wire list")
    return 0


LICENCE = """/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenSMACX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenSMACX. If not, see <http://www.gnu.org/licenses/>.
 */
"""

BANNER = """/*
 * Guarded delegation thunks - GENERATED by
 * tools/generate_delegation_thunks.py. Do not edit by hand; edit the
 * generator and regenerate.
 *
 * One shape, %d times over: read a member object pointer, answer a constant
 * when it is absent, otherwise forward the arguments through a slot of that
 * member's OWN vtable. The four numbers that differ per function - the member
 * offset, the slot, the forwarded argument count and the absent-member
 * constant - are read from the bytes of each function and are kept as inline
 * literals here so a mutation sweep has something to perturb.
 *
 * These are free redirects over opaque storage rather than methods. The
 * classes involved are largely unmodelled, the redirect is what gets
 * installed at the original address either way, and nothing here touches a
 * class header.
 */
"""


def emit(symbols, accepted):
    """(header text, source text, test fragment, wire list)."""
    arities = sorted({entry[5] for entry in accepted})
    banner = BANNER % len(accepted)

    lines = [LICENCE, "#pragma once", "", banner]
    lines.append("#if defined(__GNUC__)")
    lines.append("#pragma GCC diagnostic push")
    lines.append('#pragma GCC diagnostic ignored "-Wattributes"')
    lines.append("#endif")
    lines.append("// The delegate is entered on the MEMBER, not on the thunk's")
    lines.append("// own object, so every one of these is __thiscall with the")
    lines.append("// member as its receiver.")
    for count in arities:
        params = ", ".join(["void *"] + ["int"] * count)
        lines.append(f"typedef int(__thiscall func_delegation_{count})"
                     f"({params});")
    lines.append("#if defined(__GNUC__)")
    lines.append("#pragma GCC diagnostic pop")
    lines.append("#endif")
    lines.append("")
    for symbol, entry in zip(symbols, accepted):
        forwarded = entry[5]
        params = "".join(f", int a{index + 1}" for index in range(forwarded))
        lines.append(f"int __fastcall {symbol}_redirect("
                     f"void *self, void *{params});")
    header = "\n".join(lines) + "\n"

    body = [LICENCE, '#include "stdafx.h"', '#include "delegation_thunks.h"',
            "", banner]
    wire = []
    for symbol, (row, _, _, member, slot, forwarded, popped, constant) in zip(
            symbols, accepted):
        params = "".join(f", int a{index + 1}" for index in range(forwarded))
        forwarded_args = "".join(f", a{index + 1}"
                                 for index in range(forwarded))
        address = int(row["address"], 16)
        body.append("/*")
        body.append(f"Purpose: {row['name']} - forward {forwarded} argument(s)"
                    f" to slot {slot:#x} of the")
        body.append(f"         object at +{member:#x}, answering {constant:#x}"
                    " when that object is absent.")
        body.append(f"Original Offset: {address:08X}")
        body.append("Return Value: the delegate's, or "
                    f"{constant:#x} when the member is null")
        body.append("Status: Complete")
        body.append("*/")
        body.append(f"int __fastcall {symbol}_redirect("
                    f"void *self, void *{params}) {{")
        body.append("    void *const member = *reinterpret_cast<void **>(")
        body.append(f"        static_cast<uint8_t *>(self) + {member:#x});")
        body.append("    if (member == nullptr) {")
        body.append(f"        return {constant:#x};")
        body.append("    }")
        body.append("    void **const vtable = "
                    "*reinterpret_cast<void ***>(member);")
        body.append(f"    return reinterpret_cast<func_delegation_{forwarded} "
                    f"*>(")
        body.append(f"        vtable[{slot:#x} / sizeof(void *)])"
                    f"(member{forwarded_args});")
        body.append("}")
        body.append("")
        wire.append(f"0x{address:08X} {symbol}_redirect")
    source = "\n".join(body)

    cases = ["// GENERATED by tools/generate_delegation_thunks.py.",
             "struct DelegationCase {",
             "    void *thunk;",
             "    size_t member;",
             "    size_t slot;",
             "    int forwarded;",
             "    int absent;",
             "};",
             "const DelegationCase g_delegation_cases[] = {"]
    for symbol, (row, _, _, member, slot, forwarded, popped, constant) in zip(
            symbols, accepted):
        cases.append(f"    {{reinterpret_cast<void *>(&{symbol}_redirect), "
                     f"{member:#x}, {slot:#x}, {forwarded}, {constant:#x}}},")
    cases.append("};")
    fragment = "\n".join(cases) + "\n"
    return header, source, fragment, "\n".join(wire) + "\n"


if __name__ == "__main__":
    raise SystemExit(main())
