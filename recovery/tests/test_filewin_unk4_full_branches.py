import pathlib
import shutil
import subprocess

import pytest


ROOT = pathlib.Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src" / "jackal" / "filewin.cpp"


def extract_body(text, signature):
    start = text.index(signature)
    brace = text.index("{", start)
    depth = 0
    for cursor in range(brace, len(text)):
        if text[cursor] == "{":
            depth += 1
        elif text[cursor] == "}":
            depth -= 1
            if depth == 0:
                return text[start:cursor + 1]
    raise AssertionError("unbalanced candidate body")


def test_all_unk4_branch_bodies_from_frozen_candidate(tmp_path):
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    if not compilers:
        pytest.skip("neither g++ nor clang++ is installed")
    body = extract_body(SOURCE.read_text(), "void FileWin::UNK4()")
    harness = r'''
#include <assert.h>
#include <stddef.h>
#include <string.h>
#define __cdecl
typedef void *HANDLE;
#define INVALID_HANDLE_VALUE ((HANDLE)(long)-1)
struct WIN32_FIND_DATAA { char cFileName[260]; };
class Win;
class FileWin;
struct WinDialogNode { unsigned unknown_00_; Win *focus_owner_; void *allocation_;
  WinDialogNode *next_; WinDialogNode *previous_; unsigned char unknown_14_[8]; };
struct WinNodeList { WinDialogNode *head_; WinDialogNode *current_; int count_; };
static int close_calls, close_invalid_calls, close_valid_calls;
static int find_calls, find_result_count, find_index;
static int populate_calls, populate_result;
static int parse_calls, popup_calls, popup_response, popup_flags;
static int release_calls, set_focus_calls;
static HANDLE find_results[8];
static char find_names[8][260], populated_name[260], popup_message[260], popup_title[64];
static Win *last_dialog_focus;
static WinDialogNode edit_node, list_node;

class Win {
 public:
  WinNodeList list_;
  void release_modal() { ++release_calls; }
  void set_dialog_focus(Win *focus) { ++set_focus_calls; last_dialog_focus = focus; }
};
static Win *WinPendingFocus;
class EditBox : public Win {
 public:
  char text_[256]; unsigned field_B40_, field_B44_;
  void select_all_text() {
    field_B40_ = 0; field_B44_ = (unsigned)strlen(text_);
    if (WinPendingFocus != 0) WinPendingFocus->list_.current_ = &list_node;
  }
  void redraw() {}
};
class ListBox : public Win {};
class FileWin {
 public:
  char current_directory_[260], selected_path_[260], selected_basename_[260];
  char extension_filters_[5][5]; int extension_count_; unsigned field_638_;
  Win *modal_window_; EditBox edit_box_; ListBox list_box_;
  int populate_directory(char *path) {
    ++populate_calls; strcpy(populated_name, path); return populate_result;
  }
  void UNK4();
};
static HANDLE FindFirstFileA(const char *path, WIN32_FIND_DATAA *) {
  strcpy(find_names[find_calls], path);
  HANDLE result = find_index < find_result_count ? find_results[find_index] : INVALID_HANDLE_VALUE;
  ++find_calls; ++find_index; return result;
}
static int FindClose(HANDLE handle) {
  ++close_calls;
  if (handle == INVALID_HANDLE_VALUE) ++close_invalid_calls;
  else ++close_valid_calls;
  return 1;
}
static int GetCurrentDirectoryA(unsigned long, char *out) { strcpy(out, "C:\\cwd"); return 6; }
static void purge_spaces(char *) {}
static void parse_says(int, char *, int, int) { ++parse_calls; }
static int pop_caption_title(char *message, char *title, int flags, int (*)()) {
  ++popup_calls; strcpy(popup_message, message); strcpy(popup_title, title);
  popup_flags = flags; return popup_response;
}
''' + body + r'''
static void setup(FileWin &file, Win &modal, const char *name, int action) {
  memset(&file, 0, sizeof(file));
  memset(&modal, 0, sizeof(modal));
  strcpy(file.current_directory_, "C:\\cwd");
  strcpy(file.edit_box_.text_, name);
  file.field_638_ = action;
  file.modal_window_ = &modal;
  modal.list_.count_ = 0; modal.list_.current_ = 0;
  WinPendingFocus = &modal;
  find_calls = find_index = find_result_count = 0;
  close_calls = close_invalid_calls = close_valid_calls = 0;
  populate_calls = parse_calls = popup_calls = release_calls = set_focus_calls = 0;
  populate_result = popup_response = popup_flags = 0;
  populated_name[0] = popup_message[0] = popup_title[0] = 0;
  last_dialog_focus = 0;
}
int main() {
  {
    FileWin file; Win modal; setup(file, modal, "..", 0xC8);
    modal.list_.count_ = 1; edit_node.focus_owner_ = &file.edit_box_;
    modal.list_.current_ = &edit_node; populate_result = 0;
    file.UNK4();
    assert(populate_calls == 1 && strcmp(populated_name, "..") == 0);
    assert(find_calls == 0 && popup_calls == 0 && release_calls == 0);
    assert(last_dialog_focus == &file.edit_box_);
  }
  {
    FileWin file; Win modal; setup(file, modal, "map", 0xC8);
    file.extension_count_ = 2; strcpy(file.extension_filters_[0], ".SC");
    strcpy(file.extension_filters_[1], ".SAV");
    find_results[0] = INVALID_HANDLE_VALUE; find_results[1] = (HANDLE)5;
    find_results[2] = INVALID_HANDLE_VALUE; find_result_count = 3;
    file.UNK4();
    assert(find_calls == 3);
    assert(strcmp(find_names[0], "map.SC") == 0 && strcmp(find_names[1], "map.SAV") == 0);
    assert(strcmp(find_names[2], "map.SAV") == 0);
    assert(close_invalid_calls == 1 && close_valid_calls == 1);
    assert(parse_calls == 1 && popup_calls == 1);
    assert(strcmp(popup_title, "FILEWIN_LOAD") == 0 && popup_flags == 0x200000);
    assert(strcmp(popup_message, "C:\\cwd\\jackal") == 0);
    assert(release_calls == 0);
  }
  {
    FileWin file; Win modal; setup(file, modal, "folder", 0xC8);
    populate_result = 1; file.UNK4();
    assert(populate_calls == 1 && strcmp(populated_name, "folder") == 0);
    assert(find_calls == 0 && popup_calls == 0);
  }
  {
    FileWin file; Win modal; setup(file, modal, "save.SC", 0x64);
    file.extension_count_ = 1; strcpy(file.extension_filters_[0], ".SC");
    find_results[0] = (HANDLE)7; find_results[1] = (HANDLE)8; find_result_count = 2;
    popup_response = 1; file.UNK4();
    assert(popup_calls == 1 && parse_calls == 1 && release_calls == 0);
    assert(strcmp(popup_title, "FILEWIN_SAVEEXISTS") == 0 && popup_flags == 0x200040);
    assert(close_valid_calls == 2 && close_invalid_calls == 0);
  }
  {
    FileWin file; Win modal; setup(file, modal, "save.SC", 0x64);
    file.extension_count_ = 1; strcpy(file.extension_filters_[0], ".SC");
    find_results[0] = (HANDLE)7; find_results[1] = (HANDLE)8; find_result_count = 2;
    popup_response = 0; file.UNK4(); assert(release_calls == 1);
  }
  {
    FileWin file; Win modal; setup(file, modal, "bad:name.SC", 0x64);
    file.extension_count_ = 1; strcpy(file.extension_filters_[0], ".SC");
    find_results[0] = (HANDLE)7; find_results[1] = INVALID_HANDLE_VALUE; find_result_count = 2;
    modal.list_.count_ = 1; edit_node.focus_owner_ = &file.edit_box_;
    modal.list_.current_ = &edit_node;
    popup_response = 0; file.UNK4();
    assert(popup_calls == 1 && parse_calls == 0 && release_calls == 0);
    assert(strcmp(popup_title, "FILEWIN_ILLEGALCHAR") == 0 && popup_flags == 0x200040);
    assert(close_valid_calls == 1 && close_invalid_calls == 0);
    assert(last_dialog_focus == &file.edit_box_);
  }
}
'''
    source = tmp_path / "filewin_unk4_branches.cpp"
    source.write_text(harness)
    for compiler in compilers:
        binary = tmp_path / ("unk4-" + pathlib.Path(compiler).name)
        subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                        str(source), "-o", str(binary)], check=True, timeout=30)
        subprocess.run([str(binary)], check=True, timeout=30)
