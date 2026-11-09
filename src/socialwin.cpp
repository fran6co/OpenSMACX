extern const char BufferJoinSpace[];
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
// ORIGINAL: 0x004B3F50 ??_GSocialWin@@UAEPAXI@Z 0x004B3F50-0x004B3F71 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical deleting-thunk source src/deleting_thunks.cpp; original full spans retained: 0x004B3F50-0x004B3F71", "Pinned class constructor ??0SocialWin@@QAE@XZ at 0x004AE9E0 stores original class/view table VA 0x66daa4 at 0x004AEBB5 (mov dword ptr [esi], 0x66daa4); the current genuine owner vftable ??_7SocialWin@@6BWin@@@ selects generated ??_GSocialWin@@UAEPAXI@Z in slot 0 (+0x0) through weak entry ??_ESocialWin@@UAEPAXI@Z (mode 2).", "Pinned ??_GSocialWin@@UAEPAXI@Z at 0x004B3F50 calls the source-mapped ??1SocialWin@@QAE@XZ at 0x004B3C80, then tests byte ptr [ebp + 8], 1; je 0x4b3f6a bypasses scalar delete when bit 0 is clear, while bit 0 set calls 0x0064557F; it save original this; call destructor; delete only when flags bit 0 is set; return saved this with ret 4.", "The real class lifetime is owned by src/socialwin.cpp; its compiler-emitted deleting symbol and vftable slot carry this original identity. Hand-written adapter bodies with live adjustor callers remain until those callers are independently qualified and transferred. The linked weak/COMDAT copy is not used to choose source ownership.", "the source writer preserves the historical mapping's exactness state (historical exactness obligation retained); current compiler extent is not asserted equal to the original span"], "reason": "Bind the original SocialWin scalar deleting-destructor identity to the actual compiler-emitted class deleting destructor in its genuine owner translation unit. The original constructor's pinned vptr store and the current owner's physical table slot identify this class/view; no callback/thunk body is hand-authored here."}
// symbol    ??_GSocialWin@@UAEPAXI@Z
// kind      game
// ORIGINAL: 0x004AE9B0 _$E21 0x004AE9B0-0x004AE9C6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x6825f0 selects initializer0x4ae9b0..0x4ae9c6, registering exit0x4ae9d0..0x4ae9da. Root independent complete decode confirms exclusive RET/JMP bounds; original/source-owned storage is0x8a6270 bound to?TutWinSocWindow@@3VSocialWin@@A.", "Original startup instruction evidence: [[\"0x4ae9b0\", \"mov\", \"ecx, 0x8a6270\"], [\"0x4ae9b5\", \"call\", \"0x4ae9e0\"], [\"0x4ae9ba\", \"push\", \"0x4ae9d0\"], [\"0x4ae9bf\", \"call\", \"0x645398\"], [\"0x4ae9c4\", \"pop\", \"ecx\"], [\"0x4ae9c5\", \"ret\", \"\"]]. Original exit instruction evidence: [[\"0x4ae9d0\", \"mov\", \"ecx, 0x8a6270\"], [\"0x4ae9d5\", \"jmp\", \"0x4b3c80\"]]. Preserve constructor arguments, array counts/strides, registration and reverse destruction without authored callback stand-ins.", "Fresh complete build-iveupb5l physical src/socialwin.cpp COFF selects _$E21 via .CRT$XCU, registering _$E19. Startup relocation targets: [\"?TutWinSocWindow@@3VSocialWin@@A\", \"??0SocialWin@@QAE@XZ\", \"_$E19\", \"_atexit\"]; exit targets: [\"?TutWinSocWindow@@3VSocialWin@@A\", \"??1SocialWin@@UAE@XZ\"]. Actual linked helper occurs once at CRT slot0x63f2d4; selected same-owner order agrees with the pinned order.", "BaseWin/DesignWin definitions now precede their Wave globals as pinned. Existing DesignWin Wave mappings retain original names/spans and current helper overrides after fresh emission proof. Scalar constructor effects and array construction/cleanup belong to genuine source globals, classes and compiler-generated lifetime machinery.", "Current local labels are emission interpretations, not original debug names or recovered historical TU boundaries. Start bindings grant no allocation extent. Selected same-owner linked order is not full CRT/global-order, retained-selection, reference-target, dependency-provider, runtime or full-game acceptance. Guarded callbacks excluded; no claim grant or bug fix."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned ?TutWinSocWindow@@3VSocialWin@@A; preserve complete original identity and remaining restoration/structure obligations."}
// symbol    _$E16
// ORIGINAL: 0x004AE9D0 _$E19 0x004AE9D0-0x004AE9DA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x6825f0 selects initializer0x4ae9b0..0x4ae9c6, registering exit0x4ae9d0..0x4ae9da. Root independent complete decode confirms exclusive RET/JMP bounds; original/source-owned storage is0x8a6270 bound to?TutWinSocWindow@@3VSocialWin@@A.", "Original startup instruction evidence: [[\"0x4ae9b0\", \"mov\", \"ecx, 0x8a6270\"], [\"0x4ae9b5\", \"call\", \"0x4ae9e0\"], [\"0x4ae9ba\", \"push\", \"0x4ae9d0\"], [\"0x4ae9bf\", \"call\", \"0x645398\"], [\"0x4ae9c4\", \"pop\", \"ecx\"], [\"0x4ae9c5\", \"ret\", \"\"]]. Original exit instruction evidence: [[\"0x4ae9d0\", \"mov\", \"ecx, 0x8a6270\"], [\"0x4ae9d5\", \"jmp\", \"0x4b3c80\"]]. Preserve constructor arguments, array counts/strides, registration and reverse destruction without authored callback stand-ins.", "Fresh complete build-iveupb5l physical src/socialwin.cpp COFF selects _$E21 via .CRT$XCU, registering _$E19. Startup relocation targets: [\"?TutWinSocWindow@@3VSocialWin@@A\", \"??0SocialWin@@QAE@XZ\", \"_$E19\", \"_atexit\"]; exit targets: [\"?TutWinSocWindow@@3VSocialWin@@A\", \"??1SocialWin@@UAE@XZ\"]. Actual linked helper occurs once at CRT slot0x63f2d4; selected same-owner order agrees with the pinned order.", "BaseWin/DesignWin definitions now precede their Wave globals as pinned. Existing DesignWin Wave mappings retain original names/spans and current helper overrides after fresh emission proof. Scalar constructor effects and array construction/cleanup belong to genuine source globals, classes and compiler-generated lifetime machinery.", "Current local labels are emission interpretations, not original debug names or recovered historical TU boundaries. Start bindings grant no allocation extent. Selected same-owner linked order is not full CRT/global-order, retained-selection, reference-target, dependency-provider, runtime or full-game acceptance. Guarded callbacks excluded; no claim grant or bug fix."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned ?TutWinSocWindow@@3VSocialWin@@A; preserve complete original identity and remaining restoration/structure obligations."}
// symbol    _$E14
#include "stdafx.h"
#include "font_metrics.h"
#include "socialwin.h"
#include "win.h"
#include "subinterface.h"
#include "game.h"
#include "strings.h"
#include "text.h"
#include "faction.h"
#include "base.h"
#include "technology.h"
#include "lock.h"
#include "alpha.h"
#include "mapwin.h"   // MainInterfaceGlobal, at 0x007AE820
#include "maininterface.h"
#include "tutwin.h"
#include "console.h"
#include "interface_fonts.h"
#include "netdaemon.h"
#include "popup.h"

extern int BreakthroughTurnsGlobal;
extern int NetIncomeGlobal;
extern uint32_t Language;

// These writable arrays are the source-owned versions of fourteen distinct
// SocialWin text inputs in the pinned image. Identical spellings remain
// separate objects where the original has separate addresses.
// RECOVERY_OBJECT: v2 0x0068749C 0x006874A0 ?SocialWinOverviewSocText@@3PADA 0x004B0410
char SocialWinOverviewSocText[] = "SOC";           // 0x0068749C
// RECOVERY_OBJECT: v2 0x006874A0 0x006874A2 ?SocialWinOverviewPlusText@@3PADA 0x004B0410
char SocialWinOverviewPlusText[] = "+";            // 0x006874A0
// RECOVERY_OBJECT: v2 0x006874A4 0x006874A6 ?SocialWinMouseoverMinusText@@3PADA 0x004B1080
char SocialWinMouseoverMinusText[] = "-";          // 0x006874A4
// RECOVERY_OBJECT: v2 0x006874A8 0x006874AA ?SocialWinMouseoverPlusText@@3PADA 0x004B1080
char SocialWinMouseoverPlusText[] = "+";           // 0x006874A8
// RECOVERY_OBJECT: v2 0x006874AC 0x006874AE ?SocialWinButtonCountPlusText@@3PADA 0x004B17A0
char SocialWinButtonCountPlusText[] = "+";         // 0x006874AC
// RECOVERY_OBJECT: v2 0x006874B0 0x006874BB ?SocialWinPcxName@@3PADA 0x004B2740
char SocialWinPcxName[] = "socwin.pcx";            // 0x006874B0
// RECOVERY_OBJECT: v2 0x006874BC 0x006874C3 ?SocialWinTutorialName@@3PADA 0x004B2740
char SocialWinTutorialName[] = "SOCWIN";           // 0x006874BC
// RECOVERY_OBJECT: v2 0x00682E90 0x00682E93 ?SocialWinCommaSpaceText@@3PADA 0x004AFB60
char SocialWinCommaSpaceText[] = ", ";             // 0x00682E90
// RECOVERY_OBJECT: v2 0x00682E94 0x00682E97 ?SocialWinColonSpaceText@@3PADA 0x004AFB60
char SocialWinColonSpaceText[] = ": ";             // 0x00682E94
// RECOVERY_OBJECT: v2 0x00687480 0x00687483 ?SocialWinPercentSpaceTextA@@3PADA 0x004AFB60
char SocialWinPercentSpaceTextA[] = "% ";           // 0x00687480
// RECOVERY_OBJECT: v2 0x00687484 0x00687487 ?SocialWinPercentSpaceTextB@@3PADA 0x004AFB60
char SocialWinPercentSpaceTextB[] = "% ";           // 0x00687484
// RECOVERY_OBJECT: v2 0x00687488 0x0068748B ?SocialWinPercentSpaceTextC@@3PADA 0x004AFB60
char SocialWinPercentSpaceTextC[] = "% ";           // 0x00687488
// RECOVERY_OBJECT: v2 0x0068748C 0x0068748F ?SocialWinPercentSpaceTextD@@3PADA 0x004AFB60
char SocialWinPercentSpaceTextD[] = "% ";           // 0x0068748C
// RECOVERY_OBJECT: v2 0x00687490 0x00687492 ?SocialWinUpheavalPlusText@@3PADA 0x004AFB60
char SocialWinUpheavalPlusText[] = "+";             // 0x00687490

