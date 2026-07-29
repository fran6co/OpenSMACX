#!/usr/bin/env python3
"""Summarise build/oracle/report.tsv over the WHOLE image, by BYTES.

The oracle's own end-of-run summary covers only the last segment of a sweep:
every tally resets when lifted_oracle_sweep.sh restarts the process after a
hang, so the printed figures describe a suffix of the plan, not the plan. The
report is the only whole-image record, so the whole-image numbers come from
here.

Four things this reports that the oracle's summary cannot:

  * BYTE WEIGHTING. This project's own measure - "40% of functions is 8% of
    bytes". A count of functions flatters any coverage claim, because the
    passing functions are small: the median is tens of bytes. Bytes lead every
    line and every column; the function count is the qualifier beside them.
  * A FIXED DENOMINATOR. Percentages are over the lift scope taken from the
    committed catalogue (`recovery_metrics.lift_scope`), NOT over whatever rows
    the report happens to contain. Those were the same number only because the
    sweep had finished; a sweep killed half way through used to print
    percentages of the half it managed, and they read exactly like percentages
    of the image. `swept` below is what makes a partial run visible.
  * HOW MANY SEEDS ACTUALLY COMPARED, attached to the number rather than
    footnoted under it. A PASS may rest on one seed of sixteen, with the other
    fifteen unable to run the original at all. Cases attempted and cases
    compared are different numbers and only the second is evidence, so the
    agreeing bytes are partitioned into evidence tiers and the full-strength
    tier is printed on the headline itself.
  * THE NUMBERS THAT MUST GO DOWN. A dashboard where every number goes up
    cannot report a regression. Four are published: bytes still carried by
    machine-derived code, bytes the oracle has never compared at all, bytes
    that agreed only on the paths sixteen seeds happened to take, and bytes
    whose agreement was obtained under a weakened condition. All four appear in
    the `--json` record whether or not they are zero, because a record is
    diffed against another record.
"""

from __future__ import annotations

import argparse
import json
import sys
from collections import Counter
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import recovery_metrics as metrics
from recovery_metrics import Tally

REPO_ROOT = Path(__file__).resolve().parent.parent
REPORT = REPO_ROOT / "build" / "oracle" / "report.tsv"
FUNCTIONS = metrics.FUNCTIONS_CSV

# Verdicts in which the LIFTED body demonstrably ran. A HANG is the supervisor
# giving up and says nothing about what ran; a SKIP never started; an
# INCONCLUSIVE-original-* is the ORIGINAL side failing to run.
# SKIP-reached-blocked is here and the other SKIPs are not: the lifted body ran
# until it walked into a construct the lift cannot model. That is the whole
# difference between it and a static refusal.
EXECUTED_VERDICTS = ("PASS", "PASS-paths-taken", "FAIL", "FAIL-lifted-fault",
                     "SKIP-reached-blocked")
EXECUTED_PREFIX = "INCONCLUSIVE-lifted"

# Verdicts in which BOTH sides ran on at least one seed, i.e. something was
# actually compared. This mirrors `OracleFoldState::compared` in
# lifted_oracle_fold.h, which counts exactly OraclePass and OracleFail - and
# PASS-paths-taken is an OraclePass fold that oracle_qualify_verdict renamed
# because the function's static flags say a path exists that no seed took.
#
# SKIP-reached-blocked is deliberately NOT here even though such a row can
# carry compared > 0 (0x004015b0 compared six seeds before case 9 reached
# 0x00644f3a). Leaving it out can only make `never compared` larger, and that
# is a number published under "MUST GO DOWN".
COMPARED_VERDICTS = ("PASS", "PASS-paths-taken", "FAIL")

# Agreement obtained on the paths sixteen seeds happened to take, on a function
# whose static flags say another path reaches something the lift cannot run.
# Real evidence, weaker than a PASS, and never added to `agreed` - see
# oracle_qualify_verdict in lifted_oracle_fold.h. Published on its own line
# rather than folded anywhere, because folding it in either direction is a lie:
# into `agreed` it inflates coverage, out of the report entirely it hides 55
# functions and 19,876 bytes of measured evidence.
PATHS_TAKEN_VERDICT = "PASS-paths-taken"

# The evidence tiers an agreeing function falls into, worst first. Each PASS is
# assigned to the FIRST tier it matches, so the tiers partition the agreeing
# bytes and their totals add up to the headline. `full-strength` is the only
# tier that is not a qualification.
TIER_FULL = "full-strength"
TIER_SINGLE_SEED = "single-seed"
TIER_ARBITRATED = "arbitrated"
TIER_SEEDS_INCOMPLETE = "seeds-incomplete"
TIER_ORDER = (TIER_SINGLE_SEED, TIER_ARBITRATED, TIER_SEEDS_INCOMPLETE,
              TIER_FULL)
