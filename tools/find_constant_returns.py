#!/usr/bin/env python3
"""List unrecovered functions whose entire body is a constant return.

The shape is the easiest thing in this image to recover and the easiest to get
subtly wrong. A body of `mov eax, N; ret K` says what to return, but not how
many bytes the caller expects cleaned off the stack - and a replacement that
pops the wrong number corrupts its caller rather than itself, failing far from
the cause.

So a candidate is only reported when its mangled name's parameter list agrees
with the callee-pop count in its body. Run against this image that check
rejects exactly one otherwise-perfect candidate, Win::on_redraw, which
declares no parameters while cleaning eight bytes; it was rejected by hand for
the same reason before this existed, which is the evidence the check works.

Bodies this short also have to be patchable. A jump redirect writes five bytes
over a function that may itself be one byte long, so the slack before the next
function is reported and anything too tight to patch is dropped.
"""

from __future__ import annotations

import argparse
import csv
import re
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import pefile  # noqa: E402
from capstone import CS_ARCH_X86, CS_MODE_32, Cs  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"
FUNCTIONS_CSV = REPO_ROOT / "docs" / "recovery" / "functions.csv"

JUMP_PATCH_BYTES = 5

# Argument sizes for the mangled type codes simple enough to be unambiguous.
SIZES = {"D": 4, "E": 4, "C": 4, "F": 4, "G": 4, "H": 4,
         "I": 4, "J": 4, "K": 4, "M": 4, "N": 8, "O": 8}


def read_bytes(pe: pefile.PE, address: int, length: int) -> bytes:
    base = pe.OPTIONAL_HEADER.ImageBase
    for section in pe.sections:
        begin = base + section.VirtualAddress
        end = begin + max(section.Misc_VirtualSize, section.SizeOfRawData)
        if begin <= address < end:
            offset = section.PointerToRawData + (address - begin)
            return pe.__data__[offset:offset + length]
    return b""


def constant_body(code: bytes) -> tuple[int | None, int] | None:
    """(returned value, bytes popped) when the body is nothing else."""
    if len(code) == 8 and code[0] == 0xB8 and code[5] == 0xC2:
        return struct.unpack("<I", code[1:5])[0], struct.unpack("<H", code[6:8])[0]
    if len(code) == 6 and code[0] == 0xB8 and code[5] == 0xC3:
        return struct.unpack("<I", code[1:5])[0], 0
    if len(code) == 5 and code[:2] == b"\x33\xc0" and code[2] == 0xC2:
        return 0, struct.unpack("<H", code[3:5])[0]
    if len(code) == 3 and code[:2] == b"\x33\xc0" and code[2] == 0xC3:
        return 0, 0
    if len(code) == 3 and code[0] == 0xC2:
        return None, struct.unpack("<H", code[1:3])[0]
    if len(code) == 1 and code[0] == 0xC3:
        return None, 0
    return None


def token(text: str, index: int) -> tuple[str | None, int]:
    if index >= len(text):
        return None, index
    char = text[index]
    if char == "_":
        return text[index:index + 2], index + 2
    if char in "PQRA":                      # pointer or reference
        scan = index + 1
        while scan < len(text):
            if text[scan] in SIZES or text[scan] == "X":
                return text[index:scan + 1], scan + 1
            if text[scan] in "VU":          # to a class or struct
                end = text.find("@@", scan)
                if end < 0:
                    return None, index
                return text[index:end + 2], end + 2
            scan += 1
        return None, index
    if char in "VU":
        end = text.find("@@", index)
        if end < 0:
            return None, index
        return text[index:end + 2], end + 2
    return char, index + 1


def declared_arity(mangled: str) -> tuple[str, int] | None:
    """(convention, bytes of parameters) for names simple enough to read."""
    match = re.match(r"^\?[\w@]+@@(.)(.*)$", mangled)
    if not match:
        return None
    first, tail = match.groups()
    # A free function is `Y` followed straight by the calling convention -
    # ?energy_limit@@YAHH@Z is Y, A (cdecl), H (int return), H (one int), @Z.
    # A member carries an access byte and a near/far byte first, so its
    # convention is two positions further along: ?requested_height@AlphaMenu@@
    # QAEHXZ is Q (public), A (near), E (thiscall), H, X, Z. Reading the
    # member layout off a free name lands on the RETURN TYPE and rejects it,
    # which is why every `@@YA...` function was invisible to this scan.
    if first == "Y":
        convention, rest = tail[:1], tail[1:]
    else:
        convention, rest = tail[1:2], tail[2:]
    if convention not in ("E", "A", "G"):
        return None
    convention = {"E": "thiscall", "A": "cdecl", "G": "stdcall"}[convention]

    item, index = token(rest, 0)            # the return type
    if item is None:
        return None
    total = 0
    # A parameter list ends with "@Z" when there are parameters and "XZ" when
    # there are none. Stopping only at "Z" leaves the terminating "@" to be
    # read as a type, which fails - so before this stopped at "@" the check
    # silently declined every function that takes an argument, and only ever
    # validated the zero-parameter ones.
    while index < len(rest) and rest[index] not in "@Z":
        item, following = token(rest, index)
        if item is None:
            return None
        if item == "X":                     # void: no parameters at all
            break
        if item[0] in "PQRA" or item.startswith("_"):
            total += 4
        elif item in SIZES:
            total += SIZES[item]
        else:
            return None
        index = following
    return convention, total


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--class-name", help="only this class")
    parser.add_argument("--show-rejected", action="store_true",
                        help="also list candidates the arity check refused")
    args = parser.parse_args()

    pe = pefile.PE(str(args.exe), fast_load=True)
    with FUNCTIONS_CSV.open() as handle:
        rows = list(csv.DictReader(handle))
    ordered = sorted(int(row["address"], 16) for row in rows)
    following = {address: ordered[index + 1]
                 for index, address in enumerate(ordered[:-1])}

    accepted, rejected = [], []
    for row in rows:
        if row["recovery_state"] != "unrecovered":
            continue
        address = int(row["address"], 16)
        size = int(row["size"] or 0)
        if not 1 <= size <= 10:
            continue
        body = constant_body(read_bytes(pe, address, size))
        if body is None:
            continue
        value, popped = body
        arity = declared_arity(row["name"])
        if arity is None:
            continue
        convention, declared = arity
        slack = following.get(address, address + size) - address
        entry = (row, convention, declared, popped, value, slack)
        expected = 0 if convention == "cdecl" else declared
        if expected != popped:
            rejected.append(entry)
        elif slack >= JUMP_PATCH_BYTES:
            accepted.append(entry)

    name_of = lambda row: (re.match(r"\?(\w+)@(\w+)@@", row["name"]) or [None, None, "(free)"])[2]
    if args.class_name:
        accepted = [e for e in accepted if name_of(e[0]) == args.class_name]
    accepted.sort(key=lambda e: (name_of(e[0]), e[0]["address"]))

    print(f"{len(accepted)} verified constant-return candidates")
    for row, convention, _, popped, value, slack in accepted:
        returns = "void" if value is None else f"0x{value:X}"
        print(f"  {row['address']}  {row['name'][:52]:<52} {convention:<9} "
              f"returns {returns:<8} pops {popped:<3} slack {slack}")

    if args.show_rejected and rejected:
        print(f"\n{len(rejected)} rejected - the name's arity and the body's "
              "stack cleanup disagree:")
        for row, convention, declared, popped, _, _ in rejected:
            print(f"  {row['address']}  {row['name'][:52]:<52} {convention} "
                  f"name says {declared}B, body pops {popped}B")
    return 0


if __name__ == "__main__":
    sys.exit(main())
