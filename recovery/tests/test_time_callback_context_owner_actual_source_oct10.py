"""Host controls compile the real borrowed-context Time callback bodies."""

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest

from match_recovery.source import Mode, read_file


ROOT = Path(__file__).resolve().parents[2]
SOURCES = {
    name: ROOT / name
    for name in (
        "src/time.cpp", "src/time.h", "src/time_lifecycle.cpp", "src/win.cpp",
        "src/basewin.cpp", "src/basewin.h", "src/basebutton.cpp",
        "src/basebutton.h", "src/flatbutton.cpp", "src/diplopop.cpp",
        "src/diplopop.h", "src/jackal/palette.cpp", "src/jackal/palette.h",
        "src/maininterface.cpp", "src/maininterface.h",
    )
}


def extract_definition(text: str, signature: str) -> str:
    start = text.index(signature)
    while True:
        opening = text.index("{", start)
        semicolon = text.find(";", start, opening)
        if semicolon < 0:
            break
        start = text.index(signature, start + len(signature))
    depth = 0
    for offset in range(opening, len(text)):
        if text[offset] == "{":
            depth += 1
        elif text[offset] == "}":
            depth -= 1
            if depth == 0:
                return text[start : offset + 1]
    raise AssertionError(f"unterminated definition: {signature}")


def compiled_source_parts() -> tuple[list[str], list[str], str]:
    time = SOURCES["src/time.cpp"].read_text(encoding="utf-8")
    setters = [
        extract_definition(time, "void Time::init(void(__cdecl *callback)(int, void *)"),
        extract_definition(time, "uint32_t Time::start(void(__cdecl *callback)(int, void *)"),
        extract_definition(time, "uint32_t Time::pulse(void(__cdecl *callback)(int, void *)"),
    ]
    callbacks = [
        extract_definition(SOURCES["src/basewin.cpp"].read_text(),
                           "void BaseWin::timer_callback(int, void *)"),
        extract_definition(SOURCES["src/basebutton.cpp"].read_text(),
                           "void __cdecl base_button_timer_callback(int stage, void *context)"),
        extract_definition(SOURCES["src/diplopop.cpp"].read_text(),
                           "void __cdecl diplo_bioscan_timer_callback(int, void *context)"),
        extract_definition(SOURCES["src/diplopop.cpp"].read_text(),
                           "void __cdecl diplo_portrait_timer_callback(int, void *context)"),
        extract_definition(SOURCES["src/jackal/palette.cpp"].read_text(),
                           "void __cdecl Palette::timer_callback(int key, void *context)"),
        extract_definition(SOURCES["src/maininterface.cpp"].read_text(),
                           "void __cdecl main_interface_timer_callback(int a1, void *context)"),
    ]
    win = SOURCES["src/win.cpp"].read_text(encoding="utf-8")
    begin = win.index("                timer->tick_posted_ = 0;", win.index("case WM_USER + 1"))
    end = win.index("                return DefWindowProcA(window, message, wparam, lparam);\n            }", begin)
    dispatch = win[begin:end]
    lifecycle = SOURCES["src/time_lifecycle.cpp"].read_text(encoding="utf-8")
    callbacks.extend([
        extract_definition(lifecycle, "inline void Time::close_impl()"),
        extract_definition(lifecycle, "void Time::close()"),
    ])
    return setters, callbacks, dispatch


