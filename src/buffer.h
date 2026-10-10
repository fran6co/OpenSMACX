/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenSMACX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenSMACX. If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include "original_seam.h"
#include "font.h"
#include "spot.h"
#include "sprite.h"

extern "C" int __cdecl sub_62d100();

class Palette;
class StringList;

// Buffer::write_raw_l dispatches this optional four-argument cdecl hook.
typedef int (__cdecl *BufferWriteRawCallback)(LPSTR, int, int, int);
typedef void (__cdecl *BufferWriteStringsCallback)(int, int);

// A polygon vertex, named by the image rather than invented: every one of the
// 35 mangled names that mentions it spells `PAUVert@@`, and `U` is MSVC's tag
// for a struct. It has to be `struct` for that reason and not as a style
// choice - declaring `class Vert;` would mangle `Buffer::poly` as `PAVVert@@`
// and match no symbol the target objects hold. Its signed coordinate members
// are defined in vert.h; this header only needs the pointer declaration.
struct Vert;

// DirectDraw surface handle passed to `init`. No definition is known - the
// original forwards it straight into the DirectDraw create call - so a forward
// declaration is all `init`'s signature needs to compile.
// `Buffer` holds an `IDirectDrawSurface *` and calls `GetDC`, `ReleaseDC`,
// `Lock` and `Unlock` on it - from `get_hdc` and `release_hdc`, whose bodies
// have to live in this header to be inlined the way the image inlines them
// into `set_clip`. So the full interface is needed here, not forward
// declarations.
#include <ddraw.h>

// 0x005D7852: `Buffer::init` takes the caller's surface from +0 and its
// clipper from +4 when the `tgl & 4` flag says the storage is borrowed.
struct ExtDirectDraw {
  IDirectDrawSurface *surface;
  IDirectDrawClipper *clipper;
};

 /*
  * Buffer class
  */
/*
 * A BITMAPINFO WITH ITS FULL 256-ENTRY COLOUR TABLE. The name is the image's
 * own, not this project's: `?set_from_dib@Palette@@QAEHPAUDib@@@Z` takes a
 * `struct Dib *` - `PAU` is a pointer to a struct, `Dib@@` puts it at global
 * scope - and the one call to it recovered so far hands it `buffer + 0x7C`.
 *
 * Windows declares `BITMAPINFO` with a one-entry table, so the 256-entry
 * form always needs a struct of its own; this is what the original called
 * theirs. Header plus table is 40 + 1024 = 0x428 bytes, and 0x7C + 0x428 is
 * exactly `Buffer::palette_seed_`, so the block is accounted for to the byte.
 */
struct Dib : BITMAPINFO {
    /*
     * BITMAPINFO IS THE BASE, not a member and not something this is cast
     * to. `CreateDIBSection` wants a `BITMAPINFO *` and gets one by
     * derivation - `CreateDIBSection(hdc_, &dib_, ...)`, no cast - which is
     * the whole reason for spelling it this way.
     *
     * Windows declares `bmiColors[1]`, so a 256-colour DIB needs 255 more
     * immediately behind it, and `bmiColors` is then indexed across the
     * whole 256. That is the idiom Windows documents for this structure and
     * every Win32 program that allocates a palettised DIB relies on it; the
     * alternative is a `reinterpret_cast` at the API call, which trades one
     * fudge for another and hides the layout instead of declaring it.
     *
     * 44 + 1020 = 0x428, and 0x7C + 0x428 is exactly `Buffer::field_4A4_`,
     * so the block is still accounted for to the byte.
     */
    RGBQUAD bmiColorsRest[255];
};
// The arithmetic above is a compiler-checked extent. Native layout checks
// observe an embedded Dib through compiler debug information.
static_assert(sizeof(Dib) == 0x428, "Dib is BITMAPINFOHEADER + 256 RGBQUAD");

class Buffer;
class GraphicWin;
class CheckBox;

/*
 * The panel-drawing record consumed by Buffer::box_sprite. Original
 * load_box_sprites (0x453A70) writes 51 mutable records, after extracting
 * their sprites and copying their masks. These borrow real objects; they
 * are neither immutable blocks nor fixed executable-address tables.
 *
 *   +0x00..+0x0c  the four CORNERS, drawn once each at 1:1 - top-left,
 *                 top-right, bottom-left, bottom-right in that order
 *   +0x10..+0x1c  the four EDGES, tiled between the corners - top, bottom,
 *                 left, right in that order
 *   +0x20         the pattern buffer every element is masked through, and
 *                 the source the panel's centre is tiled from
 *   +0x24         the colour the mask is read at
 *
 * A null entry skips that element: a panel with no corners is all edges, and
 * a null mask skips both the shadows and the centre fill.
 */
struct BoxSpriteParams {
  Sprite *corner0_;  // 0x00 top-left
  Sprite *corner1_;  // 0x04 top-right
  Sprite *corner2_;  // 0x08 bottom-left
  Sprite *corner3_;  // 0x0c bottom-right
  Sprite *edge0_;    // 0x10 top edge
  Sprite *edge1_;    // 0x14 bottom edge
  Sprite *edge2_;    // 0x18 left edge
  Sprite *edge3_;    // 0x1c right edge
  Buffer *mask_;     // 0x20 pattern buffer and centre source
  int mask_color_;   // 0x24
};

static_assert(sizeof(BoxSpriteParams) == 0x28,
              "BoxSpriteParams is ten dwords: eight sprite pointers, a mask "
              "buffer and a colour");

extern BoxSpriteParams InterfaceBoxStyles[51];

// Observed input prefix at 5D7AE0: data +0x0C, size +0x10 on the original
// 32-bit ABI. Producer, full allocation and original type name remain unknown.
struct PcxInputView {
  unsigned char prefix_00_0b[0x0C];
  BYTE *data_;
  DWORD size_;
};

