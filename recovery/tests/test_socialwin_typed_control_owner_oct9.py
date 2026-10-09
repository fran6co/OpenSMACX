"""Actual-source host controls for SocialWin's typed control receivers.

The selected toggle block is compiled from src/socialwin.cpp. This checks host
pointer identity, member ordering, and the regression from the old second-link
offset; it does not establish VC6 ABI or runtime acceptance.
"""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/socialwin.cpp"
SETTER = """flat_toggle->set_report_icons(&energyAllocArrow_[first + 0],
            &energyAllocArrow_[first + 1], &energyAllocArrow_[first + 2]);"""


def function_body(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError(f"unterminated body: {signature}")


def selected_source():
    text = SOURCE.read_text()
    init = function_body(text, "void SocialWin::init(")
    info = function_body(text, "void SocialWin::draw_info()")
    mouseover = function_body(text, "void SocialWin::draw_mouseover()")

    assert "static_cast<Buffer *>(static_cast<GraphicWin *>(this))" in info
    assert "own_buffer->restore_clip();" in info
    assert "GraphicWin::soft_update(header_rect);" in mouseover
    assert "reinterpret_cast<Buffer *>(self + 0x444)" not in init
    assert "reinterpret_cast<char *>(button) + 0x444" not in init
    assert "FlatButton *flat_button = &flatButtons_[\n                    17 + offset / static_cast<int>(sizeof(FlatButton))];" in init
    assert "static_cast<BaseButton *>(&flatButtons_[27])" in init
    assert "static_cast<BaseButton *>(&flatButtons_[28])" in init
    assert "FlatButton elements 29..34" in init
    assert "toggle_offsets" not in init and "toggle_links" not in init
    assert "if (toggle_index == 1)" not in init
    assert init.count(SETTER) == 1

    begin = init.index("    // These six receivers are FlatButton elements 29..34;")
    end = init.index("\n    if (positive == 0) {", begin)
    return init[begin:end]


HARNESS = r'''#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <vector>
struct RECT { int left, top, right, bottom; };
struct Sprite { unsigned char bytes[0x2c]; };
static Sprite *observed_icons[35][3];
struct BaseButton {
    unsigned char bytes[0xab8];
    int init(const char *, int id, int, int, int, int, void *, int) {
        events.push_back(100 + id); return 1;
    }
    void hide() { events.push_back(200 + id_of(this)); }
    int set_bubble_text(const char *) {
        events.push_back(400 + id_of(this)); return 1;
    }
    static int id_of(const BaseButton *);
    static std::vector<int> events;
};
std::vector<int> BaseButton::events;
struct FlatButton : BaseButton {
    uint32_t field_AB8_;
    uint32_t field_ABC_[3];
    uint32_t border_styles_[3];
    uint32_t field_AD4_[3];
    uint32_t config[27];
    void set_report_icons(Sprite *a, Sprite *b, Sprite *c) {
        const int id = BaseButton::id_of(this);
        field_AD4_[0] = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(a));
        field_AD4_[1] = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(b));
        field_AD4_[2] = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(c));
        observed_icons[id][0] = a;
        observed_icons[id][1] = b;
        observed_icons[id][2] = c;
        events.push_back(300 + BaseButton::id_of(this));
    }
};
static_assert(sizeof(BaseButton) == 0xab8, "BaseButton size");
static_assert(sizeof(BaseButton) + sizeof(uint32_t) + 3 * sizeof(uint32_t)
              + 3 * sizeof(uint32_t) == 0xad4, "sprite member offset");
static_assert(sizeof(FlatButton) == 0xb4c, "FlatButton stride");
struct SocialWinFixture {
    unsigned char before_arrows[0x3de8];
    Sprite energyAllocArrow_[6];
    unsigned char between[0x6020 - 0x3ef0];
    FlatButton flatButtons_[35];
    RECT tutRects1_[9];
    void run_toggle_initializers();
};
static_assert(sizeof(((SocialWinFixture *)0)->before_arrows)
              + sizeof(((SocialWinFixture *)0)->energyAllocArrow_)
              + sizeof(((SocialWinFixture *)0)->between) == 0x6020,
              "button array base");
static_assert(0x6020 + 17 * sizeof(FlatButton) == 0x1202c,
              "effect button base");
static_assert(0x6020 + 27 * sizeof(FlatButton) == 0x19124,
              "button 27");
static_assert(0x6020 + 28 * sizeof(FlatButton) == 0x19c70,
              "button 28");
static_assert(0x6020 + 29 * sizeof(FlatButton) == 0x1a7bc,
              "toggle 0");
static_assert(0x6020 + 34 * sizeof(FlatButton) == 0x1e038,
              "toggle 5");

static SocialWinFixture panel;
int BaseButton::id_of(const BaseButton *p) {
    const uintptr_t first = reinterpret_cast<uintptr_t>(&panel.flatButtons_[0]);
    const uintptr_t current = reinterpret_cast<uintptr_t>(p);
    return static_cast<int>((current - first) / sizeof(FlatButton));
}
struct LabelsType { unsigned char *strings_ptr; };
alignas(4) static unsigned char label_bytes[0xd40];
static LabelsType Labels = { label_bytes };
struct Strings { const char *get(int) const { return "bubble"; } };
static Strings StringTable;
static void *MainInterfaceGlobal = &panel;
BLOCK
void SocialWinFixture::run_toggle_initializers() {
    BLOCK_METHOD
}
static void fail(const char *message) {
    std::fprintf(stderr, "%s\n", message); std::exit(1);
}
int main() {
    for (int i = 0; i < 6; ++i)
        panel.tutRects1_[3 + i] = {i, i + 1, i + 8, i + 9};
    panel.run_toggle_initializers();
    for (int i = 0; i < 6; ++i) {
        FlatButton &button = panel.flatButtons_[29 + i];
        const int first = (i % 2 == 0) ? 0 : 3;
        for (int j = 0; j < 3; ++j) {
            if (button.field_ABC_[j] != 0 || button.border_styles_[j] != 0)
                fail("color/style clobber");
            if (observed_icons[29 + i][j] != &panel.energyAllocArrow_[first + j])
                fail("typed arrow pointer triplet");
            if (button.field_AD4_[j] != static_cast<uint32_t>(reinterpret_cast<uintptr_t>(
                    &panel.energyAllocArrow_[first + j])))
                fail("typed pointer storage slot");
        }
    }
    std::vector<int> expected;
    for (int i = 0; i < 6; ++i) {
        expected.push_back(100 + 0x1d + i);
        expected.push_back(200 + 29 + i);
        expected.push_back(300 + 29 + i);
        expected.push_back(400 + 29 + i);
    }
    if (BaseButton::events != expected) fail("receiver and callback order");
    return 0;
}
'''


def harness_source(block: str) -> str:
    return HARNESS.replace("BLOCK_METHOD", block).replace("BLOCK", "")


def run_host(tmp_path, compiler, block):
    source_path = tmp_path / "socialwin-typed-control.cpp"
    binary_path = tmp_path / "socialwin-typed-control"
    source_path.write_text(harness_source(block))
    build = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
         str(source_path), "-o", str(binary_path)],
        capture_output=True, text=True, timeout=20,
    )
    return build, subprocess.run([str(binary_path)], capture_output=True,
                                 text=True, timeout=10) if build.returncode == 0 else None


