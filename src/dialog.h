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
#include "heap.h"

#include <cstddef>

struct DialogEntry {
  uint32_t vtable;
  int id;
  void *payload;
  DialogEntry *next;
  DialogEntry *previous;
  uint32_t secondary_vtable;
  void *heap;
};

static_assert(sizeof(DialogEntry) == 0x1C,
              "DialogEntry layout must match the original executable");
static_assert(offsetof(DialogEntry, id) == 0x4,
              "DialogEntry ID offset must match the original executable");
static_assert(offsetof(DialogEntry, next) == 0xC,
              "DialogEntry next offset must match the original executable");
static_assert(offsetof(DialogEntry, previous) == 0x10,
              "DialogEntry previous offset must match the original executable");

 /*
  * Dialog class
  */
class Dialog {
 public:
  // VIRTUAL, and this is the Dialog virtual ListBox overrides - which is why
  // ListBox carries a Dialog vtordisp and CheckBox/RadioButton/SpriteBox do
  // not. The evidence is independent of this file: filewin.h pins
  // `ListBox list_box_; // 0x286C, sizeof 0xB54, ends 0x33C0` with a field
  // immediately after, so ListBox is 0xB54 - four more than the 0xB50 it
  // measures with only the GraphicWin displacement.
  // TRIED, AND IT IS RIGHT BUT NOT YET AFFORDABLE. `on_redraw` and `attach`
  // ARE Dialog virtuals - the image's own vtables say so. Every class that
  // virtually derives from Dialog installs a Dialog vtable whose slots point
  // back into that class through VTORDISP THUNKS: ListBox's at 0x00670408
  // reaching 0x0060D010..0x0060D050, CheckBox's at 0x0067059C reaching
  // 0x0060FEB0..0x0060FEF0, and each thunk opens
  // `sub ecx, dword ptr [ecx - N]` then `sub ecx, <const>` - that first
  // instruction IS the vtordisp adjustment. adjustor_thunks.cpp already
  // catalogues them by name.
  //
  // Declaring both virtual here was measured 2026-08-22 and it RESTORES THE
  // SIZES: CheckBox 0xB24 -> 0xB28, RadioButton 0xB20 -> 0xB24, SpriteBox
  // 0xB94 -> 0xB98 - three of them back to the pins that were already in this
  // tree before they were "corrected" downward - and EditGroup to 0xB9C. All
  // nine byte-exact bodies in those classes STAY byte-exact, and Dialogs'
  // independent 0xC94 still holds.
  //
  // The cost is 12 REGRESSIONS in adjustor_thunks.cpp, and they are the reason
  // this is not landed. Those twelve are HAND-WRITTEN MODELS of the very
  // thunks the compiler now generates, so each emits one instruction too many:
  // the model does `sub ecx, [ecx-4]` itself and the compiler then adds its
  // own `add ecx, 0xa60`. The image's thunk is two instructions; ours becomes
  // three. The right end state is to delete the hand-written models and let
  // the markers name the compiler's `$4` thunks through a `// symbol` fact -
  // a coupled edit across twelve functions in one file, not a spelling fix.
  //
  // Until then `item` stays virtual because it PRODUCES the proven layout for
  // ListBox, and it is not the image's override: ListBox::item (0x0060C920)
  // and Dialogs::item (0x00612A70) both open on an unadjusted receiver.
  virtual int item(char *text, int index);
  // 0x00608F50, a pending_bodies forwarder. checkbox.cpp and
  // radiobutton.cpp both reach it, and both did so through a pointer.
  // VIRTUAL, and this is what earns the vtordisp before the Dialog base in
  // every class that virtually derives from it. VC6 emits a displacement only
  // where the derived class overrides a NON-DESTRUCTOR virtual of a virtual
  // base - measured 2026-08-22 on a minimal case: with only virtual
  // destructors it emits none, with a plain virtual override it emits one per
  // base. RadioButton::close() has the same `void ()` signature and so
  // overrides this.
  uint32_t close();

