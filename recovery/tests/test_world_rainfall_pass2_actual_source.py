"""Actual-source controls for the pinned rainfall pass-two decision tree."""
from importlib.util import module_from_spec, spec_from_file_location
from pathlib import Path
import shutil
import subprocess
import os

import pytest


ROOT = Path(__file__).resolve().parents[2]
ACTUAL_TEST = ROOT / "recovery/tests/test_world_rainfall_behavior.py"
SOURCE = ROOT / "src/map.cpp"
SPEC = spec_from_file_location("rainfall_controls", ACTUAL_TEST)
assert SPEC and SPEC.loader
rainfall_controls = module_from_spec(SPEC)
SPEC.loader.exec_module(rainfall_controls)

def actual_harness():
    source = SOURCE.read_text()
    fragments = [rainfall_controls.extract(source, rainfall_controls.PASS2_SIGNATURE)]
    host = rainfall_controls.HOST.replace(
        rainfall_controls.MARKER, "\n\n".join(fragments))
    host = host[:host.index("int main() {")]
    host = host.replace("static Map Tiles[15];",
        "static Map Tiles[15], AlternateTiles[15];\n"
        "static bool callback_replace_map;", 1)
    host = host.replace("static void reset_world() {",
        "static void reset_world() {\n"
        "    MapTiles = Tiles; callback_replace_map = false;\n"
        "    memset(AlternateTiles, 0, sizeof(AlternateTiles));", 1)
    host = host.replace(
        "static int find3_result;",
        "static int find3_result;\n"
        "static int callback_mutate_index = -1;\n"
        "static unsigned char callback_mutate_value;",
    )
    host = host.replace(
        "    ++callback_count;\n}",
        "    ++callback_count;\n"
        "    if (callback_mutate_index >= 0)\n"
        "        Tiles[callback_mutate_index].climate = callback_mutate_value;\n"
        "    if (callback_replace_map) {\n"
        "        MapTiles = AlternateTiles; callback_replace_map = false;\n"
        "    }\n"
        "}\n",
    )
    extra = r'''
int main() {
    int failures = 0;
    reset_world();
    memset(Tiles, 0, sizeof(Tiles));
    MapLongitude = 2; MapLongitudeBounds = 4; MapLatitudeBounds = 3;
    MapArea = 6; MapCloudCover = 0;
    Tiles[2].climate = 0x40; // row 1, logical x 1: altitude 2
    world_rainfall_pass2(2);
    if (((Tiles[2].climate >> 3) & 3) != 1)
        failures += fail("altitude 0..2 sets native carry and class 1");

    reset_world();
    memset(Tiles, 0, sizeof(Tiles));
    MapLongitude = 2; MapLongitudeBounds = 4; MapLatitudeBounds = 3;
    MapArea = 6; MapCloudCover = 0;
    Tiles[2].climate = 0x60; // altitude 3, zero incoming carry/pass count
    world_rainfall_pass2(2);
    if (((Tiles[2].climate >> 3) & 3) != 0)
        failures += fail("altitude 3 zero-carry path preserves zero carry");

    reset_world();
    memset(Tiles, 0, sizeof(Tiles));
    MapLongitude = 2; MapLongitudeBounds = 4; MapLatitudeBounds = 3;
    MapArea = 6; MapCloudCover = 0;
    Tiles[3].climate = 0x40; // row 1, logical x 3 => column 1
    Tiles[3].bit = BIT_RIVER;
    Tiles[3].unk_1 = 0x10; // negative adjustment before river correction
    world_rainfall_pass2(2);
    if (((Tiles[3].climate >> 3) & 3) != 1)
        failures += fail("river correction uses logical column modulo 3");

    reset_world();
    memset(Tiles, 0, sizeof(Tiles));
    MapLongitude = 1; MapLongitudeBounds = 2; MapLatitudeBounds = 3;
    MapArea = 3; MapCloudCover = 0;
    Tiles[1].climate = 0x70; // one interior tile; no later callback can repair a bad merge
    Tiles[1].unk_1 = 0x02;
    GameDataStateStorage.fields_.turn_current_num_ = 1;
    callback_mutate_index = 1;
    callback_mutate_value = 0x65;
    find2_result = -1; find3_result = -1;
    world_rainfall_pass2(2);
    callback_mutate_index = -1;
    if (Tiles[1].climate != 0x65)
        failures += fail("post-base_find climate merge rereads live tile byte");

    reset_world();
    memset(Tiles, 0, sizeof(Tiles));
    MapLongitude = 1; MapLongitudeBounds = 2; MapLatitudeBounds = 3;
    MapArea = 3; MapCloudCover = 0;
    Tiles[1].climate = 0x70;
    Tiles[1].unk_1 = 0x02;
    GameDataStateStorage.fields_.turn_current_num_ = 1;
    TerraformTileFaction = 8;
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 1u << 8;
    find2_result = -1; find3_result = -1;
    world_rainfall_pass2(2);
    if (callback_count != 2 || callbacks[0][0] != 2) {
        fprintf(stderr, "lock callbacks=%d first=%d mask=%08x faction=%d class=%d\n",
                callback_count, callbacks[0][0],
                GameDataStateStorage.fields_.lock_masks_.enable_mask_,
                TerraformTileFaction, (Tiles[1].climate >> 3) & 3);
        failures += fail("native lock test observes only LockEnableMask low byte");
    }
    reset_world();
    memset(Tiles, 0, sizeof(Tiles));
    MapLongitude = 3; MapLongitudeBounds = 6; MapLatitudeBounds = 3;
    MapArea = 9; MapCloudCover = 0;
    Tiles[3].climate = 0xA1; // first hill grows pass counter
    Tiles[4].climate = 0x40; // low tile must decay it to zero
    Tiles[5].climate = 0xA1; // next hill must retain the carry
    world_rainfall_pass2(2);
    if (((Tiles[5].climate >> 3) & 3) != 1)
        failures += fail("low-altitude tile decays pass counter before next hill");

    reset_world();
    memset(Tiles, 0, sizeof(Tiles));
    MapLongitude = 2; MapLongitudeBounds = 4; MapLatitudeBounds = 3;
    MapArea = 6; MapCloudCover = 0;
    Tiles[2].climate = 0x70; Tiles[2].unk_1 = 2;
    Tiles[3].climate = 0xC5; Tiles[3].unk_1 = 1;
    AlternateTiles[3].climate = 0x40;
    GameDataStateStorage.fields_.turn_current_num_ = 1;
    find2_result = find3_result = -1;
    callback_replace_map = true;
    world_rainfall_pass2(2);
    if (MapTiles != AlternateTiles || Tiles[3].climate != 0xD5
        || AlternateTiles[3].climate != 0x40)
        failures += fail("saved climate cursor and indexed terrain reads remain distinct");

    if (failures != 0) return 1;
    puts("actual map.cpp pass2 host: pass");
    return 0;
}
'''
    host += extra
    return host


