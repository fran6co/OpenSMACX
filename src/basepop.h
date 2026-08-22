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
#include "graphicwin.h"
#include "dialogs.h"
#include "checkbox.h"
#include "flatbutton.h"

class SpriteBox;  // sprite_box() below returns a pointer only; spritebox.h
                  // is not included here to avoid pulling in its own chain.

 /*
  * BasePop class
  */
class BasePop : public GraphicWin {
  // `pops()` (popup.cpp) reaches BasePop's fields directly - field_2144_,
  // field_2274_, field_A14_, field_30B4_..field_30C0_, field_3100_,
  // field_3104_, field_3108_ - the way the image's own free function does,
  // through the same object rather than through an accessor per field.
  friend int __cdecl pops(char *caption, char *label, int a3, char *a4,
                          int a5, Sprite *sprite, int a7, int a8,
                          int (__cdecl *callback)());

 public:
  // 0x006015B0, a pending_bodies forwarder.
  int init(int a1, long a2);

 public:
  // STATIC, AND THE CATALOGUED NAME IS WRONG. It reads
  // `?set_def_ok_text@BasePop@@QAAHPAD@Z` - QAA, a `__cdecl` MEMBER, which
  // would push `this` as the first argument. The image reads its string from
  // [esp+8], the FIRST slot, so there is no receiver: declared as a member
  // the body reads [esp+0xc] and every argument shifts. The `symbol` fact on
  // each marker records what this tree emits instead.
  // 0x00600F00, still a pending_bodies forwarder. Declared HERE because
  // `BasePop` inherits GraphicWin privately, so an undeclared `close`
  // resolves to the inaccessible base one.
  void close();
  // 0x00601BF0, still a pending_bodies forwarder: `pops()` (popup.cpp) calls
  // it BY NAME so it emits the image's `call rel32`.
  int start(char *a1, const char *a2, int a3, char *a4, int a5,
           GraphicWin *a6);
  // 0x00602600, still a pending_bodies forwarder: the two `exec`
  // overloads below call it BY NAME so they emit the image's `E8`.
  int exec(int flags, int(__cdecl *callback)());
  static int __cdecl set_def_ok_text(LPSTR text);
  static int __cdecl set_def_cancel_text(LPSTR text);

 public:
  static void fallout();

