#!/usr/bin/env python3
"""Unit tests for the whole-image oracle summary.

Every headline number this project publishes about the lift is printed by
`lifted_oracle_summary.py`: the byte-weighted verdict table, "compared and
agreed", the evidence tiers under it, the three numbers that must go down, and
the seed histogram. Nothing else recomputes them, so an arithmetic slip here is
not caught downstream - it is simply reported, and believed.

What is worth asserting is the handful of places where a wrong number is still
a *plausible* number - where the tool prints a well-formed table that means
something other than what the reader takes it to mean:

* BYTE WEIGHTING IS THE WHOLE POINT. "40% of functions is 8% of bytes" is this
  project's own measure, and a verdict on a few large functions must outrank a
  verdict on many small ones. Counting functions instead of bytes produces the
  same table shape with the columns telling the opposite story, so the tests
  assert the actual byte totals and the actual percentages, not the ordering.
* THE DENOMINATOR IS THE CATALOGUE, NOT THE REPORT. This is the flattering
  failure that used to be pinned here as a hazard: percentages over "whatever
  rows the sweep managed to write" read exactly like percentages of the image,
  so a sweep that died half way through published a coverage figure of the part
  it finished. `swept` and `NOT SWEPT` now make a partial run visible, and
  PartialSweepTests holds the tool to it.
* "COMPARED AND AGREED" IS A STRICTLY SMALLER CLAIM than "the lifted body ran".
  A FAIL executed the lifted body and disagreed. The two lines sit next to each
  other and differ only in which verdicts they admit, so the fixture gives every
  verdict a distinct power-of-two size: any wrong subset yields a total no other
  subset can produce.
* AND "FULL-STRENGTH AGREEMENT" IS SMALLER AGAIN. A PASS resting on one seed of
  sixteen is the weakest row in the report; it used to be a footnote under the
  headline and is now a tier of it. The tiers partition the agreeing bytes, so
  the tests assert they add up as well as that each is right - a row counted
  twice or dropped shows as an arithmetic failure, not just a wrong label.
* THE SEED HISTOGRAM BUCKETS BY `compared`, NOT `cases`. Attempted and compared
  are different columns and only the second is evidence. The fixture holds
  `cases` constant at 16 so a tool reading the wrong column collapses the whole
  histogram into one bucket.
* THE NUMBERS THAT MUST GO DOWN MUST NOT BE ABLE TO GO UP BY ACCIDENT. A
  dashboard where every figure rises cannot report a regression, so each of the
  three is asserted against a fixture where the flattering mistake produces a
  different number.
* UNKNOWN VERDICTS MUST NOT VANISH. The verdict set grows (HANG and SKIP-trap
  both post-date the tool). A new verdict that silently dropped out of the table
  would leave the percentages summing to 100% of a smaller image.
* MALFORMED ROWS MUST NOT PRODUCE A PLAUSIBLE WRONG TOTAL. Truncated lines,
  duplicated boundary rows from a resumed sweep, a non-numeric size, and a last
  line with no trailing newline each perturb the total in a different direction,
  so the malformed fixture pins one exact swept total that only correct handling
  of all four can produce.
"""

from __future__ import annotations

import io
import contextlib
import json
import re
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import lifted_oracle_summary as summary

NEW_HEADER = ["address", "verdict", "cases", "compared", "detail", "name"]
OLD_HEADER = ["address", "verdict", "cases", "detail", "name"]

# The caveat text `oracle_pass_caveat` writes onto an arbitrated PASS row.
CAVEAT = ("passed on the seeds that could be judged; 2 case(s) thrown out - "
          "the original read the unmodellable top 64 KiB")


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
    out = ["address,end_address,size,name,binary_kind,recovery_state"]
    for entry in entries:
        address, size = entry[0], entry[1]
        state = entry[2] if len(entry) > 2 else "unrecovered"
        end = entry[3] if len(entry) > 3 else f"0x{int(address, 16) + 1:08X}"
        out.append(f"{address},{end},{size},fn_{address},game,{state}")
    path.write_text("\n".join(out) + "\n")
    return path


def run_summary(report_path, functions_path, *extra):
    """Run main() over the fixtures and return (exit code, stdout)."""
    captured = io.StringIO()
    old_argv = sys.argv
    old_functions = summary.FUNCTIONS
    sys.argv = ["lifted_oracle_summary.py", str(report_path), *extra]
    summary.FUNCTIONS = Path(functions_path)
    try:
        with contextlib.redirect_stdout(captured):
            code = summary.main()
    finally:
        sys.argv = old_argv
        summary.FUNCTIONS = old_functions
    return code, captured.getvalue()


def verdict_table(out):
    """{verdict: (functions, bytes, "pct%")} parsed back out of the table.

    The printed column order is bytes, percent, functions - bytes lead - but
    the tuple keeps functions first so that every assertion below states the
    count and the bytes explicitly and neither can be read as the other.
    """
    table = {}
    for line in out.splitlines():
        parts = line.split()
        if (len(parts) == 4 and parts[1].isdigit()
                and parts[2].endswith("%") and parts[3].isdigit()):
            table[parts[0]] = (int(parts[3]), int(parts[1]), parts[2])
    return table


