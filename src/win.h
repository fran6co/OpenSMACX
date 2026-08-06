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
#include "autosound.h"
#include "buffer.h"
#include "palette.h"
#include "heap.h"
#include "spot.h"
#include "sprite.h"

class Menu; // forward declaration
// The ninth Win::init / GraphicWin::init argument, spelled PAUBorderSizing@@
// in both mangled names - a struct, hence the tag. Nothing on the recovered
// surface dereferences it, so it stays incomplete.
struct BorderSizing;
class Scroll; // forward declaration

 /*
  * Win class: Most basic window class.
  */
class DLLEXPORT Win {
  friend class Scroll;
  // BaseButton's colour setters test the parent link before drawing.
  friend class BaseButton;

 public:
  int UNK1(int a, int b, int c, int d, int e, int f, int g, int h, int i);
  int UNK5();
  int UNK6(int a);
  int on_set_cursor(void *a, unsigned int b, unsigned int c);
  Win() { ; }
  ~Win() { ; }

  void construct();
  int move(int x, int y);
  int is_visible();
  int is_descendant(Win *candidate);
  int is_dialog_focus();
  int set_cursor(int name);
  static void clear_bubble_text();
  static void set_def_focus(int focus);
  void UNK8(int value);
  void UNK9(int value);
  void reset_window_clip();
  void sync_palette();
  int UNK3(int value);
  static int OnSetCursor(void *a1, void *a2, unsigned int a3, unsigned int a4);
  void set_vert_pos(int position);
  void set_horz_pos(int position);
  void set_vert_range(int minimum, int maximum);
  void set_horz_range(int minimum, int maximum);

  // The process-wide device context every window shares. The legacy bodies
  // take no instance and clean no stack, so they are statics here.
  static HDC get_hdc();
  static void release_hdc();
  void client_to_screen(int *x, int *y);
  int on_query_new_palette();
  int get_vert_pos();
  int get_horz_pos();
  void set_vert_paging(int paging);
  void set_horz_paging(int paging);

  void on_move(int a1, int a2);
  void on_size(unsigned int a1, int a2, int a3);
  void on_size_nc(unsigned int a1, int a2, int a3);
  void on_sys_command(unsigned int a1, int a2, int a3);
 private:
  AutoSound auto_sound_;
  uint32_t iFlags_;
  uint32_t iSomeFlag_;
  uint32_t field_A0_;
  uint32_t field_A4_;
  uint32_t poWinBase_;
  uint32_t iVertScaleDenom_;
  uint32_t iVertScaleNum_;
  Buffer *buffer1_;
  Buffer *buffer2_;
  Buffer *buffer3_;
  Buffer *buffer4_;
  Win *win_parent_;
  uint32_t field_C8_;
  uint32_t field_CC_;
  uint32_t field_D0_;
  uint32_t field_D4_;
  uint32_t field_D8_;
  Heap heap_;
  Menu *menu_;
  uint32_t field_F4_;
  uint32_t field_F8_;
  uint32_t field_FC_;
  uint32_t field_100_;
  uint32_t field_104_;
  uint32_t field_108_;
  uint32_t field_10C_;
  uint32_t field_110_;
  int caption_height_;
  int border_thickness_;
  int bottom_border_thickness_;
  uint32_t field_120_;
  uint32_t field_124_;
  uint32_t field_128_;
  uint32_t field_12C_;
  uint32_t field_130_;
  uint32_t field_134_;
  uint32_t field_138_;
  RECT outer_rect_;
  RECT client_rect_;
  uint32_t field_15C_;
  uint32_t field_160_;
  uint32_t field_164_;
  uint32_t field_168_;
  uint32_t field_16C_;
  uint32_t field_170_;
  uint32_t field_174_;
  uint32_t field_178_;
  uint32_t field_17C_;
  uint32_t field_180_;
  uint32_t field_184_;
  Sprite *cursor_sprite_;
  uint32_t field_18C_;
  uint32_t field_190_;
  HCURSOR *cursor_handle_;
  int cursor_name_;
  uint32_t field_19C_;
  uint32_t field_1A0_;
  // The child list. PROVEN from the bytes: Win::add_child 0x005F5200 does
  // `mov [ecx + eax*4 + 0x1a4], esi` with eax = child_count_, so the base
  // is 0x1A4 and the stride is 4; recurse_zorder 0x005F4EC0 and
  // is_descendant 0x005ECE20 walk it the same way, and add_child does NOT
  // bound-check before storing.
  // INFERRED, and the reason the `buffer5_`/`spot_`/`buffer6_` guesses that
  // used to sit in this range are gone: the array runs the whole distance
  // to child_count_, giving 150 slots. Nothing referenced those three, and
  // an unbounded add_child would corrupt anything parked among the slots.
  // If a later recovery proves a real member here, this is what to revisit.
  Win *children_[150];
  // Was `Buffer *buffer6_`. Every caller treats it as a SIGNED count:
  // is_descendant does `test eax,eax / jle` and then `cmp edi, eax / jl`
  // against a loop index.
  int child_count_;
  uint32_t field_400_;
  uint32_t field_404_;
  uint32_t field_408_;
  uint32_t field_40C_;
  uint32_t field_410_;
  uint32_t field_414_;
  uint32_t field_418_;
  uint32_t field_41C_;
  uint32_t field_420_;
  uint32_t field_424_;
  uint32_t field_428_;
  uint32_t field_42C_;
  uint32_t field_430_;
  uint32_t field_434_;
  uint32_t field_438_;
  Scroll *scroll_vert_;
  Scroll *scroll_horz_;
};

