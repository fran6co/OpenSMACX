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
#include "stringstruct.h"

#include <cstddef>

class GraphicWin;


 /*
  * Dialog class
  */
class Dialog {
 public:
  friend class CheckBox;
  friend class RadioButton;
  // VIRTUAL, and this is the Dialog virtual ListBox overrides - which is why
  // ListBox carries a Dialog vtordisp and CheckBox/RadioButton/SpriteBox do
  // not. The evidence is independent of this file: filewin.h pins
  // `ListBox list_box_; // 0x286C, sizeof 0xB54, ends 0x33C0` with a field
  // immediately after, so ListBox is 0xB54 - four more than the 0xB50 it
  // measures with only the GraphicWin displacement.
  // `on_redraw` and `attach` are VIRTUAL below, and the image's own vtables are
  // why. Every class that virtually derives from Dialog installs a Dialog
  // vtable whose slots point back into that class through VTORDISP THUNKS -
  // ListBox's at 0x00670408 reaching 0x0060D010..0x0060D050, CheckBox's at
  // 0x0067059C reaching 0x0060FEB0..0x0060FEF0 - and each thunk opens
  // `sub ecx, dword ptr [ecx - N]`, which IS the vtordisp adjustment.
  //
  // Declaring them RESTORED FOUR LAYOUTS: CheckBox 0xB28, RadioButton 0xB24,
  // SpriteBox 0xB98 - three of them pins that were already in this tree before
  // being wrongly reduced - and EditGroup 0xB9C.
  //
  // TWO EARLIER NOTES HERE WERE WRONG AND ARE WITHDRAWN. One said the fix was
  // to delete the twelve hand-written thunk models and name the compiler's
  // `$4` thunks; there are none, because these classes install their vtable by
  // hand so VC6 emits no vtable. The next said that made the whole thing
  // unreachable until the vtable modelling changed. IT DID NOT: the thunks
  // simply DOUBLE-COUNTED. A model calling `((ListBox*)(object - vtordisp))
  // ->ListBox::on_redraw()` gets a compiler-emitted `add ecx, 0xa60` to reach
  // the Dialog subobject, one instruction the image's two-instruction thunk
  // lacks; handing it a pointer already 0xA60 low folds the pair away. Twelve
  // call sites, two shapes, every offset taken from its own class's layout.
  //
  // `item` is NOT one of these - ListBox::item (0x0060C920) and Dialogs::item
  // (0x00612A70) both open on an unadjusted receiver - and it is no longer
  // virtual.
  int item(char *text, int index);
  // THE VIRTUAL ORDER IS THE IMAGE'S, dumped from the ListBox Dialog-part
  // table 0x00670408 on 2026-09-01: slot 0 = the dtor, slot 1 = attach,
  // slot 2 = on_redraw. The declarations follow that order.
  // DECLARATION-ONLY (2026-09-02): ??1Dialog is called out-of-line 35 times
  // in the image - ??1DipEdit and ??1FileWin tear their Dialog virtual bases
  // down with `call ??1Dialog` - so the definition must not be inline-visible
  // in those TUs. Defined in dialog.cpp.
  virtual ~Dialog();
  virtual int attach(GraphicWin *a1, int a2, int a3, int a4) { return 0; }
  virtual void on_redraw() { ; }
  void attach_at_stored_position(GraphicWin *window, int flags);
  // 0x00608F50: reusable, non-virtual reset. The control families call this
  // directly; it clears entries before resetting their heap and settings.
  void close();
  // THE TWO SHOW-BIT SETTERS, proved by BasePop::exec (0x00602758..0x00602775):
  //   mov eax,[esi+0x21d0]          the embedded Dialogs' vbptr
  //   mov ecx,[eax+8]               the Dialog virtual base's displacement
  //   lea edi,[ecx+esi+0x21d0]      = &dialogs_.Dialog, i.e. this+0x2D70
  //   mov ecx,edi / push 1 / call 0x00605180
  //   mov ecx,edi / push 1 / call 0x006051A0
  // Both callees are the 27-byte `if (v) f |= bit; else f &= ~bit;` pair over
  // the Dialog style word at +0x20 that this tree claims as
  // ?UNK3@BasePop@@QAEXH@Z / ?UNK4@BasePop@@QAEXH@Z (0x00605180/0x006051A0,
  // both BYTE_EXACT in basepop.cpp). The receiver the image hands them is a
  // DIALOG subobject, not a BasePop, so the catalogue's owner is the folded
  // twin: the original had this pair on Dialog (or on Dialogs) and VC6's
  // identical-COMDAT folding merged it with BasePop's own identical body,
  // keeping one name.
  //
  // LANDED (2026-09-06, dialogs finisher): both are defined OUT OF LINE in
  //        dialog.cpp - an in-class body is two instructions VC6 expands at
  //        the call site, where the image makes a real `call rel32` - and
  //        BasePop::exec makes the two real calls. The claim stays with the
  //        folded twin in basepop.cpp; the receiver at the call site is bound
  //        ONCE (`Dialog &dialog = dialogs_;`) because two qualified
  //        recomputations of the vbtable walk cost exec 22 -> 15 agreeing
  //        instructions where the image holds the subobject in one register.
  void set_style_bit_0(int on);
  void set_style_bit_1(int on);
  // TRIED (2026-09-02, phase B): `virtual uint32_t close()` - the four
  // Dialog-part thunks in the image's ListBox table suggested close was a
  // Dialog virtual. REFUTED by the family closes' own bodies: the image's
  // closes open on an unadjusted receiver and walk the runtime vbtable
  // (`mov eax,[esi]; mov edx,[eax+4]`), which a Q-spelled plain member
  // called directly produces exactly - the virtual spelling enters via the
  // vtordisp and reaches the Dialog subobject through the direct
  // displacement instead, different semantics for the embedded cases and
  // different bytes (7/20, 9/37). The four-table decode belongs to
  // ListBox's dtor/attach/on_redraw/+1, not close.

