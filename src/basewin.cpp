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
#include "fx.h"
#include "game.h"
#include "random.h"
#include "log.h"
#include "popmenu.h"
#include "infowin.h"
#include "messagewin.h"
#include "statuswin.h"
#include "worldwin.h"
#include <mmsystem.h>
#include <cstdint>
#include <cstring>

// Existing shared text.pcx palette publication, owned by palette.cpp.
extern int TextColourTable[195];
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
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/0040a1d0.cpp", "source:src/basewin.h", "image:0x40a1d0"], "reason": "Recover the complete original operation through genuine owning C++ classes, members and natural lifetimes; preserve retained seed evidence and historical restoration obligations separately from fresh matching."}
void BaseWin::farm_click(int x, int y, int) {
    compute_selected_base();
    const int faction = BaseCurrent->faction_id_current;
    if ((faction != ConsoleGlobal.faction_id_ && !(GameState & STATE_OMNISCIENT_VIEW))
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
            if ((static_cast<unsigned char>(LockEnableMask) & (1u << faction))
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
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/0040a880.cpp", "source:src/basewin.h", "image:0x40a880"], "reason": "Recover the complete original operation through genuine owning C++ classes, members and natural lifetimes; preserve retained seed evidence and historical restoration obligations separately from fresh matching."}
void BaseWin::pop_click(int position, int, int, int) {
    Popup popup;
    compute_selected_base();
    if (BaseCurrent->faction_id_current != ConsoleGlobal.faction_id_
        && !(GameState & STATE_OMNISCIENT_VIEW)) return;
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
        strcpy(StringTemp, reinterpret_cast<char *>(StringTable.get(reinterpret_cast<int>(citizen.singular_name))));
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
            strcat(text, reinterpret_cast<char *>(StringTable.get(reinterpret_cast<int>(Energy[k].abbrev))));
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
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/00409200.cpp", "source:src/basewin.h", "image:0x409200"], "reason": "Recover the complete original operation through genuine owning C++ classes, members and natural lifetimes; preserve retained seed evidence and historical restoration obligations separately from fresh matching."}
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
        else parse_say(0, reinterpret_cast<int>(Facility[-entry].name), -1, -1);
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
        if (picked < 0 || (GameState & 0x800)) return;
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
// retained src/unrecovered/0040c070.cpp and /tmp/basewin-40c070.json.
// All popup strings independently read from original .rdata 682904..682AD4.
// No compiler/measurement performed. Existing deeper callee debt remains.
// Original bulk-sale counting deliberately tests BaseCurrent->state, not
// each candidate base's state. The subsequent bulk mutation deliberately
// does not retest possession or the one-sale-per-turn bit. Preserve both.
// ORIGINAL: 0x0040C070 ?facilities_click@BaseWin@@QAEHHHHH@Z 0x0040C070-0x0040C842
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/0040c070.cpp", "source:src/basewin.h", "image:0x40c070"], "reason": "Recover the complete original operation through genuine owning C++ classes, members and natural lifetimes; preserve retained seed evidence and historical restoration obligations separately from fresh matching."}
int BaseWin::facilities_click(int facility, int, int, int) {
    const int faction = BaseCurrent->faction_id_current;
    if (faction != ConsoleGlobal.faction_id_ && !(GameState & STATE_OMNISCIENT_VIEW))
        return 0;
    parse_says(0, BaseCurrent->name_string, -1, -1);
    parse_say(1, reinterpret_cast<int>(Facility[facility].name), -1, -1);
    const int selection = popp(reinterpret_cast<const char *>(*ClimatePopupTitle), "FACMENU", 0,
        (Players[faction].rule_flags & 0x80) ? "alfacblt_sm.pcx" : "facblt_sm.pcx", 0);
    if (!selection) return 0;
    if (selection == 1) {
        help_topic(facility >= 70 ? 11 : 10, facility);
        return 0;
    }
    if (facility >= 70) {
        popp(reinterpret_cast<const char *>(*ClimatePopupTitle), "NOTPROJECT", 0, "secproj_sm.pcx", 0);
        return 0;
    }
    int offset, mask;
    if (facility < 65) bitmask(facility, &offset, &mask);
    if (facility >= 65 || !(Bases[BaseIDCurrentSelected].facilities_built[offset] & (uint8_t)mask)) {
        popp(reinterpret_cast<const char *>(*ClimatePopupTitle), "DONTHAVEFAC", 0,
            (Players[faction].rule_flags & 0x80) ? "alfacblt_sm.pcx" : "facblt_sm.pcx", 0);
        return 0;
    }
    if (ConsoleControlTurnActive && IsMultiplayerNet) {
        X_pop("DURINGUPKEEP", 0);
        return 0;
    }
    if ((BaseCurrent->state & BSTATE_FACILITY_SCRAPPED) && !(GameState & STATE_OMNISCIENT_VIEW)) {
        parse_says(0, BaseCurrent->name_string, -1, -1);
        popp(reinterpret_cast<const char *>(*ClimatePopupTitle), "ONERECYCLE", 0,
            (Players[faction].rule_flags & 0x80) ? "alfacblt_sm.pcx" : "facblt_sm.pcx", 0);
        return 0;
    }
    if (facility == 1) {
        popp(reinterpret_cast<const char *>(*ClimatePopupTitle), "HQRECYCLE", 0,
            (Players[faction].rule_flags & 0x80) ? "alfacblt_sm.pcx" : "facblt_sm.pcx", 0);
        return 0;
    }
    if (facility == 26 && (map_tiles()[BaseCurrent->y * MapLongitude + (BaseCurrent->x >> 1)].contour & 0xE0) < 0x60) {
        popp(reinterpret_cast<const char *>(*ClimatePopupTitle), "PRESSURERECYCLE", 0,
            (Players[faction].rule_flags & 0x80) ? "alfacblt_sm.pcx" : "facblt_sm.pcx", 0);
        return 0;
    }
    for (int bonus = 0; bonus < Players[faction].faction_bonus_count; ++bonus) {
        const int kind = Players[faction].faction_bonus_id[bonus];
        const int granted = Players[faction].faction_bonus_val1[bonus];
        if ((kind == 2 || (kind == 12 && has_tech(Facility[granted].preq_tech, faction)))
            && facility == granted) {
            parse_say(1, reinterpret_cast<int>(Facility[facility].name), -1, -1);
            popp(reinterpret_cast<const char *>(*ClimatePopupTitle), "FREERECYCLE", 0,
                (Players[faction].rule_flags & 0x80) ? "alfacblt_sm.pcx" : "facblt_sm.pcx", 0);
            return 0;
        }
    }
    if (NetDaemonNet.lock_base(BaseIDCurrentSelected, selection == 3 ? 2 : 0, -1, -1)) return 0;
    int proceeds;
    if (selection == 3) {
        int payable = 0;
        int possessed = 0;
        for (int base = 0; base < BaseCurrentCount; ++base) {
            if (Bases[base].faction_id_current != faction) continue;
            if (facility == 26 && (map_tiles()[Bases[base].y * MapLongitude + (Bases[base].x >> 1)].contour & 0xE0) < 0x60) continue;
            bitmask(facility, &offset, &mask);
            if (Bases[base].facilities_built[offset] & (uint8_t)mask) {
                ++possessed;
                if (!(BaseCurrent->state & BSTATE_FACILITY_SCRAPPED) || (GameState & STATE_OMNISCIENT_VIEW)) ++payable;
            }
        }
        proceeds = static_cast<int>(static_cast<uint32_t>(Facility[facility].cost) * payable * 5u);
        parse_num(0, proceeds);
        parse_num(1, possessed);
        parse_say(0, reinterpret_cast<int>(Facility[facility].name), -1, -1);
    } else {
        proceeds = static_cast<int>(static_cast<uint32_t>(Facility[facility].cost) * 5u);
        parse_say(0, reinterpret_cast<int>(Facility[facility].name), -1, -1);
        parse_num(0, proceeds);
    }
    StringTemp[0] = 0;
    strcat(StringTemp, "RECYCLEME");
    if (selection == 3) strcat(StringTemp, "ALL");
    else if (facility == 8 && (BaseCurrent->state & BSTATE_ARTIFACT_LINKED)) {
        proceeds = 0;
        strcat(StringTemp, "0");
    }
    if (!popp(reinterpret_cast<const char *>(*ClimatePopupTitle), StringTemp, 0,
        (Players[faction].rule_flags & 0x80) ? "alfacblt_sm.pcx" : "facblt_sm.pcx", 0)) {
        BaseComputedID = -1;
        base_compute(0);
        NetDaemonNet.unlock_base(BaseIDCurrentSelected);
        return 0;
    }
    if (selection == 3) {
        for (int base = 0; base < BaseCurrentCount; ++base) {
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
extern int TextColourTable[195];

// ORIGINAL: 0x0040D250 ?draw_flash@BaseWin@@QAEXH@Z 0x0040D250-0x0040E35B;0x00651AA2-0x00651ACD
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/0040d250.cpp", "source:src/basewin.h", "image:0x40d250"], "reason": "Recover the complete original operation through genuine owning C++ classes, members and natural lifetimes; preserve retained seed evidence and historical restoration obligations separately from fresh matching."}
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
            && strlen(StringTable.get(reinterpret_cast<int>(Ability[ability].abbreviation))) != 0) {
            strcat(StringTemp, ", ");
            strcat(StringTemp, StringTable.get(reinterpret_cast<int>(Ability[ability].abbreviation)));
        }
    }
    strcat(StringTemp, ")");
    y = canvas.wrap(StringTemp, x, y, text_width);
    if ((vehicle.faction_id != 0 && prototype.plan < 8)
        || (prototype.plan == 11 && morale_veh(vehicle_id, 1, 0) > 1)) {
        StringTemp[0] = 0;
        say_morale(vehicle_id, 0);
        if (vehicle.faction_id != ConsoleGlobal.faction_id_ && !(GameState & 0x80))
            y = canvas.wrap(StringTemp, x, y, text_width);
        else
            y = canvas.wrap_cent(StringTemp, x, y, text_width);
    }
    if (vehicle.faction_id == ConsoleGlobal.faction_id_ || (GameState & 0x80)) {
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
    if (vehicle.faction_id == ConsoleGlobal.faction_id_ || (GameState & 0x80)
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
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/0041c7a0.cpp", "source:src/basewin.h", "image:0x41c7a0"], "reason": "Recover the complete original operation through genuine owning C++ classes, members and natural lifetimes; preserve retained seed evidence and historical restoration obligations separately from fresh matching."}
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
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00408eb0.cpp", "source:src/basewin.h", "image:original instruction and dispatch evidence for the BaseWin owner"], "reason": "Recover this original base-window operation through the real BaseWin owner, existing typed members and source-owned shared dependencies; catalogue naming and original intervals remain distinct from current compiler symbols."}
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
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/0041a4c0.cpp", "source:src/basewin.h", "image:original instruction and dispatch evidence for the BaseWin owner"], "reason": "Recover this original base-window operation through the real BaseWin owner, existing typed members and source-owned shared dependencies; catalogue naming and original intervals remain distinct from current compiler symbols."}
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
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/0041a620.cpp", "source:src/basewin.h", "image:original instruction and dispatch evidence for the BaseWin owner"], "reason": "Recover this original base-window operation through the real BaseWin owner, existing typed members and source-owned shared dependencies; catalogue naming and original intervals remain distinct from current compiler symbols."}
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
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/00419a90.cpp", "source:src/basewin.h", "image:original instruction and dispatch evidence for the BaseWin owner"], "reason": "Recover this original base-window operation through the real BaseWin owner, existing typed members and source-owned shared dependencies; catalogue naming and original intervals remain distinct from current compiler symbols."}
int BaseWin::governor(int settings_only) {
    compute_selected_base();
    if (Bases[focusedBaseID_].faction_id_current != ConsoleGlobal.faction_id_
        && !(GameState & STATE_OMNISCIENT_VIEW)) return 1;
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
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/0041a2b0.cpp", "source:src/basewin.h", "image:original instruction and dispatch evidence for the BaseWin owner"], "reason": "Recover this original base-window operation through the real BaseWin owner, existing typed members and source-owned shared dependencies; catalogue naming and original intervals remain distinct from current compiler symbols."}
void BaseWin::rename() {
    compute_selected_base();
    if (BaseCurrent->faction_id_current != ConsoleGlobal.faction_id_
        && !(GameState & STATE_OMNISCIENT_VIEW)) return;
    if (NetDaemonNet.lock_base(BaseIDCurrentSelected, 0, -1, -1)) return;
    parse_says(0, BaseCurrent->name_string, -1, -1);
    if (X_pop_ask("RENAME", 0x18, BaseCurrent->name_string, nullptr, 0)) {
        NetDaemonNet.unlock_base(BaseIDCurrentSelected);
        return;
    }
    strcpy(BaseCurrent->name_string, ParseStrBuffer[0].str);
    if (BaseCurrent->faction_id_current == ConsoleGlobal.faction_id_) {
        const int former = BaseCurrent->faction_id_former;
        if (former != ConsoleGlobal.faction_id_ && !((1 << former) & LockEnableMask)
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
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/0041b4f0.cpp", "source:src/basewin.h", "image:original instruction and dispatch evidence for the BaseWin owner"], "reason": "Recover this original base-window operation through the real BaseWin owner, existing typed members and source-owned shared dependencies; catalogue naming and original intervals remain distinct from current compiler symbols."}
void BaseWin::staple_the_bastards() {
    compute_selected_base();
    if (Bases[focusedBaseID_].faction_id_current != ConsoleGlobal.faction_id_
        && !(GameState & STATE_OMNISCIENT_VIEW)) return;
    const int faction = BaseCurrent->faction_id_current;
    const bool alien = (Players[faction].rule_flags & 0x80) != 0;
    parse_says(0, BaseCurrent->name_string, -1, -1);
    const int police = PlayersData[faction].soc_effect_active.police
        + has_fac(0x23, BaseIDCurrentSelected, 0) * 2;
    const char *caption = reinterpret_cast<const char *>(*ClimatePopupTitle);
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
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/0041b1f0.cpp", "source:src/basewin.h", "image:original instruction and dispatch evidence for the BaseWin owner"], "reason": "Recover this original base-window operation through the real BaseWin owner, existing typed members and source-owned shared dependencies; catalogue naming and original intervals remain distinct from current compiler symbols."}
// symbol    ?on_mouse_feedback@BaseWin@@UAEXHH@Z
void BaseWin::on_mouse_feedback(int x, int y) {
    if (!is_visible() || (IsMultiplayerNet && NetDaemonNet.lock_state_[0].dword_[3])) return;
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
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004160f0.cpp", "source:src/basewin.h", "image:original instruction and dispatch evidence for the BaseWin owner"], "reason": "Recover this original base-window operation through the real BaseWin owner, existing typed members and source-owned shared dependencies; catalogue naming and original intervals remain distinct from current compiler symbols."}
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
    if (!is_visible() || (IsMultiplayerNet && NetDaemonNet.lock_state_[0].dword_[3])) return;
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
    if (faction != ConsoleGlobal.faction_id_ && !(GameState & STATE_OMNISCIENT_VIEW)
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
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004165d0.cpp", "source:src/basewin.h", "image:original instruction and dispatch evidence for the BaseWin owner"], "reason": "Recover this original base-window operation through the real BaseWin owner, existing typed members and source-owned shared dependencies; catalogue naming and original intervals remain distinct from current compiler symbols."}
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
    if (!is_visible() || (IsMultiplayerNet && NetDaemonNet.lock_state_[0].dword_[3])) return;
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
    if (faction != ConsoleGlobal.faction_id_ && !(GameState & STATE_OMNISCIENT_VIEW)
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
// ORIGINAL: 0x0041A010 ?prev@BaseWin@@QAEXXZ 0x0041A010-0x0041A15D
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/0041a010.cpp;source:src/base.h;image:0x0041A010-0x0041A15D"], "reason": "Recover previous base selection through the actual Base array, selected-base member, faction context and case-insensitive name order."}
void BaseWin::prev() {
    set_base(static_cast<int>(focusedBaseID_));
    if (BaseCurrent->faction_id_current != ConsoleGlobal.faction_id_
        && !(GameState & STATE_OMNISCIENT_VIEW)) {
        return;
    }
    if (ConsoleControlTurnActive) {
        X_pop("ARROWUPKEEP", nullptr);
        return;
    }

    int selected = static_cast<int>(focusedBaseID_);
    int nearest = -1;
    for (int candidate = 0; candidate < BaseCurrentCount; ++candidate) {
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

// ORIGINAL: 0x0041A160 ?next@BaseWin@@QAEXXZ 0x0041A160-0x0041A2AD
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/0041a160.cpp;source:src/base.h;image:0x0041A160-0x0041A2AD"], "reason": "Recover next base selection through the actual Base array, selected-base member, faction context and case-insensitive name order."}
void BaseWin::next() {
    set_base(static_cast<int>(focusedBaseID_));
    if (BaseCurrent->faction_id_current != ConsoleGlobal.faction_id_
        && !(GameState & STATE_OMNISCIENT_VIEW)) {
        return;
    }
    if (ConsoleControlTurnActive) {
        X_pop("ARROWUPKEEP", nullptr);
        return;
    }

    int selected = static_cast<int>(focusedBaseID_);
    int nearest = -1;
    for (int candidate = 0; candidate < BaseCurrentCount; ++candidate) {
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
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/00415080.cpp", "src/basewin.h:queue rectangles and font1_", "src/base.h:Base queue and Facility", "src/veh.h:VehPrototypes", "src/alpha.h:Labels and Language", "src/buffer.h:InterfaceBoxStyles", "image:0x00682BAC-0x00682BC0 queue strings"], "reason": "Render the eight queued production rows through owned BaseWin rectangles, game queue and prototype/facility records, shared interface styles and palette. Retain the original seed for byte restoration; the old raw self-offset transcription is not product source."}
// TRIED: the retained instruction-level seed with local Font lifetimes and
// raw self offsets scored about 0.70 in a framed build. Its several RECT-copy
// temporaries blocked byte restoration; this typed body keeps that evidence
// without treating those stack-slot spellings as the class model.
void BaseWin::draw_queue(int update_mode) {
    Font row_font;
    Font compact_font;
    Buffer *canvas = static_cast<Buffer *>(&MainInterfaceGlobal);
    if (update_mode != 0 && !is_visible()) return;

    RECT saved_clip = canvas->rect1_;
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
    string_upper_slot()(StringTemp);
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
                // Facility.name holds the StringTable key in the original;
                // StatusWin resolves the same field through get().
                strcat(item_name, StringTable.get(
                    reinterpret_cast<int>(Facility[-production].name)));
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
            canvas->set_clip(&canvas->rect2_);
        } else {
            canvas->set_font(&compact_font, nullptr, nullptr, nullptr);
            const bool truncated = compact_font.width(StringTemp)
                > text_rect.right - text_rect.left;
            if (truncated) text_rect.right -= compact_font.width("+");
            canvas->set_clip(&text_rect);
            if (StringTemp[0] != 0) {
                canvas->write_l(StringTemp, &text_rect, strlen(StringTemp));
            }
            canvas->set_clip(&canvas->rect2_);
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
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/0040e360.cpp", "source:src/basewin.h"], "reason": "Keep the BaseWin production-panel dependency linkable while its original rendering behavior remains in the retained seed."}
// TRIED: BaseWin is opaque past field_40b10_ in this scaffold, so every `this`-relative access below 0x40C6C etc. is a raw I32/B8 cast rather than a named member; the two Font locals + one Sprite local (matching the two Font ctor calls and the single Sprite ctor/close pair in the disassembly) reproduce that RAII shape. The header/border setup and the unit-vs-facility info panel are transcribed from the disassembly; the queue-grid cell-size search and its per-item draw loop (~110 instructions) are replaced with a single representative call rather than the original's two-pass sizing + nested row/column loop, since the unnamed FUN_ callees for that section have no recovered signature to transcribe against.
void BaseWin::draw_production(int) {
    // BODY GOES HERE.
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

// Garrison frames use style3. Original 0x410485 loads style9's mask pointer
// from 0x78D6B0; that address is a member slot, not a Buffer object itself.
// ORIGINAL: 0x004103B0 ?draw_garrison@BaseWin@@QAEXH@Z 0x004103B0-0x00410868;0x00651B0D-0x00651B22 FILE
// RECOVERY_BINDING: v1 {"evidence": ["image:0x004104DD-0x004107F9 typed stack links, column layout, vehicle drawing and hit regions", "image:0x00410573-0x004105E5 controls at 0x007B5080/5BCC/6718 are MainInterfaceGlobal.flat_button_[8..10] by 0xB4C stride", "image:0x004105A7-0x004105CD overflow bounds use adjacent MainInterface rectangles", "src/veh.h:Vehs and veh_at", "src/veh.cpp:say_stats_3 and veh_draw", "src/basewin.h:BaseWin garrison_rect_ and spot1_", "src/maininterface.h:MainInterfaceGlobal"], "reason": "Render the current base's real vehicle stack through owned state, drawing APIs and typed MainInterface buttons and adjacent panel rectangles. Scroll-position behavior remains a restoration obligation."}
// size      1208 bytes
// prototype void (__thiscall ?draw_garrison@BaseWin@@QAEXH@Z)(BaseWin* this, int)
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// The body walks Vehs' typed stack links and renders visible vehicles. The
// original's scroll-position behavior still needs recovery; this currently
// starts with the first vehicle even when more than one row is available.
// LEVER: 0x40C2C is one RECT, not the four unnamed dwords the IDB records -
//  set_clip takes it whole and the tile/frame sizing reads its fields.
//  0x00691B2C is font.h's DefaultFontFace, passed as the pointer it is; the
//  artifact's `*DefaultFontFace` through a pointer-to-pointer is two loads
//  where the image has one.
// 0x004104E6-0x004104F9 scales the column pitch by 100, not 20 as the
// earlier partial transcription did. The stack starts at veh_at(base.x,y),
// climbs prev_veh_id_stack and then walks next_veh_id_stack.
void BaseWin::draw_garrison(int update_mode) {
    Buffer *canvas = static_cast<Buffer *>(&MainInterfaceGlobal);

    RECT saved_clip = canvas->rect1_;
    RECT clip_rect = garrison_rect_;
    canvas->set_clip(&clip_rect);

    Font font;
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
    if (MapWinCaptionScale > 0 && MapWinCaptionDivisor != 0) {
        spacing = (MapWinCaptionScale * 100) / MapWinCaptionDivisor;
    } else {
        spacing = 0x38;
    }

    int vehicle = veh_at(BaseCurrent->x, BaseCurrent->y);
    if (vehicle >= 0) {
        while (Vehs[vehicle].prev_veh_id_stack >= 0) {
            vehicle = Vehs[vehicle].prev_veh_id_stack;
        }
    }

    const int capacity = spacing + 5 > 0
        ? (frame_rect.right - frame_rect.left) / (spacing + 5) : 0;
    if (vehicle >= 0) {
        int stack_count = 0;
        for (int at = vehicle; at >= 0; at = Vehs[at].next_veh_id_stack) {
            ++stack_count;
        }
        MainInterfaceGlobal.flat_button_[8].hide();
        if (stack_count <= capacity) {
            MainInterfaceGlobal.flat_button_[9].hide();
            MainInterfaceGlobal.flat_button_[10].hide();
        } else {
            frame_rect.left = MainInterfaceGlobal.adjacent_panel_left_rect_.right + 1;
            frame_rect.right = MainInterfaceGlobal.adjacent_panel_right_rect_.left - 1;
            if (capacity > 0) {
                spacing = (frame_rect.right - frame_rect.left) / capacity - 5;
            }
            MainInterfaceGlobal.flat_button_[9].show(0);
            MainInterfaceGlobal.flat_button_[10].show(0);
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
        if ((GameState & 0x1040) == 0x1040) {
            char number[80];
            strcat(StringTemp, " (");
            _itoa(vehicle, number, 10);
            strcat(StringTemp, number);
            strcat(StringTemp, ")");
        }
        canvas->set_font(&font, nullptr, nullptr, nullptr);
        canvas->set_text_color(TextColourTable[112], TextColourTable[113], 1, 1);
        canvas->write_cent_l(StringTemp, column_left,
                             frame_rect.bottom - line_height + 6, spacing,
                             strlen(StringTemp));
        spot1_.add(vehicle, 6, column_left, frame_rect.top,
                   spacing + 5, frame_rect.bottom - frame_rect.top);
        column_left += spacing + 5;
        vehicle = Vehs[vehicle].next_veh_id_stack;
        ++column;
    }

    canvas->set_clip(&clip_rect);
    if (update_mode != 0) MainInterfaceGlobal.update(&garrison_rect_, nullptr);
    else MainInterfaceGlobal.soft_update(&garrison_rect_);
    canvas->set_clip(&saved_clip);
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
    if (is_visible() != 0 && (IsMultiplayerNet == 0 || NetDaemonNet.lock_state_[0].dword_[3] == 0)) {
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

    if (is_visible() != 0 && (IsMultiplayerNet == 0 || NetDaemonNet.lock_state_[0].dword_[3] == 0)) {
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
// ORIGINAL: 0x0041D500 ?on_button_clicked@BaseWin@@QAEXH@Z 0x0041D500-0x0041DA6C
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/0041d500.cpp", "source:src/basewin.h", "image:original instruction and dispatch evidence for the BaseWin owner"], "reason": "Recover this original base-window operation through the real BaseWin owner, existing typed members and source-owned shared dependencies; catalogue naming and original intervals remain distinct from current compiler symbols."}
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
            && !(GameState & STATE_OMNISCIENT_VIEW)) return;
        if (BaseCurrent->governor & 0x80000000u) {
            BaseCurrent->governor &= 0x7FFFFFFFu;
            redraw();
            MainInterfaceGlobal.redraw();
        } else gov_on(0);
        return;
    case 20:
    case 21:
        if (BaseCurrent->faction_id_current == ConsoleGlobal.faction_id_
            || (GameState & STATE_OMNISCIENT_VIEW)) governor(0);
        return;
    case 6:
    case 7:
    case 8:
    case 9:
        if (BaseCurrent->faction_id_current == ConsoleGlobal.faction_id_
            || (GameState & STATE_OMNISCIENT_VIEW))
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
            || (GameState & STATE_OMNISCIENT_VIEW)) staple_the_bastards();
        return;
    case 23:
        if (BaseCurrent->faction_id_current == ConsoleGlobal.faction_id_
            || (GameState & STATE_OMNISCIENT_VIEW)) base_editor();
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
// ORIGINAL: 0x0041DAD0 ?on_status@BaseWin@@QAEXH@Z 0x0041DAD0-0x0041DC7F BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/0041dad0.cpp", "source:src/basewin.h", "image:original instruction and dispatch evidence for the BaseWin owner"], "reason": "Recover this original base-window operation through the real BaseWin owner, existing typed members and source-owned shared dependencies; catalogue naming and original intervals remain distinct from current compiler symbols."}
// symbol    ?on_status@BaseWin@@UAEXH@Z
void BaseWin::on_status(int mode) {
    if (mode) {
        WorldClimateWorldWin.hide_all();
        on_redraw();
        for (int i = 0; i < 22; ++i) {
            if (i == 16 && (Players[Bases[focusedBaseID_].faction_id_current].rule_flags & 0x80)) {
                flatButton_.show(4);
            } else if (i != 21 || (GameState & 0x40)) {
                flatButtons_[i].show(4);
            }
        }
        MainInterfaceGlobal.update(0);
        field_40B54_ = -1;
        if ((GamePreferences & 0x400) && !g_BASEWIN_WAVE.is_playing())
            g_BASEWIN_WAVE.play();
        ambience(0xCD);
        do_all_draws();
        slide_show(this, 0);
        Win::show(3);
        if (GamePreferences & 0x400) g_BASEWIN_WAVE.fade();
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
// ORIGINAL: 0x0041DF50 ?on_iface_mouse_move@BaseWin@@QAEXHH@Z 0x0041DF50-0x0041E3DC
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/0041df50.cpp", "source:src/basewin.h", "image:original instruction and dispatch evidence for the BaseWin owner"], "reason": "Recover this original base-window operation through the real BaseWin owner, existing typed members and source-owned shared dependencies; catalogue naming and original intervals remain distinct from current compiler symbols."}
// symbol    ?on_iface_mouse_move@BaseWin@@UAEXHH@Z
void BaseWin::on_iface_mouse_move(int a1, int a2) {
    if (!is_visible() || (IsMultiplayerNet && NetDaemonNet.lock_state_[0].dword_[3])) return;
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
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/0041e790.cpp", "src/basewin.h"], "reason": "Recover the BaseWin redraw call sequence through its owned FlatButton array, panel methods and source-owned interface mode globals; panel implementations remain independent obligations."}
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
// ORIGINAL: 0x0041E560 ?update_gov_buttons@BaseWin@@QAEXXZ 0x0041E560-0x0041E788
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0041e560.cpp", "src/basewin.h", "src/base.h", "src/faction.h", "src/basebutton.h"], "reason": "Recover governor-priority button state through BaseCurrent, typed PlayerData AI preferences, the BaseWin FlatButton array and BaseButton's owned auxiliary state. The retained seed remains the measurement reference."}
// The seed's four 0x0096CDC0..0x0096CDCC indexed words are
// PlayersData[faction].ai_growth/ai_tech/ai_wealth/ai_power: each index step
// is 2099 dwords, the established 0x20CC-byte PlayerData stride.
void BaseWin::update_gov_buttons() {
    const Base &base = *BaseCurrent;
    const int faction = base.faction_id_current;
    const unsigned int governor = base.governor;
    const bool human = (LockEnableMask & (1u << faction)) != 0;

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
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/00412aa0.cpp", "src/basewin.h"], "reason": "Preserve the separately identified expenses panel reached by BaseWin redraw as an explicitly unfinished linkable body."}
void BaseWin::draw_expenses(int) {
    // BODY GOES HERE.
}
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
    RECT previous_clip = screen->rect1_;
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
    string_upper_slot()(StringTemp);

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
// ORIGINAL: 0x00415730 ?draw_misc@BaseWin@@QAEXH@Z 0x00415730-0x00415B15
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00415730.cpp", "src/basewin.h", "src/alpha.h", "src/faction.h", "src/base.h", "src/palette.cpp", "src/font.h"], "reason": "Recover the miscellaneous base panel through its owned rectangle, label and string table, live Base and PlayerData fields, font line advance, and shared palette rather than executable addresses and raw self-offsets. The retained seed remains for byte restoration."}
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
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/0040f0f0.cpp", "src/basewin.h"], "reason": "Preserve the separately identified farm panel reached by BaseWin redraw as an explicitly unfinished linkable body."}
void BaseWin::draw_farm(int) {
    // BODY GOES HERE.
}
// ORIGINAL: 0x00414B40 ?draw_pop@BaseWin@@QAEXH@Z 0x00414B40-0x00415078;0x00651B76-0x00651B93
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00414b40.cpp", "image:0x00414C58-0x00414CA3 population spacing uses panel width minus height", "image:0x00414CB4-0x00414CDD expansion and alien rule select alien sprites", "src/sprite.cpp", "src/base.h", "src/faction.h", "src/basewin.h"], "reason": "Draw the current base's citizen and specialist row through owned BaseWin panel and Spot members, typed Base mood/specialist fields, and the four real large-sprite arrays. The retained seed reverses the alien branch and mistranscribes the spacing; the pinned instructions resolve both."}
void BaseWin::draw_pop(int update_mode) {
    Font font;
    Random random;
    random.reseed(timeGetTime());

    Buffer *canvas = static_cast<Buffer *>(&MainInterfaceGlobal);
    RECT saved_clip = canvas->rect1_;
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
void BaseWin::draw_commerce(int update_mode) {
    Buffer *canvas = this;
    RECT saved_clip = canvas->rect1_;
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
        string_upper_slot()(StringTemp);
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
        for (int candidate = 0; candidate < BaseCurrentCount; ++candidate) {
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
        memcpy(project_homes, &SecretProject, sizeof(project_homes));
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
            string_upper_slot()(StringTemp);
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
            string_upper_slot()(StringTemp);
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
            if (own_sanctions != 0 || partner_sanctions != 0) {
                canvas->set_text_color(TextColourTable[110], TextColourTable[111], 1, 1);
                strcat(StringTemp, label_get(450));
                strcat(StringTemp, " (");
                char turns[32];
                _itoa(own_sanctions > partner_sanctions
                          ? own_sanctions : partner_sanctions, turns, 10);
                strcat(StringTemp, turns);
                strcat(StringTemp, ")");
                string_upper_slot()(StringTemp);
                canvas->write_l(StringTemp, report.left + 5, detail_y,
                                strlen(StringTemp));
            } else {
                strcat(StringTemp, label_get((PlayersData[owner].diplo_treaties[partner]
                                              & DTREATY_PACT) != 0 ? 645 : 644));
                int text_end = canvas->write_l(StringTemp, report.left + 5,
                                               detail_y, strlen(StringTemp));
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
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/00411240.cpp:1912-2151", "src/basewin.h:nutrient rectangles and sprites3_", "src/base.h:Base nutrients_accumulated and nutrient_surplus", "src/base.cpp:BaseIDCurrentSelected and cost_factor", "image:0x00682B20-0x00682B28 nutrient punctuation"], "reason": "Render the growth grid and caption through owned BaseWin rectangles, sprites, current Base and label table. Retain the instruction-level seed for restoration rather than hardcoding its self/global addresses."}
void BaseWin::draw_nutrients(int update_mode) {
    Buffer *canvas = this;
    RECT saved_clip = canvas->rect1_;
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
    string_upper_slot()(StringTemp);
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
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/0041b790.cpp", "source:src/basewin.h", "image:0x41b790"], "reason": "Recover the full base-editor callee in its actual BaseWin owner with one genuine Popup lifetime and original cancellation, menu IDs, typed data mutations and virtual redraw; preserve original seed evidence and outstanding structural/matching obligations."}
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
            strcat(StringTemp, StringTable.get(reinterpret_cast<int>(Facility[facility].name)));
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
                strcat(StringTemp, StringTable.get(reinterpret_cast<int>(Facility[facility].name)));
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
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/0041bbc0.cpp", "source:src/basewin.h", "image:0x41bbc0"], "reason": "Recover the full base-editor callee in its actual BaseWin owner with one genuine Popup lifetime and original cancellation, menu IDs, typed data mutations and virtual redraw; preserve original seed evidence and outstanding structural/matching obligations."}
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
        strcat(StringTemp, StringTable.get(reinterpret_cast<int>(facility.name)));
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
    parse_say(0, reinterpret_cast<int>(Facility[FacilitySPStart + project].name), -1, -1);
    PopupModalState = 6;
    popup.start(PopupStartCaption, "PICKAPLACE", -1, 0, 0x42, 0);
    popup.dialogs().item(label_get(0xD64 / 4), 9999);
    popup.dialogs().item(label_get(0xD68 / 4), 9998);
    // Current-faction bases precede other bases, preserving original index IDs.
    // Re-read BaseCurrent and the live base count after each item call, as
    // original41BEBA..41BEC2 does; neither is captured as immutable UI state.
    for (int pass = 0; pass < 2; ++pass) {
        for (int base = 0; base < BaseCurrentCount; ++base) {
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
    *(&SecretProject.human_genome_project + project) = owner_base;
    redraw();
    // Popup owns complete member/base teardown, including cancellation paths.
}

// ORIGINAL: 0x0041C240 ?base_editor_copy@BaseWin@@QAEXH@Z 0x0041C240-0x0041C798;0x006526E3-0x006527EF
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/0041c240.cpp", "source:src/basewin.h", "image:0x41c240"], "reason": "Recover the full base-editor callee in its actual BaseWin owner with one genuine Popup lifetime and original cancellation, menu IDs, typed data mutations and virtual redraw; preserve original seed evidence and outstanding structural/matching obligations."}
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
        for (int base = 0; base < BaseCurrentCount; ++base) {
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
            for (int base = 0; base < BaseCurrentCount; ++base) {
                if (base == BaseIDCurrentSelected) continue;
                if (selection == 9998 && Bases[base].faction_id_current != BaseCurrent->faction_id_current) continue;
                Bases[base].facilities_built[byte] = BaseCurrent->facilities_built[byte];
            }
        }
    }
    redraw();
}
