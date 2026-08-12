#!/usr/bin/env python3
"""Unit tests for the member-offset verifier, and its damage case.

The tool exists because `sizeof` cannot see a wrong member boundary. A test
suite for it can fail in exactly the same way - assert only that a clean tree
reports clean, and the check could have been `return 0` all along. So the
load-bearing test here is `DamageTest`, which builds a header the image
CONTRADICTS and requires the gate's own invocation - `main(["--check", ...])`,
the argv CMake passes - to exit non-zero.

That test needs VC6, because reading the declared offsets means compiling. It
SKIPS when VC6 is absent and says so loudly rather than passing, which is the
distinction this tree has been bitten by before: a control that cannot run is
not a control that passed.
"""

import io
import sys
import tempfile
import unittest
from contextlib import redirect_stdout
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import verify_member_offsets as verifier  # noqa: E402


class MemberNameTest(unittest.TestCase):
    """What the scanner sees. A member it misses becomes a gap, and a gap is
    reported as `unclaimed` - which reads as "padding, fine" and is not."""

    def test_a_plain_member_is_found_with_its_type(self):
        self.assertEqual(verifier.member_names("uint32_t volume_;"),
                         {"volume_": "uint32_t"})

    def test_an_access_label_sharing_the_line_does_not_hide_the_member(self):
        self.assertIn("seed_", verifier.member_names(" private: uint32_t seed_;"))

    def test_an_array_member_is_found(self):
        self.assertIn("pad_", verifier.member_names("uint8_t pad_[0x24];"))

    def test_a_function_pointer_member_is_found(self):
        # Time declares two of these back to back. Missing them read its
        # constructor writing 0x8 and 0xC as two holes in a class with none.
        found = verifier.member_names("void(__cdecl *callback1_)(int);")
        self.assertIn("callback1_", found)

    def test_a_function_pointer_PARAMETER_is_not_a_member(self):
        # The shape is identical inside a parameter list; only the line anchor
        # tells them apart. Reading `callback` as a member of Time was C2039,
        # which took the whole header's probe unit down.
        found = verifier.member_names(
            "  void init(void(__cdecl *callback)(int), int param);")
        self.assertNotIn("callback", found)

    def test_a_member_declared_by_a_nested_definition_is_found(self):
        found = verifier.member_names(
            "struct Internal { int a; int b; } internal_[5];")
        self.assertIn("internal_", found)
        self.assertNotIn("a", found)

    def test_a_pointer_member_declared_by_a_nested_definition_is_found(self):
        found = verifier.member_names("struct S { int a; } *spots_;")
        self.assertIn("spots_", found)

    def test_a_method_is_not_a_member(self):
        self.assertEqual(verifier.member_names("int play(int a1);"), {})

    def test_a_static_member_is_not_storage_in_the_object(self):
        self.assertNotIn("TimeModal",
                         verifier.member_names("static Time *TimeModal;"))


class NamespaceTest(unittest.TestCase):
    """An in-namespace class is not a class the image has - every mangled name
    scopes its class directly under `@@` - and naming one unqualified in the
    probe is C2065, which loses the whole header."""

    def test_a_namespace_block_is_removed_whole(self):
        text = verifier.without_namespaces(
            "class A {};\nnamespace n {\nstruct B { int x; };\n}\nclass C {};")
        self.assertIn("class A", text)
        self.assertIn("class C", text)
        self.assertNotIn("struct B", text)

    def test_nested_braces_inside_the_block_do_not_end_it_early(self):
        text = verifier.without_namespaces(
            "namespace n {\nstruct B { int x; };\nstruct D { int y; };\n}\n"
            "class C {};")
        self.assertNotIn("struct D", text)
        self.assertIn("class C", text)

    def test_text_with_no_namespace_is_returned_unchanged(self):
        self.assertEqual(verifier.without_namespaces("class A {};"),
                         "class A {};")


class ProbeTableTest(unittest.TestCase):
    """The sentinel round trip. A symbol-table walk would break on the first
    COMDAT the compiler folded; this only needs the payload to be findable."""

    def test_the_payload_between_the_sentinels_is_read_back(self):
        import struct
        blob = (b"\x00" * 7
                + struct.pack("<I", verifier.SENTINEL)
                + struct.pack("<3I", 4, 0x68, 0x6C)
                + struct.pack("<I", verifier.SENTINEL)
                + b"\xff" * 5)
        self.assertEqual(verifier.read_table(blob), [4, 0x68, 0x6C])

    def test_a_missing_sentinel_yields_nothing_rather_than_garbage(self):
        self.assertEqual(verifier.read_table(b"\x01\x02\x03\x04"), [])

    def test_one_sentinel_alone_is_not_a_table(self):
        import struct
        self.assertEqual(
            verifier.read_table(struct.pack("<I", verifier.SENTINEL) + b"\x00" * 8),
            [])


