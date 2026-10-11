from __future__ import annotations

import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parent
SRC = ROOT.parents[1] / "src"


def test_actual_frame_body_rejects_schedule_and_pitch_mutants(tmp_path: Path) -> None:
    source = (SRC / "movie_decoder_frame.cpp").read_text()
    harness = ROOT / "movie_decoder_frame_test.cpp"
    mutants = {
        "wrong_block_x_step": (
            "x += 0x10",
            "x += 0x08",
        ),
        "wrong_byte_offset": (
            "((pixel_index >> 1) << 2)",
            "(pixel_index << 2)",
        ),
        "destination_pitch_from_width": (
            "destination += args.output_pitch",
            "destination += args.width",
        ),
        "wrong_selected_plane": (
            "args.record_kind == 7\n      ? *args.planes->temporary_b",
            "args.record_kind != 7\n      ? *args.planes->temporary_b",
        ),
        "include_final_row": (
            "row < args.height - 1",
            "row < args.height",
        ),
        "packet_words_after_count_publication": (
            "const uint32_t packet_accumulator =\n"
            "      (static_cast<uint32_t>(read_u16(args.compressed + 0x10)) << 16) |\n"
            "      read_u16(args.compressed + 0x12);\n"
            "  const uint8_t *const packet_cursor = args.compressed + 0x14;",
            "args.bit_state->available_bits = 0x20;\n"
            "  const uint32_t packet_accumulator =\n"
            "      (static_cast<uint32_t>(read_u16(args.compressed + 0x10)) << 16) |\n"
            "      read_u16(args.compressed + 0x12);\n"
            "  const uint8_t *const packet_cursor = args.compressed + 0x14;",
        ),
        "record_mode_before_reader_publication": (
            "args.bit_state->available_bits = 0x20;\n"
            "  args.bit_state->accumulator = packet_accumulator;\n"
            "  const int32_t first_scaled = signed_word(\n"
            "      static_cast<uint32_t>(args.transform_products[0]) << 15);\n"
            "  args.bit_state->cursor = packet_cursor;\n"
            "  *args.record_mode = record_mode_value;",
            "*args.record_mode = record_mode_value;\n"
            "  args.bit_state->available_bits = 0x20;\n"
            "  args.bit_state->accumulator = packet_accumulator;\n"
            "  const int32_t first_scaled = signed_word(\n"
            "      static_cast<uint32_t>(args.transform_products[0]) << 15);\n"
            "  args.bit_state->cursor = packet_cursor;",
        ),
    }

    for name, (old, replacement) in mutants.items():
        assert source.count(old) == 1, (name, source.count(old))
        mutated = source.replace(old, replacement, 1)
        mutated_path = tmp_path / f"{name}.cpp"
        executable = tmp_path / name
        mutated_path.write_text(mutated)
        compile_result = subprocess.run(
            [
                "g++",
                "-std=c++17",
                "-Wall",
                "-Wextra",
                "-Werror",
                "-pedantic",
                "-iquote",
                str(SRC),
                str(mutated_path),
                str(harness),
                "-o",
                str(executable),
            ],
            capture_output=True,
            text=True,
            check=False,
        )
        assert compile_result.returncode == 0, compile_result.stderr
        execution = subprocess.run(
            [str(executable)], capture_output=True, text=True, check=False
        )
        assert execution.returncode != 0, name
        if name == "packet_words_after_count_publication":
            assert "alias_bits->accumulator == expected_accumulator" in execution.stderr
        elif name == "record_mode_before_reader_publication":
            assert "alias_bits->available_bits == 1" in execution.stderr
