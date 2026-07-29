#!/usr/bin/env python3
"""Unit tests for the one module that decides what this project's numbers mean.

`recovery_metrics` owns three decisions, and every published figure inherits
all three:

* WHICH ROWS ARE IN SCOPE. The catalogue holds 6,000 rows and the oracle plans
  5,673; the difference is `external_library`, excluded by written policy. Both
  totals were published and neither said which it was. The tests give every
  recovery_state a distinct power of two so that any wrong exclusion produces a
  total no correct implementation can reach - excluding one state too many, or
  one too few, each lands somewhere unique.
* HOW A ROW IS PRICED. `size` is the SUM OF BODY SPANS, not the address extent.
  This repository has substituted one for the other once already and inflated a
  chunked function 40x; the fixture below keeps the two a factor of 40 apart so
  the substitution cannot pass.
* WHICH WAY A NUMBER MOVES. `machine_carried` is the only published figure that
  falls when the project succeeds. It is asserted as an exact complement -
  recovered + machine_carried == lift_scope, in BOTH fields - because a
  membership test that drifts by one state would still print a plausible
  number.

Two further hazards get their own tests because they are invisible by
construction:

* ONE IMPLEMENTATION, TWO CALLERS. The exporter passes in-memory dicts whose
  `size` is an int; `verify_recovery_metadata` and the oracle summary pass CSV
  rows whose `size` is a string. Both must produce byte-identical blocks, or
  the cheap reused-export path and the full export would publish different
  numbers for the same tree and only the byte comparison at the very end would
  notice.
* THE PUBLISHED FILE MUST AGREE WITH THE CATALOGUE IT SUMMARISES. A recovery
  that regenerates functions.csv without regenerating summary.json leaves a
  byte block describing a tree that no longer exists.
"""

from __future__ import annotations

import csv
import json
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import recovery_metrics as metrics
from recovery_metrics import Tally

REPO_ROOT = Path(__file__).resolve().parent.parent

# The six values that actually occur in docs/recovery/functions.csv, each with
# a distinct power of two so that any subset sums to a unique total.
STATES = [
    ("0x00401000", 1, "unrecovered", "game"),
    ("0x00402000", 2, "source_complete", "game"),
    ("0x00403000", 4, "source_in_progress", "game"),
    ("0x00404000", 8, "original_dependency", "game"),
    ("0x00405000", 16, "thunk", "thunk"),
    ("0x00406000", 32, "external_library", "library"),
]


def write_functions(directory, entries, name="functions.csv"):
    """A synthetic functions.csv. `entries` are
    (address, size, recovery_state, binary_kind[, end_address])."""
    path = Path(directory) / name
    with (Path(directory) / name).open("w", newline="", encoding="utf-8") as f:
        writer = csv.writer(f, lineterminator="\n")
        writer.writerow(["address", "end_address", "size", "name",
                         "binary_kind", "recovery_state"])
        for entry in entries:
            address, size, state, kind = entry[:4]
            end = (entry[4] if len(entry) > 4
                   else f"0x{int(address, 16) + 1:08X}")
            writer.writerow([address, end, size, f"fn_{address}", kind, state])
    return path


def rows_of(entries):
    tmp = tempfile.TemporaryDirectory()
    try:
        return metrics.load_catalogue(write_functions(tmp.name, entries))
    finally:
        tmp.cleanup()


