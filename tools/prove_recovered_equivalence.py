#!/usr/bin/env python3
"""Try to prove a recovered body equivalent to the original's machine code.

This is the SMT spike's gate, run against real data rather than fixtures. It
takes the ORIGINAL's bytes from the hash-bound executable and OUR compiler's
bytes from the release objects, encodes both with `tools/x86_smt.py`, and asks
Z3 whether a caller could tell them apart.

WHAT COUNTS AS SUCCESS IS FIXED IN ADVANCE by the plan, and is deliberately
hostile:

  positive controls   functions already carrying an independent proof must
                      PROVE here too - the spike must agree with what is
                      already known, or it is measuring something else
  must-refuse         a single-instruction mutation of a proven body must NOT
                      prove equal, or the prover is vacuous
  UNKNOWN is neither  a solver that declines to answer has not proved anything,
                      and folding that into the pass column is the one mistake
                      that would make every number here worthless

THE HONEST OBSTACLE, stated up front because it bounds what this can reach: the
recovered bytes come out of an OBJECT FILE, where every reference to a global
or another function is an unrelocated placeholder awaiting the linker. The
original's bytes have real addresses. A body that touches either therefore
compares a real address against a zero and refutes for a reason that says
nothing about the recovery. Bodies that touch neither are the reachable
population, and this reports how large that is instead of hiding it.
"""

from __future__ import annotations

import argparse
import csv
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import pefile  # noqa: E402
from capstone import Cs, CS_ARCH_X86, CS_MODE_32  # noqa: E402

import extract_compiled_recovery as extract  # noqa: E402
import x86_smt  # noqa: E402
from generator_support import read_bytes  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"
FUNCTIONS_CSV = REPO_ROOT / "docs" / "recovery" / "functions.csv"
PROVEN_CSV = REPO_ROOT / "docs" / "recovery" / "proven.csv"


def has_relocations(code: bytes, address: int) -> bool:
    """Does this body name an absolute address or call anything?

    Both are unrelocated in an object file, so either makes the comparison a
    statement about the linker rather than about the recovery.
    """
    engine = Cs(CS_ARCH_X86, CS_MODE_32)
    engine.detail = True
    for item in engine.disasm(code, address):
        if item.mnemonic == "call":
            return True
        for operand in item.operands:
            if operand.type == 3 and not operand.mem.base and not operand.mem.index:
                return True                      # absolute [imm32]
            if operand.type == 2 and abs(operand.imm) > 0x10000:
                return True                      # a large immediate is a pointer
    return False


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--limit", type=int)
    parser.add_argument("--timeout-ms", type=int, default=60000)
    parser.add_argument("--mutate", action="store_true",
                        help="also run the must-refuse control on every body "
                             "that proves: flip one instruction and require "
                             "the prover to refuse it")
    args = parser.parse_args(argv)

    if not args.exe.is_file():
        print(f"error: {args.exe} is missing", file=sys.stderr)
        return 2

    with FUNCTIONS_CSV.open(newline="", encoding="utf-8-sig") as handle:
        rows = {int(row["address"], 16): row for row in csv.DictReader(handle)}
    sizes = {address: int(row["size"] or 0) for address, row in rows.items()}
    # PROJECT THE RETURN BY PROTOTYPE. EAX is a return value only when the
    # function returns something; on a void function it is caller-clobbered
    # scratch, and the original is free to leave a different value in it than
    # the recovery does. Comparing it unconditionally refuted
    # ?set_text_color@Buffer, where the original uses EDX as a spare register
    # and our compiler does not - a difference no caller can observe.
    #
    # EXCEPT FOR CONSTRUCTORS, where the prototype understates the ABI. IDA
    # types `??0X@@QAE@XZ` as returning void, but an MSVC __thiscall
    # constructor returns `this` in EAX and every `new X` relies on it. Trusting
    # the prototype there let a mutant survive: `mov eax, ecx` flipped to
    # `mov al, cl` proved equal on ??0Vector@@QAE@XZ, because the upper 24 bits
    # of the returned `this` were not being compared. The must-refuse control
    # found that, which is what it is for.
    returns = {}
    for address, row in rows.items():
        prototype = (row.get("prototype") or "").strip()
        constructor = row.get("name", "").startswith("??0")
        returns[address] = (constructor
                            or (bool(prototype)
                                and not prototype.startswith("void ")))
    with PROVEN_CSV.open(newline="", encoding="utf-8-sig") as handle:
        proven = [int(row["address"], 16) for row in csv.DictReader(handle)]

    pe = pefile.PE(str(args.exe), fast_load=True)
    bodies, _ = extract.extract(proven, extract.DEFAULT_DLLMAIN,
                                extract.DEFAULT_OBJECTS)
    by_address = {body.address: body for body in bodies}

    tally = {x86_smt.PROVED: 0, x86_smt.REFUTED: 0, x86_smt.UNKNOWN: 0}
    skipped_relocation = 0
    refused_mutants = 0
    surviving_mutants = []
    reasons: dict[str, int] = {}
    slowest = 0.0

    targets = proven[:args.limit] if args.limit else proven
    for address in targets:
        body = by_address.get(address)
        size = sizes.get(address, 0)
        if not body or not size:
            continue
        try:
            original = read_bytes(pe, address, size)
        except Exception:
            continue
        if has_relocations(original, address) or has_relocations(body.code, 0):
            skipped_relocation += 1
            continue
        started = time.monotonic()
        verdict, detail = x86_smt.prove_equivalent(
            original, body.code, address=address, recovered_address=0,
            returns_value=returns.get(address, True),
            timeout_ms=args.timeout_ms)
        slowest = max(slowest, time.monotonic() - started)
        tally[verdict] += 1
        if verdict == x86_smt.UNKNOWN and detail:
            key = str(detail)[:60]
            reasons[key] = reasons.get(key, 0) + 1
        if verdict == x86_smt.PROVED and args.mutate:
            # MUST-REFUSE. Flip the low bit of one byte in the recovered body
            # and require the prover to notice. A prover that still proves this
            # is proving nothing at all.
            for index in range(len(body.code)):
                mutant = bytearray(body.code)
                mutant[index] ^= 0x01
                again, _ = x86_smt.prove_equivalent(
                    original, bytes(mutant), address=address,
                    recovered_address=0,
                    returns_value=returns.get(address, True),
                    timeout_ms=args.timeout_ms)
                if again == x86_smt.PROVED:
                    surviving_mutants.append((address, index))
                else:
                    refused_mutants += 1
                break

    total = sum(tally.values())
    print(f"attempted {total} of {len(targets)} proven function(s)")
    print(f"  {tally[x86_smt.PROVED]:5} PROVED")
    print(f"  {tally[x86_smt.REFUTED]:5} REFUTED")
    print(f"  {tally[x86_smt.UNKNOWN]:5} UNKNOWN   (not a proof, counted apart)")
    print(f"  {skipped_relocation:5} skipped: the body names an address or "
          f"calls, and object-file references are unrelocated")
    print(f"  slowest single query {slowest:.2f}s")
    for reason, count in sorted(reasons.items(), key=lambda kv: -kv[1])[:4]:
        print(f"      {count:4} unknown: {reason}")
    if args.mutate:
        print(f"\nmust-refuse control: {refused_mutants} mutant(s) refused, "
              f"{len(surviving_mutants)} SURVIVED")
        for addr, index in surviving_mutants[:6]:
            print(f"      {addr:#010x} byte {index} still proved equal")
        if surviving_mutants:
            print("  a surviving mutant means the prover is vacuous here")
            return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
