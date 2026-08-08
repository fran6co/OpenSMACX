#!/usr/bin/env python3
"""Unit tests for pulling a proved body back out of a build artefact.

Whether a body still compiles and still matches needs wine, and
`mizuchi_writeback` is what decides it. What is tested here is the cut: where
the scaffolding stops and the body starts, and that the answer does not depend
on the emitter having stayed exactly the size it was when the unit was written.
"""

import unittest
from unittest import mock

import harvest_proven_units as harvest

TYPEDEFS = ("typedef int int32_t;\n"
            "typedef unsigned int uint32_t;\n"
            "typedef unsigned char uint8_t;\n")
BODY = ("class Shim { public:\n"
        "    void *method();\n"
        "};\n"
        "void *Shim::method() {\n"
        "    return this;\n"
        "}\n")


def extract(scaffolding: str, unit: str) -> str:
    """`body_of` with the emitter stubbed; only the cut is under test."""
    with mock.patch.object(harvest.emit, "emit", return_value=scaffolding):
        return harvest.body_of(0x00401000, unit, {}, {}, {}, None)


class CutTest(unittest.TestCase):
    def test_the_body_is_what_follows_the_scaffolding(self):
        self.assertEqual(extract(TYPEDEFS, TYPEDEFS + BODY), BODY)

    def test_the_header_comment_is_not_load_bearing(self):
        """The emitter's prose has been rewritten repeatedly and every unit
        predating a rewrite diverges from it. Only declarations anchor."""
        unit = "// rules that have since moved elsewhere\n" + TYPEDEFS + BODY
        self.assertEqual(extract("// entirely different prose\n" + TYPEDEFS,
                                 unit), BODY)

    def test_a_grown_emitter_still_finds_the_cut(self):
        """The regression this exists for. The emitter gained a second typedef
        block, so units written before it no longer contain today's LAST
        declaration - and 49 proved recoveries were refused although their
        scaffolding was a strict prefix of the current one."""
        grown = TYPEDEFS + "typedef unsigned char uint8;\n"
        self.assertEqual(extract(grown, TYPEDEFS + BODY), BODY)

    def test_an_agents_shim_declarations_are_kept(self):
        """Agents declare shim structs and typedefs ABOVE the definition to
        reach fields the opaque shell does not expose. Cutting at the
        subject's own head threw those away and the body stopped compiling."""
        self.assertIn("class Shim", extract(TYPEDEFS, TYPEDEFS + BODY))

    def test_a_closing_brace_is_never_the_anchor(self):
        """`};` closes the emitted shell AND every shim struct beside the
        body. Anchoring on it cuts at the last brace in the file, which is
        inside the body - so the body would be truncated to its own tail."""
        scaffolding = TYPEDEFS + "class Shell { public:\n    int m();\n};\n"
        out = extract(scaffolding, scaffolding + BODY)
        self.assertEqual(out, BODY)

    def test_a_unit_sharing_nothing_is_refused_not_guessed(self):
        with self.assertRaises(ValueError) as caught:
            extract(TYPEDEFS, "int unrelated_translation_unit(void) { return 0; }\n")
        self.assertIn("shares no scaffolding", str(caught.exception))

    def test_an_empty_body_is_refused(self):
        with self.assertRaises(ValueError):
            extract(TYPEDEFS, TYPEDEFS)

    def test_scaffolding_with_no_code_is_refused(self):
        with self.assertRaises(ValueError):
            extract("// comment only\n", TYPEDEFS + BODY)


if __name__ == "__main__":
    unittest.main()
