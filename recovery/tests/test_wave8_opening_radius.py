"""Host controls for the actual wave-eight bodies; no game ABI claim."""

from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]
SOURCES = {
    0x589A30: ROOT / "src/game.cpp",
    0x565160: ROOT / "src/map.cpp",
}


def live_body(address, signature):
    source_path = SOURCES[address]
    records = [record for record in read_file(source_path) if record.address == address]
    assert len(records) == 1
    source = source_path.read_text()
    assert source.count(signature) == 1
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated live body")


HARNESS = r"""
    #include <cstdint>
    #include <cstdlib>
    #include <string>
    #define __cdecl
    void check(bool ok) { if (!ok) std::exit(91); }

    struct GraphicWin;
    GraphicWin *opening = nullptr, *after_load = nullptr, *after_copy = nullptr;
    int opening_reads = 0, phase = 0, created = 0, destroyed = 0;
    int copied_width = 0, copied_height = 0, copied_src_w = 0, copied_src_h = 0;
    bool load_succeeds = true;
    GraphicWin *&opening_window() { ++opening_reads; return opening; }
    struct Dib { struct { int biWidth, biHeight; } bmiHeader; };
    struct Buffer {
        virtual ~Buffer() { ++destroyed; check(phase == 0 || phase == 4); }
        Dib dib_ = {{0, 0}};
        Buffer() { ++created; }
        int load_pcx(char *filename, int a, int b, int c) {
            check(phase == 1 && std::string(filename) == "opening.pcx");
            check(a == 0 && b == 10 && c == 0xec);
            dib_.bmiHeader.biWidth = 13;
            dib_.bmiHeader.biHeight = -17;
            opening = after_load;
            phase = 2;
            return load_succeeds ? 0 : 7;
        }
        int copy(Buffer *target, int sx, int sy, int sw, int sh,
                 int dx, int dy, int dw, int dh);
    };
    struct Win { virtual ~Win() {} virtual void on_update() { check(false); } int pad; };
    struct GraphicWin : Win, Buffer {
        void on_update() override { check(phase == 3 && this == after_copy); phase = 4; }
    };
    int Buffer::copy(Buffer *target, int sx, int sy, int sw, int sh,
                     int dx, int dy, int dw, int dh) {
        check(phase == 2 && target == static_cast<Buffer *>(after_load));
        check(sx == 0 && sy == 0 && dx == 0 && dy == 0);
        copied_src_w = sw; copied_src_h = sh;
        copied_width = dw; copied_height = dh;
        opening = after_copy;
        phase = 3;
        return 5;  // ignored by the image
    }

    // LIVE_OPENING

    int VehMoveDeltaX[9] = {1, 2, 1, 0, -1, -2, -1, 0, 0};
    int VehMoveDeltaY[9] = {-1, 0, 1, 2, 1, 0, -1, -2, 0};
    int MapIsFlat = 1, MapLongitudeBounds = 16, MapLatitudeBounds = 16;
    unsigned MapLongitude = 8;
    const int DTREATY_PACT = 1, ALT_BIT_SHORE_LINE = 0x60;
    const int ALT_BIT_OCEAN_SHELF = 0x40, BIT_FUNGUS = 0x20;
    struct Map { unsigned char climate; unsigned char pad[7]; unsigned bit; };
    Map tiles[128] = {};
    Map *map_tiles() { return tiles; }
    struct PlayerData { unsigned diplo_treaties[8]; } PlayersData[8] = {};
    struct Base { short x, y; unsigned char faction_id_current; } Bases[8] = {};
    int base_ids[16][16] = {};
    int bonus = 0, base_calls = 0;
    int bonus_at(int, int, int) { return bonus; }
    int base_at(int x, int y) { ++base_calls; return base_ids[y][x] - 1; }

    // LIVE_RADIUS

    void reset_map() {
        for (auto &row : base_ids) for (auto &id : row) id = 0;
        for (auto &tile : tiles) { tile.climate = 0x60; tile.bit = 0; }
        for (auto &player : PlayersData)
            for (auto &treaty : player.diplo_treaties) treaty = 0;
        bonus = 0; base_calls = 0; MapIsFlat = 1;
    }
    void place_base(int id, int x, int y, int owner) {
        Bases[id].x = x; Bases[id].y = y;
        Bases[id].faction_id_current = owner;
        base_ids[y][x] = id + 1;
    }
    void tile(int x, int y, int altitude, int bit) {
        Map &m = tiles[y * MapLongitude + (x >> 1)];
        m.climate = altitude; m.bit = bit;
    }
    int main() {
        char filename[] = "opening.pcx";
        check(change_opening(filename) == 7);
        check(created == 1 && destroyed == 1 && opening_reads == 1 && phase == 0);
        GraphicWin first, second, third;
        first.dib_.bmiHeader = {2, -3};
        second.dib_.bmiHeader = {19, -23};
        third.dib_.bmiHeader = {29, -31};
        opening = &first; after_load = &second; after_copy = &third;
        opening_reads = 0; phase = 1; load_succeeds = false;
        check(change_opening(filename) == 0);
        check(phase == 4 && copied_width == 19 && copied_height == 23);
        check(copied_src_w == 13 && copied_src_h == 17);
        check(opening_reads == 3 && created == 5 && destroyed == 2);

        reset_map(); bonus = 3;
        check(sub_565160(1, 8, 8) == 1 && base_calls == 0);
        reset_map(); check(sub_565160(1, 8, 8) == 0);
        reset_map();
        place_base(0, 9, 7, 2); place_base(1, 10, 8, 3);
        PlayersData[1].diplo_treaties[2] = DTREATY_PACT;
        PlayersData[1].diplo_treaties[3] = DTREATY_PACT;
        tile(10, 10, 0x80, 0); // only the last allied base has a good neighbor
        check(sub_565160(1, 8, 8) == 0); // on land, no qualifying neighbor
        tile(10, 10, 0x20, 0);
        check(sub_565160(1, 8, 8) == 1);

        reset_map();
        place_base(0, 9, 7, 2); place_base(1, 10, 8, 1);
        place_base(2, 9, 9, 3);
        PlayersData[1].diplo_treaties[2] = DTREATY_PACT;
        PlayersData[1].diplo_treaties[3] = DTREATY_PACT;
        tile(10, 10, 0x20, 0);
        check(sub_565160(1, 8, 8) == 1); // own base stops selection

        reset_map(); MapIsFlat = 0;
        place_base(0, 15, 7, 1); tile(14, 6, 0x20, 0);
        check(sub_565160(1, 0, 8) == 1); // first scan wraps

        reset_map(); place_base(0, 9, 7, 1);
        tile(10, 6, 0x40, BIT_FUNGUS);
        check(sub_565160(1, 8, 8) == 0);
        tile(10, 6, 0x20, BIT_FUNGUS);
        check(sub_565160(1, 8, 8) == 1); // trench fungus qualifies
        tile(10, 6, 0x40, 0);
        check(sub_565160(1, 8, 8) == 1); // shelf without fungus qualifies
        return 0;
    }
"""


