"""Actual-source behavioral controls for Font's public/private body sharing.

The host fixture deliberately supplies renamed API spies instead of Windows
headers/imports. It exercises the product bodies, not a transcription of them.
This is a bounded semantic control; physical VC6 emission remains a separate
leased verification obligation.
"""
from pathlib import Path
import os
import re
import shutil
import subprocess
import sys
import tempfile

import pytest

ROOT = Path(__file__).resolve().parents[2]
HEADER = ROOT / "src/jackal/font.h"
SOURCE = ROOT / "src/jackal/font.cpp"
BASE = "b3be21d06467489ac8befa258c111ae77806a108"


def extract(source: str, signature: str) -> str:
    """Return one C++ definition, ignoring braces in comments and strings."""
    from match_recovery.includes import COMMENTS_OR_STRINGS

    code = COMMENTS_OR_STRINGS.sub(lambda match: re.sub(r"[^\n]", " ", match[0]), source)
    assert code.count(signature) == 1, f"expected one definition for {signature!r}"
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
    raise AssertionError(f"unterminated definition: {signature}")


def product_bodies():
    header = HEADER.read_text(encoding="utf-8")
    source = SOURCE.read_text(encoding="utf-8")
    return {
        "release": extract(header, "void release_resources()"),
        "bounded_private": extract(header, "int measure_bounded_width(LPSTR input, size_t max_len)"),
        "close": extract(source, "void Font::close()"),
        "bounded_public": extract(source, "int Font::width(LPSTR input, size_t max_len)"),
        "destructor": extract(source, "Font::~Font()"),
        "line_break": extract(source, "LPSTR Font::find_line_break_l(LPSTR input, int *break_len, size_t len)"),
    }


def compact_body(definition: str) -> str:
    from match_recovery.includes import COMMENTS_OR_STRINGS

    code = COMMENTS_OR_STRINGS.sub(" ", definition)
    opening = code.index("{")
    return re.sub(r"\s+", "", code[opening:])


