"""Host controls for the complete, extracted TutWin::tut_map source body."""
from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/tutwin.cpp"
SIGNATURE = "int TutWin::tut_map("


def _definition(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    state = "code"
    quote = ""
    i = opening
    while i < len(text):
        char = text[i]
        next_char = text[i + 1] if i + 1 < len(text) else ""
        if state == "line":
            if char == "\n":
                state = "code"
        elif state == "block":
            if char == "*" and next_char == "/":
                state = "code"
                i += 1
        elif state == "literal":
            if char == "\\":
                i += 1
            elif char == quote:
                state = "code"
        elif char == "/" and next_char == "/":
            state = "line"
            i += 1
        elif char == "/" and next_char == "*":
            state = "block"
            i += 1
        elif char in "\"'":
            quote = char
            state = "literal"
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return text[start:i + 1]
        i += 1
    raise AssertionError("unterminated TutWin::tut_map definition")


def _actual_body() -> str:
    return _definition(SOURCE.read_text(encoding="utf-8"), SIGNATURE)


HARNESS = r"""
#include <cstdio>
#include <cstdlib>

struct Sprite { int tag; };
struct GraphicWin {
    explicit GraphicWin(int value) : tag(value) {}
    virtual ~GraphicWin() {}
    int tag;
};

struct MapWin : virtual GraphicWin {
    explicit MapWin(int value) : GraphicWin(value) {}
    int field_1DDA4_ = 0;
    int field_1DDA8_ = 0;
    int field_1DDBC_ = 0;
    int field_1DDC0_ = 0;
    int field_1DDCC_ = 0;
    int field_1DDD0_ = 0;
    int field_1DDD4_ = 0;
    int field_1DDD8_ = 0;
    int focus(int x_coord, int y_coord);
    void tile_to_pixel(int tile_x, int tile_y, int *pixel_x, int *pixel_y);
};

struct MapWinTableType {
    MapWin *slots[1] = {nullptr};
    MapWin *operator[](unsigned index) const { return slots[index]; }
};

static MapWinTableType MapWinTable;
static unsigned MapIsFlat;
static int MapLongitudeBounds;
static int focus_replacement_tag;
static int tile_replacement_tag[4];
static int focus_calls, tile_calls;
static int focus_receiver[4], focus_x[4], focus_y[4];
static int tile_receiver[4], recorded_tile_x[4], recorded_tile_y[4];

static MapWin *by_tag[8];

int MapWin::focus(int x_coord, int y_coord) {
    const int n = focus_calls++;
    if (n < 4) {
        focus_receiver[n] = this->tag;
        focus_x[n] = x_coord;
        focus_y[n] = y_coord;
    }
    if (focus_replacement_tag != 0)
        MapWinTable.slots[0] = by_tag[focus_replacement_tag];
    return 0;
}

void MapWin::tile_to_pixel(int tile_x, int tile_y, int *pixel_x, int *pixel_y) {
    const int n = tile_calls++;
    if (n < 4) {
        tile_receiver[n] = this->tag;
        recorded_tile_x[n] = tile_x;
        recorded_tile_y[n] = tile_y;
    }
    *pixel_x = 100 + 10 * this->tag + tile_x;
    *pixel_y = 200 + 10 * this->tag + tile_y;
    if (n < 4 && tile_replacement_tag[n] != 0)
        MapWinTable.slots[0] = by_tag[tile_replacement_tag[n]];
}

struct TutWin {
    int field_537C_ = -101;
    int field_5380_ = -102;
    int field_53AC_ = -103;
    int result = 17;
    int calls = 0;
    int owner_tag = -1;
    const char *text_seen = nullptr;
    int pixel_x_seen = 0, pixel_y_seen = 0;
    Sprite *sprite_seen = nullptr;
    int forwarded_seen = 0;
    int arrow_x_seen = 0, arrow_y_seen = 0;
    int tut_win(GraphicWin *owner, const char *text, int pixel_x, int pixel_y,
                Sprite *sprite, int forwarded, int arrow_x, int arrow_y) {
        ++calls;
        owner_tag = owner == nullptr ? -1 : owner->tag;
        text_seen = text;
        pixel_x_seen = pixel_x;
        pixel_y_seen = pixel_y;
        sprite_seen = sprite;
        forwarded_seen = forwarded;
        arrow_x_seen = arrow_x;
        arrow_y_seen = arrow_y;
        return result;
    }
    ACTUAL_BODY
};

static MapWin maps[4] = {MapWin(1), MapWin(2), MapWin(3), MapWin(4)};
static void reset_events() {
    focus_calls = tile_calls = 0;
    focus_replacement_tag = 0;
    for (int i = 0; i != 4; ++i) {
        tile_replacement_tag[i] = 0;
        focus_receiver[i] = focus_x[i] = focus_y[i] = -999;
        tile_receiver[i] = recorded_tile_x[i] = recorded_tile_y[i] = -999;
    }
}

static void init_maps() {
    for (int i = 0; i != 4; ++i) {
        by_tag[i + 1] = &maps[i];
        maps[i].field_1DDA4_ = 5000;
        maps[i].field_1DDA8_ = 700;
        maps[i].field_1DDCC_ = 10;
        maps[i].field_1DDD0_ = 5;
        maps[i].field_1DDD4_ = 20;
        maps[i].field_1DDD8_ = 20;
        maps[i].field_1DDBC_ = 10 * (i + 1);
        maps[i].field_1DDC0_ = 20 * (i + 1);
    }
    maps[2].field_1DDA4_ = 990;
    maps[2].field_1DDA8_ = 100;
}

#define REQUIRE(condition, diagnostic) do { \
    if (!(condition)) { std::fprintf(stderr, "ASSERT: %s\n", diagnostic); return 1; } \
} while (0)

static int fresh_receiver_post_call_values_and_arrow_owner() {
    reset_events();
    init_maps();
    MapWinTable.slots[0] = &maps[0];
    focus_replacement_tag = 2; // focus on 1, next receiver must be 2
    tile_replacement_tag[0] = 3; // first conversion on 2, post-call map is 3
    tile_replacement_tag[1] = 4; // arrow conversion on 3, post-call map is 4
    MapIsFlat = 0;
    MapLongitudeBounds = 1000;
    TutWin tut;
    const char *label = "MAP";
    const int result = tut.tut_map(label, 9, 7, -1, nullptr, 2, 5, 110);

    REQUIRE(result == 17 && tut.calls == 1, "events/result: tut_win call and return");
    REQUIRE(focus_calls == 1 && focus_receiver[0] == 1 &&
            focus_x[0] == 9 && focus_y[0] == 7,
            "events/receivers: focus receiver and arguments");
    REQUIRE(tile_calls == 2 && tile_receiver[0] == 2 &&
            tile_receiver[1] == 3 && recorded_tile_x[0] == 9 &&
            recorded_tile_y[0] == 7 && recorded_tile_x[1] == 5 &&
            recorded_tile_y[1] == 110,
            "events/receivers: fresh tile receivers and preserved input y/arrow x");
    REQUIRE(tut.pixel_x_seen == 159 && tut.pixel_y_seen == 287,
            "events/offsets: first outputs use map 3 post-call offsets");
    REQUIRE(tut.arrow_x_seen == 175 && tut.arrow_y_seen == 420,
            "events/offsets: arrow outputs use map 4 post-call offsets");
    REQUIRE(tut.owner_tag == 4 && tut.text_seen == label &&
            tut.sprite_seen == nullptr && tut.forwarded_seen == 2,
            "events/owner: final owner follows last post-call reload");
    REQUIRE(tut.field_537C_ == 9 && tut.field_5380_ == 7 &&
            tut.field_53AC_ == -103,
            "events/fields: pixel_x-negative sprite-null branch despite forwarded value");
    return 0;
}

static int forwarded_sprite_and_arrow_disabled_path() {
    reset_events();
    init_maps();
    MapWinTable.slots[0] = &maps[0];
    focus_replacement_tag = 0;
    tile_replacement_tag[0] = 2;
    tile_replacement_tag[1] = 0;
    MapIsFlat = 1;
    MapLongitudeBounds = 1000;
    Sprite marker = {88};
    TutWin tut;
    tut.field_537C_ = 61;
    tut.field_5380_ = 62;
    const char *label = "FORWARD";
    const int result = tut.tut_map(label, 21, 33, 4, &marker, -1, -1, 110);

    REQUIRE(result == 17 && tut.calls == 1, "events/result: disabled-arrow tut_win call");
    REQUIRE(focus_calls == 1 && focus_receiver[0] == 1 &&
            tile_calls == 1 && tile_receiver[0] == 1 &&
            recorded_tile_x[0] == 21 && recorded_tile_y[0] == 33,
            "events/receivers: ordinary fresh receiver sequence");
    REQUIRE(tut.owner_tag == 2 && tut.pixel_x_seen == 151 &&
            tut.pixel_y_seen == 283 && tut.arrow_x_seen == -1 &&
            tut.arrow_y_seen == -1,
            "events/owner: disabled arrow retains post-tile map and initialized arrow outputs");
    REQUIRE(tut.field_537C_ == 61 && tut.field_5380_ == 62 &&
            tut.field_53AC_ == 4 && tut.sprite_seen == &marker &&
            tut.forwarded_seen == -1 && tut.text_seen == label,
            "events/fields: pixel_x store preserves sprite and forwarded argument");
    return 0;
}

int main() {
    if (fresh_receiver_post_call_values_and_arrow_owner()) return 1;
    if (forwarded_sprite_and_arrow_disabled_path()) return 1;
    return 0;
}
"""


def _program(body: str) -> str:
    return HARNESS.replace("ACTUAL_BODY", body.replace("int TutWin::tut_map(", "int tut_map(", 1))


def _cached_pointer_mutant(body: str) -> str:
    # Recreate the prior snapshot behavior: acquire once and reuse it for focus,
    # both conversions, the later member reads, and the final owner.
    focus = "MapWinTable[0]->focus("
    assert body.count(focus) == 1
    assert body.count("MapWinTable[0]") >= 3
    mutant = body.replace("MapWinTable[0]", "cached_map")
    focus_at = mutant.index("cached_map->focus(")
    line_start = mutant.rfind("\n", 0, focus_at) + 1
    mutant = (mutant[:line_start] + "    MapWin *cached_map = MapWinTable[0];\n" +
              mutant[line_start:])
    assert "MapWinTable[0]->tile_to_pixel(" not in mutant
    assert mutant.count("cached_map") >= 4
    return mutant


def _compile_and_run(tmp_path: Path, compiler: str, body: str, name: str):
    cpp = tmp_path / (name + ".cpp")
    exe = tmp_path / name
    cpp.write_text(_program(body), encoding="utf-8")
    subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         str(cpp), "-o", str(exe)],
        check=True, capture_output=True, text=True, timeout=60,
    )
    return subprocess.run([str(exe)], capture_output=True, text=True, timeout=20)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_complete_tut_map_tracks_receiver_replacements_and_values(
    tmp_path: Path, compiler_name: str,
) -> None:
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    actual = _actual_body()
    result = _compile_and_run(tmp_path, compiler, actual, "tut-map-actual")
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_cached_map_pointer_mutant_fails_receiver_assertion(
    tmp_path: Path, compiler_name: str,
) -> None:
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    actual = _actual_body()
    result = _compile_and_run(
        tmp_path, compiler, _cached_pointer_mutant(actual), "tut-map-cached-mutant",
    )
    diagnostic = result.stderr.lower()
    assert result.returncode != 0, "cached MapWin pointer mutant unexpectedly passed"
    assert "assert:" in diagnostic and "events/receivers" in diagnostic, result.stderr