HARNESS = r'''
#include <cassert>
#include <cstdint>
#include <vector>
#include <algorithm>
#define __cdecl
using UINT = unsigned int;
using UINT_PTR = uintptr_t;
using DWORD = unsigned long;
using LPTIMECALLBACK = void (*)();
using TIMERPROC = void (*)();
using HWND = void*;
using WPARAM = uintptr_t;
using LPARAM = intptr_t;
using LRESULT = intptr_t;
constexpr int TIME_PERIODIC = 1;
constexpr int TIME_ONESHOT = 0;
constexpr int TimeOneShot = 2;
constexpr int TimeOneShotFired = 4;
constexpr int WM_USER = 0x400;
constexpr int FALSE = 0;
static int MsgStatus = 0;
static HWND HandleMain = nullptr;
static int timer_result = 17;
static int multimedia_result = 19;
static std::vector<int> timer_modes;
static std::vector<int> events;
static void TimerProc() {}
static void MultimediaProc() {}
static UINT_PTR SetTimer(HWND, UINT_PTR, UINT, TIMERPROC) {
    events.push_back(2);
    return static_cast<UINT_PTR>(timer_result);
}
static UINT_PTR timeSetEvent(UINT, UINT, LPTIMECALLBACK, UINT_PTR, UINT mode) {
    events.push_back(1);
    timer_modes.push_back(static_cast<int>(mode));
    return static_cast<UINT_PTR>(multimedia_result);
}
static LRESULT DefWindowProcA(HWND, UINT, WPARAM, LPARAM) { return 91; }
struct RECT { long left, top, right, bottom; };
static RECT* UnionRect(RECT* out, const RECT* a, const RECT* b) {
    *out = {std::min(a->left,b->left), std::min(a->top,b->top),
            std::max(a->right,b->right), std::max(a->bottom,b->bottom)};
    return out;
}
static int PaletteUsesSystemColours = 0;
static void* PaletteInitialized = reinterpret_cast<void*>(1);
static int PaletteSeedCache = 0;
struct PALETTEENTRY { unsigned char peRed, peGreen, peBlue, peFlags; };
static int animate_calls = 0;
static void AnimatePalette(void*, unsigned int, unsigned int, PALETTEENTRY*) {
    ++animate_calls;
}

struct Time {
    static Time* TimeModal;
    int oneshot_state_ = 0;
    UINT_PTR id_event_ = 0;
    void (__cdecl *callback1_)(int) = nullptr;
    void (__cdecl *callback2_)(int, void*) = nullptr;
    void* cb_param2_ = nullptr;
    int cb_param1_ = 0;
    uint32_t count_ = 0;
    uint32_t resolution_ = 5;
    int unk_2_ = 0;
    int tick_posted_ = 0;
    int stop_count = 0;
    void init(void (__cdecl *callback)(int, void*), int, void*, uint32_t, uint32_t);
    uint32_t start(void (__cdecl *callback)(int, void*), int, void*, uint32_t, uint32_t);
    uint32_t pulse(void (__cdecl *callback)(int, void*), int, void*, uint32_t, uint32_t);
    void stop() { ++stop_count; id_event_ = 0; events.push_back(3); }
    void close_impl();
    void close();
};
Time* Time::TimeModal = nullptr;

struct BaseWin { static void timer_callback(int, void*); };
struct BaseButton { std::vector<int> stages; void timer_callback(int stage) { stages.push_back(stage); } };
struct DiploPop {
    std::vector<int> calls;
    void bioscan_anim() { calls.push_back(1); }
    void portrait_anim() { calls.push_back(2); }
};
struct MainInterface {
    uint32_t alert_ = 0;
    Time time1_, time2_;
    RECT menu_animation_bounds_{1,2,5,7};
    RECT multiplayer_animation_bounds_{0,4,9,6};
    RECT last_update{};
    int update_count = 0;
    void soft_update(RECT* rect) { last_update = *rect; ++update_count; }
};
struct Palette {
    struct Slot { int key = -1; unsigned int first = 0; unsigned int count = 0; };
    Slot internal_[5]{};
    PALETTEENTRY entries_[16]{};
    unsigned int seed_ = 0;
    static void __cdecl timer_callback(int, void*);
};

// Actual Time setter definitions follow.
'''


