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
 * but WITHOUT ANY WARRANTY_; without even the implied warranty of
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

 /*
  * Buffer class
  */
class DLLEXPORT Buffer {
 public:
  Buffer() { ; }
  ~Buffer() { ; }

  void construct();
  void clear_links();
  int set_font(Font *font1, Font *font2, Font *font3, Font *font4);
  void set_text_color(int color1, int color2, int color3, int color4);
  void set_text_color2(int color1, int color2, int color3, int color4);
  void set_text_color3(int color1, int color2, int color3, int color4);
  void set_text_color_hyper(int color1, int color2, int color3, int color4);
  int init_class();
  void close_class();
  int sync_to_palette(Palette *palette);
  HDC get_hdc();
  void release_hdc(int count);
  int get_data();
  int set_clip(RECT *rect);
  int text_width(LPSTR text);
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
  // Destructor body kept as a named method so the trivial ~Buffer() stays
  // trivial and embedding classes keep their existing implicit destruction.
  void destroy();
  void free_data(int count);

 private:
  typedef int32_t Dib;
  
  LPVOID vtable_;
  uint32_t poOwner_;
  uint32_t field_8_;
  uint32_t field_C_;
  uint32_t field_10_;
  uint32_t field_14_;
  uint32_t field_18_;
  uint32_t field_1C_;
  RECT rect1_;
  RECT rect2_;
  uint32_t field_40_[4];
  uint32_t field_50_;
  LPVOID *ppv_bits_;
  uint32_t field_58_;
  uint32_t field_5C_;
  HDC hdc2_;
  HDC hdc_;
  uint32_t field_68_;
  uint32_t field_6C_;
  HRGN field_70_;
  uint32_t field_74_;
  HBITMAP bitmap_handle_;
  const BITMAPINFO *bitmap_info_;
  uint32_t width_;
  uint32_t height_;
  uint16_t field_88_;
  uint16_t field_8A_;
  uint32_t field_8C_;
  uint32_t field_90_;
  uint32_t field_94_;
  uint32_t field_98_;
  uint32_t field_9C_;
  uint32_t field_A0_;
  Dib dib_[256];
  uint32_t field_4A4_;
  uint32_t field_4A8_;
  uint32_t field_4AC_;
  Spot spot_;
  uint8_t field_4BC_[80];
  uint32_t field_50C_;
  uint32_t field_510_;
  uint32_t field_514_;
  uint32_t field_518_;
  uint32_t field_51C_;
  uint32_t field_520_;
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
  uint32_t field_57C_;
  int8_t field_580_;
  uint32_t field_584_;
};

static_assert(sizeof(Buffer) == 0x588, "Buffer layout must match the original executable");

// DirectDraw surface vtable slots the data lock/release pair dispatches
// through; both are stdcall COM methods on the surface interface.
constexpr size_t BufferSurfaceLockSlot = 0x64;
constexpr size_t BufferSurfaceUnlockSlot = 0x80;

// The seven-argument copy is the real blitter and is not recovered yet, so the
// convenience overload reaches it through a seam.
typedef int (OriginalObject::*func_buffer_copy_full)(Buffer *, int, int, int, int, int, int);
extern func_buffer_copy_full BufferCopyFull;

int __fastcall buffer_copy_redirect(Buffer *self, void *, Buffer *buffer,
                                    int xCoord, int yCoord,
                                    int width, int height);
int __fastcall buffer_copy_rect_redirect(Buffer *self, void *, Buffer *buffer,
                                         RECT *rect);

int __fastcall buffer_get_data_redirect(Buffer *self, void *);
int __fastcall buffer_text_line_height_redirect(Buffer *self, void *);
void __fastcall buffer_close_redirect(Buffer *self, void *);
void __fastcall buffer_destructor_redirect(Buffer *self, void *);
extern const uint32_t BufferVtable;
extern Palette **BufferPalette;
Buffer *__fastcall buffer_construct_redirect(Buffer *self, void *);

