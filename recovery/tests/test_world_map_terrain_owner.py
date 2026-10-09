"""Actual-source controls for the terrain-fixup receiver ordering, not game acceptance."""
from pathlib import Path
import shutil
import subprocess
import pytest

ROOT = Path(__file__).resolve().parents[2]
COMPILERS = [p for p in (shutil.which("g++"), shutil.which("clang++")) if p]


def _function():
    source = (ROOT / "src/mapwin.cpp").read_text()
    start = source.index("void __cdecl mapwin_terrain_fixup() {")
    end = source.index("\n}\n", start) + 2
    return source[start:end]


def test_tutorial_mapwin_is_existing_basewin_member_with_lifetime():
    mapwin_source = (ROOT / "src/mapwin.cpp").read_text()
    basewin_source = (ROOT / "src/basewin.cpp").read_text()
    basewin_header = (ROOT / "src/basewin.h").read_text()

    assert "RECOVERY_OBJECT: v1 0x006A7628 ?TutWinBaseWindow@@3VBaseWin@@A" in basewin_source
    assert "BaseWin TutWinBaseWindow;  // 0x006A7628" in basewin_source
    assert "MapWin mapWin_;  // 0x1E674" in basewin_header
    assert "BaseWin::BaseWin() {" in basewin_source
    assert "BaseWin::~BaseWin() {}" in basewin_source
    assert "ORIGINAL: 0x00408490 ??0BaseWin@@QAE@XZ" in basewin_source
    assert "ORIGINAL: 0x00420A70 ??1BaseWin@@QAE@XZ" in basewin_source
    assert "TutWinBaseWindow.mapWin_" in _function()
    assert "MapWinTerrainFixupExtra2" not in mapwin_source


@pytest.mark.parametrize("compiler", COMPILERS or [None])
def test_real_world_tutorial_members_and_slot_order(tmp_path, compiler):
    if compiler is None:
        pytest.skip("no host C++ compiler")
    body = _function()
    assert "reinterpret_cast" not in body
    assert "MapWinTerrainFixupExtra1" not in (ROOT / "src/mapwin.cpp").read_text()
    harness = r"""
#include <stddef.h>
#define __cdecl
struct MapWin { int field_1DD74_; };
struct WorldWin { MapWin mapWin_; };
WorldWin WorldClimateWorldWin;
struct BaseWin { char prefix[0x1E674]; MapWin mapWin_; };
BaseWin TutWinBaseWindow;
MapWin FarmMapWindow, Slots[4];
const unsigned int MapWinTableSlots = 4;
MapWin *MapWinTable[MapWinTableSlots];
MapWin *calls[8]; unsigned int count;
void terrain_fixup(MapWin *window) { calls[count++] = window; }
""" + body + r"""
int main() {
    if (offsetof(BaseWin, mapWin_) != 0x1E674) return 3;
    for (unsigned int i=0; i<4; ++i) MapWinTable[i]=&Slots[i];
    Slots[0].field_1DD74_=0; Slots[1].field_1DD74_=1;
    Slots[2].field_1DD74_=0; Slots[3].field_1DD74_=-1;
    mapwin_terrain_fixup();
    if (count!=6 || calls[0]!=&Slots[0] || calls[1]!=&Slots[1] ||
        calls[2]!=&Slots[3] || calls[3]!=&WorldClimateWorldWin.mapWin_ ||
        calls[4]!=&TutWinBaseWindow.mapWin_ || calls[5]!=&FarmMapWindow) return 1;
    // Native skips the active-field read only for slot zero, even if null.
    count=0; MapWinTable[0]=0; mapwin_terrain_fixup();
    if (count!=6 || calls[0]!=0) return 2;
    return 0;
}
"""
    for name, text, succeeds in (
        ("actual", harness, True),
        ("missing-world", harness.replace("terrain_fixup(&WorldClimateWorldWin.mapWin_);", ""), False),
        ("missing-tutorial", harness.replace("terrain_fixup(&TutWinBaseWindow.mapWin_);", ""), False),
        ("slot-zero-gated", harness.replace("slot == 0 || ", ""), False),
    ):
        # The slot-zero mutant fails its first assertion before the null case.
        cpp, exe = tmp_path / (name + ".cpp"), tmp_path / name
        cpp.write_text(text)
        built = subprocess.run([compiler,"-std=c++98","-Wall","-Wextra","-Werror",str(cpp),"-o",str(exe)], capture_output=True,text=True,timeout=30)
        assert built.returncode == 0, built.stdout + built.stderr
        result = subprocess.run([str(exe)],capture_output=True,text=True,timeout=30)
        assert (result.returncode == 0) == succeeds
