#!/usr/bin/env python3
"""Unit tests for the whole-image oracle summary.

Every headline number this project publishes is printed by
`lifted_oracle_summary.py`: the byte-weighted verdict table, "compared and
agreed", the percent-of-image figure, and the seed histogram. Nothing else
recomputes them, so an arithmetic slip here is not caught downstream - it is
simply reported, and believed.

What is worth asserting is the handful of places where a wrong number is still
a *plausible* number - where the tool prints a well-formed table that means
something other than what the reader takes it to mean:

* BYTE WEIGHTING IS THE WHOLE POINT. "40% of functions is 8% of bytes" is this
  project's own measure, and a verdict on a few large functions must outrank a
  verdict on many small ones. Counting functions instead of bytes produces the
  same table shape with the columns telling the opposite story, so the tests
  assert the actual byte totals and the actual percentages, not the ordering.
* THE SIZE COLUMN IS A SUM OF SPANS, NOT AN EXTENT. A function's body can be
  split, so `end_address - address` over-counts it. This project has made that
  substitution once already; a fixture here has the two differ by 40x so the
  substitution cannot pass.
* "COMPARED AND AGREED" IS A STRICTLY SMALLER CLAIM than "the lifted body ran".
  A FAIL executed the lifted body and disagreed. The two lines sit next to each
  other and differ only in which verdicts they admit, so the fixture gives every
  verdict a distinct power-of-two size: any wrong subset yields a total no other
  subset can produce.
* THE SEED HISTOGRAM BUCKETS BY `compared`, NOT `cases`. Attempted and compared
  are different columns and only the second is evidence; a PASS resting on one
  seed of sixteen is the weakest row in the report and is exactly the row the
  histogram exists to surface. The fixture holds `cases` constant at 16 so a
  tool reading the wrong column collapses the whole histogram into one bucket.
* UNKNOWN VERDICTS MUST NOT VANISH. The verdict set grows (HANG and SKIP-trap
  both post-date the tool). A new verdict that silently dropped out of the table
  would leave the percentages summing to 100% of a smaller image.
* MALFORMED ROWS MUST NOT PRODUCE A PLAUSIBLE WRONG TOTAL. Truncated lines,
  duplicated boundary rows from a resumed sweep, a non-numeric size, and a last
  line with no trailing newline each perturb the total in a different direction,
  so the malformed fixture pins one exact total that only correct handling of
  all four can produce.

Two tests below pin behaviour that is DEFECTIVE rather than desired; both say so
in their own docstrings and name the defect. They are here because an untested
defect is how this file's numbers went wrong before.
"""

from __future__ import annotations

import io
import contextlib
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import lifted_oracle_summary as summary

NEW_HEADER = ["address", "verdict", "cases", "compared", "detail", "name"]
OLD_HEADER = ["address", "verdict", "cases", "detail", "name"]


def write_report(directory, rows, *, header=NEW_HEADER, trailing_newline=True,
                 newline="\n", name="report.tsv"):
    """A synthetic report.tsv. `rows` are lists of already-stringified fields.

    Rows are written verbatim, so a row may be deliberately short, deliberately
    not an address, or deliberately a repeat of an earlier one.
    """
    path = Path(directory) / name
    lines = ["\t".join(header)] + ["\t".join(row) for row in rows]
    text = newline.join(lines) + (newline if trailing_newline else "")
    path.write_text(text, newline="")
    return path


def write_functions(directory, entries, name="functions.csv"):
    """A synthetic functions.csv. `entries` are (address, size) or
    (address, size, recovery_state) or (address, size, state, end_address)."""
    path = Path(directory) / name
    out = ["address,end_address,size,name,recovery_state"]
    for entry in entries:
        address, size = entry[0], entry[1]
        state = entry[2] if len(entry) > 2 else "unrecovered"
        end = entry[3] if len(entry) > 3 else f"0x{int(address, 16) + 1:08X}"
        out.append(f"{address},{end},{size},fn_{address},{state}")
    path.write_text("\n".join(out) + "\n")
    return path


def run_summary(report_path, functions_path):
    """Run main() over the fixtures and return (exit code, stdout)."""
    captured = io.StringIO()
    old_argv = sys.argv
    old_functions = summary.FUNCTIONS
    sys.argv = ["lifted_oracle_summary.py", str(report_path)]
    summary.FUNCTIONS = Path(functions_path)
    try:
        with contextlib.redirect_stdout(captured):
            code = summary.main()
    finally:
        sys.argv = old_argv
        summary.FUNCTIONS = old_functions
    return code, captured.getvalue()


def verdict_table(out):
    """{verdict: (functions, bytes, "pct%")} parsed back out of the table."""
    table = {}
    for line in out.splitlines():
        parts = line.split()
        if len(parts) == 4 and parts[3].endswith("%") and parts[1].isdigit():
            table[parts[0]] = (int(parts[1]), int(parts[2]), parts[3])
    return table


def verdict_order(out):
    """The verdicts in the order the table prints them."""
    order = []
    for line in out.splitlines():
        parts = line.split()
        if len(parts) == 4 and parts[3].endswith("%") and parts[1].isdigit():
            order.append(parts[0])
    return order


