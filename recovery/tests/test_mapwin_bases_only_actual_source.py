"""Actual-source fidelity control for MapWin::gen_bases_only's live row bound."""

from pathlib import Path
import os
import re
import shlex
import shutil
import subprocess
import tempfile

import pytest


ROOT = Path(__file__).resolve().parents[2]


def _definition(text: str, signature: str) -> str:
    match = re.search(r"(?m)^\s*" + re.escape(signature) + r"\s*\{", text)
    assert match, "missing source definition: " + signature
    opening = text.index("{", match.start())
    depth = 0
    state = "code"
    index = opening
    while index < len(text):
        char = text[index]
        next_char = text[index + 1] if index + 1 < len(text) else ""
        if state == "code":
            if char == "/" and next_char == "/":
                state = "line"
                index += 1
            elif char == "/" and next_char == "*":
                state = "block"
                index += 1
            elif char == '"':
                state = "string"
            elif char == "'":
                state = "char"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return text[match.start():index + 1]
        elif state == "line" and char == "\n":
            state = "code"
        elif state == "block" and char == "*" and next_char == "/":
            state = "code"
            index += 1
        elif state in ("string", "char"):
            if char == "\\":
                index += 1
            elif (state == "string" and char == '"') or (
                    state == "char" and char == "'"):
                state = "code"
        index += 1
    raise AssertionError("unterminated source definition: " + signature)


def _actual_source() -> tuple[str, str]:
    source = (ROOT / "src/mapwin.cpp").read_text()
    helpers = "\n\n".join(
        _definition(source, signature)
        for signature in ("int map_dest_signed(uint32_t value)",
                          "int mapwin_dest_add(int left, int right)")
    )
    body = _definition(source, "void MapWin::gen_bases_only()")
    return helpers, body


HARNESS = r"""
#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

void require_at(bool condition, int line) {
    if (!condition) {
        fprintf(stderr, "assertion failed at line %d\n", line);
        exit(93);
    }
}
#define require(value) require_at((value), __LINE__)

struct Tile { unsigned int bit; unsigned char val2; };
typedef Tile Map;
int MapLongitudeBounds, MapLatitudeBounds, MapIsFlat;
Tile Tiles[16 * 16];
struct Coordinate { int x, y; };
std::vector<Coordinate> MapReads;
Tile *map_loc(int x, int y) {
    require(x >= 0 && x < MapLongitudeBounds);
    require(y >= 0 && y < MapLatitudeBounds);
    MapReads.push_back(Coordinate{x, y});
    return &Tiles[y * MapLongitudeBounds + x];
}
struct Console { int faction_id_; } ConsoleGlobal;
struct GameStateFields { uint32_t game_state_; };
struct GameState { GameStateFields fields_; } GameDataStateStorage;
static const uint32_t STATE_OMNISCIENT_VIEW = 0x00000001U;

struct BaseDraw { int x, y, pixel_x, pixel_y, faction; };
std::vector<BaseDraw> BaseDraws;
int Mutation;
struct MapWin {
    int field_1DDA4_, field_1DDA8_, field_1DDC0_;
    int field_1DDCC_, field_1DDD0_, field_1DDD4_, field_1DDD8_;
    void tile_to_pixel(int x, int y, int *pixel_x, int *pixel_y) {
        *pixel_x = x * 10 + 100;
        *pixel_y = y * 20 + 200;
    }
    void draw_bases(int x, int y, int pixel_x, int pixel_y, int faction);
    void gen_bases_only();
};

HELPERS
BODY

void MapWin::draw_bases(int x, int y, int pixel_x, int pixel_y, int faction) {
    BaseDraws.push_back(BaseDraw{x, y, pixel_x, pixel_y, faction});
    if (Mutation == 1 && BaseDraws.size() == 1) field_1DDD8_ = 2;
    if (Mutation == 2 && BaseDraws.size() == 1) field_1DDD8_ = 0;
}

void reset(MapWin &window) {
    MapLongitudeBounds = 8;
    MapLatitudeBounds = 8;
    MapIsFlat = 1;
    window.field_1DDA4_ = 2;
    window.field_1DDA8_ = 2;
    window.field_1DDC0_ = 3;
    window.field_1DDCC_ = 1;
    window.field_1DDD0_ = 0;
    window.field_1DDD4_ = 1;
    window.field_1DDD8_ = 1;
    ConsoleGlobal.faction_id_ = 5;
    GameDataStateStorage.fields_.game_state_ = 0;
    Mutation = 0;
    MapReads.clear();
    BaseDraws.clear();
    for (int i = 0; i < 16 * 16; ++i) {
        Tiles[i].bit = 1;
        Tiles[i].val2 = 7;
    }
}

void dynamic_growth() {
    MapWin window;
    reset(window);
    Mutation = 1;
    window.gen_bases_only();
    require(BaseDraws.size() == 3);
    require(BaseDraws[0].x == 2 && BaseDraws[0].y == 2);
    require(BaseDraws[1].x == 3 && BaseDraws[1].y == 3);
    require(BaseDraws[2].x == 2 && BaseDraws[2].y == 4);
}

void dynamic_shrink() {
    MapWin window;
    reset(window);
    window.field_1DDD8_ = 3;
    Mutation = 2;
    window.gen_bases_only();
    require(BaseDraws.size() == 1);
    require(BaseDraws[0].x == 2 && BaseDraws[0].y == 2);
}

void initial_limits_and_wrapping() {
    MapWin window;
    reset(window);
    window.field_1DDD8_ = -1;
    window.gen_bases_only();
    require(MapReads.empty() && BaseDraws.empty());

    reset(window);
    window.field_1DDD0_ = INT_MAX;
    window.field_1DDD8_ = 1;
    window.gen_bases_only();
    require(MapReads.empty() && BaseDraws.empty());

    reset(window);
    MapLongitudeBounds = 8;
    MapIsFlat = 0;
    window.field_1DDA4_ = 7;
    window.field_1DDA8_ = 1;
    window.field_1DDD8_ = 0;
    window.field_1DDD4_ = 2;
    window.gen_bases_only();
    require(BaseDraws.size() == 2);
    require(BaseDraws[0].x == 7 && BaseDraws[1].x == 1);
}

void parity_predicates_and_arguments() {
    MapWin window;
    reset(window);
    window.field_1DDD0_ = 2;
    window.field_1DDD8_ = 1;
    window.field_1DDD4_ = 2;
    window.field_1DDCC_ = 2;
    Tiles[2 * MapLongitudeBounds + 4].bit = 0;
    Tiles[3 * MapLongitudeBounds + 3].val2 = 8;
    GameDataStateStorage.fields_.game_state_ = STATE_OMNISCIENT_VIEW;
    window.gen_bases_only();
    const int expected[6][2] = {{2,2},{5,3},{2,4},{4,4},{3,5},{5,5}};
    require(BaseDraws.size() == 6);
    for (int i = 0; i < 6; ++i) {
        require(BaseDraws[i].x == expected[i][0]);
        require(BaseDraws[i].y == expected[i][1]);
        require(BaseDraws[i].pixel_x == expected[i][0] * 10 + 100);
        require(BaseDraws[i].pixel_y == expected[i][1] * 20 + 200 - 3);
        require(BaseDraws[i].faction == -1);
    }
    require(MapReads.size() == 8);
}

int main() {
    dynamic_growth();
    dynamic_shrink();
    initial_limits_and_wrapping();
    parity_predicates_and_arguments();
    return 0;
}
"""