class ScopeTests(unittest.TestCase):
    def setUp(self):
        self.rows = rows_of(STATES)

    def test_lift_scope_excludes_external_library_and_nothing_else(self):
        # 1+2+4+8+16 = 31. Excluding any other single state as well gives 30,
        # 29, 27, 23 or 15; excluding none gives 63. Only 31 is right.
        self.assertEqual(metrics.lift_scope(self.rows), Tally(5, 31))

    def test_catalogued_counts_every_row_including_the_excluded_one(self):
        self.assertEqual(metrics.catalogued(self.rows), Tally(6, 63))

    def test_excluded_is_published_beside_the_scope_not_hidden(self):
        self.assertEqual(metrics.excluded(self.rows), Tally(1, 32))

    def test_scope_and_exclusion_reconstruct_the_catalogue(self):
        """The two totals in play must add up, in both fields.

        This is the reconciliation the project lacked: 6,000 rows against
        5,673, with no statement anywhere that the difference was the 327
        library rows.
        """
        scope = metrics.lift_scope(self.rows)
        excluded = metrics.excluded(self.rows)
        catalogued = metrics.catalogued(self.rows)
        self.assertEqual(scope.functions + excluded.functions,
                         catalogued.functions)
        self.assertEqual(scope.byte_count + excluded.byte_count,
                         catalogued.byte_count)

    def test_exclusion_is_by_recovery_state_not_by_binary_kind(self):
        """The two disagree on the live catalogue: 338 library rows, 327
        external_library. A library row that acquires a source annotation stops
        being external, and the lift follows recovery_state.
        """
        rows = rows_of([
            ("0x00401000", 100, "source_complete", "library"),
            ("0x00402000", 7, "external_library", "library"),
        ])
        # Filtering on binary_kind would price the scope at 0 bytes.
        self.assertEqual(metrics.lift_scope(rows), Tally(1, 100))
        self.assertEqual(metrics.excluded(rows), Tally(1, 7))


class DirectionTests(unittest.TestCase):
    """`machine_carried` is the number that must go down."""

    def setUp(self):
        self.rows = rows_of(STATES)

    def test_machine_carried_is_the_exact_complement_of_recovered(self):
        scope = metrics.lift_scope(self.rows)
        recovered = metrics.recovered(self.rows)
        carried = metrics.machine_carried(self.rows)
        self.assertEqual(recovered + carried, scope)
        # 2 bytes recovered, 1+4+8+16 = 29 carried, out of 31 in scope.
        self.assertEqual(recovered, Tally(1, 2))
        self.assertEqual(carried, Tally(4, 29))

    def test_recovering_a_function_moves_bytes_out_of_the_debt(self):
        """The direction itself, not just the arithmetic.

        Promoting the largest unrecovered row to source_complete must make the
        debt fall by exactly that row's bytes and leave the scope untouched.
        """
        before = rows_of([("0x00401000", 900, "unrecovered", "game"),
                          ("0x00402000", 100, "source_complete", "game")])
        after = rows_of([("0x00401000", 900, "source_complete", "game"),
                         ("0x00402000", 100, "source_complete", "game")])
        self.assertEqual(metrics.lift_scope(before), metrics.lift_scope(after))
        self.assertEqual(metrics.machine_carried(before), Tally(1, 900))
        self.assertEqual(metrics.machine_carried(after), Tally(0, 0))

    def test_a_lifted_body_is_not_a_recovery(self):
        """original_dependency and thunk are machine-derived, not source.

        Both describe behaviour still supplied by the original image or by a
        generator. Counting either as recovered would make the debt fall
        without a line of source being written.
        """
        rows = rows_of([("0x00401000", 8, "original_dependency", "game"),
                        ("0x00402000", 16, "thunk", "thunk")])
        self.assertEqual(metrics.recovered(rows), Tally(0, 0))
        self.assertEqual(metrics.machine_carried(rows), Tally(2, 24))


class PricingTests(unittest.TestCase):
    def test_size_is_the_sum_of_spans_not_the_address_extent(self):
        """A chunked body owns 100 of the 0x1000 bytes it spans."""
        rows = rows_of([("0x00401000", 100, "unrecovered", "game",
                         "0x00402000")])
        self.assertEqual(metrics.lift_scope(rows), Tally(1, 100))

    def test_an_unreadable_size_is_priced_at_zero_and_keeps_its_row(self):
        rows = rows_of([("0x00401000", "n/a", "unrecovered", "game"),
                        ("0x00402000", 7, "unrecovered", "game")])
        self.assertEqual(metrics.lift_scope(rows), Tally(2, 7))

    def test_an_unknown_recovery_state_is_priced_not_dropped(self):
        """A state added to functions.csv later must not silently cost bytes.

        The filter is a positive test for one string, so a new state prices
        normally. An implementation that allow-listed the known states would
        drop the new one and quietly shrink the denominator.
        """
        rows = rows_of([("0x00401000", 777, "state_invented_next_quarter",
                         "game")])
        self.assertEqual(metrics.lift_scope(rows), Tally(1, 777))
        self.assertEqual(metrics.machine_carried(rows), Tally(1, 777))


