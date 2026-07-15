#!/usr/bin/env python3

from collections import defaultdict
import unittest

from capstone import CS_ARCH_X86, CS_MODE_32, Cs

from extract_legacy_leaves import (
    REPO_ROOT,
    classify_function,
    select_rows,
    validate_output_path,
)


class LegacyLeafClassificationTest(unittest.TestCase):
    START = 0x00100000
    IMAGE_START = 0x00400000
    IMAGE_END = 0x00A10000

    def setUp(self):
        self.disassembler = Cs(CS_ARCH_X86, CS_MODE_32)
        self.disassembler.detail = True

    def classify(self, code, relocations=(), references=None):
        reason, _instructions = classify_function(
            self.disassembler,
            code,
            self.START,
            self.START + len(code),
            self.IMAGE_START,
            self.IMAGE_END,
            relocations,
            {self.START},
            references or defaultdict(set),
            512,
        )
        return reason

    def test_accepts_closed_stack_relative_leaf(self):
        self.assertEqual(self.classify(bytes.fromhex("8B442404C3")), "candidate")

    def test_accepts_closed_internal_loop(self):
        self.assertEqual(self.classify(bytes.fromhex("31C941E2FDC3")), "candidate")

    def test_rejects_call(self):
        self.assertEqual(
            self.classify(bytes.fromhex("E800000000C3")), "contains_call")

    def test_rejects_external_loop(self):
        self.assertEqual(
            self.classify(bytes.fromhex("E27FC3")), "contains_external_branch")

    def test_rejects_reachable_fallthrough(self):
        self.assertEqual(
            self.classify(bytes.fromhex("7401C390")), "reachable_fallthrough")

    def test_rejects_mid_instruction_branch(self):
        self.assertEqual(
            self.classify(bytes.fromhex("EB01B800000000C3")),
            "branch_targets_mid_instruction")

    def test_rejects_indexed_image_displacement(self):
        self.assertEqual(
            self.classify(bytes.fromhex("8B8000104000C3")),
            "contains_absolute_memory")

    def test_rejects_absolute_zero_address(self):
        self.assertEqual(
            self.classify(bytes.fromhex("A100000000C3")),
            "contains_absolute_memory")

    def test_rejects_instruction_pointer_environment_save(self):
        self.assertEqual(
            self.classify(bytes.fromhex("D9742404C3")),
            "observes_instruction_pointer")

    def test_rejects_unreachable_bytes(self):
        self.assertEqual(
            self.classify(bytes.fromhex("C390")), "contains_unreachable_bytes")

    def test_rejects_overlapping_relocation(self):
        self.assertEqual(
            self.classify(bytes.fromhex("C3"), [(self.START - 1, self.START + 1)]),
            "contains_relocation")

    def test_rejects_external_interior_reference(self):
        references = defaultdict(set)
        references[self.START + 1].add(self.START + 0x100)
        self.assertEqual(
            self.classify(bytes.fromhex("90C3"), references=references),
            "contains_external_interior_reference")


class LegacyLeafSelectionTest(unittest.TestCase):
    ROWS = [
        {"address": "0x00401000", "binary_kind": "game",
         "recovery_state": "unrecovered"},
        {"address": "0x00402000", "binary_kind": "game",
         "recovery_state": "source_complete"},
        {"address": "0x00403000", "binary_kind": "library",
         "recovery_state": "unrecovered"},
    ]

    def test_default_selection_uses_unrecovered_game_functions(self):
        self.assertEqual(select_rows(self.ROWS, []), [self.ROWS[0]])

    def test_explicit_selection_includes_recovered_function(self):
        self.assertEqual(
            select_rows(self.ROWS, [0x00402000]), [self.ROWS[1]])

    def test_explicit_selection_rejects_unknown_address(self):
        with self.assertRaisesRegex(RuntimeError, "0x00404000"):
            select_rows(self.ROWS, [0x00404000])


class LegacyLeafOutputTest(unittest.TestCase):
    def test_accepts_ignored_output_roots(self):
        validate_output_path((REPO_ROOT / "build" / "oracle-test").resolve())
        validate_output_path((REPO_ROOT / ".opensmacx" / "oracle-test").resolve())

    def test_rejects_tracked_output_location(self):
        with self.assertRaisesRegex(RuntimeError, "ignored .opensmacx/ or build/"):
            validate_output_path((REPO_ROOT / "docs" / "oracle-test").resolve())

    def test_rejects_parent_traversal_out_of_ignored_root(self):
        with self.assertRaisesRegex(RuntimeError, "ignored .opensmacx/ or build/"):
            validate_output_path(REPO_ROOT / "build" / ".." / "docs" / "oracle-test")

    def test_rejects_replacing_an_ignored_root(self):
        with self.assertRaisesRegex(RuntimeError, "ignored .opensmacx/ or build/"):
            validate_output_path((REPO_ROOT / "build").resolve())


if __name__ == "__main__":
    unittest.main()
