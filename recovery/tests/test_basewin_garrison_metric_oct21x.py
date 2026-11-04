"""Actual-source controls for draw_garrison's originally inline Font metric."""

from __future__ import annotations

from pathlib import Path
import re
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/basewin.cpp"
FONT_SOURCE = ROOT / "src/buffer.cpp"
SIGNATURE = "void BaseWin::draw_garrison(int update_mode)"


def extract_function(source: str, signature: str) -> str:
    from match_recovery.includes import COMMENTS_OR_STRINGS

    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: re.sub(r"[^\n]", " ", match[0]), source
    )
    hits = [match.start() for match in re.finditer(re.escape(signature), masked)]
    assert len(hits) == 1, f"expected one definition for {signature!r}"
    start = hits[0]
    opening = masked.index("{", start)
    depth = 0
    for end in range(opening, len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError(f"unterminated function {signature!r}")


def actual_metric_block() -> str:
    body = extract_function(SOURCE.read_text(encoding="utf-8"), SIGNATURE)
    start_anchor = "    const int metric_adjustment = font.unk_1_;"
    end_anchor = "\n\n    int left = garrison_rect_.left;"
    assert body.count(start_anchor) == 1
    start = body.index(start_anchor)
    end = body.index(end_anchor, start)
    block = body[start:end]
    assert "font.line_advance()" not in block
    return block


def actual_public_metric() -> str:
    source = FONT_SOURCE.read_text(encoding="utf-8")
    return extract_function(source, "int Font::line_advance() const")


def actual_metric_helper() -> str:
    return extract_function(
        (ROOT / "src/jackal/font_metrics.h").read_text(encoding="utf-8"),
        "inline int font_line_advance(")


FIXTURE = r"""
#include <stdint.h>
#include <stdio.h>

static int failures;
static void check(int value, const char *name) {
    if (!value) {
        fprintf(stderr, "FAIL: %s\n", name);
        ++failures;
    }
}

class Font {
public:
    int unk_1_;
    int line_height_;
    int height_;
    mutable int pixel_reads;
    mutable int line_reads;

    int pixel_height() const { ++pixel_reads; return height_; }
    int line_height() const { ++line_reads; return line_height_; }
    int line_advance() const;
};
/*__METRIC_HELPER__*/
/*__PUBLIC_METRIC__*/

static int metric_from_draw_garrison(Font &font) {
    /*__ACTUAL_METRIC_BLOCK__*/
    return line_height;
}

static void check_case(int adjustment, int height, int line_height,
                       int expected, const char *name) {
    Font font = { adjustment, line_height, height, 0, 0 };
    const int actual = metric_from_draw_garrison(font);
    check(actual == expected, name);
    const int pixel_reads_after_inline = font.pixel_reads;
    const int line_reads_after_inline = font.line_reads;
    if (adjustment < 0) {
        check(pixel_reads_after_inline == 0, "negative reads only line-height accessor");
        check(line_reads_after_inline == 1, "negative selects line-height once");
    } else {
        check(pixel_reads_after_inline == 1, "nonnegative selects pixel-height once");
        check(line_reads_after_inline == 0, "nonnegative does not read line-height");
    }
    check(actual == font.line_advance(), "inline selection agrees with Font API");
    if (adjustment < 0) {
        check(font.pixel_reads == pixel_reads_after_inline, "public negative read count");
        check(font.line_reads == line_reads_after_inline + 1, "public negative selector read");
    } else {
        check(font.pixel_reads == pixel_reads_after_inline + 1, "public pixel selector read");
        check(font.line_reads == line_reads_after_inline, "public positive avoids line-height");
    }
}

int main() {
    check_case(-1, 10, 14, 14, "negative chooses line height");
    check_case(-2147483647 - 1, 10, 17, 17, "INT_MIN chooses line height");
    check_case(0, 10, 17, 10, "zero adjustment chooses pixel height");
    check_case(3, 10, 17, 13, "positive adjustment is added");
    check_case(1, 2147483647, 17, (-2147483647 - 1),
               "addition wraps as 32-bit arithmetic");
    check_case(2, 2147483647, 17, (-2147483647),
               "multi-step addition preserves wrapped value");
    return failures == 0 ? 0 : 1;
}
"""


def compile_run(tmp_path: Path, metric: str, compiler_name: str) -> subprocess.CompletedProcess[str]:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    source = tmp_path / f"garrison_metric_{compiler_name}.cpp"
    binary = tmp_path / f"garrison_metric_{compiler_name}"
    fixture = FIXTURE.replace("/*__ACTUAL_METRIC_BLOCK__*/", metric)
    fixture = fixture.replace("/*__METRIC_HELPER__*/", actual_metric_helper())
    fixture = fixture.replace("/*__PUBLIC_METRIC__*/", actual_public_metric())
    source.write_text(fixture, encoding="utf-8")
    subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(source), "-o", str(binary)],
        check=True,
        capture_output=True,
        text=True,
        timeout=60,
    )
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_draw_garrison_metric_matches_font_selector(tmp_path, compiler_name):
    result = compile_run(tmp_path, actual_metric_block(), compiler_name)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_compiled_metric_mutants_are_rejected(tmp_path, compiler_name):
    block = actual_metric_block()
    mutants = (
        ("if (metric_adjustment >= 0)", "if (metric_adjustment > 0)"),
        ("font.pixel_height()", "font.line_height()"),
        ("static_cast<uint32_t>(font_height) +\n            static_cast<uint32_t>(metric_adjustment)",
         "static_cast<uint32_t>(font_height)"),
        ("line_height = font.line_height();", "line_height = font.pixel_height();"),
    )
    for index, (old, new) in enumerate(mutants):
        assert old in block
        mutant = block.replace(old, new, 1)
        case = tmp_path / str(index)
        case.mkdir()
        result = compile_run(case, mutant, compiler_name)
        assert result.returncode != 0, f"mutant {index} survived"
