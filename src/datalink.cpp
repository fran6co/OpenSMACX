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
// ORIGINAL: 0x00432800 ??_GDatalink@@UAEPAXI@Z 0x00432800-0x00432821 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical deleting-thunk source src/deleting_thunks.cpp; original full spans retained: 0x00432800-0x00432821", "Pinned class constructor ??0Datalink@@QAE@XZ at 0x00428FC0 stores original class/view table VA 0x66af84 at 0x00429145 (mov dword ptr [esi], 0x66af84); the current genuine owner vftable ??_7Datalink@@6BWin@@@ selects generated ??_GDatalink@@UAEPAXI@Z in slot 0 (+0x0) through weak entry ??_EDatalink@@UAEPAXI@Z (mode 2).", "Pinned ??_GDatalink@@UAEPAXI@Z at 0x00432800 calls the source-mapped ??1Datalink@@QAE@XZ at 0x00432290, then tests byte ptr [ebp + 8], 1; je 0x43281a bypasses scalar delete when bit 0 is clear, while bit 0 set calls 0x0064557F; it save original this; call destructor; delete only when flags bit 0 is set; return saved this with ret 4.", "The real class lifetime is owned by src/datalink.cpp; its compiler-emitted deleting symbol and vftable slot carry this original identity. Hand-written adapter bodies with live adjustor callers remain until those callers are independently qualified and transferred. The linked weak/COMDAT copy is not used to choose source ownership.", "the source writer preserves the historical mapping's exactness state (historical exactness obligation retained); current compiler extent is not asserted equal to the original span"], "reason": "Bind the original Datalink scalar deleting-destructor identity to the actual compiler-emitted class deleting destructor in its genuine owner translation unit. The original constructor's pinned vptr store and the current owner's physical table slot identify this class/view; no callback/thunk body is hand-authored here."}
// symbol    ??_GDatalink@@UAEPAXI@Z
// kind      game
#include "stdafx.h"
#include "game.h"  // shared state-window owner
#include "original_seam.h"
#include "datalink.h"
// Datalink::show and Datalink::hide reach the SubInterface subobject the
// original places at +0xA14 by casting a hand-computed address, so the type
// must be complete.
#include "subinterface.h"
#include "worldwin.h"
#include "menu.h"
#include "maininterface.h"
#include "win.h"
#include "spritebox.h"
#include "vtable_shim.h"
#include "reportwin.h"
#include "strings.h"
#include "alpha.h"
#include "veh.h"
#include "base.h"
#include "text.h"
#include "faction.h"
#include "general.h"
#include "popup.h"
#include "reportif.h"
#include "technology.h"
#include "console.h"
#include "council.h"  // source-owned Proposal table and its original eleven-entry bound
#include "terraforming.h"
#include "texture.h"

extern int MainInterfaceTileFlag;
// RECOVERY_OBJECT: v2 0x00691B60 0x00691B66 ?DatalinkHelpFileText@@3PADA 0x00430770
char DatalinkHelpFileText[] = "HELPX";
// RECOVERY_OBJECT: v2 0x00691B68 0x00691B70 ?DatalinkBlurbFileText@@3PADA 0x00430B30
char DatalinkBlurbFileText[] = "BLURBSX";
// RECOVERY_OBJECT: v2 0x00691B00 0x00691B04 ?DatalinkHelpFile@@3PADA 0x00430770
char *DatalinkHelpFile = DatalinkHelpFileText;
// RECOVERY_OBJECT: v2 0x00691B04 0x00691B08 ?DatalinkBlurbFile@@3PADA 0x00430B30
char *DatalinkBlurbFile = DatalinkBlurbFileText;

// draw_terraform passes this writable empty string. Only its initial NUL is
// established; the complete original storage extent remains unresolved.
// RECOVERY_OBJECT: v1 0x0071F238 ?DatalinkTerraformExtraText@@3PADA
char DatalinkTerraformExtraText[1];

static void datalink_vehicle_line_start(int label) {
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(label));
    strcat(StringTemp, ": ");
    strcat(StringTemp, " ");
    strcat(StringTemp, "{");
}

static void datalink_vehicle_line_finish(StringList &list) {
    strcat(StringTemp, "}");
    list.pending_text_ = StringTemp;
    list.field_20_ = 4;
    list.field_24_ = 0;
    list.add(0);
}

static void datalink_vehicle_append_number(int value) {
    char number[80];
    _itoa(value, number, 10);
    strcat(StringTemp, number);
}

static void datalink_vehicle_append_rating(int rating) {
    if (rating < 0) {
        strcat(StringTemp, label_get(0x310 / 4));
    } else {
        datalink_vehicle_append_number(rating);
    }
}

static void datalink_vehicle_append_tech(int tech) {
    char name[256];
    name[0] = 0;
    say_tech(name, tech, 0);
    if (tech < 0) {
        strcat(StringTemp, name);
    } else {
        char id[256];
        id[0] = 0;
        _itoa(tech + 140000, id, 10);
        help_create_link(StringTemp, name, id);
    }
}

// The Datalink singleton, 0x00703EA0 in the shipped image - the storage the
// `Datalink *const DatalinkMain` binding used to name (see datalink.h). Plain
// data, not a call target; the Datalink constructor that builds it runs from
// the game's class bring-up, as in the image.
Datalink DatalinkMain;  // 0x00703EA0

// Member/base lifetimes run naturally in declaration order. The original
// ListBox constructor's extra 1 is its hidden most-derived ABI flag, not an
// explicit C++ parameter: the real virtual bases generate it automatically.
// ORIGINAL: 0x004080B0 sub_4080b0 0x004080B0-0x004080D3 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:0x004080B0-0x004080D3 calls ListBox, Dialog and GraphicWin destructor stages in that order at the evidenced subobject views", "source:src/listbox.h ListBox is the genuine virtual GraphicWin/Dialog class; src/listbox.cpp ListBox::~ListBox calls close()", "source:src/datalink.h Datalink owns a typed ListBox member; src/datalink.cpp Datalink lifetime naturally emits the complete-object stage", "build/recovery/builds/build-gyzprt2j/OpenSMACX.map selects current ??_DListBox at 0x004D3740 from datalink.cpp.obj; the selected physical COFF code COMDAT is 48 bytes", "the selected ListBox COFF relocations at +0x0A/+0x15/+0x1C resolve to ??1ListBox@@UAE@XZ, ??1Dialog@@UAE@XZ, ??1GraphicWin@@UAE@XZ; section bytes preserve the pinned call order, +0x48 shared receiver and +0xA60 Dialog view, followed by 13 bytes linker/compiler padding", "current MAP/COFF proves natural emission and the selected current linked copy only; it does not prove the pinned executable's original translation unit or whole-game acceptance"], "reason": "The pinned complete-object identity destroys the real ListBox subobject and its shared virtual bases. Datalink owns a typed ListBox member; the naturally emitted compiler complete-object destructor is recorded in this existing consumer TU. This is source-owner/lifetime evidence, not a claim that the historical original translation unit was datalink.cpp."}
// kind      game
// symbol    ??_DListBox@@QAEXXZ
// LEVER: the image's three calls are direct E8s to the destructors
// LEVER: nullptr also costs the caller a `xor edx, edx` it never had to pay
// TRIED: VC6 tail-call-folds the LAST call into `jmp` regardless of which
// RECOVERY_HISTORY: /*
// RECOVERY_HISTORY: Purpose: Destroy the ListBox at 0x48, the Dialog at 0xa60, and the GraphicWin
// RECOVERY_HISTORY:          base the ListBox shares.
// RECOVERY_HISTORY:
// RECOVERY_HISTORY:              mov esi,ecx / lea edi,[esi+0x48] / mov ecx,edi
// RECOVERY_HISTORY:              call ??1ListBox@@QAE@XZ / lea ecx,[esi+0xa60]
// RECOVERY_HISTORY:              call ??1Dialog@@QAE@XZ / mov ecx,edi
// RECOVERY_HISTORY:              call ??1GraphicWin@@QAE@XZ / ret
// RECOVERY_HISTORY:
// RECOVERY_HISTORY:          EDI is computed ONCE and used twice - for the ListBox and again for
// RECOVERY_HISTORY:          the GraphicWin. They are the same subobject at 0x48, destroyed at two
// RECOVERY_HISTORY:          levels, with an unrelated Dialog at 0xa60 in between. A body that gave
// RECOVERY_HISTORY:          GraphicWin the outer object would be wrong only where 0x48 matters.
// RECOVERY_HISTORY:
// RECOVERY_HISTORY:          0x48 is ListBoxDestructorAdjustment, and list_box_destructor_redirect
// RECOVERY_HISTORY:          takes exactly that already-adjusted pointer, so it is passed through
// RECOVERY_HISTORY:          rather than recomputed.
// RECOVERY_HISTORY:
// RECOVERY_HISTORY: // RECOVERY_ROLE: SCAFFOLD Transitional leaf collection, not completed genuine owner recovery; preserve body evidence during owner migration.
// RECOVERY_HISTORY: // original marker: 0x004080B0 sub_4080b0 0x004080B0-0x004080D3
// RECOVERY_HISTORY: // symbol    ?leaf_004080b0_redirect@@YIXPAX0@Z
// RECOVERY_HISTORY: // size      35 bytes
// RECOVERY_HISTORY: // prototype
// RECOVERY_HISTORY: // callers   3   call targets   3
// RECOVERY_HISTORY: // kind      game
// RECOVERY_HISTORY: // flags     hidden;sp_ready;purged_ok
// RECOVERY_HISTORY: // calls     0x005D4DD0 0x00608E10 0x00609EC0
// RECOVERY_HISTORY: // LEVER: the image's three calls are direct E8s to the destructors
// RECOVERY_HISTORY: // themselves - going through the `Leaf*Destructor` pointer variables compiles
// RECOVERY_HISTORY: // `call dword ptr [...]` and call_diff reports 0 real calls for it. Every
// RECOVERY_HISTORY: // OTHER caller of these redirects (basebutton.cpp, dialogs.cpp, pulldown.cpp,
// RECOVERY_HISTORY: // scroll.cpp) already calls them by name; doing the same here reaches the
// RECOVERY_HISTORY: // image's call sequence. The `Leaf*Destructor` globals stay declared for the
// RECOVERY_HISTORY: // fixture use the comment above describes - nothing currently reads them.
// RECOVERY_HISTORY: // LEVER: nullptr also costs the caller a `xor edx, edx` it never had to pay
// RECOVERY_HISTORY: // (the destructors ignore their second slot) - casting each redirect through
// RECOVERY_HISTORY: // `SingleArgDtor` before calling drops that instruction too.
// RECOVERY_HISTORY: // TRIED: VC6 tail-call-folds the LAST call into `jmp` regardless of which
// RECOVERY_HISTORY: // spelling reaches it (SingleArgDtor or the plain 2-arg redirect+nullptr) -
// RECOVERY_HISTORY: // the image has a real `call 0x5d4dd0` / `pop edi` / `pop esi` / `ret` there.
// RECOVERY_HISTORY: // Reordering the three calls is not legal (it would call the wrong symbol
// RECOVERY_HISTORY: // last); adding a `volatile` sink after the discarded `GraphicWin*` result
// RECOVERY_HISTORY: // does not stop the fold either (measured). SingleArgDtor on all three scores
// RECOVERY_HISTORY: // higher (0.880, 9/13) than nullptr-on-the-last (0.846, 11/13) despite fewer
// RECOVERY_HISTORY: // raw agreements, so all three are cast; the trailing jmp is the residue.
// RECOVERY_HISTORY: Return Value: n/a
// RECOVERY_HISTORY: Status: Complete
// RECOVERY_HISTORY: */

// ORIGINAL: 0x00428FC0 ??0Datalink@@QAE@XZ 0x00428FC0-0x00429178;0x00653450-0x00653579 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:6201d294:src/unrecovered/00428fc0.cpp"], "reason": "Original construction and reverse teardown establish real Flic/Caviar, an eighteen-element FlatButton array and eleven Font members. The owner performs its scalar initialization/reset; genuine member/base lifetimes replace manual vtables and original-address teardown."}
// Historical scaffold experiment, retained from the replaced seed:
// TRIED: the scaffold's own guessed Datalink layout (FlatButton x18, 3 Fonts, raw byte fields) does not match this ctor's real member offsets at all; rebuilt from the disassembly's own lea/call offsets instead (Flic@0xa28, Caviar@0x150c, an 18x0xb4c array block@0x2a3c, ButtonGroup@0xf594, ListBox(1)@0xf628, 11 Fonts, 4 StringBoxes), using `uint8_t pad[N - sizeof(Prev)]` gaps so the compiler validates each offset by refusing a negative/zero array. Declaring explicit `~Type()` on each real member (absent from the header) is what makes MSVC synthesize the EH frame/state-byte machinery at all - matches the original prologue, all 20 constructor calls incl. exact per-member SEH checkpoint values 0x0-0x13, and the whole epilogue byte-for-byte. Only remaining diff: one `lea`/`push` scheduling swap around the array block's argument setup (register-allocator choice, not a source-form lever found).
Datalink::Datalink() {
    history_[0] = 0;
    field_2A34_ = 0;
    field_2A38_ = 0;
    field_29DC_ = 2;
}

// Reverse-order member/base destruction belongs to the compiler. In particular,
// Caviar::~Caviar owns close(); this body must not close the renderer twice.
// ORIGINAL: 0x00432290 ??1Datalink@@QAE@XZ 0x00432290-0x004325AA;0x006531D0-0x006533A2
// RECOVERY_BINDING: v1 {"evidence": ["git:6201d294:src/recovered/units/00432290.cpp"], "reason": "Original construction and reverse teardown establish real Flic/Caviar, an eighteen-element FlatButton array and eleven Font members. The owner performs its scalar initialization/reset; genuine member/base lifetimes replace manual vtables and original-address teardown."}
// symbol    ??1Datalink@@UAE@XZ
Datalink::~Datalink() {
    history_[0] = 0;
    field_2A34_ = 0;
    field_2A38_ = 0;
}

