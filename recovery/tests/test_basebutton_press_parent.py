"""Actual-source callback tests for BaseButton::on_left_down."""

from pathlib import Path
import shutil
import subprocess

import pytest

from test_autosound_sound_slots20_31_actual_source_oct13x import extract_definition


ROOT = Path(__file__).resolve().parents[2]

HARNESS = r"""
#include <cstdint>
#include <string>

typedef uint32_t DWORD;
class BaseButton;
static BaseButton *WinPointerOwner3;
static BaseButton *WinPointerOwner4;
static BaseButton *button_under_test;
static void (*clock_callback)();
static DWORD clock_result;
static unsigned int clock_calls, redraw_calls, pulse_calls;
static int pulse_delay, pulse_argument;
static std::string events;

struct Sound {
    int playing;
    unsigned int status_calls, play_calls;
    void (*status_callback)(Sound *);
    void (*play_callback)(Sound *);
    Sound() : playing(0), status_calls(0), play_calls(0),
              status_callback(0), play_callback(0) {}
    int is_playing() {
        ++status_calls;
        events += 'q';
        if (status_callback != 0) status_callback(this);
        return playing;
    }
    void play() {
        ++play_calls;
        events += 'p';
        if (play_callback != 0) play_callback(this);
    }
};

struct Win {
    Sound *sound_30;
    Sound *sound_28;
    unsigned int sound30_calls, sound28_calls, pressed_calls, v43_calls;
    int pressed_id, v43_id, pressed_result;
    void (*sound30_callback)();
    void (*sound28_callback)();
    Win() : sound_30(0), sound_28(0), sound30_calls(0), sound28_calls(0),
            pressed_calls(0), v43_calls(0), pressed_id(-1), v43_id(-1),
            pressed_result(0), sound30_callback(0), sound28_callback(0) {}
    static void clear_bubble_text() { events += 'b'; }
    Sound *legacy_sound_30() {
        ++sound30_calls;
        events += '3';
        if (sound30_callback != 0) sound30_callback();
        return sound_30;
    }
    Sound *legacy_sound_28() {
        ++sound28_calls;
        events += '8';
        if (sound28_callback != 0) sound28_callback();
        return sound_28;
    }
    virtual int on_button_pressed(int id) {
        ++pressed_calls;
        pressed_id = id;
        events += 'h';
        return pressed_result;
    }
    virtual void vslot_43(int id) {
        ++v43_calls;
        v43_id = id;
        events += 'v';
    }
};

static Win *WinCallbackWindow;
static DWORD timeGetTime() {
    ++clock_calls;
    events += 'g';
    if (clock_callback != 0) clock_callback();
    return clock_result;
}
static void base_button_timer_callback(void *, int) { events += 'm'; }

struct TimeProbe {
    void pulse(void (*)(void *, int), int argument, void *, uint32_t delay, int) {
        ++pulse_calls;
        pulse_argument = argument;
        pulse_delay = static_cast<int>(delay);
        events += 't';
    }
};

typedef void (*WinNotificationCallback)(int);
struct BaseButton {
    Win *win_parent_;
    uint32_t field_A9C_, field_A78_, field_A44_, field_AA4_, field_AAC_;
    WinNotificationCallback field_AB4_;
    TimeProbe time2_;
    BaseButton() : win_parent_(0), field_A9C_(0), field_A78_(0),
                   field_A44_(0xFFFFFFFFU), field_AA4_(0), field_AAC_(0),
                   field_AB4_(0) {}
    void redraw() { ++redraw_calls; events += 'r'; }
    void on_left_down(int, int);
};

BODY

static Win *after_first_sound_status;
static Win *after_first_sound_play;
static uint32_t id_after_status, id_after_play;
static void mutate_after_status(Sound *) {
    button_under_test->win_parent_ = after_first_sound_status;
    button_under_test->field_A78_ = id_after_status;
}
static void mutate_after_play(Sound *) {
    button_under_test->win_parent_ = after_first_sound_play;
    button_under_test->field_A78_ = id_after_play;
}

static Win *parent_after_clock;
static Win *after_second_sound_status;
static Win *after_second_sound_play;
static uint32_t id_after_clock, id_after_second_status, id_after_second_play;
static void mutate_after_clock() {
    button_under_test->win_parent_ = parent_after_clock;
    button_under_test->field_A78_ = id_after_clock;
}
static void mutate_after_second_status(Sound *) {
    button_under_test->win_parent_ = after_second_sound_status;
    button_under_test->field_A78_ = id_after_second_status;
}
static void mutate_after_second_play(Sound *) {
    button_under_test->win_parent_ = after_second_sound_play;
    button_under_test->field_A78_ = id_after_second_play;
}
static int notification_id;
static void notification(int id) { notification_id = id; events += 'n'; }

static void reset_case(BaseButton &button) {
    events.clear();
    WinPointerOwner3 = WinPointerOwner4 = 0;
    WinCallbackWindow = 0;
    button_under_test = &button;
    clock_callback = 0;
    clock_result = 321;
    clock_calls = redraw_calls = pulse_calls = 0;
    pulse_delay = pulse_argument = 0;
    notification_id = -1;
    button.win_parent_ = 0;
    button.field_A9C_ = button.field_A78_ = button.field_AA4_ = 0;
    button.field_A44_ = 0xFFFFFFFFU;
    button.field_AAC_ = 0;
    button.field_AB4_ = 0;
}

static int test_initial_press_reloads_parent_after_sound_callbacks() {
    BaseButton button;
    Win initial, after_status, after_play;
    Sound captured, alternate_status, alternate_play;
    reset_case(button);
    button.win_parent_ = &initial;
    button.field_A9C_ = 0;  // initial press: sound 30, then vslot 43
    button.field_A78_ = 40;
    button.field_A44_ = 17;
    initial.sound_30 = &captured;
    after_status.sound_30 = &alternate_status;
    after_play.sound_30 = &alternate_play;
    captured.status_callback = mutate_after_status;
    captured.play_callback = mutate_after_play;
    after_first_sound_status = &after_status;
    after_first_sound_play = &after_play;
    id_after_status = 41;
    id_after_play = 42;

    button.on_left_down(0, 0);
    if (events != "b3qpvrt") return 11;
    if (initial.sound30_calls != 1 || after_status.sound30_calls != 0 ||
        after_play.sound30_calls != 0 || captured.status_calls != 1 ||
        captured.play_calls != 1 || alternate_status.play_calls != 0 ||
        alternate_play.play_calls != 0) return 12;
    if (initial.v43_calls != 0 || after_status.v43_calls != 0 ||
        after_play.v43_calls != 1 || after_play.v43_id != 42) return 13;
    if (button.field_A9C_ != 1 || redraw_calls != 1 || pulse_calls != 1 ||
        pulse_argument != 1 || pulse_delay != 17 || clock_calls != 0 ||
        WinPointerOwner3 != &button ||
        WinPointerOwner4 != 0) return 14;
    return 0;
}

static int test_repeat_press_reloads_parent_at_notification_boundaries() {
    BaseButton button;
    Win initial, clock_parent, after_status, after_play;
    Sound captured_sound, other_sound;
    reset_case(button);
    button.win_parent_ = &initial;
    button.field_A9C_ = 1;  // repeat press: clock, sound 28, pressed notification
    button.field_A78_ = 50;
    button.field_A44_ = 23;
    button.field_AB4_ = notification;
    clock_parent.sound_28 = &captured_sound;
    after_status.sound_28 = &other_sound;
    after_play.pressed_result = 1;
    clock_callback = mutate_after_clock;
    parent_after_clock = &clock_parent;
    after_second_sound_status = &after_status;
    after_second_sound_play = &after_play;
    id_after_clock = 51;
    id_after_second_status = 52;
    id_after_second_play = 53;
    captured_sound.status_callback = mutate_after_second_status;
    captured_sound.play_callback = mutate_after_second_play;
    clock_result = 900;

    button.on_left_down(0, 0);
    if (events != "bg8qphtn") return 21;
    if (clock_calls != 1 || button.field_AA4_ != 900 ||
        WinCallbackWindow != &clock_parent) return 22;
    if (initial.sound28_calls != 0 || clock_parent.sound28_calls != 1 ||
        after_status.sound28_calls != 0 || after_play.sound28_calls != 0 ||
        captured_sound.status_calls != 1 || captured_sound.play_calls != 1 ||
        other_sound.play_calls != 0) return 23;
    if (after_play.pressed_calls != 1 || after_play.pressed_id != 53 ||
        initial.pressed_calls != 0 || clock_parent.pressed_calls != 0 ||
        after_status.pressed_calls != 0) return 24;
    if (notification_id != 53 || redraw_calls != 0 || pulse_calls != 1 ||
        pulse_argument != 1 || pulse_delay != 23 || button.field_A9C_ != 1 ||
        WinPointerOwner3 != &button || WinPointerOwner4 != 0) return 25;
    return 0;
}

int main() {
    int result = test_initial_press_reloads_parent_after_sound_callbacks();
    if (result != 0) return result;
    return test_repeat_press_reloads_parent_at_notification_boundaries();
}
"""


