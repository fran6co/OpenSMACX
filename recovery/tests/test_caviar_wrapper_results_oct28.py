from pathlib import Path
import shutil
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/caviar.cpp"


def body(source, signature):
    begin = source.index(signature)
    opening = source.index("{", begin)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[begin:index + 1]
    raise AssertionError("unbalanced source function: " + signature)


def test_actual_multi_table_wrapper_returns_errors_and_orders_success_effects():
    function = body(SOURCE.read_text(),
                    "int Caviar::render_multi_table_dest(Buffer *buffer")
    harness = r'''#include <assert.h>
struct ColorTable {};
static int events[16], event_count;
static int seen_flags, seen_x, seen_y, seen_fill, seen_clear;
struct Buffer {
  int free_data(int value) { events[event_count++] = 4; assert(value == 1); return 0; }
};
struct Surface {
  void fill(int value) { events[event_count++] = value == 9 ? 5 : 1; seen_fill = value; }
  void draw_multi_table_dest(Buffer *, int fill, int x, int y, int sx, int sy, ColorTable *) {
    events[event_count++] = 3;
    assert(fill == 7 && x == 13 && y == 24 && sx == 1 && sy == 1);
  }
};
struct SurfaceOwner { Surface value; Surface &buffer() { return value; } };
SurfaceOwner CaviarSceneBufferA;
struct Caviar {
  int result;
  int render(Buffer *, int x, int y, int flags) {
    events[event_count++] = 2;
    seen_x = x; seen_y = y; seen_flags = flags;
    return result;
  }
  void clear_buffers(int value) { events[event_count++] = 6; seen_clear = value; }
  int render_multi_table_dest(Buffer *, int, int, int, ColorTable *, int);
};
/* ACTUAL_FUNCTION */
static void reset() { event_count = 0; seen_fill = -1; seen_clear = -1; }
int main() {
  Caviar owner = {};
  Buffer buffer;
  ColorTable table;
  owner.result = -17;
  reset();
  assert(owner.render_multi_table_dest(&buffer, 141, 152, 7, &table, 5) == -17);
  assert(event_count == 2 && events[0] == 1 && events[1] == 2);
  assert(seen_flags == (5 | 8) && seen_x == 141 && seen_y == 152);
  assert(seen_fill == 7 && seen_clear == -1);
  owner.result = 0;
  reset();
  assert(owner.render_multi_table_dest(&buffer, 141, 152, 7, &table, 5) == 0);
  assert(event_count == 6);
  assert(events[0] == 1 && events[1] == 2 && events[2] == 3);
  assert(events[3] == 4 && events[4] == 5 && events[5] == 6);
  assert(seen_flags == (5 | 8) && seen_fill == 9 && seen_clear == (5 & 1));
  return 0;
}
'''.replace("/* ACTUAL_FUNCTION */", function)
    compiler = shutil.which("g++") or shutil.which("clang++")
    assert compiler, "GCC or Clang is required for actual-source controls"
    with tempfile.TemporaryDirectory() as directory:
        cpp = Path(directory) / "wrapper.cpp"
        exe = Path(directory) / "wrapper"
        cpp.write_text(harness)
        built = subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", str(cpp), "-o", str(exe)],
            capture_output=True, text=True, timeout=60)
        assert built.returncode == 0, built.stdout + built.stderr
        ran = subprocess.run([str(exe)], capture_output=True, text=True, timeout=30)
        assert ran.returncode == 0, ran.stdout + ran.stderr
