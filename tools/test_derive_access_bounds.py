#!/usr/bin/env python3
"""Unit tests for the observed-access lower bound.

The bound is allowed to be loose and is not allowed to be wrong, so every test
here is about the second property. Each of the three restrictions can only
lower the answer, and each is tested by the case that would raise it.
"""

import unittest

from capstone import CS_ARCH_X86, CS_MODE_32, Cs

import derive_access_bounds as bounds


class FakeImage:
    """Just the two attributes `reach` reads."""

    def __init__(self, code, start=0x400000):
        self.code = code
        self.code_start = start


def engine():
    found = Cs(CS_ARCH_X86, CS_MODE_32)
    found.detail = True
    return found


def reach(code):
    image = FakeImage(code)
    return bounds.reach(image, engine(), image.code_start, len(code))[0]


class ReachTest(unittest.TestCase):
    def test_the_width_of_the_access_counts(self):
        # mov eax, [ecx+0x10] touches bytes 0x10..0x13, so the object must be
        # at least 0x14 bytes. Reporting 0x10 would understate by the width of
        # the read.
        self.assertEqual(reach(b"\x8b\x41\x10"), 0x14)

    def test_a_byte_access_counts_one(self):
        # mov al, [ecx+0x10]
        self.assertEqual(reach(b"\x8a\x41\x10"), 0x11)

    def test_the_largest_access_wins(self):
        # mov eax,[ecx+0x10] ; mov edx,[ecx+4]
        self.assertEqual(reach(b"\x8b\x41\x10\x8b\x51\x04"), 0x14)

    def test_a_write_to_ecx_ends_the_window(self):
        """After `mov ecx, eax` the register is not the receiver, and reading
        on would attribute another object's offsets to this class."""
        # mov ecx, eax ; mov eax, [ecx+0x40]
        self.assertEqual(reach(b"\x8b\xc8\x8b\x41\x40"), 0)

    def test_accesses_before_the_write_still_count(self):
        # mov eax,[ecx+8] ; mov ecx,eax ; mov eax,[ecx+0x40]
        self.assertEqual(reach(b"\x8b\x41\x08\x8b\xc8\x8b\x41\x40"), 0xC)

    def test_a_negative_displacement_is_refused(self):
        """A negative offset is an adjusted `this` pointing into a base
        subobject, and those offsets belong to the base."""
        # mov eax, [ecx-0x1c]
        self.assertEqual(reach(b"\x8b\x41\xe4"), 0)

    def test_an_indexed_operand_is_refused(self):
        # mov eax, [ecx+edx*4+0x10] - the offset reached depends on edx.
        self.assertEqual(reach(b"\x8b\x44\x91\x10"), 0)

    def test_another_base_register_is_refused(self):
        # mov eax, [edx+0x10]
        self.assertEqual(reach(b"\x8b\x42\x10"), 0)

    def test_an_empty_body_reaches_nothing(self):
        self.assertEqual(reach(b"\xc3"), 0)


class ReceiverScopeTest(unittest.TestCase):
    """Which mangled names put `this` in ECX.

    Both directions have been wrong here. A lazy `[~\\w@]*?@(\\w+)@@` invented
    classes out of free functions, and `recovery_symbols.is_nonstatic_member`
    refuses the special-name family - so constructors, which write the WHOLE
    object and reach further into a class than anything else, were excluded
    from every bound.
    """

    def test_an_ordinary_thiscall_method(self):
        self.assertEqual(bounds.receiver_scope("?tech@SAmbience@@QAEXXZ"),
                         "SAmbience")

    def test_constructors_and_destructors_are_receivers(self):
        self.assertEqual(bounds.receiver_scope("??0Buffer@@QAE@XZ"), "Buffer")
        self.assertEqual(bounds.receiver_scope("??1Win@@UAE@XZ"), "Win")
        self.assertEqual(bounds.receiver_scope("??_GWin@@UAEPAXI@Z"), "Win")
        self.assertEqual(bounds.receiver_scope("??4Buffer@@QAEAAV0@ABV0@@Z"),
                         "Buffer")

    def test_a_free_function_is_not_a_class(self):
        # ?f@@YAXPAUGraphicWin@@@Z is a free function taking GraphicWin*. The
        # old regex read the convention code and the parameter type as a
        # class and invented scopes like YAXPAUGraphicWin.
        self.assertEqual(bounds.receiver_scope("?f@@YAXPAUGraphicWin@@@Z"), "")
        self.assertEqual(bounds.receiver_scope("?amovie_project@@YAXH@Z"), "")

    def test_a_static_member_has_no_receiver(self):
        self.assertEqual(bounds.receiver_scope("?g@Win@@SAHXZ"), "")

    def test_a_cdecl_member_takes_this_on_the_stack(self):
        # QAA, not QAE: the receiver is pushed, so ECX is not `this`.
        self.assertEqual(bounds.receiver_scope("?p@Win@@QAAXXZ"), "")

    def test_a_nested_scope_yields_the_innermost_class(self):
        self.assertEqual(bounds.receiver_scope("?h@Inner@Outer@@QAEXXZ"),
                         "Inner")

    def test_junk_is_refused(self):
        for name in ("", "sub_401000", "nullsub_1", "_main"):
            self.assertEqual(bounds.receiver_scope(name), "")


class FalsifiedTest(unittest.TestCase):
    def test_a_total_below_the_observed_reach_is_reported(self):
        found = {"PullDown": (0xF35, "mov byte ptr [ecx + 0xf34], 1")}
        original = bounds.idb_totals
        bounds.idb_totals = lambda: {"PullDown": 0xA14}
        try:
            wrong = bounds.falsified(found)
        finally:
            bounds.idb_totals = original
        self.assertEqual([(name, total, low) for name, total, low, _ in wrong],
                         [("PullDown", 0xA14, 0xF35)])

    def test_a_total_at_or_above_the_reach_is_not(self):
        found = {"S": (0x10, "")}
        original = bounds.idb_totals
        bounds.idb_totals = lambda: {"S": 0x10}
        try:
            self.assertEqual(bounds.falsified(found), [])
        finally:
            bounds.idb_totals = original


class RenderTest(unittest.TestCase):
    def test_committed_output_is_sorted_and_headed(self):
        text = bounds.render({"Win": (0x444, "a"), "Buffer": (0x588, "b")})
        rows = text.splitlines()
        self.assertEqual(rows[0], "class,lower_bound,evidence")
        self.assertEqual([row.split(",")[0] for row in rows[1:]],
                         ["Buffer", "Win"])

    def test_bounds_are_written_as_hex(self):
        text = bounds.render({"Win": (0x444, "")})
        self.assertIn("0x444", text)


if __name__ == "__main__":
    unittest.main()
