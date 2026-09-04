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
#include "stringstruct.h"

// ONE OF THE POPUP'S TWO 0x118 TEXT BLOCKS (0x2E64 and 0x2F7C inside the
// object). 0x104 of string plus five numbers: BasePop::start (0x00601BF0)
// strcat's into `text` and atoi/atol's into each of the five, and
// BasePop::close (0x00600F00) restores the whole 0x118 from the file-backed
// defaults. The five numbers are named by their offsets because nothing in
// the tree reads them back - see the member comment in the class below.
struct BasePopTextBlock {
  char text[0x104];  // 0x0000
  int field_104_;    // 0x0104
  int field_108_;    // 0x0108
  int field_10C_;    // 0x010C
  int field_110_;    // 0x0110
  int field_114_;    // 0x0114
};

static_assert(sizeof(BasePopTextBlock) == 0x118,
              "BasePopTextBlock layout must match the original executable");
static_assert(offsetof(BasePopTextBlock, field_104_) == 0x104,
              "BasePopTextBlock first number must follow the text");
static_assert(offsetof(BasePopTextBlock, field_114_) == 0x114,
              "BasePopTextBlock last number must end the block");

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
  int init(int group_id, long flags);

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
  // Declared MEASURED (dllexport) to match the `inline` definitions in
  // basepop.cpp: the image EXPANDS both of them into init_class (0x00604590)
  // AND keeps the standalone copies at 0x006018A0 / 0x00601910, so the /Ob1
  // inliner has to see `inline` while the dllexport keeps that standalone
  // COMDAT from being dropped once this TU's only caller folds it. game.cpp's
  // callers never see the definition, so they keep the real `call`s the image
  // has there.
  static MEASURED int __cdecl set_def_ok_text(LPSTR text);
  static MEASURED int __cdecl set_def_cancel_text(LPSTR text);

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
  static BasePop *basepop_alloc();
  void UNK3(int a1);
  void UNK4(int a1);
  int on_key_click(int a, int b);
  int on_key_up(int a);
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
  // DEFINED IN-CLASS, the ~FlatButton move, because the image INLINES this
  // destructor at the popup's destruction sites and CALLS it out of line 75
  // times - and only an in-class definition lets both happen.
  // ??1BasePop@@QAE@XZ (0x004064D0, claimed in basepop.cpp) keeps being
  // emitted out-of-line by the MEASURED storage class; the expansion is what
  // Caviar::init_class (0x006185A0) lays down at each of its early returns
  // and what sound_version_warning (0x004C9080) lays down at its one scope
  // exit. Out-of-line here those sites can only emit `call ??1BasePop`, and
  // init_class measured 16/392 with those 200 instructions of teardown
  // MISSING outright rather than divergent.
  //
  // The body is `close()` and nothing else: the rest of the image's body -
  // Spot at +0x3098, the Dialogs slab at +0x21d0, unit_ids_ and item_ids_,
  // sprite_, both FlatButtons, heap_ and the GraphicWin base, in exactly
  // that reverse-declaration order - is the compiler's, generated from the
  // member declarations below, the same way ??1DipEdit is generated from
  // dipedit.h. An empty body produces the identical chain; the one call the
  // member list cannot produce is the image's opening `call ?close`.
  MEASURED ~BasePop() { close(); }
  void set_loc(int x, int y);
  void set_width(int width);
  void write_check(long value);
  int exec();
  int exec(int (__cdecl *callback)());
  int set_string_font(Font *font1, Font *font2, Font *font3, Font *font4);

 private:
  uint32_t field_A14_;  // 0x0a14
  // The STARTING loc_a_/loc_b_ pair: copied over them unless the value is
  // 0x2000, the keep-current sentinel set_loc uses too.
  uint32_t default_loc_a_;  // 0x0a18
  uint32_t default_loc_b_;  // 0x0a1c
  // The DEFAULT dialog width/height, seeded from the file-backed
  // 0x00696EC4 table; the ctor's tail writes them into the embedded
  // Dialog's width/height slots (the height grown 3/2 for wide screens).
  uint32_t dialog_width_;   // 0x0a20
  uint32_t dialog_height_;  // 0x0a24
  Heap heap_;
  // TYPED AS THE POINTER IT HOLDS, but nothing more is claimed: the ctor
  // stores `this` here (image `mov [ebp+0xa3c], ebp` at 0x00600ab0) and
  // NOTHING in the tree reads it back, so the slot's purpose - owner? list
  // link? handle? - is unknown. Named by its offset until someone can say
  // what the popup uses it for.
  void *field_A3C_;  // 0x0a3c
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
  // EIGHT ZEROED DWORDS at 0x20F8, ONE writer and ONE meaning: the ctor's
  // `lea edi,[ebp+0x20f8]; mov ecx,8; xor eax,eax` is a single `rep stosd`
  // (0x006018xx), so this is a block the class clears, not eight separate
  // flags. Typed 2026-09-04; the eight offset names it replaces said nothing
  // the memset does not.
  uint32_t zero_block_[8];  // 0x20F8
  Sprite sprite_;
  uint32_t field_2144_;
  uint32_t field_2148_;
  uint32_t field_214C_;
  // The popup's ITEM-TEXT list, carved 2026-08-30 beside unit_ids_ below on
  // the identical evidence: 0x30 = sizeof(StringList), and the ctor's stores
  // for this block are StringList's two-stage construction (the same
  // 0x0066B0EC vbtable and staged vtables, +0x006693AC virtual-base vtable),
  // which the compiler emits from the declaration. BasePop::start adds the
  // popup's per-item strings here (0x006021A1) as it adds the unit ids to
  // unit_ids_ (0x006024F4).
  StringList item_ids_;  // 0x2150
  public:
  // THE POPUP'S UNIT-NAME LIST, carved 2026-08-30 from what was the twelve
  // dwords field_2180_..field_21AC_: exactly 0x30 = sizeof(StringList), and
  // the image's own ctor writes are StringList's construction, not raw
  // stores - the vbtable (0x0066B0EC) at +0x2184, the StringAllocationBase
  // vftable
  // (0x006693AC) at +0x21A8, the allocator hand-off capture into +0x21AC,
  // and the StringStruct zeroing. The old hand-spelled copies of those
  // stores are deleted from the ctor; the compiler now emits them from this
  // declaration, as it already does for StringBox's stringList_ at 0x2B70.
  // PUBLIC because Caviar::init_class flattens it into StringTemp for the
  // unsupported-CPU popup, and access specifiers do not change layout.
  StringList unit_ids_;  // 0x2180

  private:
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
  // sprite replaces the popup's own - has a name.
  // THE EMBEDDED DIALOGS, real now: the three spans it replaces
  // (dialogs_[0xA4] + field_2274_ + dialogs_tail_[0xBEC]) total exactly
  // sizeof(Dialogs) = 0xC94 at 0x21D0..0x2E64. Its construction is the
  // compiler's (the image's `push 1; call 0x612830` mid-body), and the
  // CheckBox at 0x2228 (+0x58), SpriteBox at 0x2240 (+0x70) and EditGroup at
  // 0x22C8 (+0xF8) are its own base subobjects - see dialogs() below.
  Dialogs dialogs_;  // 0x21D0

 public:
  // THE POPUP'S SUBOBJECTS ARE REACHED THROUGH THE REAL MEMBER `dialogs_`,
  // and this returns it - the same type, so no conversion. The three former
  // offset accessors (check_box 0x2228, sprite_box 0x2240, edit_group
  // 0x22C8) are GONE: each was `reinterpret_cast<Subclass *>(
  // reinterpret_cast<uint8_t *>(this) + OFF)`, a self-pun re-deriving by hand
  // what `dialogs_`'s own base list already states - CheckBox is at
  // 0x21D0+0x58, SpriteBox at +0x70, EditGroup at +0xF8 (dialogs.h).
  //
  // THEY CANNOT COME BACK AS TYPED SUBOBJECT ACCESSORS, measured 2026-09-04
  // against 0x00601BB0 (image 5 instructions):
  //
  //   dialogs_.set_state_flag(value)        BYTE_EXACT 5/5
  //   CheckBox *p = &dialogs_; p->...       MISMATCH 0/5, compiled 13
  //   CheckBox &r = dialogs_; r....         MISMATCH 0/5, compiled 13
  //   check_box() -> CheckBox &, r....      MISMATCH 0/5, compiled 13
  //
  // Binding a base SUBOBJECT to a pointer or reference makes VC6 emit its
  // adjustor's null guard - `test eax,eax / je` with an `xor ecx,ecx` on the
  // skipped arm - and the two constants stop folding: `lea eax,[ecx+0x21d0]`
  // then `+0x58` never becomes the image's one `add ecx, 0x2228`. Calling the
  // INHERITED method straight on the member names no pointer at all, so the
  // compiler folds the base offset into the receiver itself. Same-type
  // `dialogs()` costs nothing; `pop->dialogs().sprite(...)`,
  // `popup.dialogs().edit(...)` and `dialogs_.set_state_flag(...)` are the
  // three former call sites.
  Dialogs &dialogs() {
    return dialogs_;
  }

 private:
  // THE TWO 0x118 TEXT BLOCKS at 0x2E64 and 0x2F7C, typed 2026-09-04. These
  // 140 dwords used to be spelled individually; the runs below replace them.
  // Evidence (all of it from the two bodies that reach the region, neither yet
  // recovered - src/unrecovered/00601bf0.cpp and 00600f00.cpp):
  //
  //   * BasePop::start (0x00601BF0) walks BOTH blocks with `idx * 0x118`,
  //     writing a STRING at block +0 (`*dst = 0; strcat(dst, v)`) and numbers
  //     at block +0x104, +0x108, +0x10C, +0x110 and +0x114.
  //   * BasePop::close (0x00600F00) restores both from the file-backed
  //     defaults 70 dwords at a time - the same 0x118 the ctor memcpy's.
  //   * BasePopDefaultBlock2E64 / BasePopDefaultBlock2F7C (basepop.cpp) are
  //     the 0x118 defaults, and member-accesses.csv records nothing else in
  //     the image touching either block.
  //
  // `text` is 0x104 because the first number sits at +0x104; nothing in the
  // image bounds what strcat may write into it, so the buffer is the size the
  // next field forces rather than a capacity the code checks. The five
  // numbers stay offset-named: start() parses each from its own token with
  // atoi/atol and nothing reads them back anywhere in the tree, so their
  // meanings are unknown and naming them would be invention.
  BasePopTextBlock text_block_a_;  // 0x2E64
  BasePopTextBlock text_block_b_;  // 0x2F7C
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
  // THE 27 DWORDS COPIED FROM BasePopDefaultBlock31B8 at 0x31B8, ONE writer
  // and ONE shape: the ctor's copy loop walks 0x9b8d10 + 4*column + 0xC*row
  // into 0x31b8 + 4*column + 0xC*row - the SAME index both sides, nine rows
  // of three dwords, 0x6C bytes ending at ok_text_. Typed 2026-09-04; the 27
  // offset names it replaces are this array. Read back by nothing in the tree
  // yet, so the rows stay flat rather than pretending to a [9][3] meaning.
  uint32_t default_block_[27];  // 0x31B8
  LPSTR ok_text_;
  LPSTR cancel_text_;
  uint32_t field_322C_;
};



