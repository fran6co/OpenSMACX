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

class Palette;

// A polygon vertex, named by the image rather than invented: every one of the
// 35 mangled names that mentions it spells `PAUVert@@`, and `U` is MSVC's tag
// for a struct. It has to be `struct` for that reason and not as a style
// choice - declaring `class Vert;` would mangle `Buffer::poly` as `PAVVert@@`
// and match no symbol the target objects hold. Nothing defines it yet, so the
// members are unknown; the declaration exists so headers that only pass a
// `Vert *` through can compile.
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
// The arithmetic above, stated so the COMPILER checks it and so
// tools/header_offsets.py can walk THROUGH a `Dib` member instead of
// stopping there and reporting a layout it never measured.
static_assert(sizeof(Dib) == 0x428, "Dib is BITMAPINFOHEADER + 256 RGBQUAD");

class Buffer {
 public:
  // Blit this buffer into a window's client area. Called by the homed
  // Win painters; its own definition is still an artifact. Arity from
  // the call site.
  void copy_to_window(Win *target, int src_x, int src_y, int x, int y,
                      int w, int h);
  // homed from 005d8290.cpp
  // -1 means "the fill colour": the body substitutes WinFillColour.
  void setup_buff_sprite(int colour);

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
  // THE TWO TRAILING INTS ARE EVIDENCE-FREE, measured 2026-08-25 rather
  // than left unfinished. `poly` forwards straight to `polygon`, `polygon`
  // forwards to 0x00626620, and that function is 736 bytes and UNDECODED -
  // buffer.cpp's own note says so. Nothing in the tree calls either with a
  // literal that would name them, so the only honest spelling is the
  // scaffold one. Naming them from the method's name alone would be a guess
  // dressed as a fact.
  int poly(Vert *verts, int a2, int a3);
  // 0x005D7210. The body is in buffer.cpp beside its marker.
  Buffer();
  // VIRTUAL, AND THE VPTR MEMBER IS GONE. What the image stores at offset
  // 0 is a C++ vtable, and this tree used to spell it as an explicit
  // `LPVOID vtable_` written by hand. The slots are `??_GBuffer@@UAEPAXI@Z`
  // at 0 and `sub_406b30` at 1; declared virtual, the compiler puts the
  // pointer there itself and `sizeof` stays 0x588.
  //
  // 0x005D7410, and the body really is this: see the annotation in
  // buffer.cpp beside the marker.
  // VIRTUAL, and not negotiable: the image's Buffer-side vftable
  // (0x0066FC48, the one ??1GraphicWin stores at +0x444) opens with a
  // deleting-destructor THUNK (0x005D7160) - a secondary-table thunk only
  // exists when the destructor is virtual. A 2026-08-30 experiment spelled
  // it non-virtual while chasing ??1Console's member-teardown shape; that
  // changed nothing there and would have emptied this slot.
  MEASURED virtual ~Buffer() { close(); }
  // `int`, and returning 0, because that is what the three bytes are:
  // `xor eax, eax; ret`. Declared `void` it compiles to `ret` alone and
  // Buffer's slot 1 stops being the function the image put there.
  virtual int surface_lost();

  // Slot 1 of this class's vtable.
  //
  // MEASURED, 2026-08-16. Declaring it `virtual` and deleting the explicit
  // `LPVOID vtable_;` below is the right shape and is what the image has -
  // the constructor's store to offset 0 is a C++ vtable spelled as data.
  // It compiles, it links, `sizeof` stays
  // 0x588, and it changes not one byte of `Buffer::init` or
  // `Buffer::close`: 90.8% and 95.7% either way. It also breaks 87 claims,
  // because `emit_translation_unit` emits its class shells in an order that
  // a polymorphic Buffer defeats - `C2504: 'GraphicWin' : base class
  // undefined` on the first three checked. The blocker is the SCAFFOLD, not
  // the class, and it is the same blocker `Win` has.