def verdict_order(out):
    """The verdicts in the order the table prints them."""
    order = []
    for line in out.splitlines():
        parts = line.split()
        if (len(parts) == 4 and parts[1].isdigit()
                and parts[2].endswith("%") and parts[3].isdigit()):
            order.append(parts[0])
    return order


def line_starting(testcase, out, prefix):
    """The single output line starting with `prefix`."""
    hits = [line for line in out.splitlines() if line.startswith(prefix)]
    testcase.assertEqual(len(hits), 1, f"expected one {prefix!r} line in:\n{out}")
    return hits[0]


FIGURE_RE = re.compile(r"(\d+) B\s+([\d.]+%) of scope\s+(\d+) fn")


def figure(testcase, out, prefix):
    """(bytes, "pct%", functions) off one of the published figure lines."""
    match = FIGURE_RE.search(line_starting(testcase, out, prefix))
    testcase.assertIsNotNone(match, f"no figure on the {prefix!r} line:\n{out}")
    return int(match.group(1)), match.group(2), int(match.group(3))


SCOPE_RE = re.compile(r"lift scope (\d+) bytes across (\d+) functions")
SWEPT_RE = re.compile(r"swept:\s+(\d+) bytes across (\d+) functions "
                      r"\(([\d.]+%) of scope bytes\)")


def denominator(testcase, out):
    """(bytes, functions) of the stated denominator."""
    match = SCOPE_RE.search(out)
    testcase.assertIsNotNone(match, f"no denominator line in:\n{out}")
    return int(match.group(1)), int(match.group(2))


def swept(testcase, out):
    """(bytes, functions, "pct%") of what the report actually covered."""
    match = SWEPT_RE.search(out)
    testcase.assertIsNotNone(match, f"no swept line in:\n{out}")
    return int(match.group(1)), int(match.group(2)), match.group(3)


def tier_table(out):
    """{tier: (functions, bytes, "pct of agreed")} from the evidence table."""
    table = {}
    for line in out.splitlines():
        parts = line.split()
        if (len(parts) > 4 and parts[0] in summary.TIER_ORDER
                and parts[1].isdigit() and parts[2].endswith("%")
                and parts[3].isdigit()):
            table[parts[0]] = (int(parts[3]), int(parts[1]), parts[2])
    return table


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
        self.assertEqual(denominator(self, out), (11000, 22))

    def test_few_large_outranks_many_small(self):
        # The percentages, not just their order: 10000/11000 and 1000/11000.
        # Function counting gives the same pair of percentages swapped over.
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        pass_bytes = verdict_table(out)["PASS"][1]
        fail_bytes = verdict_table(out)["FAIL"][1]
        self.assertGreater(pass_bytes, fail_bytes)
        self.assertEqual(pass_bytes, 10 * fail_bytes)
        # ...and the table's row order follows the bytes, so the two big
        # PASSes are printed above the twenty small FAILs. Ordering by function
        # count - which is what this table used to do - reverses it.
        self.assertEqual(verdict_order(out), ["PASS", "FAIL"])

    def test_headline_percentages_are_byte_weighted(self):
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        # Everything here ran, so executed is the whole scope; agreed is the
        # two big functions only.
        self.assertEqual(figure(self, out, "lifted body executed"),
                         (11000, "100.00%", 22))
        self.assertEqual(figure(self, out, "compared and agreed"),
                         (10000, "90.91%", 2))


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
        self.assertEqual(denominator(self, out), (100, 1))
        self.assertEqual(verdict_table(out)["PASS"], (1, 100, "100.00%"))

    def test_external_library_functions_are_outside_the_denominator(self):
        """Library code is not the project's to recover, so it is not image.

        It still appears as a row of the verdict table - dropping it entirely
        would make the table disagree with the report it summarises - but it is
        priced at zero, it is not in the denominator, and the fact that the
        report contained a row the scope does not is stated rather than hidden.
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
        self.assertEqual(denominator(self, out), (100, 1))
        self.assertEqual(verdict_table(out)["PASS"], (2, 100, "100.00%"))
        self.assertIn("NOT PRICED: 1 report rows", out)
        # ...and the swept figure counts one function, not two: a row outside
        # the denominator cannot be part of what was swept out of it.
        self.assertEqual(swept(self, out), (100, 1, "100.00%"))


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
        # 100 bytes is PASS alone. 300 would mean FAIL is being called
        # agreement, which is the exact opposite of what FAIL means.
        self.assertEqual(figure(self, out, "compared and agreed"),
                         (100, "1.59%", 1))

    def test_executed_counts_fail_as_well_as_pass(self):
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        # PASS + FAIL + FAIL-lifted-fault + INCONCLUSIVE-lifted-timeout.
        self.assertEqual(figure(self, out, "lifted body executed"),
                         (1500, "23.81%", 4))

    def test_agreed_is_strictly_smaller_than_executed(self):
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        executed = figure(self, out, "lifted body executed")
        agreed = figure(self, out, "compared and agreed")
        self.assertNotEqual(executed, agreed)
        self.assertEqual(executed[0], 1500)
        self.assertEqual(agreed[0], 100)

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
        self.assertEqual(denominator(self, out), (6300, 6))  # ...but in scope.

    def test_never_compared_is_the_complement_of_pass_and_fail(self):
        """The must-go-down mirror of "compared and agreed".

        Only PASS and FAIL ran both sides on a seed, matching
        `OracleFoldState::compared`. 6300 - 100 - 200 = 6000. Admitting
        FAIL-lifted-fault as compared gives 5600; admitting the whole executed
        set gives 4800. Neither can reach 6000.
        """
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        self.assertEqual(figure(self, out, "  never compared"),
                         (6000, "95.24%", 4))


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
        # must be absent rather than invented from `cases`. The evidence tiers
        # go with it: unqualified agreement must never be inferred.
        self.assertNotIn("seeds compared", out)
        self.assertNotIn("rest on a single seed", out)
        self.assertNotIn("evidence quality", out)
        self.assertNotIn("FULL-STRENGTH", out)

    def test_attempted_denominator_is_the_cases_column(self):
        """"out of N attempted" reads `cases`, the seeds the sweep tried.

        It used to read the largest `compared` seen anywhere in the report,
        which is right only by luck - only while some function happens to
        compare every seed. Here every row attempted 16 and the best compared
        3, so the old reading claimed 3 were attempted and made every line of
        the histogram look like a larger fraction of the effort than it was.
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
        self.assertIn("(out of 16 attempted)", out)
        self.assertNotIn("(out of 3 attempted)", out)