FIXTURE = r"""
#include <stddef.h>
#include <limits.h>
#include <string.h>

typedef char *LPSTR;
typedef const char *LPCSTR;
typedef int BOOL;
typedef void *HFONT;
typedef void *HDC;
struct SIZE { long cx; long cy; };
enum { SYSTEM_FONT = 13 };

struct Event { int kind; const void *a; const void *b; int value; };
enum { E_DELETE = 1, E_REMOVE, E_FREE, E_STRLEN, E_SELECT, E_EXTENT, E_STOCK };
static Event events[128];
static int event_count;
static int failure_code;
static int nested_remove_once;
static class Font *reenter_owner;
static int extent_width;
static HFONT observed_stock;
static HFONT expected_handle;
static class Font *observed_owner;

static void log_event(int kind, const void *a = 0, const void *b = 0, int value = 0) {
    if (event_count >= 128) { failure_code = 250; return; }
    events[event_count].kind = kind;
    events[event_count].a = a;
    events[event_count].b = b;
    events[event_count].value = value;
    ++event_count;
}
static void reset_events() { event_count = 0; nested_remove_once = 0; reenter_owner = 0; observed_owner = 0; }

static BOOL font_probe_DeleteObject(HFONT value);
static BOOL font_probe_RemoveFontResourceA(LPSTR value);
static void font_probe_free(void *value);
static size_t font_probe_strlen(const char *value);
static HFONT font_probe_SelectObject(HDC dc, HFONT object);
static BOOL font_probe_GetTextExtentPoint32A(HDC dc, LPSTR text, int length, SIZE *size);
static HFONT font_probe_GetStockObject(int object);

#define DeleteObject font_probe_DeleteObject
#define RemoveFontResourceA font_probe_RemoveFontResourceA
#define free font_probe_free
#define strlen font_probe_strlen
#define SelectObject font_probe_SelectObject
#define GetTextExtentPoint32A font_probe_GetTextExtentPoint32A
#define GetStockObject font_probe_GetStockObject
#define FONT_MIN(a, b) ((a) < (b) ? (a) : (b))

class Font {
public:
    Font();
    ~Font();
    void close();
    int width(LPSTR input, size_t max_len);
    LPSTR find_line_break_l(LPSTR input, int *break_len, size_t len);
    void test_private_cleanup() { release_resources(); }
    int test_private_width(LPSTR input, size_t max_len) {
        return measure_bounded_width(input, max_len);
    }
    static HDC FontHDC;
    int unk_1_;
    BOOL is_fot_set_;
    HFONT font_obj_;
    int line_height_;
    int height_;
    int internal_leading_;
    int ascent_;
    int descent_;
    int pad_;
    LPSTR fot_file_name_;
private:
    /* PRODUCT_RELEASE */
    /* PRODUCT_BOUNDED_PRIVATE */
};
#if defined(_M_IX86)
typedef char font_size_check[(sizeof(Font) == 0x28) ? 1 : -1];
typedef char font_offsets_check[(offsetof(Font, unk_1_) == 0x00 && offsetof(Font, is_fot_set_) == 0x04 &&
              offsetof(Font, font_obj_) == 0x08 && offsetof(Font, line_height_) == 0x0c &&
              offsetof(Font, height_) == 0x10 && offsetof(Font, internal_leading_) == 0x14 &&
              offsetof(Font, ascent_) == 0x18 && offsetof(Font, descent_) == 0x1c &&
              offsetof(Font, pad_) == 0x20 && offsetof(Font, fot_file_name_) == 0x24)
              ? 1 : -1];
#endif

HDC Font::FontHDC = reinterpret_cast<HDC>(0x1110);
static int BufferStrHeight;
static HFONT font_probe_GetStockObject(int object) {
    log_event(E_STOCK, 0, 0, object);
    return observed_stock;
}
static BOOL font_probe_DeleteObject(HFONT value) {
    int state = observed_owner && value == expected_handle && observed_owner->font_obj_ == expected_handle &&
        observed_owner->unk_1_ == -1 && observed_owner->height_ == 0 &&
        observed_owner->line_height_ == 0 && observed_owner->ascent_ == 0 &&
        observed_owner->descent_ == 0;
    log_event(E_DELETE, value, 0, state);
    return 1;
}
static BOOL font_probe_RemoveFontResourceA(LPSTR value) {
    int state = observed_owner && observed_owner->font_obj_ == 0 &&
        observed_owner->fot_file_name_ == value && observed_owner->unk_1_ == -1 &&
        observed_owner->height_ == 0 && observed_owner->line_height_ == 0;
    log_event(E_REMOVE, value, reenter_owner, state);
    if (nested_remove_once && reenter_owner) {
        nested_remove_once = 0;
        reenter_owner->close();
    }
    return 1;
}
static void font_probe_free(void *value) {
    int state = observed_owner && observed_owner->fot_file_name_ == value;
    log_event(E_FREE, value, 0, state);
}
static size_t font_probe_strlen(const char *value) {
    log_event(E_STRLEN, value, 0, 0);
    size_t count = 0;
    while (value[count]) ++count;
    return count;
}
static HFONT font_probe_SelectObject(HDC dc, HFONT object) {
    log_event(E_SELECT, dc, object, 0);
    return 0;
}
static BOOL font_probe_GetTextExtentPoint32A(HDC dc, LPSTR text, int length, SIZE *size) {
    log_event(E_EXTENT, dc, text, length);
    size->cx = extent_width;
    size->cy = 0;
    return 1;
}

/* PRODUCT_CLOSE */
/* PRODUCT_BOUNDED_PUBLIC */
/* PRODUCT_DESTRUCTOR */
/* PRODUCT_LINE_BREAK */

Font::Font() : unk_1_(71), is_fot_set_(9), font_obj_(0), line_height_(72),
    height_(73), internal_leading_(74), ascent_(75), descent_(76), pad_(77),
    fot_file_name_(0) {}

static void seed(Font &font, HFONT hfont, LPSTR name) {
    expected_handle = hfont;
    font.unk_1_ = 42;
    font.is_fot_set_ = 9;
    font.font_obj_ = hfont;
    font.line_height_ = 101;
    font.height_ = 102;
    font.internal_leading_ = 103;
    font.ascent_ = 104;
    font.descent_ = 105;
    font.pad_ = 106;
    font.fot_file_name_ = name;
}
static int count_kind(int kind) {
    int count = 0;
    for (int i = 0; i < event_count; ++i) count += events[i].kind == kind;
    return count;
}
static Event nth_kind(int kind, int ordinal) {
    for (int i = 0; i < event_count; ++i) {
        if (events[i].kind == kind && ordinal-- == 0) return events[i];
    }
    failure_code = 251;
    return Event();
}
static int index_kind(int kind, int ordinal) {
    for (int i = 0; i < event_count; ++i) {
        if (events[i].kind == kind && ordinal-- == 0) return i;
    }
    failure_code = 252;
    return event_count;
}
static void require(bool condition, int code) { if (!condition && !failure_code) failure_code = code; }
static int same_text(const char *left, const char *right) {
    while (*left && *left == *right) { ++left; ++right; }
    return *left == *right;
}

static int cleanup_public() {
    Font font;
    char filename[] = "sentinel.fot";
    HFONT handle = reinterpret_cast<HFONT>(0x2220);
    seed(font, handle, filename);
    reset_events();
    observed_owner = &font;
    font.close();
    require(font.unk_1_ == -1 && font.height_ == 0 && font.line_height_ == 0 &&
            font.ascent_ == 0 && font.descent_ == 0, 11);
    require(font.font_obj_ == 0 && font.fot_file_name_ == 0, 12);
    require(font.is_fot_set_ == 9 && font.internal_leading_ == 103 && font.pad_ == 106, 13);
    require(count_kind(E_DELETE) == 1 && count_kind(E_REMOVE) == 1 && count_kind(E_FREE) == 1, 14);
    require(nth_kind(E_DELETE, 0).a == handle, 15);
    require(nth_kind(E_REMOVE, 0).a == filename, 16);
    require(nth_kind(E_FREE, 0).a == filename, 17);
    require(nth_kind(E_DELETE, 0).value == 1 && nth_kind(E_REMOVE, 0).value == 1 &&
            nth_kind(E_FREE, 0).value == 1, 18);
    require(index_kind(E_DELETE, 0) < index_kind(E_REMOVE, 0) &&
            index_kind(E_REMOVE, 0) < index_kind(E_FREE, 0), 19);
    return 0;
}

static int cleanup_reentrant() {
    Font font;
    char filename[] = "reentrant.fot";
    HFONT handle = reinterpret_cast<HFONT>(0x2230);
    seed(font, handle, filename);
    reset_events();
    observed_owner = &font;
    nested_remove_once = 1;
    reenter_owner = &font;
    font.close();
    require(count_kind(E_REMOVE) == 2 && count_kind(E_FREE) == 1, 21);
    require(nth_kind(E_REMOVE, 0).a == filename && nth_kind(E_REMOVE, 1).a == filename, 22);
    require(font.fot_file_name_ == 0 && font.font_obj_ == 0, 23);
    require(font.unk_1_ == -1 && font.height_ == 0 && font.line_height_ == 0, 24);
    require(font.is_fot_set_ == 9 && font.internal_leading_ == 103 && font.pad_ == 106, 25);
    require(nth_kind(E_DELETE, 0).value == 1 && nth_kind(E_REMOVE, 0).value == 1 &&
            nth_kind(E_REMOVE, 1).value == 1 && nth_kind(E_FREE, 0).value == 1, 26);
    return 0;
}

static int cleanup_private_and_destructor() {
    char filename[] = "private.fot";
    HFONT handle = reinterpret_cast<HFONT>(0x2240);
    reset_events();
    {
        Font font;
        seed(font, handle, filename);
        observed_owner = &font;
        font.test_private_cleanup();
        require(font.fot_file_name_ == 0 && font.font_obj_ == 0, 31);
        require(count_kind(E_DELETE) == 1 && count_kind(E_REMOVE) == 1 && count_kind(E_FREE) == 1, 32);
    }
    reset_events();
    {
        Font font;
        seed(font, handle, filename);
        observed_owner = &font;
    }
    require(count_kind(E_DELETE) == 1 && count_kind(E_REMOVE) == 1 && count_kind(E_FREE) == 1, 33);
    return 0;
}

static int cleanup_private_reentrant() {
    Font font;
    char filename[] = "private-reentrant.fot";
    HFONT handle = reinterpret_cast<HFONT>(0x2238);
    seed(font, handle, filename);
    reset_events();
    observed_owner = &font;
    nested_remove_once = 1;
    reenter_owner = &font;
    font.test_private_cleanup();
    require(count_kind(E_REMOVE) == 2 && count_kind(E_FREE) == 1, 27);
    require(font.fot_file_name_ == 0 && font.font_obj_ == 0, 28);
    require(nth_kind(E_DELETE, 0).value == 1 && nth_kind(E_REMOVE, 0).value == 1 &&
            nth_kind(E_REMOVE, 1).value == 1 && nth_kind(E_FREE, 0).value == 1, 29);
    return 0;
}

static int check_width(Font &font, LPSTR text, size_t max_len, int expected_len,
                       int expected_strlen, int code) {
    reset_events();
    extent_width = 17;
    int value = font.width(text, max_len);
    require(value == 17 && count_kind(E_STRLEN) == expected_strlen, code);
    require(count_kind(E_SELECT) == 2 && count_kind(E_EXTENT) == 1 && count_kind(E_STOCK) == 1, code + 1);
    require(nth_kind(E_SELECT, 0).a == Font::FontHDC &&
            nth_kind(E_SELECT, 0).b == font.font_obj_, code + 2);
    require(nth_kind(E_EXTENT, 0).a == Font::FontHDC && nth_kind(E_EXTENT, 0).b == text &&
            nth_kind(E_EXTENT, 0).value == expected_len, code + 3);
    require(nth_kind(E_STOCK, 0).value == SYSTEM_FONT &&
            nth_kind(E_SELECT, 1).b == observed_stock, code + 4);
    require(nth_kind(E_EXTENT, 0).a == nth_kind(E_SELECT, 0).a &&
            nth_kind(E_EXTENT, 0).b == nth_kind(E_STRLEN, 0).a, code + 5);
    require(nth_kind(E_SELECT, 1).a == Font::FontHDC, code + 6);
    require(index_kind(E_STRLEN, expected_strlen - 1) < index_kind(E_SELECT, 0) &&
            index_kind(E_SELECT, 0) < index_kind(E_EXTENT, 0) &&
            index_kind(E_EXTENT, 0) < index_kind(E_STOCK, 0) &&
            index_kind(E_STOCK, 0) < index_kind(E_SELECT, 1), code + 7);
    return 0;
}

static int width_behavior() {
    Font font;
    char text[] = "abc";
    HFONT handle = reinterpret_cast<HFONT>(0x3320);
    observed_stock = reinterpret_cast<HFONT>(0x3340);
    seed(font, handle, 0);
    reset_events();
    require(font.width(0, 3) == 0 && event_count == 0, 41);
    require(check_width(font, text, 2, 2, 1, 42) == 0, 50);
    require(check_width(font, text, 3, 3, 2, 51) == 0, 60);
    require(check_width(font, text, 8, 3, 2, 61) == 0, 70);
    require(check_width(font, text, static_cast<size_t>(0x80000000U), INT_MIN, 1, 71) == 0, 80);
    return 0;
}

static int private_width_and_line_break() {
    Font font;
    char text[] = "ab cd";
    observed_stock = reinterpret_cast<HFONT>(0x3350);
    seed(font, reinterpret_cast<HFONT>(0x3330), 0);
    reset_events();
    extent_width = 1;
    int direct = font.test_private_width(text, 2);
    require(direct == 1 && count_kind(E_STRLEN) == 1 && count_kind(E_EXTENT) == 1, 91);
    require(nth_kind(E_EXTENT, 0).value == 2 && nth_kind(E_EXTENT, 0).b == text, 92);
    require(count_kind(E_SELECT) == 2 && count_kind(E_STOCK) == 1 &&
            index_kind(E_SELECT, 0) < index_kind(E_EXTENT, 0) &&
            index_kind(E_EXTENT, 0) < index_kind(E_STOCK, 0) &&
            index_kind(E_STOCK, 0) < index_kind(E_SELECT, 1), 97);

    reset_events();
    font.test_private_width(text, static_cast<size_t>(0x80000000U));
    require(count_kind(E_STRLEN) == 1 && count_kind(E_EXTENT) == 1 &&
            nth_kind(E_EXTENT, 0).value == INT_MIN, 100);

    reset_events();
    BufferStrHeight = 55;
    extent_width = 2;
    int break_len = 3;
    LPSTR result = font.find_line_break_l(text, &break_len, 5);
    require(result == text + 3 && break_len == 0 && BufferStrHeight == 55, 93);
    require(count_kind(E_EXTENT) == 2 && count_kind(E_STRLEN) == 3, 94);
    require(count_kind(E_SELECT) == 4 && count_kind(E_STOCK) == 2, 98);
    require(index_kind(E_SELECT, 0) < index_kind(E_EXTENT, 0) &&
            index_kind(E_EXTENT, 0) < index_kind(E_STOCK, 0) &&
            index_kind(E_STOCK, 0) < index_kind(E_SELECT, 1) &&
            index_kind(E_SELECT, 1) < index_kind(E_SELECT, 2), 99);
    require(nth_kind(E_EXTENT, 0).b == text && nth_kind(E_EXTENT, 0).value == 2, 95);
    require(nth_kind(E_EXTENT, 1).b == text + 2 && nth_kind(E_EXTENT, 1).value == 3, 96);
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 2) return 99;
    failure_code = 0;
    if (same_text(argv[1], "cleanup")) { cleanup_public(); return failure_code; }
    if (same_text(argv[1], "reentrant")) { cleanup_reentrant(); return failure_code; }
    if (same_text(argv[1], "private-reentrant")) { cleanup_private_reentrant(); return failure_code; }
    if (same_text(argv[1], "private")) { cleanup_private_and_destructor(); return failure_code; }
    if (same_text(argv[1], "width")) { width_behavior(); return failure_code; }
    if (same_text(argv[1], "line")) { private_width_and_line_break(); return failure_code; }
    return 98;
}
"""


