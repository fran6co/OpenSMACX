#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>

typedef void *HWND;
typedef void *HANDLE;
typedef intptr_t LPARAM;
typedef uintptr_t WPARAM;
typedef unsigned int UINT;
typedef unsigned long DWORD;
typedef int BOOL;
typedef long LRESULT;
struct WIN32_FIND_DATAA { char cFileName[260]; };

static const UINT LB_GETCOUNT = 0x18B;
static const UINT LB_GETTEXT = 0x189;
static std::vector<std::string> directory_list;
static std::vector<std::string> staged_entries;
static int dialog_list_calls;
static int last_error_calls;
static int set_directory_calls;
static BOOL set_directory_results[2];
static int find_calls;
static int find_close_calls;
static bool fail_first_find;
static bool list_fail_first;
static bool list_fail_second;

static DWORD GetCurrentDirectoryA(DWORD size, char *buffer) {
  const char previous[] = "C:\\root";
  assert(size == 260);
  strcpy(buffer, previous);
  return static_cast<DWORD>(sizeof(previous) - 1);
}

static BOOL SetCurrentDirectoryA(const char *) {
  const int call = set_directory_calls++;
  return call < 2 ? set_directory_results[call] : 1;
}

static BOOL DlgDirListA(HWND, char *, int, int, int attributes) {
  ++dialog_list_calls;
  if ((attributes == 0x8010 && list_fail_first) ||
      (attributes == 0x20 && list_fail_second)) {
    return 0;
  }
  staged_entries.clear();
  if (attributes == 0x8010) {
    staged_entries.push_back("[..]");
    staged_entries.push_back("[Folder]");
  } else {
    staged_entries.push_back("READ.TXT");
    staged_entries.push_back("SKIP.BIN");
  }
  return 1;
}

static DWORD GetLastError() {
  ++last_error_calls;
  return 5;
}

static LRESULT SendMessageA(HWND, UINT message, WPARAM index, LPARAM data) {
  if (message == LB_GETCOUNT) {
    return static_cast<LRESULT>(staged_entries.size());
  }
  assert(message == LB_GETTEXT);
  assert(index < staged_entries.size());
  strcpy(reinterpret_cast<char *>(data), staged_entries[index].c_str());
  return static_cast<LRESULT>(staged_entries[index].size());
}

static HANDLE FindFirstFileA(const char *path, WIN32_FIND_DATAA *data) {
  const bool fail = fail_first_find && find_calls == 0;
  ++find_calls;
  if (fail) {
    // Deterministic output makes the original unguarded use of the record
    // observable without relying on uninitialized test memory.
    strcpy(data->cFileName, "failed-find-record");
    return 0;
  }
  const char *leaf = strrchr(path, '\\');
  leaf = leaf == 0 ? path : leaf + 1;
  strcpy(data->cFileName, leaf);
  return reinterpret_cast<HANDLE>(static_cast<uintptr_t>(find_calls));
}

static BOOL FindClose(HANDLE) {
  ++find_close_calls;
  return 1;
}

static int _strcmpi(const char *left, const char *right) {
  while (*left && *right) {
    const int l = tolower(static_cast<unsigned char>(*left++));
    const int r = tolower(static_cast<unsigned char>(*right++));
    if (l != r) return l - r;
  }
  return static_cast<unsigned char>(*left) - static_cast<unsigned char>(*right);
}

struct ItemList {
  int removals;
  ItemList() : removals(0) {}
  void remove_all() { ++removals; }
};

struct GraphicWin {};

struct Dialog {
  ItemList item_list_;
  int selected_position_;
  uint32_t field_24_;
  uint32_t field_28_;
  int attach_result;
  int attach_calls;
  int attached_x;
  int attached_y;
  int attached_flags;
  GraphicWin *attached_window;
  Dialog()
      : selected_position_(0), field_24_(17), field_28_(29), attach_result(0),
        attach_calls(0), attached_x(0), attached_y(0), attached_flags(0),
        attached_window(0) {}
  virtual int attach(GraphicWin *window, int x, int y, int flags) {
    ++attach_calls;
    attached_window = window;
    attached_x = x;
    attached_y = y;
    attached_flags = flags;
    return attach_result;
  }
  void attach_at_stored_position(GraphicWin *window, int flags) {
    attach(window, static_cast<int>(field_24_), static_cast<int>(field_28_), flags);
  }
};

struct ListBox : Dialog {
  bool dib;
  int field_18_;
  int field_20_;
  int refresh_result;
  int refresh_calls;
  int markup_enabled;
  int shown;
  std::vector<std::string> inserted_names;
  std::vector<int> inserted_kinds;
  ListBox()
      : dib(false), field_18_(7), field_20_(9), refresh_result(0),
        refresh_calls(0), markup_enabled(1), shown(1) {}
  bool has_dib_bits() const { return dib; }
  int update_changes() { ++refresh_calls; return refresh_result; }
  void set_markup_enabled(int value) { markup_enabled = value; }
  void show(int visible) { shown = visible; }
  void item(const char *name, int kind) {
    inserted_names.push_back(name);
    inserted_kinds.push_back(kind);
  }
};