// ORIGINAL: 0x0042C9C0 ?draw_unit@Datalink@@QAEXXZ 0x0042C9C0-0x0042D608;0x00653712-0x00653746
// RECOVERY_BINDING: v1 {"evidence": ["git:6201d294:src/unrecovered/0042c9c0.cpp"], "reason": "The Datalink owner implements this original topic renderer through typed rules, real Buffer/Caviar members and constructed Popup/StringList lifetimes; original intervals and seed provenance remain restoration inputs."}
// Historical scaffold experiment, retained from the replaced seed:
// TRIED: RAII StringStruct local (declared ~StringStruct() to force a real dtor call) in place of the original's inlined vtable-pointer-swap destructor state machine (local_3c/ local_40/local_18/local_14) - diverges at instr #0, prologue entirely different SEH shape; the inlined dtor loop appears twice in the original (second copy is dead - guard is always false at runtime) and was not hand-modeled given the 3196-byte scope. The 7 repeated get-string/strcat/itoa/add() blocks and the veh_fake/veh_draw calls ARE transcribed field-for-field.
void Datalink::draw_unit() {
    StringList description;
    const int id = history_[0];
    const VehPrototype &unit = VehPrototypes[id];
    const RulesWeapon &weapon = Weapon[unit.weapon_id];
    const RulesArmor &armor = Armor[unit.armor_id];
    const RulesChassis &chassis = Chassis[unit.chassis_id];
    const int vehicle = veh_fake(id, ConsoleGlobal.faction_id_);
    Vehs[vehicle].morale = 4; // 0x42CA86: Vehs+0x24, stride 0x34.
    set_clip(&illustration_rect_);
    veh_draw(static_cast<Buffer *>(this), vehicle, illustration_rect_.left,
             illustration_rect_.top, 4, 0x40, 0);
    restore_clip();
    set_font(&font1_, 0, 0, 0);
    set_text_color(0xE0, -1, 1, 1);
    StringTemp[0] = 0;
    // Original reloads the selected prototype id at 0x42CAEC, not fake id2048.
    strcat(StringTemp, unit.veh_name);
    CharUpperA(StringTemp);
    write_cent_l(StringTemp, &title_rect_, strlen(StringTemp));
    const int left = description_rect_.left + 9
                   + illustration_rect_.right - illustration_rect_.left;
    description.remove_all();
    if (weapon.mode < 3) {
        datalink_vehicle_line_start(0x204 / 4);
        datalink_vehicle_append_rating(weapon.offense_rating);
    } else {
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x200 / 4));
        strcat(StringTemp, ": ");
        strcat(StringTemp, " ");
        if (weapon.mode > 6 && weapon.mode < 13) {
            strcat(StringTemp, "{");
            strcat(StringTemp, StringTable.get(reinterpret_cast<int>(weapon.name)));
        } else {
            strcat(StringTemp, label_get(0x204 / 4));
            strcat(StringTemp, ": ");
            strcat(StringTemp, " ");
            strcat(StringTemp, "{");
            datalink_vehicle_append_rating(weapon.offense_rating);
        }
    }
    datalink_vehicle_line_finish(description);
    datalink_vehicle_line_start(0x154 / 4);
    datalink_vehicle_append_rating(armor.defense_rating);
    datalink_vehicle_line_finish(description);
    datalink_vehicle_line_start(0x1C / 4);
    strcat(StringTemp, StringTable.get(reinterpret_cast<int>(Triad[chassis.triad])));
    strcat(StringTemp, " ");
    datalink_vehicle_append_number(chassis.speed);
    datalink_vehicle_line_finish(description);
    if (unit.carry_capacity) {
        datalink_vehicle_line_start(0x360 / 4);
        datalink_vehicle_append_number(unit.carry_capacity);
        datalink_vehicle_line_finish(description);
    }
    if (chassis.triad == 2 && chassis.range) {
        datalink_vehicle_line_start(0x30C / 4);
        datalink_vehicle_append_number(chassis.range);
        strcat(StringTemp, " ");
        strcat(StringTemp, label_get(chassis.range == 1 ? 0xC0 / 4 : 0xC4 / 4));
        datalink_vehicle_line_finish(description);
    }
    datalink_vehicle_line_start(0x160 / 4);
    datalink_vehicle_append_number(cost_factor(ConsoleGlobal.faction_id_, 1, -1) * unit.cost);
    datalink_vehicle_line_finish(description);
    datalink_vehicle_line_start(0xF24 / 4);
    datalink_vehicle_append_tech(unit.preq_tech);
    datalink_vehicle_line_finish(description);
    set_font(&body_font_, 0, 0, &link_font_);
    set_text_color(0x98, -1, 1, 1);
    set_text_color2(0x96, -1, 1, 1);
    set_text_color_hyper(0x96, -1, 1, 1);
    // 0x42D416 loads description right, not bottom (seed uses bottom).
    write_strings(&description, left, description_rect_.top + 6,
                  description_rect_.right - 6 - left, 0);
    StringTemp[0] = 0;
    strcat(StringTemp, "UNITDESC");
    datalink_vehicle_append_number(id);
    detail_box_.clear();
    if (detail_box_.strings().load(DatalinkHelpFile, StringTemp, 1, 0) == 0) {
        detail_box_.add_fixup();
        detail_box_.show(0);
    }
    // 0x42D4A6 selects the whole entry update rectangle (+0x103B4).
    soft_update(&entry_update_rect_);
}
// ORIGINAL: 0x0042D610 ?draw_chassis@Datalink@@QAEXXZ 0x0042D610-0x0042DEEF;0x00653746-0x0065377A
// RECOVERY_BINDING: v1 {"evidence": ["git:6201d294:src/unrecovered/0042d610.cpp"], "reason": "The Datalink owner implements this original topic renderer through typed rules, real Buffer/Caviar members and constructed Popup/StringList lifetimes; original intervals and seed provenance remain restoration inputs."}
// Historical scaffold experiment, retained from the replaced seed:
// TRIED: full transcription landed (MISMATCH #2, no SEH frame reproduced). Caught a real Ghidra error via byte-level cross check: Ghidra's `write_strings(&local_44,local_54,local_50, local_4c-local_54,0)` invents a `local_54` stack slot that never appears anywhere in the raw disassembly (only -0x50, -0x4c, -0x48, -0x44 are ever touched); the real call reads [ebp-0x50] and [ebp-0x48] for the width term. Used the StringStruct(0) local + explicit remove_all()/add(0) calls, matching this function's calls to those symbols directly (unlike the inlined form seen on other StringStruct-using functions in this batch).
void Datalink::draw_chassis() {
    StringList description;
    const int id = history_[0];
    const RulesChassis &chassis = Chassis[id];
    caviar_.reset_objects();
    vehdraw_construct_chassis(ConsoleGlobal.faction_id_, id, 0, 0, 1, &caviar_);
    caviar_.set_scaling(0.1f);
    caviar_.render(static_cast<Buffer *>(this),
                   (illustration_rect_.left + illustration_rect_.right) / 2,
                   (illustration_rect_.top + illustration_rect_.bottom) / 2, 6);
    set_font(&font1_, 0, 0, 0);
    set_text_color(0xE0, -1, 1, 1);
    StringTemp[0] = 0;
    strcat(StringTemp, StringTable.get(reinterpret_cast<int>(chassis.offsv_1_name)));
    CharUpperA(StringTemp);
    write_cent_l(StringTemp, &title_rect_, strlen(StringTemp));
    const int left = description_rect_.left + 9
                   + illustration_rect_.right - illustration_rect_.left;
    description.remove_all();
    datalink_vehicle_line_start(0x1C / 4);
    strcat(StringTemp, StringTable.get(reinterpret_cast<int>(Triad[chassis.triad])));
    strcat(StringTemp, " ");
    datalink_vehicle_append_number(chassis.speed);
    datalink_vehicle_line_finish(description);
    if (chassis.cargo) {
        datalink_vehicle_line_start(0x360 / 4);
        datalink_vehicle_append_number(chassis.cargo);
        if (chassis.triad == 1) {
            strcat(StringTemp, " ");
            strcat(StringTemp, label_get(0xF34 / 4));
        }
        datalink_vehicle_line_finish(description);
    }
    if (chassis.triad == 2) {
        datalink_vehicle_line_start(0x30C / 4);
        if (!chassis.range) {
            strcat(StringTemp, label_get(0xF30 / 4));
        } else {
            datalink_vehicle_append_number(chassis.range);
            strcat(StringTemp, " ");
            strcat(StringTemp, label_get(chassis.range == 1 ? 0xC0 / 4 : 0xC4 / 4));
        }
        datalink_vehicle_line_finish(description);
    }
    datalink_vehicle_line_start(0x160 / 4);
    datalink_vehicle_append_number(chassis.cost);
    datalink_vehicle_line_finish(description);
    datalink_vehicle_line_start(0xF24 / 4);
    datalink_vehicle_append_tech(chassis.preq_tech);
    datalink_vehicle_line_finish(description);
    set_font(&body_font_, 0, 0, &link_font_);
    set_text_color(0x98, -1, 1, 1);
    set_text_color2(0x96, -1, 1, 1);
    set_text_color_hyper(0x96, -1, 1, 1);
    write_strings(&description, left, description_rect_.top + 6,
                  description_rect_.right - 6 - left, 0);
    StringTemp[0] = 0;
    strcat(StringTemp, "CHASSISDESC");
    datalink_vehicle_append_number(id);
    detail_box_.clear();
    if (detail_box_.strings().load(DatalinkHelpFile, StringTemp, 1, 0) == 0) {
        detail_box_.add_fixup();
        detail_box_.show(0);
    }
    soft_update(&description_rect_);
    description.remove_all();
}
// ORIGINAL: 0x0042DEF0 ?draw_reactor@Datalink@@QAEXXZ 0x0042DEF0-0x0042E2D3;0x0065377A-0x006537AE
// RECOVERY_BINDING: v1 {"evidence": ["git:6201d294:src/recovered/units/0042def0.cpp"], "reason": "The Datalink owner implements this original topic renderer through typed rules, real Buffer/Caviar members and constructed Popup/StringList lifetimes; original intervals and seed provenance remain restoration inputs."}
void Datalink::draw_reactor() {
    StringList description;
    const int id = history_[0];
    // Original indexes 0x9527EC; the source-owned array begins at0x9527F8.
    const RulesReactor &reactor = Reactor[id - 1];
    caviar_.reset_objects();
    vehdraw_construct_reactor(ConsoleGlobal.faction_id_, -1, -1, id, 0, &caviar_);
    caviar_.set_scaling(0.2f);
    caviar_.render(static_cast<Buffer *>(this),
                   (illustration_rect_.left + illustration_rect_.right) / 2,
                   (illustration_rect_.top + illustration_rect_.bottom) / 2, 6);
    set_font(&font1_, 0, 0, 0);
    set_text_color(0xE0, -1, 1, 1);
    StringTemp[0] = 0;
    strcat(StringTemp, StringTable.get(reinterpret_cast<int>(reactor.name)));
    CharUpperA(StringTemp);
    write_cent_l(StringTemp, &title_rect_, strlen(StringTemp));
    const int left = description_rect_.left + 9
                   + illustration_rect_.right - illustration_rect_.left;
    parse_num(0, id);
    parse_says(1, tech_name(reactor.preq_tech, 0), -1, -1);
    parse_num(1, reactor.preq_tech + 140000);
    description.remove_all();
    description.load(DatalinkHelpFile, "HELPREACTOR", 1, 0);
    set_font(&body_font_, 0, 0, reactor.preq_tech < 0 ? 0 : &link_font_);
    set_text_color(0x98, -1, 1, 1);
    set_text_color2(0x96, -1, 1, 1);
    set_text_color_hyper(0x96, -1, 1, 1);
    write_strings(&description, left, description_rect_.top + 6,
                  description_rect_.right - 6 - left, 0);
    StringTemp[0] = 0;
    strcat(StringTemp, "REACTORDESC");
    detail_box_.clear();
    if (detail_box_.strings().load(DatalinkHelpFile, StringTemp, 1, 0) == 0) {
        detail_box_.add_fixup();
        detail_box_.show(0);
    }
    soft_update(&description_rect_);
    description.remove_all();
}
// ORIGINAL: 0x0042E2E0 ?draw_weapon@Datalink@@QAEXXZ 0x0042E2E0-0x0042E8D3;0x006537AE-0x006537E2
// RECOVERY_BINDING: v1 {"evidence": ["git:6201d294:src/unrecovered/0042e2e0.cpp"], "reason": "The Datalink owner implements this original topic renderer through typed rules, real Buffer/Caviar members and constructed Popup/StringList lifetimes; original intervals and seed provenance remain restoration inputs."}
// Historical scaffold experiment, retained from the replaced seed:
// TRIED: the SEH __try/__finally frame and the local StringList's multiple-inheritance secondary-vtable adjustor (the two funclets at 0x006537C5/0x006537CD exist only for that); the entry-deleting virtual call is a double-indirected adjustor thunk (vtable[1] read back as a BYTE OFFSET to a second vtable pointer) that no portable dispatch can reproduce, so it goes through the VCall shim directly instead; several argument orders for batched-__cdecl-cleanup call clusters (parse_num/ parse_says/tech_name) are best-guessed from push proximity to the call, not fully hand-verified against the stack-adjustment bytes.
void Datalink::draw_weapon() {
    StringList description;
    const int id = history_[0];
    const RulesWeapon &weapon = Weapon[id];
    set_clip(&illustration_rect_);
    caviar_.reset_objects();
    if (id == 0) {
        caviar_.add_object(&g_VW00_CAVIARDATA, 0);
    } else {
        vehdraw_construct_weapon(ConsoleGlobal.faction_id_, -1, id, &caviar_);
    }
    caviar_.set_scaling(id == 0x10 || id == 0x17 ? 0.1f
                         : (weapon.mode < 3 ? 0.3f : 0.2f));
    caviar_.render(static_cast<Buffer *>(this),
                   (illustration_rect_.left + illustration_rect_.right) / 2,
                   (illustration_rect_.top + illustration_rect_.bottom) / 2, 6);
    restore_clip();
    set_font(&font1_, 0, 0, 0);
    set_text_color(0xE0, -1, 1, 1);
    StringTemp[0] = 0;
    strcat(StringTemp, StringTable.get(reinterpret_cast<int>(weapon.name)));
    CharUpperA(StringTemp);
    write_cent_l(StringTemp, &title_rect_, strlen(StringTemp));
    const int left = description_rect_.left + 12
                   + illustration_rect_.right - illustration_rect_.left;
    StringTemp[0] = 0;
    if (weapon.offense_rating < 0) {
        strcat(StringTemp, label_get(0x310 / 4));
    } else if (weapon.mode < 3) {
        strcat(StringTemp, StringTable.get(reinterpret_cast<int>(OffenseModes[weapon.mode].name)));
    } else {
        strcat(StringTemp, label_get(0x200 / 4));
    }
    parse_says(1, StringTemp, -1, -1);
    parse_num(0, weapon.offense_rating);
    parse_num(1, weapon.cost);
    parse_says(2, tech_name(weapon.preq_tech, 0), -1, -1);
    parse_num(2, weapon.preq_tech + 140000);
    StringTemp[0] = 0;
    strcat(StringTemp, "HELPWEAPON");
    if (weapon.offense_rating == weapon.cost) {
        datalink_vehicle_append_number(2);
    }
    description.remove_all();
    description.load(DatalinkHelpFile, StringTemp, 1, 0);
    set_font(&body_font_, 0, 0, weapon.preq_tech < 0 ? 0 : &link_font_);
    set_text_color(0x98, -1, 1, 1);
    set_text_color2(0x96, -1, 1, 1);
    set_text_color_hyper(0x96, -1, 1, 1);
    write_strings(&description, left, description_rect_.top + 6,
                  description_rect_.right - 6 - left, 0);
    description.remove_all();
    StringTemp[0] = 0;
    strcat(StringTemp, "WEAPONDESC");
    datalink_vehicle_append_number(id); // 0x42E711 loads saved weapon id.
    detail_box_.clear();
    if (detail_box_.strings().load(DatalinkHelpFile, StringTemp, 1, 0) == 0) {
        detail_box_.add_fixup();
        detail_box_.show(0); // 0x42E76E is virtual slot1, not seed slot0.
    }
    soft_update(&description_rect_);
}
// ORIGINAL: 0x0042E8E0 ?draw_defense@Datalink@@QAEXXZ 0x0042E8E0-0x0042EE26;0x006537E2-0x00653816
// RECOVERY_BINDING: v1 {"evidence": ["git:6201d294:src/recovered/units/0042e8e0.cpp"], "reason": "The Datalink owner implements this original topic renderer through typed rules, real Buffer/Caviar members and constructed Popup/StringList lifetimes; original intervals and seed provenance remain restoration inputs."}
void Datalink::draw_defense() {
    StringList help;
    const int armor_id = history_[0];
    const RulesArmor &armor = Armor[armor_id];
    RECT text = description_rect_;
    text.left += 6;
    text.top += 6;
    text.right -= 6;
    if (armor_id != 0) {
        set_clip(&illustration_rect_);
        caviar_.reset_objects();
        vehdraw_construct_armor(ConsoleGlobal.faction_id_, -1, -1,
                                armor_id, 1, &caviar_);
        caviar_.set_scaling(armor_id == 8 ? 0.15f : 0.20f);
        const int x = (illustration_rect_.left + illustration_rect_.right) / 2;
        const int y = (illustration_rect_.top + illustration_rect_.bottom) / 2;
        if (armor_id == 5) {
            caviar_.render_multi_table_dest(static_cast<Buffer *>(this), x, y,
                24, reinterpret_cast<unsigned char *>(&ArmorTransparentColorTables), 6);
        } else {
            caviar_.render(static_cast<Buffer *>(this), x, y, 6);
        }
        set_clip(buffer_working_rect());
    }
    set_font(&font1_, 0, 0, 0);
    set_text_color(0xE0, -1, 1, 1);
    StringTemp[0] = 0;
    strcat(StringTemp, StringTable.get(reinterpret_cast<int>(armor.name)));
    CharUpperA(StringTemp);
    write_cent_l(StringTemp, &title_rect_, strlen(StringTemp));
    text.left += illustration_rect_.right - illustration_rect_.left + 3;
    StringTemp[0] = 0;
    const int mode_name = armor.defense_rating < 0
        ? reinterpret_cast<int>(reinterpret_cast<LPSTR *>(Labels.strings_ptr)[0x310 / 4])
        : reinterpret_cast<int>(DefenseModes[armor.mode].name);
    strcat(StringTemp, StringTable.get(mode_name));
    parse_says(1, StringTemp, -1, -1);
    parse_num(0, armor.defense_rating);
    parse_says(2, tech_name(armor.preq_tech, 0), -1, -1);
    parse_num(2, armor.preq_tech + 140000);
    help.remove_all();
    help.load(DatalinkHelpFile,
              armor.defense_rating < 0 ? "HELPDEFENSE2" : "HELPDEFENSE", 1, 0);
    set_font(&body_font_, 0, 0, armor.preq_tech < 0 ? 0 : &link_font_);
    set_text_color(0x98, -1, 1, 1);
    set_text_color2(0x96, -1, 1, 1);
    set_text_color_hyper(0x96, -1, 1, 1);
    write_strings(&help, text.left, text.top, text.right - text.left, 0);
    StringTemp[0] = 0;
    strcat(StringTemp, "ARMORDESC");
    if (armor_id >= 9) {
        char number[80];
        _itoa(armor_id, number, 10);
        strcat(StringTemp, number);
    }
    detail_box_.clear();
    if (detail_box_.strings().load(DatalinkHelpFile, StringTemp, 1, 0) == 0) {
        detail_box_.add_fixup();
        detail_box_.show(0);
    }
    soft_update(&description_rect_);
}
// ORIGINAL: 0x0042EE30 ?draw_ability@Datalink@@QAEXXZ 0x0042EE30-0x0042F468;0x00653816-0x0065384A
// RECOVERY_BINDING: v1 {"evidence": ["git:6201d294:src/unrecovered/0042ee30.cpp"], "reason": "The Datalink owner implements this original topic renderer through typed rules, real Buffer/Caviar members and constructed Popup/StringList lifetimes; original intervals and seed provenance remain restoration inputs."}
// Historical scaffold experiment, retained from the replaced seed:
// TRIED: a genuine `StringStruct local40(0);` local to pick up the compiler's own /GX SEH frame (as worked elsewhere) still diverges at instruction #2 (sub vs push) - this function's frame layout (0x98-byte locals, three linked-list teardown passes at the tail) needs more than one matching ctor call to reproduce. Landed the full field-setup/string-building/switch/ StringList::load/write_strings/StringBox path faithfully (reusing the same local as StringStruct then StringList via reinterpret_cast, matching the disassembly's own reuse of one `this` across two mangled entry points); the two vtable-walk destructor passes at the end are omitted as they only free temporaries and don't produce any value this function returns.
void Datalink::draw_ability() {
    StringList help;
    const int ability_id = history_[0];
    const RulesAbility &ability = Ability[ability_id];
    set_font(&font1_, 0, 0, 0);
    set_text_color(0xE0, -1, 1, 1);
    StringTemp[0] = 0;
    strcat(StringTemp, StringTable.get(reinterpret_cast<int>(ability.name)));
    CharUpperA(StringTemp);
    write_cent_l(StringTemp, &title_rect_, strlen(StringTemp));
    StringTemp[0] = 0;
    if (ability.cost_factor >= 0) {
        char number[80];
        _itoa(ability.cost_factor, number, 10);
        strcat(StringTemp, number);
    } else {
        switch (ability.cost_factor) {
        case -1:
            strcat(StringTemp, StringTable.get(reinterpret_cast<int>(
                reinterpret_cast<LPSTR *>(Labels.strings_ptr)[0x1F0 / 4])));
            strcat(StringTemp, "/");
            strcat(StringTemp, StringTable.get(reinterpret_cast<int>(
                reinterpret_cast<LPSTR *>(Labels.strings_ptr)[0x154 / 4])));
            break;
        case -2:
            strcat(StringTemp, StringTable.get(reinterpret_cast<int>(
                reinterpret_cast<LPSTR *>(Labels.strings_ptr)[0x1F0 / 4])));
            strcat(StringTemp, "-1");
            break;
        case -3:
            strcat(StringTemp, StringTable.get(reinterpret_cast<int>(
                reinterpret_cast<LPSTR *>(Labels.strings_ptr)[0x154 / 4])));
            strcat(StringTemp, "-1");
            break;
        case -4:
            strcat(StringTemp, StringTable.get(reinterpret_cast<int>(
                reinterpret_cast<LPSTR *>(Labels.strings_ptr)[0xB14 / 4])));
            strcat(StringTemp, "-1");
            break;
        case -5:
            strcat(StringTemp, StringTable.get(reinterpret_cast<int>(
                reinterpret_cast<LPSTR *>(Labels.strings_ptr)[0x1F0 / 4])));
            strcat(StringTemp, "+");
            strcat(StringTemp, StringTable.get(reinterpret_cast<int>(
                reinterpret_cast<LPSTR *>(Labels.strings_ptr)[0x154 / 4])));
            strcat(StringTemp, "-2");
            break;
        case -6:
            strcat(StringTemp, StringTable.get(reinterpret_cast<int>(
                reinterpret_cast<LPSTR *>(Labels.strings_ptr)[0x1F0 / 4])));
            strcat(StringTemp, "+");
            strcat(StringTemp, StringTable.get(reinterpret_cast<int>(
                reinterpret_cast<LPSTR *>(Labels.strings_ptr)[0xB14 / 4])));
            strcat(StringTemp, "-2");
            break;
        case -7:
            strcat(StringTemp, StringTable.get(reinterpret_cast<int>(
                reinterpret_cast<LPSTR *>(Labels.strings_ptr)[0x154 / 4])));
            strcat(StringTemp, "+");
            strcat(StringTemp, StringTable.get(reinterpret_cast<int>(
                reinterpret_cast<LPSTR *>(Labels.strings_ptr)[0xB14 / 4])));
            strcat(StringTemp, "-2");
            break;
        }
    }
    parse_says(1, tech_name(ability.preq_tech, 0), -1, -1);
    parse_say(2, reinterpret_cast<int>(ability.description), -1, -1);
    parse_says(3, StringTemp, -1, -1);
    parse_num(0, ability.preq_tech + 140000);
    help.remove_all();
    help.load(DatalinkHelpFile, "HELPABIL", 1, 0);
    set_font(&body_font_, 0, 0, &link_font_);
    set_text_color(0x98, -1, 1, 1);
    set_text_color2(0x96, -1, 1, 1);
    set_text_color_hyper(0x96, -1, 1, 1);
    write_strings(&help, description_rect_.left + 6,
                  description_rect_.top + 6,
                  description_rect_.right - description_rect_.left - 12, 0);
    StringTemp[0] = 0;
    strcat(StringTemp, "ABILDESC");
    char number[80];
    _itoa(ability_id, number, 10);
    strcat(StringTemp, number);
    detail_box_.clear();
    if (detail_box_.strings().load(DatalinkHelpFile, StringTemp, 1, 0) == 0) {
        detail_box_.add_fixup();
        detail_box_.show(0);
    }
    soft_update(&description_rect_);
}
// ORIGINAL: 0x00431180 ?draw_terraform@Datalink@@QAEXXZ 0x00431180-0x00431D51;0x00653C7A-0x00653E66
// RECOVERY_BINDING: v1 {"evidence": ["git:6201d294:src/unrecovered/00431180.cpp"], "reason": "The Datalink owner implements this original topic renderer through typed rules, real Buffer/Caviar members and constructed Popup/StringList lifetimes; original intervals and seed provenance remain restoration inputs."}
// Historical scaffold experiment, retained from the replaced seed:
// TRIED: no SEH/frame modelling (plain body, not try/finally); the giant local widget teardown (Popup/Scroll/2xFlatButton/Spot/Dialogs region/Sprite/2xFlatButton/Heap) modelled as one raw ebp-relative byte buffer with explicit close()/dtor calls in original order, rather than real RAII locals - the vtable-poke-then-call shape the original uses for each teardown step is not reproduced, only the call itself (identity of the call target is masked by the scorer regardless). MISMATCH at instruction #0 (prologue: no SEH frame).
void Datalink::draw_terraform() {
    Popup popup;
    const int topic = history_[0];
    const bool land = topic < 20;
    const int action = topic % 20;
    const bool illustration = action < 16 || action > 18;
    const int x = description_rect_.left + 5;
    const int y = description_rect_.top + 5;
    Vert quad[4] = {{x, y + 50}, {x + 50, y + 25},
                    {x + 100, y + 50}, {x + 50, y + 75}};
    Vert uv[4] = {{0, 0}, {55, 0}, {55, 55}, {0, 55}};
    int z[4] = {0, 0, 0, 0};
    Buffer *canvas = static_cast<Buffer *>(this);
    if (illustration) {
        Texture &ground = land ? g_MOIST_LAND_TEXTURES[0] : g_OCEAN_TEXTURES[1];
        ground.draw(canvas, quad, z, 4, uv);
    }
    Sprite *sprite = 0;
    switch (action) {
    case 0:
        if (land) g_FARM_TEXTURES[0].draw(canvas, quad, z, 4, uv);
        else sprite = &g_TER1_KELP_FARM_SPRITE;
        break;
    case 1: sprite = &g_TER1_SOIL_ENRICHER_SPRITES[0]; break;
    case 2:
        sprite = land ? &g_TER1_MINE_SPRITE : &g_TER1_MINING_PLATFORM_SPRITE;
        break;
    case 3:
        sprite = land ? &g_TER1_SOLAR_COLLECTOR_SPRITE : &g_TER1_TIDAL_HARNESS_SPRITE;
        break;
    case 4: g_FOREST_TEXTURES[0].draw_trans(canvas, quad, z, 4, uv); break;
    case 5: g_ROAD_TEXTURES[0].draw_trans(canvas, quad, z, 4, uv); break;
    case 6: g_MAGTUBE_TEXTURES[0].draw_trans(canvas, quad, z, 4, uv); break;
    case 7: sprite = &g_TER1_BUNKER_SPRITE; break;
    case 8: sprite = &g_TER1_AIRBASE_SPRITE; break;
    case 9: sprite = &g_TER1_SENSOR_ARRAY_SPRITE; break;
    case 11:
        g_FUNGUS_TEXTURES[land ? 0 : 15].draw_trans(canvas, quad, z, 4, uv);
        break;
    case 12: sprite = &g_TER1_CONDENSER_SPRITE; break;
    case 13: sprite = &g_TER1_ECHELON_MIRROR_SPRITE; break;
    case 14: sprite = &g_TER1_BOREHOLE_SPRITE; break;
    case 15: g_RIVER_TEXTURES[0].draw_trans(canvas, quad, z, 4, uv); break;
    case 19: sprite = &g_TER1_MONOLITH_SPRITE; break;
    }
    if (sprite != 0) {
        sprite->draw(canvas, sprite->transparent_index(), x, y + 25, 1, 1);
    }
    set_font(&font1_, 0, 0, 0);
    set_text_color(0xE0, -1, 1, 1);
    StringTemp[0] = 0;
    const RulesOrder &order = Order[action + 4];
    strcat(StringTemp, StringTable.get(reinterpret_cast<int>(
        land ? order.order : order.order_sea)));
    CharUpperA(StringTemp);
    write_cent_l(StringTemp, &title_rect_, strlen(StringTemp));
    const RulesTerraforming &terraform = Terraforming[action];
    const int technology = land ? terraform.preq_tech : terraform.preq_tech_sea;
    parse_says(1, tech_name(technology, 0), -1, -1);
    parse_num(0, technology + 140000);
    parse_num(1, terraform.rate);
    char *extra = DatalinkTerraformExtraText;
    if (action == 3 || action == 5 || action == 6) {
        extra = StringTable.get(reinterpret_cast<int>(
            reinterpret_cast<LPSTR *>(Labels.strings_ptr)[0x1110 / 4]));
    }
    parse_says(2, extra, -1, -1);
    StringTemp[0] = 0;
    strcat(StringTemp, "HELPTERRAFORM");
    popup.start(DatalinkHelpFile, StringTemp, -1, 0, 0, 0);
    set_font(&body_font_, 0, 0, &link_font_);
    set_text_color(0x98, -1, 1, 1);
    set_text_color2(0x96, -1, 1, 1);
    set_text_color_hyper(0x96, -1, 1, 1);
    const int text_left = x + (illustration ? 103 : 0);
    write_strings(&popup.unit_ids_, text_left, y,
                  description_rect_.right - 5 - text_left, 0);
    StringTemp[0] = 0;
    strcat(StringTemp, "HELPTERRA");
    strcat(StringTemp, land ? "LAND" : "SEA");
    char number[16];
    _itoa(action, number, 10);
    strcat(StringTemp, number);
    popup.start(DatalinkHelpFile, StringTemp, -1, 0, 0, 0);
    set_font(&body_font_, 0, 0, &link_font_);
    set_text_color(0x98, -1, 1, 1);
    set_text_color2(0x96, -1, 1, 1);
    set_text_color_hyper(0x96, -1, 1, 1);
    const int second_left = static_cast<int>(field_10394_) + 5 +
                            (illustration ? 103 : 0);
    write_strings(&popup.unit_ids_, second_left,
                  static_cast<int>(field_10398_) + 5,
                  static_cast<int>(field_1039C_) - 5 - second_left, 0);
}
// ORIGINAL: 0x0042F490 ?draw_facility@Datalink@@QAEXXZ 0x0042F490-0x0042FAC5;0x0065384A-0x00653927
// RECOVERY_BINDING: v1 {"evidence": ["git:6201d294:src/unrecovered/0042f490.cpp"], "reason": "The Datalink owner implements this original topic renderer through typed rules, real Buffer/Caviar members and constructed Popup/StringList lifetimes; original intervals and seed provenance remain restoration inputs."}
// Historical scaffold experiment, retained from the replaced seed:
// TRIED: this function's tail is a compiler-generated SEH unwind chain for ~20 local C++ objects (Popup embeds Scroll+BasePop, and BasePop already embeds Heap/2xFlatButton/Sprite/dialogs_ per the scaffold, so declaring one `Popup popup;` local reproduces a lot of that chain automatically); the separate Dialog/StringStruct/sub_4066c0 teardown calls visible in the disassembly are NOT accounted for by any local this body declares, since their true declared type/order could not be pinned down from the given evidence - left unmodelled rather than guessed. Ghidra's own decompile of this function is unusable (SEH state-machine noise, not real control flow); this body was written directly from the raw disassembly instead. popup.start()/write_strings() receiver args (the ebp-0x3274 StringList) are approximated with a fresh default-constructed local, since the real asm never visibly initializes that memory in view.
void Datalink::draw_facility() {
    Popup popup;
    int id = history_[0];
    int left = description_rect_.left + 3;
    int top = description_rect_.top + 3;
    int right = description_rect_.right - 3;
    set_font(&font1_, 0, 0, 0);
    set_text_color(0xE0, -1, 1, 1);
    StringTemp[0] = 0;
    strcat(StringTemp, StringTable.get(reinterpret_cast<int>(Facility[id].name)));
    CharUpperA(StringTemp);
    write_cent_l(StringTemp, &title_rect_, strlen(StringTemp));
    Sprite &icon = g_FACILITY_ICON_SPRITES[id];
    int image_width = illustration_rect_.right - illustration_rect_.left;
    icon.draw(this, icon.transparent_index(), illustration_rect_.left,
              illustration_rect_.top, image_width, icon.width());
    left += image_width + 6;
    parse_num(0, cost_factor(ConsoleGlobal.faction_id_, 1, -1) * Facility[id].cost);
    parse_num(1, fac_maint(id, ConsoleGlobal.faction_id_));
    int technology = Facility[id].preq_tech;
    parse_says(1, tech_name(technology, 0), -1, -1);
    parse_num(2, technology + 140000);
    StringTemp[0] = 0;
    strcat(StringTemp, "HELPFAC");
    popup.start(DatalinkHelpFile, StringTemp, -1, 0, 0, 0);
    set_font(&body_font_, 0, 0, technology >= 0 ? &link_font_ : 0);
    set_text_color(0x98, -1, 1, 1);
    set_text_color2(0x96, -1, 1, 1);
    set_text_color_hyper(0x96, -1, 1, 1);
    write_strings(&popup.unit_ids_, left, top, right - left, 0);
    StringTemp[0] = 0;
    strcat(StringTemp, "HELPFAC");
    char number[80];
    _itoa(id, number, 10);
    strcat(StringTemp, number);
    popup.start(DatalinkHelpFile, StringTemp, -1, 0, 0, 0);
    set_font(&body_font_, 0, 0, technology >= 0 ? &link_font_ : 0);
    set_text_color(0x98, -1, 1, 1);
    set_text_color2(0x96, -1, 1, 1);
    set_text_color_hyper(0x96, -1, 1, 1);
    left = field_10394_ + 3;
    top = field_10398_ + 3;
    right = field_1039C_ - 3;
    write_strings(&popup.unit_ids_, left, top, right - left, 0);
    StringTemp[0] = 0;
    strcat(StringTemp, "HELPFAC");
    _itoa(id, number, 10);
    strcat(StringTemp, number);
    if (MainInterfaceTileFlag) {
        blurb_box_.clear();
        blurb_box_.strings().load(DatalinkBlurbFile, StringTemp, 1, 0);
        blurb_box_.add_fixup();
        blurb_box_.show(0);
    }
}
// ORIGINAL: 0x0042FAD0 ?draw_project@Datalink@@QAEXXZ 0x0042FAD0-0x00430087;0x00653927-0x006539F9
// RECOVERY_BINDING: v1 {"evidence": ["git:6201d294:src/unrecovered/0042fad0.cpp"], "reason": "The Datalink owner implements this original topic renderer through typed rules, real Buffer/Caviar members and constructed Popup/StringList lifetimes; original intervals and seed provenance remain restoration inputs."}
// Historical scaffold experiment, retained from the replaced seed:
// TRIED: exact local-object footprint for Popup's embedded destructor cascade (same shape as 0x00430C80's draw_faction) - modelled as a single `Popup popup;` with popup.close() and implicit dtor; business logic (project/tech title, cost_factor/parse_num/ tech_name/parse_says formatting, sprite draw, two write_strings panels, StringBox blurb branch) transcribed field-by-field.
void Datalink::draw_project() {
    Popup popup;
    int id = history_[0];
    int left = description_rect_.left + 3;
    int top = description_rect_.top + 3;
    int right = description_rect_.right - 3;
    set_font(&font1_, 0, 0, 0);
    set_text_color(0xE0, -1, 1, 1);
    StringTemp[0] = 0;
    strcat(StringTemp, StringTable.get(reinterpret_cast<int>(Facility[id].name)));
    CharUpperA(StringTemp);
    write_cent_l(StringTemp, &title_rect_, strlen(StringTemp));
    Sprite &icon = g_SECRET_PROJECT_ICON_SPRITES[id - 70];
    int image_width = illustration_rect_.right - illustration_rect_.left;
    // 42FBEC loads the first project sprite's +0x18, without indexing it.
    icon.draw(this, icon.transparent_index(), illustration_rect_.left,
              illustration_rect_.top, image_width, g_SECRET_PROJECT_ICON_SPRITES[0].width());
    left += image_width + 6;
    parse_num(0, cost_factor(ConsoleGlobal.faction_id_, 1, -1) * Facility[id].cost);
    parse_num(1, Facility[id].maint);
    int technology = Facility[id].preq_tech;
    parse_says(1, tech_name(technology, 0), -1, -1);
    parse_num(2, technology + 140000);
    popup.start(DatalinkHelpFile, "HELPPROJ", -1, 0, 0, 0);
    set_font(&body_font_, 0, 0, &link_font_);
    set_text_color(0x98, -1, 1, 1);
    set_text_color2(0x96, -1, 1, 1);
    set_text_color_hyper(0x96, -1, 1, 1);
    write_strings(&popup.unit_ids_, left, top, right - left, 0);
    StringTemp[0] = 0;
    strcat(StringTemp, "HELPPROJ");
    char number[80];
    _itoa(id, number, 10);
    strcat(StringTemp, number);
    popup.start(DatalinkHelpFile, StringTemp, -1, 0, 0, 0);
    set_font(&body_font_, 0, 0, &link_font_);
    set_text_color(0x98, -1, 1, 1);
    set_text_color2(0x96, -1, 1, 1);
    set_text_color_hyper(0x96, -1, 1, 1);
    left = field_10394_ + 3;
    top = field_10398_ + 3;
    right = field_1039C_ - 3;
    write_strings(&popup.unit_ids_, left, top, right - left, 0);
    StringTemp[0] = 0;
    strcat(StringTemp, "PROJECT");
    _itoa(id, number, 10);
    strcat(StringTemp, number);
    if (MainInterfaceTileFlag) {
        blurb_box_.clear();
        blurb_box_.strings().load(DatalinkBlurbFile, StringTemp, 1, 0);
        blurb_box_.add_fixup();
        blurb_box_.show(0);
    }
}
// ORIGINAL: 0x00430090 ?draw_social@Datalink@@QAEXXZ 0x00430090-0x00430765;0x006539F9-0x00653AD6
// RECOVERY_BINDING: v1 {"evidence": ["git:6201d294:src/unrecovered/00430090.cpp"], "reason": "The Datalink owner implements this original topic renderer through typed rules, real Buffer/Caviar members and constructed Popup/StringList lifetimes; original intervals and seed provenance remain restoration inputs."}
// Historical scaffold experiment, retained from the replaced seed:
// TRIED: Popup popup; (RAII) for the outer object, mechanical RD()-based field reads for the tech-tier/quotient-remainder lookups, itoa/strcat scratch buffer building, and the per-faction tally loop; deliberately did NOT model the second local object built by sub_4066c0 (Dialog/Dialogs/ StringStruct/Sprite/extra FlatButtons torn down around 0x430642) - its class is unrecoverable from this brief, so popup.close() + RAII stands in for that whole cascade. NO_COMPILE C2660 (set_font arg count) fixed first; MISMATCH #0 (prologue) not chased further.
void Datalink::draw_social() {
    Popup popup;
    int category = history_[0] / 5;
    int model = history_[0] % 5 - 1;
    int left = description_rect_.left + 3;
    int top = description_rect_.top + 3;
    int right = description_rect_.right - 3;
    set_font(&font1_, 0, 0, 0);
    set_text_color(0xE0, -1, 1, 1);
    StringTemp[0] = 0;
    char *name = model < 0 ? SocialCategories[category].type : SocialCategories[category].name[model];
    strcat(StringTemp, StringTable.get(reinterpret_cast<int>(name)));
    CharUpperA(StringTemp);
    write_cent_l(StringTemp, &title_rect_, strlen(StringTemp));
    if (model >= 0) {
        int technology = SocialCategories[category].preq_tech[model];
        parse_says(0, tech_name(technology, 0), -1, -1);
        parse_num(0, technology + 140000);
    }
    StringTemp[0] = 0;
    strcat(StringTemp, "HELPSOC");
    char number[80];
    _itoa(category, number, 10);
    strcat(StringTemp, number);
    if (model >= 0) {
        _itoa(model, number, 10);
        strcat(StringTemp, number);
    }
    popup.start(DatalinkHelpFile, StringTemp, -1, 0, 0, 0);
    set_font(&body_font_, 0, 0, &link_font_);
    set_text_color(0x98, -1, 1, 1);
    set_text_color2(0x96, -1, 1, 1);
    set_text_color_hyper(0x96, -1, 1, 1);
    write_strings(&popup.unit_ids_, left, top, right - left, 0);
    left = field_10394_ + 3;
    top = field_10398_ + 3;
    right = field_1039C_ - 3;
    int bottom = field_103A0_ - 3;
    if (model >= 0) {
        const SocialEffect &effect = SocialCategories[category].model_effect[model];
        int values[11] = {effect.values_[SocialEffect::ECONOMY], effect.values_[SocialEffect::EFFICIENCY], effect.values_[SocialEffect::SUPPORT],
            effect.values_[SocialEffect::TALENT], effect.values_[SocialEffect::MORALE], effect.values_[SocialEffect::POLICE], effect.values_[SocialEffect::GROWTH],
            effect.values_[SocialEffect::PLANET], effect.values_[SocialEffect::PROBE], effect.values_[SocialEffect::INDUSTRY], effect.values_[SocialEffect::RESEARCH]};
        int count = 0;
        int i;
        for (i = 0; i < 11; ++i) if (values[i] != 0) ++count;
        left += 20;
        if (count == 0) {
            char *text = label_get(0xFD4 / 4);
            if (text) {
                RECT rect = {left, top, right, bottom};
                write_l(text, &rect, strlen(text));
            }
        } else {
            int line_height = body_font_.line_advance();
            int y = top + (bottom - line_height * count - top) / 2;
            for (i = 0; i < 11; ++i) {
                if (values[i] != 0) {
                    StringTemp[0] = 0;
                    _itoa(values[i], number, 10);
                    strcat(StringTemp, number);
                    write_right_l(StringTemp, left, y, strlen(StringTemp));
                    StringTemp[0] = 0;
                    strcat(StringTemp, " ");
                    char link[80];
                    _itoa(i + 130000, link, 10);
                    strcat(StringTemp, "$LINK<");
                    strcat(StringTemp, SocialEffects[i].set2);
                    strcat(StringTemp, "=");
                    strcat(StringTemp, link);
                    strcat(StringTemp, ">");
                    write_l(StringTemp, left, y, strlen(StringTemp));
                    y += body_font_.line_advance();
                }
            }
        }
    }
}
// ORIGINAL: 0x00430C80 ?draw_faction@Datalink@@QAEXXZ 0x00430C80-0x0043117E;0x00653BA8-0x00653C7A
// RECOVERY_BINDING: v1 {"evidence": ["git:6201d294:src/unrecovered/00430c80.cpp"], "reason": "The Datalink owner implements this original topic renderer through typed rules, real Buffer/Caviar members and constructed Popup/StringList lifetimes; original intervals and seed provenance remain restoration inputs."}
// Historical scaffold experiment, retained from the replaced seed:
// TRIED: exact local-object footprint for Popup's embedded Scroll/FlatButton/ Heap/Sprite/Spot/Dialogs cascade (huge MI vtable-poke cleanup chain, 0x53a0-byte frame) - modelled as a single `Popup popup;` with popup.close() and implicit dtor instead of the full inlined destructor cascade; business logic (font/text-color/string/sprite/ popup.start branches) transcribed field-by-field from the disasm.
void Datalink::draw_faction() {
    Popup popup;
    int faction = history_[0];
    int left = panel_rect_.left + 5;
    int top = panel_rect_.top + 5;
    int right = panel_rect_.right - 5;
    set_font(&font1_, 0, 0, 0);
    set_text_color(0xE0, -1, 1, 1);
    StringTemp[0] = 0;
    strcat(StringTemp, Players[faction].formal_name_faction);
    CharUpperA(StringTemp);
    write_cent_l(StringTemp, &title_rect_, strlen(StringTemp));
    Sprite &portrait = FactionsArt[faction].datalink;
    portrait.draw(this, portrait.transparent_index(), left, top, 1, 1);
    left += portrait.width() + 5;
    char *file = Players[faction].filename;
    popup.start(file, "DATALINKS1", -1, 0, 0, 0);
    set_font(&body_font_, 0, 0, 0);
    set_wrap_width(20);
    set_text_color(0x98, -1, 1, 1);
    set_text_color2(0x96, -1, 1, 1);
    set_text_color_hyper(0xFE, -1, 1, 1);
    int width = right - left;
    top = write_strings(&popup.unit_ids_, left, top, width, 0) + body_font_.line_advance();
    if (WinScreenWidth == 800) {
        set_wrap_width(0);
        stringBox_.clear();
        if (stringBox_.strings().load(file, "DATALINKS2", 1, 0) == 0) {
            stringBox_.add_fixup();
            stringBox_.show(0);
        }
    } else {
        popup.start(file, "DATALINKS2", -1, 0, 0, 0);
        set_font(&secondary_body_font_, 0, 0, 0);
        write_strings(&popup.unit_ids_, left, top, width, 0);
        set_wrap_width(0);
    }
    popup.start(file, "BLURB", -1, 0, 0, 0);
    if (MainInterfaceTileFlag) {
        blurb_box_.clear();
        blurb_box_.strings().load(file, "BLURB", 1, 0);
        blurb_box_.add_fixup();
        blurb_box_.show(0);
    }
}

