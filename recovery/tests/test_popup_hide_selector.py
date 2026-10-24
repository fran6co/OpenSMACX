"""Actual-source Popup::hide divisor and callback-order control."""

from pathlib import Path
import shutil
import subprocess

import pytest

from test_autosound_sound_slots20_31_actual_source_oct13x import extract_definition


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/IMAGE_PIN").is_file())

HARNESS = r"""
#include <stdint.h>
#include <string>

static std::string events;
class Popup;
class Win;
static Popup *active_popup;
static Win *active_popup_win;
static int mutate_in_scroll;
static int scroll_hide_calls, slide_hide_calls, scroll_query_calls;
static int main_visibility_calls, popup_visibility_calls, base_hide_calls;
static int helper_receiver_error;

struct MainInterfaceProbe {
    int visible;
    MainInterfaceProbe() : visible(1) {}
    int is_visible() { ++main_visibility_calls; events += 'I'; return visible; }
};
static MainInterfaceProbe MainInterfaceGlobal;

class Win {
public:
    int visible;
    Win() : visible(1) {}
    virtual int is_visible() {
        if (this == active_popup_win) {
            ++popup_visibility_calls;
            events += 'V';
        }
        return visible;
    }
    virtual void hide() { ++base_hide_calls; events += 'W'; visible = 0; }
};
class GraphicWin : public Win {};

struct Scroll {
    int visible;
    Scroll() : visible(1) {}
    int is_visible() { ++scroll_query_calls; events += 'S'; return visible; }
};

class Popup : public GraphicWin {
public:
    uint32_t field_30AC_;
    GraphicWin *external_window_;
    Scroll scroll_;
    Popup() : field_30AC_(0), external_window_(0) {}
    void hide();
};

static GraphicWin external_window;

void scroll_hide(GraphicWin *window, Scroll *scroll) {
    ++scroll_hide_calls;
    events += 'H';
    if (window != static_cast<GraphicWin *>(active_popup) ||
        scroll != &active_popup->scroll_) helper_receiver_error = 1;
    if (mutate_in_scroll) {
        active_popup->field_30AC_ = 10000000U;
        active_popup->external_window_ = &external_window;
        active_popup->scroll_.visible = 0;
        active_popup->visible = 0;
        MainInterfaceGlobal.visible = 0;
    }
}
void slide_hide(GraphicWin *window, int mode) {
    ++slide_hide_calls;
    events += 'D';
    if (window != static_cast<GraphicWin *>(active_popup) || mode != 0)
        helper_receiver_error = 1;
}

BODY

static void reset_case(Popup &popup) {
    events.clear();
    active_popup = &popup;
    mutate_in_scroll = 0;
    scroll_hide_calls = slide_hide_calls = scroll_query_calls = 0;
    main_visibility_calls = popup_visibility_calls = base_hide_calls = 0;
    helper_receiver_error = 0;
    active_popup_win = static_cast<Win *>(&popup);
    popup.visible = 1;
    popup.field_30AC_ = 0;
    popup.external_window_ = 0;
    popup.scroll_.visible = 1;
    MainInterfaceGlobal.visible = 1;
}

static int expect_case(uint32_t selector, int popup_visible, int has_external,
                       int main_visible, int scroll_visible, int mutate_scroll,
                       const char *wanted_events, int wanted_main_calls,
                       int wanted_scroll_queries, int wanted_scroll_hides,
                       int wanted_slide_hides, int wanted_base_hides) {
    Popup popup;
    reset_case(popup);
    popup.field_30AC_ = selector;
    popup.visible = popup_visible;
    popup.external_window_ = has_external ? &external_window : 0;
    MainInterfaceGlobal.visible = main_visible;
    popup.scroll_.visible = scroll_visible;
    mutate_in_scroll = mutate_scroll;
    popup.hide();
    if (events != wanted_events || helper_receiver_error != 0 ||
        main_visibility_calls != wanted_main_calls ||
        scroll_query_calls != wanted_scroll_queries ||
        scroll_hide_calls != wanted_scroll_hides ||
        slide_hide_calls != wanted_slide_hides ||
        base_hide_calls != wanted_base_hides || popup_visibility_calls != 1)
        return 1;
    if (mutate_scroll && (popup.field_30AC_ != 10000000U ||
        popup.external_window_ != &external_window || popup.visible != 0 ||
        popup.scroll_.visible != 0 || MainInterfaceGlobal.visible != 0)) return 2;
    return 0;
}

static int test_selector_boundaries() {
    if (expect_case(0U, 1, 0, 1, 1, 0, "VISHDW", 1, 1, 1, 1, 1)) return 11;
    if (expect_case(9999U, 1, 0, 1, 1, 0, "VISHDW", 1, 1, 1, 1, 1)) return 12;
    if (expect_case(10000U, 1, 0, 1, 1, 0, "VIW", 1, 0, 0, 0, 1)) return 13;
    if (expect_case(49999U, 1, 0, 1, 1, 0, "VIW", 1, 0, 0, 0, 1)) return 14;
    if (expect_case(50000U, 1, 0, 1, 1, 0, "VISHDW", 1, 1, 1, 1, 1)) return 15;
    if (expect_case(59999U, 1, 0, 1, 1, 0, "VISHDW", 1, 1, 1, 1, 1)) return 16;
    if (expect_case(60000U, 1, 0, 1, 1, 0, "VIW", 1, 0, 0, 0, 1)) return 17;
    if (expect_case(10000000U, 1, 0, 1, 1, 0, "VW", 0, 0, 0, 0, 1)) return 18;
    if (expect_case(10009999U, 1, 0, 1, 1, 0, "VW", 0, 0, 0, 0, 1)) return 19;
    if (expect_case(10010000U, 1, 0, 1, 1, 0, "VIW", 1, 0, 0, 0, 1)) return 20;
    return 0;
}

static int test_visibility_and_overlay_gates() {
    if (expect_case(50000U, 0, 0, 1, 1, 0, "V", 0, 0, 0, 0, 0)) return 31;
    if (expect_case(50000U, 1, 1, 1, 1, 0, "VW", 0, 0, 0, 0, 1)) return 32;
    if (expect_case(50000U, 1, 0, 0, 1, 0, "VIW", 1, 0, 0, 0, 1)) return 33;
    if (expect_case(50000U, 1, 0, 1, 0, 0, "VISDW", 1, 1, 0, 1, 1)) return 34;
    return 0;
}

static int test_signed_truncation_and_high_bit_inputs() {
    if (expect_case(static_cast<uint32_t>(-1), 1, 0, 1, 1, 0,
                    "VISHDW", 1, 1, 1, 1, 1)) return 41;
    if (expect_case(static_cast<uint32_t>(-9999), 1, 0, 1, 1, 0,
                    "VISHDW", 1, 1, 1, 1, 1)) return 42;
    if (expect_case(static_cast<uint32_t>(-10000), 1, 0, 1, 1, 0,
                    "VIW", 1, 0, 0, 0, 1)) return 43;
    if (expect_case(0xFFFFD8F1U, 1, 0, 1, 1, 0,
                    "VISHDW", 1, 1, 1, 1, 1)) return 44;
    if (expect_case(0x80000000U, 1, 0, 1, 1, 0,
                    "VIW", 1, 0, 0, 0, 1)) return 45;
    return 0;
}

static int test_helper_mutation_does_not_reselect_hide_branch() {
    return expect_case(50000U, 1, 0, 1, 1, 1,
                       "VISHDW", 1, 1, 1, 1, 1);
}

int main() {
    int result = test_selector_boundaries();
    if (result != 0) return result;
    result = test_visibility_and_overlay_gates();
    if (result != 0) return result;
    result = test_signed_truncation_and_high_bit_inputs();
    if (result != 0) return result;
    result = test_helper_mutation_does_not_reselect_hide_branch();
    return result;
}
"""


def actual_body():
    source = (ROOT / "src/popup.cpp").read_text(encoding="utf-8")
    return extract_definition(source, "void Popup::hide()")


def old_divisor_mutant(body):
    anchor = "static_cast<int>(field_30AC_) / 10000"
    assert body.count(anchor) == 1
    return body.replace(anchor, "static_cast<int>(field_30AC_) / 1000", 1)


def compile_and_run(tmp_path, compiler, body):
    cpp = tmp_path / "popup_hide_selector.cpp"
    executable = tmp_path / "popup_hide_selector"
    cpp.write_text(HARNESS.replace("BODY", body), encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-O1", "-Wall", "-Wextra",
         "-fsanitize=undefined", "-fno-sanitize-recover=undefined",
         str(cpp), "-o", str(executable)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(executable)], capture_output=True,
                          text=True, timeout=10)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_popup_hide_selector_and_order(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} required")
    run = compile_and_run(tmp_path, compiler, actual_body())
    assert run.returncode == 0, run.stdout + run.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_old_1000_divisor_mutant_fails_selector_boundaries(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} required")
    run = compile_and_run(tmp_path, compiler, old_divisor_mutant(actual_body()))
    assert run.returncode != 0, "old /1000 divisor mutant survived selector cases"