class FindingTest(unittest.TestCase):
    """The judgement itself, on hand-built layouts - the one control that runs
    with no compiler and no image, so it is live on every machine."""

    LAYOUT = {"C": [("a", 0, 4, "uint32_t"), ("b", 4, 1, "uint8_t"),
                    ("pad", 5, 3, "uint8_t")]}
    SIZES = {"C": 8}

    def test_an_access_inside_one_member_is_not_a_finding(self):
        rows = verifier.findings(self.LAYOUT, self.SIZES,
                                 {"C": {(0, 4): "evidence"}})
        self.assertEqual(rows, [])

    def test_an_access_crossing_a_member_end_is_a_straddle(self):
        rows = verifier.findings(self.LAYOUT, self.SIZES,
                                 {"C": {(4, 4): "evidence"}})
        self.assertEqual([row[0] for row in rows], ["straddle"])
        self.assertIn("b ends at 0x5", rows[0][4])

    def test_an_access_at_sizeof_is_an_overrun(self):
        rows = verifier.findings(self.LAYOUT, self.SIZES,
                                 {"C": {(8, 4): "evidence"}})
        self.assertEqual([row[0] for row in rows], ["overrun"])

    def test_an_access_in_a_hole_is_unclaimed_and_not_a_violation(self):
        rows = verifier.findings({"C": [("a", 0, 4, "uint32_t")]}, {"C": 16},
                                 {"C": {(8, 4): "evidence"}})
        self.assertEqual([row[0] for row in rows], ["unclaimed"])

    def test_a_byte_read_inside_a_dword_member_is_not_a_finding(self):
        # `test byte ptr [ecx + 0x99], 2` reaches into a dword flag field all
        # over this image; calling that a boundary error would bury the tool.
        rows = verifier.findings(self.LAYOUT, self.SIZES,
                                 {"C": {(1, 1): "evidence"}})
        self.assertEqual(rows, [])

    def test_an_x87_access_names_the_type(self):
        rows = verifier.findings(self.LAYOUT, self.SIZES, {"C": {(0, 4): "e"}},
                                 {"C": {(0, 4): "fld dword ptr [ecx]"}})
        self.assertEqual([row[0] for row in rows], ["typing"])
        self.assertIn("float", rows[0][4])

    def test_a_member_already_spelled_float_is_not_reported(self):
        rows = verifier.findings({"C": [("a", 0, 4, "float")]}, {"C": 4},
                                 {"C": {(0, 4): "e"}},
                                 {"C": {(0, 4): "fld dword ptr [ecx]"}})
        self.assertEqual(rows, [])

    def test_an_x87_access_that_starts_no_member_is_not_a_typing_claim(self):
        # Inside an array or a by-value sub-object the float belongs to that
        # member's own layout, not to a member this class could respell.
        rows = verifier.findings(self.LAYOUT, self.SIZES, {"C": {(1, 4): "e"}},
                                 {"C": {(1, 4): "fld dword ptr [ecx + 1]"}})
        self.assertNotIn("typing", [row[0] for row in rows])

    def test_a_class_with_no_declared_layout_is_skipped_not_guessed(self):
        rows = verifier.findings({}, {}, {"Unknown": {(0, 4): "evidence"}})
        self.assertEqual(rows, [])

    def test_overruns_sort_before_straddles(self):
        rows = verifier.findings(self.LAYOUT, self.SIZES,
                                 {"C": {(4, 4): "e1", (8, 4): "e2"}})
        self.assertEqual([row[0] for row in rows], ["overrun", "straddle"])


class PinAuditTest(unittest.TestCase):
    """Which pinned sizes the image corroborates. The verdicts have to stay
    distinguishable: collapsing `short` into `reached` would report a size
    nothing checked as one an independent source agrees with."""

    def _src(self, work, text):
        (work / "a.h").write_text(text)
        return work

    def test_an_access_ending_exactly_at_sizeof_is_reached(self):
        with tempfile.TemporaryDirectory() as name:
            src = self._src(Path(name), "static_assert(sizeof(C) == 0x10, );")
            audit = verifier.audit_pins({}, {"C": {(0xC, 4): "e"}}, src)
        self.assertEqual(audit["C"][0], "reached")

    def test_an_access_ending_short_of_sizeof_is_short(self):
        with tempfile.TemporaryDirectory() as name:
            src = self._src(Path(name), "static_assert(sizeof(C) == 0x100, );")
            audit = verifier.audit_pins({}, {"C": {(0, 4): "e"}}, src)
        self.assertEqual(audit["C"][0], "short")

    def test_a_tail_within_eight_bytes_is_padding_not_a_gap(self):
        with tempfile.TemporaryDirectory() as name:
            src = self._src(Path(name), "static_assert(sizeof(C) == 0x10, );")
            audit = verifier.audit_pins({}, {"C": {(0xA, 2): "e"}}, src)
        self.assertEqual(audit["C"][0], "padded")

    def test_a_class_no_method_touches_is_silent_not_corroborated(self):
        # An array-element type looks exactly like this: its size comes from
        # the stride of the arrays that hold it, and nothing reads `this`.
        with tempfile.TemporaryDirectory() as name:
            src = self._src(Path(name), "static_assert(sizeof(C) == 0x10, );")
            audit = verifier.audit_pins({}, {}, src)
        self.assertEqual(audit["C"][0], "silent")

    def test_an_access_past_sizeof_is_refuted(self):
        with tempfile.TemporaryDirectory() as name:
            src = self._src(Path(name), "static_assert(sizeof(C) == 0x10, );")
            audit = verifier.audit_pins({}, {"C": {(0x20, 4): "e"}}, src)
        self.assertEqual(audit["C"][0], "REFUTED")

    def test_a_decimal_pin_is_read_as_decimal(self):
        with tempfile.TemporaryDirectory() as name:
            src = self._src(Path(name), "static_assert(sizeof(C) == 16, );")
            audit = verifier.audit_pins({}, {"C": {(0xC, 4): "e"}}, src)
        self.assertEqual(audit["C"][0], "reached")