TIER_MEANING = {
    TIER_SINGLE_SEED: "one seed of the attempt could be judged; the original "
                      "side could not be run on any other",
    TIER_ARBITRATED: "cases were thrown out - the original read the "
                     "unmodellable top 64 KiB",
    TIER_SEEDS_INCOMPLETE: "some attempted seeds were never compared",
    TIER_FULL: "every attempted seed compared, nothing thrown out",
}


def evidence_tier(cases: int, compared: int, detail: str) -> str:
    """Which tier one PASS row belongs to.

    Order is severity, not convenience: a row that is both single-seed and
    arbitrated is reported as single-seed, because that is the weaker claim.

    A MISSING detail column cannot promote a row into `full-strength`, and that
    is a property of the fold rather than of care taken here. `arbitrated`
    counts OracleInconclusiveTopPage cases, which increment `ran` and never
    increment `compared`; so any arbitration forces compared < cases, and
    `compared == cases` proves nothing was thrown out without reading the
    caveat at all. Detail is needed only to tell one qualified tier from
    another, never to tell qualified from full.
    """
    if compared <= 1 and cases > 1:
        return TIER_SINGLE_SEED
    if detail.strip():
        # The only detail an oracle PASS row ever carries is the arbitration
        # caveat from `oracle_pass_caveat`; see lifted_oracle_fold.h.
        return TIER_ARBITRATED
    if compared < cases:
        return TIER_SEEDS_INCOMPLETE
    return TIER_FULL


class Row:
    __slots__ = ("address", "verdict", "cases", "compared", "detail", "name",
                 "size", "priced")

    def __init__(self, address, verdict, cases, compared, detail, name, size,
                 priced):
        self.address = address
        self.verdict = verdict
        self.cases = cases
        self.compared = compared
        self.detail = detail
        self.name = name
        self.size = size
        self.priced = priced


def read_report(path: Path, sizes: dict[int, int]):
    """Parse the report, priced from the catalogue, first verdict per address.

    A resumed sweep re-emits the boundary row it was killed on, sometimes with
    a different verdict; the first one wins and the repeat is dropped whole, so
    it can neither change the verdict nor add its bytes twice.

    `priced` records whether the catalogue could price the row at all. A row
    the lift scope does not contain is still shown in the verdict table - the
    table has to agree with the report it summarises - but it is not counted as
    swept, because the denominator it would be swept out of does not contain
    it either.
    """
    rows: list[Row] = []
    seen: set[int] = set()
    # Opened in text mode ON PURPOSE, and that is the whole CRLF story. The
    # report is written under Wine, so every line ends "\r\n"; universal
    # newlines translates them on read. There used to be an explicit
    # `.rstrip("\r")` here as well, and the pair masked each other - damaging
    # either one left the other to cover it, so no test could tell whether
    # carriage returns were being handled at all. One mechanism, one test.
    with path.open() as handle:
        header = handle.readline().rstrip("\n").split("\t")
        has_compared = "compared" in header
        dropped = 0
        for line in handle:
            parts = line.rstrip("\n").split("\t")
            if len(parts) < 4 or not parts[0].startswith("0x"):
                # SAY SO. A harness that dies mid-case can write rows whose
                # address is "0000000000" and whose name is "(null)", and this
                # test silently drops them - correctly, since they name no
                # function. But dropping 3,928 of 5,673 rows in silence made a
                # broken run print INCONCLUSIVE-original-fault at 15.85% of
                # scope, which reads like the headline figure collapsing when
                # it is the report collapsing. `never compared` at 95% gave it
                # away; a reader who checked only the one line would not have
                # looked. Count them and print it.
                if line.strip():
                    dropped += 1
                continue
            address = int(parts[0], 16)
            if address in seen:
                continue
            seen.add(address)
            detail_column = 4 if has_compared else 3
            rows.append(Row(
                address=address,
                verdict=parts[1],
                cases=int(parts[2]) if parts[2].isdigit() else 0,
                compared=(int(parts[3])
                          if has_compared and parts[3].isdigit() else 0),
                detail=(parts[detail_column]
                        if len(parts) > detail_column else ""),
                name=parts[-1],
                size=sizes.get(address, 0),
                priced=address in sizes))
    if dropped:
        print(f"WARNING: {dropped} report line(s) named no function and were "
              f"dropped. A figure computed from the rest describes only "
              f"{len(rows)} of {dropped + len(rows)} rows - suspect the RUN, "
              f"not the lift.", file=sys.stderr)
    return rows, has_compared


