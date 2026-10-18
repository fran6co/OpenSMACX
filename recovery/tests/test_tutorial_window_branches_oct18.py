"""Actual-source host regressions for TutWin::tut_win branch/placement behavior.

The full mapped C++ body is compiled with bounded host doubles. This checks
source-level effects, not VC6 ABI, SEH, Win32 drawing, or game runtime behavior.
"""

from __future__ import annotations

import hashlib
import os
from pathlib import Path
import shutil
import subprocess

from capstone import CS_ARCH_X86, CS_MODE_32, Cs
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
import pytest

from test_tutorial_script_provider_oct18 import _disassemble, _expect, _method_body


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
ORIGINAL_SPAN = (0x4BDFE0, 0x4BE643)
METHOD_MARKER = "/* ACTUAL_TUTWIN_TUT_WIN */"


HARNESS = r'''#include <cassert>
#include <vector>

struct Sprite { int id; };
enum {
    E_POPUPS_TUTORIAL = 1, E_RESET, E_START, E_OK, E_CANCEL, E_CALLER_PICTURE,
    E_SPRITE, E_VEH_CALLBACK, E_BASE_CALLBACK, E_TERRAIN_CALLBACK, E_EXEC1,
    E_EXEC0, E_LINE, E_SET_LOC,
    E_POPUPS_NORMAL, E_TOUR, E_KEY
};
static std::vector<int> events, callbacks, exec_modes;
static std::vector<Sprite *> caller_pictures, displayed_sprites;
static std::vector<int> start_flags, locations;
struct GraphicWin {
    int lines;
    int x1, y1, x2, y2, color, on, off;
    GraphicWin() : lines(0), x1(0), y1(0), x2(0), y2(0), color(0), on(0), off(0) { }
    void window_line_raw(int ax, int ay, int bx, int by, int c, int p, int q) {
        ++lines; x1 = ax; y1 = ay; x2 = bx; y2 = by; color = c; on = p; off = q;
        events.push_back(E_LINE);
    }
};
typedef void (*DrawCallback)(Sprite *, GraphicWin *, int, int, int);
struct RECT { int left, top, right, bottom; };
static int WinScreenWidth = 1024;
static int WinScreenHeight = 768;
static int TutWinShownFlag = 0;
static int PopupThemeWindowColor = 0x555555;
static int configured_width = 490;
static int configured_height = 211;
static int publish_modal_result = 0;
static Sprite g_VEH_SPRITES[1] = {{1}};
static Sprite g_TER1_TUT_BLANK_SPRITE = {2};
static char TutorialPopupCaption[] = "TUTOR";
static char ok_label[] = "OK", cancel_label[] = "CANCEL";

struct LabelsTable {
    char *get(char *) { return ok_label; }
} StringTable;
static char *tutorial_button_label(unsigned int offset) {
    return offset == 0x3AC ? cancel_label : ok_label;
}
struct ConsoleStub {
    void on_key_click(int, int) { events.push_back(E_KEY); }
} ConsoleGlobal;

struct SpriteBox {
    void sprite(Sprite *picture, int, int) {
        displayed_sprites.push_back(picture);
        events.push_back(E_SPRITE);
    }
};
struct Dialogs : public SpriteBox { };

static void tut_veh_draw(Sprite *, GraphicWin *, int, int, int) { }
static void tut_terrain_draw(Sprite *, GraphicWin *, int, int, int) { }

struct TutWin {
    int field_53D0_, field_53C4_, field_5384_, field_5388_, field_538C_;
    int field_53AC_, field_537C_, field_5390_, field_5394_, field_5398_, field_53A8_;
    int field_539C_, field_53A0_, field_53B8_, field_53A4_, field_53B4_;
    int field_53BC_, field_53C8_, field_53CC_;
    GraphicWin *field_53D4_;
    bool picture_is_enabled;
    Dialogs dialog;
    TutWin() : field_53D0_(0), field_53C4_(0), field_5384_(0), field_5388_(0), field_538C_(0),
        field_53AC_(-1), field_537C_(-1), field_5390_(0), field_5394_(-1),
        field_5398_(-1), field_53A8_(0), field_539C_(-1), field_53A0_(0), field_53B8_(0),
        field_53A4_(0), field_53B4_(73), field_53BC_(0), field_53C8_(0),
        field_53CC_(0), field_53D4_(0), picture_is_enabled(false) { }
    void popups_tutorial() { events.push_back(E_POPUPS_TUTORIAL); }
    void popups_normal() { events.push_back(E_POPUPS_NORMAL); }
    int reset() { events.push_back(E_RESET); return 0; }
    void start(char *, const char *, int, int, int flags, int) {
        start_flags.push_back(flags); events.push_back(E_START);
    }
    void set_ok_text(char *) { events.push_back(E_OK); }
    void set_cancel_text(char *) { events.push_back(E_CANCEL); }
    bool picture_enabled() const { return picture_is_enabled; }
    void set_caller_picture(Sprite *picture) {
        caller_pictures.push_back(picture); events.push_back(E_CALLER_PICTURE);
    }
    Dialogs &dialogs() { return dialog; }
    void set_redraw_callback(DrawCallback callback) {
        if (callback == tut_veh_draw) { callbacks.push_back(1); events.push_back(E_VEH_CALLBACK); }
        else if (callback == TutWin::base_draw) { callbacks.push_back(2); events.push_back(E_BASE_CALLBACK); }
        else if (callback == tut_terrain_draw) { callbacks.push_back(3); events.push_back(E_TERRAIN_CALLBACK); }
        else { callbacks.push_back(99); }
    }
    void exec(int first, int) {
        exec_modes.push_back(first);
        events.push_back(first ? E_EXEC1 : E_EXEC0);
        if (!first && publish_modal_result) {
            field_53BC_ = 1;
            field_53C4_ = 1;
            field_53C8_ = 0x41;
            field_53CC_ = 0x80;
        }
    }
    int layout_width() { return configured_width; }
    int layout_height() { return configured_height; }
    void set_loc(int x, int y) {
        locations.push_back(x); locations.push_back(y); events.push_back(E_SET_LOC);
    }
    void tour() { events.push_back(E_TOUR); }
    static void base_draw(Sprite *, GraphicWin *, int, int, int) { }
    int tut_win(GraphicWin *owner, const char *text, int x, int y,
                Sprite *picture, int flags, int arrow_x, int arrow_y);
};

/* ACTUAL_TUTWIN_TUT_WIN */

static void reset_observations() {
    events.clear(); callbacks.clear(); exec_modes.clear(); caller_pictures.clear();
    displayed_sprites.clear(); start_flags.clear(); locations.clear();
    publish_modal_result = 0;
}

static void assert_two_passes() {
    assert(exec_modes.size() == 2 && exec_modes[0] == 1 && exec_modes[1] == 0);
    assert(start_flags.size() == 2 && start_flags[0] == start_flags[1]);
    assert(events.front() == E_POPUPS_TUTORIAL);
    assert(events[1] == E_RESET);
    assert(events[2] == E_START);
    assert(events.back() == E_POPUPS_NORMAL);
}

static void assert_events(const std::vector<int> &expected) {
    assert(events.size() == expected.size());
    for (unsigned int i = 0; i < expected.size(); ++i) assert(events[i] == expected[i]);
}

static int run_callback_cases() {
    GraphicWin owner;
    Sprite explicit_picture = {3};
    struct Case { int chassis, terrain, callback; Sprite *first_sprite; bool explicit_pic; };
    const Case cases[] = {
        {-1, -1, 0, 0, false},             // -1 is a sentinel, not the base-draw range.
        {-1, 0, 3, &g_TER1_TUT_BLANK_SPRITE, false},
        {-2, -1, 2, &g_VEH_SPRITES[0], false},
        {0, -1, 1, &g_VEH_SPRITES[0], false},
        {-2, 0, 0, &explicit_picture, true},
    };
    for (unsigned int i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        reset_observations();
        TutWin window;
        window.field_53AC_ = cases[i].chassis;
        window.field_537C_ = cases[i].terrain;
        const int result = window.tut_win(&owner, "branch", 640, 480,
                                          cases[i].explicit_pic ? &explicit_picture : 0,
                                          0, -1, -1);
        assert(result == 73);
        assert_two_passes();
        if (cases[i].callback == 0) assert(callbacks.empty());
        else assert(callbacks.size() == 2 && callbacks[0] == cases[i].callback
                    && callbacks[1] == cases[i].callback);
        if (cases[i].first_sprite == 0) {
            assert(displayed_sprites.empty() && caller_pictures.empty());
        } else {
            assert(displayed_sprites.size() == 2 && caller_pictures.size() == 2);
            assert(displayed_sprites[0] == cases[i].first_sprite);
            assert(displayed_sprites[1] != 0);
            if (cases[i].explicit_pic) assert(displayed_sprites[1] == &explicit_picture);
            if (window.picture_is_enabled) {
                assert(caller_pictures[0] == 0 && caller_pictures[1] == 0);
            } else {
                assert(caller_pictures[0] == displayed_sprites[0]);
                assert(caller_pictures[1] == displayed_sprites[1]);
            }
        }
        std::vector<int> expected;
        expected.push_back(E_POPUPS_TUTORIAL); expected.push_back(E_RESET);
        expected.push_back(E_START); expected.push_back(E_OK);
        if (cases[i].first_sprite != 0) {
            expected.push_back(E_CALLER_PICTURE); expected.push_back(E_SPRITE);
            if (cases[i].callback != 0) {
                expected.push_back(cases[i].callback == 1 ? E_VEH_CALLBACK :
                                   cases[i].callback == 2 ? E_BASE_CALLBACK : E_TERRAIN_CALLBACK);
            }
        }
        expected.push_back(E_EXEC1); expected.push_back(E_LINE);
        expected.push_back(E_START); expected.push_back(E_OK); expected.push_back(E_SET_LOC);
        if (cases[i].first_sprite != 0) {
            expected.push_back(E_CALLER_PICTURE); expected.push_back(E_SPRITE);
            if (cases[i].callback != 0) {
                expected.push_back(cases[i].callback == 1 ? E_VEH_CALLBACK :
                                   cases[i].callback == 2 ? E_BASE_CALLBACK : E_TERRAIN_CALLBACK);
            }
        }
        expected.push_back(E_EXEC0); expected.push_back(E_POPUPS_NORMAL);
        assert_events(expected);
    }

    reset_observations();
    TutWin enabled;
    enabled.picture_is_enabled = true;
    enabled.field_53AC_ = 0;
    assert(enabled.tut_win(&owner, "enabled", 640, 480, 0, 0, -1, -1) == 73);
    assert(callbacks.size() == 2 && callbacks[0] == 1 && callbacks[1] == 1);
    assert(caller_pictures.size() == 2 && caller_pictures[0] == 0 && caller_pictures[1] == 0);
    return 0;
}

static void assert_geometry(int input_x, int input_y, int expected_x, int expected_y,
                            int expected_center_x, int expected_line_y) {
    reset_observations();
    GraphicWin owner;
    TutWin window;
    window.field_53AC_ = -1;
    assert(window.tut_win(&owner, "geometry", input_x, input_y, 0, 0, -1, -1) == 73);
    assert_two_passes();
    assert(locations.size() == 2);
    assert(locations[0] == expected_x && locations[1] == expected_y);
    assert(owner.lines == 1);
    assert(owner.x1 == expected_center_x && owner.y1 == expected_line_y);
}

static int run_geometry_cases() {
    WinScreenWidth = 1024; WinScreenHeight = 768;
    configured_width = 490; configured_height = 211;
    // Mode 0 anchors are x=640/y=480. The boundary is inclusive on the
    // left/bottom branches; opposite-side coordinates select right/top.
    assert_geometry(640, 480, 5, 549, 250, 549);
    assert_geometry(641, 481, 529, 60, 774, 271);
    assert_geometry(640, 481, 5, 60, 250, 271);
    assert_geometry(641, 480, 529, 549, 774, 549);

    // The original uses sign subtraction before SAR, matching truncation
    // toward zero for negative half-widths. These are odd negative values so
    // the old +7 / wrong-sign expressions change the selected side/center.
    WinScreenWidth = -1025; WinScreenHeight = 768;
    configured_width = 490; configured_height = 211;
    assert_geometry(-640, 480, 5, 549, 250, 549);

    WinScreenWidth = 1024; WinScreenHeight = 768;
    configured_width = -491; configured_height = 211;
    assert_geometry(640, 480, 5, 549, -240, 549);
    return 0;
}

static int run_mode_and_order_case() {
    reset_observations();
    GraphicWin owner;
    TutWin window;
    window.field_53AC_ = -1;
    publish_modal_result = 1;
    TutWinShownFlag = 0;
    assert(window.tut_win(&owner, "mode", 640, 480, 0, 0xA50101, -1, -1) == 73);
    assert(window.field_53D0_ == 0xA50000);
    assert(window.field_53B8_ == 1);
    assert(TutWinShownFlag == 1);
    assert(start_flags.size() == 2 && start_flags[0] == 0 && start_flags[1] == 0);
    assert(events[events.size() - 3] == E_POPUPS_NORMAL);
    assert(events[events.size() - 2] == E_TOUR);
    assert(events.back() == E_KEY);
    return 0;
}

int main() {
    assert(run_callback_cases() == 0);
    assert(run_geometry_cases() == 0);
    assert(run_mode_and_order_case() == 0);
}
'''