  // `static`, because the receiver its catalogued name claims is a receiver
  // NOTHING PASSES AND NOTHING READS. The body at 0x00604E40 is a factory -
  // `operator new(0x3230)` then `BasePop::BasePop()` on the result - and its
  // `push ecx` at 0x00604E55 is VC6 reserving four stack bytes, overwritten
  // by `mov [esp], eax` two instructions later; every later `mov ecx, ...`
  // WRITES ecx, for the constructor call and for the SEH unlink. All seven
  // `call dword ptr [0x696ecc]` sites reach it with no ecx set up at all.
  //
  // So `QAE` in `?basepop_alloc@BasePop@@QAEHXZ` describes a `this` that does
  // not exist, and declaring it static is what lets WinMain take its ADDRESS
  // - which is the only thing WinMain does with it.
  static int init_class();          // 00604590
  static int basepop_alloc();
  void UNK3(int a1);
  void UNK4(int a1);
  int on_key_click(int a1, int a2);
  int on_key_up(int a1);
  void set_string_color(int c1, int c2, int c3, int c4);
  void set_string_color2(int c1, int c2, int c3, int c4);
  void set_string_color3(int c1, int c2, int c3, int c4);
  void set_string_color_hyper(int c1, int c2, int c3, int c4);
  // The FIRST parameter is a `char` - `D` in
  // ?set_button_color@BasePop@@QAEXDHHH@Z - and the bytes agree: 0x00604830
  // is `mov al, byte ptr [esp+4]` into `mov byte ptr [ecx+0x3178], al`,
  // while parameters 2-4 are dword loads into dword fields. `char`, not
  // `signed char`: they are distinct MSVC types mangling D and C.
  void set_button_color(char c1, int c2, int c3, int c4);
  void set_button_color2(char c1, int c2, int c3, int c4);
  void set_button_color3(char c1, int c2, int c3, int c4);
  static void set_def_string_color(int c1, int c2, int c3, int c4);
  static void set_def_string_color2(int c1, int c2, int c3, int c4);
  static void set_def_string_color3(int c1, int c2, int c3, int c4);
  static void set_def_string_color_hyper(int c1, int c2, int c3, int c4);
  static void set_def_button_color(int c1, int c2, int c3, int c4);
  static void set_def_button_color2(int c1, int c2, int c3, int c4);
  static void set_def_button_color3(int c1, int c2, int c3, int c4);
  // Static defaults shared by every popup; __cdecl in the original because
  // they take no instance.
  static int set_def_string_font(Font *font1, Font *font2, Font *font3,
                                 Font *font4);
  uint32_t read_check();
  int item(char *text, int index);
  int set_button_font(Font *font1, Font *font2, Font *font3);
  static int set_def_button_font(Font *font1, Font *font2, Font *font3);
  // DECLARED, NOT DEFINED INLINE, and that is what lets `basepop_alloc` be
  // recovered. Its body is `return (int)new BasePop();` and the image calls
  // `??0BasePop@@QAE@XZ` at 0x00604E75; an empty inline constructor is
  // inlined away, the call never appears, and every instruction after it
  // shifts. Measured on that body with this line as the only difference:
  //
  //     BasePop();        BYTE_EXACT
  //     BasePop() { ; }   MISMATCH at #1
  //
  // src/*.h has 145 empty inline constructors and destructors across 75
  // classes, so the same question is waiting on most of them.
  BasePop();
  // 0x004064D0 is not recovered: a
  // pending_bodies forwarder, because an empty inline stub emits
  // nothing and the deleting destructor needs a `call rel32`.
  ~BasePop();
  void set_loc(int x, int y);
  void set_width(int width);
  void write_check(long value);
  int exec();
  int exec(int (__cdecl *callback)());
  int set_string_font(Font *font1, Font *font2, Font *font3, Font *font4);

 private:
  uint32_t field_A14_;
  uint32_t field_A18_;
  uint32_t field_A1C_;
  uint32_t field_A20_;
  uint32_t field_A24_;
  Heap heap_;
  uint32_t field_A3C_;
  uint32_t field_A40_;
  uint32_t field_A44_;
  uint32_t field_A48_;
  uint32_t field_A4C_;
  uint32_t field_A50_;
  uint32_t field_A54_;
  uint32_t field_A58_;
  FlatButton flat_button1_;
  FlatButton flat_button2_;
  uint32_t field_20F4_;
  uint32_t field_20F8_;
  uint32_t field_20FC_;
  uint32_t field_2100_;
  uint32_t field_2104_;
  uint32_t field_2108_;
  uint32_t field_210C_;
  uint32_t field_2110_;
  uint32_t field_2114_;
  Sprite sprite_;
  uint32_t field_2144_;
  uint32_t field_2148_;
  uint32_t field_214C_;
  uint32_t field_2150_;
  uint32_t field_2154_;
  uint32_t field_2158_;
  uint32_t field_215C_;
  uint32_t field_2160_;
  uint32_t field_2164_;
  uint32_t field_2168_;
  uint32_t field_216C_;
  uint32_t field_2170_;
  uint32_t field_2174_;
  uint32_t field_2178_;
  uint32_t field_217C_;
  uint32_t field_2180_;
  uint32_t field_2184_;
  uint32_t field_2188_;
  uint32_t field_218C_;
  uint32_t field_2190_;
  uint32_t field_2194_;
  uint32_t field_2198_;
  uint32_t field_219C_;
  uint32_t field_21A0_;
  uint32_t field_21A4_;
  uint32_t field_21A8_;
  uint32_t field_21AC_;
  uint32_t field_21B0_;
  uint32_t field_21B4_;
  uint32_t field_21B8_;
  uint32_t field_21BC_;
  uint32_t field_21C0_;
  uint32_t field_21C4_;
  uint32_t field_21C8_;
  uint32_t field_21CC_;
  // Split so `field_2274_` - the flag `pops()` (popup.cpp) tests, right
  // after the SpriteBox call below, to decide whether a caller-supplied
  // sprite replaces the popup's own - has a name. The CheckBox at +0x2228
  // and the SpriteBox at +0x2240 both still live in the opaque first span;
  // neither is carved out as a stored member (see check_box() below).
  uint8_t dialogs_[0x2274 - 0x21D0];
  uint32_t field_2274_;
  uint8_t dialogs_tail_[0x2E64 - 0x2278];