 public:
  // Static default shared by every dialog; __cdecl in the original.
  static int set_def_dialog_font(Font *font1, Font *font2, Font *font3);
  // ??0Dialog@@QAE@XZ (0x00608C10), a real out-of-line constructor. The
  // implicit member machinery owns most of the image's body: heap_'s Heap
  // construction (its `lea ecx,[esi+4]; call 0x5d4560`), item_list_'s whole
  // StringList chain (the 0x66B0EC vbtable, the 0x6693AC/0x6693A4/0x6693A0
  // stages, the owner capture off 0x009B3374, the five field zeros, and the
  // derived 0x6698C4/0x6698C0 refresh), and the Dialog vftable store at
  // [this]. The body keeps the field initialization in the image's store
  // order - including the three -1 stores, which the image computes once
  // (or eax,-1) and stores to field_F0_, field_48_ and field_1C_ at three
  // separate points.
  Dialog();
  // The dtor is VIRTUAL (declared first above, matching the image's slot 0),
  // so VC6 emits the vfptr the image has at offset 0 instead of the
  // hand-modelled `LPVOID vtable_` this replaced. That vfptr is the 4 of the
  // 8 bytes `: virtual GraphicWin, virtual Dialog` came up short by on
  // CheckBox/RadioButton/EditGroup/ListBox/SpriteBox/Dialogs - see
  // radiobutton.h, which measured the shortfall and named this as the cause.
  // A `construct` METHOD is GONE (2026-09-03): it was the seam this header
  // needed while 0x00608C10 was still a pending forwarder, and the real
  // ??0Dialog@@QAE@XZ is homed and BYTE_EXACT in dialog.cpp - the forwarder
  // had no caller left, and a live one would have been a second definition
  // of the same construction.

