#!/usr/bin/env python3
"""Unit tests for vtable-install detection.

The disassembly needs the pinned image and is exercised by running the tool.
What is tested here is the shape of the artifact - and, in the docstring of the
tool itself, the thing it refuses to do.
"""

import unittest

import derive_vtables as vtables


class RenderTest(unittest.TestCase):
    def test_sorted_with_a_header(self):
        text = vtables.render([
            ("Win", 0x0, 0x669754, "??_GWin@@UAEPAXI@Z"),
            ("BaseWin", 0x444, 0x66974C, "??3FlatButton@@SAXPAXI@Z"),
        ])
        rows = text.splitlines()
        self.assertEqual(rows[0], "class,pointer_offset,vtable,first_slot")
        self.assertEqual([row.split(",")[0] for row in rows[1:]],
                         ["BaseWin", "Win"])

    def test_addresses_are_written_wide_and_offsets_are_not(self):
        # The vtable is an address and reads as one; the pointer offset is a
        # member offset and reads like the rest of this tree's offsets.
        text = vtables.render([("BaseWin", 0x444, 0x66974C, "f")])
        self.assertIn("0x444", text)
        self.assertIn("0x0066974C", text)

    def test_a_class_can_install_more_than_one(self):
        """Nine classes install a second vtable at 0x444, which is sizeof(Win),
        so a second polymorphic base begins there. Collapsing rows per class
        would lose exactly that."""
        text = vtables.render([
            ("BaseWin", 0x0, 0x669754, "a"),
            ("BaseWin", 0x444, 0x66974C, "b"),
        ])
        self.assertEqual(len(text.splitlines()), 3)

    def test_the_header_names_every_column(self):
        self.assertEqual(vtables.render([]).strip(),
                         "class,pointer_offset,vtable,first_slot")


class InstallerTest(unittest.TestCase):
    def test_only_constructors_and_destructors_install_one(self):
        # A vtable pointer is written when the object is built or torn down;
        # anywhere else the store is something else.
        self.assertEqual(vtables.INSTALLERS, ("??0", "??1"))


if __name__ == "__main__":
    unittest.main()
