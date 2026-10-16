"""Controls for the evidence-backed signed MapArea/world-build owner model."""
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest
from match_recovery.source import read_file
from match_recovery.pe_coff import PeImage
from match_recovery.model import Span
from match_recovery.source.asm import Listing

ROOT = next(p for p in Path(__file__).resolve().parents if (p / "src/map.cpp").is_file())
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def definition(text, signature):
    start = text.index(signature)
    depth = 0
    opened = False
    for token in re.finditer(r"//[^\n]*|/\*.*?\*/|\"(?:\\.|[^\"\\])*\"|'(?:\\.|[^'\\])*'|[{}]", text[start:], re.S):
        if token.group() == "{":
            opened = True
            depth += 1
        elif token.group() == "}":
            depth -= 1
            if opened and depth == 0:
                return text[start:start + token.end()]
    raise AssertionError("unterminated actual definition")


def test_signed_count_types_cover_the_real_shared_owner_and_consumers():
    header = (ROOT / "src/map.h").read_text()
    alpha_header = (ROOT / "src/alpha.h").read_text()
    map_source = (ROOT / "src/map.cpp").read_text()
    assert re.search(r"\bint area_;\s*// \+0x014", header)
    assert re.search(r"extern int BrushVal1;", header)
    assert re.search(r"extern int WorldBuildVal1;", header)
    for index in range(1, 6):
        assert re.search(rf"\bint cont_size_ratio{index};", alpha_header)
    assert re.search(r"\bint islands;", alpha_header)
    assert "int BrushVal1;  // 0x009B22B0" in map_source
    assert "int WorldBuildVal1;  // 0x009B22B4" in map_source
    assert "int ratio = (size * 3200) / MapArea;" in definition(
        map_source, "void __cdecl build_continent(int size) {")
    assert "int area_index;\n    for (area_index = 0; area_index < MapArea; area_index++)" in definition(
        map_source, "void __cdecl world_analysis() {")
    assert "MapArea * 44u" in (ROOT / "src/netdaemon.cpp").read_text()
    path_source = (ROOT / "src/path.cpp").read_text()
    assert "MapArea * sizeof(*map_table_)" in path_source
    assert "MapArea * 4);" not in path_source

    # Keep the historical identity/member binding load-bearing and unchanged.
    assert "// RECOVERY_OBJECT: v3 0x00949884 ?MapArea@@3IA member ?MapStorage@@3UMapStorageWindow@@A metadata_.area_" in map_source


def test_signed_maparea_counters_are_paired_at_current_consumers():
    expected = {
        "src/technology.cpp": [
            "for (int tile_id = 0; tile_id < MapArea; ++tile_id)",
            "for (int tile_id = 0; tile_id < MapArea; ++tile_id, ++tile)",
        ],
        "src/faction.cpp": ["for (int fill_idx = 0; fill_idx < MapArea; ++fill_idx)"],
        "src/map.cpp": [
            "for (int i = 0; i < MapArea; i++)",
            "int area = MapArea;",
        ],
    }
    for path, anchors in expected.items():
        text = (ROOT / path).read_text()
        for anchor in anchors:
            assert anchor in text


def test_compiled_actual_area_guard_and_ratio_preserve_high_bit_sign(tmp_path):
    compiler = shutil.which("c++")
    if compiler is None:
        pytest.skip("host compiler unavailable")
    map_header = (ROOT / "src/map.h").read_text()
    landmark = re.search(r"struct Landmark \{.*?\n\};", map_header, re.S).group()
    start = map_header.index("struct MapMetadataFields {")
    layout = map_header[start:map_header.index("// The tile-flag mask", start)]
    map_source = (ROOT / "src/map.cpp").read_text()
    continent = definition(map_source, "void __cdecl build_continent(int size) {")
    reset_walk = re.search(
        r"Map \*tile = map_tiles\(\);\s*for \(int i = 0; i < MapArea; i\+\+\) \{\s*tile->region = 0;\s*tile\+\+;\s*\}",
        continent,
    ).group()
    ratio_statement = re.search(r"int ratio = \(size \* 3200\) / MapArea;", continent).group()
    source = tmp_path / "signed_map_area.cpp"
    source.write_text(
        "#include <stdint.h>\n#include <stddef.h>\n#include <string.h>\n"
        "typedef int BOOL; struct Map { int region; };\n"
        + landmark + "\nconst int MaxLandmarkNum = 64;\n" + layout
        + "\nMapStorageWindow MapStorage;\nstatic Map *map_tiles() { return MapTiles; }\n"
        + "void reset_area() { " + reset_walk + " }\n"
        + "int signed_ratio(int size) { " + ratio_statement + " return ratio; }\n"
        + "int main() { Map tile = {7}; MapTiles = &tile; MapArea = -1; reset_area(); "
          "if (tile.region != 7 || signed_ratio(1) != -3200) return 1; "
          "MapArea = 1; reset_area(); return tile.region == 0 ? 0 : 2; }\n"
    )
    binary = tmp_path / "signed_map_area"
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(source), "-o", str(binary)],
        capture_output=True, timeout=60,
    )
    assert built.returncode == 0, built.stderr.decode(errors="replace")
    result = subprocess.run([str(binary)], capture_output=True, timeout=15)
    assert result.returncode == 0, result.stderr.decode(errors="replace")


