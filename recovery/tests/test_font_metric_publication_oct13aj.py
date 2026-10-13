"""Actual-source controls for Font metric publication and signed line advance.

The host fixtures compile literal mapped bodies with narrow Win32 API spies.
They are behavioral controls, not native ABI or game-runtime proof.
"""
from dataclasses import fields
from pathlib import Path
import re
import shutil
import subprocess

import pytest

from match_recovery.source import read_file, read_text
from match_recovery.tasks import TaskStore
from test_font_internal_sharing_oct12g import extract


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/jackal/font.cpp"
BASE = "1a5e6a4b274bd8b0dc44be3df7ae8128755f1b04"
TASK = "font-metric-publication-owner-oct13aj"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def _tokens(source: str) -> list[str]:
    token = re.compile(
        r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|'
        r'//[^\n]*|/\*.*?\*/|[A-Za-z_]\w*|0[xX][0-9A-Fa-f]+|\d+|'
        r'::|->|\+\+|--|&&|\|\||==|!=|<=|>=|<<|>>|'
        r'\+=|-=|\*=|/=|%=|&=|\|=|\^=|##|.', re.S)
    result = []
    for match in token.finditer(source):
        value = match.group(0)
        if value.startswith("//") or value.startswith("/*") or value.isspace():
            continue
        result.append(value)
    return result


def _history(row):
    return tuple((item.kind, item.key, item.prose,
                  tuple(line for _, line in item.lines)) for item in row.origin)


def _bodies(source=None):
    text = SOURCE.read_text(encoding="utf-8") if source is None else source
    return {
        "init": extract(text, "int Font::init(LPCSTR font_name, int height, int style)"),
        "line_advance": extract(text, "int Font::line_advance() const"),
    }


def _assert_exact_source_delta(current: str, baseline: str) -> None:
    pinned_call_note = """// Pinned-call correction: 0x006190A0 pushes SYSTEM_FONT as argument setup;
    // GetStockObject is actually called at 0x006190AB and SelectObject at
    // 0x006190B9. The image publishes all five metrics before either call.
"""
    assert current.count(pinned_call_note) == 1
    current_init = extract(current, "int Font::init(LPCSTR font_name, int height, int style)")
    old_init = extract(baseline, "int Font::init(LPCSTR font_name, int height, int style)")
    old_tail = """    int leading = tm.tmInternalLeading;
    line_height_ = tm.tmHeight + tm.tmExternalLeading;
    ascent_ = tm.tmAscent;
    height_ = ascent_ - leading;
    SelectObject(FontHDC, GetStockObject(SYSTEM_FONT));
    internal_leading_ = leading;
    descent_ = tm.tmDescent;
    return 0;
    SelectObject(FontHDC, GetStockObject(SYSTEM_FONT));
    return 0;"""
    new_tail = """    int leading = tm.tmInternalLeading;
    line_height_ = tm.tmHeight + tm.tmExternalLeading;
    ascent_ = tm.tmAscent;
    height_ = ascent_ - leading;
    internal_leading_ = leading;
    descent_ = tm.tmDescent;
    SelectObject(FontHDC, GetStockObject(SYSTEM_FONT));
    return 0;"""
    assert old_init.count(old_tail) == 1
    assert current_init.count(new_tail) == 1
    restored_init = current_init.replace(new_tail, old_tail, 1)
    current = current.replace(current_init, restored_init, 1)

    current_line = extract(current, "int Font::line_advance() const")
    old_line = extract(baseline, "int Font::line_advance() const")
    assert current.count(current_line) == 1 and baseline.count(old_line) == 1
    current = current.replace(current_line, old_line, 1)
    assert _tokens(current) == _tokens(baseline), "only the two authorized body deltas"
    current_comments = current.replace(pinned_call_note, "", 1)
    comments = lambda text: re.findall(r"//[^\n]*|/\*.*?\*/", text, re.S)
    assert comments(current_comments) == comments(baseline), (
        "only the pinned-call evidence comment may be added")