// ORIGINAL: 0x0042A100 ?on_dialog_back_draw@Datalink@@QAEHPAUGraphicWin@@@Z 0x0042A100-0x0042A150 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x42A100", "git:8e3029104ab590bec35cc6932e637765b2ea4074:src/recovered/units/0042a100.cpp"], "reason": "Recover the seed identity as a typed Datalink member using real window, font, list, and popup lifetimes."}
// symbol    ?on_dialog_back_draw@Datalink@@QAEHPAVGraphicWin@@@Z
int Datalink::on_dialog_back_draw(GraphicWin *win) {
    RECT rect = win->clip_rect();
    const RECT &outer = win->outer_rect();
    win->tile(InterfaceBoxStyles[0].mask_, outer.left, outer.top,
              0, 0, rect.right - rect.left, rect.bottom - rect.top);
    return 1;
}

// ORIGINAL: 0x0042A150 ?on_dialog_item_back_draw@Datalink@@QAEHPAUGraphicWin@@HHPAURECT@@@Z 0x0042A150-0x0042A260
// RECOVERY_BINDING: v1 {"evidence": ["image:0x42A150", "git:8e3029104ab590bec35cc6932e637765b2ea4074:src/unrecovered/0042a150.cpp"], "reason": "Recover the seed identity as a typed Datalink member using real window, font, list, and popup lifetimes."}
// symbol    ?on_dialog_item_back_draw@Datalink@@QAEHPAVGraphicWin@@HHPAUtagRECT@@@Z
// Historical scaffold experiment, retained from the replaced seed:
// TRIED: computing width/height/x/y once and branching only on the `tile()` colour argument folds the compiler's output to 187 bytes (vs 272) - VC6 does not hoist a same-polarity guard's body out of line (see 0x0042A100's sibling note), so each of the three `a3` cases needs its own fully-duplicated width/height/x/y/call block. That form (cand2, below) reaches 0.834 mnemonic similarity but still differs on the exact ebx/esi/edi register allocation the original chose; landing the closest.
int Datalink::on_dialog_item_back_draw(GraphicWin *win, int id, int mode,
                                      RECT *rect) {
    const RECT &outer = win->outer_rect();
    int style = mode == 2 ? 28 : (mode == 1 ? 27 : 0);
    win->tile(InterfaceBoxStyles[style].mask_, outer.left + rect->left,
              outer.top + rect->top, rect->left, rect->top,
              rect->right - rect->left, rect->bottom - rect->top);
    if (field_29DC_ == 12) {
        win->set_text_color(255 - (id % 10000) / 5, -1, 1, 1);
    }
    return 1;
}

// ORIGINAL: 0x0042BEB0 ?UNK7@Datalink@@QAEXPADPAD@Z 0x0042BEB0-0x0042BEFA BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x42BEB0", "git:8e3029104ab590bec35cc6932e637765b2ea4074:src/recovered/0042beb0.cpp"], "reason": "Recover the seed identity as a typed Datalink member using real window, font, list, and popup lifetimes."}
// symbol    ?UNK7@Datalink@@QAEXPAD0@Z
void Datalink::UNK7(char *file, char *section) {
    if (MainInterfaceTileFlag) {
        blurb_box_.clear();
        blurb_box_.strings().load(file, section, 1, 0);
        blurb_box_.add_fixup();
        blurb_box_.show(0);
    }
}

// ORIGINAL: 0x0042C2E0 ?draw_concept@Datalink@@QAEXXZ 0x0042C2E0-0x0042C64D;0x0065358E-0x00653650
// RECOVERY_BINDING: v1 {"evidence": ["image:0x42C2E0", "git:8e3029104ab590bec35cc6932e637765b2ea4074:src/recovered/units/0042c2e0.cpp"], "reason": "Recover the seed identity as a typed Datalink member using real window, font, list, and popup lifetimes."}
void Datalink::draw_concept() {
    Popup popup;
    set_font(&font1_, 0, 0, 0);
    set_text_color(0xE0, -1, 1, 1);
    StringTemp[0] = 0;
    listBox_.item_list_.seek_id(history_[0]);
    strcat(StringTemp, listBox_.item_list_.current_text());
    CharUpperA(StringTemp);
    write_cent_l(StringTemp, &title_rect_, strlen(StringTemp));
    StringTemp[0] = 0;
    strcat(StringTemp, "CONCEPT");
    char number[80];
    _itoa(history_[0], number, 10);
    strcat(StringTemp, number);
    concept_box_.clear();
    concept_box_.strings().load("CONCEPTSX", StringTemp, 1, 0);
    concept_box_.add_fixup();
    concept_box_.show(0);
    soft_update(&description_rect_);
}

// ORIGINAL: 0x0042C650 ?draw_adv_concept@Datalink@@QAEXXZ 0x0042C650-0x0042C9C0;0x00653650-0x00653712
// RECOVERY_BINDING: v1 {"evidence": ["image:0x42C650", "git:8e3029104ab590bec35cc6932e637765b2ea4074:src/recovered/units/0042c650.cpp"], "reason": "Recover the seed identity as a typed Datalink member using real window, font, list, and popup lifetimes."}
void Datalink::draw_adv_concept() {
    Popup popup;
    set_font(&font1_, 0, 0, 0);
    set_text_color(0xE0, -1, 1, 1);
    StringTemp[0] = 0;
    listBox_.item_list_.seek_id(history_[0] + 10000);
    strcat(StringTemp, listBox_.item_list_.current_text());
    CharUpperA(StringTemp);
    write_cent_l(StringTemp, &title_rect_, strlen(StringTemp));
    StringTemp[0] = 0;
    strcat(StringTemp, "ADVCONCEPT");
    char number[80];
    _itoa(history_[0], number, 10);
    strcat(StringTemp, number);
    concept_box_.clear();
    concept_box_.strings().load("CONCEPTSX", StringTemp, 1, 0);
    concept_box_.add_fixup();
    concept_box_.show(0);
    soft_update(&description_rect_);
}

// ORIGINAL: 0x00430770 ?draw_effect@Datalink@@QAEXXZ 0x00430770-0x00430B30;0x00653AD6-0x00653BA8
// RECOVERY_BINDING: v1 {"evidence": ["image:0x430770", "git:8e3029104ab590bec35cc6932e637765b2ea4074:src/recovered/units/00430770.cpp"], "reason": "Recover the preserved topic drawer with source-owned state and real popup/window lifetimes."}
void Datalink::draw_effect() {
    Popup popup;
    set_font(&font1_, 0, 0, 0);
    set_text_color(0xE0, -1, 1, 1);
    StringTemp[0] = 0;
    strcat(StringTemp, SocialEffects[history_[0]].set2);
    CharUpperA(StringTemp);
    write_cent_l(StringTemp, &title_rect_, strlen(StringTemp));
    StringTemp[0] = 0;
    strcat(StringTemp, "HELPEFFECT");
    char number[80];
    _itoa(history_[0], number, 10);
    strcat(StringTemp, number);
    popup.start(DatalinkHelpFile, StringTemp, -1, 0, 0, 0);
    set_font(&body_font_, 0, 0, &link_font_);
    set_text_color(0x98, -1, 1, 1);
    set_text_color2(0x96, -1, 1, 1);
    set_text_color_hyper(0x96, -1, 1, 1);
    // The original Popup starts at [ebp-0x53F0] (0x430797/0x4308A3).
    // 0x430909 passes [ebp-0x3270] to Buffer::write_strings: exactly
    // Popup+0x2180, BasePop's constructed unit_ids_ text list. BasePop::start
    // appends parsed ordinary lines to that list; no separate local list
    // is constructed here.
    write_strings(&popup.unit_ids_, description_rect_.left + 3,
                  description_rect_.top + 3,
                  description_rect_.right - description_rect_.left - 6, 0);
}

// ORIGINAL: 0x00430B30 ?draw_tech@Datalink@@QAEXXZ 0x00430B30-0x00430C76
// RECOVERY_BINDING: v1 {"evidence": ["image:0x430B30", "git:8e3029104ab590bec35cc6932e637765b2ea4074:src/recovered/units/00430b30.cpp"], "reason": "Recover the preserved topic drawer with source-owned state and real popup/window lifetimes."}
void Datalink::draw_tech() {
    set_font(&font1_, 0, 0, 0);
    set_text_color(0xE0, -1, 1, 1);
    StringTemp[0] = 0;
    strcat(StringTemp, StringTable.get(reinterpret_cast<int>(Technology[history_[0]].name)));
    CharUpperA(StringTemp);
    // Both the standalone original (0x430BA5..0x430BAC) and draw_entry's
    // inline technology case (0x42C1A7..0x42C1AE) test the buffer ADDRESS,
    // not its first byte. StringTemp has real storage, so even an empty
    // technology title reaches write_cent_l.
    write_cent_l(StringTemp, &title_rect_, strlen(StringTemp));
    ReportWinGlobal.reset();
    ReportWinGlobal.shared_research_focus() = history_[0];
    ReportIfGlobal.field_8_ = 2;
    ReportWinGlobal.draw_labs();
    StringTemp[0] = 0;
    strcat(StringTemp, "TECH");
    char number[80];
    _itoa(history_[0], number, 10);
    strcat(StringTemp, number);
    UNK7(DatalinkBlurbFile, StringTemp);
}

// ORIGINAL: 0x0042BF10 ?draw_entry@Datalink@@QAEXXZ 0x0042BF10-0x0042C29A
// RECOVERY_BINDING: v1 {"evidence": ["image:0x42BF10", "git:8e3029104ab590bec35cc6932e637765b2ea4074:src/recovered/units/0042bf10.cpp"], "reason": "Recover the seed identity as a typed Datalink member using real window, font, list, and popup lifetimes."}
void Datalink::draw_entry() {
    restore_clip();
    if (field_29DC_ == 2) {
        ReportWinGlobal.reset();
    }
    clear_links();
    int selected = listBox_.get_selected_id();
    int category = selected / 10000;
    history_[0] = selected - category * 10000;
    if (!field_2A38_) {
        push_history(category, history_[0]);
    }
    set_clip(&entry_clip_rect_);
    RECT frame = entry_frame_rect_;
    frame.left -= 23;
    frame.right -= 9;
    box_sprite(&frame, &InterfaceBoxStyles[9]);
    restore_clip();
    box_sprite(&panel_rect_, &InterfaceBoxStyles[0]);
    box_sprite(&title_rect_, &InterfaceBoxStyles[6]);
    blurb_box_.hide();
    concept_box_.hide();
    detail_box_.hide();
    stringBox_.hide();
    switch (category) {
    case 0: draw_concept(); break;
    case 1: draw_adv_concept(); break;
    case 2: break;
    case 3: draw_unit(); break;
    case 4: draw_chassis(); break;
    case 5: draw_reactor(); break;
    case 6: draw_weapon(); break;
    case 7: draw_defense(); break;
    case 8: draw_ability(); break;
    case 9: draw_terraform(); break;
    case 10: draw_facility(); break;
    case 11: draw_project(); break;
    case 12: draw_social(); break;
    case 13: draw_effect(); break;
    case 14: draw_tech(); break;
    case 15: draw_faction(); break;
    }
    soft_update(&entry_update_rect_);
    MainInterfaceGlobal.soft_update(&interface_update_rect_);
}

/*
 * Fixed-slot bindings the homed category fillers arrived carrying, named from
 * evidence. Each folds to the bare immediate, so a rename moves no byte.
 *
 * CategoryIdSeparator  0x006832CC - image .data reads "|" (0x7C, NUL). It is
 *                      the first of seven one-character separator strings
 *                      packed four bytes apart (0x6832CC..0x6832E4), one per
 *                      category filler, and each filler appends it before the
 *                      id suffix that `GameState & 0x1000` asks for.
 * Panel frames borrow InterfaceBoxStyles[6], original 0x78D618, from the
 * loader-owned shared pool rather than a fixed-address record.
 * CategoryEntryRect    the category_rect_ member, not a global - listed here
 *                      only to say the fillers share it.
 */
static const char *const CategoryIdSeparator = (const char *)0x006832CC;
static const char *const TerraformSecondSeparator = (const char *)0x0068333C;
static const char *const ConceptArticle = (const char *)0x006832E8;
static const char *const ConceptFile = (const char *)0x006832F8;
static const char *const ConceptSection = (const char *)0x00683300;
static const char *const AdvConceptFile = (const char *)0x00683310;

/*
 * The rules tables the remaining fillers walk. Each is named from what its
 * own loop does - the id range it files and the stride it steps - and from
 * the tree's own neighbours where those pin it:
 *
 *   SecretProjectTable   0x009A5888, 64 records of 0x30, ids 68534..68597,
 *                        preq at +0x14. base.h's SecretProject (0x009A6514)
 *                        sits just past its end, which is what makes this a
 *                        measurement and not a guess.
 *   TechRowBase          0x0094F380 + 9 dwords, so the walk's `rec[-9]` name
 *                        read lands on the technology record's own head.
 *   SocialCategoryRow    0x0094B014, SocialCategories (faction.h,
 *                        0x0094B000) + 0x14 - the first category's first
 *                        choice, whose header id the walk reads at -5.
 *   SocialEffectRowBase  0x00946598, SocialEffects (faction.h, 0x00946580)
 *                        + 0x18, rows 0x68 apart, end 0x00946A10; the Null
 *                        row the filler skips is 0x009466D0.
 *   FactionRowBase       0x009472EC, records 0x167 bytes, ids 150001..150008.
 *   TerraformNameBase    0x0096C8AC, three dwords per entry.
 *   TerraformFlagBase    0x00691884, eight dwords per entry - the preq words
 *                        the walk reads at -1 and 0.
 *   ProjectPrefixHidden  0x009BC054, clear while the "The " article is to be
 *                        stripped.
 *   the faction filler publishes each record's head dwords to general.h's
 *   GenderDefault / PluralityDefault pair before filing its name, and reads
 *   alpha.h's Language to decide whether the "The " article stays.
 */