 public:
  // The CheckBox subobject at 0x2228, which is INSIDE `dialogs_` above. Not
  // carved out as a member: CheckBox has virtual bases, so declaring one here
  // would give BasePop a generated constructor writing vbtables the image's
  // does not write. Until the block is split this is the one place the
  // address arithmetic lives, and it is typed - the call it feeds compiles
  // `call rel32` like the image's, which a pointer-to-member could not.
  CheckBox *check_box() {
    return reinterpret_cast<CheckBox *>(
        reinterpret_cast<uint8_t *>(this) + 0x2228);
  }

  // The SpriteBox subobject at 0x2240, also inside `dialogs_` above and not
  // carved out as a stored member for the same reason as check_box():
  // SpriteBox composes its own GraphicWin/Dialog virtual bases by hand (see
  // spritebox.h), so a real `SpriteBox` member here would give BasePop a
  // generated constructor the image does not have.
  SpriteBox *sprite_box() {
    return reinterpret_cast<SpriteBox *>(
        reinterpret_cast<uint8_t *>(this) + 0x2240);
  }

 private:
  uint32_t field_2E64_;
  uint32_t field_2E68_;
  uint32_t field_2E6C_;
  uint32_t field_2E70_;
  uint32_t field_2E74_;
  uint32_t field_2E78_;
  uint32_t field_2E7C_;
  uint32_t field_2E80_;
  uint32_t field_2E84_;
  uint32_t field_2E88_;
  uint32_t field_2E8C_;
  uint32_t field_2E90_;
  uint32_t field_2E94_;
  uint32_t field_2E98_;
  uint32_t field_2E9C_;
  uint32_t field_2EA0_;
  uint32_t field_2EA4_;
  uint32_t field_2EA8_;
  uint32_t field_2EAC_;
  uint32_t field_2EB0_;
  uint32_t field_2EB4_;
  uint32_t field_2EB8_;
  uint32_t field_2EBC_;
  uint32_t field_2EC0_;
  uint32_t field_2EC4_;
  uint32_t field_2EC8_;
  uint32_t field_2ECC_;
  uint32_t field_2ED0_;
  uint32_t field_2ED4_;
  uint32_t field_2ED8_;
  uint32_t field_2EDC_;
  uint32_t field_2EE0_;
  uint32_t field_2EE4_;
  uint32_t field_2EE8_;
  uint32_t field_2EEC_;
  uint32_t field_2EF0_;
  uint32_t field_2EF4_;
  uint32_t field_2EF8_;
  uint32_t field_2EFC_;
  uint32_t field_2F00_;
  uint32_t field_2F04_;
  uint32_t field_2F08_;
  uint32_t field_2F0C_;
  uint32_t field_2F10_;
  uint32_t field_2F14_;
  uint32_t field_2F18_;
  uint32_t field_2F1C_;
  uint32_t field_2F20_;
  uint32_t field_2F24_;
  uint32_t field_2F28_;
  uint32_t field_2F2C_;
  uint32_t field_2F30_;
  uint32_t field_2F34_;
  uint32_t field_2F38_;
  uint32_t field_2F3C_;
  uint32_t field_2F40_;
  uint32_t field_2F44_;
  uint32_t field_2F48_;
  uint32_t field_2F4C_;
  uint32_t field_2F50_;
  uint32_t field_2F54_;
  uint32_t field_2F58_;
  uint32_t field_2F5C_;
  uint32_t field_2F60_;
  uint32_t field_2F64_;
  uint32_t field_2F68_;
  uint32_t field_2F6C_;
  uint32_t field_2F70_;
  uint32_t field_2F74_;
  uint32_t field_2F78_;
  uint32_t field_2F7C_;
  uint32_t field_2F80_;
  uint32_t field_2F84_;
  uint32_t field_2F88_;
  uint32_t field_2F8C_;
  uint32_t field_2F90_;
  uint32_t field_2F94_;
  uint32_t field_2F98_;
  uint32_t field_2F9C_;
  uint32_t field_2FA0_;
  uint32_t field_2FA4_;
  uint32_t field_2FA8_;
  uint32_t field_2FAC_;
  uint32_t field_2FB0_;
  uint32_t field_2FB4_;
  uint32_t field_2FB8_;
  uint32_t field_2FBC_;
  uint32_t field_2FC0_;
  uint32_t field_2FC4_;
  uint32_t field_2FC8_;
  uint32_t field_2FCC_;
  uint32_t field_2FD0_;
  uint32_t field_2FD4_;
  uint32_t field_2FD8_;
  uint32_t field_2FDC_;
  uint32_t field_2FE0_;
  uint32_t field_2FE4_;
  uint32_t field_2FE8_;
  uint32_t field_2FEC_;
  uint32_t field_2FF0_;
  uint32_t field_2FF4_;
  uint32_t field_2FF8_;
  uint32_t field_2FFC_;
  uint32_t field_3000_;
  uint32_t field_3004_;
  uint32_t field_3008_;
  uint32_t field_300C_;
  uint32_t field_3010_;
  uint32_t field_3014_;
  uint32_t field_3018_;
  uint32_t field_301C_;
  uint32_t field_3020_;
  uint32_t field_3024_;
  uint32_t field_3028_;
  uint32_t field_302C_;
  uint32_t field_3030_;
  uint32_t field_3034_;
  uint32_t field_3038_;
  uint32_t field_303C_;
  uint32_t field_3040_;
  uint32_t field_3044_;
  uint32_t field_3048_;
  uint32_t field_304C_;
  uint32_t field_3050_;
  uint32_t field_3054_;
  uint32_t field_3058_;
  uint32_t field_305C_;
  uint32_t field_3060_;
  uint32_t field_3064_;
  uint32_t field_3068_;
  uint32_t field_306C_;
  uint32_t field_3070_;
  uint32_t field_3074_;
  uint32_t field_3078_;
  uint32_t field_307C_;
  uint32_t field_3080_;
  uint32_t field_3084_;
  uint32_t field_3088_;
  uint32_t field_308C_;
  uint32_t field_3090_;
  uint32_t field_3094_;
  Spot spot_;
  uint32_t field_30A4_;
  uint32_t field_30A8_;
  uint32_t field_30AC_;
  uint32_t field_30B0_;
  uint32_t field_30B4_;
  uint32_t field_30B8_;
  uint32_t field_30BC_;
  uint32_t field_30C0_;
  uint32_t field_30C4_;
  uint32_t field_30C8_;
  uint32_t field_30CC_;
  uint32_t field_30D0_;
  uint32_t field_30D4_;
  uint32_t field_30D8_;
  uint32_t field_30DC_;
  uint32_t field_30E0_;
  uint32_t field_30E4_;
  uint32_t field_30E8_;
  uint32_t field_30EC_;
  uint32_t field_30F0_;
  uint32_t field_30F4_;
  uint32_t loc_a_;
  uint32_t loc_b_;
  uint32_t field_3100_;
  uint32_t field_3104_;
  uint32_t field_3108_;
  uint32_t field_310C_;
  Font *string_font1_;
  Font *string_font2_;
  Font *string_font3_;
  Font *string_font4_;
  int string_color_a_;
  int string_color_2a_;
  int string_color_3a_;
  int string_color_hyper_a_;
  int string_color_b_;
  int string_color_2b_;
  int string_color_3b_;
  int string_color_hyper_b_;
  int string_color_c_;
  int string_color_2c_;
  int string_color_3c_;
  int string_color_hyperc_;
  int string_color_d_;
  int string_color_2d_;
  int string_color_3d_;
  int string_color_hyper_d_;
  uint32_t field_3160_;
  uint32_t field_3164_;
  uint32_t field_3168_;
  Font *button_font1_;
  Font *button_font2_;
  Font *button_font3_;
  uint8_t button_color_a_;
  uint8_t button_color_2a_;
  uint8_t button_color_3a_;
  uint8_t field_317B_;
  uint32_t button_color_b_;
  uint32_t button_color_2b_;
  uint32_t button_color_3b_;
  uint32_t button_color_c_;
  uint32_t button_color_2c_;
  uint32_t button_color_3c_;
  uint32_t button_color_d_;
  uint32_t button_color_2d_;
  uint32_t button_color_3d_;
  uint8_t field_31A0_;
  uint8_t field_31A1_;
  uint8_t field_31A2_;
  uint8_t field_31A3_;
  uint32_t field_31A4_;
  uint32_t field_31A8_;
  uint32_t field_31AC_;
  uint32_t field_31B0_;
  uint32_t field_31B4_;
  uint32_t field_31B8_;
  uint32_t field_31BC_;
  uint32_t field_31C0_;
  uint32_t field_31C4_;
  uint32_t field_31C8_;
  uint32_t field_31CC_;
  uint32_t field_31D0_;
  uint32_t field_31D4_;
  uint32_t field_31D8_;
  uint32_t field_31DC_;
  uint32_t field_31E0_;
  uint32_t field_31E4_;
  uint32_t field_31E8_;
  uint32_t field_31EC_;
  uint32_t field_31F0_;
  uint32_t field_31F4_;
  uint32_t field_31F8_;
  uint32_t field_31FC_;
  uint32_t field_3200_;
  uint32_t field_3204_;
  uint32_t field_3208_;
  uint32_t field_320C_;
  uint32_t field_3210_;
  uint32_t field_3214_;
  uint32_t field_3218_;
  uint32_t field_321C_;
  uint32_t field_3220_;
  LPSTR ok_text_;
  LPSTR cancel_text_;
  uint32_t field_322C_;
};


