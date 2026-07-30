#!/usr/bin/env python3
"""Check the Z3 encoding against the PROCESSOR, not against another model.

`validate_x86_smt.py` compares the encoding with `lifted_x86.h`. That is the
right reference for the lift - it is the semantics the lifted image runs - but
both are models written by people who read the same manual, so a shared
misunderstanding passes both. This closes that gap by executing the real
instruction bytes on a real CPU and demanding all three agree.

WHAT A DISAGREEMENT MEANS DEPENDS ON WHO DISAGREES, and the three-way split is
the whole point of running it this way:

  z3 alone differs        the encoding is wrong. Fix x86_smt.py.
  header alone differs    lifted_x86.h is wrong, which is a finding about the
                          LIFT and not about this spike - every lifted body
                          that uses that operation inherits it.
  both differ together    a shared misreading of the manual, which is the
                          failure a second model cannot detect by construction.

Instruction bytes are written out per operation rather than assembled, so what
executes is exactly what is named here and no assembler's opinion enters.
Requires Wine, because the executor is a 32-bit Windows binary; this host has
no 32-bit Linux multilib.
"""

from __future__ import annotations

import argparse
import collections
import random
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import x86_smt  # noqa: E402
import validate_x86_smt as against_header  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent

# (operation, width) -> the bytes that compute it into EAX/AL from ECX/CL.
# `ret` is appended by the driver. A shift takes its count in CL, which is why
# the shift rows read the second operand from ecx like everything else.
ENCODINGS = {
    ("add", 32): "01c8", ("add", 8): "00c8",
    ("adc", 32): "11c8", ("adc", 8): "10c8",
    ("sub", 32): "29c8", ("sub", 8): "28c8",
    ("sbb", 32): "19c8", ("sbb", 8): "18c8",
    ("and", 32): "21c8", ("and", 8): "20c8",
    ("or", 32): "09c8", ("or", 8): "08c8",
    ("xor", 32): "31c8", ("xor", 8): "30c8",
    ("inc", 32): "40", ("inc", 8): "fec0",
    ("dec", 32): "48", ("dec", 8): "fec8",
    ("neg", 32): "f7d8", ("neg", 8): "f6d8",
    ("not", 32): "f7d0", ("not", 8): "f6d0",
    ("shl", 32): "d3e0", ("shl", 8): "d2e0",
    ("shr", 32): "d3e8", ("shr", 8): "d2e8",
    ("sar", 32): "d3f8", ("sar", 8): "d2f8",
    ("imul", 32): "0fafc1",
}

WINDOW = "00" * 256
FLAG_MASK = 0x8D5


def undefined_flags(op: str, width: int, count: int) -> int:
    """Flags this operation leaves ARCHITECTURALLY UNDEFINED.

    These must be excluded from a comparison against silicon, and the reason is
    not tolerance for disagreement - it is that there is nothing to agree with.
    Where the manual defines nothing, the CPU is free to produce anything and a
    different stepping may.

    lifted_x86.h writes several of them anyway, and says so: "SF/ZF/PF/AF are
    architecturally undefined; they are written from the truncated result so
    that the same lifted code always behaves the same way." That is a choice of
    DETERMINISM over fidelity, and it is the right one for a lift - but it
    means the header and the CPU genuinely differ there, and reporting that as
    a defect would be reporting a decision as a bug.

    Found by measurement, not by reading: the first silicon run flagged 32
    cases where both models agreed with each other and differed from the
    processor, and every one was in this territory.
    """
    if op == "imul":
        return x86_smt.SF | x86_smt.ZF | x86_smt.AF | x86_smt.PF
    if op in ("shl", "shr", "sar"):
        # AF is undefined for every shift, and OF for every count except 1.
        undefined = x86_smt.AF
        if (count & 31) != 1:
            undefined |= x86_smt.OF
        return undefined
    if op in ("and", "or", "xor", "not"):
        return x86_smt.AF
    return 0


def build_executor(source: Path, output: Path) -> Path:
    output.parent.mkdir(parents=True, exist_ok=True)
    done = subprocess.run(
        ["i686-w64-mingw32-g++", "-std=c++17", "-O1", "-static",
         str(source), "-o", str(output)],
        capture_output=True, text=True)
    if done.returncode != 0:
        raise SystemExit(f"cannot build the executor:\n{done.stderr}")
    return output