class DamageTest(unittest.TestCase):
    """THE CONTROL. A header the image contradicts must fail the gate's own
    invocation - the same argv CMake passes, not a hand-called helper.

    The distinction matters here more than usual: `main` returns 0 and prints a
    reason when VC6 is missing, so a damage case that does not assert the
    non-zero exit would pass on every machine that cannot run the check.
    """

    HEADER = """#pragma once
class Probe {
 public:
  %s
  unsigned long tail_;
};
"""
    ACCESSES = ("class,offset,width,x87,evidence\n"
                "Probe,0x0,4,0,mov eax dword ptr [ecx]\n")

    def setUp(self):
        import byte_match

        reason = byte_match.available()
        if reason:
            raise unittest.SkipTest(
                f"NOT RUN, not passed: the damage case needs VC6 ({reason}). "
                "The gate this test controls is equally silent here.")

    def _run(self, declaration):
        with tempfile.TemporaryDirectory() as name:
            work = Path(name)
            (work / "stdafx.h").write_text("#pragma once\n")
            (work / "probe.h").write_text(self.HEADER % declaration)
            csv = work / "accesses.csv"
            csv.write_text(self.ACCESSES)
            buffer = io.StringIO()
            with redirect_stdout(buffer):
                code = verifier.main(["--check", "--src", str(work),
                                      "--accesses", str(csv)])
            return code, buffer.getvalue()

    def test_the_healthy_declaration_passes(self):
        code, output = self._run("unsigned long first_;")
        self.assertEqual(code, 0, output)

    def test_splitting_the_dword_the_image_reads_whole_FAILS_the_check(self):
        # This is the error sizeof cannot see: the class is the same size and
        # every offset after it is unchanged. Only the access width refutes it.
        code, output = self._run("unsigned char first_; unsigned char pad_[3];")
        self.assertEqual(code, 1, output)
        self.assertIn("straddle", output)

    def test_a_class_too_short_for_the_access_FAILS_the_check(self):
        with tempfile.TemporaryDirectory() as name:
            work = Path(name)
            (work / "stdafx.h").write_text("#pragma once\n")
            (work / "probe.h").write_text(
                "#pragma once\nclass Probe {\n public:\n  unsigned long a_;\n};\n")
            csv = work / "accesses.csv"
            csv.write_text("class,offset,width,x87,evidence\n"
                           "Probe,0x10,4,0,mov eax dword ptr [ecx + 0x10]\n")
            buffer = io.StringIO()
            with redirect_stdout(buffer):
                code = verifier.main(["--check", "--src", str(work),
                                      "--accesses", str(csv)])
            self.assertEqual(code, 1, buffer.getvalue())
            self.assertIn("overrun", buffer.getvalue())


class RenderTest(unittest.TestCase):
    """The published view round-trips, including the x87 column - a lost
    column would silently retire every typing finding."""

    def test_an_x87_access_survives_the_round_trip(self):
        text = verifier.render({"C": {(0, 4): "fld dword ptr [ecx]"}},
                               {"C": {(0, 4): "fld dword ptr [ecx]"}})
        with tempfile.TemporaryDirectory() as name:
            path = Path(name) / "a.csv"
            path.write_text(text)
            observed, typed = verifier.load_accesses(path)
        self.assertEqual(observed["C"][(0, 4)], "fld dword ptr [ecx]")
        self.assertIn((0, 4), typed["C"])

    def test_an_ordinary_access_does_not_come_back_typed(self):
        text = verifier.render({"C": {(0, 4): "mov eax, [ecx]"}}, {})
        with tempfile.TemporaryDirectory() as name:
            path = Path(name) / "a.csv"
            path.write_text(text)
            observed, typed = verifier.load_accesses(path)
        self.assertEqual(observed["C"][(0, 4)], "mov eax, [ecx]")
        self.assertEqual(typed, {})


if __name__ == "__main__":
    unittest.main()
