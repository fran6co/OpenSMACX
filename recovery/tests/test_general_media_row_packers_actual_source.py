"""Live-body row-packer controls with synthetic tables; not native ABI or codec table initialization proof."""

from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest


def repository_root() -> Path:
    for candidate in Path(__file__).resolve().parents:
        if (candidate / "src/general.cpp").is_file():
            return candidate
    raise AssertionError("could not locate source-owned general.cpp")


ROOT = repository_root()
SOURCE = ROOT / "src/general.cpp"


def definition(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start : end + 1]
    raise AssertionError("unterminated definition: " + signature)


HOST_PREFIX = r"""
#include <cassert>
#include <cstdint>
#include <cstring>
typedef uint32_t DWORD;
typedef unsigned char BYTE;
#define __cdecl
struct MockConversion {
  BYTE prefix[0x70c];
  struct { DWORD words[1024]; } remap_or_lookup_storage;
};
static MockConversion movie_media_conversion_tables = {};
static DWORD MovieBlendTableData[512];
static DWORD MovieBlendTableQuant[512];
static DWORD MovieBlendTableQuant2[512];
static DWORD MovieBlendTableHi[512];
static DWORD MovieBlendTableMid[512];
static DWORD MovieBlendTableLow[512];
"""

HOST_MAIN = r"""
static DWORD blend(DWORD sum) {
  return MovieBlendTableHi[sum >> 23] |
         MovieBlendTableMid[(sum & 0xff800u) >> 11] |
         MovieBlendTableLow[sum & 0x1ffu];
}

int main() {
  for (DWORD i = 0; i != 512; ++i) {
    MovieBlendTableHi[i] = i << 23;
    MovieBlendTableMid[i] = i << 11;
    MovieBlendTableLow[i] = i;
  }

  int data[16] = {};
  int selectors[128] = {};
  for (int i = 0; i != 8; ++i) {
    data[2*i] = 10 + i;
    data[2*i+1] = 100 + i;
    selectors[i] = 20 + i;
    selectors[64+i] = 40 + i;
    MovieBlendTableData[data[2*i]] = 0x1000u + (DWORD)i;
    MovieBlendTableData[data[2*i+1]] = 0x2000u + (DWORD)i;
    MovieBlendTableQuant[selectors[i]] = 0x3000u + (DWORD)i;
    MovieBlendTableQuant2[selectors[64+i]] = 0x4000u + (DWORD)i;
  }
  // A modular-add edge distinguishes the x86 DWORD accumulator contract.
  MovieBlendTableData[data[0]] = 0xfffffff0u;
  MovieBlendTableQuant[selectors[0]] = 0x30u;
  MovieBlendTableQuant2[selectors[64]] = 0x20u;

  DWORD out16[18];
  for (int i = 0; i != 18; ++i) out16[i] = 0xdeadbeefu;
  sub_5d1b88(out16 + 1, data, selectors);
  assert(out16[0] == 0xdeadbeefu && out16[17] == 0xdeadbeefu);
  for (int i = 0; i != 8; ++i) {
    DWORD even = MovieBlendTableQuant[selectors[i]] +
                 MovieBlendTableData[data[2*i]] +
                 MovieBlendTableQuant2[selectors[64+i]];
    DWORD odd = MovieBlendTableData[data[2*i+1]] +
                MovieBlendTableQuant2[selectors[64+i]] +
                MovieBlendTableQuant[selectors[i]];
    assert(out16[1+2*i] == blend(even));
    assert(out16[2+2*i] == blend(odd));
  }

  DWORD out8[10];
  for (int i = 0; i != 10; ++i) out8[i] = 0xa5a5a5a5u;
  sub_5d2037(data, selectors, out8 + 1);
  assert(out8[0] == 0xa5a5a5a5u && out8[9] == 0xa5a5a5a5u);
  for (int i = 0; i != 8; ++i) {
    DWORD even = MovieBlendTableQuant[selectors[i]] +
                 MovieBlendTableData[data[2*i]] +
                 MovieBlendTableQuant2[selectors[64+i]];
    DWORD odd = MovieBlendTableData[data[2*i+1]] +
                MovieBlendTableQuant2[selectors[64+i]] +
                MovieBlendTableQuant[selectors[i]];
    assert(out8[1+i] == ((blend(odd) & 0xffff0000u) |
                         (blend(even) & 0xffffu)));
  }

  // Pinned pack table is at owner+0xF0C (words[512]), not owner+0x90C
  // (words[128]). Give the old offset a conflicting pattern as a mutant trap.
  for (DWORD i = 0; i != 256; ++i) {
    movie_media_conversion_tables.remap_or_lookup_storage.words[128+2*i] = 0xeeu;
  }
  for (DWORD i = 0; i != 256; ++i) {
    movie_media_conversion_tables.remap_or_lookup_storage.words[512+2*i] =
        7u + (i * 13u) % 241u;
  }
  for (int i = 0; i != 16; ++i) data[i] = 80 + i;
  for (int i = 0; i != 8; ++i) {
    selectors[i] = 10 + i;
    selectors[64+i] = 30 + i;
  }
  int packed_guard[10];
  for (int i = 0; i != 10; ++i) packed_guard[i] = (int)0x12345678u;
  sub_5d2536(data, selectors, packed_guard + 1);
  assert((DWORD)packed_guard[0] == 0x12345678u);
  assert((DWORD)packed_guard[9] == 0x12345678u);
  for (DWORD i = 0; i != 8; ++i) {
    const DWORD upper =
        (movie_media_conversion_tables.remap_or_lookup_storage.words[
             512 + 2*(DWORD)selectors[64+i]] << 8) |
        movie_media_conversion_tables.remap_or_lookup_storage.words[
             512 + 2*(DWORD)data[2*i+1]];
    const DWORD lower =
        (movie_media_conversion_tables.remap_or_lookup_storage.words[
             512 + 2*(DWORD)selectors[i]] << 8) |
        movie_media_conversion_tables.remap_or_lookup_storage.words[
             512 + 2*(DWORD)data[2*i]];
    const DWORD expected = (upper << 16) | lower;
    assert((DWORD)packed_guard[1+i] == expected);
  }
  return 0;
}
"""


