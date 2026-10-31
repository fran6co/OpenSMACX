"""Actual-source PCX/BMP format and call-boundary controls; not game proof."""

import re
import shutil
import subprocess
import tempfile
from pathlib import Path

from recovery.tests.test_pact_ascent import definition


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/buffer.cpp").is_file())


def _source_parts():
    source = (ROOT / "src/buffer.cpp").read_text()
    view = re.search(r"struct\s+PcxHeaderView\s*\{.*?\};", source, re.S)
    prefix = re.search(r'const\s+char\s+PcxAssetPrefix\[\]\s*=\s*"[^"]*"\s*;',
                       source)
    assert view and prefix, "the actual shared PCX format view/literal is missing"
    writer = definition(source, r"int\s+Buffer::write_pcx\s*\(")
    memory = definition(source, r"int\s+Buffer::load_pcx\s*\(BYTE\s*\*data")
    dimensions = definition(source,
                            r"int\s+Buffer::get_pcx_dimensions\s*\(")
    return source, view.group(), prefix.group(), writer, memory, dimensions


def _compile_run(code, compiler, directory, expect_success=True):
    cpp = directory / "pcx_actual_source.cpp"
    binary = directory / "pcx_actual_source"
    cpp.write_text(code)
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         "-Wno-unused-parameter", str(cpp), "-o", str(binary)],
        capture_output=True, text=True, timeout=45)
    assert built.returncode == 0, built.stdout + built.stderr
    run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=15)
    assert (run.returncode == 0) == expect_success, (
        compiler, run.returncode, run.stdout, run.stderr)


