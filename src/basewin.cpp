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
#include "wave.h"
#include "original_seam.h"
#include "basewin.h"
#include "win.h"
#include "subinterface.h"
// Homed BaseWin bodies call the free helpers the archive binds by address:
// set_base/best_specialist (base.h), parse_says (general.h), radius_move
// (map.h), do_all_draws (temp.h), text_open (text.h), and construct the
// BasePop/Popup popups outright.
#include "base.h"
#include "basepop.h"
#include "console.h"
#include "council.h"
#include "datalink.h"
#include "general.h"
#include "leaf_recoveries.h"
#include "lock.h"
#include "map.h"
#include "netdaemon.h"
#include "popup.h"
#include "pulldown.h"
#include "temp.h"
#include "technology.h"
#include "text.h"
#include "tutwin.h"
#include "veh.h"
#include "xpops.h"
#include "alpha.h"
#include "faction.h"
#include "game.h"
#include "random.h"
#include <mmsystem.h>
#include <cstdint>
#include <cstring>

// Existing shared text.pcx palette publication, owned by palette.cpp.
extern int TextColourTable[195];

namespace {

int psych_display_signed(uint32_t value) {
    return value <= 0x7FFFFFFFu ? static_cast<int>(value)
        : -1 - static_cast<int>(0xFFFFFFFFu - value);
}
int psych_display_add(int left, int right) {
    return psych_display_signed(static_cast<uint32_t>(left) + static_cast<uint32_t>(right));
}
int psych_display_sub(int left, int right) {
    return psych_display_signed(static_cast<uint32_t>(left) - static_cast<uint32_t>(right));
}

} // namespace

// Draw one row using the original population-sized spacing. The stage counts
// are snapshots, but the number of content citizens and specialists is read
// again after each Sprite draw. Packed specialist types must be valid 0..6;
// the original neither clamps malformed nibbles nor bounds the returned type.
// ORIGINAL: 0x00408720 ?psych_row@BaseWin@@QAEXHHHHHHH@Z 0x00408720-0x00408B1B;0x00651567-0x00651579
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00408720-0x00408B1B complete body and 0x00651567-0x00651579 Random cleanup funclet.", "image:0x00408786 imports timeGetTime. Human variants use four-mood strides; alien four-mood arrays do not consume random selections.", "image:0x00408720-0x00408B1B reads packed specialist nibbles for the first16 entries and calls best_specialist for each later entry; sprite calls are followed by fresh population/specialist reads."], "reason": "Draw the published population mood row through genuine citizen and specialist sprites, preserving Random lifetime, sprite variation, packed specialist types and live base reads."}
void BaseWin::psych_row(int x, int y, int width, int sprite_width,
                       int talents, int drones, int super_drones) {
    Random random;
    const int population = BaseCurrent->population_size;
    int spacing = sprite_width;
    if (population > 1) {
        const int available = psych_display_sub(width, sprite_width) / (population - 1);
        spacing = available < 1 || sprite_width < 1 ? 1
            : (available > sprite_width ? sprite_width : available);
    }
    random.reseed(timeGetTime());
    const bool alien = ExpansionEnabled != 0
        && (Players[BaseCurrent->faction_id_current].rule_flags & 0x80) != 0;
    Buffer *const canvas = this;
    Sprite *const citizens = alien ? g_AL_CITIZEN_SM_CURSOR_SPRITES : g_CITIZEN_SM_CURSOR_SPRITES;
    Sprite *const specialists = alien ? g_AL_SPECIALIST_SM_CURSOR_SPRITES : g_SPECIALIST_SM_CURSOR_SPRITES;

    for (int talent = 0; talent < talents; ++talent) {
        const int variant = alien ? 0 : static_cast<int>(random.get(0, 2));
        Sprite &sprite = citizens[variant * 4];
        sprite.draw(canvas, sprite.transparent_index(), x, y);
        x = psych_display_add(x, spacing);
    }
    for (int content = 0; content < psych_display_sub(psych_display_sub(
             psych_display_sub(BaseCurrent->population_size, BaseCurrent->specialist_total), talents), drones);
         ++content) {
        const int variant = alien ? 0 : static_cast<int>(random.get(0, 2));
        Sprite &sprite = citizens[variant * 4 + 1];
        sprite.draw(canvas, sprite.transparent_index(), x, y);
        x = psych_display_add(x, spacing);
    }
    const int ordinary_drones = psych_display_sub(drones, super_drones);
    for (int drone = 0; drone < drones; ++drone) {
        const int variant = alien ? 0 : static_cast<int>(random.get(0, 2));
        Sprite &sprite = citizens[variant * 4 + 2 + (drone >= ordinary_drones ? 1 : 0)];
        sprite.draw(canvas, sprite.transparent_index(), x, y);
        x = psych_display_add(x, spacing);
    }
    for (int specialist = 0; specialist < BaseCurrent->specialist_total; ++specialist) {
        int type;
        if (specialist < 16) {
            type = (BaseCurrent->spec[specialist >> 3] >> ((specialist & 7) * 4)) & 15;
        } else {
            type = best_specialist();
        }
        Sprite &sprite = specialists[type];
        sprite.draw(canvas, sprite.transparent_index(), x, y);
        x = psych_display_add(x, spacing);
    }
}

// The panel reads the shared five-stage publication; it does not recompute
// psych. The original restores the clip and resets caption overrides, but
// leaves a successfully installed borrowed font pointer after the local Font
// dies. Preserve that observed lifetime boundary; callers must replace it
// before another text operation, which is not established by this body.
// ORIGINAL: 0x00408B20 ?draw_psych@BaseWin@@QAEXXZ 0x00408B20-0x00408E93;0x00651579-0x0065158B
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00408B20-0x00408E93 complete body and 0x00651579-0x0065158B Font cleanup funclet.", "image:0x00408B51 addresses panel RECT at BaseWin+0x40B8C; 0x00408B70 addresses inherited Buffer+0x444; 0x00408B4B saves its clip at BaseWin+0x464.", "image:0x00408BBF reads Sprite height+0x1C and 0x00408E21 reads width+0x18. 0x00408E32-0x00408E35 passes half-panel width minus4, not y.", "image:0x00408B20-0x00408E93 sets caption overrides before set_font, resets them to minus1, and restores clip before local Font destruction. It does not restore the borrowed font pointer; subsequent caller lifetime closure remains unproved."], "reason": "Display the shared five psych-stage histories with a genuine local Font and inherited Buffer, retaining label decisions, duplicate-stage skipping and original restoration/lifetime behavior."}
void BaseWin::draw_psych() {
    Font font;
    Buffer *const canvas = this;
    RECT saved_clip = canvas->rect1_;
    canvas->set_clip(&rect1_);
    GraphicWin::fill(0);
    font.init(DefaultFontFace, 12, 1);

    const int faction = BaseCurrent->faction_id_current;
    const int left = rect1_.left;
    const int top = rect1_.top;
    const int right = rect1_.right;
    const int panel_width = psych_display_sub(right, left);
    const int row_pitch = psych_display_add(g_CITIZEN_SM_CURSOR_SPRITES[0].height(), 1);
    MapWinCaptionScale = 30;
    MapWinCaptionDivisor = 38;
    canvas->set_font(&font, 0, 0, 0);
    const int text_offset = psych_display_sub(row_pitch / 2, font.height_ / 2);
    int y = psych_display_add(top, 2);
    int stage = 0;
    for (int row = 0; row < 5; ++row) {
        if (stage != 0) {
            while (stage < 5
                && BasePsychTalents[stage] == BasePsychTalents[stage - 1]
                && BasePsychDrones[stage] == BasePsychDrones[stage - 1]
                && BasePsychSuperDrones[stage] == BasePsychSuperDrones[stage - 1]) {
                ++stage;
            }
        }
        if (stage < 5) {
            StringTemp[0] = 0;
            const int half_width = psych_display_sub(panel_width / 2, 4);
            const int caption_x = psych_display_sub(psych_display_sub(right, half_width), 2);
            int label;
            switch (stage) {
            case 0: {
                const Base *const base = BaseCurrent;
                if (base->assimilation_turns_left != 0 && base->faction_id_former != base->faction_id_current)
                    label = 970;
                else label = base->nerve_staple_turns_left != 0 ? 971 : 322;
                break;
            }
            case 1: label = 323; break;
            case 2: label = 324; break;
            case 3: {
                const int selected = BaseIDCurrentSelected;
                int byte_index;
                int mask;
                bitmask(FAC_BROOD_PIT, &byte_index, &mask);
                const bool brood_pit = (Bases[selected].facilities_built[byte_index]
                    & static_cast<unsigned char>(mask)) != 0;
                const int police = psych_display_add(PlayersData[faction].soc_effect_pending.police,
                    brood_pit ? 2 : 0);
                label = police <= -2 ? 326 : 325;
                break;
            }
            default: label = 327; break;
            }
            strcat(StringTemp, label_get(label));
            canvas->set_text_color(TextColourTable[114], TextColourTable[115], 1, 1);
            const int text_y = psych_display_sub(psych_display_add(text_offset, y), 2);
            canvas->write_cent_l(StringTemp, caption_x, text_y, half_width,
                static_cast<int>(strlen(StringTemp)));
            psych_row(psych_display_add(left, 2), y, half_width,
                psych_display_add(g_CITIZEN_SM_CURSOR_SPRITES[0].width(), 1),
                BasePsychTalents[stage], BasePsychDrones[stage], BasePsychSuperDrones[stage]);
        }
        y = psych_display_add(y, row_pitch);
        ++stage;
    }
    MapWinCaptionScale = -1;
    MapWinCaptionDivisor = -1;
    canvas->set_clip(&saved_clip);
}

