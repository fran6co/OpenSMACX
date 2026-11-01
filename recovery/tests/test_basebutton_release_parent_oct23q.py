"""Actual-source BaseButton release controls through its synchronous callbacks."""

from pathlib import Path
import shutil
import subprocess

import pytest

from test_autosound_sound_slots20_31_actual_source_oct13x import extract_definition


ROOT = Path(__file__).resolve().parents[2]


HARNESS = r"""
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>

typedef uint32_t DWORD;
class BaseButton;
struct ButtonGroup {};
static ButtonGroup *configured_group;
static BaseButton *WinPointerOwner3;
static BaseButton *WinPointerOwner4;
static BaseButton *button_under_test;
static void (*sleep_callback)();
static DWORD clock_value;
static unsigned int clock_calls, sleep_calls, sleep_duration, timer_stop_calls;
static unsigned int redraw_calls, lose_capture_calls;
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
    Sound *sound;
    int id;
    unsigned int sound_lookups, vslot_calls;
    int vslot_argument;
    Win() : sound(0), id(0), sound_lookups(0), vslot_calls(0),
            vslot_argument(-1) {}
    Sound *legacy_sound_29() {
        ++sound_lookups;
        events += 'l';
        return sound;
    }
    virtual void vslot_42(int argument) {
        ++vslot_calls;
        vslot_argument = argument;
        events += 'v';
    }
};

struct TimeProbe {
    void stop() { ++timer_stop_calls; events += 'T'; }
};

DWORD timeGetTime() {
    ++clock_calls;
    events += 'g';
    return clock_value;
}
void Sleep(DWORD duration) {
    ++sleep_calls;
    sleep_duration = duration;
    events += 's';
    if (sleep_callback != 0) sleep_callback();
}

struct BaseButton {
    Win *win_parent_;
    ButtonGroup *group_;
    DWORD field_AA0_, field_AA4_;
    uint32_t field_A9C_, field_A78_;
    TimeProbe time2_;
    BaseButton() : win_parent_(0), group_(0), field_AA0_(0), field_AA4_(0),
                   field_A9C_(0), field_A78_(0) {}
    void on_lose_mouse_capture() { ++lose_capture_calls; events += 'c'; }
    void redraw() { ++redraw_calls; events += 'r'; }
    void on_left_up(int, int);
};

BODY

static void reset_case(BaseButton &button) {
    events.clear();
    WinPointerOwner3 = 0;
    WinPointerOwner4 = 0;
    button_under_test = &button;
    sleep_callback = 0;
    clock_value = 100;
    clock_calls = sleep_calls = sleep_duration = timer_stop_calls = 0;
    redraw_calls = lose_capture_calls = 0;
    button.win_parent_ = 0;
    button.group_ = configured_group;
    button.field_AA0_ = button.field_AA4_ = 0;
    button.field_A9C_ = button.field_A78_ = 0;
}

static Win *parent_after_sleep;
static void install_parent_after_sleep() {
    button_under_test->win_parent_ = parent_after_sleep;
}
static Win *parent_after_status;
static Win *parent_after_play;
static void change_parent_during_status(Sound *) {
    button_under_test->win_parent_ = parent_after_status;
}
static void change_parent_during_play(Sound *) {
    button_under_test->win_parent_ = parent_after_play;
}

static int test_initial_null_and_capture_release() {
    BaseButton button;
    reset_case(button);
    WinPointerOwner3 = &button;
    WinPointerOwner4 = &button;
    button.field_A9C_ = 1;
    button.on_left_up(0, 0);
    if (events != "c" || lose_capture_calls != 1 ||
        WinPointerOwner3 != 0 || WinPointerOwner4 != 0) return 11;
    if (timer_stop_calls != 0 || clock_calls != 0 || sleep_calls != 0 ||
        redraw_calls != 0 || button.field_A9C_ != 1) return 12;
    return 0;
}

static int test_sleep_reloads_parent_and_skips_playing_sound() {
    BaseButton button;
    Win before_sleep, after_sleep;
    Sound sound_before, playing_sound;
    reset_case(button);
    button.win_parent_ = &before_sleep;
    button.field_AA0_ = 30;
    button.field_AA4_ = 100;
    button.field_A9C_ = 1;
    button.field_A78_ = 42;
    clock_value = 110;
    before_sleep.sound = &sound_before;
    after_sleep.sound = &playing_sound;
    playing_sound.playing = 1;
    parent_after_sleep = &after_sleep;
    sleep_callback = install_parent_after_sleep;
    button.on_left_up(0, 0);
    if (events != "Tgslqvr" || timer_stop_calls != 1 || clock_calls != 1 ||
        sleep_calls != 1 || sleep_duration != 20) return 21;
    if (before_sleep.sound_lookups != 0 || after_sleep.sound_lookups != 1 ||
        sound_before.status_calls != 0 || playing_sound.status_calls != 1 ||
        playing_sound.play_calls != 0) return 22;
    if (after_sleep.vslot_calls != 1 || after_sleep.vslot_argument != 42 ||
        before_sleep.vslot_calls != 0 || redraw_calls != 1 ||
        button.field_A9C_ != 0) return 23;
    return 0;
}

static int test_sound_pointer_is_captured_but_notification_reloads_parent() {
    BaseButton button;
    Win original, after_status, after_play;
    Sound captured_sound, other_sound, final_sound;
    reset_case(button);
    button.win_parent_ = &original;
    button.field_AA0_ = 10;
    button.field_AA4_ = 90;
    button.field_A9C_ = 1;
    button.field_A78_ = 42;
    original.sound = &captured_sound;
    after_status.sound = &other_sound;
    after_play.sound = &final_sound;
    captured_sound.playing = 0;
    captured_sound.status_callback = change_parent_during_status;
    captured_sound.play_callback = change_parent_during_play;
    parent_after_status = &after_status;
    parent_after_play = &after_play;
    button.on_left_up(0, 0);
    if (events != "Tglqpvr" || clock_calls != 1 || sleep_calls != 0) return 31;
    if (original.sound_lookups != 1 || after_status.sound_lookups != 0 ||
        after_play.sound_lookups != 0 || captured_sound.status_calls != 1 ||
        captured_sound.play_calls != 1 || other_sound.play_calls != 0 ||
        final_sound.play_calls != 0) return 32;
    if (after_play.vslot_calls != 1 || after_play.vslot_argument != 42 ||
        original.vslot_calls != 0 || after_status.vslot_calls != 0 ||
        redraw_calls != 1 || button.field_A9C_ != 0) return 33;
    return 0;
}

static int test_null_sound_still_notifies_and_redraws() {
    BaseButton button;
    Win parent;
    reset_case(button);
    button.win_parent_ = &parent;
    button.field_AA0_ = 10;
    button.field_AA4_ = 90;
    button.field_A9C_ = 1;
    button.field_A78_ = 42;
    button.on_left_up(0, 0);
    if (events != "Tglvr" || parent.sound_lookups != 1 ||
        parent.vslot_calls != 1 || parent.vslot_argument != 42 ||
        redraw_calls != 1) return 41;
    return 0;
}

int main() {
    ButtonGroup group;
    for (int grouped = 0; grouped != 2; ++grouped) {
        configured_group = grouped ? &group : 0;
        int result = test_initial_null_and_capture_release();
        if (result != 0) return result;
        result = test_sound_pointer_is_captured_but_notification_reloads_parent();
        if (result != 0) return result;
        result = test_sleep_reloads_parent_and_skips_playing_sound();
        if (result != 0) return result;
        result = test_null_sound_still_notifies_and_redraws();
        if (result != 0) return result;
    }
    return 0;
}
"""


