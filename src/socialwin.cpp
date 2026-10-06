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
#include "stdafx.h"
#include "socialwin.h"
#include "win.h"
#include "subinterface.h"
#include "vector_teardown.h"
#include "game.h"
#include "strings.h"
#include "text.h"
#include "faction.h"
#include "base.h"
#include "technology.h"
#include "lock.h"
#include "alpha.h"
#include "spying_recovery.h"
#include "mapwin.h"   // MainInterfaceGlobal, at 0x007AE820
#include "tutwin.h"
#include "console.h"
#include "netdaemon.h"
#include "popup.h"

extern int BreakthroughTurnsGlobal;
extern int NetIncomeGlobal;

// ORIGINAL: 0x004AEF10 ?exit@SocialWin@@QAEXXZ 0x004AEF10-0x004AF10C BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/004aef10.cpp", "image:terranx_original.exe:0x004AEF10", "Original instructions establish owner, bounded body and callback/state order; SocialWin reloads faction after social_upheaval and every later stateful call. GraphicWin dispatches through the real Buffer base.", "recovery/tasks/interface-contract-report-closure.json; bounded UI-five scope preserves original baseline"], "reason": "Recover the original in its genuine UI owner using actual members, globals and base/virtual calls. Retain seed evidence and historical obligations; this binding does not grant a current matching or runtime claim. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
void SocialWin::exit() {
    const uint32_t gross_upheaval = social_upheaval(factionID_, &socCategory_);
    int upheaval = static_cast<int>(
        gross_upheaval - PlayersData[factionID_].soc_upheaval_cost_paid);

    if ((GameState & STATE_SCENARIO_EDITOR) == 0
        || (GameState & STATE_OMNISCIENT_VIEW) == 0) {
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

void __fastcall SocialWinFlatButtonCtor(void *self) { reinterpret_cast<FlatButton *>(self)->FlatButton::FlatButton(); }
void __fastcall SocialWinFlatButtonDtor(void *self) { reinterpret_cast<FlatButton *>(self)->~FlatButton(); }
void __fastcall SocialWinCheckButtonCtor(void *self) { reinterpret_cast<CheckButton *>(self)->CheckButton::CheckButton(); }
void __fastcall SocialWinCheckButtonDtor(void *self) { reinterpret_cast<CheckButton *>(self)->~CheckButton(); }
void __fastcall SocialWinButtonGroupCtor(void *self) { reinterpret_cast<ButtonGroup *>(self)->ButtonGroup::ButtonGroup(); }
void __fastcall SocialWinButtonGroupDtor(void *self) { reinterpret_cast<ButtonGroup *>(self)->~ButtonGroup(); }

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
// ORIGINAL: 0x004AE9E0 ??0SocialWin@@QAE@XZ 0x004AE9E0-0x004AEBE7;0x006598A0-0x00659A0A
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
// ORIGINAL: 0x004B2220 ?on_iface_right_click@SocialWin@@QAEXHH@Z 0x004B2220-0x004B23FA
// RECOVERY_BINDING: v1 {"evidence": ["Original4B2220..4B23FA first resets TutorialWindow then tests interface-relative1E84C/1E85C/1E86C/1E83C, which are complete-object rectangles1F260/1F270/1F280/1F250. Labels.strings_ptr offsetsD24/D28/D2C/D1C select labels841/842/843/839; first match wins.", "Original4B2256/59 swaps argument stack contents before passing their reversed slot addresses to MainInterfaceGlobal.client_to_screen; this preserves logical x/y rather than the swapped coordinates in the seed. Every matching path constructs a degenerate screen rectangle and calls set_bubble_text on7AE820; no match only dismisses the tutorial."], "reason": "Recover social-panel tooltip selection with genuine rectangle members, label storage and the main-interface window receiver."}
// symbol    ?on_iface_right_click@SocialWin@@UAEXHH@Z
void SocialWin::on_iface_right_click(int x, int y) {
    TutorialWindow.reset();
    int label;
    if (in_box(x, y, &tooltipEconomy_)) {
        label = 841;
    } else if (in_box(x, y, &tooltipLabs_)) {
        label = 842;
    } else if (in_box(x, y, &tooltipPsych_)) {
        label = 843;
    } else if (in_box(x, y, &tooltipEffects_)) {
        label = 839;
    } else {
        return;
    }
    char *text = label_get(label);
    // The original reuses its argument slots after swapping their contents;
    // the resulting conversion is still x then y, on the main interface.
    MainInterfaceGlobal.client_to_screen(&x, &y);
    RECT rectangle = {x, y, x, y};
    MainInterfaceGlobal.Win::set_bubble_text(text, &rectangle);
}

// ORIGINAL: 0x004B1B20 ?on_left_click@SocialWin@@QAEHHH@Z 0x004B1B20-0x004B1B2D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["The 0x004B1B20 body ignores its coordinates and returns TutWin::reset at 0x004BDDE0 on the process-lifetime object at 0x008C6E68.", "src/tutwin.cpp owns that object as TutorialWindow and its reset service returns int."], "reason": "Home the SocialWin click dismissal through the typed tutorial singleton."}
int SocialWin::on_left_click(int, int) {
    return TutorialWindow.reset();
}

// ORIGINAL: 0x004B1BC0 ?on_right_click@SocialWin@@QAEXHH@Z 0x004B1BC0-0x004B1CD1 FILE
// RECOVERY_BINDING: v1 {"evidence": ["The 0x004B1BC0 body resets TutorialWindow, probes four contiguous category rectangles beginning at +0x1F110 then the faction-power rectangle at +0x1F2A0, and retrieves label slots 828 or 840.", "The existing SocialWin layout has those rectangles and the on_iface_right_click body establishes label_get, client_to_screen and set_bubble_text as the typed tooltip path."], "reason": "Recover the SocialWin right-click tooltip through real rectangle members and source-owned labels."}
void SocialWin::on_right_click(int x, int y) {
    TutorialWindow.reset();
    RECT *categories[4] = {&tooltipSocEng_, &tutRect4_,
                           &tutRects3_[0], &tutRects3_[1]};
    int label = -1;
    for (int i = 0; i < 4; ++i) {
        if (in_box(x, y, categories[i])) {
            label = 828;
            break;
        }
    }
    if (label < 0 && in_box(x, y, &tooltipFactionPower_)) {
        label = 840;
    }
    if (label < 0) {
        return;
    }
    char *text = label_get(label);
    client_to_screen(&x, &y);
    RECT rectangle = {x, y, x, y};
    Win::set_bubble_text(text, &rectangle);
}

// ORIGINAL: 0x004B1FC0 ?on_mouse_leave@SocialWin@@QAEXHH@Z 0x004B1FC0-0x004B1FEC FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["The 0x004B1FC0 body calls Win::clear_bubble_text twice, then checks the signed hover value at +0xA2C and redraws mouseover only while it is nonnegative.", "The sibling on_iface_mouse_leave already proves the double clear is intentional, and draw_mouseover uses field_A2C_ as its hover selection."], "reason": "Home the mouse-leave path through the existing hover member and typed Win service."}
// symbol    ?on_mouse_leave@SocialWin@@UAEXHH@Z
void SocialWin::on_mouse_leave(int, int) {
    ::Win::clear_bubble_text();
    ::Win::clear_bubble_text();
    if (static_cast<int>(field_A2C_) >= 0) {
        field_A2C_ = 0xFFFFFFFFU;
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
    field_A2C_ = 0xFFFFFFFFU;
    field_A30_ = 0xFFFFFFFFU;
    effectMaximize_ = 0xFFFFFFFFU;
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
// ORIGINAL: 0x004B2470 ?on_iface_button_clicked@SocialWin@@QAEXH@Z 0x004B2470-0x004B264E FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004b2470.cpp retained original identity and spans", "recovery/tasks/interface-contract-report-closure.json authorized ownership recovery", "original:0x004B2470 complete control/member accesses and concrete interface table"], "reason": "Recover the concrete SubInterface owner operation through genuine window members and source-owned globals. Original table/call evidence identifies the callback receiver; the retained compatible seed supplies its complete original intervals, not an executable-address forwarding implementation."}
// symbol    ?on_iface_button_clicked@SocialWin@@UAEXH@Z
void SocialWin::on_iface_button_clicked(int button_id) {
    TutWinGlobal->reset();

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

    uint32_t *allocations[3] = {
        &economyAlloc3_, &psychAlloc3_, &labsAlloc3_
    };
    uint32_t &allocation = *allocations[index];
    const int limit = energy_limit(static_cast<int>(factionID_));
    allocation = static_cast<uint32_t>(static_cast<int>(allocation) + delta);
    if (static_cast<int>(allocation) < 0) allocation = 0;
    if (static_cast<int>(allocation) > limit) {
        field_2C5C_ = 1;
        allocation = static_cast<uint32_t>(
            energy_limit(static_cast<int>(factionID_)));
    }

    allocate_energy(index, static_cast<int>(allocation));

    economyAlloc3_ = economyAllocPending_;
    psychAlloc3_ = psychAllocPending_;
    labsAlloc3_ = labsAllocPending_;

    PlayerData &player = PlayersData[factionID_];
    player.se_alloc_psych = static_cast<int>(psychAllocPending_);
    player.se_alloc_labs = static_cast<int>(labsAllocPending_);
    social_set(factionID_);

    social_calc(&socCategory_, &socEffect_, factionID_, 0, 0);
    social_calc(&socCategory_, &player.soc_effect_pending, factionID_, 0, 0);
    energy_compute(factionID_, 1);

    breakthroughTurns_ = BreakthroughTurnsGlobal;
    netIncome_ = NetIncomeGlobal;
    social_calc(&player.soc_category_pending, &player.soc_effect_pending,
                factionID_, 0, 0);
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
    if (!reinterpret_cast<Win *>(this)->is_visible()) {
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
    if (reinterpret_cast<Win *>(this)->is_visible()) {
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

/*
 * Fixed-slot bindings the homed bodies arrived carrying, named from evidence.
 * Each is an `int *const` at file scope, which folds to the bare immediate, so
 * a rename moves no byte; the cast to the real type stays at the point of use.
 *
 * Upper-case operations use the normal CharUpperA import. Original
 * 0x0066931C is its loader-owned IAT slot, not a private function table.
 * Panel styles now borrow shared mutable InterfaceBoxStyles elements 3/6;
 * their original starts 0x78D5A0/0x78D618 are no longer address aliases.
 * InterfaceBuffer    0x007AEC64, the shared full-screen drawing Buffer. 55
 *                    archive sites cast it to `Buffer *` and draw through it -
 *                    DesignWin::draw_info, Datalink, DiploPop::start, ReportIf,
 *                    this file. 0x009B7490 is palette.h's ScreenBuffer, a
 *                    different object.
 * InterfaceClipRect  0x007AEC94 = InterfaceBuffer + 0x30, the RECT every one of
 *                    those bodies hands to Buffer::set_clip - the buffer's own
 *                    clip rectangle, not an unrelated global.
 * SpaceText          0x00682820 - image .data reads " " (one space). technology
 *                    .cpp already names it in prose for the same reason.
 * PercentText        0x00682EA0 - image .data reads "%".
 * SocialEffectTable  0x0094B024 - indexed [category * 53 + row * 11 + effect]
 *                    with MaxSocialEffectNum == 11; a negative entry marks an
 *                    effect the faction cannot take.
 * SocialEffectRow    0x0094B2CC - indexed [effect] the same way, and the walk
 *                    in draw_psych_alloc steps it three entries at a time.
 * SocialEffectRowEnd SocialEffectRow + 3 - the walk's own end sentinel.
 * SocBaseIdProbe / SocBaseIdCompare - 0x009A6568 / 0x009A6574. Both are an int
 *                    base id read the same way: `>= 0`, then
 *                    Bases[id].faction_id_current compared to
 *                    the faction, i.e. "does this base belong to us". immune()
 *                    uses network_backbone and cloning_vats project locations.
 * SocialAllocTable   0x0096CD04 - per-faction, stride 2099 like the other
 *                    faction social arrays in SocialWin::exit; allocate_energy
 *                    reads the allocation it scales by 10 for display.
 */
static int *const StringUpperSlot = (int *)0x0066931C;
static int *const SocialEffectTable = (int *)0x0094B024;
static int *const SocialEffectRow = (int *)0x0094B2CC;
static int *const SocialEffectRowEnd = (int *)0x0094B2D8;
// Historical SocBaseIdProbe/Compare at 0x009A6568/0x009A6574 are
// SecretProject.network_backbone/cloning_vats, not independent selectors.

// ORIGINAL: 0x004AF310 ?draw_social@SocialWin@@QAEXH@Z 0x004AF310-0x004AF8BC FILE
// TRIED: transcribed directly from the disassembly (Ghidra's own decompilation agreed on the control flow and formulas once its confusing overlapping-stack-slot local numbering was resolved against the raw ebp-relative offsets) - the header/derived RECT pair, the per-row avail==0 header-only path, the count-then- draw two-pass loop over the fixed 11-entry table at 0x94b2cc (index 3 excluded), and the `this!=0 ? obj : 0` guard before each Sprite::draw. Draw receivers are the big/small Sprite sub-objects themselves (`bigIcon->draw(buffer, ...)`), not the Buffer - an early draft called the wrong receiver and was fixed before landing. MISMATCH #0 (`push`/`sub` prologue) on a 1452-byte function with a deeply nested sign-magnitude classification loop - not chased to byte-exactness, this is a structural transcription.
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
    Buffer *buffer = static_cast<Buffer *>(static_cast<GraphicWin *>(this));
    const int faction = factionID_;

    RECT *headerSlot;
    switch (a1) {
    case 0: headerSlot = &tooltipSocEng_; break;
    case 1: headerSlot = &tutRect4_; break;
    case 2: headerSlot = &tutRects3_[0]; break;
    default: headerSlot = &tutRects3_[1]; break;
    }
    RECT rect1 = *headerSlot;
    buffer->set_clip(&rect1);
    buffer->box_sprite(&rect1, &InterfaceBoxStyles[3]);

    RECT rect2;
    rect2.left = rect1.left + 3;
    rect2.top = rect1.top + 3;
    rect2.right = rect1.right - 3;
    rect2.bottom = rect1.top + 3 + 0x14;
    buffer->box_sprite(&rect2, &InterfaceBoxStyles[6]);

    StringTemp[0] = 0;
    char *name = StringTable.get(reinterpret_cast<int>(SocialCategories[a1].type));
    strcat(StringTemp, name);
    buffer->set_text_color(0x9f, -1, 1, 1);
    buffer->set_font(&font1_, (Font *)0, (Font *)0, (Font *)0);
    CharUpperA(StringTemp);
    char *heading = StringTemp;
    if (heading != 0) {
        unsigned int len = strlen(heading);
        buffer->write_cent_l(heading, &rect2, len);
    }

    for (int row = 0; row < 4; row++) {
        RECT *slot = &tutRects3_[2 + a1 * 4 + row];
        if (society_avail(a1, row, faction) == 0) {
            RECT hdr = *slot;
            buffer->box_sprite(&hdr, &InterfaceBoxStyles[6]);
            hdr.top += 0x16;
            hdr.bottom += 0x16;
            buffer->box_sprite(&hdr, &InterfaceBoxStyles[6]);
            continue;
        }

        RECT body = *slot;
        body.top += 0x16;
        body.bottom += 0x16;
        buffer->box_sprite(&body, &InterfaceBoxStyles[6]);

        int countNonzero = 0;
        int countBig = 0;
        int idx = 0;
        int *p = SocialEffectRow;
        while (p != SocialEffectRowEnd) {
            if (p != SocialEffectRow + 3) {
                bool skip = false;
                if (SecretProject.cloning_vats() >= 0 &&
                    (unsigned int)Bases[SecretProject.cloning_vats()].faction_id_current == (unsigned int)faction) {
                    if (a1 == 2) {
                        if (row == 1) {
                            if (SocialEffectTable[a1 * 0x35 + idx + row * 0xb] < 0) {
                                skip = true;
                            }
                        }
                    } else if (a1 == 3 && row == 3) {
                        if (SocialEffectTable[a1 * 0x35 + idx + row * 0xb] < 0) {
                            skip = true;
                        }
                    }
                }
                if (!skip) {
                    if (!((SecretProject.network_backbone() < 0) ||
                          ((unsigned int)Bases[SecretProject.network_backbone()].faction_id_current != (unsigned int)faction) ||
                          (a1 != 3) ||
                          ((row != 1) || (*p >= 0)))) {
                        skip = true;
                    }
                }
                if (!skip) {
                    int v = SocialEffectTable[a1 * 0x35 + idx + row * 0xb];
                    int mag = (v < 0) ? -v : v;
                    countNonzero += (mag != 0) ? 1 : 0;
                    countBig += (v < 0 ? -v : v) > 1 ? 1 : 0;
                }
            }
            idx++;
            p++;
        }
        if (countNonzero < 1) {
            countNonzero = 1;
        }

        int drawX = (body.right - (effectIcons_[0].width() + 1) * countNonzero -
                     socEngModifierNums_[0].source_width() * countBig - body.left) / 2 + body.left;
        int drawYBase = (body.bottom - effectIcons_[0].width() - body.top) / 2 + body.top;

        for (int phase = 0; phase < 3; phase++) {
            int drawX2 = drawX;
            int idx2 = 0;
            int *p2 = SocialEffectRow;
            while (p2 != SocialEffectRowEnd) {
                if (p2 != SocialEffectRow + 3) {
                    bool skip = false;
                    if (SecretProject.cloning_vats() >= 0 &&
                        (unsigned int)Bases[SecretProject.cloning_vats()].faction_id_current == (unsigned int)faction) {
                        if (a1 == 2) {
                            if (row == 1) {
                                if (SocialEffectTable[a1 * 0x35 + idx2 + row * 0xb] < 0) {
                                    skip = true;
                                }
                            }
                        } else if (a1 == 3 && row == 3) {
                            if (SocialEffectTable[a1 * 0x35 + idx2 + row * 0xb] < 0) {
                                skip = true;
                            }
                        }
                    }
                    if (!skip) {
                        if (!((SecretProject.network_backbone() < 0) ||
                              ((unsigned int)Bases[SecretProject.network_backbone()].faction_id_current != (unsigned int)faction) ||
                              (a1 != 3) ||
                              ((row != 1) || (*p2 >= 0)))) {
                            skip = true;
                        }
                    }
                    if (!skip) {
                        int v = SocialEffectTable[a1 * 0x35 + idx2 + row * 0xb];
                        int mag = (v < 0) ? -v : v;
                        bool draw = false;
                        if (phase == 0) {
                            draw = (mag > 1);
                        } else if (phase == 1) {
                            draw = (v == 1);
                        } else {
                            draw = (v < 0);
                        }
                        if (draw) {
                            Buffer *bufOrNull = (this != 0) ? buffer : (Buffer *)0;
                            if (mag > 1) {
                                int bigSpriteIndex = (v < 1 ? 1 : 0) + mag * 2 - 4;
                                Sprite *bigSprite = &socEngModifierNums_[bigSpriteIndex];
                                int y = body.bottom - bigSprite->height();
                                bigSprite->draw(bufOrNull, bigSprite->transparent_index(), drawX2, y, 1, 1);
                                drawX2 = drawX2 + 1 + bigSprite->source_width();
                            }
                            int *p2adj = p2;
                            if (p2 > SocialEffectRow + 10) {
                                p2adj = p2 - 9;
                            }
                            int smallIdx = (int)(p2adj - SocialEffectRow) + (v > 0 ? 1 : 0) + 1;
                            Sprite *smallSprite = &effectIcons_[6 + smallIdx];
                            smallSprite->draw(bufOrNull, smallSprite->transparent_index(), drawX2, drawYBase, 1, 1);
                            drawX2 = drawX2 + 1 + effectIcons_[0].width();
                        }
                    }
                }
                idx2++;
                p2++;
            }
        }
    }

    soft_update(headerSlot);
    do_sound();
}

// Fixed-slot bindings carried from 004b0d50.cpp
static int *const PercentText = (int *)0x00682EA0;
static int *const InterfaceBuffer = (int *)0x007AEC64;
static int *const InterfaceClipRect = (int *)0x007AEC94;
static int *const InterfacePaletteIndex = (int *)0x007AEC74;
static int *const SocialAllocTable = (int *)0x0096CD04;

typedef char *(__cdecl *ItoaFn)(int, char *, int);

typedef void (__cdecl *SoundHookFn)(char *);

// ORIGINAL: 0x004B19A0 ?on_redraw@SocialWin@@QAEXXZ 0x004B19A0-0x004B1B19
// RECOVERY_BINDING: v1 {"evidence": ["git:seed:src/recovered/units/004b19a0.cpp; source:src/socialwin.h:tooltipEconomy_,tooltipLabs_,tooltipPsych_,tutRect11_,tutRect12_,tutRect13_,flatButtons_; source:src/graphicwin.h:GraphicWin Buffer base; image:0x004B19A0-0x004B1B19"], "reason": "Recover SocialWin repaint orchestration through its real category/allocation rectangles, inherited GraphicWin Buffer and typed FlatButton child controls; retain the established panel ordering and child redraw sequence."}
// symbol    ?on_redraw@SocialWin@@UAEXXZ
void SocialWin::on_redraw() {
    Buffer *interface_buffer = reinterpret_cast<Buffer *>(InterfaceBuffer);
    interface_buffer->set_clip(reinterpret_cast<RECT *>(InterfaceClipRect));

    Buffer *window_buffer = static_cast<Buffer *>(static_cast<GraphicWin *>(this));
    window_buffer->restore_clip();
    MainInterfaceGlobal.restore_back(1);

    RECT allocation_panel;
    UnionRect(&allocation_panel, &tooltipLabs_, &tooltipPsych_);
    UnionRect(&allocation_panel, &allocation_panel, &tooltipEconomy_);
    allocation_panel.right += 3;
    allocation_panel.left -= 3;
    allocation_panel.bottom += 3;
    allocation_panel.top -= 3;
    interface_buffer->box_sprite(&allocation_panel, &InterfaceBoxStyles[3]);

    RECT tutorial_panel;
    UnionRect(&tutorial_panel, &tutRect11_, &tutRect13_);
    UnionRect(&tutorial_panel, &tutorial_panel, &tutRect12_);
    tutorial_panel.left -= 3;
    tutorial_panel.right += 3;
    tutorial_panel.bottom += 3;
    tutorial_panel.top -= 3;
    window_buffer->box_sprite(&tutorial_panel, &InterfaceBoxStyles[3]);

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

    for (int button = 0; button < 11; ++button) {
        if (button != 3) {
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
    Buffer *buf = reinterpret_cast<Buffer *>(InterfaceBuffer);
    buf->set_clip(reinterpret_cast<RECT *>(InterfaceClipRect));

    RECT local_rect = tooltipPsych_;
    buf->box_sprite(&local_rect, &InterfaceBoxStyles[6]);

    local_rect.left += 3;
    local_rect.top += 3;
    local_rect.right -= 3;
    local_rect.bottom -= 3;

    RECT box_rect;
    box_rect.left = local_rect.left + 0x17;
    box_rect.top = local_rect.top;
    box_rect.right = local_rect.right - 0x17;
    box_rect.bottom = local_rect.top + 0x14;
    buf->box(&box_rect, 0xbc, 0xbc);

    StringTemp[0] = 0;
    const int allocation = PlayersData[factionID_].se_alloc_psych;
    strcat(StringTemp, label_get(318));
    strcat(StringTemp, BufferJoinSpace);

    char numbuf[80];
    reinterpret_cast<ItoaFn>(_itoa)(allocation * 10, numbuf, 10);
    strcat(StringTemp, numbuf);
    strcat(StringTemp, reinterpret_cast<char *>(PercentText));

    CharUpperA(StringTemp);

    buf->set_font(&font1_, 0, 0, 0);
    buf->set_text_color(0x9f, -1, 1, 1);

    char *buf_str = StringTemp;
    if (buf_str) {
        int len = strlen(buf_str);
        buf->write_cent_l(buf_str, &box_rect, len);
    }

    int row_height = local_rect.bottom - energyAllocLock_[0].height();
    int start_x = local_rect.left + energyAllocLock_[0].width();
    int avail_width = local_rect.right - start_x;
    int scaled = (avail_width * allocation) / 5;
    int end_x = start_x + scaled;

    int adjusted_end_x = box_rect.right;
    if (end_x < local_rect.right) {
        adjusted_end_x = end_x + (allocation != 0 ? 1 : 0);
        int remaining_w = local_rect.right - end_x;
        int remaining_h = local_rect.bottom - row_height;
        buf->tile(&energyAllocSlider_[4], end_x, row_height, end_x, row_height,
                  remaining_w, remaining_h);
    }

    if (start_x < adjusted_end_x) {
        int width2 = adjusted_end_x - start_x;
        int height2 = box_rect.bottom - row_height;
        buf->tile(&energyAllocSlider_[5], start_x, row_height, start_x, row_height,
                  width2, height2);
    }

    MainInterfaceGlobal.soft_update(&tooltipPsych_);
    do_sound();
}

// Draw the unassigned energy share with the two slider buffers owned by the
// SocialWin. PlayerData is the source owner for the psych and lab shares.
// ORIGINAL: 0x004B08A0 ?draw_economy_alloc@SocialWin@@QAEXXZ 0x004B08A0-0x004B0AF2
// RECOVERY_BINDING: v1 {"evidence": ["git:seed:src/recovered/units/004b08a0.cpp; source:src/socialwin.h:tooltipEconomy_,energyAllocSlider_,energyAllocLock_; source:src/faction.h:PlayerData.se_alloc_psych,se_alloc_labs; image:0x004B08A0-0x004B0AF2"], "reason": "Home the economy allocation panel on SocialWin’s typed rectangles, slider buffers, font, and PlayerData allocation values."}
void SocialWin::draw_economy_alloc() {
    Buffer *buffer = reinterpret_cast<Buffer *>(InterfaceBuffer);
    buffer->set_clip(reinterpret_cast<RECT *>(InterfaceClipRect));

    RECT panel = tooltipEconomy_;
    buffer->box_sprite(&panel, &InterfaceBoxStyles[6]);
    panel.left += 3;
    panel.top += 3;
    panel.right -= 3;
    panel.bottom -= 3;

    RECT title = {panel.left + 0x17, panel.top,
                  panel.right - 0x17, panel.top + 0x14};
    buffer->box(&title, 0xbc, 0xbc);

    const PlayerData &player = PlayersData[factionID_];
    const int remaining = 10 - player.se_alloc_psych - player.se_alloc_labs;
    StringTemp[0] = '\0';
    strcat(StringTemp, label_get(366));
    strcat(StringTemp, BufferJoinSpace);
    char number[80];
    reinterpret_cast<ItoaFn>(_itoa)(remaining * 10, number, 10);
    strcat(StringTemp, number);
    strcat(StringTemp, reinterpret_cast<char *>(PercentText));
    CharUpperA(StringTemp);

    buffer->set_font(&font1_, 0, 0, 0);
    buffer->set_text_color(0x9f, -1, 1, 1);
    buffer->write_cent_l(StringTemp, &title, strlen(StringTemp));

    const int top = panel.bottom - energyAllocLock_[0].height();
    const int left = panel.left + energyAllocLock_[0].width();
    const int split = left + (panel.right - left) * remaining / 10;
    int right_split = panel.right;
    if (split < panel.right) {
        right_split = split + (remaining != 0);
        buffer->tile(&energyAllocSlider_[0], split, top, split, top,
                     panel.right - split, panel.bottom - top);
    }
    if (left < right_split) {
        buffer->tile(&energyAllocSlider_[1], left, top, left, top,
                     right_split - left, panel.bottom - top);
    }

    MainInterfaceGlobal.soft_update(&tooltipEconomy_);
    do_sound();
}

// Draw the labs share with the panel's own buffers and typed faction data.
// ORIGINAL: 0x004B0B00 ?draw_labs_alloc@SocialWin@@QAEXXZ 0x004B0B00-0x004B0D42
// RECOVERY_BINDING: v1 {"evidence": ["git:seed:src/recovered/units/004b0b00.cpp; source:src/socialwin.h:tooltipLabs_,energyAllocSlider_,energyAllocLock_; source:src/faction.h:PlayerData.se_alloc_labs; image:0x004B0B00-0x004B0D42"], "reason": "Home the labs allocation panel on SocialWin’s typed rectangles, slider buffers, font, and PlayerData lab allocation."}
void SocialWin::draw_labs_alloc() {
    Buffer *buffer = reinterpret_cast<Buffer *>(InterfaceBuffer);
    buffer->set_clip(reinterpret_cast<RECT *>(InterfaceClipRect));

    RECT panel = tooltipLabs_;
    buffer->box_sprite(&panel, &InterfaceBoxStyles[6]);
    panel.left += 3;
    panel.top += 3;
    panel.right -= 3;
    panel.bottom -= 3;

    RECT title = {panel.left + 0x17, panel.top,
                  panel.right - 0x17, panel.top + 0x14};
    buffer->box(&title, 0xbc, 0xbc);

    const int allocation = PlayersData[factionID_].se_alloc_labs;
    StringTemp[0] = '\0';
    strcat(StringTemp, label_get(60));
    strcat(StringTemp, BufferJoinSpace);
    char number[80];
    reinterpret_cast<ItoaFn>(_itoa)(allocation * 10, number, 10);
    strcat(StringTemp, number);
    strcat(StringTemp, reinterpret_cast<char *>(PercentText));
    CharUpperA(StringTemp);

    buffer->set_font(&font1_, 0, 0, 0);
    buffer->set_text_color(0x9f, -1, 1, 1);
    buffer->write_cent_l(StringTemp, &title, strlen(StringTemp));

    const int top = panel.bottom - energyAllocLock_[0].height();
    const int left = panel.left + energyAllocLock_[0].width();
    const int split = left + (panel.right - left) * allocation / 10;
    int right_split = panel.right;
    if (split < panel.right) {
        right_split = split + (allocation != 0);
        buffer->tile(&energyAllocSlider_[2], split, top, split, top,
                     panel.right - split, panel.bottom - top);
    }
    if (left < right_split) {
        buffer->tile(&energyAllocSlider_[3], left, top, left, top,
                     right_split - left, panel.bottom - top);
    }

    MainInterfaceGlobal.soft_update(&tooltipLabs_);
    do_sound();
}

// Paint the faction power strip from its owning Player name and base effects.
// ORIGINAL: 0x004AF8C0 ?draw_faction_basal@SocialWin@@QAEXXZ 0x004AF8C0-0x004AFB5D
// RECOVERY_BINDING: v1 {"evidence": ["git:seed:src/recovered/units/004af8c0.cpp; source:src/socialwin.h:tooltipFactionPower_,effectIcons_,font1_; source:src/faction.h:Player.noun_faction,noun_gender,is_noun_plural,PlayerData.soc_effect_base; image:0x004AF8C0-0x004AFB5D"], "reason": "Recover the faction-power panel on SocialWin typed rectangle, inherited Buffer, effect sprites, and Player records, including the noun-format defaults consumed by faction text rendering."}
void SocialWin::draw_faction_basal() {
    Buffer *buffer = static_cast<Buffer *>(static_cast<GraphicWin *>(this));
    RECT panel = tooltipFactionPower_;
    buffer->set_clip(&panel);
    buffer->box_sprite(&panel, &InterfaceBoxStyles[3]);

    panel.left += 3;
    panel.top += 3;
    panel.right -= 3;
    panel.bottom -= 3;
    RECT title = {panel.left, panel.top, panel.right, panel.top + 0x17};
    buffer->box_sprite(&title, &InterfaceBoxStyles[6]);

    const Player &player = Players[factionID_];
    GenderDefault = player.noun_gender;
    PluralityDefault = player.is_noun_plural;
    StringTemp[0] = '\0';
    strcat(StringTemp, player.noun_faction);
    CharUpperA(StringTemp);
    buffer->set_font(&font1_, 0, 0, 0);
    buffer->set_text_color(0x9f, -1, 1, 1);
    buffer->write_cent_l(StringTemp, &title, strlen(StringTemp));

    RECT bar = {panel.left, panel.bottom - 0x14, panel.right, panel.bottom};
    buffer->box_sprite(&bar, &InterfaceBoxStyles[6]);
    const SocialEffect &effects = PlayersData[factionID_].soc_effect_base;
    int total = 0;
    for (int effect = 0; effect < MaxSocialEffectNum; ++effect) {
        if (effect != 3) {
            const int value = social_effect_value(effects, effect);
            total += value < 0 ? -value : value;
        }
    }

    if (total != 0) {
        const int step = effectIcons_[0].width() + 2;
        int x = (bar.right - step * total - bar.left) / 2 + bar.left;
        int y = (bar.bottom - effectIcons_[0].height() - bar.top) / 2 + bar.top;
        for (int effect = 0; effect < MaxSocialEffectNum; ++effect) {
            if (effect == 3) {
                continue;
            }
            const int value = social_effect_value(effects, effect);
            const int count = value < 0 ? -value : value;
            const int row = effect > 2 ? effect - 1 : effect;
            const int sprite_index = 6 + row * 9 + (value > 0 ? 2 : 1);
            for (int icon = 0; icon < count; ++icon) {
                Sprite &sprite = effectIcons_[sprite_index];
                sprite.draw(buffer, sprite.transparent_index(), x, y, 1, 1);
                x += step;
            }
        }
    }

    soft_update(&tooltipFactionPower_);
    do_sound();
}

// ORIGINAL: 0x004B1FF0 ?allocate_energy@SocialWin@@QAEXHH@Z 0x004B1FF0-0x004B211C FILE
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
    char *self = reinterpret_cast<char *>(this);
    int limit = energy_limit(*reinterpret_cast<int *>(self + 0xCF4));
    *reinterpret_cast<int *>(self + 0x2C50 + a1 * 4) = a2;

    while (*reinterpret_cast<int *>(self + 0x2C58) +
               *reinterpret_cast<int *>(self + 0x2C50) +
               *reinterpret_cast<int *>(self + 0x2C54) <
           10) {
        int idx = a1 + 1;
        int attempts = 0;
        do {
            int candidate = idx % 3;
            if (a1 == candidate ||
                (*reinterpret_cast<int *>(self + 0x1780 + candidate * 0xA2C) == 0 &&
                 *reinterpret_cast<int *>(self + 0x2C50 + candidate * 4) < limit)) {
                ++*reinterpret_cast<int *>(self + 0x2C50 + candidate * 4);
                break;
            }
            ++attempts;
            ++idx;
        } while (attempts < 3);
    }

    while (*reinterpret_cast<int *>(self + 0x2C58) +
               *reinterpret_cast<int *>(self + 0x2C50) +
               *reinterpret_cast<int *>(self + 0x2C54) >
           10) {
        int idx = a1 + 1;
        int attempts = 0;
        do {
            int candidate = idx % 3;
            if (a1 == candidate ||
                (*reinterpret_cast<int *>(self + 0x1780 + candidate * 0xA2C) == 0 &&
                 *reinterpret_cast<int *>(self + 0x2C50 + candidate * 4) != 0)) {
                --*reinterpret_cast<int *>(self + 0x2C50 + candidate * 4);
                break;
            }
            ++attempts;
            ++idx;
        } while (attempts < 3);
    }
}

// The image's presets title is label slot 69 (0x114 / sizeof(LPSTR)) in the
// loaded label table. The former unit scaffold kept that lookup as a raw
// pointer walk; label_get owns the same lookup in the product source.
// ORIGINAL: 0x004AF1E0 ?draw_presets@SocialWin@@QAEXXZ 0x004AF1E0-0x004AF307
// RECOVERY_BINDING: v1 {"evidence": ["git:seed:src/recovered/units/004af1e0.cpp; source:src/socialwin.h:tutRect1_,font1_; source:src/alpha.h:label_get; source:src/strings.h:StringTemp; source:src/game.cpp:InterfaceBoxStyles"], "reason": "Home the presets panel painter on SocialWin with its typed RECT, Buffer base, font and source-owned strings."}
void SocialWin::draw_presets() {
    RECT panel = tutRect1_;
    Buffer *buffer = static_cast<Buffer *>(static_cast<GraphicWin *>(this));
    buffer->set_clip(&panel);
    buffer->box_sprite(&panel, &InterfaceBoxStyles[3]);

    panel.left += 3;
    panel.top += 3;
    panel.right -= 3;
    panel.bottom -= 3;

    RECT title = {panel.left, panel.top, panel.right, panel.top + 0x14};
    buffer->box_sprite(&title, &InterfaceBoxStyles[6]);

    StringTemp[0] = '\0';
    strcat(StringTemp, label_get(69));
    CharUpperA(StringTemp);

    buffer->set_font(&font1_, 0, 0, 0);
    buffer->set_text_color(0x9f, -1, 1, 1);
    buffer->write_cent_l(StringTemp, &title, strlen(StringTemp));

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
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00445130.cpp", "src/base.h", "src/faction.h", "src/hypothesis_layouts.h"], "reason": "Recover the shared energy accounting pass through Base and PlayerData fields, owning its report totals and per-base/facility tables instead of calling a fixed executable address."}
// TRIED: translated per-instruction against the disasm (cross-checked against the Ghidra hypothesis for the per-base accumulation, tech-level clamp, and pairwise unit-bonus loop) rather than reusing ghidra's `(int*)param_1` pointer-cast style; the neg/sbb/neg boolean idiom collapsed to a plain `(a&b)!=0` comparison, the cdq/sar halving collapsed to `/2`. MISMATCH #10 mov/xor in the zero-fill prologue; compiles and follows the disasm's three sections (per-base loop, tech-rate energy cost, cross-faction facility bonus loop) in order.
void __cdecl energy_compute(int faction_id, int recompute_bases) {
    memset(EnergyFacilityCounts, 0, sizeof(EnergyFacilityCounts));
    EnergyLabsTotal = 0;
    NetIncomeGlobal = 0;
    EnergyMaintenanceTotal = 0;
    EnergyEconomyTotal = 0;
    EnergyCommerceDelta = 0;

    int discount_carry = 0;
    for (int base_id = 0; base_id < BaseCurrentCount; ++base_id) {
        EnergyBaseMaintenance[base_id] = 0;
        Base &base = Bases[base_id];
        if (base.faction_id_current != faction_id) continue;
        if (recompute_bases) {
            set_base(base_id);
            base_compute(1);
        }

        int maintenance = 0;
        for (int facility = 0; facility < 70; ++facility) {
            if (facility < 65 && has_fac_built(facility, base_id)) {
                ++EnergyFacilityCounts[facility];
                maintenance += fac_maint(facility, faction_id);
            }
        }
        if (SecretProject.self_aware_colony() >= 0 &&
            SecretProject.self_aware_colony() < BaseCurrentCount &&
            Bases[SecretProject.self_aware_colony()].faction_id_current == faction_id) {
            // The image carries the odd half-maintenance remainder across
            // bases; keep its order rather than rounding each base alone.
            if (discount_carry) ++maintenance;
            discount_carry = maintenance & 1;
            maintenance /= 2;
        }

        EnergyBaseMaintenance[base_id] = maintenance;
        EnergyLabsTotal += base.labs_total;
        EnergyEconomyTotal += base.economy_total;
        NetIncomeGlobal += base.economy_total - maintenance;
        EnergyMaintenanceTotal += maintenance;
    }

    const int research_cost = tech_rate(faction_id);
    int research_level = PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::RESEARCH];
    if (research_level < -5) research_level = -5;
    if (research_level > 5) research_level = 5;
    EnergyLabsTotal = (research_level + 10) * EnergyLabsTotal * 10;
    BreakthroughTurnsGlobal = EnergyLabsTotal <= 0 ? 9999
        : (EnergyLabsTotal - 1 + research_cost * 100) / EnergyLabsTotal;

    const unsigned int alive = (LockEnableMask >> 8) & 0xffu;
    for (int other = 1; other < 8; ++other) {
        if (other == faction_id ||
            !(alive & (1u << other)) || !(alive & (1u << faction_id)) ||
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

// Fixed-slot bindings carried from the 004aebf0 work, named from the roles
// their stores fill. The report totals above now have genuine storage.
// PlayerSocialCategoryBlock 0x0096CC08 / PlayerSocialEffectBlock 0x0096CC2C -
// the per-faction social blocks social_calc consumes, reached through
// PlayersData's own 0x20CC stride (0x96C9E0 + 0x228 / + 0x24C). No catalogue
// names; the parameter types come from social_calc's own signature.
static SocialCategory *const PlayerSocialCategoryBlock = (SocialCategory *)0x0096CC08;
static SocialEffect *const PlayerSocialEffectBlock = (SocialEffect *)0x0096CC2C;

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
 *          effect block as the undo copy, recalculates the player's pending
 *          and stored effect rows, then recomputes energy from the new model.
 *          The category itself stays 0 for the player-block walks; the faction
 *          index selects them.
// ORIGINAL: 0x004AEBF0 ?UNK1@SocialWin@@QAEXHH@Z 0x004AEBF0-0x004AECD2 FILE
// size      226 bytes
// prototype void (__thiscall ?UNK1@SocialWin@@QAEXHH@Z)(SocialWin* this, int, int)
// callers   2   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00445130 0x004AF310 0x005B4210
*/
void SocialWin::UNK1(int soc_category, int soc_model) {
    field_A1C_ = 0xFFFFFFFFU;
    set_social_category_value(socCategory_, soc_category, soc_model);
    draw_social(soc_category);

    // The original hand-copies the 11-int effect block with a dec/jne loop;
    // a plain loop or block copy compiles to `rep movsd` instead. Keeping the
    // counter `volatile` blocks that recognition without spilling the cursor.
    socEffectOrig_ = socEffect_;

    social_calc(&socCategory_, &socEffect_, factionID_, 0, 0);
    social_calc(&socCategory_,
                reinterpret_cast<SocialEffect *>(
                    reinterpret_cast<char *>(PlayerSocialEffectBlock)
                    + factionID_ * sizeof(PlayerData)),
                factionID_, 0, 0);

    energy_compute(factionID_, 1);

    breakthroughTurns_ = BreakthroughTurnsGlobal;
    netIncome_ = NetIncomeGlobal;

    social_calc(reinterpret_cast<SocialCategory *>(
                    reinterpret_cast<char *>(PlayerSocialCategoryBlock)
                    + factionID_ * sizeof(PlayerData)),
                reinterpret_cast<SocialEffect *>(
                    reinterpret_cast<char *>(PlayerSocialEffectBlock)
                    + factionID_ * sizeof(PlayerData)),
                factionID_, 0, 0);
    // The image's third social_calc call pushes only four arguments - the
    // fifth slot is left to stack residue, which no C spelling can
    // reproduce; this call passes 0 where the image reads garbage.
}

/*
 * Purpose: Maximize one effect column: arm effectMaximize_, then for every
 *          category walk the four model rows for the best available score on
 *          that effect, select it in the category's button group and apply it
 *          through UNK1. A trailing redraw leaves the panels in step, and the
 *          arm clears once the clicks that follow consume it.
// ORIGINAL: 0x004AF110 ?maximize_effect@SocialWin@@QAEXH@Z 0x004AF110-0x004AF1D7 FILE
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
    int cat_base = 0;
    ButtonGroup *group = buttonGroups_;
    for (int category = 0; category < 4; ++category) {
        int best_score = 0;
        int best_model = 0;
        int model = 0;
        int row = 0;
        do {
            int avail = society_avail(category, row, factionID_);
            if (avail != 0 && best_score < SocialEffectTable[cat_base + model + effect_id]) {
                best_model = row;
                best_score = SocialEffectTable[cat_base + model + effect_id];
            }
            model += 0xb;
            ++row;
        } while (model < 0x2c);
        group->set(best_model + category * 4, 1);
        UNK1(category, best_model);
        cat_base += 0x35;
        ++group;
    }
    redraw();
    effectMaximize_ = 0xFFFFFFFFU;
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
    int base_id = SecretProject.cloning_vats();
    if (base_id >= 0) {
        unsigned char base_faction_id = Bases[base_id].faction_id_current;
        faction_owns_base = (static_cast<int>(base_faction_id) == factionID_);
    }
    if (faction_owns_base) {
        if ((soc_category == 2 && soc_model == 1)
            || (soc_category == 3 && soc_model == soc_category)) {
            if (SocialEffectTable[soc_model * 11 + effect_id + soc_category * 53] < 0) {
                return 1;
            }
        }
    }

    faction_owns_base = 0;
    base_id = SecretProject.network_backbone();
    if (base_id >= 0) {
        unsigned char base_faction_id = Bases[base_id].faction_id_current;
        faction_owns_base = (static_cast<int>(base_faction_id) == factionID_);
    }
    if (faction_owns_base) {
        if (soc_category == 3 && soc_model == 1) {
            if (SocialEffectRow[effect_id] < 0) {
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
// ORIGINAL: 0x004B1CE0 ?on_group_clicked@SocialWin@@QAEXHH@Z 0x004B1CE0-0x004B1D64 FILE
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
        int effect = a2 - 0x11;
        if (effect >= 3) {
            ++effect;
        }
        maximize_effect(effect);
        return;
    }
    if (a2 >= 0 && a2 < 0x10) {
        int model = a2 % 4;
        int category = a2 / 4;
        UNK1(category, model);
        if (effectMaximize_ == 0xFFFFFFFFU && field_2C28_ == 0) {
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
// ORIGINAL: 0x004B1D70 ?on_button_passover@SocialWin@@QAEXHH@Z 0x004B1D70-0x004B1DF3 FILE
// size      131 bytes
// prototype void (__thiscall ?on_button_passover@SocialWin@@QAEXHH@Z)(SocialWin* this, int, int)
// callers   0   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004B0410 0x004B1080 0x005FCBB0
*/
void SocialWin::on_button_passover(int a1, int a2) {
    if (a1 >= 0 && a1 < 0x10 && a2 != 0 && field_A2C_ != static_cast<uint32_t>(a1)) {
        field_A2C_ = static_cast<uint32_t>(a1);
        draw_mouseover();
    }
    if (a1 >= 0x11 && a1 <= 0x1b) {
        if (a2 == 0) {
            if (field_A30_ != 0xFFFFFFFFU) {
                field_A30_ = 0xFFFFFFFFU;
                draw_overview();
                do_all_draws();
                return;
            }
        } else if (field_A30_ != static_cast<uint32_t>(a1)) {
            field_A30_ = static_cast<uint32_t>(a1);
            draw_overview();
        }
    }
    do_all_draws();
}

// Link-only placeholders; the passover path and init are what reference them.
// The section keys and short UI strings are image-owned read-only data. The
// file, rows and receiver are product objects; these four strings are the
// remaining immutable resources the original painter reads directly.
static const char *const SocialOverviewSection = "SOCIO";
static const char *const SocialOverviewPath = (const char *)0x0068749c;
static const char *const SocialOverviewPlus = (const char *)0x006874a0;
// ORIGINAL: 0x004B0410 ?draw_overview@SocialWin@@QAEXXZ 0x004B0410-0x004B089A
// RECOVERY_BINDING: v1 {"evidence": ["image:0x004B0410-0x004B089A draws the effects overview, skips the reserved fourth row, loads each SocialEffects section and repaints the selected/comparison rows", "source:src/socialwin.h: tooltipEffects_, effectMaximize_, socEffect_, font1_ and font7_ are the corresponding declared members", "source:src/faction.h: SocialEffect and RulesSocialEffect define the eleven live rows and their section names"], "reason": "Home the overview painter on the SocialWin-owned rectangles, effects, fonts and rules records; preserve its original identity while replacing the seed's opaque object accesses with the product model."}
void SocialWin::draw_overview() {
    Buffer *buffer = reinterpret_cast<Buffer *>(InterfaceBuffer);
    int selected = static_cast<int>(field_A30_);
    int selected_effect = -1;
    if (selected >= 0) {
        selected_effect = selected - 0x11;
        if (selected_effect > 2) {
            selected_effect = selected - 0x10;
        }
    }

    buffer->set_clip(reinterpret_cast<RECT *>(InterfaceClipRect));
    *InterfacePaletteIndex = 5;

    RECT panel = tooltipEffects_;
    buffer->box_sprite(&panel, &InterfaceBoxStyles[3]);
    RECT interior = {panel.left + 3, panel.top + 3,
                     panel.right - 3, panel.bottom - 3};
    buffer->box_sprite(&interior, &InterfaceBoxStyles[6]);

    buffer->set_font(&font1_, 0, 0, 0);
    buffer->set_text_color(0x9f, -1, 1, 1);

    char *title = label_get(0x2e4);
    if (title != 0) {
        buffer->write_cent_l(title, &interior, strlen(title));
    }

    interior.top = panel.top + 0x1c;
    RECT effects_area = {interior.left, interior.top,
                         interior.right, interior.bottom};
    buffer->box_sprite(&effects_area, &InterfaceBoxStyles[0]);
    buffer->set_font(&font7_, 0, 0, 0);

    int text_x = effects_area.left + 0x14;
    int number_x = effects_area.left + 0xf;
    int text_right = effects_area.right - 3;
    int text_bottom = effects_area.bottom - 3;
    const int available_width = text_right - text_x;
    const SocialEffect &player_effect = PlayersData[factionID_].soc_effect_pending;
    char descriptions[MaxSocialEffectNum][256];
    int content_height = 0;

    text_open(AlphaxFileID, SocialOverviewSection);

    // Each rules entry names one SOCIAL section. The fourth entry is the
    // reserved slot at index 3 and has no section, as shown by the seed's
    // 0x009466B8 skip (three 0x68-byte records after SocialEffects).
    int effect;
    for (effect = 0; effect < MaxSocialEffectNum; ++effect) {
        descriptions[effect][0] = 0;
        if (effect == 3) {
            continue;
        }

        char section[64];
        strcpy(section, SocialOverviewPath);
        strcat(section, SocialEffects[effect].set1);
        if (text_open(0, section) != 0) {
            text_close();
            return;
        }

        text_get();
        int row = text_item_number();
        strcpy(descriptions[effect], text_item());
        while (row < social_effect_value(socEffect_, effect)) {
            char *line = text_get();
            if (line == 0 || line[0] == 0) {
                break;
            }
            row = text_item_number();
            strcpy(descriptions[effect], text_item());
        }
        content_height += buffer->wrap_height(descriptions[effect], available_width);
    }
    text_close();

    int y = effects_area.top
        + ((text_bottom - content_height) - effects_area.top) / 2;
    for (effect = 0; effect < MaxSocialEffectNum; ++effect) {
        if (effect == 3) {
            continue;
        }
        if (selected_effect == effect) {
            buffer->set_text_color(0xff, -1, 1, 1);
        } else {
            int color;
            if (effectMaximize_ == static_cast<uint32_t>(effect)) {
                color = 0x9c;
            } else if (social_effect_value(socEffect_, effect)
                       == social_effect_value(player_effect, effect)) {
                color = 0xa0;
            } else if (social_effect_value(player_effect, effect)
                       < social_effect_value(socEffect_, effect)) {
                color = 0xfa;
            } else {
                color = 0xf9;
            }
            buffer->set_text_color(color, -1, 1, 1);
        }

        StringTemp[0] = 0;
        const int value = social_effect_value(socEffect_, effect);
        if (value > 0) {
            strcat(StringTemp, SocialOverviewPlus);
        }
        char number[32];
        reinterpret_cast<ItoaFn>(_itoa)(value, number, 10);
        strcat(StringTemp, number);
        buffer->write_right_l(StringTemp, number_x, y, strlen(StringTemp));

        RECT description = {text_x, y, text_right, text_bottom};
        y = buffer->wrap(descriptions[effect], &description);
        // The original uses Font::line_height for negative entries and
        // Font::height + value for nonnegative entries. Font currently only
        // exposes line_advance(), so the typed path uses its configured height
        // plus the effect adjustment; exact negative-row pitch remains open.
        y += font7_.line_advance() - font7_.unk_1_
            + (value > 0 ? value : 0);
    }

    *InterfacePaletteIndex = 5;
    soft_update(&tooltipEffects_);
    do_sound();
}

// ORIGINAL: 0x004AECE0 ?reset@SocialWin@@QAEXXZ 0x004AECE0-0x004AEEF1
// RECOVERY_BINDING: v1 {"evidence": ["image:0x004AECE0-0x004AEEF1 resets the selected category, button groups, effect snapshots, social allocation values, calculated energy and visible-window redraw", "source:src/faction.h: PlayersData owns the pending SocialCategory, SocialEffect and social allocation values", "source:src/socialwin.h: SocialWin owns the working/original values and category button groups"], "reason": "Restore the panel from its owning PlayerData record through typed social calculation and button-group operations while retaining the mapped reset identity."}
void SocialWin::reset() {
    field_A1C_ = 0xFFFFFFFFU;
    social_ai(factionID_, -1, -1, -1, -1, 0);

    field_2C28_ = 1;
    SocialCategory &pending_category = PlayersData[factionID_].soc_category_pending;
    for (int category = 0; category < 4; ++category) {
        const int model = social_category_value(pending_category, category);
        set_social_category_value(socCategory_, category, model);
        buttonGroups_[category].set(model + category * 4, 1);
    }
    field_2C28_ = 0;

    const SocialEffect &pending_effect = PlayersData[factionID_].soc_effect_pending;
    socEffect_ = pending_effect;
    socEffectOrig_ = pending_effect;

    PlayerData &player = PlayersData[factionID_];
    const bool allocations_changed =
        player.se_alloc_psych != static_cast<int>(psychAllocOrig_) ||
        player.se_alloc_labs != static_cast<int>(labsAllocOrig_);
    psychAllocOrig_ = static_cast<uint32_t>(player.se_alloc_psych);
    labsAllocOrig_ = static_cast<uint32_t>(player.se_alloc_labs);

    economyAllocPending_ = economyAllocOrig_;
    psychAllocPending_ = psychAllocOrig_;
    labsAllocPending_ = labsAllocOrig_;

    if (allocations_changed) {
        social_set(factionID_);
        social_calc(&socCategory_, &socEffect_, factionID_, 0, 0);
        social_calc(&socCategory_, &player.soc_effect_pending, factionID_, 0, 0);
    }

    energy_compute(factionID_, 1);
    breakthroughTurns_ = BreakthroughTurnsGlobal;
    netIncome_ = NetIncomeGlobal;
    social_calc(&player.soc_category_pending, &player.soc_effect_pending,
                factionID_, 0, 0);

    if (is_visible()) {
        redraw();
    }
}

// Fixed-slot binding carried from the 004b17a0 work: the "+" the effect
// count is prefixed with. Image .data reads "+" (0x006874AC, beside
// "socwin.pcx"); no catalogue name.
static char *const PlusText = (char *)0x006874AC;

/*
 * Purpose: Track the pointer over the social panel: the bubble is cleared
 *          (twice, exactly as the original does), then the four category
 *          tooltip rectangles are scanned row by row - each available row
 *          tests its grid rectangle against the cursor, and the inner row
 *          scan stops at a hit while the outer category scan keeps running,
 *          so a later category's hit wins. The faction-power rectangle maps
 *          the hover to -2, anything else to the grid index or -1, and a
 *          changed hover repaints the mouseover panel.
// ORIGINAL: 0x004B1EC0 ?on_mouse_move@SocialWin@@QAEXHH@Z 0x004B1EC0-0x004B1FB2 FILE
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
    // The four category tooltip rectangles: tooltipSocEng_ plus the three
    // RECT-sized slots that follow it, one per category.
    RECT *category_rects[4] = {&tooltipSocEng_, &tutRect4_,
                               &tutRects3_[0], &tutRects3_[1]};
    for (int category = 0; category < 4; ++category) {
        for (int row = 0; row < 4; ++row) {
            if (society_avail(category, row, factionID_) == 0) {
                continue;
            }
            RECT rect = *category_rects[category];
            RECT *grid = &tutRects3_[2 + row + category * 4];
            rect.left = grid->left;
            rect.right = grid->right;
            if (in_box(a1, a2, &rect) != 0) {
                hover = row + category * 4;
                break;
            }
        }
    }

    if (in_box(a1, a2, &tooltipFactionPower_) != 0) {
        hover = -2;
    }
    if (field_A2C_ != static_cast<uint32_t>(hover)) {
        field_A2C_ = static_cast<uint32_t>(hover);
        draw_mouseover();
    }
}

/*
 * Purpose: Draw one social button's text band: the effect icon at the left
 *          (its colour picked from the button's status), the effect name, and
 *          the count on the right - signed "+" and recoloured when it differs
 *          from the faction's stored value. Invoked for the effect column's
 *          buttons only (0x11..0x1a), with the separator slot skipped.
// ORIGINAL: 0x004B17A0 ?on_button_text_draw@SocialWin@@QAEHPAUGraphicWin@@PADH@Z 0x004B17A0-0x004B1991 FILE
// symbol    ?on_button_text_draw@SocialWin@@QAEHPAVGraphicWin@@PADH@Z
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

    char *win = reinterpret_cast<char *>(window);
    Buffer *win_buffer = reinterpret_cast<Buffer *>(win + 0x444);

    // The buffer's own working rectangle (Buffer::rect2_, 0x474).
    RECT rect;
    RECT *source = reinterpret_cast<RECT *>(win + 0x474);
    rect.left = source->left;
    rect.top = source->top;
    rect.right = source->right;
    rect.bottom = source->bottom;
    int line_base = effectIcons_[0].height();
    int y_center = (rect.bottom - line_base - rect.top) / 2;

    char *button = reinterpret_cast<char *>(flatButtons_) + button_id * 0xB4C;
    int status = *reinterpret_cast<int *>(button + 0xAB8);
    int flag;
    if (status == 0) {
        flag = 1;
    } else if (status == 1 || status == 2
               || *reinterpret_cast<int *>(button + 0xA18) != 0) {
        flag = 0;
    } else {
        flag = (*reinterpret_cast<int *>(button + 0xA74) == 0) ? 1 : 0;
    }

    Buffer *icon_buffer = (window != 0) ? win_buffer : 0;
    int sprite_index = flag + button_id * 3;
    unsigned char sprite_frame = *reinterpret_cast<unsigned char *>(
        reinterpret_cast<char *>(this) + 0x13D4 + sprite_index * 0x84);
    Sprite *sprite = reinterpret_cast<Sprite *>(
        reinterpret_cast<char *>(this) + 0x13CC + sprite_index * 0x84);
    sprite->draw(icon_buffer, sprite_frame, 5, y_center, 1, 1);

    win_buffer->set_text_color(0, -1, 1, 1);
    rect.left += 0x25;
    if (text != 0) {
        win_buffer->write_l(text, &rect, strlen(text));
    }

    int current = reinterpret_cast<const int *>(&socEffect_)[effect];
    int stored = reinterpret_cast<const int *>(PlayerSocialEffectBlock)
        [2099 * factionID_ + effect];
    if (current > stored) {
        win_buffer->set_text_color(0xfa, -1, 1, 1);
    } else if (current < stored) {
        win_buffer->set_text_color(0xf9, -1, 1, 1);
    }

    StringTemp[0] = 0;
    if (current > 0) {
        strcat(StringTemp, PlusText);
    }
    char number[80];
    reinterpret_cast<ItoaFn>(_itoa)(current, number, 10);
    strcat(StringTemp, number);

    rect.right -= 3;
    if (StringTemp != 0) {
        win_buffer->write_right_l(StringTemp, &rect, strlen(StringTemp));
    }
    return 1;
}

// Fixed-slot bindings carried from the 004b1080 work, named from the same
// evidence datalink.cpp records for the shared tables.
// SocialCategoryRow   0x0094B014 - SocialCategories + 0x14; the header id
//                     the mouseover title reads, indexed
//                     [category * 0x35 + row].
// SocialEffectRowBase 0x00946598 - SocialEffects + 0x18; the effect name
//                     strings, rows 0x68 apart.
// PlayerPowerBlock    0x0096CCB0 - the per-faction power integers the -2
//                     (faction-power) hover reads, PlayersData stride.
// MinusText 0x006874A4 / MouseoverPlusText 0x006874A8 - image .data reads
//                     "-" and "+" (beside the count prefix PlusText below).
static const int *const SocialCategoryRow = (const int *)0x0094B014;
static const char *const SocialEffectRowBase = (const char *)0x00946598;
static int *const PlayerPowerBlock = (int *)0x0096CCB0;
static char *const MinusText = (char *)0x006874A4;
static char *const MouseoverPlusText = (char *)0x006874A8;

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
    Buffer *buf = reinterpret_cast<Buffer *>(InterfaceBuffer);

    buf->set_clip(reinterpret_cast<RECT *>(InterfaceClipRect));

    RECT *header_rect = &tutRect9_;
    RECT rect1 = *header_rect;

    buf->box_sprite(&rect1, &InterfaceBoxStyles[3]);

    RECT rect2;
    rect2.left = rect1.left + 3;
    rect2.top = rect1.top + 3;
    rect2.right = rect1.right - 3;
    rect2.bottom = rect1.top + 0x17;
    rect1 = rect2;

    buf->box_sprite(&rect2, &InterfaceBoxStyles[6]);

    int mode = static_cast<int>(field_A2C_);
    int cat_idx = 0;
    int row_idx = 0;

    if (mode != -1) {
        int table_index;
        if (mode == -2) {
            table_index = *reinterpret_cast<int *>(
                reinterpret_cast<char *>(Labels.strings_ptr) + 0xB94);
        } else {
            cat_idx = mode / 4;
            row_idx = mode % 4;
            table_index = SocialCategoryRow[row_idx + cat_idx * 0x35];
        }
        StringTemp[0] = 0;
        strcat(StringTemp, StringTable.get(table_index));
        CharUpperA(StringTemp);
        buf->set_font(&font1_, NULL, NULL, NULL);
        buf->set_text_color(0x9f, -1, 1, 1);
        buf->write_cent_l(StringTemp, &rect1, strlen(StringTemp));
    }

    rect1.top = rect2.bottom + 2;
    buf->box_sprite(&rect1, &InterfaceBoxStyles[0]);

    if (field_A2C_ != 0xFFFFFFFFU) {
        rect1.left += 3;
        rect1.right -= 3;
        rect1.top += 3;
        rect1.bottom -= 3;

        int count = 0;
        int self_faction = factionID_;

        for (int i = 0; i < 11; ++i) {
            if (i == 3) {
                continue;
            }
            int v;
            if (field_A2C_ == 0xFFFFFFFEU) {
                v = PlayerPowerBlock[i + self_faction * 2099];
            } else {
                bool skip = false;
                if (SecretProject.cloning_vats() >= 0 &&
                    Bases[SecretProject.cloning_vats()].faction_id_current ==
                        static_cast<unsigned int>(self_faction)) {
                    if ((cat_idx == 2 && row_idx == 1) || (cat_idx == 3 && row_idx == 3)) {
                        if (SocialEffectTable[row_idx * 0xb + (i + cat_idx * 0x35)] < 0) {
                            skip = true;
                        }
                    }
                }
                if (!skip && SecretProject.network_backbone() >= 0 &&
                    Bases[SecretProject.network_backbone()].faction_id_current ==
                        static_cast<unsigned int>(self_faction) &&
                    cat_idx == 3 && row_idx == 1 &&
                    SocialEffectRow[i] < 0) {
                    skip = true;
                }
                if (skip) {
                    continue;
                }
                v = SocialEffectTable[row_idx * 0xb + (i + cat_idx * 0x35)];
            }
            if (v < 0) {
                v = -v;
            }
            if (v != 0) {
                ++count;
            }
        }

        int line_height = effectIcons_[0].height();
        buf->set_font(&font3_, NULL, NULL, NULL);

        rect1.top += ((rect1.bottom - line_height * count) - rect1.top) / 2;
        rect1.left += 3;
        rect1.bottom = line_height + rect1.top;

        for (int pass = 0; pass < 2; ++pass) {
            for (int i = 0; i < 11; ++i) {
                if (i == 3) {
                    continue;
                }
                if (SecretProject.cloning_vats() >= 0 &&
                    Bases[SecretProject.cloning_vats()].faction_id_current ==
                        static_cast<unsigned int>(self_faction)) {
                    if ((cat_idx == 2 && row_idx == 1) || (cat_idx == 3 && row_idx == 3)) {
                        if (SocialEffectTable[row_idx * 0xb + (i + cat_idx * 0x35)] < 0) {
                            continue;
                        }
                    }
                }
                if (SecretProject.network_backbone() >= 0 &&
                    Bases[SecretProject.network_backbone()].faction_id_current ==
                        static_cast<unsigned int>(self_faction) &&
                    cat_idx == 3 && row_idx == 1 &&
                    SocialEffectRow[i] < 0) {
                    continue;
                }

                int v2;
                if (field_A2C_ == 0xFFFFFFFEU) {
                    v2 = PlayerPowerBlock[i + self_faction * 2099];
                } else {
                    v2 = SocialEffectTable[row_idx * 0xb + (i + cat_idx * 0x35)];
                }

                if (v2 == 0) {
                    continue;
                }
                bool draw = (pass == 0) ? (v2 >= 0) : (v2 < 0);
                if (!draw) {
                    continue;
                }

                StringTemp[0] = 0;
                int color_id;
                if (v2 < 0) {
                    strcat(StringTemp, MinusText);
                    color_id = 0x67;
                } else {
                    strcat(StringTemp, MouseoverPlusText);
                    color_id = 0x98;
                }
                buf->set_text_color(color_id, -1, 1, 1);

                int magnitude = (v2 < 0) ? -v2 : v2;
                char number[80];
                reinterpret_cast<ItoaFn>(_itoa)(magnitude, number, 10);
                strcat(StringTemp, number);
                buf->write_l(StringTemp, &rect1, strlen(StringTemp));

                int icon_row = i + (v2 >= 0 ? 2 : 1);
                Sprite *icon = &effectIcons_[6 + icon_row];
                unsigned char frame = reinterpret_cast<unsigned char *>(icon)[8];
                icon->draw(buf, frame, rect1.left + 0x10, rect1.top, 1, 1);

                StringTemp[0] = 0;
                rect1.left = rect1.left + 0x10 + effectIcons_[0].width();
                strcat(StringTemp, SocialEffectRowBase + i * 0x68);
                buf->write_l(StringTemp, &rect1, strlen(StringTemp));
                rect1.left = rect1.left - 0x10 - effectIcons_[0].width();
                rect1.top += line_height;
                rect1.bottom += line_height;
            }
        }
    }

    reinterpret_cast<GraphicWin *>(this)->soft_update(header_rect);
    do_sound();
}

// Fixed-slot bindings carried from the 004afb60 work, named from what their
// reads feed. PlayerUpheavalBlock 0x0096CC28 - the per-faction upheaval
// baseline social_upheaval's result is compared against. CommaSpaceText
// 0x00682E90 and ColonSpaceText 0x00682E94 - image .data reads ", " and ": ".
// PercentSpaceText{A,B,C,D} 0x00687480/84/88/8C - four distinct image copies
// of "% " the deficit text concatenates.
static int *const PlayerUpheavalBlock = (int *)0x0096CC28;
static char *const CommaSpaceText = (char *)0x00682E90;
static char *const ColonSpaceText = (char *)0x00682E94;
static char *const PercentSpaceTextA = (char *)0x00687480;
static char *const PercentSpaceTextB = (char *)0x00687484;
static char *const PercentSpaceTextC = (char *)0x00687488;
static char *const PercentSpaceTextD = (char *)0x0068748C;
// UpheavalPlusText 0x00687490 - image .data reads "+" (beside "SOCIO...").
static char *const UpheavalPlusText = (char *)0x00687490;

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
    Buffer *own_buffer = reinterpret_cast<Buffer *>(
        reinterpret_cast<char *>(this) + 0x444);
    Buffer *buf = reinterpret_cast<Buffer *>(InterfaceBuffer);

    own_buffer->set_clip(&tutRect14_);

    RECT header;
    header.left = tutRect14_.left;
    header.top = tutRect14_.top;
    header.right = tutRect14_.right;
    header.bottom = tutRect14_.bottom;

    RECT title;
    title.left = header.left;
    title.top = header.top;
    title.bottom = header.top + 0x14;
    title.right = header.right;
    buf->box_sprite(&title, &InterfaceBoxStyles[6]);

    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0x2BB));
    CharUpperA(StringTemp);

    buf->set_font(&font1_, 0, 0, 0);
    buf->set_text_color(0x9c, -1, 1, 1);
    buf->write_cent_l(StringTemp, &title, strlen(StringTemp));

    int text_top = title.bottom + 2;
    buf->box_sprite(&tutRect14_, &InterfaceBoxStyles[0]);
    tutRect14_.right -= 3;
    tutRect14_.bottom -= 3;
    int text_left = tutRect14_.left + 3;
    text_top += 3;
    buf->set_font(&font6_, 0, 0, 0);

    int upheaval = static_cast<int>(social_upheaval(
        factionID_, &socCategory_));
    upheaval -= PlayerUpheavalBlock[factionID_ * 2099];

    int text_y;
    int row_y;
    if (WinScreenWidth == 800) {
        text_y = font6_.line_advance() + text_top;
    } else {
        text_y = ((tutRect14_.bottom + font6_.line_advance() * -2) - text_top) / 2
            + text_top;
        row_y = text_y;
    }

    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0x3BF));
    strcat(StringTemp, ColonSpaceText);
    char number[80];
    reinterpret_cast<ItoaFn>(_itoa)(NetIncomeGlobal, number, 10);
    strcat(StringTemp, number);
    buf->set_text_color(0xa0, -1, 1, 1);
    buf->write_l(StringTemp, text_left, text_y, strlen(StringTemp));

    row_y = text_y + font6_.line_advance();

    StringTemp[0] = 0;
    strcat(StringTemp, label_get(150));
    strcat(StringTemp, ColonSpaceText);
    reinterpret_cast<ItoaFn>(_itoa)(BreakthroughTurnsGlobal, number, 10);
    strcat(StringTemp, number);
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, label_get(49));
    buf->write_l(StringTemp, text_left, row_y, strlen(StringTemp));

    if (WinScreenWidth == 800) {
        row_y += font6_.line_advance();
    } else {
        text_left += (tutRect14_.right - tutRect14_.left) / 2;
        row_y -= font6_.line_advance();
    }

    int faction_stride = factionID_ * 2099;
    int imbalance = SocialAllocTable[faction_stride] - 10
        + SocialAllocTable[faction_stride + 1] * 2;
    if (imbalance < 0) {
        imbalance = (5 - SocialAllocTable[faction_stride + 1]) * 2
            - SocialAllocTable[faction_stride];
    }
    int model_shift = socEffect_.values_[SocialEffect::EFFICIENCY];
    imbalance /= 2;
    int clamped_shift;
    if (model_shift < -4) {
        clamped_shift = -4;
    } else {
        clamped_shift = model_shift;
        if (4 < model_shift) {
            clamped_shift = 4;
        }
    }
    int shift_side;
    if (clamped_shift - 4 < 0) {
        if (model_shift < -4) {
            model_shift = -4;
        } else if (4 < model_shift) {
            model_shift = 4;
        }
        shift_side = 4 - model_shift;
    } else {
        if (model_shift < -4) {
            model_shift = -4;
        } else if (4 < model_shift) {
            model_shift = 0;
            goto scaled;
        }
        model_shift = model_shift - 4;
    }
scaled:
    shift_side *= 2;

    if (imbalance != 0 && shift_side != 0) {
        StringTemp[0] = 0;
        int reserve = SocialAllocTable[faction_stride + 1];
        if ((10 - SocialAllocTable[faction_stride]) - reserve < reserve) {
            int percent = shift_side * imbalance;
            int shown = (percent < 0) ? 0 : ((0x28 < percent) ? 0x28 : percent);
            reinterpret_cast<ItoaFn>(_itoa)(-shown, number, 10);
            strcat(StringTemp, number);
            strcat(StringTemp, PercentSpaceTextA);
            strcat(StringTemp, label_get(60));

            if (10 - SocialAllocTable[faction_stride] != reserve) {
                strcat(StringTemp, CommaSpaceText);
                int percent2 = shift_side * imbalance * 2;
                percent2 = (percent2 < 0) ? 0 : ((0x50 < percent2) ? 0x50 : percent2);
                reinterpret_cast<ItoaFn>(_itoa)(-percent2, number, 10);
                strcat(StringTemp, number);
                strcat(StringTemp, PercentSpaceTextB);
                strcat(StringTemp, label_get(169));
            }
        } else {
            if (reserve != 0) {
                int percent = shift_side * imbalance * 2;
                percent = (percent < 0) ? 0 : ((0x50 < percent) ? 0x50 : percent);
                reinterpret_cast<ItoaFn>(_itoa)(-percent, number, 10);
                strcat(StringTemp, number);
                strcat(StringTemp, PercentSpaceTextC);
                strcat(StringTemp, label_get(60));
                strcat(StringTemp, CommaSpaceText);
            }
            int percent = shift_side * imbalance;
            percent = (percent < 0) ? 0 : ((0x28 < percent) ? 0x28 : percent);
            reinterpret_cast<ItoaFn>(_itoa)(-percent, number, 10);
            strcat(StringTemp, number);
            strcat(StringTemp, PercentSpaceTextD);
            strcat(StringTemp, label_get(169));
        }

        buf->set_text_color(0xf9, -1, 1, 1);
        buf->write_l(StringTemp, text_left, row_y, strlen(StringTemp));

        row_y += font6_.line_advance();
    }

    if (upheaval < 0) {
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(222));
        strcat(StringTemp, ColonSpaceText);
        strcat(StringTemp, UpheavalPlusText);
        reinterpret_cast<ItoaFn>(_itoa)(-upheaval, number, 10);
        strcat(StringTemp, number);
        strcat(StringTemp, BufferJoinSpace);
        strcat(StringTemp, label_get(230));
        buf->set_text_color(0xa0, -1, 1, 1);
    } else if (upheaval >= 1) {
        buf->set_text_color(0xf9, -1, 1, 1);
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(221));
        strcat(StringTemp, ColonSpaceText);
        reinterpret_cast<ItoaFn>(_itoa)(upheaval, number, 10);
        strcat(StringTemp, number);
        strcat(StringTemp, BufferJoinSpace);
        strcat(StringTemp, label_get(230));
    }
    if (upheaval != 0) {
        buf->write_l(StringTemp, text_left, row_y, strlen(StringTemp));
    }

    MainInterfaceGlobal.soft_update(&tutRect14_);
    do_sound();
    own_buffer->set_clip(reinterpret_cast<RECT *>(
        reinterpret_cast<char *>(own_buffer) + 0x30));
}


// Fixed-slot bindings carried from the 004b2740 work, named from the roles
// their reads fill.
// SocialSubInterfaceSlot 0x007AE7F8 - the interface-slot object pointer the
//                     interface-mode registry walks.
// InterfaceScreenRect 0x007AF5B0 - the RECT battlewin.cpp binds; the panel
//                     rectangle MainInterface keeps at front+0xD90.
// The full panel rectangle copied into tutRect14_ is the now-typed
// MainInterfaceGlobal.info_panel_frame_ at original 0x007AF5C0.
// MainPanelLeft/Top/Right 0x007AF5A0/A4/A8 and SocialColumnTop/Right/Bottom
//                     0x007AF5D4/D8/DC - the layout edges the tooltip and
//                     column rectangles derive from.
// MainPanelHeight     0x007AF59C - the height the window's y centres against.
// FontNameArialSlot 0x00691B2C / FontNameNarrowSlot 0x00691B30 - char* slots
//                     holding "Arial" and "Arial Narrow".
// SocWinPcxName 0x006874B0 - image .data reads "socwin.pcx".
// TutorialStage 0x008CC228 - the int the tutorial entry stores 5 into.
// TutorialSocRect 0x008C5380 - the RECT TutWin::soc_rect fills.
// TutSocWinText 0x006874BC - image .data reads "SOCWIN", the tutorial call's
//                     text.
// ConsoleIfaceSlot 0x009156B0 - console.h's ConsoleGlobal; this TU reaches
//                     only its first two dwords, so the binding stays local.
static int *const SocialSubInterfaceSlot = (int *)0x007AE7F8;
static RECT *const InterfaceScreenRect = (RECT *)0x007AF5B0;
static int *const MainPanelLeft = (int *)0x007AF5A0;
static int *const MainPanelTop = (int *)0x007AF5A4;
static int *const MainPanelRight = (int *)0x007AF5A8;
static int *const SocialColumnTop = (int *)0x007AF5D4;
static int *const SocialColumnRight = (int *)0x007AF5D8;
static int *const SocialColumnBottom = (int *)0x007AF5DC;
static int *const MainPanelHeight = (int *)0x007AF59C;
static char **const FontNameArialSlot = (char **)0x00691B2C;
static char **const FontNameNarrowSlot = (char **)0x00691B30;
static char *const SocWinPcxName = (char *)0x006874B0;
static int *const TutorialStage = (int *)0x008CC228;
static RECT *const TutorialSocRect = (RECT *)0x008C5380;
static char *const TutSocWinText = (char *)0x006874BC;
static int *const ConsoleIfaceSlot = (int *)0x009156B0;
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
    char *self = reinterpret_cast<char *>(this);
    Buffer sheet;

    for (int i = 0; i < 0x23; ++i) {
        flatButtons_[i].close();
    }
    for (int group_index = 0; group_index < 4; ++group_index) {
        buttonGroups_[group_index].close();
    }
    buttonGroup_.close();

    field_A2C_ = 0xFFFFFFFFU;
    field_A30_ = 0xFFFFFFFFU;
    field_2C28_ = 0;
    effectMaximize_ = 0xFFFFFFFFU;
    // The interface-mode registry's own state: the SubInterface base pointer
    // and its mode field.
    *SocialSubInterfaceSlot = reinterpret_cast<int>(
        static_cast<SubInterface *>(this));
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
    tooltipSocEng_.left = 0x1a;
    tooltipSocEng_.top = 4;
    tooltipSocEng_.right = 0x26e;
    tooltipSocEng_.bottom = 0x4a;
    tutRect4_.left = 0x1a; tutRect4_.top = 0x4d; tutRect4_.right = 0x26e; tutRect4_.bottom = 0x93;
    tutRects3_[0].left = 0x1a; tutRects3_[0].top = 0x96; tutRects3_[0].right = 0x26e; tutRects3_[0].bottom = 0xdc;
    tutRects3_[1].left = 0x1a; tutRects3_[1].top = 0xdf; tutRects3_[1].right = 0x26e; tutRects3_[1].bottom = 0x125;
    tutRects3_[2].left = 0x1d; tutRects3_[2].top = 0x1d; tutRects3_[2].right = 0xaf; tutRects3_[2].bottom = 0x31;
    tutRects3_[3].left = 0xb1; tutRects3_[3].top = 0x1d; tutRects3_[3].right = 0x143; tutRects3_[3].bottom = 0x31;
    tutRects3_[4].left = 0x145; tutRects3_[4].top = 0x1d; tutRects3_[4].right = 0x1d7; tutRects3_[4].bottom = 0x31;
    tutRects3_[5].left = 0x1d9; tutRects3_[5].top = 0x1d; tutRects3_[5].right = 0x26b; tutRects3_[5].bottom = 0x31;
    tutRects3_[6].left = 0x1d; tutRects3_[6].top = 0x66; tutRects3_[6].right = 0xaf; tutRects3_[6].bottom = 0x7a;
    tutRects3_[7].left = 0xb1; tutRects3_[7].top = 0x66; tutRects3_[7].right = 0x143; tutRects3_[7].bottom = 0x7a;
    tutRects3_[8].left = 0x145; tutRects3_[8].top = 0x66; tutRects3_[8].right = 0x1d7; tutRects3_[8].bottom = 0x7a;
    tutRects3_[9].left = 0x1d9; tutRects3_[9].top = 0x66; tutRects3_[9].right = 0x26b; tutRects3_[9].bottom = 0x7a;
    tutRects3_[10].left = 0x1d; tutRects3_[10].top = 0xaf; tutRects3_[10].right = 0xaf; tutRects3_[10].bottom = 0xc3;
    tutRects3_[11].left = 0xb1; tutRects3_[11].top = 0xaf; tutRects3_[11].right = 0x143; tutRects3_[11].bottom = 0xc3;
    tutRects3_[12].left = 0x145; tutRects3_[12].top = 0xaf; tutRects3_[12].right = 0x1d7; tutRects3_[12].bottom = 0xc3;
    tutRects3_[13].left = 0x1d9; tutRects3_[13].top = 0xaf; tutRects3_[13].right = 0x26b; tutRects3_[13].bottom = 0xc3;
    tutRects3_[14].left = 0x1d; tutRects3_[14].top = 0xf8; tutRects3_[14].right = 0xaf; tutRects3_[14].bottom = 0x10c;
    tutRects3_[15].left = 0xb1; tutRects3_[15].top = 0xf8; tutRects3_[15].right = 0x143; tutRects3_[15].bottom = 0x10c;
    tutRects3_[16].left = 0x145; tutRects3_[16].top = 0xf8; tutRects3_[16].right = 0x1d7; tutRects3_[16].bottom = 0x10c;
    tutRects3_[17].left = 0x1d9; tutRects3_[17].top = 0xf8; tutRects3_[17].right = 0x26b; tutRects3_[17].bottom = 0x10c;
    tooltipFactionPower_.left = 0x271;
    tooltipFactionPower_.top = 0xfd;
    tooltipFactionPower_.right = 0x306;
    tooltipFactionPower_.bottom = 0x12d;

    {
        int left = *MainPanelLeft + 7;
        int bottom = *SocialColumnBottom - 7;
        int top = *SocialColumnTop + 7;
        int split = ((*SocialColumnRight - left) - 4) / 3 - 3 + left;
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
        int right = *SocialColumnRight - 7;
        tooltipLabs_.left = split + 3;
        tooltipLabs_.top = top;
        tooltipLabs_.right = right;
        tooltipLabs_.bottom = bottom;
        tutRect9_.left = *MainPanelLeft + 4;
        tutRect9_.top = *MainPanelTop + 4;
        tutRect9_.right = *MainPanelRight - 4;
        tutRect9_.bottom = tooltipLabs_.top - 6;
        tooltipEffects_ = *InterfaceScreenRect;
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

    font1_.init(*FontNameArialSlot, 0xe, 1);
    font2_.init(*FontNameArialSlot, 0xe, 1);
    font3_.init(*FontNameNarrowSlot, 0xe, 1);
    font4_.init(*FontNameNarrowSlot, 0xe, 1);
    font5_.init(*FontNameArialSlot, 0xc, 0);
    font6_.init(*FontNameArialSlot, 0xb, 1);
    font6_.unk_1_ = 3;
    font7_.init(*FontNameArialSlot, 0xb, 0);
    font7_.unk_1_ = 3;
    font8_.init(*FontNameNarrowSlot, 0xe, 1);
    font9_.init(*FontNameNarrowSlot, 0xe, 1);

    if (*reinterpret_cast<int *>(
            reinterpret_cast<char *>(&effectIcons_[0]) + 4) == 0) {
        sheet.load_pcx(SocWinPcxName, 0, 10, 0xec);
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
        int cy = (WinScreenHeight - *MainPanelHeight) - 0x162;
        int flags = 0x1000000
            | (*reinterpret_cast<int *>(reinterpret_cast<char *>(ConsoleIfaceSlot) + 4)
               + reinterpret_cast<int>(ConsoleIfaceSlot));
        GraphicWin::init(cx, cy, 0x341, 0x162, 0, flags, 0, 0, 0);
    }
    GraphicWin::fill(9);
    {
        RECT box_rect;
        box_rect.left = 0x17;
        box_rect.top = 0;
        box_rect.right = 0x30a;
        box_rect.bottom = 0x162;
        reinterpret_cast<Buffer *>(self + 0x444)->box_sprite(&box_rect,
            &InterfaceBoxStyles[9]);
    }
    {
        int stride = factionID_ * 2099;
        int net_min = 10 - SocialAllocTable[stride + 1] - SocialAllocTable[stride];
        economyAllocOrig_ = net_min;
        economyAllocPending_ = net_min;
        economyAlloc3_ = net_min;
        int lo = SocialAllocTable[stride];
        psychAllocOrig_ = lo;
        psychAllocPending_ = lo;
        psychAlloc3_ = lo;
        int hi = SocialAllocTable[stride + 1];
        labsAllocOrig_ = hi;
        labsAllocPending_ = hi;
        labsAlloc3_ = hi;
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
                strcat(StringTemp, StringTable.get(SocialCategoryRow[row_accum + col]));
                CharUpperA(StringTemp);
                int idx = col + row * 4;
                RECT *cell = &tutRects3_[2 + idx];
                FlatButton *button = &flatButtons_[idx];
                reinterpret_cast<BaseButton *>(button)->init(StringTemp, idx,
                    cell->left, cell->top, cell->right - cell->left,
                    cell->bottom - cell->top, this, 0);
                button->hide();
                reinterpret_cast<Buffer *>(reinterpret_cast<char *>(button) + 0x444)
                    ->set_font(&font2_, 0, 0, 0);
                button->redraw();
                reinterpret_cast<BaseButton *>(button)->set_bubble_text(
                    StringTable.get(*reinterpret_cast<int *>(
                        reinterpret_cast<char *>(Labels.strings_ptr) + 0xCF0)));
                group->add(reinterpret_cast<BaseButton *>(button));
            }
            group->set(social_category_value(socCategory_, row), 1);
            ++row;
            row_accum += 0x35;
            ++group;
        } while (row_accum < 0xd4);
    }

    buttonGroup_.init(2, 0);
    {
        const char *name = SocialEffectRowBase;
        const char *name_end = SocialEffectRowBase + 0x138;
        const char *name_stop = SocialEffectRowBase + 0x478;
        int row = 0;
        int button_offset = 0;
        int entry = 0;
        while (name < name_stop) {
            if (name != name_end) {
                int col;
                int offset;
                if (name < name_end) {
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
                BaseButton *button = reinterpret_cast<BaseButton *>(
                    self + 0x1202C + offset);
                button->init(StringTemp, col + 0x11, x, y,
                    cell->right - x, cell->bottom - y, this, 0);
                button->hide();
                reinterpret_cast<Buffer *>(reinterpret_cast<char *>(button) + 0x444)
                    ->set_font(reinterpret_cast<Font *>(0), 0, 0, 0);
                button->redraw();
                reinterpret_cast<BaseButton *>(button)->set_bubble_text(
                    StringTable.get(*reinterpret_cast<int *>(
                        reinterpret_cast<char *>(Labels.strings_ptr)
                        + col * 4 + 0xCF4)));
            }
            name += 0x68;
            ++row;
            entry += 1;
            button_offset += 0xB4C;
        }
    }

    {
        StringTemp[0] = 0;
        strcat(StringTemp, StringTable.get(*reinterpret_cast<int *>(
            reinterpret_cast<char *>(Labels.strings_ptr) + 0x198)));
        if (*reinterpret_cast<int *>(0x009BC054) != 2) {
            CharUpperA(StringTemp);
        }
        FlatButton *button = &flatButtons_[16];
        reinterpret_cast<BaseButton *>(button)->init(StringTemp, 0x10,
            tutRect11_.left, tutRect11_.top, tutRect11_.right - tutRect11_.left,
            tutRect11_.bottom - tutRect11_.top, this, 0);
        button->hide();
        reinterpret_cast<Buffer *>(reinterpret_cast<char *>(button) + 0x444)
            ->set_font(reinterpret_cast<Font *>(0), 0, 0, 0);
        button->redraw();
    }

    reinterpret_cast<BaseButton *>(self + 0x19124)->set_bubble_text(
        StringTable.get(*reinterpret_cast<int *>(
            reinterpret_cast<char *>(Labels.strings_ptr) + 0x4F4)));

    reinterpret_cast<BaseButton *>(self + 0x19C70)->set_bubble_text(
        StringTable.get(*reinterpret_cast<int *>(
            reinterpret_cast<char *>(Labels.strings_ptr) + 0xD34)));

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

    // The six header toggles: the original reaches each at its own raw
    // offset (0x2D3 stride, receivers that are not FlatButton boundaries),
    // links it to one of the two arrow-sprite triplets, and reads its frame
    // from tutRects1_[3..8].
    static const int toggle_offsets[6] = {
        0x1A7BC, 0x1B308, 0x1BE54, 0x1C9A0, 0x1D4EC, 0x1E038 };
    static const int toggle_links[6] = {
        0x1B290, 0x1BDCC, 0x1C928, 0x1D474, 0x1DFC0, 0x1EB0C };
    static const int toggle_bubbles[6] = { 0xD24, 0xD24, 0xD28, 0xD28, 0xD2C, 0xD2C };
    for (int toggle_index = 0; toggle_index < 6; ++toggle_index) {
        BaseButton *toggle = reinterpret_cast<BaseButton *>(
            self + toggle_offsets[toggle_index]);
        const RECT &rect = tutRects1_[3 + toggle_index];
        toggle->init(0, 0x1d + toggle_index, rect.left, rect.top,
            rect.right - rect.left, rect.bottom - rect.top,
            &MainInterfaceGlobal, 0);
        toggle->hide();
        int first = (toggle_index % 2 == 0) ? 0 : 3;
        reinterpret_cast<int *>(self + toggle_links[toggle_index])[0]
            = reinterpret_cast<int>(self + 0x3DE8 + (first + 0) * 0x2C);
        reinterpret_cast<int *>(self + toggle_links[toggle_index])[1]
            = reinterpret_cast<int>(self + 0x3DE8 + (first + 1) * 0x2C);
        reinterpret_cast<int *>(self + toggle_links[toggle_index])[2]
            = reinterpret_cast<int>(self + 0x3DE8 + (first + 2) * 0x2C);
        toggle->set_bubble_text(StringTable.get(*reinterpret_cast<int *>(
            reinterpret_cast<char *>(Labels.strings_ptr)
            + toggle_bubbles[toggle_index])));
    }

    if (positive == 0) {
        if (tut_check2(8) != 0) {
            show(0);
            *TutorialStage = 5;
            int out_a = 0;
            int out_b = 0;
            TutorialWindow.soc_rect(TutorialSocRect, &out_a, &out_b);
            int *base_object = *reinterpret_cast<int **>(0x007D3C3C);
            int window_arg = 0;
            if (base_object != 0) {
                window_arg = *reinterpret_cast<int *>(
                    *reinterpret_cast<int *>(base_object) + 4)
                    + reinterpret_cast<int>(base_object);
            }
            TutorialWindow.tut_win(reinterpret_cast<GraphicWin *>(window_arg),
                TutSocWinText, out_a, out_b, 0, 1, -1, -1);
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

// ORIGINAL: 0x005B5620 ?social_select@@YAXH@Z 0x005B5620-0x005B5688 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/005b5620.cpp retained original identity/extent; pinned instructions5B5620..5B5688 establish flags, typed receivers and ordered calls; recovery/tests/test_social_select_behavior.py actual-source controls", "git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/005b5620.cpp"], "reason": "SocialWin owner opens the real shared window, synchronizes choices and applies them offline, preserving native entry-state guards and editor selection. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
// TRIED: homed into general.cpp 2026-09-03 - does not compile: the body calls
void __cdecl social_select(int faction_id) {
    const uint8_t state = static_cast<uint8_t>(GameState);
    if ((state & (STATE_SCENARIO_EDITOR | STATE_OMNISCIENT_VIEW)) == 0 &&
        (NetTurnFlags & 8u) != 0)
        return;

    int selected_faction = faction_id;
    if ((state & STATE_OMNISCIENT_VIEW) != 0) {
        selected_faction = ConsoleGlobal.editor_who(faction_id, 1);
        if (selected_faction < 0)
            return;
    }

    TutWinSocWindow.init(selected_faction);
    TutWinSocWindow.close();
    synch_soc(selected_faction);
    if (IsMultiplayerNet == 0)
        social_set(selected_faction);
}

// C++ owns member/base teardown after these explicit close/reset operations.
// ORIGINAL: 0x004B3C80 ??1SocialWin@@QAE@XZ 0x004B3C80-0x004B3EB7;0x00659700-0x00659878
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
    field_A2C_ = 0xFFFFFFFFU;
    field_A30_ = 0xFFFFFFFFU;
    field_2C28_ = 0;
    effectMaximize_ = 0xFFFFFFFFU;
}
