#!/usr/bin/env python3

import unittest

from prepare_hybrid_image import classify_source_binary


class SourceBinaryClassificationTests(unittest.TestCase):
    def test_canonical_binary(self):
        self.assertEqual(
            classify_source_binary("canonical", "canonical", "ghidra"),
            "same_binary")

    def test_independently_analyzed_binary(self):
        self.assertEqual(
            classify_source_binary("ghidra", "canonical", "ghidra"),
            "independently_analyzed_cross_build")

    def test_hash_pinned_runtime_binary(self):
        self.assertEqual(
            classify_source_binary(
                "84432e3a1465a05f32b5bb70693f5c4099661d5c1511dbbf27233b4890071b1c",
                "canonical", "ghidra"),
            "hash_pinned_runtime_build")

    def test_unknown_binary(self):
        with self.assertRaisesRegex(RuntimeError, "unsupported executable SHA-256"):
            classify_source_binary("unknown", "canonical", "ghidra")


if __name__ == "__main__":
    unittest.main()