// ORIGINAL: 0x004AEF10 ?exit@SocialWin@@QAEXXZ 0x004AEF10-0x004AF10C BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/recovered/units/004aef10.cpp", "image:terranx_original.exe:0x004AEF10", "Original instructions establish owner, bounded body and callback/state order; SocialWin reloads faction after social_upheaval and every later stateful call. GraphicWin dispatches through the real Buffer base.", "recovery/tasks/interface-contract-report-closure.json; bounded UI-five scope preserves original baseline"], "reason": "Recover the original in its genuine UI owner using actual members, globals and base/virtual calls. Retain seed evidence and historical obligations; this binding does not grant a current matching or runtime claim. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
void SocialWin::exit() {
    int upheaval = social_upheaval(factionID_, &socCategory_)
        - PlayersData[factionID_].soc_upheaval_cost_paid;

    if ((GameDataStateStorage.fields_.game_state_ & STATE_SCENARIO_EDITOR) == 0
        || (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW) == 0) {
        if (upheaval > 0) {
            parse_num(0, upheaval);
            parse_num(1, PlayersData[factionID_].energy_reserves);
            if (upheaval > PlayersData[factionID_].energy_reserves) {
                popp(ScriptPopupFile, "UPHEAVALTOOMUCH", 0, "genwarning_sm.pcx",
                     pop_wait);
                return;
            }
            if ((Players[factionID_].rule_flags & RFLAG_ALIEN) != 0) {
                if (popp(ScriptPopupFile, "UPHEAVAL", 0, "Alopdir.pcx", pop_wait) == 0) {
                    return;
                }
            } else {
                if (popp(ScriptPopupFile, "UPHEAVAL", 0, "talent_sm.pcx", pop_wait) == 0) {
                    return;
                }
            }
        }
    } else {
        upheaval = 0;
    }

    if (upheaval >= PlayersData[factionID_].energy_reserves) {
        upheaval = PlayersData[factionID_].energy_reserves;
    }
    PlayersData[factionID_].soc_upheaval_cost_paid += upheaval;
    for (int i = 0; i < 4; ++i) {
        PlayersData[factionID_].soc_category_pending.models_[i] = socCategory_.models_[i];
    }
    synch_alloc(factionID_);
    synch_soc(factionID_);
    net_energy(factionID_, -upheaval, 0, 0, 0);
    social_calc(&PlayersData[factionID_].soc_category_pending,
                &PlayersData[factionID_].soc_effect_pending, factionID_, 0, 0);
    release_modal();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004B1E80 ?on_key_click@SocialWin@@QAEHHH@Z 0x004B1E80-0x004B1EB5 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/004b1e80.cpp", "image:terranx_original.exe:0x004B1E80", "Original instructions establish owner, bounded body and callback/state order; SocialWin reloads faction after social_upheaval and every later stateful call. GraphicWin dispatches through the real Buffer base.", "recovery/tasks/interface-contract-report-closure.json; bounded UI-five scope preserves original baseline"], "reason": "Recover the original in its genuine UI owner using actual members, globals and base/virtual calls. Retain seed evidence and historical obligations; this binding does not grant a current matching or runtime claim."}
// symbol    ?on_key_click@SocialWin@@UAEHHH@Z
int SocialWin::on_key_click(int, int key) {
    TutorialWindow.reset();
    switch (key) {
        case 27:
            reset();
            // Native Escape handling resets, then shares the Enter exit path.
        case 13:
            exit();
            break;
    }
    return 1;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// RECOVERY_ASSOCIATED: v1 switch_targets 0x004B1E00 0x004B1E4C 0x004B1E5C 0x004B1E23
// RECOVERY_ASSOCIATED: v1 switch_selector 0x004B1E00 0x004B1E5C 0x004B1E7B 0x004B1E1D
// ORIGINAL: 0x004B1E00 ?on_button_clicked@SocialWin@@QAEXH@Z 0x004B1E00-0x004B1E49
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004b1e00.cpp", "image:terranx_original.exe:0x004B1E00", "Original instructions establish owner, bounded body and callback/state order; SocialWin reloads faction after social_upheaval and every later stateful call. GraphicWin dispatches through the real Buffer base.", "recovery/tasks/interface-contract-report-closure.json; bounded UI-five scope preserves original baseline"], "reason": "Recover the original in its genuine UI owner using actual members, globals and base/virtual calls. Retain seed evidence and historical obligations; this binding does not grant a current matching or runtime claim."}
// symbol    ?on_button_clicked@SocialWin@@UAEXH@Z
void SocialWin::on_button_clicked(int button_id) {
    TutorialWindow.reset();
    switch (button_id) {
        case -2:
        case 16:
            reset();
            // Native reset-and-exit entries share the following exit path.
        case -1:
        case 28:
            exit();
            break;
        case 27:
            reset();
            break;
    }
}

/*
Purpose: Install the SubInterface vftable, construct the Spot, nine runs of
         further sub-objects (a CheckButton[3], four Sprite runs, a
         Buffer[6], a FlatButton[35], a ButtonGroup[4] and one more
         ButtonGroup, nine Fonts), then install SocialWin's own vftables and
         set factionID_ to -1.
// TRIED (2026-08-25): declaring the five member ARRAYS the constructor was
// hand-constructing. The arithmetic proves what they are - the body called
// `VectorCtorIterator(self + 0x2C60, 0x2C, 0xA, ...)` and `sizeof(Sprite)`
// is 0x2C, so 0x2C * 0xA == 0x1B8 is exactly the `uint8_t[0x1B8]` that sat
// at 0x2C60; likewise 0x5A and 2 and 6 Sprites, and 6 Buffers at 0x588.
// The `const void *` ctor/dtor constants were Sprite's and Buffer's own
// constructors and destructors, which C++ cannot spell as function
// references at all - the same tell that made FontQueue's array obvious.
// SCORE WENT DOWN, 20 of 121 to 16 of 121, and the fix is KEPT anyway:
// agreement is not monotonic in correctness, the declarations are proved
// by arithmetic rather than guessed, and no claim was at stake (this body
// has never been BYTE_EXACT). VC6 emits its generated iterator calls in a
// different ORDER than the image, which is the next thing to chase here -
// most likely because the image's class declares members this one still
// does not.
// ORIGINAL: 0x004AE9E0 ??0SocialWin@@QAE@XZ 0x004AE9E0-0x004AEBE7;0x006598A0-0x00659A0A BYTE_EXACT
// TRIED: deriving from ConstructedGraphicWin (graphicwin.h) to move the
//   base construction ahead of the members. The ORDER IS RIGHT - this image
//   really does `call 0x5d4cf0` before its first `lea ecx, [esi + N]`, checked
//   2026-08-22 - but making the source agree took this body 16 -> 16
//   of 121. Same shape as Scroll (scroll.cpp), where the reorder alone was
//   worth 1 instruction and the payoff came from two levers underneath that it
//   made reachable; here those levers are not the vtable-immediate one, since
//   these constants already fold, and this body is structurally far off
//   (133 compiled against 121). Re-derive the ordering fix as part
//   of a dedicated pass on the whole body, not on its own.
// TRIED: register allocation - the SEH prologue agrees (7/7) then the
//            compiled body reserves an extra `sub esp, 0xc` and an extra
//            callee-save (ebx) the image does not. MISMATCH, 15/121
//            instructions agree.
// size      881 bytes
// prototype void (__thiscall ??0SocialWin@@QAE@XZ)(SocialWin* this)
// callers   1   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005D4CF0 0x005FA860 0x00618EA0 0x0062B7C0 0x006456E4 0x006457C2
*/
// Bases precede members in the native construction sequence. Their ordinary
// C++ construction must not be repeated with placement-new in this body.
// The CheckButton array boundary is supported by both lifecycle entries;
// remaining provisional field names do not establish complete layout recovery.
SocialWin::SocialWin() {
    // Declared object members construct automatically in declaration order.
    // POD social state is subsequently initialized by reset.
    factionID_ = 0xFFFFFFFFU;
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x004AEF00 ?UNK2@SocialWin@@QAEXXZ 0x004AEF00-0x004AEF01 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK2@SocialWin@@QAEXXZ)(SocialWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void SocialWin::UNK2() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x004B1790 ?UNK3@SocialWin@@QAEXXZ 0x004B1790-0x004B1791 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK3@SocialWin@@QAEXXZ)(SocialWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void SocialWin::UNK3() {
}

// Original interface slot 3 ignores its coordinates and dismisses the
// tutorial. Its seed's int result is incidental EAX from reset; interface
// callers consume no result. Installing the full interface contract remains
// part of the shared SubInterface hierarchy recovery.
// ORIGINAL: 0x004B2210 ?on_iface_left_click@SocialWin@@QAEHHH@Z 0x004B2210-0x004B221D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original4B2210 loads8C6E68 and calls4BDDE0, then returns with8bytes callee cleanup. Neither coordinate is read. Table66DA50 slot3 identifies the secondary-interface callback; the current incomplete SubInterface contract still needs coordinated repair.", "The catalogued int return forwards incidental EAX from reset; interface slot3 callers do not consume it and the family contract is void(int,int). Preserve the seed identity and original exact obligation while publishing the compiler-observed source symbol."], "reason": "Home the tutorial-dismissal interface handler on SocialWin using the real process-lifetime TutorialWindow and recovered reset service."}
// symbol    ?on_iface_left_click@SocialWin@@UAEXHH@Z
void SocialWin::on_iface_left_click(int, int) {
    TutorialWindow.reset();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004B2220 ?on_iface_right_click@SocialWin@@QAEXHH@Z 0x004B2220-0x004B23FA BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original4B2220..4B23FA first resets TutorialWindow then tests interface-relative1E84C/1E85C/1E86C/1E83C, which are complete-object rectangles1F260/1F270/1F280/1F250. Labels.strings_ptr offsetsD24/D28/D2C/D1C select labels841/842/843/839; first match wins.", "Original4B2256/59 swaps argument stack contents before passing their reversed slot addresses to MainInterfaceGlobal.client_to_screen; this preserves logical x/y rather than the swapped coordinates in the seed. Every matching path constructs a degenerate screen rectangle and calls set_bubble_text on7AE820; no match only dismisses the tutorial."], "reason": "Recover social-panel tooltip selection with genuine rectangle members, label storage and the main-interface window receiver."}
// symbol    ?on_iface_right_click@SocialWin@@UAEXHH@Z
void SocialWin::on_iface_right_click(int x, int y) {
    TutorialWindow.reset();
    const int click_y = y;
    const int click_x = x;
    if (in_box(click_x, click_y, &tooltipEconomy_)) {
        x = click_y;
        y = click_x;
        char *text = StringTable.get(Labels.strings_ptr[841]);
        MainInterfaceGlobal.client_to_screen(&y, &x);
        RECT rectangle = {y, x, y, x};
        MainInterfaceGlobal.Win::set_bubble_text(text, &rectangle);
        return;
    }
    if (in_box(click_x, click_y, &tooltipLabs_)) {
        x = click_y;
        y = click_x;
        char *text = StringTable.get(Labels.strings_ptr[842]);
        MainInterfaceGlobal.client_to_screen(&y, &x);
        RECT rectangle = {y, x, y, x};
        MainInterfaceGlobal.Win::set_bubble_text(text, &rectangle);
        return;
    }
    if (in_box(click_x, click_y, &tooltipPsych_)) {
        x = click_y;
        y = click_x;
        char *text = StringTable.get(Labels.strings_ptr[843]);
        MainInterfaceGlobal.client_to_screen(&y, &x);
        RECT rectangle = {y, x, y, x};
        MainInterfaceGlobal.Win::set_bubble_text(text, &rectangle);
        return;
    }
    if (in_box(click_x, click_y, &tooltipEffects_)) {
        x = click_y;
        y = click_x;
        char *text = StringTable.get(Labels.strings_ptr[839]);
        MainInterfaceGlobal.client_to_screen(&y, &x);
        RECT rectangle = {y, x, y, x};
        MainInterfaceGlobal.Win::set_bubble_text(text, &rectangle);
    }
}

// ORIGINAL: 0x004B1B20 ?on_left_click@SocialWin@@QAEHHH@Z 0x004B1B20-0x004B1B2D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["The 0x004B1B20 body ignores its coordinates and returns TutWin::reset at 0x004BDDE0 on the process-lifetime object at 0x008C6E68.", "src/tutwin.cpp owns that object as TutorialWindow and its reset service returns int."], "reason": "Home the SocialWin click dismissal through the typed tutorial singleton."}
// symbol    ?on_left_click@SocialWin@@UAEXHH@Z
void SocialWin::on_left_click(int, int) {
    TutorialWindow.reset();
}

// ORIGINAL: 0x004B1BC0 ?on_right_click@SocialWin@@QAEXHH@Z 0x004B1BC0-0x004B1CD1 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["The 0x004B1BC0 body resets TutorialWindow, probes four contiguous category rectangles beginning at +0x1F110 then the faction-power rectangle at +0x1F2A0, and retrieves label slots 828 or 840.", "The existing SocialWin layout has those rectangles and the on_iface_right_click body establishes label_get, client_to_screen and set_bubble_text as the typed tooltip path."], "reason": "Recover the SocialWin right-click tooltip through real rectangle members and source-owned labels."}
// symbol    ?on_right_click@SocialWin@@UAEXHH@Z
void SocialWin::on_right_click(int x, int y) {
    TutorialWindow.reset();
    RECT *category = categoryHeaders_;
    for (int i = 0; i < 4; ++i, ++category) {
        if (in_box(x, y, category)) {
            bubble_me_elmo2(StringTable.get(Labels.strings_ptr[828]), x, y);
            return;
        }
    }
    if (in_box(x, y, &tooltipFactionPower_)) {
        bubble_me_elmo2(StringTable.get(Labels.strings_ptr[840]), x, y);
    }
}

// ORIGINAL: 0x004B1FC0 ?on_mouse_leave@SocialWin@@QAEXHH@Z 0x004B1FC0-0x004B1FEC FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["The 0x004B1FC0 body calls Win::clear_bubble_text twice, then checks the signed hover value at +0xA2C and redraws mouseover only while it is nonnegative.", "The sibling on_iface_mouse_leave already proves the double clear is intentional, and draw_mouseover uses hover_selection_ as its hover selection."], "reason": "Home the mouse-leave path through the existing hover member and typed Win service."}
// symbol    ?on_mouse_leave@SocialWin@@UAEXHH@Z
void SocialWin::on_mouse_leave(int, int) {
    ::Win::clear_bubble_text();
    ::Win::clear_bubble_text();
    if (hover_selection_ >= 0) {
        hover_selection_ = -1;
        draw_mouseover();
    }
}

// ORIGINAL: 0x004B26D0 ?close@SocialWin@@QAEXXZ 0x004B26D0-0x004B273C FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["The 0x004B26D0 body closes 35 FlatButton objects from +0x6020 at sizeof(FlatButton) stride, then four ButtonGroup objects from +0x1EB84 and a final group at +0x1EDD4.", "The existing SocialWin layout declares those exact arrays and the image finishes by clearing +0x2C28 and setting hover/effect selectors +0xA2C,+0xA30,+0x2C24 to minus one."], "reason": "Recover SocialWin cleanup through its real button arrays and named state members."}
void SocialWin::close() {
    for (int i = 0; i < 35; ++i) {
        flatButtons_[i].close();
    }
    for (int j = 0; j < 4; ++j) {
        buttonGroups_[j].close();
    }
    buttonGroup_.close();
    field_2C28_ = 0;
    hover_selection_ = -1;
    effect_hover_button_id_ = -1;
    effectMaximize_ = -1;
}



/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004B2400 ?on_iface_left_down@SocialWin@@QAEXHH@Z 0x004B2400-0x004B2403 BYTE_EXACT
// symbol    ?on_iface_left_down@SocialWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_down@SocialWin@@QAEXHH@Z)(SocialWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void SocialWin::on_iface_left_down(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004B2410 ?on_iface_right_down@SocialWin@@QAEXHH@Z 0x004B2410-0x004B2413 BYTE_EXACT
// symbol    ?on_iface_right_down@SocialWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_down@SocialWin@@QAEXHH@Z)(SocialWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void SocialWin::on_iface_right_down(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004B2440 ?on_iface_selected@SocialWin@@QAEXHH@Z 0x004B2440-0x004B2443 BYTE_EXACT
// symbol    ?on_iface_selected@SocialWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_selected@SocialWin@@QAEXHH@Z)(SocialWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void SocialWin::on_iface_selected(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004B2450 ?on_iface_left_double_click@SocialWin@@QAEXHH@Z 0x004B2450-0x004B2453 BYTE_EXACT
// symbol    ?on_iface_left_double_click@SocialWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_double_click@SocialWin@@QAEXHH@Z)(SocialWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void SocialWin::on_iface_left_double_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004B2460 ?on_iface_right_double_click@SocialWin@@QAEXHH@Z 0x004B2460-0x004B2463 BYTE_EXACT
// symbol    ?on_iface_right_double_click@SocialWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_double_click@SocialWin@@QAEXHH@Z)(SocialWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void SocialWin::on_iface_right_double_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004B2670 ?on_iface_button_toggled@SocialWin@@QAEXHH@Z 0x004B2670-0x004B2673 BYTE_EXACT
// symbol    ?on_iface_button_toggled@SocialWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_button_toggled@SocialWin@@QAEXHH@Z)(SocialWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void SocialWin::on_iface_button_toggled(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004B2680 ?on_iface_group_clicked@SocialWin@@QAEXHHH@Z 0x004B2680-0x004B2683 BYTE_EXACT
// symbol    ?on_iface_group_clicked@SocialWin@@UAEXHHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_group_clicked@SocialWin@@QAEXHHH@Z)(SocialWin* this, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void SocialWin::on_iface_group_clicked(int, int, int) {
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004B2470 ?on_iface_button_clicked@SocialWin@@QAEXH@Z 0x004B2470-0x004B264E FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004b2470.cpp retained original identity and spans", "recovery/tasks/interface-contract-report-closure.json authorized ownership recovery", "original:0x004B2470 complete control/member accesses and concrete interface table"], "reason": "Recover the concrete SubInterface owner operation through genuine window members and source-owned globals. Original table/call evidence identifies the callback receiver; the retained compatible seed supplies its complete original intervals, not an executable-address forwarding implementation."}
// symbol    ?on_iface_button_clicked@SocialWin@@UAEXH@Z
// RECOVERY_ASSOCIATED: v1 switch_targets 0x004B2470 0x004B2650 0x004B2668 0x004B2490
void SocialWin::on_iface_button_clicked(int button_id) {
    TutorialWindow.reset();

    int index;
    int delta;
    switch (button_id) {
    case 0x1D: index = 0; delta = -1; break;
    case 0x1E: index = 0; delta = 1; break;
    case 0x1F: index = 2; delta = -1; break;
    case 0x20: index = 2; delta = 1; break;
    case 0x21: index = 1; delta = -1; break;
    case 0x22: index = 1; delta = 1; break;
    default: return;
    }

    currentAlloc_[index] += delta;
    if (static_cast<int>(currentAlloc_[index]) < 0) currentAlloc_[index] = 0;
    const int limit = energy_limit(factionID_);
    if (static_cast<int>(currentAlloc_[index]) > limit) {
        field_2C5C_ = 1;
        currentAlloc_[index] = static_cast<uint32_t>(energy_limit(factionID_));
    }

    allocate_energy(index, static_cast<int>(currentAlloc_[index]));

    for (int allocation = 0; allocation < 3; ++allocation) {
        currentAlloc_[allocation] = pendingAlloc_[allocation];
    }

    PlayersData[factionID_].se_alloc_psych = static_cast<int>(pendingAlloc_[1]);
    PlayersData[factionID_].se_alloc_labs = static_cast<int>(pendingAlloc_[2]);
    social_set(factionID_);

    social_calc(&socCategory_, &socEffect_, factionID_, 0, 0);
    social_calc(&socCategory_, &PlayersData[factionID_].soc_effect_pending,
                factionID_, 0, 0);
    energy_compute(factionID_, 1);

    breakthroughTurns_ = BreakthroughTurnsGlobal;
    netIncome_ = NetIncomeGlobal;
    social_calc(&PlayersData[factionID_].soc_category_pending,
                &PlayersData[factionID_].soc_effect_pending, factionID_, 0, 0);
    redraw();
}


/*
Purpose: Dismiss the bubble text when the pointer leaves. The whole body is two
         consecutive calls to the static Win::clear_bubble_text and nothing
         else; the arguments are ignored.

         The SECOND call is not a transcription error. The original issues it,
         and the two are observationally identical unless something re-arms the
         bubble between them - which is why the fixture re-arms it rather than
         asserting a single refresh and calling that agreement.
// ORIGINAL: 0x004B2420 ?on_iface_mouse_move@SocialWin@@QAEXHH@Z 0x004B2420-0x004B242D BYTE_EXACT
// symbol    ?on_iface_mouse_move@SocialWin@@UAEXHH@Z
// size      13 bytes
// prototype void (__thiscall ?on_iface_mouse_move@SocialWin@@QAEXHH@Z)(SocialWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F8500
Return Value: n/a
Status: Complete
*/
void SocialWin::on_iface_mouse_move(int, int) {
    ::Win::clear_bubble_text();
    ::Win::clear_bubble_text();
}


/*
Purpose: Dismiss the bubble text when the pointer leaves. The whole body is two
         consecutive calls to the static Win::clear_bubble_text and nothing
         else; the arguments are ignored.

         The SECOND call is not a transcription error. The original issues it,
         and the two are observationally identical unless something re-arms the
         bubble between them - which is why the fixture re-arms it rather than
         asserting a single refresh and calling that agreement.
// ORIGINAL: 0x004B2430 ?on_iface_mouse_leave@SocialWin@@QAEXHH@Z 0x004B2430-0x004B243D BYTE_EXACT
// symbol    ?on_iface_mouse_leave@SocialWin@@UAEXHH@Z
// size      13 bytes
// prototype void (__thiscall ?on_iface_mouse_leave@SocialWin@@QAEXHH@Z)(SocialWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F8500
Return Value: n/a
Status: Complete
*/
void SocialWin::on_iface_mouse_leave(int, int) {
    ::Win::clear_bubble_text();
    ::Win::clear_bubble_text();
}


/*
// ORIGINAL: 0x004B2690 ?show@SocialWin@@QAEXH@Z 0x004B2690-0x004B26AB BYTE_EXACT
// symbol    ?show@SocialWin@@UAEXH@Z
// size      27 bytes
// prototype void (__thiscall ?show@SocialWin@@QAEXH@Z)(SocialWin* this, int)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045D310 0x005F7E90
Status: Complete
*/
void SocialWin::show(int visible) {
    if (!is_visible()) {
        SubInterface::set_iface_mode();
    }
}

/*
// ORIGINAL: 0x004B26B0 ?hide@SocialWin@@QAEXXZ 0x004B26B0-0x004B26C9 BYTE_EXACT
// symbol    ?hide@SocialWin@@UAEXXZ
// size      25 bytes
// prototype void (__thiscall ?hide@SocialWin@@QAEXXZ)(SocialWin* this)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045D380 0x005F7E90
Status: Complete
*/
void SocialWin::hide() {
    if (is_visible()) {
        SubInterface::release_iface_mode();
    }
}

/*
Purpose: Step the receiver back to the subobject ??_GSocialWin@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x004B3F90 ??_GSocialWin@@WEEE@AEPAXI@Z 0x004B3F90-0x004B3F9B BYTE_EXACT
// symbol    ??_ESocialWin@@WEEE@AEPAXI@Z
// CORRECTED from ??3SocialWin@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x004B3F50` into
//   ??_GSocialWin@@UAEPAXI@Z, which executes `ret 4`; no stack access
//   and the receiver stays in ECX. `WEEE@` re-demangles to
//   adjustor{1092} and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/

// The process-wide social window, 0x008A6270 in the image - REAL STORAGE, not
// the `Win *const TutWinSocWindow` binding that named terranx.exe data (see
// tutwin.h). The image's dynamic initializer at 0x004AE9B0 runs the real
// SocialWin constructor against it and registers the deleting destructor
// with atexit; here the same recovered constructor runs through this
// build's own startup and the destructor closes it at exit.
// RECOVERY_OBJECT: v1 0x008A6270 ?TutWinSocWindow@@3VSocialWin@@A
SocialWin TutWinSocWindow;  // 0x008A6270

// ORIGINAL: 0x004B1B30 ?bubble_me_elmo2@SocialWin@@QAEXPADHH@Z 0x004B1B30-0x004B1B70 BYTE_EXACT
// size      64 bytes
// prototype void (__thiscall ?bubble_me_elmo2@SocialWin@@QAEXPADHH@Z)(SocialWin* this, LPCSTR, int, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005ED240 0x005F83D0
// 0x004B1B30  ?bubble_me_elmo2@SocialWin@@QAEXPADHH@Z  ->  ?bubble_me_elmo2@SocialWin@@QAEXPADHH@Z
// LEVER: homed from 004b1b30.cpp. Idiomatic base calls (client_to_screen and
// set_bubble_text are Win's, at SocialWin's own front) and the four RECT
// stores spelled as one initialiser replaced two casts of `this`.
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void SocialWin::bubble_me_elmo2(char *text, int x, int y) {
    client_to_screen(&x, &y);
    RECT rect = { x, y, x, y };
    set_bubble_text(text, &rect);
}

// ORIGINAL: 0x004B1B70 ?bubble_me_elmo2@SocialWin@@QAGXPADHH@Z 0x004B1B70-0x004B1BB2 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:datalink-interface-callback-owner", "Pinned4B1B70..4B1BB2 is a three-argument stdcall callback using explicit MainInterfaceGlobal7AE820 and its primary Win base. Historical SocialWin label remains canonical identity but does not imply a receiver. Converts coordinates before passing a point RECT to Win5F83D0.", "historical seed:src/recovered/004b1b70.cpp"], "reason": "Recover typed UI helper in its genuine source owner, retaining original identity, spans and historical obligations; remove superseded fixed-address seed."}
// symbol    ?main_interface_bubble_callback@@YGXPADHH@Z
void __stdcall main_interface_bubble_callback(char *text, int x, int y) {
    MainInterfaceGlobal.client_to_screen(&x, &y);
    RECT rectangle;
    rectangle.left = x;
    rectangle.top = y;
    rectangle.right = x;
    rectangle.bottom = y;
    MainInterfaceGlobal.Win::set_bubble_text(text, &rectangle);
}

/*
 * Fixed-slot bindings the homed bodies arrived carrying, named from evidence.
 * Each is an `int *const` at file scope, which folds to the bare immediate, so
 * a rename moves no byte; the cast to the real type stays at the point of use.
 *
 * Upper-case operations use the normal CharUpperA import. Original
 * 0x0066931C is its loader-owned IAT slot, not a private function table.
 * Panel styles now borrow shared mutable InterfaceBoxStyles elements 3/6;
 * their original starts 0x78D5A0/0x78D618 are no longer address aliases.
 * MainInterfaceGlobal is the actual owner of this shared canvas Buffer base;
 * its restore_clip() restores that Buffer's own rect2_ clip rectangle.
 * SpaceText          0x00682820 - image .data reads " " (one space). technology
 *                    .cpp already names it in prose for the same reason.
 * PercentText        0x00682EA0 - image .data reads "%".
 * SocialEffectTable  0x0094B024 - SocialCategories[0].model_effect[0]; the
 *                    53-dword category stride and 11-dword model stride are
 *                    the declared RulesSocialCategory / SocialEffect layout.
 *                    A negative entry marks an unavailable effect.
 * SocialEffectRow    0x0094B2CC - SocialCategories[3].model_effect[1].values_.
 *                    Native draw_social walks eleven entries through
 *                    0x0094B2F8, skipping effect index 3.
 * SocBaseIdProbe / SocBaseIdCompare - 0x009A6568 / 0x009A6574. Both are an int
 *                    base id read the same way: `>= 0`, then
 *                    Bases[id].faction_id_current compared to
 *                    the faction, i.e. "does this base belong to us". immune()
 *                    uses network_backbone and cloning_vats project locations.
 * SocialAllocTable   0x0096CD04 - PlayersData[0].se_alloc_psych; the following
 *                    dword is se_alloc_labs, with the native 0x20CC PlayerData
 *                    stride (2099 dwords).
 */
// Historical SocBaseIdProbe/Compare at 0x009A6568/0x009A6574 are
// SecretProject.network_backbone/cloning_vats, not independent selectors.

static inline RECT *social_expand_rect(RECT *rect, int horizontal, int vertical) {
    rect->left -= horizontal;
    rect->right += horizontal;
    rect->top -= vertical;
    rect->bottom += vertical;
    return rect;
}

static inline void social_draw_icon(Sprite *sprite, Buffer *dest, int x, int y) {
    sprite->draw(dest, sprite->transparent_index(), x, y, 1, 1);
}

// Text writes with the image's null test on the text.
static inline void social_write_cent(char *text, RECT *rect) {
    if (text != 0) {
        MainInterfaceGlobal.Buffer::write_cent_l(text, rect, strlen(text));
    }
}
static inline void social_write_l(char *text, RECT *rect) {
    if (text != 0) {
        MainInterfaceGlobal.Buffer::write_l(text, rect, strlen(text));
    }
}

static inline void social_write_l_at(char *text, int x, int y) {
    if (text != 0) {
        MainInterfaceGlobal.Buffer::write_l(text, x, y, strlen(text));
    }
}

// Operand-twice magnitude, as the image re-derives it at each use.
#define SOCIAL_ABS(x) ((x) < 0 ? -(x) : (x))
// The same magnitude with the opposite branch polarity (draw_info, and
// draw_social's zero test, where the image threads it per switch case).
#define SOCIAL_MAGNITUDE(x) ((x) >= 0 ? (x) : -(x))
// ORIGINAL: 0x004AF310 ?draw_social@SocialWin@@QAEXH@Z 0x004AF310-0x004AF8BC FILE BYTE_EXACT
// TRIED: transcribed directly from the disassembly (Ghidra's own decompilation agreed on the control flow and formulas once its confusing overlapping-stack-slot local numbering was resolved against the raw ebp-relative offsets) - the header/derived RECT pair, the per-row avail==0 header-only path, the count-then- draw two-pass loop over the fixed 11-entry table at 0x94b2cc (index 3 excluded), and the `this!=0 ? obj : 0` guard before each Sprite::draw. Draw receivers are the big/small Sprite sub-objects themselves (`bigIcon->draw(buffer, ...)`), not the Buffer - an early draft called the wrong receiver and was fixed before landing. Byte-exact once the zero test used SOCIAL_MAGNITUDE (the image's shared neg block has the `x >= 0 ? x : -x` polarity) and the icon index multiplied effect by 9 inside each arm (the image keeps a separate effect*9 induction variable).
// working copy - scaffold materialised by --work
// size      1452 bytes
// prototype void (__thiscall ?draw_social@SocialWin@@QAEXH@Z)(SocialWin* this, int socCategory)
// callers   2   call targets   12
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005B4730 0x005D5930 0x005D8000 0x005DAC70 0x005DACB0 0x005DD130 0x005E2DD3 0x005E4B4A 0x005FD2B0 0x006169A0 0x006453E0 0x00645470
// indirect  0x004AF409


static int social_effect_value(const SocialEffect &effect, int index);

void SocialWin::draw_social(int a1) {
    RECT *header = &categoryHeaders_[a1];
    RECT rect = *header;
    Buffer::set_clip(&rect);
    Buffer::box_sprite(&rect, &InterfaceBoxStyles[3]);
    social_expand_rect(&rect, -3, -3);
    RECT title = rect;
    title.bottom = title.top + 20;
    Buffer::box_sprite(&title, &InterfaceBoxStyles[6]);

    StringTemp[0] = 0;
    char *name = SocialCategories[a1].type;
    strcat(StringTemp, StringTable.get(name));
    Buffer::set_text_color(0x9f, -1, 1, 1);
    Buffer::set_font(&font1_, 0, 0, 0);
    CharUpperA(StringTemp);
    char *heading = StringTemp;
    if (heading != 0) {
        Buffer::write_cent_l(heading, &title, strlen(heading));
    }

    for (int row = 0; row < 4; ++row) {
        if (society_avail(a1, row, factionID_) == 0) {
            title = modelRects_[a1 * 4 + row];
            Buffer::box_sprite(&title, &InterfaceBoxStyles[6]);
            title.top += 0x16;
            title.bottom += 0x16;
            Buffer::box_sprite(&title, &InterfaceBoxStyles[6]);
            continue;
        }

        rect = modelRects_[a1 * 4 + row];
        rect.top += 0x16;
        rect.bottom += 0x16;
        Buffer::box_sprite(&rect, &InterfaceBoxStyles[6]);

        int icon_count = 0;
        int number_count = 0;
        int effect;
        for (effect = 0; effect < MaxSocialEffectNum; ++effect) {
            if (effect == 3) {
                continue;
            }
            // Cloning Vats and the Network Backbone remove the penalties of
            // the models they neutralise.
            if (has_project(SP_CLONING_VATS, factionID_)
                && ((a1 == 2 && row == 1) || (a1 == 3 && row == 3))
                && SocialCategories[a1].model_effect[row].values_[effect] < 0) {
                continue;
            }
            if (has_project(SP_NETWORK_BACKBONE, factionID_) && a1 == 3 && row == 1
                && SocialCategories[3].model_effect[1].values_[effect] < 0) {
                continue;
            }
            const int value = SocialCategories[a1].model_effect[row].values_[effect];
            icon_count += SOCIAL_ABS(value) != 0;
            number_count += SOCIAL_ABS(value) > 1;
        }
        if (icon_count < 1) {
            icon_count = 1;
        }

        int x = (rect.right - (effectIcons_[0].width() + 1) * icon_count
                 - socEngModifierNums_[0].source_width() * number_count - rect.left) / 2
                + rect.left;
        const int y = (rect.bottom - effectIcons_[0].width() - rect.top) / 2 + rect.top;

        // Larger bonuses first, then +1 effects, then penalties.
        for (int phase = 0; phase < 3; ++phase) {
            for (effect = 0; effect < MaxSocialEffectNum; ++effect) {
                if (effect == 3) {
                    continue;
                }
                if (has_project(SP_CLONING_VATS, factionID_)
                    && ((a1 == 2 && row == 1) || (a1 == 3 && row == 3))
                    && SocialCategories[a1].model_effect[row].values_[effect] < 0) {
                    continue;
                }
                if (has_project(SP_NETWORK_BACKBONE, factionID_) && a1 == 3 && row == 1
                    && SocialCategories[3].model_effect[1].values_[effect] < 0) {
                    continue;
                }
                const int value = SocialCategories[a1].model_effect[row].values_[effect];
                switch (phase) {
                case 0:
                    if (value < 2) {
                        continue;
                    }
                    break;
                case 1:
                    if (value != 1) {
                        continue;
                    }
                    break;
                case 2:
                    if (value >= 0) {
                        continue;
                    }
                    break;
                }
                if (SOCIAL_MAGNITUDE(value) == 0) {
                    continue;
                }
                if (SOCIAL_ABS(value) > 1) {
                    Sprite *number = &socEngModifierNums_[(value <= 0) + SOCIAL_ABS(value) * 2 - 4];
                    social_draw_icon(number, this, x, rect.bottom - number->height());
                    x += socEngModifierNums_[0].source_width() + 1;
                }
                social_draw_icon(&effectIcons_[(effect < 3 ? effect * 9 : (effect - 1) * 9) + 6
                                               + (value >= 1 ? 2 : 1)],
                                 this, x, y);
                x += effectIcons_[0].width() + 1;
            }
        }
    }

    soft_update(header);
    do_sound();
}

// The pinned text at 0x00682EA0 is the percent character and its terminator.
// Its complete allocation extent remains unproved; this names only the text
// consumed by the three allocation renderers.
// RECOVERY_OBJECT: v1 0x00682EA0 _PercentText
static const char PercentText[] = "%";


typedef void (__cdecl *SoundHookFn)(char *);

// ORIGINAL: 0x004B19A0 ?on_redraw@SocialWin@@QAEXXZ 0x004B19A0-0x004B1B19 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:seed:src/recovered/units/004b19a0.cpp; source:src/socialwin.h:tooltipEconomy_,tooltipLabs_,tooltipPsych_,tutRect11_,tutRect12_,tutRect13_,flatButtons_; source:src/graphicwin.h:GraphicWin Buffer base; image:0x004B19A0-0x004B1B19"], "reason": "Recover SocialWin repaint orchestration through its real category/allocation rectangles, inherited GraphicWin Buffer and typed FlatButton child controls; retain the established panel ordering and child redraw sequence."}
// symbol    ?on_redraw@SocialWin@@UAEXXZ
void SocialWin::on_redraw() {
    MainInterfaceGlobal.Buffer::restore_clip();

    Buffer::restore_clip();
    MainInterfaceGlobal.restore_back(1);

    RECT panel;
    UnionRect(&panel, &tooltipLabs_, &tooltipPsych_);
    UnionRect(&panel, &panel, &tooltipEconomy_);
    social_expand_rect(&panel, 3, 3);
    MainInterfaceGlobal.Buffer::box_sprite(&panel, &InterfaceBoxStyles[3]);

    UnionRect(&panel, &tutRect11_, &tutRect13_);
    UnionRect(&panel, &panel, &tutRect12_);
    social_expand_rect(&panel, 3, 3);
    Buffer::box_sprite(&panel, &InterfaceBoxStyles[3]);

    draw_presets();
    for (int category = 0; category < MaxSocialCatNum; ++category) {
        draw_social(category);
    }
    draw_faction_basal();
    draw_info();
    draw_overview();
    draw_economy_alloc();
    draw_labs_alloc();
    draw_psych_alloc();
    draw_mouseover();

    for (int slot = 0; slot < 11; ++slot) {
        if (slot != 3) {
            int button = slot;
            if (slot >= 3) --button;
            flatButtons_[17 + button].redraw();
        }
    }
}

// ORIGINAL: 0x004B0D50 ?draw_psych_alloc@SocialWin@@QAEXXZ 0x004B0D50-0x004B0F92 FILE
// size      578 bytes
// prototype void (__thiscall ?draw_psych_alloc@SocialWin@@QAEXXZ)(SocialWin* this)
// callers   1   call targets   13
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D5930 0x005D8000 0x005DA860 0x005DAC70 0x005DACB0 0x005DD130 0x005E2DD3 0x005E3203 0x005FD2B0 0x006169A0 0x006453E0 0x00645470 0x0064FC88
// indirect  0x004B0E7F
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004B0D50
// measured tier  NO_COMPILE
// refusal        u004b0d50.cpp(212) : error C2079: 'buffer_' uses undefined class 'Buffer' u004b0d50.cpp(343) : error C2079: 'buffer_' uses undefined class 'Buffer'
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004b0d50/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw_psych_alloc@SocialWin@@QAEXXZ  at 0x004B0D50  (578 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw_psych_alloc@SocialWin@@QAEXXZ  at 0x004B0D50  (578 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.


void SocialWin::draw_psych_alloc() {
    RECT local_rect = tooltipPsych_;
    MainInterfaceGlobal.Buffer::restore_clip();
    MainInterfaceGlobal.Buffer::box_sprite(&local_rect, &InterfaceBoxStyles[6]);

    social_expand_rect(&local_rect, -3, -3);

    RECT box_rect = local_rect;
    box_rect.left += 0x17;
    box_rect.right -= 0x17;
    box_rect.bottom = box_rect.top + 0x14;
    MainInterfaceGlobal.Buffer::box(&box_rect, 0xbc, 0xbc);

    const int allocation = PlayersData[factionID_].se_alloc_psych;
    StringTemp[0] = '\0';
    strcat(StringTemp, label_get(318));
    strcat(StringTemp, BufferJoinSpace);
    char number[80];
    _itoa(allocation * 10, number, 10);
    strcat(StringTemp, number);
    strcat(StringTemp, PercentText);
    CharUpperA(StringTemp);

    MainInterfaceGlobal.Buffer::set_font(&font1_, 0, 0, 0);
    MainInterfaceGlobal.Buffer::set_text_color(0x9f, -1, 1, 1);
    char *const text = StringTemp;
    if (text != 0) {
        MainInterfaceGlobal.Buffer::write_cent_l(text, &box_rect, strlen(text));
    }

    local_rect.top = local_rect.bottom - energyAllocLock_[0].height();
    local_rect.left += energyAllocLock_[0].width();
    box_rect = local_rect;
    local_rect.left += (local_rect.right - local_rect.left) * allocation / 10;
    if (local_rect.left < local_rect.right) {
        box_rect.right = local_rect.left + (allocation != 0);
        MainInterfaceGlobal.Buffer::tile(&energyAllocSlider_[4], local_rect.left, local_rect.top,
                     local_rect.left, local_rect.top, local_rect.right - local_rect.left,
                     local_rect.bottom - local_rect.top);
    }
    if (box_rect.left < box_rect.right) {
        MainInterfaceGlobal.Buffer::tile(&energyAllocSlider_[5], box_rect.left, box_rect.top,
                     box_rect.left, box_rect.top, box_rect.right - box_rect.left,
                     box_rect.bottom - box_rect.top);
    }

    MainInterfaceGlobal.soft_update(&tooltipPsych_);
    do_sound();
}

// Draw the unassigned energy share with the two slider buffers owned by the
// SocialWin. PlayerData is the source owner for the psych and lab shares.
// ORIGINAL: 0x004B08A0 ?draw_economy_alloc@SocialWin@@QAEXXZ 0x004B08A0-0x004B0AF2
// RECOVERY_BINDING: v1 {"evidence": ["git:seed:src/recovered/units/004b08a0.cpp; source:src/socialwin.h:tooltipEconomy_,energyAllocSlider_,energyAllocLock_; source:src/faction.h:PlayerData.se_alloc_psych,se_alloc_labs; image:0x004B08A0-0x004B0AF2"], "reason": "Home the economy allocation panel on SocialWin’s typed rectangles, slider buffers, font, and PlayerData allocation values."}
void SocialWin::draw_economy_alloc() {
    RECT panel = tooltipEconomy_;
    MainInterfaceGlobal.Buffer::restore_clip();
    MainInterfaceGlobal.Buffer::box_sprite(&panel, &InterfaceBoxStyles[6]);
    social_expand_rect(&panel, -3, -3);

    RECT title = panel;
    title.left += 0x17;
    title.right -= 0x17;
    title.bottom = title.top + 0x14;
    MainInterfaceGlobal.Buffer::box(&title, 0xbc, 0xbc);

    const int remaining = 10 - PlayersData[factionID_].se_alloc_psych - PlayersData[factionID_].se_alloc_labs;
    StringTemp[0] = '\0';
    strcat(StringTemp, label_get(366));
    strcat(StringTemp, BufferJoinSpace);
    char number[80];
    _itoa(remaining * 10, number, 10);
    strcat(StringTemp, number);
    strcat(StringTemp, PercentText);
    CharUpperA(StringTemp);

    MainInterfaceGlobal.Buffer::set_font(&font1_, 0, 0, 0);
    MainInterfaceGlobal.Buffer::set_text_color(0x9f, -1, 1, 1);
    char *const text = StringTemp;
    if (text != 0) {
        MainInterfaceGlobal.Buffer::write_cent_l(text, &title, strlen(text));
    }

    panel.top = panel.bottom - energyAllocLock_[0].height();
    panel.left += energyAllocLock_[0].width();
    title = panel;
    panel.left += (panel.right - panel.left) * remaining / 10;
    if (panel.left < panel.right) {
        title.right = panel.left + (remaining != 0);
        MainInterfaceGlobal.Buffer::tile(&energyAllocSlider_[0], panel.left, panel.top,
                     panel.left, panel.top, panel.right - panel.left,
                     panel.bottom - panel.top);
    }
    if (title.left < title.right) {
        MainInterfaceGlobal.Buffer::tile(&energyAllocSlider_[1], title.left, title.top,
                     title.left, title.top, title.right - title.left,
                     title.bottom - title.top);
    }

    MainInterfaceGlobal.soft_update(&tooltipEconomy_);
    do_sound();
}

// Draw the labs share with the panel's own buffers and typed faction data.
// ORIGINAL: 0x004B0B00 ?draw_labs_alloc@SocialWin@@QAEXXZ 0x004B0B00-0x004B0D42
// RECOVERY_BINDING: v1 {"evidence": ["git:seed:src/recovered/units/004b0b00.cpp; source:src/socialwin.h:tooltipLabs_,energyAllocSlider_,energyAllocLock_; source:src/faction.h:PlayerData.se_alloc_labs; image:0x004B0B00-0x004B0D42"], "reason": "Home the labs allocation panel on SocialWin’s typed rectangles, slider buffers, font, and PlayerData lab allocation."}
void SocialWin::draw_labs_alloc() {
    RECT panel = tooltipLabs_;
    MainInterfaceGlobal.Buffer::restore_clip();
    MainInterfaceGlobal.Buffer::box_sprite(&panel, &InterfaceBoxStyles[6]);
    social_expand_rect(&panel, -3, -3);

    RECT title = panel;
    title.left += 0x17;
    title.right -= 0x17;
    title.bottom = title.top + 0x14;
    MainInterfaceGlobal.Buffer::box(&title, 0xbc, 0xbc);

    const int allocation = PlayersData[factionID_].se_alloc_labs;
    StringTemp[0] = '\0';
    strcat(StringTemp, label_get(60));
    strcat(StringTemp, BufferJoinSpace);
    char number[80];
    _itoa(allocation * 10, number, 10);
    strcat(StringTemp, number);
    strcat(StringTemp, PercentText);
    CharUpperA(StringTemp);

    MainInterfaceGlobal.Buffer::set_font(&font1_, 0, 0, 0);
    MainInterfaceGlobal.Buffer::set_text_color(0x9f, -1, 1, 1);
    char *const text = StringTemp;
    if (text != 0) {
        MainInterfaceGlobal.Buffer::write_cent_l(text, &title, strlen(text));
    }

    // The title rectangle is reused for the allocated share and the panel
    // rectangle for the remainder.
    panel.top = panel.bottom - energyAllocLock_[0].height();
    panel.left += energyAllocLock_[0].width();
    title = panel;
    panel.left = title.left + (title.right - title.left) * allocation / 10;
    if (panel.left < panel.right) {
        title.right = panel.left + (allocation != 0);
        MainInterfaceGlobal.Buffer::tile(&energyAllocSlider_[2], panel.left, panel.top,
                     panel.left, panel.top, panel.right - panel.left,
                     panel.bottom - panel.top);
    }
    if (title.left < title.right) {
        MainInterfaceGlobal.Buffer::tile(&energyAllocSlider_[3], title.left, title.top,
                     title.left, title.top, title.right - title.left,
                     title.bottom - title.top);
    }

    MainInterfaceGlobal.soft_update(&tooltipLabs_);
    do_sound();
}

// Draws one effect icon; the image converts the destination before indexing.

// Paint the faction power strip from its owning Player name and base effects.
// ORIGINAL: 0x004AF8C0 ?draw_faction_basal@SocialWin@@QAEXXZ 0x004AF8C0-0x004AFB5D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:seed:src/recovered/units/004af8c0.cpp; source:src/socialwin.h:tooltipFactionPower_,effectIcons_,font1_; source:src/faction.h:Player.noun_faction,noun_gender,is_noun_plural,PlayerData.soc_effect_base; image:0x004AF8C0-0x004AFB5D"], "reason": "Recover the faction-power panel on SocialWin typed rectangle, inherited Buffer, effect sprites, and Player records, including the noun-format defaults consumed by faction text rendering."}
void SocialWin::draw_faction_basal() {
    RECT panel = tooltipFactionPower_;
    Buffer::set_clip(&panel);
    Buffer::box_sprite(&panel, &InterfaceBoxStyles[3]);

    social_expand_rect(&panel, -3, -3);
    RECT title = panel;
    title.bottom = title.top + 0x14;
    Buffer::box_sprite(&title, &InterfaceBoxStyles[6]);

    StringTemp[0] = '\0';
    const int faction = factionID_;
    parse_set(Players[faction].noun_gender, Players[faction].is_noun_plural);
    strcat(StringTemp, Players[faction].noun_faction);
    CharUpperA(StringTemp);
    Buffer::set_font(&font1_, 0, 0, 0);
    Buffer::set_text_color(0x9f, -1, 1, 1);
    Buffer::write_cent_l(StringTemp, &title);

    title = panel;
    title.top = title.bottom - 0x14;
    Buffer::box_sprite(&title, &InterfaceBoxStyles[6]);
    panel = title;
    int total = 0;
    for (int effect = 0; effect < MaxSocialEffectNum; ++effect) {
        if (effect != 3) {
            const int value = PlayersData[factionID_].soc_effect_base.values_[effect];
            total += value < 0 ? -value : value;
        }
    }

    if (total != 0) {
        // BUG IN THE ORIGINAL: the total skips effect 3, the drawing loop
        // does not (it reuses row 2 for it).
        const int step = effectIcons_[0].width() + 2;
        int x = (title.right - step * total - title.left) / 2 + title.left;
        const int y = (title.bottom - effectIcons_[0].height() - title.top) / 2 + title.top;
        for (int effect = 0; effect < MaxSocialEffectNum; ++effect) {
            const int value = PlayersData[factionID_].soc_effect_base.values_[effect];
            for (int icon = 0; icon < (value < 0 ? -value : value); ++icon) {
                social_draw_icon(&effectIcons_[6 + (effect < 3 ? effect : effect - 1) * 9 + (value >= 1 ? 2 : 1)],
                                 this, x, y);
                x += step;
            }
        }
    }

    soft_update(&tooltipFactionPower_);
    do_sound();
}

// ORIGINAL: 0x004B1FF0 ?allocate_energy@SocialWin@@QAEXHH@Z 0x004B1FF0-0x004B211C FILE BYTE_EXACT
// size      300 bytes
// prototype void (__thiscall ?allocate_energy@SocialWin@@QAEXHH@Z)(SocialWin* this, int energyAllocID, int energyAllocSet)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00445440
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004B1FF0
// measured tier  MISMATCH
// divergence     1
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004b1ff0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?allocate_energy@SocialWin@@QAEXHH@Z  at 0x004B1FF0  (300 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.


void SocialWin::allocate_energy(int a1, int a2) {
    int limit = energy_limit(static_cast<int>(factionID_));
    pendingAlloc_[a1] = static_cast<uint32_t>(a2);

    while (static_cast<int32_t>(pendingAlloc_[2] + pendingAlloc_[0] + pendingAlloc_[1]) < 10) {
        for (int attempts = 0; attempts < 3; ++attempts) {
            int candidate = (a1 + attempts + 1) % 3;
            if (a1 == candidate ||
                (energyLockButtons_[candidate].checked_ == 0 &&
                 static_cast<int32_t>(pendingAlloc_[candidate]) < limit)) {
                ++pendingAlloc_[candidate];
                break;
            }
        }
    }

    while (static_cast<int32_t>(pendingAlloc_[2] + pendingAlloc_[0] + pendingAlloc_[1]) > 10) {
        for (int attempts = 0; attempts < 3; ++attempts) {
            int candidate = (a1 + attempts + 1) % 3;
            if (a1 == candidate ||
                (energyLockButtons_[candidate].checked_ == 0 &&
                 pendingAlloc_[candidate] != 0)) {
                --pendingAlloc_[candidate];
                break;
            }
        }
    }
}

// The image's presets title is label slot 69 (0x114 / sizeof(LPSTR)) in the
// loaded label table. The former unit scaffold kept that lookup as a raw
// pointer walk; label_get owns the same lookup in the product source.
// ORIGINAL: 0x004AF1E0 ?draw_presets@SocialWin@@QAEXXZ 0x004AF1E0-0x004AF307 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:seed:src/recovered/units/004af1e0.cpp; source:src/socialwin.h:tutRect1_,font1_; source:src/alpha.h:label_get; source:src/strings.h:StringTemp; source:src/game.cpp:InterfaceBoxStyles"], "reason": "Home the presets panel painter on SocialWin with its typed RECT, Buffer base, font and source-owned strings."}
void SocialWin::draw_presets() {
    RECT panel = tutRect1_;
    Buffer::set_clip(&panel);
    Buffer::box_sprite(&panel, &InterfaceBoxStyles[3]);

    panel.left += 3;
    panel.right -= 3;
    panel.top += 3;
    panel.bottom -= 3;

    RECT title = panel;
    title.bottom = title.top + 0x14;
    Buffer::box_sprite(&title, &InterfaceBoxStyles[6]);

    StringTemp[0] = '\0';
    strcat(StringTemp, label_get(69));
    CharUpperA(StringTemp);

    Buffer::set_font(&font1_, 0, 0, 0);
    Buffer::set_text_color(0x9f, -1, 1, 1);
    // The image null-checks the text before measuring it.
    char *const text = StringTemp;
    if (text != 0) {
        Buffer::write_cent_l(text, &title, strlen(text));
    }

    soft_update(&tutRect1_);
    do_sound();
}

// Callees owned outside this TU. slide_show now has its full C++ body in
// popup.cpp. The remaining original-address helpers retain their seed
// findings until complete compiled owners replace them; these are only
// declarations, not additional function definitions.
void __cdecl energy_compute(int faction_id, int mode);   // 0x00445130
void __cdecl slide_show(GraphicWin *window, int mode);   // 0x0045D440
void __cdecl slide_hide(GraphicWin *window, int mode);   // 0x0045D970
void __cdecl do_all_draws();                             // 0x005FCBB0, in temp.cpp

// The report and social screens share this accounting block. The original
// accesses a 512-entry per-base table immediately after the maintenance sum,
// followed by three totals, 70 facility counters, and commerce/net totals.
// RECOVERY_OBJECT: v2 0x007492DC 0x007492E0 ?EnergyMaintenanceTotal@@3HA 0x00445130
int EnergyMaintenanceTotal;
// RECOVERY_OBJECT: v2 0x007492E0 0x00749AE0 ?EnergyBaseMaintenance@@3PAHA 0x00445130
int EnergyBaseMaintenance[MaxBaseNum];
// RECOVERY_OBJECT: v2 0x00749AE0 0x00749AE4 ?EnergyLabsTotal@@3HA 0x00445130
int EnergyLabsTotal;
// RECOVERY_OBJECT: v2 0x00749AE4 0x00749AE8 ?BreakthroughTurnsGlobal@@3HA 0x00445130
int BreakthroughTurnsGlobal;
// RECOVERY_OBJECT: v2 0x00749AE8 0x00749AEC ?EnergyEconomyTotal@@3HA 0x00445130
int EnergyEconomyTotal;
// RECOVERY_OBJECT: v2 0x00749AEC 0x00749C04 ?EnergyFacilityCounts@@3PAHA 0x00445130
int EnergyFacilityCounts[70];
// RECOVERY_OBJECT: v2 0x00749C04 0x00749C08 ?EnergyCommerceDelta@@3HA 0x00445130
int EnergyCommerceDelta;
// RECOVERY_OBJECT: v2 0x00749C0C 0x00749C10 ?NetIncomeGlobal@@3HA 0x00445130
int NetIncomeGlobal;
// Historical EnergyDiscountBaseIndex at 0x009A6578 is the source-owned
// SecretProject.self_aware_colony field (+0x64), written with the other
// project locations. A separate initialized scalar severed that lifetime.

// ORIGINAL: 0x00445130 ?energy_compute@@YAXHH@Z 0x00445130-0x00445433
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/00445130.cpp", "src/base.h", "src/faction.h", "git:3ea0467addad98356ec7ab97d41d8b02fab9debd:src/hypothesis_layouts.h"], "reason": "Recover the shared energy accounting pass through Base and PlayerData fields, owning its report totals and per-base/facility tables instead of calling a fixed executable address."}
// TRIED: translated per-instruction against the disasm (cross-checked against the Ghidra hypothesis for the per-base accumulation, tech-level clamp, and pairwise unit-bonus loop) rather than reusing ghidra's `(int*)param_1` pointer-cast style; the neg/sbb/neg boolean idiom collapsed to a plain `(a&b)!=0` comparison, the cdq/sar halving collapsed to `/2`. MISMATCH #10 mov/xor in the zero-fill prologue; compiles and follows the disasm's three sections (per-base loop, tech-rate energy cost, cross-faction facility bonus loop) in order.
void __cdecl energy_compute(int faction_id, int recompute_bases) {
    memset(EnergyFacilityCounts, 0, sizeof(EnergyFacilityCounts));
    EnergyLabsTotal = 0;
    NetIncomeGlobal = 0;
    EnergyMaintenanceTotal = 0;
    EnergyEconomyTotal = 0;
    EnergyCommerceDelta = 0;

    int discount_carry = 0;
    for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
        EnergyBaseMaintenance[base_id] = 0;
        if (Bases[base_id].faction_id_current != faction_id) continue;
        if (recompute_bases) {
            set_base(base_id);
            base_compute(1);
        }

        int maintenance = 0;
        for (int facility = 0; facility < 70; ++facility) {
            if (has_fac(facility, base_id, 0)) {
                ++EnergyFacilityCounts[facility];
                maintenance += fac_maint(facility, faction_id);
            }
        }
        if (has_project(SP_SELF_AWARE_COLONY, faction_id)) {
            // The image carries the odd half-maintenance remainder across
            // bases; keep its order rather than rounding each base alone.
            if (discount_carry) ++maintenance;
            discount_carry = maintenance & 1;
            maintenance /= 2;
        }

        EnergyBaseMaintenance[base_id] = maintenance;
        EnergyLabsTotal += Bases[base_id].labs_total;
        NetIncomeGlobal += Bases[base_id].economy_total - maintenance;
        EnergyMaintenanceTotal += maintenance;
        EnergyEconomyTotal += Bases[base_id].economy_total;
    }

    const int research_cost = tech_rate(faction_id);
    EnergyLabsTotal = (range(PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::RESEARCH], -5, 5)
        + 10) * EnergyLabsTotal * 10;
    if (EnergyLabsTotal > 0) {
        BreakthroughTurnsGlobal = (EnergyLabsTotal - 1 + research_cost * 100) / EnergyLabsTotal;
    } else {
        BreakthroughTurnsGlobal = 9999;
    }

    for (int other = 1; other < 8; ++other) {
        if (other == faction_id || !is_alive(other) || !is_alive(faction_id) ||
            (PlayersData[faction_id].diplo_treaties[other] & 0x10u)) continue;

        if (PlayersData[faction_id].loan_balance[other] &&
            !PlayersData[other].sanction_turns) {
            const int payment = PlayersData[faction_id].loan_payment[other];
            NetIncomeGlobal -= payment;
            EnergyCommerceDelta -= payment;
        }
        if (PlayersData[other].loan_balance[faction_id] &&
            !PlayersData[faction_id].sanction_turns) {
            const int payment = PlayersData[other].loan_payment[faction_id];
            NetIncomeGlobal += payment;
            EnergyCommerceDelta += payment;
        }
    }
}


static int social_category_value(const SocialCategory &category, int index) {
    switch (index) {
    case 0: return static_cast<int>(category.models_[SocialCategory::POLITICS]);
    case 1: return static_cast<int>(category.models_[SocialCategory::ECONOMICS]);
    case 2: return static_cast<int>(category.models_[SocialCategory::VALUES]);
    default: return static_cast<int>(category.models_[SocialCategory::FUTURE]);
    }
}

static void set_social_category_value(SocialCategory &category, int index,
                                      int value) {
    switch (index) {
    case 0: category.models_[SocialCategory::POLITICS] = value; break;
    case 1: category.models_[SocialCategory::ECONOMICS] = value; break;
    case 2: category.models_[SocialCategory::VALUES] = value; break;
    default: category.models_[SocialCategory::FUTURE] = value; break;
    }
}

static int social_effect_value(const SocialEffect &effect, int index) {
    return effect.values_[index];
}

/*
 * Purpose: Recompute the social model for one category. Stores the chosen
 *          model in the category slot, redraws it, snapshots the working
 *          effect block as the undo copy, recalculates working and pending
 *          effects, recomputes energy, then refreshes the pending player row.
 *          The faction index selects the player rows; both calculation flags
 *          remain zero.
// ORIGINAL: 0x004AEBF0 ?UNK1@SocialWin@@QAEXHH@Z 0x004AEBF0-0x004AECD2 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Recompute the selected faction's pending social model through its existing PlayerData members. Keep each row lookup at the original call site and retain global rereads after energy_compute. The final social_calc call has five arguments, not an omitted stack-residue argument.","evidence":["source:PlayersData begins at 0x0096C9E0 with PlayerData stride 0x20CC; pending category/effect members +0x228/+0x24C resolve to original 0x0096CC08/0x0096CC2C","pinned:0x004AEC65 addresses the pending effect row;0x004AECB6/0x004AECBC address pending effect/category after energy_compute","pinned:third social_calc arguments are pushed at 0x004AEC86,0x004AECA2,0x004AECA9,0x004AECC2 and0x004AECC3; both trailing flags are zero;0x004AECC9 cleans the complete deferred 0x44-byte call-argument area"]}
// size      226 bytes
// prototype void (__thiscall ?UNK1@SocialWin@@QAEXHH@Z)(SocialWin* this, int, int)
// callers   2   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00445130 0x004AF310 0x005B4210
*/
void SocialWin::UNK1(int soc_category, int soc_model) {
    field_A1C_ = 0xFFFFFFFFU;
    socCategory_.models_[soc_category] = soc_model;
    draw_social(soc_category);

    // Snapshot the eleven-word working effect block as the undo copy.
    // Preserve the original scalar copy loop over all eleven effect words.
    for (int effect = 0; effect < 11; ++effect) {
        socEffectOrig_.values_[effect] = socEffect_.values_[effect];
    }

    social_calc(&socCategory_, &socEffect_, factionID_, 0, 0);
    social_calc(&socCategory_, &PlayersData[factionID_].soc_effect_pending,
                factionID_, 0, 0);

    energy_compute(factionID_, 1);

    breakthroughTurns_ = BreakthroughTurnsGlobal;
    netIncome_ = NetIncomeGlobal;

    social_calc(&PlayersData[factionID_].soc_category_pending,
                &PlayersData[factionID_].soc_effect_pending,
                factionID_, 0, 0);
}

/*
 * Purpose: Maximize one effect column: arm effectMaximize_, then for every
 *          category walk the four model rows for the best available score on
 *          that effect, select it in the category's button group and apply it
 *          through UNK1. A trailing redraw leaves the panels in step, and the
 *          arm clears once the clicks that follow consume it.
// ORIGINAL: 0x004AF110 ?maximize_effect@SocialWin@@QAEXH@Z 0x004AF110-0x004AF1D7 FILE BYTE_EXACT
// size      199 bytes
// prototype void (__thiscall ?maximize_effect@SocialWin@@QAEXH@Z)(SocialWin* this, int effectID)
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004AEBF0 0x005B4730 0x0062B870
// indirect  0x004AF1BE
*/
void SocialWin::maximize_effect(int effect_id) {
    effectMaximize_ = effect_id;
    for (int category = 0; category < 4; ++category) {
        int best_score = 0;
        int best_model = 0;
        for (int row = 0; row < 4; ++row) {
            if (society_avail(category, row, factionID_)
                && SocialCategories[category].model_effect[row].values_[effect_id]
                    > best_score) {
                best_model = row;
                best_score =
                    SocialCategories[category].model_effect[row].values_[effect_id];
            }
        }
        buttonGroups_[category].set(best_model + category * 4, 1);
        UNK1(category, best_model);
    }
    redraw();
    effectMaximize_ = -1;
}

/*
 * Purpose: The SubInterface status slot. Opening repaints the front, shows
 *          every flat button and energy lock, refreshes the main view, slides
 *          the window in and shows it; closing slides it back out, hides it,
 *          and hides the same buttons again.
// ORIGINAL: 0x004B2120 ?on_status@SocialWin@@QAEXH@Z 0x004B2120-0x004B2204 FILE BYTE_EXACT
// symbol    ?on_status@SocialWin@@UAEXH@Z
// size      228 bytes
// prototype void (__thiscall ?on_status@SocialWin@@QAEXH@Z)(SocialWin* this, int)
// callers   0   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0045D440 0x0045D970 0x005D56B0 0x005ED9D0 0x005EDCD0 0x005FCBB0
// indirect  0x004B213E 0x004B2152 0x004B216F 0x004B21D6 0x004B21F1
*/
void SocialWin::on_status(int mode) {
    if (mode != 0) {
        static_cast<GraphicWin *>(this)->on_redraw();
        FlatButton *button = flatButtons_;
        int count = 0x23;
        do {
            button->show(0);
            ++button;
            --count;
        } while (count != 0);
        CheckButton *lock = energyLockButtons_;
        count = 3;
        do {
            lock->show(0);
            ++lock;
            --count;
        } while (count != 0);
        MainInterfaceGlobal.update(0);
        do_all_draws();
        slide_show(this, 0);
        this->Win::show(3);
    } else {
        slide_hide(this, 0);
        this->Win::hide();
        do_all_draws();
        FlatButton *button = flatButtons_;
        int count = 0x23;
        do {
            button->hide();
            ++button;
            --count;
        } while (count != 0);
        CheckButton *lock = energyLockButtons_;
        count = 3;
        do {
            lock->hide();
            ++lock;
            --count;
        } while (count != 0);
    }
}

/*
 * Purpose: True when the faction may take one social effect. A negative
 *          entry in the player's own effect table marks a blocked pick, and
 *          both halves gate on a base id whose owner is this faction: the
 *          compare id for the category/model pairs, the probe id for the
 *          frenzy row. The comparison has to survive as a VALUE across the
 *          join below, or VC6 folds it into a `cmp`/`jne` pair and keeps
 *          `this` in ecx.
// ORIGINAL: 0x004B0FA0 ?immune@SocialWin@@QAEHHHH@Z 0x004B0FA0-0x004B1074 FILE BYTE_EXACT
// LEVER: push/mov the faction test lives in an `int` set on BOTH paths of the
//        `baseID >= 0` branch and tested after the join, not inside it. The
//        join forces VC6 to MATERIALISE the comparison (`xor eax,eax; sete al;
//        test eax,eax`) instead of folding it into `cmp/jne`, and that one
//        extra live value is what pushes `this` out of ecx into the
//        callee-saved edi (`mov edi, ecx`) - the whole prologue interleave
//        follows from it.
// size      212 bytes
// prototype int (__thiscall ?immune@SocialWin@@QAEHHHH@Z)(SocialWin* this, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
*/
int SocialWin::immune(int soc_category, int soc_model, int effect_id) {
    int faction_owns_base = 0;
    int base_id = GameDataStateStorage.fields_.secret_project_.cloning_vats();
    if (base_id >= 0) {
        unsigned char base_faction_id = Bases[base_id].faction_id_current;
        faction_owns_base = (static_cast<int>(base_faction_id) == factionID_);
    }
    if (faction_owns_base) {
        if ((soc_category == 2 && soc_model == 1)
            || (soc_category == 3 && soc_model == soc_category)) {
            if (SocialCategories[soc_category].model_effect[soc_model]
                    .values_[effect_id] < 0) {
                return 1;
            }
        }
    }

    faction_owns_base = 0;
    base_id = GameDataStateStorage.fields_.secret_project_.network_backbone();
    if (base_id >= 0) {
        unsigned char base_faction_id = Bases[base_id].faction_id_current;
        faction_owns_base = (static_cast<int>(base_faction_id) == factionID_);
    }
    if (faction_owns_base) {
        if (soc_category == 3 && soc_model == 1) {
            if (SocialCategories[3].model_effect[1].values_[effect_id] < 0) {
                return 1;
            }
        }
    }
    return 0;
}

/*
 * Purpose: Dispatch a click on the social panel's button grid. The right
 *          column (0x11..0x1a) is the maximize column: the row picks which
 *          effect to maximize, skipping the separator slot after the third
 *          row. The left grid (0..0xf) applies one model per category: the
 *          row is the category, the column the model, and when no maximize is
 *          armed and nothing is pending the group deselects and the panels
 *          redraw.
// ORIGINAL: 0x004B1CE0 ?on_group_clicked@SocialWin@@QAEXHH@Z 0x004B1CE0-0x004B1D64 FILE BYTE_EXACT
// symbol    ?on_group_clicked@SocialWin@@UAEXHH@Z
// size      132 bytes
// prototype void (__thiscall ?on_group_clicked@SocialWin@@QAEXHH@Z)(SocialWin* this, int, int)
// callers   0   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004AEBF0 0x004AF110 0x0062B870
// indirect  0x004B1D59
*/
void SocialWin::on_group_clicked(int a1, int a2) {
    if (a2 >= 0x11 && a2 < 0x1b) {
        a2 -= 0x11;
        if (a2 >= 3) {
            ++a2;
        }
        maximize_effect(a2);
        return;
    }
    if (a2 >= 0 && a2 < 0x10) {
        int model = a2 % 4;
        int category = a2 / 4;
        UNK1(category, model);
        if (effectMaximize_ == -1 && field_2C28_ == 0) {
            buttonGroup_.set(-1, 1);
            redraw();
        }
    }
}

/*
 * Purpose: Track the pointer across the social panel's two hover regions.
 *          Rows 0..0xf are the category/model buttons: entering a new one
 *          repaints just the mouseover panel. Rows 0x11..0x1b are the effect
 *          column: entering clears to -1 and repaints the overview (leaving
 *          the hover cleared), otherwise the new effect id repaints it too.
 *          Every path ends in the shared do_all_draws pass.
// ORIGINAL: 0x004B1D70 ?on_button_passover@SocialWin@@QAEXHH@Z 0x004B1D70-0x004B1DF3 FILE BYTE_EXACT
// symbol    ?on_button_passover@SocialWin@@UAEXHH@Z
// size      131 bytes
// prototype void (__thiscall ?on_button_passover@SocialWin@@QAEXHH@Z)(SocialWin* this, int, int)
// callers   0   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004B0410 0x004B1080 0x005FCBB0
*/
void SocialWin::on_button_passover(int a1, int a2) {
    if (a1 >= 0 && a1 < 0x10 && a2 != 0 && hover_selection_ != a1) {
        hover_selection_ = a1;
        draw_mouseover();
    }
    if (a1 >= 0x11 && a1 <= 0x1b) {
        if (a2 == 0) {
            if (effect_hover_button_id_ != -1) {
                effect_hover_button_id_ = -1;
                draw_overview();
                do_all_draws();
                return;
            }
        } else if (effect_hover_button_id_ != a1) {
            effect_hover_button_id_ = a1;
            draw_overview();
        }
    }
    do_all_draws();
}

// Right-aligned label write with the image's null test on the text.
static inline void social_write_right(char *text, int x, int y) {
    if (text != 0) {
        MainInterfaceGlobal.Buffer::write_right_l(text, x, y, strlen(text));
    }
}

// ORIGINAL: 0x004B0410 ?draw_overview@SocialWin@@QAEXXZ 0x004B0410-0x004B089A BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x004B0410-0x004B089A draws the effects overview, skips the reserved fourth row, loads each SocialEffects section and repaints the selected/comparison rows", "source:src/socialwin.h: tooltipEffects_, effectMaximize_, socEffect_, font1_ and font7_ are the corresponding declared members", "source:src/faction.h: SocialEffect and RulesSocialEffect define the eleven live rows and their section names"], "reason": "Home the overview painter on the SocialWin-owned rectangles, effects, fonts and rules records; preserve its original identity while replacing the seed's opaque object accesses with the product model."}
void SocialWin::draw_overview() {
    int selected_effect;
    if (effect_hover_button_id_ < 0) {
        selected_effect = -1;
    } else {
        selected_effect = effect_hover_button_id_ - 0x11;
        if (selected_effect >= 3) {
            ++selected_effect;
        }
    }

    MainInterfaceGlobal.Buffer::restore_clip();
    MainInterfaceGlobal.Buffer::set_wrap_width(5);

    RECT panel = tooltipEffects_;
    MainInterfaceGlobal.Buffer::box_sprite(&panel, &InterfaceBoxStyles[3]);
    panel.left += 3;
    panel.top += 3;
    panel.right -= 3;
    panel.bottom -= 3;
    RECT title_rect = panel;
    title_rect.bottom = title_rect.top + 20;
    MainInterfaceGlobal.Buffer::box_sprite(&title_rect, &InterfaceBoxStyles[6]);

    MainInterfaceGlobal.Buffer::set_font(&font1_, 0, 0, 0);
    MainInterfaceGlobal.Buffer::set_text_color(0x9f, -1, 1, 1);

    char *title = StringTable.get(Labels.strings_ptr[0x2e4]);
    if (title != 0) {
        MainInterfaceGlobal.Buffer::write_cent_l(title, &title_rect, strlen(title));
    }

    panel.top = title_rect.bottom + 2;
    MainInterfaceGlobal.Buffer::box_sprite(&panel, &InterfaceBoxStyles[0]);
    MainInterfaceGlobal.Buffer::set_font(&font7_, 0, 0, 0);

    panel.left += 20;
    const int number_x = panel.left - 5;
    panel.right -= 3;
    int row_pitch_cursor = panel.top;
    char descriptions[MaxSocialEffectNum][256];
    int content_height = 0;
    int last_description_length;

    text_open(AlphaxFileID, "SOCIO");

    // Each rules entry names one SOCIAL section; index 3 is the reserved
    // slot with no section.
    int effect;
    for (effect = 0; effect < MaxSocialEffectNum; ++effect) {
        if (effect == 3) {
            continue;
        }

        StringTemp[0] = 0;
        strcat(StringTemp, SocialWinOverviewSocText);
        strcat(StringTemp, SocialEffects[effect].set1);
        if (text_open(0, StringTemp) != 0) {
            text_close();
            return;
        }

        descriptions[effect][0] = 0;
        text_get();
        int row = text_item_number();
        strcpy(descriptions[effect], text_item());
        while (row < socEffect_.values_[effect]) {
            text_get();
            if (strlen(TextBufferGetPtr) == 0) {
                break;
            }
            row = text_item_number();
            strcpy(descriptions[effect], text_item());
        }
        last_description_length = strlen(descriptions[effect]);
        content_height += MainInterfaceGlobal.Buffer::wrap_height(
            descriptions[effect], panel.right - panel.left);
    }
    text_close();

    panel.top += ((panel.bottom - content_height) - panel.top) / 2;
    for (effect = 0; effect < MaxSocialEffectNum; ++effect) {
        if (effect == 3) {
            continue;
        }
        if (selected_effect == effect) {
            MainInterfaceGlobal.Buffer::set_text_color(0xff, -1, 1, 1);
        } else if (effectMaximize_ == effect) {
            MainInterfaceGlobal.Buffer::set_text_color(0x9c, -1, 1, 1);
        } else if (socEffect_.values_[effect]
                   == PlayersData[factionID_].soc_effect_pending.values_[effect]) {
            MainInterfaceGlobal.Buffer::set_text_color(0xa0, -1, 1, 1);
        } else if (socEffect_.values_[effect]
                   > PlayersData[factionID_].soc_effect_pending.values_[effect]) {
            MainInterfaceGlobal.Buffer::set_text_color(0xfa, -1, 1, 1);
        } else {
            MainInterfaceGlobal.Buffer::set_text_color(0xf9, -1, 1, 1);
        }

        StringTemp[0] = 0;
        if (socEffect_.values_[effect] > 0) {
            strcat(StringTemp, SocialWinOverviewPlusText);
        }
        char number[80];
        _itoa(socEffect_.values_[effect], number, 10);
        strcat(StringTemp, number);
        MainInterfaceGlobal.Buffer::set_font(&font7_, 0, 0, 0);
        social_write_right(StringTemp, number_x, panel.top);
        socEffectOrig_.values_[effect] =
            PlayersData[factionID_].soc_effect_pending.values_[effect];

        // The original reuses the final parsed description's length for
        // every row, skipping all wraps when that last description is empty.
        if (last_description_length != 0) {
            MainInterfaceGlobal.Buffer::set_font(&font7_, 0, 0, 0);
            panel.top = MainInterfaceGlobal.Buffer::wrap(descriptions[effect], &panel);
        }
        // A separate cursor that never feeds the wrapping RECT's top.
        row_pitch_cursor += font_line_advance(font7_);
    }

    MainInterfaceGlobal.Buffer::set_wrap_width(5);
    MainInterfaceGlobal.soft_update(&tooltipEffects_);
    do_sound();
}

// ORIGINAL: 0x004AECE0 ?reset@SocialWin@@QAEXXZ 0x004AECE0-0x004AEEF1 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x004AECE0-0x004AEEF1 resets the selected category, button groups, effect snapshots, social allocation values, calculated energy and visible-window redraw", "source:src/faction.h: PlayersData owns the pending SocialCategory, SocialEffect and social allocation values", "source:src/socialwin.h: SocialWin owns the working/original values and category button groups"], "reason": "Restore the panel from its owning PlayerData record through typed social calculation and button-group operations while retaining the mapped reset identity."}
void SocialWin::reset() {
    field_A1C_ = 0xFFFFFFFFU;
    social_ai(factionID_, -1, -1, -1, -1, 0);

    field_2C28_ = 1;
    for (int category = 0; category < 4; ++category) {
        const int model = static_cast<int>(
            PlayersData[factionID_].soc_category_pending.models_[category]);
        socCategory_.models_[category] = static_cast<uint32_t>(model);
        buttonGroups_[category].set(model + category * 4, 1);
    }
    field_2C28_ = 0;

    for (int effect = 0; effect < MaxSocialEffectNum; ++effect) {
        const int value = PlayersData[factionID_].soc_effect_pending.values_[effect];
        socEffectOrig_.values_[effect] = value;
        socEffect_.values_[effect] = value;
    }

    const int allocations_changed =
        PlayersData[factionID_].se_alloc_psych != static_cast<int>(savedAlloc_[1]) ||
        PlayersData[factionID_].se_alloc_labs != static_cast<int>(savedAlloc_[2]);
    PlayersData[factionID_].se_alloc_psych = static_cast<int>(savedAlloc_[1]);
    PlayersData[factionID_].se_alloc_labs = static_cast<int>(savedAlloc_[2]);

    for (int allocation = 0; allocation < 3; ++allocation) {
        pendingAlloc_[allocation] = savedAlloc_[allocation];
    }

    if (allocations_changed) {
        social_set(factionID_);
        social_calc(&socCategory_, &socEffect_, factionID_, 0, 0);
        social_calc(&socCategory_, &PlayersData[factionID_].soc_effect_pending,
                    factionID_, 0, 0);
    }

    energy_compute(factionID_, 1);
    breakthroughTurns_ = BreakthroughTurnsGlobal;
    netIncome_ = NetIncomeGlobal;
    social_calc(&PlayersData[factionID_].soc_category_pending,
                &PlayersData[factionID_].soc_effect_pending, factionID_, 0, 0);

    if (is_visible()) {
        redraw();
    }
}

/*
 * Purpose: Track the pointer over the social panel: the bubble is cleared
 *          (twice, exactly as the original does), then the four category
 *          tooltip rectangles are scanned row by row - each available row
 *          tests its grid rectangle against the cursor, and the inner row
 *          scan stops at a hit while the outer category scan keeps running,
 *          so a later category's hit wins. The faction-power rectangle maps
 *          the hover to -2, anything else to the grid index or -1, and a
 *          changed hover repaints the mouseover panel.
// ORIGINAL: 0x004B1EC0 ?on_mouse_move@SocialWin@@QAEXHH@Z 0x004B1EC0-0x004B1FB2 FILE BYTE_EXACT
// symbol    ?on_mouse_move@SocialWin@@UAEXHH@Z
// size      242 bytes
// prototype void (__thiscall ?on_mouse_move@SocialWin@@QAEXHH@Z)(SocialWin* this, int, int)
// callers   0   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004B1080 0x005B4730 0x005F8500 0x005FA7E0
*/
void SocialWin::on_mouse_move(int a1, int a2) {
    ::Win::clear_bubble_text();
    ::Win::clear_bubble_text();

    int hover = -1;
    for (int category = 0; category < 4; ++category) {
        for (int row = 0; row < 4; ++row) {
            if (society_avail(category, row, factionID_) == 0) {
                continue;
            }
            RECT rect = categoryHeaders_[category];
            rect.left = modelRects_[row + category * 4].left;
            rect.right = modelRects_[row + category * 4].right;
            if (in_box(a1, a2, &rect) != 0) {
                hover = row + category * 4;
                break;
            }
        }
    }

    if (in_box(a1, a2, &tooltipFactionPower_) != 0) {
        hover = -2;
    }
    if (hover != hover_selection_) {
        hover_selection_ = hover;
        draw_mouseover();
    }
}

/*
 * Purpose: Draw one social button's text band: the effect icon at the left
 *          (its colour picked from the button's status), the effect name, and
 *          the count on the right - signed "+" and recoloured when it differs
 *          from the faction's stored value. Invoked for the effect column's
 *          buttons only (0x11..0x1a), with the separator slot skipped.
// ORIGINAL: 0x004B17A0 ?on_button_text_draw@SocialWin@@QAEHPAUGraphicWin@@PADH@Z 0x004B17A0-0x004B1991 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Compare the displayed effect against the selected faction's pending SocialEffect member rather than an original-address word view. Keep the existing selector and unchecked index behavior.","evidence":["pinned:0x004B18DA reads [edx*4+0x0096CC2C] with the selected faction's 2099-dword PlayerData stride and effect index","source:PlayersData base 0x0096C9E0,PlayerData stride 0x20CC,soc_effect_pending+0x24C,SocialEffect::values_+0x0"]}
// symbol    ?on_button_text_draw@SocialWin@@UAEHPAVGraphicWin@@PADH@Z
// TRIED: strlen/strcat named plainly get inlined by /O2 (fixed via the nullary
//        _strlen/_strcat function-pointer lever, sim 0.71->0.90); remaining
//        divergence at #17 is this/a1 register-allocation (esi vs edi) not
//        chased further; not byte-exact.
// size      497 bytes
// prototype int (__thiscall ?on_button_text_draw@SocialWin@@QAEHPAUGraphicWin@@PADH@Z)(SocialWin* this, GraphicWin*, int8*, int)
// callers   0   call targets   7
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005DACB0 0x005DCF40 0x005DD450 0x005E4B4A 0x006453E0 0x00645470 0x0064FC88
*/
int SocialWin::on_button_text_draw(GraphicWin *window, char *text, int button_id) {
    if (button_id < 0x11 || button_id >= 0x1b) {
        return 0;
    }
    int effect = button_id - 0x11;
    if (effect >= 3) {
        ++effect;
    }

    // The buffer's own working rectangle (Buffer::rect2_, 0x474).
    RECT rect = window->clip_rect();
    int y_center = (rect.bottom - effectIcons_[0].height() - rect.top) / 2;

    int flag;
    switch (flatButtons_[button_id].field_AB8()) {
    case 0:
        flag = 1;
        break;
    case 1:
        flag = 0;
        break;
    case 2:
        flag = 0;
        break;
    default:
        if (flatButtons_[button_id].selected_state() != 0) {
            flag = 0;
        } else {
            flag = (flatButtons_[button_id].field_A74() == 0) ? 1 : 0;
        }
        break;
    }

    // Original stride 0x84 is three 0x2C-byte Sprites, not one Sprite.
    social_draw_icon(&effectIcons_[(button_id - 0x11) * 9 + flag * 3], window, 5, y_center);

    window->set_text_color(0, -1, 1, 1);
    rect.left += 0x25;
    if (text != 0) {
        window->write_l(text, &rect, strlen(text));
    }

    if (socEffect_.values_[effect]
        > PlayersData[factionID_].soc_effect_pending.values_[effect]) {
        window->set_text_color(0xfa, -1, 1, 1);
    } else if (socEffect_.values_[effect]
               < PlayersData[factionID_].soc_effect_pending.values_[effect]) {
        window->set_text_color(0xf9, -1, 1, 1);
    }

    StringTemp[0] = 0;
    if (socEffect_.values_[effect] > 0) {
        strcat(StringTemp, SocialWinButtonCountPlusText);
    }
    char number[80];
    _itoa(socEffect_.values_[effect], number, 10);
    strcat(StringTemp, number);

    rect.right -= 3;
    if (StringTemp != 0) {
        window->write_right_l(StringTemp, &rect, strlen(StringTemp));
    }
    return 1;
}

/*
 * Purpose: Repaint the mouseover panel. The header shows the hovered effect's
 *          name centred; the body then walks the eleven effects of the current
 *          category (skipping the Null row 3), counting the live ones and
 *          drawing each as sign, magnitude, name and icon - skipping entries
 *          the faction's own base probes mark unavailable. The -2 hover (the
 *          faction-power rectangle) shows the faction's power column instead,
 *          and -1 draws the empty frame only.
// ORIGINAL: 0x004B1080 ?draw_mouseover@SocialWin@@QAEXXZ 0x004B1080-0x004B178F FILE
// TRIED: MISMATCH #9 'call' vs 'mov' (framed build, sim 0.57). Diplomacy mood
//        tooltip: two nested 11-slot table walks (skipping index 3, the "self"
//        faction) with two neg/border-guard special cases.
// size      1807 bytes
// prototype void (__thiscall ?draw_mouseover@SocialWin@@QAEXXZ)(SocialWin* this)
// callers   4   call targets   13
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005D5930 0x005D8000 0x005DAC70 0x005DACB0 0x005DCF40 0x005DD130 0x005E2DD3 0x005E4B4A 0x005FD2B0 0x006169A0 0x006453E0 0x00645470 0x0064FC88
// indirect  0x004B1162 0x004B12CA
*/
void SocialWin::draw_mouseover() {
    MainInterfaceGlobal.Buffer::restore_clip();

    RECT rect = tutRect9_;
    MainInterfaceGlobal.Buffer::box_sprite(&rect, &InterfaceBoxStyles[3]);
    rect.left += 3;
    rect.top += 3;
    rect.right -= 3;
    rect.bottom -= 3;
    RECT title = rect;
    title.bottom = title.top + 20;
    MainInterfaceGlobal.Buffer::box_sprite(&title, &InterfaceBoxStyles[6]);

    // The image leaves these unset on the -2 path and still reads them in
    // the drawing pass's project tests.
    int category;
    int model;
    if (hover_selection_ != -1) {
        if (hover_selection_ == -2) {
            StringTemp[0] = 0;
            strcat(StringTemp, label_get(0xB94 / 4));
            CharUpperA(StringTemp);
            MainInterfaceGlobal.Buffer::set_font(&font1_, 0, 0, 0);
            MainInterfaceGlobal.Buffer::set_text_color(0x9f, -1, 1, 1);
            social_write_cent(StringTemp, &title);
        } else {
            category = hover_selection_ / 4;
            model = hover_selection_ % 4;
            StringTemp[0] = 0;
            char *name = SocialCategories[category].name[model];
            strcat(StringTemp, StringTable.get(name));
            CharUpperA(StringTemp);
            MainInterfaceGlobal.Buffer::set_font(&font1_, 0, 0, 0);
            MainInterfaceGlobal.Buffer::set_text_color(0x9f, -1, 1, 1);
            social_write_cent(StringTemp, &title);
        }
    }

    rect.top = title.bottom + 2;
    MainInterfaceGlobal.Buffer::box_sprite(&rect, &InterfaceBoxStyles[0]);

    if (hover_selection_ != -1) {
        rect.left += 3;
        rect.right -= 3;
        rect.top += 3;
        rect.bottom -= 3;

        int count = 0;
        int effect;
        for (effect = 0; effect < MaxSocialEffectNum; ++effect) {
            if (effect == 3) {
                continue;
            }
            int value;
            if (hover_selection_ == -2) {
                value = SOCIAL_ABS(PlayersData[factionID_].soc_effect_base.values_[effect]);
            } else {
                if (has_project(SP_CLONING_VATS, factionID_)
                    && ((category == 2 && model == 1) || (category == 3 && model == 3))
                    && SocialCategories[category].model_effect[model].values_[effect] < 0) {
                    continue;
                }
                if (has_project(SP_NETWORK_BACKBONE, factionID_) && category == 3 && model == 1
                    && SocialCategories[3].model_effect[1].values_[effect] < 0) {
                    continue;
                }
                value = SOCIAL_ABS(SocialCategories[category].model_effect[model].values_[effect]);
            }
            count += value != 0;
        }

        const int line_height = effectIcons_[0].height();
        MainInterfaceGlobal.Buffer::set_font(&font3_, 0, 0, 0);
        rect.top += ((rect.bottom - line_height * count) - rect.top) / 2;
        rect.left += 3;
        rect.bottom = line_height + rect.top;

        // Bonuses first, then penalties.
        for (int pass = 0; pass < 2; ++pass) {
            for (effect = 0; effect < MaxSocialEffectNum; ++effect) {
                if (effect == 3) {
                    continue;
                }
                if (has_project(SP_CLONING_VATS, factionID_)
                    && ((category == 2 && model == 1) || (category == 3 && model == 3))
                    && SocialCategories[category].model_effect[model].values_[effect] < 0) {
                    continue;
                }
                if (has_project(SP_NETWORK_BACKBONE, factionID_) && category == 3 && model == 1
                    && SocialCategories[3].model_effect[1].values_[effect] < 0) {
                    continue;
                }
                int value;
                if (hover_selection_ == -2) {
                    value = PlayersData[factionID_].soc_effect_base.values_[effect];
                } else {
                    value = SocialCategories[category].model_effect[model].values_[effect];
                }
                if (value == 0) {
                    continue;
                }
                if (pass == 0) {
                    if (value < 0) {
                        continue;
                    }
                } else if (value > 0) {
                    continue;
                }

                StringTemp[0] = 0;
                if (value < 0) {
                    strcat(StringTemp, SocialWinMouseoverMinusText);
                    MainInterfaceGlobal.Buffer::set_text_color(0x67, -1, 1, 1);
                } else {
                    strcat(StringTemp, SocialWinMouseoverPlusText);
                    MainInterfaceGlobal.Buffer::set_text_color(0x98, -1, 1, 1);
                }
                char number[80];
                _itoa(SOCIAL_ABS(value), number, 10);
                strcat(StringTemp, number);
                social_write_l(StringTemp, &rect);

                social_draw_icon(&effectIcons_[(effect < 3 ? effect * 9 : (effect - 1) * 9) + 6
                                               + (value >= 0 ? 2 : 1)],
                                 &MainInterfaceGlobal, rect.left + 0x10, rect.top);

                rect.left = rect.left + 0x10 + effectIcons_[0].width();
                StringTemp[0] = 0;
                strcat(StringTemp, SocialEffects[effect].set2);
                social_write_l(StringTemp, &rect);
                rect.left = rect.left - 0x10 - effectIcons_[0].width();
                rect.top += line_height;
                rect.bottom += line_height;
            }
        }
    }

    MainInterfaceGlobal.soft_update(&tutRect9_);
    do_sound();
}

/*
 * Purpose: Repaint the info header above the social panel: the category
 *          title, the energy line (net income and the per-turn breakthrough
 *          figure), the environment line showing how far the pending
 *          allocation sits from balanced (each unit of imbalance is a
 *          percentage, signed and recoloured), and the upheaval line showing
 *          what the pending model change costs. Clipped to the panel
 *          rectangle, which is restored as the buffer's own clip on the way
 *          out.
// ORIGINAL: 0x004AFB60 ?draw_info@SocialWin@@QAEXXZ 0x004AFB60-0x004B0405 FILE
// TRIED: checked the return-type flag directly - the disassembly has exactly
//        ONE `ret` (0x004B0404), reached only via fallthrough after `call
//        set_clip`, with no `mov eax,N` immediately before it and no
//        early-exit paths. The 4/0x28/0x50 values the audit saw are
//        percentage-bar clamp bounds, not return-value setup; `void` stands.
//        Reaches MISMATCH #4 'mov' vs 'push' (register/stack allocation), not
//        chased further given size (2213 bytes).
// size      2213 bytes
// prototype void (__thiscall ?draw_info@SocialWin@@QAEXXZ)(SocialWin* this)
// callers   1   call targets   13
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005B4550 0x005D5930 0x005D8000 0x005DAC70 0x005DACB0 0x005DCEA0 0x005DD130 0x005E2DD3 0x005FD2B0 0x006169A0 0x006453E0 0x00645470 0x0064FC88
*/
void SocialWin::draw_info() {
    RECT rect = tutRect14_;
    Buffer::set_clip(&tutRect14_);
    RECT title = rect;
    title.bottom = rect.top + 20;
    MainInterfaceGlobal.Buffer::box_sprite(&title, &InterfaceBoxStyles[6]);

    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0x2BB));
    CharUpperA(StringTemp);
    MainInterfaceGlobal.Buffer::set_font(&font1_, 0, 0, 0);
    MainInterfaceGlobal.Buffer::set_text_color(0x9c, -1, 1, 1);
    social_write_cent(StringTemp, &title);

    rect.top = title.bottom + 2;
    MainInterfaceGlobal.Buffer::box_sprite(&rect, &InterfaceBoxStyles[0]);
    social_expand_rect(&rect, -3, -3);
    MainInterfaceGlobal.Buffer::set_font(&font5_, 0, 0, 0);

    // Original 0x96CC28 is PlayersData::soc_upheaval_cost_paid at +0x248.
    const int upheaval = static_cast<int>(social_upheaval(factionID_, &socCategory_))
        - PlayersData[factionID_].soc_upheaval_cost_paid;

    int x = rect.left;
    int y;
    if (WinScreenWidth == 800) {
        y = font_line_advance(font5_) + rect.top;
    } else {
        y = ((rect.bottom - font_line_advance(font5_) * 2) - rect.top) / 2 + rect.top;
    }

    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0x3BF));
    strcat(StringTemp, SocialWinColonSpaceText);
    char number[80];
    _itoa(NetIncomeGlobal, number, 10);
    strcat(StringTemp, number);
    MainInterfaceGlobal.Buffer::set_text_color(0xa0, -1, 1, 1);
    social_write_l_at(StringTemp, x, y);

    y += font_line_advance(font5_);
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(150));
    strcat(StringTemp, SocialWinColonSpaceText);
    _itoa(BreakthroughTurnsGlobal, number, 10);
    strcat(StringTemp, number);
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, label_get(49));
    social_write_l_at(StringTemp, x, y);

    if (WinScreenWidth == 800) {
        y += font_line_advance(font5_);
    } else {
        x += (rect.right - rect.left) / 2;
        y -= font_line_advance(font5_);
    }

    int imbalance = PlayersData[factionID_].se_alloc_psych
        + PlayersData[factionID_].se_alloc_labs * 2 - 10;
    if (imbalance < 0) {
        imbalance = (5 - PlayersData[factionID_].se_alloc_labs) * 2
            - PlayersData[factionID_].se_alloc_psych;
    }
    imbalance /= 2;
    const int shift = SOCIAL_MAGNITUDE(range(socEffect_.values_[1], -4, 4) - 4) * 2;

    if (imbalance != 0 && shift != 0) {
        StringTemp[0] = 0;
        if (PlayersData[factionID_].se_alloc_labs
            > 10 - PlayersData[factionID_].se_alloc_psych - PlayersData[factionID_].se_alloc_labs) {
            _itoa(-range(shift * imbalance, 0, 0x28), number, 10);
            strcat(StringTemp, number);
            strcat(StringTemp, SocialWinPercentSpaceTextA);
            strcat(StringTemp, label_get(60));
            if (10 - PlayersData[factionID_].se_alloc_psych - PlayersData[factionID_].se_alloc_labs != 0) {
                strcat(StringTemp, SocialWinCommaSpaceText);
                _itoa(-range(shift * imbalance * 2, 0, 0x50), number, 10);
                strcat(StringTemp, number);
                strcat(StringTemp, SocialWinPercentSpaceTextB);
                strcat(StringTemp, label_get(169));
            }
        } else {
            if (PlayersData[factionID_].se_alloc_labs != 0) {
                _itoa(-range(shift * imbalance * 2, 0, 0x50), number, 10);
                strcat(StringTemp, number);
                strcat(StringTemp, SocialWinPercentSpaceTextC);
                strcat(StringTemp, label_get(60));
                strcat(StringTemp, SocialWinCommaSpaceText);
            }
            _itoa(-range(shift * imbalance, 0, 0x28), number, 10);
            strcat(StringTemp, number);
            strcat(StringTemp, SocialWinPercentSpaceTextD);
            strcat(StringTemp, label_get(169));
        }
        MainInterfaceGlobal.Buffer::set_text_color(0xf9, -1, 1, 1);
        social_write_l_at(StringTemp, x, y);
        y += font_line_advance(font5_);
    }

    if (upheaval < 0) {
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(350));
        strcat(StringTemp, SocialWinColonSpaceText);
        strcat(StringTemp, SocialWinUpheavalPlusText);
        _itoa(-upheaval, number, 10);
        strcat(StringTemp, number);
        strcat(StringTemp, BufferJoinSpace);
        strcat(StringTemp, label_get(351));
        MainInterfaceGlobal.Buffer::set_text_color(0xa0, -1, 1, 1);
        social_write_l_at(StringTemp, x, y);
    } else if (upheaval > 0) {
        MainInterfaceGlobal.Buffer::set_text_color(0xf9, -1, 1, 1);
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(349));
        strcat(StringTemp, SocialWinColonSpaceText);
        _itoa(upheaval, number, 10);
        strcat(StringTemp, number);
        strcat(StringTemp, BufferJoinSpace);
        strcat(StringTemp, label_get(351));
        social_write_l_at(StringTemp, x, y);
    }

    MainInterfaceGlobal.soft_update(&tutRect14_);
    do_sound();
    Buffer::restore_clip();
}


