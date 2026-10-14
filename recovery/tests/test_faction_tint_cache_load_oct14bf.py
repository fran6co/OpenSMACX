from pathlib import Path
import subprocess
import tempfile


HERE = Path(__file__).parent
ROOT = next(parent for parent in HERE.parents if (parent / "src").is_dir())


def extract_function(signature: str) -> str:
    source = (ROOT / "src/game.cpp").read_text()
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError(f"unterminated function {signature}")


HARNESS = r"""
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define __cdecl
using DWORD = uint32_t;

struct Player { char filename[32]; };
Player Players[8] = {{"fac0"}, {"fac1"}, {"fac2"}, {"fac3"},
                    {"fac4"}, {"fac5"}, {"fac6"}, {"fac7"}};
char StringTemp[256];
const int WinFillColour = 255;
unsigned char TerritoryColorTables[8][256];
unsigned char BodyTintColorTables[8][24][256];

struct PALETTEENTRY { uint8_t peRed, peGreen, peBlue, peFlags; };
struct HSV { int h, s, v; };
struct Palette {
  Palette() = default;
};

static unsigned char values_cache[12];
static unsigned char table_cache[0xC100];
static int create_table_calls;
static int nearest_palette_calls;
static int rgb_to_hsv_calls;
static int create_calls;
static int time_calls;
static int values_close_calls;
static int final_close_calls;
static int cache_open_calls;

struct Buffer {
  int load_pcx(const char *, Palette *, int, int) { return 0; }
  int get_pixel(int x, int y) {
    if (x == 0xA1 && y == 0x2EB) return 23;
    if (x == 0xA1 && y == 0x2F5) return 100;
    if (x == 0x1B3 && y == 0x2E9) return 42;
    return 0;
  }
};

struct Filemap {
  void *open(char *path, int) {
    ++cache_open_calls;
    if (std::strstr(path, "vals.tmp") != nullptr) {
      return values_cache;
    }
    return table_cache;
  }
  void *create(char *, unsigned long, int) {
    ++create_calls;
    return table_cache;
  }
  uint32_t get_size() const { return 0xC100; }
  void close(unsigned char *end) {
    assert(end == values_cache + 12);
    ++values_close_calls;
  }
  void close() { ++final_close_calls; }
};

struct PaletteAccess {
  PALETTEENTRY entries_[256];
  PaletteAccess() {
    for (int i = 0; i < 256; ++i) {
      entries_[i] = {static_cast<uint8_t>(i), static_cast<uint8_t>(i),
                     static_cast<uint8_t>(i), 0};
    }
  }
  int create_table_from_color(int, uint8_t *, int, int, int, int) {
    ++create_table_calls;
    return 0;
  }
  PALETTEENTRY *entries() { return entries_; }
  int get_nearest_palette_index(uint8_t, uint8_t, uint8_t, int) {
    ++nearest_palette_calls;
    return 0;
  }
} g_PALETTE1;

struct CvrRecord { unsigned char palette_[0x302 + 0xC000]; } cvr_record;
struct CaviarData { CvrRecord *record() { return &cvr_record; } };
CaviarData g_VBP_CAVIARDATA;

int compare_palettes() { return 0; }
DWORD timeGetTime() { ++time_calls; return 100; }
void RGB_to_HSV(const PALETTEENTRY *, HSV *) { ++rgb_to_hsv_calls; }

static void *guarded_memcpy(void *destination, const void *source,
                            std::size_t count) {
  const uintptr_t begin = reinterpret_cast<uintptr_t>(&BodyTintColorTables[0][0][0]);
  const uintptr_t end = begin + sizeof(BodyTintColorTables);
  const uintptr_t address = reinterpret_cast<uintptr_t>(destination);
  if (address >= begin && address < end && count > 0x100) {
    std::fprintf(stderr,
                 "oversized body tint cache load: %zu bytes exceeds 256\n",
                 count);
    std::exit(97);
  }
  return std::memcpy(destination, source, count);
}

"""


def test_cache_load_copies_only_native_faction_table_prefix():
    """Exercise the extracted cache-hit path, not full runtime startup."""
    function = extract_function("void __cdecl colortables_init_faction(int faction_id)")
    assert "memcpy(TerritoryColorTables[faction_id], table_view, 0x100);" in function
    assert ("memcpy(static_cast<unsigned char *>(table_view) + 0x100,\n"
            "               BodyTintColorTables[faction_id], 0xC000);") in function

    tests = r"""
#define memcpy guarded_memcpy
""" + function + r"""
#undef memcpy

int main() {
  const uint32_t keys[3] = {35, 23, 42};
  std::memcpy(values_cache, keys, sizeof(keys));
  for (int i = 0; i < 0xC100; ++i) {
    table_cache[i] = static_cast<unsigned char>((i * 29 + 7) & 0xFF);
  }
  std::memset(BodyTintColorTables, 0xA5, sizeof(BodyTintColorTables));

  for (int faction = 0; faction < 8; ++faction) {
    std::memset(TerritoryColorTables[faction], 0x5A, 0x100);
    colortables_init_faction(faction);

    for (int i = 0; i < 0x100; ++i) {
      assert(TerritoryColorTables[faction][i] == table_cache[i]);
    }
    for (int checked_faction = 0; checked_faction < 8; ++checked_faction) {
      for (int row = 0; row < 24; ++row) {
        for (int color = 0; color < 256; ++color) {
          const unsigned char expected =
              checked_faction <= faction && row == 0
                  ? table_cache[0x100 + color] : 0xA5;
          assert(BodyTintColorTables[checked_faction][row][color] == expected);
        }
      }
    }
  }

  assert(cache_open_calls == 16);
  assert(values_close_calls == 8);
  assert(final_close_calls == 8);
  assert(time_calls == 8);
  assert(create_calls == 0);
  assert(create_table_calls == 0);
  assert(nearest_palette_calls == 0);
  assert(rgb_to_hsv_calls == 0);
  return 0;
}
"""
    code = HARNESS + tests

    with tempfile.TemporaryDirectory(prefix="opensmacx-faction-tint-cache-") as directory:
        exe = Path(directory) / "cache-load"
        build = subprocess.run(
            ["g++", "-std=c++17", "-O0", "-Wall", "-Wextra", "-Werror",
             "-pedantic", "-x", "c++", "-", "-o", str(exe)],
            input=code, text=True, capture_output=True, check=False)
        assert build.returncode == 0, build.stdout + build.stderr
        run = subprocess.run([str(exe)], capture_output=True, text=True, check=False)
        assert run.returncode == 0, run.stdout + run.stderr

        old_copy = ("memcpy(BodyTintColorTables[faction_id],\n"
                    "                   static_cast<unsigned char *>(table_view) + 0x100,\n"
                    "                   0x100);")
        new_copy = old_copy.replace("0x100);", "0xC000);")
        assert old_copy in function
        mutant_function = function.replace(old_copy, new_copy, 1)
        mutant_code = code.replace(function, mutant_function, 1)
        mutant = Path(directory) / "oversized-load-mutant"
        mutant_build = subprocess.run(
            ["g++", "-std=c++17", "-O0", "-Wall", "-Wextra", "-Werror",
             "-pedantic", "-x", "c++", "-", "-o", str(mutant)],
            input=mutant_code, text=True, capture_output=True, check=False)
        assert mutant_build.returncode == 0, mutant_build.stdout + mutant_build.stderr
        mutant_run = subprocess.run([str(mutant)], capture_output=True, text=True,
                                    check=False)
        assert mutant_run.returncode == 97
        assert "oversized body tint cache load: 49152 bytes exceeds 256" in mutant_run.stderr
