"""Actual-source controls for SetupWin::draw_item's two-stage text lookup."""
from pathlib import Path
import shutil
import subprocess

import pytest
from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]
PRODUCT = ROOT / "src" / "setupwin.cpp"
SIGNATURE = "int SetupWin::draw_item(int position, int selected)"


def extract_body(source):
    start = source.find(SIGNATURE)
    assert start >= 0, "mapped draw_item definition missing"
    opening = source.find("{", start)
    depth = 0
    state = "code"
    i = opening
    while i < len(source):
        ch = source[i]
        nxt = source[i + 1] if i + 1 < len(source) else ""
        if state == "code":
            if ch == "/" and nxt == "/":
                state = "line"; i += 2; continue
            if ch == "/" and nxt == "*":
                state = "block"; i += 2; continue
            if ch == '"': state = "string"
            elif ch == "'": state = "char"
            elif ch == "{": depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0: return source[start:i + 1]
        elif state == "line":
            if ch == "\n": state = "code"
        elif state == "block":
            if ch == "*" and nxt == "/": state = "code"; i += 2; continue
        else:
            if ch == "\\": i += 2; continue
            if (state == "string" and ch == '"') or (state == "char" and ch == "'"):
                state = "code"
        i += 1
    raise AssertionError("unterminated draw_item body")


def actual_body():
    rows = [row for row in read_file(PRODUCT) if row.address == 0x004AE4D0]
    assert len(rows) == 1 and rows[0].state.value == "implemented"
    return extract_body(PRODUCT.read_text(encoding="utf-8"))


FIXTURE = r"""
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
static void fail(const char *message) {
    std::fprintf(stderr, "SEMANTIC97: %s\n", message);
    std::exit(97);
}
struct RECT { int left, top, right, bottom; };
struct Buffer {
    struct { struct { int biWidth, biHeight; } bmiHeader; } dib_;
    int writes, upper_initialized;
    char written[80];
    Buffer() : writes(0), upper_initialized(1) {
        written[0] = 0;
        dib_.bmiHeader.biWidth = 8;
        dib_.bmiHeader.biHeight = -4;
    }
    void set_clip(RECT *) {}
    void set_text_color(int, int, int, int) {}
    void copy(Buffer *, int, int, int, int, int, int) {}
    void write_cent_l(char *text, RECT *, int length) {
        if (length < 0 || length >= (int)sizeof(written)) fail("bad write length");
        ++writes;
        std::memcpy(written, text, (size_t)length);
        written[length] = 0;
    }
    void restore_clip() {}
    int get_pixel(int, int) { return 0; }
    void draw(Buffer *, int, int, int, int, int) {}
};
struct GraphicWin : Buffer {
    void soft_update(int, int, int, int) {}
};
struct Payload { char *text; };
struct StringStructEntry {
    StringStructEntry *next;
    Payload *payload;
    Payload *string_payload() { return payload; }
};
struct StringList {
    int entry_count_;
    StringStructEntry *head_;
    StringStructEntry *current_;
    int current_position_;
};
struct Spot {
    void get_rect(int, int, RECT *r) {
        r->left = 10; r->top = 20; r->right = 100; r->bottom = 40;
    }
};
struct SetupWin : GraphicWin {
    Buffer buffer_;
    Buffer buffer2_;
    Spot spot_;
    unsigned int row_count_;
    StringList *item_texts_;
    Buffer *overlay_buffers_[3];
    int overlay_x_[3], overlay_y_[3];
    SetupWin() : row_count_(4), item_texts_(0) {
        overlay_buffers_[0] = overlay_buffers_[1] = overlay_buffers_[2] = 0;
        overlay_x_[0] = overlay_y_[0] = 0;
    }
    int draw_item(int position, int selected);
};
static int upper_calls, strchr_calls, null_upper_calls, null_strchr_calls;
static void (*upper_hook)();
static char *safe_upper(char *text) {
    ++upper_calls;
    if (text == 0) { ++null_upper_calls; }
    else {
        for (char *p = text; *p; ++p)
            *p = (char)std::toupper((unsigned char)*p);
    }
    if (upper_hook != 0) upper_hook();
    return text;
}
static char *safe_strchr(char *text, int ch) {
    ++strchr_calls;
    if (text == 0) { ++null_strchr_calls; return 0; }
    return std::strchr(text, ch);
}
#define CharUpperA safe_upper
#define strchr safe_strchr
static GraphicWin *opening_window() { return 0; }
static int WinScreenWidth = 640;

// ACTUAL_BODY_INSERTION_POINT

static SetupWin *hook_setup;
static StringList *hook_replacement;
static void replace_lookup_after_upper() {
    hook_setup->item_texts_ = hook_replacement;
}
static int reload_case() {
    char original[] = "old|label (tail)";
    char fresh[] = "Fresh label";
    Payload old_payload = { original }, fresh_payload = { fresh };
    StringStructEntry old_entry = { 0, &old_payload };
    StringStructEntry replacement_head = { 0, 0 };
    StringStructEntry replacement_current = { 0, &fresh_payload };
    StringList old_list = { 1, &old_entry, 0, -1 };
    StringList fresh_list = { 1, &replacement_head, &replacement_current, 73 };
    SetupWin setup;
    setup.item_texts_ = &old_list;
    hook_setup = &setup;
    hook_replacement = &fresh_list;
    upper_hook = replace_lookup_after_upper;
    upper_calls = strchr_calls = null_upper_calls = null_strchr_calls = 0;
    setup.draw_item(0, 0);
    upper_hook = 0;
    if (old_list.current_ != &old_entry || old_list.current_position_ != 0)
        fail("first owner cursor was not updated before string calls");
    if (setup.writes != 1 || std::strcmp(setup.written, "Fresh label") != 0)
        fail("second lookup did not reload owner/cursor/payload/text");
    if (upper_calls != 1 || strchr_calls != 2 ||
        std::strcmp(original, "OLD LABEL") != 0)
        fail("first text transform order changed");
    return 0;
}
static int traversal_case() {
    char zero[] = "zero", one[] = "one";
    Payload p0 = { zero }, p1 = { one };
    StringStructEntry e1 = { 0, &p1 }, e0 = { &e1, &p0 };
    StringList list = { 2, &e0, 0, -1 };
    SetupWin setup;
    setup.item_texts_ = &list;
    setup.draw_item(1, 0);
    if (list.current_ != &e1 || list.current_position_ != 1 ||
        setup.writes != 1 || std::strcmp(setup.written, "ONE") != 0)
        fail("positive position traversal changed");
    return 0;
}
static int stale_cursor_case() {
    char head_text[] = "head", stale_text[] = "stale";
    Payload phead = { head_text }, pstale = { stale_text };
    StringStructEntry stale = { 0, &pstale }, head = { 0, &phead };
    StringList list = { 1, &head, &stale, 91 };
    SetupWin setup;
    setup.item_texts_ = &list;
    setup.draw_item(2, 0);
    if (list.current_ != &stale || list.current_position_ != 91 ||
        setup.writes != 1 || std::strcmp(setup.written, "STALE") != 0)
        fail("out-of-list position did not retain stale cursor");
    return 0;
}
static int null_admission_case(int null_head) {
    Payload payload = { 0 };
    StringStructEntry entry = { 0, &payload };
    StringList list = { 1, null_head ? 0 : &entry, &entry, 66 };
    SetupWin setup;
    setup.item_texts_ = &list;
    upper_calls = strchr_calls = null_upper_calls = null_strchr_calls = 0;
    setup.draw_item(0, 0);
    if (upper_calls != 1 || strchr_calls != 2 ||
        null_upper_calls != 1 || null_strchr_calls != 2 || setup.writes != 0)
        fail("null label did not reach the original string-call probes");
    return 0;
}
static int preceding_byte_case() {
    char storage[] = { 'Q', '(', 'A', 0 };
    char *label = storage + 1;
    Payload payload = { label };
    StringStructEntry entry = { 0, &payload };
    StringList list = { 1, &entry, &entry, 0 };
    SetupWin setup;
    setup.item_texts_ = &list;
    upper_hook = 0;
    upper_calls = strchr_calls = null_upper_calls = null_strchr_calls = 0;
    setup.draw_item(0, 0);
    if (storage[0] != 0)
        fail("parenthesis-at-start did not truncate preceding byte");
    if (setup.writes != 1 || std::strcmp(setup.written, "(A") != 0)
        fail("parenthesis-at-start output changed");
    return 0;
}
int main() {
    reload_case();
    traversal_case();
    stale_cursor_case();
    null_admission_case(1);
    null_admission_case(0);
    preceding_byte_case();
    std::puts("SetupWin item text owner control: PASS");
    return 0;
}
"""


