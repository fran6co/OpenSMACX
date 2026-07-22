#!/usr/bin/env python3

import unittest

import find_constant_returns as finder


class DeclaredArityTest(unittest.TestCase):
    """Reading the parameter list out of a mangled name.

    A replacement that pops the wrong number of bytes corrupts its caller
    rather than itself, so this is the check that decides whether a candidate
    is safe. It is only useful if it can actually read names that take
    arguments - the first version stopped scanning at "Z" and so hit the "@"
    that terminates a parameter list, silently declining every function with a
    parameter and validating only the zero-argument ones.
    """

    def test_reads_a_void_no_argument_method(self):
        self.assertEqual(("thiscall", 0),
                         finder.declared_arity("?close@BaseWin@@QAEXXZ"))

    def test_reads_a_single_int_argument(self):
        self.assertEqual(("thiscall", 4),
                         finder.declared_arity("?change@InfoWin@@QAEXH@Z"))

    def test_reads_several_int_arguments(self):
        self.assertEqual(("thiscall", 16),
                         finder.declared_arity("?UNK1@Font@@QAEHHHHH@Z"))

    def test_reads_two_arguments_after_a_void_return(self):
        self.assertEqual(("thiscall", 8),
                         finder.declared_arity("?set_loc@StatusWin@@QAEXHH@Z"))

    def test_counts_a_pointer_as_one_slot(self):
        self.assertEqual(
            ("thiscall", 8),
            finder.declared_arity("?set_bubble_text@X@@QAEHPADH@Z"))

    def test_counts_a_double_as_two_slots(self):
        self.assertEqual(("thiscall", 8),
                         finder.declared_arity("?scale@X@@QAEXN@Z"))

    def test_reads_a_cdecl_static(self):
        self.assertEqual(("cdecl", 0),
                         finder.declared_arity("?close_class@CheckButton@@QAAXXZ"))

    def test_reads_a_stdcall(self):
        self.assertEqual(("stdcall", 4),
                         finder.declared_arity("?handler@X@@QAGXH@Z"))

    def test_declines_a_name_it_cannot_read(self):
        self.assertIsNone(finder.declared_arity("sub_628220"))


class ConstantBodyTest(unittest.TestCase):
    def test_reads_a_constant_with_callee_cleanup(self):
        # mov eax, 0xB; ret 0x10
        self.assertEqual((0xB, 0x10),
                         finder.constant_body(b"\xb8\x0b\x00\x00\x00\xc2\x10\x00"))

    def test_reads_a_constant_with_no_cleanup(self):
        self.assertEqual((0xB, 0), finder.constant_body(b"\xb8\x0b\x00\x00\x00\xc3"))

    def test_reads_a_zeroed_return(self):
        self.assertEqual((0, 4), finder.constant_body(b"\x33\xc0\xc2\x04\x00"))

    def test_reads_a_bare_return(self):
        self.assertEqual((None, 0), finder.constant_body(b"\xc3"))

    def test_reads_a_bare_return_with_cleanup(self):
        self.assertEqual((None, 4), finder.constant_body(b"\xc2\x04\x00"))

    def test_declines_a_body_that_does_anything_else(self):
        # push ebp; mov ebp, esp; ... - a real function, not a stub.
        self.assertIsNone(finder.constant_body(b"\x55\x8b\xec\xc3"))

    def test_declines_a_truncated_body(self):
        self.assertIsNone(finder.constant_body(b"\xb8\x0b\x00"))


class AgreementTest(unittest.TestCase):
    """The pairing that decides acceptance."""

    def check(self, mangled, popped):
        convention, declared = finder.declared_arity(mangled)
        expected = 0 if convention == "cdecl" else declared
        return expected == popped

    def test_accepts_when_the_name_and_the_body_agree(self):
        self.assertTrue(self.check("?change@InfoWin@@QAEXH@Z", 4))

    def test_rejects_the_known_disagreement(self):
        # Win::on_redraw declares no parameters while its body cleans eight
        # bytes; it is the one candidate in the image that fails this.
        self.assertFalse(self.check("?on_redraw@Win@@QAEHXZ", 8))

    def test_a_cdecl_static_must_not_clean_up(self):
        self.assertTrue(self.check("?close_class@CheckButton@@QAAXXZ", 0))
        self.assertFalse(self.check("?close_class@CheckButton@@QAAXXZ", 4))


if __name__ == "__main__":
    unittest.main()