INIT_FIXTURE = r"""
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef const char *LPCSTR;
typedef char *LPSTR;
typedef int BOOL;
typedef void *HDC;
typedef void *HFONT;
typedef int32_t LONG;
typedef unsigned char BYTE;
typedef char TCHAR;
enum { SYSTEM_FONT = 13 };
struct LOGFONT {
    LONG lfHeight, lfWidth, lfEscapement, lfOrientation, lfWeight;
    BYTE lfItalic, lfUnderline, lfStrikeOut, lfCharSet, lfOutPrecision;
    BYTE lfClipPrecision, lfQuality, lfPitchAndFamily;
    TCHAR lfFaceName[32];
};
struct TEXTMETRIC {
    LONG tmHeight, tmAscent, tmDescent, tmInternalLeading, tmExternalLeading;
    LONG tmAveCharWidth, tmMaxCharWidth, tmWeight, tmOverhang;
    BYTE tmDigitizedAspectX, tmDigitizedAspectY;
    TCHAR tmFirstChar, tmLastChar, tmDefaultChar, tmBreakChar;
    BYTE tmItalic, tmUnderlined, tmStruckOut, tmPitchAndFamily, tmCharSet;
};
static_assert(sizeof(LOGFONT) == 60 && offsetof(LOGFONT, lfFaceName) == 28,
              "Win32 LOGFONT wire shape");
static_assert(sizeof(TEXTMETRIC) == 48 && offsetof(TEXTMETRIC, tmDescent) == 8 &&
              offsetof(TEXTMETRIC, tmInternalLeading) == 12 &&
              offsetof(TEXTMETRIC, tmExternalLeading) == 16,
              "Win32 TEXTMETRIC32 shape");
class Font {
public:
    static HDC FontHDC;
    int unk_1_, is_fot_set_;
    HFONT font_obj_;
    int line_height_, height_, internal_leading_, ascent_, descent_, pad_;
    LPSTR fot_file_name_;
    int init(LPCSTR font_name, int height, int style);
};
HDC Font::FontHDC = reinterpret_cast<HDC>(0x1110);

static const int FAIL_CODE = 98;
static int failure_code;
static int create_fails;
static int deletes, removes, frees;
static int metric_reads, callback_stock, callback_restore;
static int mutate_filename_in_remove;
static LPSTR replacement_filename;
static LPSTR removed_filename;
static void *freed_filename;
static LOGFONT observed_logfont;
static TEXTMETRIC *captured_metrics;
static Font *owner;
static HFONT created_font = reinterpret_cast<HFONT>(0x2220);
static HFONT stock_font = reinterpret_cast<HFONT>(0x3330);

static void require(bool okay, const char *name) {
    if (!okay && !failure_code) {
        fprintf(stderr, "FAIL font-metric-publication:%s\n", name);
        failure_code = FAIL_CODE;
    }
}
static int all_metrics(int line, int asc, int height, int leading, int descent) {
    return owner && owner->line_height_ == line && owner->ascent_ == asc &&
           owner->height_ == height && owner->internal_leading_ == leading &&
           owner->descent_ == descent;
}
static BOOL DeleteObject(HFONT) { ++deletes; return 0; }
static BOOL RemoveFontResourceA(LPSTR filename) {
    ++removes;
    removed_filename = filename;
    if (mutate_filename_in_remove) {
        mutate_filename_in_remove = 0;
        owner->fot_file_name_ = replacement_filename;
    }
    return 0;
}
static void free(void *filename) { ++frees; freed_filename = filename; }
static HFONT CreateFontIndirectA(const LOGFONT *font) {
    observed_logfont = *font;
    return create_fails ? 0 : created_font;
}
static BOOL GetTextMetricsA(HDC, TEXTMETRIC *out) {
    ++metric_reads;
    captured_metrics = out;
    out->tmHeight = 20;
    out->tmExternalLeading = 3;
    out->tmInternalLeading = 5;
    out->tmAscent = 15;
    out->tmDescent = 4;
    return 0; /* Original ignores this API result. */
}
static HFONT GetStockObject(int object) {
    require(object == SYSTEM_FONT, "stock-object-argument");
    require(all_metrics(23, 15, 10, 5, 4), "GetStockObject-saw-all-five");
    ++callback_stock;
    if (captured_metrics) {
        captured_metrics->tmHeight = 101;
        captured_metrics->tmExternalLeading = 102;
        captured_metrics->tmInternalLeading = 103;
        captured_metrics->tmAscent = 104;
        captured_metrics->tmDescent = 105;
    }
    return stock_font;
}
static HFONT SelectObject(HDC dc, HFONT object) {
    require(dc == Font::FontHDC, "select-dc");
    if (object == stock_font) {
        require(all_metrics(23, 15, 10, 5, 4), "SelectObject-saw-all-five");
        ++callback_restore;
    }
    return 0; /* Original ignores this API result. */
}

/* PRODUCT_INIT */

static int run_success() {
    Font f = {};
    owner = &f;
    f.is_fot_set_ = 1;
    int result = f.init("Metric Face", 22, 7);
    require(result == 0, "success-result");
    require(strcmp(observed_logfont.lfFaceName, "Metric Face") == 0, "LOGFONT-name");
    require(observed_logfont.lfHeight == -22 && observed_logfont.lfWeight == 700 &&
            observed_logfont.lfItalic == 1 && observed_logfont.lfUnderline == 1,
            "LOGFONT-fields");
    require(metric_reads == 1 && callback_stock == 1 && callback_restore == 1,
            "callback-counts");
    require(all_metrics(23, 15, 10, 5, 4), "no-post-callback-metric-rereads");
    require(f.font_obj_ == created_font && deletes == 0 && removes == 0 && frees == 0,
            "resource-path");
    return 0;
}
static int run_null() {
    Font f = {};
    owner = &f;
    f.line_height_ = 71; f.height_ = 72; f.ascent_ = 73;
    f.internal_leading_ = 74; f.descent_ = 75; f.font_obj_ = created_font;
    int result = f.init(0, 20, 0);
    require(result == 3 && f.line_height_ == 71 && f.height_ == 72 &&
            f.ascent_ == 73 && f.internal_leading_ == 74 && f.descent_ == 75,
            "null-name-early-return");
    require(deletes == 0 && metric_reads == 0 && callback_stock == 0,
            "null-name-no-side-effects");
    return 0;
}
static int run_create_failure() {
    Font f = {};
    owner = &f;
    f.is_fot_set_ = 1;
    f.font_obj_ = created_font;
    f.line_height_ = 71; f.height_ = 72; f.ascent_ = 73;
    f.internal_leading_ = 74; f.descent_ = 75;
    create_fails = 1;
    int result = f.init("No Font", 20, 0);
    require(result == 13 && deletes == 1 && f.font_obj_ == 0,
            "create-failure-return-and-delete");
    require(f.line_height_ == 0 && f.height_ == 0 && f.ascent_ == 0 &&
            f.internal_leading_ == 74 && f.descent_ == 0,
            "create-failure-state");
    require(metric_reads == 0 && callback_stock == 0 && callback_restore == 0,
            "create-failure-no-metric-callbacks");
    return 0;
}
static int run_teardown_reread() {
    Font f = {};
    char old_name[] = "old.fot";
    char replacement[] = "replacement.fot";
    owner = &f;
    f.is_fot_set_ = 0;
    f.font_obj_ = created_font;
    f.fot_file_name_ = old_name;
    replacement_filename = replacement;
    mutate_filename_in_remove = 1;
    int result = f.init("Metric Face", 22, 7);
    require(result == 0 && deletes == 1 && removes == 1 && frees == 1,
            "teardown-resource-counts");
    require(removed_filename == old_name && freed_filename == replacement,
            "teardown-live-filename-reread");
    require(f.fot_file_name_ == 0 && f.font_obj_ == created_font,
            "teardown-final-state");
    require(all_metrics(23, 15, 10, 5, 4) && callback_stock == 1 &&
            callback_restore == 1, "teardown-success-publication");
    return 0;
}
int main(int argc, char **argv) {
    if (argc != 2) return 97;
    failure_code = 0;
    if (!strcmp(argv[1], "success")) run_success();
    else if (!strcmp(argv[1], "null")) run_null();
    else if (!strcmp(argv[1], "create-failure")) run_create_failure();
    else if (!strcmp(argv[1], "teardown")) run_teardown_reread();
    else return 96;
    return failure_code;
}
"""


