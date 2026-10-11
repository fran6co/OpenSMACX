"""Pointer-valued Texture raster scratch is exercised through the real owner."""
from pathlib import Path
import shutil

import pytest

import test_texture_draw_coast_actual_source as coast_control


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/texture.cpp"


def test_mapped_pointer_cells_keep_original_storage_identity_and_width() -> None:
    source = SOURCE.read_text(encoding="utf-8")
    required = (
        "// RECOVERY_OBJECT: v1 0x009BB4EC _TextureRasterPixelBase\n"
        "static void *TextureRasterPixelBase;",
        "// RECOVERY_OBJECT: v1 0x009BB544 _TextureRasterScratch544\n"
        "static void *TextureRasterScratch544;",
        "// RECOVERY_OBJECT: v1 0x009BB550 _TextureRasterScratch550\n"
        "static const void *TextureRasterScratch550;",
        "// RECOVERY_OBJECT: v1 0x009BB554 _TextureRasterScratch554\n"
        "static void *TextureRasterScratch554;",
        "static void **const blit_pixel_base = &TextureRasterPixelBase;",
        "static void **const blit_texture_pixels = &TextureRasterScratch544;",
        "static const void **const blit_remap_table = &TextureRasterScratch550;",
        "static void **const blit_shade_table = &TextureRasterScratch554;",
    )
    for declaration in required:
        assert declaration in source
    assert "TextureRasterScratch550;\n// RECOVERY_OBJECT" in source
    assert "blit_remap_table[0] = a5;" in source
    assert "blit_remap_table[0] = reinterpret_cast<uintptr_t>(a5);" not in source
    assert "*blit_remap_table = reinterpret_cast<uintptr_t>(remap);" not in source
    assert "texture_pointer_offset(*blit_remap_table, texel * 4)" in source
    assert "(*blit_remap_table) = remap;" in source


def test_pinned_raster_table_cell_is_written_as_pointer_and_read_as_bytes_and_dwords() -> None:
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage

    image_path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not image_path.is_file():
        pytest.skip("pinned original unavailable")
    image = PeImage(
        image_path.read_bytes(),
        "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf",
    )
    disassembler = Cs(CS_ARCH_X86, CS_MODE_32)
    coast = {i.address: (i.mnemonic, i.op_str) for i in disassembler.disasm(
        image.read(Span(0x61EAD0, 0x61F0C6)), 0x61EAD0)}
    assert coast[0x61EB39] == ("mov", "dword ptr [0x9bb550], ebx")
    assert coast[0x61F03E] == ("add", "edx, dword ptr [0x9bb550]")
    assert coast[0x61F044] == ("mov", "dl, byte ptr [edx]")
    dword_table = {i.address: (i.mnemonic, i.op_str) for i in disassembler.disasm(
        image.read(Span(0x6202E0, 0x62063A)), 0x6202E0)}
    assert dword_table[0x6204F9] == ("shl", "eax, 2")
    assert dword_table[0x6204FC] == ("add", "eax, dword ptr [0x9bb550]")
    assert dword_table[0x620502] == ("mov", "eax, dword ptr [eax]")
    assert dword_table[0x620568] == ("shl", "eax, 2")
    assert dword_table[0x62056B] == ("add", "eax, dword ptr [0x9bb550]")
    assert dword_table[0x620571] == ("mov", "eax, dword ptr [eax]")


def test_numeric_row_address_keeps_target_width_and_shade_owner_is_mutable() -> None:
    source = SOURCE.read_text(encoding="utf-8")
    row_setup = source.index("const uint32_t row_displacement =")
    row_setup_end = source.index("if (min_x >=", row_setup)
    row_code = source[row_setup:row_setup_end]
    assert "static_cast<uint32_t>(*blit_buffer_stride) *" in row_code
    assert "static_cast<uint32_t>(min_y)" in row_code
    assert "static_cast<int>(static_cast<uint32_t>(" in row_code
    assert "texture_pointer_offset(data, row_displacement)" in row_code
    assert "data + *blit_buffer_stride" not in row_code

    shade = source.index("const unsigned char *shade_pixels =", source.index(
        "void Texture::sub_61cb50"))
    shade_end = source.index("if (shade_pixels == 0)", shade)
    shade_code = source[shade:shade_end]
    assert "shade->pixels_" in shade_code
    assert "*blit_shade_table = shade->pixels_;" in shade_code
    assert "*blit_shade_table = shade_pixels;" not in shade_code


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_coast_uses_distinct_live_pointer_cells_and_mutant_is_caught(
    compiler_name: str,
) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    helpers, setup, owner_helpers, method = coast_control.actual_parts()
    compiled = coast_control.compile_and_run(
        compiler,
        coast_control.harness(helpers, setup, owner_helpers, method),
    )
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr

    # Both pointer cells are written from their real owners before source
    # traversal; aliasing them makes the host output oracle fail.
    assert "blit_pixel_base[0] = a1->get_data();" in method
    assert "blit_texture_pixels[0] = pixels_;" in method
    mutant = method.replace(
        "blit_texture_pixels[0] = pixels_;",
        "blit_texture_pixels[0] = a4->pixels_;",
        1,
    )
    assert mutant != method
    rejected = coast_control.compile_and_run(
        compiler,
        coast_control.harness(helpers, setup, owner_helpers, mutant),
    )
    assert rejected.returncode != 0