def run_cpu(binary: Path, cases: list[tuple]) -> list[tuple[int, int]]:
    lines = []
    for op, width, a, b, flags in cases:
        code = ENCODINGS[(op, width)] + "c3"
        lines.append(f"{code} {a:x} {b:x} 0 0 0 0 0 {flags:x} {WINDOW}")
    done = subprocess.run(["wine", str(binary)], input="\n".join(lines) + "\n",
                          capture_output=True, text=True)
    answers = []
    for line in done.stdout.splitlines():
        parts = line.split()
        if len(parts) < 8:
            answers.append(None)
            continue
        answers.append((int(parts[0], 16), int(parts[7], 16)))
    if not answers:
        raise SystemExit(f"the executor produced nothing:\n{done.stderr[:400]}")
    return answers


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--cases", type=int, default=200)
    parser.add_argument("--seed", type=int, default=20260730)
    parser.add_argument("--executor", type=Path,
                        default=REPO_ROOT / "build" / "x86_smt_execute.exe")
    parser.add_argument("--source", type=Path,
                        default=REPO_ROOT / "tools" / "x86_smt_execute.cpp")
    args = parser.parse_args(argv)

    binary = build_executor(args.source, args.executor)
    reference = against_header.build_reference(
        REPO_ROOT / "tools" / "x86_smt_reference.cpp",
        REPO_ROOT / "build" / "x86_smt_reference")
    rng = random.Random(args.seed)

    cases = []
    for (op, width) in sorted(ENCODINGS):
        for _ in range(args.cases):
            a = against_header.sample(width, rng)
            b = against_header.sample(width, rng)
            flags = rng.choice([0, x86_smt.CF, FLAG_MASK, rng.getrandbits(12)])
            cases.append((op, width, a, b, flags & FLAG_MASK))

    cpu = run_cpu(binary, cases)
    header = against_header.run_reference(reference, cases)

    checked = 0
    excluded = collections.defaultdict(int)
    only_z3, only_header, both = [], [], []
    for case, silicon, model in zip(cases, cpu, header):
        if silicon is None or model is None:
            continue
        op, width, a, b, flags = case
        encoded = x86_smt.concrete(op, width, a, b, flags)
        # The CPU reports a full 32-bit EAX even for an 8-bit operation, and
        # only the low byte is defined by the operation; mask before comparing
        # or every 8-bit case reports a difference in bits nothing wrote.
        mask = x86_smt.mask_of(width)
        flag_mask = FLAG_MASK & ~undefined_flags(op, width, b)
        excluded[op] |= undefined_flags(op, width, b)
        silicon_value = (silicon[0] & mask, silicon[1] & flag_mask)
        model_value = (model[0] & mask, model[1] & flag_mask)
        encoded_value = (encoded[0] & mask, encoded[1] & flag_mask)
        checked += 1
        if encoded_value == silicon_value and model_value == silicon_value:
            continue
        if model_value == silicon_value:
            only_z3.append((case, silicon_value, encoded_value))
        elif encoded_value == silicon_value:
            only_header.append((case, silicon_value, model_value))
        else:
            both.append((case, silicon_value, encoded_value, model_value))

    print(f"{checked} case(s) run on the CPU, in lifted_x86.h and in Z3")
    names = {x86_smt.CF: "CF", x86_smt.PF: "PF", x86_smt.AF: "AF",
             x86_smt.ZF: "ZF", x86_smt.SF: "SF", x86_smt.OF: "OF"}
    for op in sorted(excluded):
        if not excluded[op]:
            continue
        spelt = ", ".join(name for bit, name in names.items()
                          if excluded[op] & bit)
        print(f"  {op}: {spelt} excluded - architecturally undefined, so there "
              f"is nothing for the CPU and a model to agree ABOUT")
    if not (only_z3 or only_header or both):
        print(f"all three agree on every case, across "
              f"{len(ENCODINGS)} encoded instruction form(s)")
        return 0

    if only_z3:
        print(f"\n{len(only_z3)} case(s) where THE Z3 ENCODING is wrong:")
        for (op, width, a, b, flags), want, got in only_z3[:6]:
            print(f"  {op}{width} a={a:#x} b={b:#x} flags={flags:#x}: "
                  f"cpu {want}, z3 {got}")
    if only_header:
        print(f"\n{len(only_header)} case(s) where LIFTED_X86.H is wrong - a "
              f"finding about the lift, not about this spike:")
        for (op, width, a, b, flags), want, got in only_header[:6]:
            print(f"  {op}{width} a={a:#x} b={b:#x} flags={flags:#x}: "
                  f"cpu {want}, header {got}")
    if both:
        print(f"\n{len(both)} case(s) where BOTH MODELS agree with each other "
              f"and differ from the CPU - a shared misreading:")
        for (op, width, a, b, flags), want, z, h in both[:6]:
            print(f"  {op}{width} a={a:#x} b={b:#x} flags={flags:#x}: "
                  f"cpu {want}, z3 {z}, header {h}")
    return 1


if __name__ == "__main__":
    sys.exit(main())