def run_host(tmp_path, compiler, body):
    source = tmp_path / "setupwin-item-text.cpp"
    binary = tmp_path / "setupwin-item-text"
    source.write_text(FIXTURE.replace("// ACTUAL_BODY_INSERTION_POINT", body),
                      encoding="utf-8")
    built = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                            str(source), "-o", str(binary)], check=False,
                           capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_item_text_two_stage_lookup_and_quirks(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    result = run_host(tmp_path, compiler, actual_body())
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("before,after,diagnostic", [
    ("CharUpperA(name);", "if (name != 0) CharUpperA(name);",
     "null label did not reach the original string-call probes"),
    ("if (paren != 0) paren[-1] = 0;",
     "if (paren != 0 && paren > name) paren[-1] = 0;",
     "parenthesis-at-start did not truncate preceding byte"),
])
def test_stale_lookup_and_added_guard_mutants_fail_semantically(
        tmp_path, compiler_name, before, after, diagnostic):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    body = actual_body()
    assert before in body, before
    result = run_host(tmp_path, compiler, body.replace(before, after, 1))
    assert result.returncode == 97, (result.returncode, result.stdout, result.stderr)
    assert ("SEMANTIC97: " + diagnostic) in result.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_cached_first_name_mutant_fails_semantically(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    body = actual_body()
    second_lookup = ("name = item_texts_->head_ != 0\n"
                     "            ? item_texts_->current_->string_payload()->text : 0;")
    assert second_lookup in body
    assert "CharUpperA(name);" in body
    body = body.replace("CharUpperA(name);",
                        "char *const stale_name = name;\n        CharUpperA(name);", 1)
    second_start = body.rfind(second_lookup)
    assert second_start >= 0
    body = body[:second_start] + "name = stale_name;" + body[second_start + len(second_lookup):]
    result = run_host(tmp_path, compiler, body)
    assert result.returncode == 97, (result.returncode, result.stdout, result.stderr)
    assert "SEMANTIC97: second lookup did not reload owner/cursor/payload/text" in result.stderr
