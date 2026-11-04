"""Actual-source controls for Buffer's two originally expanded Font selectors.

These host controls exercise text_line_height and both complete consumer
bodies. They do not establish VC6 emission, native ABI, or linked selection.
"""
from __future__ import annotations

from pathlib import Path
import re
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]
HEADER = ROOT / "src/jackal/font.h"
FONT_SOURCE = ROOT / "src/buffer.cpp"
BUFFER_SOURCE = ROOT / "src/buffer.cpp"


def extract(source: str, signature: str) -> str:
    from match_recovery.includes import COMMENTS_OR_STRINGS

    code = COMMENTS_OR_STRINGS.sub(lambda match: re.sub(r"[^\n]", " ", match[0]), source)
    assert code.count(signature) == 1, f"expected one body for {signature!r}"
    start = code.index(signature)
    opening = code.index("{", start)
    depth = 0
    for end in range(opening, len(code)):
        if code[end] == "{":
            depth += 1
        elif code[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError(f"unterminated body {signature!r}")


def product_parts():
    header = HEADER.read_text(encoding="utf-8")
    font_source = FONT_SOURCE.read_text(encoding="utf-8")
    buffer_source = BUFFER_SOURCE.read_text(encoding="utf-8")
    metrics_source = (ROOT / "src/jackal/font_metrics.h").read_text(encoding="utf-8")
    return {
        "public": extract(metrics_source, "inline int font_line_advance(") + "\n" +
                   extract(font_source, "int Font::line_advance() const"),
        "metric": extract(buffer_source, "int Buffer::text_line_height()"),
        "vertical": extract(buffer_source,
                            "int Buffer::write_vertical(char *text, RECT *area)"),
        "flying": extract(buffer_source,
                          "int Buffer::wrap_cent_flying(char *text)"),
        "buffer": buffer_source,
        "header": header,
    }


FIXTURE = r"""
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

struct RECT { int left, top, right, bottom; };
static int failures;
static void check(int value, const char *name) {
    if (!value) { fprintf(stderr, "FAIL: %s\n", name); ++failures; }
}

class Font {
public:
    int unk_1_;
    int line_height_;
    int height_;
    int pixel_height() const { return height_; }
    int line_height() const { return line_height_; }
    int line_advance() const;
};
/* PRODUCT_PUBLIC */

static Font default_font = { -1, 14, 10 };
static Font *FontDefault = &default_font;
static char BufferFlyingText[128];
static char BufferJoinSpace[] = " ";
static char BufferCenteredSpace[] = " ";
static int BufferFlyingX;
static int BufferFlyingY;
static int BufferFlyingWidth;
static int BufferStrHeight;
static int break_calls;
static int mutate_font_on_write;
static int write_count;
static int write_y[4];

class Buffer {
public:
    Font *fonts_[4];
    RECT rect1_;
    int field_18_;
    RECT active_clip;

    void set_clip(const RECT *r) { active_clip = *r; }
    int write_cent_l(char *text, int x, int y, int width, int length) {
        (void)text; (void)x; (void)width; (void)length;
        if (write_count < 4) write_y[write_count] = y;
        ++write_count;
        if (mutate_font_on_write && write_count == 1) fonts_[0]->unk_1_ = -1;
        return 0;
    }
    int text_width(char *text, int length) { (void)text; return length; }
    char *find_line_break_l(char *text, int *available, int length) {
        (void)available; (void)length;
        ++break_calls;
        return break_calls == 1 ? text + 1 : 0;
    }
    int text_line_height();
    int write_vertical(char *text, RECT *area);
    int wrap_cent_flying(char *text);
};

/* PRODUCT_TEXT_LINE_HEIGHT */
/* PRODUCT_VERTICAL */
/* PRODUCT_FLYING */

int main() {
    Buffer metrics = {};
    Font f = { -1, 14, 20 };
    metrics.fonts_[0] = &f;
    check(metrics.text_line_height() == 14, "negative selects line_height");
    f.unk_1_ = INT_MIN;
    check(metrics.text_line_height() == 14, "INT_MIN selects line_height");
    f.unk_1_ = 0;
    check(metrics.text_line_height() == 20, "zero selects height");
    check(f.line_advance() == 20, "public mapped body remains functional");
    f.unk_1_ = 3;
    check(metrics.text_line_height() == 23, "positive adds adjustment");
    f.height_ = INT_MAX; f.unk_1_ = 1;
    check(metrics.text_line_height() == INT_MIN,
          "addition wraps as defined 32-bit arithmetic");
    FontDefault = &f;
    metrics.fonts_[0] = 0;
    check(metrics.text_line_height() == INT_MIN && metrics.fonts_[0] == &f,
          "Buffer metric lazily installs FontDefault");
    FontDefault = &default_font;

    Buffer b = {};
    Font vertical_font = { 2, 14, 10 };
    b.fonts_[0] = &vertical_font;
    b.rect1_ = (RECT){ 1, 2, 3, 4 };
    RECT area = { 0, 3, 80, 23 };
    check(b.write_vertical((char *)"ab", &area) == 23,
          "vertical line advance is shrunk to fit room");
    check(write_count == 2 && write_y[0] == 3 && write_y[1] == 13,
          "vertical writes preserve per-character advance");
    check(b.active_clip.left == 1 && b.active_clip.top == 2 &&
          b.active_clip.right == 3 && b.active_clip.bottom == 4,
          "vertical writer restores saved clip");

    write_count = 0;
    b.rect1_ = (RECT){ 5, 6, 7, 8 };
    area = (RECT){ 0, 5, 80, 35 };
    check(b.write_vertical((char *)"ab", &area) == 29,
          "vertical writer retains unshrunk selected advance when it fits");
    check(write_count == 2 && write_y[0] == 5 && write_y[1] == 17,
          "vertical fit path uses selected Font metric");

    Buffer flying = {};
    Font flying_font = { 2, 14, 10 };
    flying.fonts_[0] = &flying_font;
    BufferFlyingText[0] = 0;
    BufferFlyingX = 2; BufferFlyingY = 5; BufferFlyingWidth = 100;
    BufferStrHeight = 0; break_calls = 0; write_count = 0;
    mutate_font_on_write = 1;
    check(flying.wrap_cent_flying((char *)"ab") == 19,
          "flying Y reads selector after text-write callback changes Font");
    check(write_count == 1 && write_y[0] == 5 && flying.field_18_ == 1,
          "flying write/count precede Y advance");
    check(strcmp(BufferFlyingText, "b") == 0,
          "flying cursor continues with unconsumed suffix");

    if (failures) return 1;
    return 0;
}
"""


def assemble(parts, mutation=None):
    metric = parts["metric"]
    vertical = parts["vertical"]
    flying = parts["flying"]
    if mutation == "zero_branch":
        metric = metric.replace("height_override >= 0", "height_override > 0", 1)
    elif mutation == "signed_add":
        metric = metric.replace(
            "static_cast<uint32_t>(font->height_)\n            + static_cast<uint32_t>(height_override)",
            "static_cast<uint32_t>(font->height_ + height_override)", 1)
    elif mutation == "vertical_member":
        vertical = vertical.replace("text_line_height()", "fonts_[0]->height_", 1)
    elif mutation == "flying_member":
        flying = flying.replace("text_line_height()", "fonts_[0]->height_", 1)
    elif mutation == "default_assignment":
        metric = metric.replace("fonts_[0] = FontDefault;", "fonts_[0] = 0;", 1)
    return (FIXTURE.replace("/* PRODUCT_PUBLIC */", parts["public"])
            .replace("/* PRODUCT_TEXT_LINE_HEIGHT */", metric)
            .replace("/* PRODUCT_VERTICAL */", vertical)
            .replace("/* PRODUCT_FLYING */", flying))


def compile_run(tmp_path: Path, compiler_name: str, parts, mutation=None):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    source = tmp_path / f"buffer_line_advance_{compiler_name}_{mutation or 'good'}.cpp"
    executable = source.with_suffix("")
    source.write_text(assemble(parts, mutation), encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         "-fsanitize=undefined", "-fno-sanitize-recover=all", str(source),
         "-o", str(executable)], text=True, capture_output=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(executable)], text=True, capture_output=True,
                          timeout=10)


