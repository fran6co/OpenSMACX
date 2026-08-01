#!/usr/bin/env python3
"""Checks for the redirect patch-fit guard.

A guard that cannot fail is worse than no guard, because it reads as evidence.
So every test here that asserts an overrun IS caught is paired with the input
that must NOT trip it, and the parsing tests exist because the two ways this
check can silently decide nothing are an empty address list and a patch width
read from the wrong place.
"""

from __future__ import annotations

import sys
import pathlib
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import verify_redirect_patch_fit as guard

DLLMAIN = """
    constexpr size_t PatchSize = 5;
    static const RedirectSpec specs[] = {
        {
            0x00406B20,
            reinterpret_cast<uintptr_t>(&a_redirect),
            OPENSMACX_SIGNATURE_00406B20,
        },
        {
            0x0061F785,
            reinterpret_cast<uintptr_t>(&b_redirect),
            OPENSMACX_SIGNATURE_0061F785,
        },
    };
    static_assert(sizeof(specs) / sizeof(specs[0]) == RedirectCount);
    const uintptr_t elsewhere = 0x00999999;
"""


class PatchWidthTests(unittest.TestCase):
    def test_the_width_comes_from_the_source_that_installs_it(self):
        self.assertEqual(5, guard.patch_size(DLLMAIN))

    def test_a_widened_patch_is_followed(self):
        # Someone moving to a 14-byte absolute jump must not leave this file
        # quietly checking the old width.
        self.assertEqual(14, guard.patch_size(
            "constexpr size_t PatchSize = 14;"))

    def test_a_missing_declaration_is_refused_rather_than_assumed(self):
        with self.assertRaises(SystemExit):
            guard.patch_size("static const RedirectSpec specs[] = {};")


class TableParsingTests(unittest.TestCase):
    def test_the_wired_addresses_are_read_from_the_table(self):
        self.assertEqual([0x00406B20, 0x0061F785], guard.wired_addresses(DLLMAIN))

    def test_an_address_outside_the_table_is_not_counted(self):
        # 0x00999999 sits after the static_assert. Scanning the whole file
        # would invent a redirect that does not exist and check it against a
        # function that is not being patched.
        self.assertNotIn(0x00999999, guard.wired_addresses(DLLMAIN))

    def test_a_missing_table_is_refused(self):
        with self.assertRaises(SystemExit):
            guard.wired_addresses("int main() { return 0; }")


class OverrunTests(unittest.TestCase):
    """The check itself, and the shapes it must not cry wolf over."""

    def test_function_starts_reads_hex_addresses_from_the_catalogue(self):
        # Nothing called function_starts(), so `int(row["address"], 16)` was
        # unexercised: base 15 and base 17 both survived every test. Base 0
        # happens to be EQUIVALENT here because every address carries the 0x
        # prefix, which is worth saying rather than chasing.
        import csv as _csv
        import tempfile
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory) / "functions.csv"
            with path.open("w", newline="", encoding="utf-8") as handle:
                writer = _csv.DictWriter(handle, fieldnames=["address", "name"])
                writer.writeheader()
                writer.writerow({"address": "0x005EE130", "name": "?b@@YAXXZ"})
                writer.writerow({"address": "0x004011AF", "name": "?a@@YAXXZ"})
            starts = guard.function_starts(path)
        # Sorted numerically, not as strings, and parsed as hexadecimal - 0x1AF
        # is 431, which no other base yields.
        self.assertEqual([0x004011AF, 0x005EE130], starts)

    def test_an_empty_catalogue_is_refused_rather_than_passing(self):
        # An empty list of starts makes every redirect look like it fits.
        import csv as _csv
        import tempfile
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory) / "functions.csv"
            with path.open("w", newline="", encoding="utf-8") as handle:
                _csv.DictWriter(handle, fieldnames=["address", "name"]).writeheader()
            with self.assertRaises(SystemExit):
                guard.function_starts(path)

    def test_a_one_byte_function_padded_to_a_slot_is_fine(self):
        # ??1FileBox: a bare `ret` with the next function 16 bytes away. The
        # patch runs into padding, which is what the 280 short redirects
        # already in the table rely on.
        self.assertEqual(
            [], guard.overruns([0x00634C20], [0x00634C20, 0x00634C30], 5))

    def test_a_function_the_patch_outgrows_is_caught(self):
        # The same one-byte function with the next one packed 4 bytes later:
        # the fifth patch byte lands on its first instruction.
        found = guard.overruns([0x00634C20], [0x00634C20, 0x00634C24], 5)
        self.assertEqual([(0x00634C20, 0x00634C24, 4)], found)

    def test_exactly_enough_room_is_not_an_overrun(self):
        # A gap of exactly PatchSize fits, and an off-by-one here would reject
        # every 5-byte function in the image.
        self.assertEqual(
            [], guard.overruns([0x00616D80], [0x00616D80, 0x00616D85], 5))

    def test_one_byte_short_is_an_overrun(self):
        found = guard.overruns([0x00616D80], [0x00616D80, 0x00616D84], 5)
        self.assertEqual([(0x00616D80, 0x00616D84, 4)], found)

    def test_an_unaligned_entry_with_room_is_fine(self):
        # ?UNK3@Sprite@@QAEXHH@Z starts at 0x005EAAFF - three bytes long and
        # not 16-byte aligned, the shape that looked dangerous and is not.
        self.assertEqual(
            [], guard.overruns([0x005EAAFF], [0x005EAAFF, 0x005EAB0F], 5))

    def test_the_last_function_in_the_image_is_not_flagged(self):
        # There is no catalogued function after it to damage. Flagging it would
        # be a false positive that trains people to ignore this check.
        self.assertEqual([], guard.overruns([0x0063C9C0], [0x0063C9C0], 5))

    def test_a_wider_patch_condemns_gaps_that_a_narrow_one_allows(self):
        # A 12-byte slot: room for the 5-byte jump, not for a 14-byte absolute
        # one. This is the whole reason the width is read from dllmain.cpp
        # rather than written here.
        starts = [0x00634C20, 0x00634C2C]
        self.assertEqual([], guard.overruns([0x00634C20], starts, 5))
        self.assertEqual(1, len(guard.overruns([0x00634C20], starts, 14)))


if __name__ == "__main__":
    unittest.main()
