#!/usr/bin/env python3
"""Decide, before running anything, which functions the differential oracle can
test against the original machine code - and for the rest, WHY NOT.

The oracle calls the original function for real, inside a process that has the
whole image mapped. That is what makes it need no fixtures, and it is also what
makes some functions untestable:

  * IMPORTS. A call that reaches the IAT leaves the image. On the original side
    it enters the real DDRAW/USER32/KERNEL32; on the lifted side there is
    nothing there. Comparing the two would be comparing a real DirectDraw call
    against a stub, and running it could pop a window, open a file or exit the
    process. Reachability is TRANSITIVE: a function three calls away from
    `ExitProcess` is exactly as untestable as one that calls it directly.

  * fs:. The 1,330 fs:[0] sites maintain the SEH chain, and the lift keeps that
    chain in a thread-local block that is deliberately NOT in the image. The
    original pushes the real chain head onto the guest stack, the lifted side
    pushes 0xFFFFFFFF, and the memory comparison then reports a difference that
    is a property of the design rather than a bug. Skipped, and counted.

  * _SELFMOD. Self-modifying code has no static translation, so the lift emits
    traps there. Nothing to compare.

  * A CALLEE THE LIFT HAS NO BODY FOR. The lifted side turns a direct call into
    a dispatch-table lookup, and an address outside the table traps. That is a
    skip with a reason, not a failure.

Two more flags are informational rather than disqualifying, because the
functions carrying them ARE tested and it matters how many:

  * x87  - the FPU is reset before each side but its registers are not
    compared, so a divergence confined to ST(0) would be missed.
  * indirect - `call [reg]`. The seed makes the arena non-executable, so an
    indirect call through soup faults rather than running the soup, and the
    case comes out INCONCLUSIVE instead of wild.

Usage:
    lifted_oracle_plan.py [--exe ...] [--functions ...] [--out build/oracle/plan.tsv]
"""

from __future__ import annotations

import argparse
import sys
from collections import Counter
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO_ROOT / "tools"))

import pefile  # noqa: E402
from capstone import CS_ARCH_X86, CS_MODE_32, Cs  # noqa: E402
from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_REG_INVALID  # noqa: E402

import lift_whole_image as lift  # noqa: E402

X87_PREFIXES = ("f",)   # every x87 mnemonic starts with f; see X87_EXCEPT below
X87_EXCEPT = {"fs"}     # nothing else in the decoder starts with f but is not x87


def iat_slots(pe: pefile.PE) -> set[int]:
    """Every address that holds an imported function pointer.

    Taken from the parsed import directory rather than from the section
    boundaries: .rdata holds much more than the IAT, and treating all of it as
    "an import" would skip half the image for no reason.
    """
    pe.parse_data_directories()
    slots: set[int] = set()
    for entry in getattr(pe, "DIRECTORY_ENTRY_IMPORT", []) or []:
        for imported in entry.imports:
            if imported.address:
                slots.add(imported.address)
    for entry in getattr(pe, "DIRECTORY_ENTRY_DELAY_IMPORT", []) or []:
        for imported in entry.imports:
            if imported.address:
                slots.add(imported.address)
    return slots


