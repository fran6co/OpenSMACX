"""Execute the product SelectPartWin navigation/feedback methods on a host harness."""

import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


def _body(source: str, prefix: str) -> str:
    start = source.index(prefix)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start : index + 1]
    raise AssertionError(f"unclosed {prefix}")


def test_selectpartwin_keyboard_and_feedback(tmp_path: Path) -> None:
    source = (ROOT / "src/selectpartwin.cpp").read_text()
    methods = "\n".join(
        _body(source, prefix)
        for prefix in (
            "int SelectPartWin::on_key_event(int, int key)",
            "void SelectPartWin::on_redraw()",
            "void SelectPartWin::on_mouse_feedback(int x, int y)",
            "void SelectPartWin::vslot_21(int x, int y)",
            "void SelectPartWin::vslot_48(int, int row)",
            "void SelectPartWin::vslot_49(int a1, int a2)",
        )
    )
    harness = r"""
#include <cassert>
struct Scroll {
    int position_ = 0;
    int paging_ = 3;
    int maximum_ = 2;
    void set_pos(int requested) {
        position_ = requested < 0 ? 0 : requested > maximum_ ? maximum_ : requested;
    }
};
struct SelectPartWin {
    Scroll scroll_;
    int field_2B60_ = 0;
    int field_2B64_ = 8;
    int field_2B6C_ = 4;
    int field_2B70_ = 2;
    int field_2B74_ = 20;
    int field_2B80_ = 2;
    int field_4648_ = 0;
    int redraws = 0;
    int updates = 0;
    int releases = 0;
    int capture_losses = 0;
    int drawn[32] = {};
    int selected[32] = {};
    int draw_count = 0;
    int draw_part(int index, int select) {
        drawn[draw_count] = index;
        selected[draw_count++] = select;
        return 1;
    }
    void redraw() { ++redraws; }
    void on_update() { ++updates; }
    void release_modal() { ++releases; }
    void on_lose_mouse_capture() { ++capture_losses; }
    int on_key_event(int, int);
    void on_redraw();
    void on_mouse_feedback(int, int);
    void vslot_21(int, int);
    void vslot_48(int, int);
    void vslot_49(int, int);
};
SelectPartWin *WinPointerOwner3 = nullptr;
SelectPartWin *WinPointerOwner4 = nullptr;
""" + methods + r"""
int main() {
    SelectPartWin win;
    // The first argument is not the key; only the second selects Escape.
    assert(win.on_key_event(27, 0) == 1);
    assert(win.releases == 0 && win.field_2B80_ == 2);
    assert(win.on_key_event(0, 39) == 1);  // right
    assert(win.field_2B80_ == 3);
    assert(win.draw_count == 2 && win.drawn[0] == 2 && win.selected[0] == 0);
    assert(win.drawn[1] == 3 && win.selected[1] == 1);
    assert(win.updates == 2);
    assert(win.on_key_event(0, 34) == 1);  // page down, Scroll clamps 0+3 to 2
    assert(win.scroll_.position_ == 2 && win.field_2B60_ == 8);
    assert(win.redraws == 1);
    win.vslot_49(999, 1);  // scrolling delegates to scrolled, second arg is row
    assert(win.field_2B60_ == 4 && win.redraws == 2);
    win.on_mouse_feedback(5, 5);
    assert(win.field_2B80_ == 4 && win.field_4648_ == 1);
    assert(win.updates == 4);
    WinPointerOwner3 = &win;
    WinPointerOwner4 = &win;
    win.vslot_21(-1, -1);
    assert(win.field_2B80_ == -1 && win.field_4648_ == 0);
    assert(WinPointerOwner3 == nullptr && WinPointerOwner4 == nullptr);
    assert(win.capture_losses == 1 && win.releases == 1);
    assert(win.on_key_event(0, 27) == 1);  // Escape
    assert(win.field_2B80_ == -1 && win.releases == 2);
    assert(win.updates == 5);
    assert(win.on_key_event(0, 26) == 1);  // default, not modal release
    assert(win.releases == 2);
    assert(win.updates == 6);
    assert(win.on_key_event(0, 13) == 1);  // Enter
    assert(win.releases == 3);
    assert(win.updates == 7);

    SelectPartWin fits;
    fits.field_2B74_ = fits.field_2B64_;
    fits.field_2B80_ = 0;
    assert(fits.on_key_event(0, 33) == 1);  // page up returns directly
    assert(fits.on_key_event(0, 34) == 1);  // page down returns directly
    assert(fits.on_key_event(0, 38) == 1);  // up at first row returns directly
    fits.field_2B80_ = 7;
    assert(fits.on_key_event(0, 40) == 1);  // down at end returns directly
    assert(fits.updates == 0 && fits.redraws == 0 && fits.draw_count == 0);

    SelectPartWin advance;
    advance.field_2B80_ = 4;
    assert(advance.on_key_event(0, 40) == 1);  // down advances selection
    assert(advance.field_2B80_ == 8 && advance.field_2B60_ == 4);
    assert(advance.scroll_.position_ == 1 && advance.redraws == 1);
    advance.field_2B80_ = 19;
    assert(advance.on_key_event(0, 40) == 1);  // down near end scrolls only
    assert(advance.field_2B80_ == 19 && advance.field_2B60_ == 8);
    assert(advance.scroll_.position_ == 2 && advance.redraws == 2);
}
"""
    source_path = tmp_path / "selectpart.cpp"
    binary = tmp_path / "selectpart"
    source_path.write_text(harness)
    subprocess.run(
        ["g++", "-std=c++17", "-Wall", "-Wextra", "-Werror", str(source_path), "-o", str(binary)],
        check=True,
    )
    subprocess.run([str(binary)], check=True)
