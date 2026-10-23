"""Source-backed signed-coordinate behavior for ProdPicker's real geometry."""
from __future__ import annotations

import re
import shutil
import subprocess
from pathlib import Path

import pytest

from match_recovery.includes import COMMENTS_OR_STRINGS


ROOT = Path(__file__).resolve().parents[2]
HEADER = ROOT / "src/prodpicker.h"
SOURCE = ROOT / "src/prodpicker.cpp"
SIGNATURE = "void ProdPicker::on_left_click(int x, int y)"


def member_declaration(name: str, offset: str | None = None) -> str:
    declaration = re.compile(rf"\s*(?:int32_t|uint32_t)\s+{re.escape(name)};")
    matches = []
    for line in HEADER.read_text(encoding="utf-8").splitlines():
        match = declaration.match(line)
        if match and (offset is None or re.search(rf"//.*\b{re.escape(offset)}\b", line)):
            matches.append(line)
    assert len(matches) == 1, (name, offset, matches)
    return matches[0].strip()


def actual_click_definition() -> str:
    source = SOURCE.read_text(encoding="utf-8")
    code = COMMENTS_OR_STRINGS.sub(lambda match: re.sub(r"[^\n]", " ", match[0]), source)
    assert code.count(SIGNATURE) == 1
    start = code.index(SIGNATURE)
    opening = code.index("{", start)
    depth = 0
    for end in range(opening, len(code)):
        if code[end] == "{":
            depth += 1
        elif code[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start : end + 1]
    raise AssertionError("unterminated ProdPicker::on_left_click definition")


def compile_and_run(tmp_path: Path, compiler_name: str) -> subprocess.CompletedProcess[str]:
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")

    declarations = "\n  ".join((
        member_declaration("field_A774_", "0xA774"),
        member_declaration("field_A77C_", "0xA77C"),
        member_declaration("column_count_", "0xA810"),
        member_declaration("cell_width_", "0xA81C"),
        member_declaration("row_height_", "0xA820"),
        member_declaration("field_B64_"),
    ))
    assert "int32_t column_count_;" in declarations
    assert "int32_t cell_width_;" in declarations
    assert "int32_t row_height_;" in declarations

    harness = r'''#include <cassert>
#include <cstdint>
#include <type_traits>
#include <vector>

struct ProdPicker {
  DECLARATIONS
  std::vector<int> events;
  void draw_item(int item, int update) { events.push_back(100 + item * 10 + update); }
  void soft_update() { events.push_back(99999); }
  void on_left_click(int x, int y);
};
static_assert(std::is_same<decltype(ProdPicker::cell_width_), int32_t>::value,
              "horizontal cell stride keeps the actual signed header type");
static_assert(std::is_same<decltype(ProdPicker::row_height_), int32_t>::value,
              "vertical row pitch keeps the actual signed header type");
static_assert(std::is_same<decltype(ProdPicker::column_count_), int32_t>::value,
              "column count keeps the actual signed header type");

BODY

int main() {
  ProdPicker p;
  p.field_A77C_ = 8;
  p.column_count_ = 4;
  p.cell_width_ = 80;
  p.row_height_ = 70;

  // Negative, non-divisible coordinates use signed x86 IDIV semantics:
  // -81/80 == -1 and -71/70 == -1, selecting row 3 from first-visible 8.
  p.field_A774_ = 11;
  p.field_B64_ = 7;
  p.on_left_click(-81, -71);
  assert(p.field_A774_ == 3 && p.field_B64_ == 0);
  assert((p.events == std::vector<int>{210, 130, 99999}));

  // C++/IDIV truncates a small negative coordinate toward zero.
  p.events.clear();
  p.field_A774_ = 9;
  p.field_B64_ = 4;
  p.on_left_click(-1, -1);
  assert(p.field_A774_ == 8 && p.field_B64_ == 0);
  assert((p.events == std::vector<int>{190, 180, 99999}));

  // Positive non-divisible coordinates preserve the original grid arithmetic.
  p.events.clear();
  p.field_A774_ = 3;
  p.field_B64_ = 5;
  p.on_left_click(161, 141);
  assert(p.field_A774_ == 18 && p.field_B64_ == 0);
  assert((p.events == std::vector<int>{130, 280, 99999}));

  // An unchanged cell is stored but does not reset rotation or redraw/update.
  p.events.clear();
  p.field_A774_ = 13;
  p.field_B64_ = 6;
  p.on_left_click(80, 70);
  assert(p.field_A774_ == 13 && p.field_B64_ == 6);
  assert(p.events.empty());
}
'''.replace("DECLARATIONS", declarations).replace("BODY", actual_click_definition())

    source = tmp_path / f"prodpicker-signed-geometry-{compiler_name.replace('+', 'p')}.cpp"
    executable = source.with_suffix("")
    source.write_text(harness)
    subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", str(source), "-o", str(executable)],
        check=True,
        capture_output=True,
        text=True,
    )
    return subprocess.run([str(executable)], capture_output=True, text=True)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_left_click_uses_signed_geometry_and_preserves_redraw_order(tmp_path, compiler_name):
    result = compile_and_run(tmp_path, compiler_name)
    assert result.returncode == 0, result.stderr
