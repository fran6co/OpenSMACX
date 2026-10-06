"""Actual-source controls for BaseWin::farm_get's pinned raster domain."""
from __future__ import annotations

from pathlib import Path
import shutil
import subprocess
import tempfile
import re

import pytest

from match_recovery.source import read_text

ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/basewin.cpp").is_file())
SIGNATURE = "int BaseWin::farm_get(int a1, int a2)"
COMPILERS = [name for name in ("g++", "clang++") if shutil.which(name)]
pytestmark = pytest.mark.skipif(not COMPILERS, reason="no host C++ compiler available")


def extract_function(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    state = "code"
    offset = opening
    while offset < len(text):
        char = text[offset]
        following = text[offset + 1] if offset + 1 < len(text) else ""
        if state == "code":
            if char == "/" and following == "*":
                state = "block"
                offset += 2
                continue
            if char == "/" and following == "/":
                state = "line"
                offset += 2
                continue
            if char == '"':
                state = "string"
            elif char == "'":
                state = "char"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return text[start:offset + 1]
        elif state == "block":
            if char == "*" and following == "/":
                state = "code"
                offset += 2
                continue
        elif state == "line":
            if char == "\n":
                state = "code"
        elif char == "\\":
            offset += 2
            continue
        elif (state == "string" and char == '"') or (state == "char" and char == "'"):
            state = "code"
        offset += 1
    raise AssertionError(f"unclosed function: {signature}")


def source_function(text: str, signature: str) -> str:
    return extract_function(text, signature)


def source_pixel_shift() -> str:
    buffer = (ROOT / "src/buffer.cpp").read_text()
    pcx_loader = source_function(buffer, "int Buffer::load_pcx(BYTE *data")
    match = re.search(
        r"memset\(pixels,\s*(static_cast<uint8_t>\(colour_base \+ value\)),",
        pcx_loader,
    )
    assert match
    return match.group(1)


PRELUDE = r'''
#include <cassert>
#include <cstdio>
#include <stdint.h>
struct Rect { int right, left, top; };
struct Buffer {
  int pixel, calls, last_x, last_y;
  int get_pixel(int x, int y) {
    ++calls; last_x=x; last_y=y; return pixel;
  }
};
struct BaseWin { Rect rect1_; Buffer buffer2_; int farm_get(int, int); };
const int RiverStepX[4] = {1, 1, -1, -1};
const int RiverStepY[4] = {-1, 1, 1, -1};
static int radius_calls, radius_x, radius_y, radius_range;
int radius_move(int x, int y, int range) {
  ++radius_calls; radius_x=x; radius_y=y; radius_range=range; return 77;
}
'''


def harness(body: str, pixel_shift: str) -> str:
    map_source = (ROOT / "src/map.cpp").read_text()
    prelude = PRELUDE
    for name in ("RiverStepX", "RiverStepY"):
        actual = re.search(r"(?m)^const int " + name + r"\[4\]\s*=\s*\{[^}]*\};", map_source)
        assert actual, name
        prelude = re.sub(r"const int " + name + r"\[4\]\s*=\s*\{[^}]*\};", actual[0], prelude)
    main = r'''
int main() {
  BaseWin win = {};
  assert(pcx_shift(10, 3)==13);
  assert(pcx_shift(10, 245)==255);
  assert(pcx_shift(10, 246)==0);
  win.rect1_.right=200; win.rect1_.left=100; win.rect1_.top=30;
  const int pixels[] = {13, 30, 52, 66, 79};
  const int expected_x[] = {-3, -2, -2, -4, -4};
  const int expected_y[] = {-3, -4, -2, -2, -4};
  for (int i=0; i<5; ++i) {
    win.buffer2_.pixel=pixels[i];
    const int result=win.farm_get(0,45);
    assert(result==77);
    assert(win.buffer2_.last_x==42 && win.buffer2_.last_y==15);
    assert(radius_x==expected_x[i] && radius_y==expected_y[i]);
    assert(radius_range==0x15);
  }
  win.buffer2_.pixel=10;
  assert(win.farm_get(0,45)==77);
  assert(radius_x==-3 && radius_y==-3);
  const int before=win.buffer2_.calls;
  const int calls=radius_calls;
  assert(win.farm_get(-43,45)==-1);
  assert(win.farm_get(0,29)==-1);
  assert(win.buffer2_.calls==before && radius_calls==calls);
  puts("actual-source farm raster domain controls passed");
}
'''
    return (prelude + "unsigned char pcx_shift(int colour_base, int value) { return "
            + pixel_shift + "; }\n" + body + main)


@pytest.mark.parametrize("compiler", COMPILERS)
def test_actual_farm_get_maps_all_pinned_asset_indices(compiler: str) -> None:
    text = (ROOT / "src/basewin.cpp").read_text()
    records = {row.address: row for row in read_text(text, ROOT / "src/basewin.cpp")}
    assert 0x00409100 in records
    body = extract_function(text, SIGNATURE)
    assert "buffer2_.get_pixel" in body
    assert "RiverStepX[idx - 1]" in body and "RiverStepY[idx - 1]" in body
    assert "if (idx != 0)" in body
    assert "idx >=" not in body and "idx <" not in body
    pixel_shift = source_pixel_shift()
    with tempfile.TemporaryDirectory(prefix="basewin-farm-source-") as name:
        directory = Path(name)
        executable = directory / "farm-control"
        built = subprocess.run(
            [compiler, "-std=c++11", "-Wall", "-Werror", "-x", "c++", "-",
            "-o", str(executable)], input=harness(body, pixel_shift), text=True,
            capture_output=True, timeout=30,
        )
        assert built.returncode == 0, built.stderr
        ran = subprocess.run([str(executable)], text=True, capture_output=True,
                             timeout=10)
        assert ran.returncode == 0, ran.stderr
        assert "actual-source farm raster domain controls passed" in ran.stdout


def test_source_producer_reader_and_unit_clip_chain() -> None:
    basewin = (ROOT / "src/basewin.cpp").read_text()
    game = (ROOT / "src/game.cpp").read_text()
    sprite = (ROOT / "src/sprite.cpp").read_text()
    terrain_loader = source_function(game, "void __cdecl load_terrain()")
    sprite_producer = source_function(game, "void terrain_extract_sprite(")
    base_init = source_function(basewin, "void BaseWin::init()")
    pixel_shift = source_pixel_shift()
    unit_scale_wrapper = source_function(
        sprite, "int Sprite::draw(Buffer *buffer, int a, int b, int c, int x, int y)"
    )
    unit_draw = source_function(
        sprite, "int Sprite::draw(Buffer *destination, int transparent, int x, int y)"
    )
    assert 'load_pcx("RAINFALL.PCX", 0, 10, 236)' in terrain_loader
    assert "change_color(263, 265)" in sprite_producer
    assert "change_color(262, 265)" in sprite_producer
    assert "sprite.extract(&g_PCX_PARSE_TEMP_BUFFER1, 265, x, y, 100, height, 0)" in sprite_producer
    assert "terrain_extract_sprite(g_RAINFALL_DOUBLE_TILE_SPRITES[0], 1, 52, 100)" in terrain_loader
    assert "buffer2_.init(100, 50, 0, 0)" in base_init
    assert "g_RAINFALL_DOUBLE_TILE_SPRITES[0].draw(&buffer2_" in base_init
    assert ", 0, 0, 1, 1);" in base_init
    assert pixel_shift == "static_cast<uint8_t>(colour_base + value)"
    assert "SpriteScaleNumerator = x" in unit_scale_wrapper
    assert "SpriteScaleDenominator = y" in unit_scale_wrapper
    assert "draw(buffer, a, b, c)" in unit_scale_wrapper
    assert "if (SpriteScaleNumerator == SpriteScaleDenominator)" in unit_draw
    assert "IntersectRect(&clipped, &clipped, &destination->rect1_)" in unit_draw