def host_unit() -> str:
    setters, callbacks, dispatch = compiled_source_parts()
    prefix = HARNESS.replace("// Actual Time setter definitions follow.", "\n".join(setters))
    prefix += "\n".join(callbacks)
    prefix += "\nstatic LRESULT dispatch_tick(Time* timer, HWND window, UINT message, WPARAM wparam, LPARAM lparam) {\n"
    prefix += dispatch
    prefix += "\n return DefWindowProcA(window, message, wparam, lparam);\n}\n"
    prefix += r'''
static Time* mutating_timer;
static int first_calls = 0, second_calls = 0;
static int second_arg = 0;
static void __cdecl second_cb(int value, void* context) {
    ++second_calls; second_arg = value; assert(context == &second_calls);
}
static void __cdecl first_cb(int) {
    ++first_calls;
    mutating_timer->callback2_ = second_cb;
    mutating_timer->cb_param2_ = &second_calls;
}

int main() {
    int context = 0;
    Time t;
    auto cb = +[](int, void*) {};
    t.init(cb, 23, &context, 71, 9);
    assert(t.stop_count == 1 && t.callback2_ == cb && t.cb_param2_ == &context);
    assert(t.cb_param1_ == 23 && t.count_ == 71 && t.resolution_ == 9);

    events.clear(); timer_modes.clear(); timer_result = 0; multimedia_result = 0;
    assert(t.start(cb, -7, &context, 49, 3) == 2);
    assert(t.stop_count == 2 && t.callback2_ == cb && t.cb_param2_ == &context);
    assert(t.oneshot_state_ == 0 && t.callback1_ == nullptr && events == std::vector<int>({3,1}));
    assert(timer_modes == std::vector<int>({TIME_PERIODIC}));

    events.clear(); timer_modes.clear(); multimedia_result = 0;
    assert(t.pulse(cb, 8, &context, 49, 4) == 2);
    assert(t.callback2_ == cb && t.cb_param2_ == &context);
    assert(t.oneshot_state_ == 0 && timer_modes == std::vector<int>({TIME_ONESHOT}));

    events.clear(); timer_result = 0;
    assert(t.pulse(cb, 8, &context, 50, 4) == 2);
    assert(t.oneshot_state_ == TimeOneShot && events == std::vector<int>({3,2}));
    assert(t.start(nullptr, 2, &context, 9, 5) == 7);

    // The real window dispatch re-reads callback2/context after callback1
    // mutates both, rather than snapshotting either before the first call.
    Time d;
    mutating_timer = &d;
    d.callback1_ = first_cb;
    d.callback2_ = nullptr;
    d.cb_param1_ = 55;
    dispatch_tick(&d, nullptr, 0, 0, 0);
    assert(first_calls == 1 && second_calls == 1 && second_arg == 55);
    d.close();
    assert(d.callback2_ == nullptr && d.cb_param2_ == nullptr);

    BaseWin::timer_callback(3, &context);
    BaseButton button;
    base_button_timer_callback(2, &button);
    base_button_timer_callback(4, nullptr);
    assert(button.stages == std::vector<int>({2}));
    DiploPop diplo;
    diplo_bioscan_timer_callback(0, &diplo);
    diplo_portrait_timer_callback(0, &diplo);
    diplo_bioscan_timer_callback(0, nullptr);
    assert(diplo.calls == std::vector<int>({1,2}));

    MainInterface main;
    main_interface_timer_callback(0, &main);
    main.alert_ = 3;
    main_interface_timer_callback(1, &main);
    assert(main.time1_.stop_count == 1 && main.alert_ == 4);
    assert(main.update_count == 1 && main.last_update.left == 0 && main.last_update.right == 9);
    main.alert_ = static_cast<uint32_t>(-1);
    main_interface_timer_callback(1, &main);
    assert(main.time2_.stop_count == 1 && main.update_count == 1);
    main_interface_timer_callback(0, nullptr);

    Palette palette;
    palette.internal_[0] = {4, 2, 3};
    palette.seed_ = 123;
    palette.entries_[2] = {1,2,3,0};
    palette.entries_[3] = {4,5,6,0};
    palette.entries_[4] = {7,8,9,0};
    Palette::timer_callback(4, &palette);
    assert(palette.entries_[2].peRed == 4 && palette.entries_[2].peBlue == 6);
    assert(palette.entries_[4].peRed == 1 && palette.entries_[4].peBlue == 3);
    assert(animate_calls == 1 && PaletteSeedCache == 123);
    const int old_animates = animate_calls;
    Palette::timer_callback(4, nullptr);
    assert(animate_calls == old_animates);
    return 0;
}
'''
    return prefix


def compile_and_run(tmp_path: Path, source: str) -> subprocess.CompletedProcess[str]:
    compiler = shutil.which("c++") or shutil.which("clang++") or shutil.which("g++")
    assert compiler, "a host C++ compiler is required for actual-source controls"
    unit = tmp_path / "time_callback_actual.cpp"
    binary = tmp_path / "time_callback_actual"
    unit.write_text(source, encoding="utf-8")
    compiled = subprocess.run([compiler, "-std=c++17", "-O0", str(unit), "-o", str(binary)],
                              text=True, capture_output=True, check=False)
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    return subprocess.run([str(binary)], text=True, capture_output=True, check=False)


