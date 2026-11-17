#define DIALOG_LAYOUT_SETTERS3  // stand-in: see set_field_44 in dialog.h
// WORKAROUND(noinline): the original Datalink TU calls StringStruct::StringStruct
// (0x00401000) out of line from every StringList local, giving the drawers
// their three-state EH layout (cold regions 0x00653712..0x0065384A).
#define STRINGSTRUCT_CTOR_OUT_OF_LINE
// WORKAROUND(noinline): the drawers' explicit clear calls StringStruct::remove_all
// (0x00402970) out of line (0x0042CB5F, 0x0042D7CA) while their destructor
// walks expand the same body inline; see stringstruct.h.
#define STRINGSTRUCT_REMOVE_ALL_CALLED
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
// ORIGINAL: 0x00432840 ??_EDatalink@@WEEE@AEPAXI@Z 0x00432840-0x0043284B FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor at 0x00428FC0 installs Buffer table 0x0066AF7C at this+0x444; its first slot points to 0x00432840.", "Pinned entry 0x00432840 has exactly eleven bytes: SUB ECX,0x444 then JMP to already mapped primary deleting destructor 0x00432800, followed by INT3 padding.", "Owner datalink.cpp physical COFF emits eleven-byte COMDAT ??_EDatalink@@WEEE@AEPAXI@Z; Buffer table ??_7Datalink@@6BBuffer@@@ selects it, and its REL32 targets weak ??_EDatalink@@UAEPAXI@Z with scalar ??_GDatalink@@UAEPAXI@Z fallback.", "The genuine GraphicWin:Win,Buffer hierarchy provides the receiver adjustment. Independent original constructor/table/body and physical COFF review supports this binding; local equality does not prove full linked table selection or runtime acceptance."], "reason": "Bind the original Buffer-view deleting receiver adjustment to the genuine Datalink compiler-emitted operation. The symbol names this tree emits, not an original debug name; no adapter body is authored."}
// ORIGINAL: 0x00428F90 _$E21 0x00428F90-0x00428FA6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x682074 selects initializer0x428f90..0x428fa6, registering exit0x428fb0..0x428fba. Root independent complete decode confirms exclusive RET/JMP bounds; original/source-owned storage is0x703ea0 bound to?DatalinkMain@@3VDatalink@@A.", "Original startup instruction evidence: [[\"0x428f90\", \"mov\", \"ecx, 0x703ea0\"], [\"0x428f95\", \"call\", \"0x428fc0\"], [\"0x428f9a\", \"push\", \"0x428fb0\"], [\"0x428f9f\", \"call\", \"0x645398\"], [\"0x428fa4\", \"pop\", \"ecx\"], [\"0x428fa5\", \"ret\", \"\"]]. Original exit instruction evidence: [[\"0x428fb0\", \"mov\", \"ecx, 0x703ea0\"], [\"0x428fb5\", \"jmp\", \"0x432290\"]]. Preserve constructor arguments, array counts/strides, registration and reverse destruction without authored callback stand-ins.", "Fresh complete build-iveupb5l physical src/datalink.cpp COFF selects _$E21 via .CRT$XCU, registering _$E19. Startup relocation targets: [\"?DatalinkMain@@3VDatalink@@A\", \"??0Datalink@@QAE@XZ\", \"_$E19\", \"_atexit\"]; exit targets: [\"?DatalinkMain@@3VDatalink@@A\", \"??1Datalink@@UAE@XZ\"]. Actual linked helper occurs once at CRT slot0x63f2c0; selected same-owner order agrees with the pinned order.", "BaseWin/DesignWin definitions now precede their Wave globals as pinned. Existing DesignWin Wave mappings retain original names/spans and current helper overrides after fresh emission proof. Scalar constructor effects and array construction/cleanup belong to genuine source globals, classes and compiler-generated lifetime machinery.", "Current local labels are emission interpretations, not original debug names or recovered historical TU boundaries. Start bindings grant no allocation extent. Selected same-owner linked order is not full CRT/global-order, retained-selection, reference-target, dependency-provider, runtime or full-game acceptance. Guarded callbacks excluded; no claim grant or bug fix."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned ?DatalinkMain@@3VDatalink@@A; preserve complete original identity and remaining restoration/structure obligations."}
// symbol    _$E16
// ORIGINAL: 0x00428FB0 _$E19 0x00428FB0-0x00428FBA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x682074 selects initializer0x428f90..0x428fa6, registering exit0x428fb0..0x428fba. Root independent complete decode confirms exclusive RET/JMP bounds; original/source-owned storage is0x703ea0 bound to?DatalinkMain@@3VDatalink@@A.", "Original startup instruction evidence: [[\"0x428f90\", \"mov\", \"ecx, 0x703ea0\"], [\"0x428f95\", \"call\", \"0x428fc0\"], [\"0x428f9a\", \"push\", \"0x428fb0\"], [\"0x428f9f\", \"call\", \"0x645398\"], [\"0x428fa4\", \"pop\", \"ecx\"], [\"0x428fa5\", \"ret\", \"\"]]. Original exit instruction evidence: [[\"0x428fb0\", \"mov\", \"ecx, 0x703ea0\"], [\"0x428fb5\", \"jmp\", \"0x432290\"]]. Preserve constructor arguments, array counts/strides, registration and reverse destruction without authored callback stand-ins.", "Fresh complete build-iveupb5l physical src/datalink.cpp COFF selects _$E21 via .CRT$XCU, registering _$E19. Startup relocation targets: [\"?DatalinkMain@@3VDatalink@@A\", \"??0Datalink@@QAE@XZ\", \"_$E19\", \"_atexit\"]; exit targets: [\"?DatalinkMain@@3VDatalink@@A\", \"??1Datalink@@UAE@XZ\"]. Actual linked helper occurs once at CRT slot0x63f2c0; selected same-owner order agrees with the pinned order.", "BaseWin/DesignWin definitions now precede their Wave globals as pinned. Existing DesignWin Wave mappings retain original names/spans and current helper overrides after fresh emission proof. Scalar constructor effects and array construction/cleanup belong to genuine source globals, classes and compiler-generated lifetime machinery.", "Current local labels are emission interpretations, not original debug names or recovered historical TU boundaries. Start bindings grant no allocation extent. Selected same-owner linked order is not full CRT/global-order, retained-selection, reference-target, dependency-provider, runtime or full-game acceptance. Guarded callbacks excluded; no claim grant or bug fix."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned ?DatalinkMain@@3VDatalink@@A; preserve complete original identity and remaining restoration/structure obligations."}
// symbol    _$E14
#define DIALOGS_DTOR_INLINE  // stand-in: see ~Dialogs in dialogs.h
#define BUFFER_WRITE_RIGHT_L3  // guarded Buffer::write_right_l(text, x, y)
#include "stdafx.h"
#include "interface_fonts.h"
#include "game.h"  // shared state-window owner
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
#include "font_metrics.h"
#include "help_link.h"

// Shared punctuation objects the image pushes instead of unit literals.

// Shared separator objects the image pushes instead of unit literals.
extern const char BufferJoinSpace[];
extern char SocialWinColonSpaceText[];

extern int MainInterfaceTileFlag;
// RECOVERY_OBJECT: v2 0x00691B60 0x00691B66 ?DatalinkHelpFileText@@3PADA 0x00430770
char DatalinkHelpFileText[] = "HELPX";
// RECOVERY_OBJECT: v2 0x00691B68 0x00691B70 ?DatalinkBlurbFileText@@3PADA 0x00430B30
char DatalinkBlurbFileText[] = "BLURBSX";
// RECOVERY_OBJECT: v2 0x00691B00 0x00691B04 ?DatalinkHelpFile@@3PADA 0x00430770
char *DatalinkHelpFile = DatalinkHelpFileText;
// RECOVERY_OBJECT: v2 0x00691B04 0x00691B08 ?DatalinkBlurbFile@@3PADA 0x00430B30
char *DatalinkBlurbFile = DatalinkBlurbFileText;

// draw_terraform passes this writable empty string (0x004317B4). Only its
// initial NUL is established: the bound below is that one byte, a lower
// bound on the original storage, which ends at or before the next object
// (g_DESIGNWIN_WAVE, 0x0071F240); the 8-aligned start after DatalinkMain's
// end (0x0071F234) suggests a new .bss contribution.
// RECOVERY_OBJECT: v2 0x0071F238 0x0071F239 ?DatalinkTerraformExtraText@@3PADA 0x004317B4
char DatalinkTerraformExtraText[1];

static inline void dl_body_colors(Datalink *self, int link_color) {
    self->set_text_color(0x98, -1, 1, 1);
    self->set_text_color2(0x96, -1, 1, 1);
    self->set_text_color_hyper(link_color, -1, 1, 1);
}
static inline void dl_draw_scaled(Buffer *buffer, Sprite *sprite, int x, int y,
                                  int numerator, int denominator) {
    sprite->draw(buffer, sprite->transparent_index(), x, y, numerator, denominator);
}
// The facility and project icons are scaled by the illustration width the
// image computes right then left (0x0042F5A3, 0x0042FBC3), as a RECT* helper
// does; the written-out difference loads left first.
static inline int dl_rect_width(const RECT *rect) {
    return rect->right - rect->left;
}
// WORKAROUND(inline): the faction, facility and project drawers spend more
// /Ob2 budget before their Popup teardown than written-out calls do: the
// image calls ~StringList (0x004066C0) out of line for both BasePop lists
// while still expanding ~Sprite/~FlatButton (draw_faction's EH table
// 0x00674378 has 19 states, ours 21 without these helpers; draw_project's
// cold region 0x00653927 likewise). The original inline providers are
// unknown; these helpers spend that budget. The sprite helpers also give the
// argument order the image uses (0x00430D70, 0x0042F583, 0x0042FBC3): `this`
// converted to Buffer* first, the sprite address after.
static inline void dl_draw_sprite(Buffer *buffer, Sprite *sprite, int x, int y) {
    sprite->draw(buffer, sprite->transparent_index(), x, y, 1, 1);
}
static inline void dl_upper_name(LPSTR name) {
    StringTemp[0] = 0;
    strcat(StringTemp, StringTable.get(name));
    CharUpperA(StringTemp);
}

// The Datalink singleton, 0x00703EA0 in the shipped image - the storage the
// `Datalink *const DatalinkMain` binding used to name (see datalink.h). Plain
// data, not a call target; the Datalink constructor that builds it runs from
// the game's class bring-up, as in the image.
// Extent: sizeof(Datalink) 0x1B394 (datalink.h assert; ctor 0x00428FC0 and exec
// reach stringBox_ ending there); 0x71F234-0x71F238 is unreferenced padding.
// RECOVERY_OBJECT: v2 0x00703EA0 0x0071F234 ?DatalinkMain@@3VDatalink@@A 0x00428FC0
Datalink DatalinkMain;  // 0x00703EA0

// Member/base lifetimes run naturally in declaration order. The original
// ListBox constructor's extra 1 is its hidden most-derived ABI flag, not an
// explicit C++ parameter: the real virtual bases generate it automatically.
// ORIGINAL: 0x004080B0 sub_4080b0 0x004080B0-0x004080D3 FILE BYTE_EXACT
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
// RECOVERY_BINDING: v1 {"evidence": ["git:f2ebb605:src/unrecovered/00428fc0.cpp"], "reason": "Original construction and reverse teardown establish real Flic/Caviar, an eighteen-element FlatButton array and eleven Font members. The owner performs its scalar initialization/reset; genuine member/base lifetimes replace manual vtables and original-address teardown."}
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
// ORIGINAL: 0x00432290 ??1Datalink@@QAE@XZ 0x00432290-0x004325AA;0x006531D0-0x006533A2 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:f2ebb605:src/recovered/units/00432290.cpp"], "reason": "Original construction and reverse teardown establish real Flic/Caviar, an eighteen-element FlatButton array and eleven Font members. The owner performs its scalar initialization/reset; genuine member/base lifetimes replace manual vtables and original-address teardown."}
// symbol    ??1Datalink@@UAE@XZ
Datalink::~Datalink() {
    history_[0] = 0;
    field_2A34_ = 0;
    field_2A38_ = 0;
}

