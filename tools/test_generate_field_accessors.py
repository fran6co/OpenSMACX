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


class TrivialBodyTests(unittest.TestCase):
    def test_a_bare_ret_is_a_do_nothing_body(self):
        kind, detail = generator.classify(decode("c3"))
        self.assertEqual("nothing", kind)
        self.assertEqual(0, detail["cleanup"])

    def test_a_do_nothing_body_keeps_its_cleanup(self):
        # ret 0xc. The cleanup is the ONLY thing such a body can get wrong,
        # and getting it wrong corrupts the caller rather than this function.
        kind, detail = generator.classify(decode("c20c00"))
        self.assertEqual("nothing", kind)
        self.assertEqual(0xC, detail["cleanup"])

    def test_zeroing_eax_is_a_constant_zero(self):
        # xor eax,eax / ret 0xc
        kind, detail = generator.classify(decode("31c0c20c00"))
        self.assertEqual("constant", kind)
        self.assertEqual(0, detail["value"])
        self.assertEqual(0xC, detail["cleanup"])

    def test_zeroing_another_register_is_not_a_constant_return(self):
        # xor ecx,ecx / ret - says nothing about EAX.
        kind = generator.classify(decode("31c9c3"))
        self.assertNotEqual("constant", kind[0] if kind else None)


class StoreSequenceTests(unittest.TestCase):
    """A run of constant stores to `this`, tracked symbolically."""

    def test_zeroing_two_fields_and_returning_this(self):
        # mov eax,ecx / xor ecx,ecx / mov [eax],ecx / mov [eax+4],ecx / ret
        kind, detail = generator.classify(decode("89c831c98908894804c3"))
        self.assertEqual("stores", kind)
        self.assertEqual([(0, 0), (4, 0)], detail["stores"])
        self.assertTrue(detail["returns_this"], "EAX still aliases this")

    def test_a_single_dword_store_is_a_one_element_sequence(self):
        # mov dword ptr [ecx+0x200],0 / ret. This reached the single-`mov`
        # branch first, which used to `return None` before the sequence check
        # ever saw it.
        kind, detail = generator.classify(decode("c78100020000 00000000 c3".replace(" ", "")))
        self.assertEqual("stores", kind)
        self.assertEqual([(0x200, 0)], detail["stores"])
        self.assertFalse(detail["returns_this"])

    def test_a_constant_held_in_a_register_is_followed(self):
        # mov edx,0x3f800000 / mov [ecx],edx / ret - the stored value is set up
        # earlier, so it has to be tracked rather than read off the store.
        kind, detail = generator.classify(decode("ba0000803f8911c3"))
        self.assertEqual("stores", kind)
        self.assertEqual([(0, 0x3F800000)], detail["stores"])

    def test_storing_an_untracked_register_is_refused(self):
        # mov [ecx],esi / ret - ESI holds something this tool cannot describe.
        self.assertIsNone(generator.classify(decode("8931c3")))

    def test_loading_a_register_from_memory_is_refused(self):
        # mov eax,[ecx] / mov [ecx],ecx / ret. The load is refused where it is
        # READ - a register may only be set from an immediate or a this-alias -
        # which is also why no separate "EAX is undescribable" guard exists:
        # it could never fire.
        self.assertIsNone(generator.classify(decode("8b0189 09c3".replace(" ", ""))))

    def test_a_body_that_never_touches_eax_makes_no_residue_claim(self):
        kind, detail = generator.classify(decode("c70100000000c3"))
        self.assertEqual("stores", kind)
        self.assertFalse(detail["returns_this"])
        self.assertIsNone(detail["eax"])


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
