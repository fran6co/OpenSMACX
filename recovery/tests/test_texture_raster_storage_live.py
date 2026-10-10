from pathlib import Path
import hashlib
import re
import shutil
import subprocess
import tempfile

from match_recovery.model import Span
from match_recovery.object_source import parse_objects
from match_recovery.pe_coff import PeImage
import pytest


ROOT = next(
    parent for parent in Path(__file__).resolve().parents
    if (parent / "src/texture.cpp").is_file()
)
IMAGE_SHA256 = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def _original_chunk(source, address):
    marker = f"// ORIGINAL: 0x{address:08X} "
    start = source.index(marker)
    following = source.find("// ORIGINAL:", start + len(marker))
    return source[start:] if following < 0 else source[start:following]


def test_pinned_storage_bytes_and_pointer_relocations():
    data = (ROOT / ".opensmacx/game/terranx_original.exe").read_bytes()
    assert hashlib.sha256(data).hexdigest() == IMAGE_SHA256
    image = PeImage(data, IMAGE_SHA256)
    assert image.read_initial(Span(0x9BB490, 0x9BB568)) == bytes(0xD8)
    assert image.read_initial(Span(0x670A84, 0x670A8C)) == bytes.fromhex("0000804700008043")
    assert image.read_initial(Span(0x6972C4, 0x6972CC)) == bytes.fromhex("20b59b0024b59b00")
    assert image.absolute_sites(Span(0x6972C4, 0x6972CC)) == frozenset((0, 4))


def test_live_source_owns_distinct_cells_aliases_and_mapped_identities():
    source = (ROOT / "src/texture.cpp").read_text()
    bindings = parse_objects(source, "src/texture.cpp", IMAGE_SHA256, address_space="pe:va")
    raster = [b for b in bindings if b.symbol and b.symbol.startswith("_TextureRaster")]
    assert len(raster) == 49
    assert all(b.span is None for b in raster)
    addresses = {b.origin.address for b in raster}
    assert {0x009BB520, 0x009BB524, 0x00670A84, 0x00670A88,
            0x006972C4, 0x006972C8, 0x009BB490, 0x009BB4F4,
            0x009BB4FC, 0x009BB52C}.issubset(addresses)
    assert not ({0x009BB4CA, 0x009BB4E6, 0x009BB4FA, 0x009BB53E} & addresses)

    assert "struct TextureRasterWordPair {\n    unsigned short low_word;\n    unsigned short high_word;" in source
    assert "&TextureRasterUAccumulator.words.high_word" in source
    assert "&TextureRasterVAccumulator.words.high_word" in source
    assert "&TextureRasterScratch4C8.bytes[2]" in source
    assert "&TextureRasterScratch4E4.bytes[2]" in source
    assert "&TextureRasterLightPackA.low_word" in source
    assert "static Vert **const g2_009bb490 = &TextureRasterEdgeVerticesA;" in source
    assert "static Vert **const g2_009bb4f4 = &TextureRasterEdgeVerticesB;" in source
    assert "static int **const g2_009bb4fc = &TextureRasterZValues;" in source
    assert "{ &TextureRasterScratch520 }" in source
    assert "{ &TextureRasterScratch524 }" in source

    # Numeric old-image addresses remain only provenance/mapping text, never
    # live pointer initializers or dereferences.
    assert not re.search(r"(?:reinterpret_cast<[^>]+>\s*\(|\(\s*(?:int|short|float)\s*\*\s*\))\s*0x009BB(?:490|4F4|4FC|52C)", source, re.I)
    assert "reinterpret_cast<short *>(blit_light_pack_a)" not in source
    assert "(short *)blit_light_pack_a" not in source


