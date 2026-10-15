"""Actual-source controls for Win::OnSysCommand's inherited Sound field."""
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

BODY = _body((ROOT / "src/win.cpp").read_text(), "void __cdecl Win::OnSysCommand(")

HARNESS = r'''
#include <cassert>
#include <cstdint>
#include <vector>
#define __cdecl
typedef void (__cdecl *WinNullaryEventCallback)();
using HWND = void *;
using LONG = intptr_t;
std::vector<char> events;
struct Sound {
    int playing;
    int play_result;
    int is_playing() { events.push_back('s'); return playing; }
    int play() { events.push_back('p'); return play_result; }
};
struct AutoSound { protected: Sound *val_6_; };
struct Win : AutoSound {
    void (*field_404_)();
    int close_calls;
    void set_sound(Sound *sound) { val_6_ = sound; }
    void on_sys_close() { ++close_calls; events.push_back('c'); }
    static void __cdecl OnSysCommand(HWND, unsigned, int, int);
};
Win *WinCallbackWindow;
Win *lookup;
LONG GetWindowLongA(HWND, int) { return reinterpret_cast<LONG>(lookup); }
int DefWindowProcA(HWND, unsigned, unsigned, long) { events.push_back('d'); return 73; }
void hook() { events.push_back('h'); }
'''

CASES = r'''
int main() {
    Win win = {};
    Sound sound = {0, 17};
    win.set_sound(&sound);
    win.field_404_ = hook;
    lookup = &win;
    events.clear();
    Win::OnSysCommand(nullptr, 0xf060, 11, 12);
    assert(WinCallbackWindow == &win && win.close_calls == 1);
    assert(events == std::vector<char>({'h', 'c', 's', 'p'}));
    events.clear(); sound.playing = 1;
    Win::OnSysCommand(nullptr, 0xf060, 0, 0);
    assert(events == std::vector<char>({'h', 'c', 's'}));
    events.clear(); win.set_sound(nullptr);
    Win::OnSysCommand(nullptr, 0xf060, 0, 0);
    assert(events == std::vector<char>({'h', 'c'}));
    lookup = nullptr; events.clear();
    Win::OnSysCommand(nullptr, 0xf060, 0, 0);
    assert(events.empty());
    lookup = &win; events.clear();
    Win::OnSysCommand(nullptr, 0xf020, 0x1234, 0x5678);
    assert(events == std::vector<char>({'d'}));
}
'''

@pytest.mark.parametrize("compiler", COMPILERS or [None])
def test_actual_on_syscommand_callback_and_sound_order(tmp_path, compiler):
    if compiler is None:
        pytest.skip("host GCC or Clang required")
    source = tmp_path / "win_audio.cpp"
    source.write_text(HARNESS + BODY + CASES)
    binary = tmp_path / "win_audio"
    compiled = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra",
                               "-Werror", "-no-pie", str(source), "-o", str(binary)],
                              capture_output=True, text=True)
    assert compiled.returncode == 0, compiled.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True)
    assert ran.returncode == 0, ran.stdout + ran.stderr

def test_actual_body_uses_proved_owner_members():
    assert "self->field_404_" in BODY
    assert "Sound *child = self->val_6_;" in BODY
    assert "reinterpret_cast<char *>(self)" not in BODY
    source = (ROOT / "src/win.cpp").read_text()
    assert "owner->field_404_;" in source
    assert "sub_5f86a0(owner->val_6_);" in source
    assert "w->iSomeFlag_ & 1" in source
    assert "win_parent_->outer_rect_.left" in source
    assert "win_parent_->outer_rect_.top" in source
    assert "this->win_parent_->iFlags_ & 0x1000" in source
    assert "child->iFlags_ & 0x2000000" in source
    assert "w->poWinBase_" in source
    assert "unsigned char *const wb" not in source
    assert "reinterpret_cast<char *>(owner) + 0x404" not in source
    assert "reinterpret_cast<char *>(owner) + 0x18" not in source