def _compiler() -> list[str]:
    configured = os.environ.get("CXX")
    if configured:
        return shlex.split(configured)
    found = shutil.which("c++")
    if not found:
        pytest.skip("no host C++ compiler available")
    return [found]


def _compile_and_run(compiler: list[str], source: str, temp: Path, label: str) -> subprocess.CompletedProcess:
    cpp = temp / (label + ".cpp")
    exe = temp / label
    cpp.write_text(source)
    built = subprocess.run(compiler + ["-std=c++11", "-O0", str(cpp), "-o", str(exe)],
                           capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(exe)], capture_output=True, text=True)


def test_actual_gen_bases_only_reloads_the_original_live_row_bound():
    helpers, actual_body = _actual_source()
    assert "row < mapwin_dest_add(mapwin_dest_add(field_1DDD0_, field_1DDD8_), 1)" in actual_body
    harness = HARNESS.replace("HELPERS", helpers).replace("BODY", actual_body)
    frozen = actual_body.replace(
        "for (int row = 0;\n"
        "         row < mapwin_dest_add(mapwin_dest_add(field_1DDD0_, field_1DDD8_), 1);\n"
        "         ++row) {",
        "const int rows = mapwin_dest_add(mapwin_dest_add(field_1DDD0_, field_1DDD8_), 1);\n"
        "    for (int row = 0; row < rows; ++row) {",
    )
    assert frozen != actual_body, "the frozen-bound negative control must mutate the actual extracted body"
    frozen_harness = HARNESS.replace("HELPERS", helpers).replace("BODY", frozen)

    with tempfile.TemporaryDirectory(prefix="mapwin-bases-only-") as directory:
        temp = Path(directory)
        actual = _compile_and_run(_compiler(), harness, temp, "actual")
        assert actual.returncode == 0, actual.stderr
        negative = _compile_and_run(_compiler(), frozen_harness, temp, "frozen-bound-mutant")
        assert negative.returncode == 93, (
            "frozen-bound mutant must compile and fail a harness assertion; "
            "got status %d, stderr=%r" % (negative.returncode, negative.stderr)
        )