  void clear_links();
  int set_font(Font *font1, Font *font2, Font *font3, Font *font4);
  void set_text_color(int color1, int color2, int color3, int color4);
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
  MEASURED int get_data() {   // 005E3373
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
          return reinterpret_cast<int>(dib_bits_);
      }
      if (locked_bits_ != nullptr) {
          ++surface_lock_count_;
          return reinterpret_cast<int>(locked_bits_);
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
          return reinterpret_cast<int>(description.lpSurface);
      }
      return 0;
  }

  MEASURED void free_data(int count) {   // 005E34A3
      // THE SURFACE TEST COMES FIRST and the subtraction is written out in
      // both arms - `cmp ecx, edi; jne` at 0x005E34AE before any arithmetic.
      // Hoisted above the test it lands two instructions early and every jump
      // after it moves, which is the same shape `Buffer::close` needed.
      if (surface_ == nullptr) {
          surface_lock_count_ -= count;
          if (surface_lock_count_ <= 0) {
              locked_bits_ = nullptr;
              surface_lock_count_ = 0;
          }
          return;
      }
      surface_lock_count_ -= count;
      if (locked_bits_ != nullptr && surface_lock_count_ <= 0) {
          surface_->Unlock(locked_bits_);
          locked_bits_ = nullptr;
          surface_lock_count_ = 0;
      }
  }
  // 0x005E33F3. The two-coordinate form over the no-argument one above: the
  // pixel row at (x, y), bounds-checked against `dib_.bmiHeader.biWidth` and
  // the negated top-down `biHeight` the same way. The body lives in
  // buffer.cpp beside its marker; `Sprite::extract` calls it by name five
  // times.
  int get_data(int x, int y);

  HDC get_hdc() {
      if (locked_bits_ != 0) {
          surface_lost();
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
          surface_lost();
      }
      ++hdc_lock_count_;
      return hdc2_;
  }
  void release_hdc(int count) {
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
              surface_lost();
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
  int write_multi_font_raw_l(LPSTR text, int x_coord, int y_coord, int len);
  int text_height();
  int text_line_height();
  int copy(Buffer *buffer, int xCoord, int yCoord, int width, int height);
  int copy(Buffer *buffer, RECT *rect);
  int box(RECT *rect, int color1, int color2);
  int write_l(LPSTR text, int x_coord, int y_coord, int len);
  int write_l(LPSTR text, RECT *rect, int len);
  int write_cent_l(LPSTR text, int x_coord, int y_coord, int width, int len);
  int write_cent_l(LPSTR text, RECT *rect, int len);
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

  int draw(Buffer *source, int index, int x, int y, int flag_a, int flag_b);
  // Tile `source` across a destination rectangle. Declared here for the same
  // reason as `draw` above: the homed Win bodies paint with it and its own
  // body is still an artifact.
  int tile(Buffer *source, int src_x, int src_y, int x, int y, int w, int h);
  int fill(int color);
  int fill(RECT *area, int color);  // 005DFCD0
  // 0x005D8240 - builds a RECT from the four coordinates and forwards to
  // the RECT overload above.
  int fill(int left, int top, int width, int height, int color);
  // 0x005DA330, a pending_bodies forwarder.
  // DECLARED AND NEVER DEFINED, so there is no body to read the four ints
  // off and no caller in this tree to read them off either. Left scaffold
  // for the same reason as `poly` above.
  int map_colors(int a1, int a2, int a3, int a4, void *table);
  int load_pcx(const char *filename, Palette *palette, int tgl, int height);
  // 0x005E2690, the actual PCX decoder: the by-name overload above maps the
  // file and hands over the bytes and their length.
  // 0x005E2690. The last two are a colour base and a colour count - the
  // by-name overload calls them `tgl` and `height` and passes them straight
  // through, but this one adds the first to every decoded pixel and copies
  // that many palette entries.
  int load_pcx(BYTE *data, DWORD size, Palette *palette, int colour_base,
               int colour_count);
  int copy(Buffer *buffer, int xCoord, int yCoord, int width, int height,
           int src_width, int src_height);

 // PROTECTED, NOT PRIVATE: GraphicWin derives from Buffer and the homed
 // window bodies read these directly - which is what the image does, since
 // the surface subobject and the window are one object (Win already grants
 // itself friendship for the same reason). Access to a member affects no
 // offset and no emitted byte, so the layout and every claim are untouched.
 protected:
  // `Win::flip` clips the screen buffer back to its own extent and reads
  // the DIB's dimensions for the final BitBlt.
  friend class Win;

  // Caviar::init_class reads the two voxel scene surfaces' `dib_bits_` into
  // its render record - the image reads BufferA/BufferB + 0x54 by fixed
  // address, and the object spelling names the field. Access control is
  // compile-time only, so no offset and no emitted byte moves.
  friend class Caviar;
  
  // 0x4. The window the surface belongs to - `GraphicWin::init` stores its
  // `this` here before the minimum-size computation, and nothing recovered
  // reads it. `void *` until a reader proves which base pointer the image
  // stores; it was `uint32_t`, which needed a cast at the one store.
  void *poOwner_;
  uint32_t field_8_;  // 0x0008
  uint32_t field_C_;
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
  IDirectDrawSurface *surface_;
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

// ?polygon@@YAHPAUBuffer@@PAUVert@@HH@Z at 0x00626620 is the 736-byte
// rasteriser Buffer::poly hands its arguments to. YA = __cdecl, H = int, and
// the arguments are (Buffer *, Vert *, int, int). Still unrecovered, so this
// is a declaration with no definition - deliberately NOT a rebindable seam,
// because Buffer::poly is byte-exact on a direct `call rel32` and a seam
// would turn it into `call dword ptr [X]`.
int polygon(Buffer *buffer, Vert *verts, int a3, int a4);

// DirectDraw surface vtable slots the data lock/release pair dispatches
// through; both are stdcall COM methods on the surface interface.
static const size_t BufferSurfaceLockSlot = 0x64;
static const size_t BufferSurfaceUnlockSlot = 0x80;



void __fastcall buffer_destructor_redirect(Buffer *self, void *);
Buffer *__fastcall buffer_construct_redirect(Buffer *self, void *);

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
extern IDirectDraw *BufferDirectDraw;

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

