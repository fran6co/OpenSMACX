"""Use-derived pointer contracts; no unseen shade class or raster proof."""
from pathlib import Path
from dataclasses import replace
import re
import subprocess
import pytest

from match_recovery.source import read_text
from test_texture_sub61f0d0_actual_source import extract_method
from test_texture_sub61f0d0_actual_source import translation_unit, build_and_run
from test_texture_raster_actual_source import _continuity_fields
from recovery.tests.texture_direct_storage_host import reverse_direct_storage_source
from recovery.tests.test_texture_edge_state_oct13al import (
    _expected_current_body as _expected_edge_state_body,
    _tokens as _tokens,
)
from recovery.tests.test_texture_shaded_edge_record_oct13an import (
    NOTE as SHADED_EDGE_NOTE,
    _expected_current_body as _expected_shaded_edge_body,
)

ROOT = Path(__file__).resolve().parents[2]
BASE = "8e6691299"
OLD_SYMBOL = "?sub_61f0d0@Texture@@QAEXPAVBuffer@@PAUVert@@PAHHHIH@Z"
NEW_SYMBOL = "?sub_61f0d0@Texture@@QAEXPAVBuffer@@PAUVert@@PAHPBXPBIPBEH@Z"
DIRECT_STORAGE_BASE = "f14619684801a682ea985c4ddefda40757422964"
DRAW_TRANS2_NOTE = """// STRUCTURAL EVIDENCE: Pinned setup_edge 0x624D80 writes EdgeScan offsets
// +0x28/+0x2c/+0x30 as fracAccum/fracDelta/fracSub; draw_trans2 row blocks
// 0x619FE2-0x61A000 and 0x61A039-0x61A074 confirm the matching DDA updates.
// This supersedes the former local-overlay interpretation above.
"""


def project_prior_edge_scan_migrations(source, baseline):
    """Reverse only the two already-reviewed EdgeScan changes for old snapshots."""
    current_draw = extract_method(source, "int Texture::draw_trans2(Buffer *a1, Vert *a2, int a3)")
    old_draw = extract_method(baseline, "int Texture::draw_trans2(Buffer *a1, Vert *a2, int a3)")
    assert _tokens(current_draw) == _tokens(_expected_edge_state_body(old_draw)), (
        "draw_trans2 must match only the pinned EdgeLocal-to-EdgeScan transition")
    source = source.replace(current_draw, old_draw, 1)

    marker = "// ORIGINAL: 0x00619D80 Texture::draw_trans2"
    draw_signature = "int Texture::draw_trans2(Buffer *a1, Vert *a2, int a3)"
    current_marker = source.index(marker)
    baseline_marker = baseline.index(marker)
    current_sig = source.index(draw_signature, current_marker)
    baseline_sig = baseline.index(draw_signature, baseline_marker)
    current_draw_region = source[current_marker:current_sig]
    baseline_draw_region = baseline[baseline_marker:baseline_sig]
    draw_calls = "// calls     0x005E3373 0x005E34A3 0x00624D80\n"
    assert baseline_draw_region.count(draw_calls) == 1
    draw_note_at = baseline_draw_region.index(draw_calls) + len(draw_calls) + 1
    assert baseline_draw_region[draw_note_at:draw_note_at + 1] == "\n"
    expected_draw_region = (baseline_draw_region[:draw_note_at] + DRAW_TRANS2_NOTE +
                            baseline_draw_region[draw_note_at:])
    assert current_draw_region == expected_draw_region, (
            "draw_trans2 annotation may differ only by its reviewed EdgeScan evidence note")
    source = source[:current_marker] + baseline_draw_region + source[current_sig:]

    edge_local_start = baseline.index("struct EdgeLocal {")
    edge_local_end = baseline.index(marker, edge_local_start)
    edge_local = baseline[edge_local_start:edge_local_end]
    assert source.count(marker) == 1 and source.count("struct EdgeLocal {") == 0
    source = source.replace(marker, edge_local + marker, 1)

    current_shaded = extract_method(source, "void Texture::sub_61c630(Buffer * bufIn, Vert * verts, Texture * texB, unsigned char * shadeTable, int vertCount, Vert *edge_vertices)")
    old_shaded = extract_method(baseline, "void Texture::sub_61c630(Buffer * bufIn, Vert * verts, Texture * texB, unsigned char * shadeTable, int vertCount, Vert *edge_vertices)")
    assert _tokens(current_shaded) == _tokens(_expected_shaded_edge_body(old_shaded)), (
        "sub_61c630 must match only the pinned zeroed-array-to-EdgeScan transition")
    source = source.replace(current_shaded, old_shaded, 1)
    shaded_marker = "// ORIGINAL: 0x0061C630 sub_61c630"
    shaded_signature = "void Texture::sub_61c630(Buffer * bufIn, Vert * verts, Texture * texB, unsigned char * shadeTable, int vertCount, Vert *edge_vertices)"
    current_marker = source.index(shaded_marker)
    baseline_marker = baseline.index(shaded_marker)
    current_sig = source.index(shaded_signature, current_marker)
    baseline_sig = baseline.index(shaded_signature, baseline_marker)
    current_region = source[current_marker:current_sig]
    baseline_region = baseline[baseline_marker:baseline_sig]
    generated = "// GENERATED SKELETON - tools/emit_translation_unit.py"
    assert baseline_region.count(generated) == 1
    expected_region = baseline_region.replace(
        generated, SHADED_EDGE_NOTE + "\n" + generated, 1)
    assert current_region == expected_region, (
            "sub_61c630 annotation may differ only by its reviewed uninitialized-read evidence note")
    source = source[:current_marker] + baseline_region + source[current_sig:]
    return source


