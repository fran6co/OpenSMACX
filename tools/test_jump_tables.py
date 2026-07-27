#!/usr/bin/env python3
"""Unit tests for switch/jump-table recovery.

The thing worth testing here is not "does it find a table" - a table is easy to
find and the naive version finds all 283 of them. It is the STOPPING RULE,
because every way of getting that wrong produces a plausible-looking answer:

* Over-reading past the end of a table is the failure this module exists to
  prevent, and it is invisible from the table itself. When two switches in one
  function put their tables back to back, the second table's entries point into
  the same function, so "stop when a dword leaves the function" sails through
  it. On the real image that rule returns 3,942 targets of which 1,416 are the
  next switch's arms. The regression test is therefore two adjacent tables, and
  it asserts the exact split - a test that only checked "some targets" passes
  against the bug.
* Over-reading is worse than not resolving at all. A short table leaves an
  out-of-range index reaching the dispatch, which traps and says so; a long one
  sends it into another switch's first arm and keeps running.
* The operand shape has to be exact. `jmp [eax + 0xe8]` is a vtable call, not a
  switch, and its displacement is a field offset - treating it as a table base
  would read whatever happens to live at 0xe8.
* An empty result must drop the site rather than emit an empty switch, which
  would be legal C++ that claims the switch has no arms.
* `bound_check` is the cross-check that makes the walk believable, so it is
  tested for the two shapes it claims and for the case it must refuse: an index
  register rewritten between the compare and the jump means the compare bounds
  something else.
"""
import unittest

from capstone import CS_ARCH_X86, CS_MODE_32, Cs
from capstone import x86

import jump_tables

BASE = 0x00401000
TABLES = 0x00402000

DECODER = Cs(CS_ARCH_X86, CS_MODE_32)
DECODER.detail = True


class FakeSection:
    def __init__(self, virtual_address, size):
        self.VirtualAddress = virtual_address
        self.Misc_VirtualSize = size
        self.SizeOfRawData = size
        self.PointerToRawData = virtual_address


class FakeHeader:
    def __init__(self, base):
        self.ImageBase = base


class FakeImage:
    """One flat section, so `read_bytes` maps an address straight to an index.

    The module reads the image through exactly one function, which is the whole
    interface a PE has to satisfy here; standing up a real PE to test a dword
    walk would test pefile.
    """

    def __init__(self, base=0x00400000, size=0x8000):
        self.OPTIONAL_HEADER = FakeHeader(base)
        self.sections = [FakeSection(0x1000, size)]
        self.__data__ = bytearray(0x1000 + size)
        self._base = base

    def poke(self, address, raw):
        offset = 0x1000 + (address - self._base - 0x1000)
        self.__data__[offset:offset + len(raw)] = raw

    def dwords(self, address, values):
        self.poke(address, b"".join(
            value.to_bytes(4, "little") for value in values))


def decode(encoded, address=BASE):
    """The single instruction encoded by `encoded`, a hex string."""
    found = list(DECODER.disasm(bytes.fromhex(encoded), address))
    if not found:
        raise AssertionError(f"{encoded} did not decode")
    return found[0]


# ff 24 85 <disp32>  jmp dword ptr [eax*4 + disp32]
JMP_EAX4 = "ff2485"
# ff 24 8d <disp32>  jmp dword ptr [ecx*4 + disp32]
JMP_ECX4 = "ff248d"
# ff 24 45 <disp32>  jmp dword ptr [eax*2 + disp32]
JMP_EAX2 = "ff2445"
# ff a4 83 <disp32>  jmp dword ptr [ebx + eax*4 + disp32]
JMP_EBX_EAX4 = "ffa483"


def little(value):
    return f"{value & 0xFFFFFFFF:08x}"[6:8] + f"{value & 0xFFFFFFFF:08x}"[4:6] \
        + f"{value & 0xFFFFFFFF:08x}"[2:4] + f"{value & 0xFFFFFFFF:08x}"[0:2]


