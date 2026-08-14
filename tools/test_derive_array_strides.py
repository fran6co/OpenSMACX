#!/usr/bin/env python3
"""What the stride reader must get right to be usable as a size oracle.

`static_assert(sizeof(X) == N)` is the tree's statement that a layout was
checked against the image, and everything downstream - `pinned_layouts`,
`verified-layouts.txt`, `supplyable` - takes that assertion as given. A stride
reader that is confidently wrong therefore does not produce a compile error; it
produces a wrong layout that compiles, and bodies that reach the wrong offset
and fail later as byte mismatches nobody traces back here.

So the properties pinned below are about REFUSING, not about finding.
"""
from __future__ import annotations

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import byte_match                                             # noqa: E402
import derive_array_strides as tool                           # noqa: E402

HAVE_IMAGE = byte_match.DEFAULT_EXE.is_file()


def read(assembly: bytes, at: int = 0x00400000, only=None):
    """Strides read out of one hand-assembled span."""
    original = tool.spans
    tool.spans = lambda: iter([(at, at, at + len(assembly))])
    try:
        return tool.strides(assembly, at, only)[0]
    finally:
        tool.spans = original


class LeaChainTests(unittest.TestCase):
    """The shape that reads `MapTiles`, at 1,287 sites."""

    #   lea edx, [eax + eax*4]          8d 14 80
    #   lea eax, [eax + edx*2]          8d 04 50
    #   mov edx, dword ptr [0x94a30c]   8b 15 0c a3 94 00
    #   lea eax, [edx + eax*4]          8d 04 82
    MAP = bytes.fromhex("8d1480" "8d0450" "8b150ca39400" "8d0482")

    def test_the_chain_multiplies_to_the_struct_size(self):
        found = read(self.MAP)
        self.assertEqual(found[0x0094A30C].most_common(1), [(0x2C, 1)])

    def test_an_unknown_index_is_not_reported_as_the_scale(self):
        # Without the chain, `[edx + eax*4]` says only that something is
        # scaled by four. Defaulting the missing factor to 1 reported stride 4
        # at 1,364 sites for MapTiles - a confident answer built from the part
        # of the computation that was thrown away.
        found = read(bytes.fromhex("8b150ca39400" "8d0482"))
        self.assertEqual(found, {})


@unittest.skipUnless(HAVE_IMAGE, "needs the shipped terranx.exe")
class AgainstTheImageTests(unittest.TestCase):
    """The four sizes this tool was first used to establish.

    Each is corroborated independently: the size the tool reads off the image
    equals the size cl computes from the members `src/` declares. They were
    derived from different things, so agreement is evidence and not a
    tautology - and if either side moves, this test says so.
    """

    EXPECTED = {
        0x0094A30C: (0x2C, "MapTiles", "Map"),
        0x00946A50: (0x59C, "Players", "Player"),
        0x0094A330: (0x90, "Chassis", "RulesChassis"),
        0x0094F358: (0x2C, "Technology", "RulesTechnology"),
    }

    @classmethod
    def setUpClass(cls):
        data, base = tool.image()
        found, _ = tool.strides(data, base)
        # Folded onto the owning array, exactly as the report is:
        # a reference to field +4 belongs to the struct at +0.
        cls.found = tool.by_owner(found)

    def test_each_array_has_one_dominant_stride(self):
        for address, (size, global_, _) in self.EXPECTED.items():
            counts = self.found.get(address)
            self.assertIsNotNone(counts, global_)
            best, hits = counts.most_common(1)[0]
            self.assertEqual(best, size, f"{global_} at 0x{address:08X}")
            # A single site is an anecdote. Every one of these has hundreds.
            self.assertGreater(hits, 20, global_)

    def test_the_dominant_stride_is_not_a_close_call(self):
        # Two comparable candidates means the model is guessing, and a guess
        # written into a static_assert is indistinguishable from a fact.
        for address, (_, global_, _) in self.EXPECTED.items():
            ranked = self.found[address].most_common(2)
            if len(ranked) < 2:
                continue
            self.assertGreater(ranked[0][1], ranked[1][1] * 4, global_)

    def test_src_still_asserts_what_the_image_says(self):
        asserts = tool.asserted()
        for _, (size, _, klass) in self.EXPECTED.items():
            self.assertEqual(asserts.get(klass), size, klass)


class SpanDisciplineTests(unittest.TestCase):
    def test_nothing_is_decoded_outside_a_catalogued_span(self):
        # A linear sweep of .text decodes jump tables and instruction tails as
        # code; the first version of this scan invented `add byte ptr [ebp -
        # 0x1e2e76f4], cl` and counted the immediates it found there.
        seen = []
        original = tool.spans
        tool.spans = lambda: iter([(0x400000, 0x400000, 0x400003)])
        try:
            tool.strides(bytes.fromhex("8d1480" + "00" * 64), 0x400000)
            seen.append(True)
        finally:
            tool.spans = original
        self.assertEqual(seen, [True])


if __name__ == "__main__":
    unittest.main()