def actual_body():
    source = (ROOT / "src/basebutton.cpp").read_text(encoding="utf-8")
    return extract_definition(source, "void BaseButton::on_left_up(")


def old_cached_parent_mutant(body):
    signature = "void BaseButton::on_left_up(int, int) {"
    assert body.count(signature) == 1
    mutant = body.replace(signature, signature + "\n    Win *const parent = win_parent_;", 1)
    assert mutant.count("if (win_parent_ == 0)") == 1
    mutant = mutant.replace("if (win_parent_ == 0)", "if (parent == 0)", 1)
    assert mutant.count("Win *const sound_parent = win_parent_;") == 1
    mutant = mutant.replace(
        "Win *const sound_parent = win_parent_;",
        "Win *const sound_parent = parent;", 1)
    assert mutant.count("win_parent_->vslot_42") == 2
    return mutant.replace("win_parent_->vslot_42", "parent->vslot_42")


def omitted_notification_reload_mutant(body):
    signature = "void BaseButton::on_left_up(int, int) {"
    assert body.count(signature) == 1
    mutant = body.replace(signature, signature + "\n    Win *const parent = win_parent_;", 1)
    assert mutant.count("win_parent_->vslot_42") == 2
    return mutant.replace("win_parent_->vslot_42", "parent->vslot_42")


def compile_and_run(tmp_path, compiler, body):
    cpp = tmp_path / "basebutton_release.cpp"
    executable = tmp_path / "basebutton_release"
    cpp.write_text(HARNESS.replace("BODY", body))
    built = subprocess.run(
        [compiler, "-std=c++11", "-O1", "-Wall", "-Wextra",
         "-fsanitize=undefined", "-fno-sanitize-recover=undefined",
         str(cpp), "-o", str(executable)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run(
        [str(executable)], capture_output=True, text=True, timeout=10)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_release_parent_reload_prefix(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} required")
    body = actual_body()
    assert body.count("sound_parent->legacy_sound_29()") == 1
    assert body.count("win_parent_->vslot_42") == 2
    run = compile_and_run(tmp_path, compiler, body)
    assert run.returncode == 0, run.stdout + run.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("mutant", (old_cached_parent_mutant,
                                    omitted_notification_reload_mutant))
def test_parent_reload_mutants_fail_host_callbacks(tmp_path, compiler_name, mutant):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} required")
    damaged = mutant(actual_body())
    run = compile_and_run(tmp_path, compiler, damaged)
    assert run.returncode != 0, f"{mutant.__name__} survived the callback controls"