WRITER_FIXTURE = r'''#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
typedef uint8_t BYTE;
typedef uint8_t uint8;
typedef uint32_t uint32;
struct RGBQUAD { uint8_t rgbBlue, rgbGreen, rgbRed, rgbReserved; };
struct BitmapInfoHeader { int biWidth, biHeight; };
struct BitmapInfo { BitmapInfoHeader bmiHeader; RGBQUAD bmiColors[256]; };
static unsigned char output_bytes[4096];
static unsigned char pixels[16] = { 1, 2, 0x41, 4, 5, 6, 7, 8 };
static int close_calls;
static void (*create_callback)();
static void (*header_copy_callback)();
static uint32_t created_size;
static char *memrchr(char *first, char *last, int value) {
  while (last != first) { --last; if ((unsigned char)*last == value) return last; }
  return 0;
}
class Filemap {
 public:
  unsigned char *create(const char *, uint32_t size, int) {
    created_size = size;
    if (create_callback) create_callback();
    return output_bytes;
  }
  void close(unsigned char *) { ++close_calls; }
};
class Buffer {
 public:
  BitmapInfo dib_;
  int stride_;
  void *locked_bits_;
  int get_data() { return 1; }
  void free_data(int) {}
  int write_pcx(char *filename);
};
// This hook observes the existing header-copy call in the extracted body.
// It does not replace product loads with service calls.
static void *copy_pcx_header(void *to, const void *from, size_t size) {
  void *result = memcpy(to, from, size);
  if (header_copy_callback) header_copy_callback();
  return result;
}
// @@PCX_HEADER_VIEW@@
#define memcpy copy_pcx_header
// @@PCX_WRITER_BODY@@
#undef memcpy
static int fail(const char *message) { fprintf(stderr, "%s\n", message); return 1; }
int main() {
  Buffer b;
  memset(output_bytes, 0, sizeof(output_bytes));
  b.dib_.bmiHeader.biWidth = 0x10005;
  b.dib_.bmiHeader.biHeight = -2;
  b.stride_ = 4;
  b.locked_bits_ = pixels;
  for (int i = 0; i < 256; ++i) {
    b.dib_.bmiColors[i].rgbRed = (uint8_t)(i + 1);
    b.dib_.bmiColors[i].rgbGreen = (uint8_t)(i + 2);
    b.dib_.bmiColors[i].rgbBlue = (uint8_t)(i + 3);
  }
  if (sizeof(PcxHeaderView) != 0x80 ||
      offsetof(PcxHeaderView, manufacturer) != 0 ||
      offsetof(PcxHeaderView, version) != 1 ||
      offsetof(PcxHeaderView, encoding) != 2 ||
      offsetof(PcxHeaderView, bits_per_pixel) != 3 ||
      offsetof(PcxHeaderView, x_min) != 4 ||
      offsetof(PcxHeaderView, y_min) != 6 ||
      offsetof(PcxHeaderView, x_max) != 8 ||
      offsetof(PcxHeaderView, y_max) != 10 ||
      offsetof(PcxHeaderView, h_res) != 12 ||
      offsetof(PcxHeaderView, v_res) != 14 ||
      offsetof(PcxHeaderView, palette) != 16 ||
      offsetof(PcxHeaderView, reserved_40) != 0x40 ||
      offsetof(PcxHeaderView, planes) != 0x41 ||
      offsetof(PcxHeaderView, bytes_per_line) != 0x42 ||
      offsetof(PcxHeaderView, palette_type) != 0x44 ||
      offsetof(PcxHeaderView, screen_width) != 0x46 ||
      offsetof(PcxHeaderView, screen_height) != 0x48 ||
      offsetof(PcxHeaderView, reserved) != 0x4a)
    return fail("format view size/offset changed");
  if (b.write_pcx((char *)"fixture") != 0 || close_calls != 1)
    return fail("actual writer result/lifecycle changed");
  if (output_bytes[0] != 10 || output_bytes[1] != 5 ||
      output_bytes[2] != 1 || output_bytes[3] != 8 ||
      output_bytes[4] != 0 || output_bytes[5] != 0 ||
      output_bytes[6] != 0 || output_bytes[7] != 0 ||
      output_bytes[8] != 4 || output_bytes[9] != 0 ||
      output_bytes[10] != 1 || output_bytes[11] != 0 ||
      output_bytes[12] != 0 || output_bytes[13] != 0 ||
      output_bytes[14] != 0 || output_bytes[15] != 0 ||
      output_bytes[0x40] != 0 || output_bytes[0x41] != 1 ||
      output_bytes[0x42] != 4 || output_bytes[0x43] != 0 ||
      output_bytes[0x44] != 0 || output_bytes[0x45] != 0 ||
      output_bytes[0x46] != 0 || output_bytes[0x47] != 0 ||
      output_bytes[0x48] != 0 || output_bytes[0x49] != 0)
    return fail("writer header fields or WORD extents changed");
  for (int i = 0x10; i < 0x40; ++i)
    if (output_bytes[i] != 0xa5) return fail("palette bytes were initialized");
  for (int i = 0x4a; i < 0x80; ++i)
    if (output_bytes[i] != 0xa5) return fail("reserved bytes were initialized");
  return 0;
}
'''


def _writer_code(view, writer):
    declaration = re.search(r"PcxHeaderView\s+header\s*;", writer)
    assert declaration, "writer must leave its local format storage uninitialized"
    # Test-only stack seeding: this instrumentation is inserted in the
    # extracted copy, never in product source. It observes which bytes the
    # actual writer assigns; it does not claim what the original stack held.
    seeded = writer[:declaration.end()] + "\n        memset(&header, 0xa5, sizeof(header));" + writer[declaration.end():]
    return (WRITER_FIXTURE
            .replace("// @@PCX_HEADER_VIEW@@", view)
            .replace("// @@PCX_WRITER_BODY@@", seeded))