  int set_dialog_font(Font *font1, Font *font2, Font *font3);
  void set_dialog_text_color(int color1, int color2, int color3, int color4);
  void set_dialog_text_color2(int color1, int color2, int color3, int color4);
  void set_dialog_text_color3(int color1, int color2, int color3, int color4);
  // MEASURED (see the definition at the end of this header): the image keeps
  // an out-of-line copy of id_to_pos at 0x00609AF0 AND inlines the same walk
  // into set_selected_id, which only the explicit `inline` earns - VC6 will
  // not auto-expand a function containing a loop.
  MEASURED int id_to_pos(int id);
  void set_selected_id(int id);
  int get_selected_id();
  int pos_to_id(int position);
  // Four of Dialog's seven init overloads; the four SpriteBox::init forwards
  // to. All unrecovered - declared so the recovered callers can name them.
  int init(RECT *group_id, int flags);  // ?init@Dialog@@QAEHPAURECT@@H@Z  0x00609470
  int init(int file_name);            // ?init@Dialog@@QAEHH@Z           0x006095F0
  int init(Heap *file_name);          // ?init@Dialog@@QAEHPAUHeap@@@Z   0x006098D0
  int init(int left, int top, int right, int bottom, int heap_size);
  int init(RECT *rect, Heap *heap);
  // The five-argument one, which the recovered SpriteBox::init (0x006104D0)
  // tail-forwards to. Its absence was the only thing stopping that body from
  // being integrated out of src/recovered/.
  int init(int a1, int a2, int a3, int a4, Heap *a5);
  // ?init@Dialog@@QAEHHHHHPAUHeap@@@Z  0x00609730
  //
  // Initializer seeds remain separate recovery work. StringStruct now owns
  // typed insertion/removal hooks and allocator-aware node deletion; close()
  // uses that cleanup without destroying the embedded list. The primary list
  // table has two hook slots, separate from the allocation-base destructor.

 private:
  // The vfptr is EMITTED by the compiler now - see the virtual destructor
  // above. It occupies the same four bytes at offset 0 that `LPVOID vtable_`
  // modelled, which is what keeps sizeof(Dialog) at 0xF4.
  Heap heap_;
  Heap *heap_ptr_;
  uint32_t field_1C_;  // 0x001c
 public:
  // field_20_, field_44_ and field_6C_ are PUBLIC for the same reason the
  // block near the bottom of the class is: the derived families write them
  // through their VIRTUAL Dialog base, and an access through a virtual base
  // can only be spelled from outside it. DipEdit::init's
  // `list_box_.field_20_ &= ~4` walks the ListBox vbtable to find the base
  // (0x004DB381..0x004DB397) and stores through the displacement it finds
  // there - no Dialog member function could emit that walk. Access is
  // compile-time only; the static_assert below still pins the layout.
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
 private:
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
 public:
  // See the note at selected_position_ below: ListBox re-arms this from
  // close() through its virtual Dialog base.
  uint32_t field_B4_;
 private:
  uint32_t field_B8_;
 public:
  // PUBLIC like selected_position_ below, and for the same reason: CheckBox's
  // two state accessors (set_state_id / get_state_id) walk it through their
  // virtual Dialog base by name. Access changes no offset; the static_assert
  // pins the layout.
  // THE EMBEDDED ITEM LIST, carved 2026-09-01 from what was field_BC_ ..
  // field_E8_: exactly 0x30 = sizeof(StringList) at 0xBC..0xEC - the two ABI
  // words at its front (the old field_BC_/field_C0_), the four list fields,
  // the allocator, StringList's own field_1C_/20_/24_, and the virtual base
  // (vptr + saved owner) at 0xE4/0xE8. The image's ??1Dialog tears this list
  // down with the two-stage derived close and the virtual-base publish -
  // which is ~StringList's own compiler-emitted member destruction chain.
  StringList item_list_;
  // PUBLIC, not private, with field_B4_ above: the derived families reach
  // these through their virtual Dialog base - ListBox re-arms field_B4_ in
  // close(), and CheckBox's four state-word accessors bit-work
  // selected_position_ (the dword doubles as the per-box state bits, which
  // is what the image's bytes do). Access changes no offset; the
  // static_assert pins the layout, as with StringStruct's own fields.
 public:
  int selected_position_;
  // 0xF0: the hover index the on_mouse_leave overrides clear (-1) through
  // their virtual Dialog base - same public reason as selected_position_.
  uint32_t field_F0_;
};