def _source_method() -> str:
    source = (ROOT / "src/tutwin.cpp").read_text(encoding="utf-8")
    return _method_body(source, "int TutWin::tut_win(")


def _compiler_candidates() -> list[str]:
    found = [compiler for compiler in (shutil.which("g++"), shutil.which("clang++"))
             if compiler]
    if not found:
        pytest.skip("host C++ compiler unavailable")
    return found


def _compile_and_run(tmp_path: Path, body: str, compiler: str, *, should_pass: bool) -> None:
    source = HARNESS.replace(METHOD_MARKER, body)
    cpp = tmp_path / "tutorial_window_actual.cpp"
    exe = tmp_path / "tutorial_window_actual"
    cpp.write_text(source, encoding="utf-8")
    compiled = subprocess.run(
        [compiler, "-std=c++98", "-O0", str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=45,
    )
    assert compiled.returncode == 0, compiled.stderr
    run = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)
    if should_pass:
        assert run.returncode == 0, run.stderr
    else:
        assert run.returncode != 0, "behavioral regression mutant unexpectedly passed"


@pytest.mark.parametrize("compiler", _compiler_candidates())
def test_actual_tutwin_method_branch_callbacks_geometry_and_order(
    tmp_path: Path, compiler: str,
) -> None:
    _compile_and_run(tmp_path, _source_method(), compiler, should_pass=True)


