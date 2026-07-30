#!/usr/bin/env python3
"""Say WHY a nearly-full-strength function is not full strength.

The report says a function passed on 15 of 16 seeds. It does not say what
happened on the sixteenth, and that missing sentence is the whole question for
the one-seed-short cohort: 98 functions / 11,615 B are exactly one seed away
from `agreed_full_strength`, led by `?wants_prop@@YAHHHH@Z` at 3,970 B, which
alone is larger than the current full-strength figure.

This replays each function under `--verbose` and reports the verdict of every
case, plus the fault triple (code, EIP, and the address the access NAMED) for
the ones that faulted. It is a DIAGNOSTIC and deliberately not a seed search: a
per-function seed salt would select for inputs the original finds easy, which is
the same shape as converting an honest SKIP into a flattering PASS. The prize is
only 1.75% -> 2.23% of image bytes; the reason is worth more than the bytes.

READ THE VERDICTS THE RIGHT WAY ROUND. `INCONCLUSIVE-original-fault` means the
ORIGINAL machine code faulted - side A, real bytes at 0x00400000. It is not a
lowering bug and it is not a defect in the recovered body. It says the harness
could not get the original to answer, usually because it read a global that no
constructor ever built.

The region column is the diagnosis. `near-null` means a zeroed pointer was
dereferenced, so the global is genuinely unconstructed and Phase 2.1's
`--build-state` would convert it with a reason. `wild` means the pointer held
seed bytes rather than zeroes, and no amount of CRT init fixes that - a
different seeding decision would be needed. Which of those two dominates decides
whether `--build-state` is aimed at this cohort at all.

CROSS-CHECK. The count of judged cases found here is compared against the
report's own `compared` column, and any function where the two disagree is
reported. The replay and the report should not be able to disagree; if they do,
one of them is lying about what ran.
"""

from __future__ import annotations

import argparse
import collections
import concurrent.futures
import csv
import os
import re
import subprocess
import sys
import tempfile
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_REPORT = REPO_ROOT / "build" / "oracle" / "report-base.tsv"
DEFAULT_OUT = REPO_ROOT / "build" / "oracle" / "one-short.tsv"
# Its own copy on purpose: the sweep's watchdog reaps with
# `pkill -f 'build.oracle.lifted_oracle.exe'`, so a probe running from the
# canonical path is killed the first time a concurrent sweep stalls.
DEFAULT_ORACLE = REPO_ROOT / "build" / "oracle" / "diag" / "lifted_oracle.exe"
FUNCTIONS_CSV = REPO_ROOT / "docs" / "recovery" / "functions.csv"
RUNNER = REPO_ROOT / "tools" / "lifted_oracle_run.sh"

# tools/lifted_loader.h:37-39. The flat array is one contiguous span and
# opensmacx_at() is a single subtraction with no bounds check, so which region a
# faulting address lands in is the entire diagnosis.
IMAGE_BEGIN = 0x00400000
IMAGE_END = 0x00A0C000
GUARD_END = 0x00A0D000
STACK_END = 0x00B0D000
# The first 64 KiB is never mapped on Windows, so anything below it came from
# dereferencing something at or near zero.
NULL_BAND = 0x00010000
TOP_PAGE = 0xFFFF0000

CASE_LINE = re.compile(r"^stage: case (0x[0-9a-fA-F]+)/(\d+)\s*$")
# `%#010x` of ZERO prints `0000000000`, with no `0x`, because C only adds the
# prefix for a non-zero value. Requiring `0x` therefore fails to match exactly
# the most interesting line there is - a dereference of literal address zero -
# and silently dropped it until the bare form was found in a real run. Accept
# both spellings; int(s, 16) reads either.
WORD = r"(?:0x)?[0-9a-fA-F]+"
VERDICT_LINE = re.compile(
    r"^stage:\s+-> (\S+)"
    rf"(?:\s+code=({WORD}) eip=({WORD}) data=({WORD}))?"
    r"\s*$")
# Anything the oracle prints as a verdict must parse. A line that looks like one
# and does not is the failure above, and it has to be loud rather than dropped.
LOOSE_VERDICT = re.compile(r"^stage:\s+-> ")

FIELDS = ("address", "name", "size", "seeds_short", "case", "verdict",
          "fault_code", "fault_eip", "fault_data", "region")


def region_of(address: int) -> str:
    if address < NULL_BAND:
        return "near-null"
    if IMAGE_BEGIN <= address < IMAGE_END:
        return "image"
    if IMAGE_END <= address < GUARD_END:
        return "stack-guard"
    if GUARD_END <= address < STACK_END:
        return "stack"
    if address >= TOP_PAGE:
        return "top-page"
    return "wild"