class Buffer {
 public:
// Descriptive utility names express observed roles where original symbol
// evidence has not established a source spelling.
int draw_unk2(Buffer *, int, int, int, int, int, int);
int draw(Buffer *, int, int, int, int, int, unsigned char *);
int draw_dest_unk2(Buffer *, int, int, int, int, int, unsigned char *);
// Names provisional; no direct original symbol names recovered for utilities.
int probe_image_file(const char *, Palette *, int, int);
// Provisional name; trailing words are unused, with no known semantic types.
int check_pcx_input_data(PcxInputView *, int, int, int);
int load_pcx_from_source(PcxInputView *, Palette *, int, int);
static int __stdcall probe_pcx(const char *, Palette *, int, int);
int copy_scaled(Buffer *, RECT *, RECT *);
int copy_scaled_to_window(Win *, RECT *, RECT *);
void set_dc_text_color(unsigned int);
// Private helper; no added original mapping. Color index must be in0..255
// when init_flags_&4 selects DIB RGB conversion, as in native unguarded path.
COLORREF text_color_to_gdi(uint32_t) const;
void select_text_slot(int);
int draw_pie(RECT *, int, int, int, int, int, int);
int gouraud(Vert *, int *, int, int, unsigned char);


// New descriptive names below are role names, not original mangled evidence.
// 005DB400
int write_vertical(Font *font, char *text, RECT *area);
// The shorter vertical-text overload owns original 005DB450.
int write_vertical(char *text, RECT *area);
// 005DE2E0
int wrap_cent_flying(char *text, int x, int y, int width);
// Required rehome prerequisite 005DE320, currently a free fastcall scaffold.
int wrap_cent_flying(char *text);
// 005DE4D0
int wrap_cent_flying_flush();
// 005E3690
int write_multi_font_raw(char *text, int x, int y);
// 005E3730. This MUST NOT replace the existing four-argument write_cent_l:
// native 005DD250 interprets its fourth argument as maximum text length,
// whereas this wrapper uses rectangle width and computes the full length.
int write_cent(char *text, int x, int y, int width);
// 005E3770; forwards a borrowed pointer returned by find_line_break_l.
char *find_line_break(char *text, int width);


void draw_dest_unk1(Buffer *, int, int, unsigned char *);
void draw_dest_unk1(Buffer *, int, int, int, int, unsigned char *);

 public:
  const RECT &current_clip() const { return rect1_; }
  void set_write_strings_callback(BufferWriteStringsCallback callback) {
    field_C_ = callback;
  }
  // The list-box draw callback uses the DIB's live width at Buffer+0x80.
  int pixel_width() const { return dib_.bmiHeader.biWidth; }
  // A top-down DIB stores the visible height as a negative biHeight.
  int pixel_height() const { return -dib_.bmiHeader.biHeight; }
  // homed from 005d7b40.cpp
  int __stdcall load_tga(const char * a1, Palette * a2, int a3, int a4);
  // 0x005D7B20 - `?load_tga@Buffer@@QAGHPBD@Z`, the one-argument overload:
  // forwards to the four-argument loader above with no palette and the full
  // 256-entry colour count. __stdcall like the loader pair (QAG, not QAE).
  int __stdcall load_tga(const char *filename);

 public:
  // Blit this buffer into a window's client area. Called by the homed
  // Win painters; its own definition is still an artifact. Arity from
  // the call site.
  int copy_to_window(Win *target, int src_x, int src_y, int x, int y,
                      int w, int h);
  // homed from 005d8290.cpp
  // -1 means "the fill colour": the body substitutes WinFillColour.
  void setup_buff_sprite(int colour);
  static void release_buff_sprite();

 public:
  // homed from 005dd300.cpp
  // X FIRST. The image adds `field_width - text_width` to the SECOND
  // parameter and returns that parameter untouched on every early exit, and
  // the raster writer it tails into hands back the advanced pen position -
  // so the pen position (x) is the second argument and y is the third. Two
  // decompiled callers name the third argument `y`, which agrees.
  int write_right_l(char *text, int x_coord, int y_coord, int field_width,
                    int len);
  // The RECT overload: same right-aligned text, one rectangle instead of
  // four coordinates. Called by the homed caption painters.
  int write_right_l(char *text, RECT *area, int len);

 public:
  // 0x00626620 stores the third argument as the vertex count and uses the
  // fourth argument's low byte for solid spans. The member delegates to it.
  int poly(Vert *verts, int count, int color);
  // 0x005D7210. The body is in buffer.cpp beside its marker.
  Buffer();
  // VIRTUAL, AND THE VPTR MEMBER IS GONE. What the image stores at offset
  // 0 is a C++ vtable, and this tree used to spell it as an explicit
  // `LPVOID vtable_` written by hand. The slots are `??_GBuffer@@UAEPAXI@Z`
  // at 0 and `sub_406b30` at 1; declared virtual, the compiler puts the
  // pointer there itself and `sizeof` stays 0x588.
  //
  // 0x005D7410, and the body really is this: see the annotation in
  // buffer.cpp beside the marker, where the definition now lives.
  // VIRTUAL, and not negotiable: the image's Buffer-side vftable
  // (0x0066FC48, the one ??1GraphicWin stores at +0x444) opens with a
  // deleting-destructor THUNK (0x005D7160) - a secondary-table thunk only
  // exists when the destructor is virtual. A 2026-08-30 experiment spelled
  // it non-virtual while chasing ??1Console's member-teardown shape; that
  // changed nothing there and would have emptied this slot.
  // OUT OF LINE, 2026-09-03, superseding the in-class spelling twice measured.
  // First measurement (mapwin batch): moving the definition to buffer.cpp fixed
  // ??1MapWin (0x00420F90, three `call ??1Buffer` where the in-class spelling
  // expands) and looked like a 1-for-1 trade - deleting_thunks.cpp's
  // ??_GBuffer claim (0x5E3610) went UNRESOLVED, VC6 emitting the destructor
  // trio only where the inline definition is used. Second measurement (midi
  // batch) dissolved the trade: with the stand-in's symbol annotation switched
  // to ?scalar_delete_buffer@@YIPAXPAX0I@Z the compiler's OWN ??_GBuffer is
  // emitted in buffer.obj and the stand-in still measures 11/11 - both sides
  // hold. The in-class definition also let /O2's /Ob1 expand close()+~Spot
  // into every local Buffer's teardown, where the image makes
  // `call ??1Buffer@@QAE@XZ` at all 87 of its sites (checkbox, radiobutton,
  // sprite, palette textcolor_init). Same lever as general.h's bitmask_call.
  MEASURED virtual ~Buffer();
  // Slot 1 queries the owning GraphicWin, or null for a plain Buffer.
  // load_pcx dispatches resize through the returned window's slot 3.
  virtual GraphicWin *get_graphic_win();
  void resize(int width, int height);