// Existing source owners behind the original storage used by init:
// InterfaceRegistry[2] is the SubInterface pointer slot at 0x007AE7F8;
// MainInterfaceGlobal.main_frame_rect() is the RECT at 0x007AF5A0, and its
// right_frame_rect()/lower_frame_rect() own the RECTs at 0x007AF5B0/0x007AF5D0.
// The info panel RECT at 0x007AF5C0 is copied through info_panel_frame().
// TutorialWindow owns stage dword 0x008CC228 at field_53C0_. The tutorial
// social RECT 0x008C5380 is TutWinSocWindow.categoryHeaders_[0], not this init
// receiver's rectangle; preserve that singleton relationship.
// DefaultFontFace and DefaultLargeFontFace are the genuine source-owned
// globals already bound to the original face-name slots at 0x00691B2C/30.
// SocialWinPcxName is the writable source-owned array for original 0x006874B0,
// whose pinned initial bytes are "socwin.pcx".
// TutorialStage 0x008CC228 - the int the tutorial entry stores 5 into.
// TutorialSocRect 0x008C5380 - the RECT TutWin::soc_rect fills.
// SocialWinTutorialName is the writable source-owned array for original
// 0x006874BC, whose pinned initial bytes are "SOCWIN".
// ConsoleGlobal is the modal parent passed to the original window init.
int __cdecl pop_wait();  // 0x005398E0, defined in faction.cpp
int __cdecl tut_check2(int veh_id);  // 0x004BA650, defined in tutwin.cpp