class EvidenceTierTests(unittest.TestCase):
    """Every PASS carries its own evidence quality, and the tiers partition it.

    The report's own qualification - "119 PASSes rest on a single seed" - was a
    footnote under a headline that did not mention it. Byte-weighted it is not
    a footnote at all: on the live report the full-strength tier is 767 of
    1,465 functions but only 42,236 of 178,248 bytes, so three quarters of the
    agreeing bytes rest on weakened evidence. The fixture gives each tier a
    distinct power of two so the tier totals name the membership exactly.
    """

    SPEC = [
        # address, cases, compared, detail, size
        ("0x00401000", "16", "1", "", 1),          # single seed
        ("0x00402000", "16", "1", CAVEAT, 2),      # single seed AND arbitrated
        ("0x00403000", "16", "13", CAVEAT, 4),     # arbitrated
        ("0x00404000", "16", "15", "", 8),         # seeds incomplete
        ("0x00405000", "16", "16", "", 16),        # full strength
    ]

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.report = write_report(self.tmp.name, [
            [a, "PASS", c, n, d, f"fn_{a}"] for a, c, n, d, _ in self.SPEC])
        self.functions = write_functions(
            self.tmp.name, [(a, str(s)) for a, _, _, _, s in self.SPEC])

    def test_each_tier_holds_exactly_the_rows_it_names(self):
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        table = tier_table(out)
        # 1+2 single-seat, 4 arbitrated, 8 incomplete, 16 full. Any row moving
        # between tiers changes two totals at once and both are asserted.
        self.assertEqual(table["single-seed"][1], 3)
        self.assertEqual(table["arbitrated"][1], 4)
        self.assertEqual(table["seeds-incomplete"][1], 8)
        self.assertEqual(table["full-strength"][1], 16)

    def test_a_row_that_is_both_single_seed_and_arbitrated_reads_as_weaker(self):
        """Severity order, not convenience order.

        0x00402000 compared one seed AND had cases thrown out. Reporting it as
        arbitrated would understate it: "some cases were thrown out" sounds
        like most were judged, and one was.
        """
        self.assertEqual(summary.evidence_tier(16, 1, CAVEAT),
                         summary.TIER_SINGLE_SEED)
        self.assertEqual(summary.evidence_tier(16, 13, CAVEAT),
                         summary.TIER_ARBITRATED)

    def test_the_tiers_add_up_to_the_agreeing_bytes(self):
        """A partition, not a set of overlapping labels.

        If a row were counted in two tiers, or dropped from all of them, the
        headline and the table would disagree - and the table is the part a
        reader checks least.
        """
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        table = tier_table(out)
        agreed_bytes, _, agreed_functions = figure(
            self, out, "compared and agreed")
        self.assertEqual(sum(v[1] for v in table.values()), agreed_bytes)
        self.assertEqual(sum(v[0] for v in table.values()), agreed_functions)
        self.assertEqual(agreed_bytes, 31)

    def test_the_headline_states_the_full_strength_share(self):
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        # 16 of 31 bytes, over a 31-byte scope. The qualification travels with
        # the number instead of sitting under it.
        self.assertEqual(figure(self, out, "  ...of which FULL-STRENGTH"),
                         (16, "51.61%", 1))

    def test_weakened_agreement_is_every_tier_but_full_strength(self):
        """The must-go-down figure is the whole qualified set, not its worst
        tier. Publishing only the single-seed rows would report 3 bytes of
        weakened agreement where there are 15, and the number would still fall
        as the project improved - it would just start four fifths too low.
        """
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        table = tier_table(out)
        weak = figure(self, out, "  agreed under a weakened")
        self.assertEqual(weak[0], 15)   # 1 + 2 + 4 + 8 of 31 bytes
        self.assertEqual(weak[2], 4)
        self.assertEqual(weak[0], sum(value[1] for tier, value in table.items()
                                      if tier != summary.TIER_FULL))
        self.assertNotEqual(weak[0], table[summary.TIER_SINGLE_SEED][1])

    def test_full_strength_is_a_strictly_smaller_claim_than_agreed(self):
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        agreed = figure(self, out, "compared and agreed")
        full = figure(self, out, "  ...of which FULL-STRENGTH")
        self.assertLess(full[0], agreed[0])
        self.assertLess(full[2], agreed[2])

    def test_a_missing_detail_column_cannot_promote_a_row_to_full_strength(self):
        """The one way a truncated line could flatter, and why it cannot.

        Arbitrated cases increment `ran` and never `compared`, so any
        arbitration forces compared < cases. `compared == cases` therefore
        proves nothing was thrown out without reading the caveat at all, and a
        row too short to carry a caveat still cannot claim full strength unless
        its own numbers earn it.
        """
        self.assertEqual(summary.evidence_tier(16, 15, ""),
                         summary.TIER_SEEDS_INCOMPLETE)
        self.assertEqual(summary.evidence_tier(16, 16, ""),
                         summary.TIER_FULL)
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        report = write_report(tmp.name, [["0x00401000", "PASS", "16", "15"]])
        functions = write_functions(tmp.name, [("0x00401000", "500")])
        code, out = run_summary(report, functions)
        self.assertEqual(code, 0)
        self.assertEqual(tier_table(out)["seeds-incomplete"][1], 500)
        self.assertEqual(tier_table(out)["full-strength"][1], 0)


