"""Win::OnSysCommand is a void message handler; test effects, not EAX."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
COMPILERS = [p for n in ("g++", "clang++") if (p := shutil.which(n))]


def _body(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated OnSysCommand body")


SOURCE = (ROOT / "src/win.cpp").read_text()
BODY = _body(SOURCE, "void __cdecl Win::OnSysCommand(")
HEADER = (ROOT / "src/win.h").read_text()

HARNESS = r'''
#include <cassert>
#include <cstdint>
#include <vector>
#define __cdecl
typedef void (__cdecl *WinNullaryEventCallback)();
using HWND = void *;
using LONG = intptr_t;
std::vector<char> events;
int last_is_playing_id = -1;
int last_play_id = -1;
struct Sound {
    int id;
    int playing;
    int play_result;
    int is_playing() { events.push_back('s'); last_is_playing_id = id; return playing; }
    int play() { events.push_back('p'); last_play_id = id; return play_result; }
};
struct AutoSound { protected: Sound *val_6_; };
struct Win : AutoSound {
    void (*field_404_)();
    int close_calls;
    Sound *replacement;
    void set_sound(Sound *sound) { val_6_ = sound; }
    void on_sys_close() {
        ++close_calls;
        events.push_back('c');
        if (replacement != nullptr) val_6_ = replacement;
    }
    static void __cdecl OnSysCommand(HWND, unsigned, int, int);
};
Win *WinCallbackWindow;
Win *lookup;
HWND seen_hwnd;
unsigned seen_message, seen_command;
long seen_lparam;
LONG GetWindowLongA(HWND, int) { return reinterpret_cast<LONG>(lookup); }
int DefWindowProcA(HWND hwnd, unsigned message, unsigned command, long lparam) {
    events.push_back('d');
    seen_hwnd = hwnd;
    seen_message = message;
    seen_command = command;
    seen_lparam = lparam;
    return -73;
}
void hook() { events.push_back('h'); }
'''

CASES = r'''
int main() {
    Win win = {};
    Sound initial = {1, 0, -27};
    Sound replacement = {2, 0, -19};
    win.set_sound(&initial);
    win.field_404_ = hook;
    lookup = &win;

    // A stopped child is played, but its int result is not a handler result.
    events.clear();
    Win::OnSysCommand(nullptr, 0xf060, 11, 12);
    assert(WinCallbackWindow == &win && win.close_calls == 1);
    assert(events == std::vector<char>({'h', 'c', 's', 'p'}));

    // Nonzero/negative is_playing values skip play; no EAX contract asserted.
    events.clear(); initial.playing = -6;
    Win::OnSysCommand(nullptr, 0xf060, 0, 0);
    assert(events == std::vector<char>({'h', 'c', 's'}));

    // The post-close reload observes a Sound replacement made during close.
    events.clear(); initial.playing = 1; win.replacement = &replacement;
    Win::OnSysCommand(nullptr, 0xf060, 0, 0);
    assert(events == std::vector<char>({'h', 'c', 's', 'p'}));
    assert(last_is_playing_id == replacement.id && last_play_id == replacement.id);

    // A null child performs no Sound calls; do not assert residual EAX.
    events.clear(); win.replacement = nullptr; win.set_sound(nullptr);
    Win::OnSysCommand(nullptr, 0xf060, 0, 0);
    assert(events == std::vector<char>({'h', 'c'}));

    // A null receiver returns before callbacks; no numeric return is asserted.
    lookup = nullptr; events.clear();
    Win::OnSysCommand(nullptr, 0xf060, 0, 0);
    assert(events.empty());

    // Forwarding packs the signed coordinate words and ignores DefWindowProc's result.
    lookup = &win; events.clear();
    HWND hwnd = reinterpret_cast<HWND>(0x1234);
    Win::OnSysCommand(hwnd, 0xf020, -2, -3);
    assert(seen_hwnd == hwnd && seen_message == 0x112 && seen_command == 0xf020);
    assert(static_cast<unsigned long>(seen_lparam) == 0xfffdfffeUL);
    assert(events == std::vector<char>({'d'}));
}
'''


@pytest.mark.parametrize("compiler", COMPILERS or [None])
def test_actual_source_void_handler_effects_and_forwarding(tmp_path, compiler):
    if compiler is None:
        pytest.skip("host GCC or Clang required")
    source = tmp_path / "win_syscommand.cpp"
    source.write_text(HARNESS + BODY + CASES)
    binary = tmp_path / "win_syscommand"
    compiled = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", "-no-pie",
         str(source), "-o", str(binary)], capture_output=True, text=True)
    assert compiled.returncode == 0, compiled.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True)
    assert ran.returncode == 0, ran.stdout + ran.stderr


def test_header_and_actual_body_keep_void_contract_without_result_claims():
    assert "static void __cdecl OnSysCommand(HWND hwnd, unsigned int command, int x, int y);" in HEADER
    assert "void __cdecl Win::OnSysCommand(" in BODY
    assert "return child->play()" not in BODY
    assert "return playing" not in BODY
    assert "return DefWindowProcA" not in BODY


@pytest.mark.parametrize("compiler", COMPILERS or [None])
@pytest.mark.parametrize("mutation", ("cache-before-callbacks", "drop-play", "drop-callback"))
def test_actual_source_controls_reject_behavior_mutants(tmp_path, compiler, mutation):
    if compiler is None:
        pytest.skip("host GCC or Clang required")
    mutant = BODY
    if mutation == "cache-before-callbacks":
        late_load = "    Sound *child = self->val_6_;\n"
        assert mutant.count(late_load) == 1
        mutant = mutant.replace(late_load, "", 1)
        anchor = "    WinCallbackWindow = self;\n"
        assert mutant.count(anchor) == 1
        mutant = mutant.replace(anchor, late_load + anchor, 1)
    elif mutation == "drop-play":
        assert mutant.count("            child->play();\n") == 1
        mutant = mutant.replace("            child->play();\n", "", 1)
    else:
        callback_arm = "        cb();\n"
        assert mutant.count(callback_arm) == 1
        mutant = mutant.replace(callback_arm, "        (void)cb;\n", 1)

    source = tmp_path / f"win_syscommand_{mutation}.cpp"
    source.write_text(HARNESS + mutant + CASES)
    binary = tmp_path / f"win_syscommand_{mutation}"
    compiled = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", "-no-pie",
         str(source), "-o", str(binary)], capture_output=True, text=True)
    assert compiled.returncode == 0, compiled.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True)
    assert ran.returncode != 0, f"{mutation} mutant unexpectedly passed"