WRITER_CALLBACK_MAIN = r'''static Buffer *active_buffer;
static unsigned char alternate_pixels[16] = { 9, 10, 11, 12 };
static void after_create() {
  active_buffer->dib_.bmiHeader.biWidth = 5;
  active_buffer->dib_.bmiHeader.biHeight = -3;
  active_buffer->stride_ = 2;
  active_buffer->locked_bits_ = alternate_pixels;
}
static void after_header_copy() {
  active_buffer->dib_.bmiHeader.biHeight = -1;
}
int main() {
  Buffer b;
  active_buffer = &b;
  create_callback = after_create;
  header_copy_callback = after_header_copy;
  b.dib_.bmiHeader.biWidth = 4;
  b.dib_.bmiHeader.biHeight = -2;
  b.stride_ = 4;
  b.locked_bits_ = pixels;
  memset(b.dib_.bmiColors, 0, sizeof(b.dib_.bmiColors));
  if (b.write_pcx((char *)"fixture") != 0 || close_calls != 1)
    return fail("PCX writer lifecycle changed");
  if (created_size != 144 || output_bytes[8] != 4 || output_bytes[10] != 2 ||
      output_bytes[0x42] != 2)
    return fail("PCX create-size/header dimension snapshots changed");
  if (output_bytes[128] != 1 || output_bytes[129] != 2 || output_bytes[130] != 12)
    return fail("PCX saved pixel pointer or post-copy row count changed");
  return 0;
}
'''


def _writer_callback_code(view, writer):
    code = _writer_code(view, writer)
    assert code.count("int main() {") == 1
    return code[:code.index("int main() {")] + WRITER_CALLBACK_MAIN


BMP_CALLBACK_FIXTURE = r'''#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define BI_RGB 0
typedef uint32_t DWORD;
struct RGBQUAD { uint8_t rgbBlue, rgbGreen, rgbRed, rgbReserved; };
struct BITMAPFILEHEADER {
  unsigned short bfType;
  unsigned int bfSize;
  unsigned short bfReserved1, bfReserved2;
  unsigned int bfOffBits;
};
struct BITMAPINFOHEADER {
  unsigned int biSize;
  int biWidth, biHeight;
  unsigned short biPlanes, biBitCount;
  unsigned int biCompression, biSizeImage;
  int biXPelsPerMeter, biYPelsPerMeter;
  unsigned int biClrUsed, biClrImportant;
};
struct BitmapInfo { BITMAPINFOHEADER bmiHeader; RGBQUAD bmiColors[256]; };
static int write_calls, close_calls, free_calls;
static unsigned char pixels[64];
class Buffer {
 public:
  BitmapInfo dib_;
  int stride_;
  void *locked_bits_;
  int get_data() { return 1; }
  void free_data(int) { ++free_calls; }
  int write_bmp(char *);
};
static Buffer *active_buffer;
static int header_width, header_height;
static size_t row_sizes[2];
static const void *row_pointers[2];
static FILE *env_open(const char *, const char *) {
  return reinterpret_cast<FILE *>(1);
}
static int observed_fclose(FILE *) { ++close_calls; return 0; }
static size_t observed_fwrite(const void *data, size_t size, size_t count, FILE *) {
  ++write_calls;
  if (write_calls == 2) {
    const BITMAPINFOHEADER *header = static_cast<const BITMAPINFOHEADER *>(data);
    header_width = header->biWidth;
    header_height = header->biHeight;
  } else if (write_calls == 3) {
    active_buffer->dib_.bmiHeader.biHeight = -2;
    active_buffer->dib_.bmiHeader.biWidth = 3;
    active_buffer->stride_ = 4;
    active_buffer->locked_bits_ = pixels + 8;
  } else if (write_calls > 3) {
    int row = write_calls - 4;
    if (row < 2) { row_sizes[row] = size; row_pointers[row] = data; }
    active_buffer->dib_.bmiHeader.biWidth = 2;
    active_buffer->dib_.bmiHeader.biHeight = -9;
    active_buffer->stride_ = 5;
  }
  return count;
}
#define fwrite observed_fwrite
#define fclose observed_fclose
// @@BMP_WRITER_BODY@@
#undef fwrite
#undef fclose
int main() {
  Buffer b;
  active_buffer = &b;
  b.dib_.bmiHeader.biWidth = 6;
  b.dib_.bmiHeader.biHeight = -1;
  b.stride_ = 8;
  b.locked_bits_ = pixels + 16;
  if (b.write_bmp((char *)"fixture") != 0 || close_calls != 1 ||
      free_calls != 1 || write_calls != 5) return 1;
  if (header_width != 6 || header_height != 1) return 2;
  if (row_sizes[0] != 3 || row_sizes[1] != 2 ||
      row_pointers[0] != pixels + 12 || row_pointers[1] != pixels + 7) return 3;
  return 0;
}
'''


