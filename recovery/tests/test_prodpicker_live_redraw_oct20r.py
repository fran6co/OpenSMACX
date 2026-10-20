"""Actual-source controls for ProdPicker's live redraw loop bound."""

from __future__ import annotations

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/prodpicker.cpp"
SIGNATURE = "void ProdPicker::on_redraw()"
HOST_COMPILERS = tuple(
    path for name in ("g++", "clang++") if (path := shutil.which(name)) is not None
)


def actual_definition() -> str:
    from match_recovery.includes import COMMENTS_OR_STRINGS

    source = SOURCE.read_text(encoding="utf-8")
    code = COMMENTS_OR_STRINGS.sub(lambda match: "".join(
        "\n" if char == "\n" else " " for char in match[0]
    ), source)
    if code.count(SIGNATURE) != 1:
        raise AssertionError(f"expected one production definition: {SIGNATURE}")
    start = code.index(SIGNATURE)
    opening = code.index("{", start)
    depth = 0
    for end in range(opening, len(code)):
        if code[end] == "{":
            depth += 1
        elif code[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError("unterminated ProdPicker::on_redraw definition")


HARNESS = r'''#include <cstdio>
#include <cstdlib>
#include <utility>
#include <vector>

struct ProdPicker {
  enum Mutation { NONE, GROW_COUNT, SHRINK_COUNT, MOVE_FIRST };
  int field_A77C_;
  int field_A82C_;
  int mutation;
  int draw_limit;
  std::vector<std::pair<int, int> > draws;

  ProdPicker() : field_A77C_(0), field_A82C_(0), mutation(NONE),
                 draw_limit(16), draws() {}

  void draw_item(int item, int update) {
    if (draws.size() >= static_cast<std::size_t>(draw_limit)) {
      std::fprintf(stderr, "bounded draw limit reached\n");
      std::exit(71);
    }
    draws.push_back(std::make_pair(item, update));
    if (draws.size() != 1) return;
    if (mutation == GROW_COUNT) field_A82C_ = 4;
    if (mutation == SHRINK_COUNT) field_A82C_ = 2;
    if (mutation == MOVE_FIRST) {
      field_A77C_ = 8;
      field_A82C_ = 1;
    }
  }

  void on_redraw();
};

BODY

static void check(const ProdPicker &picker, const int *items,
                  std::size_t count, const char *name) {
  if (picker.draws.size() != count) {
    std::fprintf(stderr, "%s: expected %lu draws, got %lu\n", name,
                 static_cast<unsigned long>(count),
                 static_cast<unsigned long>(picker.draws.size()));
    std::exit(72);
  }
  for (std::size_t index = 0; index < count; ++index) {
    if (picker.draws[index].first != items[index] ||
        picker.draws[index].second != 0) {
      std::fprintf(stderr, "%s: wrong item/update at %lu: %d/%d\n", name,
                   static_cast<unsigned long>(index),
                   picker.draws[index].first, picker.draws[index].second);
      std::exit(73);
    }
  }
}

int main() {
  {
    ProdPicker picker;
    picker.field_A77C_ = 2;
    picker.field_A82C_ = 3;
    const int expected[] = {2, 3, 4};
    picker.on_redraw();
    check(picker, expected, 3, "stable");
  }
  {
    ProdPicker picker;
    picker.field_A77C_ = 5;
    picker.field_A82C_ = 0;
    picker.on_redraw();
    check(picker, 0, 0, "empty");
  }
  {
    ProdPicker picker;
    picker.field_A77C_ = 4;
    picker.field_A82C_ = -2;
    picker.on_redraw();
    check(picker, 0, 0, "negative-count");
  }
  {
    ProdPicker picker;
    picker.field_A77C_ = -3;
    picker.field_A82C_ = 2;
    const int expected[] = {-3, -2};
    picker.on_redraw();
    check(picker, expected, 2, "negative-origin");
  }
  {
    ProdPicker picker;
    picker.field_A77C_ = 4;
    picker.field_A82C_ = 2;
    picker.mutation = ProdPicker::GROW_COUNT;
    const int expected[] = {4, 5, 6, 7};
    picker.on_redraw();
    check(picker, expected, 4, "grown-count");
  }
  {
    ProdPicker picker;
    picker.field_A77C_ = 4;
    picker.field_A82C_ = 7;
    picker.mutation = ProdPicker::SHRINK_COUNT;
    const int expected[] = {4, 5};
    picker.on_redraw();
    check(picker, expected, 2, "shrunk-count");
  }
  {
    ProdPicker picker;
    picker.field_A77C_ = 4;
    picker.field_A82C_ = 2;
    picker.mutation = ProdPicker::MOVE_FIRST;
    const int expected[] = {4, 5, 6, 7, 8};
    picker.on_redraw();
    check(picker, expected, 5, "moved-first-retains-cursor");
  }
  return 0;
}
'''


def compile_and_run(tmp_path: Path, compiler: str, body: str,
                    label: str) -> subprocess.CompletedProcess[str]:
    unit = tmp_path / f"{label}.cpp"
    executable = tmp_path / label
    unit.write_text(HARNESS.replace("BODY", body), encoding="utf-8")
    subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         str(unit), "-o", str(executable)],
        check=True, capture_output=True, text=True, timeout=30,
    )
    return subprocess.run([str(executable)], capture_output=True,
                          text=True, timeout=10)


@pytest.mark.parametrize("compiler", HOST_COMPILERS)
def test_redraw_uses_live_bound_and_retains_initial_cursor(tmp_path, compiler):
    body = actual_definition()
    result = compile_and_run(tmp_path, compiler, body, "redraw_actual")
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("compiler", HOST_COMPILERS)
def test_live_bound_damage_mutants_fail_bounded_controls(tmp_path, compiler):
    body = actual_definition()
    condition = "item < field_A77C_ + field_A82C_"
    assert body.count(condition) == 1

    frozen = body.replace(
        "int item = static_cast<int>(field_A77C_);",
        "int item = static_cast<int>(field_A77C_);\n"
        "    const int frozen_end = field_A77C_ + field_A82C_;",
        1,
    ).replace(condition, "item < frozen_end", 1)
    restarted = body.replace(
        "draw_item(item, 0);",
        "draw_item(item, 0);\n        item = field_A77C_ - 1;",
        1,
    )
    removed_call = body.replace("draw_item(item, 0);", "(void)item;", 1)
    assert frozen != body and restarted != body and removed_call != body

    for label, mutant in (("redraw_frozen", frozen),
                          ("redraw_restarted", restarted),
                          ("redraw_no_call", removed_call)):
        result = compile_and_run(tmp_path, compiler, mutant, label)
        assert result.returncode != 0, label
