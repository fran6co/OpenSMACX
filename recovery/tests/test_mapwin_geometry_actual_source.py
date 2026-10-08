"""Actual-body host controls for MapWin's dimensions and pixel conversion.

The test extracts complete product bodies and never rewrites their statements. VC6/native receiver adjustment is outside this host proof.
"""

from pathlib import Path
import os
import re
import shutil
import subprocess
import pytest


ROOT = Path(__file__).resolve().parents[2]


def braced(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start : end + 1]
    raise AssertionError(f"unterminated actual definition: {signature}")


def member_declaration(header: str, name: str) -> str:
    for raw in header.splitlines():
        line = raw.split("//", 1)[0].strip()
        if line.endswith(";") and re.search(r"\b" + re.escape(name) + r"\b", line):
            return line
    raise AssertionError(f"no actual MapWin declaration for {name}")


def actual_parts():
    map_header = (ROOT / "src/mapwin.h").read_text()
    buffer_header = (ROOT / "src/buffer.h").read_text()
    source = (ROOT / "src/mapwin.cpp").read_text()
    calculate = braced(source, "void MapWin::calculate_dimensions()")
    pixel = braced(source, "int MapWin::pixel_to_tile(")
    for body in (calculate, pixel):
        assert "entered_receiver" not in body
        assert "reinterpret_cast<int" not in body
        assert "0x4c4" not in body.lower()
        assert "0x4c8" not in body.lower()
    assert "int32_t w = " not in calculate and "int32_t h = " not in calculate
    assert calculate.count("pixel_width()") == 3
    assert calculate.count("dib_.bmiHeader.biHeight") == 3
    assert "field_1DDAC_ = (pixel_width() - tileProduct) / 2;" in calculate
    assert "field_1DDB0_ = -((dib_.bmiHeader.biHeight + tileProduct2) / 2);" in calculate
    assert "this == static_cast<MapWin *>(&ConsoleGlobal)" in calculate
    assert "font_a_.init(DefaultFontFace, fs1, 0)" in calculate
    assert "font_c_.init(DefaultFontFace, fs2, 2)" in calculate
    assert "font_b_.init(DefaultFontFace, fs3, 1)" in calculate
    assert re.search(
        r"if\s*\(\(x_pixel\s*<\s*pixel_width\(\)\)\s*&&\s*"
        r"\(y_pixel\s*<\s*pixel_height\(\)\)\)", pixel
    )

    field_names = set(re.findall(r"\bfield_[A-Za-z0-9_]+\b", calculate + pixel))
    field_names.update(("owned_", "font_a_", "font_b_", "font_c_"))
    declarations = [member_declaration(map_header, name) for name in sorted(field_names)]
    initializers = [name + " = 0;" for name in sorted(field_names)
                    if name.startswith("field_")]
    initializers.append("owned_ = 0;")

    dib = braced(buffer_header, "struct Dib : BITMAPINFO") + ";"
    cache = braced(map_header, "struct TerrainCacheEntry") + ";"
    width_getter = braced(buffer_header, "int pixel_width() const")
    height_getter = braced(buffer_header, "int pixel_height() const")
    # Avoid earlier inline uses of dib_ (e.g. clipping expressions); include
    # the actual typed storage declaration from Buffer.
    dib_member_match = re.search(r"^\s*Dib\s+dib_\s*;", buffer_header, re.M)
    assert dib_member_match, "Buffer's actual Dib dib_ storage declaration missing"
    dib_member = dib_member_match.group(0).strip()
    pixel_decl = re.search(r"^\s*int pixel_to_tile\([^;]+;", map_header, re.M).group(0).strip()
    calc_decl = re.search(r"^\s*void calculate_dimensions\(\);", map_header, re.M).group(0).strip()
    alt_decl = re.search(r"^\s*int get_alt\([^;]+;", map_header, re.M).group(0).strip()
    return {
        "calculate": calculate,
        "pixel": pixel,
        "dib": dib,
        "cache": cache,
        "width_getter": width_getter,
        "height_getter": height_getter,
        "dib_member": dib_member,
        "map_declarations": "\n".join(declarations + [calc_decl, pixel_decl, alt_decl]),
        "initializers": "\n".join(initializers),
    }