void __fastcall base_pop_write_check_redirect(BasePop *self, void *, long value);

// The two-argument exec these forward to is not recovered.
typedef int (OriginalObject::*func_base_pop_exec)(int flag, int (__cdecl *callback)());

int __fastcall base_pop_exec_void_redirect(BasePop *self, void *);
int __fastcall base_pop_exec_callback_redirect(BasePop *self, void *,
                                               int (__cdecl *callback)());

// The screen width the high-resolution scaling switches on, read at a
// fixed address. Rebindable so tests can drive both sides of the branch.
int32_t *const BasePopScreenWidth = (int32_t *)0x009B7B1C;

void __fastcall base_pop_set_width_redirect(BasePop *self, void *, int width);

static_assert(sizeof(BasePop) == 0x3230,
              "BasePop layout must match the original executable");

void __fastcall base_pop_set_loc_redirect(BasePop *self, void *, int x, int y);
int __fastcall base_pop_set_string_font_redirect(
    BasePop *self, void *, Font *font1, Font *font2, Font *font3, Font *font4);

int __cdecl base_pop_set_def_string_font_redirect(
    Font *font1, Font *font2, Font *font3, Font *font4);
int __cdecl base_pop_set_def_button_font_redirect(
    Font *font1, Font *font2, Font *font3);

