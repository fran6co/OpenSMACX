"""Actual-source host controls for MCIVideo's observed word-sized MCI id.

Each owner body is extracted from alphamovie.cpp. The fixture supplies only
the small SDK/application surface those bodies call; it does not replace them
with a Python behavioral model or infer the unobserved extent after byte +5.
"""

from __future__ import annotations

import re
import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]
METHODS = {
    "ctor": r"^MCIVideo::MCIVideo\s*\(\s*\)",
    "load": r"^int\s+MCIVideo::load\s*\(",
    "play": r"^void\s+MCIVideo::play\s*\(\s*\)",
    "stop": r"^void\s+MCIVideo::stop\s*\(\s*\)",
    "close": r"^void\s+MCIVideo::close\s*\(\s*\)",
    "status": r"^int\s+MCIVideo::status\s*\(\s*\)",
}


def _extract_body(source: str, signature: str) -> str:
    match = re.search(signature, source, re.MULTILINE)
    assert match, "actual owner definition missing: " + signature
    opening = source.find("{", match.end())
    assert opening >= 0
    depth = 0
    state = "code"
    i = opening
    while i < len(source):
        char = source[i]
        nxt = source[i + 1] if i + 1 < len(source) else ""
        if state == "code":
            if char == "/" and nxt == "/":
                state = "line"
                i += 2
                continue
            if char == "/" and nxt == "*":
                state = "block"
                i += 2
                continue
            if char == '"':
                state = "string"
            elif char == "'":
                state = "char"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[match.start():i + 1]
        elif state == "line":
            if char == "\n":
                state = "code"
        elif state == "block":
            if char == "*" and nxt == "/":
                state = "code"
                i += 2
                continue
        else:
            if char == "\\":
                i += 2
                continue
            if (state == "string" and char == '"') or (state == "char" and char == "'"):
                state = "code"
        i += 1
    raise AssertionError("unterminated owner definition: " + signature)


def _actual_sources():
    cpp = (ROOT / "src/alphamovie.cpp").read_text(encoding="utf-8")
    header = (ROOT / "src/alphamovie.h").read_text(encoding="utf-8")
    field = re.search(r"^\s*(uint16_t\s+mciId_\s*;)\s*//\s*0x0*4\s*$",
                      header, re.MULTILINE)
    unknown = re.search(r"^\s*(unsigned char\s+unknown_6_\s*\[2\]\s*;)\s*//\s*0x0*6,",
                        header, re.MULTILINE)
    assert field, "MCIVideo must declare the recovered 16-bit id at +4"
    assert unknown, "the two unknown adjacent bytes must remain explicit and untouched"
    return (field.group(1), unknown.group(1)), {name: _extract_body(cpp, pattern)
                                                for name, pattern in METHODS.items()}


