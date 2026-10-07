"""Actual-source Popup callback controls, not native layout proof."""
from pathlib import Path
import shutil
import subprocess

import pytest

from test_listbox_dimensions_actual_source import extract_definition

ROOT = Path(__file__).resolve().parents[2]
WIDTH = "void Popup::on_adjust_button_width()"
HIT = "int Popup::on_nc_hittest(int x, int y)"
SUPPORT = r'''
#include <cassert>
#include <stdint.h>
struct Win {
    int calls, last_x, last_y;
    const Win *last_receiver;
    Win() : calls(0), last_x(0), last_y(0), last_receiver(0) {}
    virtual int on_nc_hittest(int x, int y) {
        ++calls; last_x = x; last_y = y; last_receiver = this;
        return x * 1000 + y;
    }
};
struct GraphicWin : public Win {};
struct BasePop : public GraphicWin {
    uint32_t field_30AC_, field_30D0_;
    BasePop() : field_30AC_(0), field_30D0_(0) {}
    virtual void on_adjust_button_width() {}
    void recalculate_width() { on_adjust_button_width(); }
};
struct Popup : public BasePop {
    void on_adjust_button_width();
    int on_nc_hittest(int x, int y);
};
/* WIDTH */
/* HIT */
int main() {
    Popup popup;
    const uint32_t values[] = {10000000u, 10009999u, 9999999u, 10010000u,
                              static_cast<uint32_t>(-10000), 0u};
    BasePop *base = &popup;
    for (int i = 0; i != 6; ++i) {
        popup.field_30AC_ = values[i]; popup.field_30D0_ = 77;
        base->recalculate_width();
        assert(popup.field_30AC_ == values[i]);
        assert(popup.field_30D0_ == (i < 2 ? 77u : 20u));
    }
    Win *window = &popup;
    assert(window->on_nc_hittest(7, 31) == 7031);
    assert(popup.calls == 1 && popup.last_x == 7 && popup.last_y == 31);
    assert(popup.last_receiver == static_cast<Win *>(&popup));
}
'''


def run_control(directory, width, hit, compiler):
    directory.mkdir()
    source, binary = directory / "control.cpp", directory / "control"
    source.write_text(SUPPORT.replace("/* WIDTH */", width).replace("/* HIT */", hit))
    subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                    str(source), "-o", str(binary)], check=True,
                   capture_output=True, text=True, timeout=30)
    return subprocess.run([str(binary)], capture_output=True, timeout=10)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_popup_actual_source_and_damage_cases(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host compiler unavailable")
    source = (ROOT / "src/popup.cpp").read_text()
    width, hit = (extract_definition(source, signature) for signature in (WIDTH, HIT))
    assert "reinterpret_cast" not in width + hit
    assert "static_cast<int32_t>(field_30AC_) / 10000" in width
    assert "friend class Popup;" in (ROOT / "src/basepop.h").read_text()
    assert run_control(tmp_path / "product", width, hit, compiler).returncode == 0
    damages = (
        (width.replace("field_30AC_", "field_30D0_", 1), hit),
        (width.replace("scaled != 1000", "scaled == 1000"), hit),
        (width.replace("field_30D0_ = 20", "field_30D0_ = 21"), hit),
        (width, hit.replace("Win::on_nc_hittest(x, y)", "Win::on_nc_hittest(y, x)")),
        (width, hit.replace("Win::on_nc_hittest(x, y)", "Win::on_nc_hittest(x, y) + 1")),
    )
    for index, (damaged_width, damaged_hit) in enumerate(damages):
        assert (damaged_width, damaged_hit) != (width, hit)
        assert run_control(tmp_path / ("damage-" + str(index)), damaged_width,
                           damaged_hit, compiler).returncode != 0
