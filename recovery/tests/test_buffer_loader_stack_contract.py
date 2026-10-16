"""Actual-source controls for the TGA/BMP loader stack contracts."""

from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/buffer.cpp").is_file())


def _definition(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated actual-source wrapper")


def test_loader_declarations_are_receiver_free_without_overclaiming_origin():
    header = (ROOT / "src/buffer.h").read_text()
    source = (ROOT / "src/buffer.cpp").read_text()
    for name, args in (
        ("load_tga", r"const char\s*\*\s*a1,\s*Palette\s*\*\s*a2,\s*int\s+a3,\s*int\s+a4"),
        ("load_tga", r"const char\s*\*\s*filename"),
        ("load_bmp", r"const char\s*\*\s*filename,\s*Palette\s*\*\s*palette,\s*int\s+a3,\s*int\s+a4"),
        ("load_bmp", r"const char\s*\*\s*filename"),
    ):
        assert re.search(r"static\s+int\s+__stdcall\s+" + name +
                         r"\s*\(" + args + r"\)", header)
    assert "historical QAG" in header
    assert "not a provenance claim" in header
    assert "ORIGINAL: 0x005D7B20" in source
    assert "ORIGINAL: 0x005D7C80" in source


@pytest.fixture(scope="module")
def actual_wrappers(tmp_path_factory):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    source = (ROOT / "src/buffer.cpp").read_text()
    tga = _definition(source, "int __stdcall Buffer::load_tga(const char *filename")
    bmp = _definition(source, "int __stdcall Buffer::load_bmp(const char *filename")
    harness = r'''#include <cstdlib>
#include <cstring>
#define __stdcall
struct Palette {};
struct Buffer {
  static int __stdcall load_tga(const char *, Palette *, int, int);
  static int __stdcall load_tga(const char *);
  static int __stdcall load_bmp(const char *, Palette *, int, int);
  static int __stdcall load_bmp(const char *);
};
static const char *seen_name;
static Palette *seen_palette;
static int seen_a3, seen_a4, seen_result;
int __stdcall Buffer::load_tga(const char *name, Palette *palette, int a3, int a4) {
  seen_name=name; seen_palette=palette; seen_a3=a3; seen_a4=a4; return seen_result;
}
int __stdcall Buffer::load_bmp(const char *name, Palette *palette, int a3, int a4) {
  seen_name=name; seen_palette=palette; seen_a3=a3; seen_a4=a4; return seen_result;
}
'''
    harness += tga + "\n" + bmp + r'''
int main() {
  const char *name="asset";
  seen_result=37;
  if (Buffer::load_tga(name)!=37 || seen_name!=name || seen_palette!=0 ||
      seen_a3!=0 || seen_a4!=0x100) return 1;
  if (Buffer::load_bmp(name)!=37 || seen_name!=name || seen_palette!=0 ||
      seen_a3!=0 || seen_a4!=0x100) return 2;
  return 0;
}
'''
    work = tmp_path_factory.mktemp("buffer-loader-stack-contract")
    cpp, exe = work / "control.cpp", work / "control"
    cpp.write_text(harness)
    subprocess.run([compiler, "-std=c++17", str(cpp), "-o", str(exe)], check=True)
    return exe


def test_actual_one_argument_wrappers_forward_explicit_values(actual_wrappers):
    subprocess.run([str(actual_wrappers)], check=True)