def _run(tmp_path, compiler, source, name):
    cpp = tmp_path / (name + ".cpp")
    exe = tmp_path / name
    cpp.write_text(source)
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         "-Wno-unused-const-variable", "-fsanitize=address,undefined",
         "-fno-omit-frame-pointer", str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run(
        [str(exe)], capture_output=True, text=True, timeout=30,
        env={**os.environ, "ASAN_OPTIONS": "detect_leaks=0"},
    )


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_pass2_decisions_and_damage_controls(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    source = actual_harness()
    clean = _run(tmp_path, compiler, source, "actual")
    assert clean.returncode == 0, clean.stdout + clean.stderr
    changes = [
        ("altitude < 3", "altitude < 0", "altitude 0..2"),
        ("((x >> 1) % 3)", "(cloud_cover % 3)", "river correction"),
        ("(tile_cursor->climate & 0xE7) |", "(climate & 0xE7) |",
         "post-base_find climate merge"),
        ("static_cast<uint8_t>(\n                                        GameDataStateStorage.fields_.lock_masks_.enable_mask_)",
         "(GameDataStateStorage.fields_.lock_masks_.enable_mask_)",
         "native lock test"),
        ("                Map *tile = &MapTiles[MapLongitude * y + (x >> 1)];",
         "                Map *tile = tile_cursor;", "saved climate cursor"),
    ]
    for index, (before, after, diagnostic) in enumerate(changes):
        assert source.count(before) == 1, before
        mutant = _run(tmp_path, compiler, source.replace(before, after, 1),
                      "mutant-" + str(index))
        assert mutant.returncode != 0
        assert diagnostic in mutant.stderr, mutant.stdout + mutant.stderr
