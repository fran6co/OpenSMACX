"""Compile the complete actual SocialWin redraw body and a receiver mutant.

The fixture models the owner's real 35-element FlatButton run with distinct
public redraw identities, rectangle/global work, and the full helper order.
It checks bounded host behavior, not native ABI or live GUI acceptance.
"""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/socialwin.cpp"
SIGNATURE = "void SocialWin::on_redraw()"


def extract_function(source: str, signature: str = SIGNATURE) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError("unterminated production SocialWin::on_redraw")


FIXTURE = r'''
#include <algorithm>
#include <cassert>
#include <vector>

struct Event { int kind; int value; };
bool operator==(const Event &left, const Event &right) {
    return left.kind == right.kind && left.value == right.value;
}
std::vector<Event> events;

struct RECT { int left, top, right, bottom; };
bool same_rect(const RECT &a, const RECT &b) {
    return a.left == b.left && a.top == b.top &&
           a.right == b.right && a.bottom == b.bottom;
}
struct UnionRecord { RECT first, second, result; };
std::vector<UnionRecord> unions;
std::vector<RECT> boxed_rects;
std::vector<const RECT *> union_outputs;
std::vector<const RECT *> box_inputs;

int UnionRect(RECT *out, const RECT *first, const RECT *second) {
    const RECT first_copy = *first;
    const RECT second_copy = *second;
    out->left = std::min(first_copy.left, second_copy.left);
    out->top = std::min(first_copy.top, second_copy.top);
    out->right = std::max(first_copy.right, second_copy.right);
    out->bottom = std::max(first_copy.bottom, second_copy.bottom);
    unions.push_back(UnionRecord{first_copy, second_copy, *out});
    union_outputs.push_back(out);
    events.push_back(Event{3, static_cast<int>(unions.size()) - 1});
    return 1;
}

struct Buffer {
    int identity;
    explicit Buffer(int value = 0) : identity(value) {}
    void restore_clip() { events.push_back(Event{1, identity}); }
    void restore_back(int value) { events.push_back(Event{2, value}); }
    void box_sprite(RECT *area, int *) {
        boxed_rects.push_back(*area);
        box_inputs.push_back(area);
        events.push_back(Event{4, identity});
    }
};
struct GraphicWin : Buffer {
    GraphicWin() : Buffer(2) {}
};
Buffer MainInterfaceGlobal(1);
int InterfaceBoxStyles[4];
const int MaxSocialCatNum = 4;

int next_button_identity;
struct FlatButton {
    int identity;
    int redraw_count;
    FlatButton() : identity(next_button_identity++), redraw_count(0) {}
    void redraw() {
        ++redraw_count;
        events.push_back(Event{8, identity});
    }
};

struct SocialWin : GraphicWin {
    RECT tooltipLabs_, tooltipPsych_, tooltipEconomy_;
    RECT tutRect11_, tutRect12_, tutRect13_;
    FlatButton flatButtons_[35];
    SocialWin()
        : tooltipLabs_{10,20,30,40}, tooltipPsych_{50,60,70,80},
          tooltipEconomy_{5,15,45,55}, tutRect11_{100,110,120,130},
          tutRect12_{120,140,155,165}, tutRect13_{140,150,160,170} {}
    void draw_presets() { events.push_back(Event{5, 0}); }
    void draw_social(int category) { events.push_back(Event{6, category}); }
    void draw_faction_basal() { events.push_back(Event{7, 0}); }
    void draw_info() { events.push_back(Event{7, 1}); }
    void draw_overview() { events.push_back(Event{7, 2}); }
    void draw_economy_alloc() { events.push_back(Event{7, 3}); }
    void draw_labs_alloc() { events.push_back(Event{7, 4}); }
    void draw_psych_alloc() { events.push_back(Event{7, 5}); }
    void draw_mouseover() { events.push_back(Event{7, 6}); }
    void on_redraw();
};
RECT_OPERATION
BODY

int main() {
    next_button_identity = 0;
    events.clear(); unions.clear(); boxed_rects.clear();
    union_outputs.clear(); box_inputs.clear();
    SocialWin window;
    window.on_redraw();

    std::vector<Event> expected{
        {1,1}, {1,2}, {2,1},
        {3,0}, {3,1}, {4,1}, {3,2}, {3,3}, {4,2},
        {5,0}, {6,0}, {6,1}, {6,2}, {6,3},
        {7,0}, {7,1}, {7,2}, {7,3}, {7,4}, {7,5}, {7,6},
        {8,17}, {8,18}, {8,19}, {8,20}, {8,21},
        {8,22}, {8,23}, {8,24}, {8,25}, {8,26}
    };
    assert(events == expected);

    assert(unions.size() == 4);
    assert((same_rect(unions[0].first, RECT{10,20,30,40})));
    assert((same_rect(unions[0].second, RECT{50,60,70,80})));
    assert((same_rect(unions[1].first, RECT{10,20,70,80})));
    assert((same_rect(unions[1].second, RECT{5,15,45,55})));
    assert((same_rect(unions[2].first, RECT{100,110,120,130})));
    assert((same_rect(unions[2].second, RECT{140,150,160,170})));
    assert((same_rect(unions[3].first, RECT{100,110,160,170})));
    assert((same_rect(unions[3].second, RECT{120,140,155,165})));
    assert(boxed_rects.size() == 2);
    for (const RECT *area : union_outputs) assert(area == union_outputs[0]);
    for (const RECT *area : box_inputs) assert(area == union_outputs[0]);
    assert((same_rect(boxed_rects[0], RECT{2,12,73,83})));
    assert((same_rect(boxed_rects[1], RECT{97,107,163,173})));

    for (int i = 0; i < 35; ++i) {
        const int expected_redraws = (i >= 17 && i <= 26) ? 1 : 0;
        assert(window.flatButtons_[i].identity == i);
        assert(window.flatButtons_[i].redraw_count == expected_redraws);
    }
}
'''