  void clear_links();
  void set_markup_enabled(int enabled) { markup_enabled_ = enabled; }
  // FileWin::populate_directory branches on the existing DIB pixel pointer.
  bool has_dib_bits() const { return dib_bits_ != nullptr; }
  bool has_locked_bits() const { return locked_bits_ != nullptr; }
  // 0x10, the wrap width every writer measures against: wrap_height and
  // wrap_height_flying seed their running width from it
  // (`field_14_ = field_10_`, src/buffer.cpp), and NetWin::draw_session_info
  // opens its eight-row panel by storing 0x14 into it (netwin.cpp:726, the
  // raw store that body still spells through the layout). Named from that
  // evidence; the field stays protected, only the store goes public.
  void set_wrap_width(uint32_t width) { field_10_ = width; }
  // InfoWin resets both the drawing and measuring markup cursors between
  // partially revealed text runs (0x00458803-0x00458819).
  void reset_text_state() {
    write_font_slot_ = 0;
    font_slot_ = 0;
    field_514_ = 0;
    markup_pending_ = 0;
  }
  void restore_clip() { set_clip(&rect2_); }
  const RECT &clip_rect() const { return rect2_; }
  int set_font(Font *font1, Font *font2, Font *font3, Font *font4);
  void set_text_color(int color1, int color2, int color3, int color4);
  // PullDown's mnemonic underline snapshots Buffer+0x53C before measuring.
  int foreground_color() const { return static_cast<int>(color_val_1_); }
  void set_text_color2(int color1, int color2, int color3, int color4);
  void set_text_color3(int color1, int color2, int color3, int color4);
  void set_text_color_hyper(int color1, int color2, int color3, int color4);
  // `static`: the image's name is `?init_class@Buffer@@QAAHXZ` - `QAA` is a
  // public member declared __cdecl, which takes no receiver - and
  // jackal_init_real calls it with no ecx set up.
  static int init_class();
  void close_class();
  int sync_to_palette(Palette *palette);
  // IN-CLASS, because the image INLINES BOTH into every drawing primitive
  // that touches the pixels - `hline`, `vline`, `fill`, `copy` - and VC6
  // inlines only what it can see here. Each still emits its own COMDAT, so
  // each keeps the claim beside its marker in buffer.cpp.
  // Pointer-valued source contract: original 5E3380/5E3396/5E33DE returns
  // dib_bits_, locked_bits_, or DirectDraw lpSurface. The stripped image
  // does not establish the original declared return type; void* preserves
  // that observed storage identity without asserting an original prototype.
  MEASURED void *get_data() {   // 005E3373
      // ONE `return 0` FOR EVERY FAILURE, at the end: the image's `je` at
      // 0x005E3388 jumps the whole length of the body to 0x005E33E5, where the
      // Lock failure lands too. Written as an early `return 0` inside the first
      // arm, VC6 emits a second epilogue there and the tail moves.
      if (surface_ == nullptr) {
          // Without a surface the buffer publishes its own storage, and the
          // store happens whether or not that storage exists. IT RETURNS THAT
          // STORAGE EITHER WAY - the null path is not a `return 0`, which is
          // why the epilogue it jumps to at 0x005E33E5 has no `xor eax, eax`:
          // eax already holds `dib_bits_`, and that IS the zero.
          locked_bits_ = dib_bits_;
          if (dib_bits_ != nullptr) {
              ++surface_lock_count_;
          }
          return dib_bits_;
      }
      if (locked_bits_ != nullptr) {
          ++surface_lock_count_;
          return locked_bits_;
      }
      {
          // THE DESCRIPTOR IS UNINITIALISED APART FROM ITS SIZE, which is what
          // the image does and what DirectDraw asks for: `Lock` fills the rest.
          DDSURFACEDESC description;
          description.dwSize = sizeof(description);
          // THIS failure returns zero where it stands - the image's
          // `xor eax, eax` at 0x005E33C6 is inline - while the `dib_bits_`
          // case above jumps the length of the body to the other zero at the
          // end. Two of them, and which is which is not interchangeable.
          if (surface_->Lock(nullptr, &description, DDLOCK_WAIT, nullptr) != 0) {
              return 0;
          }
          ++surface_lock_count_;
          stride_ = description.lPitch;
          locked_bits_ = description.lpSurface;
          return description.lpSurface;
      }
      return 0;
  }

  // 0x005E34A3, OUT OF LINE in buffer.cpp. The image CALLS this at 43 sites
  // rather than folding it, so an in-class definition - which VC6 inlines
  // everywhere it can - costs every caller the whole body: render_multi_table_dest
  // and render_mono in caviar.cpp were +20 instructions each from it. The
  // definition lives beside its marker in buffer.cpp, like get_data's.
  void free_data(int count);
  // 0x005E33F3. The two-coordinate form over the no-argument one above: the
  // pixel row at (x, y), bounds-checked against `dib_.bmiHeader.biWidth` and
  // the negated top-down `biHeight` the same way. The body lives in
  // buffer.cpp beside its marker; `Sprite::extract` calls it by name five
  // times.
  unsigned char *get_data(int x, int y);