def run_host(tmp_path, opening=None, radius=None):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host g++ required")
    opening = opening or live_body(0x589A30, "int __cdecl change_opening(char *filename)")
    radius = radius or live_body(0x565160, "extern \"C\" int __cdecl sub_565160(")
    source = tmp_path / "wave8.cpp"
    executable = tmp_path / "wave8"
    source.write_text(HARNESS.replace("// LIVE_OPENING", opening).replace("// LIVE_RADIUS", radius))
    built = subprocess.run([compiler, "-std=c++11", "-O1", "-fsanitize=undefined",
                            "-fno-sanitize-recover=undefined", str(source), "-o", str(executable)],
                           capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)


def test_live_opening_and_radius(tmp_path):
    result = run_host(tmp_path)
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("damage", ["stale_window", "wrong_scale", "first_allied",
                                    "no_wrap", "fungus_blocks_trench"])
def test_mutations_fail_runtime(tmp_path, damage):
    opening = live_body(0x589A30, "int __cdecl change_opening(char *filename)")
    radius = live_body(0x565160, "extern \"C\" int __cdecl sub_565160(")
    if damage == "stale_window":
        old, new = "opening_window()->on_update();", "window->on_update();"
        opening = opening.replace(old, new)
    elif damage == "wrong_scale":
        old, new = "int c1 = surface->dib_.bmiHeader.biWidth;", "int c1 = buf.dib_.bmiHeader.biWidth;"
        opening = opening.replace(old, new)
    elif damage == "first_allied":
        old, new = "if (owner == static_cast<unsigned int>(faction_id)) {", "if (found >= 0) {"
        radius = radius.replace(old, new)
    elif damage == "no_wrap":
        old, new = "x += MapLongitudeBounds;", "x += 0;"
        radius = radius.replace(old, new)
    else:
        old, new = "if (altitude < ALT_BIT_OCEAN_SHELF) {", "if (false) {"
        radius = radius.replace(old, new)
    assert old not in (opening if damage in ("stale_window", "wrong_scale") else radius)
    result = run_host(tmp_path, opening, radius)
    assert result.returncode == 91, result.stderr
