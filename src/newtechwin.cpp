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
// ORIGINAL: 0x00483850 _$E21 0x00483850-0x00483866 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x682564 selects original initializer0x483850..0x483866, registering fini0x483870..0x48387a; complete independent decode establishes exclusive boundaries.", "Source-owned object?NewTechWindow@@3VNewTechWin@@A identifies original storage0x811e40. Startup arguments/calls: call 0x4849d0; push 0x483870. Cleanup receiver/calls: mov ecx, 0x811e40; jmp 0x484810. Constructor/destructor targets retain their existing class-owned source mappings.", "Fresh physical src/newtechwin.cpp COFF selects _$E21 through .CRT$XCU and registers _$E19, both referencing the genuine source global. Original NetMsg argument triples and implicit PlanWin/Console most-derived flags are preserved by the existing declarations; complete virtual-base teardown is compiler-owned, not authored offsets.", "Current local helper symbols interpret present emission, not original debug names or historical TU boundaries. Object starts do not prove allocation extents. This binding establishes neither cross-TU global order, masked-reference correctness, strict linked preservation, dependency-provider equivalence nor runtime/game acceptance; excluded guarded sprite cleanup remains separate."], "reason": "Bind original startup identity to the genuine compiler-generated lifetime of ?NewTechWindow@@3VNewTechWin@@A in its actual source owner; no manual lifecycle callback is authored."}
// ORIGINAL: 0x00483870 _$E19 0x00483870-0x0048387A FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x682564 selects original initializer0x483850..0x483866, registering fini0x483870..0x48387a; complete independent decode establishes exclusive boundaries.", "Source-owned object?NewTechWindow@@3VNewTechWin@@A identifies original storage0x811e40. Startup arguments/calls: call 0x4849d0; push 0x483870. Cleanup receiver/calls: mov ecx, 0x811e40; jmp 0x484810. Constructor/destructor targets retain their existing class-owned source mappings.", "Fresh physical src/newtechwin.cpp COFF selects _$E21 through .CRT$XCU and registers _$E19, both referencing the genuine source global. Original NetMsg argument triples and implicit PlanWin/Console most-derived flags are preserved by the existing declarations; complete virtual-base teardown is compiler-owned, not authored offsets.", "Current local helper symbols interpret present emission, not original debug names or historical TU boundaries. Object starts do not prove allocation extents. This binding establishes neither cross-TU global order, masked-reference correctness, strict linked preservation, dependency-provider equivalence nor runtime/game acceptance; excluded guarded sprite cleanup remains separate."], "reason": "Bind original exit identity to the genuine compiler-generated lifetime of ?NewTechWindow@@3VNewTechWin@@A in its actual source owner; no manual lifecycle callback is authored."}
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
// ORIGINAL: 0x00484810 ??1NewTechWin@@QAE@XZ 0x00484810-0x004849CD;0x00657510-0x006575C9
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00484810.cpp preserved lifecycle and unwind spans; real embedded arrays own destruction"], "reason": "Ordinary C++ reverse member/base destruction replaces the original-address pending forwarder; compiler owns unwind support."}
// symbol    ??1NewTechWin@@UAE@XZ
NewTechWin::~NewTechWin() {
}

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

// ORIGINAL: 0x00483F90 ?exec@NewTechWin@@QAEXHH@Z 0x00483F90-0x0048480F
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00483f90.cpp retained full original extent; independent pinned geometry review and actual-source controls in recovery/tests/test_newtechwin_exec_behavior.py"], "reason": "Technology popup uses real embedded controls, callback receiver, shared data owners, audio and modal hyperlink dispatch."}
// TRIED: literal transcription of the RECT-cascade prologue (traced every register's concrete value through ~150 straight-line instrs) plus the loop-over-4-StringBox body using a LOCAL scratch array (the original inset math lives on the stack, not the object's fields - the loop never writes field_AA4_ etc back). Named scaffold members (flatButtons_[3], buttonGroup_, stringBoxes_[4], stringBox2_) used directly instead of raw offsets where the class already typed them. Compiles; not chased to byte-exact given size (2175 bytes).
// Include in the real owner: game.h, alpha.h, strings.h, technology.h,
// font.h, map.h, console.h and buffer.h are current owners of these APIs.
extern BoxSpriteParams InterfaceStandardPopupBoxStyle;
extern Sprite g_TECH_ICON_SPRITES[MaxTechnologyNum];
// The pinned image's null-terminated data at VA 0x00682820 is one ASCII
// space; this source-owned literal preserves the CENTEMP separator.
static const char NewTechCentempSuffix[] = " ";

