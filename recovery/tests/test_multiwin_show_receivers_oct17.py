"""Actual-source and damage controls for MultiWin::show.

The fixture extracts the whole current method body. It is not a native ABI,
linked-image, or game-runtime test.
"""

from pathlib import Path
import re
import shutil
import subprocess

import pytest


PROJECT = Path(__file__).resolve().parents[2]
SOURCE = PROJECT / "src/multiwin.cpp"
WIN_HEADER = PROJECT / "src/win.h"
SHOW_SIGNATURE = "void MultiWin::show()"
MARK_VISIBLE_SIGNATURE = "void mark_visible()"


def extract_function(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    state = "code"
    index = opening
    while index < len(source):
        char = source[index]
        next_char = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if char == "/" and next_char == "/":
                state = "line_comment"
                index += 1
            elif char == "/" and next_char == "*":
                state = "block_comment"
                index += 1
            elif char == '"':
                state = "string"
            elif char == "'":
                state = "char"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[start:index + 1]
        elif state == "line_comment":
            if char == "\n":
                state = "code"
        elif state == "block_comment":
            if char == "*" and next_char == "/":
                state = "code"
                index += 1
        elif state == "string":
            if char == "\\":
                index += 1
            elif char == '"':
                state = "code"
        elif state == "char":
            if char == "\\":
                index += 1
            elif char == "'":
                state = "code"
        index += 1
    raise AssertionError("MultiWin::show definition is not brace-balanced")


def actual_show_body() -> str:
    source = SOURCE.read_text()
    body = extract_function(source, SHOW_SIGNATURE)
    assert "BODY GOES HERE." not in body
    return body


def actual_mark_visible_method() -> str:
    header = WIN_HEADER.read_text()
    method = extract_function(header, MARK_VISIBLE_SIGNATURE)
    assert "iSomeFlag_" in method
    assert "1U" in method or "WinFlagVisible" in method
    return method


HARNESS = r'''#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

using std::string;
using std::vector;

typedef uint32_t DWORD;

static vector<string> events;
static void record(const char *event) { events.push_back(event); }
static int failures = 0;
static void check(bool ok, const char *message) {
    if (!ok) {
        ++failures;
        fprintf(stderr, "FAIL: %s\n", message);
    }
}

struct RECT { int left, top, right, bottom; };
static bool same_rect(const RECT &a, const RECT &b) {
    return a.left == b.left && a.top == b.top &&
           a.right == b.right && a.bottom == b.bottom;
}

static const unsigned int WinFlagVisible = 1U;
static const int WinFillColour = 7;
static const int WindowAnimationFrameDelay = 2;
static int IsMultiplayerNet = 0;
static int ExpansionEnabled = 0;
static int MultiWindowVisibilityState = -1;
static int MultiWindowRedrawSuppressed = 0;
static int WinFlipClipped = 0;
static unsigned int fake_clock = 0;

struct Player { unsigned int rule_flags; };
static Player Players[8];
struct Console { int faction_id_; };
static Console ConsoleGlobal = {0};
struct GameDataState { unsigned int game_preferences_; };
struct GameData { GameDataState fields_; };
static GameData GameDataStateStorage;

struct RECT;
struct Buffer {
    const char *role;
    bool local;
    explicit Buffer(const char *name, bool is_local)
        : role(name), local(is_local) {
        if (local) record("background.ctor");
    }
    Buffer() : role("background"), local(true) { record("background.ctor"); }
    ~Buffer() { if (local) record("background.dtor"); }
    void resize(int width, int height) {
        check(width == 320 && height == 200, "background resize dimensions");
        record("background.resize");
    }
    void fill(int colour) {
        check(colour == WinFillColour, "background fill colour");
        record("background.fill");
    }
    void draw(Buffer *source, int mode, int x, int y, int sx, int sy);
    void set_clip(const RECT *rect);
    void restore_clip() { record("screen.restore_clip"); }
};

struct DrawCall {
    string receiver;
    string source;
    int mode, x, y, sx, sy;
};
static vector<DrawCall> draw_calls;
void Buffer::draw(Buffer *source, int mode, int x, int y, int sx, int sy) {
    DrawCall call = {role, source ? source->role : "null", mode, x, y, sx, sy};
    draw_calls.push_back(call);
    record("buffer.draw");
}

static RECT clipped_rect;
void Buffer::set_clip(const RECT *rect) {
    check(rect != 0, "non-null clip rectangle");
    clipped_rect = *rect;
    record("screen.set_clip");
}

struct Wave {
    int plays;
    int fades;
    Wave() : plays(0), fades(0) {}
    void play() { ++plays; record("wave.play"); }
    void fade(int frames) {
        check(frames == 50, "wave fade argument");
        ++fades;
        record("wave.fade");
    }
};
static Wave g_MULTIWIN_WAVE;

static void do_sound() { record("do_sound"); }
static DWORD timeGetTime() {
    record("timeGetTime");
    return fake_clock++;
}

struct Win {
    // Borrower receiver double only; no private startup guard is modeled here.
    static Buffer screen_buffer_;
    unsigned int iSomeFlag_;
    Win() : iSomeFlag_(0) {}
    void show(int mode) {
        check(mode == 3, "base Win::show mode");
        record("Win.show.3");
    }
    static void flip(const RECT *rect) {
        check(WinFlipClipped == 1, "flip occurs while clipping flag is set");
        check(rect != 0 && same_rect(*rect, clipped_rect), "flip rectangle equals clip");
        record("Win.flip");
    }
};
Buffer Win::screen_buffer_("screen", false);

struct WinView {
protected:
    unsigned int iSomeFlag_;
public:
__WIN_MARK_VISIBLE_METHOD__
    unsigned int flags_for_test() const { return iSomeFlag_; }
    void set_flags_for_test(unsigned int value) { iSomeFlag_ = value; }
};
struct FlatButton : public WinView {
    int show_calls;
    FlatButton() : show_calls(0) {}
    void show(int) {
        ++show_calls;
        mark_visible();
        record("child.show");
    }
};

struct Sprite {
    int value;
    const char *name;
    Sprite() : value(0), name("sprite") {}
    int height() {
        record(name);
        return value;
    }
};

struct BitmapHeader { int biWidth, biHeight; };
struct Dib { BitmapHeader bmiHeader; };

class MultiWin : public Win {
public:
    unsigned int panel_open_;
    int hovered_spot_;
    FlatButton flat_button1_;
    FlatButton flat_button2_;
    Dib dib_;
    Sprite sprite3_;
    Sprite sprite4_;
    RECT rect2_;
    int client_dx, client_dy;
    Buffer *last_updated;
    int redraws, draws;

    MultiWin() : panel_open_(0), hovered_spot_(22), client_dx(3), client_dy(5),
                 last_updated(0), redraws(0), draws(0) {
        dib_.bmiHeader.biWidth = 320;
        dib_.bmiHeader.biHeight = -200;
        sprite3_.value = 30;
        sprite3_.name = "sprite3.height";
        sprite4_.value = 8;
        sprite4_.name = "sprite4.height";
        rect2_.left = 10; rect2_.top = 20;
        rect2_.right = 210; rect2_.bottom = 140;
    }
    void update_window_to_buffer(Buffer *target) {
        last_updated = target;
        check(target != 0 && string(target->role) == "background",
              "window redraw goes into local background");
        record("update.background");
    }
    void draw(int mode) {
        check(mode == 0, "redraw draw mode");
        ++draws;
        record("draw.0");
    }
    void on_redraw() {
        check(MultiWindowRedrawSuppressed == 1,
              "redraw callback observes suppression");
        ++redraws;
        record("on_redraw");
        draw(0);
    }
    void client_to_screen(RECT *rect) {
        check(rect != 0, "client_to_screen rectangle");
        rect->left += client_dx; rect->right += client_dx;
        rect->top += client_dy; rect->bottom += client_dy;
        record("client_to_screen");
    }
    void show();
};

__MULTIWIN_SHOW_BODY__

static void reset_globals() {
    events.clear(); draw_calls.clear();
    IsMultiplayerNet = 0; ExpansionEnabled = 0;
    MultiWindowVisibilityState = -1; MultiWindowRedrawSuppressed = 0;
    WinFlipClipped = 0; fake_clock = 0;
    GameDataStateStorage.fields_.game_preferences_ = 0;
    Players[0].rule_flags = 0;
    g_MULTIWIN_WAVE.plays = g_MULTIWIN_WAVE.fades = 0;
}

static bool has_event(const char *name) {
    for (size_t i = 0; i < events.size(); ++i)
        if (events[i] == name) return true;
    return false;
}

static void visibility_matrix() {
    for (int net = 0; net != 2; ++net) {
        for (int expansion = 0; expansion != 2; ++expansion) {
            for (int rule = 0; rule != 2; ++rule) {
                reset_globals();
                MultiWin window;
                IsMultiplayerNet = net;
                ExpansionEnabled = expansion;
                Players[0].rule_flags = rule ? 0x80 : 0;
                window.flat_button1_.set_flags_for_test(0xA0);
                window.flat_button2_.set_flags_for_test(0x40);
                window.show();
                const bool want_first = !expansion || !rule;
                const bool want_second = net != 0;
                const unsigned int expected_first = 0xA0 | (want_first ? WinFlagVisible : 0);
                const unsigned int expected_second = 0x40 | (want_second ? WinFlagVisible : 0);
                check(window.flat_button1_.flags_for_test() == expected_first,
                      "button1 visible-bit condition and unrelated-bit preservation");
                check(window.flat_button2_.flags_for_test() == expected_second,
                      "button2 network-visible bit and unrelated-bit preservation");
                check(window.flat_button1_.show_calls == 0 && window.flat_button2_.show_calls == 0,
                      "show must not call child show callbacks");
                check(window.hovered_spot_ == -1 && window.panel_open_ == 1,
                      "show resets hover and marks panel open");
                check(window.redraws == 1 && window.draws == 1,
                      "one redraw callback and draw");
                check(!has_event("child.show"), "no child show event");
                check(MultiWindowRedrawSuppressed == 0,
                      "redraw suppression is cleared after callback");
            }
        }
    }
}

static void early_return_destroys_background_only() {
    reset_globals();
    MultiWin window;
    window.panel_open_ = 1;
    window.hovered_spot_ = 19;
    window.flat_button1_.set_flags_for_test(0x20);
    window.flat_button2_.set_flags_for_test(0x40);
    const unsigned int first = window.flat_button1_.flags_for_test();
    const unsigned int second = window.flat_button2_.flags_for_test();
    window.show();
    check(events.size() == 2 && events[0] == "background.ctor" &&
          events[1] == "background.dtor", "early return still destroys local background");
    check(window.panel_open_ == 1 && window.hovered_spot_ == 19,
          "early return leaves panel and hover state unchanged");
    check(window.flat_button1_.flags_for_test() == first &&
          window.flat_button2_.flags_for_test() == second,
          "early return leaves controls untouched");
}

static void background_roles_geometry_and_solo_animation() {
    reset_globals();
    MultiWin window;
    GameDataStateStorage.fields_.game_preferences_ = 0x400 | 0x08000000;
    window.show();

    check(window.last_updated != 0 && string(window.last_updated->role) == "background",
          "redraw populated the local background");
    check(draw_calls.size() == 6, "six slide frames are drawn");
    const int positions[] = {115, 97, 79, 61, 43, 25};
    RECT expected_clip = {13, 25, 213, 145};
    check(same_rect(clipped_rect, expected_clip), "client-to-screen clip rectangle");
    for (size_t i = 0; i < draw_calls.size(); ++i) {
        check(draw_calls[i].receiver == "background" && draw_calls[i].source == "screen",
              "local background draws from screen buffer, not vice versa");
        check(draw_calls[i].mode == 9 && draw_calls[i].x == 13 &&
              draw_calls[i].y == positions[i] && draw_calls[i].sx == 1 &&
              draw_calls[i].sy == 1, "slide draw arguments and position sequence");
    }
    check(fake_clock == 18, "monotonic fake time bounds each frame delay");
    check(has_event("wave.play") && has_event("wave.fade"),
          "wave plays before draw and fades after animation");
    check(g_MULTIWIN_WAVE.plays == 1 && g_MULTIWIN_WAVE.fades == 1,
          "wave callback counts");
    check(WinFlipClipped == 0, "flip-clipped flag cleared after animation");
    check(has_event("screen.set_clip") && has_event("screen.restore_clip"),
          "screen clip is restored");
    check(has_event("Win.show.3"), "solo mode finishes with Win::show(3)");
    check(!has_event("child.show"), "animation path still avoids child show callbacks");

    vector<string> expected;
    const char *prefix[] = {"background.ctor", "wave.play", "on_redraw", "draw.0",
        "background.resize", "background.fill", "update.background",
        "client_to_screen", "sprite3.height", "screen.set_clip"};
    for (size_t i = 0; i < sizeof(prefix) / sizeof(prefix[0]); ++i)
        expected.push_back(prefix[i]);
    for (int frame = 0; frame < 6; ++frame) {
        expected.push_back("timeGetTime");
        expected.push_back("buffer.draw");
        expected.push_back("Win.flip");
        expected.push_back("do_sound");
        expected.push_back("timeGetTime");
        expected.push_back("do_sound");
        expected.push_back("timeGetTime");
    }
    expected.push_back("screen.restore_clip");
    expected.push_back("wave.fade");
    expected.push_back("Win.show.3");
    expected.push_back("background.dtor");
    check(events.size() == expected.size(), "full solo animation event count");
    for (size_t i = 0; i < expected.size() && i < events.size(); ++i)
        check(events[i] == expected[i], "full redraw/animation/wave/show ordering");
    check(!events.empty() && events[events.size() - 1] == "background.dtor",
          "local background is destroyed at function exit");
}

static void network_skips_animation_and_base_show_but_keeps_wave_order() {
    reset_globals();
    MultiWin window;
    IsMultiplayerNet = 1;
    GameDataStateStorage.fields_.game_preferences_ = 0x400 | 0x08000000;
    window.show();
    check(draw_calls.empty(), "network mode skips slide animation");
    check(has_event("screen.set_clip") && !has_event("screen.restore_clip"),
          "network mode preserves the unconditional clip setup and skips animation restore");
    check(!has_event("Win.show.3"), "network mode skips base Win show");
    check(g_MULTIWIN_WAVE.plays == 1 && g_MULTIWIN_WAVE.fades == 1,
          "network mode retains wave callbacks");
    check(events.size() >= 3 && events[1] == "wave.play" &&
          events[events.size() - 2] == "wave.fade",
          "network wave play/fade bracket redraw and background work");
    check(has_event("sprite4.height"), "network branch reads the second sprite height");
    check(!events.empty() && events[events.size() - 1] == "background.dtor",
          "network local background is destroyed at function exit");
}

int main() {
    visibility_matrix();
    early_return_destroys_background_only();
    background_roles_geometry_and_solo_animation();
    network_skips_animation_and_base_show_but_keeps_wave_order();
    return failures == 0 ? 0 : 1;
}
'''


HOST_COMPILERS = tuple(name for name in ("g++", "clang++") if shutil.which(name)) or (None,)


@pytest.fixture(params=HOST_COMPILERS)
def host_compiler(request):
    assert request.param is not None, "neither g++ nor clang++ is available"
    return request.param


def compile_run(tmp_path: Path, body: str, compiler: str):
    tmp_path.mkdir(parents=True, exist_ok=True)
    harness = HARNESS.replace("__MULTIWIN_SHOW_BODY__", body)
    harness = harness.replace("__WIN_MARK_VISIBLE_METHOD__", actual_mark_visible_method())
    cpp = tmp_path / "multiwin_show_actual_source.cpp"
    cpp.write_text(harness)
    executable = tmp_path / "multiwin_show_actual_source"
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         str(cpp), "-o", str(executable)],
        capture_output=True, text=True, timeout=30,
    )
    if built.returncode != 0:
        return built, None
    ran = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
    return built, ran