static RulesFacility *const SecretProjectRules = (RulesFacility *)0x009A5888;
static const int *const TechRowBase = (const int *)0x0094F3A4;
static const int *const SocialCategoryRow = (const int *)0x0094B014;
static const char *const SocialEffectRowBase = (const char *)0x00946598;
static const char *const SocialEffectNullRow = (const char *)0x009466D0;
static const char *const SocialEffectRowEnd = (const char *)0x00946A10;
static const char *const FactionRowBase = (const char *)0x009472EC;
static const int *const TerraformNameBase = (const int *)0x0096C8AC;
static const int *const TerraformFlagBase = (const int *)0x00691884;

// The label each category's filler draws centred in the entry rectangle, read
// out of the label blob the way dipedit.cpp reads it:
// `mov eax, ds:0x9b90f8; mov ecx, [eax + <slot>]` - slot / 4 is the index.
#define CATEGORY_LABEL(slot) StringTable.get((int) * ((LPSTR *)Labels.strings_ptr + (slot)))

// See datalink.h. Defined out of line only so VCall's declaration, which
// basewin.cpp also carries locally, is not dragged into every TU that includes
// this header; /Ob2 re-inlines the body at each of the five callers below.
void Datalink::repaint_slot() {
    listBox_.redraw();
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x0042BEA0 ?UNK6@Datalink@@QAEXXZ 0x0042BEA0-0x0042BEA1 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK6@Datalink@@QAEXXZ)(Datalink* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::UNK6() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x0042BF00 ?UNK8@Datalink@@QAEXXZ 0x0042BF00-0x0042BF01 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK8@Datalink@@QAEXXZ)(Datalink* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::UNK8() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x00431DB0 ?UNK9@Datalink@@QAEXXZ 0x00431DB0-0x00431DB1 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK9@Datalink@@QAEXXZ)(Datalink* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::UNK9() {
}




/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00431EB0 ?on_left_click@Datalink@@QAEXHH@Z 0x00431EB0-0x00431EB3 BYTE_EXACT
// symbol    ?on_left_click@Datalink@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_left_click@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::on_left_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00431FB0 ?on_group_clicked@Datalink@@QAEXHH@Z 0x00431FB0-0x00431FB3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_group_clicked@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::on_group_clicked(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00432000 ?on_mouse_move@Datalink@@QAEXHH@Z 0x00432000-0x00432003 BYTE_EXACT
// symbol    ?on_mouse_move@Datalink@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_mouse_move@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::on_mouse_move(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00432160 ?on_iface_left_click@Datalink@@QAEXHH@Z 0x00432160-0x00432163 BYTE_EXACT
// symbol    ?on_iface_left_click@Datalink@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_click@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_left_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00432170 ?on_iface_right_click@Datalink@@QAEXHH@Z 0x00432170-0x00432173 BYTE_EXACT
// symbol    ?on_iface_right_click@Datalink@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_click@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_right_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00432180 ?on_iface_left_down@Datalink@@QAEXHH@Z 0x00432180-0x00432183 BYTE_EXACT
// symbol    ?on_iface_left_down@Datalink@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_down@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_left_down(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00432190 ?on_iface_right_down@Datalink@@QAEXHH@Z 0x00432190-0x00432193 BYTE_EXACT
// symbol    ?on_iface_right_down@Datalink@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_down@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_right_down(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004321A0 ?on_iface_mouse_move@Datalink@@QAEXHH@Z 0x004321A0-0x004321A3 BYTE_EXACT
// symbol    ?on_iface_mouse_move@Datalink@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_mouse_move@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_mouse_move(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004321B0 ?on_iface_mouse_leave@Datalink@@QAEXHH@Z 0x004321B0-0x004321B3 BYTE_EXACT
// symbol    ?on_iface_mouse_leave@Datalink@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_mouse_leave@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_mouse_leave(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004321C0 ?on_iface_selected@Datalink@@QAEXHH@Z 0x004321C0-0x004321C3 BYTE_EXACT
// symbol    ?on_iface_selected@Datalink@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_selected@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_selected(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004321D0 ?on_iface_left_double_click@Datalink@@QAEXHH@Z 0x004321D0-0x004321D3 BYTE_EXACT
// symbol    ?on_iface_left_double_click@Datalink@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_double_click@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_left_double_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004321E0 ?on_iface_right_double_click@Datalink@@QAEXHH@Z 0x004321E0-0x004321E3 BYTE_EXACT
// symbol    ?on_iface_right_double_click@Datalink@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_double_click@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_right_double_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004321F0 ?on_iface_button_clicked@Datalink@@QAEXH@Z 0x004321F0-0x004321F3 BYTE_EXACT
// symbol    ?on_iface_button_clicked@Datalink@@UAEXH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_button_clicked@Datalink@@QAEXH@Z)(Datalink* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_button_clicked(int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00432200 ?on_iface_button_toggled@Datalink@@QAEXHH@Z 0x00432200-0x00432203 BYTE_EXACT
// symbol    ?on_iface_button_toggled@Datalink@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_button_toggled@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_button_toggled(int, int) {
}


// The image routes this callback through DatalinkIface slot 16. The compiler
// adjusts the secondary-base receiver back to Datalink; the seed's +0xEC14
// ListBox access from that receiver is the owned listBox_ at +0xF628.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00432210 ?on_iface_group_clicked@Datalink@@QAEXHHH@Z 0x00432210-0x0043228B BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/00432210.cpp preserves the 123-byte BYTE_EXACT historical callback; image:0x0066AF30 slot 16 points to 0x00432210; source:src/datalink.h models DatalinkIface at +0xA14 and ListBox at +0xF628"], "reason": "Restore the category-click path through the real Datalink owner and ListBox member, with the compiler's secondary-base adjustment rather than seed pointer arithmetic."}
// symbol    ?on_iface_group_clicked@Datalink@@UAEXHHH@Z
void Datalink::on_iface_group_clicked(int, int, int category) {
    if (listBox_.item_list_.entry_count_ != 0) {
        set_cat(category, 0);
        // 0x43224B/0x43225A use the secondary receiver (full this+0xA14).
        // Its +0x2024 is full Datalink+0x2A38: suppress history during selection.
        field_2A38_ = 1;
        listBox_.set_selected_pos(0, 1);
        field_2A38_ = 0;
        listBox_.redraw();
        draw_entry();
    }
}


/*
Purpose: Close the datalink display, clearing the three values it caches.
// ORIGINAL: 0x0042BD20 ?close@Datalink@@QAEXXZ 0x0042BD20-0x0042BD35 BYTE_EXACT
// size      21 bytes
// prototype void (__thiscall ?close@Datalink@@QAEXXZ)(Datalink* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::close() {
    history_[0] = 0;
    field_2A34_ = 0;
    field_2A38_ = 0;
}


/*
Purpose: Show the technology help topic for the given id.
// ORIGINAL: 0x0044C880 ?help_tech@@YAXH@Z 0x0044C880-0x0044C895 BYTE_EXACT
// size      21 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
void __cdecl help_tech(int id) {
    DatalinkMain.exec(0xE, id);
}

/*
Purpose: Show the weapon help topic for the given id.
// ORIGINAL: 0x0044C910 ?help_weapon@@YAXH@Z 0x0044C910-0x0044C925 BYTE_EXACT
// size      21 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
// BODY IN datalink.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Show the armor help topic for the given id.
// ORIGINAL: 0x0044C940 ?help_armor@@YAXH@Z 0x0044C940-0x0044C955 BYTE_EXACT
// size      21 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
// BODY IN datalink.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Show the chassis help topic for the given id.
// ORIGINAL: 0x0044C980 ?help_chassis@@YAXH@Z 0x0044C980-0x0044C995 BYTE_EXACT
// size      21 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
// BODY IN datalink.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Show the facility help topic for the given id.
// ORIGINAL: 0x0044C9B0 ?help_facility@@YAXH@Z 0x0044C9B0-0x0044C9C5 BYTE_EXACT
// size      21 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
// BODY IN datalink.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Show the secret project/ability help topic for the given id.
// ORIGINAL: 0x0044CA10 ?help_abil@@YAXH@Z 0x0044CA10-0x0044CA25 BYTE_EXACT
// size      21 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
// BODY IN datalink.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Show the social engineering help topic for the given id.
// ORIGINAL: 0x0044CA40 ?help_social@@YAXH@Z 0x0044CA40-0x0044CA55 BYTE_EXACT
// size      21 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
// BODY IN datalink.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Show the faction help topic for the given id.
// ORIGINAL: 0x0044CA70 ?help_faction@@YAXH@Z 0x0044CA70-0x0044CA85 BYTE_EXACT
// size      21 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
// BODY IN datalink.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Show the vehicle help topic for the given id.
// ORIGINAL: 0x0044CAD0 ?help_veh@@YAXH@Z 0x0044CAD0-0x0044CAE5 BYTE_EXACT
// size      21 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
// BODY IN datalink.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Show an arbitrary help topic/index pair, for callers that already
         hold the topic id rather than going through one of the typed
         help_* wrappers above.
// ORIGINAL: 0x0044CB60 ?help_topic@@YAXIH@Z 0x0044CB60-0x0044CB77 BYTE_EXACT
// size      23 bytes
// prototype void (__cdecl ?help_topic@@YAXIH@Z)(DatalinkID id, int entryID)
// callers   9   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
// BODY IN datalink.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Combine two values as `a1 * 10000 + a2`.

             lea eax,[eax+eax*4]  x4   -> a1 * 625
             shl eax,4                 -> a1 * 10000
             add eax,ecx

         The four `lea` steps multiply by five each time and the shift by
         sixteen, which is 625 * 16 = 10000. Written as the constant, because
         the shift-and-add chain is the compiler's encoding of it and not a
         behaviour of its own - it wraps identically at 32 bits either way.

         Touches no field; both operands are arguments.
// ORIGINAL: 0x0042A020 ?UNK1@Datalink@@QAEHHH@Z 0x0042A020-0x0042A03E BYTE_EXACT
// size      30 bytes
// prototype int (__thiscall ?UNK1@Datalink@@QAEHHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: a1 * 10000 + a2, wrapping at 32 bits
Status: Complete
*/
int Datalink::UNK1(int a1, int a2) {
    return static_cast<int32_t>(static_cast<uint32_t>(a1) * 10000U
                                + static_cast<uint32_t>(a2));
}


/*
Purpose: Split a combined id into its two halves - `id / 10000` into the first
         output and `id % 10000` into the second.

             mov eax,0x68DB8BAD / imul ecx / sar edx,0xC
             mov eax,edx / shr eax,0x1F / add edx,eax   ; id / 10000
             mov [eax],edx
             lea .. x4 / shl edx,4 / sub ecx,edx        ; id - quotient*10000
             mov [eax],ecx

         The magic multiply is a signed divide by 10000, checked against C++
         truncating division over 200,000 random dividends and the boundaries,
         and the remainder is formed as `id - quotient * 10000` rather than by
         a second divide - which is what C++ `%` means for truncating division,
         verified the same way.

         This is the exact INVERSE of UNK1 above, which builds `a1 * 10000 + a2`
         from a shift-and-add chain. The two were decoded independently and
         agree on the constant.

         Neither output is null-checked, because the original checks neither.
// ORIGINAL: 0x0042A040 ?parse_id@Datalink@@QAEXHPAUDatalinkID@@PAH@Z 0x0042A040-0x0042A076 BYTE_EXACT
// size      54 bytes
// prototype int (__thiscall ?parse_id@Datalink@@QAEXHPAUDatalinkID@@PAH@Z)(Datalink* this, int, DatalinkID*, int*)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::parse_id(int id, DatalinkID *out_id, int *out_remainder) {
    const int32_t quotient = id / 10000;
    // Only the first four bytes of the DatalinkID are written; the type stays
    // incomplete, so the store is expressed against its address.
    *reinterpret_cast<int32_t *>(out_id) = quotient;
    *out_remainder = static_cast<int32_t>(
        static_cast<uint32_t>(id) - static_cast<uint32_t>(quotient) * 10000U);
}

/*
Purpose: Parse a decimal Datalink id into its category and entry components.
         The quotient/remainder are the same split used by parse_id; this
         overload receives the encoded id as text from a link target.
*/
// ORIGINAL: 0x0042A0B0 ?UNK2@Datalink@@QAEXPADPAHPAH@Z 0x0042A0B0-0x0042A0F1 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x0042A0B0-0x0042A0F1 calls atoi then stores quotient and remainder; git:08336bcc:src/recovered/units/0042a0b0.cpp contains the historical analysis and source:src/datalink.cpp:parse_id implements the matching numeric split"], "reason": "The product Datalink parser owns this text-to-category/remainder operation; its original span and name are preserved in Git provenance."}
// symbol    ?UNK2@Datalink@@QAEXPADPAH1@Z
void Datalink::UNK2(char *text, int *category, int *entry) {
    const int id = atoi(text);
    const int quotient = id / 10000;
    *category = quotient;
    *entry = id - quotient * 10000;
}

/*
Purpose: Format a category and entry suffix as the decimal Datalink id.
         This is the inverse of parse_id's quotient/remainder split.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0042A080 ?construct_id@Datalink@@QAEXPADIH@Z 0x0042A080-0x0042A0AD BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/0042a080.cpp preserves the 45-byte one-call seed and original span", "source:src/datalink.cpp parse_id divides the same combined id by 10000", "source:src/leaf_recoveries.cpp help builders call the declared Datalink member"], "reason": "The original Datalink member formats category*10000 plus entry into its caller-provided buffer; the former leaf-TU link-only body is replaced in the owning product TU."}
// symbol    ?construct_id@Datalink@@QAEXPADHH@Z
// size      45 bytes
Return Value: n/a
Status: Product body; measure before granting any new byte claim.
*/
void Datalink::construct_id(char *out, int category, int suffix) {
    _itoa(category * 10000 + suffix, out, 10);
}

/*
Purpose: Append one clickable cross-reference to a caller-owned help buffer.
         The original strings at 0x00683544/40/3C are "$LINK<", "=" and
         ">", respectively; the caller supplies visible text and its id.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0044CB80 ?help_create_link@@YAXPAD00@Z 0x0044CB80-0x0044CBC2 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x0068353C-0x0068354A contains the original markup strings >, = and $LINK<", "source:src/recovered/units/0044cb80.cpp preserves five strcat calls and the original 0x0044CB80 span", "source:src/leaf_recoveries.cpp help builders call this free helper"], "reason": "Home the Datalink-family link formatter as ordinary C++ markup construction in the product TU, replacing the empty leaf placeholder while retaining the original identity."}
// size      66 bytes
Return Value: n/a
Status: Product body; measure before granting a byte claim.
*/
void __cdecl help_create_link(char *out, char *label, char *id) {
    strcat(out, "$LINK<");
    strcat(out, label);
    strcat(out, "=");
    strcat(out, id);
    strcat(out, ">");
}


/*
// ORIGINAL: 0x00432040 ?on_selected@Datalink@@QAEXH@Z 0x00432040-0x00432052 BYTE_EXACT
// size      18 bytes
// prototype void (__thiscall ?on_selected@Datalink@@QAEXH@Z)(Datalink* this, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0042BF10
Status: Complete
*/
void Datalink::on_selected(int index) {
    int v = field_2A38_;
    if (v == 0) {
        draw_entry();
    }
}

/*
// ORIGINAL: 0x0042BD40 ?show@Datalink@@QAEXH@Z 0x0042BD40-0x0042BD5B BYTE_EXACT
// symbol    ?show@Datalink@@UAEXH@Z
// size      27 bytes
// prototype void (__thiscall ?show@Datalink@@QAEXH@Z)(Datalink* this, int)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045D310 0x005F7E90
Status: Complete
*/
void Datalink::show(int visible) {
    if (!Win::is_visible()) {
        SubInterface::set_iface_mode();
    }
}

/*
// ORIGINAL: 0x0042BD60 ?hide@Datalink@@QAEXXZ 0x0042BD60-0x0042BD79 BYTE_EXACT
// symbol    ?hide@Datalink@@UAEXXZ
// size      25 bytes
// prototype void (__thiscall ?hide@Datalink@@QAEXXZ)(Datalink* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045D380 0x005F7E90
Status: Complete
*/
void Datalink::hide() {
    if (Win::is_visible()) {
        SubInterface::release_iface_mode();
    }
}



/*
Purpose: Push a topic onto the back-history ring UNK5 pops, unless the panel
         is already showing it or the ring is switched off.

         The 10000-folded topic id is compared against the ring slot AFTER
         the last one pushed - `0x29e0 + count*4` - and stored one slot past
         that, `0x29e4 + count*4`. Slot 0 of the ring is the facility id, so
         the first store lands at ring[1] and never at ring[0]: that is the
         original's own arithmetic, not a slipped base.
// ORIGINAL: 0x0042BD80 ?UNK4@Datalink@@QAEXHH@Z 0x0042BD80-0x0042BDFA FILE
// MISMATCH 19/39, first divergence at 6. TRIED: the ring declared as `facilityID_` plus a tail array (two bases where the image's compare and store read `0x29e0+count*4` and `0x29e4+count*4` off one), and the eviction loop as `int *slot = &history_[1]` (which matches).
//            The image's catalogue name is ours to spell from behaviour:
//            the catalogue calls this ?UNK4@Datalink@@QAEXHH@Z.
// symbol    ?push_history@Datalink@@QAEXHH@Z
// size      122 bytes
// prototype void (__thiscall ?UNK4@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// TRIED: a single named `int *ring` base for the 20-entry buffer (the compiler keeps it live across the whole body and schedules the two prologue pushes differently); recomputing `self+0x29e0` and `self+0x29e4` separately (matching the two distinct bases the disasm itself uses) got the divergence from #4 to #29, where `--*(int*)(...); count = *(int*)(...);` CSEs into a register (`mov`) instead of the original's separate `dec [mem]` + reload.
Return Value: n/a
Status: Complete
*/
void Datalink::push_history(int category, int index) {
    if (field_2A38_ != 0) {
        return;
    }

    const int topic = category * 10000 + index;
    int count = field_2A34_;
    if (count != 0 && history_[count] == topic) {
        return;
    }

    int *slot = &history_[1];
    if (count == 0x14) {
        for (int i = 0; i < 0x13; ++i) {
            slot[i] = slot[i + 1];
        }
        --field_2A34_;
        count = field_2A34_;
    }

    slot[count] = topic;
    field_2A34_ = count + 1;
}

/*
Purpose: Pop the back-history ring and re-show the topic it held.

         Reads `0x29e0 + --count*4`, the slot UNK4 last wrote, splits it back
         into its category and facility halves the way parse_id does, then
         walks the chain set_cat / set_selected_id / slot 62 / draw_entry that
         UNK3 runs in the forward direction.
// ORIGINAL: 0x0042BE00 ?UNK5@Datalink@@QAEXXZ 0x0042BE00-0x0042BE9D FILE
// MISMATCH 8/49, first divergence at 4 - the receiver-spill order again: the image takes `this` into esi after saving ebx/edi, ours before. TRIED: `history_[--field_2A34_]` for the pop, `topic - category*10000` for the remainder instead of `%`, and a hoisted `&listBox_` base.
//            The image's catalogue name is ours to spell from behaviour:
//            the catalogue calls this ?UNK5@Datalink@@QAEXXZ.
// symbol    ?pop_history@Datalink@@QAEXXZ
// size      157 bytes
// prototype void (__thiscall ?UNK5@Datalink@@QAEXXZ)(Datalink* this)
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0042A2C0 0x0042BF10 0x00609FA0
// indirect  0x0042BE8C
Return Value: n/a
Status: Complete
*/
void Datalink::pop_history() {
    if (field_2A34_ <= 1) {
        return;
    }

    const int topic = history_[--field_2A34_];
    const int category = topic / 10000;
    const int facility = topic - category * 10000;

    set_cat(static_cast<unsigned int>(category), 0);
    history_[0] = facility;
    listBox_.set_selected_id(field_29DC_ * 10000 + facility);
    repaint_slot();
    draw_entry();
}

/*
Purpose: Show one topic outright - the forward direction of UNK5.

         Writes the facility id into ring slot 0, so the topic UNK4 pushes
         next lands after it, then runs the same chain set_cat /
         set_selected_id / slot 62 that every other entry point here runs.
// ORIGINAL: 0x0042A260 ?UNK3@Datalink@@QAEXIH@Z 0x0042A260-0x0042A2BF FILE
// TRIED: inlining repaint_slot in the header - basewin.cpp carries its own local `class VCall`, which then collides with vtable_shim.h's, so the definition lives in this TU and /Ob2 re-inlines it. MISMATCH 6/31 - the image is 31 instructions where ours is 25: its `push edi` sits before the `push 0` argument of set_cat, ours after, so the argument is evaluated before the callee-saved saves here and after them there.
//            The image's catalogue name is ours to spell from behaviour:
//            the catalogue calls this ?UNK3@Datalink@@QAEXIH@Z.
// symbol    ?show_topic@Datalink@@QAEXIH@Z
// size      95 bytes
// prototype void (__thiscall ?UNK3@Datalink@@QAEXIH@Z)(Datalink* this, DatalinkID, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0042A2C0 0x00609FA0
// indirect  0x0042A2B3
Return Value: n/a
Status: Complete
*/
void Datalink::show_topic(unsigned int category, int index) {
    set_cat(category, 0);
    history_[0] = index;
    listBox_.set_selected_id(field_29DC_ * 10000 + index);
    repaint_slot();
}

/*
Purpose: Follow a hyperlink in the topic text: "category*10000+facility".

         Facilities above 1000 are not real ones - they are the ids the
         prototype list invented - so they are ignored. The return is 0
         whatever happened; the image returns 0 on the ignored path and
         falls off the end (also 0) after the chain.
// ORIGINAL: 0x00431DC0 ?on_link_clicked@Datalink@@QAEHHPAD@Z 0x00431DC0-0x00431E64 FILE
// symbol    ?on_link_clicked@Datalink@@UAEHHPAD@Z
// MISMATCH 24/58, first divergence at 24 - the image reloads the facility slot after the set_cat call where ours folds it into the store. TRIED: the guard as `facility > 1000` in both polarities, and `atoi` against the CRT's own binding.
// size      161 bytes
// prototype int (__thiscall ?on_link_clicked@Datalink@@QAEHHPAD@Z)(Datalink* this, int, char *)
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0042A2C0 0x00609FA0
// indirect  0x00431E5B
Return Value: always 0
Status: Complete
*/
int Datalink::on_link_clicked(int, char *link) {
    const int topic = atoi(link);
    const int category = topic / 10000;
    const int facility = topic - category * 10000;

    if (facility > 1000) {
        return 0;
    }

    set_cat(static_cast<unsigned int>(category), 0);
    history_[0] = facility;
    listBox_.set_selected_id(field_29DC_ * 10000 + facility);
    repaint_slot();
    return 0;
}

/*
Purpose: Rebuild the topic list for one category and redraw it.

         The early-out asks three questions in the image's order: was this a
         plain re-select (index == 0), is that category already the one on
         show, and does the entry list still hold anything. Only when all
         three hold does it do nothing.

         The 0xE test and the unconditional second reset bracket the category
         store - the report window is told twice when the category was 0xE,
         once before and once after.

         Category 0xC skips the sort that every other category gets, and so
         falls straight into the update. The image's `cmp ebx, 0xc / je` after
         the jump table is that same skip reached from the switch's default
         arm, which no category above 0xF can satisfy.
// ORIGINAL: 0x0042A2C0 ?set_cat@Datalink@@QAEXIH@Z 0x0042A2C0-0x0042A4AC FILE
// MISMATCH 34/152, first divergence at 3. TRIED: transcribing StringStruct::remove_all's walk inline the way the image does - it is a real body here (stringstruct.cpp), so a call is what ours emits, 139 instructions against the image's 152.
// size      492 bytes
// prototype void (__thiscall ?set_cat@Datalink@@QAEXIH@Z)(Datalink* this, DatalinkID, int)
// callers   6   call targets   20
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0042A4F0 0x0042A630 0x0042A790 0x0042A900 0x0042AA60 0x0042ABC0 0x0042AD20 0x0042AE80 0x0042AFE0 0x0042B190 0x0042B300 0x0042B490 0x0042B630 0x0042B7E0 0x0042B940 0x0042BAB0 0x00496BB0 0x006087A0 0x0060C9B0 0x0062B870
// indirect  0x0042A34D 0x0042A35F 0x0042A37F
Return Value: n/a
Status: Complete
*/
void Datalink::set_cat(unsigned int category, int index) {
    if (index == 0 && field_29DC_ == category
        && listBox_.item_list_.entry_count_ != 0) {
        return;
    }

    // The topic panel's report window - the fixed 0x00876478 object
    // reportif.cpp names ReportIfModalWindow.
    if (field_29DC_ == 0xE) {
        ReportWinGlobal.reset();
    }

    field_29DC_ = category;
    ReportWinGlobal.reset();

    listBox_.item_list_.remove_all();
    listBox_.selected_position_ = 0;
    listBox_.field_18_ = 0;
    listBox_.field_20_ = 0;

    buttonGroup_.set(static_cast<int>(category), 0);

    switch (category) {
    case 2:  set_cat_all();         break;
    case 3:  set_cat_unit();        break;
    case 4:  set_cat_chassis();     break;
    case 5:  set_cat_reactor();     break;
    case 6:  set_cat_weapon();      break;
    case 7:  set_cat_defense();     break;
    case 8:  set_cat_ability();     break;
    case 9:  set_cat_terraform();   break;
    case 10: set_cat_facility();    break;
    case 11: set_cat_project();     break;
    case 12: set_cat_social();      break;
    case 13: set_cat_effect();      break;
    case 14: set_cat_tech();        break;
    case 15: set_cat_faction();     break;
    case 0:  set_cat_concept();     break;
    case 1:  set_cat_adv_concept(); break;
    }

    if (category != 0xC) {
        listBox_.item_list_.sort(0);
    }

    field_2A38_ = 1;
    listBox_.update_changes();
    field_2A38_ = 0;
}

