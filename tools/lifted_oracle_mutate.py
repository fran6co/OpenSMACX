#!/usr/bin/env python3
"""Prove the differential oracle has teeth, by breaking the lifted side on
purpose and checking that the oracle says so.

A differential that cannot fail is worth nothing, and a differential over 5,673
functions is very easy to make unable to fail: skip the hard ones, compare only
the return value, forget the flags. This script is the control. It takes
functions the oracle currently PASSES, applies one deliberate corruption to the
generated C++ of each - a flipped comparison, a dropped store, two registers
swapped, a constant shifted by one - rebuilds only the shard that changed, and
runs the oracle on that one function. Every mutant must come back FAILED.

The mutations are applied to a COPY of the generated tree under build/, never to
anything committed, and the original objects are restored afterwards.

Usage:
    lifted_oracle_mutate.py [--count 8]
"""

from __future__ import annotations

import argparse
import random
import re
import shutil
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
LIFTED = REPO_ROOT / "build" / "lifted"
ORACLE = REPO_ROOT / "build" / "oracle"
REPORT = ORACLE / "report.tsv"
WINE = "/Applications/Wine Staging.app/Contents/Resources/wine/bin/wine"
EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"


def windows_path(path: Path) -> str:
    return "Z:" + str(path).replace("/", "\\")


# Each mutation is (name, regex, replacement). They are chosen to be the
# mistakes a lowerer actually makes, not arbitrary text damage: an inverted
# branch condition, a store that never happens, a register substituted for its
# neighbour, an off-by-one displacement, a sign-extension dropped.
MUTATIONS = [
    ("flip a branch condition",
     re.compile(r"if \(opensmacx_zf\(s\)\) goto"), "if (!opensmacx_zf(s)) goto"),
    ("flip a branch condition",
     re.compile(r"if \(!opensmacx_zf\(s\)\) goto"), "if (opensmacx_zf(s)) goto"),
    ("drop a store",
     re.compile(r"^(\s*)opensmacx_store32\(", re.M), r"\1if (false) opensmacx_store32("),
    ("swap two registers",
     re.compile(r"s\.esi = opensmacx_mem32"), "s.edi = opensmacx_mem32"),
    ("shift a displacement by four",
     re.compile(r"\+ 0x00000008U\)"), "+ 0x0000000cU)"),
    ("shift an immediate by one",
     re.compile(r", 0x00000001U\);"), ", 0x00000002U);"),
    ("turn a signed shift into an unsigned one",
     re.compile(r"opensmacx_sar32"), "opensmacx_shr32"),
    ("drop a flag update",
     re.compile(r"^(\s*)\(void\)opensmacx_sub32\(", re.M), r"\1if (false) (void)opensmacx_sub32("),
    # The x87 file was not compared at all until the register-file comparison
    # landed, so these three had nowhere to show up. They are the three shapes
    # an x87 lowering gets wrong: the wrong operation, a push that never
    # happens (the stack then drifts and every later ST index is off by one),
    # and a store to the wrong slot.
    ("turn an x87 multiply into an add",
     re.compile(r"opensmacx_x87_binary_st0\(OpensmacxX87Mul,"),
     "opensmacx_x87_binary_st0(OpensmacxX87Add,"),
    ("drop an x87 load",
     re.compile(r"^(\s*)opensmacx_x87_fld32\(", re.M),
     r"\1if (false) opensmacx_x87_fld32("),
    ("shift an x87 store by four",
     re.compile(r"opensmacx_x87_fst32\(s\.ebp \+ 0xfffffff4U"),
     "opensmacx_x87_fst32(s.ebp + 0xfffffff0U"),
    # A width mistake: reading a byte where the original reads a word. Invisible
    # unless some byte in the operand's neighbourhood is non-zero, which is what
    # the drawn seeds put there.
    ("widen a byte read to a word",
     re.compile(r"opensmacx_mem8\("), "opensmacx_mem16("),
]