 public:
  // Static default shared by every dialog; __cdecl in the original.
  static int set_def_dialog_font(Font *font1, Font *font2, Font *font3);
  Dialog() { ; }
  // VIRTUAL, so VC6 emits the vfptr the image has at offset 0 instead of the
  // hand-modelled `LPVOID vtable_` this replaces. That vfptr is the 4 of the
  // 8 bytes `: virtual GraphicWin, virtual Dialog` came up short by on
  // CheckBox/RadioButton/EditGroup/ListBox/SpriteBox/Dialogs - see
  // radiobutton.h, which measured the shortfall and named this as the cause.
  virtual ~Dialog() { ; }
  // A `construct` METHOD, NOT A CONSTRUCTOR - the `Win::construct` idiom.
  // The real body (??0Dialog@@QAE@XZ, 0x00608C10) is not promoted yet;
  // classes that hold a Dialog as a virtual-base-shaped subobject (CheckBox,
  // RadioButton, EditGroup, SpriteBox) reach it by name so their own
  // constructors emit the image's `E8` here. Forwarded in pending_bodies.cpp.
  void construct();

  // The recovered complete-object destructor body (0x00608E10); the inline
  // ~Dialog stays trivial so classes embedding a Dialog keep their layout-only
  // semantics, exactly as Scroll::destroy does.
  void destroy();
  int set_dialog_font(Font *font1, Font *font2, Font *font3);
  void set_dialog_text_color(int color1, int color2, int color3, int color4);
  void set_dialog_text_color2(int color1, int color2, int color3, int color4);
  void set_dialog_text_color3(int color1, int color2, int color3, int color4);
  int id_to_pos(int id);
  void set_selected_id(int id);
  int get_selected_id();
  int pos_to_id(int position);
  // Four of Dialog's seven init overloads; the four SpriteBox::init forwards
  // to. All unrecovered - declared so the recovered callers can name them.
  int init(RECT *a1, int a2);  // ?init@Dialog@@QAEHPAURECT@@H@Z  0x00609470
  int init(int a1);            // ?init@Dialog@@QAEHH@Z           0x006095F0
  int init(Heap *a1);          // ?init@Dialog@@QAEHPAUHeap@@@Z   0x006098D0
  // The five-argument one, which the recovered SpriteBox::init (0x006104D0)
  // tail-forwards to. Its absence was the only thing stopping that body from
  // being integrated out of src/recovered/.
  int init(int a1, int a2, int a3, int a4, Heap *a5);
  // ?init@Dialog@@QAEHHHHHPAUHeap@@@Z  0x00609730

 private:
  // The vfptr is EMITTED by the compiler now - see the virtual destructor
  // above. It occupies the same four bytes at offset 0 that `LPVOID vtable_`
  // modelled, which is what keeps sizeof(Dialog) at 0xF4.
  Heap heap_;
  Heap *heap_ptr_;
  uint32_t field_1C_;
  uint32_t field_20_;
  uint32_t field_24_;
  uint32_t field_28_;
  uint32_t field_2C_;
  uint32_t field_30_;
  uint32_t field_34_;
  uint32_t field_38_;
  uint32_t field_3C_;
  uint32_t field_40_;
  uint32_t field_44_;
  uint32_t field_48_;
  uint32_t field_4C_;
  uint32_t field_50_;
  uint32_t field_54_;
  uint32_t field_58_;
  uint32_t field_5C_;
  uint32_t field_60_;
  uint32_t field_64_;
  uint32_t field_68_;
  uint32_t field_6C_;
  Font *font1_;
  Font *font2_;
  Font *font3_;
  uint32_t text_color_a_;
  uint32_t text_color_2a_;
  uint32_t text_color_3a_;
  uint32_t text_color_b_;
  uint32_t text_color_2b_;
  uint32_t text_color_3b_;
  uint32_t text_color_c_;
  uint32_t text_color_2c_;
  uint32_t text_color_3c_;
  uint32_t text_color_d_;
  uint32_t text_color_2d_;
  uint32_t text_color_3d_;
  uint32_t field_AC_;
  uint32_t field_B0_;
  uint32_t field_B4_;
  uint32_t field_B8_;
  uint32_t field_BC_;
  uint32_t field_C0_;
  DialogEntry *entry_head_;
  DialogEntry *current_entry_;
  int entry_count_;
  int entry_position_;
  uint32_t field_D4_;
  uint32_t field_D8_;
  uint32_t field_DC_;
  uint32_t field_E0_;
  uint32_t field_E4_;
  uint32_t field_E8_;
  int selected_position_;
  uint32_t field_F0_;
};

