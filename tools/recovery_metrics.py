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

Nothing in this module reads the oracle report or the original executable; it
is arithmetic over the committed catalogue, so it is testable without Wine, an
IDB, or the user's copy of the game.
"""

from __future__ import annotations

import csv
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
FUNCTIONS_CSV = REPO_ROOT / "docs" / "recovery" / "functions.csv"

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


def load_catalogue(path: Path | str = FUNCTIONS_CSV) -> list[dict]:
    with Path(path).open(newline="", encoding="utf-8-sig") as handle:
        return list(csv.DictReader(handle))


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


def main() -> int:
    import sys

    rows = load_catalogue(sys.argv[1] if len(sys.argv) > 1 else FUNCTIONS_CSV)
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
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
