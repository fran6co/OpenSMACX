#!/usr/bin/env python3
"""Ask of every recovered function: does ANYTHING observe it?

WHY THIS IS NOT JUST A MUTATION SWEEP. A full sweep answers "which parts of this
body are unobserved" and costs one build-and-test per mutant - measured at 0.83 s
each, with a mean of 152 mutants for a function of 200 B or more. Over the 224
such functions that is about eight hours, which is why nobody had run it, and why
nobody knew that five of a random eight kill NOTHING.

The weaker question is far cheaper and is the one that matters first. To learn
whether a function is observed at all, wreck it as thoroughly as it can be
wrecked and still compile, then run the suite ONCE. If the tests do not notice
every constant in the body changing simultaneously, no assertion depends on this
function, and no amount of per-mutant detail changes that verdict.

  full sweep   ~152 builds per function
  this         1 build per function

WHAT "WRECKED" MEANS, precisely. Every line the mutation operators can perturb is
perturbed at once - one replacement per line, the first the operators offer,
which for a constant is the zero variant. Statement drops are excluded because
they change the line count and cannot be composed line-wise. So a surviving
wreck means every constant in the body went to zero and every comparison that
could invert did, and the suite still passed.

WHAT A VERDICT MEANS, AND DOES NOT:

  OBSERVED    at least one assertion depends on this body. It does NOT mean the
              body is well tested - a full sweep is what separates 94% from 8%.
  UNOBSERVED  nothing in the suite compiling this file can tell the body from a
              wrecked one. Not a subtle gap: absence.
  NO-COMPILE  the maximal mutant did not build, so nothing was learned. Never
              folded into either verdict.

A function can be UNOBSERVED here and still be verified elsewhere - by a
generated signature oracle, a runtime oracle, or proven.csv. Those are reported
alongside so the two are never confused.
"""
from __future__ import annotations

import argparse
import csv
import json
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import mutate_and_verify as mutation  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
FUNCTIONS = REPO_ROOT / "docs" / "recovery" / "functions.csv"
ORACLE = REPO_ROOT / "src" / "generated_signature_oracle.cpp"
PROVEN = REPO_ROOT / "docs" / "recovery" / "proven.csv"
TESTS = REPO_ROOT / "tests"

OBSERVED, UNOBSERVED, NO_COMPILE, STALE = (
    "OBSERVED", "UNOBSERVED", "NO-COMPILE", "STALE")


def maximal_mutant(lines, function):
    """One line-wise replacement per perturbable line, applied together.

    Drops are skipped: they shorten the file, so composing them line-wise would
    misalign every later replacement.
    """
    mutants = mutation.build_mutants(lines, function)
    by_line = {}
    for mutant in mutants:
        if len(mutant.lines) != len(lines):
            continue                      # a drop, not a replacement
        index = mutant.line_number - 1
        if index in by_line or lines[index] == mutant.lines[index]:
            continue
        by_line[index] = mutant.lines[index]
    return by_line