def tally_rows(rows, predicate) -> Tally:
    """Sum a subset of the report - but only rows the lift scope contains.

    Every published figure is a share of the scope, so a row the scope does not
    hold cannot be a share of it. Counting one anyway is not merely untidy: it
    made "never compared" read -1 functions on a fixture where the report and
    the catalogue disagreed, and a negative count is the shape of a figure that
    could just as easily have been quietly too large.
    """
    total = Tally()
    for row in rows:
        if row.priced and predicate(row):
            total = total + Tally(1, row.size)
    return total


def executed(row: Row) -> bool:
    return (row.verdict in EXECUTED_VERDICTS
            or row.verdict.startswith(EXECUTED_PREFIX))


def line(label: str, value: Tally, scope: Tally) -> str:
    """One published figure. Bytes first, percent second, functions last."""
    return (f"{label:36s} {value.byte_count:9d} B  "
            f"{value.percent_of(scope):6.2f}% of scope  "
            f"{value.functions:5d} fn")


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("report", nargs="?", default=None)
    parser.add_argument("--functions", type=Path, default=None,
                        help="catalogue to price the report from")
    parser.add_argument("--json", type=Path, default=None,
                        help="also write the byte-weighted record here")
    args = parser.parse_args(argv)

    path = Path(args.report) if args.report else REPORT
    if not path.exists():
        print(f"no {path}", file=sys.stderr)
        return 2

    catalogue = metrics.load_catalogue(args.functions or FUNCTIONS)
    sizes = metrics.scope_sizes(catalogue)
    scope = metrics.lift_scope(catalogue)
    rows, has_compared = read_report(path, sizes)

    swept = tally_rows(rows, lambda row: True)
    unpriced = Tally(sum(1 for row in rows if not row.priced), 0)
    # The verdict table, unlike every figure below it, counts EVERY report row.
    # It has to agree with the report it summarises; a row the scope does not
    # price still appears, at zero bytes.
    by_verdict: dict[str, Tally] = {}
    for row in rows:
        by_verdict[row.verdict] = by_verdict.get(row.verdict,
                                                 Tally()) + Tally(1, row.size)

    print(f"{path}")
    print(f"denominator: lift scope {scope.byte_count} bytes across "
          f"{scope.functions} functions")
    print(f"  {metrics.EXCLUSION_REASON}")
    print(f"swept:       {swept.byte_count} bytes across {swept.functions} "
          f"functions ({swept.percent_of(scope):.2f}% of scope bytes)")
    unswept = Tally(scope.functions - swept.functions,
                    scope.byte_count - swept.byte_count)
    if unswept.functions or unswept.byte_count:
        print(f"  NOT SWEPT: {unswept.byte_count} bytes across "
              f"{unswept.functions} functions - every percentage below is "
              f"over the whole scope, so these bytes count as no evidence")
    if unpriced.functions:
        print(f"  NOT PRICED: {unpriced.functions} report rows the lift scope "
              f"does not contain; they carry 0 bytes here")
    print()

    print(f"{'verdict':30s} {'bytes':>10s} {'% scope':>9s} {'functions':>10s}")
    for verdict, value in sorted(by_verdict.items(),
                                 key=lambda item: -item[1].byte_count):
        print(f"{verdict:30s} {value.byte_count:10d} "
              f"{value.percent_of(scope):8.2f}% {value.functions:10d}")

    ran = tally_rows(rows, executed)
    agreed = tally_rows(rows, lambda row: row.verdict == "PASS")
    compared_any = tally_rows(rows,
                              lambda row: row.verdict in COMPARED_VERDICTS)

    tiers: dict[str, Tally] = {name: Tally() for name in TIER_ORDER}
    if has_compared:
        for row in rows:
            if row.verdict != "PASS" or not row.priced:
                continue
            name = evidence_tier(row.cases, row.compared, row.detail)
            tiers[name] = tiers[name] + Tally(1, row.size)
    full = tiers[TIER_FULL]
    weakened = Tally(agreed.functions - full.functions,
                     agreed.byte_count - full.byte_count)

    print()
    print(line("lifted body executed at least once:", ran, scope))
    print(line("compared and agreed:", agreed, scope))
    if has_compared:
        print(line("  ...of which FULL-STRENGTH agreement:", full, scope))
        print(f"{'':36s} {'':9s}    {full.percent_of(agreed):6.2f}% of the "
              f"agreeing bytes")

    if has_compared and agreed.functions:
        print("\nevidence quality of the agreeing bytes "
              "(each PASS in exactly one tier):")
        print(f"  {'tier':20s} {'bytes':>9s} {'% agreed':>9s} "
              f"{'functions':>10s}")
        for name in TIER_ORDER:
            value = tiers[name]
            print(f"  {name:20s} {value.byte_count:9d} "
                  f"{value.percent_of(agreed):8.2f}% {value.functions:10d}"
                  f"   {TIER_MEANING[name]}")

    debt = metrics.machine_carried(catalogue)
    never = Tally(scope.functions - compared_any.functions,
                  scope.byte_count - compared_any.byte_count)
    paths_taken = tally_rows(rows,
                             lambda row: row.verdict == PATHS_TAKEN_VERDICT)
    print("\nNUMBERS THAT MUST GO DOWN")
    print(line("  carried by machine-derived code:", debt, scope))
    print(line("  never compared by the oracle:", never, scope))
    if paths_taken.functions:
        print(line("  agreed only on the paths seeds took:", paths_taken,
                   scope))
    if has_compared:
        print(line("  agreed under a weakened condition:", weakened, scope))
        print(f"{'':36s} {'':9s}    {weakened.percent_of(agreed):6.2f}% of the "
              f"agreeing bytes")

    compared_histogram: Counter[int] = Counter()
    attempted = 0
    if has_compared:
        for row in rows:
            if row.verdict == "PASS" and row.priced:
                # Priced only, so that this and the byte-weighted tier table
                # above are two views of one set of rows rather than two sets.
                compared_histogram[row.compared] += 1
            attempted = max(attempted, row.cases)
    if compared_histogram:
        # The denominator is the `cases` column - seeds ATTEMPTED - not the
        # best `compared` seen. Reading the latter made the header say "out of
        # 4" for a whole sweep after the case count became 16.
        print(f"\nseeds that actually compared, among the PASSes "
              f"(out of {attempted} attempted):")
        for seeds in sorted(compared_histogram):
            count = compared_histogram[seeds]
            print(f"  {seeds:2d} seeds compared: {count:5d} "
                  f"({100.0 * count / max(agreed.functions, 1):.1f}%)")
        one = compared_histogram.get(1, 0)
        print(f"  -> {one} PASSes rest on a single seed; the original side "
              f"could not be run on any other.")

    failures = [row for row in rows if row.verdict.startswith("FAIL")]
    if failures:
        print(f"\n{len(failures)} FAILING functions "
              f"({sum(row.size for row in failures)} bytes):")
        for row in sorted(failures, key=lambda item: -item.size):
            print(f"\n  {row.address:#010x}  {row.size:6d} B  {row.verdict}  "
                  f"{row.name}")
            for piece in row.detail.split("; "):
                if piece.strip():
                    print(f"      {piece.strip()}")

    if args.json:
        record = {
            "report": str(path),
            "denominator": {
                "name": "lift_scope",
                "bytes": scope.byte_count,
                "functions": scope.functions,
                "exclusion": metrics.EXCLUSION_REASON,
            },
            "swept": swept.as_dict(scope),
            "unswept": unswept.as_dict(scope),
            "by_verdict": {verdict: value.as_dict(scope)
                           for verdict, value in sorted(by_verdict.items())},
            "executed": ran.as_dict(scope),
            "agreed": agreed.as_dict(scope),
            "agreed_full_strength": full.as_dict(scope),
            "agreement_evidence": {name: tiers[name].as_dict(scope)
                                   for name in TIER_ORDER},
            "must_go_down": {
                "machine_carried": debt.as_dict(scope),
                "never_compared": never.as_dict(scope),
                # Unconditionally present, unlike the printed line, which is
                # suppressed at zero. A record is diffed against another
                # record: a key that appears only when it is non-zero reads as
                # a schema change on the run where the number first goes wrong.
                "agreed_only_on_paths_taken": paths_taken.as_dict(scope),
                "agreed_under_weakened_conditions": weakened.as_dict(scope),
            },
            "seeds_attempted": attempted,
            "seeds_compared_histogram": {str(seeds): count for seeds, count
                                         in sorted(compared_histogram.items())},
        }
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(json.dumps(record, indent=2, sort_keys=True)
                             + "\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    sys.exit(main())
