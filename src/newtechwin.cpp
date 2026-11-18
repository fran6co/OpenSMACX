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
// ORIGINAL: 0x00484A80 ??_GNewTechWin@@UAEPAXI@Z 0x00484A80-0x00484AA1 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical deleting-thunk source src/deleting_thunks.cpp; original full spans retained: 0x00484A80-0x00484AA1", "Pinned class constructor ??0NewTechWin@@QAE@XZ at 0x004849D0 stores original class/view table VA 0x66ce5c at 0x00484A5A (mov dword ptr [esi], 0x66ce5c); the current genuine owner vftable ??_7NewTechWin@@6BWin@@@ selects generated ??_GNewTechWin@@UAEPAXI@Z in slot 0 (+0x0) through weak entry ??_ENewTechWin@@UAEPAXI@Z (mode 2).", "Pinned ??_GNewTechWin@@UAEPAXI@Z at 0x00484A80 calls the source-mapped ??1NewTechWin@@QAE@XZ at 0x00484810, then tests byte ptr [ebp + 8], 1; je 0x484a9a bypasses scalar delete when bit 0 is clear, while bit 0 set calls 0x0064557F; it save original this; call destructor; delete only when flags bit 0 is set; return saved this with ret 4.", "The real class lifetime is owned by src/newtechwin.cpp; its compiler-emitted deleting symbol and vftable slot carry this original identity. Hand-written adapter bodies with live adjustor callers remain until those callers are independently qualified and transferred. The linked weak/COMDAT copy is not used to choose source ownership.", "the source writer preserves the historical mapping's exactness state (historical exactness obligation retained); current compiler extent is not asserted equal to the original span"], "reason": "Bind the original NewTechWin scalar deleting-destructor identity to the actual compiler-emitted class deleting destructor in its genuine owner translation unit. The original constructor's pinned vptr store and the current owner's physical table slot identify this class/view; no callback/thunk body is hand-authored here."}
// symbol    ??_GNewTechWin@@UAEPAXI@Z
// kind      game
// ORIGINAL: 0x00484AB0 ??_ENewTechWin@@WEEE@AEPAXI@Z 0x00484AB0-0x00484ABB FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor at 0x004849D0 installs Buffer table 0x0066CE54 at this+0x444; its first slot points to 0x00484AB0.", "Pinned entry 0x00484AB0 has exactly eleven bytes: SUB ECX,0x444 then JMP to already mapped primary deleting destructor 0x00484A80, followed by INT3 padding.", "Owner newtechwin.cpp physical COFF emits eleven-byte COMDAT ??_ENewTechWin@@WEEE@AEPAXI@Z; Buffer table ??_7NewTechWin@@6BBuffer@@@ selects it, and its REL32 targets weak ??_ENewTechWin@@UAEPAXI@Z with scalar ??_GNewTechWin@@UAEPAXI@Z fallback.", "The genuine GraphicWin:Win,Buffer hierarchy provides the receiver adjustment. Independent original constructor/table/body and physical COFF review supports this binding; local equality does not prove full linked table selection or runtime acceptance."], "reason": "Bind the original Buffer-view deleting receiver adjustment to the genuine NewTechWin compiler-emitted operation. The symbol names this tree emits, not an original debug name; no adapter body is authored."}
// ORIGINAL: 0x00483850 _$E21 0x00483850-0x00483866 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x682564 selects original initializer0x483850..0x483866, registering fini0x483870..0x48387a; complete independent decode establishes exclusive boundaries.", "Source-owned object?NewTechWindow@@3VNewTechWin@@A identifies original storage0x811e40. Startup arguments/calls: call 0x4849d0; push 0x483870. Cleanup receiver/calls: mov ecx, 0x811e40; jmp 0x484810. Constructor/destructor targets retain their existing class-owned source mappings.", "Fresh physical src/newtechwin.cpp COFF selects _$E21 through .CRT$XCU and registers _$E19, both referencing the genuine source global. Original NetMsg argument triples and implicit PlanWin/Console most-derived flags are preserved by the existing declarations; complete virtual-base teardown is compiler-owned, not authored offsets.", "Current local helper symbols interpret present emission, not original debug names or historical TU boundaries. Object starts do not prove allocation extents. This binding establishes neither cross-TU global order, masked-reference correctness, strict linked preservation, dependency-provider equivalence nor runtime/game acceptance; excluded guarded sprite cleanup remains separate."], "reason": "Bind original startup identity to the genuine compiler-generated lifetime of ?NewTechWindow@@3VNewTechWin@@A in its actual source owner; no manual lifecycle callback is authored."}
// symbol    _$E15
// ORIGINAL: 0x00483870 _$E19 0x00483870-0x0048387A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x682564 selects original initializer0x483850..0x483866, registering fini0x483870..0x48387a; complete independent decode establishes exclusive boundaries.", "Source-owned object?NewTechWindow@@3VNewTechWin@@A identifies original storage0x811e40. Startup arguments/calls: call 0x4849d0; push 0x483870. Cleanup receiver/calls: mov ecx, 0x811e40; jmp 0x484810. Constructor/destructor targets retain their existing class-owned source mappings.", "Fresh physical src/newtechwin.cpp COFF selects _$E21 through .CRT$XCU and registers _$E19, both referencing the genuine source global. Original NetMsg argument triples and implicit PlanWin/Console most-derived flags are preserved by the existing declarations; complete virtual-base teardown is compiler-owned, not authored offsets.", "Current local helper symbols interpret present emission, not original debug names or historical TU boundaries. Object starts do not prove allocation extents. This binding establishes neither cross-TU global order, masked-reference correctness, strict linked preservation, dependency-provider equivalence nor runtime/game acceptance; excluded guarded sprite cleanup remains separate."], "reason": "Bind original exit identity to the genuine compiler-generated lifetime of ?NewTechWindow@@3VNewTechWin@@A in its actual source owner; no manual lifecycle callback is authored."}
// symbol    _$E13
// ORIGINAL: 0x00483880 sub_483880 0x00483880-0x004838A4 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned0x00483880..0x004838A4 constructs six stride0x2C Sprites at0x00822610..0x00822718 using forward construction with prefix unwind, then registers0x004838B0..0x004838D8. Cleanup publishes mask1 of byte0x00822604 before reverse vector destruction; constructors do not reset it.", "NewTechWin::technology_sprites_[6] is ordinary class-static storage, defined after the existing NewTechWin singleton. Physical compiler output supplies the vector constructor/destructor iterators, matching guard, and registration edge. Runtime consumers access the same six elements directly; no value initialization or authored lifetime wrapper is introduced.", "Current compiler names are carried by the function symbol and object-target facts, not treated as historical debug names. Static members add no instance fields. Fresh strict measurement remains required; this binding transports no claim."], "reason": "Bind the pinned lifetime to typed class-static storage and compiler-generated initialization/destruction. Preserve original identity, spans, historical claims and guard/order obligations. Original nominal declarations and complete historical TU boundaries, linked data placement and game behavior remain unproved."}
// symbol    _$E21
// WORKAROUND(decl): implicit ~NewTechWin; see newtechwin.h.
#define NEWTECHWIN_IMPLICIT_DTOR
#define WIN_EXPAND_RECT_INLINE  // stand-in: see expand_rect in win.h
#include "stdafx.h"
#include "interface_fonts.h"
#include "newtechwin.h"
#include "game.h"
#include "alpha.h"
#include "console.h"
#include "datalink.h"
#include "faction.h"
#include "font.h"
#include "lock.h"
#include "maininterface.h"
#include "map.h"  // mandate_color
#include "strings.h"
#include "stringstruct.h"
#include "technology.h"
#include "win.h"
#include "wave.h"
#include "wave_device.h"
#include <cstring>

