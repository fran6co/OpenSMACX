"""Source and actual-body controls for Console's bounded parse/base arrays."""
from __future__ import annotations

from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery.includes import COMMENTS_OR_STRINGS

ROOT = next(path for path in Path(__file__).resolve().parents
            if (path / "src/IMAGE_PIN").is_file())


def actual_body(source: str, signature: str) -> str:
    start = source.index(signature)
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " " for char in match[0]),
        source,
    )
    opening = masked.index("{", start)
    depth = 0
    for pos in range(opening, len(masked)):
        if masked[pos] == "{":
            depth += 1
        elif masked[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[start:pos + 1]
    raise AssertionError("unterminated production body")


def test_array_bindings_retain_original_extent_and_compile_symbols() -> None:
    general = (ROOT / "src/general.cpp").read_text(encoding="utf-8")
    alpha = (ROOT / "src/alpha.cpp").read_text(encoding="utf-8")
    base_header = (ROOT / "src/base.h").read_text(encoding="utf-8")
    assert "RECOVERY_OBJECT: v2 0x009BB598 0x009BB5C0 ?ParseNumTable@@3PAHA 0x00625E30" in general
    assert "int ParseNumTable[10];" in general
    assert "RECOVERY_OBJECT: v2 0x0097D040 0x009A3840 ?Bases@@3PAUBase@@A 0x004E4B80" in alpha
    assert "Base Bases[MaxBaseNum];" in alpha
    assert "static const int MaxBaseNum = 512;" in base_header
    assert "static_assert(sizeof(Base) == 0x134" in base_header
    assert 0x0097D040 + 512 * 0x134 == 0x009A3840


def test_actual_parse_num_preserves_signed_negative_id_behavior(tmp_path: Path) -> None:
    compiler = shutil.which("c++") or shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    source = (ROOT / "src/general.cpp").read_text(encoding="utf-8")
    body = actual_body(source, "int __cdecl parse_num(int id, int value)")
    harness = r'''#include <cstring>
#define __cdecl
int parse_storage[12];
int *ParseNumTable = &parse_storage[1];
''' + body + r'''
int main() {
  std::memset(parse_storage, 0, sizeof(parse_storage));
  parse_storage[0] = 41;
  if (parse_num(-1, 77) != 0 || parse_storage[0] != 77) return 1;
  if (parse_num(9, 99) != 0 || parse_storage[10] != 99) return 2;
  if (parse_num(10, 123) != 3 || parse_storage[11] != 0) return 3;
  return 0;
}
'''
    test_source = tmp_path / "parse_num_actual.cpp"
    executable = tmp_path / "parse_num_actual"
    test_source.write_text(harness, encoding="utf-8")
    built = subprocess.run([compiler, "-std=c++11", str(test_source), "-o", str(executable)],
                           capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    ran = subprocess.run([str(executable)], capture_output=True, text=True)
    assert ran.returncode == 0, ran.stdout + ran.stderr
