"""C++98 host controls for the actual Win input-router definitions.

The test extracts only the three current definitions from the new source owner
and supplies bounded host Win/global stubs. It checks routing and argument
ordering, not VC6 code generation, Win32 callback registration, original layout,
or whole-game behavior.
"""
import os
from pathlib import Path
import re
import shutil
import subprocess

import pytest


HERE = Path(__file__).resolve()
_root = os.environ.get("OPENSMACX_ROOT")
ROOT = Path(_root).resolve() if _root else HERE.parents[2]
PRODUCT = ROOT / "src/win.cpp"
RETAINED = ROOT / "src/win_input.cpp"


def _mask_cpp(text):
    out = list(text)
    state = "code"
    i = 0
    while i < len(text):
        c = text[i]
        n = text[i + 1] if i + 1 < len(text) else ""
        if state == "code":
            if c == "/" and n == "/":
                out[i] = out[i + 1] = " "
                i += 2
                state = "line"
                continue
            if c == "/" and n == "*":
                out[i] = out[i + 1] = " "
                i += 2
                state = "block"
                continue
            if c == '"':
                out[i] = " "
                state = "string"
            elif c == "'":
                out[i] = " "
                state = "char"
        elif state == "line":
            if c == "\n":
                state = "code"
            else:
                out[i] = " "
        elif state == "block":
            out[i] = " "
            if c == "*" and n == "/":
                out[i + 1] = " "
                i += 2
                state = "code"
                continue
        else:
            out[i] = " "
            if c == "\\" and i + 1 < len(text):
                out[i + 1] = " "
                i += 2
                continue
            if (state == "string" and c == '"') or (state == "char" and c == "'"):
                state = "code"
        i += 1
    return "".join(out)


def _definition(text, signature, label):
    code = _mask_cpp(text)
    found = []
    for match in re.finditer(signature + r"\s*\(", code):
        depth = 1
        pos = match.end()
        while pos < len(code) and depth:
            depth += (code[pos] == "(") - (code[pos] == ")")
            pos += 1
        if depth:
            continue
        while pos < len(code) and code[pos].isspace():
            pos += 1
        if pos < len(code) and code[pos] == "{":
            found.append((match.start(), pos))
    assert len(found) == 1, "expected one actual definition: " + label
    start, opening = found[0]
    depth = 0
    for end in range(opening, len(code)):
        depth += (code[end] == "{") - (code[end] == "}")
        if depth == 0:
            body = text[start:end + 1]
            assert "BODY GOES HERE" not in body, label + " is unfinished"
            return body
    raise AssertionError("unterminated definition: " + label)


def _actual_definitions():
    assert PRODUCT.is_file(), "actual source owner is missing: " + str(PRODUCT)
    assert RETAINED.is_file(), "retained Win source is missing: " + str(RETAINED)
    product = PRODUCT.read_text()
    retained = RETAINED.read_text()
    assert "void __cdecl Win::OnKey(" not in retained
    assert "void __cdecl OnChar(" not in retained
    assert "void __cdecl OnSysKey(" not in retained
    assert "void __cdecl flush_input()" in retained
    assert "void __cdecl do_all_chars()" in retained
    return {
        "key": _definition(product, r"void\s+__cdecl\s+Win::OnKey", "Win::OnKey"),
        "char": _definition(product, r"void\s+__cdecl\s+OnChar", "OnChar"),
        "sys": _definition(product, r"void\s+__cdecl\s+OnSysKey", "OnSysKey"),
    }