extern char *DatalinkBlurbFile;  // 0x00691B04, owned by datalink.cpp

// Pending technology hyperlink selected by4838E0; exec483F90 consumes it
// after releasing its modal loop and resets it to-1.
// RECOVERY_OBJECT: v2 0x00686960 0x00686964 _PendingTechnologyLink 0x004838E0
static int PendingTechnologyLink = -1;
// Native callback reads the index written immediately before help dispatch.
// Initial storage is -1; this is independent of the adjacent pending link.
// RECOVERY_OBJECT: v2 0x00686964 0x00686968 _CurrentTechnologyBoxIndex 0x00483920
static int CurrentTechnologyBoxIndex = -1;

// Native initializer483850 and technology caller5BBA1D use receiver811E40.
// The final embedded StringBox at+DC24 ends+107C4; lifetime is ordinary C++.
// Initial storage binding does not establish complete CRT registration order.
// RECOVERY_OBJECT: v2 0x00811E40 0x00822604 ?NewTechWindow@@3VNewTechWin@@A 0x004849D0
NewTechWin NewTechWindow;

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00483920 ?info@NewTechWin@@QAAXPAD@Z 0x00483920-0x00483950 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original callback reads686964 and addresses811E40+2DA4+index*2BA0; exec484434 publishes the index before help dispatch"], "reason": "Genuine cdecl callback appends text to the current embedded technology box in the source-owned NewTech singleton."}
// symbol    ?newtech_info@@YAXPAD@Z
static void __cdecl newtech_info(char *text) {
    NewTechWindow.stringBoxes_[CurrentTechnologyBoxIndex].add_special(text);
}