def wreck_candidates(lines, by_line):
    """Progressively smaller wrecks, largest first.

    The maximal mutant does not always compile - measured at 2 of the first 5
    functions tried, both of them large - and a NO-COMPILE measures nothing at
    all. Perturbing every line of a 10 kB body at once produces type errors
    that a single-line mutant never would: a zeroed loop bound feeding an
    inverted comparison, a zeroed size passed where a positive one is required.

    So halve until it builds. A smaller wreck is weaker evidence for UNOBSERVED
    and exactly as strong for OBSERVED, which is why lines_wrecked is reported
    with every verdict rather than hidden.
    """
    order = sorted(by_line)
    while order:
        wrecked = list(lines)
        for index in order:
            wrecked[index] = by_line[index]
        yield wrecked, len(order)
        if len(order) == 1:
            return
        order = order[:len(order) // 2]


def other_verification(row, tests_text, oracle_text, proven):
    stem = re.match(r"\?(\??[\w@]+?)@@", row["name"])
    stem = stem.group(1) if stem else row["name"]
    where = []
    if stem and re.search(rf"\b{re.escape(stem)}\b", tests_text):
        where.append("named-in-tests")
    if stem and stem in oracle_text:
        where.append("generated-oracle")
    if row["address"].lower() in proven:
        where.append("proven.csv")
    return where


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--build-dir", default="build/mingw-i686-debug")
    parser.add_argument("--min-size", type=int, default=200)
    parser.add_argument("--limit", type=int)
    parser.add_argument("--timeout", type=float, default=600.0)
    parser.add_argument("--report", type=Path)
    parser.add_argument("--sources", nargs="*")
    parser.add_argument("--target",
                        help="test executable to build and run, for sources "
                             "compiled into more than one; without it those "
                             "functions cannot be measured at all")
    arguments = parser.parse_args()

    rows = {r["address"].lower(): r for r in
            csv.DictReader(FUNCTIONS.open(newline="", encoding="utf-8-sig"))}
    tests_text = "".join(p.read_text(errors="replace")
                         for p in TESTS.rglob("*.cpp"))
    oracle_text = ORACLE.read_text(errors="replace") if ORACLE.is_file() else ""
    proven = set()
    if PROVEN.is_file():
        proven = {r["address"].lower() for r in csv.DictReader(PROVEN.open())}

    sources = ([Path(s) for s in arguments.sources] if arguments.sources
               else sorted(Path("src").glob("*.cpp")))
    work = []
    for path in sources:
        lines = path.read_text(encoding="utf-8").splitlines()
        for function in mutation.parse_functions(lines):
            key = "0x" + function.address.lower().removeprefix("0x").zfill(8)
            row = rows.get(key)
            if not row or row["recovery_state"] != "source_complete":
                continue
            if int(row["size"]) < arguments.min_size:
                continue
            work.append((path, function, row))
    work.sort(key=lambda item: -int(item[2]["size"]))
    if arguments.limit:
        work = work[:arguments.limit]
    print(f"observability: {len(work)} recovered function(s) of "
          f">={arguments.min_size} B", flush=True)

    results = []
    for index, (path, function, row) in enumerate(work, 1):
        original = path.read_text(encoding="utf-8")
        lines = original.splitlines()
        by_line = maximal_mutant(lines, function)
        if not by_line:
            continue
        # resolve_target returns an ERROR MESSAGE on failure and None on
        # success, deriving the target into the namespace it is handed. Reading
        # its return value as the target skips every function while printing a
        # correct-looking derivation line for each - which is exactly how this
        # first ran, reporting six functions and measuring none.
        resolution = argparse.Namespace(
            build_dir=arguments.build_dir, target=None, test=None,
            source=str(path))
        if arguments.target:
            # 25 recovered functions live in sources compiled into three test
            # executables each - buffer.cpp reaches gameplay, leaf AND oracle -
            # so resolve_target correctly refuses to guess and they were simply
            # unmeasurable. A function is observed if ANY suite observes it, so
            # the caller names one and the runs are unioned.
            resolution.target = resolution.test = arguments.target
            problem = None
        else:
            problem = mutation.resolve_target(resolution)
        if problem or not resolution.target:
            print(f"  skipped {row['address']}: {problem or 'no target'}",
                  flush=True)
            continue
        harness = mutation.Harness(argparse.Namespace(
            build_dir=arguments.build_dir, target=resolution.target,
            test=resolution.test or resolution.target,
            timeout=arguments.timeout, artifact=None))
        verdict, changed = NO_COMPILE, 0
        try:
            for candidate, count in wreck_candidates(lines, by_line):
                path.write_text("\n".join(candidate) + "\n", encoding="utf-8")
                status = harness.build()
                if status == mutation.FAILED:
                    continue                      # too aggressive; halve it
                changed = count
                if status == mutation.STALE:
                    verdict = STALE
                else:
                    verdict = (UNOBSERVED if harness.check() == mutation.PASSED
                               else OBSERVED)
                break
        finally:
            path.write_text(original, encoding="utf-8")
        elsewhere = other_verification(row, tests_text, oracle_text, proven)
        results.append({"address": row["address"], "name": row["name"],
                        "size": int(row["size"]), "source": str(path),
                        "lines_wrecked": changed, "verdict": verdict,
                        "other_verification": elsewhere})
        print(f"[{index}/{len(work)}] {row['address']} {row['name'][:42]:42} "
              f"{int(row['size']):6} B  {verdict}"
              + (f"  ({', '.join(elsewhere)})" if elsewhere else ""),
              flush=True)

    counts = {}
    for item in results:
        counts[item["verdict"]] = counts.get(item["verdict"], 0) + 1
    measured = counts.get(OBSERVED, 0) + counts.get(UNOBSERVED, 0)
    print("\nobservability:")
    for verdict in (OBSERVED, UNOBSERVED, NO_COMPILE, STALE):
        if counts.get(verdict):
            print(f"  {verdict:12} {counts[verdict]}")
    if measured:
        unobserved = counts.get(UNOBSERVED, 0)
        bytes_unobserved = sum(i["size"] for i in results
                               if i["verdict"] == UNOBSERVED)
        print(f"  {unobserved}/{measured} measured functions are observed by "
              f"nothing ({100 * unobserved / measured:.0f}%), "
              f"{bytes_unobserved} B")
    if arguments.report:
        arguments.report.write_text(
            json.dumps(results, indent=2, sort_keys=True) + "\n",
            encoding="utf-8")
        print(f"  wrote {arguments.report}")
    if not measured:
        print("observability: nothing was measured, so this run proves NOTHING",
              file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