def project_raster_pointer_cell_change(source):
    """Project the later 9bb550 pointer-cell migration out of this checkpoint."""
    def exact(before, after, count=1):
        nonlocal source
        assert source.count(before) == count, (before, source.count(before), count)
        source = source.replace(before, after)

    exact("// Callers publish a table pointer here. The pinned 0x61EB05/0x61EB39 path\n"
          "// writes the remap pointer, 0x61F03E/0x61F044 consumes byte entries, and\n"
          "// 0x6204F9/0x6204FC/0x620502 reads four-byte entries from the same cell. The\n"
          "// pointer is generic because these consumers use different table element\n"
          "// widths; this does not assert a capacity for every producer.\n", "")
    exact("static const void *TextureRasterScratch550;", "static uintptr_t TextureRasterScratch550;")
    exact("static const void **const blit_remap_table = &TextureRasterScratch550;",
          "static uintptr_t *const blit_remap_table = &TextureRasterScratch550;")
    exact("blit_remap_table[0] = a5;", "blit_remap_table[0] = reinterpret_cast<uintptr_t>(a5);")
    exact("*blit_remap_table = remap;", "*blit_remap_table = reinterpret_cast<uintptr_t>(remap);", 4)
    exact("*blit_remap_table = a5;", "*blit_remap_table = reinterpret_cast<uintptr_t>(a5);")
    exact("*blit_remap_table = a6;", "*blit_remap_table = reinterpret_cast<uintptr_t>(a6);")
    exact("*blit_remap_table = shadeTable;", "*blit_remap_table = reinterpret_cast<uintptr_t>(shadeTable);")
    exact("(*blit_remap_table) = remap;", "(*blit_remap_table) = texture_pointer_address(remap);")

    # Restore the former address-to-pointer spelling at byte-table consumers.
    exact("static_cast<const unsigned char *>(*blit_remap_table)",
          "reinterpret_cast<const unsigned char *>(*blit_remap_table)", 4)
    byte_cast = "reinterpret_cast<const unsigned char *>(*blit_remap_table)"
    assert source.count(byte_cast) == 4
    first_byte_cast = source.index(byte_cast)
    source = source[:first_byte_cast] + "reinterpret_cast<const unsigned char *>(static_cast<intptr_t>(*blit_remap_table))" + source[first_byte_cast + len(byte_cast):]
    byte_cast = "reinterpret_cast<const unsigned char *>(*blit_remap_table)"
    assert source.count(byte_cast) == 3
    last_byte_cast = source.rfind(byte_cast)
    source = source[:last_byte_cast] + "reinterpret_cast<unsigned char *>(*blit_remap_table)" + source[last_byte_cast + len(byte_cast):]
    exact("const unsigned char *table =\n                                                reinterpret_cast<unsigned char *>(*blit_remap_table);",
          "unsigned char *table =\n                                                reinterpret_cast<unsigned char *>(*blit_remap_table);")

    # Restore the original byte-offset lookup and four-byte-entry lookup
    # expressions, with exact occurrence counts so this remains a narrow
    # historical projection rather than a generic cast normalizer.
    exact("""texture_pointer_offset(*blit_remap_table, static_cast<uint32_t>(concat22(
                                                        static_cast<unsigned short>(
                                                            static_cast<unsigned int>(texture_pointer_address(*blit_shade_table)) >> 16),
                                                        static_cast<unsigned short>(
                                                            reinterpret_cast<unsigned int>(puVar11)))))""",
          "static_cast<unsigned int>(concat22(\n                                                        static_cast<unsigned short>(\n                                                            static_cast<unsigned int>(texture_pointer_address(*blit_shade_table)) >> 16),\n                                                        static_cast<unsigned short>(\n                                                            reinterpret_cast<unsigned int>(puVar11)))) +\n                                                    *blit_remap_table")
    exact("unsigned char texel = *reinterpret_cast<const unsigned char *>(\n                                                    static_cast<unsigned int>(concat22(",
          "unsigned char texel = *reinterpret_cast<unsigned char *>(\n                                                    static_cast<unsigned int>(concat22(")
    source, count = re.subn(
        r"\*reinterpret_cast<const unsigned int \*>\(\s*texture_pointer_offset\(\*blit_remap_table, texel \* 4\)\)",
        "*reinterpret_cast<int *>((*blit_remap_table) + texel * 4)", source)
    assert count == 4, count
    dword_index = re.compile(
        r"\*reinterpret_cast<const int \*>\(texture_pointer_offset\(\s*\*blit_remap_table,\s*static_cast<uint32_t>\(\*reinterpret_cast<const unsigned char \*>\(\*blit_slot_558\)\) \* 4\)\)")
    matches = list(dword_index.finditer(source))
    assert len(matches) == 6, len(matches)
    spellings = (
        "*reinterpret_cast<int *>((*reinterpret_cast<const unsigned char *>(*blit_slot_558)) * 4 + *blit_remap_table)",
        "*(int *)((uint)(*(byte *)(*blit_slot_558)) * 4 + (*blit_remap_table))",
    )
    for index, match in reversed(list(enumerate(matches))):
        source = source[:match.start()] + spellings[0 if index < 3 else 1] + source[match.end():]
    return source


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("damage", ("none", "lost_indirection", "truncated_lookup"))
def test_actual_prefix_load_and_full_lookup_address(compiler, damage):
    body=extract_method((ROOT/"src/texture.cpp").read_text())
    if damage=="lost_indirection":
        before="std::memcpy(&shade_pixels, shade_source, sizeof shade_pixels);"
        assert body.count(before)==1
        body=body.replace(before,"std::memcpy(&shade_pixels, &shade_source, sizeof shade_pixels);",1)
    elif damage=="truncated_lookup":
        before="static_cast<uint32_t>(texture_pointer_address(lookup_bytes))"
        assert body.count(before)==1
        body=body.replace(before,"static_cast<unsigned char>(texture_pointer_address(lookup_bytes))",1)
    build_and_run(translation_unit(body),compiler,expect_success=damage=="none")