def switch_jump(table, address=BASE, prefix=JMP_EAX4):
    return decode(prefix + little(table), address)


class IndexedJumpShape(unittest.TestCase):
    def test_scale_four_index_only_is_a_switch(self):
        found = jump_tables.indexed_jump(switch_jump(0x00407D64))
        self.assertEqual(found, (0x00407D64, x86.X86_REG_EAX))

    def test_index_register_is_reported_not_assumed(self):
        found = jump_tables.indexed_jump(
            switch_jump(0x0040BFE4, prefix=JMP_ECX4))
        self.assertEqual(found, (0x0040BFE4, x86.X86_REG_ECX))

    def test_base_register_form_is_not_a_switch(self):
        # `jmp dword ptr [eax + 0xe8]` is a vtable dispatch. 0xe8 is a field
        # offset, and reading a "table" there would walk unrelated memory.
        self.assertIsNone(jump_tables.indexed_jump(decode("ffa0e8000000")))

    def test_a_base_register_alongside_the_index_is_rejected(self):
        # `[ebx + eax*4 + T]` computes its table address at RUNTIME, so the
        # displacement alone names the wrong table. The previous test cannot
        # see this rule because that form has no index and is already rejected
        # for its scale; this one has scale 4 and a valid index, so only the
        # base check can refuse it.
        self.assertIsNone(jump_tables.indexed_jump(
            decode(JMP_EBX_EAX4 + little(0x00408E94))))

    def test_a_scale_other_than_four_is_rejected(self):
        # Same blind spot in the other direction: `[eax*2 + T]` has an index
        # and no base, so it passes every check except the scale. A table of
        # dwords is indexed by 4, and reading one strided by 2 would return
        # halves of adjacent entries.
        self.assertIsNone(jump_tables.indexed_jump(
            decode(JMP_EAX2 + little(0x00408E94))))

    def test_register_indirect_is_not_a_switch(self):
        self.assertIsNone(jump_tables.indexed_jump(decode("ffe0")))  # jmp eax

    def test_direct_jump_is_not_a_switch(self):
        self.assertIsNone(jump_tables.indexed_jump(decode("eb10")))


class Walk(unittest.TestCase):
    def setUp(self):
        self.image = FakeImage()
        self.spans = [(BASE, BASE + 0x100)]

    def test_stops_at_the_first_dword_outside_the_function(self):
        self.image.dwords(TABLES, [BASE + 0x40, BASE + 0x44, 0x90909090,
                                   BASE + 0x48])
        self.assertEqual(
            jump_tables.walk(self.image, self.spans, TABLES, 1 << 32),
            [BASE + 0x40, BASE + 0x44])

    def test_stops_at_the_limit_even_when_dwords_still_qualify(self):
        self.image.dwords(TABLES, [BASE + 0x40, BASE + 0x44, BASE + 0x48])
        self.assertEqual(
            jump_tables.walk(self.image, self.spans, TABLES, TABLES + 8),
            [BASE + 0x40, BASE + 0x44])

    def test_honours_a_second_span(self):
        # An outlined cold block is part of the function, so an arm there is a
        # real arm; a walk that only knew the entry span would stop early.
        spans = [(BASE, BASE + 0x100), (0x00650000, 0x00650040)]
        self.image.dwords(TABLES, [BASE + 0x40, 0x00650010, 0x90909090])
        self.assertEqual(jump_tables.walk(self.image, spans, TABLES, 1 << 32),
                         [BASE + 0x40, 0x00650010])

    def test_the_span_end_is_exclusive(self):
        # A span is half-open, so `high` is the first byte AFTER the function -
        # normally the next function's entry point. An inclusive test would
        # accept it as an arm and hand the lowerer a target that is not in this
        # body at all. No table in the image happens to end on that value, so
        # only a written test can hold the boundary.
        self.image.dwords(TABLES, [BASE + 0x40, BASE + 0x100, BASE + 0x44])
        self.assertEqual(
            jump_tables.walk(self.image, self.spans, TABLES, 1 << 32),
            [BASE + 0x40])

    def test_the_span_start_is_inclusive(self):
        # The entry point itself is a legal arm; a switch arm at offset zero is
        # unusual but excluding it would silently shorten the table.
        self.image.dwords(TABLES, [BASE, BASE + 0x40, 0x90909090])
        self.assertEqual(
            jump_tables.walk(self.image, self.spans, TABLES, 1 << 32),
            [BASE, BASE + 0x40])

    def test_never_exceeds_the_entry_cap(self):
        self.image.dwords(TABLES, [BASE + 0x40] * 5000)
        self.assertEqual(
            len(jump_tables.walk(self.image, self.spans, TABLES, 1 << 32)),
            jump_tables.MAX_ENTRIES)