class TallyTests(unittest.TestCase):
    def test_a_zero_denominator_reads_as_no_coverage_not_total_coverage(self):
        """A stale catalogue prices the whole sweep at zero.

        0.00% is the honest reading; 100.00% is the flattering one, and a naive
        `or 1` divide-by-zero guard produces exactly the flattering one for any
        numerator that is also zero.
        """
        self.assertEqual(Tally(1, 0).percent_of(Tally(0, 0)), 0.0)
        self.assertEqual(Tally(0, 0).percent_of(Tally(0, 0)), 0.0)

    def test_a_published_figure_always_carries_both_fields(self):
        payload = Tally(3, 400).as_dict(Tally(10, 1000))
        self.assertEqual(payload["bytes"], 400)
        self.assertEqual(payload["functions"], 3)
        self.assertEqual(payload["percent_of_lift_scope_bytes"], 40.0)
        # Bytes lead: the key order is what a reader of the JSON sees first.
        self.assertEqual(list(payload)[0], "bytes")

    def test_percent_is_rounded_the_way_the_reports_print_it(self):
        # 178248/2410317 is 7.395...%, which must publish as 7.4 and not as
        # 7.39 - the JSON and the printed table have to agree.
        self.assertEqual(Tally(1, 178248).percent_of(Tally(1, 2410317)), 7.4)


class BlockTests(unittest.TestCase):
    def setUp(self):
        self.rows = rows_of(STATES)
        self.block = metrics.bytes_block(self.rows)

    def test_the_denominator_is_stated_inside_the_block(self):
        denominator = self.block["denominator"]
        self.assertEqual(denominator["bytes"], 31)
        self.assertEqual(denominator["functions"], 5)
        self.assertIn("external_library", denominator["exclusion"])
        self.assertIn("lift_whole_image.py", denominator["exclusion"])

    def test_every_percentage_in_the_block_is_over_that_denominator(self):
        scope = self.block["denominator"]["bytes"]
        for state, payload in self.block["by_recovery_state"].items():
            if payload.get("excluded"):
                continue
            with self.subTest(state=state):
                self.assertEqual(
                    payload["percent_of_lift_scope_bytes"],
                    round(100.0 * payload["bytes"] / scope, 2))

    def test_the_excluded_state_is_marked_and_contributes_no_percentage(self):
        """It keeps its row - dropping it would make the table disagree with
        the catalogue - but it must not claim a share of a scope it is not in.
        """
        payload = self.block["by_recovery_state"]["external_library"]
        self.assertEqual(payload["bytes"], 32)
        self.assertEqual(payload["excluded"], {"bytes": 32, "functions": 1})
        self.assertEqual(payload["in_scope"], {"bytes": 0, "functions": 0})
        self.assertEqual(payload["percent_of_lift_scope_bytes"], 0.0)

    def test_the_in_scope_percentages_account_for_the_whole_scope(self):
        """EVERY grouped table, not just the one whose groups happen to line up
        with the exclusion.

        This test used to check `by_recovery_state` alone, and that is the one
        table where the bug could not appear: there, "excluded" IS a group, so
        zeroing the group and pricing only in-scope rows give the same answer.
        `by_binary_kind` cuts ACROSS the exclusion, and on the live catalogue it
        published 99.97 + 1.90 + 0.01 = 101.88% of a scope those three figures
        are presented as shares of.
        """
        for table in ("by_recovery_state", "by_binary_kind"):
            with self.subTest(table=table):
                total = sum(payload["percent_of_lift_scope_bytes"]
                            for payload in self.block[table].values())
                self.assertAlmostEqual(total, 100.0, places=1)

    def test_machine_carried_is_published_with_its_direction(self):
        carried = self.block["machine_carried"]
        self.assertEqual(carried["bytes"], 29)
        self.assertEqual(carried["direction"], "must go down")

    def test_by_binary_kind_prices_library_code_normally(self):
        """binary_kind is a description of the original, not a scope decision.

        Zeroing library bytes here as well would double-count the exclusion and
        make the kind table stop adding up to the catalogue.
        """
        self.assertEqual(self.block["by_binary_kind"]["library"]["bytes"], 32)


