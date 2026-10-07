"""Actual-body phase, dispatch and mutation controls for the splash window."""
from pathlib import Path
import shutil

import pytest
from test_leaf_neutral_views_actual_source import compile_run, extract

ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/IMAGE_PIN").is_file())
SOURCE = ROOT / "src/game.cpp"

PREFIX = r'''#include <assert.h>
struct Demo {
    int timer_phase_;
    int calls_;
    int seen_phase_;
    virtual void redraw() { assert(false); }
    void toggle_and_repaint();
};
struct DerivedDemo : Demo {
    virtual void redraw() { ++calls_; seen_phase_ = timer_phase_; }
};
'''
MAIN = r'''int main() {
    DerivedDemo d;
    d.calls_ = 0;
    int values[] = {0, 1, 7, -1, (-2147483647-1)};
    for (int i = 0; i < 5; ++i) {
        d.timer_phase_ = values[i];
        d.seen_phase_ = -7;
        d.toggle_and_repaint();
        const int expected = values[i] == 0 ? 1 : 0;
        assert(d.timer_phase_ == expected);
        assert(d.seen_phase_ == expected);
        assert(d.calls_ == i + 1);
    }
    return 0;
}
'''


def test_known_demo_owner_retains_original_identity_and_drops_fake_layout():
    from match_recovery.source import read_file
    source = SOURCE.read_text()
    row, = [row for row in read_file(SOURCE) if row.address == 0x524320]
    assert row.name == "sub_524320" and row.byte_exact
    assert row.image_spans == ((0x524320, 0x52433B),)
    assert row.symbol == "?toggle_and_repaint@Demo@@QAEXXZ"
    assert "RECOVERY_MEMBER: v1 Demo timer_phase_ 0xA18 0x00524340" in source
    assert "struct Base524320" not in (ROOT / "src/hypothesis_layouts.h").read_text()
    body = extract(source, "void Demo::toggle_and_repaint()")
    assert "reinterpret_cast" not in body
    assert "Slot524320" not in source


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None, "wrong_toggle", "omit_redraw",
                                    "redraw_before_toggle", "qualified_dispatch"))
def test_actual_body_virtual_dispatch_and_phase_order(compiler_name, damage):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " is unavailable")
    body = extract(SOURCE.read_text(), "void Demo::toggle_and_repaint()")
    if damage == "wrong_toggle":
        body = body.replace("timer_phase_ == 0", "timer_phase_ != 0", 1)
    elif damage == "omit_redraw":
        body = body.replace("    redraw();", "", 1)
    elif damage == "redraw_before_toggle":
        body = body.replace("    timer_phase_ = (timer_phase_ == 0);\n    redraw();",
                            "    redraw();\n    timer_phase_ = (timer_phase_ == 0);", 1)
    elif damage == "qualified_dispatch":
        body = body.replace("    redraw();", "    Demo::redraw();", 1)
    compile_run(compiler, PREFIX + body + MAIN, expected=damage is None)
