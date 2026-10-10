"""Compile Buffer font consumers extracted from their mapped product bodies."""

from __future__ import annotations

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/buffer.cpp"

FUNCTIONS = {
    "text_line_height": (
        "0x005DCAB0", "int Buffer::text_line_height()",
    ),
    "write_cent_l": (
        "0x005DD250", "int Buffer::write_cent_l(char * a1, int a2, int a3, int a4)",
    ),
    "write_right_l": (
        "0x005DD450", "int Buffer::write_right_l(char * a1, RECT * a2, int a3)",
    ),
    "wrap_flying": (
        "0x005DDFB0", "int Buffer::wrap_flying(char * a1)",
    ),
    "wrap": (
        "0x005DD530", "int Buffer::wrap(char * a1, RECT * a2)",
    ),
}


def extract_definition(source: str, address: str, signature: str) -> str:
    marker = f"// ORIGINAL: {address} "
    positions = [index for index in range(len(source))
                 if source.startswith(marker, index)]
    assert len(positions) == 1, f"expected one mapping for {address}"
    start = source.find(signature, positions[0])
    assert start >= 0, f"missing mapped definition for {address}"
    opening = source.find("{", start)
    assert opening >= 0
    depth = 0
    state = "code"
    escaped = False
    index = opening
    while index < len(source):
        char = source[index]
        following = source[index + 1] if index + 1 < len(source) else ""
        if state == "line":
            if char == "\n":
                state = "code"
        elif state == "block":
            if char == "*" and following == "/":
                state = "code"
                index += 1
        elif state in ("string", "char"):
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif (state == "string" and char == '"') or (
                    state == "char" and char == "'"):
                state = "code"
        else:
            if char == "/" and following == "/":
                state = "line"
                index += 1
            elif char == "/" and following == "*":
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
                    return source[start:index + 1]
        index += 1
    raise AssertionError(f"unterminated body for {address}")


FIXTURE = r'''#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

struct RECT { int left, top, right, bottom; };

// Test double: members and pointer distinctions are modeled by name/type;
// this host layout is not evidence for the product ABI or Font extent.
struct Font {
    int unk_1_;
    int is_fot_set_;
    void *font_obj_;
    int line_height_;
    int height_;
};
Font *FontDefault;

int BufferFlyingWidth, BufferFlyingX, BufferFlyingY, BufferStrHeight;
char BufferFlyingText[256];
const char BufferJoinSpace[] = " ";
const char BufferWriteSpace[] = " ";
int BufferLastWrapWriteResult;

struct Buffer {
    Font *font1_;
    int field_10_, field_14_, field_18_;
    int split_after_, break_calls_;
    int measure_width_, write_result_;
    int measure_calls_, write_calls_;
    int clear_font_on_measure_, clear_font_on_write_;
    int last_x_, last_y_, last_length_;
    char last_text_[64];

    Buffer() : font1_(0), field_10_(0), field_14_(0), field_18_(0),
        split_after_(0), break_calls_(0), measure_width_(0), write_result_(73),
        measure_calls_(0), write_calls_(0), clear_font_on_measure_(0),
        clear_font_on_write_(0), last_x_(0), last_y_(0), last_length_(0),
        last_text_() {}

    int text_line_height();
    int write_cent_l(char *, int, int, int);
    int write_right_l(char *, RECT *, int);
    int wrap_flying(char *);
    int wrap(char *, RECT *);

    int text_width(char *, unsigned int) {
        ++measure_calls_;
        if (clear_font_on_measure_) font1_ = 0;
        return measure_width_;
    }
    char *find_line_break_l(char *text, int *, int length) {
        ++break_calls_;
        if (split_after_ > 0 && break_calls_ == 1 && length > split_after_)
            return text + split_after_;
        return 0;
    }
    int write_multi_font_raw_l(char *text, int x, int y, unsigned int length) {
        ++write_calls_;
        last_x_ = x;
        last_y_ = y;
        last_length_ = static_cast<int>(length);
        const unsigned int copy_length = length < sizeof(last_text_) - 1
            ? length : sizeof(last_text_) - 1;
        memcpy(last_text_, text, copy_length);
        last_text_[copy_length] = 0;
        if (clear_font_on_write_) font1_ = 0;
        return write_result_;
    }
};

'''