def test_live_52c_writers_preserve_adjacent_word_and_6202e0_uses_shared_view():
    source = (ROOT / "src/texture.cpp").read_text()
    pair_decl = re.search(
        r"struct TextureRasterWordPair \{\s*unsigned short low_word;\s*unsigned short high_word;\s*\};",
        source,
    )
    object_decl = re.search(r"static TextureRasterWordPair TextureRasterLightPackA;", source)
    alias_decl = re.search(
        r"static unsigned short \*const blit_light_pack_a =\s*&TextureRasterLightPackA\.low_word;",
        source,
    )
    assert pair_decl and object_decl and alias_decl

    wide = _original_chunk(source, 0x0061EAD0)
    signed_reader = _original_chunk(source, 0x00623F80)
    narrow = _original_chunk(source, 0x006202E0)
    wide_store_match = re.search(
        r"\*blit_light_pack_a\s*=\s*static_cast<int>\(light_step\);", wide
    )
    assert wide_store_match
    assert "unsigned short *const g_bb52c_w = blit_light_pack_a;" in narrow
    actual_store = "*g_bb52c_w = static_cast<unsigned short>(num52c / divisor64);"
    assert actual_store in narrow
    signed_read_match = re.search(r"static_cast<short>\(\*blit_light_pack_a\)", signed_reader)
    assert signed_read_match

    # Compile the live type declaration and the two actual store statements,
    # extracted from the installed owner bodies. This checks low-word writes
    # without disturbing the adjacent high halfword (including 0x8001).
    harness = r'''#include <cassert>
__PAIR_DECL__
__OBJECT_DECL__
__ALIAS_DECL__
static void wide_store(unsigned int light_step) { __WIDE_STORE__ }
static short signed_read() { return __SIGNED_READ__; }
static void row_store(long long num52c, long long divisor64) {
    unsigned short *const g_bb52c_w = blit_light_pack_a;
    __ROW_STORE__
}
int main() {
    TextureRasterLightPackA.high_word = 0x7a55;
    wide_store(0x8001);
    assert(TextureRasterLightPackA.low_word == 0x8001 && TextureRasterLightPackA.high_word == 0x7a55);
    assert(signed_read() == -32767);
    TextureRasterLightPackA.high_word = 0x6b44;
    row_store(0x8001, 1);
    assert(TextureRasterLightPackA.low_word == 0x8001 && TextureRasterLightPackA.high_word == 0x6b44);
}
'''
    harness = harness.replace("__PAIR_DECL__", pair_decl.group(0))
    harness = harness.replace("__OBJECT_DECL__", object_decl.group(0))
    harness = harness.replace("__ALIAS_DECL__", alias_decl.group(0))
    harness = harness.replace("__WIDE_STORE__", wide_store_match.group(0))
    harness = harness.replace("__SIGNED_READ__", signed_read_match.group(0))
    harness = harness.replace("__ROW_STORE__", actual_store)
    with tempfile.TemporaryDirectory(prefix="texture-raster-live-52c-") as temp:
        path = Path(temp) / "storage.cpp"
        path.write_text(harness)
        available = 0
        for compiler in ("g++", "clang++"):
            exe = shutil.which(compiler)
            if not exe:
                continue
            available += 1
            target = Path(temp) / compiler.replace("+", "p")
            subprocess.run([exe, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(path), "-o", str(target)], check=True, capture_output=True, text=True)
            subprocess.run([str(target)], check=True, capture_output=True, text=True)
        assert available


def test_6202e0_was_rehomed_from_numeric_old_address():
    source = (ROOT / "src/texture.cpp").read_text()
    owner = _original_chunk(source, 0x006202E0)
    assert "unsigned short *const g_bb52c_w = blit_light_pack_a;" in owner
    assert "0x009BB52C" not in owner
    assert "*g_bb52c_w = static_cast<unsigned short>(num52c / divisor64);" in owner
    # Pinned word accesses for this owner: 0x62035F, 0x620430,
    # 0x62054C and 0x620603. These are evidence coordinates, not source PCs.


