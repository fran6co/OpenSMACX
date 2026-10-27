"""Actual-source host controls for SocialWin's final two flat buttons."""
from pathlib import Path
import shutil
import subprocess
import re

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/socialwin.cpp"
START = "    // Initialize SocialWin flat buttons 27 and 28 from their separate labels."
END = "\n    {\n        CheckButton *lock0 = &energyLockButtons_[0];"


def function_body(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError(f"unterminated body: {signature}")


def selected_block() -> str:
    init = function_body(SOURCE.read_text(), "void SocialWin::init(")
    begin = init.index(START)
    end = init.index(END, begin)
    return init[begin:end]


HARNESS = r'''#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

struct RECT { int left, top, right, bottom; };
struct Font {};
struct SocialWinFixture;
struct BaseButton {
    int init(const char *, int, int, int, int, int, void *, int);
    void hide();
    void redraw();
    int set_bubble_text(const char *);
    static int id_of(const BaseButton *);
};
struct Buffer {
    void set_font(Font *, int, int, int);
};
struct FlatButton : BaseButton, Buffer {};
struct SocialWinFixture {
    FlatButton flatButtons_[35];
    RECT tutRect12_;
    RECT tutRect13_;
    Font font2_;
    void run_buttons();
};

static SocialWinFixture panel;
static std::vector<std::string> events;
static std::vector<std::string> captions(35);
static std::vector<std::string> bubbles(35);
static int init_args[35][8] = {};
static void *init_parent[35] = {};
static Font *font_arg[35] = {};
static int font_values[35][3] = {};
static char StringTemp[128];
static int Language;

struct LabelsType { const int *strings_ptr; };
static int label_ids[0xd34 / 4 + 1];
static LabelsType Labels = { label_ids };
struct StringTableType {
    const char *get(int id) const {
        switch (id) {
        case 0x4f4: return "Caption Twenty Seven";
        case 0x194: return "Caption Twenty Eight";
        case 0xd34: return "Bubble Twenty Seven";
        default: return "unexpected label";
        }
    }
};
static StringTableType StringTable;

static void CharUpperA(char *text) {
    for (; *text; ++text)
        if (*text >= 'a' && *text <= 'z') *text = static_cast<char>(*text - 'a' + 'A');
}
static void fail(const char *message) {
    std::fprintf(stderr, "%s\n", message);
    std::exit(1);
}
int BaseButton::id_of(const BaseButton *button) {
    for (int i = 0; i < 35; ++i)
        if (button == static_cast<const BaseButton *>(&panel.flatButtons_[i])) return i;
    return -1;
}
int BaseButton::init(const char *text, int id, int x, int y, int w, int h,
                     void *parent, int trailing) {
    const int index = id_of(this);
    if (index >= 0) {
        captions[index] = text;
        init_args[index][0] = id;
        init_args[index][1] = x;
        init_args[index][2] = y;
        init_args[index][3] = w;
        init_args[index][4] = h;
        init_args[index][5] = trailing;
        init_parent[index] = parent;
    }
    events.push_back("init:" + std::to_string(index));
    return 1;
}
void BaseButton::hide() { events.push_back("hide:" + std::to_string(id_of(this))); }
void BaseButton::redraw() { events.push_back("redraw:" + std::to_string(id_of(this))); }
int BaseButton::set_bubble_text(const char *text) {
    const int index = id_of(this);
    if (index >= 0) bubbles[index] = text;
    events.push_back("bubble:" + std::to_string(index));
    return 1;
}
void Buffer::set_font(Font *font, int first, int second, int third) {
    int index = -1;
    for (int i = 0; i < 35; ++i)
        if (this == static_cast<Buffer *>(&panel.flatButtons_[i])) index = i;
    if (index >= 0) {
        font_arg[index] = font;
        font_values[index][0] = first;
        font_values[index][1] = second;
        font_values[index][2] = third;
    }
    events.push_back("font:" + std::to_string(index));
}
void SocialWinFixture::run_buttons() {
    BLOCK
}

static void reset_observations() {
    events.clear();
    captions.assign(35, std::string());
    bubbles.assign(35, std::string());
    for (int i = 0; i < 35; ++i) {
        for (int j = 0; j < 8; ++j) init_args[i][j] = 0;
        init_parent[i] = 0;
        font_arg[i] = 0;
        for (int j = 0; j < 3; ++j) font_values[i][j] = -1;
    }
}
static bool check_case(int language) {
    reset_observations();
    Language = language;
    panel.tutRect12_ = { 10, 20, 110, 70 };
    panel.tutRect13_ = { 30, 40, 230, 140 };
    panel.run_buttons();
    const std::vector<std::string> expected = {
        "init:27", "hide:27", "font:27", "redraw:27", "bubble:27",
        "init:28", "hide:28", "font:28", "redraw:28"
    };
    if (events != expected) return false;
    if (captions[27] != "CAPTION TWENTY SEVEN" ||
        captions[28] != "CAPTION TWENTY EIGHT" ||
        bubbles[27] != "Bubble Twenty Seven" || !bubbles[28].empty()) return false;
    if (init_args[27][0] != 0x1b || init_args[27][1] != 10 ||
        init_args[27][2] != 20 || init_args[27][3] != 100 ||
        init_args[27][4] != 50 || init_args[27][5] != 0 ||
        init_parent[27] != &panel || font_arg[27] != &panel.font2_ ||
        font_values[27][0] != 0 || font_values[27][1] != 0 ||
        font_values[27][2] != 0) return false;
    if (init_args[28][0] != 0x1c || init_args[28][1] != 30 ||
        init_args[28][2] != 40 || init_args[28][3] != 200 ||
        init_args[28][4] != 100 || init_args[28][5] != 0 ||
        init_parent[28] != &panel || font_arg[28] != &panel.font2_ ||
        font_values[28][0] != 0 || font_values[28][1] != 0 ||
        font_values[28][2] != 0) return false;
    return true;
}
int main() {
    label_ids[0x4f4 / 4] = 0x4f4;
    label_ids[0x194 / 4] = 0x194;
    label_ids[0xd34 / 4] = 0xd34;
    if (!check_case(2) || !check_case(0))
        fail("button receiver, arguments, labels, or callback sequence");
    return 0;
}
'''


def build_and_run(tmp_path, compiler, block):
    source_path = tmp_path / "socialwin-missing-buttons.cpp"
    binary_path = tmp_path / "socialwin-missing-buttons"
    source_path.write_text(HARNESS.replace("BLOCK", block))
    build = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
         str(source_path), "-o", str(binary_path)],
        capture_output=True, text=True, timeout=20,
    )
    run = (subprocess.run([str(binary_path)], capture_output=True, text=True, timeout=10)
           if build.returncode == 0 else None)
    return build, run