MAIN = r'''
int main() {
    Font primary = {3, 0, 0, 17, 29};
    Font fallback = {-1, 0, 0, 13, 41};
    int handle_primary = 1, handle_fallback = 2;
    primary.font_obj_ = &handle_primary;
    fallback.font_obj_ = &handle_fallback;
    FontDefault = &fallback;

    // Actual-source metric selection, default publication, and 32-bit wrap.
    Buffer metrics;
    metrics.font1_ = &primary;
    assert(metrics.text_line_height() == 32);
    primary.unk_1_ = -1;
    assert(metrics.text_line_height() == 17);
    metrics.font1_ = 0;
    fallback.unk_1_ = 2;
    assert(metrics.text_line_height() == 43 && metrics.font1_ == &fallback);
    primary.unk_1_ = 1;
    primary.height_ = INT_MAX;
    metrics.font1_ = &primary;
    assert(metrics.text_line_height() == INT_MIN);
    primary.height_ = 29;
    primary.unk_1_ = 3;

    // Actual-source centered writer retains null, handle, empty and valid paths.
    RECT area = {4, 10, 100, 90};
    Buffer centered;
    centered.font1_ = &primary;
    assert(centered.write_cent_l(0, 50, 7, 20) == 50);
    primary.font_obj_ = 0;
    assert(centered.write_cent_l(const_cast<char *>("abc"), 50, 7, 20) == 3);
    primary.font_obj_ = &handle_primary;
    assert(centered.write_cent_l(const_cast<char *>(""), 50, 7, 20) == 50);
    centered.measure_width_ = 12;
    centered.write_result_ = 61;
    char centered_text[] = "abcd";
    assert(centered.write_cent_l(centered_text, 50, 7, 20) == 61);
    assert(centered.write_calls_ == 1 && centered.last_x_ == 44 &&
           centered.last_y_ == 7 && centered.last_length_ == 4);
    assert(centered.write_cent_l(centered_text, 50, 7, 0) == 50);
    assert(centered.write_cent_l(centered_text, 50, 7, -1) == 50);

    // Right writer sees a re-entrant nulling of font1_ after width measurement
    // and lazily reloads the distinct default font for its vertical metric.
    Buffer right;
    right.font1_ = &primary;
    right.measure_width_ = 18;
    right.clear_font_on_measure_ = 1;
    right.write_result_ = 79;
    char right_text[] = "abc";
    assert(right.write_right_l(right_text, &area, 20) == 79);
    assert(right.font1_ == &fallback && right.measure_calls_ == 1);
    assert(right.last_x_ == 82 && right.last_y_ == 29 &&
           right.last_length_ == 3);
    Buffer no_primary;
    assert(no_primary.write_right_l(right_text, &area, 20) == 3);
    primary.font_obj_ = 0;
    Buffer no_handle;
    no_handle.font1_ = &primary;
    assert(no_handle.write_right_l(right_text, &area, 20) == 3);
    primary.font_obj_ = &handle_primary;
    assert(right.write_right_l(0, &area, 20) == 0);

    // wrap_flying draws through a valid HFONT, then observes the callback's
    // pointer clear and uses FontDefault's distinct line-height branch.
    strcpy(BufferFlyingText, "");
    BufferFlyingWidth = 80;
    BufferFlyingX = 12;
    BufferFlyingY = 5;
    BufferStrHeight = 0;
    Buffer flying;
    flying.font1_ = &primary;
    flying.split_after_ = 2;
    flying.clear_font_on_write_ = 1;
    char flying_text[] = "abcd";
    fallback.unk_1_ = -1;
    assert(flying.wrap_flying(flying_text) == 18);
    assert(flying.write_calls_ == 1 && flying.last_x_ == 12 &&
           flying.last_y_ == 5 && strcmp(flying.last_text_, "ab") == 0);
    assert(flying.font1_ == &fallback && strcmp(BufferFlyingText, "cd") == 0);
    primary.font_obj_ = 0;
    strcpy(BufferFlyingText, "");
    BufferFlyingWidth = 80;
    BufferFlyingX = 2;
    BufferFlyingY = 0;
    BufferStrHeight = 0;
    Buffer flying_no_handle;
    flying_no_handle.font1_ = &primary;
    flying_no_handle.split_after_ = 1;
    char one_line[] = "xy";
    assert(flying_no_handle.wrap_flying(one_line) == 32);
    assert(flying_no_handle.write_calls_ == 0);
    primary.font_obj_ = &handle_primary;

    // Rectangular wrap keeps the selected Font's separate override/height
    // metrics, the live slot reload after find_line_break_l, and error value 3.
    RECT wrap_area = {10, 20, 70, 90};
    BufferLastWrapWriteResult = 0;
    Buffer rect_wrap;
    rect_wrap.font1_ = &primary;
    rect_wrap.field_10_ = 3;
    rect_wrap.split_after_ = 2;
    char wrap_text[] = "abcd";
    assert(rect_wrap.wrap(wrap_text, &wrap_area) == 84);
    assert(rect_wrap.write_calls_ == 2 && rect_wrap.last_x_ == 13 &&
           rect_wrap.last_y_ == 52 && strcmp(rect_wrap.last_text_, "cd") == 0);
    assert(BufferLastWrapWriteResult == 73 && rect_wrap.field_14_ == 0);

    primary.font_obj_ = 0;
    Buffer no_handle_wrap;
    no_handle_wrap.font1_ = &primary;
    no_handle_wrap.split_after_ = 2;
    char no_handle_text[] = "abcd";
    assert(no_handle_wrap.wrap(no_handle_text, &wrap_area) == 84);
    assert(no_handle_wrap.write_calls_ == 0 && BufferLastWrapWriteResult == 3);

    Buffer fallback_wrap;
    fallback_wrap.font1_ = 0;
    fallback_wrap.split_after_ = 2;
    char fallback_text[] = "abcd";
    fallback.unk_1_ = 2;
    assert(fallback_wrap.wrap(fallback_text, &wrap_area) == 106);
    assert(fallback_wrap.font1_ == &fallback && fallback_wrap.write_calls_ == 1);
    assert(fallback_wrap.last_y_ == 63);

    RECT safe_empty = {0, 14, 0, 0};
    Buffer null_text;
    assert(null_text.wrap(0, &safe_empty) == 14);
    return 0;
}
'''


