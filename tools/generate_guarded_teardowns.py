#!/usr/bin/env python3
"""Generate complete recoveries for once-guarded teardown thunks.

Thirteen `??__F` functions share one shape: a bit in a shared flag byte says
whether this global has already been torn down, and the body runs the teardown
at most once.

    mov cl, [FLAG] ; mov al, MASK ; test al, cl ; jne skip
    or cl, al ; mov [FLAG], cl ; mov ecx, OBJECT ; jmp TEARDOWN
  skip:
    ret

Note the sense: the teardown runs only when the bit is CLEAR, and setting it
happens BEFORE the call, so a teardown that re-entered would not recurse.
Several of these share a flag byte and differ only in the bit - three
FileWin sprites use bits 1, 2 and 4 of 0x009B9014 - so the mask is per
function and never inferred from the object.

Every teardown target is already source_complete, so nothing here waits on a
recovery. 0x005D7410 (~Buffer) is REUSED from atexit_thunks rather than bound
a second time under a new name; only Sprite::close needs a new one.

The flag bytes and object addresses live in the original image and are
unmapped in a standalone test process, so all of them are bound rather than
inlined - the same precondition that BufferResetValue520, BufferPalette and
the global-arithmetic leaves each established the hard way.

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
OWN_SOURCE = "src/guarded_teardowns.cpp"

# Teardown targets some other header already binds. Reused by ADDRESS, never
# re-bound under a second name - a duplicate binding for one address is a link
# error, and has cost this project a debugging session before.
HANDLED_TARGETS = {
    0x005D7410: ("BufferElementTeardown", "atexit_thunks.h"),
}

# Targets that are source_complete AND reachable as ordinary C++. The seam
# defaults to the RECOVERED body, not to the original address: pointing it at
# the address would make it an original dependency it is not, would need a
# classification row saying so, and would keep the leaf suite out of the
# recovered code. Same call as generate_init_thunks makes for its ctor seams.
RECOVERED_TARGETS = {
    0x005E3820: ("Sprite", "sprite.h", "sprite_close_default",
                 "static_cast<Sprite *>(object)->close();"),
}

PATTERN = [
    r"mov cl, byte ptr \[(0x[0-9a-f]+)\]",
    r"mov al, (\d+)",
    r"test al, cl",
    r"jne 0x[0-9a-f]+",
    r"or cl, al",
    r"mov byte ptr \[(0x[0-9a-f]+)\], cl",
    r"mov ecx, (0x[0-9a-f]+)",
    r"jmp 0x([0-9a-f]+)",
    r"ret",
]


class Unsettled(Exception):
    """The bytes do not pin something this generator refuses to guess."""


def existing_bindings() -> dict[int, tuple[str, str]]:
    """address -> (symbol, header) for globals the tree already binds.

    Written after this generator minted TeardownObject009B8F28 for an address
    init_thunks.cpp already binds as g_RADIOBUTTON_SPRITE_2 - eleven of its
    twenty bindings were duplicates. Two names for one address is not a link
    error, which is precisely why it is dangerous: a fixture that rebinds one
    of them leaves the other pointing at the original image and silently tests
    nothing. Discovered by scanning rather than by a hardcoded table, so the
    next global someone adds is picked up without touching this file.
    """
    found: dict[int, tuple[str, str]] = {}
    declared: dict[str, str] = {}
    for header in sorted((REPO_ROOT / "src").glob("*.h")):
        for symbol in re.findall(r"^extern\s+[\w:]+\s*\*+\s*(\w+)\s*;",
                                 header.read_text(), re.M):
            declared[symbol] = header.name
    pattern = re.compile(
        r"^(?:[\w:]+\s*\*+\s*)?(\w+)\s*=\s*"
        r"(?:\([^)]*\)|reinterpret_cast<[^>]+>\()\s*0x([0-9A-Fa-f]{8})",
        re.M)
    for source in sorted((REPO_ROOT / "src").glob("*.cpp")):
        if source.name == Path(OWN_SOURCE).name:
            continue
        for symbol, address in pattern.findall(source.read_text()):
            if symbol in declared:
                found.setdefault(int(address, 16), (symbol, declared[symbol]))
    return found


def read_bytes(pe: pefile.PE, address: int, length: int) -> bytes:
    base = pe.OPTIONAL_HEADER.ImageBase
    for section in pe.sections:
        low = section.VirtualAddress + base
        high = low + max(section.Misc_VirtualSize, section.SizeOfRawData)
        if low <= address < high:
            return section.get_data()[address - low:address - low + length]
    return b""


def decode(code: bytes, address: int) -> tuple[int, int, int, int]:
    """(flag address, mask, object address, teardown address)."""
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    text = [f"{i.mnemonic} {i.op_str}".rstrip()
            for i in md.disasm(code, address)]
    if len(text) != len(PATTERN):
        raise Unsettled("not a once-guarded teardown")
    matches = [re.fullmatch(pattern, entry)
               for pattern, entry in zip(PATTERN, text)]
    if not all(matches):
        raise Unsettled("not a once-guarded teardown")
    flag = int(matches[0].group(1), 16)
    stored = int(matches[5].group(1), 16)
    if stored != flag:
        # The byte read and the byte written must be the same one, or this is
        # some other shape wearing this one's clothes.
        raise Unsettled("reads one flag byte and writes another")
    return flag, int(matches[1].group(1)), int(matches[6].group(1), 16), \
        int(matches[7].group(1), 16)


SEQUENCE_ONE = [r"mov ecx, (0x[0-9a-f]+)", r"jmp 0x([0-9a-f]+)"]
SEQUENCE_TWO = [r"mov ecx, (0x[0-9a-f]+)", r"call 0x([0-9a-f]+)",
                r"mov ecx, (0x[0-9a-f]+)", r"jmp 0x([0-9a-f]+)"]


def decode_sequence(code: bytes, address: int) -> list[tuple[int, int]]:
    """[(object, teardown)] for the unguarded forwarders.

    One or two steps, the last tail-jumped. The tail jump means the final
    callee's `ret` returns to OUR caller, so the step list is the whole body -
    there is nothing after it to transcribe.
    """
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    text = [f"{i.mnemonic} {i.op_str}".rstrip()
            for i in md.disasm(code, address)]
    for pattern in (SEQUENCE_ONE, SEQUENCE_TWO):
        if len(text) != len(pattern):
            continue
        matches = [re.fullmatch(entry, line)
                   for entry, line in zip(pattern, text)]
        if not all(matches):
            continue
        values = [int(m.group(1), 16) for m in matches]
        return list(zip(values[0::2], values[1::2]))
    raise Unsettled("not an unguarded forwarder")


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
    states = {int(row["address"], 16): row for row in rows}
    ordered = sorted(states)
    following = {address: ordered[index + 1]
                 for index, address in enumerate(ordered[:-1])}

    accepted, skipped = [], []
    for row in rows:
        owned = OWN_SOURCE in row["source_locations"]
        if row["recovery_state"] != "unrecovered" and not owned:
            continue
        size = int(row["size"] or 0)
        if not 5 <= size <= 48:
            continue
        address = int(row["address"], 16)
        code = read_bytes(pe, address, size)
        try:
            flag, mask, obj, target = decode(code, address)
            steps = [(obj, target)]
        except Unsettled:
            flag, mask = None, None
            try:
                steps = decode_sequence(code, address)
            except Unsettled:
                continue
        unresolved = [target for _, target in steps
                      if states.get(target) is None]
        if unresolved:
            skipped.append((row, f"teardown {unresolved[0]:#x} is uncatalogued"))
            continue
        slack = following.get(address, address + size) - address
        if slack < JUMP_PATCH_BYTES:
            skipped.append((row, f"only {slack}B of slack"))
            continue
        accepted.append((row, flag, mask, steps,
                         [states[target]["name"] for _, target in steps]))

    accepted.sort(key=lambda entry: int(entry[0]["address"], 16))
    targets = sorted({target for entry in accepted
                      for _, target in entry[3]})
    guarded = sum(1 for entry in accepted if entry[1] is not None)
    print(f"{guarded} once-guarded teardowns, {len(accepted) - guarded} "
          f"unguarded forwarders, {len(targets)} distinct targets")
    for row, flag, mask, steps, names in accepted:
        gate = f"flag {flag:#x} bit {mask}" if flag is not None else "unguarded"
        chain = " then ".join(
            f"{obj:#x}->{name[:24]}" for (obj, _), name in zip(steps, names))
        print(f"  {row['address']}  {gate:<22} {chain}")
    if skipped:
        print(f"\n{len(skipped)} skipped - never guessed, always named:")
        for row, reason in sorted(skipped, key=lambda e: e[0]["address"]):
            print(f"  {row['address']}  {row['name'][:38]:<38} {reason}")

    if args.dry_run:
        return 0
    header, source, fragment, wires = emit(
        accepted, targets, existing_bindings())
    if args.source_dir:
        args.source_dir.mkdir(parents=True, exist_ok=True)
        (args.source_dir / "guarded_teardowns.h").write_text(header)
        (args.source_dir / "guarded_teardowns.cpp").write_text(source)
    if args.scratch_dir:
        args.scratch_dir.mkdir(parents=True, exist_ok=True)
        (args.scratch_dir / "guarded-teardown-tests.cpp").write_text(fragment)
        (args.scratch_dir / "guarded-teardown-wire.txt").write_text(wires)
    print("\nemitted guarded_teardowns.{h,cpp}, test fragment, wire list")
    return 0


LICENCE = (REPO_ROOT / "src" / "vector_teardown.h").read_text().split(
    "#pragma once")[0]

BANNER = """/*
 * Once-guarded teardown thunks - GENERATED by
 * tools/generate_guarded_teardowns.py. Do not edit by hand; edit the
 * generator and regenerate.
 *
 * %d ??__F bodies that tear a global down AT MOST ONCE, gated on a bit in a
 * shared flag byte. The teardown runs only when the bit is CLEAR, and the bit
 * is set BEFORE the call rather than after. Flag bytes are shared: three
 * FileWin sprites use bits 1, 2 and 4 of one byte, so the mask belongs to the
 * function and is never inferred from the object.
 *
 * Flags and objects are BOUND rather than inlined. Both live in the original
 * image and are unmapped in a standalone test process.
 */