def test_pointer_contract_changes_only_selected_body_and_symbol_bindings():
    path = ROOT / "src/texture.cpp"
    before = subprocess.check_output(["git","show",BASE+":src/texture.cpp"],cwd=ROOT,text=True)
    current = path.read_text()
    old = extract_method(before, "void Texture::sub_61f0d0(Buffer *param_2, Vert *param_3, int *param_4, int param_5,")
    edits = (
        ("void Texture::sub_61f0d0(Buffer *param_2, Vert *param_3, int *param_4, int param_5,\n                             int param_6, uint32_t param_7, int param_8) {",
         "void Texture::sub_61f0d0(Buffer *param_2, Vert *param_3, int *param_4,\n                             const void *shade_source, const uint32_t *remap,\n                             const unsigned char *lookup_bytes, int param_8) {\n    // Native argument slots are reused for fixed-point accumulators only\n    // after pointer publication. Keep those numeric lifetimes separate.\n    int param_5;\n    uint32_t param_7;"),
        ("(param_6 != 0)", "(remap != 0)"),
        ("((param_5 != 0 && (param_7 != 0)))", "((shade_source != 0 && (lookup_bytes != 0)))"),
        ("      (*blit_slot_55c) = param_7;\n      (*blit_remap_table) = param_6;\n      (*blit_shade_table) = reinterpret_cast<void *>(\n          static_cast<uintptr_t>(*reinterpret_cast<int32_t *>(param_5)));",
         "      (*blit_slot_55c) = static_cast<uint32_t>(texture_pointer_address(lookup_bytes));\n      (*blit_remap_table) = texture_pointer_address(remap);\n      // Only a leading pointer-sized cell is proved. Copy its representation\n      // without overlaying an unproved class or aliasing a Texture member.\n      void *shade_pixels;\n      std::memcpy(&shade_pixels, shade_source, sizeof shade_pixels);\n      (*blit_shade_table) = shade_pixels;"),
    )
    body = old
    for left,right in edits:
        assert body.count(left)==1, left
        body=body.replace(left,right,1)
    assert before.count(old)==1
    expected=before.replace(old,body,1)
    assert expected.count(OLD_SYMBOL)==2
    expected=expected.replace(OLD_SYMBOL,NEW_SYMBOL)
    # First reverse the exact alias-removal projection, then only the two
    # separately reviewed EdgeScan migrations and the later pointer-cell move.
    storage_base = subprocess.check_output(
        ["git", "show", f"{DIRECT_STORAGE_BASE}:src/texture.cpp"],
        cwd=ROOT, text=True)
    restored = reverse_direct_storage_source(current, storage_base)
    restored = project_raster_pointer_cell_change(restored)
    restored = project_prior_edge_scan_migrations(restored, before)
    restored=restored.replace("static unsigned char *TextureRasterPalette;", "static int TextureRasterPalette;")
    restored=restored.replace("static unsigned char **const blit_slot_564 = &TextureRasterPalette;",
                              "static int *const blit_slot_564 = &TextureRasterPalette;")
    restored=restored.replace("*blit_slot_564 = palette;",
                              "*blit_slot_564 = static_cast<int>(reinterpret_cast<intptr_t>(palette));")
    assert restored==expected
    prior={r.address:r for r in read_text(before,path)}
    asset_path = ROOT / "src/texture_objects.cpp"
    assets = read_text(asset_path.read_text(), asset_path)
    rows = read_text(current,path) + assets
    after={r.address:r for r in rows}
    assert len(rows) == len(after), "original identities must have unique owners"
    assert set(prior)==set(after)
    asset_addresses = {r.address for r in assets}
    assert len(asset_addresses) == 40
    for address,record in prior.items():
        actual = after[address]
        if address in asset_addresses:
            assert not record.byte_exact or actual.byte_exact
            actual = replace(actual, byte_exact=record.byte_exact)
        assert _continuity_fields(record)==_continuity_fields(actual)