/*
 * Purpose: Build the social panel for one faction: close every button and
 *          group, reset the hover state, lay out the panel's rectangles and
 *          fonts from MainInterface's screen metrics, extract the sprite
 *          sheets into the icon runs, size the window over the main view,
 *          seed the allocation sliders from SocialAllocTable, then create the
 *          category grid, the effect column, the energy locks and the six
 *          header toggles - each with its label and bubble text. A negative
 *          faction id builds without the tutorial hand-off.
// ORIGINAL: 0x004B2740 ?init@SocialWin@@QAEXH@Z 0x004B2740-0x004B3C7D;0x00659A0A-0x00659A1F FILE
// TRIED: full-function transcription (~1448 instructions, sim 0.57 vs
//        original at /O2 /Oy-); diverges at instr #9. Receivers for member
//        calls were traced by hand from `mov ecx,...` immediately preceding
//        each call site.
// size      5458 bytes
// prototype void (__thiscall ?init@SocialWin@@QAEXH@Z)(SocialWin* this, int factionID)
// callers   2   call targets   26
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0045D380 0x004AECE0 0x004BA650 0x004BC640 0x004BDFE0 0x005B4730 0x005D4EF0 0x005D5250 0x005D7210 0x005D7410 0x005D7670 0x005D7DE0 0x005DAC70 0x005DFF00 0x005E2DD3 0x005E39A0 0x00607210 0x00607550 0x006169A0 0x00618F40 0x0062B7F0 0x0062B810 0x0062B850 0x0062B870 0x006337E0 0x00645470
*/
void SocialWin::init(int faction_id) {
    Buffer sheet;

    for (int i = 0; i < 0x23; ++i) {
        flatButtons_[i].close();
    }
    for (int group_index = 0; group_index < 4; ++group_index) {
        buttonGroups_[group_index].close();
    }
    buttonGroup_.close();

    hover_selection_ = -1;
    effect_hover_button_id_ = -1;
    field_2C28_ = 0;
    effectMaximize_ = -1;
    // The interface-mode registry's own state: the SubInterface base pointer
    // and its mode field.
    InterfaceRegistry[2] = static_cast<SubInterface *>(this);
    field_4_ = 2;
    int a1 = faction_id;
    int positive = (a1 < 0) ? 1 : 0;
    if (a1 < 0) {
        a1 = -a1;
    }
    factionID_ = a1;
    int wide_screen = (WinScreenWidth != 800) ? 1 : 0;
    (void)wide_screen;

    tutRect14_ = MainInterfaceGlobal.info_panel_frame();
    tutRect14_.left += 4;
    tutRect14_.right -= 4;
    tutRect14_.top += 4;
    tutRect14_.bottom -= 4;

    tutRect1_.left = 0x271;
    tutRect1_.top = 4;
    tutRect1_.right = 0x306;
    tutRect1_.bottom = 0xfa;

    {
        int v = 0x1d;
        for (int i = 0; i < 10; ++i) {
            tutRects2_[i].left = 0x274;
            tutRects2_[i].top = v;
            tutRects2_[i].right = 0x303;
            tutRects2_[i].bottom = v + 0x14;
            v += 0x16;
        }
    }

    tutRect13_.left = 0x9a; tutRect13_.top = 0x13a; tutRect13_.right = 0x13c; tutRect13_.bottom = 0x14e;
    tutRect12_.left = 0x13f; tutRect12_.top = 0x13a; tutRect12_.right = 0x1e1; tutRect12_.bottom = 0x14e;
    tutRect11_.left = 0x1e4; tutRect11_.top = 0x13a; tutRect11_.right = 0x286; tutRect11_.bottom = 0x14e;
    categoryHeaders_[0].left = 0x1a;
    categoryHeaders_[0].top = 4;
    categoryHeaders_[0].right = 0x26e;
    categoryHeaders_[0].bottom = 0x4a;
    categoryHeaders_[1].left = 0x1a; categoryHeaders_[1].top = 0x4d; categoryHeaders_[1].right = 0x26e; categoryHeaders_[1].bottom = 0x93;
    categoryHeaders_[2].left = 0x1a; categoryHeaders_[2].top = 0x96; categoryHeaders_[2].right = 0x26e; categoryHeaders_[2].bottom = 0xdc;
    categoryHeaders_[3].left = 0x1a; categoryHeaders_[3].top = 0xdf; categoryHeaders_[3].right = 0x26e; categoryHeaders_[3].bottom = 0x125;
    modelRects_[0].left = 0x1d; modelRects_[0].top = 0x1d; modelRects_[0].right = 0xaf; modelRects_[0].bottom = 0x31;
    modelRects_[1].left = 0xb1; modelRects_[1].top = 0x1d; modelRects_[1].right = 0x143; modelRects_[1].bottom = 0x31;
    modelRects_[2].left = 0x145; modelRects_[2].top = 0x1d; modelRects_[2].right = 0x1d7; modelRects_[2].bottom = 0x31;
    modelRects_[3].left = 0x1d9; modelRects_[3].top = 0x1d; modelRects_[3].right = 0x26b; modelRects_[3].bottom = 0x31;
    modelRects_[4].left = 0x1d; modelRects_[4].top = 0x66; modelRects_[4].right = 0xaf; modelRects_[4].bottom = 0x7a;
    modelRects_[5].left = 0xb1; modelRects_[5].top = 0x66; modelRects_[5].right = 0x143; modelRects_[5].bottom = 0x7a;
    modelRects_[6].left = 0x145; modelRects_[6].top = 0x66; modelRects_[6].right = 0x1d7; modelRects_[6].bottom = 0x7a;
    modelRects_[7].left = 0x1d9; modelRects_[7].top = 0x66; modelRects_[7].right = 0x26b; modelRects_[7].bottom = 0x7a;
    modelRects_[8].left = 0x1d; modelRects_[8].top = 0xaf; modelRects_[8].right = 0xaf; modelRects_[8].bottom = 0xc3;
    modelRects_[9].left = 0xb1; modelRects_[9].top = 0xaf; modelRects_[9].right = 0x143; modelRects_[9].bottom = 0xc3;
    modelRects_[10].left = 0x145; modelRects_[10].top = 0xaf; modelRects_[10].right = 0x1d7; modelRects_[10].bottom = 0xc3;
    modelRects_[11].left = 0x1d9; modelRects_[11].top = 0xaf; modelRects_[11].right = 0x26b; modelRects_[11].bottom = 0xc3;
    modelRects_[12].left = 0x1d; modelRects_[12].top = 0xf8; modelRects_[12].right = 0xaf; modelRects_[12].bottom = 0x10c;
    modelRects_[13].left = 0xb1; modelRects_[13].top = 0xf8; modelRects_[13].right = 0x143; modelRects_[13].bottom = 0x10c;
    modelRects_[14].left = 0x145; modelRects_[14].top = 0xf8; modelRects_[14].right = 0x1d7; modelRects_[14].bottom = 0x10c;
    modelRects_[15].left = 0x1d9; modelRects_[15].top = 0xf8; modelRects_[15].right = 0x26b; modelRects_[15].bottom = 0x10c;
    tooltipFactionPower_.left = 0x271;
    tooltipFactionPower_.top = 0xfd;
    tooltipFactionPower_.right = 0x306;
    tooltipFactionPower_.bottom = 0x12d;

    {
        int left = MainInterfaceGlobal.main_frame_rect().left + 7;
        int bottom = MainInterfaceGlobal.lower_frame_rect().bottom - 7;
        int top = MainInterfaceGlobal.lower_frame_rect().top + 7;
        int split = ((MainInterfaceGlobal.lower_frame_rect().right - left) - 4) / 3 - 3 + left;
        tooltipEconomy_.left = left;
        tooltipEconomy_.top = top;
        tooltipEconomy_.right = split;
        tooltipEconomy_.bottom = bottom;
        int width = (split - left) + 3;
        split += width;
        tooltipPsych_.left = left + width;
        tooltipPsych_.top = top;
        tooltipPsych_.right = split;
        tooltipPsych_.bottom = bottom;
        int right = MainInterfaceGlobal.lower_frame_rect().right - 7;
        tooltipLabs_.left = split + 3;
        tooltipLabs_.top = top;
        tooltipLabs_.right = right;
        tooltipLabs_.bottom = bottom;
        tutRect9_.left = MainInterfaceGlobal.main_frame_rect().left + 4;
        tutRect9_.top = MainInterfaceGlobal.main_frame_rect().top + 4;
        tutRect9_.right = MainInterfaceGlobal.main_frame_rect().right - 4;
        tutRect9_.bottom = tooltipLabs_.top - 6;
        tooltipEffects_ = MainInterfaceGlobal.right_frame_rect();
        tooltipEffects_.left += 4;
        tooltipEffects_.right -= 4;
        tooltipEffects_.top += 4;
        tooltipEffects_.bottom -= 4;
    }

    // The three category frames and their header strips, inside
    // tutRects1_[3..8].
    RECT *frame = &tutRects1_[3];
    frame[0].left = tooltipEconomy_.left + 3;
    frame[0].top = tooltipEconomy_.top + 3;
    frame[0].right = tooltipEconomy_.left + 0x17;
    frame[0].bottom = tooltipEconomy_.top + 0x17;
    frame[1].left = tooltipEconomy_.right - 0x17;
    frame[1].top = tooltipEconomy_.top + 3;
    frame[1].right = tooltipEconomy_.right - 3;
    frame[1].bottom = tooltipEconomy_.top + 0x17;
    frame[2].left = tooltipPsych_.left + 3;
    frame[2].top = tooltipPsych_.top + 3;
    frame[2].right = tooltipPsych_.left + 0x17;
    frame[2].bottom = tooltipPsych_.top + 0x17;
    frame[3].left = tooltipPsych_.right - 0x17;
    frame[3].top = tooltipPsych_.top + 3;
    frame[3].right = tooltipPsych_.right - 3;
    frame[3].bottom = tooltipPsych_.top + 0x17;
    frame[4].left = tooltipLabs_.left + 3;
    frame[4].top = tooltipLabs_.top + 3;
    frame[4].right = tooltipLabs_.left + 0x17;
    frame[4].bottom = tooltipLabs_.top + 0x17;
    frame[5].left = tooltipLabs_.right - 0x17;
    frame[5].top = tooltipLabs_.top + 3;
    frame[5].right = tooltipLabs_.right - 3;
    frame[5].bottom = tooltipLabs_.top + 0x17;

    tutRects1_[0].left = tooltipEconomy_.left + 3;
    tutRects1_[0].top = tooltipEconomy_.bottom - 0x11;
    tutRects1_[0].right = tooltipEconomy_.left + 0x1b;
    tutRects1_[0].bottom = tooltipEconomy_.bottom - 3;
    tutRects1_[1].left = tooltipPsych_.left + 3;
    tutRects1_[1].top = tooltipPsych_.bottom - 0x11;
    tutRects1_[1].right = tooltipPsych_.left + 0x1b;
    tutRects1_[1].bottom = tooltipPsych_.bottom - 3;
    tutRects1_[2].left = tooltipLabs_.left + 3;
    tutRects1_[2].top = tooltipLabs_.bottom - 0x11;
    tutRects1_[2].right = tooltipLabs_.left + 0x1b;
    tutRects1_[2].bottom = tooltipLabs_.bottom - 3;

    font1_.init(DefaultFontFace, 0xe, 1);
    font2_.init(DefaultFontFace, 0xe, 1);
    font3_.init(DefaultLargeFontFace, 0xe, 1);
    font4_.init(DefaultLargeFontFace, 0xe, 1);
    font5_.init(DefaultFontFace, 0xc, 0);
    font6_.init(DefaultFontFace, 0xb, 1);
    font6_.unk_1_ = 3;
    font7_.init(DefaultFontFace, 0xb, 0);
    font7_.unk_1_ = 3;
    font8_.init(DefaultLargeFontFace, 0xe, 1);
    font9_.init(DefaultLargeFontFace, 0xe, 1);

    if (!effectIcons_[0].has_pixel_pointer()) {
        sheet.load_pcx(SocialWinPcxName, 0, 10, 0xec);
        Sprite *icon = effectIcons_;
        int row = 1;
        for (int r = 0; r < 10; ++r) {
            int col = 1;
            for (int c = 0; c < 9; ++c) {
                icon->extract(&sheet, WinFillColour, col, row, 0x1a, 0x1a, 0);
                ++icon;
                col += 0x1b;
            }
            row += 0x1b;
        }
        effectIcons_[0].extract(&sheet, WinFillColour, 0xf5, 1, 0x18, 0xe, 0);
        effectIcons_[0].extract(&sheet, WinFillColour, 0xf5, 0x10, 0x18, 0xe, 0);
        for (int x = 1; x < 0x40; x += 0x15) {
            energyAllocLock_[0].extract(&sheet, WinFillColour, 0x13b, x, 0x14, 0x14, 0);
            energyAllocArrow_[0].extract(&sheet, WinFillColour, 0x150, x, 0x14, 0x14, 0);
        }
        {
            int y = 0x10e;
            Buffer *slider = energyAllocSlider_;
            for (int r = 0; r < 3; ++r) {
                int x = 1;
                for (int c = 0; c < 2; ++c) {
                    slider->init(0xe, 0xe, 0, 0);
                    // The original's call pushes seven arguments; the two
                    // trailing parameters read stack residue.
                    sheet.copy(slider, y, x, 0, 0, 0xe, 0xe);
                    x += 0xf;
                    ++slider;
                }
                y += 0xf;
            }
        }
    }
    {
        int y = 0x115;
        for (int i = 0; i < 5; ++i) {
            socEngModifierNums_[i].extract(&sheet, WinFillColour, y, 0x6f, 0xd, 0xd, 0);
            socEngModifierNums_[1 + i].extract(&sheet, WinFillColour, y, 0x7d, 0xd, 0xd, 0);
            y += 0xe;
        }
    }

    {
        int cx = (WinScreenWidth - 0x321) / 2;
        // Original 0x007AF59C is MainInterface::field_D7C_ at +0xD7C.
        int cy = (WinScreenHeight - MainInterfaceGlobal.panel_height()) - 0x162;
        GraphicWin::init(cx, cy, 0x341, 0x162, 0, 0x1000000,
            static_cast<Win *>(&ConsoleGlobal), 0, 0);
    }
    GraphicWin::fill(9);
    {
        RECT box_rect;
        box_rect.left = 0x17;
        box_rect.top = 0;
        box_rect.right = 0x30a;
        box_rect.bottom = 0x162;
        static_cast<Buffer *>(static_cast<GraphicWin *>(this))->box_sprite(&box_rect,
            &InterfaceBoxStyles[9]);
    }
    {
        int net_min = 10 - PlayersData[factionID_].se_alloc_labs
            - PlayersData[factionID_].se_alloc_psych;
        savedAlloc_[0] = net_min;
        pendingAlloc_[0] = net_min;
        currentAlloc_[0] = net_min;
        int lo = PlayersData[factionID_].se_alloc_psych;
        savedAlloc_[1] = lo;
        pendingAlloc_[1] = lo;
        currentAlloc_[1] = lo;
        int hi = PlayersData[factionID_].se_alloc_labs;
        savedAlloc_[2] = hi;
        pendingAlloc_[2] = hi;
        currentAlloc_[2] = hi;
    }
    reset();

    {
        ButtonGroup *group = buttonGroups_;
        int row = 0;
        int row_accum = 0;
        do {
            group->init(row, 0);
            for (int col = 0; col < 4; ++col) {
                if (society_avail(row, col, factionID_) == 0) {
                    continue;
                }
                StringTemp[0] = 0;
                // Preserve the LPSTR-to-string-table validation path for the
                // category name pointers stored by Strings::put.
                strcat(StringTemp, StringTable.get(
                    SocialCategories[row].name[col]));
                CharUpperA(StringTemp);
                int idx = col + row * 4;
                RECT *cell = &modelRects_[idx];
                FlatButton *button = &flatButtons_[idx];
                static_cast<BaseButton *>(button)->init(StringTemp, idx,
                    cell->left, cell->top, cell->right - cell->left,
                    cell->bottom - cell->top, this, 0);
                button->hide();
                static_cast<Buffer *>(button)->set_font(&font2_, 0, 0, 0);
                button->redraw();
                static_cast<BaseButton *>(button)->set_bubble_text(
                    StringTable.get(Labels.strings_ptr[0xCF0 / 4]));
                group->add(static_cast<BaseButton *>(button));
            }
            group->set(social_category_value(socCategory_, row), 1);
            ++row;
            row_accum += 0x35;
            ++group;
        } while (row_accum < 0xd4);
    }

    buttonGroup_.init(2, 0);
    {
        int row = 0;
        int button_offset = 0;
        int entry = 0;
        while (row < 11) {
            if (row != 3) {
                const char *name = SocialEffects[row].set2;
                int col;
                int offset;
                if (row < 3) {
                    col = row * 4;
                    offset = button_offset;
                } else {
                    col = row * 4 - 4;
                    entry -= 1;
                    offset = button_offset - 0xB4C;
                }
                StringTemp[0] = 0;
                strcat(StringTemp, name);
                CharUpperA(StringTemp);
                RECT *cell = &tutRects2_[entry];
                int y = cell->top;
                int x = cell->left;
                FlatButton *flat_button = &flatButtons_[
                    17 + offset / static_cast<int>(sizeof(FlatButton))];
                BaseButton *button = static_cast<BaseButton *>(flat_button);
                button->init(StringTemp, col + 0x11, x, y,
                    cell->right - x, cell->bottom - y, this, 0);
                button->hide();
                static_cast<Buffer *>(flat_button)
                    ->set_font(reinterpret_cast<Font *>(0), 0, 0, 0);
                button->redraw();
                button->set_bubble_text(
                    StringTable.get(Labels.strings_ptr[0xCF4 / 4 + col]));
            }
            ++row;
            entry += 1;
            button_offset += 0xB4C;
        }
    }

    {
        StringTemp[0] = 0;
        strcat(StringTemp, StringTable.get(Labels.strings_ptr[0x198 / 4]));
        if (Language != 2) {
            CharUpperA(StringTemp);
        }
        FlatButton *button = &flatButtons_[16];
        static_cast<BaseButton *>(button)->init(StringTemp, 0x10,
            tutRect11_.left, tutRect11_.top, tutRect11_.right - tutRect11_.left,
            tutRect11_.bottom - tutRect11_.top, this, 0);
        button->hide();
        static_cast<Buffer *>(button)->set_font(reinterpret_cast<Font *>(0), 0, 0, 0);
        button->redraw();
    }

    // Initialize SocialWin flat buttons 27 and 28 from their separate labels.
    {
        StringTemp[0] = 0;
        strcat(StringTemp, StringTable.get(Labels.strings_ptr[0x4F4 / 4]));
        CharUpperA(StringTemp);
        FlatButton *button = &flatButtons_[27];
        BaseButton *base_button = static_cast<BaseButton *>(button);
        base_button->init(StringTemp, 0x1B,
            tutRect12_.left, tutRect12_.top,
            tutRect12_.right - tutRect12_.left,
            tutRect12_.bottom - tutRect12_.top, this, 0);
        base_button->hide();
        static_cast<Buffer *>(button)->set_font(&font2_, 0, 0, 0);
        base_button->redraw();
        base_button->set_bubble_text(
            StringTable.get(Labels.strings_ptr[0xD34 / 4]));
    }

    {
        StringTemp[0] = 0;
        strcat(StringTemp, StringTable.get(Labels.strings_ptr[0x194 / 4]));
        CharUpperA(StringTemp);
        FlatButton *button = &flatButtons_[28];
        BaseButton *base_button = static_cast<BaseButton *>(button);
        base_button->init(StringTemp, 0x1C,
            tutRect13_.left, tutRect13_.top,
            tutRect13_.right - tutRect13_.left,
            tutRect13_.bottom - tutRect13_.top, this, 0);
        base_button->hide();
        static_cast<Buffer *>(button)->set_font(&font2_, 0, 0, 0);
        base_button->redraw();
    }

    {
        CheckButton *lock0 = &energyLockButtons_[0];
        CheckButton *lock1 = &energyLockButtons_[1];
        CheckButton *lock2 = &energyLockButtons_[2];
        // Each lock links its own sprite pointers to the energy lock and
        // arrow runs.
        lock0->unchecked_sprite_ = &energyAllocLock_[0];
        lock0->checked_sprite_ = &energyAllocLock_[1];
        lock0->init(tutRects1_[0].left,
            tutRects1_[0].top, &MainInterfaceGlobal, 0);
        lock1->unchecked_sprite_ = &energyAllocLock_[0];
        lock1->checked_sprite_ = &energyAllocLock_[1];
        lock1->init(tutRects1_[1].left,
            tutRects1_[1].top, &MainInterfaceGlobal, 0);
        lock2->unchecked_sprite_ = &energyAllocLock_[0];
        lock2->checked_sprite_ = &energyAllocLock_[1];
        lock2->init(tutRects1_[2].left,
            tutRects1_[2].top, &MainInterfaceGlobal, 0);

        lock0->hide();
        lock1->hide();
        lock2->hide();
    }

    // These six receivers are FlatButton elements 29..34; their original
    // 0xB4C stride is sizeof(FlatButton). Each borrows its arrow triplet.
    static const int toggle_bubbles[6] = { 0xD24, 0xD24, 0xD28, 0xD28, 0xD2C, 0xD2C };
    for (int toggle_index = 0; toggle_index < 6; ++toggle_index) {
        FlatButton *flat_toggle = &flatButtons_[29 + toggle_index];
        BaseButton *toggle = static_cast<BaseButton *>(flat_toggle);
        const RECT &rect = tutRects1_[3 + toggle_index];
        toggle->init(0, 0x1d + toggle_index, rect.left, rect.top,
            rect.right - rect.left, rect.bottom - rect.top,
            &MainInterfaceGlobal, 0);
        toggle->hide();
        int first = (toggle_index % 2 == 0) ? 0 : 3;
        flat_toggle->set_report_icons(&energyAllocArrow_[first + 0],
            &energyAllocArrow_[first + 1], &energyAllocArrow_[first + 2]);
        toggle->set_bubble_text(StringTable.get(
            Labels.strings_ptr[toggle_bubbles[toggle_index] / 4]));
    }

    if (positive == 0) {
        if (tut_check2(8) != 0) {
            show(0);
            TutorialWindow.set_tour_stage(5);
            int out_a = 0;
            int out_b = 0;
            TutorialWindow.soc_rect(&TutWinSocWindow.categoryHeaders_[0], &out_a, &out_b);
            MapWin *window = MapWinTable[0];
            GraphicWin *graphics = window != 0
                ? static_cast<GraphicWin *>(window) : 0;
            TutorialWindow.tut_win(graphics, SocialWinTutorialName, out_a, out_b,
                0, 1, -1, -1);
        }
        set_modal(0, pop_wait, &MainInterfaceGlobal);
        release_iface_mode();
    }
}

