"""Actual-source compass width controls; not native ABI/runtime proof."""
from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def _masked(source: str) -> str:
    out = list(source)
    state = "code"
    quote = ""
    i = 0
    while i < len(source):
        ch = source[i]
        nxt = source[i + 1] if i + 1 < len(source) else ""
        if state == "code":
            if ch == "/" and nxt == "/":
                out[i] = out[i + 1] = " "
                i += 2
                state = "line"
                continue
            if ch == "/" and nxt == "*":
                out[i] = out[i + 1] = " "
                i += 2
                state = "block"
                continue
            if ch in ('"', "'"):
                quote = ch
                out[i] = " "
                state = "quoted"
        elif state == "line":
            if ch == "\n":
                state = "code"
            else:
                out[i] = " "
        elif state == "block":
            if ch == "*" and nxt == "/":
                out[i] = out[i + 1] = " "
                i += 2
                state = "code"
                continue
            if ch != "\n":
                out[i] = " "
        else:
            if ch == "\\":
                out[i] = " "
                if i + 1 < len(source) and source[i + 1] != "\n":
                    out[i + 1] = " "
                    i += 2
                    continue
            elif ch == quote:
                out[i] = " "
                state = "code"
            elif ch != "\n":
                out[i] = " "
        i += 1
    return "".join(out)


def _function(source: str, signature: str) -> str:
    masked = _masked(source)
    starts = [m.start() for m in re.finditer(re.escape(signature), masked)]
    assert len(starts) == 1, f"expected one code occurrence of {signature}, got {len(starts)}"
    start = starts[0]
    opening = masked.index("{", start)
    depth = 0
    for pos in range(opening, len(masked)):
        if masked[pos] == "{":
            depth += 1
        elif masked[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[start:pos + 1]
    raise AssertionError("unterminated compass_move body")


def _array_definition(source: str, name: str) -> str:
    match = re.search(rf"(?m)^int {name}\[9\]\s*=\s*\{{[^}}]*\}};", source)
    assert match, f"missing actual source definition of {name}"
    return match.group(0)


def _actual_and_candidate() -> tuple[str, str, str, str]:
    map_source = (ROOT / "src/map.cpp").read_text()
    veh_source = (ROOT / "src/veh.cpp").read_text()
    candidate = _function(map_source, "int __cdecl compass_move(")
    assert len(re.findall(r"\bMapLongitudeBounds\b", candidate)) == 4
    assert not re.search(r"\bMapLongitude\b", candidate)
    original = candidate.replace("MapLongitudeBounds", "MapLongitude")
    return original, candidate, _array_definition(veh_source, "VehMoveDeltaX"), _array_definition(veh_source, "VehMoveDeltaY")


HARNESS = r"""
#include <assert.h>
#include <stdint.h>
#define __cdecl
int MapLongitudeBounds = 16;
uint32_t MapLongitude = 8; // decoy: true half-width row-stride value
ACTUAL_VEH_X
ACTUAL_VEH_Y
ACTUAL_FUNCTIONS

int main() {
  // Full width 16 and decoy stride 8: ±6 must retain sign under a full-width
  // half-boundary of 8. The old source flips both signs at the decoy boundary 4.
  assert(compass_move(0, 0, 6, 0) == 1);
  assert(compass_move(0, 0, -6, 0) == 5);
  assert(compass_move(0, 0, 6, 1) == 2);
  assert(compass_move(0, 0, -6, 1) == 4);

  // Strict comparisons: exactly ±8 is not wrapped; sign-normalization still
  // resolves to the corresponding horizontal movement-table entry.
  assert(compass_move(0, 0, 8, 0) == 1);
  assert(compass_move(0, 0, -8, 0) == 5);

  // One conditional wrap, not modulo/loop normalization. At valid coordinate
  // endpoints on a width-16 map, ±14 becomes the opposite two-step vector.
  assert(compass_move(0, 0, 14, 0) == 5);
  assert(compass_move(0, 0, -14, 0) == 1);
  assert(compass_move(0, 0, 16, 0) == 8); // explicit full-width displacement -> center
}
"""


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_corrected_complete_body_matches_full_width_wrap(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    _, candidate, veh_x, veh_y = _actual_and_candidate()
    source = (HARNESS.replace("ACTUAL_VEH_X", veh_x)
              .replace("ACTUAL_VEH_Y", veh_y)
              .replace("ACTUAL_FUNCTIONS", candidate))
    cpp = tmp_path / "compass.cpp"
    exe = tmp_path / "compass"
    cpp.write_text(source)
    subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(cpp), "-o", str(exe)], check=True, capture_output=True, text=True)
    subprocess.run([str(exe)], check=True)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_old_half_stride_body_is_a_live_damage_control(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    original, candidate, veh_x, veh_y = _actual_and_candidate()
    assert candidate != original
    # Both bodies in one TU demonstrate that these cases reject the old
    # half-stride behavior rather than merely checking textual substitutions.
    original = original.replace("compass_move(", "old_compass_move(", 1)
    harness = HARNESS.replace("int main() {", """int old_compass_move(int,int,int,int);
int main() {
  assert(old_compass_move(0,0,6,0) == 5);
  assert(old_compass_move(0,0,-6,0) == 1);
""")
    source = (harness.replace("ACTUAL_VEH_X", veh_x)
              .replace("ACTUAL_VEH_Y", veh_y)
              .replace("ACTUAL_FUNCTIONS", original + "\n" + candidate))
    cpp = tmp_path / "compass-damage-control.cpp"
    exe = tmp_path / "compass-damage-control"
    cpp.write_text(source)
    subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(cpp), "-o", str(exe)], check=True, capture_output=True, text=True)
    subprocess.run([str(exe)], check=True)


def test_proposal_preserves_signed_half_and_single_wrap_shape():
    _, candidate, _, _ = _actual_and_candidate()
    assert candidate.count("((int)MapLongitudeBounds / 2)") == 2
    assert candidate.count("x_radius_off += MapLongitudeBounds;") == 1
    assert candidate.count("x_radius_off -= MapLongitudeBounds;") == 1
    assert "while (" not in candidate and "%" not in candidate