class AdjacentTables(unittest.TestCase):
    """The regression test for the over-read this module exists to fix."""

    def setUp(self):
        self.image = FakeImage()
        self.spans = [(BASE, BASE + 0x100)]
        # Two tables laid out back to back, exactly as MSVC emits two switches
        # in one function. Every entry of BOTH points inside the function.
        self.image.dwords(TABLES, [BASE + 0x40, BASE + 0x44])
        self.image.dwords(TABLES + 8, [BASE + 0x50, BASE + 0x54, BASE + 0x58])
        self.image.dwords(TABLES + 0x14, [0x90909090])
        self.first = switch_jump(TABLES, BASE)
        self.second = switch_jump(TABLES + 8, BASE + 0x10, prefix=JMP_ECX4)

    def test_the_naive_walk_would_run_through_both(self):
        # Not an assertion about the module - it pins the bug the clamp fixes,
        # so this test fails loudly if the fixture stops reproducing it.
        self.assertEqual(
            len(jump_tables.walk(self.image, self.spans, TABLES, 1 << 32)), 5)

    def test_each_table_is_clamped_at_the_next_one(self):
        tables = jump_tables.resolve_decoded(
            self.image, self.spans, [self.first, self.second])
        self.assertEqual(tables, {
            BASE: [BASE + 0x40, BASE + 0x44],
            BASE + 0x10: [BASE + 0x50, BASE + 0x54, BASE + 0x58],
        })

    def test_the_clamp_does_not_depend_on_decode_order(self):
        # The clamp comes from a first pass over every site, so a jump decoded
        # before the table that bounds it still gets bounded.
        tables = jump_tables.resolve_decoded(
            self.image, self.spans, [self.second, self.first])
        self.assertEqual(len(tables[BASE]), 2)


class Resolve(unittest.TestCase):
    def setUp(self):
        self.image = FakeImage()
        self.spans = [(BASE, BASE + 0x100)]

    def test_single_table_ends_at_padding(self):
        self.image.dwords(TABLES, [BASE + 0x40, BASE + 0x44, BASE + 0x48])
        self.image.dwords(TABLES + 0xC, [0xCCCCCCCC])
        tables = jump_tables.resolve_decoded(
            self.image, self.spans, [switch_jump(TABLES)])
        self.assertEqual(tables,
                         {BASE: [BASE + 0x40, BASE + 0x44, BASE + 0x48]})

    def test_a_site_with_no_in_function_entries_is_dropped(self):
        # Emitting `switch (s.eax) { }` would be legal C++ asserting the switch
        # has no arms. Dropping the site keeps the dispatch, which traps.
        self.image.dwords(TABLES, [0x90909090])
        self.assertEqual(
            jump_tables.resolve_decoded(
                self.image, self.spans, [switch_jump(TABLES)]),
            {})

    def test_non_switch_jumps_contribute_nothing(self):
        self.assertEqual(
            jump_tables.resolve_decoded(
                self.image, self.spans, [decode("ffa0e8000000")]),
            {})

    def test_self_decoding_entry_point_agrees(self):
        self.image.dwords(TABLES, [BASE + 0x40, BASE + 0x44])
        self.image.dwords(TABLES + 8, [0x90909090])
        body = bytes.fromhex(JMP_EAX4 + little(TABLES))
        self.image.poke(BASE, body)
        self.image.poke(BASE + len(body), b"\x90" * (0x100 - len(body)))
        self.assertEqual(
            jump_tables.resolve(self.image, self.spans, DECODER),
            {BASE: [BASE + 0x40, BASE + 0x44]})