static_assert(sizeof(Win) == 0x444, "Win layout must match the legacy ABI");

extern const uint32_t WinPrimaryVtable;
extern const uint32_t WinSecondaryVtable;
extern uint32_t *WinStaticDefaults;
extern uint32_t *WinDynamicDefaults;
Win *__fastcall win_construct_redirect(Win *self, void *);
int __fastcall win_move_redirect(Win *self, void *, int x, int y);
int __fastcall win_is_visible_redirect(Win *self, void *);
void __fastcall win_client_to_screen_redirect(
    Win *self, void *, int *x, int *y);
int __fastcall win_on_query_new_palette_redirect(Win *self, void *);
int __fastcall win_get_vert_pos_redirect(Win *self, void *);
int __fastcall win_get_horz_pos_redirect(Win *self, void *);
void __fastcall win_set_vert_paging_redirect(Win *self, void *, int paging);
void __fastcall win_set_horz_paging_redirect(Win *self, void *, int paging);

DLLEXPORT BOOL __cdecl in_box(int x, int y, const RECT *rect);
DLLEXPORT int __cdecl in_box(
    int x, int y, int left, int top, int width, int height);
void __cdecl offset_rect(RECT *rect, int dx, int dy);
RECT *__cdecl make_rect(RECT *rect, int x, int y, int width, int height);
int __cdecl rect_center(RECT *rect, int *x, int *y);
int __fastcall tutwin_rect_center_redirect(
    void *self, void *, RECT *rect, int *x, int *y);

int __fastcall win_is_dialog_focus_redirect(Win *self, void *);

HDC __cdecl win_get_hdc_redirect();
void __cdecl win_release_hdc_redirect();

// Shared device-context state: the reference count, the cached handle, and
// the optional DirectDraw surface that supplies it. Tests rebind these.
extern int *WinHdcRefCount;
extern HDC *WinSharedHdc;
extern void **WinHdcSurface;
extern HWND *WinHdcWindow;

int __fastcall win_set_cursor_redirect(Win *self, void *, int name);

// The cursor refresh this setter triggers is a 2528-byte body with six call
// targets, still an original dependency. Tests rebind this seam.
typedef int(__cdecl func_win_update_cursor)(Win *, int);
extern func_win_update_cursor *WinUpdateCursorOriginal;

void __cdecl win_clear_bubble_text_redirect();

// Bubble state: the pending flag, its companion slot, and the rectangle the
// refresh republishes. Tests rebind all three.
extern int *WinBubbleActive;
extern int *WinBubbleCompanion;
extern RECT *WinBubbleRect;

// Both refresh bodies remain original dependencies: update_screen is 383
// bytes with four call targets, flip 1223 bytes with fourteen.
typedef int(__cdecl func_win_update_screen)(RECT *, Win *);
typedef void(__cdecl func_win_flip)(RECT *);
extern func_win_update_screen *WinUpdateScreenOriginal;
extern func_win_flip *WinFlipOriginal;

int __fastcall win_unk1_redirect(
    Win *self, void *, int a, int b, int c, int d, int e, int f, int g, int h, int i);
int __fastcall win_unk5_redirect(
    Win *self, void *);
int __fastcall win_unk6_redirect(
    Win *self, void *, int a);
int __fastcall win_on_set_cursor_redirect(
    Win *self, void *, void *a, unsigned int b, unsigned int c);

void __cdecl win_set_def_focus_redirect(int focus);
extern int *WinDefaultFocus;

void __fastcall win_unk8_redirect(Win *self, void *, int value);
void __fastcall win_unk9_redirect(Win *self, void *, int value);
void __fastcall win_reset_window_clip_redirect(Win *self, void *);
void __fastcall win_on_move_redirect(Win *self, void *, int a1, int a2);
void __fastcall win_on_size_redirect(Win *self, void *, unsigned int a1, int a2, int a3);
void __fastcall win_on_size_nc_redirect(Win *self, void *, unsigned int a1, int a2, int a3);
void __fastcall win_on_sys_command_redirect(Win *self, void *, unsigned int a1, int a2, int a3);

// The active palette lives at a fixed address; rebindable so tests can
// point it at a local rather than requiring the mapped global.
extern Palette **WinActivePalette;

void __fastcall win_sync_palette_redirect(Win *self, void *);

void __fastcall win_set_vert_pos_redirect(Win *self, void *, int position);
void __fastcall win_set_horz_pos_redirect(Win *self, void *, int position);
void __fastcall win_set_vert_range_redirect(Win *self, void *, int minimum, int maximum);
void __fastcall win_set_horz_range_redirect(Win *self, void *, int minimum, int maximum);
int __cdecl win_onsetcursor_redirect(void *a1, void *a2, unsigned int a3, unsigned int a4);
int __fastcall win_unk3_redirect(Win *self, void *, int value);

int __fastcall win_is_descendant_redirect(Win *self, void *, Win *candidate);
