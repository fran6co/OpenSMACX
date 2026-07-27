#!/usr/bin/env python3

import unittest
from unittest import mock

import generate_adjustor_thunks as adjustor


class Instruction:
    def __init__(self, text):
        self.mnemonic, _, self.op_str = text.partition(" ")


def decode(*texts):
    return [Instruction(text) for text in texts]


class NameTest(unittest.TestCase):
    """The emitted symbol has to survive names that are already snake_case.

    `on_redraw` losing its underscore would collide `thunk1_X::on_redraw` with
    nothing, but it would produce `adjust_x1_onredraw` in committed source and
    in the redirect wire list, where nothing downstream would notice."""

    def test_splits_camel_case_but_keeps_snake_case(self):
        self.assertEqual("radio_button", adjustor.snake("RadioButton"))
        self.assertEqual("on_redraw", adjustor.snake("on_redraw"))
        self.assertEqual("wave_device", adjustor.snake("Wave_Device"))

    def test_parses_the_three_thunk_spellings(self):
        self.assertEqual(
            ("RadioButton", 1, "on_redraw", "QAEXXZ"),
            adjustor.parse_thunk_name("?on_redraw@thunk1_RadioButton@@QAEXXZ"))
        self.assertEqual(
            ("RadioButton", 3, "dtor", "QAE@H@Z"),
            adjustor.parse_thunk_name("??1thunk3_RadioButton@@QAE@H@Z"))
        self.assertEqual(
            ("Dialogs", 2, "scalar_delete", "UAEPAXI@Z"),
            adjustor.parse_thunk_name("??_Gthunk2_Dialogs@@UAEPAXI@Z"))

    def test_ignores_a_class_that_merely_contains_thunk(self):
        self.assertIsNone(adjustor.parse_thunk_name("?draw@Trunk@@QAEXXZ"))


class TemplateTest(unittest.TestCase):
    """Anything but the exact template is skipped and named, never guessed."""

    def decode_with(self, *texts):
        with mock.patch.object(adjustor.delegates, "decode",
                               return_value=decode(*texts)), \
             mock.patch.object(adjustor.delegates, "trailing_padding",
                               side_effect=lambda i: i):
            return adjustor.decode_template(
                None, {"address": "0x00406FF0", "size": "8"})

    def test_accepts_the_two_instruction_form(self):
        self.assertEqual(
            (4, None, 0x60E220),
            self.decode_with("sub ecx, dword ptr [ecx - 4]", "jmp 0x60e220"))

    def test_accepts_the_fixed_adjustment_form(self):
        self.assertEqual(
            (4, 0x5E4, 0x46EB90),
            self.decode_with("sub ecx, dword ptr [ecx - 4]",
                             "sub ecx, 0x5e4", "jmp 0x46eb90"))

    def test_rejects_an_indirect_tail_jump(self):
        # The target of `jmp [edx+0x100]` is a vtable slot, not an address a
        # seam could be bound to.
        self.assertIsInstance(
            self.decode_with("sub ecx, dword ptr [ecx - 4]",
                             "jmp dword ptr [edx + 0x100]"), str)

    def test_rejects_a_body_that_does_more_than_adjust_and_jump(self):
        self.assertIsInstance(
            self.decode_with("sub ecx, dword ptr [ecx - 4]",
                             "mov eax, dword ptr [ecx]", "jmp 0x60e220"), str)

    def test_rejects_a_missing_vtordisp_fixup(self):
        self.assertIsInstance(self.decode_with("jmp 0x60e220"), str)


class RawTemplateTest(unittest.TestCase):
    """The encoding is decoded a second time, without capstone's renderer.

    collect() requires both readings to agree, so this path exists to keep a
    text-rendering quirk from becoming a wrong displacement in emitted source.
    It has to be at least as strict as the capstone matcher, or it would rubber
    stamp whatever that one accepted."""

    def test_reads_the_disp8_form(self):
        # sub ecx, [ecx-4] ; jmp +0 (the next instruction)
        self.assertEqual(
            (4, None, 0x406FF8),
            adjustor.decode_raw_template(
                b"\x2b\x49\xfc\xe9\x00\x00\x00\x00", 0x406FF0))

    def test_reads_the_disp32_form_with_an_adjustment(self):
        # sub ecx, [ecx-0x448] ; sub ecx, 0x444 ; jmp -0x11C
        data = (b"\x2b\x89\xb8\xfb\xff\xff"
                b"\x81\xe9\x44\x04\x00\x00"
                b"\xe9\xef\xfe\xff\xff")
        self.assertEqual((0x448, 0x444, 0x406F60),
                         adjustor.decode_raw_template(data, 0x407060))

    def test_rejects_a_sub_through_another_register(self):
        # sub ecx, [eax-4] is not the template, however it renders.
        self.assertIsNone(adjustor.decode_raw_template(
            b"\x2b\x48\xfc\xe9\x00\x00\x00\x00", 0x406FF0))

    def test_rejects_a_missing_tail_jump(self):
        self.assertIsNone(
            adjustor.decode_raw_template(b"\x2b\x49\xfc\xc3", 0x406FF0))