// ORIGINAL: 0x00483950 ?on_redraw@NewTechWin@@QAEXXZ 0x00483950-0x00483E7F
// RECOVERY_BINDING: v1 {"reason":"Recover the complete technology popup redraw in its real NewTechWin owner.","evidence":["Pinned NewTechWin table 0x66CE5C slot 12 selects 0x483950.","Current NewTechWin rectangle words are written by exec; existing label, technology, player-data, buffer, style and sprite owners supply every draw.","The previous unit's NO_COMPILE finding was its fake standalone class/undefined Buffer scaffold; this candidate instead uses newtechwin.h's genuine GraphicWin hierarchy and embedded rectangle words."]}
// kind      game
// symbol    ?on_redraw@NewTechWin@@UAEXXZ
void NewTechWin::on_redraw() {
    const int faction_snapshot = ConsoleGlobal.faction_id_;
    Buffer *buffer = static_cast<Buffer *>(this);
    buffer->clear_links();

    RECT working = *buffer_working_rect();
    buffer->box_sprite(&working, &InterfaceStandardPopupBoxStyle);

    RECT main_area = {
        static_cast<LONG>(field_A24_), static_cast<LONG>(field_A28_),
        static_cast<LONG>(field_A2C_), static_cast<LONG>(field_A30_)
    };
    buffer->box_sprite(&main_area, &InterfaceBoxStyles[3]);

    RECT inset_area = {
        static_cast<LONG>(field_AF4_), static_cast<LONG>(field_AF8_),
        static_cast<LONG>(field_AFC_), static_cast<LONG>(field_B00_)
    };
    inset_area.left -= 3;
    inset_area.top -= 3;
    inset_area.right += 3;
    inset_area.bottom += 3;
    buffer->box_sprite(&inset_area, &InterfaceBoxStyles[3]);

    buffer->set_font(find_font(0x10, 1), 0, 0, 0);
    buffer->set_text_color(0xE1, -1, 1, 1);

    RECT title_area = {
        static_cast<LONG>(field_A34_), static_cast<LONG>(field_A38_),
        static_cast<LONG>(field_A3C_), static_cast<LONG>(field_A40_)
    };
    buffer->box_sprite(&title_area, &InterfaceBoxStyles[0]);

    StringTemp[0] = '\0';
    strcat(StringTemp, label_get(static_cast<int>(field_B28_) > 0 ? 542 : 541));
    CharUpperA(StringTemp);
    if (StringTemp != 0) {
        buffer->write_cent_l(StringTemp, &title_area,
                             static_cast<unsigned int>(strlen(StringTemp)));
    }

    const int title_offsets[4] = {0x110 / 4, 0x130 / 4, 0xEF0 / 4, 0xEF4 / 4};
    const int title_rect_offsets[4] = {0xA94, 0xAA4, 0xAB4, 0xAC4};
    int i;
    for (i = 0; i != 4; ++i) {
        RECT text_area;
        switch (title_rect_offsets[i]) {
        case 0xA94:
            text_area.left = static_cast<LONG>(field_A94_);
            text_area.top = static_cast<LONG>(field_A98_);
            text_area.right = static_cast<LONG>(field_A9C_);
            text_area.bottom = static_cast<LONG>(field_AA0_);
            break;
        case 0xAA4:
            text_area.left = static_cast<LONG>(field_AA4_);
            text_area.top = static_cast<LONG>(field_AA8_);
            text_area.right = static_cast<LONG>(field_AAC_);
            text_area.bottom = static_cast<LONG>(field_AB0_);
            break;
        case 0xAB4:
            text_area.left = static_cast<LONG>(field_AB4_);
            text_area.top = static_cast<LONG>(field_AB8_);
            text_area.right = static_cast<LONG>(field_ABC_);
            text_area.bottom = static_cast<LONG>(field_AC0_);
            break;
        default:
            text_area.left = static_cast<LONG>(field_AC4_);
            text_area.top = static_cast<LONG>(field_AC8_);
            text_area.right = static_cast<LONG>(field_ACC_);
            text_area.bottom = static_cast<LONG>(field_AD0_);
            break;
        }
        buffer->box_sprite(&text_area, &InterfaceBoxStyles[6]);
        StringTemp[0] = '\0';
        strcat(StringTemp, label_get(title_offsets[i]));
        CharUpperA(StringTemp);
        if (StringTemp != 0) {
            buffer->write_cent_l(StringTemp, &text_area,
                                 static_cast<unsigned int>(strlen(StringTemp)));
        }
    }

    RECT row_rects[4];
    row_rects[0].left = static_cast<LONG>(field_A54_);
    row_rects[0].top = static_cast<LONG>(field_A58_);
    row_rects[0].right = static_cast<LONG>(field_A5C_);
    row_rects[0].bottom = static_cast<LONG>(field_A60_);
    row_rects[1].left = static_cast<LONG>(field_A64_);
    row_rects[1].top = static_cast<LONG>(field_A68_);
    row_rects[1].right = static_cast<LONG>(field_A6C_);
    row_rects[1].bottom = static_cast<LONG>(field_A70_);
    row_rects[2].left = static_cast<LONG>(field_A74_);
    row_rects[2].top = static_cast<LONG>(field_A78_);
    row_rects[2].right = static_cast<LONG>(field_A7C_);
    row_rects[2].bottom = static_cast<LONG>(field_A80_);
    row_rects[3].left = static_cast<LONG>(field_A84_);
    row_rects[3].top = static_cast<LONG>(field_A88_);
    row_rects[3].right = static_cast<LONG>(field_A8C_);
    row_rects[3].bottom = static_cast<LONG>(field_A90_);
    for (i = 0; i != 4; ++i) {
        buffer->box_sprite(&row_rects[i], &InterfaceBoxStyles[0]);
    }

    RECT category_area = {
        static_cast<LONG>(field_A44_), static_cast<LONG>(field_A48_),
        static_cast<LONG>(field_A4C_), static_cast<LONG>(field_A50_)
    };
    buffer->box_sprite(&category_area, &InterfaceBoxStyles[0]);

    const int technology_id = static_cast<int>(field_B24_);
    const int category = tech_category(technology_id);
    buffer->set_font(find_font(0x28, 0), 0, 0, 0);
    buffer->set_text_color(mandate_color(category), -1, 1, 1);

    RECT category_label = category_area;
    category_label.left += 3;
    category_label.top += 3;
    category_label.right -= 3;
    category_label.bottom -= 3;
    category_label.bottom = buffer->text_line_height() + category_label.top;

    StringTemp[0] = '\0';
    say_tech(StringTemp, technology_id, 0);
    if (technology_id == TECH_CENTEMP) {
        // PlayersData+0x368 is theory_of_everything; the neutral member name
        // and exact offset are already independently established in
        // faction.h and consumed by picktech/faction/reportif.
        strcat(StringTemp, NewTechCentempSuffix);
        char number[80];
        _itoa(PlayersData[faction_snapshot].theory_of_everything,
              number, 10);
        strcat(StringTemp, number);
    }
    if (StringTemp != 0) {
        buffer->write_cent_l(StringTemp, &category_label,
                             static_cast<unsigned int>(strlen(StringTemp)));
    }

    const int line_height = buffer->text_line_height();
    const int image_top = category_area.top + line_height + 3;
    const int image_bottom = category_area.bottom - 3;
    const int image_height = image_bottom - image_top;
    const int image_width = category_area.right - category_area.left;
    Sprite *icon = &g_TECH_ICON_SPRITES[technology_id];
    const int icon_y = image_top + (image_height - 0xA0) / 2;
    const int icon_x = category_area.left + (image_width - 0xA0) / 2;
    icon->draw_mono(buffer, icon->transparent_index(), icon_x, icon_y,
                    mandate_color(category), 0xA0, 0xD7);

    RECT bottom_area = {
        static_cast<LONG>(field_AD4_), static_cast<LONG>(field_AD8_),
        static_cast<LONG>(field_ADC_), static_cast<LONG>(field_AE0_)
    };
    buffer->box_sprite(&bottom_area, &InterfaceBoxStyles[0]);
}