  // MEASURED on both of these: the class stopped being dllexport'ed as a
  // side effect of ~Buffer moving out of line (below), and without the
  // specifier VC6 no longer emits their COMDATs - every caller inlines them,
  // and the two claims in buffer.cpp have nothing to measure.
  MEASURED HDC get_hdc() {
      if (locked_bits_ != 0) {
          get_graphic_win();
      }
      IDirectDrawSurface *const surface = surface_;
      // Without a surface the buffer owns its context directly, so acquiring is
      // just publishing the stored handle and counting the reference.
      if (!surface) {
          hdc2_ = hdc_;
          ++hdc_lock_count_;
          return hdc_;
      }
      if (hdc2_ != nullptr) {
          ++hdc_lock_count_;
          return hdc2_;
      }
      if (surface->GetDC(&hdc2_) != 0) {
          get_graphic_win();
      }
      ++hdc_lock_count_;
      return hdc2_;
  }
  MEASURED void release_hdc(int count) {
      // THE SURFACE TEST COMES FIRST, and the subtraction is inside each arm.
      // The image reads `[esi + 0x58]` before either `sub`, and has two of
      // them - one per arm - where a single subtraction above the branch
      // would need only one.
      if (surface_ == nullptr) {
          hdc_lock_count_ -= count;
          if (hdc_lock_count_ <= 0) {
              hdc2_ = nullptr;
              hdc_lock_count_ = 0;
          }
          return;
      }
      hdc_lock_count_ -= count;
      // `hdc2_` is tested before the count, in that order.
      if (hdc2_ == nullptr) {
          return;
      }
      if (hdc_lock_count_ <= 0) {
          if (surface_->ReleaseDC(hdc2_) != 0) {
              get_graphic_win();
          }
          hdc_lock_count_ = 0;
          hdc2_ = nullptr;
      }
  }
  int set_clip(RECT *rect);
  // 0x005D8200. The catalogue's prototype names the last two `length` and
  // `width`, but they are added to the first two to make right and bottom,
  // so they are a width and a height.
  int set_clip(int left, int top, int width, int height);
  int text_width(LPSTR text);
  int text_width(LPSTR text, int len);
  void hline(int x1, int x2, int y, int color);
  void vline(int x, int y1, int y2, int color);
  int write_raw_l(LPSTR text, int x_coord, int y_coord, int len);
  void set_write_raw_callback(BufferWriteRawCallback callback) {
    field_8_ = callback;
  }
  int write_multi_font_raw_l(LPSTR text, int x_coord, int y_coord, int len);
  int text_height();
  int text_line_height();
  int copy(Buffer *buffer, int xCoord, int yCoord, int width, int height);
  int copy(Buffer *buffer, RECT *rect);
  int box(RECT *rect, int color1, int color2);
  // 0x005D8840 - `?draw_mono@Buffer@@QAEHPAVBuffer@@HHHH@Z`, the fixed-size
  // member of the draw_mono family: target, transparency, position and
  // colour, with the scale denominators the seven-argument form takes left
  // at the Sprite defaults.
  int draw_mono(Buffer *target, int transparent, int x, int y, int colour);
  // 0x005DA6C0 - `?tile_mask@Buffer@@QAEHPAVBuffer@@HHHHHHH@Z`. The same
  // rectangle tiled through a mask colour: `box_sprite` paints every frame
  // element twice, once as the sprite and once as its shadow.
  int tile_mask(Buffer *mask, int src_x, int src_y, int x, int y, int w, int h,
                int color);
  // 0x005E2B00 - `?copy_mask@Buffer@@QAEHPAVBuffer@@HHHHHHH@Z`. `tile_mask`'s
  // per-tile blit: one source rectangle from this buffer drawn into `mask`,
  // replacing every pixel already holding the mask colour.
  int copy_mask(Buffer *mask, int src_x, int src_y, int x, int y, int w, int h,
                int color);
  // ?box_sprite@Buffer@@QAEHPAURECT@@PAUBoxSpriteParams@@@Z (0x005E2DD3),
  // the image's universal panel drawer. The second argument is one of the
  // image's own `BoxSpriteParams` blocks in .data - DipEdit's frame and title
  // pass 0x0078D690 and 0x0078D528, ReplayWin::on_redraw passes 0x0078D690
  // and 0x0078D618.
  int box_sprite(RECT *rect, BoxSpriteParams *params);
  // THE ADAPTER THE RAW-ADDRESS CALLERS STILL NEED. `DipEdit::init` passes
  int write_l(LPSTR text, int x_coord, int y_coord, int len);
  int write_l(LPSTR text, RECT *rect, int len);
  int write_cent_l(LPSTR text, int x_coord, int y_coord, int width, int len);
  int write_cent_l(LPSTR text, RECT *rect, int len);
  int write_cent_l(LPSTR text, RECT *rect);
  void close();
  // Surface setup and image blits. Declared so the recovered window
  // initialization bodies (Win::init_class and the per-control init_class
  // functions) can call them; their own bodies are still pending_bodies
  // forwarders. The seven-argument `copy` is a distinct overload from the
  // four-argument one above.
  int init(int width, int height, int tgl, ExtDirectDraw *direct_draw);
  // 0x005D9E70 - blit a source buffer's region at (x, y). Declared here for
  // the same reason as the rest of this block: the homed Win bodies call it,
  // and its own body is still an artifact. `Win::redraw_nc_buffer` and the
  // caption-button painters are the callers.
  // Declared 2026-08-25 for get_mouse_window_recurse, homed into win.cpp:
  // its pixel-precise hit test reads one pixel from each of a window's
  // four render buffers and compares it against the transparent key.
  // The body is not recovered yet - this is the declaration only.
  int get_pixel(int x, int y);

