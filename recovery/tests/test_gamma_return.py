"""Exercise the actual Gamma::exec body with host-side UI collaborators.

This checks return values and cleanup sequencing, not Win32 ABI or game runtime.
"""
from pathlib import Path
import re
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


def actual_body():
    text = (ROOT / "src/gamma.cpp").read_text()
    start = re.search(r"(?:void|float|double) Gamma::exec\(", text).start()
    end = text.index("\n}\n", start) + 3
    header = (ROOT / "src/gamma.h").read_text()
    declaration = re.search(r"(?:void|float|double) exec\([^;]+;", header).group()
    return declaration, text[start:end]


STUBS = r'''
#include <cassert>
#include <cmath>
#include <string>
#include <vector>
struct RECT { int left, top, right, bottom; };
struct Palette {};
struct Font { template<class... A> void init(A...) {} };
struct Buffer {
    template<class... A> void set_font(A...) {}
};
struct Scroll {
    int field_A64_, field_A48_, paging_;
    void *thumb_sprite_;
    template<class... A> void init(A...) {}
    void set_border_color(int) {}
    void set_button_color(int) {}
    void set_bevel_thickness(int) {}
    void set_bevel_upper(int) {}
    void set_bevel_lower(int) {}
    void set_sprite_left(int,int,int) {}
    void set_sprite_right(int,int,int) {}
    void set_range(int,int) {}
    void set_pos(int) {}
    void show(int) {}
};
struct PushButton : Buffer {
    int color_, bevel_upper_, bevel_lower_, bevel_thickness_;
    int field_AC8_, field_AEC_, field_AC0_, field_AE4_, field_ACC_, field_AF0_;
    template<class... A> void init(A...) {}
    void redraw() {}
    void set_text_color(int,int,int,int) {}
};
struct GraphicWin {
    template<class... A> void init(A...) {}
    void fill(int) {}
};
int WinScreenWidth = 800, WinScreenHeight = 600;
const char *BasePopDefaultOkText = "OK", *BasePopDefaultCancelText = "Cancel";
const float GammaSliderScale = 100;
void do_all_draws() {}
double modal_value, cleanup_value;
std::vector<std::string> calls;
std::vector<double> adjustments;
struct Gamma : GraphicWin {
    Palette *target_palette_;
    float initialGamma_;
    double gamma_ = 1.0;
    RECT rect1_, rect2_, rect3_, rect4_, rect5_;
    Font font_;
    Scroll scroll_;
    PushButton pushButton1_, pushButton2_;
    DECLARATION
    void adjust_pal(double value) {
        calls.push_back("apply"); adjustments.push_back(value);
    }
    void show(int) { calls.push_back("show"); }
    void adjust_palette() { calls.push_back("ramp"); gamma_ = modal_value; }
    void redraw() {}
    void set_modal(int,int,int) { calls.push_back("modal"); }
    void hide() { calls.push_back("hide"); }
    void close() {
        calls.push_back("close"); target_palette_ = nullptr;
        // A collaborator mutation witnesses the return occurring after close,
        // rather than caching the member before cleanup. Real close is separate.
        gamma_ = cleanup_value;
    }
};
'''

HARNESS = r'''
int main() {
    Palette target;
    Gamma apply;
    double result = apply.exec(&target, 0.75f, 1);
    assert(result == 0.75);
    assert(apply.target_palette_ == &target);
    assert(apply.initialGamma_ == 0.75f);
    assert(calls == std::vector<std::string>{"apply"});
    assert(adjustments == std::vector<double>{0.75});

    calls.clear(); adjustments.clear();
    modal_value = 1.25;
    cleanup_value = 1.0 + std::ldexp(1.0, -40);
    Gamma dialog;
    result = dialog.exec(&target, 0.5f, 0);
    assert(result == cleanup_value); // Would fail for a float-return API.
    assert(dialog.target_palette_ == nullptr);
    assert(dialog.initialGamma_ == 0.5f);
    assert(calls == (std::vector<std::string>{
        "show", "ramp", "modal", "hide", "apply", "close"}));
    assert(adjustments == std::vector<double>{modal_value});
}
'''


def execute(tmp_path, body, label):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    declaration, _ = actual_body()
    text = STUBS.replace("DECLARATION", declaration) + body + HARNESS
    executable = tmp_path / label
    subprocess.run([compiler, "-std=c++17", "-x", "c++", "-", "-o", str(executable)],
                   input=text, text=True, capture_output=True, check=True)
    return subprocess.run([str(executable)], capture_output=True).returncode


def test_actual_exec_returns_initial_and_final_values_after_cleanup(tmp_path):
    _, body = actual_body()
    assert execute(tmp_path, body, "gamma-return") == 0


@pytest.mark.parametrize("before,after", [
    ("return initial_gamma;", "return gamma_;"),
    ("return gamma_;", "return initial_gamma;"),
    ("    close();\n    return gamma_;", "    return gamma_;\n    close();"),
])
def test_wrong_return_or_cleanup_order_is_detected(tmp_path, before, after):
    _, body = actual_body()
    assert before in body
    assert execute(tmp_path, body.replace(before, after, 1), "gamma-damaged") != 0