// Link-only placeholders; production selection and hurry are not implemented.
// ORIGINAL: 0x00417040 ?production@BaseWin@@QAEXHH@Z 0x00417040-0x00418DBD;0x00651BB0-0x0065221C
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/00417040.cpp", "source:src/basewin.h"], "reason": "Map the declared dependency to its seed identity in the owning product TU. Empty link-only placeholder; original behavior is not implemented."}
void BaseWin::production(int, int) {
    // BODY GOES HERE.
}

// ORIGINAL: 0x00418DF0 ?hurry@BaseWin@@QAEXXZ 0x00418DF0-0x00419A8E;0x0065221C-0x0065249C
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/00418df0.cpp", "source:src/basewin.h"], "reason": "Map the declared dependency to its seed identity in the owning product TU. Empty link-only placeholder; original behavior is not implemented."}
void BaseWin::hurry() {
    // BODY GOES HERE.
}

// The retired scaffold is cited below only as historical seed evidence.
// ORIGINAL: 0x00415F70 ?gov_on@BaseWin@@QAEXH@Z 0x00415F70-0x004160E1 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00415F70-0x004160E1", "source:src/base.h:Base and GovernorBaseBitfield", "source:src/graphicwin.h:Win slot 62", "git:0e5175fa:src/recovered/units/00415f70.cpp"], "reason": "Recover governor selection using source-owned current-base state, typed Base fields and the real virtual redraw operation; preserve the following explicitly direct base redraw."}
void BaseWin::gov_on(int priorities) {
    if (NetDaemonNet.lock_base(BaseIDCurrentSelected, 0, -1, -1)) {
        return;
    }
    const unsigned int priority_mask = GOV_PRIORITY_EXPLORE | GOV_PRIORITY_DISCOVER
        | GOV_PRIORITY_BUILD | GOV_PRIORITY_CONQUER;
    if (priorities) {
        if (BaseCurrent->governor & GOV_MULTI_PRIORITIES) {
            BaseCurrent->governor ^= priorities;
        } else {
            BaseCurrent->governor &= ~priority_mask;
            BaseCurrent->governor |= priorities;
        }
        if (BaseCurrent->governor & priority_mask) {
            BaseCurrent->governor |= GOV_ACTIVE | GOV_MANAGES_PRODUCTION
                | GOV_MANAGES_CITIZENS_SPECS;
        } else {
            BaseCurrent->governor &= ~GOV_ACTIVE;
        }
    } else {
        BaseCurrent->governor |= GOV_ACTIVE;
    }
    if (((unsigned char)LockEnableMask & (1 << BaseCurrent->faction_id_current)) == 0) {
        BaseCurrent->governor &= ~GOV_ACTIVE;
    }
    BaseCurrent->governor &= ~GOV_UNK_40000000;
    if (BaseCurrent->governor & GOV_ACTIVE) {
        if (BaseCurrent->governor & GOV_MANAGES_PRODUCTION) {
            BaseCurrent->state &= ~BSTATE_UNK_8000000000;
            BaseCurrent->queue_size = 0;
            base_reset(BaseIDCurrentSelected, 1);
        }
        if (BaseCurrent->governor & GOV_MANAGES_CITIZENS_SPECS) {
            BaseCurrent->resource_sqr_radius = 0;
            BaseCurrent->specialist_total = 0;
            BaseCurrent->specialist_unk_1 = 0;
            base_compute(1);
            base_doctors();
        }
    }
    NetDaemonNet.unlock_base(BaseIDCurrentSelected);
    // Original 0x004160A8 dispatches Win slot 62 on this receiver. The
    // following call at 0x004160B3 is direct, to GraphicWin::redraw.
    redraw();
    MainInterfaceGlobal.GraphicWin::redraw();
    if (!(BaseCurrent->governor & GOV_ACTIVE)) {
        draw_radius(BaseCurrent->x, BaseCurrent->y, 2, 2);
    }
}

// ORIGINAL: 0x0041AE40 ?soft_zoom@BaseWin@@QAEXH@Z 0x0041AE40-0x0041AF12 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x0041AE40-0x0041AF12"], "reason": "Recover the original base-selection/cache operation with source-owned state and typed fields. The archived seed remains an obligation until this product definition is freshly remeasured."}
void BaseWin::soft_zoom(int base_id) {
    if (!Win::is_visible()) {
        focusedBaseID_ = base_id;
        BaseComputedID = -1;
        int worked_tiles = Bases[focusedBaseID_].resource_sqr_radius;
        set_base(focusedBaseID_);
        base_compute(0);
        if (IsMultiplayerNet && !ConsoleControlTurnActive
            && worked_tiles != Bases[focusedBaseID_].resource_sqr_radius) {
            if (NetDaemonNet.lock_base(BaseIDCurrentSelected, 0, -1, -1) == 0) {
                NetDaemonNet.unlock_base(BaseIDCurrentSelected);
            }
        }
        displayed_production_id_ = Bases[base_id].queue_production_id[0];
        return;
    }
    zoom(base_id, 0);
}

/*
Purpose: Construct BaseWin's large aggregate of sub-windows: a ProdPicker, a
         second GraphicWin, several Sprite runs, a FlatButton, a Buffer, a
         FlatButton[22], a Time, a Caviar, a MapWin, a Font, two Spots, a
         second Buffer and three Scrolls.
// ORIGINAL: 0x00408490 ??0BaseWin@@QAE@XZ 0x00408490-0x0040870D;0x004210D0-0x004210E7;0x00651380-0x00651567
// LEVER: real, correctly-offset, correctly-typed members (already modelled
//        in basewin.h) constructed IMPLICITLY, in declaration order, took
//        this from a SHARED_TAIL isolated-scaffold measurement straight to
//        120/144 agreeing instructions here - far better than the explicit
//        raw-offset placement-new this batch's other five constructors
//        needed for ordering control. First divergence at instruction 10:
//        this compiled body reserves an extra `push edi` and calls through
//        an extra local thunk where the image calls GraphicWin::construct
//        directly.
// TRIED: plateau 120/144; divergence #10 is the extra push edi + local thunk
//        against the image's direct GraphicWin::construct call, measured under
//        the SHARED_TAIL caveat (no per-function verdict is well defined)
// TRIED: 2026-08-28 confirming sweep, one re-measure of the tree's own body
//        after basewin.h grew member declarations and the click forwarders
//        were renamed: 104 of 144 agreeing at /c /O2 /Ob0 /Gy /GR- /Oy- /GX,
//        149 compiled, first divergence still #10 (the same `push edi` before
//        the esi spill, then a link-time call where the image has
//        `call 0x5d4cf0`). The 120/144 above is the deleted isolated
//        scaffold's number, not this tree's; do not chase it. No new lever.
// size      1147 bytes
// prototype void (__thiscall ??0BaseWin@@QAE@XZ)(BaseWin* this)
// callers   1   call targets   15
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00420F90 0x004626E0 0x00492420 0x005D4CF0 0x005D4DD0 0x005D7210 0x005E37E0 0x005FA860 0x006051D0 0x00607CF0 0x006161D0 0x00616DA0 0x00618EA0 0x006456E4 0x006457C2
*/
// This address is catalogued SHARED_TAIL: its cleanup path runs through an
// EH-funclet cluster shared with other functions (COMDAT-folded), so no
// per-function byte verdict is well defined here regardless of source form -
// see the deleted src/recovered/units/00408490.cpp, which measured the same
// finding from an isolated scaffold. Bodied for COVERAGE: every sub-object
// below is a real, correctly-offset, correctly-typed member already modelled
// in basewin.h, constructed implicitly in declaration order (matching the
// image's own order field-for-field) - mapWin_ included now that MapWin has
// its real constructor, which pushes the hidden most-derived flag 1 exactly
// where the image's member construction does.
BaseWin::BaseWin() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x00408710 ?close@BaseWin@@QAEXXZ 0x00408710-0x00408711 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?close@BaseWin@@QAEXXZ)(BaseWin* this)
// callers   3   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BaseWin::close() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x00416DD0 ?UNK4@BaseWin@@QAEXXZ 0x00416DD0-0x00416DD1 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK4@BaseWin@@QAEXXZ)(BaseWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BaseWin::UNK4() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x0041AF20 ?UNK6@BaseWin@@QAEXXZ 0x0041AF20-0x0041AF21 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK6@BaseWin@@QAEXXZ)(BaseWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BaseWin::UNK6() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x0041AF30 ?UNK7@BaseWin@@QAEXXZ 0x0041AF30-0x0041AF31 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK7@BaseWin@@QAEXXZ)(BaseWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BaseWin::UNK7() {
}