def parse_cases(text: str) -> list[dict]:
    """Pair each `stage: case` line with the verdict line that follows it.

    Pairing rather than matching one combined pattern, because a case whose
    original never returns prints its header and no verdict at all. Dropping
    the unpaired header silently would turn a hang into a case that was never
    attempted.
    """
    cases: list[dict] = []
    pending: int | None = None
    for line in text.splitlines():
        header = CASE_LINE.match(line)
        if header:
            pending = int(header.group(2))
            continue
        verdict = VERDICT_LINE.match(line)
        if verdict and pending is not None:
            code, eip, data = verdict.group(2), verdict.group(3), verdict.group(4)
            cases.append({
                "case": pending,
                "verdict": verdict.group(1),
                "fault_code": int(code, 16) if code else 0,
                "fault_eip": int(eip, 16) if eip else 0,
                "fault_data": int(data, 16) if data else 0,
            })
            pending = None
    return cases


def unparsed_verdict_lines(text: str) -> list[str]:
    """Verdict lines the strict pattern could not read.

    Exists because the alternative is a silent drop, and the first thing that
    got dropped was a null dereference - the single most diagnostic line in the
    whole cohort. A dropped case looks exactly like a case that never happened.
    """
    return [line for line in text.splitlines()
            if LOOSE_VERDICT.match(line) and not VERDICT_LINE.match(line)]


def load_report(path: Path) -> list[dict]:
    with path.open(newline="", encoding="utf-8") as handle:
        return list(csv.DictReader(handle, delimiter="\t"))


def load_sizes(path: Path = FUNCTIONS_CSV) -> dict:
    if not path.is_file():
        return {}
    with path.open(newline="", encoding="utf-8-sig") as handle:
        return {int(row["address"], 16): (row.get("name", ""),
                                          int(row.get("size") or 0))
                for row in csv.DictReader(handle)}


def cohort(rows: list[dict], short: int, sizes: dict) -> list[dict]:
    """Functions that passed on every judged seed but were short exactly N.

    Only PASS rows qualify: a function that FAILED is not one seed from full
    strength, it is wrong, and mixing the two would put a defect in a cohort
    labelled "nearly proven".
    """
    found = []
    for row in rows:
        try:
            cases, compared = int(row["cases"]), int(row["compared"])
        except (KeyError, ValueError):
            continue
        if row.get("verdict") != "PASS" or not cases or not compared:
            continue
        if cases - compared != short:
            continue
        address = int(row["address"], 16)
        name, size = sizes.get(address, (row.get("name", ""), 0))
        found.append({"address": address, "name": name or row.get("name", ""),
                      "size": size, "cases": cases, "compared": compared,
                      "seeds_short": short})
    found.sort(key=lambda entry: -entry["size"])
    return found


def replay(entry: dict, oracle: Path, cases: int, timeout: float,
           runner: Path = RUNNER) -> dict:
    """Re-run one function under --verbose and collect its per-case verdicts."""
    environment = dict(os.environ)
    environment["ORACLE"] = str(oracle)
    with tempfile.TemporaryDirectory() as directory:
        # Its own report path per job: the driver truncates whatever it is
        # given, so a shared one would have concurrent jobs overwrite each
        # other and, with the default, would clobber the canonical report.
        environment["REPORT"] = str(Path(directory) / "probe.tsv")
        command = [str(runner), "--only", f"{entry['address']:#010x}",
                   "--cases", str(cases), "--verbose"]
        try:
            done = subprocess.run(command, capture_output=True, text=True,
                                  timeout=timeout, env=environment)
            text = done.stdout + done.stderr
            timed_out = False
        except subprocess.TimeoutExpired as expired:
            text = "".join(
                stream.decode("utf-8", "replace") if isinstance(stream, bytes)
                else (stream or "")
                for stream in (expired.stdout, expired.stderr))
            timed_out = True
    result = dict(entry)
    result["cases_seen"] = parse_cases(text)
    result["unparsed"] = unparsed_verdict_lines(text)
    result["timed_out"] = timed_out
    return result


def summarise(results: list[dict]) -> dict:
    """Distributions, plus the report-versus-replay disagreements."""
    verdicts: collections.Counter = collections.Counter()
    regions: collections.Counter = collections.Counter()
    blocked_bytes: collections.Counter = collections.Counter()
    disagreed = []
    unexplained = []
    for result in results:
        judged = sum(1 for case in result["cases_seen"]
                     if case["verdict"] == "PASS")
        if judged != result["compared"]:
            disagreed.append((result["address"], result["compared"], judged))
        # A function that is N seeds short must produce N reasons. Fewer means
        # a case went unrecorded, and an unrecorded case is indistinguishable
        # from one that never happened - which is how the null-dereference
        # lines went missing the first time this ran.
        reasons = len(result["cases_seen"]) - judged
        if reasons != result["seeds_short"]:
            unexplained.append((result["address"], result["seeds_short"],
                                reasons))
        for case in result["cases_seen"]:
            if case["verdict"] == "PASS":
                continue
            verdicts[case["verdict"]] += 1
            if case["fault_code"]:
                region = region_of(case["fault_data"])
                regions[region] += 1
                blocked_bytes[region] += result["size"]
    return {"verdicts": verdicts, "regions": regions,
            "blocked_bytes": blocked_bytes, "disagreed": disagreed,
            "unexplained": unexplained}


