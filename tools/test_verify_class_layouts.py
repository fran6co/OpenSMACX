#!/usr/bin/env python3
"""Unit tests for the class-layout verifier.

This tool decides which classes an agent is handed with REAL members rather
than an opaque shell, and until this file existed nothing tested it. The
compile itself needs VC6 under wine and is exercised by running the tool; what
is tested here is everything around it, because those are the parts that fail
QUIETLY:

  * the exact bytes of the committed list, which `--check` compares character
    for character
  * whether `--check` actually reads the file it was pointed at
  * whether a missing toolchain reads as "no layout survived"

`test_class_layouts.py` has a VerificationGateTest, and it tests a different
thing: that `verified_names() <= pinned_layouts()`. That catches a class which
stopped being EXTRACTED and is blind to one whose extracted SHAPE changed.
"""

import io
import sys
import tempfile
import unittest
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path

import verify_class_layouts as verifier


class RenderTest(unittest.TestCase):
    """The committed bytes. A reworded banner or a lost trailing newline would
    report every checkout as stale without a layout having moved."""

    def test_every_name_appears_on_its_own_line(self):
        text = verifier.render(["Buffer", "Font", "Win"])
        self.assertEqual(
            [line for line in text.splitlines() if not line.startswith("#")],
            ["Buffer", "Font", "Win"])

    def test_the_banner_says_what_absence_means(self):
        # A reader finding a class missing from this file needs to know that
        # means "shell", not "not checked yet".
        text = verifier.render([])
        self.assertIn("opaque shell", text)
        self.assertTrue(text.startswith("#"))

    def test_it_ends_in_exactly_one_newline(self):
        text = verifier.render(["Buffer"])
        self.assertTrue(text.endswith("Buffer\n"))
        self.assertFalse(text.endswith("\n\n"))

    def test_an_empty_list_still_renders_the_banner(self):
        text = verifier.render([])
        self.assertEqual(len(text.splitlines()), 4)

    def test_the_order_given_is_the_order_written(self):
        # `verify` sorts; render must not re-sort or the two could disagree
        # about a name with an underscore or a digit.
        self.assertEqual(
            [l for l in verifier.render(["b", "a"]).splitlines()
             if not l.startswith("#")], ["b", "a"])


class CheckTest(unittest.TestCase):
    """`--check` against a file it was pointed at, with the compile stubbed."""

    def setUp(self):
        self.available = verifier.bm.available
        self.verify = verifier.verify
        self.pinned = verifier.class_layouts.pinned_layouts
        verifier.bm.available = lambda: ""
        verifier.verify = lambda candidates, src=None: (
            ["Buffer", "Font"], [], [])
        verifier.class_layouts.pinned_layouts = lambda: {"Buffer": 1, "Font": 1}
        self.work = Path(tempfile.mkdtemp())

    def tearDown(self):
        verifier.bm.available = self.available
        verifier.verify = self.verify
        verifier.class_layouts.pinned_layouts = self.pinned

    def run_main(self, *argv):
        out, err = io.StringIO(), io.StringIO()
        with redirect_stdout(out), redirect_stderr(err):
            status = verifier.main(list(argv))
        return status, out.getvalue() + err.getvalue()

    def test_a_current_list_passes(self):
        listing = self.work / "verified.txt"
        listing.write_text(verifier.render(["Buffer", "Font"]))
        status, output = self.run_main("--check", "--verified", str(listing))
        self.assertEqual(status, 0)
        self.assertIn("up to date", output)

    def test_a_dropped_class_fails(self):
        # The failure this gate exists for: a layout that stopped being
        # verified and would silently stop reaching agents.
        listing = self.work / "verified.txt"
        listing.write_text(verifier.render(["Buffer"]))
        status, output = self.run_main("--check", "--verified", str(listing))
        self.assertEqual(status, 1)
        self.assertIn("stale", output)

    def test_an_added_class_fails(self):
        listing = self.work / "verified.txt"
        listing.write_text(verifier.render(["Buffer", "Font", "Invented"]))
        self.assertEqual(
            self.run_main("--check", "--verified", str(listing))[0], 1)

    def test_a_missing_file_fails_rather_than_passing_empty(self):
        status, _ = self.run_main(
            "--check", "--verified", str(self.work / "absent.txt"))
        self.assertEqual(status, 1)

    def test_without_check_it_writes_the_file(self):
        listing = self.work / "nested" / "verified.txt"
        status, _ = self.run_main("--verified", str(listing))
        self.assertEqual(status, 0)
        self.assertEqual(listing.read_text(),
                         verifier.render(["Buffer", "Font"]))


