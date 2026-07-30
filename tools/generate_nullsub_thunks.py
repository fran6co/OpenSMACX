#!/usr/bin/env python3
"""Generate complete recoveries for do-nothing leaves, in bulk.

Roughly sixty functions in the image are one of exactly four bodies:

    ret            ret N            xor eax,eax; ret      xor eax,eax; ret N

They read nothing, write nothing, and branch nowhere. Almost all are IDA's
`nullsub_*` and `sub_*` - they have no mangled name, so nothing declares their
calling convention or their parameter list, which is precisely why
find_constant_returns and bulk_recover_stubs cannot touch them: both need a
name to cross-check the callee-pop count against.

THAT CROSS-CHECK IS NOT NEEDED HERE, and the reason is worth stating because
it is the whole justification for this generator. The only two facts a
redirect must reproduce are how many bytes it pops and what it leaves in EAX,
and the terminating `ret N` states the first outright. The convention is
undecidable and also irrelevant: a `__fastcall` redirect taking (self, unused,
N stack ints) lays its stack arguments out at [esp+4]... and pops N, which is
byte-identical to __thiscall with N args AND to __stdcall with N args. The
only difference between those readings is what ECX and EDX mean on entry, and
a body that reads neither cannot tell them apart. So the emitted body is
correct under every convention the bytes admit, rather than correct under a
guessed one.

What it still refuses, and prints:

  * Anything with less than five bytes of slack before the next function. A
    jump redirect writes five, and two of these leaves sit one and three bytes
    from their neighbour.
  * Anything carrying a mangled name. Those belong to their class, and
    bulk_recover_stubs owns that path - including ?on_redraw@Win@@QAEHXZ,
    whose name says no parameters while its body pops eight, and which is the
    canonical example that check exists to catch.

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

from generator_support import LICENSE, read_bytes

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"
FUNCTIONS_CSV = REPO_ROOT / "docs" / "recovery" / "functions.csv"
JUMP_PATCH_BYTES = 5
OWN_SOURCE = "src/nullsub_thunks.cpp"


def decode(code: bytes, address: int) -> tuple[int, bool] | None:
    """(bytes popped, whether EAX is zeroed), or None if not a do-nothing."""
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    text = [f"{i.mnemonic} {i.op_str}".rstrip()
            for i in md.disasm(code, address)]
    if not text:
        return None
    zeroes = text[0] == "xor eax, eax"
    if zeroes:
        text = text[1:]
    if len(text) != 1:
        return None
    found = re.fullmatch(r"ret(?: (\d+))?", text[0])
    if not found:
        return None
    return int(found.group(1) or 0), zeroes


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

    accepted, skipped = [], []
    for row in rows:
        owned = OWN_SOURCE in row["source_locations"]
        if row["recovery_state"] != "unrecovered" and not owned:
            continue
        size = int(row["size"] or 0)
        if not 1 <= size <= 8:
            continue
        address = int(row["address"], 16)
        body = decode(read_bytes(pe, address, size), address)
        if body is None:
            continue
        popped, zeroes = body
        if row["name"].startswith("?"):
            skipped.append((row, "mangled name - belongs to its class"))
            continue
        slack = following.get(address, address + size) - address
        if slack < JUMP_PATCH_BYTES:
            skipped.append((row, f"only {slack}B of slack"))
            continue
        accepted.append((row, popped, zeroes))

    accepted.sort(key=lambda entry: int(entry[0]["address"], 16))
    print(f"{len(accepted)} do-nothing leaves")
    for row, popped, zeroes in accepted:
        answer = "0" if zeroes else "-"
        print(f"  {row['address']}  pops {popped:<3} eax {answer:<3} "
              f"{row['name']}")
    if skipped:
        print(f"\n{len(skipped)} skipped - never guessed, always named:")
        for row, reason in sorted(skipped, key=lambda e: e[0]["address"]):
            print(f"  {row['address']}  {row['name'][:44]:<44} {reason}")

    if args.dry_run:
        return 0
    header, source, fragment, wires = emit(accepted)
    if args.source_dir:
        args.source_dir.mkdir(parents=True, exist_ok=True)
        (args.source_dir / "nullsub_thunks.h").write_text(header)
        (args.source_dir / "nullsub_thunks.cpp").write_text(source)
    if args.scratch_dir:
        args.scratch_dir.mkdir(parents=True, exist_ok=True)
        (args.scratch_dir / "nullsub-thunk-tests.cpp").write_text(fragment)
        (args.scratch_dir / "nullsub-wire.txt").write_text(wires)
    print("\nemitted nullsub_thunks.{h,cpp}, test fragment, wire list")
    return 0


BANNER = """/*
 * Do-nothing leaves - GENERATED by tools/generate_nullsub_thunks.py. Do not
 * edit by hand; edit the generator and regenerate.
 *
 * %d functions whose entire body is a return, optionally zeroing EAX first,
 * optionally popping arguments. They read nothing and write nothing.
 *
 * These are free redirects over opaque storage. None of these functions
 * carries a mangled name, so nothing in the image declares a calling
 * convention for them - and none is needed: the only observable facts are the
 * pop count, which the `ret N` states outright, and EAX. A __fastcall
 * redirect with N trailing stack parameters is byte-identical to __thiscall
 * and to __stdcall for a body that reads neither ECX nor EDX.
 */
"""


def symbol_for(row) -> str:
    name = row["name"]
    if re.fullmatch(r"(nullsub|sub)_[0-9a-fA-F]+", name):
        return f"nullsub_{int(row['address'], 16):08x}"
    return re.sub(r"\W", "_", name).lower()


def emit(accepted):
    lines = [LICENSE, "#pragma once", "", BANNER % len(accepted)]
    body = [LICENSE, '#include "stdafx.h"', '#include "nullsub_thunks.h"',
            "", BANNER % len(accepted)]
    cases = ["// GENERATED by tools/generate_nullsub_thunks.py.",
             "struct NullsubCase {",
             "    void *thunk;",
             "    int declared;   // stack arguments the thunk must POP",
             "    int answer;     // -1 when it returns void",
             "};",
             "const NullsubCase g_nullsub_cases[] = {"]
    wire = []
    for row, popped, zeroes in accepted:
        symbol = symbol_for(row)
        declared = popped // 4
        params = "".join(f", int" for _ in range(declared))
        named = "".join(f", int" for _ in range(declared))
        returns = "int" if zeroes else "void"
        address = int(row["address"], 16)
        lines.append(f"{returns} __fastcall {symbol}_redirect("
                     f"void *self, void *{params});")
        body.append("/*")
        body.append(f"Purpose: {row['name']} - a do-nothing leaf. The whole "
                    "body is a return")
        body.append(f"         popping {popped} bytes"
                    + (" after zeroing EAX." if zeroes else "."))
        body.append(f"Original Offset: {address:08X}")
        body.append("Return Value: " + ("0" if zeroes else "n/a"))
        body.append("Status: Complete")
        body.append("*/")
        body.append(f"{returns} __fastcall {symbol}_redirect("
                    f"void *, void *{named}) {{")
        if zeroes:
            body.append("    return 0;")
        body.append("}")
        body.append("")
        cases.append(f"    {{reinterpret_cast<void *>(&{symbol}_redirect), "
                     f"{declared}, {0 if zeroes else -1}}},")
        wire.append(f"0x{address:08X} {symbol}_redirect")
    cases.append("};")
    return ("\n".join(lines) + "\n", "\n".join(body),
            "\n".join(cases) + "\n", "\n".join(wire) + "\n")


if __name__ == "__main__":
    raise SystemExit(main())