// ---- the fourteen category fillers ----------------------------------------
//
// Every one walks its rules table, appends the matching names to StringTemp
// (with a "|" and the entry's own id when `GameState & 0x1000` says the ids
// should be visible), hands each line to the entry list, then frames the
// shared category rectangle, centres the category's label in it and repaints.
// The eight lines of the tail are the same eight lines in all of them, which
// is why they read as a family rather than as one function.

/*
Purpose: Refresh all fifteen categories at once - the "ALL" entry of the
         category strip.

         The image's call order is not sorted: terraform comes seventh and
         facility ninth, and that order is what this body keeps.
// ORIGINAL: 0x0042A4F0 ?set_cat_all@Datalink@@QAEXXZ 0x0042A4F0-0x0042A621 BYTE_EXACT
// size      305 bytes
// prototype void (__thiscall ?set_cat_all@Datalink@@QAEXXZ)(Datalink* this)
// callers   1   call targets   15
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0042A630 0x0042A790 0x0042A900 0x0042AA60 0x0042ABC0 0x0042AD20 0x0042BAB0 0x0042B300 0x0042AE80 0x0042AFE0 0x0042B630 0x0042B7E0 0x0042B190 0x0042B940
Return Value: n/a
Status: Complete
*/
void Datalink::set_cat_all() {
    set_cat_unit();
    set_cat_chassis();
    set_cat_reactor();
    set_cat_weapon();
    set_cat_defense();
    set_cat_ability();
    set_cat_terraform();
    set_cat_concept();
    set_cat_facility();
    set_cat_project();
    set_cat_social();
    set_cat_effect();
    set_cat_tech();
    set_cat_faction();

    RECT rect = category_rect_;
    box_sprite(&rect, &InterfaceBoxStyles[6]);

    rect.left += 3;
    rect.right -= 3;
    rect.top += 3;
    rect.bottom -= 3;

    set_text_color(0xE0, -1, 1, 1);
    set_font(&font3_, 0, 0, 0);

    const char *text = CATEGORY_LABEL(714);
    if (text != 0) {
        write_cent_l((char *)text, &rect, strlen(text));
    }

    soft_update(&category_rect_);
}

/*
Purpose: List the built unit designs, ids 30000 upward.

         Prototype 4 is skipped outright - it is the placeholder slot - and
         the walk stops after a hundred prototypes regardless of how many the
         rules file defines.
// ORIGINAL: 0x0042A630 ?set_cat_unit@Datalink@@QAEXXZ 0x0042A630-0x0042A78E BYTE_EXACT
// size      350 bytes
// prototype void (__thiscall ?set_cat_unit@Datalink@@QAEXXZ)(Datalink* this)
// callers   1   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0060C920 0x005E2DD3 0x005DACB0 0x005DD130
Return Value: n/a
Status: Complete
*/
void Datalink::set_cat_unit() {
    for (int index = 0; index < 100; ++index) {
        VehPrototype &proto = VehPrototypes[index];
        if ((proto.flags & 1) != 0 && index != 4) {
            StringTemp[0] = '\0';
            strcat(StringTemp, proto.veh_name);
            if ((GameDataStateStorage.fields_.game_state_ & 0x1000) != 0) {
                strcat(StringTemp, CategoryIdSeparator);
                char digits[80];
                _itoa(index + 30000, digits, 10);
                strcat(StringTemp, digits);
            }
            listBox_.item(StringTemp, index + 30000);
        }
    }

    RECT rect = category_rect_;
    box_sprite(&rect, &InterfaceBoxStyles[6]);

    rect.left += 3;
    rect.right -= 3;
    rect.top += 3;
    rect.bottom -= 3;

    set_text_color(0xE0, -1, 1, 1);
    set_font(&font3_, 0, 0, 0);

    const char *text = CATEGORY_LABEL(715);
    if (text != 0) {
        write_cent_l((char *)text, &rect, strlen(text));
    }

    soft_update(&category_rect_);
}

/*
Purpose: List the chassis, ids 40000 upward.

         The preq test reads the LAST word of the 0x90 record, which is where
         veh.h puts preq_tech.
// ORIGINAL: 0x0042A790 ?set_cat_chassis@Datalink@@QAEXXZ 0x0042A790-0x0042A8FA BYTE_EXACT
// size      356 bytes
// prototype void (__thiscall ?set_cat_chassis@Datalink@@QAEXXZ)(Datalink* this)
// callers   1   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0060C920 0x005E2DD3 0x005DACB0 0x005DD130
Return Value: n/a
Status: Complete
*/
void Datalink::set_cat_chassis() {
    for (int index = 0; index < MaxChassisNum; ++index) {
        if (Chassis[index].preq_tech >= -1) {
            StringTemp[0] = '\0';
            strcat(StringTemp, StringTable.get((int)Chassis[index].offsv_1_name));
            if ((GameDataStateStorage.fields_.game_state_ & 0x1000) != 0) {
                strcat(StringTemp, CategoryIdSeparator);
                char digits[80];
                _itoa(index + 40000, digits, 10);
                strcat(StringTemp, digits);
            }
            listBox_.item(StringTemp, index + 40000);
        }
    }

    RECT rect = category_rect_;
    box_sprite(&rect, &InterfaceBoxStyles[6]);

    rect.left += 3;
    rect.right -= 3;
    rect.top += 3;
    rect.bottom -= 3;

    set_text_color(0xE0, -1, 1, 1);
    set_font(&font3_, 0, 0, 0);

    const char *text = CATEGORY_LABEL(716);
    if (text != 0) {
        write_cent_l((char *)text, &rect, strlen(text));
    }

    soft_update(&category_rect_);
}

/*
Purpose: List the reactors, ids 50001 upward - the table starts at id 1, not
         0.

         The walk runs five records against a four-entry array: the fifth is
         the zero record past the end, whose name id 0 makes it an empty line.
         That is the original's own bound, reproduced here on purpose.
// ORIGINAL: 0x0042A900 ?set_cat_reactor@Datalink@@QAEXXZ 0x0042A900-0x0042AA5A BYTE_EXACT
// size      346 bytes
// prototype void (__thiscall ?set_cat_reactor@Datalink@@QAEXXZ)(Datalink* this)
// callers   1   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0060C920 0x005E2DD3 0x005DACB0 0x005DD130
Return Value: n/a
Status: Complete
*/
void Datalink::set_cat_reactor() {
    int id = 50001;
    const RulesReactor *record = Reactor;
    for (; id - 50000 < 5; ++record, ++id) {
        if (record->preq_tech >= -1) {
            StringTemp[0] = '\0';
            strcat(StringTemp, StringTable.get((int)record->name));
            if ((GameDataStateStorage.fields_.game_state_ & 0x1000) != 0) {
                strcat(StringTemp, CategoryIdSeparator);
                char digits[80];
                _itoa(id, digits, 10);
                strcat(StringTemp, digits);
            }
            listBox_.item(StringTemp, id);
        }
    }

    RECT rect = category_rect_;
    box_sprite(&rect, &InterfaceBoxStyles[6]);

    rect.left += 3;
    rect.right -= 3;
    rect.top += 3;
    rect.bottom -= 3;

    set_text_color(0xE0, -1, 1, 1);
    set_font(&font3_, 0, 0, 0);

    const char *text = CATEGORY_LABEL(717);
    if (text != 0) {
        write_cent_l((char *)text, &rect, strlen(text));
    }

    soft_update(&category_rect_);
}

/*
Purpose: List the weapons, ids 60000 upward.
// ORIGINAL: 0x0042AA60 ?set_cat_weapon@Datalink@@QAEXXZ 0x0042AA60-0x0042ABBC BYTE_EXACT
// size      346 bytes
// prototype void (__thiscall ?set_cat_weapon@Datalink@@QAEXXZ)(Datalink* this)
// callers   1   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0060C920 0x005E2DD3 0x005DACB0 0x005DD130
Return Value: n/a
Status: Complete
*/
void Datalink::set_cat_weapon() {
    for (int index = 0; index < MaxWeaponNum; ++index) {
        if (Weapon[index].preq_tech >= -1) {
            StringTemp[0] = '\0';
            strcat(StringTemp, StringTable.get((int)Weapon[index].name));
            if ((GameDataStateStorage.fields_.game_state_ & 0x1000) != 0) {
                strcat(StringTemp, CategoryIdSeparator);
                char digits[80];
                _itoa(index + 60000, digits, 10);
                strcat(StringTemp, digits);
            }
            listBox_.item(StringTemp, index + 60000);
        }
    }

    RECT rect = category_rect_;
    box_sprite(&rect, &InterfaceBoxStyles[6]);

    rect.left += 3;
    rect.right -= 3;
    rect.top += 3;
    rect.bottom -= 3;

    set_text_color(0xE0, -1, 1, 1);
    set_font(&font3_, 0, 0, 0);

    const char *text = CATEGORY_LABEL(718);
    if (text != 0) {
        write_cent_l((char *)text, &rect, strlen(text));
    }

    soft_update(&category_rect_);
}

/*
Purpose: List the armour, ids 70000 upward.
// ORIGINAL: 0x0042ABC0 ?set_cat_defense@Datalink@@QAEXXZ 0x0042ABC0-0x0042AD1C BYTE_EXACT
// size      346 bytes
// prototype void (__thiscall ?set_cat_defense@Datalink@@QAEXXZ)(Datalink* this)
// callers   1   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0060C920 0x005E2DD3 0x005DACB0 0x005DD130
Return Value: n/a
Status: Complete
*/
void Datalink::set_cat_defense() {
    for (int index = 0; index < MaxArmorNum; ++index) {
        if (Armor[index].preq_tech >= -1) {
            StringTemp[0] = '\0';
            strcat(StringTemp, StringTable.get((int)Armor[index].name));
            if ((GameDataStateStorage.fields_.game_state_ & 0x1000) != 0) {
                strcat(StringTemp, CategoryIdSeparator);
                char digits[80];
                _itoa(index + 70000, digits, 10);
                strcat(StringTemp, digits);
            }
            listBox_.item(StringTemp, index + 70000);
        }
    }

    RECT rect = category_rect_;
    box_sprite(&rect, &InterfaceBoxStyles[6]);

    rect.left += 3;
    rect.right -= 3;
    rect.top += 3;
    rect.bottom -= 3;

    set_text_color(0xE0, -1, 1, 1);
    set_font(&font3_, 0, 0, 0);

    const char *text = CATEGORY_LABEL(719);
    if (text != 0) {
        write_cent_l((char *)text, &rect, strlen(text));
    }

    soft_update(&category_rect_);
}

/*
Purpose: List the abilities, ids 80000 upward.
// ORIGINAL: 0x0042AD20 ?set_cat_ability@Datalink@@QAEXXZ 0x0042AD20-0x0042AE7C BYTE_EXACT
// size      346 bytes
// prototype void (__thiscall ?set_cat_ability@Datalink@@QAEXXZ)(Datalink* this)
// callers   1   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0060C920 0x005E2DD3 0x005DACB0 0x005DD130
Return Value: n/a
Status: Complete
*/
void Datalink::set_cat_ability() {
    for (int index = 0; index < MaxAbilityNum; ++index) {
        if (Ability[index].preq_tech >= -1) {
            StringTemp[0] = '\0';
            strcat(StringTemp, StringTable.get((int)Ability[index].name));
            if ((GameDataStateStorage.fields_.game_state_ & 0x1000) != 0) {
                strcat(StringTemp, CategoryIdSeparator);
                char digits[80];
                _itoa(index + 80000, digits, 10);
                strcat(StringTemp, digits);
            }
            listBox_.item(StringTemp, index + 80000);
        }
    }

    RECT rect = category_rect_;
    box_sprite(&rect, &InterfaceBoxStyles[6]);

    rect.left += 3;
    rect.right -= 3;
    rect.top += 3;
    rect.bottom -= 3;

    set_text_color(0xE0, -1, 1, 1);
    set_font(&font3_, 0, 0, 0);

    const char *text = CATEGORY_LABEL(720);
    if (text != 0) {
        write_cent_l((char *)text, &rect, strlen(text));
    }

    soft_update(&category_rect_);
}


/*
Purpose: List the facilities, ids 100001 upward - the table's slot 0 is the
         unused one, so the walk starts one record in.

         The preq word is at +0x14, the fifth dword, which is where base.h
         puts preq_tech.
// ORIGINAL: 0x0042AE80 ?set_cat_facility@Datalink@@QAEXXZ 0x0042AE80-0x0042AFDC BYTE_EXACT
// size      345 bytes
// prototype void (__thiscall ?set_cat_facility@Datalink@@QAEXXZ)(Datalink* this)
// callers   1   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0060C920 0x005E2DD3 0x005DACB0 0x005DD130
Return Value: n/a
Status: Complete
*/
void Datalink::set_cat_facility() {
    int id = 100001;
    const RulesFacility *record = Facility + 1;
    for (; id - 100000 < 0x46; ++record, ++id) {
        if (record->preq_tech >= -1) {
            StringTemp[0] = '\0';
            strcat(StringTemp, StringTable.get((int)record->name));
            if ((GameDataStateStorage.fields_.game_state_ & 0x1000) != 0) {
                strcat(StringTemp, CategoryIdSeparator);
                char digits[80];
                _itoa(id, digits, 10);
                strcat(StringTemp, digits);
            }
            listBox_.item(StringTemp, id);
        }
    }

    RECT rect = category_rect_;
    box_sprite(&rect, &InterfaceBoxStyles[6]);

    rect.left += 3;
    rect.right -= 3;
    rect.top += 3;
    rect.bottom -= 3;

    set_text_color(0xE0, -1, 1, 1);
    set_font(&font3_, 0, 0, 0);

    const char *text = CATEGORY_LABEL(722);
    if (text != 0) {
        write_cent_l((char *)text, &rect, strlen(text));
    }

    soft_update(&category_rect_);
}

/*
Purpose: List the secret projects, ids 68534 upward.

         When `ProjectPrefixHidden` is clear the "The " prefix the rules file
         carries is stripped off the front of the name first, so the list
         reads "Ascension" rather than "The Ascension".
// ORIGINAL: 0x0042AFE0 ?set_cat_project@Datalink@@QAEXXZ 0x0042AFE0-0x0042B18E FILE
// MISMATCH 10/133 against our 113, first divergence at 2 - the image keeps a pointer to the current record; ours indexes SecretProjectRules every iteration. TRIED: the record hoisted out of the loop as `const RulesFacility *`.
// size      426 bytes
// prototype void (__thiscall ?set_cat_project@Datalink@@QAEXXZ)(Datalink* this)
// callers   1   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E2DD3 0x005DACB0 0x005DD130 0x0060C920 0x005DA330
Return Value: n/a
Status: Complete
*/
void Datalink::set_cat_project() {
    for (int index = 0; index < 0x40; ++index) {
        if (SecretProjectRules[index].preq_tech >= -1) {
            StringTemp[0] = '\0';
            strcat(StringTemp,
                   StringTable.get((int)SecretProjectRules[index].name));
            if (Language == 0 && strncmp(StringTemp, ConceptArticle, 4) == 0) {
                char full[256];
                strcpy(full, StringTemp);
                strcpy(StringTemp, full + 4);
            }
            if ((GameDataStateStorage.fields_.game_state_ & 0x1000) != 0) {
                strcat(StringTemp, CategoryIdSeparator);
                char digits[80];
                _itoa(index + 68534, digits, 10);
                strcat(StringTemp, digits);
            }
            listBox_.item(StringTemp, index + 68534);
        }
    }

    RECT rect = category_rect_;
    box_sprite(&rect, &InterfaceBoxStyles[6]);

    rect.left += 3;
    rect.right -= 3;
    rect.top += 3;
    rect.bottom -= 3;

    set_text_color(0xE0, -1, 1, 1);
    set_font(&font3_, 0, 0, 0);

    const char *text = CATEGORY_LABEL(723);
    if (text != 0) {
        write_cent_l((char *)text, &rect, strlen(text));
    }

    soft_update(&category_rect_);
}

/*
Purpose: List the technologies, ids 140000 upward.

         The technology record the tree materialises is eleven dwords wide and
         the walk reads its two preq words at -1 and 0 from the pointer it
         holds, which is why both are tested before the name at -9 is fetched.
// ORIGINAL: 0x0042B190 ?set_cat_tech@Datalink@@QAEXXZ 0x0042B190-0x0042B2FA FILE
// MISMATCH 31/116, first divergence at 6. LEVER: `char digits[0x54]` not [80] - the image's frame is 0x64 and the 80-byte buffer gives 0x60. TRIED: the record walk as `record[-1] >= -1 && record[0] >= -1` against a hoisted `TechRowBase`, which is the image's own shape (base in esi, stepped 11 dwords); the residual divergence is the image taking `this` into edi and ours into ebx.
// size      355 bytes
// prototype void (__thiscall ?set_cat_tech@Datalink@@QAEXXZ)(Datalink* this)
// callers   1   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E2DD3 0x005DACB0 0x005DD130 0x0060C920 0x005DA330
Return Value: n/a
Status: Complete
*/
void Datalink::set_cat_tech() {
    const int *record = TechRowBase;
    for (int index = 0; index < 0x59; ++index) {
        if (record[-1] >= -1 && record[0] >= -1) {
            StringTemp[0] = '\0';
            strcat(StringTemp, StringTable.get(record[-9]));
            if ((GameDataStateStorage.fields_.game_state_ & 0x1000) != 0) {
                strcat(StringTemp, CategoryIdSeparator);
                char digits[0x54];
                _itoa(index + 140000, digits, 10);
                strcat(StringTemp, digits);
            }
            listBox_.item(StringTemp, index + 140000);
        }
        record += 11;
    }

    RECT rect = category_rect_;
    box_sprite(&rect, &InterfaceBoxStyles[6]);

    rect.left += 3;
    rect.right -= 3;
    rect.top += 3;
    rect.bottom -= 3;

    set_text_color(0xE0, -1, 1, 1);
    set_font(&font3_, 0, 0, 0);

    const char *text = CATEGORY_LABEL(726);
    if (text != 0) {
        write_cent_l((char *)text, &rect, strlen(text));
    }

    soft_update(&category_rect_);
}

/*
Purpose: List the concept encyclopedia entries straight out of CONCEPTS.TXT.

         The walk is the text reader's own: get a line, file it, get the next,
         until the reader hands back an empty one. ids run 0 upward, which is
         why this is the one category whose ids are not five-digit.
// ORIGINAL: 0x0042B300 ?set_cat_concept@Datalink@@QAEXXZ 0x0042B300-0x0042B48C FILE
// MISMATCH 36/125, first divergence at 3 - register pressure, not frame size: the frame matches at 0x60 but the image saves only edi where ours saves ebx/esi/edi, because the reader loop keeps the line pointer live across the text_get call here.
// size      391 bytes
// prototype void (__thiscall ?set_cat_concept@Datalink@@QAEXXZ)(Datalink* this)
// callers   1   call targets   8
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E2DD3 0x005DACB0 0x005DD130 0x0060C920 0x005DA330 0x005FD570 0x005FD820 0x005FD910
Return Value: n/a
Status: Complete
*/
void Datalink::set_cat_concept() {
    if (text_open(ConceptSection, ConceptFile) == 0) {
        text_get();
        for (int index = 0; TextBufferGetPtr != 0
                            && strlen(TextBufferGetPtr) != 0;
             ++index) {
            StringTemp[0] = '\0';
            strcat(StringTemp, TextBufferGetPtr);
            if ((GameDataStateStorage.fields_.game_state_ & 0x1000) != 0) {
                strcat(StringTemp, CategoryIdSeparator);
                char digits[80];
                _itoa(index, digits, 10);
                strcat(StringTemp, digits);
            }
            listBox_.item(StringTemp, index);
            text_get();
        }
        text_close();
    }

    RECT rect = category_rect_;
    box_sprite(&rect, &InterfaceBoxStyles[6]);

    rect.left += 3;
    rect.right -= 3;
    rect.top += 3;
    rect.bottom -= 3;

    set_text_color(0xE0, -1, 1, 1);
    set_font(&font3_, 0, 0, 0);

    const char *text = CATEGORY_LABEL(712);
    if (text != 0) {
        write_cent_l((char *)text, &rect, strlen(text));
    }

    soft_update(&category_rect_);
}

/*
Purpose: List the advanced concepts, the second CONCEPTS.TXT section.

         Same reader walk as set_cat_concept, with the ids starting at 10000
         so the two lists cannot collide.
// ORIGINAL: 0x0042B490 ?set_cat_adv_concept@Datalink@@QAEXXZ 0x0042B490-0x0042B62A FILE
// MISMATCH 32/127, first divergence at 10. TRIED: the reader walk spelled exactly as set_cat_concept's, with the early return as the image's inverted `!= 0`.
// size      410 bytes
// prototype void (__thiscall ?set_cat_adv_concept@Datalink@@QAEXXZ)(Datalink* this)
// callers   1   call targets   8
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E2DD3 0x005DACB0 0x005DD130 0x0060C920 0x005DA330 0x005FD570 0x005FD820 0x005FD910
Return Value: n/a
Status: Complete
*/
void Datalink::set_cat_adv_concept() {
    if (text_open(ConceptSection, AdvConceptFile) != 0) {
        return;
    }

    text_get();
    for (int index = 0;
         TextBufferGetPtr != 0 && strlen(TextBufferGetPtr) != 0; ++index) {
        StringTemp[0] = '\0';
        strcat(StringTemp, TextBufferGetPtr);
        if ((GameDataStateStorage.fields_.game_state_ & 0x1000) != 0) {
            strcat(StringTemp, CategoryIdSeparator);
            char digits[80];
            _itoa(index + 10000, digits, 10);
            strcat(StringTemp, digits);
        }
        listBox_.item(StringTemp, index + 10000);
        text_get();
    }
    text_close();

    RECT rect = category_rect_;
    box_sprite(&rect, &InterfaceBoxStyles[6]);

    rect.left += 3;
    rect.right -= 3;
    rect.top += 3;
    rect.bottom -= 3;

    set_text_color(0xE0, -1, 1, 1);
    set_font(&font3_, 0, 0, 0);

    const char *text = CATEGORY_LABEL(713);
    if (text != 0) {
        write_cent_l((char *)text, &rect, strlen(text));
    }

    soft_update(&category_rect_);
}

/*
Purpose: List the social engineering categories and, under each, the four
         choices it offers in that column.

         The header line carries the category's own id - 120000 upward - and
         the four lines under it carry the choice ids. Rows are 0xD4 apart and
         the four choices are the first four dwords of the row.
// ORIGINAL: 0x0042B630 ?set_cat_social@Datalink@@QAEXXZ 0x0042B630-0x0042B7DC FILE
// MISMATCH 130/137, first divergence at 12 - the loop tail reloads `category` before `row` here and the image the other way round. TRIED: the increment reordered `row += 53, category += 5` to swap them - 123/137 against 130/137, so the source order is already the image's.
// size      421 bytes
// prototype void (__thiscall ?set_cat_social@Datalink@@QAEXXZ)(Datalink* this)
// callers   1   call targets   6
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E2DD3 0x005DACB0 0x005DD130 0x0060C920 0x005DA330
Return Value: n/a
Status: Complete
*/
void Datalink::set_cat_social() {
    const int *row = SocialCategoryRow;
    for (int category = 120001; category < 120021; category += 5, row += 53) {
        listBox_.item(StringTable.get(row[-5]), category - 1);

        for (int choice = 0; choice < 4; ++choice) {
            int id = category + choice;
            StringTemp[0] = '\0';
            strcat(StringTemp, StringTable.get(row[choice]));
            if ((GameDataStateStorage.fields_.game_state_ & 0x1000) != 0) {
                strcat(StringTemp, CategoryIdSeparator);
                char digits[80];
                _itoa(id, digits, 10);
                strcat(StringTemp, digits);
            }
            listBox_.item(StringTemp, id);
        }
    }

    RECT rect = category_rect_;
    box_sprite(&rect, &InterfaceBoxStyles[6]);

    rect.left += 3;
    rect.right -= 3;
    rect.top += 3;
    rect.bottom -= 3;

    set_text_color(0xE0, -1, 1, 1);
    set_font(&font3_, 0, 0, 0);

    const char *text = CATEGORY_LABEL(724);
    if (text != 0) {
        write_cent_l((char *)text, &rect, strlen(text));
    }

    soft_update(&category_rect_);
}

