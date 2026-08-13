#!/usr/bin/env python3
"""One denominator, byte-weighted, for every number this project publishes.

This project's defining measurement error was counting FUNCTIONS. 40% of the
functions was 8% of the bytes, and the published figure was the 40%. The lesson
was only half-applied: `lifted_oracle_summary.py` weights by bytes, while
`docs/recovery/summary.json` - the file the repository actually publishes - was
counts and nothing but counts.

Everything here exists so that there is exactly ONE place that decides:

  * WHICH ROWS ARE IN SCOPE. Two totals were in play. The catalogue holds 6,000
    rows; the oracle plans 5,673, because `tools/lift_whole_image.py` drops
    `external_library` rows by written policy - CRT and Windows code the lift
    must call and must never translate. Both numbers were published, neither
    said which it was, and a reader could not tell 7.4% of one from 7.4% of the
    other. `lift_scope` below is the denominator; `EXCLUSION_REASON` travels
    with it so the exclusion is stated wherever the number is.

  * HOW A ROW IS PRICED. `size` is the SUM OF BODY SPANS. It is NOT
    `end_address - address`: a chunked function spans address range that
    belongs to other functions, and this repository has made that substitution
    once already, inflating one function 40x. Every byte figure in the project
    comes from `row_size` here.

  * WHICH DIRECTION A NUMBER MOVES. A dashboard where every number goes up
    cannot report a regression. `machine_carried` is the number that must go
    DOWN and had no metric at all: the bytes whose behaviour is still supplied
    by machine-derived code rather than by recovered repository source.

    `--delta <ref>` prices that direction over time: it reads the catalogue as
    it stood at a git ref, prices it through the SAME `machine_carried`, and
    says `better`/`worse`/`same` in words rather than leaving a signed integer
    for the reader to interpret. The old catalogue arrives through
    `git show <ref>:<path>`, which never touches the worktree - no checkout, no
    stash - and a ref git cannot produce is an error, never a silent fallback
    to the worktree file, because that would print a delta of 0 and read as
    "no regression".

    `--delta` now REQUIRES the catalogue path to be named, and refuses the
    `src/` store. `git show` can only produce a file, the store is 3,499
    annotated files carrying 6,031 annotations (measured 2026-08-13), and the
    one export it could have shown - docs/recovery/functions.csv - was deleted
    in 185dd977. The default that
    survived that deletion did not stop working, which was the problem: for
    refs at or before it, `--delta` compared the CSV's taxonomy against the
    store's and reported the difference as recovery. See
    `load_catalogue_at_ref` for the measurement.

Nothing in this module reads the oracle report or the original executable; it
is arithmetic over the committed catalogue, so it is testable without Wine, an
IDB, or the user's copy of the game.
"""

from __future__ import annotations

import argparse
import csv
import io
import subprocess
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent

# The one row class that is not the project's to recover, and so is not image.
# It is a `recovery_state`, NOT `binary_kind == "library"`: the two differ,
# measurably - 338 rows are library code and 327 are external_library, because
# a library row that acquires a source annotation stops being external. The
# lift filters on recovery_state, so the denominator must too or the two would
# disagree by eleven functions and nobody would notice.
EXCLUDED_RECOVERY_STATE = "external_library"

EXCLUSION_REASON = (
    "excludes recovery_state == external_library: CRT and Windows library code "
    "that the lift calls and never translates "
    "(tools/lift_whole_image.py drops these rows by written policy)"
)

# The one row class whose behaviour is supplied by repository source. Every
# other state in scope - unrecovered, original_dependency, source_in_progress,
# thunk - is still carried by machine-derived code: the whole-image lift, a
# fixed-address call into the original, or a generated thunk.
RECOVERED_RECOVERY_STATE = "source_complete"