def program_source(bodies, damage=None) -> str:
    fixture = FIXTURE
    product = dict(bodies)
    if damage:
        key, before, after = damage
        assert before in product[key], f"mutant anchor missing in {key}: {before!r}"
        product[key] = product[key].replace(before, after, 1)
    inserts = {
        "/* PRODUCT_RELEASE */": product["release"],
        "/* PRODUCT_BOUNDED_PRIVATE */": product["bounded_private"],
        "/* PRODUCT_CLOSE */": product["close"],
        "/* PRODUCT_BOUNDED_PUBLIC */": product["bounded_public"],
        "/* PRODUCT_DESTRUCTOR */": product["destructor"],
        "/* PRODUCT_LINE_BREAK */": product["line_break"],
    }
    for marker, definition in inserts.items():
        assert fixture.count(marker) == 1
        fixture = fixture.replace(marker, definition)
    return fixture


def build_program(bodies, tmp_path: Path, compiler: str, name: str, damage=None) -> Path:
    fixture = program_source(bodies, damage)
    path = tmp_path / f"{name}.cpp"
    path.write_text(fixture, encoding="utf-8")
    executable = tmp_path / name
    subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                    str(path), "-o", str(executable)], check=True,
                   capture_output=True, text=True, timeout=30)
    return executable


