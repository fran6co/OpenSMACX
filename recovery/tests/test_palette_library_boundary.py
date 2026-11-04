"""Source ownership controls for the generic Jackal Palette boundary."""

from pathlib import Path
import re

from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]


def test_application_palette_storage_and_bodies_are_outside_the_library():
    generic = (ROOT / "src/jackal/palette.cpp").read_text()
    generic_header = (ROOT / "src/jackal/palette.h").read_text()
    # Consumer evidence in comments does not create an application dependency.
    without_comments = lambda text: re.sub(r"//[^\n]*|/\*.*?\*/", "", text, flags=re.S)
    app = (ROOT / "src/interface_palette.cpp").read_text()
    assets = (ROOT / "src/interface_assets.cpp").read_text()
    app_header = (ROOT / "src/interface_palette.h").read_text()

    for app_only in ("prefs_get", "PrefWinGammaCorrection", "TextColourTable",
                     "textcolor_init", "load_palette"):
        assert app_only not in without_comments(generic)
        assert app_only not in without_comments(generic_header)
    assert 'extern Palette g_PALETTE1;' not in generic_header
    assert 'extern Palette g_PALETTE2;' not in generic_header
    assert '#include "gamma.h"' not in generic
    assert '#include "alpha.h"' not in generic
    for app_only in ("g_PALETTE1", "g_PALETTE2", "TextColourTable",
                     "textcolor_init"):
        assert app_only in app
    assert "load_palette" in assets
    assert "load_palette" not in app
    assert "extern Palette g_PALETTE1;" in app_header
    assert "extern Palette g_PALETTE2;" in app_header
    assert "extern int TextColourTable[195];" in app_header
    assert "PrefWinGammaCorrection" in assets

    generic_rows = read_file(ROOT / "src/jackal/palette.cpp")
    app_rows = read_file(ROOT / "src/interface_palette.cpp")
    asset_rows = read_file(ROOT / "src/interface_assets.cpp")
    assert sum(row.address == 0x005FE650 for row in generic_rows) == 1
    assert sum(row.address == 0x004BA220 for row in app_rows) == 1
    assert sum(row.address == 0x00451A10 for row in asset_rows) == 1
    assert not any(row.address == 0x00451A10 for row in app_rows)
    assert not any(row.address in (0x004BA220, 0x00451A10) for row in generic_rows)


def test_palette_archive_membership_and_application_consumers_are_explicit():
    root_cmake = (ROOT / "CMakeLists.txt").read_text()
    jackal_cmake = (ROOT / "src/jackal/CMakeLists.txt").read_text()
    build = (ROOT / "recovery/build.json").read_text()
    assert "src/interface_palette.cpp" in root_cmake
    assert "src/palette.cpp" not in root_cmake
    target, = re.findall(r"add_library\(jackal\s+STATIC\s+([^)]*)\)", jackal_cmake, re.S)
    assert {"jackal.cpp", "palette.cpp", "font.cpp"} <= set(target.split())
    assert '"object": "src/jackal/CMakeFiles/jackal.dir/palette.cpp.obj"' in build

    consumers = (
        "basewin", "designwin", "dipedit", "map", "picktech", "pickwin",
        "alphamenu", "alphasave", "diplowin", "popup", "prefwin", "replaywin",
        "statuswin", "veh", "game", "main", "newgame", "alphamovie",
    )
    for name in consumers:
        source = (ROOT / f"src/{name}.cpp").read_text()
        assert '#include "interface_palette.h"' in source, name
