"""Execute the actual map reset body; host checks do not prove target fidelity."""
from pathlib import Path
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_map_reset_preserves_unwritten_fields(compiler, tmp_path):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"host compiler unavailable: {compiler}")
    source = (ROOT / "src/map.cpp").read_text()
    start = source.index("void __cdecl map_wipe() {")
    end = source.index("\n}", start) + 2
    body = source[start:end]
    harness = r'''
#include <cassert>
#include <cstring>
#include <cstddef>
#define __cdecl
#define ZeroMemory(p,n) std::memset(p,0,n)
enum { ALT_BIT_OCEAN = 0x20 };
struct Map {
    unsigned char climate, contour, val2, region, visibility, val3, unk_1, territory;
    unsigned int bit, bit2, bit_visible[7];
};
static Map tiles[4];
static int MapSeaLevel, MapSeaLevelCouncil, MapLandmarkCount, MapRandSeed;
static unsigned int MapArea;
static int random_calls;
static int random(int low, int high) {
    assert(low == 0 && high == 0x7fff);
    assert(MapSeaLevel == 0 && MapSeaLevelCouncil == 0 && MapLandmarkCount == 0);
    ++random_calls;
    return 23;
}
static Map *map_tiles() { return tiles; }
''' + body + r'''
int main() {
    assert(sizeof(Map) == 44 && offsetof(Map,bit) == 8 && offsetof(Map,bit2) == 12);
    for (unsigned int count = 0; count <= 4; ++count) {
        std::memset(tiles,0xa5,sizeof(tiles));
        MapSeaLevel = MapSeaLevelCouncil = MapLandmarkCount = -7;
        MapRandSeed = -1;
        MapArea = count;
        map_wipe();
        assert(MapRandSeed == 24 && random_calls == static_cast<int>(count + 1));
        for (unsigned int i=0; i<4; ++i) {
            const Map &t=tiles[i];
            if (i >= count) {
                const unsigned char *bytes = reinterpret_cast<const unsigned char *>(&t);
                for (unsigned int j=0; j<sizeof(t); ++j) assert(bytes[j] == 0xa5);
                continue;
            }
            assert(t.climate==0x20 && t.contour==20 && t.val2==15);
            assert(t.region==0 && t.visibility==0 && t.val3==0);
            assert(t.unk_1==0xa5 && t.territory==0xa5);
            assert(t.bit==0 && t.bit2==0);
            for (int j=0;j<7;++j) assert(t.bit_visible[j]==0);
        }
    }
}
'''
    cpp = tmp_path / "actual.cpp"
    binary = tmp_path / "control"
    cpp.write_text(harness)
    built = subprocess.run([executable, "-std=c++98", "-Wall", "-Wextra",
                            "-Werror", str(cpp), "-o", str(binary)],
                           capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)
    assert ran.returncode == 0, ran.stdout + ran.stderr