class MustGoDownTests(unittest.TestCase):
    """The three figures that fall when the project succeeds.

    Nothing published before this reported a number that could go down, so a
    regression had nowhere to show. Each is asserted against a fixture where
    the flattering mistake produces a different, still-plausible number.
    """

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.report = write_report(self.tmp.name, [
            ["0x00401000", "PASS", "16", "16", "", "recovered_and_agreed"],
            ["0x00402000", "PASS", "16", "1", "", "lifted_and_agreed_weakly"],
            ["0x00403000", "SKIP", "0", "0", "reaches an import", "untouched"],
        ])
        self.functions = write_functions(self.tmp.name, [
            ("0x00401000", "100", "source_complete"),
            ("0x00402000", "200", "unrecovered"),
            ("0x00403000", "700", "unrecovered"),
            # Never swept, never lifted, and five times the size of everything
            # else - so a debt published over the CATALOGUE instead of over the
            # lift scope reads 15.00% where the truth is 90.00%.
            ("0x00404000", "5000", "external_library"),
        ])

    def test_machine_carried_counts_everything_that_is_not_recovered_source(self):
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        # 900 of 1000 bytes. Counting the agreeing bytes as recovered would
        # give 700; counting nothing as recovered would give 1000.
        self.assertEqual(figure(self, out, "  carried by machine-derived"),
                         (900, "90.00%", 2))

    def test_agreeing_with_the_oracle_is_not_recovering(self):
        """A PASS is evidence about the lift, not a line of recovered source.

        0x00402000 agrees with the original on every seed and is still carried
        by machine-derived code. If agreement moved bytes out of the debt, the
        debt would fall without anyone writing anything.
        """
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        carried = figure(self, out, "  carried by machine-derived")
        agreed = figure(self, out, "compared and agreed")
        self.assertEqual(carried[0], 900)
        self.assertEqual(agreed[0], 300)
        self.assertGreater(carried[0] + agreed[0], 1000)  # they overlap

    def test_weakened_agreement_is_the_complement_of_full_strength(self):
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        agreed = figure(self, out, "compared and agreed")
        full = figure(self, out, "  ...of which FULL-STRENGTH")
        weak = figure(self, out, "  agreed under a weakened")
        self.assertEqual(full[0] + weak[0], agreed[0])
        self.assertEqual(full[2] + weak[2], agreed[2])
        # 200 of the 300 agreeing bytes rest on a single seed.
        self.assertEqual(weak, (200, "20.00%", 1))

    def test_never_compared_includes_the_bytes_no_sweep_reached(self):
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        # Only the SKIP, 700 bytes. A tool that counted "not PASS" would say
        # 700 as well - so the fixture is checked from the other side in
        # AgreedVersusExecutedTests, where FAIL makes the two differ.
        self.assertEqual(figure(self, out, "  never compared"),
                         (700, "70.00%", 1))