class BuildControlTest(unittest.TestCase):
    """A probe that does not COMPILE is not a verdict about a layout.

    The case this exists for actually happened: `src/buffer.h` referenced an
    undeclared `Vert`, and because `graphicwin.h` and `win.h` include it, four
    classes whose layouts were correct - ButtonGroup, MenuEntry, PullDownItem,
    TutWin - stopped producing an object. Before this control they were
    reported as "the extracted layout is not the real size", and regenerating
    the list would have dropped all four.
    """

    def setUp(self):
        self.available = verifier.bm.available
        self.verify = verifier.verify
        self.pinned = verifier.class_layouts.pinned_layouts
        verifier.bm.available = lambda: ""
        verifier.class_layouts.pinned_layouts = lambda: {"Buffer": 1, "Font": 1}
        verifier.verify = lambda candidates, src=None: (
            ["Buffer"], [], [("Font", "font.h(9) : error C2061: 'Vert'")])
        self.work = Path(tempfile.mkdtemp())

    def tearDown(self):
        verifier.bm.available = self.available
        verifier.verify = self.verify
        verifier.class_layouts.pinned_layouts = self.pinned

    def run_main(self, *argv):
        out, err = io.StringIO(), io.StringIO()
        with redirect_stdout(out), redirect_stderr(err):
            status = verifier.main(list(argv))
        return status, out.getvalue() + err.getvalue()

    def test_an_unbuildable_probe_refuses_to_write_the_list(self):
        # The damaging outcome is not the red exit, it is a REGENERATION that
        # quietly drops the class. The file must not appear at all.
        listing = self.work / "verified.txt"
        status, _ = self.run_main("--verified", str(listing))
        self.assertEqual(status, 1)
        self.assertFalse(listing.exists())

    def test_it_says_header_defect_rather_than_wrong_layout(self):
        status, output = self.run_main("--verified",
                                       str(self.work / "verified.txt"))
        self.assertEqual(status, 1)
        self.assertIn("header defect", output)
        self.assertNotIn("not the real size", output)

    def test_it_names_the_class_and_the_compiler_error(self):
        # Naming the diagnostic is the whole point: `C2061: 'Vert'` points at
        # a header, and "ButtonGroup is the wrong size" points nowhere.
        _, output = self.run_main("--verified", str(self.work / "v.txt"))
        self.assertIn("Font", output)
        self.assertIn("C2061", output)

    def test_check_also_refuses_rather_than_reporting_stale(self):
        # Reporting "stale" here would blame the committed list for a typo in
        # a header, which is the wrong thing to go fix.
        listing = self.work / "verified.txt"
        listing.write_text(verifier.render(["Buffer"]))
        status, output = self.run_main("--check", "--verified", str(listing))
        self.assertEqual(status, 1)
        self.assertNotIn("stale", output)


class ErrorAttributionTest(unittest.TestCase):
    """`cl` compiles the batch and announces each unit by file name; that
    line is the only thing tying a diagnostic to the class it came from."""

    def test_it_attributes_each_error_to_the_unit_above_it(self):
        output = ("c000.cpp\n"
                  "buffer.h(32) : error C2061: syntax error : 'Vert'\n"
                  "c001.cpp\n"
                  "menu.h(9) : error C2065: undeclared 'Foo'\n")
        self.assertEqual(
            verifier.errors_by_unit(output, {"c000": "A", "c001": "B"}),
            {"A": "buffer.h(32) : error C2061: syntax error : 'Vert'",
             "B": "menu.h(9) : error C2065: undeclared 'Foo'"})

    def test_it_keeps_the_first_error_not_the_last(self):
        # A cascade reports the root cause first; the tail is usually noise
        # about the type the first error stopped it from parsing.
        output = ("c000.cpp\n"
                  "buffer.h(32) : error C2061: 'Vert'\n"
                  "buffer.h(40) : error C2238: unexpected token\n")
        self.assertEqual(verifier.errors_by_unit(output, {"c000": "A"}),
                         {"A": "buffer.h(32) : error C2061: 'Vert'"})

    def test_a_unit_that_compiled_gets_no_entry(self):
        self.assertEqual(
            verifier.errors_by_unit("c000.cpp\nc001.cpp\n",
                                    {"c000": "A", "c001": "B"}), {})

    def test_output_before_any_unit_name_is_not_attributed(self):
        # cl prints its own banner and any command-line diagnostics before the
        # first file name. Blaming those on a class would be worse than
        # reporting none.
        self.assertEqual(
            verifier.errors_by_unit("error D8021: invalid option\n",
                                    {"c000": "A"}), {})


class ToolchainAbsentTest(unittest.TestCase):
    """Without VC6 every probe fails to build, which reads as "no layout
    survived" and is indistinguishable from "all 36 are wrong". As a CTest
    gate that turns a missing toolchain into 36 phantom regressions."""

    def setUp(self):
        self.available = verifier.bm.available
        self.verify = verifier.verify
        verifier.bm.available = lambda: "wine is not on PATH"
        verifier.verify = lambda candidates, src=None: self.fail(
            "verify() must not run when the compiler is absent")

    def tearDown(self):
        verifier.bm.available = self.available
        verifier.verify = self.verify

    def test_it_skips_rather_than_reporting_a_regression(self):
        out = io.StringIO()
        with redirect_stdout(out):
            status = verifier.main(["--check"])
        self.assertEqual(status, 0)
        self.assertIn("SKIP", out.getvalue())


class DeclaringHeaderTest(unittest.TestCase):
    def test_it_maps_the_tree_s_own_classes(self):
        home = verifier.declaring_header()
        self.assertEqual(home.get("Buffer"), "buffer.h")
        self.assertEqual(home.get("Font"), "font.h")

    def test_every_home_is_a_header_that_exists(self):
        for name, header in verifier.declaring_header().items():
            self.assertTrue((verifier.SRC / header).is_file(), name)


if __name__ == "__main__":
    unittest.main()
