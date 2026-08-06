#!/usr/bin/env python3
"""Pin the layout extraction, especially where it must REFUSE.

A layout that is wrong is worse than no layout: the offsets move, and a
recovered body indexing through the wrong one still compiles. It fails later
as a byte mismatch nobody traces back here. So the tests that matter are the
ones about refusing.
"""

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import class_layouts as tool  # noqa: E402


class MembersTest(unittest.TestCase):
    def test_plain_members_are_read_in_order(self):
        body = ("\n public:\n  void method(int);\n private:\n"
                "  uint32_t first_;\n  Entry *second_;\n  int third_;\n")
        self.assertEqual(
            [("uint32_t", "first_", ""), ("Entry *", "second_", ""),
             ("int", "third_", "")],
            tool.members_of(body))

    def test_the_star_binds_to_the_name_in_this_tree(self):
        # `Entry *head_`, not `Entry* head_`. Matching only the second
        # spelling found nothing at all and reported 11 classes instead of 25.
        self.assertEqual([("Entry *", "head_", "")],
                         tool.members_of("  Entry *head_;\n"))

    def test_an_array_keeps_its_bound(self):
        self.assertEqual([("int", "grid_", "[4]")],
                         tool.members_of("  int grid_[4];\n"))

    def test_a_virtual_refuses_the_whole_class(self):
        # A vtable pointer this cannot see sits at offset 0 and moves every
        # member after it.
        self.assertIsNone(tool.members_of("  virtual void f();\n  int a_;\n"))

    def test_a_union_refuses_the_whole_class(self):
        self.assertIsNone(tool.members_of("  union { int a; };\n"))

    def test_anything_unrecognised_refuses(self):
        self.assertIsNone(tool.members_of("  int a_ : 3;\n"))

    def test_a_function_pointer_member_refuses(self):
        # It IS storage and it contains a `(`, so the method test would skip
        # it and every offset after it would move. sizeof catches that only
        # when padding does not absorb the four bytes.
        self.assertIsNone(tool.members_of("  void (*callback)(int);\n"))

    def test_a_method_taking_a_function_pointer_still_parses(self):
        # The difference is the identifier before the first paren.
        self.assertEqual(
            [("int", "a_", "")],
            tool.members_of("  void init(void (*cb)(int));\n  int a_;\n"))

    def test_methods_and_access_labels_are_not_members(self):
        self.assertEqual([], tool.members_of(" public:\n  void f(int);\n"))


class VerificationGateTest(unittest.TestCase):
    """Only a PROVED layout reaches an agent."""

    def test_an_unverified_class_supplies_nothing(self):
        self.assertEqual([], tool.declaration_for("NotAClassAnywhere"))

    def test_the_verified_list_is_a_subset_of_what_is_extracted(self):
        extracted = set(tool.pinned_layouts())
        self.assertTrue(tool.verified_names() <= extracted,
                        "the verified list names a class no longer extracted; "
                        "regenerate it with tools/verify_class_layouts.py")

    def test_every_verified_class_supplies_members(self):
        for name in tool.verified_names():
            self.assertTrue(tool.declaration_for(name), name)

    def test_a_scalar_is_never_forward_declared(self):
        # `struct uint32_t;` is `error C2371: redefinition; different basic
        # types`, and it took out every unit that supplied a layout.
        for name in tool.verified_names():
            self.assertFalse(tool.referenced_types(name) & tool.SCALAR, name)


if __name__ == "__main__":
    unittest.main()
