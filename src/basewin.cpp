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
// ORIGINAL: 0x004213D0 ??_GBaseWin@@UAEPAXI@Z 0x004213D0-0x004213F1 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical deleting-thunk source src/deleting_thunks.cpp; original full spans retained: 0x004213D0-0x004213F1", "Pinned class constructor ??0BaseWin@@QAE@XZ at 0x00408490 stores original class/view table VA 0x66a40c at 0x004086E5 (mov dword ptr [esi], 0x66a40c); the current genuine owner vftable ??_7BaseWin@@6BWin@@@ selects generated ??_GBaseWin@@UAEPAXI@Z in slot 0 (+0x0) through weak entry ??_EBaseWin@@UAEPAXI@Z (mode 2).", "Pinned ??_GBaseWin@@UAEPAXI@Z at 0x004213D0 calls the source-mapped ??1BaseWin@@QAE@XZ at 0x00420A70, then tests byte ptr [ebp + 8], 1; je 0x4213ea bypasses scalar delete when bit 0 is clear, while bit 0 set calls 0x0064557F; it save original this; call destructor; delete only when flags bit 0 is set; return saved this with ret 4.", "The real class lifetime is owned by src/basewin.cpp; its compiler-emitted deleting symbol and vftable slot carry this original identity. Hand-written adapter bodies with live adjustor callers remain until those callers are independently qualified and transferred. The linked weak/COMDAT copy is not used to choose source ownership.", "the source writer preserves the historical mapping's exactness state (historical exactness obligation retained); current compiler extent is not asserted equal to the original span"], "reason": "Bind the original BaseWin scalar deleting-destructor identity to the actual compiler-emitted class deleting destructor in its genuine owner translation unit. The original constructor's pinned vptr store and the current owner's physical table slot identify this class/view; no callback/thunk body is hand-authored here."}
// symbol    ??_GBaseWin@@UAEPAXI@Z
// kind      game
// ORIGINAL: 0x004083B0 _$E21 0x004083B0-0x004083C6 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x682060 selects initializer0x4083b0..0x4083c6, registering exit0x4083d0..0x4083da. Root independent complete decode confirms exclusive RET/JMP bounds; original/source-owned storage is0x6a7628 bound to?TutWinBaseWindow@@3VBaseWin@@A.", "Original startup instruction evidence: [[\"0x4083b0\", \"mov\", \"ecx, 0x6a7628\"], [\"0x4083b5\", \"call\", \"0x408490\"], [\"0x4083ba\", \"push\", \"0x4083d0\"], [\"0x4083bf\", \"call\", \"0x645398\"], [\"0x4083c4\", \"pop\", \"ecx\"], [\"0x4083c5\", \"ret\", \"\"]]. Original exit instruction evidence: [[\"0x4083d0\", \"mov\", \"ecx, 0x6a7628\"], [\"0x4083d5\", \"jmp\", \"0x420a70\"]]. Preserve constructor arguments, array counts/strides, registration and reverse destruction without authored callback stand-ins.", "Fresh complete build-iveupb5l physical src/basewin.cpp COFF selects _$E21 via .CRT$XCU, registering _$E19. Startup relocation targets: [\"?TutWinBaseWindow@@3VBaseWin@@A\", \"??0BaseWin@@QAE@XZ\", \"_$E19\", \"_atexit\"]; exit targets: [\"?TutWinBaseWindow@@3VBaseWin@@A\", \"??1BaseWin@@UAE@XZ\"]. Actual linked helper occurs once at CRT slot0x63f2b0; selected same-owner order agrees with the pinned order.", "BaseWin/DesignWin definitions now precede their Wave globals as pinned. Existing DesignWin Wave mappings retain original names/spans and current helper overrides after fresh emission proof. Scalar constructor effects and array construction/cleanup belong to genuine source globals, classes and compiler-generated lifetime machinery.", "Current local labels are emission interpretations, not original debug names or recovered historical TU boundaries. Start bindings grant no allocation extent. Selected same-owner linked order is not full CRT/global-order, retained-selection, reference-target, dependency-provider, runtime or full-game acceptance. Guarded callbacks excluded; no claim grant or bug fix."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned ?TutWinBaseWindow@@3VBaseWin@@A; preserve complete original identity and remaining restoration/structure obligations."}
// ORIGINAL: 0x004083D0 _$E19 0x004083D0-0x004083DA FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x682060 selects initializer0x4083b0..0x4083c6, registering exit0x4083d0..0x4083da. Root independent complete decode confirms exclusive RET/JMP bounds; original/source-owned storage is0x6a7628 bound to?TutWinBaseWindow@@3VBaseWin@@A.", "Original startup instruction evidence: [[\"0x4083b0\", \"mov\", \"ecx, 0x6a7628\"], [\"0x4083b5\", \"call\", \"0x408490\"], [\"0x4083ba\", \"push\", \"0x4083d0\"], [\"0x4083bf\", \"call\", \"0x645398\"], [\"0x4083c4\", \"pop\", \"ecx\"], [\"0x4083c5\", \"ret\", \"\"]]. Original exit instruction evidence: [[\"0x4083d0\", \"mov\", \"ecx, 0x6a7628\"], [\"0x4083d5\", \"jmp\", \"0x420a70\"]]. Preserve constructor arguments, array counts/strides, registration and reverse destruction without authored callback stand-ins.", "Fresh complete build-iveupb5l physical src/basewin.cpp COFF selects _$E21 via .CRT$XCU, registering _$E19. Startup relocation targets: [\"?TutWinBaseWindow@@3VBaseWin@@A\", \"??0BaseWin@@QAE@XZ\", \"_$E19\", \"_atexit\"]; exit targets: [\"?TutWinBaseWindow@@3VBaseWin@@A\", \"??1BaseWin@@UAE@XZ\"]. Actual linked helper occurs once at CRT slot0x63f2b0; selected same-owner order agrees with the pinned order.", "BaseWin/DesignWin definitions now precede their Wave globals as pinned. Existing DesignWin Wave mappings retain original names/spans and current helper overrides after fresh emission proof. Scalar constructor effects and array construction/cleanup belong to genuine source globals, classes and compiler-generated lifetime machinery.", "Current local labels are emission interpretations, not original debug names or recovered historical TU boundaries. Start bindings grant no allocation extent. Selected same-owner linked order is not full CRT/global-order, retained-selection, reference-target, dependency-provider, runtime or full-game acceptance. Guarded callbacks excluded; no claim grant or bug fix."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned ?TutWinBaseWindow@@3VBaseWin@@A; preserve complete original identity and remaining restoration/structure obligations."}
// ORIGINAL: 0x004083E0 _$E26 0x004083E0-0x004083F6 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x682064 selects initializer0x4083e0..0x4083f6, registering exit0x408400..0x40840a. Root independent complete decode confirms exclusive RET/JMP bounds; original/source-owned storage is0x6eee68 bound to?g_BASEWIN_WAVE@@3VWave@@A.", "Original startup instruction evidence: [[\"0x4083e0\", \"mov\", \"ecx, 0x6eee68\"], [\"0x4083e5\", \"call\", \"0x4c66e0\"], [\"0x4083ea\", \"push\", \"0x408400\"], [\"0x4083ef\", \"call\", \"0x645398\"], [\"0x4083f4\", \"pop\", \"ecx\"], [\"0x4083f5\", \"ret\", \"\"]]. Original exit instruction evidence: [[\"0x408400\", \"mov\", \"ecx, 0x6eee68\"], [\"0x408405\", \"jmp\", \"0x4c67c0\"]]. Preserve constructor arguments, array counts/strides, registration and reverse destruction without authored callback stand-ins.", "Fresh complete build-iveupb5l physical src/basewin.cpp COFF selects _$E26 via .CRT$XCU, registering _$E24. Startup relocation targets: [\"?g_BASEWIN_WAVE@@3VWave@@A\", \"??0Wave@@QAE@XZ\", \"_$E24\", \"_atexit\"]; exit targets: [\"?g_BASEWIN_WAVE@@3VWave@@A\", \"??1Wave@@UAE@XZ\"]. Actual linked helper occurs once at CRT slot0x63f2b4; selected same-owner order agrees with the pinned order.", "BaseWin/DesignWin definitions now precede their Wave globals as pinned. Existing DesignWin Wave mappings retain original names/spans and current helper overrides after fresh emission proof. Scalar constructor effects and array construction/cleanup belong to genuine source globals, classes and compiler-generated lifetime machinery.", "Current local labels are emission interpretations, not original debug names or recovered historical TU boundaries. Start bindings grant no allocation extent. Selected same-owner linked order is not full CRT/global-order, retained-selection, reference-target, dependency-provider, runtime or full-game acceptance. Guarded callbacks excluded; no claim grant or bug fix."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned ?g_BASEWIN_WAVE@@3VWave@@A; preserve complete original identity and remaining restoration/structure obligations."}
// ORIGINAL: 0x00408400 _$E24 0x00408400-0x0040840A FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x682064 selects initializer0x4083e0..0x4083f6, registering exit0x408400..0x40840a. Root independent complete decode confirms exclusive RET/JMP bounds; original/source-owned storage is0x6eee68 bound to?g_BASEWIN_WAVE@@3VWave@@A.", "Original startup instruction evidence: [[\"0x4083e0\", \"mov\", \"ecx, 0x6eee68\"], [\"0x4083e5\", \"call\", \"0x4c66e0\"], [\"0x4083ea\", \"push\", \"0x408400\"], [\"0x4083ef\", \"call\", \"0x645398\"], [\"0x4083f4\", \"pop\", \"ecx\"], [\"0x4083f5\", \"ret\", \"\"]]. Original exit instruction evidence: [[\"0x408400\", \"mov\", \"ecx, 0x6eee68\"], [\"0x408405\", \"jmp\", \"0x4c67c0\"]]. Preserve constructor arguments, array counts/strides, registration and reverse destruction without authored callback stand-ins.", "Fresh complete build-iveupb5l physical src/basewin.cpp COFF selects _$E26 via .CRT$XCU, registering _$E24. Startup relocation targets: [\"?g_BASEWIN_WAVE@@3VWave@@A\", \"??0Wave@@QAE@XZ\", \"_$E24\", \"_atexit\"]; exit targets: [\"?g_BASEWIN_WAVE@@3VWave@@A\", \"??1Wave@@UAE@XZ\"]. Actual linked helper occurs once at CRT slot0x63f2b4; selected same-owner order agrees with the pinned order.", "BaseWin/DesignWin definitions now precede their Wave globals as pinned. Existing DesignWin Wave mappings retain original names/spans and current helper overrides after fresh emission proof. Scalar constructor effects and array construction/cleanup belong to genuine source globals, classes and compiler-generated lifetime machinery.", "Current local labels are emission interpretations, not original debug names or recovered historical TU boundaries. Start bindings grant no allocation extent. Selected same-owner linked order is not full CRT/global-order, retained-selection, reference-target, dependency-provider, runtime or full-game acceptance. Guarded callbacks excluded; no claim grant or bug fix."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned ?g_BASEWIN_WAVE@@3VWave@@A; preserve complete original identity and remaining restoration/structure obligations."}
#include "stdafx.h"
#include "interface_fonts.h"
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
#include "lock.h"
#include "map.h"
#include "netdaemon.h"
#include "popup.h"
#include "pulldown.h"
#include "mapwin.h"
#include "technology.h"
#include "text.h"
#include "tutwin.h"
#include "veh.h"
#include "xpops.h"
#include "alpha.h"
#include "faction.h"
#include "fx.h"
#include "game.h"
#include "interface_palette.h"
#include "random.h"
#include "log.h"
#include "popmenu.h"
#include "infowin.h"
#include "interlude.h"
#include "strings.h"
#include "messagewin.h"
#include "statuswin.h"
#include "worldwin.h"
#include <mmsystem.h>
#include <cstdint>
#include <cstring>

// Existing shared text.pcx palette publication, owned by palette.cpp.
extern int PopupWriteCheckState;
extern Wave g_MENU_DOWN_WAVE;
extern Wave g_BASEWIN_WAVE;
void __cdecl action_staple(int base_id);
void __cdecl ok_callback();
void __cdecl base_kill(int base_id);
int __cdecl popm(char *label, int x, int y, int (__cdecl *poll)());
void __cdecl slide_show(GraphicWin *window, int mode);
void __cdecl slide_hide(GraphicWin *window, int mode);

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

// Recompute can change the worked-square allocation. Network notification is
// conditional on that live state changing, not simply on invoking the screen.
void BaseWin::compute_selected_base() {
    const int old = Bases[focusedBaseID_].resource_sqr_radius;
    set_base(static_cast<int>(focusedBaseID_));
    base_compute(0);
    if (IsMultiplayerNet && !ConsoleControlTurnActive
        && old != Bases[focusedBaseID_].resource_sqr_radius) {
        if (NetDaemonNet.lock_base(BaseIDCurrentSelected, 0, -1, -1) == 0)
            NetDaemonNet.unlock_base(BaseIDCurrentSelected);
    }
}

namespace {
int specialist_at(const Base &base, int position) {
    return (base.spec[position >> 3] >> ((position & 7) * 4)) & 15;
}
void set_specialist_at(Base &base, int position, int type) {
    const int shift = (position & 7) * 4;
    base.spec[position >> 3] = (base.spec[position >> 3] & ~(15u << shift))
        | (static_cast<unsigned>(type) << shift);
}
}

// ORIGINAL: 0x0040A1D0 ?farm_click@BaseWin@@QAEXHHH@Z 0x0040A1D0-0x0040A87F
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/unrecovered/0040a1d0.cpp", "source:src/basewin.h", "image:0x40a1d0"], "reason": "Recover the complete original operation through genuine owning C++ classes, members and natural lifetimes; preserve retained seed evidence and historical restoration obligations separately from fresh matching."}
void BaseWin::farm_click(int x, int y, int) {
    compute_selected_base();
    const int faction = BaseCurrent->faction_id_current;
    if ((faction != ConsoleGlobal.faction_id_ && !(GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW))
        || field_40AFC_) return;
    int previous = field_40B4C_ == 0 ? farm_get(field_40B44_, field_40B48_) : -1;
    const int position = farm_get(x, y);
    if (previous == position) previous = -1;
    else if (previous == 0) return;
    if (position < 0 || NetDaemonNet.lock_base(BaseIDCurrentSelected, 0, -1, -1)) return;
    log_say("Farm Click", BaseCurrent->name_string, position, 0, 0);
    BaseCurrent->governor &= ~0x40u;
    const unsigned bit = 1u << position;
    bool changed = false;
    if (field_40B4C_ == 1) {
        if (!(BaseCurrent->resource_sqr_radius & bit)) {
            BaseCurrent->resource_sqr_radius |= bit;
            for (int i = field_40B50_; i < BaseCurrent->specialist_total; ++i) {
                const int next = i + 1;
                const int type = next < 16 ? specialist_at(*BaseCurrent, next) : best_specialist();
                if (next < 17) set_specialist_at(*BaseCurrent, i, type);
            }
            if (BaseCurrent->specialist_total) --BaseCurrent->specialist_total;
            changed = true;
        }
    } else if (previous > 0) {
        const unsigned old_bit = 1u << previous;
        if ((BaseCurrent->resource_sqr_radius & old_bit) && !(BaseCurrent->resource_sqr_radius & bit)) {
            BaseCurrent->resource_sqr_radius &= ~old_bit;
            BaseCurrent->resource_sqr_radius |= bit;
            changed = true;
        }
    } else if (position == 0) {
        BaseCurrent->resource_sqr_radius = 0;
        BaseCurrent->specialist_total = 0;
        BaseCurrent->specialist_unk_1 = 0;
        changed = true;
    } else if (BaseCurrent->resource_sqr_radius & bit) {
        BaseCurrent->resource_sqr_radius &= ~bit;
        const int type = best_specialist();
        if (BaseCurrent->specialist_total < 16)
            set_specialist_at(*BaseCurrent, BaseCurrent->specialist_total, type);
        ++BaseCurrent->specialist_total;
        changed = true;
    } else if (!FarmStatus[position]) {
        bool available = BaseCurrent->specialist_total != 0;
        if (available) --BaseCurrent->specialist_total;
        else {
            int fight, growth, tech, wealth, power;
            if ((static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & (1u << faction))
                && (BaseCurrent->governor & 0x80000000u)) {
                growth = (BaseCurrent->governor >> 24) & 1;
                tech = (BaseCurrent->governor >> 25) & 1;
                wealth = (BaseCurrent->governor >> 26) & 1;
                power = (BaseCurrent->governor >> 27) & 1;
                fight = power ? 1 : growth ? 0 : (!tech && !wealth ? PlayersData[faction].ai_fight : -1);
            } else {
                fight = PlayersData[faction].ai_fight;
                growth = PlayersData[faction].ai_growth;
                tech = PlayersData[faction].ai_tech;
                wealth = PlayersData[faction].ai_wealth;
                power = PlayersData[faction].ai_power;
            }
            int worst_score = 999, worst_position = 0;
            for (int i = 1; i <= 20; ++i) {
                int tile_x = BaseCurrent->x + RadiusOffsetX[i];
                if (!(MapIsFlat & 1)) {
                    if (tile_x < 0) tile_x += MapLongitudeBounds;
                    else if (tile_x >= MapLongitudeBounds) tile_x -= MapLongitudeBounds;
                }
                const int tile_y = BaseCurrent->y + RadiusOffsetY[i];
                if (tile_x < 0 || tile_x >= MapLongitudeBounds || tile_y < 0 || tile_y >= MapLatitudeBounds
                    || !(BaseCurrent->resource_sqr_radius & (1u << i))) continue;
                const int nutrient_divisor = range(BaseCurrent->nutrient_intake_2
                    - BaseCurrent->population_size * static_cast<int>(Rules.nutrient_req_citizen), 1, 99);
                int score = resource_yield(0, faction, BaseIDCurrentSelected, tile_x, tile_y)
                    * (growth + 3) * 4 / nutrient_divisor;
                const int mineral_divisor = range(BaseCurrent->mineral_intake_2
                    - static_cast<int>(BaseCurrentForcesMaintCost), 1, 99);
                score += resource_yield(1, faction, BaseIDCurrentSelected, tile_x, tile_y)
                    * (fight + 4 + power) * BaseCurrent->population_size / mineral_divisor;
                score += resource_yield(2, faction, BaseIDCurrentSelected, tile_x, tile_y)
                    * BaseCurrent->population_size * (wealth + tech + 2)
                    / range(BaseCurrent->energy_intake_2, 1, 99);
                if (score < worst_score) { worst_score = score; worst_position = i; }
            }
            if (worst_position) {
                BaseCurrent->resource_sqr_radius &= ~(1u << worst_position);
                available = true;
            }
        }
        if (available) { BaseCurrent->resource_sqr_radius |= bit; changed = true; }
    }
    if (is_visible()) {
        set_base(focusedBaseID_);
        base_compute(1);
        redraw();
        MainInterfaceGlobal.redraw();
    }
    if (changed) log_say("BASE: changed farming locations", BaseIDCurrentSelected, 0, 0);
    NetDaemonNet.unlock_base(BaseIDCurrentSelected);
}

// ORIGINAL: 0x0040A880 ?pop_click@BaseWin@@QAEXHHHH@Z 0x0040A880-0x0040B11A;0x006518EA-0x00651A78
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/unrecovered/0040a880.cpp", "source:src/basewin.h", "image:0x40a880"], "reason": "Recover the complete original operation through genuine owning C++ classes, members and natural lifetimes; preserve retained seed evidence and historical restoration obligations separately from fresh matching."}
void BaseWin::pop_click(int position, int, int, int) {
    Popup popup;
    compute_selected_base();
    if (BaseCurrent->faction_id_current != ConsoleGlobal.faction_id_
        && !(GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)) return;
    int current = position < 0 ? 999 : position < 16
        ? specialist_at(*BaseCurrent, position) : best_specialist();
    if (current < 0) current = 1;
    popup.start(PopupStartCaption, "CITIZEN", -1, 0, 0x40, 0);
    strcpy(StringTemp, label_get(0x2C8 / 4));
    popup.dialogs().item(StringTemp, 999);
    bool too_small = false;
    int selected = 999;
    for (int i = 0; i < 7; ++i) {
        const RulesCitizen &citizen = Citizen[i];
        const int faction = BaseCurrent->faction_id_current;
        if (!has_tech(citizen.preq_tech, faction)
            || (has_tech(citizen.obsol_tech, faction) && i != current)) continue;
        if (BaseCurrent->population_size < static_cast<int>(Rules.min_base_size_specialists)
            && citizen.psych_bonus < 2) { too_small = true; continue; }
        strcpy(StringTemp, StringTable.get(citizen.singular_name));
        strcat(StringTemp, "|");
        char text[224] = "";
        const int bonuses[3] = { citizen.ops_bonus, citizen.psych_bonus, citizen.research_bonus };
        for (int k = 0; k < 3; ++k) if (bonuses[k]) {
            if (*text) strcat(text, ", ");
            strcat(text, "+");
            char number[80];
            _itoa(bonuses[k], number, 10);
            strcat(text, number);
            strcat(text, " ");
            strcat(text, StringTable.get(Energy[k].abbrev));
        }
        strcat(StringTemp, text);
        popup.dialogs().item(StringTemp, i);
        if (i == current) selected = i;
    }
    if (too_small) {
        parse_num(0, Rules.min_base_size_specialists);
        popup.start(PopupStartCaption, "CITIZEN2", -1, 0, 0x80, 0);
    }
    popup.set_value(selected);
    const int result = popup.exec(0, 0);
    if (result < 0 || result == current
        || NetDaemonNet.lock_base(BaseIDCurrentSelected, 0, -1, -1)) return;
    BaseCurrent->governor &= ~0x40u;
    if (result == 999) {
        if (BaseCurrent->specialist_total) --BaseCurrent->specialist_total;
    } else {
        int packed_position = position;
        if (current == 999) {
            int freed = -1;
            if (field_40B4C_) {
                for (int i = 20; i > 0; --i)
                    if (BaseCurrent->resource_sqr_radius & (1u << i)) { freed = i; break; }
            } else freed = farm_get(field_40B44_, field_40B48_);
            if (freed > 0) BaseCurrent->resource_sqr_radius &= ~(1u << freed);
            packed_position = BaseCurrent->specialist_total++;
        }
        if (packed_position < 16) set_specialist_at(*BaseCurrent, packed_position, result);
    }
    log_say("BASE: changed citizen professions", BaseIDCurrentSelected, 0, 0);
    if (is_visible()) {
        set_base(focusedBaseID_);
        base_compute(1);
        redraw();
        MainInterfaceGlobal.redraw();
    }
    NetDaemonNet.unlock_base(BaseIDCurrentSelected);
}

// ORIGINAL: 0x00409200 ?queue_click@BaseWin@@QAEXHHHH@Z 0x00409200-0x0040A1B9;0x0065158B-0x006518EA
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/unrecovered/00409200.cpp", "source:src/basewin.h", "image:0x409200"], "reason": "Recover the complete original operation through genuine owning C++ classes, members and natural lifetimes; preserve retained seed evidence and historical restoration obligations separately from fresh matching."}
void BaseWin::queue_click(int row, int x, int y, int right) {
    PopMenu menu;
    const int faction = BaseCurrent->faction_id_current;
    if (row <= 1) return;
    const int position = row - 2;
    if (position == field_40B2C_) return;
    if (!right) {
        field_40B2C_ = min(static_cast<int>(BaseCurrent->queue_size) + 1, position);
        field_40B30_ = 0;
        draw_queue(1);
        if (prodPicker_.is_visible() && prodPicker_.queue_change_blocked()) return;
        if (NetDaemonNet.lock_base(BaseIDCurrentSelected, 0, -1, -1) == 0) {
            production(position, 1);
            NetDaemonNet.unlock_base(BaseIDCurrentSelected);
        }
        return;
    }
    if (prodPicker_.is_visible()) return;
    const bool empty = position > static_cast<int>(BaseCurrent->queue_size);
    if (!empty) {
        const int entry = BaseCurrent->queue_production_id[position];
        if (entry >= 0) parse_says(0, VehPrototypes[entry].veh_name, -1, -1);
        else parse_say(0, Facility[-entry].name, -1, -1);
    }
    MainInterfaceGlobal.client_to_screen(&x, &y);
    strcpy(StringTemp, empty ? "QUEUE2" : "QUEUE1");
    int action = popm(StringTemp, x + 2, y, 0) + 1;
    if (empty && action > 1) ++action;
    --action;
    if (static_cast<unsigned>(action) > 4) return;
    if (action == 0) {
        field_40B2C_ = min(static_cast<int>(BaseCurrent->queue_size) + 1, position);
        field_40B30_ = 0;
        draw_queue(1);
        if (prodPicker_.is_visible()) return;
        if (NetDaemonNet.lock_base(BaseIDCurrentSelected, 0, -1, -1) == 0) {
            production(position, 1);
            NetDaemonNet.unlock_base(BaseIDCurrentSelected);
        }
    } else if (action == 1) {
        if (!prodPicker_.is_visible()
            && NetDaemonNet.lock_base(BaseIDCurrentSelected, 0, -1, -1)) return;
        if (static_cast<int>(BaseCurrent->queue_size) < position) {
            field_40B2C_ = max(static_cast<int>(field_40B2C_) - 1, 1);
        } else {
            for (int i = position; i < 9; ++i)
                BaseCurrent->queue_production_id[i] = BaseCurrent->queue_production_id[i + 1];
            --BaseCurrent->queue_size;
            if (static_cast<int>(BaseCurrent->queue_size) < static_cast<int>(field_40B2C_))
                field_40B2C_ = max(static_cast<int>(BaseCurrent->queue_size), 1);
            draw_queue(1);
        }
        if (!prodPicker_.is_visible()) NetDaemonNet.unlock_base(BaseIDCurrentSelected);
    } else if (action == 2) {
        if (!prodPicker_.is_visible()
            && NetDaemonNet.lock_base(BaseIDCurrentSelected, 0, -1, -1)) return;
        if (X_pop("DELETEQUEUE", 0)) { BaseCurrent->queue_size = 0; draw_queue(1); }
        if (!prodPicker_.is_visible()) NetDaemonNet.unlock_base(BaseIDCurrentSelected);
    } else {
        if (action == 4 && NetDaemonNet.lock_base(BaseIDCurrentSelected, 0, -1, -1)) return;
        menu.init();
        for (int i = 0; i < 8; ++i) menu.dialogs().item(PlayersData[faction].saved_queue_name[i], i);
        const int picked = menu.exec(x, y, pop_expire);
        compute_selected_base();
        // Original cancellation exits directly, even after the load case's
        // lock. Do not manufacture an unlock absent from that error path.
        if (picked < 0 || (GameDataStateStorage.fields_.game_state_ & 0x800)) return;
        if (action == 3) {
            char *name = PlayersData[faction].saved_queue_name[picked];
            if (X_pop_ask("TEMPNAME", 23, name, 0, 0) != 0) return;
            if (!X_pop("OVERWRITETEMPLATE", 0)) return;
            strcpy(name, ParseStrBuffer[0].str);
            PlayersData[faction].saved_queue_size[picked] = BaseCurrent->queue_size;
            for (int i = 0; i < 10; ++i) {
                int entry = BaseCurrent->queue_production_id[i];
                if (entry >= 0)
                    entry += static_cast<unsigned char>(VehPrototypes[entry].unk_1) << 16;
                PlayersData[faction].saved_queue_items[picked][i] = entry;
            }
            synch_template(faction);
            NetDaemonNet.unlock_base(BaseIDCurrentSelected);
            draw_queue(1);
        } else {
            for (int old_index = 0; old_index <= static_cast<int>(BaseCurrent->queue_size); ++old_index) {
                const int entry = BaseCurrent->queue_production_id[old_index];
                if (entry >= 0) --PlayersData[faction].proto_id_queue[entry];
            }
            BaseCurrent->queue_size = PlayersData[faction].saved_queue_size[picked];
            for (int i = 0; i <= static_cast<int>(BaseCurrent->queue_size); ++i) {
                const int stored = PlayersData[faction].saved_queue_items[picked][i];
                int entry = stored;
                if (stored >= 0) {
                    entry = stored & 0xFFFF;
                    const unsigned category = static_cast<unsigned>(stored) >> 16;
                    if (static_cast<unsigned char>(VehPrototypes[entry].unk_1) != category
                        || (VehPrototypes[entry].obsolete_factions & (1u << faction))) {
                        for (int k = 0; k < 64; ++k) {
                            const int candidate = faction * 64 + k;
                            const VehPrototype &prototype = VehPrototypes[candidate];
                            if ((prototype.flags & 1) && !(prototype.obsolete_factions & (1u << faction))
                                && static_cast<unsigned char>(prototype.unk_1) == category) entry = candidate;
                        }
                    }
                }
                BaseCurrent->queue_production_id[i] = entry;
                if (entry >= 0) ++PlayersData[faction].proto_id_queue[entry];
            }
            NetDaemonNet.unlock_base(BaseIDCurrentSelected);
            draw_queue(1);
        }
    }
}

// Candidate only. Evidence: original 40C070..40C842, pinned terranx_original.exe;
// retained git:a538e0a3:src/unrecovered/0040c070.cpp and /tmp/basewin-40c070.json.
// All popup strings independently read from original .rdata 682904..682AD4.
// No compiler/measurement performed. Existing deeper callee debt remains.
// Original bulk-sale counting deliberately tests BaseCurrent->state, not
// each candidate base's state. The subsequent bulk mutation deliberately
// does not retest possession or the one-sale-per-turn bit. Preserve both.
// ORIGINAL: 0x0040C070 ?facilities_click@BaseWin@@QAEHHHHH@Z 0x0040C070-0x0040C842
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/unrecovered/0040c070.cpp", "source:src/basewin.h", "image:0x40c070"], "reason": "Recover the complete original operation through genuine owning C++ classes, members and natural lifetimes; preserve retained seed evidence and historical restoration obligations separately from fresh matching."}
int BaseWin::facilities_click(int facility, int, int, int) {
    const int faction = BaseCurrent->faction_id_current;
    if (faction != ConsoleGlobal.faction_id_ && !(GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW))
        return 0;
    parse_says(0, BaseCurrent->name_string, -1, -1);
    parse_say(1, Facility[facility].name, -1, -1);
    const int selection = popp(ScriptPopupFile, "FACMENU", 0,
        (Players[faction].rule_flags & 0x80) ? "alfacblt_sm.pcx" : "facblt_sm.pcx", 0);
    if (!selection) return 0;
    if (selection == 1) {
        help_topic(facility >= 70 ? 11 : 10, facility);
        return 0;
    }
    if (facility >= 70) {
        popp(ScriptPopupFile, "NOTPROJECT", 0, "secproj_sm.pcx", 0);
        return 0;
    }
    int offset, mask;
    if (facility < 65) bitmask(facility, &offset, &mask);
    if (facility >= 65 || !(Bases[BaseIDCurrentSelected].facilities_built[offset] & (uint8_t)mask)) {
        popp(ScriptPopupFile, "DONTHAVEFAC", 0,
            (Players[faction].rule_flags & 0x80) ? "alfacblt_sm.pcx" : "facblt_sm.pcx", 0);
        return 0;
    }
    if (ConsoleControlTurnActive && IsMultiplayerNet) {
        X_pop("DURINGUPKEEP", 0);
        return 0;
    }
    if ((BaseCurrent->state & BSTATE_FACILITY_SCRAPPED) && !(GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)) {
        parse_says(0, BaseCurrent->name_string, -1, -1);
        popp(ScriptPopupFile, "ONERECYCLE", 0,
            (Players[faction].rule_flags & 0x80) ? "alfacblt_sm.pcx" : "facblt_sm.pcx", 0);
        return 0;
    }
    if (facility == 1) {
        popp(ScriptPopupFile, "HQRECYCLE", 0,
            (Players[faction].rule_flags & 0x80) ? "alfacblt_sm.pcx" : "facblt_sm.pcx", 0);
        return 0;
    }
    if (facility == 26 && (map_tiles()[BaseCurrent->y * MapLongitude + (BaseCurrent->x >> 1)].contour & 0xE0) < 0x60) {
        popp(ScriptPopupFile, "PRESSURERECYCLE", 0,
            (Players[faction].rule_flags & 0x80) ? "alfacblt_sm.pcx" : "facblt_sm.pcx", 0);
        return 0;
    }
    for (int bonus = 0; bonus < Players[faction].faction_bonus_count; ++bonus) {
        const int kind = Players[faction].faction_bonus_id[bonus];
        const int granted = Players[faction].faction_bonus_val1[bonus];
        if ((kind == 2 || (kind == 12 && has_tech(Facility[granted].preq_tech, faction)))
            && facility == granted) {
            parse_say(1, Facility[facility].name, -1, -1);
            popp(ScriptPopupFile, "FREERECYCLE", 0,
                (Players[faction].rule_flags & 0x80) ? "alfacblt_sm.pcx" : "facblt_sm.pcx", 0);
            return 0;
        }
    }
    if (NetDaemonNet.lock_base(BaseIDCurrentSelected, selection == 3 ? 2 : 0, -1, -1)) return 0;
    int proceeds;
    if (selection == 3) {
        int payable = 0;
        int possessed = 0;
        for (int base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
            if (Bases[base].faction_id_current != faction) continue;
            if (facility == 26 && (map_tiles()[Bases[base].y * MapLongitude + (Bases[base].x >> 1)].contour & 0xE0) < 0x60) continue;
            bitmask(facility, &offset, &mask);
            if (Bases[base].facilities_built[offset] & (uint8_t)mask) {
                ++possessed;
                if (!(BaseCurrent->state & BSTATE_FACILITY_SCRAPPED) || (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)) ++payable;
            }
        }
        proceeds = static_cast<int>(static_cast<uint32_t>(Facility[facility].cost) * payable * 5u);
        parse_num(0, proceeds);
        parse_num(1, possessed);
        parse_say(0, Facility[facility].name, -1, -1);
    } else {
        proceeds = static_cast<int>(static_cast<uint32_t>(Facility[facility].cost) * 5u);
        parse_say(0, Facility[facility].name, -1, -1);
        parse_num(0, proceeds);
    }
    StringTemp[0] = 0;
    strcat(StringTemp, "RECYCLEME");
    if (selection == 3) strcat(StringTemp, "ALL");
    else if (facility == 8 && (BaseCurrent->state & BSTATE_ARTIFACT_LINKED)) {
        proceeds = 0;
        strcat(StringTemp, "0");
    }
    if (!popp(ScriptPopupFile, StringTemp, 0,
        (Players[faction].rule_flags & 0x80) ? "alfacblt_sm.pcx" : "facblt_sm.pcx", 0)) {
        BaseComputedID = -1;
        base_compute(0);
        NetDaemonNet.unlock_base(BaseIDCurrentSelected);
        return 0;
    }
    if (selection == 3) {
        for (int base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
            if (Bases[base].faction_id_current != faction) continue;
            if (facility == 26 && (map_tiles()[Bases[base].y * MapLongitude + (Bases[base].x >> 1)].contour & 0xE0) < 0x60) continue;
            Bases[base].state |= BSTATE_FACILITY_SCRAPPED;
            set_fac(facility, base, 0);
            if (base != BaseIDCurrentSelected) synch_base(base);
        }
        PlayersData[faction].energy_reserves = static_cast<int>(static_cast<uint32_t>(PlayersData[faction].energy_reserves) + static_cast<uint32_t>(proceeds));
        log_say("BASE: global scrapped facility", BaseIDCurrentSelected, 0, 0);
    } else {
        BaseCurrent->state |= BSTATE_FACILITY_SCRAPPED;
        set_fac(facility, BaseIDCurrentSelected, 0);
        PlayersData[faction].energy_reserves = static_cast<int>(static_cast<uint32_t>(PlayersData[faction].energy_reserves) + static_cast<uint32_t>(proceeds));
        log_say("BASE: scrapped facility", BaseIDCurrentSelected, 0, 0);
    }
    BaseComputedID = -1;
    base_compute(0);
    NetDaemonNet.unlock_base(BaseIDCurrentSelected);
    ConsoleInfoWin.set_redraw_phase(0);
    ConsoleInfoWin.on_redraw();
    redraw();
    MainInterfaceGlobal.GraphicWin::redraw();
    return 0;
}

// Original 40D250..40E35B, automatic cleanup of Sprite and two Fonts.
// Integration dependencies: genuine RECTs flash_rect_a_/flash_rect_b_ at
// BaseWin+40DEC/+40DFC; Buffer::current_clip() exposing rect1_ (not rect2_);
// TextColourTable is the genuine 195-int text.pcx palette owner at8C6B5C;
// title indices82/83 and normal indices14/15, not separate globals.

// ORIGINAL: 0x0040D250 ?draw_flash@BaseWin@@QAEXH@Z 0x0040D250-0x0040E35B;0x00651AA2-0x00651ACD
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/unrecovered/0040d250.cpp", "source:src/basewin.h", "image:0x40d250"], "reason": "Recover the complete original operation through genuine owning C++ classes, members and natural lifetimes; preserve retained seed evidence and historical restoration obligations separately from fresh matching."}
void BaseWin::draw_flash(int redraw_mode) {
    Sprite scratch;
    Font normal;
    Font title;
    const int vehicle_id = field_40B00_;
    if (vehicle_id < 0) return;
    Buffer &canvas = static_cast<Buffer &>(MainInterfaceGlobal);
    RECT panel = queue_help_rect_;
    RECT saved_clip = canvas.current_clip();
    canvas.restore_clip();
    BoxSpriteParams style = InterfaceBoxStyles[3];
    style.edge3_ = 0;
    style.corner3_ = 0;
    canvas.box_sprite(&panel, &style);
    style.corner0_ = 0;
    style.edge0_ = 0;
    style.corner1_ = 0;
    RECT area;
    area.left = queue_outer_rect_.left - 5;
    area.top = queue_outer_rect_.top + 5;
    area.right = queue_outer_rect_.right - 5;
    area.bottom = queue_outer_rect_.bottom + 5;
    canvas.box_sprite(&area, &style);
    IntersectRect(&area, &flash_rect_a_, &flash_rect_b_);
    area.left -= 3; area.top -= 3; area.right += 3; area.bottom += 3;
    scratch.create_blank(6, 6, WinFillColour);
    style = InterfaceBoxStyles[3];
    style.corner0_ = &scratch;
    style.edge2_ = 0;
    style.corner2_ = 0;
    canvas.box_sprite(&area, &style);
    canvas.set_clip(&panel);
    area.left = panel.left + 3; area.top = panel.top + 3;
    area.right = panel.right - 3; area.bottom = panel.bottom - 3;
    canvas.box_sprite(&area, &InterfaceBoxStyles[0]);
    area.left += 3; area.top += 3; area.right -= 3; area.bottom -= 3;
    normal.init(DefaultFontFace, WinScreenWidth == 800 ? 10 : 12, 0);
    title.init(DefaultFontFace, 12, 1);
    MapWinCaptionDivisor = 100;
    MapWinCaptionScale = (area.right - area.left) * 3 / 4;
    const int width = MapWinCaptionScale;
    const int height = MapWinCaptionScale * 75 / 100;
    veh_draw(&canvas, vehicle_id, (area.left + area.right) / 2 - width / 2,
             area.top + 40 - height / 2, 1, 8, 0);
    MapWinCaptionScale = -1;
    MapWinCaptionDivisor = -1;
    area.top += 65;
    const Veh &vehicle = Vehs[vehicle_id];
    const int prototype_id = vehicle.proto_id;
    const VehPrototype &prototype = VehPrototypes[prototype_id];
    strcpy(StringTemp, prototype.veh_name);
    CharUpperA(StringTemp);
    canvas.set_text_color(TextColourTable[82], TextColourTable[83], 1, 1);
    canvas.set_font(&title, 0, 0, 0);
    area.top = canvas.wrap_cent(StringTemp, &area) + 2;
    area.left += 2; area.right -= 2; area.bottom -= 2;
    area.bottom -= normal.line_advance();
    canvas.set_font(&normal, 0, 0, 0);
    canvas.set_text_color(TextColourTable[14], TextColourTable[15], 1, 1);
    const int x = area.left;
    const int text_width = area.right - area.left;
    const int line = normal.line_advance();
    int y = area.top;
    char number[16];
    strcpy(StringTemp, "(");
    say_stats_3(StringTemp, prototype_id);
    for (int ability = 0; ability < 29; ++ability) {
        if (has_abil(prototype_id, 1 << ability)
            && strlen(StringTable.get(Ability[ability].abbreviation)) != 0) {
            strcat(StringTemp, ", ");
            strcat(StringTemp, StringTable.get(Ability[ability].abbreviation));
        }
    }
    strcat(StringTemp, ")");
    y = canvas.wrap(StringTemp, x, y, text_width);
    if ((vehicle.faction_id != 0 && prototype.plan < 8)
        || (prototype.plan == 11 && morale_veh(vehicle_id, 1, 0) > 1)) {
        StringTemp[0] = 0;
        say_morale(vehicle_id, 0);
        if (vehicle.faction_id != ConsoleGlobal.faction_id_ && !(GameDataStateStorage.fields_.game_state_ & 0x80))
            y = canvas.wrap(StringTemp, x, y, text_width);
        else
            y = canvas.wrap_cent(StringTemp, x, y, text_width);
    }
    if (vehicle.faction_id == ConsoleGlobal.faction_id_ || (GameDataStateStorage.fields_.game_state_ & 0x80)) {
        strcpy(StringTemp, vehicle.home_base_id < 0 ? label_get(0x308 / 4)
               : Bases[vehicle.home_base_id].name_string);
        y = canvas.wrap_cent(StringTemp, x, y, text_width);
        if (vehicle.faction_id == ConsoleGlobal.faction_id_) {
            const int remaining = range(speed(vehicle_id, 0) - vehicle.moves_expended, 0, 999);
            if (remaining == 0) {
                strcpy(StringTemp, label_get(0x9E4 / 4));
                canvas.set_text_color(0xF9, -1, 1, 1);
            } else {
                strcpy(StringTemp, label_get(0x1C / 4));
                strcat(StringTemp, ": ");
                // The original calls speed again in the positive branch.
                const int moves = range(speed(vehicle_id, 0) - vehicle.moves_expended, 0, 999);
                const int divisor = static_cast<int>(Rules.move_rate_roads);
                strcat(StringTemp, _itoa(moves / divisor, number, 10));
                if (moves % divisor) {
                    strcat(StringTemp, " ");
                    strcat(StringTemp, _itoa(moves % divisor, number, 10));
                    strcat(StringTemp, "/");
                    strcat(StringTemp, _itoa(static_cast<int>(Rules.move_rate_roads), number, 10));
                }
            }
            y = canvas.wrap_cent(StringTemp, x, y, text_width);
        }
        const int missile = Chassis[prototype.chassis_id].missile;
        if (missile) {
            strcpy(StringTemp, label_get(0x30C / 4));
            strcat(StringTemp, ": ");
            const int next_x = canvas.write_l(StringTemp, x, y, strlen(StringTemp));
            StringTemp[0] = 0;
            const int remaining = range(speed(vehicle_id, 0) - vehicle.moves_expended, 0, 999);
            const int moves = speed(vehicle_id, 0) * (missile + (remaining == 0)
                                                     - vehicle.terraforming_turns)
                              - vehicle.moves_expended;
            strcat(StringTemp, _itoa(moves / static_cast<int>(Rules.move_rate_roads), number, 10));
            strcat(StringTemp, "/");
            strcat(StringTemp, _itoa(speed(vehicle_id, 0) * missile / static_cast<int>(Rules.move_rate_roads),
                                    number, 10));
            canvas.set_text_color(0xFA, -1, 1, 1);
            if (vehicle.terraforming_turns != 0 && vehicle.terraforming_turns >= missile / 2)
                canvas.set_text_color(0xFB, -1, 1, 1);
            if (missile == 1
                && range(speed(vehicle_id, 0) - vehicle.moves_expended, 0, 999) <= speed(vehicle_id, 0) / 2)
                canvas.set_text_color(0xFB, -1, 1, 1);
            if (action_home(vehicle_id, -1)) canvas.set_text_color(0xF9, -1, 1, 1);
            canvas.write_l(StringTemp, next_x, y, strlen(StringTemp));
            y += line;
        }
        canvas.set_text_color(TextColourTable[14], TextColourTable[15], 1, 1);
        if (veh_cargo(vehicle_id)) {
            strcpy(StringTemp, label_get(0x360 / 4));
            strcat(StringTemp, ": ");
            strcat(StringTemp, _itoa(veh_cargo(vehicle_id) - stack_veh(vehicle_id, 0), number, 10));
            strcat(StringTemp, "/");
            strcat(StringTemp, _itoa(veh_cargo(vehicle_id), number, 10));
            canvas.write_l(StringTemp, x, y, strlen(StringTemp));
            y += line;
        }
    }
    if (vehicle.dmg_incurred) {
        strcpy(StringTemp, label_get(0x628 / 4));
        strcat(StringTemp, ": ");
        const int next_x = canvas.write_l(StringTemp, x, y, strlen(StringTemp));
        canvas.set_text_color(0xFA, -1, 1, 1);
        const int maximum = prototype.plan == 12 ? 1 : range(prototype.reactor_id, 1, 100) * 10;
        if (vehicle.dmg_incurred >= maximum / 3) canvas.set_text_color(0xFB, -1, 1, 1);
        if (vehicle.dmg_incurred >= maximum * 2 / 3) canvas.set_text_color(0xF9, -1, 1, 1);
        StringTemp[0] = 0;
        const int remaining = range(maximum - vehicle.dmg_incurred, 0, 9999);
        strcat(StringTemp, _itoa((maximum - remaining) * 100 / maximum, number, 10));
        strcat(StringTemp, "%");
        canvas.write_l(StringTemp, next_x, y, strlen(StringTemp));
        canvas.set_text_color(MessageWinTextColor, MessageWinTextBackground, 1, 1);
        y += line;
    }
    canvas.set_text_color(TextColourTable[14], TextColourTable[15], 1, 1);
    if (vehicle.faction_id == ConsoleGlobal.faction_id_ || (GameDataStateStorage.fields_.game_state_ & 0x80)
        || (PlayersData[ConsoleGlobal.faction_id_].diplo_treaties[vehicle.faction_id] & 1)) {
        StringTemp[0] = 0;
        say_orders(vehicle_id);
        y = canvas.wrap_cent(StringTemp, x, y, text_width);
        if (vehicle.order >= 4 && vehicle.order < 24) {
            const int turns = action_terraform(vehicle_id, vehicle.order - 4, 0);
            strcpy(StringTemp, "[");
            strcat(StringTemp, label_get(0x71C / 4));
            strcat(StringTemp, ": ");
            strcat(StringTemp, _itoa(turns + 1, number, 10));
            strcat(StringTemp, "]");
            canvas.set_text_color(0xFB, -1, 1, 1);
            canvas.write_l(StringTemp, x, y, strlen(StringTemp));
        }
    }
    if (redraw_mode) MainInterfaceGlobal.update(&queue_help_rect_, 0);
    else MainInterfaceGlobal.soft_update(&queue_help_rect_);
    // Original 40E30D deliberately selects this+444, not the world canvas.
    Buffer::set_clip(&saved_clip);
}

// Original 41C7A0..41D4BA, cleanup 6527EF..652A94.
// Requires PullDown::item_id(int), an unchecked typed slot accessor: the
// original tests only selected < 64, not selected >= 0. Do not substitute
// get_selected_id(), which changes that signed error path.
// Both modal objects are constructed at entry; their complete teardown is RAII.
// ORIGINAL: 0x0041C7A0 ?base_editor@BaseWin@@QAEXXZ 0x0041C7A0-0x0041D4BA;0x006527EF-0x00652A94
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/unrecovered/0041c7a0.cpp", "source:src/basewin.h", "image:0x41c7a0"], "reason": "Recover the complete original operation through genuine owning C++ classes, members and natural lifetimes; preserve retained seed evidence and historical restoration obligations separately from fresh matching."}
void BaseWin::base_editor() {
    PullDown menu;
    Popup popup;
    int x, y;
    get_mouse_pos(&x, &y);
    client_to_screen(&x, &y);
    menu.init(0);
    menu.add_item(1, label_get(0xD40 / 4));
    menu.add_item(2, label_get(0xD44 / 4));
    menu.add_separator(99);
    menu.add_item(5, label_get(0xD58 / 4));
    menu.add_item(10, label_get(0xE60 / 4));
    menu.add_item(6, label_get(0xD5C / 4));
    menu.add_item(7, label_get(0xD60 / 4));
    menu.add_separator(99);
    menu.add_item(8, label_get((BaseCurrent->event & 0x8000 ? 0xE04 : 0xE00) / 4));
    menu.add_separator(99);
    menu.add_item(3, label_get(0xD48 / 4));
    menu.add_item(4, label_get(0xD4C / 4));
    menu.add_separator(99);
    menu.add_item(11, label_get(0xF58 / 4));
    menu.add_item(12, label_get(0xF5C / 4));
    menu.add_separator(99);
    menu.add_item(13, label_get(0x103C / 4));
    menu.add_item(14, label_get(0x1040 / 4));
    menu.add_item(9, label_get(0xE18 / 4));
    menu.show(x, y, pop_expire);
    const int slot = menu.get_selected();
    const int action = slot < 64 ? menu.item_id(slot) : -1;
    if (action < 0) return;
    switch (action) {
    case 1:
    case 2:
        if (action == 1) {
            if (BaseCurrent->population_size < 99) ++BaseCurrent->population_size;
        } else if (BaseCurrent->population_size > 1) {
            --BaseCurrent->population_size;
        }
        if (is_visible()) {
            set_base(focusedBaseID_);
            base_compute(1);
            redraw();
            MainInterfaceGlobal.redraw();
        }
        break;
    case 3: base_editor_fac(); break;
    case 4: base_editor_proj(); break;
    case 5: base_editor_copy(0); break;
    case 10: base_editor_copy(1); break;
    case 6: {
        // 41CC00..41CC0F clears exactly nine bytes, not sizeof facilities_built.
        for (int i = 0; i < 9; ++i) BaseCurrent->facilities_built[i] = 0;
        redraw();
        break;
    }
    case 7: {
        for (int pass = 0; pass < 10; ++pass) {
            for (int facility = 1; facility <= 64; ++facility) {
                if (facility_avail(facility, BaseCurrent->faction_id_current,
                                   BaseIDCurrentSelected, 0))
                    set_fac(facility, BaseIDCurrentSelected, 1);
            }
        }
        redraw();
        break;
    }
    case 8:
        BaseCurrent->event ^= 0x8000;
        redraw();
        break;
    case 9:
        parse_says(0, BaseCurrent->name_string, -1, -1);
        if (X_pop("REALLYKILLBASE", 0)) {
            base_kill(BaseIDCurrentSelected);
            const int was_visible = TutWinBaseWindow.is_visible();
            zoom_wait_pending_ = 0;
            time_.stop();
            if (TutWinBaseWindow.is_visible()) TutWinBaseWindow.release_iface_mode();
            do_all_draws();
            if (was_visible && ConsoleGlobal.cursor_active_
                && ConsoleGlobal.selected_vehicle_ >= 0) {
                const Veh &vehicle = Vehs[ConsoleGlobal.selected_vehicle_];
                if (!ConsoleGlobal.tile_in_view(vehicle.x, vehicle.y, 2))
                    ConsoleGlobal.center();
            }
            draw_map(1);
        }
        break;
    case 11:
        if (X_pop_ask_number("EDITBASEMINERALS", BaseCurrent->minerals_accumulated, pop_wait))
            return;
        BaseCurrent->minerals_accumulated_2 = ParseNumTable[0] < 0 ? 0 : ParseNumTable[0];
        BaseCurrent->minerals_accumulated = BaseCurrent->minerals_accumulated_2;
        BaseCurrent->production_id_last = BaseCurrent->queue_production_id[0];
        redraw();
        break;
    case 12:
        if (X_pop_ask_number("EDITBASENUTRIENT", BaseCurrent->nutrients_accumulated, pop_wait))
            return;
        BaseCurrent->nutrients_accumulated = ParseNumTable[0] < 0 ? 0 : ParseNumTable[0];
        redraw();
        break;
    case 13: {
        popup.start(PopupStartCaption, "CHANGEOWNER", -1, 0, 0x40, 0);
        for (int item = 1; item < 8; ++item) {
            GenderDefault = Players[item].noun_gender;
            PluralityDefault = Players[item].is_noun_plural;
            popup.dialogs().item(Players[item].noun_faction, item);
        }
        popup.set_value(BaseCurrent->faction_id_current);
        const int faction = popup.exec(0, 0);
        if (faction > 0) {
            owner_set(BaseCurrent->x, BaseCurrent->y, faction);
            BaseCurrent->faction_id_former = static_cast<uint8_t>(faction);
            BaseCurrent->faction_id_current = static_cast<uint8_t>(faction);
            BaseCurrent->queue_size = 0;
            BaseCurrent->production_id_last = 0;
            BaseCurrent->queue_production_id[0] = 0;
            reset_territory();
            base_compute(1);
            redraw();
            draw_map(1);
            ConsoleGlobal.update_data(0);
        }
        break;
    }
    case 14: {
        popup.start(PopupStartCaption, "CHANGEFORMER", -1, 0, 0x40, 0);
        for (int item = 1; item < 8; ++item) {
            GenderDefault = Players[item].noun_gender;
            PluralityDefault = Players[item].is_noun_plural;
            StringTemp[0] = 0;
            parse_says(0, Players[item].noun_faction, -1, -1);
            popup.dialogs().item(StringTemp, item);
        }
        popup.set_value(BaseCurrent->faction_id_former);
        const int faction = popup.exec(0, 0);
        if (faction > 0) {
            if (BaseCurrent->faction_id_former != faction) {
                // This write precedes the cancellable prompt and survives cancellation.
                if (BaseCurrent->assimilation_turns_left == 0)
                    BaseCurrent->assimilation_turns_left = 20;
                if (X_pop_ask_number("CHANGECONQUEST", BaseCurrent->assimilation_turns_left, 0))
                    return;
                BaseCurrent->assimilation_turns_left = static_cast<uint8_t>(ParseNumTable[0]);
            } else {
                BaseCurrent->assimilation_turns_left = 0;
            }
            BaseCurrent->faction_id_former = static_cast<uint8_t>(faction);
            base_compute(1);
            redraw();
            draw_map(1);
            ConsoleGlobal.update_data(0);
        }
        break;
    }
    }
}

// ORIGINAL: 0x00408EB0 ?draw_support@BaseWin@@QAEXXZ 0x00408EB0-0x004090BF
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/recovered/units/00408eb0.cpp", "source:src/basewin.h", "image:original instruction and dispatch evidence for the BaseWin owner"], "reason": "Recover this original base-window operation through the real BaseWin owner, existing typed members and source-owned shared dependencies; catalogue naming and original intervals remain distinct from current compiler symbols."}
void BaseWin::draw_support() {
    RECT saved = Buffer::rect1_;
    Buffer *canvas = this;
    canvas->set_clip(&rect1_);
    fill(&rect1_, 0);
    const RECT area = rect1_;
    mapWin_.init(2, 0);
    mapWin_.field_1DD6C_ = static_cast<int>(mapWin_.field_1DD6C_) >> 1;
    const int width = mapWin_.pixel_width();
    const int height = mapWin_.pixel_height();
    mapWin_.field_1DD70_ = 0x200402;
    int zoom = 2;
    int width_step = 900;
    int height_step = 450;
    do {
        const int horizontal = MapWinCaptionScale > 0
            ? MapWinCaptionScale * 50 / MapWinCaptionDivisor : width_step / 16;
        if ((MapLongitudeBounds / 2) * horizontal <= width) {
            const int vertical = MapWinCaptionScale > 0
                ? MapWinCaptionScale * 50 / MapWinCaptionDivisor : height_step / 16;
            if ((MapLatitudeBounds / 2) * vertical <= height) break;
        }
        --zoom;
        width_step -= 50;
        height_step -= 25;
    } while (width_step > 50);
    mapWin_.field_1DD98_ = zoom;
    mapWin_.field_1DD9C_ = BaseCurrent->x;
    mapWin_.field_1DDA0_ = BaseCurrent->y;
    mapWin_.redraw();
    mapWin_.copy(canvas, 0, 0,
        area.left + (area.right - area.left) / 2 - width / 2,
        area.top + (area.bottom - area.top) / 2 - height / 2, width, height);
    soft_update(&rect1_);
    canvas->set_clip(&saved);
}

// ORIGINAL: 0x0041A4C0 ?exit@BaseWin@@QAEXXZ 0x0041A4C0-0x0041A609
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/recovered/units/0041a4c0.cpp", "source:src/basewin.h", "image:original instruction and dispatch evidence for the BaseWin owner"], "reason": "Recover this original base-window operation through the real BaseWin owner, existing typed members and source-owned shared dependencies; catalogue naming and original intervals remain distinct from current compiler symbols."}
void BaseWin::exit() {
    const int visible = TutWinBaseWindow.is_visible();
    zoom_wait_pending_ = 0;
    time_.stop();
    if (TutWinBaseWindow.is_visible()) TutWinBaseWindow.release_iface_mode();
    do_all_draws();
    if (!visible || !ConsoleGlobal.cursor_active_ || ConsoleGlobal.selected_vehicle_ < 0) return;
    // ECX holds y and is checked against MapLatitudeBounds; EDX holds x.
    // The seed's misleading local names do not change the typed coordinates.
    const int horizontal = Vehs[ConsoleGlobal.selected_vehicle_].y;
    int vertical = Vehs[ConsoleGlobal.selected_vehicle_].x;
    const int left = ConsoleGlobal.field_1DDA8_;
    const int top = ConsoleGlobal.field_1DDA4_;
    int height = MapLongitudeBounds;
    if (horizontal >= 0 && horizontal < MapLatitudeBounds
        && vertical >= 0 && vertical < height) {
        const unsigned char terrain = map_tiles()[MapLongitude * horizontal + (vertical >> 1)].climate;
        if ((terrain & 0xE0) > 0x60
            && horizontal - (terrain >> 5) + 3 < left + 2) {
            ConsoleGlobal.center();
            return;
        }
        height = MapLongitudeBounds;
    }
    if (horizontal < left + 2
        || horizontal >= ConsoleGlobal.field_1DDD0_ + ConsoleGlobal.field_1DDD8_ + left - 2) {
        ConsoleGlobal.center();
        return;
    }
    if (!(MapIsFlat & 1)) {
        if (vertical < top + 2) vertical += height;
        if (vertical >= ConsoleGlobal.field_1DDCC_ + ConsoleGlobal.field_1DDD4_ + top - 2)
            vertical -= height;
    }
    if (vertical < top + 2
        || vertical >= ConsoleGlobal.field_1DDCC_ + ConsoleGlobal.field_1DDD4_ + top - 2)
        ConsoleGlobal.center();
}

// ORIGINAL: 0x0041A620 ?base_killed@BaseWin@@QAEXH@Z 0x0041A620-0x0041A79D
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/recovered/units/0041a620.cpp", "source:src/basewin.h", "image:original instruction and dispatch evidence for the BaseWin owner"], "reason": "Recover this original base-window operation through the real BaseWin owner, existing typed members and source-owned shared dependencies; catalogue naming and original intervals remain distinct from current compiler symbols."}
void BaseWin::base_killed(int base_id) {
    if (!is_visible()) return;
    if (base_id < static_cast<int>(focusedBaseID_)) {
        --focusedBaseID_;
        return;
    }
    if (base_id != static_cast<int>(focusedBaseID_)) return;
    // The original selected-base destruction tail repeats exit's view checks.
    const int visible = TutWinBaseWindow.is_visible();
    zoom_wait_pending_ = 0;
    time_.stop();
    if (TutWinBaseWindow.is_visible()) TutWinBaseWindow.release_iface_mode();
    do_all_draws();
    if (!visible || !ConsoleGlobal.cursor_active_ || ConsoleGlobal.selected_vehicle_ < 0) return;
    const int x = Vehs[ConsoleGlobal.selected_vehicle_].y;
    int y = Vehs[ConsoleGlobal.selected_vehicle_].x;
    const int left = ConsoleGlobal.field_1DDA8_;
    const int top = ConsoleGlobal.field_1DDA4_;
    const int height = MapLongitudeBounds;
    if (x >= 0 && x < MapLatitudeBounds && y >= 0 && y < height) {
        const unsigned char terrain = map_tiles()[MapLongitude * x + (y >> 1)].climate;
        if ((terrain & 0xE0) > 0x60 && x - (terrain >> 5) + 3 < left + 2) {
            ConsoleGlobal.center();
            return;
        }
    }
    if (x < left + 2 || x >= ConsoleGlobal.field_1DDD0_ + ConsoleGlobal.field_1DDD8_ + left - 2) {
        ConsoleGlobal.center();
        return;
    }
    if (!(MapIsFlat & 1)) {
        if (y < top + 2) y += height;
        if (y >= ConsoleGlobal.field_1DDCC_ + ConsoleGlobal.field_1DDD4_ + top - 2) y -= height;
    }
    if (y < top + 2 || y >= ConsoleGlobal.field_1DDCC_ + ConsoleGlobal.field_1DDD4_ + top - 2)
        ConsoleGlobal.center();
}

// ORIGINAL: 0x00419A90 ?governor@BaseWin@@QAEHH@Z 0x00419A90-0x0041A010
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/unrecovered/00419a90.cpp", "source:src/basewin.h", "image:original instruction and dispatch evidence for the BaseWin owner"], "reason": "Recover this original base-window operation through the real BaseWin owner, existing typed members and source-owned shared dependencies; catalogue naming and original intervals remain distinct from current compiler symbols."}
int BaseWin::governor(int settings_only) {
    compute_selected_base();
    if (Bases[focusedBaseID_].faction_id_current != ConsoleGlobal.faction_id_
        && !(GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)) return 1;
    Base *base = BaseCurrent;
    const int faction = base->faction_id_current;
    const uint32_t source_bits[19] = {0x80000000u,0x200000,0x80,0x40,1,
        0x400000,0x200,0x400,0x800,0x1000,0x2000,0x80000,0x800000,
        0x100000,0x8000,0x20000,0x10000,0x40000,0x20};
    PopupWriteCheckState = 0;
    for (int i = 0; i < 19; ++i)
        if (base->governor & source_bits[i]) PopupWriteCheckState |= 1 << i;
    parse_says(0, base->name_string, -1, -1);
    if (NetDaemonNet.lock_base(BaseIDCurrentSelected, 0, -1, -1)) return 1;
    if (x_pop_short("GOVOPTIONS", 0x41, nullptr) < 0) {
        NetDaemonNet.unlock_base(BaseIDCurrentSelected);
        return 1;
    }
    base = BaseCurrent;
    uint32_t flags = base->governor & 0x0F000000;
    for (int j = 0; j < 19; ++j)
        if (PopupWriteCheckState & (1 << j)) flags |= source_bits[j];
    base->governor = flags;
    PlayersData[faction].base_governor_adv = flags;
    if (!settings_only) {
        if (base->governor & 0x80000000u) {
            if (base->governor & 1) {
                base->state &= 0x7FFFFFFFu;
                base->queue_size = 0;
                base_reset(BaseIDCurrentSelected, 1);
                base = BaseCurrent;
            }
            if (base->governor & 0x40) {
                base->resource_sqr_radius = 0;
                base->specialist_total = 0;
                base->specialist_unk_1 = 0;
                base_compute(1);
                base_doctors();
                base = BaseCurrent;
            }
        }
        if (!(base->governor & 0x80000000u) || !(base->governor & 1))
            draw_radius(base->x, base->y, 2, 2);
    }
    NetDaemonNet.unlock_base(BaseIDCurrentSelected);
    redraw();
    MainInterfaceGlobal.redraw();
    return 0;
}

// ORIGINAL: 0x0041A2B0 ?rename@BaseWin@@QAEXXZ 0x0041A2B0-0x0041A4B6
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/recovered/units/0041a2b0.cpp", "source:src/basewin.h", "image:original instruction and dispatch evidence for the BaseWin owner"], "reason": "Recover this original base-window operation through the real BaseWin owner, existing typed members and source-owned shared dependencies; catalogue naming and original intervals remain distinct from current compiler symbols."}
void BaseWin::rename() {
    compute_selected_base();
    if (BaseCurrent->faction_id_current != ConsoleGlobal.faction_id_
        && !(GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)) return;
    if (NetDaemonNet.lock_base(BaseIDCurrentSelected, 0, -1, -1)) return;
    parse_says(0, BaseCurrent->name_string, -1, -1);
    if (X_pop_ask("RENAME", 0x18, BaseCurrent->name_string, nullptr, 0)) {
        NetDaemonNet.unlock_base(BaseIDCurrentSelected);
        return;
    }
    strcpy(BaseCurrent->name_string, ParseStrBuffer[0].str);
    if (BaseCurrent->faction_id_current == ConsoleGlobal.faction_id_) {
        const int former = BaseCurrent->faction_id_former;
        if (former != ConsoleGlobal.faction_id_ && !((1 << former) & GameDataStateStorage.fields_.lock_masks_.enable_mask_)
            && !IsMultiplayerNet) {
            cause_friction(former, ConsoleGlobal.faction_id_, 3);
            const uint32_t treaty = PlayersData[BaseCurrent->faction_id_former]
                .diplo_treaties[ConsoleGlobal.faction_id_];
            if ((treaty & 7) && (treaty & 0x20) && random(0, 2) == 0)
                set_treaty(BaseCurrent->faction_id_former, ConsoleGlobal.faction_id_, 0x400, 1);
        }
    }
    NetDaemonNet.unlock_base(BaseIDCurrentSelected);
    draw_info(1);
    draw_radius(BaseCurrent->x, BaseCurrent->y, 3, 2);
}

// ORIGINAL: 0x0041B4F0 ?staple_the_bastards@BaseWin@@QAEXXZ 0x0041B4F0-0x0041B78D
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/recovered/units/0041b4f0.cpp", "source:src/basewin.h", "image:original instruction and dispatch evidence for the BaseWin owner"], "reason": "Recover this original base-window operation through the real BaseWin owner, existing typed members and source-owned shared dependencies; catalogue naming and original intervals remain distinct from current compiler symbols."}
void BaseWin::staple_the_bastards() {
    compute_selected_base();
    if (Bases[focusedBaseID_].faction_id_current != ConsoleGlobal.faction_id_
        && !(GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)) return;
    const int faction = BaseCurrent->faction_id_current;
    const bool alien = (Players[faction].rule_flags & 0x80) != 0;
    parse_says(0, BaseCurrent->name_string, -1, -1);
    const int police = PlayersData[faction].soc_effect_active.values_[SocialEffect::POLICE]
        + has_fac(0x23, BaseIDCurrentSelected, 0) * 2;
    const char *caption = ScriptPopupFile;
    if (police < 0) {
        popp(caption, "NERVESTAPLEHELLNO", 0, alien ? "Alopdir.pcx" : "talent_sm.pcx", nullptr);
        return;
    }
    if (BaseCurrent->nerve_staple_turns_left) {
        popp(caption, "NERVESTAPLED", 0, alien ? "ALdrone_sm.pcx" : "drone_sm.pcx", nullptr);
        return;
    }
    if (NetDaemonNet.lock_base(BaseIDCurrentSelected, 0, -1, -1)) return;
    if (!popp(caption, "NERVESTAPLE", 0, alien ? "ALdrone_sm.pcx" : "drone_sm.pcx", nullptr)) {
        NetDaemonNet.unlock_base(BaseIDCurrentSelected);
        return;
    }
    if (IsMultiplayerNet) {
        message_data(0x2414, 0, BaseIDCurrentSelected, 0, 0, 0);
        NetDaemonNet.await_exec(1);
        NetDaemonNet.unlock_base(BaseIDCurrentSelected);
        return;
    }
    action_staple(BaseIDCurrentSelected);
    NetDaemonNet.unlock_base(BaseIDCurrentSelected);
}

void BaseWin::update_drag_cursor(int spot_type, int spot_pos, int x, int y, int large) {
    const int previous = field_40B54_;
    if (field_40B4C_ != 0 && field_40B4C_ != 1) return;
    if (previous < 0) return;
    if (field_40B4C_ == spot_type) {
        if (field_40B4C_ == 0) {
            if (farm_get(field_40B44_, field_40B48_) >= 0) {
                const int current = farm_get(x, y);
                field_40B54_ = (farm_get(field_40B44_, field_40B48_) != current) + 1;
            }
        } else {
            field_40B54_ = (spot_pos != field_40B50_) + 1;
        }
    } else field_40B54_ = 2;
    if (field_40B54_ <= 0 || previous != 0) return;
    int cursor = 1;
    if (field_40B4C_ == 1 && field_40B50_ >= 0) {
        const int specialist = field_40B50_;
        cursor = (specialist < 16
            ? (BaseCurrent->spec[specialist >> 3] >> ((specialist & 7) * 4)) & 15
            : best_specialist()) + 4;
        if (large && cursor < 4) cursor = 1;
    }
    const bool alien = ExpansionEnabled && (Players[BaseCurrent->faction_id_current].rule_flags & 0x80);
    Sprite *sprite;
    if (cursor < 4) {
        Sprite *citizens = large
            ? (alien ? g_AL_CITIZEN_LG_CURSOR_SPRITES : g_CITIZEN_LG_CURSOR_SPRITES + 4)
            : (alien ? g_AL_CITIZEN_SM_CURSOR_SPRITES : g_CITIZEN_SM_CURSOR_SPRITES + 4);
        sprite = citizens + cursor;
    } else {
        Sprite *specialists = large
            ? (alien ? g_AL_SPECIALIST_LG_CURSOR_SPRITES : g_SPECIALIST_LG_CURSOR_SPRITES)
            : (alien ? g_AL_SPECIALIST_SM_CURSOR_SPRITES : g_SPECIALIST_SM_CURSOR_SPRITES);
        sprite = specialists + cursor - 4;
    }
    set_cursor(sprite, large ? 19 : 7, large ? 24 : 9);
    MainInterfaceGlobal.set_cursor(sprite, large ? 19 : 7, large ? 24 : 9);
}

// ORIGINAL: 0x0041B1F0 ?on_mouse_move@BaseWin@@QAEXHH@Z 0x0041B1F0-0x0041B491
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/recovered/units/0041b1f0.cpp", "source:src/basewin.h", "image:original instruction and dispatch evidence for the BaseWin owner"], "reason": "Recover this original base-window operation through the real BaseWin owner, existing typed members and source-owned shared dependencies; catalogue naming and original intervals remain distinct from current compiler symbols."}
// symbol    ?on_mouse_move@BaseWin@@UAEXHH@Z
void BaseWin::on_mouse_move(int x, int y) {
    if (!is_visible() || (IsMultiplayerNet && NetDaemonNet.lock_state_[0].leading_words_[3])) return;
    clear_bubble_text();
    clear_bubble_text();
    if (field_40B00_ >= 0) {
        field_40B00_ = -1;
        field_40B04_ = 0;
        compute_selected_base();
        draw_production(1);
    }
    if ((field_40B4C_ != 0 && field_40B4C_ != 1) || field_40B54_ < 0) return;
    int position, type;
    if (spot2_.check(x, y, &position, &type) < 0) return;
    update_drag_cursor(type, position, x, y, 0);
}

void BaseWin::show_panel_help(int x, int y, int label_offset, Win *window) {
    char *text = label_get(label_offset / 4);
    window->client_to_screen(&x, &y);
    RECT point = {x, y, x, y};
    window->set_bubble_text(text, &point);
}

// ORIGINAL: 0x004160F0 ?iface_click@BaseWin@@QAEXHHHH@Z 0x004160F0-0x00416512
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/recovered/units/004160f0.cpp", "source:src/basewin.h", "image:original instruction and dispatch evidence for the BaseWin owner"], "reason": "Recover this original base-window operation through the real BaseWin owner, existing typed members and source-owned shared dependencies; catalogue naming and original intervals remain distinct from current compiler symbols."}
void BaseWin::iface_click(int x, int y, int right, int is_double) {
    TutorialWindow.reset();
    if (field_40B54_ > 0) {
        set_cursor(0x7F00);
        MainInterfaceGlobal.set_cursor(0x7F00);
    }
    field_40B54_ = -1;
    if (WinPointerOwner3 == this) {
        WinPointerOwner3 = nullptr;
        on_lose_mouse_capture();
    }
    if (WinPointerOwner4 == this) WinPointerOwner4 = nullptr;
    if (!is_visible() || (IsMultiplayerNet && NetDaemonNet.lock_state_[0].leading_words_[3])) return;
    int position, type;
    if (spot1_.check(x, y, &position, &type) < 0) {
        if (right && in_box(x, y, &population_rect_)) {
            show_panel_help(x, y, 0xCA4, &MainInterfaceGlobal);
            return;
        }
        if (in_box(x, y, &interface_help_rect_)) {
            show_panel_help(x, y, 0xC9C, &MainInterfaceGlobal);
            return;
        }
        if (in_box(x, y, &garrison_rect_)) {
            show_panel_help(x, y, 0xCA0, &MainInterfaceGlobal);
            return;
        }
        if (in_box(x, y, &queue_help_rect_))
            show_panel_help(x, y, 0xCD0, &MainInterfaceGlobal);
        if (!in_box(x, y, &population_rect_)) return;
        type = 1;
    }
    if (prodPicker_.is_visible() && type != 8) return;
    compute_selected_base();
    const int faction = BaseCurrent->faction_id_current;
    if (faction != ConsoleGlobal.faction_id_ && !(GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)
        && !(PlayersData[ConsoleGlobal.faction_id_].diplo_treaties[faction] & 1)
        && type != 6) return;
    if (is_double) return;
    if (field_40B4C_ == 0 && type == 1) position = -1;
    // Original416514's eight entries are indexed by type-1; the retained
    // seed's ordinal switch incorrectly assigned queue and garrison types.
    switch (type) {
    case 1: pop_click(position, x, y, right); return;
    case 5: production(0, 0); break;
    case 6:
    case 7: garrison_click(position, x, y, right); return;
    case 8: queue_click(position, x, y, right); return;
    }
}

// ORIGINAL: 0x004165D0 ?click@BaseWin@@QAEXHHHH@Z 0x004165D0-0x00416D83
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/unrecovered/004165d0.cpp", "source:src/basewin.h", "image:original instruction and dispatch evidence for the BaseWin owner"], "reason": "Recover this original base-window operation through the real BaseWin owner, existing typed members and source-owned shared dependencies; catalogue naming and original intervals remain distinct from current compiler symbols."}
void BaseWin::click(int x, int y, int right, int is_double) {
    TutorialWindow.reset();
    if (field_40B54_ > 0) {
        set_cursor(0x7F00);
        MainInterfaceGlobal.set_cursor(0x7F00);
    }
    field_40B54_ = -1;
    if (WinPointerOwner3 == this) {
        WinPointerOwner3 = nullptr;
        on_lose_mouse_capture();
    }
    if (WinPointerOwner4 == this) WinPointerOwner4 = nullptr;
    if (!is_visible() || (IsMultiplayerNet && NetDaemonNet.lock_state_[0].leading_words_[3])) return;
    if (right) {
        if (in_box(x, y, &rect1_)) {
            int label;
            if (field_40AFC_ == 0) label = 0xC98;
            else if (field_40AFC_ == 2) label = 0xC20;
            else if (field_40AFC_ == 1) label = 0xC24;
            else return;
            show_panel_help(x, y, label, this);
            return;
        }
        if (in_box(x, y, &commerce_outer_rect_)) {
            const bool alien = (Players[Bases[focusedBaseID_].faction_id_current].rule_flags & 0x80) != 0;
            show_panel_help(x, y, alien ? 0x1190 : 0xC74, this);
            return;
        }
        if (in_box(x, y, &facilities_help_rect_)) {
            show_panel_help(x, y, 0xC78, this);
            return;
        }
        if (in_box(x, y, &nutrient_outer_rect_)) {
            show_panel_help(x, y, 0xC7C, this);
            return;
        }
        const int order[6] = {0, 2, 1, 3, 4, 5};
        for (int panel = 0; panel < 6; ++panel) {
            if (in_box(x, y, &expenses_help_rects_[order[panel]])
                && (WinScreenWidth == 0x400 || (panel < 3
                    ? !flatButtons_[17].selected_state() : flatButtons_[17].selected_state()))) {
                show_panel_help(x, y, 0xC80 + panel * 4, this);
                return;
            }
        }
    }
    int position, type;
    if (spot2_.check(x, y, &position, &type) < 0) return;
    compute_selected_base();
    const int faction = BaseCurrent->faction_id_current;
    if (faction != ConsoleGlobal.faction_id_ && !(GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)
        && !(PlayersData[ConsoleGlobal.faction_id_].diplo_treaties[faction] & 1)
        && type != 6) return;
    if (is_double) return;
    if (type == 0) farm_click(x, y, right);
    else if (type == 2) facilities_click(position, x, y, right);
}

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
    RECT saved_clip = canvas->current_clip();
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
    const int text_offset = psych_display_sub(row_pitch / 2, font.pixel_height() / 2);
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
                const int police = psych_display_add(PlayersData[faction].soc_effect_pending.values_[SocialEffect::POLICE],
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

namespace {
int hurry_signed_bits(uint32_t value) {
    return value <= 0x7FFFFFFFu ? static_cast<int>(value)
        : -1 - static_cast<int>(0xFFFFFFFFu - value);
}
int hurry_add(int left, int right) {
    return hurry_signed_bits(static_cast<uint32_t>(left) + static_cast<uint32_t>(right));
}
int hurry_sub(int left, int right) {
    return hurry_signed_bits(static_cast<uint32_t>(left) - static_cast<uint32_t>(right));
}
int hurry_mul(int left, int right) {
    return hurry_signed_bits(static_cast<uint32_t>(left) * static_cast<uint32_t>(right));
}
struct HurryPrice { int adjusted_cost; int gross_cost; };
HurryPrice hurry_price(int base_id, int production_id, int accounting_faction) {
    const int negative_id = hurry_sub(0, production_id);
    const int item_cost = production_id >= 0 ? veh_cost(production_id, base_id, 0)
                                             : Facility[negative_id].cost;
    Base *live_base = BaseCurrent;
    const int factor_faction = live_base->faction_id_current;
    const int gross_cost = hurry_mul(cost_factor(factor_faction, 1, -1), item_cost);
    live_base = BaseCurrent;
    const int current_production_id = live_base->queue_production_id[0];
    int current_minerals = live_base->minerals_accumulated;
    const int remaining = hurry_sub(gross_cost, current_minerals);
    int cost = current_production_id >= 0
        ? hurry_add(hurry_mul(remaining, remaining) / 20, hurry_add(remaining, remaining))
        : hurry_add(remaining, remaining);
    if (negative_id >= 0x46) {
        cost = hurry_add(cost, cost);
        live_base = BaseCurrent;
        const int low_minerals = live_base->minerals_accumulated;
        current_minerals = low_minerals;
        const int second_factor_faction = live_base->faction_id_current;
        if (low_minerals < hurry_mul(cost_factor(second_factor_faction, 1, -1), 4))
            cost = hurry_add(cost, cost);
    }
    if (current_minerals < static_cast<int>(Rules.retool_exemption)) cost = hurry_add(cost, cost);
    if (GameDataStateStorage.fields_.secret_project_.voice_of_planet() >= 0) cost = hurry_add(cost, cost);
    const int hurry_percent = Players[accounting_faction].rule_hurry;
    if (hurry_percent != 100) cost = hurry_mul(hurry_percent, cost) / 100;
    HurryPrice result = {cost, gross_cost};
    return result;
}
char *expand_hurry_dialog_item() {
    char *source = text_get();
    StringTemp[0] = 0;
    parse_string(source, StringTemp);
    return StringTemp;
}
} // namespace

static int production_signed_bits(uint32_t bits) {
    return bits <= 0x7FFFFFFFu ? static_cast<int>(bits)
        : -1 - static_cast<int>(0xFFFFFFFFu - bits);
}
static int production_add(int left, int right) {
    return production_signed_bits(static_cast<uint32_t>(left)
                                  + static_cast<uint32_t>(right));
}
static int production_sub(int left, int right) {
    return production_signed_bits(static_cast<uint32_t>(left)
                                  - static_cast<uint32_t>(right));
}
static int production_mul(int left, int right) {
    return production_signed_bits(static_cast<uint32_t>(left)
                                  * static_cast<uint32_t>(right));
}

static void set_production_parse_name(int slot, int production_id) {
    if (production_id >= 0) {
        parse_says(slot, VehPrototypes[production_id].veh_name, -1, -1);
    } else {
        parse_say(slot,
                  Facility[-production_id].name,
                  -1, -1);
    }
}

// Selected production IDs are signed picker results. This is the image's
// exact 0x417944 mapping; keep 9999 out of this transform because it branches
// to 0x417EDD first.
static int normalize_production_selection(int selected_id) {
    return selected_id >= 0x200 ? 0x200 - selected_id : selected_id;
}

// Called at native 0x417930 for the ordinary action (field_A778_ == 0,
// whose decremented switch index is 0xFFFFFFFF and fails the unsigned table
// range check). `normalized_id` is the already-normalized 0x417944 result;
// callers retain the separate 0x417EDD action-9999 path.
static bool apply_retool_cost_prompt(BaseWin &window, Popup &popup,
                                     int normalized_id, int faction_snapshot,
                                     int &queue_slot, int a1) {
    if (a1 != 0 || normalized_id != BaseCurrent->queue_production_id[0]) {
        BaseCurrent->state |= 0x80000000u;  // Base+0x30, native 0x417961
        if (BaseCurrent->governor & 0x40000000u)
            BaseCurrent->governor &= 0x3FFFFFFFu; // native AND at 0x41797D
    }
    if (a1 != 0) return false;  // native 0x417985 -> 0x417C95

    // 0x417992..0x4179C6: reserve must be nonzero and selected/current
    // retool categories must differ. Equality copies the saved reserve to
    // minerals_accumulated at native 0x417C8C instead of opening a popup.
    if (BaseCurrent->minerals_accumulated_2 == 0) return false;
    const int first_base_id = BaseIDCurrentSelected;
    const int selected_category = base_making(normalized_id, first_base_id);
    const int current_displayed_id = window.displayed_production_id();
    const int second_base_id = BaseIDCurrentSelected;
    const int displayed_category = base_making(current_displayed_id, second_base_id);
    if (selected_category == displayed_category) {
        BaseCurrent->minerals_accumulated =
            BaseCurrent->minerals_accumulated_2;
        return false;
    }

    if (PlayersData[faction_snapshot].diff_level <= 1
        || Rules.retool_pct_pen_prod_chg == 0
        || BaseCurrent->minerals_accumulated_2
               <= static_cast<int>(Rules.retool_exemption)
        || window.displayed_production_id() == -0x45) {
        BaseCurrent->minerals_accumulated =
            BaseCurrent->minerals_accumulated_2;
        return false;
    }

    // Native 0x417A23..0x417A4C requires the live current-production and
    // displayed-production retool categories to agree; unlike the earlier
    // equality case, failure skips the prompt without copying reserve.
    // Native 0x417A14 saves this displayed ID before the first callback.
    const int second_displayed_id = window.displayed_production_id();
    const int current_queue_id = BaseCurrent->queue_production_id[0];
    const int queue_base_id = BaseIDCurrentSelected;
    const int current_category = base_making(current_queue_id, queue_base_id);
    const int displayed_base_id = BaseIDCurrentSelected;
    const int second_displayed_category =
        base_making(second_displayed_id, displayed_base_id);
    if (current_category != second_displayed_category) {
        return false;
    }

    set_production_parse_name(0, BaseCurrent->queue_production_id[0]);
    set_production_parse_name(1, normalized_id);

    const int exemption = static_cast<int>(Rules.retool_exemption);
    const int percent = static_cast<int>(Rules.retool_pct_pen_prod_chg);
    const int reserve = BaseCurrent->minerals_accumulated_2;
    const int proposed_reserve = production_add(
        exemption, production_mul(production_sub(reserve, exemption),
                                  production_sub(100, percent)) / 100);

    parse_num(0, percent);
    parse_num(1, static_cast<int>(Rules.retool_exemption));
    parse_num(2, production_sub(BaseCurrent->minerals_accumulated, proposed_reserve));

    // Image literals at 0x682BC0 and 0x682BD0 were read from the pinned PE:
    // "RETOOLPENALTY" and "A". The byte strings are source literals here,
    // not fixed-address executable calls.
    StringTemp[0] = 0;
    strcat(StringTemp, "RETOOLPENALTY");
    char strictness_text[0x50];
    _itoa(static_cast<int>(Rules.retool_strictness), strictness_text, 10);
    strcat(StringTemp, strictness_text);
    if (Rules.retool_exemption != 0) strcat(StringTemp, "A");

    popup.start(PopupStartCaption, StringTemp, -1, 0, 0, 0);
    if (BaseCurrent->queue_size == 0)
        popup.dialogs().item(label_get(0xB0C / 4), 2);
    popup.dialogs().item(label_get(0xB10 / 4), 3);
    const int result = popup.exec();

    // Native table 0x418DDC has only result 0..3; unsigned-above falls to
    // 0x417C95 without applying a mutation.
    // Native table index 0 goes to 0x4181FD: unlock/visibility restoration
    // and method teardown, without logging or applying the ordinary tail.
    if (result == 0) {
        return true;
    } else if (result == 1) {
        BaseCurrent->minerals_accumulated = proposed_reserve;
    } else if (result == 2) {
        int slot;
        for (slot = 8; slot >= 1; --slot)
            BaseCurrent->queue_production_id[slot + 1] =
                BaseCurrent->queue_production_id[slot];
        uint32_t count = BaseCurrent->queue_size + 1;
        if (static_cast<int32_t>(count) < 0) count = 0;
        else if (count > 9) count = 9;
        BaseCurrent->queue_size = count;
        queue_slot = 1;
        BaseCurrent->queue_production_id[1] = normalized_id;
    } else if (result == 3) {
        uint32_t count = BaseCurrent->queue_size;
        if (static_cast<int32_t>(count) < 9) {
            ++count;
            BaseCurrent->queue_size = count;
        }
        queue_slot = static_cast<int32_t>(count);
        BaseCurrent->queue_production_id[queue_slot] = normalized_id;
    }
    return false;
}

// Caller edge before this fragment: selected value 9999 branches to 0x417EDD;
// otherwise normalize_production_selection() supplies the ID used below.
// Native edge following all result arms: 0x417C95 logs the currently chosen
// slot and current queue item. If the chosen slot is zero, 0x417CC0 writes the
// selected ID to queue_production_id[0] (the production-ID-last value is not
// this native field). This source-level continuation completes those edges:
static bool finish_ordinary_selection(BaseWin &window, Popup &popup,
                                      int selected_id,
                                      int faction_snapshot,
                                      int &queue_slot, int a1) {
    const int normalized_id = normalize_production_selection(selected_id);
    if (apply_retool_cost_prompt(window, popup, normalized_id,
                                 faction_snapshot, queue_slot, a1))
        return true;

    log_say("BASE: changed production", BaseIDCurrentSelected, queue_slot,
            BaseCurrent->queue_production_id[queue_slot]);
    if (queue_slot != 0) return false; // native 0x417CB8 -> 0x417D9B
    BaseCurrent->queue_production_id[0] = normalized_id;

    // The native UI reaction is based on the newly written live queue[0].
    // The callback can change BaseCurrent, so both drawing and subsequent
    // selection are intentionally sequenced around the live reloads.
    int live_queue_id = BaseCurrent->queue_production_id[0];
    if (live_queue_id <= -0x46) wave_it(0x24); // native 0x417CD1/4455F0
    draw_radius(BaseCurrent->x, BaseCurrent->y, 2, 2); // native 0x417CF3
    live_queue_id = BaseCurrent->queue_production_id[0];

    // Native 0x417D01 gate reads queue[0] again after draw_radius. The
    // secondary plan byte is VehPrototype+0x2A, not Base+0x0A.
    const bool update_stack = live_queue_id <= -0x46
        || (live_queue_id > 0x40
            && (VehPrototypes[live_queue_id].flags & 4u) == 0);
    if (update_stack) {
        int vehicle_id = veh_at(BaseCurrent->x, BaseCurrent->y);
        while (vehicle_id >= 0) {
            Veh &vehicle = Vehs[vehicle_id];
            if (vehicle.faction_id == BaseCurrent->faction_id_current) {
                const int proto_id = vehicle.proto_id; // native Veh+0x0A
                const uint8_t plan = VehPrototypes[proto_id].plan;
                if (plan == 12 || plan == 10) {
                    vehicle.year_end_lurking = 0;
                    vehicle.state &= ~VSTATE_UNK_200;
                    if (IsMultiplayerNet) synch_veh(vehicle_id);
                }
            }
            vehicle_id = vehicle.next_veh_id_stack;
        }
    }

    // Native caller tests ProdPicker::is_visible() only after this complete
    // tail: nonzero loops directly to 0x417751, zero takes 0x41850C success.
    // Keep that callback-sensitive test in the owning method.
    return false;
}

// Native success label 0x41850C..0x41870B. `old_queue_id` is the queue item
// copied at entry for the selected row, and faction_snapshot is the entry
// Base faction used by both native visibility walks. All Base/faction/count
// reads below are reloaded after the picker and any Popup callbacks.
static void finish_production_success(BaseWin &window, int a1, int a2,
                                      int old_queue_id,
                                      int faction_snapshot) {
    if (a2 == 0)
        NetDaemonNet.unlock_base(BaseIDCurrentSelected);

    Base *live_base = BaseCurrent;
    const int last_queue_slot = static_cast<int>(live_base->queue_size);
    for (int slot = 0; slot <= last_queue_slot; ++slot) {
        const int id = live_base->queue_production_id[slot];
        if (id >= 0) {
            uint8_t &queued = PlayersData[faction_snapshot].proto_id_queue[id];
            queued = static_cast<uint8_t>(queued + 1);
        }
    }

    live_base = BaseCurrent;
    const int new_queue_id = live_base->queue_production_id[0];
    if (IsMultiplayerNet && a1 == 0 && new_queue_id != old_queue_id) {
        const int live_faction = live_base->faction_id_current;
    int new_id_count = 0;
    int old_id_count = 0;
    const int focused_faction = Bases[window.focused_base_id()].faction_id_current;
    if (focused_faction == live_faction) {
        for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
            if (base_id == BaseIDCurrentSelected) continue;
            const int queued_id = Bases[base_id].queue_production_id[0];
            if (queued_id == new_queue_id) ++new_id_count;
            if (queued_id == old_queue_id) ++old_id_count;
        }
    }

        // 0x4185A4..F9 compares the focused Base faction against the live
        // BaseCurrent faction once; only if equal does it count queue leaders
        // across all Bases except BaseIDCurrentSelected. The native loop does
        // not read each candidate's faction byte. The exact signed predicates
        // and payload shapes below follow 0x418604..0x418662.
        const bool old_is_facility = old_queue_id <= -0x46;
        const bool new_is_facility = new_queue_id <= -0x46;
        if (old_is_facility && new_is_facility) {
            if (old_id_count != 0 && new_id_count == 0) {
                message_data(0x4D00, 0, -0x46 - new_queue_id, 0, 0, 0);
            } else if (old_id_count == 0 && new_id_count != 0) {
                message_data(0x4D00, 0, -0x46 - old_queue_id, 1, 0, 0);
            } else if (old_id_count == 0 && new_id_count == 0) {
                message_data(0x4D00, 0, -0x46 - new_queue_id, 2,
                             -0x46 - old_queue_id, new_id_count);
            }
        } else if (old_is_facility && old_id_count == 0) {
            message_data(0x4D00, 0, -0x46 - old_queue_id, 1, 0, 0);
        } else if (new_is_facility && new_id_count == 0) {
            message_data(0x4D00, 0, -0x46 - new_queue_id, 0, 0, 0);
        }
    }

    base_compute(1);
    window.redraw(); // native virtual slot +0xF8 on this BaseWin
    MainInterfaceGlobal.GraphicWin::redraw();

    draw_radius(BaseCurrent->x, BaseCurrent->y, 1, 2);
    live_base = BaseCurrent; // native reload after draw_radius callback
    const int x = live_base->x;
    const int y = live_base->y;
    const int cursor_history_index = ConsoleGlobal.cursor_history_index_;
    if (ConsoleGlobal.cursor_history_x_[cursor_history_index] == x
        && ConsoleGlobal.cursor_history_y_[cursor_history_index] == y)
        ConsoleGlobal.update_data(0);

    if (a1 == 0 && BaseCurrent->queue_production_id[0] == -0x65) {
        interlude(10, 0, 1, 0);
        GameDataStateStorage.fields_.game_state_ |= 0x800u;
    }
}

// Production selection uses the real picker, queue, retool dialog and UI owners.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00417040 ?production@BaseWin@@QAEXHH@Z 0x00417040-0x00418DBD;0x00651BB0-0x0065221C
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/00417040.cpp", "image:0x00417040-0x00418DBD and companion cleanup0x00651BB0-0x0065221C", "image:0x00418DC0 seven action targets and0x00418DDC four retool targets", "source:src/basewin.h;src/prodpicker.h;src/base.h;src/veh.h"], "reason": "Recover production selection through genuine Popup/GraphicWin lifetimes, embedded ProdPicker action state, typed production queues and vehicle prototypes. Preserve callback-time live state, cancellation, visibility accounting and signed arithmetic. The workshop action reaches an explicitly unfinished owning design_workshop dependency, so this implementation is not complete gameplay or matching acceptance."}
void BaseWin::production(int a1, int a2) {
    Popup popup;
    Popup popup_design;
    Popup popup_secondary;
    Popup popup_tertiary;
    GraphicWin picker_preview;
    (void)popup_design;
    (void)popup_secondary;
    (void)popup_tertiary;

    compute_selected_base();
    const int entry_base_id = focused_base_id();
    if (Bases[entry_base_id].faction_id_current != ConsoleGlobal.faction_id_
        && (GameDataStateStorage.fields_.game_state_ & 0x80u) == 0)
        return;

    Base *entry_base = BaseCurrent;
    const int faction_snapshot = entry_base->faction_id_current;
    const int old_queue_id = entry_base->queue_production_id[a1];
    const uint32_t entry_queue_size = entry_base->queue_size;
    int entry_queue[10];
    std::memcpy(entry_queue, entry_base->queue_production_id,
                sizeof(entry_queue));
    displayed_production_id_ = entry_base->production_id_last;
    if (a1 == 0) entry_base->state &= ~0x00800000u;

    if (a2 == 0
        && NetDaemonNet.lock_base(BaseIDCurrentSelected, 0, -1, -1) != 0)
        return;

    // Native 0x4175F4 subtracts queued-prototype visibility exactly once
    // before the picker loop; loop backs at 0x41763D do not repeat it.
    int last_queue_slot = static_cast<int32_t>(BaseCurrent->queue_size);
    for (int slot = 0; slot <= last_queue_slot; ++slot) {
        const int id = BaseCurrent->queue_production_id[slot];
        if (id >= 0) {
            uint8_t &queued = PlayersData[faction_snapshot].proto_id_queue[id];
            queued = static_cast<uint8_t>(queued - 1);
        }
    }

    bool run_picker_setup = true;
    for (;;) {
        if (run_picker_setup) { // native label 0x41763D
            compute_selected_base();
            const int preview_width = pixel_width() - 0x2E;
            const int preview_height = pixel_height();
            picker_preview.init(0x17, 0, preview_width, preview_height, 0,
                                0x01000020, this, 0, 0);
            buffer1_.copy(&picker_preview, 0x17, 0, 0, 0,
                          preview_width, preview_height);
            picker_preview.tile(&sprite6_, 0, 0, 3, 3,
                                preview_width - 6, preview_height - 6);
        }
        run_picker_setup = false;

        int selected_id = prodPicker_.exec(BaseCurrent,
            BaseCurrent->faction_id_current, &picker_preview, field_40B2C_);

        // Native special cancel with a nonzero a1 restores the entry queue,
        // then resets the Base before entering the ordinary post-picker path.
        if (a1 != 0 && selected_id == -9999) {
            Base *live_base = BaseCurrent;
            std::memcpy(live_base->queue_production_id, entry_queue,
                        sizeof(entry_queue));
            live_base->queue_size = entry_queue_size;
            draw_production(0); // native 0x4177B7 after queue snapshot restore
        }

        // 0x4177BC refreshes the selected Base after picker callbacks, then
        // plays the selection sound. All subsequent decisions use the live
        // selected Base except the specifically snapshotted queue/faction IDs.
        compute_selected_base();
        g_FX.play(0x0F);

        if (selected_id <= -9999 || (GameDataStateStorage.fields_.game_state_ & 0x800u) != 0) {
            field_40B2C_ = 0;
            draw_queue(1);
            if (a2 == 0)
                NetDaemonNet.unlock_base(BaseIDCurrentSelected);
            Base *live_base = BaseCurrent;
            last_queue_slot = static_cast<int32_t>(live_base->queue_size);
            for (int slot = 0; slot <= last_queue_slot; ++slot) {
                const int id = live_base->queue_production_id[slot];
                if (id >= 0) {
                    uint8_t &queued = PlayersData[faction_snapshot].proto_id_queue[id];
                    queued = static_cast<uint8_t>(queued + 1);
                }
            }
            return; // native 0x418B20 then lexical owner teardown
        }

        const int action = prodPicker_.selected_action() - 1;
        if (action == 0) { // native 0x417DB3: help/design selected row
            if (selected_id < 0) {
                const int facility_id = -selected_id;
                help_topic(facility_id < 0x46 ? 10u : 11u, facility_id);
            } else if (selected_id == 9999) {
                run_picker_setup = true;
                continue; // native 0x417DE6 -> full setup 0x41763D
            } else if (selected_id < 0x200) {
                TutWinDesWindow.exec(BaseCurrent->faction_id_current,
                                     selected_id);
            } else {
                const int facility_id = selected_id - 0x200;
                help_topic(facility_id < 0x46 ? 10u : 11u, facility_id);
            }
            run_picker_setup = true;
            continue; // native 0x41763D
        }

        if (action == 1 || action == 2) {
            TutWinDesWindow.exec(BaseCurrent->faction_id_current,
                                 action == 1
                                     ? (selected_id < 0 ? -1 : selected_id)
                                     : -1);
            compute_selected_base();
            prodPicker_.calculate();
            run_picker_setup = true;
            continue; // native 0x417F6A -> 0x41763D
        }

        if (action == 3 || action == 5) {
            if (a1 == 0) {
                base_reset(BaseIDCurrentSelected, 0);
                if (a2 == 0)
                    NetDaemonNet.unlock_base(BaseIDCurrentSelected);
                redraw();
                MainInterfaceGlobal.GraphicWin::redraw();
                Base *live_base = BaseCurrent;
                last_queue_slot = static_cast<int32_t>(live_base->queue_size);
                for (int slot = 0; slot <= last_queue_slot; ++slot) {
                    const int id = live_base->queue_production_id[slot];
                    if (id >= 0) {
                        uint8_t &queued = PlayersData[faction_snapshot].proto_id_queue[id];
                        queued = static_cast<uint8_t>(queued + 1);
                    }
                }
                // 0x417FF5 begins the preview/UI teardown and 0x4181F8
                // transfers to the function's common epilogue. This branch
                // returns; it does not re-enter the picker/design path.
                return;
            }

            const int selected_slot = static_cast<int>(field_40B2C_);
            if (selected_slot > static_cast<int>(BaseCurrent->queue_size)) {
                --field_40B2C_;
                if (static_cast<int32_t>(field_40B2C_) <= 1)
                    field_40B2C_ = 1;
                draw_queue(1);
            } else {
                for (int slot = selected_slot; slot < 9; ++slot)
                    BaseCurrent->queue_production_id[slot] =
                        BaseCurrent->queue_production_id[slot + 1];
                --BaseCurrent->queue_size;
                if (static_cast<int32_t>(field_40B2C_)
                    > static_cast<int32_t>(BaseCurrent->queue_size)) {
                    field_40B2C_ = BaseCurrent->queue_size;
                    if (static_cast<int32_t>(field_40B2C_) <= 1)
                        field_40B2C_ = 1;
                }
                draw_queue(1);
                draw_queue(1);
            }
        } else if (action == 4 && a1 != 0) {
            add_queue(selected_id, 1);
        } else if (action == 6 && a1 != 0) {
            add_queue(selected_id, 0);
        }

        if (selected_id == 9999) {
            design_workshop(BaseCurrent->faction_id_current, 1);
            compute_selected_base();
            prodPicker_.calculate();
            run_picker_setup = true;
            continue; // native 0x417EDD..0x417F75
        }

        int queue_slot = a1;
        const bool cancelled_retool = finish_ordinary_selection(
            *this, popup, selected_id, faction_snapshot, queue_slot, a1);
        if (cancelled_retool) {
            if (a2 == 0)
                NetDaemonNet.unlock_base(BaseIDCurrentSelected);
            Base *live_base = BaseCurrent;
            last_queue_slot = static_cast<int32_t>(live_base->queue_size);
            for (int slot = 0; slot <= last_queue_slot; ++slot) {
                const int id = live_base->queue_production_id[slot];
                if (id >= 0) {
                    uint8_t &queued =
                        PlayersData[faction_snapshot].proto_id_queue[id];
                    queued = static_cast<uint8_t>(queued + 1);
                }
            }
            return; // native 0x4181FD result-table entry 0
        }
        if (prodPicker_.is_visible()) {
            run_picker_setup = false;
            continue; // native 0x417DAE -> picker body 0x417751
        }

        field_40B2C_ = 0; // native 0x41866C, immediately before base_compute
        finish_production_success(*this, a1, a2, old_queue_id,
                                  faction_snapshot);
        return;
    }
}

// ORIGINAL: 0x00418DF0 ?hurry@BaseWin@@QAEXXZ 0x00418DF0-0x00419A8E;0x0065221C-0x0065249C
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00418df0.cpp", "image:0x00418DF0-0x00419A8E native control-flow and operand review", "source:src/basewin.h", "source:src/base.h"], "reason": "Recover full hurry pricing, dialog, payment, lock and Base update flow through source-owned Base/PlayerData/UI state; retain native signed 32-bit arithmetic and callback-time reloads. Shared energy-mirror producer/consumer bodies remain seed-only, so that object's source binding is not proof of end-to-end energy runtime behavior."}
// TRIED: `Popup popup;` RAII with raw popup-relative offsets for scroll_/flat_button/dialogs_/spot_/sprite_/heap_ reproduces the close cascades' shape and offsets (cross-checked against the scaffold's independently-derived field layout), but the success-path teardown ends with a manual GraphicWin dtor call rather than suppressing the compiler's own implicit ~Popup() at the closing brace, which the original does not emit on that path; MISMATCH #2.
void BaseWin::hurry() {
    Popup popup;
    int lock_held = 0;
    int selection_result = 0;
    int payment = 0;
    compute_selected_base();
    const int entry_base_id = focused_base_id();
    if (Bases[entry_base_id].faction_id_current != ConsoleGlobal.faction_id_
        && (GameDataStateStorage.fields_.game_state_ & 0x80u) == 0) return;

    const int base_id = BaseIDCurrentSelected;
    Base *base = BaseCurrent;
    const int production_id = base->queue_production_id[0];
    const int negative_production_id = hurry_sub(0, production_id);
    const int faction_id = base->faction_id_current;
    const HurryPrice price = hurry_price(base_id, production_id, faction_id);
    const int cost = price.adjusted_cost;
    int mineral_target = price.gross_cost;
    if (cost <= 0) return;

    parse_says(0, BaseCurrent->name_string, -1, -1);
    base = BaseCurrent;
    const int current_production_id = base->queue_production_id[0];
    if (current_production_id >= 0) {
        parse_says(1, VehPrototypes[production_id].veh_name, -1, -1);
    } else {
        parse_says(1, StringTable.get(
            Facility[negative_production_id].name), -1, -1);
    }
    parse_num(0, cost);
    parse_num(1, hurry_sub(PlayersData[faction_id].energy_reserves,
                          PlayersData[faction_id].hurry_cost_total));
    popup.start(PopupStartCaption, "HURRY", -1, 0, 0, 0);

    base = BaseCurrent;
    const int drone_production_id = base->queue_production_id[0];
    if ((base->state & 2u) == 0
        || (drone_production_id < 0 && drone_production_id > -0x41)) {
        const int available = hurry_sub(PlayersData[faction_id].energy_reserves,
                                        PlayersData[faction_id].hurry_cost_total);
        if (available >= 0) {
            if (X_text_open(ScriptPopupFile, "HURRYOPTIONS") != 0) return;
            popup.dialogs().item(expand_hurry_dialog_item(), 0);
            char *full_payment_item = expand_hurry_dialog_item();
            const int available_after_expansion = hurry_sub(
                PlayersData[faction_id].energy_reserves,
                PlayersData[faction_id].hurry_cost_total);
            if (available_after_expansion >= cost)
                popup.dialogs().item(full_payment_item, 1);
            popup.dialogs().item(expand_hurry_dialog_item(), 2);
            if (NetDaemonNet.lock_base(BaseIDCurrentSelected, 0, -1, -1) != 0) return;
            lock_held = 1;
        }
    } else {
        popup.start(PopupStartCaption, "HURRYDRONE", -1, 0, 0x80, 0);
    }

    selection_result = popup.exec(0, ConsoleControlTurnActive ? 0 : pop_expire);
    g_FX.play(0x0F);
    compute_selected_base();
    if (selection_result < 1 || (GameDataStateStorage.fields_.game_state_ & 0x800u) != 0) {
        if (lock_held) NetDaemonNet.unlock_base(BaseIDCurrentSelected);
        return;
    }
    base = BaseCurrent;
    if ((base->state & 0x40000000u) != 0) {
        X_pop("PARTIALONCE", 0);
        if (lock_held) NetDaemonNet.unlock_base(BaseIDCurrentSelected);
        return;
    }

    if (selection_result > 1) {
        int available = hurry_sub(PlayersData[faction_id].energy_reserves,
                                  PlayersData[faction_id].hurry_cost_total);
        int ask_amount = cost;
        if (ask_amount > available) ask_amount = available;
        parse_num(0, ask_amount);
        available = hurry_sub(PlayersData[faction_id].energy_reserves,
                              PlayersData[faction_id].hurry_cost_total);
        ask_amount = cost;
        if (ask_amount > available) ask_amount = available;
        if (X_pop_ask_number("HURRYPAYMENT", ask_amount, 0) != 0) {
            if (lock_held) NetDaemonNet.unlock_base(BaseIDCurrentSelected);
            return;
        }
        available = hurry_sub(PlayersData[faction_id].energy_reserves,
                              PlayersData[faction_id].hurry_cost_total);
        payment = ParseNumTable[0];
        if (payment < 0 || available < 0) payment = 0;
        else if (payment > available) payment = available;
        const int current_minerals = BaseCurrent->minerals_accumulated;
        mineral_target = hurry_add(
            hurry_mul(hurry_sub(price.gross_cost, current_minerals), payment)
                / price.adjusted_cost,
            current_minerals);
        BaseCurrent->state |= 0x40000000u;
    } else {
        payment = cost;
    }

    if (IsMultiplayerNet && ConsoleControlTurnActive) {
        PlayersData[faction_id].hurry_cost_total = hurry_add(
            PlayersData[faction_id].hurry_cost_total, payment);
    } else {
        PlayersData[faction_id].energy_reserves = hurry_sub(
            PlayersData[faction_id].energy_reserves, payment);
        CurrentFactionEnergyReserves = hurry_sub(CurrentFactionEnergyReserves, payment);
        if (IsMultiplayerNet) synch_energy(faction_id);
    }
    field_40B38_ = mineral_target;
    BaseCurrent->minerals_accumulated_2 = mineral_target;
    BaseCurrent->minerals_accumulated = mineral_target;
    BaseCurrent->production_id_last = BaseCurrent->queue_production_id[0];
    log_say("BASE: hurried production", BaseIDCurrentSelected, 0, 0);
    NetDaemonNet.unlock_base(BaseIDCurrentSelected);
    lock_held = 0;
    redraw();
    MainInterfaceGlobal.redraw();
    ConsoleInfoWin.set_redraw_phase(0);
    ConsoleInfoWin.on_redraw();
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
    if (((unsigned char)GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1 << BaseCurrent->faction_id_current)) == 0) {
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

// ORIGINAL: 0x00415F00 ?gov_activate@BaseWin@@QAEXXZ 0x00415F00-0x00415F70 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00415F00-0x00415F70 governor flags and Base field accesses", "source:src/base.h", "source:src/base.cpp"], "reason": "Activate the selected base's configured governor through typed Base state and existing base computation helpers; preserve the source seed's historical exact claim."}
void BaseWin::gov_activate() {
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
}

// ORIGINAL: 0x00416D90 ?recompute@BaseWin@@QAEXH@Z 0x00416D90-0x00416DCA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00416D90-0x00416DCA tests Win visibility, selects focusedBaseID_, recomputes, dispatches Win redraw slot 62 and directly redraws MainInterfaceGlobal", "source:src/basewin.h", "source:src/base.h", "source:src/maininterface.h"], "reason": "Refresh the selected base through typed focus state and real virtual/direct redraw calls; the integer argument is unused by the original."}
void BaseWin::recompute(int) {
    if (Win::is_visible()) {
        set_base(static_cast<int>(focusedBaseID_));
        base_compute(1);
        redraw();
        MainInterfaceGlobal.GraphicWin::redraw();
    }
}

// The image walks Bases by the 0x134-byte Base record and compares names
// case-insensitively.  The first qualifying name in the requested direction
// wins; the second comparison supplies the wrap-around candidate.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0041A010 ?prev@BaseWin@@QAEXXZ 0x0041A010-0x0041A15D
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/0041a010.cpp;source:src/base.h;image:0x0041A010-0x0041A15D"], "reason": "Recover previous base selection through the actual Base array, selected-base member, faction context and case-insensitive name order."}
void BaseWin::prev() {
    set_base(static_cast<int>(focusedBaseID_));
    if (BaseCurrent->faction_id_current != ConsoleGlobal.faction_id_
        && !(GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)) {
        return;
    }
    if (ConsoleControlTurnActive) {
        X_pop("ARROWUPKEEP", nullptr);
        return;
    }

    int selected = static_cast<int>(focusedBaseID_);
    int nearest = -1;
    for (int candidate = 0; candidate < GameDataStateStorage.fields_.base_current_count_; ++candidate) {
        if (candidate == static_cast<int>(focusedBaseID_)
            || Bases[candidate].faction_id_current != Bases[focusedBaseID_].faction_id_current) {
            continue;
        }
        const int comparison = _strcmpi(Bases[candidate].name_string,
                                         Bases[focusedBaseID_].name_string);
        if (comparison < 0
            && (nearest < 0 || _strcmpi(Bases[candidate].name_string,
                                         Bases[nearest].name_string) > 0)) {
            nearest = candidate;
        }
        if (_strcmpi(Bases[candidate].name_string, Bases[selected].name_string) > 0) {
            selected = candidate;
        }
    }
    if (nearest >= 0) {
        selected = nearest;
    }
    focusedBaseID_ = selected;
    zoom(selected, 0);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0041A160 ?next@BaseWin@@QAEXXZ 0x0041A160-0x0041A2AD
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/0041a160.cpp;source:src/base.h;image:0x0041A160-0x0041A2AD"], "reason": "Recover next base selection through the actual Base array, selected-base member, faction context and case-insensitive name order."}
void BaseWin::next() {
    set_base(static_cast<int>(focusedBaseID_));
    if (BaseCurrent->faction_id_current != ConsoleGlobal.faction_id_
        && !(GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)) {
        return;
    }
    if (ConsoleControlTurnActive) {
        X_pop("ARROWUPKEEP", nullptr);
        return;
    }

    int selected = static_cast<int>(focusedBaseID_);
    int nearest = -1;
    for (int candidate = 0; candidate < GameDataStateStorage.fields_.base_current_count_; ++candidate) {
        if (candidate == static_cast<int>(focusedBaseID_)
            || Bases[candidate].faction_id_current != Bases[focusedBaseID_].faction_id_current) {
            continue;
        }
        const int comparison = _strcmpi(Bases[candidate].name_string,
                                         Bases[focusedBaseID_].name_string);
        if (comparison > 0
            && (nearest < 0 || _strcmpi(Bases[candidate].name_string,
                                         Bases[nearest].name_string) < 0)) {
            nearest = candidate;
        }
        if (_strcmpi(Bases[candidate].name_string, Bases[selected].name_string) < 0) {
            selected = candidate;
        }
    }
    if (nearest >= 0) {
        selected = nearest;
    }
    focusedBaseID_ = selected;
    zoom(selected, 0);
}

// Queue mutation and its rendered panel use the same typed Base queue and
// BaseWin selection state recovered from the original record strides.
// ORIGINAL: 0x00415080 ?draw_queue@BaseWin@@QAEXH@Z 0x00415080-0x00415724;0x00651B93-0x00651BB0 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00415080.cpp", "src/basewin.h:queue rectangles and font1_", "src/base.h:Base queue and Facility", "src/veh.h:VehPrototypes", "src/alpha.h:Labels and Language", "src/buffer.h:InterfaceBoxStyles", "image:0x00682BAC-0x00682BC0 queue strings"], "reason": "Render the eight queued production rows through owned BaseWin rectangles, game queue and prototype/facility records, shared interface styles and palette. Retain the original seed for byte restoration; the old raw self-offset transcription is not product source."}
// TRIED: the retained instruction-level seed with local Font lifetimes and
// TRIED: instruction-level transcription with real Font locals (to get the SEH frame/dtor thunks) and byte-offset casts for BaseWin fields; compiles, control flow/offsets/call order match the disasm, sim 0.70 framed build. Extra RECT-copy temporaries (rectA/rectQ/rectR/rectP vs the original's reused stack slots) keep it off byte-exact. Not retried with slot reuse given size (1729 bytes, SEH).
// raw self offsets scored about 0.70 in a framed build. Its several RECT-copy
// temporaries blocked byte restoration; this typed body keeps that evidence
// without treating those stack-slot spellings as the class model.
void BaseWin::draw_queue(int update_mode) {
    Font row_font;
    Font compact_font;
    Buffer *canvas = static_cast<Buffer *>(&MainInterfaceGlobal);
    if (update_mode != 0 && !is_visible()) return;

    RECT saved_clip = canvas->current_clip();
    RECT outer = queue_outer_rect_;
    canvas->set_clip(&outer);
    canvas->box_sprite(&queue_outer_rect_, &InterfaceBoxStyles[3]);

    const bool low_resolution = WinScreenWidth == 0x320;
    row_font.init(DefaultFontFace, low_resolution ? 10 : 12, 0);
    row_font.unk_1_ = 4;
    compact_font.init(DefaultFontFace, low_resolution ? 8 : 10, 0);
    compact_font.unk_1_ = 4;
    spot1_.kill_type(8);

    RECT title = queue_title_rect_;
    canvas->box_sprite(&title, &InterfaceBoxStyles[6]);
    title.left += 3;
    title.top += 3;
    title.right -= 3;
    title.bottom -= 3;
    canvas->set_font(&font1_, nullptr, nullptr, nullptr);
    canvas->set_text_color(TextColourTable[82], TextColourTable[83], 1, 1);
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(479));
    CharUpperA(StringTemp);
    canvas->write_cent_l(StringTemp, &title, strlen(StringTemp));

    canvas->set_font(&row_font, nullptr, nullptr, nullptr);
    RECT content = queue_content_rect_;
    canvas->box_sprite(&content, &InterfaceBoxStyles[0]);
    const int margin = low_resolution ? 5 : 2;
    content.left += margin;
    content.top += margin;
    content.right -= margin;
    content.bottom -= margin;

    int row_top = content.top;
    for (int queue_index = 1; queue_index <= 8; ++queue_index) {
        canvas->set_font(&row_font, nullptr, nullptr, nullptr);
        const int row_height = row_font.line_advance();
        RECT row = {content.left, row_top, content.right, row_top + row_height};
        spot1_.add(queue_index + 2, 8, row.left, row.top,
                   row.right - row.left, row_height);

        const bool selected = field_40B2C_ == static_cast<uint32_t>(queue_index)
            || field_40B30_ == static_cast<uint32_t>(queue_index);
        if (selected) {
            Buffer *pattern = field_40B2C_ == static_cast<uint32_t>(queue_index)
                ? InterfaceBoxStyles[29].mask_ : InterfaceBoxStyles[27].mask_;
            canvas->tile(pattern, row.left, row.top, row.left, row.top,
                         row.right - row.left, row.bottom - row.top);
        }

        StringTemp[0] = 0;
        char number[64];
        _itoa(queue_index, number, 10);
        strcat(StringTemp, number);
        strcat(StringTemp, ". ");

        char item_name[256];
        item_name[0] = 0;
        if (queue_index > static_cast<int>(BaseCurrent->queue_size)) {
            canvas->set_text_color(TextColourTable[138], TextColourTable[139], 1, 1);
            strcat(item_name, label_get(460));
        } else {
            canvas->set_text_color(TextColourTable[136], TextColourTable[137], 1, 1);
            const int production = BaseCurrent->queue_production_id[queue_index];
            if (production >= 0) {
                strcat(item_name, VehPrototypes[production].veh_name);
            } else {
                // Facility.name is already an LPSTR; resolve it through the
                // shared Strings API as the other facility-name callers do.
                strcat(item_name, StringTable.get(
                    Facility[-production].name));
            }
        }
        if (selected) canvas->set_text_color(0x41, -1, 1, 1);
        const char *display_name = item_name;
        if (Language == 0 && strncmp(item_name, "The ", 4) == 0) {
            display_name += 4;
        }
        strcat(StringTemp, display_name);

        RECT text_rect = row;
        if (row_font.width(StringTemp) <= text_rect.right - text_rect.left) {
            canvas->set_clip(&text_rect);
            canvas->write_l(StringTemp, &text_rect, strlen(StringTemp));
            canvas->restore_clip();
        } else {
            canvas->set_font(&compact_font, nullptr, nullptr, nullptr);
            const bool truncated = compact_font.width(StringTemp)
                > text_rect.right - text_rect.left;
            if (truncated) text_rect.right -= compact_font.width("+");
            canvas->set_clip(&text_rect);
            if (StringTemp[0] != 0) {
                canvas->write_l(StringTemp, &text_rect, strlen(StringTemp));
            }
            canvas->restore_clip();
            if (truncated) {
                text_rect.left = text_rect.right;
                canvas->write_l("+", &text_rect, 1);
            }
        }
        row_top += row_font.line_advance();
    }

    if (update_mode != 0) MainInterfaceGlobal.update(&queue_content_rect_, nullptr);
    else MainInterfaceGlobal.soft_update(&queue_content_rect_);
    canvas->set_clip(&saved_clip);
}

// ORIGINAL: 0x0040E360 ?draw_production@BaseWin@@QAEXH@Z 0x0040E360-0x0040F0E5;0x00651ACD-0x00651AF8 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:f1948a8610bea1917865ab8599ffa231b86813d6:src/unrecovered/0040e360.cpp", "pinned:40E360..40F0E5 and651ACD..651AF8; native grid40EB3F..40EDF4", "source:src/basewin.h;src/base.h;src/caviar.h;src/interface_palette.cpp", "source:recovery/tests/test_basewin_production_actual_source.py"], "reason": "Recover the complete production panel through genuine BaseWin members, source-owned Base/Veh/Facility records, MainInterface Buffer operations and automatic two-Font/one-Sprite lifetimes. Both primary and exception-cleanup spans and the seed transcription lessons remain historical obligations. The original nested mineral grid, signed production selection, callback-sensitive live BaseCurrent reads, caption override lifetime and unclamped-surplus numerator are retained; no intentional bug fix or byte claim is granted. The embedded Caviar count is reset through its existing owner, not a new BaseWin field. Text colours use the existing initialized palette table, not executable-address aliases."}
// TRIED: BaseWin is opaque past field_40b10_ in this scaffold, so every `this`-relative access below 0x40C6C etc. is a raw I32/B8 cast rather than a named member; the two Font locals + one Sprite local (matching the two Font ctor calls and the single Sprite ctor/close pair in the disassembly) reproduce that RAII shape. The header/border setup and the unit-vs-facility info panel are transcribed from the disassembly; the queue-grid cell-size search and its per-item draw loop (~110 instructions) are replaced with a single representative call rather than the original's two-pass sizing + nested row/column loop, since the unnamed FUN_ callees for that section have no recovered signature to transcribe against.
void BaseWin::draw_production(int update_mode) {
    Font unit_font;
    Font detail_font;
    Sprite blank_sprite;
    RECT saved_clip = MainInterfaceGlobal.clip_rect();
    RECT panel = queue_help_rect_;
    RECT outer = queue_outer_rect_;
    RECT border;
    RECT content;
    char number[16];
    BoxSpriteParams panel_style = InterfaceBoxStyles[3];
    BoxSpriteParams outer_style = InterfaceBoxStyles[3];
    BoxSpriteParams flash_style = InterfaceBoxStyles[3];

    panel_style.corner3_ = 0;
    panel_style.edge3_ = 0;
    outer_style.corner0_ = 0;
    outer_style.edge0_ = 0;
    outer_style.corner1_ = 0;
    flash_style.corner0_ = &blank_sprite;
    flash_style.corner2_ = 0;
    flash_style.edge2_ = 0;

    // The native body copies style 3 before masking individual edge members.
    // Use the source-owned style record and typed Buffer API for both frames.
    MainInterfaceGlobal.set_clip(&panel);
    MainInterfaceGlobal.box_sprite(&panel, &panel_style);
    outer.left -= 5;
    outer.top += 5;
    outer.right -= 5;
    outer.bottom += 5;
    MainInterfaceGlobal.box_sprite(&outer, &outer_style);
    UnionRect(&border, &flash_rect_a_, &flash_rect_b_);
    border.left -= 3;
    border.top -= 3;
    border.right += 3;
    border.bottom += 3;
    blank_sprite.create_blank(6, 6, WinFillColour);
    MainInterfaceGlobal.box_sprite(&border, &flash_style);
    MainInterfaceGlobal.set_clip(&panel);

    content.left = panel.left + 3;
    content.top = panel.top + 3;
    content.right = panel.right - 3;
    content.bottom = panel.bottom - 3;
    MainInterfaceGlobal.box_sprite(&content, &InterfaceBoxStyles[0]);
    content.left += 3;
    content.top += 3;
    content.right -= 3;
    content.bottom -= 3;

    spot1_.kill_type(5);
    spot1_.add(0, 5, panel.left, panel.top,
               panel.right - panel.left, panel.bottom - panel.top);

    const int font_height = WinScreenWidth == 0x320 ? 10 : 12;
    unit_font.init(DefaultFontFace, font_height, 0);
    detail_font.init(DefaultFontFace, 12, 1);

    Base *base = BaseCurrent;
    const int production_id = base->queue_production_id[0];
    const bool is_vehicle = production_id >= 0;
    const int item_id = is_vehicle ? production_id : -production_id;
    int has_proto_cost = 0;
    int signed_facility_id = -1;
    if (is_vehicle) {
        veh_cost(item_id, BaseIDCurrentSelected, &has_proto_cost);
    } else {
        signed_facility_id = item_id;
    }
    int display_prototype = item_id;
    const int panel_center_x = (content.left + content.right) / 2;
    int text_y = content.top + 0x28;

    MainInterfaceGlobal.set_clip(&content);
    if (is_vehicle) {
        const int render_width = ((content.right - content.left) * 3) / 4;
        const int render_height = (render_width * 75) / 100;
        display_prototype = veh_fake(item_id, BaseCurrent->faction_id_current);
        if (item_id < 0x40 &&
            Weapon[VehPrototypes[item_id].weapon_id].offense_rating < 0) {
            Vehs[display_prototype].morale = static_cast<uint8_t>(
                breed_mod(BaseIDCurrentSelected,
                          BaseCurrent->faction_id_current) + 1);
        }
        MapWinCaptionScale = render_width;
        MapWinCaptionDivisor = 0x64;
        const int drawn = veh_draw(&MainInterfaceGlobal, display_prototype,
                                   panel_center_x - render_width / 2,
                                   text_y - render_height / 2, 1, 0xCC, 0);
        if (drawn != 0) {
            caviar_.reset_objects();
            vehdraw_construct_vehicle(display_prototype, &caviar_);
            caviar_.set_scaling(static_cast<float>(render_width) *
                                0.000699999975040555f);
            caviar_.set_scene_rotation(
                static_cast<float>(field_1E670_ + 0x1e) *
                    0.017444444820284843f,
                0.0f, 0.0f);
            caviar_.render(&MainInterfaceGlobal, panel_center_x, text_y, 0);
        }
        MapWinCaptionScale = -1;
        MapWinCaptionDivisor = -1;
        content.top += 0x41;
        StringTemp[0] = 0;
        strcat(StringTemp, VehPrototypes[item_id].veh_name);
        CharUpperA(StringTemp);
        MainInterfaceGlobal.set_text_color(TextColourTable[82],
                                           TextColourTable[83], 1, 1);
        MainInterfaceGlobal.set_font(&detail_font, 0, 0, 0);
        content.top = MainInterfaceGlobal.wrap_cent(StringTemp, &content);
        if (has_proto_cost != 0) {
            MainInterfaceGlobal.set_text_color(TextColourTable[118],
                                               TextColourTable[119], 1, 1);
            MainInterfaceGlobal.set_font(&unit_font, 0, 0, 0);
            StringTemp[0] = 0;
            strcat(StringTemp, label_get(185));
            strcat(StringTemp, " ");
            strcat(StringTemp, "+");
            _itoa(prototype_factor(item_id), number, 10);
            strcat(StringTemp, number);
            strcat(StringTemp, "%");
            content.top = MainInterfaceGlobal.wrap_cent(StringTemp,
                panel_center_x, text_y, 0);
        }
        content.right -= 2;
        content.top += 2;
        content.left += 2;
        content.bottom -= 2;
        content.bottom -= 2 * unit_font.line_advance();
    } else {
        int icon_color;
        Sprite *icon_sprite;
        if (item_id < 0x46) {
            icon_color = mandate_color(prodPicker_.edbc_fac(item_id));
            icon_sprite = &g_FACILITY_ICON_SPRITES[item_id];
        } else {
            icon_color = mandate_color(1);
            icon_sprite = &g_SECRET_PROJECT_ICON_SPRITES[item_id - FacilitySPStart];
        }
        icon_sprite->draw_mono(&MainInterfaceGlobal,
            icon_sprite->transparent_index(), panel_center_x - 0x14,
            text_y - 0x14, icon_color, 0x28,
            (item_id < 0x46 ? g_FACILITY_ICON_SPRITES[0].width()
                            : g_SECRET_PROJECT_ICON_SPRITES[0].width()));
        content.top += 0x41;
        StringTemp[0] = 0;
        strcat(StringTemp, StringTable.get(
            Facility[item_id].name));
        CharUpperA(StringTemp);
        MainInterfaceGlobal.set_text_color(TextColourTable[116],
                                           TextColourTable[117], 1, 1);
        content.top = MainInterfaceGlobal.wrap_cent(&detail_font, StringTemp,
                                                    &content);
        content.left += 2;
        content.top += 2;
        content.right -= 2;
        content.bottom -= 2;
        content.bottom -= 2 * unit_font.line_advance();
    }

    // Reload live economy state after the rendering/text callbacks, matching
    // the native reads at 0x40E9A5/0x40EAD6; cost_factor may run only after
    // the mineral value has been captured.
    Base *economy_base = BaseCurrent;
    const int current_minerals = economy_base->minerals_accumulated;
    int item_turn_cost = cost_factor(economy_base->faction_id_current, 1, -1);
    int item_cost = is_vehicle
        ? veh_cost(item_id, BaseIDCurrentSelected, &has_proto_cost)
        : Facility[item_id].cost;

    // Native local values at [ebp-0x30] and [ebp-0x20] are the stored
    // mineral count and total production cost. Keep them local: neither is a
    // BaseWin member. The native dimension search and row-major cell walk are
    // retained, including its unguarded integer divisions and signed limits.
    const int gross_cost = item_cost * item_turn_cost;
    if (gross_cost == 0) {
        if (update_mode != 0) MainInterfaceGlobal.update(&panel, 0);
        else MainInterfaceGlobal.soft_update(&panel);
        MainInterfaceGlobal.set_clip(&saved_clip);
        return;
    }
    const int mineral_surplus = BaseCurrent->mineral_surplus;
    const int max_cell_index = gross_cost > current_minerals
        ? gross_cost : current_minerals;
    const int progress_boundary = gross_cost < current_minerals
        ? gross_cost : current_minerals;
    {
        const int available_width = content.right - content.left;
        const int available_height = content.bottom - content.top;
        int columns = item_turn_cost;
        if (2 * columns < available_width) {
            while ((max_cell_index / columns) * 2 > available_height)
                columns *= 2;
        } else {
            columns = 0;
            while (columns * columns < gross_cost) ++columns;
            columns += 2;
        }

        int rows = max_cell_index / columns;
        if (rows * columns < max_cell_index) ++rows;
        const int cell_width = available_width / columns;
        const int cell_height = available_height / rows;
        int start_x = content.left + (available_width - cell_width * columns) / 2;
        int start_y = content.top + (available_height - cell_height * rows) / 2;
        RECT cell;
        int cell_index = 0;

        for (int row = 0; row < rows; ++row) {
            for (int column = 0; column < columns; ++column, ++cell_index) {
                cell.left = start_x + column * cell_width;
                cell.top = start_y + row * cell_height;
                cell.right = cell.left + cell_width - 1;
                cell.bottom = cell.top + cell_height - 1;
                if (cell_height >= 8) {
                    ++cell.left;
                    ++cell.top;
                    --cell.right;
                    --cell.bottom;
                } else {
                    --cell.right;
                }
                if (cell_index < progress_boundary && cell_height >= 8) {
                    MainInterfaceGlobal.tile(&sprites2_[1], cell.left,
                        cell.top, cell.left, cell.top,
                        cell.right - cell.left, cell.bottom - cell.top);
                } else if (cell_height < 8) {
                    const int color = cell_index < progress_boundary
                        ? 0x42
                        : (current_minerals >= gross_cost ? 0x6C : 0xDF);
                    MainInterfaceGlobal.fill(&cell, color);
                } else if (current_minerals < gross_cost) {
                    MainInterfaceGlobal.tile(&sprites2_[0], cell.left,
                        cell.top, cell.left, cell.top,
                        cell.right - cell.left, cell.bottom - cell.top);
                } else {
                    MainInterfaceGlobal.tile(&sprite5_, cell.left,
                        cell.top, cell.left, cell.top,
                        cell.right - cell.left, cell.bottom - cell.top);
                }

                if (current_minerals < gross_cost && mineral_surplus < 0 &&
                    cell_index >= progress_boundary + mineral_surplus &&
                    cell_index < progress_boundary) {
                    MainInterfaceGlobal.box(&cell, 0x6C, 0x6C);
                } else if (current_minerals < gross_cost && mineral_surplus > 0 &&
                           cell_index + 1 > progress_boundary &&
                           cell_index + 1 <= progress_boundary + mineral_surplus) {
                    MainInterfaceGlobal.box(&cell, 0x42, 0x42);
                }

                if (cell_index >= max_cell_index) {
                    ++cell_index;
                    break;
                }
            }
            if (cell_index > max_cell_index) break;
        }
    }

    int project_is_owned = 0;
    if (signed_facility_id != -1 && signed_facility_id < 0x41) {
        int bit_offset;
        int bit_mask;
        bitmask(signed_facility_id, &bit_offset, &bit_mask);
        project_is_owned =
            (BaseCurrent->facilities_built[bit_offset] & bit_mask) != 0;
    } else if (signed_facility_id >= 0x46) {
        project_is_owned =
            base_project(signed_facility_id - FacilitySPStart) == BaseIDCurrentSelected;
    }
    if (project_is_owned) {
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(489));
        MainInterfaceGlobal.set_text_color(TextColourTable[120],
                                           TextColourTable[121], 1, 1);
        MainInterfaceGlobal.write_cent_l(&detail_font, StringTemp, &content,
                                         strlen(StringTemp));
    } else if (BaseCurrent->mineral_surplus > 0) {
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(49));
        CharUpperA(StringTemp);
        strcat(StringTemp, ": ");
        if (max_cell_index > current_minerals) {
            const int remaining = max_cell_index - current_minerals;
            int divisor = BaseCurrent->mineral_surplus;
            if (divisor < 1) divisor = 1;
            if (divisor > 0x1869F) divisor = 0x1869F;
            int turns = (BaseCurrent->mineral_surplus + remaining - 1) / divisor;
            if (turns < 0) turns = 0;
            if (turns > 0xF423F) turns = 0xF423F;
            _itoa(turns, number, 10);
            strcat(StringTemp, number);
        } else {
            _itoa(0, number, 10);
            strcat(StringTemp, number);
        }
        MainInterfaceGlobal.set_text_color(TextColourTable[104],
                                           TextColourTable[105], 1, 1);
        content.top = content.bottom;
        content.bottom += 2 * unit_font.pixel_height();
        MainInterfaceGlobal.write_cent_l(&unit_font, StringTemp, &content,
                                         strlen(StringTemp));
    } else {
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(67));
        MainInterfaceGlobal.set_text_color(TextColourTable[104],
                                           TextColourTable[105], 1, 1);
        content.top = content.bottom;
        content.bottom += 2 * unit_font.pixel_height();
        MainInterfaceGlobal.write_cent_l(&unit_font, StringTemp, &content,
                                         strlen(StringTemp));
    }

    if (update_mode != 0) MainInterfaceGlobal.update(&panel, 0);
    else MainInterfaceGlobal.soft_update(&panel);
    MainInterfaceGlobal.set_clip(&saved_clip);
}

// ORIGINAL: 0x00416DE0 ?add_queue@BaseWin@@QAEXHH@Z 0x00416DE0-0x00416FA8 FILE
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00416DE0-0x00416FA8 queue index/count and production-id accesses", "source:src/base.h:Base::queue_size", "source:src/base.h:Base::queue_production_id", "source:src/basewin.h:BaseWin queue selection fields"], "reason": "Apply the original queue insert/replace, sentinel removal and redraw sequence through typed Base queue storage; drawing remains an explicit unfinished dependency."}
void BaseWin::add_queue(int insert, int production_id) {
    int index = static_cast<int>(field_40B2C_);
    if (index == 0 || index == 9) {
        return;
    }
    if (insert == 0 && BaseCurrent->queue_production_id[index] == production_id
        && BaseCurrent->queue_size >= static_cast<uint32_t>(index)) {
        return;
    }

    if (production_id < 0
        && (-production_id < 0x41 || (production_id != -0x44 && -production_id > 0x43))) {
        int slot = 1;
        while (slot < 9) {
            if (BaseCurrent->queue_production_id[slot] == production_id) {
                if (slot < index) {
                    field_40B2C_ = static_cast<uint32_t>(index - 1);
                }
                --BaseCurrent->queue_size;
                if (slot < 8) {
                    int offset = slot;
                    do {
                        BaseCurrent->queue_production_id[offset] =
                            BaseCurrent->queue_production_id[offset + 1];
                        ++offset;
                    } while (offset < 8);
                }
                break;
            }
            ++slot;
        }
    }

    if (insert != 0) {
        int end = 9;
        if (field_40B2C_ < 9) {
            do {
                BaseCurrent->queue_production_id[end] =
                    BaseCurrent->queue_production_id[end - 1];
                --end;
            } while (field_40B2C_ < static_cast<uint32_t>(end));
        }
        if (BaseCurrent->queue_size < 9) {
            ++BaseCurrent->queue_size;
        }
    }

    index = static_cast<int>(field_40B2C_);
    BaseCurrent->queue_production_id[index] = production_id;
    if (BaseCurrent->queue_size < static_cast<uint32_t>(index)) {
        ++BaseCurrent->queue_size;
    }

    int visited = 0;
    if (insert != 0) {
        const int count = static_cast<int>(BaseCurrent->queue_size);
        field_40B30_ = 0;
        field_40B2C_ = static_cast<uint32_t>(count + 1);
    }

    int count = static_cast<int>(BaseCurrent->queue_size);
    if (count + 1 > 0) {
        int slot = 0;
        do {
            if (BaseCurrent->queue_production_id[slot] == -0x45 && visited < count) {
                for (int following = slot; following < 9; ++following) {
                    BaseCurrent->queue_production_id[following] =
                        BaseCurrent->queue_production_id[following + 1];
                }
                --BaseCurrent->queue_size;
                count = static_cast<int>(BaseCurrent->queue_size);
                if (count < static_cast<int>(field_40B2C_)) {
                    if (count < 2) {
                        count = 1;
                    }
                    field_40B2C_ = static_cast<uint32_t>(count);
                }
                draw_queue(1);
            }
            draw_production(1);
            ++visited;
            ++slot;
            count = static_cast<int>(BaseCurrent->queue_size);
        } while (visited < count + 1);
    }
    draw_queue(1);
}

// ORIGINAL: 0x00416FB0 ?delete_queue@BaseWin@@QAEXH@Z 0x00416FB0-0x0041703C FILE
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00416FB0-0x0041703C removes one queue entry, shifts later ids, clamps the selected index to at least one, and redraws the queue", "source:src/base.h:Base::queue_size", "source:src/base.h:Base::queue_production_id"], "reason": "Delete a selected production queue entry through the typed Base queue representation; the drawing dependency remains explicitly unfinished."}
void BaseWin::delete_queue(int index) {
    if (index > static_cast<int>(BaseCurrent->queue_size)) {
        int selection = static_cast<int>(field_40B2C_) - 1;
        if (selection <= 1) {
            selection = 1;
        }
        field_40B2C_ = static_cast<uint32_t>(selection);
        return;
    }
    if (index < 9) {
        for (int slot = index; slot < 9; ++slot) {
            BaseCurrent->queue_production_id[slot] =
                BaseCurrent->queue_production_id[slot + 1];
        }
    }
    --BaseCurrent->queue_size;
    const int count = static_cast<int>(BaseCurrent->queue_size);
    if (count < static_cast<int>(field_40B2C_)) {
        field_40B2C_ = static_cast<uint32_t>(count <= 1 ? 1 : count);
    }
    draw_queue(1);
}

// Refresh only a visible base screen displaying the affected base. The
// recomputation uses the already-selected current base: neither original
// refresh hook calls set_base. Its first redraw is virtual; the subsequent
// main-interface redraw explicitly calls the GraphicWin implementation.
// ORIGINAL: 0x0041A7A0 ?check_base@BaseWin@@QAEXH@Z 0x0041A7A0-0x0041A7DF BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original41A7A6 calls Win::is_visible before reading focusedBaseID_+40B0C.41A7BA rejects unequal base-id bits; the remaining chain is base_compute(1), virtual Win slot62 on this, then direct GraphicWin::redraw on MainInterfaceGlobal7AE820. No set_base or additional guard occurs.", "Existing source base_compute4EC3B0 has the observed void interface; the seed return-int declaration and manual VCall table are not the product model. Seed claim and full original span remain restoration obligations. Real downstream base-energy and BaseWin painting recovery remain separate."], "reason": "Recover the visible selected-base refresh through real BaseWin state and C++ dispatch, replacing the vehicle callers original-address seam."}
void BaseWin::check_base(int base_id) {
    if (Win::is_visible() && base_id == focusedBaseID_) {
        base_compute(1);
        redraw();
        MainInterfaceGlobal.GraphicWin::redraw();
    }
}

// Visibility gates every base-coordinate read. Original 0x0041A80A captures
// the signed y coordinate before computing wrapped x distance. This uses
// the game's existing hex metric, not square-grid or Euclidean distance.
// ORIGINAL: 0x0041A7E0 ?check_loc@BaseWin@@QAEXHHH@Z 0x0041A7E0-0x0041A8A7
// RECOVERY_BINDING: v1 {"evidence": ["Original41A7F3 uses focusedBaseID_+40B0C and Bases stride134;41A80A/11 sign extend y/x before distance calculation. Win visibility gates every coordinate access. MapIsFlat bit0, MapLongitude half-width and MapLongitudeBounds width govern the wrapped x difference.", "Original41A81B..41A87D is x_dist followed by abs(y-base_y) and the two-distance vector_dist metric;41A882 accepts distance equal to radius. The refresh chain forces base_compute once, virtual this redraw, then direct main-interface GraphicWin redraw, without selecting another current base. Existing helpers retain the finite valid-coordinate domain; no new malformed-index or overflow policy is introduced."], "reason": "Recover the visible base screen proximity refresh using typed Base coordinates and existing cylindrical-map hex distance helpers."}
void BaseWin::check_loc(int x, int y, int radius) {
    if (Win::is_visible()) {
        const int base_y = Bases[focusedBaseID_].y;
        const int base_x = Bases[focusedBaseID_].x;
        const int dx = x_dist(x, base_x);
        const int dy = abs(y - base_y);
        if (vector_dist(dx, dy) <= radius) {
            base_compute(1);
            redraw();
            MainInterfaceGlobal.GraphicWin::redraw();
        }
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

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0041A920 ?zoom@BaseWin@@QAEXHH@Z 0x0041A920-0x0041AE40
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/0041a920.cpp", "image:0x0041A920-0x0041AE40 sets base focus, recomputes, refreshes the interface, then waits on zoom_wait_pending_", "source:src/basewin.h:BaseWin::zoom_wait_pending_", "source:src/worldwin.cpp:WorldWin::click cancels the same zoom wait"], "reason": "Recover the base selection and display lifecycle through owned objects and typed state. The tutorial-message branches in the retained seed are still pending; preserve that seed and do not claim a byte match."}
void BaseWin::zoom(int base_id, int wait) {
    field_40B00_ = -1;
    TutorialWindow.reset();
    focusedBaseID_ = base_id;
    BaseComputedID = -1;

    const int worked_tiles = Bases[focusedBaseID_].resource_sqr_radius;
    set_base(focusedBaseID_);
    base_compute(0);
    if (IsMultiplayerNet && !ConsoleControlTurnActive
        && worked_tiles != Bases[focusedBaseID_].resource_sqr_radius
        && NetDaemonNet.lock_base(BaseIDCurrentSelected, 0, -1, -1) == 0) {
        NetDaemonNet.unlock_base(BaseIDCurrentSelected);
    }

    BaseCurrent->state &= ~BSTATE_UNK_10;
    zoom_wait_pending_ = 1;
    field_40B10_ = 0;
    field_40B14_ = 0;
    field_40B18_ = 1;
    displayed_production_id_ = BaseCurrent->queue_production_id[0];

    g_FX.play(15);
    ambience(30);
    if (!Win::is_visible()) {
        show(0);
    } else {
        redraw();
    }
    MainInterfaceGlobal.GraphicWin::redraw();
    field_1E670_ = 0;

    // The original runs several tutorial-message branches here. Their
    // rectangles, messages, and faction-state ownership still need recovery.
    if (wait != 0) {
        while (zoom_wait_pending_ && Win::is_visible() && !ExitTurnLoop) {
            if (IsMultiplayerNet) {
                NetDaemonNet.net_tasks();
            } else {
                wait_task();
            }
        }
    }
}

// ORIGINAL: 0x00415DD0 ?focus@BaseWin@@QAEXXZ 0x00415DD0-0x00415E52 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00415DD0-0x00415E52 reads focusedBaseID_ and Base::resource_sqr_radius, selects and computes that base, then conditionally acquires/releases the multiplayer base lock after observing a changed radius.", "source:src/basewin.h", "source:src/base.h", "source:src/game.h", "source:src/console.h", "source:src/netdaemon.h"], "reason": "Recover BaseWin focus through its source-owned focused base, typed Base fields and NetDaemon lock interface while retaining the original refresh and lock sequence."}
void BaseWin::focus() {
    const int base_id = static_cast<int>(focusedBaseID_);
    const int worked_tiles = Bases[base_id].resource_sqr_radius;
    set_base(base_id);
    base_compute(0);
    if (IsMultiplayerNet != 0 && ConsoleControlTurnActive == 0
        && worked_tiles != Bases[focusedBaseID_].resource_sqr_radius) {
        if (NetDaemonNet.lock_base(BaseIDCurrentSelected, 0, -1, -1) == 0) {
            NetDaemonNet.unlock_base(BaseIDCurrentSelected);
        }
    }
}

// ORIGINAL: 0x00415E60 ?UNK3@BaseWin@@QAEXH@Z 0x00415E60-0x00415EFB BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00415E60-0x00415EFB stores its argument in BaseWin+0x40B0C, invalidates BaseComputedID at 0x00689374, compares Base::resource_sqr_radius before and after set_base/base_compute, and conditionally locks/unlocks the selected multiplayer base.", "source:src/basewin.h", "source:src/base.h", "source:src/base.cpp", "source:src/game.h", "source:src/console.h", "source:src/netdaemon.h"], "reason": "Recover the focused-base selection entry through typed BaseWin/Base state and the existing NetDaemon API, preserving its cache invalidation and conditional lock behavior."}
// SEED LEVER: legacy table[i*77] is Bases[i].resource_sqr_radius at +0x78
// in the 0x134-byte Base record. focusedBaseID_ is at +0x40B0C; the earlier
// scaffold's pad-derived field name was based on the wrong +0x40B10 offset.
void BaseWin::UNK3(int base_id) {
    focusedBaseID_ = static_cast<uint32_t>(base_id);
    BaseComputedID = -1;
    const int worked_tiles = Bases[focusedBaseID_].resource_sqr_radius;
    set_base(static_cast<int>(focusedBaseID_));
    base_compute(0);
    if (IsMultiplayerNet != 0 && ConsoleControlTurnActive == 0
        && worked_tiles != Bases[focusedBaseID_].resource_sqr_radius) {
        if (NetDaemonNet.lock_base(BaseIDCurrentSelected, 0, -1, -1) == 0) {
            NetDaemonNet.unlock_base(BaseIDCurrentSelected);
        }
    }
}

// ORIGINAL: 0x0041A8B0 ?check_veh@BaseWin@@QAEXH@Z 0x0041A8B0-0x0041A91D
// RECOVERY_BINDING: v1 {"evidence": ["image:0x0041A8B0-0x0041A91D multiplies the vehicle index by 0x34 and focusedBaseID_ by 0x134 before reading coordinates/home_base_id.", "source:src/veh.h", "source:src/base.h", "source:src/basewin.h", "source:src/graphicwin.h"], "reason": "Refresh the visible base when a vehicle is physically at the focused base or lists it as home, using the established Veh/Base records and virtual GraphicWin redraw."}
// The recovered seed's `int *` indexed expressions accidentally scaled those
// byte strides by four. Direct typed array indexing preserves the disassembly's
// 0x34 Veh and 0x134 Base strides without an intermediate pointer.
void BaseWin::check_veh(int veh_id) {
    if (!is_visible()) {
        return;
    }

    const Veh &vehicle = Vehs[veh_id];
    const Base &base = Bases[focusedBaseID_];
    if ((vehicle.x == base.x && vehicle.y == base.y) ||
        vehicle.home_base_id == static_cast<int16_t>(focusedBaseID_)) {
        base_compute(1);
        redraw();
    }
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
// symbol    ?on_left_click@BaseWin@@UAEXHH@Z
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
// symbol    ?on_right_click@BaseWin@@UAEXHH@Z
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
// symbol    ?timer_callback@BaseWin@@SAXHPAX@Z
// size      1 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BaseWin::timer_callback(int, void *) {
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
// RECOVERY_BINDING: v1 {"evidence": ["pinned original own table 0x0066A40C, pointer word at +0xC0, identifies original entry 0x0041B4A0", "source:src/win.h inherited virtual interface; actual emitted method ?vslot_48@BaseWin@@UAEXHH@Z in primary Win table ??_7BaseWin@@6BWin@@@"], "reason": "The original own Win table binds this entry as virtual slot 48. Use genuine member dispatch, not an opaque redirect; retain the catalogued original spelling as historical evidence. The previous on_scrolled definition is the class slot-48 target; renaming connects the unchanged body to its real virtual slot."}
// symbol    ?on_scrolled@BaseWin@@UAEXHH@Z
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
// The process-wide base window, 0x006A7628 in the image - REAL STORAGE, not
// the `Win *const TutWinBaseWindow` binding that named terranx.exe data
// (see tutwin.h). The image's dynamic initializer at 0x004083B0 runs the
// real BaseWin constructor against it and registers the destructor
// with atexit; here the same recovered constructor runs through this
// build's own startup and the destructor closes it at exit.
// RECOVERY_OBJECT: v1 0x006A7628 ?TutWinBaseWindow@@3VBaseWin@@A
BaseWin TutWinBaseWindow;  // 0x006A7628
// Original CRT slots 0x00682060/0x00682064 construct BaseWin before Wave;
// their registered destructors run in reverse order. Keep the real definitions
// together in that order rather than spelling compiler-owned callbacks.
// RECOVERY_OBJECT: v1 0x006EEE68 ?g_BASEWIN_WAVE@@3VWave@@A
Wave g_BASEWIN_WAVE;  // 0x006EEE68
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
    Buffer *canvas = static_cast<Buffer *>(this);
    Font font;

    const int faction_id = BaseCurrent->faction_id_current;
    const int wide_mode = WinScreenWidth != 0x320;
    RECT clip_target = production_content_rect_;
    RECT saved_clip = canvas->current_clip();
    canvas->set_clip(&clip_target);
    font.init(DefaultFontFace, 0xC, 1);
    canvas->set_font(&font, 0, 0, 0);

    RECT outer_frame = *static_cast<GraphicWin *>(this)->buffer_working_rect();
    outer_frame.left = psych_display_add(outer_frame.left, 0x17);
    outer_frame.right = psych_display_sub(outer_frame.right, 0x37);
    canvas->box_sprite(&outer_frame, &InterfaceBoxStyles[9]);
    canvas->box_sprite(&production_content_rect_, &InterfaceBoxStyles[3]);

    int frame_delta = 0;
    if (wide_mode) {
        RECT working_frame = *static_cast<GraphicWin *>(this)->buffer_working_rect();
        frame_delta = psych_display_add(
            psych_display_sub(working_frame.bottom, working_frame.top), 3);
    }
    RECT chart_frame = {0x15A, psych_display_add(0x11C, frame_delta),
                        0x2B8, psych_display_add(0x15A, frame_delta)};
    canvas->box_sprite(&chart_frame, &InterfaceBoxStyles[0]);

    int row_delta = 0;
    if (wide_mode) {
        RECT working_rows = *static_cast<GraphicWin *>(this)->buffer_working_rect();
        row_delta = psych_display_add(
            psych_display_sub(working_rows.bottom, working_rows.top), 3);
    }
    RECT row_bounds = {0xA9, psych_display_add(0x107, row_delta),
                       0x2B8, psych_display_add(0x11B, row_delta)};
    canvas->set_font(&font, 0, 0, 0);

    const int psych_percent = PlayersData[faction_id].se_alloc_psych;
    const int labs_percent = PlayersData[faction_id].se_alloc_labs;
    const int economy_percent = psych_display_sub(
        psych_display_sub(10, psych_percent), labs_percent);

    int energy = BaseCurrent->energy_surplus;
    if (energy < 0) energy = 0;
    else if (energy > 0x270F) energy = 0x270F;

    const int psych_product = psych_display_signed(
        static_cast<uint32_t>(psych_percent) * static_cast<uint32_t>(energy) + 4u);
    int psych_share = psych_product / 10;
    if (psych_share < 0 || energy < 0) psych_share = 0;
    else if (psych_share > energy) psych_share = energy;

    const int after_psych = psych_display_sub(energy, psych_share);
    const int economy_product = psych_display_signed(
        static_cast<uint32_t>(economy_percent) * static_cast<uint32_t>(energy) + 4u);
    int economy_share = economy_product / 10;
    if (economy_share < 0 || after_psych < 0) economy_share = 0;
    else if (economy_share > after_psych) economy_share = after_psych;

    const int labs_share = psych_display_sub(after_psych, economy_share);

    Base *totals_base = BaseCurrent;
    const int economy_total = totals_base->economy_total;
    const int economy_residual = psych_display_sub(economy_total, economy_share);
    const int labs_total = totals_base->labs_total;
    const int labs_residual = psych_display_sub(labs_total, labs_share);
    const int psych_total = totals_base->psych_total;
    const int psych_residual = psych_display_sub(psych_total, psych_share);

    RECT title_rect = {wide_mode ? psych_display_add(production_content_rect_.left, 3) : 0xBF,
                       row_bounds.top, row_bounds.right, row_bounds.bottom};
    canvas->box_sprite(&title_rect, &InterfaceBoxStyles[6]);
    title_rect.left = psych_display_add(title_rect.left, 3);
    title_rect.top = psych_display_add(title_rect.top, 3);
    title_rect.right = psych_display_sub(title_rect.right, 3);
    title_rect.bottom = psych_display_sub(title_rect.bottom, 3);
    canvas->set_text_color(TextColourTable[86], TextColourTable[87], 1, 1);
    StringTemp[0] = '\0';
    strcat(StringTemp, label_get(0x90 / 4));
    strcat(StringTemp, " ");
    strcat(StringTemp, label_get(0xA38 / 4));
    CharUpperA(StringTemp);
    if (wide_mode == 0)
        canvas->write_l(StringTemp, &title_rect, strlen(StringTemp));
    else
        canvas->write_cent_l(StringTemp, &title_rect, strlen(StringTemp));

    RECT hit_rect = {row_bounds.left, psych_display_add(row_bounds.top, 0x15),
                     row_bounds.right, psych_display_add(row_bounds.bottom, 0x15)};
    expenses_help_rects_[5].left = hit_rect.left;
    expenses_help_rects_[5].top = hit_rect.top;
    expenses_help_rects_[5].right = hit_rect.right;
    expenses_help_rects_[5].bottom = hit_rect.bottom;
    canvas->set_text_color(TextColourTable[128], TextColourTable[129], 1, 1);
    RECT graph_rect = {hit_rect.left, hit_rect.top, 0x129, hit_rect.bottom};
    canvas->box_sprite(&graph_rect, &InterfaceBoxStyles[6]);
    graph_rect.left = psych_display_add(graph_rect.left, 3);
    graph_rect.top = psych_display_add(graph_rect.top, 3);
    graph_rect.right = psych_display_sub(graph_rect.right, 3);
    graph_rect.bottom = psych_display_sub(graph_rect.bottom, 3);
    StringTemp[0] = '\0';
    strcat(StringTemp, label_get(0x5B8 / 4));
    CharUpperA(StringTemp);
    canvas->write_cent_l(StringTemp, &graph_rect, strlen(StringTemp));

    RECT value_cell = {0x12B, hit_rect.top, 0x158, hit_rect.bottom};
    canvas->box_sprite(&value_cell, &InterfaceBoxStyles[6]);
    value_cell.right = 0x150;
    char number[16];
    StringTemp[0] = '\0';
    _itoa(psych_display_signed(static_cast<uint32_t>(economy_percent) * 10u),
          number, 10);
    strcat(StringTemp, number);
    strcat(StringTemp, "%");
    canvas->write_right_l(StringTemp, &value_cell, strlen(StringTemp));
    value_cell.right = 0x178;
    StringTemp[0] = '\0';
    _itoa(economy_share, number, 10);
    strcat(StringTemp, number);
    canvas->write_right_l(StringTemp, &value_cell, strlen(StringTemp));

    RECT label_value = {0x178, hit_rect.top, hit_rect.right, hit_rect.bottom};
    StringTemp[0] = '\0';
    strcat(StringTemp, " ");
    strcat(StringTemp, label_get(0x90 / 4));
    CharUpperA(StringTemp);
    strcat(StringTemp, "  +");
    canvas->set_text_color(TextColourTable[84], TextColourTable[85], 1, 1);
    const int economy_label_end = canvas->write_l(
        StringTemp, &label_value, strlen(StringTemp));
    label_value.left = economy_label_end;
    value_cell.right = psych_display_add(economy_label_end, 0x1A);

    canvas->set_text_color(TextColourTable[128], TextColourTable[129], 1, 1);
    StringTemp[0] = '\0';
    _itoa(economy_residual, number, 10);
    strcat(StringTemp, number);
    Base *marker_base = BaseCurrent;
    if (marker_base->queue_production_id[0] == -69
        && marker_base->mineral_surplus > 0)
        strcat(StringTemp, "*");
    canvas->write_right_l(StringTemp, &value_cell, strlen(StringTemp));

    canvas->set_text_color(TextColourTable[84], TextColourTable[85], 1, 1);
    StringTemp[0] = '\0';
    strcat(StringTemp, " ");
    strcat(StringTemp, label_get(0x17C / 4));
    CharUpperA(StringTemp);
    strcat(StringTemp, " ");
    strcat(StringTemp, "=");
    const int economy_total_label_end = canvas->write_l(
        StringTemp, &label_value, strlen(StringTemp));
    label_value.left = economy_total_label_end;
    value_cell.right = psych_display_add(economy_total_label_end, 0x1D);
    canvas->set_text_color(TextColourTable[128], TextColourTable[129], 1, 1);
    StringTemp[0] = '\0';
    _itoa(economy_total, number, 10);
    strcat(StringTemp, number);
    canvas->write_right_l(StringTemp, &value_cell, strlen(StringTemp));

    hit_rect.left = row_bounds.left;
    hit_rect.top = psych_display_add(row_bounds.top, 0x2A);
    hit_rect.right = row_bounds.right;
    hit_rect.bottom = psych_display_add(row_bounds.bottom, 0x2A);
    expenses_help_rects_[4].left = hit_rect.left;
    expenses_help_rects_[4].top = hit_rect.top;
    expenses_help_rects_[4].right = hit_rect.right;
    expenses_help_rects_[4].bottom = hit_rect.bottom;
    canvas->set_text_color(TextColourTable[130], TextColourTable[131], 1, 1);
    graph_rect = hit_rect;
    graph_rect.right = 0x129;
    canvas->box_sprite(&graph_rect, &InterfaceBoxStyles[6]);
    graph_rect.left = psych_display_add(graph_rect.left, 3);
    graph_rect.top = psych_display_add(graph_rect.top, 3);
    graph_rect.right = psych_display_sub(graph_rect.right, 3);
    graph_rect.bottom = psych_display_sub(graph_rect.bottom, 3);
    StringTemp[0] = '\0';
    strcat(StringTemp, label_get(0xF0 / 4));
    CharUpperA(StringTemp);
    canvas->write_cent_l(StringTemp, &graph_rect, strlen(StringTemp));

    value_cell.left = 0x12B;
    value_cell.top = hit_rect.top;
    value_cell.right = 0x158;
    value_cell.bottom = hit_rect.bottom;
    canvas->box_sprite(&value_cell, &InterfaceBoxStyles[6]);
    value_cell.right = 0x150;
    StringTemp[0] = '\0';
    _itoa(psych_display_signed(static_cast<uint32_t>(labs_percent) * 10u),
          number, 10);
    strcat(StringTemp, number);
    strcat(StringTemp, "%");
    canvas->write_right_l(StringTemp, &value_cell, strlen(StringTemp));
    value_cell.right = 0x178;
    StringTemp[0] = '\0';
    _itoa(labs_share, number, 10);
    strcat(StringTemp, number);
    canvas->write_right_l(StringTemp, &value_cell, strlen(StringTemp));

    label_value.left = 0x178;
    label_value.top = hit_rect.top;
    label_value.right = hit_rect.right;
    label_value.bottom = hit_rect.bottom;
    StringTemp[0] = '\0';
    strcat(StringTemp, " ");
    strcat(StringTemp, label_get(0x90 / 4));
    CharUpperA(StringTemp);
    strcat(StringTemp, "  +");
    canvas->set_text_color(TextColourTable[84], TextColourTable[85], 1, 1);
    const int labs_label_end = canvas->write_l(
        StringTemp, &label_value, strlen(StringTemp));
    label_value.left = labs_label_end;
    value_cell.right = psych_display_add(labs_label_end, 0x1A);

    canvas->set_text_color(TextColourTable[130], TextColourTable[131], 1, 1);
    StringTemp[0] = '\0';
    _itoa(labs_residual, number, 10);
    strcat(StringTemp, number);
    canvas->write_right_l(StringTemp, &value_cell, strlen(StringTemp));

    canvas->set_text_color(TextColourTable[84], TextColourTable[85], 1, 1);
    StringTemp[0] = '\0';
    strcat(StringTemp, " ");
    strcat(StringTemp, label_get(0x17C / 4));
    CharUpperA(StringTemp);
    strcat(StringTemp, " ");
    strcat(StringTemp, "=");
    const int saved_top_label_end = canvas->write_l(
        StringTemp, &label_value, strlen(StringTemp));
    label_value.left = saved_top_label_end;
    value_cell.right = psych_display_add(saved_top_label_end, 0x1D);
    canvas->set_text_color(TextColourTable[130], TextColourTable[131], 1, 1);
    StringTemp[0] = '\0';
    _itoa(saved_clip.top, number, 10);
    strcat(StringTemp, number);
    canvas->write_right_l(StringTemp, &value_cell, strlen(StringTemp));

    hit_rect.left = row_bounds.left;
    hit_rect.top = psych_display_add(row_bounds.top, 0x3F);
    hit_rect.right = row_bounds.right;
    hit_rect.bottom = psych_display_add(row_bounds.bottom, 0x3F);
    expenses_help_rects_[3].left = hit_rect.left;
    expenses_help_rects_[3].top = hit_rect.top;
    expenses_help_rects_[3].right = hit_rect.right;
    expenses_help_rects_[3].bottom = hit_rect.bottom;
    canvas->set_text_color(TextColourTable[132], TextColourTable[133], 1, 1);
    graph_rect = hit_rect;
    graph_rect.right = 0x129;
    canvas->box_sprite(&graph_rect, &InterfaceBoxStyles[6]);
    graph_rect.left = psych_display_add(graph_rect.left, 3);
    graph_rect.top = psych_display_add(graph_rect.top, 3);
    graph_rect.right = psych_display_sub(graph_rect.right, 3);
    graph_rect.bottom = psych_display_sub(graph_rect.bottom, 3);
    StringTemp[0] = '\0';
    strcat(StringTemp, label_get(0x4F8 / 4));
    CharUpperA(StringTemp);
    canvas->write_cent_l(StringTemp, &graph_rect, strlen(StringTemp));

    value_cell.left = 0x12B;
    value_cell.top = hit_rect.top;
    value_cell.right = 0x158;
    value_cell.bottom = hit_rect.bottom;
    canvas->box_sprite(&value_cell, &InterfaceBoxStyles[6]);
    value_cell.right = 0x150;
    StringTemp[0] = '\0';
    _itoa(psych_display_signed(static_cast<uint32_t>(psych_percent) * 10u),
          number, 10);
    strcat(StringTemp, number);
    strcat(StringTemp, "%");
    canvas->write_right_l(StringTemp, &value_cell, strlen(StringTemp));
    value_cell.right = 0x178;
    StringTemp[0] = '\0';
    _itoa(psych_share, number, 10);
    strcat(StringTemp, number);
    canvas->write_right_l(StringTemp, &value_cell, strlen(StringTemp));

    label_value.left = 0x178;
    label_value.top = hit_rect.top;
    label_value.right = hit_rect.right;
    label_value.bottom = hit_rect.bottom;
    StringTemp[0] = '\0';
    strcat(StringTemp, " ");
    strcat(StringTemp, label_get(0x90 / 4));
    CharUpperA(StringTemp);
    strcat(StringTemp, "  +");
    canvas->set_text_color(TextColourTable[84], TextColourTable[85], 1, 1);
    const int psych_label_end = canvas->write_l(
        StringTemp, &label_value, strlen(StringTemp));
    label_value.left = psych_label_end;
    value_cell.right = psych_display_add(psych_label_end, 0x1A);

    canvas->set_text_color(TextColourTable[132], TextColourTable[133], 1, 1);
    StringTemp[0] = '\0';
    _itoa(psych_residual, number, 10);
    strcat(StringTemp, number);
    canvas->write_right_l(StringTemp, &value_cell, strlen(StringTemp));

    canvas->set_text_color(TextColourTable[84], TextColourTable[85], 1, 1);
    StringTemp[0] = '\0';
    strcat(StringTemp, " ");
    strcat(StringTemp, label_get(0x17C / 4));
    CharUpperA(StringTemp);
    strcat(StringTemp, " ");
    strcat(StringTemp, "=");
    const int psych_total_label_end = canvas->write_l(
        StringTemp, &label_value, strlen(StringTemp));
    label_value.left = psych_total_label_end;
    value_cell.right = psych_display_add(psych_total_label_end, 0x1D);
    canvas->set_text_color(TextColourTable[132], TextColourTable[133], 1, 1);
    StringTemp[0] = '\0';
    _itoa(psych_total, number, 10);
    strcat(StringTemp, number);
    canvas->write_right_l(StringTemp, &value_cell, strlen(StringTemp));

    if (a1 != 0)
        static_cast<GraphicWin *>(this)->update(&production_content_rect_, 0);
    else
        static_cast<GraphicWin *>(this)->soft_update(&production_content_rect_);
    canvas->set_clip(&saved_clip);
}


// Garrison frames use style3. Original 0x410485 loads style9's mask pointer
// from 0x78D6B0; that address is a member slot, not a Buffer object itself.
// ORIGINAL: 0x004103B0 ?draw_garrison@BaseWin@@QAEXH@Z 0x004103B0-0x00410868;0x00651B0D-0x00651B22 FILE
// RECOVERY_BINDING: v1 {"evidence": ["image:0x004104DD-0x004107F9 starts from the veh_at result, counts from the previous-link head, invokes overflow buttons, then walks previous links again before rendering; no scroll offset is loaded", "image:0x00410573-0x004105E5 controls at 0x007B5080/5BCC/6718 are MainInterfaceGlobal.flat_button_[8..10] by 0xB4C stride", "image:0x004105A7-0x004105CD overflow bounds use adjacent MainInterface rectangles", "image:0x004104E2-0x004104FB and 0x0041074A-0x00410768 fall back only for nonpositive scale and otherwise divide by divisor", "image:0x0041056A-0x00410571 divides by spacing+5 after counting a present stack; overflow 0x004105BD-0x004105C0 divides by capacity without a guard", "image:0x0041074A-0x00410768 rereads pitch after text-color calls for write width; 0x004107FF-0x00410846 clips to Buffer::rect2_, updates, then restores saved clip", "src/veh.h:Vehs and veh_at", "src/veh.cpp:say_stats_3 and veh_draw", "src/basewin.h:BaseWin garrison_rect_ and spot1_", "src/maininterface.h:MainInterfaceGlobal"], "reason": "Preserve the pinned stack traversal, callback-visible rereads, unchecked signed divisions, Font/clip/update ordering and source-owned receivers. The original body has no independent scroll-offset read."}
// size      1208 bytes
// prototype void (__thiscall ?draw_garrison@BaseWin@@QAEXH@Z)(BaseWin* this, int)
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// The body walks Vehs' typed stack links from the head and renders at most the
// computed capacity. The pinned body does not load a scroll-position field.
// LEVER: 0x40C2C is one RECT, not the four unnamed dwords the IDB records -
//  set_clip takes it whole and the tile/frame sizing reads its fields.
//  0x00691B2C is interface_fonts.h's DefaultFontFace, passed as the pointer it is; the
//  artifact's `*DefaultFontFace` through a pointer-to-pointer is two loads
//  where the image has one.
// 0x004104E6-0x004104F9 scales the column pitch by 100, not 20 as the
// earlier partial transcription did. The stack starts at veh_at(base.x,y),
// climbs prev_veh_id_stack and then walks next_veh_id_stack.
void BaseWin::draw_garrison(int update_mode) {
    Font font;
    Buffer *canvas = static_cast<Buffer *>(&MainInterfaceGlobal);

    RECT saved_clip = canvas->current_clip();
    RECT clip_rect = garrison_rect_;
    canvas->set_clip(&clip_rect);

    font.init(DefaultFontFace, 0xc, 0);

    // The row pitch every garrison line uses: `height_ + unk_1_` when the
    // font's own ascent adjustment is not negative, `line_height_` when it is.
    int line_height = font.line_advance();

    int left = garrison_rect_.left;
    int top = garrison_rect_.top;
    int right = garrison_rect_.right;
    int width = right - left;
    int bottom = garrison_rect_.bottom;
    int height = bottom - top;
    canvas->tile(InterfaceBoxStyles[9].mask_, left, top,
                 left, top, width, height);

    RECT frame_rect;
    frame_rect.left = left;
    frame_rect.top = top;
    frame_rect.right = right;
    frame_rect.bottom = bottom;
    canvas->box_sprite(&frame_rect,
                       &InterfaceBoxStyles[3]);

    frame_rect.left += 3;
    frame_rect.top += 3;
    frame_rect.right -= 3;
    frame_rect.bottom -= 3;

    // Type 6 is the garrison's own highlight spot; the panel clears it before
    // it lays the row spots out again.
    spot1_.kill_type(6);

    int spacing;
    if (MapWinCaptionScale > 0) {
        spacing = (MapWinCaptionScale * 100) / MapWinCaptionDivisor;
    } else {
        spacing = 0x38;
    }

    int initial_vehicle = veh_at(BaseCurrent->x, BaseCurrent->y);
    int vehicle = initial_vehicle;
    int capacity;
    if (vehicle >= 0) {
        while (Vehs[vehicle].prev_veh_id_stack >= 0) {
            vehicle = Vehs[vehicle].prev_veh_id_stack;
        }
        int stack_count = 0;
        for (int at = vehicle; at >= 0; at = Vehs[at].next_veh_id_stack) {
            ++stack_count;
        }
        // Original 0x410571 divides without guarding zero/negative pitch.
        // An absent stack skips this division entirely (0x41051C).
        capacity = (frame_rect.right - frame_rect.left) / (spacing + 5);
        MainInterfaceGlobal.flat_button_[8].hide();
        if (stack_count <= capacity) {
            MainInterfaceGlobal.flat_button_[9].hide();
            MainInterfaceGlobal.flat_button_[10].hide();
        } else {
            frame_rect.left = MainInterfaceGlobal.adjacent_panel_left_rect_.right + 1;
            frame_rect.right = MainInterfaceGlobal.adjacent_panel_right_rect_.left - 1;
            spacing = (frame_rect.right - frame_rect.left) / capacity - 5;
            MainInterfaceGlobal.flat_button_[9].show(0);
            MainInterfaceGlobal.flat_button_[10].show(0);
        }
    }

    // Keep the veh_at result across the button callbacks, then find the head
    // again: the original rereads previous links at 0x4105F6..0x410626.
    vehicle = initial_vehicle;
    if (vehicle >= 0) {
        while (Vehs[vehicle].prev_veh_id_stack >= 0) {
            vehicle = Vehs[vehicle].prev_veh_id_stack;
        }
    }

    int column = 0;
    int column_left = frame_rect.left;
    while (vehicle >= 0 && column < capacity) {
        canvas->set_clip(column_left, frame_rect.top, spacing,
                         frame_rect.bottom - frame_rect.top);
        veh_draw(canvas, vehicle, column_left, frame_rect.top, -7, 72, 0);
        veh_draw(canvas, vehicle, column_left, frame_rect.top, 0, 0x1800, 0);

        StringTemp[0] = 0;
        say_stats_3(StringTemp, Vehs[vehicle].proto_id);
        if ((GameDataStateStorage.fields_.game_state_ & 0x1040) == 0x1040) {
            char number[80];
            strcat(StringTemp, " ");
            strcat(StringTemp, "(");
            _itoa(vehicle, number, 10);
            strcat(StringTemp, number);
            strcat(StringTemp, ")");
        }
        canvas->set_font(&font, nullptr, nullptr, nullptr);
        canvas->set_text_color(TextColourTable[112], TextColourTable[113], 1, 1);
        // Text width is recomputed after the calls above, independently of
        // overflow-adjusted column spacing (0x41074A..0x41079D).
        int text_spacing;
        if (MapWinCaptionScale > 0) {
            text_spacing = (MapWinCaptionScale * 100) / MapWinCaptionDivisor;
        } else {
            text_spacing = 0x38;
        }
        canvas->write_cent_l(StringTemp, column_left,
                             frame_rect.bottom - line_height + 6, text_spacing,
                             strlen(StringTemp));
        spot1_.add(vehicle, 6, column_left, frame_rect.top,
                   spacing + 5, frame_rect.bottom - frame_rect.top);
        column_left += spacing + 5;
        vehicle = Vehs[vehicle].next_veh_id_stack;
        ++column;
    }

    canvas->restore_clip();
    if (update_mode != 0) MainInterfaceGlobal.update(&garrison_rect_, nullptr);
    else MainInterfaceGlobal.soft_update(&garrison_rect_);
    canvas->set_clip(&saved_clip);
}

// ORIGINAL: 0x0041AFD0 ?on_left_up@BaseWin@@QAEXHH@Z 0x0041AFD0-0x0041B10D FILE BYTE_EXACT
// symbol    ?on_left_up@BaseWin@@UAEXHH@Z
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
            // Original 0x007AEC94 is MainInterfaceGlobal + 0x474:
            // its inherited Buffer::rect2_, not separate clip storage.
            const RECT &interface_clip = MainInterfaceGlobal.Buffer::clip_rect();
            rect.left = interface_clip.left;
            rect.top = interface_clip.top;
            rect.right = interface_clip.right;
            rect.bottom = interface_clip.bottom;

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
            // slot `on_left_click_event`, so the dispatch spells by name.
            on_left_click(xCoord, yCoord);
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

// ORIGINAL: 0x0041B110 ?on_left_down@BaseWin@@QAEXHH@Z 0x0041B110-0x0041B1D9 FILE BYTE_EXACT
// symbol    ?on_left_down@BaseWin@@UAEXHH@Z
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
    if (is_visible() != 0 && (IsMultiplayerNet == 0 || NetDaemonNet.lock_state_[0].leading_words_[3] == 0)) {
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
    client_to_screen(&x, &y);
    RECT rect = { x, y, x, y };
    set_bubble_text(text, &rect);
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

    if (is_visible() != 0 && (IsMultiplayerNet == 0 || NetDaemonNet.lock_state_[0].leading_words_[3] == 0)) {
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
// symbol    ?on_iface_button_clicked@BaseWin@@UAEXH@Z
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
// The genuine interface name shares slot13 across owners. BattleWin's direct
// call to MainInterface uses qualification, so it does not require an empty
// differently named slot or a false nonvirtual MainInterface callback.
void BaseWin::on_iface_button_clicked(int button_id) {
    on_button_clicked(button_id);
}

// HANDOFF (basewin, added by the win.h slot-44/45 rename, 2026-09-06): Win's
//        slot 44 is spelled `on_button_clicked`, so this implementation is
//        the front table's override as well as the mapped original body.
// symbol    ?on_button_clicked@BaseWin@@UAEXH@Z
//           Q->U 2026-09-06, Win's slot 44 taking the image's name: the body
//           is now an override of Win's slot, so VC6 mangles it virtual and
//           enters it on the front table, while the image spells the same
//           body Q and its own table 0x66A4BC holds the raw address.
// ORIGINAL: 0x0041D500 ?on_button_clicked@BaseWin@@QAEXH@Z 0x0041D500-0x0041DA6C
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/recovered/units/0041d500.cpp", "source:src/basewin.h", "image:original instruction and dispatch evidence for the BaseWin owner"], "reason": "Recover this original base-window operation through the real BaseWin owner, existing typed members and source-owned shared dependencies; catalogue naming and original intervals remain distinct from current compiler symbols."}
// symbol    ?on_button_clicked@BaseWin@@UAEXH@Z
void BaseWin::on_button_clicked(int button_id) {
    compute_selected_base();
    TutorialWindow.reset();
    if (IfaceIdToSlotStorage[ActiveIfaceIdStorage] != 3) return;
    ok_callback();
    if (button_id == 1009 || button_id == 1010) {
        int vehicle = veh_at(BaseCurrent->x, BaseCurrent->y);
        if (vehicle < 0) return;
        if (button_id == 1009) {
            // Original41D9FE/41DA15 follows nonnegative links. The retained
            // seed had reversed this condition, indexing a negative id.
            while (Vehs[vehicle].next_veh_id_stack >= 0)
                vehicle = Vehs[vehicle].next_veh_id_stack;
            veh_promote(vehicle);
        } else veh_demote(vehicle);
        redraw();
        return;
    }
    const unsigned selection = static_cast<unsigned>(button_id) + 2u;
    if (selection > 23) return;
    // Original's 24-entry table at41DA6C is not the ordinal switch the
    // retained seed printed. Two close entries and two governor entries
    // share their bodies; the no-op entries are deliberately unhandled.
    switch (selection) {
    case 22:
        field_40B2C_ = BaseCurrent->queue_size + 1;
        if (static_cast<int>(field_40B2C_) >= 10) field_40B2C_ = 9;
        draw_queue(1);
        if (!prodPicker_.is_visible()) production(field_40B2C_, 0);
        return;
    case 0:
    case 2: {
        const int visible = TutWinBaseWindow.is_visible();
        zoom_wait_pending_ = 0;
        time_.stop();
        if (TutWinBaseWindow.is_visible()) TutWinBaseWindow.release_iface_mode();
        do_all_draws();
        if (visible && ConsoleGlobal.cursor_active_ && ConsoleGlobal.selected_vehicle_ >= 0
            && !ConsoleGlobal.tile_in_view(Vehs[ConsoleGlobal.selected_vehicle_].x,
                Vehs[ConsoleGlobal.selected_vehicle_].y, 2)) ConsoleGlobal.center();
        return;
    }
    case 3: rename(); return;
    case 4: prev(); return;
    case 5: next(); return;
    case 14: production(0, 0); return;
    case 15: hurry(); return;
    case 10:
        if (BaseCurrent->faction_id_current != ConsoleGlobal.faction_id_
            && !(GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)) return;
        if (BaseCurrent->governor & 0x80000000u) {
            BaseCurrent->governor &= 0x7FFFFFFFu;
            redraw();
            MainInterfaceGlobal.redraw();
        } else gov_on(0);
        return;
    case 20:
    case 21:
        if (BaseCurrent->faction_id_current == ConsoleGlobal.faction_id_
            || (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)) governor(0);
        return;
    case 6:
    case 7:
    case 8:
    case 9:
        if (BaseCurrent->faction_id_current == ConsoleGlobal.faction_id_
            || (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW))
            gov_on(1 << (24 + selection - 6));
        return;
    case 11:
    case 12:
    case 13: {
        const int mode = selection == 11 ? 2 : (selection == 12 ? 0 : 1);
        if (static_cast<int>(field_40AFC_) != mode) {
            field_40AFC_ = mode;
            redraw();
        }
        return;
    }
    case 19: {
        const bool selected = flatButtons_[17].selected_state() != 0;
        StringTemp[0] = 0;
        strcat(StringTemp, label_get((selected ? 0xA4C : 0xA50) / 4));
        CharUpperA(StringTemp);
        flatButtons_[17].set_name(StringTemp);
        flatButtons_[17].set(selected ? 0 : 1);
        draw_expenses(1);
        return;
    }
    case 18:
        if (BaseCurrent->faction_id_current == ConsoleGlobal.faction_id_
            || (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)) staple_the_bastards();
        return;
    case 23:
        if (BaseCurrent->faction_id_current == ConsoleGlobal.faction_id_
            || (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)) base_editor();
        return;
    }
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
// GraphicWin's Buffer base is at +0x444; its DIB biWidth is at +0x80,
// explaining the original +0x4C4 read. Keep the qualified Win dispatch.
int BaseWin::on_nc_hittest(int a1, int a2) {
    if (a1 >= 0x17 &&
        a1 <= pixel_width() - 0x37) {
        return Win::on_nc_hittest(a1, a2);
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
// Independent count and eight native32-byte rows end at the separately
// constructed BaseWin singleton; original unchecked parser limits remain.
// RECOVERY_OBJECT: v2 0x006EEE60 0x006EEE64 ?InvertEntryCount@@3HA 0x00408410
int InvertEntryCount;
// RECOVERY_OBJECT: v2 0x006A7528 0x006A7628 ?InvertEntries@@3PAY0CA@DA 0x0041E8D0
char InvertEntries[8][32];

void __cdecl load_inverts() {
    if (text_open("BASENAME", "INVERTLIST") == 0) {
        text_get();
        int count = text_item_number();
        InvertEntryCount = count;
        if (count > 0) {
            char *dest = InvertEntries[0];
            int i = 0;
            do {
                text_get();
                strcpy(dest, TextBufferGetPtr);
                count = InvertEntryCount;
                i++;
                dest += 0x20;
            } while (i < count);
        }
        text_close();
    }
}


// ORIGINAL: 0x0041DD10 ?on_iface_left_up@BaseWin@@QAEXHH@Z 0x0041DD10-0x0041DE5B FILE BYTE_EXACT
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
//        `on_left_click_event(...)` / `on_lose_mouse_capture()` the front already spelled, and the
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
//        casts are the front's own `on_left_click_event(...)` / `on_lose_mouse_capture()`, entry slot
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

// Original 0x006A7A9C is TutWinBaseWindow (0x006A7628) + 0x474:
// GraphicWin's Buffer subobject at +0x444 and its rect2_ at +0x30.
// It is not BaseWin::rect2_ at +0x40BAC or an independent drag rectangle.
// The two pointer slots are the already-owned WinPointerOwner3/4 globals;
// original 0x0041DE30 clears owner 3 before notifying capture loss, and
// 0x0041DE41 reloads owner 4 after that callback.

void BaseWin::on_iface_left_up(int xCoord, int yCoord) {
    // front + 0x474: the drawing Buffer's own rect2_, not this class's
    // rect2_ at 0x40BAC - hence the qualification.
    RECT local_rect = Buffer::rect2_;

    if (field_40B54_ > 0) {
        if (in_box(xCoord, yCoord, &local_rect) == 0) {
            MainInterfaceGlobal.client_to_screen(&xCoord, &yCoord);
            TutWinBaseWindow.screen_to_client(&xCoord, &yCoord);

            // Retain four assignments to the existing local. Original
            // 0x0041DD88..A8 loads left/top/right before storing left, then
            // loads bottom and stores top/right/bottom. Historical spelling
            // trials above are not a proof of the current compiler schedule.
            const RECT &tutorial_clip = TutWinBaseWindow.Buffer::clip_rect();
            local_rect.left = tutorial_clip.left;
            local_rect.top = tutorial_clip.top;
            local_rect.right = tutorial_clip.right;
            local_rect.bottom = tutorial_clip.bottom;

            if (in_box(xCoord, yCoord, &local_rect) == 0) {
                set_cursor(0x7f00);
                MainInterfaceGlobal.set_cursor(0x7f00);
                field_40B54_ = -1;
                field_40B4C_ = -1;
                field_40B50_ = -1;
            } else {
                // Front vtable slot 19 (0x4C) - the front's on_left_click.
                on_left_click(xCoord, yCoord);
            }
        } else {
            // This table's own slot 3, dispatched with the entry untouched -
            // `mov edx,[esi] / call [edx+0xc]`.
            on_iface_left_click(xCoord, yCoord);
        }
    }

    // Slots hold the primary Win base, not the SubInterface receiver.
    // Original 0x0041DE26 adjusts the latter by -0xA14 before comparing;
    // the declared hierarchy owns that adjustment, not a raw pointer cast.
    Win *const self = static_cast<Win *>(this);
    if (WinPointerOwner3 == self) {
        WinPointerOwner3 = 0;
        // Front vtable slot 4 (0x10) - Win's on_lose_mouse_capture, the shared no-op.
        on_lose_mouse_capture();
    }
    if (WinPointerOwner4 == self) {
        WinPointerOwner4 = 0;
    }
}


/*
Purpose: Switch BaseWin panel visibility and audio state for an interface
         mode change using the owner's actual members and window lifecycle.
         Original receiver evidence: entry copies `this` into EDI, and the
         front GraphicWin subobject is addressed as `[EDI - 0xA14]`.
// symbol    ?on_status@BaseWin@@UAEXH@Z
// size      431 bytes
// prototype void (__thiscall ?on_status@BaseWin@@QAEXH@Z)(BaseWin* this, int)
// callers   1   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00415B20 0x00415080 0x0041E560 0x0040D250 0x004C3A40
*/
// ORIGINAL: 0x0041DAD0 ?on_status@BaseWin@@QAEXH@Z 0x0041DAD0-0x0041DC7F BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/recovered/units/0041dad0.cpp", "source:src/basewin.h", "image:original instruction and dispatch evidence for the BaseWin owner"], "reason": "Recover this original base-window operation through the real BaseWin owner, existing typed members and source-owned shared dependencies; catalogue naming and original intervals remain distinct from current compiler symbols."}
// symbol    ?on_status@BaseWin@@UAEXH@Z
void BaseWin::on_status(int mode) {
    if (mode) {
        WorldClimateWorldWin.hide_all();
        on_redraw();
        for (int i = 0; i < 22; ++i) {
            if (i == 16 && (Players[Bases[focusedBaseID_].faction_id_current].rule_flags & 0x80)) {
                flatButton_.show(4);
            } else if (i != 21 || (GameDataStateStorage.fields_.game_state_ & 0x40)) {
                flatButtons_[i].show(4);
            }
        }
        MainInterfaceGlobal.update(0);
        field_40B54_ = -1;
        if ((GameDataStateStorage.fields_.game_preferences_ & 0x400) && !g_BASEWIN_WAVE.is_playing())
            g_BASEWIN_WAVE.play();
        ambience(0xCD);
        do_all_draws();
        slide_show(this, 0);
        Win::show(3);
        if (GameDataStateStorage.fields_.game_preferences_ & 0x400) g_BASEWIN_WAVE.fade();
    } else {
        for (int j = 0; j < 22; ++j) {
            if (j == 16) flatButton_.hide();
            flatButtons_[j].hide();
        }
        scroll2_.hide();
        ambience(0xCE);
        g_MENU_DOWN_WAVE.play();
        slide_hide(this, 0);
        Win::hide();
        do_all_draws();
    }
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
Purpose: Handle pointer movement over BaseWin, including queue hover, drag
         previews, citizen cursors, production hover and multiplayer updates.
         Original receiver evidence: at `0x0041DF5B`, the body computes the
         front subobject with `lea ebx, [esi - 0xA14]`.
// symbol    ?on_iface_mouse_move@BaseWin@@UAEXHH@Z
// size      1164 bytes
// prototype void (__thiscall ?on_iface_mouse_move@BaseWin@@QAEXHH@Z)(BaseWin* this, int, int)
// callers   0   call targets   9
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005F7E90 0x005F8500 0x005FAB00 0x005EC7C0 0x004E39D0
*/
// ORIGINAL: 0x0041DF50 ?on_iface_mouse_move@BaseWin@@QAEXHH@Z 0x0041DF50-0x0041E3DC
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/recovered/units/0041df50.cpp", "source:src/basewin.h", "image:original instruction and dispatch evidence for the BaseWin owner"], "reason": "Recover this original base-window operation through the real BaseWin owner, existing typed members and source-owned shared dependencies; catalogue naming and original intervals remain distinct from current compiler symbols."}
// symbol    ?on_iface_mouse_move@BaseWin@@UAEXHH@Z
void BaseWin::on_iface_mouse_move(int a1, int a2) {
    if (!is_visible() || (IsMultiplayerNet && NetDaemonNet.lock_state_[0].leading_words_[3])) return;
    clear_bubble_text();
    clear_bubble_text();
    int position, type;
    const int found = spot1_.check(a1, a2, &position, &type);
    if (found == -1 || type != 8) {
        if (field_40B30_) {
            field_40B30_ = 0;
            draw_queue(1);
        }
    } else {
        field_40B30_ = position - 2;
        draw_queue(1);
    }
    if (prodPicker_.is_visible()) return;
    if (found == -1) {
        if (field_40B00_ < 0) return;
        field_40B00_ = -1;
        field_40B04_ = 0;
        compute_selected_base();
        draw_production(1);
        return;
    }
    if (field_40B54_ >= 0) {
        update_drag_cursor(type, position, a1, a2, 1);
        return;
    }
    if (type == 6 || type == 7) {
        if (field_40B00_ == position) return;
        field_40B00_ = position;
        field_40B04_ = type == 7;
        compute_selected_base();
        draw_flash(1);
        return;
    }
    if (field_40B00_ < 0) return;
    field_40B00_ = -1;
    field_40B04_ = 0;
    compute_selected_base();
    draw_production(1);
}

/*
Purpose: Clear the queue hover and restore the base-production selection
         when the pointer leaves the BaseWin interface.
// ORIGINAL: 0x0041E3E0 ?on_iface_mouse_leave@BaseWin@@QAEXHH@Z 0x0041E3E0-0x0041E4C0 FILE
// RECOVERY_BINDING: v1 {"evidence": ["image:0x0041E3E0-0x0041E4C0 clears the bubble twice, conditionally clears queue hover, resets fields 0x400EC/0x400F0, recomputes focused base, and invokes the multiplayer radius lock path", "source:src/basewin.h:BaseWin focused/queue fields", "source:src/base.h:Base records", "source:src/win.h:Win::clear_bubble_text", "source:src/netdaemon.h"], "reason": "Recover the mouse-leave state transition with typed BaseWin/Base state and existing Win, base-computation, NetDaemon and panel interfaces; panel rendering remains explicitly unfinished."}
// symbol    ?on_iface_mouse_leave@BaseWin@@UAEXHH@Z
// TRIED: two `char*` bases (self, self-0xa14) computed via arithmetic instead of matching the single-esi-reuse register shape
*/
void BaseWin::on_iface_mouse_leave(int, int) {
    Win::clear_bubble_text();
    Win::clear_bubble_text();

    if (field_40B30_ != 0) {
        field_40B30_ = 0;
        draw_queue(0);
    }

    if (field_40B00_ >= 0) {
        field_40B00_ = -1;
        field_40B04_ = 0;

        const int worked_tiles = Bases[focusedBaseID_].resource_sqr_radius;
        set_base(static_cast<int>(focusedBaseID_));
        base_compute(0);

        if (IsMultiplayerNet != 0 && ConsoleControlTurnActive == 0
            && worked_tiles != Bases[focusedBaseID_].resource_sqr_radius) {
            if (NetDaemonNet.lock_base(BaseIDCurrentSelected, 0, -1, -1) == 0) {
                NetDaemonNet.unlock_base(BaseIDCurrentSelected);
            }
        }

        draw_production(1);
    }
}

// The original redraws the production selection buttons and every base-screen
// pane while interface mode 3 is active. The three receivers are successive
// FlatButtons in this object's established array at 0x13D6C, 0x148B8 and
// 0x15404; the seed's raw receiver arithmetic is unnecessary.
// ORIGINAL: 0x0041E790 ?on_redraw@BaseWin@@QAEXXZ 0x0041E790-0x0041E885 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/0041e790.cpp", "src/basewin.h"], "reason": "Recover the BaseWin redraw call sequence through its owned FlatButton array, panel methods and source-owned interface mode globals; panel implementations remain independent obligations."}
// TRIED: caching `[this+0x40afc]` into one local reused for all three BaseButton::set checks (mnemonic similarity 0.945, 8 edit blocks) vs. re-reading it through a `volatile int *` for each check (0.959, 6 edit blocks) - the original re-reads the field three separate times (0x0041E7B9, 0x0041E7D2, 0x0041E7EC), so the volatile re-read is the more faithful form and scores closer. Neither reached BYTE_EXACT; first divergence is prologue/register scheduling around the button-set sequence.
// symbol    ?on_redraw@BaseWin@@UAEXXZ
void BaseWin::on_redraw() {
    if (IfaceIdToSlotStorage[ActiveIfaceIdStorage] != 3) return;

    MainInterfaceGlobal.restore_back(1);
    update_gov_buttons();
    flatButtons_[10].set(field_40AFC_ == 0);
    flatButtons_[11].set(field_40AFC_ == 1);
    flatButtons_[9].set(field_40AFC_ == 2);
    draw_production(0);
    draw_expenses(0);
    draw_queue(0);
    draw_info(0);
    draw_misc(0);
    draw_farm(0);
    draw_pop(0);
    draw_facilities(0);
    draw_commerce(0);
    draw_nutrients(0);
    draw_supported(0);
    draw_garrison(0);
    GraphicWin::soft_update();
    MainInterfaceGlobal.soft_update();
}

// The redraw callback reaches these separately owned panels. Their historical
// seed bodies are retained; these link-only bodies do not pretend to paint.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0041E560 ?update_gov_buttons@BaseWin@@QAEXXZ 0x0041E560-0x0041E788
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0041e560.cpp", "src/basewin.h", "src/base.h", "src/faction.h", "src/basebutton.h"], "reason": "Recover governor-priority button state through BaseCurrent, typed PlayerData AI preferences, the BaseWin FlatButton array and BaseButton's owned auxiliary state. The retained seed remains the measurement reference."}
// The seed's four 0x0096CDC0..0x0096CDCC indexed words are
// PlayersData[faction].ai_growth/ai_tech/ai_wealth/ai_power: each index step
// is 2099 dwords, the established 0x20CC-byte PlayerData stride.
void BaseWin::update_gov_buttons() {
    const Base &base = *BaseCurrent;
    const int faction = base.faction_id_current;
    const unsigned int governor = base.governor;
    const bool human = (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1u << faction)) != 0;

    flatButtons_[8].set((governor & GOV_ACTIVE) != 0);

    const bool explore_priority = human
        ? (governor & (GOV_ACTIVE | GOV_PRIORITY_EXPLORE)) ==
              (GOV_ACTIVE | GOV_PRIORITY_EXPLORE)
        : PlayersData[faction].ai_growth != 0;
    flatButtons_[4].set(explore_priority);
    if (!explore_priority) flatButtons_[4].clear_auxiliary_state_for_disabled_button();

    const bool discover_priority = human
        ? (governor & (GOV_ACTIVE | GOV_PRIORITY_DISCOVER)) ==
              (GOV_ACTIVE | GOV_PRIORITY_DISCOVER)
        : PlayersData[faction].ai_tech != 0;
    flatButtons_[5].set(discover_priority);
    if (!discover_priority) flatButtons_[5].clear_auxiliary_state_for_disabled_button();

    const bool build_priority = human
        ? (governor & (GOV_ACTIVE | GOV_PRIORITY_BUILD)) ==
              (GOV_ACTIVE | GOV_PRIORITY_BUILD)
        : PlayersData[faction].ai_wealth != 0;
    flatButtons_[6].set(build_priority);
    if (!build_priority) flatButtons_[6].clear_auxiliary_state_for_disabled_button();

    const bool conquer_priority = human
        ? (governor & (GOV_ACTIVE | GOV_PRIORITY_CONQUER)) ==
              (GOV_ACTIVE | GOV_PRIORITY_CONQUER)
        : PlayersData[faction].ai_power != 0;
    flatButtons_[7].set(conquer_priority);
    if (!conquer_priority) flatButtons_[7].clear_auxiliary_state_for_disabled_button();
}
// ORIGINAL: 0x00412AA0 ?draw_expenses@BaseWin@@QAEXH@Z 0x00412AA0-0x00414B38;0x00651B61-0x00651B76
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00412AA0; original spans remain load-bearing.", "Retired seed provenance git:1432fed4:src/unrecovered/00412aa0.cpp; historical transcription conflicts remain evidence, not runtime targets.", "image:41E8D0 owns panel geometry; source:src/basewin.h owns real GraphicWin/Buffer and existing Base/Player/Veh/Rules receivers."], "reason": "Recover the complete BaseWin draw_expenses panel using genuine owned rectangles, automatic Font lifetime and existing application globals, retaining original branch/call order and unchecked input obligations."}
void BaseWin::draw_expenses(int a1) {
    typedef unsigned int uint;

    Buffer *buffer = static_cast<Buffer *>(this);
    Font local_font;
    RECT saved_clip, panel, frame, title, line, tile_rect;

    int param_2 = a1;

    int iVar2;
    int uVar3;
    size_t sVar4;
    int iVar5;
    int iVar6;
    uint uVar7;
    int uVar9;
    char local_f0[80];
    uint local_90;
    int local_8c;
    int local_88;
    uint local_84;
    int local_80;
    int local_7c;
    int local_78;
    int local_74;
    int local_70;
    int local_6c;
    int local_68;
    int local_64;
    int local_60;
    int local_5c;
    int local_58;
    int local_54;
    int local_28;
    int local_14;

  local_90 = (uint)(WinScreenWidth != 800);
  local_8c = 0;
  if (local_90 == 0) {
    if ((flatButtons_[17].selected_state() != 0) && (param_2 < 2)) {
      draw_energy_alloc(param_2);
      return;
    }
  }
  else {
    draw_energy_alloc(param_2);
  }
  local_font.init(DefaultFontFace, 0xc, 1);
  panel.left = production_outer_rect_.left;
  panel.top = production_outer_rect_.top;
  panel.right = production_outer_rect_.right;
  panel.bottom = production_outer_rect_.bottom;
  saved_clip.left = buffer->current_clip().left;
  saved_clip.top = buffer->current_clip().top;
  saved_clip.right = buffer->current_clip().right;
  saved_clip.bottom = buffer->current_clip().bottom;
  buffer->set_clip(&panel);
  frame.left = 0x17;
  frame.top = 0;
  frame.right = 0x2bf;
  frame.bottom = 0x194;
  buffer->box_sprite(&frame, &InterfaceBoxStyles[9]);
  buffer->box_sprite(&production_outer_rect_, &InterfaceBoxStyles[3]);
  frame.left = 0x12a;
  frame.top = 0x107;
  frame.right = 0x2b9;
  frame.bottom = 0x145;
  buffer->box_sprite(&frame, &InterfaceBoxStyles[0]);
  frame.left = production_outer_rect_.left;
  frame.top = production_outer_rect_.top;
  frame.right = production_outer_rect_.right;
  frame.bottom = production_outer_rect_.bottom;
  local_60 = BaseCurrent->energy_inefficiency;
  local_74 = BaseCurrent->energy_surplus;
  local_54 = BaseCurrent->energy_consumption + local_60;
  local_64 = BaseCurrent->nutrient_surplus;
  local_68 = BaseCurrent->nutrient_consumption;
  local_14 = local_68 + BaseCurrent->nutrient_inefficiency;
  local_6c = BaseCurrent->mineral_consumption;
  local_88 = BaseCurrent->mineral_inefficiency + local_6c;
  local_5c = BaseCurrent->mineral_surplus;
  local_70 = local_74;
  if (local_74 < 0) {
    local_70 = -local_74;
  }
  iVar2 = local_5c;
  if (local_5c < 0) {
    iVar2 = -local_5c;
  }
  iVar6 = local_64;
  if (local_64 < 0) {
    iVar6 = -local_64;
  }
  if (iVar6 + local_14 < iVar2 + local_88) {
    local_28 = local_5c;
    if (local_5c < 0) {
      local_28 = -local_5c;
    }
    local_28 = local_28 + local_88;
  }
  else {
    local_28 = local_64;
    if (local_64 < 0) {
      local_28 = -local_64;
    }
    local_28 = local_28 + local_14;
  }
  if (local_28 < local_70 + local_54) {
    local_58 = local_74;
    if (local_74 < 0) {
      local_58 = -local_74;
    }
    local_58 = local_58 + local_54;
  }
  else {
    iVar2 = local_5c;
    if (local_5c < 0) {
      iVar2 = -local_5c;
    }
    iVar6 = local_64;
    if (local_64 < 0) {
      iVar6 = -local_64;
    }
    if (iVar6 + local_14 < local_88 + iVar2) {
      local_58 = local_5c;
      if (local_5c < 0) {
        local_58 = -local_5c;
      }
      local_58 = local_58 + local_88;
    }
    else {
      local_58 = local_64;
      if (local_64 < 0) {
        local_58 = -local_64;
      }
      local_58 = local_58 + local_14;
    }
  }
  {
    // Native 412D5C divides by the maximum span just computed, not the
    // energy total: the retained seed used the wrong local here.
    double ratioF = 194.0 / static_cast<double>(local_58);
    double lowerBoundF = 1.0;
    double upperBoundF = 5.0;
    if (ratioF > upperBoundF) {
      ratioF = upperBoundF;
    } else if (!(ratioF >= lowerBoundF)) {
      ratioF = lowerBoundF;
    }
    iVar2 = static_cast<int>(ratioF);
  }
  line.right = 0x2b8;
  line.bottom = 0x11b;
  local_7c = 0x2b8;
  local_78 = 0x11b;
  local_58 = (0xc2 / iVar2) + 1;
  local_70 = iVar2 * local_58;
  line.top = 0x107;
  local_80 = 0x107;
  line.left = 0xa9;
  local_84 = 0xa9;
  buffer->set_font(&local_font,0,0,0);
  title.bottom = line.bottom;
  title.top = line.top;
  expenses_help_rects_[1].left = line.left;
  expenses_help_rects_[1].top = line.top;
  title.right = 0x128;
  expenses_help_rects_[1].right = line.right;
  expenses_help_rects_[1].bottom = line.bottom;
  if (local_90 == 0) {
    title.left = 0xbf;
  }
  else {
    title.left = production_outer_rect_.left + 3;
  }
  buffer->box_sprite(&title, &InterfaceBoxStyles[6]);
  title.left = title.left + 3;
  title.right = title.right + -3;
  title.bottom = title.bottom + -3;
  title.top = title.top + 3;
  buffer->set_text_color(TextColourTable[90],TextColourTable[91],1,1);
  StringTemp[0] = 0;
  strcat(StringTemp, label_get(520));
  CharUpperA(StringTemp);
  sVar4 = strlen(StringTemp);
  buffer->write_cent_l(StringTemp, &title, sVar4);
  iVar6 = BaseCurrent->nutrient_intake_2;
  line.right = line.right + -3;
  line.top = line.top + 3;
  line.bottom = line.bottom + -3;
  line.left = 0x133;
  StringTemp[0] = 0;
  uVar3 = TextColourTable[110];
  uVar9 = TextColourTable[111];
  if (-1 < iVar6) {
    uVar3 = TextColourTable[90];
    uVar9 = TextColourTable[91];
  }
  buffer->set_text_color(uVar3,uVar9,1,1);
  iVar5 = iVar6;
  if (iVar6 < 0) {
    strcat(StringTemp, "-");
    sVar4 = strlen(StringTemp);
    line.left = buffer->write_l(StringTemp, &line, sVar4);
    StringTemp[0] = 0;
    iVar5 = -iVar6;
  }
  if (iVar5 < 100) {
    if (-1 < iVar6) {
      _itoa(0,local_f0,10);
      strcat(StringTemp, local_f0);
      sVar4 = strlen(StringTemp);
      line.left = buffer->write_l(StringTemp, &line, sVar4);
      StringTemp[0] = 0;
      goto LAB_00413015;
    }
LAB_0041301d:
    iVar5 = -iVar6;
  }
  else {
LAB_00413015:
    iVar5 = iVar6;
    if (iVar6 < 0) goto LAB_0041301d;
  }
  if (iVar5 < 10) {
    _itoa(0,local_f0,10);
    strcat(StringTemp, local_f0);
    sVar4 = strlen(StringTemp);
    line.left = buffer->write_l(StringTemp, &line, sVar4);
    StringTemp[0] = 0;
  }
  if (iVar6 < 0) {
    iVar6 = -iVar6;
  }
  _itoa(iVar6,local_f0,10);
  strcat(StringTemp, local_f0);
  sVar4 = strlen(StringTemp);
  line.left = buffer->write_l(StringTemp, &line, sVar4);
  StringTemp[0] = 0;
  strcat(StringTemp, " - ");
  sVar4 = strlen(StringTemp);
  line.left = buffer->write_l(StringTemp, &line, sVar4);
  iVar6 = local_14;
  StringTemp[0] = 0;
  if (local_14 < 100) {
    _itoa(0,local_f0,10);
    strcat(StringTemp, local_f0);
    sVar4 = strlen(StringTemp);
    line.left = buffer->write_l(StringTemp, &line, sVar4);
  }
  StringTemp[0] = 0;
  if (iVar6 < 10) {
    _itoa(0,local_f0,10);
    strcat(StringTemp, local_f0);
    sVar4 = strlen(StringTemp);
    line.left = buffer->write_l(StringTemp, &line, sVar4);
  }
  StringTemp[0] = 0;
  if (iVar6 < 0) {
    iVar6 = -iVar6;
  }
  _itoa(iVar6,local_f0,10);
  strcat(StringTemp, local_f0);
  strcat(StringTemp, " ");
  strcat(StringTemp, " ");
  sVar4 = strlen(StringTemp);
  tile_rect.left = buffer->write_l(StringTemp, &line, sVar4);
  StringTemp[0] = 0;
  local_28 = 1;
  if (0 < iVar2 + -1) {
    local_28 = iVar2 + -1;
  }
  tile_rect.right = local_28 + tile_rect.left;
  tile_rect.top = ((line.bottom - line.top) - local_font.pixel_height()) / 2 + line.top;
  tile_rect.bottom = tile_rect.top + local_font.pixel_height();
  local_14 = 0;

  line.left = tile_rect.left;
  if (local_64 < 1) {
    if (0 < local_68) {

      local_14 = local_68;
      iVar6 = local_68;
      do {
        buffer->tile(&sprites3_[0], tile_rect.left,tile_rect.top,tile_rect.left,tile_rect.top,tile_rect.right - tile_rect.left,
                     tile_rect.bottom - tile_rect.top);
        tile_rect.left = tile_rect.left + iVar2;
        tile_rect.right = tile_rect.right + iVar2;
        iVar6 = iVar6 + -1;
      } while (iVar6 != 0);
    }
    iVar6 = 0;
    while( true ) {
      iVar5 = local_64;
      if (local_64 < 0) {
        iVar5 = -local_64;
      }
      if (iVar5 <= iVar6) break;
      buffer->tile(&sprite5_, tile_rect.left,tile_rect.top,tile_rect.left,tile_rect.top,tile_rect.right - tile_rect.left,
                   tile_rect.bottom - tile_rect.top);
      tile_rect.left = tile_rect.left + iVar2;
      tile_rect.right = tile_rect.right + iVar2;
      local_14 = local_14 + 1;
      iVar6 = iVar6 + 1;
    }
  }
  else {
    if (0 < local_68) {

      local_14 = local_68;
      iVar6 = local_68;
      do {
        buffer->tile(&sprites3_[0], tile_rect.left,tile_rect.top,tile_rect.left,tile_rect.top,tile_rect.right - tile_rect.left,
                     tile_rect.bottom - tile_rect.top);
        tile_rect.left = tile_rect.left + iVar2;
        tile_rect.right = tile_rect.right + iVar2;
        iVar6 = iVar6 + -1;
      } while (iVar6 != 0);
    }
    iVar6 = (local_58 - local_64) - local_68;
    if (0 < iVar6) {
      local_14 = local_14 + iVar6;
      local_28 = iVar6;
      do {
        this->fill(&tile_rect,0xf0);
        tile_rect.left = tile_rect.left + iVar2;
        tile_rect.right = tile_rect.right + iVar2;
        local_28 = local_28 + -1;
      } while (local_28 != 0);
      local_28 = 0;
    }
    if (0 < local_64) {
      local_28 = local_64;
      local_14 = local_14 + local_64;
      do {
        buffer->tile(&sprites3_[1], tile_rect.left,tile_rect.top,tile_rect.left,tile_rect.top,tile_rect.right - tile_rect.left,
                     tile_rect.bottom - tile_rect.top);
        tile_rect.left = tile_rect.left + iVar2;
        tile_rect.right = tile_rect.right + iVar2;
        local_28 = local_28 + -1;
      } while (local_28 != 0);
      local_28 = 0;
    }
  }
  if (local_14 < local_58) {
    local_28 = local_58 - local_14;
    do {
      this->fill(&tile_rect,0xf0);
      tile_rect.left = tile_rect.left + iVar2;
      tile_rect.right = tile_rect.right + iVar2;
      local_28 = local_28 + -1;
    } while (local_28 != 0);
  }
  line.left = line.left + local_70;
  StringTemp[0] = 0;
  strcat(StringTemp, "  =  ");
  iVar6 = local_64;
  uVar3 = TextColourTable[92];
  uVar9 = TextColourTable[93];
  if (local_64 < 0) {
    uVar3 = TextColourTable[110];
    uVar9 = TextColourTable[111];
  }
  buffer->set_text_color(uVar3,uVar9,1,1);
  if (iVar6 < 0) {
    local_8c = 1;
    strcat(StringTemp, "-");
    sVar4 = strlen(StringTemp);
  }
  else {
    strcat(StringTemp, "+");
    sVar4 = strlen(StringTemp);
  }
  line.left = buffer->write_l(StringTemp, &line, sVar4);
  StringTemp[0] = 0;
  iVar5 = iVar6;
  if (iVar6 < 0) {
    iVar5 = -iVar6;
  }
  if (iVar5 < 100) {
    _itoa(0,local_f0,10);
    strcat(StringTemp, local_f0);
    sVar4 = strlen(StringTemp);
    line.left = buffer->write_l(StringTemp, &line, sVar4);
  }
  StringTemp[0] = 0;
  iVar5 = iVar6;
  if (iVar6 < 0) {
    iVar5 = -iVar6;
  }
  if (iVar5 < 10) {
    _itoa(0,local_f0,10);
    strcat(StringTemp, local_f0);
    sVar4 = strlen(StringTemp);
    line.left = buffer->write_l(StringTemp, &line, sVar4);
  }
  StringTemp[0] = 0;
  if (iVar6 < 0) {
    iVar6 = -iVar6;
  }
  _itoa(iVar6,local_f0,10);
  strcat(StringTemp, local_f0);
  sVar4 = strlen(StringTemp);
  buffer->write_l(StringTemp, &line, sVar4);
  line.right = local_7c;
  local_80 = local_80 + 0x15;
  line.left = local_84;
  expenses_help_rects_[2].left = local_84;
  title.left = local_84;
  expenses_help_rects_[2].top = local_80;
  local_78 = local_78 + 0x15;
  expenses_help_rects_[2].right = local_7c;
  expenses_help_rects_[2].bottom = local_78;
  title.right = 0x128;
  title.top = local_80;
  title.bottom = local_78;
  line.top = local_80;
  line.bottom = local_78;
  buffer->box_sprite(&title, &InterfaceBoxStyles[6]);
  title.left = title.left + 3;
  title.top = title.top + 3;
  title.right = title.right + -3;
  title.bottom = title.bottom + -3;
  buffer->set_text_color(TextColourTable[94],TextColourTable[95],1,1);
  StringTemp[0] = 0;
  strcat(StringTemp, label_get(40));
  CharUpperA(StringTemp);
  sVar4 = strlen(StringTemp);
  buffer->write_cent_l(StringTemp, &title, sVar4);
  iVar6 = BaseCurrent->mineral_intake_2;
  line.right = line.right + -3;
  line.top = line.top + 3;
  line.bottom = line.bottom + -3;
  line.left = 0x133;
  StringTemp[0] = 0;
  uVar3 = TextColourTable[110];
  uVar9 = TextColourTable[111];
  if (-1 < iVar6) {
    uVar3 = TextColourTable[94];
    uVar9 = TextColourTable[95];
  }
  buffer->set_text_color(uVar3,uVar9,1,1);
  iVar5 = iVar6;
  if (iVar6 < 0) {
    strcat(StringTemp, "-");
    sVar4 = strlen(StringTemp);
    line.left = buffer->write_l(StringTemp, &line, sVar4);
    StringTemp[0] = 0;
    iVar5 = -iVar6;
  }
  if (iVar5 < 100) {
    if (-1 < iVar6) {
      _itoa(0,local_f0,10);
      strcat(StringTemp, local_f0);
      sVar4 = strlen(StringTemp);
      line.left = buffer->write_l(StringTemp, &line, sVar4);
      StringTemp[0] = 0;
      goto LAB_00413885;
    }
LAB_0041388d:
    iVar5 = -iVar6;
  }
  else {
LAB_00413885:
    iVar5 = iVar6;
    if (iVar6 < 0) goto LAB_0041388d;
  }
  if (iVar5 < 10) {
    _itoa(0,local_f0,10);
    strcat(StringTemp, local_f0);
    sVar4 = strlen(StringTemp);
    line.left = buffer->write_l(StringTemp, &line, sVar4);
    StringTemp[0] = 0;
  }
  if (iVar6 < 0) {
    iVar6 = -iVar6;
  }
  _itoa(iVar6,local_f0,10);
  strcat(StringTemp, local_f0);
  sVar4 = strlen(StringTemp);
  line.left = buffer->write_l(StringTemp, &line, sVar4);
  StringTemp[0] = 0;
  strcat(StringTemp, " - ");
  local_90 = line.left;
  sVar4 = strlen(StringTemp);
  line.left = buffer->write_l(StringTemp, &line, sVar4);
  iVar6 = local_88;
  StringTemp[0] = 0;
  if (local_88 < 100) {
    _itoa(0,local_f0,10);
    strcat(StringTemp, local_f0);
    sVar4 = strlen(StringTemp);
    line.left = buffer->write_l(StringTemp, &line, sVar4);
  }
  StringTemp[0] = 0;
  if (iVar6 < 10) {
    _itoa(0,local_f0,10);
    strcat(StringTemp, local_f0);
    sVar4 = strlen(StringTemp);
    line.left = buffer->write_l(StringTemp, &line, sVar4);
  }
  StringTemp[0] = 0;
  if (iVar6 < 0) {
    iVar6 = -iVar6;
  }
  _itoa(iVar6,local_f0,10);
  strcat(StringTemp, local_f0);
  strcat(StringTemp, " ");
  strcat(StringTemp, " ");
  buffer->set_text_color(TextColourTable[94],TextColourTable[95],1,1);
  sVar4 = strlen(StringTemp);
  tile_rect.left = buffer->write_l(StringTemp, &line, sVar4);
  StringTemp[0] = 0;
  local_28 = 1;
  if (0 < iVar2 + -1) {
    local_28 = iVar2 + -1;
  }
  tile_rect.right = local_28 + tile_rect.left;
  tile_rect.top = ((line.bottom - line.top) - local_font.pixel_height()) / 2 + line.top;
  tile_rect.bottom = tile_rect.top + local_font.pixel_height();
  local_14 = 0;

  line.left = tile_rect.left;
  if (local_5c < 1) {
    if (local_5c < 0) {
      local_8c = 1;
    }
    if (0 < local_6c) {

      local_14 = local_6c;
      iVar6 = local_6c;
      do {
        buffer->tile(&sprites2_[0], tile_rect.left,tile_rect.top,tile_rect.left,tile_rect.top,tile_rect.right - tile_rect.left,
                     tile_rect.bottom - tile_rect.top);
        tile_rect.left = tile_rect.left + iVar2;
        tile_rect.right = tile_rect.right + iVar2;
        iVar6 = iVar6 + -1;
      } while (iVar6 != 0);
    }
    iVar6 = 0;
    while( true ) {
      iVar5 = local_5c;
      if (local_5c < 0) {
        iVar5 = -local_5c;
      }
      if (iVar5 <= iVar6) break;
      buffer->tile(&sprite5_, tile_rect.left,tile_rect.top,tile_rect.left,tile_rect.top,tile_rect.right - tile_rect.left,
                   tile_rect.bottom - tile_rect.top);
      tile_rect.left = tile_rect.left + iVar2;
      tile_rect.right = tile_rect.right + iVar2;
      local_14 = local_14 + 1;
      iVar6 = iVar6 + 1;
    }
  }
  else {
    if (0 < local_6c) {

      local_14 = local_6c;
      iVar6 = local_6c;
      do {
        buffer->tile(&sprites2_[0], tile_rect.left,tile_rect.top,tile_rect.left,tile_rect.top,tile_rect.right - tile_rect.left,
                     tile_rect.bottom - tile_rect.top);
        tile_rect.left = tile_rect.left + iVar2;
        tile_rect.right = tile_rect.right + iVar2;
        iVar6 = iVar6 + -1;
      } while (iVar6 != 0);
    }
    iVar6 = (local_58 - local_5c) - local_6c;
    if (0 < iVar6) {
      local_14 = local_14 + iVar6;
      local_28 = iVar6;
      do {
        this->fill(&tile_rect,0xf0);
        tile_rect.left = tile_rect.left + iVar2;
        tile_rect.right = tile_rect.right + iVar2;
        local_28 = local_28 + -1;
      } while (local_28 != 0);
      local_28 = 0;
    }
    if (0 < local_5c) {
      local_28 = local_5c;
      local_14 = local_14 + local_5c;
      do {
        buffer->tile(&sprites2_[1], tile_rect.left,tile_rect.top,tile_rect.left,tile_rect.top,tile_rect.right - tile_rect.left,
                     tile_rect.bottom - tile_rect.top);
        tile_rect.left = tile_rect.left + iVar2;
        tile_rect.right = tile_rect.right + iVar2;
        local_28 = local_28 + -1;
      } while (local_28 != 0);
      local_28 = 0;
    }
  }
  if (local_14 < local_58) {
    local_28 = local_58 - local_14;
    do {
      this->fill(&tile_rect,0xf0);
      tile_rect.left = tile_rect.left + iVar2;
      tile_rect.right = tile_rect.right + iVar2;
      local_28 = local_28 + -1;
    } while (local_28 != 0);
  }
  line.left = line.left + local_70;
  StringTemp[0] = 0;
  strcat(StringTemp, "  =  ");
  iVar6 = local_5c;
  uVar3 = TextColourTable[110];
  uVar9 = TextColourTable[111];
  if ((-1 < local_5c) &&
     (uVar3 = TextColourTable[96], uVar9 = TextColourTable[97], BaseCurrent->queue_production_id[0] == -0x45)) {
    uVar3 = TextColourTable[128];
    uVar9 = TextColourTable[129];
  }
  buffer->set_text_color(uVar3,uVar9,1,1);
  if (iVar6 < 0) {
    strcat(StringTemp, "-");
    sVar4 = strlen(StringTemp);
  }
  else {
    strcat(StringTemp, "+");
    sVar4 = strlen(StringTemp);
  }
  line.left = buffer->write_l(StringTemp, &line, sVar4);
  StringTemp[0] = 0;
  iVar5 = iVar6;
  if (iVar6 < 0) {
    iVar5 = -iVar6;
  }
  if (iVar5 < 100) {
    _itoa(0,local_f0,10);
    strcat(StringTemp, local_f0);
    sVar4 = strlen(StringTemp);
    line.left = buffer->write_l(StringTemp, &line, sVar4);
  }
  StringTemp[0] = 0;
  iVar5 = iVar6;
  if (iVar6 < 0) {
    iVar5 = -iVar6;
  }
  if (iVar5 < 10) {
    _itoa(0,local_f0,10);
    strcat(StringTemp, local_f0);
    sVar4 = strlen(StringTemp);
    line.left = buffer->write_l(StringTemp, &line, sVar4);
  }
  StringTemp[0] = 0;
  iVar5 = iVar6;
  if (iVar6 < 0) {
    iVar5 = -iVar6;
  }
  _itoa(iVar5,local_f0,10);
  strcat(StringTemp, local_f0);
  if ((-1 < iVar6) && (BaseCurrent->queue_production_id[0] == -0x45)) {
    strcat(StringTemp, "*");
  }
  sVar4 = strlen(StringTemp);
  buffer->write_l(StringTemp, &line, sVar4);
  line.right = local_7c;
  local_80 = local_80 + 0x15;
  local_78 = local_78 + 0x15;
  expenses_help_rects_[0].left = local_84;
  expenses_help_rects_[0].top = local_80;
  expenses_help_rects_[0].right = local_7c;
  line.left = local_84;
  expenses_help_rects_[0].bottom = local_78;
  title.left = production_outer_rect_.left + 3;
  title.right = 0x128;
  title.top = local_80;
  title.bottom = local_78;
  line.top = local_80;
  line.bottom = local_78;
  buffer->box_sprite(&title, &InterfaceBoxStyles[6]);
  title.left = title.left + 3;
  title.top = title.top + 3;
  title.right = title.right + -3;
  title.bottom = title.bottom + -3;
  buffer->set_text_color(TextColourTable[86],TextColourTable[87],1,1);
  StringTemp[0] = 0;
  strcat(StringTemp, label_get(36));
  CharUpperA(StringTemp);
  sVar4 = strlen(StringTemp);
  buffer->write_cent_l(StringTemp, &title, sVar4);
  iVar6 = BaseCurrent->energy_intake_2;
  line.right = line.right + -3;
  line.top = line.top + 3;
  line.bottom = line.bottom + -3;
  line.left = 0x133;
  StringTemp[0] = 0;
  uVar3 = TextColourTable[110];
  uVar9 = TextColourTable[111];
  if (-1 < iVar6) {
    uVar3 = TextColourTable[86];
    uVar9 = TextColourTable[87];
  }
  buffer->set_text_color(uVar3,uVar9,1,1);
  iVar5 = iVar6;
  if (iVar6 < 0) {
    strcat(StringTemp, "-");
    sVar4 = strlen(StringTemp);
    line.left = buffer->write_l(StringTemp, &line, sVar4);
    StringTemp[0] = 0;
    iVar5 = -iVar6;
  }
  if (iVar5 < 100) {
    if (-1 < iVar6) {
      _itoa(0,local_f0,10);
      strcat(StringTemp, local_f0);
      sVar4 = strlen(StringTemp);
      line.left = buffer->write_l(StringTemp, &line, sVar4);
      StringTemp[0] = 0;
      goto LAB_00414178;
    }
LAB_00414180:
    iVar5 = -iVar6;
  }
  else {
LAB_00414178:
    iVar5 = iVar6;
    if (iVar6 < 0) goto LAB_00414180;
  }
  if (iVar5 < 10) {
    _itoa(0,local_f0,10);
    strcat(StringTemp, local_f0);
    sVar4 = strlen(StringTemp);
    line.left = buffer->write_l(StringTemp, &line, sVar4);
    StringTemp[0] = 0;
  }
  if (iVar6 < 0) {
    iVar6 = -iVar6;
  }
  _itoa(iVar6,local_f0,10);
  strcat(StringTemp, local_f0);
  sVar4 = strlen(StringTemp);
  line.left = buffer->write_l(StringTemp, &line, sVar4);
  StringTemp[0] = 0;
  strcat(StringTemp, " - ");
  sVar4 = strlen(StringTemp);
  line.left = buffer->write_l(StringTemp, &line, sVar4);
  iVar6 = local_54;
  StringTemp[0] = 0;
  if (local_54 < 100) {
    _itoa(0,local_f0,10);
    strcat(StringTemp, local_f0);
    sVar4 = strlen(StringTemp);
    line.left = buffer->write_l(StringTemp, &line, sVar4);
  }
  StringTemp[0] = 0;
  if (iVar6 < 10) {
    _itoa(0,local_f0,10);
    strcat(StringTemp, local_f0);
    sVar4 = strlen(StringTemp);
    line.left = buffer->write_l(StringTemp, &line, sVar4);
  }
  StringTemp[0] = 0;
  if (iVar6 < 0) {
    iVar6 = -iVar6;
  }
  _itoa(iVar6,local_f0,10);
  strcat(StringTemp, local_f0);
  strcat(StringTemp, " ");
  strcat(StringTemp, " ");
  sVar4 = strlen(StringTemp);
  tile_rect.left = buffer->write_l(StringTemp, &line, sVar4);
  StringTemp[0] = 0;
  local_54 = 1;
  if (0 < iVar2 + -1) {
    local_54 = iVar2 + -1;
  }
  tile_rect.right = local_54 + tile_rect.left;
  tile_rect.top = ((line.bottom - line.top) - local_font.pixel_height()) / 2 + line.top;
  tile_rect.bottom = tile_rect.top + local_font.pixel_height();
  local_14 = 0;

  line.left = tile_rect.left;
  if (local_74 < 0) {
    local_8c = 1;
    if (0 < local_60) {

      local_14 = local_60;
      iVar6 = local_60;
      do {
        buffer->tile(&sprite5_, tile_rect.left,tile_rect.top,tile_rect.left,tile_rect.top,tile_rect.right - tile_rect.left,
                     tile_rect.bottom - tile_rect.top);
        tile_rect.left = tile_rect.left + iVar2;
        tile_rect.right = tile_rect.right + iVar2;
        iVar6 = iVar6 + -1;
      } while (iVar6 != 0);
    }
    for (iVar6 = 0; iVar6 < -local_74; iVar6 = iVar6 + 1) {
      buffer->tile(&sprite5_, tile_rect.left,tile_rect.top,tile_rect.left,tile_rect.top,tile_rect.right - tile_rect.left,
                   tile_rect.bottom - tile_rect.top);
      tile_rect.left = tile_rect.left + iVar2;
      tile_rect.right = tile_rect.right + iVar2;
      local_14 = local_14 + 1;
    }
  }
  else {
    if (0 < local_60) {

      local_14 = local_60;
      iVar6 = local_60;
      do {
        buffer->tile(&sprite5_, tile_rect.left,tile_rect.top,tile_rect.left,tile_rect.top,tile_rect.right - tile_rect.left,
                     tile_rect.bottom - tile_rect.top);
        tile_rect.left = tile_rect.left + iVar2;
        tile_rect.right = tile_rect.right + iVar2;
        iVar6 = iVar6 + -1;
      } while (iVar6 != 0);
    }
    iVar6 = (local_58 - local_74) - local_60;
    if (0 < iVar6) {
      local_14 = local_14 + iVar6;
      do {
        this->fill(&tile_rect,0xf0);
        tile_rect.left = tile_rect.left + iVar2;
        tile_rect.right = tile_rect.right + iVar2;
        iVar6 = iVar6 + -1;
      } while (iVar6 != 0);
    }
    if (0 < local_74) {

      local_14 = local_14 + local_74;
      iVar6 = local_74;
      do {
        buffer->tile(&sprites1_[1], tile_rect.left,tile_rect.top,tile_rect.left,tile_rect.top,tile_rect.right - tile_rect.left,
                     tile_rect.bottom - tile_rect.top);
        tile_rect.left = tile_rect.left + iVar2;
        tile_rect.right = tile_rect.right + iVar2;
        iVar6 = iVar6 + -1;
      } while (iVar6 != 0);
    }
  }
  if (local_14 < local_58) {
    iVar6 = local_58 - local_14;
    do {
      this->fill(&tile_rect,0xf0);
      tile_rect.left = tile_rect.left + iVar2;
      tile_rect.right = tile_rect.right + iVar2;
      iVar6 = iVar6 + -1;
    } while (iVar6 != 0);
  }
  uVar7 = line.left + local_70;
  StringTemp[0] = 0;
  line.left = uVar7;
  strcat(StringTemp, "  =  ");
  iVar2 = local_74;
  uVar3 = TextColourTable[88];
  uVar9 = TextColourTable[89];
  if (local_74 < 0) {
    uVar3 = TextColourTable[110];
    uVar9 = TextColourTable[111];
  }
  buffer->set_text_color(uVar3,uVar9,1,1);
  if (iVar2 < 0) {
    strcat(StringTemp, "-");
    sVar4 = strlen(StringTemp);
  }
  else {
    strcat(StringTemp, "+");
    sVar4 = strlen(StringTemp);
  }
  line.left = buffer->write_l(StringTemp, &line, sVar4);
  StringTemp[0] = 0;
  iVar6 = iVar2;
  if (iVar2 < 0) {
    iVar6 = -iVar2;
  }
  if (iVar6 < 100) {
    _itoa(0,local_f0,10);
    strcat(StringTemp, local_f0);
    sVar4 = strlen(StringTemp);
    line.left = buffer->write_l(StringTemp, &line, sVar4);
  }
  StringTemp[0] = 0;
  iVar6 = iVar2;
  if (iVar2 < 0) {
    iVar6 = -iVar2;
  }
  if (iVar6 < 10) {
    _itoa(0,local_f0,10);
    strcat(StringTemp, local_f0);
    sVar4 = strlen(StringTemp);
    line.left = buffer->write_l(StringTemp, &line, sVar4);
  }
  StringTemp[0] = 0;
  if (iVar2 < 0) {
    iVar2 = -iVar2;
  }
  _itoa(iVar2,local_f0,10);
  strcat(StringTemp, local_f0);
  sVar4 = strlen(StringTemp);
  line.left = buffer->write_l(StringTemp, &line, sVar4);
  buffer->set_text_color(TextColourTable[84],TextColourTable[85],1,1);
  line.bottom = local_78 + 0x15;
  line.top = local_80 + 0x15;
  line.left = local_84;
  line.right = local_7c;
  buffer->box_sprite(&line, &InterfaceBoxStyles[6]);
  StringTemp[0] = 0;
  line.left = uVar7;
  strcat(StringTemp, "  =  ");
  if (local_8c == 0) {
    strcat(StringTemp, label_get(52));
    CharUpperA(StringTemp);
  }
  else {
    buffer->set_text_color(TextColourTable[110],TextColourTable[111],1,1);
    strcat(StringTemp, label_get(697));
  }
  sVar4 = strlen(StringTemp);
  buffer->write_l(StringTemp, &line, sVar4);
  buffer->set_text_color(TextColourTable[84],TextColourTable[85],1,1);
  line.left = local_90;
  line.right = local_90;
  StringTemp[0] = 0;
  strcat(StringTemp, label_get(655));
  CharUpperA(StringTemp);
  sVar4 = strlen(StringTemp);
  buffer->write_right_l(StringTemp, &line, sVar4);
  StringTemp[0] = 0;
  strcat(StringTemp, " - ");
  sVar4 = strlen(StringTemp);
  iVar6 = buffer->write_l(StringTemp, &line, sVar4);
  iVar2 = local_60;
  line.left = iVar6 + 5;
  StringTemp[0] = 0;
  if (local_60 != 0) {
    strcat(StringTemp, "(");
  }
  strcat(StringTemp, label_get(656));
  if (iVar2 != 0) {
    strcat(StringTemp, " + ");
  }
  CharUpperA(StringTemp);
  sVar4 = strlen(StringTemp);
  uVar7 = buffer->write_l(StringTemp, &line, sVar4);
  if (local_60 != 0) {
    buffer->set_text_color(TextColourTable[110],TextColourTable[111],1,1);
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(56));
    CharUpperA(StringTemp);
    line.left = uVar7;
    sVar4 = strlen(StringTemp);
    line.left = buffer->write_l(StringTemp, &line, sVar4);
    buffer->set_text_color(TextColourTable[84],TextColourTable[85],1,1);
    sVar4 = strlen(")");
    buffer->write_l(const_cast<char *>(")"), &line, sVar4);
  }
  if (param_2 == 0) {
    this->soft_update(&production_outer_rect_);
  }
  else {
    this->update(&production_outer_rect_, 0);
  }
  buffer->set_clip(&saved_clip);
  return;
}
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00415B20 ?draw_info@BaseWin@@QAEXH@Z 0x00415B20-0x00415DCE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00415b20.cpp", "src/basewin.h", "src/base.h", "src/faction.h", "src/maininterface.h", "src/buffer.h"], "reason": "Recover the base title panel through owned BaseWin rectangles and Sprite dimensions, MainInterface's Buffer and shared frame styles, and typed Base/Player name and grammar fields. The seed retains compiler/reference evidence."}
void BaseWin::draw_info(int update_mode) {
    Buffer *screen = static_cast<Buffer *>(&MainInterfaceGlobal);

    RECT outer = rect12_;
    outer.left -= 3;
    outer.top -= 3;
    outer.right += 3;
    outer.bottom += 3;
    screen->set_clip(&outer);
    screen->box_sprite(&outer, &InterfaceBoxStyles[3]);

    RECT panel = info_title_rect_;
    screen->set_clip(&panel);
    RECT previous_clip = screen->current_clip();
    screen->box_sprite(&panel, &InterfaceBoxStyles[3]);

    const int trim = -6 - sprites8_[6].width();
    panel.left -= trim;
    panel.right += trim;
    panel.top += 3;
    panel.bottom -= 3;
    screen->box_sprite(&panel, &InterfaceBoxStyles[0]);
    panel.bottom -= 3;
    panel.left += 3;
    panel.right -= 3;
    panel.top += 3;

    StringTemp[0] = 0;
    strcat(StringTemp, BaseCurrent->name_string);
    const int faction = BaseCurrent->faction_id_current;
    if (faction != ConsoleGlobal.faction_id_) {
        strcat(StringTemp, " ");
        strcat(StringTemp, "(");
        GenderDefault = Players[faction].noun_gender;
        PluralityDefault = Players[faction].is_noun_plural;
        strcat(StringTemp, Players[faction].noun_faction);
        strcat(StringTemp, ")");
    }
    CharUpperA(StringTemp);

    int font_size = 32;
    do {
        Font *candidate = find_font(font_size, 1);
        if (candidate->width(StringTemp) <= panel.right - panel.left) break;
        font_size -= 2;
    } while (font_size > 0);

    screen->set_text_color(ChatItemIdTable[faction], -1, 1, 1);
    screen->set_font(find_font(font_size, 1), 0, 0, 0);
    screen->write_cent_l(StringTemp, &panel, strlen(StringTemp));
    if (update_mode == 0) {
        MainInterfaceGlobal.soft_update(&info_title_rect_);
    } else {
        MainInterfaceGlobal.update(&info_title_rect_, nullptr);
    }
    screen->set_clip(&previous_clip);
}
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00415730 ?draw_misc@BaseWin@@QAEXH@Z 0x00415730-0x00415B15
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00415730.cpp", "src/basewin.h", "src/alpha.h", "src/faction.h", "src/base.h", "src/interface_palette.cpp", "src/jackal/font.h"], "reason": "Recover the miscellaneous base panel through its owned rectangle, label and string table, live Base and PlayerData fields, font line advance, and shared palette rather than executable addresses and raw self-offsets. The retained seed remains for byte restoration."}
void BaseWin::draw_misc(int update_mode) {
    Buffer *canvas = this;
    RECT panel = rect5_;
    canvas->set_clip(&panel);
    canvas->box_sprite(&panel, &InterfaceBoxStyles[3]);
    panel.left += 3;
    panel.top += 3;
    panel.right -= 3;
    panel.bottom -= 3;
    canvas->box_sprite(&panel, &InterfaceBoxStyles[0]);
    panel.left += 3;
    panel.top += 3;
    panel.right -= 3;
    panel.bottom -= 3;

    Font *font = find_font(12, 0);
    canvas->set_font(font, nullptr, nullptr, nullptr);
    const int advance = font->line_advance();
    const int row_height = panel.bottom - advance * 3 - panel.top;
    int y = panel.top + row_height / 2;
    int x = panel.left;

    canvas->set_text_color(TextColourTable[104], TextColourTable[105], 1, 1);
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(730));
    x = canvas->write_l(StringTemp, x, y, strlen(StringTemp));

    canvas->set_text_color(TextColourTable[106], TextColourTable[107], 1, 1);
    StringTemp[0] = 0;
    strcat(StringTemp, " ");
    say_year(StringTemp);
    canvas->write_l(StringTemp, x, y, strlen(StringTemp));

    y += font->line_advance();
    x = panel.left;
    const int faction = BaseCurrent->faction_id_current;
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(36));
    strcat(StringTemp, ": ");
    canvas->set_text_color(TextColourTable[104], TextColourTable[105], 1, 1);
    x = canvas->write_l(StringTemp, x, y, strlen(StringTemp));

    const int reserves = PlayersData[faction].energy_reserves;
    const int hurry_cost = PlayersData[faction].hurry_cost_total;
    StringTemp[0] = 0;
    char number[80];
    _itoa(reserves - hurry_cost, number, 10);
    strcat(StringTemp, number);
    canvas->set_text_color(TextColourTable[106], TextColourTable[107], 1, 1);
    canvas->write_l(StringTemp, x, y, strlen(StringTemp));

    y += font->line_advance();
    x = panel.left;
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(958));
    strcat(StringTemp, ": ");
    strcat(StringTemp, " ");
    const int eco_damage = BaseCurrent->eco_damage;
    _itoa(eco_damage, number, 10);
    strcat(StringTemp, number);
    if (eco_damage != 0) {
        canvas->set_text_color(TextColourTable[100], TextColourTable[101], 1, 1);
    } else {
        canvas->set_text_color(TextColourTable[98], TextColourTable[99], 1, 1);
    }
    canvas->write_l(StringTemp, x, y, strlen(StringTemp));

    if (update_mode != 0) {
        update(&rect5_, nullptr);
    } else {
        soft_update(&rect5_);
    }
}
// ORIGINAL: 0x0040F0F0 ?draw_farm@BaseWin@@QAEXH@Z 0x0040F0F0-0x0040FCB3
// RECOVERY_BINDING: v1 {"evidence": ["image:0x0040F0F0; original spans remain load-bearing.", "Retired seed provenance git:1432fed4:src/unrecovered/0040f0f0.cpp; historical transcription conflicts remain evidence, not runtime targets.", "image:41E8D0 owns panel geometry; source:src/basewin.h owns real GraphicWin/Buffer and existing Base/Player/Veh/Rules receivers."], "reason": "Recover the complete BaseWin draw_farm panel using genuine owned rectangles, automatic Font lifetime and existing application globals, retaining original branch/call order and unchecked input obligations."}
void BaseWin::draw_farm(int update_mode) {
    int tile; int resource;
    Buffer *canvas = static_cast<Buffer *>(this);
    RECT panel = rect1_;
    RECT saved_clip = canvas->current_clip();
    canvas->set_clip(&panel);
    canvas->box_sprite(&rect1_, &InterfaceBoxStyles[15]);
    ++panel.left;
    ++panel.top;
    --panel.right;
    --panel.bottom;
    fill(&panel, 0x41);
    spot2_.kill_type(0);
    spot2_.add(0, 0, rect1_.left, rect1_.top,
               rect1_.right - rect1_.left, rect1_.bottom - rect1_.top);

    // Modes2/3 explicitly select psych/support; modes<=1 select live state.
    if ((update_mode < 2 && field_40AFC_ == 1) || update_mode == 2) {
        draw_psych();
    } else if ((update_mode < 2 && field_40AFC_ == 2) || update_mode == 3) {
        draw_support();
    } else {
        int quarter = MapWinCaptionScale < 1 ? 50
            : (MapWinCaptionScale * 50) / MapWinCaptionDivisor;
        quarter = (quarter + 1) / 4;
        const int origin_x = (rect1_.right - rect1_.left) / 2
            - quarter * 4 + rect1_.left;
        const int origin_y = quarter * 6 + 4 + rect1_.top;
        const int half_tile = quarter * 2;

        FarmMapWindow.clear_terrain();
        FarmMapWindow.field_1DD6C_ = 0;
        MapWinCaptionScale = MapWinCaptionDivisor = 1;
        for (tile = 0; tile < 21; ++tile) {
            const int dx = BaseRadiusOffsetX[tile];
            const int dy = BaseRadiusOffsetY[tile];
            int x = BaseCurrent->x + dx;
            if (!(MapIsFlat & 1)) {
                if (x < 0) x += MapLongitudeBounds;
                else if (x >= MapLongitudeBounds) x -= MapLongitudeBounds;
            }
            const int y = BaseCurrent->y + dy;
            if (y < 0 || y >= MapLatitudeBounds || x < 0 || x >= MapLongitudeBounds)
                continue;
            int radius = 0;
            for (; radius < 21; ++radius) {
                if (dx == RadiusOffsetX[radius] && dy == RadiusOffsetY[radius]) break;
            }
            if (radius == 21) radius = 0;
            if (FarmStatus[radius] & 2) continue;
            const int px = dx * quarter * 4 + origin_x;
            const int py = dy * quarter * 2 + origin_y;
            gen_terrain_poly(canvas, 0, 0, BaseCurrent->faction_id_current,
                             radius, 0x4010621, x, y, px, py);
            if (FarmStatus[radius] & 0x20)
                polygon_dest(canvas, MapCornerScratch, 4, ReddishColorTable);
            const unsigned status = FarmStatus[radius];
            if ((status & 4) || radius == 0) {
                const int base = base_at(x, y);
                if (base >= 0) base_draw(canvas, base, px, py - quarter * 2, 0, 0);
            } else if (status & 8) {
                const int vehicle = veh_at(x, y);
                if (vehicle >= 0) veh_draw(canvas, vehicle, px, py - quarter * 2, 0, 0, 0);
            } else if (status & 0x10) {
                polygon_dest(canvas, MapCornerScratch, 4, ReddishColorTable);
                g_RADIUS1_TEXTURE.draw_trans(canvas, MapCornerScratch, 0);
            }
        }

        FarmMapWindow.clear_terrain();
        FarmMapWindow.field_1DD6C_ <<= 2;
        MapWinCaptionScale = MapWinCaptionDivisor = 1;
        SpriteScaleNumerator = SpriteScaleDenominator = 1;
        Base *base = BaseCurrent;
        int longitude = MapLongitudeBounds;
        for (tile = 0; tile < 21; ++tile) {
            const int dx = BaseRadiusOffsetX[tile];
            const int dy = BaseRadiusOffsetY[tile];
            int x = base->x + dx;
            if (!(MapIsFlat & 1)) {
                if (x < 0) x += longitude;
                else if (x >= longitude) x -= longitude;
            }
            const int y = base->y + dy;
            if (y >= 0 && y < MapLatitudeBounds && x >= 0 && x < longitude) {
                int radius = 0;
                for (; radius < 21; ++radius) {
                    if (dx == RadiusOffsetX[radius] && dy == RadiusOffsetY[radius]) break;
                }
                if (radius == 21) radius = 0;
                if (!(FarmStatus[radius] & 2)
                    && (base->resource_sqr_radius & (1u << radius))) {
                    const int px = dx * half_tile * 2 + origin_x;
                    int tile_alt = FarmMapWindow.get_alt(x, y, 4);
                    tile_alt += FarmMapWindow.get_alt(x, y, 3);
                    tile_alt += FarmMapWindow.get_alt(x, y, 2);
                    tile_alt += FarmMapWindow.get_alt(x, y, 1);
                    const int base_x = BaseCurrent->x;
                    const int base_y = BaseCurrent->y;
                    int base_alt = FarmMapWindow.get_alt(base_x, base_y, 4);
                    base_alt += FarmMapWindow.get_alt(base_x, base_y, 3);
                    base_alt += FarmMapWindow.get_alt(base_x, base_y, 2);
                    base_alt += FarmMapWindow.get_alt(base_x, base_y, 1);
                    const int py = dy * half_tile - (tile_alt >> 2)
                        + (base_alt >> 2) + origin_y;
                    int yield[4], restricted[4];
                    int total = 0;
                    for (resource = 0; resource < 4; ++resource) {
                        TileYieldRestricted = 0;
                        yield[resource] = resource_yield(resource,
                            BaseCurrent->faction_id_current, BaseIDCurrentSelected, x, y);
                        total += yield[resource];
                        restricted[resource] = TileYieldRestricted;
                    }
                    int width = 0;
                    if (total) {
                        for (resource = 0; resource < 4; ++resource) {
                            if (!yield[resource]) continue;
                            int index = yield[resource] - 1;
                            if (index < 0) index = 0;
                            else if (index > 7) index = 7;
                            const int sw = g_RESOURCE_ICON_SPRITES[resource * 8 + index].source_width();
                            width += MapWinCaptionScale < 1 ? sw * 17 / 16
                                : sw * MapWinCaptionScale / MapWinCaptionDivisor;
                        }
                    } else {
                        const int sw = g_NULL_RESOURCE_ICON_SPRITE.width();
                        width = MapWinCaptionScale < 1 ? sw * 17 / 16
                            : sw * MapWinCaptionScale / MapWinCaptionDivisor;
                    }
                    int cursor = half_tile * 4 / 2 - width / 2 + px;
                    const int icon_height = MapWinCaptionScale < 1 ? 42
                        : MapWinCaptionScale * 40 / MapWinCaptionDivisor;
                    const int icon_y = py - (icon_height >> 1) + half_tile;
                    for (resource = 0; resource < 4; ++resource) {
                        if (!yield[resource]) continue;
                        int actual = yield[resource] - 1;
                        if (actual < 0) actual = 0;
                        else if (actual > 7) actual = 7;
                        int potential = restricted[resource] + actual;
                        if (potential < 0) potential = 0;
                        else if (potential > 7) potential = 7;
                        Sprite &actual_icon = g_RESOURCE_ICON_SPRITES[resource * 8 + actual];
                        Sprite &potential_icon = g_RESOURCE_ICON_SPRITES[resource * 8 + potential];
                        if (potential != actual) {
                            potential_icon.draw_mono(canvas, potential_icon.transparent_index(),
                                                     cursor, icon_y, 0, 1, 1);
                            actual_icon.draw(canvas, actual_icon.transparent_index(),
                                cursor + (potential - actual) / 2, icon_y, 1, 1);
                        } else {
                            actual_icon.draw(canvas, actual_icon.transparent_index(),
                                             cursor, icon_y, 1, 1);
                        }
                        cursor += potential_icon.source_width();
                    }
                    if (!total)
                        g_NULL_RESOURCE_ICON_SPRITE.draw(canvas,
                            g_NULL_RESOURCE_ICON_SPRITE.transparent_index(), cursor, icon_y);
                    base = BaseCurrent;
                    longitude = MapLongitudeBounds;
                }
            }
        }

        Font *font = find_font(12, 1);
        const int text_right = rect1_.right - 2;
        const int text_top = rect1_.top + 2;
        canvas->set_font(font, 0, 0, 0);
        StringTemp[0] = 0;
        const unsigned event = BaseCurrent->event;
        int label = -1, suffix = 0, colour = 0;
        if (event & 0x200) { label = 0xBE0 / 4; suffix = 0xBD8 / 4; colour = 90; }
        else if (event & 0x400) { label = 0xBE4 / 4; suffix = 0xBDC / 4; colour = 90; }
        else if (event & 0x800) { label = 0xBE8 / 4; suffix = 0xBD8 / 4; colour = 94; }
        else if (event & 0x1000) { label = 0xBEC / 4; suffix = 0xBDC / 4; colour = 94; }
        else if (event & 0x2000) { label = 0xBF0 / 4; suffix = 0xBD8 / 4; colour = 86; }
        else if (event & 0x4000) { label = 0xBF4 / 4; suffix = 0xBDC / 4; colour = 86; }
        else if (GameDataStateStorage.fields_.dust_cloud_duration_ > 0) { label = 0xBF8 / 4; suffix = 0xBDC / 4; colour = 86; }
        if (label >= 0) {
            strcat(StringTemp, label_get(label));
            strcat(StringTemp, ": ");
            strcat(StringTemp, label_get(suffix));
            canvas->set_text_color(TextColourTable[colour], TextColourTable[colour + 1], 1, 1);
        }
        canvas->write_right_l(StringTemp, text_right, text_top, strlen(StringTemp));
    }
    SpriteScaleNumerator = SpriteScaleDenominator = 1;
    MapWinCaptionScale = MapWinCaptionDivisor = -1;
    if (!update_mode) soft_update(&rect1_);
    else update(&rect1_, 0);
    canvas->set_clip(&saved_clip);
}
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00414B40 ?draw_pop@BaseWin@@QAEXH@Z 0x00414B40-0x00415078;0x00651B76-0x00651B93
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00414b40.cpp", "image:0x00414C58-0x00414CA3 population spacing uses panel width minus height", "image:0x00414CB4-0x00414CDD expansion and alien rule select alien sprites", "src/sprite.cpp", "src/base.h", "src/faction.h", "src/basewin.h"], "reason": "Draw the current base's citizen and specialist row through owned BaseWin panel and Spot members, typed Base mood/specialist fields, and the four real large-sprite arrays. The retained seed reverses the alien branch and mistranscribes the spacing; the pinned instructions resolve both."}
void BaseWin::draw_pop(int update_mode) {
    Font font;
    Random random;
    random.reseed(timeGetTime());

    Buffer *canvas = static_cast<Buffer *>(&MainInterfaceGlobal);
    RECT saved_clip = canvas->current_clip();
    RECT inner = population_rect_;
    canvas->set_clip(&inner);
    canvas->box_sprite(&population_rect_, &InterfaceBoxStyles[3]);
    font.init(DefaultFontFace, 9, 1);
    spot1_.kill_type(1);

    inner.left += 3;
    inner.top += 3;
    inner.right -= 3;
    inner.bottom -= 3;
    canvas->set_clip(&inner);

    const int height = inner.bottom - inner.top;
    const int width = inner.right - inner.left;
    const int population = BaseCurrent->population_size;
    int spacing = height;
    if (population > 1) {
        spacing = (width - height) / (population - 1);
        if (spacing < 1 || height < 1) spacing = 1;
        else if (spacing > height) spacing = height;
    }

    const int faction = BaseCurrent->faction_id_current;
    const bool alien = ExpansionEnabled != 0
        && (Players[faction].rule_flags & 0x80) != 0;
    Sprite *citizens = alien ? g_AL_CITIZEN_LG_CURSOR_SPRITES
                             : g_CITIZEN_LG_CURSOR_SPRITES;
    Sprite *specialists = alien ? g_AL_SPECIALIST_LG_CURSOR_SPRITES
                                : g_SPECIALIST_LG_CURSOR_SPRITES;
    int x = inner.left;
    for (int i = 0; i < BaseCurrent->population_size; ++i) {
        const Base *base = BaseCurrent;
        const int citizen_count = base->population_size - base->specialist_total;
        const int y = inner.top + (height - 48) / 2;
        if (i < citizen_count) {
            int mood = i >= base->talent_total ? 1 : 0;
            if (i >= citizen_count - base->drone_total) mood = 2;
            if (i >= citizen_count - base->super_drone_total) mood = 3;
            const int variant = alien ? 0 : static_cast<int>(random.get(0, 2));
            Sprite &sprite = citizens[variant * 4 + mood];
            sprite.draw(canvas, sprite.transparent_index(), x, y, 1, 1);
            spot1_.add(-1, 1, x, inner.top, height, height);
        } else {
            const int specialist = i - citizen_count;
            const int type = specialist < 16
                ? (base->spec[specialist >> 3] >> ((specialist & 7) * 4)) & 15
                : best_specialist();
            Sprite &sprite = specialists[type];
            sprite.draw(canvas, sprite.transparent_index(), x, y, 1, 1);
            spot1_.add(specialist, 1, x, inner.top, height, height);
        }
        x += spacing;
    }

    if (update_mode == 0) MainInterfaceGlobal.soft_update(&population_rect_);
    else MainInterfaceGlobal.update(&population_rect_, nullptr);
    canvas->set_clip(&saved_clip);
}
// ORIGINAL: 0x00410870 ?draw_commerce@BaseWin@@QAEXH@Z 0x00410870-0x00411235;0x00651B22-0x00651B37
// RECOVERY_BINDING: v1 {"evidence": ["image:0x004108CC-0x00410A95 copies/clips three BaseWin rectangles, initializes a Font, draws the title and sets the report palette", "image:0x00410B3C-0x00410D54 counts the first 70 Base facility bits and 64 SecretProject homes at the current base, weights projects five, then formats the alien score or sanction warning", "image:0x00410D54-0x0041111C selects partner rows and formats receipts, pact/treaty status, unequal exchanges and sanctions from Base/PlayerData fields", "image:0x004111DE-0x00411211 updates the outer rectangle and restores the clip"], "reason": "Recover the commerce panel's alien score and normal-faction partner rows through owned BaseWin rectangles, typed Base/SecretProject/diplomacy state and source-owned commerce ledgers; original compiler/reference/runtime proof remains separate."}
// TRIED: SEH-framed function with one real local object (`Font font;`, matching the single ??0Font/??1Font pair) reproduced via plain RAII, same as 0x004ADB70. Diverges at #12 (sim 0.53), not chased further given time. The "own faction" index scan, both box_sprite/set_font/ set_text_color RECT setups, and the tail (soft_update vs update on a1, restore clip) are a direct instruction-level transcription off raw disassembly. The per-faction commerce-report loop body (the `slot` loop building relation strings via write_l) and the military-score ELSE branch are adapted from Ghidra with real field offsets and real calls but approximated message-selection logic - not verified call-for-call against raw bytes given the remaining budget.
void BaseWin::draw_commerce(int update_mode) {
    Buffer *canvas = this;
    RECT saved_clip = canvas->current_clip();
    canvas->set_clip(&commerce_outer_rect_);
    canvas->box_sprite(&commerce_outer_rect_, &InterfaceBoxStyles[3]);

    Font font;
    font.init(DefaultFontFace, 10, 1);
    RECT title = commerce_title_rect_;
    canvas->box_sprite(&title, &InterfaceBoxStyles[6]);
    title.left += 3;
    title.top += 3;
    title.right -= 3;
    title.bottom -= 3;
    canvas->set_font(&font1_, nullptr, nullptr, nullptr);
    canvas->set_text_color(TextColourTable[82], TextColourTable[83], 1, 1);
    const int faction = Bases[focusedBaseID_].faction_id_current;
    StringTemp[0] = 0;
    if ((Players[faction].rule_flags & RFLAG_ALIEN) != 0) {
        strcat(StringTemp, label_get(1107));
    } else {
        strcat(StringTemp, label_get(313));
        CharUpperA(StringTemp);
    }
    canvas->write_cent_l(StringTemp, &title, strlen(StringTemp));

    canvas->set_font(&font, nullptr, nullptr, nullptr);
    canvas->set_text_color(TextColourTable[104], TextColourTable[105], 1, 1);
    RECT report = commerce_report_rect_;
    canvas->box_sprite(&report, &InterfaceBoxStyles[0]);
    report.left += 3;
    report.top += 3;
    report.right -= 3;
    report.bottom -= 3;
    const int row_pitch = (report.bottom - report.top) / 6;
    int row_y = report.top;
    bool wrote_row = false;
    if ((Players[BaseCurrent->faction_id_current].rule_flags & RFLAG_ALIEN) != 0) {
        canvas->set_font(&font1_, nullptr, nullptr, nullptr);
        int current_base_id = 0;
        for (int candidate = 0; candidate < GameDataStateStorage.fields_.base_current_count_; ++candidate) {
            if (&Bases[candidate] == BaseCurrent) current_base_id = candidate;
        }
        int score = 0;
        for (int facility = 0; facility < 70; ++facility) {
            if ((BaseCurrent->facilities_built[facility >> 3] &
                 (1u << (facility & 7))) != 0) ++score;
        }
        // The original's 64 dword slots are the whole BaseSecretProject ABI
        // record. Copy its representation into a homogeneous local view so
        // this census does not walk across C++ struct members by pointer.
        typedef char ProjectRecordSizeCheck[
            sizeof(BaseSecretProject) == MaxSecretProjectNum * sizeof(int) ? 1 : -1];
        (void)sizeof(ProjectRecordSizeCheck);
        int project_homes[MaxSecretProjectNum];
        memcpy(project_homes, &GameDataStateStorage.fields_.secret_project_, sizeof(project_homes));
        for (int project = 0; project < MaxSecretProjectNum; ++project) {
            if (project_homes[project] == current_base_id) score += 5;
        }
        score /= 2;
        StringTemp[0] = 0;
        if (PlayersData[BaseCurrent->faction_id_current].sanction_turns != 0) {
            canvas->set_text_color(TextColourTable[110], TextColourTable[111], 1, 1);
            strcat(StringTemp, label_get(1125));
            wrote_row = true;
        } else if (score != 0) {
            canvas->set_text_color(0xFF, -1, 1, 1);
            char number[32];
            strcat(StringTemp, "+");
            _itoa(score, number, 10);
            strcat(StringTemp, number);
            strcat(StringTemp, " ");
            strcat(StringTemp, label_get(36));
            wrote_row = true;
        }
        if (wrote_row) {
            CharUpperA(StringTemp);
            canvas->write_cent_l(StringTemp, &report, strlen(StringTemp));
        }
    } else {
        const int owner = BaseCurrent->faction_id_current;
        for (int partner = 1; partner < MaxPlayerNum; ++partner) {
            const int slot = partner - 1;
            if (partner == owner ||
                (PlayersData[owner].diplo_treaties[partner] &
                 (DTREATY_PACT | DTREATY_TREATY)) == 0 ||
                (BaseCommerceReceipts[slot] == 0 &&
                 BaseCommercePartnerReceipts[slot] == 0) ||
                (Players[partner].rule_flags & RFLAG_ALIEN) != 0) {
                continue;
            }
            wrote_row = true;
            GenderDefault = Players[partner].noun_gender;
            PluralityDefault = Players[partner].is_noun_plural;
            canvas->set_text_color(ChatItemIdTable[partner], -1, 1, 1);
            StringTemp[0] = 0;
            strcat(StringTemp, Players[partner].noun_faction);
            CharUpperA(StringTemp);
            const int own_sanctions = PlayersData[owner].sanction_turns;
            const int partner_sanctions = PlayersData[partner].sanction_turns;
            if (own_sanctions == 0 && partner_sanctions == 0) {
                char amount[32];
                strcat(StringTemp, ": +");
                _itoa(BaseCommerceReceipts[slot], amount, 10);
                strcat(StringTemp, amount);
            }
            canvas->write_l(StringTemp, report.left, row_y, strlen(StringTemp));

            StringTemp[0] = 0;
            const int detail_y = row_y + font.line_advance();
            int text_end;
            if (own_sanctions != 0 || partner_sanctions != 0) {
                canvas->set_text_color(TextColourTable[110], TextColourTable[111], 1, 1);
                strcat(StringTemp, label_get(450));
                strcat(StringTemp, " (");
                char turns[32];
                _itoa(own_sanctions > partner_sanctions
                          ? own_sanctions : partner_sanctions, turns, 10);
                strcat(StringTemp, turns);
                strcat(StringTemp, ")");
                CharUpperA(StringTemp);
                text_end = canvas->write_l(StringTemp, report.left + 5,
                                           detail_y, strlen(StringTemp));
            } else {
                strcat(StringTemp, label_get((PlayersData[owner].diplo_treaties[partner]
                                              & DTREATY_PACT) != 0 ? 645 : 644));
                text_end = canvas->write_l(StringTemp, report.left + 5,
                                           detail_y, strlen(StringTemp));
            }
            if (BaseCommercePartnerReceipts[slot] != BaseCommerceReceipts[slot]) {
                StringTemp[0] = 0;
                strcat(StringTemp, " (");
                strcat(StringTemp, label_get(648));
                strcat(StringTemp, " +");
                char partner_amount[32];
                _itoa(BaseCommercePartnerReceipts[slot], partner_amount, 10);
                strcat(StringTemp, partner_amount);
                strcat(StringTemp, ")");
                canvas->write_l(StringTemp, text_end, detail_y, strlen(StringTemp));
            }
            row_y += row_pitch;
        }
    }
    if (!wrote_row) {
        canvas->set_font(&font1_, nullptr, nullptr, nullptr);
        canvas->set_text_color(TextColourTable[104], TextColourTable[105], 1, 1);
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(25));
        canvas->write_cent_l(StringTemp, &commerce_report_rect_, strlen(StringTemp));
    }
    if (update_mode == 0) MainInterfaceGlobal.soft_update(&commerce_outer_rect_);
    else MainInterfaceGlobal.update(&commerce_outer_rect_, nullptr);
    canvas->set_clip(&saved_clip);
}
// ORIGINAL: 0x00411240 ?draw_nutrients@BaseWin@@QAEXH@Z 0x00411240-0x00411973;0x00651B37-0x00651B4C
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00411240.cpp:1912-2151", "src/basewin.h:nutrient rectangles and sprites3_", "src/base.h:Base nutrients_accumulated and nutrient_surplus", "src/base.cpp:BaseIDCurrentSelected and cost_factor", "image:0x00682B20-0x00682B28 nutrient punctuation"], "reason": "Render the growth grid and caption through owned BaseWin rectangles, sprites, current Base and label table. Retain the instruction-level seed for restoration rather than hardcoding its self/global addresses."}
// TRIED: literal transcription of Ghidra's (clean, non-SEH-mangled) pseudocode, mapping the three unlabelled thiscall calls (0x618EA0/0x618EE0/0x618F40) onto BaseWin's already-declared UNK4/UNK7/UNK6 slots by call-site position and declaring the other dozen FUN_ helpers as plain cdecl free functions (their true receiver/convention was not established) compiles and matches the arithmetic/branch structure, but the original installs a real SEH frame (fs:[0] chain) that this body does not reproduce since nothing here needs unwind-safety in the C++ source; diverges from the framed build around #2.
void BaseWin::draw_nutrients(int update_mode) {
    Buffer *canvas = this;
    RECT saved_clip = canvas->current_clip();
    canvas->set_clip(&nutrient_outer_rect_);
    canvas->box_sprite(&nutrient_outer_rect_, &InterfaceBoxStyles[3]);

    Font font;
    font.init(DefaultFontFace, WinScreenWidth == 800 ? 10 : 12, 0);
    RECT title = nutrient_title_rect_;
    canvas->box_sprite(&title, &InterfaceBoxStyles[6]);
    title.left += 3;
    title.top += 3;
    title.right -= 3;
    title.bottom -= 3;
    canvas->set_font(&font1_, nullptr, nullptr, nullptr);
    canvas->set_text_color(TextColourTable[82], TextColourTable[83], 1, 1);
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(520));
    CharUpperA(StringTemp);
    canvas->write_cent_l(StringTemp, &title, strlen(StringTemp));

    RECT content = nutrient_content_rect_;
    canvas->box_sprite(&content, &InterfaceBoxStyles[0]);
    content.left += 3;
    content.top += 3;
    content.right -= 3;
    content.bottom -= 3;
    canvas->set_font(&font, nullptr, nullptr, nullptr);
    canvas->set_text_color(TextColourTable[104], TextColourTable[105], 1, 1);
    const int line_height = font.line_advance();
    const int grid_bottom = content.bottom - 2 * line_height;
    const int stored = BaseCurrent->nutrients_accumulated;
    const int surplus = BaseCurrent->nutrient_surplus;
    const int growth_cost = cost_factor(BaseCurrent->faction_id_current, 0,
                                        BaseIDCurrentSelected);
    const int total = (BaseCurrent->population_size + 1) * growth_cost;

    if (total > 0 && growth_cost > 0) {
        const int width = content.right - content.left;
        const int height = grid_bottom - content.top;
        int columns = growth_cost;
        if (columns * 2 < width) {
            while (height <= (total / columns) * 2 && height != (total / columns) * 2)
                columns *= 2;
        } else {
            columns = 0;
            do { ++columns; } while (columns * columns < total);
            columns += 2;
        }
        const int rows = (total + columns - 1) / columns;
        if (rows > 0) {
            const int cell_width = width / columns;
            const int cell_height = height / rows;
            int left = content.left + (width - cell_width * columns) / 2;
            int top = content.top + (height - cell_height * rows) / 2;
            if (cell_height > 7) { ++left; ++top; }
            for (int index = 0; index < total; ++index) {
                const int column = index % columns;
                const int row = index / columns;
                RECT cell = {left + column * cell_width, top + row * cell_height,
                             left + (column + 1) * cell_width - 1,
                             top + (row + 1) * cell_height - 1};
                if (index < stored) {
                    if (cell_height < 8) GraphicWin::fill(&cell, 0x95);
                    else canvas->tile(&sprites3_[1], cell.left, cell.top,
                                      cell.left, cell.top, cell_width - 1, cell_height - 1);
                } else {
                    if (cell_height > 7) canvas->tile(&sprites3_[0], cell.left, cell.top,
                                      cell.left, cell.top, cell_width - 1, cell_height - 1);
                    else GraphicWin::fill(&cell, 0x98);
                }
                if (surplus < 0 && index >= stored + surplus && index < stored)
                    canvas->box(&cell, 0x6c, 0x6c);
                else if (surplus > 0 && index >= stored && index < stored + surplus)
                    canvas->box(&cell, 0x95, 0x95);
            }
        }
    }

    const int remaining = growth_cost * (BaseCurrent->population_size + 1) - stored;
    int turns = 1;
    if (remaining > 0) {
        if (surplus > 0) turns = (surplus - 1 + remaining) / surplus + 1;
        else if (surplus < 0) turns = (-surplus - 1 + stored) / -surplus + 1;
        else turns = update_mode;
    }
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(surplus < 0 ? 51 : 47));
    if (surplus == 0 && remaining > 0) {
        strcat(StringTemp, ": -");
        strcat(StringTemp, label_get(50));
        strcat(StringTemp, "-");
    } else if (surplus < 0 && remaining > 0) {
        strcat(StringTemp, "!");
    } else {
        char number[32];
        if (turns < 0) turns = 0;
        if (turns > 9999) turns = 9999;
        strcat(StringTemp, ": ");
        _itoa(turns, number, 10);
        strcat(StringTemp, number);
        strcat(StringTemp, " ");
        strcat(StringTemp, label_get(turns == 1 ? 48 : 49));
    }
    canvas->write_cent_l(StringTemp, content.left,
                         content.bottom - line_height - 2,
                         content.right - content.left, strlen(StringTemp));
    if (update_mode == 0) MainInterfaceGlobal.soft_update(&nutrient_outer_rect_);
    else MainInterfaceGlobal.update(&nutrient_outer_rect_, nullptr);
    canvas->set_clip(&saved_clip);
}

// ORIGINAL: 0x0041B790 ?base_editor_fac@BaseWin@@QAEXXZ 0x0041B790-0x0041BBBC;0x0065249C-0x0065255E
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/recovered/units/0041b790.cpp", "source:src/basewin.h", "image:0x41b790"], "reason": "Recover the full base-editor callee in its actual BaseWin owner with one genuine Popup lifetime and original cancellation, menu IDs, typed data mutations and virtual redraw; preserve original seed evidence and outstanding structural/matching obligations."}
void BaseWin::base_editor_fac() {
    Popup popup;
    int previous = -1;
    for (;;) {
        // Original41B7DD publishes the modal kind before parsing the name.
        PopupModalState = 6;
        parse_says(0, BaseCurrent->name_string, -1, -1);
        popup.start(PopupStartCaption, "EDITORFAC", -1, 0, 0x42, 0);
        popup.set_cancel_text(label_get(0x438 / 4));
        popup.set_ok_text(label_get(0xFC0 / 4));
        for (int facility = 1; facility <= 64; ++facility) {
            StringTemp[0] = 0;
            strcat(StringTemp, StringTable.get(Facility[facility].name));
            strcat(StringTemp, "|");
            if (has_fac(facility, BaseIDCurrentSelected, 0)) {
                // Unlike the old seed, retain the facility-name prefix on
                // the already-built branch (original41B859..41B8A6).
                strcat(StringTemp, label_get(0xD50 / 4));
            } else {
                const int available = facility_avail(facility,
                    BaseCurrent->faction_id_current, BaseIDCurrentSelected, 0);
                // This clear and rebuild are in both original branches.
                StringTemp[0] = 0;
                strcat(StringTemp, StringTable.get(Facility[facility].name));
                strcat(StringTemp, "|");
                strcat(StringTemp, label_get((available ? 0x3DC : 0x4D8) / 4));
            }
            popup.dialogs().item(StringTemp, facility);
        }
        if (previous >= 0) popup.set_value(previous);
        const int selected = popup.exec(0, 0);
        PopupModalState = 0;
        if (selected < 0) return;
        bool built = false;
        if (selected < 65) {
            int offset, mask;
            bitmask(selected, &offset, &mask);
            built = (Bases[BaseIDCurrentSelected].facilities_built[offset]
                     & static_cast<unsigned char>(mask)) != 0;
        }
        // Query uses the live selected base; mutation uses this window's
        // focused ID, as the original41B99B/41B9DE separately loads them.
        set_fac(selected, focusedBaseID_, built ? 0 : 1);
        previous = selected;
        redraw(); // primary virtual slot62, not the on_redraw callback
    }
}

// ORIGINAL: 0x0041BBC0 ?base_editor_proj@BaseWin@@QAEXXZ 0x0041BBC0-0x0041C231;0x0065255E-0x006526E3
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/unrecovered/0041bbc0.cpp", "source:src/basewin.h", "image:0x41bbc0"], "reason": "Recover the full base-editor callee in its actual BaseWin owner with one genuine Popup lifetime and original cancellation, menu IDs, typed data mutations and virtual redraw; preserve original seed evidence and outstanding structural/matching obligations."}
void BaseWin::base_editor_proj() {
    Popup popup;
    PopupModalState = 6;
    popup.start(PopupStartCaption, "PICKAPROJ", -1, 0, 0x42, 0);
    // Original41BC22..41BD0E walks Facility[70..133] at48-byte stride and
    // the64 SecretProject words at4-byte stride, independently of expansion.
    for (int project_index = 0; project_index < MaxSecretProjectNum; ++project_index) {
        const int project = project_index;
        const RulesFacility &facility = Facility[FacilitySPStart + project];
        StringTemp[0] = 0;
        strcat(StringTemp, StringTable.get(facility.name));
        strcat(StringTemp, "|");
        const int owner_base = base_project(project);
        if (owner_base == -2) {
            strcat(StringTemp, label_get(0xD54 / 4));
        } else if (owner_base == -1) {
            strcat(StringTemp, label_get(has_tech(facility.preq_tech,
                           BaseCurrent->faction_id_current) ? 0x3DC / 4 : 0x4D8 / 4));
        } else {
            // Original41BCCD..41BCDD computes Base's0x134 stride, not
            // the seed's incorrect Player0x59C stride. No new index guard.
            strcat(StringTemp, Bases[owner_base].name_string);
        }
        popup.dialogs().item(StringTemp, 0);
    }
    const int project = popup.exec(0, 0);
    PopupModalState = 0;
    if (project < 0) return;
    parse_say(0, Facility[FacilitySPStart + project].name, -1, -1);
    PopupModalState = 6;
    popup.start(PopupStartCaption, "PICKAPLACE", -1, 0, 0x42, 0);
    popup.dialogs().item(label_get(0xD64 / 4), 9999);
    popup.dialogs().item(label_get(0xD68 / 4), 9998);
    // Current-faction bases precede other bases, preserving original index IDs.
    // Re-read BaseCurrent and the live base count after each item call, as
    // original41BEBA..41BEC2 does; neither is captured as immutable UI state.
    for (int pass = 0; pass < 2; ++pass) {
        for (int base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
            const int owner = Bases[base].faction_id_current;
            const bool same_owner = owner == BaseCurrent->faction_id_current;
            if ((pass == 0 && !same_owner) || (pass != 0 && same_owner)) continue;
            StringTemp[0] = 0;
            strcat(StringTemp, Bases[base].name_string);
            strcat(StringTemp, "|");
            PluralityDefault = Players[owner].is_noun_plural;
            GenderDefault = Players[owner].noun_gender;
            strcat(StringTemp, Players[owner].noun_faction);
            popup.dialogs().item(StringTemp, base);
        }
    }
    int owner_base = popup.exec(0, 0);
    PopupModalState = 0;
    if (owner_base < 0) return;
    if (owner_base >= 9000) owner_base -= 10000;
    // Use the existing typed SecretProject table owner/access convention.
    // Its current declaration is64 consecutive named int fields; base_project
    // uses this same indexed run. A future array migration must preserve all
    // named consumers centrally; this body introduces no independent storage.
    GameDataStateStorage.fields_.secret_project_[project] = owner_base;
    redraw();
    // Popup owns complete member/base teardown, including cancellation paths.
}

// ORIGINAL: 0x0041C240 ?base_editor_copy@BaseWin@@QAEXH@Z 0x0041C240-0x0041C798;0x006526E3-0x006527EF
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/unrecovered/0041c240.cpp", "source:src/basewin.h", "image:0x41c240"], "reason": "Recover the full base-editor callee in its actual BaseWin owner with one genuine Popup lifetime and original cancellation, menu IDs, typed data mutations and virtual redraw; preserve original seed evidence and outstanding structural/matching obligations."}
void BaseWin::base_editor_copy(int copy_to) {
    Popup popup;
    parse_says(0, BaseCurrent->name_string, -1, -1);
    PopupModalState = 6;
    popup.start(PopupStartCaption, copy_to ? "EDITORBASECOPYTO" : "EDITORBASECOPY",
                -1, 0, 0x42, 0);
    if (copy_to) {
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0xE64 / 4));
        popup.dialogs().item(StringTemp, 9998);
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0xE68 / 4));
        popup.dialogs().item(StringTemp, 9999);
    }
    // Own-faction bases first, then other-faction bases. Both include the
    // selected base; the original does not filter it from this menu.
    for (int group = 0; group < 2; ++group) {
        for (int base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
            const int faction = Bases[base].faction_id_current;
            if (group == 0 ? faction != BaseCurrent->faction_id_current
                           : faction == BaseCurrent->faction_id_current) continue;
            StringTemp[0] = 0;
            strcat(StringTemp, Bases[base].name_string);
            strcat(StringTemp, "|");
            PluralityDefault = Players[faction].is_noun_plural;
            GenderDefault = Players[faction].noun_gender;
            strcat(StringTemp, Players[faction].noun_faction);
            popup.dialogs().item(StringTemp, base);
        }
    }
    const int selection = popup.exec(0, 0);
    PopupModalState = 0;
    if (selection < 0) return;
    for (int byte = 0; byte < 9; ++byte) {
        if (!copy_to) {
            BaseCurrent->facilities_built[byte] = Bases[selection].facilities_built[byte];
        } else if (selection < 9998) {
            Bases[selection].facilities_built[byte] = BaseCurrent->facilities_built[byte];
        } else {
            for (int base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
                if (base == BaseIDCurrentSelected) continue;
                if (selection == 9998 && Bases[base].faction_id_current != BaseCurrent->faction_id_current) continue;
                Bases[base].facilities_built[byte] = BaseCurrent->facilities_built[byte];
            }
        }
    }
    redraw();
}

// Original interface table 0x0066A3B8 slot 14 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404250 0x00404255 ?on_iface_button_pressed@BaseWin@@UAEHH@Z 0x0041DAD0
int BaseWin::on_iface_button_pressed(int) {
    return 0;
}

// Original interface table 0x0066A3B8 slot 16 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00406B20 0x00406B23 ?on_iface_group_clicked@BaseWin@@UAEXHHH@Z 0x0041DAD0
void BaseWin::on_iface_group_clicked(int, int, int) {
}

// Original interface table 0x0066A3B8 slot 17 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404250 0x00404255 ?on_iface_dialog_back_draw@BaseWin@@UAEHPAVGraphicWin@@@Z 0x0041DAD0
int BaseWin::on_iface_dialog_back_draw(GraphicWin *) {
    return 0;
}

// Original interface table 0x0066A3B8 slot 18 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404240 0x00404245 ?on_iface_dialog_item_back_draw@BaseWin@@UAEHPAVGraphicWin@@HHPAUtagRECT@@@Z 0x0041DAD0
int BaseWin::on_iface_dialog_item_back_draw(GraphicWin *, int, int, RECT *) {
    return 0;
}

#include "stdafx.h"
#include "basewin.h"
#include "base.h"
#include "console.h"
#include "reportwin.h"
#include "reportif.h"
#include "tutwin.h"
#include "netdaemon.h"
#include "veh.h"
#include "alpha.h"
#include "game.h"
#include "faction.h"
#include "strings.h"
#include "xpops.h"

#include <cstring>


namespace {
int base_panel_signed(uint32_t value) {
    return value <= 0x7FFFFFFFu ? static_cast<int>(value)
        : -1 - static_cast<int>(0xFFFFFFFFu - value);
}
void base_panel_inset(RECT &rect, int amount) {
    rect.left += amount; rect.top += amount;
    rect.right -= amount; rect.bottom -= amount;
}
int base_native_clamp(int value, int lower, int upper) {
    // Both limits and their order are explicit in the original branches.
    if (value < lower || upper < lower) return lower;
    return value > upper ? upper : value;
}
bool base_convoy_tile(const Veh &vehicle) {
    const Map &tile = map_tiles()[vehicle.y * MapLongitude + (vehicle.x >> 1)];
    return (tile.bit & 1u) != 0 && (tile.val2 & 15) < 8;
}
void base_panel_sprite(Sprite &sprite, Buffer *canvas, int x, int y) {
    sprite.draw(canvas, sprite.transparent_index(), x, y);
}
}

// 0040B140..0040BFE3, EH cleanup 00651A78..00651A8D.
// The fourth argument is the right-button selector; the middle pair is x/y.
// ORIGINAL: 0x0040B140 ?garrison_click@BaseWin@@QAEXHHHH@Z 0x0040B140-0x0040BFE3;0x00651A78-0x00651A8D
// RECOVERY_BINDING: v1 {"evidence": ["image:0x0040B140; retired seed provenance git:1432fed4:src/unrecovered/0040b140.cpp; all original spans and historical obligations retained.", "image:41E8D0 initializes existing Font/FlatButton/Sprite/RECT and Caviar camera members; 420A70 tears down the same declaration order.", "image:41AF5B supplies ConsoleGlobal GraphicWin virtual-base receiver939444; qualified Console callback leaves the result intact.", "image:40B8F8 borrows the vehicle-ID pointer in NetDaemon row1 payload; normal menu cancel-1 reads inherited GraphicWinA0C. Undefined icon-spacing/invalid-index/modal-borrow inputs remain explicit."], "reason": "Genuine BaseWin owner with actual members, automatic base/member lifetimes and native callback receivers, not fixed executable-address forwarding or raw self-offset access."}
// TRIED: literal goto-per-branch transcription of the repeated viewport/recenter check (4 occurrences) simplified to one shared boolean form each time, not the exact per-occurrence instruction sharing (e.g. the 0x40b581/0x40bf94 cross-jump).
void BaseWin::garrison_click(int vehicle_id, int x, int y, int right) {
    PullDown menu;
    if (Vehs[vehicle_id].faction_id != ConsoleGlobal.faction_id_) return;
    if (IsMultiplayerNet && (NetDaemonNet.lock_state_[2].leading_words_[2]
        || NetDaemonNet.lock_state_[2].payload_.flags || ConsoleControlTurnActive)) return;
    if (!right && (GameDataStateStorage.fields_.game_preferences_ & 0x40000u)) {
        if (IsMultiplayerNet && veh_selectable(vehicle_id)
            && Vehs[vehicle_id].order == 0
            && !(Vehs[vehicle_id].state & 0x8004200u)) {
            ConsoleGlobal.selected_vehicle_ = vehicle_id;
            ConsoleGlobal.set_move(1);
            ConsoleGlobal.mark_garrison_selection();
            dismiss_garrison_selection();
            return;
        }
        if (!veh_selectable(vehicle_id)) return;
        if (NetDaemonNet.lock_veh(&vehicle_id, 0, -1, -1, 0)) return;
        ConsoleGlobal.selected_vehicle_ = vehicle_id;
        Vehs[vehicle_id].order = 0;
        Vehs[vehicle_id].state &= 0xFCFFBDFFu;
        synch_veh(vehicle_id);
        NetDaemonNet.await_synch();
        NetDaemonNet.unlock_veh();
        ConsoleGlobal.set_move(1);
        ConsoleGlobal.mark_garrison_selection();
        dismiss_garrison_selection();
        return;
    }

    if (TutWinBaseWindow.is_visible()) MainInterfaceGlobal.client_to_screen(&x, &y);
    else if (ReportWinGlobal.is_visible()) ReportWinGlobal.client_to_screen(&x, &y);
    if (Vehs[vehicle_id].faction_id != ConsoleGlobal.faction_id_) return;
    menu.init(0);
    const unsigned state = Vehs[vehicle_id].state;
    if (Vehs[vehicle_id].order != 0 || (state & 0x4200u)
        || (state & 0x1000200u) == 0x1000200u
        || ((state & 0x2000200u) == 0x2000200u
            && Vehs[vehicle_id].patrol_current_point < Vehs[vehicle_id].waypoint_count))
        menu.add_item(1, label_get(0xB6C / 4));
    menu.add_item(2, label_get(0x958 / 4));
    if (GameDataStateStorage.fields_.game_state_ & 0x40u) menu.add_item(90, label_get(0xDFC / 4));
    menu.add_separator(99);
    menu.add_item(10, label_get(0xB70 / 4));
    menu.add_item(11, label_get(0xB74 / 4));
    menu.add_item(12, label_get(0xB78 / 4));
    if (Vehs[vehicle_id].home_base_id >= 0
        && Vehs[vehicle_id].home_base_id != BaseIDCurrentSelected) {
        menu.add_separator(99);
        if (Bases[BaseIDCurrentSelected].faction_id_current == Vehs[vehicle_id].faction_id)
            menu.add_item(20, label_get(0xB7C / 4));
        menu.add_item(21, label_get(0xB80 / 4));
    }
    if (Vehs[vehicle_id].home_base_id == BaseIDCurrentSelected
        && (Vehs[vehicle_id].x != BaseCurrent->x || Vehs[vehicle_id].y != BaseCurrent->y)) {
        const Map &vehicle_tile = map_tiles()[Vehs[vehicle_id].y * MapLongitude
            + (Vehs[vehicle_id].x >> 1)];
        const Map &base_tile = map_tiles()[BaseCurrent->y * MapLongitude
            + (BaseCurrent->x >> 1)];
        // Original 40B85C takes SAME region directly to add-return-home.
        if (vehicle_tile.region == base_tile.region
            || Chassis[VehPrototypes[Vehs[vehicle_id].proto_id].chassis_id].triad != 0) {
            menu.add_separator(99);
            menu.add_item(30, label_get(0xB84 / 4));
        }
        menu.add_item(31, label_get(0x910 / 4));
    }
    const int previous_vehicle = vehicle_id;
    // This pointer is borrowed only; the modal callback may remap its referent.
    // The accessor uses daemon-owned +1B78, not a second global.
    NetDaemonNet.set_vehicle_selection_follow(&vehicle_id);
    menu.show(x, y, pop_expire);
    if (GameDataStateStorage.fields_.game_state_ & 0x800u) return;
    const int slot = menu.get_selected();
    const int action = slot == -1 ? base_panel_signed(menu.field_A0C_value())
        : slot < 64 ? menu.item_id(slot) : -1;
    // Native cancellation reads actual GraphicWin+A0C, not an array element:
    // PullDown.items_ startsA18, id+8=A20, minus one item(14hex)=A0C.
    // The approved narrow base getter preserves this read without array UB.
    // Other negative slots remain original unvalidated-input obligations.
    if (vehicle_id != previous_vehicle) return;
    if (NetDaemonNet.lock_veh(&vehicle_id, 0, -1, -1, 0)) return;
    const int faction = Vehs[vehicle_id].faction_id;
    if (faction != ConsoleGlobal.faction_id_) { NetDaemonNet.unlock_veh(); return; }
    switch (action) {
    case 1:
        if (veh_jail(vehicle_id)) X_pop("JAIL", 0);
        else { Vehs[vehicle_id].order = 0; synch_veh(vehicle_id); NetDaemonNet.await_synch(); }
        break;
    case 2:
        if (veh_jail(vehicle_id)) { X_pop("JAIL", 0); break; }
        Vehs[vehicle_id].order = 0;
        Vehs[vehicle_id].state &= 0xFCFFBDFFu;
        synch_veh(vehicle_id);
        NetDaemonNet.await_synch();
        ConsoleGlobal.selected_vehicle_ = vehicle_id;
        ConsoleGlobal.set_move(1);
        ConsoleGlobal.mark_garrison_selection();
        dismiss_garrison_selection();
        NetDaemonNet.unlock_veh(); // native unlock is AFTER dismissal in this arm
        return;
    case 10:
        if (veh_jail(vehicle_id)) X_pop("JAIL", 0);
        else { sleep(vehicle_id); synch_veh(vehicle_id); NetDaemonNet.await_synch(); }
        break;
    case 11:
        if (veh_jail(vehicle_id)) X_pop("JAIL", 0);
        else { Vehs[vehicle_id].order = 2; synch_veh(vehicle_id); NetDaemonNet.await_synch(); }
        break;
    case 12:
        NetDaemonNet.unlock_veh();
        ConsoleGlobal.disband(vehicle_id);
        if (TutWinBaseWindow.is_visible()) { redraw(); MainInterfaceGlobal.redraw(); }
        else if (ReportWinGlobal.is_visible()) ReportWinGlobal.redraw();
        return;
    case 20:
        if (Bases[focusedBaseID_].faction_id_current == faction) {
            Vehs[vehicle_id].home_base_id = static_cast<int16_t>(focusedBaseID_);
            synch_veh(vehicle_id); NetDaemonNet.await_synch();
        }
        break;
    case 21:
        NetDaemonNet.unlock_veh(); zoom(Vehs[vehicle_id].home_base_id, 0); return;
    case 30:
        if (veh_jail(vehicle_id)) { X_pop("JAIL", 0); break; }
        Vehs[vehicle_id].state &= 0xFCFFBDFFu;
        Vehs[vehicle_id].order = 24;
        Vehs[vehicle_id].waypoint_x[0] = Bases[Vehs[vehicle_id].home_base_id].x;
        Vehs[vehicle_id].waypoint_y[0] = Bases[Vehs[vehicle_id].home_base_id].y;
        synch_veh(vehicle_id); NetDaemonNet.await_synch();
        break;
    case 31:
        NetDaemonNet.unlock_veh(); ConsoleGlobal.set_view(0);
        // Native 40BE21 LOADS slot zero, not the separate FarmMapWindow object.
        MapWinTable[0]->set_center(Vehs[vehicle_id].x, Vehs[vehicle_id].y, 1);
        ConsoleGlobal.set_cursor(Vehs[vehicle_id].x, Vehs[vehicle_id].y);
        dismiss_garrison_selection(); return;
    case 90: ConsoleGlobal.editor_edit_veh(vehicle_id); break;
    default: break;
    }
    NetDaemonNet.unlock_veh();
    if (TutWinBaseWindow.is_visible()) {
        if (TutWinBaseWindow.is_visible()) {
            set_base(static_cast<int>(TutWinBaseWindow.focusedBaseID_));
            base_compute(1);
            TutWinBaseWindow.redraw();
            MainInterfaceGlobal.redraw();
        }
        TutWinBaseWindow.draw_garrison(1);
    } else if (ReportWinGlobal.is_visible()) ReportWinGlobal.redraw();
}

void BaseWin::dismiss_garrison_selection() {
    // Original four copies of 40B24D..40B38C. exit() already implements the
    // identical three visibility reads, zoom latch/time, iface release, draws
    // and terrain-sensitive Console recenter checks in the genuine owner.
    if (TutWinBaseWindow.is_visible()) exit();
    else if (ReportWinGlobal.is_visible()) ReportIfGlobal.done();
}

// 0040C850..0040D24C, EH cleanup 00651A8D..00651AA2.
// ORIGINAL: 0x0040C850 ?draw_supported@BaseWin@@QAEXH@Z 0x0040C850-0x0040D24C;0x00651A8D-0x00651AA2
// RECOVERY_BINDING: v1 {"evidence": ["image:0x0040C850; retired seed provenance git:1432fed4:src/unrecovered/0040c850.cpp; all original spans and historical obligations retained.", "image:41E8D0 initializes existing Font/FlatButton/Sprite/RECT and Caviar camera members; 420A70 tears down the same declaration order.", "image:41AF5B supplies ConsoleGlobal GraphicWin virtual-base receiver939444; qualified Console callback leaves the result intact.", "image:40B8F8 borrows the vehicle-ID pointer in NetDaemon row1 payload; normal menu cancel-1 reads inherited GraphicWinA0C. Undefined icon-spacing/invalid-index/modal-borrow inputs remain explicit."], "reason": "Genuine BaseWin owner with actual members, automatic base/member lifetimes and native callback receivers, not fixed executable-address forwarding or raw self-offset access."}
// TRIED: literal per-instruction register/stack transcription (eax/ebx/ecx/edx/esi/edi + s_XX stack slots + RECT groups rectA/rectB/rect28/rectScroll), goto-based control flow mirroring every branch; diverges at instr #9 (early prologue push/mov shape), not chased further given function size (2577 bytes).
void BaseWin::draw_supported(int update_mode) {
    Font font;
    Buffer *canvas = &MainInterfaceGlobal;
    RECT saved_clip = canvas->current_clip();
    RECT outer = interface_help_rect_;
    canvas->set_clip(&outer);
    canvas->box_sprite(&interface_help_rect_, &InterfaceBoxStyles[3]);
    font.init(DefaultFontFace, 10, 1);
    spot1_.kill_type(7);
    RECT title = supported_title_rect_;
    canvas->box_sprite(&title, &InterfaceBoxStyles[6]);
    base_panel_inset(title, 3);
    canvas->set_font(&font1_, 0, 0, 0);
    canvas->set_text_color(TextColourTable[82], TextColourTable[83], 1, 1);
    StringTemp[0] = 0; strcat(StringTemp, label_get(0x100 / 4)); CharUpperA(StringTemp);
    canvas->write_cent_l(StringTemp, &title, static_cast<int>(strlen(StringTemp)));
    canvas->set_font(&font, 0, 0, 0);
    RECT content = supported_content_rect_;
    canvas->box_sprite(&content, &InterfaceBoxStyles[0]);
    base_panel_inset(content, 3);
    const int faction = BaseCurrent->faction_id_current;
    const int scale = MapWinCaptionScale, divisor = MapWinCaptionDivisor;
    SpriteScaleNumerator = scale > 0 ? scale : 10;
    SpriteScaleDenominator = scale > 0 ? divisor : 16;
    const int vehicle_width = scale > 0 ? scale * 100 / divisor : 62;
    const int vehicle_height = scale > 0 ? scale * 75 / divisor : 46;
    const int icon_width = scale > 0 ? scale * 22 / divisor : 13;
    const int icon_offset = scale > 0 ? scale * 22 / divisor : 13;
    int count = 0;
    for (int i = 0; i < GameDataStateStorage.fields_.vehicle_current_count_; ++i)
        if (Vehs[i].faction_id == BaseCurrent->faction_id_current
            && Vehs[i].home_base_id == BaseIDCurrentSelected) ++count;
    const int columns = base_native_clamp((content.right - content.left - 12) / vehicle_width, 1, 3);
    const int rows = base_native_clamp((columns + count - 1) / columns, 1, 3);
    const int used_columns = count < columns ? count : columns;
    int horizontal_offset;
    if (count > columns * rows) {
        horizontal_offset = (content.right - content.left - used_columns * vehicle_width - 12) / 2;
        RECT scroll_rect = {content.right - 12, content.top, content.right, content.bottom};
        scroll_small();
        scroll2_.init(&scroll_rect, &MainInterfaceGlobal, 2, 0);
        scroll2_.set_range(0, rows - 1); scroll2_.set_pos(static_cast<int>(field_40B14_));
        scroll2_.set_paging((columns - 1) * rows); scroll2_.show(0); scroll_normal();
        field_40B20_ = 1;
    } else {
        horizontal_offset = (content.right - content.left - used_columns * vehicle_width) / 2;
        scroll2_.close(); field_40B20_ = 0;
    }
    const int start_x = content.left + horizontal_offset;
    int x = start_x;
    int y = content.top + (content.bottom - content.top - rows * vehicle_height) / 2;
    int ordinal = -1, drawn = 0, drawn_rows = 0;
    // Native EBP-50 first reads40CF5B/40D023 precede the sole write40D05C.
    // Retain that uninitialized first use and previous-row pitch thereafter;
    // initialization would be a separately reviewed behavior change.
    int icon_pitch;
    for (int vehicle_id = 0; vehicle_id < GameDataStateStorage.fields_.vehicle_current_count_; ++vehicle_id) {
        if (Vehs[vehicle_id].faction_id != BaseCurrent->faction_id_current
            || Vehs[vehicle_id].home_base_id != BaseIDCurrentSelected) continue;
        ++ordinal;
        if (field_40B20_ && (ordinal < static_cast<int>(field_40B14_) * columns
            || ordinal >= (static_cast<int>(field_40B14_) + rows) * columns)) continue;
        veh_draw(canvas, vehicle_id, x, y, -6, 72, 0);
        SpriteScaleNumerator = MapWinCaptionScale > 0 ? MapWinCaptionScale : 10;
        SpriteScaleDenominator = MapWinCaptionScale > 0 ? MapWinCaptionDivisor : 16;
        int support = 0, total = 0, convoy_yield; // native yield only read in convoy arm
        if (Vehs[vehicle_id].state & 0x10u) {
            const int pending_support = PlayersData[faction].soc_effect_pending.values_[SocialEffect::SUPPORT];
            support = has_abil(Vehs[vehicle_id].proto_id, 0x10000)
                ? 0 : 1 + (pending_support <= -4);
            total = support;
        }
        if (Vehs[vehicle_id].state & 0x600000u) {
            const int facility = has_fac_built(35, BaseIDCurrentSelected);
            total += 1 + (PlayersData[faction].soc_effect_pending.values_[SocialEffect::POLICE] + 2 * facility <= -5);
        }
        if (VehPrototypes[Vehs[vehicle_id].proto_id].plan == 10 && Vehs[vehicle_id].order == 3) {
            if (base_convoy_tile(Vehs[vehicle_id])) { convoy_yield = 1; ++total; }
            else {
                convoy_yield = resource_yield(Vehs[vehicle_id].order_auto_type, faction,
                    BaseIDCurrentSelected, Vehs[vehicle_id].x, Vehs[vehicle_id].y);
                total += convoy_yield;
            }
        }
        if (total != 0) {
            const int icon_y = y + vehicle_height - icon_offset;
            int icon_x = x;
            if (Vehs[vehicle_id].state & 0x400000u) {
                base_panel_sprite(g_PEACE_SIGN_SPRITES[0], canvas, icon_x, icon_y);
                icon_x += icon_pitch;
            } else if (Vehs[vehicle_id].state & 0x200000u) {
                // The original recomputes this live helper and police value
                // on every iteration, including the terminating test.
                for (int i = 0;; ++i) {
                    const int facility = has_fac_built(35, BaseIDCurrentSelected);
                    const int peace = 1 + (PlayersData[faction].soc_effect_pending.values_[SocialEffect::POLICE] + 2 * facility <= -5);
                    if (i >= peace) break;
                    base_panel_sprite(g_PEACE_SIGN_SPRITES[1], canvas, icon_x, icon_y);
                    icon_x += icon_pitch;
                }
            }
            icon_pitch = total > 1
                ? base_native_clamp((vehicle_width - icon_width) / (total - 1), 1, icon_width)
                : icon_width;
            for (int i = support; i > 0; --i) {
                base_panel_sprite(g_ICONS_GENERAL_SPRITES[5], canvas, icon_x, icon_y);
                icon_x += icon_pitch;
            }
            if (VehPrototypes[Vehs[vehicle_id].proto_id].plan == 10 && Vehs[vehicle_id].order == 3) {
                if (base_convoy_tile(Vehs[vehicle_id])) {
                    base_panel_sprite(g_ICONS_GENERAL_SPRITES[1 + 4 * Vehs[vehicle_id].order_auto_type],
                                      canvas, icon_x, icon_y);
                } else for (int i = convoy_yield; i > 0; --i) {
                    base_panel_sprite(g_ICONS_GENERAL_SPRITES[2 + 4 * Vehs[vehicle_id].order_auto_type],
                                      canvas, icon_x, icon_y);
                    icon_x += icon_pitch;
                }
            }
        }
        spot1_.add(vehicle_id, 7, x, y, vehicle_width, vehicle_height);
        x += vehicle_width;
        if (++drawn % columns == 0) {
            x = start_x; y += vehicle_height;
            if (++drawn_rows >= rows) break;
        }
    }
    SpriteScaleNumerator = SpriteScaleDenominator = 1;
    if (update_mode) MainInterfaceGlobal.update(&interface_help_rect_, 0);
    else MainInterfaceGlobal.soft_update(&interface_help_rect_);
    canvas->set_clip(&saved_clip);
}

// 0040FCC0..004103A7, EH cleanup00651AF8..00651B0D.
// ORIGINAL: 0x0040FCC0 ?draw_facilities@BaseWin@@QAEXH@Z 0x0040FCC0-0x004103A7;0x00651AF8-0x00651B0D
// RECOVERY_BINDING: v1 {"evidence": ["image:0x0040FCC0; retired seed provenance git:1432fed4:src/unrecovered/0040fcc0.cpp; all original spans and historical obligations retained.", "image:41E8D0 initializes existing Font/FlatButton/Sprite/RECT and Caviar camera members; 420A70 tears down the same declaration order.", "image:41AF5B supplies ConsoleGlobal GraphicWin virtual-base receiver939444; qualified Console callback leaves the result intact.", "image:40B8F8 borrows the vehicle-ID pointer in NetDaemon row1 payload; normal menu cancel-1 reads inherited GraphicWinA0C. Undefined icon-spacing/invalid-index/modal-borrow inputs remain explicit."], "reason": "Genuine BaseWin owner with actual members, automatic base/member lifetimes and native callback receivers, not fixed executable-address forwarding or raw self-offset access."}
// TRIED: automatic `Font font;` local (real ctor/dtor from the class, matching draw_psych's precedent) to get the compiler-generated SEH try-finally frame, instead of hand-rolled ExceptionList plumbing; VCall::slot001 retyped to take an int (Scroll vtable slot 1 dispatch with arg 0). Traced the real bytes (not just Ghidra, which mislabels several stack slots by a constant in the Font-adjacent region) for the rowHeight formula ((font.unk_1_>=0) ? font.height_+font.unk_1_ : font.line_height_) and the Spot::add(facId,2,left,y,width,rowHeight) arg order. Not chased to byte-exact given size (1788 bytes / SEH frame).
void BaseWin::draw_facilities(int update_mode) {
    Font font;
    Buffer *canvas = this;
    RECT saved_clip = canvas->current_clip();
    RECT outer = facilities_help_rect_;
    canvas->set_clip(&outer); spot2_.kill_type(2);
    font.init(DefaultFontFace, WinScreenWidth != 800 ? 12 : 10, 0);
    canvas->box_sprite(&facilities_help_rect_, &InterfaceBoxStyles[3]);
    StringTemp[0] = 0;
    RECT title = facilities_title_rect_;
    canvas->box_sprite(&title, &InterfaceBoxStyles[6]); base_panel_inset(title, 3);
    canvas->set_font(&font1_, 0, 0, 0);
    strcat(StringTemp, label_get(0x110 / 4));
    canvas->set_text_color(TextColourTable[82], TextColourTable[83], 1, 1);
    CharUpperA(StringTemp);
    canvas->write_cent_l(StringTemp, &title, static_cast<int>(strlen(StringTemp)));
    canvas->set_font(&font, 0, 0, 0);
    canvas->set_text_color(TextColourTable[104], TextColourTable[105], 1, 1);
    RECT content = rect2_;
    if (flatButtons_[21].is_visible()) content.bottom = rect20_.top;
    canvas->box_sprite(&content, &InterfaceBoxStyles[0]); base_panel_inset(content, 3);
    const int row_height = font.line_advance();
    int capacity = (content.bottom - content.top) / row_height;
    if (row_height * capacity + font.pixel_height() <= content.bottom - content.top) ++capacity;
    int count = 0;
    for (int facility = 0; facility < 134; ++facility) {
        bool present = false;
        if (facility < 70) {
            if (facility < 65) present = has_fac_built(facility, BaseIDCurrentSelected) != 0;
            if (!present) present = redundant(facility, BaseCurrent->faction_id_current) != 0;
        } else present = base_project(facility - 70) == BaseIDCurrentSelected;
        if (present) ++count;
    }
    if (count < capacity) scroll1_.close();
    else {
        RECT scroll_rect = {content.right - 12, content.top, content.right, content.bottom};
        scroll_small(); scroll1_.init(&scroll_rect, this, 1, 0);
        scroll1_.set_range(0, count - 1); scroll1_.set_pos(static_cast<int>(field_40B10_));
        scroll1_.set_paging(capacity - 1); scroll1_.show(0); scroll_normal();
    }
    int ordinal = -1, y = content.top;
    for (int row = 0; row < 134; ++row) {
        const int facility = row == 0 ? 26 : row == 26 ? 0 : row;
        bool present = false;
        if (row < 70) {
            if (facility < 65) present = has_fac_built(facility, BaseIDCurrentSelected) != 0;
            if (!present) present = redundant(facility, BaseCurrent->faction_id_current) != 0;
        } else present = base_project(row - 70) == BaseIDCurrentSelected;
        if (!present) continue;
        ++ordinal;
        if (ordinal < static_cast<int>(field_40B10_)
            || ordinal >= static_cast<int>(field_40B10_) + capacity) continue;
        StringTemp[0] = 0;
        if (redundant(facility, BaseCurrent->faction_id_current)) {
            if (facility < 65 && has_fac_built(facility, BaseIDCurrentSelected)) strcat(StringTemp, "*");
            canvas->set_text_color(TextColourTable[124], TextColourTable[125], 1, 1);
        } else canvas->set_text_color(TextColourTable[122], TextColourTable[123], 1, 1);
        strcat(StringTemp, StringTable.get(Facility[facility].name));
        if (facility == 8 && (BaseCurrent->state & 0x400u)) {
            strcat(StringTemp, " "); strcat(StringTemp, "(");
            strcat(StringTemp, label_get(0x3A0 / 4)); strcat(StringTemp, ")");
        }
        const int width = content.right - content.left;
        while (font.width(StringTemp) > width) {
            const size_t length = strlen(StringTemp);
            if (!length) break;
            char *last_space = strrchr(StringTemp, ' ');
            if (last_space) *last_space = 0;
            else StringTemp[length - 1] = 0;
        }
        canvas->write_l(StringTemp, content.left + 1, y, static_cast<int>(strlen(StringTemp)));
        spot2_.add(facility, 2, content.left, y, width, row_height);
        y += row_height;
    }
    if (update_mode) update(&facilities_help_rect_, 0);
    else soft_update(&facilities_help_rect_);
    canvas->set_clip(&saved_clip);
}

// 0041AF50..0041AF69. Native immediate939444 is ConsoleGlobal's real
// GraphicWin virtual base (+23D94), NOT an executable-address pointer load.
// ORIGINAL: 0x0041AF50 ?on_key_click@BaseWin@@QAEXHH@Z 0x0041AF50-0x0041AF69 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x0041AF50; retired seed provenance git:1432fed4:src/recovered/0041af50.cpp; all original spans and historical obligations retained.", "image:41E8D0 initializes existing Font/FlatButton/Sprite/RECT and Caviar camera members; 420A70 tears down the same declaration order.", "image:41AF5B supplies ConsoleGlobal GraphicWin virtual-base receiver939444; qualified Console callback leaves the result intact.", "image:40B8F8 borrows the vehicle-ID pointer in NetDaemon row1 payload; normal menu cancel-1 reads inherited GraphicWinA0C. Undefined icon-spacing/invalid-index/modal-borrow inputs remain explicit."], "reason": "Genuine BaseWin owner with actual members, automatic base/member lifetimes and native callback receivers, not fixed executable-address forwarding or raw self-offset access."}
// symbol    ?on_key_click@BaseWin@@UAEHHH@Z
int BaseWin::on_key_click(int repeat, int key) {
    return ConsoleGlobal.Console::on_key_click(repeat, key);
}

// 00420A70..00420F8E plus original cleanup/virtual-base teardown regions.
// All member/base destructors execute implicitly in reverse declaration order.
// ORIGINAL: 0x00420A70 ??1BaseWin@@QAE@XZ 0x00420A70-0x00420F8E;0x004210D0-0x004210E7;0x00650E20-0x00651184
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00420A70; retired seed provenance git:1432fed4:src/unrecovered/00420a70.cpp; all original spans and historical obligations retained.", "image:41E8D0 initializes existing Font/FlatButton/Sprite/RECT and Caviar camera members; 420A70 tears down the same declaration order.", "image:41AF5B supplies ConsoleGlobal GraphicWin virtual-base receiver939444; qualified Console callback leaves the result intact.", "image:40B8F8 borrows the vehicle-ID pointer in NetDaemon row1 payload; normal menu cancel-1 reads inherited GraphicWinA0C. Undefined icon-spacing/invalid-index/modal-borrow inputs remain explicit."], "reason": "Genuine BaseWin owner with actual members, automatic base/member lifetimes and native callback receivers, not fixed executable-address forwarding or raw self-offset access."}
// symbol    ??1BaseWin@@UAE@XZ
BaseWin::~BaseWin() {}

// Original init: 0041E8D0..00420A3D; cleanup funclet 00652A94..00652AA9.
#include "stdafx.h"
#include "basewin.h"
#include "console.h"
#include "wave.h"
#include "wave_device.h"
#include "text.h"
#include "alpha.h"
#include "strings.h"

// The earlier declaration and definition own this Wave's single lifetime.
// A post-definition extern makes VC6 register its construction again.
extern int InvertEntryCount;
extern char InvertEntries[8][32];

namespace {
void base_init_rect(RECT &rect, int left, int top, int right, int bottom) {
    rect.left = left; rect.top = top; rect.right = right; rect.bottom = bottom;
}
void base_init_inset(RECT &rect, int amount) {
    rect.left += amount; rect.top += amount;
    rect.right -= amount; rect.bottom -= amount;
}
void base_init_label(int label_offset, bool upper) {
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(label_offset / 4));
    if (upper) CharUpperA(StringTemp);
}
void base_init_button(FlatButton &button, const char *caption, int id,
                      const RECT &rect, Win *parent) {
    button.init(caption, id, rect.left, rect.top,
                rect.right - rect.left, rect.bottom - rect.top, parent, 0);
}
// Keep the three distinct borrowed runs distinct; these are existing config
// rows, not vtables or ownership transfers. All nine pointers are Sprite*.
void base_init_button_slices(FlatButton &button, int sprite, int colours) {
    button.set_slice_icons(&g_BASEWIN_SPRITES[sprite],
                          &g_BASEWIN_SPRITES[sprite + 3],
                          &g_BASEWIN_SPRITES[sprite + 6],
                          &g_BASEWIN_SPRITES[sprite + 1],
                          &g_BASEWIN_SPRITES[sprite + 4],
                          &g_BASEWIN_SPRITES[sprite + 7],
                          &g_BASEWIN_SPRITES[sprite + 2],
                          &g_BASEWIN_SPRITES[sprite + 5],
                          &g_BASEWIN_SPRITES[sprite + 8]);
    button.set_slice_colours(TextColourTable[colours],
                            TextColourTable[colours + 1],
                            TextColourTable[colours + 2]);
}
}

// ORIGINAL: 0x0041E8D0 ?init@BaseWin@@QAEXXZ 0x0041E8D0-0x00420A3D;0x00652A94-0x00652AA9
// RECOVERY_BINDING: v1 {"evidence": ["image:0x0041E8D0; retired seed provenance git:1432fed4:src/unrecovered/0041e8d0.cpp; all original spans and historical obligations retained.", "image:41E8D0 initializes existing Font/FlatButton/Sprite/RECT and Caviar camera members; 420A70 tears down the same declaration order.", "image:41AF5B supplies ConsoleGlobal GraphicWin virtual-base receiver939444; qualified Console callback leaves the result intact.", "image:40B8F8 borrows the vehicle-ID pointer in NetDaemon row1 payload; normal menu cancel-1 reads inherited GraphicWinA0C. Undefined icon-spacing/invalid-index/modal-borrow inputs remain explicit."], "reason": "Genuine BaseWin owner with actual members, automatic base/member lifetimes and native callback receivers, not fixed executable-address forwarding or raw self-offset access."}
// TRIED: sim 0.70 (framed /O2 /Oy-), first divergence #9 push-vs-mov in the prologue/Wave-init region; every field store, BaseButton/Sprite/Buffer /Caviar/Time/Spot/Font/Wave call and its receiver was cross-checked against the raw disassembly (not Ghidra's dropped-this renderings), including the trailing 22x1 BaseButton array and the two vtable slot002/slot062 dispatch loops; RECT/float scratch temporaries use fresh locals rather than the original's exact reused stack slots.
// TRIED: binding fates. This body IS a full transcription, so it is the one of the four a rename can home; 66 of the 72 bindings resolve to names the tree already carries or to literals, and only the ten below need deciding. Reading them is the remaining work: ALREADY NAMED ELSEWHERE 0x0066931C game.h's function-pointer slot;  0x00669328 console.h's runtime slot;  0x0066AA40 the float -10000.0f (basewin.h documents it) 0x00682824 "BASENAME";  0x00682830 "INVERTLIST";  0x00682E60 "fx\\menu2.wav";  0x00682E70 "ter1wreck.pcx";  0x00682E80 "interface.pcx"  - .rdata literals, spelled as literals 0x00691B2C DefaultFontFace (font.h);  0x006A7528 InvertEntryTable and 0x006EEE60 InvertEntryCount (basewin.cpp);  0x006EEE68 g_BASEWIN_WAVE 0x0077A5C8..0x0077A8B4  Sprite g_BASEWIN_SPRITES[0..16] - sprite.cpp names the array at 0x77A5C8 with a 0x2C stride, so these seventeen are indexes into it, not seventeen bindings 0x0078D5A0 BoxSpriteStyle3;  0x0078D690 BoxSpriteStyle9 (dipedit.cpp); 0x007ACBB8 g_RAINFALL_DOUBLE_TILE_SPRITES[0] (sprite.cpp) 0x007AE820 MainInterfaceGlobal;  0x009156B0 ConsoleGlobal; 0x009B7B1C / 0x009B7B20 WinScreenWidth / WinScreenHeight; 0x009B7D00 TextBufferGetPtr;  0x009B86A0 the string temp; 0x009B90D8 StringTable;  0x009B90F8 Labels; 0x00689370 BaseIDCurrentSelected;  0x0090EA30 BaseCurrent() 0x0041AF40 BaseWin::timer_callback's own address - a function reference wearing a data binding, so it needs the callee declared, not a name 0x00652A9F and the 0x00652A94-0x00652AA9 span - the EH scope table, the compiler's STILL UNNAMED, with what the body does with each 0x006734E0  0x00696D14  0x0078D708  0x007ACBC0  0x007AE7FC 0x007AF59C..0x007AF5DC (a run of 17 dwords whose +0x14 dword, 0x7AF5B0, is the RECT BaseWin::init insets into its own rects - see BattleWin::on_status's block in battlewin.cpp, which names 0x7AF5B0 InterfaceScreenRect and proves the four-dword read) 0x008C6C54..0x008C6C68  0x008C6D1C / 0x008C6D20  0x0090D978
void BaseWin::init() {
    int index;
    Buffer assets;  // Real local lifetime, including exceptional cleanup.
    RECT area;
    int window_height;

    field_40B00_ = -1;
    g_BASEWIN_WAVE.load("fx\\menu2.wav", 16);
    WaveDeviceGlobal.add_to_group(1, &g_BASEWIN_WAVE);
    g_BASEWIN_WAVE.set_volume(65);
    g_BASEWIN_WAVE.set_pitch(300);
    InterfaceRegistry[3] = static_cast<SubInterface *>(this);
    const bool expanded = WinScreenWidth != 800;
    SubInterface::field_4_ = 3;

    // Both original resolution paths initialize these same rectangles.
    base_init_rect(rect1_, 166, 33, 560, 257);
    base_init_rect(commerce_report_rect_, 30, 170, 160, 293);
    base_init_rect(commerce_title_rect_, 30, 149, 160, 169);
    base_init_rect(commerce_outer_rect_, 27, 146, 163, 296);
    base_init_rect(rect2_, 566, 57, 696, 254);
    base_init_rect(facilities_title_rect_, 566, 36, 696, 56);
    base_init_rect(facilities_help_rect_, 563, 33, 699, 257);
    base_init_rect(nutrient_content_rect_, 30, 57, 160, 140);
    base_init_rect(nutrient_title_rect_, 30, 36, 160, 56);
    base_init_rect(nutrient_outer_rect_, 27, 33, 163, 143);
    base_init_rect(production_outer_rect_, 166, 260, 699, 349);
    production_content_rect_ = production_outer_rect_;
    if (!expanded) {
        base_init_rect(rect7_, 191, 354, 369, 374);
        base_init_rect(rect6_, 373, 354, 551, 374);
        window_height = WinScreenHeight - MainInterfaceGlobal.field_D7C_;
        if (window_height > 398) window_height = 398;
    } else {
        const int half_old_height =
            (production_content_rect_.bottom - production_content_rect_.top) / 2;
        const int shift = production_content_rect_.bottom -
                          production_content_rect_.top + 3;
        production_content_rect_.top += shift;
        production_content_rect_.bottom += shift;
        const int half_new_height =
            (production_content_rect_.bottom - production_content_rect_.top) / 2;
        commerce_report_rect_.top += half_new_height;
        commerce_report_rect_.bottom += half_new_height;
        commerce_outer_rect_.top += half_old_height;
        commerce_outer_rect_.bottom += half_old_height;
        commerce_title_rect_.top += half_old_height;
        commerce_title_rect_.bottom += half_old_height;
        commerce_report_rect_.bottom += half_old_height;
        commerce_outer_rect_.bottom += half_old_height;
        nutrient_content_rect_.bottom += half_old_height;
        nutrient_outer_rect_.bottom += half_old_height;
        const int production_height =
            production_content_rect_.bottom - production_content_rect_.top;
        base_init_rect(rect6_, 381, production_height + 360,
                       619, production_height + 380);
        base_init_rect(rect7_, 131, production_height + 360,
                       369, production_height + 380);
        window_height = production_height + 401;
    }
    base_init_rect(rect5_, commerce_outer_rect_.left,
                   commerce_outer_rect_.bottom + 3,
                   commerce_outer_rect_.right, rect6_.top - 6);
    field_40B2C_ = 0;
    field_40B30_ = 0;
    garrison_rect_ = MainInterfaceGlobal.lower_frame_rect_;
    base_init_inset(garrison_rect_, 4);
    garrison_rect_.left -= 4;  // Original removes only the left inset again.
    queue_help_rect_ = MainInterfaceGlobal.rect4_;
    queue_help_rect_.right = (queue_help_rect_.right - queue_help_rect_.left) * 7 / 12;
    base_init_inset(queue_help_rect_, 4);
    base_init_rect(flash_rect_b_, queue_help_rect_.right + 3,
                   queue_help_rect_.bottom - 23,
                   garrison_rect_.left - 7, queue_help_rect_.bottom - 3);
    base_init_rect(flash_rect_a_, queue_help_rect_.right + 3,
                   queue_help_rect_.bottom - 45,
                   garrison_rect_.left - 7, queue_help_rect_.bottom - 25);
    const RECT &main_frame = MainInterfaceGlobal.rect4_;
    base_init_rect(queue_outer_rect_, queue_help_rect_.right + 4,
                   main_frame.top + 4, main_frame.right - 4,
                   flash_rect_a_.top - 7);
    base_init_rect(queue_title_rect_, queue_help_rect_.right + 7,
                   main_frame.top + 7, main_frame.right - 7,
                   flash_rect_a_.top - 10);
    queue_title_rect_.bottom = queue_title_rect_.top + 20;
    base_init_rect(queue_content_rect_, queue_help_rect_.right + 7,
                   main_frame.top + 7, main_frame.right - 7,
                   flash_rect_a_.top - 10);
    queue_content_rect_.top = queue_title_rect_.bottom + 2;
    queue_button_rect_ = queue_title_rect_;
    const RECT &lower_frame = MainInterfaceGlobal.info_panel_frame_;
    base_init_rect(population_rect_, lower_frame.left + 4,
                   lower_frame.bottom - 58, lower_frame.right - 62,
                   lower_frame.bottom - 4);
    info_title_rect_ = lower_frame;
    info_title_rect_.bottom = population_rect_.top;
    base_init_inset(info_title_rect_, 4);
    ++info_title_rect_.bottom;
    base_init_rect(rect12_, population_rect_.right + 7,
                   population_rect_.top + 3, lower_frame.right - 7,
                   population_rect_.bottom - 3);
    const RECT &right_frame = MainInterfaceGlobal.right_frame_rect_;
    interface_help_rect_ = right_frame;
    base_init_inset(interface_help_rect_, 4);
    base_init_rect(supported_title_rect_, right_frame.left + 7,
                   right_frame.top + 7, right_frame.right - 7,
                   right_frame.bottom - 7);
    supported_title_rect_.bottom = supported_title_rect_.top + 20;
    base_init_rect(supported_content_rect_, right_frame.left + 7,
                   supported_title_rect_.bottom + 2, right_frame.right - 7,
                   right_frame.bottom - 7);
    base_init_rect(rect20_, rect2_.left + 3, rect2_.bottom - 23,
                   rect2_.right - 3, rect2_.bottom - 3);

    font1_.init(DefaultFontFace, 14, 1);
    if (text_open("BASENAME", "INVERTLIST") == 0) {
        text_get();
        InvertEntryCount = text_item_number();
        for (index = 0; index < InvertEntryCount; ++index) {
            text_get();
            strcpy(InvertEntries[index], TextBufferGetPtr);
        }
        text_close();
    }
    buffer2_.init(100, 50, 0, 0);
    g_RAINFALL_DOUBLE_TILE_SPRITES[0].draw(&buffer2_,
        g_RAINFALL_DOUBLE_TILE_SPRITES[0].transparent_index(), 0, 0, 1, 1);
    field_40B1C_ = 0xFFFFFFFFU;
    field_40B20_ = 0xFFFFFFFFU;
    field_40B24_ = 0xFFFFFFFFU;
    field_40B54_ = -1;
    field_40B58_ = 223;
    field_40AFC_ = 0;
    field_40AF4_ = 0;
    field_40AF8_ = 0;
    field_40B40_ = 0;
    spot2_.init(200);
    spot1_.init(200);
    caviar_.init();
    caviar_.set_field_A4(0);
    camera_angles_.values[0] = 0.0f;
    camera_angles_.values[1] = 0.0f;
    camera_angles_.values[2] = 0.785f;
    caviar_apply_rotation(camera_angles_.values, &camera_rotation_);
    camera_rotation_.values[2][0] = -camera_rotation_.values[2][0];
    camera_rotation_.values[2][1] = -camera_rotation_.values[2][1];
    camera_rotation_.values[2][2] = -camera_rotation_.values[2][2];
    VOX_Vect camera;
    camera.values[0] = camera_rotation_.values[2][0] * -10000.0f;
    camera.values[1] = camera_rotation_.values[2][1] * -10000.0f;
    camera.values[2] = camera_rotation_.values[2][2] * -10000.0f;
    caviar_.set_camera_direct(&camera, &camera_rotation_);
    caviar_.set_scaling(0.18f);
    caviar_.shadow_vector().values[0] = 0.0f;
    caviar_.shadow_vector().values[1] = 0.0f;
    caviar_.shadow_vector().values[2] = 1.0f;
    caviar_.light_vector().values[0] = 0.0f;
    caviar_.light_vector().values[1] = 0.0f;
    caviar_.light_vector().values[2] = 1.0f;
    time_.init(BaseWin::timer_callback, 0, this, 50, 5);
    GraphicWin::init((WinScreenWidth - 726) / 2,
        WinScreenHeight - MainInterfaceGlobal.field_D7C_ - window_height,
        758, window_height, 0, 0x05000000,
        static_cast<Win *>(&ConsoleGlobal), 0, 0);
    GraphicWin::fill(9);
    area = *buffer_working_rect();
    area.left += 23; area.right -= 55; area.bottom += 3;
    Buffer::box_sprite(&area, &InterfaceBoxStyles[9]);
    base_init_rect(area, 27, 4, 699, 30);
    Buffer::box_sprite(&area, &InterfaceBoxStyles[12]);
    UnionRect(&area, &rect6_, &rect7_);
    base_init_inset(area, -3);
    Buffer::box_sprite(&area, &InterfaceBoxStyles[3]);

    assets.load_pcx("ter1wreck.pcx", 0, 10, 236);
    for (index = 0; index < 3; ++index)
        sprites7_[index].extract(&assets, WinFillColour, 768,
                                1 + index * 49, 48, 48, 0);
    assets.load_pcx("interface.pcx", 0, 10, 236);
    for (index = 0; index < 3; ++index)
        sprites8_[index].extract(&assets, WinFillColour, 247,
                                1 + index * 21, 20, 20, 0);
    for (index = 0; index < 3; ++index)
        sprites8_[3 + index].extract(&assets, WinFillColour, 268,
                                1 + index * 21, 21, 20, 0);
    for (index = 0; index < 3; ++index)
        sprites8_[6 + index].extract(&assets, WinFillColour, 290,
                                64 + index * 36, 20, 35, 0);
    for (index = 0; index < 3; ++index)
        sprites8_[9 + index].extract(&assets, WinFillColour, 311,
                                64 + index * 36, 20, 35, 0);
    for (index = 0; index < 3; ++index)
        sprites8_[12 + index].extract(&assets, WinFillColour, 332,
                                1 + index * 49, 48, 48, 0);
    sprites1_[0].extract(&assets, WinFillColour, 397, 1, 20, 20, 0);
    sprites1_[1].extract(&assets, WinFillColour, 418, 1, 20, 20, 0);
    sprites3_[0].extract(&assets, WinFillColour, 397, 22, 20, 20, 0);
    sprites3_[1].extract(&assets, WinFillColour, 418, 22, 20, 20, 0);
    sprites2_[0].extract(&assets, WinFillColour, 397, 43, 20, 20, 0);
    sprites2_[1].extract(&assets, WinFillColour, 418, 43, 20, 20, 0);
    sprite4_.extract(&assets, WinFillColour, 397, 64, 20, 20, 0);
    sprite5_.extract(&assets, WinFillColour, 418, 64, 20, 20, 0);
    sprite6_.extract(&assets, WinFillColour, 439, 64, 20, 20, 0);

    base_init_label(0x77C, true);
    base_init_button(flatButtons_[20], StringTemp, 20, queue_button_rect_, &MainInterfaceGlobal);
    flatButtons_[20].set_bubble_text(label_get(0xC30 / 4));
    base_init_label(0x108, false);
    base_init_rect(previous_button_rect_, info_title_rect_.left + 3,
        info_title_rect_.top + 3,
        info_title_rect_.left + 3 + sprites8_[6].width(),
        info_title_rect_.top + 3 + sprites8_[6].height());
    base_init_button(flatButtons_[2], 0, 2, previous_button_rect_, &MainInterfaceGlobal);
    flatButtons_[2].set_report_icons(&sprites8_[6], &sprites8_[7], &sprites8_[8]);
    flatButtons_[2].set_bubble_text(StringTemp);
    base_init_label(0x104, false);
    next_button_rect_ = previous_button_rect_;
    next_button_rect_.right = info_title_rect_.right - 3;
    next_button_rect_.left = next_button_rect_.right - sprites8_[6].width();
    base_init_button(flatButtons_[3], 0, 3, next_button_rect_, &MainInterfaceGlobal);
    flatButtons_[3].set_report_icons(&sprites8_[9], &sprites8_[10], &sprites8_[11]);
    flatButtons_[3].set_field_A14(1);
    flatButtons_[3].set_bubble_text(StringTemp);
    base_init_label(0x194, true);
    base_init_button(flatButtons_[0], StringTemp, 0, rect6_, this);
    base_init_label(0x1F4, true);
    base_init_button(flatButtons_[1], StringTemp, 1, rect7_, this);
    base_init_label(0x814, true);
    base_init_button(flatButtons_[12], StringTemp, 12, flash_rect_a_, &MainInterfaceGlobal);
    flatButtons_[12].set_bubble_text(label_get(0xC28 / 4));
    base_init_label(0xAC, true);
    base_init_button(flatButtons_[13], StringTemp, 13, flash_rect_b_, &MainInterfaceGlobal);
    flatButtons_[13].set_bubble_text(label_get(0xC2C / 4));
    base_init_label(0x4FC, true);
    base_init_rect(rect10_, 169, 234, 297, 254);
    base_init_button(flatButtons_[10], StringTemp, 10, rect10_, this);
    flatButtons_[10].set_bubble_text(label_get(0xC1C / 4));
    base_init_label(0xD4, true);
    base_init_rect(rect9_, 299, 234, 427, 254);
    base_init_button(flatButtons_[9], StringTemp, 9, rect9_, this);
    flatButtons_[9].set_bubble_text(label_get(0xC20 / 4));
    base_init_label(0x4F8, true);
    base_init_rect(rect11_, 429, 234, 557, 254);
    base_init_button(flatButtons_[11], StringTemp, 11, rect11_, this);
    flatButtons_[11].set_bubble_text(label_get(0xC24 / 4));
    base_init_label(0x824, true);
    base_init_rect(rect14_, 30, 7, 142, 27);
    base_init_button(flatButtons_[4], StringTemp, 4, rect14_, this);
    base_init_button_slices(flatButtons_[4], 9, 62);
    flatButtons_[4].set_bubble_text(label_get(0xC08 / 4));
    base_init_label(0x828, true);
    base_init_rect(rect15_, 144, 7, 256, 27);
    base_init_button(flatButtons_[5], StringTemp, 5, rect15_, this);
    base_init_button_slices(flatButtons_[5], 9, 62);
    flatButtons_[5].set_bubble_text(label_get(0xC0C / 4));
    base_init_label(0xA54, true);
    base_init_rect(rect16_, 258, 7, 258 + sprites8_[0].width(), 7 + sprites8_[0].height());
    base_init_button(flatButtons_[18], 0, 18, rect16_, this);
    flatButtons_[18].set_report_icons(&sprites8_[0], &sprites8_[1], &sprites8_[2]);
    flatButtons_[18].set_bubble_text(StringTemp);
    flatButtons_[18].set_slice_colours(TextColourTable[65], TextColourTable[66], TextColourTable[67]);
    base_init_label(0x724, true);
    base_init_rect(rect8_, 280, 7, 446, 27);
    base_init_button(flatButtons_[8], StringTemp, 8, rect8_, this);
    base_init_button_slices(flatButtons_[8], 0, 65);
    flatButtons_[8].set_bubble_text(label_get(0xC18 / 4));
    base_init_label(0xA54, true);
    base_init_rect(rect17_, 448, 7, 448 + sprites8_[0].width(), 7 + sprites8_[0].height());
    base_init_button(flatButtons_[19], 0, 19, rect17_, this);
    flatButtons_[19].set_report_icons(&sprites8_[0], &sprites8_[1], &sprites8_[2]);
    flatButtons_[19].set_bubble_text(StringTemp);
    flatButtons_[19].set_slice_colours(TextColourTable[65], TextColourTable[66], TextColourTable[67]);
    base_init_label(0x82C, true);
    base_init_rect(rect18_, 470, 7, 582, 27);
    base_init_button(flatButtons_[6], StringTemp, 6, rect18_, this);
    base_init_button_slices(flatButtons_[6], 9, 62);
    flatButtons_[6].set_bubble_text(label_get(0xC10 / 4));
    base_init_label(0x830, true);
    base_init_rect(rect19_, 584, 7, 696, 27);
    base_init_button(flatButtons_[7], StringTemp, 7, rect19_, this);
    base_init_button_slices(flatButtons_[7], 9, 62);
    flatButtons_[7].set_bubble_text(label_get(0xC14 / 4));
    base_init_label(0xA58, true);
    base_init_button(flatButtons_[16], 0, 16, rect12_, &MainInterfaceGlobal);
    flatButtons_[16].set_report_icons(&sprites8_[12], &sprites8_[13], &sprites8_[14]);
    flatButtons_[16].set_bubble_text(StringTemp);
    base_init_label(0xA58, true);
    base_init_button(flatButton_, 0, 16, rect12_, &MainInterfaceGlobal);
    flatButton_.set_report_icons(&sprites7_[0], &sprites7_[1], &sprites7_[2]);
    flatButton_.set_bubble_text(StringTemp);
    base_init_label(0xD38, true);
    base_init_button(flatButtons_[21], StringTemp, 21, rect20_, this);
    flatButtons_[21].set_bubble_text(label_get(0xD3C / 4));
    if (!expanded) {
        base_init_label(0xA4C, true);
        base_init_rect(rect13_, 169, 263, 169 + sprites8_[3].width(), 263 + sprites8_[3].height());
        base_init_button(flatButtons_[17], 0, 17, rect13_, this);
        flatButtons_[17].set_report_icons(&sprites8_[3], &sprites8_[4], &sprites8_[5]);
        flatButtons_[17].set_bubble_text(StringTemp);
    }
    // Includes untouched controls14/15 and control17 on expanded displays.
    // Preserve both virtual redraw calls; the first is not interchangeable
    // with the second when a derived painter has stateful behavior.
    for (index = 0; index < 22; ++index) {
        flatButtons_[index].hide();
        static_cast<Buffer *>(&flatButtons_[index])->set_font(&font1_, 0, 0, 0);
        flatButtons_[index].redraw();
        flatButtons_[index].redraw();
    }
    flatButton_.hide();
    static_cast<Buffer *>(&flatButton_)->set_font(&font1_, 0, 0, 0);
    flatButton_.redraw();
    flatButton_.redraw();
}



// Farm renderer: corrected native receivers, array strides and explicit scale
// overloads; no original executable addresses remain in the operation.

/*
Purpose: ?on_scrolling@BaseWin@@QAEXHH@Z - dispatch to slot 0xc0 of this object's OWN
         vtable, forwarding 2 argument(s).
// ORIGINAL: 0x0041B4C0 ?on_scrolling@BaseWin@@QAEXHH@Z 0x0041B4C0-0x0041B4D7 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned original own table 0x0066A40C, pointer word at +0xC4, identifies original entry 0x0041B4C0", "source:src/win.h inherited virtual interface; actual emitted method ?vslot_49@BaseWin@@UAEXHH@Z in primary Win table ??_7BaseWin@@6BWin@@@"], "reason": "The original own Win table binds this entry as virtual slot 49. Use genuine member dispatch, not an opaque redirect; retain the catalogued original spelling as historical evidence."}
// symbol    ?on_scrolling@BaseWin@@UAEXHH@Z
// size      23 bytes
// prototype void (__thiscall ?on_scrolling@BaseWin@@QAEXHH@Z)(BaseWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// indirect  0x0041B4CD
Return Value: n/a
Status: Complete
*/
void BaseWin::on_scrolling(int code, int position) {
  on_scrolled(code, position);
}

/*
Purpose: Map a click on the base screen's farm panel back to the tile it
         picks: unproject the 0x60-by-0x30 half-scale sprite pixel through
         rect1_'s placement and report the move offset of that radius.
// ORIGINAL: 0x00409100 ?farm_get@BaseWin@@QAEHHH@Z 0x00409100-0x004091FB FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned original disassembly 0x00409100-0x004091FB: pixel result is adjusted by 10, divided by 16, and for nonzero index loads from 0x0066EF24 + 4*idx and 0x0066EF38 + 4*idx before calling radius_move(…, …, 0x15)", "source:src/buffer.cpp Buffer::load_pcx(BYTE*,DWORD,Palette*,int,int) adds colour_base to each decoded pixel and truncates to uint8_t; src/game.cpp load_terrain uses RAINFALL.PCX with base 10", "source:src/game.cpp terrain_extract_sprite recolors 262/263 to 265 and extracts x=1,y=52,100x100; source:src/sprite.cpp Sprite::draw uses unit-scale clipping, and BaseWin::init draws it into buffer2_.init(100,50,0,0)", "asset:Rainfall.pcx sha256 03b87d7873f5a9419d8c4ddd6c5a8bd333fe81a6b3ce4730056f129dcb94d364; decoded region [1,101)x[52,102) has raw values 3,20,42,56,69"], "reason": "The pinned installed asset’s ordinary initialization path is source owned: Buffer::load_pcx shifts each rainfall PCX pixel by colour_base 10 (with byte truncation); terrain_extract_sprite changes only source colors 262 and 263, then extracts x=1,y=52,width=100,height=100; BaseWin copies at unit scale into buffer2_ 100x50, whose native clip limits reads to source rows 52..101. The pinned Rainfall.pcx SHA-256 is 03b87d7873f5a9419d8c4ddd6c5a8bd333fe81a6b3ce4730056f129dcb94d364; decoding that window gives only raw values {3,20,42,56,69}, so the actual pixel argument values become {13,30,52,66,79} and the original index expression yields {0,1,2,3,4}. Keep the real RiverStepX/Y arrays and original unguarded accesses. This supports the pinned asset path; modified or corrupt assets remain subject to the original unchecked indexing and are not covered by the asset observation."}
// TRIED: the two field reads (0x40b8c, 0x40b94) as named int locals - compiles with an extra `push`/register spill the original (which keeps the value live in a register, never re-materialised) doesn't have. `get_pixel` called through `reinterpret_cast<Buffer*>(this)` since `ecx` (this) is never reassigned before that call in the original either. `extern int g_66ef24[]/g_66ef38[]` for the two lookup tables so indexing compiles as scaled-index addressing instead of folding the base.
// SUPERSEDED_BINDING_HISTORY: v1 {"evidence": ["git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/unrecovered/00409100.cpp"], "reason": "Retain the existing source implementation and complete original mapping. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
// size      251 bytes
Status: Coverage transcription of the retained seed
        (git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/unrecovered/00409100.cpp, plain FILE). The three rect reads
        are the typed rect1_ members (right 0x40B94, left 0x40B8C, top
        0x40B90). Two known divergences from the image, both structural:
        the image CALLS 0x005A65A0 (radius_move) here while the tree's
        radius_move is MEASURED inline in map.h with no out-of-line body,
        so this body can only expand it; and 0x0066EF24/0x0066EF38 are
        unnamed per-radius offset tables, spelled as raw const-int
        indexing like every other unnamed datum in this file.
*/
int BaseWin::farm_get(int a1, int a2) {
    const int right = rect1_.right;
    const int left = rect1_.left;
    const int top = rect1_.top;

    int y = a1 + ((0xc0 - (right - left) / 2) - left);
    if (y >= 0 && y < 0x180) {
        int x = a2 - top;
        if (x >= 0 && x < 0xc0) {
            int col = (y / 0x60) * 2 - 3;
            int row = (x / 0x30) * 2 - 3;
            int p = buffer2_.get_pixel(y % 0x60,
                            x % 0x30);
            int idx = (p - 10) / 16;
            if (idx != 0) {
                return radius_move(
                    col + RiverStepX[idx - 1],
                    row + RiverStepY[idx - 1],
                    0x15);
            }
            return radius_move(col, row, 0x15);
        }
    }
    return -1;
}