// The screen width the high-resolution scaling switches on, read at a
// fixed address. Rebindable so tests can drive both sides of the branch.


static_assert(sizeof(BasePop) == 0x3230,
              "BasePop layout must match the original executable");



// Default string and button font slots at 0x009B8D98 and 0x009B8DA8; tests
// outside the hybrid process rebind them.
// ARRAYS, NOT POINTERS. Four string fonts at 0x009B8D98 and three button
// fonts at 0x009B8DA8; the image stores straight into them.
extern Font *BasePopDefaultStringFonts[4];  // 0x009B8D98
extern Font *BasePopDefaultButtonFonts[3];  // 0x009B8DA8


// Two interleaved default-colour tables. The string table carries four tiers
// at a 0x10 slot stride; the button table carries three at 0xC. Tests rebind
// both.
extern uint32_t BasePopDefaultStringColors[4][4];  // 0x00696EE4
extern uint32_t BasePopDefaultButtonColors[4][3];  // 0x00696F24



// Set when the fallout flag is raised, but only while the gate global is
// set. Real storage, defined in basepop.cpp; in the image these are the two
// zero-initialised dwords at 0x009B8D00 and 0x009B8CFC.
extern int BasePopFalloutGate;  // 0x009B8D00
extern int BasePopFalloutFlag;  // 0x009B8CFC

// The two singleton BasePop instances `init_class` allocates (basepop.cpp,
// which defines them) and `pops` (popup.cpp) chooses between: in the image
// these are the two zero-initialised pointer dwords at 0x009BC074/78. Real
// pointer variables here - one storage, one name - instead of the per-TU
// pointer bindings into the image's data that read and wrote unmapped
// memory in the standalone build.
extern Win *PopupInstanceSlotA;  // 0x009BC074
extern Win *PopupInstanceSlotB;  // 0x009BC078

// The two default button captions, heap copies the class owns. `init_class`
// seeds them from the literals "OK" and "Cancel"; the two setters below
// replace them.
extern char *BasePopDefaultOkText;      // 0x009B8D80
extern char *BasePopDefaultCancelText;  // 0x009B8D84

// Defined in basepop.cpp (0x009B8DD0, "copied to +0x322c" of the popup
// defaults). close_opening publishes the address of the Console's GraphicWin
// virtual base here on the way out of the opening, which is why the slot
// holds a window pointer at run time despite the popup-defaults name it
// carried before that call was recovered.
extern uint32_t BasePopDefaultField322C;  // 0x009B8DD0