static_assert(sizeof(Dialog) == 0xF4, "Dialog layout must match the original executable");



// Default dialog font slots at 0x009B8EC0; tests rebind this.
// AN ARRAY, NOT A POINTER: three fonts at 0x009B8EC0, stored into
// directly by `set_def_dialog_font`.
extern Font *DialogDefaultFonts[3];

// THE DIALOG DEFAULTS TABLE, the image's .data run 0x0069704C..0x006970DC
// (37 dwords, shipped values; index = (address - 0x0069704C) / 4):
// BaseButton's bevel set at [0..4], the dimmed "..." pair at [17]/[18], the
// twelve text-color slots at [24]..[35], and the geometry/flag defaults at
// [19]/[20]/[21]/[22]/[23]/[36] (the last is main.cpp's DialogDefaultStyle).
// NOT CONST: the image's set_def_dialog_text_color (0x00609D50, homed in
// dialog.cpp) writes the twelve text-color slots at runtime.
extern uint32_t DialogDefaults[37];

// 0x00609D50, homed in dialog.cpp. The catalogued name spells a __thiscall
// member; the body is a __cdecl free function over the defaults table, so
// the emitted name is the extern "C" one and the `// symbol` fact under the
// marker carries it.
extern "C" MEASURED void __cdecl dialog_set_def_dialog_text_color(
    int color_a, int color_b, int color_c, int color_d, int slot);

// The zero-init .bss slots the Dialog constructor reads its field defaults
// from (0x009B8EB4..0x009B8ED8), rebindable like ListBoxCloseDynamicDefault.
extern uint32_t DialogField30Default;  // 0x009B8EB4
extern uint32_t DialogField34Default;  // 0x009B8EB8
extern uint32_t DialogField40Default;  // 0x009B8EBC
extern uint32_t DialogFieldACDefault;  // 0x009B8ECC
extern uint32_t DialogFieldB0Default;  // 0x009B8ED0
extern uint32_t DialogFieldB4Default;  // 0x009B8ED4
extern uint32_t DialogFieldB8Default;  // 0x009B8ED8

// The four rebindable list-stage table constants left with the hand
// close_with_tables calls they fed: ~Dialog runs the compiler-owned
// item_list_ destructor chain, which stages the same tables (0x006698C4/
// 0x006698C0 derived, 0x006693A4/0x006693A0 base) from the StringList
// declarations. The virtual base's final table and the owner publish are
// ~StringAllocationBase's own body (stringstruct.cpp).

// ?init@Dialog@@QAEHH@Z (0x006095F0), the one-int init overload. Unrecovered;
// SpriteBox forwards to it, so its definition is a seam into the original.

// 0x00609AF0, MEASURED inline. The image INLINES this into set_selected_id
// (0x006099D0 is the same walk with the return value stored straight into
// selected_position_) and keeps an out-of-line copy at 0x00609AF0 for the
// same reason it keeps one for every other helper. VC6's automatic inline
// expansion (/Ob2) will not take a function containing a loop, so the
// `inline` keyword is what makes the call site expand - and MEASURED, which
// is __declspec(dllexport), is what pins the out-of-line COMDAT this body's
// own claim is measured against. The marker stays in dialog.cpp.

inline int Dialog::id_to_pos(int id) {
    if (item_list_.head_) {
        int count = item_list_.entry_count_;
        item_list_.current_position_ = 0;
        item_list_.current_ = item_list_.head_;
        for (int traversed = 0; traversed < count; traversed++) {
            StringStructEntry *entry = item_list_.current_;
            if (entry->id == id) {
                break;
            }
            item_list_.current_position_++;
            item_list_.current_ = entry->next;
        }
    }
    return item_list_.current_position_;
}

void __fastcall dialog_destructor_redirect(Dialog *self, void *);