class MixedGroupTests(unittest.TestCase):
    """A group that straddles the exclusion, which is the live shape.

    `binary_kind == "library"` holds 338 rows: 327 excluded and 11 that carry a
    source annotation and are therefore in scope. Its byte figure must stay 338
    rows' worth so the table reconciles with the catalogue, and its percentage
    must be the 11 rows' worth, because that is all of it the denominator
    contains. Publishing 338 rows' bytes over a 5,673-row scope was the defect:
    45,686 bytes read as 1.90% of a scope holding 531 of them.

    The fixture keeps the two an order of magnitude apart so no rounding can
    make the wrong one look right.
    """

    def setUp(self):
        # 900 excluded library bytes, 100 in-scope library bytes, 900 game.
        self.rows = rows_of([
            ("0x00401000", 900, "external_library", "library"),
            ("0x00402000", 100, "source_complete", "library"),
            ("0x00403000", 900, "unrecovered", "game"),
        ])
        self.kinds = metrics.grouped(self.rows, "binary_kind",
                                     metrics.lift_scope(self.rows))

    def test_the_percentage_counts_only_the_rows_the_scope_contains(self):
        # Scope is 1,000 bytes. The library group carries 1,000 bytes of which
        # 100 are in scope: 10.00%, not the 100.00% the whole group would give.
        self.assertEqual(self.kinds["library"]["bytes"], 1000)
        self.assertEqual(self.kinds["library"]["functions"], 2)
        self.assertEqual(self.kinds["library"]["percent_of_lift_scope_bytes"],
                         10.0)

    def test_the_split_is_published_so_the_two_figures_reconcile(self):
        """`bytes` and the percentage describe different row sets here, so the
        object has to say so - otherwise a reader divides one by the scope and
        gets a number this file does not print anywhere."""
        self.assertEqual(self.kinds["library"]["in_scope"],
                         {"bytes": 100, "functions": 1})
        self.assertEqual(self.kinds["library"]["excluded"],
                         {"bytes": 900, "functions": 2 - 1})

    def test_a_group_with_nothing_excluded_carries_no_split(self):
        """The split is evidence of a straddle. Printing it everywhere would
        make it noise and stop it meaning anything."""
        self.assertNotIn("excluded", self.kinds["game"])
        self.assertNotIn("in_scope", self.kinds["game"])
        self.assertEqual(self.kinds["game"]["percent_of_lift_scope_bytes"],
                         90.0)

    def test_the_column_being_tabulated_does_not_decide_the_exclusion(self):
        """The old guard was `column != "recovery_state"`, so the exclusion was
        applied or not applied depending on WHICH TABLE was being built. It is a
        property of the rows: the same rows, grouped by any column, must leave
        the same bytes out of every percentage."""
        scope = metrics.lift_scope(self.rows)
        for column in ("binary_kind", "recovery_state", "name"):
            with self.subTest(column=column):
                table = metrics.grouped(self.rows, column, scope)
                self.assertAlmostEqual(
                    sum(payload["percent_of_lift_scope_bytes"]
                        for payload in table.values()), 100.0, places=1)
                self.assertEqual(
                    sum(payload["bytes"] for payload in table.values()),
                    metrics.catalogued(self.rows).byte_count)


class OneImplementationTwoCallersTests(unittest.TestCase):
    """The exporter's dicts and the CSV's rows must price identically.

    `export_recovery_inventory` builds dicts whose `size` is an int and never
    writes a CSV before summarising; `verify_recovery_metadata` re-reads the
    CSV, where every field is a string. They publish into the same key of the
    same file, so a difference between them is a difference between the full
    export and the reused-export path - two numbers for one tree.
    """

    def test_int_sizes_and_string_sizes_produce_the_same_block(self):
        from_csv = rows_of(STATES)
        in_memory = [{"size": size, "recovery_state": state,
                      "binary_kind": kind}
                     for _, size, state, kind in STATES]
        self.assertEqual(metrics.bytes_block(in_memory),
                         metrics.bytes_block(from_csv))
        self.assertEqual(metrics.bytes_block(in_memory)["denominator"]["bytes"],
                         31)