def _bmp_callback_code(writer):
    return BMP_CALLBACK_FIXTURE.replace("// @@BMP_WRITER_BODY@@", writer)


MEMORY_FIXTURE = r'''#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
typedef uint8_t BYTE;
typedef uint32_t DWORD;
typedef void *HDC;
static const size_t PcxHeaderSize = 0x80;
struct RGBQUAD { uint8_t rgbBlue, rgbGreen, rgbRed, rgbReserved; };
struct DIB { RGBQUAD bmiColors[256]; };
class IDirectDrawSurface2 {
 public:
  int GetDC(HDC *) { return 0; }
  int ReleaseDC(HDC) { return 0; }
};
class GraphicWin {
 public:
  int resize(int, int, int) { return 0; }
};
struct Palette {
  int seed_;
  int set_calls;
  void set_from_dib(DIB *) { ++set_calls; }
  void get_rgbquad(RGBQUAD *, int, int) {}
};
// @@PCX_PREFIX@@
// @@PCX_HEADER_VIEW@@
static int seen_width, seen_height, init_calls, data_calls, free_calls;
static PcxHeaderView *mutated_header_view;
static BYTE pixel_storage[1024];
static int mutate_stride_on_init;
static unsigned int SetDIBColorTable(HDC, unsigned int, unsigned int, RGBQUAD *) { return 0; }
static void word(unsigned char *p, unsigned offset, unsigned value) {
  p[offset] = (unsigned char)value; p[offset + 1] = (unsigned char)(value >> 8);
}
class Buffer {
 public:
  IDirectDrawSurface2 *surface_;
  DIB dib_;
  unsigned int stride_;
  void *locked_bits_;
  uint8_t field_580_;
  int palette_seed_;
  int has_palette_;
  Palette *palette_;
  HDC hdc2_, hdc_;
  int hdc_lock_count_;
  unsigned int init_flags_;
  int load_pcx(BYTE *, DWORD, Palette *, int, int);
  GraphicWin *get_graphic_win() { return 0; }
  int init(int width, int height, int, void *) {
    ++init_calls; seen_width = width; seen_height = height;
    if (mutate_stride_on_init) mutated_header_view->bytes_per_line = 4;
    return 0;
  }
  void *get_data() { ++data_calls; return pixel_storage; }
  void free_data(int) { ++free_calls; }
};
// @@PCX_MEMORY_BODY@@
static int fail(const char *message) { fprintf(stderr, "%s\n", message); return 1; }
int main() {
  unsigned char bytes[0x80 + 1024];
  memset(bytes, 0, sizeof(bytes));
  bytes[0] = 10; bytes[1] = 3; bytes[2] = 1; bytes[3] = 8;
  word(bytes, 4, 0); word(bytes, 6, 0);
  word(bytes, 8, 1); word(bytes, 10, 1);
  bytes[0x41] = 1; word(bytes, 0x42, 2);
  for (int i = 0; i < 8; ++i) bytes[0x80 + i] = (unsigned char)(i + 1);
  Buffer b; b.surface_ = 0; b.stride_ = 4; b.locked_bits_ = 0;
  b.palette_seed_ = 9; b.palette_ = 0; b.hdc2_ = 0; b.hdc_ = 0;
  b.hdc_lock_count_ = 0; b.init_flags_ = 0; b.field_580_ = 0; b.has_palette_ = 0;
  memset(b.dib_.bmiColors, 0, sizeof(b.dib_.bmiColors));
  Palette palette = { 9, 0 };
  mutate_stride_on_init = 0;
  if (b.load_pcx(bytes, sizeof(bytes), &palette, 0, 0) != 3)
    return fail("old version with a palette was not rejected");
  if (init_calls || data_calls) return fail("old-version rejection called later stages");

  unsigned char prefixed[sizeof(PcxAssetPrefix) + sizeof(bytes)];
  memcpy(prefixed, PcxAssetPrefix, sizeof(PcxAssetPrefix));
  memcpy(prefixed + sizeof(PcxAssetPrefix), bytes, sizeof(bytes));
  mutate_stride_on_init = 1;
  if (b.load_pcx(prefixed, sizeof(prefixed), 0, 0, 0) != 0 ||
      seen_width != 2 || seen_height != 2 || init_calls != 1 ||
      data_calls != 1 || free_calls != 1)
    return fail("full memory decode path or dimensions changed");
  if (pixel_storage[0] != 1 || pixel_storage[1] != 2 ||
      pixel_storage[2] != 0 || pixel_storage[3] != 0 ||
      pixel_storage[4] != 3 || pixel_storage[5] != 4)
    return fail("stride WORD was not snapshotted before init; row order changed");

  memset(bytes + 0x80, 1, 300);
  bytes[1] = 4;
  word(bytes, 4, 0); word(bytes, 6, 0);
  word(bytes, 8, 0); word(bytes, 10, 0);
  word(bytes, 0x42, 0x0102);
  mutate_stride_on_init = 0;
  b.stride_ = 260;
  memset(pixel_storage, 0, sizeof(pixel_storage));
  if (b.load_pcx(bytes, sizeof(bytes), 0, 0, 0) != 0 ||
      pixel_storage[0] != 1 || pixel_storage[257] != 1)
    return fail("high WORD byte of bytes-per-line was not used");

  if (b.load_pcx(bytes, sizeof(bytes), &palette, 0, 0) != 0)
    return fail("version four with palette was rejected");
  bytes[0] = 11;
  if (b.load_pcx(bytes, sizeof(bytes), 0, 0, 0) != 3)
    return fail("bad manufacturer was accepted");
  return 0;
}
'''


