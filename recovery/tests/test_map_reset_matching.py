"""Execute the actual map reset body; host checks do not prove target fidelity."""
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
RESET_ORDER = ("MapSeaLevelCouncil", "MapLandmarkCount", "MapSeaLevel")


def _map_wipe_body(source):
    from match_recovery.source import function_line
    from match_recovery.source.reader import _extract_forward_text
    line = function_line(source, "map_wipe")
    return _extract_forward_text(source.splitlines(), line, "map_wipe").strip()


def _leading_reset_order(body):
    prefix = re.search(r"void __cdecl map_wipe\(\) \{(.*?)MapRandSeed =", body, re.S)
    assert prefix, "could not isolate map_wipe resets before the RNG call"
    statements = re.findall(
        r"(?m)^\s*(MapSeaLevelCouncil|MapLandmarkCount|MapSeaLevel)\s*=\s*0;\s*$",
        prefix.group(1),
    )
    assert len(statements) == 3 and len(re.sub(
        r"(?m)^\s*(?:MapSeaLevelCouncil|MapLandmarkCount|MapSeaLevel)\s*=\s*0;\s*$",
        "", prefix.group(1),
    ).strip()) == 0, "expected exactly three zero resets before the RNG call"
    return tuple(statements)


def _pinned_image_path():
    explicit = os.environ.get("OPENSMACX_IMAGE")
    if explicit is not None:
        return Path(explicit)
    candidates = [ROOT / ".opensmacx/game/terranx_original.exe"]
    candidates.extend(parent / ".opensmacx/game/terranx_original.exe" for parent in ROOT.parents)
    return next((path for path in candidates if path.is_file()), candidates[-1])


def _load_pinned_image(path):
    from match_recovery.pe_coff import PeImage
    return PeImage(Path(path).read_bytes(), PIN)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_map_reset_preserves_unwritten_fields(compiler, tmp_path):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"host compiler unavailable: {compiler}")
    source = (ROOT / "src/map.cpp").read_text()
    body = _map_wipe_body(source)
    assert _leading_reset_order(body) == RESET_ORDER
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
static int MapArea;
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


def test_reset_order_guard_rejects_previous_source_order():
    source = (ROOT / "src/map.cpp").read_text()
    body = _map_wipe_body(source)
    old_order = body.replace(
        "    MapSeaLevelCouncil = 0;\n    MapLandmarkCount = 0;\n    MapSeaLevel = 0;",
        "    MapSeaLevel = 0;\n    MapSeaLevelCouncil = 0;\n    MapLandmarkCount = 0;",
        1,
    )
    assert _leading_reset_order(body) == RESET_ORDER
    assert _leading_reset_order(old_order) != RESET_ORDER


def test_pinned_original_map_wipe_reset_targets_and_order():
    from match_recovery.model import Span
    from match_recovery.source.asm import Listing

    image = _load_pinned_image(_pinned_image_path())
    span = Span(0x00591042, 0x0059105C)
    instructions = Listing(image.read(span), span.start).instructions
    assert [(item.mnemonic, item.op_str) for item in instructions] == [
        ("xor", "ebx, ebx"),
        ("push", "0x7fff"),
        ("push", "ebx"),
        ("mov", "dword ptr [0x949880], ebx"),
        ("mov", "dword ptr [0x949890], ebx"),
        ("mov", "dword ptr [0x94987c], ebx"),
    ]


def test_pinned_image_loader_refuses_missing_corrupt_and_bad_override(tmp_path, monkeypatch):
    missing = tmp_path / "missing.exe"
    with pytest.raises(FileNotFoundError):
        _load_pinned_image(missing)

    corrupt = tmp_path / "corrupt.exe"
    corrupt.write_bytes(b"not the pinned executable")
    with pytest.raises(ValueError, match="digest mismatch|PE"):
        _load_pinned_image(corrupt)

    monkeypatch.setenv("OPENSMACX_IMAGE", str(missing))
    assert _pinned_image_path() == missing
    with pytest.raises(FileNotFoundError):
        _load_pinned_image(_pinned_image_path())

    monkeypatch.setenv("OPENSMACX_IMAGE", "")
    assert _pinned_image_path() == Path("")
    with pytest.raises(IsADirectoryError):
        _load_pinned_image(_pinned_image_path())
