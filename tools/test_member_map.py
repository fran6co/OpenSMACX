#!/usr/bin/env python3
"""The member map is only safe while its two claims stay true.

It tells an agent (a) these offsets are read from the image, so they are not a
guess, and (b) the field at 0x838 is spelled `field_838_`, so you can write one
without looking it up. Claim (b) is held by `verify_member_offsets --check-names`
against the compiler. What is pinned here is claim (a)'s edge: the map must say
NOT DECLARED when the header does not declare an offset, because that is the
one case where an agent following (b) would invent a member that does not
exist - and it must stay silent when it knows nothing, never guess.
"""
from __future__ import annotations

import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import member_map as tool                                     # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
HAVE_ACCESSES = (REPO_ROOT / "docs" / "recovery" /
                 "member-accesses.csv").is_file()


class HeaderLookupTests(unittest.TestCase):
    def test_the_header_is_found_by_reading_not_by_guessing(self):
        # `src/<lowercase>.h` is wrong often enough to matter: Popup is
        # declared in basepop.h, and a guessing lookup would report the class
        # as having no header and mark every offset NOT DECLARED.
        with tempfile.TemporaryDirectory() as where:
            src = Path(where)
            (src / "basepop.h").write_text(
                "class DLLEXPORT Popup {\n  uint32_t field_10_;  // 0x10\n};\n")
            self.assertEqual(tool.header_for("Popup", src).name, "basepop.h")
            self.assertIsNone(tool.header_for("Nonesuch", src))

    def test_a_field_name_states_its_offset(self):
        with tempfile.TemporaryDirectory() as where:
            src = Path(where)
            path = src / "x.h"
            path.write_text("class X {\n"
                            "  uint32_t field_10_;  // 0x10\n"
                            "  uint8_t field_B9_[0x77F];\n"
                            "  int32_t field_A8_;\n"      # no trailing comment
                            "};\n")
            self.assertEqual(tool.declared_offsets(path),
                             {0x10, 0xB9, 0xA8})

    def test_a_missing_header_claims_no_offsets(self):
        self.assertEqual(tool.declared_offsets(None), set())


class RenderTests(unittest.TestCase):
    """Rendered against a synthetic header, so the assertions do not move when
    the tree's own headers gain a field."""

    def setUp(self):
        self.where = tempfile.TemporaryDirectory()
        self.src = Path(self.where.name)
        self.addCleanup(self.where.cleanup)
        self._real = tool.observed
        self.addCleanup(lambda: setattr(tool, "observed", self._real))

    def use(self, rows, header: str):
        (self.src / "thing.h").write_text(header)
        tool.observed = lambda klass: rows if klass == "Thing" else []

    def test_an_undeclared_offset_is_called_out(self):
        # THE DAMAGE CASE. The image touches 0x20; the header stops at 0x10.
        # Silence here would have an agent write `field_20_`, which does not
        # exist - or worse, add it at the wrong place.
        self.use([(0x10, 4, "witness"), (0x20, 4, "witness")],
                 "class Thing {\n  uint32_t field_10_;  // 0x10\n};\n")
        text = tool.render("Thing", self.src)
        self.assertIn("NOT DECLARED", text)
        self.assertIn("1 offset(s) above have no member declaring", text)

    def test_a_fully_declared_class_calls_nothing_out(self):
        self.use([(0x10, 4, "witness")],
                 "class Thing {\n  uint32_t field_10_;  // 0x10\n};\n")
        text = tool.render("Thing", self.src)
        self.assertNotIn("NOT DECLARED", text)
        self.assertIn("field_10_", text)

    def test_a_class_the_image_says_nothing_about_renders_nothing(self):
        # Silence must mean "the image proves no access", never "the reader
        # gave up" - the same rule jump_tables and frame_objects follow.
        self.use([], "class Thing {\n};\n")
        self.assertEqual(tool.render("Nothing", self.src), "")


@unittest.skipUnless(HAVE_ACCESSES, "needs docs/recovery/member-accesses.csv")
class AgainstTheTreeTests(unittest.TestCase):
    """0x00456300 `InfoWin::setup_text` was deferred with the reason "every
    offset used would be a guess". Every offset it needed was named."""

    def test_every_infowin_access_is_a_declared_field(self):
        rows = tool.observed("InfoWin")
        self.assertTrue(rows, "InfoWin has observed accesses in the tree")
        text = tool.render("InfoWin")
        self.assertNotIn("NOT DECLARED", text)

    def test_the_map_names_the_function_that_was_deferred(self):
        self.assertIn("setup_text", tool.render("InfoWin"))


if __name__ == "__main__":
    unittest.main()