  int draw(Buffer *destination, int index, int x, int y, int numerator, int denominator);
  int draw_mono(Buffer *destination, int transparent, int x, int y,
                int colour, int numerator, int denominator);
  // __STDCALL, like load_tga: the image's name is QAG, not QAE, so the
  // loader pair was declared with the explicit convention.
  int __stdcall load_bmp(const char *filename, Palette *palette, int a3,
                         int a4);
  // 0x005D7C80 - `?load_bmp@Buffer@@QAGHPBD@Z`, the one-argument overload:
  // the same forwarder shape as the one-argument load_tga above.
  int __stdcall load_bmp(const char *filename);
  // OVERLOAD BLOCK, homed from the archive in the buffer homing pass. The
  // catalogue's own prototypes; each body lives beside its marker in
  // buffer.cpp. The five-int `draw` above is the six-argument form the
  // sprite-sheet callers use.
  int draw(Buffer *source, int a, int b, int c);
  // Original 5D8650/5D8FE0/5D91D0: shared-Sprite publishing draw siblings.
  int draw_unk2(Buffer *destination, int transparent, int x, int y, int table);
  int draw(Buffer *destination, int transparent, int x, int y,
           unsigned char *table);
  int draw_dest_unk2(Buffer *destination, int transparent, int x, int y,
                     unsigned char *table);
  int draw_dest(Buffer *source, int a, int b, int c, int d);
  int draw_dest(Buffer *source, int a, int b, int c, int d, int e,
                unsigned char *bits);
  int draw_dest_source(Buffer *source, int a, int b, int c,
                       unsigned char *bits);
  int draw_dest_source(Buffer *source, int a, int b, int c, int d, int e,
                       unsigned char *bits);
  int draw_multi_table_dest(Buffer *source, int a, int b, int c, int d);
  int draw_multi_table_dest(Buffer *source, int a, int b, int c, int d, int e,
                            unsigned char *bits);
  int copy(Buffer *source, int src_x, int src_y, int src_w, int src_h,
           int dest_x, int dest_y, int dest_w, int dest_h);
  // Coordinate bounds are exclusive. The RECT overload additionally
  // subtracts one from right/bottom, as in the original remapping path.
  int map_colors(int left, int top, int right, int bottom, unsigned char *table);
  int map_colors(RECT *area, unsigned char *table);
  int write_strings_height(StringList *strings, int a2, int a3);
  // (left, top, width, height) from the two recovered call sites, which pass
  // `left, top, right - left, 0` - the writer lays a StringList out from that
  // origin and clips on the height.
  int write_strings(StringList *strings, int left, int top, int width,
                    int height);
  int box(int left, int top, int right, int bottom, int color1, int color2);
  int dotted_box(int left, int top, int right, int bottom, int color);
  int dotted_box(RECT *area, int color);
  void dotted_hline(int x1, int x2, int y, int color);
  void dotted_vline(int x, int y1, int y2, int color);
  void line(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8,
           int a9);
  void line(int x1, int y1, int x2, int y2, int color);
  int fill_line(int x1, int y1, int x2, int y2, char fill_colour);
  // THE PATTERNED RUN PAIR, 0x005E10C0 and 0x005E1350
  // (`?hline@Buffer@@QAEXHHHHHHHH@Z` / `?vline@Buffer@@QAEXHHHHHHHH@Z`): the
  // clipped writers above with a dash pattern - two colours, an on length,
  // an off length and a phase - beside the nine-argument `line` that names
  // the same five pattern words. VOID in the image (QAEX); the old `int`
  // spelled a symbol nothing holds.
  void hline(int x1, int x2, int y, int first_color, int second_color,
             int pattern_on, int pattern_off, int phase);
  void vline(int x, int y1, int y2, int first_color, int second_color,
             int pattern_on, int pattern_off, int phase);
  int put_pixel(int x, int y, int color);
  int change_color(int a1, int a2);
  int change_color(RECT *area, int a2, int a3);
  int change_color(int a1, int a2, int a3, int a4, int a5, int a6);
  static int get_pcx_dimensions(const char *filename, int *width, int *height);
  int write_pcx(char *filename);
  int write_bmp(char *filename);
  char *find_line_break_l(char *text, int *break_len, int len);
  int write_l(Font *font, char *text, int x_coord, int y_coord, int len);
  int write_l(Font *font, char *text, RECT *area, int len);
  int wrap_cent(Font *font, char *text, RECT *area);
  int write_cent_l(Font *font, char *text, int x_coord, int y_coord, int width,
                   int len);
  int write_cent_l(Font *font, char *text, int x_coord, int y_coord, int len);
  int write_cent_l(Font *font, char *text, RECT *area, int len);
  int write_cent_l(Font *font, char *text, int x_coord, int y_coord, int width,
                   int height, int len);
  int write_cent_l(char *text, int x_coord, int y_coord, int width, int height, int len);
  int write_cent_l(char *text, int x_coord, int y_coord, int width);
  int write_right_l(Font *font, char *text, int x_coord, int y_coord,
                    int width, int len);
  int write_right_l(Font *font, char *text, int x_coord, int y_coord, int len);
  // 0x005DB930 - the RECT member of the Font-first family: same temporary
  // font-slot override around the four-argument RECT writer below.
  int write_right_l(Font *font, char *text, RECT *area, int len);
  int write_right_l(char *text, int x_coord, int y_coord, int width);
  int wrap(Font *font, char *text, RECT *area);
  int wrap(char *text, RECT *area);
  int wrap(char *text, int x_coord, int y_coord, int width);
  int wrap_cent(Font *font, char *text, int x_coord, int y_coord, int width);
  int wrap_cent(Font *font, char *text, int x_coord, int y_coord, int width,
                int line_step);
  int wrap_cent(char *text, RECT *area);
  int wrap_cent(char *text, int x_coord, int y_coord, int width);
  int wrap_cent(char *text, int x_coord, int y_coord, int width,
                int line_step);
  int wrap_right(Font *font, char *text, int x_coord, int y_coord, int width);
  int wrap_right(Font *font, char *text, int len);
  int wrap_right(Font *font, char *text, int x_coord, int len);
  int wrap_right(char *text, int x_coord, int y_coord, int width);
  int wrap_right(char *text, int x_coord);
  int wrap_height(char *text, int width);
  int wrap_height_flying(char *text, int width);
  int wrap_height_flying(char *text);
  int wrap_flying(char *text, int x_coord, int y_coord, int width);
  int wrap_flying(char *text);
  int wrap_flying_flush();
  int wrap_height_flying_flush();
  // 0x005DA860 - tile `source` across a destination rectangle, `tile_mask`
  // without the mask. The dimensions stepped by are the source's own.
  int tile(Buffer *source, int src_x, int src_y, int x, int y, int w, int h);
  // 0x005DA9F0 - `?tile@Buffer@@QAEHPAVSprite@@HHHHHH@Z`, the SPRITE overload
  // of the tile above: `box_sprite` tiles the frame's four edge strips from
  // sprites rather than from a buffer.
  int tile(Sprite *sprite, int src_x, int src_y, int x, int y, int w, int h);
  int fill(int color);
  int fill(RECT *area, int color);  // 005DFCD0
  // 0x005D8240 - builds a RECT from the four coordinates and forwards to
  // the RECT overload above.
  int fill(int left, int top, int width, int height, int color);
  int load_pcx(const char *filename, Palette *palette, int tgl, int height);
  // 0x005E2690, the actual PCX decoder: the by-name overload above maps the
  // file and hands over the bytes and their length.
  // 0x005E2690. The last two are a colour base and a colour count - the
  // by-name overload calls them `tgl` and `height` and passes them straight
  // through, but this one adds the first to every decoded pixel and copies
  // that many palette entries.
  int load_pcx(BYTE *data, DWORD size, Palette *palette, int colour_base,
               int colour_count);
  // 0x005DFF00 - THE BLITTER, IN-CLASS since 2026-09-17 (the body lived out
  // of line in buffer.cpp; the ORIGINAL marker and its full history stay
  // there beside `// body src/buffer.h`, like get_data's). The image's own
  // copy was header-inline, and its flic unit was U-keyed (init is
  // catalogued `PAUBuffer` there): that unit emitted its own PAU COMDAT
  // alongside this PAV one and the linker ICF-folded them - the only way
  // one image can hold both spellings of the name. This tree keys every
  // unit V through this `class Buffer` declaration, so the one PAV COMDAT
  // spelling serves all callers; flic.cpp records its PAV emissions with
  // `// symbol` facts instead of re-keying the unit.
  MEASURED int copy(Buffer *buffer, int xCoord, int yCoord, int wx, int wy,
                    int width, int height) {
      if (dib_bits_ == nullptr && surface_ == nullptr) {
          return 7;
      }
      if (buffer == nullptr) {
          return 3;
      }

      // A negative read position shrinks the extent and shifts the write
      // position by the overhang, then clamps: the standard blit clip.
      if (xCoord < 0) {
          width += xCoord;
          wx -= xCoord;
          xCoord = 0;
      }
      if (yCoord < 0) {
          height += yCoord;
          wy -= yCoord;
          yCoord = 0;
      }

      // `biHeight` is stored negative, so `-biHeight` is the row count.
      if (xCoord > dib_.bmiHeader.biWidth
          || yCoord > -dib_.bmiHeader.biHeight) {
          return 0;
      }
      if (xCoord + width > dib_.bmiHeader.biWidth) {
          width = dib_.bmiHeader.biWidth - xCoord;
      }
      if (yCoord + height > -dib_.bmiHeader.biHeight) {
          height = -(yCoord + dib_.bmiHeader.biHeight);
      }

      // The write rectangle, clipped against the destination's own clip rect.
      RECT clipped;
      clipped.left = wx;
      clipped.top = wy;
      clipped.right = wx + width;
      clipped.bottom = wy + height;
      if (!IntersectRect(&clipped, &clipped, &buffer->rect1_)) {
          return 0;
      }

      // Whatever the intersection moved the write rect by, the read position
      // moves with it; the extent comes from the intersected rect.
      xCoord += clipped.left - wx;
      yCoord += clipped.top - wy;
      width = clipped.right - clipped.left;
      height = clipped.bottom - clipped.top;

      if (clipped.left >= buffer->dib_.bmiHeader.biWidth
          || clipped.top >= -buffer->dib_.bmiHeader.biHeight
          || buffer->get_data() == 0) {
          return 3;
      }
      uint8_t *const write = static_cast<uint8_t *>(buffer->locked_bits_)
          + buffer->stride_ * clipped.top + clipped.left;
      if (write == nullptr) {
          return 3;
      }

      if (xCoord >= dib_.bmiHeader.biWidth
          || yCoord >= -dib_.bmiHeader.biHeight
          || get_data() == 0) {
          buffer->free_data(1);
          return 3;
      }
      uint8_t *const read = static_cast<uint8_t *>(locked_bits_)
          + stride_ * yCoord + xCoord;
      if (read == nullptr) {
          buffer->free_data(1);
          return 3;
      }

      if (buffer != this) {
          // Different objects cannot overlap, so rows go in either order.
          uint8_t *destination = write;
          const uint8_t *source = read;
          for (int row = 0; row < height; ++row) {
              memcpy(destination, source, static_cast<size_t>(width));
              source += stride_;
              destination += buffer->stride_;
          }
      } else {
          // SAME BUFFER, so the two rectangles can overlap. Rows are walked
          // from whichever end keeps the read ahead of the write, and each row
          // is moved rather than copied because the horizontal shift can
          // overlap too.
          const int step = (wy < yCoord) ? 1 : -1;
          const int first = (wy < yCoord) ? 0 : height - 1;
          uint8_t *destination = write + first * static_cast<int>(buffer->stride_);
          uint8_t *source = read + first * static_cast<int>(stride_);
          for (int row = 0; row < height; ++row) {
              memmove(destination, source, static_cast<size_t>(width));
              destination += step * static_cast<int>(buffer->stride_);
              source += step * static_cast<int>(stride_);
          }
      }

      free_data(1);
      buffer->free_data(1);
      return 0;
  }