def line_starting(testcase, out, prefix):
    """The single output line starting with `prefix`."""
    hits = [line for line in out.splitlines() if line.startswith(prefix)]
    testcase.assertEqual(len(hits), 1, f"expected one {prefix!r} line in:\n{out}")
    return hits[0]


class ByteWeightingTests(unittest.TestCase):
    """Two big PASSes against twenty small FAILs.

    Counting functions says FAIL is 90.91% of the work and PASS 9.09%.
    Counting bytes says exactly the reverse. The numbers are chosen so the two
    readings are each other's mirror image: no test that asserts only ordering,
    and no test that asserts only one of the two columns, can tell them apart.
    """

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        rows = [["0x00401000", "PASS", "16", "16", "", "big_a"],
                ["0x00402000", "PASS", "16", "16", "", "big_b"]]
        entries = [("0x00401000", "5000"), ("0x00402000", "5000")]
        for i in range(20):
            address = f"0x0041{i:04x}"
            rows.append([address, "FAIL", "16", "16", "eax 1 != 2", f"sm_{i}"])
            entries.append((address, "50"))
        self.report = write_report(self.tmp.name, rows)
        self.functions = write_functions(self.tmp.name, entries)

    def test_byte_totals_and_percentages(self):
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        table = verdict_table(out)
        # 2 x 5000 bytes of PASS against 20 x 50 bytes of FAIL.
        self.assertEqual(table["PASS"], (2, 10000, "90.91%"))
        self.assertEqual(table["FAIL"], (20, 1000, "9.09%"))
        self.assertIn("22 functions, 11000 bytes", out)

    def test_few_large_outranks_many_small(self):
        # The percentages, not just their order: 10000/11000 and 1000/11000.
        # Function counting gives the same pair of percentages swapped over.
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        pass_bytes = verdict_table(out)["PASS"][1]
        fail_bytes = verdict_table(out)["FAIL"][1]
        self.assertGreater(pass_bytes, fail_bytes)
        self.assertEqual(pass_bytes, 10 * fail_bytes)
        # ...while the table's own row order is by function count, so PASS is
        # printed second. The reader is meant to take the bytes column, and a
        # test that read the order would read the wrong story.
        self.assertEqual(verdict_order(out), ["FAIL", "PASS"])

    def test_headline_percentages_are_byte_weighted(self):
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        executed = line_starting(self, out, "lifted body executed")
        agreed = line_starting(self, out, "compared and agreed")
        # Everything here ran, so executed is the whole image; agreed is the
        # two big functions only.
        self.assertIn("22 functions, 11000 bytes (100.00% of image bytes)",
                      executed)
        self.assertIn("2 functions, 10000 bytes (90.91% of image bytes)",
                      agreed)


class SizeColumnTests(unittest.TestCase):
    def test_size_is_the_sum_of_spans_not_the_address_extent(self):
        """A chunked body's size is smaller than end_address - address.

        The function below spans 0x1000 bytes of address range but owns only
        100 of them; the rest belongs to other functions interleaved in the
        range. Weighting by the extent inflates it 40x.
        """
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        report = write_report(
            tmp.name, [["0x00401000", "PASS", "16", "16", "", "chunked"]])
        functions = write_functions(
            tmp.name, [("0x00401000", "100", "unrecovered", "0x00402000")])
        code, out = run_summary(report, functions)
        self.assertEqual(code, 0)
        self.assertIn("1 functions, 100 bytes", out)
        self.assertEqual(verdict_table(out)["PASS"], (1, 100, "100.00%"))

    def test_external_library_functions_contribute_no_bytes(self):
        """Library code is not the project's to recover, so it is not image.

        It still appears as a function - dropping the row entirely would make
        the verdict table disagree with the report it summarises.
        """
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        report = write_report(tmp.name, [
            ["0x00401000", "PASS", "16", "16", "", "ours"],
            ["0x00402000", "PASS", "16", "16", "", "crt_memcpy"],
        ])
        functions = write_functions(tmp.name, [
            ("0x00401000", "100"),
            ("0x00402000", "9999", "external_library"),
        ])
        code, out = run_summary(report, functions)
        self.assertEqual(code, 0)
        self.assertIn("2 functions, 100 bytes", out)


