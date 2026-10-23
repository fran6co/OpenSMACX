"""Actual-source controls for BaseWin::draw_info's original clip lifetime."""

from __future__ import annotations

from pathlib import Path
import re
import shutil
import subprocess

import pytest

from match_recovery.source import Mode, read_file
from match_recovery.includes import COMMENTS_OR_STRINGS


ROOT = Path(__file__).resolve().parents[2]
ADDRESS = 0x00415B20
SIGNATURE = "void BaseWin::draw_info(int update_mode)"
SOURCE_SHA256 = "e19fddef03e8a5e9d73ee13d6ab04c3b1086fd2a8fb04b801320ef6bc21b1dfc"


def _function_body(region: str) -> str:
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: re.sub(r"[^\n]", " ", match[0]), region
    )
    hits = [match.start() for match in re.finditer(re.escape(SIGNATURE), masked)]
    assert len(hits) == 1, f"expected one mapped body, found {len(hits)}"
    start = hits[0]
    opening = masked.index("{", start)
    depth = 0
    for end in range(opening, len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                return region[start:end + 1]
    raise AssertionError("unterminated draw_info body")


def actual_body() -> str:
    rows = [row for row in read_file(ROOT / "src/basewin.cpp")
            if row.address == ADDRESS]
    assert len(rows) == 1, "missing or duplicate native draw_info owner"
    row = rows[0]
    assert row.mode == Mode.BODY
    assert row.name == "?draw_info@BaseWin@@QAEXH@Z"
    assert row.image_spans == ((0x00415B20, 0x00415DCE),)
    assert row.region and SIGNATURE in row.region
    body = _function_body(row.region)
    assert "BODY GOES HERE." not in body
    assert "(void)update_mode;" not in body
    return body


FIXTURE = r"""
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <vector>

struct RECT { int left, top, right, bottom; };

static bool same_rect(const RECT &a, const RECT &b) {
    return a.left == b.left && a.top == b.top &&
           a.right == b.right && a.bottom == b.bottom;
}

static RECT rect(int left, int top, int right, int bottom) {
    RECT value = {left, top, right, bottom};
    return value;
}

static int failures;
static std::vector<const char *> calls;
static int InterfaceBoxStyles[4];
static void check(bool condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

struct Font {
    int width(char *text) {
        calls.push_back("font.width");
        check(text != 0, "font measures the current title string");
        return 1;
    }
};
static Font candidate_font;
static Font *find_font(int size, int mode) {
    calls.push_back("find_font");
    check(size == 32 && mode == 1, "first title-font candidate is retained");
    return &candidate_font;
}

class Buffer {
public:
    RECT active_clip;
    RECT full_bounds;
    RECT box_clip_mutations[3];
    RECT drawn_rects[3];
    RECT draw_clips[3];
    int draw_styles[3];
    int box_count;
    RECT written_rect;
    char written_text[256];
    int written_length;
    int selected_color;
    Font *selected_font;
    RECT updated_rect;

    Buffer() : box_count(0), written_length(-1), selected_color(-1),
               selected_font(0) {
        active_clip = rect(1, 2, 3, 4);       // distinct incoming clip
        full_bounds = rect(10, 11, 910, 711); // Buffer::rect2_
        box_clip_mutations[0] = rect(20, 21, 220, 221);
        box_clip_mutations[1] = rect(30, 31, 230, 231);
        box_clip_mutations[2] = rect(40, 41, 240, 241);
    }

    const RECT &current_clip() const {
        calls.push_back("current_clip");
        return active_clip;
    }
    void restore_clip() {
        calls.push_back("restore_clip");
        set_clip(&full_bounds);
    }
    int set_clip(RECT *value) {
        calls.push_back("set_clip");
        active_clip = *value;
        return 0;
    }
    void box_sprite(RECT *bounds, int *style) {
        calls.push_back("box_sprite");
        const int index = box_count++;
        check(index < 3, "only the three original box draws occur");
        if (index >= 3) return;
        drawn_rects[index] = *bounds;
        draw_clips[index] = active_clip;
        draw_styles[index] = static_cast<int>(style - InterfaceBoxStyles);
        active_clip = box_clip_mutations[index];
    }
    void set_text_color(int color, int, int, int) {
        calls.push_back("set_text_color");
        selected_color = color;
    }
    int set_font(Font *font, Font *, Font *, Font *) {
        calls.push_back("set_font");
        selected_font = font;
        return 0;
    }
    int write_cent_l(char *text, RECT *bounds, int length) {
        calls.push_back("write_cent_l");
        strncpy(written_text, text, sizeof(written_text) - 1);
        written_text[sizeof(written_text) - 1] = '\0';
        written_rect = *bounds;
        written_length = length;
        return 0;
    }
};

class MainInterface : public Buffer {
public:
    int update_count;
    int soft_update_count;
    MainInterface() : update_count(0), soft_update_count(0) {}
    void soft_update(RECT *bounds) {
        calls.push_back("soft_update");
        ++soft_update_count;
        updated_rect = *bounds;
    }
    void update(RECT *bounds, void *) {
        calls.push_back("update");
        ++update_count;
        updated_rect = *bounds;
    }
};
static MainInterface MainInterfaceGlobal;

struct Base { char *name_string; int faction_id_current; };
struct Player { int noun_gender; int is_noun_plural; char *noun_faction; };
static Base *BaseCurrent;
static Player Players[8];
static struct { int faction_id_; } ConsoleGlobal;
static int GenderDefault;
static int PluralityDefault;
static int ChatItemIdTable[8] = {0, 101, 202, 303, 404, 505, 606, 707};
static char StringTemp[256];

static char *CharUpperA(char *text) {
    calls.push_back("CharUpperA");
    for (char *cursor = text; *cursor; ++cursor)
        *cursor = static_cast<char>(toupper(static_cast<unsigned char>(*cursor)));
    return text;
}

class BaseWin {
public:
    RECT rect12_;
    RECT info_title_rect_;
    struct Sprite { int width() const { return 10; } } sprites8_[7];
    void draw_info(int update_mode);
};

/*__ACTUAL_BODY__*/

static void run_case(int update_mode, int faction, const char *expected_text) {
    calls.clear();
    failures = 0;
    MainInterfaceGlobal = MainInterface();
    MainInterfaceGlobal.active_clip = rect(1, 2, 3, 4);
    MainInterfaceGlobal.full_bounds = rect(10, 11, 910, 711);
    MainInterfaceGlobal.box_clip_mutations[0] = rect(20, 21, 220, 221);
    MainInterfaceGlobal.box_clip_mutations[1] = rect(30, 31, 230, 231);
    MainInterfaceGlobal.box_clip_mutations[2] = rect(40, 41, 240, 241);

    Base base = {const_cast<char *>("New Eden"), faction};
    BaseCurrent = &base;
    ConsoleGlobal.faction_id_ = 1;
    Players[faction].noun_gender = 7;
    Players[faction].is_noun_plural = 1;
    Players[faction].noun_faction = const_cast<char *>("Gaian");
    GenderDefault = -1;
    PluralityDefault = -1;
    BaseWin window;
    window.rect12_ = rect(100, 200, 500, 600);
    window.info_title_rect_ = rect(300, 310, 700, 390);

    window.draw_info(update_mode);

    const RECT inflated = rect(97, 197, 503, 603);
    const RECT panel = rect(300, 310, 700, 390);
    const RECT inner = rect(316, 313, 684, 387);
    const RECT text = rect(319, 316, 681, 384);
    check(MainInterfaceGlobal.box_count == 3, "outer, panel, and inner boxes draw");
    check(same_rect(MainInterfaceGlobal.drawn_rects[0], inflated),
          "outer box keeps the inflated outer drawing rectangle");
    check(same_rect(MainInterfaceGlobal.draw_clips[0],
                    MainInterfaceGlobal.full_bounds),
          "outer box is drawn under Buffer full bounds");
    check(MainInterfaceGlobal.draw_styles[0] == 3 &&
          MainInterfaceGlobal.draw_styles[1] == 3 &&
          MainInterfaceGlobal.draw_styles[2] == 0,
          "box styles retain native order");
    check(same_rect(MainInterfaceGlobal.drawn_rects[1], panel) &&
          same_rect(MainInterfaceGlobal.draw_clips[1], panel),
          "title panel draws with its own panel clip");
    check(same_rect(MainInterfaceGlobal.drawn_rects[2], inner) &&
          same_rect(MainInterfaceGlobal.draw_clips[2],
                    MainInterfaceGlobal.box_clip_mutations[1]),
          "inner title box retains its contracted panel geometry");
    check(same_rect(MainInterfaceGlobal.active_clip,
                    MainInterfaceGlobal.box_clip_mutations[0]),
          "final clip is the live clip captured after the outer box draw");
    check(same_rect(MainInterfaceGlobal.written_rect, text),
          "centered text uses the final contracted title rectangle");
    check(strcmp(MainInterfaceGlobal.written_text, expected_text) == 0,
          "title string and faction grammar are preserved");
    check(MainInterfaceGlobal.written_length ==
          static_cast<int>(strlen(expected_text)), "write length matches title");
    check(MainInterfaceGlobal.selected_color == ChatItemIdTable[faction],
          "text color uses current base faction");
    check(MainInterfaceGlobal.selected_font == &candidate_font,
          "selected font is the measured candidate");
    check(GenderDefault == (faction == ConsoleGlobal.faction_id_ ? -1 : 7) &&
          PluralityDefault == (faction == ConsoleGlobal.faction_id_ ? -1 : 1),
          "foreign-faction grammar defaults update only for foreign bases");
    check(MainInterfaceGlobal.update_count == (update_mode != 0 ? 1 : 0),
          "nonzero update mode uses hard update");
    check(MainInterfaceGlobal.soft_update_count == (update_mode == 0 ? 1 : 0),
          "zero update mode uses soft update");
    check(same_rect(MainInterfaceGlobal.updated_rect, panel),
          "publication uses the original title panel rectangle");

    const char *required[] = {
        "restore_clip", "set_clip", "box_sprite", "current_clip",
        "set_clip", "box_sprite", "box_sprite", "CharUpperA", "find_font",
        "font.width", "set_text_color", "find_font", "set_font", "write_cent_l"
    };
    check(calls.size() >= sizeof(required) / sizeof(required[0]) + 2,
          "call trace contains body operations");
    if (calls.size() >= sizeof(required) / sizeof(required[0]) + 2) {
        for (unsigned index = 0; index < sizeof(required) / sizeof(required[0]); ++index)
            check(calls[index] == required[index], "native title call order");
        check(calls[calls.size() - 2] ==
              (update_mode == 0 ? "soft_update" : "update"),
              "selected update callback follows text draw");
        check(strcmp(calls.back(), "set_clip") == 0,
              "saved clip restores after publication");
    }
}

int main() {
    run_case(0, 1, "NEW EDEN");
    run_case(1, 2, "NEW EDEN (GAIAN)");
    return failures == 0 ? 0 : 1;
}
"""


def compile_run(tmp_path: Path, body: str, compiler_name: str) -> subprocess.CompletedProcess[str]:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    marker = "/*__ACTUAL_BODY__*/"
    assert FIXTURE.count(marker) == 1
    source = tmp_path / f"basewin_info_clip_{compiler_name}.cpp"
    binary = tmp_path / f"basewin_info_clip_{compiler_name}"
    source.write_text(FIXTURE.replace(marker, body), encoding="utf-8")
    subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         str(source), "-o", str(binary)],
        check=True, capture_output=True, text=True, timeout=60,
    )
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_draw_info_full_clip_and_panel_lifetime(tmp_path, compiler_name):
    result = compile_run(tmp_path, actual_body(), compiler_name)
    assert result.returncode == 0, f"{compiler_name}: {result.stdout}{result.stderr}"


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_clip_mutants_are_rejected(tmp_path, compiler_name):
    body = actual_body()
    mutants = (
        ("screen->restore_clip();", "screen->set_clip(&outer);"),
        ("RECT previous_clip = screen->current_clip();\n"
         "    screen->set_clip(&panel);",
         "screen->set_clip(&panel);\n"
         "    RECT previous_clip = screen->current_clip();"),
    )
    for index, (old, new) in enumerate(mutants):
        assert body.count(old) == 1, f"unanchored damage control {index}"
        case = tmp_path / str(index)
        case.mkdir()
        result = compile_run(case, body.replace(old, new, 1), compiler_name)
        assert result.returncode != 0, f"{compiler_name} accepted clip mutant {index}"