static_assert(sizeof(Dialog) == 0xF4, "Dialog layout must match the original executable");

int __fastcall dialog_set_font_redirect(
    Dialog *self, void *, Font *font1, Font *font2, Font *font3);
void __fastcall dialog_set_text_color_redirect(
    Dialog *self, void *, int color1, int color2, int color3, int color4);
void __fastcall dialog_set_text_color2_redirect(
    Dialog *self, void *, int color1, int color2, int color3, int color4);
void __fastcall dialog_set_text_color3_redirect(
    Dialog *self, void *, int color1, int color2, int color3, int color4);
int __fastcall dialog_id_to_pos_redirect(Dialog *self, void *, int id);
void __fastcall dialog_set_selected_id_redirect(Dialog *self, void *, int id);
int __fastcall dialog_get_selected_id_redirect(Dialog *self, void *);
int __fastcall dialog_pos_to_id_redirect(Dialog *self, void *, int position);

int __cdecl dialog_set_def_dialog_font_redirect(
    Font *font1, Font *font2, Font *font3);

// Default dialog font slots at 0x009B8EC0; tests rebind this.
// AN ARRAY, NOT A POINTER: three fonts at 0x009B8EC0, stored into
// directly by `set_def_dialog_font`.
extern Font *DialogDefaultFonts[3];

// Every teardown that reaches the still-original Dialog::close (0x00608F50)
// binds it through this signature; RadioButton/CheckBox/ListBox share it.
typedef void (OriginalObject::*func_dialog_close)();

// ~Dialog's own body reaches Dialog::close through a rebindable seam, and the
// scalar deleting destructor frees through the executable's operator delete.
extern func_dialog_close DialogOriginalClose;          // default 0x00608F50

// Virtual tables the destructor stages. The Dialog primary table and the
// list virtual base's final table are written but never dispatched, so they
// are fixed constants like Scroll's. The four list-stage tables ARE dispatched
// through - the embedded StringStruct walk reads the table installed at
// this+0xBC - so they are rebindable: outside the hybrid process the game
// addresses are unmapped and a leaf test must substitute a stand-in.
extern const uint32_t DialogPrimaryVtable;              // this+0x00 = 0x006703FC
extern uint32_t DialogListDerivedVtable;                // this+0xBC = 0x006698C4
extern uint32_t DialogListDerivedVirtualBaseVtable;     // this+0xE4 = 0x006698C0
extern uint32_t DialogListVtable;                       // this+0xBC = 0x006693A4
extern uint32_t DialogListVirtualBaseVtable;            // this+0xE4 = 0x006693A0
extern const uint32_t DialogVirtualBaseFinalVtable;     // this+0xE4 = 0x006693AC

// The list virtual base's context word is published here on teardown.
uint32_t *const DialogPublishedGlobal = (uint32_t *)0x009B3374;                 // 0x009B3374

// ?init@Dialog@@QAEHH@Z (0x006095F0), the one-int init overload. Unrecovered;
// SpriteBox forwards to it, so its definition is a seam into the original.

void __fastcall dialog_destructor_redirect(Dialog *self, void *);
void *__fastcall dialog_scalar_dtor_redirect(
    Dialog *self, void *, unsigned int mode);