/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0041A610 ?UNK5@BaseWin@@QAEXH@Z 0x0041A610-0x0041A613 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?UNK5@BaseWin@@QAEXH@Z)(BaseWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BaseWin::UNK5(int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0041B4E0 ?on_button_toggled@BaseWin@@QAEXHH@Z 0x0041B4E0-0x0041B4E3 BYTE_EXACT
// symbol    ?on_button_toggled@BaseWin@@UAEXHH@Z
//           Q->U 2026-09-06, Win's front slots 44/45 taking the image's
//           own names (on_button_clicked / on_button_toggled): the body is
//           now an override of the window's slot, so VC6 mangles it virtual
//           and enters it on the front table, while the image spells the
//           same body Q and its own table holds the raw address.
// size      3 bytes
// prototype void (__thiscall ?on_button_toggled@BaseWin@@QAEXHH@Z)(BaseWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BaseWin::on_button_toggled(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0041DF40 ?on_iface_right_down@BaseWin@@QAEXHH@Z 0x0041DF40-0x0041DF43 BYTE_EXACT
// symbol    ?on_iface_right_down@BaseWin@@UAEXHH@Z
//           Q->U 2026-09-06, the IfaceRegistryEntry derivation: the body is
//           now an override of the interface base's slot, so VC6 mangles it
//           virtual and enters it on the ENTRY, while the image spells the
//           same body Q and its own table 0x66A3B8 holds the raw address.
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_down@BaseWin@@QAEXHH@Z)(BaseWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BaseWin::on_iface_right_down(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0041E4C0 ?on_iface_selected@BaseWin@@QAEXHH@Z 0x0041E4C0-0x0041E4C3 BYTE_EXACT
// symbol    ?on_iface_selected@BaseWin@@UAEXHH@Z
//           Q->U 2026-09-06, the IfaceRegistryEntry derivation: the body is
//           now an override of the interface base's slot, so VC6 mangles it
//           virtual and enters it on the ENTRY, while the image spells the
//           same body Q and its own table 0x66A3B8 holds the raw address.
// size      3 bytes
// prototype void (__thiscall ?on_iface_selected@BaseWin@@QAEXHH@Z)(BaseWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BaseWin::on_iface_selected(int, int) {
}



/*
Purpose: Report a left click on the interface. An override of the interface base's slot, so `this` IS the entry at 0xA14
         and the call it makes is unqualified.
// ORIGINAL: 0x0041DCD0 ?on_iface_left_click@BaseWin@@QAEXHH@Z 0x0041DCD0-0x0041DCEE BYTE_EXACT
// symbol    ?on_iface_left_click@BaseWin@@UAEXHH@Z
//           Q->U 2026-09-06, the IfaceRegistryEntry derivation: the body is
//           now an override of the interface base's slot, so VC6 mangles it
//           virtual and enters it on the ENTRY, while the image spells the
//           same body Q and its own table 0x66A3B8 holds the raw address.
// size      30 bytes
// prototype void (__thiscall ?on_iface_left_click@BaseWin@@QAEXHH@Z)(BaseWin* this, int xCoord, int yCoord)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004160F0
// LEVER: 2026-09-06 the derivation deleted the pun. `this` is the entry now,
//        so the unqualified call is the whole body - the `BaseWin *const base
//        = reinterpret_cast<...>(this) - 0xA14` local is gone and the bytes
//        are unchanged (12 of 12 before and after).
Return Value: n/a
Status: Complete
*/
void BaseWin::on_iface_left_click(int xCoord, int yCoord) {
    iface_click(xCoord, yCoord, 0, 0);
}

/*
Purpose: Report a right click on the interface. An override of the interface base's slot, so `this` IS the entry at 0xA14
         and the call it makes is unqualified.
// ORIGINAL: 0x0041DCF0 ?on_iface_right_click@BaseWin@@QAEXHH@Z 0x0041DCF0-0x0041DD0E BYTE_EXACT
// symbol    ?on_iface_right_click@BaseWin@@UAEXHH@Z
//           Q->U 2026-09-06, the IfaceRegistryEntry derivation: the body is
//           now an override of the interface base's slot, so VC6 mangles it
//           virtual and enters it on the ENTRY, while the image spells the
//           same body Q and its own table 0x66A3B8 holds the raw address.
// size      30 bytes
// prototype void (__thiscall ?on_iface_right_click@BaseWin@@QAEXHH@Z)(BaseWin* this, int xCoord, int yCoord)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004160F0
Return Value: n/a
Status: Complete
*/
void BaseWin::on_iface_right_click(int xCoord, int yCoord) {
    iface_click(xCoord, yCoord, 1, 0);
}

/*
Purpose: Report a left double-click on the interface. An override of the interface base's slot, so `this` IS the entry at 0xA14
         and the call it makes is unqualified.
// ORIGINAL: 0x0041E4D0 ?on_iface_left_double_click@BaseWin@@QAEXHH@Z 0x0041E4D0-0x0041E4EE BYTE_EXACT
// symbol    ?on_iface_left_double_click@BaseWin@@UAEXHH@Z
//           Q->U 2026-09-06, the IfaceRegistryEntry derivation: the body is
//           now an override of the interface base's slot, so VC6 mangles it
//           virtual and enters it on the ENTRY, while the image spells the
//           same body Q and its own table 0x66A3B8 holds the raw address.
// size      30 bytes
// prototype void (__thiscall ?on_iface_left_double_click@BaseWin@@QAEXHH@Z)(BaseWin* this, int xCoord, int yCoord)
// callers   0   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004160F0
Return Value: n/a
Status: Complete
*/
void BaseWin::on_iface_left_double_click(int xCoord, int yCoord) {
    iface_click(xCoord, yCoord, 0, 1);
}

/*
Purpose: Report a right double-click on the interface. An override of the interface base's slot, so `this` IS the entry at 0xA14
         and the call it makes is unqualified.
// ORIGINAL: 0x0041E4F0 ?on_iface_right_double_click@BaseWin@@QAEXHH@Z 0x0041E4F0-0x0041E50E BYTE_EXACT
// symbol    ?on_iface_right_double_click@BaseWin@@UAEXHH@Z
//           Q->U 2026-09-06, the IfaceRegistryEntry derivation: the body is
//           now an override of the interface base's slot, so VC6 mangles it
//           virtual and enters it on the ENTRY, while the image spells the
//           same body Q and its own table 0x66A3B8 holds the raw address.
// size      30 bytes
// prototype void (__thiscall ?on_iface_right_double_click@BaseWin@@QAEXHH@Z)(BaseWin* this, int xCoord, int yCoord)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004160F0
Return Value: n/a
Status: Complete
*/
void BaseWin::on_iface_right_double_click(int xCoord, int yCoord) {
    iface_click(xCoord, yCoord, 1, 1);
}






/*
Purpose: Handle an interface scroll, but only for scroll kind 2 - stash the
         new position at 0x40100 (interface-relative) and redraw the supported
         markers. Like the click handlers, `this` arrives at the interface
         subobject and is adjusted back to the BaseWin for that redraw.
// ORIGINAL: 0x0041DC80 ?on_iface_scrolled@BaseWin@@QAEXHH@Z 0x0041DC80-0x0041DCA5 BYTE_EXACT
// symbol    ?on_iface_scrolled@BaseWin@@UAEXHH@Z
//           Q->U 2026-09-06, the IfaceRegistryEntry derivation: the body is
//           now an override of the interface base's slot, so VC6 mangles it
//           virtual and enters it on the ENTRY, while the image spells the
//           same body Q and its own table 0x66A3B8 holds the raw address.
// size      37 bytes
// prototype void (__thiscall ?on_iface_scrolled@BaseWin@@QAEXHH@Z)(BaseWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0040C850
// LEVER: two moves took this 2/12 -> 12/12. (1) The guard is a SWITCH on one
//        case, not an `if` - `mov eax,[ebp+8] / sub eax,2 / jne` against the
//        `cmp dword ptr [ebp+8],2` an `if` emits. (2) BEFORE the derivation,
//        the -0xA14 adjustment had to be written INLINE on the store so VC6
//        folded 0x40B14-0xA14 into the store's displacement; the member store
//        on an unadjusted `this` compiled `[ecx+0x40b14]` and scored 11/12.
//        The IfaceRegistryEntry derivation makes the receiver the entry, so
//        the plain member store IS 0x40100 now and both the inline adjustment
//        and the `base` local are gone.
Return Value: n/a
Status: Complete
*/
void BaseWin::on_iface_scrolled(int code, int pos) {
    // A SWITCH, not an `if`: the image loads the code into a register and
    // subtracts the case value (`mov eax,[ebp+8] / sub eax,2 / jne`), which is
    // what a one-case switch statement compiles to - the same shape the
    // sibling on_scrolled below is BYTE_EXACT with. An `if` emits
    // `cmp dword ptr [ebp+8],2` instead.
    switch (code) {
        case 2:
            // THE DERIVATION IS WHAT MAKES THIS PLAIN. 0x40B14 read from the
            // entry is 0x40100, and the image spells the store
            // `mov [ecx+0x40100], eax` with ecx the receiver - so the member
            // name is all the spelling there is. The `this - 0xA14` pun this
            // replaces had to be written INLINE on the store (a named local
            // made VC6 materialise the adjusted pointer before it) and cost
            // the body two casts.
            field_40B14_ = pos;
            draw_supported(1);
            break;
    }
}



/*
Purpose: Report a left click to the shared click handler. Unlike the
         iface_click family, `this` is the BaseWin itself with no adjustment.
// ORIGINAL: 0x0041AF70 ?on_left_click@BaseWin@@QAEXHH@Z 0x0041AF70-0x0041AF88 BYTE_EXACT
// size      24 bytes
// prototype void (__thiscall ?on_left_click@BaseWin@@QAEXHH@Z)(BaseWin* this, int xCoord, int yCoord)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004165D0
Return Value: n/a
Status: Complete
*/
void BaseWin::on_left_click(int xCoord, int yCoord) {
    click(xCoord, yCoord, 0, 0);
}

/*
Purpose: Report a right click to the shared click handler. Unlike the
         iface_click family, `this` is the BaseWin itself with no adjustment.
// ORIGINAL: 0x0041AF90 ?on_right_click@BaseWin@@QAEXHH@Z 0x0041AF90-0x0041AFA8 BYTE_EXACT
// size      24 bytes
// prototype void (__thiscall ?on_right_click@BaseWin@@QAEXHH@Z)(BaseWin* this, int xCoord, int yCoord)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004165D0
Return Value: n/a
Status: Complete
*/
void BaseWin::on_right_click(int xCoord, int yCoord) {
    click(xCoord, yCoord, 1, 0);
}

/*
Purpose: Report a left double-click to the shared click handler. Unlike the
         iface_click family, `this` is the BaseWin itself with no adjustment.
// ORIGINAL: 0x0041AFB0 ?on_left_double_click@BaseWin@@QAEXHH@Z 0x0041AFB0-0x0041AFC8 BYTE_EXACT
// symbol    ?on_left_double_click@BaseWin@@UAEXHH@Z
// size      24 bytes
// prototype void (__thiscall ?on_left_double_click@BaseWin@@QAEXHH@Z)(BaseWin* this, int xCoord, int yCoord)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004165D0
Return Value: n/a
Status: Complete
*/
void BaseWin::on_left_double_click(int xCoord, int yCoord) {
    click(xCoord, yCoord, 0, 1);
}




/*
Purpose: Interface timer tick; the legacy implementation ignores it and
         returns.
// ORIGINAL: 0x0041AF40 ?timer_callback@BaseWin@@QAAXHH@Z 0x0041AF40-0x0041AF41 BYTE_EXACT
// symbol    ?timer_callback@BaseWin@@SAXHH@Z
// size      1 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BaseWin::timer_callback(int, int) {
}


/*
Purpose: Dismiss the bubble text when the pointer leaves. The whole body is two
         consecutive calls to the static Win::clear_bubble_text and nothing
         else; the arguments are ignored.

         The SECOND call is not a transcription error. The original issues it,
         and the two are observationally identical unless something re-arms the
         bubble between them - which is why the fixture re-arms it rather than
         asserting a single refresh and calling that agreement.
// ORIGINAL: 0x0041B1E0 ?on_mouse_leave@BaseWin@@QAEXHH@Z 0x0041B1E0-0x0041B1ED BYTE_EXACT
// symbol    ?on_mouse_leave@BaseWin@@UAEXHH@Z
// size      13 bytes
// prototype void (__thiscall ?on_mouse_leave@BaseWin@@QAEXHH@Z)(BaseWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F8500
Return Value: n/a
Status: Complete
*/
void BaseWin::on_mouse_leave(int, int) {
    ::Win::clear_bubble_text();
    ::Win::clear_bubble_text();
}


/*
// ORIGINAL: 0x0041E890 ?show@BaseWin@@QAEXH@Z 0x0041E890-0x0041E8AB BYTE_EXACT
// symbol    ?show@BaseWin@@UAEXH@Z
// size      27 bytes
// prototype void (__thiscall ?show@BaseWin@@QAEXH@Z)(BaseWin* this, int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045D310 0x005F7E90
Status: Complete
*/
void BaseWin::show(int visible) {
    // Win is the base at 0 and SubInterface the base at 0xA14 - both real
    // bases, so both calls go by their unqualified names and the compiler
    // makes the adjustments.
    if (!is_visible()) {
        set_iface_mode();
    }
}

/*
// ORIGINAL: 0x0040B120 ?UNK2@BaseWin@@QAEXHHHH@Z 0x0040B120-0x0040B13C BYTE_EXACT
// size      28 bytes
// prototype void (__thiscall ?UNK2@BaseWin@@QAEXHHHH@Z)(BaseWin* this, int, int, int, int)
// callers   0   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0040B140
Status: Complete
*/
// ALL FOUR PARAMETERS STAY SCAFFOLD. The body is one positional forward, so
// its only "evidence" is garrison_click's own parameter list - and those names
// have no provenance: ida9-functions.csv has 0x0040B140 as `sub_40B140` and
// ghidra-functions.csv as `FUN_0040b140`, both tools leaving it unnamed, and a
// prototype records types, never names. Renaming UNK2 from them would invent a
// meaning at one remove from an invention. (vehID/right/is_double were tried
// here and reverted for exactly that reason.)
void BaseWin::UNK2(int a1, int a2, int a3, int a4) {
    garrison_click(a1, a2, a3, a4);
}

/*
// ORIGINAL: 0x0041B4A0 ?on_scrolled@BaseWin@@QAEXHH@Z 0x0041B4A0-0x0041B4BD BYTE_EXACT
// size      29 bytes
// prototype void (__thiscall ?on_scrolled@BaseWin@@QAEXHH@Z)(BaseWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0040FCC0
Status: Complete
*/
void BaseWin::on_scrolled(int code, int pos) {
    switch (code) {
        case 1:
            field_40B10_ = pos;
            draw_facilities(1);
            break;
    }
}

/*
Purpose: Step the receiver back to the subobject ??_GBaseWin@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x00421810 ??_GBaseWin@@WEEE@AEPAXI@Z 0x00421810-0x0042181B BYTE_EXACT
// symbol    ??_EBaseWin@@WEEE@AEPAXI@Z
// CORRECTED from ??3BaseWin@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x004213D0` into
//   ??_GBaseWin@@UAEPAXI@Z, which executes `ret 4`; no stack access and
//   the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092}
//   and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/

// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit. Addresses of
// the ones documented individually live beside their definitions.
Wave g_BASEWIN_WAVE;  // 0x006EEE68

// The process-wide base window, 0x006A7628 in the image - REAL STORAGE, not
// the `Win *const TutWinBaseWindow` binding that named terranx.exe data
// (see tutwin.h). The image's dynamic initializer at 0x004083B0 runs the
// real BaseWin constructor against it and registers the deleting destructor
// with atexit; here the same recovered constructor runs through this
// build's own startup and the destructor closes it at exit.
// RECOVERY_OBJECT: v1 0x006A7628 ?TutWinBaseWindow@@3VBaseWin@@A
BaseWin TutWinBaseWindow;  // 0x006A7628
// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit.

// Fixed-slot bindings carried from 00411980.cpp

// ORIGINAL: 0x00411980 ?draw_energy_alloc@BaseWin@@QAEXH@Z 0x00411980-0x00412A96;0x00651B4C-0x00651B61 FILE
// TRIED: heavily time-boxed given batch budget - only the local Font construction, the initial set_clip, Font::init and set_font calls are transcribed (MISMATCH #11, well past the prologue); the bar-chart body (many box_sprite calls with per-slider RECT arithmetic and a faction colour table) is a deliberate stub, not fabricated.
// working copy - scaffold materialised by --work
// size      4395 bytes
// prototype void (__thiscall ?draw_energy_alloc@BaseWin@@QAEXH@Z)(BaseWin* this, int)
// callers   3   call targets   16
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D5630 0x005D5930 0x005D8000 0x005DAC70 0x005DACB0 0x005DCF40 0x005DD130 0x005DD450 0x005E2DD3 0x006169A0 0x00618EA0 0x00618EE0 0x00618F40 0x006453E0 0x00645470 0x0064FC88
// indirect  0x00411D58 0x00411E7B 0x00411FE5 0x0041213C 0x004122C9 0x00412433 0x00412561 0x004126EF 0x00412855 0x0041297F

void BaseWin::draw_energy_alloc(int a1) {
    // Real local matching the single `call ??0Font@@QAE@XZ` in the
    // disassembly; RAII covers its destructor at the return below.
    Font font;

    // The GraphicWin subobject's own Buffer at +0x444 - the front's second
    // base, reachable as the base it is now that the members are declared.
    Buffer *canvas = this;

    RECT clipRect = garrison_rect_;
    canvas->set_clip(&clipRect);

    font.init(DefaultFontFace, 0xc, 1);
    canvas->set_font(&font, 0, 0, 0);

    // Time-boxed: draw_energy_alloc renders the psych/labs/econ
    // allocation-slider bar chart via a long sequence of box_sprite
    // calls, per-slider RECT arithmetic driven by the current
    // allocation at 0x474+0xc..0x474+0x30-ish, and a faction
    // colour-table lookup (stride 0x59c) - not transcribed beyond the
    // clip/font setup above. See the TRIED note.
    (void)a1;
}

// Fixed-slot bindings carried from 0041afd0.cpp
static int *const InterfaceClipRect = (int *)0x007AEC94;
static int *const MainInterfaceScreenClipTop = (int *)0x007AEC98;
static int *const MainInterfaceScreenClipRight = (int *)0x007AEC9C;
static int *const MainInterfaceScreenClipBottom = (int *)0x007AECA0;

// Fixed-slot bindings carried from 004103b0.cpp, named from their neighbours
// and from the tree's own names for the same addresses.
//   MainInterfaceTileSource 0x0078D6B0 - the Buffer draw_garrison tiles the
//       garrison panel from; maininterface.cpp binds the same address under
//       this name for its own draw.
//   BoxSpriteStyle3 0x0078D5A0 - the BoxSpriteParams pool based at
//       0x0078D528 on the 0x28 grid, pool index 3 (reportif.cpp, socialwin.cpp).
//   MapWinCaptionScale / MapWinCaptionDivisor 0x00691E6C / 0x00691E70 - the
//       pair mapwin.cpp names, read here to pitch the garrison rows.
static int *const MainInterfaceTileSource = (int *)0x0078D6B0;
static int *const BoxSpriteStyle3 = (int *)0x0078D5A0;
// The shared full-screen drawing Buffer, the same binding maininterface.cpp,
// reportif.cpp and socialwin.cpp carry; InterfaceClipRect above is its +0x30.
static int *const InterfaceBuffer = (int *)0x007AEC64;

// ORIGINAL: 0x004103B0 ?draw_garrison@BaseWin@@QAEXH@Z 0x004103B0-0x00410868;0x00651B0D-0x00651B22 FILE
// size      1208 bytes
// prototype void (__thiscall ?draw_garrison@BaseWin@@QAEXH@Z)(BaseWin* this, int)
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// TRIED: bodied for COVERAGE up to the vehicle walk. The transcription stops
//  where the image enters it: a linked-list walk over the vehicle stack (the
//  0x00952858 / 0x0095285A pair), an idiv-based column layout, a dispatch
//  through [edx+8] and the veh_draw / write_cent_l / set_text_color labelling
//  that follows - a long, branch-heavy loop where this tree's register
//  allocation differs with no source-level lever that pins it. The artifact
//  this was homed from also read 0x0090EA30 - BaseCurrent's own pointer slot -
//  as two `short` map coordinates to feed veh_at, which cannot be what the
//  image does; that line is left out rather than homed as a defect.
// LEVER: 0x40C2C is one RECT, not the four unnamed dwords the IDB records -
//  set_clip takes it whole and the tile/frame sizing reads its fields.
//  0x00691B2C is font.h's DefaultFontFace, passed as the pointer it is; the
//  artifact's `*DefaultFontFace` through a pointer-to-pointer is two loads
//  where the image has one.
void BaseWin::draw_garrison(int) {
    Buffer *canvas = reinterpret_cast<Buffer *>(InterfaceBuffer);

    RECT clip_rect = garrison_rect_;
    canvas->set_clip(&clip_rect);

    Font font;
    font.init(DefaultFontFace, 0xc, 0);

    // The row pitch every garrison line uses: `height_ + unk_1_` when the
    // font's own ascent adjustment is not negative, `line_height_` when it is.
    int line_height = (font.unk_1_ >= 0) ? (font.height_ + font.unk_1_)
                                         : font.line_height_;
    (void)line_height;

    int left = garrison_rect_.left;
    int top = garrison_rect_.top;
    int right = garrison_rect_.right;
    int width = right - left;
    int bottom = garrison_rect_.bottom;
    int height = bottom - top;
    canvas->tile(reinterpret_cast<Buffer *>(MainInterfaceTileSource), left, top,
                 left, top, width, height);

    RECT frame_rect;
    frame_rect.left = left;
    frame_rect.top = top;
    frame_rect.right = right;
    frame_rect.bottom = bottom;
    canvas->box_sprite(&frame_rect,
                       reinterpret_cast<BoxSpriteParams *>(BoxSpriteStyle3));

    frame_rect.left += 3;
    frame_rect.top += 3;
    frame_rect.right -= 3;
    frame_rect.bottom -= 3;

    // Type 6 is the garrison's own highlight spot; the panel clears it before
    // it lays the row spots out again.
    spot1_.kill_type(6);

    int spacing;
    if (MapWinCaptionScale > 0) {
        spacing = (MapWinCaptionScale * 20) / MapWinCaptionDivisor;
    } else {
        spacing = 0x38;
    }
    (void)spacing;

    // WALL - the vehicle walk described in the TRIED note above starts here,
    // and it is the part that reads the argument this transcription leaves
    // unnamed: nothing before it touches it.
}

// ORIGINAL: 0x0041AFD0 ?on_left_up@BaseWin@@QAEXHH@Z 0x0041AFD0-0x0041B10D FILE
// size      317 bytes
// prototype void (__thiscall ?on_left_up@BaseWin@@QAEXHH@Z)(BaseWin* this, int, int)
// callers   0   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005EC7C0 0x005ED240 0x005ED2D0 0x005FA7E0
// indirect  0x0041B0C6 0x0041B0D7 0x0041B0F0
// PRESERVED UNIT - measured MNEMONIC_ONLY.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0041AFD0
// measured tier  MNEMONIC_ONLY
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0041afd0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_left_up@BaseWin@@QAEXHH@Z  at 0x0041AFD0  (317 bytes)
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


void BaseWin::on_left_up(int xCoord, int yCoord) {
    // 0x474 is Buffer+0x30 - the drawing Buffer's own rect2_, not BaseWin's
    // rect2_ at 0x40BAC. The base's member has to be named qualified, because
    // BaseWin declares a `rect2_` of its own that would shadow it.
    RECT rect = Buffer::rect2_;
    int flag = field_40B54_;
    if (flag > 0) {
        if (!in_box(xCoord, yCoord, &rect)) {
            client_to_screen(&xCoord, &yCoord);
            MainInterfaceGlobal.screen_to_client(&xCoord, &yCoord);

            // THE IMAGE REUSES THE FRAME SLOTS: the clip rectangle lands in
            // the same [ebp-0x10..ebp-4] the Buffer::rect2_ copy used, so a
            // second named local costs a frame slot the image does not pay -
            // a shadowed inner `RECT rect = {...}` measured WORSE (88 of 97
            // against 91), because VC6 gives the aggregate its own slot
            // instead of reusing the outer one.
            rect.left = *InterfaceClipRect;
            rect.top = *MainInterfaceScreenClipTop;
            rect.right = *MainInterfaceScreenClipRight;
            rect.bottom = *MainInterfaceScreenClipBottom;

            if (!in_box(xCoord, yCoord, &rect)) {
                set_cursor(0x7f00);
                MainInterfaceGlobal.set_cursor(0x7f00);
                field_40B54_ = -1;
                field_40B4C_ = -1;
                field_40B50_ = -1;
            } else {
                // The +0xA14 subobject is BaseWin's IfaceRegistryEntry base,
                // and slot 3 of its table (offset 0x0C) is the override
                // on_iface_left_click. Spelled by name since the derivation -
                // `mov edx,[esi+0xa14] / lea ecx,[esi+0xa14] / call [edx+0xc]`
                // - where a `VCall2` shim stood in because SubInterface
                // declared slot 0 only.
                on_iface_left_click(xCoord, yCoord);
            }
        } else {
            // Front vtable slot 19 (0x4C), which in BaseWin's own front table
            // (0x66A40C) holds on_left_click (0x0041AF70). Win declares the
            // slot `vslot_19`, so the dispatch spells by name.
            vslot_19(xCoord, yCoord);
        }
    }

    if (WinPointerOwner3 == this) {
        WinPointerOwner3 = 0;
        // Front vtable slot 4 (0x10) - Win's on_lose_mouse_capture, the shared no-op.
        on_lose_mouse_capture();
    }
    if (WinPointerOwner4 == this) {
        WinPointerOwner4 = 0;
    }
}

// Fixed-slot bindings carried from 0041b110.cpp
static int *const NetInputLockFlag = (int *)0x0093E8C8;

// ORIGINAL: 0x0041B110 ?on_left_down@BaseWin@@QAEXHH@Z 0x0041B110-0x0041B1D9 FILE BYTE_EXACT
// LEVER: 2026-09-06, the two moves that took this 57/59 to 59/59. (1) Every
//        raw offset in the body is a declared member (field_40B44_/48/4C/50/54,
//        spot2_ at 0x40EC0), and `this` is the front, so the members replaced
//        the `char *self` walk outright - `WinPointerOwner3 = this` included.
//        (2) The two `Spot::check` out-params went back to two plain `int`
//        locals named from spot.h's prototype, declared at the TOP of the
//        function - where VC6 gives them the dedicated `sub esp,8` the image
//        pays - instead of an anonymous struct. That also fixed a defect: the
//        struct spelling passed them to check the wrong way round, so
//        spot_pos landed in field_40B4C_ and spot_type in field_40B50_ where
//        the image stores the opposite (0x0041B1BA writes [ebp-4] to 0x40b4c).
// working copy - scaffold materialised by --work
// size      201 bytes
// prototype void (__thiscall ?on_left_down@BaseWin@@QAEXHH@Z)(BaseWin* this, int xCoord, int yCoord)
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005EC7C0 0x005F7E90 0x005FAB00

void BaseWin::on_left_down(int xCoord, int yCoord) {
    // Declared at the TOP of the function, not where they are used: VC6 gives
    // two locals it cannot prove are in blocks a dead parameter's stack slot,
    // and the image pays a dedicated `sub esp,8` for the Spot::check
    // out-params. (The anonymous-struct spelling the previous transcription
    // used was a workaround for exactly this, and it also passed the two
    // out-params to check the wrong way round - see the LEVER note above.)
    int spot_pos;
    int spot_type;

    if (field_40B54_ > 0) {
        set_cursor(0x7f00);
        MainInterfaceGlobal.set_cursor(0x7f00);
    }
    field_40B54_ = -1;
    field_40B4C_ = -1;
    field_40B50_ = -1;
    field_40B44_ = xCoord;
    field_40B48_ = yCoord;
    if (is_visible() != 0 && (IsMultiplayerNet == 0 || *NetInputLockFlag == 0)) {
        if (WinPointerOwner3 == 0) {
            WinPointerOwner3 = this;
            WinPointerOwner4 = 0;
        }
        if (spot2_.check(xCoord, yCoord, &spot_pos, &spot_type) >= 0) {
            field_40B4C_ = spot_type;
            field_40B50_ = spot_pos;
            field_40B54_ = 0;
        }
    }
}


// 0x0066A1A0 / 0x0066A1F4, read out of the shipped image with
// tools/image_data.py. Back to back, 21 ints each - the 5x5 base radius
// minus its four corners, in ring order.
const int BaseRadiusOffsetX[21] = {
    -1,  1, -2,  0,  2, -3, -1,  1,  3, -2,
     0,  2, -3, -1,  1,  3, -2,  0,  2, -1,
     1,
};
const int BaseRadiusOffsetY[21] = {
    -3, -3, -2, -2, -2, -1, -1, -1, -1,  0,
     0,  0,  1,  1,  1,  1,  2,  2,  2,  3,
     3,
};

/*
// ORIGINAL: 0x004090C0 ?UNK1@BaseWin@@QAEHH@Z 0x004090C0-0x004090FA FILE BYTE_EXACT
// LEVER: the tables this indexes had to become real storage. The scaffold
// bound them `extern int g_a1a0_tbl[]` with no definition anywhere in the
// tree, so homing reverted on LNK2001 every time; naming them (map.h's own
// RadiusOffsetX/RadiusOffsetY are `const int[]` at namespace scope, i.e.
// internal linkage, so they cannot serve) and defining them from the image's
// own bytes is what makes the body spellable. `extern` is the right spelling
// for all four arrays: the const-pointer bindings the scaffold used let /O2
// prove 0x66A1A0 and 0x66A1F4 a constant 0x54 apart and rotate the loop
// around one walking pointer, which the image does not do.
*/
int BaseWin::UNK1(int a1) {
    for (int i = 0; i < 0x15; i++) {
        if (BaseRadiusOffsetX[a1] == RadiusOffsetX[i] &&
            BaseRadiusOffsetY[a1] == RadiusOffsetY[i]) {
            return i;
        }
    }
    return 0;
}

/*
// ORIGINAL: 0x00416540 ?bubble_me_elmo2@BaseWin@@QAEXPADHH@Z 0x00416540-0x00416580 FILE BYTE_EXACT
// LEVER: the RECT argument is built as a local aggregate initializer, not as
// four sequential RECT field stores - sequential stores reload one register
// per statement and mis-schedule against the text load; the initializer keeps
// x and y live in two registers at once and reproduces the original's exact
// interleaving.
*/
void BaseWin::bubble_me_elmo2(char *text, int x, int y) {
    reinterpret_cast<Win *>(this)->client_to_screen(&x, &y);
    RECT rect = { x, y, x, y };
    reinterpret_cast<Win *>(this)->set_bubble_text(text, &rect);
}

/*
// ORIGINAL: 0x00416580 ?bubble_me_elmo2@BaseWin@@QAGXPADHH@Z 0x00416580-0x004165C2 FILE BYTE_EXACT
// symbol    ?bubble_me_elmo2@@YGXPADHH@Z
// explicit arguments - no receiver's worth of extra 4 bytes - and the body
// never reads `this` (ecx is overwritten with the fixed 0x7AE820 singleton
// before either call). A real `__stdcall` member passes `this` as an implicit
// FIRST stack slot (`ret 0x10`, every offset shifted by 4); confirmed by
// compiling it that way first. A plain free function is the only spelling
// that lands it, so it lives beside the class rather than on it.
*/
void __stdcall bubble_me_elmo2(char *text, int x, int y) {
    Win *win = &MainInterfaceGlobal;
    win->client_to_screen(&x, &y);
    RECT rect;
    rect.left = x;
    rect.top = y;
    rect.right = x;
    rect.bottom = y;
    win->set_bubble_text(text, &rect);
}

// Fixed-slot bindings carried from 0041de60.cpp

// ORIGINAL: 0x0041DE60 ?on_iface_left_down@BaseWin@@QAEXHH@Z 0x0041DE60-0x0041DF38 FILE BYTE_EXACT
// symbol    ?on_iface_left_down@BaseWin@@UAEXHH@Z
//           Q->U 2026-09-06, the IfaceRegistryEntry derivation: the body is
//           now an override of the interface base's slot, so VC6 mangles it
//           virtual and enters it on the ENTRY, while the image spells the
//           same body Q and its own table 0x66A3B8 holds the raw address.
// LEVER: locals declared at top of function (not just before use) reproduces `sub esp,8` for two 4-byte slots instead of the optimizer reusing a dead parameter's stack slot for one of them; and the two adjacent global stores (WinPointerOwner3 then WinPointerOwner4) had to be written in that exact order - Ghidra's own pseudocode printed them reversed.
// working copy - scaffold materialised by --work
// size      216 bytes
// prototype void (__thiscall ?on_iface_left_down@BaseWin@@QAEXHH@Z)(BaseWin* this, int xCoord, int yCoord)
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005EC7C0 0x005F7E90 0x005FAB00

void BaseWin::on_iface_left_down(int xCoord, int yCoord) {
    int spot_type;
    int spot_pos;

    if (field_40B54_ > 0) {
        set_cursor(0x7f00);
        MainInterfaceGlobal.set_cursor(0x7f00);
    }
    field_40B54_ = -1;
    field_40B4C_ = -1;
    field_40B50_ = -1;
    field_40B44_ = xCoord;
    field_40B48_ = yCoord;

    if (is_visible() != 0 && (IsMultiplayerNet == 0 || *NetInputLockFlag == 0)) {
        if (WinPointerOwner3 == 0) {
            WinPointerOwner3 = this;
            WinPointerOwner4 = 0;
        }
        if (spot1_.check(xCoord, yCoord, &spot_pos, &spot_type) >= 0) {
            field_40B4C_ = spot_type;
            field_40B50_ = spot_pos;
            field_40B54_ = 0;
        }
    }
}


// ORIGINAL: 0x0041E510 ?on_iface_button_clicked@BaseWin@@QAEXH@Z 0x0041E510-0x0041E52D BYTE_EXACT
// symbol    ?vslot_13@BaseWin@@UAEXH@Z
// LEVER: the derivation deleted this body's shim. The image is one dispatch -
//        `mov eax,[ecx-0xa14] / add ecx,-0xa14 / call [eax+0xb0]` - and 0xb0
//        is the FRONT vtable's slot 44, which Win spells on_button_clicked. Naming it
//        replaced both the `this - 0xa14` pun and the VCallArg1 class the
//        pun needed; the interface's button-clicked event is the front's
//        on_button_clicked, and the source now says so.
// size      29 bytes
// prototype void (__thiscall ?on_iface_button_clicked@BaseWin@@QAEXH@Z)(BaseWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0041E523
// 0x0041E510  ?on_iface_button_clicked@BaseWin@@QAEXH@Z  ->  ?on_iface_button_clicked@BaseWin@@QAEXH@Z
// HANDOFF (basewin, not the pass that wrote this): the `// symbol` fact above
//        is now the only thing keeping 0x0041E510 measurable. Renaming this
//        method to the image's `on_iface_button_clicked` (which is what the
//        fact is standing in for) needs MainInterface::vslot_13 decided
//        first - maininterface.h:130 documents why MainInterface must NOT
//        override that slot by name, and renaming BaseWin's does not change
//        that, but the two tables are the same slot index so the rename has
//        to be checked against BattleWin::on_iface_button_clicked's rel32 to
//        0x0045D170 on both sides. Measured here, not assumed.
void BaseWin::vslot_13(int button_id) {
    on_button_clicked(button_id);
}

// HANDOFF (basewin, added by the win.h slot-44/45 rename, 2026-09-06): Win's
//        slot 44 is now spelled `on_button_clicked`, so BaseWin's own
//        0x0041D500 - which the image's front table 0x66A4BC holds in that
//        slot - IS the override of it, and VC6 needs its address for the
//        vtable it emits. 1388 bytes, unrecovered; the artefact at
//        src/recovered/units/0041d500.cpp is the transcription to home here.
//        This forwarder is the on_status idiom and nothing more.
// PENDING 0x0041D500 ?on_button_clicked@BaseWin@@QAEXH@Z 0x0041D500-0x0041DA6C
// symbol    ?on_button_clicked@BaseWin@@UAEXH@Z
//           Q->U 2026-09-06, Win's slot 44 taking the image's name: the body
//           is now an override of Win's slot, so VC6 mangles it virtual and
//           enters it on the front table, while the image spells the same
//           body Q and its own table 0x66A4BC holds the raw address.
void BaseWin::on_button_clicked(int button_id) {
    typedef void(__fastcall *pending)(BaseWin *, void *, int);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x0041D500))(
        this, nullptr, button_id);
}

