#!/usr/bin/env python3
"""Checks for the whole-image lifter's non-obvious decisions.

The generator's output is two million lines nobody reads, so the parts worth
testing are the small judgements that decide what that output MEANS: which
functions are excluded, how big the flat memory model has to be, and whether
the dispatch table is total. A silent mistake in any of those produces a
skeleton that compiles and links perfectly while being wrong about the image.
"""

from __future__ import annotations

import json
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import lift_whole_image as lifter


class FakeSection:
    def __init__(self, name: str, rva: int, virtual: int, raw: int):
        self.Name = name.encode("ascii").ljust(8, b"\0")
        self.VirtualAddress = rva
        self.Misc_VirtualSize = virtual
        self.SizeOfRawData = raw
        self.PointerToRawData = rva


class FakeHeader:
    def __init__(self, base: int):
        self.ImageBase = base


class FakePE:
    def __init__(self, sections):
        self.OPTIONAL_HEADER = FakeHeader(0x00400000)
        self.sections = sections


def build_pe():
    return FakePE([
        FakeSection(".text", 0x1000, 0x2000, 0x2000),
        # Virtual size far exceeds raw size, which is how .data carries
        # megabytes of zero-initialised globals in a small file. A span
        # computed from raw sizes would under-allocate the model and turn
        # every BSS access into an out-of-bounds write.
        FakeSection(".data", 0x4000, 0x300000, 0x1000),
        FakeSection("_SELFMOD", 0x310000, 0x1000, 0x1000),
    ])


class ImageSpanTests(unittest.TestCase):
    def test_span_covers_virtual_size_not_raw(self):
        base, size = lifter.image_span(build_pe())
        self.assertEqual(base, 0x00400000)
        # .data ends at rva 0x304000; the span must reach it, page-rounded.
        self.assertGreaterEqual(size, 0x310000 + 0x1000)
        self.assertEqual(size % 0x1000, 0)

    def test_span_is_page_rounded(self):
        pe = FakePE([FakeSection(".text", 0x1000, 0x1001, 0x1001)])
        _, size = lifter.image_span(pe)
        self.assertEqual(size, 0x3000)


class SectionTests(unittest.TestCase):
    def test_section_name_is_lowercased_and_trimmed(self):
        pe = build_pe()
        self.assertEqual(lifter.section_of(pe, 0x00400000 + 0x1000), ".text")
        self.assertEqual(
            lifter.section_of(pe, 0x00400000 + 0x310000), "_selfmod")

    def test_selfmod_is_declared_unliftable(self):
        # Self-modifying code cannot have a static translation by definition,
        # so this is the one exclusion the generator must never lose.
        self.assertIn("_selfmod", lifter.UNLIFTABLE_SECTIONS)

    def test_address_outside_every_section_is_not_attributed(self):
        self.assertEqual(lifter.section_of(build_pe(), 0x7F000000), "")


class LoadFunctionsTests(unittest.TestCase):
    def write_csv(self, directory: Path, rows: list[str]) -> Path:
        path = directory / "functions.csv"
        header = ("address,end_address,size,segment,name,prototype,"
                  "body_ranges,binary_kind,flags,source_locations,"
                  "source_statuses,redirect_exports,original_dependencies,"
                  "recovery_state,priority,notes,comments,call_target_count,"
                  "caller_count")
        path.write_text("\n".join([header] + rows) + "\n", encoding="utf-8")
        return path

    def test_external_library_rows_are_excluded(self):
        # The CRT and Windows come from the host toolchain. Lifting them would
        # translate code the build already supplies, and their addresses must
        # NOT enter the dispatch table or an indirect call into the CRT would
        # reach a lifted copy instead of the real one.
        with tempfile.TemporaryDirectory() as raw:
            directory = Path(raw)
            path = self.write_csv(directory, [
                "0x00401000,0x00401010,16,.text,game_fn,,,game,,,,,,unrecovered,,,,0,0",
                "0x00402000,0x00402010,16,.text,crt_fn,,,game,,,,,,external_library,,,,0,0",
            ])
            functions = lifter.load_functions(build_pe(), path)
        self.assertEqual([f["name"] for f in functions], ["game_fn"])

    def test_zero_size_rows_are_excluded(self):
        with tempfile.TemporaryDirectory() as raw:
            directory = Path(raw)
            path = self.write_csv(directory, [
                "0x00401000,0x00401000,0,.text,empty,,,game,,,,,,unrecovered,,,,0,0",
            ])
            self.assertEqual(lifter.load_functions(build_pe(), path), [])

    def test_functions_are_sorted_for_binary_search(self):
        # lifted_dispatch.cpp does a std::lower_bound over the emitted table,
        # so emission order IS the correctness condition, not a formatting
        # preference. Out of order, lookups silently miss.
        with tempfile.TemporaryDirectory() as raw:
            directory = Path(raw)
            path = self.write_csv(directory, [
                "0x00403000,0x00403010,16,.text,c,,,game,,,,,,unrecovered,,,,0,0",
                "0x00401000,0x00401010,16,.text,a,,,game,,,,,,unrecovered,,,,0,0",
                "0x00402000,0x00402010,16,.text,b,,,game,,,,,,unrecovered,,,,0,0",
            ])
            functions = lifter.load_functions(build_pe(), path)
        self.assertEqual([f["name"] for f in functions], ["a", "b", "c"])