class CalleePopTest(unittest.TestCase):
    """Arity is the one wrong guess that would corrupt a caller's stack.

    The thunk pops nothing itself, so its contract is the target's; the walk
    has to reach a real `ret` through the jump thunks that sit between."""

    def setUp(self):
        self.bodies = {}
        self.functions = {}

    def add(self, address, *texts):
        self.bodies[address] = decode(*texts)
        self.functions[address] = {"address": hex(address), "size": "8"}

    def pop(self, address):
        with mock.patch.object(adjustor.delegates, "decode",
                               side_effect=lambda pe, a, n: self.bodies[a]), \
             mock.patch.object(adjustor.delegates, "trailing_padding",
                               side_effect=lambda i: i):
            return adjustor.callee_pop(None, self.functions, address)

    def test_reads_the_pop_count_off_the_ret(self):
        self.add(0x1000, "push ebp", "ret 8")
        self.assertEqual(8, self.pop(0x1000))

    def test_treats_a_bare_ret_as_a_cdecl_style_zero_pop(self):
        self.add(0x1000, "xor eax, eax", "ret")
        self.assertEqual(0, self.pop(0x1000))

    def test_follows_a_pure_jump_thunk(self):
        self.add(0x1000, "jmp 0x2000")
        self.add(0x2000, "push ebp", "ret 12")
        self.assertEqual(12, self.pop(0x1000))

    def test_follows_a_tail_jump_out_of_a_body_with_no_ret(self):
        # EditGroup::pass_dialog_focus: every exit is a tail call, so the
        # stack contract is whatever those forward to.
        self.add(0x1000, "mov eax, dword ptr [ecx]",
                 "jmp dword ptr [edx + 0x100]", "jmp 0x2000")
        self.add(0x2000, "push ebp", "ret 0")
        self.assertEqual(0, self.pop(0x1000))

    def test_refuses_a_body_with_two_different_pop_counts(self):
        self.add(0x1000, "ret 4", "ret 8")
        self.assertIsNone(self.pop(0x1000))

    def test_refuses_to_loop_forever_on_a_jump_cycle(self):
        self.add(0x1000, "jmp 0x2000")
        self.add(0x2000, "jmp 0x1000")
        self.assertIsNone(self.pop(0x1000))


class SeamTableTest(unittest.TestCase):
    def row(self, seam, target, code):
        return {"seam": seam, "target": target, "code": code}

    def test_shares_one_seam_between_thunks_onto_the_same_target(self):
        rows = [self.row("MapWinOnRedrawTarget", 0x46B1D0, "v"),
                self.row("MapWinOnRedrawTarget", 0x46B1D0, "v")]
        self.assertEqual({"MapWinOnRedrawTarget": (0x46B1D0, "v")},
                         adjustor.seam_table(rows))

    def test_refuses_one_seam_at_two_signatures(self):
        rows = [self.row("MapWinOnRedrawTarget", 0x46B1D0, "v"),
                self.row("MapWinOnRedrawTarget", 0x46B1D0, "v_ii")]
        with self.assertRaises(SystemExit):
            adjustor.seam_table(rows)


class DeclarationTest(unittest.TestCase):
    def test_leaves_a_short_declaration_alone(self):
        self.assertEqual(["void f(int a);"],
                         adjustor.declaration("void f", ["int a"], ";"))

    def test_wraps_under_the_paren_without_trailing_space(self):
        lines = adjustor.declaration(
            "void __fastcall adjust_radio_button1_on_mouse_move",
            ["void *self", "void *", "int arg0", "int arg1"], " {")
        self.assertTrue(all(len(line) <= 79 for line in lines))
        self.assertTrue(all(line == line.rstrip() for line in lines))
        self.assertEqual(
            "void __fastcall adjust_radio_button1_on_mouse_move(void *self, "
            "void *, int arg0, int arg1) {",
            " ".join([lines[0]] + [line.lstrip() for line in lines[1:]]))


if __name__ == "__main__":
    unittest.main()