class PartialSweepTests(unittest.TestCase):
    """A sweep that did not finish must not read like one that did.

    This is the failure that used to be pinned here as a structural hazard: the
    denominator was the rows the report happened to hold, so half a sweep
    printed percentages of the half it managed and they looked exactly like
    percentages of the image.
    """

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.report = write_report(
            self.tmp.name, [["0x00401000", "PASS", "16", "16", "", "swept"]])
        self.functions = write_functions(self.tmp.name, [
            ("0x00401000", "100"), ("0x00402000", "100000")])

    def test_percent_of_image_is_percent_of_the_catalogue(self):
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        self.assertEqual(denominator(self, out), (100100, 2))
        # 100 of 100,100 bytes. Over the report's own rows this reads 100.00%.
        self.assertEqual(figure(self, out, "compared and agreed"),
                         (100, "0.10%", 1))
        # Over the report's own rows every coverage line here reads 100.00%.
        # The debt lines legitimately do - nothing is recovered - so the check
        # is on the coverage lines by name, not on the whole page.
        for prefix in ("compared and agreed", "lifted body executed",
                       "  ...of which FULL-STRENGTH"):
            self.assertNotEqual(figure(self, out, prefix)[1], "100.00%")

    def test_the_unswept_remainder_is_stated(self):
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        self.assertEqual(swept(self, out), (100, 1, "0.10%"))
        self.assertIn("NOT SWEPT: 100000 bytes across 1 functions", out)

    def test_a_complete_sweep_says_so_and_says_nothing_more(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        report = write_report(
            tmp.name, [["0x00401000", "PASS", "16", "16", "", "swept"]])
        functions = write_functions(tmp.name, [("0x00401000", "100")])
        code, out = run_summary(report, functions)
        self.assertEqual(code, 0)
        self.assertEqual(swept(self, out), (100, 1, "100.00%"))
        self.assertNotIn("NOT SWEPT", out)
        self.assertNotIn("NOT PRICED", out)

    def test_an_all_zero_image_does_not_divide_by_zero(self):
        """Every reported function missing from functions.csv is a live risk -
        a stale CSV silently prices the whole sweep at zero.

        The percentages must read 0.00%, never 100.00%.
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
        self.assertEqual(verdict_table(out)["FAIL"], (1, 0, "0.00%"))
        self.assertIn("NOT PRICED: 2 report rows", out)
        # Neither row is in the scope, so nothing was swept out of it.
        self.assertEqual(swept(self, out), (0, 0, "0.00%"))
        # No coverage figure may claim a share of a scope it is not in - and
        # none may report a NEGATIVE function count either, which is what
        # subtracting out-of-scope rows from an in-scope total produces.
        for prefix in ("compared and agreed", "lifted body executed",
                       "  never compared", "  carried by machine-derived"):
            with self.subTest(prefix=prefix):
                self.assertGreaterEqual(figure(self, out, prefix)[2], 0)
        self.assertEqual(figure(self, out, "compared and agreed"),
                         (0, "0.00%", 0))
        self.assertEqual(figure(self, out, "  never compared"),
                         (500, "100.00%", 1))

    def test_empty_report_prints_an_empty_table_and_succeeds(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        report = write_report(tmp.name, [])
        functions = write_functions(tmp.name, [("0x00401000", "100")])
        code, out = run_summary(report, functions)
        self.assertEqual(code, 0)
        self.assertEqual(swept(self, out), (0, 0, "0.00%"))
        self.assertEqual(verdict_table(out), {})
        self.assertNotIn("seeds compared", out)
        self.assertNotIn("FAILING functions", out)
        self.assertNotIn("evidence quality", out)

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

    def test_the_exclusion_is_stated_where_the_number_is_published(self):
        """Two totals were in play - 6,000 catalogued rows and 5,673 planned -
        and neither published figure said which it was over."""
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        self.assertIn("external_library", out)
        self.assertIn("lift_whole_image.py", out)


class UnknownVerdictTests(unittest.TestCase):
    def test_a_verdict_the_tool_has_never_seen_still_appears(self):
        """HANG and SKIP-trap both post-date this tool; the next one will too.

        A verdict that fell out of the table would not just lose its own row -
        it would shrink the swept total and hide bytes from every figure.
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
        self.assertEqual(denominator(self, out), (1000, 2))
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
        self.assertEqual(figure(self, out, "lifted body executed"),
                         (250, "25.00%", 1))
        self.assertEqual(figure(self, out, "compared and agreed"),
                         (250, "25.00%", 1))
        # ...and its bytes land in the number that must go down.
        self.assertEqual(figure(self, out, "  never compared"),
                         (750, "75.00%", 1))

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
    """One fixture, four defects, one exact swept total.

    Sizes are chosen so each mishandling lands somewhere different:
      correct                        -> 3 functions, 1007 bytes
      short row accepted             -> 4 functions, 5007 bytes
      duplicate boundary row counted -> 4 functions, 2007 bytes
      unterminated last line dropped -> 2 functions, 1000 bytes
      non-numeric size row dropped   -> 2 functions, 1007 bytes
    No two agree, so "1007" is not a total a broken parser can reach by luck.
    The denominator is the whole catalogue, 5007 bytes, and is asserted apart
    from the swept figure so the two cannot be confused for each other.
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
        self.assertEqual(swept(self, out)[:2], (1007, 3))
        self.assertEqual(denominator(self, out), (5007, 4))

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
        # swept total is a round 1000 and looks entirely reasonable.
        self.assertEqual(swept(self, out)[0], 1007)
        self.assertIn("  16 seeds compared:     3", out)

    def test_non_numeric_size_contributes_zero_and_keeps_its_row(self):
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        # Three PASS rows, one of which the size table cannot price. The row is
        # in scope, so it is swept and it is not reported as unpriced.
        self.assertEqual(verdict_table(out)["PASS"], (3, 1007, "20.11%"))
        self.assertNotIn("NOT PRICED", out)

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
        self.assertEqual(swept(self, out)[:2], (100, 1))

    def test_crlf_report_parses_without_carriage_returns_in_names(self):
        """The real report is written under Wine, so its lines end \\r\\n.

        The name is the last field, so an untranslated \\r rides along on every
        function name printed in the failure list. The tool relies on universal
        newlines and on nothing else - opening the report with `newline=""` is
        the damage this test exists to catch.
        """
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        report = write_report(tmp.name, [
            ["0x00401000", "FAIL", "16", "16", "eax 1 != 2", "crlf_fn"],
        ], newline="\r\n")
        functions = write_functions(tmp.name, [("0x00401000", "100")])
        code, out = run_summary(report, functions)
        self.assertEqual(code, 0)
        self.assertEqual(swept(self, out)[:2], (100, 1))
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


class RecoveryStateTests(unittest.TestCase):
    """`external_library` is the ONLY state that costs a function its bytes.

    The scope is priced by filtering on one string. Every other value of
    `recovery_state` must be priced normally - and there are five of them in
    the real functions.csv, not one. Measured on the live report, the states no
    earlier fixture ever used carry 29.86% of the published PASS byte total:

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
        self.assertEqual(denominator(self, out), (31, 5))
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
                external = state == "external_library"
                self.assertEqual(denominator(self, out),
                                 (0, 0) if external else (size, 1))

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
        self.assertEqual(denominator(self, out), (777, 1))


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
    """"lifted body executed" admits exactly the verdicts where it really ran.

    PASS, PASS-paths-taken, FAIL, FAIL-lifted-fault, SKIP-reached-blocked, and
    anything INCONCLUSIVE-lifted-*. Adding HANG to the set moved the live
    figure from 7.49% to 7.68% with every test green, because no earlier
    fixture named a real verdict the tool does not know. A HANG is a supervisor
    kill; nothing is known about what ran.

    SKIP-reached-blocked is the counter-intuitive member and it is here on
    purpose: it is the one SKIP whose lifted body DID run, all the way up to
    the construct the lift cannot model. That is the entire difference between
    it and the static refusal it is named after.

    The SPEC below is the ONLY copy of the membership in this class. It was not
    always - the exclusion rule was also spelled out by hand in the last test,
    and when the verdict set grew by two the hand-written copy silently stopped
    covering them, so the new verdicts rode in the fixture without ever being
    asserted about. The rule is now read off the SPEC.

    Distinct powers of two again, so the total names the membership.
    """

    SPEC = [
        ("0x00401000", "PASS", True, 1),
        ("0x00402000", "FAIL", True, 2),
        ("0x00403000", "FAIL-lifted-fault", True, 4),
        ("0x00404000", "INCONCLUSIVE-lifted-timeout", True, 8),
        ("0x00405000", "INCONCLUSIVE-lifted-out-of-span", True, 16),
        ("0x0040c000", "PASS-paths-taken", True, 2048),
        ("0x0040d000", "SKIP-reached-blocked", True, 4096),
        # ...and everything below never got the lifted body off the ground.
        ("0x00406000", "HANG", False, 32),
        ("0x00407000", "SKIP", False, 64),
        ("0x00408000", "SKIP-trap", False, 128),
        ("0x00409000", "INCONCLUSIVE-original-fault", False, 256),
        ("0x0040a000", "INCONCLUSIVE-original-timeout", False, 512),
        ("0x0040b000", "INCONCLUSIVE-original-top-page", False, 1024),
    ]

    EXECUTED_BYTES = sum(size for _, _, executed, size in SPEC if executed)
    TOTAL_BYTES = sum(size for _, _, _, size in SPEC)

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.report = write_report(self.tmp.name, [
            [a, v, "16", "16", "d", f"fn_{a}"] for a, v, _, _ in self.SPEC])
        self.functions = write_functions(
            self.tmp.name, [(a, str(s)) for a, _, _, s in self.SPEC])

    def test_executed_is_exactly_the_lifted_side_verdicts(self):
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        # 1+2+4+8+16+2048+4096 = 6175 of 8191 total. Any wrong subset of
        # thirteen distinct powers of two lands on a total no right one can.
        self.assertEqual(self.EXECUTED_BYTES, 6175)
        self.assertEqual(figure(self, out, "lifted body executed")[0],
                         self.EXECUTED_BYTES)
        self.assertEqual(figure(self, out, "lifted body executed")[2], 7)
        self.assertEqual(denominator(self, out), (self.TOTAL_BYTES, 13))

    def test_a_hang_did_not_execute_the_lifted_body(self):
        """HANG is the supervisor giving up, not a run.

        Adding it to the set gives 6207 bytes - still a plausible-looking
        number, which is why the total is asserted and not the ordering.
        """
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        executed = figure(self, out, "lifted body executed")
        self.assertNotEqual(executed[0], self.EXECUTED_BYTES + 32)
        self.assertNotEqual(executed[2], 8)

    def test_the_skip_that_ran_is_not_grouped_with_the_skips_that_did_not(self):
        """SKIP-reached-blocked in, SKIP and SKIP-trap out.

        Deciding this on the `SKIP` prefix - the obvious implementation - drops
        4,096 of the 6,175 bytes; deciding it on "not a SKIP" drops the same
        4,096. Only naming the three separately gives the total.
        """
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        executed = figure(self, out, "lifted body executed")[0]
        self.assertEqual(executed, self.EXECUTED_BYTES)
        self.assertNotEqual(executed, self.EXECUTED_BYTES - 4096)
        self.assertNotEqual(executed, self.EXECUTED_BYTES + 64 + 128)

    def test_each_excluded_verdict_is_excluded_on_its_own(self):
        for address, verdict, executed_side, size in self.SPEC:
            with self.subTest(verdict=verdict):
                tmp = tempfile.TemporaryDirectory()
                self.addCleanup(tmp.cleanup)
                report = write_report(tmp.name, [
                    [address, verdict, "16", "16", "d", "subject"]])
                functions = write_functions(tmp.name, [(address, str(size))])
                code, out = run_summary(report, functions)
                self.assertEqual(code, 0)
                executed = figure(self, out, "lifted body executed")
                self.assertEqual(executed[0], size if executed_side else 0)
                self.assertEqual(executed[2], 1 if executed_side else 0)


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
        self.assertEqual(swept(self, out)[:2], (5, 2))

    def test_a_truncated_failure_row_survives_with_no_detail(self):
        """A half-written line yields incomplete detail, never no numbers.

        This used to raise IndexError: `detail = parts[4]` on a row the length
        guard had already admitted, so one truncated line anywhere in a
        5,673-row report produced a traceback instead of a summary. The row now
        keeps its verdict, its bytes and its place in the failure list, and
        simply has nothing to say about why it failed.
        """
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        report = write_report(tmp.name, [["0x00401000", "FAIL", "16", "16"]])
        functions = write_functions(tmp.name, [("0x00401000", "10")])
        code, out = run_summary(report, functions)
        self.assertEqual(code, 0)
        self.assertIn("1 FAILING functions (10 bytes)", out)
        self.assertEqual(verdict_table(out)["FAIL"], (1, 10, "100.00%"))

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
        self.assertEqual(swept(self, out)[:2], (340, 2))
        self.assertEqual(verdict_table(out)["PASS"], (2, 340, "100.00%"))


class TableOrderTests(unittest.TestCase):
    """The verdict table is ordered by BYTES, descending.

    Three orderings are possible and all three look reasonable in a printed
    table, so the fixture makes them disagree with each other: by bytes gives
    FAIL, SKIP, PASS; alphabetically gives FAIL, PASS, SKIP; by function count
    gives PASS first. Only one of the three can pass.
    """

    def test_row_order_is_by_bytes_not_by_count_and_not_by_name(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        rows = [[f"0x0041{i:04x}", "PASS", "16", "16", "", f"p{i}"]
                for i in range(5)]
        rows.append(["0x00420000", "FAIL", "16", "16", "eax 1 != 2", "f0"])
        rows.append(["0x00430000", "SKIP", "0", "0", "reaches an import", "s0"])
        report = write_report(tmp.name, rows)
        sizes = {"0x00420000": "900", "0x00430000": "400"}
        functions = write_functions(
            tmp.name, [(row[0], sizes.get(row[0], "10")) for row in rows])
        code, out = run_summary(report, functions)
        self.assertEqual(code, 0)
        self.assertEqual(verdict_order(out), ["FAIL", "SKIP", "PASS"])


class JsonRecordTests(unittest.TestCase):
    """The machine-readable record must say the same thing as the print-out.

    Two renderings of one computation is how a dashboard drifts from its
    source. The record exists so that two sweeps can be diffed - which is the
    only way a number that must go down can be seen not to.
    """

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.report = write_report(self.tmp.name, [
            ["0x00401000", "PASS", "16", "16", "", "full"],
            ["0x00402000", "PASS", "16", "1", "", "weak"],
            ["0x00403000", "SKIP", "0", "0", "reaches an import", "skipped"],
            ["0x00404000", "PASS-paths-taken", "16", "16", "", "paths"],
        ])
        self.functions = write_functions(self.tmp.name, [
            ("0x00401000", "100", "source_complete"),
            ("0x00402000", "200", "unrecovered"),
            ("0x00403000", "700", "unrecovered"),
            ("0x00404000", "4000", "unrecovered"),
        ])
        self.json_path = Path(self.tmp.name) / "record.json"

    def record(self):
        code, out = run_summary(self.report, self.functions,
                                "--json", str(self.json_path))
        self.assertEqual(code, 0)
        return json.loads(self.json_path.read_text()), out

    def test_the_record_repeats_the_printed_figures(self):
        record, out = self.record()
        for key, prefix in (("agreed", "compared and agreed"),
                            ("executed", "lifted body executed"),
                            ("agreed_full_strength",
                             "  ...of which FULL-STRENGTH")):
            with self.subTest(key=key):
                printed = figure(self, out, prefix)
                self.assertEqual(record[key]["bytes"], printed[0])
                self.assertEqual(record[key]["functions"], printed[2])

    def test_the_record_carries_the_denominator_and_the_exclusion(self):
        record, _ = self.record()
        self.assertEqual(record["denominator"]["bytes"], 5000)
        self.assertEqual(record["denominator"]["functions"], 4)
        self.assertIn("external_library", record["denominator"]["exclusion"])

    def test_the_record_carries_every_downward_number(self):
        record, _ = self.record()
        down = record["must_go_down"]
        self.assertEqual(down["machine_carried"]["bytes"], 4900)
        self.assertEqual(down["never_compared"]["bytes"], 700)
        self.assertEqual(down["agreed_under_weakened_conditions"]["bytes"], 200)
        # Printed but not recorded is how a dashboard drifts from its source,
        # and this figure is 4,000 of the 5,000 bytes in the fixture: an
        # omission that would read as an improvement.
        self.assertEqual(down["agreed_only_on_paths_taken"]["bytes"], 4000)

    def test_a_downward_number_is_recorded_even_when_it_is_zero(self):
        """A key that appears only when non-zero reads as a schema change on
        the run where the number first goes wrong."""
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        report = write_report(
            tmp.name, [["0x00401000", "PASS", "16", "16", "", "clean"]])
        functions = write_functions(tmp.name, [("0x00401000", "100")])
        json_path = Path(tmp.name) / "clean.json"
        code, out = run_summary(report, functions, "--json", str(json_path))
        self.assertEqual(code, 0)
        down = json.loads(json_path.read_text())["must_go_down"]
        self.assertEqual(down["agreed_only_on_paths_taken"]["bytes"], 0)
        self.assertNotIn("agreed only on the paths", out)

    def test_the_tiers_in_the_record_partition_the_agreeing_bytes(self):
        record, _ = self.record()
        tiers = record["agreement_evidence"]
        self.assertEqual(sum(tier["bytes"] for tier in tiers.values()),
                         record["agreed"]["bytes"])
        self.assertEqual(tiers["full-strength"]["bytes"], 100)
        self.assertEqual(tiers["single-seed"]["bytes"], 200)


class QualifiedVerdictMembershipTests(unittest.TestCase):
    """The two verdicts that are neither a clean PASS nor a clean SKIP.

    Both were added to the membership tuples with nothing asserting them, and
    both sit exactly where a wrong answer flatters: each one, moved by a single
    word, either inflates coverage or shrinks a number published under MUST GO
    DOWN. Verified by damaging the tuples in memory rather than on disk -
    removing `PASS-paths-taken` from either set changed no test at all before
    this class existed.

    * PASS-paths-taken is an OraclePass renamed because the function's static
      flags say a path exists that no seed took. It RAN and it COMPARED, so it
      belongs in both of those figures - but it is NOT agreement about the
      function, only about the paths sixteen seeds happened to take, so folding
      it into `agreed` would buy coverage with evidence that was never
      collected. It gets its own must-go-down line instead.
    * SKIP-reached-blocked RAN - the lifted body executed until it walked into
      a construct the lift cannot model - but nothing was COMPARED, so it must
      leave `never compared` larger, not smaller.

    Distinct powers of two, so each figure's total names its membership.
    """

    SPEC = [
        ("0x00401000", "PASS", 1),
        ("0x00402000", "PASS-paths-taken", 2),
        ("0x00403000", "SKIP-reached-blocked", 4),
        ("0x00404000", "SKIP", 8),
        ("0x00405000", "FAIL", 16),
    ]

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.report = write_report(self.tmp.name, [
            [a, v, "16", "16", "", f"fn_{a}"] for a, v, _ in self.SPEC])
        self.functions = write_functions(
            self.tmp.name, [(a, str(s)) for a, _, s in self.SPEC])

    def test_both_qualified_verdicts_ran_the_lifted_body(self):
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        # 1 + 2 + 4 + 16 = 23. Dropping PASS-paths-taken gives 21, dropping
        # SKIP-reached-blocked gives 19, dropping both gives 17, and admitting
        # the plain SKIP gives 31.
        self.assertEqual(figure(self, out, "lifted body executed"),
                         (23, "74.19%", 4))

    def test_paths_taken_compared_but_reached_blocked_did_not(self):
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        # never compared = 31 - (PASS 1 + paths-taken 2 + FAIL 16) = 12.
        # Admitting SKIP-reached-blocked as compared gives 8 - a smaller
        # must-go-down number obtained by counting a row that compared nothing.
        self.assertEqual(figure(self, out, "  never compared"),
                         (12, "38.71%", 2))

    def test_agreement_on_the_paths_seeds_took_is_not_agreement(self):
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        # `agreed` is the plain PASS alone: 1 byte, not 3.
        self.assertEqual(figure(self, out, "compared and agreed"),
                         (1, "3.23%", 1))
        self.assertEqual(figure(self, out, "  agreed only on the paths"),
                         (2, "6.45%", 1))

    def test_paths_taken_is_not_given_an_evidence_tier(self):
        """The tiers partition `agreed`, and this verdict is not in `agreed`.

        Tiering it would make the tier totals exceed the headline they are a
        breakdown of, which is the one arithmetic error a reader would not
        check.
        """
        code, out = run_summary(self.report, self.functions)
        self.assertEqual(code, 0)
        table = tier_table(out)
        agreed_bytes, _, agreed_functions = figure(
            self, out, "compared and agreed")
        self.assertEqual(sum(v[1] for v in table.values()), agreed_bytes)
        self.assertEqual(sum(v[0] for v in table.values()), agreed_functions)
        self.assertEqual(table[summary.TIER_FULL], (1, 1, "100.00%"))

    def test_the_paths_taken_line_is_absent_when_no_row_carries_it(self):
        """An unconditional line would print "0 B" on every clean report and
        train the reader to skip it."""
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        report = write_report(
            tmp.name, [["0x00401000", "PASS", "16", "16", "", "clean"]])
        functions = write_functions(tmp.name, [("0x00401000", "100")])
        code, out = run_summary(report, functions)
        self.assertEqual(code, 0)
        self.assertNotIn("agreed only on the paths", out)


if __name__ == "__main__":
    unittest.main()
