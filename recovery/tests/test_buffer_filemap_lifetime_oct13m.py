"""Actual-source Filemap path/lifetime controls; not native SEH proof."""

import re
import shutil
import subprocess
import tempfile
from pathlib import Path

from recovery.tests.test_pact_ascent import definition


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/buffer.cpp").is_file())


def _actual_source():
    source = (ROOT / "src/buffer.cpp").read_text()
    extensions = []
    for name in ("BufferExtTgf", "BufferExtTgg", "BufferExtTga",
                 "BufferExtBmf", "BufferExtBmg", "BufferExtBmp"):
        match = re.search(r"const\s+char\s+" + name + r"\[\]\s*=\s*\"[^\"]*\"\s*;",
                          source)
        assert match, "actual extension literal missing: " + name
        extensions.append(match.group())
    tga = definition(source, r"int\s+__stdcall\s+Buffer::load_tga\s*\(const char\s*\*\s*a1")
    bmp = definition(source, r"int\s+__stdcall\s+Buffer::load_bmp\s*\(const char\s*\*\s*a1")
    return source, "\n".join(extensions), tga, bmp


FIXTURE = r'''#include <stdio.h>
#include <string.h>
#include <stdint.h>
#define __stdcall
struct Palette {};
static int Language;
static int ctor_calls, dtor_calls, open_calls, handle_calls, filesize_calls;
static int throw_from_ctor, throw_from_open, open_sequential;
static void *open_result;
static int last_handle;
static char last_path[512];
static uint32_t filesize_result;
// @@ACTUAL_EXTENSIONS@@
static uint32_t GetFileSize(int handle, int *) {
  ++filesize_calls; last_handle = handle; return filesize_result;
}
class Filemap {
 public:
  Filemap() {
    ++ctor_calls;
    if (throw_from_ctor) throw 41;
  }
  ~Filemap() { ++dtor_calls; }
  void *open_read(const char *path, int sequential) {
    ++open_calls; open_sequential = sequential;
    strncpy(last_path, path, sizeof(last_path) - 1);
    last_path[sizeof(last_path) - 1] = '\0';
    if (throw_from_open) throw 42;
    return open_result;
  }
  int get_handle() { ++handle_calls; return 73; }
};
class Buffer {
 public:
  int __stdcall load_tga(const char *, Palette *, int, int);
  int __stdcall load_bmp(const char *, Palette *, int, int);
};
// @@ACTUAL_TGA_BODY@@
// @@ACTUAL_BMP_BODY@@
static int fail(const char *message) { fprintf(stderr, "%s\n", message); return 1; }
static void reset() {
  ctor_calls = dtor_calls = open_calls = handle_calls = filesize_calls = 0;
  throw_from_ctor = throw_from_open = open_sequential = 0;
  open_result = (void *)1; last_handle = 0; last_path[0] = '\0';
  filesize_result = 0xfedcba98U;
}
static int check_null(Buffer &buffer, int bmp) {
  reset();
  const int result = bmp ? buffer.load_bmp(0, 0, 0, 0)
                         : buffer.load_tga(0, 0, 0, 0);
  return result == 3 && ctor_calls == 1 && dtor_calls == 1 &&
         open_calls == 0 && filesize_calls == 0 ? 0 : 1;
}
static int check_open_failure(Buffer &buffer, int bmp) {
  reset(); open_result = 0;
  const int result = bmp ? buffer.load_bmp("asset", 0, 3, 7)
                         : buffer.load_tga("asset", 0, 3, 7);
  return result == 6 && ctor_calls == 1 && dtor_calls == 1 &&
         open_calls == 1 && open_sequential == 1 &&
         handle_calls == 0 && filesize_calls == 0 ? 0 : 1;
}
static int check_success(Buffer &buffer, int bmp) {
  reset();
  const int result = bmp ? buffer.load_bmp("asset", 0, 3, 7)
                         : buffer.load_tga("asset", 0, 3, 7);
  return result == 8 && ctor_calls == 1 && dtor_calls == 1 &&
         open_calls == 1 && open_sequential == 1 &&
         handle_calls == 1 && filesize_calls == 1 && last_handle == 73 ? 0 : 1;
}
static int check_path(Buffer &buffer, int bmp, int language,
                      const char *input, const char *expected) {
  reset(); Language = language;
  const int result = bmp ? buffer.load_bmp(input, 0, 0, 0)
                         : buffer.load_tga(input, 0, 0, 0);
  if (result != 8 || strcmp(last_path, expected) != 0 || dtor_calls != 1)
    return 1;
  return 0;
}
static int check_open_throw(Buffer &buffer, int bmp) {
  reset(); throw_from_open = 1;
  try {
    if (bmp) buffer.load_bmp("asset", 0, 0, 0);
    else buffer.load_tga("asset", 0, 0, 0);
  } catch (int value) {
    return value == 42 && ctor_calls == 1 && dtor_calls == 1 &&
           open_calls == 1 && handle_calls == 0 ? 0 : 1;
  }
  return 1;
}
static int check_ctor_throw(Buffer &buffer, int bmp) {
  reset(); throw_from_ctor = 1;
  try {
    if (bmp) buffer.load_bmp(0, 0, 0, 0);
    else buffer.load_tga(0, 0, 0, 0);
  } catch (int value) {
    return value == 41 && ctor_calls == 1 && dtor_calls == 0 &&
           open_calls == 0 ? 0 : 1;
  }
  return 1;
}
int main() {
  Buffer buffer;
  for (int bmp = 0; bmp != 2; ++bmp) {
    if (check_null(buffer, bmp)) return fail("null result or ctor-before-null lifetime changed");
    if (check_open_failure(buffer, bmp)) return fail("open failure result/cleanup changed");
    if (check_success(buffer, bmp)) return fail("success/GetFileSize/lifetime changed");
    if (check_open_throw(buffer, bmp)) return fail("open exception cleanup changed");
    if (check_ctor_throw(buffer, bmp)) return fail("throwing-constructor lifetime changed");
  }
  if (check_path(buffer, 0, 1, "asset", "asset.tgf") ||
      check_path(buffer, 0, 2, "asset", "asset.tgg") ||
      check_path(buffer, 0, 0, "asset", "asset.tga") ||
      check_path(buffer, 1, 1, "asset", "asset.bmf") ||
      check_path(buffer, 1, 2, "asset", "asset.bmg") ||
      check_path(buffer, 1, 0, "asset", "asset.bmp"))
    return fail("localized extension selection changed");
  if (check_path(buffer, 0, 0, ".hidden", ".hidden.tga") ||
      check_path(buffer, 1, 0, ".hidden", ".hidden.bmp"))
    return fail("leading-period path behavior changed");
  if (check_path(buffer, 0, 0, "dir.with.dot/leaf", "dir.with.dot/leaf") ||
      check_path(buffer, 1, 0, "dir.with.dot/leaf", "dir.with.dot/leaf") ||
      check_path(buffer, 0, 0, "already.ext", "already.ext") ||
      check_path(buffer, 1, 0, "already.ext", "already.ext"))
    return fail("backward extension scan behavior changed");
  return 0;
}
'''


