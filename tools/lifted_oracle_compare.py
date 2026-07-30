#!/usr/bin/env python3
"""Diff two oracle reports: what the move, or the change, actually cost.

`lifted_oracle_summary.py` describes one report. The question this answers is
the other one - "is the new report the same result as the old" - and it is not
answerable from two summaries side by side, for a reason that is specific to
this project:

  THE TOTALS CAN AGREE WHILE THE SET DOES NOT. `agreed` is a byte sum over
  whichever functions passed. Twenty small functions can start passing while
  one large one stops, and the byte total moves by a few hundred either way
  while the function count barely shifts. Reading only the totals, that is a
  rounding wobble; read as a set, it is a regression with a name and an
  address. So the comparison here is a per-function verdict TRANSITION, and
  the totals are printed under it rather than instead of it.

  FULL-STRENGTH AND TOTAL AGREEMENT MOVE IN OPPOSITE DIRECTIONS. A change that
  trades large single-seed PASSes for small full-strength ones lowers `agreed`
  and raises `agreed_full_strength`, which is an improvement that looks like a
  loss on the headline. Both are printed, separately, always.

  DIRECTION IS A PROPERTY OF THE FIGURE, NOT OF THE READER. `must_go_down`
  figures are regressions when they RISE; every other figure is a regression
  when it FALLS. A diff that prints signed deltas and leaves the reader to
  remember which is which is the same dashboard that once reported every
  number going up. Each line here says `better`, `worse`, or `same` in its own
  terms.

The numbers come from `lifted_oracle_summary`'s own functions, not from a
second implementation of them: two definitions of "full-strength" that drift
apart would make every comparison this tool prints meaningless in exactly the
way it exists to prevent.

Usage:
    lifted_oracle_compare.py BEFORE.tsv AFTER.tsv [--only-regressions]
"""

from __future__ import annotations

import argparse
import sys
from collections import defaultdict
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import lifted_oracle_summary as summary  # noqa: E402
import recovery_metrics as metrics  # noqa: E402
from recovery_metrics import Tally  # noqa: E402

# Every figure this tool compares, and which way is good. The direction lives
# in the table beside the name so that adding a figure forces the author to
# state it; a default would silently make the next `must_go_down` figure read
# as an improvement while it rose.
HIGHER_IS_BETTER = "higher"
LOWER_IS_BETTER = "lower"


def summarise(path: Path, sizes: dict[int, int]):
    """One report, as {address: Row} plus the aggregate tallies."""
    rows, _ = summary.read_report(path, sizes)
    return {row.address: row for row in rows}


def tier_of(row) -> str:
    """The evidence tier of an agreeing row; '' for a row that does not agree."""
    if row.verdict != "PASS":
        return ""
    return summary.evidence_tier(row.cases, row.compared, row.detail)


def tally(rows, predicate) -> Tally:
    return summary.tally_rows(rows, predicate)


def figures(rows) -> dict:
    """The published figures, computed by the summary's own definitions."""
    def agreed(row):
        return row.verdict == "PASS"

    def full_strength(row):
        return agreed(row) and tier_of(row) == summary.TIER_FULL

    def weakened(row):
        return agreed(row) and tier_of(row) != summary.TIER_FULL

    def never_compared(row):
        return row.verdict not in summary.COMPARED_VERDICTS

    def paths_taken(row):
        return row.verdict == summary.PATHS_TAKEN_VERDICT

    return {
        "agreed": (tally(rows, agreed), HIGHER_IS_BETTER),
        "agreed_full_strength": (tally(rows, full_strength), HIGHER_IS_BETTER),
        "executed": (tally(rows, summary.executed), HIGHER_IS_BETTER),
        "never_compared": (tally(rows, never_compared), LOWER_IS_BETTER),
        "agreed_under_weakened_conditions": (tally(rows, weakened),
                                             LOWER_IS_BETTER),
        "agreed_only_on_paths_taken": (tally(rows, paths_taken),
                                       LOWER_IS_BETTER),
    }


def verdict(delta: int, direction: str) -> str:
    """`better`, `worse` or `same` - never a bare sign."""
    if delta == 0:
        return "same"
    improved = delta > 0 if direction == HIGHER_IS_BETTER else delta < 0
    return "better" if improved else "worse"


def compare_figures(before: dict, after: dict) -> list[str]:
    lines = ["FIGURES (bytes lead; direction is stated, not implied)",
             f"  {'figure':<34}{'before':>10}{'after':>10}{'delta':>10}"
             f"  {'':<7}{'functions':>18}"]
    for name, (old, direction) in before.items():
        new, _ = after[name]
        delta = new.byte_count - old.byte_count
        mark = verdict(delta, direction)
        arrow = "" if direction == HIGHER_IS_BETTER else "  (must go down)"
        lines.append(
            f"  {name:<34}{old.byte_count:>10}{new.byte_count:>10}"
            f"{delta:>+10}  {mark:<7}"
            f"{old.functions:>8} -> {new.functions:<7}{arrow}")
    return lines