class AgreedVersusExecutedTests(unittest.TestCase):
    """Which verdicts count as "the lifted body ran" and which as "agreed".

    Every verdict gets a distinct power of two, so each candidate subset sums to
    a unique total: 100 alone is PASS-only, 300 admits FAIL, 1500 is the four
    executed verdicts, 6300 is everything. A wrong membership test cannot
    accidentally land on the right number.
    """

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        spec = [
            ("0x00401000", "PASS", 100),
            ("0x00402000", "FAIL", 200),
            ("0x00403000", "FAIL-lifted-fault", 400),
            ("0x00404000", "INCONCLUSIVE-lifted-timeout", 800),
            ("0x00405000", "INCONCLUSIVE-original-fault", 1600),
            ("0x00406000", "SKIP", 3200),
        ]
        rows = [[a, v, "16", "16", "detail text", f"fn_{a}"] for a, v, _ in spec]
        self.report = write_report(self.tmp.name, rows)
        self.functions = write_functions(
            self.tmp.name, [(a, str(s)) for a, _, s in spec])

    def test_agreed_counts_pass_only(self):
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        agreed = line_starting(self, out, "compared and agreed")
        # 100 bytes is PASS alone. 300 would mean FAIL is being called
        # agreement, which is the exact opposite of what FAIL means.
        self.assertIn("1 functions, 100 bytes (1.59% of image bytes)", agreed)
        self.assertNotIn("300 bytes", agreed)

    def test_executed_counts_fail_as_well_as_pass(self):
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        executed = line_starting(self, out, "lifted body executed")
        # PASS + FAIL + FAIL-lifted-fault + INCONCLUSIVE-lifted-timeout.
        self.assertIn("4 functions, 1500 bytes (23.81% of image bytes)",
                      executed)

    def test_agreed_is_strictly_smaller_than_executed(self):
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        executed = line_starting(self, out, "lifted body executed")
        agreed = line_starting(self, out, "compared and agreed")
        self.assertNotEqual(executed.split(":", 1)[1], agreed.split(":", 1)[1])
        self.assertIn("1500 bytes", executed)
        self.assertIn("100 bytes", agreed)

    def test_skip_and_original_fault_never_executed_the_lifted_body(self):
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        executed = line_starting(self, out, "lifted body executed")
        # 3200 (SKIP) and 1600 (INCONCLUSIVE-original-fault) are the two
        # verdicts where the lifted side never ran; neither may be inside the
        # 1500. Assert the totals they would produce are absent.
        self.assertNotIn("3100", executed)
        self.assertNotIn("4700", executed)
        self.assertNotIn("6300", executed)
        self.assertIn("6300 bytes", out)  # ...but they are in the image total.


