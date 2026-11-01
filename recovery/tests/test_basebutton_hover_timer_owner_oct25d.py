"""Host proposal for BaseButton's pinned hover-timer member selection.

Extracts the complete actual on_mouse_move body. The fixture distinguishes
the two Time members and observes timer/notification effects; it does not
claim native layout or Win32 runtime behavior.
"""
from __future__ import annotations

import os
from pathlib import Path
import shutil
import subprocess

import pytest

from test_autosound_sound_slots20_31_actual_source_oct13x import extract_definition


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/basebutton.cpp"
SIGNATURE = "void BaseButton::on_mouse_move(int x, int y)"

HARNESS = r'''#include <cassert>
#include <cstdint>
#ifndef __cdecl
#define __cdecl
#endif

struct RECT { int left, top, right, bottom; };
struct Buffer { static RECT rect2_; };
RECT Buffer::rect2_ = {4, 5, 24, 25};
struct Win;
struct BaseButton;
struct ButtonGroup {};
static Win *WinPointerOwner1;
static Win *WinPointerOwner2;
static int WinSizingFlag;
static int configured_inside;
static int configured_lbutton;
static int updates, redraws;
static int forwarded_presses;
static int v42_calls, v43_calls;
static int sound29_gets, sound30_gets;
static int sound29_plays, sound30_plays;
static int time1_starts, time1_stops, time2_starts, time2_stops;
static int callback_arguments, callback_contexts, callback_delays, callback_resolutions;
static void __cdecl base_button_timer_callback(int, void *) {}

struct Sound {
    int *plays;
    explicit Sound(int *counter) : plays(counter) {}
    int is_playing() { return 0; }
    void play() { ++*plays; }
};
static Sound Sound29(&sound29_plays), Sound30(&sound30_plays);

struct Win {
    Sound *legacy_sound_29() { ++sound29_gets; return &Sound29; }
    Sound *legacy_sound_30() { ++sound30_gets; return &Sound30; }
    void vslot_42(int id) { assert(id == 23); ++v42_calls; }
    void vslot_43(int id) { assert(id == 23); ++v43_calls; }
    virtual void on_left_down(int x, int y) { assert(x == 10 && y == 11); ++forwarded_presses; }
};

struct TimeProbe {
    int *starts;
    int *stops;
    TimeProbe(int *a, int *b) : starts(a), stops(b) {}
    void start(void (__cdecl *callback)(int, void *), int argument, void *context,
               uint32_t delay, int resolution) {
        assert(callback == base_button_timer_callback);
        ++*starts;
        callback_arguments = argument;
        callback_contexts = context != 0;
        callback_delays = static_cast<int>(delay);
        callback_resolutions = resolution;
    }
    void stop() { ++*stops; }
};

struct BaseButton : Win {
    Win *win_parent_;
    ButtonGroup *group_;
    uint32_t field_A9C_, field_A44_, field_A78_;
    TimeProbe time1_, time2_;
    BaseButton() : win_parent_(0), group_(0), field_A9C_(0), field_A44_(17), field_A78_(23),
                   time1_(&time1_starts, &time1_stops),
                   time2_(&time2_starts, &time2_stops) {}
    int in_box(int x, int y, RECT *area) {
        assert(x == 10 && y == 11);
        assert(area->left == 0 && area->top == 0 &&
               area->right == 20 && area->bottom == 20);
        return configured_inside;
    }
    int get_lbutton_state() { return configured_lbutton; }
    void on_update() { ++updates; }
    void redraw() { ++redraws; }
    void on_mouse_move(int x, int y);
};

ACTUAL_BODY

static void reset() {
    WinPointerOwner1 = WinPointerOwner2 = 0;
    WinSizingFlag = configured_inside = configured_lbutton = 0;
    updates = redraws = forwarded_presses = v42_calls = v43_calls = 0;
    sound29_gets = sound30_gets = sound29_plays = sound30_plays = 0;
    time1_starts = time1_stops = time2_starts = time2_stops = 0;
    callback_arguments = callback_contexts = callback_delays = callback_resolutions = 0;
}

int main() {
    ButtonGroup group;
    for (int grouped = 0; grouped != 2; ++grouped) {
        BaseButton button;
        Win parent;
        button.win_parent_ = &parent;
        button.group_ = grouped ? &group : 0;

        // Ownership/sizing gates short-circuit before hit testing or timer effects.
        reset();
        WinPointerOwner1 = 0;
        WinSizingFlag = 1;
        configured_inside = 1;
        button.on_mouse_move(10, 11);
        assert(time1_starts == 0 && time2_starts == 0 && updates == 0);
        WinPointerOwner1 = &button;
        WinSizingFlag = 0;
        button.on_mouse_move(10, 11);
        assert(time1_starts == 0 && time2_starts == 0 && updates == 0);

        // Leaving an active hover stops the pinned second timer member before the
        // release sound, parent notification, and update.
        reset();
        WinPointerOwner1 = &button;
        WinSizingFlag = 1;
        configured_inside = 0;
        button.field_A9C_ = 1;
        button.on_mouse_move(10, 11);
        assert(time2_stops == 1 && time1_stops == 0);
        assert(button.field_A9C_ == 0 && sound29_gets == 1 && sound29_plays == 1);
        assert(v42_calls == 1 && redraws == 1 && updates == 0);

        // Entering a hover uses that same second member and keeps timer parameters.
        reset();
        WinPointerOwner1 = &button;
        WinSizingFlag = 1;
        configured_inside = 1;
        button.field_A9C_ = 0;
        button.field_A44_ = 17;
        button.on_mouse_move(10, 11);
        assert(time2_starts == 1 && time1_starts == 0);
        assert(callback_arguments == 1 && callback_contexts == 1 &&
               callback_delays == 17 && callback_resolutions == 5);
        assert(button.field_A9C_ == 1 && sound30_gets == 1 && sound30_plays == 1);
        assert(v43_calls == 1 && redraws == 1 && updates == 0);

        // The sentinel suppresses only timer start; the rest of entry still runs.
        reset();
        WinPointerOwner1 = &button;
        WinSizingFlag = 1;
        configured_inside = 1;
        button.field_A9C_ = 0;
        button.field_A44_ = 0xFFFFFFFFU;
        button.on_mouse_move(10, 11);
        assert(time1_starts == 0 && time2_starts == 0);
        assert(button.field_A9C_ == 1 && sound30_gets == 1 && v43_calls == 1 &&
               redraws == 1 && updates == 0);

        // A held button forwards the press through the inherited Win method.
        reset();
        WinPointerOwner1 = &button;
        WinSizingFlag = 1;
        configured_inside = configured_lbutton = 1;
        button.field_A9C_ = 1;
        button.on_mouse_move(10, 11);
        assert(forwarded_presses == 1 && time1_stops == 0 && time2_stops == 0);
    }
}
'''


