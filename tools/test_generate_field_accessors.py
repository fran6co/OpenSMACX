#!/usr/bin/env python3
"""Checks for the field-accessor generator.

A generator's dangerous failure is not a crash, it is emitting a plausible body
for a shape it did not actually understand - which then compiles, passes a
fixture written from the same misreading, and lands. So most of these tests are
about what it REFUSES.
"""

from __future__ import annotations

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import generate_field_accessors as generator
from capstone import CS_ARCH_X86, CS_MODE_32, Cs


def decode(encoded: str, address: int = 0x00401000):
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    return list(decoder.disasm(bytes.fromhex(encoded), address))


class AcceptedShapeTests(unittest.TestCase):
    def test_a_dword_field_read(self):
        # mov eax,[ecx+0x48] / ret
        kind, detail = generator.classify(decode("8b4148c3"))
        self.assertEqual("read", kind)
        self.assertEqual(0x48, detail["offset"])
        self.assertEqual(0, detail["cleanup"])

    def test_a_masked_field_read(self):
        # mov eax,[ecx] / and eax,1 / ret
        kind, detail = generator.classify(decode("8b0183e001c3"))
        self.assertEqual("masked", kind)
        self.assertEqual(0, detail["offset"])
        self.assertEqual(1, detail["mask"])

    def test_a_constant_return_keeps_its_cleanup(self):
        # mov eax,8 / ret 0x14 - the cleanup is the ONLY statement of arity
        # these unnamed bodies have, so it has to survive into the adapter.
        kind, detail = generator.classify(decode("b808000000c21400"))
        self.assertEqual("constant", kind)
        self.assertEqual(8, detail["value"])
        self.assertEqual(0x14, detail["cleanup"])

    def test_an_increment(self):
        # inc dword ptr [ecx+0x58] / ret
        kind, detail = generator.classify(decode("ff4158c3"))
        self.assertEqual("increment", kind)
        self.assertEqual(0x58, detail["offset"])

    def test_a_byte_store(self):
        # mov byte ptr [ecx+0x6d],1 / ret
        kind, detail = generator.classify(decode("c6416d01c3"))
        self.assertEqual("store_byte", kind)
        self.assertEqual(0x6D, detail["offset"])
        self.assertEqual(1, detail["value"])


class RefusalTests(unittest.TestCase):
    """What it declines, which is the part that keeps it honest."""

    def test_a_body_reading_the_stack_is_refused(self):
        # mov eax,[esp+4] / ret 4 - an ARGUMENT. These functions have no
        # mangled name, so nothing says what that argument is; a generator
        # that guessed would be inventing a signature. Refused by the base
        # check in this_offset, which is the single place a memory operand is
        # admitted at all.
        self.assertIsNone(generator.classify(decode("8b442404c20400")))

    def test_a_body_reading_a_register_other_than_ecx_is_refused(self):
        # mov eax,[edx+8] / ret - EDX is not `this` and nothing here says
        # what it holds.
        self.assertIsNone(generator.classify(decode("8b4208c3")))

    def test_an_absolute_load_is_refused(self):
        # mov eax,[0x009156b0] / ret - a global, not a field.
        self.assertIsNone(generator.classify(decode("a1b0569100c3")))

    def test_a_branch_is_refused(self):
        # test eax,eax / je +2 / ret - more than one path.
        self.assertIsNone(generator.classify(decode("85c07400c3")))

    def test_a_call_is_refused(self):
        self.assertIsNone(generator.classify(decode("e8fb0f0000c3")))

    def test_two_effects_are_refused(self):
        # mov [ecx+4],eax / mov [ecx+8],eax / ret - a shape this generator
        # has no template for, so it must decline rather than emit the first.
        self.assertIsNone(generator.classify(decode("8941048941 08c3".replace(" ", ""))))

    def test_a_body_with_no_ret_is_refused(self):
        self.assertIsNone(generator.classify(decode("8b4148")))

    def test_an_empty_body_is_refused(self):
        self.assertIsNone(generator.classify([]))


class EmissionTests(unittest.TestCase):
    def test_the_name_is_the_address(self):
        header, source, wires = generator.emit(
            [(0x00448310, "read", {"offset": 0x48, "cleanup": 0},
              "mov eax, dword ptr [ecx + 0x48] / ret")])
        self.assertIn("field_accessor_00448310_redirect", header)
        self.assertIn("field_accessor_00448310_redirect", source)
        self.assertIn("0x00448310 field_accessor_00448310_redirect", wires)

    def test_a_constant_body_leaves_its_parameter_unnamed(self):
        # It never reads `this`, and this tree builds with -Wall -Wextra where
        # an unused named parameter is an error, not a note.
        _, source, _ = generator.emit(
            [(0x00406840, "constant", {"value": 1, "cleanup": 0},
              "mov eax, 1 / ret")])
        self.assertIn("(void *, void *)", source)
        self.assertNotIn("void *self", source)

    def test_a_field_body_names_its_parameter(self):
        _, source, _ = generator.emit(
            [(0x00448310, "read", {"offset": 0x48, "cleanup": 0}, "x")])
        self.assertIn("void *self", source)

    def test_the_original_offset_comment_is_emitted(self):
        # Without it the body has no home for find_leaf_testable, and
        # mutate_and_verify cannot locate it either.
        _, source, _ = generator.emit(
            [(0x00448310, "read", {"offset": 0x48, "cleanup": 0}, "x")])
        self.assertIn("Original Offset: 00448310", source)


if __name__ == "__main__":
    unittest.main()