def actual_body():
    source = (ROOT / "src/basebutton.cpp").read_text(encoding="utf-8")
    return extract_definition(source, "void BaseButton::on_left_down(")


def cached_parent_mutant(body):
    signature = "void BaseButton::on_left_down(int, int) {"
    assert body.count(signature) == 1
    if "Win *const parent = win_parent_;" in body:
        return body
    mutant = body.replace(signature, signature + "\n    Win *const parent = win_parent_;", 1)
    anchors = (
        ("Sound *const owner = win_parent_->legacy_sound_30();",
         "Sound *const owner = parent->legacy_sound_30();"),
        ("win_parent_->vslot_43", "parent->vslot_43"),
        ("WinCallbackWindow = win_parent_;", "WinCallbackWindow = parent;"),
        ("Sound *const owner = win_parent_->legacy_sound_28();",
         "Sound *const owner = parent->legacy_sound_28();"),
        ("win_parent_->on_button_pressed", "parent->on_button_pressed"),
    )
    for old, new in anchors:
        assert mutant.count(old) == 1, old
        mutant = mutant.replace(old, new, 1)
    return mutant


def compile_and_run(tmp_path, compiler, body):
    cpp = tmp_path / "basebutton_press.cpp"
    executable = tmp_path / "basebutton_press"
    cpp.write_text(HARNESS.replace("BODY", body), encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++11", "-O1", "-Wall", "-Wextra",
         "-fsanitize=undefined", "-fno-sanitize-recover=undefined",
         str(cpp), "-o", str(executable)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_left_down_reloads_parent_across_callbacks(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} required")
    run = compile_and_run(tmp_path, compiler, actual_body())
    assert run.returncode == 0, run.stdout + run.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_cached_parent_mutant_fails_callback_reentry_controls(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} required")
    run = compile_and_run(tmp_path, compiler, cached_parent_mutant(actual_body()))
    assert run.returncode != 0, "cached-parent mutant survived reentry controls"