HOST = r'''#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <new>
#include <vector>

typedef uint32_t DWORD;
typedef uintptr_t DWORD_PTR;
typedef uintptr_t UINT_PTR;
typedef intptr_t LONG;
typedef uint32_t MCIDEVICEID;
typedef uint32_t MCIERROR;
typedef void *HWND;
typedef void *HPALETTE;
struct Win {};
typedef intptr_t LRESULT;
typedef LRESULT (*WNDPROC)(void);
struct RECT { LONG left, top, right, bottom; };
struct PALETTEENTRY { unsigned char peRed, peGreen, peBlue, peFlags; };
struct MCI_DGV_OPEN_PARMS {
  DWORD_PTR dwCallback; const char *lpstrDeviceType; const char *lpstrElementName;
  DWORD dwStyle; HWND hWndParent; MCIDEVICEID wDeviceID;
};
struct MCI_DGV_STATUS_PARMS { DWORD_PTR dwCallback, dwReturn; DWORD dwItem; };
struct MCI_DGV_WHERE_PARMS { DWORD_PTR dwCallback; RECT rc; };
struct MCI_DGV_PUT_PARMS { DWORD_PTR dwCallback; RECT rc; };
struct MCI_DGV_WINDOW_PARMS { DWORD_PTR dwCallback; int nCmdShow; };
struct MCI_PLAY_PARMS { DWORD_PTR dwCallback; DWORD_PTR dwFrom, dwTo; };
struct MCI_GENERIC_PARMS { DWORD_PTR dwCallback; };
enum {
  MCI_OPEN=1, MCI_STATUS=2, MCI_WHERE=3, MCI_PUT=4, MCI_WINDOW=5,
  MCI_PLAY=6, MCI_STOP=7, MCI_CLOSE=8,
  MCI_OPEN_TYPE=0x100, MCI_OPEN_ELEMENT=0x200, MCI_DGV_OPEN_WS=0x400,
  MCI_DGV_OPEN_PARENT=0x800, MCI_STATUS_ITEM=0x1000,
  MCI_DGV_STATUS_HWND=0x11, MCI_DGV_STATUS_HPAL=0x12,
  MCI_DGV_WHERE_SOURCE=0x20, MCI_DGV_RECT=0x40,
  MCI_DGV_PUT_DESTINATION=0x80, MCI_DGV_WINDOW_STATE=0x100,
  MCI_NOTIFY=0x200, WS_CHILD=0x40000000, SW_SHOW=5, GWL_WNDPROC=-4, FALSE=0
};

int palette_ctor_calls=0;
struct Palette {
  PALETTEENTRY entries_[256];
  int init_calls, reseed_calls, set_calls;
  Palette() : init_calls(0), reseed_calls(0), set_calls(0) { ++palette_ctor_calls; }
  void init() { ++init_calls; }
  void reseed() { ++reseed_calls; }
  void set() { ++set_calls; }
};
struct WaveDevice { int disables, enables; WaveDevice() : disables(0), enables(0) {}
  void disable() { ++disables; } void enable() { ++enables; } };
WaveDevice WaveDeviceGlobal;
HWND HandleMain = reinterpret_cast<HWND>(0x4444);
struct Call { MCIDEVICEID device; DWORD command, flags; };
std::vector<Call> calls;
int fail_command = 0;
int mutate_id_after_first_status = 0;
int mutate_id_after_first_window = 0;
int mutate_id_after_first_stop_window = 0;
MCIDEVICEID opened_id = 0x1234BEEF;
DWORD status_hwnd = 0x1111, status_palette = 0x2222, status_value = 0x76543210;
int open_window_calls=0, move_window_calls=0, get_palette_calls=0;
HWND moved_window=0; LONG moved_x=0, moved_y=0, moved_width=0, moved_height=0;
struct MCIVideo;
MCIVideo *current_video=0;
MCIVideo *ActiveVideoPlayer=0;
HWND MciVideoCallbackWindow=0;
LRESULT MciVideoWindowProc(void) { return 0; }

MCIERROR mciSendCommandA(MCIDEVICEID, DWORD, DWORD, DWORD_PTR);
int GetPaletteEntries(HPALETTE, unsigned, unsigned, PALETTEENTRY *);
LONG SetWindowLongA(HWND, int, LONG);
int MoveWindow(HWND, int, int, int, int, int);

struct MCIVideo {
  uint32_t field_0_;
  // INSERT_ACTUAL_FIELD
  // INSERT_ACTUAL_UNKNOWN
  HWND window_handle_;
  HPALETTE palette_handle_;
  Palette palette_;
  WNDPROC previous_window_proc_;
  Win *target_window_;
  int blit_x_, blit_y_, blit_width_, blit_height_;
  MCIVideo();
  int load(char *, Win *, int, int);
  void play(); void stop(); void close(); int status();
};
'''