LINE_FIXTURE = r"""
#include <stdint.h>
#include <limits.h>
#include <stdio.h>
class Font {
public:
    int unk_1_, line_height_, height_;
    int line_advance() const;
};
/* PRODUCT_LINE_ADVANCE */
static int failure;
static void require(bool okay, const char *name) {
    if (!okay && !failure) { fprintf(stderr, "FAIL line-advance:%s\n", name); failure = 98; }
}
static int expected_sum(int height, int adjustment) {
    return static_cast<int32_t>(static_cast<uint32_t>(height) +
                                static_cast<uint32_t>(adjustment));
}
int main() {
    Font f;
    f.line_height_ = 19; f.height_ = 8; f.unk_1_ = -1;
    require(f.line_advance() == 19, "negative-selects-natural-line-height");
    f.unk_1_ = INT_MIN;
    require(f.line_advance() == 19, "signed-negative-boundary");
    f.unk_1_ = 0;
    require(f.line_advance() == 8, "zero-selects-natural-height");
    f.unk_1_ = 3;
    require(f.line_advance() == 11, "positive-adjustment");
    f.height_ = INT_MAX; f.unk_1_ = 1;
    require(f.line_advance() == expected_sum(INT_MAX, 1), "positive-wrap");
    f.height_ = INT_MIN; f.unk_1_ = INT_MAX;
    require(f.line_advance() == expected_sum(INT_MIN, INT_MAX), "boundary-wrap");
    return failure;
}
"""