def transitions(before: dict, after: dict):
    """{(old_verdict, new_verdict): [rows]} over the addresses in both."""
    moved: dict[tuple, list] = defaultdict(list)
    for address, old in before.items():
        new = after.get(address)
        if new is None:
            continue
        if old.verdict != new.verdict:
            moved[(old.verdict, new.verdict)].append((old, new))
    return moved


def tier_moves(before: dict, after: dict):
    """Agreement that stayed agreement but changed evidence quality."""
    moved: dict[tuple, list] = defaultdict(list)
    for address, old in before.items():
        new = after.get(address)
        if new is None or old.verdict != "PASS" or new.verdict != "PASS":
            continue
        old_tier, new_tier = tier_of(old), tier_of(new)
        if old_tier != new_tier:
            moved[(old_tier, new_tier)].append((old, new))
    return moved


def is_regression(old_verdict: str, new_verdict: str) -> bool:
    """Losing agreement is the regression this tool exists to surface.

    Stated as "was PASS and is not" rather than as a ranking of every verdict
    against every other, because the interesting direction is unambiguous and a
    total order over verdicts would be an invention. A new FAIL is called out
    separately below whatever it came from - that is the finding the migration
    was looking for, and it is not necessarily a regression.
    """
    return old_verdict == "PASS" and new_verdict != "PASS"


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("before", type=Path)
    parser.add_argument("after", type=Path)
    parser.add_argument("--functions", type=Path, default=None)
    parser.add_argument("--only-regressions", action="store_true",
                        help="print only lost agreement and new FAILs")
    parser.add_argument("--limit", type=int, default=25,
                        help="rows listed per transition class")
    args = parser.parse_args(argv)

    catalogue = metrics.load_catalogue(args.functions or summary.FUNCTIONS)
    sizes = metrics.scope_sizes(catalogue)
    before = summarise(args.before, sizes)
    after = summarise(args.after, sizes)

    print(f"before: {args.before}  ({len(before)} rows)")
    print(f"after:  {args.after}  ({len(after)} rows)")

    only_before = set(before) - set(after)
    only_after = set(after) - set(before)
    if only_before or only_after:
        # Never silently intersected. Two reports over different address sets
        # are not comparable, and a tool that quietly compares the overlap
        # would report "no change" for a sweep that stopped early.
        print(f"\nADDRESS SETS DIFFER: {len(only_before)} only in before, "
              f"{len(only_after)} only in after")
        print("  every figure below covers the addresses present in BOTH")

    if not args.only_regressions:
        print()
        for text in compare_figures(
                figures(list(before.values())), figures(list(after.values()))):
            print(text)

    moved = transitions(before, after)
    lost = {key: rows for key, rows in moved.items() if is_regression(*key)}
    gained = {key: rows for key, rows in moved.items() if key[1] == "PASS"}
    new_fails = {key: rows for key, rows in moved.items()
                 if key[1].startswith("FAIL")}

    def dump(title: str, group: dict, note: str = "") -> None:
        total = Tally()
        for rows in group.values():
            for old, _ in rows:
                if old.priced:
                    total = total + Tally(1, old.size)
        print(f"\n{title}: {total.byte_count} B across {total.functions} fn")
        if note:
            print(f"  {note}")
        for (old_verdict, new_verdict), rows in sorted(
                group.items(), key=lambda item: -sum(r[0].size for r in item[1])):
            rows.sort(key=lambda pair: -pair[0].size)
            span = sum(old.size for old, _ in rows)
            print(f"  {old_verdict} -> {new_verdict}: "
                  f"{span} B across {len(rows)} fn")
            for old, new in rows[:args.limit]:
                detail = (new.detail or "").strip()
                suffix = f"  {detail[:100]}" if detail else ""
                print(f"    {old.address:#010x} {old.size:6d} B  "
                      f"{old.name}{suffix}")
            if len(rows) > args.limit:
                print(f"    ... and {len(rows) - args.limit} more")

    dump("LOST AGREEMENT (regression)", lost,
         "these functions agreed before and do not now")
    dump("NEW FAILS", new_fails,
         "a FAIL is a divergence the harness could actually judge; "
         "apply the layout-shim control before believing any of them")
    if not args.only_regressions:
        dump("GAINED AGREEMENT", gained)
        tiers = tier_moves(before, after)
        weakened = {k: v for k, v in tiers.items() if k[1] != summary.TIER_FULL
                    and k[0] == summary.TIER_FULL}
        strengthened = {k: v for k, v in tiers.items()
                        if k[1] == summary.TIER_FULL}
        dump("AGREEMENT WEAKENED (still PASS, worse evidence)", weakened)
        dump("AGREEMENT STRENGTHENED (still PASS, better evidence)",
             strengthened)

    # The exit status is the machine-readable answer, so this can gate a script
    # rather than only inform a reader.
    return 1 if lost else 0


if __name__ == "__main__":
    sys.exit(main())
