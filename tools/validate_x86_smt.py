#!/usr/bin/env python3
"""Require the Z3 encoding and lifted_x86.h to agree, bit for bit.

The equivalence spike introduces a second semantics for x86. A second semantics
is a second chance to be wrong, and the dangerous direction is COLLAPSING: an
encoding that makes two different programs look equal proves wrong recoveries
correct and is indistinguishable from success. So the encoding is not reviewed
into correctness, it is driven against the header the lifted image actually
runs and required to match on every bit of the result AND of EFLAGS.

The inputs are deliberately hostile rather than uniform. Uniform 32-bit draws
essentially never produce a carry out of bit 3, a signed overflow, or a zero
result, so a uniform-only run agrees with almost anything - which is how an
encoding with a broken AF or OF passes a thousand samples. The pool here is
boundary-weighted: 0, 1, -1, the width's sign bit and its neighbours, small
values, and only then random ones.

Exit status is non-zero on any disagreement, and the first few are printed with
the operands that produced them so the fix does not need a re-run to reproduce.
"""

from __future__ import annotations

import argparse
import random
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import x86_smt  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_REFERENCE = REPO_ROOT / "build" / "x86_smt_reference"


def interesting(width: int) -> list[int]:
    """Values that actually exercise the flags."""
    mask = x86_smt.mask_of(width)
    sign = x86_smt.sign_of(width)
    pool = {0, 1, 2, 3, 7, 8, 15, 16, 17, 0x7F, 0x80, 0x81, 0xFF,
            mask, mask - 1, sign, sign - 1, sign + 1}
    if width >= 16:
        pool |= {0x7FFF, 0x8000, 0xFFFF, 0x0100, 0x1000}
    if width == 32:
        pool |= {0x7FFFFFFF, 0x80000000, 0xFFFFFFFF, 0x10000, 0xDEADBEEF}
    return sorted(value & mask for value in pool)


def sample(width: int, rng: random.Random) -> int:
    pool = interesting(width)
    # Two thirds boundary values, one third uniform: the boundaries are where
    # AF, OF and CF actually change, and uniform draws hit them almost never.
    if rng.random() < 0.67:
        return rng.choice(pool)
    return rng.getrandbits(width)


def build_reference(source: Path, output: Path) -> Path:
    output.parent.mkdir(parents=True, exist_ok=True)
    done = subprocess.run(
        ["g++", "-std=c++17", "-O1", "-I", str(REPO_ROOT / "tools"),
         "-I", str(REPO_ROOT / "build" / "lifted"), str(source),
         "-o", str(output)],
        capture_output=True, text=True)
    if done.returncode != 0:
        # The include directory below is GENERATED, by
        # tools/lift_whole_image.py, and is gitignored - so on a fresh tree the
        # failure is a missing header reported as a compiler error, which reads
        # like the encoding is broken rather than like a prerequisite is absent.
        # Say which it is.
        if "lifted_runtime.h" in done.stderr:
            raise SystemExit(
                "cannot build the reference harness: build/lifted/"
                "lifted_runtime.h is absent. It is generated, not committed - "
                "run tools/lift_whole_image.py first. Nothing was validated.")
        raise SystemExit(f"cannot build the reference harness:\n{done.stderr}")
    return output


def run_reference(binary: Path, cases: list[tuple]) -> list[tuple[int, int]]:
    payload = "".join(f"{op}{width} {a:x} {b:x} {flags:x}\n"
                      for op, width, a, b, flags in cases)
    done = subprocess.run([str(binary)], input=payload, capture_output=True,
                          text=True)
    if done.returncode != 0:
        raise SystemExit(f"the reference harness failed: {done.stderr}")
    answers = []
    for line in done.stdout.splitlines():
        parts = line.split()
        if len(parts) != 2:
            answers.append(None)
            continue
        answers.append((int(parts[0], 16), int(parts[1], 16)))
    return answers


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--cases", type=int, default=4000,
                        help="random cases per operation and width")
    parser.add_argument("--seed", type=int, default=20260730)
    parser.add_argument("--reference", type=Path, default=DEFAULT_REFERENCE)
    parser.add_argument("--source", type=Path,
                        default=REPO_ROOT / "tools" / "x86_smt_reference.cpp")
    args = parser.parse_args(argv)

    binary = build_reference(args.source, args.reference)
    rng = random.Random(args.seed)

    cases: list[tuple] = []
    for op in x86_smt.SUPPORTED:
        for width in x86_smt.WIDTHS:
            # The reference harness only declares the widths it declares; a
            # missing one must be a loud skip, never a silent pass.
            for _ in range(args.cases):
                a = sample(width, rng)
                b = sample(width, rng)
                flags = rng.choice([0, x86_smt.CF, x86_smt.ALU_FLAGS,
                                    rng.getrandbits(12)])
                cases.append((op, width, a, b, flags))

    answers = run_reference(binary, cases)
    if len(answers) != len(cases):
        print(f"error: asked {len(cases)} cases, got {len(answers)} answers",
              file=sys.stderr)
        return 2

    checked = skipped = 0
    mismatches = []
    for case, expected in zip(cases, answers):
        op, width, a, b, flags = case
        if expected is None:
            skipped += 1
            continue
        got = x86_smt.concrete(op, width, a, b, flags)
        checked += 1
        if got != expected:
            mismatches.append((case, expected, got))

    print(f"{checked} case(s) checked against lifted_x86.h, "
          f"{skipped} unsupported by the reference harness")
    if skipped:
        # A width the harness does not declare is a hole in the validation, not
        # a property of the encoding, and must not read as agreement.
        print(f"WARNING: {skipped} case(s) were never validated")
    if not mismatches:
        # The number CHECKED, not the number supported. Printing
        # len(SUPPORTED) meant the success line was identical whether every
        # operation had been validated or one had: adding an operation to
        # SUPPORTED without teaching the reference harness gave "225 checked,
        # 15 unsupported, 16 operation(s) agree" and exit 0.
        operations = sorted({case[0] for case, expected
                             in zip(cases, answers) if expected is not None})
        print(f"{len(operations)} operation(s) agree on every checked bit of "
              f"result and EFLAGS, over {checked} case(s)")
        if skipped:
            # An unvalidated width is a hole in the validation, not a property
            # of the encoding. The warning above said so and then returned 0.
            print(f"error: {skipped} case(s) were never validated, so this run "
                  f"does not establish the encoding is correct", file=sys.stderr)
            return 1
        return 0

    print(f"\n{len(mismatches)} DISAGREEMENT(S) - the encoding is not usable:")
    for (op, width, a, b, flags), expected, got in mismatches[:12]:
        print(f"  {op}{width} a={a:#010x} b={b:#010x} flags={flags:#x}")
        print(f"      lifted_x86.h  result={expected[0]:#010x} "
              f"eflags={expected[1]:#06x}")
        print(f"      z3 encoding   result={got[0]:#010x} "
              f"eflags={got[1]:#06x}   (differs in "
              f"{expected[1] ^ got[1]:#06x})")
    return 1


if __name__ == "__main__":
    sys.exit(main())