def test_complete_observed_callback_closure_uses_borrowed_context():
    time_h = SOURCES["src/time.h"].read_text()
    assert time_h.count("void *param2") == 3
    assert "void(__cdecl *callback2_)(int, void *)" in time_h
    assert "void *cb_param2_" in time_h
    assert "static_assert(sizeof(Time) == 0x28" in time_h
    assert "void init(void(__cdecl *callback)(int), int param" in time_h
    assert "uint32_t start(void(__cdecl *callback)(int), int param" in time_h
    assert "uint32_t pulse(void(__cdecl *callback)(int), int param" in time_h
    for name in ("src/time.cpp", "src/win.cpp", "src/basebutton.cpp", "src/flatbutton.cpp",
                 "src/diplopop.cpp", "src/jackal/palette.cpp", "src/basewin.cpp", "src/maininterface.cpp"):
        text = SOURCES[name].read_text()
        assert "reinterpret_cast<void (__cdecl *)(int, int)>" not in text or name == "src/win.cpp"
        if name != "src/flatbutton.cpp":
            assert "reinterpret_cast<int>(this)" not in text
    registrations = {
        "src/basewin.cpp": ("time_.init(BaseWin::timer_callback, 0, this, 50, 5);",),
        "src/diplopop.cpp": ("diplo_portrait_timer_callback, 0, this, 2000, 5);", "diplo_bioscan_timer_callback, 0, this, interval, 5);"),
        "src/jackal/palette.cpp": ("t->init(Palette::timer_callback, key, this,",),
        "src/maininterface.cpp": ("::main_interface_timer_callback, 0, this, 4000, 5);", "::main_interface_timer_callback, 1, this, 100, 5);"),
        "src/basebutton.cpp": ("base_button_timer_callback, 1, this", "base_button_timer_callback, 1, this", "base_button_timer_callback, 1, this", "base_button_timer_callback, 2, this"),
        "src/flatbutton.cpp": ("base_button_timer_callback, 0,\n                        static_cast<BaseButton *>(this), 1000, 5);",),
    }
    assert sum(len(expected) for expected in registrations.values()) == 11
    for name, fragments in registrations.items():
        source = SOURCES[name].read_text()
        for fragment in set(fragments):
            assert source.count(fragment) >= fragments.count(fragment)
    assert "if (!context) return;" in SOURCES["src/diplopop.cpp"].read_text()
    assert "void BaseWin::timer_callback(int, void *)" in SOURCES["src/basewin.cpp"].read_text()
    lifecycle = SOURCES["src/time_lifecycle.cpp"].read_text()
    assert "callback2_ = 0;" in lifecycle and "cb_param2_ = 0;" in lifecycle
    mappings = [row for row in read_file(SOURCES["src/time.cpp"])
                if row.address in {0x006162D0, 0x00616410, 0x00616590}]
    assert len(mappings) == 3
    assert all(row.mode is Mode.BODY and row.byte_exact for row in mappings)


def test_actual_setters_dispatch_and_six_callbacks(tmp_path: Path):
    result = compile_and_run(tmp_path, host_unit())
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize(
    ("label", "mutate"),
    [
        ("lost context store", lambda s: s.replace("cb_param2_ = param2;", "cb_param2_ = nullptr;", 1)),
        ("broken diplo null guard", lambda s: s.replace("if (!context) return;\n    static_cast<DiploPop *>(context)->bioscan_anim();", "static_cast<DiploPop *>(context)->bioscan_anim();", 1)),
        ("skip the reloaded callback", lambda s: s.replace("if (timer->callback2_ != nullptr)", "if (timer->callback2_ == nullptr)", 1)),
    ],
)
def test_actual_source_mutants_reject_damage(tmp_path: Path, label: str, mutate):
    source = host_unit()
    damaged = mutate(source)
    assert damaged != source, f"mutant did not alter source: {label}"
    result = compile_and_run(tmp_path, damaged)
    assert result.returncode != 0, f"actual-source controls accepted mutant: {label}"
