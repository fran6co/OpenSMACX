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


if __name__ == "__main__":
    unittest.main()
