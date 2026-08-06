#!/usr/bin/env python3
"""Unit tests for behaviour-derived member naming.

Every other naming source is a lookup; this one is an inference, so what needs
guarding is not coverage but the two ways it was observed to be confidently
wrong before the rules were tightened.
"""

import unittest

from capstone import CS_ARCH_X86, CS_MODE_32, Cs

import name_members_from_behaviour as namer


class FakeImage:
    def __init__(self, code, start=0x400000):
        self.code = code
        self.code_start = start


def engine():
    found = Cs(CS_ARCH_X86, CS_MODE_32)
    found.detail = True
    return found


def touched(code):
    image = FakeImage(code)
    return namer.touched(image, engine(), image.code_start, len(code))


class MemberNameTest(unittest.TestCase):
    """Only the accessor forms. A first attempt took any informatively-named
    method and produced 58 rows, most of them verbs."""

    def test_accessor_prefixes_are_stripped(self):
        self.assertEqual(namer.member_name("get_ms_length"), "ms_length_")
        self.assertEqual(namer.member_name("set_scaling"), "scaling_")
        self.assertEqual(namer.member_name("is_visible"), "visible_")
        self.assertEqual(namer.member_name("has_focus"), "focus_")

    def test_a_verb_is_not_a_member_name(self):
        # These were all proposed by the loose rule and all wrong: an event
        # handler is named for the EVENT, a command for what it DOES.
        for method in ("on_right_click", "clear_group", "delete_queue",
                       "menu_update", "close", "timer_callback_daemon"):
            self.assertEqual(namer.member_name(method), "", method)

    def test_placeholder_method_names_say_nothing(self):
        for method in ("UNK3", "sub_401000", "nullsub_1", ""):
            self.assertEqual(namer.member_name(method), "", method)

    def test_an_empty_subject_is_refused(self):
        self.assertEqual(namer.member_name("get_"), "")


class MethodNameTest(unittest.TestCase):
    def test_the_identifier_comes_out_of_the_mangled_name(self):
        self.assertEqual(namer.method_name("?get_ms_length@Wave@@QAEHXZ"),
                         "get_ms_length")

    def test_a_special_name_has_no_method_identifier(self):
        self.assertEqual(namer.method_name("??0Wave@@QAE@XZ"), "")


class TouchedTest(unittest.TestCase):
    def test_a_plain_read_is_recorded_with_its_width(self):
        # mov eax, [ecx+0x60]
        self.assertEqual(touched(b"\x8b\x41\x60"), {(0x60, 4)})

    def test_offset_zero_is_never_a_member(self):
        """`[ecx+0]` is the vtable pointer, a vbtable pointer or a base
        subobject. Including it was wrong 3 times out of 3."""
        self.assertEqual(touched(b"\x8b\x01"), set())

    def test_a_pointer_being_followed_is_not_the_subject(self):
        """THE DELEGATING ACCESSOR. `mov ecx,[ecx+0x14]` loads a pointer out
        of the member and dispatches through it, so the accessor's subject
        lives at the far end. Measured on Midi_Device::is_disabled and
        VoiceRx::get_buffer_size, which this was about to name `disabled_`
        and `buffer_size_` when 0x14 and 0x3C are the held device."""
        # mov ecx, [ecx+0x14]
        self.assertEqual(touched(b"\x8b\x49\x14"), set())

    def test_a_read_before_the_pointer_hop_still_counts(self):
        # mov eax,[ecx+0x60] ; mov ecx,[ecx+0x14]
        self.assertEqual(touched(b"\x8b\x41\x60\x8b\x49\x14"), {(0x60, 4)})

    def test_a_byte_access_reports_one_byte(self):
        # mov al, [ecx+0x9c]
        self.assertEqual(touched(b"\x8a\x81\x9c\x00\x00\x00"), {(0x9C, 1)})

    def test_an_indexed_operand_is_refused(self):
        # mov eax, [ecx+edx*4+0x10]
        self.assertEqual(touched(b"\x8b\x44\x91\x10"), set())


class RenderTest(unittest.TestCase):
    def test_sorted_with_a_header_and_hex_offsets(self):
        text = namer.render([
            ("Wave", 0x60, "ms_length_", 4, "?get_ms_length@Wave@@QAEHXZ"),
            ("Caviar", 0xD5, "scaling_", 4, "?set_scaling@Caviar@@QAEXM@Z"),
        ])
        rows = text.splitlines()
        self.assertEqual(rows[0], "class,offset,name,size,evidence")
        self.assertEqual([row.split(",")[0] for row in rows[1:]],
                         ["Caviar", "Wave"])
        self.assertIn("0xD5", rows[1])


if __name__ == "__main__":
    unittest.main()