void NewTechWin::exec(int technology_id, int other_faction_id) {
    field_B28_ = static_cast<uint32_t>(other_faction_id);
    field_B24_ = static_cast<uint32_t>(technology_id);
    field_A14_ = 0;
    field_A18_ = 0;
    field_A1C_ = 0x226;
    field_A20_ = 0x258;

    int left = static_cast<int>(field_A14_) + 0xc;
    int top = static_cast<int>(field_A18_) + 0xc;
    int right = static_cast<int>(field_A1C_) - 0xc;
    int bottom = static_cast<int>(field_A20_) - 0xc;
    int inset_right = right - 3;
    bottom -= 3;
    const int row_top = top + 3;
    const int base_bottom = bottom;
    field_AF4_ = static_cast<uint32_t>(left + 3);
    field_AF8_ = static_cast<uint32_t>(bottom - 0x14);
    field_AFC_ = static_cast<uint32_t>(inset_right);
    field_B00_ = static_cast<uint32_t>(base_bottom);
    int row_bottom = base_bottom - 0x14 - 7;

    field_A24_ = static_cast<uint32_t>(left);
    field_A28_ = static_cast<uint32_t>(top);
    field_A2C_ = static_cast<uint32_t>(right);
    field_A30_ = static_cast<uint32_t>(row_bottom);
    field_A34_ = static_cast<uint32_t>(row_top);
    field_A38_ = static_cast<uint32_t>(top + 3);
    field_A3C_ = static_cast<uint32_t>(right - 3);
    field_A40_ = static_cast<uint32_t>(row_bottom - 3);

    int y = static_cast<int>(field_A38_) + 0x17;
    field_A40_ = static_cast<uint32_t>(y);
    field_A44_ = static_cast<uint32_t>(row_top);
    field_A48_ = static_cast<uint32_t>(field_A38_);
    field_A4C_ = static_cast<uint32_t>(field_A3C_);
    field_A50_ = static_cast<uint32_t>(row_bottom - 3);
    y = static_cast<int>(field_A40_) + 3;
    field_A48_ = static_cast<uint32_t>(y);
    y += 0xd9;
    field_A50_ = static_cast<uint32_t>(y);
    field_A94_ = static_cast<uint32_t>(row_top);
    y += 3;
    field_A98_ = static_cast<uint32_t>(y);
    field_A9C_ = static_cast<uint32_t>(field_A3C_);
    field_AA0_ = static_cast<uint32_t>(field_A40_);
    y = static_cast<int>(field_A98_) + 0x14;
    field_A54_ = static_cast<uint32_t>(row_top);
    field_AA0_ = static_cast<uint32_t>(y);
    y += 3;
    field_A58_ = static_cast<uint32_t>(y);
    field_A5C_ = static_cast<uint32_t>(field_A3C_);
    field_A60_ = static_cast<uint32_t>(field_A40_);
    y = static_cast<int>(field_A58_) + 0x4c;
    field_A60_ = static_cast<uint32_t>(y);
    y += 3;
    field_AB4_ = static_cast<uint32_t>(row_top);
    field_AB8_ = static_cast<uint32_t>(y);
    field_ABC_ = static_cast<uint32_t>(field_A3C_);
    field_AC0_ = static_cast<uint32_t>(field_A40_);
    y = static_cast<int>(field_AB8_) + 0x14;
    field_A74_ = static_cast<uint32_t>(row_top);
    field_AC0_ = static_cast<uint32_t>(y);
    y += 3;
    field_A78_ = static_cast<uint32_t>(y);
    field_A7C_ = static_cast<uint32_t>(field_A3C_);
    field_A80_ = static_cast<uint32_t>(field_A40_);
    y = static_cast<int>(field_A78_) + 0x4c;
    field_A80_ = static_cast<uint32_t>(y);
    y += 3;
    field_AD4_ = static_cast<uint32_t>(row_top);
    field_AD8_ = static_cast<uint32_t>(y);
    field_ADC_ = static_cast<uint32_t>(field_A3C_);
    field_AE0_ = static_cast<uint32_t>(row_bottom - 3);
    field_AE0_ = static_cast<uint32_t>(static_cast<int>(field_A30_) - 3);

    field_AA4_ = field_A94_;
    field_AA8_ = field_A98_;
    field_AAC_ = field_A9C_;
    field_AB0_ = field_AA0_;
    int width = static_cast<int>(field_A9C_) - static_cast<int>(field_A94_);
    width += 3;
    width >>= 1;
    width = width + static_cast<int>(field_A94_) - 3;
    field_A9C_ = static_cast<uint32_t>(width);
    technology_id = width;
    field_AA4_ = static_cast<uint32_t>(width + 3);

    field_A64_ = field_A54_;
    field_A68_ = field_A58_;
    field_A6C_ = field_A5C_;
    field_A70_ = field_A60_;
    field_A64_ = static_cast<uint32_t>(width + 3);
    field_A5C_ = static_cast<uint32_t>(width);
    field_AC4_ = field_AB4_;
    field_AC8_ = field_AB8_;
    field_ACC_ = field_ABC_;
    field_AD0_ = field_AC0_;
    field_ABC_ = static_cast<uint32_t>(width);
    field_AC4_ = static_cast<uint32_t>(width + 3);

    field_A84_ = field_A74_;
    field_A88_ = field_A78_;
    field_A8C_ = field_A7C_;
    field_A7C_ = static_cast<uint32_t>(technology_id);
    field_A90_ = field_A80_;

    field_B14_ = static_cast<uint32_t>(static_cast<int>(field_A2C_) - 6 - 0x1c);
    field_B18_ = static_cast<uint32_t>(static_cast<int>(field_A30_) - 6 - 0x22);
    field_B1C_ = static_cast<uint32_t>(static_cast<int>(field_A2C_) - 6);
    field_B20_ = static_cast<uint32_t>(static_cast<int>(field_A30_) - 6);
    field_B04_ = static_cast<uint32_t>(static_cast<int>(field_B14_) - 0x1f);
    field_B08_ = field_B18_;
    field_B0C_ = static_cast<uint32_t>(static_cast<int>(field_B1C_) - 0x1f);
    field_B10_ = field_B20_;
    field_AE4_ = field_AD4_;
    field_AE8_ = field_AD8_;
    field_AEC_ = field_ADC_;
    field_AF0_ = field_AE0_;
    field_AEC_ = static_cast<uint32_t>(static_cast<int>(field_B04_) - 4);

    RECT bounds;
    bounds.left = static_cast<LONG>(field_A14_);
    bounds.top = static_cast<LONG>(field_A18_);
    bounds.right = static_cast<LONG>(field_A1C_);
    bounds.bottom = static_cast<LONG>(field_A20_);
    init(&bounds, 0, 0x05000000, 0, 0, 0);
    fill(9);
    field_16C_ = 0x1a;
    center();

    RECT local_rects[4];
    local_rects[0].left = static_cast<LONG>(field_AA4_);
    local_rects[0].top = static_cast<LONG>(field_AA8_);
    local_rects[0].right = static_cast<LONG>(field_AAC_);
    local_rects[0].bottom = static_cast<LONG>(field_AB0_);
    local_rects[1].left = static_cast<LONG>(field_A64_);
    local_rects[1].top = static_cast<LONG>(field_A68_);
    local_rects[1].right = static_cast<LONG>(field_A6C_);
    local_rects[1].bottom = static_cast<LONG>(field_A70_);
    local_rects[2].left = static_cast<LONG>(field_AD4_);
    local_rects[2].top = static_cast<LONG>(field_AD8_);
    local_rects[2].right = static_cast<LONG>(field_ADC_);
    local_rects[2].bottom = static_cast<LONG>(field_AE0_);
    local_rects[3].left = static_cast<LONG>(field_A84_);
    local_rects[3].top = static_cast<LONG>(field_A88_);
    local_rects[3].right = static_cast<LONG>(field_A8C_);
    local_rects[3].bottom = static_cast<LONG>(field_A90_);

    for (int box_index = 0; box_index < 4; ++box_index) {
        StringBox &box = stringBoxes_[box_index];
        RECT &rect = local_rects[box_index];
        rect.left += 4;
        rect.top += 4;
        rect.right -= 4;
        rect.bottom -= 4;
        box.init(rect.left, rect.top, rect.right - rect.left,
                 rect.bottom - rect.top, this, 0, -1);

        const int multiplayer_style = IsMultiplayerNet != 0 ? 2 : 3;
        Buffer *const buffer = static_cast<Buffer *>(&box);
        buffer->set_font(find_font(0xe, 0), find_font(0xe, 1),
                         find_font(0xe, 2),
                         find_font(0xe, multiplayer_style));
        buffer->set_text_color(0xe1, -1, 1, 1);

        CurrentTechnologyBoxIndex = box_index;
        help_tech_info(newtech_info, static_cast<int>(field_B24_),
                       ConsoleGlobal.faction_id_, box_index);
        box.show(0);
    }

    const int left2 = static_cast<int>(field_AE8_) + 3;
    const int top2 = static_cast<int>(field_AE4_) + 3;
    const int height2 = static_cast<int>(field_AF0_) - 3 - left2;
    const int width2 = static_cast<int>(field_AEC_) - 3 - top2;
    stringBox2_.init(top2, left2, width2, height2, this, 0, -1);
    Buffer *const description_buffer = static_cast<Buffer *>(&stringBox2_);
    description_buffer->set_font(find_font(0xc, 0), find_font(0xc, 1),
                                 find_font(0xc, 2), 0);
    description_buffer->set_text_color(0x98, -1, 1, 1);
    StringTemp[0] = '\0';
    strcat(StringTemp, "TECH");
    char technology_number[24];
    _itoa(static_cast<int>(field_B24_), technology_number, 10);
    strcat(StringTemp, technology_number);
    stringBox2_.strings().load(DatalinkBlurbFile, StringTemp, 1, 0);
    stringBox2_.add_fixup();
    stringBox2_.show(0);

    const int button_width = static_cast<int>(field_B00_) -
                             static_cast<int>(field_AF8_);
    const int button_height = static_cast<int>(field_AFC_) -
                              static_cast<int>(field_AF4_);
    const int button_x = static_cast<int>(field_AF4_);
    const int button_y = static_cast<int>(field_AF8_);
    char *const button_label = label_get(0x194 / 4);
    flatButtons_[0].init(button_label, -1, button_x, button_y,
                         button_height, button_width, this, 0);
    flatButtons_[0].set_border_styles(&InterfaceBoxStyles[27],
                                      &InterfaceBoxStyles[28],
                                      &InterfaceBoxStyles[29]);
    static_cast<Buffer *>(&flatButtons_[0])->set_font(
        MainInterfaceGlobal.primary_font(), 0, 0, 0);
    flatButtons_[0].redraw();
    flatButtons_[0].redraw();

    buttonGroup_.init(0, 0);
    flatButtons_[1].init(0, 1, static_cast<int>(field_B04_),
        static_cast<int>(field_B08_),
        static_cast<int>(field_B0C_) - static_cast<int>(field_B04_),
        static_cast<int>(field_B10_) - static_cast<int>(field_B08_), this, 0);
    flatButtons_[1].set_report_icons(&g_NEWTECHWIN_SPRITES[0],
        &g_NEWTECHWIN_SPRITES[1], &g_NEWTECHWIN_SPRITES[2]);
    flatButtons_[1].redraw();
    flatButtons_[2].init(0, 2, static_cast<int>(field_B14_),
        static_cast<int>(field_B18_),
        static_cast<int>(field_B1C_) - static_cast<int>(field_B14_),
        static_cast<int>(field_B20_) - static_cast<int>(field_B18_), this, 0);
    flatButtons_[2].set_report_icons(&g_NEWTECHWIN_SPRITES[3],
        &g_NEWTECHWIN_SPRITES[4], &g_NEWTECHWIN_SPRITES[5]);
    flatButtons_[2].redraw();
    buttonGroup_.add(&flatButtons_[1]);
    buttonGroup_.add(&flatButtons_[2]);
    buttonGroup_.set(1, 1);

    show(0);
    do_all_draws();

    StringTemp[0] = '\0';
    strcat(StringTemp, "voices\\tech");
    char voice_number[24];
    _itoa(static_cast<int>(field_B24_), voice_number, 10);
    strcat(StringTemp, voice_number);
    strcat(StringTemp, ".mp3");
    if ((GameDataStateStorage.fields_.game_more_preferences_ & MPREF_AV_VOICEOVER_TECH_FAC) != 0) {
        const int faction = ConsoleGlobal.faction_id_;
        if ((static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) &
             static_cast<uint8_t>(1u << (faction & 0x1F))) != 0) {
            g_WAVE_GENERAL.unload();
            g_WAVE_GENERAL.load(StringTemp);
            g_WAVE_GENERAL.set_volume(WaveDeviceGlobal.get_group_volume(0));
            g_WAVE_GENERAL.play();
        }
    }

    do {
        set_modal(1, 0, 0);
        hide();
        do_all_draws();
        if (PendingTechnologyLink == -1) break;
        uint32_t id_storage = 0;
        int entry_id = 0;
        DatalinkMain.parse_id(PendingTechnologyLink,
            reinterpret_cast<DatalinkID *>(&id_storage), &entry_id);
        DatalinkMain.exec(id_storage, entry_id);
        PendingTechnologyLink = -1;
    } while (true);

    if ((GameDataStateStorage.fields_.game_more_preferences_ & MPREF_AV_VOICEOVER_TECH_FAC) != 0 &&
        (GameDataStateStorage.fields_.game_more_preferences_ & MPREF_AV_VOICEOVER_STOP_CLOSE_POPUP) != 0) {
        g_WAVE_GENERAL.fade(0x7d0);
    }
    close();
    do_all_draws();
}