class ScopeSizesTests(unittest.TestCase):
    def test_out_of_scope_addresses_are_absent_rather_than_zero(self):
        """Absent and zero mean different things to the oracle summary.

        Absent says "the scope does not contain this address"; zero says "it
        does, and the catalogue could not price it". Collapsing the two would
        make a library row indistinguishable from an unpriced game row.
        """
        rows = rows_of([("0x00401000", 100, "unrecovered", "game"),
                        ("0x00402000", "n/a", "unrecovered", "game"),
                        ("0x00403000", 32, "external_library", "library")])
        sizes = metrics.scope_sizes(rows)
        self.assertEqual(sizes[0x00401000], 100)
        self.assertEqual(sizes[0x00402000], 0)
        self.assertNotIn(0x00403000, sizes)


class ProvenTests(unittest.TestCase):
    """`unproven_recovered`: the number that catches a WRONG recovery.

    `machine_carried` cannot. It falls the moment a function is declared
    recovered, whether or not the declaration is true, so a body that compiles
    and is wrong improves every published figure. These tests pin the one that
    does not.
    """

    def rows(self):
        return [
            {"address": "0x00400000", "size": "100",
             "recovery_state": "source_complete"},
            {"address": "0x00400100", "size": "200",
             "recovery_state": "source_complete"},
            {"address": "0x00400200", "size": "400",
             "recovery_state": "unrecovered"},
            {"address": "0x00400300", "size": "800",
             "recovery_state": "external_library"},
        ]

    def test_proven_and_unproven_split_the_recovered_bytes(self):
        proven = {0x00400000}
        self.assertEqual(100, metrics.proven_recovered(self.rows(), proven).byte_count)
        self.assertEqual(200, metrics.unproven_recovered(self.rows(), proven).byte_count)
        # They partition `recovered` exactly - no byte in both, none in neither.
        self.assertEqual(metrics.recovered(self.rows()).byte_count,
                         metrics.proven_recovered(self.rows(), proven).byte_count
                         + metrics.unproven_recovered(self.rows(), proven).byte_count)

    def test_proving_an_unrecovered_function_counts_for_nothing(self):
        # An oracle may cover a function that is NOT recovered - most of the
        # legacy-leaf islands do, since their whole purpose is to stand in for
        # code nobody has written yet. Counting those would inflate the metric
        # with the opposite of progress.
        proven = {0x00400200, 0x00400300}
        self.assertEqual(0, metrics.proven_recovered(self.rows(), proven).byte_count)
        self.assertEqual(300, metrics.unproven_recovered(self.rows(), proven).byte_count)

    def test_an_empty_proven_set_leaves_everything_unproven(self):
        self.assertEqual(0, metrics.proven_recovered(self.rows(), set()).byte_count)
        self.assertEqual(300, metrics.unproven_recovered(self.rows(), set()).byte_count)

    def test_the_percentage_names_the_denominator_it_used(self):
        # The founding error of this module was a percentage whose name stated
        # a denominator it was not computed over. These are shares of RECOVERED
        # bytes, not of the lift scope, and 100/300 is 33.33 either way only by
        # coincidence of this fixture - so the KEY is what is asserted.
        block = metrics.bytes_block(self.rows(), {0x00400000})
        self.assertIn("percent_of_recovered_bytes", block["unproven_recovered"])
        self.assertNotIn("percent_of_lift_scope_bytes", block["unproven_recovered"])
        self.assertEqual("recovered", block["unproven_recovered"]["denominator"])
        self.assertEqual(66.67, block["unproven_recovered"]["percent_of_recovered_bytes"])

    def test_it_is_declared_a_number_that_must_go_down(self):
        block = metrics.bytes_block(self.rows(), set())
        self.assertEqual("must go down", block["unproven_recovered"]["direction"])
        # And its sibling is not, because proving more is progress upward.
        self.assertNotIn("direction", block["proven_recovered"])

    def test_omitting_the_proven_set_does_not_silently_claim_coverage(self):
        # A caller that forgets to pass it must get 100% unproven, never 100%
        # proven: the safe default for an unknown is "not demonstrated".
        block = metrics.bytes_block(self.rows())
        self.assertEqual(0, block["proven_recovered"]["bytes"])
        self.assertEqual(300, block["unproven_recovered"]["bytes"])

    def test_the_inventory_row_shape_matches_too(self):
        # export_recovery_inventory builds rows with an int `start`; the CSV
        # has a hex-string `address`. Reading only one key published 100%
        # unproven from the inventory while the CSV said 32 functions, and
        # nothing failed - both are "valid" answers to a caller.
        rows = [{"start": 0x00400000, "size": 100,
                 "recovery_state": "source_complete"},
                {"start": 0x00400100, "size": 200,
                 "recovery_state": "source_complete"}]
        self.assertEqual(100, metrics.proven_recovered(rows, {0x00400000}).byte_count)
        self.assertEqual(200, metrics.unproven_recovered(rows, {0x00400000}).byte_count)

    def test_an_unreadable_address_is_unproven_rather_than_fatal(self):
        rows = self.rows() + [{"address": None, "size": "50",
                               "recovery_state": "source_complete"}]
        self.assertEqual(250, metrics.unproven_recovered(rows, {0x00400000}).byte_count)


