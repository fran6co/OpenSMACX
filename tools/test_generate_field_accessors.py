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


class PaddingTests(unittest.TestCase):
    """IDA sizes a function to its whole slot; the slack is not code."""

    def test_trailing_nops_are_padding(self):
        # nullsub_185: `ret 4` followed by fifteen nops. Reading those as body
        # made it fail the "last instruction is a ret" check and sat in the
        # queue as if it were something to understand.
        kind, detail = generator.classify(decode("c20400" + "90" * 13))
        self.assertEqual("nothing", kind)
        self.assertEqual(4, detail["cleanup"])

    def test_only_trailing_nops_are_stripped(self):
        # nop / mov eax,[ecx+0x48] / ret. Stripping every nop would turn this
        # into a shape the generator recognises and emit a body for it; the
        # leading nop must survive so the whole thing is refused instead.
        kept = generator.strip_padding(decode("908b4148c3"))
        self.assertEqual(["nop", "mov", "ret"], [one.mnemonic for one in kept])
        self.assertIsNone(generator.classify(decode("908b4148c3")))

    def test_padding_after_a_real_body_is_stripped(self):
        kept = generator.strip_padding(decode("8b4148c39090"))
        self.assertEqual(["mov", "ret"], [one.mnemonic for one in kept])

    def test_padding_alone_is_still_refused(self):
        # All nops and no ret is not a function this tool can describe.
        self.assertIsNone(generator.classify(decode("90909090")))


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


class ParameterStoreTests(unittest.TestCase):
    """`this->field = argument`, where the `ret N` is the only arity there is."""

    def test_an_unframed_store_reads_esp(self):
        # sub_5f05c0: mov eax,[esp+4] / mov [ecx+0x128],eax / ret 4
        kind, detail = generator.classify(decode("8b442404898128010000c20400"))
        self.assertEqual("param_stores", kind)
        self.assertEqual([(0x128, 0)], detail["stores"])
        self.assertEqual(4, detail["cleanup"])

    def test_a_framed_store_reads_ebp(self):
        # sub_589750: push ebp / mov ebp,esp / mov eax,[ebp+8] /
        #             mov [ecx+0xa34],eax / pop ebp / ret 4
        kind, detail = generator.classify(
            decode("5589e58b45088981340a00005dc20400"))
        self.assertEqual("param_stores", kind)
        self.assertEqual([(0xA34, 0)], detail["stores"])

    def test_two_arguments_land_in_slot_order(self):
        # sub_590cb0: the second argument is at [ebp+0xc], not [ebp+8]. Getting
        # the base wrong here would swap the two fields silently.
        kind, detail = generator.classify(
            decode("5589e58b45088b550c89018951045dc20800"))
        self.assertEqual([(0, 0), (4, 1)], detail["stores"])
        self.assertEqual(8, detail["cleanup"])

    def test_reading_past_what_the_ret_cleans_is_refused(self):
        # mov eax,[ebp+0x10] with `ret 4`: that slot is the caller's, and one
        # of the two - the arity or the read - is misunderstood.
        self.assertIsNone(generator.classify(
            decode("5589e58b451089015dc20400")))

    def test_a_cdecl_body_is_refused(self):
        # sub_57dee0 reads [ebp+8] and cleans nothing, so its caller cleans:
        # a different convention, and emitting __fastcall for it would leave
        # the argument on the stack twice over.
        self.assertIsNone(generator.classify(
            decode("5589e58b450889015dc3")))

    def test_a_stray_push_is_refused(self):
        # push esi moves ESP, so [esp+4] no longer names the first argument
        # and the slot arithmetic would silently name the wrong one.
        self.assertIsNone(generator.classify(
            decode("568b44240489015ec20400")))

    def test_storing_a_register_that_holds_no_argument_is_refused(self):
        # mov [ecx],esi / ret 4 - ESI was never loaded from a slot.
        self.assertIsNone(generator.classify(decode("8931c20400")))

    def test_a_byte_wide_argument_read_is_refused(self):
        # mov al,[ebp+8] is not a dword copy, and describing it as one would
        # write three bytes of whatever EAX happened to hold.
        self.assertIsNone(generator.classify(
            decode("5589e58a450889015dc20400")))


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