"""


def symbol_for(row) -> str:
    name = row["name"]
    found = re.fullmatch(r"\?\?__F(\w+)@@YAXXZ", name)
    if found:
        return f"teardown_{found.group(1).lower()}"
    return f"teardown_{int(row['address'], 16):08x}"


def emit(accepted, targets, existing):
    banner = BANNER % len(accepted)
    flags = sorted({entry[1] for entry in accepted}
                   - {None})
    objects = sorted({obj for entry in accepted
                      for obj, _ in entry[3]})

    lines = [LICENCE, "#pragma once", '#include "vector_teardown.h"',
             '#include "atexit_thunks.h"', "", banner]
    for flag in flags:
        lines.append(f"extern uint8_t *TeardownFlags{flag:08X};")
    reuse_headers = set()
    for obj in objects:
        if obj in existing:
            reuse_headers.add(existing[obj][1])
            continue
        lines.append(f"extern void *TeardownObject{obj:08X};")
    for target in targets:
        if target in HANDLED_TARGETS or target in existing:
            if target in existing:
                reuse_headers.add(existing[target][1])
            continue
        lines.append(f"extern func_thiscall_teardown "
                     f"*TeardownTarget{target:08X};")
    for target in sorted(set(targets) & set(RECOVERED_TARGETS)):
        lines.insert(3, f'#include "{RECOVERED_TARGETS[target][1]}"')
    for name in sorted(reuse_headers):
        lines.insert(3, f'#include "{name}"')
    lines.append("")
    for row, *_ in accepted:
        lines.append(f"void __cdecl {symbol_for(row)}();")

    body = [LICENCE, '#include "stdafx.h"', '#include "guarded_teardowns.h"',
            "", banner]
    for flag in flags:
        body.append(f"uint8_t *TeardownFlags{flag:08X} = "
                    f"reinterpret_cast<uint8_t *>(0x{flag:08X});")
    for obj in objects:
        if obj in existing:
            continue
        body.append(f"void *TeardownObject{obj:08X} = "
                    f"reinterpret_cast<void *>(0x{obj:08X});")
    for target in targets:
        if target in HANDLED_TARGETS or target in existing:
            continue
        recovered = RECOVERED_TARGETS.get(target)
        if recovered is None:
            body.append(f"func_thiscall_teardown *TeardownTarget{target:08X} ="
                        f"\n    (func_thiscall_teardown *)0x{target:08X};")
            continue
        _, _, default, statement = recovered
        body.append("#if defined(__GNUC__)")
        body.append("#pragma GCC diagnostic push")
        body.append('#pragma GCC diagnostic ignored "-Wattributes"')
        body.append("#endif")
        body.append(f"void __thiscall {default}(void *object) {{")
        body.append(f"    {statement}")
        body.append("}")
        body.append("#if defined(__GNUC__)")
        body.append("#pragma GCC diagnostic pop")
        body.append("#endif")
        body.append(f"func_thiscall_teardown *TeardownTarget{target:08X} ="
                    f" &{default};")
    body.append("")

    wire, cases = [], []
    for row, flag, mask, steps, names in accepted:
        symbol = symbol_for(row)
        address = int(row["address"], 16)
        arguments = []
        for obj, target in steps:
            holder = existing.get(obj)
            arguments.append(holder[0] if holder
                             else f"TeardownObject{obj:08X}")
        seams, raw_seams = [], []
        for _, target in steps:
            reuse = HANDLED_TARGETS.get(target) or existing.get(target)
            if reuse is None:
                seams.append(f"TeardownTarget{target:08X}")
                raw_seams.append(f"TeardownTarget{target:08X}")
            else:
                raw_seams.append(reuse[0])
                # Cast the pointer VALUE, not a fixed address: the seam is
                # read at call time, so rebinding it still takes effect.
                seams.append(f"reinterpret_cast<func_thiscall_teardown *>(\n"
                             f"        {reuse[0]})")
        body.append("/*")
        if flag is not None:
            body.append(f"Purpose: {row['name']} - tear down the global at "
                        f"{steps[0][0]:#x} through")
            body.append(f"         {names[0]}, at most once, gated on bit "
                        f"{mask} of the flag byte")
            body.append(f"         at {flag:#x}.")
        else:
            body.append(f"Purpose: {row['name']} - run "
                        f"{len(steps)} teardown(s) on fixed globals,")
            body.append("         unguarded. The last is a tail jump in the "
                        "original, so its")
            body.append("         return goes straight to this function's "
                        "caller.")
        body.append(f"Original Offset: {address:08X}")
        body.append("Return Value: n/a")
        body.append("Status: Complete")
        body.append("*/")
        body.append(f"void __cdecl {symbol}() {{")
        if flag is not None:
            body.append(f"    const uint8_t flags = *TeardownFlags{flag:08X};")
            body.append(f"    if (({mask} & flags) != 0) {{")
            body.append("        return;")
            body.append("    }")
            body.append("    // Set BEFORE the teardown, not after: the "
                        "original stores at the")
            body.append("    // instruction preceding its tail jump.")
            body.append(f"    *TeardownFlags{flag:08X} = "
                        f"static_cast<uint8_t>(flags | {mask});")
        for seam, argument in zip(seams, arguments):
            body.append(f"    {seam}({argument});")
        body.append("}")
        body.append("")
        wire.append(f"0x{address:08X} {symbol}")
        cases.append(
            f"    {{&{symbol}, "
            + (f"&TeardownFlags{flag:08X}, {mask}, " if flag is not None
               else "nullptr, 0, ")
            + f"{len(steps)}, "
            + "{"
            + ", ".join(f"reinterpret_cast<void **>(&{a})" for a in arguments)
            + (", nullptr" if len(steps) < 2 else "")
            + "}, {"
            # The fixture needs a rebindable HANDLE, so it takes the address of
            # the seam variable and casts the pointer-to-pointer - `&` cannot
            # be applied to the cast expression the call site uses.
            + ", ".join(
                f"reinterpret_cast<func_thiscall_teardown **>(&{raw})"
                for raw in raw_seams)
            + (", nullptr" if len(steps) < 2 else "")
            + "}},")

    fragment = ["// GENERATED by tools/generate_guarded_teardowns.py.",
                "struct GuardedTeardownCase {",
                "    void (__cdecl *thunk)();",
                "    uint8_t **flags;   // null when the body is unguarded",
                "    int mask;",
                "    int steps;",
                "    void **object[2];",
                "    func_thiscall_teardown **target[2];",
                "};",
                "const GuardedTeardownCase g_guarded_teardown_cases[] = {"]
    fragment += cases + ["};"]
    return ("\n".join(lines) + "\n", "\n".join(body),
            "\n".join(fragment) + "\n", "\n".join(wire) + "\n")


if __name__ == "__main__":
    raise SystemExit(main())
