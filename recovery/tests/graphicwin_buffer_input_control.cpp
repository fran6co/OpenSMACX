// Compiled with actual source definitions by test_graphicwin_buffer_input.py.
// This deliberately small host projection tests behavior, not x86 layout,
// COFF symbols, original claims, or native acceptance.
#include <stdio.h>
#include <string.h>

typedef unsigned char BYTE;
typedef unsigned int DWORD;
struct Palette { int id; };

extern int WinScreenWidth;
extern int WinScreenHeight;
extern Palette *PaletteActive;

class Buffer;
struct PcxInputView;

class Win {
 public:
  virtual int resize(int width, int height, int repaint);
  int move(int x, int y);
};

class Buffer {
 public:
  int probe_image_file(const char *, Palette *, int, int);
  int load_pcx_from_source(PcxInputView *, Palette *, int, int);
  int load_pcx(BYTE *, DWORD, Palette *, int, int);
  int sync_to_palette(Palette *);
};

class GraphicWin : public Win, public Buffer {
 public:
  int probe_image_file(const char *, Palette *, int, int);
  int load_pcx_from_source(PcxInputView *, Palette *, int, int);
  int resize_screen_and_sync_palette();
};

static char calls[16];
static int call_count;
static int resize_result;
static int helper_result;
static int decode_result;
static int observed_width;
static int observed_height;
static int observed_repaint;
static int observed_x;
static int observed_y;
static const char *observed_filename;
static Palette *observed_palette;
static int observed_base;
static int observed_count;
static BYTE *observed_data;
static DWORD observed_size;
static int helper_calls;
static int decode_calls;
static int sync_calls;

static void note(char c) {
  if (call_count < (int)sizeof(calls)) calls[call_count++] = c;
}

int WinScreenWidth = 1024;
int WinScreenHeight = 768;
static Palette active_palette = { 17 };
Palette *PaletteActive = &active_palette;

int Win::resize(int width, int height, int repaint) {
  note('R');
  observed_width = width;
  observed_height = height;
  observed_repaint = repaint;
  return resize_result;
}

int Win::move(int x, int y) {
  note('M');
  observed_x = x;
  observed_y = y;
  return 0;
}

int Buffer::probe_image_file(const char *filename, Palette *palette,
                             int color_base, int color_count) {
  note('P');
  ++helper_calls;
  observed_filename = filename;
  observed_palette = palette;
  observed_base = color_base;
  observed_count = color_count;
  return helper_result;
}

int Buffer::load_pcx(BYTE *data, DWORD size, Palette *palette,
                     int color_base, int color_count) {
  note('D');
  ++decode_calls;
  observed_data = data;
  observed_size = size;
  observed_palette = palette;
  observed_base = color_base;
  observed_count = color_count;
  return decode_result;
}

int Buffer::sync_to_palette(Palette *palette) {
  note('S');
  ++sync_calls;
  observed_palette = palette;
  return 0;
}

// Candidate source bodies are appended here by run_controls.py.

static int fail(const char *message) {
  fprintf(stderr, "%s\n", message);
  return 1;
}

int main() {
  GraphicWin window;
  Palette local_palette = { 23 };
  const char *filename = "host-control.pcx";

  helper_result = 41;
  if (window.probe_image_file(filename, &local_palette, 5, 16) != 41)
    return fail("probe wrapper did not return delegated result");
  if (helper_calls != 1 || observed_filename != filename ||
      observed_palette != &local_palette || observed_base != 5 ||
      observed_count != 16)
    return fail("probe wrapper changed its arguments");

  BYTE pixels[4] = { 1, 2, 3, 4 };
  PcxInputView source;
  memset(&source, 0, sizeof(source));
  decode_result = 37;
  if (window.load_pcx_from_source(0, &local_palette, 7, 8) != 3)
    return fail("null source status changed");
  source.data_ = 0;
  if (window.load_pcx_from_source(&source, &local_palette, 7, 8) != 7)
    return fail("null data status changed");
  if (decode_calls != 0) return fail("invalid source reached decoder");
  source.data_ = pixels;
  source.size_ = 4;
  if (window.load_pcx_from_source(&source, &local_palette, 7, 8) != 37)
    return fail("valid source did not return decoder result");
  if (decode_calls != 1 || observed_data != pixels || observed_size != 4 ||
      observed_palette != &local_palette || observed_base != 7 ||
      observed_count != 8)
    return fail("source fields/arguments were not forwarded exactly");

  call_count = 0;
  resize_result = 19;
  if (window.resize_screen_and_sync_palette() != 19)
    return fail("resize error result was not preserved");
  if (call_count != 1 || calls[0] != 'R' || sync_calls != 0)
    return fail("resize failure did not short-circuit cleanup/update");
  if (observed_width != 1024 || observed_height != 768 || observed_repaint != 0)
    return fail("screen resize arguments changed");

  call_count = 0;
  resize_result = 0;
  if (window.resize_screen_and_sync_palette() != 0)
    return fail("successful resize must return zero");
  if (call_count != 3 || calls[0] != 'R' || calls[1] != 'M' || calls[2] != 'S')
    return fail("native resize/move/palette order changed");
  if (observed_x != 0 || observed_y != 0 || sync_calls != 1 ||
      observed_palette != PaletteActive)
    return fail("move or active-palette target changed");
  return 0;
}
