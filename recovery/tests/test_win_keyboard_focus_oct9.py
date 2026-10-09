"""Actual-source controls for Win's three keyboard-focus routes."""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/win.cpp"
COMPILERS = [path for name in ("g++", "clang++")
             if (path := shutil.which(name)) is not None]


def _body(text, signature):
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError(f"unterminated function {signature}")


def _actual_source(text=None):
    text = SOURCE.read_text() if text is None else text
    get_key = _body(text, "Win *Win::get_key_window()")
    on_char = _body(text, "void __cdecl OnChar(void *hwnd, char ch, int flags)")
    on_key = _body(text, "void __cdecl Win::OnKey(void * hwnd, unsigned int key, long flags, int repeat, unsigned int scan)")
    for body in (get_key, on_char, on_key):
        assert "p_9b" not in body
        assert "reinterpret_cast" not in body
        assert "(char *)" not in body
    return get_key + "\n" + on_char + "\n" + on_key


HARNESS = r'''
#include <cassert>
#include <cstdint>
#define __cdecl
using WPARAM = unsigned int;
static int events[16], event_count;
struct Win {
    Win *poWinBase_;
    unsigned int iSomeFlag_, iFlags_;
    Win *win_parent_;
    int visible, visible_calls;
    Win *visible_receiver;
    int char_calls, key_calls;
    char char_value;
    int char_flags;
    unsigned int key_value;
    long key_flags;
    int key_repeat;
    unsigned int key_scan;
    int is_visible() { ++visible_calls; visible_receiver=this; events[event_count++]=1; return visible; }
    void on_char(char ch, int flags) { ++char_calls; char_value=ch; char_flags=flags; events[event_count++]=2; }
    void on_key(unsigned int key, long flags, int repeat, unsigned int scan) {
        ++key_calls; key_value=key; key_flags=flags; key_repeat=repeat; key_scan=scan; events[event_count++]=3;
    }
    Win *get_key_window();
    void OnKey(void *, unsigned int, long, int, unsigned int);
};
static Win *WinFocusWindow, *WinModalFocus, *WinInputFocus, *WinDefaultFocus, *WinActiveWindow;
static void (__cdecl *WinKeyHook)(WPARAM);
static void (__cdecl *WinMessageHook)();
static Win *replacement_focus;
static void key_hook(WPARAM key) { assert(key == 0x80000011U); events[event_count++]=4; WinFocusWindow=replacement_focus; WinMessageHook=nullptr; }
static void message_hook() { events[event_count++]=5; }
'''