PRELUDE = r'''#include <assert.h>
#define __cdecl
#define nullptr 0
typedef unsigned int WPARAM;

enum EventKind { EV_VISIBLE=1, EV_KEY=2, EV_CHAR=3, EV_SYSKEY=4,
                 EV_KEY_HOOK=5, EV_MESSAGE_HOOK=6 };
struct Event { int kind, receiver, a, b, c, d; };
static Event events[64];
static int event_count = 0;
static void record(int kind, int receiver, int a=0, int b=0, int c=0, int d=0) {
  assert(event_count < 64);
  events[event_count].kind=kind; events[event_count].receiver=receiver;
  events[event_count].a=a; events[event_count].b=b;
  events[event_count].c=c; events[event_count].d=d;
  ++event_count;
}

struct Win {
  int id;
  unsigned int iSomeFlag_;
  Win *poWinBase_;
  Win *win_parent_;
  int visible;
  Win() : id(0), iSomeFlag_(0), poWinBase_(0), win_parent_(0), visible(1) {}
  virtual int is_visible() { record(EV_VISIBLE, id); return visible; }
  virtual void on_key(unsigned int key, long flags, int repeat, unsigned int scan) {
    record(EV_KEY, id, (int)key, (int)flags, repeat, (int)scan);
  }
  virtual void on_char(char ch, int flags) {
    record(EV_CHAR, id, (unsigned char)ch, flags);
  }
  virtual int on_sys_key(unsigned int key, long flags, int repeat, unsigned int scan) {
    record(EV_SYSKEY, id, (int)key, (int)flags, repeat, (int)scan);
    return 0;
  }
  static void __cdecl OnKey(void *, unsigned int, long, int, unsigned int);
};

Win *WinFocusWindow=0, *WinModalFocus=0, *WinInputFocus=0;
Win *WinDefaultFocus=0, *WinActiveWindow=0;
static void __cdecl key_hook(WPARAM key) { record(EV_KEY_HOOK, 0, (int)key); }
static void __cdecl message_hook() { record(EV_MESSAGE_HOOK, 0); }
void (__cdecl *WinKeyHook)(WPARAM)=0;
void (__cdecl *WinMessageHook)()=0;

static void clear_events() { event_count=0; }
static void expect(int i, int kind, int receiver, int a=0, int b=0, int c=0, int d=0) {
  assert(i < event_count);
  assert(events[i].kind==kind && events[i].receiver==receiver);
  assert(events[i].a==a && events[i].b==b && events[i].c==c && events[i].d==d);
}
'''