def _harness(members: tuple[str, str], bodies: dict[str, str], damage=None) -> str:
    field, unknown = members
    bodies = dict(bodies)
    if damage == "initialize_unknown_bytes":
        before = "mciId_ = 0;"
        assert before in bodies["ctor"]
        bodies["ctor"] = bodies["ctor"].replace(
            before, before + "\n    unknown_6_[0] = 0;", 1)
    elif damage == "truncate_to_byte":
        before = "static_cast<uint16_t>(openParams.wDeviceID)"
        assert before in bodies["load"]
        bodies["load"] = bodies["load"].replace(
            before, "static_cast<unsigned char>(openParams.wDeviceID)", 1)
    source = HOST.replace("// INSERT_ACTUAL_FIELD", field)
    source = source.replace("// INSERT_ACTUAL_UNKNOWN", unknown)
    source += r'''
MCIERROR mciSendCommandA(MCIDEVICEID device, DWORD command, DWORD flags, DWORD_PTR param) {
  Call call = {device, command, flags}; calls.push_back(call);
  if (command == MCI_OPEN && fail_command != MCI_OPEN) {
    MCI_DGV_OPEN_PARMS *p = reinterpret_cast<MCI_DGV_OPEN_PARMS *>(param);
    p->wDeviceID = opened_id;
  }
  if (command == MCI_STATUS && fail_command != MCI_STATUS) {
    MCI_DGV_STATUS_PARMS *p = reinterpret_cast<MCI_DGV_STATUS_PARMS *>(param);
    if (p->dwItem == MCI_DGV_STATUS_HWND) p->dwReturn = status_hwnd;
    else if (p->dwItem == MCI_DGV_STATUS_HPAL) p->dwReturn = status_palette;
    else p->dwReturn = status_value;
  }
  if (command == MCI_WHERE && fail_command != MCI_WHERE) {
    MCI_DGV_WHERE_PARMS *p = reinterpret_cast<MCI_DGV_WHERE_PARMS *>(param);
    p->rc.left=3; p->rc.top=4; p->rc.right=23; p->rc.bottom=34;
  }
  if (command == MCI_STATUS && mutate_id_after_first_status &&
      current_video && calls.size() == 2) current_video->mciId_ = 0x1357;
  if (command == MCI_WINDOW && mutate_id_after_first_window &&
      current_video && calls.size() == 1) current_video->mciId_ = 0x1357;
  if (command == MCI_WINDOW && mutate_id_after_first_stop_window &&
      current_video && calls.size() == 1) current_video->mciId_ = 0x1357;
  return command == fail_command ? 1 : 0;
}
int GetPaletteEntries(HPALETTE, unsigned, unsigned count, PALETTEENTRY *entries) {
  ++get_palette_calls;
  for (unsigned i=0; i<count; ++i) {
    entries[i].peRed=(unsigned char)i; entries[i].peGreen=(unsigned char)(i+1);
    entries[i].peBlue=(unsigned char)(i+2); entries[i].peFlags=0x5a;
  }
  return (int)count;
}
LONG SetWindowLongA(HWND, int, LONG) { return 0x7777; }
int MoveWindow(HWND window, int x, int y, int width, int height, int) {
  ++move_window_calls; moved_window=window; moved_x=x; moved_y=y;
  moved_width=width; moved_height=height; return 1;
}
'''
    source += "\n".join(bodies[name] for name in METHODS)
    source += r'''
static void reset_globals() {
  calls.clear(); fail_command=0; mutate_id_after_first_status=0;
  mutate_id_after_first_window=0; mutate_id_after_first_stop_window=0;
  opened_id=0x1234BEEF; status_hwnd=0x1111; status_palette=0x2222;
  status_value=0x76543210; open_window_calls=move_window_calls=get_palette_calls=0;
  moved_window=0; moved_x=moved_y=moved_width=moved_height=0;
  WaveDeviceGlobal.disables=WaveDeviceGlobal.enables=0;
  ActiveVideoPlayer=0; MciVideoCallbackWindow=0; current_video=0;
}
static void check(bool condition, const char *what) {
  if (!condition) { fprintf(stderr, "CHECK FAILED: %s\n", what); abort(); }
}

static void test_constructor_preserves_unknown_adjacent_word() {
  union Storage { long double alignment; unsigned char bytes[2048]; } storage;
  memset(storage.bytes, 0xA5, sizeof(storage.bytes));
  palette_ctor_calls=0;
  MCIVideo *video = new (storage.bytes) MCIVideo();
  current_video=video;
  check(palette_ctor_calls==1,
        "MCIVideo constructor naturally constructs its Palette member exactly once");
  check((unsigned char)storage.bytes[6] == 0xA5 &&
        (unsigned char)storage.bytes[7] == 0xA5,
        "constructor preserves unknown bytes +6..+7");
  check(video->mciId_ == 0, "constructor clears the word at +4");
  check(video->window_handle_ == 0 && video->palette_handle_ == 0 &&
        video->target_window_ == 0, "constructor clears observed pointer fields");
  video->~MCIVideo();
}

static void test_invalid_and_failed_open_paths() {
  reset_globals(); MCIVideo video; current_video=&video;
  Win window; video.mciId_=0x1234; video.blit_x_=91; video.blit_y_=92;
  check(video.load(0, &window, 4, 5) == 0x10, "null path error code");
  check(calls.empty() && WaveDeviceGlobal.disables==0 && video.mciId_==0x1234 &&
        video.blit_x_==91 && video.blit_y_==92, "null path exits before side effects");
  check(video.load((char*)"movie.avi", 0, 4, 5) == 0x10,
        "null target error code");
  check(calls.empty() && WaveDeviceGlobal.disables==0, "null target exits before side effects");

  reset_globals(); video.mciId_=0x4321; fail_command=MCI_OPEN;
  check(video.load((char*)"movie.avi", &window, 14, 15) == 1, "open failure result");
  check(calls.size()==1 && calls[0].command==MCI_OPEN && calls[0].device==0,
        "open failure makes only the open command");
  check(video.mciId_==0x4321, "failed open leaves prior device word unchanged");
  check(video.target_window_==&window && video.blit_x_==14 && video.blit_y_==15 &&
        WaveDeviceGlobal.disables==1 && MciVideoCallbackWindow==HandleMain,
        "failed open retains earlier state changes and wave disable");
}

static void test_load_truncation_repeated_reads_and_error_continuation() {
  reset_globals(); MCIVideo video; current_video=&video; Win window;
  video.mciId_=0x7777; video.unknown_6_[0]=0x5A; video.unknown_6_[1]=0xA5;
  opened_id=0x1234BEEF; fail_command=MCI_STATUS; mutate_id_after_first_status=1;
  check(video.load((char*)"movie.avi", &window, 8, 9)==0, "load success despite status errors");
  check(video.mciId_==0x1357, "MCI device word stores the low 16-bit returned id");
  check(video.unknown_6_[0]==0x5A && video.unknown_6_[1]==0xA5,
        "load does not initialize adjacent unknown bytes");
  const DWORD expected_commands[] = {MCI_OPEN,MCI_STATUS,MCI_STATUS,MCI_WHERE,MCI_PUT};
  // The body has five MCI calls; ancillary Win32 calls are separately observed.
  check(calls.size()==5, "load has exactly open, two status, where and put calls");
  for (size_t i=0; i<5; ++i)
    check(calls[i].command==expected_commands[i], "load MCI command order");
  check(calls[0].device==0 && calls[1].device==0xBEEF &&
        calls[2].device==0x1357 && calls[3].device==0x1357 &&
        calls[4].device==0x1357, "load re-reads id for every later API call");
  check(video.window_handle_==0 && video.palette_handle_==0,
        "failed status calls leave zero outputs but do not stop load");
  check(get_palette_calls==1 && video.palette_.init_calls==1 &&
        video.palette_.reseed_calls==1 && move_window_calls==1,
        "load continues palette and window work after ignored MCI errors");
  check(video.blit_width_==40 && video.blit_height_==60,
        "successful where result still drives dimensions after status errors");
  reset_globals(); current_video=&video; video.mciId_=0x7777; fail_command=MCI_WHERE;
  check(video.load((char*)"movie.avi", &window, 8, 9)==0,
        "where failure is ignored and load still succeeds");
  check(calls.size()==5 && calls[3].command==MCI_WHERE &&
        calls[4].command==MCI_PUT && move_window_calls==1 &&
        video.blit_width_==0 && video.blit_height_==0,
        "where failure leaves zero dimensions but continues through put/window calls");
}

static void test_status_returns_sdk_value_and_uses_word_device() {
  reset_globals(); MCIVideo video; current_video=&video; video.mciId_=0xABCD;
  status_value=0x76543210;
  check(video.status()==(int)status_value, "status returns the SDK dwReturn value");
  check(calls.size()==1 && calls[0].device==0xABCD && calls[0].command==MCI_STATUS &&
        calls[0].flags==MCI_STATUS_ITEM, "status uses zero-extended word id and flags");
  video.mciId_=0x9876;
  check(video.status()==(int)status_value && calls.size()==2 &&
        calls[1].device==0x9876, "status re-reads id on each invocation");
  video.mciId_=0;
  check(video.status()==(int)status_value && calls.size()==3 && calls[2].device==0,
        "status still dispatches MCI_STATUS for a zero id");
}

static void test_play_branches_order_and_repeated_device_reads() {
  reset_globals(); MCIVideo video; current_video=&video; video.mciId_=0xCAFE;
  video.field_0_=0x20; fail_command=MCI_WINDOW;
  video.play();
  check(calls.size()==1 && calls[0].command==MCI_WINDOW &&
        video.palette_.set_calls==1 && ActiveVideoPlayer==&video &&
        video.field_0_==0x20, "window failure returns after active/palette setup");

  reset_globals(); current_video=&video; video.mciId_=0xCAFE; video.field_0_=0x20;
  fail_command=MCI_PLAY; mutate_id_after_first_window=1;
  // Mutate the field after the first play-side API call to expose a cached id.
  video.play();
  check(calls.size()==2 && calls[0].command==MCI_WINDOW &&
        calls[1].command==MCI_PLAY && calls[0].device==0xCAFE &&
        calls[1].device==0x1357 && video.field_0_==0x20,
        "play failure preserves command order and does not set playing bit");

  reset_globals(); current_video=&video; video.mciId_=0; video.field_0_=0x20;
  fail_command=0; video.play();
  check(calls.size()==2 && calls[0].device==0 && calls[1].device==0 &&
        video.field_0_==0x21,
        "zero-id play still sends both commands and then sets bit zero");
}

static void test_stop_and_close_keep_distinct_zero_and_error_paths() {
  reset_globals(); MCIVideo video; current_video=&video;
  video.mciId_=0x1234; video.field_0_=0x21; ActiveVideoPlayer=&video;
  video.unknown_6_[0]=0x31; video.unknown_6_[1]=0x42;
  fail_command=MCI_WINDOW; mutate_id_after_first_stop_window=1;
  video.stop();
  check(calls.size()==3 && calls[0].command==MCI_WINDOW &&
        calls[1].command==MCI_STOP && calls[2].command==MCI_CLOSE,
        "stop continues all three commands after first error");
  check(calls[0].device==0x1234 && calls[1].device==0x1357 && calls[2].device==0x1357,
        "stop re-reads the word id for each native API call");
  check(video.mciId_==0 && video.field_0_==0x20 && ActiveVideoPlayer==0,
        "stop clears id/player/playing state after command sequence");
  check(video.unknown_6_[0]==0x31 && video.unknown_6_[1]==0x42,
        "nonzero stop preserves unknown adjacent bytes");
  reset_globals(); current_video=&video; video.mciId_=0; video.field_0_=0x21; ActiveVideoPlayer=&video;
  video.unknown_6_[0]=0x53; video.unknown_6_[1]=0x64;
  video.stop();
  check(calls.empty() && video.field_0_==0x21 && ActiveVideoPlayer==&video,
        "zero-id stop does not clear player or playing state");
  check(video.unknown_6_[0]==0x53 && video.unknown_6_[1]==0x64,
        "zero-id stop preserves unknown adjacent bytes");

  reset_globals(); current_video=&video; video.mciId_=0x9876; video.field_0_=0x21;
  video.unknown_6_[0]=0x75; video.unknown_6_[1]=0x86;
  video.window_handle_=(HWND)0x1111; video.palette_handle_=(HPALETTE)0x2222;
  video.target_window_=(Win*)0x3333; ActiveVideoPlayer=&video; fail_command=MCI_STOP;
  mutate_id_after_first_stop_window=1;
  video.close();
  check(calls.size()==3 && calls[0].command==MCI_WINDOW &&
        calls[1].command==MCI_STOP && calls[2].command==MCI_CLOSE,
        "close continues all commands after stop error");
  check(calls[0].device==0x9876 && calls[1].device==0x1357 &&
        calls[2].device==0x1357, "close re-reads the word id per API call");
  check(video.mciId_==0 && video.field_0_==0x20 && ActiveVideoPlayer==0,
        "nonzero close clears id/player/playing state");
  check(video.window_handle_==0 && video.palette_handle_==0 &&
        video.target_window_==0 && WaveDeviceGlobal.enables==1,
        "close clears observed handles and enables wave output");
  check(video.unknown_6_[0]==0x75 && video.unknown_6_[1]==0x86,
        "nonzero close preserves unknown adjacent bytes");
  reset_globals(); current_video=&video; video.mciId_=0; video.field_0_=0x21;
  video.unknown_6_[0]=0x97; video.unknown_6_[1]=0xA8;
  video.window_handle_=(HWND)0x1111; video.palette_handle_=(HPALETTE)0x2222;
  video.target_window_=(Win*)0x3333; ActiveVideoPlayer=&video;
  video.close();
  check(calls.empty() && video.mciId_==0 && video.field_0_==0x21 &&
        ActiveVideoPlayer==&video, "zero-id close skips stop-state clearing");
  check(video.window_handle_==0 && video.palette_handle_==0 &&
        video.target_window_==0 && WaveDeviceGlobal.enables==1,
        "zero-id close still clears handles and enables wave output");
  check(video.unknown_6_[0]==0x97 && video.unknown_6_[1]==0xA8,
        "zero-id close preserves unknown adjacent bytes");
}

int main() {
  static_assert(sizeof(MCIDEVICEID)==4, "SDK device-id argument is a widened value");
  static_assert(offsetof(MCIVideo, mciId_)==4, "device word stays at +4");
  static_assert(offsetof(MCIVideo, unknown_6_)==6, "unknown adjacent word starts +6");
  test_constructor_preserves_unknown_adjacent_word();
  test_invalid_and_failed_open_paths();
  test_load_truncation_repeated_reads_and_error_continuation();
  test_status_returns_sdk_value_and_uses_word_device();
  test_play_branches_order_and_repeated_device_reads();
  test_stop_and_close_keep_distinct_zero_and_error_paths();
  return 0;
}
'''
    return source