def run(executable: Path, scenario: str) -> subprocess.CompletedProcess:
    return subprocess.run([str(executable), scenario], capture_output=True,
                          text=True, timeout=10)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_font_public_and_private_bodies(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    executable = build_program(product_bodies(), tmp_path, compiler, "font-good")
    for scenario in ("cleanup", "reentrant", "private", "private-reentrant", "width", "line"):
        result = run(executable, scenario)
        assert result.returncode == 0, f"{scenario}: {result.stdout}{result.stderr} ({result.returncode})"


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_font_behavioral_damage_mutants(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    bodies = product_bodies()
    mutants = (
        ("close", "DeleteObject(font_obj_);", "font_obj_ = 0; DeleteObject(font_obj_);"),
        ("release", "RemoveFontResourceA(fot_file_name_);", "fot_file_name_ = 0; RemoveFontResourceA(fot_file_name_);"),
        ("release", "if (fot_file_name_) {\n        free(fot_file_name_);\n      }", "free(fot_file_name_);"),
        ("bounded_private", "SelectObject(FontHDC, GetStockObject(SYSTEM_FONT));", "GetTextExtentPoint32A(FontHDC, input, len, &size); SelectObject(FontHDC, GetStockObject(SYSTEM_FONT));"),
        ("bounded_private", "int len = FONT_MIN(static_cast<int>(max_len),\n                       static_cast<int>(strlen(input)));", "int len = static_cast<int>(strlen(input)); if (len > static_cast<int>(max_len)) len = static_cast<int>(max_len);"),
        ("bounded_private", "int len = FONT_MIN(static_cast<int>(max_len),\n                       static_cast<int>(strlen(input)));", "int len = FONT_MIN(static_cast<size_t>(max_len),\n                       static_cast<size_t>(strlen(input)));"),
    )
    for index, mutant in enumerate(mutants):
        executable = build_program(bodies, tmp_path, compiler, f"font-mutant-{index}", mutant)
        scenario = "reentrant" if index == 0 else (
            "private-reentrant" if index in (1, 2) else ("line" if index >= 3 else "cleanup"))
        result = run(executable, scenario)
        assert result.returncode != 0, f"behavioral control missed mutant {index}: {result.stdout}{result.stderr}"


def test_source_identity_claim_and_helper_body_continuity():
    bodies = product_bodies()
    source = SOURCE.read_text(encoding="utf-8")
    header = HEADER.read_text(encoding="utf-8")

    assert compact_body(bodies["release"]) == compact_body(bodies["close"])
    assert compact_body(bodies["bounded_private"]) == compact_body(bodies["bounded_public"])
    assert "release_resources();" in bodies["destructor"]
    assert "release_resources();" in extract(source, "int Font::init(LPCSTR file, LPCSTR font_name, int height, int style)")
    assert source.count("measure_bounded_width(") == 2
    assert "measure_bounded_width(search_str, len)" in bodies["line_break"]
    assert "measure_bounded_width(search_str, space - search_str)" in bodies["line_break"]

    for address, symbol in (
        ("0x00619230", "?close@Font@@QAEXXZ"),
        ("0x006192F0", "?width@Font@@QAEHPADH@Z"),
        ("0x00619370", "?find_line_break_l@Font@@QAEHPADPAHH@Z"),
        ("0x00618EE0", "??1Font@@QAE@XZ"),
        ("0x006190D0", "?init@Font@@QAEHPADPADHH@Z"),
        ("0x00618F40", "?init@Font@@QAEHPADHH@Z"),
    ):
        assert address in source and symbol in source, (address, symbol)
    assert "void Font::close()" in bodies["close"]
    assert "int Font::width(LPSTR input, size_t max_len)" in bodies["bounded_public"]
    assert "MEASURED void close()" not in header
    assert "MEASURED int width(LPSTR input, size_t max_len)" not in header
    assert "ORIGINAL:" not in bodies["release"] and "ORIGINAL:" not in bodies["bounded_private"]
    release_pos = header.index("void release_resources()")
    bounded_pos = header.index("int measure_bounded_width(LPSTR input, size_t max_len)")
    assert header.rfind("private:", 0, release_pos) > header.rfind("public:", 0, release_pos)
    assert header.rfind("private:", 0, bounded_pos) > header.rfind("public:", 0, bounded_pos)
    for address in ("0x00619230", "0x006192F0"):
        marker = re.search(rf"^// ORIGINAL: {address} .*BYTE_EXACT$", source, re.MULTILINE)
        assert marker is not None
    width_claim = source.index("// ORIGINAL: 0x006192F0")
    width_definition = source.index("int Font::width(LPSTR input, size_t max_len)")
    assert "// LEVER:" in source[width_claim:width_definition]

    from match_recovery.source import read_file, read_text
    old_cpp = subprocess.check_output(["git", "show", BASE + ":src/jackal/font.cpp"], cwd=ROOT).decode()
    old_rows = {row.address: row for row in read_text(old_cpp, SOURCE)}
    current_rows = {row.address: row for row in read_file(SOURCE)}
    for address in (0x618EE0, 0x618F40, 0x6190D0, 0x619230, 0x6192F0, 0x619370):
        old, current = old_rows[address], current_rows[address]
        for field in ("address", "name", "symbol", "image_spans", "byte_exact", "semantic",
                      "levers", "ruled_out", "deferred", "unrecoverable"):
            assert getattr(current, field) == getattr(old, field), (hex(address), field)


def test_fixture_carries_pinned_32bit_layout_contract():
    # The lease-guarded VC6 test compiles this conditional x86 contract.
    header = HEADER.read_text(encoding="utf-8")
    names = ("int unk_1_;", "BOOL is_fot_set_;", "HFONT font_obj_;", "int line_height_;",
             "int height_;", "int internal_leading_;", "int ascent_;", "int descent_;",
             "int pad_;", "LPSTR fot_file_name_;")
    positions = [header.index(name) for name in names]
    assert positions == sorted(positions)
    assert "static_assert(sizeof(Font) == 0x28" in header
    assert "#if defined(_M_IX86)" in FIXTURE
    assert "font_size_check[(sizeof(Font) == 0x28) ? 1 : -1]" in FIXTURE


def test_vc6_x86_actual_source_behavior_and_damage_controls(tmp_path):
    """Run extracted product bodies as 32-bit code in both configured profiles."""
    if not os.environ.get("MATCH_RECOVERY_RESOURCE_LEASE"):
        pytest.skip("native Font controls require an existing coordinator compiler lease")
    from match_recovery.measurement import Measurement
    from match_recovery.processes import run_bounded
    from recovery.environment.compile import compile_object
    from recovery.environment.wine import VC6_ROOT, environment

    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine32") is None:
        pytest.skip("VC6 and Wine32 are required under the coordinator lease")
    settings = Measurement.read(ROOT)
    wrapper = ROOT / "recovery/environment/wine.py"
    command = (sys.executable, str(wrapper), "--tool=cl", "/nologo")

    def build_native(name, flags, damage=None):
        source = tmp_path / (name + ".cpp")
        obj = tmp_path / (name + ".obj")
        exe = tmp_path / (name + ".exe")
        source.write_text(program_source(product_bodies(), damage), encoding="utf-8")
        obj.write_bytes(compile_object(source, command, tuple(flags) + settings.layout_flags))
        linked = run_bounded([sys.executable, str(wrapper), "--tool=link", "/nologo",
                              "/subsystem:console", "/incremental:no", "/out:" + str(exe),
                              str(obj), "kernel32.lib"], cwd=tmp_path, timeout=60)
        assert linked.returncode == 0, (linked.stdout + linked.stderr).decode(errors="replace")
        return exe

    def run_native(exe, scenario):
        with tempfile.TemporaryFile() as diagnostics:
            result = subprocess.run(["wine32", str(exe), scenario], env=environment(),
                                    stdout=diagnostics, stderr=diagnostics, timeout=30)
            diagnostics.seek(0)
            output = diagnostics.read().decode(errors="replace")
        return result.returncode, output

    bodies = product_bodies()
    mutants = (
        ("close", "DeleteObject(font_obj_);", "font_obj_ = 0; DeleteObject(font_obj_);", "reentrant"),
        ("release", "RemoveFontResourceA(fot_file_name_);", "fot_file_name_ = 0; RemoveFontResourceA(fot_file_name_);", "private-reentrant"),
        ("release", "if (fot_file_name_) {\n        free(fot_file_name_);\n      }", "free(fot_file_name_);", "private-reentrant"),
        ("bounded_private", "SelectObject(FontHDC, GetStockObject(SYSTEM_FONT));",
         "GetTextExtentPoint32A(FontHDC, input, len, &size); SelectObject(FontHDC, GetStockObject(SYSTEM_FONT));", "line"),
        ("bounded_private", "int len = FONT_MIN(static_cast<int>(max_len),\n                       static_cast<int>(strlen(input)));",
         "int len = static_cast<int>(strlen(input)); if (len > static_cast<int>(max_len)) len = static_cast<int>(max_len);", "line"),
        ("bounded_private", "int len = FONT_MIN(static_cast<int>(max_len),\n                       static_cast<int>(strlen(input)));",
         "int len = FONT_MIN(static_cast<size_t>(max_len),\n                       static_cast<size_t>(strlen(input)));", "line"),
    )
    for profile_index, flags in enumerate(settings.flag_sets):
        good = build_native("font-good-" + str(profile_index), flags)
        for scenario in ("cleanup", "reentrant", "private", "private-reentrant", "width", "line"):
            code, output = run_native(good, scenario)
            assert code == 0, f"profile {profile_index} {scenario}: {output} ({code})"
        for mutant_index, (key, before, after, scenario) in enumerate(mutants):
            damaged = build_native("font-bad-%d-%d" % (profile_index, mutant_index), flags,
                                   (key, before, after))
            code, output = run_native(damaged, scenario)
            assert code != 0, f"profile {profile_index} mutant {mutant_index} survived: {output}"
