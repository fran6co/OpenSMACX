#!/usr/bin/env python3

from pathlib import Path
import tempfile
import unittest

from prepare_hybrid_image import (
    classify_source_binary,
    sha256_file,
    validate_analysis_input_hashes,
)


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

    def test_analysis_inputs_require_exact_hashes(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "bindings.csv"
            path.write_text("symbol,category\n", encoding="ascii")
            summary = {"inputs": {"binding_classifications_sha256": sha256_file(path)}}
            inputs = (("binding_classifications_sha256", path,
                       "binding classifications"),)
            validate_analysis_input_hashes(summary, inputs)
            path.write_text("symbol,category\nchanged,test\n", encoding="ascii")
            with self.assertRaisesRegex(RuntimeError, "binding classifications"):
                validate_analysis_input_hashes(summary, inputs)


if __name__ == "__main__":
    unittest.main()