def test_actual_source_show_roles_and_callbacks(tmp_path, host_compiler):
    built, ran = compile_run(tmp_path, actual_show_body(), host_compiler)
    assert built.returncode == 0, built.stdout + built.stderr
    assert ran is not None and ran.returncode == 0, "positive control: " + (ran.stdout + ran.stderr if ran else "not run")


def test_actual_source_receiver_reversal_is_detected(tmp_path, host_compiler):
    body = actual_show_body()
    good = re.compile(
        r"background\s*\.\s*draw\s*\(\s*&\s*Win::screen_buffer_\s*,\s*9\s*,\s*screen_rect\.left\s*,\s*"
        r"position\s*,\s*1\s*,\s*1\s*\)\s*;"
    )
    bad = "Win::screen_buffer_.draw(&background, 9, screen_rect.left, position, 1, 1);"
    assert good.search(body), "positive control must contain the corrected typed receiver order"
    baseline_build, baseline_run = compile_run(tmp_path / "baseline", body, host_compiler)
    assert baseline_build.returncode == 0, "positive control did not compile: " + baseline_build.stderr
    assert baseline_run is not None and baseline_run.returncode == 0, "positive control failed: " + (baseline_run.stderr if baseline_run else "not run")
    damaged = good.sub(bad, body, count=1)
    damaged_build, damaged_run = compile_run(tmp_path / "reversed", damaged, host_compiler)
    assert damaged_build.returncode == 0, "receiver mutant did not compile: " + damaged_build.stderr
    assert damaged_run is not None and damaged_run.returncode != 0, "receiver reversal escaped the runtime oracle"


def test_actual_source_child_show_callback_mutation_is_detected(tmp_path, host_compiler):
    body = actual_show_body()
    good1 = re.compile(r"flat_button1_\s*\.\s*mark_visible\s*\(\s*\)\s*;")
    good2 = re.compile(r"flat_button2_\s*\.\s*mark_visible\s*\(\s*\)\s*;")
    assert good1.search(body) and good2.search(body), "positive control must use the source-owned inline visibility method"
    baseline_build, baseline_run = compile_run(tmp_path / "baseline", body, host_compiler)
    assert baseline_build.returncode == 0, "positive control did not compile: " + baseline_build.stderr
    assert baseline_run is not None and baseline_run.returncode == 0, "positive control failed: " + (baseline_run.stderr if baseline_run else "not run")
    damaged = good1.sub("flat_button1_.show(1);", body, count=1)
    damaged_build, damaged_run = compile_run(tmp_path / "callback", damaged, host_compiler)
    assert damaged_build.returncode == 0, "callback mutant did not compile: " + damaged_build.stderr
    assert damaged_run is not None and damaged_run.returncode != 0, "reintroduced child show callback escaped the runtime oracle"