@pytest.mark.parametrize("mutation", [None, "high_word_at_zero", "wide_light_store"])
def test_live_storage_pointer_identity_and_overlapping_views(tmp_path, mutation):
    source = (ROOT / "src/texture.cpp").read_text()
    start = source.index("// Shared Texture-owned storage")
    end = source.index("static int texture_coast_advance_edge", start)
    storage = source[start:end]
    if mutation == "high_word_at_zero":
        assert "&TextureRasterUAccumulator.words.high_word" in storage
        storage = storage.replace("&TextureRasterUAccumulator.words.high_word",
                                  "&TextureRasterUAccumulator.words.low_word", 1)
    actual_store = re.search(r"\*blit_light_pack_a\s*=\s*static_cast<int>\(light_step\);",
                            _original_chunk(source, 0x61EAD0)).group(0)
    if mutation == "wide_light_store":
        actual_store = "*reinterpret_cast<unsigned int *>(&TextureRasterLightPackA) = light_step;"
    control = r'''#include <cassert>
#include <cstddef>
#include <stdint.h>
unsigned int TextureRasterKey;
__ACTUAL_VERT__
__LIVE_STORAGE__
int main() {
    assert(sizeof(TextureRasterWordPair) == 4);
    assert(offsetof(TextureRasterWordPair, low_word) == 0);
    assert(offsetof(TextureRasterWordPair, high_word) == 2);
    assert(TextureRasterAlternateRowBase.initial_target == &TextureRasterScratch520);
    assert(TextureRasterAlternateRowNext.initial_target == &TextureRasterScratch524);
    assert(TextureRasterEdgeVerticesA == 0 && TextureRasterEdgeVerticesB == 0 && TextureRasterZValues == 0);
    static Vert first[1] = {{11, 22}}, second[1] = {{33, 44}};
    static int light[2] = {55, 66};
    *blit_edge_uvs_a = first;
    *blit_edge_uvs_b = second;
    *blit_z_values = light;
    assert((*g2_009bb490)[0].y == 22 && (*g2_009bb4f4)[0].y == 44 && (*g2_009bb4fc)[1] == 66);
    *g2_009bb490 = second;
    assert(*blit_edge_uvs_a == second && *blit_edge_uvs_b == second && *blit_z_values == light);
    assert(texture_pointer_offset(first, 0) == texture_pointer_address(first));
    assert(texture_pointer_from_address(texture_pointer_address(second)) == second);
    TextureRasterScratch4C8.dword = 0x11223344;
    TextureRasterScratch4E4.dword = 0x55667788;
    TextureRasterUAccumulator.dword = 0x11223344;
    TextureRasterVAccumulator.dword = 0x55667788;
    assert(*blit_slot_4ca == 0x22 && *blit_slot_4e6 == 0x66);
    assert(*blit_slot_4fa == 0x1122 && *blit_slot_53e == 0x5566);
    *blit_slot_4fa = 0x1357;
    assert(TextureRasterUAccumulator.dword == 0x13573344);
    *blit_slot_53e = 0x2468;
    assert(TextureRasterVAccumulator.dword == 0x24687788);
    TextureRasterLightPackA.high_word = 0x7a55;
    unsigned int light_step = 0x8001;
    __ACTUAL_STORE__
    assert(TextureRasterLightPackA.low_word == 0x8001 && TextureRasterLightPackA.high_word == 0x7a55);
}
'''.replace("__ACTUAL_VERT__", re.search(
        r"struct Vert\s*\{.*?\};", (ROOT / "src/vert.h").read_text(), re.S).group(0)).replace("__LIVE_STORAGE__", storage).replace("__ACTUAL_STORE__", actual_store)
    available = 0
    for name in ("g++", "clang++"):
        compiler = shutil.which(name)
        if not compiler:
            continue
        available += 1
        cpp, executable = tmp_path / (name + ".cpp"), tmp_path / name
        cpp.write_text(control)
        built = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                                "-Wno-unused-variable", str(cpp), "-o", str(executable)],
                               capture_output=True, text=True, timeout=30)
        assert built.returncode == 0, built.stdout + built.stderr
        ran = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
        assert (ran.returncode == 0) == (mutation is None), ran.stdout + ran.stderr
    assert available
