#!/usr/bin/env python3
"""Checks for the pieces that let the lifted image BOOT.

Booting is the only route to the half of the image the differential oracle
structurally cannot reach, and it rests on three small judgements that are easy
to get silently wrong:

  * which catalogue rows are the CRT the lift never translates,
  * which addresses are the C++ dynamic initialisers, and
  * which call targets are inside the calling function's own body.

Each produces a generated table that compiles perfectly whatever it contains,
so a mistake in any of them is invisible until a boot stops somewhere
confusing - or, worse, does not stop and runs the wrong initialisers.
"""

from __future__ import annotations

import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import lift_whole_image as lifter

CATALOGUE = (
    "address,size,name,recovery_state,body_ranges\n"
    "0x00401000,16,??__Ea,source_complete,0x00401000-0x00401010\n"
    "0x00401010,16,_strlen,external_library,0x00401010-0x00401020\n"
    "0x00401020,16,game_thing,unrecovered,0x00401020-0x00401030\n"
    "0x00401030,16,??__Eb,unrecovered,0x00401030-0x00401040\n"
    "0x00401040,0,_zero_sized,external_library,0x00401040-0x00401040\n"
)


def write_catalogue(directory: Path, text: str = CATALOGUE) -> Path:
    path = directory / "functions.csv"
    path.write_text(text)
    return path


class ExternalLibraryTests(unittest.TestCase):
    """The rows load_functions drops, which the PROGRAM still calls."""

    def load(self, text: str = CATALOGUE):
        with tempfile.TemporaryDirectory() as directory:
            return lifter.load_external_library(
                write_catalogue(Path(directory), text))

    def test_only_external_library_rows_are_returned(self):
        names = [entry["name"] for entry in self.load()]
        self.assertEqual(["_strlen"], names)

    def test_a_zero_sized_row_is_not_a_function(self):
        # It has no body, so nothing can call into it and a shim bound to it
        # would be bound to an address the catalogue does not really describe.
        self.assertNotIn("_zero_sized",
                         [entry["name"] for entry in self.load()])

    def test_the_result_is_sorted_by_address(self):
        text = (CATALOGUE
                + "0x00400500,8,_early,external_library,0x00400500-0x00400508\n")
        addresses = [entry["address"] for entry in self.load(text)]
        self.assertEqual(sorted(addresses), addresses)

    def test_the_lift_and_this_partition_the_catalogue(self):
        # Every row is either lifted or supplied, never both and never neither.
        # A row falling through both is a call that reaches nothing at all.
        with tempfile.TemporaryDirectory() as directory:
            path = write_catalogue(Path(directory))
            external = {entry["address"] for entry in
                        lifter.load_external_library(path)}
        lifted = {0x00401000, 0x00401020, 0x00401030}
        self.assertEqual(set(), lifted & external)


class CrtTableTests(unittest.TestCase):
    """The generated address -> name table the shims are bound through."""

    def generate(self, externals):
        with tempfile.TemporaryDirectory() as directory:
            out = Path(directory)
            lifter.write_crt_table(out, externals)
            return (out / "lifted_crt_table.h").read_text()

    def test_each_entry_carries_its_catalogued_name(self):
        text = self.generate([{"address": 0x0064FC88, "name": "__itoa"}])
        self.assertIn('{0x0064fc88U, "__itoa"}', text)

    def test_the_size_is_derived_from_the_table_not_written_down(self):
        # A hand-written count that disagreed with the array would silently
        # truncate every lookup past the shorter of the two.
        text = self.generate([{"address": 0x1000, "name": "a"}])
        self.assertIn("sizeof(OpensmacxCrtTable) / sizeof(OpensmacxCrtTable[0])",
                      text)

    def test_an_empty_catalogue_still_compiles_to_a_table(self):
        text = self.generate([])
        self.assertIn("OpensmacxCrtTable[]", text)


