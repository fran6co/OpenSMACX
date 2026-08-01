#!/usr/bin/env python3

from pathlib import Path
import unittest

import indirect_call_sites as sites


def decoder():
    import capstone
    return capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)


def image(payload, base=0x400000):
    def body(start, end):
        if start < base or start >= base + len(payload):
            return b""
        return payload[start - base:end - base]
    return body


class DetectionTests(unittest.TestCase):
    def scan(self, payload):
        start = 0x400000
        return sites.indirect_sites(image(payload), decoder(),
                                    start, start + len(payload))

    def test_direct_call_is_not_a_site(self):
        # E8 rel32 - the only kind the callgraph records.
        self.assertEqual(self.scan(b"\xE8\x10\x00\x00\x00\xC3"), [])

    def test_call_through_a_register(self):
        found = self.scan(b"\xFF\xD0\xC3")          # call eax
        self.assertEqual(len(found), 1)
        self.assertIn("eax", found[0][1])

    def test_call_through_a_vtable_slot(self):
        found = self.scan(b"\xFF\x52\x64\xC3")      # call [edx+0x64]
        self.assertEqual(len(found), 1)
        self.assertIn("0x64", found[0][1])

    def test_call_through_an_absolute_pointer(self):
        # FF 15 - the import-table shape, `call dword ptr [0x669338]`.
        found = self.scan(b"\xFF\x15\x38\x93\x66\x00\xC3")
        self.assertEqual(len(found), 1)
        self.assertIn("0x669338", found[0][1])

    def test_a_jump_is_not_a_call(self):
        self.assertEqual(self.scan(b"\xFF\xE0\xC3"), [])   # jmp eax

    def test_mixed_body_reports_only_the_indirect_ones(self):
        payload = (b"\xE8\x10\x00\x00\x00"    # direct
                   b"\xFF\xD0"                # call eax
                   b"\xE8\x20\x00\x00\x00"    # direct
                   b"\xFF\x52\x64"            # call [edx+0x64]
                   b"\xC3")
        self.assertEqual(len(self.scan(payload)), 2)

    def test_unmapped_range_yields_nothing_rather_than_raising(self):
        self.assertEqual(
            sites.indirect_sites(image(b"\xFF\xD0"), decoder(),
                                 0x900000, 0x900002),
            [])


class CommittedImageTests(unittest.TestCase):
    """Against the real executable, if it is present."""

    def setUp(self):
        if not sites.DEFAULT_EXE.is_file():
            self.skipTest("the original executable is absent")
        if not sites.DEFAULT_FUNCTIONS.is_file():
            self.skipTest("docs/recovery/functions.csv is absent")
        self.rows = sites.read_rows(sites.DEFAULT_FUNCTIONS)
        self.body, self.decoder = sites.load_image(sites.DEFAULT_EXE)

    def scan(self, address):
        row = self.rows[address]
        return sites.indirect_sites(self.body, self.decoder,
                                    int(row["address"], 16),
                                    int(row["end_address"], 16))

    def test_hline_dispatches_indirectly_despite_zero_callees(self):
        # The whole reason this tool exists: the catalogue calls it a leaf.
        row = self.rows["0x005E10C0"]
        self.assertEqual(row.get("call_target_count"), "0")
        self.assertGreaterEqual(len(self.scan("0x005E10C0")), 1)

    def test_fill_dispatches_indirectly_despite_zero_callees(self):
        self.assertGreaterEqual(len(self.scan("0x005DFCD0")), 1)

    def test_a_genuinely_direct_function_reports_none(self):
        # scan_prototypes was recovered as a real leaf; if this starts
        # reporting sites, the decoder is finding calls that are not there.
        self.assertEqual(self.scan("0x0053A4A0"), [])


class FrontierPopulationTests(unittest.TestCase):
    """The seam filter must actually filter.

    It did not. It asked the callgraph for `graph[address]["callees"]`, but the
    file is an edge list, so every lookup returned [] and `all([])` admitted the
    entire unrecovered population - 2,784 rows where 947 qualify. Nothing here
    covered this function, and the two percentages published from it were wrong
    by a factor of 2.9 in the population.
    """

    def rows(self):
        # 0x2000 is the candidate; 0x3000 is a resolved callee, 0x4000 is not.
        return {
            "0x00002000": {"address": "0x00002000", "size": "10",
                           "recovery_state": "unrecovered", "binary_kind": "game"},
            "0x00005000": {"address": "0x00005000", "size": "10",
                           "recovery_state": "unrecovered", "binary_kind": "game"},
            "0x00003000": {"address": "0x00003000", "size": "10",
                           "recovery_state": "source_complete", "binary_kind": "game"},
            "0x00004000": {"address": "0x00004000", "size": "10",
                           "recovery_state": "unrecovered", "binary_kind": "game"},
        }

    def write_graph(self, payload):
        path = Path(self.enterContext(
            __import__("tempfile").TemporaryDirectory())) / "callgraph.json"
        path.write_text(__import__("json").dumps(payload), encoding="utf-8")
        return path

    def test_a_function_with_an_unrecovered_callee_is_excluded(self):
        graph = self.write_graph({"edges": [
            {"source": "0x00002000", "target": "0x00003000"},
            {"source": "0x00005000", "target": "0x00004000"},
        ]})
        selected = {row["address"]
                    for row in sites.frontier_population(self.rows(), graph)}
        # 0x5000 calls an unrecovered function, so it must NOT qualify - that is
        # the whole job, and before the fix it appeared anyway. 0x2000 calls only
        # a source_complete one and 0x4000 has no callees at all, so both do.
        self.assertNotIn("0x00005000", selected)
        self.assertEqual({"0x00002000", "0x00004000"}, selected)

    def test_an_edge_list_read_as_a_map_would_admit_everything(self):
        # The exact regression: no edge names 0x5000's unrecovered callee, so a
        # lookup that finds nothing must not be read as "no seams".
        graph = self.write_graph({"edges": []})
        selected = sites.frontier_population(self.rows(), graph)
        self.assertEqual(3, len(selected))  # the three unrecovered game rows
        self.assertLess(len(selected), len(self.rows()),
                        "admitting every row means the filter filtered nothing")

    def test_a_callgraph_without_edges_refuses_rather_than_admitting_all(self):
        graph = self.write_graph({"function_count": 3})
        with self.assertRaises(SystemExit):
            sites.frontier_population(self.rows(), graph)


if __name__ == "__main__":
    unittest.main()