// Original interface table 0x0066DA50 slot 1 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_iface_scrolled@SocialWin@@UAEXHH@Z 0x004B2120
void SocialWin::on_iface_scrolled(int, int) {}
// Original interface table 0x0066DA50 slot 2 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_iface_scrolling@SocialWin@@UAEXHH@Z 0x004B2120
void SocialWin::on_iface_scrolling(int, int) {}
// Original interface table 0x0066DA50 slot 6 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_iface_left_up@SocialWin@@UAEXHH@Z 0x004B2120
void SocialWin::on_iface_left_up(int, int) {}

// Original interface table 0x0066DA50 slot 14 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404250 0x00404255 ?on_iface_button_pressed@SocialWin@@UAEHH@Z 0x004B2120
int SocialWin::on_iface_button_pressed(int) {
    return 0;
}

// Original interface table 0x0066DA50 slot 17 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404250 0x00404255 ?on_iface_dialog_back_draw@SocialWin@@UAEHPAVGraphicWin@@@Z 0x004B2120
int SocialWin::on_iface_dialog_back_draw(GraphicWin *) {
    return 0;
}

// Original interface table 0x0066DA50 slot 18 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404240 0x00404245 ?on_iface_dialog_item_back_draw@SocialWin@@UAEHPAVGraphicWin@@HHPAUtagRECT@@@Z 0x004B2120
int SocialWin::on_iface_dialog_item_back_draw(GraphicWin *, int, int, RECT *) {
    return 0;
}

