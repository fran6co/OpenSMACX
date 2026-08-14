#!/usr/bin/env python3
"""Unit tests for telling a vtable dispatch from a function-pointer field.

`call [reg+disp]` is both, and getting it wrong is not harmless in either
direction. A false POSITIVE makes the emitter hand an agent a shim class for a
vtable that does not exist, which it then has to notice and ignore. A false
NEGATIVE emits no shim for a call that needs one, and the agent hand-rolls
ninety lines of filler the emitter exists to save it from.

Both were live: `0x00644910` got a ten-slot shim for a raw function pointer at
`+0x24`, and the first fix for that stopped detecting `0x0060FB90`, whose
virtual base reads its vtable through a computed address.
"""

import unittest

import pefile

import byte_match
import emit_translation_unit as emit


class VtableSlotTest(unittest.TestCase):
    """Measured against real bodies, because the discriminator is a fact about
    this image's codegen rather than about x86 in general."""

    @classmethod
    def setUpClass(cls):
        if not byte_match.DEFAULT_EXE.is_file():
            raise unittest.SkipTest("the pinned executable is absent")
        cls.pe = pefile.PE(str(byte_match.DEFAULT_EXE))
        cls.rows = byte_match.load_rows()
        cls.shared = byte_match.shared_span_index(cls.rows)

    def slots(self, address):
        spans = byte_match.classify_body(
            self.pe, self.rows[address], self.shared).primary
        return emit.vtable_slots(self.pe, spans)[0]

    def test_a_function_pointer_field_is_not_a_slot(self):
        # 0x00644910 reads the pointer straight off the stack -
        # `mov eax,[esp+0xc]` - and calls `[eax+0x24]`. There is no vtable.
        self.assertEqual(self.slots(0x00644910), [])

    def test_a_plain_receiver_dispatch_is_a_slot(self):
        # `mov ecx,[ecx+0x3c]; mov eax,[ecx]; call [eax+0x20]`
        self.assertEqual(self.slots(0x004C88B0), [8])

    def test_a_virtual_base_dispatch_is_a_slot(self):
        # The vtable comes through a COMPUTED address here,
        # `mov eax,[edx+ecx-0x1c]`, which is why "dereferenced at offset zero"
        # was the wrong test and dropped these entirely.
        self.assertEqual(self.slots(0x0060FB90), [62])
        self.assertEqual(self.slots(0x006115E0), [62])

    def test_an_argument_carrying_slot_is_found(self):
        self.assertEqual(self.slots(0x00629D40), [48])


class ComDispatchTest(unittest.TestCase):
    """`This` PUSHED as argument zero, which `VCall` cannot express.

    Reported by an agent recovering `?create_session@Net@@` after the
    generated shim produced a call shape that could not match however the body
    was written. The detector is held to that function, because the four slot
    indices below were arrived at INDEPENDENTLY - the agent read them off the
    disassembly by hand before this existed.
    """

    @classmethod
    def setUpClass(cls):
        if not byte_match.DEFAULT_EXE.is_file():
            raise unittest.SkipTest("the pinned executable is absent")
        cls.pe = pefile.PE(str(byte_match.DEFAULT_EXE))
        cls.rows = byte_match.load_rows()
        cls.shared = byte_match.shared_span_index(cls.rows)

    def spans(self, address):
        return byte_match.classify_body(
            self.pe, self.rows[address], self.shared).primary

    def test_the_directplay_call_sites_are_found(self):
        self.assertEqual(emit.com_slots(self.pe, self.spans(0x0062EAA0)),
                         [4, 14, 22, 24])

    def test_the_vtable_load_may_sit_between_the_push_and_the_call(self):
        """0x0062E540, reported by an agent whose brief was missing a slot.

            mov  eax, [0x009BE600]
            push eax               <- `This`
            mov  ecx, [eax]        <- the vtable load, AFTER the push
            call [ecx + 0x7c]

        The first rule looked at the instruction IMMEDIATELY before the call
        and missed every site ordered this way - 35 functions' worth. What
        distinguishes COM from thiscall is the last PUSH being the object,
        not its adjacency to the call.
        """
        self.assertIn(31, emit.com_slots(self.pe, self.spans(0x0062E540)))

    def test_it_splits_one_function_the_way_an_agent_did_by_hand(self):
        """0x005E2690 dispatches SIX slots and only four are COM.

        An INDEPENDENT ORACLE: an agent recovering this determined by reading
        the disassembly that slots 17, 25, 26 and 32 push the receiver, and
        separately retyped slots 1 and 3 as ordinary `void`->`int` thiscall
        virtuals. It reported that before this test existed and its evidence
        was not used to build the detector.

        Discriminating WITHIN one function is the strong form of the claim: a
        rule that keyed on anything about the function as a whole - the file,
        the callee, a DirectX heuristic - would take all six or none.
        """
        self.assertEqual(emit.com_slots(self.pe, self.spans(0x005E2690)),
                         [17, 25, 26, 32])

    def test_a_function_where_every_dispatch_is_com(self):
        """0x005CB6C0, the same agent: "all 16 indirect calls push the
        receiver explicitly". Eight distinct slots, and every one of them."""
        self.assertEqual(emit.com_slots(self.pe, self.spans(0x005CB6C0)),
                         emit.vtable_slots(self.pe, self.spans(0x005CB6C0))[0])

    def test_a_thiscall_dispatch_is_not_com(self):
        """The discriminator is the LAST push being the object itself. An
        ordinary virtual call pushes arguments too, so anything softer than
        that calls every dispatch COM-shaped."""
        self.assertEqual(emit.com_slots(self.pe, self.spans(0x004C88B0)), [])
        self.assertEqual(emit.com_slots(self.pe, self.spans(0x00629D40)), [])

    def test_a_function_pointer_field_is_not_com_either(self):
        self.assertEqual(emit.com_slots(self.pe, self.spans(0x00644910)), [])

    def test_the_slot_stays_in_the_thiscall_shim_as_well(self):
        """ADDITIVE. A slot leaving `VCall` is a body that stops compiling,
        so both shapes are offered and the difference is stated."""
        self.assertEqual(
            sorted(set(emit.com_slots(self.pe, self.spans(0x0062EAA0)))
                   - set(emit.vtable_slots(self.pe, self.spans(0x0062EAA0))[0])),
            [])

    def test_the_shim_says_which_slots_and_how_to_spell_them(self):
        text = emit.com_shim([4, 14])
        self.assertIn("ComSlot004", text)
        self.assertIn("ComSlot014", text)
        self.assertIn("__stdcall", text)
        self.assertIn("slot(s): 4, 14", text)

    def test_no_com_slots_means_no_shim(self):
        self.assertEqual("", emit.com_shim([]))


if __name__ == "__main__":
    unittest.main()
