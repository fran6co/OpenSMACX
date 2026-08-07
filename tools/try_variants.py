#!/usr/bin/env python3
"""Score many candidate spellings of one function, ranked, in one command.

Matching decompilation is a search over SOURCE FORM. The semantics get decided
early; the rest of the work is finding which of several equivalent spellings
VC6 lowers the way the original was lowered - ternary against `if`, condition
polarity, a temp that changes an operand's addressing mode, a loop counting the
other way. There is no reasoning all the way to the answer. The compiler has to
be asked, and the useful question is "which of these nine" rather than "is this
one it".

    tools/try_variants.py 0x006281B0 --dir /tmp/variants

Every `*.cpp` in the directory is one candidate body. All of them are scored
against the same loaded image and catalogue and ranked best-first. Measured on
four candidates: 14.5 s here against 20.9 s for the same four through
`verify_recovered_function.py`, so about a third off - the saving is the image
load, not the compiles, and it does not grow much with more candidates.

SPEED IS NOT THE POINT, though. The point is that "which of these nine" is one
question with one answer, and an agent that can ask it stops guessing one
spelling at a time.

THIS EXISTS BECAUSE AN AGENT WROTE IT FOR ITSELF. Recovering 0x006281B0 took
nine structural variants to find that binding a component to a local flips
VC6's fmul scheduling, and the agent built its own batch harness mid-task to
get there - spending budget on tooling instead of on the problem. The next one
should not have to, and an agent that can enumerate cheaply stops guessing.
"""

from __future__ import annotations

import argparse
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import byte_match  # noqa: E402
import emit_translation_unit as emit  # noqa: E402
import mizuchi_writeback as writeback  # noqa: E402
import verify_recovered_function as verifier  # noqa: E402


def score_all(address: int, bodies: dict) -> list:
    """[(name, verdict)] for every candidate, best first.

    Each verdict comes from `byte_match.match_function` on a unit assembled the
    way `mizuchi_writeback.verify` assembles it, so a tier here means exactly
    what the integrator's gate means. The only thing shared across candidates
    is the loaded image and catalogue, which is the expensive part.
    """
    import pefile

    functions = emit.load_functions()
    callees = emit.load_callees()
    derived = emit.load_derived()
    scaffolding_pe = pefile.PE(str(byte_match.DEFAULT_EXE), fast_load=True)

    pe = pefile.PE(str(byte_match.DEFAULT_EXE))
    catalogue = byte_match.load_rows()
    shared = byte_match.shared_span_index(catalogue)

    results = {}
    with tempfile.TemporaryDirectory() as directory:
        work = Path(directory)
        for name, body in sorted(bodies.items()):
            # The form check runs FIRST and refuses before compiling. A
            # byte-exact `__asm` answer would only be harder to argue with.
            blocked, _ = verifier.form_report(body)
            if blocked:
                results[name] = {"tier": "REFUSED", "refusal_reason": blocked[0]}
                continue
            try:
                unit = writeback.build_unit(address, body, functions, callees,
                                            derived, scaffolding_pe)
            except emit.Unsettled as error:
                results[name] = {"tier": "REFUSED",
                                 "refusal_reason": f"no scaffolding: {error}"}
                continue
            results[name] = byte_match.match_function(
                pe, catalogue, shared, address, unit, work, "", f"v_{name}")

    order = {tier: index for index, tier in enumerate(byte_match.TIER_ORDER)}
    return sorted(results.items(),
                  key=lambda item: (order.get(item[1].get("tier"), 99),
                                    -(item[1].get("mnemonic_similarity") or 0),
                                    item[0]))


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("address")
    parser.add_argument("--dir", type=Path, required=True,
                        help="a directory of *.cpp candidate bodies")
    arguments = parser.parse_args(argv)

    try:
        address = int(arguments.address, 16)
    except ValueError:
        print(f"error: {arguments.address} is not a hex address",
              file=sys.stderr)
        return 2

    bodies = {p.stem: p.read_text()
              for p in sorted(arguments.dir.glob("*.cpp"))}
    if not bodies:
        print(f"error: no *.cpp found in {arguments.dir}", file=sys.stderr)
        return 2

    results = score_all(address, bodies)
    print(f"{len(bodies)} candidate(s) for 0x{address:08X}, best first:\n")
    for name, verdict in results:
        detail = (verdict.get("note") or verdict.get("refusal_reason") or "")
        print(f"   {verdict.get('tier', '?'):14} {name:26} {detail[:54]}")
    winner = results[0][1].get("tier") if results else None
    if winner == "BYTE_EXACT":
        print(f"\n{results[0][0]} is BYTE_EXACT.")
        return 0
    print("\nnone reached BYTE_EXACT; the ranking shows which spelling is "
          "closest.")
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