// C++ owns member/base teardown after these explicit close/reset operations.
// ORIGINAL: 0x004B3C80 ??1SocialWin@@QAE@XZ 0x004B3C80-0x004B3EB7;0x00659700-0x00659878 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical seed:src/recovered/units/004b3c80.cpp", "task:socialwin-lifetime-owner", "Pinned4B3C80:35 virtual FlatButton closes; four ButtonGroup closes then standalone close; resets+A2C,+A30,+2C28,+2C24.", "Automatic reverse teardown: nine Fonts, standalone group, ButtonGroup[4], FlatButton[35], Buffer[6], Sprite[6/2/90/10], CheckButton[3], Spot, GraphicWin. Constructor4AE9E0 and destructor agree CheckButton starts+D6C, strideA2C; no complete layout or runtime acceptance claim."], "reason": "Recover complete SocialWin destruction through actual embedded members and bases, not an executable-address forwarder or explicit destructor cascade."}
// symbol    ??1SocialWin@@UAE@XZ
SocialWin::~SocialWin() {
    for (int i = 0; i < 35; ++i) {
        flatButtons_[i].close();
    }
    for (int j = 0; j < 4; ++j) {
        buttonGroups_[j].close();
    }
    buttonGroup_.close();
    hover_selection_ = -1;
    effect_hover_button_id_ = -1;
    field_2C28_ = 0;
    effectMaximize_ = -1;
}