HARNESS = r'''#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <cassert>

typedef const char *LPCSTR;
struct BITMAPINFOHEADER {
    uint32_t biSize; int32_t biWidth, biHeight; uint16_t biPlanes, biBitCount;
    uint32_t biCompression, biSizeImage; int32_t biXPelsPerMeter, biYPelsPerMeter;
    uint32_t biClrUsed, biClrImportant;
};
struct RGBQUAD { uint8_t blue, green, red, reserved; };
struct BITMAPINFO { BITMAPINFOHEADER bmiHeader; RGBQUAD bmiColors[1]; };
struct RECT { int32_t left, top, right, bottom; };
CACHE_POD
DIB_POD
struct Font;
struct FontCall { const Font *receiver; LPCSTR name; int height, style; };
static std::vector<FontCall> font_calls;
static LPCSTR FontOne, FontTwo, DefaultFontFace;
static int mutate_default_font;
struct Font {
    int init(LPCSTR name, int height, int style) {
        FontCall call = {this, name, height, style};
        font_calls.push_back(call);
        if (mutate_default_font && font_calls.size() == 1) DefaultFontFace = FontTwo;
        return 0;
    }
};
struct Win {};
struct Buffer {
    DIB_MEMBER
    WIDTH_GETTER
    HEIGHT_GETTER
};
struct GraphicWin : public Win, public Buffer {};
struct MapWin : public virtual GraphicWin {
public:
    MAP_DECLARATIONS
    MapWin() { INITIALIZERS }
};
struct Console : public MapWin {};
Console ConsoleGlobal;
RECT MapWinPrimaryCorner;
int32_t MapWinCaptionScale, MapWinCaptionDivisor;
int MapLongitudeBounds, MapLatitudeBounds, MapIsFlat;
uint32_t MapArea;
struct GameFields { uint32_t game_more_preferences_; };
struct GameData { GameFields fields_; } GameDataStateStorage;
static std::vector<int> alt_arguments;
int MapWin::get_alt(int x, int y, int corner) {
    alt_arguments.push_back(x); alt_arguments.push_back(y); alt_arguments.push_back(corner);
    return corner == 1 ? -1 : 0;
}
CALCULATE_BODY
PIXEL_BODY

static void init_pixel(MapWin &w, int longitude, int flat) {
    w.dib_.bmiHeader.biWidth = 100;
    w.dib_.bmiHeader.biHeight = -80;
    w.field_1DDAC_ = 0; w.field_1DDB0_ = 0;
    w.field_1DDA4_ = 0; w.field_1DDA8_ = 0;
    w.field_1DDB4_ = 20; w.field_1DDB8_ = 20;
    w.field_1DDBC_ = 10; w.field_1DDC0_ = 10;
    MapLongitudeBounds = longitude; MapLatitudeBounds = 100; MapIsFlat = flat;
    alt_arguments.clear();
}
static void init_dimensions(MapWin &w, int width, int signed_height,
                            int zoom, int bounds) {
    w.dib_.bmiHeader.biWidth = width;
    w.dib_.bmiHeader.biHeight = signed_height;
    w.field_1DD98_ = zoom; w.field_1EEC0_ = 0;
    w.field_1EEC4_ = 25; w.field_1DDA0_ = 25; w.field_1DD9C_ = 25;
    w.field_1EEF0_ = 0; w.field_1EF48_ = 0; w.field_1EF1C_ = 0;
    w.field_1DD70_ = 0; w.owned_ = 0;
    MapWinCaptionScale = -1; MapWinCaptionDivisor = 16;
    MapLongitudeBounds = bounds; MapLatitudeBounds = bounds; MapIsFlat = 1;
    MapArea = 0; GameDataStateStorage.fields_.game_more_preferences_ = 0;
}
int main() {
    FontOne = "Arial"; FontTwo = "Tahoma"; DefaultFontFace = FontOne;
    // Full extracted pixel_to_tile body: valid interior, inclusive lower side,
    // width/height exclusive boundaries, and signed-negative caller inputs.
    MapWin map;
    init_pixel(map, 100, 1);
    int tile_x = 777, tile_y = 888;
    assert(map.pixel_to_tile(50, 20, &tile_x, &tile_y) == 0);
    assert(tile_x == 5 && tile_y == 1);
    assert((alt_arguments == std::vector<int>{6, 2, 1, 6, 2, 2}));

    // Cylindrical seam: a negative candidate coordinate wraps once to the
    // final tile; all arithmetic remains small and representable.
    init_pixel(map, 8, 0);
    map.field_1DDA4_ = -6;
    tile_x = 777; tile_y = 888;
    assert(map.pixel_to_tile(50, 20, &tile_x, &tile_y) == 0);
    assert(tile_x == 7 && tile_y == 1);
    assert((alt_arguments == std::vector<int>{8, 2, 1, 8, 2, 2}));

    // Rejected paths leave caller outputs alone and never query altitude.
    init_pixel(map, 100, 1);
    tile_x = 777; tile_y = 888;
    assert(map.pixel_to_tile(100, 20, &tile_x, &tile_y) == 1);
    assert(tile_x == 777 && tile_y == 888 && alt_arguments.empty());
    assert(map.pixel_to_tile(50, 80, &tile_x, &tile_y) == 1);
    assert(tile_x == 777 && tile_y == 888 && alt_arguments.empty());
    assert(map.pixel_to_tile(-1, 20, &tile_x, &tile_y) == 1);
    assert(tile_x == 777 && tile_y == 888 && alt_arguments.empty());
    assert(map.pixel_to_tile(50, -1, &tile_x, &tile_y) == 1);
    assert(tile_x == 777 && tile_y == 888 && alt_arguments.empty());

    // Full extracted calculate_dimensions body: independently vary actual
    // Buffer DIB fields and verify visible tile extents use both dimensions.
    // Only the Console-derived receiver publishes the primary rectangle.
    // The rectangle uses initial extents, before the subsequent map clamp.
    init_dimensions(ConsoleGlobal, 250, -100, 20, 2);
    mutate_default_font = 0;
    ConsoleGlobal.calculate_dimensions();
    assert(MapWinPrimaryCorner.left == 27 && MapWinPrimaryCorner.right == 23);
    assert(MapWinPrimaryCorner.top == 27 && MapWinPrimaryCorner.bottom == 21);
    font_calls.clear();

    MapWin dimensions;
    init_dimensions(dimensions, 250, -100, 20, 2);
    mutate_default_font = 1;
    dimensions.calculate_dimensions();
    assert(MapWinPrimaryCorner.left == 27 && MapWinPrimaryCorner.right == 23);
    assert(MapWinPrimaryCorner.top == 27 && MapWinPrimaryCorner.bottom == 21);
    assert(dimensions.field_1DDC8_ == 28);
    assert(dimensions.field_1DDCC_ == 1 && dimensions.field_1DDD4_ == 1);
    assert(dimensions.field_1DDD0_ == 1 && dimensions.field_1DDD8_ == 2);
    assert(dimensions.field_1DDAC_ == 13 && dimensions.field_1DDB0_ == -6);
    assert(font_calls.size() == 3);
    assert(font_calls[0].receiver == &dimensions.font_a_ && font_calls[0].style == 0);
    assert(font_calls[1].receiver == &dimensions.font_c_ && font_calls[1].style == 2);
    assert(font_calls[2].receiver == &dimensions.font_b_ && font_calls[2].style == 1);
    assert(font_calls[0].height == 12 && font_calls[1].height == 12 &&
           font_calls[2].height == 12);
    assert(font_calls[0].name == FontOne);
    assert(font_calls[1].name == FontTwo && font_calls[2].name == FontTwo);
    assert(DefaultFontFace == FontTwo);

    MapWin smaller;
    init_dimensions(smaller, 100, -20, -10, 100);
    smaller.field_1EEF0_ = 5; smaller.field_1EF48_ = 5; smaller.field_1EF1C_ = 6;
    font_calls.clear(); mutate_default_font = 0;
    smaller.calculate_dimensions();
    assert(smaller.field_1DDC8_ == 4 && smaller.field_1DDB8_ == 16);
    assert(smaller.field_1DDCC_ == 4 && smaller.field_1DDD4_ == 4);
    assert(smaller.field_1DDD0_ == 2 && smaller.field_1DDD8_ == 2);
    assert(font_calls.empty());
}
'''


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_complete_actual_geometry_methods_with_real_dib_owner(tmp_path, compiler_name):
    p = actual_parts()
    text = (HARNESS.replace("CACHE_POD", p["cache"])
            .replace("DIB_POD", p["dib"])
            .replace("DIB_MEMBER", p["dib_member"])
            .replace("WIDTH_GETTER", p["width_getter"])
            .replace("HEIGHT_GETTER", p["height_getter"])
            .replace("MAP_DECLARATIONS", p["map_declarations"])
            .replace("INITIALIZERS", p["initializers"])
            .replace("CALCULATE_BODY", p["calculate"])
            .replace("PIXEL_BODY", p["pixel"]))
    compiler = shutil.which(compiler_name)
    assert compiler, "host C++ compiler required for the actual-source control"
    cpp, exe = tmp_path / "mapwin-geometry.cpp", tmp_path / "mapwin-geometry"
    cpp.write_text(text)
    build = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         # These are existing product expressions over preserved Windows
         # signed/unsigned declarations. The extracted body and declarations
         # remain unchanged; this host-only flag avoids turning that legacy
         # comparison diagnostic into a fixture failure.
         "-Wno-sign-compare",
         "-fsanitize=address,undefined",
         "-fno-sanitize-recover=all", str(cpp), "-o", str(exe)],
        text=True, capture_output=True, timeout=60,
    )
    assert build.returncode == 0, build.stderr
    run = subprocess.run([str(exe)], text=True, capture_output=True, timeout=15,
                         env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))
    assert run.returncode == 0, run.stderr