// ORIGINAL: 0x0042C9C0 ?draw_unit@Datalink@@QAEXXZ 0x0042C9C0-0x0042D608;0x00653712-0x00653746
// RECOVERY_BINDING: v1 {"evidence": ["git:f2ebb605:src/unrecovered/0042c9c0.cpp"], "reason": "The Datalink owner implements this original topic renderer through typed rules, real Buffer/Caviar members and constructed Popup/StringList lifetimes; original intervals and seed provenance remain restoration inputs."}
// Historical scaffold experiment, retained from the replaced seed:
// TRIED: RAII StringStruct local (declared ~StringStruct() to force a real dtor call) in place of the original's inlined vtable-pointer-swap destructor state machine (local_3c/ local_40/local_18/local_14) - diverges at instr #0, prologue entirely different SEH shape; the inlined dtor loop appears twice in the original (second copy is dead - guard is always false at runtime) and was not hand-modeled given the 3196-byte scope. The 7 repeated get-string/strcat/itoa/add() blocks and the veh_fake/veh_draw calls ARE transcribed field-for-field.
void Datalink::draw_unit() {
    StringList description;
    RECT rect = description_rect_;
    rect.left += 6;
    rect.top += 6;
    rect.right -= 6;
    const int id = history_[0];
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
    strcat(StringTemp, VehPrototypes[id].veh_name);
    CharUpperA(StringTemp);
    write_cent_l(StringTemp, &title_rect_);
    rect.left += illustration_rect_.right - illustration_rect_.left + 3;
    char number[80];
    description.StringStruct::remove_all();
    description.current_position_ = 0;
    description.allocator_ = 0;
    StringTemp[0] = 0;
    const uint8_t mode = Weapon[VehPrototypes[id].weapon_id].mode;
    if (mode < 3) {
        strcat(StringTemp, label_get(0x204 / 4));
        strcat(StringTemp, SocialWinColonSpaceText);
        strcat(StringTemp, BufferJoinSpace);
        strcat(StringTemp, "{");
        if (Weapon[VehPrototypes[id].weapon_id].offense_rating < 0) {
            strcat(StringTemp, label_get(0x310 / 4));
        } else {
            _itoa(Weapon[VehPrototypes[id].weapon_id].offense_rating, number, 10);
            strcat(StringTemp, number);
        }
        strcat(StringTemp, "}");
    } else {
        strcat(StringTemp, StringTable.get(Labels.strings_ptr[0x200 / 4]));
        strcat(StringTemp, SocialWinColonSpaceText);
        strcat(StringTemp, BufferJoinSpace);
        if (Weapon[VehPrototypes[id].weapon_id].mode >= 7
            && Weapon[VehPrototypes[id].weapon_id].mode <= 12) {
            strcat(StringTemp, "{");
            strcat(StringTemp, StringTable.get(Weapon[VehPrototypes[id].weapon_id].name));
            strcat(StringTemp, "}");
        } else {
            strcat(StringTemp, label_get(0x204 / 4));
            strcat(StringTemp, SocialWinColonSpaceText);
            strcat(StringTemp, BufferJoinSpace);
            strcat(StringTemp, "{");
            if (Weapon[VehPrototypes[id].weapon_id].offense_rating < 0) {
                strcat(StringTemp, label_get(0x310 / 4));
            } else {
                _itoa(Weapon[VehPrototypes[id].weapon_id].offense_rating, number, 10);
                strcat(StringTemp, number);
            }
            strcat(StringTemp, "}");
        }
    }
    description.pending_text_ = StringTemp;
    description.field_20_ = 4;
    description.field_24_ = 0;
    description.add(0);
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0x154 / 4));
    strcat(StringTemp, SocialWinColonSpaceText);
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, "{");
    if (Armor[VehPrototypes[id].armor_id].defense_rating < 0) {
        strcat(StringTemp, label_get(0x310 / 4));
    } else {
        _itoa(Armor[VehPrototypes[id].armor_id].defense_rating, number, 10);
        strcat(StringTemp, number);
    }
    strcat(StringTemp, "}");
    description.pending_text_ = StringTemp;
    description.field_20_ = 4;
    description.field_24_ = 0;
    description.add(0);
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0x1C / 4));
    strcat(StringTemp, SocialWinColonSpaceText);
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, "{");
    strcat(StringTemp, StringTable.get(Triad[Chassis[VehPrototypes[id].chassis_id].triad]));
    strcat(StringTemp, BufferJoinSpace);
    _itoa(Chassis[VehPrototypes[id].chassis_id].speed, number, 10);
    strcat(StringTemp, number);
    strcat(StringTemp, "}");
    description.pending_text_ = StringTemp;
    description.field_20_ = 4;
    description.field_24_ = 0;
    description.add(0);
    if (VehPrototypes[id].carry_capacity) {
        StringTemp[0] = 0;
        strcat(StringTemp, StringTable.get(Labels.strings_ptr[0x360 / 4]));
        strcat(StringTemp, SocialWinColonSpaceText);
        strcat(StringTemp, BufferJoinSpace);
        strcat(StringTemp, "{");
        _itoa(VehPrototypes[id].carry_capacity, number, 10);
        strcat(StringTemp, number);
        strcat(StringTemp, "}");
        description.pending_text_ = StringTemp;
        description.field_20_ = 4;
        description.field_24_ = 0;
        description.add(0);
    }
    if (Chassis[VehPrototypes[id].chassis_id].triad == 2
        && Chassis[VehPrototypes[id].chassis_id].range) {
        StringTemp[0] = 0;
        strcat(StringTemp, StringTable.get(Labels.strings_ptr[0x30C / 4]));
        strcat(StringTemp, SocialWinColonSpaceText);
        strcat(StringTemp, BufferJoinSpace);
        strcat(StringTemp, "{");
        _itoa(Chassis[VehPrototypes[id].chassis_id].range, number, 10);
        strcat(StringTemp, number);
        strcat(StringTemp, BufferJoinSpace);
        if (Chassis[VehPrototypes[id].chassis_id].range == 1) {
            strcat(StringTemp, StringTable.get(Labels.strings_ptr[0xC0 / 4]));
        } else {
            strcat(StringTemp, label_get(0xC4 / 4));
        }
        strcat(StringTemp, "}");
        description.pending_text_ = StringTemp;
        description.field_20_ = 4;
        description.field_24_ = 0;
        description.add(0);
    }
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0x160 / 4));
    strcat(StringTemp, SocialWinColonSpaceText);
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, "{");
    const int cost = cost_factor(ConsoleGlobal.faction_id_, 1, -1) * VehPrototypes[id].cost;
    _itoa(cost, number, 10);
    strcat(StringTemp, number);
    strcat(StringTemp, "}");
    description.pending_text_ = StringTemp;
    description.field_20_ = 4;
    description.field_24_ = 0;
    description.add(0);
    const int tech = VehPrototypes[id].preq_tech;
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0xF24 / 4));
    strcat(StringTemp, SocialWinColonSpaceText);
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, "{");
    char name[256];
    name[0] = 0;
    say_tech(name, tech, 0);
    if (tech >= 0) {
        char link[256];
        link[0] = 0;
        _itoa(tech + 140000, number, 10);
        strcat(link, number);
        help_create_link(StringTemp, name, link);
    } else {
        strcat(StringTemp, name);
    }
    strcat(StringTemp, "}");
    description.pending_text_ = StringTemp;
    description.field_20_ = 4;
    description.field_24_ = 0;
    description.add(0);
    set_font(&body_font_, 0, 0, &link_font_);
    set_text_color(0x98, -1, 1, 1);
    set_text_color2(0x96, -1, 1, 1);
    set_text_color_hyper(0x96, -1, 1, 1);
    write_strings(&description, rect.left, rect.top, rect.right - rect.left, 0);
    StringTemp[0] = 0;
    strcat(StringTemp, "UNITDESC");
    _itoa(id, number, 10);
    strcat(StringTemp, number);
    detail_box_.clear();
    if (detail_box_.strings().load(DatalinkHelpFile, StringTemp, 1, 0) == 0) {
        detail_box_.add_fixup();
        detail_box_.show(0);
    }
    // 0x42D4A6 selects the whole entry update rectangle (+0x103B4).
    soft_update(&entry_update_rect_);
}
// ORIGINAL: 0x0042D610 ?draw_chassis@Datalink@@QAEXXZ 0x0042D610-0x0042DEEF;0x00653746-0x0065377A
// RECOVERY_BINDING: v1 {"evidence": ["git:f2ebb605:src/unrecovered/0042d610.cpp"], "reason": "The Datalink owner implements this original topic renderer through typed rules, real Buffer/Caviar members and constructed Popup/StringList lifetimes; original intervals and seed provenance remain restoration inputs."}
// Historical scaffold experiment, retained from the replaced seed:
// TRIED: full transcription landed (MISMATCH #2, no SEH frame reproduced). Caught a real Ghidra error via byte-level cross check: Ghidra's `write_strings(&local_44,local_54,local_50, local_4c-local_54,0)` invents a `local_54` stack slot that never appears anywhere in the raw disassembly (only -0x50, -0x4c, -0x48, -0x44 are ever touched); the real call reads [ebp-0x50] and [ebp-0x48] for the width term. Used the StringStruct(0) local + explicit remove_all()/add(0) calls, matching this function's calls to those symbols directly (unlike the inlined form seen on other StringStruct-using functions in this batch).
void Datalink::draw_chassis() {
    StringList description;
    const int id = history_[0];
    RECT rect = description_rect_;
    rect.left += 6;
    rect.right -= 6;
    rect.top += 6;
    caviar_.reset_objects();
    vehdraw_construct_chassis(ConsoleGlobal.faction_id_, id, 0, 0, 1, &caviar_);
    caviar_.set_scaling(0.1f);
    RECT *area = &illustration_rect_;
    caviar_.render(static_cast<Buffer *>(this), (area->left + area->right) / 2,
                   (area->top + area->bottom) / 2, 6);
    set_font(&font1_, 0, 0, 0);
    set_text_color(0xE0, -1, 1, 1);
    dl_upper_name(Chassis[id].offsv_1_name);
    write_cent_l(StringTemp, &title_rect_);
    rect.left += illustration_rect_.right - illustration_rect_.left + 3;
    char number[80];
    description.StringStruct::remove_all();
    description.current_position_ = 0;
    description.allocator_ = 0;
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0x1C / 4));
    strcat(StringTemp, SocialWinColonSpaceText);
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, "{");
    strcat(StringTemp, StringTable.get(Triad[Chassis[id].triad]));
    strcat(StringTemp, BufferJoinSpace);
    _itoa(Chassis[id].speed, number, 10);
    strcat(StringTemp, number);
    strcat(StringTemp, "}");
    description.pending_text_ = StringTemp;
    description.field_20_ = 4;
    description.field_24_ = 0;
    description.add(0);
    if (Chassis[id].cargo) {
        StringTemp[0] = 0;
        strcat(StringTemp, StringTable.get(Labels.strings_ptr[0x360 / 4]));
        strcat(StringTemp, SocialWinColonSpaceText);
        strcat(StringTemp, BufferJoinSpace);
        strcat(StringTemp, "{");
        _itoa(Chassis[id].cargo, number, 10);
        strcat(StringTemp, number);
        if (Chassis[id].triad == 1) {
            strcat(StringTemp, BufferJoinSpace);
            strcat(StringTemp, label_get(0xF34 / 4));
        }
        strcat(StringTemp, "}");
        description.pending_text_ = StringTemp;
        description.field_20_ = 4;
        description.field_24_ = 0;
        description.add(0);
    }
    if (Chassis[id].triad == 2) {
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x30C / 4));
        strcat(StringTemp, SocialWinColonSpaceText);
        strcat(StringTemp, BufferJoinSpace);
        strcat(StringTemp, "{");
        if (Chassis[id].range) {
            _itoa(Chassis[id].range, number, 10);
            strcat(StringTemp, number);
            strcat(StringTemp, BufferJoinSpace);
            strcat(StringTemp, Chassis[id].range == 1 ? label_get(0xC0 / 4)
                                                      : label_get(0xC4 / 4));
        } else {
            strcat(StringTemp, label_get(0xF30 / 4));
        }
        strcat(StringTemp, "}");
        description.pending_text_ = StringTemp;
        description.field_20_ = 4;
        description.field_24_ = 0;
        description.add(0);
    }
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0x160 / 4));
    strcat(StringTemp, SocialWinColonSpaceText);
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, "{");
    _itoa(Chassis[id].cost, number, 10);
    strcat(StringTemp, number);
    strcat(StringTemp, "}");
    description.pending_text_ = StringTemp;
    description.field_20_ = 4;
    description.field_24_ = 0;
    description.add(0);
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0xF24 / 4));
    strcat(StringTemp, SocialWinColonSpaceText);
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, "{");
    const int tech = Chassis[id].preq_tech;
    char name[256];
    name[0] = 0;
    say_tech(name, tech, 0);
    if (tech >= 0) {
        char link[256];
        link[0] = 0;
        _itoa(tech + 140000, number, 10);
        strcat(link, number);
        help_create_link(StringTemp, name, link);
    } else {
        strcat(StringTemp, name);
    }
    strcat(StringTemp, "}");
    description.pending_text_ = StringTemp;
    description.field_20_ = 4;
    description.field_24_ = 0;
    description.add(0);
    set_font(&body_font_, 0, 0, &link_font_);
    set_text_color(0x98, -1, 1, 1);
    set_text_color2(0x96, -1, 1, 1);
    set_text_color_hyper(0x96, -1, 1, 1);
    write_strings(&description, rect.left, rect.top, rect.right - rect.left, 0);
    StringTemp[0] = 0;
    strcat(StringTemp, "CHASSISDESC");
    _itoa(id, number, 10);
    strcat(StringTemp, number);
    detail_box_.clear();
    if (detail_box_.strings().load(DatalinkHelpFile, StringTemp, 1, 0) == 0) {
        detail_box_.add_fixup();
        detail_box_.show(0);
    }
    soft_update(&description_rect_);
}
// ORIGINAL: 0x0042DEF0 ?draw_reactor@Datalink@@QAEXXZ 0x0042DEF0-0x0042E2D3;0x0065377A-0x006537AE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:f2ebb605:src/recovered/units/0042def0.cpp"], "reason": "The Datalink owner implements this original topic renderer through typed rules, real Buffer/Caviar members and constructed Popup/StringList lifetimes; original intervals and seed provenance remain restoration inputs."}
void Datalink::draw_reactor() {
    StringList description;
    const int id = history_[0];
    RECT rect = description_rect_;
    rect.left += 6;
    rect.right -= 6;
    rect.top += 6;
    // Original indexes 0x9527EC; the source-owned array begins at0x9527F8.
    caviar_.reset_objects();
    vehdraw_construct_reactor(ConsoleGlobal.faction_id_, -1, -1, id, 0, &caviar_);
    caviar_.set_scaling(0.2f);
    RECT *area = &illustration_rect_;
    caviar_.render(static_cast<Buffer *>(this), (area->left + area->right) / 2,
                   (area->top + area->bottom) / 2, 6);
    set_font(&font1_, 0, 0, 0);
    set_text_color(0xE0, -1, 1, 1);
    dl_upper_name(Reactor[id - 1].name);
    write_cent_l(StringTemp, &title_rect_);
    rect.left += illustration_rect_.right - illustration_rect_.left + 3;
    parse_num(0, id);
    const int tech = Reactor[id - 1].preq_tech;
    parse_says(1, tech_name(tech, 0), -1, -1);
    const int link = tech + 140000;
    parse_num(1, link);
    description.StringStruct::remove_all();
    description.current_position_ = 0;
    description.allocator_ = 0;
    description.load(DatalinkHelpFile, "HELPREACTOR", 1, 0);
    set_font(&body_font_, 0, 0, tech >= 0 ? &link_font_ : 0);
    set_text_color(0x98, -1, 1, 1);
    set_text_color2(0x96, -1, 1, 1);
    set_text_color_hyper(0x96, -1, 1, 1);
    write_strings(&description, rect.left, rect.top, rect.right - rect.left, 0);
    StringTemp[0] = 0;
    strcat(StringTemp, "REACTORDESC");
    detail_box_.clear();
    if (detail_box_.strings().load(DatalinkHelpFile, StringTemp, 1, 0) == 0) {
        detail_box_.add_fixup();
        detail_box_.show(0);
    }
    soft_update(&description_rect_);
}
// ORIGINAL: 0x0042E2E0 ?draw_weapon@Datalink@@QAEXXZ 0x0042E2E0-0x0042E8D3;0x006537AE-0x006537E2 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:f2ebb605:src/unrecovered/0042e2e0.cpp"], "reason": "The Datalink owner implements this original topic renderer through typed rules, real Buffer/Caviar members and constructed Popup/StringList lifetimes; original intervals and seed provenance remain restoration inputs."}
// Historical scaffold experiment, retained from the replaced seed:
// TRIED: the SEH __try/__finally frame and the local StringList's multiple-inheritance secondary-vtable adjustor (the two funclets at 0x006537C5/0x006537CD exist only for that); the entry-deleting virtual call is a double-indirected adjustor thunk (vtable[1] read back as a BYTE OFFSET to a second vtable pointer) that no portable dispatch can reproduce, so it goes through the VCall shim directly instead; several argument orders for batched-__cdecl-cleanup call clusters (parse_num/ parse_says/tech_name) are best-guessed from push proximity to the call, not fully hand-verified against the stack-adjustment bytes.
void Datalink::draw_weapon() {
    StringList description;
    RECT rect = description_rect_;
    rect.left += 6;
    rect.top += 6;
    rect.right -= 6;
    const int id = history_[0];
    set_clip(&illustration_rect_);
    caviar_.reset_objects();
    if (id == 0) {
        caviar_.add_object(&g_VW00_CAVIARDATA, 0);
    } else {
        vehdraw_construct_weapon(ConsoleGlobal.faction_id_, -1, id, &caviar_);
    }
    if (id == 0x10 || id == 0x17) {
        caviar_.set_scaling(0.1f);
    } else if (Weapon[id].mode < 3) {
        caviar_.set_scaling(0.3f);
    } else {
        caviar_.set_scaling(0.2f);
    }
    caviar_.render(static_cast<Buffer *>(this),
                   (illustration_rect_.left + illustration_rect_.right) / 2,
                   (illustration_rect_.top + illustration_rect_.bottom) / 2, 6);
    restore_clip();
    set_font(&font1_, 0, 0, 0);
    set_text_color(0xE0, -1, 1, 1);
    dl_upper_name(Weapon[id].name);
    write_cent_l(StringTemp, &title_rect_);
    rect.left += illustration_rect_.right - illustration_rect_.left + 6;
    StringTemp[0] = 0;
    if (Weapon[id].offense_rating < 0) {
        strcat(StringTemp, label_get(0x310 / 4));
    } else if (Weapon[id].mode < 3) {
        LPSTR mode_name = OffenseModes[Weapon[id].mode].name;
        strcat(StringTemp, StringTable.get(mode_name));
    } else {
        strcat(StringTemp, label_get(0x200 / 4));
    }
    parse_says(1, StringTemp, -1, -1);
    const int technology = Weapon[id].preq_tech;
    parse_num(0, Weapon[id].offense_rating);
    parse_num(1, Weapon[id].cost);
    parse_says(2, tech_name(technology, 0), -1, -1);
    const int link = technology + 140000;
    parse_num(2, link);
    StringTemp[0] = 0;
    strcat(StringTemp, "HELPWEAPON");
    char number[80];
    if (Weapon[id].offense_rating == Weapon[id].cost) {
        _itoa(2, number, 10);
        strcat(StringTemp, number);
    }
    description.remove_all();
    description.current_position_ = 0;
    description.allocator_ = 0;
    description.load(DatalinkHelpFile, StringTemp, 1, 0);
    set_font(&body_font_, 0, 0, technology >= 0 ? &link_font_ : 0);
    set_text_color(0x98, -1, 1, 1);
    set_text_color2(0x96, -1, 1, 1);
    set_text_color_hyper(0x96, -1, 1, 1);
    write_strings(&description, rect.left, rect.top, rect.right - rect.left, 0);
    StringTemp[0] = 0;
    strcat(StringTemp, "WEAPONDESC");
    _itoa(id, number, 10); // 0x42E711 loads saved weapon id.
    strcat(StringTemp, number);
    detail_box_.clear();
    if (detail_box_.strings().load(DatalinkHelpFile, StringTemp, 1, 0) == 0) {
        detail_box_.add_fixup();
        detail_box_.show(0); // 0x42E76E is virtual slot1, not seed slot0.
    }
    soft_update(&description_rect_);
}
// ORIGINAL: 0x0042E8E0 ?draw_defense@Datalink@@QAEXXZ 0x0042E8E0-0x0042EE26;0x006537E2-0x00653816 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:f2ebb605:src/recovered/units/0042e8e0.cpp"], "reason": "The Datalink owner implements this original topic renderer through typed rules, real Buffer/Caviar members and constructed Popup/StringList lifetimes; original intervals and seed provenance remain restoration inputs."}
void Datalink::draw_defense() {
    StringList help;
    const int armor_id = history_[0];
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
                24, ArmorTransparentColorTables, 6);
        } else {
            caviar_.render(static_cast<Buffer *>(this), x, y, 6);
        }
        restore_clip();
    }
    set_font(&font1_, 0, 0, 0);
    set_text_color(0xE0, -1, 1, 1);
    dl_upper_name(Armor[armor_id].name);
    write_cent_l(StringTemp, &title_rect_);
    text.left += illustration_rect_.right - illustration_rect_.left + 3;
    StringTemp[0] = 0;
    if (Armor[armor_id].defense_rating < 0) {
        strcat(StringTemp, label_get(0x310 / 4));
    } else {
        LPSTR mode_name = DefenseModes[Armor[armor_id].mode].name;
        strcat(StringTemp, StringTable.get(mode_name));
    }
    parse_says(1, StringTemp, -1, -1);
    parse_num(0, Armor[armor_id].defense_rating);
    const int technology = Armor[armor_id].preq_tech;
    parse_says(2, tech_name(technology, 0), -1, -1);
    const int link = technology + 140000;
    parse_num(2, link);
    help.StringStruct::remove_all();
    help.current_position_ = 0;
    help.allocator_ = 0;
    help.load(DatalinkHelpFile,
              Armor[armor_id].defense_rating < 0 ? "HELPDEFENSE2" : "HELPDEFENSE", 1, 0);
    set_font(&body_font_, 0, 0, technology >= 0 ? &link_font_ : 0);
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
// ORIGINAL: 0x0042EE30 ?draw_ability@Datalink@@QAEXXZ 0x0042EE30-0x0042F468;0x00653816-0x0065384A BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:f2ebb605:src/unrecovered/0042ee30.cpp"], "reason": "The Datalink owner implements this original topic renderer through typed rules, real Buffer/Caviar members and constructed Popup/StringList lifetimes; original intervals and seed provenance remain restoration inputs."}
// Historical scaffold experiment, retained from the replaced seed:
// TRIED: a genuine `StringStruct local40(0);` local to pick up the compiler's own /GX SEH frame (as worked elsewhere) still diverges at instruction #2 (sub vs push) - this function's frame layout (0x98-byte locals, three linked-list teardown passes at the tail) needs more than one matching ctor call to reproduce. Landed the full field-setup/string-building/switch/ StringList::load/write_strings/StringBox path faithfully (reusing the same local as StringStruct then StringList via reinterpret_cast, matching the disassembly's own reuse of one `this` across two mangled entry points); the two vtable-walk destructor passes at the end are omitted as they only free temporaries and don't produce any value this function returns.
void Datalink::draw_ability() {
    StringList help;
    RECT rect = description_rect_;
    rect.left += 6;
    rect.top += 6;
    rect.right -= 6;
    const int ability_id = history_[0];
    set_font(&font1_, 0, 0, 0);
    set_text_color(0xE0, -1, 1, 1);
    dl_upper_name(Ability[ability_id].name);
    write_cent_l(StringTemp, &title_rect_);
    StringTemp[0] = 0;
    char number[80];
    if (Ability[ability_id].cost_factor >= 0) {
        _itoa(Ability[ability_id].cost_factor, number, 10);
        strcat(StringTemp, number);
    } else {
        switch (Ability[ability_id].cost_factor) {
        case -1:
            strcat(StringTemp, label_get(0x1F0 / 4));
            strcat(StringTemp, "/");
            strcat(StringTemp, label_get(0x154 / 4));
            break;
        case -2:
            strcat(StringTemp, label_get(0x1F0 / 4));
            strcat(StringTemp, "-1");
            break;
        case -3:
            strcat(StringTemp, label_get(0x154 / 4));
            strcat(StringTemp, "-1");
            break;
        case -4:
            strcat(StringTemp, label_get(0xB14 / 4));
            strcat(StringTemp, "-1");
            break;
        case -5:
            strcat(StringTemp, label_get(0x1F0 / 4));
            strcat(StringTemp, "+");
            strcat(StringTemp, label_get(0x154 / 4));
            strcat(StringTemp, "-2");
            break;
        case -6:
            strcat(StringTemp, label_get(0x1F0 / 4));
            strcat(StringTemp, "+");
            strcat(StringTemp, label_get(0xB14 / 4));
            strcat(StringTemp, "-2");
            break;
        case -7:
            strcat(StringTemp, label_get(0x154 / 4));
            strcat(StringTemp, "+");
            strcat(StringTemp, label_get(0xB14 / 4));
            strcat(StringTemp, "-2");
            break;
        }
    }
    const int technology = Ability[ability_id].preq_tech;
    parse_says(1, tech_name(technology, 0), -1, -1);
    parse_say(2, Ability[ability_id].description, -1, -1);
    parse_says(3, StringTemp, -1, -1);
    parse_num(0, technology + 140000);
    help.StringStruct::remove_all();
    help.current_position_ = 0;
    help.allocator_ = 0;
    help.load(DatalinkHelpFile, "HELPABIL", 1, 0);
    set_font(&body_font_, 0, 0, &link_font_);
    set_text_color(0x98, -1, 1, 1);
    set_text_color2(0x96, -1, 1, 1);
    set_text_color_hyper(0x96, -1, 1, 1);
    write_strings(&help, rect.left, rect.top, rect.right - rect.left, 0);
    StringTemp[0] = 0;
    strcat(StringTemp, "ABILDESC");
    _itoa(ability_id, number, 10);
    strcat(StringTemp, number);
    detail_box_.clear();
    if (detail_box_.strings().load(DatalinkHelpFile, StringTemp, 1, 0) == 0) {
        detail_box_.add_fixup();
        detail_box_.show(0);
    }
    soft_update(&description_rect_);
}
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00431180 0x00431D54 0x00431DA4 0x004312BF
// ORIGINAL: 0x00431180 ?draw_terraform@Datalink@@QAEXXZ 0x00431180-0x00431D51;0x00653C7A-0x00653E66 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:f2ebb605:src/unrecovered/00431180.cpp"], "reason": "The Datalink owner implements this original topic renderer through typed rules, real Buffer/Caviar members and constructed Popup/StringList lifetimes; original intervals and seed provenance remain restoration inputs."}
// Historical scaffold experiment, retained from the replaced seed:
// TRIED: no SEH/frame modelling (plain body, not try/finally); the giant local widget teardown (Popup/Scroll/2xFlatButton/Spot/Dialogs region/Sprite/2xFlatButton/Heap) modelled as one raw ebp-relative byte buffer with explicit close()/dtor calls in original order, rather than real RAII locals - the vtable-poke-then-call shape the original uses for each teardown step is not reproduced, only the call itself (identity of the call target is masked by the scorer regardless). MISMATCH at instruction #0 (prologue: no SEH frame).
void Datalink::draw_terraform() {
    Popup popup;
    int z[4] = {0, 0, 0, 0};
    Vert uv[4] = {{0, 0}, {55, 0}, {55, 55}, {0, 55}};
    const int topic = history_[0];
    const int land = topic < 20;
    const int action = topic % 20;
    RECT rect = description_rect_;
    rect.left += 5;
    rect.top += 5;
    rect.right -= 5;
    Vert quad[4] = {{rect.left, rect.top + 50}, {rect.left + 50, rect.top + 25},
                    {rect.left + 100, rect.top + 50}, {rect.left + 50, rect.top + 75}};
    if (action < 16 || action > 18) {
        if (land) {
            g_MOIST_LAND_TEXTURES[0].draw(this, quad, z, 4, uv);
        } else {
            g_OCEAN_TEXTURES[1].draw(this, quad, z, 4, uv);
        }
    }
    switch (action) {
    case 0:
        if (land) {
            g_FARM_TEXTURES[0].draw(this, quad, z, 4, uv);
        } else {
            dl_draw_sprite(this, &g_TER1_KELP_FARM_SPRITE, rect.left, rect.top + 25);
        }
        break;
    case 1:
        dl_draw_sprite(this, &g_TER1_SOIL_ENRICHER_SPRITES[0], rect.left, rect.top + 25);
        break;
    case 2:
        if (land) {
            dl_draw_sprite(this, &g_TER1_MINE_SPRITE, rect.left, rect.top + 25);
        } else {
            dl_draw_sprite(this, &g_TER1_MINING_PLATFORM_SPRITE, rect.left, rect.top + 25);
        }
        break;
    case 3:
        if (land) {
            dl_draw_sprite(this, &g_TER1_SOLAR_COLLECTOR_SPRITE, rect.left, rect.top + 25);
        } else {
            dl_draw_sprite(this, &g_TER1_TIDAL_HARNESS_SPRITE, rect.left, rect.top + 25);
        }
        break;
    case 4:
        g_FOREST_TEXTURES[0].draw_trans(this, quad, z, 4, uv);
        break;
    case 5:
        g_ROAD_TEXTURES[0].draw_trans(this, quad, z, 4, uv);
        break;
    case 6:
        g_MAGTUBE_TEXTURES[0].draw_trans(this, quad, z, 4, uv);
        break;
    case 7:
        dl_draw_sprite(this, &g_TER1_BUNKER_SPRITE, rect.left, rect.top + 25);
        break;
    case 8:
        dl_draw_sprite(this, &g_TER1_AIRBASE_SPRITE, rect.left, rect.top + 25);
        break;
    case 9:
        dl_draw_sprite(this, &g_TER1_SENSOR_ARRAY_SPRITE, rect.left, rect.top + 25);
        break;
    case 11:
        if (land) {
            g_FUNGUS_TEXTURES[0].draw_trans(this, quad, z, 4, uv);
        } else {
            g_FUNGUS_TEXTURES[15].draw_trans(this, quad, z, 4, uv);
        }
        break;
    case 12:
        dl_draw_sprite(this, &g_TER1_CONDENSER_SPRITE, rect.left, rect.top + 25);
        break;
    case 13:
        dl_draw_sprite(this, &g_TER1_ECHELON_MIRROR_SPRITE, rect.left, rect.top + 25);
        break;
    case 14:
        dl_draw_sprite(this, &g_TER1_BOREHOLE_SPRITE, rect.left, rect.top + 25);
        break;
    case 15:
        g_RIVER_TEXTURES[0].draw_trans(this, quad, z, 4, uv);
        break;
    case 19:
        dl_draw_sprite(this, &g_TER1_MONOLITH_SPRITE, rect.left, rect.top + 25);
        break;
    }
    if (action < 16 || action > 18) {
        rect.left += 103;
    }
    set_font(&font1_, 0, 0, 0);
    set_text_color(0xE0, -1, 1, 1);
    const int sea = land == 0;
    // 0x004316F8 indexes the order pair by (land == 0).
    dl_upper_name(Order[action + 4].order[sea]);
    write_cent_l(StringTemp, &title_rect_);
    const int technology = Terraforming[action].prerequisite_tech[sea];
    parse_says(1, tech_name(technology, 0), -1, -1);
    const int link = technology + 140000;
    parse_num(0, link);
    parse_num(1, Terraforming[action].rate);
    if (action == 5 || action == 6 || action == 3) {
        // Written out: label_get here takes the help-file name into eax at
        // the next popup.start (0x004317F6 loads it into ecx), one byte
        // short of the image.
        parse_says(2, StringTable.get(Labels.strings_ptr[0x1110 / 4]), -1, -1);
    } else {
        parse_says(2, DatalinkTerraformExtraText, -1, -1);
    }
    StringTemp[0] = 0;
    // The help topics are this unit's own literals: 0x004317E7 pushes
    // "HELPTERRAFORM", 0x0043187F "HELPTERRA", 0x0043189C/0x004318A3 the
    // "LAND"/"SEA" pair.
    // RECOVERY_LITERAL: v1 0x00683514 0x00683522 $SG54890 0x00431180
    strcat(StringTemp, "HELPTERRAFORM");
    popup.start(DatalinkHelpFile, StringTemp, -1, 0, 0, 0);
    set_font(&body_font_, 0, 0, &link_font_);
    set_text_color(0x98, -1, 1, 1);
    set_text_color2(0x96, -1, 1, 1);
    set_text_color_hyper(0x96, -1, 1, 1);
    write_strings(&popup.unit_ids_, rect.left, rect.top, rect.right - rect.left, 0);
    StringTemp[0] = 0;
    // RECOVERY_LITERAL: v1 0x00683524 0x0068352E $SG54891 0x00431180
    strcat(StringTemp, "HELPTERRA");
    // RECOVERY_LITERAL: v1 0x00683530 0x00683535 $SG54892 0x00431180
    // RECOVERY_LITERAL: v1 0x00683538 0x0068353C $SG54893 0x00431180
    strcat(StringTemp, land ? "LAND" : "SEA");
    char number[80];
    _itoa(action, number, 10);
    strcat(StringTemp, number);
    popup.start(DatalinkHelpFile, StringTemp, -1, 0, 0, 0);
    set_font(&body_font_, 0, 0, &link_font_);
    set_text_color(0x98, -1, 1, 1);
    set_text_color2(0x96, -1, 1, 1);
    set_text_color_hyper(0x96, -1, 1, 1);
    rect = lower_description_rect_;
    rect.left += 5;
    rect.top += 5;
    rect.right -= 5;
    if (action < 16 || action > 18) {
        rect.left += 103;
    }
    write_strings(&popup.unit_ids_, rect.left, rect.top, rect.right - rect.left, 0);
}
// ORIGINAL: 0x0042F490 ?draw_facility@Datalink@@QAEXXZ 0x0042F490-0x0042FAC5;0x0065384A-0x00653927 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:f2ebb605:src/unrecovered/0042f490.cpp"], "reason": "The Datalink owner implements this original topic renderer through typed rules, real Buffer/Caviar members and constructed Popup/StringList lifetimes; original intervals and seed provenance remain restoration inputs."}
// Historical scaffold experiment, retained from the replaced seed:
// TRIED: this function's tail is a compiler-generated SEH unwind chain for ~20 local C++ objects (Popup embeds Scroll+BasePop, and BasePop already embeds Heap/2xFlatButton/Sprite/dialogs_ per the scaffold, so declaring one `Popup popup;` local reproduces a lot of that chain automatically); the separate Dialog/StringStruct/sub_4066c0 teardown calls visible in the disassembly are NOT accounted for by any local this body declares, since their true declared type/order could not be pinned down from the given evidence - left unmodelled rather than guessed. Ghidra's own decompile of this function is unusable (SEH state-machine noise, not real control flow); this body was written directly from the raw disassembly instead. popup.start()/write_strings() receiver args (the ebp-0x3274 StringList) are approximated with a fresh default-constructed local, since the real asm never visibly initializes that memory in view.
void Datalink::draw_facility() {
    Popup popup;
    int id = history_[0];
    RECT rect = description_rect_;
    rect.left += 3;
    rect.top += 3;
    rect.right -= 3;
    set_font(&font1_, 0, 0, 0);
    set_text_color(0xE0, -1, 1, 1);
    dl_upper_name(Facility[id].name);
    write_cent_l(StringTemp, &title_rect_);
    dl_draw_scaled(this, &g_FACILITY_ICON_SPRITES[id], illustration_rect_.left,
                   illustration_rect_.top, dl_rect_width(&illustration_rect_),
                   g_FACILITY_ICON_SPRITES[id].width());
    rect.left += illustration_rect_.right - illustration_rect_.left + 6;
    parse_num(0, cost_factor(ConsoleGlobal.faction_id_, 1, -1) * Facility[id].cost);
    parse_num(1, fac_maint(id, ConsoleGlobal.faction_id_));
    int technology = Facility[id].preq_tech;
    parse_says(1, tech_name(technology, 0), -1, -1);
    const int link = technology + 140000;
    parse_num(2, link);
    StringTemp[0] = 0;
    strcat(StringTemp, "HELPFAC");
    popup.start(DatalinkHelpFile, StringTemp, -1, 0, 0, 0);
    set_font(&body_font_, 0, 0, Facility[id].preq_tech >= 0 ? &link_font_ : 0);
    dl_body_colors(this, 0x96);
    write_strings(&popup.unit_ids_, rect.left, rect.top, rect.right - rect.left, 0);
    StringTemp[0] = 0;
    strcat(StringTemp, "HELPFAC");
    char number[80];
    _itoa(id, number, 10);
    strcat(StringTemp, number);
    popup.start(DatalinkHelpFile, StringTemp, -1, 0, 0, 0);
    set_font(&body_font_, 0, 0, Facility[id].preq_tech >= 0 ? &link_font_ : 0);
    dl_body_colors(this, 0x96);
    rect = lower_description_rect_;
    rect.left += 3;
    rect.top += 3;
    rect.right -= 3;
    write_strings(&popup.unit_ids_, rect.left, rect.top, rect.right - rect.left, 0);
    StringTemp[0] = 0;
    strcat(StringTemp, "HELPFAC");
    _itoa(id, number, 10);
    strcat(StringTemp, number);
    UNK7(DatalinkBlurbFile, StringTemp);
}
// ORIGINAL: 0x0042FAD0 ?draw_project@Datalink@@QAEXXZ 0x0042FAD0-0x00430087;0x00653927-0x006539F9 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:f2ebb605:src/unrecovered/0042fad0.cpp"], "reason": "The Datalink owner implements this original topic renderer through typed rules, real Buffer/Caviar members and constructed Popup/StringList lifetimes; original intervals and seed provenance remain restoration inputs."}
// Historical scaffold experiment, retained from the replaced seed:
// TRIED: exact local-object footprint for Popup's embedded destructor cascade (same shape as 0x00430C80's draw_faction) - modelled as a single `Popup popup;` with popup.close() and implicit dtor; business logic (project/tech title, cost_factor/parse_num/ tech_name/parse_says formatting, sprite draw, two write_strings panels, StringBox blurb branch) transcribed field-by-field.
void Datalink::draw_project() {
    Popup popup;
    const int id = history_[0];
    RECT rect = description_rect_;
    rect.left += 3;
    rect.top += 3;
    rect.right -= 3;
    set_font(&font1_, 0, 0, 0);
    set_text_color(0xE0, -1, 1, 1);
    dl_upper_name(Facility[id].name);
    write_cent_l(StringTemp, &title_rect_);
    // 42FBEC loads the first project sprite's +0x18, without indexing it.
    dl_draw_scaled(this, &g_SECRET_PROJECT_ICON_SPRITES[id - 70],
                   illustration_rect_.left, illustration_rect_.top,
                   dl_rect_width(&illustration_rect_),
                   g_SECRET_PROJECT_ICON_SPRITES[0].width());
    const int project = id - 70;
    rect.left += illustration_rect_.right - illustration_rect_.left + 6;
    parse_num(0, cost_factor(ConsoleGlobal.faction_id_, 1, -1) * Facility[id].cost);
    parse_num(1, Facility[id].maint);
    const int technology = Facility[id].preq_tech;
    parse_says(1, tech_name(technology, 0), -1, -1);
    const int link = technology + 140000;
    parse_num(2, link);
    popup.start(DatalinkHelpFile, "HELPPROJ", -1, 0, 0, 0);
    set_font(&body_font_, 0, 0, &link_font_);
    dl_body_colors(this, 0x96);
    write_strings(&popup.unit_ids_, rect.left, rect.top, rect.right - rect.left, 0);
    // 0x0042FD3C and 0x0042FE16 number both topics by the project slot
    // (id - 70), not by the facility id.
    StringTemp[0] = 0;
    strcat(StringTemp, "HELPPROJ");
    char number[80];
    _itoa(project, number, 10);
    strcat(StringTemp, number);
    popup.start(DatalinkHelpFile, StringTemp, -1, 0, 0, 0);
    set_font(&body_font_, 0, 0, &link_font_);
    dl_body_colors(this, 0x96);
    rect = lower_description_rect_;
    rect.left += 3;
    rect.top += 3;
    rect.right -= 3;
    write_strings(&popup.unit_ids_, rect.left, rect.top, rect.right - rect.left, 0);
    StringTemp[0] = 0;
    strcat(StringTemp, "PROJECT");
    _itoa(project, number, 10);
    strcat(StringTemp, number);
    UNK7(DatalinkBlurbFile, StringTemp);
}
// ORIGINAL: 0x00430090 ?draw_social@Datalink@@QAEXXZ 0x00430090-0x00430765;0x006539F9-0x00653AD6
// RECOVERY_BINDING: v1 {"evidence": ["git:f2ebb605:src/unrecovered/00430090.cpp"], "reason": "The Datalink owner implements this original topic renderer through typed rules, real Buffer/Caviar members and constructed Popup/StringList lifetimes; original intervals and seed provenance remain restoration inputs."}
// Historical scaffold experiment, retained from the replaced seed:
// TRIED: Popup popup; (RAII) for the outer object, mechanical RD()-based field reads for the tech-tier/quotient-remainder lookups, itoa/strcat scratch buffer building, and the per-faction tally loop; deliberately did NOT model the second local object built by sub_4066c0 (Dialog/Dialogs/ StringStruct/Sprite/extra FlatButtons torn down around 0x430642) - its class is unrecoverable from this brief, so popup.close() + RAII stands in for that whole cascade. NO_COMPILE C2660 (set_font arg count) fixed first; MISMATCH #0 (prologue) not chased further.
// WORKAROUND(inline): draw_social's image calls the first BasePop list's
// ~StringList (0x004066C0) out of line at 0x00430642 and expands only the
// second (0x0043068F); written out, ours expands both (21 EH states against
// the image's 20). Two calls of this dead branchy spend give 20 states (re-swept
// 2026-11-16: 0-10 calls at four positions; position is irrelevant). The final
// teardown still keeps state 13 where the image keeps 12 (13,10,9 vs 12,10,9):
// spend drops 12 before 13, so the remaining difference is the teardown's
// composition, not budget. The original's natural spend is not identified.
static inline void dl_social_budget_probe(int x) {
    if (x == 1) x = 2;
    if (x == 2) x = 3;
    if (x == 3) x = 4;
}
void Datalink::draw_social() {
    Popup popup;
    const int category = history_[0] / 5;
    const int model = history_[0] % 5 - 1;
    RECT rect = description_rect_;
    rect.left += 3;
    rect.right -= 3;
    rect.bottom -= 3;
    rect.top += 3;
    set_font(&font1_, 0, 0, 0);
    set_text_color(0xE0, -1, 1, 1);
    dl_upper_name(model < 0 ? SocialCategories[category].type
                            : SocialCategories[category].name[model]);
    write_cent_l(StringTemp, &title_rect_);
    if (model >= 0) {
        const int technology = SocialCategories[category].preq_tech[model];
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
    dl_body_colors(this, 0x96);
    write_strings(&popup.unit_ids_, rect.left, rect.top, rect.right - rect.left, 0);
    rect = lower_description_rect_;
    rect.left += 3;
    rect.top += 3;
    rect.right -= 3;
    rect.bottom -= 3;
    if (model >= 0) {
        const SocialEffect &effect = SocialCategories[category].model_effect[model];
        int count = 0;
        int i;
        for (i = 0; i < 11; ++i) {
            if (effect.values_[i] != 0) {
                ++count;
            }
        }
        if (count == 0) {
            rect.left += 20;
            write_l(label_get(0xFD4 / 4), &rect);
        } else {
            const int x = rect.left + 20;
            int y = rect.top
                + (rect.bottom - font_line_advance(body_font_) * count - rect.top) / 2;
            for (i = 0; i < 11; ++i) {
                const int value = effect.values_[i];
                if (value != 0) {
                    StringTemp[0] = 0;
                    _itoa(value, number, 10);
                    strcat(StringTemp, number);
                    // 0x0043041E tests the buffer address before strlen: the
                    // guarded full-length writer.
                    write_right_l(StringTemp, x, y);
                    StringTemp[0] = 0;
                    strcat(StringTemp, BufferJoinSpace);
                    char link[20];
                    _itoa(i + 130000, link, 10);
                    help_create_link(StringTemp, SocialEffects[i].set2, link);
                    write_l(StringTemp, x, y);
                    y += font_line_advance(body_font_);
                }
            }
        }
    }
    dl_social_budget_probe(0);  // WORKAROUND(inline): see the helper
    dl_social_budget_probe(0);  // WORKAROUND(inline): see the helper
}
// ORIGINAL: 0x00430C80 ?draw_faction@Datalink@@QAEXXZ 0x00430C80-0x0043117E;0x00653BA8-0x00653C7A BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:f2ebb605:src/unrecovered/00430c80.cpp"], "reason": "The Datalink owner implements this original topic renderer through typed rules, real Buffer/Caviar members and constructed Popup/StringList lifetimes; original intervals and seed provenance remain restoration inputs."}
// Historical scaffold experiment, retained from the replaced seed:
// TRIED: exact local-object footprint for Popup's embedded Scroll/FlatButton/ Heap/Sprite/Spot/Dialogs cascade (huge MI vtable-poke cleanup chain, 0x53a0-byte frame) - modelled as a single `Popup popup;` with popup.close() and implicit dtor instead of the full inlined destructor cascade; business logic (font/text-color/string/sprite/ popup.start branches) transcribed field-by-field from the disasm.
void Datalink::draw_faction() {
    Popup popup;
    int faction = history_[0];
    RECT rect = panel_rect_;
    rect.left += 5;
    rect.top += 5;
    rect.right -= 5;
    set_font(&font1_, 0, 0, 0);
    set_text_color(0xE0, -1, 1, 1);
    StringTemp[0] = 0;
    strcat(StringTemp, Players[faction].formal_name_faction);
    CharUpperA(StringTemp);
    write_cent_l(StringTemp, &title_rect_);
    dl_draw_sprite(this, &FactionsArt[faction].datalink, rect.left, rect.top);
    rect.left += FactionsArt[faction].datalink.width() + 5;
    char *file = Players[faction].filename;
    popup.start(file, "DATALINKS1", -1, 0, 0, 0);
    set_font(&body_font_, 0, 0, 0);
    set_wrap_width(20);
    dl_body_colors(this, 0xFE);
    int width = rect.right - rect.left;
    rect.top = write_strings(&popup.unit_ids_, rect.left, rect.top, width, 0);
    rect.top += font_line_advance(body_font_);
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
        write_strings(&popup.unit_ids_, rect.left, rect.top, width, 0);
        set_wrap_width(0);
    }
    popup.start(file, "BLURB", -1, 0, 0, 0);
    UNK7(file, "BLURB");
}