// Original initializer 0x483880 constructs six Sprites at stride 0x2C;
// initial storage is zero. Registered teardown 0x4838B0 tests/sets mask0x01 of
// byte0x822604 before destruction; this initial-data binding does not establish
// that guard or complete startup/lifetime equivalence.
// ORIGINAL: 0x004838B0 ??__Fg_NEWTECHWIN_SPRITES@@YAXXZ 0x004838B0-0x004838D8 FILE
// RECOVERY_BINDING: v1 {"evidence": ["historical seed source sha256 1d4c408a4394b9aed1ffafeb2daceaef8b8dd8e51a3cbd5c422db155ddd369a4; original complete span 0x004838B0-0x004838D8 and all seed source findings retained", "historical seed measurement was NO_COMPILE due to the verifier skeleton's free-function __thiscall-pointer C4234; retain the finding, but do not carry that non-product skeleton into the real source owner", "historical seed note (preserved here as provenance): // `??__Fg_NEWTECHWIN_SPRITES@@YAXXZ` is the guarded, atexit-registered // teardown closure for a 6-element `NewTechSprite` array (element size 0x2c, // per-element teardown thunk at 0x406850 - the same address `src/ // atexit_thunks.cpp` binds as `SpriteElementTeardown`); the callee // `??_M@YGXPAXIHP6EX0@Z@Z` demangles to \"eh vector destructor iterator\", not // a constructor iterator. Confirmed twice (a function-local magic static // with a non-trivial dtor, and a file-scope static array with only a // non-trivial dtor) that VC6 ALWAYS emits this exact closure - same guard // idiom `mov cl,[addr]; mov al,1; test al,cl; jne`, same call - under // IMAGE_SYM_CLASS_STATIC (internal) linkage, never external, regardless of // the array's own linkage. `object_code()` requires exactly one EXTERNAL // .text symbol, so this exact closure is unselectable from a standalone // translation unit under the current verifier - a structural gap shared by // every `??__F...` / `??__E...` row, not a body defect.", "pinned original initializer 0x00483880 constructs six stride-0x2C Sprites at 0x00822610 and registers 0x004838B0 with atexit; its CRT-table pointer is at 0x00682568, immediately after the NewTechWin object initializer pointer", "pinned callback 0x004838B0 tests/sets bit 1 of byte 0x00822604 then calls the vector destructor iterator for six objects at 0x00822610; the guard is initially zero and has no other absolute references in the image", "current source already owns the exact bounded Sprite[6] allocation at 0x00822610-0x00822718; retained newtechwin.cpp.obj's actual _$E24 relocation targets that global and the vector destructor iterator", "retained current _$E26 initializer relocates to _$E24 and _atexit; .CRT$XCU selects _$E26. Thus this is the compiler closure for the same process-start object, not a manually authored callback or lazy static", "physical current COFF callback is 32 bytes and has no guard reference; current CRT registers it once. Preserve the original 40-byte guard difference for restoration; no matching/byte claim is granted"], "reason": "Bind the original global Sprite-array teardown identity to the actual compiler-generated destructor for the real source-owned Sprite[6] global. The current registered callback is unguarded and physically 32 bytes; the original's idempotence bit check remains restoration debt, not a byte-equivalence claim."}
// symbol    _$E24
// RECOVERY_OBJECT: v2 0x00822610 0x00822718 ?g_NEWTECHWIN_SPRITES@@3PAVSprite@@A 0x00453A70
Sprite g_NEWTECHWIN_SPRITES[6];

