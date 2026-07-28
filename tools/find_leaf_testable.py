#!/usr/bin/env python3
"""List unrecovered functions that can be recovered AND verified as leaves.

The queue this produces is the answer to a question that cost a session to ask
properly: which functions are cheap to RECOVER is not the same as which are
cheap to VERIFY, and only the second one bounds the work.

`BattleWin::stop_timer` is the case that established it. Eight bytes,
`add ecx, 8` / `jmp Time::stop`, its offset already pinned by the recovered
destructor beside it, and its callee already `source_complete`. It still could
not be landed: `Time::stop` lives in `src/time.cpp` with `init`/`start`/`pulse`,
which reach fixed-address bindings into the original executable
(`HandleMain` at 0x009B7B28, `MsgStatus` at 0x009B7B9C), and that translation
unit is not linked into `recovery-leaf-tests`. Per the working rules a function
reaching absolute globals has to be exercised at its original address inside
the verified hybrid process, so an eight-byte body turned out to cost a staged
hybrid run.

So a candidate here must satisfy THREE conditions, and the third is the one
that is easy to forget:

  1. its own body touches no absolute global inside the image, and makes no
     call this tool cannot account for;
  2. every function it calls is already `source_complete`;
  3. every function it calls is COMPILED INTO `recovery-leaf-tests`.

(3) is read out of THE BUILD - the target's own object directory - rather than
out of CMakeLists.txt. Parsing the source list from CMake was the first
attempt and it silently over-matched, collecting 122 sources where the target
has 100; the one false entry was `src/time.cpp`, which made this tool accept
BattleWin::stop_timer, the very function whose failure to link is why the tool
exists. A configured build is therefore a precondition.

The output is a work queue and a COUNT. The count is the point: it is what
makes "exhaust this closure" a goal that can be finished rather than a
direction to walk in, and it grows as recoveries land - each one can make its
callers eligible - so a falling count is real progress and a rising one is
information rather than a bug.

Usage:
    find_leaf_testable.py [--exe ...] [--functions ...] [--show-rejected]
"""

from __future__ import annotations

import argparse
import csv
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import pefile  # noqa: E402
from capstone import CS_ARCH_X86, CS_MODE_32, Cs  # noqa: E402
from capstone.x86 import X86_OP_IMM, X86_OP_MEM  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"
FUNCTIONS_CSV = REPO_ROOT / "docs" / "recovery" / "functions.csv"

# A jump redirect writes five bytes over the target, so a body with less slack
# than that cannot be patched at all - the same rule find_constant_returns.py
# applies, and for the same reason.
REDIRECT_BYTES = 5


def leaf_test_sources(build_dir: Path) -> set[str]:
    """The src/*.cpp files actually compiled into recovery-leaf-tests.

    Taken from the BUILD, not from CMakeLists.txt, and that is the whole point.
    Parsing the target's source list out of CMake looked right and was wrong:
    the regex ran past the end of add_executable() and collected 122 sources
    where the target has 100, which let `src/time.cpp` in. That single false
    entry made the scanner accept BattleWin::stop_timer - the exact function
    whose failure to link is the reason this tool exists.

    The object directory cannot drift from the target, because ninja wrote it.
    A configured build is therefore a precondition, and saying so is better
    than guessing from source text that only resembles the answer.
    """
    for preset in ("mingw-i686-release", "mingw-i686-debug"):
        objects = build_dir / preset / "CMakeFiles" / "recovery-leaf-tests.dir" / "src"
        if objects.is_dir():
            found = {path.name[:-len(".cpp.obj")]
                     for path in objects.glob("*.cpp.obj")}
            if found:
                return found
    raise SystemExit(
        "no recovery-leaf-tests object directory under build/; configure and "
        "build a preset first - this tool reads the target's real source list "
        "from the build rather than parsing CMakeLists.txt, because a parse "
        "that over-matches silently accepts functions that cannot link")


def load_rows(functions_csv: Path) -> list[dict]:
    with functions_csv.open() as handle:
        return list(csv.DictReader(handle))


def body_bytes(pe: pefile.PE, address: int, length: int) -> bytes:
    base = pe.OPTIONAL_HEADER.ImageBase
    for section in pe.sections:
        begin = base + section.VirtualAddress
        end = begin + max(section.Misc_VirtualSize, section.SizeOfRawData)
        if begin <= address < end:
            offset = section.PointerToRawData + (address - begin)
            return pe.__data__[offset:offset + length]
    return b""


def image_span(pe: pefile.PE) -> tuple[int, int]:
    base = pe.OPTIONAL_HEADER.ImageBase
    end = base
    for section in pe.sections:
        end = max(end, base + section.VirtualAddress
                  + max(section.Misc_VirtualSize, section.SizeOfRawData))
    return base, end