class Tally:
    """A count of functions AND the bytes they carry, never one without the
    other. Both fields are required at construction so that no caller can
    accidentally publish a count that has no byte figure beside it."""

    __slots__ = ("functions", "byte_count")

    def __init__(self, functions: int = 0, byte_count: int = 0):
        self.functions = functions
        self.byte_count = byte_count

    def __add__(self, other: "Tally") -> "Tally":
        return Tally(self.functions + other.functions,
                     self.byte_count + other.byte_count)

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, Tally):
            return NotImplemented
        return (self.functions, self.byte_count) == (other.functions,
                                                     other.byte_count)

    def __repr__(self) -> str:
        return f"Tally(functions={self.functions}, bytes={self.byte_count})"

    def percent_of(self, denominator: "Tally") -> float:
        """Byte percentage, rounded to the two places the reports print.

        A zero denominator yields 0.0, never 100.0: a stale catalogue that
        prices the whole image at zero must read as no coverage, not as total
        coverage.
        """
        if denominator.byte_count <= 0:
            return 0.0
        return round(100.0 * self.byte_count / denominator.byte_count, 2)

    def as_dict(self, denominator: "Tally" | None = None,
                percent_key: str = "percent_of_lift_scope_bytes") -> dict:
        """The published shape. `bytes` is listed first deliberately: this is
        the field that leads, and `functions` is the qualifier beside it.

        `percent_key` exists because not every published percentage is over
        the lift scope. `unproven_recovered` is a share of RECOVERED bytes, and
        emitting it as `percent_of_lift_scope_bytes` would repeat this file's
        founding error in a new place: a percentage whose name states a
        denominator it was not computed over. The caller names the denominator
        it used, and the key says so."""
        payload = {"bytes": self.byte_count, "functions": self.functions}
        if denominator is not None:
            payload[percent_key] = self.percent_of(denominator)
        return payload


def row_size(row: dict) -> int:
    """The bytes a catalogue row carries: the `size` column and nothing else.

    A row whose size cannot be read is priced at ZERO and keeps its place in
    the count. Guessing from `end_address` would silently substitute an address
    extent for a body size on exactly the rows least able to defend themselves.
    """
    try:
        return int(row.get("size") or 0)
    except (TypeError, ValueError):
        return 0


def row_state(row: dict) -> str:
    return (row.get("recovery_state") or "").strip()


def parse_catalogue(handle) -> list[dict]:
    """THE dialect decision, in one place, for both catalogue sources.

    A catalogue arrives either from the worktree (a file) or from a git ref (a
    string). Both go through here so that the header, the quoting and the BOM
    are decided identically: a second reader that decoded plain `utf-8` would
    name the first column `﻿address` on a BOM'd ref, leaving `row_state`
    and `row_size` working - so the totals would still look plausible - while
    `row_address` returned None for every row and the proven split silently
    collapsed to zero.
    """
    return list(csv.DictReader(handle))


def load_catalogue(path: Path | str = None) -> list[dict]:
    """Every catalogued row, from `src/` - the store since functions.csv went.

    This module holds the project's ONE byte-weighted denominator, so it was
    the worst possible thing to leave reading a deleted file: it did not report
    a wrong number, it raised FileNotFoundError, which at least fails loudly.
    A path still wins so a regenerated export can be measured against.
    """
    if path is not None:
        with Path(path).open(newline="", encoding="utf-8-sig") as handle:
            return parse_catalogue(handle)
    import sys as _sys
    _sys.path.insert(0, str(Path(__file__).resolve().parent))
    import emit_translation_unit as _emit
    return list(_emit.load_functions().values())


class CatalogueRefError(RuntimeError):
    """A catalogue git could not produce at the requested ref.

    Its own class so that no caller can mistake it for "the file is fine, the
    numbers are equal". Every failure here - unknown ref, path absent at that
    ref, git missing - must reach the user as an error; falling back to the
    worktree file would print `0 B` and read as "no regression".
    """