// Default string and button font slots at 0x009B8D98 and 0x009B8DA8; tests
// outside the hybrid process rebind them.
// ARRAYS, NOT POINTERS. Four string fonts at 0x009B8D98 and three button
// fonts at 0x009B8DA8; the image stores straight into them.
extern Font *BasePopDefaultStringFonts[4];  // 0x009B8D98
extern Font *BasePopDefaultButtonFonts[3];  // 0x009B8DA8

void __cdecl base_pop_set_def_string_color_redirect(int c1, int c2, int c3, int c4);
void __cdecl base_pop_set_def_string_color2_redirect(int c1, int c2, int c3, int c4);
void __cdecl base_pop_set_def_string_color3_redirect(int c1, int c2, int c3, int c4);
void __cdecl base_pop_set_def_string_color_hyper_redirect(
    int c1, int c2, int c3, int c4);
void __cdecl base_pop_set_def_button_color_redirect(int c1, int c2, int c3, int c4);
void __cdecl base_pop_set_def_button_color2_redirect(int c1, int c2, int c3, int c4);
void __cdecl base_pop_set_def_button_color3_redirect(int c1, int c2, int c3, int c4);

// Two interleaved default-colour tables. The string table carries four tiers
// at a 0x10 slot stride; the button table carries three at 0xC. Tests rebind
// both.
extern uint32_t BasePopDefaultStringColors[4][4];  // 0x00696EE4
extern uint32_t BasePopDefaultButtonColors[4][3];  // 0x00696F24