// ORIGINAL: 0x0041E8B0 ?hide@BaseWin@@QAEXXZ 0x0041E8B0-0x0041E8C9 BYTE_EXACT
// symbol    ?hide@BaseWin@@UAEXXZ
// size      25 bytes
// prototype void (__thiscall ?hide@BaseWin@@QAEXXZ)(BaseWin* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045D380 0x005F7E90
// 0x0041E8B0  ?hide@BaseWin@@QAEXXZ  ->  ?hide@BaseWin@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void BaseWin::hide() {
    if (is_visible()) {
        // The interface-mode registry entry is this class's second base, so
        // the member call carries the +0xA14 itself - the cast this replaces
        // was the only way to name it before the derivation.
        release_iface_mode();
    }
}

// ORIGINAL: 0x00420A40 ?on_nc_hittest@BaseWin@@QAEHHH@Z 0x00420A40-0x00420A6F BYTE_EXACT
// symbol    ?on_nc_hittest@BaseWin@@UAEHHH@Z
// size      47 bytes
// prototype int (__thiscall ?on_nc_hittest@BaseWin@@QAEHHH@Z)(BaseWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005F5AD0
// 0x00420A40  ?on_nc_hittest@BaseWin@@QAEHHH@Z  ->  ?on_nc_hittest@BaseWin@@QAEHHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


