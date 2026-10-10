"""Bounded actual-source controls, not original caller/layout acceptance."""
from pathlib import Path
import shutil

import pytest
from test_leaf_neutral_views_actual_source import compile_run, extract, extract_struct

ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/IMAGE_PIN").is_file())


def test_complete_original_binding_is_unclaimed():
    from match_recovery.source import read_file
    row, = [row for row in read_file(ROOT / "src/mapwin.cpp")
            if row.address == 0x46FC30]
    assert row.image_spans == ((0x46FC30, 0x46FC7B),)
    assert row.symbol == "?clear_all_altitude@MapWin@@QAEXXZ"
    assert not row.byte_exact


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None, "short_samples", "extra_sample",
                                    "zero_fill", "extra_row", "missing_null_gate"))
def test_altitude_reset_preserves_other_fields_and_signed_count(compiler_name, damage):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " is unavailable")
    record = extract_struct((ROOT / "src/mapwin.h").read_text(), "TerrainCacheEntry")
    body = extract((ROOT / "src/mapwin.cpp").read_text(),
                   "void MapWin::clear_all_altitude()")
    if damage == "short_samples":
        body = body.replace("sample < 5", "sample < 4", 1)
    elif damage == "extra_sample":
        body = body.replace("sample < 5", "sample < 6", 1)
    elif damage == "zero_fill":
        body = body.replace("altitude[sample] = -1", "altitude[sample] = 0", 1)
    elif damage == "extra_row":
        body = body.replace("tile < static_cast<int>(MapArea)",
                            "tile <= static_cast<int>(MapArea)", 1)
    elif damage == "missing_null_gate":
        body = body.replace("if (owned_ != 0)", "if (true)", 1)
    prefix = "#include <assert.h>\n#include <string.h>\n" + record + r'''
unsigned int MapArea;
struct MapWin {
    TerrainCacheEntry *owned_;
    void clear_all_altitude();
};
'''
    main = r'''
int main() {
    TerrainCacheEntry cells[4], before[4];
    memset(cells, 0x35, sizeof cells);
    memcpy(before, cells, sizeof cells);
    MapWin win;
    win.owned_ = cells;
    unsigned int empty_counts[] = {0, 0x80000000u, 0xffffffffu};
    for (int i = 0; i < 3; ++i) {
        MapArea = empty_counts[i];
        win.clear_all_altitude();
        assert(memcmp(cells, before, sizeof cells) == 0);
    }
    MapArea = 3;
    win.clear_all_altitude();
    for (int row = 0; row < 3; ++row) {
        for (int sample = 0; sample < 5; ++sample)
            assert(cells[row].altitude[sample] == -1);
        assert(memcmp(cells[row].brightness, before[row].brightness, 4) == 0);
        assert(cells[row].terrain_bits == before[row].terrain_bits);
        assert(cells[row].altitude_detail == before[row].altitude_detail);
    }
    assert(memcmp(&cells[3], &before[3], sizeof cells[3]) == 0);
    win.clear_all_altitude();
    assert(memcmp(&cells[3], &before[3], sizeof cells[3]) == 0);
    win.owned_ = 0;
    MapArea = 3;
    win.clear_all_altitude();
    return 0;
}
'''
    compile_run(compiler, prefix + body + main, expected=damage is None)
