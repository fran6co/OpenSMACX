"""Behavior controls for the actual alt_natural owner body."""

from pathlib import Path
import resource
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src" / "map_grid.cpp"
SIGNATURE = "int __cdecl alt_natural(int x, int y) {"


def _matching_brace(source: str, opening: int) -> int:
    depth = 0
    state = "code"
    escaped = False
    pos = opening
    while pos < len(source):
        char = source[pos]
        next_char = source[pos + 1] if pos + 1 < len(source) else ""
        if state == "line_comment":
            if char == "\n":
                state = "code"
        elif state == "block_comment":
            if char == "*" and next_char == "/":
                state = "code"
                pos += 1
        elif state in ("string", "character"):
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif (state == "string" and char == '"') or (
                state == "character" and char == "'"
            ):
                state = "code"
        else:
            if char == "/" and next_char == "/":
                state = "line_comment"
                pos += 1
            elif char == "/" and next_char == "*":
                state = "block_comment"
                pos += 1
            elif char == '"':
                state = "string"
            elif char == "'":
                state = "character"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return pos
        pos += 1
    raise AssertionError("unterminated alt_natural definition")


def actual_definition() -> str:
    source = SOURCE.read_text()
    assert source.count(SIGNATURE) == 1
    start = source.index(SIGNATURE)
    opening = source.index("{", start)
    closing = _matching_brace(source, opening)
    return source[start : closing + 1]


HARNESS = r"""
#include <cassert>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <stdint.h>

#define __cdecl

const int ALT_3_LEVELS_ABOVE_SEA = 6;
int MapSeaLevel = 0;
uint32_t AltNatural[11] = { 0 };
int detail_value = 0;
int detail_calls = 0;
int detail_x_seen = 0;
int detail_y_seen = 0;

int alt_detail_at(int x, int y) {
  ++detail_calls;
  detail_x_seen = x;
  detail_y_seen = y;
  return detail_value;
}

FUNCTION_BODY

static int64_t signed32_oracle(uint32_t bits) {
  if (bits & 0x80000000u)
    return static_cast<int64_t>(bits) - INT64_C(0x100000000);
  return static_cast<int64_t>(bits);
}

static int expected_altitude(int detail, int sea) {
  const uint32_t wrapped = static_cast<uint32_t>(detail) -
                           static_cast<uint32_t>(sea);
  const int64_t contour = signed32_oracle(wrapped);
  int index = 6;
  while (contour < signed32_oracle(AltNatural[index]) && index != 0)
    --index;
  return index;
}

static void run_case(int detail, int sea, int x, int y) {
  detail_value = detail;
  detail_calls = 0;
  detail_x_seen = detail_y_seen = -999;
  MapSeaLevel = sea;
  const int expected = expected_altitude(detail, sea);
  const int actual = alt_natural(x, y);
  assert(detail_calls == 1);
  assert(detail_x_seen == x && detail_y_seen == y);
  if (actual != expected) {
    std::fprintf(stderr, "detail=%d sea=%d expected=%d actual=%d\n",
                 detail, sea, expected, actual);
    std::abort();
  }
}

static void set_profile(int profile) {
  if (profile == 0) {
    const uint32_t values[7] = { 0u, 20u, 40u, 60u, 80u, 100u, 120u };
    for (int i = 0; i != 7; ++i)
      AltNatural[i] = values[i];
  } else if (profile == 1) {
    const uint32_t values[7] = {
      0x80000000u, 0xffffffffu, 0x80000001u, 0u,
      1u, 0x7fffffffu, 0xfffffffeu
    };
    for (int i = 0; i != 7; ++i)
      AltNatural[i] = values[i];
  } else {
    const uint32_t values[7] = {
      0x7fffffffu, 0x80000000u, 0xffffffffu, 1u,
      0u, 0x7ffffffeu, 0x80000001u
    };
    for (int i = 0; i != 7; ++i)
      AltNatural[i] = values[i];
  }
  AltNatural[7] = AltNatural[8] = AltNatural[9] = AltNatural[10] = 0xdeadbeefu;
}

int main() {
  // A concrete ordinary below-sea witness distinguishes unsigned ordering.
  set_profile(0);
  run_case(20, 21, 37, -12);

  const int sea_values[] = {
    INT_MIN, INT_MIN + 1, -100, -21, -1, 0, 1, 20, 21, 255, 256,
    INT_MAX - 1, INT_MAX
  };
  for (int profile = 0; profile != 3; ++profile) {
    set_profile(profile);
    for (int detail = 0; detail != 256; ++detail) {
      for (unsigned int si = 0;
           si != sizeof(sea_values) / sizeof(sea_values[0]); ++si) {
        const int x = detail - 91;
        const int y = sea_values[si] ^ (profile * 17);
        run_case(detail, sea_values[si], x, y);
      }
    }
    uint32_t bits = 0x76543210u;
    for (int sample = 0; sample != 512; ++sample) {
      bits = bits * 1664525u + 1013904223u;
      // The wide oracle is in int's range before conversion; do not borrow
      // the candidate's implementation-defined unsigned-to-signed cast.
      const int sea = static_cast<int>(signed32_oracle(bits));
      run_case(sample & 255, sea, sample - 257, profile * 11);
    }
  }
}
"""


