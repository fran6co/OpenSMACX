#!/usr/bin/env python3
"""Check whole BLOCKS - memory included - against the processor.

`validate_x86_silicon.py` checks one instruction at a time, which is enough for
the ALU but says nothing about the parts with no second model to compare
against: the flat `Array(BV32, BV8)` memory, little-endian access at three
widths, sub-register writes that must preserve the bits around them, and the
address arithmetic behind `base + index*scale + disp`.

Those are checked here the only way that means anything - by running the same
bytes on a real CPU with the same registers and the same 256-byte window, then
demanding the encoder's formula, fully concretised, produce the same registers,
the same defined flags AND the same 256 bytes back.

THE WINDOW IS COMPARED IN FULL, not just the addresses a sequence was expected
to touch. A store to the wrong address is exactly the bug this is for, and
checking only the intended addresses would miss it by construction.
"""

from __future__ import annotations

import argparse
import random
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import z3  # noqa: E402

import x86_smt  # noqa: E402
import validate_x86_silicon as silicon  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
WINDOW_BASE = 0x30000000
WINDOW_SIZE = 256
FLAG_MASK = 0x8D5

# Straight-line sequences, written as bytes so what executes is what is named.
# Each ends with `ret`, and each is chosen to exercise something the
# instruction-at-a-time validator cannot reach.
# The ESP the executor loads before running the block. Fixed in both places so
# push and pop are comparable: with the host's own ESP the encoder would have
# to know where the harness's frame landed, which is not a property of the
# program under test.
GUEST_ESP = 0x3001F000

# Straight-line sequences, written as bytes so what executes is what is named.
# `small` names registers that take part in an ADDRESS and must therefore stay
# inside the window - a random 32-bit value in a base or index register reads
# somewhere unmapped and takes the executor down, which is a bug in the test
# rather than a finding about the encoder.
BLOCKS = {
    "load add store":      ("a100000030" "01c8" "a304000030" "c3", ()),
    "byte load and store": ("8a0d00000030" "880d10000030" "c3", ()),
    "16-bit through ax":   ("66a108000030" "6601c8" "66a318000030" "c3", ()),
    # 898c98, not 890c98: the latter is `mov [eax+ebx*4], ecx` with NO
    # displacement, which silently turns the four following bytes into stray
    # instructions that write near address zero. Hand-written encodings get
    # disassembled below for exactly this reason.
    "base plus index":     ("8b8c9800000030" "898c9820000030" "c3",
                            ("eax", "ebx")),
    "sub-register write":  ("b040" "a320000030" "c3", ()),
    "push then pop":       ("50" "51" "58" "5b" "c3", ()),
    "lea is not a load":   ("8d0d40000030" "890d30000030" "c3", ()),
    "movzx widens":        ("0fb60500000030" "a324000030" "c3", ()),
    "movsx sign extends":  ("0fbe0500000030" "a328000030" "c3", ()),
    "read back a write":   ("a320000030" "8b1d20000030" "c3", ()),
    # Branches. Each has two rets, and the decode guard below still requires
    # the LAST instruction to be one - a sequence whose final byte is not a
    # ret would run off the end into whatever the page holds.
    "branch on equality":  ("39c8" "7506" "b801000000" "c3"
                            "b802000000" "c3", ()),
    "branch on sign":      ("85c0" "7906" "b8ffffffff" "c3"
                            "b801000000" "c3", ()),
}

# EVERY CONDITION, not a selection. A control that damaged `ge` to lose its
# equality case went undetected, because no sequence tested `ge` at all - the
# validator was silently covering five conditions out of sixteen while reading
# as though it covered the family. `cmp eax, ecx` then `set<cc> al`, one per
# encoding, closes that: the second opcode byte is 0x90 + the condition's own
# number, so the table below IS the x86 encoding rather than a transcription of
# it.
SETCC_CONDITIONS = (
    "o", "no", "b", "ae", "e", "ne", "be", "a",
    "s", "ns", "p", "np", "l", "ge", "le", "g",
)
for _index, _name in enumerate(SETCC_CONDITIONS):
    BLOCKS[f"setcc {_name}"] = (f"39c80f{0x90 + _index:02x}c0c3", ())