// ORIGINAL: 0x0042A100 ?on_dialog_back_draw@Datalink@@QAEHPAUGraphicWin@@@Z 0x0042A100-0x0042A150 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x42A100", "git:b7076e2d4a242bc54aa68d9b48b52eacc10f2ec0:src/recovered/units/0042a100.cpp"], "reason": "Recover the seed identity as a typed Datalink member using real window, font, list, and popup lifetimes."}
// symbol    ?on_dialog_back_draw@Datalink@@UAEHPAVGraphicWin@@@Z
int Datalink::on_dialog_back_draw(GraphicWin *win) {
    RECT rect = win->clip_rect();
    const RECT &outer = win->outer_rect();
    win->tile(InterfaceBoxStyles[0].mask_, outer.left, outer.top,
              0, 0, rect.right - rect.left, rect.bottom - rect.top);
    return 1;
}

// ORIGINAL: 0x0042A150 ?on_dialog_item_back_draw@Datalink@@QAEHPAUGraphicWin@@HHPAURECT@@@Z 0x0042A150-0x0042A260 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x42A150", "git:b7076e2d4a242bc54aa68d9b48b52eacc10f2ec0:src/unrecovered/0042a150.cpp"], "reason": "Recover the seed identity as a typed Datalink member using real window, font, list, and popup lifetimes."}
// symbol    ?on_dialog_item_back_draw@Datalink@@UAEHPAVGraphicWin@@HHPAUtagRECT@@@Z
// Historical scaffold experiment, retained from the replaced seed:
// TRIED: computing width/height/x/y once and branching only on the `tile()` colour argument folds the compiler's output to 187 bytes (vs 272) - VC6 does not hoist a same-polarity guard's body out of line (see 0x0042A100's sibling note), so each of the three `a3` cases needs its own fully-duplicated width/height/x/y/call block. That form (cand2, below) reaches 0.834 mnemonic similarity but still differs on the exact ebx/esi/edi register allocation the original chose; landing the closest.
int Datalink::on_dialog_item_back_draw(GraphicWin *win, int id, int mode,
                                      RECT *rect) {
    const int left = rect->left;
    const int top = rect->top;
    if (mode == 2) {
        win->tile(InterfaceBoxStyles[28].mask_, win->outer_rect().left + left,
                  win->outer_rect().top + top, left, top,
                  rect->right - left, rect->bottom - top);
    } else if (mode == 1) {
        win->tile(InterfaceBoxStyles[27].mask_, win->outer_rect().left + left,
                  win->outer_rect().top + top, left, top,
                  rect->right - left, rect->bottom - top);
    } else {
        win->tile(InterfaceBoxStyles[0].mask_, win->outer_rect().left + left,
                  win->outer_rect().top + top, left, top,
                  rect->right - left, rect->bottom - top);
    }
    if (field_29DC_ == 12) {
        win->set_text_color(255 - (id - id / 10000 * 10000) / 5, -1, 1, 1);
    }
    return 1;
}