def _memory_code(view, prefix, memory):
    declaration = re.search(r"PcxHeaderView\s+header\s*;", memory)
    assert declaration, "memory reader must use an actual local format view"
    # Test-only observation hook: the init stub changes this local view so the
    # actual extracted body demonstrates that its WORD local was snapshotted
    # before init. This is not a claim about original runtime aliasing.
    instrumented = (memory[:declaration.end()] +
                    "\n    mutated_header_view = &header;" +
                    memory[declaration.end():])
    return (MEMORY_FIXTURE
            .replace("// @@PCX_PREFIX@@", prefix)
            .replace("// @@PCX_HEADER_VIEW@@", view)
            .replace("// @@PCX_MEMORY_BODY@@", instrumented))


DIMENSION_FIXTURE = r'''#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define MAX_PATH 512
#define TRUE 1
static int Language;
static const char *image_content;
static int close_count;
// @@PCX_PREFIX@@
// @@PCX_HEADER_VIEW@@
static void *GetFileSize(int, void *) { return 0; }
class Filemap {
 public:
  const char *open_read(const char *, int) { return image_content; }
  int get_handle() { return 7; }
  const char *get_view() { return image_content; }
  void close() { ++close_count; }
};
class Buffer { public: static int get_pcx_dimensions(const char *, int *, int *); };
// @@PCX_DIMENSION_BODY@@
static int fail(const char *message) { fprintf(stderr, "%s\n", message); return 1; }
static void word(unsigned char *p, unsigned offset, unsigned value) {
  p[offset] = (unsigned char)value; p[offset + 1] = (unsigned char)(value >> 8);
}
int main() {
  unsigned char raw[0x80]; memset(raw, 0, sizeof(raw));
  raw[0] = 10; raw[1] = 0; raw[2] = 1;
  word(raw, 4, 0x0101); word(raw, 6, 0x0202);
  word(raw, 8, 0x0104); word(raw, 10, 0x0204);
  char prefixed[sizeof(PcxAssetPrefix) + sizeof(raw)];
  memcpy(prefixed, PcxAssetPrefix, sizeof(PcxAssetPrefix));
  memcpy(prefixed + sizeof(PcxAssetPrefix), raw, sizeof(raw));
  image_content = prefixed;
  int width = -1, height = -1;
  if (Buffer::get_pcx_dimensions("fixture.pcx", &width, &height) != 0 ||
      width != 4 || height != 3 || close_count != 1)
    return fail("dimension reader did not use prefixed WORD header");
  raw[0] = 11;
  memcpy(prefixed + sizeof(PcxAssetPrefix), raw, sizeof(raw));
  close_count = 0;
  if (Buffer::get_pcx_dimensions("fixture.pcx", &width, &height) != 3 || close_count)
    return fail("dimension reader accepted bad manufacturer or changed close path");
  raw[0] = 10; raw[2] = 2;
  memcpy(prefixed + sizeof(PcxAssetPrefix), raw, sizeof(raw));
  if (Buffer::get_pcx_dimensions("fixture.pcx", &width, &height) != 3)
    return fail("dimension reader accepted non-RLE encoding");
  return 0;
}
'''


