#!/usr/bin/env python3
"""Unit tests for the PRACX reducer.

PRACX shares a lineage with Thinker, so the only thing it can add is coverage,
and the only thing that can go wrong is the name map: PRACX uses its own
vocabulary, and pairing `CCity` with the wrong class would write city fields
into it. The metric took three attempts and each failure is pinned below.
"""

import unittest

import correlate_pracx_layouts as pracx


class ParseTest(unittest.TestCase):
    def test_members_come_out_in_declaration_order(self):
        text = ("struct CSprite\n{\n"
                "  void *ppszFileName;\n"
                "  char *pcBits;\n"
                "  BYTE cTransparentIndex;\n"
                "};\n")
        found = pracx.parse(text)
        self.assertEqual([name for name, _, _ in found["CSprite"]],
                         ["ppszFileName", "pcBits", "cTransparentIndex"])

    def test_an_array_keeps_its_count(self):
        found = pracx.parse("struct S\n{\n  BYTE f9[3];\n};\n")
        self.assertEqual(found["S"][0][2], "3")

    def test_a_struct_with_no_members_is_not_recorded(self):
        self.assertEqual(pracx.parse("struct CWinBase;\n"), {})


class NameMapTest(unittest.TestCase):
    """Three metrics were tried and two were wrong."""

    def test_containment_refuses_a_big_struct_that_merely_overlaps(self):
        # COUNT rewarded size: CCity and CUnit both matched Caviar because
        # Caviar is large, and it is neither a city nor a unit.
        source = {"CCity": [(f"c{i}", "int", None) for i in range(20)]}
        twins = {"Caviar": [f"c{i}" for i in range(4)] + [f"z{i}" for i in
                                                          range(200)]}
        mapped, refused = pracx.name_map(source, twins)
        self.assertEqual(mapped, {})
        self.assertEqual(refused[0][0], "CCity")

    def test_containment_accepts_a_source_that_knows_more(self):
        """JACCARD divided by the union, so it punished the one case worth
        finding - a source with EXTRA members scored lower for having them.
        CSprite/Sprite came out at 0.77 and was refused with 12 of its 12
        members matching."""
        source = {"CSprite": [(f"m{i}", "int", None) for i in range(12)]}
        twins = {"Sprite": [f"m{i}" for i in range(12)] + ["extra1", "extra2",
                                                           "extra3"]}
        mapped, _ = pracx.name_map(source, twins)
        self.assertEqual(mapped, {"CSprite": "Sprite"})

    def test_a_handful_of_shared_names_is_coincidence(self):
        # Two structs both starting with a vtable pointer share one name and
        # mean nothing by it.
        source = {"S": [("vtbl", "void*", None), ("a", "int", None)]}
        twins = {"T": [("vtbl")]}
        mapped, _ = pracx.name_map(source, twins)
        self.assertEqual(mapped, {})

    def test_a_refusal_is_reported_with_its_score(self):
        source = {"S": [(f"m{i}", "int", None) for i in range(10)]}
        twins = {"T": [f"m{i}" for i in range(5)] + [f"z{i}" for i in range(5)]}
        _, refused = pracx.name_map(source, twins)
        self.assertEqual(refused[0][0], "S")
        self.assertLess(refused[0][2], pracx.CONFIDENT)


class AdditionsTest(unittest.TestCase):
    def test_only_members_the_twin_lacks_are_returned(self):
        source = {"CS": [("known", "int", None), ("extra", "char", None)]}
        twins = {"S": ["known"]}
        rows = pracx.additions(source, twins, {"CS": "S"})
        self.assertEqual(rows, [("S", "extra", "char")])

    def test_an_unmapped_struct_contributes_nothing(self):
        source = {"CS": [("extra", "int", None)]}
        self.assertEqual(pracx.additions(source, {}, {}), [])


class RenderTest(unittest.TestCase):
    def test_sorted_with_a_header(self):
        text = pracx.render([("Win", "oClass0A", "CClass0A"),
                             ("Console", "oMap", "CMap")])
        rows = text.splitlines()
        self.assertEqual(rows[0], "class,field,type,evidence")
        self.assertEqual([row.split(",")[0] for row in rows[1:]],
                         ["Console", "Win"])


if __name__ == "__main__":
    unittest.main()
