#!/usr/bin/env python3
"""The frame map has to be right about nesting, or it invents locals.

Its whole value is telling an agent "declare ONE `Popup`, not twelve objects".
Get that backwards and it does the opposite of its job, so what is pinned here
is the distinction between a frame slot and a sub-object, and the arithmetic
that says one object is inside another.
"""
from __future__ import annotations

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import byte_match                                             # noqa: E402
import frame_objects as tool                                  # noqa: E402

HAVE_IMAGE = byte_match.DEFAULT_EXE.is_file()


class DestructorNamesTests(unittest.TestCase):
    def test_a_destructor_name_yields_its_class(self):
        rows = {1: {"name": "??1Popup@@QAE@XZ"},
                2: {"name": "??1BasePop@@QAE@XZ"},
                3: {"name": "?close@Popup@@QAEXXZ"},
                4: {"name": "sub_401000"}}
        self.assertEqual(tool.destructors(rows), {1: "Popup", 2: "BasePop"})


@unittest.skipUnless(HAVE_IMAGE, "needs the shipped terranx.exe")
class AgainstTheImageTests(unittest.TestCase):
    """0x004D61A0, where an agent derived the same map by hand from the EH
    funclet table and used it to collapse a dozen guessed locals into one."""

    ADDRESS = 0x004D61A0

    @classmethod
    def setUpClass(cls):
        import pefile
        import emit_translation_unit as emit
        cls.functions = emit.load_functions()
        cls.pe = pefile.PE(str(byte_match.DEFAULT_EXE), fast_load=True)
        row = cls.functions[cls.ADDRESS]
        cls.spans = emit.parse_body_ranges(row.get("body_ranges") or "")
        cls.frame, cls.fields = tool.scan(cls.pe, cls.spans, cls.functions)

    def offsets_for(self, klass):
        return {offset for (_, offset), classes in self.frame.items()
                if klass in classes}

    def test_the_popup_and_its_scroll_are_one_basepop_apart(self):
        # THE ARITHMETIC THAT MATTERS. The Popup's own base sits at one slot
        # and its embedded Scroll at another, and the gap IS sizeof(BasePop) -
        # which three agents derived independently across three batches before
        # this tool existed.
        basepop = self.offsets_for("BasePop")
        scroll = self.offsets_for("Scroll")
        self.assertTrue(basepop and scroll)
        gaps = {abs(b - s) for b in basepop for s in scroll}
        self.assertIn(0x3230, gaps)

    def test_a_base_and_its_derived_share_one_offset(self):
        # Two destructors at ONE address are a class and its base, not two
        # objects. Reporting them as two locals is the error this prevents.
        for key, classes in self.frame.items():
            if "Scroll" in classes:
                self.assertIn("GraphicWin", classes, key)
                break
        else:
            self.fail("no Scroll slot found")

    def test_register_receivers_are_not_frame_slots(self):
        # `lea ecx, [eax + 0x8c]` names an offset inside whatever eax points
        # at. Counting it as a frame local invents an object that does not
        # exist, which is the failure this whole tool exists to stop.
        self.assertTrue(self.fields)
        for reg, _ in self.fields:
            self.assertNotIn(reg, ("ebp", "esp"))

    def test_a_flatbutton_gap_is_reported_as_a_size(self):
        text = tool.render(self.pe, self.spans, self.functions)
        self.assertIn("sizeof(FlatButton)", text)

    def test_a_function_with_no_objects_says_nothing(self):
        # Silence must mean "no C++ objects in this frame", never "the reader
        # gave up" - the same rule the jump-table resolver follows.
        import emit_translation_unit as emit
        row = self.functions[0x004E10C0]     # Console::editor_fast
        spans = emit.parse_body_ranges(row.get("body_ranges") or "")
        self.assertEqual(tool.render(self.pe, spans, self.functions), "")


if __name__ == "__main__":
    unittest.main()


class FrameSizeTests(unittest.TestCase):
    """The number four of seven bodies in one batch needed and did not have."""

    @unittest.skipUnless(HAVE_IMAGE, "needs the shipped terranx.exe")
    def test_the_reserved_size_is_read_from_the_prologue(self):
        import pefile
        import emit_translation_unit as emit
        pe = pefile.PE(str(byte_match.DEFAULT_EXE), fast_load=True)
        functions = emit.load_functions()
        row = functions[0x0053A980]      # the body that diagnosed the cause
        spans = emit.parse_body_ranges(row.get("body_ranges") or "")
        self.assertGreater(tool.frame_size(pe, spans), 0)

    def test_no_span_is_not_a_frame_of_zero(self):
        # -1 and 0 must stay distinct: a function that reserves nothing is a
        # fact, and "could not read it" is not that fact.
        self.assertEqual(tool.frame_size(None, []), -1)