def _dimension_code(view, prefix, dimensions):
    return (DIMENSION_FIXTURE
            .replace("// @@PCX_PREFIX@@", prefix)
            .replace("// @@PCX_HEADER_VIEW@@", view)
            .replace("// @@PCX_DIMENSION_BODY@@", dimensions))


def test_writer_actual_fields_unwritten_bytes_and_damage_controls():
    source, view, _prefix, writer, _memory, _dimensions = _source_parts()
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    assert compilers, "GCC or Clang is required for actual-source PCX controls"
    code = _writer_code(view, writer)
    for compiler in compilers:
        with tempfile.TemporaryDirectory(prefix="pcx-writer-control-") as temp:
            directory = Path(temp)
            _compile_run(code, compiler, directory)

            for old, new in (
                ("header.x_max = static_cast<short>(dib_.bmiHeader.biWidth) - 1;",
                 "header.x_min = static_cast<short>(dib_.bmiHeader.biWidth) - 1;"),
                ("header.bytes_per_line = static_cast<short>(stride_);",
                 "header.palette_type = static_cast<short>(stride_);"),
            ):
                assert writer.count(old) == 1
                _compile_run(_writer_code(view, writer.replace(old, new, 1)),
                             compiler, directory, expect_success=False)

    # This is intentionally a source invariant: seeding the extracted local
    # above is test-only and cannot establish that product code leaves it so.
    assert re.search(r"PcxHeaderView\s+header\s*;", writer)
    assert not re.search(r"PcxHeaderView\s+header\s*(?:=|\{)", writer)
    zeroed = writer.replace("PcxHeaderView header;", "PcxHeaderView header = {};", 1)
    assert not re.search(r"PcxHeaderView\s+header\s*;", zeroed), \
        "zero-initialization damage must violate the actual uninitialized-local invariant"
    assert "ORIGINAL: 0x005DF590" in source


def test_pcx_writer_preserves_original_call_boundary_snapshots():
    _source, view, _prefix, writer, _memory, _dimensions = _source_parts()
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    assert compilers, "GCC or Clang is required for actual-source writer controls"

    # Pin5DF6BE captures pixels before create5DF6DD;5DF740..75B reads
    # header dimensions afterward;5DF7BC reads height after memcpy5DF7B7.
    damage = []
    for old, new in (
        ("static_cast<short>(dib_.bmiHeader.biWidth) - 1;",
         "static_cast<short>(width) - 1;"),
        ("-1 - static_cast<short>(dib_.bmiHeader.biHeight);",
         "-1 - static_cast<short>(height);"),
        ("int rows_left = -dib_.bmiHeader.biHeight;",
         "int rows_left = -height;"),
    ):
        assert writer.count(old) == 1
        damage.append(writer.replace(old, new, 1))

    saved_pixels = "const uint8_t *bits = static_cast<const uint8_t *>(locked_bits_);"
    assert writer.count(saved_pixels) == 1
    assert writer.count("unsigned char *out = view + 0x80;") == 1
    late_pixels = writer.replace(saved_pixels, "", 1).replace(
        "unsigned char *out = view + 0x80;",
        saved_pixels + "\n        unsigned char *out = view + 0x80;", 1)
    damage.append(late_pixels)

    rows = "int rows_left = -dib_.bmiHeader.biHeight;"
    assert writer.count("if (view != 0) {") == 1
    early_rows = writer.replace(rows, "", 1).replace(
        "if (view != 0) {", "if (view != 0) {\n        " + rows, 1)
    damage.append(early_rows)

    for compiler in compilers:
        with tempfile.TemporaryDirectory(prefix="pcx-writer-snapshot-") as temp:
            directory = Path(temp)
            _compile_run(_writer_callback_code(view, writer), compiler, directory)
            for damaged in damage:
                _compile_run(_writer_callback_code(view, damaged), compiler,
                             directory, expect_success=False)


