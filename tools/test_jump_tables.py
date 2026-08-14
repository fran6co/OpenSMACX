#!/usr/bin/env python3
"""A resolved jump table is only useful if it stops where the table stops.

An over-long table is worse than no table: the brief prints addresses that
belong to whatever follows in the data section, and an agent trusts them
because they look like the real thing. So what is pinned here is the bounds,
not the finding.
"""
from __future__ import annotations

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import byte_match                                             # noqa: E402
import jump_tables as tool                                    # noqa: E402

HAVE_IMAGE = byte_match.DEFAULT_EXE.is_file()


@unittest.skipUnless(HAVE_IMAGE, "needs the shipped terranx.exe")
class BoundsTests(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        import pefile
        cls.pe = pefile.PE(str(byte_match.DEFAULT_EXE), fast_load=True)
        cls.low, cls.high = tool.code_bounds(cls.pe)

    def test_every_entry_is_inside_text(self):
        # The stop condition. Without it a table runs into the next object in
        # .rdata and the brief prints fiction with the confidence of fact.
        for address in (0x00490B00, 0x0045B550, 0x00636960):
            import emit_translation_unit as emit
            row = emit.load_functions()[address]
            spans = emit.parse_body_ranges(row.get("body_ranges") or "")
            for table in tool.tables_in(self.pe, spans):
                for target in table["targets"]:
                    self.assertTrue(self.low <= target < self.high,
                                    f"0x{target:08X} from 0x{address:08X}")

    def test_a_table_is_found_where_agents_reported_guessing(self):
        # 0x00490B00: "3 byte-indexed jump-table dispatches simplified to a
        # uniform field test (table bytes not in the brief)".
        # 0x00636960: "the 6-entry jmp table is data so its case targets are
        # unrecoverable from this brief".
        import emit_translation_unit as emit
        functions = emit.load_functions()
        for address, least in ((0x00490B00, 8), (0x00636960, 6),
                               (0x0045B550, 4)):
            spans = emit.parse_body_ranges(
                functions[address].get("body_ranges") or "")
            tables = tool.tables_in(self.pe, spans, functions)
            self.assertTrue(tables, f"no table at 0x{address:08X}")
            self.assertGreaterEqual(
                max(len(t["targets"]) for t in tables), least,
                f"0x{address:08X}")

    def test_targets_land_inside_or_near_the_function(self):
        # A switch jumps WITHIN its own function. An entry far outside the
        # span is the strongest sign the table ran on past its end.
        import emit_translation_unit as emit
        functions = emit.load_functions()
        row = functions[0x00490B00]
        spans = emit.parse_body_ranges(row.get("body_ranges") or "")
        span_low = min(low for low, _ in spans)
        span_high = max(high for _, high in spans)
        for table in tool.tables_in(self.pe, spans, functions):
            for target in table["targets"]:
                self.assertTrue(span_low <= target <= span_high,
                                f"0x{target:08X} outside "
                                f"0x{span_low:08X}-0x{span_high:08X}")

    def test_a_function_with_no_switch_yields_nothing(self):
        # Silence must mean "no table", never "the reader gave up".
        import emit_translation_unit as emit
        functions = emit.load_functions()
        row = functions[0x004E10C0]          # Console::editor_fast, no switch
        spans = emit.parse_body_ranges(row.get("body_ranges") or "")
        self.assertEqual(tool.tables_in(self.pe, spans, functions), [])
        self.assertEqual(tool.render(self.pe, spans, functions), "")


if __name__ == "__main__":
    unittest.main()