/*
Purpose: List the social effects, ids 130000 upward.

         Row 3 - the NullEffect slot - is skipped outright, which is why the
         list is one shorter than the table.
// ORIGINAL: 0x0042B7E0 ?set_cat_effect@Datalink@@QAEXXZ 0x0042B7E0-0x0042B938 FILE
// MISMATCH 21/110, first divergence at 6 - register allocation around the loop counter. TRIED: the walk as `row < SocialEffectRowEnd` with the Null row excluded by address, and the name read as `row` itself (the effects table holds its LPSTRs inline).
// size      340 bytes
// prototype void (__thiscall ?set_cat_effect@Datalink@@QAEXXZ)(Datalink* this)
// callers   1   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E2DD3 0x005DACB0 0x005DD130 0x0060C920 0x005DA330
Return Value: n/a
Status: Complete
*/
void Datalink::set_cat_effect() {
    const char *row = SocialEffectRowBase;
    for (int index = 0; row < SocialEffectRowEnd; row += 0x68, ++index) {
        if (row != SocialEffectNullRow) {
            StringTemp[0] = '\0';
            strcat(StringTemp, row);
            if ((GameDataStateStorage.fields_.game_state_ & 0x1000) != 0) {
                strcat(StringTemp, CategoryIdSeparator);
                char digits[80];
                _itoa(index + 130000, digits, 10);
                strcat(StringTemp, digits);
            }
            listBox_.item(StringTemp, index + 130000);
        }
    }

    RECT rect = category_rect_;
    box_sprite(&rect, &InterfaceBoxStyles[6]);

    rect.left += 3;
    rect.right -= 3;
    rect.top += 3;
    rect.bottom -= 3;

    set_text_color(0xE0, -1, 1, 1);
    set_font(&font3_, 0, 0, 0);

    const char *text = CATEGORY_LABEL(725);
    if (text != 0) {
        write_cent_l((char *)text, &rect, strlen(text));
    }

    soft_update(&category_rect_);
}

/*
Purpose: List the factions, ids 150000 upward.

         Before each name is filed the two dwords at the head of the record
         are published to the faction-id pair general.h owns, so the list
         builder knows which faction the line it is adding belongs to.
// ORIGINAL: 0x0042B940 ?set_cat_faction@Datalink@@QAEXXZ 0x0042B940-0x0042BACC FILE
// MISMATCH 16/135 against our 111 - the image's frame is 24 bytes larger, so its digits buffer is not the 80 ours is. TRIED: `record += 0x167` on a `const char *` with the head dwords published through general.h's GenderDefault / PluralityDefault, and the name read at +0x84.
// size      355 bytes
// prototype void (__thiscall ?set_cat_faction@Datalink@@QAEXXZ)(Datalink* this)
// callers   1   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E2DD3 0x005DACB0 0x005DD130 0x0060C920 0x005DA330
Return Value: n/a
Status: Complete
*/
void Datalink::set_cat_faction() {
    const char *record = FactionRowBase;
    for (int index = 0; index < 8; record += 0x167, ++index) {
        GenderDefault = reinterpret_cast<const int *>(record)[-1];
        PluralityDefault = reinterpret_cast<const int *>(record)[0];
        StringTemp[0] = '\0';
        strcat(StringTemp, record + 0x84);
        if ((GameDataStateStorage.fields_.game_state_ & 0x1000) != 0) {
            strcat(StringTemp, CategoryIdSeparator);
            char digits[80];
            _itoa(index + 150001, digits, 10);
            strcat(StringTemp, digits);
        }
        listBox_.item(StringTemp, index + 150001);
    }

    RECT rect = category_rect_;
    box_sprite(&rect, &InterfaceBoxStyles[6]);

    rect.left += 3;
    rect.right -= 3;
    rect.top += 3;
    rect.bottom -= 3;

    set_text_color(0xE0, -1, 1, 1);
    set_font(&font3_, 0, 0, 0);

    const char *text = CATEGORY_LABEL(727);
    if (text != 0) {
        write_cent_l((char *)text, &rect, strlen(text));
    }

    soft_update(&category_rect_);
}


/*
Purpose: List the terraforming options, ids 90000 upward.

         Two tables drive this one - the name ids from one, the preq/flag
         words from another - and every pass handles TWO entries, the second
         carrying its own preq test and its own id-suffix separator. The two
         ids 0x15FB4 and 0x15FB5 (raise/lower terrain) never reach the list:
         the second half of their pass is skipped, which is the image's own
         pair of compares and not a slipped loop bound.

         The goto shape is the image's: the suffix append and the `item` call
         share one exit, so the "no ids" case still files the name.
// ORIGINAL: 0x0042BAB0 ?set_cat_terraform@Datalink@@QAEXXZ 0x0042BAB0-0x0042BCA4 FILE
// MISMATCH 14/150 against our 159, first divergence at 8. TRIED: the second entry's skip spelled as the image's own pair of compares on ids 90036/90037 and as a `continue` - the two produce the same 159.
// size      513 bytes
// prototype void (__thiscall ?set_cat_terraform@Datalink@@QAEXXZ)(Datalink* this)
// callers   1   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E2DD3 0x005DACB0 0x005DD130 0x0060C920 0x005DA330
Return Value: n/a
Status: Complete
*/
void Datalink::set_cat_terraform() {
    const int *names = TerraformNameBase;
    const int *flags = TerraformFlagBase;
    for (int category = 90020; category - 90020 < 0x14;
         flags += 8, names += 3, ++category) {
        if (flags[-1] >= -1 && names[-1] != 0) {
            StringTemp[0] = '\0';
            strcat(StringTemp, StringTable.get(names[-1]));
            if ((GameDataStateStorage.fields_.game_state_ & 0x1000) != 0) {
                strcat(StringTemp, CategoryIdSeparator);
                char digits[80];
                _itoa(category - 20, digits, 10);
                strcat(StringTemp, digits);
            }
            listBox_.item(StringTemp, category - 20);
        }

        if (category == 90036 || category == 90037) {
            continue;
        }
        if (flags[0] < -1) {
            continue;
        }

        StringTemp[0] = '\0';
        strcat(StringTemp, StringTable.get(names[0]));
        if ((GameDataStateStorage.fields_.game_state_ & 0x1000) != 0) {
            strcat(StringTemp, TerraformSecondSeparator);
            char digits[80];
            _itoa(category, digits, 10);
            strcat(StringTemp, digits);
        }
        listBox_.item(StringTemp, category);
    }

    RECT rect = category_rect_;
    box_sprite(&rect, &InterfaceBoxStyles[6]);

    rect.left += 3;
    rect.right -= 3;
    rect.top += 3;
    rect.bottom -= 3;

    set_text_color(0xE0, -1, 1, 1);
    set_font(&font3_, 0, 0, 0);

    const char *text = CATEGORY_LABEL(721);
    if (text != 0) {
        write_cent_l((char *)text, &rect, strlen(text));
    }

    soft_update(&category_rect_);
}

// ---- seams the category fillers call out to -------------------------------
//
// ReportWin::reset now lives in reportwin.cpp; StringList::sort lives with its
// real list nodes in stringstruct.cpp, and ListBox::update_changes lives in
// listbox.cpp. The old fixed-image reset forwarder is retired.

// ORIGINAL: 0x00431EC0 ?on_button_clicked@Datalink@@QAEXH@Z 0x00431EC0-0x00431FA9 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Recover Datalink's history and report-button dispatch in its owning class rather than forwarding into the original image.","evidence":["image:0x00431EC0-0x00431FA9 dispatches button ids 0x10/0x11 and calls Datalink::set_cat, ListBox::set_selected_id, Datalink::draw_entry and ReportWin::on_button_clicked","source:src/win.h identifies virtual slot 58 at +0xE8 as release_modal; source:src/reportwin.h binds ReportWinGlobal to 0x00876478","source:src/datalink.h places history_, field_2A34_, field_29DC_ and listBox_ at the original offsets"]}
// symbol    ?on_button_clicked@Datalink@@UAEXH@Z
// Purpose: Route history controls and report-window buttons from the Datalink panel.
void Datalink::on_button_clicked(int button_id) {
    switch (button_id) {
    default:
        if (field_29DC_ == 0xe) {
            ReportWinGlobal.on_button_clicked(button_id);
        }
        break;
    case 0x10: {
        if (field_2A34_ <= 1) {
            release_modal();
            return;
        }

        --field_2A34_;
        const int entry = history_[field_2A34_];
        const int category = entry / 10000;
        const int item = entry - category * 10000;
        set_cat(static_cast<unsigned int>(category), 0);
        history_[0] = item;

        const int selected = field_29DC_ * 10000 + item;
        listBox_.set_selected_id(selected);
        repaint_slot();
        draw_entry();
        break;
    }
    case 0x11:
        release_modal();
        break;
    }
}

// ORIGINAL: 0x00432010 ?on_button_passover@Datalink@@QAEXHH@Z 0x00432010-0x00432032 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00432010-0x00432032 tests Datalink category at +0x29DC against 0xE before forwarding both arguments", "source:src/datalink.h field_29DC_ is the category at 0x29DC", "source:src/reportwin.h declares ReportWin::on_button_passover; its authorized link-only body remains explicitly unfinished in reportwin.cpp"], "reason": "Forward button passover only while the Datalink is displaying report category 0xE, using the established category member and typed ReportWin callback."}
void Datalink::on_button_passover(int button_id, int state) {
    if (field_29DC_ == 0xe) {
        ReportWinGlobal.on_button_passover(button_id, state);
    }
}

// ORIGINAL: 0x0042BCC0 ?set_index@Datalink@@QAEXHH@Z 0x0042BCC0-0x0042BD14 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x0042BCC0-0x0042BD14 stores the first argument at Datalink+0x29E0, combines it with category at +0x29DC for ListBox::set_selected_id, then dispatches GraphicWin redraw through the embedded list", "source:src/datalink.h maps history_[0], field_29DC_ and listBox_ at those offsets", "source:src/datalink.cpp repaint_slot names the embedded ListBox redraw operation"], "reason": "Select a Datalink entry through its typed history/category/ListBox state and the already named repaint operation, replacing seed receiver arithmetic and an opaque slot shim."}
void Datalink::set_index(int index, int) {
    const int category = field_29DC_;
    history_[0] = index;
    listBox_.set_selected_id(category * 10000 + index);
    repaint_slot();
}

/*
Purpose: Restore the interface and Datalink surface clips, repaint the
         interface background, then draw the selected Datalink entry.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00431E70 ?on_redraw@Datalink@@QAEXXZ 0x00431E70-0x00431EA6 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00431E70-0x00431EA6 calls Buffer::set_clip twice, MainInterface::restore_back and Datalink::draw_entry", "image:0x007AEC64 is the Buffer base of MainInterfaceGlobal at 0x007AE820+0x444; 0x007AEC94 is its rect2_ at +0x474", "source:src/recovered/units/00431e70.cpp preserves the byte-exact historical unit"], "reason": "Use the existing MainInterface object and real Buffer bases instead of fixed executable addresses and a raw Datalink offset."}
// symbol    ?on_redraw@Datalink@@UAEXXZ
// size      54 bytes
// callers   0   call targets   3
// calls     0x0042BF10 0x0045C290 0x005D8000
Return Value: n/a
Status: Product body; historical seed claim remains a restoration obligation.
*/
void Datalink::on_redraw() {
    MainInterfaceGlobal.restore_clip();
    restore_clip();
    MainInterfaceGlobal.restore_back(1);
    draw_entry();
}

// Escape and Enter close the Datalink; other keys are delegated to its entry
// list's Win key handler.
// ORIGINAL: 0x00431FC0 ?on_key_click@Datalink@@QAEHHH@Z 0x00431FC0-0x00431FFB BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00431FC0-0x00431FFB branches on key codes 0x0D and 0x1B, dynamically calls ListBox slot 26 otherwise, then Win slot 58; git:08336bcc:src/recovered/units/00431fc0.cpp; source:src/datalink.h embeds ListBox listBox_; source:src/win.h names slot 58 release_modal and slot 26 on_key_down"], "reason": "The Datalink key callback closes the modal window for Enter/Escape and delegates other keys through its embedded ListBox Win handler."}
// symbol    ?on_key_click@Datalink@@UAEHHH@Z
int Datalink::on_key_click(int, int key) {
    if (key != 0x0D) {
        if (key != 0x1B) {
            return listBox_.on_key_down(key);
        }
    }
    release_modal();
    return 1;
}

// The original status receiver is the SubInterface at Datalink+0xA14.
// The genuine override and named members express that adjustment; the two
// string panels are real owned StringBoxes, not independently addressed Win
// stand-ins. Original4320DB..4320FC hides blurb then concept after clearing
// the history count, before sliding/hiding the main window and entry list.
void __cdecl slide_show(GraphicWin *, int);
void __cdecl slide_hide(GraphicWin *, int);

/*
Purpose: Show or hide the datalink status layer: the topic drawer, the
         eighteen per-category flat buttons and the entry list all follow
         the mode, and hiding it also clears the history-slot count.
// ORIGINAL: 0x00432060 ?on_status@Datalink@@QAEXH@Z 0x00432060-0x00432152 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/unrecovered/00432060.cpp"], "reason": "Retain the existing source implementation and complete original mapping. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
// LEVER: statement-order not declaration order - `this - 0xA14` (the GraphicWin-adjusted receiver used for slide_show/slide_hide, draw_entry, show, hide) is recomputed fresh inside each branch rather than hoisted once; and in the a1==0 branch the `*(self+0x2020)=0` store sits between computing the 0x126A0 vtable-slot address and dereferencing it, matching the lea/mov/mov/call order byte for byte. Vtable dispatch and the recursive `esi-0xa14` receiver both use the OriginalObject/ original_method/original_slot pointer-to-member pattern from src/original_seam.h, inlined here since the scaffold TU cannot #include project headers and `__thiscall` on a function-pointer typedef is C4234 in this cl.
// symbol    ?on_status@Datalink@@UAEXH@Z
// size      242 bytes
Status: Coverage transcription of the retained seed
        (git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/unrecovered/00432060.cpp), which is BYTE_EXACT in scaffold
        form and keeps the banked claim until this body measures fresh.
        Model note, from the seed's own arithmetic: the image's receiver
        is the window at Datalink+0xA14 - its 0x2020 is field_2A34_,
        its 0x2028 the topic_buttons_ array (eighteen buttons, stride
        == sizeof(FlatButton)), and its 0xEC14 is listBox_. The current
        owner body names these mapped members and the typed blurb_box_ and
        concept_box_ children; the seed's raw receiver arithmetic is evidence
        for the member map, not source syntax to reproduce here.
*/
void Datalink::on_status(int visible) {
    if (visible != 0) {
        on_redraw();
        slide_show(this, 2);
        draw_entry();
        this->Win::show(3);

        FlatButton *entry = topic_buttons_;
        for (int i = 0; i < 0x12; ++i) {
            entry->show(0);
            ++entry;
        }

        listBox_.show(0);
        return;
    }

    field_2A34_ = 0;
    blurb_box_.hide();
    concept_box_.hide();

    slide_hide(this, 2);
    this->Win::hide();

    listBox_.hide();

    FlatButton *entry = topic_buttons_;
    for (int i = 0; i < 0x12; ++i) {
        entry->hide();
        ++entry;
    }
}

// Original interface table 0x0066AF30 slot 1 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_iface_scrolled@Datalink@@UAEXHH@Z 0x00432060
void Datalink::on_iface_scrolled(int, int) {}
// Original interface table 0x0066AF30 slot 2 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_iface_scrolling@Datalink@@UAEXHH@Z 0x00432060
void Datalink::on_iface_scrolling(int, int) {}
// Original interface table 0x0066AF30 slot 6 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_iface_left_up@Datalink@@UAEXHH@Z 0x00432060
void Datalink::on_iface_left_up(int, int) {}

// Original interface table 0x0066AF30 slot 14 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404250 0x00404255 ?on_iface_button_pressed@Datalink@@UAEHH@Z 0x00432060
int Datalink::on_iface_button_pressed(int) {
    return 0;
}

// Original interface table 0x0066AF30 slot 17 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404250 0x00404255 ?on_iface_dialog_back_draw@Datalink@@UAEHPAVGraphicWin@@@Z 0x00432060
int Datalink::on_iface_dialog_back_draw(GraphicWin *) {
    return 0;
}

// Original interface table 0x0066AF30 slot 18 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404240 0x00404245 ?on_iface_dialog_item_back_draw@Datalink@@UAEHPAVGraphicWin@@HHPAUtagRECT@@@Z 0x00432060
int Datalink::on_iface_dialog_item_back_draw(GraphicWin *, int, int, RECT *) {
    return 0;
}

// Original wrapper 0x44C805 publishes its Popup receiver before invoking
// the synchronous help-text builder; callback 0x44C7D6 reads that same slot.
// Keep the original single shared receiver, including its reentrant overwrite
// behavior, rather than introducing a stack or an executable-address alias.
// RECOVERY_OBJECT: v2 0x007591B0 0x007591B4 ?TechnologyHelpPopup@@3PAVPopup@@A 0x0044C7F0
Popup *TechnologyHelpPopup;

void __cdecl help_tech_info(void (__cdecl *emit)(char *), int technology,
                            int faction, int link_mode);

// These are independent source-owned data objects. The two caret strings
// are separate literals; neither denotes a writable shared buffer.
// RECOVERY_OBJECT: v1 0x007591BC ?TechnologyHelpPrefixEmitted@@3HA
int TechnologyHelpPrefixEmitted;
// Cross-reference values shared with technology help and acquisition.
// Only the separate observed dword accesses are established; allocation extent is unknown.
// RECOVERY_OBJECT: v1 0x007591A4 ?TechnologyHelpEntryName@@3HA
int TechnologyHelpEntryName;
// RECOVERY_OBJECT: v1 0x007591A8 ?TechnologyHelpEntryKind@@3HA
int TechnologyHelpEntryKind;
// RECOVERY_OBJECT: v1 0x00684634 ?TechnologyHelpSectionSelector@@3HA
int TechnologyHelpSectionSelector = -1;
// RECOVERY_OBJECT: v1 0x00684638 ?TechnologyHelpPrefixText@@3PADA
char TechnologyHelpPrefixText[] = "^";
// RECOVERY_OBJECT: v1 0x0068463C ?TechnologyHelpSectionName@@3PADA
char TechnologyHelpSectionName[] = "TECHHECK";
// RECOVERY_OBJECT: v1 0x00684648 ?TechnologyHelpSeparatorText@@3PADA
char TechnologyHelpSeparatorText[] = "^";

