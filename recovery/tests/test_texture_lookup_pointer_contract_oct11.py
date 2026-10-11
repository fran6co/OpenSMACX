"""Use-derived pointer contracts; no unseen shade class or raster proof."""
from pathlib import Path
import subprocess
import pytest

from match_recovery.source import read_text
from test_texture_sub61f0d0_actual_source import extract_method
from test_texture_sub61f0d0_actual_source import translation_unit, build_and_run
from test_texture_raster_actual_source import _continuity_fields

ROOT = Path(__file__).resolve().parents[2]
BASE = "8e6691299"
OLD_SYMBOL = "?sub_61f0d0@Texture@@QAEXPAVBuffer@@PAUVert@@PAHHHIH@Z"
NEW_SYMBOL = "?sub_61f0d0@Texture@@QAEXPAVBuffer@@PAUVert@@PAHPBXPBIPBEH@Z"


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
    assert current==expected
    prior={r.address:r for r in read_text(before,path)}
    after={r.address:r for r in read_text(current,path)}
    assert set(prior)==set(after)
    for address,record in prior.items():
        assert _continuity_fields(record)==_continuity_fields(after[address])


def test_pinned_three_pointer_depths_and_lookup_byte_stride():
    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    import pytest
    path=ROOT/".opensmacx/game/terranx_original.exe"
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
    assert body.count("CARRY1((byte)(*blit_slot_55c),*pbVar12)")==2
    assert body.count("*piVar6 =")==0
    assert "*blit_slot_55c" in body
