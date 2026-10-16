"""Keep the mapped FileWin save/load bodies independent and behavior-faithful."""
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
def test_actual_filewin_save_and_load_bodies(compiler, tmp_path):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"host compiler unavailable: {compiler}")
    source = (ROOT / "src/jackal/filewin.cpp").read_text()
    header = (ROOT / "src/jackal/filewin.h").read_text()
    save = definition(source, "char *FileWin::save()")
    load = definition(source, "char *FileWin::load()")
    setter = definition(source, "int FileWin::set_def_name(char *name)")
    assert "run_file_dialog" not in source + header
    assert "// ORIGINAL: 0x00613D50 ?save@FileWin@@QAEXXZ 0x00613D50-0x00613F5C" in source
    assert "// ORIGINAL: 0x00613F60 ?load@FileWin@@QAEXXZ 0x00613F60-0x00614133" in source
    assert "set_def_name(0);" in save
    assert "set_def_name(0);" in load
    assert "edit_box_.set_text(file_name_)" in save
    assert "edit_box_.set_text(file_name_)" not in load

    bodies = "\n".join((save, load, setter))
    harness = r'''
#include <assert.h>
#include <string.h>
typedef void *HANDLE;
static const HANDLE INVALID_HANDLE_VALUE = (HANDLE)-1;
struct WIN32_FIND_DATAA { char cFileName[260]; };
static char current_directory[260];
static char searched_name[260];
static int get_cwd_calls, set_cwd_calls, find_calls, find_close_calls;
static int modal_calls, hidden_calls, selected_calls, list_redraws;
static int edit_set_calls, edit_redraws, selected_all_calls, find_valid;
static int list_position;
static void GetCurrentDirectoryA(int size, char *out) {
  assert(size == 260); ++get_cwd_calls; strcpy(out, current_directory);
}
static int SetCurrentDirectoryA(char *path) {
  ++set_cwd_calls; strcpy(current_directory, path); return 1;
}
static HANDLE FindFirstFileA(char *path, WIN32_FIND_DATAA *out) {
  ++find_calls; strcpy(searched_name, path);
  if (!find_valid) return INVALID_HANDLE_VALUE;
  strcpy(out->cFileName, "found.SC"); return (HANDLE)1;
}
static void FindClose(HANDLE handle) {
  assert(handle == (HANDLE)1); ++find_close_calls;
}
static int _strcmpi(const char *left, const char *right) {
  while (*left && *right) {
    char a = *left >= 'A' && *left <= 'Z' ? *left + ('a' - 'A') : *left;
    char b = *right >= 'A' && *right <= 'Z' ? *right + ('a' - 'A') : *right;
    if (a != b) return (unsigned char)a - (unsigned char)b;
    ++left; ++right;
  }
  return (unsigned char)*left - (unsigned char)*right;
}
struct ListBox {
  int UNK2(char *name) { assert(strcmp(name, "found.SC") == 0); return list_position; }
  void set_selected_pos(int position, int selected) {
    assert(position == list_position && selected == 1); ++selected_calls;
  }
  void redraw() { ++list_redraws; }
};
struct EditBox {
  char text[260]; int field_B40_, field_B44_;
  void set_text(char *value) { ++edit_set_calls; strcpy(text, value); }
  void select_all_text() {
    ++selected_all_calls; field_B40_ = 0; field_B44_ = (int)strlen(text);
  }
  void redraw() { ++edit_redraws; }
};
struct Modal {
  int *action; int result;
  void set_modal(int enabled, int a, int b) {
    assert(enabled == 1 && a == 0 && b == 0); ++modal_calls;
    *action = result; strcpy(current_directory, "C:\\after-modal");
  }
  void hide() { ++hidden_calls; }
};
struct FileWin {
  char previous_directory_[260], current_directory_[260];
  char selected_path_[260], selected_basename_[260], file_name_[260];
  char directory_[263], extension_filters_[5][5];
  int extension_count_, field_638_; Modal *modal_window_;
  ListBox list_box_; EditBox edit_box_;
  int populate_directory(char *) {
    strcpy(current_directory, "C:\\inside-dialog"); return 0;
  }
  int set_def_name(char *name);
  char *save(); char *load();
};
static FileWin *ActiveFileWin;
''' + bodies + r'''
static void reset_counters() {
  get_cwd_calls = set_cwd_calls = find_calls = find_close_calls = 0;
  modal_calls = hidden_calls = selected_calls = list_redraws = 0;
  edit_set_calls = edit_redraws = selected_all_calls = 0;
  find_valid = 0; list_position = -1;
  strcpy(current_directory, "C:\\original"); searched_name[0] = 0;
}
static void initialize(FileWin &f, Modal &modal) {
  memset(&f, 0, sizeof(f));
  modal.action = &f.field_638_; modal.result = 0x12C; f.modal_window_ = &modal;
  strcpy(f.selected_path_, "stale-selected-path");
}
int main() {
  FileWin f; Modal modal;

  // No suffix gets the first configured extension. Save's failed lookup
  // populates/selects the editor; cancellation preserves both path buffers.
  reset_counters(); initialize(f, modal);
  strcpy(f.file_name_, "draft"); strcpy(f.extension_filters_[0], ".SC");
  f.extension_count_ = 1;
  assert(f.save() == 0);
  assert(strcmp(searched_name, "draft.SC") == 0);
  assert(edit_set_calls == 1 && strcmp(f.edit_box_.text, "draft.SC") == 0);
  assert(selected_all_calls == 1 && f.edit_box_.field_B40_ == 0 &&
         f.edit_box_.field_B44_ == 8 && edit_redraws == 1);
  assert(find_close_calls == 0 && modal_calls == 1 && hidden_calls == 1);
  assert(get_cwd_calls == 1 && set_cwd_calls == 1);
  assert(strcmp(current_directory, "C:\\original") == 0);
  assert(strcmp(f.selected_path_, "stale-selected-path") == 0);

  // Load clears the filename after the first mismatch, but continues using
  // the suffix pointer: a later matching filter breaks with an empty name.
  reset_counters(); initialize(f, modal); modal.result = 0xC8;
  strcpy(f.file_name_, "old.BAD");
  strcpy(f.extension_filters_[0], ".SC");
  strcpy(f.extension_filters_[1], ".BAD"); f.extension_count_ = 2;
  strcpy(f.directory_, "C:\\saved-dir"); list_position = 4;
  find_valid = 1;
  assert(f.load() == f.selected_path_);
  assert(strcmp(searched_name, "") == 0 && f.file_name_[0] == 0);
  assert(find_calls == 1 && find_close_calls == 1);
  assert(selected_calls == 1 && list_redraws == 1);
  assert(edit_set_calls == 0 && edit_redraws == 0);
  assert(strcmp(f.directory_, "C:\\after-modal") == 0);
  assert(get_cwd_calls == 2 && set_cwd_calls == 1);
  assert(strcmp(current_directory, "C:\\original") == 0);
  assert(ActiveFileWin == &f && modal_calls == 1 && hidden_calls == 1);

  // With zero filters load leaves a dotted name unchanged on lookup failure.
  reset_counters(); initialize(f, modal);
  strcpy(f.file_name_, "old.BAD"); f.extension_count_ = 0;
  assert(f.load() == 0);
  assert(strcmp(searched_name, "old.BAD") == 0);
  assert(strcmp(f.file_name_, "old.BAD") == 0);
  assert(edit_set_calls == 0 && find_close_calls == 0);
  assert(get_cwd_calls == 1 && set_cwd_calls == 1);

  // Save mismatch is cleared only after the complete filter scan.
  reset_counters(); initialize(f, modal);
  strcpy(f.file_name_, "old.BAD"); strcpy(f.extension_filters_[0], ".SC");
  f.extension_count_ = 1;
  assert(f.save() == 0);
  assert(f.file_name_[0] == 0 && strcmp(searched_name, "") == 0);
  assert(edit_set_calls == 1 && f.edit_box_.text[0] == 0);

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