def load_catalogue_at_ref(ref: str, path: Path | str,
                          repo: Path | str = REPO_ROOT) -> list[dict]:
    """The catalogue as it stood at `ref`, without touching the worktree.

    `git show` writes to stdout only: no checkout, no stash, no index change.
    That matters more than convenience - the comparison must be runnable while
    the tree is dirty, which is precisely when someone wants to know whether
    the debt moved.

    `path` HAS NO DEFAULT, and the absence is load bearing. It defaulted to
    docs/recovery/functions.csv, which 185dd977 deleted - and the default did
    not then fail, which is why it was worth removing rather than repointing.
    It kept resolving for every ref at or before that commit and answered with
    a comparison between two different measurements. Measured 2026-08-13:
    `--delta 185dd977^` printed "better: debt FELL 440339 B" over a day in
    which 24 commits changed 28 annotation lines. 1,666 of the 6,000 rows
    disagree on recovery_state between that export and today's `src/` store,
    and 341 of those disagree because the export spells states the store never
    emits at all - original_dependency, thunk, source_in_progress against the
    store's external_library / source_complete / unrecovered. That is the
    failure DeltaTests names IT SILENTLY REDEFINES THE METRIC, arriving through
    the default argument instead of through a bad ref.

    So the ref side and the worktree side must be two snapshots of ONE
    catalogue file, and only the caller knows which file that is.
    """
    root = Path(repo).resolve()
    try:
        relative = Path(path).resolve().relative_to(root).as_posix()
    except ValueError as error:
        raise CatalogueRefError(
            f"{path} is outside {root}, so no git ref can name it") from error
    try:
        completed = subprocess.run(
            ["git", "-C", str(root), "show", f"{ref}:{relative}"],
            capture_output=True, check=True)
    except FileNotFoundError as error:  # no git on this machine
        raise CatalogueRefError(
            "git is not on PATH, so no earlier catalogue can be read") from error
    except subprocess.CalledProcessError as error:
        detail = (error.stderr or b"").decode("utf-8", "replace").strip()
        raise CatalogueRefError(
            f"cannot read {relative} at {ref}: "
            f"{detail or f'git show exited {error.returncode}'}") from error
    text = completed.stdout.decode("utf-8-sig")
    return parse_catalogue(io.StringIO(text, newline=""))


def tally(rows, predicate=None) -> Tally:
    result = Tally()
    for row in rows:
        if predicate is not None and not predicate(row):
            continue
        result = result + Tally(1, row_size(row))
    return result


def catalogued(rows) -> Tally:
    """Every row the catalogue holds, in scope or not."""
    return tally(rows)


def excluded(rows) -> Tally:
    """The rows `lift_scope` leaves out. Published beside it, never silently."""
    return tally(rows, lambda row: row_state(row) == EXCLUDED_RECOVERY_STATE)


def lift_scope(rows) -> Tally:
    """THE denominator. Every percentage this project publishes is over this."""
    return tally(rows, lambda row: row_state(row) != EXCLUDED_RECOVERY_STATE)


def recovered(rows) -> Tally:
    """In-scope bytes whose behaviour comes from repository source."""
    return tally(rows, lambda row: row_state(row) == RECOVERED_RECOVERY_STATE)


def machine_carried(rows) -> Tally:
    """MUST GO DOWN. In-scope bytes still carried by machine-derived code.

    The complement of `recovered` within `lift_scope`, and the only published
    number that falls when the project succeeds. Recovering a function moves
    its bytes out of here; adding a lifted body, a fixed-address dependency or
    a generated thunk does not, because none of those is source.
    """
    return tally(rows, lambda row: row_state(row) not in (
        EXCLUDED_RECOVERY_STATE, RECOVERED_RECOVERY_STATE))


def row_address(row: dict) -> int | None:
    """The canonical start of a catalogue row, or None when it has none.

    Tolerant in the same two directions `row_size` is, and for the same reason:
    this module is called both with rows straight from `csv.DictReader` (every
    field a string, `address` written `0x004BA830`) and with rows built in
    memory by tests and by `export_recovery_inventory`, where it may already be
    an int. A row with no readable address is not proven - it simply cannot be
    matched against the proven catalogue - so it returns None rather than
    raising and taking the whole summary down with it.
    """
    value = row.get("address")
    if value is None:
        # `export_recovery_inventory` names it `start` and holds it as an int;
        # the CSV names it `address` and holds it as `0x004BA830`. This module
        # is called with both, which its own header says, and looking for only
        # one key made every row from the inventory unmatchable - publishing
        # 100% unproven while the same arithmetic over the CSV said 32
        # functions.
        value = row.get("start")
    if isinstance(value, int):
        return value
    try:
        return int(value, 16)
    except (TypeError, ValueError):
        return None