def compile_and_run(tmp_path, compiler: str, body: str, stem: str):
    source = tmp_path / f"{stem}.cpp"
    executable = tmp_path / stem
    operation = extract_function(SOURCE.read_text(encoding="utf-8"),
                                 "static inline RECT *social_expand_rect(")
    source.write_text(FIXTURE.replace("RECT_OPERATION", operation).replace("BODY", body))
    compiled = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
         str(source), "-o", str(executable)],
        capture_output=True,
        text=True,
    )
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_socialwin_redraw_receivers_and_old_loop_mutant(
    tmp_path, compiler_name
):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")

    body = extract_function(SOURCE.read_text(encoding="utf-8"))
    result = compile_and_run(tmp_path, compiler, body, "socialwin-redraw-actual")
    assert result.returncode == 0, result.stderr

    correct_loop = """    for (int slot = 0; slot < 11; ++slot) {
        if (slot != 3) {
            int button = slot;
            if (slot >= 3) --button;
            flatButtons_[17 + button].redraw();
        }
    }"""
    old_loop = """    for (int button = 0; button < 11; ++button) {
        if (button != 3) {
            flatButtons_[17 + button].redraw();
        }
    }"""
    mutant = body.replace(correct_loop, old_loop, 1)
    assert mutant != body
    result = compile_and_run(tmp_path, compiler, mutant, "socialwin-redraw-old-loop")
    assert result.returncode != 0

    mutant = body.replace("if (slot >= 3) --button;", "", 1)
    assert mutant != body
    result = compile_and_run(tmp_path, compiler, mutant,
                             "socialwin-redraw-uncompressed-slot")
    assert result.returncode != 0

    marker = "    UnionRect(&panel, &tutRect11_, &tutRect13_);"
    assert body.count(marker) == 1
    start = body.index(marker)
    end = body.index("    draw_presets();", start)
    separate = "    RECT second_panel;\n" + body[start:end].replace("panel", "second_panel")
    mutant = body[:start] + separate + body[end:]
    result = compile_and_run(tmp_path, compiler, mutant,
                             "socialwin-redraw-separate-rect")
    assert result.returncode != 0