def source_program(root: Path, mutation: str | None = None) -> str:
    source = SOURCE.read_text(encoding="utf-8")
    definitions = {
        name: extract_definition(source, *identity)
        for name, identity in FUNCTIONS.items()
    }
    if mutation == "text_height_field":
        old = "static_cast<uint32_t>(font->height_)"
        assert old in definitions["text_line_height"]
        definitions["text_line_height"] = definitions["text_line_height"].replace(
            old, "static_cast<uint32_t>(font->line_height_)", 1)
    elif mutation == "center_handle_polarity":
        old = "font1->font_obj_ != 0"
        assert old in definitions["write_cent_l"]
        definitions["write_cent_l"] = definitions["write_cent_l"].replace(
            old, "font1->font_obj_ == 0", 1)
    elif mutation == "right_default_reload":
        old = "font1_ = FontDefault;"
        assert old in definitions["write_right_l"]
        definitions["write_right_l"] = definitions["write_right_l"].replace(
            old, "font1_ = 0;", 1)
    elif mutation == "flying_handle_polarity":
        old = "font1_->font_obj_ != 0"
        assert old in definitions["wrap_flying"]
        definitions["wrap_flying"] = definitions["wrap_flying"].replace(
            old, "font1_->font_obj_ == 0", 1)
    elif mutation == "rect_wrap_metric":
        old = "lineHeight = fontData->height_ + fontData->unk_1_;"
        assert old in definitions["wrap"]
        definitions["wrap"] = definitions["wrap"].replace(
            old, "lineHeight = fontData->line_height_ + fontData->unk_1_;", 1)
    elif mutation is not None:
        raise AssertionError(f"unknown mutation: {mutation}")
    return FIXTURE + "\n".join(definitions.values()) + MAIN


def test_mapped_consumers_name_font_fields_and_keep_strlen_reloads() -> None:
    source = SOURCE.read_text(encoding="utf-8")
    bodies = {
        name: extract_definition(source, *identity)
        for name, identity in FUNCTIONS.items()
    }
    assert "font->unk_1_" in bodies["text_line_height"]
    assert "font->height_" in bodies["text_line_height"]
    assert "font->line_height_" in bodies["text_line_height"]
    assert "Font *font1 = font1_;" in bodies["write_cent_l"]
    assert "font1->font_obj_" in bodies["write_cent_l"]
    assert "font1_->font_obj_" in bodies["wrap_flying"]
    assert "fontData->height_" in bodies["wrap"]
    assert "fontData->line_height_" in bodies["wrap"]
    assert "reinterpret_cast<const int *>(font)" not in bodies["text_line_height"]
    assert "reinterpret_cast<int *>(font1_" not in bodies["write_cent_l"]
    assert "reinterpret_cast<int *>(reinterpret_cast<int>(font1_)" not in bodies["wrap_flying"]
    assert "reinterpret_cast<Font *>(FontDefault)" not in bodies["write_right_l"]
    assert "reinterpret_cast<Font *>(FontDefault)" not in bodies["wrap"]
    assert bodies["write_cent_l"].count("strlen(a1)") == 4
    assert bodies["write_right_l"].count("strlen(a1)") == 4


def compile_and_run(compiler: str, program: str, tmp_path: Path,
                    label: str) -> subprocess.CompletedProcess[str]:
    source = tmp_path / f"buffer_fonts_{Path(compiler).name}_{label}.cpp"
    executable = tmp_path / f"buffer_fonts_{Path(compiler).name}_{label}"
    source.write_text(program, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         "-fsanitize=undefined", "-fno-sanitize-recover=all",
         str(source), "-o", str(executable)],
        capture_output=True, text=True, timeout=30,
    )
    if built.returncode != 0:
        return built
    return subprocess.run([str(executable)], capture_output=True,
                          text=True, timeout=10)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_font_consumers_preserve_metrics_and_live_fallbacks(
        compiler_name: str, tmp_path: Path) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    result = compile_and_run(compiler, source_program(ROOT), tmp_path, "good")
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("mutation", [
    "text_height_field", "center_handle_polarity", "right_default_reload",
    "flying_handle_polarity", "rect_wrap_metric",
])
def test_actual_font_consumers_reject_member_and_branch_mutants(
        compiler_name: str, mutation: str, tmp_path: Path) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    result = compile_and_run(
        compiler, source_program(ROOT, mutation), tmp_path, mutation)
    assert result.returncode != 0, f"{compiler_name} accepted {mutation} mutation"