def proven_recovered(rows, proven: set[int]) -> Tally:
    """Recovered bytes that have been RUN against the original and agreed.

    `recovered` counts a declaration: `Status: Complete` in a source annotation,
    which `export_recovery_inventory` promotes to `source_complete` on the
    annotation alone. Nothing checked that the body it annotates behaves like
    the function it replaces, so a wrong recovery counted exactly as much as a
    right one and no published number could tell them apart.

    `proven` comes from `docs/recovery/proven.csv`, which lists the functions an
    oracle actually executes against the original. It is a COMMITTED catalogue
    rather than a build artefact on purpose: this module is arithmetic over the
    repository and must stay runnable without Wine, an IDB, or the game.
    """
    return tally(rows, lambda row: (row_state(row) == RECOVERED_RECOVERY_STATE
                                    and row_address(row) in proven))


def unproven_recovered(rows, proven: set[int]) -> Tally:
    """MUST GO DOWN. Recovered bytes never executed against the original.

    The second number in this file that falls when the project succeeds, and
    the only one that can catch a recovery that is complete and WRONG.
    `machine_carried` cannot: it goes down the moment a function is declared
    recovered, whether or not the declaration is true.
    """
    return tally(rows, lambda row: (row_state(row) == RECOVERED_RECOVERY_STATE
                                    and row_address(row) not in proven))


SHAPES_CSV = (
    Path(__file__).resolve().parent.parent / "docs" / "recovery"
    / "recovered-shapes.csv")


def load_shapes(path=None) -> dict:
    """address -> shape, from the committed catalogue, or {} if absent.

    Like `load_proven_addresses`, this reads a committed file rather than
    deciding anything, so this module keeps working with no IDB, no Wine and no
    executable. An empty result means the split is not published - which reads
    as "we do not know why", not as "there is no reason".

    The file is FROZEN, and callers must price the split as a snapshot rather
    than a current figure: classify_recovered_shapes.py, its only writer, was
    deleted in 1058cb94 and the `recovered-shapes-current` gate retired before
    it (docs/RETIRED_ROUTES.md). Measured 2026-08-13 the file holds 2,577 rows
    against 4,112 source_complete functions in src/, so 1,563 of them classify
    as `unclassified` - staleness wearing the look of ignorance, which is the
    one way this table misleads. It cannot lose bytes doing it: every row still
    lands in exactly one bucket, which is what the tests defend.
    """
    source = SHAPES_CSV if path is None else Path(path)
    if not source.is_file():
        return {}
    shapes = {}
    with source.open(newline="", encoding="utf-8-sig") as handle:
        for row in csv.DictReader(handle):
            address = row_address(row)
            if address is not None:
                shapes[address] = row["shape"]
    return shapes


def unproven_by_shape(rows, proven: set[int], shapes: dict) -> dict:
    """WHY each unproven byte is unproven, so the total is not read as a target.

    190,037 bytes published as one figure reads as a debt that could reach zero.
    It cannot. `seam_forwarding` is a floor no differential can reach at any
    effort - those recovered bodies hand the ORIGINAL address onward through a
    shared seam, so both sides of a comparison run the same original code. And
    the buckets are wildly uneven in a way the single number hides: measured
    2026-07-30, 58.86% of the unproven bytes sit behind a loop while 7.80% are
    the floor, so a plan aimed at "the debt" is really aimed at whichever bucket
    it happens to be able to move.

    A shape is a property of the ORIGINAL body, so it does not depend on the
    recovery being right, and it does not change when a function is proven -
    the function simply leaves this table.
    """
    unproven = [row for row in rows
                if row_state(row) == RECOVERED_RECOVERY_STATE
                and row_address(row) not in proven]
    out: dict[str, dict] = {}
    total = sum(row_size(row) for row in unproven)
    for row in unproven:
        shape = shapes.get(row_address(row), "unclassified")
        entry = out.setdefault(shape, {"bytes": 0, "functions": 0})
        entry["bytes"] += row_size(row)
        entry["functions"] += 1
    for shape, entry in out.items():
        entry["percent_of_unproven_bytes"] = (
            round(100.0 * entry["bytes"] / total, 2) if total else 0.0)
    return dict(sorted(out.items(), key=lambda kv: -kv[1]["bytes"]))