def passing_functions(seed: int) -> list[tuple[int, str]]:
    """Every PASSing function, SHUFFLED.

    Address order was the whole problem. Each mutation kind scanned the
    candidate list from the top and took the first function it matched, so all
    eight mutants landed on four tiny functions and five of the eight on one
    (0x00401000). Three oracle-level defects - deleting the whole-image memory
    comparison, comparing only EAX, excusing ESP - then produced the IDENTICAL
    7-caught/1-missed score as the healthy oracle, because the four functions
    it leaned on touch no memory and return in EAX.

    Shuffling with a fixed seed keeps the run reproducible while spreading the
    mutants over the whole PASS set. The seed is a parameter so a suspicious
    result can be re-drawn rather than argued about.
    """
    rows = []
    for line in REPORT.read_text(encoding="utf-8").splitlines()[1:]:
        parts = line.split("\t")
        if len(parts) >= 2 and parts[1] == "PASS":
            rows.append((int(parts[0], 16), parts[-1]))
    random.Random(seed).shuffle(rows)
    return rows


def shard_of(address: int) -> Path | None:
    symbol = f"void lifted_{address:08x}(OpensmacxStaticRecompileState &s) {{"
    for shard in sorted(LIFTED.glob("lifted_[0-9][0-9][0-9].cpp")):
        if symbol in shard.read_text(encoding="utf-8", errors="replace"):
            return shard
    return None


def body_span(text: str, address: int) -> tuple[int, int] | None:
    start = text.find(f"void lifted_{address:08x}(OpensmacxStaticRecompileState &s) {{")
    if start < 0:
        return None
    end = text.find("\n}\n", start)
    return (start, end) if end > 0 else None


CASES = 16


def run_oracle(address: int) -> str:
    """The oracle's own verdict string for this function, read from the report.

    Scraping the end-of-run summary could not tell the two kinds of
    INCONCLUSIVE apart, and they mean opposite things here. A mutant that makes
    the LIFTED side loop forever has been detected - the oracle is saying the
    lifted body no longer terminates - while a case whose ORIGINAL faulted says
    nothing about the mutant at all. Both printed as `INCONCLUSIVE 1`, and the
    first flipped-branch mutant of the new run was scored MISSED because of it.
    """
    report = ORACLE / "mutate_one.tsv"
    report.unlink(missing_ok=True)
    subprocess.run(
        [WINE, str(ORACLE / "lifted_oracle.exe"),
         "--exe", windows_path(EXE), "--only", f"{address:#010x}",
         "--report", windows_path(report),
         # All sixteen cases, not three. Cases 0..3 are the legacy seeds and
         # 4..15 are the drawn ones; running three of them tested only the old
         # generator, which is the thing being replaced. A mutant that only the
         # drawn seeds reach would have been scored UNREACHED.
         "--cases", str(CASES), "--no-blame"],
        capture_output=True, text=True, timeout=300)
    if not report.exists():
        return "NO-VERDICT"
    rows = [r for r in report.read_text().splitlines() if r.startswith("0x")]
    return rows[0].split("\t")[1] if rows else "NO-VERDICT"


# Verdicts that mean the oracle noticed the lifted side misbehaving. A lifted
# fault, a lifted body that stops terminating, and a lifted body that reaches an
# instruction the lift never lowered are all detections, and all of them were
# previously scored as misses or as nothing.
CAUGHT = {"FAIL", "FAIL-lifted-fault", "INCONCLUSIVE-lifted-budget",
          "INCONCLUSIVE-lifted-out-of-span", "SKIP-trap"}


