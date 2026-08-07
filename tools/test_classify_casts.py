#!/usr/bin/env python3
"""Unit tests for the cast classifier.

Every case here is a mistake the classifier actually made before it was
written. A classifier that answers confidently and wrongly is worse than one
that refuses, because a wrong row sends someone to edit a header on evidence
that was never there - so the tests are mostly about the refusals and the
distinctions, not about the happy path.
"""

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import classify_casts as tool  # noqa: E402


class OperandTest(unittest.TestCase):
    """Where the displacement actually lives."""

    def test_a_bare_this_has_no_offset(self):
        self.assertEqual(("this", None, False), tool.read_operand("this", ""))

    def test_the_offset_is_usually_inside_the_operand(self):
        # `reinterpret_cast<MCIVideo *>(self + 0xa14)`. Matching only a bare
        # `this`/`self` classified 124 of 237 casts as having no displacement
        # when the offset was sitting right there.
        self.assertEqual(("self", 0xA14, False),
                         tool.read_operand("self + 0xa14", ""))

    def test_the_offset_may_follow_the_closing_paren(self):
        # `reinterpret_cast<char *>(this) + 0xD4`. The trailing text starts
        # PAST the closing paren - `balanced()` returns the index after it -
        # so it is ` + 0xD4;` and not `) + 0xD4;`. Written the second way
        # first, which failed against a correct parser.
        self.assertEqual(("this", 0xD4, False),
                         tool.read_operand("this", " + 0xD4;"))

    def test_the_trailing_text_is_what_the_extractor_really_yields(self):
        # Pins the pairing the case above got wrong, against the real thing
        # rather than against a hand-written string.
        line, _, operand, trailing, _ = next(
            tool.casts_in("x = reinterpret_cast<char *>(this) + 0xD4;"))
        self.assertEqual(("this", " + 0xD4;"), (operand, trailing))
        self.assertEqual(("this", 0xD4, False),
                         tool.read_operand(operand, trailing))

    def test_a_negative_displacement_keeps_its_sign(self):
        self.assertEqual(("this", -0x1C, False),
                         tool.read_operand("this - 0x1c", ""))

    def test_an_indexed_run_keeps_its_base_and_is_flagged(self):
        # `bytes + 0x928 + (index - 1) * 0x19C` - the constant is the base of
        # an indexed run, which is worth keeping, not discarding.
        base, offset, indexed = tool.read_operand(
            "bytes + 0x928 + (index - 1) * 0x19C", "")
        self.assertEqual(("bytes", 0x928), (base, offset))
        self.assertTrue(indexed)


class BalancedTest(unittest.TestCase):
    def test_an_operand_containing_a_call_is_read_whole(self):
        # A regex for `\\(([^)]*)\\)` stops at the first `)`, so
        # `reinterpret_cast<T *>(base(x) + 4)` would report `base(x`.
        text = "reinterpret_cast<T *>(base(x) + 4);"
        casts = list(tool.casts_in(text))
        self.assertEqual(1, len(casts))
        self.assertEqual("base(x) + 4", casts[0][2])

    def test_an_unbalanced_cast_is_skipped_not_guessed(self):
        self.assertEqual([], list(tool.casts_in("reinterpret_cast<T *>(a")))


class AliasTest(unittest.TestCase):
    def test_a_single_binding_is_an_alias(self):
        self.assertEqual(
            {"self"},
            tool.aliases_of("  uint8_t *const self = "
                            "reinterpret_cast<uint8_t *>(this);\n"))

    def test_a_rebound_name_is_not_an_alias(self):
        # Two bindings mean an offset from it is no longer certainly an offset
        # from `this`, and a wrong attribution is worse than none.
        body = ("  char *p = reinterpret_cast<char *>(this);\n"
                "  char *p = reinterpret_cast<char *>(this);\n")
        self.assertEqual(set(), tool.aliases_of(body))

    def test_a_name_whose_address_is_taken_is_not_an_alias(self):
        body = ("  char *p = reinterpret_cast<char *>(this);\n"
                "  f(&p);\n")
        self.assertEqual(set(), tool.aliases_of(body))


class AlignmentTest(unittest.TestCase):
    """The accumulator agreed with 1083 of 1113 name-encoded offsets before
    alignment was modelled, and with all 1113 after."""

    def test_a_member_starts_on_its_own_alignment(self):
        self.assertEqual(0x140, tool.align_to(0x13D, 8))
        self.assertEqual(0x140, tool.align_to(0x140, 8))
        self.assertEqual(0x13C, tool.align_to(0x13A, 4))

    def test_a_struct_aligns_to_its_widest_member_not_its_size(self):
        # RECT is sixteen bytes of four `long`, so it aligns to 4. Reading the
        # SIZE here gave it 8 and pushed Win::outer_rect_ from 0x13C to 0x140,
        # taking thirty later members with it - all thirty of which encode
        # their own offset in their name and all thirty of which disagreed.
        self.assertEqual(4, tool.alignment_of("RECT", ""))
        self.assertEqual(16, tool.SIZES["RECT"])

    def test_nothing_aligns_beyond_eight(self):
        self.assertLessEqual(tool.alignment_of("double", ""), 8)