uint32_t __fastcall base_pop_read_check_redirect(BasePop *self, void *);
int __fastcall base_pop_item_redirect(BasePop *self, void *, char *text, int index);
int __fastcall base_pop_set_button_font_redirect(BasePop *self, void *, Font *font1, Font *font2, Font *font3);
void __fastcall base_pop_unk3_redirect(BasePop *self, void *, int a1);
void __fastcall base_pop_unk4_redirect(BasePop *self, void *, int a1);
int __fastcall base_pop_on_key_click_redirect(BasePop *self, void *, int a1, int a2);
int __fastcall base_pop_on_key_up_redirect(BasePop *self, void *, int a1);
void __fastcall base_pop_set_string_color_redirect(
    BasePop *self, void *, int c1, int c2, int c3, int c4);
void __fastcall base_pop_set_string_color2_redirect(
    BasePop *self, void *, int c1, int c2, int c3, int c4);
void __fastcall base_pop_set_string_color3_redirect(
    BasePop *self, void *, int c1, int c2, int c3, int c4);
void __fastcall base_pop_set_string_color_hyper_redirect(
    BasePop *self, void *, int c1, int c2, int c3, int c4);
void __fastcall base_pop_set_button_color_redirect(
    BasePop *self, void *, char c1, int c2, int c3, int c4);
void __fastcall base_pop_set_button_color2_redirect(
    BasePop *self, void *, char c1, int c2, int c3, int c4);
void __fastcall base_pop_set_button_color3_redirect(
    BasePop *self, void *, char c1, int c2, int c3, int c4);

void __cdecl base_pop_fallout_redirect();

// Set when the fallout flag is raised, but only while the gate global is set.
int *const BasePopFalloutGate = (int *)0x009B8D00;
int *const BasePopFalloutFlag = (int *)0x009B8CFC;

// The two default button captions, heap copies the class owns. `init_class`
// seeds them from the literals "OK" and "Cancel"; the two setters below
// replace them.
extern char *BasePopDefaultOkText;      // 0x009B8D80
extern char *BasePopDefaultCancelText;  // 0x009B8D84