def _body() -> str:
    return extract_definition(SOURCE.read_text(encoding="utf-8"), SIGNATURE)


def _old_first_timer_mutant(body: str) -> str:
    stop = "time2_.stop();"
    start = "time2_.start(base_button_timer_callback, 1, this, field_A44_, 5);"
    assert body.count(stop) == 1 and body.count(start) == 1
    return body.replace(stop, "time1_.stop();", 1).replace(start, start.replace("time2_", "time1_"), 1)


def _on_update_mutant(body: str) -> str:
    assert body.count("redraw();") == 2
    return body.replace("redraw();", "on_update();")


def _compile_and_run(tmp_path: Path, compiler: str, body: str, name: str) -> subprocess.CompletedProcess[str]:
    source = HARNESS.replace("ACTUAL_BODY", body)
    cpp = tmp_path / f"{name}.cpp"
    exe = tmp_path / name
    cpp.write_text(source, encoding="utf-8")
    subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(cpp), "-o", str(exe)], check=True, capture_output=True,
                   text=True, timeout=60)
    return subprocess.run([str(exe)], capture_output=True, text=True, timeout=20)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_hover_transitions_use_second_time_member(tmp_path: Path, compiler_name: str) -> None:
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    result = _compile_and_run(tmp_path, compiler, _body(), f"hover-{compiler_name}")
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_restoring_first_time_member_compiles_and_is_rejected(tmp_path: Path, compiler_name: str) -> None:
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    result = _compile_and_run(tmp_path, compiler, _old_first_timer_mutant(_body()),
                              f"old-first-timer-{compiler_name}")
    assert result.returncode != 0, "time1_ timer mutant survived the slot-specific assertions"


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_on_update_dispatch_mutant_compiles_and_is_rejected(tmp_path: Path, compiler_name: str) -> None:
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    result = _compile_and_run(tmp_path, compiler, _on_update_mutant(_body()),
                              f"on-update-{compiler_name}")
    assert result.returncode != 0, "on_update dispatch mutant survived redraw-specific assertions"
