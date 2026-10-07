"""Host-test the current actual Map nutrient-bonus body without owner guesses."""

from pathlib import Path
import shutil
import subprocess

from match_recovery.source import read_file


ROOT = next(path for path in Path(__file__).resolve().parents
            if (path / "src/IMAGE_PIN").is_file())
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
SIGNATURE = "extern \"C\" int __cdecl sub_4c99d0("


def actual_definition():
    source_path = ROOT / "src/map.cpp"
    source = source_path.read_text(encoding="utf-8")
    rows = [row for row in read_file(source_path) if row.address == 0x004C99D0]
    assert len(rows) == 1, "expected one unique map-yield source mapping"
    row = rows[0]
    assert row.name == "sub_4c99d0"
    assert row.image_spans == ((0x004C99D0, 0x004C9A43),)
    assert row.mode.value == "file"
    starts = [index for index in range(len(source)) if source.startswith(SIGNATURE, index)]
    assert len(starts) == 1
    start = starts[0]
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                body = source[start:index + 1]
                assert "BODY GOES HERE." not in body
                assert (
                    "int result = (bits & 4) != 0\n"
                    "        ? MapTerrainBonusYieldRow6FirstWord\n"
                    "        : MapTerrainBonusYieldRow5FirstWord;"
                ) in body
                return body
    raise AssertionError("unterminated sub_4c99d0 actual source definition")


def test_pinned_map_table_words_and_exact_source_identity():
    pin = (ROOT / "src/IMAGE_PIN").read_text(encoding="utf-8").split()[-1]
    assert pin == PIN
    source = (ROOT / "src/map.cpp").read_text(encoding="utf-8")
    assert "RECOVERY_OBJECT: v1 0x00691930 ?MapTerrainBonusYieldRow5FirstWord@@3HB" in source
    assert "extern const int MapTerrainBonusYieldRow5FirstWord = 0;" in source
    assert "RECOVERY_OBJECT: v1 0x00691950 ?MapTerrainBonusYieldRow6FirstWord@@3HB" in source
    assert "extern const int MapTerrainBonusYieldRow6FirstWord = 0;" in source


def test_actual_nutrient_bonus_body_in_host_fixture(tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        import pytest
        pytest.skip("g++ is unavailable")
    body = actual_definition()
    fixture = "\n".join((
        "#include <assert.h>",
        "#include <stdint.h>",
        "#define __cdecl",
        "struct Map {",
        "    uint8_t climate, contour, val2, region, visibility, val3, unk_1;",
        "    int8_t territory;",
        "    uint32_t bit, bit2, bit_visible[7];",
        "};",
        "static Map tiles[32];",
        "Map *map_tiles() { return tiles; }",
        "uint32_t MapLongitude = 4;",
        "extern const int MapTerrainBonusYieldRow5FirstWord = 0;",
        "extern const int MapTerrainBonusYieldRow6FirstWord = 0;",
        body,
        "int main() {",
        "    Map &tile = tiles[4 * 2 + 1];",
        "    assert(sub_4c99d0(2, 2) == 0);",
        "    tile.val3 = 0xC0;",
        "    assert(sub_4c99d0(2, 2) == 3);",
        "    tile.bit = 0x004;",
        "    assert(sub_4c99d0(2, 2) == 3); // selects the distinct zero row 6 word",
        "    tile.bit = 0x080;",
        "    assert(sub_4c99d0(2, 2) == 4); // one bonus from the resource bit",
        "    tile.bit = 0x020;",
        "    tile.climate = 0x20;",
        "    assert(sub_4c99d0(2, 2) == 3); // raised-resource bonus is not active",
        "    tile.climate = 0x40;",
        "    assert(sub_4c99d0(2, 2) == 5); // threshold adds two",
        "    tile.bit = 0x020 | 0x080 | 0x200000;",
        "    assert(sub_4c99d0(2, 2) == 5); // special resource returns before val3",
        "    return 0;",
        "}",
    ))
    source = tmp_path / "map_yield_actual_source.cpp"
    binary = tmp_path / "map_yield_actual_source"
    source.write_text(fixture, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(source), "-o", str(binary)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stderr
    result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
    assert result.returncode == 0, result.stderr