class SeedHistogramTests(unittest.TestCase):
    """`cases` is held at 16 for every row so only `compared` can distinguish.

    Three PASSes rest on a single seed, one on three, one on all sixteen. A
    histogram keyed on `cases` would print a single bucket of five and report
    zero single-seed PASSes - a report that looks healthier than the data.
    """

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        spec = [
            ("0x00401000", "PASS", "16", "1"),
            ("0x00402000", "PASS", "16", "1"),
            ("0x00403000", "PASS", "16", "1"),
            ("0x00404000", "PASS", "16", "3"),
            ("0x00405000", "PASS", "16", "16"),
            # Not a PASS: its compared count is not evidence of agreement and
            # must not enter the histogram.
            ("0x00406000", "FAIL", "16", "1"),
            ("0x00407000", "INCONCLUSIVE-original-fault", "16", "0"),
        ]
        rows = [[a, v, c, n, "detail", f"fn_{a}"] for a, v, c, n in spec]
        self.report = write_report(self.tmp.name, rows)
        self.functions = write_functions(
            self.tmp.name, [(a, "100") for a, _, _, _ in spec])

    def test_buckets_are_keyed_on_compared_not_cases(self):
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        self.assertIn("   1 seeds compared:     3 (60.0%)", out)
        self.assertIn("   3 seeds compared:     1 (20.0%)", out)
        self.assertIn("  16 seeds compared:     1 (20.0%)", out)
        # Keying on `cases` would put all five PASSes in the 16 bucket.
        self.assertNotIn("  16 seeds compared:     5", out)

    def test_single_seed_bucket_is_counted_and_surfaced(self):
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        arrow = line_starting(self, out, "  -> ")
        self.assertIn("-> 3 PASSes rest on a single seed", arrow)
        # Off by one in either direction is the failure mode that matters:
        # the single-seed bucket is the weakest evidence in the report.
        self.assertNotIn("-> 2 PASSes", arrow)
        self.assertNotIn("-> 4 PASSes", arrow)

    def test_histogram_admits_passes_only(self):
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        # The FAIL row also compared 1 seed. If it were bucketed the single-seed
        # count would read 4, and the percentages would be over a PASS
        # denominator of 5 - i.e. 80%, a figure with no meaning.
        self.assertNotIn("   1 seeds compared:     4", out)
        self.assertNotIn("(80.0%)", out)
        # The INCONCLUSIVE row compared 0 seeds; a 0 bucket must not exist.
        self.assertNotIn("   0 seeds compared", out)

    def test_percentages_are_over_the_pass_count(self):
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        # 3/5 and 1/5, not 3/7 (42.9%) or 1/7 (14.3%) over all rows.
        self.assertIn("(60.0%)", out)
        self.assertNotIn("(42.9%)", out)
        self.assertNotIn("(14.3%)", out)

    def test_no_histogram_section_when_the_report_has_no_compared_column(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        report = write_report(
            tmp.name, [["0x00401000", "PASS", "16", "", "old_fn"]],
            header=OLD_HEADER)
        functions = write_functions(tmp.name, [("0x00401000", "100")])
        code, out = run_summary(report, functions)
        self.assertEqual(code, 0)
        # Without the column there is no evidence about seeds, so the section
        # must be absent rather than invented from `cases`.
        self.assertNotIn("seeds compared", out)
        self.assertNotIn("rest on a single seed", out)

    def test_attempted_denominator_is_max_compared_a_known_understatement(self):
        """DEFECT, pinned deliberately: "out of N attempted" reads N from the
        `compared` column, not from `cases`.

        Here every row attempted 16 seeds and the best row compared 3, so the
        header claims 3 were attempted. It is right on the real report only
        because some function there compares all 16. If you fix this to read
        `cases`, this test SHOULD fail - update it, do not delete it.
        """
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        rows = [["0x00401000", "PASS", "16", "1", "", "a"],
                ["0x00402000", "PASS", "16", "3", "", "b"]]
        report = write_report(tmp.name, rows)
        functions = write_functions(
            tmp.name, [("0x00401000", "100"), ("0x00402000", "100")])
        code, out = run_summary(report, functions)
        self.assertEqual(code, 0)
        self.assertIn("(out of 3 attempted)", out)
        self.assertNotIn("(out of 16 attempted)", out)


class UnknownVerdictTests(unittest.TestCase):
    def test_a_verdict_the_tool_has_never_seen_still_appears(self):
        """HANG and SKIP-trap both post-date this tool; the next one will too.

        A verdict that fell out of the table would not just lose its own row -
        it would shrink the denominator every other percentage is over.
        """
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        report = write_report(tmp.name, [
            ["0x00401000", "PASS", "16", "16", "", "known"],
            ["0x00402000", "MOONBEAM", "16", "0", "who knows", "novel"],
        ])
        functions = write_functions(
            tmp.name, [("0x00401000", "250"), ("0x00402000", "750")])
        code, out = run_summary(report, functions)
        self.assertEqual(code, 0)
        table = verdict_table(out)
        self.assertIn("MOONBEAM", table)
        self.assertEqual(table["MOONBEAM"], (1, 750, "75.00%"))
        self.assertIn("2 functions, 1000 bytes", out)
        # And it is in the denominator: PASS is 25% of 1000, not 100% of 250.
        self.assertEqual(table["PASS"], (1, 250, "25.00%"))

    def test_unknown_verdict_does_not_count_as_executed_or_agreed(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        report = write_report(tmp.name, [
            ["0x00401000", "PASS", "16", "16", "", "known"],
            ["0x00402000", "MOONBEAM", "16", "0", "who knows", "novel"],
        ])
        functions = write_functions(
            tmp.name, [("0x00401000", "250"), ("0x00402000", "750")])
        code, out = run_summary(report, functions)
        self.assertEqual(code, 0)
        self.assertIn("1 functions, 250 bytes",
                      line_starting(self, out, "lifted body executed"))
        self.assertIn("1 functions, 250 bytes",
                      line_starting(self, out, "compared and agreed"))

    def test_verdict_percentages_account_for_the_whole_image(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        report = write_report(tmp.name, [
            ["0x00401000", "PASS", "16", "16", "", "a"],
            ["0x00402000", "MOONBEAM", "16", "0", "x", "b"],
            ["0x00403000", "SKIP", "0", "0", "reaches an import", "c"],
        ])
        functions = write_functions(tmp.name, [
            ("0x00401000", "250"), ("0x00402000", "750"), ("0x00403000", "1000")])
        code, out = run_summary(report, functions)
        self.assertEqual(code, 0)
        table = verdict_table(out)
        self.assertEqual(sum(v[1] for v in table.values()), 2000)
        total = sum(float(v[2].rstrip("%")) for v in table.values())
        self.assertAlmostEqual(total, 100.0, places=2)


class MalformedRowTests(unittest.TestCase):
    """One fixture, four defects, one exact total.

    Sizes are chosen so each mishandling lands somewhere different:
      correct                        -> 3 functions, 1007 bytes
      short row accepted             -> 4 functions, 5007 bytes
      duplicate boundary row counted -> 4 functions, 2007 bytes
      unterminated last line dropped -> 2 functions, 1000 bytes
      non-numeric size row dropped   -> 2 functions, 1007 bytes
    No two agree, so "1007" is not a total a broken parser can reach by luck.
    """

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        rows = [
            ["0x00401000", "PASS", "16", "16", "", "first"],
            # Truncated by a killed sweep: three fields, no verdict detail.
            ["0x00402000", "SKIP", "0"],
            # A resumed sweep re-emits the boundary row it was killed on, this
            # time with a different verdict. It is the same function.
            ["0x00401000", "FAIL", "16", "16", "eax 1 != 2", "first"],
            # Present in the report, but functions.csv cannot say how big.
            ["0x00403000", "PASS", "16", "16", "", "sizeless"],
            # ...and this last row is written with no trailing newline.
            ["0x00404000", "PASS", "16", "16", "", "last"],
        ]
        self.report = write_report(self.tmp.name, rows, trailing_newline=False)
        self.functions = write_functions(self.tmp.name, [
            ("0x00401000", "1000"),
            ("0x00402000", "4000"),
            ("0x00403000", "n/a"),
            ("0x00404000", "7"),
        ])

    def test_one_exact_total_survives_all_four_defects(self):
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        self.assertIn("3 functions, 1007 bytes", out)
        for wrong in ("5007 bytes", "2007 bytes", "4 functions", "2 functions"):
            self.assertNotIn(wrong, out)

    def test_short_row_is_dropped_not_half_read(self):
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        self.assertNotIn("SKIP", verdict_table(out))

    def test_duplicate_boundary_row_does_not_double_count(self):
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        # The first verdict wins and the repeat is ignored entirely - it must
        # not appear as a FAIL, and it must not add its 1000 bytes again.
        self.assertNotIn("FAIL", verdict_table(out))
        self.assertEqual(verdict_table(out)["PASS"][0], 3)

    def test_unterminated_last_line_is_still_a_row(self):
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        # 7 bytes is the whole contribution of the last row; without it the
        # total is a round 1000 and looks entirely reasonable.
        self.assertIn("1007 bytes", out)
        self.assertIn("  16 seeds compared:     3", out)

    def test_non_numeric_size_contributes_zero_and_keeps_its_row(self):
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        # Three PASS rows, one of which the size table cannot price.
        self.assertEqual(verdict_table(out)["PASS"], (3, 1007, "100.00%"))

    def test_non_address_lines_are_skipped(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        report = write_report(tmp.name, [
            ["0x00401000", "PASS", "16", "16", "", "real"],
            # A sweep log line that landed in the report, and a footer.
            ["-- resuming after hang --", "", "", "", "", ""],
            ["TOTAL", "PASS", "16", "16", "", "summary"],
        ])
        functions = write_functions(tmp.name, [("0x00401000", "100")])
        code, out = run_summary(report, functions)
        self.assertEqual(code, 0)
        self.assertIn("1 functions, 100 bytes", out)

    def test_missing_report_refuses_rather_than_printing_zeroes(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        functions = write_functions(tmp.name, [("0x00401000", "100")])
        missing = Path(tmp.name) / "not-here.tsv"
        code, out = run_summary(missing, functions)
        # A zero table is indistinguishable from a swept image that agreed on
        # nothing, so the tool must fail loudly instead of printing one.
        self.assertEqual(code, 2)
        self.assertEqual(out, "")

    def test_crlf_report_parses_without_carriage_returns_in_names(self):
        """The real report is written under Wine, so its lines end \\r\\n.

        The name is the last field, so an untranslated \\r rides along on every
        function name printed in the failure list.
        """
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        report = write_report(tmp.name, [
            ["0x00401000", "FAIL", "16", "16", "eax 1 != 2", "crlf_fn"],
        ], newline="\r\n")
        functions = write_functions(tmp.name, [("0x00401000", "100")])
        code, out = run_summary(report, functions)
        self.assertEqual(code, 0)
        self.assertIn("1 functions, 100 bytes", out)
        self.assertIn("FAIL  crlf_fn\n", out)
        self.assertNotIn("\r", out)


class FailureListTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        rows = [
            ["0x00401000", "FAIL", "16", "16", "eax 1 != 2; edx 3 != 4", "tiny"],
            ["0x00402000", "FAIL-lifted-fault", "16", "0", "code 0xc0000005",
             "huge"],
            ["0x00403000", "PASS", "16", "16", "", "fine"],
        ]
        self.report = write_report(self.tmp.name, rows)
        self.functions = write_functions(self.tmp.name, [
            ("0x00401000", "10"), ("0x00402000", "900"), ("0x00403000", "50")])

    def test_failures_are_listed_largest_first(self):
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        # Byte weighting again: the 900-byte failure is the one that matters,
        # and address order would have put the 10-byte one first.
        self.assertLess(out.index("huge"), out.index("tiny"))

    def test_failure_header_totals_only_failing_bytes(self):
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        self.assertIn("2 FAILING functions (910 bytes)", out)

    def test_detail_column_is_read_by_header_shape(self):
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        # Split on "; " onto separate lines, from column 4 of the new format.
        self.assertIn("      eax 1 != 2\n", out)
        self.assertIn("      edx 3 != 4\n", out)
        # Reading a fixed column index would print the name as the detail.
        self.assertNotIn("      tiny\n", out)

    def test_detail_column_moves_left_in_the_old_header_shape(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        report = write_report(tmp.name, [
            ["0x00401000", "FAIL", "16", "eax 1 != 2", "old_fn"],
        ], header=OLD_HEADER)
        functions = write_functions(tmp.name, [("0x00401000", "10")])
        code, out = run_summary(report, functions)
        self.assertEqual(code, 0)
        self.assertIn("      eax 1 != 2\n", out)
        self.assertNotIn("      old_fn\n", out)


class DenominatorTests(unittest.TestCase):
    def test_percent_of_image_is_percent_of_the_report(self):
        """STRUCTURAL HAZARD, pinned deliberately: the denominator is the rows
        present in the report, not the image in functions.csv.

        CORRECTION. An earlier version of this docstring claimed the live
        figure was an overstatement because "the report holds 5,673 of 6,000
        functions, so the denominator is missing 327". That claim is FALSE and
        was measured to be false: the 327 rows absent from the report are
        *exactly* the 327 `external_library` functions, and `function_sizes`
        already prices those at zero. sum(size) over the non-external image and
        sum(size) over the report rows are the same number, 2410317, so today's
        published percentages are over the whole non-library image and are not
        inflated.

        What is still true, and is what this test pins, is the SHAPE: the
        denominator is derived from whatever rows the report happens to hold.
        A sweep that died half way through therefore prints percentages of the
        part it managed to sweep, and they read exactly like percentages of the
        image. Here the report covers 100 bytes of a 100,100-byte image and the
        tool prints 100.00%. If you make the denominator the image, this test
        SHOULD fail - update it, do not delete it.
        """
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        report = write_report(
            tmp.name, [["0x00401000", "PASS", "16", "16", "", "swept"]])
        functions = write_functions(tmp.name, [
            ("0x00401000", "100"), ("0x00402000", "100000")])
        code, out = run_summary(report, functions)
        self.assertEqual(code, 0)
        self.assertIn("1 functions, 100 bytes", out)
        self.assertIn("100.00% of image bytes",
                      line_starting(self, out, "compared and agreed"))

    def test_an_all_zero_image_does_not_divide_by_zero(self):
        """Every reported function missing from functions.csv is a live risk -
        a stale CSV silently prices the whole sweep at zero.

        The percentages must read 0.00%, never 100.00%. (The header line does
        print "1 bytes" rather than 0, from the `or 1` divide-by-zero guard;
        that is a separate cosmetic defect, not asserted here.)
        """
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        report = write_report(tmp.name, [
            ["0x00401000", "PASS", "16", "16", "", "a"],
            ["0x00402000", "FAIL", "16", "16", "eax 1 != 2", "b"],
        ])
        functions = write_functions(tmp.name, [("0x00409000", "500")])
        code, out = run_summary(report, functions)
        self.assertEqual(code, 0)
        self.assertEqual(verdict_table(out)["PASS"], (1, 0, "0.00%"))
        self.assertIn("0 bytes (0.00% of image bytes)",
                      line_starting(self, out, "compared and agreed"))
        self.assertNotIn("100.00%", out)

    def test_empty_report_prints_an_empty_table_and_succeeds(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        report = write_report(tmp.name, [])
        functions = write_functions(tmp.name, [("0x00401000", "100")])
        code, out = run_summary(report, functions)
        self.assertEqual(code, 0)
        self.assertIn("0 functions", out)
        self.assertEqual(verdict_table(out), {})
        self.assertNotIn("seeds compared", out)
        self.assertNotIn("FAILING functions", out)


class RecoveryStateTests(unittest.TestCase):
    """`external_library` is the ONLY state that costs a function its bytes.

    `function_sizes` prices the image by filtering on one string. Every other
    value of `recovery_state` must be priced normally - and there are five of
    them in the real functions.csv, not one. Measured on the live report, the
    states no earlier fixture ever used carry 29.86% of the published PASS byte
    total:

        unrecovered          574 fn  125021 B  70.14%   (the only one covered)
        source_complete      865 fn   50383 B  28.27%
        original_dependency   21 fn    2819 B   1.58%
        thunk                  5 fn      25 B   0.01%

    So widening the filter by one string - `in ("external_library",
    "source_complete")` - moved "compared and agreed" from 7.40% to 5.77% of
    image bytes with the whole suite green. That is a headline number, changed
    by a one-word edit, behind an unasserted branch. Every state gets a
    distinct power of two below so no wrong filter can sum to the right total.
    """

    # The six values that actually occur in docs/recovery/functions.csv.
    STATES = [
        ("0x00401000", 1, "unrecovered"),
        ("0x00402000", 2, "source_complete"),
        ("0x00403000", 4, "source_in_progress"),
        ("0x00404000", 8, "original_dependency"),
        ("0x00405000", 16, "thunk"),
        ("0x00406000", 32, "external_library"),
    ]

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.report = write_report(self.tmp.name, [
            [a, "PASS", "16", "16", "", f"fn_{a}"] for a, _, _ in self.STATES])
        self.functions = write_functions(
            self.tmp.name, [(a, str(s), st) for a, s, st in self.STATES])

    def test_only_external_library_is_priced_at_zero(self):
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        # 1+2+4+8+16 = 31. Excluding any other single state gives 30, 29, 27,
        # 23 or 15; excluding none gives 63. Only 31 is right.
        self.assertIn("6 functions, 31 bytes", out)
        self.assertEqual(verdict_table(out)["PASS"], (6, 31, "100.00%"))

    def test_each_non_library_state_carries_its_bytes(self):
        """One state at a time, so the failure names the state that broke."""
        for address, size, state in self.STATES:
            with self.subTest(state=state):
                tmp = tempfile.TemporaryDirectory()
                self.addCleanup(tmp.cleanup)
                report = write_report(tmp.name, [
                    [address, "PASS", "16", "16", "", "subject"]])
                functions = write_functions(tmp.name, [
                    (address, str(size), state)])
                code, out = run_summary(report, functions)
                self.assertEqual(code, 0)
                expected = 0 if state == "external_library" else size
                self.assertIn(f"1 functions, {expected} bytes", out)

    def test_an_unknown_recovery_state_is_priced_not_dropped(self):
        """A state added to functions.csv later must not silently cost bytes.

        The filter is a positive test for one string, so a new state prices
        normally. Asserting it keeps that direction: an implementation that
        allow-listed known states instead would drop the new one to zero and
        quietly shrink the image.
        """
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        report = write_report(tmp.name, [
            ["0x00401000", "PASS", "16", "16", "", "novel"]])
        functions = write_functions(tmp.name, [
            ("0x00401000", "777", "state_invented_next_quarter")])
        code, out = run_summary(report, functions)
        self.assertEqual(code, 0)
        self.assertIn("1 functions, 777 bytes", out)


class FailureListMembershipTests(unittest.TestCase):
    """The failure list admits FAIL* and nothing else.

    Widening the test to `startswith("FAIL") or startswith("INCONCLUSIVE")`
    took the live report from "13 FAILING functions (2166 bytes)" to "447
    FAILING functions (86856 bytes)" with the whole suite green: a 34x
    inflation of the number the project uses to decide what to fix next. An
    INCONCLUSIVE is the absence of evidence, not evidence of a divergence, and
    the two must never be pooled.

    Every non-failing verdict below carries a distinct power of two so the
    "(N bytes)" header pins exactly which rows were admitted.
    """

    NOT_FAILURES = [
        ("0x00411000", "PASS", 1),
        ("0x00412000", "SKIP", 2),
        ("0x00413000", "INCONCLUSIVE-original-fault", 4),
        ("0x00414000", "INCONCLUSIVE-original-timeout", 8),
        ("0x00415000", "INCONCLUSIVE-original-top-page", 16),
        ("0x00416000", "INCONCLUSIVE-lifted-timeout", 32),
        ("0x00417000", "INCONCLUSIVE-lifted-out-of-span", 64),
        ("0x00418000", "SKIP-trap", 128),
        ("0x00419000", "HANG", 256),
    ]
    FAILURES = [
        ("0x00401000", "FAIL", 1000),
        ("0x00402000", "FAIL-lifted-fault", 2000),
    ]

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        spec = self.FAILURES + self.NOT_FAILURES
        self.report = write_report(self.tmp.name, [
            [a, v, "16", "16", "detail text", f"fn_{a}"] for a, v, _ in spec])
        self.functions = write_functions(
            self.tmp.name, [(a, str(s)) for a, _, s in spec])

    def test_only_fail_verdicts_reach_the_failure_list(self):
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        # 3000 is FAIL + FAIL-lifted-fault. Admitting the INCONCLUSIVEs adds
        # 124; admitting everything adds 511. Neither can reach 3000.
        self.assertIn("2 FAILING functions (3000 bytes)", out)

    def test_no_inconclusive_function_is_named_as_a_failure(self):
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        listing = out.split("FAILING functions", 1)[1]
        for address, verdict, _ in self.NOT_FAILURES:
            with self.subTest(verdict=verdict):
                self.assertNotIn(f"fn_{address}", listing)
        for address, verdict, _ in self.FAILURES:
            with self.subTest(verdict=verdict):
                self.assertIn(f"fn_{address}", listing)


class ExecutedMembershipTests(unittest.TestCase):
    """"lifted body executed" admits exactly four verdict shapes.

    PASS, FAIL, FAIL-lifted-fault, and anything INCONCLUSIVE-lifted-*: those
    are the verdicts where the lifted body demonstrably ran. Adding HANG to the
    set moved the live figure from 7.49% to 7.68% with every test green,
    because no earlier fixture named a real verdict the tool does not know.
    A HANG is a supervisor kill; nothing is known about what ran.

    Distinct powers of two again, so the total names the membership.
    """

    SPEC = [
        ("0x00401000", "PASS", 1),
        ("0x00402000", "FAIL", 2),
        ("0x00403000", "FAIL-lifted-fault", 4),
        ("0x00404000", "INCONCLUSIVE-lifted-timeout", 8),
        ("0x00405000", "INCONCLUSIVE-lifted-out-of-span", 16),
        # ...and everything below never got the lifted body off the ground.
        ("0x00406000", "HANG", 32),
        ("0x00407000", "SKIP", 64),
        ("0x00408000", "SKIP-trap", 128),
        ("0x00409000", "INCONCLUSIVE-original-fault", 256),
        ("0x0040a000", "INCONCLUSIVE-original-timeout", 512),
        ("0x0040b000", "INCONCLUSIVE-original-top-page", 1024),
    ]

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.report = write_report(self.tmp.name, [
            [a, v, "16", "16", "d", f"fn_{a}"] for a, v, _ in self.SPEC])
        self.functions = write_functions(
            self.tmp.name, [(a, str(s)) for a, _, s in self.SPEC])

    def test_executed_is_exactly_the_five_lifted_side_verdicts(self):
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        executed = line_starting(self, out, "lifted body executed")
        # 1+2+4+8+16 = 31 of 2047 total.
        self.assertIn("5 functions, 31 bytes", executed)
        self.assertIn("2047 bytes", out)

    def test_a_hang_did_not_execute_the_lifted_body(self):
        """HANG is the supervisor giving up, not a run.

        Adding it to the set gives 63 bytes - still a plausible-looking
        number, which is why the total is asserted and not the ordering.
        """
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        executed = line_starting(self, out, "lifted body executed")
        self.assertNotIn("63 bytes", executed)
        self.assertNotIn("6 functions", executed)

    def test_each_excluded_verdict_is_excluded_on_its_own(self):
        for address, verdict, size in self.SPEC:
            executed_side = verdict in (
                "PASS", "FAIL", "FAIL-lifted-fault") or verdict.startswith(
                    "INCONCLUSIVE-lifted")
            with self.subTest(verdict=verdict):
                tmp = tempfile.TemporaryDirectory()
                self.addCleanup(tmp.cleanup)
                report = write_report(tmp.name, [
                    [address, verdict, "16", "16", "d", "subject"]])
                functions = write_functions(tmp.name, [(address, str(size))])
                code, out = run_summary(report, functions)
                self.assertEqual(code, 0)
                executed = line_starting(self, out, "lifted body executed")
                want = f"{1 if executed_side else 0} functions, " \
                       f"{size if executed_side else 0} bytes"
                self.assertIn(want, executed)


class RowShapeTests(unittest.TestCase):
    """Where the "is this a row" boundary actually sits.

    The guard is `len(parts) < 4`. Nothing pinned the boundary, so tightening
    it to `< 5` - which silently drops a whole class of row - was invisible.
    """

    def test_a_four_field_row_is_the_shortest_row_accepted(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        report = write_report(tmp.name, [
            ["0x00401000", "PASS", "16", "16"],          # exactly four
            ["0x00402000", "PASS", "16"],                # three: too short
            ["0x00403000", "PASS", "16", "16", "", "ok"],
        ])
        functions = write_functions(tmp.name, [
            ("0x00401000", "1"), ("0x00402000", "2"), ("0x00403000", "4")])
        code, out = run_summary(report, functions)
        self.assertEqual(code, 0)
        # 5 = the four-field row plus the full row. Dropping the four-field row
        # gives 4; accepting the three-field row gives 7.
        self.assertIn("2 functions, 5 bytes", out)

    def test_a_truncated_failure_row_still_crashes_the_summary(self):
        """DEFECT, pinned: `len(parts) < 4` admits rows the failure list then
        indexes past.

        A FAIL row truncated to four fields passes the guard, and
        `detail = parts[4]` raises IndexError - so a single truncated line
        yields no numbers at all rather than slightly wrong ones. Not reachable
        on today's report (all 5,673 live rows have all six fields, measured),
        so this is pinned rather than fixed: changing it is a decision about
        what a half-written line means, not a refactor. If you make truncated
        failure rows survive, this test SHOULD fail - update it, do not delete
        it.
        """
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        report = write_report(tmp.name, [["0x00401000", "FAIL", "16", "16"]])
        functions = write_functions(tmp.name, [("0x00401000", "10")])
        with self.assertRaises(IndexError):
            run_summary(report, functions)

    def test_two_functions_sharing_a_name_are_two_functions(self):
        """Dedup is keyed on address; a name is not an identity.

        The image has repeated names - the same static helper compiled into
        several translation units - so collapsing on name would delete real
        rows and shrink the denominator.
        """
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        report = write_report(tmp.name, [
            ["0x00401000", "PASS", "16", "16", "", "same_name"],
            ["0x00402000", "PASS", "16", "16", "", "same_name"],
        ])
        functions = write_functions(tmp.name, [
            ("0x00401000", "300"), ("0x00402000", "40")])
        code, out = run_summary(report, functions)
        self.assertEqual(code, 0)
        self.assertIn("2 functions, 340 bytes", out)
        self.assertEqual(verdict_table(out)["PASS"], (2, 340, "100.00%"))


class TableOrderTests(unittest.TestCase):
    """The verdict table is ordered by function count, descending.

    ByteWeightingTests asserts the order is ["FAIL", "PASS"] - but "FAIL"
    sorts before "PASS" alphabetically too, so that assertion passes just as
    happily against `sorted(counts.items())` and does not test the property its
    comment names. Here PASS is the more numerous verdict, so count order and
    alphabetical order disagree and only one of them can pass.
    """

    def test_row_order_is_by_function_count_not_by_name(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        rows = [[f"0x0041{i:04x}", "PASS", "16", "16", "", f"p{i}"]
                for i in range(5)]
        rows.append(["0x00420000", "FAIL", "16", "16", "eax 1 != 2", "f0"])
        report = write_report(tmp.name, rows)
        functions = write_functions(
            tmp.name, [(row[0], "10") for row in rows])
        code, out = run_summary(report, functions)
        self.assertEqual(code, 0)
        # Alphabetical would print FAIL first. Count order prints PASS first.
        self.assertEqual(verdict_order(out), ["PASS", "FAIL"])


if __name__ == "__main__":
    unittest.main()