MAIN = r'''
int main() {
  Win focus, modal, input, fallback, owner, parent;
  focus.id=1; modal.id=2; input.id=3; fallback.id=4;
  owner.id=5; parent.id=6;
  focus.poWinBase_=&owner; modal.poWinBase_=&owner; input.poWinBase_=&owner;
  owner.iSomeFlag_=1; owner.win_parent_=&parent;
  WinKeyHook=&key_hook; WinMessageHook=&message_hook;

  // Focus beats input; with no modal, the focus owner is used. Invisible
  // parent suppresses key dispatch but must not suppress either hook.
  WinFocusWindow=&focus; WinModalFocus=0; WinInputFocus=&input;
  WinDefaultFocus=&fallback; parent.visible=0;
  Win::OnKey(0, 0x41, 3, 2, 0x25);
  assert(WinActiveWindow==&focus && event_count==3);
  expect(0,EV_VISIBLE,parent.id); expect(1,EV_KEY_HOOK,0,0x41);
  expect(2,EV_MESSAGE_HOOK,0);

  // Modal selection, visible-parent dispatch, exact four key arguments, and
  // post-dispatch key-hook then message-hook ordering.
  clear_events(); WinModalFocus=&modal; parent.visible=1;
  Win::OnKey(0, 0x42, 7, 4, 0x31);
  assert(WinActiveWindow==&modal && event_count==4);
  expect(0,EV_VISIBLE,parent.id); expect(1,EV_KEY,owner.id,0x42,7,4,0x31);
  expect(2,EV_KEY_HOOK,0,0x42); expect(3,EV_MESSAGE_HOOK,0);

  // No focus/input uses the default. A null owner skips dispatch; zero flags
  // suppress only the key hook, not the unconditional message hook.
  clear_events(); WinFocusWindow=0; WinModalFocus=&modal;
  WinInputFocus=0; WinDefaultFocus=&fallback; fallback.poWinBase_=0;
  Win::OnKey(0, 0x43, 0, 0, 0x32);
  assert(WinActiveWindow==&fallback && event_count==1);
  expect(0,EV_MESSAGE_HOOK,0);

  // WM_CHAR focus fallback and null-parent bypass preserve the character and
  // flags as supplied to the actual source definition.
  clear_events(); WinFocusWindow=&focus; WinModalFocus=0;
  focus.poWinBase_=&owner; owner.win_parent_=0; owner.iSomeFlag_=1;
  OnChar(0, 'Q', 9);
  assert(WinActiveWindow==&focus && event_count==1);
  expect(0,EV_CHAR,owner.id,'Q',9);

  // A nonnull invisible parent suppresses WM_CHAR; visible parent dispatches.
  clear_events(); owner.win_parent_=&parent; parent.visible=0;
  OnChar(0, 'R', 10);
  assert(event_count==1); expect(0,EV_VISIBLE,parent.id);
  clear_events(); parent.visible=1;
  OnChar(0, 'S', 11);
  assert(event_count==2); expect(0,EV_VISIBLE,parent.id);
  expect(1,EV_CHAR,owner.id,'S',11);

  // WM_SYSKEY uses input focus when ordinary focus is null. An invisible
  // parent returns before dispatch; visible and null parents permit it.
  clear_events(); WinFocusWindow=0; WinModalFocus=&modal;
  WinInputFocus=&input; WinDefaultFocus=&fallback;
  input.poWinBase_=&owner; owner.win_parent_=&parent; parent.visible=0;
  OnSysKey(0, 0xA1, 0x22, 3, 0x4C);
  assert(WinActiveWindow==&input && event_count==1);
  expect(0,EV_VISIBLE,parent.id);
  clear_events(); parent.visible=1;
  OnSysKey(0, 0xA2, 0x23, 5, 0x4D);
  assert(event_count==2); expect(0,EV_VISIBLE,parent.id);
  expect(1,EV_SYSKEY,owner.id,0xA2,0x23,5,0x4D);
  clear_events(); owner.win_parent_=0;
  OnSysKey(0, 0xA3, 0x24, 6, 0x4E);
  assert(event_count==1); expect(0,EV_SYSKEY,owner.id,0xA3,0x24,6,0x4E);

  // The flag gate suppresses the receiver dispatch, without rewriting active.
  clear_events(); owner.iSomeFlag_=0;
  OnSysKey(0, 0xA4, 0x25, 7, 0x4F);
  assert(WinActiveWindow==&input && event_count==0);
  return 0;
}
'''


def _compile_and_run(tmp_path, compiler, bodies, should_pass):
    program = PRELUDE + "\n".join(bodies.values()) + MAIN
    cpp = tmp_path / "win_input_host.cpp"
    exe = tmp_path / "win_input_host"
    cpp.write_text(program)
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         "-Wno-unused-parameter", "-pedantic", str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)
    if should_pass:
        assert ran.returncode == 0, ran.stdout + ran.stderr
    else:
        assert ran.returncode != 0, "compile-success callback mutant survived"


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_win_input_callbacks_and_compile_success_mutants(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " is unavailable")
    actual = _actual_definitions()
    _compile_and_run(tmp_path, compiler, actual, should_pass=True)

    mutants = (
        ("key", "if (WinFocusWindow != nullptr)",
         "if (WinModalFocus != nullptr)"),
        ("char", "parent->is_visible() != 0", "parent->is_visible() == 0"),
        ("sys", "owner->on_sys_key(key, flags, repeat, scan);",
         "owner->on_sys_key(flags, key, repeat, scan);"),
    )
    for name, old, new in mutants:
        assert actual[name].count(old) == 1, "mutation anchor changed: " + name
        changed = dict(actual)
        changed[name] = changed[name].replace(old, new, 1)
        _compile_and_run(tmp_path, compiler, changed, should_pass=False)