def classify(instructions, address: int, size: int, span: tuple[int, int]):
    """(callees, reasons, bindings) for one body.

    `bindings` is separate from `reasons` on purpose. An absolute IMMEDIATE -
    `cmp ecx, 0x9156b0`, comparing `this` against a known global object - does
    not disqualify a function from being leaf-tested, because nothing is
    dereferenced and nothing can fault. But recovering it hardcodes an address
    of the original image, which is a fixed data binding, and the release rule
    requires zero of those. So it stays in the queue and is FLAGGED: it needs a
    row in docs/recovery-binding-classifications.csv, and whoever picks it up
    should know that before writing it rather than at the gate.

    An absolute memory operand is different and is a rejection: it is a read or
    write through an address that is simply not mapped in a standalone test
    executable.
    """
    low, high = span
    callees: set[int] = set()
    reasons: list[str] = []
    bindings: list[int] = []
    inside = range(address, address + size)
    for one in instructions:
        mnemonic = one.mnemonic
        operands = one.operands
        # An absolute memory operand is a global. `opensmacx_at` aside, a
        # recovered body reaching one is a fixed-address binding, which is the
        # thing that puts a function in the hybrid oracle rather than here.
        for operand in operands:
            if (operand.type == X86_OP_MEM and operand.mem.base == 0
                    and operand.mem.index == 0
                    and low <= operand.mem.disp < high):
                reasons.append(f"absolute global {operand.mem.disp:#010x}")
            # An address used as a VALUE, not dereferenced. Testable, but it is
            # a fixed data binding and has to be declared as one.
            if (operand.type == X86_OP_IMM and one.mnemonic != "call"
                    and one.mnemonic != "jmp"
                    and low <= operand.imm < high):
                bindings.append(operand.imm)
        if mnemonic == "call":
            if len(operands) == 1 and operands[0].type == X86_OP_IMM:
                callees.add(operands[0].imm)
            else:
                reasons.append("indirect call")
        elif mnemonic == "jmp":
            if len(operands) == 1 and operands[0].type == X86_OP_IMM:
                if operands[0].imm not in inside:
                    callees.add(operands[0].imm)
            else:
                # `jmp [eax+0xE8]` is a virtual dispatch: the callee is not
                # knowable statically, so condition (2) cannot be decided.
                reasons.append("indirect jump")
    return callees, reasons, bindings


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--functions", type=Path, default=FUNCTIONS_CSV)
    parser.add_argument("--build-dir", type=Path,
                        default=REPO_ROOT / "build")
    parser.add_argument("--max-size", type=int, default=64,
                        help="skip bodies larger than this many bytes")
    parser.add_argument("--show-rejected", action="store_true")
    args = parser.parse_args(argv)

    sources = leaf_test_sources(args.build_dir)

    rows = load_rows(args.functions)
    state = {int(row["address"], 16): row["recovery_state"] for row in rows}
    names = {int(row["address"], 16): row["name"] for row in rows}
    ordered = sorted(state)

    pe = pefile.PE(str(args.exe), fast_load=True)
    span = image_span(pe)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True

    def source_for(name: str) -> str | None:
        match = re.match(r"\?\w+@(\w+)@@", name)
        return match.group(1).lower() if match else None

    accepted, rejected = [], []
    for row in rows:
        if row["recovery_state"] != "unrecovered":
            continue
        size = int(row["size"] or 0)
        if not size or size > args.max_size:
            continue
        address = int(row["address"], 16)
        owner = source_for(row["name"])
        why: list[str] = []
        if owner is None:
            why.append("no class in the mangled name")
        elif owner not in sources:
            why.append(f"class {owner} is not in recovery-leaf-tests")

        data = body_bytes(pe, address, size)
        if len(data) < size:
            why.append("body not readable from the image")
            rejected.append((address, row["name"], why))
            continue
        instructions = list(decoder.disasm(data, address))
        if sum(one.size for one in instructions) < size:
            why.append("does not fully decode")
        callees, reasons, bindings = classify(instructions, address, size, span)
        why.extend(reasons)

        # The patch slack rule: a five-byte jump has to fit.
        position = ordered.index(address) if address in state else -1
        if 0 <= position < len(ordered) - 1:
            slack = ordered[position + 1] - address
            if slack < REDIRECT_BYTES:
                why.append(f"only {slack}B of slack")

        for callee in sorted(callees):
            if state.get(callee) != "source_complete":
                why.append(f"callee {callee:#010x} is "
                           f"{state.get(callee, 'uncatalogued')}")
                continue
            callee_owner = source_for(names.get(callee, ""))
            if callee_owner is None or callee_owner not in sources:
                # Condition (3), and the one BattleWin::stop_timer failed.
                why.append(f"callee {names.get(callee, hex(callee))} is not "
                           f"linked into recovery-leaf-tests")

        if why:
            rejected.append((address, row["name"], why))
        else:
            accepted.append((size, address, row["name"], bindings))

    accepted.sort()
    print(f"{len(accepted)} leaf-testable candidates "
          f"({sum(size for size, _, _, _ in accepted)} bytes)\n")
    for size, address, name, bindings in accepted:
        note = ""
        if bindings:
            note = ("   [fixed-address binding: "
                    + ", ".join(f"{one:#010x}" for one in sorted(set(bindings)))
                    + " - needs classification]")
        print(f"  {address:#010x} {size:4d} B  {name}{note}")
    if args.show_rejected:
        print(f"\n{len(rejected)} rejected:")
        for address, name, why in sorted(rejected):
            print(f"  {address:#010x} {name[:52]}")
            for reason in dict.fromkeys(why):
                print(f"        {reason}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