@pytest.fixture(scope="module")
def extracted_owner_source():
    return _actual_sources()


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_mci_owner_methods_preserve_word_storage_and_api_paths(
        tmp_path, compiler_name, extracted_owner_source):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " is unavailable")
    members, bodies = extracted_owner_source
    source = _harness(members, bodies)
    cpp = tmp_path / ("mci_actual_" + compiler_name + ".cpp")
    executable = tmp_path / ("mci_actual_" + compiler_name)
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                            "-Wno-missing-field-initializers", "-Wno-sign-compare",
                            str(cpp), "-o", str(executable)],
                           text=True, capture_output=True, timeout=45)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(executable)], text=True, capture_output=True, timeout=30)
    assert ran.returncode == 0, ran.stdout + ran.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("damage", ("initialize_unknown_bytes", "truncate_to_byte"))
def test_actual_mci_controls_reject_adjacent_write_and_wrong_truncation(
        tmp_path, compiler_name, damage, extracted_owner_source):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " is unavailable")
    members, bodies = extracted_owner_source
    source = _harness(members, bodies, damage=damage)
    cpp = tmp_path / ("mci_damage_" + damage + ".cpp")
    executable = tmp_path / ("mci_damage_" + damage)
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                            "-Wno-missing-field-initializers", "-Wno-sign-compare",
                            str(cpp), "-o", str(executable)],
                           text=True, capture_output=True, timeout=45)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(executable)], text=True, capture_output=True, timeout=30)
    assert ran.returncode != 0, "host control accepted damaged extracted MCI behavior"
