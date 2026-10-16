"""Actual-source patrol endpoint indexing controls, not native memory proof."""
from pathlib import Path
import re
import shutil
import subprocess

import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/veh.cpp").is_file())
SIGNATURE = "void __cdecl action_patrol(int veh_id, int x, int y)"


def _actual_body() -> str:
    source = (ROOT / "src/veh.cpp").read_text(encoding="utf-8")
    masked = COMMENTS_OR_STRINGS.sub(lambda match: " " * len(match[0]), source)
    hits = [m.start() for m in re.finditer(re.escape(SIGNATURE), masked)]
    assert len(hits) == 1, f"expected one production definition, found {len(hits)}"
    start = hits[0]
    opening = masked.index("{", start)
    depth = 0
    for pos in range(opening, len(masked)):
        if masked[pos] == "{":
            depth += 1
        elif masked[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[start:pos + 1]
    raise AssertionError("unterminated action_patrol body")


HARNESS = r"""
#include <assert.h>
#include <stdint.h>
#define __cdecl
typedef int16_t int16;
enum { ORDER_MOVE_TO = 7 };
static bool patrol_valid = true;
static int writes_x[4], writes_y[4], values_x[4], values_y[4];
static int events[8], nevents, nx, ny;
struct WordProxy {
  int index;
  bool is_x;
  WordProxy& operator=(int value) {
    if (is_x) {
      events[nevents++] = 1;
      writes_x[nx] = index; values_x[nx++] = (int16_t)value;
    } else {
      events[nevents++] = 2;
      writes_y[ny] = index; values_y[ny++] = (int16_t)value;
    }
    return *this;
  }
};
struct WordArray {
  bool is_x;
  WordProxy operator[](int index) { WordProxy p = {index, is_x}; return p; }
};
struct MockVeh {
  int order;
  uint8_t waypoint_count;
  int patrol_current_point;
  WordArray waypoint_x, waypoint_y;
  int16_t x, y;
  uint32_t state;
  MockVeh() : order(0), waypoint_count(0), patrol_current_point(9),
    waypoint_x{true}, waypoint_y{false}, x(31), y(-12), state(0) {}
};
static MockVeh Vehs[1];
static int valid_patrol(int, int, int) { return patrol_valid; }
ACTUAL_BODY
int main(int argc, char** argv) {
  (void)argc; (void)argv;
  Vehs[0].waypoint_count = 255;
  action_patrol(0, 100, 200);
  assert(Vehs[0].waypoint_count == 0);
  assert(nx == 2 && ny == 2);
  assert(nevents == 4 && events[0] == 1 && events[1] == 2 &&
         events[2] == 1 && events[3] == 2);
  assert(writes_x[0] == 255 && writes_x[1] == 0);
  assert(writes_y[0] == 255 && writes_y[1] == 0);
  assert(values_x[0] == 100 && values_y[0] == 200);
  assert(values_x[1] == 31 && values_y[1] == -12);
  assert(Vehs[0].order == ORDER_MOVE_TO && Vehs[0].patrol_current_point == 0);
  assert(Vehs[0].state == 0x1000200);

  nx = ny = nevents = 0;
  Vehs[0].waypoint_count = 2;
  Vehs[0].state = 0x40;
  action_patrol(0, -7, 8);
  assert(Vehs[0].waypoint_count == 3);
  assert(nx == 2 && ny == 2);
  assert(nevents == 4 && events[0] == 1 && events[1] == 2 &&
         events[2] == 1 && events[3] == 2);
  assert(writes_x[0] == 2 && writes_x[1] == 3);
  assert(writes_y[0] == 2 && writes_y[1] == 3);
  assert(values_x[0] == -7 && values_y[0] == 8);
  assert(values_x[1] == 31 && values_y[1] == -12);
  assert(Vehs[0].state == (0x40 | 0x1000200));

  nx = ny = nevents = 0;
  patrol_valid = false;
  Vehs[0].waypoint_count = 10;
  Vehs[0].order = 11;
  Vehs[0].state = 0x40;
  Vehs[0].patrol_current_point = 6;
  action_patrol(0, 1, 2);
  assert(nx == 0 && ny == 0 && nevents == 0);
  assert(Vehs[0].waypoint_count == 10 && Vehs[0].order == 11);
  assert(Vehs[0].state == 0x40 && Vehs[0].patrol_current_point == 6);
  return 0;
}
"""


def _run(tmp_path: Path, body: str, compiler_name: str) -> subprocess.CompletedProcess[str]:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    source = tmp_path / f"patrol_{compiler_name}.cpp"
    binary = tmp_path / f"patrol_{compiler_name}"
    source.write_text(HARNESS.replace("ACTUAL_BODY", body), encoding="utf-8")
    subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(source), "-o", str(binary)], check=True,
                   capture_output=True, text=True, timeout=60)
    return subprocess.run([str(binary)], check=False, capture_output=True,
                          text=True, timeout=30)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_patrol_source_uses_byte_wrapped_endpoint_index(compiler_name, tmp_path):
    body = _actual_body()
    assert "waypoint_x[Vehs[veh_id].waypoint_count] = Vehs[veh_id].x;" in body
    assert "waypoint_x[count + 1]" not in body
    result = _run(tmp_path, body, compiler_name)
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_promoted_count_index_is_a_live_damage_control(compiler_name, tmp_path):
    body = _actual_body()
    mutant = body.replace(
        "waypoint_x[Vehs[veh_id].waypoint_count] = Vehs[veh_id].x;",
        "waypoint_x[count + 1] = Vehs[veh_id].x;",
        1,
    )
    assert mutant != body
    result = _run(tmp_path, mutant, compiler_name)
    assert result.returncode != 0, "promoted index mutant unexpectedly passed wrap control"


def test_boundary_proxy_is_not_claimed_as_native_out_of_bounds_proof():
    """Proxy observes conceptual indices; it is not native out-of-bounds proof."""
    assert "waypoint_x[count + 1]" not in _actual_body()