def concrete_state(regs: dict, flags: int, window: bytes) -> x86_smt.State:
    """A State whose every term is a literal, so the result simplifies out."""
    state = x86_smt.State.__new__(x86_smt.State)
    state.regs = {name: z3.BitVecVal(regs[name], 32)
                  for name in x86_smt.REGISTERS}
    state.flags = z3.BitVecVal(flags, 32)
    memory = z3.K(z3.BitVecSort(32), z3.BitVecVal(0, 8))
    for index, byte in enumerate(window):
        memory = z3.Store(memory, z3.BitVecVal(WINDOW_BASE + index, 32),
                          z3.BitVecVal(byte, 8))
    state.mem = memory
    return state


def encoded_result(code: bytes, regs: dict, flags: int, window: bytes):
    state = concrete_state(regs, flags, window)
    # encode_function, not encode_block: it handles a straight run as the
    # degenerate one-path case and additionally merges branches, so every
    # sequence goes through the same code the prover will use.
    out = x86_smt.encode_function(code, 0x1000, state)
    values = {name: z3.simplify(out.regs[name]).as_long()
              for name in x86_smt.REGISTERS}
    got_flags = z3.simplify(out.flags).as_long()
    got_window = bytes(
        z3.simplify(z3.Select(out.mem, z3.BitVecVal(WINDOW_BASE + i, 32))
                    ).as_long()
        for i in range(WINDOW_SIZE))
    return values, got_flags, got_window


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--cases", type=int, default=25)
    parser.add_argument("--seed", type=int, default=20260730)
    parser.add_argument("--executor", type=Path,
                        default=REPO_ROOT / "build" / "x86_smt_execute.exe")
    args = parser.parse_args(argv)

    # EVERY BLOCK IS DISASSEMBLED BEFORE IT RUNS. These byte strings are
    # written by hand, and a wrong ModRM does not fail - it decodes into a
    # DIFFERENT program that still executes. One of them did: `890c98` has no
    # displacement, so four bytes of intended address became two stray
    # instructions that wrote near address zero and took the executor down.
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    engine = Cs(CS_ARCH_X86, CS_MODE_32)
    for label, (code_hex, _) in sorted(BLOCKS.items()):
        decoded = list(engine.disasm(bytes.fromhex(code_hex), 0x1000))
        consumed = sum(len(item.bytes) for item in decoded)
        if consumed != len(bytes.fromhex(code_hex)):
            print(f"error: [{label}] does not decode cleanly", file=sys.stderr)
            return 2
        if not decoded or decoded[-1].mnemonic != "ret":
            print(f"error: [{label}] does not end in ret", file=sys.stderr)
            return 2

    binary = silicon.build_executor(
        REPO_ROOT / "tools" / "x86_smt_execute.cpp", args.executor)
    rng = random.Random(args.seed)

    cases, lines = [], []
    for label, (code_hex, small) in sorted(BLOCKS.items()):
        for _ in range(args.cases):
            regs = {name: rng.getrandbits(32) for name in x86_smt.REGISTERS}
            regs["esp"] = GUEST_ESP
            regs["ebp"] = 0
            # THE COMPARED PAIR NEEDS ITS BOUNDARIES. Every branch and setcc
            # sequence is `cmp eax, ecx`, and with two uniform 32-bit draws the
            # two are never equal and never adjacent - so ZF is always clear and
            # every condition that differs only on equality is untested. A
            # control that damaged `ge` into `g` went undetected for exactly
            # that reason: they differ ONLY when the operands are equal.
            choice = rng.random()
            if choice < 0.25:
                regs["ecx"] = regs["eax"]                      # ZF set
            elif choice < 0.35:
                regs["ecx"] = (regs["eax"] + 1) & 0xFFFFFFFF   # off by one
            elif choice < 0.45:
                regs["ecx"] = (regs["eax"] - 1) & 0xFFFFFFFF
            elif choice < 0.55:
                # Straddles the signed/unsigned split, where `l` and `b`
                # disagree and a swapped condition would otherwise pass.
                regs["eax"], regs["ecx"] = 0x7FFFFFFF, 0x80000000
            for name in small:
                # Small enough that base + index*4 + disp stays in the window.
                regs[name] = rng.randrange(0, 16)
            window = bytes(rng.getrandbits(8) for _ in range(WINDOW_SIZE))
            flags = rng.getrandbits(12) & FLAG_MASK
            cases.append((label, code_hex, regs, flags, window))
            lines.append(
                f"{code_hex} {regs['eax']:x} {regs['ecx']:x} {regs['edx']:x} "
                f"{regs['ebx']:x} {regs['ebp']:x} {regs['esi']:x} "
                f"{regs['edi']:x} {flags:x} {window.hex()}")

    executable, launcher = binary
    done = subprocess.run(launcher + [str(executable)],
                          input="\n".join(lines) + "\n",
                          capture_output=True, text=True)
    answers = [line.split() for line in done.stdout.splitlines()]
    if len(answers) != len(cases):
        print(f"error: {len(cases)} cases, {len(answers)} answers\n"
              f"{done.stderr[:400]}", file=sys.stderr)
        return 2

    checked = 0
    unsupported: dict[str, str] = {}
    bad: list = []
    for (label, code_hex, regs, flags, window), parts in zip(cases, answers):
        if len(parts) < 10:
            continue
        try:
            got, got_flags, got_window = encoded_result(
                bytes.fromhex(code_hex), regs, flags, window)
        except x86_smt.Unsupported as reason:
            # Loud, and counted apart from agreement: an instruction the
            # encoder cannot model is not a case that passed.
            unsupported.setdefault(label, str(reason))
            continue
        checked += 1
        cpu = {name: int(parts[i], 16) for i, name in enumerate(
            ("eax", "ecx", "edx", "ebx", "ebp", "esi", "edi"))}
        # ESP is compared too, which is what makes push/pop mean anything.
        cpu["esp"] = int(parts[8], 16)
        cpu_flags = int(parts[7], 16) & FLAG_MASK
        cpu_window = bytes.fromhex(parts[9])
        wrong = [name for name in cpu if cpu[name] != got[name]]
        if (got_flags & FLAG_MASK) != cpu_flags:
            wrong.append("eflags")
        if cpu_window != got_window:
            first = next(i for i in range(WINDOW_SIZE)
                         if cpu_window[i] != got_window[i])
            wrong.append(f"memory at +{first:#x}")
        if wrong:
            bad.append((label, code_hex, regs, flags, wrong,
                        cpu, got, cpu_flags, got_flags))

    print(f"{checked} block execution(s) compared against the CPU, "
          f"registers, defined flags and all {WINDOW_SIZE} window bytes")
    for label, reason in sorted(unsupported.items()):
        print(f"  NOT MODELLED - {label}: {reason}")
    if not bad:
        print(f"every block agrees, across {len(BLOCKS) - len(unsupported)} "
              f"sequence(s)")
        return 1 if unsupported else 0

    print(f"\n{len(bad)} disagreement(s):")
    for label, code_hex, regs, flags, wrong, cpu, got, cf, gf in bad[:6]:
        print(f"  [{label}] {code_hex}  differs in: {', '.join(wrong)}")
        for name in wrong:
            if name in cpu:
                print(f"      {name}: cpu {cpu[name]:#010x} "
                      f"z3 {got[name]:#010x}")
        if "eflags" in wrong:
            print(f"      eflags: cpu {cf:#06x} z3 {gf & FLAG_MASK:#06x}")
    return 1


if __name__ == "__main__":
    sys.exit(main())