void __cdecl help_tech_info(void (__cdecl *append)(char *), int technology,
                            int faction, int box_index);

// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// TRIED: real `FlatButton flatButtons_[3]`/`StringBox stringBoxes_[4]` array members (the scaffold's own layout) - with no dtor declared on FlatButton/StringBox, MSVC synthesizes its OWN local `??_H@` vector-constructor-iterator COMDAT for the array member init that precedes the body of ANY constructor for that class, which is a SECOND external .text symbol and makes `verify_recovered_function.py`'s `object_code()` raise an uncaught ValueError ("expected one external .text symbol, found 2") instead of reporting NO_COMPILE - crashes --dir and --body scoring outright. Sidestepped the same way the sibling destructor (0x00484810) does: NewTechWin here is its OWN flat class (no real inheritance, byte-array "members"), and every subobject is constructed EXPLICITLY via placement `new` / direct calls, so nothing is ever implicitly member-initialized.
// TRIED: reproducing the `push -1/push handler/mov fs:[0]` SEH frame and the `ebp-4` progressive construction-state byte - that is MSVC's own codegen for base/member initializer-list unwind, which explicit placement-new statements in an ordinary function body do not trigger; landed without it.
// ORIGINAL: 0x004849D0 ??0NewTechWin@@QAE@XZ 0x004849D0-0x00484A78;0x006575D0-0x00657626 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/004849d0.cpp retained primary and unwind spans; pinned instructions construct3FlatButtons and4StringBoxes before final StringBox"], "reason": "Ordinary GraphicWin inheritance and embedded FlatButton/ButtonGroup/StringBox members reproduce the original construction graph without duplicate placement construction."}
NewTechWin::NewTechWin() {
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00484810 ??1NewTechWin@@QAE@XZ 0x00484810-0x004849CD;0x00657510-0x006575C9 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00484810.cpp preserved lifecycle and unwind spans; real embedded arrays own destruction"], "reason": "Ordinary C++ reverse member/base destruction replaces the original-address pending forwarder; compiler owns unwind support."}
// symbol    ??1NewTechWin@@UAE@XZ
// The destructor is compiler-generated: members in reverse order, then GraphicWin.

// ORIGINAL: 0x004838E0 ?on_link_clicked@NewTechWin@@QAEHHPAD@Z 0x004838E0-0x0048391F BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original table66CE5C selects4838E0 in slot88. Full body checks IsMultiplayerNet93F660 and nonnull text; atoi645DA7 result is stored686960 before virtual release_modal58; returns0 only for admitted links, otherwise1.", "PendingTechnologyLink owns initial-1 DWORD686960. Native exec48479F/4847CE consumes/resets it; the full source-owned exec now models that flow while the original seed and historical restoration obligation remain preserved."], "reason": "Recover original GraphicWin link-hook overrides through the genuine slot88 hierarchy, preserving seed identity and historical obligations."}
// symbol    ?on_link_clicked@NewTechWin@@UAEHHPAD@Z
int NewTechWin::on_link_clicked(int, char *link) {
    if (IsMultiplayerNet == 0 && link != 0) {
        PendingTechnologyLink = atoi(link);
        release_modal();
        return 0;
    }
    return 1;
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00483E90 ?on_left_click@NewTechWin@@QAEXHH@Z 0x00483E90-0x00483E93 BYTE_EXACT
// symbol    ?on_left_click@NewTechWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_left_click@NewTechWin@@QAEXHH@Z)(NewTechWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void NewTechWin::on_left_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 1.
// ORIGINAL: 0x00483EA0 ?on_key_click@NewTechWin@@QAEHHH@Z 0x00483EA0-0x00483EA8 BYTE_EXACT
// symbol    ?on_key_click@NewTechWin@@UAEHHH@Z
// size      8 bytes
// prototype int (__thiscall ?on_key_click@NewTechWin@@QAEHHH@Z)(NewTechWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 1, always
Status: Complete
*/
int NewTechWin::on_key_click(int, int) {
    return 1;
}

// Include in the real owner: game.h, alpha.h, strings.h, technology.h,
// font.h, map.h, console.h and buffer.h are current owners of these APIs.
extern BoxSpriteParams InterfaceStandardPopupBoxStyle;
extern Sprite g_TECH_ICON_SPRITES[MaxTechnologyNum];
// The Transcendent Thought separator is the shared space object
// BufferJoinSpace (0x00682820, buffer.h), not a unit-owned copy.

// WORKAROUND(decl): 0x00483FB3..483FF2 loads all four source edges through a
// copy of the source pointer, then +n/-n into the destination.
static inline void newtechwin_inset_copy(RECT *dst, const RECT *src, int amount) {
    dst->left = src->left + amount;
    dst->top = src->top + amount;
    dst->right = src->right - amount;
    dst->bottom = src->bottom - amount;
}

// ORIGINAL: 0x00483950 ?on_redraw@NewTechWin@@QAEXXZ 0x00483950-0x00483E7F
// RECOVERY_BINDING: v1 {"reason":"Recover the complete technology popup redraw in its real NewTechWin owner.","evidence":["Pinned NewTechWin table 0x66CE5C slot 12 selects 0x483950.","Current NewTechWin rectangle words are written by exec; existing label, technology, player-data, buffer, style and sprite owners supply every draw.","The previous unit's NO_COMPILE finding was its fake standalone class/undefined Buffer scaffold; this candidate instead uses newtechwin.h's genuine GraphicWin hierarchy and embedded rectangle words."]}
// kind      game
// symbol    ?on_redraw@NewTechWin@@UAEXXZ
void NewTechWin::on_redraw() {
    const int faction_snapshot = ConsoleGlobal.faction_id_;
    this->clear_links();

    RECT scratch = *buffer_working_rect();
    this->box_sprite(&scratch, &InterfaceStandardPopupBoxStyle);

    this->box_sprite(&main_rect_, &InterfaceBoxStyles[3]);

    RECT *rect = &ok_button_rect_;
    scratch.left = rect->left - 3;
    scratch.top = rect->top - 3;
    scratch.right = rect->right + 3;
    scratch.bottom = rect->bottom + 3;
    this->box_sprite(&scratch, &InterfaceBoxStyles[3]);

    this->set_font(find_font(0x10, 1), 0, 0, 0);
    this->set_text_color(0xE1, -1, 1, 1);

    this->box_sprite(&title_rect_, &InterfaceBoxStyles[0]);

    StringTemp[0] = '\0';
    strcat(StringTemp, StringTable.get(static_cast<int>(field_B28_) <= 0
                                           ? Labels.strings_ptr[541]
                                           : Labels.strings_ptr[542]));
    CharUpperA(StringTemp);
    if (StringTemp != 0) {
        this->write_cent_l(StringTemp, &title_rect_,
                             static_cast<unsigned int>(strlen(StringTemp)));
    }

    this->box_sprite(&label_rects_[0], &InterfaceBoxStyles[6]);
    StringTemp[0] = '\0';
    strcat(StringTemp, label_get(0x110 / 4));
    CharUpperA(StringTemp);
    if (StringTemp != 0) {
        this->write_cent_l(StringTemp, &label_rects_[0],
                             static_cast<unsigned int>(strlen(StringTemp)));
    }

    this->box_sprite(&label_rects_[1], &InterfaceBoxStyles[6]);
    StringTemp[0] = '\0';
    strcat(StringTemp, label_get(0x130 / 4));
    CharUpperA(StringTemp);
    if (StringTemp != 0) {
        this->write_cent_l(StringTemp, &label_rects_[1],
                             static_cast<unsigned int>(strlen(StringTemp)));
    }

    this->box_sprite(&label_rects_[2], &InterfaceBoxStyles[6]);
    StringTemp[0] = '\0';
    strcat(StringTemp, label_get(0xEF0 / 4));
    CharUpperA(StringTemp);
    if (StringTemp != 0) {
        this->write_cent_l(StringTemp, &label_rects_[2],
                             static_cast<unsigned int>(strlen(StringTemp)));
    }

    this->box_sprite(&label_rects_[3], &InterfaceBoxStyles[6]);
    StringTemp[0] = '\0';
    strcat(StringTemp, label_get(0xEF4 / 4));
    CharUpperA(StringTemp);
    if (StringTemp != 0) {
        this->write_cent_l(StringTemp, &label_rects_[3],
                             static_cast<unsigned int>(strlen(StringTemp)));
    }

    this->box_sprite(&row_rects_[0], &InterfaceBoxStyles[0]);
    this->box_sprite(&row_rects_[1], &InterfaceBoxStyles[0]);
    this->box_sprite(&row_rects_[2], &InterfaceBoxStyles[0]);
    this->box_sprite(&row_rects_[3], &InterfaceBoxStyles[0]);

    rect = &category_rect_;
    this->box_sprite(rect, &InterfaceBoxStyles[0]);

    this->set_font(find_font(0x28, 0), 0, 0, 0);
    this->set_text_color(mandate_color(tech_category(static_cast<int>(field_B24_))),
                         -1, 1, 1);

    scratch = *rect;
    scratch.left += 3;
    scratch.right -= 3;
    scratch.top += 3;
    scratch.bottom -= 3;
    scratch.bottom = this->text_line_height() + scratch.top;

    StringTemp[0] = '\0';
    say_tech(StringTemp, static_cast<int>(field_B24_), 0);
    if (static_cast<int>(field_B24_) == TECH_TRANT) {
        // PlayersData+0x368 is theory_of_everything; the neutral member name
        // and exact offset are already independently established in
        // faction.h and consumed by picktech/faction/reportif.
        strcat(StringTemp, BufferJoinSpace);
        char number[80];
        _itoa(PlayersData[faction_snapshot].theory_of_everything,
              number, 10);
        strcat(StringTemp, number);
    }
    if (StringTemp != 0) {
        this->write_cent_l(StringTemp, &scratch,
                             static_cast<unsigned int>(strlen(StringTemp)));
    }

    scratch = *rect;
    scratch.top += this->text_line_height() + 3;
    scratch.bottom -= 3;
    const int image_height = scratch.bottom - scratch.top;
    const int image_width = scratch.right - scratch.left;
    g_TECH_ICON_SPRITES[static_cast<int>(field_B24_)].draw_mono(this,
                         scratch.left + (image_width - 0xA0) / 2,
                         scratch.top + (image_height - 0xA0) / 2,
                         mandate_color(tech_category(static_cast<int>(field_B24_))),
                         0xA0, 0xD7);

    this->box_sprite(&bottom_rect_, &InterfaceBoxStyles[0]);
}

// WORKAROUND(type): 0x00483FA7..483FC9 stores the four bounds constants
// through a pointer copy and 0x00483FD0.. reads them back through another
// instead of folding the constants: the original store and the RECT read use
// different types (a RECT-typed setter folds them). This word-typed setter
// reproduces the unfolded store/reload.
static inline void newtechwin_set_words(uint32_t *rect, int left, int top, int right,
                                        int bottom) {
    rect[0] = left;
    rect[1] = top;
    rect[2] = right;
    rect[3] = bottom;
}

// WORKAROUND(inline): 0x00484783..4847A7 is a non-rotated loop whose head
// runs the modal pass, hide and redraw before the -1 test, the shape of a
// break test on an inline helper (VC6 rotates every other loop form).
static inline int newtechwin_run_modal(NewTechWin *win) {
    win->set_modal(1, 0, 0);
    win->hide();
    do_all_draws();
    return PendingTechnologyLink;
}
// ORIGINAL: 0x00483F90 ?exec@NewTechWin@@QAEXHH@Z 0x00483F90-0x0048480F
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/00483f90.cpp retained full original extent; independent pinned geometry review and actual-source controls in recovery/tests/test_newtechwin_exec_behavior.py"], "reason": "Technology popup uses real embedded controls, callback receiver, shared data owners, audio and modal hyperlink dispatch."}
// TRIED: literal transcription of the RECT-cascade prologue (traced every register's concrete value through ~150 straight-line instrs) plus the loop-over-4-StringBox body using a LOCAL scratch array (the original inset math lives on the stack, not the object's fields - the loop never writes field_AA4_ etc back). Named scaffold members (flatButtons_[3], buttonGroup_, stringBoxes_[4], stringBox2_) used directly instead of raw offsets where the class already typed them. Compiles; not chased to byte-exact given size (2175 bytes).
void NewTechWin::exec(int technology_id, int other_faction_id) {
    field_B28_ = static_cast<uint32_t>(other_faction_id);
    field_B24_ = static_cast<uint32_t>(technology_id);
    char number[16];
    RECT boxes[4];
    RECT strip;
    RECT rect;
    uint32_t category_id;
    RECT *const bounds = &field_A14_rect_;
    newtechwin_set_words(reinterpret_cast<uint32_t *>(&field_A14_rect_), 0, 0, 0x226, 0x258);
    newtechwin_inset_copy(&rect, bounds, 12);
    strip = rect;
    strip.left += 3;
    strip.right -= 3;
    strip.bottom -= 3;
    strip.top = strip.bottom - 0x14;
    RECT *const button = &ok_button_rect_;
    *button = strip;
    // 0x0048401E..0x0048403B: main_rect_.bottom is strip.top - 7 (EBP-0x20
    // reloaded, add -7, stored at +0xA30); the copy follows the update.
    rect.bottom = strip.top - 7;
    main_rect_ = rect;

    RECT inner;
    newtechwin_inset_copy(&inner, &rect, 3);
    title_rect_ = inner;
    title_rect_.bottom = title_rect_.top + 0x17;
    category_rect_ = inner;
    category_rect_.top = title_rect_.bottom + 3;
    category_rect_.bottom = category_rect_.top + 0xd9;
    label_rects_[0] = inner;
    label_rects_[0].top = category_rect_.bottom + 3;
    label_rects_[0].bottom = label_rects_[0].top + 0x14;
    row_rects_[0] = inner;
    row_rects_[0].top = label_rects_[0].bottom + 3;
    row_rects_[0].bottom = row_rects_[0].top + 0x4c;
    label_rects_[2] = inner;
    label_rects_[2].top = row_rects_[0].bottom + 3;
    label_rects_[2].bottom = label_rects_[2].top + 0x14;
    row_rects_[2] = inner;
    row_rects_[2].top = label_rects_[2].bottom + 3;
    row_rects_[2].bottom = row_rects_[2].top + 0x4c;
    bottom_rect_ = inner;
    bottom_rect_.top = row_rects_[2].bottom + 3;
    bottom_rect_.bottom = main_rect_.bottom - 3;

    label_rects_[1] = label_rects_[0];
    const int split = label_rects_[0].left +
                      (label_rects_[0].right - label_rects_[0].left + 3) / 2 - 3;
    label_rects_[0].right = split;
    const int split_right = split + 3;
    label_rects_[1].left = split_right;
    row_rects_[1] = row_rects_[0];
    row_rects_[0].right = split;
    row_rects_[1].left = split_right;
    label_rects_[3] = label_rects_[2];
    label_rects_[2].right = split;
    label_rects_[3].left = split_right;
    row_rects_[3] = row_rects_[2];
    row_rects_[2].right = split;
    row_rects_[3].left = split_right;

    RECT *const next_button = &next_button_rect_;
    RECT *const prev_button = &prev_button_rect_;
    RECT *const text_rect = &text_rect_;
    rect = main_rect_;
    rect.right -= 6;
    rect.bottom -= 6;
    rect.top = rect.bottom - 0x22;
    rect.left = rect.right - 0x1c;
    *next_button = rect;
    offset_rect(&rect, -0x1f, 0);
    *prev_button = rect;
    *text_rect = bottom_rect_;
    text_rect->right = prev_button->left - 4;

    init(bounds, 0, 0x05000000, 0, 0, 0);
    fill(9);
    field_16C_ = 0x1a;
    center();

    boxes[0] = row_rects_[0];
    boxes[1] = row_rects_[1];
    boxes[2] = row_rects_[2];
    boxes[3] = row_rects_[3];
    for (int box_index = 0; box_index < 4; ++box_index) {
        RECT *const rect = &boxes[box_index];
        expand_rect(rect, -4, -4);
        StringBox &box = stringBoxes_[box_index];
        box.init(rect->left, rect->top, rect->right - rect->left,
                 rect->bottom - rect->top, this, 0, -1);
        box.Buffer::set_font(find_font(0xe, 0), find_font(0xe, 1),
                             find_font(0xe, 2),
                             find_font(0xe, IsMultiplayerNet != 0 ? 2 : 3));
        box.Buffer::set_text_color(0xe1, -1, 1, 1);
        CurrentTechnologyBoxIndex = box_index;
        help_tech_info(newtech_info, static_cast<int>(field_B24_),
                       ConsoleGlobal.faction_id_, box_index);
        box.show(0);
    }

    newtechwin_inset_copy(&rect, text_rect, 3);
    stringBox2_.init(rect.left, rect.top, rect.right - rect.left,
                     rect.bottom - rect.top, this, 0, -1);
    stringBox2_.Buffer::set_font(find_font(0xc, 0), find_font(0xc, 1),
                                 find_font(0xc, 2), 0);
    stringBox2_.Buffer::set_text_color(0x98, -1, 1, 1);
    StringTemp[0] = '\0';
    strcat(StringTemp, "TECH");
    _itoa(static_cast<int>(field_B24_), number, 10);
    strcat(StringTemp, number);
    stringBox2_.strings().load(DatalinkBlurbFile, StringTemp, 1, 0);
    stringBox2_.add_fixup();
    stringBox2_.show(0);

    FlatButton &ok_button = flatButtons_[0];
    ok_button.init(label_get(0x194 / 4), -1, ok_button_rect_.left,
                   ok_button_rect_.top,
                   ok_button_rect_.right - ok_button_rect_.left,
                   ok_button_rect_.bottom - ok_button_rect_.top,
                   this, 0);
    ok_button.set_border_styles(&InterfaceBoxStyles[27], &InterfaceBoxStyles[28],
                                &InterfaceBoxStyles[29]);
    ok_button.Buffer::set_font(MainInterfaceGlobal.primary_font(), 0, 0, 0);
    ok_button.redraw();
    ok_button.redraw();

    buttonGroup_.init(0, 0);
    flatButtons_[1].init(0, 1, prev_button->left, prev_button->top,
                         prev_button->right - prev_button->left,
                         prev_button->bottom - prev_button->top, this, 0);
    flatButtons_[1].set_report_icons(&technology_sprites_[0],
        &technology_sprites_[1], &technology_sprites_[2]);
    flatButtons_[1].redraw();
    flatButtons_[2].init(0, 2, next_button->left, next_button->top,
                         next_button->right - next_button->left,
                         next_button->bottom - next_button->top, this, 0);
    flatButtons_[2].set_report_icons(&technology_sprites_[3],
        &technology_sprites_[4], &technology_sprites_[5]);
    flatButtons_[2].redraw();
    buttonGroup_.add(&flatButtons_[1]);
    buttonGroup_.add(&flatButtons_[2]);
    buttonGroup_.set(1, 1);

    show(0);
    do_all_draws();

    StringTemp[0] = '\0';
    strcat(StringTemp, "voices\\tech");
    _itoa(static_cast<int>(field_B24_), number, 10);
    strcat(StringTemp, number);
    strcat(StringTemp, ".mp3");
    if ((GameDataStateStorage.fields_.game_more_preferences_ & MPREF_AV_VOICEOVER_TECH_FAC) != 0 &&
        is_human(ConsoleGlobal.faction_id_)) {
        g_WAVE_GENERAL.unload();
        g_WAVE_GENERAL.load(StringTemp);
        g_WAVE_GENERAL.set_volume(WaveDeviceGlobal.get_group_volume(0));
        g_WAVE_GENERAL.play();
    }

    while (1) {
        if (newtechwin_run_modal(this) == -1) {
            break;
        }
        int entry_id;
        DatalinkMain.parse_id(PendingTechnologyLink, &category_id, &entry_id);
        DatalinkMain.exec(category_id, entry_id);
        PendingTechnologyLink = -1;
    }

    if ((GameDataStateStorage.fields_.game_more_preferences_ & MPREF_AV_VOICEOVER_TECH_FAC) != 0 &&
        (GameDataStateStorage.fields_.game_more_preferences_ & MPREF_AV_VOICEOVER_STOP_CLOSE_POPUP) != 0) {
        g_WAVE_GENERAL.fade(0x7d0);
    }
    close();
    do_all_draws();
}

// Original initializer 0x483880 constructs six Sprites at stride 0x2C;
// initial storage is zero. Registered teardown 0x4838B0 tests/sets mask0x01 of
// the separate byte0x822604 before reverse array destruction. Keep that byte
// outside both the NewTechWindow singleton and the array's bounded storage.
// ORIGINAL: 0x004838B0 ??__Fg_NEWTECHWIN_SPRITES@@YAXXZ 0x004838B0-0x004838D8 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned0x00483880..0x004838A4 constructs six stride0x2C Sprites at0x00822610..0x00822718 using forward construction with prefix unwind, then registers0x004838B0..0x004838D8. Cleanup publishes mask1 of byte0x00822604 before reverse vector destruction; constructors do not reset it.", "NewTechWin::technology_sprites_[6] is ordinary class-static storage, defined after the existing NewTechWin singleton. Physical compiler output supplies the vector constructor/destructor iterators, matching guard, and registration edge. Runtime consumers access the same six elements directly; no value initialization or authored lifetime wrapper is introduced.", "Current compiler names are carried by the function symbol and object-target facts, not treated as historical debug names. Static members add no instance fields. Fresh strict measurement remains required; this binding transports no claim."], "reason": "Bind the pinned lifetime to typed class-static storage and compiler-generated initialization/destruction. Preserve original identity, spans, historical claims and guard/order obligations. Original nominal declarations and complete historical TU boundaries, linked data placement and game behavior remain unproved."}
// symbol    _$E18
// RECOVERY_OBJECT: v4 0x00822604 ?NewTechSpritesCleanupGuard@@3EA target _$S19
// RECOVERY_OBJECT: v4 0x00822610 0x00822718 ?g_NEWTECHWIN_SPRITES@@3PAVSprite@@A 0x00453A70 target ?technology_sprites_@NewTechWin@@2PAVSprite@@A
Sprite NewTechWin::technology_sprites_[6];

// Equal section prefixes have separate original storage for the two branches.
// RECOVERY_OBJECT: v2 0x00686968 0x0068696D _NewTechBlurbSectionPrefix 0x00483EB0
static const char NewTechBlurbSectionPrefix[] = "TECH";
// RECOVERY_OBJECT: v2 0x00686970 0x00686975 _NewTechLongSectionPrefix 0x00483EB0
static const char NewTechLongSectionPrefix[] = "TECH";

// ORIGINAL: 0x00483EB0 ?on_group_clicked@NewTechWin@@QAEXHH@Z 0x00483EB0-0x00483F8C BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Recover the complete body in its genuine C++ owner, with real embedded members, shared virtual bases, lifecycle and parent operations. Compiler-generated overrides/adjustors replace executable-address or vbtable stand-ins; no current matching claim is granted.","evidence":["git:391d42a9:src/unrecovered/00483eb0.cpp","image:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf","recovery/tasks/ui-eight-owner-coverage.json","Original entry 0x00483eb0; physical VC6 COFF emitted symbol ?on_group_clicked@NewTechWin@@QAEXHH@Z."]}
// symbol    ?on_group_clicked@NewTechWin@@UAEXHH@Z
// TRIED: `#pragma function(strcat)` - without it VC6 inlines strcat as a scasb/movs sequence; with it plus a switch(a2){case 1: case 2:} (matching the dec/je/dec/jne shape) this reaches 0.96 mnemonic similarity, MISMATCH only on the final load()-call arg pushes.
void NewTechWin::on_group_clicked(int, int group) {
    char number[80];
    switch (group) {
    case 1:
        StringTemp[0] = '\0';
        strcat(StringTemp, NewTechBlurbSectionPrefix);
        _itoa(static_cast<int>(field_B24_), number, 10);
        strcat(StringTemp, number);
        stringBox2_.clear();
        stringBox2_.strings().load(DatalinkBlurbFile, StringTemp, 1, 0);
        break;
    case 2:
        StringTemp[0] = '\0';
        strcat(StringTemp, NewTechLongSectionPrefix);
        _itoa(static_cast<int>(field_B24_), number, 10);
        strcat(StringTemp, number);
        stringBox2_.clear();
        stringBox2_.strings().load("techlongs", StringTemp, 1, 0);
        break;
    default:
        return;
    }
    stringBox2_.add_fixup();
}

/*
Purpose: ?on_button_clicked@NewTechWin@@QAEXH@Z - dispatch to slot 0xe8 of this object's OWN
         vtable with NO arguments: the original pushes nothing between the
         vtable load and the call, so the parameters this function declares are
         discarded and only the `ret` cleans them up.
// ORIGINAL: 0x00483E80 ?on_button_clicked@NewTechWin@@QAEXH@Z 0x00483E80-0x00483E8B BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned original own table 0x0066CE5C, pointer word at +0xB0, identifies original entry 0x00483E80", "source:src/win.h inherited virtual interface; actual emitted method ?on_button_clicked@NewTechWin@@UAEXH@Z in primary Win table ??_7NewTechWin@@6BWin@@@"], "reason": "The original own Win table binds this entry as virtual slot 44. Use genuine member dispatch, not an opaque redirect; retain the catalogued original spelling as historical evidence."}
// symbol    ?on_button_clicked@NewTechWin@@UAEXH@Z
// size      11 bytes
// prototype void (__thiscall ?on_button_clicked@NewTechWin@@QAEXH@Z)(NewTechWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00483E82
Return Value: n/a
Status: Complete
*/
void NewTechWin::on_button_clicked(int) {
  release_modal();
}
// RECOVERY_LITERAL: v1 0x00686978 0x00686982 $SG56706 0x00483EB0
// RECOVERY_GENERATED: v1 vtable 0x0066CE5C ? ??_7NewTechWin@@6BWin@@@ 0x004849D0
// RECOVERY_GENERATED: v1 vtable 0x0066CE54 ? ??_7NewTechWin@@6BBuffer@@@ 0x004849D0