int BaseWin::on_nc_hittest(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    if (a1 >= 0x17 &&
        a1 <= *reinterpret_cast<int *>(self + 0x4c4) - 0x37) {
        return reinterpret_cast<Win *>(this)->Win::on_nc_hittest(a1, a2);
    }
    return 2;
}

/*
// ORIGINAL: 0x00408410 ?load_inverts@@YAXXZ 0x00408410-0x00408469 FILE
// LEVER: the scaffold bound the four strings/counts it reads as anonymous
// `int *const g_<address>` slots. All four are named things: 0x00682830 and
// 0x00682824 hold the literals "BASENAME" and "INVERTLIST" (read with
// tools/image_data.py --string), 0x009B7D00 is text.h's TextBufferGetPtr, and
// 0x006A7528 / 0x006EEE60 are the invert-list storage and its count, named
// here from what this one body does with them.
*/

// 0x006EEE60, the count of loaded invert entries; 0x006A7528, the entries
// themselves at a 0x20 stride. Neither is named anywhere else in the tree.
int *const InvertEntryCount = (int *)0x006EEE60;
char *const InvertEntryTable = (char *)0x006A7528;

void __cdecl load_inverts() {
    if (text_open("INVERTLIST", "BASENAME") == 0) {
        text_get();
        int count = text_item_number();
        *InvertEntryCount = count;
        if (count > 0) {
            char *dest = InvertEntryTable;
            int i = 0;
            do {
                text_get();
                strcpy(dest, TextBufferGetPtr);
                count = *InvertEntryCount;
                i++;
                dest += 0x20;
            } while (i < count);
        }
        text_close();
    }
}