def test_bmp_writer_reloads_row_dimensions_after_writes():
    source = (ROOT / "src/buffer.cpp").read_text()
    writer = definition(source, r"int\s+Buffer::write_bmp\s*\(")
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    assert compilers, "GCC or Clang is required for actual-source writer controls"

    # Pin5DFAA5 reloads height after three writes;5DFAC7 reloads width
    # per row. Row count stays captured while5DFAD7 reloads stride.
    damage = []
    for old, new in (
        ("const int current_height = dib_.bmiHeader.biHeight;",
         "const int current_height = height;"),
        ("fwrite(row, dib_.bmiHeader.biWidth, 1, file);",
         "fwrite(row, width, 1, file);"),
    ):
        assert writer.count(old) == 1
        damage.append(writer.replace(old, new, 1))
    assert writer.count("int rows = -current_height;") == 1
    assert writer.count("row -= stride_;") == 1
    fixed_stride = writer.replace(
        "int rows = -current_height;",
        "int rows = -current_height;\n    const int row_stride = stride_;", 1)
    damage.append(fixed_stride.replace("row -= stride_;", "row -= row_stride;", 1))

    for compiler in compilers:
        with tempfile.TemporaryDirectory(prefix="bmp-writer-snapshot-") as temp:
            directory = Path(temp)
            _compile_run(_bmp_callback_code(writer), compiler, directory)
            for damaged in damage:
                _compile_run(_bmp_callback_code(damaged), compiler, directory,
                             expect_success=False)


def test_readers_actual_predicates_prefix_and_word_dimensions():
    source, view, prefix, writer, memory, dimensions = _source_parts()
    assert "PcxHeaderView header;" in writer
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    assert compilers, "GCC or Clang is required for actual-source PCX controls"
    memory_code = _memory_code(view, prefix, memory)
    dimension_code = _dimension_code(view, prefix, dimensions)
    for compiler in compilers:
        with tempfile.TemporaryDirectory(prefix="pcx-reader-control-") as temp:
            directory = Path(temp)
            _compile_run(memory_code, compiler, directory)
            _compile_run(dimension_code, compiler, directory)

            # A merged version predicate is a behavior change: the memory
            # overload rejects v<4 only for a supplied Palette; dimensions do
            # not examine version at all.
            damaged_memory, count = re.subn(
                r"if\s*\(header\.version\s*<\s*4\s*&&\s*palette\s*!=\s*0\)",
                "if (header.version < 4)", memory, count=1)
            assert count == 1
            damaged = _memory_code(view, prefix, damaged_memory)
            _compile_run(damaged, compiler, directory, expect_success=False)

            old_row_bytes = "const int row_bytes = (8 / bits_per_pixel) * bytes_per_line * planes;"
            new_row_bytes = "const int row_bytes = (8 / bits_per_pixel) * header.bytes_per_line * planes;"
            assert memory.count(old_row_bytes) == 1
            late_memory = memory.replace(old_row_bytes, new_row_bytes, 1)
            late_memory = late_memory.replace(
                "const uint16_t bytes_per_line = header.bytes_per_line;",
                "const uint16_t bytes_per_line = header.bytes_per_line; (void)bytes_per_line;",
                1)
            late_stride = _memory_code(view, prefix, late_memory)
            _compile_run(late_stride, compiler, directory, expect_success=False)

            assert "header.version" not in dimensions
            assert "ORIGINAL: 0x005E2690" in source
            assert "ORIGINAL: 0x005E2480" in source