COMPILERS = [path for path in (shutil.which("g++"), shutil.which("clang++")) if path]


@pytest.mark.parametrize("compiler", COMPILERS or [None])
def test_actual_socialwin_toggle_source_host_control(tmp_path, compiler):
    if compiler is None:
        pytest.skip("host C++ compiler required")
    build, result = run_host(tmp_path, compiler, selected_source())
    assert build.returncode == 0, build.stderr
    assert result is not None and result.returncode == 0, result.stderr if result else "no executable"


@pytest.mark.parametrize("compiler", COMPILERS or [None])
def test_old_second_link_offset_is_detected(tmp_path, compiler):
    if compiler is None:
        pytest.skip("host C++ compiler required")
    block = selected_source()
    assert SETTER in block
    old_write = """if (toggle_index == 1) {
            uint32_t *old_link = reinterpret_cast<uint32_t *>(
                reinterpret_cast<unsigned char *>(this) + 0x1bdcc);
            old_link[0] = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(
                &energyAllocArrow_[first + 0]));
            old_link[1] = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(
                &energyAllocArrow_[first + 1]));
            old_link[2] = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(
                &energyAllocArrow_[first + 2]));
        } else {
            flat_toggle->set_report_icons(&energyAllocArrow_[first + 0],
                &energyAllocArrow_[first + 1], &energyAllocArrow_[first + 2]);
        }"""
    block = block.replace(SETTER, old_write, 1)
    build, result = run_host(tmp_path, compiler, block)
    assert build.returncode == 0, build.stderr
    assert result is not None and result.returncode != 0
    assert "color/style clobber" in result.stderr