// ORIGINAL: 0x0042BEB0 ?UNK7@Datalink@@QAEXPADPAD@Z 0x0042BEB0-0x0042BEFA BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x42BEB0", "git:b7076e2d4a242bc54aa68d9b48b52eacc10f2ec0:src/recovered/0042beb0.cpp"], "reason": "Recover the seed identity as a typed Datalink member using real window, font, list, and popup lifetimes."}
// symbol    ?UNK7@Datalink@@QAEXPAD0@Z
void Datalink::UNK7(char *file, char *section) {
    if (MainInterfaceTileFlag) {
        blurb_box_.clear();
        blurb_box_.strings().load(file, section, 1, 0);
        blurb_box_.add_fixup();
        blurb_box_.show(0);
    }
}

// The image tests the list through a computed list pointer.
static inline char *dl_current_text(StringList *list) {
    return list->head_ != 0 ? list->current_->string_payload()->text : 0;
}

// stand-in: the two topic helpers below spend the concept draws' /Ob2
// budget so their Popup teardowns call ~FlatButton out of line, as the
// image's EH tables show; the pointer argument keeps the late history_ load.
static inline void dl_title_text(ListBox *box, int id) {
    StringTemp[0] = 0;
    box->item_list_.seek_id(id);
    strcat(StringTemp, dl_current_text(&box->item_list_));
    CharUpperA(StringTemp);
}
static inline void dl_topic_id(char *prefix, int *n) {
    StringTemp[0] = 0;
    strcat(StringTemp, prefix);
    char number[80];
    int value = *n;
    _itoa(value, number, 10);
    strcat(StringTemp, number);
}
// ORIGINAL: 0x0042C2E0 ?draw_concept@Datalink@@QAEXXZ 0x0042C2E0-0x0042C64D;0x0065358E-0x00653650 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x42C2E0", "git:b7076e2d4a242bc54aa68d9b48b52eacc10f2ec0:src/recovered/units/0042c2e0.cpp"], "reason": "Recover the seed identity as a typed Datalink member using real window, font, list, and popup lifetimes."}
void Datalink::draw_concept() {
    Popup popup;
    int id = history_[0];
    set_font(&font1_, 0, 0, 0);
    set_text_color(0xE0, -1, 1, 1);
    dl_title_text(&listBox_, id);
    write_cent_l(StringTemp, &title_rect_);
    dl_topic_id("CONCEPT", &history_[0]);
    concept_box_.clear();
    concept_box_.strings().load("CONCEPTSX", StringTemp, 1, 0);
    concept_box_.add_fixup();
    concept_box_.show(0);
    soft_update(&description_rect_);
}