static GraphicWin MockModalWindow;

struct FileWin {
  char previous_directory_[260];
  int extension_count_;
  char extension_filters_[5][5];
  GraphicWin *modal_window_;
  HWND directory_list_handle_;
  ListBox list_box_;
  int style_flags;
  FileWin()
      : extension_count_(1), modal_window_(&MockModalWindow),
        directory_list_handle_(reinterpret_cast<HWND>(0x55)), style_flags(0) {
    strcpy(extension_filters_[0], ".TXT");
  }
  int populate_directory(char *directory);
};

static const int HandleMainValue = 1;
static HWND HandleMain = reinterpret_cast<HWND>(HandleMainValue);
struct FileWinInitParameters { int field_00_; };
static FileWinInitParameters FileWinInitSharedParameters = {0};

static void reset_file(FileWin &file) {
  file.list_box_ = ListBox();
}

/* EXTRACTED_POPULATE_DIRECTORY_METHOD */

static void reset_world() {
  directory_list.clear();
  staged_entries.clear();
  dialog_list_calls = last_error_calls = set_directory_calls = 0;
  find_calls = find_close_calls = 0;
  set_directory_results[0] = set_directory_results[1] = 1;
  fail_first_find = false;
  list_fail_first = list_fail_second = false;
  FileWinInitSharedParameters.field_00_ = 0;
}

int main() {
  FileWin file;
  reset_world();
  reset_file(file);
  assert(file.populate_directory(0) == 0);
  assert(set_directory_calls == 0 && dialog_list_calls == 0);

  reset_world();
  reset_file(file);
  set_directory_results[0] = 0;
  set_directory_results[1] = 0;
  assert(file.populate_directory(const_cast<char *>("C:\\missing")) == 0);
  assert(set_directory_calls == 2 && dialog_list_calls == 0);

  reset_world();
  reset_file(file);
  list_fail_first = true;
  assert(file.populate_directory(const_cast<char *>("C:\\ok")) == 1);
  assert(last_error_calls == 1 && file.list_box_.attach_calls == 0);

  reset_world();
  reset_file(file);
  list_fail_second = true;
  assert(file.populate_directory(const_cast<char *>("C:\\ok")) == 1);
  assert(file.list_box_.inserted_names.size() == 2);
  assert(file.list_box_.attach_calls == 0 && file.list_box_.shown == 1);

  reset_world();
  reset_file(file);
  file.list_box_.dib = true;
  file.list_box_.refresh_result = 0;
  assert(file.populate_directory(const_cast<char *>("C:\\ok")) == 2);
  assert(file.list_box_.refresh_calls == 1 && file.list_box_.attach_calls == 0);
  assert(file.list_box_.inserted_names.size() == 3);
  assert(file.list_box_.inserted_names[0] == "..");
  assert(file.list_box_.inserted_names[1] == "Folder");
  assert(file.list_box_.inserted_names[2] == "READ.TXT");
  assert(file.list_box_.markup_enabled == 0 && file.list_box_.shown == 0);
  assert(file.list_box_.field_18_ == 0 && file.list_box_.field_20_ == 0);

  reset_world();
  reset_file(file);
  FileWinInitSharedParameters.field_00_ = 0x12345678;
  assert(file.populate_directory(const_cast<char *>("C:\\ok")) == 2);
  assert(file.list_box_.attach_calls == 1 && file.list_box_.refresh_calls == 0);
  assert(file.list_box_.attached_window == file.modal_window_);
  assert(file.list_box_.attached_x == 17 && file.list_box_.attached_y == 29);
  assert(file.list_box_.attached_flags == 0x12345678);
  assert(file.list_box_.markup_enabled == 0 && file.list_box_.shown == 0);

  reset_world();
  reset_file(file);
  assert(file.populate_directory(const_cast<char *>("C:\\ok")) == 2);
  assert(file.list_box_.attached_flags == 0x80060);

  reset_world();
  reset_file(file);
  file.list_box_.dib = true;
  file.list_box_.refresh_result = 8;
  assert(file.populate_directory(const_cast<char *>("C:\\ok")) == 1);
  assert(file.list_box_.markup_enabled == 1 && file.list_box_.shown == 1);

  reset_world();
  reset_file(file);
  file.list_box_.attach_result = 8;
  assert(file.populate_directory(const_cast<char *>("C:\\ok")) == 1);
  assert(file.list_box_.markup_enabled == 1 && file.list_box_.shown == 1);

  reset_world();
  reset_file(file);
  file.list_box_.dib = true;
  fail_first_find = true;
  assert(file.populate_directory(const_cast<char *>("C:\\ok")) == 2);
  assert(find_close_calls >= 1);
  bool consumed_failed_record = false;
  for (size_t i = 0; i < file.list_box_.inserted_names.size(); ++i) {
    if (file.list_box_.inserted_names[i] == "failed-find-record") {
      consumed_failed_record = true;
    }
  }
  assert(consumed_failed_record);
}