 // PROTECTED, NOT PRIVATE: GraphicWin derives from Buffer and the homed
 // window bodies read these directly - which is what the image does, since
 // the surface subobject and the window are one object (Win already grants
 // itself friendship for the same reason). Access to a member affects no
 // offset and no emitted byte, so the layout and every claim are untouched.
 protected:
  // The free GraphicWin caption painter passes this live clip back to set_clip.
  friend int __cdecl def_caption_draw(GraphicWin *, char *, Buffer *);
  friend class CheckBox;
  // EditBox's redraw uses the Buffer text foreground at original +0x53C.
  friend class EditBox;
  // `Win::flip` clips the screen buffer back to its own extent and reads
  // the DIB's dimensions for the final BitBlt.
  friend class Win;
  // Shared display init/teardown own ScreenBuffer's borrowed COM handles.
  friend int __cdecl sub_62cd50(int bit_depth);
  friend int __cdecl sub_62d100();

  // Caviar::init_class reads the two voxel scene surfaces' `dib_bits_` into
  // its render record - the image reads BufferA/BufferB + 0x54 by fixed
  // address, and the object spelling names the field. Access control is
  // compile-time only, so no offset and no emitted byte moves.
  friend class Caviar;
  // Sprite rasterization uses the destination's clip and acquired pixel data.
  // Cursor initialization borrows the color DIB palette.
  friend class Cursor;
  friend class Sprite;
  // Texture::draw_trans reads the destination stride at 0x0061A2BB and
  // rect1_ at 0x0061A2DA..0x0061A2F9. Its get_data call at 0x0061A253
  // acquires the pixels published in locked_bits_ for the rasterizer.
  friend class Texture;
  // The free polygon family acquires this Buffer before reading its clip
  // and pitch (0x00626648 / 0x00626E38), then releases the same acquisition.
  friend int __cdecl polygon(Buffer *, Vert *, int, int);
  friend int __cdecl polygon_dest(Buffer *, Vert *, int, unsigned char *);
  friend int __cdecl polygon_gouraud(Buffer *, Vert *, int *);

  // 0x4. The window the surface belongs to - `GraphicWin::init` stores its
  // `this` here before the minimum-size computation, and nothing recovered
  // reads it. `void *` until a reader proves which base pointer the image
  // stores; it was `uint32_t`, which needed a cast at the one store.
  void *poOwner_;
  BufferWriteRawCallback field_8_;  // 0x0008
  BufferWriteStringsCallback field_C_;
  uint32_t field_10_;
  uint32_t field_14_;
  uint32_t field_18_;
  // 0x1C. The `tgl` argument `Buffer::init` was given, stored as its last
  // act; `Buffer::close` tests bit 2 of it - the same `tgl & 4` that made
  // `init` take the surface and clipper from the caller - to decide whether
  // the device context is the buffer's to destroy.
  uint32_t init_flags_;
  RECT rect1_;
  RECT rect2_;
  uint32_t field_40_[4];
  // 0x50. The mapped pixels while the surface is locked: `Buffer::fill`
  // sets it from `dib_bits_` when there is no surface and from
  // `DDSURFACEDESC::lpSurface` when there is, and clears it when the last
  // lock goes.
  LPVOID locked_bits_;
  // 0x54. THE BITS, not a pointer to them. `Buffer::init` passes `&`
  // this to `CreateDIBSection` - `lea edx, [esi + 0x54]` at 0x005D777A -
  // and `Buffer::fill` reads it as the destination base. Declared
  // `LPVOID *` it compiled either way, because `LPVOID *` and the `VOID **`
  // that CreateDIBSection wants are the same type.
  LPVOID dib_bits_;
  // 0x58 and 0x5C. The DirectDraw pair `Buffer::init` creates: slot 4 of
  // IDirectDraw is CreateClipper and stores into 0x5C, and the surface it
  // creates is QueryInterface'd into 0x58. `Buffer::get_hdc` then calls
  // slot 0x44 - IDirectDrawSurface::GetDC - on 0x58 and had to cast it back
  // out of a `uint32_t` to do it.
  // RECOVERY_MEMBER: v3 Buffer surface_ IDirectDrawSurface* 0x58 0x0062CD50
  IDirectDrawSurface *surface_;
  // RECOVERY_MEMBER: v3 Buffer clipper_ IDirectDrawClipper* 0x5C 0x0062CD50
  IDirectDrawClipper *clipper_;
  HDC hdc2_;
  HDC hdc_;
  // 0x68. How many holders the device context at `hdc2_` has: `init`,
  // `get_hdc` and `release_hdc` all raise it before GetDC and lower it
  // before ReleaseDC, and zero it when the last one goes.
  int hdc_lock_count_;
  // 0x6C. How many holders that lock has - raised beside `locked_bits_`
  // and lowered with it, exactly as `hdc_lock_count_` tracks `hdc2_`.
  int surface_lock_count_;
  // 0x70. An HRGN the buffer owns: `Buffer::close` is the only thing that
  // touches it here, deleting it beside the bitmap. `Buffer::set_clip` is
  // the obvious producer but nothing recovered yet proves it.
  HRGN clip_region_;
  // 0x74. What `SelectObject` returned when `Buffer::init` selected the DIB
  // into `hdc_` - that is, whatever was in the context before - and
  // `Buffer::close` selects it back before `DeleteDC`. The GDI save/restore
  // pair, written out.
  // `HGDIOBJ`, which is what both ends of its life say it is: `SelectObject`
  // returns one into it and `SelectObject` takes it back. It was `uint32_t`
  // with a `reinterpret_cast` at each end, and the casts were the type
  // trying to be declared.
  HGDIOBJ previous_bitmap_;
  HBITMAP bitmap_handle_;

