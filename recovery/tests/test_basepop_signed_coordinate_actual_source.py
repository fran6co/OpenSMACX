"""Actual-source controls for BasePop's signed optional X member.

These host tests check the extracted source declarations/branches and their
32-bit arithmetic. They do not prove VC6 ABI, game callers, or linked layout.
"""
from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def read(path):
    return (ROOT / path).read_text(encoding="utf-8")


def definition(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 1
    for pos in range(brace + 1, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[start:pos + 1]
    raise AssertionError("unterminated definition: " + signature)


def block_from(text, start_text, condition):
    start = text.index(start_text)
    condition_at = text.index(condition, start)
    brace = text.index("{", condition_at)
    depth = 1
    for pos in range(brace + 1, len(text)):
        if text[pos] == "{":
            depth += 1
        elif text[pos] == "}":
            depth -= 1
            if depth == 0:
                end = pos + 1
                tail = text[end:].lstrip()
                if tail.startswith("else"):
                    else_at = end + len(text[end:]) - len(tail)
                    else_brace = text.index("{", else_at + len("else"))
                    else_depth = 1
                    for else_pos in range(else_brace + 1, len(text)):
                        if text[else_pos] == "{":
                            else_depth += 1
                        elif text[else_pos] == "}":
                            else_depth -= 1
                            if else_depth == 0:
                                end = else_pos + 1
                                break
                return text[start:end]
    raise AssertionError("unterminated source branch: " + condition)


def source_parts(unsigned=False):
    header = read("src/basepop.h")
    basepop = read("src/basepop.cpp")
    popup = read("src/popup.cpp")
    declared = re.search(r"^\s*(?:int32_t|uint32_t) field_21CC_;\s*$", header, re.M)
    assert declared, "BasePop field declaration missing"
    field_decl = declared.group(0).strip()
    assert field_decl == ("int32_t field_21CC_;" if not unsigned
                          else "uint32_t field_21CC_;"), field_decl
    border_decl = re.search(r"^\s*uint32_t field_30E0_;\s*$", header, re.M)
    left_decl = re.search(r"^\s*uint32_t field_30BC_;\s*$", header, re.M)
    limit_decl = re.search(r"^\s*uint32_t field_30D8_;\s*$", header, re.M)
    assert border_decl and left_decl and limit_decl
    assert "static_assert(sizeof(BasePop) == 0x3230" in header
    popup_header = read("src/popup.h")
    assert "class Popup : public BasePop" in popup_header
    assert "static_assert(sizeof(Popup) == 0x537C" in popup_header
    assert basepop.count("field_21CC_ = -1;") == 2

    # This coordinate branch is in BasePop::exec (0x00602600), not in
    # BasePop::start (0x00601BF0); keep the source identity explicit.
    start_body = definition(basepop, "int BasePop::exec(int flags,")
    start = block_from(start_body, "int dialog_x =",
                       "if (dialog_x < 0)")
    popup_body = definition(popup, "int Popup::on_draw_background()")
    popup_guard = block_from(popup_body, "if (field_21CC_ >= 0)",
                             "if (field_21CC_ >= 0)")
    calc_body = definition(basepop, "int BasePop::calculate_dimensions()")
    calc = block_from(calc_body,
                      "int edge = dialogs_.Dialog::field_5C_ != 0",
                      "if (field_21CC_ == -1)")
    assert "static_cast<int>(field_21CC_)" not in start_body + popup_body
    return field_decl, border_decl.group(0).strip(), left_decl.group(0).strip(), \
        limit_decl.group(0).strip(), start, popup_guard, calc


SUPPORT = r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

uint32_t BasePopDrawnRight = 0;
struct Dialog { int field_5C_, field_2C_; };
struct Dialogs : Dialog {};
struct BasePopProbe {
  /* FIELD */
  /* LIMIT */
  /* BORDER */
  /* LEFT */
  Dialogs dialogs_;
  int exec_dialog_x() { /* START */ return dialog_x; }
  int popup_x(int x) { /* POPUP */ return x; }
  uint32_t calculate_left() { /* CALC */ return field_30BC_; }
};

uint32_t wide_modulo_sum(int32_t coordinate, uint32_t border, int edge) {
  int64_t wide = static_cast<int64_t>(coordinate) +
                 static_cast<int64_t>(border) + static_cast<int64_t>(edge);
  return static_cast<uint32_t>(static_cast<uint64_t>(wide) & 0xffffffffULL);
}

int main() {
  const uint32_t raw[] = {0u, 1u, 0x7fffffffu, 0x80000000u,
                          0xfffffffeu, 0xffffffffu};
  BasePopProbe p;
  p.field_30D8_ = 7u;
  p.field_30E0_ = 0xfffffff1u;
  p.dialogs_.field_5C_ = 5;
  p.dialogs_.field_2C_ = 9;
  BasePopDrawnRight = 0x1234u;
  for (unsigned i = 0; i != sizeof(raw) / sizeof(raw[0]); ++i) {
    int32_t coordinate;
    memcpy(&coordinate, &raw[i], sizeof(coordinate));
    p.field_21CC_ = coordinate;

    const int expected_start = coordinate < 0 ? 0x1234 : coordinate;
    assert(p.exec_dialog_x() == expected_start);
    const int initial_x = 0x55;
    assert(p.popup_x(initial_x) == (coordinate >= 0 ? coordinate : initial_x));

    const uint32_t actual = p.calculate_left();
    uint32_t expected;
    if (raw[i] == 0xffffffffu) {
      expected = static_cast<uint32_t>((uint64_t(5) +
                         uint64_t(p.field_30E0_) * 2u) & 0xffffffffULL);
    } else {
      expected = wide_modulo_sum(coordinate, p.field_30E0_, 7);
    }
    if (actual != expected) {
      fprintf(stderr, "raw=%08x coord=%d actual=%08x expected=%08x\\n",
              raw[i], coordinate, actual, expected);
      return 10;
    }
  }

  // Cover the exact -1 sentinel's re-selection when the initial edge exceeds
  // field_30D8_; this branch retains the original field_5C_/field_2C_ choice.
  p.field_21CC_ = -1;
  p.field_30D8_ = 3u;
  p.field_30E0_ = 0x80000001u;
  p.dialogs_.field_5C_ = 8;
  p.dialogs_.field_2C_ = 11;
  assert(p.calculate_left() == static_cast<uint32_t>(
      (uint64_t(8) + uint64_t(0x80000001u) * 2u) & 0xffffffffULL));
  return 0;
}
'''


def program(parts):
    field, border, left, limit, start, popup, calc = parts
    replacements = {
        "/* FIELD */": field,
        "/* LIMIT */": limit,
        "/* BORDER */": border,
        "/* LEFT */": left,
        "/* START */": start,
        "/* POPUP */": popup,
        "/* CALC */": calc,
    }
    result = SUPPORT
    for marker, value in replacements.items():
        assert result.count(marker) == 1
        result = result.replace(marker, value)
    return result


def compiler_names():
    available = [(name, shutil.which(name)) for name in ("g++", "clang++")]
    return [(name, path) for name, path in available if path]


def compile_and_run(tmp_path, compiler, text, stem):
    tmp_path.mkdir(parents=True, exist_ok=True)
    source = tmp_path / (stem + ".cpp")
    binary = tmp_path / stem
    source.write_text(text, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-O2", "-Wall", "-Wextra",
         "-fsanitize=undefined", "-fno-sanitize-recover=undefined",
         str(source), "-o", str(binary)], capture_output=True, text=True, timeout=40,
    )
    assert built.returncode == 0, built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
    return ran


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_signed_coordinate_and_modulo_arithmetic(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host compiler unavailable")
    parts = source_parts()
    run = compile_and_run(tmp_path, compiler, program(parts), "actual")
    assert run.returncode == 0, run.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_source_rejects_unsigned_or_collapsed_sentinel_damages(
        compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host compiler unavailable")
    parts = source_parts()
    compile_and_run(tmp_path / "control", compiler, program(parts), "control")

    # Same-width unsigned storage makes the uncast Popup test always true.
    unsigned_parts = list(parts)
    unsigned_parts[0] = "uint32_t field_21CC_;"
    damages = [program(unsigned_parts)]

    # Start/Popup treat every negative coordinate as fallback/keep; replacing
    # those with exact -1 would incorrectly collapse the -2 and INT_MIN paths.
    any_negative_start = list(parts)
    any_negative_start[4] = parts[4].replace(
        "if (dialog_x < 0)", "if (dialog_x == -1)", 1)
    damages.append(program(any_negative_start))

    # calculate_dimensions deliberately distinguishes exactly -1 from -2.
    any_negative_dimensions = list(parts)
    any_negative_dimensions[6] = parts[6].replace(
        "if (field_21CC_ == -1)", "if (field_21CC_ < 0)", 1)
    damages.append(program(any_negative_dimensions))

    for index, damaged in enumerate(damages):
        result = compile_and_run(tmp_path / ("damage" + str(index)), compiler,
                                 damaged, "mutant")
        assert result.returncode != 0, (compiler_name, index)