def _replace_exact(body: str, old: str, new: str) -> str:
    assert body.count(old) == 1, (old, body.count(old))
    return body.replace(old, new, 1)


@pytest.mark.parametrize("compiler", _compiler_candidates())
@pytest.mark.parametrize(
    "old,new,reason",
    [
        ("field_53AC_ < -1", "field_53AC_ >= -1", "first-pass -1/base boundary"),
        ("field_53AC_ < -1", "field_53AC_ >= -1", "second-pass -1/base boundary"),
        ("if (y <= vertical_anchor)", "if (y > vertical_anchor)", "vertical placement boundary"),
        ("if (x <= horizontal_anchor)", "if (x > horizontal_anchor)", "horizontal placement boundary"),
        (
            "screen_width -\n                         (screen_width < 0 ? -1 : 0)",
            "screen_width -\n                         (screen_width < 0 ? 7 : 0)",
            "negative screen-width sign bias",
        ),
        (
            "static_cast<int>(layout_width()) -\n                       (static_cast<int>(layout_width()) < 0 ? -1 : 0)",
            "static_cast<int>(layout_width()) +\n                       (static_cast<int>(layout_width()) < 0 ? -1 : 0)",
            "negative left-layout-width sign bias",
        ),
    ],
)
def test_original_branch_and_sign_regression_mutants_fail(
    tmp_path: Path, compiler: str, old: str, new: str, reason: str,
) -> None:
    body = _source_method()
    if reason in ("first-pass -1/base boundary", "second-pass -1/base boundary"):
        # The two identical guards are independently observable on separate
        # setup passes; mutate only the later one in this control.
        assert body.count(old) == 2
        first = body.index(old)
        second = body.index(old, first + len(old))
        index = first if reason == "first-pass -1/base boundary" else second
        body = body[:index] + new + body[index + len(old):]
    else:
        body = _replace_exact(body, old, new)
    _compile_and_run(tmp_path, body, compiler, should_pass=False)


