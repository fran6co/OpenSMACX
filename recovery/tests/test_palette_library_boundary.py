"""Source ownership controls for the generic Jackal Palette boundary."""

from pathlib import Path

from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]


def test_application_palette_storage_and_bodies_are_outside_the_library():
    generic = (ROOT / "src/jackal/palette.cpp").read_text()
    generic_header = (ROOT / "src/jackal/palette.h").read_text()
    app = (ROOT / "src/interface_palette.cpp").read_text()
    app_header = (ROOT / "src/interface_palette.h").read_text()

    for app_only in ("prefs_get", "PrefWinGammaCorrection", "TextColourTable",
                     "textcolor_init", "load_palette"):
        assert app_only not in generic
        assert app_only not in generic_header
    assert 'extern Palette g_PALETTE1;' not in generic_header
    assert 'extern Palette g_PALETTE2;' not in generic_header
    assert '#include "gamma.h"' not in generic
    assert '#include "alpha.h"' not in generic
    for app_only in ("g_PALETTE1", "g_PALETTE2", "TextColourTable",
                     "textcolor_init", "load_palette"):
        assert app_only in app
    assert "extern Palette g_PALETTE1;" in app_header
    assert "extern Palette g_PALETTE2;" in app_header
    assert "extern int TextColourTable[195];" in app_header
    assert "PrefWinGammaCorrection" in app

    generic_rows = read_file(ROOT / "src/jackal/palette.cpp")
    app_rows = read_file(ROOT / "src/interface_palette.cpp")
    assert sum(row.address == 0x005FE650 for row in generic_rows) == 1
    for address in (0x004BA220, 0x00451A10):
        assert sum(row.address == address for row in app_rows) == 1
        assert not any(row.address == address for row in generic_rows)


def test_palette_archive_membership_and_application_consumers_are_explicit():
    root_cmake = (ROOT / "CMakeLists.txt").read_text()
    jackal_cmake = (ROOT / "src/jackal/CMakeLists.txt").read_text()
    build = (ROOT / "recovery/build.json").read_text()
    assert "src/interface_palette.cpp" in root_cmake
    assert "src/palette.cpp" not in root_cmake
    assert "add_library(jackal STATIC jackal.cpp palette.cpp font.cpp)" in jackal_cmake
    assert '"object": "src/jackal/CMakeFiles/jackal.dir/palette.cpp.obj"' in build

    consumers = (
        "basewin", "designwin", "dipedit", "map", "picktech", "pickwin",
        "alphamenu", "alphasave", "diplowin", "popup", "prefwin", "replaywin",
        "statuswin", "veh", "game", "main", "newgame", "alphamovie",
    )
    for name in consumers:
        source = (ROOT / f"src/{name}.cpp").read_text()
        assert '#include "interface_palette.h"' in source, name