def _compiler(name: str) -> str:
    path = shutil.which(name)
    if path is None:
        pytest.skip(f"{name} is unavailable")
    return path


def _compile_run(tmp_path: Path, compiler: str, body: str,
                 should_pass: bool) -> None:
    program = HARNESS.replace("FUNCTION_BODY", body)
    source_path = tmp_path / "alt_natural.cpp"
    executable = tmp_path / "alt_natural"
    source_path.write_text(program)
    built = subprocess.run(
        [compiler, "-std=c++98", "-O2", "-Wall", "-Wextra", "-Werror",
         "-pedantic", "-fsanitize=undefined", "-fno-sanitize-recover=all",
         str(source_path), "-o", str(executable)],
        capture_output=True,
        text=True,
        cwd=tmp_path,
        timeout=30,
    )
    assert built.returncode == 0, built.stderr
    def no_core():
        _, hard = resource.getrlimit(resource.RLIMIT_CORE)
        resource.setrlimit(resource.RLIMIT_CORE, (0, hard))
    result = subprocess.run([str(executable)], capture_output=True, text=True,
                            cwd=tmp_path, timeout=10, preexec_fn=no_core)
    assert (result.returncode == 0) is should_pass, result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_alt_natural_matches_wrapped_signed_oracle(
    tmp_path: Path, compiler_name: str
) -> None:
    _compile_run(tmp_path, _compiler(compiler_name), actual_definition(), True)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_compile_success_altitude_mutants_are_detected(
    tmp_path: Path, compiler_name: str
) -> None:
    compiler = _compiler(compiler_name)
    body = actual_definition()
    signed_conversion = (
        "const int32_t contour = static_cast<int32_t>(\n"
        "        static_cast<uint32_t>(alt_detail_at(x, y)) - static_cast<uint32_t>(MapSeaLevel));"
    )
    unsigned_mutant = body.replace(
        signed_conversion,
        "const uint32_t contour = static_cast<uint32_t>(alt_detail_at(x, y)) - "
        "static_cast<uint32_t>(MapSeaLevel);",
        1,
    ).replace(
        "contour >= static_cast<int32_t>(AltNatural[threshold])",
        "contour >= AltNatural[threshold]",
        1,
    )
    assert unsigned_mutant != body
    _compile_run(tmp_path, compiler, unsigned_mutant, False)

    wrong_start = body.replace(
        "int threshold = ALT_3_LEVELS_ABOVE_SEA;",
        "int threshold = ALT_3_LEVELS_ABOVE_SEA - 1;",
        1,
    )
    assert wrong_start != body
    _compile_run(tmp_path, compiler, wrong_start, False)

    inclusive_threshold = body.replace(
        "contour >= static_cast<int32_t>(AltNatural[threshold])",
        "contour > static_cast<int32_t>(AltNatural[threshold])",
        1,
    )
    assert inclusive_threshold != body
    _compile_run(tmp_path, compiler, inclusive_threshold, False)

    wrong_sea_sign = body.replace(
        "static_cast<uint32_t>(alt_detail_at(x, y)) - static_cast<uint32_t>(MapSeaLevel)",
        "static_cast<uint32_t>(alt_detail_at(x, y)) + static_cast<uint32_t>(MapSeaLevel)",
        1,
    )
    assert wrong_sea_sign != body
    _compile_run(tmp_path, compiler, wrong_sea_sign, False)
