#!/usr/bin/env python3
"""Unit tests for the guarded delegation-thunk generator.

These exercise the decoder against hand-assembled bytes rather than the
executable, so they run anywhere and pin the refusals as firmly as the
acceptances. The refusals are the point: this generator writes 34 bodies
unattended, and a decoder that guesses instead of declining would put a wrong
one among them.
"""
import unittest

import generate_delegation_thunks as generator


class DecodeTest(unittest.TestCase):
    def test_declines_anything_outside_the_family(self):
        with self.assertRaises(generator.Unsettled):
            generator.decode(b"\x33\xc0\xc3", 0x400000)  # xor eax,eax; ret

    def test_declines_undecodable_bytes(self):
        with self.assertRaises(generator.Unsettled):
            generator.decode(b"", 0x400000)

    def test_reads_a_tail_form_with_a_constant(self):
        # mov ecx,[ecx+0x3c]; test ecx,ecx; je +7; mov eax,[ecx];
        # jmp [eax+0x1c]; mov eax,0x14; ret     - this is Midi::play.
        code = (b"\x8b\x49\x3c\x85\xc9\x74\x07\x8b\x01"
                b"\xff\xa0\x1c\x00\x00\x00"
                b"\xb8\x14\x00\x00\x00\xc3")
        member, slot, forwarded, popped, absent = generator.decode(
            code, 0x4C7760)
        self.assertEqual(0x3C, member)
        self.assertEqual(0x1C, slot)
        self.assertEqual(0, forwarded)
        self.assertEqual(0, popped)
        self.assertEqual(0x14, absent)

    def test_reads_a_tail_form_whose_constant_is_a_zeroing(self):
        code = (b"\x8b\x49\x3c\x85\xc9\x74\x07\x8b\x01"
                b"\xff\xa0\xc4\x00\x00\x00"
                b"\x33\xc0\xc3")
        self.assertEqual(0, generator.decode(code, 0x4C7BB0)[4])

    def test_refuses_a_tail_form_that_answers_caller_eax(self):
        # The absent-member path falls straight to the ret. No C++ body can
        # reproduce a caller's register residue, so this must not be emitted.
        code = (b"\x8b\x49\x3c\x85\xc9\x74\x07\x8b\x01"
                b"\xff\xa0\x1c\x00\x00\x00"
                b"\xc3")
        with self.assertRaises(generator.Unsettled) as caught:
            generator.decode(code, 0x400000)
        self.assertIn("residue", str(caught.exception))


class ConstantFromTest(unittest.TestCase):
    def test_reads_a_move(self):
        self.assertEqual(0x14, generator._constant_from(["mov eax, 0x14"]))

    def test_reads_a_zeroing(self):
        self.assertEqual(0, generator._constant_from(["xor eax, eax"]))

    def test_steps_over_the_epilogue(self):
        self.assertEqual(
            0x13, generator._constant_from(["pop ebp", "mov eax, 0x13"]))

    def test_declines_a_bare_epilogue(self):
        # Nothing loads EAX, so the answer is the caller's.
        self.assertIsNone(generator._constant_from(["pop ebp", "ret 4"]))

    def test_declines_a_tail_it_has_not_read(self):
        self.assertIsNone(
            generator._constant_from(["mov eax, dword ptr [ecx + 8]"]))


class SymbolTest(unittest.TestCase):
    def test_splits_a_class_and_method(self):
        self.assertEqual(("midi", "set_tempo"),
                         generator.symbol_for("?set_tempo@Midi@@QAEHK@Z"))

    def test_breaks_a_camel_case_class(self):
        self.assertEqual(
            ("wave_in_device", "set_codec"),
            generator.symbol_for("?set_codec@Wave_In_Device@@QAEHK@Z"))

    def test_declines_a_free_function(self):
        # A free name has no class to prefix with, and two free functions
        # could collide; the generator refuses rather than risk it.
        with self.assertRaises(generator.Unsettled):
            generator.symbol_for("?energy_limit@@YAHH@Z")


if __name__ == "__main__":
    unittest.main()