def grouped(rows, column: str, denominator: Tally) -> dict:
    """{value: {bytes, functions, percent}} over one catalogue column.

    Rows outside `lift_scope` keep their group and their byte figure - dropping
    them would make the table disagree with the catalogue it summarises - but a
    PERCENTAGE OF THE LIFT SCOPE may only ever be the group's IN-SCOPE bytes
    over the scope. The two are not the same number for any column that cuts
    across the exclusion, and `binary_kind` is exactly such a column: 45,686
    bytes are `library` and only 531 of them are in the scope, so pricing the
    whole group against the scope published 1.9% for a group that is 0.02% of
    it, and made the three kinds sum to 101.88% of a total they are presented
    as shares of.

    This used to be special-cased on `column == "recovery_state"`, which is
    what let `binary_kind` through: the exclusion is a property of the ROWS,
    never of the column being tabulated. A group holding excluded rows carries
    the split - `in_scope` and `excluded` - so the difference between its byte
    figure and its percentage is visible in the same object rather than left
    for a reader to discover by adding the column up.
    """
    in_scope: dict[str, Tally] = {}
    left_out: dict[str, Tally] = {}
    for row in rows:
        key = (row.get(column) or "").strip()
        weight = Tally(1, row_size(row))
        side = (left_out if row_state(row) == EXCLUDED_RECOVERY_STATE
                else in_scope)
        side[key] = side.get(key, Tally()) + weight
    out = {}
    for key in sorted(set(in_scope) | set(left_out)):
        counted = in_scope.get(key, Tally())
        dropped = left_out.get(key, Tally())
        payload = (counted + dropped).as_dict()
        payload["percent_of_lift_scope_bytes"] = counted.percent_of(denominator)
        if dropped.functions:
            payload["in_scope"] = counted.as_dict()
            payload["excluded"] = dropped.as_dict()
        out[key] = payload
    return out


def bytes_block(rows, proven: set[int] | None = None) -> dict:
    """The byte-weighted block published inside `summary.json`.

    Self-describing on purpose. A reader who finds a percentage in this file
    can find, in the same object, the denominator it is over, the rows that
    denominator leaves out, and why.
    """
    scope = lift_scope(rows)
    proven = set() if proven is None else proven
    recovered_tally = recovered(rows)
    return {
        "denominator": {
            "name": "lift_scope",
            "bytes": scope.byte_count,
            "functions": scope.functions,
            "note": ("every percent_of_lift_scope_bytes in this file is over "
                     "these bytes"),
            "exclusion": EXCLUSION_REASON,
        },
        "catalogued": catalogued(rows).as_dict(),
        "excluded": excluded(rows).as_dict(),
        "recovered": recovered(rows).as_dict(scope),
        "machine_carried": {
            **machine_carried(rows).as_dict(scope),
            "direction": "must go down",
            "meaning": ("in-scope bytes whose behaviour is still supplied by "
                        "machine-derived code rather than recovered source"),
        },
        "proven_recovered": {
            **proven_recovered(rows, proven).as_dict(
                recovered_tally, "percent_of_recovered_bytes"),
            "denominator": "recovered",
            "meaning": ("recovered bytes an oracle has executed against the "
                        "original and found to agree"),
        },
        "unproven_recovered": {
            **unproven_recovered(rows, proven).as_dict(
                recovered_tally, "percent_of_recovered_bytes"),
            "denominator": "recovered",
            "direction": "must go down",
            "meaning": ("recovered bytes never executed against the original: "
                        "declared complete, never demonstrated equivalent"),
        },
        "unproven_by_shape": unproven_by_shape(
            rows, proven, load_shapes()),
        "by_recovery_state": grouped(rows, "recovery_state", scope),
        "by_binary_kind": grouped(rows, "binary_kind", scope),
    }


def scope_sizes(rows) -> dict[int, int]:
    """{address: size} for the lift scope, for pricing an oracle report.

    Out-of-scope rows are absent rather than present at zero, so a caller can
    tell "this address is library code" from "this address is not catalogued".
    """
    sizes: dict[int, int] = {}
    for row in rows:
        if row_state(row) == EXCLUDED_RECOVERY_STATE:
            continue
        try:
            address = int(row["address"], 16)
        except (KeyError, TypeError, ValueError):
            continue
        sizes[address] = row_size(row)
    return sizes