 public:
  /*
   * 0x7C. THE DIB DESCRIPTION ITSELF, and PUBLIC because the original
   * reached it from outside: `Win::init_class` centres the splash bitmap on
   * `ScreenBuffer`'s width and the logo's own. `Buffer::init` hands it to
   * `CreateDIBSection` as `lea eax, [esi + 0x7c]` - the address
   * of a member, not a load - and the constructor fills it field by field,
   * `biSize = 40`, `biPlanes = 1`, `biBitCount = 8`, `biClrUsed = 256`. An
   * 8-bit palettised DIB, which is why the 256 colours follow inline.
   *
   * `BITMAPINFO` is declared by Windows with a one-entry colour table, so
   * the 256-entry form always needs its own struct; this is that struct.
   * Header plus table is 40 + 1024 = 0x428 bytes, and 0x7C + 0x428 is
   * exactly `palette_seed_`, so the block is accounted for to the byte.
   *
   * `biHeight` IS STORED NEGATIVE, by `Buffer::init` and by `set_dib_bits`.
   * That is not a quirk of this engine - a negative `biHeight` is how a DIB
   * declares itself top-down, rows running downwards from the first byte,
   * which is the order a blitter wants. It is why `init`'s early-out reads
   * `height == -dib_.bmiHeader.biHeight`.
   */
  Dib dib_;

 private:
  /*
   * 0x4A4. THE PALETTE GENERATION WHOSE COLOURS ARE IN `dib_.bmiColors`,
   * and it sits immediately after the table it guards.
   *
   * `Palette::init` picks `seed_` at random and loops until it is non-zero,
   * so a palette that has been initialised never carries zero and zero is
   * free to mean "nothing cached". The constructor and `close` store zero,
   * which is why the first publish after either always happens.
   *
   * SIX FUNCTIONS CONSULT IT, all with the same five lines: compare against
   * the live palette's `seed_`, and on a difference store it, refill the
   * 256 entries through `Palette::get_rgbquad` and push them into the
   * device context with `SetDIBColorTable`. `Buffer::sync_to_palette` is
   * that code written out; `copy`, `copy_to_window`, `load_pcx` and two
   * bodies still unrecovered at 0x005D98F0 and 0x005D9FB0 have it inlined,
   * twice each where they touch two buffers. Reading any one of them alone
   * would suggest a private cache; reading all six is what makes it a
   * generation tag.
   */
  uint32_t palette_seed_;
  // PUBLIC, not private: `Sprite::extract` reads it straight off the object -
  // `mov edx, [buffer + 0x4A8]` at 0x005E3AB3 - the way callers of the
  // drawing primitives read `dib_` above. Same offset; only the access run
  // changes, and access never moves a member.
 public:
  // 0x4A8. `Buffer::init` sets it to `(width + 3) & ~3`: the row pitch a
  // DIB needs, the width rounded up to a DWORD.
  uint32_t stride_;
 private:
  /*
   * 0x4AC. HOW MANY OF `owned_` ARE IN USE, and it is ONE-BASED:
   * `write_multi_font_raw_l` increments it first and then stores through
   * `[this + count * 4 + 0x4B8]`, which is `owned_[count - 1]`. Every read
   * of it is bounded against 20 - `cmp [this + 0x4ac], 0x14; jge` - which
   * is `OwnedAllocationCount`, so the two really are the same table.
   * `clear_links` zeroes it beside freeing all twenty.
   */
  uint32_t link_count_;
  Spot spot_;
  // 0x4BC. TWENTY OWNED HEAP POINTERS - `Buffer::init` and `Buffer::close`
  // both walk `for (20) { if (*p) { free(*p); *p = 0; } p += 4; }` from
  // 0x4BC, and the walk ends at 0x50C, exactly where `field_50C_` starts.
  //
  // THEY HOLD LINK TEXT. `write_multi_font_raw_l` is the writer: for each
  // link it allocates `end - start + 1` bytes, stores the pointer at
  // `owned_[link_count_ - 1]`, and copies the substring in. That is also
  // what makes `clear_links` a pair of operations rather than two unrelated
  // ones - it reinitialises `spot_` to forty hit regions and frees these
  // twenty strings, which are the two halves of one link table. They are
  // still only ever reached through a walking pointer, never named
  // individually, so
  // the name says the one thing that is established: the buffer owns them
  // and frees them.
  void *owned_[20];
  uint32_t field_50C_;
  // 0x510. The WRITER's font slot, the counterpart of `font_slot_` for
  // `write_multi_font_raw_l`: same four values, kept apart because measuring
  // and drawing walk the same string independently.
  uint32_t write_font_slot_;
  uint32_t field_514_;
  // 0x518. WHICH OF THE FOUR FONTS THE NEXT SEGMENT IS MEASURED IN, and it
  // is an INDEX rather than a pointer: `text_width` reads
  // `[edi + eax*4 + 0x52c]`, which is `(&font1_)[font_slot_]`. `{` selects 1,
  // `[` selects 2, `$LINK<` selects 3, and `}`/`]` return to 0.
  uint32_t font_slot_;
  // 0x51C. The markup token still being consumed: 1 while a `$LINK<` header
  // is being stepped over - five more bytes - and 2 while an `=` span runs
  // to its `>`. Zero otherwise, and cleared when the string ends.
  uint32_t markup_pending_;
  // 0x520. Whether the text is parsed for markup at all. Zero takes
  // `text_width` straight to `font1_->width(text, len)` with no scan.
  uint32_t markup_enabled_;
  uint32_t field_524_;
  uint32_t field_528_;
  Font *font1_;
  Font *font2_;
  Font *font3_;
  Font *font4_;
  uint32_t color_val_1_;
  uint32_t color_2_val_1_;
  uint32_t color_3_val_1_;
  uint32_t color_hyper_val_1_;
  uint32_t color_val_2_;
  uint32_t color_2_val_2_;
  uint32_t color_3_val_2_;
  uint32_t color_hyper_val_2_;
  uint32_t color_val_3_;
  uint32_t color_2_val_3_;
  uint32_t color_3_val_3_;
  uint32_t color_hyper_val_3_;
  uint32_t color_val_4_;
  uint32_t color_2_val_4_;
  uint32_t color_3_val_4_;
  uint32_t color_hyper_val_4_;
  /*
   * 0x57C. A PALETTE HAS BEEN ATTACHED. Set to 1 at all eight sites that
   * publish one - `sync_to_palette`, both halves of `copy`, `copy_to_window`,
   * `load_pcx` and the two bodies still unrecovered at 0x005D98F0 and
   * 0x005D9FB0 - and cleared to 0 by the constructor and `close`, always in
   * the instruction immediately before `palette_` is written beside it.
   *
   * SET ON BOTH PATHS, which is what rules out "the colour table is stale":
   * the branch that skips the republish because `palette_seed_` still
   * matches lands directly on this store.
   *
   * NOTHING IN THE IMAGE READS IT. All thirteen references to a +0x57C
   * displacement in `.text` are stores, ten of them Buffer's and three
   * belonging to other classes, and none of them is SIB-based. It carries
   * exactly what `palette_ != nullptr` already carries, so it is a flag the
   * shipped build writes and never asks about.
   */
  uint32_t has_palette_;
  int8_t field_580_;
  // 0x584, and it is the alignment after `int8_t field_580_` that puts it
  // there rather than at 0x581 - which is also what makes
  // `sizeof(Buffer) == 0x588` come out right.
  /*
   * 0x584. THE PALETTE THIS BUFFER LAST SYNCED TO, and a `Palette *` by
   * dereference rather than by what is stored into it: `copy_to_window`
   * reads it, null-checks it, and then reads `[it + 0x400]` - which is
   * `Palette::seed_`, since `entries_[256]` occupies the 0x400 bytes before
   * it. Every other reference is a store, of `PaletteActive` in the copy
   * paths and of the argument in `sync_to_palette`, and stores alone could
   * not have told a pointer from a tag.
   *
   * It pairs with `palette_seed_`: this is WHICH palette, that is WHICH
   * GENERATION of it, and a publish is skipped only when both still agree.
   */
  Palette *palette_;
};