def test_two_expansion_sites_reuse_existing_buffer_metric():
    parts = product_parts()
    assert "measure_line_advance" not in parts["header"]
    assert parts["buffer"].count("fonts_[0]->line_advance()") == 3
    assert "fonts_[0]->measure_line_advance()" not in parts["buffer"]
    assert "int advance = text_line_height();" in parts["vertical"]
    assert "int advance = text_line_height();" in parts["flying"]
    assert parts["vertical"].index("int advance = text_line_height();") < \
        parts["vertical"].index("for (;;)")
    # The loop's later fallback before the separate height clamp is retained.
    assert parts["vertical"].count("if (fonts_[0] == 0) fonts_[0] = FontDefault;") == 1
    assert "if (fonts_[0] == 0) fonts_[0] = FontDefault;" not in parts["flying"]
    # The mapped Font API and existing Buffer metric are complete unchanged bodies.
    assert "int Font::line_advance() const" in parts["public"]
    assert "fonts_[0] = FontDefault;" in parts["metric"]
    assert "height_override >= 0" in parts["metric"]
    assert "font->line_height_" in parts["metric"]


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_metric_and_buffer_bodies(compiler_name, tmp_path):
    result = compile_run(tmp_path, compiler_name, product_parts())
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("mutation", ("zero_branch", "signed_add",
                                      "vertical_member", "flying_member",
                                      "default_assignment"))
def test_compile_success_mutants_are_detected(compiler_name, mutation, tmp_path):
    result = compile_run(tmp_path, compiler_name, product_parts(), mutation)
    assert result.returncode != 0, f"behavior control missed {mutation}"