def delta_lines(old_rows, new_rows, ref: str) -> list[str]:
    """`machine_carried: <old> -> <new> (<signed delta> B)`, plus its meaning.

    The sign is never left bare. `machine_carried` must go DOWN, so a NEGATIVE
    delta is progress - the one place in this project where a minus sign is the
    good news, and the one most likely to be read the other way round. The word
    `better`/`worse`/`same` carries the direction, exactly as
    `lifted_oracle_compare.verdict` does, and `fell`/`rose` restates it so that
    a line quoted out of context still says which way the debt moved.

    The old catalogue's shape is printed beside the delta because
    `machine_carried` is a membership test over two state NAMES: a ref whose
    catalogue spelled a state differently, or held a different number of rows,
    would move the number with no recovery having happened. Row count and lift
    scope make that visible instead of letting it read as progress.
    """
    old = machine_carried(old_rows)
    new = machine_carried(new_rows)
    delta = new.byte_count - old.byte_count
    if delta == 0:
        mark, moved = "same", "debt unchanged"
    elif delta < 0:
        mark, moved = "better", f"debt FELL {-delta} B"
    else:
        mark, moved = "worse", f"debt ROSE {delta} B"
    old_scope, new_scope = lift_scope(old_rows), lift_scope(new_rows)
    return [
        f"machine_carried: {old.byte_count} -> {new.byte_count} "
        f"({delta:+d} B)  {mark}: {moved} (must go down)",
        f"  functions: {old.functions} -> {new.functions}",
        f"  old ({ref}): {len(old_rows)} catalogue rows, lift scope "
        f"{old_scope.byte_count} B / {old_scope.functions} fn",
        f"  new (worktree): {len(new_rows)} catalogue rows, lift scope "
        f"{new_scope.byte_count} B / {new_scope.functions} fn",
    ]


def main(argv=None) -> int:
    import sys

    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    # Positional, optional, and still the documented way to price another
    # catalogue: `recovery_metrics.py <some.csv>` worked before argparse
    # existed here and must keep working.
    parser.add_argument("catalogue", nargs="?", default=None,
                        help="catalogue CSV to price (default: the src/ store, "
                             "which is where the catalogue lives)")
    parser.add_argument("--delta", metavar="REF",
                        help="also print how machine_carried moved since this "
                             "git ref, read with `git show REF:<catalogue>` - "
                             "the worktree is never touched")
    args = parser.parse_args(sys.argv[1:] if argv is None else argv)

    # A ref comparison needs the SAME file on both sides, and `git show` can
    # only produce a file. Named here rather than left to fail inside
    # load_catalogue_at_ref: the old default made this invocation report a
    # 440,339 B improvement that never happened (see load_catalogue_at_ref),
    # so it fails before any number is printed, not after.
    if args.delta and args.catalogue is None:
        print("error: --delta REF needs a catalogue CSV named as well - the "
              "src/ store is thousands of annotated source files, not one file "
              "`git show` can produce, and docs/recovery/functions.csv was "
              "deleted in 185dd977. Pass a committed catalogue: "
              "recovery_metrics.py <some.csv> --delta REF", file=sys.stderr)
        return 1

    # `None` is the store. It is the ONLY spelling of the store now: the
    # argument used to default to docs/recovery/functions.csv and be compared
    # back against that constant here, so the store was reached by recognising
    # a path that had not existed since 185dd977.
    rows = load_catalogue(args.catalogue)
    scope = lift_scope(rows)
    print(f"catalogued      {catalogued(rows).byte_count:9d} B  "
          f"{catalogued(rows).functions:5d} fn")
    print(f"excluded        {excluded(rows).byte_count:9d} B  "
          f"{excluded(rows).functions:5d} fn   ({EXCLUSION_REASON})")
    print(f"lift scope      {scope.byte_count:9d} B  {scope.functions:5d} fn  "
          f"<- the denominator")
    debt = machine_carried(rows)
    print(f"machine-carried {debt.byte_count:9d} B  {debt.functions:5d} fn   "
          f"{debt.percent_of(scope):.2f}% of scope bytes  (must go down)")
    if args.delta:
        try:
            # REPO_ROOT passed, not defaulted: a default argument binds at
            # def time, so the CLI's ref path could only ever be tested
            # against THIS repository - and once functions.csv went, that
            # test skipped instead of running. Read from the module global
            # here and a throwaway repo can stand in for it.
            old_rows = load_catalogue_at_ref(args.delta, args.catalogue,
                                             REPO_ROOT)
        except CatalogueRefError as error:
            print(f"error: {error}", file=sys.stderr)
            return 1
        print()
        for line in delta_lines(old_rows, rows, args.delta):
            print(line)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