def write_rows(path: Path, results: list[dict]) -> int:
    path.parent.mkdir(parents=True, exist_ok=True)
    written = 0
    with path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.writer(handle, delimiter="\t", lineterminator="\n")
        writer.writerow(FIELDS)
        for result in results:
            for case in result["cases_seen"]:
                if case["verdict"] == "PASS":
                    continue
                writer.writerow([
                    f"{result['address']:#010x}", result["name"],
                    result["size"], result["seeds_short"], case["case"],
                    case["verdict"], f"{case['fault_code']:#010x}",
                    f"{case['fault_eip']:#010x}", f"{case['fault_data']:#010x}",
                    region_of(case["fault_data"]) if case["fault_code"] else ""])
                written += 1
    return written


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--report", type=Path, default=DEFAULT_REPORT)
    parser.add_argument("--out", type=Path, default=DEFAULT_OUT)
    parser.add_argument("--oracle", type=Path, default=DEFAULT_ORACLE)
    parser.add_argument("--short", type=int, default=1,
                        help="how many seeds short the cohort is (default 1)")
    parser.add_argument("--cases", type=int, default=16)
    parser.add_argument("--jobs", type=int, default=4)
    parser.add_argument("--timeout", type=float, default=300.0)
    parser.add_argument("--limit", type=int,
                        help="replay only the largest N, for a quick look")
    args = parser.parse_args(argv)

    if not args.report.is_file():
        print(f"error: {args.report} is missing; sweep one first",
              file=sys.stderr)
        return 2
    if not args.oracle.is_file():
        print(f"error: {args.oracle} is missing; copy "
              f"build/oracle/lifted_oracle.exe there so this does not share a "
              f"path with a sweep's watchdog", file=sys.stderr)
        return 2

    selected = cohort(load_report(args.report), args.short, load_sizes())
    if args.limit:
        selected = selected[:args.limit]
    if not selected:
        print(f"no function in {args.report} is exactly {args.short} seed(s) "
              f"short; nothing to diagnose")
        return 0
    total = sum(entry["size"] for entry in selected)
    print(f"cohort: {len(selected)} fn / {total} B exactly {args.short} "
          f"seed(s) short, from {args.report}")

    results = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
        futures = {pool.submit(replay, entry, args.oracle, args.cases,
                               args.timeout): entry for entry in selected}
        for done in concurrent.futures.as_completed(futures):
            results.append(done.result())
            print(f"  replayed {len(results)}/{len(selected)}", end="\r",
                  flush=True)
    print()

    written = write_rows(args.out, results)
    found = summarise(results)
    print(f"wrote {written} non-comparable case(s) -> {args.out}")

    print("\nverdict on the cases that could not be judged:")
    for verdict, count in found["verdicts"].most_common():
        print(f"  {count:5}  {verdict}")
    if found["regions"]:
        print("\nwhere the faulting access pointed (side A, the ORIGINAL):")
        for region, count in found["regions"].most_common():
            print(f"  {count:5}  {region:12} "
                  f"{found['blocked_bytes'][region]:8} B of cohort")
        print("\n  near-null means a zeroed pointer, which is what "
              "--build-state would construct.")
        print("  wild means the pointer held seed bytes; CRT init would not "
              "change it.")
    stray = [(result["address"], line) for result in results
             for line in result["unparsed"]]
    if stray:
        print(f"\nWARNING: {len(stray)} verdict line(s) this tool could not "
              f"read, so their cases are MISSING from everything above:")
        for address, line in stray[:10]:
            print(f"  {address:#010x}  {line.strip()}")
    if found["unexplained"]:
        # Every function here is N seeds short, so it owes N reasons. Owing
        # fewer is not a smaller answer, it is an unrecorded case.
        print(f"\nWARNING: {len(found['unexplained'])} function(s) produced "
              f"fewer reasons than they are seeds short:")
        for address, short, reasons in found["unexplained"][:20]:
            print(f"  {address:#010x} is {short} short but recorded "
                  f"{reasons} reason(s)")

    hung = [result for result in results if result["timed_out"]]
    if hung:
        print(f"\n{len(hung)} function(s) timed out on replay and their cases "
              f"are incomplete:")
        for result in hung:
            print(f"  {result['address']:#010x} {result['name']}")
    if found["disagreed"]:
        # The report and a replay of the same binary should not be able to
        # disagree about how many seeds were judged. If they do, one of them is
        # describing a run that did not happen, and that outranks the cohort.
        print(f"\nWARNING: {len(found['disagreed'])} function(s) where the "
              f"report's `compared` and this replay disagree:")
        for address, reported, seen in found["disagreed"][:20]:
            print(f"  {address:#010x} report says {reported}, replay judged "
                  f"{seen}")
    else:
        print(f"\nreport and replay agree on judged-seed counts for all "
              f"{len(results)} function(s)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