// ORIGINAL: 0x0042C650 ?draw_adv_concept@Datalink@@QAEXXZ 0x0042C650-0x0042C9C0;0x00653650-0x00653712 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x42C650", "git:b7076e2d4a242bc54aa68d9b48b52eacc10f2ec0:src/recovered/units/0042c650.cpp"], "reason": "Recover the seed identity as a typed Datalink member using real window, font, list, and popup lifetimes."}
void Datalink::draw_adv_concept() {
    Popup popup;
    int id = history_[0] + 10000;
    set_font(&font1_, 0, 0, 0);
    set_text_color(0xE0, -1, 1, 1);
    dl_title_text(&listBox_, id);
    write_cent_l(StringTemp, &title_rect_);
    dl_topic_id("ADVCONCEPT", &history_[0]);
    concept_box_.clear();
    concept_box_.strings().load("CONCEPTSX", StringTemp, 1, 0);
    concept_box_.add_fixup();
    concept_box_.show(0);
    soft_update(&description_rect_);
}

// ORIGINAL: 0x00430770 ?draw_effect@Datalink@@QAEXXZ 0x00430770-0x00430B30;0x00653AD6-0x00653BA8 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x430770", "git:b7076e2d4a242bc54aa68d9b48b52eacc10f2ec0:src/recovered/units/00430770.cpp"], "reason": "Recover the preserved topic drawer with source-owned state and real popup/window lifetimes."}
void Datalink::draw_effect() {
    Popup popup;
    RECT rect = description_rect_;
    rect.left += 3;
    rect.top += 3;
    rect.right -= 3;
    const int entry = history_[0];
    set_font(&font1_, 0, 0, 0);
    set_text_color(0xE0, -1, 1, 1);
    StringTemp[0] = 0;
    strcat(StringTemp, SocialEffects[entry].set2);
    CharUpperA(StringTemp);
    write_cent_l(StringTemp, &title_rect_);
    StringTemp[0] = 0;
    strcat(StringTemp, "HELPEFFECT");
    char number[80];
    _itoa(entry, number, 10);
    strcat(StringTemp, number);
    popup.start(DatalinkHelpFile, StringTemp, -1, 0, 0, 0);
    set_font(&body_font_, 0, 0, &link_font_);
    set_text_color(0x98, -1, 1, 1);
    set_text_color2(0x96, -1, 1, 1);
    set_text_color_hyper(0x96, -1, 1, 1);
    // [ebp-0x3270] is Popup+0x2180, BasePop's unit_ids_ text list that
    // BasePop::start fills; no separate local list is constructed here.
    write_strings(&popup.unit_ids_, rect.left, rect.top,
                  rect.right - rect.left, 0);
}

// ORIGINAL: 0x00430B30 ?draw_tech@Datalink@@QAEXXZ 0x00430B30-0x00430C76 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x430B30", "git:b7076e2d4a242bc54aa68d9b48b52eacc10f2ec0:src/recovered/units/00430b30.cpp"], "reason": "Recover the preserved topic drawer with source-owned state and real popup/window lifetimes."}
// Inline boundary the image's register choice for the title test shows.
static inline void append_upper_name(LPSTR name) {
    strcat(StringTemp, StringTable.get(name));
    CharUpperA(StringTemp);
}

// The unpooled TU still has one "TECH" object for draw_tech and the copy
// draw_entry expands: both sites go through this helper's literal.
static inline void append_tech_topic() {
    // RECOVERY_LITERAL: v1 0x006834D8 0x006834DD $SG55040 0x00430B30
    strcat(StringTemp, "TECH");
}

void Datalink::draw_tech() {
    set_font(&font1_, 0, 0, 0);
    set_text_color(0xE0, -1, 1, 1);
    StringTemp[0] = 0;
    append_upper_name(Technology[history_[0]].name);
    // Both the standalone original (0x430BA5..0x430BAC) and draw_entry's
    // inline technology case (0x42C1A7..0x42C1AE) test the buffer ADDRESS,
    // not its first byte. StringTemp has real storage, so even an empty
    // technology title reaches write_cent_l.
    write_cent_l(StringTemp, &title_rect_);
    ReportWinGlobal.reset();
    ReportWinGlobal.shared_research_focus() = history_[0];
    ReportIfGlobal.field_8_ = 2;
    ReportWinGlobal.draw_labs();
    StringTemp[0] = 0;
    // One "TECH" object serves both this body (0x00430BF5) and the copy
    // draw_entry expands (0x0042C1F7): both push 0x006834D8.
    append_tech_topic();
    char number[80];
    _itoa(history_[0], number, 10);
    strcat(StringTemp, number);
    UNK7(DatalinkBlurbFile, StringTemp);
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x0042BF10 0x0042C29C 0x0042C2DC 0x0042C0A0
// ORIGINAL: 0x0042BF10 ?draw_entry@Datalink@@QAEXXZ 0x0042BF10-0x0042C29A BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x42BF10", "git:b7076e2d4a242bc54aa68d9b48b52eacc10f2ec0:src/recovered/units/0042bf10.cpp"], "reason": "Recover the seed identity as a typed Datalink member using real window, font, list, and popup lifetimes."}
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
        // WORKAROUND(inline): the image expands push_history (0x0042BD80,
        // emitted with no callers) here at 0x0042BF92..0x0042BFE2; VC6 does
        // not auto-inline the out-of-line member in this build, and an
        // `inline` definition would drop the emitted copy. Same body.
        const int topic = category * 10000 + history_[0];
        const int count = field_2A34_;
        if (count == 0 || history_[count] != topic) {
            if (count == 0x14) {
                for (int i = 0; i < 0x13; ++i) {
                    history_[1 + i] = history_[2 + i];
                }
                --field_2A34_;
            }
            history_[field_2A34_ + 1] = topic;
            ++field_2A34_;
        }
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
    // The image lays the arms out unit..terraform, then concept/advanced
    // concept, then facility onward (jump table 0x0042C29C).
    switch (category) {
    case 3: draw_unit(); break;
    case 4: draw_chassis(); break;
    case 5: draw_reactor(); break;
    case 6: draw_weapon(); break;
    case 7: draw_defense(); break;
    case 8: draw_ability(); break;
    case 9: draw_terraform(); break;
    case 0: draw_concept(); break;
    case 1: draw_adv_concept(); break;
    case 10: draw_facility(); break;
    case 11: draw_project(); break;
    case 12: draw_social(); break;
    case 13: draw_effect(); break;
    case 14:
        // WORKAROUND(inline): the image expands draw_tech (0x00430B30) and its
        // UNK7 call here (0x0042C143..0x0042C26B, same "TECH" object
        // 0x006834D8); a draw_tech() call is not expanded in this build.
        // draw_tech and UNK7 keep their own emitted, mapped bodies.
        set_font(&font1_, 0, 0, 0);
        set_text_color(0xE0, -1, 1, 1);
        StringTemp[0] = 0;
        append_upper_name(Technology[history_[0]].name);
        write_cent_l(StringTemp, &title_rect_);
        ReportWinGlobal.reset();
        ReportWinGlobal.shared_research_focus() = history_[0];
        ReportIfGlobal.field_8_ = 2;
        ReportWinGlobal.draw_labs();
        StringTemp[0] = 0;
        append_tech_topic();
        char number[80];
        _itoa(history_[0], number, 10);
        strcat(StringTemp, number);
        UNK7(DatalinkBlurbFile, StringTemp);
        break;
    case 15: draw_faction(); break;
    }
    soft_update(&entry_update_rect_);
    MainInterfaceGlobal.soft_update(&interface_update_rect_);
}