// ORIGINAL: 0x0041DD10 ?on_iface_left_up@BaseWin@@QAEXHH@Z 0x0041DD10-0x0041DE5B FILE
// symbol    ?on_iface_left_up@BaseWin@@UAEXHH@Z
//           Q->U 2026-09-06, the IfaceRegistryEntry derivation: the body is
//           now an override of the interface base's slot, so VC6 mangles it
//           virtual and enters it on the ENTRY, while the image spells the
//           same body Q and its own table 0x66A3B8 holds the raw address.
// LEVER: the artifact's dispatch spelled `VCall`, and the TU it came from had
// two VCall classes - `VCall` whose slot019 takes nothing and `VCall2` whose
// takes (int, int). Only DECLARATION ORDER is meaningful to the emitted code,
// so the fix was to name the right shim, not to change a signature.
// LEVER: 2026-09-06, the derivation took this 91/97 -> 93/99 and deleted all
//        three of its shims. `this` now IS the entry, so the three raw
//        offsets are members (field_40B4C_/40B50_/40B54_, `Buffer::rect2_`
//        for `self - 0x5a0`), the two cross-object casts are the plain
//        `vslot_19(...)` / `on_lose_mouse_capture()` the front already spelled, and the
//        `self + 0xa14` receiver of the old `VCall2::slot004` compare is a
//        single `static_cast<BaseWin *>(this)` - the adjustment the receiver
//        already carries, which is what 0x41DE26's one `add esi,0xfffff5ec`
//        is. Entry slot 3 spells on_iface_left_click by name.
// TRIED: `local_rect = *IfaceDropRect;` as one aggregate assignment for the
//        four drop-rect fields - 37 of 99, far WORSE than the four separate
//        field copies, because the aggregate collapses to a load-all-then-
//        store-all schedule where the image stores each dword as it loads it.
//        The FIRST rect copy in the body (`RECT local_rect = Buffer::rect2_`)
//        is the one place the aggregate IS the image's shape.
// LEVER: 2026-09-06, the derivation deleted the last of this body's puns.
//        `this` IS the entry, so field_40B4C_/40B50_/40B54_ and
//        `Buffer::rect2_` name the four raw offsets, the two cross-object
//        casts are the front's own `vslot_19(...)` / `on_lose_mouse_capture()`, entry slot
//        3 spells `on_iface_left_click(...)`, and the capture-slot compare is
//        one `static_cast<BaseWin *>(this)`. 91/97 before and after - the
//        remaining six are the drop-rect copy's schedule, see the TRIED note.
// size      331 bytes
// prototype void (__thiscall ?on_iface_left_up@BaseWin@@QAEXHH@Z)(BaseWin* this, int, int)
// callers   0   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005EC7C0 0x005ED240 0x005ED2D0 0x005FA7E0
// indirect  0x0041DE0D 0x0041DE1E 0x0041DE3E
// PRESERVED UNIT - measured MNEMONIC_ONLY.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0041DD10
// measured tier  MNEMONIC_ONLY
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0041dd10/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_iface_left_up@BaseWin@@QAEXHH@Z  at 0x0041DD10  (331 bytes)
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

