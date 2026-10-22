"""Ensure the complete Gouraud row loops live in their mapped owner bodies.

The companion host fixture executes these literal bodies. This test only
guards the structural boundary: the two selected entries may not delegate
their complete row traversal to the retired private helper.
"""
from __future__ import annotations

import pytest

import test_texture_raster_actual_source as raster
import test_texture_gouraud_key_policy_oct20e as key_policy


def test_gouraud_row_work_is_in_both_mapped_owner_bodies():
    source = raster.TEXTURE.read_text(encoding="utf-8")
    definitions = raster.actual_installed_definitions(source)
    by_signature = {
        raster.METHODS[index]: definitions[len(raster.HELPERS) + index]
        for index in range(len(raster.METHODS))
    }

    assert "static void texture_raster_gouraud_rows(" not in source
    for signature in (raster.METHODS[0], raster.METHODS[2]):
        body = by_signature[signature]
        assert "texture_raster_gouraud_rows" not in body
        assert "while (row_offset < TextureRasterClipBottom)" in body or \
               "while (TextureRasterRowOffset < TextureRasterClipBottom)" in body
        assert body.count("setup_edge_gouraud(") == 6
        assert "buffer->free_data(1)" in body
        assert "texture_reset_fpu();" in body


def test_key_policy_is_attached_to_the_pinned_borrowed_branch_only():
    source = raster.TEXTURE.read_text(encoding="utf-8")
    entry = raster._extract_definition(source, raster.METHODS[0])
    borrowed_start = entry.index("if (borrowed) {")
    owned_start = entry.index(
        "            } else {\n                int row_offset = TextureRasterRowOffset;\n"
        "                const int pitch = TextureRowPitchScratch;",
        borrowed_start,
    )
    assert "if (texel != key) {" in entry[borrowed_start:owned_start]
    assert "if (texel != key) {" not in entry[owned_start:]
    assert "if (true) {" not in entry


def _owned_branch(entry):
    borrowed_start = entry.index("if (borrowed) {")
    return entry.index(
        "            } else {\n                int row_offset = TextureRasterRowOffset;\n"
        "                const int pitch = TextureRowPitchScratch;",
        borrowed_start,
    )


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_clipped_horizontal_skip_retains_all_32_bits(compiler, tmp_path):
    source = raster.TEXTURE.read_text(encoding="utf-8")
    entry = raster._extract_definition(source, raster.METHODS[0])
    original = "static_cast<int64_t>(du) * skip"
    assert entry.count(original) >= 1
    narrowed = entry.replace(
        original,
        "static_cast<int64_t>(du) * static_cast<unsigned short>(skip)",
        1,
    )
    assert narrowed != entry
    mutated = source.replace(entry, narrowed, 1)
    ran = key_policy._run(
        raster._translation_unit(mutated), compiler, tmp_path,
        "texture_gouraud_narrow_clip_skip_mutant",
    )
    assert ran.returncode != 0, "narrowing the real clip skip escaped the wide-span control"


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_owned_light_accumulator_retains_16_bit_wrap(compiler, tmp_path):
    source = raster.TEXTURE.read_text(encoding="utf-8")
    entry = raster._extract_definition(source, raster.METHODS[0])
    owned_at = _owned_branch(entry)
    old = "((light + static_cast<uint32_t>(dl)) &\n                                     0xffffu);"
    assert entry[owned_at:].count(old) == 1
    owned = entry[owned_at:].replace(
        old, "((light + static_cast<uint32_t>(dl)) &\n                                     0xffu);", 1
    )
    mutated_entry = entry[:owned_at] + owned
    mutated = source.replace(entry, mutated_entry, 1)
    ran = key_policy._run(
        raster._translation_unit(mutated), compiler, tmp_path,
        "texture_gouraud_narrow_light_wrap_mutant",
    )
    assert ran.returncode != 0, "narrowing owned Gouraud light wrap escaped pixel controls"


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_61d090_clip_skip_keeps_full_width(compiler, tmp_path):
    source = raster.TEXTURE.read_text(encoding="utf-8")
    entry = raster._extract_definition(source, raster.METHODS[2])
    original = "static_cast<int64_t>(du) * skip"
    assert entry.count(original) >= 1
    narrowed = entry.replace(
        original,
        "static_cast<int64_t>(du) * static_cast<unsigned short>(skip)",
        1,
    )
    mutated = source.replace(entry, narrowed, 1)
    ran = key_policy._run(
        raster._translation_unit(mutated), compiler, tmp_path,
        "texture_61d090_narrow_clip_skip_mutant",
    )
    assert ran.returncode != 0, "narrowing the 0x61D090 clip skip escaped its wide-span control"


@pytest.mark.parametrize("branch", ("borrowed", "owned"))
@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_61d090_modes_retain_16_bit_light_wrap(branch, compiler, tmp_path):
    source = raster.TEXTURE.read_text(encoding="utf-8")
    entry = raster._extract_definition(source, raster.METHODS[2])
    old = "((light_accumulator + light_step) & 0xffffu);"
    assert entry.count(old) == 2
    positions = [index for index in range(len(entry)) if entry.startswith(old, index)]
    occurrence = 0 if branch == "borrowed" else 1
    position = positions[occurrence]
    replacement = "((light_accumulator + light_step) & 0xffu);"
    mutated_entry = entry[:position] + replacement + entry[position + len(old):]
    mutated = source.replace(entry, mutated_entry, 1)
    assert mutated != source
    ran = key_policy._run(
        raster._translation_unit(mutated), compiler, tmp_path,
        f"texture_61d090_{branch}_narrow_light_wrap_mutant",
    )
    assert ran.returncode != 0, f"narrowing {branch} 0x61D090 light wrap escaped pixel controls"
