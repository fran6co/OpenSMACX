"""Shared palette pointer storage, not complete raster/runtime equivalence."""
import json
from pathlib import Path
import subprocess
import shutil
import struct

import pytest

from match_recovery.source import read_text
from match_recovery.tasks import Task

ROOT = Path(__file__).resolve().parents[2]
BASE = "268fb06faa36d40050b29eaa068b954e40a30615"
EDITS = (
    ("static int TextureRasterPalette;", "static unsigned char *TextureRasterPalette;", 1),
    ("static int *const blit_slot_564 = &TextureRasterPalette;",
     "static unsigned char **const blit_slot_564 = &TextureRasterPalette;", 1),
    ("*blit_slot_564 = static_cast<int>(reinterpret_cast<intptr_t>(palette));",
     "*blit_slot_564 = palette;", 2),
)


@pytest.mark.parametrize("owner", ("mask", "dual"))
@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (False, True))
def test_actual_palette_publication_keeps_high_host_pointer(owner, compiler_name, damage, tmp_path):
    if struct.calcsize("P") != 8:
        pytest.skip("high-address control requires a 64-bit host")
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    if owner == "mask":
        from recovery.tests.test_texture_draw_mask_actual_source import _actual_tu
        source, _ = _actual_tu((ROOT / "src/texture.cpp").read_text())
        before = "static unsigned char palette[256 * 256];"
        assert source.count(before) == 1
        source = source.replace(before, "unsigned char palette[256 * 256];\n"
            "    assert(reinterpret_cast<uintptr_t>(palette) > UINT32_MAX);")
    else:
        from recovery.tests.test_texture_sub61df60_actual_source import live_translation_unit
        source = live_translation_unit()
        assert source.count("static unsigned char palette_data[65536];") == 1
        source = source.replace("static unsigned char palette_data[65536];",
                                "static unsigned char *palette_data;")
        source = source.replace("memset(palette_data, 0, sizeof(palette_data));",
                                "memset(palette_data, 0, 65536);")
        assert source.count("int main() {") == 1
        source = source.replace("int main() {", "int main() {\n"
            "    unsigned char high_palette[65536];\n"
            "    assert(reinterpret_cast<uintptr_t>(high_palette) > UINT32_MAX);\n"
            "    palette_data = high_palette;")
    before = "*blit_slot_564 = palette;"
    assert source.count(before) == 1
    if damage:
        source = source.replace(before, "*blit_slot_564 = reinterpret_cast<unsigned char *>("
            "static_cast<uintptr_t>(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(palette))));")
    cpp, binary = tmp_path / "control.cpp", tmp_path / "control"
    cpp.write_text(source)
    standard = "-std=c++11" if owner == "mask" else "-std=c++98"
    built = subprocess.run([compiler, standard, "-no-pie", str(cpp), "-o", str(binary)],
                           capture_output=True, text=True, timeout=60)
    assert built.returncode == 0, built.stderr
    run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)
    assert (run.returncode == 0) == (not damage), run.stderr


def test_exact_bounded_storage_migration_and_identity_continuity():
    path = ROOT / "src/texture.cpp"
    before = subprocess.check_output(["git", "show", BASE + ":src/texture.cpp"], text=True)
    expected = before
    for old, new, count in EDITS:
        assert expected.count(old) == count
        expected = expected.replace(old, new)
    assert path.read_text() == expected
    from recovery.tests.test_texture_raster_actual_source import _continuity_fields
    assert [_continuity_fields(r) for r in read_text(before, path)] == [
        _continuity_fields(r) for r in read_text(expected, path)]
    assert expected.count("// RECOVERY_OBJECT: v1 0x009BB564 _TextureRasterPalette") == 1


def test_task_preserves_two_owner_scope():
    task = Task.from_dict(json.loads((ROOT / "recovery/tasks/texture-palette-storage-oct11.json").read_text()))
    assert task.base_revision == BASE
    assert {s.address for s in task.subjects} == {0x61DF60, 0x61E540}
    assert "src/texture.cpp" in task.editable_files


def test_pinned_palette_cell_producers_and_byte_lookup_consumers():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    from match_recovery.source_inventory import image_pin
    image = PeImage((ROOT / ".opensmacx/game/terranx_original.exe").read_bytes(), image_pin(ROOT))
    instructions = {}
    for span in (Span(0x61DF60, 0x61E540), Span(0x61E540, 0x61EFA0)):
        instructions.update({i.address: (i.mnemonic, i.op_str) for i in
            Cs(CS_ARCH_X86, CS_MODE_32).disasm(image.read(span), span.start)})
    assert instructions[0x61DFD5] == ("mov", "dword ptr [0x9bb564], esi")
    assert instructions[0x61E5D3] == ("mov", "edx, dword ptr [ebp + 0x14]")
    assert instructions[0x61E5D8] == ("mov", "dword ptr [0x9bb564], edx")
    assert instructions[0x61E4D3] == ("add", "edx, dword ptr [0x9bb564]")
    assert "byte ptr [edx]" in instructions[0x61E4D9][1]
    assert any("[0x9bb564]" in operands for address, (_, operands) in instructions.items()
               if 0x61EA54 <= address <= 0x61EA63)
    assert any("byte ptr [edx]" in operands for address, (_, operands) in instructions.items()
               if 0x61EA54 <= address <= 0x61EA63)
