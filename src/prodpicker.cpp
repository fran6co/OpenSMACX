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
#include "stdafx.h"
#include "prodpicker.h"
#include "vector_teardown.h"
#include <cstring>

// Sprite and FlatButton are constructed in a loop through the CRT's own
// vector constructor iterator - see vector_teardown.h. The addresses are the
// two element types' own real constructor/destructor.
const void *const ProdPickerSpriteCtor = (const void *)0x005E37E0;
const void *const ProdPickerSpriteDtor = (const void *)0x00406850;
const void *const ProdPickerFlatButtonCtor = (const void *)0x00607CF0;
const void *const ProdPickerFlatButtonDtor = (const void *)0x00406880;

const uint32_t ProdPickerPrimaryVtable = 0x0066A250;
const uint32_t ProdPickerBufferVtable = 0x0066A248;

// The three 0x30-byte blocks at 0xA780/0xA7B0/0xA7E0 are unnamed: each
// installs a vtable pair through a computed adjustor offset (the second
// dword of the object at +4 is itself a small vbtable-shaped blob whose
// second entry is the byte offset to re-stamp), the same "construct a
// generic sub-object, then immediately overwrite its vtable with the real
// one" shape GraphicWin::construct()/FlatButton use, just for something this
// header does not otherwise model. Reproduced at the raw offset because
// nothing establishes what class it is.
static void prod_picker_unknown_a7_block(char *self, unsigned int offset) {
    uint32_t *const global_sequence = reinterpret_cast<uint32_t *>(0x009B3374);
    *reinterpret_cast<uint32_t *>(self + offset + 0x04) = 0x0066B0EC;
    *reinterpret_cast<uint32_t *>(self + offset + 0x28) = 0x006693AC;
    *reinterpret_cast<uint32_t *>(self + offset + 0x2C) = *global_sequence;
    *global_sequence = 0;

    uint32_t vtbl = *reinterpret_cast<uint32_t *>(self + offset + 0x04);
    *reinterpret_cast<uint32_t *>(self + offset) = 0x006693A4;
    int adj = *reinterpret_cast<int *>(reinterpret_cast<char *>(vtbl) + 4);
    *reinterpret_cast<uint32_t *>(self + offset + 0x04 + adj) = 0x006693A0;

    vtbl = *reinterpret_cast<uint32_t *>(self + offset + 0x04);
    *reinterpret_cast<uint32_t *>(self + offset) = 0x006698C4;
    *reinterpret_cast<uint32_t *>(self + offset + 0x08) = 0;
    *reinterpret_cast<uint32_t *>(self + offset + 0x0C) = 0;
    *reinterpret_cast<uint32_t *>(self + offset + 0x10) = 0;
    *reinterpret_cast<uint32_t *>(self + offset + 0x14) = 0;
    *reinterpret_cast<uint32_t *>(self + offset + 0x18) = 0;
    adj = *reinterpret_cast<int *>(reinterpret_cast<char *>(vtbl) + 4);
    *reinterpret_cast<uint32_t *>(self + offset + 0x04 + adj) = 0x006698C0;
}