// ORIGINAL: 0x00483EB0 ?on_group_clicked@NewTechWin@@QAEXHH@Z 0x00483EB0-0x00483F8C
// RECOVERY_BINDING: v1 {"reason":"Recover the complete body in its genuine C++ owner, with real embedded members, shared virtual bases, lifecycle and parent operations. Compiler-generated overrides/adjustors replace executable-address or vbtable stand-ins; no current matching claim is granted.","evidence":["git:de91fffe:src/unrecovered/00483eb0.cpp","image:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf","recovery/tasks/ui-eight-owner-coverage.json","Original entry 0x00483eb0; physical VC6 COFF emitted symbol ?on_group_clicked@NewTechWin@@QAEXHH@Z."]}
// TRIED: `#pragma function(strcat)` - without it VC6 inlines strcat as a scasb/movs sequence; with it plus a switch(a2){case 1: case 2:} (matching the dec/je/dec/jne shape) this reaches 0.96 mnemonic similarity, MISMATCH only on the final load()-call arg pushes.
void NewTechWin::on_group_clicked(int, int group) {
    char number[80];
    char *title;
    switch (group) {
    case 1:
        StringTemp[0] = '\0';
        strcat(StringTemp, "TECH");
        _itoa(static_cast<int>(field_B24_), number, 10);
        strcat(StringTemp, number);
        stringBox2_.clear();
        title = DatalinkBlurbFile;
        break;
    case 2:
        StringTemp[0] = '\0';
        strcat(StringTemp, "TECH");
        _itoa(static_cast<int>(field_B24_), number, 10);
        strcat(StringTemp, number);
        stringBox2_.clear();
        title = const_cast<char *>("techlongs");
        break;
    default:
        return;
    }
    stringBox2_.strings().load(title, StringTemp, 1, 0);
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