class CrtInitTests(unittest.TestCase):
    """The `??__E` dynamic initialisers, whose absence zeroes every global."""

    def generate(self, functions):
        with tempfile.TemporaryDirectory() as directory:
            out = Path(directory)
            lifter.write_crt_init(out, functions)
            return (out / "lifted_crt_init.h").read_text()

    def test_only_dynamic_initialisers_are_listed(self):
        text = self.generate([
            {"address": 0x00401000, "name": "??__Ea"},
            {"address": 0x00401020, "name": "game_thing"},
        ])
        self.assertIn("0x00401000U", text)
        self.assertNotIn("0x00401020U", text)

    def test_the_table_is_sorted_because_the_lookup_binary_searches_it(self):
        # lifted_boot.cpp binary-searches this to decide membership while
        # scanning for the .CRT array. Unsorted, the scan misses initialisers
        # and the located array comes out short - which reads as "the image has
        # fewer initialisers" rather than as a bug here.
        # THREE entries, deliberately. Two in reverse order are also two in
        # sorted order once reversed, so a two-entry fixture passes even when
        # the sort is replaced by a reversal - which is exactly what a first
        # attempt at this test did, and it proved nothing.
        text = self.generate([
            {"address": 0x00401030, "name": "??__Eb"},
            {"address": 0x00401000, "name": "??__Ea"},
            {"address": 0x00401060, "name": "??__Ec"},
        ])
        listed = [line.strip().rstrip("U,")
                  for line in text.splitlines() if line.strip().startswith("0x")]
        self.assertEqual(3, len(listed))
        self.assertEqual(sorted(listed), listed)


class InternalCallTargetTests(unittest.TestCase):
    """Calls into a function's own body, which need their own entry point."""

    class Operand:
        def __init__(self, imm):
            self.type = lifter.X86_OP_IMM
            self.imm = imm

    class Instruction:
        def __init__(self, address, mnemonic, target=None):
            self.address = address
            self.mnemonic = mnemonic
            self.operands = ([InternalCallTargetTests.Operand(target)]
                             if target is not None else [])

    def items(self, *instructions):
        return [("code", one) for one in instructions]

    def test_a_call_into_this_body_is_an_extra_entry_point(self):
        items = self.items(
            self.Instruction(0x1000, "call", 0x1008),
            self.Instruction(0x1008, "ret"))
        self.assertEqual([0x1008], lifter.internal_call_targets(items, set()))

    def test_a_call_to_a_catalogued_entry_is_an_ordinary_call(self):
        # It already has a dispatch entry; adding a second definition for it
        # would be two lifted bodies for one address.
        items = self.items(
            self.Instruction(0x1000, "call", 0x1008),
            self.Instruction(0x1008, "ret"))
        self.assertEqual([], lifter.internal_call_targets(items, {0x1008}))

    def test_a_call_that_leaves_the_body_is_not_one(self):
        items = self.items(self.Instruction(0x1000, "call", 0x9999))
        self.assertEqual([], lifter.internal_call_targets(items, set()))

    def test_a_jump_inside_the_body_is_not_one(self):
        # A jump becomes a goto and needs no entry point; treating it as one
        # would emit a duplicate body for every loop head in the image.
        items = self.items(
            self.Instruction(0x1000, "jmp", 0x1008),
            self.Instruction(0x1008, "ret"))
        self.assertEqual([], lifter.internal_call_targets(items, set()))

    def test_a_target_that_is_not_an_instruction_start_is_not_one(self):
        # Mid-instruction: an entry point there would lower from a byte offset
        # the decoder never produced.
        items = self.items(
            self.Instruction(0x1000, "call", 0x1004),
            self.Instruction(0x1008, "ret"))
        self.assertEqual([], lifter.internal_call_targets(items, set()))

    def test_duplicates_collapse_to_one_entry_point(self):
        items = self.items(
            self.Instruction(0x1000, "call", 0x1010),
            self.Instruction(0x1008, "call", 0x1010),
            self.Instruction(0x1010, "ret"))
        self.assertEqual([0x1010], lifter.internal_call_targets(items, set()))


if __name__ == "__main__":
    unittest.main()