class SymbolTests(unittest.TestCase):
    def test_symbols_are_address_derived_and_unique(self):
        first = lifter.symbol_for({"address": 0x004BA870})
        second = lifter.symbol_for({"address": 0x004ba871})
        self.assertEqual(first, "lifted_004ba870")
        self.assertNotEqual(first, second)


class EmissionTests(unittest.TestCase):
    def test_unliftable_function_body_traps_instead_of_lowering(self):
        with tempfile.TemporaryDirectory() as raw:
            out = Path(raw)
            functions = [{
                "address": 0x00664000, "size": 16, "name": "sub_664000",
                "section": "_selfmod", "instructions": 4,
                "fully_decoded": True, "state": "unrecovered",
            }]
            shards = lifter.write_shards(out, functions, 1)
            text = shards[0].read_text(encoding="utf-8")
        self.assertIn("self-modifying section", text)
        # A trapping body must contain no placeholder work at all, or a later
        # reader would take the statements for a real partial lowering.
        self.assertNotIn("opensmacx_cmp32", text)

    def test_statement_budget_scales_with_instruction_count(self):
        small = lifter.body_statements(
            {"address": 0x00401000, "instructions": 10})
        large = lifter.body_statements(
            {"address": 0x00401000, "instructions": 100})
        self.assertEqual(len(small), 10 * lifter.STATEMENTS_PER_INSTRUCTION)
        self.assertEqual(len(large), 100 * lifter.STATEMENTS_PER_INSTRUCTION)

    def test_image_source_carries_no_original_bytes(self):
        # The generated tree must describe the image's shape and never its
        # contents; contents are loaded at runtime from the user's own file.
        with tempfile.TemporaryDirectory() as raw:
            out = Path(raw)
            lifter.write_image(out, build_pe(), 0x00400000, 0x1000)
            text = (out / "lifted_image.cpp").read_text(encoding="utf-8")
        self.assertIn("unsigned char opensmacx_image[OpensmacxImageSize];",
                      text)
        self.assertNotIn("0x90,", text)

    def test_dispatch_table_holds_every_function(self):
        with tempfile.TemporaryDirectory() as raw:
            out = Path(raw)
            functions = [
                {"address": 0x00401000, "size": 8, "name": "a",
                 "section": ".text", "instructions": 2,
                 "fully_decoded": True, "state": "unrecovered"},
                {"address": 0x00402000, "size": 8, "name": "b",
                 "section": ".text", "instructions": 2,
                 "fully_decoded": True, "state": "unrecovered"},
            ]
            lifter.write_dispatch(out, functions)
            text = (out / "lifted_dispatch.cpp").read_text(encoding="utf-8")
        self.assertIn("0x00401000U, &lifted_00401000", text)
        self.assertIn("0x00402000U, &lifted_00402000", text)

    def test_selfmod_functions_still_appear_in_the_dispatch_table(self):
        # They are not lowered, but they must remain reachable: an indirect
        # call into _SELFMOD has to hit the trap and name the address, not
        # fall off the end of the table with a misleading message.
        with tempfile.TemporaryDirectory() as raw:
            out = Path(raw)
            functions = [{
                "address": 0x00664000, "size": 16, "name": "sub_664000",
                "section": "_selfmod", "instructions": 4,
                "fully_decoded": True, "state": "unrecovered",
            }]
            lifter.write_dispatch(out, functions)
            text = (out / "lifted_dispatch.cpp").read_text(encoding="utf-8")
        self.assertIn("0x00664000U, &lifted_00664000", text)


if __name__ == "__main__":
    unittest.main()
