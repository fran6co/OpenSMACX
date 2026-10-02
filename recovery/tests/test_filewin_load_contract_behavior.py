"""Actual modal/load bodies: pointer identity, cancellation and hotkey restore.

SDK/window doubles do not establish target ABI, original stack or game runtime.
"""
from pathlib import Path
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


def definition(text, signature):
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        if text[end] == "{":
            depth += 1
        elif text[end] == "}":
            depth -= 1
            if depth == 0:
                return text[start:end + 1]
    raise AssertionError("unterminated actual body")


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_load_returns_full_path_or_null(compiler, tmp_path):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"host compiler unavailable: {compiler}")
    filewin = (ROOT / "src/filewin.cpp").read_text()
    alpha = (ROOT / "src/alphasave.cpp").read_text()
    bodies = "\n".join([
        definition(filewin, "char *FileWin::load()"),
        definition(filewin, "char *FileWin::run_file_dialog(bool saving)"),
        definition(alpha, "char *AlphaSave::load()"),
    ])
    harness = r'''
#include <assert.h>
#include <string.h>
typedef void *HANDLE;
static const HANDLE INVALID_HANDLE_VALUE = (HANDLE)-1;
struct WIN32_FIND_DATAA { char cFileName[260]; };
struct RECT { int left, top, right, bottom; };
static int cwd_reads, cwd_restored, modal_calls, hidden_calls;
static void GetCurrentDirectoryA(int n, char *out) {
  assert(n==260); ++cwd_reads; strcpy(out,"C:\\maps");
}
static void SetCurrentDirectoryA(char *out) {
  assert(strcmp(out,"C:\\maps")==0); ++cwd_restored;
}
static HANDLE FindFirstFileA(char *, WIN32_FIND_DATAA *) { return INVALID_HANDLE_VALUE; }
static void FindClose(HANDLE) { assert(false); }
static int _strcmpi(const char *a,const char *b) { return strcmp(a,b); }
struct ListBox {
  int UNK2(char *) { return -1; }
  void set_selected_pos(int,int) {} void redraw() {}
};
struct EditBox {
  void set_text(char *) {} void select_all_text() {} void redraw() {}
};
struct Modal {
  int *action; int result;
  void set_modal(int,int,int) {
    assert(*action==0xC8); ++modal_calls; *action=result;
  }
  void hide() { ++hidden_calls; }
};
struct FileWin {
  char previous_directory_[260],current_directory_[260];
  char selected_path_[260],selected_basename_[260],file_name_[260];
  char directory_[263],extension_filters_[5][5];
  int extension_count_,field_638_; Modal *modal_window_;
  ListBox list_box_; EditBox edit_box_;
  int populate_directory(char *) { return 0; }
  char *load(); char *run_file_dialog(bool);
};
static FileWin *ActiveFileWin;
struct BoxSpriteParams {};
static BoxSpriteParams InterfaceBoxStyles[51];
struct Buffer {
  RECT rectangle;
  Buffer() { rectangle.left=rectangle.top=0; rectangle.right=200; rectangle.bottom=100; }
  RECT *buffer_working_rect() { return &rectangle; }
  void box_sprite(RECT *,BoxSpriteParams *) {}
  void set_text_color(int,int,int,int) {} void set_font(void *,int,int,int) {}
  void write_cent_l(char *,RECT *,unsigned long) {}
};
static struct Console { bool is_visible() { return true; } } ConsoleGlobal;
static void *find_font(int,int) { return 0; }
static char StringTemp[1024];
static char *label_get(int) { return (char *)"Load"; }
static char *CharUpperA(char *s) { return s; }
static struct NetDaemon { int voice_hotkey_48_,voice_hotkey_4c_; } NetDaemonNet;
struct AlphaSave: Buffer { FileWin file_win_; char *load(); };
static AlphaSave *ActiveAlphaSave;
''' + bodies + r'''
int main() {
  AlphaSave dialog;
  FileWin &f=dialog.file_win_;
  memset(&f,0,sizeof(f));
  strcpy(f.file_name_,"old.SC");
  strcpy(f.selected_path_,"C:\\maps\\old.SC");
  strcpy(f.selected_basename_,"old.SC");
  strcpy(f.extension_filters_[0],".SC"); f.extension_count_=1;
  f.modal_window_=0;
  assert(dialog.load()==0);
  assert(modal_calls==0 && cwd_reads==0 && cwd_restored==0);
  assert(NetDaemonNet.voice_hotkey_48_==0xDC && NetDaemonNet.voice_hotkey_4c_==0x100DC);
  Modal modal; modal.action=&f.field_638_; modal.result=0x12C;
  f.modal_window_=&modal;
  assert(dialog.load()==0);
  assert(strcmp(f.file_name_,"old.SC")==0);
  assert(strcmp(f.selected_path_,"C:\\maps\\old.SC")==0);
  assert(modal_calls==1 && hidden_calls==1 && cwd_restored==1);
  modal.result=0xC8;
  char *selected=dialog.load();
  assert(selected==f.selected_path_ && selected!=f.file_name_);
  assert(strcmp(selected,"C:\\maps\\old.SC")==0);
  assert(modal_calls==2 && hidden_calls==2 && cwd_restored==2);
  assert(NetDaemonNet.voice_hotkey_48_==0xDC && NetDaemonNet.voice_hotkey_4c_==0x100DC);
  assert(ActiveAlphaSave==&dialog && ActiveFileWin==&f);
  return 0;
}
'''
    cpp = tmp_path / "actual.cpp"
    binary = tmp_path / "control"
    cpp.write_text(harness)
    built = subprocess.run([
        executable, "-std=c++98", "-Wall", "-Wextra", "-Werror",
        str(cpp), "-o", str(binary),
    ], capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)
    assert ran.returncode == 0, ran.stdout + ran.stderr