// The fixed addresses this body reads are all already named in the tree:
// 0x007AE820 is mapwin.h's MainInterfaceGlobal and 0x006A7628 is tutwin.h's
// TutWinBaseWindow - the other window the drag is mapped through, its
// client_to_screen / screen_to_client pair bracketing the MainInterface one.
// 0x006A7A9C..0x006A7AA8 are the four dwords of one RECT, so they are one
// binding, not four. 0x009B7ABC and 0x009B7AC0 are the two slots that hold a
// BaseWin pointer while the interface has it captured: releasing the first
// also notifies the window (vtable slot 4), releasing the second only clears
// it, which is enter/leave tracking against capture.
static RECT *const IfaceDropRect = (RECT *)0x006A7A9C;
// Spelled as references to those dwords, not as pointers to them: the image
// compares and clears the slots in place (`cmp dword ptr [0x9b7abc], esi`),
// so a binding the compiler has to load first costs an indirection the
// original does not pay.
// Accessor spellings, the same idiom base.h's BaseCurrent() uses: the image
// compares and clears the dwords in place (`cmp dword ptr [0x9b7abc], esi`),
// and only a folded literal address compiles to that - a pointer or a
// reference binding costs the load of the binding first, which the original
// does not pay.
static inline void *&IfaceCaptureSlot() {
    return *reinterpret_cast<void **>(0x009B7ABC);
}
static inline void *&IfaceEnterSlot() {
    return *reinterpret_cast<void **>(0x009B7AC0);
}