def test_pinned_three_pointer_depths_and_lookup_byte_stride():
    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    import pytest
    path=ROOT/".opensmacx/game/terranx_original.exe"
    if not path.is_file():
        path = ROOT.parents[2] / ".opensmacx/game/terranx_original.exe"
    if not path.is_file():
        pytest.skip("pinned original unavailable")
    image=PeImage(path.read_bytes(),"01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf")
    instructions={i.address:(i.mnemonic,i.op_str) for i in
        Cs(CS_ARCH_X86,CS_MODE_32).disasm(image.read(Span(0x61F0D0,0x61F785)),0x61F0D0)}
    for address,instruction in {
        0x61F108:("mov","edi, dword ptr [ebp + 0x18]"),
        0x61F1AE:("mov","ecx, dword ptr [ebp + 0x14]"),
        0x61F1B7:("mov","dword ptr [0x9bb55c], eax"),
        0x61F1BC:("mov","dword ptr [0x9bb550], edi"),
        0x61F1C2:("mov","eax, dword ptr [ecx]"),
        0x61F1C6:("mov","dword ptr [0x9bb554], eax"),
    }.items():
        assert instructions[address]==instruction
    # Original byte-add/carry address construction still lives in the raster
    # body; this input migration must not turn it into DWORD-index addressing.
    body=extract_method((ROOT/"src/texture.cpp").read_text())
    assert body.count("CARRY1((byte)(TextureRasterScratch55C),*pbVar12)")==2
    assert body.count("*piVar6 =")==0
    assert "(TextureRasterScratch55C) = static_cast<uint32_t>(texture_pointer_address(lookup_bytes));" in body