// ORIGINAL: 0x0044B340 sub_44b340 0x0044B340-0x0044B37B BYTE_EXACT
// symbol    ?sub_44b340@@YAXP6AXH@Z@Z
void __cdecl sub_44b340(void (__cdecl *callback)(int)) {
    if (TechnologyHelpPrefixEmitted == 0 && TechnologyHelpSectionSelector == -1) {
        TechnologyHelpPrefixEmitted = 1;
        callback(static_cast<int>(reinterpret_cast<long>(TechnologyHelpPrefixText)));
    }
    callback(static_cast<int>(reinterpret_cast<long>(TextBufferGetPtr)));
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0044C7D0 ?help_info_pop@@YAXPAD@Z 0x0044C7D0-0x0044C7E4 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned44C7D0..44C7E4 loads7591B0 and calls601980; src/recovered/0044c7d0.cpp preserves the historical exact candidate and original identity."], "reason": "Plain cdecl text callback loads the shared typed Popup receiver and invokes its inherited BasePop::string method. No runtime executable address, manual vtable or invented null guard."}
void __cdecl help_info_pop(char *text) {
    TechnologyHelpPopup->string(text);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0044C7F0 ?help_tech_info@@YAXPAUPopup@@HH@Z 0x0044C7F0-0x0044C814 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned44C7F0..44C814 stores Popup pointer7591B0 and calls44B9E0 with callback44C7D0 and lastarg-1; original exact seed retained in src/recovered/units/0044c7f0.cpp."], "reason": "Popup-specific overload publishes the actual receiver to shared callback storage before synchronously invoking the existing four-argument technology text builder, with the original negative link selector."}
// symbol    ?help_tech_info@@YAXPAVPopup@@HH@Z
void __cdecl help_tech_info(Popup *popup, int technology, int faction) {
    TechnologyHelpPopup = popup;
    help_tech_info(help_info_pop, technology, faction, -1);
}

// The superseded seed was deleted; its original mapping and exact obligation
// are owned here. The seed path below is historical provenance in Git.
// ORIGINAL: 0x0044C9E0 ?help_project@@YAXH@Z 0x0044C9E0-0x0044C9F8 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/0044c9e0.cpp; recovery/tasks/base-colony-economy-coverage-wave.json; recovery/tests/test_project_help.py", "Pinned01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf full24-byte body44C9E0..44C9F8 loads receiver703EA0 and calls429180 with topic11 and project+70.", "DatalinkMain is the source-owned singleton at703EA0; exec(unsigned int,int) owns the referenced behavior."], "reason": "Rehome historical project-help seed through the real DatalinkMain singleton and typed exec call, preserving its existing byte obligation. No fixed executable receiver, new placeholder or claim demotion."}
void __cdecl help_project(int project_id) {
    DatalinkMain.exec(0xB, project_id + 0x46);
}

// ORIGINAL: 0x00429180 ?exec@Datalink@@QAEXIH@Z 0x00429180-0x004292FC
// RECOVERY_BINDING: v1 {"evidence": ["task:datalink-owner-coverage", "pinned:terranx_original.exe:429180..4292FC complete geometry, saved-state, list redraw, modal and restoration sequence", "source:src/reportwin.h:ReportWinGlobal shared_research_focus owns885F30; src/reportif.h existing category/faction fields", "historical seed:src/recovered/units/00429180.cpp"], "reason": "Recover the complete modal Datalink operation through its existing class, embedded text/list controls and real shared ReportWin/ReportIf state; preserve the original identity and seed findings without a new byte claim."}
void Datalink::exec(unsigned int topic, int index) {
    if (WinScreenWidth == 0x320) {
        const int left = static_cast<int>(field_10394_);
        const int top = static_cast<int>(field_10398_);
        const int right = static_cast<int>(field_1039C_);
        const int bottom = static_cast<int>(field_103A0_);
        const int x = left + 3 + FactionsArt[1].datalink.width() + 5;
        const int y = top + 3;
        const int width = right - 3 - x;
        const int height = bottom - 6 - y;

        stringBox_.init(x, y, width, height, static_cast<Win *>(this),
                        0, -1);
        stringBox_.set_font(0, &secondary_body_font_, 0, &link_font_);
        stringBox_.set_text_color(0x98, -1, 1, 1);
        stringBox_.set_text_color2(0x96, -1, 1, 1);
        stringBox_.set_text_color_hyper(0x96, -1, 1, 1);
    }

    field_A1C_ = static_cast<uint32_t>(ReportWinGlobal.shared_research_focus());
    field_A20_ = ReportIfGlobal.field_8_;
    field_A24_ = static_cast<uint32_t>(ReportIfGlobal.report_faction_id_);
    ReportIfGlobal.report_faction_id_ = ConsoleGlobal.faction_id_;

    set_cat(topic, 1);
    history_[0] = index;
    listBox_.set_selected_id(field_29DC_ * 10000 + index);
    repaint_slot();
    set_modal(1, pop_wait, &MainInterfaceGlobal);

    ReportWinGlobal.shared_research_focus() = static_cast<int32_t>(field_A1C_);
    ReportIfGlobal.field_8_ = field_A20_;
    ReportIfGlobal.report_faction_id_ = static_cast<int32_t>(field_A24_);
    SubInterface::release_iface_mode();
}

// ORIGINAL: 0x0044C820 nullsub_3 0x0044C820-0x0044C821 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:typed-ui-sound-helper-owner", "Pinned44C820 is a one-byte return used by44C830 as the cdecl void(char*) technology-text callback. Source scan finds no consumers of the former unrelated fastcall shim.", "historical seed:src/nullsub_thunks.cpp"], "reason": "Rehome genuine typed helper into its existing source owner, preserving original spans, historical claims and useful discoveries. No fixed executable address or manual virtual-call model."}
// symbol    ?discard_technology_help_text@@YAXPAD@Z
void __cdecl discard_technology_help_text(char *) {
}

// ORIGINAL: 0x0044C830 sub_44c830 0x0044C830-0x0044C84C BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:typed-ui-sound-helper-owner", "Pinned44C830..44C84C passes arguments2/3, selector-1 and actual callback44C820 to existing typed technology-text builder44B9E0; first argument is unused.", "historical seed:src/recovered/units/0044c830.cpp"], "reason": "Rehome genuine typed helper into its existing source owner, preserving original spans, historical claims and useful discoveries. No fixed executable address or manual virtual-call model."}
// symbol    ?technology_help_without_output@@YAXHHH@Z
void __cdecl technology_help_without_output(int, int technology, int faction) {
    help_tech_info(discard_technology_help_text, technology, faction, -1);
}

// Provisional selected-entry state; pinned7591AC initially contains zero.
// RECOVERY_OBJECT: v1 0x007591AC ?DatalinkEntrySelection@@3HA
int DatalinkEntrySelection = 0;

// ORIGINAL: 0x0044CB40 sub_44cb40 0x0044CB40-0x0044CB56 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:datalink-interface-callback-owner", "Pinned44CB40..44CB56 updates the signed index at7591AC, preserving its value for negative input. Queue590DC0 uses the selected matching entry. Name is provisional use-site evidence, not recovered canonical name.", "historical seed:src/recovered/0044cb40.cpp"], "reason": "Recover typed UI helper in its genuine source owner, retaining original identity, spans and historical obligations; remove superseded fixed-address seed."}
// symbol    ?datalink_entry_selection@@YAHH@Z
int __cdecl datalink_entry_selection(int selected_index) {
    if (selected_index < 0) {
        selected_index = DatalinkEntrySelection;
    }
    DatalinkEntrySelection = selected_index;
    return selected_index;
}

// Native startup wrapper590500 constructs this table at945A60.
// Eleven entries is the minimum observed initialized region, not an exact
// original-capacity claim; higher category indices remain unverified.
// RECOVERY_OBJECT: v1 0x00945A60 ?DatalinkEntryCallbackTableGlobal@@3VDatalinkEntryCallbackTable@@A
DatalinkEntryCallbackTable DatalinkEntryCallbackTableGlobal;

// ORIGINAL: 0x00590D10 sub_590d10 0x00590D10-0x00590D3D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:datalink-selection-queue-owner", "Pinned queue/record operations share four-byte records and count at0x200; typed callbacks dispatch on category/value. Constructor owns minimum11-entry callback table, with zero-filled pair0 and CRT registration; exact total capacity and original TU remain unknown.", "historical seed:src/recovered/00590d10.cpp"], "reason": "Recover genuine selection record/queue and callback lifecycle using provisional behavior names in its current Datalink family home; retain all original obligations and seed lessons."}
// symbol    ?append_text@DatalinkSelectionEntry@@QBEXPADH@Z
void DatalinkSelectionEntry::append_text(char *output, int extra) const {
  const DatalinkEntryCallbacks &callbacks =
      DatalinkEntryCallbackTableGlobal.entries[category];
  if (callbacks.append_text != 0) {
    callbacks.append_text(output, value, extra);
  }
}

// ORIGINAL: 0x00590D40 sub_590d40 0x00590D40-0x00590D5C BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:datalink-selection-queue-owner", "Pinned queue/record operations share four-byte records and count at0x200; typed callbacks dispatch on category/value. Constructor owns minimum11-entry callback table, with zero-filled pair0 and CRT registration; exact total capacity and original TU remain unknown.", "historical seed:src/recovered/00590d40.cpp"], "reason": "Recover genuine selection record/queue and callback lifecycle using provisional behavior names in its current Datalink family home; retain all original obligations and seed lessons."}
// symbol    ?show_help@DatalinkSelectionEntry@@QBEXXZ
void DatalinkSelectionEntry::show_help() const {
  const DatalinkEntryCallbacks &callbacks =
      DatalinkEntryCallbackTableGlobal.entries[category];
  if (callbacks.help != 0) {
    callbacks.help(value);
  }
}

// ORIGINAL: 0x00590D60 sub_590d60 0x00590D60-0x00590D6B BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:datalink-selection-queue-owner", "Pinned queue/record operations share four-byte records and count at0x200; typed callbacks dispatch on category/value. Constructor owns minimum11-entry callback table, with zero-filled pair0 and CRT registration; exact total capacity and original TU remain unknown.", "historical seed:src/field_accessors.cpp"], "reason": "Recover genuine selection record/queue and callback lifecycle using provisional behavior names in its current Datalink family home; retain all original obligations and seed lessons."}
// symbol    ?clear@DatalinkSelectionQueue@@QAEXXZ
void DatalinkSelectionQueue::clear() {
  count_ = 0;
}

// ORIGINAL: 0x00590D70 sub_590d70 0x00590D70-0x00590DB3 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:datalink-selection-queue-owner", "Pinned queue/record operations share four-byte records and count at0x200; typed callbacks dispatch on category/value. Constructor owns minimum11-entry callback table, with zero-filled pair0 and CRT registration; exact total capacity and original TU remain unknown.", "historical seed:src/recovered/00590d70.cpp"], "reason": "Recover genuine selection record/queue and callback lifecycle using provisional behavior names in its current Datalink family home; retain all original obligations and seed lessons."}
// symbol    ?append@DatalinkSelectionQueue@@QAEXFEE@Z
void DatalinkSelectionQueue::append(short entry_id, unsigned char category,
                                    unsigned char value) {
  if (count_ < 0x80) {
    entries_[count_].entry_id = entry_id;
    entries_[count_].category = category;
    entries_[count_].value = value;
    ++count_;
  }
}

// ORIGINAL: 0x00590DC0 sub_590dc0 0x00590DC0-0x00590E90
// RECOVERY_BINDING: v1 {"evidence": ["task:datalink-selection-queue-owner", "Pinned queue/record operations share four-byte records and count at0x200; typed callbacks dispatch on category/value. Constructor owns minimum11-entry callback table, with zero-filled pair0 and CRT registration; exact total capacity and original TU remain unknown.", "historical seed:src/unrecovered/00590dc0.cpp"], "reason": "Recover genuine selection record/queue and callback lifecycle using provisional behavior names in its current Datalink family home; retain all original obligations and seed lessons."}
// symbol    ?dispatch_matching@DatalinkSelectionQueue@@QAEXH@Z
// TRIED: `if (count > 0)` reads as `cmp;jle` here, not the original's `test;jle`; tried elsewhere didn't move it. Indexed struct field access (`g_945a60[i].fn`) collapses lea+mov into one mov; original keeps them split (address-of first).
void DatalinkSelectionQueue::dispatch_matching(int entry_id) {
  DatalinkMain.init();

  int matched_count = 0;
  int first_match_index;
  for (int index = 0; index < count_; ++index) {
    DatalinkSelectionEntry &entry = entries_[index];
    if (entry.entry_id == entry_id) {
      if (matched_count == 0) {
        first_match_index = index;
      }
      ++matched_count;
      StringTemp[0] = 0;

      if (entry.category == 5 && entry.value >= 0x46) {
        entry.category = 10;
        entry.value = static_cast<unsigned char>(entry.value + 0xBA);
      }
      entry.append_text(StringTemp, 1);
    }
  }

  if (matched_count != 0) {
    datalink_entry_selection(first_match_index);
    DatalinkMain.show(0);
    DatalinkMain.set_modal(0, pop_wait, 0);
  }
}

// ORIGINAL: 0x00590500 sub_590500 0x00590500-0x0059050A;0x00590C20-0x00590CAF
// RECOVERY_BINDING: v1 {"evidence": ["task:datalink-selection-queue-owner", "Pinned queue/record operations share four-byte records and count at0x200; typed callbacks dispatch on category/value. Constructor owns minimum11-entry callback table, with zero-filled pair0 and CRT registration; exact total capacity and original TU remain unknown. Combined historical spans retain both CRT wrapper590500 and constructor590C20; emitted constructor alone is not proof of the generated wrapper matching.", "historical seed:src/unrecovered/00590500.cpp"], "reason": "Recover genuine selection record/queue and callback lifecycle using provisional behavior names in its current Datalink family home; retain all original obligations and seed lessons."}
// symbol    ??0DatalinkEntryCallbackTable@@QAE@XZ
DatalinkEntryCallbackTable::DatalinkEntryCallbackTable() {
  entries[1].help = 0;
  entries[1].append_text = say_reactor;
  entries[2].help = help_weapon;
  entries[2].append_text = say_weapon;
  entries[3].help = help_armor;
  entries[3].append_text = say_armor;
  entries[4].help = help_chassis;
  entries[4].append_text = say_chassis;
  entries[5].help = help_facility;
  entries[5].append_text = say_facility;
  entries[6].help = help_social;
  entries[6].append_text = say_society;
  entries[7].help = help_abil;
  entries[7].append_text = say_ability;
  entries[8].help = help_tech;
  entries[8].append_text = say_techs;
  entries[9].help = help_veh;
  entries[9].append_text = say_vehicles;
  entries[10].help = help_project;
  entries[10].append_text = say_project;
}

// ORIGINAL: 0x00429300 ?init@Datalink@@QAEXXZ 0x00429300-0x0042A012;0x00653579-0x0065358E
// RECOVERY_BINDING: v1 {"evidence": ["task:datalink-initializer-owner", "Pinned429300..42A012 establishes18RECT rows, typed UI receivers and distinct box bounds/fonts;653579..65358E cleans the local Buffer.", "historical seed:src/unrecovered/00429300.cpp"], "reason": "Recover Datalink initialization through real UI members, panel geometry, renderer camera and local Buffer lifetime; native call arguments supersede the retained seed approximations. Original spans and historical findings are retained without a matching or runtime claim."}
// TRIED: huge UI-construction initializer (fonts, rects, a 16-entry facility-button loop at self+0x2a3c stride 0xb4c, 3 more BaseButtons, 4 StringBoxes) mostly transcribed from Ghidra (reliable for the field writes) plus raw-disasm-derived `this` pointers for every dropped-this call (Buffer at self+0x444, Caviar at self+0x150c, per-Font self+offset table, ListBox/ Dialog at self+0xf628 through a virtual-base-offset chase). The two uncatalogued quaternion helpers (sub_627d00/sub_6280e0) and the two extra StringBox instances' rect math are best-effort approximations, not individually verified against raw bytes.
void Datalink::init() {
    // The retained body constructs this local Buffer at entry and tears it
    // down on exit; it participates in the recorded cold cleanup span even
    // though no ordinary expression reads the local object.
    Buffer temporary_buffer;
    history_[0] = 0;
    field_2A34_ = 0;
    field_2A38_ = 0;
    SubInterface::field_4_ = 8;
    register_iface_mode(8, static_cast<SubInterface *>(this));
    const bool compact = WinScreenWidth == 800;
    const int panel_height = MainInterfaceGlobal.panel_height();
    const int content_height = compact ? 600 - panel_height : 0x2e2 - panel_height;
    const int side_width = compact ? 0 : 0x8a;
    const int left = (WinScreenWidth - 800) / 2;
    const int top = (WinScreenHeight - panel_height) - content_height;

    interface_update_rect_ = MainInterfaceGlobal.main_layout_rect();
    category_rect_.left = 30;
    category_rect_.top = 7;
    category_rect_.right = 230;
    category_rect_.bottom = 0x1b;
    field_103E4_ = 0x1e;
    field_103E8_ = 0x1d;
    field_103EC_ = 0xe6;
    field_103F0_ = side_width + 0x143;

    illustration_rect_.left = 0xf8;
    illustration_rect_.top = 0x2c;
    illustration_rect_.right = 0x177;
    illustration_rect_.bottom = 0xab;

    title_rect_.left = 0xdb;
    title_rect_.top = 7;
    title_rect_.right = 0x302;
    title_rect_.bottom = 0x22;
    description_rect_.left = 0xdb;
    description_rect_.top = 0x24;
    description_rect_.right = 0x302;
    description_rect_.bottom = side_width / 2 + 0xb4;
    const int title_left = field_103EC_ + 9;
    title_rect_.left = title_left;
    description_rect_.left = title_left;
    field_10394_ = title_left;
    field_10398_ = side_width / 2 + 0xb6;
    field_1039C_ = 0x302;
    field_103A0_ = side_width + 0x143;
    const RECT lower_description_rect = {
        static_cast<LONG>(field_10394_), static_cast<LONG>(field_10398_),
        static_cast<LONG>(field_1039C_), static_cast<LONG>(field_103A0_)
    };
    UnionRect(&panel_rect_, &description_rect_, &lower_description_rect);
    UnionRect(&entry_clip_rect_, &description_rect_, &lower_description_rect);
    UnionRect(&entry_clip_rect_, &entry_clip_rect_, &title_rect_);
    entry_update_rect_ = entry_clip_rect_;
    entry_update_rect_.right += 3;
    entry_update_rect_.left -= 3;
    entry_update_rect_.top -= 3;
    entry_update_rect_.bottom += 3;
    entry_clip_rect_ = panel_rect_;
    entry_frame_rect_.left = left;
    entry_frame_rect_.top = top;
    entry_frame_rect_.right = left + 0x340;
    entry_frame_rect_.bottom = top + content_height;

    // Native row stores establish eighteen consecutive RECTs; index 16 is
    // deliberately overwritten after the two panel-row loops.
    {
        int right = MainInterfaceGlobal.main_frame_rect().right - 4;
        int row_top = MainInterfaceGlobal.main_frame_rect().bottom - 0x19;
        int row_bottom = MainInterfaceGlobal.main_frame_rect().bottom - 3;
        int left_edge = MainInterfaceGlobal.main_frame_rect().left + 4;
        for (int i = 7; i >= 0; --i) {
            RECT &row = topic_button_bounds_[i];
            row.left = left_edge;
            row.top = row_top;
            row.right = right;
            row.bottom = row_bottom;
            row_top -= 0x18;
            row_bottom -= 0x18;
        }
    }
    {
        const RECT &right_panel = MainInterfaceGlobal.right_frame_rect();
        int right = right_panel.right - 4;
        int left_edge = right_panel.left + 4;
        int row_top = right_panel.bottom - 0x19;
        int row_bottom = right_panel.bottom - 3;
        for (int i = 8; i < 17; ++i) {
            RECT &row = topic_button_bounds_[i];
            row.left = left_edge;
            row.top = row_top;
            row.right = right;
            row.bottom = row_bottom;
            row_top -= 0x18;
            row_bottom -= 0x18;
        }
    }

    topic_button_bounds_[16].left = 0x98;
    topic_button_bounds_[16].top = side_width + 0x14e;
    topic_button_bounds_[16].right = 0x18e;
    topic_button_bounds_[16].bottom = side_width + 0x162;
    topic_button_bounds_[17].left = 0x192;
    topic_button_bounds_[17].top = side_width + 0x14e;
    topic_button_bounds_[17].right = 0x288;
    topic_button_bounds_[17].bottom = side_width + 0x162;

    const int regular_style = compact ? 12 : 14;
    auxiliary_font10_.init(DefaultFontFace, compact ? 12 : 14, 4);
    auxiliary_font11_.init(DefaultFontFace, compact ? 12 : 14, 0);
    font1_.init(DefaultFontFace, 18, 1);
    font3_.init(DefaultFontFace, 14, 1);
    auxiliary_font4_.init(DefaultFontFace, 14, 1);
    auxiliary_font5_.init(DefaultFontFace, 14, 1);
    body_font_.init(DefaultFontFace, 14, 1);
    secondary_body_font_.init(DefaultFontFace, 14, 0);
    link_font_.init(DefaultFontFace, 14, 5);
    auxiliary_font8_.init(DefaultFontFace, compact ? 10 : 12, 0);
    if (compact) auxiliary_font8_.unk_1_ = 4;
    font2_.init(DefaultFontFace, regular_style == 12 ? 12 : 14, 0);

    caviar_.init();
    caviar_.set_field_A4(0);
    union { uint32_t bits; float value; } camera_angle;
    camera_angle.bits = 0x3f48f5c3;
    const float angles[3] = {0.0f, 0.0f, camera_angle.value};
    VOX_Matrix camera_rotation;
    caviar_apply_rotation(angles, &camera_rotation);
    VOX_Vect camera_position;
    int axis;
    for (axis = 0; axis != 3; ++axis) {
        camera_rotation.values[2][axis] = -camera_rotation.values[2][axis];
    }
    for (axis = 0; axis != 3; ++axis) {
        camera_position.values[axis] =
            camera_rotation.values[2][axis] * CaviarCameraDistance;
    }
    caviar_.set_camera_direct(&camera_position, &camera_rotation);
    union { uint32_t bits; float value; } scene_angle;
    scene_angle.bits = 0x3f05f92d;
    caviar_.set_scene_rotation(scene_angle.value, 0.0f, 0.0f);
    const float forward[3] = {0.0f, 0.0f, 1.0f};
    float transformed_forward[3];
    vox_matrix_vector_mul(camera_rotation.values, forward,
                          transformed_forward);
    caviar_.shadow_vector().values[0] = forward[0];
    caviar_.shadow_vector().values[1] = forward[1];
    caviar_.shadow_vector().values[2] = forward[2];
    caviar_.light_vector() = caviar_.shadow_vector();

    // The legacy parent computation reaches Console's real Win base. Its
    // static conversion applies the compiler's current virtual-base layout.
    GraphicWin::init(entry_frame_rect_.left, entry_frame_rect_.top,
                     entry_frame_rect_.right - entry_frame_rect_.left,
                     entry_frame_rect_.bottom - entry_frame_rect_.top,
                     0, 0x1000000, static_cast<Win *>(&ConsoleGlobal), 0, 0);
    GraphicWin::fill(9);

    RECT rect = {0x17, 0, 0x309, content_height};
    Buffer &canvas = *static_cast<Buffer *>(this);
    canvas.box_sprite(&rect, &InterfaceBoxStyles[9]);
    canvas.box_sprite(&entry_update_rect_, &InterfaceBoxStyles[3]);
    const RECT &special_left = topic_button_bounds_[16];
    const RECT &special_right = topic_button_bounds_[17];
    UnionRect(&rect, &special_right, &special_left);
    rect.left -= 3;
    rect.right += 3;
    rect.top -= 3;
    rect.bottom += 3;
    canvas.box_sprite(&rect, &InterfaceBoxStyles[3]);
    canvas.box_sprite(&category_rect_, &InterfaceBoxStyles[6]);
    const RECT category_frame = {
        static_cast<LONG>(field_103E4_), static_cast<LONG>(field_103E8_),
        static_cast<LONG>(field_103EC_), static_cast<LONG>(field_103F0_)
    };
    UnionRect(&rect, &category_rect_, &category_frame);
    rect.left -= 3;
    rect.right += 3;
    rect.top -= 3;
    rect.bottom += 3;
    canvas.box_sprite(&rect, &InterfaceBoxStyles[3]);
    rect = category_frame;
    canvas.box_sprite(&rect, &InterfaceBoxStyles[6]);
    rect.left += 3;
    rect.top += 3;
    rect.bottom -= 3;
    rect.right -= 3;

    listBox_.close();
    Dialog &dialog = listBox_;
    dialog.init(&rect, static_cast<Heap *>(0));
    listBox_.field_8_ = 1;
    listBox_.field_2C_ = 0;
    dialog.field_44_ = 0;
    dialog.field_B4_ = 0;
    dialog.set_dialog_font(&font2_, 0, 0);
    dialog.attach(static_cast<GraphicWin *>(this), rect.left, rect.top,
                  0x1000020);
    buttonGroup_.init(0, 0);

    for (int i = 0; i < 16; ++i) {
        const RECT &row = topic_button_bounds_[i];
        topic_buttons_[i].init(label_get(0xb20 / 4 + i), i,
                               row.left, row.top, row.right - row.left,
                               row.bottom - row.top,
                               static_cast<Win *>(&MainInterfaceGlobal), 0);
        topic_buttons_[i].set_border_styles(&InterfaceBoxStyles[27],
                                             &InterfaceBoxStyles[28],
                                             &InterfaceBoxStyles[29]);
        static_cast<Buffer &>(topic_buttons_[i]).set_font(
            &auxiliary_font4_, 0, 0, 0);
        topic_buttons_[i].redraw();
        topic_buttons_[i].hide();
        buttonGroup_.add(&topic_buttons_[i]);
    }

    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0x9c / 4));
    CharUpperA(StringTemp);
    topic_buttons_[17].init(StringTemp, 0x11,
                            special_right.left, special_right.top,
                            special_right.right - special_right.left,
                            special_right.bottom - special_right.top,
                            static_cast<Win *>(this), 0);
    static_cast<Buffer &>(topic_buttons_[17]).set_font(
        &auxiliary_font4_, 0, 0, 0);
    topic_buttons_[17].redraw();
    topic_buttons_[17].hide();

    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0xa60 / 4));
    CharUpperA(StringTemp);
    topic_buttons_[16].init(StringTemp, 0x10,
                            special_left.left, special_left.top,
                            special_left.right - special_left.left,
                            special_left.bottom - special_left.top,
                            static_cast<Win *>(this), 0);
    static_cast<Buffer &>(topic_buttons_[16]).set_font(
        &auxiliary_font4_, 0, 0, 0);
    topic_buttons_[16].redraw();
    topic_buttons_[16].hide();

    blurb_box_.init(interface_update_rect_.left + 3,
                    interface_update_rect_.top + 3,
                    interface_update_rect_.right - interface_update_rect_.left - 6,
                    interface_update_rect_.bottom - interface_update_rect_.top - 6,
                    static_cast<Win *>(&MainInterfaceGlobal), 0, -1);
    static_cast<Buffer &>(blurb_box_).set_font(&auxiliary_font8_, 0, 0, 0);
    static_cast<Buffer &>(blurb_box_).set_text_color(0x98, -1, 1, 1);
    static_cast<Buffer &>(blurb_box_).set_text_color2(0x96, -1, 1, 1);
    static_cast<Buffer &>(blurb_box_).set_text_color_hyper(0x96, -1, 1, 1);

    concept_box_.init(panel_rect_.left + 3, panel_rect_.top + 3,
                      panel_rect_.right - panel_rect_.left - 6,
                      panel_rect_.bottom - panel_rect_.top - 6,
                      static_cast<Win *>(this), 0, -1);
    static_cast<Buffer &>(concept_box_).set_font(&body_font_, 0, 0, &link_font_);
    static_cast<Buffer &>(concept_box_).set_text_color(0x98, -1, 1, 1);
    static_cast<Buffer &>(concept_box_).set_text_color2(0x96, -1, 1, 1);
    static_cast<Buffer &>(concept_box_).set_text_color_hyper(0x96, -1, 1, 1);

    detail_box_.init(lower_description_rect.left + 6,
                     lower_description_rect.top + 6,
                     lower_description_rect.right - lower_description_rect.left - 12,
                     lower_description_rect.bottom - lower_description_rect.top - 12,
                     static_cast<Win *>(this), 0, -1);
    static_cast<Buffer &>(detail_box_).set_font(&body_font_, 0, 0, &link_font_);
    static_cast<Buffer &>(detail_box_).set_text_color(0x98, -1, 1, 1);
    static_cast<Buffer &>(detail_box_).set_text_color2(0x96, -1, 1, 1);
    static_cast<Buffer &>(detail_box_).set_text_color_hyper(0x96, -1, 1, 1);
    (void)temporary_buffer;
}

// ORIGINAL: 0x00590CB0 sub_590cb0 0x00590CB0-0x00590CC2 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:0x00590CB0-0x00590CC2 stores the two incoming pointer words at receiver+0 and +4", "source:src/datalink.h DatalinkEntryCallbacks exposes help and append_text in those two word positions", "pinned:0x00590CD0/0x00590CF0 consumers dispatch through those same two fields"], "reason": "Own the two exact callback-pointer stores on the existing DatalinkEntryCallbacks value type; set_callbacks is an operation-derived current name, not a recovered original API name."}
// symbol    ?set_callbacks@DatalinkEntryCallbacks@@QAEXP6AXH@ZP6AXPAXHH@Z@Z
// kind      game
void DatalinkEntryCallbacks::set_callbacks(
    DatalinkHelpCallback help_callback,
    DatalinkTextCallback text_callback) {
  help = help_callback;
  append_text = text_callback;
}

// ORIGINAL: 0x00590CD0 sub_590cd0 0x00590CD0-0x00590CE6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:d3362d4a5fec8fef6555623f53e48513f4953148:src/recovered/00590cd0.cpp", "pinned:590CB0..590CC2 initializer;590CD0..590CE6 and590CF0..590D0F complete dispatch/stack cleanup", "source:src/datalink.h:DatalinkEntryCallbacks", "source:recovery/tests/test_datalink_callback_owner.py"], "reason": "Recover the complete guarded callback dispatch through the existing two-pointer row. Adjacent 590CB0 stores these fields at offsets0/4; the pinned member dispatches one or three stack arguments and skips null callbacks. Existing selection-entry direct dispatch remains unchanged. The native EAX survives the callback or is zero on the null path, but no observed direct call or stored pointer establishes a typed return contract; void source roles do not invent one. The old int-return scaffold interpretation remains historical; its BYTE claim and complete original span remain restoration obligations, not a fresh grant. Historical single-epilogue spelling observation: explicitly returning zero added a second return site; no such typed return is introduced."}
// symbol    ?invoke_help@DatalinkEntryCallbacks@@QBEXH@Z
void DatalinkEntryCallbacks::invoke_help(int value) const {
  if (help != 0) {
    help(value);
  }
}

