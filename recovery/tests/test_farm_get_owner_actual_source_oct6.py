"""Actual-source controls for BaseWin's farm raster reader.

The asset-domain evidence is specific to the pinned installed Rainfall.pcx.
"""
from __future__ import annotations

from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

import pytest

from match_recovery.source import read_text

ROOT = Path(__file__).resolve().parents[2]
SIGNATURE = "int BaseWin::farm_get(int a1, int a2)"


def installed_basewin() -> str:
    text = (ROOT / "src/basewin.cpp").read_text()
    rows = {row.address: row for row in read_text(text, ROOT / "src/basewin.cpp")}
    assert 0x00409100 in rows and rows[0x00409100].source_role.value == "unclassified"
    return text


def extract(text: str, signature: str) -> str:
    start = text.index(signature)
    open_brace = text.index("{", start)
    depth = 0
    state = "code"
    i = open_brace
    while i < len(text):
        c = text[i]
        n = text[i + 1] if i + 1 < len(text) else ""
        if state == "code":
            if c == "/" and n == "*": state = "block"; i += 2; continue
            if c == "/" and n == "/": state = "line"; i += 2; continue
            if c == '"': state = "string"
            elif c == "'": state = "char"
            elif c == "{": depth += 1
            elif c == "}":
                depth -= 1
                if depth == 0:
                    return text[start:i + 1]
        elif state == "block":
            if c == "*" and n == "/": state = "code"; i += 2; continue
        elif state == "line":
            if c == "\n": state = "code"
        else:
            if c == "\\": i += 2; continue
            if (state == "string" and c == '"') or (state == "char" and c == "'"):
                state = "code"
        i += 1
    raise AssertionError("unclosed farm_get actual body")


PRELUDE = r'''
#include <cassert>
#include <cstdio>
struct Rect { int right, left, top; };
struct Buffer {
  int calls, last_x, last_y;
  static Buffer *expected;
  static Buffer *receiver;
  int get_pixel(int x, int y) {
    ++calls; last_x=x; last_y=y; receiver=this; return 10;
  }
};
Buffer *Buffer::expected = 0;
Buffer *Buffer::receiver = 0;
struct BaseWin { Rect rect1_; Buffer buffer2_; int farm_get(int, int); };
const int RiverStepX[4] = {1, 1, -1, -1};
const int RiverStepY[4] = {-1, 1, 1, -1};
static int radius_calls, radius_x, radius_y, radius_range;
int radius_move(int x, int y, int range) {
  ++radius_calls; radius_x=x; radius_y=y; radius_range=range; return 77;
}
'''


def make_source(body: str, wrong_receiver: bool = False) -> str:
    if wrong_receiver:
        body = body.replace("buffer2_.get_pixel", "reinterpret_cast<Buffer *>(this)->get_pixel", 1)
    check = r'''
int main() {
  BaseWin win = {};
  win.rect1_.right=200; win.rect1_.left=100; win.rect1_.top=30;
  Buffer::expected=&win.buffer2_;
  int result=win.farm_get(18,44);
  assert(result==77);
  assert(Buffer::receiver==Buffer::expected);
  assert(win.buffer2_.calls==1 && win.buffer2_.last_x==60 && win.buffer2_.last_y==14);
  assert(radius_calls==1 && radius_x==-3 && radius_y==-3 && radius_range==0x15);
  assert(win.farm_get(-43,44)==-1);
  assert(win.buffer2_.calls==1 && radius_calls==1);
  assert(win.farm_get(18,29)==-1);
  assert(win.buffer2_.calls==1 && radius_calls==1);
  puts("actual projected farm_get receiver controls passed");
}
'''
    return PRELUDE + body + check


COMPILERS = [c for c in ("g++", "clang++") if shutil.which(c)]
pytestmark = pytest.mark.skipif(not COMPILERS, reason="no host C++ compiler available")


@pytest.mark.parametrize("compiler", COMPILERS)
def test_farm_get_uses_real_buffer2_subobject_and_rejects_cast_mutant(compiler: str) -> None:
    projected = installed_basewin()
    body = extract(projected, SIGNATURE)
    assert "buffer2_.get_pixel" in body
    assert "reinterpret_cast<Buffer *>(this)->get_pixel" not in body
    for wrong_receiver in (False, True):
        source = make_source(body, wrong_receiver)
        with tempfile.TemporaryDirectory(prefix="farm-owner-host-") as temp_name:
            binary = Path(temp_name) / "control"
            built = subprocess.run(
                [compiler, "-std=c++11", "-Wall", "-Werror", "-x", "c++", "-", "-o", str(binary)],
                input=source, text=True, capture_output=True, timeout=30,
            )
            assert built.returncode == 0, built.stderr
            ran = subprocess.run([str(binary)], text=True, capture_output=True, timeout=10)
            if wrong_receiver:
                assert ran.returncode != 0, "wrong BaseWin-as-Buffer receiver survived"
            else:
                assert ran.returncode == 0, ran.stderr
                assert "actual projected farm_get receiver controls passed" in ran.stdout