def _compile_and_run(tmp_path, body, compiler):
    tmp_path.mkdir(parents=True, exist_ok=True)
    source = tmp_path / "win_keyboard_focus.cpp"
    source.write_text(HARNESS + body + r'''
static void clear_globals() {
    WinFocusWindow=WinModalFocus=WinInputFocus=WinDefaultFocus=WinActiveWindow=nullptr;
    WinKeyHook=nullptr; WinMessageHook=nullptr; replacement_focus=nullptr;
    event_count=0;
}
int main() {
    Win focus{}, modal{}, input{}, fallback{}, base{}, parent{};
    focus.poWinBase_=&base; modal.poWinBase_=&base; input.poWinBase_=&base; fallback.poWinBase_=&base;
    base.poWinBase_=&base;
    base.iSomeFlag_=1;

    // Modal wins over a non-null focus pointer; each fallback branch is distinct.
    clear_globals(); WinFocusWindow=&focus; WinModalFocus=&modal;
    assert(focus.get_key_window()==&base && WinActiveWindow==&modal);
    clear_globals(); WinFocusWindow=&focus;
    assert(focus.get_key_window()==&base && WinActiveWindow==&focus);
    clear_globals(); WinInputFocus=&input;
    assert(focus.get_key_window()==&base && WinActiveWindow==&input);
    clear_globals(); WinModalFocus=&modal; WinInputFocus=&input;
    assert(focus.get_key_window()==&base && WinActiveWindow==&input);
    clear_globals(); WinDefaultFocus=&fallback;
    assert(focus.get_key_window()==&base && WinActiveWindow==&fallback);
    clear_globals(); WinModalFocus=&modal; WinDefaultFocus=&fallback;
    assert(focus.get_key_window()==&base && WinActiveWindow==&fallback);
    clear_globals(); assert(focus.get_key_window()==nullptr && WinActiveWindow==nullptr);

    // The free/member keyboard handlers use their own focus/modal branch:
    // modal is consulted only when focus is non-null, and otherwise the
    // input/default fallback is selected.
    clear_globals(); WinFocusWindow=&focus; WinModalFocus=&modal;
    OnChar(nullptr, 'm', 1); assert(WinActiveWindow==&modal);
    clear_globals(); WinFocusWindow=&focus;
    focus.OnKey(nullptr, 1, 0, 0, 0); assert(WinActiveWindow==&focus);
    clear_globals(); WinModalFocus=&modal; WinInputFocus=&input;
    OnChar(nullptr, 'i', 1); assert(WinActiveWindow==&input);
    clear_globals(); WinModalFocus=&modal; WinDefaultFocus=&fallback;
    focus.OnKey(nullptr, 1, 0, 0, 0); assert(WinActiveWindow==&fallback);

    // The bit is iSomeFlag_+9C, not the neighboring iFlags_ word.
    clear_globals(); WinFocusWindow=&focus; base.iSomeFlag_=0; base.iFlags_=1;
    assert(focus.get_key_window()==nullptr);
    base.iSomeFlag_=1; base.iFlags_=0;
    assert(focus.get_key_window()==&base);

    // Null inner window suppresses all three dispatches.
    focus.poWinBase_=nullptr; clear_globals(); WinFocusWindow=&focus;
    assert(focus.get_key_window()==nullptr);
    int before_null_chars=base.char_calls, before_null_keys=base.key_calls;
    OnChar(nullptr, 'x', 7); focus.OnKey(nullptr, 9, 2, 3, 4);
    assert(base.char_calls==before_null_chars && base.key_calls==before_null_keys);
    focus.poWinBase_=&base; base.iSomeFlag_=1;

    // get_key_window checks the parent pointer, but calls visibility on the
    // base itself. OnChar/OnKey call visibility on the parent itself.
    base.win_parent_=&parent; base.visible=0; parent.visible=1;
    clear_globals(); WinFocusWindow=&focus;
    assert(focus.get_key_window()==nullptr);
    assert(base.visible_receiver==&base);
    int old_chars=base.char_calls, old_keys=base.key_calls;
    OnChar(nullptr, static_cast<char>(0xFE), 0x12345678);
    assert(base.char_calls==old_chars+1 && base.char_value==static_cast<char>(0xFE));
    assert(base.char_flags==0x12345678 && parent.visible_receiver==&parent);
    focus.OnKey(nullptr, 0x80000011U, 1, -7, 0xFEDCBA98U);
    assert(base.key_calls==old_keys+1 && base.key_value==0x80000011U);
    assert(base.key_flags==1 && base.key_repeat==-7 && base.key_scan==0xFEDCBA98U);

    // Parent visibility false suppresses char/key even when base is visible.
    base.visible=1; parent.visible=0;
    old_chars=base.char_calls; old_keys=base.key_calls;
    OnChar(nullptr, 'q', 2); focus.OnKey(nullptr, 11, 3, 4, 5);
    assert(base.char_calls==old_chars && base.key_calls==old_keys);

    // Hook order and reread: the key hook replaces the message hook before
    // the latter is loaded; zero flags skip only the key hook.
    base.win_parent_=nullptr; base.iSomeFlag_=1;
    base.visible=1; event_count=0; clear_globals(); WinFocusWindow=&focus;
    WinKeyHook=&key_hook; WinMessageHook=&message_hook; replacement_focus=&modal;
    focus.OnKey(nullptr, 0x80000011U, 1, 2, 3);
    assert(event_count==2 && events[0]==3 && events[1]==4);
    assert(WinActiveWindow==&focus && WinFocusWindow==&modal);
    event_count=0; WinFocusWindow=&focus; WinKeyHook=&key_hook; WinMessageHook=&message_hook;
    focus.OnKey(nullptr, 0x80000011U, 0, 2, 3);
    assert(event_count==2 && events[0]==3 && events[1]==5);
}
''')
    binary = tmp_path / "win_keyboard_focus"
    built = subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                            "-Wno-unused-parameter", str(source), "-o", str(binary)],
                           capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True)
    assert ran.returncode == 0, ran.stdout + ran.stderr


@pytest.mark.parametrize("compiler", COMPILERS or [None],
                         ids=lambda value: Path(value).name if value else "no-host-cxx")
def test_actual_focus_routes_preserve_selection_members_receivers_and_hooks(tmp_path, compiler):
    if compiler is None:
        pytest.skip("host GCC or Clang required")
    _compile_and_run(tmp_path, _actual_source(), compiler)


def test_actual_source_mutants_break_receiver_and_hook_controls(tmp_path):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host GCC or Clang required")
    text = SOURCE.read_text()
    mutations = (
        text.replace("base_window->is_visible() == 0", "base_window->win_parent_->is_visible() == 0", 1),
        text.replace("if (WinKeyHook != nullptr && flags != 0)", "if (WinKeyHook != nullptr)", 1),
    )
    for index, mutant in enumerate(mutations):
        assert mutant != text
        with pytest.raises(AssertionError):
            _compile_and_run(tmp_path / str(index), _actual_source(mutant), compiler)
