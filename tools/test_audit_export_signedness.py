#!/usr/bin/env python3

from pathlib import Path
import tempfile
import unittest

import audit_export_signedness as audit


class MsvcTypeTests(unittest.TestCase):
    def test_free_function_with_repeated_pointer(self):
        self.assertEqual(
            audit.msvc_types("?bitmask@@YAXHPAHPAH@Z"),
            ["void", "int", "int*", "int*"])

    def test_back_reference_repeats_the_previous_argument(self):
        # `0` is "same as the argument before", which VC6 emits for a repeat.
        self.assertEqual(
            audit.msvc_types("?bitmask@@YAXHPAH0@Z"),
            ["void", "int", "int*", "int*"])

    def test_unsigned_and_signed_are_distinguished(self):
        self.assertEqual(
            audit.msvc_types("?whose_territory@@YAHIIIPAHH@Z"),
            ["int", "uint", "uint", "uint", "int*", "int"])

    def test_thiscall_member_skips_three_prefix_characters(self):
        self.assertEqual(
            audit.msvc_types("?get_alt@MapWin@@QAEHHHH@Z"),
            ["int", "int", "int", "int"])

    def test_unknown_encoding_is_refused_rather_than_guessed(self):
        self.assertIsNone(audit.msvc_types("?f@@YAHV?$vector@H@std@@@Z"))
        self.assertIsNone(audit.msvc_types("not a decorated name"))


class GccTypeTests(unittest.TestCase):
    def test_length_prefix_is_honoured(self):
        # A lazy name match would stop at the first `i` inside "bitmask".
        self.assertEqual(audit.gcc_types("_Z7bitmaskiPiS_"),
                         ["int", "int*", "int*"])

    def test_unsigned_arguments(self):
        self.assertEqual(audit.gcc_types("_Z15whose_territoryjjjPii"),
                         ["uint", "uint", "uint", "int*", "int"])

    def test_nested_name(self):
        self.assertEqual(audit.gcc_types("_ZN6MapWin7get_altEiii"),
                         ["int", "int", "int"])

    def test_unknown_encoding_is_refused(self):
        self.assertIsNone(audit.gcc_types("bitmask"))
        self.assertIsNone(audit.gcc_types("_Z3fooSt6vector"))


class AuditTests(unittest.TestCase):
    HEADER = "address,end_address,name\n"

    def write(self, functions_rows, def_lines):
        directory = Path(tempfile.mkdtemp(dir=audit.REPO_ROOT / "build"))
        functions = directory / "functions.csv"
        functions.write_text(self.HEADER + functions_rows, encoding="utf-8")
        definitions = directory / "OpenSMACX.def"
        definitions.write_text(def_lines, encoding="utf-8")
        return functions, definitions

    def test_agreeing_signatures_produce_no_finding(self):
        functions, definitions = self.write(
            "0x1000,0x1010,?bitmask@@YAXHPAHPAH@Z\n",
            '    "?bitmask@@YAXHPAHPAH@Z" = _Z7bitmaskiPiS_\n')
        compared, findings = audit.audit(functions, definitions, Path("absent"))
        self.assertEqual(compared, 1)
        self.assertEqual(findings, [])

    def test_unsigned_recovery_of_a_signed_original_is_flagged(self):
        functions, definitions = self.write(
            "0x1000,0x1010,?bitmask@@YAXHPAHPAH@Z\n",
            '    "?bitmask@@YAXIPAI0@Z" = _Z7bitmaskjPjS_\n')
        _, findings = audit.audit(functions, definitions, Path("absent"))
        self.assertEqual(len(findings), 1)
        self.assertEqual(
            [(a["index"], a["original"], a["recovered"])
             for a in findings[0]["arguments"]],
            [(0, "int", "uint"), (1, "int*", "uint*"), (2, "int*", "uint*")])

    def test_alias_is_matched_by_stem_not_by_decoration(self):
        # The whole point: a wrong decoration means the alias and the
        # catalogue name DIFFER, so keying on the full name finds nothing.
        functions, definitions = self.write(
            "0x1000,0x1010,?whose_territory@@YAHHHHPAHH@Z\n",
            '    "?whose_territory@@YAHIIIPAHH@Z" = _Z15whose_territoryjjjPii\n')
        _, findings = audit.audit(functions, definitions, Path("absent"))
        self.assertEqual(len(findings), 1)
        self.assertEqual(findings[0]["original"],
                         "?whose_territory@@YAHHHHPAHH@Z")

    def test_ambiguous_stem_is_skipped_rather_than_guessed(self):
        functions, definitions = self.write(
            "0x1000,0x1010,?f@@YAXH@Z\n0x2000,0x2010,?f@@YAXI@Z\n",
            '    "?f@@YAXI@Z" = _Z1fj\n')
        compared, findings = audit.audit(functions, definitions, Path("absent"))
        self.assertEqual((compared, findings), (0, []))


class CommittedTreeTests(unittest.TestCase):
    """The real .def against the real catalogue."""

    def setUp(self):
        if not audit.DEFAULT_FUNCTIONS.is_file():
            self.skipTest("docs/recovery/functions.csv is absent")
        self.compared, self.findings = audit.audit(
            audit.DEFAULT_FUNCTIONS, audit.DEFAULT_DEF, Path("absent"))

    def test_bitmask_stays_fixed(self):
        # bitmask is the one confirmed member of this class, corrected in
        # 312b5cf after byte-matching found it. If it reappears here, the
        # signature regressed.
        flagged = [one for one in self.findings
                   if one["original"].startswith("?bitmask@@")]
        self.assertEqual(flagged, [])

    def test_population_is_within_the_recorded_baseline(self):
        self.assertLessEqual(len(self.findings),
                             audit.BASELINE["disagreements"])

    def test_enough_exports_are_comparable_for_the_count_to_mean_anything(self):
        # Without this, a parser regression that understands nothing would
        # report zero findings and read as a clean audit.
        self.assertGreaterEqual(self.compared, 300)


if __name__ == "__main__":
    unittest.main()
