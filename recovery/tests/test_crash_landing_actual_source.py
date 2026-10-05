"""Host controls that compile and execute the actual product source body."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[2]



STUBS = r'''
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstring>
#define __cdecl
typedef unsigned int DWORD;
typedef char *LPSTR;
struct RECT { long left, top, right, bottom; };
static const int VK_SHIFT = 0x10;
static int g_tick = 0;
static int g_decodes = 0;
static int g_flic_created = 0;
static int g_decode_order[128];
static int g_base_unique = 0;
static int g_popup_start = 0;
static int g_draw = 1;
static int g_smoke_draw = 0;
static int g_smoke_transparency = -1;
static unsigned char *g_smoke_table_pointer = 0;
static int g_explicit_popup_close = 0;
static int g_explicit_flic_close = 0;
static int g_flic_destructors = 0;
static int g_popup_destructors = 0;
static int g_restore_calls = 0;
static long g_restore_left = 0;
static long g_restore_top = 0;
static long g_first_restore_left = 0;
static long g_first_restore_top = 0;
static int g_capture_width = 0;
static int g_capture_height = 0;
static int g_first_pod_x = -1;
static int g_first_pod_y = -1;
static int g_last_pod_x = -1;
static int g_last_pod_y = -1;
static int g_map_draw = 0;
static int g_draw_all = 0;
static int g_flush = 0;
struct Buffer {
  int init(int w, int h, int, int) { g_capture_width = w; g_capture_height = h; return 1; }
  int pixel_width() const { return 1024; }
  int pixel_height() const { return 768; }
  int copy(Buffer *, int, int, int, int) { return 1; }
  int copy(Buffer *, RECT *rect) {
    if (g_restore_calls == 0) { g_first_restore_left = rect->left; g_first_restore_top = rect->top; }
    ++g_restore_calls; g_restore_left = rect->left; g_restore_top = rect->top; return 1;
  }
  int draw(Buffer *, int, int x, int y, int, int) {
    if (g_first_pod_x < 0) { g_first_pod_x = x; g_first_pod_y = y; }
    g_last_pod_x = x; g_last_pod_y = y;
    return g_draw;
  }
  int draw_multi_table_dest(Buffer *, int transparent, int, int, int, int, unsigned char *table) {
    ++g_smoke_draw; g_smoke_transparency = transparent;
    g_smoke_table_pointer = table; return 1;
  }
  void close() {}
};
struct GraphicWin : Buffer {
  void update(RECT *, GraphicWin *) {}
};
struct MapWin : GraphicWin {
  unsigned int field_1DD70_;
  unsigned int field_1DDB4_;
  unsigned int field_1DDB8_;
  MapWin() : field_1DD70_(~0u), field_1DDB4_(640), field_1DDB8_(480) {}
  void set_center(int, int, int) {}
  void draw_map(int) { ++g_map_draw; }
  void tile_to_pixel(int x, int y, int *px, int *py) { *px=x*4; *py=y*4; }
};
struct Flic {
  int id;
  Flic() : id(++g_flic_created) {}
  ~Flic() { ++g_flic_destructors; }
  int init(Buffer *, char *, int, int, int) { return 1; }
  int decode_frame(int, int) { g_decode_order[g_decodes++] = id; return 1; }
  void close() { ++g_explicit_flic_close; }
};
struct Popup {
  Popup() {}
  ~Popup() { ++g_popup_destructors; close(); }
  void close() { ++g_explicit_popup_close; }
  int start(char *, const char *, int, char *, int, GraphicWin *) {
    ++g_popup_start; return 1;
  }
  int exec(int, int (*)()) { return 0; }
};
struct Wave {
  int init(char *, int) { return 1; }
  int play() { return 1; }
  int fade(int) { return 1; }
};
struct Base { short x, y; unsigned char faction_id_current; char name_string[25]; };
struct Veh { short x, y; unsigned char faction_id; };
struct Player { char filename[24]; int rule_flags; };
struct Console { int faction_id_; int update_data(int) { return 1; } };
struct Char256 { char str[256]; };
static Base Bases[32];
static Veh Vehs[32];
static Player Players[8];
static Char256 ParseStrBuffer[10];
static MapWin *MapWinTable[8];
static unsigned char ExplosionColorTables[13][16][256];
static Console ConsoleGlobal;
static Wave g_CRASH_LANDING_WAVE;
static char StringTemp[1032];
static char PopupStartCaption[256];
static int BaseCurrentCount, VehCurrentCount, TurnCurrentNum;
static int IsMultiplayerNet, GamePreferences;
static int SpriteScaleNumerator, SpriteScaleDenominator;
static int ConsoleNoMapOverrideLatch;
static void do_all_draws() { ++g_draw_all; }
static void flush_input() { ++g_flush; }
static void make_base_unique(int) { ++g_base_unique; }
static DWORD timeGetTime() { g_tick += 100; return static_cast<DWORD>(g_tick); }
static short GetAsyncKeyState(int) { return 0; }
static char *CharUpperA(char *s) {
  for (char *p=s; *p; ++p) *p=static_cast<char>(std::toupper(static_cast<unsigned char>(*p)));
  return s;
}
static int UnionRect(RECT *out, const RECT *a, const RECT *b) {
  out->left = a->left < b->left ? a->left : b->left;
  out->top = a->top < b->top ? a->top : b->top;
  out->right = a->right > b->right ? a->right : b->right;
  out->bottom = a->bottom > b->bottom ? a->bottom : b->bottom;
  return 1;
}
'''


def make_program(source: str) -> str:
    return STUBS + "\n" + source + r'''
int main(int argc, char **argv) {
  MapWin window;
  MapWinTable[0] = &window;
  ConsoleGlobal.faction_id_ = 1;
  const bool fungboy = argc > 1 && std::strcmp(argv[1], "fungboy") == 0;
  Players[1].rule_flags = fungboy || (argc > 1 && std::strcmp(argv[1], "fungus") == 0) ? 0x80 : 0;
  std::strcpy(Players[1].filename, fungboy ? "FUNGBOY" : "HUMAN");
  std::strcpy(ParseStrBuffer[0].str, "NEW NAME");
  BaseCurrentCount = 0;
  VehCurrentCount = 1;
  Vehs[0].faction_id = 7;
  Vehs[0].x = 12;
  Vehs[0].y = 9;
  crash_landing(7);
  if (fungboy) {
    if (TurnCurrentNum != 5 || g_decodes != 0 || g_draw_all != 0) return 31;
    if (g_map_draw != 1 || g_flush != 2 || g_smoke_draw != 0) return 32;
    if (g_popup_destructors != 1 || g_flic_destructors != 2) return 33;
    return 0;
  }
  if (argc > 1) {
    const int fungus = std::strcmp(argv[1], "fungus") == 0;
    const int expected_first_x = fungus ? 13 : 650;
    const int expected_last_x = 200;
    if (g_decodes != 60) return 21;
    if (g_first_pod_x != expected_first_x) return 22;
    if (g_first_pod_y != 351) return 23;
    if (g_last_pod_x != expected_last_x) return 24;
    if (g_last_pod_y != 83) return 25;
    if (g_first_restore_left != (fungus ? 0 : 673) || g_first_restore_top != 376) return 26;
    if (g_smoke_draw != (fungus ? 0 : 30)) return 27;
    return 0;
  }
  if (g_decodes != 60 || g_map_draw != 2 || g_draw_all != 1 || g_flush != 2) return 2;
  if (g_decode_order[0] != 1 || g_decode_order[1] != 2 || g_decode_order[58] != 1 || g_decode_order[59] != 2) return 3;
  if (g_popup_start != 0 || g_base_unique != 0 || g_smoke_draw != 30) return 4;
  if (g_restore_calls != 30 || g_first_restore_left != 673 || g_first_restore_top != 376) return 9;
  if (g_capture_width != 1024 || g_capture_height != 768) return 10;
  if (g_first_pod_x != 650) return 11;
  if (g_first_pod_y != 351) return 14;
  if (g_last_pod_x != 200 || g_last_pod_y != 83) return 17;
  if (g_explicit_popup_close != 1 || g_explicit_flic_close != 0) return 12;
  if (g_popup_destructors != 1 || g_flic_destructors != 2) return 13;

  g_draw = 0;
  BaseCurrentCount = 1;
  Bases[0].faction_id_current = 8;
  Bases[0].x = 4;
  Bases[0].y = 5;
  std::strcpy(Bases[0].name_string, "OLD");
  crash_landing(8);
  if (g_decodes != 120 || g_popup_start != 1 || g_base_unique != 1) return 5;
  if (g_smoke_draw != 60 || g_smoke_transparency != 0xff) return 6;
  if (g_smoke_table_pointer != &ExplosionColorTables[12][0][0]) return 7;
  if (std::strcmp(Bases[0].name_string, "NEW NAME") != 0) return 8;
  const int map_draws_before_no_match = g_map_draw;
  VehCurrentCount = 0;
  crash_landing(9);
  if (g_map_draw != map_draws_before_no_match || g_popup_destructors != 3) return 15;
  if (g_flic_destructors != 6 || g_explicit_flic_close != 0) return 16;
  return 0;
}
'''


class CrashLandingCandidateTests(unittest.TestCase):
    @staticmethod
    def extract_function(source: str) -> str:
        marker = "void __cdecl crash_landing(int faction_id) {"
        start = source.index(marker)
        opening = source.index("{", start)
        depth = 0
        for index in range(opening, len(source)):
            if source[index] == "{":
                depth += 1
            elif source[index] == "}":
                depth -= 1
                if depth == 0:
                    return source[start:index + 1]
        raise AssertionError("crash_landing body is not closed")

    def compile_and_run(self, candidate: str, args=None) -> subprocess.CompletedProcess:
        compiler = shutil.which("g++") or shutil.which("clang++")
        if compiler is None:
            self.skipTest("host C++ compiler unavailable")
        source = make_program(candidate)
        with tempfile.TemporaryDirectory(prefix="crash-landing-control-") as temp:
            cpp = Path(temp) / "control.cpp"
            exe = Path(temp) / "control"
            cpp.write_text(source)
            subprocess.run([compiler, "-std=c++11", "-Wall", "-Werror", str(cpp), "-o", str(exe)], check=True)
            return subprocess.run([str(exe)] + (args or []), check=False)

    def test_complete_candidate_host_control(self):
        candidate = self.extract_function((ROOT / "src/newgame.cpp").read_text())
        result = self.compile_and_run(candidate)
        self.assertEqual(result.returncode, 0)

    def test_actual_candidate_rejects_frame_count_mutant(self):
        candidate = self.extract_function((ROOT / "src/newgame.cpp").read_text()).replace("frame < 0x1e", "frame < 0x1d", 1)
        result = self.compile_and_run(candidate, ["mutant"])
        self.assertNotEqual(result.returncode, 0)

    def test_actual_candidate_preserves_fungus_frame_zero_drift(self):
        result = self.compile_and_run(self.extract_function((ROOT / "src/newgame.cpp").read_text()), ["fungus"])
        self.assertEqual(result.returncode, 0)

    def test_actual_source_preserves_fungboy_shortcut_and_teardown(self):
        candidate = self.extract_function((ROOT / "src/newgame.cpp").read_text())
        self.assertEqual(self.compile_and_run(candidate, ["fungboy"]).returncode, 0)

    def test_actual_source_rejects_missing_restore(self):
        candidate = self.extract_function((ROOT / "src/newgame.cpp").read_text())
        damaged = candidate.replace("captured_screen.copy(&screen, &previous_rect);", "", 1)
        self.assertNotEqual(candidate, damaged)
        self.assertNotEqual(self.compile_and_run(damaged).returncode, 0)

    def test_actual_source_rejects_smoke_result_gate(self):
        candidate = self.extract_function((ROOT / "src/newgame.cpp").read_text())
        damaged = candidate.replace("if (!fungus) {\n            smoke_frame.draw_multi_table_dest(",
                                    "if (!fungus && g_draw == 0) {\n            smoke_frame.draw_multi_table_dest(", 1)
        self.assertNotEqual(candidate, damaged)
        self.assertNotEqual(self.compile_and_run(damaged).returncode, 0)



if __name__ == "__main__":
    unittest.main()