def rebuild(shard: Path) -> bool:
    compile_ok = subprocess.run(
        ["/opt/homebrew/bin/i686-w64-mingw32-g++", "-std=c++17", "-O2", "-c",
         f"-I{LIFTED}", f"-I{REPO_ROOT / 'tools'}", str(shard),
         "-o", str(shard) + ".o"], capture_output=True, text=True)
    if compile_ok.returncode != 0:
        return False
    link = subprocess.run(["sh", str(REPO_ROOT / "tools" / "lifted_oracle_build.sh")],
                          capture_output=True, text=True)
    return link.returncode == 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--count", type=int, default=8)
    parser.add_argument("--seed", type=int, default=20260728,
                        help="shuffle seed for drawing mutation targets")
    parser.add_argument("--cases", type=int, default=16,
                        help="cases per function. 4 runs ONLY the legacy seeds, "
                             "which makes this an A/B against the old input "
                             "generator on the identical mutants")
    parser.add_argument("--min-size", type=int, default=0,
                        help="only mutate functions whose lowered body has at "
                             "least this many statements; the median passing "
                             "function is 34 bytes, so the default draw is "
                             "dominated by bodies too small to exercise "
                             "memory or the callee-saved registers")
    args = parser.parse_args()
    global CASES
    CASES = args.cases

    if not REPORT.exists():
        print(f"no {REPORT}; run the oracle first", file=sys.stderr)
        return 2

    candidates = passing_functions(args.seed)
    print(f"{len(candidates)} functions currently PASS; mutating up to "
          f"{args.count} (seed {args.seed}, {CASES} cases per function"
          f"{' - LEGACY SEEDS ONLY' if CASES <= 4 else ''})")

    caught = missed = unusable = unreached = 0
    spent: set[int] = set()
    used_shards: dict[Path, bytes] = {}
    attempts = 0
    for name, pattern, replacement in MUTATIONS:
        if attempts >= args.count:
            break
        for address, symbol in candidates:
            if address in spent:
                continue
            shard = shard_of(address)
            if shard is None:
                continue
            text = shard.read_text(encoding="utf-8")
            span = body_span(text, address)
            if span is None:
                continue
            head, body, tail = text[:span[0]], text[span[0]:span[1]], text[span[1]:]
            if body.count(";") < args.min_size:
                continue
            mutated, count = pattern.subn(replacement, body, count=1)
            if count == 0:
                continue

            # THE CANARY, and it is the difference between this script meaning
            # something and meaning nothing.
            #
            # A mutant that comes back PASS has two completely different
            # explanations: the oracle failed to notice a divergence, or the
            # mutated statement never executed under any of the four seeds. The
            # first is a hole in the oracle; the second is a statement about
            # INPUT COVERAGE and says nothing about the comparison at all.
            # Scoring them the same way is what let a 7-of-8 result look like
            # evidence.
            #
            # So the same site is first replaced by a trap. If the oracle
            # reports the trap, the statement is reached and a subsequent PASS
            # is a genuine MISS. If it does not, the mutant is UNREACHED and is
            # not counted either way.
            canary = pattern.sub(
                lambda m: 'opensmacx_trap(0x0CA4A8E0U, "reachability canary"); '
                          + m.group(0),
                body, count=1)

            attempts += 1
            if shard not in used_shards:
                used_shards[shard] = shard.read_bytes()

            shard.write_text(head + canary + tail, encoding="utf-8")
            reached = None
            if rebuild(shard):
                reached = run_oracle(address) in CAUGHT

            shard.write_text(head + mutated + tail, encoding="utf-8")
            built = rebuild(shard)
            verdict = run_oracle(address) if built else "DID-NOT-COMPILE"
            shard.write_bytes(used_shards[shard])
            rebuild(shard)

            if verdict == "DID-NOT-COMPILE":
                unusable += 1
                mark = "unusable"
            elif verdict in CAUGHT:
                caught += 1
                mark = "caught"
            elif reached is False:
                unreached += 1
                mark = "UNREACHED (no seed runs that line)"
            else:
                missed += 1
                mark = "MISSED"
            print(f"  {address:#010x} {name:<42} -> {verdict:<32} {mark}   {symbol}")
            # One function per mutation kind, AND never the same function
            # twice: six of eight mutants landing on ?base_making@@YAHHH@Z is a
            # measurement of that one function, not of the oracle.
            spent.add(address)
            break

    print(f"\nmutants {attempts}: caught {caught}, missed {missed}, "
          f"unreached {unreached}, unusable {unusable}")
    if unreached:
        print(f"  {unreached} mutant(s) sat on a line no seed executes. That is "
              f"a coverage figure,\n  not an oracle figure: four seeds do not "
              f"reach every path of a large function.\n  A PASS from this "
              f"oracle means 'agreed on the paths the seeds reached'.")
    return 0 if missed == 0 and caught > 0 else 1


if __name__ == "__main__":
    sys.exit(main())