class BoundCheck(unittest.TestCase):
    """The independent second opinion on how long a table is."""

    def setUp(self):
        self.image = FakeImage()

    def test_direct_compare_gives_entries(self):
        # cmp eax, 3 / ja +2 / jmp [eax*4 + T]  ->  four arms
        stream = [decode("83f803", BASE), decode("7702", BASE + 3),
                  switch_jump(TABLES, BASE + 5)]
        self.assertEqual(jump_tables.bound_check(self.image, stream, 2),
                         (4, "direct"))

    def test_a_scheduled_instruction_between_compare_and_branch_is_fine(self):
        # MSVC hoists unrelated work into the delay between the two; requiring
        # adjacency loses 15 of the image's 186 direct sites.
        stream = [decode("83f803", BASE),
                  decode("8955f8", BASE + 3),      # mov [ebp-8], edx
                  decode("7702", BASE + 6),
                  switch_jump(TABLES, BASE + 8)]
        self.assertEqual(jump_tables.bound_check(self.image, stream, 3),
                         (4, "direct"))

    def test_a_rewritten_index_invalidates_the_compare(self):
        # `cmp eax, 3` says nothing about the table if EAX is reloaded before
        # the jump. Answering anyway would manufacture a false cross-check,
        # which is worse than having none.
        stream = [decode("83f803", BASE), decode("7702", BASE + 3),
                  decode("b80a000000", BASE + 5),  # mov eax, 10
                  switch_jump(TABLES, BASE + 10)]
        self.assertIsNone(jump_tables.bound_check(self.image, stream, 3))

    def test_a_compare_without_a_branch_is_not_a_bound(self):
        stream = [decode("83f803", BASE), switch_jump(TABLES, BASE + 3)]
        self.assertIsNone(jump_tables.bound_check(self.image, stream, 1))

    def test_byte_table_length_is_the_distinct_value_count(self):
        # cmp edi, 5 / ja / mov cl, [edi + B] / jmp [ecx*4 + T].
        # Six bytes selecting three arms means the dword table has three.
        byte_table = TABLES + 0x100
        self.image.poke(byte_table, bytes([0, 1, 2, 1, 0, 2]))
        stream = [decode("83ff05", BASE), decode("7702", BASE + 3),
                  decode("8a8f" + little(byte_table), BASE + 5),
                  switch_jump(TABLES, BASE + 11, prefix=JMP_ECX4)]
        self.assertEqual(jump_tables.bound_check(self.image, stream, 3),
                         (3, "byte-table"))

    def test_byte_table_bound_follows_the_byte_table_index_register(self):
        # The compare bounds EDI, the index into the BYTE table, never ECX.
        # Matching the compare against ECX would find nothing here.
        byte_table = TABLES + 0x100
        self.image.poke(byte_table, bytes([0, 1, 2, 3]))
        stream = [decode("83ff03", BASE), decode("7702", BASE + 3),
                  decode("33c9", BASE + 5),        # xor ecx, ecx
                  decode("8a8f" + little(byte_table), BASE + 7),
                  switch_jump(TABLES, BASE + 13, prefix=JMP_ECX4)]
        self.assertEqual(jump_tables.bound_check(self.image, stream, 4),
                         (4, "byte-table"))

    def test_no_bound_at_all(self):
        stream = [switch_jump(TABLES, BASE)]
        self.assertIsNone(jump_tables.bound_check(self.image, stream, 0))

    def test_a_non_switch_has_no_bound(self):
        stream = [decode("83f803", BASE), decode("7702", BASE + 3),
                  decode("ffe0", BASE + 5)]
        self.assertIsNone(jump_tables.bound_check(self.image, stream, 2))


if __name__ == "__main__":
    unittest.main()