def harness(source: str) -> str:
    bodies = [
        definition(source, "static int movie_blend_read_index("),
        definition(source, "static DWORD movie_blend_read_word("),
        definition(source, "static DWORD movie_blend_low_half_ordered("),
        definition(source, "static DWORD movie_blend_high_half_ordered("),
        definition(source, "void __cdecl sub_5d1b88("),
        definition(source, "void __cdecl sub_5d2037("),
        definition(source, "static DWORD movie_coefficient_pack_word("),
        definition(source, "void __cdecl sub_5d2536("),
    ]
    return HOST_PREFIX + "\n".join(bodies) + HOST_MAIN


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_installed_helpers_match_typed_input_output_contracts(compiler_name: str) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is not installed")
    source = SOURCE.read_text(encoding="utf-8")
    generated = harness(source)
    with tempfile.TemporaryDirectory(prefix="osmx-general-row-packers-") as temp:
        cpp = Path(temp) / "host.cpp"
        exe = Path(temp) / "host"
        cpp.write_text(generated, encoding="utf-8")
        subprocess.run(
            [compiler, "-std=c++17", "-O2", "-Wall", "-Wextra", str(cpp), "-o", str(exe)],
            check=True,
            capture_output=True,
            text=True,
        )
        subprocess.run([str(exe)], check=True, capture_output=True, text=True)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("old,new", [
    ("remap_or_lookup_storage.words[512]", "remap_or_lookup_storage.words[128]"),
    ("upper_selector_index = selector_indices[0x40 + index]",
     "upper_selector_index = coefficient_indices[0x40 + index]"),
    ("const DWORD even_sum = selector_0 + data_even + selector_1;",
     "const DWORD even_sum = (int)selector_0 + (int)data_even + (int)selector_1;"),
])
def test_wrong_lookup_operands_and_signed_overflow_are_rejected(
        compiler_name: str, old: str, new: str, tmp_path: Path) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is not installed")
    source = SOURCE.read_text(encoding="utf-8")
    assert old in source
    generated = harness(source.replace(old, new, 1))
    # Keep the wrong-operand mutant within its fixture arrays, so rejection
    # establishes differing output rather than relying on an invalid read.
    generated = generated.replace("int data[16] = {};", "int data[128] = {};")
    # Cross INT_MAX rather than unsigned zero: this traps signed-overflow
    # normalization even when an optimizing host happens to produce wrap.
    generated = generated.replace("0xfffffff0u;", "0x7ffffff0u;")
    cpp = tmp_path / "mutant.cpp"
    exe = tmp_path / "mutant"
    cpp.write_text(generated, encoding="utf-8")
    subprocess.run(
        [compiler, "-std=c++17", "-O2", "-fsanitize=undefined",
         "-fno-sanitize-recover=undefined", str(cpp), "-o", str(exe)],
        check=True, capture_output=True, text=True, timeout=30)
    result = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)
    assert result.returncode != 0, "incorrect packer mutant escaped the control"