def scan(pe: pefile.PE, functions: list[dict], slots: set[int]) -> None:
    """Per-function local facts plus the direct-call edges, in one pass."""
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    md.detail = True
    owner: dict[int, int] = {}
    for function in functions:
        for low, high in function["spans"]:
            owner[low] = function["address"]

    entries = {function["address"] for function in functions}
    for function in functions:
        local = set()
        edges: set[int] = set()
        inside = set()
        for low, high in function["spans"]:
            inside.update(range(low, high))
        if function["section"] in lift.UNLIFTABLE_SECTIONS:
            local.add("selfmod")
        for low, high in function["spans"]:
            data = lift.read_bytes(pe, low, high - low)
            for one in md.disasm(data, low):
                mnemonic = one.mnemonic
                if mnemonic.startswith("f") and mnemonic not in X87_EXCEPT:
                    local.add("x87")
                for operand in one.operands:
                    if operand.type != X86_OP_MEM:
                        continue
                    if operand.mem.segment != X86_REG_INVALID:
                        # Test the SEGMENT REGISTER, not the mnemonic. The
                        # mnemonic test was there to exclude the string
                        # instructions' implicit es:, but `movsb`/`movsw`/
                        # `movsd` all start with "mov", so four functions were
                        # flagged and skipped for an fs: they do not contain -
                        # 0x00592940, 0x005a6ac0, 0x00639090, 0x0063f9b0 - and
                        # 0x00592940 PASSES when it is actually run. Naming the
                        # register is both narrower and exactly the question.
                        if one.reg_name(operand.mem.segment) == "fs":
                            local.add("fs")
                    if (operand.mem.base == X86_REG_INVALID
                            and operand.mem.index == X86_REG_INVALID
                            and operand.mem.disp in slots):
                        local.add("iat")
                if mnemonic in ("call", "jmp"):
                    operand = one.operands[0]
                    if operand.type == X86_OP_IMM:
                        target = operand.imm
                        if target not in inside:
                            if target in entries:
                                edges.add(target)
                            else:
                                # A direct transfer to something that is not a
                                # catalogued function entry: a CRT routine, an
                                # import thunk, or the middle of a neighbour.
                                # The lift has no body for it either way.
                                local.add("extcall")
                    elif mnemonic == "call":
                        local.add("indirect")
        function["local"] = local
        function["edges"] = edges


def close_over_calls(functions: list[dict]) -> None:
    """Propagate the disqualifying facts backwards along the call graph.

    Reachability is what matters: calling a neighbour that calls DirectDraw is
    calling DirectDraw. Iterated to a fixpoint rather than recursed, so the
    image's mutual recursion cannot blow the stack or loop forever.
    """
    by_address = {function["address"]: function for function in functions}
    for function in functions:
        function["flags"] = set(function["local"])
    spreading = {"iat", "fs", "extcall", "selfmod", "x87", "indirect"}
    changed = True
    rounds = 0
    while changed:
        changed = False
        rounds += 1
        for function in functions:
            inherited: set[str] = set()
            for target in function["edges"]:
                callee = by_address.get(target)
                if callee:
                    inherited |= callee["flags"] & spreading
            if not inherited <= function["flags"]:
                function["flags"] |= inherited
                changed = True
    return rounds


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--exe", type=Path, default=lift.DEFAULT_EXE)
    parser.add_argument("--functions", type=Path, default=lift.FUNCTIONS_CSV)
    parser.add_argument("--out", type=Path,
                        default=REPO_ROOT / "build" / "oracle" / "plan.tsv")
    args = parser.parse_args()

    pe = pefile.PE(str(args.exe), fast_load=True)
    functions = lift.load_functions(pe, args.functions)
    slots = iat_slots(pe)
    scan(pe, functions, slots)
    rounds = close_over_calls(functions)

    args.out.parent.mkdir(parents=True, exist_ok=True)
    histogram: Counter = Counter()
    testable = 0
    with args.out.open("w", encoding="utf-8") as handle:
        handle.write("# address\tflags\tname   (generated by lifted_oracle_plan.py)\n")
        for function in sorted(functions, key=lambda item: item["address"]):
            flags = function["flags"]
            blocking = flags & {"iat", "fs", "extcall", "selfmod"}
            if not blocking:
                testable += 1
                histogram["testable"] += 1
            for flag in flags:
                histogram[flag] += 1
            text = ",".join(sorted(flags)) if flags else "ok"
            handle.write(f"{function['address']:#010x}\t{text}\t{function['name']}\n")

    print(f"functions              {len(functions)}")
    print(f"IAT slots              {len(slots)}")
    print(f"call-graph rounds      {rounds}")
    print(f"testable (no blocker)  {testable}")
    for flag, count in histogram.most_common():
        print(f"  {flag:<10} {count}")
    print(f"\nwrote {args.out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