def test_compiled_actual_erosion_scale_keeps_wrapped_signed_divide(tmp_path):
    compiler = shutil.which("c++")
    if compiler is None:
        pytest.skip("host compiler unavailable")
    map_source = (ROOT / "src/map.cpp").read_text()
    erosion = definition(map_source, "void __cdecl world_erosion() {")
    scale = re.search(
        r"uint32_t scaled_area_bits =\s*static_cast<uint32_t>\(MapArea\) \* 0x32u;\s*"
        r"int scaled_area;\s*memcpy\(&scaled_area, &scaled_area_bits, sizeof\(scaled_area\)\);",
        erosion,
    ).group()
    source = tmp_path / "erosion_area.cpp"
    source.write_text(
        "#include <stdint.h>\n#include <string.h>\nint MapArea;\n"
        + "int scaled_area() { " + scale + " return scaled_area / 0xC80; }\n"
        + "int main() { MapArea = 0x40000000; if (scaled_area() != -671088) return 1; "
          "MapArea = (int)0x80000001u; if (scaled_area() != 0) return 2; "
          "MapArea = -1; return scaled_area() == 0 ? 0 : 3; }\n"
    )
    binary = tmp_path / "erosion_area"
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(source), "-o", str(binary)],
        capture_output=True, timeout=60,
    )
    assert built.returncode == 0, built.stderr.decode(errors="replace")
    result = subprocess.run([str(binary)], capture_output=True, timeout=15)
    assert result.returncode == 0, result.stderr.decode(errors="replace")


def test_original_signed_division_branching_and_preincrement_cutoff():
    explicit = os.environ.get("OPENSMACX_IMAGE")
    candidates = [Path(explicit)] if explicit else [
        p / ".opensmacx/game/terranx_original.exe" for p in (ROOT, *ROOT.parents)
    ]
    image = next((p for p in candidates if p.is_file()), None)
    if image is None:
        pytest.skip("pinned original not available")
    pe = PeImage(image.read_bytes(), PIN)

    ratio = Listing(pe.read(Span(0x5C2A39, 0x5C2A40)), 0x5C2A39).instructions
    assert any(instruction.mnemonic == "idiv" for instruction in ratio)
    ratio_branches = Listing(pe.read(Span(0x5C2A41, 0x5C2A8F)), 0x5C2A41).instructions
    assert {instruction.mnemonic for instruction in ratio_branches} & {"jl", "jle", "jg", "jge"}

    limit = Listing(pe.read(Span(0x5C87CC, 0x5C87E0)), 0x5C87CC).instructions
    assert [(instruction.mnemonic, instruction.op_str) for instruction in limit[:3]] == [
        ("inc", "esi"), ("cmp", "esi, 0x64"), ("jge", "0x5c87db")
    ]
    assert any(instruction.mnemonic == "call" and instruction.op_str == "0x5c28f0"
               for instruction in limit)

    erosion_scale = Listing(pe.read(Span(0x5C300C, 0x5C3035)), 0x5C300C).instructions
    assert any(instruction.mnemonic == "sar" for instruction in erosion_scale)
    assert any(instruction.mnemonic == "jl" for instruction in erosion_scale)