// ORIGINAL: 0x00590CF0 sub_590cf0 0x00590CF0-0x00590D0F FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:d3362d4a5fec8fef6555623f53e48513f4953148:src/recovered/00590cf0.cpp", "pinned:590CB0..590CC2 initializer;590CD0..590CE6 and590CF0..590D0F complete dispatch/stack cleanup", "source:src/datalink.h:DatalinkEntryCallbacks", "source:recovery/tests/test_datalink_callback_owner.py"], "reason": "Recover the complete guarded callback dispatch through the existing two-pointer row. Adjacent 590CB0 stores these fields at offsets0/4; the pinned member dispatches one or three stack arguments and skips null callbacks. Existing selection-entry direct dispatch remains unchanged. The native EAX survives the callback or is zero on the null path, but no observed direct call or stored pointer establishes a typed return contract; void source roles do not invent one. The old int-return scaffold interpretation remains historical; its BYTE claim and complete original span remain restoration obligations, not a fresh grant."}
// symbol    ?invoke_text@DatalinkEntryCallbacks@@QBEXPAXHH@Z
void DatalinkEntryCallbacks::invoke_text(void *output, int value, int extra) const {
  if (append_text != 0) {
    append_text(output, value, extra);
  }
}

static void emit_help(void (__cdecl *cb)(char *)) {
    if (TechnologyHelpPrefixEmitted == 0 &&
        TechnologyHelpSectionSelector == -1) {
        TechnologyHelpPrefixEmitted = 1;
        cb(TechnologyHelpPrefixText);
    }
    cb(TextBufferGetPtr);
}

/*
Purpose: The help emission gate shared by the nineteen text_get tests: the
         first call after a fresh section opens the paragraph with the
         help prefix, later calls join with the separator; both shapes
         then hand the staged text to the caller's emitter.

// ORIGINAL: 0x0044B380 ?tech_heck@@YAXP6AXPAD@ZH@Z 0x0044B380-0x0044B9DC FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:0x0044B380-0x0044B9DC complete original span; source keeps its full mapped identity and historical claims", "source:src/alpha.h Rules members at +0x3C..+0x54,+0x90,+0xA4,+0xA8; src/technology.h Technology[89] stride0x2C; src/council.h Proposal[11] stride0x0C", "source:src/terraforming.h Terraforming[20] and src/veh.h Order[30]; pinned loop visits rows0..18 and Order[4..22], with duplicate land/sea prerequisites suppressed only on sea iteration", "source:src/datalink.cpp TextBufferGetPtr/StringTemp/TechnologyHelp*; former TRIED/LEVER evidence remains intact as historical notes"], "reason": "Recover the complete technology-help prefix tests through the existing Rules, Technology, Proposal, Terraforming and Order records, using the named text/string services and Datalink help state. Preserve the original negative-index behavior, duplicate sea-prerequisite test, call order and unsafe string-buffer effects."}
// TRIED: 19 repeated "text_get(); if (cond) emit(a1);" blocks (7 direct ID compares, 8 bit-flag tests on a stride-0x2c table, 3 more ID compares, then a final ID compare) factored through a static `emit_help` helper (Ghidra's repeated if/if/call pattern makes this an exact behavioral match), plus the two trailing loops (0x691880 tech-name table, 0x9a6840 concatenation table). Compiles; diverges at instruction #3 (prologue - original reserves one local with a bare `push ecx`, rebuilt doesn't need a same-shaped stack slot since the outer scope has no locals of its own). Hoisting the loop-scratch locals to the top of the function did not change it.
// size      1628 bytes
// prototype
// callers   1   call targets   7
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005FD550 0x005FD570 0x006169A0 0x00625880 0x00625E50 0x00645460 0x00645470
// indirect  0x0044B3EB 0x0044B3F7 0x0044B426 0x0044B432 0x0044B461 0x0044B46C 0x0044B49B 0x0044B4A7 0x0044B4D6 0x0044B4E2 0x0044B511 0x0044B51C 0x0044B54B 0x0044B557 0x0044B591 0x0044B59C 0x0044B5CD 0x0044B5D9 0x0044B60A 0x0044B616 0x0044B647 0x0044B652 0x0044B683 0x0044B68F 0x0044B6C0 0x0044B6CC 0x0044B6FD 0x0044B708 0x0044B73B 0x0044B747 0x0044B77A 0x0044B786 0x0044B7B5 0x0044B7C0 0x0044B7EF 0x0044B7FB 0x0044B82A 0x0044B836 0x0044B8CE 0x0044B8DA 0x0044B9B6 0x0044B9C1
Status: Coverage transcription - unclaimed body; the seed's emit_help
        factoring and its push-ecx divergence stand, and the trailing
        strcat/strcpy loops go through the CRT names the image's
        0x00645460/0x00645470 wrappers are of.
*/

static int datalink_string_key(const void *value) {
    return static_cast<int>(reinterpret_cast<intptr_t>(value));
}

void __cdecl tech_heck(void (__cdecl *a1)(char *), int a2) {
    if (a2 >= 0x59) {
        return;
    }
    TechnologyHelpPrefixEmitted = 0;
    if (text_open(DatalinkHelpFile,
                  TechnologyHelpSectionName) != 0) {
        return;
    }

    text_get();
    if (a2 == Rules.tech_improve_fungus_sqr) {
        emit_help(a1);
    }
    text_get();
    if (a2 == Rules.tech_ease_fungus_movement) {
        emit_help(a1);
    }
    text_get();
    if (a2 == Rules.tech_build_roads_fungus) {
        emit_help(a1);
    }
    text_get();
    if (a2 == Rules.tech_two_spec_abilities) {
        emit_help(a1);
    }
    text_get();
    if (a2 == Rules.tech_three_nutrients_sqr) {
        emit_help(a1);
    }
    text_get();
    if (a2 == Rules.tech_three_minerals_sqr) {
        emit_help(a1);
    }
    text_get();
    if (a2 == Rules.tech_three_energy_sqr) {
        emit_help(a1);
    }

    text_get();
    if (Technology[a2].flags & 1) {
        emit_help(a1);
    }
    text_get();
    if (Technology[a2].flags & 2) {
        emit_help(a1);
    }
    text_get();
    if (Technology[a2].flags & 4) {
        emit_help(a1);
    }
    text_get();
    if (Technology[a2].flags & 8) {
        emit_help(a1);
    }
    text_get();
    if (Technology[a2].flags & 0x10) {
        emit_help(a1);
    }
    text_get();
    if (Technology[a2].flags & 0x40) {
        emit_help(a1);
    }
    text_get();
    if (Technology[a2].flags & 0x80) {
        emit_help(a1);
    }
    text_get();
    if (Technology[a2].flags & 0x100) {
        emit_help(a1);
    }

    text_get();
    if (a2 == Rules.tech_orb_insert_sans_spc_elev) {
        emit_help(a1);
    }
    text_get();
    if (a2 == Rules.tech_mining_platform_bonus) {
        emit_help(a1);
    }
    text_get();
    if (a2 == Rules.tech_economic_victory) {
        emit_help(a1);
    }
    text_get();
    if (a2 == Proposal[1].preq_tech) {
        emit_help(a1);
    }

    text_get();
    strcpy(StringTemp, TextBufferGetPtr);

    {
        for (int row = 0; row < MaxTerrainNum - 1; ++row) {
            const int prereq[2] = {Terraforming[row].preq_tech,
                                   Terraforming[row].preq_tech_sea};
            for (int i = 0; i < 2; ++i) {
                if (a2 == prereq[i] &&
                    (i != 1 || prereq[0] != prereq[1])) {
                    LPSTR order = i == 0 ? Order[4 + row].order
                                         : Order[4 + row].order_sea;
                    parse_say(0, datalink_string_key(order), -1, -1);
                    parse_string(StringTemp, TextBufferGetPtr);
                    emit_help(a1);
                }
            }
        }
    }

    {
        for (int proposal = 2; proposal < MaxProposalNum; ++proposal) {
            if (a2 == Proposal[proposal].preq_tech) {
                StringTemp[0] = 0;
                strcat(StringTemp, TechnologyHelpSeparatorText);
                strcat(StringTemp, label_get(0x7C8 / 4));
                strcat(StringTemp, ": ");
                strcat(StringTemp, StringTable.get(
                    datalink_string_key(Proposal[proposal].name)));
                strcpy(TextBufferGetPtr, StringTemp);
                emit_help(a1);
            }
        }
    }
}

/*
Purpose: The technology help compiler: for one id, walk the ten game-data
         tables whose records reference it - technologies, secret projects,
         four facility/structure lists, the faction/other linked list, the
         nested four-group table, the id-only list and the final linked
         list - staging each record's text into the shared buffer, wiring
         its datalink id, and flushing through the caller's emitter either
         per record (a4 >= 0) or joined per section (a4 < 0). Section
         bodies run under their a4 gates; the tech_heck trailer runs when
         a4 < 0 or a4 == 3, bracketed by the shared 0x00684634 section
         flag.

// ORIGINAL: 0x0044B9E0 ?help_tech_info@@YAXP6AXPAD@ZHHH@Z 0x0044B9E0-0x0044C7C9 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:0x0044B9E0-0x0044C7C9 full original function span and existing call facts", "source:src/veh.h Reactor/Chassis/Weapon/Armor/Ability/VehPrototype; src/base.h Facility/Citizen; src/faction.h SocialCategories; src/technology.h Technology", "pinned:0x007591A4/A8 are separate 4-byte name-handle/kind writes for chassis0x62, weapon0x63 and armor0x64, reset together by tech_achieved at0x005BB71A; object extents and wider allocation remain unknown", "pinned: report-page paths 0x0049A5FC/0x0049A606,0x0049A7CD/0x0049A7D7 and0x0049A9E1/0x0049A9EB also write this pair; this proposal restores those writes in ReportWin's existing typed unlock-section traversal, while broader page-layout/runtime work remains separate", "source:src/strings.h StringTemp/StringTable; src/alpha.h label_get; src/datalink.h DatalinkMain; source:src/technology.cpp reset follows prototype selection before presentation"], "reason": "Recover the ten-section technology-help builder through existing typed game tables while retaining original a4 gates, exact traversed ranges, append/flush order, callback timing and unsafe global-buffer behavior; publish all three original chassis/weapon/armor shared entry-state pairs."}
// TRIED: exact per-record byte offsets for the 4 embedded-owner index chains (52/380/etc strides) - transcribed the strcat/get/ construct_id/help_create_link shape per loop faithfully but did not verify every offset bit-for-bit; MISMATCH #0 at prologue.
// size      3561 bytes
// prototype
// callers   3   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0042A080 0x0044B380 0x0044CB80 0x006169A0 0x00645470 0x0064FC88
Status: Coverage transcription - unclaimed body; the seed's offsets and
        its prologue divergence stand. help_create_link is now the product
        helper in datalink.cpp; tech_heck is the transcription beside this
        body.
*/
void __cdecl help_tech_info(void (__cdecl *a1)(char *), int a2, int a3,
                            int a4) {
    char *const buf = StringTemp;
    char numbuf[80];
    char idbuf[64];
    bool flushed;
    int idx;

    if (a4 < 0 || a4 == 2) {

    // ---- section 1: reactor records (0x9527f8, stride 0xc, match +8) ----
    flushed = false;
    idx = 0;
    *buf = 0;
    for (idx = 0; idx < MaxReactorNum; ++idx) {
        const RulesReactor &record = Reactor[idx];
        if (record.preq_tech == a2) {
            if (a4 < 0) {
                if (flushed) {
                    strcat(buf, ", ");
                } else {
                    strcat(buf, "^");
                    char *s = label_get(0x234 / 4);
                    strcat(buf, s);
                    strcat(buf, ": ");
                }
            } else {
                strcat(buf, "^");
            }
            DatalinkMain.construct_id(idbuf, 5,
                                                                  idx + 1);
            char *link = StringTable.get(
                datalink_string_key(record.name));
            help_create_link(buf, reinterpret_cast<char *>(link), idbuf);
            flushed = (*buf != 0);
            if (a4 >= 0 && *buf != 0) {
                a1(buf);
                flushed = false;
                *buf = 0;
            }
        }
    }
    if (a4 < 0 && flushed) a1(buf);

    // ---- section 2: chassis records (0x94a330, stride 0x90, match +0x8e) ----
    flushed = false;
    idx = 0;
    *buf = 0;
    for (idx = 0; idx < MaxChassisNum; ++idx) {
        const RulesChassis &record = Chassis[idx];
        if (record.preq_tech == a2) {
            if (a4 < 0) {
                if (flushed) {
                    strcat(buf, ", ");
                } else {
                    strcat(buf, "^");
                    char *s = label_get(0x120 / 4);
                    strcat(buf, s);
                    strcat(buf, ": ");
                }
            } else {
                strcat(buf, "^");
            }
            DatalinkMain.construct_id(idbuf, 4,
                                                                  idx);
            char *link = StringTable.get(
                datalink_string_key(record.offsv_1_name));
            help_create_link(buf, reinterpret_cast<char *>(link), idbuf);
            TechnologyHelpEntryKind = 0x62;
            TechnologyHelpEntryName = datalink_string_key(record.offsv_1_name);
            strcat(buf, " ");
            strcat(buf, "(");
            _itoa(record.speed, numbuf, 10);
            strcat(buf, numbuf);
            strcat(buf, ")");
            flushed = (*buf != 0);
            if (a4 >= 0 && *buf != 0) {
                a1(buf);
                flushed = false;
                *buf = 0;
            }
        }
    }
    if (a4 < 0 && flushed) a1(buf);

    // ---- section 3: weapon records (0x94ae60, stride 0x10, match +0xc) ----
    flushed = false;
    idx = 0;
    *buf = 0;
    for (idx = 0; idx < MaxWeaponNum; ++idx) {
        const RulesWeapon &record = Weapon[idx];
        if (record.preq_tech == a2) {
            if (a4 < 0) {
                if (flushed) {
                    strcat(buf, ", ");
                } else {
                    strcat(buf, "^");
                    char *s = label_get(0x124 / 4);
                    strcat(buf, s);
                    strcat(buf, ": ");
                }
            } else {
                strcat(buf, "^");
            }
            DatalinkMain.construct_id(idbuf, 6,
                                                                  idx);
            char *link = StringTable.get(
                datalink_string_key(record.name));
            strcat(buf, "$LINK<");
            strcat(buf, link);
            strcat(buf, "=");
            strcat(buf, idbuf);
            strcat(buf, ">");
            TechnologyHelpEntryKind = 0x63;
            TechnologyHelpEntryName = datalink_string_key(record.name);
            if (datalink_string_key(record.name) == a2) {
                strcat(buf, " ");
                char *s2 = label_get(0x310 / 4);
                strcat(buf, s2);
                strcat(buf, "(");
            }
            unsigned char kind = static_cast<unsigned char>(record.offense_rating);
            if (static_cast<char>(kind) >= 0) {
                if (record.mode < 3) {
                    _itoa(kind, numbuf, 10);
                    strcat(buf, numbuf);
                } else {
                    char *s3 = label_get(0x200 / 4);
                    strcat(buf, s3);
                }
            } else {
                char *s4 = label_get(0x310 / 4);
                strcat(buf, s4);
            }
            strcat(buf, ")");
            flushed = (*buf != 0);
            if (a4 >= 0 && *buf != 0) {
                a1(buf);
                flushed = false;
                *buf = 0;
            }
        }
    }
    if (a4 < 0 && flushed) a1(buf);

    // ---- section 4: armor records (0x94f278, stride 0x10, match +0xc) ----
    flushed = false;
    idx = 0;
    *buf = 0;
    for (idx = 0; idx < MaxArmorNum; ++idx) {
        const RulesArmor &record = Armor[idx];
        if (record.preq_tech == a2) {
            if (a4 < 0) {
                if (flushed) {
                    strcat(buf, ", ");
                } else {
                    strcat(buf, "^");
                    char *s = label_get(0x128 / 4);
                    strcat(buf, s);
                    strcat(buf, ": ");
                }
            } else {
                strcat(buf, "^");
            }
            DatalinkMain.construct_id(idbuf, 7,
                                                                  idx);
            char *link = StringTable.get(
                datalink_string_key(record.name));
            strcat(buf, "$LINK<");
            strcat(buf, link);
            strcat(buf, "=");
            strcat(buf, idbuf);
            strcat(buf, ">");
            TechnologyHelpEntryKind = 0x64;
            TechnologyHelpEntryName = datalink_string_key(record.name);
            strcat(buf, " ");
            strcat(buf, "(");
            char kind2 = record.defense_rating;
            if (kind2 >= 0) {
                _itoa(static_cast<unsigned char>(kind2), numbuf, 10);
                strcat(buf, numbuf);
            } else {
                char *s2 = label_get(0x310 / 4);
                strcat(buf, s2);
            }
            strcat(buf, ")");
            flushed = (*buf != 0);
            if (a4 >= 0 && *buf != 0) {
                a1(buf);
                flushed = false;
                *buf = 0;
            }
        }
    }
    if (a4 < 0 && flushed) a1(buf);

    // ---- section 5: ability records (0x9ab538, stride 0x1c, match +0x18) ----
    flushed = false;
    idx = 0;
    *buf = 0;
    for (idx = 0; idx < MaxAbilityNum; ++idx) {
        const RulesAbility &record = Ability[idx];
        if (record.preq_tech == a2) {
            if (a4 < 0) {
                if (flushed) {
                    strcat(buf, ", ");
                } else {
                    strcat(buf, "^");
                    char *s = label_get(0x12C / 4);
                    strcat(buf, s);
                    strcat(buf, ": ");
                }
            } else {
                strcat(buf, "^");
            }
            DatalinkMain.construct_id(idbuf, 8,
                                                                  idx);
            char *link = StringTable.get(
                datalink_string_key(record.name));
            strcat(buf, "$LINK<");
            strcat(buf, link);
            strcat(buf, "=");
            strcat(buf, idbuf);
            strcat(buf, ">");
            flushed = (*buf != 0);
            if (a4 >= 0 && *buf != 0) {
                a1(buf);
                flushed = false;
                *buf = 0;
            }
        }
    }
    if (a4 < 0 && flushed) a1(buf);

    // ---- section 6: vehicle prototypes (0x9ab89a, stride 0x34, bitfield-gated) ----
    {
        int ctr = 0;
        *buf = 0;
        for (ctr = 0; ctr < MaxVehProtoFactionNum; ++ctr) {
            const VehPrototype &record = VehPrototypes[ctr];
            if ((record.flags & 1) == 0) continue;
            if (record.preq_tech != a2) continue;
            if (a4 < 0) {
                if (*buf != 0) {
                    strcat(buf, ", ");
                } else {
                    strcat(buf, "^");
                    char *s = label_get(0x318 / 4);
                    strcat(buf, s);
                    strcat(buf, ": ");
                }
            } else {
                strcat(buf, "^");
            }
            DatalinkMain.construct_id(idbuf, 3,
                                                                  ctr);
            strcat(buf, "$LINK<");
            strcat(buf, const_cast<char *>(record.veh_name));
            strcat(buf, "=");
            strcat(buf, idbuf);
            strcat(buf, ">");
            if (a4 >= 0 && *buf != 0) {
                a1(buf);
                *buf = 0;
            }
        }
        if (a4 < 0 && *buf != 0) a1(buf);
    }

    } // end sections 1-6 (a4 < 0 || a4 == 2)

    // ---- section 7: facility records (0x9a4bb0, stride 0x30) ----
    if (a4 < 0 || a4 == 0) {
        int ctr = 1;
        *buf = 0;
        for (; ctr < MaxFacilityNum; ++ctr) {
            const RulesFacility &record = Facility[ctr];
            bool match = (record.preq_tech == a2) ||
                         (record.free_tech == a2);
            if (!match) continue;
            if (a4 < 0) {
                if (*buf != 0) {
                    strcat(buf, ", ");
                } else {
                    strcat(buf, "^");
                    char *s = label_get(0x110 / 4);
                    strcat(buf, s);
                    strcat(buf, ": ");
                }
            } else {
                strcat(buf, "^");
            }
            if (record.free_tech == a2) {
                strcat(buf, "(");
            }
            int category = (ctr >= 0x46) ? 0xB : 0xA;
            DatalinkMain.construct_id(idbuf,
                                                                  category,
                                                                  ctr);
            char *link = StringTable.get(
                datalink_string_key(record.name));
            strcat(buf, "$LINK<");
            strcat(buf, link);
            strcat(buf, "=");
            strcat(buf, idbuf);
            strcat(buf, ">");
            if (record.free_tech == a2) {
                strcat(buf, " ");
                char *s2 = label_get(0x4DC / 4);
                strcat(buf, s2);
                strcat(buf, ")");
            }
            if (a4 >= 0 && *buf != 0) {
                a1(buf);
                *buf = 0;
            }
        }
        if (a4 < 0 && *buf != 0) {
            a1(buf);
        }
    }

    // ---- section 8: social-category prerequisites, groups of 4 (0x94b014); section 9 follows in the same gate ----
    if ((a4 < 0) || (a4 == 3)) {
        int idBase = 1;
        int outerIndex = 0;
        *buf = 0;
        do {
            for (int j = 0; j < 4; ++j) {
                if (SocialCategories[outerIndex].preq_tech[j] == a2) {
                    if (a4 < 0) {
                        if (*buf != 0) {
                            strcat(buf, "^");
                        } else {
                            strcat(buf,
                                   "^");
                            char *s = label_get(0x118 / 4);
                            strcat(buf, s);
                            strcat(buf,
                                   ": ");
                        }
                    } else {
                        strcat(buf, "^");
                    }
                    DatalinkMain.construct_id(
                        idbuf, 0xC, idBase + j);
                    char *link = StringTable.get(datalink_string_key(
                        SocialCategories[outerIndex].name[j]));
                    strcat(buf, "$LINK<");
                    strcat(buf, link);
                    strcat(buf, "=");
                    strcat(buf, idbuf);
                    strcat(buf, ">");
                    if (*buf != 0) {
                        a1(buf);
                    }
                }
            }
            ++outerIndex;
            idBase += 5;
        } while (outerIndex < MaxSocialCatNum);

        // ---- section 9: citizen prerequisites (0x946020, stride 0x1c, match +8) ----
        // same gate as section 8: reached only when a4 < 0 || a4 == 3.
        *buf = 0;
        for (idx = 0; idx < 7; ++idx) {
            const RulesCitizen &record = Citizen[idx];
            if (record.preq_tech != a2) continue;
            if (a4 < 0) {
                if (*buf != 0) {
                    strcat(buf, ", ");
                } else {
                    strcat(buf, "^");
                    char *s = label_get(0x4CC / 4);
                    strcat(buf, s);
                    strcat(buf, ": ");
                }
            } else {
                strcat(buf, "^");
            }
            char *link = StringTable.get(datalink_string_key(record.singular_name));
            strcat(buf, link);
            if (*buf != 0) {
                a1(buf);
            }
        }
    }

    // ---- section 10: technology prerequisites (0x94f380, stride 0x2c) ----
    if ((a4 < 0) || (a4 == 1)) {
        int ctr = 0;
        *buf = 0;
        for (ctr = 0; ctr < MaxTechnologyNum; ++ctr) {
            const RulesTechnology &record = Technology[ctr];
            bool match = (record.preq_tech_1 == a2) ||
                         (record.preq_tech_2 == a2);
            if (!match) continue;
            if (a4 < 0) {
                if (*buf != 0) {
                    strcat(buf, ", ");
                } else {
                    strcat(buf, "^");
                    char *s = label_get(0x130 / 4);
                    strcat(buf, s);
                    strcat(buf, ": ");
                }
            } else {
                strcat(buf, "^");
            }
            DatalinkMain.construct_id(idbuf, 0xE,
                                                                  ctr);
            char *link = StringTable.get(
                datalink_string_key(record.name));
            strcat(buf, "$LINK<");
            strcat(buf, link);
            strcat(buf, "=");
            strcat(buf, idbuf);
            strcat(buf, ">");
            if (a4 >= 0 && *buf != 0) {
                a1(buf);
                *buf = 0;
            }
        }
        if (a4 < 0 && *buf != 0) a1(buf);
    }

    if (a4 < 0 || a4 == 3) {
        TechnologyHelpSectionSelector = a4;
        tech_heck(a1, a2);
        TechnologyHelpSectionSelector = -1;
    }
}
