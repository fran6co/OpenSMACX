"""Exercise the real row packer through the decoder's void callback contract."""
from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest

from match_recovery.source import function_line


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/general.cpp").is_file())


def definition(text, signature):
    name = signature.split("(", 1)[0].split()[-1]
    line = function_line(text, name)
    start = text.index(signature, sum(len(item) + 1 for item in text.splitlines()[:line - 1]))
    opening = text.index("{", start)
    depth = 0
    for pos in range(opening, len(text)):
        if text[pos] == "{":
            depth += 1
        elif text[pos] == "}":
            depth -= 1
            if depth == 0:
                return text[start:pos + 1]
    raise AssertionError("unterminated live source definition: " + signature)


def live_source():
    general = (ROOT / "src/movie_media_decoder.cpp").read_text()
    decoder = (ROOT / "src/movie_media_decoder.cpp").read_text()
    callback_header = (ROOT / "src/movie_decoder_frame.h").read_text()
    body = definition(general, "void __cdecl sub_5d2761(")
    adapter = definition(decoder, "static void movie_decoder_pack_row(")
    assert "void __cdecl sub_5d2761" in (ROOT / "src/general.h").read_text()
    assert "extern void __cdecl sub_5d2761" in decoder
    assert "void (*pack_plane_row)(const uint32_t *source," in callback_header
    assert "uint32_t *destination, int count);" in callback_header
    assert "return 0;" not in body
    return body, adapter


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_pack_row_is_void_callback_and_writes_ordered_words(compiler_name):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + " unavailable")
    body, adapter = live_source()
    fixture = r'''#include <assert.h>
#include <stdint.h>
typedef uint32_t DWORD;
#define __cdecl
static DWORD MovieColourTableByte0[512], MovieColourTableByte1[512];
static DWORD MovieColourTableByte3[512];
static DWORD MovieBlendTableHi[512], MovieBlendTableMid[512];
static DWORD MovieBlendTableLow[512];
''' + body + "\n" + adapter + r'''
typedef void (__cdecl *RowPackHelper)(unsigned int *, unsigned int *, int);
typedef void (*PackRowCallback)(const uint32_t *, uint32_t *, int);

// Fixed outputs derived from the pinned byte-lane lookup order and table
// values initialized below; keep this oracle independent of the live formula.
static const uint32_t expected_words[3] = {
  0x007dba24u, 0x01f89af9u, 0x00bc841cu
};

static void check_output(const uint32_t *output) {
  assert(output[0] == 0xdeadbeefu);
  for (int i = 0; i < 3; ++i) {
    assert(output[i + 1] == expected_words[i]);
  }
  assert(output[4] == 0xdeadbeefu);
}

int main() {
  for (unsigned int i = 0; i < 512; ++i) {
    MovieColourTableByte0[i] = i * 40009u + 13u;
    MovieColourTableByte1[i] = i * 27103u + 29u;
    MovieColourTableByte3[i] = i * 31991u + 47u;
    MovieBlendTableHi[i] = i * 0x10203u + 3u;
    MovieBlendTableMid[i] = i * 0x10101u + 5u;
    MovieBlendTableLow[i] = i * 0x1201u + 7u;
  }
  uint32_t source[4] = {0x14321107u, 0x381123abu, 0x99542c11u, 0xeaa00077u};
  uint32_t output[5];
  for (int i = 0; i < 5; ++i) output[i] = 0xdeadbeefu;

  // The helper's actual type is void; the pointer assignment rejects an
  // invented int result while checking the source-level callable contract.
  RowPackHelper helper = &sub_5d2761;
  helper(source, output + 1, 3);
  check_output(output);

  for (int i = 0; i < 5; ++i) output[i] = 0xdeadbeefu;
  PackRowCallback callback = &movie_decoder_pack_row;
  callback(source, output + 1, 3);
  check_output(output);
}
'''
    with tempfile.TemporaryDirectory(prefix="movie-pack-row-contract-") as temp:
        source = Path(temp) / "control.cpp"
        binary = Path(temp) / "control"
        source.write_text(fixture)
        built = subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-pedantic",
             str(source), "-o", str(binary)], capture_output=True, text=True)
        assert built.returncode == 0, built.stdout + built.stderr
        ran = subprocess.run([str(binary)], capture_output=True, text=True)
        assert ran.returncode == 0, ran.stdout + ran.stderr