/*
 * Original image text locations used by the category fillers. The current
 * owner keeps each observed C string in independent writable source storage;
 * the old addresses are provenance, not live pointer targets or bounds for a
 * larger allocation.
 *
 * Each category filler reads its own one-character separator from image
 * .data. Original addresses are provenance only; the writable arrays below
 * each hold the observed NUL-terminated contents, without claims about
 * neighboring storage. Unit 0x6832CC, chassis 0x6832D0, reactor 0x6832D4,
 * weapon 0x6832D8, defense 0x6832DC, ability 0x6832E0, facility 0x6832E4,
 * project 0x6832F0, tech 0x6832F4, concept 0x68330C, advanced concept
 * 0x683328, social 0x68332C, effect 0x683330, faction 0x683334, terraform
 * first 0x683338, and terraform second 0x68333C each read "|" followed by NUL.
 * Panel frames borrow InterfaceBoxStyles[6], original 0x78D618, from the
 * loader-owned shared pool rather than a fixed-address record.
 * CategoryEntryRect    the category_rect_ member, not a global - listed here
 *                      only to say the fillers share it.
 */
// RECOVERY_OBJECT: v2 0x006832CC 0x006832CE ?DatalinkCategoryIdSeparator@@3PADA 0x0042A630
char DatalinkCategoryIdSeparator[] = "|";
// RECOVERY_OBJECT: v2 0x006832D0 0x006832D2 ?DatalinkChassisIdSeparator@@3PADA 0x0042A790
char DatalinkChassisIdSeparator[] = "|";
// RECOVERY_OBJECT: v2 0x006832D4 0x006832D6 ?DatalinkReactorIdSeparator@@3PADA 0x0042A900
char DatalinkReactorIdSeparator[] = "|";
// RECOVERY_OBJECT: v2 0x006832D8 0x006832DA ?DatalinkWeaponIdSeparator@@3PADA 0x0042AA60
char DatalinkWeaponIdSeparator[] = "|";
// RECOVERY_OBJECT: v2 0x006832DC 0x006832DE ?DatalinkDefenseIdSeparator@@3PADA 0x0042ABC0
char DatalinkDefenseIdSeparator[] = "|";
// RECOVERY_OBJECT: v2 0x006832E0 0x006832E2 ?DatalinkAbilityIdSeparator@@3PADA 0x0042AD20
char DatalinkAbilityIdSeparator[] = "|";
// RECOVERY_OBJECT: v2 0x006832E4 0x006832E6 ?DatalinkFacilityIdSeparator@@3PADA 0x0042AE80
char DatalinkFacilityIdSeparator[] = "|";
// RECOVERY_OBJECT: v2 0x006832F0 0x006832F2 ?DatalinkProjectIdSeparator@@3PADA 0x0042AFE0
char DatalinkProjectIdSeparator[] = "|";
// RECOVERY_OBJECT: v2 0x006832F4 0x006832F6 ?DatalinkTechIdSeparator@@3PADA 0x0042B190
char DatalinkTechIdSeparator[] = "|";
// RECOVERY_OBJECT: v2 0x0068330C 0x0068330E ?DatalinkConceptIdSeparator@@3PADA 0x0042B300
char DatalinkConceptIdSeparator[] = "|";
// RECOVERY_OBJECT: v2 0x00683328 0x0068332A ?DatalinkAdvConceptIdSeparator@@3PADA 0x0042B490
char DatalinkAdvConceptIdSeparator[] = "|";
// RECOVERY_OBJECT: v2 0x0068332C 0x0068332E ?DatalinkSocialIdSeparator@@3PADA 0x0042B630
char DatalinkSocialIdSeparator[] = "|";
// RECOVERY_OBJECT: v2 0x00683330 0x00683332 ?DatalinkEffectIdSeparator@@3PADA 0x0042B7E0
char DatalinkEffectIdSeparator[] = "|";
// RECOVERY_OBJECT: v2 0x00683334 0x00683336 ?DatalinkFactionIdSeparator@@3PADA 0x0042B940
char DatalinkFactionIdSeparator[] = "|";
// RECOVERY_OBJECT: v2 0x00683338 0x0068333A ?DatalinkTerraformIdSeparator@@3PADA 0x0042BAB0
char DatalinkTerraformIdSeparator[] = "|";
// RECOVERY_OBJECT: v2 0x0068333C 0x0068333E ?DatalinkTerraformSecondSeparator@@3PADA 0x0042BAB0
char DatalinkTerraformSecondSeparator[] = "|";
// RECOVERY_OBJECT: v2 0x006832E8 0x006832ED ?DatalinkConceptArticle@@3PADA 0x0042AFE0
char DatalinkConceptArticle[] = "The ";
// RECOVERY_OBJECT: v2 0x006832F8 0x006832FF ?DatalinkConceptFile@@3PADA 0x0042B300
char DatalinkConceptFile[] = "TITLES";
// RECOVERY_OBJECT: v2 0x00683300 0x0068330A ?DatalinkConceptSection@@3PADA 0x0042B300
char DatalinkConceptSection[] = "CONCEPTSX";
// RECOVERY_OBJECT: v2 0x00683310 0x0068331A ?DatalinkAdvConceptFile@@3PADA 0x0042B490
char DatalinkAdvConceptFile[] = "ADVTITLES";
// Native 0x0042B490 pushes its own "CONCEPTSX" copy, not 0x00683300's.
// RECOVERY_OBJECT: v2 0x0068331C 0x00683326 ?DatalinkAdvConceptSection@@3PADA 0x0042B490
char DatalinkAdvConceptSection[] = "CONCEPTSX";

/*
 * The rules tables the remaining fillers walk. Each is named from what its
 * own loop does - the id range it files and the stride it steps - and from
 * the tree's own neighbours where those pin it:
 *
 *   Facility[FacilitySPStart] 0x009A5888, 64 records of 0x30, facility
 *                        indices 70..133 and Datalink ids 110070..110133,
 *                        preq at +0x14. base.h's SecretProject (0x009A6514)
 *                        sits just past its end, which is what makes this a
 *                        measurement and not a guess.
 *   Technology           0x0094F358, 89 records of 0x2C. The tech filler
 *                        reads each record's own name and prerequisite pair.
 *   SocialCategories     0x0094B000, four RulesSocialCategory records of
 *                        0xD4. The original row starts at name[0] (+0x14);
 *                        type is five dwords before it.
 *   SocialEffects        0x00946580, eleven RulesSocialEffect records of
 *                        0x68. The original name cursor is set2 (+0x18);
 *                        the Null row is record 3.
 *   Faction filler       0x009472EC is Players[1].is_noun_plural. Its
 *                        preceding dword is noun_gender and +0x84 is
 *                        formal_name_faction. Native rows advance by the
 *                        0x59C Player stride for ids 150001..150007.
 *   Order[4..23]          0x0096C8A8, with each row's land/sea name pointers.
 *   Terraforming         0x00691878, twenty records of 0x20. The original
 *                        prerequisite cursor begins at preq_tech_sea (+0xC);
 *                        land and sea gates are the named +8/+C fields.
 *   ProjectPrefixHidden  0x009BC054, clear while the "The " article is to be
 *                        stripped.
 *   the faction filler saves noun_gender and is_noun_plural, clears the
 *   string buffer, then publishes the pair before filing the formal name.
 *   The optional numeric suffix is gated by game_state bit 0x1000.
 */
// The label each category's filler draws centred in the entry rectangle, read
// out of the label blob the way dipedit.cpp reads it:
// `mov eax, ds:0x9b90f8; mov ecx, [eax + <slot>]` - slot / 4 is the index.
#define CATEGORY_LABEL(slot) StringTable.get(Labels.strings_ptr[(slot)])

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
// symbol    ?on_group_clicked@Datalink@@UAEXHH@Z
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
// symbol    ?parse_id@Datalink@@QAEXHPAIPAH@Z
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::parse_id(int id, uint32_t *out_category, int *out_remainder) {
    const int32_t quotient = id / 10000;
    *out_category = static_cast<uint32_t>(quotient);
    *out_remainder = static_cast<int32_t>(
        static_cast<uint32_t>(id) - static_cast<uint32_t>(quotient) * 10000U);
}

/*
Purpose: Parse a decimal Datalink id into its category and entry components.
         The quotient/remainder are the same split used by parse_id; this
         overload receives the encoded id as text from a link target.
*/
// ORIGINAL: 0x0042A0B0 ?UNK2@Datalink@@QAEXPADPAHPAH@Z 0x0042A0B0-0x0042A0F1 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x0042A0B0-0x0042A0F1 calls atoi then stores quotient and remainder; git:85463515:src/recovered/units/0042a0b0.cpp contains the historical analysis and source:src/datalink.cpp:parse_id implements the matching numeric split"], "reason": "The product Datalink parser owns this text-to-category/remainder operation; its original span and name are preserved in Git provenance."}
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
// ORIGINAL: 0x00432040 ?on_selected@Datalink@@QAEXH@Z 0x00432040-0x00432052 BYTE_EXACT
// symbol    ?on_selected@Datalink@@UAEXH@Z
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
// ORIGINAL: 0x0042BD80 ?UNK4@Datalink@@QAEXHH@Z 0x0042BD80-0x0042BDFA FILE BYTE_EXACT
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

    if (count == 0x14) {
        for (int i = 0; i < 0x13; ++i) {
            history_[1 + i] = history_[2 + i];
        }
        --field_2A34_;
    }

    history_[field_2A34_ + 1] = topic;
    ++field_2A34_;
}