/*
// ORIGINAL: 0x00492420 ??0ProdPicker@@QAE@XZ 0x00492420-0x004926AD;0x00658B70-0x00658C09
// TRIED: register allocation - compiled body agrees on the SEH prologue
//            (7/7) then diverges at the push ebx/esi/edi save order; VC6
//            chose a different callee-save order for this body's register
//            pressure. MISMATCH, 11/127 instructions agree. The three
//            0x30-byte "mystery" sub-objects at 0xA780/0xA7B0/0xA7E0 are
//            reproduced at their raw offset (prod_picker_unknown_a7_block) -
//            nothing in this header names what class they are, and the
//            per-block instruction SCHEDULING (which register holds the
//            vtable pointer, when it gets reloaded) varies between the three
//            in the image in a way plain repeated C++ does not reproduce.
// TRIED: real declared members (Sprite[3]/Font x4/Time/FlatButton[9]/
//            Scroll/Caviar) built implicitly instead of placement-new -
//            measured WORSE, 8/127 against this baseline's 11/127. This
//            constructor's image already emits `sub esp, 8` at instruction 7
//            (the placement-new-costs-a-null-guard lever's target elsewhere
//            in this batch does NOT apply here: this frame slot is already
//            reproduced by the explicit body), and real members instead cost
//            the push ebx/esi/edi order, so the placement-new form stands.
// size      806 bytes
// prototype void (__thiscall ??0ProdPicker@@QAE@XZ)(ProdPicker* this)
// callers   1   call targets   7
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005D4CF0 0x006051D0 0x006161D0 0x00616DA0 0x00618EA0 0x006456E4 0x006457C2
*/
// The construction chain is GraphicWin::construct(), not a base constructor
// call - GraphicWin's own constructor is the empty inline stub, and every
// sub-object below is placement-new'd explicitly, in the exact order the
// disassembly shows, because C++'s implicit member construction would run
// them all BEFORE this body - and GraphicWin::construct() has to be first.
ProdPicker::ProdPicker() {
    new (static_cast<GraphicWin *>(this)) GraphicWin();
    char *const self = reinterpret_cast<char *>(this);

    VectorCtorIterator(sprites_, 0x2C, 3, ProdPickerSpriteCtor, ProdPickerSpriteDtor);

    new (font1_) Font();
    new (font2_) Font();
    new (font3_) Font();
    new (font4_) Font();
    new (time_) Time();

    VectorCtorIterator(flatButtons_, 0xB4C, 9, ProdPickerFlatButtonCtor, ProdPickerFlatButtonDtor);

    new (scroll_) Scroll();
    new (caviar_) Caviar();

    prod_picker_unknown_a7_block(self, 0xA780);
    prod_picker_unknown_a7_block(self, 0xA7B0);
    prod_picker_unknown_a7_block(self, 0xA7E0);

    field_A830_ = 0x86;
    field_A834_ = 0x86;
    field_A14_ = 0;
    uint32_t *const object = reinterpret_cast<uint32_t *>(this);
    field_A838_ = 0x6C;
    field_A83C_ = 0x5E;
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 1.
// ORIGINAL: 0x00493C10 ?UNK1@ProdPicker@@QAEHH@Z 0x00493C10-0x00493C18 BYTE_EXACT
// size      8 bytes
// prototype int (__thiscall ?UNK1@ProdPicker@@QAEHH@Z)(ProdPicker* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 1, always
Status: Complete
*/
int ProdPicker::UNK1(int) {
    return 1;
}

int __fastcall prod_picker_unk1_redirect(ProdPicker *self, void *, int a1) {
    return self->UNK1(a1);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00493E90 ?on_mouse_move@ProdPicker@@QAEXHH@Z 0x00493E90-0x00493E93 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_mouse_move@ProdPicker@@QAEXHH@Z)(ProdPicker* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ProdPicker::on_mouse_move(int, int) {
}

void __fastcall prod_picker_on_mouse_move_redirect(ProdPicker *self, void *, int a1, int a2) {
    self->on_mouse_move(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00495160 ?UNK3@ProdPicker@@QAEXH@Z 0x00495160-0x00495163 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?UNK3@ProdPicker@@QAEXH@Z)(ProdPicker* this, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ProdPicker::UNK3(int) {
}

void __fastcall prod_picker_unk3_redirect(ProdPicker *self, void *, int a1) {
    self->UNK3(a1);
}

/*
Purpose: Close the picker - clear the field at 0xA14 that marks it live, then
         run the GraphicWin close it inherits.
// ORIGINAL: 0x004926B0 ?close@ProdPicker@@QAEXXZ 0x004926B0-0x004926BF BYTE_EXACT
// size      15 bytes
// prototype void (__thiscall ?close@ProdPicker@@QAEXXZ)(ProdPicker* this)
// callers   2   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ProdPicker::close() {
    field_A14_ = 0;
    GraphicWin::close();
}

void __fastcall prod_picker_close_redirect(ProdPicker *self, void *) {
    self->close();
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x004945C0 ?on_redraw_nc@ProdPicker@@QAEXPAURECT@@H@Z 0x004945C0-0x004945C3 BYTE_EXACT
// symbol    ?on_redraw_nc@ProdPicker@@QAEXPAUtagRECT@@H@Z
// size      3 bytes
// prototype void (__thiscall ?on_redraw_nc@ProdPicker@@QAEXPAURECT@@H@Z)(ProdPicker* this, RECT*, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ProdPicker::on_redraw_nc(RECT *, int) {
}

void __fastcall prod_picker_on_redraw_nc_redirect(
        ProdPicker *self, void *, RECT *a1, int a2) {
    self->on_redraw_nc(a1, a2);
}

/*
Purpose: Step the receiver back to the subobject ??_GProdPicker@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x00421970 ??_GProdPicker@@WEEE@AEPAXI@Z 0x00421970-0x0042197B BYTE_EXACT
// symbol    ??_EProdPicker@@WEEE@AEPAXI@Z
// CORRECTED from ??3ProdPicker@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x004213A0` into
//   ??_GProdPicker@@UAEPAXI@Z, which executes `ret 4`; no stack access
//   and the receiver stays in ECX. `WEEE@` re-demangles to
//   adjustor{1092} and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
