#!/usr/bin/env python3
"""Decide, before running anything, which functions the differential oracle can
test against the original machine code - and for the rest, WHY NOT.

The oracle calls the original function for real, inside a process that has the
whole image mapped. That is what makes it need no fixtures, and it is also what
makes some functions untestable:

  * IMPORTS. A call that reaches the IAT leaves the image. On the original side
    it enters the real DDRAW/USER32/KERNEL32; on the lifted side there is
    nothing there. Comparing the two would be comparing a real DirectDraw call
    against a stub. Reachability is TRANSITIVE: a function three calls away from
    `ExitProcess` is exactly as untestable as one that calls it directly.

    THESE FLAGS ARE NO LONGER A REFUSAL BY DEFAULT. `lifted_oracle_main.cpp`
    attempts a flagged function and lets the run decide; `--refuse-blocked`
    restores the refusal. This entry used to justify itself with "running it
    could pop a window, open a file or exit the process", and that is not what
    happens: `oracle_load_image` never processes the import directory, so an
    IAT slot still holds its file-image value and the call lands in the walled
    low 118 MiB and faults. The hazard is contained - but it was contained by
    accident rather than by statement, so it is stated here now. What DOES
    escape containment is the host: on arm64 Rosetta refuses some of these
    outright and kills the process (see lifted_oracle_sweep.sh).

  * fs:. The 1,632 fs:[0] sites maintain the SEH chain, and the lift keeps that
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
from capstone import x86 as cs_x86  # noqa: E402
from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_REG_INVALID  # noqa: E402

import lift_whole_image as lift  # noqa: E402

X87_PREFIXES = ("f",)   # every x87 mnemonic starts with f; see X87_EXCEPT below
X87_EXCEPT = {"fs"}     # nothing else in the decoder starts with f but is not x87

# ---------------------------------------------------------------------------
# Flags the ORIGINAL is allowed to disagree about
#
# IDIV, DIV, MUL and IMUL leave some flags architecturally UNDEFINED.
# `lifted_muldiv.h` and `lifted_x86.h` write a deterministic value there on
# purpose; the silicon - or, on this host, the x86-on-ARM layer Wine runs the
# original through - writes a different one. Measured on this host with the
# oracle's own seeds and entry flags 0x202:
#
#   ?fixed_div@@YAHJJ@Z    the whole `sar/shl/idiv` body comes back with the
#                          ENTRY flags: the emulator's IDIV discards the
#                          pending flag update from the shifts.
#   ?speed_proto@@YAHH@Z   `imul eax,esi` comes back with SF ZF PF AF all
#                          CLEAR whatever the product is - including for a
#                          NEGATIVE product, which is why SF is undefined too.
#   ?guard_check@@YAHHH@Z  the same IDIV comes back with CF SET, where the
#                          same instruction on the same operands in a
#                          standalone probe leaves CF clear.
#
# Three different answers from one instruction class is what "undefined"
# means, and no lowering can be written that matches all three. So a flag that
# is undefined on every path reaching a RET is not compared. Everything else
# still is - including CF and OF after IMUL, which ARE defined, and every flag
# after the shifts, whose OF was measured to agree with lifted_x86.h's uniform
# rule in 651 of 651 cases.
#
# The mask is computed per function and travels in the plan as `undef=<hex>`,
# in the oracle's own OracleFlagBit numbering.
ORACLE_FLAG_BITS = {
    "CF": 1 << 0, "PF": 1 << 1, "AF": 1 << 2, "ZF": 1 << 3,
    "SF": 1 << 4, "OF": 1 << 5, "DF": 1 << 6,
}

# capstone reports each flag three ways: MODIFY/RESET/SET write a DEFINED
# value, UNDEFINED writes an undefined one. TEST and PRIOR are reads.
_CS_DEFINES: dict[int, int] = {}
_CS_UNDEFINES: dict[int, int] = {}
for _name, _bit in ORACLE_FLAG_BITS.items():
    for _kind in ("MODIFY", "RESET", "SET"):
        _value = getattr(cs_x86, f"X86_EFLAGS_{_kind}_{_name}", 0)
        if _value:
            _CS_DEFINES[_value] = _CS_DEFINES.get(_value, 0) | _bit
    _value = getattr(cs_x86, f"X86_EFLAGS_UNDEFINED_{_name}", 0)
    if _value:
        _CS_UNDEFINES[_value] = _CS_UNDEFINES.get(_value, 0) | _bit

# capstone is WRONG about one flag, and it is one this image depends on. For
# IMUL in every form the SDM says "the SF, ZF, AF, and PF flags are undefined";
# capstone reports MODIFY_SF. Measured on this host: `imul` of -5 * 7 = -35 and
# of -1 * 1 = -1 both come back with SF CLEAR, so SF is not the sign of the
# product there and the manual is right. Believing capstone left sub_559210 -
# `cdq / idiv ecx / imul eax,ecx` - reported as a lowering defect in SF. MUL is
# already correct in capstone; it is listed anyway so the two multiplies cannot
# drift apart.
MULTIPLY_UNDEFINED = (ORACLE_FLAG_BITS["SF"] | ORACLE_FLAG_BITS["ZF"]
                      | ORACLE_FLAG_BITS["AF"] | ORACLE_FLAG_BITS["PF"])


def instruction_flag_effect(one) -> tuple[int, int]:
    """(defined, undefined) masks in OracleFlagBit numbering."""
    eflags = one.eflags
    defined = undefined = 0
    if one.mnemonic in ("imul", "mul"):
        undefined |= MULTIPLY_UNDEFINED
    for value, bit in _CS_DEFINES.items():
        if eflags & value:
            defined |= bit
    for value, bit in _CS_UNDEFINES.items():
        if eflags & value:
            undefined |= bit
    # A flag cannot be both; capstone never says so, but a decoder change that
    # did would silently widen the mask, so undefined loses.
    return defined & ~undefined, undefined


def undefined_exit_flags(instructions: dict, spans: list) -> int:
    """Flags that are undefined on SOME path reaching a return.

    A forward, union-merge dataflow over the function's own instructions.
    Entry state is "nothing undefined": the seeded entry flags are a defined
    input both sides receive. `call` resets the state, because the flags after
    a call belong to the callee - a callee whose own exit flags are undefined
    is that callee's plan row, not this one's. An indirect or out-of-function
    transfer is treated as a return, which masks more rather than less.

    The result is a union over every RET, because the oracle cannot know which
    one a case took. A function that reaches one RET after a divide and another
    after a compare therefore masks the flag at both.
    """
    if not instructions:
        return 0
    inside = set()
    for low, high in spans:
        inside.update(range(low, high))
    entry = min(instructions)
    state = {entry: 0}
    work = [entry]
    exits = 0
    while work:
        address = work.pop()
        one = instructions.get(address)
        if one is None:
            continue
        incoming = state[address]
        defined, undefined = instruction_flag_effect(one)
        outgoing = (incoming & ~defined) | undefined
        mnemonic = one.mnemonic
        if mnemonic.startswith("ret") or mnemonic in ("iret", "iretd", "hlt"):
            exits |= incoming          # RET itself writes no flag
            continue
        if mnemonic == "call":
            outgoing = 0               # the callee decides the flags
            successors = [address + one.size]
        elif mnemonic == "jmp":
            operand = one.operands[0]
            if operand.type == X86_OP_IMM and operand.imm in inside:
                successors = [operand.imm]
            else:
                exits |= outgoing      # tail call or switch: treat as a return
                continue
        elif mnemonic.startswith("j") or mnemonic.startswith("loop"):
            successors = [address + one.size]
            operand = one.operands[0]
            if operand.type == X86_OP_IMM:
                if operand.imm in inside:
                    successors.append(operand.imm)
                else:
                    exits |= outgoing
        else:
            successors = [address + one.size]
        for successor in successors:
            if successor not in instructions:
                exits |= outgoing
                continue
            merged = state.get(successor, 0) | outgoing
            if successor not in state or merged != state[successor]:
                state[successor] = merged
                work.append(successor)
    return exits



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
        decoded: dict[int, object] = {}
        for low, high in function["spans"]:
            data = lift.read_bytes(pe, low, high - low)
            for one in md.disasm(data, low):
                decoded[one.address] = one
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
        function["undef"] = undefined_exit_flags(decoded, function["spans"])


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
    # The `undef=` mask exists because a TRANSLATED host gave three different
    # answers for one instruction class's architecturally-undefined flags, so
    # those flags could not be compared against anything. On native silicon
    # there is one answer, and suppressing the token compares them instead.
    #
    # This direction is always safe to try: dropping the mask makes the oracle
    # STRICTER - it compares bits it was previously ignoring - so it can only
    # turn a PASS into a FAIL, never the reverse. That is why it is a real
    # experiment. A new FAIL here is not a regression in the lift; it is a
    # lowering bug the mask was hiding, and it is the more valuable finding.
    parser.add_argument(
        "--no-undef", action="store_true",
        help="do not emit undef= masks; compare the undefined flags too")
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
            undef = 0 if args.no_undef else function.get("undef", 0)
            printed = set(flags)
            if undef:
                printed.add(f"undef={undef:x}")
                histogram["undef"] += 1
                for name, bit in ORACLE_FLAG_BITS.items():
                    if undef & bit:
                        histogram[f"undef-{name}"] += 1
            text = ",".join(sorted(printed)) if printed else "ok"
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
