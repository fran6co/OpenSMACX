import pathlib
import shutil
import subprocess
import pytest


ROOT = pathlib.Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src" / "game.cpp"


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
    raise AssertionError("unbalanced method body")


def test_load_map_candidate_actual_body(tmp_path):
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    if not compilers:
        pytest.skip("neither g++ nor clang++ is installed")
    body = extract_body(SOURCE.read_text(), "int __cdecl load_map()")
    harness = r'''
#include <assert.h>
#include <string.h>
#define __cdecl
typedef char *LPSTR;
typedef const char *LPCSTR;
static const char MapFileExtension[] = "MP";
static const char SaveGameFileExtension[] = "SAV";
static int IsMultiplayerNet = 0;
static char StringTemp[1024];
static int daemon_result = 0, draw_count = 0, popup_count = 0;
static int parsed_value = -1, parse_count = 0;
static char selected_value[260], pref_value[260], initial_default[260];
static char preference_default[260];
static char recorded_key[64], recorded_popup[1024], daemon_path[260];
static int extensions_count = 0, init_count = 0, destructor_count = 0;
static int destructor_order[2], destructor_order_count = 0;
static char extensions[2][16];
struct NetWinMock { char loaded_file_path_[0x104]; int field_7728_; };
static NetWinMock NetWinDialog;
class FileBox { public: FileBox() {} ~FileBox() { ++destructor_count; destructor_order[destructor_order_count++] = 1; } };
class AlphaSave {
 public:
  AlphaSave() {}
  ~AlphaSave() { ++destructor_count; destructor_order[destructor_order_count++] = 2; }
  void init() { ++init_count; }
  int add_default_extension(char *s) { strcpy(extensions[extensions_count++], s); return 0; }
  int set_default_filename(char *s) { strcpy(initial_default, s); return 0; }
  char *load() { return selected_value[0] ? selected_value : 0; }
};
static char *prefs_get(const char *key, const char *fallback, int) {
  strcpy(recorded_key, key);
  strcpy(preference_default, fallback);
  return pref_value;
}
static void prefs_put(const char *key, const char *value) {
  strcpy(recorded_key, key); strcpy(pref_value, value);
}
static int load_map_daemon(char *path) { strcpy(daemon_path, path); return daemon_result; }
static void do_all_draws() { ++draw_count; }
static void parse_says(int, char *, int, int) { ++parse_count; }
static void parse_num(int, int n) { parsed_value = n; }
static int X_pop(const char *s, int) { ++popup_count; strcpy(recorded_popup, s); return 0; }
''' + body + r'''
static void reset_case() {
  daemon_result = draw_count = popup_count = parse_count = extensions_count = init_count = destructor_count = 0;
  destructor_order_count = 0;
  parsed_value = -1; IsMultiplayerNet = 0;
  memset(&NetWinDialog, 0, sizeof(NetWinDialog));
  selected_value[0] = pref_value[0] = initial_default[0] = recorded_key[0] = 0;
  preference_default[0] = 0;
  recorded_popup[0] = daemon_path[0] = 0;
}
int main() {
  reset_case(); strcpy(pref_value, "old-pref"); selected_value[0] = 0;
  assert(load_map() == 1);
  assert(init_count == 1 && draw_count == 1 && destructor_count == 2);
  assert(destructor_order_count == 2 && destructor_order[0] == 2 && destructor_order[1] == 1);
  assert(extensions_count == 2 && strcmp(extensions[0], ".MP") == 0);
  assert(strcmp(extensions[1], ".SAV") == 0);
  assert(strcmp(preference_default, "maps\\*.MP") == 0);
  assert(strcmp(initial_default, "old-pref") == 0);
  assert(popup_count == 0 && daemon_path[0] == 0);

  reset_case(); strcpy(pref_value, "last-choice"); strcpy(selected_value, "maps\\north.MP");
  assert(load_map() == 0);
  assert(strcmp(daemon_path, "maps\\north.MP") == 0);
  assert(strcmp(pref_value, "maps\\north") == 0);
  assert(strcmp(recorded_key, "Latest Map") == 0);
  assert(draw_count == 1 && popup_count == 0 && NetWinDialog.field_7728_ == 0);

  reset_case(); IsMultiplayerNet = 1; strcpy(pref_value, "last");
  strcpy(selected_value, "maps\\two.part.SC");
  assert(load_map() == 0);
  assert(strcmp(NetWinDialog.loaded_file_path_, "maps\\two") == 0);
  assert(NetWinDialog.field_7728_ == 1 && popup_count == 0);
  assert(strcmp(pref_value, "maps\\two") == 0);

  reset_case(); daemon_result = 2; strcpy(pref_value, "last");
  strcpy(selected_value, "bad.SC");
  assert(load_map() == 2);
  assert(popup_count == 1 && strcmp(recorded_popup, "LOADNOT") == 0);
  assert(parse_count == 1 && parsed_value == 2 && draw_count == 2);

  reset_case(); IsMultiplayerNet = 1; daemon_result = 7;
  strcpy(pref_value, "last"); strcpy(selected_value, "bad.SC");
  assert(load_map() == 7);
  assert(popup_count == 1 && strcmp(recorded_popup, "LOADERROR") == 0);
  assert(parse_count == 1 && parsed_value == 7 && draw_count == 2);

  reset_case(); daemon_result = 1; strcpy(pref_value, "last"); strcpy(selected_value, "none.SC");
  assert(load_map() == 1);
  assert(popup_count == 1 && strcmp(recorded_popup, "LOADNONE") == 0);
  reset_case(); daemon_result = 3; strcpy(pref_value, "last"); strcpy(selected_value, "old.SC");
  assert(load_map() == 3);
  assert(popup_count == 1 && strcmp(recorded_popup, "LOADOLD") == 0);
  reset_case(); daemon_result = 4; strcpy(pref_value, "last"); strcpy(selected_value, "bad.SC");
  assert(load_map() == 4);
  assert(popup_count == 1 && strcmp(recorded_popup, "LOADERROR") == 0);
}
'''
    source = tmp_path / "load_map_control.cpp"
    source.write_text(harness)
    for compiler in compilers:
        binary = tmp_path / ("load-map-" + pathlib.Path(compiler).name)
        subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                        str(source), "-o", str(binary)], check=True, timeout=30)
        subprocess.run([str(binary)], check=True, timeout=30)