static_assert(sizeof(Buffer) == 0x588, "Buffer layout must match the original executable");

// 0x00696C58 / 0x00696C60 / 0x00696C68, and 0x00696C70 / 0x00696C78 behind
// them - the per-language graphics-file extensions `load_tga` and `load_bmp`
// append to a stem the caller gave with none. Language 1 gets `.tgf`,
// language 2 `.tgg`, anything else `.tga`; the same table's second row is the
// bitmap pair. Read straight out of the image's .rdata: five NUL-padded
// four-character strings in a row.
extern const char BufferExtTgf[];
extern const char BufferExtTgg[];
extern const char BufferExtTga[];
extern const char BufferExtBmf[];
extern const char BufferExtBmg[];
extern const char BufferExtBmp[];

// Solid and destination-remapping polygon rasterizers, owned by polygon.cpp.
// The original remapping loop indexes the caller's live 256-byte table with
// each unsigned destination pixel; it does not copy or null-check that table.
int __cdecl polygon(Buffer *buffer, Vert *verts, int count, int color);
int __cdecl polygon_dest(Buffer *buffer, Vert *verts, int count,
                         unsigned char *colors);
// Gouraud's original entry always consumes four vertices and four lights.
int __cdecl polygon_gouraud(Buffer *buffer, Vert *verts, int *lights);

// DirectDraw surface vtable slots the data lock/release pair dispatches
// through; both are stdcall COM methods on the surface interface.
static const size_t BufferSurfaceLockSlot = 0x64;
static const size_t BufferSurfaceUnlockSlot = 0x80;


// A scratch Buffer the PCX reader builds a decoded frame into before it is
// blitted onward. Defined in buffer.cpp (0x00798668); declared here so the
// maininterface body that reads it back can name it instead of binding the
// address. Wave-3 handoff, 2026-09-05.
extern Buffer g_PCX_PARSE_TEMP_BUFFER1;  // 0x00798668
extern Buffer g_IFACE_BOX_SPRITE_BUFFERS[51];
extern Buffer g_IFACE_STD_POPUPS_MIDDLE_BUFFER;
extern Buffer g_VEHDRAW_BUFFER;  // 0x008CC298



// Selects the DirectDraw teardown path over the GDI device-context path.
// 0x009BC494. Non-zero while DirectDraw owns the screen, so the GDI paths
// stand down. AN OBJECT: `Palette::set` opens with
// `mov eax, dword ptr [0x9bc494]` / `test eax, eax`, a direct load of the
// VALUE. Through `extern int *` the same source compiles to a load of the
// pointer and then `cmp dword ptr [eax], 0` - one instruction more, and the
// first divergence in that body.
// 0x009BC494. AN IDirectDraw, not a flag. Every test of it is `!= 0`,
// which is what made `int` survive - but `Buffer::init` does
// `mov ecx, [eax]` and `call [ecx + 0x18]` on it, which is slot 6,
// IDirectDraw::CreateSurface. It also calls slot 4, CreateClipper.
extern IDirectDraw2 *BufferDirectDraw;

// Shared file-backed sprite staging surface (original 0x009B6080).
extern Buffer g_BUFFER;

// Value the close reset writes at offset 0x520; its meaning is unconfirmed.
extern uint32_t BufferField520Default;
// Releases a Sprite-style allocation through the executable's own CRT.


void __cdecl buffer_close_class_redirect();






// The four-argument line primitives at 0x005E1A80 (hline) and 0x005E1BF0
// (vline) are clipped pixel writers, still original dependencies. The
// recovered box outline reaches them through these seams; tests rebind them.
// Both are thiscall voids (ret 0x10) whose EAX residue the box body
// deliberately discards - the original zeroes EAX after the last call
// (`xor eax, eax` at 0x005E327A).


// The measured overload this one wraps is a 578-byte body with three call
// targets, still an original dependency. Tests rebind this seam.

// The 835-byte multi-font raster writer at 0x005DCAE0 is the real glyph
// emitter behind every length-limited text entry point, and is still an
// original dependency. All four recovered writers reach it through this seam;
// the leaf tests rebind it to a recorder. Its EAX passes straight through -
// the original ends `mov eax, edi` at 0x005DCE18, handing back the advanced
// pen position, which is why the scalar writers return the incoming x when
// they emit nothing.

extern char BufferFlyingText[];
extern int BufferFlyingX,BufferFlyingY,BufferFlyingWidth;
extern int BufferStrHeight;  // 0x009B3A9C, shared running text-width accumulator
extern int BufferLastWrapWriteResult;  // 0x009B39D8, shared line endpoint/fallback result
extern const char BufferJoinSpace[],BufferMeasureSpace[],BufferWriteSpace[],BufferCenteredSpace[];
char *__cdecl get_special(char *, int *);
char *__cdecl get_special_link(char *, int *);
