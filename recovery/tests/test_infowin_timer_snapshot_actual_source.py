"""Host controls for the actual InfoWin timer_proc body.

The harness uses typed doubles for the required owner members and callback
boundaries. It verifies member behavior, not original ABI or object layout.
"""
from pathlib import Path
import resource
import shutil
import subprocess

import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src" / "infowin.cpp"
SIGNATURE = "void InfoWin::timer_proc(int a1) {"


def actual_method() -> str:
    source = SOURCE.read_text(encoding="utf-8")
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " "
                               for char in match[0]), source)
    assert masked.count(SIGNATURE) == 1, "expected one installed timer_proc body"
    start = masked.index(SIGNATURE)
    opening = masked.index("{", start)
    depth = 0
    for pos in range(opening, len(masked)):
        if masked[pos] == "{":
            depth += 1
        elif masked[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[start:pos + 1]
    raise AssertionError("unterminated InfoWin::timer_proc")


HARNESS = r"""
#include <cassert>

struct RECT { int left, top, right, bottom; };
struct InfoWin;
struct FlatButtonProbe {
    int visible;
    int is_visible() { return visible; }
};
struct MainInterfaceProbe {
    FlatButtonProbe flat_button_[12];
    InfoWin *owner;
    int mouse_calls;
    int switch_on_mouse;
    int mouse_pane;
    int mouse_length;
    int get_mouse_pos(int *x, int *y);
};
static MainInterfaceProbe MainInterfaceGlobal;
static int IfaceIdToSlotStorage[30];
static int ActiveIfaceIdStorage;
static int box_result;
static int box_switch_pane;
static int box_length;
static int box_calls;
static int timer_change_calls;
static int timer_change_arg;
static int draw_calls;
static int draw_arg;

struct InfoWin {
    int field_AC_;
    int active_pane_;
    int visible_characters_[2];
    int text_lengths_[2];
    int field_A8_;
    int field_94_;
    RECT bounds_;

    void draw_text(int pane);
    void timer_change(int tick);
    void timer_proc(int tick);
};

int MainInterfaceProbe::get_mouse_pos(int *x, int *y) {
    ++mouse_calls;
    *x = 17;
    *y = 23;
    if (switch_on_mouse) {
        owner->active_pane_ = mouse_pane;
        owner->text_lengths_[owner->active_pane_] = mouse_length;
        // The old pane changes after its threshold values have been read.
        owner->text_lengths_[0] = 1000;
        owner->visible_characters_[0] = 0;
    }
    return 0;
}
int in_box(int, int, const RECT *) {
    ++box_calls;
    if (MainInterfaceGlobal.owner != 0 && box_switch_pane >= 0) {
        MainInterfaceGlobal.owner->active_pane_ = box_switch_pane;
        MainInterfaceGlobal.owner->text_lengths_[box_switch_pane] = box_length;
    }
    return box_result;
}
void InfoWin::draw_text(int pane) {
    ++draw_calls;
    draw_arg = pane;
}
void InfoWin::timer_change(int tick) {
    ++timer_change_calls;
    timer_change_arg = tick;
}

// ACTUAL_METHOD

static void reset(InfoWin &info) {
    info.field_AC_ = 0;
    info.active_pane_ = 0;
    info.visible_characters_[0] = 0;
    info.visible_characters_[1] = 0;
    info.text_lengths_[0] = 0;
    info.text_lengths_[1] = 0;
    info.field_A8_ = 0;
    info.field_94_ = 0;
    info.bounds_.left = 1;
    info.bounds_.top = 2;
    info.bounds_.right = 3;
    info.bounds_.bottom = 4;
    MainInterfaceGlobal.owner = &info;
    MainInterfaceGlobal.mouse_calls = 0;
    MainInterfaceGlobal.switch_on_mouse = 0;
    MainInterfaceGlobal.mouse_pane = 0;
    MainInterfaceGlobal.mouse_length = 0;
    for (int i = 0; i != 12; ++i)
        MainInterfaceGlobal.flat_button_[i].visible = 0;
    for (int i = 0; i != 30; ++i) IfaceIdToSlotStorage[i] = 0;
    ActiveIfaceIdStorage = 4;
    IfaceIdToSlotStorage[0] = 1; // The active index, not the first slot, controls entry.
    box_result = 0;
    box_switch_pane = -1;
    box_length = 0;
    box_calls = 0;
    timer_change_calls = 0;
    timer_change_arg = -1;
    draw_calls = 0;
    draw_arg = -99;
}

int main() {
    InfoWin info;

    // Early gates preserve order and do not advance either pane.
    reset(info);
    info.field_AC_ = 1;
    info.visible_characters_[0] = 8;
    info.timer_proc(13);
    assert(info.visible_characters_[0] == 8);
    assert(MainInterfaceGlobal.mouse_calls == 0 && draw_calls == 0);

    reset(info);
    IfaceIdToSlotStorage[ActiveIfaceIdStorage] = 1;
    info.timer_proc(13);
    assert(info.visible_characters_[0] == 0);
    assert(MainInterfaceGlobal.mouse_calls == 0 && draw_calls == 0);

    // The pin tests the full DWORD slot; its low byte need not be nonzero.
    reset(info);
    IfaceIdToSlotStorage[ActiveIfaceIdStorage] = 0x100;
    info.timer_proc(13);
    assert(info.visible_characters_[0] == 0);
    assert(MainInterfaceGlobal.mouse_calls == 0 && draw_calls == 0);

    reset(info);
    MainInterfaceGlobal.flat_button_[11].visible = 1;
    info.timer_proc(13);
    assert(info.visible_characters_[0] == 0);
    assert(MainInterfaceGlobal.mouse_calls == 0 && draw_calls == 0);

    // Ordinary count path keeps the pre-call active pane and draws it only
    // while count <= the original text length plus one.
    reset(info);
    info.active_pane_ = 0;
    info.visible_characters_[0] = 0;
    info.text_lengths_[0] = 0;
    info.timer_proc(13);
    assert(info.visible_characters_[0] == 1);
    assert(draw_calls == 1 && draw_arg == 0);
    assert(MainInterfaceGlobal.mouse_calls == 0);

    // Threshold uses pane 0 count/length captured before callbacks. Both
    // callbacks then select pane 1 and set its current text length to 41.
    // The clamp writes 41+0x28 to pane 1; it must not use cached pane 0 or
    // cached length 0. field_94_ toggles and full-pane redraw remain intact.
    reset(info);
    info.active_pane_ = 0;
    info.visible_characters_[0] = 51;
    info.text_lengths_[0] = 0;
    info.visible_characters_[1] = 7;
    MainInterfaceGlobal.switch_on_mouse = 1;
    MainInterfaceGlobal.mouse_pane = 0;
    MainInterfaceGlobal.mouse_length = 19;
    box_result = 1;
    box_switch_pane = 1;
    box_length = 41;
    info.timer_proc(13);
    assert(info.visible_characters_[1] == 81);
    assert(info.text_lengths_[0] == 1000);
    assert(info.text_lengths_[1] == 41);
    assert(info.active_pane_ == 1 && info.field_94_ == 1);
    assert(MainInterfaceGlobal.mouse_calls == 1 && box_calls == 1);
    assert(draw_calls == 1 && draw_arg == -1);
    assert(timer_change_calls == 0);

    // The field_94_ fallback remains part of the clamp test even when the
    // box check misses, and its old value toggles back to zero.
    reset(info);
    info.visible_characters_[0] = 51;
    info.text_lengths_[0] = 0;
    info.field_94_ = 1;
    box_result = 0;
    info.timer_proc(29);
    assert(info.visible_characters_[0] == 40);
    assert(info.field_94_ == 0 && draw_arg == -1 && draw_calls == 1);

    // A threshold overrun with no clamp condition advances to timer_change.
    reset(info);
    info.visible_characters_[0] = 52;
    info.text_lengths_[0] = 0;
    info.timer_proc(29);
    assert(info.visible_characters_[0] == 53);
    assert(timer_change_calls == 1 && timer_change_arg == 29);
    assert(draw_calls == 0 && MainInterfaceGlobal.mouse_calls == 1);
}
"""


def _compile_run(tmp_path: Path, compiler: str, body: str):
    source_path = tmp_path / "infowin_timer_proc.cpp"
    executable = tmp_path / "infowin_timer_proc"
    source_path.write_text(HARNESS.replace("// ACTUAL_METHOD", body),
                           encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-pedantic",
         str(source_path), "-o", str(executable)],
        capture_output=True, text=True, cwd=tmp_path, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run(
        [str(executable)], capture_output=True, cwd=tmp_path, timeout=10,
        preexec_fn=lambda: resource.setrlimit(resource.RLIMIT_CORE, (0, 0)))


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_timer_proc_retains_callback_boundary_reads(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    result = _compile_run(tmp_path, compiler, actual_method())
    assert result.returncode == 0, result.stderr.decode()


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_stale_pane_and_length_mutant_fails_the_callback_witness(
    tmp_path, compiler_name
):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    body = actual_method()
    fresh = (
        "            const int current_pane = active_pane_;\n"
        "            visible_characters_[current_pane] =\n"
        "                text_lengths_[current_pane] + 0x28;"
    )
    stale = "            visible_characters_[pane] = length + 0x28;"
    mutant = body.replace(fresh, stale, 1)
    assert mutant != body, "actual source lacks the admitted post-callback reload"
    result = _compile_run(tmp_path, compiler, mutant)
    diagnostic = result.stderr.decode().lower()
    assert result.returncode != 0
    assert "assert" in diagnostic and "visible_characters" in diagnostic, (
        "mutant must fail the intended stale-pane assertion, not crash for an "
        "unrelated reason:\n" + diagnostic)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("damage", ("omit", "wrong-index", "low-byte"))
def test_interface_slot_guard_damage_is_rejected(tmp_path, compiler_name, damage):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    body = actual_method()
    guard = "if (IfaceIdToSlotStorage[ActiveIfaceIdStorage] != 0) return;"
    replacements = {
        "omit": "",
        "wrong-index": "if (IfaceIdToSlotStorage[0] != 0) return;",
        "low-byte": "if (static_cast<unsigned char>(IfaceIdToSlotStorage[ActiveIfaceIdStorage]) != 0) return;",
    }
    mutant = body.replace(guard, replacements[damage], 1)
    assert mutant != body, "actual source lacks the admitted interface-slot guard"
    result = _compile_run(tmp_path, compiler, mutant)
    diagnostic = result.stderr.decode().lower()
    assert result.returncode != 0
    assert "assert" in diagnostic and "visible_characters" in diagnostic, diagnostic
