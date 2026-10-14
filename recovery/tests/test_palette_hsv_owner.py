"""Host controls over the actual Palette::create_table body.

The HSV declarations and method body come from the product source. Palette
services are typed doubles; this does not exercise the game's RGB or palette
search implementations.
"""

from __future__ import annotations

import re
import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]


def braced_region(source: str, start: int) -> str:
    opening = source.find("{", start)
    assert opening >= 0
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start : index + 1]
    raise AssertionError("unterminated source body")


def actual_hsv_declaration() -> str:
    header = (ROOT / "src/jackal/palette.h").read_text()
    match = re.search(r"(?m)^struct HSV\s*\{", header)
    assert match, "missing product HSV declaration"
    return braced_region(header, match.start()) + ";"


def actual_create_table_body() -> str:
    source = (ROOT / "src/jackal/palette.cpp").read_text()
    signature = "int Palette::create_table(unsigned char *table, int start, int count,"
    start = source.find(signature)
    assert start >= 0, "missing actual Palette::create_table definition"
    return braced_region(source, start)


def actual_hsv_scale() -> str:
    source = (ROOT / "src/jackal/palette.cpp").read_text()
    match = re.search(r"(?m)^const double hsv_percent_scale\s*=\s*([^;]+);", source)
    assert match, "missing product hsv_percent_scale initializer"
    return match.group(1)


def harness(hsv_declaration: str, hsv_scale: str, body: str) -> str:
    return r"""
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstring>

struct PALETTEENTRY { unsigned char peRed, peGreen, peBlue, peFlags; };
__HSV_DECLARATION__

static void *PaletteInitialized = reinterpret_cast<void *>(1);
static const double hsv_percent_scale = __HSV_SCALE__;
static int conversions;
static int searches;
static int protocol_error;
static int active_brightness;
static double original_v[256];

class Palette {
public:
    PALETTEENTRY entries_[256];
    int create_table(unsigned char *table, int start, int count,
                     int brightness_boost);
    int get_nearest_palette_index(const HSV *query, const HSV *block,
                                  int skip_animated);
};

static void reset_observers(int brightness) {
    conversions = searches = protocol_error = 0;
    active_brightness = brightness;
}

static void RGB_to_HSV(const PALETTEENTRY *entry, HSV *out) {
    ++conversions;
    const int index = static_cast<int>(entry->peRed);
    out->h = static_cast<double>(index);
    out->s = static_cast<double>(entry->peGreen) / 255.0;
    out->v = static_cast<double>(entry->peBlue) / 255.0;
    original_v[index] = out->v;
}

int Palette::get_nearest_palette_index(const HSV *query, const HSV *block,
                                      int skip_animated) {
    ++searches;
    const int index = static_cast<int>(query->h);
    if (index < 0 || index >= 256 || skip_animated != 1 ||
        query == &block[index]) {
        protocol_error = 1;
        return 0;
    }
    const double first = static_cast<double>(active_brightness + 100) *
                         original_v[index];
    const double expected = first * hsv_percent_scale;
    if (query->s != static_cast<double>(255 - index) / 255.0 ||
        query->v != expected || block[index].v != original_v[index]) {
        protocol_error = 1;
    }
    return index ^ 0x55;
}

__BODY__

static int run_case(int brightness) {
    Palette palette = {};
    unsigned char table[256];
    for (int i = 0; i < 256; ++i) {
        palette.entries_[i].peRed = static_cast<unsigned char>(i);
        palette.entries_[i].peGreen = static_cast<unsigned char>(255 - i);
        palette.entries_[i].peBlue = static_cast<unsigned char>(i);
        palette.entries_[i].peFlags = 0;
        table[i] = 0xA5;
    }
    reset_observers(brightness);
    if (palette.create_table(table, 20, 6, brightness) != 0) return 10;
    if (conversions != 256 || searches != 6 || protocol_error) return 11;
    for (int i = 0; i < 256; ++i) {
        const unsigned char expected = (i >= 20 && i < 26)
            ? static_cast<unsigned char>(i ^ 0x55)
            : static_cast<unsigned char>(i);
        if (table[i] != expected) return 12;
    }
    return 0;
}

int main() {
    Palette palette = {};
    unsigned char table[256];
    std::memset(table, 0x5A, sizeof(table));
    reset_observers(0);
    PaletteInitialized = 0;
    if (palette.create_table(table, 0, 1, 0) != 7 || conversions || searches)
        return 1;
    if (table[0] != 0x5A) return 2;

    PaletteInitialized = reinterpret_cast<void *>(1);
    if (palette.create_table(0, 0, 1, 0) != 0x10 || conversions || searches)
        return 3;
    if (palette.create_table(table, 0, 1, 101) != 3 || conversions || searches)
        return 4;
    if (palette.create_table(table, 0, 1, -101) != 3 || conversions || searches)
        return 5;

    if (run_case(-100) != 0) return 6;
    if (run_case(0) != 0) return 7;
    if (run_case(100) != 0) return 8;
    return 0;
}
""".replace("__HSV_DECLARATION__", hsv_declaration).replace(
    "__HSV_SCALE__", hsv_scale
).replace("__BODY__", body)


def compile_and_run(
    tmp_path: Path, compiler: str, body: str, name: str
) -> tuple[subprocess.CompletedProcess[str], subprocess.CompletedProcess[str] | None]:
    translation_unit = harness(actual_hsv_declaration(), actual_hsv_scale(), body)
    source_path = tmp_path / f"{name}.cpp"
    binary_path = tmp_path / name
    source_path.write_text(translation_unit)
    compiled = subprocess.run(
        [compiler, "-std=c++11", "-O0", str(source_path), "-o", str(binary_path)],
        capture_output=True,
        text=True,
        check=False,
    )
    if compiled.returncode != 0:
        return compiled, None
    ran = subprocess.run([str(binary_path)], capture_output=True, text=True, check=False)
    return compiled, ran


@pytest.fixture(params=("g++", "clang++"), scope="module")
def host_compiler(request: pytest.FixtureRequest) -> str:
    compiler = shutil.which(str(request.param))
    if not compiler:
        pytest.skip(f"{request.param} is not installed")
    return compiler


def test_actual_create_table_body(host_compiler: str, tmp_path: Path) -> None:
    body = actual_create_table_body()
    compiled, ran = compile_and_run(tmp_path, host_compiler, body, "baseline")
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    assert ran is not None
    assert ran.returncode == 0, ran.stdout + ran.stderr


@pytest.mark.parametrize(
    ("name", "before", "after"),
    [
        (
            "wrong_record",
            "local1 = hsv[i];",
            "local1 = hsv[(i + 1) & 0xff];",
        ),
        (
            "wrong_brightness",
            "brightness_boost + 100",
            "brightness_boost + 99",
        ),
        (
            "reordered_multiplication",
            "double scaled = (double)(brightness_boost + 100) * local1.v;\n"
            "        local1.v = scaled * hsv_percent_scale;",
            "double scaled = (double)(brightness_boost + 100) * hsv_percent_scale;\n"
            "        local1.v = scaled * local1.v;",
        ),
    ],
)
def test_compiling_mutants_fail_runtime(
    host_compiler: str,
    tmp_path: Path,
    name: str,
    before: str,
    after: str,
) -> None:
    body = actual_create_table_body()
    assert body.count(before) == 1, f"missing unique mutation anchor for {name}"
    mutant = body.replace(before, after)
    compiled, ran = compile_and_run(tmp_path, host_compiler, mutant, name)
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    assert ran is not None
    assert ran.returncode != 0, f"{name} compiled and escaped the runtime control"