def test_pinned_original_tutwin_branch_and_signed_placement_instructions() -> None:
    configured_image = os.environ.get("OPENSMACX_IMAGE")
    candidates = ([Path(configured_image)] if configured_image else []) + [
        ROOT / ".opensmacx/game/terranx_original.exe",
        ROOT.parents[2] / ".opensmacx/game/terranx_original.exe",
    ]
    image_path = next((candidate for candidate in candidates if candidate.is_file()), candidates[0])
    if not image_path.is_file():
        pytest.skip("pinned original image unavailable")
    raw = image_path.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == PIN
    image = PeImage(raw, PIN)
    instructions = _disassemble(image, *ORIGINAL_SPAN)

    # First and second Sprite setup passes: >=0 is vehicle; -1 skips the
    # base_draw range and proceeds to terrain; values below -1 select base.
    _expect(instructions, 0x4BE1DA, "mov", "eax, dword ptr [esi + 0x53ac]")
    _expect(instructions, 0x4BE1E0, "cmp", "eax, ebx")
    _expect(instructions, 0x4BE1E2, "jl", "0x4be21e")
    _expect(instructions, 0x4BE21E, "cmp", "eax, -1")
    _expect(instructions, 0x4BE221, "jge", "0x4be25d")
    _expect(instructions, 0x4BE50C, "mov", "eax, dword ptr [esi + 0x53ac]")
    _expect(instructions, 0x4BE512, "cmp", "eax, edi")
    _expect(instructions, 0x4BE514, "jl", "0x4be54c")
    _expect(instructions, 0x4BE54C, "cmp", "eax, -1")
    _expect(instructions, 0x4BE54F, "jge", "0x4be587")

    # The original inclusive y/x thresholds and signed divide-by-two use.
    _expect(instructions, 0x4BE309, "cmp", "dword ptr [ebp + 0x14], edi")
    _expect(instructions, 0x4BE30C, "jg", "0x4be321")
    _expect(instructions, 0x4BE344, "cmp", "eax, ecx")
    _expect(instructions, 0x4BE346, "jg", "0x4be365")
    _expect(instructions, 0x4BE2B8, "cdq", "")
    _expect(instructions, 0x4BE2B9, "sub", "eax, edx")
    _expect(instructions, 0x4BE2BB, "mov", "ecx, eax")
    _expect(instructions, 0x4BE2D6, "sar", "ecx, 1")
    _expect(instructions, 0x4BE355, "cdq", "")
    _expect(instructions, 0x4BE356, "sub", "eax, edx")
    _expect(instructions, 0x4BE358, "sar", "eax, 1")