class WidthTest(unittest.TestCase):
    def test_an_array_multiplies_its_element(self):
        self.assertEqual(1024, tool.width_of("int32_t", "dib_", "[256]"))

    def test_a_multidimensional_array_multiplies_every_bound(self):
        self.assertEqual(36, tool.width_of("float", "values", "[3][3]"))

    def test_an_expression_bound_refuses_rather_than_guessing(self):
        # A guessed width does not fail, it silently moves every offset after
        # it, which is the one outcome this whole file exists to catch.
        self.assertIsNone(tool.width_of("int", "a", "[COUNT]"))

    def test_an_unknown_type_refuses(self):
        self.assertIsNone(tool.width_of("Whatever", "a", ""))


class ResolveTest(unittest.TestCase):
    LAYOUT = [(0x00, "head_", "void *", 4),
              (0x04, "rect_", "RECT", 16),
              (0x14, "tail_", "int", 4)]

    def test_an_offset_on_a_member_boundary_is_exact(self):
        self.assertEqual(("exact-member", "rect_", 0x04, 16),
                         tool.resolve(0x04, self.LAYOUT))

    def test_an_offset_inside_a_member_says_so(self):
        self.assertEqual(("inside-declared-member", "rect_", 0x04, 16),
                         tool.resolve(0x08, self.LAYOUT))

    def test_an_offset_past_the_layout_says_so(self):
        self.assertEqual("past-declared-layout",
                         tool.resolve(0x100, self.LAYOUT)[0])

    def test_an_unreadable_layout_is_a_row_not_a_dropped_cast(self):
        # The interesting number is how much cannot be answered for. A
        # classifier that emits only what it understood reports a clean sheet
        # by discarding the evidence.
        self.assertEqual("layout-unreadable", tool.resolve(0x10, None)[0])


class ReportTest(unittest.TestCase):
    def test_every_row_carries_every_column(self):
        rows = tool.classify()
        self.assertTrue(rows)
        for row in rows:
            self.assertEqual(set(tool.FIELDS), set(row))

    def test_every_row_has_a_resolution_and_a_confidence(self):
        for row in tool.classify():
            self.assertTrue(row["resolution"], row)
            self.assertIn(row["confidence"], ("proved", "derived",
                                              "hypothesis", ""))

    def test_the_computed_size_agrees_with_every_pinned_sizeof(self):
        """The second independent control, and the one that catches WIDTH.

        63 classes carry `static_assert(sizeof(X) == N)`, which is the
        compiler's own answer rather than this file's arithmetic. It is a
        different question from the offset check below: offsets can all be
        right while a member's EXTENT is wrong, because alignment absorbs the
        error. `Heap` summed to 17 against its asserted 0x14 and every offset
        after it still landed correctly, since 17 rounds up to 20 under
        4-byte alignment. What gave it away was the image reading a dword at
        Heap+16, which looked like an access straddling the end of the member.
        """
        import re
        pinned = {}
        for header in tool.SRC.glob("*.h"):
            for hit in re.finditer(
                    r"static_assert\(\s*sizeof\(\s*(\w+)\s*\)\s*==\s*"
                    r"(0x[0-9A-Fa-f]+|\d+)",
                    header.read_text(errors="ignore")):
                pinned[hit.group(1)] = int(hit.group(2), 0)
        self.assertTrue(pinned)
        disagreed = []
        for name, want in sorted(pinned.items()):
            got = tool.width_of(name, "probe", "")
            if got is not None and got != want:
                disagreed.append(f"{name}: asserted {want:#x}, computed {got:#x}")
        self.assertEqual([], disagreed)

    def test_a_nested_struct_is_laid_out_not_summed(self):
        # Heap is int8_t then four 4-byte members: 1+4+4+4+4 is 17, and the
        # real object pads the int8_t to 4 and ends on its own alignment.
        self.assertEqual(0x14, tool.width_of("Heap", "heap_", ""))

    def test_the_accumulator_agrees_with_every_name_encoded_offset(self):
        """The strongest available control, and it needs no compiler.

        1,113 members in this tree encode their own offset in their name
        (`field_15C_`). That is an independent statement of where each sits,
        derived from the IDB rather than from this file's arithmetic, so any
        disagreement is a defect in one of them. Both alignment bugs this
        classifier had were found here.
        """
        import class_layouts
        disagreed, seen = [], set()
        for header in sorted(tool.SRC.glob("*.h")):
            if header.name == "hypothesis_layouts.h":
                continue
            text = header.read_text(errors="ignore")
            for name, _, _ in class_layouts.class_bodies(text):
                if name in seen:
                    continue
                seen.add(name)
                layout = tool.layout_offsets(name)
                for at, member, _, _ in layout or ():
                    encoded = tool.FIELD_NAME_OFFSET.match(member)
                    if encoded and int(encoded.group(1), 16) != at:
                        disagreed.append(
                            f"{name}::{member} name says "
                            f"{int(encoded.group(1), 16):#x}, computed {at:#x}")
        self.assertEqual([], disagreed)


if __name__ == "__main__":
    unittest.main()