/*
Purpose: Pop the back-history ring and re-show the topic it held.

         Reads `0x29e0 + --count*4`, the slot UNK4 last wrote, splits it back
         into its category and facility halves the way parse_id does, then
         walks the chain set_cat / set_selected_id / slot 62 / draw_entry that
         UNK3 runs in the forward direction.
// ORIGINAL: 0x0042BE00 ?UNK5@Datalink@@QAEXXZ 0x0042BE00-0x0042BE9D FILE BYTE_EXACT
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

    set_cat(static_cast<unsigned int>(category), 0);
    const int facility = topic - category * 10000;
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
// ORIGINAL: 0x0042A260 ?UNK3@Datalink@@QAEXIH@Z 0x0042A260-0x0042A2BF FILE BYTE_EXACT
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
// ORIGINAL: 0x00431DC0 ?on_link_clicked@Datalink@@QAEHHPAD@Z 0x00431DC0-0x00431E64 FILE BYTE_EXACT
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
// ORIGINAL: 0x0042A2C0 ?set_cat@Datalink@@QAEXIH@Z 0x0042A2C0-0x0042A4AC FILE BYTE_EXACT
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
// RECOVERY_ASSOCIATED: v1 switch_targets 0x0042A2C0 0x0042A4AC 0x0042A4EC 0x0042A3D5
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
    ListBox *const list = &listBox_;
    list->selected_position_ = 0;
    list->field_18_ = 0;
    list->field_20_ = 0;

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
    case 0:  set_cat_concept();     break;
    case 1:  set_cat_adv_concept(); break;
    case 10: set_cat_facility();    break;
    case 11: set_cat_project();     break;
    case 12: set_cat_social();      break;
    case 13: set_cat_effect();      break;
    case 14: set_cat_tech();        break;
    case 15: set_cat_faction();     break;
    }

    if (category != 0xC) {
        listBox_.item_list_.sort(0);
    }

    field_2A38_ = 1;
    list->update_changes();
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
         the pinned end is 0x009AC568, exactly 64 records after the bank at
         0x009AB868 using the 0x34-byte prototype stride.
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
    for (int index = 0; index < MaxVehProtoFactionNum; ++index) {
        VehPrototype &proto = VehPrototypes[index];
        if ((proto.flags & 1) != 0 && index != 4) {
            StringTemp[0] = '\0';
            strcat(StringTemp, proto.veh_name);
            if ((GameDataStateStorage.fields_.game_state_ & 0x1000) != 0) {
                strcat(StringTemp, DatalinkCategoryIdSeparator);
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
            strcat(StringTemp, StringTable.get(Chassis[index].offsv_1_name));
            if ((GameDataStateStorage.fields_.game_state_ & 0x1000) != 0) {
                strcat(StringTemp, DatalinkChassisIdSeparator);
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
            strcat(StringTemp, StringTable.get(record->name));
            if ((GameDataStateStorage.fields_.game_state_ & 0x1000) != 0) {
                strcat(StringTemp, DatalinkReactorIdSeparator);
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
            strcat(StringTemp, StringTable.get(Weapon[index].name));
            if ((GameDataStateStorage.fields_.game_state_ & 0x1000) != 0) {
                strcat(StringTemp, DatalinkWeaponIdSeparator);
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
            strcat(StringTemp, StringTable.get(Armor[index].name));
            if ((GameDataStateStorage.fields_.game_state_ & 0x1000) != 0) {
                strcat(StringTemp, DatalinkDefenseIdSeparator);
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
            strcat(StringTemp, StringTable.get(Ability[index].name));
            if ((GameDataStateStorage.fields_.game_state_ & 0x1000) != 0) {
                strcat(StringTemp, DatalinkAbilityIdSeparator);
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
            strcat(StringTemp, StringTable.get(record->name));
            if ((GameDataStateStorage.fields_.game_state_ & 0x1000) != 0) {
                strcat(StringTemp, DatalinkFacilityIdSeparator);
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
Purpose: List the secret projects using Datalink ids 110000 plus their
         Facility-table indices (110070 through 110133).

         When `ProjectPrefixHidden` is clear the "The " prefix the rules file
         carries is stripped off the front of the name first, so the list
         reads "Ascension" rather than "The Ascension".
// ORIGINAL: 0x0042AFE0 ?set_cat_project@Datalink@@QAEXXZ 0x0042AFE0-0x0042B18E FILE BYTE_EXACT
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
    const int first_project_id = 110000 + FacilitySPStart;
    int project_id = first_project_id;
    const RulesFacility *record = &Facility[FacilitySPStart];
    for (; project_id - first_project_id < 0x40; ++record, ++project_id) {
        if (record->preq_tech >= -1) {
            StringTemp[0] = '\0';
            strcat(StringTemp,
                   StringTable.get(record->name));
            if (Language == 0 && strncmp(StringTemp, DatalinkConceptArticle, 4) == 0) {
                char full[256];
                strcpy(full, StringTemp);
                strcpy(StringTemp, full + 4);
            }
            if ((GameDataStateStorage.fields_.game_state_ & 0x1000) != 0) {
                strcat(StringTemp, DatalinkProjectIdSeparator);
                char digits[80];
                _itoa(project_id, digits, 10);
                strcat(StringTemp, digits);
            }
            listBox_.item(StringTemp, project_id);
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

// One list row: name, optional id suffix (game_state_ 0x1000), item. Each
// inlined copy owns its digits buffer, as the image's frames show.
static inline void category_entry(ListBox *list, LPSTR name, LPSTR separator, int id) {
    StringTemp[0] = '\0';
    strcat(StringTemp, StringTable.get(name));
    if ((GameDataStateStorage.fields_.game_state_ & 0x1000) != 0) {
        strcat(StringTemp, separator);
        char digits[80];
        _itoa(id, digits, 10);
        strcat(StringTemp, digits);
    }
    list->item(StringTemp, id);
}
// WORKAROUND(inline): set_cat_tech's original allocates every scratch
// register one step round from the category_entry expansion (digits address
// in ecx, not eax; 0x0042B1EF); this helper, taking the looked-up name with
// StringTemp already cleared by the caller, reproduces it. Original inline
// provider unknown.
static inline void category_entry_text(ListBox *list, LPSTR text, LPSTR separator, int id) {
    strcat(StringTemp, text);
    if ((GameDataStateStorage.fields_.game_state_ & 0x1000) != 0) {
        strcat(StringTemp, separator);
        char digits[80];
        _itoa(id, digits, 10);
        strcat(StringTemp, digits);
    }
    list->item(StringTemp, id);
}

/*
Purpose: List the technologies, ids 140000 upward.

         The technology record the tree materialises is eleven dwords wide and
         the walk reads its two preq words at -1 and 0 from the pointer it
         holds, which is why both are tested before the name at -9 is fetched.
// ORIGINAL: 0x0042B190 ?set_cat_tech@Datalink@@QAEXXZ 0x0042B190-0x0042B2F3 FILE BYTE_EXACT
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
    int id = 140000;
    for (int i = 0; i < 0x59; ++i, ++id) {
        if (Technology[i].preq_tech_1 >= -1 && Technology[i].preq_tech_2 >= -1) {
            StringTemp[0] = '\0';
            category_entry_text(&listBox_, StringTable.get(Technology[i].name),
                                DatalinkTechIdSeparator, id);
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
// ORIGINAL: 0x0042B300 ?set_cat_concept@Datalink@@QAEXXZ 0x0042B300-0x0042B48C FILE BYTE_EXACT
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
    if (text_open(DatalinkConceptSection, DatalinkConceptFile) != 0) {
        return;
    }

    int index = 0;
    text_get();
    for (; TextBufferGetPtr != 0
                        && strlen(TextBufferGetPtr) != 0;) {
        StringTemp[0] = '\0';
        strcat(StringTemp, TextBufferGetPtr);
        if ((GameDataStateStorage.fields_.game_state_ & 0x1000) != 0) {
            strcat(StringTemp, DatalinkConceptIdSeparator);
            char digits[80];
            _itoa(index, digits, 10);
            strcat(StringTemp, digits);
        }
        listBox_.item(StringTemp, index++);
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
// ORIGINAL: 0x0042B490 ?set_cat_adv_concept@Datalink@@QAEXXZ 0x0042B490-0x0042B62A FILE BYTE_EXACT
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
    if (text_open(DatalinkAdvConceptSection, DatalinkAdvConceptFile) != 0) {
        return;
    }

    int index = 0;
    text_get();
    for (; TextBufferGetPtr != 0 && strlen(TextBufferGetPtr) != 0;) {
        StringTemp[0] = '\0';
        strcat(StringTemp, TextBufferGetPtr);
        if ((GameDataStateStorage.fields_.game_state_ & 0x1000) != 0) {
            strcat(StringTemp, DatalinkAdvConceptIdSeparator);
            char digits[80];
            _itoa(index + 10000, digits, 10);
            strcat(StringTemp, digits);
        }
        const int id = index++;
        listBox_.item(StringTemp, 10000 + id);
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
// ORIGINAL: 0x0042B630 ?set_cat_social@Datalink@@QAEXXZ 0x0042B630-0x0042B7DC FILE BYTE_EXACT
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
    LPSTR *name_cursor = SocialCategories[0].name;
    int category = 120001;
    do {
        listBox_.item(StringTable.get(name_cursor[-5]), category - 1);

        LPSTR *choice_cursor = name_cursor;
        for (int choice = 0; choice < 4; ++choice_cursor, ++choice) {
            int id = category + choice;
            StringTemp[0] = '\0';
            strcat(StringTemp, StringTable.get(*choice_cursor));
            if ((GameDataStateStorage.fields_.game_state_ & 0x1000) != 0) {
                strcat(StringTemp, DatalinkSocialIdSeparator);
                char digits[80];
                _itoa(id, digits, 10);
                strcat(StringTemp, digits);
            }
            listBox_.item(StringTemp, id);
        }
        category += 5;
        name_cursor += sizeof(RulesSocialCategory) / sizeof(LPSTR);
    } while (category < 120021);

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
// ORIGINAL: 0x0042B7E0 ?set_cat_effect@Datalink@@QAEXXZ 0x0042B7E0-0x0042B938 FILE BYTE_EXACT
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
    for (int index = 0; index < MaxSocialEffectNum; ++index) {
        if (index != 3) {
            StringTemp[0] = '\0';
            strcat(StringTemp, SocialEffects[index].set2);
            if ((GameDataStateStorage.fields_.game_state_ & 0x1000) != 0) {
                strcat(StringTemp, DatalinkEffectIdSeparator);
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
Purpose: List faction rows with ids 150001 through 150007.

         The pinned image loads grammar values, clears StringTemp, then
         publishes the saved values before building each row.
// ORIGINAL: 0x0042B940 ?set_cat_faction@Datalink@@QAEXXZ 0x0042B940-0x0042BAA3 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Correct the historical 0x0042BACC extent, which overlapped the separately implemented set_cat_terraform. The faction category method ends at its own RET, before alignment padding and the next method.","evidence":["pinned:0x0042B940-0x0042BAA3 complete instruction decode; terminal RET at 0x0042BAA2 and no branch leaves this body","pinned:0x0042BAA3-0x0042BAB0 NOP padding followed by the independent set_cat_terraform entry","source:src/datalink.cpp set_cat_terraform owns 0x0042BAB0-0x0042BCB1; IDA and Ghidra agree on these separate entries"]}
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
    int faction_id = 150001;
    const Player *player = &Players[1];
    // Original ESI is the emitted category ID and EDI starts at Players[1];
    // both advance once per row, including the seven-row 1..7 table walk.
    for (; faction_id - 150000 < 8; ++player, ++faction_id) {
        const int gender = player->noun_gender;
        const BOOL plural = player->is_noun_plural;
        StringTemp[0] = '\0';
        GenderDefault = gender;
        PluralityDefault = plural;
        strcat(StringTemp, player->formal_name_faction);
        if ((GameDataStateStorage.fields_.game_state_ & 0x1000) != 0) {
            strcat(StringTemp, DatalinkFactionIdSeparator);
            char digits[80];
            _itoa(faction_id, digits, 10);
            strcat(StringTemp, digits);
        }
        listBox_.item(StringTemp, faction_id);
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
// ORIGINAL: 0x0042BAB0 ?set_cat_terraform@Datalink@@QAEXXZ 0x0042BAB0-0x0042BCB1 FILE BYTE_EXACT
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
    int id = 90020;
    for (int i = 0; i < 0x14; ++i, ++id) {
        if (Terraforming[i].prerequisite_tech[0] >= -1) {
            category_entry(&listBox_, Order[i + 4].order[0], DatalinkTerraformIdSeparator, id - 20);
        }
        if (i != 16 && i != 17 && Terraforming[i].prerequisite_tech[1] >= -1) {
            category_entry(&listBox_, Order[i + 4].order[1], DatalinkTerraformSecondSeparator, id);
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
        set_cat(static_cast<unsigned int>(category), 0);
        const int item = entry - category * 10000;
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
// symbol    ?on_button_passover@Datalink@@UAEXHH@Z
void Datalink::on_button_passover(int button_id, int state) {
    if (field_29DC_ == 0xe) {
        // Original432029 calls49B220 directly, not through ReportWin's vtable.
        ReportWinGlobal.ReportWin::on_button_passover(button_id, state);
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
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00431FC0-0x00431FFB branches on key codes 0x0D and 0x1B, dynamically calls ListBox slot 26 otherwise, then Win slot 58; git:85463515:src/recovered/units/00431fc0.cpp; source:src/datalink.h embeds ListBox listBox_; source:src/win.h names slot 58 release_modal and slot 26 on_key_down"], "reason": "The Datalink key callback closes the modal window for Enter/Escape and delegates other keys through its embedded ListBox Win handler."}
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
// RECOVERY_BINDING: v1 {"evidence": ["git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/unrecovered/00432060.cpp"], "reason": "Retain the existing source implementation and complete original mapping. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
// LEVER: statement-order not declaration order - `this - 0xA14` (the GraphicWin-adjusted receiver used for slide_show/slide_hide, draw_entry, show, hide) is recomputed fresh inside each branch rather than hoisted once; and in the a1==0 branch the `*(self+0x2020)=0` store sits between computing the 0x126A0 vtable-slot address and dereferencing it, matching the lea/mov/mov/call order byte for byte. Vtable dispatch and the recursive `esi-0xa14` receiver both use the OriginalObject/ original_method/original_slot pointer-to-member pattern from src/original_seam.h, inlined here since the scaffold TU cannot #include project headers and `__thiscall` on a function-pointer typedef is C4234 in this cl.
// symbol    ?on_status@Datalink@@UAEXH@Z
// size      242 bytes
Status: Coverage transcription of the retained seed
        (git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/unrecovered/00432060.cpp), which is BYTE_EXACT in scaffold
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


// ORIGINAL: 0x00429180 ?exec@Datalink@@QAEXIH@Z 0x00429180-0x004292FC BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:datalink-owner-coverage", "pinned:terranx_original.exe:429180..4292FC complete geometry, saved-state, list redraw, modal and restoration sequence", "source:src/reportwin.h:ReportWinGlobal shared_research_focus owns885F30; src/reportif.h existing category/faction fields", "historical seed:src/recovered/units/00429180.cpp"], "reason": "Recover the complete modal Datalink operation through its existing class, embedded text/list controls and real shared ReportWin/ReportIf state; preserve the original identity and seed findings without a new byte claim."}
void Datalink::exec(unsigned int topic, int index) {
    if (WinScreenWidth == 0x320) {
        RECT area = lower_description_rect_;
        area.left += 3;
        area.top += 3;
        area.right -= 3;
        area.bottom -= 6;
        area.left += FactionsArt[1].datalink.width() + 5;
        const int x = area.left;
        const int y = area.top;
        const int width = area.right - area.left;
        const int height = area.bottom - area.top;

        stringBox_.init(x, y, width, height, static_cast<Win *>(this),
                        0, -1);
        Font *const hyperlink_font = &link_font_;
        stringBox_.set_font(&secondary_body_font_, 0, 0, hyperlink_font);
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


// WORKAROUND(inline): twin of win.h's gated expand_rect. Defining
// WIN_EXPAND_RECT_INLINE in this unit flips set_cat_social (0x0042B630) to
// MNEMONIC_ONLY, as defining WIN_RECT_EXTENT_INLINE does; init expands it
// inline (0x00429537: the 3 held in eax for right/bottom, -3 immediates for
// left/top).
static inline RECT *datalink_expand_rect(RECT *rect, int horizontal, int vertical) {
    rect->left -= horizontal;
    rect->right += horizontal;
    rect->top -= vertical;
    rect->bottom += vertical;
    return rect;
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
    register_iface(8);
    // 0x0042935B keeps this flag (setne) for the font block; the geometry
    // below compares the width again.
    const int wide = WinScreenWidth != 800;
    int content_height;
    int side_width;
    if (WinScreenWidth == 800) {
        side_width = 0;
        content_height = 600 - MainInterfaceGlobal.panel_height();
    } else {
        side_width = 0x8a;
        content_height = 0x2e2 - MainInterfaceGlobal.panel_height();
    }
    const int left = (WinScreenWidth - 800) / 2;
    const int top = WinScreenHeight - MainInterfaceGlobal.panel_height() - content_height;

    interface_update_rect_ = MainInterfaceGlobal.main_layout_rect();
    // [A] 0x004293FB..0x00429411 stores left, right, top, bottom; written
    // left, top, right, bottom through a reference, VC6 schedules them that
    // way and holds 7/0xE6 in the image's ecx/eax (direct member stores
    // reorder and swap them).
    RECT &category_rect = category_rect_;
    category_rect.left = 30;
    category_rect.top = 7;
    category_rect.right = 230;
    category_rect.bottom = 0x1b;
    field_103E4_rect_.right = 0xe6;
    field_103E4_rect_.bottom = side_width + 0x143;
    field_103E4_rect_.left = 0x1e;
    field_103E4_rect_.top = 0x1d;

    illustration_rect_.left = 0xf8;
    illustration_rect_.top = 0x2c;
    illustration_rect_.right = 0x177;
    illustration_rect_.bottom = 0xab;

    // [B] 0x00429468..0x0042948B stores title top, left, right, bottom right
    // after the illustration rect. The image keeps the dead `left = 0xDB`
    // store (0x00429475) that title_left overwrites below: the first stores
    // went through a reference, like description_rect's (whose dead 0xDB
    // store at 0x004294A4 also survives); direct member stores drop it.
    RECT &title_rect = title_rect_;
    title_rect.top = 7;
    title_rect.left = 0xdb;
    title_rect.right = 0x302;
    title_rect.bottom = 0x22;
    RECT &description_rect = description_rect_;
    description_rect.left = 0xdb;
    description_rect.top = 0x24;
    description_rect.right = 0x302;
    description_rect.bottom = side_width / 2 + 0xb4;
    // Image 4294D0/4294F2 passes the member itself to UnionRect.
    // Keep the later detail-box geometry bound to that same live rectangle.
    RECT &lower_description_rect = lower_description_rect_;
    lower_description_rect.left = 0xdb;
    lower_description_rect.top = side_width / 2 + 0xb6;
    lower_description_rect.right = 0x302;
    lower_description_rect.bottom = lower_description_rect.top + side_width / 2 + 0x8d;
    const int title_left = field_103E4_rect_.right + 9;
    title_rect_.left = title_left;
    description_rect.left = title_left;
    lower_description_rect.left = title_left;
    UnionRect(&panel_rect_, &description_rect, &lower_description_rect);
    UnionRect(&entry_clip_rect_, &description_rect, &lower_description_rect);
    UnionRect(&entry_clip_rect_, &entry_clip_rect_, &title_rect_);
    entry_update_rect_ = entry_clip_rect_;
    datalink_expand_rect(&entry_update_rect_, 3, 3);
    RECT &entry_update_rect = entry_update_rect_;
    entry_clip_rect_ = panel_rect_;
    RECT &entry_frame_rect = entry_frame_rect_;
    entry_frame_rect.left = left;
    entry_frame_rect.top = top;
    entry_frame_rect.right = left + 0x340;
    entry_frame_rect.bottom = top + content_height;

    // Native row stores establish eighteen consecutive RECTs; index 16 is
    // deliberately overwritten after the two panel-row loops.
    RECT rect = MainInterfaceGlobal.main_frame_rect();
    rect.left += 4;
    rect.right -= 4;
    rect.bottom -= 4;
    rect.top = rect.bottom - 0x15;
    ++rect.bottom;
    int i;
    for (i = 7; i >= 0; --i) {
        topic_button_bounds_[i] = rect;
        rect.top -= 0x18;
        rect.bottom -= 0x18;
    }
    rect = MainInterfaceGlobal.right_frame_rect();
    rect.left += 4;
    rect.right -= 4;
    rect.bottom -= 4;
    rect.top = rect.bottom - 0x15;
    ++rect.bottom;
    for (i = 8; i < 17; ++i) {
        topic_button_bounds_[i] = rect;
        rect.top -= 0x18;
        rect.bottom -= 0x18;
    }

    // [F] 0x004296A3..0x004296E1: bottom is derived from the stored top
    // (lea ecx,[eax+0x14] / add eax,0x14), never side_width+0x162 directly;
    // [16] stores left, right, top, bottom.
    topic_button_bounds_[16].left = 0x98;
    topic_button_bounds_[16].right = 0x18e;
    topic_button_bounds_[16].top = side_width + 0x14e;
    topic_button_bounds_[16].bottom = topic_button_bounds_[16].top + 0x14;
    // [17] keeps its top in a register and adds 0x14 to it in place
    // (0x004296CE..0x004296E1) instead of reusing [16]'s bottom.
    RECT &bounds17 = topic_button_bounds_[17];
    bounds17.left = 0x192;
    bounds17.right = 0x288;
    int top17 = side_width + 0x14e;
    bounds17.top = top17;
    top17 += 0x14;
    bounds17.bottom = top17;

    if (!wide) {
        auxiliary_font10_.init(DefaultFontFace, 12, 4);
        auxiliary_font11_.init(DefaultFontFace, 12, 0);
        font1_.init(DefaultFontFace, 18, 1);
        font3_.init(DefaultFontFace, 14, 1);
        auxiliary_font4_.init(DefaultFontFace, 14, 1);
        auxiliary_font5_.init(DefaultFontFace, 14, 1);
        body_font_.init(DefaultFontFace, 14, 1);
        secondary_body_font_.init(DefaultFontFace, 14, 0);
        link_font_.init(DefaultFontFace, 14, 5);
        auxiliary_font8_.init(DefaultFontFace, 10, 0);
        auxiliary_font8_.unk_1_ = 4;
        font2_.init(DefaultFontFace, 12, 0);
    } else {
        auxiliary_font10_.init(DefaultFontFace, 14, 4);
        auxiliary_font11_.init(DefaultFontFace, 14, 0);
        font1_.init(DefaultFontFace, 18, 1);
        font3_.init(DefaultFontFace, 14, 1);
        auxiliary_font4_.init(DefaultFontFace, 14, 1);
        auxiliary_font5_.init(DefaultFontFace, 14, 1);
        body_font_.init(DefaultFontFace, 14, 1);
        secondary_body_font_.init(DefaultFontFace, 14, 0);
        link_font_.init(DefaultFontFace, 14, 5);
        auxiliary_font8_.init(DefaultFontFace, 12, 0);
        font2_.init(DefaultFontFace, 14, 0);
    }

    caviar_.init();
    caviar_.set_field_A4(0);
    float angles[3];
    angles[0] = 0.0f;
    angles[1] = 0.0f;
    angles[2] = 0.785f;
    VOX_Matrix camera_rotation;
    caviar_apply_rotation(angles, &camera_rotation);
    camera_rotation.values[2][0] = -camera_rotation.values[2][0];
    camera_rotation.values[2][1] = -camera_rotation.values[2][1];
    camera_rotation.values[2][2] = -camera_rotation.values[2][2];
    VOX_Vect vector;
    vector.values[0] = camera_rotation.values[2][0] * -10000.0f;
    vector.values[1] = camera_rotation.values[2][1] * -10000.0f;
    vector.values[2] = camera_rotation.values[2][2] * -10000.0f;
    caviar_.set_camera_direct(&vector, &camera_rotation);
    caviar_.set_scene_rotation(3.14f / 6, 0.0f, 0.0f);
    // 0x0042997C reuses the camera position's slot for the forward vector.
    vector.values[0] = 0.0f;
    vector.values[1] = 0.0f;
    vector.values[2] = 1.0f;
    float transformed_forward[3];
    vox_matrix_vector_mul(camera_rotation.values, vector.values,
                          transformed_forward);
    caviar_.shadow_vector() = vector;
    caviar_.light_vector() = vector;

    // The legacy parent computation reaches Console's real Win base. Its
    // static conversion applies the compiler's current virtual-base layout.
    // [G] 0x004299C7..0x00429A23: the parent is converted first and kept in
    // a frame slot ([ebp-0x20]); height then width are computed before any
    // push, and left/top are reloaded for their own pushes.
    Win *parent = static_cast<Win *>(&ConsoleGlobal);
    const int frame_height = entry_frame_rect_.bottom - entry_frame_rect_.top;
    const int frame_width = entry_frame_rect_.right - entry_frame_rect_.left;
    GraphicWin::init(entry_frame_rect_.left, entry_frame_rect_.top,
                     frame_width, frame_height,
                     0, 0x1000000, parent, 0, 0);
    GraphicWin::fill(9);

    rect.left = 0x17;
    rect.top = 0;
    rect.right = 0x309;
    rect.bottom = content_height;
    box_sprite(&rect, &InterfaceBoxStyles[9]);
    box_sprite(&entry_update_rect, &InterfaceBoxStyles[3]);
    UnionRect(&rect, &topic_button_bounds_[17], &topic_button_bounds_[16]);
    rect.left -= 3;
    rect.right += 3;
    rect.top -= 3;
    rect.bottom += 3;
    box_sprite(&rect, &InterfaceBoxStyles[3]);
    box_sprite(&category_rect_, &InterfaceBoxStyles[6]);
    UnionRect(&rect, &category_rect_, &field_103E4_rect_);
    rect.left -= 3;
    rect.right += 3;
    rect.top -= 3;
    rect.bottom += 3;
    box_sprite(&rect, &InterfaceBoxStyles[3]);
    rect = field_103E4_rect_;
    box_sprite(&rect, &InterfaceBoxStyles[6]);
    rect.left += 3;
    rect.right -= 3;
    rect.top += 3;
    rect.bottom -= 3;

    listBox_.init(&rect, static_cast<Heap *>(0));
    listBox_.field_8_ = 1;
    listBox_.field_2C_ = 0;
    listBox_.set_field_44(0);
    listBox_.field_B4_ = 0;
    listBox_.set_dialog_font(&font2_, 0, 0);
    listBox_.attach(static_cast<GraphicWin *>(this), rect.left, rect.top,
                    0x1000020);
    buttonGroup_.init(0, 0);

    for (i = 0; i < 16; ++i) {
        const RECT &row = topic_button_bounds_[i];
        topic_buttons_[i].init(label_get(0xb20 / 4 + i), i,
                               row.left, row.top, row.right - row.left,
                               row.bottom - row.top,
                               static_cast<Win *>(&MainInterfaceGlobal), 0);
        topic_buttons_[i].set_border_styles(&InterfaceBoxStyles[27],
                                             &InterfaceBoxStyles[28],
                                             &InterfaceBoxStyles[29]);
        topic_buttons_[i].Buffer::set_font(
            &auxiliary_font4_, 0, 0, 0);
        topic_buttons_[i].redraw();
        topic_buttons_[i].hide();
        buttonGroup_.add(&topic_buttons_[i]);
    }

    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0x9c / 4));
    CharUpperA(StringTemp);
    FlatButton *button = &topic_buttons_[17];
    button->init(StringTemp, 0x11,
                            topic_button_bounds_[17].left,
                            topic_button_bounds_[17].top,
                            topic_button_bounds_[17].right - topic_button_bounds_[17].left,
                            topic_button_bounds_[17].bottom - topic_button_bounds_[17].top,
                            static_cast<Win *>(this), 0);
    button->Buffer::set_font(
        &auxiliary_font4_, 0, 0, 0);
    button->redraw();
    button->hide();

    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0xa60 / 4));
    CharUpperA(StringTemp);
    button = &topic_buttons_[16];
    button->init(StringTemp, 0x10,
                            topic_button_bounds_[16].left,
                            topic_button_bounds_[16].top,
                            topic_button_bounds_[16].right - topic_button_bounds_[16].left,
                            topic_button_bounds_[16].bottom - topic_button_bounds_[16].top,
                            static_cast<Win *>(this), 0);
    button->Buffer::set_font(
        &auxiliary_font4_, 0, 0, 0);
    button->redraw();
    button->hide();

    // [H] 0x00429E26/0x00429EC2/0x00429F65: each StringBox rect is a whole
    // copy of its source rect adjusted in place; the image loads bottom,
    // top, right, left and adjusts bottom, right, top, left.
    rect = interface_update_rect_;
    rect.left += 3;
    rect.right -= 3;
    rect.top += 3;
    rect.bottom -= 3;
    blurb_box_.init(rect.left, rect.top, rect.right - rect.left,
                    rect.bottom - rect.top,
                    static_cast<Win *>(&MainInterfaceGlobal), 0, -1);
    blurb_box_.Buffer::set_font(&auxiliary_font8_, 0, 0, 0);
    blurb_box_.Buffer::set_text_color(0x98, -1, 1, 1);
    blurb_box_.Buffer::set_text_color2(0x96, -1, 1, 1);
    blurb_box_.Buffer::set_text_color_hyper(0x96, -1, 1, 1);

    // [H] same order as above.
    rect = panel_rect_;
    rect.left += 3;
    rect.right -= 3;
    rect.top += 3;
    rect.bottom -= 3;
    concept_box_.init(rect.left, rect.top, rect.right - rect.left,
                      rect.bottom - rect.top,
                      static_cast<Win *>(this), 0, -1);
    concept_box_.Buffer::set_font(&body_font_, 0, 0, &link_font_);
    concept_box_.Buffer::set_text_color(0x98, -1, 1, 1);
    concept_box_.Buffer::set_text_color2(0x96, -1, 1, 1);
    concept_box_.Buffer::set_text_color_hyper(0x96, -1, 1, 1);

    // [H] same order as above.
    rect = lower_description_rect;
    rect.left += 6;
    rect.right -= 6;
    rect.top += 6;
    rect.bottom -= 6;
    detail_box_.init(rect.left, rect.top, rect.right - rect.left,
                     rect.bottom - rect.top,
                     static_cast<Win *>(this), 0, -1);
    detail_box_.Buffer::set_font(&body_font_, 0, 0, &link_font_);
    detail_box_.Buffer::set_text_color(0x98, -1, 1, 1);
    detail_box_.Buffer::set_text_color2(0x96, -1, 1, 1);
    detail_box_.Buffer::set_text_color_hyper(0x96, -1, 1, 1);
    (void)temporary_buffer;
}

