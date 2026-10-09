"""Actual-source controls for typed MainInterface/Buffer access paths.

The host harness checks the live member/API expressions and rejects plausible
wrong-owner substitutions. It does not establish VC6 layout or game runtime
acceptance.
"""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def body(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError(f"unterminated body: {signature}")


def compile_and_run(tmp_path: Path, source: str) -> subprocess.CompletedProcess[str]:
    compiler = shutil.which("c++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    cpp = tmp_path / "interface_buffer_control.cpp"
    exe = tmp_path / "interface_buffer_control"
    cpp.write_text(source)
    built = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
        text=True, capture_output=True, check=False,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(exe)], text=True, capture_output=True, check=False)


HARNESS = r'''#include <cstdint>
#include <cstdlib>
struct RECT { int left, top, right, bottom; };
struct Buffer {
    RECT rect2_{10, 20, 110, 120};
    RECT *active_clip = nullptr;
    uint32_t wrap_width = 0;
    uint32_t palette_index = 0;
    void set_clip(RECT *rect) { active_clip = rect; }
    void restore_clip() { set_clip(&rect2_); }
    void set_wrap_width(uint32_t value) { wrap_width = value; }
    void set_palette_index(uint32_t value) { palette_index = value; }
};
struct MainInterface : Buffer {
    RECT main_frame_{1, 2, 3, 4};
    RECT right_frame_{31, 32, 33, 34};
    RECT panel_rect2_{71, 72, 73, 74};
    const RECT &main_frame_rect() const { return main_frame_; }
    const RECT &right_frame_rect() const { return right_frame_; }
    const RECT &panel_rect2() const { return panel_rect2_; }
} MainInterfaceGlobal;
int battle_clip_left() { return __BATTLE_EXPR__; }
void design_restore_clip() {
    Buffer *canvas = static_cast<Buffer *>(&MainInterfaceGlobal);
    __DESIGN_STMT__
}
void social_set_state() {
    Buffer *buffer = static_cast<Buffer *>(&MainInterfaceGlobal);
    __SOCIAL_STMT__
}
static void check(bool value) { if (!value) std::abort(); }
int main() {
    check(battle_clip_left() == 31);
    design_restore_clip();
    check(MainInterfaceGlobal.active_clip == &MainInterfaceGlobal.rect2_);
    social_set_state();
    check(MainInterfaceGlobal.wrap_width == 5);
}
'''


def make_harness(battle: str, design: str, social: str) -> str:
    return (HARNESS.replace("__BATTLE_EXPR__", battle)
            .replace("__DESIGN_STMT__", design)
            .replace("__SOCIAL_STMT__", social))


def test_live_interface_buffer_consumers_and_wrong_owner_mutants(tmp_path: Path) -> None:
    battle = (ROOT / "src/battlewin.cpp").read_text()
    design = (ROOT / "src/designwin.cpp").read_text()
    social = (ROOT / "src/socialwin.cpp").read_text()

    status = body(battle, "void BattleWin::on_status(int mode)")
    stack = body(design, "void DesignWin::draw_stack()")
    flash = body(design, "void DesignWin::draw_flash()")
    overview = body(social, "void SocialWin::draw_overview()")

    battle_expr = "MainInterfaceGlobal.right_frame_rect().left"
    assert f"0, 0, {battle_expr}," in status
    assert status.index("set_clip(") < status.index("restore_back(0)")
    assert "static_cast<Buffer *>(&MainInterfaceGlobal)" in stack
    assert "static_cast<Buffer *>(&MainInterfaceGlobal)" in flash
    assert stack.count("canvas->restore_clip();") == 4
    assert flash.count("canvas->restore_clip();") == 1
    assert "canvas->set_clip(reinterpret_cast<RECT *>(InterfaceClipRect))" not in stack + flash
    assert overview.count("buffer->set_wrap_width(5);") == 2

    live_code = "\n".join(
        line for text in (battle, design, social) for line in text.splitlines()
        if not line.lstrip().startswith(("//", "*"))
    )
    for retired in ("InterfaceScreenRect", "InterfaceBuffer", "InterfaceClipRect",
                    "InterfacePaletteIndex"):
        assert retired not in live_code

    positive = compile_and_run(
        tmp_path, make_harness(battle_expr, "canvas->restore_clip();",
                               "buffer->set_wrap_width(5);"))
    assert positive.returncode == 0, positive.stdout + positive.stderr

    mutants = (
        make_harness("MainInterfaceGlobal.main_frame_rect().left",
                     "canvas->restore_clip();", "buffer->set_wrap_width(5);"),
        make_harness(battle_expr,
                     "canvas->set_clip(const_cast<RECT *>(&MainInterfaceGlobal.panel_rect2()));",
                     "buffer->set_wrap_width(5);"),
        make_harness(battle_expr, "canvas->restore_clip();",
                     "buffer->set_palette_index(5);"),
    )
    for mutant in mutants:
        rejected = compile_and_run(tmp_path, mutant)
        assert rejected.returncode != 0, "wrong-owner mutant unexpectedly passed"
