"""Actual NetWin base/member migration controls; not game runtime proof."""

from pathlib import Path
import re
import shutil
import subprocess

import pytest

from test_selectpart_vehicle_owners import _definition

ROOT = Path(__file__).resolve().parents[2]
BASE = "4658a8a27"
TARGETS = ("UNK3", "on_redraw", "draw_session_info", "draw_player_names",
           "draw_custom_rules", "draw_diffs", "draw_factions")


def test_buffer_owner_expressions_survive_later_row_migrations():
    old = subprocess.check_output(["git", "show", f"{BASE}:src/netwin.cpp"],
                                  cwd=ROOT, text=True)
    current = (ROOT / "src/netwin.cpp").read_text()
    for name in TARGETS:
        signature = f"void NetWin::{name}()"
        before = _definition(old, signature)
        after = _definition(current, signature)
        assert before.count("Buffer *buffer = reinterpret_cast<Buffer *>(self + 0x444);") == 1
        expected = before.replace(
            "    unsigned char *const self = reinterpret_cast<unsigned char *>(this);\n", ""
        ).replace("reinterpret_cast<Buffer *>(self + 0x444)", "static_cast<Buffer *>(this)")
        expected = expected.replace("reinterpret_cast<RECT *>(self + 0x474)",
                                    "buffer_working_rect()")
        if name in ("UNK3", "on_redraw", "draw_custom_rules"):
            assert after == expected, name
        else:
            # Later lobby-row ownership work changes these complete bodies.
            # Keep the base/member invariants here, not a frozen row spelling.
            assert "Buffer *buffer = static_cast<Buffer *>(this);" in after
            assert "self + 0x444" not in after and "self + 0x474" not in after
            if name == "draw_session_info":
                assert after.count("buffer_working_rect()") == 2


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_panel_and_redraw_bodies(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    source = (ROOT / "src/netwin.cpp").read_text()
    bodies = "\n".join(_definition(source, f"void NetWin::{name}()")
                        for name in ("UNK3", "on_redraw"))
    header = (ROOT / "src/graphicwin.h").read_text()
    accessor = re.search(r"RECT \*buffer_working_rect\(\) \{[^}]+\}", header).group()
    harness = r'''
#include <cassert>
#include <cstring>
#include <cctype>
#include <string>
#include <vector>
struct RECT { int left, top, right, bottom; };
struct BoxSpriteParams { int value; };
BoxSpriteParams InterfaceBoxStyles[4];
struct Font {} font;
static char storage[80];
char *StringTemp = storage;
int labels[300];
struct Label { char *strings_ptr; } Labels = {reinterpret_cast<char *>(labels)};
static std::vector<std::string> events;
struct Strings {
    char *get(int index) {
        assert(index == 37); events.push_back("get");
        static char title[] = "title"; return title;
    }
} StringTable;
void CharUpperA(char *text) {
    events.push_back("upper");
    for (; *text; ++text) *text = static_cast<char>(std::toupper(*text));
}
Font *find_font(int size, int bold) {
    assert(size == 14 && bold == 1); events.push_back("find_font");return &font;
}
struct Buffer;
static Buffer *expected_buffer;
struct BoxCall { RECT *address; RECT value; int style; };
std::vector<BoxCall> boxes;
struct Buffer {
    RECT rect2_;
    void box_sprite(RECT *rect, BoxSpriteParams *style) {
        assert(this == expected_buffer);
        boxes.push_back({rect,*rect,static_cast<int>(style-InterfaceBoxStyles)});
        events.push_back("box");
    }
    void set_font(Font *f, int a, int b, int c) {
        assert(this == expected_buffer && f == &font && !a && !b && !c);
        events.push_back("set_font");
    }
    void set_text_color(int a, int b, int c, int d) {
        assert(this == expected_buffer && a == 225 && b == -1 && c == 1 && d == 1);
        events.push_back("color");
    }
    void write_cent_l(char *text, RECT *rect, size_t length) {
        assert(this == expected_buffer && std::strcmp(text,"TITLE") == 0 && length == 5);
        assert(rect->left == 13 && rect->top == 23 && rect->right == 87 && rect->bottom == 67);
        events.push_back("write");
    }
};
struct Win { int state[11]; virtual ~Win() {} };
struct GraphicWin : Win, Buffer {
    ACCESSOR
};
struct NetWin : GraphicWin {
    RECT setup_rects_[50];
    bool visible;
    int is_visible() { events.push_back("visible");return visible; }
    void draw_custom_rules() { events.push_back("rules"); rect2_.left = 700; }
    void draw_player_names() { events.push_back("names"); }
    void draw_diffs() { events.push_back("diffs"); }
    void draw_factions() { events.push_back("factions"); }
    void draw_session_info() { events.push_back("session"); }
    void soft_update() { events.push_back("update"); }
    void UNK3();
    void on_redraw();
};
'''.replace("ACCESSOR", accessor) + bodies + r'''
int main() {
    NetWin window;
    expected_buffer = static_cast<Buffer *>(&window);
    window.rect2_ = {1,2,3,4};
    window.setup_rects_[0] = {10,20,90,70};
    window.setup_rects_[43] = {43,0,0,0};
    window.setup_rects_[44] = {44,0,0,0};
    window.setup_rects_[45] = {45,0,0,0};
    labels[0x3f0/sizeof(int)] = 37;
    window.UNK3();
    assert(boxes.size() == 3);
    assert(boxes[0].address == &window.setup_rects_[45] && boxes[0].style == 3);
    assert(boxes[1].address == &window.setup_rects_[44] && boxes[1].style == 0);
    assert(boxes[2].address == &window.setup_rects_[43] && boxes[2].style == 0);
    events.clear(); boxes.clear(); window.visible = false;
    window.on_redraw();
    assert(events == std::vector<std::string>{"visible"} && boxes.empty());
    events.clear(); window.visible = true;
    window.on_redraw();
    assert(events == (std::vector<std::string>{"visible","box","box","box",
        "get","upper","find_font","set_font","color","write","rules",
        "box","box","box","names","diffs","factions","session","update"}));
    assert(boxes.size() == 6 && boxes[0].address == &window.rect2_);
    assert(boxes[0].value.left == 1 && boxes[0].style == 3);
    assert(window.rect2_.left == 700); // callback sees the real shared rectangle
    assert(boxes[1].value.left == 10 && boxes[1].value.right == 90 && boxes[1].style == 1);
    assert(boxes[2].value.left == 13 && boxes[2].value.right == 87 && boxes[2].style == 2);
    assert(window.setup_rects_[0].left == 10 && window.setup_rects_[0].right == 90);
    assert(boxes[3].address == &window.setup_rects_[45] && boxes[3].style == 1);
    assert(boxes[4].address == &window.setup_rects_[44] && boxes[4].style == 0);
    assert(boxes[5].address == &window.setup_rects_[43] && boxes[5].style == 0);
}
'''
    path = tmp_path / "netwin.cpp"
    binary = tmp_path / "netwin"
    path.write_text(harness)
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    "-fsanitize=address,undefined", str(path), "-o", str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