class PublishedFileTests(unittest.TestCase):
    """The committed summary.json must describe the committed functions.csv.

    Both are regenerated together by `verify-recovery-metadata`, so a mismatch
    means one of them was hand-edited or half-regenerated - and the byte block
    would then be publishing a tree that no longer exists.
    """

    FUNCTIONS = REPO_ROOT / "docs" / "recovery" / "functions.csv"
    SUMMARY = REPO_ROOT / "docs" / "recovery" / "summary.json"

    def test_the_published_block_matches_the_published_catalogue(self):
        if not (self.FUNCTIONS.is_file() and self.SUMMARY.is_file()):
            self.skipTest("committed recovery catalogue not present")
        summary = json.loads(self.SUMMARY.read_text(encoding="utf-8"))
        published = summary["functions"].get("bytes")
        self.assertIsNotNone(
            published,
            "docs/recovery/summary.json has no byte-weighted block; "
            "regenerate it with verify-recovery-metadata")
        rows = metrics.load_catalogue(self.FUNCTIONS)
        # The proven set has to be passed. Calling bytes_block without it
        # computes 100% unproven, and a summary.json that ALSO said 100%
        # agreed with it - two wrong numbers matching, which is the one
        # outcome a comparison test must not accept.
        proven = self.load_proven()
        self.assertEqual(
            published, metrics.bytes_block(rows, proven),
            "docs/recovery/summary.json disagrees with functions.csv; "
            "one of them was regenerated without the other")

    def load_proven(self) -> set[int]:
        path = self.FUNCTIONS.parent / "proven.csv"
        if not path.is_file():
            return set()
        with path.open(newline="") as handle:
            return {int(row["address"], 16) for row in csv.DictReader(handle)}

    def test_the_published_proven_figure_is_not_trivially_zero(self):
        """A guard on the guard.

        `unproven_recovered` starting at 100% is a legitimate value, so a bug
        that silently produced 100% forever would look exactly like an honest
        starting point. This asserts the pipeline is wired: some function is
        proven, and the two halves add up to `recovered`.
        """
        if not (self.FUNCTIONS.is_file() and self.SUMMARY.is_file()):
            self.skipTest("committed recovery catalogue not present")
        block = json.loads(self.SUMMARY.read_text(encoding="utf-8"))
        block = block["functions"]["bytes"]
        self.assertGreater(block["proven_recovered"]["bytes"], 0,
                           "no recovered function is proven; the proven.csv "
                           "pipeline is not wired to summary.json")
        self.assertEqual(block["recovered"]["bytes"],
                         block["proven_recovered"]["bytes"]
                         + block["unproven_recovered"]["bytes"])

    def test_the_published_counts_agree_with_the_published_bytes(self):
        """The old count block and the new byte block describe one catalogue.

        They are computed separately - `Counter` in the exporter, `grouped`
        here - so a state present in one and missing from the other is exactly
        the drift this pair of tests exists to catch.
        """
        if not self.SUMMARY.is_file():
            self.skipTest("committed recovery summary not present")
        summary = json.loads(self.SUMMARY.read_text(encoding="utf-8"))
        functions = summary["functions"]
        published = functions.get("bytes")
        if published is None:
            self.skipTest("no byte-weighted block to compare")
        counts = functions["by_recovery_state"]
        byte_counts = {state: payload["functions"]
                       for state, payload in
                       published["by_recovery_state"].items()}
        self.assertEqual(counts, byte_counts)
        self.assertEqual(functions["total"], published["catalogued"]["functions"])


if __name__ == "__main__":
    unittest.main()
