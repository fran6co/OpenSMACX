from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]


def _body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for pos in range(opening, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[start:pos + 1]
    raise AssertionError(f"unterminated definition: {signature}")


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_sub_5d2901_wraps_before_logical_shift(
    tmp_path: Path, compiler_name: str
) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    general = (ROOT / "src/general.cpp").read_text()
    function = _body(general, "int __cdecl sub_5d2901(")
    unit = tmp_path / "sink.cpp"
    unit.write_text(r'''#include <cassert>
#include <cstdint>
#define __cdecl
using DWORD = unsigned int;
DWORD MovieColourTableByte0[256], MovieColourTableByte1[256];
DWORD MovieColourTableByte3[256];
DWORD MovieBlendTableHi[512], MovieBlendTableMid[512], MovieBlendTableLow[512];
''' + function + r'''
static void init_tables() {
  for (unsigned i = 0; i != 256; ++i) {
    MovieColourTableByte0[i] = i;
    MovieColourTableByte1[i] = i << 8;
    MovieColourTableByte3[i] = i << 16;
  }
  for (unsigned i = 0; i != 512; ++i) {
    MovieBlendTableLow[i] = i * 37U + 5U;
    MovieBlendTableMid[i] = i * 101U + 11U;
    MovieBlendTableHi[i] = i * 157U + 19U;
  }
}
int main() {
  init_tables();
  int a[2] = {0x7fffffff, 0x12345678};
  int b[2] = {1, 0x23456789};
  DWORD out[4] = {};
  sub_5d2901(a, b, reinterpret_cast<int *>(out), 2);
  for (unsigned i = 0; i != 2; ++i) {
    const DWORD lanes = ((static_cast<DWORD>(a[i]) +
                          static_cast<DWORD>(b[i])) >> 1) & 0x7f7f7f7fU;
    const DWORD common = (lanes & 0xffU) + (lanes & 0xff00U);
    const DWORD colours[2] = {
        common + ((lanes >> 24) << 16),
        common + (((lanes >> 16) & 0xffU) << 16)};
    for (unsigned j = 0; j != 2; ++j) {
      const DWORD colour = colours[j];
      const DWORD expected = ((colour & 511U) * 37U + 5U) +
          (((colour >> 11) & 511U) * 101U + 11U) +
          ((colour >> 23) * 157U + 19U);
      assert(out[2 * i + j] == expected);
    }
  }
}
''')
    binary = tmp_path / "sink"
    subprocess.run(
        [compiler, "-std=c++17", "-O2", "-ftrapv", "-Wall", "-Wextra",
         "-Werror", str(unit), "-o", str(binary)],
        check=True,
    )
    subprocess.run([str(binary)], check=True, timeout=10)