def _program(extensions, tga, bmp):
    return (FIXTURE
            .replace("// @@ACTUAL_EXTENSIONS@@", extensions)
            .replace("// @@ACTUAL_TGA_BODY@@", tga)
            .replace("// @@ACTUAL_BMP_BODY@@", bmp))


def _compile_run(code, compiler, directory, success=True):
    cpp = directory / "filemap_actual_source.cpp"
    binary = directory / "filemap_actual_source"
    cpp.write_text(code)
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         "-Wno-unused-parameter", str(cpp), "-o", str(binary)],
        capture_output=True, text=True, timeout=45)
    assert built.returncode == 0, built.stdout + built.stderr
    run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=15)
    assert (run.returncode == 0) == success, (compiler, run.returncode, run.stdout, run.stderr)


def test_actual_filemap_loader_paths_lifetimes_and_mutations():
    source, extensions, tga, bmp = _actual_source()
    # These body-local invariants deliberately catch a host-semantic pass from
    # the old artificial catch/rethrow: the pinned image has cleanup-only
    # Filemap lifetime metadata, not a body catch map.
    for name, body in (("load_tga", tga), ("load_bmp", bmp)):
        assert "try" not in body and "catch" not in body, \
            name + " must not force an original-absent catch/rethrow"
        assert body.index("Filemap fm;") < body.index("if (a1 == 0)")
        assert body.index("fm.open_read(path, 1)") < body.index("GetFileSize")
        assert body.index("GetFileSize") < body.index("return 8;")
    assert "ORIGINAL: 0x005D7B40" in source
    assert "ORIGINAL: 0x005D7CA0" in source

    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    assert compilers, "GCC or Clang is required for actual-source lifetime controls"
    code = _program(extensions, tga, bmp)
    for compiler in compilers:
        with tempfile.TemporaryDirectory(prefix="buffer-filemap-lifetime-") as temp:
            directory = Path(temp)
            _compile_run(code, compiler, directory)

            # Return-path and literal mutations must be observed behaviorally.
            changed_status = _program(
                extensions,
                tga.replace("return 6;", "return 60;", 1),
                bmp)
            _compile_run(changed_status, compiler, directory, success=False)
            changed_suffix = _program(
                extensions.replace('const char BufferExtTga[] = ".tga";',
                                   'const char BufferExtTga[] = ".tgx";', 1),
                tga, bmp)
            _compile_run(changed_suffix, compiler, directory, success=False)