COMPILERS = [path for path in (shutil.which("g++"), shutil.which("clang++")) if path]


@pytest.mark.parametrize("compiler", COMPILERS or [None])
def test_actual_socialwin_button_initializers(tmp_path, compiler):
    if compiler is None:
        pytest.skip("host C++ compiler required")
    build, run = build_and_run(tmp_path, compiler, selected_block())
    assert build.returncode == 0, build.stderr
    assert run is not None and run.returncode == 0, run.stderr if run else "no executable"


@pytest.mark.parametrize(
    "mutation",
    [
        lambda block: re.sub(
            r"(?s)        base_button->init\(StringTemp, 0x1B,.*?this, 0\);\n",
            "", block, count=1),
        lambda block: re.sub(
            r"(?s)        base_button->init\(StringTemp, 0x1C,.*?this, 0\);\n",
            "", block, count=1),
        lambda block: block.replace("0x4F4 / 4", "0x194 / 4", 1),
        lambda block: block.replace("0x194 / 4", "0x4F4 / 4", 1),
        lambda block: block.replace("&flatButtons_[27]", "&flatButtons_[28]", 1),
    ],
    ids=["missing-button-27-init", "missing-button-28-init",
         "wrong-button-27-caption", "wrong-button-28-caption",
         "wrong-button-27-receiver"],
)
@pytest.mark.parametrize("compiler", COMPILERS or [None])
def test_button_initialization_damage_is_detected(tmp_path, compiler, mutation):
    if compiler is None:
        pytest.skip("host C++ compiler required")
    block = mutation(selected_block())
    build, run = build_and_run(tmp_path, compiler, block)
    assert build.returncode == 0, build.stderr
    assert run is not None and run.returncode != 0
    assert "button receiver, arguments, labels, or callback sequence" in run.stderr