def _compile(compiler: str, source: str, path: Path, flags=()):
    path.write_text(source, encoding="utf-8")
    executable = path.with_suffix("")
    result = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", *flags, str(path), "-o", str(executable)],
        text=True, capture_output=True, timeout=30)
    assert result.returncode == 0, result.stdout + result.stderr
    return executable


def _init_program(body: str) -> str:
    assert INIT_FIXTURE.count("/* PRODUCT_INIT */") == 1
    return INIT_FIXTURE.replace("/* PRODUCT_INIT */", body)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_initializer_publishes_metrics_before_both_callbacks(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    body = _bodies()["init"]
    executable = _compile(compiler, _init_program(body), tmp_path / "font-init.cpp")
    for scenario in ("success", "null", "create-failure", "teardown"):
        result = subprocess.run([str(executable), scenario], text=True,
                                 capture_output=True, timeout=10)
        assert result.returncode == 0, (scenario, result.stdout, result.stderr, result.returncode)

    # This mutation still compiles. The callbacks must reject the old publication order.
    before = """    internal_leading_ = leading;
    descent_ = tm.tmDescent;
    SelectObject(FontHDC, GetStockObject(SYSTEM_FONT));"""
    after = """    SelectObject(FontHDC, GetStockObject(SYSTEM_FONT));
    internal_leading_ = leading;
    descent_ = tm.tmDescent;"""
    assert body.count(before) == 1
    mutant = _compile(compiler, _init_program(body.replace(before, after, 1)),
                      tmp_path / "font-init-wrong-order.cpp")
    damaged = subprocess.run([str(mutant), "success"], text=True,
                              capture_output=True, timeout=10)
    assert damaged.returncode == 98
    assert "FAIL font-metric-publication:GetStockObject-saw-all-five" in damaged.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_line_advance_signed_selection_and_defined_wrap(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    body = _bodies()["line_advance"]
    source = LINE_FIXTURE.replace("/* PRODUCT_LINE_ADVANCE */", body)
    executable = _compile(compiler, source, tmp_path / "font-line-advance.cpp",
                          ("-fsanitize=undefined", "-fno-sanitize-recover=all"))
    result = subprocess.run([str(executable)], text=True, capture_output=True, timeout=10)
    assert result.returncode == 0, (result.stdout, result.stderr, result.returncode)


def test_task_base_source_recipe_and_all_font_identity_history_are_preserved():
    task = TaskStore(ROOT).read(TASK)
    assert task.base_revision == BASE
    assert all(subject.image_sha256 == PIN and subject.address_space == "pe:va"
               for subject in task.subjects)
    old_bytes = subprocess.check_output(
        ["git", "show", f"{BASE}:src/jackal/font.cpp"], cwd=ROOT, timeout=30)
    old_source = old_bytes.decode("utf-8")
    current = SOURCE.read_text(encoding="utf-8")
    _assert_exact_source_delta(current, old_source)

    old_rows = {row.address: row for row in read_text(old_source, SOURCE)}
    current_rows = {row.address: row for row in read_file(SOURCE)}
    assert old_rows.keys() == current_rows.keys()
    for address, previous in old_rows.items():
        actual = current_rows[address]
        for field in fields(previous):
            if field.name in {"line", "region", "origin"}:
                continue
            assert getattr(actual, field.name) == getattr(previous, field.name), (
                hex(address), field.name)
        assert _history(actual) == _history(previous), (hex(address), "annotation history")