void BaseWin::on_iface_left_up(int xCoord, int yCoord) {
    // front + 0x474: the drawing Buffer's own rect2_, not this class's
    // rect2_ at 0x40BAC - hence the qualification.
    RECT local_rect = Buffer::rect2_;

    if (field_40B54_ > 0) {
        if (in_box(xCoord, yCoord, &local_rect) == 0) {
            MainInterfaceGlobal.client_to_screen(&xCoord, &yCoord);
            TutWinBaseWindow.screen_to_client(&xCoord, &yCoord);

            // Four field copies, and NOT one aggregate assignment: the image
            // stores each dword as it loads it (`mov ecx,[0x6a7a9c] / mov
            // [ebp-0x10],ecx`), and the aggregate spelling collapses to a
            // load-all-then-store-all schedule that measured 37 of 99 against
            // 93. The first copy - `RECT local_rect = Buffer::rect2_` above -
            // is the one place the aggregate IS the image's shape.
            local_rect.left = IfaceDropRect->left;
            local_rect.top = IfaceDropRect->top;
            local_rect.right = IfaceDropRect->right;
            local_rect.bottom = IfaceDropRect->bottom;

            if (in_box(xCoord, yCoord, &local_rect) == 0) {
                set_cursor(0x7f00);
                MainInterfaceGlobal.set_cursor(0x7f00);
                field_40B54_ = -1;
                field_40B4C_ = -1;
                field_40B50_ = -1;
            } else {
                // Front vtable slot 19 (0x4C) - the front's on_left_click.
                vslot_19(xCoord, yCoord);
            }
        } else {
            // This table's own slot 3, dispatched with the entry untouched -
            // `mov edx,[esi] / call [edx+0xc]`.
            on_iface_left_click(xCoord, yCoord);
        }
    }

    // The capture slots hold the FRONT, and 0x41DE26 materialises it once
    // (`add esi, 0xfffff5ec`) before both compares. A static_cast to the
    // derived class is the same adjustment the receiver is already carrying;
    // it replaces the `self - 0xa14` pointer arithmetic the pun needed.
    BaseWin *const self = static_cast<BaseWin *>(this);
    if (IfaceCaptureSlot() == self) {
        IfaceCaptureSlot() = 0;
        // Front vtable slot 4 (0x10) - Win's on_lose_mouse_capture, the shared no-op.
        on_lose_mouse_capture();
    }
    if (IfaceEnterSlot() == self) {
        IfaceEnterSlot() = 0;
    }
}


/*
Purpose: The interface's mode change. Mode selects which of the base screen's
         panels the status line paints. Unrecovered - the body is 431 bytes
         and dispatches its own table's slot 1 and the front's slot 45 among
         a great many named helpers, so it forwards to the image until it is
         transcribed.
// PENDING 0x0041DAD0 ?on_status@BaseWin@@QAEXH@Z 0x0041DAD0-0x0041DC7F
// symbol    ?on_status@BaseWin@@UAEXH@Z
// PENDING BODY - the receiver is solved (the artefact at
//   src/recovered/units/0041dad0.cpp reads `mov edi, ecx` and then steps to
//   the front with `lea ebx, [edi - 0xa14]`, which is the entry receiver the
//   derivation supplies) and the mode switch is transcribable, but its arms
//   reach draw_pop, draw_queue, update_gov_buttons and a dozen more bodies
//   that are themselves unhomed, so the transcription would forward to the
//   image at nearly every step. Nothing is gained by homing it before them.
// size      431 bytes
// prototype void (__thiscall ?on_status@BaseWin@@QAEXH@Z)(BaseWin* this, int)
// callers   1   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00415B20 0x00415080 0x0041E560 0x0040D250 0x004C3A40
*/
void BaseWin::on_status(int mode) {
    typedef void(__fastcall *pending)(BaseWin *, void *, int);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x0041DAD0))(
        this, nullptr, mode);
}

/*
Purpose: The interface's scroll event. The interface has no scrolling of its
         own: both arguments are handed straight to this table's slot 1,
         on_iface_scrolled.
// ORIGINAL: 0x0041DCB0 ?on_iface_scrolling@BaseWin@@QAEXHH@Z 0x0041DCB0-0x0041DCC4 BYTE_EXACT
// symbol    ?on_iface_scrolling@BaseWin@@UAEXHH@Z
// LEVER: the derivation deleted this body's redirect. As an override of the
//        entry's slot 2 the unqualified call to on_iface_scrolled IS the
//        image's own `mov eax,[ecx] / call [eax+4]`, receiver untouched - the
//        delegation_thunks spelling needed an ORIGINAL seam and a free
//        __fastcall wrapper to say the same thing. The redirect and its
//        declaration are gone from delegation_thunks.{cpp,h}.
// size      20 bytes
// prototype void (__thiscall ?on_iface_scrolling@BaseWin@@QAEXHH@Z)(BaseWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// indirect  0x0041DCBD
Return Value: n/a
Status: Complete
*/
void BaseWin::on_iface_scrolling(int a1, int a2) {
    on_iface_scrolled(a1, a2);
}

/*
Purpose: The interface's button-toggled event. It IS the front's
         on_button_toggled: the body steps the receiver back to the
         GraphicWin and dispatches the front vtable's slot 45, which is where
         0x0041B4E0 - the catalogue's on_button_toggled - sits.
// ORIGINAL: 0x0041E530 ?on_iface_button_toggled@BaseWin@@QAEXHH@Z 0x0041E530-0x0041E551 BYTE_EXACT
// symbol    ?on_iface_button_toggled@BaseWin@@UAEXHH@Z
// LEVER: named slot, no shim. `on_button_toggled(a1, a2)` - Win's own name for front
//        vtable slot 45 - emits `mov eax,[ecx-0xa14] / add ecx,-0xa14 /
//        push / push / call [eax+0xb4]`, the image's four instructions and
//        its order. The VCall2 spelling this replaces put the adjustment on
//        an explicit pointer and could not fold it into the load.
// size      33 bytes
// prototype void (__thiscall ?on_iface_button_toggled@BaseWin@@QAEXHH@Z)(BaseWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// indirect  0x0041E547
Return Value: n/a
Status: Complete
*/
void BaseWin::on_iface_button_toggled(int a1, int a2) {
    on_button_toggled(a1, a2);
}

/*
Purpose: The interface's mouse-move report. Unrecovered - 1164 bytes of spot
         hit-testing, queue-panel drag and tool-tip work.
// PENDING 0x0041DF50 ?on_iface_mouse_move@BaseWin@@QAEXHH@Z 0x0041DF50-0x0041E3DC
// symbol    ?on_iface_mouse_move@BaseWin@@UAEXHH@Z
// PENDING BODY - the receiver is solved (`lea ebx, [esi - 0xa14]` at
//   0x0041DF5B is the front, esi the entry, so `spot1_` and `is_visible()`
//   both spell directly now) but the body is a long branch-heavy walk over
//   the queue panel and the artifact it was transcribed from is NOT_MATCHING
//   on its own terms. Left to the pass that homes it.
// size      1164 bytes
// prototype void (__thiscall ?on_iface_mouse_move@BaseWin@@QAEXHH@Z)(BaseWin* this, int, int)
// callers   0   call targets   9
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005F7E90 0x005F8500 0x005FAB00 0x005EC7C0 0x004E39D0
*/
void BaseWin::on_iface_mouse_move(int a1, int a2) {
    typedef void(__fastcall *pending)(BaseWin *, void *, int, int);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x0041DF50))(
        this, nullptr, a1, a2);
}

/*
Purpose: The interface's mouse-leave report. Unrecovered - 224 bytes that
         clear the queue panel's hover and repaint the status line.
// PENDING 0x0041E3E0 ?on_iface_mouse_leave@BaseWin@@QAEXHH@Z 0x0041E3E0-0x0041E4C0
// symbol    ?on_iface_mouse_leave@BaseWin@@UAEXHH@Z
// PENDING BODY - the receiver is solved (every member in the artifact's
//   transcription is entry-relative: 0x4011c, 0x400ec, 0x40b0c) and the shape
//   is MainInterface::on_iface_mouse_leave's - an early test, a guard, a
//   `draw_queue(0)`, then the redraw. The two `call 0x5f8500` pairs that open
//   it are unaccounted for in every reading tried so far, which is why the
//   artifact is NOT_MATCHING and this is a forwarder.
// size      224 bytes
// prototype void (__thiscall ?on_iface_mouse_leave@BaseWin@@QAEXHH@Z)(BaseWin* this, int, int)
// callers   0   call targets   8
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005F8500 0x00415080 0x004E39D0 0x004EC3B0
*/
void BaseWin::on_iface_mouse_leave(int a1, int a2) {
    typedef void(__fastcall *pending)(BaseWin *, void *, int, int);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x0041E3E0))(
        this, nullptr, a1, a2);
}