// Selects the DirectDraw teardown path over the GDI device-context path.
extern int *BufferDirectDrawActive;
// Value the close reset writes at offset 0x520; its meaning is unconfirmed.
extern uint32_t *BufferResetValue520;
// Releases a Sprite-style allocation through the executable's own CRT.
extern func_sprite_free *BufferFree;

void __fastcall buffer_clear_links_redirect(Buffer *self, void *);
void __fastcall buffer_free_data_redirect(Buffer *self, void *, int count);

int __fastcall buffer_set_font_redirect(
    Buffer *self, void *, Font *font1, Font *font2, Font *font3, Font *font4);
void __fastcall buffer_set_text_color_redirect(
    Buffer *self, void *, int color1, int color2, int color3, int color4);
void __fastcall buffer_set_text_color2_redirect(
    Buffer *self, void *, int color1, int color2, int color3, int color4);
void __fastcall buffer_set_text_color3_redirect(
    Buffer *self, void *, int color1, int color2, int color3, int color4);
void __fastcall buffer_set_text_color_hyper_redirect(
    Buffer *self, void *, int color1, int color2, int color3, int color4);
int __cdecl buffer_init_class_redirect();
void __cdecl buffer_close_class_redirect();

HDC __fastcall buffer_get_hdc_redirect(Buffer *self, void *);
void __fastcall buffer_release_hdc_redirect(Buffer *self, void *, int count);

int __fastcall buffer_sync_to_palette_redirect(
    Buffer *self, void *, Palette *palette);

int __fastcall buffer_text_height_redirect(Buffer *self, void *);

int __fastcall buffer_set_clip_redirect(Buffer *self, void *, RECT *rect);

int __fastcall buffer_text_width_redirect(Buffer *self, void *, LPSTR text);

// The four-argument line primitives at 0x005E1A80 (hline) and 0x005E1BF0
// (vline) are clipped pixel writers, still original dependencies. The
// recovered box outline reaches them through these seams; tests rebind them.
// Both are thiscall voids (ret 0x10) whose EAX residue the box body
// deliberately discards - the original zeroes EAX after the last call
// (`xor eax, eax` at 0x005E327A).
typedef void (OriginalObject::*func_buffer_line)(int, int, int, int);
extern func_buffer_line BufferHLine;
extern func_buffer_line BufferVLine;

int __fastcall buffer_box_redirect(Buffer *self, void *, RECT *rect,
                                   int color1, int color2);

// The measured overload this one wraps is a 578-byte body with three call
// targets, still an original dependency. Tests rebind this seam.
typedef int (OriginalObject::*func_buffer_text_width_measured)(LPSTR, size_t);
extern func_buffer_text_width_measured BufferTextWidthMeasured;

// The 835-byte multi-font raster writer at 0x005DCAE0 is the real glyph
// emitter behind every length-limited text entry point, and is still an
// original dependency. All four recovered writers reach it through this seam;
// the leaf tests rebind it to a recorder. Its EAX passes straight through -
// the original ends `mov eax, edi` at 0x005DCE18, handing back the advanced
// pen position, which is why the scalar writers return the incoming x when
// they emit nothing.
typedef int (OriginalObject::*func_buffer_write_multi_font_raw_l)(LPSTR, int, int, int);
extern func_buffer_write_multi_font_raw_l BufferWriteMultiFontRawL;

int __fastcall buffer_write_l_redirect(Buffer *self, void *, LPSTR text,
                                       int x_coord, int y_coord, int len);
int __fastcall buffer_write_l_rect_redirect(Buffer *self, void *, LPSTR text,
                                            RECT *rect, int len);
int __fastcall buffer_write_cent_l_redirect(Buffer *self, void *, LPSTR text,
                                            int x_coord, int y_coord,
                                            int width, int len);
int __fastcall buffer_write_cent_l_rect_redirect(Buffer *self, void *,
                                                 LPSTR text, RECT *rect,
                                                 int len);
