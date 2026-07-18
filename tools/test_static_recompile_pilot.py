#!/usr/bin/env python3

from pathlib import Path
import tempfile
import unittest

from static_recompile_pilot import (
    REPO_ROOT,
    decode,
    format_source,
    lower_instruction,
)
from local_artifact import require_local_artifact_path


class StaticRecompileLoweringTests(unittest.TestCase):
    def lower(self, content):
        instructions = decode(bytes.fromhex(content), 0x00100000)
        self.assertEqual(len(instructions), 1)
        return lower_instruction(instructions[0])

    def test_lowers_partial_register_immediate_without_clobbering_upper_bits(self):
        self.assertEqual(
            self.lower("b05a"),
            ["opensmacx_set_low8(state->eax, UINT8_C(0x5A));"])

    def test_lowers_sete_through_explicit_zero_flag(self):
        self.assertIn("OpensmacxZeroFlag", " ".join(self.lower("0f94c2")))

    def test_lowers_unsigned_comparison_and_branch(self):
        compare = " ".join(self.lower("83f807"))
        self.assertIn("opensmacx_cmp32", compare)
        branch = " ".join(self.lower("7600"))
        self.assertIn("opensmacx_below_or_equal", branch)

    def test_lowers_indexed_object_byte_read(self):
        lowered = " ".join(self.lower("8a548124"))
        self.assertIn("state->eax * 4U", lowered)
        self.assertIn("UINT32_C(0x00000024)", lowered)

    def test_rejects_unsupported_instruction(self):
        instruction = decode(bytes.fromhex("90"), 0x00100000)[0]
        with self.assertRaisesRegex(RuntimeError, "unsupported instruction"):
            lower_instruction(instruction)

    def test_generated_source_is_deterministic(self):
        instructions = decode(bytes.fromhex("b001c20400"), 0x00100000)
        self.assertEqual(format_source(instructions), format_source(instructions))

    def test_rejects_branch_to_non_instruction_boundary(self):
        instructions = decode(bytes.fromhex("76ff90"), 0x00100000)
        with self.assertRaisesRegex(RuntimeError, "outside instruction boundaries"):
            format_source(instructions)


class StaticRecompileOutputTests(unittest.TestCase):
    def test_accepts_ignored_output_directory(self):
        output = REPO_ROOT / "build" / "static-recompile-output-test"
        self.assertEqual(require_local_artifact_path(output), output.resolve())

    def test_rejects_tracked_output_directory(self):
        with self.assertRaisesRegex(RuntimeError, "ignored .opensmacx/ or build/"):
            require_local_artifact_path(REPO_ROOT / "generated")

    def test_rejects_symlinked_output_ancestor(self):
        with tempfile.TemporaryDirectory(dir=REPO_ROOT / "build") as directory:
            with tempfile.TemporaryDirectory() as external:
                link = Path(directory) / "link"
                link.symlink_to(Path(external), target_is_directory=True)
                with self.assertRaisesRegex(RuntimeError, "must not contain symlinks"):
                    require_local_artifact_path(link / "generated")


if __name__ == "__main__":
    unittest.main()
