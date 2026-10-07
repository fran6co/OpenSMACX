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
#include "temp.h"
#include "base.h"
#include "council.h"
#include "alpha.h"
#include "faction.h"
#include "game.h"
#include "log.h"
#include "map.h"
#include "text.h"
#include "technology.h"
#include "veh.h"
#include "console.h"
#include "datalink.h"
#include "netdaemon.h"
#include "xpops.h"
#include "buffer.h"
#include "font.h"
#include "infowin.h"
#include "maininterface.h"
#include "mapwin.h"
#include "general.h"
#include "netmsg.h"
#include "replaywin.h"
#include "messagewin.h"
#include "tutwin.h"
#include "path.h"
#include "terraforming.h"
#include "interlude.h"
#include "worldwin.h"


// The UI, network and AI callers share the same base-domain operation.
// ORIGINAL: 0x004CA7F0 ?action_staple@@YAXH@Z 0x004CA7F0-0x004CAA4B
// RECOVERY_BINDING: v1 {"evidence": ["git:bca75cb0:src/recovered/units/004ca7f0.cpp", "image:original complete function instructions", "source:src/base.cpp"], "reason": "Home the original behavioral dependency in its base/audio domain using typed state and existing named callees; retained seed provenance and original spans remain restoration evidence, not runtime acceptance."}
void __cdecl action_staple(int base_id) {
    Base &base = Bases[base_id];
    const int attempts = ++base.nerve_staple_count;
    const unsigned int faction = base.faction_id_current;
    bool success = false;
    if (attempts <= 2) {
        success = true;
    } else if (attempts <= 8 && rand() % 2 != 0) {
        success = true;
    }
    if (success) {
        base.state &= ~BSTATE_DRONE_RIOTS_ACTIVE;
        base.nerve_staple_turns_left += 10;
        draw_tile(base.x, base.y, 2);
        if (base.faction_id_current == ConsoleGlobal.faction_id_) {
            parse_says(0, base.name_string, -1, -1);
            // The original retains this faction across draw_tile and does
            // not add an ExpansionEnabled test to the alien rule check.
            popp(reinterpret_cast<char *>(*ClimatePopupTitle), "NERVESTAPLEDONE", 0,
                 (Players[faction].rule_flags & RFLAG_ALIEN)
                     ? "ALdrone_sm.pcx" : "drone_sm.pcx", 0);
        } else {
            parse_says(0, base.name_string, -1, -1);
            PluralityDefault = 0;
            GenderDefault = Players[base.faction_id_current].is_leader_female;
            parse_says(1, Players[base.faction_id_current].title_leader, -1, -1);
            PluralityDefault = 0;
            GenderDefault = Players[base.faction_id_current].is_leader_female;
            parse_says(2, Players[base.faction_id_current].name_leader, -1, -1);
            NetMsgPrimary.pop("NERVESTAPLETHEM", 5000, 0, 0);
        }
    } else if (base.faction_id_current == ConsoleGlobal.faction_id_) {
        parse_says(0, base.name_string, -1, -1);
        popp(reinterpret_cast<char *>(*ClimatePopupTitle), "NERVESTAPLEFAIL", 0,
             (Players[faction].rule_flags & RFLAG_ALIEN)
                 ? "ALdrone_sm.pcx" : "drone_sm.pcx", 0);
    }
    // Failed attempts and foreign bases still report the atrocity.
    atrocity(base.faction_id_current, base.faction_id_former,
             base.faction_id_former != base.faction_id_current, 1);
}

// The base-caption renderer retains the font selected from its shared queue.
// Original storage: 0x0093FC24. No other product owner currently publishes it.
// RECOVERY_OBJECT: v2 0x0093FC24 0x0093FC28 ?BaseDrawFont@@3PAVFont@@A 0x0055AF20
Font *BaseDrawFont;

// base_draw is the 2073-byte base renderer reached by MapWin::draw_bases.
// The seed's coordinate, sprite, caption, visibility and status branches are
// expressed through their owning objects below. The original span and seed
// remain available for compiler and reference comparison.
// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// TRIED: full line-by-line translation compiles and matches control flow, call args verified against raw disasm (bitmask/box/fill/draw_dest/ get_zoom/set_font/set_text_color/write_l/width/itoa/strcat all cross-checked); prologue register allocation differs (#0 push/sub).
// ORIGINAL: 0x0055AF20 ?base_draw@@YAXPAUBuffer@@HHHHH@Z 0x0055AF20-0x0055B739
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/0055af20.cpp retains the 2073-byte transcription and original span; the seed's Base, Map, Sprite, FontQueue and Buffer dependencies have typed product owners."], "reason": "Recover the base renderer's distant zoom badge, faction sprite, population caption, visibility check and riot marker through typed objects; compiler and runtime verification remain separate."}
// symbol    ?base_draw@@YAXPAVBuffer@@HHHHH@Z
void __cdecl base_draw(Buffer *target, int base_id, int pixel_x, int pixel_y,
                       int style, int flags) {
    Base &base = Bases[base_id];
    const int faction = base.faction_id_current;
    RECT rect;

    // At distant zoom, show the base as a colored lozenge rather than a
    // building sprite and population caption.
    if ((style < -8 || (flags & 0x100)) && !(flags & (8 | 0x800))) {
        int height;
        int width;
        int inset;
        if (MapWinCaptionScale < 1) {
            height = ((style * 5 + 80) * 10) / 16;
            width = ((style * 5 + 80) * 20) / 16;
            inset = (style * 4 + 64) / 16;
        } else {
            height = MapWinCaptionScale * 50 / MapWinCaptionDivisor;
            width = MapWinCaptionScale * 100 / MapWinCaptionDivisor;
            inset = MapWinCaptionScale * 4 / MapWinCaptionDivisor;
        }
        if (flags & 0x200) {
            rect.left = pixel_x - 2;
            rect.right = pixel_x + width + 2;
            rect.top = pixel_y - 2;
            rect.bottom = pixel_y + height + 2;
            inset = 2;
        } else {
            rect.left = pixel_x + inset;
            rect.right = pixel_x + width - inset;
            rect.top = pixel_y + inset;
            rect.bottom = pixel_y + height - inset;
        }
        const int horizontal_trim = (rect.right - rect.bottom - rect.left + rect.top) / 2;
        rect.left += horizontal_trim;
        rect.right -= horizontal_trim;
        const int vertical_shift = MapWinCaptionScale < 1
            ? (style * 3 + 48) * 2 / 16
            : MapWinCaptionScale * 6 / MapWinCaptionDivisor;
        rect.top += vertical_shift - inset;
        rect.bottom += vertical_shift - inset;
        const int border = strcmp(Players[faction].search_key, "SPARTANS") == 0 ? 44 : 0;
        target->box(&rect, border, border);
        ++rect.left;
        --rect.right;
        ++rect.top;
        --rect.bottom;
        target->fill(&rect, InfoFactionTextColors[faction]);
        return;
    }

    if (MapWinCaptionScale < 1) {
        SpriteScaleNumerator = style + 16;
        SpriteScaleDenominator = 16;
    } else {
        SpriteScaleNumerator = MapWinCaptionScale;
        SpriteScaleDenominator = MapWinCaptionDivisor;
    }

    const int population = base.population_size;
    int mode = 3;
    if (population < 4) {
        mode = 0;
    } else if (population < static_cast<int>(Rules.pop_limit_sans_hab_complex + 1 < 8
                                      ? 7 : Rules.pop_limit_sans_hab_complex + 1)) {
        mode = 1;
    } else if (population < static_cast<int>(Rules.pop_limit_sans_hab_dome + 1 < 11
                                      ? 10 : Rules.pop_limit_sans_hab_dome + 1)) {
        mode = 2;
    }
    int byte_index, bit;
    bitmask(1, &byte_index, &bit);
    if (base.facilities_built[byte_index] & bit && mode < 3) ++mode;
    int danger = 0;
    bitmask(4, &byte_index, &bit);
    if ((base.facilities_built[byte_index] & bit)
        || (GameDataStateStorage.fields_.secret_project_.citizens_defense_force() >= 0
            && Bases[GameDataStateStorage.fields_.secret_project_.citizens_defense_force()].faction_id_current == faction)) {
        danger = 1;
    }
    bitmask(5, &byte_index, &bit);
    if (base.facilities_built[byte_index] & bit) danger = 2;

    if (!(flags & 0x800)) {
        Map *tile = map_loc(base.x, base.y);
        const int terrain = (static_cast<unsigned char>(tile->bit) & 0xE0) < 0x60 ? 1 : 0;
        Sprite *sprite = FactionBaseSprites[base.faction_id_former][terrain][mode][danger];
        sprite->draw_dest(target, sprite->transparent_index(), pixel_x, pixel_y,
                          ShadowColorTable);
    }

    BaseDrawFont = g_FONTQUEUE_VAL1.get_zoom(style, 1, 3, 1);
    if (flags & 0x40) {
        SpriteScaleNumerator = 1;
        SpriteScaleDenominator = 1;
        return;
    }

    char number[80];
    char measure_text[80] = "";
    char caption[80] = "";
    _itoa(population, number, 10);
    strcat(caption, number);
    strcat(measure_text, "8");
    if (population >= 10) strcat(measure_text, "8");
    rect.left = pixel_x;
    rect.right = pixel_x + BaseDrawFont->width(measure_text) + 5;
    rect.top = pixel_y;
    rect.bottom = pixel_y + BaseDrawFont->pixel_height() + 4;

    Map *tile = map_loc(base.x, base.y);
    int visible_occupier = -1;
    if (tile->bit & 2) {
        visible_occupier = tile->val2 & 15;
        if (visible_occupier > 7) visible_occupier = -1;
    }
    bool reveal = false;
    const int viewer = ConsoleGlobal.faction_id_;
    if (visible_occupier < 0) {
        reveal = (PlayersData[faction].diplo_treaties[viewer] & 1)
            || spying(faction)
            || (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)
            || (PlayersData[viewer].flags & PFLAG_MAP_REVEALED)
            || faction == viewer;
        if (!reveal) {
            for (int direction = 0; direction < 8; ++direction) {
                int nx = base.x + VehMoveDeltaX[direction];
                if (!(MapIsFlat & 1)) {
                    if (nx < 0) nx += MapLongitudeBounds;
                    else if (nx >= MapLongitudeBounds) nx -= MapLongitudeBounds;
                }
                const int ny = base.y + VehMoveDeltaY[direction];
                if (ny >= 0 && ny < MapLatitudeBounds
                    && nx >= 0 && nx < MapLongitudeBounds) {
                    Map *neighbor = map_loc(nx, ny);
                    int occupant = -1;
                    if (neighbor->bit & 2) {
                        occupant = neighbor->val2 & 15;
                        if (occupant > 7) occupant = -1;
                    }
                    if (occupant == viewer) { reveal = true; break; }
                }
            }
        }
    }

    if (reveal) {
        rect.left = 0;
        rect.top = 0;
        target->box(&rect, 0, 0);
        ++rect.left;
        --rect.right;
        ++rect.top;
        --rect.bottom;
        target->set_font(BaseDrawFont, 0, 0, 0);
        target->set_text_color(ChatItemIdTable[faction], -1, 1, 1);
    } else {
        const int border = strcmp(Players[faction].search_key, "SPARTANS") == 0
            ? InfoFactionShadowColors[faction] : 0;
        target->box(&rect, border, border);
        ++rect.left;
        --rect.right;
        ++rect.top;
        --rect.bottom;
        target->fill(&rect, InfoFactionTextColors[faction]);
        target->set_font(BaseDrawFont, 0, 0, 0);
        target->set_text_color(strcmp(Players[faction].search_key, "SPARTANS") == 0
                                   ? 44 : 0, -1, 1, 1);
    }
    target->write_l(caption, rect.bottom + 2, rect.top + 1, strlen(caption));
    --rect.left;
    ++rect.right;
    --rect.top;
    ++rect.bottom;
    if (base.state & BSTATE_DRONE_RIOTS_ACTIVE) {
        int icon_x = pixel_x + 2 + rect.right - rect.left;
        int icon_y = pixel_y;
        Sprite *icon = &g_RED_MALE_HEAD_ICON_SPRITE;
        if (Players[faction].rule_flags & RFLAG_ALIEN) {
            icon = &g_RED_ALIEN_HEAD_ICON_SPRITE;
            icon_x += 2;
            icon_y -= 6;
        }
        icon->draw(target, icon->transparent_index(), icon_x, icon_y);
    }
    SpriteScaleNumerator = 1;
    SpriteScaleDenominator = 1;
}

// RECOVERY_OBJECT: v2 0x00689370 0x00689374 ?BaseIDCurrentSelected@@3HA 0x004E68B0
int BaseIDCurrentSelected = -1;  // 0x00689370
// set_base publishes the pointer itself; resource computations and the popup
// sound callback load it before accessing the selected Base's members.
// RECOVERY_OBJECT: v2 0x0090EA30 0x0090EA34 ?BaseCurrent@@3PAUBase@@A 0x004E39D0
Base *BaseCurrent;
// production_phase publishes the current faction's reserves here (526FB7);
// allocate_energy reads the retained value (526821), and hurry deducts from it.
// The name is provisional; producer/allocator bodies remain retained seeds.
// RECOVERY_OBJECT: v2 0x0093A958 0x0093A95C ?CurrentFactionEnergyReserves@@3HA 0x00526E70
int CurrentFactionEnergyReserves;
// base_compute publishes the selected id before running the resource passes.
// Window refreshes invalidate this cache without changing the selected base.
// RECOVERY_OBJECT: v2 0x00689374 0x00689378 ?BaseComputedID@@3HA 0x004EC3B0
int BaseComputedID = -1;
// Historical object identity moved to GameDataStateStorage; see the transport manifest.
// 0x009A64CC
int BaseFindDist;  // 0x0090EA04
// 0x009A6814
// RECOVERY_OBJECT: v2 0x0090EA34 0x0090EA38 ?BaseUpkeepStage@@3IA 0x004E68B0
uint32_t BaseUpkeepStage;  // 0x0090EA34

// Shared publications of the base resource-allocation pass, initially zero.
// The adequacy flag is also read by base_doctors; successful reuse of an
// existing allocation deliberately leaves its previous value untouched.
// RECOVERY_OBJECT: v2 0x0090E938 0x0090E93C ?BasePopulationUnassigned@@3HA 0x004E80B0
int BasePopulationUnassigned;
// RECOVERY_OBJECT: v2 0x0090E900 0x0090E904 ?BaseNutrientAdequate@@3HA 0x004E80B0
int BaseNutrientAdequate;

// base_support clears four resource slots in each initially-zero publication.
// Its convoy-order selector and the resource passes use the original 0..3
// resource domain; invalid selectors are not evidence of larger arrays.
// RECOVERY_OBJECT: v2 0x0090E904 0x0090E914 ?BaseCurrentConvoyFrom@@3PAIA 0x004E9550
uint32_t BaseCurrentConvoyFrom[4];
// RECOVERY_OBJECT: v2 0x0090E93C 0x0090E94C ?BaseCurrentConvoyTo@@3PAIA 0x004E9550
uint32_t BaseCurrentConvoyTo[4];

// Original base_energy writes one entry for each partner faction 1..7 and
// draw_commerce reads those same 4-byte slots. The adjacent known objects at
// 0x0090E980 and 0x0090EA30 bound each seven-entry publication.
// RECOVERY_OBJECT: v2 0x0090EA10 0x0090EA2C ?BaseCommerceReceipts@@3PAHA 0x004EB560
int BaseCommerceReceipts[7];
// RECOVERY_OBJECT: v2 0x0090E964 0x0090E980 ?BaseCommercePartnerReceipts@@3PAHA 0x004EB560
int BaseCommercePartnerReceipts[7];

// Defined beside base_claimed below; base_reset uses them as the four
// mutable candidate-site publications returned by base_build.
extern int BaseSiteClaimX[4];
extern int BaseSiteClaimY[4];

namespace {

// The scoring expressions use x86 low-word arithmetic followed by signed
// division. Express that wrapping without signed overflow or negative shifts.
int base_yield_signed_bits(uint32_t value) {
    return value <= 0x7FFFFFFFu ? static_cast<int>(value)
        : -1 - static_cast<int>(0xFFFFFFFFu - value);
}
int base_yield_add(int left, int right) {
    return base_yield_signed_bits(static_cast<uint32_t>(left) + static_cast<uint32_t>(right));
}
int base_yield_sub(int left, int right) {
    return base_yield_signed_bits(static_cast<uint32_t>(left) - static_cast<uint32_t>(right));
}
int base_yield_mul(int left, int right) {
    return base_yield_signed_bits(static_cast<uint32_t>(left) * static_cast<uint32_t>(right));
}
int base_yield_shift(int value, int shift) {
    return base_yield_signed_bits(static_cast<uint32_t>(value) << shift);
}
int base_yield_denominator(int value) {
    return value < 1 ? 1 : (value > 99 ? 99 : value);
}

// These are distinct scalar members, not an array spanning their addresses.
int &base_yield_intake(Base &base, int resource, bool second) {
    if (second) {
        switch (resource) {
        case 0: return base.nutrient_intake_2;
        case 1: return base.mineral_intake_2;
        case 2: return base.energy_intake_2;
        default: return base.unused_intake_2;
        }
    }
    switch (resource) {
    case 0: return base.nutrient_intake_1;
    case 1: return base.mineral_intake_1;
    case 2: return base.energy_intake_1;
    default: return base.unused_intake_1;
    }
}
int base_yield_satellite(int faction, int resource) {
    switch (resource) {
    case 0: return base_yield_signed_bits(PlayersData[faction].satellites_[0]);
    case 1: return base_yield_signed_bits(PlayersData[faction].satellites_[1]);
    default: return base_yield_signed_bits(PlayersData[faction].satellites_[2]);
    }
}
bool base_yield_has_facility(int facility) {
    const int selected = BaseIDCurrentSelected;
    int byte_index;
    int mask;
    bitmask(facility, &byte_index, &mask);
    return (Bases[selected].facilities_built[byte_index]
        & static_cast<unsigned char>(mask)) != 0;
}
int base_yield_specialist(Base *&base, int index) {
    if (index < 16) {
        return (base->spec[index >> 3] >> ((index & 7) * 4)) & 15;
    }
    const int result = best_specialist();
    base = BaseCurrent;
    return result;
}
void base_yield_set_specialist(Base *&base, int index, int specialist) {
    if (index < 16) {
        const int word = index >> 3;
        const int shift = (index & 7) * 4;
        base->spec[word] &= ~(15u << shift);
        base = BaseCurrent;
        base->spec[word] |= static_cast<uint32_t>(specialist) << shift;
        base = BaseCurrent;
    }
}

// The caller has already wrapped x. Invalid selected positions still consume
// a worker, but perform no resource calls and do not refresh map_width.
void base_yield_apply_tile(Base *&base, int &map_width, int x, int y, bool subtract) {
    if (y < 0 || y >= MapLatitudeBounds || x < 0 || x >= map_width) {
        return;
    }
    if ((map_tiles()[static_cast<int>(MapLongitude) * y + (x >> 1)].climate & 0xE0) < 0x60) {
        base->state |= 0x8000;
        base = BaseCurrent;
    }
    for (int resource = 0; resource < 4; ++resource) {
        const int amount = resource_yield(resource, base->faction_id_current,
                                          BaseIDCurrentSelected, x, y);
        base = BaseCurrent;
        int &intake = base_yield_intake(*base, resource, true);
        intake = subtract ? base_yield_sub(intake, amount) : base_yield_add(intake, amount);
        base = BaseCurrent;
    }
    // Resource 3 returns zero but clears TileYieldRestricted: keep its call.
    map_width = MapLongitudeBounds;
}
int base_yield_wrap_x(int x, int map_width) {
    if ((MapIsFlat & 1) == 0) {
        if (x < 0) x += map_width;
        else if (x >= map_width) x -= map_width;
    }
    return x;
}

} // namespace

// The selected-base energy pass owns convoy/commerce accounting, upkeep-stage
// reserve transfer, allocation and specialist yields, facility/project multipliers,
// faction efficiency adjustment, Network Backbone research credits and the final
// energy-production special case. Historical byte and restoration obligations
// remain attached to the complete original span.
// ORIGINAL: 0x004EB560 ?base_energy@@YAXXZ 0x004EB560-0x004EC3A6
// RECOVERY_BINDING: v1 {"evidence": ["git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/unrecovered/004eb560.cpp", "image:0x004EB560-0x004EC3A6 ranks the selected base and publishes seven commerce pairs at 0x0090EA10/0x0090E964; its commerce base doubles when 0x009A6628 is below 0x009A6624", "source:src/base.h", "source:src/faction.h"], "reason": "Recover the selected-base energy pass through typed Base, PlayerData, Player, Citizen, secret-project and shared-ledger owners, including trade, upkeep, specialist, facility, research-efficiency and final production stages. This source mapping makes no exactness claim; preserve historical claims and seed findings as restoration obligations."}
// kind      game
// Historical scaffold-era TRIED note follows; it records the earlier offset-transliteration attempt, not this typed owner body.
// TRIED: void return confirmed - the function's only exit is the fall- through after `call base_psych` (itself void, ?base_psych@@YAXXZ) with no eax write in between, so it is a real `ret`, not a status. The 6/40/80/99999 constants the audit flagged are all clamp bounds (min/max against a computed value that is then stored into a per-base or per-faction field) or array strides, never delivered to the caller. Body is a direct, offset-checked transcription of the Ghidra decompilation with DAT_ globals replaced by explicit fixed-address casts (BASE macro re-reads [0x90ea30] at each use rather than caching it, matching the repeated `mov eax,[0x90ea30]` in the disassembly) and callee names resolved to the scaffold's declared black_market/ base_rank/best_specialist/has_tech/bitmask/base_psych. Compiles and preserves every branch, loop and goto (LAB_004ebbde, LAB_004ec1f6, LAB_004ebddd); did not chase the early-prologue register-scheduling divergence given the size already spent.
void __cdecl base_energy() {
    Base &base = *BaseCurrent;
    const int faction = base.faction_id_current;
    // Deliberately assigned only by a successful treaty/base-rank path below.
    // The original later reads this stack local even when no path assigned it.
    int base_energy_alien_receipt_subtotal;
    // The independent ordinal is set only when the scan encounters BaseCurrent;
    // retain the native local's unset no-match path.
    int base_energy_selected_ordinal;
    base.energy_intake_2 += BaseCurrentConvoyTo[2];
    base.energy_consumption = BaseCurrentConvoyFrom[2];
    int available = base.energy_intake_2 - base.energy_consumption;
    base.energy_inefficiency = black_market(available);
    base.energy_surplus = available - base.energy_inefficiency;

    // Rank the selected base among its own faction by pre-commerce energy.
    // base_rank sorts that same signed key in descending order and returns a
    // partner's base at the corresponding rank (or -1 if it has fewer bases).
    int commerce_rank = 0;
    for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
        const Base &candidate = Bases[base_id];
        if (candidate.faction_id_current == faction) {
            if (&candidate == BaseCurrent) base_energy_selected_ordinal = base_id;
            if (base.energy_intake_1 * 512 + BaseIDCurrentSelected <
                candidate.energy_intake_1 * 512 + base_id) ++commerce_rank;
        }
    }
    for (int partner = 1; partner < MaxPlayerNum; ++partner) {
        const int slot = partner - 1;
        BaseCommerceReceipts[slot] = 0;
        BaseCommercePartnerReceipts[slot] = 0;
        if (partner == faction ||
            ((PlayersData[faction].diplo_treaties[partner] & DTREATY_TREATY) == 0 &&
             (PlayersData[partner].diplo_treaties[faction] & DTREATY_TREATY) == 0) ||
            PlayersData[partner].current_num_bases == 0) {
            continue;
        }
        const int partner_base = base_rank(partner, commerce_rank);
        if (partner_base < 0) continue;
        int commerce_base = (Bases[partner_base].energy_intake_1 + 7 +
                             base.energy_intake_1) / 8;
        if (GameDataStateStorage.fields_.council_resolution_count_[PROP_REPEAL_GLOBAL_TRADE_PACT] <
            GameDataStateStorage.fields_.council_resolution_count_[PROP_GLOBAL_TRADE_PACT]) commerce_base *= 2;
        const int divisor = TechCommerceCount + 1;
        int partner_receipt = ((PlayersData[partner].tech_commerce_bonus + 1) *
                               commerce_base + divisor / 2) / divisor;
        int receipt = ((PlayersData[faction].tech_commerce_bonus + 1) *
                       commerce_base + divisor / 2) / divisor;
        if ((PlayersData[faction].diplo_treaties[partner] & DTREATY_PACT) == 0) {
            receipt /= 2;
            partner_receipt /= 2;
        }
        if (GameDataStateStorage.fields_.governor_faction_ == faction) ++receipt;
        if (GameDataStateStorage.fields_.governor_faction_ == partner) ++partner_receipt;
        if (PlayersData[faction].sanction_turns != 0 ||
            PlayersData[partner].sanction_turns != 0 ||
            (Players[faction].rule_flags & RFLAG_ALIEN) != 0 ||
            (Players[partner].rule_flags & RFLAG_ALIEN) != 0) {
            receipt = 0;
            partner_receipt = 0;
        }
        BaseCommerceReceipts[slot] = receipt;
        BaseCommercePartnerReceipts[slot] = partner_receipt;
        base_energy_alien_receipt_subtotal = receipt;
        base.energy_intake_2 += receipt;
    }
    if (ExpansionEnabled != 0 && (Players[faction].rule_flags & RFLAG_ALIEN) != 0) {
        for (int facility = 0; facility < 0x46; ++facility) {
            if ((base.facilities_built[facility >> 3] & (1u << (facility & 7))) != 0)
                ++base_energy_alien_receipt_subtotal;
        }
        for (int project = 0; project < 64; ++project) {
            if (GameDataStateStorage.fields_.secret_project_[project] == base_energy_selected_ordinal)
                base_energy_alien_receipt_subtotal += 5;
        }
        base_energy_alien_receipt_subtotal /= 2;
        if (PlayersData[faction].sanction_turns != 0) base_energy_alien_receipt_subtotal = 0;
        base.energy_intake_2 += base_energy_alien_receipt_subtotal;
    }
    if (BaseUpkeepStage == 1) {
        int reserve_credit = base.energy_surplus;
        if (reserve_credit < 0) reserve_credit = 0;
        else if (reserve_credit > 99999) reserve_credit = 99999;
        PlayersData[faction].unk_13 += reserve_credit;
    }
    available = base.energy_intake_2 - base.energy_consumption;
    base.energy_inefficiency = black_market(available);
    base.energy_surplus = available - base.energy_inefficiency;
    int distributed = base.energy_surplus;
    if (distributed < 0) distributed = 0;
    if (distributed > 9999) distributed = 9999;

    const int psych_share = PlayersData[faction].se_alloc_psych;
    const int labs_share = PlayersData[faction].se_alloc_labs;
    int psych = (psych_share * distributed + 4) / 10;
    if (psych < 0) psych = 0;
    if (psych > distributed) psych = distributed;
    int economy = ((10 - labs_share - psych_share) * distributed + 4) / 10;
    if (economy < 0) economy = 0;
    if (economy > distributed - psych) economy = distributed - psych;
    base.psych_total = psych;
    base.economy_total = economy;
    base.labs_total = distributed - psych - economy;

    // The extra allocation is retained separately by the original pass.
    int research_allocation = ((9 - labs_share - psych_share) * distributed + 4) / 10;
    if (research_allocation < 0) research_allocation = 0;
    if (research_allocation > distributed - psych) research_allocation = distributed - psych;
    base.unk_10 = research_allocation;

    for (int citizen = 0; citizen < base.specialist_total; ++citizen) {
        int specialist = citizen < 16
            ? (base.spec[citizen >> 3] >> ((citizen & 7) * 4)) & 15
            : static_cast<int>(best_specialist());
        if (specialist < 0) specialist = 0;
        if (specialist > 6) specialist = 6;
        if (has_tech(Citizen[specialist].obsol_tech, faction)) {
            for (int candidate = 0; candidate < 7; ++candidate) {
                if (has_tech(Citizen[candidate].preq_tech, faction)
                    && !has_tech(Citizen[candidate].obsol_tech, faction)
                    && Citizen[specialist].ops_bonus <= Citizen[candidate].ops_bonus
                    && Citizen[specialist].psych_bonus <= Citizen[candidate].psych_bonus
                    && Citizen[specialist].research_bonus <= Citizen[candidate].research_bonus) {
                    specialist = candidate;
                    if (citizen < 16) {
                        const int shift = (citizen & 7) * 4;
                        base.spec[citizen >> 3] &= ~(0xFu << shift);
                        base.spec[citizen >> 3] |= specialist << shift;
                    }
                    break;
                }
            }
        }
        base.economy_total += Citizen[specialist].ops_bonus;
        base.psych_total += Citizen[specialist].psych_bonus;
        base.labs_total += Citizen[specialist].research_bonus;
    }

    base.labs_total += Players[faction].rule_research;
    if (has_fac(FAC_BIOLOGY_LAB, BaseIDCurrentSelected, 0)) base.labs_total += 2;

    int psych_multiplier = 4;
    if (has_fac(FAC_HOLOGRAM_THEATRE, BaseIDCurrentSelected, 0)) psych_multiplier = 6;
    int labs_multiplier = (has_fac(FAC_NETWORK_NODE, BaseIDCurrentSelected, 0)
        || GameDataStateStorage.fields_.secret_project_[SP_VIRTUAL_WORLD] == BaseIDCurrentSelected) ? 6 : 4;
    int economy_multiplier = (has_fac(FAC_ENERGY_BANK, BaseIDCurrentSelected, 0)
        || GameDataStateStorage.fields_.secret_project_[SP_PLANETARY_ENERGY_GRID] == BaseIDCurrentSelected) ? 6 : 4;
    if (has_fac(FAC_RESEARCH_HOSPITAL, BaseIDCurrentSelected, 0)) {
        psych_multiplier += 1;
        labs_multiplier += 2;
    }
    if (has_fac(FAC_NANOHOSPITAL, BaseIDCurrentSelected, 0)) {
        psych_multiplier += 1;
        labs_multiplier += 2;
    }
    if (has_fac(FAC_TREE_FARM, BaseIDCurrentSelected, 0)) {
        psych_multiplier += 2;
        economy_multiplier += 2;
    }
    if (has_fac(FAC_HYBRID_FOREST, BaseIDCurrentSelected, 0)) {
        psych_multiplier += 2;
        economy_multiplier += 2;
    }
    if (has_fac(FAC_FUSION_LAB, BaseIDCurrentSelected, 0)) {
        economy_multiplier += 2;
        labs_multiplier += 2;
    }
    if (has_fac(FAC_QUANTUM_LAB, BaseIDCurrentSelected, 0)) {
        economy_multiplier += 2;
        labs_multiplier += 2;
    }
    base.psych_total = (base.psych_total * psych_multiplier + 3) / 4;
    base.labs_total = (base.labs_total * labs_multiplier + 3) / 4;
    base.economy_total = (base.economy_total * economy_multiplier + 3) / 4;
    base.unk_10 = (base.unk_10 * economy_multiplier + 3) / 4;

    if (GameDataStateStorage.fields_.secret_project_[SP_SUPERCOLLIDER] == BaseIDCurrentSelected)
        base.labs_total *= 2;
    if (GameDataStateStorage.fields_.secret_project_[SP_THEORY_OF_EVERYTHING] == BaseIDCurrentSelected)
        base.labs_total *= 2;
    if (GameDataStateStorage.fields_.secret_project_[SP_SPACE_ELEVATOR] == BaseIDCurrentSelected)
        base.economy_total *= 2;
    if (GameDataStateStorage.fields_.secret_project_[SP_LONGEVITY_VACCINE] == BaseIDCurrentSelected
        && PlayersData[faction].soc_category_pending.models_[SocialCategory::ECONOMICS] == 1)
        base.economy_total += base.economy_total / 2;

    if (GameDataStateStorage.fields_.secret_project_[SP_NETWORK_BACKBONE] == BaseIDCurrentSelected) {
        for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
            if ((Bases[base_id].facilities_built[FAC_NETWORK_NODE >> 3]
                 & (1u << (FAC_NETWORK_NODE & 7))) != 0)
                ++base.labs_total;
        }
        if ((Players[faction].rule_flags & RFLAG_ALIEN) != 0)
            base.labs_total += base_energy_alien_receipt_subtotal;
        if (PlayersData[faction].sanction_turns == 0) {
            for (int partner = 1; partner < MaxPlayerNum; ++partner) {
                if (PlayersData[partner].sanction_turns == 0
                    && (Players[partner].rule_flags & RFLAG_ALIEN) == 0
                    && (Players[faction].rule_flags & RFLAG_ALIEN) == 0)
                    base.labs_total += BaseCommerceReceipts[partner - 1];
            }
        }
    }

    int commerce_delta = psych_share + 2 * labs_share - 10;
    if (commerce_delta < 0) commerce_delta = (5 - labs_share) * 2 - psych_share;
    commerce_delta /= 2;
    const int raw_efficiency = PlayersData[faction].soc_effect_pending.values_[SocialEffect::EFFICIENCY];
    int clamped_efficiency = raw_efficiency;
    if (clamped_efficiency < -4) clamped_efficiency = -4;
    if (clamped_efficiency > 4) clamped_efficiency = 4;
    int efficiency_factor = (4 - clamped_efficiency) * 2;
    if (commerce_delta != 0 && efficiency_factor != 0) {
        int penalty;
        if ((10 - labs_share) - psych_share < labs_share) {
            penalty = efficiency_factor * commerce_delta;
            if (penalty < 0) penalty = 0;
            else if (penalty > 40) penalty = 40;
            base.labs_total = ((100 - penalty) * base.labs_total + 50) / 100;
            penalty *= 2;
            if (penalty < 0) penalty = 0;
            else if (penalty > 80) penalty = 80;
            base.economy_total = ((100 - penalty) * base.economy_total + 50) / 100;
        } else {
            penalty = efficiency_factor * commerce_delta * 2;
            if (penalty < 0) penalty = 0;
            else if (penalty > 80) penalty = 80;
            base.labs_total = ((100 - penalty) * base.labs_total + 50) / 100;
            penalty = efficiency_factor * commerce_delta;
            if (penalty < 0) penalty = 0;
            else if (penalty > 40) penalty = 40;
            base.economy_total = ((100 - penalty) * base.economy_total + 50) / 100;
        }
    }
    if (base.queue_production_id[0] == -0x45 && base.mineral_surplus > 0
        && base.production_id_last == -0x45) {
        int special_energy = (base.mineral_surplus + 1) / 2;
        const int grid_base = GameDataStateStorage.fields_.secret_project_[SP_PLANETARY_ENERGY_GRID];
        if (grid_base >= 0 && Bases[grid_base].faction_id_current == faction)
            special_energy = (special_energy * 5 + 3) / 4;
        base.economy_total += special_energy;
    }
    base_psych();
}

// ORIGINAL: 0x004E80B0 ?base_yield@@YAXXZ 0x004E80B0-0x004E954B
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004e80b0.cpp", "source:src/base.h", "image:0x004E80CC-0x004E80D9 signed population plus one publishes90E938;0x004E86D9 resets90E900 only on allocation, whereas0x004E8456 jumps to0x004E923D and retains it.", "image:0x004E83D9-0x004E8426 four resource calls per admitted tile;0x004E8E0C-0x004E923D performs one replacement with fresh-call undo;0x004E930C-0x004E954B publishes mask/specialists/intakes and accounts for all21valid radius positions."], "reason": "Recover the complete selected-base worker and specialist allocation pass with typed resource fields, real shared counters, original selection and replacement behavior, and final ecology accounting. Retain its seed identity and evidence; the former link-only placeholder is replaced, not accepted by a source binding."}
// TRIED: literal register-machine transliteration (pseudo-regs eax..edi + stack slots v04..v58, goto per basic block); covers ~100% of instructions (5804 rebuilt vs 5275 original bytes). cdq/sub/sar-1 divide-by-2 idioms folded to `/2`; neg/sbb/not boolean tricks folded to ternaries; magic-const /3 folded to `/3`. Divergence is from stack/local layout (more temporaries than the original's register allocation), not from control flow or field offsets.
void __cdecl base_yield() {
    // Original domain: live selected base/map, valid specialist encodings and
    // population counts, and a positive nutrient requirement for IDIV. The
    // original does not guard malformed data or the zero-divisor rules case.
    farm_compute();
    Base *base = BaseCurrent;
    const int faction = base->faction_id_current;
    const uint32_t faction_bit = 1u << faction;
    BasePopulationUnassigned = base->population_size + 1;
    const bool automatic = (static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & faction_bit) == 0
        || (base->governor & 0x80000040u) == 0x80000040u;
    uint32_t worked = 0;
    base->state &= ~0x8000u;
    BaseCurrent->specialist_unk_1 = 0;
    BaseCurrent->resource_sqr_radius &= ~FarmUnavailableMask | 1u;

    int resource;
    for (resource = 0; resource < 4; ++resource) {
        if (resource < 3) {
            base_yield_intake(*BaseCurrent, resource, true) = base_yield_satellite(faction, resource);
            const bool aerospace = base_yield_has_facility(29);
            bool full_yield = aerospace;
            if (!full_yield && GameDataStateStorage.fields_.secret_project_.space_elevator() >= 0) {
                full_yield = Bases[GameDataStateStorage.fields_.secret_project_.space_elevator()].faction_id_current == faction;
            }
            if (!full_yield && GameDataStateStorage.fields_.secret_project_.cloudbase_academy() >= 0) {
                full_yield = Bases[GameDataStateStorage.fields_.secret_project_.cloudbase_academy()].faction_id_current == faction;
            }
            if (!full_yield) {
                int &intake = base_yield_intake(*BaseCurrent, resource, true);
                intake = base_yield_add(intake, 1) / 2;
            }
            Base *const selected = BaseCurrent;
            int &intake = base_yield_intake(*selected, resource, true);
            if (selected->population_size < intake) intake = selected->population_size;
        } else {
            base_yield_intake(*BaseCurrent, resource, true) = 0;
        }
    }

    bool reuse_complete = false;
    int map_width = MapLongitudeBounds;
    if (!automatic || UnkGlobal0093A934 == 0) {
        base = BaseCurrent;
        if (automatic) {
            for (int specialist = 0; specialist < base->specialist_total; ++specialist) {
                const int type = base_yield_specialist(base, specialist);
                if (Citizen[type].psych_bonus == 0) {
                    ++base->specialist_unk_1;
                    base = BaseCurrent;
                }
            }
            base->specialist_total -= base->specialist_unk_1;
        }
        const int used = bit_count(BaseCurrent->resource_sqr_radius);
        if (BasePopulationUnassigned >= used) {
            base = BaseCurrent;
            map_width = MapLongitudeBounds;
            for (int position = 0; position < 21; ++position) {
                const uint32_t bit = 1u << position;
                if (position != 0 && (base->resource_sqr_radius & bit) == 0) continue;
                worked |= bit;
                const int x = base_yield_wrap_x(base->x + RadiusOffsetX[position], map_width);
                const int y = base->y + RadiusOffsetY[position];
                base_yield_apply_tile(base, map_width, x, y, false);
                if (--BasePopulationUnassigned == 0) break;
            }
            reuse_complete = BasePopulationUnassigned
                == base->specialist_total + base->specialist_unk_1;
        }
    }

    if (!reuse_complete) {
        base = BaseCurrent;
        map_width = MapLongitudeBounds;
        if (automatic && base->specialist_total != 0) {
            // Preserve the original compaction: shift through the old endpoint,
            // then advance the outer index instead of re-testing the replacement.
            for (int index = 0; index < base->specialist_total + base->specialist_unk_1; ++index) {
                const int type = base_yield_specialist(base, index);
                if (index >= 16) map_width = MapLongitudeBounds;
                if (Citizen[type].psych_bonus == 0) {
                    for (int shift = index; shift < base->specialist_total + base->specialist_unk_1; ++shift) {
                        const int next = base_yield_specialist(base, shift + 1);
                        if (shift + 1 >= 16) map_width = MapLongitudeBounds;
                        base_yield_set_specialist(base, shift, next);
                        if (shift < 16) map_width = MapLongitudeBounds;
                    }
                    if (base->specialist_unk_1 != 0) {
                        --base->specialist_unk_1;
                        base = BaseCurrent;
                        map_width = MapLongitudeBounds;
                    }
                }
            }
        }
        if ((worked & 1) == 0) {
            const int x = base_yield_wrap_x(base->x + RadiusOffsetX[0], map_width);
            const int y = base->y + RadiusOffsetY[0];
            base_yield_apply_tile(base, map_width, x, y, false);
            worked |= 1;
            --BasePopulationUnassigned;
        }

        // This reset is intentionally absent from the completed-reuse path.
        BaseNutrientAdequate = 0;
        int fight = 0, growth = 0, tech = 0, wealth = 0, power = 0;
        if ((static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & faction_bit) != 0) {
            if ((base->governor & 0x80000000u) != 0) {
                growth = (base->governor >> 24) & 1;
                tech = (base->governor >> 24) & 2;
                wealth = (base->governor >> 25) & 2;
                power = (base->governor >> 26) & 2;
                fight = power ? 2 : ((growth || wealth) ? 0 : (tech ? -2 : PlayersData[faction].ai_fight));
            }
        } else {
            fight = PlayersData[faction].ai_fight;
            growth = PlayersData[faction].ai_growth;
            tech = PlayersData[faction].ai_tech;
            wealth = PlayersData[faction].ai_wealth;
            power = PlayersData[faction].ai_power;
        }

        for (;;) {
            const bool replacing = BasePopulationUnassigned <= base->specialist_total + base->specialist_unk_1;
            const uint32_t excluded = replacing ? 0 : worked;
            int best_position = 0, best_score = 0;
            int worst_position = -1, worst_score = 9999;
            int max_nutrients = 0;
            for (int position = 1; position < 21; ++position) {
                const uint32_t bit = 1u << position;
                if ((bit & (excluded | FarmUnavailableMask)) != 0) continue;
                const int x = base_yield_wrap_x(base->x + RadiusOffsetX[position], map_width);
                const int y = base->y + RadiusOffsetY[position];
                if (y < 0 || y >= MapLatitudeBounds || x < 0 || x >= map_width) continue;
                int nutrients = crop_yield(base->faction_id_current, BaseIDCurrentSelected, x, y, 0);
                int minerals = mine_yield(BaseCurrent->faction_id_current, BaseIDCurrentSelected, x, y, 0);
                int energy = energy_yield(BaseCurrent->faction_id_current, BaseIDCurrentSelected, x, y, 0);
                if (nutrients > max_nutrients) max_nutrients = nutrients;
                base = BaseCurrent;
                const int population = base->population_size;
                const int requirement = static_cast<int>(Rules.nutrient_req_citizen);
                const int needed = base_yield_mul(population, requirement);
                const int intake = base->nutrient_intake_2;
                int food_bonus = 0;
                if (base_yield_add(BasePopulationUnassigned / requirement, intake) < needed) {
                    food_bonus = base_yield_add(intake, BasePopulationUnassigned) >= needed ? 2 : 8;
                    const int extra = nutrients < 0 ? 0 : (nutrients < food_bonus ? nutrients : food_bonus);
                    nutrients = base_yield_add(nutrients, extra);
                } else {
                    BaseNutrientAdequate = 1;
                }
                nutrients = base_yield_add(nutrients, nutrients);
                minerals = base_yield_add(minerals, minerals);
                energy = base_yield_add(energy, energy);
                if (intake <= base_yield_add(population, needed) && nutrients == 0) {
                    minerals /= 2;
                    energy /= 2;
                }
                if (minerals == 0 && food_bonus == 0) nutrients /= 2;
                int denominator = base_yield_add(base_yield_sub(base_yield_sub(intake, population / 3), needed), BasePopulationUnassigned);
                denominator = base_yield_denominator(base_yield_sub(base_yield_add(denominator, denominator), 2));
                int score = base_yield_shift(base_yield_mul(base_yield_add(growth, 4), nutrients), 8) / denominator;
                const int mineral_priority = base->queue_production_id[0] < -70 ? 2 : base_yield_sub(0, tech);
                denominator = base_yield_denominator(base_yield_add(base_yield_sub(base->mineral_intake_2,
                    base_yield_signed_bits(BaseCurrentForcesMaintCost)), 1) / 2);
                int weight = base_yield_add(base_yield_add(base_yield_add(fight, mineral_priority), power), wealth);
                weight = base_yield_add(weight, 6);
                score = base_yield_add(score, base_yield_shift(base_yield_mul(base_yield_mul(weight, population), minerals), 6) / denominator);
                weight = base_yield_add(base_yield_add(wealth, base_yield_mul(tech, 2)), 1);
                const int energy_population = base_yield_denominator(base_yield_sub(population, power));
                score = base_yield_add(score, base_yield_shift(base_yield_mul(base_yield_mul(weight, energy_population), energy), 6)
                    / base_yield_denominator(base->energy_intake_2));
                if ((worked & bit) != 0) {
                    if (score < worst_score) { worst_score = score; worst_position = position; }
                } else if (score > best_score) {
                    best_score = score;
                    best_position = position;
                }
                map_width = MapLongitudeBounds;
            }

            if (automatic && base->population_size >= 5
                && (base->nutrient_intake_2 > base_yield_mul(base->population_size, static_cast<int>(Rules.nutrient_req_citizen))
                    || max_nutrients == 0) && GameDataStateStorage.fields_.turn_current_num_ > 100) {
                for (int specialist = 0; specialist < 7; ++specialist) {
                    if (has_tech(Citizen[specialist].preq_tech, faction)
                        && !has_tech(Citizen[specialist].obsol_tech, faction)) {
                        base = BaseCurrent;
                        const int population = base->population_size;
                        const int denominator = base_yield_denominator(base_yield_add(base->energy_intake_2, base->specialist_unk_1));
                        int score = base_yield_shift(Citizen[specialist].psych_bonus, 6);
                        int weight = base_yield_mul(base_yield_add(wealth, 2), population);
                        score = base_yield_add(score, base_yield_shift(base_yield_mul(weight, Citizen[specialist].ops_bonus), 6) / denominator);
                        weight = base_yield_mul(base_yield_add(tech, 2), population);
                        score = base_yield_add(score, base_yield_shift(base_yield_mul(weight, Citizen[specialist].research_bonus), 6) / denominator);
                        if (score > best_score) { best_score = score; best_position = -specialist - 1; }
                    } else {
                        base = BaseCurrent;
                    }
                }
                map_width = MapLongitudeBounds;
            }

            if (!replacing) {
                if (best_position < 0) {
                    base_yield_set_specialist(base, base->specialist_unk_1, -best_position - 1);
                    ++base->specialist_unk_1;
                    base = BaseCurrent;
                    map_width = MapLongitudeBounds;
                } else if (best_position > 0) {
                    worked |= 1u << best_position;
                    const int x = base_yield_wrap_x(base->x + RadiusOffsetX[best_position], map_width);
                    const int y = base->y + RadiusOffsetY[best_position];
                    base_yield_apply_tile(base, map_width, x, y, false);
                    --BasePopulationUnassigned;
                } else {
                    break;
                }
                if (BasePopulationUnassigned > base->specialist_total + base->specialist_unk_1) continue;
                if (base->nutrient_intake_2 >= base_yield_mul(base->population_size,
                        static_cast<int>(Rules.nutrient_req_citizen))) break;
            } else {
                if (best_position <= 0 || best_score <= worst_score || worst_position <= 0) break;
                const uint32_t old_bit = 1u << worst_position;
                const uint32_t new_bit = 1u << best_position;
                worked = (worked & ~old_bit) | new_bit;
                int x = base_yield_wrap_x(base->x + RadiusOffsetX[worst_position], map_width);
                int y = base->y + RadiusOffsetY[worst_position];
                base_yield_apply_tile(base, map_width, x, y, true);
                x = base_yield_wrap_x(base->x + RadiusOffsetX[best_position], map_width);
                y = base->y + RadiusOffsetY[best_position];
                base_yield_apply_tile(base, map_width, x, y, false);
                if (base->nutrient_intake_2 < base_yield_mul(base->population_size + 1,
                        static_cast<int>(Rules.nutrient_req_citizen))) {
                    worked = (worked & ~new_bit) | old_bit;
                    x = base_yield_wrap_x(base->x + RadiusOffsetX[best_position], map_width);
                    y = base->y + RadiusOffsetY[best_position];
                    base_yield_apply_tile(base, map_width, x, y, true);
                    x = base_yield_wrap_x(base->x + RadiusOffsetX[worst_position], map_width);
                    y = base->y + RadiusOffsetY[worst_position];
                    base_yield_apply_tile(base, map_width, x, y, false);
                }
                break;
            }
        }
    }

    // Fill remaining specialist slots. Unlike candidate scoring, this pass
    // checks prerequisites only, and weights psych three times as heavily.
    for (int index = base->specialist_total; index < BasePopulationUnassigned; ++index) {
        int best = 0, best_bonus = -999;
        for (int specialist = 0; specialist < 7; ++specialist) {
            const bool known = has_tech(Citizen[specialist].preq_tech, base->faction_id_current);
            base = BaseCurrent;
            if (known) {
                int bonus = base_yield_mul(Citizen[specialist].psych_bonus, 3);
                if (base->population_size >= static_cast<int>(Rules.min_base_size_specialists)) {
                    bonus = base_yield_add(bonus, base_yield_add(Citizen[specialist].ops_bonus, Citizen[specialist].research_bonus));
                }
                if (bonus > best_bonus) { best_bonus = bonus; best = specialist; }
            }
        }
        base_yield_set_specialist(base, index, best);
    }
    base->resource_sqr_radius = worked;
    BaseCurrent->specialist_total = BasePopulationUnassigned;
    BaseCurrent->state &= ~0x100u;
    for (resource = 0; resource < 4; ++resource) {
        Base *const selected = BaseCurrent;
        base_yield_intake(*selected, resource, false) = base_yield_intake(*selected, resource, true);
    }

    BaseCurrent->eco_damage = 0;
    for (int position = 0; position < 21; ++position) {
        base = BaseCurrent;
        map_width = MapLongitudeBounds;
        const int x = base_yield_wrap_x(base->x + RadiusOffsetX[position], map_width);
        const int y = base->y + RadiusOffsetY[position];
        if (y < 0 || y >= MapLatitudeBounds || x < 0 || x >= map_width) continue;
        const uint32_t improvements = map_tiles()[static_cast<int>(MapLongitude) * y + (x >> 1)].bit;
        const bool used = (base->resource_sqr_radius & (1u << position)) != 0;
        int damage = bit_count(improvements & 0x1C8805Cu);
        if (used) damage += damage;
        if ((improvements & 0x1000000) != 0) damage += 8;
        if ((improvements & 0x800000) != 0) damage += 6;
        if ((improvements & 0x400000) != 0) damage += 4;
        if ((improvements & 0x200000) != 0) --damage;
        if (base_yield_has_facility(13) && base_yield_has_facility(14)) damage = 0;
        if (base_yield_has_facility(13) || base_yield_has_facility(14)) damage /= 2;
        BaseCurrent->eco_damage = base_yield_add(BaseCurrent->eco_damage, damage);
    }
}

namespace {

// The original repeats this choice in each direction of its worker/specialist
// search. Keep the scoring and packed storage in Base's own representation.
int base_doctors_best_specialist() {
    int best = CITIZEN_TECHNICIAN;
    int best_score = -999;
    for (int type = 0; type < MaxSpecialistNum; ++type) {
        Base *base = BaseCurrent;
        if (!has_tech(Citizen[type].preq_tech, base->faction_id_current)) continue;
        base = BaseCurrent;
        int score = Citizen[type].psych_bonus * 3;
        if (base->population_size >= static_cast<int>(Rules.min_base_size_specialists)) {
            score += Citizen[type].ops_bonus + Citizen[type].research_bonus;
        }
        if (score > best_score) {
            best = type;
            best_score = score;
        }
    }
    return best;
}

void base_doctors_add_specialist() {
    const int best = base_doctors_best_specialist();
    Base *base = BaseCurrent;
    const int slot = base->specialist_total++;
    if (slot < 16) {
        const unsigned int shift = (static_cast<unsigned int>(slot) & 7u) * 4;
        const int word_index = slot >> 3;
        uint32_t &word = base->spec[word_index];
        word = (word & ~(15u << shift))
             | (static_cast<uint32_t>(best) << shift);
    }
}

} // namespace

// ORIGINAL: 0x004F5F30 ?base_doctors@@YAXXZ 0x004F5F30-0x004F650C
// RECOVERY_BINDING: v1 {"evidence": ["git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/unrecovered/004f5f30.cpp", "source:src/base.h"], "reason": "Recover the selected Base's complete specialist/worker adjustment, riot eligibility, Matrix-gated removal and console warning paths through typed Base, citizen, Rules and project state. Preserve selected-base versus entry-faction distinctions and live BaseCurrent reloads across callbacks. Original unsafe accesses, full identity, seed history and matching/restoration obligations remain; no byte, ABI or runtime acceptance is implied."}
// kind      game
// TRIED: near-verbatim transcription of the (clean, non-SEH) Ghidra output with DAT_/FUN_ substituted for the catalogued globals and callees, and `*(char**)g_0090ea30` for the base-pointer dereference; the two duplicated "pick best doctor" loops are written out twice (matching the original's own duplication) rather than factored into a helper, to avoid the one-external-symbol risk seen on 0x00405140. Reaches MISMATCH #6 ('xor' vs 'push'); did not chase the exact register/stack layout further given the function's size.
void __cdecl base_doctors() {
    Base *base = BaseCurrent;
    const int faction = base->faction_id_current;
    int riot_doctor_adjustment = 0;

    if ((base->state & BSTATE_DRONE_RIOTS_ACTIVE) != 0) {
        const int brood_pit = has_fac(FAC_BROOD_PIT, BaseIDCurrentSelected, 0) ? 1 : 0;
        const int police = PlayersData[faction].soc_effect_pending.values_[SocialEffect::POLICE]
                         + brood_pit * 2;
        if (police < -1) {
            const int selected_base_id = BaseIDCurrentSelected;
            Base *selected_base = &Bases[selected_base_id];
            const int selected_faction = selected_base->faction_id_current;
            const int ascetic_owner = base_project(SP_ASCETIC_VIRTUES);
            const int ascetic_reduction = ascetic_owner >= 0
                && Bases[ascetic_owner].faction_id_current == selected_faction ? 2 : 0;
            const int population_pressure = static_cast<int>(selected_base->population_size)
                - ascetic_reduction + Players[selected_faction].rule_population;
            if ((population_pressure < static_cast<int>(Rules.pop_limit_sans_hab_complex)
                 || has_fac(FAC_HAB_COMPLEX, selected_base_id, 0))
                && (population_pressure < static_cast<int>(Rules.pop_limit_sans_hab_dome)
                    || has_fac(FAC_HABITATION_DOME, selected_base_id, 0))) {
                const int cloning_vats_owner = base_project(SP_CLONING_VATS);
                const bool cloning_vats_for_faction = cloning_vats_owner >= 0
                    && Bases[cloning_vats_owner].faction_id_current == faction;
                bool eligible = false;
                if ((cloning_vats_for_faction || BaseCurrentGrowthRate > 5)
                    && Rules.nutrient_req_citizen != 0
                    && static_cast<int>(Rules.nutrient_req_citizen)
                        <= BaseCurrent->nutrient_surplus) {
                    eligible = true;
                    base = BaseCurrent;
                } else {
                    const int nutrient_cost = cost_factor(
                        faction, 0, BaseIDCurrentSelected);
                    base = BaseCurrent;
                    if (nutrient_cost * (static_cast<int>(base->population_size) + 1)
                        <= base->nutrients_accumulated + base->nutrient_surplus) {
                        eligible = true;
                    }
                }
                if (eligible) riot_doctor_adjustment = 1;
            }
        }
    }
    // Native branch setup reloads the current Base after each callback-bearing
    // riot path, including the failed-pressure and failed-facility branches.
    base = BaseCurrent;

    const unsigned int faction_bit = 1u << (static_cast<unsigned int>(faction) & 0x1Fu);
    if ((static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)
         & faction_bit) != 0
        && (base->governor & (GOV_MANAGES_CITIZENS_SPECS | GOV_ACTIVE))
            != (GOV_MANAGES_CITIZENS_SPECS | GOV_ACTIVE)) {
        return;
    }

    int loop_counter = 1;
    for (;;) {
        if ((static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)
             & faction_bit) != 0) {
            if ((base->governor & GOV_MANAGES_CITIZENS_SPECS) == 0) return;
            if ((base->governor & GOV_ACTIVE) == 0) return;
        }

        const int matrix_owner = base_project(SP_TELEPATHIC_MATRIX);
        const bool matrix_for_faction = matrix_owner >= 0
            && Bases[matrix_owner].faction_id_current == faction;
        if (BaseNutrientAdequate == 0
            || base->drone_total + riot_doctor_adjustment <= base->talent_total
            || matrix_for_faction) {
            const int old_specialist_total = base->specialist_total;
            if (old_specialist_total == base->specialist_unk_1
                || old_specialist_total - base->specialist_unk_1 < 0) return;
            if ((base->state & BSTATE_DRONE_RIOTS_ACTIVE) != 0) return;

            if (base->drone_total != 0
                && base->talent_total <= base->drone_total + riot_doctor_adjustment) {
                const int removal_matrix_owner = base_project(SP_TELEPATHIC_MATRIX);
                if (removal_matrix_owner < 0
                    || Bases[removal_matrix_owner].faction_id_current != faction) return;
            }

            base->specialist_total = old_specialist_total - 1;
            BaseComputedID = BaseIDCurrentSelected;
            base_support();
            base_yield();
            base_nutrient();
            base_minerals();
            base_energy();
            base = BaseCurrent;
            const int post_removal_matrix_owner = base_project(SP_TELEPATHIC_MATRIX);
            if (base->drone_total + riot_doctor_adjustment > base->talent_total
                && (post_removal_matrix_owner < 0
                    || Bases[post_removal_matrix_owner].faction_id_current != faction)) {
                base_doctors_add_specialist();
                return;
            }
            if (old_specialist_total <= base->specialist_total) return;
        } else {
            base_doctors_add_specialist();
            BaseComputedID = BaseIDCurrentSelected;
            base_support();
            base_yield();
            base_nutrient();
            base_minerals();
            base_energy();
            base = BaseCurrent;

            if (base->drone_total + riot_doctor_adjustment <= base->talent_total) {
                if (UnkGlobal0093A934 == 0) return;
                if (faction != ConsoleGlobal.faction_id_) return;
                if (PlayersData[faction].se_alloc_psych > 1) return;
                if (base->specialist_total - base->specialist_unk_1 < 2) return;
                const int production = base->queue_production_id[0];
                if (production == -6 || production == -11 || production == -12
                    || production == -17 || production == -18) return;
                if (production == 8) {
                    const int virtual_world_owner = base_project(SP_VIRTUAL_WORLD);
                    if (virtual_world_owner >= 0
                        && Bases[virtual_world_owner].faction_id_current == faction) return;
                }
                if ((Players[faction].rule_flags & RFLAG_ALIEN) != 0) {
                    popb("PSYCHREQUEST", 0, -1, "Alopdir.pcx", 0);
                } else {
                    popb("PSYCHREQUEST", 0, -1, "talent_sm.pcx", 0);
                }
                return;
            }
        }

        ++loop_counter;
        if (loop_counter > 9) return;
    }
}

/*
Purpose: Reset every base of one faction: the faction's bases are visited in
         table order and each is handed to base_reset when the caller's
         context allows it - the faction is not locked out by the human/alive
         status bytes, the base optionally lies in the requested region, and
         with the occupancy flag the tile must be free of vehicles or without
         a current owner.

// ORIGINAL: 0x004E6300 ?bases_reset@@YAXHHH@Z 0x004E6300-0x004E63FC FILE
// RECOVERY_BINDING: v1 {"evidence":["src/base.h declares bases_reset and owns the actual Base array and BaseCurrent; its Base members x/y/faction_id_current/state have RECOVERY_MEMBER evidence.","The pinned map-record stride is 0x2C; src/map.h's Map is size 0x2C with climate/contour/val2/region at +0..+3 and bit at +8. Typed indexing preserves the original field reads.","The body keeps both live enable-mask reads, the original short-circuit order, the base_reset callback, and the post-callback MapTiles/MapLongitude reload. No alias or manual table is introduced."],"reason":"Move the existing reset loop into the Base owner and express its already-established Base/Map storage through actual members while preserving branch, callback, reload, and unsafe-input behavior."}
// TRIED: single AND-chained `if` (faction==a2 && (a1<0||idx_byte==a1) && (a3==0||bits check) && (mask flag||special)) reproduces the loop body; `int mask`/`unsigned char mask` both leave the divergence at MISMATCH #2 (original reserves the one local with `push ecx` right after `mov ebp,esp`; every C form tried here gets `sub esp,4` instead) - same prologue-reservation wall as the mon_* family (0x00477840).
// size      252 bytes
// callers   4   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E5FE0
// notes     Seed provenance (TRIED): the single AND-chained `if` reproduces
//           the loop body; the residual divergence is the prologue local
//           reservation - the original reserves its one local with `push ecx`
//           right after `mov ebp,esp`, and every C form tried reaches for
//           `sub esp,4` (the mon_* family wall at 0x00477840).
Return Value: n/a
Status: Coverage transcription - unclaimed body; the seed
        git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004e6300.cpp is retained until this product
        definition measures fresh BYTE_EXACT.
*/
void __cdecl bases_reset(int region_id, int faction_id, int skip_occupied) {
    int mask = 1 << faction_id;
    if (((unsigned char)mask &
         *reinterpret_cast<const unsigned char *>(
             &GameDataStateStorage.fields_.lock_masks_.enable_mask_)) == 0) {
        const int count = GameDataStateStorage.fields_.base_current_count_;
        int i = 0;
        if (count > 0) {
            Map *tiles = map_tiles();
            int mult = static_cast<int>(MapLongitude);
            Base *base = Bases;
            do {
                if ((base->faction_id_current == faction_id) &&
                    (region_id < 0 ||
                     tiles[base->y * mult + (base->x >> 1)].region == region_id) &&
                    (skip_occupied == 0 ||
                     (tiles[base->y * mult + (base->x >> 1)].bit & 2) == 0 ||
                     7 < (tiles[base->y * mult + (base->x >> 1)].val2 & 0xf)) &&
                    (((unsigned char)mask &
                      *reinterpret_cast<const unsigned char *>(
                          &GameDataStateStorage.fields_.lock_masks_.enable_mask_)) == 0 ||
                     (BaseCurrent->state & 0x80000000) == 0)) {
                    base_reset(i, 0);
                    tiles = map_tiles();
                    mult = static_cast<int>(MapLongitude);
                }
                ++i;
                ++base;
            } while (i < count);
        }
    }
}

/*
Purpose: Sum one base's terrain yields into the selected Base's intake_2
         slots: walk the base's centre plus its radius offsets, wrap the x
         edge on a flat map, latch the 0x8000 out-of-bounds state bit when
         the tile is non-fungal rock, and add each of the four yield
         passes' result into BaseCurrent's nutrient/mineral/energy/unused
         intake_2 word, returning the sum.

// ORIGINAL: 0x004E7E50 sub_4e7e50 0x004E7E50-0x004E7F63 FILE
// RECOVERY_BINDING: v1 {"evidence":["The body reads the existing BaseCurrent/Bases layout (x, y, faction, state and contiguous intake_2 words), Map records, map bounds, and declared crop_yield/mine_yield/energy_yield APIs.","The function's fourth pass deliberately clears TileYieldRestricted; the body and original pass order are carried unchanged.","The identity remains the unmangled global sub_4e7e50; no original class name is inferred from its Base dataflow."],"reason":"Place the complete existing base-radius yield routine beside its genuine Base/Map dependencies without renaming it, changing pass order, or normalizing the original state updates."}
// TRIED: various expression orderings for the x/y base+offset sums and the wrap-around guard placement all keep the divergence at the first instructions (register allocation for edx/ecx/a1 differs from the original's push-ebx/push-esi-before-movsx order).
// size      276 bytes
// prototype
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004E6E50 0x004E7310 0x004E7750
Return Value: the four-pass yield sum
Status: Coverage transcription - unclaimed body
*/
extern "C" int __cdecl sub_4e7e50(int radius_index) {
    Base *base = BaseCurrent;
    int x = static_cast<int>(base->x) + RadiusOffsetX[radius_index];
    if (!(MapIsFlat & 1)) {
        if (x < 0) {
            x += MapLongitudeBounds;
        } else if (x >= MapLongitudeBounds) {
            x -= MapLongitudeBounds;
        }
    }
    int y = static_cast<int>(base->y) + RadiusOffsetY[radius_index];
    if (y < 0 || y >= MapLatitudeBounds || x < 0 || x >= MapLongitudeBounds) {
        return 0;
    }
    const Map *tile = map_tiles() + MapLongitude * y + (x >> 1);
    if ((tile->climate & 0xE0) < 0x60) {
        BaseCurrent->state |= 0x8000;
    }
    int sum = 0;
    const uint8_t owner = base->faction_id_current;
    for (int pass = 0; pass < 4; ++pass) {
        int value;
        switch (pass) {
        case 0:
            value = crop_yield(owner, BaseIDCurrentSelected, x, y, 0);
            break;
        case 1:
            value = mine_yield(owner, BaseIDCurrentSelected, x, y, 0);
            break;
        case 2:
            value = energy_yield(owner, BaseIDCurrentSelected, x, y, 0);
            break;
        default:
            value = 0;
            TileYieldRestricted = 0;
            break;
        }
        // The image selects C0/C4/C8/CC and reloads BaseCurrent each pass.
        // These are separate declared members, not a C++ array.
        base = BaseCurrent;
        switch (pass) {
        case 0: base->nutrient_intake_2 += value; break;
        case 1: base->mineral_intake_2 += value; break;
        case 2: base->energy_intake_2 += value; break;
        default: base->unused_intake_2 += value; break;
        }
        sum += value;
    }
    return sum;
}

/*
Purpose: The sibling pass of sub_4e7e50: identical walk and tile gating,
         but each pass's yield is SUBTRACTED from the matching intake_2
         word while the same value still accumulates into the sum.

// ORIGINAL: 0x004E7F80 sub_4e7f80 0x004E7F80-0x004E8094 FILE
// RECOVERY_BINDING: v1 {"evidence":["The body reads the existing BaseCurrent/Bases layout, Map records, map bounds, and declared crop_yield/mine_yield/energy_yield APIs.","This is the subtracting sibling of 0x004E7E50; the original sum still accumulates positive yield values while intake_2 is decremented.","The identity remains the unmangled global sub_4e7f80; no original class name is inferred from its Base dataflow."],"reason":"Place the complete subtracting base-radius yield routine beside its genuine Base/Map dependencies, preserving its distinct subtraction and positive return-sum behavior."}
// TRIED: the same prologue register-allocation divergence as its 0x004E7E50 sibling.
// size      276 bytes
// prototype
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004E6E50 0x004E7310 0x004E7750
Return Value: the negated-form four-pass yield sum
Status: Coverage transcription - unclaimed body
*/
extern "C" int __cdecl sub_4e7f80(int radius_index) {
    Base *base = BaseCurrent;
    int x = static_cast<int>(base->x) + RadiusOffsetX[radius_index];
    if (!(MapIsFlat & 1)) {
        if (x < 0) {
            x += MapLongitudeBounds;
        } else if (x >= MapLongitudeBounds) {
            x -= MapLongitudeBounds;
        }
    }
    int y = static_cast<int>(base->y) + RadiusOffsetY[radius_index];
    if (y < 0 || y >= MapLatitudeBounds || x < 0 || x >= MapLongitudeBounds) {
        return 0;
    }
    const Map *tile = map_tiles() + MapLongitude * y + (x >> 1);
    if ((tile->climate & 0xE0) < 0x60) {
        BaseCurrent->state |= 0x8000;
    }
    int sum = 0;
    const uint8_t owner = base->faction_id_current;
    for (int pass = 0; pass < 4; ++pass) {
        int value;
        switch (pass) {
        case 0:
            value = crop_yield(owner, BaseIDCurrentSelected, x, y, 0);
            break;
        case 1:
            value = mine_yield(owner, BaseIDCurrentSelected, x, y, 0);
            break;
        case 2:
            value = energy_yield(owner, BaseIDCurrentSelected, x, y, 0);
            break;
        default:
            value = 0;
            TileYieldRestricted = 0;
            break;
        }
        base = BaseCurrent;
        switch (pass) {
        case 0: base->nutrient_intake_2 -= value; break;
        case 1: base->mineral_intake_2 -= value; break;
        case 2: base->energy_intake_2 -= value; break;
        default: base->unused_intake_2 -= value; break;
        }
        sum += value;
    }
    return sum;
}

/*
Purpose: Bubble-sort an id list into base-name order: repeatedly walk the
         caller's id array comparing the named Bases records case-blind,
         swapping neighbours whose names order backwards, until a pass
         makes no swap.

// ORIGINAL: 0x004D5FF0 sub_4d5ff0 0x004D5FF0-0x004D606E FILE
// RECOVERY_BINDING: v1 {"evidence":["The pinned routine repeatedly compares Bases[id].name_string using case-insensitive string comparison and swaps neighboring entries in the caller-owned id list.","The current Base array/count are source-owned in base.h/base.cpp; the global address-derived identity remains sub_4d5ff0, not a newly named Base method.","The one-element/negative-count early return, repeated bubble passes, comparator polarity, and caller-list mutation are preserved."],"reason":"Move the existing base-name id sort to the owner of its genuine Base storage while retaining the original global identity and exact comparison/swap control flow."}
// size      126 bytes
// prototype
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00628A50 0x0064FD20
Return Value: n/a
Status: Coverage transcription - unclaimed body
*/
extern "C" void __cdecl sub_4d5ff0(int *id_list) {
    bool did_swap;
    do {
        did_swap = false;
        int *cursor = id_list;
        const int count = GameDataStateStorage.fields_.base_current_count_;
        if (count == 1 || count - 1 < 0) {
            return;
        }
        for (int index = 0; index < count - 1; ++index, ++cursor) {
            if (_stricmp(Bases[cursor[0]].name_string,
                         Bases[cursor[1]].name_string) > 0) {
                did_swap = true;
                swap(cursor, cursor + 1);
            }
        }
    } while (did_swap);
}

// ORIGINAL: 0x004E4B80 ?base_init@@YAHHHH@Z 0x004E4B80-0x004E5250
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004e4b80.cpp: canonical identity, original spans and retained seed history", "source:src/base.h: real Base storage and lifecycle contracts", "source:recovery/tasks/base-lifecycle-owner-closure.json: bounded coordinator authorization"], "reason": "The original base creation operation owns initialization of Bases records and faction base counts; source expresses real Base/PlayerData fields, facility bitmaps, technology and named UI/replay dependencies."}
// TRIED: `>= 0x200` guard (matches original cmp/jl) over `> 0x1ff` moved divergence #4->#12; raw byte-offset arithmetic on the fixed 0x97dXXX/0x96XXXX globals (record stride 0x134, per-faction stride 0x20cc) rather than the Base struct - field order in src/base.h is only size-checked, not per-field, and several 0x97d0cc-family writes index past their nominal field width (a genuine original OOB-write reproduced faithfully). Not chased to byte-exact given size (1744 bytes / ~330 asm lines).
int __cdecl base_init(int faction_id, int x, int y) {
    if (GameDataStateStorage.fields_.base_current_count_ >= 512) return -1;
    Map *tile = map_loc(x, y);
    const unsigned int region = tile->region;
    bit_set(x, y, 1, 1);
    owner_set(x, y, faction_id);
    synch_bit(x, y, faction_id);
    const int base_id = GameDataStateStorage.fields_.base_current_count_++;
    PlayerData &faction = PlayersData[faction_id];
    ++faction.current_num_bases;
    ++faction.region_total_bases[region];
    replay_base(0, x, y, faction_id);
    Base &base = Bases[base_id];
    base.x = static_cast<int16_t>(x);
    base.y = static_cast<int16_t>(y);
    base.faction_id_current = base.faction_id_former = static_cast<uint8_t>(faction_id);
    base.nerve_staple_turns_left = base.assimilation_turns_left = base.unk_1 = 0;
    base.population_size = GameDataStateStorage.fields_.secret_project_.planetary_transit_system() >= 0
        && Bases[GameDataStateStorage.fields_.secret_project_.planetary_transit_system()].faction_id_current == faction_id ? 3 : 1;
    base.visibility = static_cast<uint8_t>(tile->visibility | (1u << faction_id));
    base.name_string[0] = 0;
    name_base(faction_id, base.name_string, TRUE, (tile->climate & 0xE0) < 0x60);
    base.state = base.event = 0;
    base.governor = faction.base_governor_adv;
    base.minerals_accumulated = base.nutrients_accumulated = 0;
    base.nerve_staple_count = 0;
    base.auto_forward_land_base_id = base.auto_forward_sea_base_id
        = base.auto_forward_air_base_id = -1;
    base.queue_size = 0;
    base_first(base_id);
    if (GameDataStateStorage.fields_.turn_current_num_ == 0) {
        const int production = has_tech(VehPrototypes[1].preq_tech, faction_id)
            ? 1 : (faction.diff_level > 1 ? 2 : 0);
        base.queue_production_id[0] = production;
        base.minerals_accumulated = cost_factor(faction_id, 1, -1)
            * VehPrototypes[production].cost / 2;
    } else {
        base.minerals_accumulated = faction.soc_effect_active.values_[SocialEffect::SUPPORT] < -1
            ? 0 : Rules.retool_exemption;
    }
    base.resource_sqr_radius = 0;
    base.specialist_total = base.specialist_unk_1 = 0;
    // The original clears only nine facility bytes, not the entire recycled
    // Base record. Fields it does not initialize deliberately retain their state.
    memset(base.facilities_built, 0, 9);
    base.eco_damage = 0;
    base.unk_x = base.unk_y = -1;
    base.random_event_turns = base.nerve_staple_count = base.unk_11 = base.location = 0;
    if (faction.current_num_bases == 1) {
        set_fac(1, base_id, TRUE);
        if (!IsMultiplayerNet) {
            base.population_size = static_cast<int8_t>(max(1, min(5, (GameDataStateStorage.fields_.turn_current_num_ + 49) / 50)));
            if (GameDataStateStorage.fields_.turn_current_num_ > 74) set_fac(3, base_id, TRUE);
            if (GameDataStateStorage.fields_.turn_current_num_ > 99) set_fac(6, base_id, TRUE);
            if (GameDataStateStorage.fields_.turn_current_num_ >= 150) set_fac(7, base_id, TRUE);
        } else {
            base.population_size = 3;
            if (Players[faction_id].rule_talent || faction.soc_effect_base.values_[SocialEffect::PLANET] > 0)
                base.population_size = 4;
            if (Players[faction_id].rule_drone || Players[faction_id].rule_population)
                --base.population_size;
            set_fac(3, base_id, TRUE);
            if (GameDataStateStorage.fields_.diff_level_current_ > 2) set_fac(6, base_id, TRUE);
            if (faction.soc_effect_base.values_[SocialEffect::ECONOMY] >= 1) set_fac(7, base_id, TRUE);
        }
    }
    if ((tile->climate & 0xE0) < 0x60) set_fac(26, base_id, TRUE);
    for (int viewer = 0; viewer < 8; ++viewer)
        base.faction_pop_size_intel[viewer] = viewer == faction_id
            || (base.visibility & (1u << viewer)) ? base.population_size : 0;
    int bonus;
    for (bonus = 0; bonus < Players[faction_id].faction_bonus_count; ++bonus)
        if (Players[faction_id].faction_bonus_id[bonus] == 2)
            set_fac(max(1, min(69, Players[faction_id].faction_bonus_val1[bonus])), base_id, TRUE);
    for (bonus = 0; bonus < Players[faction_id].faction_bonus_count; ++bonus) {
        const int facility = Players[faction_id].faction_bonus_val1[bonus];
        // The prerequisite lookup precedes clamping in the original too.
        if (Players[faction_id].faction_bonus_id[bonus] == 12
            && has_tech(Facility[facility].preq_tech, faction_id))
            set_fac(max(1, min(69, facility)), base_id, TRUE);
    }
    for (int facility = 1; facility < 70; ++facility)
        if (has_tech(Facility[facility].free_tech, faction_id)) set_fac(facility, base_id, TRUE);
    base.production_id_last = base.queue_production_id[0];
    base.minerals_accumulated_2 = base.minerals_accumulated;
    base.mineral_surplus_final = 0;
    base_mark(base_id);
    reset_territory();
    BaseIDCurrentSelected = BaseComputedID = base_id;
    BaseCurrent = &base;
    base_support();
    base_yield();
    base_nutrient();
    base_minerals();
    base_energy();
    UnkBitfield1 |= 2;
    return base_id;
}

static void base_remap_projects(int removed) {
    int *projects[] = {
        &GameDataStateStorage.fields_.secret_project_.human_genome_project(),
        &GameDataStateStorage.fields_.secret_project_.command_nexus(),
        &GameDataStateStorage.fields_.secret_project_.weather_paradigm(),
        &GameDataStateStorage.fields_.secret_project_.merchant_exchange(),
        &GameDataStateStorage.fields_.secret_project_.empath_guild(),
        &GameDataStateStorage.fields_.secret_project_.citizens_defense_force(),
        &GameDataStateStorage.fields_.secret_project_.virtual_world(),
        &GameDataStateStorage.fields_.secret_project_.planetary_transit_system(),
        &GameDataStateStorage.fields_.secret_project_.xenoempathy_dome(),
        &GameDataStateStorage.fields_.secret_project_.neural_amplifier(),
        &GameDataStateStorage.fields_.secret_project_.maritime_control_center(),
        &GameDataStateStorage.fields_.secret_project_.planetary_datalinks(),
        &GameDataStateStorage.fields_.secret_project_.supercollider(),
        &GameDataStateStorage.fields_.secret_project_.ascetic_virtues(),
        &GameDataStateStorage.fields_.secret_project_.longevity_vaccine(),
        &GameDataStateStorage.fields_.secret_project_.hunter_seeker_algorithm(),
        &GameDataStateStorage.fields_.secret_project_.pholus_mutagen(),
        &GameDataStateStorage.fields_.secret_project_.cyborg_factory(),
        &GameDataStateStorage.fields_.secret_project_.theory_of_everything(),
        &GameDataStateStorage.fields_.secret_project_.dream_twister(),
        &GameDataStateStorage.fields_.secret_project_.universal_translator(),
        &GameDataStateStorage.fields_.secret_project_.network_backbone(),
        &GameDataStateStorage.fields_.secret_project_.nano_factory(),
        &GameDataStateStorage.fields_.secret_project_.living_refinery(),
        &GameDataStateStorage.fields_.secret_project_.cloning_vats(),
        &GameDataStateStorage.fields_.secret_project_.self_aware_colony(),
        &GameDataStateStorage.fields_.secret_project_.clinical_immortality(),
        &GameDataStateStorage.fields_.secret_project_.space_elevator(),
        &GameDataStateStorage.fields_.secret_project_.singularity_inductor(),
        &GameDataStateStorage.fields_.secret_project_.bulk_matter_transmitter(),
        &GameDataStateStorage.fields_.secret_project_.telepathic_matrix(),
        &GameDataStateStorage.fields_.secret_project_.voice_of_planet(),
        &GameDataStateStorage.fields_.secret_project_.ascent_to_transcendence(),
        &GameDataStateStorage.fields_.secret_project_.manifold_harmonics(),
        &GameDataStateStorage.fields_.secret_project_.nethack_terminus(),
        &GameDataStateStorage.fields_.secret_project_.cloudbase_academy(),
        &GameDataStateStorage.fields_.secret_project_.planetary_energy_grid(),
        &GameDataStateStorage.fields_.secret_project_.user_defined_38(),
        &GameDataStateStorage.fields_.secret_project_.user_defined_39(),
        &GameDataStateStorage.fields_.secret_project_.user_defined_40(),
        &GameDataStateStorage.fields_.secret_project_.user_defined_41(),
        &GameDataStateStorage.fields_.secret_project_.user_defined_42(),
        &GameDataStateStorage.fields_.secret_project_.user_defined_43(),
        &GameDataStateStorage.fields_.secret_project_.user_defined_44(),
        &GameDataStateStorage.fields_.secret_project_.user_defined_45(),
        &GameDataStateStorage.fields_.secret_project_.user_defined_46(),
        &GameDataStateStorage.fields_.secret_project_.user_defined_47(),
        &GameDataStateStorage.fields_.secret_project_.user_defined_48(),
        &GameDataStateStorage.fields_.secret_project_.user_defined_49(),
        &GameDataStateStorage.fields_.secret_project_.user_defined_50(),
        &GameDataStateStorage.fields_.secret_project_.user_defined_51(),
        &GameDataStateStorage.fields_.secret_project_.user_defined_52(),
        &GameDataStateStorage.fields_.secret_project_.user_defined_53(),
        &GameDataStateStorage.fields_.secret_project_.user_defined_54(),
        &GameDataStateStorage.fields_.secret_project_.user_defined_55(),
        &GameDataStateStorage.fields_.secret_project_.user_defined_56(),
        &GameDataStateStorage.fields_.secret_project_.user_defined_57(),
        &GameDataStateStorage.fields_.secret_project_.user_defined_58(),
        &GameDataStateStorage.fields_.secret_project_.user_defined_59(),
        &GameDataStateStorage.fields_.secret_project_.user_defined_60(),
        &GameDataStateStorage.fields_.secret_project_.user_defined_61(),
        &GameDataStateStorage.fields_.secret_project_.user_defined_62(),
        &GameDataStateStorage.fields_.secret_project_.user_defined_63(),
        &GameDataStateStorage.fields_.secret_project_.user_defined_64(),
    };
    for (int project = 0; project < 64; ++project) {
        if (*projects[project] == removed) *projects[project] = -2;
        else if (*projects[project] > removed) --*projects[project];
    }
}

static void base_remap_reference(int16_t &reference, int removed) {
    if (reference == removed) reference = -1;
    else if (reference > removed) --reference;
}

// ORIGINAL: 0x004E5250 ?base_kill@@YAXH@Z 0x004E5250-0x004E5A58
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004e5250.cpp: canonical identity, original spans and retained seed history", "source:src/base.h: real Base storage and lifecycle contracts", "source:recovery/tasks/base-lifecycle-owner-closure.json: bounded coordinator authorization"], "reason": "The original base destruction operation belongs to the Base owner and repairs real faction/project/base/vehicle references and count/selection state rather than forwarding into the original executable."}
// TRIED: full line-by-line transcription landed (MISMATCH #3, prologue push/mov divergence); stack-slot reuse across phases not replicated (used per-phase named locals instead), and the two adjustor-thunk virtual calls (vtable[1] read as delta, then slot0 on the adjusted `this`) route through VCall with a manually computed adjusted receiver rather than the shim alone.
void __cdecl base_kill(int base_id) {
    Base &base = Bases[base_id];
    const int x = base.x, y = base.y;
    const int faction_id = base.faction_id_current;
    PlayerData &faction = PlayersData[faction_id];
    if (has_fac(1, base_id, 0) && GameDataStateStorage.fields_.turn_current_num_ < faction.corner_market_turn
        && faction.corner_market_active) {
        faction.corner_market_turn = faction.corner_market_active = 0;
        if (faction_id == ConsoleGlobal.faction_id_)
            popp(ScriptPopupFile, "CORNERFOILED", 0, "econwin_sm.pcx", 0);
        else {
            parse_says(0, Players[faction_id].adj_name_faction, -1, -1);
            popp(ScriptPopupFile, "CORNERTHEMFOILED", 0, "econwin_sm.pcx", 0);
        }
    }
    if (base.population_size < 0) base.population_size = 0;
    replay_base(2, x, y, faction_id);
    --faction.current_num_bases;
    --faction.region_total_bases[map_loc(x, y)->region];
    int vehicle;
    for (vehicle = GameDataStateStorage.fields_.vehicle_current_count_ - 1; vehicle >= 0; --vehicle) {
        if (vehicle >= GameDataStateStorage.fields_.vehicle_current_count_ || Vehs[vehicle].home_base_id != base_id) continue;
        if (Vehs[vehicle].faction_id != faction_id) {
            Vehs[vehicle].home_base_id = -1;
            continue;
        }
        base.faction_id_current = 255;
        int nearest = -1, distance = 9999;
        for (int candidate = 0; candidate < GameDataStateStorage.fields_.base_current_count_; ++candidate) {
            if (Bases[candidate].faction_id_current != faction_id) continue;
            const int dx = abs(x_dist(x, Bases[candidate].x));
            const int dy = abs(y - Bases[candidate].y);
            const int range = max(dx, dy) - ((((dx + dy) >> 1) - min(dx, dy) + 1) >> 1);
            if (range <= distance) { nearest = candidate; distance = range; }
        }
        if (nearest >= 0) BaseFindDist = distance;
        base.faction_id_current = static_cast<uint8_t>(faction_id);
        if (nearest < 0) kill(vehicle);
        else Vehs[vehicle].home_base_id = static_cast<int16_t>(nearest);
    }
    bit_set(x, y, 1, 0);
    // This overlapping copy is memcpy in the original, not memmove.
    if (base_id < GameDataStateStorage.fields_.base_current_count_ - 1)
        memcpy(&Bases[base_id], &Bases[base_id + 1],
            (GameDataStateStorage.fields_.base_current_count_ - base_id - 1) * sizeof(Base));
    --GameDataStateStorage.fields_.base_current_count_;
    if (BaseComputedID == base_id) BaseComputedID = -1;
    for (vehicle = 0; vehicle < GameDataStateStorage.fields_.vehicle_current_count_; ++vehicle)
        if (Vehs[vehicle].home_base_id > base_id) --Vehs[vehicle].home_base_id;
    int remaining;
    for (remaining = 0; remaining < GameDataStateStorage.fields_.base_current_count_; ++remaining) {
        base_remap_reference(Bases[remaining].auto_forward_land_base_id, base_id);
        base_remap_reference(Bases[remaining].auto_forward_sea_base_id, base_id);
        base_remap_reference(Bases[remaining].auto_forward_air_base_id, base_id);
    }
    StringList &messages = InterfaceMessageWin.listBox_.item_list_;
    const int message_id = base_id | 0x40000000;
    if (messages.head_) {
        for (int index = 0; index < messages.entry_count_; ++index) {
            StringStructEntry *entry = messages.current_;
            if (entry->id == message_id) {
                entry->next->previous = entry->previous;
                entry->previous->next = entry->next;
                if (entry == messages.head_) messages.head_ = entry->next;
                messages.current_ = entry->next;
                messages.on_remove(entry->string_payload());
                delete entry->string_payload();
                entry->payload = 0;
                delete entry;
                --messages.entry_count_;
                --index;
            } else {
                if (entry->id > message_id) --entry->id;
                messages.current_ = entry->next;
            }
        }
        if (!messages.entry_count_) messages.head_ = 0;
        messages.current_position_ = messages.entry_count_ - 1;
    }
    InterfaceMessageWin.listBox_.update_changes();
    base_remap_projects(base_id);
    for (int owner = 1; owner < 8; ++owner) {
        int &target = PlayersData[owner].base_id_atk_target;
        if (target == base_id) target = -1;
        else if (target > base_id) --target;
    }
    for (int radius = 0; radius < 49; ++radius) {
        int wx = x + RadiusOffsetX[radius];
        if (!(MapIsFlat & 1)) {
            if (wx < 0) wx += MapLongitudeBounds;
            else if (wx >= MapLongitudeBounds) wx -= MapLongitudeBounds;
        }
        const int wy = y + RadiusOffsetY[radius];
        if (wy < 0 || wy >= MapLatitudeBounds || wx < 0 || wx >= MapLongitudeBounds) continue;
        site_set(wx, wy, 0);
        if (radius < 21) {
            bit_set(wx, wy, 0x1000, 0);
            if ((map_loc(wx, wy)->val3 & 7) == faction_id) using_set(wx, wy, 0);
        }
    }
    const bool ocean = (map_loc(x, y)->climate & 0xE0) < 0x60;
    if (ocean) {
        for (vehicle = 0; vehicle < GameDataStateStorage.fields_.vehicle_current_count_; ++vehicle)
            if (VehPrototypes[Vehs[vehicle].proto_id].plan == 7
                && ((x_dist(Vehs[vehicle].x, x) + abs(Vehs[vehicle].y - y)) & ~1) == 0)
                stack_veh(vehicle, 0);
    }
    // Original terrain cleanup begins at the one-past active vehicle slot.
    // Do not silently repair that stale-slot/error-path read with a new guard.
    for (vehicle = GameDataStateStorage.fields_.vehicle_current_count_; vehicle >= 0; --vehicle) {
        if (get_triad(vehicle) == (ocean ? 0 : 1)
            && ((x_dist(Vehs[vehicle].x, x) + abs(Vehs[vehicle].y - y)) & ~1) == 0
            && (!ocean || Vehs[vehicle].order != 1 || Vehs[vehicle].waypoint_x[0] < 0))
            kill(vehicle);
    }
    TutWinBaseWindow.base_killed(base_id);
    for (remaining = 0; remaining < GameDataStateStorage.fields_.base_current_count_; ++remaining) base_mark(remaining);
    reset_territory();
    UnkBitfield1 |= 2;
}

// Complete base_build was moved here from its retired seed:
// src/unrecovered/004f81a0.cpp. Its original findings remain below.
// The independent base_change seed remains retained at
// src/recovered/units/004e5a60.cpp.
// ORIGINAL: 0x004F81A0 ?base_build@@YAHHPAH00@Z 0x004F81A0-0x004FFFB8
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004f81a0.cpp", "source:src/base.cpp:base_reset", "source:src/unrecovered/004f81a0.cpp: complete original identity/span and historical seed findings preserved", "Pinned image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; original32280-byte span and68-selector/36-target facility dispatcher", "build/recovery/base-build-owner-resume-oct6/review-receipt-oct6.md: bounded independent review, not exhaustive or native acceptance", "recovery/tests/test_base_build_owner.py: actual-source bounded host/damage controls", "BaseSecretProject true64-word storage; historical scalar-member bindings remain unresolved primitive-array accessor interpretations"], "reason": "Recover complete build selection through real Base, faction, map, prototype, facility and project owners, preserving original unchecked indexes, arithmetic, lifecycle and output contracts. Prior source decision retained as history: Explicitly unfinished Base-owned dependency required to link the recovered base_reset caller; full 72-arm build-selection behavior remains in its seed"}
// TRIED: mismatch #3 (call vs test) - this is the largest function in the batch (32280 bytes, a 72-entry switch reached via the jump table at 0x4FA56E documented in the brief, one arm per buildable unit/facility/project id) preceded by ~0x360 bytes of reactor/rules-bit eligibility computation and several 0x200/0x36-int array-zeroing blocks. Only the entry parameter normalization (a1<0 fold) plus set_base()/base_compute(1) are transcribed; the switch and its scoring tables are not modelled - the body returns the same -1/"no candidate" sentinel the original's accumulator starts in, rather than fabricate any of the 72 per-item eligibility rules.
int __cdecl base_build(int base_id,int *out_secondary,int *out_tertiary,int *out_score)
{
  int *v1;
  PlayerData *scan_faction;
  Goal *scan_site;
  Veh *scan_vehicle;
  Base *scan_base;
  VehPrototype *scan_prototype;
  int scan_tech;
  int scan_diplo_target;
  uint32_t v10;
  int v100;
  int v101;
  int v102;
  uint32_t v103;
  uint32_t v104;
  int v105;
  int v106;
  int v107;
  int v108;
  int v109;
  int v11;
  uint32_t v110;
  int v111;
  uint32_t v112;
  int v113;
  int v114;
  int v115;
  int v116;
  int v117;
  int v118;
  int v119;
  int v12;
  int v120;
  int v121;
  uint32_t v122;
  int v123;
  int v124;
  int v125;
  int v126;
  int v127;
  uint32_t v128;
  uint32_t v129;
  int v13;
  int v130;
  uint32_t v131;
  int v132;
  uint32_t v133;
  uint32_t v134;
  int v135;
  uint32_t v136;
  int v137;
  uint32_t v138;
  uint32_t v139;
  int *v14;
  int v140;
  uint32_t v141;
  int v142;
  uint32_t v143;
  int v144;
  int v145;
  int v146;
  uint32_t v147;
  uint32_t v148;
  uint32_t v149;
  uint8_t v15;
  uint32_t v150;
  int v151;
  uint32_t v152;
  int v153;
  uint32_t v154;
  uint32_t v155;
  uint32_t v156;
  uint32_t v157;
  uint32_t v158;
  uint32_t v159;
  int v16;
  uint32_t v160;
  uint32_t v161;
  uint32_t v162;
  uint32_t v163;
  uint32_t v164;
  uint32_t v165;
  int v166;
  int v167;
  uint32_t region;
  uint32_t regional_plan;
  uint32_t v17;
  uint32_t faction_id;
  int v171;
  uint32_t v18;
  int prototype_count_0 [512];
  uint32_t *v2;
  int prototype_count_1 [512];
  int prototype_count_2 [512];
  int plan_count_0 [54];
  int plan_count_1 [54];
  int plan_count_2 [54];
  uint32_t plan_count_3 [54];
  int plan_count_4 [54];
  int v27;
  int v28;
  uint32_t v29 [8];
  char v3;
  uint32_t v30 [64];
  int v31;
  int v32;
  int v33;
  int v34;
  int v35;
  int v36;
  int v37;
  int v38;
  int v39;
  char v4;
  int v40;
  int v41;
  int v42;
  int v43;
  int v44;
  int v45;
  int v46;
  int v47;
  int v48;
  int v49;
  int16_t v5;
  int v50;
  int v51;
  int v52;
  int v53;
  int v54;
  int v55 [3];
  int v56;
  int v57;
  int v58;
  int v59;
  int16_t v6;
  int v60;
  int v61;
  int v62;
  int v63;
  int selected_id [3]; // production ID array
  int initial_facility_10_present;
  int v65;
  int v66;
  int v67;
  int v68;
  uint16_t v7;
  int v73 [3];
  int selected_score [3]; // score array
  int selection_mode;
  int local_state_flags;
  int best_reactor_id;
  int v75;
  uint32_t v76;
  int v77;
  int v78; // esi
  int v79;
  int v8;
  uint32_t *v80;
  Base *v82;
  bool v83;
  uint32_t v9;
  int v93;
  int v94;
  int v95;
  int v96;
  int v97;
  int v98;
  int v99;

  if (0 <= base_id)
    selection_mode = 0;
  else {
    selection_mode = 1;
    if (0 <= base_id + 1)
      base_id = base_id + 1;
    else {
      base_id = 0xffffffff - base_id;
    }
  }
  set_base(base_id);
  base_compute(1);
  v137 = (int)(int16_t)(Bases[base_id].y);
  v151 = (int)Bases[base_id].x;
  v15 = (uint8_t)(Bases[base_id].faction_id_current);
  v18 = (uint32_t)v15;
  v8 = MapLongitude * (int)v137 + ((int)v151 >> 1);
  v158 = (uint32_t)((MapTiles[v8].climate & 0xe0) < 0x60);
  v76 = (uint32_t)MapTiles[v8].region;
  v75 = v18 * 0x20cc;
  regional_plan = (uint32_t)(uint8_t)(PlayersData[v18].region_base_plan[v76]);
  region = v76;
  faction_id = v18;
  v171 = v75;
  best_reactor_id = best_reactor(v18);
  v8 = BaseIDCurrentSelected;
  v9 = 1 << (v15 & 0x1f);
  if (((uint8_t)v9 & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_) && (v10 = BaseCurrent->governor, v10 & 0x80000000)) {
    if ((!(v10 & 0xf200000)) && (8 <= BaseIDCurrentSelected)) {
      v15 = (uint8_t)(PlayersData[0].region_base_plan[v75 + v76]);
      if (v15 <= 2)
        v10 |= 0x8000000;
      if (v15 == 8)
        v10 |= 0x1000000;
      if (v15 == 7)
        v10 |= 0x2000000;
      v10 |= 0x1000000 << ((uint8_t)BaseIDCurrentSelected & 3);
    }
    v131 = v10 >> 0x18 & 1;
    v150 = v10 >> 0x19 & 1;
    v134 = v10 >> 0x1a & 1;
    v148 = v10 >> 0x1b & 1;
    if (v148)
      v141 = 1;
    else if (v131)
      v141 = 0;
    else if ((v150) || (v134))
      v141 = 0xffffffff;
    else {
      v141 = (uint32_t)(PlayersData[v18].ai_fight);
    }
    v147 = v131;
    v156 = v134;
    v157 = v150;
    v160 = v148;
    if ((((v131) && (v150)) && (v134)) && (v148)) {
      v160 = 0;
      v156 = 0;
      v157 = 0;
      v147 = 0;
    }
  }
  else {
    v131 = (uint32_t)(PlayersData[v18].ai_growth);
    v150 = (uint32_t)(PlayersData[v18].ai_tech);
    v134 = (uint32_t)(PlayersData[v18].ai_wealth);
    v148 = (uint32_t)(PlayersData[v18].ai_power);
    v141 = (uint32_t)(PlayersData[v18].ai_fight);
    v160 = 0;
    v156 = 0;
    v157 = 0;
    v147 = 0;
  }
  v16 = 0;
  v11 = 0;
  do {
    if ((char)(PlayersData[v18].region_total_bases[v11]))
      v16 += 1;
    v11 += 1;
  } while (v11 < 0x80);
  v75 = 0x200;
  v14 = prototype_count_0;
  while (v75) {
    v1 = &v14[1];
    *v14 = 0;
    v75 -= 1;
    v14 = v1;
  }
  v75 = 0x200;
  v14 = prototype_count_1;
  while (v75) {
    v1 = &v14[1];
    *v14 = 0;
    v75 -= 1;
    v14 = v1;
  }
  v75 = 0x200;
  v14 = prototype_count_2;
  while (v75) {
    v1 = &v14[1];
    *v14 = 0;
    v75 -= 1;
    v14 = v1;
  }
  v75 = 0x36;
  v14 = plan_count_0;
  while (v75) {
    v1 = &v14[1];
    *v14 = 0;
    v75 -= 1;
    v14 = v1;
  }
  v75 = 0x36;
  v14 = plan_count_1;
  while (v75) {
    v1 = &v14[1];
    *v14 = 0;
    v75 -= 1;
    v14 = v1;
  }
  v75 = 0x36;
  v14 = plan_count_2;
  while (v75) {
    v1 = &v14[1];
    *v14 = 0;
    v75 -= 1;
    v14 = v1;
  }
  v75 = 0x36;
  v80 = plan_count_3;
  while (v75) {
    v2 = &v80[1];
    *v80 = 0;
    v75 -= 1;
    v80 = v2;
  }
  v55[2] = 0;
  v55[0] = 0;
  v55[1] = 0;
  v75 = 0x36;
  v14 = plan_count_4;
  while (v75) {
    v1 = &v14[1];
    *v14 = 0;
    v75 -= 1;
    v14 = v1;
  }
  v99 = 0;
  v105 = 0;
  v107 = 0;
  v115 = 0;
  v125 = 0;
  v108 = 0;
  v129 = 0;
  v113 = 0;
  local_state_flags = 0;
  v109 = v16;
  v165 = v9;
  bitmask(10,&v28,&v27);
  initial_facility_10_present = (uint32_t)(((uint8_t)(Bases[v8].facilities_built[v28]) & (uint8_t)v27) != 0);
  v132 = 0;
  v106 = 0;
  v155 = 0;
  v117 = 0;
  v119 = 0;
  v96 = 0;
  v101 = 0;
  v127 = 0;
  v146 = 0;
  v97 = 0;
  v130 = 0;
  v121 = 0;
  v102 = 0;
  if ((!regional_plan) || (v144 = 0, regional_plan == 2))
    v144 = 1;
  v76 = 1;
  v9 = 0xffffffff;
  do {
    if (((v18 != v76) && ((uint8_t)(PlayersData[v18].diplo_treaties[v76]) & 1)) && (((uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (uint8_t)(1 << ((uint8_t)v76 & 0x1f)) || ((int)v9 <= -1))))
      v9 = v76;
    v76 += 1;
  } while ((int)v76 < 8);
  scan_faction = &PlayersData[1];
  v103 = 0xffffffff;
  v110 = 0xffffffff;
  v111 = 0;
  v76 = 1;
  do {
    if (v18 != v76) {
      v10 = (uint32_t)(PlayersData[v18].diplo_treaties[v76]);
      v133 = v10 & 0x10;
      v15 = (uint8_t)v76;
      if ((v133) || ((((uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (uint8_t)(1 << (v15 & 0x1f)) && (v10 & 8)) && (!(v10 & 1))))) {
        v8 = 0;
        if (1 <= GameDataStateStorage.fields_.base_current_count_) {
          scan_base = &Bases[0];
          do {
            if ((scan_base->faction_id_current == v18) && ((scan_base->faction_id_current == v76 || (scan_base->visibility & (uint8_t)(1 << (v15 & 0x1f)))))) {
              v111 = 1;
              break;
            }
            v8 += 1;
            ++scan_base;
          } while (v8 < GameDataStateStorage.fields_.base_current_count_);

        }
      }
      if (v133) {
        if (regional_plan == 7)
          v121 = 1;
        if (((0 <= (int)v9) && ((uint8_t)(PlayersData[v9].diplo_treaties[v76]) & 0x10)) || ((((uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (uint8_t)(1 << (v15 & 0x1f)) && (GameDataStateStorage.fields_.game_rules_ & 0x40)) || (((v10 & 0x20 && (3 <= scan_faction->diff_level)) || (v10 & 0x40)))))) {
label_4f86ee:
          if (regional_plan == 7)
            v144 = 1;
          if (scan_faction->region_total_bases[region])
            v103 = v76;
          else if (regional_plan == 7)
            v110 = v76;
        }
        else if ((int)(PlayersData[faction_id].ranking) < scan_faction->ranking) {
          if (0 <= (int)v141) goto label_4f86ee;
        }
      }
    }
    ++scan_faction;
    v76 += 1;


  } while (scan_faction < &PlayersData[8]);
  if (((regional_plan == 7) && (v121)) && (GameDataStateStorage.fields_.game_state_ & 0x200))
    v144 = 2;
  v104 = 0;
  v12 = (PlayersData[faction_id].x_target);
  if (0 <= (int)v12) {
    v104 = (uint32_t)(MapTiles[(int)(PlayersData[faction_id].y_target) * MapLongitude + ((int)v12 >> 1)].region == region);
    if ((v151 == v12) && (v137 == (PlayersData[faction_id].y_target)))
      v104 = 2;
  }
  v98 = 0;
  v135 = 0x19;
  scan_site = &PlayersData[faction_id].sites[0];
  do {
    if ((scan_site->type == 3) && (1 <= scan_site->priority)) {
      if ((MapTiles[scan_site->y * MapLongitude + (scan_site->x >> 1)].region == region) && ((v9 = whose_territory(v18,scan_site->x,scan_site->y,NULL,0), (int)v9 < 0 || (v9 == v18))))
        v98 += 1;
    }
    ++scan_site;
    v135 -= 1;
  } while (v135);
  v8 = BaseCurrent->drone_total;
  v143 = 0;
  if (v8) {
    v83 = BaseCurrent->talent_total <= v8;
    v143 = (uint32_t)v83;
    v9 = (uint32_t)v83;
    if (((!((uint8_t)v165 & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)) || (BaseCurrent->minerals_accumulated <= (int)Rules.retool_exemption)) && ((BaseCurrent->state & 2 || ((v9 && (2 <= BaseCurrent->specialist_total - BaseCurrent->specialist_unk_1))))))
      v143 = v9 + 1;
  }
  v75 = BaseCurrent->mineral_surplus;
  v8 = BaseCurrent->mineral_intake_2 + v75 * 2;
  if (out_score) {
    v128 = !((-0x45 <= BaseCurrent->production_id_last) || (BaseCurrent->minerals_accumulated <= (int)Rules.retool_exemption));
  }
  else {
    v128 = (uint32_t)(BaseCurrent->queue_production_id[0] <= -0x46);
  }
  v16 = 0;
  v11 = 8;
  v80 = v29;
  while (v11) {
    v2 = &v80[1];
    *v80 = 0;
    v11 -= 1;
    v80 = v2;
  }
  v11 = 0x40;
  v80 = v30;
  while (v11) {
    v2 = &v80[1];
    *v80 = 0;
    v11 -= 1;
    v80 = v2;
  }
  v100 = 0;
  do {
    v11 = GameDataStateStorage.fields_.secret_project_[v16];
    if (v11 == BaseIDCurrentSelected)
      v100 += 1;
    if ((0 <= v11) && ((uint8_t)(Bases[v11].faction_id_current) == v18)) {
      v11 = (int)(v16 + (v16 >> 0x1f & 7U)) >> 3;
      v29[v11] = v29[v11] + 1;
    }
    v16 += 1;
  } while (v16 < 0x40);
  if ((((v128) && (1 <= BaseCurrent->nutrient_surplus)) && (1 <= v75)) && (BaseCurrent->specialist_total - BaseCurrent->specialist_unk_1 <= ((int)((int)BaseCurrent->population_size + ((int)BaseCurrent->population_size >> 0x1f & 3U)) >> 2) + 1))
    v143 = 1;
  v75 = 0;
  v95 = 0x18;
  v93 = 0x18;
  v145 = 0;
  v120 = 0;
  v94 = 0;
  v123 = 0;
  v118 = 0;
  base_id = 0;
  v135 = 0;
  v116 = v8;
  if (1 <= GameDataStateStorage.fields_.base_current_count_) {
    do {
      v8 = v75 * 0x134;
      v15 = (uint8_t)(Bases[v75].faction_id_current);
      v9 = (uint32_t)v15;
      if (v18 != v9) {
        if (((int)(PlayersData[faction_id].base_id_atk_target) == v75) && ((uint8_t)(PlayersData[v18].diplo_treaties[v9]) & 0x10)) {
          if (MapTiles[(int)(int16_t)(Bases[v75].y) * MapLongitude + ((int)Bases[v75].x >> 1)].region != region) {
            if (regional_plan == 7)
              local_state_flags |= 1;
          }
          else {
            local_state_flags |= 2;
          }
        }
        v8 = (int)(Bases[v75].queue_production_id[0]);
        if ((v8 == -0x66) && (v146 = (int)((int)v146 + 1), (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (uint8_t)(1 << (v15 & 0x1f))))
          v97 += 1;
        if (v8 == BaseCurrent->queue_production_id[0])
          v155 += 1;
        if (v8 <= -0x46) {
          v80 = &v30[-v8 - 70];
          *v80 = *v80 | 1 << (v15 & 0x1f);
        }
        if (((((uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (uint8_t)(1 << (v15 & 0x1f))) && ((uint8_t)(Bases[v75].faction_id_former) == faction_id)) && (MapTiles[(int)(int16_t)(Bases[v75].y) * MapLongitude + ((int)Bases[v75].x >> 1)].region == region)) && ((!((uint8_t)(PlayersData[faction_id].diplo_treaties[v9]) & 1) && (v132 = v132 + 1, (int)(1 - v141) < v132))))
          v144 = 1;
      }
      else {
        v11 = (int)(char)(Bases[v75].population_size);
        if (v118 < v11)
          v118 = v11;
        v11 = (int)(int16_t)(Bases[v75].y);
        if ((MapTiles[v11 * MapLongitude + ((int)Bases[v75].x >> 1)].climate & 0xe0) <= 0x5f)
          v130 += 1;
        v16 = abs((int)v151 - (int)Bases[v75].x);
        if ((!(MapIsFlat & 1U)) && ((int)MapLongitude < v16))
          v16 = MapLongitudeBounds - v16;
        v11 = abs((int)v137 - v11);
        v16 = abs(v16);
        v77 = abs(v11);
        v11 = v16;
        if (v16 <= v77)
          v11 = v77;
        v13 = v16;
        if (v77 <= v16)
          v13 = v77;
        v11 -= ((v16 + v77 >> 1) - v13) + 1 >> 1;
        bitmask(0x1c,&v31,&v32);
        v152 = (uint32_t)(((uint8_t)(Bases[(v8 / 0x134)].facilities_built[v31]) & (uint8_t)v32) != 0);
        if ((v152) && (v11 < v95))
          v95 = v11;
        bitmask(0x1d,&v34,&v33);
        v152 = (uint32_t)(((uint8_t)(Bases[(v8 / 0x134)].facilities_built[v34]) & (uint8_t)v33) != 0);
        if (((v152) || ((0 <= GameDataStateStorage.fields_.secret_project_.cloudbase_academy() && ((char)(Bases[GameDataStateStorage.fields_.secret_project_.cloudbase_academy()].faction_id_current) == (char)(Bases[v75].faction_id_current))))) && (v11 < v93))
          v93 = v11;
        bitmask(0x21,&v36,&v35);
        v152 = (uint32_t)(((uint8_t)(Bases[(v8 / 0x134)].facilities_built[v36]) & (uint8_t)v35) != 0);
        if (v152)
          v127 += 1;
        bitmask(0x17,&v38,&v37);
        v152 = (uint32_t)(((uint8_t)(Bases[(v8 / 0x134)].facilities_built[v38]) & (uint8_t)v37) != 0);
        if (v152)
          v102 += 1;
        bitmask(1,&v40,&v39);
        v152 = (uint32_t)(((uint8_t)(Bases[(v8 / 0x134)].facilities_built[v40]) & (uint8_t)v39) != 0);
        if ((v152) || ((int)(Bases[v75].queue_production_id[0]) == -1))
          v99 += 1;
        if (base_id != BaseIDCurrentSelected) {
          if ((uint32_t)(Bases[v75].state) & 0x100000)
            v120 = (int)((int)v120 + 1);
          v11 = (int)(char)(Bases[v75].population_size);
          if (v101 < v11)
            v101 = v11;
          v11 = BaseCurrent->queue_production_id[0];
          if (((int)(Bases[v75].queue_production_id[0]) == v11) && (v106 = v106 + 1, v11 <= -0x46)) {
            if (1 <= BaseCurrent->mineral_surplus_final) {
              if ((1 <= (int)(Bases[v75].mineral_surplus_final)) && (v16 = cost_factor((uint32_t)BaseCurrent->faction_id_current,1,-1), v16 = v16 * (int)(Facility[(v11 * -1)].cost), (v16 - (int)(Bases[v75].minerals_accumulated)) / (int)(Bases[v75].mineral_surplus_final) < (v16 - BaseCurrent->minerals_accumulated) / BaseCurrent->mineral_surplus_final)) {
label_4f8eb5:
                v117 += 1;
              }
            }
            else if (1 <= (int)(Bases[v75].mineral_surplus_final)) goto label_4f8eb5;
          }
          v11 = (int)(Bases[v75].queue_production_id[0]);
          if (v11 <= -0x46) {
            v80 = &v30[-v11 - 70];
            *v80 = *v80 | v165;
          }
          v163 = v11 * 0x34;
          v18 = (uint32_t)(uint8_t)(VehPrototypes[v11].plan);
          v164 = (uint32_t)(uint8_t)(Chassis[VehPrototypes[v11].chassis_id].triad);
          if ((v18 == 7) && ((v15 = (uint8_t)(VehPrototypes[v11].carry_capacity), v15 < 2 || ((int)(uint32_t)v15 < (int)((int)(PlayersData[faction_id].unk_70) + ((int)(PlayersData[faction_id].unk_70) >> 0x1f & 0x3fffffU)) >> 0x16))))
            v18 = 0xf;
          if ((!v164) && (has_abil(v11,0x8000)))
            v18 = 0x10;
          if (0 <= v11) {
            plan_count_3[v164 + v18 * 3] = plan_count_3[v164 + v18 * 3] + (best_reactor_id <= (int)(uint32_t)(uint8_t)(VehPrototypes[(v163 / 0x34)].reactor_id)) + 1;
            prototype_count_1[v11] = prototype_count_1[v11] + 1;
          }
          if (region == MapTiles[(int)(int16_t)(Bases[v75].y) * MapLongitude + ((int)Bases[v75].x >> 1)].region) {
            if ((uint8_t)(Bases[v75].state) & 1)
              v145 = 0x1;
            if ((int)(Bases[v75].queue_production_id[0]) <= -0x46)
              v119 += 1;
            bitmask(0x1b,&v42,&v41);
            v152 = (uint32_t)(((uint8_t)(Bases[(v8 / 0x134)].facilities_built[v42]) & (uint8_t)v41) != 0);
            if ((v152) || ((int)(Bases[v75].queue_production_id[0]) == -0x1b))
              v107 = ((int)v107 + 1);
            bitmask(0x1e,&v44,&v43);
            v152 = (uint32_t)(((uint8_t)(Bases[(v8 / 0x134)].facilities_built[v44]) & (uint8_t)v43) != 0);
            if ((v152) || ((int)(Bases[v75].queue_production_id[0]) == -0x1e))
              v105 = ((int)v105 + 1);
            bitmask(0x1c,&v46,&v45);
            v152 = (uint32_t)(((uint8_t)(Bases[(v8 / 0x134)].facilities_built[v46]) & (uint8_t)v45) != 0);
            if ((v152) || ((int)(Bases[v75].queue_production_id[0]) == -0x1c))
              v108 += 1;
            bitmask(0x1d,&v48,&v47);
            v152 = (uint32_t)(((uint8_t)(Bases[(v8 / 0x134)].facilities_built[v48]) & (uint8_t)v47) != 0);
            if ((v152) || ((int)(Bases[v75].queue_production_id[0]) == -0x1d))
              v129 += 1;
            bitmask(10,&v50,&v49);
            v152 = (uint32_t)(((uint8_t)(Bases[(v8 / 0x134)].facilities_built[v50]) & (uint8_t)v49) != 0);
            if ((v152) || ((int)(Bases[v75].queue_production_id[0]) == -10))
              v113 += 1;
            bitmask(0x1f,&v52,&v51);
            v152 = (uint32_t)(((uint8_t)(Bases[(v8 / 0x134)].facilities_built[v52]) & (uint8_t)v51) != 0);
            if (v152)
              v125 = (v125 + 1);
            bitmask(0x20,&v54,&v53);
            v11 = BaseIDCurrentSelected;
            v152 = (uint32_t)(((uint8_t)(Bases[(v8 / 0x134)].facilities_built[v54]) & (uint8_t)v53) != 0);
            if (v152)
              v115 += 1;
            v16 = (int)(Bases[v75].queue_production_id[0]);
            if (0 <= v16) {
              prototype_count_0[v16] = prototype_count_0[v16] + 1;
              v77 = v164 + v18 * 3;
              plan_count_2[v77] = plan_count_2[v77] + (best_reactor_id <= (int)(uint32_t)(uint8_t)(VehPrototypes[(v163 / 0x34)].reactor_id)) + 1;
              v55[v164] = v55[v164] + 1;
              if ((0x40 <= v16) && (!((uint8_t)(VehPrototypes[v16].flags) & 4)))
                v96 += 1;
            }
            bitmask(0x1b,&v57,&v56);
            v152 = (uint32_t)(((uint8_t)(Bases[v11].facilities_built[v57]) & (uint8_t)v56) != 0);
            if (v152) {
              bitmask(0x1b,&v59,&v58);
              v152 = (uint32_t)(((uint8_t)(Bases[(v8 / 0x134)].facilities_built[v59]) & (uint8_t)v58) != 0);
              if (!v152) {
                v5 = (int16_t)(Bases[v75].y);
                v6 = Bases[v75].x;
                v8 = stack_fix(veh_at((int)v6,(int)v5));
                v112 = garrison_check(base_id);
                v122 = defensive_check(base_id);
                v18 = faction_id;
                v149 = stack_check(v8,0xb,0x10000000,0x10000000,faction_id);
                v18 = stack_check(v8,2,2,v18,-1);
                if ((0 <= (int)v18) && (0 <= (int)v122)) {
                  if ((int)v122 < (int)v18)
                    v18 = v122;
                  if ((v18) && (v149))
                    v18 += 1;
                }
                else {
                  v18 = 0;
                }
                v3 = (char)(PlayersData[faction_id].region_base_plan[region]);
                if ((v3 == '\x02') || (v3 == '\b')) {
                  v11 = stack_check(v8,2,1,faction_id,-1);
                  v18 += v11;
                }
                if (2 <= (int)(PlayersData[faction_id].best_armor_value)) {
                  if (!v18)
                    v18 = (uint32_t)(stack_check(v8,2,3,-1,-1) != 0);
                }
                else {
                  v18 += stack_check(v8,2,3,-1,-1);
                }
                v135 = 0;
                if (1 <= GameDataStateStorage.fields_.vehicle_current_count_) {
                  scan_vehicle = &Vehs[0];
                  v114 = GameDataStateStorage.fields_.vehicle_current_count_;
                  do {
                    if ((((scan_vehicle->faction_id == faction_id) && ((((char)(VehPrototypes[scan_vehicle->proto_id].plan) == '\x02' || (scan_vehicle->state & 0x10000000)) && (v8 = (int)scan_vehicle->y * MapLongitude + ((int)scan_vehicle->x >> 1), MapTiles[v8].region == region)))) && ((!(MapTiles[v8].bit & 1) || (8 <= (MapTiles[v8].val2 & 0xf))))) && ((v8 = (int)scan_vehicle->home_base_id, v8 == base_id || (v8 == BaseIDCurrentSelected)))) {
                      if (v8 == BaseIDCurrentSelected) {
                        v135 += 1;
                        if (v123 < v135) goto label_4f947a;
                        v123 += 1;
                      }
                      v18 += 1;
                    }
label_4f947a:
                    ++scan_vehicle;
                    v114 -= 1;
                  } while (v114);
                }
                if ((int)v18 < (int)v112) {
                  v8 = (int)(Bases[v75].queue_production_id[0]);
                  if ((v149) || (v112 == v122)) {
                    if ((0 <= v8) && ((char)(VehPrototypes[v8].plan) == '\x02')) {
label_4f94d9:
                      if ((int)v112 <= (int)(v18 + 1)) goto label_4f94e1;
                    }
                  }
                  else if ((0 <= v8) && ((uint8_t)(VehPrototypes[v8].plan) <= 1)) goto label_4f94d9;
                  v94 += 1;
                }
              }
            }
          }
        }
      }
label_4f94e1:
      v75 = base_id + 1;
      base_id = v75;
      if (GameDataStateStorage.fields_.base_current_count_ <= v75) break;
      v18 = faction_id;
    } while( true );
  }
  if (BaseCurrent->state & 1)
    v145 = 0x2;
  v146 = 0;
  v115 = 0;
  v114 = 0;
  v153 = 0;
  if (1 <= GameDataStateStorage.fields_.vehicle_current_count_) {
    scan_vehicle = &Vehs[0];
    do {
      v8 = v153;
      if (scan_vehicle->faction_id == faction_id) {
        v18 = (uint32_t)(uint8_t)(VehPrototypes[scan_vehicle->proto_id].plan);
        v146 = ((int)v146 + 1);
        v164 = (uint32_t)(uint8_t)(Chassis[VehPrototypes[scan_vehicle->proto_id].chassis_id].triad);
        if ((v18 == 7) && ((v75 = veh_cargo(v153), v75 <= 1 || (v8 = veh_cargo(v8), v8 < (int)((int)(PlayersData[faction_id].unk_70) + ((int)(PlayersData[faction_id].unk_70) >> 0x1f & 0x3fffffU)) >> 0x16))))
          v18 = 0xf;
        v9 = v164;
        if ((!v164) && (v5 = scan_vehicle->proto_id, has_abil((int)v5,0x8000)))
          v18 = 0x10;
        v8 = (int)scan_vehicle->proto_id;
        if ((char)(VehPrototypes[v8].unk_1) == '\x15')
          v18 = 0x11;
        if (scan_vehicle->state & 0x10000000)
          v18 = 2;
        v75 = v9 + v18 * 3;
        plan_count_1[v75] = plan_count_1[v75] + (best_reactor_id <= (int)(uint32_t)(uint8_t)(VehPrototypes[v8].reactor_id)) + 1;
        if ((((scan_vehicle->home_base_id == BaseIDCurrentSelected) && (v3 = (char)(VehPrototypes[v8].plan), plan_count_4[v75] = plan_count_4[v75] + 1, v3 == '\x03')) && (v5 = scan_vehicle->proto_id, has_abil((int)v5,0x40000))) && (region == MapTiles[(int)scan_vehicle->y * MapLongitude + ((int)scan_vehicle->x >> 1)].region))
          v115 += 1;
        v8 = (int)scan_vehicle->y * MapLongitude + ((int)scan_vehicle->x >> 1);
        if (region == MapTiles[v8].region) {
          v11 = (int)scan_vehicle->proto_id;
          prototype_count_2[v11] = prototype_count_2[v11] + 1;
          v9 = MapTiles[v8].bit;
          plan_count_0[v75] = plan_count_0[v75] + (best_reactor_id <= (int)(uint32_t)(uint8_t)(VehPrototypes[v11].reactor_id)) + 1;
          if (((!(v9 & 1)) || (8 <= (MapTiles[v8].val2 & 0xf))) && (v18 <= 3))
            v114 += 1;
        }
      }
      v153 += 1;
      ++scan_vehicle;
    } while (v153 < GameDataStateStorage.fields_.vehicle_current_count_);
  }
  v75 = v171;
  v8 = BaseIDCurrentSelected;
  v11 = 0;
  do {
    v77 = 3;
    v16 = v11;
    do {
      v11 = v16 + 4;
      plan_count_3[v16 / 4] = plan_count_3[v16 / 4] / 2;
      plan_count_2[v16 / 4] = plan_count_2[v16 / 4] / 2;
      plan_count_1[v16 / 4] = plan_count_1[v16 / 4] / 2;
      plan_count_0[v16 / 4] = plan_count_0[v16 / 4] / 2;
      v77 -= 1;
      v16 = v11;
    } while (v77);
  } while (v11 < 0xcc);
  v11 = 0;
  v16 = 0;
  do {
    if (3 <= (uint8_t)(PlayersData[faction_id].region_total_bases[v16]))
      v11 += 1;
    v16 += 1;
  } while (v16 < 0x40);
  if (10 <= (int)(PlayersData[faction_id].current_num_bases))
    v11 += 1;
  if (0x14 <= (int)(PlayersData[faction_id].current_num_bases))
    v11 += 1;
  v11 += (v130 + 2) / 3;
  v123 = v11;
  if ((0 <= GameDataStateStorage.fields_.secret_project_.maritime_control_center()) && ((uint8_t)(Bases[GameDataStateStorage.fields_.secret_project_.maritime_control_center()].faction_id_current) == faction_id)) {
    v11 = v11 * 2 + 1;
    v123 = v11;
  }
  bitmask(0x23,&v61,&v60);
  v16 = BaseIDCurrentSelected;
  if ((int)((int)(PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::POLICE]) + (uint32_t)(((uint8_t)(Bases[v8].facilities_built[v61]) & (uint8_t)v60) != 0) * 2) <= -3) {
    v11 = (v11 + 1) / 2;
    v123 = v11;
  }
  bitmask(0x23,&v63,&v62);
  v18 = faction_id;
  if ((int)((int)(PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::POLICE]) + (uint32_t)(((uint8_t)(Bases[v16].facilities_built[v63]) & (uint8_t)v62) != 0) * 2) <= -5)
    v123 = v11 / 2;
  v75 = 0;
  v135 = 0;
  v139 = 0;
  v8 = MapLongitudeBounds;
  do {
    v12 = v151 + RadiusOffsetX[v75];
    if (!(MapIsFlat & 1U)) {
      if (0 <= (int)v12) {
        if (v8 <= (int)v12)
          v12 = (v12 + -v8);
      }
      else {
        v12 = (v12 + v8);
      }
    }
    v11 = RadiusOffsetY[v75] + (int)v137;
    if ((((0 <= v11) && (v11 < MapLatitudeBounds)) && (0 <= (int)v12)) && ((int)v12 < v8)) {
      if (MapTiles[v11 * MapLongitude + ((int)v12 >> 1)].bit & 0x200000)
        v139 += ((1 << ((uint8_t)v75 & 0x1f) & BaseCurrent->resource_sqr_radius) != 0) + 1;
      v8 = veh_at((int)v12,v11);
      if (((0 <= v8) && (v9 = (uint32_t)(uint8_t)(Vehs[v8].faction_id), v9 != v18)) && ((uint8_t)(PlayersData[v18].diplo_treaties[v9]) & 0x30))
        v135 += stack_check(v8,1,-1,-1,-1);
      v8 = MapLongitudeBounds;
    }
    v75 += 1;
  } while (v75 < 0x15);
  v5 = BaseCurrent->y;
  v6 = BaseCurrent->x;
  v8 = stack_fix(veh_at((int)v6,(int)v5));
  v153 = v8;
  v112 = garrison_check(BaseIDCurrentSelected);
  v18 = defensive_check(BaseIDCurrentSelected);
  v122 = v18;
  v9 = stack_check(v8,0xb,0x10000000,0x10000000,faction_id);
  v149 = v9;
  v76 = stack_check(v8,2,2,faction_id,-1);
  if ((0 <= (int)v76) && (0 <= (int)v18)) {
    if ((int)v76 <= (int)v18)
      v18 = v76;
    v138 = v18;
    if ((v18) && (v9)) {
      v18 += 1;
      v138 = v18;
    }
  }
  else {
    v18 = 0;
    v138 = v18;
  }
  if ((BaseCurrent->state & 0x4000000) && (3 <= (uint8_t)(PlayersData[faction_id].region_base_plan[region]))) {
    v18 += 1;
    v138 = v18;
  }
  v3 = (char)(PlayersData[faction_id].region_base_plan[region]);
  if ((v3 == '\x02') || (v3 == '\b')) {
    v75 = stack_check(v8,2,1,faction_id,-1);
    v18 += v75;
    v138 = v18;
  }
  if (2 <= (int)(PlayersData[faction_id].best_armor_value)) {
    if (!v18) {
      v18 = (uint32_t)(stack_check(v8,2,3,-1,-1) != 0);
      v138 = v18;
    }
  }
  else {
    v18 += stack_check(v8,2,3,-1,-1);
    v138 = v18;
  }
  v9 = faction_id;
  stack_check(v8,2,7,faction_id,-1);
  stack_check(v8,2,6,v9,-1);
  if ((uint8_t)v165 & VehPrototypes[2].obsolete_factions) {
    v8 = stack_check(v8,0,2,v9,-1);
    if (0 <= v8) {
      if (2 <= v8)
        v8 = 1;
    }
    else {
      v8 = 0;
    }
    v18 -= v8;
    v138 = v18;
  }
  if ((3 <= (uint8_t)(PlayersData[faction_id].region_base_plan[region])) && (BaseCurrent->state & 0x8000000)) {
    v18 += 1;
    v138 = v18;
  }
  if (1 <= (int)v18) {
    if ((int)(v112 + -1) <= (int)v18)
      v18 = (uint32_t)((int)v18 < (int)v112);
    else {
      v18 = 2;
    }
  }
  else {
    v18 = 3;
  }
  v133 = GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xff & v165;
  v162 = v18;
  if (((v133) && ((int)Rules.retool_exemption < BaseCurrent->minerals_accumulated)) && (!(BaseCurrent->state & 8))) {
    if (((v147) || (v157)) || (v156)) {
      if ((!v160) && (!v147)) {
        v162 = v147;
        goto label_4f9e89;
      }
    }
    else if (!v160) goto label_4f9c43;
    if (2 <= v18)
      v162 = 1;
  }
  else {
label_4f9c43:
    if (((regional_plan == 8) || ((regional_plan == 7 && (((int)(PlayersData[faction_id].current_num_bases) < 4 || (!v111)))))) || ((v133 && (!v160)))) {
      v76 = 1;
      scan_diplo_target = v9;
      v159 = 1;

      scan_faction = &PlayersData[1];

      do {
        if (((faction_id != v76) && ((scan_faction->region_total_combat_vehs[region] || (scan_faction->region_total_bases[region])))) && ((v9 = (uint32_t)(PlayersData[faction_id].diplo_treaties[v76]), v9 & 0x30 || (((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xff & 1 << ((uint8_t)v76 & 0x1f) && ((uint8_t)scan_faction->diplo_treaties[scan_diplo_target] & 8)) && ((!(v9 & 2) || (scan_faction->integrity_blemishes))))))))
          v159 = 0;
        ++scan_faction;

        v76 += 1;


      } while (scan_faction < &PlayersData[8]);
      if (((v18) && (v159)) && (((v133 && (!v160)) || ((BaseCurrent->population_size <= 2 && (((v131 || ((int)(PlayersData[faction_id].current_num_bases) <= 2)) && (!(BaseCurrent->state & 0x1000000))))))))) {
        v162 = v18 - 1;
        v133 = (uint32_t)BaseCurrent->x;
        v154 = (uint32_t)BaseCurrent->y;
        v8 = 0;
        do {
          v75 = RadiusOffsetX[v8 / 4] + v133;
          if (!(MapIsFlat & 1U)) {
            if (0 <= v75) {
              if (MapLongitudeBounds <= v75)
                v75 -= MapLongitudeBounds;
            }
            else {
              v75 += MapLongitudeBounds;
            }
          }
          v11 = RadiusOffsetY[v8 / 4] + v154;
          if (((((0 <= v11) && (v11 < MapLatitudeBounds)) && ((0 <= v75 && ((v75 < MapLongitudeBounds && (v75 = v11 * MapLongitude + (v75 >> 1), MapTiles[v75].region == region)))))) && ((PlayersData[faction_id].flags & 0x200 || ((uint8_t)v165 & MapTiles[v75].visibility)))) && (MapTiles[v75].bit & 3)) {
            v9 = MapTiles[v75].val2 & 0xf;
            if (((v9 <= 7) && (v9 != faction_id)) && ((!(MapTiles[v75].val2 & 0xf) || ((uint8_t)(PlayersData[faction_id].diplo_treaties[v9]) & 0x30)))) {
              v162 = v18;
              break;
            }
          }
          v8 += 4;
        } while (v8 < 0x1e4);
      }
    }
    v9 = faction_id;
  }
label_4f9e89:
  v18 = 0;
  v124 = 0;
  v155 = 0;
  do {
    v8 = RadiusOffsetX[v18 / 4] + (int)BaseCurrent->x;
    if (!(MapIsFlat & 1U)) {
      if (0 <= v8) {
        if (MapLongitudeBounds <= v8)
          v8 -= MapLongitudeBounds;
      }
      else {
        v8 = MapLongitudeBounds + v8;
      }
    }
    v75 = RadiusOffsetY[v18 / 4] + (int)BaseCurrent->y;
    if ((((0 <= v75) && (v75 < MapLatitudeBounds)) && (0 <= v8)) && ((v8 < MapLongitudeBounds && (v11 = v75 * MapLongitude + (v8 >> 1), MapTiles[v11].region == region)))) {
      if ((!(PlayersData[faction_id].flags & 0x200)) && (!(MapTiles[v11].visibility & (uint8_t)v165))) {
label_4f9fde:
        v124 = 1;
        break;
      }
      if (MapTiles[v11].bit & 3) {
        v18 = MapTiles[v11].val2 & 0xf;
        if (((v18 <= 7) && (MapTiles[v11].val2 & 0xf)) && (v18 != v9)) goto label_4f9fde;
      }
      v18 = whose_territory(v9,v8,v75,NULL,0);
      if ((1 <= (int)v18) && (v18 != v9)) {
        if ((uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (uint8_t)(1 << ((uint8_t)v18 & 0x1f)))
          v15 = (uint8_t)(PlayersData[v9].diplo_treaties[v18]) & 1;
        else {
          v15 = (uint8_t)(PlayersData[v9].diplo_treaties[v18]) & 2;
        }
        if (!v15) goto label_4f9fde;
      }
    }
    v18 = v155 + 4;
    v155 = v18;
  } while ((int)v18 <= 0xc3);
  v8 = 0;
  selected_score[2] = 999999;
  selected_score[0] = 999999;
  selected_score[1] = 999999;
  do {
    selected_id[v8] = (-(uint32_t)(v8 != 0) & 0x186e4) - 0x45;
    v8 += 1;
  } while (v8 < 3);
  if ((int)(PlayersData[faction_id].total_mil_veh) <= 0x1ff) {
    v133 = 999999;
    v167 = 0;
    do {
      v75 = veh_avail(v167,faction_id,BaseIDCurrentSelected);
      v8 = v167;
      if (!v75) goto label_4fd8f6;
      v163 = v167 * 0x34;
      v18 = (uint32_t)(uint8_t)(Chassis[VehPrototypes[v167].chassis_id].triad);
      v161 = (uint32_t)(uint8_t)(VehPrototypes[v167].plan);
      v164 = v18;
      if ((!v18) && (v75 = has_abil(v167,0x8000), v75))
        v161 = 0x10;
      v75 = BaseIDCurrentSelected;
      v15 = (uint8_t)(VehPrototypes[v8].plan);
      v142 = 0;
      v136 = (uint32_t)(v15 < 8);
      v9 = ((uint8_t)(VehPrototypes[v8].cost) + 2) * 0x100;
      v159 = v9;
      if (v18) {
        if (v18 != 1) {
          if (v18 != 2) {
label_4fa54b:
            v8 = v166;
            goto label_4fa54e;
          }
          if (((regional_plan == 7) || (v158)) || ((uint16_t)((uint16_t)(uint8_t)(PlayersData[faction_id].region_total_bases[region]) * 2 + 2) <= (uint16_t)(PlayersData[faction_id].region_total_combat_vehs[region]))) {
            v154 = plan_count_3[v161 * 3 + 2];
            v126 = plan_count_1[v161 * 3 + 2];
            v18 = (uint32_t)(v126 + v154 != 0);
            v8 = v18 + 1;
            if (((regional_plan != 7) && (v161 != 5)) && (!v158)) {
              if (0 <= (int)(PlayersData[faction_id].x_target))
                v8 = v18 + 2;
              if (regional_plan == 2)
                v8 += 1;
              if ((uint8_t)((PlayersData[faction_id].unk_77[region] >> 8)) & 4)
                v8 += 1;
              v8 += v55[2];
            }
            v11 = v8 * v9;
            v159 = v11;
            bitmask(0x1d,&v66,&v65);
            v155 = (uint32_t)(((uint8_t)(Bases[v75].facilities_built[v66]) & (uint8_t)v65) != 0);
            if (v155) {
              v11 = (int)((v129 + 2) * v11) / (int)(v129 + 3);
              v159 = v11;
            }
            if ((!((uint8_t)v165 & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)) && (PlayersData[faction_id].flags & 0x10000000))
              v159 = v11 / 2;
            v8 -= 2;
            if (0 <= v8) {
              if (v8 > 99)
                v8 = 99;
            }
            else {
              v8 = 0;
            }
            v8 = ((v161 != 4) + 1) * v154 + v8 + v126;
            v75 = 8 - v109;
            v75 = range(v75,4,8) * v159;
            v78 = (int)(v75 + (v75 >> 0x1f & 7U)) >> 3;
            v16 = v78 * 10;
            v11 = v78;
            v77 = veh_cost(v167,BaseIDCurrentSelected,NULL);
            v13 = cost_factor(faction_id,1,-1);
            v75 = BaseCurrent->mineral_surplus;
            v9 = range((v77 * v13 * v78) / (range(v75,1,999) * 0xf),v11,v16);
            v159 = v9;
            v18 = v158;
            goto label_4fa2fe;
          }
        }
        else if ((((regional_plan == 7) || (v158)) || ((uint16_t)((uint16_t)(uint8_t)(PlayersData[faction_id].region_total_bases[region]) * 2 + 2) <= (uint16_t)(PlayersData[faction_id].region_total_combat_vehs[region]))) && (v5 = (int16_t)(Bases[BaseIDCurrentSelected].y), v6 = Bases[BaseIDCurrentSelected].x, is_coast((int)v6,(int)v5,0))) {
          v18 = base_coast(BaseIDCurrentSelected);
          if ((((v18 & 0x3f) != 0x3f) && (v18 & 0x3f)) && (((2 <= sea_coasts(v18) || (v158)) || ((v161 != 7 && (0x30 <= (int)Continents[v18].tile_count)))))) {
            v166 = plan_count_1[v161 * 3 + 1] + plan_count_3[v161 * 3 + 1] * 2;
            if ((regional_plan != 7) && (!v158)) {
              if (regional_plan == 2)
                v166 += 1;
              if ((uint8_t)((PlayersData[faction_id].unk_77[region] >> 8)) & 4)
                v166 += 1;
              v166 += v55[1];
            }
            v18 = GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xff & v165;
            if ((!v18) && (PlayersData[faction_id].flags & 0x8000000))
              v9 /= 2;
            if ((!v18) && ((uint32_t)(Players[faction_id].rule_flags) & 0x100)) {
              v9 /= 2;
              v109 += 1;
            }
            v8 = 8 - v109;
            if (4 <= v8) {
              if (9 <= v8)
                v8 = 8;
            }
            else {
              v8 = 4;
            }
            v75 = BaseCurrent->mineral_surplus;
            v159 = (int)(v8 * v9 + ((int)(v8 * v9) >> 0x1f & 7U)) >> 3;
            if (1 <= v75) {
              if (1000 <= v75)
                v75 = 999;
            }
            else {
              v75 = 1;
            }
            v76 = v159 * 10;
            v8 = veh_cost(v167,BaseIDCurrentSelected,NULL);
            v11 = cost_factor(faction_id,1,-1);
            v18 = (int)(v8 * v11 * v159) / (v75 * 0xf);
            v9 = v159;
            if (((int)v159 <= (int)v18) && ((int)v159 <= (int)v76)) {
              if ((int)v76 < (int)v18)
                v18 = v76;
              v9 = v18;
              v159 = v18;
            }
            goto label_4fa54b;
          }
        }
        goto label_4fd8f6;
      }
      if (3 <= (uint8_t)(Weapon[VehPrototypes[v8].weapon_id].mode)) {
        if ((((v161 != 8) || (regional_plan != 8)) || (BaseCurrent->state & 0x2000000)) || (GameDataStateStorage.fields_.diff_level_current_ <= 2)) {
          v8 = plan_count_0[v161 * 3];
          if ((((v161 != 8) || (regional_plan != 8)) || ((BaseCurrent->state & 0x1000000 || (GameDataStateStorage.fields_.diff_level_current_ <= 2)))) && (v8 = v8 + prototype_count_0[v167] + plan_count_2[v161 * 3], v161 == 8))
            v8 += plan_count_3[24];
        }
        else {
          v8 = 0;
        }
      }
      else {
        v8 = 99;
        if (v15 <= 1) {
          if ((int)(PlayersData[faction_id].unk_71)) {
label_4fa634:
            v8 = 1;
          }
          else if (regional_plan != 2) {
            if ((uint8_t)((PlayersData[faction_id].unk_77[region] >> 8)) & 6)
              v8 = 2;
          }
          else {
            if ((uint8_t)((PlayersData[faction_id].unk_77[region] >> 8)) & 4) goto label_4fa634;
            v8 = 2;
          }
        }
        if ((regional_plan != 2) && (!((uint8_t)((PlayersData[faction_id].unk_77[region] >> 8)) & 4)))
          v11 = plan_count_2[v161 * 3];
        else {
          v11 = 0;
        }
        v16 = prototype_count_2[v167] + plan_count_3[v161 * 3] + plan_count_0[v161 * 3];
        if (0 <= v16) {
          if (v8 < v16)
            v16 = v8;
        }
        else {
          v16 = 0;
        }
        v8 = (prototype_count_0[v167] + v16 + 1 + v11) / 2;
        bitmask(0x1b,&v67,&v68);
        v154 = (uint32_t)(((uint8_t)(Bases[v75].facilities_built[v67]) & (uint8_t)v68) != 0);
        if (v154) {
          v9 = (int)((int)((int)v107 + 2) * v9) / (int)((int)v107 + 3);
          v159 = v9;
        }
        if (!((uint8_t)v165 & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)) {
          v18 = PlayersData[faction_id].flags & 0x4000000;
label_4fa2fe:
          if (v18) {
            v9 = (int)v9 / 2;
            v159 = v9;
          }
        }
      }
label_4fa54e:
      v10 = faction_id;
      v76 = region;
      v75 = v167;
      v18 = v164;
      v166 = v8 + 1;
      v17 = (uint32_t)(uint8_t)(VehPrototypes[(v163 / 0x34)].plan);
      v152 = (v152 & 0xffffff00U) | (uint8_t)VehPrototypes[v167].plan;
      switch(v17) {
        case 0:
          if ((3 <= (int)regional_plan) && (((regional_plan != 7 && (!v158)) || (!v121))))
            v8 = 0x40;
          else {
            v8 = 0x20;
          }
          v9 *= v8;
          if ((v138) && (!v149)) {
            v75 = guard_check(faction_id,region);
            v8 = v171;
            if ((int)(uint32_t)(uint8_t)(PlayersData[faction_id].region_total_offensive_vehs[v76]) < v75)
              v9 = (int)v9 / 2;
          }
          else {
            v8 = v171;
          }
          if (local_state_flags & 2)
            v9 = (int)v9 / 2;
          else if (local_state_flags & 1)
            v9 = (int)(v9 * 3 + ((int)(v9 * 3) >> 0x1f & 3U)) >> 2;
          if ((v158) && (!v164))
            v9 *= 2;
          if ((v160) && (((v164 == 2 || (!v158)) && (v9 = (int)v9 / 2, GameDataStateStorage.fields_.turn_current_num_ <= 0x31))))
            v9 = (int)v9 / 2;
          if ((v164 == 2) && (regional_plan != 2)) {
            v75 = (int)(PlayersData[faction_id].unk_66);
            v8 = (int)(PlayersData[faction_id].unk_67);
            v11 = v75;
            if (v75 <= v8)
              v11 = v8;
            if ((int)(plan_count_3[v161 * 3 + 2] + plan_count_1[v161 * 3 + 2]) < v11)
              v9 = (int)v9 / 2;
            if (v75 <= v8)
              v75 = v8;
            if ((int)(plan_count_3[v161 * 3 + 2] + plan_count_1[v161 * 3 + 2]) < v75 * 2)
              v9 = (int)v9 / 2;
          }
          if (((!((uint8_t)v165 & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)) && ((char)(Chassis[VehPrototypes[(v163 / 0x34)].chassis_id].missile))) && ((uint8_t)(PlayersData[faction_id].flags_ext) & 0x10))
            v9 = (int)v9 / 3;
          goto label_4fc68f;
        case 1:
          if ((3 <= (int)regional_plan) && (((regional_plan != 7 && (!v158)) || (!v121))))
            v8 = 0x20;
          else {
            v8 = 0x18;
          }
          v9 *= v8;
          if (((v138) && (!v149)) && (v8 = guard_check(faction_id,region), (int)(uint32_t)(uint8_t)(PlayersData[faction_id].region_total_offensive_vehs[v76]) < v8))
            v9 = (int)v9 / 2;
          if ((v158) && (!v164))
            v9 *= 2;
          if (((v160) && (!v158)) && (v9 = (int)v9 / 2, 0x33 <= GameDataStateStorage.fields_.turn_current_num_))
            v9 = (int)v9 / 2;
          goto label_4fc68f;
        case 2:
          v9 *= 0x10;
          if (((v94) && (!v162)) && (has_fac_built(0x1b)))
            v9 = (int)v9 / 2;
          v18 = region;
          if ((2 <= (int)v138) && (!v149)) {
            v75 = guard_check(faction_id,region);
            v8 = v171;
            if ((int)(uint32_t)(uint8_t)(PlayersData[faction_id].region_total_offensive_vehs[v18]) < v75)
              v9 *= 2;
          }
          else {
            v8 = v171;
          }
          v75 = v167;
          v11 = has_abil(v167,0x40000);
          if ((((v11) && (v11 = stack_check(v153,6,0x40000,-1,-1), !v11)) && (4 <= (int)(PlayersData[faction_id].planet_ecology))) && (v9 = (int)(v9 * 2) / 3, (int)(PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::MORALE]) <= 0))
            v9 = (int)(v9 * 2) / 3;
          if (has_abil(v75,0x100)) {
            if (BaseCurrent->state & 0x100000)
              v9 = (int)v9 / 2;
            else {
              v13 = -1;
              v77 = -1;
              v16 = 0x100;
              v11 = 6;
              v75 = veh_at((int)BaseCurrent->x,(int)BaseCurrent->y);
              if (stack_check(v75,v11,v16,v77,v13))
                v9 *= 3;
              if (v120)
                v9 = (int)v9 / 2;
              else if ((!(int)(PlayersData[faction_id].unk_66)) && (!(int)(PlayersData[faction_id].unk_68)))
                v9 <<= 2;
            }
          }
          if (((v160) || (v157)) || (v147)) {
            if (0x1a <= GameDataStateStorage.fields_.turn_current_num_)
              v9 *= 2;
            if (0x4c <= GameDataStateStorage.fields_.turn_current_num_)
              v9 *= 2;
          }
          goto label_4fc68f;
        case 3:
          v154 = has_abil(v167,0x40000);
          v8 = v171;
          if ((!v162) && ((v115 || (!v154)))) {
            if ((v147) || (regional_plan)) {
              if (regional_plan == 2) {
label_4fac9f:
                v142 = 1;
              }
              if (regional_plan != 7) goto label_4fabb2;
              v11 = 1;
            }
            else {
              if ((prototype_count_2[v75]) || (prototype_count_0[v75])) goto label_4fac9f;
label_4fabb2:
              v11 = (int)(Continents[region].tile_count + 199) / 200;
            }
            v16 = prototype_count_2[v75];
            if ((v11 < v16 + prototype_count_0[v75] * 2) && (regional_plan != 8))
              v142 = 1;
            v75 = range(GameDataStateStorage.fields_.turn_current_num_ / 0x32,0,(int)(PlayersData[faction_id].current_num_bases) + -1);
            if ((int)(PlayersData[faction_id].current_num_bases) - v75 < v16)
              v142 = 1;
          }
          if (((((regional_plan != 8) && (v114)) || ((int)(v98 + v147 * 2) <= v114)) || ((!v147 && (((v160 || (v156)) || (v157)))))) || (((int)(PlayersData[faction_id].current_num_bases) + 1) / (int)((v147 == 0) + 1) <= v114)) {
            if ((v154) && (!v115)) {
              v9 *= 0x20;
              if (BaseCurrent->eco_damage) {
                if (regional_plan == 2) goto label_4fad22;
                v9 = (int)v9 / 2;
              }
              if ((int)regional_plan > 2)
                v9 = (int)v9 / 2;
            }
            else {
              if ((v162) || ((v147 || (v8 = 0x80, (int)(PlayersData[faction_id].best_armor_value) <= 1))))
                v8 = 0x20;
              v9 = v8 * v9;
            }
          }
          else {
            v9 <<= 4;
          }
label_4fad22:
          if (((v160) && ((int)(PlayersData[faction_id].best_armor_value) <= 1)) && (!v158))
            v9 = (int)v9 / 2;
          goto label_4fc68f;
        case 4:
          v18 = v9 * 0x40;
          v8 = (int)(PlayersData[faction_id].unk_106);
          v75 = (int)(PlayersData[faction_id].unk_105);
          if (v75 <= v8 * 2) {
            v8 = v164 + v161 * 3;
            v154 = plan_count_3[v8] + plan_count_1[v8];
            if (((int)(PlayersData[faction_id].unk_66) << 2) / 3 < (int)v154)
              v18 = v9 * 0x80;
          }
          else {
            if ((v8 < v75 / 2) && (v18 = (int)v18 / 2, v8 < (int)(v75 + (v75 >> 0x1f & 3U)) >> 2))
              v18 = (int)v18 / 2;
          }
          v9 = v18;
          v8 = v164 + v161 * 3;
          v75 = plan_count_1[v8];
          if ((int)(plan_count_3[v8] + v75) < (int)(PlayersData[faction_id].unk_66)) {
            if (!v75)
              v9 = (int)v9 / 2;
            v9 = (int)v9 / 2;
            if (regional_plan != 2) {
              if (!v75)
                v9 = (int)v9 / 2;
              if ((int)((plan_count_3[v8] + v75) * 2) <= (int)(PlayersData[faction_id].unk_66))
                v9 = (int)v9 / 2;
              v9 = (int)v9 / 2;
            }
          }
          if ((!(BaseCurrent->state & 0x100000)) && (v120))
            v9 = (int)v9 / 2;
          goto label_4fc68f;
        case 5:
          v9 *= 0x40;
          if ((v160) && (v9 = (int)v9 / 2, v156))
            v9 = (int)v9 / 2;
          v8 = v164 + v161 * 3;
          if (plan_count_2[v8] <= 1) {
            if ((!plan_count_3[v8]) && (plan_count_1[v8] <= 1))
              v9 = (int)v9 / 2;
            v151 = 0x4;
            v155 = 0;
            v18 = 1;
            scan_faction = &PlayersData[1];
            do {
              if ((v18 != faction_id) && ((v8 = has_treaty(faction_id,v18,1), !v8 || ((uint8_t)(1 << ((uint8_t)v18 & 0x1f)) & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)))) {
                if ((int)v151 <= scan_faction->satellites_[3])
                  v151 = scan_faction->satellites_[3];
                if ((!v155) && (((v8 = has_treaty(faction_id,v18,0x30), v8 || (scan_faction->major_atrocities)) && ((int)(PlayersData[faction_id].planet_busters) < scan_faction->satellites_[3] + 1)))) {
                  v155 = 1;
                  v9 = (int)v9 / 2;
                }
              }
              ++scan_faction;
              v18 += 1;
            } while (scan_faction < &PlayersData[8]);
            if ((int)(PlayersData[faction_id].planet_busters) <= (int)v151) {
              if (!((uint8_t)v165 & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)) {
                if ((uint8_t)(PlayersData[faction_id].flags_ext) & 0x10)
                  v9 = (int)v9 / 2;
                if (PlayersData[faction_id].flags & 0x1000000)
                  v9 = (int)v9 / 2;
              }
              goto label_4fc68f;
            }
          }
          break;
        case 6:
          v8 = naval_base(BaseIDCurrentSelected);
          if (v8) {
            v9 = (int)(((-(uint32_t)(v144 != 1) & 0xffffff80) + 0x100) * v9 * 0x38) / (int)MapAreaSqRoot;
            if (regional_plan == 7)
              v9 = (int)(v9 * 2) / 3;
            if (has_fac_built(0x1c))
              v9 = (int)((v108 + 2) * v9) / (v108 + 3);
            if ((char)(VehPrototypes[(v163 / 0x34)].unk_1) != '\x15') {
              v75 = at_goal(faction_id,6,(int)BaseCurrent->x,(int)BaseCurrent->y);
              v8 = v171;
              if (v75)
                v9 = (int)v9 / 2;
              v18 = (uint32_t)(PlayersData[faction_id].unk_64);
              v75 = (&plan_count_1[18])[v164];
              v11 = (int)(PlayersData[faction_id].unk_65);
              v75 = (range(((v75 - (v18 & 0x7ff)) * 2 + 8) - v11 / 2,0,8) + 8) * v9;
              v75 += v75 >> 0x1f & 0xfU;
              v9 = v75 >> 4;
              if (plan_count_1[19] + plan_count_3[19] * 2 < v123) {
                v75 = v9 - (v75 >> 0x1f);
                v9 = v75 >> 1;
                if (regional_plan == 7)
                  v9 = (int)(v9 - (v75 >> 0x1f)) >> 1;
              }
              if ((plan_count_3[19] + plan_count_1[19] < plan_count_3[22] + plan_count_1[22]) && (v9 = (int)v9 / 2, regional_plan == 7))
                v9 = (int)v9 / 2;
              if (v147)
                v9 = (int)(v9 * 3 + ((int)(v9 * 3) >> 0x1f & 3U)) >> 2;
              if (PlayersData[faction_id].flags & 0x80000)
                v9 = (int)v9 / 2;
              if ((uint32_t)(Players[faction_id].rule_flags) & 0x100)
                v9 = (int)v9 / 2;
              if (v158)
                v9 = (int)v9 / 2;
              if ((v160) && (v158))
                v9 = (int)v9 / 2;
            }
            else {
              if (v158)
                v9 = (int)v9 / 2;
              if (plan_count_1[22] <= (int)(plan_count_3[v161 * 3 + 1] + plan_count_1[v161 * 3 + 1]))
                v9 *= 2;
              if ((int)((plan_count_3[v161 * 3 + 1] + plan_count_1[v161 * 3 + 1]) * 3) / 2 < plan_count_1[22])
                v9 = (int)v9 / 2;
            }
            goto label_4fc68f;
          }
          break;
        case 7:
          if ((v164 != 1) || (v8 = transport_base(BaseIDCurrentSelected), v8)) {
            if (regional_plan != 7)
              v8 = (-(uint32_t)(regional_plan != 8) & 0x40) + 0x40;
            else {
              v8 = 0x10;
            }
            v9 = v8 * v9;
            if ((((v147) || ((v160 && (regional_plan == 7)))) && (v9 = (int)v9 / 2, !(&plan_count_3[18])[v18 + 3])) && ((!(char)(PlayersData[faction_id].proto_id_active[v167]) && (v161 == 7))))
              v9 = (int)(v9 + ((int)v9 >> 0x1f & 3U)) >> 2;
            if ((v158) && ((int)v162 <= 2)) {
              v8 = v9 + ((int)v9 >> 0x1f & 3U);
              v9 = v8 >> 2;
              if (!(&plan_count_1[18])[v18 + 3])
                v9 = (int)(v9 + (v8 >> 0x1f & 3U)) >> 2;
            }
            v8 = has_abil(v167,0x80000);
            if (v8)
              v9 = (int)(v9 * 3 + ((int)(v9 * 3) >> 0x1f & 3U)) >> 2;
            v8 = (int)(PlayersData[faction_id].current_num_bases) / 2;
            if (v8 <= 1)
              v8 = 2;
            v75 = (&plan_count_1[18])[v18 + 3];
            if (v8 <= v75)
              v9 *= 2;
            if (v161 == 7) {
              if (((&plan_count_3[18])[v18] + (&plan_count_1[18])[v18] + v75 + (&plan_count_3[18])[v18 + 3] <= 1) && (GameDataStateStorage.fields_.turn_current_num_ <= 99))
                v9 = (int)v9 / 2;
              if ((((!v75) && (!(&plan_count_3[18])[v18 + 3])) && (((&plan_count_1[18])[v18 + 6] || ((&plan_count_3[18])[v18 + 6])))) && (3 <= (int)regional_plan))
                v9 = (int)v9 / 2;
            }
            if (BaseCurrent->unk_1)
              v9 *= 2;
            if (regional_plan != 7) {
              if (v144 == 1)
                v9 *= 2;
              if ((!((uint8_t)v165 & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)) && (((1 < prototype_count_2[v167] || (regional_plan == 2)) || ((uint8_t)((PlayersData[faction_id].unk_77[region] >> 8)) & 4))))
                v142 = 1;
            }
            else if (((int)v162 <= 1) && (v161 == 7)) {
              v75 = 0;
              v8 = 1;
              do {
                if ((char)(PlayersData[faction_id].region_total_bases[v8]))
                  v75 += 1;
                v8 += 1;
              } while (v8 < 0x80);
              v8 = (int)((uint8_t)(PlayersData[faction_id].region_total_bases[region]) + 3) >> 2;
              if (v8 < v75)
                v8 = v75;
              if (((int)((&plan_count_1[18])[v18 + 3] - (uint32_t)((char)(PlayersData[faction_id].proto_id_active[13]) != '\0')) < v8) && (!(&plan_count_3[18])[v18 + 3]))
                v9 = (int)(v9 + ((int)v9 >> 0x1f & 3U)) >> 2;
            }
            goto label_4fc68f;
          }
          break;
        case 8:
          v18 = GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xff & v165;
          if ((v18) && ((BaseCurrent->governor & 0xdfbe00) == 0x20000)) goto label_4fb650;
          if ((((v18) && (BaseCurrent->governor & 0x80000000)) || ((((!v18 && ((int)v162 < 3)) || (2 <= BaseCurrent->population_size)) && (((int)((&plan_count_2[24])[v164] + (&plan_count_0[24])[v164]) <= 1 || (regional_plan == 8)))))) && ((1 <= BaseCurrent->nutrient_surplus || ((!(&plan_count_1[18])[v164 + 6] && (!(&plan_count_2[24])[v164])))))) {
            if (v164) {
              if (((v158) || ((!v18 && ((uint8_t)(PlayersData[faction_id].flags_ext) & 4)))) || ((!(char)(PlayersData[faction_id].proto_id_active[v167]) && (!prototype_count_1[v167])))) {
label_4fb650:
                v76 = BaseCurrent->state & 0x1000000;
                if (v76) {
                  v10 = ((((int)(PlayersData[faction_id].current_num_bases) - GameDataStateStorage.fields_.turn_current_num_ / (int)((-(uint32_t)(v18 != 0) & 0x14) + 0x14)) - v148) - v131) - 1;
                  v166 += v10 & (uint32_t)((int)v10 < 0) - 1;
                }
                v10 = v9 * 5;
                if (('\x02' <= BaseCurrent->population_size) || ((BaseCurrent->mineral_surplus <= BaseCurrent->nutrient_surplus && (1 <= BaseCurrent->nutrient_surplus)))) {
                  if ((3 <= (int)regional_plan) && (!v76)) {
                    if (v18) {
                      if (((regional_plan == 7) && (GameDataStateStorage.fields_.base_current_count_ / 5 <= (int)(PlayersData[faction_id].current_num_bases))) && (10 <= (int)(PlayersData[faction_id].current_num_bases))) goto label_4fb765;
                      if ((!v18) || (!(BaseCurrent->governor & 0x80000000))) goto label_4fb755;
                      if (!v147) {
                        if ((v156) || (v160))
                          v10 = (int)v10 / 2;
                        goto label_4fb765;
                      }
                      v8 = range(GameDataStateStorage.fields_.diff_level_current_ + 1,2,99);
                    }
                    else {
label_4fb755:
                      v8 = GameDataStateStorage.fields_.diff_level_current_ + 1;
                    }
                    v10 = (int)v10 / v8;
                  }
                }
                else {
                  v10 = v9 * 10;
                }
label_4fb765:
                v76 = faction_id;
                v18 = regional_plan;
                v9 = v10;
                if ((uint8_t)(PlayersData[faction_id].flags_ext) & 1)
                  v9 = (int)v10 / 2;
                v151 = (region + faction_id * 0x1066);
                if ((uint8_t)((PlayersData[faction_id].unk_77[region] >> 8)) & 4) {
                  v9 *= 2;
                  v142 = 1;
                }
                if ((GameDataStateStorage.fields_.game_rules_ & 0x20000000) && (v9 = (int)v9 / 2, regional_plan == 8))
                  v9 = (int)v9 / 2;
                if (v164) {
                  if (v158) {
                    v9 = (int)v9 / 2;
                    if ((int)(PlayersData[faction_id].current_num_bases) <= 3)
                      v9 = (int)v9 / 2;
                    if ((int)(PlayersData[faction_id].current_num_bases) <= 7)
                      v9 = (int)v9 / 2;
                    if ((uint32_t)(Players[faction_id].rule_flags) & 0x100)
                      v9 = (int)v9 / 2;
                  }
                  if (((2 <= (int)MapOceanCoverage) && (v9 = (int)v9 / 2, (int)v162 <= 2)) && ((v8 = v164 + v161 * 3, !plan_count_2[v8] && ((!plan_count_0[v8] && (!(BaseCurrent->state & 0x1000000)))))))
                    v9 = (int)v9 / 2;
                  if ((int)Rules.nutrient_req_citizen <= BaseCurrent->nutrient_surplus) {
                    if ((regional_plan == 7) || (v158)) {
                      v9 = (int)v9 / 2;
                      goto label_4fba09;
                    }
label_4fba0d:
                    if ((regional_plan != 7) && (((uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (uint8_t)v165 || (!((uint8_t)(PlayersData[faction_id].flags_ext) & 4)))))
                      v9 *= 2;
                  }
                  else {
label_4fba09:
                    if (!v158) goto label_4fba0d;
                  }
                  if ((!((uint8_t)v165 & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)) && ((uint8_t)(PlayersData[faction_id].flags_ext) & 4))
                    v9 = (int)v9 / 2;
label_4fba4f:
                  v76 = faction_id;
                  if ((regional_plan != 8) || (2 <= (uint8_t)(PlayersData[faction_id].region_total_bases[region]))) goto label_4fba68;
                }
                else {
                  if (((uint8_t)v165 & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_) && (100 < GameDataStateStorage.fields_.turn_current_num_)) break;
                  if ((((regional_plan == 8) && (((plan_count_0[9] || (plan_count_2[9])) || (!(char)(PlayersData[faction_id].unk_83[region]))))) && (((plan_count_0[27] || (plan_count_2[27])) || (v8 = has_tech((int)VehPrototypes[1].preq_tech,faction_id), !v8)))) && (!((uint8_t)(PlayersData[faction_id].unk_77[region]) & 0x20)))
                    v9 = (int)v9 / 2;
                  if ((((!plan_count_1[v161 * 3]) && (!plan_count_3[v161 * 3])) && ((0 <= BaseCurrent->nutrient_surplus && (v18 != 2)))) && ((!((uint16_t)(PlayersData[faction_id].unk_77[region]) & 0x401) || (v18 == 7))))
                    v9 = (int)v9 / 2;
                  if (((3 <= (int)v162) || (plan_count_0[v161 * 3])) || (plan_count_2[v161 * 3])) goto label_4fba4f;
                  if (v18 == 8) {
label_4fb90c:
                    v9 = (int)v9 / 2;
                    if ((int)(PlayersData[faction_id].current_num_bases) == 1)
                      v9 = (int)(v9 + ((int)v9 >> 0x1f & 3U)) >> 2;
                    goto label_4fba4f;
                  }
                  if (v18 != 7) goto label_4fba4f;
                  if ((!(BaseCurrent->state & 0x1000000)) && ((int)MapOceanCoverage <= 0)) goto label_4fb90c;
label_4fba68:
                  v8 = (int)(PlayersData[faction_id].current_num_bases);
                  if ((3 <= v8) && (v116 == (int)(PlayersData[faction_id].unk_28)))
                    v9 *= 2;
                  if ((4 <= v8) && (v101 < BaseCurrent->population_size))
                    v9 *= 2;
                  if (((uint8_t)v165 & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_) && ((((int)(PlayersData[faction_id].diff_level) + 1) * GameDataStateStorage.fields_.turn_current_num_) / 0x28 <= v8))
                    v9 *= 3;
                }
                if (!strcmp(Players[v76].filename,"BELIEVE")) {
                  if ((!((uint8_t)v165 & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)) && (!((uint8_t)(PlayersData[faction_id].unk_77[region]) & 1)))
                    v9 = (int)v9 / 2;
                }
                if (v147)
                  v9 = (int)v9 / 2;
                goto label_4fc68f;
              }
            }
            else if (((!v158) || (((int)(PlayersData[faction_id].current_num_bases) <= v130 && (3 <= v130)))) && (regional_plan != 2)) goto label_4fb650;
          }
          break;
        case 9:
          v9 *= (-(uint32_t)(v144 != 0) & 2) + 4;
          if (v156)
            v9 = (int)v9 / 2;
          else if (v160)
            v9 *= 2;
          if ((uint8_t)(PlayersData[faction_id].flags_ext) & 2)
            v9 = (int)v9 / 2;
          if (v164 != 1) {
            if (!v158) {
              v155 = (&plan_count_0[24])[v164 + 3];
              if ((((v155) || (prototype_count_0[v167])) || (0x19 <= GameDataStateStorage.fields_.turn_current_num_)) || ((v160 || (v147)))) {
                if ((int)v162 <= 2) goto label_4fbceb;
label_4fbd33:
                if (regional_plan == 2) break;
              }
              else {
                v162 = 0;
label_4fbceb:
                if ((v155) || (prototype_count_0[v167])) goto label_4fbd33;
                if ((!((uint8_t)v165 & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)) && ((regional_plan == 8 || (GameDataStateStorage.fields_.turn_current_num_ <= 0x27))))
                  v9 = (int)(v9 + ((int)v9 >> 0x1f & 0xfU)) >> 4;
                if (regional_plan != 7) goto label_4fbd33;
                v9 = (int)v9 / 2;
              }
              v7 = (uint16_t)(PlayersData[faction_id].unk_77[region]);
              v126 = (int)(((region + faction_id * 0x1066) & 0xffff0000U) | v7);
              if (v7 & 0x400)
                v9 <<= 2;
              if ((!v162) && (!v160)) {
                v154 = BaseCurrent->state;
                if ((!(v154 & 0x20)) && (plan_count_0[v161 * 3 + v164] < (int)(uint32_t)(uint8_t)(PlayersData[faction_id].region_total_bases[region]))) {
                  if ((regional_plan != 2) && (((v134 || (v150)) || ((!v124 && (3 <= (int)regional_plan))))))
                    v9 = (int)v9 / 2;
                  if ((regional_plan == 7) || (v154 & 0x2000000)) {
                    if ((v134) || ((v150 || (!v124))))
                      v9 = (int)(v9 + ((int)v9 >> 0x1f & 3U)) >> 2;
                    else {
                      v9 = (int)v9 / 2;
                    }
                  }
                }
                if (3 <= (int)regional_plan) {
                  v18 = (uint32_t)(uint8_t)(PlayersData[faction_id].region_total_bases[region]);
                  if (plan_count_0[v161 * 3 + v164] <= (int)v18)
                    v9 = (int)v9 / 2;
                  if (((plan_count_0[v161 * 3 + v164] <= (int)((v18 + 1) / 2)) && ((!(v7 & 0x20) || (!v124)))) && ((v134 || ((v150 || (!v124))))))
                    v9 = (int)v9 / 2;
                }
              }
              if (BaseCurrent->state & 0x20)
                v9 *= 3;
              v18 = (uint32_t)(uint8_t)(PlayersData[faction_id].region_total_bases[region]);
              if ((int)v155 < (int)v18) {
                if (((int)((v18 * 2) / 3) <= (int)v155) && ((!regional_plan || (regional_plan == 2))))
                  v9 = (int)(v9 * 3) / 2;
                if (!MapCloudCover)
                  v9 = (int)v9 / 2;
                goto label_4fc68f;
              }
            }
          }
          else if (((((!prototype_count_0[v167]) && (plan_count_1[28] <= v130 / 2)) || (v158)) && (!(BaseCurrent->state & 0x80000))) && (((v158 || ((3 <= (int)regional_plan && ((regional_plan == 7 || (!plan_count_1[28])))))) && ((!v121 || ((v158 || (!plan_count_1[28])))))))) {
            if (!v162) {
              if ((v158) && (prototype_count_2[v167] < v130))
                v9 = (int)v9 / 2;
              if (prototype_count_2[v167] < v130 / 2)
                v9 = (int)v9 / 2;
              if ((!plan_count_1[28]) && (!v121))
                v9 = (int)v9 / 2;
            }
            if (plan_count_0[28] < v130) {
              if (v158)
                v9 = (int)v9 / 2;
              goto label_4fc68f;
            }
          }
          break;
        case 10:
          if ((!((uint8_t)v165 & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)) && (3 <= (int)regional_plan)) {
            v8 = 0;
            v159 = 3;
            if (1 <= GameDataStateStorage.fields_.vehicle_current_count_) {
              scan_vehicle = &Vehs[0];
              do {
                if ((scan_vehicle->faction_id == faction_id) && ((char)(VehPrototypes[scan_vehicle->proto_id].plan) == (char)v17)) {
                  if (scan_vehicle->home_base_id == BaseIDCurrentSelected)
                    v9 *= 2;
                  if (scan_vehicle->order != '\x03') goto label_4fd8f6;
                  if (v159)
                    v159 -= 1;
                  v9 = (int)(v9 * 3) / 2;
                  v17 = v152;
                }
                v8 += 1;
                ++scan_vehicle;
              } while (v8 < GameDataStateStorage.fields_.vehicle_current_count_);
              if (!v159) break;
            }
            if (((!v128) && (regional_plan == 7)) && ((int)(PlayersData[faction_id].ai_tech)))
              v9 = (int)v9 / 2;
            v9 = (int)(v9 * 0x300) / (int)BaseCurrent->population_size;
            goto label_4fc68f;
          }
          break;
        case 0xb:
          v8 = v164 + v161 * 3;
          v140 = v8 * 4;
          if ((plan_count_0[v8] < (int)(uint32_t)(uint8_t)(PlayersData[faction_id].region_total_bases[region])) && ((!((uint8_t)v165 & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_) || ((int)(plan_count_3[v8] + plan_count_1[v8]) <= 1)))) {
            v12 = 0;
            v151 = 0;
            if (((int)v162 <= 1) && ((3 <= GameDataStateStorage.fields_.diff_level_current_ && ((int)v143 <= 1)))) {
              v8 = faction_id * 0x59c;
              v83 = strcmp(Players[faction_id].filename,"PEACE") == 0;
              if (v83)
                v151 = 0x1;
              v12 = (uint32_t)v83;
              if (at_climax(v10)) {
                v12 = (v12 + 1);
                v151 = v12;
              }
            }
            v18 = faction_id;
            if ((uint8_t)(PlayersData[faction_id].flags_ext) & 8) {
              v12 = (v12 + 2);
              v151 = v12;
            }
            if (plan_count_2[v140 / 4] <= (int)v12) {
              v76 = 0xffffffff;
              v155 = 0;
              v132 = 0;
              v127 = 0;
              v9 = 1;
              scan_faction = &PlayersData[1];

              do {
                if (v9 != v18) {
                  v132 += (int)(PlayersData[v18].diplo_mind_control[v9]);
                  v155 += (int)(PlayersData[v18].stolen_data_count[v9]);
                  if (scan_faction->region_total_bases[region]) {
                    if (!has_project(0xf,v9)) {
                      if (has_treaty(v18,v9,8)) {
                        v127 += 1;
                        if (has_treaty(v18,v9,7)) {
                          if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xff & 1 << ((uint8_t)v9 & 0x1f)) || (v165 & GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xff)) {
                            if (v151) goto label_4fc14e;
                            if (has_treaty(v18,v9,0x1000000)) {
                              if (((int)(PlayersData[faction_id].ranking) <= scan_faction->ranking) && ((int)((-(uint32_t)(scan_faction->ranking != 7) & 0xfffffffd) + 6) <= (scan_faction->tech_ranking - (int)(PlayersData[faction_id].tech_ranking)) / 2)) goto label_4fc14e;
                            }
                            else if (((int)(PlayersData[faction_id].ranking) <= scan_faction->ranking) || ((int)(PlayersData[faction_id].tech_ranking) <= scan_faction->tech_ranking)) goto label_4fc14e;
                          }
                        }
                        else {
label_4fc14e:
                          v76 = v9;
                          if (((uint8_t)(1 << ((uint8_t)v9 & 0x1f)) & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_) && (!IsMultiplayerNet)) break;
                        }
                      }
                    }
                  }
                }
                v9 += 1;

                ++scan_faction;
              } while (scan_faction < &PlayersData[8]);
              if ((int)v76 <= -1) {
                if ((((regional_plan == 7) && (1 < GameDataStateStorage.fields_.diff_level_current_)) || (v157)) && (!plan_count_0[v140 / 4]))
                  v76 = GameDataStateStorage.fields_.faction_rankings_[7];
                if ((int)v76 < 0) break;
                v18 = faction_id;
              }
              if (v76 != v18) {
                if ((uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (uint8_t)v165)
                  v126 = 0;
                else {
                  v8 = range((int)(PlayersData[v76].energy_reserves),100,999999);
                  v126 = (int)(PlayersData[faction_id].energy_reserves) / v8;
                }
                if ((uint8_t)v165 & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)
                  v137 = 0;
                else {
                  v8 = range((int)(PlayersData[faction_id].mil_strength_1),(int)(PlayersData[faction_id].current_num_bases) + 1,999999);
                  v137 = ((int)(PlayersData[v76].mil_strength_1) / v8);
                }
                if (GameDataStateStorage.fields_.game_rules_ & 0x40)
                  v125 = 0x5;
                else {
                  v125 = (PlayersData[v76].diff_level);
                }
                v77 = 0x10;
                v16 = 1;
                v11 = (-(uint32_t)(((uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (uint8_t)v165) != 0) & 2) + 2;
                v154 = v76 * 0x20cc;
                v8 = has_treaty(faction_id,v76,7);
                v75 = bit_count(v8);
                v11 = ((((((int)(PlayersData[faction_id].tech_ranking) / v11 - (int)(PlayersData[v76].tech_ranking) / v11) + (int)(PlayersData[v76].soc_effect_pending.values_[SocialEffect::PROBE])) - (int)v125) - (int)v137) - v126) - (int)v151;
                v8 = plan_count_0[v140 / 4];
                v9 = (int)(range(v11 + 0xd + v8 + v75,v16,v77) * v159 * 3) / (int)((v150 - v148) + 3);
                if (!((uint8_t)v165 & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)) {
                  v8 = (int)Chassis[5].preq_tech;
                  if ((has_tech(v8,v76)) && (v8 = has_tech((int)Chassis[5].preq_tech,faction_id), !v8))
                    v9 = (int)v9 / 2;
                  v8 = has_tech((int)Chassis[8].preq_tech,v76);
                  v18 = faction_id;
                  if ((v8) && (v8 = has_tech((int)Chassis[8].preq_tech,faction_id), !v8))
                    v9 = (int)v9 / 2;
                  v8 = (int)Weapon[0x10].preq_tech;
                  if ((has_tech(v8,v76)) && (v8 = (int)Weapon[0x10].preq_tech, !has_tech(v8,v18)))
                    v9 = (int)v9 / 2;
                  v8 = has_tech(Facility[0x66].preq_tech,v76);
                  if ((v8) && (v8 = has_tech(Facility[0x66].preq_tech,v18), !v8))
                    v9 = (int)v9 / 2;
                  scan_tech = 1;
                  do {
                    v5 = Reactor[scan_tech].preq_tech;
                    if ((has_tech((int)v5,v76)) && (v8 = has_tech((int)Reactor[scan_tech].preq_tech,faction_id), !v8))
                      v9 = (int)v9 / 2;
                    ++scan_tech;
                  } while (scan_tech <= 3);
                  if (((int)(PlayersData[faction_id].best_weapon_value) < (int)(PlayersData[v76].best_weapon_value)) || ((int)(PlayersData[faction_id].best_armor_value) < (int)(PlayersData[v76].best_armor_value)))
                    v9 = (int)v9 / 2;
                }
                if (((regional_plan != 7) && ((uint8_t)(PlayersData[faction_id].unk_77[region]) & 0x20)) && (v127)) {
                  if (((v157) && (!plan_count_1[v140 / 4])) && (!plan_count_3[v140 / 4]))
                    v9 = (int)v9 / 2;
                }
                else {
                  v8 = has_tech((int)Chassis[3].preq_tech,faction_id);
                  if ((!v8) || ((plan_count_0[v140 / 4] && ((int)(v132 + v155) < 3)))) break;
                  if ((uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (uint8_t)v165)
                    v9 *= 2;
                  if (plan_count_3[v140 / 4] + plan_count_1[v140 / 4])
                    v9 *= 2;
                }
                if ((!((uint8_t)v165 & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)) && ((uint8_t)(PlayersData[faction_id].flags_ext) & 8))
                  v9 = (int)(v9 + ((int)v9 >> 0x1f & 3U)) >> 2;
                if ((v144 == 1) && (1 <= (int)v141))
                  v166 *= 2;
                goto label_4fc68f;
              }
            }
          }
          break;
        default:
label_4fc68f:
          if ((0x40 <= v167) && (!((uint8_t)(VehPrototypes[(v163 / 0x34)].flags) & 4))) {
            v8 = 0;
            v140 = 0;
            v73[2] = 0;
            v73[1] = 0;
            v73[0] = 0;
            v154 = 0;
            v155 = 0;
            v132 = 0;
            v127 = 0;
            do {
              if (0x40 <= v8)
                v75 = faction_id * 0x40 + -0x40 + v8;
              else {
                v75 = v8;
              }
              if (v75 != v167) {
                v7 = (uint16_t)(VehPrototypes[v75].flags);
                if ((v7 & 1) && ((v75 < 0x40 || (v7 & 4)))) {
                  v8 = weap_val(v75,faction_id);
                  if (v132 <= v8)
                    v132 = weap_val(v75,faction_id);
                  v8 = arm_val(v75,faction_id);
                  if ((int)v155 <= v8)
                    v155 = arm_val(v75,faction_id);
                  v154 |= (uint32_t)(VehPrototypes[v75].ability_flags);
                  v126 = v73[(uint8_t)(Chassis[VehPrototypes[v75].chassis_id].triad)];
                  if (v126 <= speed_proto(v75))
                    v8 = speed_proto(v75);
                  else {
                    v8 = v126;
                  }
                  v73[(uint8_t)(Chassis[VehPrototypes[v75].chassis_id].triad)] = v8;
                  v8 = v127;
                }
              }
              v18 = faction_id;
              v8 += 1;
              v127 = v8;
            } while (v8 < 0x80);
            v75 = weap_val(v167,faction_id);
            v11 = arm_val(v167,v18);
            v16 = speed_proto(v167);
            v8 = initial_facility_10_present;
            if (v161 != 2) {
              if ((int)v161 <= 1) {
                v11 = ((regional_plan != 7) || (!v111));
                if (v132 + v11 < v75)
                  v140 = 1;
              }
            }
            else if ((((char)(Armor[VehPrototypes[(v163 / 0x34)].armor_id].defense_rating) != '\x01') && ((int)v155 < v11)) && (((int)(v155 + 1) < v11 || (regional_plan == 7)))) {
              v9 = (int)(v9 * 2) / 3;
              v140 = 1;
            }
            if (v16 <= v73[(uint8_t)(Chassis[VehPrototypes[(v163 / 0x34)].chassis_id].triad)])
              v75 = v140;
            else {
              v140 = 1;
              v75 = 1;
            }
            if (!initial_facility_10_present) {
              if ((v161 != 5) || ((int)(PlayersData[faction_id].planet_busters))) {
                if (!v75) {
                  v11 = ((v160) || (v157));
                  v11 = ((uint32_t)(2 < (int)regional_plan) - v11) + v131;
                  v166 += range(v11,0,2);
                }
                if ((regional_plan == 8) || ((regional_plan == 7 && (!(BaseCurrent->state & 0x1000000))))) {
                  if (v161 != 8) {
                    if (((!v160) && (v166 = v166 + 1, !v111)) && (!((uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (uint8_t)v165)))
                      v9 *= 2;
                    if (!v75) {
                      if (GameDataStateStorage.fields_.turn_current_num_ <= 0x31)
                        v9 *= 2;
                      if ((GameDataStateStorage.fields_.turn_current_num_ <= 99) && (!((uint8_t)v165 & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)))
                        v9 *= 2;
                    }
                    if (!(BaseCurrent->state & 0x1000000))
                      v9 *= 2;
                  }
                  else {
                    v9 = (int)v9 / 2;
                  }
                }
              }
              if (3 <= (int)v162)
                v9 <<= 3;
            }
            if (prototype_count_1[v167]) {
              if (!v8) break;
            }
            else {
              v75 = (int)(PlayersData[faction_id].unk_28) * 3;
              v75 = (int)(v75 + (v75 >> 0x1f & 3U)) >> 2;
              v11 = (int)(PlayersData[faction_id].unk_28) + -1;
              if (v11 < v75)
                v75 = v11;
              if ((v75 <= v116) && (-0x45 <= BaseCurrent->queue_production_id[0])) {
                if (!v8) {
                  if (v113 <= 1)
                    v9 = (int)v9 / 2;
                  v15 = (uint8_t)(PlayersData[faction_id].region_base_plan[region]);
                  if ((uint8_t)(VehPrototypes[(v163 / 0x34)].plan) == v15)
                    v9 = (int)v9 / 2;
                  if (((char)(VehPrototypes[(v163 / 0x34)].plan) == '\x01') && (v15 <= 2))
                    v9 = (int)v9 / 2;
                  if (((v140) && (v15 == 7)) && (v111))
                    v9 = (int)v9 / 2;
                  if (((char)(VehPrototypes[(v163 / 0x34)].plan) == '\x01') && ((v150 || (v134))))
                    v9 = (int)v9 / 2;
                  if ((!(char)(VehPrototypes[(v163 / 0x34)].plan)) && ((v148 || (v131))))
                    v9 = (int)v9 / 2;
                  goto label_4fcb64;
                }
label_4fcad9:
                v9 = (int)(v9 * 2) / 3;
                if (!v8) goto label_4fcaf5;
              }
              else {
                if (v8) goto label_4fcad9;
label_4fcb64:
                if (v113)
                  v9 = (int)((v113 + 5) * v9) / 5;
label_4fcaf5:
                if ((int)((int)(PlayersData[faction_id].current_num_bases) + ((int)(PlayersData[faction_id].current_num_bases) >> 0x1f & 3U)) >> 2 <= v113) goto label_4fcbbf;
              }
              v3 = (char)(PlayersData[faction_id].region_base_plan[region]);
              switch(v3) {
                case 0:
                case 2:
                  if ((char)(VehPrototypes[(v163 / 0x34)].plan) == v3)
                    v9 = (int)v9 / 2;
                  if ((char)(VehPrototypes[(v163 / 0x34)].plan) == '\x01') goto label_4fcb4e;
                  break;
                case 7:
                  if ((char)(VehPrototypes[(v163 / 0x34)].plan) == '\a')
                    v9 = (int)v9 / 2;
                  if ((char)(VehPrototypes[(v163 / 0x34)].plan) == '\x06')
                    v9 = (int)(v9 * 3 + ((int)(v9 * 3) >> 0x1f & 3U)) >> 2;
                  break;
                case 8:
label_4fcb4e:
                  v9 = (int)(v9 * 3 + ((int)(v9 * 3) >> 0x1f & 3U)) >> 2;

              }
            }
label_4fcbbf:
            if ((3 <= (uint8_t)(Weapon[VehPrototypes[(v163 / 0x34)].weapon_id].mode)) && (!initial_facility_10_present))
              v9 *= 2;
            if ((((int)(PlayersData[faction_id].unk_28) * 2) / 3 <= v116) && (regional_plan != 8))
              v9 = (int)(v9 * 2) / 3;
            if ((int)(PlayersData[faction_id].unk_28) / 2 <= v116) {
              if ((!v96) && (regional_plan != 8))
                v9 = (int)(v9 * 3 + ((int)(v9 * 3) >> 0x1f & 3U)) >> 2;
              if ((v157) && (!initial_facility_10_present))
                v9 = (int)v9 / 2;
            }
            else if ((!initial_facility_10_present) && ((!v140 || (v161 != 2))))
              v9 *= 2;
          }
          v18 = region;
          if (v162) {
            if (v161 != 2) {
              if (v161 != 3) {
                if (v161) {
                  v76 = v162;
                  if ((v161 != 1) || ((((v162 == 3 || (((int)v112 <= (int)(v149 + v122) && (v8 = guard_check(faction_id,region), v8 <= (int)(uint32_t)(uint8_t)(PlayersData[faction_id].region_total_offensive_vehs[v18]))))) && ((!regional_plan || (regional_plan == 7)))) && (v76 = v162, (int)v112 <= (int)(v149 + v122))))) {
label_4fcd31:
                    if (v76 == 3) {
                      if ((!((uint8_t)v165 & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)) || (BaseCurrent->governor & 0x1000))
                        v142 = 1;
                    }
                    goto label_4fcf6d;
                  }
                }
                else {
                  v8 = has_abil(v167,0x8000);
                  if ((v8) && (BaseCurrent->state & 0x200000)) {
                    v76 = v162;
                    goto label_4fcd31;
                  }
                }
              }
              else if ((2 <= (int)(PlayersData[faction_id].best_armor_value)) && ((int)v162 <= 2))
                v9 *= 2;
            }
            v8 = v167;
            v18 = v122;
            if (((!v149) && ((int)v122 < (int)v112)) && (2 <= (uint8_t)(VehPrototypes[(v163 / 0x34)].plan)))
              v9 *= 2;
            v75 = has_abil(v167,0x40000);
            if ((v75) && (v75 = stack_check(v153,6,0x40000,-1,-1), v75 < (int)v18 / 2 + -1))
              v9 = (int)v9 / 2;
            if ((has_abil(v8,0x200)) && (v8 = stack_check(v153,6,0x200,-1,-1), v8 < (int)v18 / 2 + -1))
              v9 = (int)v9 / 2;
            v18 = v161;
            if (((v162 != 3) || ((v9 = v9 * (((uint8_t)(VehPrototypes[(v163 / 0x34)].cost) + 1) / 2), v161 != 2 && ((int)(PlayersData[faction_id].best_armor_value) != 1)))) || ((0x40 <= v167 && (!((uint8_t)(VehPrototypes[(v163 / 0x34)].flags) & 4))))) {
              if ((int)v143 <= 1) {
                v8 = 1 << (3U - (char)v162 & 0x1fU);
                if (v8 <= v166)
                  v166 = v8;
              }
            }
            else {
              v166 = 0;
            }
          }
          else {
            if ((((uint8_t)(VehPrototypes[(v163 / 0x34)].plan) <= 9) && (v8 = BaseCurrent->mineral_intake_2, v8)) && (((uint8_t)(VehPrototypes[(v163 / 0x34)].plan) != 7 || (regional_plan != 7)))) {
              v75 = (int)(PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::SUPPORT]);
              v11 = BaseCurrent->mineral_consumption;
              if (3 <= v75) {
                v16 = (int)BaseCurrent->population_size;
label_4fcf10:
                v75 = 1;
              }
              else {
                if (-4 < v75) {
                  if (-2 <= v75) {
                    v16 = 1;
                    if (v75 != -2)
                      v16 = v75 + 2;
                  }
                  else {
                    v16 = 0;
                  }
                  goto label_4fcf10;
                }
                v16 = 0;
                v75 = 2;
              }
              if (v16 <= (int)BaseCurrentForcesSupported)
                v11 += v75;
              v75 = ((v144) || (v121));
              v11 = (((v134 - v75) - v148) + 3) * v11;
              v9 = (int)(((v11 * v11) / 9 + v8) * v9) / v8;
            }
label_4fcf6d:
            v18 = v161;
          }
          v76 = faction_id;
          v7 = (uint16_t)(PlayersData[faction_id].unk_77[region]);
          if ((v7 & 0x4200) && ((((((v3 = (char)(VehPrototypes[(v163 / 0x34)].plan), !v3 || (v3 == '\x01')) || (v3 == '\x04')) || ((v3 == '\v' || (v3 == '\x05')))) && (v9 = (int)v9 / 2, regional_plan == 2)) && (v7 & 0x4000))))
            v9 = (int)v9 / 2;
          if ((v104) && ((!regional_plan || (1 <= (int)v110)))) {
            if (v164) {
              if (2 <= (int)v104)
                v9 *= 2;
            }
            else if ((uint8_t)(VehPrototypes[(v163 / 0x34)].plan) <= 1) {
              v8 = (4 - v104) * v9;
              v9 = (int)(v8 + (v8 >> 0x1f & 3U)) >> 2;
            }
          }
          if (((uint8_t)(Weapon[VehPrototypes[(v163 / 0x34)].weapon_id].mode) <= 2) && (has_fac_built(0x1e)))
            v9 = (int)((int)((int)v105 + 2) * v9) / (int)((int)v105 + 3);
          v8 = (int)(PlayersData[faction_id].unk_66);
          if (((v8) && ((int)(PlayersData[faction_id].unk_106) < v8)) && ((int)(PlayersData[faction_id].unk_105) < v8)) {
            v8 = has_abil(v167,0x120);
            if (v8)
              v9 = (int)v9 / 2;
            else {
              v8 = has_abil(v167,0x8000);
              if (v8)
                v9 *= 2;
              else if ((!v164) && (!v18))
                v9 = (int)(v9 * 2) / 3;
            }
          }
          v8 = has_abil(v167,0x8000);
          if (v8) {
            if (((!((uint8_t)(PlayersData[faction_id].flags_ext) & 0x40)) && (v5 = BaseCurrent->y, v6 = BaseCurrent->x, !at_goal(v76,0x10,(int)v6,(int)v5))) && (!(BaseCurrent->state & 0x200000))) {
              v8 = v164 + v18 * 3;
              v166 += plan_count_0[v8] + plan_count_2[v8];
            }
            v8 = stack_check(v153,0xf,-1,-1,-1);
            if (!v8) {
              v5 = BaseCurrent->y;
              v6 = BaseCurrent->x;
              if ((at_goal(v76,0x10,(int)v6,(int)v5)) || (BaseCurrent->state & 0x200000))
                v9 = (int)v9 / 3;
              else if (BaseCurrent->event & 0x100) {
                v9 *= 2;
                v9 = (int)v9 / 3;
              }
              if ((int)regional_plan <= 2)
                v9 = (int)(v9 * 2) / 3;
            }
            if (((0 <= (int)v103) || (0 <= (int)v110)) && ((v8 = stack_check(v153,0xf,-1,-1,-1), !v8 || (!v162)))) {
label_4fd242:
              v9 = (int)(v9 * 2) / 3;
            }
          }
          else if ((0 <= (int)v110) && (!v162)) {
            if (v18) {
              if (v18 == 1) {
                v8 = (int)(v9 * 2) / 3 + ((int)(v9 * 2) >> 0x1f);
                v9 = v8 - (v8 >> 0x1f);
              }
            }
            else {
              v9 = (int)v9 / 2;
            }
          }
          else if ((0 <= (int)v103) && (!v162)) {
            if (!v18) goto label_4fd242;
            if (v18 == 1) {
              v8 = (int)(v9 << 2) / 5 + ((int)(v9 << 2) >> 0x1f);
              v9 = v8 - (v8 >> 0x1f);
            }
          }
          v8 = v167;
          v75 = has_abil(v167,8);
          if ((v75) && ((v158 || (regional_plan == 7))))
            v9 = (int)(v9 * 2) / 3;
          v75 = has_abil(v8,0x10);
          v18 = faction_id;
          if (v75) {
            if ((regional_plan != 7) && (regional_plan))
              v9 = (int)(v9 * 2) / 3;
            else {
              v9 = (int)v9 / 2;
              if ((!(char)(PlayersData[faction_id].proto_id_active[v8])) && (!(char)(PlayersData[faction_id].proto_id_queue[v8])))
                v9 = (int)v9 / 2;
            }
          }
          if ((char)(Weapon[VehPrototypes[(v163 / 0x34)].weapon_id].offense_rating)) {
            v75 = worm_mod(BaseIDCurrentSelected,faction_id);
            v8 = morale_mod(BaseIDCurrentSelected,v18,(uint32_t)(uint8_t)(Chassis[VehPrototypes[(v163 / 0x34)].chassis_id].triad));
            v8 += (int)(PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::MORALE]);
            if (v161 == 2) {
              if (has_fac_built(2)) {
                v11 = (v8 - (int)(PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::MORALE])) + 1;
                if (v8 <= v11)
                  v8 = v11;
              }
            }
            if (('\0' <= (char)(Weapon[VehPrototypes[(v163 / 0x34)].weapon_id].offense_rating)) || (0x40 <= v167)) {
              if ((v164 != 2) && (v8 + 1 < v75))
                v166 += 1;
              v9 = (int)(v9 * 8) / range((v75 - v8) + 8,1,0x10);
            }
            else {
              v77 = 2;
              v16 = 0;
              v11 = worm_mod(BaseIDCurrentSelected,faction_id);
              v166 += 2U - range(v11,v16,v77);
              if (v75 < v8)
                v166 += v8 - v75;
              v9 = (int)(v9 * 8) / range((v75 - v8) + 6,1,0x10);
              v11 = (int)(Players[faction_id].rule_psi);
              if (v11)
                v9 = (int)(v9 * 100) / range(v11 + 100,0x32,200);
              if ((((uint8_t)v165 & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_) && (0 <= v8)) && (v75 <= 2)) {
                v166 += 1;
                v9 *= 2;
              }
            }
          }
          v8 = arm_strat((uint32_t)(uint8_t)(VehPrototypes[(v163 / 0x34)].armor_id),faction_id);
          v75 = planet_buster2(v167);
          if (v75)
            v75 = 0x32;
          else {
            v75 = weap_strat((uint32_t)(uint8_t)(VehPrototypes[(v163 / 0x34)].weapon_id),faction_id);
          }
          if ((v162) && (((v161 == 2 || (v161 == 3)) || ((v161 == 1 && ((regional_plan == 2 || (regional_plan == 8))))))))
            v8 *= 2;
          v77 = 999;
          v16 = 1;
          v11 = (int)(VehPrototypes[(v163 / 0x34)].ability_flags);
          v8 = range(bit_count(v11) + v75 + v8,v16,v77);
          v18 = v161;
          v9 = (int)v9 / (v8 * ((int)((int)(uint8_t)(Chassis[VehPrototypes[Vehs[v153].proto_id].chassis_id].speed) / (int)Rules.move_rate_roads) + 1));
          if ((((v121) && (regional_plan == 7)) && ((uint8_t)(VehPrototypes[(v163 / 0x34)].plan) <= 1)) && (!v162)) {
            if (v141) {
              if (v141 == 1)
                v9 = (int)v9 / 2;
            }
            else {
              v9 = (int)(v9 * 2) / 3;
            }
            if (!plan_count_2[v161 * 3 + v164])
              v9 = (int)v9 / 2;
          }
          if (0x100 <= (int)v146)
            v9 *= 2;
          if (((v160) && ((char)(Weapon[VehPrototypes[(v163 / 0x34)].weapon_id].offense_rating))) && (v161 != 2))
            v9 = (int)v9 / 2;
          if (v157)
            v9 *= 2;
          if ((v156) && ((char)(VehPrototypes[(v163 / 0x34)].plan) != '\t'))
            v9 *= (-(uint32_t)(v161 != 2) & 6) + 2;
          if (v147) {
            if ((uint8_t)(VehPrototypes[(v163 / 0x34)].plan) <= 1)
              v9 *= 2;
            if (3 <= (uint8_t)(VehPrototypes[(v163 / 0x34)].plan))
              v9 = (int)v9 / 2;
          }
          v8 = base_lose_minerals(BaseIDCurrentSelected,v167);
          if (1 <= v8) {
            if (((!((uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (uint8_t)v165)) || ((!out_score && (BaseCurrent->governor & 0x80000000)))) && (regional_plan != 2)) {
              v75 = veh_cost(v167,BaseIDCurrentSelected,NULL);
              v11 = cost_factor(faction_id,1,-1);
              v75 *= v11;
              v9 = (int)((v8 + v75) * v9) / v75;
              v18 = v161;
            }
          }
          if (v145 != 0x1) {
            if (v145) {
label_4fd6f8:
              if (((3 <= (int)regional_plan) && ((v15 = (uint8_t)(VehPrototypes[(v163 / 0x34)].plan), v15 != 2 || (v162)))) && ((v15 <= 3 || ((v15 == 8 || (v15 == 0xb))))))
                v9 = (int)v9 / ((int)v145 + 1);
              else if (((char)(VehPrototypes[(v163 / 0x34)].plan) == '\a') && (v145 == 0x2))
                v9 *= 2;
            }
          }
          else {
            if (regional_plan == 7) goto label_4fd6f8;
            v145 = 0;
          }
          v8 = v166 * v9;
          v166 = v8;
          if (v142) {
            v136 = 0;
            if (((((selected_id[1] <= 89999) && (v18 != 9)) && (v18 != 8)) || ((!(char)(VehPrototypes[(v163 / 0x34)].plan) || (v18 == 4)))) || (v18 == 6)) break;
          }
          else {
            if ((uint8_t)v165 & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_) {
              v18 = 0;
              switch((char)(VehPrototypes[(v163 / 0x34)].plan)) {
                case 0:
                  v18 = 0x200;
                  break;
                case 1:
                  v18 = (-(uint32_t)((BaseCurrent->governor & 0x200) != 0) & 0xfffff200) + 0x1000;
                  break;
                case 2:
                  v18 = 0x1000;
                  break;
                case 3:
                  v18 = 0x1000;
                  if ((!(BaseCurrent->governor & 0x1000)) || ((char)(VehPrototypes[(v163 / 0x34)].chassis_id)))
                    v18 = 0x400000;
                  break;
                case 4:
                  v18 = 0x2000;
                  break;
                case 5:
                  v18 = 0x800;
                  break;
                case 6:
                  v18 = 0x400;
                  break;
                case 7:
                case 10:
                  v18 = 0x800000;
                  break;
                case 8:
                  v18 = 0x20000;
                  break;
                case 9:
                  v18 = 0x8000;
                  break;
                case 0xb:
                  v18 = 0x100000;

              }
              if ((0x40 <= v167) && (!((uint8_t)(VehPrototypes[(v163 / 0x34)].flags) & 4)))
                v18 |= 0x80000;
              if ((BaseCurrent->governor & v18) != v18) break;
            }
            v75 = v8;
            if ((out_score) && (v11 = base_claimed(v167,v8,v147,v157,v156,v160), v11))
              v75 = v8 + 100000;
            if (v75 <= selected_score[0]) {
              if ((selected_score[0] <= v75) && (((selected_id[0] <= -1 && ((int)v162 <= 2)) || ((int)v133 <= (int)v9)))) {
                v8 = 999999;
                v166 = 999999;
              }
              else {
                selected_id[0] = v167;
                selected_score[0] = v75;
                v133 = v9;
              }
            }
          }
          v75 = (v136 != 0) + 1;
          if (v8 <= selected_score[v75]) {
            selected_score[v75] = v8;
            selected_id[v75] = v167;
          }
          break;
        case 0xd:
          v9 *= 0x40;
          if (plan_count_2[v161 * 3 + v164] <= 1) {
            if (!((uint8_t)v165 & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)) {
              if ((v150) && (!v148))
                v9 = (int)v9 / 2;
              if ((uint8_t)(PlayersData[faction_id].flags_ext) & 0x10)
                v9 = (int)v9 / 2;
            }
            goto label_4fc68f;
          }
          break;
        case 0xe:
          v9 *= 0x40;
          if (1 <= (int)(PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::PLANET]))
            v9 = (int)v9 / 2;
          if (plan_count_2[v161 * 3 + v164] <= 1) {
            if (!((uint8_t)v165 & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)) {
              if ((v131) && (!v148))
                v9 = (int)v9 / 2;
              if ((uint8_t)(PlayersData[faction_id].flags_ext) & 0x10)
                v9 = (int)v9 / 2;
            }
            goto label_4fc68f;
          }

      }
label_4fd8f6:
      v167 += 1;
    } while (v167 < 0x200);
  }
  v164 = 1;
  v8 = 1;
  do {
    v8 = facility_avail(v8,faction_id,BaseIDCurrentSelected,0);
    v10 = faction_id;
    v76 = regional_plan;
    v9 = v164;
    v18 = v156;
    if (!v8) goto label_4ff11f;
    v167 = 0;
    v136 = 0;
    v145 = (int)(BaseCurrent->specialist_total - BaseCurrent->specialist_unk_1);
    v8 = 999;
    v163 = 0x10000;
    v159 = 0x100;
    switch(v164) {
      case 1:
        if (((regional_plan != 2) && (-3 <= (int)(PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::EFFICIENCY]))) && ((!v99 || ((v99 == 1 && (BaseCurrent->queue_production_id[0] == -1))))))
          v8 = 3 - (int)BaseCurrent->population_size / 5;
        break;
      case 2:
        if (-3 <= (int)(PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::EFFICIENCY]))
          v8 = BaseCurrent->energy_inefficiency;
        else {
          v8 = 0;
        }
        v75 = (int)(PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::MORALE]);
        if (1 <= v75)
          v11 = 1;
        else {
          v11 = (uint32_t)(0 <= v75) - 1;
        }
        v75 = range((v75 + 6 + v11) / 2,1,999);
        v9 = faction_id;
        v75 = (0xcU - v8) - (int)((int)BaseCurrent->population_size * (((int)regional_plan <= 2) + 1)) / v75;
        v8 = has_project(0x18,faction_id);
        if (((!v8) && (3 <= BaseCurrent->mineral_surplus)) && (5 <= v75))
          v75 = 4;
        if (((int)(Players[v9].rule_morale) + (int)(PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::MORALE]) <= -1) && (regional_plan == 2))
          v75 /= 2;
        v8 = range(v75,1,0xc);
        if (((has_fac_built(3)) || (5 <= BaseCurrent->population_size)) && (((int)BaseCurrent->population_size / 2 <= BaseCurrent->energy_inefficiency || (BaseCurrent->energy_intake_2 / 2 <= BaseCurrent->energy_inefficiency))))
          v18 = v156 * 2;
        break;
      case 3:
      case 0x24:
      case 0x25:
      case 0x26:
        v8 = ((v148 - v134) - v150) + 4;
        if (v164 != 3)
          v8 -= (uint32_t)(Players[faction_id].rule_flags) >> 7 & 2;
        if (regional_plan != 7) {
          v166 = v8;
          if ((int)regional_plan <= 2) {
            v8 += 1;
            v166 = v8;
          }
        }
        else {
          v8 -= 1;
          v166 = v8;
        }
        v9 = BaseCurrent->state;
        v152 = v9 & 0x1000;
        if (v152) {
          v8 -= ResourceInfo[4].energy;
          v166 = v8;
        }
        if (v9 & 0x2000) {
          v8 -= ResourceInfo[4].minerals;
          v166 = v8;
        }
        if (v158) {
          if ((v9 & 0x4000) && (v164 == 0x24)) {
            v8 /= 2;
            v166 = v8;
          }
          if ((v9 & 0x2000) && (v164 == 0x25)) {
            v8 /= 2;
            v166 = v8;
          }
          if ((v152) && (v164 == 0x26)) {
            v8 /= 2;
            v166 = v8;
          }
        }
        v142 = 0;
        if (v164 != 3) {
          v145 = 0;
          do {
            v8 = xrange(RadiusOffsetX[v145 / 4] + (int)BaseCurrent->x);
            v75 = RadiusOffsetY[v145 / 4] + (int)BaseCurrent->y;
            if ((on_map(v8,v75)) && (is_ocean(v8,v75))) {
              if (v164 != 0x24) {
                if (v164 != 0x25) {
                  if (v164 != 0x26) goto label_4fdc5e;
                  v9 = bit_at(v8,v75) & 0x40;
                }
                else {
                  v9 = bit_at(v8,v75) & 0x10;
                }
              }
              else {
                v9 = bit_at(v8,v75) & 0x8000;
              }
              if (v9)
                v142 += 1;
            }
label_4fdc5e:
            v145 = (int)((int)v145 + 4);
          } while ((int)v145 <= 0x53);
          if (!v142) {
            v8 = 999;
            goto label_4fef49;
          }
          v8 = v166 - v142;
          v166 = v8;
        }
        if (BaseCurrent->population_size <= 4)
          v166 = v8 + -1;
        v8 = range(v166,1,4);
        v9 = BaseCurrent->state & 0x1000000;
        if ((!v9) && ((regional_plan == 8 || ((int)((uint32_t)(uint8_t)(PlayersData[faction_id].proto_id_active[0]) + (int)(PlayersData[faction_id].current_num_bases)) <= 3))))
          v8 *= 2;
        if (v157) {
          if ((!v9) && (regional_plan == 8)) goto label_4fef0c;
          v18 = 2;
        }
        break;
      case 4:
        v75 = has_project(5,faction_id);
        if (!v75) {
          v75 = 1;
          v11 = 10;
          v8 = 1;
          v136 = 1;
          v8 = range((8U - (int)BaseCurrent->population_size) - (-(uint32_t)(has_fac_built(1) != 0) & 2),v8,v11);
          scan_faction = &PlayersData[1];

          do {
            v11 = has_treaty(faction_id,v75,0x10);
            if (!v11) {
              v18 = has_treaty(faction_id,v75,0x40020);
              v18 |= 6 <= scan_faction->integrity_blemishes;
              if (v18) {
                if ((char)scan_faction->region_total_bases[region])
                  v11 = bit_count(v18);
                else {
                  v11 = 1;
                }
                v8 -= v11;
                if ((char)(PlayersData[faction_id].region_total_bases[region]) == '\x01')
                  v8 -= 1;
              }
            }
            ++scan_faction;
            v75 += 1;

          } while (scan_faction < &PlayersData[8]);
          if (4 <= (int)(PlayersData[faction_id].planet_ecology)) {
            v75 = BaseCurrent->eco_damage;
            v8 -= range(v75 / 10,1,4);
          }
          if ((v8 <= 0) && (!v162))
            v159 = 0x80;
          v8 = range(v8,1,10);
          if (((!v160) || (regional_plan != 2)) || (v162)) goto label_4fec33;
          v18 = v156 | 1;
        }
        break;
      case 5:
      case 0x27:
        v75 = 1;
        v16 = 10;
        v11 = 1;
        v136 = 1;
        v8 = has_fac_built(1);
        v8 = range((10U - (int)BaseCurrent->population_size / 2) - (-(uint32_t)(v8 != 0) & 2),v11,v16);
        v18 = faction_id;
        do {
          if ((!has_treaty(v18,v75,0x10)) && (v11 = has_treaty(v18,v75,0x40020), v11)) {
            if ((uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (uint8_t)(1 << ((uint8_t)v75 & 0x1f)))
              v11 = bit_count(v11);
            else {
              v11 = 1;
            }
            v8 -= v11;
          }
          v75 += 1;
        } while (v75 < 8);
        if ((4 <= (int)(PlayersData[faction_id].planet_ecology)) && (v164 != 0x27)) {
          v75 = BaseCurrent->eco_damage;
          v8 -= range(v75 / 10,1,4);
        }
        if ((v8 <= 0) && (!v162))
          v159 = 0x80;
        v8 = range(v8,1,10);
        if (((v160) && (regional_plan == 2)) && (!v162))
          v18 = v156 | 1;
        else {
label_4fec33:
          v18 = v156;
        }
        break;
      case 6:
        if (2 <= (int)v143) {
          v163 = 0;
          v8 = -10;
          goto label_4fef49;
        }
        v8 = has_project(6,faction_id);
        v8 = (7 - (uint32_t)(v8 != 0)) - (v143 + 2) * (int)v145;
        v18 = v156 | v157;
        if ((4 <= (int)(PlayersData[faction_id].diff_level)) && (v157))
          v18 = 2;
        if (!has_project(0x18,v10)) goto label_4fef49;
        v8 /= 2;
        goto label_4fef47;
      case 7:
        v8 = range((6 - v134) * 2 - BaseCurrent->energy_intake_2 / 2,1,10);
        if ((has_fac_built(3)) || (5 <= BaseCurrent->population_size))
          v18 = v156 * 2;
        break;
      case 8:
        v11 = 10;
        v75 = 1;
        v8 = stack_check(v153,2,0xc,faction_id,-1);
        v8 = range(((5 - v150) - v8) * 2 - BaseCurrent->energy_intake_2 / 3,v75,v11);
        if (has_project(6,v10)) {
          if (2 <= (int)v143) {
            v163 = 0;
            v8 = -10;
          }
          else {
            v75 = 6 - (v143 + 2) * (int)v145;
            if (v75 <= v8)
              v8 = v75;
          }
        }
        v18 = ((-(uint32_t)(has_project(6,v10) != 0) & 2) + 1) * v157;
        break;
      case 9:
        v11 = (5 - v150) - v131;
        v8 = v11 * 2;
        v75 = (int)(Players[faction_id].rule_psi);
        if (1 <= v75) {
          v8 = (v11 * 200) / range(v75 + 100,0x32,200);
          v167 = v144;
        }
        v75 = (int)(PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::MORALE]);
        if (v75 <= -1) {
          v8 = (v75 + -1) / 2 + v8 / 2;
          v167 = v144;
        }
        if ((!v157) && ((((!v160 || (0 <= v75)) || (regional_plan == 2)) || ((uint8_t)((PlayersData[faction_id].unk_77[region] >> 8)) & 0x40)))) goto label_4fe410;
        goto label_4fef0c;
      case 10:
        v163 = 0x90000;
        v75 = (((v113 - v148) * 2 + 6) - (~-(uint32_t)(((uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (uint8_t)v165) != 0) & v141)) + v106;
        if (has_fac_built(0x1b))
          v75 -= 1;
        if (has_fac_built(0x1e))
          v75 -= 1;
        v8 = range(v116,1,999);
        v167 = 1;
        v8 = ((int)(PlayersData[faction_id].unk_28) * v75) / v8;
        v18 = v157;
        break;
      case 0xb:
        if (2 <= (int)v143) {
          v163 = 0;
          v8 = -8;
        }
        else {
          v8 = 8 - (v143 + 2) * (int)v145;
          v75 = has_project(0x18,faction_id);
          if (v75) {
            v8 /= 2;
            goto label_4fef47;
          }
        }
        break;
      case 0xc:
        if (2 <= (int)v143) {
          v163 = 0;
          v8 = -6;
        }
        else {
          v8 = 8 - (int)v145;
        }
        break;
      case 0xd:
        v8 = aah_ooga(faction_id,faction_id);
        if (v8)
          v139 *= 2;
        v16 = 10;
        v8 = 1;
        v75 = ((-((BaseCurrent->eco_damage + 1) / 2) - ((int)(v139 + 2 + ((int)(v139 + 2) >> 0x1f & 3U)) >> 2)) - BaseCurrent->energy_intake_2 / 3) + 0xc;
        goto label_4fef3f;
      case 0xe:
        v16 = 10;
        v8 = 1;
        v75 = ((-((BaseCurrent->eco_damage + 2) / 3) - ((int)(v139 + 2 + ((int)(v139 + 2) >> 0x1f & 3U)) >> 2)) - ((int)(BaseCurrent->energy_intake_2 + (BaseCurrent->energy_intake_2 >> 0x1f & 3U)) >> 2)) + 0xc;
        goto label_4fef3f;
      case 0xf:
        v8 = (6 - v150) * 2 - BaseCurrent->energy_intake_2 / 3;
        goto label_4fe4ee;
      case 0x10:
        v8 = (6 - v150) * 2 - ((int)(BaseCurrent->energy_intake_2 + (BaseCurrent->energy_intake_2 >> 0x1f & 3U)) >> 2);
label_4fe4ee:
        v8 = range(v8,1,0xc);
        v18 = v156 | v157;
        break;
      case 0x11:
        if (2 <= (int)v143) {
          v163 = 0;
          v8 = -4;
        }
        else {
          v11 = 10;
          v75 = 1;
          v8 = stack_check(v153,2,0xc,faction_id,-1);
          v8 = range(((5 - v150) - v8) * 2 - ((int)(BaseCurrent->energy_intake_2 + (BaseCurrent->energy_intake_2 >> 0x1f & 3U)) >> 2),v75,v11);
          v18 = v157;
        }
        break;
      case 0x12:
        if (2 <= (int)v143) {
          v163 = 0;
          v8 = -2;
        }
        else {
          v11 = 10;
          v75 = 1;
          v8 = stack_check(v153,2,0xc,faction_id,-1);
          v8 = range(((6 - v150) - v8) * 2 - BaseCurrent->energy_intake_2 / 5,v75,v11);
          v18 = v157;
        }
        break;
      case 0x13:
        v8 = range(10 - BaseCurrent->mineral_intake_1,1,10);
        if ((!v160) || ((int)regional_plan <= 2)) goto label_4fec33;
        v18 = v156 | 1;
        break;
      case 0x14:
        v16 = 10;
        v75 = 10 - BaseCurrent->mineral_intake_1 / 2;
        v8 = 1;
        goto label_4fef3f;
      case 0x15:
        v16 = 10;
        v75 = 10 - BaseCurrent->mineral_intake_1 / 2;
        v8 = 1;
        goto label_4fef3f;
      case 0x16:
        if (!((uint8_t)v165 & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)) {
          v11 = 0x10;
          v75 = 1;
          v8 = has_fac_built(0x23);
          v8 = range(((((BaseCurrent->drone_total - BaseCurrent->talent_total) - v8) * 2 + 0x10) - BaseCurrent->mineral_intake_1) - (int)(PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::POLICE]),v75,v11);
          if ((!v160) || ((int)regional_plan <= 2)) goto label_4fe410;
          v18 = 1;
        }
        break;
      case 0x17:
        if (!((uint8_t)v165 & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)) {
          v75 = has_fac_built(0x23);
          if (((((int)(PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::POLICE]) + v75 * 2 <= 0) && (2 <= BaseCurrent->drone_total)) && ((int)regional_plan <= 2)) && ((2 <= (int)(PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::ECONOMY]) && ((BaseCurrent->assimilation_turns_left || (!v102))))))
            v8 = range((8U - BaseCurrent->drone_total) * 2 - (int)BaseCurrent->population_size / 2,1,0x10);
          v18 = v160;
        }
        break;
      case 0x18:
      case 0x19:
        v18 = Rules.pop_limit_sans_hab_complex;
        if (v164 != 0x18)
          v18 = Rules.pop_limit_sans_hab_dome;
        v11 = v18 - (int)(Players[faction_id].rule_population);
        v75 = has_project(0xd,faction_id);
        if (v75)
          v11 += 2;
        v75 = range(Rules.nutrient_req_citizen,1,10);
        if (v11 <= (BaseCurrent->nutrient_surplus + 1) / v75 + (int)BaseCurrent->population_size) {
          v16 = 10;
          v75 = 1;
          v8 = range(Rules.nutrient_req_citizen,1,10);
          v8 = range(((v11 - ((int)Rules.nutrient_req_citizen / 2 + BaseCurrent->nutrient_surplus) / v8) - (int)BaseCurrent->population_size) + 4,v75,v16);
          if (3 <= (int)regional_plan)
            v8 /= 2;
        }
        if (has_project(0x18,v10))
          v8 /= 2;
        v18 = v156 | v147;
        if (v11 <= BaseCurrent->population_size) {
          if (!v160)
            v18 *= 2;
          v75 = aah_ooga(faction_id,faction_id);
label_4feda1:
          if (v75) {
            v8 /= 2;
            goto label_4fef47;
          }
        }
        break;
      case 0x1a:
        v5 = BaseCurrent->y;
        v6 = BaseCurrent->x;
        if (alt_at((int)v6,(int)v5) == 3) {
          v5 = BaseCurrent->y;
          v6 = BaseCurrent->x;
          if (GameDataStateStorage.fields_.climate_sea_level_delta_ + AltNatural[3] <= alt_detail_at((int)v6,(int)v5) - MapSeaLevel) {
            if (PlayersData[faction_id].flags & 0x80000)
              v8 = 0;
            else {
              v75 = 0;
              do {
                if (base_project(v75) == BaseIDCurrentSelected) {
                  v8 = alt_detail_at((int)BaseCurrent->x,(int)BaseCurrent->y);
                  if ((int)(WorldBuilder.sea_level_rises + GameDataStateStorage.fields_.climate_sea_level_delta_ + AltNatural[3]) <= v8 - MapSeaLevel) {
                    v16 = 0xc;
                    v8 = (-(uint32_t)(((uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (uint8_t)v165) != 0) & 2) + 4;
                    v75 = 0x14 - (int)BaseCurrent->population_size / 2;
                  }
                  else {
                    v16 = 8;
                    v8 = (-(uint32_t)(((uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (uint8_t)v165) != 0) & 2) + 2;
                    v75 = 8 - (int)BaseCurrent->population_size / 3;
                  }
                  goto label_4fef3f;
                }
                v75 += 1;
              } while (v75 < 0x40);
            }
          }
          else {
            v8 = -0x10;
          }
        }
        break;
      case 0x1b:
      case 0x1e:
      case 0x23:
        v163 = 0x10200;
        v136 = 1;
        v167 = 1;
        v146 = (v164 != 0x1b) ? v105 : v107;
        v8 = v106 + 5 + (v150 - v148) * 2;
        if (v164 != 0x23)
          v79 = ((int)v146 + v8);
        else {
          v79 = (v8 + 5);
        }
        if (!((uint8_t)v165 & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_))
          v79 = (v79 + v141 * -2);
        v8 = fac_maint(v164,faction_id);
        v75 = (v8 + 4) * (int)v79;
        v8 = (int)(Players[v10].rule_psi);
        v75 = (int)(v75 + (v75 >> 0x1f & 3U)) >> 2;
        if (1 <= v8)
          v75 = ((v8 + 100) * v75) / 100;
        if ((((v9 == 0x1b) && (!v146)) && (regional_plan != 7)) && (4 <= (uint8_t)(PlayersData[faction_id].region_total_bases[region])))
          v75 /= 2;
        v8 = range(v75,1,99);
        v15 = (uint8_t)(PlayersData[faction_id].region_total_bases[region]);
        if ((((uint16_t)(PlayersData[v10].region_total_combat_vehs[region]) < (uint16_t)((uint16_t)v15 * 2)) || ((uint8_t)((PlayersData[v10].unk_77[region] >> 8)) & 0x40)) || ((100 <= GameDataStateStorage.fields_.turn_current_num_ && ((v15 <= 8 && (regional_plan != 7)))))) goto label_4fe410;
        v18 = v160;
        break;
      case 0x1c:
        v75 = naval_base(BaseIDCurrentSelected);
        if (v75) {
          v163 = 0x10400;
          v136 = 1;
          v8 = (((((((v150 - v141) - v148) + v131) * 2 + 0x18) - (int)(char)(Bases[base_id].population_size) / 3) - (int)(PlayersData[faction_id].unk_65)) - v95 / 2) + v108;
          if (regional_plan == 7)
            v8 /= 2;
          v8 = range(v8,1,0x18);
          if ((uint32_t)(Players[faction_id].rule_flags) & 0x100)
            v8 /= 3;
        }
        goto label_4feef7;
      case 0x1d:
        v163 = 0x10800;
        v136 = 1;
        v167 = 1;
        v8 = (((((((v150 - (int)(PlayersData[faction_id].unk_66)) - v141) + v131) * 2 - (int)(char)(Bases[base_id].population_size) / 3) - (int)(PlayersData[faction_id].unk_68)) + (8 - v148) * 3) - v93 / 3) + v129;
        if (regional_plan == 7)
          v8 /= 2;
        if (BaseCurrent->state & 0x100000)
          v8 /= 2;
        if (best_reactor_id <= 1)
          v8 *= 2;
        v8 = range(v8,1,0x18);
        if ((!v160) || (v76 == 2)) goto label_4fe410;
        v83 = ((uint8_t)((PlayersData[faction_id].unk_77[region] >> 8)) & 0x40) == 0;
        goto label_4fef06;
      case 0x1f:
      case 0x20:
        v75 = 0xb - BaseCurrent->eco_damage;
        v8 = (int)(Players[faction_id].rule_psi);
        if (1 <= v8) {
          v75 = (v75 * 100) / range(v8 + 100,0x32,200);
          v167 = v144;
        }
        if ((int)(PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::MORALE]) <= -1) {
          v75 = ((int)(PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::MORALE]) + -1) / 2 + v75 / 2;
          v167 = v144;
          if ((v160) && (!((uint8_t)((PlayersData[faction_id].unk_77[region] >> 8)) & 0x40))) {
            v16 = 0xc;
            v18 = v156 | 1;
            v8 = 1;
            goto label_4fef3f;
          }
          v18 = v156;
        }
        v16 = 0xc;
        v8 = 1;
        goto label_4fef3f;
      case 0x21:
        v16 = 4;
        v8 = 0;
        v75 = ((-(uint32_t)(climactic_battle() != 0) & 0xfffffffe) + 4) - v97;
        v11 = at_climax(faction_id);
        v75 -= v11;
label_4fef3f:
        v8 = range(v75,v8,v16);
label_4fef47:
        break;
      case 0x22:
        v8 = (2 - ((uint32_t)(PlayersData[faction_id].flags_ext) >> 3 & 1)) * 5;
        break;
      case 0x28:
        if ((uint8_t)(Players[faction_id].rule_flags) & 0x80) {
          v75 = generators(faction_id,NULL);
          if (v75 < (int)Rules.subspace_generators_needed) {
            v75 = (int)(char)(Bases[base_id].population_size);
            if ((int)Rules.size_base_subspace_gen <= v75)
              v8 = 1;
            else if ((int)(Rules.size_base_subspace_gen * 4) / 5 <= v75)
              v8 = 6;
          }
        }
        break;
      case 0x29:
        v8 = 7;
        if ((int)(PlayersData[faction_id].ai_growth))
          v153 += 1;
        if ((int)(PlayersData[faction_id].ai_tech))
          v153 -= 1;
        if ((int)(PlayersData[faction_id].ai_wealth))
          v153 -= 3;
        if ((int)(PlayersData[faction_id].ai_power))
          v153 -= 1;
        break;
      case 0x41:
      case 0x42:
      case 0x43:
        v75 = PlayersData[faction_id].satellites_[v164 - 0x41];
        if (v75 < BaseCurrent->population_size) {
          v8 = range((v75 + (4 - v134) * 2 + v148) * 2 - (int)BaseCurrent->population_size,2,0x10);
          if (v9 == 0x41) {
            v75 = has_project(0x18,faction_id);
            goto label_4feda1;
          }
        }
        break;
      case 0x44:
        v75 = 1;
        v163 = 0x12000;
        v136 = 1;
        v149 = 1;
        scan_faction = &PlayersData[1];
        v82 = BaseCurrent;
        do {
          if (v149 != faction_id) {
            v11 = has_treaty(faction_id,v149,1);

            if (!v11) {
              v11 = scan_faction->planet_busters;
              if (scan_faction->major_atrocities)
                v16 = v11;
              else {
                v16 = (v11 + 1) / 2;
              }
              if (v75 <= v16) {
                if (scan_faction->major_atrocities)
                  v75 = v11;
                else {
                  v75 = (v11 + 1) / 2;
                }
              }
              v11 = has_treaty(faction_id,v149,0x30);
              if (!v11) {
                if (!((uint8_t)(1 << ((uint8_t)v149 & 0x1f)) & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)) goto label_4fee63;
              }
              if (v75 <= scan_faction->satellites_[3])
                v75 = scan_faction->satellites_[3];
            }
label_4fee63:
            v82 = BaseCurrent;
          }
          v149 += 1;
          ++scan_faction;
        } while (scan_faction < &PlayersData[8]);
        if (((((int)(PlayersData[faction_id].satellites_[3]) < v75 + 2) && (v8 = range((((int)(PlayersData[faction_id].satellites_[3]) - v75) * 2 + 0xc) - (int)v82->population_size,1,0xc), !(int)(PlayersData[GameDataStateStorage.fields_.ranking_faction_id_unk1_].planet_busters))) && (!(int)(PlayersData[GameDataStateStorage.fields_.ranking_faction_id_unk1_].major_atrocities))) && ((int)(PlayersData[GameDataStateStorage.fields_.ranking_faction_id_unk1_].satellites_[3]) <= (int)(PlayersData[faction_id].satellites_[3])))
          v8 *= 2;
        v167 = 1;
label_4feef7:
        if (v160) {
          v83 = regional_plan == 7;
label_4fef06:
          if (v83) {
label_4fef0c:
            v18 = 1;
            break;
          }
        }
label_4fe410:
        v18 = 0;

    }
label_4fef49:
    v75 = base_lose_minerals(BaseIDCurrentSelected,-v164);
    if (1 <= v75) {
      if ((!((uint8_t)v165 & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)) || ((BaseCurrent->governor & 0x80000000 && (!out_score)))) {
        v11 = cost_factor(faction_id,1,-1);
        v11 *= (int)(Facility[v164].cost);
        if (v11)
          v159 = (int)((v75 + v11) * v159) / v11;
      }
    }
    v8 = (int)(v8 * v159 * 0x20) / (int)(((v134 - v148) + 3) * (v18 + 1));
    if (((v167 != 0) != (v144 != 0)) || ((v166 = v8, v144 && (v162)))) {
      v8 *= 2;
      v166 = v8;
    }
    if ((regional_plan == 2) || ((uint8_t)((PlayersData[faction_id].unk_77[region] >> 8)) & 0x40)) {
      v8 *= 2;
      v166 = v8;
    }
    if (((int)regional_plan <= 2) && (!v136)) {
      v8 *= 2;
      v166 = v8;
    }
    if (((0 <= (int)v110) && (!v136)) && ((1 <= (int)v141 || (((uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (uint8_t)(1 << ((uint8_t)v110 & 0x1f)) && ((0 <= (int)v141 || (v75 = has_treaty(faction_id,v110,0x20), v75))))))))
      v166 = v8 * 2;
    v8 = 0;
    do {
      v18 = v164;
      v75 = v166;
      if (v8) {
        v11 = (v136 != 0) + 1;
label_4ff0fc:
        if (v75 < selected_score[v11]) {
          selected_score[v11] = v75;
          selected_id[v11] = -v18;
        }
      }
      else {
        if (!((uint8_t)v165 & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)) {
label_4ff0fa:
          v11 = 0;
          goto label_4ff0fc;
        }
        if (BaseCurrent->governor & 0x10000) {
          if ((BaseCurrent->governor & v163) != v163)
            v75 = v166 + 100000;
          if ((out_score) && (v11 = base_claimed(-v164,v75,v147,v157,v156,v160), v11))
            v75 += 100000;
          goto label_4ff0fa;
        }
      }
      v8 += 1;
    } while (v8 < 2);
label_4ff11f:
    v8 = v164 + 1;
    v164 = v8;
  } while (v8 <= 0x45);
  if (selection_mode) goto label_4ffea4;
  if (((!v128) || ((v15 = (uint8_t)(PlayersData[faction_id].region_base_plan[region]), v15 <= 2 && ((int)((v15 != 2) + 1) <= v119)))) && ((v18 = GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xff & v165, !v18 || ((BaseCurrent->governor & 0xdfbe00) != 0x40000)))) {
    if ((((((char)(PlayersData[faction_id].region_base_plan[region]) == '\x02') || (v4 = BaseCurrent->population_size, v4 < 3)) || (2 <= (int)v162)) || (((v162 && ((int)v138 <= 1)) && (!v18)))) || (v116 < v4 * 2)) goto label_4ffea4;
    v8 = BaseCurrent->drone_total;
    if (((BaseCurrent->talent_total < v8) || (BaseCurrent->state & 2)) || (((v8 && (((BaseCurrent->talent_total <= v8 && (BaseCurrent->specialist_unk_1 < BaseCurrent->specialist_total)) && (v4 < 8)))) || (((int)v4 <= (v118 + 1) / 2 && (v116 <= ((int)(PlayersData[faction_id].unk_28) + 1) / 2)))))) goto label_4ffea4;
    if (regional_plan != 7)
      v18 = (uint32_t)(uint8_t)(PlayersData[faction_id].region_total_bases[region]);
    else {
      v18 = (uint32_t)(PlayersData[faction_id].current_num_bases);
    }
    if ((int)(v18 + v119 * -3) < (int)((((regional_plan == 8) - v157) - v150) + 3 + v148)) goto label_4ffea4;
  }
  base_id = 0;
  do {
    v18 = faction_id;
    v8 = base_id + 0x46;
    v75 = facility_avail(v8,faction_id,BaseIDCurrentSelected,0);
    if ((v75) && ((((uint8_t)v165 & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_ || (3 <= GameDataStateStorage.fields_.diff_level_current_)) || ((v75 = (int)(Facility[v8].preq_tech), v75 < 0 || ((uint8_t)(GameDataStateStorage.fields_.game_tech_achieved_[v75]) & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)))))) {
      v167 = 0;
      v139 = v30[base_id] & v165;
      v154 = v29[(int)(base_id + (base_id >> 0x1f & 7U)) >> 3];
      v152 = (((((((v119 * 2 + 8) - (int)(Facility[v8].sp_ai_fight) * v141) - (int)(Facility[v8].sp_ai_power) * v148) - (int)(Facility[v8].sp_ai_wealth) * v134) - (int)(Facility[v8].sp_ai_tech) * v150) - (int)(Facility[v8].sp_ai_growth) * v131) - (uint32_t)(5 <= (int)(PlayersData[faction_id].ranking))) - (int)BaseCurrent->population_size;
      v75 = 0x100;
      v166 = v100 / 2 + v154 + (v152 - GameDataStateStorage.fields_.turn_current_num_ / 0x28) + v135;
      selection_mode = v156;
      if (v157)
        selection_mode = 2;
      if ((base_id != 7) && (base_id)) {
label_4ff44c:
        if ((((base_id == 8) || (base_id == 0x10)) || (base_id == 0x13)) || (base_id == 9)) {
          v75 = ((int)(Players[v18].rule_morale) + 8 + (int)(PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::MORALE])) * v75;
          v75 = (int)(v75 + (v75 >> 0x1f & 7U)) >> 3;
        }
        if (((!base_id) || (base_id == 6)) || ((base_id == 0xe || ((base_id == 0x1a || (base_id == 0x1e)))))) goto label_4ff4a5;
      }
      else {
        if (regional_plan == 8)
          v75 = 0xaa;
        if (base_id) goto label_4ff44c;
        if (((int)(Players[v18].rule_talent)) || (1 <= (int)(PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::TALENT])))
          v75 *= 2;
label_4ff4a5:
        if ((int)(Players[v18].rule_talent))
          v75 = (v75 * 3) / 2;
        if ((int)(Players[v18].rule_drone))
          v75 /= 2;
      }
      if (base_id != 5) {
        if (base_id == 1) {
label_4ff539:
          v7 = (uint16_t)(PlayersData[v18].unk_77[region]);
          if (v7 & 0x40)
            v75 /= 2;
          else if (v7 & 0x20)
            v75 = (v75 * 2) / 3;
        }
        if (((base_id == 10) || (base_id == 2)) && (regional_plan == 7))
          v75 /= 2;
        if (base_id != 0xf) {
          if (base_id != 0x22) {
            if (((base_id == 0x24) || (base_id == 0x21)) && ((int)(PlayersData[faction_id].ai_wealth)))
              v75 /= 2;
            if (base_id != 0x23) {
              if (base_id != 0xb) {
                if (base_id == 4) {
                  v9 = 1;
                  v18 = faction_id;
                  do {
                    if (v18 != v9) {
                      v11 = has_treaty(faction_id,v9,8);
                      v75 = ((-(uint32_t)(v11 != 0) & 2) + 7) * v75;
                      v75 = (int)(v75 + (v75 >> 0x1f & 7U)) >> 3;
                      v18 = faction_id;
                    }
                    v9 += 1;
                  } while ((int)v9 < 8);
                  if (GameDataStateStorage.fields_.governor_faction_ == v18)
                    v75 /= 2;
                  if ((int)(PlayersData[faction_id].ranking) == 7)
                    v75 /= 2;
                  v16 = 0;
                  v11 = (int)(Players[v18].faction_bonus_count);
                  if (1 <= v11) {

                    do {
                      if (Players[v18].faction_bonus_id[v16] == 0xb) goto label_4ff5f6;
                      v16 += 1;

                    } while (v16 < v11);
                  }
                }
              }
              else {
                scan_faction = &PlayersData[1];
                do {
                  if ((int)(PlayersData[faction_id].tech_ranking) <= scan_faction->tech_ranking)
                    v75 = (int)(v75 * 7 + (v75 * 7 >> 0x1f & 7U)) >> 3;
                  ++scan_faction;
                } while (scan_faction < &PlayersData[8]);
              }
            }
            else {
              v7 = (uint16_t)(PlayersData[v18].unk_77[region]);
              if (v7 & 0x40) goto label_4ff5f6;
              if (v7 & 0x20)
                v75 = (v75 * 2) / 3;
            }
          }
          else if ((uint8_t)(PlayersData[faction_id].flags_ext) & 8) {
label_4ff5f6:
            v75 /= 2;
          }
        }
        else {
          if (!(int)(PlayersData[faction_id].ai_power))
            v75 /= 2;
          if ((int)(PlayersData[faction_id].ai_tech))
            v75 /= 2;
          if ((int)(PlayersData[faction_id].soc_effect_base.values_[SocialEffect::PROBE]) <= -1)
            v75 /= 2;
          if (1 <= (int)(PlayersData[faction_id].soc_effect_base.values_[SocialEffect::PROBE]))
            v75 *= 2;
        }
        if (((!v128) && (base_id <= 0x1e)) && ((regional_plan == 2 || ((uint8_t)((PlayersData[faction_id].unk_77[region] >> 8)) & 0x40))))
          v75 *= 2;
        if (((base_id == 0x1f) && (!v117)) && (GameDataStateStorage.fields_.game_rules_ & 0x800)) {
          v75 /= 2;
          if (0 <= Facility[0x65].preq_tech) {
            v18 = 1;
            scan_faction = &PlayersData[1];
            v149 = 1;
            do {
              if ((v18 != faction_id) && ((int)(PlayersData[faction_id].ranking) < scan_faction->ranking)) {
                v11 = has_tech(Facility[0x65].preq_tech,v18);
                if (v11)
                  v75 *= 2;
                else {
                  v75 /= 2;
                }
              }
              ++scan_faction;
              v18 = v149 + 1;
              v149 = v18;
            } while (scan_faction < &PlayersData[8]);
          }
        }
        if ((!v154) && (!v119))
          v75 = (int)(v75 + (v75 >> 0x1f & 3U)) >> 2;
        if (((base_id == 0x20) && (!v117)) && ((int)(PlayersData[faction_id].unk_28) / 2 <= v116))
          v75 = (int)(v75 + (v75 >> 0x1f & 3U)) >> 2;
        clear();
        v154 = faction_id * 0x59c;
        says(Players[faction_id].filename);
        CharUpperA(StringTemp);
        if ((!strcmp(StringTemp,"CARETAKE")) && (base_id == 0x20)) {
          v166 = 99;
          v75 = 0x400;
        }
        v166 = range(v166,1,99);
        v129 = v166 * v75 * 0x20;
        v152 = -(base_id + 0x46);
        v11 = base_lose_minerals(BaseIDCurrentSelected,v152);
        if (1 <= v11) {
          if ((!((uint8_t)v165 & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)) || ((!out_score && (BaseCurrent->governor & 0x80000000)))) {
            v16 = cost_factor(faction_id,1,-1);
            v16 *= (int)(Facility[v8].cost);
            v75 = ((v11 + v16) * v75) / v16;
          }
        }
        v75 = (((int)(PlayersData[faction_id].unk_28) + 8) * v166 * v75 * 0x20) / (int)(((v150 - v148) + 3) * (v116 + 8));
        if (v167 != v144) {
          v11 = cost_factor(faction_id,1,-1);
          v11 = v11 * (int)(Facility[v8].cost) + 1;
          v75 += ((v11 - BaseCurrent->minerals_accumulated) * v75) / v11;
        }
        if (v139)
          v11 = 2;
        else {
          v11 = v117 + 1;
        }
        v16 = (int)(PlayersData[faction_id].energy_reserves);
        v13 = GameDataStateStorage.fields_.turn_current_num_ / 100;
        v77 = BaseCurrent->mineral_surplus;
        v75 = (((v75 * v11 * 0x400) / (v16 + 0x400)) * 8) / (range(v77 / (v13 + 3),0,8) + 8);
        if (selection_mode)
          v75 /= (int)(selection_mode + 1);
        if ((has_fac_built(1)) && (!v139))
          v75 = (int)(v75 * 3 + (v75 * 3 >> 0x1f & 3U)) >> 2;
        if ((!((uint8_t)v165 & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)) && (BaseCurrent->queue_production_id[0] == v152)) {
          v129 = (int)(v129 + ((int)v129 >> 0x1f & 3U)) >> 2;
          v11 = cost_factor(faction_id,1,-1);
          if ((v11 * (int)(Facility[v8].cost) <= BaseCurrent->minerals_accumulated) && (v75 = v75 / 2, (int)v162 <= 2))
            v162 = 0;
        }
        v8 = 0;
        if (1 <= GameDataStateStorage.fields_.base_current_count_) {
          scan_base = &Bases[0];
          do {
            if (((scan_base->faction_id_current == faction_id) && (v8 != BaseIDCurrentSelected)) && (scan_base->queue_production_id[0] == -(base_id + 0x46))) {
              v5 = scan_base->y;
              v11 = (int)(((uint32_t)(uint16_t)scan_base->y << 16) | (uint16_t)scan_base->x);
              if (region_at((int)(int16_t)v11,(int)v5) != region) goto label_4ffe94;
              if ((!v139) || (v128))
                v75 *= 2;
            }
            v8 += 1;
            ++scan_base;
          } while (v8 < GameDataStateStorage.fields_.base_current_count_);
        }
        if (0 <= v75) {
          v9 = 9999;
          v8 = has_fac_built(0x18);
          v18 = faction_id;
          if ((v8) || (v8 = has_tech(Facility[0x18].preq_tech,faction_id), !v8)) {
            if ((!has_fac_built(0x19)) && (v8 = has_tech(Facility[0x19].preq_tech,v18), v8))
              v9 = Rules.pop_limit_sans_hab_dome;
          }
          else {
            v9 = Rules.pop_limit_sans_hab_complex;
          }
          v76 = v139;
          if ((int)(v9 - (int)(Players[faction_id].rule_population)) <= (int)BaseCurrent->population_size) {
            if (!v128) goto label_4ffe94;
label_4ffbe9:
            v15 = (uint8_t)(PlayersData[faction_id].region_base_plan[region]);
            if ((v15 != 2) || (!v162)) {
              if (3 <= v15) {
                v75 /= 3;
                if (((((int)v162 <= 2) && ((int)v143 <= 1)) && (v75 = v75 / 2, !(BaseCurrent->state & 8))) && ((GameDataStateStorage.fields_.secret_project_[base_id] == -1 && (-0x45 <= selected_id[0])))) {
                  selected_score[0] = 9999999;
                  v143 = 0;
                  v162 = 0;
                }
              }
              else if (!v162)
                v75 /= 2;
            }
          }
          else if (v128) goto label_4ffbe9;
          if ((!v139) || (((v119 && (v5 = BaseCurrent->y, v6 = BaseCurrent->x, !is_ocean((int)v6,(int)v5))) && (plan_count_3[30] + plan_count_1[30] <= 2)))) {
            v118 = base_id + 0x46;
            if (!has_fac_built(6)) {
              if ((!((uint8_t)v165 & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)) || (BaseCurrent->governor & 0x10000)) {
                v8 = has_tech(Facility[6].preq_tech,v18);
                if (!v8) goto label_4ffe94;
                v118 = 6;
              }
            }
            if ((uint8_t)v165 & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_) {
              if (v76) goto label_4ffe94;
            }
            else if ((v76) && (v8 = veh_avail(5,v18,BaseIDCurrentSelected), !v8)) {
              v76 = 0;
              v139 = 0;
            }
            if ((-0x45 <= selected_id[0]) || (v76)) {
              v8 = 0;
              do {
                if ((!((uint8_t)v165 & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)) || (BaseCurrent->governor & 0x40000)) {
                  v11 = v75;
                  if ((out_score) && (v16 = base_claimed(-(base_id + 0x46),v75,v147,v157,v156,v160), v16))
                    v11 = v75 + 100000;
                  if (v11 < selected_score[v8 / 4]) {
                    selected_score[v8 / 4] = v11;
                    if (v139)
                      v11 = 5;
                    else {
                      v11 = -v118;
                    }
                    selected_id[v8 / 4] = v11;
                    v133 = v129;
                  }
                }
                v8 += 4;
              } while (v8 < 8);
            }
            else if ((int)v129 < (int)v133) {
              v133 = v129;
              v8 = 0;
              do {
                if ((!((uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (uint8_t)v165)) || (BaseCurrent->governor & 0x40000)) {
                  v11 = v75;
                  if ((out_score) && (v16 = base_claimed(-(base_id + 0x46),v75,v147,v157,v156,v160), v16))
                    v11 = v75 + 100000;
                  selected_id[v8 / 4] = -v118;
                  if (v75 < selected_score[v8 / 4])
                    selected_score[v8 / 4] = v11;
                }
                v8 += 4;
              } while (v8 < 8);
            }
          }
        }
      }
      else {
        v16 = 0;
        v167 = v144;
        v11 = (int)(Players[v18].faction_bonus_count);
        if (1 <= v11) {

          do {
            if (((Players[v18].faction_bonus_id[v16] == 2) && (Players[v18].faction_bonus_val1[v16] == 4)) || ((Players[v18].faction_bonus_id[v16] == 0xc && (Players[v18].faction_bonus_val1[v16] == 4)))) break;
            v16 += 1;

          } while (v16 < v11);
        }
        if (v16 >= v11) {
          v18 = faction_id;
          goto label_4ff539;
        }
      }
    }
label_4ffe94:
    base_id += 1;
  } while (base_id < 0x40);
label_4ffea4:
  if (out_secondary) {
    *out_secondary = selected_id[1];
    parse_num(1,selected_score[1]);
  }
  if (out_tertiary) {
    *out_tertiary = selected_id[2];
    parse_num(0,selected_score[2]);
  }
  v8 = selected_id[0];
  if ((selected_id[0] < 0) || (!(char)(Weapon[VehPrototypes[selected_id[0]].weapon_id].offense_rating))) {
    v5 = BaseCurrent->y;
    v6 = BaseCurrent->x;
    if ((veh_who((int)v6,(int)v5) <= -1) && ((int)v143 <= 1)) {
      if ((!((uint8_t)v165 & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)) || (BaseCurrent->governor & 0x1000)) {
        v18 = 99999;
        v8 = 0;
        scan_prototype = &VehPrototypes[0];
        do {
          v75 = veh_avail(v8,faction_id,BaseIDCurrentSelected);
          if (v75) {
            v3 = scan_prototype->plan;
            if ((v3 != '\x02') && (v3 != '\x03')) {
              if (v3 != '\x01') goto label_4fff8e;
            }
            if ((uint8_t)scan_prototype->flags & 4U) {
              if ((uint8_t)scan_prototype->cost < v18) {
                v18 = (uint32_t)(uint8_t)scan_prototype->cost;
                selected_id[0] = v8;
              }
            }
          }
label_4fff8e:
          ++scan_prototype;
          v8 += 1;
        } while (scan_prototype < &VehPrototypes[512]);
        v8 = selected_id[0];
      }
    }
  }
  if (out_score)
    *out_score = selected_score[0];
  return v8;
}







// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004E5A60 ?base_change@@YAXHH@Z 0x004E5A60-0x004E5FDC
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004e5a60.cpp", "source:src/base.cpp:base_reset", "source:src/base.cpp:base_lose_minerals"], "reason": "Recover production selection, retool mineral adjustment and radius redraw through the typed Base and Rules. Project announcement and faction production counts remain unresolved; this is not a byte or behavioral claim."}
void __cdecl base_change(int base_id, int production_id) {
    if (production_id == 99999) return;

    Base &base = Bases[base_id];
    const int previous = base.queue_production_id[0];
    base.queue_production_id[0] = production_id;
    if (previous == production_id) return;
    PlayerData &faction = PlayersData[base.faction_id_current];
    if (previous >= 0) --faction.proto_id_queue[previous];
    if (production_id >= 0) ++faction.proto_id_queue[production_id];

    // Switching production applies the rule's retool penalty to the stored
    // minerals, while keeping the unpenalized turn total for later accounting.
    // base_lose_minerals already owns the matching classification and arithmetic.
    int loss = 0;
    if (Rules.retool_pct_pen_prod_chg
        && (static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)
            & (1u << base.faction_id_current))
        && base_making(base.production_id_last, base_id)
            != base_making(production_id, base_id)
        && base.minerals_accumulated_2 > static_cast<int>(Rules.retool_exemption)) {
        const int exempt = static_cast<int>(Rules.retool_exemption);
        const int penalty = static_cast<int>(Rules.retool_pct_pen_prod_chg);
        loss = base.minerals_accumulated_2
             - (base.minerals_accumulated_2 - exempt) * (100 - penalty) / 100
             - exempt;
    }
    base.minerals_accumulated = base.minerals_accumulated_2 - loss;
    draw_radius(base.x, base.y, 2, 2);
    const int owner = base.faction_id_current;
    if (owner == ConsoleGlobal.faction_id_ || AiResearchBatchActive) return;
    const int next = base.queue_production_id[0];
    const bool new_unbuilt = next <= -70 && base_project(-next - 70) == -1;
    bool old_relevant = false;
    if (previous <= -70 && (new_unbuilt || base_project(-previous - 70) == -1)) {
        old_relevant = true;
        const int project_base = base_project(-previous - 70);
        if (project_base >= 0 && Bases[project_base].faction_id_current == owner)
            old_relevant = false;
    }
    if ((!new_unbuilt && !old_relevant) || ((GameDataStateStorage.fields_.game_state_ & 1) && !(GameDataStateStorage.fields_.game_state_ & 8))) return;
    const int facility = abs(next);
    const int project = facility - 70;
    int word, mask;
    bitmask(new_unbuilt ? project : abs(previous) - 70, &word, &mask);
    int old_count = 0, new_count = 0;
    int other;
    for (other = 0; other < GameDataStateStorage.fields_.base_current_count_; ++other) {
        if (Bases[other].faction_id_current == base.faction_id_current && other != base_id) {
            if (Bases[other].queue_production_id[0] == previous) ++old_count;
            if (Bases[other].queue_production_id[0] == base.queue_production_id[0]) ++new_count;
        }
    }
    if (!new_unbuilt || new_count) return;
    Player &player = Players[owner];
    if (old_relevant && (!old_count || base_project(-previous - 70) != -1)) {
        for (other = 0; other < GameDataStateStorage.fields_.base_current_count_; ++other)
            if (Bases[other].faction_id_current == owner
                && Bases[other].queue_production_id[0] == previous)
                Bases[other].queue_production_id[0] = base.queue_production_id[0];
        if (GameDataStateStorage.fields_.sunspot_duration_ > 0) return;
        parse_says(0, player.adj_name_faction, -1, -1);
        // RulesFacility's existing name field still carries the original
        // StringTable index with a legacy pointer spelling, as in base_reset.
        parse_say(1, reinterpret_cast<int>(Facility[abs(previous)].name), -1, -1);
        GenderDefault = player.noun_gender;
        PluralityDefault = player.is_noun_plural;
        parse_says(2, player.noun_faction, -1, -1);
        parse_say(3, reinterpret_cast<int>(Facility[facility].name), -1, -1);
        popp(ScriptPopupFile, "CHANGEPROJECT", 0, "secproj_sm.pcx", 0);
        faction.secret_project_intel[word] |= mask;
        bitmask(abs(previous) - 70, &word, &mask);
        faction.secret_project_intel[word] &= ~mask;
        return;
    }
    if (!(faction.secret_project_intel[word] & mask)) {
        if (GameDataStateStorage.fields_.sunspot_duration_ > 0 && project != 31) return;
        GenderDefault = player.is_leader_female;
        PluralityDefault = 0;
        parse_says(0, player.title_leader, -1, -1);
        GenderDefault = player.is_leader_female;
        PluralityDefault = 0;
        parse_says(1, player.name_leader, -1, -1);
        GenderDefault = player.noun_gender;
        PluralityDefault = player.is_noun_plural;
        parse_says(2, player.noun_faction, -1, -1);
        parse_say(3, reinterpret_cast<int>(Facility[facility].name), -1, -1);
        wave_it((PlayersData[ConsoleGlobal.faction_id_].diplo_treaties[owner] & 1) ? 36 : 38);
        popp(ScriptPopupFile, "BEGINPROJECT", 0, "secproj_sm.pcx", 0);
        faction.secret_project_intel[word] |= mask;
    }
    if (project == 31) {
        interlude(11, 0, 1, 0);
        GameDataStateStorage.fields_.game_interlude_flags_ |= 0x400;
    }
}

// ORIGINAL: 0x004E5FE0 ?base_reset@@YAXHH@Z 0x004E5FE0-0x004E62FB
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004e5fe0.cpp", "source:src/base.h", "source:src/base.cpp:BaseSiteClaimX/BaseSiteClaimY are the four 0x0090EA44/54 site-output arrays", "source:src/base.h:Base.location is the 0x130 field read as 0x0097D090 + base_id*0x134", "pinned image:0x00689484 contains secproj_sm.pcx;0x00689494 contains GOVPROJECT"], "reason": "Recover the base reset/site selection operation using the typed Base and faction state, four source-owned site claims, and source-owned popup text. Preserve the existing seed and mismatch obligation; base_build/base_change remain explicit link-only dependencies."}
void __cdecl base_reset(int base_id, int force) {
    const uint8_t faction = Bases[base_id].faction_id_current;
    const uint8_t faction_bit = static_cast<uint8_t>(1u << faction);
    const uint8_t active_mask = static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_);
    uint32_t state = Bases[base_id].state;

    if ((faction_bit & active_mask) != 0
        && (state & BSTATE_UNK_8000000000) != 0 && force != 0) {
        int loop_count = 0;
        for (int site = 0; site < 4; ++site) {
            BaseSiteClaimX[site] = 0;
            BaseSiteClaimY[site] = 999999;
        }

        if ((state & 0x0F000000u) != 0) {
            for (;;) {
                Bases[base_id].state = (Bases[base_id].state & 0xF1FFFFFFu)
                    | BSTATE_UNK_1000000;
                BaseSiteClaimX[0] = base_build(base_id, 0, 0, &BaseSiteClaimY[0]);
                Bases[base_id].state = (Bases[base_id].state & 0xF2FFFFFFu)
                    | BSTATE_UNK_2000000;
                BaseSiteClaimX[1] = base_build(base_id, 0, 0, &BaseSiteClaimY[1]);
                if (BaseSiteClaimX[0] == BaseSiteClaimX[1]) {
                    ++loop_count;
                    if (loop_count < 8) continue;
                }

                Bases[base_id].state = (Bases[base_id].state & 0xF4FFFFFFu)
                    | BSTATE_UNK_4000000;
                BaseSiteClaimX[2] = base_build(base_id, 0, 0, &BaseSiteClaimY[2]);
                if ((BaseSiteClaimX[0] != BaseSiteClaimX[2]
                     && BaseSiteClaimX[1] != BaseSiteClaimX[2])
                    || (++loop_count, loop_count > 7)) {
                    Bases[base_id].state = (Bases[base_id].state & 0xF8FFFFFFu)
                        | BSTATE_UNK_8000000;
                    BaseSiteClaimX[3] = base_build(base_id, 0, 0, &BaseSiteClaimY[3]);
                    if ((BaseSiteClaimX[0] != BaseSiteClaimX[3]
                         && BaseSiteClaimX[1] != BaseSiteClaimX[3]
                         && BaseSiteClaimX[2] != BaseSiteClaimX[3])
                        || (++loop_count, loop_count > 7)) {
                        Bases[base_id].state = state;
                        break;
                    }
                }
            }
        }

        const int production = base_build(base_id, 0, 0, &loop_count);
        base_change(base_id, production);
        return;
    }

    const int location = Bases[base_id].location;
    int production = base_build(base_id, 0, 0, 0);
    base_change(base_id, production);

    const uint8_t current_active_mask = static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_);
    if ((faction_bit & current_active_mask) == 0) return;
    if ((Bases[base_id].state & BSTATE_UNK_8000000000) == 0) return;
    if (location <= -0x46) return;
    if (Bases[base_id].location > -0x46) return;
    if (IsMultiplayerNet != 0) return;
    if ((GameDataStateStorage.fields_.game_state_ & STATE_UNK_1) != 0) return;

    const int absolute_location = location < 0 ? -location : location;
    const char *const base_name = Bases[base_id].name_string;
    int say_result;
    for (;;) {
        parse_says(0, base_name, -1, -1);
        say_result = parse_say(1, static_cast<int>(reinterpret_cast<intptr_t>(
                                   Facility[absolute_location].name)), -1, -1);
        // These two original .data slots contain these NUL-terminated strings
        // in the pinned PE; use application-owned literals in the rebuilt tree.
        if (popp(reinterpret_cast<LPCSTR>(ClimatePopupTitle[0]), "GOVPROJECT", 0,
                 "secproj_sm.pcx", 0) != 3) {
            break;
        }
        help_topic(0x0B, absolute_location);
    }

    if (say_result == 0) return;

    production = base_build(-1 - base_id, 0, 0, 0);
    base_change(base_id, production);
    if (say_result <= 1) return;

    BaseCurrent->governor &= ~0x00040000u;
    PlayersData[faction].flags &= ~0x00040000u;
}

// The cached path has no result contract: its EAX is the selected id, while
// recomputation leaves the last resource pass's register residue. Callers use
// the published base fields, not a returned integer.
// ORIGINAL: 0x004EC3B0 ?base_compute@@YAHH@Z 0x004EC3B0-0x004EC3E7 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x004EC3B0-0x004EC3E7"], "reason": "Recover the original base-selection/cache operation with source-owned state and typed fields. The archived seed remains an obligation until this product definition is freshly remeasured."}
// symbol    ?base_compute@@YAXH@Z
void __cdecl base_compute(int force) {
    if (force || BaseComputedID != BaseIDCurrentSelected) {
        BaseComputedID = BaseIDCurrentSelected;
        base_support();
        base_yield();
        base_nutrient();
        base_minerals();
        base_energy();
    }
}
/*
 * THE PER-FACTION PROTOTYPE-STATUS BYTE RUN, as the image addresses it:
 * PlayersData (0x0096C9E0) + 0x858, indexed faction * 0x20CC + prototype id.
 * veh.cpp names the same address FactionProtoSeenCount (it decrements it when
 * a vehicle of that prototype dies); upgrade_prototypes reads it as the gate
 * on whether the faction may still upgrade that prototype at all, which is
 * the same fact read from the other end. Both are internal-linkage
 * definitions of one address because base.h's PlayerData puts its
 * proto_id_active / proto_id_queue arrays at different offsets from this (the
 * struct's middle and tail are where the image's own layout disagrees), so
 * neither file can reach the byte through a field name.
 */
uint8_t *const FactionProtoSeenCount = (uint8_t *)0x0096D238;

/*
 * "Don't ask again" latch for the prototype-upgrade confirmation. The automatic
 * upgrade sweep (0x004F0650) clears it, upgrade_prototype (0x004EFE80) shows
 * the confirm popup only while it is zero and sets it when the caller answers
 * "yes to all" (popup result > 1); 0x00583E30, 0x005BB000, 0x005290A0 and
 * 0x00438460 clear it at the top of their own upgrade passes.
 */
uint32_t UpgradePromptAnswered;  // 0x0090EA3C

int BaseCurrentGrowthRate;  // 0x0090E918
// RECOVERY_OBJECT: v2 0x0090E980 0x0090E984 ?BaseCurrentVehPacifismCount@@3IA 0x004E9550
uint32_t BaseCurrentVehPacifismCount;  // 0x0090E980
uint32_t BaseCurrentForcesSupported;  // 0x0090E8FC
uint32_t BaseCurrentForcesMaintCost;  // 0x0090E91C

/*
 * How many of the base's supported units are past its free allowance, counted
 * alongside BaseCurrentForcesMaintCost. base_support() (0x004E9550) is the only
 * function in the executable that touches this address at all - it zeroes it,
 * reads it back and stores the increment, and nothing ever reads the total - so
 * the count is written and then dropped. Kept because it is observable state.
 */
uint32_t BaseCurrentForcesMaintCount;  // 0x0090EA08

/*
 * Out-parameter of the three terrain yield functions: how much of a resource
 * the square lost to the "three per square needs a technology" restriction, and
 * to mine_yield's roadless-mine mineral limit. Callers zero it, ask for a
 * yield, then read it back - BaseWin::draw_farm (0x0040F0F0) does exactly that
 * for each of the three resources, and resource_yield zeroes it for RSC_PSI.
 * Purpose is observed from those call sites; the executable does not name it.
 */
int TileYieldRestricted;  // 0x0090E998

// farm_compute publishes 25 status words, including four excluded outer
// positions. The separate mask covers only the first 21 workable positions.
// Both objects are initially zero in the image; their producer, not that
// initial state, determines availability for the current base.
// Status bits: 1 excluded outer position, 2 off-map/unrevealed, 4 base tile,
// 8 stack-check or visible non-allied vehicle, 0x10 another base's worked
// square, 0x20 the foreign-territory rule below. These describe the observed
// tests, not recovered historical names. The recovered base_yield reads the
// mask; the farm_click/draw_farm word consumers remain unfinished obligations.
// RECOVERY_OBJECT: v2 0x0090E99C 0x0090EA00 ?FarmStatus@@3PAIA 0x004E68B0
uint32_t FarmStatus[25];
// RECOVERY_OBJECT: v2 0x0090EA00 0x0090EA04 ?FarmUnavailableMask@@3IA 0x004E68B0
uint32_t FarmUnavailableMask;

// Requires the original game's valid faction, base, tile and vehicle indices
// and well-formed vehicle chains. Keep the entry coordinates/faction separate
// from the later BaseCurrent reload used to select nearby competing bases.
// ORIGINAL: 0x004E68B0 ?farm_compute@@YAXXZ 0x004E68B0-0x004E6E4E
// RECOVERY_BINDING: v1 {"evidence": ["4E68B6 loads BaseCurrent pointer before signedshort x/y and unsignedbyte faction.4E68E3/4E6BA5 establish25 DWORD outputs, with final4 set1;4E6E23..43 reduce onlyfirst21 into90EA00.4E6C7A tests Base+78 worked mask, not seed+3C.", "4E6966/4E69C4/4E6B99 reload row stride after calls; map-pointer loads4E697C/4E6A0A/4E6A46 must remain live.4E6B38 exhausts vehicle iterator before territory check4E6B7C.4E6BE7 reloads BaseCurrent for neighbor admission, while inner radius matching retains entry coordinates."], "reason": "This no-argument resource pass computes shared per-radius availability from the selected Base, typed map/vehicle/diplomacy state and neighboring worked squares; it owns25 status words and a separate21-position mask."}
void __cdecl farm_compute() {
    Base *const base = BaseCurrent;
    const int center_x = base->x;
    const int center_y = base->y;
    const int faction = base->faction_id_current;
    int row_stride = static_cast<int>(MapLongitude);
    int position;

    for (position = 0; position < 25; ++position) {
        if (position >= 21) {
            FarmStatus[position] = 1;
            continue;
        }
        const int x = xrange(center_x + RadiusOffsetX[position]);
        const int y = center_y + RadiusOffsetY[position];
        if (!on_map(x, y)) {
            FarmStatus[position] = 2;
            continue;
        }

        if (BaseUpkeepStage == 1) {
            const int goody = goody_at(x, y);
            row_stride = static_cast<int>(MapLongitude);
            if (goody != 0
                && (map_tiles()[row_stride * y + (x >> 1)].climate & 0xE0) >= 0x60) {
                add_goal(faction, 3, 1, x, y, -1);
                add_goal(faction, 2, 1, x, y, -1);
                add_goal(faction, 0, 1, x, y, -1);
                row_stride = static_cast<int>(MapLongitude);
            }
        }
        if ((PlayersData[faction].flags & 0x200) == 0
            && (map_tiles()[row_stride * y + (x >> 1)].visibility
                & (1u << faction)) == 0) {
            FarmStatus[position] = 2;
            continue;
        }

        FarmStatus[position] = 0;
        Map *const tile = &map_tiles()[row_stride * y + (x >> 1)];
        if ((tile->bit & 1) != 0 && (tile->val2 & 0xF) < 8) {
            FarmStatus[position] = 4;
        }
        int vehicle = stack_fix(veh_at(x, y));
        if (vehicle >= 0) {
            if (stack_check(vehicle, 9, 3, -1, -1) != 0) {
                FarmStatus[position] |= 8;
            }
            while (Vehs[vehicle].prev_veh_id_stack >= 0) {
                vehicle = Vehs[vehicle].prev_veh_id_stack;
            }
            for (; vehicle >= 0; vehicle = Vehs[vehicle].next_veh_id_stack) {
                const int other_faction = Vehs[vehicle].faction_id;
                if (other_faction != faction
                    && (Vehs[vehicle].visibility & (1u << faction)) != 0
                    && (PlayersData[faction].diplo_treaties[other_faction] & 3) == 0) {
                    FarmStatus[position] |= 8;
                }
            }
        }
        const int territory = whose_territory(faction, x, y, nullptr, 0);
        // The original uses the exhausted iterator here, not the saved stack
        // head. Do not turn this into a new friendly-occupant exemption.
        if (territory >= 0 && territory != faction
            && ((PlayersData[faction].diplo_treaties[territory] & 3) != 0
                || vehicle < 0 || Vehs[vehicle].faction_id != faction)) {
            FarmStatus[position] |= 0x20;
        }
        row_stride = static_cast<int>(MapLongitude);
    }

    for (int other_id = 0; other_id < GameDataStateStorage.fields_.base_current_count_; ++other_id) {
        if (other_id == BaseIDCurrentSelected) {
            continue;
        }
        Base *const other = &Bases[other_id];
        Base *const current = BaseCurrent;
        if (vector_dist(x_dist(current->x, other->x),
                        abs(current->y - other->y)) > 5) {
            continue;
        }
        int map_width = MapLongitudeBounds;
        for (int worked = 0; worked < 21; ++worked) {
            if (worked != 0 && (other->resource_sqr_radius & (1u << worked)) == 0) {
                continue;
            }
            int x = other->x + RadiusOffsetX[worked];
            if ((MapIsFlat & 1) == 0) {
                if (x < 0) {
                    x += map_width;
                } else if (x >= map_width) {
                    x -= map_width;
                }
            }
            const int y = other->y + RadiusOffsetY[worked];
            if (y < 0 || y >= MapLatitudeBounds || x < 0 || x >= map_width) {
                continue;
            }
            if (other->faction_id_current != faction
                && (map_tiles()[MapLongitude * y + (x >> 1)].climate & 0xE0) >= 0x60) {
                continue;
            }
            const int distance = vector_dist(x_dist(center_x, x), abs(center_y - y));
            map_width = MapLongitudeBounds;
            if (distance > 2) {
                continue;
            }
            const int flat = MapIsFlat & 1;
            for (position = 0; position < 21; ++position) {
                int candidate_x = center_x + RadiusOffsetX[position];
                if (!flat) {
                    if (candidate_x < 0) {
                        candidate_x += map_width;
                    } else if (candidate_x >= map_width) {
                        candidate_x -= map_width;
                    }
                }
                if (candidate_x == x && center_y + RadiusOffsetY[position] == y) {
                    FarmStatus[position] |= 0x10;
                }
            }
        }
    }

    FarmUnavailableMask = 0;
    uint32_t unavailable = 0;
    for (position = 0; position < 21; ++position) {
        if (FarmStatus[position] != 0) {
            unavailable |= 1u << position;
        }
    }
    FarmUnavailableMask = unavailable;
}

/*
 * general.h's swap() is `MEASURED inline`, so VC6 folds it into every caller
 * in this file - and base_rank (0x004EB490) is a site where the image makes
 * two real `call 0x628a50`s instead. Same lever as general.h's bitmask_call:
 * an ordinary out-of-line forwarder the inliner has no definition to fold.
 */
static void __cdecl swap_call(int *var1, int *var2) { swap(var1, var2); }

/*
 * energy_yield publishes the clamped energy of a square that contains a base
 * here. Its only other reader in the image is base_terraform (0x004ECCBD).
 */
int BaseSquareEnergy;  // 0x0090E914

/*
 * The faction elected Planetary Governor: written by CouncWin::make_resolution
 * (0x004246E5), persisted by game_data (0x005A6B61), read by call_council,
 * wants_prop, num_objectives and the council windows. energy_yield grants it a
 * point of energy on a base square.
 *
 * spying.cpp's former SpyingObserverFaction view named this same datum:
 * the Governor may view other non-alien factions' bases. Both Council writes
 * and visibility checks must use this one source object, not independent
 * integers justified by the retired separate-suite build.
 */
// Historical object identity moved to GameDataStateStorage; see the transport manifest.
// 0x009A6614
// Former independent scalar views CommerceDoubleTarget (009A6624) and
// CommerceDoubleProgress (009A6628) actually denote CouncilResolutionCount[3]
// and [4]. council_action increments those proposal counters and setup_game
// resets the whole array. base_energy doubles commerce while enacted trade
// pacts outnumber repeals; separate scalar storage would lose that relationship.

/*
 * A two-turn state advanced by random_events (0x0051F2F4 steps it 1 -> 2 -> 0
 * at turn start and arms it at 0x00520529). While the selected bit is set,
 * energy_yield triples its result.
 */
// 0x009A6818

/*
 * Selects which bit of GlobalEnergyEventState energy_yield consults. Read by
 * Console::on_key_click, faction_upkeep, NetDaemon::lock_base, wait_loop,
 * check_spock, base_yield and base_doctors; sits immediately below
 * ConsoleControlTurnActive (0x0093A938) and near IsMultiplayerPBEM (0x0093A95C).
 * TODO: identify global - its writer was not located, so it is not named.
 */
int UnkGlobal0093A934;  // 0x0093A934

/*
Purpose: Check if the base already has a particular facility built or if it's in the queue.
// ORIGINAL: 0x00421670 ?has_fac@@YA_NHHH@Z 0x00421670-0x004216E9 BYTE_EXACT
// LEVER: deleted the added `queue_count > 10` upper-bounds guard (see the BUG IN THE ORIGINAL comment at the site) - the image has no such check, only the for-loop's own entry test. Best similarity 0.755 -> 0.779 (2/54 agreeing either way). This is a behavioural change, not just a byte-match one: a caller passing queue_count > 10 now walks past the end of queue_production_id, matching the original's own defect.
// LEVER: swapped the inline `has_fac_built(facility_id, base_id)` call for
//   base.h's `has_fac_built_call` (real out-of-line bitmask_call, not the
//   folded-shift `MEASURED inline bitmask`), which the earlier TRIED note
//   on this marker had left unused - moved similarity 2/54 -> 17/54 by
//   itself. The rest closed by changing has_fac_built_call's RETURN TYPE
//   from `bool` to `int`: a `bool`-returning inline forces a `setne al` /
//   `and eax, 0xff` widen-to-BOOL at the call site, where the image's own
//   `(field & mask) != 0` compiles the `neg al; sbb eax, eax; neg eax`
//   idiom instead, so returning `int` (already 0/1 from the boolean
//   expression) drops the widen. 54/54 BYTE_EXACT once both landed.
// symbol    ?has_fac@@YAHHHH@Z
// size      121 bytes
// prototype bool (__cdecl ?has_fac@@YA_NHHH@Z)(int facilityID, int baseID, int queueCount)
// callers   14   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0050BA00
Return Value: Does base already have or planning on building facility? true/false
Status: Complete
*/
BOOL __cdecl has_fac(int facility_id, int base_id, int queue_count) {
    if (facility_id >= FacilityRepStart) {
        return false;
    }
    BOOL is_built = has_fac_built_call(facility_id, base_id);
    if (is_built || !queue_count) {
        return is_built;
    }
    // BUG IN THE ORIGINAL: no upper bound on queue_count. A caller passing
    // queue_count > 10 (the queue_production_id array's real length) walks
    // past the end of it. Reproduced deliberately - the image has no such
    // guard, only the for-loop's own entry test (which is why queue_count
    // <= 0 needed no separate check either: a negative count just never
    // enters the loop).
    for (int i = 0; i < queue_count; i++) {
        if (Bases[base_id].queue_production_id[i] == -facility_id) {
            return true;
        }
    }
    return false;
}

/*
Purpose: Set the current base globals.
// ORIGINAL: 0x004E39D0 ?set_base@@YAXH@Z 0x004E39D0-0x004E39F3 SEMANTIC
// TRIED: reordering the two stores (pointer before id) drops to 5/11;
//            a named local `Base *base = &Bases[base_id];` still 8/11, same
//            eax/ecx vs edx/ecx register choice on the last two lea/mov -
//            plateau, best flags /c /O2 /Gy /GR- /Oy- /GX.
// size      35 bytes
// prototype void (__cdecl ?set_base@@YAXH@Z)(int baseID)
// callers   40   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl set_base(int base_id) {
    BaseIDCurrentSelected = base_id;
    BaseCurrent = &Bases[base_id];
}


/*
Purpose: Get base name string from base id and store it in base_str. If base id is -1, use 'NONE'.
// ORIGINAL: 0x004E3A00 ?say_base@@YAXPADH@Z 0x004E3A00-0x004E3A4B
// size      75 bytes
// prototype void (__cdecl ?say_base@@YAXPADH@Z)(int8* strBase, int baseID)
// callers   13   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x006169A0 0x00645470
// LEVER: dropped std::string entirely for the image's two-branch form - a
//        real strcat() call in each branch rather than selecting a pointer
//        and calling once - and made label_get __forceinline (alpha.h) so it
//        folds to `StringTable.get(...)` the way the image does. Best
//        similarity 0.964 (16/28 agreeing, /c /O2 /Gy /GR- /Oy- /GX), up
//        from 2/28. Residual: the image keeps a full ebp frame here and
//        this tree's winning flag set omits it despite /Oy-, not chased
//        further.
Return Value: n/a
Status: Complete
*/
void __cdecl say_base(LPSTR base_str, int base_id) {
    if (base_id < 0) {
        strcat(base_str, label_get(25)); // 'NONE'
    } else {
        strcat(base_str, Bases[base_id].name_string);
    }
}

/*
Purpose: Return the base at the specified map coordinates and repair stale base map bits.
// ORIGINAL: 0x004E3A50 ?base_at@@YAHHH@Z 0x004E3A50-0x004E3B7F BYTE_EXACT
// size      303 bytes
// prototype int (__cdecl ?base_at@@YAHHH@Z)(int xCoord, int yCoord)
// callers   62   call targets   5
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005108A0 0x00532B70 0x005BF310 0x00625E30 0x006262F0
// indirect  0x004E3B59
// notes     Runtime redirect installed by DllMain after byte-signature validation
// LEVER: the missing debug path (STATE_DEBUG_MODE gate, reentrancy guard, BASEBIT
//        popup, GetAsyncKeyState(VK_CONTROL) poll, Console::focus) was the whole
//        call-count gap. Once restored, a pointer walk over Bases (lever 7) -
//        `Base *base = Bases; ...; base_id++, base++` instead of `&Bases[base_id]`
//        each iteration - closed the last two instructions.
Return Value: Base id or -1 when no base is present
Status: Complete - testing

The original does display the debug-only BASEBIT popup and focus the internal console when the map
bit has no matching base, gated by STATE_DEBUG_MODE and a reentrancy guard; that path is reproduced
below. BaseAtDebugPopupFlag and BaseAtKeyPollFlag gate a parse_num/X_pop popup and a
GetAsyncKeyState(VK_CONTROL) poll respectively - purpose inferred only from this one call site, the
executable does not name either.
*/
// Reentrancy guard for the debug block below: set while it runs, so a callee
// that re-enters base_at() (e.g. via rebuild_base_bits or Console::focus)
// while it is busy skips straight to returning -1.
int BaseAtErrorGuard;  // 0x0090EA38
int BaseAtDebugPopupFlag;  // 0x0093A94C
// Shared popup/action result lives in console.cpp as ConsoleGroupAction.

int __cdecl base_at(int x, int y) {
    if (!on_map(x, y) || !(bit_at(x, y) & BIT_BASE_IN_TILE)) {
        return -1;
    }
    Base *base = Bases;
    for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++, base++) {
        if (base->x == x && base->y == y) {
            return base_id;
        }
    }
    if (!BaseAtErrorGuard) {
        log_say("Base Bits Error  (x, y)", x, y, 0);
    }
    if (!(GameDataStateStorage.fields_.game_state_ & STATE_DEBUG_MODE)) {
        rebuild_base_bits();
        return -1;
    }
    if (BaseAtErrorGuard) {
        return -1;
    }
    BaseAtErrorGuard = 1;
    if (BaseAtDebugPopupFlag) {
        parse_num(0, x);
        parse_num(1, y);
        X_pop("BASEBIT", NULL);
    }
    BaseAtErrorGuard = 0;
    if (!ConsoleGroupAction) {
        GetAsyncKeyState(VK_CONTROL);
    }
    rebuild_base_bits();
    ConsoleGlobal.focus(x, y, ConsoleGlobal.faction_id_);
    return -1;
}

/*
Purpose: Find the base id closest to the specified coordinates.
// ORIGINAL: 0x004E3B80 ?base_find@@YAHHH@Z 0x004E3B80-0x004E3C5F
// size      223 bytes
// prototype int (__cdecl ?base_find@@YAHHH@Z)(int xCoord, int yCoord)
// callers   23   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00644F3A
// TRIED: vector_dist(x, y, Bases[i].x, Bases[i].y) - the image never calls
//            vector_dist, only abs() x4 (see del_site 0x00579E70 for the same
//            expansion); open-coding it moved the mismatch from the whole
//            function to a prologue scheduling difference (/O2 /Oi- gets the
//            first 9 instructions and the push/pop shape byte-exact) and one
//            residual +4 byte size difference in the loop body, not chased
//            further - del_site's own comment records the same plateau.
//            Indexing Bases[i] directly (not a walking Base* - lever 7)
//            scored higher here; the two functions disagree on that lever.
Return Value: Base id or -1 if not found
Status: Complete
*/
int __cdecl base_find(int x, int y) {
    int proximity = 9999;
    int base_id = -1;
    for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
        // Open-coded vector_dist(x, y, Bases[i].x, Bases[i].y): the image calls
        // abs() four times (never vector_dist itself) - see del_site
        // (0x00579E70) for the same expansion.
        int dx = x_dist(x, Bases[i].x);
        int dy = abs(y - Bases[i].y);
        int abs_dx = abs(dx);
        int abs_dy = abs(dy);
        int largest = abs_dx;
        if (abs_dx <= abs_dy) {
            largest = abs_dy;
        }
        int smallest = abs_dx;
        if (abs_dx >= abs_dy) {
            smallest = abs_dy;
        }
        int dist = largest - ((((abs_dy + abs_dx) >> 1) - smallest + 1) >> 1);
        if (dist <= proximity) {
            proximity = dist;
            base_id = i;
        }
    }
    if (base_id >= 0) {
        BaseFindDist = proximity;
    }
    return base_id;
}

/*
Purpose: Find the base id closest to the specified coordinates owned by faction.
// ORIGINAL: 0x004E3C60 ?base_find@@YAHHHH@Z 0x004E3C60-0x004E3D4B
// size      235 bytes
// prototype int (__cdecl ?base_find@@YAHHHH@Z)(int xCoord, int yCoord, int factionID)
// callers   27   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00644F3A
// LEVER: open-coded the vector_dist(x, y, Bases[i].x, Bases[i].y) call as the
//        x_dist()+abs()x4 expansion (same shape as the sibling base_find
//        overloads) - the image never calls vector_dist, only abs() x4. Best
//        similarity across flag sets moved 0.551 -> 0.920 (/c /O2 /Oi- /Gy
//        /GR- /Oy- /GX, 5/84 agreeing), though the tool's auto-picked flag
//        set only shows 5/84 vs the prior 4/84 - the similarity gain is the
//        real signal.
Return Value: Base id or -1 if not found
Status: Complete
*/
int __cdecl base_find(int x, int y, int faction_id) {
    if (GameDataStateStorage.fields_.base_current_count_ <= 0) {
        return -1;
    }
    int proximity = 9999;
    int base_id = -1;
    for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
        if (Bases[i].faction_id_current == faction_id) {
            // Open-coded vector_dist(x, y, Bases[i].x, Bases[i].y): the
            // image calls abs() four times, never vector_dist itself - see
            // del_site (0x00579E70) and base_find(x,y) (0x004E3B80) for the
            // same expansion.
            int dx = x_dist(x, Bases[i].x);
            int dy = abs(y - Bases[i].y);
            int abs_dx = abs(dx);
            int abs_dy = abs(dy);
            int largest = abs_dx;
            if (abs_dx <= abs_dy) {
                largest = abs_dy;
            }
            int smallest = abs_dx;
            if (abs_dx >= abs_dy) {
                smallest = abs_dy;
            }
            int dist = largest - ((((abs_dy + abs_dx) >> 1) - smallest + 1) >> 1);
            if (dist <= proximity) {
                proximity = dist;
                base_id = i;
            }
        }
    }
    if (base_id >= 0) {
        BaseFindDist = proximity;
    }
    return base_id;
}

/*
Purpose: Find the base id closest to the specified coordinates meeting various conditional checks.
// ORIGINAL: 0x004E3D50 ?base_find@@YAHHHHHHH@Z 0x004E3D50-0x004E3EEB
// TRIED: region_at(base->x, base->y) - the image widens base->y (int16_t)
//   to int with `movsx` BEFORE the `sar edx, 1` inside map_loc's `x >> 1`;
//   this tree's compiled body does the shift in 16-bit (`sar dx, 1`) and
//   widens only afterward. Forcing `region_at((int)base->x, (int)base->y)`
//   made no difference at all to the emitted bytes (same divergence at the
//   same instruction, every flag set) - this is the same register-
//   allocation plateau already documented across region_at/alt_at/is_ocean
//   and their neighbours (see AGENT_BRIEF's "PICKING A BATCH" note), not
//   something reachable from this call site's source shape.
// size      411 bytes
// prototype int (__cdecl ?base_find@@YAHHHHHHH@Z)(int xCoord, int yCoord, int factionID, int region, int factionID2, int factionID3)
// callers   14   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00644F3A
// LEVER: same vector_dist -> abs()x4 open-coding as base_find(x,y) 0x004E3B80.
//        A `Base *base = Bases; ...base++` walk (lever 7) reused across the
//        whole loop body (matching ebx-relative offsets in the disasm) ties
//        with plain `Bases[i]` indexing here - no measured difference either
//        way, kept for the offset match to the disassembly's ebx addressing.
//        /O2 /Oi- gets the first 11 instructions and the push order
//        byte-exact; the rest is a 1-byte body-size difference cascading
//        through every later jump target, not chased further given size.
Return Value: Base id or -1 if not found
Status: Complete
*/
int __cdecl base_find(int x, int y, int faction_id, int region, int faction_id_2,
                      int faction_id_3) {
    int proximity = 9999;
    int base_id = -1;
    BaseFindDist = 9999; // difference from other two functions where this is reset at start
    Base *base = Bases;
    for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++, base++) {
        if (region < 0 || region_at(base->x, base->y) == (uint32_t)region) {
            if (faction_id < 0 ? (faction_id_2 < 0 || base->faction_id_current != faction_id_2)
                : (faction_id == base->faction_id_current || (faction_id_2 == -2
                    ? has_treaty(faction_id, base->faction_id_current, DTREATY_PACT)
                    : (faction_id_2 >= 0 && faction_id_2 == base->faction_id_current)))) {
                if (faction_id_3 < 0 || base->faction_id_current == faction_id_3
                    || ((1 << faction_id_3) & base->visibility)) {
                    // Open-coded vector_dist(x, y, base->x, base->y): the
                    // image calls abs() four times, never vector_dist itself -
                    // see del_site (0x00579E70) and base_find(x,y) (0x004E3B80)
                    // for the same expansion.
                    int dx = x_dist(x, base->x);
                    int dy = abs(y - base->y);
                    int abs_dx = abs(dx);
                    int abs_dy = abs(dy);
                    int largest = abs_dx;
                    if (abs_dx <= abs_dy) {
                        largest = abs_dy;
                    }
                    int smallest = abs_dx;
                    if (abs_dx >= abs_dy) {
                        smallest = abs_dy;
                    }
                    int dist = largest - ((((abs_dy + abs_dx) >> 1) - smallest + 1) >> 1);
                    if (dist <= proximity) {
                        proximity = dist;
                        base_id = i;
                    }
                }
            }
        }
    }
    if (base_id >= 0) {
        BaseFindDist = proximity;
    }
    return base_id;
}

/*
Purpose: Find the best specialist available to the current base with more weight placed on psych.
// ORIGINAL: 0x004E4020 ?best_specialist@@YAHXZ 0x004E4020-0x004E4090 BYTE_EXACT
// LEVER: two fixes. (1) has_tech(...) was inlining its whole preq_tech walk
//   here where the image keeps a real `call 0x5b9f20` - added
//   technology.h/.cpp's `has_tech_call` non-inline forwarder (same idiom as
//   general.h's bitmask_call) and called that instead, which alone moved
//   0/44 -> ~40/44 agreeing. (2) the remaining gap was BaseCurrent()'s two
//   textual reads: the image reads the global ONCE before the loop (priming
//   iteration 1) then reloads it UNCONDITIONALLY right after the has_tech
//   call - even on a false return - which feeds both the current
//   iteration's population_size check AND the next iteration's top-of-loop
//   read, eliminating what would otherwise be a second load. Hoisting
//   `Base *base_current = BaseCurrent();` before the loop and reassigning it
//   right after `has_tech_call` (not inside the `if`, since the image
//   reloads even when the call returns false) reproduces that exactly.
//   44/44 BYTE_EXACT.
// symbol    ?best_specialist@@YAIXZ
// size      112 bytes
// prototype
// callers   9   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005B9F20
Return Value: Best citizen id (always going to be 1, 4, or 6 based on default weights)
Status: Complete
*/
uint32_t  __cdecl best_specialist() {
    int current_bonus = -999;
    uint32_t citizen_id = 0;
    Base *base_current = BaseCurrent;
    for (int i = 0; i < MaxSpecialistNum; i++) {
        BOOL known = has_tech_call(Citizen[i].preq_tech, base_current->faction_id_current);
        base_current = BaseCurrent;
        if (known) {
            uint32_t bonus = Citizen[i].psych_bonus * 3;
            if (base_current->population_size >= (int)Rules.min_base_size_specialists) {
                bonus += Citizen[i].ops_bonus + Citizen[i].research_bonus;
            }
            if ((int)bonus > current_bonus) {
                current_bonus = bonus;
                citizen_id = i;
            }
        }
    }
    return citizen_id;
}

/*
Purpose: Generate a base name. Added some minor randomization for sea bases and ensuring offset gets
         increased for default base name. TODO: Eventually improve base name shuffling.
// ORIGINAL: 0x004E4090 ?name_base@@YAXHPADHH@Z 0x004E4090-0x004E4350
// size      704 bytes
// prototype void (__cdecl ?name_base@@YAXHPADHH@Z)(int factionID, LPSTR nameOut, BOOL isFinal, BOOL isSeaBase)
// callers   3   call targets   9
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005FD530 0x005FD550 0x005FD570 0x005FD670 0x006453E0 0x00645470 0x00645E90 0x0064FC88 0x0064FDF0
// LEVER: replaced sprintf_s(name_out, 25, "%s %d", get_noun(faction_id), offset)
//        with the image's strcat/_itoa idiom - name_out[0]=0, strcat the
//        noun, strcat " ", _itoa the offset into a stack buffer, strcat that
//        - and swapped both strncpy_s(name_out, 25, text_item(), 24) calls
//        (water-base and land-base copy paths) for plain 3-argument
//        strncpy(0x00645E90) + name_out[23]=0, and made get_noun
//        `MEASURED inline` at the end of faction.h (it has its own ORIGINAL
//        marker at faction.cpp:120) so it folds to parse_set()+field read in
//        place. Best similarity moved 0.110 -> 0.183 - still far from a
//        match; this function's #END-scan/PRNG-reseed loops account for
//        most of the remaining 236 instructions and were not touched.
Return Value: n/a
Status: Complete
*/
void __cdecl name_base(int faction_id, LPSTR name_out, BOOL is_final, BOOL is_sea_base) {
    if (is_sea_base && !text_open(Players[faction_id].filename, "WATERBASES")) {
        uint32_t offset_sea = PlayersData[faction_id].base_sea_name_offset + 1;
        if (offset_sea > 1) {
            uint32_t total;
            for (total = 0; ; total++) {
                text_get();
                if (!strlen(TextBufferGetPtr) || !_strnicmp(TextBufferGetPtr, "#END", 4)) {
                    break;
                }
            }
            if (offset_sea <= total) {
                int seed = ((MapRandSeed + faction_id) & 0xFE) | 1;
                uint32_t loop = 1;
                do {
                    if (seed & 1) {
                        seed ^= 0x170;
                    }
                    seed >>= 1;
                } while (seed >= (int)total || ++loop != offset_sea);
                offset_sea = seed + 1;
            }
        }
        if (!text_open(Players[faction_id].filename, "WATERBASES")) {
            uint32_t count;
            for (count = 0; count < offset_sea; count++) {
                text_get();
                if (!strlen(TextBufferGetPtr) || !_strnicmp(TextBufferGetPtr, "#END", 4)) {
                    break;
                }
            }
            if (count == offset_sea) {
                // water base name available
                strncpy(name_out, text_item(), 24);
                name_out[23] = 0;
                if (is_final) {
                    PlayersData[faction_id].base_sea_name_offset++;
                }
                text_close();
                return;
            }
        }
        text_close();
    }
    // Land base names or generic
    uint32_t offset = PlayersData[faction_id].base_name_offset + 1;
    // default if names exhausted
    name_out[0] = 0;
    strcat(name_out, get_noun(faction_id));
    strcat(name_out, " ");
    char digits[0x54];
    _itoa(offset, digits, 10);
    strcat(name_out, digits);
    if (is_final) {
        PlayersData[faction_id].base_name_offset++;
    }
    if (!text_open(Players[faction_id].filename, "BASES")) {
        if (offset > 1) {
            uint32_t total;
            for (total = 0; ; total++) {
                text_get();
                if (!strlen(TextBufferGetPtr) || !_strnicmp(TextBufferGetPtr, "#END", 4)) {
                    break;
                }
            }
            if (offset <= total) {
                int seed = ((MapRandSeed + faction_id) & 0xFE) | 1;
                uint32_t loop = 1;
                do {
                    if (seed & 1) {
                        seed ^= 0x170;
                    }
                    seed >>= 1;
                } while (seed >= (int)total || ++loop != offset);
                offset = seed + 1;
            }
        }
        if (!text_open(Players[faction_id].filename, "BASES")) {
            uint32_t count;
            for (count = 0; count < offset; count++) {
                text_get();
                if (!strlen(TextBufferGetPtr) || !_strnicmp(TextBufferGetPtr, "#END", 4)) {
                    break;
                }
            }
            if (count != offset) {
                if (text_open("BASENAME", "GENERIC")) {
                    return;
                }
                while (count < offset) {
                    text_get();
                    if (!strlen(TextBufferGetPtr) || !_strnicmp(TextBufferGetPtr, "#END", 4)) {
                        return;
                    }
                    count++;
                }
            }
            strncpy(name_out, text_item(), 24);
            name_out[23] = 0;
        }
        text_close();
    }
}

/*
Purpose: TBD
// ORIGINAL: 0x004E4350 ?base_mark@@YAXH@Z 0x004E4350-0x004E442D
// size      221 bytes
// prototype void (__cdecl ?base_mark@@YAXH@Z)(int baseID)
// callers   2   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00579E70 0x00591B50 0x00591C10 0x00591D60
// LEVER: two changes. (1) the BIT_BASE_RADIUS site now calls the new
//        `bit_set_call` non-inline forwarder (map.h/map.cpp) instead of the
//        `MEASURED inline bit_set` every other caller uses, matching the
//        image's real `call 0x591D60` here. (2) the radius loop bound is
//        the literal `49` (image `cmp ebx, 0xc4`, a byte-offset stride)
//        instead of `RadiusRange[3]`, which the compiler had to reload from
//        the global every iteration and which blocked the walking-byte-
//        offset strength reduction. Best similarity 0.662 -> 0.805; agreeing
//        count unchanged at 2/81 but compiled size moved from 96 to 83
//        instructions against the image's 81.
Return Value: n/a
Status: Complete - testing
*/
void __cdecl base_mark(int base_id) {
    int x = Bases[base_id].x;
    int y = Bases[base_id].y;
    int x_radius = 0;
    int y_radius = 0;
    uint32_t faction_id = Bases[base_id].faction_id_current;
    for (int i = 0; i < 49; i++) {
        x_radius = xrange(x + RadiusOffsetX[i]);
        y_radius = y + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius)) {
            if (i < 21) {
                bit_set_call(x_radius, y_radius, BIT_BASE_RADIUS, true);
                using_set(x_radius, y_radius, faction_id); // rolled this into one conditional check
            } else {
                site_set(x_radius, y_radius, 0);
            }
        }
    }
    for (int f = 0; f < MaxPlayerNum; f++) {
        // TODO: verify if using last radius coordinates is best logic, seems odd
        // It would seem using the Base coordinates would be better suited versus radius
        del_site(f, AI_GOAL_COLONIZE, x_radius, y_radius, 3);
    }
}

// has_fac_built_call(), open-coded with a REAL bitmask() call via
// `bitmask_call` (general.h) rather than the folded shift/and `MEASURED
// inline bitmask` gives everywhere else, MOVED TO base.h (still `static
// __forceinline`, one copy per TU) so breed_mod there can reach it too -
// see the LEVER note at breed_mod's definition.

/*
Purpose: Calculate the cost factor for the specified faction and resource type. Optional base param.
// ORIGINAL: 0x004E4430 ?cost_factor@@YAHHHH@Z 0x004E4430-0x004E46C2
// TRIED: call count already agrees (call_diff: 0 disagree, both call
// bitmask and 0x539c00 through has_fac_built_call/great_satan), so the
// 9/251 MISMATCH is not a call-shape defect. The image's `is_human(faction_id)`
// ternary loads the bitmask table (`mov eax,[0x9a64e8]`) BEFORE the `push
// esi`/`push edi` register saves; this tree's /O2 schedules that load after
// them - pure instruction scheduling in the preamble. The larger gap is
// past that: image is 251 instructions against this tree's 159 for the
// same body, which the RSC_MINERALS `switch` on
// `soc_effect_pending.industry` (cases -7..4) likely accounts for most of -
// not chased further; a full match would need the same compare-chain-vs-
// jump-table investigation `base_making`'s note describes for its own
// `switch`, at a size (658 image bytes) outside this pass's budget.
// size      658 bytes
// prototype int (__cdecl ?cost_factor@@YAHHHH@Z)(uint32_t factionID, uint32_t rscType, int baseID)
// callers   24   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0050BA00 0x00539C00
Return Value: Cost factor
Status: Complete
*/
int __cdecl cost_factor(int faction_id, int rsc_type, int base_id) {
    static const int diff_cost_base[] = { 13, 12, 11, 10, 8, 7 };
    int factor = is_human(faction_id) ? 10 : diff_cost_base[GameDataStateStorage.fields_.diff_level_current_]
        - great_satan(GameDataStateStorage.fields_.faction_rankings_[7], false)
        - (!IsMultiplayerNet && is_human(GameDataStateStorage.fields_.faction_rankings_[7]));
    int cost_multiplier = rsc_type ? Rules.mineral_cost_multi : Rules.nutrient_cost_multi;
    if (cost_multiplier != 10) {
        factor = (factor * cost_multiplier) / 10;
    }
    if (MapSizePlanet == 1) {
        factor = (factor * 9) / 10;
    } else if (!MapSizePlanet) {
        factor = (factor * 8) / 10;
    }
    if (rsc_type) {
        if (rsc_type == RSC_MINERALS) {
            switch (PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::INDUSTRY]) {
              case -7:
              case -6:
              case -5:
              case -4:
              case -3:
                return (factor * 13 + 9) / 10;
              case -2:
                return (factor * 6 + 4) / 5;
              case -1:
                return (factor * 11 + 9) / 10;
              case 0:
                return factor;
              case 1:
                return (factor * 9 + 9) / 10;
              case 2:
                return (factor * 4 + 4) / 5;
              case 3:
                return (factor * 7 + 9) / 10;
              case 4:
                return (factor * 3 + 4) / 5;
              default:
                return (factor + 1) / 2;
            }
        } else {
            return factor; // Energy/PSI? Legacy code logic in case cost used these?
        }
    } else { // nutrient
        int growth_factor = PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::GROWTH];
        if (base_id >= 0) {
            if (has_fac_built_call(FAC_CHILDREN_CRECHE, base_id)) {
                growth_factor += 2; // +2 on growth scale
            }
            if (Bases[base_id].state & BSTATE_GOLDEN_AGE_ACTIVE) {
                growth_factor += 2;
            }
        }
        growth_factor = range(growth_factor, -2, 5);
        return (factor * (10 - growth_factor) + 9) / 10;
    }
}

/*
Purpose: Determine if the specified base has any restrictions around production item retooling.
// ORIGINAL: 0x004E4700 ?base_making@@YAHHH@Z 0x004E4700-0x004E4802
// size      258 bytes
// prototype int (__cdecl ?base_making@@YAHHH@Z)(int productionID, int baseID)
// callers   4   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0050BA00
// LEVER: `int retool` rather than `uint32_t retool` (lever 4) so `retool >=
//        1` compiles a signed compare like the image's, kept WITHOUT
//        touching the guard. Best similarity 0.546 -> 0.555.
// LEVER: WRONG CALLEE - the image's two calls are both to bitmask (0x50ba00,
//        push 0xa then a dynamic id), never to facility_offset or has_tech.
//        The RFLAG_FREEPROTO/facility_offset("Skunkworks")/has_tech disjunct
//        was this tree's own invention (facility_offset is a project helper,
//        "Original Offset: n/a") - the image's guard is plainly
//        `has_fac_built(FAC_SKUNKWORKS, base_id) && retool >= 1`, first push
//        is the literal 0xa (FAC_SKUNKWORKS). Dropping the disjunct AND
//        switching both has_fac_built() calls in this function to
//        has_fac_built_call() (general.h bitmask_call, forcing the real
//        E8 the image emits instead of the folded shift/and) moved best
//        similarity 0.555 -> 0.652, 30/120 agreeing. A prior pass dropped
//        the disjunct alone (without forcing the real bitmask call) and saw
//        0.549, WORSE - that was the wrong half of this fix, not proof the
//        disjunct belongs.
// TRIED: remaining divergence is the switch(retool) codegen - image
//            compiles it as a compare chain, this tree's /O2 default emits a
//            jump table (`jmp dword ptr [ebx*4]`), plus an omit-frame-pointer
//            stack layout (esp-relative) vs the image's ebp frame. Neither
//            responded to the flag sweep (--all-flags best is this same
//            0.652 set); left as source-shaped register/codegen noise.
Return Value: Fixed value (-1, 0, 1, 2, 3, -70) or productionID
Status: Complete
*/
int __cdecl base_making(int production_id, int base_id) {
    int retool = Rules.retool_strictness;
    if (has_fac_built_call(FAC_SKUNKWORKS, base_id) // has Skunkworks
        && retool >= 1) { // don't override if retool strictness is already set to always free (0)
        retool = 1; // Skunkworks or FREEPROTO + prerequisite tech > 'Free in Category'
    }
    if (production_id < 0) { // facility or SP to build
        int queue_id = Bases[base_id].queue_production_id[0]; // current production item
        if (queue_id < 0) { // non-Veh
            queue_id = -queue_id;
            if (queue_id < FacilityRepStart && has_fac_built_call(queue_id, base_id)) {
                return -1; // facility completed outside normal process, no retool penalty to change
            }
        }
    }
    switch (retool) { // converted into switch to improve readability and performance
      case 0: // Always Free
        return 0;
      case 1: // Free in Category
        if (production_id >= 0) {
            return 0; // Veh
        }
        // SP (1), repeatable facility (2), non-repeatable facility (3)
        return (production_id > -FacilitySPStart) ? (production_id > -FacilityRepStart) + 2 : 1;
      case 2: // Free switching between SPs (default behavior)
        return (production_id <= -FacilitySPStart) ? -FacilitySPStart : production_id;
      case 3: // Never Free
        return production_id;
      default:
        return 1; // should never be reached unless problem with retool value
    }
}

/*
Purpose: Calculate the mineral loss if production were to changed at the specified base.
// ORIGINAL: 0x004E4810 ?base_lose_minerals@@YAHHH@Z 0x004E4810-0x004E48B0 BYTE_EXACT
// size      160 bytes
// prototype int (__cdecl ?base_lose_minerals@@YAHHH@Z)(int baseID, int)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004E4700
// LEVER: signedness on the /100 divide. `retool_pct_pen_prod_chg` and
//        `retool_exemption` are declared uint32_t in alpha.h, so the return
//        expression was an unsigned division; casting both operands to
//        `(int)` restores the image's signed magic-multiply/sar sequence.
//        41/60 agreeing, up from 10/60.
Return Value: Minerals that would be lost or 0 if not applicable.
Status: Complete
*/
int __cdecl base_lose_minerals(int base_id, int UNUSED(production_id)) {
    int min_accum;
    if (Rules.retool_pct_pen_prod_chg && is_human(Bases[base_id].faction_id_current)
        && base_making(Bases[base_id].production_id_last, base_id)
        != base_making(Bases[base_id].queue_production_id[0], base_id)
        && (min_accum = Bases[base_id].minerals_accumulated_2,
            min_accum > (int)Rules.retool_exemption)) {
        return min_accum - (100 - (int)Rules.retool_pct_pen_prod_chg)
            * (min_accum - (int)Rules.retool_exemption) / 100 - (int)Rules.retool_exemption;
    }
    return 0;
}


// POP2 has an owning implementation in leaf_recoveries.cpp; the public
// xpops header currently omits its declaration.
int __cdecl POP2(const char *caption, const char *image, int base_id);

static int ecology_wrap_int(uint32_t bits) {
    if (bits <= 0x7FFFFFFFu) return static_cast<int>(bits);
    return -1 - static_cast<int>(~bits);
}

static int ecology_wrapped_add(int left, int right) {
    return ecology_wrap_int(static_cast<uint32_t>(left) + static_cast<uint32_t>(right));
}

static int ecology_wrapped_shift_two(int value) {
    return ecology_wrap_int(static_cast<uint32_t>(value) << 2);
}

static int ecology_wrapped_double(int value) {
    return ecology_wrap_int(static_cast<uint32_t>(value) * 2u);
}

static int ecology_wrapped_multiply(int left, int right) {
    return ecology_wrap_int(static_cast<uint32_t>(left) * static_cast<uint32_t>(right));
}

static int ecology_native_abs(int value) {
    return value < 0 ? ecology_wrap_int(0u - static_cast<uint32_t>(value)) : value;
}

static int ecology_wrapped_neg_sub(int value, int subtract) {
    return ecology_wrap_int(0u - static_cast<uint32_t>(value) - static_cast<uint32_t>(subtract));
}

// The superseded partial scaffold was deleted after full owner recovery.
// Its seed path below is historical Git provenance, not a source dependency.
// ORIGINAL: 0x004F67F0 ?base_ecology@@YAXXZ 0x004F67F0-0x004F79B3
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/004f67f0.cpp; recovery/tasks/base-colony-economy-coverage-wave.json; recovery/tests/test_base_colony_production.py", "Pinned image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf, full original span 4F67F0..4F79B3.", "Native 4F6C73 reads faction difficulty, not ranking; 4F739B reads population total and 4F73B9 independently reloads ecology.", "Native callbacks are followed by live ecology reloads at 4F6E94/4F6EC2 (including the current viewer), 4F7024, 4F70C7, 4F715B, 4F73B9, 4F74A2/4F7512, 4F75B8 and 4F76EE; signed 32-bit climate arithmetic retains native wrapping.", "Native reuses the spread cursor for the 19-row terraforming scan; preserve that original cursor behavior rather than silently changing the event."], "reason": "Recover the complete ecology event owner using real Base, faction, map, terraforming, Vehicle and climate storage. Retain original seed and obligations; this is no new byte or game acceptance claim."}
// symbol    ?base_ecology@@YAXXZ
// TRIED: heavily time-boxed given batch budget - only the leading probability gate (faction-table accumulator update, a rand()%100 roll against a base-stat threshold, and the two disabling-flag checks) is transcribed; the ecology event itself, most of the function, is a deliberate stub. MISMATCH #0, prologue only.
void __cdecl base_ecology() {
    Base *base = BaseCurrent;
    const int faction = base->faction_id_current;
    PlayerData &player = PlayersData[faction];

    int ecology_divisor = ecology_wrap_int(
        4u - static_cast<uint32_t>(player.soc_effect_pending.values_[SocialEffect::PLANET]));
    if (ecology_divisor < 1) ecology_divisor = 1;
    else if (ecology_divisor > 99) ecology_divisor = 99;
    player.unk_49 = ecology_wrapped_add(player.unk_49,
        ecology_wrapped_shift_two(base->eco_damage) / ecology_divisor);

    const int game_state = GameDataStateStorage.fields_.game_state_;
    const int damage_divisor = (game_state & 0x100000) != 0 ? 2 : 1;
    if (rand() % 100 >= base->eco_damage / damage_divisor
        || (GameDataStateStorage.fields_.game_rules_ & 0x200000) != 0
        || ((game_state & 1) != 0 && (game_state & 8) == 0)) {
        return;
    }

    int best_roll = 0;
    int best_radius_index = -1;
    const int base_x = base->x;
    const int base_y = base->y;
    // The native radius cursor starts at index 1 in the shared 289-entry
    // radius table and processes 1..48 inclusive.
    for (int search_radius_index = 1; search_radius_index <= 48; ++search_radius_index) {
        int x = base_x + RadiusOffsetX[search_radius_index];
        if ((MapIsFlat & 1) == 0) {
            if (x < 0) x += MapLongitudeBounds;
            else if (x >= MapLongitudeBounds) x -= MapLongitudeBounds;
        }
        const int y = base_y + RadiusOffsetY[search_radius_index];
        if (y < 0 || y >= MapLatitudeBounds || x < 0 || x >= MapLongitudeBounds) continue;

        Map *tile = map_loc(x, y);
        if (((tile->bit & BIT_FUNGUS) != 0 && (tile->climate & 0xE0) > 0x3F)
            || ((tile->bit & BIT_BASE_IN_TILE) != 0 && (tile->val2 & 0x0F) < 8)) {
            continue;
        }

        int probability = 0;
        if (search_radius_index > 24 && best_roll != 0) continue;
        bool nearest_base_is_current = false;
        if (search_radius_index > 24) {
            int nearest_id = -1;
            int nearest_distance = 9999;
            for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
                const Base &candidate = Bases[base_id];
                int dx = abs(x - candidate.x);
                if ((MapIsFlat & 1) == 0 && dx > static_cast<int>(MapLongitude))
                    dx = MapLongitudeBounds - dx;
                const int distance = vector_dist(dx, abs(y - candidate.y));
                if (distance <= nearest_distance) {
                    nearest_distance = distance;
                    nearest_id = base_id;
                }
            }
            if (nearest_id >= 0) BaseFindDist = nearest_distance;
            nearest_base_is_current = nearest_id == BaseIDCurrentSelected;
            if (nearest_base_is_current) probability = 1;
        }
        if (search_radius_index <= 24 || nearest_base_is_current) {
            for (int diagonal = 0; diagonal < 4; ++diagonal) {
                int nx = x + RiverStepX[diagonal];
                if ((MapIsFlat & 1) == 0) {
                    if (nx < 0) nx += MapLongitudeBounds;
                    else if (nx >= MapLongitudeBounds) nx -= MapLongitudeBounds;
                }
                const int ny = y + RiverStepY[diagonal];
                if (ny < 0 || ny >= MapLatitudeBounds || nx < 0 || nx >= MapLongitudeBounds)
                    continue;
                const Map *neighbor = map_loc(nx, ny);
                if ((neighbor->bit & BIT_FUNGUS) != 0
                    && (neighbor->climate & 0xE0) >= ALT_BIT_OCEAN_SHELF) {
                    probability += 2;
                }
            }
            if (probability != 0) {
                if (probability >= 2) {
                    probability += bit_count(static_cast<int>(tile->bit & MapTileFlagClearMask));
                    if (search_radius_index <= 8) ++probability;
                }
                if ((tile->climate & 0xE0) <= 0x5F) probability = (probability + 1) / 2;
                probability *= 10;
                probability = probability != 1 && probability > 0
                    ? rand() % probability : 0;
                if (best_roll < probability) {
                    best_roll = probability;
                    best_radius_index = search_radius_index;
                }
            }
        }
    }
    if (best_radius_index < 0) return;

    const int event_count = ecology_wrapped_add(player.planet_ecology, 1);
    player.planet_ecology = event_count;
    int event_x = base_x + RadiusOffsetX[best_radius_index];
    if ((MapIsFlat & 1) == 0) {
        if (event_x < 0) event_x += MapLongitudeBounds;
        else if (event_x >= MapLongitudeBounds) event_x -= MapLongitudeBounds;
    }
    const int event_y = base_y + RadiusOffsetY[best_radius_index];
    int spread_count = 1;
    if ((map_loc(event_x, event_y)->climate & 0xE0) <= 0x5F && event_count >= 4) {
        const int difficulty_adjustment = (event_count + (event_count >> 31 & 7)) >> 3;
        const int chance = ecology_wrapped_neg_sub(player.diff_level, difficulty_adjustment);
        const int spread_denominator = ecology_wrapped_add(chance, 10);
        if (spread_denominator <= 1 || rand() % spread_denominator == 0) spread_count = 9;
    }

    int report_terraform = -1;
    bool changed_visible_tile = false;
    int spread_index = 0;
    while (spread_index < spread_count) {
        int x = event_x + RadiusOffsetX[spread_index];
        if ((MapIsFlat & 1) == 0) {
            if (x < 0) x += MapLongitudeBounds;
            else if (x >= MapLongitudeBounds) x -= MapLongitudeBounds;
        }
        const int y = event_y + RadiusOffsetY[spread_index];
        {
        if (y < 0 || y >= MapLatitudeBounds || x < 0 || x >= MapLongitudeBounds) goto next_spread_tile;
        if (spread_index != 0 && (map_loc(x, y)->climate & 0xE0) > 0x5F) goto next_spread_tile;

        Map *tile = map_loc(x, y);
        if (((tile->bit & BIT_FUNGUS) != 0 && (tile->climate & 0xE0) > 0x3F)
            || ((tile->bit & BIT_BASE_IN_TILE) != 0 && (tile->val2 & 0x0F) < 8)
            || ((tile->bit2 & 0x80000002u) == 2)) goto next_spread_tile;

        if (report_terraform < 0) {
            // Native reuses the spread-index local as the Terraforming cursor;
            // after this 19-row scan the resumed spread loop observes that
            // cursor value rather than its previous one.
            spread_index = 0;
            do {
                if ((tile->bit & Terraforming[spread_index].bit & MapTileFlagClearMask) != 0)
                    report_terraform = spread_index;
                ++spread_index;
            } while (spread_index < 19);
        }
        bit_set(x, y, BIT_FUNGUS, 1);
        bit_set(x, y, MapTileFlagClearMask, 0);
        if ((tile->climate & 0xE0) <= 0x3F) world_alt_set(x, y, ALT_OCEAN_SHELF, 0);

        const int current_viewer = ConsoleGlobal.faction_id_;
        const bool known = (PlayersData[current_viewer].flags & 0x200u) != 0
            || (tile->visibility & (1u << (current_viewer & 31))) != 0
            || (GameDataStateStorage.fields_.game_state_ & 0x80) != 0;
        if (!known) goto next_spread_tile;
        changed_visible_tile = true;
        if (faction == current_viewer) {
            ConsoleGlobal.focus(x, y, current_viewer);
            if (PlayersData[faction].planet_ecology < 2) interlude(3, nullptr, 1, 0);
            boom(x, y, 0x80);
        }
        // focus() may change the active viewer; native reloads the viewer
        // global after the callback before synchronizing this tile.
        synch_bit(x, y, ConsoleGlobal.faction_id_);
        draw_tile(x, y, 2);
        }

next_spread_tile:
        ++spread_index;
    }

    if (changed_visible_tile) {
        const short shift_state = GetAsyncKeyState(0x10);
        if ((shift_state & static_cast<short>(0xFF00)) == 0 && IsMultiplayerNet == 0)
            clock_wait(20);
        const Map *event_tile = map_loc(event_x, event_y);
        const int current_viewer = ConsoleGlobal.faction_id_;
        const int occupier = (event_tile->bit & BIT_VEH_IN_TILE) != 0
            ? (event_tile->val2 & 0x0F) : -1;
        if (faction == current_viewer || occupier == current_viewer) {
            MapWinTable[MapWinSelectedSlot]->set_center(event_x, event_y, 1);
            ConsoleGlobal.focus(event_x, event_y, faction);
            parse_says(0, BaseCurrent->name_string, -1, -1);
            if (report_terraform >= 0 && faction == current_viewer) {
                const char *improvement_name =
                    (map_loc(event_x, event_y)->climate & 0xE0) < 0x60
                        ? Terraforming[report_terraform].name_sea
                        : Terraforming[report_terraform].name;
                parse_say(1, static_cast<int>(reinterpret_cast<intptr_t>(improvement_name)), -1, -1);
                POP2("FUNGUSGROWS2", "fung_sm.pcx", BaseIDCurrentSelected);
            } else {
                POP2("FUNGUSGROWS", "fung_sm.pcx", -1);
            }
        }
        if (faction == ConsoleGlobal.faction_id_
            && PlayersData[faction].planet_ecology >= 2)
            interlude(5, BaseCurrent->name_string, 1, 0);
    }

    if (!is_human(faction)) {
        int chance = rand() & 0x80000001;
        if ((chance == 0 || static_cast<uint32_t>(chance) == 0x80000000u) && GameDataStateStorage.fields_.diff_level_current_ >= 1
            && rand() % (GameDataStateStorage.fields_.diff_level_current_ + 1) != 0) {
            --player.planet_ecology;
            return;
        }
    }
    if ((player.flags & 0x4000) != 0) {
        player.flags &= ~0x4000u;
        return;
    }

    if (PlayersData[faction].planet_ecology < 7) return;
    if (base_project(SP_VOICE_OF_PLANET) != SP_Unbuilt) return;
    {
        if (faction == ConsoleGlobal.faction_id_) interlude(8, nullptr, 1, 0);
        const int event_roll = rand() & 0x80000003;
        if ((event_roll == 0 || static_cast<uint32_t>(event_roll) == 0x80000000u) && ExpansionEnabled
            && (map_loc(event_x, event_y)->climate & 0xE0) >= 0x60) {
            veh_init(0x13, 0, event_x, event_y);
        }
        int created = 0;
        int spawn_gate_count = 0;
        int spawn_index = 0;
        while (true) {
            // Native reloads planet_ecology at the shared loop head on every
            // pass, after callbacks/vehicle creation from the previous pass.
            const int live_spawn_ecology = PlayersData[faction].planet_ecology;
            const int live_spawn_limit = live_spawn_ecology < 9 ? live_spawn_ecology : 9;
            if (spawn_index >= live_spawn_limit) break;
            int x = event_x + RadiusOffsetX[spawn_index];
            if ((MapIsFlat & 1) == 0) {
                if (x < 0) x += MapLongitudeBounds;
                else if (x >= MapLongitudeBounds) x -= MapLongitudeBounds;
            }
            const int y = event_y + RadiusOffsetY[spawn_index];
            if (x >= 0 && x < MapLongitudeBounds && y >= 0 && y < MapLatitudeBounds) {
                Map *tile = map_loc(x, y);
                if ((tile->bit & BIT_FUNGUS) != 0 && (tile->climate & 0xE0) >= 0x40) {
                    int nearest_id = -1;
                    int nearest_distance = 9999;
                    for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
                        const Base &candidate = Bases[base_id];
                        int dx = abs(x - candidate.x);
                        if ((MapIsFlat & 1) == 0 && dx > static_cast<int>(MapLongitude))
                            dx = MapLongitudeBounds - dx;
                        const int distance = vector_dist(dx, abs(y - candidate.y));
                        if (distance <= nearest_distance) {
                            nearest_distance = distance;
                            nearest_id = base_id;
                        }
                    }
                    if (nearest_id >= 0) {
                        BaseFindDist = nearest_distance;
                        const uint8_t occupier = tile->val2 & 0x0F;
                        const bool no_foreign_occupier =
                            ((tile->bit & BIT_VEH_IN_TILE) == 0 || occupier >= 8 || occupier == 0)
                            && ((tile->bit & BIT_BASE_IN_TILE) == 0 || occupier >= 8 || occupier == 0);
                        if (Bases[nearest_id].faction_id_current == faction && no_foreign_occupier) {
                            int unit_kind;
                            if (tile->region >= 0x40) {
                                unit_kind = (spawn_gate_count % 3 != 0) ? 9 : 10;
                            } else {
                                unit_kind = PlayersData[faction].region_total_bases[tile->region] != 0
                                    ? 8 : 10;
                            }
                            bool create = unit_kind == 8;
                            if (!create) {
                                ++spawn_gate_count;
                                create = spawn_gate_count
                                        < ecology_wrapped_double(PlayersData[faction].pop_total) / 3
                                    && spawn_gate_count < PlayersData[faction].planet_ecology / 3;
                            }
                            if (create) {
                                if (unit_kind == 8 && rand() % 5 == 0 && ExpansionEnabled)
                                    unit_kind = 15;
                                const int unit_id = veh_init(unit_kind, 0, x, y);
                                if (unit_id >= 0)
                                    Vehs[unit_id].order_auto_type = static_cast<uint8_t>(faction);
                                draw_tile(x, y, 2);
                                ++created;
                            }
                        }
                    }
                }
            }
            ++spawn_index;
        }

        Map *center_tile = map_loc(event_x, event_y);
        const int center_occupier = center_tile->val2 & 0x0F;
        if (((center_tile->bit & (BIT_BASE_IN_TILE | BIT_VEH_IN_TILE)) == 0
             || center_occupier >= 8 || center_occupier == 0)) {
            const int region = center_tile->region;
            int unit_kind = 10;
            if (player.region_total_bases[region] != 0)
                unit_kind = (region >= 0x40) + 8;
            while (created < PlayersData[faction].planet_ecology / 3
                && created < BaseCurrent->population_size) {
                if (unit_kind == 8 && rand() % 5 == 0 && ExpansionEnabled)
                    unit_kind = 15;
                veh_init(unit_kind, 0, event_x, event_y);
                ++created;
                if (unit_kind == 15) unit_kind = 8;
            }
        }
    }

    if (((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1u << (faction & 31))) != 0) || GameDataStateStorage.fields_.diff_level_current_ >= 4) {
        GameDataStateStorage.fields_.ecology_event_accumulator_ = ecology_wrapped_add(GameDataStateStorage.fields_.ecology_event_accumulator_, 1);
        int warming = MapSeaLevel;
        if (warming < 0) warming = 0;
        else if (warming > 100) warming = 100;
        const int raw_sea_level_rises = ecology_wrap_int(WorldBuilder.sea_level_rises);
        int rise = raw_sea_level_rises;
        if (rise < 1) rise = 1;
        else if (rise >= 100) rise = 99;
        const int warming_product = ecology_wrap_int(
            static_cast<uint32_t>(Rules.freq_global_warming_num)
            * static_cast<uint32_t>(GameDataStateStorage.fields_.ecology_event_accumulator_));
        const int warming_denominator =
            ecology_wrap_int(static_cast<uint32_t>(Rules.freq_global_warming_denom));
        if ((warming * 3) / rise + 6 <= warming_product / warming_denominator) {
            GameDataStateStorage.fields_.ecology_event_accumulator_ = 0;
            int increase = PlayersData[faction].planet_ecology / 6 - 1;
            if (increase < 1) increase = 1;
            else if (increase >= 4) increase = 3;
            MapSeaLevelCouncil = ecology_wrapped_add(MapSeaLevelCouncil, increase);
            GameDataStateStorage.fields_.climate_rise_limit_ = 20;
            const int sea_level_product = ecology_wrapped_multiply(
                raw_sea_level_rises, increase);
            GameDataStateStorage.fields_.climate_sea_level_delta_ = ecology_wrapped_add(
                GameDataStateStorage.fields_.climate_sea_level_delta_, sea_level_product / 3);
            GameDataStateStorage.fields_.climate_rise_step_ = ecology_native_abs(GameDataStateStorage.fields_.climate_sea_level_delta_);
            GameDataStateStorage.fields_.climate_rise_accum_ = 0;
            parse_num(0, 20);
            const int absolute_delta = ecology_native_abs(GameDataStateStorage.fields_.climate_sea_level_delta_);
            const int scaled_delta = ecology_wrapped_multiply(absolute_delta, 1000);
            parse_num(1, scaled_delta / 15);
            if (faction != ConsoleGlobal.faction_id_) {
                GenderDefault = Players[faction].noun_gender;
                PluralityDefault = Players[faction].is_noun_plural;
                parse_says(0, Players[faction].noun_faction, -1, -1);
                popp(*ClimatePopupTitle, "TRIGGERWARMING2", 0, "searis_sm.pcx", nullptr);
                return;
            }
            popp(*ClimatePopupTitle, "TRIGGERWARMING", 0, "searis_sm.pcx", nullptr);
            return;
        }
    }

    if (PlayersData[faction].planet_ecology < 10 || (GameDataStateStorage.fields_.game_state_ & 0x20000) != 0) return;
    const int volcano_x_roll = MapLongitudeBounds > 1 ? rand() % MapLongitudeBounds : 0;
    const int volcano_y = MapLatitudeBounds > 1 ? rand() % MapLatitudeBounds : 0;
    int even_x = volcano_x_roll;
    if ((even_x & 1) != 0) --even_x;
    if ((volcano_y & 1) != 0) ++even_x;
    if (even_x < 0 || even_x >= MapLongitudeBounds || volcano_y < 0
        || volcano_y >= MapLatitudeBounds) return;
    const Map *volcano_tile = map_loc(even_x, volcano_y);
    if ((volcano_tile->climate & 0xE0) > 0x5F) return;
    int occupied_neighbor = 0;
    for (int volcano_radius_index = 0; volcano_radius_index < 49; ++volcano_radius_index) {
        int x = even_x + RadiusOffsetX[volcano_radius_index];
        if ((MapIsFlat & 1) == 0) {
            if (x < 0) x += MapLongitudeBounds;
            else if (x >= MapLongitudeBounds) x -= MapLongitudeBounds;
        }
        const int y = volcano_y + RadiusOffsetY[volcano_radius_index];
        if (x < 0 || x >= MapLongitudeBounds || y < 0 || y >= MapLatitudeBounds) continue;
        const Map *nearby = map_loc(x, y);
        if ((nearby->bit & (BIT_BASE_IN_TILE | BIT_VEH_IN_TILE)) != 0
            && (nearby->val2 & 0x0F) <= 7) {
            occupied_neighbor = 1;
            break;
        }
    }
    if (occupied_neighbor) return;
    if ((PlayersData[ConsoleGlobal.faction_id_].flags & 0x200u) != 0
        || (volcano_tile->visibility & (1u << (ConsoleGlobal.faction_id_ & 31))) != 0) {
        ConsoleGlobal.focus(even_x, volcano_y, ConsoleGlobal.faction_id_);
    }
    GameDataStateStorage.fields_.game_state_ |= 0x20000;
    world_volcano(even_x, volcano_y, 1);
    world_climate();
    draw_map(1);
    clock_wait(1000);
    GenderDefault = Players[ConsoleGlobal.faction_id_].is_leader_female;
    PluralityDefault = 0;
    parse_says(0, Players[ConsoleGlobal.faction_id_].title_leader, -1, -1);
    GenderDefault = Players[ConsoleGlobal.faction_id_].is_leader_female;
    PluralityDefault = 0;
    parse_says(1, Players[ConsoleGlobal.faction_id_].name_leader, -1, -1);
    if ((PlayersData[ConsoleGlobal.faction_id_].flags & 0x200u) != 0
        || (map_loc(even_x, volcano_y)->visibility & (1u << (ConsoleGlobal.faction_id_ & 31))) != 0) {
        popp(*ClimatePopupTitle, "VOLCANO", 0, "volc_sm.pcx", nullptr);
    }
}

// ORIGINAL: 0x004F3FE0 ?base_hurry@@YAXXZ 0x004F3FE0-0x004F4826
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004f3fe0.cpp; recovery/tasks/base-colony-economy-coverage-wave.json; recovery/tests/test_base_colony_economy.py", "Pinned01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf full4F3FE0..4F4826 instruction review and measured VC6 PlayerData/SocialEffect/Base/prototype owners.", "Native4F4365 reads pending.police(+260), not morale;4F4390 biases the wrapped reserve product before SAR2; facilities4/5/33/projects use /50, not /100.", "NativeVoice45F8..464E halves hurry budget;46C4 reads current queue;46D7 preserves explicit zero-factor branch;4702/474C reread lock mask and479D/47CF reread popup faction."], "reason": "Recover complete colony hurry budgets, mineral purchases and project notification using real Base/faction/prototype/facility owners and explicit native 32-bit wrapping. Preserve seed/history and original behavior; no new byte, linked-preservation or runtime acceptance claim."}
// symbol    ?base_hurry@@YAXXZ
// TRIED: full register-transliteration from raw disasm (not Ghidra, whose DAT_0090ea30 offsets are all mis-scaled x2 - it typed that global short*, and its 4th arg to at_goal() is wrong too, using DAT_0090ea30[1] where raw disasm shows edi/cost). Division-by-constant idioms (magic-multiply+shift) written as plain C `/` and trusted to re-lower; verified against Ghidra's own reconstructed divisors (10, 25,50,100,250) rather than reverse-engineering each magic constant by hand. Compiles; frameless prologue vs original's framed - not chased given size.
void __cdecl base_hurry() {
    Base *current_base = BaseCurrent;
    const int faction = current_base->faction_id_current;
    const uint32_t faction_bit = 1u << faction;
    const bool is_human = (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & faction_bit) != 0;

    if (is_human
        && ((current_base->governor & GOV_MAY_HURRY_PRODUCTION) == 0
            || (current_base->governor & GOV_ACTIVE) == 0)) {
        return;
    }
    if (current_base->minerals_accumulated == 0) {
        return;
    }

    const int production_id = current_base->queue_production_id[0];
    const int facility_index = production_id < 0
        ? base_yield_sub(0, production_id) : 0;
    int production_cost;
    if (production_id >= 0) {
        production_cost = veh_cost(production_id, BaseIDCurrentSelected, nullptr);
    } else {
        production_cost = Facility[facility_index].cost;
    }
    const int scaled_cost = base_yield_mul(
        cost_factor(faction, 1, -1), production_cost);

    current_base = BaseCurrent;
    const Map &base_tile = *map_loc(current_base->x, current_base->y);
    current_base = BaseCurrent;
    const int region = base_tile.region;
    int hurry_amount = 0;

    if (production_id >= 0) {
        const VehPrototype &prototype = VehPrototypes[production_id];
        if (prototype.plan == PLAN_COLONIZATION
            && current_base->population_size < 2) {
            return;
        }

        if ((base_tile.bit & BIT_VEH_IN_TILE) != 0
            && (base_tile.val2 & 0x0Fu) < MaxPlayerNum
            && (current_base->state & BSTATE_COMBAT_LOSS_LAST_TURN) == 0
            && !at_goal(faction, 0x10, current_base->x, current_base->y)) {
            current_base = BaseCurrent;
            if (production_id >= 0x40
                && (prototype.flags & PROTO_TYPED_COMPLETE) == 0) {
                const int denominator = base_yield_mul(
                    base_yield_add(PlayersData[faction].ai_wealth, 2), 10);
                hurry_amount = PlayersData[faction].energy_reserves / denominator;
            } else {
                const uint8_t plan = prototype.plan;
                const uint8_t regional_plan =
                    PlayersData[faction].region_base_plan[region];
                if (regional_plan == plan
                    || (plan == PLAN_TERRAFORMING
                        && regional_plan == PLAN_COLONIZATION)
                    || (plan == PLAN_COMBAT && regional_plan < PLAN_RECONNAISANCE)) {
                    const int denominator =
                        base_yield_sub(base_yield_mul(
                            base_yield_add(PlayersData[faction].ai_wealth, 2), 25),
                            base_yield_mul(PlayersData[faction].ai_power, 10));
                    hurry_amount = PlayersData[faction].energy_reserves / denominator;
                    if (regional_plan == PLAN_COLONIZATION) {
                        const int adjusted = base_yield_mul(hurry_amount,
                            base_yield_add(PlayersData[faction].ai_growth, 5));
                        hurry_amount = base_yield_add(
                            adjusted, adjusted < 0 ? 3 : 0) >> 2;
                        if (PlayersData[faction].energy_reserves > GameDataStateStorage.fields_.turn_current_num_) {
                            hurry_amount = base_yield_add(hurry_amount, hurry_amount);
                        }
                    }
                }
            }
        } else {
            hurry_amount = PlayersData[faction].energy_reserves / 10;
        }

        if (planet_buster2(production_id) != 0
            && PlayersData[faction].planet_busters == 0) {
            current_base = BaseCurrent;
            for (int other = 1; other < MaxPlayerNum; ++other) {
                if (other != faction
                    && (PlayersData[faction].diplo_treaties[other] & 0x10u) != 0
                    && PlayersData[other].major_atrocities != 0
                    && PlayersData[faction].major_atrocities
                        <= PlayersData[other].major_atrocities) {
                    hurry_amount = PlayersData[faction].energy_reserves / 10;
                }
            }
        }
    } else if ((current_base->state & BSTATE_DRONE_RIOTS_ACTIVE) != 0
               && facility_index != 0) {
        hurry_amount = PlayersData[faction].energy_reserves
            / base_yield_mul(base_yield_add(base_yield_add(
                PlayersData[faction].ai_wealth,
                PlayersData[faction].ai_power), 1), 5);

        if (current_base->drone_total > current_base->talent_total) {
            const int selected_base_id = BaseIDCurrentSelected;
            int offset;
            int mask;
            bitmask(FAC_BROOD_PIT, &offset, &mask);
            current_base = BaseCurrent;
            const int police_bonus =
                (Bases[selected_base_id].facilities_built[offset] & mask) != 0
                    ? 2 : 0;
            if (base_yield_add(
                    PlayersData[faction].soc_effect_pending.values_[SocialEffect::POLICE], police_bonus) <= -2
                && facility_index < 0x46) {
                const int remaining_cost =
                    base_yield_sub(scaled_cost, current_base->minerals_accumulated);
                const int human_factor =
                    (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & faction_bit) != 0 ? 2 : 0;
                const int available = base_yield_mul(remaining_cost, human_factor);
                const int reserves = PlayersData[faction].energy_reserves;
                const int wrapped_three_quarters = base_yield_mul(reserves, 3);
                const int population_limit = wrapped_three_quarters / 4;
                if (available >= 0 && population_limit >= 0) {
                    hurry_amount = available <= population_limit
                        ? available : population_limit;
                } else {
                    hurry_amount = 0;
                }
            }
        }
    } else if (facility_index == 1) {
        hurry_amount = PlayersData[faction].energy_reserves / 10;
    } else if (facility_index == 4 || facility_index == 5) {
        hurry_amount = PlayersData[faction].energy_reserves / 50;
        for (int other = 1; other < MaxPlayerNum; ++other) {
            if (other != faction
                && (PlayersData[faction].diplo_treaties[other] & 0x40030u) != 0
                && (PlayersData[other].region_total_bases[other] != 0
                    || (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1u << other)) != 0)) {
                hurry_amount = PlayersData[faction].energy_reserves / 10;
            }
        }
    } else if (facility_index < 0x46) {
        if (facility_index == 0x21) {
            hurry_amount = PlayersData[faction].energy_reserves / 50;
            if (at_climax(faction) != 0) {
                hurry_amount = base_yield_add(hurry_amount, hurry_amount);
            }
        }
    } else {
        hurry_amount = PlayersData[faction].energy_reserves / 50;
        if (PlayersData[faction].energy_reserves >= 0x7D0) {
            hurry_amount = base_yield_mul(hurry_amount, 4);
        }
        if (facility_index == 0x66) {
            hurry_amount = base_yield_mul(hurry_amount, 4);
        }
    }

    if (PlayersData[faction].energy_reserves >= 0x12C0) {
        hurry_amount = base_yield_add(hurry_amount,
            PlayersData[faction].energy_reserves / 25);
    } else if (PlayersData[faction].energy_reserves > 0xC80) {
        hurry_amount = base_yield_add(hurry_amount,
            PlayersData[faction].energy_reserves / 100);
    } else if (PlayersData[faction].energy_reserves > 0x7D0) {
        hurry_amount = base_yield_add(hurry_amount,
            PlayersData[faction].energy_reserves / 250);
    }

    if (hurry_amount < 0 || PlayersData[faction].energy_reserves < 0) {
        hurry_amount = 0;
    } else if (hurry_amount > PlayersData[faction].energy_reserves) {
        hurry_amount = PlayersData[faction].energy_reserves;
    }

    current_base = BaseCurrent;
    int remaining_cost =
        base_yield_sub(scaled_cost, current_base->minerals_accumulated);
    if (hurry_amount >= 0 && remaining_cost >= 0) {
        if (hurry_amount > remaining_cost) {
            hurry_amount = remaining_cost;
        }
        if (hurry_amount != 0) {
            if (GameDataStateStorage.fields_.secret_project_.voice_of_planet() >= 0) {
                hurry_amount /= 2;
            } else {
                if (strcmp(Players[faction].filename, "MORGAN") == 0) {
                    hurry_amount = base_yield_mul(hurry_amount, 2) / 3;
                } else if (strcmp(Players[faction].filename, "PEACE") == 0) {
                    hurry_amount = base_yield_mul(hurry_amount, 3) / 2;
                }
            }

            const int rule_hurry = Players[faction].rule_hurry;
            if (rule_hurry != 100
                && hurry_amount < PlayersData[faction].energy_reserves / 10) {
                hurry_amount = base_yield_mul(hurry_amount, 100) / rule_hurry;
            }

            current_base = BaseCurrent;
            remaining_cost =
                base_yield_sub(scaled_cost, current_base->minerals_accumulated);
            int turns_factor = base_yield_add(
                base_yield_mul(remaining_cost, remaining_cost) / 20,
                base_yield_mul(remaining_cost, 2));
            current_base = BaseCurrent;
            if (current_base->queue_production_id[0] <= -0x46) {
                turns_factor = base_yield_add(turns_factor, turns_factor);
            }
            if (GameDataStateStorage.fields_.secret_project_.voice_of_planet() >= 0) {
                turns_factor = base_yield_add(turns_factor, turns_factor);
            }
            int bought_turns = turns_factor != 0
                ? base_yield_mul(remaining_cost, hurry_amount) / turns_factor
                : 0;
            bought_turns = base_yield_mul(bought_turns, 100) / rule_hurry;
            current_base = BaseCurrent;
            current_base->minerals_accumulated =
                base_yield_add(current_base->minerals_accumulated, bought_turns);

            int energy_change;
            if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & faction_bit) != 0) {
                energy_change = 0;
            } else {
                const int ranking = PlayersData[GameDataStateStorage.fields_.ranking_faction_id_unk1_].ranking;
                const int ranking_factor = base_yield_mul(
                    base_yield_sub(8, ranking), 10);
                energy_change = base_yield_mul(
                    GameDataStateStorage.fields_.diff_level_current_, hurry_amount) / ranking_factor;
            }
            PlayersData[faction].energy_reserves = base_yield_add(
                PlayersData[faction].energy_reserves,
                base_yield_sub(energy_change, hurry_amount));
        }
    }

    if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & faction_bit) != 0
        || facility_index < FacilitySPStart) {
        return;
    }
    if (base_project(facility_index - FacilitySPStart) != SP_Unbuilt
        || GameDataStateStorage.fields_.sunspot_duration_ > 0
        || (GameDataStateStorage.fields_.game_state_ & STATE_UNK_1) != 0) {
        return;
    }

    current_base = BaseCurrent;
    const int facility_cost = base_yield_mul(
        cost_factor(current_base->faction_id_current, 1, -1),
        Facility[facility_index].cost);
    current_base = BaseCurrent;
    if (base_yield_add(current_base->mineral_surplus,
                       current_base->minerals_accumulated) < facility_cost) {
        return;
    }

    parse_says(0, Players[current_base->faction_id_current].adj_name_faction,
               -1, -1);
    parse_say(1, static_cast<int>(reinterpret_cast<intptr_t>(
        Facility[facility_index].name)), -1, -1);
    popp(reinterpret_cast<const char *>(*ClimatePopupTitle), "DONEPROJECT", 0,
         "secproj_sm.pcx", nullptr);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004F4830 ?base_check_support@@YAXXZ 0x004F4830-0x004F4DBB
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004f4830.cpp", "recovery/tasks/base-colony-economy-coverage-wave.json", "recovery/tests/test_base_colony_economy.py", "Pinned original 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; complete original extent reviewed against native instructions and measured VC6 owner layouts.", "Native facility35 uses selected-base record; riot score reads prototype.cost(+29), not carry_capacity(+28); first distance scan reapplies abs after wrap, second does not."], "reason": "Recover the complete support deficit and pacifism selection through typed Base/Vehicle/faction owners, preserving original AI double-decrement and selected-base facility semantics. Seed retained; no new byte or runtime claim."}
// symbol    ?base_check_support@@YAXXZ
void __cdecl base_check_support() {
    const unsigned faction_bit = 1u << BaseCurrent->faction_id_current;
    const unsigned faction = BaseCurrent->faction_id_current;

    while (BaseCurrent->mineral_intake_2 < (int)BaseCurrentForcesMaintCost) {
        int selected_vehicle = -1;
        int selected_score = -1;

        for (int vehicle_id = 0; vehicle_id < GameDataStateStorage.fields_.vehicle_current_count_; ++vehicle_id) {
            const Veh &vehicle = Vehs[vehicle_id];
            if (vehicle.faction_id != faction
                || vehicle.home_base_id != BaseIDCurrentSelected) {
                continue;
            }

            const VehPrototype &prototype = VehPrototypes[vehicle.proto_id];
            if (prototype.plan == PLAN_SUPPLY_CONVOY) {
                if (vehicle.order == ORDER_CONVOY
                    && vehicle.order_auto_type == ORDERA_TERRA_AUTO_ROAD) {
                    const Map *tile = map_loc(vehicle.x, vehicle.y);
                    if (selected_score < 99999
                        && (tile->bit & BIT_BASE_IN_TILE) != 0
                        && (tile->val2 & 0x0Fu) < MaxPlayerNum) {
                        selected_vehicle = vehicle_id;
                        selected_score = 99999;
                    }
                }
            } else if ((vehicle.state & VSTATE_REQUIRES_SUPPORT) != 0
                       && prototype.plan < 10) {
                int dx = abs((int)vehicle.x - (int)BaseCurrent->x);
                if ((MapIsFlat & 1) == 0 && (int)MapLongitude < dx) {
                    dx = base_yield_sub(MapLongitudeBounds, dx);
                }
                dx = abs(dx);
                const int dy = abs((int)vehicle.y - (int)BaseCurrent->y);
                const int greater = dx > dy ? dx : dy;
                const int lesser = dx > dy ? dy : dx;
                const int distance = greater
                    - (((dx + dy) / 2 - lesser + 1) / 2);
                if (selected_score < distance) {
                    selected_vehicle = vehicle_id;
                    selected_score = distance;
                }
            }
        }

        if (selected_vehicle < 0) {
            break;
        }

        Veh &vehicle = Vehs[selected_vehicle];
        const int prototype_id = vehicle.proto_id;
        const int plan = VehPrototypes[prototype_id].plan;
        if (plan == PLAN_SUPPLY_CONVOY) {
            vehicle.order = ORDER_NONE;
            --BaseCurrentForcesMaintCost;
            continue;
        }

        if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & faction_bit) == 0) {
            if (plan == PLAN_COLONIZATION) {
                vehicle.state &= ~VSTATE_REQUIRES_SUPPORT;
                if (!has_abil(prototype_id, ABL_CLEAN_REACTOR)) {
                    const int support = PlayersData[faction].soc_effect_pending.values_[SocialEffect::SUPPORT];
                    BaseCurrentForcesMaintCost -= support < -3 ? 2 : 1;
                }
            }
        } else {
            parse_says(1, VehPrototypes[prototype_id].veh_name, -1, -1);
            popb("NOSUPPORT", 0x8000, 0xD, "genwarning_sm.pcx", nullptr);
        }

        if (has_abil(prototype_id, ABL_CLEAN_REACTOR)) {
            break;
        }

        const int support = PlayersData[faction].soc_effect_pending.values_[SocialEffect::SUPPORT];
        BaseCurrentForcesMaintCost -= support < -3 ? 2 : 1;
        kill(selected_vehicle);
    }

    const bool is_human = (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & faction_bit) != 0;
    const uint32_t governor = BaseCurrent->governor;
    const bool state_allows_riot_check = !is_human
        || ((governor & GOV_MANAGES_CITIZENS_SPECS) != 0
            && (governor & GOV_ACTIVE) != 0);

    if (!state_allows_riot_check
        || (BaseCurrent->state & BSTATE_DRONE_RIOTS_ACTIVE) == 0) {
        return;
    }

    int morale_bonus_offset;
    int morale_bonus_mask;
    bitmask(0x23, &morale_bonus_offset, &morale_bonus_mask);
    const int morale_bonus =
        (Bases[BaseIDCurrentSelected].facilities_built[morale_bonus_offset]
         & morale_bonus_mask) != 0 ? 2 : 0;
    const int morale = PlayersData[faction].soc_effect_pending.values_[SocialEffect::MORALE];
    if (base_yield_add(morale, morale_bonus) >= -2
        || (BaseCurrent->drone_total <= BaseCurrent->talent_total
            && BaseCurrent->nutrient_surplus >= 0)) {
        return;
    }

    int selected_score = -1;
    int selected_vehicle = -1;
    const int riot_probability = PlayersData[faction].ai_fight;
    for (int vehicle_id = 0; vehicle_id < GameDataStateStorage.fields_.vehicle_current_count_; ++vehicle_id) {
        const Veh &vehicle = Vehs[vehicle_id];
        if (vehicle.faction_id != faction
            || vehicle.home_base_id != BaseIDCurrentSelected
            || (vehicle.state & VSTATE_PACIFISM_DRONE) == 0) {
            continue;
        }

        if (is_human
            && ((vehicle.state & VSTATE_UNK_200) == 0
                || (vehicle.state & 0x01000200u) == 0x01000200u
                || ((vehicle.state & 0x02000200u) == 0x02000200u
                    && vehicle.waypoint_count > vehicle.patrol_current_point)
                || vehicle.order_auto_type == ORDERA_BOMBING_RUN
                || vehicle.order_auto_type == ORDERA_ON_ALERT
                || vehicle.order_auto_type != 0)) {
            continue;
        }

        const int random_divisor = base_yield_add(riot_probability, 2);
        if (base_yield_sub(random_divisor, 1) > 0
            && rand() % random_divisor != 0) {
            continue;
        }

        int dx = abs((int)vehicle.x - (int)BaseCurrent->x);
        if ((MapIsFlat & 1) == 0 && (int)MapLongitude < dx) {
            dx = base_yield_sub(MapLongitudeBounds, dx);
        }
        const int dy = abs((int)vehicle.y - (int)BaseCurrent->y);
        int range = VehPrototypes[vehicle.proto_id].cost;
        if (range < 1) range = 1;
        else if (range > 99) range = 99;

        int score = base_yield_mul(base_yield_add(dx, dy) / 2, 100) / range;
        const int plan = VehPrototypes[vehicle.proto_id].plan;
        if (plan == PLAN_DEFENSIVE) {
            score = base_yield_add(score, score);
        } else if (plan == PLAN_COMBAT) {
            score /= 2;
        }

        if (selected_score < score) {
            selected_score = score;
            selected_vehicle = vehicle_id;
        }
    }

    if (selected_vehicle >= 0) {
        kill(selected_vehicle);
        BaseCurrent->state &= ~BSTATE_DRONE_RIOTS_ACTIVE;
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004F4E80 ?base_research@@YAXXZ 0x004F4E80-0x004F4FAB
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004f4e80.cpp", "recovery/tasks/base-colony-economy-coverage-wave.json", "recovery/tests/test_base_colony_economy.py", "Pinned original 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; complete original extent reviewed against native instructions and measured VC6 owner layouts.", "Original4F4E80..4F4FAB reads Base.labs_total(+108), PlayerData.soc_effect_pending.research and net_random_event; faction-indexed Base bitset and signed remainder are intentional retained behavior."], "reason": "Recover the complete base laboratory contribution, scenario/early-turn gates and signed fractional research dispatch with native 32-bit wrap. Preserve original Bases[faction_id] facility lookup rather than silently correcting its behavior. Seed retained; no matching or game claim."}
// symbol    ?base_research@@YAXXZ
void __cdecl base_research() {
    if ((GameDataStateStorage.fields_.game_rules_ & RULES_SCN_NO_TECH_ADVANCES) != 0) {
        return;
    }

    Base *base = BaseCurrent;
    const int faction_id = base->faction_id_current;
    const int research_bonus = base->labs_total;
    int mask_offset;
    int mask;
    int points = research_bonus;
    bitmask(0x17, &mask_offset, &mask);

    if ((Bases[faction_id].facilities_built[mask_offset] & mask) != 0) {
        points = research_bonus / 2;
    }
    PlayerData &player = PlayersData[faction_id];
    if (GameDataStateStorage.fields_.turn_current_num_ < base_yield_mul(-5, player.soc_effect_pending.values_[SocialEffect::RESEARCH])) {
        points = 0;
    }
    if (GameDataStateStorage.fields_.turn_current_num_ < 5 && player.diff_level < 2) {
        points = 0;
    }

    player.labs_total = base_yield_add(player.labs_total, points);

    int research_rate = player.soc_effect_pending.values_[SocialEffect::RESEARCH];
    if (research_rate < -5) {
        research_rate = -5;
    } else if (research_rate > 5) {
        research_rate = 5;
    }

    int scaled = base_yield_mul(
        base_yield_mul(base_yield_add(research_rate, 10), points), 10);
    const int remainder = base_yield_add(player.net_random_event, scaled % 100);
    player.net_random_event = remainder;
    if (remainder > 99) {
        scaled = base_yield_add(scaled, 100);
        player.net_random_event = base_yield_sub(remainder, 100);
    }

    tech_research(faction_id, scaled / 100);
}

/*
Purpose: Set or unset the specified base's facility.
// ORIGINAL: 0x004E48B0 ?set_fac@@YAXHHH@Z 0x004E48B0-0x004E48FA BYTE_EXACT
// LEVER: bitmask_call, and the offset out-param is THIS parameter's slot.
//   The folded `MEASURED inline bitmask` turned the call into a shift/and
//   pair in place (0/32); general.h's bitmask_call restores the image's
//   `call 0x50ba00`. And the image passes &facility_id - not a separate
//   local - as the offset pointer, which puts offset in the dead parameter
//   slot at [ebp+8] and mask at [ebp-4]. TRIED: `int offset; int mask;`
//   with `&offset`, the shape has_fac_built_call and sea_coasts use - 3/32,
//   because offset lands at [ebp-4] and pushes mask out of the frame.
// size      74 bytes
// prototype void (__cdecl ?set_fac@@YAXHHH@Z)(int facilityID, int baseID, BOOL set)
// callers   10   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0050BA00
Return Value: n/a
Status: Complete
*/
void __cdecl set_fac(int facility_id, int base_id, BOOL set) {
    int mask;
    // The image hands bitmask() THIS parameter's own slot as the offset
    // out-param: facility_id is dead after the call, and its stack slot
    // holds the offset both subscripts below read. A separate `int offset`
    // local lands at [ebp-4] instead and pushes mask out of the frame -
    // measured 3/32 against 32/32 for this spelling.
    bitmask_call(facility_id, &facility_id, &mask);
    if (set) {
        Bases[base_id].facilities_built[facility_id] |= mask;
    } else {
        Bases[base_id].facilities_built[facility_id] &= ~mask;
    }
}

/*
Purpose: Check whether the facility audio blurb announcement has played for the faction.
// ORIGINAL: 0x004E4900 has_fac_announced 0x004E4900-0x004E4952 BYTE_EXACT
// LEVER: signedness. `facility_id` is uint32_t, so `facility_id > FacilitySPStart` forced an unsigned `jbe`; casting to `(int)facility_id` before the compare, without changing the parameter's mangled type, restores the image's signed `jle`. BYTE_EXACT.
// LEVER: bitmask_call with &facility_id as the offset out-param, exactly as set_fac records - the image reuses the dead parameter slot at [ebp+0xc] for the offset and keeps mask at [ebp-4]. The `(int *)` cast is the uint32_t the mangled name (?has_fac_announced@@YAHII@Z) fixes.
// symbol    ?has_fac_announced@@YAHII@Z
// size      82 bytes
// prototype int (__cdecl has_fac_announced)(int factionID, int facilityID)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0050BA00
Return Value: Has the facility audio blurb played? true/false
Status: Complete
*/
BOOL __cdecl has_fac_announced(uint32_t faction_id, uint32_t facility_id) {
    if ((int)facility_id > FacilitySPStart) {
        return true;
    }
    int mask;
    // Same parameter-slot reuse the image shows in set_fac below: bitmask's
    // offset out-param is this function's own facility_id slot, dead after
    // the call. The cast is the uint32_t parameter the mangled name fixes
    // (?has_fac_announced@@YAHII@Z), not a pun of this.
    bitmask_call(facility_id, (int *)&facility_id, &mask);
    return (PlayersData[faction_id].facility_announced[facility_id] & mask) != 0;
}

/*
Purpose: Set or unset whether a particular facility audio blurb has played for specified faction.
// ORIGINAL: 0x004E4960 set_fac_announced 0x004E4960-0x004E49CA BYTE_EXACT
// LEVER: bitmask_call with &facility_id as the offset out-param - set_fac's
//   note above covers both halves of it. 0/42 -> 42/42.
// symbol    ?set_fac_announced@@YAXIIH@Z
// size      106 bytes
// prototype int (__cdecl set_fac_announced)(int factionID, int facilityID, int set)
// callers   0   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0050BA00
Return Value: n/a
Status: Complete
*/
void __cdecl set_fac_announced(uint32_t faction_id, uint32_t facility_id, BOOL set) {
    int mask;
    // set_fac's parameter-slot reuse again - the image's offset out-param is
    // this facility_id slot itself. The cast is the uint32_t the mangled
    // name (?set_fac_announced@@YAXIIH@Z) fixes, not a pun of this.
    bitmask_call(facility_id, (int *)&facility_id, &mask);
    if (set) {
        PlayersData[faction_id].facility_announced[facility_id] |= mask;
    } else {
        PlayersData[faction_id].facility_announced[facility_id] &= ~mask;
    }
}

/*
Purpose: Determine what unit the specified base should start building 1st then add it to the queue.
// ORIGINAL: 0x004E4AA0 ?base_first@@YAXH@Z 0x004E4AA0-0x004E4B7B
// size      219 bytes
// prototype void (__cdecl ?base_first@@YAXH@Z)(int baseID)
// callers   2   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005BA910
// TRIED: pre-storing `Bases[base_id].queue_production_id[0] = proto_id;`
//            before the loop (to match the image's early store at
//            0x004E4AA0 instr 7) and reading VehPrototypes[i].plan directly
//            at each compare instead of hoisting it into a local both
//            measured WORSE - best flag set /c /O2 /Gy /GR- /Oy- /GX dropped
//            from 0.912 to 0.883 similar (agreeing stays 4/78 either way).
Return Value: n/a
Status: Complete
*/
void __cdecl base_first(int base_id) {
    int priority = -1;
    uint32_t proto_id = BSC_SCOUT_PATROL;
    uint32_t faction_id = Bases[base_id].faction_id_current;
    for (int i = 0; i < MaxVehProtoNum; i++) {
        if (veh_avail(i, faction_id, base_id)) {
            if (i < MaxVehProtoFactionNum || VehPrototypes[i].flags & PROTO_TYPED_COMPLETE) {
                int compare = get_proto_defense_rating(i) * 32;
                uint32_t plan = VehPrototypes[i].plan;
                if (plan == PLAN_DEFENSIVE) {
                    compare *= 4;
                } else if (plan == PLAN_COMBAT) {
                    compare *= 3;
                } else if (plan == PLAN_RECONNAISANCE) {
                    compare *= 2;
                }
                if (get_proto_triad(i) != TRIAD_LAND) {
                    compare /= 4;
                }
                compare -= VehPrototypes[i].cost;
                if (compare >= priority) {
                    priority = compare;
                    proto_id = i;
                }
            }
        }
    }
    Bases[base_id].queue_production_id[0] = proto_id;
}

/*
Purpose: Calculate the new unit morale bonus modifier provided by the base and faction for a triad.
// ORIGINAL: 0x004E6400 ?morale_mod@@YAHHHH@Z 0x004E6400-0x004E65B1
// LEVER: image makes 4 real `call 0x50ba00` (bitmask), one per
// has_fac_built() site; the `inline bool has_fac_built(int, int)` in
// base.h was not being inlined here and compiled to a real call to
// has_fac_built itself instead (call_diff: MORE, wrong callee). Switching
// all four checks to has_fac_built_call() (base.h, __forceinline, the same
// lever recorded on cost_factor/breed_mod) fixes the call count (4 real
// bitmask calls, matching) and moves best similarity from a much lower
// score to 0.715, 7/153 agreeing.
// TRIED (still short of the image): the remaining gap starts with a
// register swap in the prologue (image keeps `ebx`=base_id, `esi`=faction_id;
// this tree's O2 default swaps them) and continues into how the bool result
// of `has_fac_built_call(...) || has_project(...)` is materialised - the
// image runs `and al,cl; neg al; sbb eax,eax; neg eax` and SPINS the boolean
// through the `triad` parameter's own stack slot ([ebp+0x10]) as scratch
// before the `||`, which this tree's direct `test ...; jne` never
// reproduces. Not a call-count or lever-shape issue at this point - register
// allocation and slot reuse the flag sweep does not reach.
// size      433 bytes
// prototype int (__cdecl ?morale_mod@@YAHHHH@Z)(int baseID, int factionID, int triad)
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0050BA00
Return Value: Morale bonus modifier
Status: Complete
*/
int __cdecl morale_mod(int base_id, int faction_id, int triad) {
    uint32_t morale_modifier = 0;
    if (triad == TRIAD_LAND) {
        if (has_fac_built_call(FAC_COMMAND_CENTER, base_id)
            || has_project(SP_COMMAND_NEXUS, faction_id)) {
            morale_modifier = 2;
        }
    } else if (triad == TRIAD_SEA) {
        if (has_fac_built_call(FAC_NAVAL_YARD, base_id)
            || has_project(SP_MARITIME_CONTROL_CENTER, faction_id)) {
            morale_modifier = 2;
        }
    } else if (triad == TRIAD_AIR) {
        if (has_fac_built_call(FAC_AEROSPACE_COMPLEX, base_id)
            || has_project(SP_CLOUDBASE_ACADEMY, faction_id)) {
            morale_modifier = 2;
        }
    }
    if (has_fac_built_call(FAC_BIOENHANCEMENT_CENTER, base_id)
        || has_project(SP_CYBORG_FACTORY, faction_id)) {
        morale_modifier += 2;
    }
    if (PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::MORALE] < -1) {
        morale_modifier /= 2;
    }
    return morale_modifier;
}

/*
Purpose: Calculate the new native unit lifecycle bonus modifier provided by a base and faction.
// ORIGINAL: 0x004E65C0 ?breed_mod@@YAHHH@Z 0x004E65C0-0x004E673D
// LEVER: body lives in base.h (MEASURED inline) as four has_fac_built_call()
//        checks instead of has_fac_built() - fixes breed_mod's OWN call
//        count (0 -> 4 real bitmask() calls) and moves its best similarity
//        to 0.749. TRADE-OFF, measured: since breed_mod is a single shared
//        inline body, the same edit reaches worm_mod's inlined copy and
//        drops ITS best similarity 0.826 -> 0.706 (worm_mod is 0x004E6740,
//        not claimed, so not a tracked regression - full detail at the
//        body in base.h and at worm_mod's own marker below).
// TRIED: register swap in the prologue (image keeps ebx=base_id,
//            esi=faction_id; /O2 default swaps them) and how the bool
//            result of `has_fac_built_call(...) || has_project(...)` is
//            materialised - the image spins it through a stack slot as
//            scratch before the `||`, which a direct `test/jne` never
//            reproduces. Register allocation, not a call-count/lever issue.
// size      381 bytes
// prototype int (__cdecl ?breed_mod@@YAHHH@Z)(int baseID, int factionID)
// callers   2   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0050BA00
Return Value: Lifecycle bonus
Status: Complete
*/
// BODY IN base.h, as `MEASURED inline`: worm_mod (base.cpp) needs it folded
// in place, matching the image, which writes breed_mod's whole body out at
// its one call site rather than calling 0x004E65C0. LEVER: also swapped
// the four has_fac_built(FAC_X, base_id) facility tests for
// has_fac(FAC_X, base_id, 0) - the recorded LEVER on base_minerals
// (base.cpp:1820, at the time of writing) found the same swap moved that
// body's call count from 1 to 6 of 7.

/*
Purpose: Calculate the count of lifecycle/psi bonuses that are provided by a base and faction.
// ORIGINAL: 0x004E6740 ?worm_mod@@YAHHH@Z 0x004E6740-0x004E68AB
// size      363 bytes
// prototype int (__cdecl ?worm_mod@@YAHHH@Z)(int baseID, int factionID)
// callers   1   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00421670
// LEVER: made breed_mod `MEASURED inline` at the end of base.h (lever 1) so
//        worm_mod open-codes it as the image does - the image's only calls
//        here are four has_fac (0x421670), never breed_mod out of line -
//        and swapped breed_mod's has_fac_built(FAC_X, base_id) tests for
//        has_fac(FAC_X, base_id, 0) so those four calls actually appear.
//        Best similarity 0.826, agreeing instructions 3->13 of 140.
// REGRESSED BY A SIBLING FIX, not yet re-chased: breed_mod's OWN address
// (0x004E65C0) needed those four has_fac() calls hand-inlined to
// has_fac_built_call() instead (see the LEVER at breed_mod's definition,
// base.h) - since breed_mod is one shared body, that same edit reaches
// worm_mod's inlined copy and drops its best similarity 0.826 -> 0.706.
// Not fixed here: worm_mod was not in the batch that made this trade.
Return Value: Native life modifier count
Status: Complete
*/
int __cdecl worm_mod(int base_id, int faction_id) {
    uint32_t worm_modifier_count = breed_mod(base_id, faction_id);
    if (Players[faction_id].rule_psi) {
        worm_modifier_count++;
    }
    if (has_project(SP_DREAM_TWISTER, faction_id)) {
        worm_modifier_count++;
    }
    if (has_project(SP_NEURAL_AMPLIFIER, faction_id)) {
        worm_modifier_count++;
    }
    return worm_modifier_count;
}


/*
Purpose: Address the map tile the terrain yield functions price.
Original Offset: n/a
Return Value: Pointer to map tile
Status: Complete

The three yield functions each inline this, and they inline it with signed
arithmetic - `imul` against y and `sar esi, 1` on x. map_loc() takes unsigned
coordinates and therefore shifts x logically, which differs for a negative x.
Neither form bounds-checks, exactly as the original does not.
*/
static __forceinline Map *yield_tile(int x, int y) {
    return &map_tiles()[(x >> 1) + y * (int)MapLongitude];
}

/*
Purpose: Identify the faction owning a base standing on the specified tile.
Original Offset: n/a
Return Value: Faction id of the base in the tile, or -1 for none/unowned
Status: Complete

All three yield functions inline this same sequence: the tile's low `bit` flag
selects it, the owner nibble of val2 supplies the id, and an id at or above
MaxPlayerNum - 0xF is the "unoccupied" marker - is rejected.
*/
static __forceinline int yield_tile_owner(const Map *tile) {
    if (!(tile->bit & BIT_BASE_IN_TILE)) {
        return -1;
    }
    int owner = tile->val2 & 0xF;
    return (owner < MaxPlayerNum) ? owner : -1;
}

/*
Purpose: Calculate the nutrients produced by a single map square.
// ORIGINAL: 0x004E6E50 ?crop_yield@@YAHHHHHH@Z 0x004E6E50-0x004E7306
// TRIED: caching bit2 to a named local before the landmark_bonus jungle/
//            fresh check (image re-reads tile->climate twice there instead
//            of the cached is_ocean_tile - now written that way, but score
//            unchanged: 0x8-vs-0xC sub esp gap under /Oy- is unexplained,
//            one 4-byte stack slot short of the image with no candidate
//            local found for it in the time spent).
// size      1206 bytes
// prototype int (__cdecl ?crop_yield@@YAHHHHHH@Z)(int factionID, int, int, int, int)
// callers   10   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0050BA00 0x00592030 0x005B9F20
// LEVER: yield_tile/yield_tile_owner (this file) marked `static __forceinline`
//        - shared with mine_yield/energy_yield/base_support, and a clear win
//        for base_support (0.160 -> 0.371 best similarity there). Tied here:
//        this function's own best-across-flags similarity is unchanged
//        (0.292, its winning flag set is /Ob0-based, where yield_tile was
//        already being folded in regardless of the marker) - kept anyway
//        since it does not regress and helps the sibling.
// TRIED: a `bonus_at_call` non-inline forwarder (map.h/map.cpp, since
//            removed) so bonus_at(x, y, 0) would emit `call 0x592030` as
//            the image does - measured NO improvement to best-across-flags
//            similarity (stayed at 0.292) on top of the forceinline change.
// LEVER: FEWER (2 calls vs image's 7: 5 bitmask() + bonus_at + has_tech) -
//        all five `has_fac_built(FAC_X, base_id)` sites constant-fold their
//        offset/mask math away here, unlike the image, which keeps
//        bitmask() out-of-line at every one. Switching all five to
//        `has_fac_built_call` took call_diff to 0 disagreeing with no
//        similarity regression (still 0.292 best-across-flags).
Return Value: Nutrients from the square
Status: Complete

`assume_improved` prices the square as if it had already been terraformed; the
name is inferred from the call sites, where StatusWin::draw_status asks for the
same square twice and base_terraform and the AI previews pass 1. Here it
suppresses the fungus path, an improved square having had its fungus cleared.
*/
int __cdecl crop_yield(int faction_id, int base_id, int x, int y,
                       BOOL assume_improved) {
    Map *tile = yield_tile(x, y);
    uint32_t bit = tile->bit;
    BOOL is_ocean_tile = (tile->climate & 0xE0) < ALT_BIT_SHORE_LINE;
    BOOL has_base_in_tile = yield_tile_owner(tile) >= 0;
    // Bonus id 1 is the nutrient bonus.
    BOOL has_nutrient_bonus = bonus_at(x, y, 0) == 1;
    // Nutrient landmarks: Jungle counts on land, Freshwater Sea on ocean. Not
    // cached to a local: bit2 is read once here (reused via register across
    // both disjuncts within this one expression), and both arms re-test
    // climate directly off tile->climate rather than reusing the cached
    // is_ocean_tile above - the image re-reads that byte twice here instead
    // of referencing the earlier local.
    BOOL landmark_bonus =
        (((tile->bit2 & (BIT2_UNK_80000000 | BIT2_JUNGLE)) == BIT2_JUNGLE)
            && (tile->climate & 0xE0) >= ALT_BIT_SHORE_LINE)
        || ((tile->bit2 & BIT2_FRESH) && (tile->climate & 0xE0) < ALT_BIT_SHORE_LINE);
    int crop;

    if (has_base_in_tile) {
        crop = ResourceInfo[RSCINFO_BASE_SQ].nutrients;
        if (has_nutrient_bonus) {
            crop *= 2;
        }
        if (landmark_bonus) {
            crop++;
        }
        if (base_id < 0) {
            return crop; // the original returns here rather than reaching the tail
        }
        if (has_fac_built_call(FAC_RECYCLING_TANKS, base_id)
            || has_fac_built_call(FAC_PRESSURE_DOME, base_id)) {
            crop += ResourceInfo[RSCINFO_RECYCLING_TANKS].nutrients;
        }
    } else if (bit & BIT_THERMAL_BORE) {
        crop = ResourceInfo[RSCINFO_BOREHOLE_SQ].nutrients;
        // Faithful: 0x004E6FCB is `test esi, esi` on the borehole value that was
        // just loaded, NOT on the resource bonus flag. mine_yield's structurally
        // parallel arm tests the flag. Transcribed literally.
        if (crop) {
            crop += ResourceInfo[RSCINFO_BONUS_SQ].nutrients;
        }
    } else if (bit & BIT_MONOLITH) {
        crop = ResourceInfo[RSCINFO_MONOLITH].nutrients;
        if (has_nutrient_bonus) {
            crop += ResourceInfo[RSCINFO_BONUS_SQ].nutrients;
        }
        if (has_project(SP_MANIFOLD_HARMONICS, faction_id)
            && PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::PLANET] > 0) {
            crop++;
        }
    } else if ((bit & BIT_FUNGUS) && !assume_improved
               && (tile->climate & 0xE0) >= ALT_BIT_OCEAN_SHELF) {
        int planet = PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::PLANET];
        int planet_mod = (planet < -3) ? -3 : ((planet > 0) ? 0 : planet);
        crop = range(PlayersData[faction_id].tech_fungus_nutrient + planet_mod,
                     0, 99);
        if (has_project(SP_MANIFOLD_HARMONICS, faction_id) && planet > 0) {
            crop++;
        }
    } else {
        if (is_ocean_tile) {
            crop = ResourceInfo[RSCINFO_OCEAN_SQ].nutrients;
            if (has_nutrient_bonus) {
                crop += ResourceInfo[RSCINFO_BONUS_SQ].nutrients;
            }
            if (landmark_bonus) {
                crop++;
            }
            if ((tile->climate & 0xE0) == ALT_BIT_OCEAN_SHELF
                || ExpansionEnabled) {
                // A kelp farm.
                if (bit & BIT_FARM) {
                    crop += ResourceInfo[RSCINFO_IMPROVED_SEA].nutrients;
                    // Legacy: unguarded for base_id < 0, exactly as 0x004E7150.
                    // Bases[(uint32_t)-1] and Bases[-1] are the same address on
                    // 32-bit x86, so this reproduces the original's read.
                    if (has_fac_built_call(FAC_AQUAFARM, (uint32_t)base_id)) {
                        crop++;
                    }
                }
                if ((bit & BIT_MINE) && crop > 1) {
                    crop += Rules.tgl_nutrient_effect_with_mine;
                }
            }
        } else if (bit & BIT_FOREST) {
            crop = ResourceInfo[RSCINFO_FOREST_SQ].nutrients;
            if (has_nutrient_bonus) {
                crop += ResourceInfo[RSCINFO_BONUS_SQ].nutrients;
            }
            if (base_id >= 0) {
                if (has_fac_built_call(FAC_HYBRID_FOREST, base_id)) {
                    crop++;
                }
                if (has_fac_built_call(FAC_TREE_FARM, base_id)) {
                    crop++;
                }
            }
            if (landmark_bonus) {
                crop++;
            }
        } else {
            int rockiness = tile->val3 >> 6;
            // Rainfall level: arid / moist / rainy.
            crop = (tile->climate >> 3) & 3;
            if (rockiness > ROCKINESS_ROLLING) {
                crop = 0;
            }
            if (has_nutrient_bonus) {
                crop += ResourceInfo[RSCINFO_BONUS_SQ].nutrients;
            }
            if (landmark_bonus) {
                crop++;
            }
            if (crop < 0) {
                crop = 0;
            }
            if ((bit & BIT_FARM) && rockiness <= ROCKINESS_ROLLING) {
                crop += ResourceInfo[RSCINFO_IMPROVED_LAND].nutrients;
            }
            if ((bit & BIT_MINE) && crop > 1) {
                crop += Rules.tgl_nutrient_effect_with_mine;
            }
        }
        if (bit & BIT_SOIL_ENRICHER) {
            crop += crop / 2;
        }
        BOOL has_condenser = (bit & BIT_CONDENSER) != 0;
        if (has_condenser) {
            crop += crop / 2;
        }
        // Only the ocean and land arms reach the restriction; the base,
        // borehole, monolith and fungus arms jump past it to the event tail.
        if (crop > 2 && !has_tech(Rules.tech_three_nutrients_sqr, faction_id)
            && !has_nutrient_bonus && !has_condenser) {
            TileYieldRestricted += crop - 2;
            crop = 2;
        }
    }

    if (base_id >= 0) {
        uint32_t event = Bases[base_id].event;
        if (event & BEVENT_BUMPER) {
            crop++;
        }
        if ((event & BEVENT_FAMINE) && crop) {
            crop--;
        }
    }
    return crop;
}

/*
Purpose: Calculate the minerals produced by a single map square.
// ORIGINAL: 0x004E7310 ?mine_yield@@YAHHHHHH@Z 0x004E7310-0x004E7748
// LEVER: bit2 was cached to a named local and read once; the image reads
//        tile->bit2 fresh at each of the two use sites (code=bit2>>24, then
//        again for the landmark_bonus mask) because it does not survive the
//        bonus_at() call. Also is_ocean_tile was hoisted; the image computes
//        it lazily, right at its one `else if`, not up front. Both fixed:
//        agreeing instructions 2->5 of 355 (/Oy-), similarity 0.379->0.407.
// TRIED: the image needs ZERO stack (reuses the dead x/y/base_id/
//            assume_improved parameter slots as scratch throughout); this
//            tree's body still needs sub esp 0xc for ~8 live locals across
//            the branch chain. That reuse is a VC6 liveness heuristic, not
//            something reachable by reordering source further in the time
//            spent here.
// size      1080 bytes
// prototype int (__cdecl ?mine_yield@@YAHHHHHH@Z)(int factionID, int, int, int, int)
// callers   6   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0050BA00 0x00592030 0x005B9F20
// LEVER: yield_tile/yield_tile_owner (this file) marked `static __forceinline`
//        - shared with crop_yield/energy_yield/base_support, a clear win for
//        base_support (0.160 -> 0.371). Tied here (best-across-flags stays
//        0.388, won by an /Ob0 flag set where yield_tile already folded
//        regardless) - kept anyway since it does not regress.
// TRIED: a `bonus_at_call` non-inline forwarder (map.h/map.cpp, since
//            removed) so bonus_at(x, y, 0) would emit `call 0x592030` -
//            measured no improvement on top of the forceinline change.
// LEVER: FEWER (3 calls vs image's 6: 3 bitmask() + bonus_at + 2 has_tech) -
//        all three `has_fac_built(FAC_X, base_id)` sites constant-fold their
//        offset/mask math away here, unlike the image, which keeps
//        bitmask() out-of-line at every one. Switching all three to
//        `has_fac_built_call` took call_diff to 0 disagreeing with no
//        similarity regression (still 0.388 best-across-flags).
Return Value: Minerals from the square
Status: Complete

`assume_improved` stands in for a mine or borehole on land and a mining platform
at sea, and suppresses the roadless-mine mineral limit.
*/
int __cdecl mine_yield(int faction_id, int base_id, int x, int y,
                       BOOL assume_improved) {
    Map *tile = yield_tile(x, y);
    // Landmark sequence code, signed exactly as `sar edi, 0x18`. Bit 31 of bit2
    // is required clear by every test below, so the sign only decides branches
    // that are not taken. Read directly off tile->bit2, not a cached local:
    // the image re-reads bit2 from memory for the landmark_bonus mask below,
    // rather than keeping this value live across the bonus_at() call.
    int code = (int)tile->bit2 >> 24;
    uint32_t bit = tile->bit;
    // Bonus id 2 is the mineral bonus.
    BOOL has_mineral_bonus = bonus_at(x, y, 0) == 2;
    BOOL landmark_bonus =
        (((tile->bit2 & (BIT2_UNK_80000000 | BIT2_VOLCANO)) == BIT2_VOLCANO)
            && code < 9)
        || (((tile->bit2 & (BIT2_UNK_80000000 | BIT2_CRATER)) == BIT2_CRATER)
            && code < 9)
        || (((tile->bit2 & (BIT2_UNK_80000000 | BIT2_FOSSIL)) == BIT2_FOSSIL)
            && code < 6)
        || ((tile->bit2 & (BIT2_UNK_80000000 | BIT2_CANYON)) == BIT2_CANYON);
    int mineral = (has_mineral_bonus ? ResourceInfo[RSCINFO_BONUS_SQ].minerals
                                     : 0)
                  + (landmark_bonus ? 1 : 0);
    BOOL restrict_yield = false;

    if (yield_tile_owner(tile) >= 0) {
        mineral += ResourceInfo[RSCINFO_BASE_SQ].minerals;
        if (base_id < 0) {
            return mineral; // the original returns here rather than reaching the tail
        }
        if (has_fac_built_call(FAC_RECYCLING_TANKS, base_id)
            || has_fac_built_call(FAC_PRESSURE_DOME, base_id)) {
            mineral += ResourceInfo[RSCINFO_RECYCLING_TANKS].minerals;
        }
    } else if (bit & BIT_MONOLITH) {
        // Assignment, not addition: the bonus and landmark prefix is discarded.
        mineral = ResourceInfo[RSCINFO_MONOLITH].minerals;
        if (has_mineral_bonus) {
            mineral += ResourceInfo[RSCINFO_BONUS_SQ].minerals;
        }
        if (has_project(SP_MANIFOLD_HARMONICS, faction_id)
            && PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::PLANET] > 1) {
            mineral++;
        }
    } else if (bit & BIT_THERMAL_BORE) {
        mineral = ResourceInfo[RSCINFO_BOREHOLE_SQ].minerals;
        if (has_mineral_bonus) {
            mineral += ResourceInfo[RSCINFO_BONUS_SQ].minerals;
        }
        // Unlike crop_yield's borehole arm, this one does reach the restriction.
        restrict_yield = true;
    } else if ((bit & BIT_FUNGUS)
               && (tile->climate & 0xE0) >= ALT_BIT_OCEAN_SHELF) {
        // Note there is no assume_improved suppressor here; crop_yield has one.
        int planet = PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::PLANET];
        int planet_mod = (planet < -3) ? -3 : ((planet > 0) ? 0 : planet);
        mineral = range(PlayersData[faction_id].tech_fungus_mineral + planet_mod,
                        0, 99);
        if (has_project(SP_MANIFOLD_HARMONICS, faction_id) && planet > 1) {
            mineral++;
        }
    } else if ((tile->climate & 0xE0) < ALT_BIT_SHORE_LINE) {
        // Not hoisted: the image computes this test lazily, right here, rather
        // than caching it in a local up front (unlike crop_yield/energy_yield,
        // which read climate for is_ocean_tile before this branch chain).
        restrict_yield = true;
        mineral += ResourceInfo[RSCINFO_OCEAN_SQ].minerals;
        // Legacy: the aquatic bonus keys off the faction owning BASE_ID, not off
        // faction_id, and is unguarded for base_id < 0 (0x004E75E2).
        int base_owner = Bases[base_id].faction_id_current;
        if ((Players[base_owner].rule_flags & RFLAG_AQUATIC)
            && (tile->climate & 0xE0) == ALT_BIT_OCEAN_SHELF) {
            mineral++;
        }
        if ((tile->climate & 0xE0) == ALT_BIT_OCEAN_SHELF || ExpansionEnabled) {
            if ((bit & BIT_MINE) || assume_improved) {
                mineral += ResourceInfo[RSCINFO_IMPROVED_SEA].minerals;
                if (has_tech(Rules.tech_mining_platform_bonus, faction_id)) {
                    mineral++;
                }
                // Legacy: unguarded for base_id < 0, exactly as 0x004E765F.
                if (has_fac_built_call(FAC_SUBSEA_TRUNKLINE, (uint32_t)base_id)) {
                    mineral++;
                }
            }
        }
    } else {
        restrict_yield = true;
        int rockiness = tile->val3 >> 6;
        if (bit & BIT_FOREST) {
            mineral += ResourceInfo[RSCINFO_FOREST_SQ].minerals;
        } else if ((bit & (BIT_MINE | BIT_THERMAL_BORE)) || assume_improved) {
            mineral += rockiness;
            int extra = rockiness ? rockiness : 1;
            if (has_mineral_bonus || landmark_bonus) {
                extra++;
            }
            if (!(bit & BIT_ROAD) && !assume_improved) {
                int limit = (int)Rules.limit_mineral_mine_sans_road;
                if (extra > limit) {
                    TileYieldRestricted += extra - limit;
                    extra = limit;
                }
            }
            mineral += extra;
        } else if (rockiness) {
            mineral++;
        }
    }

    if (restrict_yield && mineral > 2
        && !has_tech(Rules.tech_three_minerals_sqr, faction_id)
        && !has_mineral_bonus) {
        TileYieldRestricted += mineral - 2;
        mineral = 2;
    }

    if (base_id >= 0) {
        uint32_t event = Bases[base_id].event;
        if (event & BEVENT_INDUSTRY) {
            mineral++;
        }
        if ((event & BEVENT_BUST) && mineral) {
            mineral--;
        }
    }
    return mineral;
}

/*
Purpose: Calculate the energy produced by a single map square.
// ORIGINAL: 0x004E7750 ?energy_yield@@YAHHHHHH@Z 0x004E7750-0x004E7DA3
// LEVER: same bit2 shape as mine_yield - cached to a named local, but the
//        image re-reads tile->bit2 from memory for the landmark_bonus mask
//        (the register holding it was already destroyed by the >>24 shift
//        for `code`, and does not survive the bonus_at() call either way).
//        Fixed: agreeing instructions 4->5 of 536 (/Oy-), and `sub esp,0x18`
//        now matches the image exactly (both sides, 6 stack-resident
//        locals) - the frame allocation itself is no longer the gap here.
// TRIED: the remaining divergence (instr 3) is prologue INSTRUCTION
//            SCHEDULING - the image multiplies width by a direct memory
//            operand on y (`imul eax, [ebp+0x14]`) and only loads y into a
//            register later, right before the bonus_at() call; this tree's
//            body loads y into a register immediately. Not reachable from
//            C source order in the time spent; it is a backend scheduling
//            choice, not a statement-order one.
// size      1619 bytes
// prototype int (__cdecl ?energy_yield@@YAHHHHHH@Z)(int factionID, int, int, int, int)
// callers   8   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0050BA00 0x00592030 0x005B9F20
// LEVER: yield_tile/yield_tile_owner (this file) marked `static __forceinline`
//        - shared with crop_yield/mine_yield/base_support, a clear win for
//        base_support (0.160 -> 0.371). Tied here (best-across-flags stays
//        0.137, won by an /Ob0 flag set where yield_tile already folded
//        regardless) - kept anyway since it does not regress.
// TRIED: a `bonus_at_call` non-inline forwarder (map.h/map.cpp, since
//            removed) so bonus_at(x, y, 0) would emit `call 0x592030` -
//            measured no improvement on top of the forceinline change.
// LEVER: FEWER (2 calls vs image's 8: 6 bitmask() + bonus_at + has_tech) -
//        all six `has_fac_built(FAC_X, base_id)` sites constant-fold their
//        offset/mask math away under this toolchain, unlike the image, which
//        keeps bitmask() out-of-line at every one. Switching all six to
//        `has_fac_built_call` (base.h's bitmask_call-forcing sibling) took
//        call_diff to 0 disagreeing with no similarity regression (still
//        0.137 best-across-flags, matching the tied score noted above).
Return Value: Energy from the square
Status: Complete

`assume_improved` stands in for a solar collector on land and a tidal harness at
sea.

The fungus arm deliberately skips the whole shared tail - river, resource bonus,
landmark, Merchant Exchange, economy and the clip to 2 - by jumping from
0x004E7AD2 straight to the base event tail at 0x004E7D4B. That is what the bytes
do, not a transcription slip.
*/
int __cdecl energy_yield(int faction_id, int base_id, int x, int y,
                         BOOL assume_improved) {
    Map *tile = yield_tile(x, y);
    uint32_t bit = tile->bit;
    // Not cached: bit2 is read directly off tile->bit2 at both use sites
    // below (here, shifted for `code`, and again in landmark_bonus). The
    // image re-reads it from memory the second time rather than keeping the
    // pre-shift value live across the bonus_at() call - same shape as
    // mine_yield's borehole/landmark computation.
    int code = (int)tile->bit2 >> 24;
    BOOL is_ocean_tile = (tile->climate & 0xE0) < ALT_BIT_SHORE_LINE;
    BOOL has_base_in_tile = yield_tile_owner(tile) >= 0;
    // Bonus id 3 is the energy bonus.
    BOOL has_energy_bonus = bonus_at(x, y, 0) == 3;
    BOOL landmark_bonus =
        (((tile->bit2 & (BIT2_UNK_80000000 | BIT2_VOLCANO)) == BIT2_VOLCANO)
            && code < 9)
        || ((tile->bit2 & (BIT2_UNK_80000000 | BIT2_URANIUM)) == BIT2_URANIUM)
        || ((tile->bit2 & (BIT2_UNK_80000000 | BIT2_GEOTHERMAL)) == BIT2_GEOTHERMAL)
        || ((tile->bit2 & (BIT2_UNK_80000000 | BIT2_RIDGE)) == BIT2_RIDGE);
    int energy = 0;
    BOOL skip_shared_tail = false;

    if (has_base_in_tile) {
        energy = ResourceInfo[RSCINFO_BASE_SQ].energy;
        // Legacy: read BEFORE the base_id sign check at 0x004E7896, so a base_id
        // of -1 reads one Base entry below the array, exactly as the original.
        if (has_fac_built_call(FAC_HEADQUARTERS, (uint32_t)base_id)) {
            energy++;
        }
        int golden_age = 0;
        if (base_id >= 0) {
            if (has_fac_built_call(FAC_RECYCLING_TANKS, base_id)
                || has_fac_built_call(FAC_PRESSURE_DOME, base_id)) {
                energy += ResourceInfo[RSCINFO_RECYCLING_TANKS].energy;
            }
            if (Bases[base_id].state & BSTATE_GOLDEN_AGE_ACTIVE) {
                golden_age = 1;
            }
        }
        int economy = PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::ECONOMY]
                      + golden_age;
        if (economy < 0) {
            // The original asks for Headquarters again here rather than reusing
            // the answer above.
            if (!has_fac_built_call(FAC_HEADQUARTERS, (uint32_t)base_id)
                || economy < -1
                || PlayersData[faction_id].current_num_bases == 1u) {
                economy++;
            }
        } else if (economy > 1) {
            if (economy > 4) {
                economy = 4;
            }
            economy = economy * 2 - 4;
        }
        energy += economy;
        if (GameDataStateStorage.fields_.governor_faction_ == faction_id) {
            energy++;
        }
    } else if (bit & BIT_MONOLITH) {
        energy = ResourceInfo[RSCINFO_MONOLITH].energy;
        if (has_project(SP_MANIFOLD_HARMONICS, faction_id)) {
            int planet = PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::PLANET];
            if (planet >= 0) {
                energy++;
            }
            if (planet >= 3) {
                energy++;
            }
        }
    } else if (bit & BIT_THERMAL_BORE) {
        energy = ResourceInfo[RSCINFO_BOREHOLE_SQ].energy;
    } else if ((bit & BIT_FUNGUS)
               && (tile->climate & 0xE0) >= ALT_BIT_OCEAN_SHELF) {
        int planet = PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::PLANET];
        int planet_mod = (planet < -3) ? -3 : ((planet > 0) ? 0 : planet);
        energy = range(PlayersData[faction_id].tech_fungus_energy + planet_mod,
                       0, 99);
        if (has_project(SP_MANIFOLD_HARMONICS, faction_id)) {
            if (planet >= 0) {
                energy++;
            }
            if (planet >= 3) {
                energy++;
            }
        }
        skip_shared_tail = true;
    } else if (is_ocean_tile) {
        if ((tile->climate & 0xE0) == ALT_BIT_OCEAN_SHELF || ExpansionEnabled) {
            energy = ResourceInfo[RSCINFO_OCEAN_SQ].energy;
            if ((bit & BIT_SOLAR_TIDAL) || assume_improved) {
                energy += ResourceInfo[RSCINFO_IMPROVED_SEA].energy;
                // Legacy: unguarded for base_id < 0, exactly as 0x004E7B19.
                if (has_fac_built_call(FAC_THERMOCLINE_TRANSDUCER,
                                  (uint32_t)base_id)) {
                    energy++;
                }
            }
        }
        if (bit & BIT_FOREST) {
            energy--;
        }
    } else if (bit & BIT_FOREST) {
        energy = ResourceInfo[RSCINFO_FOREST_SQ].energy;
        if (base_id >= 0 && has_fac_built_call(FAC_HYBRID_FOREST, base_id)) {
            energy++;
        }
    } else {
        // Altitude levels above 2.
        int solar = (tile->climate >> 5) - 2;
        if (solar < 0) {
            solar = 0;
        }
        if ((bit & (BIT_SOLAR_TIDAL | BIT_ECH_MIRROR | BIT_THERMAL_BORE))
            || assume_improved) {
            energy = solar;
        }
        if (bit & BIT_SOLAR_TIDAL) {
            // Each adjacent echelon mirror belonging to the current base's
            // faction reflects one more energy onto this collector. The x wrap
            // is xrange() inlined, except that the original masks MapIsFlat with
            // 1 rather than testing it for nonzero.
            BOOL is_flat = MapIsFlat & 1;
            int longitude_bounds = MapLongitudeBounds;
            for (int i = 0; i < 8; i++) {
                int x_adj = x + RadiusBaseX[i];
                if (!is_flat) {
                    if (x_adj < 0) {
                        x_adj += longitude_bounds;
                    } else if (x_adj >= longitude_bounds) {
                        x_adj -= longitude_bounds;
                    }
                }
                int y_adj = y + RadiusBaseY[i];
                if (y_adj < 0 || y_adj >= MapLatitudeBounds) {
                    continue;
                }
                if (x_adj < 0 || x_adj >= longitude_bounds) {
                    continue;
                }
                Map *adjacent = yield_tile(x_adj, y_adj);
                if (!(adjacent->bit & BIT_ECH_MIRROR)) {
                    continue;
                }
                if (adjacent->territory
                    != (int)BaseCurrent->faction_id_current) {
                    continue;
                }
                energy++;
            }
        }
    }

    if (!skip_shared_tail) {
        if ((bit & BIT_RIVER) && !is_ocean_tile) {
            energy++;
        }
        if (has_energy_bonus) {
            energy += ResourceInfo[RSCINFO_BONUS_SQ].energy;
        }
        if (landmark_bonus) {
            energy++;
        }
        int golden_age = 0;
        if (base_id >= 0) {
            if (GameDataStateStorage.fields_.secret_project_.merchant_exchange() == base_id) {
                energy++;
            }
            if (Bases[base_id].state & BSTATE_GOLDEN_AGE_ACTIVE) {
                golden_age = 1;
            }
        }
        if (PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::ECONOMY] + golden_age
            >= 2) {
            energy++;
        }
        if (has_base_in_tile) {
            energy = range(energy, 0, 99);
            BaseSquareEnergy = energy;
        } else if (energy > 2
                   && !has_tech(Rules.tech_three_energy_sqr, faction_id)
                   && !has_energy_bonus) {
            TileYieldRestricted += energy - 2;
            energy = 2;
        }
    }

    if (base_id >= 0) {
        uint32_t event = Bases[base_id].event;
        if (event & BEVENT_HEAT_WAVE) {
            energy++;
        }
        if ((event & BEVENT_CLOUD_COVER) && energy) {
            energy--;
        }
    }
    if (GameDataStateStorage.fields_.dust_cloud_duration_ && energy) {
        energy--;
    }
    int event_active = UnkGlobal0093A934 ? (GameDataStateStorage.fields_.global_energy_event_state_ & 2)
                                          : (GameDataStateStorage.fields_.global_energy_event_state_ & 1);
    return event_active ? energy * 3 : energy;
}

/*
Purpose: Tally what the current base's forces cost it: the resources its supply
         convoys move in and out, the units it supports, the minerals their
         maintenance takes, and the pacifism drones they cause.
// ORIGINAL: 0x004E9550 ?base_support@@YAXXZ 0x004E9550-0x004E9B4A
// LEVER: the FAC_BROOD_PIT police-modifier check was `has_fac_built(...)`,
//        which constant-folds to a bit test here (same systemic gap as
//        has_fac/base_nutrient/base_minerals - see their notes). Swapping it
//        for `has_fac_built_call(...)` (base.h, forces bitmask_call's real
//        E8) took call_diff from FEWER by 2 to FEWER by 1 and best-across-
//        flags similarity 0.371 -> 0.431 (45/465 raw agreeing, /O2 /Gy /GR- /GX).
// TRIED: hoisting `Base *base_current = BaseCurrent();` above the loop -
//            already ruled out for a sibling function a few hundred lines up
//            in this file (see the TRIED note near line 396): the image
//            re-reads the BaseCurrent() global fresh at every use, so this
//            was not retried here. Under /Oy-, `sub esp` is 0x34 against the
//            image's 0x3c (one 8-byte gap, i.e. two stack slots) - closer
//            than the auto-picked flag set's 0x2c, but no specific local was
//            identified as the source of the gap in the time spent on a
//            465-instruction body with ~20 live locals across the veh loop.
// size      1530 bytes
// prototype
// callers   7   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3EF0 0x004E6E50 0x004E7310 0x004E7750 0x0050BA00 0x005BF1F0
// LEVER: unrolled the `for (rsc=0..3) { ConvoyTo[rsc]=0; ConvoyFrom[rsc]=0; }`
//        loop into eight explicit assignments, keeping the pair order
//        (ConvoyTo before ConvoyFrom) the loop already used - the image
//        writes [0x90e93c] before [0x90e904], [0x90e940] before [0x90e908],
//        and so on, fully unrolled. Combined with yield_tile/yield_tile_owner
//        becoming `static __forceinline` (this file, shared with
//        crop_yield/mine_yield/energy_yield), best similarity moved 0.129 ->
//        0.371 and agreeing instructions 9->14 of 465.
Return Value: n/a
Status: Complete

One pass over every Veh of the base's faction, splitting on whether the unit is
homed here. The five accumulators this zeroes are read by base_nutrient
(0x004E9B70), base_minerals (0x004E9CB0) and base_psych, which is why the whole
tally has to be finished before any of them run.

Two of the eight `free_support` entries are not `support + 2`: -4 and -3 both
give nothing, and +3 gives max(4, population), so the table is built rather than
computed. It is indexed by the SUPPORT rating offset by four, -4 at [0].

The original recomputes the Brood Pit police adjustment a second time in the
`police != -3` arm, with a second inlined bitmask() over the same facility and
the same base. Nothing between the two writes facilities_built, so it is written
once here. The three has_abil(ABL_CLEAN_REACTOR) calls are NOT collapsed: the
first one gates the whole support section, so the second and third can only ever
see false, but has_abil() logs on a negative prototype id and the call count is
therefore observable.
*/
void __cdecl base_support() {
    int faction_id = BaseCurrent->faction_id_current;
    // Unrolled (image writes each ConvoyTo/ConvoyFrom pair explicitly, not
    // as a loop): [0x90e93c] before [0x90e904], then [0x90e940]/[0x90e908],
    // and so on.
    BaseCurrentConvoyTo[0] = 0;
    BaseCurrentConvoyFrom[0] = 0;
    BaseCurrentConvoyTo[1] = 0;
    BaseCurrentConvoyFrom[1] = 0;
    BaseCurrentConvoyTo[2] = 0;
    BaseCurrentConvoyFrom[2] = 0;
    BaseCurrentConvoyTo[3] = 0;
    BaseCurrentConvoyFrom[3] = 0;
    BaseCurrentVehPacifismCount = 0;
    BaseCurrentForcesMaintCount = 0;
    BaseCurrentForcesSupported = 0;
    BaseCurrentForcesMaintCost = 0;
    int free_support[8]; // units supported for free, by SUPPORT rating + 4
    for (int support = -4; support <= 3; support++) {
        int free_units;
        if (support >= 3) {
            int pop_size = BaseCurrent->population_size;
            free_units = (pop_size > 4) ? pop_size : 4;
        } else if (support < -2) {
            free_units = 0;
        } else if (support == -2) {
            free_units = 1;
        } else {
            free_units = support + 2;
        }
        free_support[support + 4] = free_units;
    }
    for (int veh_id = 0; veh_id < GameDataStateStorage.fields_.vehicle_current_count_; veh_id++) {
        Veh *veh = &Vehs[veh_id];
        if (veh->faction_id != faction_id) {
            continue;
        }
        int proto_id = veh->proto_id;
        if (veh->home_base_id != BaseIDCurrentSelected) {
            /*
            * A convoy homed at another base but standing on this base's own
            * tile counts one, not a yield; base_nutrient() adds
            * BaseCurrentConvoyTo to the base's intake all the same.
            */
            if (VehPrototypes[proto_id].plan == PLAN_SUPPLY_CONVOY
                && veh->x == BaseCurrent->x && veh->y == BaseCurrent->y
                && veh->order == ORDER_CONVOY && veh->home_base_id >= 0) {
                BaseCurrentConvoyTo[veh->order_auto_type]++;
            }
            continue;
        }
        if (VehPrototypes[proto_id].plan == PLAN_SUPPLY_CONVOY
            && yield_tile_owner(yield_tile(veh->x, veh->y)) < 0
            && veh->order == ORDER_CONVOY) {
            int convoyed;
            switch (veh->order_auto_type) {
              case RSC_NUTRIENTS:
                convoyed = crop_yield(faction_id, BaseIDCurrentSelected, veh->x, veh->y,
                                      false);
                break;
              case RSC_MINERALS:
                convoyed = mine_yield(faction_id, BaseIDCurrentSelected, veh->x, veh->y,
                                      false);
                break;
              case RSC_ENERGY:
                convoyed = energy_yield(faction_id, BaseIDCurrentSelected, veh->x, veh->y,
                                        false);
                break;
              case RSC_PSI:
                TileYieldRestricted = 0;
                convoyed = 0;
                break;
              default:
                convoyed = 0;
                break;
            }
            BaseCurrentConvoyTo[veh->order_auto_type] += convoyed;
        }
        /*
        * The AI's offensive strength tally. A negative offense rating is a psi
        * weapon: those count only on a custom prototype, never on one of the
        * predefined native life forms.
        */
        int offense = Weapon[VehPrototypes[proto_id].weapon_id].offense_rating;
        if (offense && (offense > 0 || proto_id >= MaxVehProtoFactionNum)) {
            uint32_t tally_plan = VehPrototypes[proto_id].plan;
            if (tally_plan != PLAN_RECONNAISANCE && tally_plan != PLAN_PLANET_BUSTER) {
                PlayersData[faction_id].unk_48++;
                if (tally_plan == PLAN_OFFENSIVE || tally_plan == PLAN_COMBAT) {
                    PlayersData[faction_id].unk_48++;
                }
            }
        }
        veh->state &= ~(VSTATE_REQUIRES_SUPPORT | VSTATE_PACIFISM_DRONE
                        | VSTATE_PACIFISM_FREE_SKIP);
        uint32_t plan = VehPrototypes[proto_id].plan;
        // A convoy of ours standing in any base is charged as consumption.
        if (plan == PLAN_SUPPLY_CONVOY && veh->order == ORDER_CONVOY
            && yield_tile_owner(yield_tile(veh->x, veh->y)) >= 0) {
            BaseCurrentConvoyFrom[veh->order_auto_type]++;
        }
        if (plan > PLAN_TERRAFORMING) {
            continue; // convoys and above are never supported
        }
        if (!has_abil(proto_id, ABL_CLEAN_REACTOR)) {
            /*
            * Native life in fungus is supported for free. The prototype test is
            * a psi weapon on a predefined prototype, or prototype 15 - the Sea
            * Lurk, whose weapon is not psi.
            */
            BOOL free_in_fungus = false;
            if (proto_id < MaxVehProtoFactionNum) {
                if (Weapon[VehPrototypes[proto_id].weapon_id].offense_rating < 0
                    || proto_id == 15) {
                    Map *tile = yield_tile(veh->x, veh->y);
                    if ((tile->bit & BIT_FUNGUS)
                        && (tile->climate & 0xE0) >= ALT_BIT_OCEAN_SHELF) {
                        free_in_fungus = true;
                    }
                }
            }
            if (!free_in_fungus) {
                (BaseCurrentForcesSupported)++;
                int support = PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::SUPPORT];
                if ((int)BaseCurrentForcesSupported
                        > free_support[range(support + 4, 0, 7)]
                    && !has_abil(proto_id, ABL_CLEAN_REACTOR)) {
                    (BaseCurrentForcesMaintCount)++;
                    veh->state |= VSTATE_REQUIRES_SUPPORT;
                    BaseCurrentForcesMaintCost += (support <= -4) ? 2 : 1;
                }
            }
            /*
            * What this unit would cost at each of the eight SUPPORT ratings,
            * for social_ai() to price a social engineering change with. The
            * marginal cost triples once it passes the base's mineral intake and
            * doubles once it passes half of it.
            */
            if (BaseUpkeepStage == 1) {
                for (int support = -4; support <= 3; support++) {
                    if ((int)BaseCurrentForcesSupported <= free_support[support + 4]) {
                        continue;
                    }
                    int cost = has_abil(proto_id, ABL_CLEAN_REACTOR)
                        ? 0 : ((support <= -4) ? 2 : 1);
                    int minerals = BaseCurrent->mineral_intake_2;
                    int half_minerals = minerals / 2;
                    if (cost >= minerals) {
                        cost += (cost - half_minerals) * 2;
                    } else if (cost > half_minerals) {
                        cost = cost * 2 - half_minerals;
                    }
                    PlayersData[faction_id].unk_38[support + 4] += cost;
                }
            }
        }
        /*
        * Pacifism. Only armed units count, and only away from home - except for
        * air units, which count anywhere unless they are interceptors.
        */
        if (!Weapon[VehPrototypes[proto_id].weapon_id].offense_rating) {
            continue;
        }
        if (yield_tile_owner(yield_tile(veh->x, veh->y)) >= 0
            || whose_territory(faction_id, veh->x, veh->y, NULL, false) == faction_id) {
            if (Chassis[VehPrototypes[proto_id].chassis_id].triad != TRIAD_AIR
                || VehPrototypes[proto_id].plan == PLAN_AIR_SUPERIORITY) {
                continue;
            }
        }
        (BaseCurrentVehPacifismCount)++;
        int police = PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::POLICE]
            + (has_fac_built_call(FAC_BROOD_PIT, BaseIDCurrentSelected) ? 2 : 0);
        if (police == -3) {
            veh->state |= (BaseCurrentVehPacifismCount == 1) ? VSTATE_PACIFISM_FREE_SKIP
                                                              : VSTATE_PACIFISM_DRONE;
        } else if (police < -3) {
            veh->state |= VSTATE_PACIFISM_DRONE;
        }
    }
}

/*
Purpose: Calculate nutrients and growth for the current base.
// ORIGINAL: 0x004E9B70 ?base_nutrient@@YAXXZ 0x004E9B70-0x004E9CAD
// LEVER: the image's single call is bitmask() with two ebp-relative
//   out-params (the has_fac_built(FAC_CHILDREN_CRECHE,...) shape - an
//   ebp-based frame exists ONLY to hold those two locals). Under this
//   tree's toolchain, has_fac_built(literal, base_id) always constant-folds
//   the offset/mask math to a direct `test byte ptr [...], imm` with no
//   call and no frame at all (same defect as base_minerals's
//   CENTAURI_PRESERVE/TEMPLE_OF_PLANET checks - see its note). has_fac_built_call
//   (base.h, forces bitmask_call's real E8 the same way breed_mod's checks
//   do) is the fix: swapping just this one site took call_diff to 0
//   disagreeing (was FEWER by 1) and best-across-flags similarity 0.790 ->
//   0.951 (10/93 raw agreeing, /O2 /Gy /GR- /Oy- /GX). Residual is the same
//   neg/sbb/neg 0-or-1 materialization plateau documented on is_objective
//   (0x005AC060) - the image boolifies the facilities_built bit test before
//   its `test eax,eax`, this tree emits a plain `test byte ptr [...]`.
// TRIED: `has_fac_built_call`'s own return type is already `int` (see
//   0x00421670's LEVER note); assigning its result to a named `BOOL` local
//   before the `if` here made no difference either - VC6 still proves the
//   value feeds only one branch and drops the materialization. Same
//   plateau, confirmed a second way.
// size      317 bytes
// prototype
// callers   7   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0050BA00
Return Value: n/a
Status: Complete
*/
void __cdecl base_nutrient() {
    uint32_t faction_id = BaseCurrent->faction_id_current;
    BaseCurrentGrowthRate = PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::GROWTH];
    if (has_fac_built_call(FAC_CHILDREN_CRECHE, BaseIDCurrentSelected)) {
        BaseCurrentGrowthRate += 2; // +2 on growth scale
    }
    if (BaseCurrent->state & BSTATE_GOLDEN_AGE_ACTIVE) {
        BaseCurrentGrowthRate += 2;
    }
    BaseCurrent->nutrient_intake_2 += BaseCurrentConvoyTo[RSC_NUTRIENTS];
    BaseCurrent->nutrient_consumption = BaseCurrentConvoyFrom[RSC_NUTRIENTS]
        + BaseCurrent->population_size * Rules.nutrient_req_citizen;
    BaseCurrent->nutrient_surplus = BaseCurrent->nutrient_intake_2
        - BaseCurrent->nutrient_consumption;
    if (BaseCurrent->nutrient_surplus >= 0) {
        if (BaseCurrent->nutrients_accumulated < 0) {
            BaseCurrent->nutrients_accumulated = 0;
        }
    } else if (!(BaseCurrent->nutrients_accumulated)) {
        BaseCurrent->nutrients_accumulated = -1;
    }
    if (BaseUpkeepStage == 1) {
        PlayersData[faction_id].nutrient_surplus_total
            += range(BaseCurrent->nutrient_surplus, 0, 99);
    }
}

/*
Purpose: Calculate minerals and ecological damage for the current base.
// ORIGINAL: 0x004E9CB0 ?base_minerals@@YAXXZ 0x004E9CB0-0x004EA1EF
// LEVER: the image calls has_fac(facility,base,0) - not has_fac_built - for
//   5 of 7 facility checks (QUANTUM_CONVERTER, ROBOTIC_ASSEMBLY_PLANT,
//   GENEJACK_FACTORY, both NANOREPLICATOR checks); switching those 5 from
//   has_fac_built(...) to has_fac(...,0) took the call count from 1 to 6 of 7.
// LEVER: CENTAURI_PRESERVE/TEMPLE_OF_PLANET were short one real bitmask()
//   call each (6 vs 7) because has_fac_built(literal,...) fully constant-folds
//   under this tree's VC6 (offset/mask collapse to a direct bit test, no
//   call at all) instead of leaving bitmask() out-of-line as the image does;
//   an explicit local `int offset,mask; bitmask(...)` folds identically, and
//   routing them through has_fac(literal,base,0) instead (matching the other
//   5) makes it WORSE: all 7 become real has_fac calls and a downstream
//   ascending() call that was inlined stops being inlined (8 total, MORE).
//   has_fac_built_call (base.h, forces bitmask_call's real E8 the same way
//   breed_mod's four checks do) is the shape that matches: switching just
//   these two took call_diff to 0 disagreeing (was FEWER by 2), for a
//   negligible similarity cost (0.239 -> 0.232 best-across-flags) - the
//   correct call graph, not the highest score, so kept.
// size      1343 bytes
// prototype
// callers   7   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00421670 0x0050BA00
Return Value: n/a
Status: Complete
*/
void __cdecl base_minerals() {
    uint32_t faction_id = BaseCurrent->faction_id_current;
    BaseCurrent->mineral_intake_2 += BaseCurrentConvoyTo[RSC_MINERALS];
    uint32_t mineral_bonus = (has_fac(FAC_QUANTUM_CONVERTER, BaseIDCurrentSelected, 0)
        || has_project(SP_SINGULARITY_INDUCTOR, faction_id)) ? 1 : 0;
    if (has_fac(FAC_ROBOTIC_ASSEMBLY_PLANT, BaseIDCurrentSelected, 0)) {
        mineral_bonus++;
    }
    if (has_fac(FAC_GENEJACK_FACTORY, BaseIDCurrentSelected, 0)) {
        mineral_bonus++;
    }
    if (has_fac(FAC_NANOREPLICATOR, BaseIDCurrentSelected, 0)) {
        mineral_bonus++;
    }
    if (has_project(SP_BULK_MATTER_TRANSMITTER, faction_id)) {
        mineral_bonus++;
    }
    BaseCurrent->mineral_intake_2 = (BaseCurrent->mineral_intake_2 * (mineral_bonus + 2)) / 2;
    BaseCurrent->mineral_consumption = BaseCurrentForcesMaintCost
        + BaseCurrentConvoyFrom[RSC_MINERALS];
    BaseCurrent->mineral_surplus = BaseCurrent->mineral_intake_2
        - BaseCurrent->mineral_consumption;
    BaseCurrent->mineral_inefficiency = 0; // ?
    BaseCurrent->mineral_surplus -= BaseCurrent->mineral_inefficiency; // ?
    BaseCurrent->mineral_surplus_final = BaseCurrent->mineral_surplus;
    BaseCurrent->eco_damage /= 8;
    int planet_eco_factor = PlayersData[faction_id].planet_ecology + 16;
    if (BaseCurrent->eco_damage > 0) {
        int planet_modifier = BaseCurrent->eco_damage;
        if (BaseCurrent->eco_damage >= planet_eco_factor) {
            planet_modifier = planet_eco_factor;
        }
        planet_eco_factor -= planet_modifier;
        BaseCurrent->eco_damage -= planet_modifier;
    }
    int eco_dmg_reduction = (has_fac(FAC_NANOREPLICATOR, BaseIDCurrentSelected, 0)
        || has_project(SP_SINGULARITY_INDUCTOR, faction_id)) ? 2 : 1;
    if (has_fac_built_call(FAC_CENTAURI_PRESERVE, BaseIDCurrentSelected)) {
        eco_dmg_reduction++;
    }
    if (has_fac_built_call(FAC_TEMPLE_OF_PLANET, BaseIDCurrentSelected)) {
        eco_dmg_reduction++;
    }
    if (has_project(SP_PHOLUS_MUTAGEN, faction_id)) {
        eco_dmg_reduction++;
    }
    BaseCurrent->eco_damage += (BaseCurrent->mineral_intake_2 - planet_eco_factor
        - range(PlayersData[faction_id].satellites_[1], 0, BaseCurrent->population_size))
        / eco_dmg_reduction;
    if (is_human(faction_id)) {
        BaseCurrent->eco_damage += ((PlayersData[faction_id].major_atrocities
            + TectonicDetonationCount[faction_id]) * 5) / (range(MapSeaLevel, 0, 100)
                / range(WorldBuilder.sea_level_rises, 1, 100) + eco_dmg_reduction);
    }
    if (BaseCurrent->eco_damage < 0) {
        BaseCurrent->eco_damage = 0;
    }
    if (ascending(faction_id) && GameDataStateStorage.fields_.game_rules_ & RULES_VICTORY_TRANSCENDENCE) {
        BaseCurrent->eco_damage *= 2;
    }
    if (GameDataStateStorage.fields_.game_state_ & STATE_PERIHELION_ACTIVE) {
        BaseCurrent->eco_damage *= 2;
    }
    uint32_t diff_factor;
    if (is_human(faction_id)) {
        int diff_lvl = PlayersData[faction_id].diff_level;
        diff_factor = !diff_lvl ? DLVL_TALENT
            : ((diff_lvl <= DLVL_LIBRARIAN) ? DLVL_LIBRARIAN : DLVL_TRANSCEND);
    } else {
        diff_factor = range(6 - GameDataStateStorage.fields_.diff_level_current_, DLVL_SPECIALIST, DLVL_LIBRARIAN);
    }
    BaseCurrent->eco_damage = ((PlayersData[faction_id].tech_ranking
        - PlayersData[faction_id].theory_of_everything)
        * (3 - range(PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::PLANET], -3, 2))
        * (MapNativeLifeForms + 1) * BaseCurrent->eco_damage * diff_factor) / 6;
    BaseCurrent->eco_damage = (BaseCurrent->eco_damage + 50) / 100;
    int queue_id;
    if (has_project(SP_SPACE_ELEVATOR, faction_id) // orbital facilities double mineral prod rate
        && (queue_id = BaseCurrent->queue_production_id[0], queue_id == -FAC_SKY_HYDRO_LAB
            || queue_id == -FAC_NESSUS_MINING_STATION || queue_id == -FAC_ORBITAL_POWER_TRANS
            || queue_id == -FAC_ORBITAL_DEFENSE_POD)) {
        BaseCurrent->mineral_intake_2 *= 2;
        BaseCurrent->mineral_surplus = // doesn't update mineral_surplus_final?
            BaseCurrent->mineral_intake_2 - BaseCurrent->mineral_consumption;
    }
}

/*
Purpose: Calculate the current base's energy loss/inefficiency for an amount of energy.
// ORIGINAL: 0x004EA1F0 ?black_market@@YAHH@Z 0x004EA1F0-0x004EA495
// LEVER: the HQ-distance check called vector_dist(4 args) as a real
//   function; the image never calls it - it open-codes x_dist()+abs()x3,
//   same shape as base_find (0x004E3D50), AND re-reads
//   (BaseCurrent())->x/y fresh inside the loop rather than hoisting them
//   to locals before it. Both fixed together: agreeing went 4/233 -> 8/233.
// TRIED: the remaining gap is the same has_fac_built(literal,...)
//   systemic issue documented on has_fac (0x00421670) and base_minerals
//   (0x004E9CB0) - the image keeps a real bitmask() call for
//   FAC_HEADQUARTERS (inside the loop) and FAC_CHILDREN_CRECHE even though
//   both facility ids are literal; this tree's toolchain always folds
//   them. Not re-derived further here; still a 0x1c vs 0xc stack-size gap
//   (the two ebp-relative bitmask out-params never materialise).
// TRIED (measured): swapping both of those has_fac_built() calls for
//   has_fac_built_call() (the general.h bitmask_call forwarder, base.h) to
//   force the real E8 the image emits. This function's best-scoring flag
//   set has no /Ob0 - plain /O2 implies /Ob2 auto-inline, which folds the
//   supposedly-non-inline bitmask_call() forwarder away too (it is a
//   trivial one-liner, exactly what /Ob2 auto-inlines regardless of the
//   `inline` keyword), so call_diff still reports the same wrong-callee
//   shape under a different name and the byte match gets WORSE across every
//   flag set: best similarity 0.772 (/Oi- /Gy /GR- /Oy- /GX) -> 0.608 (best
//   moves to /O1 /Gy /GR- /Oy- /GX). Reverted to has_fac_built().
// symbol    ?black_market@@YAIH@Z
// size      677 bytes
// prototype
// callers   1   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0050BA00 0x00644F3A
Return Value: Amount of energy inefficiency
Status: Complete
*/
uint32_t __cdecl black_market(int energy) {
    if (energy <= 0) {
        return 0;
    }
    uint32_t faction_id = BaseCurrent->faction_id_current;
    int dist_hq = 999;
    for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) { // modified version of vulnerable()
        if (Bases[i].faction_id_current == faction_id && has_fac_built_call(FAC_HEADQUARTERS, i)) {
            // Open-coded vector_dist(x,y,a,b): the image calls abs() four
            // times, never vector_dist itself - see base_find (0x004E3D50)
            // for the same expansion. (BaseCurrent())->x/y are re-read here,
            // not hoisted, matching the image reading the global fresh.
            int dx = x_dist(Bases[i].x, BaseCurrent->x);
            int dy = abs(Bases[i].y - BaseCurrent->y);
            int abs_dx = abs(dx);
            int abs_dy = abs(dy);
            int largest = abs_dx;
            if (abs_dx <= abs_dy) {
                largest = abs_dy;
            }
            int smallest = abs_dx;
            if (abs_dx >= abs_dy) {
                smallest = abs_dy;
            }
            int dist = largest - ((((abs_dy + abs_dx) >> 1) - smallest + 1) >> 1);
            if (dist < dist_hq) {
                dist_hq = dist;
            }
        }
    }
    if (dist_hq == 999) {
        dist_hq = 16;
    } else if (dist_hq == 0) {
        return 0;
    }
    BOOL has_creche = has_fac_built(FAC_CHILDREN_CRECHE, BaseIDCurrentSelected);
    if (BaseUpkeepStage == 1) {
        for (int i = 0, j = 0; i >= -64; i -= 8, j++) {
            int ineff;
            int factor;
            if (has_creche) {
                ineff = j - 2; // +2 on efficiency scale
                factor = i + 16; // ?
            } else {
                ineff = j;
                factor = i;
            }
            if (ineff >= 8) {
                PlayersData[faction_id].unk_46[j] += energy;
            } else {
                PlayersData[faction_id].unk_46[j] += energy * dist_hq / (factor + 64);
            }
        }
    }
    int ineffciency = 4 - PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::EFFICIENCY];
    if (has_creche) {
        ineffciency -= 2; // +2 on efficiency scale
    }
    if (ineffciency >= 8) {
        return energy;
    }
    return range(energy * dist_hq / ((8 - ineffciency) * 8), 0, energy);
}

// Five ordered stages: base/social, psych spending, facilities, police,
// projects and suppression. These are independent shared publications, not
// per-call locals; draw_psych and base_growth consume their original storage.
// RECOVERY_OBJECT: v2 0x0090E984 0x0090E998 ?BasePsychTalents@@3PAHA 0x004EA540
int BasePsychTalents[5];
// RECOVERY_OBJECT: v2 0x0090E920 0x0090E934 ?BasePsychDrones@@3PAHA 0x004EA540
int BasePsychDrones[5];
// RECOVERY_OBJECT: v2 0x0090E94C 0x0090E960 ?BasePsychSuperDrones@@3PAHA 0x004EA540
int BasePsychSuperDrones[5];

namespace {

int base_psych_clamp(int value, int maximum) {
    return value < 0 || maximum < 0 ? 0 : (value > maximum ? maximum : value);
}
bool base_psych_project_owned(int project_base, int faction) {
    return project_base >= 0 && Bases[project_base].faction_id_current == faction;
}
void base_psych_balance() {
    Base *base = BaseCurrent;
    for (;;) {
        const int capacity = base_yield_sub(base->population_size, base->specialist_total);
        const int total = base_yield_add(base_yield_add(base->super_drone_total,
            base->talent_total), base->drone_total);
        if (total <= capacity) return;
        if (base->talent_total != 0) {
            base->talent_total = base_yield_sub(base->talent_total, 1);
            base = BaseCurrent;
            if (base->super_drone_total != 0) {
                base->super_drone_total = base_yield_sub(base->super_drone_total, 1);
            } else if (base->drone_total != 0) {
                base->drone_total = base_yield_sub(base->drone_total, 1);
            }
        } else if (base->drone_total > capacity) {
            base->drone_total = base_yield_sub(base->drone_total, 1);
        } else {
            // Original early exit: this need not make D+S <= capacity.
            base->super_drone_total = base_psych_clamp(base->super_drone_total, base->drone_total);
            return;
        }
        base = BaseCurrent;
    }
}
void base_psych_clamp_totals() {
    Base *base = BaseCurrent;
    base->drone_total = base_psych_clamp(base->drone_total, base->population_size);
    base = BaseCurrent;
    base->talent_total = base_psych_clamp(base->talent_total, base->population_size);
    base_psych_balance();
}
void base_psych_publish(int stage) {
    const Base *base = BaseCurrent;
    BasePsychTalents[stage] = base->talent_total;
    BasePsychDrones[stage] = base->drone_total;
    BasePsychSuperDrones[stage] = base->super_drone_total;
}
bool base_psych_native_police(int vehicle, int faction) {
    if ((Players[faction].rule_flags & 0x40) == 0) return false;
    const int prototype = Vehs[vehicle].proto_id;
    if (prototype >= 64) return false;
    if (Weapon[VehPrototypes[prototype].weapon_id].offense_rating >= 0 && prototype != 15) return false;
    return Chassis[VehPrototypes[prototype].chassis_id].triad == 0;
}
void base_psych_reduce_drone() {
    Base *base = BaseCurrent;
    if (base->drone_total != 0) base->drone_total = base_yield_sub(base->drone_total, 1);
}

} // namespace

/*
Purpose: Calculate the current base's psych values.
// ORIGINAL: 0x004EA540 ?base_psych@@YAXXZ 0x004EA540-0x004EB481
// RECOVERY_BINDING: v1 {"evidence": ["image:0x004EA540-0x004EB481 complete original body; 0x004EA712-0x004EA722 clamps the first NoDrone adjustment immediately, and 0x004EAA23-0x004EAA3E applies signed talent rather than police.", "image:0x004EAB85,0x004EACD1,0x004EAE30,0x004EB209,0x004EB3D1 publish the five independent stage histories; draw_psych and base_growth remain seed-only consumers.", "image:0x004EAEDF-0x004EB0C5 scans fresh vehicle stacks in two passes with repeated ability queries; 0x004EB0CA-0x004EB0E3 uses a signed pacifism-count comparison.", "image:0x004EA946 and 0x004EB44E-0x004EB45E use talent=i-4 and matrix row=i-1; original social_ai at 0x005B50DB-0x005B511A instead consumes row=clamp(talent+4,0,7). Preserve the original one-step discrepancy."], "reason": "Recover the complete five-stage psych calculation and shared stage histories, including social simulations followed by the actual case, using genuine base, faction, project and vehicle state. Existing partial source was not a verified prefix."}
// Historical compiler lesson: the first facility queries are real has_fac
// calls, followed by real bitmask calls for hospitals, punishment and brood
// pit. Earlier inlining experiments lost calls despite similar source forms.
// Historical reconstruction correction: the second has_fac uses facility 6,
// Recreation Commons, not facility 2, Children's Creche. This is not evidence
// of an original game bug. The old partial prefix also substituted police for
// the signed talent effect and omitted the five-stage shared publications.
*/
void __cdecl base_psych() {
    // Original domain: valid game indices, finite vehicle stacks, and valid
    // faction rule divisors. No malformed-data guards are present in the image.
    const int faction = BaseCurrent->faction_id_current;
    const bool human = (static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & (1u << faction)) != 0;
    const int difficulty = human ? PlayersData[faction].diff_level : 3;
    const int content = base_yield_sub(6, difficulty);
    const int efficiency = PlayersData[faction].soc_effect_pending.values_[SocialEffect::EFFICIENCY];
    int bureaucracy = base_yield_mul(base_yield_mul(base_yield_add(content, 2),
        base_yield_add(efficiency < 0 ? 0 : efficiency, 4)),
        base_yield_signed_bits(MapAreaSqRoot)) / 56 / 2;
    if (bureaucracy != 0) {
        const int population = BaseCurrent->population_size;
        const int count = base_yield_signed_bits(PlayersData[faction].current_num_bases);
        bureaucracy = base_psych_clamp(base_yield_sub(base_yield_add(count,
            BaseIDCurrentSelected % bureaucracy), bureaucracy) / bureaucracy, population);
    }
    const int population = BaseCurrent->population_size;
    const int assimilation_limit = base_yield_sub(base_yield_add(population,
        human ? PlayersData[faction].diff_level : 3), 2) / 4;
    int initial_drones = base_psych_clamp(base_yield_sub(population, content), population);
    initial_drones = base_yield_add(initial_drones,
        base_psych_clamp((BaseCurrent->assimilation_turns_left + 9) / 10, assimilation_limit));
    if (Players[faction].rule_drone != 0) {
        initial_drones = base_yield_add(initial_drones, population / Players[faction].rule_drone);
    }
    int initial_talents = 0;
    if (Players[faction].rule_talent != 0) {
        initial_talents = base_yield_sub(base_yield_add(Players[faction].rule_talent, population), 1)
            / Players[faction].rule_talent;
    }
    for (int bonus = 0; bonus < Players[faction].faction_bonus_count; ++bonus) {
        if (Players[faction].faction_bonus_id[bonus] == RULE_NODRONE) {
            initial_drones = base_yield_sub(initial_drones, Players[faction].faction_bonus_val1[bonus]);
            if (initial_drones < 0) initial_drones = 0;
            break;
        }
    }

    int facility_drones = has_fac(FAC_GENEJACK_FACTORY, BaseIDCurrentSelected, 0)
        ? Rules.drones_genejack_factory : 0;
    if (has_fac(FAC_RECREATION_COMMONS, BaseIDCurrentSelected, 0)) facility_drones = base_yield_sub(facility_drones, 2);
    if ((has_fac(FAC_NETWORK_NODE, BaseIDCurrentSelected, 0)
         && base_psych_project_owned(GameDataStateStorage.fields_.secret_project_.virtual_world(), faction))
        || has_fac(FAC_HOLOGRAM_THEATRE, BaseIDCurrentSelected, 0)) {
        facility_drones = base_yield_sub(facility_drones, 2);
    }
    if (base_psych_project_owned(GameDataStateStorage.fields_.secret_project_.planetary_transit_system(), faction)
        && BaseCurrent->population_size <= 3) facility_drones = base_yield_sub(facility_drones, 1);
    const int facility_talents = has_fac(FAC_PARADISE_GARDEN, BaseIDCurrentSelected, 0) ? 2 : 0;
    if (base_yield_has_facility(FAC_RESEARCH_HOSPITAL)) facility_drones = base_yield_sub(facility_drones, 1);
    if (base_yield_has_facility(FAC_NANOHOSPITAL)) facility_drones = base_yield_sub(facility_drones, 1);
    int project_talents = base_psych_project_owned(GameDataStateStorage.fields_.secret_project_.human_genome_project(), faction) ? 1 : 0;
    if (base_psych_project_owned(GameDataStateStorage.fields_.secret_project_.clinical_immortality(), faction)) project_talents += 2;
    const bool punishment = base_yield_has_facility(FAC_PUNISHMENT_SPHERE);

    for (int talent_case = 8; talent_case >= 0; --talent_case) {
        if (talent_case != 0 && BaseUpkeepStage != 1) continue;
        const int talent = talent_case != 0 ? talent_case - 4 : PlayersData[faction].soc_effect_pending.values_[SocialEffect::TALENT];
        for (int police_case = 9; police_case >= 0; --police_case) {
            int police;
            if (police_case == 0) {
                // This query also occurs at the end of each hypothetical row.
                const bool brood_pit = base_yield_has_facility(FAC_BROOD_PIT);
                police = base_yield_add(PlayersData[faction].soc_effect_pending.values_[SocialEffect::POLICE], brood_pit ? 2 : 0);
                if (talent_case != 0) continue;
            } else {
                if (BaseUpkeepStage != 1 || talent_case == 0) continue;
                police = police_case - 6;
            }
            int police_limit = base_psych_clamp(base_yield_add(police, 2), 4);
            if (police_limit > 1) --police_limit;

            BaseCurrent->drone_total = initial_drones;
            BaseCurrent->talent_total = initial_talents;
            if (talent < 0) BaseCurrent->drone_total = base_yield_sub(BaseCurrent->drone_total, talent);
            else BaseCurrent->talent_total = base_yield_add(BaseCurrent->talent_total, talent);
            BaseCurrent->drone_total = base_yield_add(base_psych_clamp(BaseCurrent->drone_total,
                BaseCurrent->population_size), bureaucracy);
            BaseCurrent->super_drone_total = base_psych_clamp(base_yield_sub(BaseCurrent->drone_total,
                BaseCurrent->population_size), BaseCurrent->population_size);
            BaseCurrent->drone_total = base_psych_clamp(BaseCurrent->drone_total, BaseCurrent->population_size);
            base_psych_balance();
            base_psych_publish(0);

            BaseCurrent->talent_total = base_yield_add(BaseCurrent->talent_total, BaseCurrent->psych_total / 2);
            base_psych_clamp_totals();
            base_psych_publish(1);

            BaseCurrent->drone_total = base_yield_add(BaseCurrent->drone_total, facility_drones);
            BaseCurrent->talent_total = base_yield_add(BaseCurrent->talent_total, facility_talents);
            base_psych_clamp_totals();
            base_psych_publish(2);

            if (police_limit > 0) {
                int used = 0;
                if (base_psych_project_owned(GameDataStateStorage.fields_.secret_project_.self_aware_colony(), faction)) {
                    used = 1;
                    base_psych_reduce_drone();
                    if (police >= 3) base_psych_reduce_drone();
                }
                // Two fresh stack queries: enhanced/native police first,
                // ordinary eligible units second. The original does not test
                // vehicle ownership, nor cache the ability result for use.
                for (int pass = 0; pass < 2 && used < police_limit; ++pass) {
                    Base *const base = BaseCurrent;
                    int vehicle = stack_fix(veh_at(base->x, base->y));
                    while (vehicle >= 0) {
                        const int prototype = Vehs[vehicle].proto_id;
                        if (VehPrototypes[prototype].plan <= 3) {
                            const bool enhanced = has_abil(prototype, 0x400000) != 0
                                || base_psych_native_police(vehicle, faction);
                            if (enhanced == (pass == 0)) {
                                base_psych_reduce_drone();
                                if (has_abil(Vehs[vehicle].proto_id, 0x400000)) base_psych_reduce_drone();
                                if (police >= 3) base_psych_reduce_drone();
                                if (base_psych_native_police(vehicle, faction)) base_psych_reduce_drone();
                                if (++used >= police_limit) break;
                            }
                        }
                        vehicle = Vehs[vehicle].next_veh_id_stack;
                    }
                }
            }
            if (police < -2) {
                const int away = base_yield_signed_bits(BaseCurrentVehPacifismCount);
                int penalty;
                if (police == -3) penalty = away > 1 ? base_yield_sub(away, 1) : 0;
                else if (police == -4) penalty = away;
                else penalty = base_yield_add(away, away);
                BaseCurrent->drone_total = base_yield_add(BaseCurrent->drone_total, penalty);
            }
            base_psych_clamp_totals();
            base_psych_publish(3);

            BaseCurrent->talent_total = base_yield_add(BaseCurrent->talent_total, project_talents);
            if (base_psych_project_owned(GameDataStateStorage.fields_.secret_project_.longevity_vaccine(), faction)) {
                if (PlayersData[faction].soc_category_pending.models_[SocialCategory::ECONOMICS] == 2)
                    BaseCurrent->drone_total = base_yield_sub(BaseCurrent->drone_total, 1);
                if (PlayersData[faction].soc_category_pending.models_[SocialCategory::ECONOMICS] != 1)
                    BaseCurrent->drone_total = base_yield_sub(BaseCurrent->drone_total, 1);
            }
            if (punishment || BaseCurrent->nerve_staple_turns_left != 0) {
                BaseCurrent->drone_total = 0;
                BaseCurrent->talent_total = 0;
            }
            base_psych_clamp_totals();
            base_psych_publish(4);

            if (talent_case != 0) {
                const Base *const base = BaseCurrent;
                int score = base_yield_sub(base->talent_total, base->drone_total);
                if (score > 0 && (base->drone_total != 0
                    || base_yield_add(base->talent_total, base->talent_total) < base->population_size)) score /= 2;
                if (score < 0) {
                    score = base_yield_sub(score, base->population_size);
                    if (base->population_size < 6) score = base_yield_add(score, score);
                }
                // The original producer stores talent (case-4) at row case-1.
                // social_ai instead reads row clamp(talent+4,0,7): preserve
                // this observed one-step discrepancy, not a shifted matrix.
                int &weight = PlayersData[faction].unk_39[talent_case - 1][police_case - 1];
                weight = base_yield_add(weight, score);
            }
        }
    }
}

/*
Purpose: Determine the faction's base with the specified position sorted by the most energy output.
// ORIGINAL: 0x004EB490 ?base_rank@@YAHHH@Z 0x004EB490-0x004EB551 BYTE_EXACT
// LEVER: the image never calls sort_descending() - it inlines the bubble sort loop directly (two swap() calls, matching general.cpp's body). Writing that loop out here instead of calling sort_descending() took the call set from {sort_descending, __alloca_probe} to {swap x2, __alloca_probe}, matching the image's (call_diff now agrees). Best flag set moved from 0.645 to 0.847 similar. Remaining divergence is instruction-order/register-scheduling around the parameter loads, not chased further.
// LEVER: branch polarity. Deleted the two added bounds-check guards (`rank_position < 0` and `!bases_found`) and replaced them with the image's single guard clause `if (rank_position >= bases_found) return -1;` written immediately after the collection loop - VC6 compiles a guard clause by testing the ORIGINAL condition and jumping forward to the work (jl to the sort), falling through into the early return, which is what the image does; the equivalent `if (rank_position < bases_found) {work} return -1;` form emits the opposite polarity (jge past the work) and does not match. Also reproduces the original's lack of a `rank_position < 0` bounds check - see the BUG IN THE ORIGINAL comment below. Took this body to BYTE_EXACT (70/70), from 4/70.
// LEVER: swap_call, the bitmask_call lever again (base.cpp file scope). general.h's swap() is `MEASURED inline`, so /Ob1 folded it into both swap sites and the bubble sort became an xor chain - 26/70, with the frame's two arrays mirrored (base_id at [ebp-0x1000], base_energy at [ebp-0x800]) against the image's (energy low, id high). Routing both swaps through a plain out-of-line `static void swap_call(int*, int*)` restores the image's two `call 0x628a50`s AND the array order, because an address-taken array is placed when its & is first materialised: with the calls out of line the sort's swap(&base_energy...) first is what puts base_energy at [ebp-0x1000]. TRIED: declaring base_energy before base_id and/or storing it first while swap() stayed folded - all three spellings measured 26/70 with the arrays still mirrored; the frame follows the address-taken order, not declaration or store order.
// size      193 bytes
// prototype int (__cdecl ?base_rank@@YAHHH@Z)(int factionID, int minBasesToRank)
// callers   1   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00628A50 0x00645550
Return Value: Base id for the specified rank position or -1 for invalid requests
Status: Complete
*/
int __cdecl base_rank(int faction_id, int rank_position) {
    // BUG IN THE ORIGINAL: the shipped code never bounds-checks rank_position
    // against 0, only against bases_found below - a negative rank_position
    // with at least one matching base reads base_id[rank_position] out of
    // bounds. Reproduced deliberately: the added guard here scored worse
    // (see LEVER below), because the image loads rank_position from memory
    // only once, at the single `cmp .../jl` test after the collection loop.
    // base_energy lands at [ebp-0x1000] and base_id above it at [ebp-0x800]
    // once the two swaps go through the out-of-line swap_call: an address-
    // taken array is placed when its & is first materialised, so the sort's
    // swap order fixes the frame. Declaring or storing energy first, with
    // swap() folded, does NOT reach that layout - 26/70.
    int base_id[MaxBaseNum];
    int base_energy[MaxBaseNum];
    int bases_found = 0;
    for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
        if (Bases[i].faction_id_current == faction_id) {
            base_id[bases_found] = i;
            base_energy[bases_found++] = (Bases[i].energy_intake_1 * 512) + i;
        }
    }
    if (rank_position >= bases_found) {
        return -1;
    }
    // Open-coded sort_descending() (see general.cpp): the image never
    // calls it, only swap() twice per inner-loop swap.
    int bounds = bases_found - 1;
    BOOL has_swapped;
    do {
        has_swapped = false;
        for (int i = 0; i < bounds; i++) {
            if (base_energy[i] < base_energy[i + 1]) {
                has_swapped = true;
                swap_call(&base_energy[i], &base_energy[i + 1]);
                swap_call(&base_id[i], &base_id[i + 1]);
            }
        }
    } while (has_swapped);
    return base_id[rank_position];
}

/*
Purpose: Check what facility (if any) a base needs for additional population growth. Stand alone
         function unused in original game and likely optimized out.
// ORIGINAL: 0x004EEF80 ?pop_goal_fac@@YAHH@Z 0x004EEF80-0x004EF087
// size      263 bytes
// prototype int (__cdecl ?pop_goal_fac@@YAHH@Z)(int baseID)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0050BA00
// LEVER: both has_fac_built(FAC_HAB_COMPLEX/FAC_HABITATION_DOME, base_id)
//        sites routed through has_fac_built_call (the same lever pop_goal,
//        its twin below, already records) - the folded `MEASURED inline
//        bitmask` turns the two tests into bare `test byte ptr`, where the
//        image makes a real `call 0x50ba00` and keeps the offset/mask local
//        pair. Compiled body 54 -> 81 instructions against the image's 102.
Return Value: Facility id needed for pop growth or zero if base already has Hab Complex and Dome.
Status: Complete
*/
int __cdecl pop_goal_fac(int base_id) {
    uint32_t faction_id = Bases[base_id].faction_id_current;
    uint32_t limit_mod = has_project(SP_ASCETIC_VIRTUES, faction_id) ? 2 : 0;
    int pop = Bases[base_id].population_size - limit_mod + Players[faction_id].rule_population;
    if (pop >= (int)Rules.pop_limit_sans_hab_complex
        && !has_fac_built_call(FAC_HAB_COMPLEX, base_id)) {
        return FAC_HAB_COMPLEX;
    }
    if (pop >= (int)Rules.pop_limit_sans_hab_dome
        && !has_fac_built_call(FAC_HABITATION_DOME, base_id)) {
        return FAC_HABITATION_DOME;
    }
    return 0; // Pop hasn't reached capacity or Base already has Hab Complex and Dome
}

/*
Purpose: Calculate population goal for a base.
// ORIGINAL: 0x004EF090 ?pop_goal@@YAHH@Z 0x004EF090-0x004EF1B2
// size      290 bytes
// prototype int (__cdecl ?pop_goal@@YAHH@Z)(int baseID)
// callers   1   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0050BA00
// LEVER: both has_fac_built(FAC_HAB_COMPLEX/FAC_HABITATION_DOME, base_id)
//        sites routed through the new `has_fac_built_call` helper (this
//        file, near cost_factor) so bitmask() emits a real `call 0x50ba00`
//        with an [ebp-4]/[ebp-8] offset/mask local pair, matching the
//        image's two `push 0x18`/`push 0x19` + call sequences instead of
//        folding to a byte test. Best similarity 0.910 (38/105 agreeing,
//        /c /O2 /Gy /GR- /Oy- /GX) - the largest jump of the bitmask-call
//        family (cost_factor TRIED the same lever; see that note).
Return Value: Goal population
Status: Complete
*/
int __cdecl pop_goal(int base_id) {
    uint32_t faction_id = Bases[base_id].faction_id_current;
    uint32_t limit_mod = has_project(SP_ASCETIC_VIRTUES, faction_id) ? 2 : 0;
    int goal = (36 - Bases[base_id].population_size) / 6 + Bases[base_id].population_size;
    if (goal <= 6) {
        goal = 6;
    }
    if (!has_fac_built_call(FAC_HAB_COMPLEX, base_id)) {
        int compare = Rules.pop_limit_sans_hab_complex - Players[faction_id].rule_population
            + limit_mod;
        if (goal >= compare) {
            goal = compare;
        }
    }
    if (!has_fac_built_call(FAC_HABITATION_DOME, base_id)) {
        int compare = Rules.pop_limit_sans_hab_dome - Players[faction_id].rule_population
            + limit_mod;
        if (goal >= compare) {
            goal = compare;
        }
    }
    return goal;
}

/*
Purpose: Move the specified base's production queue forward if applicable.
// ORIGINAL: 0x004F06E0 ?base_queue@@YAHH@Z 0x004F06E0-0x004F07D8
// size      248 bytes
// prototype int (__cdecl ?base_queue@@YAHH@Z)(int baseID)
// callers   1   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004455F0 0x0046AEF0 0x005BA0E0 0x005BA910
// LEVER: `Base *base = &Bases[base_id];` HOISTED, rather than
//        `Bases[base_id]` re-subscripted at every field: the image computes
//        the base's address exactly ONCE (esi = &Bases[base_id]) and reuses
//        it for every field (queue_size, faction, queue_production_id, x,
//        y) through the whole function, where re-subscripting recomputes
//        (and partially re-folds) the base_id*sizeof(Base) multiply at each
//        site. Moved 0/89 (0.804) -> 15/89 (0.859).
Return Value: Is there a valid item in queue to be built? true/false
Status: Complete
*/
BOOL __cdecl base_queue(int base_id) {
    Base *base = &Bases[base_id];
    uint32_t faction_id = base->faction_id_current;
    while (base->queue_size) {
        int queue_prod_id = base->queue_production_id[0];
        if (queue_prod_id >= 0) {
            PlayersData[faction_id].proto_id_queue[queue_prod_id]--;
        }
        for (uint32_t i = 0; i < base->queue_size; i++) {
            base->queue_production_id[i] = base->queue_production_id[i + 1];
        }
        base->queue_size--;
        queue_prod_id = base->queue_production_id[0];
        if((queue_prod_id >= 0) ? veh_avail(queue_prod_id, faction_id, base_id)
            : facility_avail(-queue_prod_id, faction_id, base_id, 0)) {
            if (queue_prod_id <= -FacilitySPStart) {
                wave_it(36); // CPU project initiated
            }
            draw_radius(base->x, base->y, 2, 2);
            return true;
        }
    }
    return false;
}

/*
Purpose: Check if current base has had an energy shortfall. If so, reset all existing energy convoy
         orders for the faction. TODO: Revisit and find a way to only reset specific base convoys.
// ORIGINAL: 0x004F4DC0 ?base_energy_costs@@YAXXZ 0x004F4DC0-0x004F4E73
// TRIED: already 0.852 similar (best flag set, 3/60 raw). The remaining
//            gap is inside base_who()/map_loc()'s inlined `x >> 1`: the
//            image widens Vehs[i].x to int (movsx) BEFORE the shift, this
//            tree's inline shifts the 16-bit value first and widens after -
//            an explicit `(int)Vehs[i].x` cast at the call site does not
//            change it (measured), so the divergence is inside the shared
//            inline body, not this call site's source shape.
// size      179 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl base_energy_costs() {
    if (BaseCurrent->energy_surplus >= 0 || GameDataStateStorage.fields_.vehicle_current_count_ <= 0) {
        return;
    }
    uint32_t faction_id = BaseCurrent->faction_id_current;
    for (int i = 0; i < GameDataStateStorage.fields_.vehicle_current_count_; i++) {
        if (Vehs[i].faction_id == faction_id
            && VehPrototypes[Vehs[i].proto_id].plan == PLAN_SUPPLY_CONVOY
            && Vehs[i].order == ORDER_CONVOY && Vehs[i].order_auto_type == RSC_ENERGY
            && base_who(Vehs[i].x, Vehs[i].y) >= 0) {
            Vehs[i].order = ORDER_NONE;
        }
    }
}

/*
Purpose: Calculate facility maintenance cost for specified faction.
// ORIGINAL: 0x004F6510 ?fac_maint@@YAHHH@Z 0x004F6510-0x004F65E7
// size      215 bytes
// prototype int (__cdecl ?fac_maint@@YAHHH@Z)(int facilityID, int factionID)
// callers   7   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0057EFA0 0x005B9F20
// LEVER: three changes. (1) walking pointers `bonus_id`/`bonus_val` over
//        Players[faction_id].faction_bonus_id/faction_bonus_val1, stepped
//        alongside `i`, instead of re-indexing the arrays each access
//        (lever 7). (2) dropped the `bonus_count` local so the loop bound
//        re-reads Players[faction_id].faction_bonus_count from the global
//        every iteration, matching the image. (3) compute diff_factor
//        before calling best_reactor(), matching the image's instruction
//        order. Best similarity 0.820 -> 0.924 (agreeing count drops 8->4,
//        but similarity - the tool's real tiebreak - is higher).
Return Value: Facility maintenance cost
Status: Complete
*/
int __cdecl fac_maint(int facility_id, int faction_id) {
    if (facility_id == FAC_COMMAND_CENTER) {
        int diff_factor = (PlayersData[faction_id].diff_level + 1) / 2;
        int reactor = best_reactor(faction_id);
        if (reactor < 0 || diff_factor < 0) {
            return 0;
        }
        return (reactor > diff_factor) ? diff_factor : reactor;
    }
    int *bonus_id = &Players[faction_id].faction_bonus_id[0];
    int *bonus_val = &Players[faction_id].faction_bonus_val1[0];
    for (int i = 0; i < Players[faction_id].faction_bonus_count; i++, bonus_id++, bonus_val++) {
        if ((*bonus_id == RULE_FACILITY || (*bonus_id == RULE_FREEFAC
            && has_tech(Facility[*bonus_val].preq_tech, faction_id)))
            && *bonus_val == (int)facility_id) {
            return 0;
        }
    }
    return Facility[facility_id].maint;
}

/*
Purpose: Calculate overall maintenance cost for the currently selected base.
// ORIGINAL: 0x004F65F0 ?base_maint@@YAXXZ 0x004F65F0-0x004F67E3
// TRIED: the image's own call list has ONE call to bitmask() (0x50BA00);
//            this tree's compile makes TWO, so `set_fac(fac,
//            BaseIDCurrentSelected, false)` - a BYTE_EXACT standalone
//            function elsewhere, called 10 times - looked like it was
//            auto-inlining here and contributing its own internal bitmask()
//            call on top of the loop guard's. Hand-inlining set_fac's body
//            (set==false is a literal at this call site, so only the
//            `&= ~mask` arm survives) does NOT improve it: best similarity
//            goes 0.702 -> 0.696, a wash rather than a win, so the
//            standalone `set_fac()` call is kept. Best flag set
//            /O2 /Gy /GR- /Oy- /GX either way.
// size      499 bytes
// prototype
// callers   1   call targets   5
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0048C650 0x004E4430 0x004F6510 0x0050BA00 0x00625E50
Return Value: Base maintenance cost
Status: Complete
*/
void __cdecl base_maint() {
    uint32_t faction_id = BaseCurrent->faction_id_current;
    for (int fac = 1; fac < FacilitySPStart; fac++) {
        if (has_fac_built(fac)) {
            uint32_t maint = fac_maint(fac, faction_id);
            if (has_project(SP_SELF_AWARE_COLONY, faction_id)) {
                if (PlayersData[faction_id].flags & PFLAG_SELF_AWARE_COLONY_LOST_MAINT) {
                    maint++; // attempt to even out maintenance costs from lossy integer division
                }
                if (maint & 1) {
                    PlayersData[faction_id].flags |= PFLAG_SELF_AWARE_COLONY_LOST_MAINT;
                } else {
                    PlayersData[faction_id].flags &= ~PFLAG_SELF_AWARE_COLONY_LOST_MAINT;
                }
                maint /= 2;
            }
            PlayersData[faction_id].energy_reserves -= maint;
            PlayersData[faction_id].maint_cost_total += maint;
            if (PlayersData[faction_id].energy_reserves < 0) {
                if (PlayersData[faction_id].diff_level <= DLVL_SPECIALIST
                    || BaseCurrent->queue_production_id[0] == -fac) {
                    PlayersData[faction_id].energy_reserves = 0;
                } else {
                    set_fac(fac, BaseIDCurrentSelected, false);
                    PlayersData[faction_id].energy_reserves
                        = cost_factor(faction_id, RSC_MINERALS, -1) * Facility[fac].cost;
                    parse_say(1, (int)*(&Facility[fac].name), -1, -1);
                    popb("POWERSHORT", 0x10000, 14, "genwarning_sm.pcx", NULL);
                }
            }
        }
    }
}

/*
Purpose: Check if the initial base name is unique. If not, append a sequential number after it.
         The function has been rewritten to work as intended.
// ORIGINAL: 0x004F7FE0 ?make_base_unique@@YAXH@Z 0x004F7FE0-0x004F8087
// size      167 bytes
// prototype void (__cdecl ?make_base_unique@@YAXH@Z)(int baseID)
// callers   3   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00645470 0x00645660 0x0064FC88
// LEVER: dropped std::string entirely for the image's strcat/_itoa idiom
//        operating directly on Bases[base_id].name_string (no separate core
//        copy) - the image has no C++ library here. Took the body from
//        0/61 to 3/61 agreeing and removed the whole SEH prologue
//        (push -1/push 0/mov eax,fs:[0]) the std::string version needed.
Return Value: n/a
Status: Complete
*/
void __cdecl make_base_unique(int base_id) {
    purge_spaces(Bases[base_id].name_string); // added to remove extraneous whitespace
    uint32_t found = 0;
    for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
        if (base_id != (uint32_t)i && !strcmp(Bases[i].name_string, Bases[base_id].name_string)) {
            // BUG IN THE ORIGINAL: only the FIRST collision truncates
            // name_string back to its 21-character core; a second collision
            // against the renamed base appends another "-N" onto the
            // already-suffixed name instead of restarting from the core, so
            // repeated collisions compound ("Name-2-3" rather than
            // "Name-3"). Reproduced deliberately - this is what the shipped
            // bytes do, even though the file's own Purpose comment says a
            // later pass "rewrote [this] to work as intended".
            if (!found) { // only create core part of base name on first collision
                Bases[base_id].name_string[21] = 0;
            }
            strcat(Bases[base_id].name_string, "-");
            char digits[0x54];
            _itoa(++found + 1, digits, 10); // start with appending "-2"
            strcat(Bases[base_id].name_string, digits);
            i = 0; // reset loop to verify base name is still unique
        }
    }
}

/*
Purpose: Check if a faction has the specified secret project built in a base they control.
// ORIGINAL: 0x004F80D0 ?has_project@@YAHHH@Z 0x004F80D0-0x004F8103 BYTE_EXACT
// size      51 bytes
// prototype int (__cdecl ?has_project@@YAHHH@Z)(int projectID, int factionID)
// callers   8   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Does faction have Secret Project? true/false
Status: Complete
*/
// BODY IN base.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Checks whether the facility (non-SP) has been build in the currently selected base.
// ORIGINAL: 0x00500290 ?has_fac_built@@YA_NH@Z 0x00500290-0x005002DE
// size      78 bytes
// prototype bool (__cdecl ?has_fac_built@@YA_NH@Z)(int facilityID)
// callers   1   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0050BA00
// LEVER: body in base.h hoists `int base_id = BaseIDCurrentSelected;` above the
//        ternary guard so the global is read unconditionally, before the
//        `facility_id >= FacilityRepStart` early-out - the image does the
//        same (its `mov esi,[0x689370]` precedes the `cmp/jl`). The tool's
//        agreeing-instruction count is pinned at 2/34 either way because
//        `measure` can only isolate this address by forcing /Ob0, which also
//        disables inlining the has_fac_built(facility_id,base_id) callee and
//        so never reaches the image's real bitmask-call shape; but similarity
//        at that flag set moved 0.571 -> 0.612, confirming the reorder is
//        real. Two divergences remain, not fixable from this call site: the
//        image's early return clears full `eax` (int width) where a `bool`
//        return clears only `al`, and the callee's bitmask() is a real call
//        in the image but constant-folds here - both are the documented
//        systemic gaps (return-width note above; bitmask folding TRIED
//        on has_fac base.cpp:104).
Return Value: Does current base have facility? true/false
Status: Complete
*/

/*
Purpose: Check if the base already has a particular facility built.
Original Offset: n/a
Return Value: Does base have facility built? true/false
Status: Complete
*/
// BODY IN base.h: eleven bodies call it where the image calls nothing -
// `call_diff` names it - so VC6 has to be able to inline it.

/*
Purpose: Get the current status of the specified project.
// ORIGINAL: 0x005002E0 ?base_project@@YAHH@Z 0x005002E0-0x005002EF BYTE_EXACT
// size      15 bytes
// prototype int (__cdecl ?base_project@@YAHH@Z)(int projectID)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Base id, if not built (-1) or destroyed (-2)
Status: Complete
*/
// BODY IN base.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Suggest the base the specified pair of factions should agree to attack together.
// ORIGINAL: 0x0054ACC0 ?suggest_plan@@YAHHH@Z 0x0054ACC0-0x0054AF9D
// TRIED: `call_diff` flags this MORE (9 calls vs the image's 8: extra
// `region_at`/`vector_dist`, missing `_abs`) but it is a KNOWN, MEASURED
// trade-off, not an unexamined gap - see the full "Verification note" /
// "TRIED" prose in the Purpose block below (kept where it was written;
// a marker-position scan does not reach prose after `Return Value:`, which
// is why this address kept reading as untouched). Open-coding the
// x_target/y_target `vector_dist` call as the `x_dist()+abs()x4` expansion
// that the image's callee list implies measured WORSE here (best similarity
// 0.728 -> 0.689 across all flag sets, same agreeing-instruction count), so
// the real `vector_dist(...)` call is kept deliberately even though it
// disagrees with `call_diff`.
// size      733 bytes
// prototype
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004E3C60 0x004E3D50 0x00644F3A
Return Value: Base id of the suggested target, or -1 if the pair has no shared target
Status: Complete

WHO THE TWO PARAMETERS ARE. battle_plans (0x0054B1C0) is the only caller and it asks this twice,
once each way round, so neither parameter is "the AI" and neither is "the player": faction_id is
the side whose plan is being drawn up and faction_id_2 is the other side of the conversation. A
base is eligible only when BOTH factions are already at vendetta with its owner, so this proposes
a joint target within a war that exists rather than a war to start, and only when faction_id can
actually see the base.

THE NUMBER IS A COST, NOT A SCORE. The loop keeps the LOWEST value and seeds it at 9999. It is
assembled from three terms:

  - where the war already is. When faction_id holds a target tile (x_target >= 0) the cost opens
    at twice the radiating distance from that tile to the base, plus 512 when the base is not even
    in the same region as the tile.
  - how far the base sits from faction_id's own territory: BaseFindDist after searching for its
    nearest base in the SAME region as the candidate. Owning nothing in that region repeats the
    search unrestricted and adds 1024, the price of an amphibious war. This whole term is doubled
    when faction_id already holds a target tile, so a faction with a war in progress is pulled
    harder toward what it can reach than one starting fresh.
  - the same distance for faction_id_2, penalised 256 instead of 1024 and never doubled. The
    partner's convenience counts, at a quarter of the weight.

AN EXISTING PLAN WINS OUTRIGHT. If faction_id has already named a base in base_id_atk_target, that
base is returned the moment the loop reaches it, whatever it costs and whatever was cheaper
earlier. The suggestion machinery only decides while nothing has been decided.

Verification note: the visibility test transcribes a disjunct that cannot fire. The original
re-tests "base owner == faction_id" at 0x0054AD52 before consulting Bases[i].visibility, exactly as
the six-argument base_find does, but the loop already skipped that base at 0x0054AD00. It is kept
because it is what the image does, and it is still covered: inverting the comparison stops the
body skipping bases faction_id cannot see, which the fixtures observe.

Verification note: three sites the image inlines are written here as calls, each of which is
instruction-for-instruction the standalone body in the image - x_dist (0x00579790) and the
two-argument vector_dist (0x004F8090), reached together through the four-argument vector_dist, and
region_at (0x00500220), which the original expands three separate times. The inlined map_loc
halves x with SAR where the exported region_at takes uint32_t and would use SHR; the two agree for
every non-negative coordinate, which is all a base or a target tile can hold.

// TRIED: open-coding the x_target/y_target vector_dist call as the
//            x_dist()+abs()x4 expansion (the same shape that works on
//            base_find and black_market) measured WORSE here: best
//            similarity across all flag sets dropped from 0.728
//            (/c /O2 /Gy /GR- /Oy- /GX, 6/233 agreeing) to 0.689
//            (/c /O2 /Ob0 /Gy /GR- /Oy- /GX, 6/233 agreeing) - same
//            agreeing count, lower similarity, and the default-flags
//            similarity at the SAME flag set collapsed to 0.354. Left as a
//            real vector_dist(...) call.
*/
int __cdecl suggest_plan(int faction_id, int faction_id_2) {
    PopupDialogFactionID = faction_id_2;
    int lowest_cost = 9999;
    int base_id_best = -1;
    for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
        int faction_id_base = Bases[i].faction_id_current;
        if (faction_id_base == faction_id || faction_id_base == faction_id_2) {
            continue;
        }
        if (!(PlayersData[faction_id].diplo_treaties[faction_id_base] & DTREATY_VENDETTA)
            || !(PlayersData[faction_id_2].diplo_treaties[faction_id_base] & DTREATY_VENDETTA)) {
            continue;
        }
        if (faction_id_base != faction_id && !((1 << faction_id) & Bases[i].visibility)) {
            continue;
        }
        int cost = 0;
        if (PlayersData[faction_id].x_target >= 0) {
            int x_target = PlayersData[faction_id].x_target;
            int y_target = PlayersData[faction_id].y_target;
            cost = 2 * vector_dist(x_target, y_target, Bases[i].x, Bases[i].y);
            if (region_at(Bases[i].x, Bases[i].y) != region_at(x_target, y_target)) {
                cost += 512;
            }
        }
        if (base_find(Bases[i].x, Bases[i].y, faction_id, region_at(Bases[i].x, Bases[i].y),
                      -1, -1) < 0) {
            base_find(Bases[i].x, Bases[i].y, faction_id);
            BaseFindDist += 1024;
        }
        cost += BaseFindDist * ((PlayersData[faction_id].x_target >= 0) ? 2 : 1);
        if (base_find(Bases[i].x, Bases[i].y, faction_id_2, region_at(Bases[i].x, Bases[i].y),
                      -1, -1) < 0) {
            base_find(Bases[i].x, Bases[i].y, faction_id_2);
            BaseFindDist += 256;
        }
        cost += BaseFindDist;
        if (PlayersData[faction_id].base_id_atk_target == i) {
            return i;
        }
        if (cost < lowest_cost) {
            lowest_cost = cost;
            base_id_best = i;
        }
    }
    return base_id_best;
}

/*
Purpose: Determine the faction's best base to attack the specified base from.
// ORIGINAL: 0x0054AFA0 ?attack_from@@YAHHH@Z 0x0054AFA0-0x0054B137
// size      407 bytes
// prototype int (__cdecl ?attack_from@@YAHHH@Z)(int baseID, int factionID)
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00644F3A
// LEVER: open-coded the vector_dist(x, y, Bases[base_id_to_atk].x,
//        Bases[base_id_to_atk].y) ternary branch as the x_dist()+abs()x4
//        expansion (same shape as base_find 0x004E3B80/0x004E3D50) - the
//        image never calls vector_dist, only abs() x4. Took best similarity
//        from 3/135 to 4/135 agreeing (0.714 similar, /c /O2 /Oi- /Gy /GR-
//        /Oy- /GX). Unlike suggest_plan (TRIED above), this one measured
//        better, not worse - the two bodies disagree on the lever.
Return Value: Base id to attack from or 0
Status: Complete
*/
int __cdecl attack_from(int base_id_to_atk, int faction_id) {
    uint32_t region_src = region_at(Bases[base_id_to_atk].x, Bases[base_id_to_atk].y);
    int base_id_atk_from = 0; // TODO: eventually change this to -1 and add handling to the return
    int search = 9999;        // value to prevent the edge case of base id 0 being incorrect target
    for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
        if (Bases[i].faction_id_current == faction_id) {
            int x = Bases[i].x;
            int y = Bases[i].y;
            int compare;
            if (x == PlayersData[faction_id].x_target && y == PlayersData[faction_id].y_target) {
                compare = 0;
            } else {
                // Open-coded vector_dist(x, y, Bases[base_id_to_atk].x,
                // Bases[base_id_to_atk].y): the image calls abs() four
                // times, never vector_dist itself - see del_site
                // (0x00579E70) and base_find (0x004E3B80) for the same
                // expansion.
                int dx = x_dist(x, Bases[base_id_to_atk].x);
                int dy = abs(y - Bases[base_id_to_atk].y);
                int abs_dx = abs(dx);
                int abs_dy = abs(dy);
                int largest = abs_dx;
                if (abs_dx <= abs_dy) {
                    largest = abs_dy;
                }
                int smallest = abs_dx;
                if (abs_dx >= abs_dy) {
                    smallest = abs_dy;
                }
                compare = largest - ((((abs_dy + abs_dx) >> 1) - smallest + 1) >> 1);
            }
            if (region_at(x, y) != region_src) {
                compare += 1000;
            }
            if (compare <= search) {
                search = compare;
                base_id_atk_from = i;
            }
        }
    }
    return base_id_atk_from;
}

/*
Purpose: Determine the value of the specified base between the requester and the respondent faction.
         This valuation could be triggered either from a gift, a threat or a base swap.
// ORIGINAL: 0x0054CB50 ?value_of_base@@YAHHHHHH@Z 0x0054CB50-0x0054CF39
// size      1001 bytes
// prototype int (__cdecl ?value_of_base@@YAHHHHHH@Z)(int baseID, int factionIDReq, int factionIDRes, int baseIDCounter, int tgl)
// callers   3   call targets   7
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3A50 0x0050BA00 0x0059E980 0x005AC060 0x005B8E10 0x005BFE90 0x00645660
// TRIED: widening `value`, `base_pop_factor`, `res_base_count_region`,
//            `req_base_count_region` and `facil_count` from uint32_t to int
//            (to get signed cdq/sar and jge/jl instead of shr/jae) measured
//            WORSE at every flag set - best case dropped from 9/337 agreeing
//            (0.226 similar, /c /O2 /Gy /GR- /Oy- /GX) to 5/337 (0.218
//            similar). Left as uint32_t; the "Bug fix: treat value as
//            unsigned" comment stays.
Return Value: Value of base or -1 for invalid requests
Status: Complete - testing
*/
int __cdecl value_of_base(int base_id, int faction_id_req, int faction_id_res,
                          int overmatch_deg, BOOL tgl) {
    if (base_id < 0) {
        return -1;
    }
    int x = Bases[base_id].x;
    int y = Bases[base_id].y;
    int dist_factor = vulnerable(faction_id_req, x, y);
    if (dist_factor <= 0) {
        return -1;
    }
    uint32_t region_base = region_at(x, y);
    for (int i = 1; i < RadiusRange[6]; i++) {
        int x_radius = xrange(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius)) {
            int base_id_found = base_at(x_radius, y_radius);
            if (base_id_found >= 0) {
                uint32_t faction_id_base = Bases[base_id_found].faction_id_current;
                uint32_t region_found = region_at(x_radius, y_radius);
                if (faction_id_base != faction_id_req) {
                    if (region_base == region_found) {
                        if (faction_id_base == faction_id_res) {
                            dist_factor++;
                        } else {
                            break;
                        }
                    }
                } else {
                    if (region_base == region_found) {
                        dist_factor--;
                    }
                    break;
                }
            }
        }
    }
    int veh_id = stack_fix(veh_at(x, y)); // reason to define here rather than below?
    int most_reserves = PlayersData[faction_id_res].energy_reserves;
    if (PlayersData[faction_id_req].energy_reserves > most_reserves) {
        most_reserves = PlayersData[faction_id_req].energy_reserves;
    }
    uint32_t base_pop_factor = Bases[base_id].population_size;
    if (base_pop_factor < 3) {
        base_pop_factor = 3;
    }
    // Bug fix: treat value as unsigned, original uses signed which could cause incorrect valuation
    uint32_t value = ((most_reserves + 1000) / (dist_factor + 4)) * base_pop_factor;
    if (value < 100) {
        value = 100;
    }
    if (veh_who(x, y) < 0) {
        value /= 2;
    }
    uint32_t res_base_count_region = PlayersData[faction_id_res].region_total_bases[region_base];
    if (!res_base_count_region) {
        value *= 2;
    }
    uint32_t req_base_count_region = PlayersData[faction_id_req].region_total_bases[region_base];
    if (req_base_count_region == 1) {
        if (!res_base_count_region) {
            value *= 2;
        }
        if (res_base_count_region == 1) {
            value *= 2;
        }
    }
    if (tgl) {
        value *= 4;
    }
    if (res_base_count_region && req_base_count_region) {
        if (res_base_count_region >= req_base_count_region * 5) {
            value /= 2;
        }
        if (overmatch_deg) {
            value /= 2;
        }
    }
    uint32_t facil_count = 0;
    for (uint32_t fac = 1; fac < FacilitySPStart; fac++) {
        if (fac < FacilityRepStart && has_fac_built(fac, base_id)) {
            facil_count++;
            value += (Facility[fac].cost * facil_count);
        }
    }
    for (uint32_t proj = 0; proj < MaxSecretProjectNum; proj++) {
        if (base_project(proj) == base_id) {
            value += (Facility[FacilitySPStart + proj].cost * 25);
        }
    }
    for (i = veh_top(veh_id); i >= 0; i = Vehs[i].next_veh_id_stack) {
        if (Vehs[i].faction_id == faction_id_req) {
            value += (VehPrototypes[Vehs[i].proto_id].cost * 2);
        }
    }
    if (!_stricmp(Players[faction_id_req].filename, "BELIEVE")) {
        value *= 2;
    }
    if (is_objective(base_id)) {
        value *= 4;
    }
    return value;
}

/*
Purpose: Determine ideal non-offense (defense, combat, recon) unit count for the specified base.
// ORIGINAL: 0x00560B30 ?garrison_check@@YAHH@Z 0x00560B30-0x00560D30
// size      512 bytes
// prototype int (__cdecl ?garrison_check@@YAHH@Z)(int baseID)
// callers   4   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0050BA00 0x005AC060 0x005C8BA0
// LEVER: deleted the `BOOL is_obj = is_objective(base_id);` local and wrote
//        is_objective(base_id) at both use sites - the image calls
//        0x5AC060 (is_objective) twice, not once with a cached result. Best
//        similarity 0.763 -> 0.809; agreeing count unchanged at 4/170. The
//        bitmask() half of the evidence (has_fac_built folding to a byte
//        test) is the documented systemic gap - TRIED on has_fac
//        base.cpp:104 - and not fixable here.
// LEVER: that gap IS fixable here, for this one site - the FAC_HEADQUARTERS
//        test has a LITERAL facility id, so routing it through
//        has_fac_built_call forces the real `call 0x50ba00` the image makes
//        (base_making/pop_goal/pop_goal_fac's lever). Compiled body
//        159 -> 167 instructions against the image's 170, differing runs
//        26 -> 19. NOTE the same lever is the WRONG way round in
//        value_of_base (0x0054CB50): its `has_fac_built(fac, base_id)` has
//        a RUNTIME fac, and forcing the call there costs 3 instructions and
//        5 more differing runs - reverted.
Return Value: Amount of non-offensive units needed (1-10)
Status: Complete
*/
int __cdecl garrison_check(int base_id) {
    int x = Bases[base_id].x;
    int y = Bases[base_id].y;
    uint32_t faction_id = Bases[base_id].faction_id_current;
    uint32_t sp_count = 0;
    for (uint32_t i = 0; i < MaxSecretProjectNum; i++) {
        if (base_project(i) == (int)base_id) {
            sp_count++;
        }
    }
    int garrison = (sp_count + 2) / 3 + (Bases[base_id].population_size + 1) / 4 + 1;
    if (has_fac_built_call(FAC_HEADQUARTERS, base_id) || bit_at(x, y) & BIT_UNK_40000000
        || is_objective(base_id)) {
        garrison++;
    }
    if (is_objective(base_id) && PlayersData[faction_id].flags & PFLAG_STRAT_DEF_OBJECTIVES) {
        garrison++;
    }
    if (PlayersData[faction_id].region_base_plan[region_at(x, y)] == PLAN_COLONIZATION) {
        garrison--;
    }
    int faction_id_sea = zoc_sea(x, y, faction_id) - 1;
    if (faction_id_sea > 0
        && (has_treaty(faction_id, faction_id_sea, DTREATY_VENDETTA | DTREATY_WANT_REVENGE)
        || has_treaty(faction_id_sea, faction_id, DTREATY_WANT_REVENGE)
        || PlayersData[faction_id_sea].integrity_blemishes >= 4)) {
        garrison++;
    }
    int support = PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::SUPPORT];
    if (support <= -4) {
        garrison--;
    }
    if (support <= -3 && garrison > 2) {
        garrison--;
    }
    return range(garrison, 1, 10);
}

/*
Purpose: Determine the number of defensive units needed to protect the specified base.
// ORIGINAL: 0x00560D30 ?defensive_check@@YAHH@Z 0x00560D30-0x00560D4D BYTE_EXACT
// size      29 bytes
// prototype int (__cdecl ?defensive_check@@YAHH@Z)(int baseID)
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00560B30
Return Value: Amount of defensive units needed (1-8)
Status: Complete
*/
int __cdecl defensive_check(int base_id) {
    int defenses = garrison_check(base_id);
    if (defenses > 5) {
        defenses--;
    }
    if (defenses > 2) {
        defenses--;
    }
    return defenses;
}

/*
Purpose: Determine if base is a port.
// ORIGINAL: 0x00579A00 ?is_port@@YAHHH@Z 0x00579A00-0x00579A30 BYTE_EXACT
// size      48 bytes
// prototype int (__cdecl ?is_port@@YAHHH@Z)(int baseID, int isBaseRadius)
// callers   2   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E49D0
Return Value: Is base a port? true/false
Status: Complete
*/
BOOL __cdecl is_port(int base_id, BOOL is_base_radius) {
    return is_coast(Bases[base_id].x, Bases[base_id].y, is_base_radius);
}

/*
Purpose: Calculate how vulnerable the coordinates are for the specified faction based on how far
         away this tile is from the faction's Headquarters.
// ORIGINAL: 0x0059E980 ?vulnerable@@YAHHHH@Z 0x0059E980-0x0059EA73
// size      243 bytes
// prototype int (__cdecl ?vulnerable@@YAHHHH@Z)(int factionID, int xCoord, int yCoord)
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0050BA00 0x00644F3A
// LEVER: open-coded the vector_dist(x, y, Bases[i].x, Bases[i].y) call as the
//        x_dist()+abs()x4 expansion (same shape as base_find). Best
//        similarity moved 0.595 -> 0.805 (/c /O2 /Oi- /Gy /GR- /Oy- /GX).
//        The bitmask side of the evidence (has_fac_built(FAC_HEADQUARTERS,i)
//        folding away the image's real bitmask() call) is the documented
//        systemic gap - TRIED on has_fac base.cpp:104 - and not
//        addressed here.
// LEVER: that note is superseded - the FAC_HEADQUARTERS test has a LITERAL
//        facility id, so routing it through has_fac_built_call (general.h
//        bitmask_call) reaches the image's real `call 0x50ba00`. Compiled
//        body 78 -> 92 instructions against the image's 96, differing runs
//        21 -> 17 (listing_diff, /c /O2 /Oi- /Gy /GR- /Oy- /GX).
Return Value: Radial distance between coordinates and faction's HQ or 12 if no HQ/bases
Status: Complete
*/
int __cdecl vulnerable(int faction_id, int x, int y) {
    int dist = 12; // default value for no bases or no HQ
    for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
        if (Bases[i].faction_id_current == faction_id && has_fac_built_call(FAC_HEADQUARTERS, i)) {
            // Open-coded vector_dist(x, y, Bases[i].x, Bases[i].y): the
            // image calls abs() four times, never vector_dist itself - see
            // del_site (0x00579E70) and base_find(x,y) (0x004E3B80) for the
            // same expansion.
            int dx = x_dist(x, Bases[i].x);
            int dy = abs(y - Bases[i].y);
            int abs_dx = abs(dx);
            int abs_dy = abs(dy);
            int largest = abs_dx;
            if (abs_dx <= abs_dy) {
                largest = abs_dy;
            }
            int smallest = abs_dx;
            if (abs_dx >= abs_dy) {
                smallest = abs_dy;
            }
            dist = largest - ((((abs_dy + abs_dx) >> 1) - smallest + 1) >> 1);
            break;
        }
    }
    return dist;
}

/*
Purpose: Determine whether the specified base is considered an objective.
// ORIGINAL: 0x005AC060 ?is_objective@@YAHH@Z 0x005AC060-0x005AC10B
// TRIED: the image still boolifies the `facilities_built[offset] & mask` test with a `neg/sbb/neg` 0-or-1 materialization into the old base_id stack slot before the final `je`, where this tree emits a plain `test`; hoisting `has_fac_built_call(...)` into its own named `BOOL` local before the `if` did not reproduce it (same 43/64, 0.927) - VC6 still proves the value is only used for one branch and drops the materialization.
// TRIED: `GameRules & X || Bases[base_id].event & Y` into two separate `if` statements (matching the image's two separate short-circuit branches instead of one `||`) moved 0.586 -> 0.762 similar. CORRECTION to the note this replaces: `calls agrees at exactly 1 (0x0050BA00, bitmask)` was wrong - the one call this tree made was to has_fac (out-of-line), not bitmask; the image inlines has_fac itself (its `cmp eax,0x41/jge` IS has_fac's own FacilityRepStart guard) and keeps ONLY the bitmask call inside that. Open-coding has_fac(ScnVictFacilityObj, base_id, 0) as its own `< FacilityRepStart` guard + has_fac_built() at the site removed the out-of-line has_fac call and moved best similarity 0.762 -> 0.783. `ScnVictFacilityObj` is a runtime facility id (read from a global, not a compile-time constant), so the folded `MEASURED inline bitmask` cannot constant-fold it away - routing it through `has_fac_built_call` (the real `bitmask()` call, same lever as cost_factor/pop_goal/num_objectives) reaches the call the image makes; call_diff agreed at 0/1 before, 1/1 after. Casting the `(int)ScnVictFacilityObj < FacilityRepStart` guard fixed a jge/jae signed-vs-unsigned split (ScnVictFacilityObj is uint32_t, compared unsigned without the cast). Moved 2/64 -> 43/64 agreeing, 0.586 -> 0.927 similar.
// TRIED: rewriting base_project()'s loop as an explicit pointer walk (`for (int *p = &SecretProject.human_genome_project; p < ... + MaxSecretProjectNum; p++)`) - the image's `cmp eax,0x9a6614/jl` reads like one, but the explicit walk makes VC6 test the pointer UNSIGNED (`jb`, 42/64) while the INDEX loop keeps the signed `jl` through strength reduction and test replacement (43/64, agreeing on `jl` exactly). Open-coding the bitmask call at block scope (int mask; int offset; bitmask_call(...); Bases[..].facilities_built[offset] & mask) instead of via `has_fac_built_call` also loses the whole shape - `push ecx` becomes `sub esp,0xc` and the image's `neg/sbb/neg` boolification never appears. Residual (listing_diff, 2 runs): the image materializes the `& mask` test 0-or-1 into base_id's own dead argument slot ([ebp+8]) with offset at [ebp-4]; this tree's inline has_fac_built assigns offset->[ebp-4] and mask->[ebp+8], the mirror image, and no source form tried at this call site flips it.
// size      171 bytes
// prototype int (__cdecl ?is_objective@@YAHH@Z)(int baseID)
// callers   14   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0050BA00
Return Value: Is base an objective? true/false
Status: Complete
*/
BOOL __cdecl is_objective(int base_id) {
    if (GameDataStateStorage.fields_.game_rules_ & RULES_SCN_VICT_ALL_BASE_COUNT_OBJ) {
        return true;
    }
    if (Bases[base_id].event & BEVENT_OBJECTIVE) {
        return true;
    }
    if (GameDataStateStorage.fields_.game_rules_ & RULES_SCN_VICT_SP_COUNT_OBJ) {
        for (int i = 0; i < MaxSecretProjectNum; i++) {
            if (base_project(i) == base_id) {
                return true;
            }
        }
    }
    if (GameDataStateStorage.fields_.game_state_ & STATE_SCN_VICT_BASE_FACIL_COUNT_OBJ) {
        // Open-coded has_fac(ScnVictFacilityObj, base_id, 0): with
        // queue_count == 0 it reduces to has_fac's own early-out guard
        // followed by has_fac_built(), and the image inlines exactly that
        // (its `cmp eax, 0x41 / jge` IS has_fac's `facility_id >=
        // FacilityRepStart` guard) rather than calling has_fac out of line.
        if ((int)GameDataStateStorage.fields_.scenario_victory_facility_ < FacilityRepStart
            && has_fac_built_call(GameDataStateStorage.fields_.scenario_victory_facility_, base_id)) {
            return true;
        }
    }
    return false;
}

/*
Purpose: Count how far the specified faction has got towards the scenario's
         objectives.
// ORIGINAL: 0x005AC110 ?num_objectives@@YAHHH@Z 0x005AC110-0x005AC599
// size      1161 bytes
// prototype int (__cdecl ?num_objectives@@YAHHH@Z)(int factionID, int)
// callers   9   call targets   5
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3A50 0x0050BA00 0x0050BA30 0x005AC110 0x005B9F20
// TRIED: routing has_fac_built(FAC_HEADQUARTERS, base_id) and/or
//            has_fac_built(ScnVictFacilityObj, base_id) through the new
//            `has_fac_built_call` helper (real bitmask() call, same lever
//            that won on pop_goal) measured WORSE here in all three
//            combinations tried (both sites, HQ only, ScnVictFacilityObj
//            only): best similarity across flag sets dropped from 0.201
//            (baseline, /c /O2 /Gy /GR- /Oy- /GX, 14/367 agreeing) to at
//            best 0.147. This function's size (1161 bytes, ~20 live locals
//            across two loops) likely means the extra out-of-line calls
//            shift register allocation enough to lose more than the call
//            shape gains. Left calling has_fac_built (folded) at both sites.
Return Value: Objective count, or zero for an eliminated faction
Status: Complete

Eleven separate scenario victory conditions add into one running total, each
gated by its own bit in GameRules or GameState, and a twelfth term - the count
the faction has already banked - is where the total starts. Nothing here is
cumulative across calls: the caller reads the return value.

The original inlines is_objective() (0x005AC060) at the objective-base test.
That is written as the call it was: the inlined sequence is instruction for
instruction the recovered body, down to has_fac()'s `>= FacilityRepStart`
guard appearing as `cmp eax, 0x41` and its zero queue_count folding the call to
has_fac_built(). The same holds for is_alive() at the entry and
has_fac_built(FAC_HEADQUARTERS) at the second unit test.

`unk_101` and `theory_of_everything` are read here under names that do not
describe what they hold; the offsets - 8324 and 872 into PlayerData - are what
the original reads, and neither field has a writer this recovery located.
*/
int __cdecl num_objectives(int faction_id, BOOL count_pact_factions) {
    if (!is_alive(faction_id)) {
        return 0;
    }
    int total = PlayersData[faction_id].unk_101;
    // Objective units that have reached a base that qualifies.
    for (int veh_id = 0; veh_id < GameDataStateStorage.fields_.vehicle_current_count_; veh_id++) {
        Veh *veh = &Vehs[veh_id];
        if (veh->faction_id != faction_id || !(veh->flags & VFLAG_IS_OBJECTIVE)) {
            continue;
        }
        int base_id;
        if (GameDataStateStorage.fields_.game_rules_ & RULES_SCN_VICT_OBJ_UNITS_REACH_FRIEND_OBJ_BASE) {
            base_id = base_at(veh->x, veh->y);
            if (base_id < 0 || !is_objective(base_id)) {
                continue;
            }
        } else {
            if (!(GameDataStateStorage.fields_.game_rules_ & RULES_SCN_VICT_OBJ_UNITS_REACH_FRIEND_HQ_BASE)) {
                total++; // neither condition is set: the unit itself counts
                continue;
            }
            base_id = base_at(veh->x, veh->y);
            if (base_id < 0 || !has_fac_built(FAC_HEADQUARTERS, base_id)) {
                continue;
            }
        }
        int owner = Bases[base_id].faction_id_current;
        if (owner == faction_id) {
            total++;
        } else if ((PlayersData[faction_id].diplo_treaties[owner] & DTREATY_PACT)
            && (GameDataStateStorage.fields_.game_rules_ & RULES_VICTORY_COOPERATIVE)) {
            total++; // a pact partner's base counts under 'One For All'
        }
    }
    // Population, and bases that are objectives in their own right.
    for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
        if (Bases[base_id].faction_id_current != faction_id) {
            continue;
        }
        if (GameDataStateStorage.fields_.game_state_ & STATE_SCN_VICT_POPULATION_COUNT_OBJ) {
            total += Bases[base_id].population_size;
        }
        if (!(GameDataStateStorage.fields_.game_rules_ & RULES_SCN_VICT_ALL_BASE_COUNT_OBJ)
            && !(Bases[base_id].event & BEVENT_OBJECTIVE)) {
            continue;
        }
        total++;
    }
    if (GameDataStateStorage.fields_.game_state_ & STATE_SCN_VICT_TECH_COUNT_OBJ) {
        for (int tech_id = 0; tech_id < MaxTechnologyNum; tech_id++) {
            if (has_tech(tech_id, faction_id)) {
                total++;
            }
        }
        total += PlayersData[faction_id].theory_of_everything;
    }
    if (GameDataStateStorage.fields_.game_state_ & STATE_SCN_VICT_CREDITS_COUNT_OBJ) {
        total += PlayersData[faction_id].energy_reserves;
    }
    if (GameDataStateStorage.fields_.game_state_ & STATE_SCN_VICT_BASE_FACIL_COUNT_OBJ) {
        for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
            if (Bases[base_id].faction_id_current == faction_id
                && (int)GameDataStateStorage.fields_.scenario_victory_facility_ < FacilityRepStart
                && has_fac_built(GameDataStateStorage.fields_.scenario_victory_facility_, base_id)) {
                total++;
            }
        }
    }
    /*
    * Both map walks step one Map along per tile visited rather than indexing,
    * because a row holds only every other x and the rows pack together; the
    * pointer is not reset between rows.
    */
    if (GameDataStateStorage.fields_.game_state_ & STATE_SCN_VICT_TERRAIN_ENH_COUNT_OBJ) {
        Map *tile = map_tiles();
        for (int y = 0; y < MapLatitudeBounds; y++) {
            for (int x = y & 1; x < MapLongitudeBounds; x += 2) {
                if (tile->territory == faction_id) {
                    total += bit_count(tile->bit & 0x81E4885C);
                }
                tile++;
            }
        }
    }
    if (GameDataStateStorage.fields_.game_state_ & STATE_SCN_VICT_TERRITORY_COUNT_OBJ) {
        Map *tile = map_tiles();
        for (int y = 0; y < MapLatitudeBounds; y++) {
            for (int x = y & 1; x < MapLongitudeBounds; x += 2) {
                if (tile->territory == faction_id) {
                    total++;
                }
                tile++;
            }
        }
    }
    if (GameDataStateStorage.fields_.game_rules_ & RULES_SCN_VICT_SP_COUNT_OBJ) {
        for (int i = 0; i < MaxSecretProjectNum; i++) {
            int base_id = base_project(i);
            if (base_id >= 0 && Bases[base_id].faction_id_current == faction_id) {
                total++;
            }
        }
    }
    // One recursion deep only: the pact partners are asked with the flag off.
    if (count_pact_factions && (GameDataStateStorage.fields_.game_rules_ & RULES_VICTORY_COOPERATIVE)) {
        for (int other_id = 1; other_id < MaxPlayerNum; other_id++) {
            if (other_id != faction_id
                && (PlayersData[faction_id].diplo_treaties[other_id] & DTREATY_PACT)) {
                total += num_objectives(other_id, false);
            }
        }
    }
    return total;
}

/*
Purpose: Check if specified faction is currently building Ascent to Transcendence. This code isn't
         used by original game. There was also a bug where it compares to a non-negative queue id.
// ORIGINAL: 0x005AC630 ?transcending@@YAHH@Z 0x005AC630-0x005AC67A BYTE_EXACT
// size      74 bytes
// prototype int (__cdecl ?transcending@@YAHH@Z)(int factionID)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Is faction transcending? true/false
Status: Complete
*/
BOOL __cdecl transcending(int faction_id) {
    if (!ascending(faction_id)) {
        return false;
    }
    for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
        // NOT NEGATED, and that is the image rather than a slip here. The
        // bytes at 0x005AC65C are `83 38 66` - `cmp dword ptr [eax], 0x66` -
        // a POSITIVE 102, where every other site in the tree writes
        // `-FAC_ASCENT_TO_TRANSCENDENCE` for a queued facility.
        //
        // The original disagrees with itself: `at_climax` (0x00539EF0) tests
        // the same facility with `cmp dword ptr [eax + 0x10], -0x66`, the
        // negated form. One of the two shipped wrong, and by the convention
        // the rest of the game follows - facilities queued as negative ids -
        // it is this one. Reproduced as shipped, the same way
        // `Probe::success_rate` reproduces its own halving bug: this is a
        // matching decompilation, not a fix.
        if (Bases[i].faction_id_current == faction_id
            && Bases[i].queue_production_id[0] == FAC_ASCENT_TO_TRANSCENDENCE) {
            return true;
        }
    }
    return false;
}

/*
Purpose: Check if Voice of Planet has been built that starts the Ascent to Transcendence sequence.
// ORIGINAL: 0x005AC680 ?ascending@@YAHH@Z 0x005AC680-0x005AC68F BYTE_EXACT
// size      15 bytes
// prototype int (__cdecl ?ascending@@YAHH@Z)(int factionID)
// callers   4   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Has Voice of Planet been built? true/false
Status: Complete
*/
// BODY IN base.h, as `MEASURED inline`: facility_avail (base.cpp) needs it
// folded in place - the image never calls 0x005AC680 there.

/*
Purpose: Determine if the facility is redundant due to a Secret Project counting as that facility.
// ORIGINAL: 0x005BA030 ?redundant@@YAHHH@Z 0x005BA030-0x005BA0DD
// size      173 bytes
// prototype int (__cdecl ?redundant@@YAHHH@Z)(int facilityID, int factionID)
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// LEVER: a `switch` assigning ONE shared `project_id` local, checked once at
//        a single `return has_project(project_id, faction_id)`, keeps
//        project_id a RUNTIME value - VC6 cannot fold `base_project()`'s
//        address per case, and 7 cases in a numeric range compile to a jump
//        table besides, where the image cascades explicit compares
//        (FAC_NAVAL_YARD checked first and alone, with its own duplicated
//        tail, before the rest). Returning the has_project() call directly
//        from each arm makes project_id a COMPILE-TIME literal at every
//        call site, so each one folds to the fixed global the image reads
//        (0x9a653c, 0x9a6528, ...), and if/else instead of switch avoids
//        the jump table. Moved 11/61 (0.478) -> 16/61 (0.950).
// TRIED: pulling the image's shared normalizing tail out (`test eax,eax / je
//        / mov eax,1 / xor eax,eax` at 0x5ba0ce, jumped at by arm 0 and
//        fallen into by arms 1-6) with three spellings - `return
//        has_project(...) ? TRUE : FALSE;` per arm; a `BOOL built`
//        accumulator assigned down one if/else-if chain with `return built ?
//        TRUE : FALSE;` after; `BOOL built` per arm + `return built;`. ALL
//        THREE lose VC6's inline expansion of has_project (a real `call` per
//        arm, 49-119 instructions against the image's 61). Only the bare
//        `return has_project(...)` keeps the body written out seven times,
//        and that form returns the inlined `sete` straight away, so the
//        image's re-normalize-through-a-branch tail never appears. Also
//        measured: a `static __forceinline` twin of has_project is IGNORED
//        by VC6 at every one of those call-site shapes (2, 3 and 7 arms)
//        while the identical `static __forceinline` has_fac_built_call IS
//        honoured in is_objective - the twin's own body calls a `MEASURED
//        inline` (dllexport) function, which the other does not.
Return Value: Is facility redundant? true/false
Status: Complete
*/
BOOL __cdecl redundant(int facility_id, int faction_id) {
    if (facility_id == FAC_NAVAL_YARD) {
        return has_project(SP_MARITIME_CONTROL_CENTER, faction_id);
    }
    if (facility_id == FAC_PERIMETER_DEFENSE) {
        return has_project(SP_CITIZENS_DEFENSE_FORCE, faction_id);
    }
    if (facility_id == FAC_COMMAND_CENTER) {
        return has_project(SP_COMMAND_NEXUS, faction_id);
    }
    if (facility_id == FAC_BIOENHANCEMENT_CENTER) {
        return has_project(SP_CYBORG_FACTORY, faction_id);
    }
    if (facility_id == FAC_QUANTUM_CONVERTER) {
        return has_project(SP_SINGULARITY_INDUCTOR, faction_id);
    }
    if (facility_id == FAC_AEROSPACE_COMPLEX) {
        return has_project(SP_CLOUDBASE_ACADEMY, faction_id);
    }
    if (facility_id == FAC_ENERGY_BANK) {
        return has_project(SP_PLANETARY_ENERGY_GRID, faction_id);
    }
    return false;
}

/*
Purpose: Determine if the provided faction can build a specific facility or Secret Project in the
         specified base. Checks are included to prevent SMACX specific facilities from being built
         in SMAC mode.
// ORIGINAL: 0x005BA0E0 ?facility_avail@@YAHHHHH@Z 0x005BA0E0-0x005BA90A
// size      2090 bytes
// prototype int (__cdecl ?facility_avail@@YAHHHHH@Z)(int facilityID, int factionID, int baseID, int queueCount)
// callers   5   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00421670 0x004E49D0 0x004F80D0 0x0050BA00 0x00645470 0x00645660
// indirect  0x005BA213
// LEVER: has_tech's body moved from technology.cpp to the end of
//        technology.h, and ascending's from base.cpp to the end of base.h,
//        both as `MEASURED inline` - both already had their own bodies
//        (`__forceinline` for has_tech, a plain out-of-line BYTE_EXACT
//        function for ascending), but neither satisfied inlining for a
//        caller in a DIFFERENT translation unit, and the image open-codes
//        both of them here (has_tech's whole preq_tech walk, and
//        ascending's single base_project() != SP_Unbuilt compare). Best
//        similarity 0.137, agreeing instructions 20->25 of 742. The ten
//        `call 0x50ba00` (bitmask) sites the image keeps are the documented
//        systemic has_fac_built(literal,...) folding, TRIED on has_fac
//        base.cpp:104, and are not fixable from this site.
Return Value: Is facility or Secret Project available to faction, base and game mode? true/false
Status: Complete
*/
BOOL __cdecl facility_avail(int facility_id, int faction_id, int base_id,
                            int queue_count) {
    // initial checks
    if (!facility_id || (facility_id == FAC_SKUNKWORKS && GameDataStateStorage.fields_.diff_level_current_ <= DLVL_SPECIALIST)
        || (facility_id >= FAC_HUMAN_GENOME_PROJ && GameDataStateStorage.fields_.game_rules_ & RULES_SCN_NO_BUILDING_SP)) {
        return false; // Skunkworks removed if there are no prototype costs
    }
    if (facility_id == FAC_ASCENT_TO_TRANSCENDENCE) { // at top since anyone can build it
        return ascending(faction_id) && GameDataStateStorage.fields_.game_rules_ & RULES_VICTORY_TRANSCENDENCE
            && _stricmp(Players[faction_id].filename, "CARETAKE"); // bug fix for Caretakers
    }
    if (!has_tech(Facility[facility_id].preq_tech, faction_id)) { // Check tech for facility + SP
        return false;
    }
    // Secret Project availability
    if (!ExpansionEnabled && (facility_id == FAC_MANIFOLD_HARMONICS
        || facility_id == FAC_NETHACK_TERMINUS || facility_id == FAC_CLOUDBASE_ACADEMY
        || facility_id == FAC_PLANETARY_ENERGY_GRID)) {
        return false;
    }
    if (facility_id == FAC_VOICE_OF_PLANET && !_stricmp(Players[faction_id].filename, "CARETAKE")) {
        return false; // shifted Caretaker Ascent check to top (never reached here)
    }
    if (facility_id >= FAC_HUMAN_GENOME_PROJ) {
        return base_project(facility_id - FAC_HUMAN_GENOME_PROJ) == SP_Unbuilt;
    }
    // Facility availability
    if (base_id < 0) {
        return true;
    }
    if (has_fac(facility_id, base_id, queue_count)) {
        return false; // already built or in queue
    }
    if (redundant(facility_id, faction_id)) {
        return false; // has SP that counts as facility
    }
    switch (facility_id) { // consolidated into switch to improve readability and performance
      case FAC_RECYCLING_TANKS:
        return !has_fac(FAC_PRESSURE_DOME, base_id, queue_count); // count as Recycling Tank, skip
      case FAC_TACHYON_FIELD:
        return has_fac(FAC_PERIMETER_DEFENSE, base_id, queue_count)
            || has_project(SP_CITIZENS_DEFENSE_FORCE, faction_id); // Cumulative
      case FAC_SKUNKWORKS:
        return !(Players[faction_id].rule_flags & RFLAG_FREEPROTO); // no prototype costs? skip
      case FAC_HOLOGRAM_THEATRE:
        return has_fac(FAC_RECREATION_COMMONS, base_id, queue_count) // not documented in manual
            && !has_project(SP_VIRTUAL_WORLD, faction_id); // Network Nodes replaces Theater
      case FAC_HYBRID_FOREST:
        return has_fac(FAC_TREE_FARM, base_id, queue_count); // Cumulative
      case FAC_QUANTUM_LAB:
        return has_fac(FAC_FUSION_LAB, base_id, queue_count); // Cumulative
      case FAC_NANOHOSPITAL:
        return has_fac(FAC_RESEARCH_HOSPITAL, base_id, queue_count); // Cumulative
      case FAC_PARADISE_GARDEN: // bug fix: added check
        return !has_fac(FAC_PUNISHMENT_SPHERE, base_id, queue_count); // antithetical
      case FAC_PUNISHMENT_SPHERE:
        return !has_fac(FAC_PARADISE_GARDEN, base_id, queue_count); // antithetical
      case FAC_NANOREPLICATOR:
        return has_fac(FAC_ROBOTIC_ASSEMBLY_PLANT, base_id, queue_count) // Cumulative
            || has_fac(FAC_GENEJACK_FACTORY, base_id, queue_count);
      case FAC_HABITATION_DOME:
        return has_fac(FAC_HAB_COMPLEX, base_id, queue_count); // must have Complex
      case FAC_TEMPLE_OF_PLANET:
        return has_fac(FAC_CENTAURI_PRESERVE, base_id, queue_count); // must have Preserve
      case FAC_QUANTUM_CONVERTER:
        return has_fac(FAC_ROBOTIC_ASSEMBLY_PLANT, base_id, queue_count); // Cumulative
      case FAC_NAVAL_YARD:
        return is_coast(Bases[base_id].x, Bases[base_id].y, false); // needs ocean
      case FAC_AQUAFARM:
      case FAC_SUBSEA_TRUNKLINE:
      case FAC_THERMOCLINE_TRANSDUCER:
        return ExpansionEnabled && is_coast(Bases[base_id].x, Bases[base_id].y, false);
      case FAC_COVERT_OPS_CENTER:
      case FAC_BROOD_PIT:
      case FAC_FLECHETTE_DEFENSE_SYS:
        return ExpansionEnabled;
      case FAC_GEOSYNC_SURVEY_POD: // SMACX only & must have Aerospace Complex
        return ExpansionEnabled && (has_fac(FAC_AEROSPACE_COMPLEX, base_id, queue_count)
            || has_project(SP_CLOUDBASE_ACADEMY, faction_id)
            || has_project(SP_SPACE_ELEVATOR, faction_id));
      case FAC_SKY_HYDRO_LAB:
      case FAC_NESSUS_MINING_STATION:
      case FAC_ORBITAL_POWER_TRANS:
      case FAC_ORBITAL_DEFENSE_POD:  // must have Aerospace Complex
        return has_fac(FAC_AEROSPACE_COMPLEX, base_id, queue_count)
            || has_project(SP_CLOUDBASE_ACADEMY, faction_id)
            || has_project(SP_SPACE_ELEVATOR, faction_id);
      case FAC_SUBSPACE_GENERATOR: // Progenitor factions only
        return is_alien_faction(faction_id);
      default:
        break;
    }
    return true;
}

/*
Purpose: To assist with locating a facility based on a name rather than using hardcoded offsets.
Original Offset: n/a
Return Value: Offset value otherwise -1 if not found
Status: Complete
*/
int __cdecl facility_offset(LPCSTR facil_search) {
    int offset = -1;
    for (uint32_t i = 0; i < MaxFacilityNum; i++) {
        LPSTR fac_name = Facility[i].name;
        if (fac_name && !_stricmp(facil_search, fac_name)) {
            offset = i;
            break;
        }
    }
    return offset;
}

// ORIGINAL: 0x004E7DC0 ?resource_yield@@YAHHHHHH@Z 0x004E7DC0-0x004E7E34 FILE BYTE_EXACT
// LEVER: TileYieldRestricted (0x0090E998, already declared in base.h) replaces
//        the carried `g_0090e998` binding; the scaffold's `g_004e7e34` was the
//        function's own end address, referenced by nothing, and is dropped.
// size      116 bytes
// prototype int (__cdecl ?resource_yield@@YAHHHHHH@Z)(int type, int factionID, int baseID, int xCoord, int yCoord)
// callers   7   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E6E50 0x004E7310 0x004E7750
// Purpose: Dispatch one of the three terrain yields for the base's current
//          square, or - for RSC_PSI (3), which has no per-square yield -
//          clear TileYieldRestricted and return 0.
// Status:  transcription homed from src/unrecovered/004e7dc0.cpp

int __cdecl resource_yield(int rsc_type, int faction_id, int base_id, int x,
                           int y) {
    switch (rsc_type) {
        case 0:
            return crop_yield(faction_id, base_id, x, y, 0);
        case 1:
            return mine_yield(faction_id, base_id, x, y, 0);
        case 2:
            return energy_yield(faction_id, base_id, x, y, 0);
        case 3:
            TileYieldRestricted = 0;
            break;
    }
    return 0;
}

// ORIGINAL: 0x005AC5A0 ?most_objectives@@YAHPAHPAH@Z 0x005AC5A0-0x005AC62F FILE BYTE_EXACT
// LEVER: the scaffold's two carried bindings are both modelled names already
//        in the tree - 0x0096C9F8 is faction.h's PlayersData[0].diplo_treaties
//        (the array front 0x0096C9E0 + 0x18) and 0x009A649C is game.h's
//        GameRules - so the raw `[best_idx + off]` indexing over the 0x833
//        dword stride becomes PlayersData[faction_id].diplo_treaties[best].
// symbol    ?most_objectives@@YAHPAH0@Z
// LEVER: the image spells the same two `int *` parameters out in full
//        (@YAHPAHPAH@Z); VC6 abbreviates the repeat, so this tree's own symbol
//        is the `0` form. Same type, same address, shorter mangled name.
// size      143 bytes
// prototype
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005AC110
// Purpose: Score the cooperative-victory winner: the faction with the most
//          current objectives. When RULES_VICTORY_COOPERATIVE is set the
//          second out-parameter also counts how many factions are tied with
//          the leader AND hold a pact (DTREATY_PACT) with it; the caller
//          awards the shared win when that count reaches the tie count.
// Status:  transcription homed from src/unrecovered/005ac5a0.cpp

int __cdecl most_objectives(int *faction_id_out, int *pact_count_out) {
    int best_count = 0;
    int best_faction = 0;
    if (pact_count_out != 0) {
        *pact_count_out = 0;
    }
    int faction_id = 1;
    do {
        int count = num_objectives(faction_id, GameDataStateStorage.fields_.game_rules_ & RULES_VICTORY_COOPERATIVE);
        if (count >= best_count) {
            if (pact_count_out != 0) {
                if (count > best_count) {
                    *pact_count_out = 0;
                } else if ((PlayersData[faction_id].diplo_treaties[best_faction] & DTREATY_PACT) == 0
                           || (GameDataStateStorage.fields_.game_rules_ & RULES_VICTORY_COOPERATIVE) == 0) {
                    ++*pact_count_out;
                }
            }
            best_count = count;
            best_faction = faction_id;
        }
        ++faction_id;
    } while (faction_id < MaxPlayerNum);
    if (faction_id_out != 0) {
        *faction_id_out = best_faction;
    }
    return best_count;
}


// ORIGINAL: 0x004EFD50 ?upgrade_cost@@YAHHHH@Z 0x004EFD50-0x004EFE7F FILE
// TRIED: original establishes a true EBP frame (flags: frame) because it spills the reassigned `protoIDInit` parameter to its own stack slot; several transliterations (ternary vs if/else for the augmentation, ternary vs branch for the three clamps, B-then-A vs A-then-B table read order) all landed the value in a GP register (ebp used as scratch, no `mov ebp, esp`) instead. Landed at MISMATCH #3 (first diff is the missing frame prologue); the rest of the instruction stream already lines up closely (same table indices, same clamp-via-mask shape for two of three diffs).
// LEVER: the four fixed-address tables the artifact bound are modelled rules
//        records - 0x009AB868+0x29/.30/.25/.26/.24 is VehPrototypes[]
//        cost/flags/weapon_id/armor_id/chassis_id, 0x0094AE6B is
//        Weapon[].cost, 0x0094F282 is Armor[].cost, 0x0094A378 is
//        Chassis[].speed, 0x009A656C is base_project(SP_NANO_FACTORY) and
//        0x0097D044 is Bases[].faction_id_current - so every `g_9xxxxx`
//        binding drops out in favour of a member read.
// TRIED: writing the `proto_id_init = cost` reassignment the archive spells,
//        to force the image's early store into the dead parameter's home
//        slot - see the BUG IN THE ORIGINAL note below; the honest form is
//        what is kept here.
// TRIED: named `init_slot`/`upg_slot` locals for the two prototype indices, to
//        break VC6's common-subexpression of `&VehPrototypes[slot]` - the image
//        keeps `slot * 0x34` in a register with the TABLE BASE as the
//        displacement (`mov cl, [esi + 0x9ab891]`), where this tree keeps the
//        record pointer in the register with the FIELD OFFSET as the
//        displacement (`mov dl, [ecx + 0x29]`). Same bytes modulo the
//        relocation, different register choice, and the divergence cascades
//        through all five field reads: 3 of 105 either way. Reproducing the
//        image's shape needs `slot * 0x34` spelled as a byte offset, which only
//        a cast of the array base spells; naming `init_slot` scored no better
//        than using the parameters directly, so the parameter form is kept.
// size      303 bytes
// prototype int (__cdecl ?upgrade_cost@@YAHHHH@Z)(int factionID, int protoIDInit, int protoIDUpg)
// callers   3   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// Purpose: Price one prototype upgrade for a faction: the base prototype's
//          mineral cost (augmented 1.5x and squared when the slot is past the
//          64-slot shared block and carries no VEHFLAG bit 4), plus the
//          positive deltas of the weapon and armour costs and the chassis
//          speed, halved when the faction that owns SP_NANO_FACTORY is asking.
// BUG IN THE ORIGINAL, AS TRANSCRIBED IN THE ARCHIVE: the archive test is
//          `cost > 0x3f`, because it reuses the parameter slot for the cost
//          before the test. The image compares EAX - the prototype ID as the
//          caller passed it, loaded at 0x004EFD53 and never reloaded - against
//          0x40. A caller passing a cheap prototype in a slot past 0x40 pays
//          the unaugmented price here. The comparison is spelled against the
//          prototype ID, which is what the image does; only the parameter
//          reuse is not reproduced.
// Status:  transcription homed from src/unrecovered/004efd50.cpp

int __cdecl upgrade_cost(int faction_id, int proto_id_init, int proto_id_upg) {
    int init_slot = proto_id_init;
    int upg_slot = proto_id_upg;
    int cost = VehPrototypes[init_slot].cost;
    if (init_slot >= 0x40 && (VehPrototypes[init_slot].flags & 4) == 0) {
        int half = (cost + 1) / 2;
        int scaled = cost + half;
        cost = (scaled + 1) * scaled;
    }
    int weapon_diff = Weapon[VehPrototypes[upg_slot].weapon_id].cost
                      - Weapon[VehPrototypes[init_slot].weapon_id].cost;
    if (weapon_diff < 0) {
        weapon_diff = 0;
    }
    int armor_diff = Armor[VehPrototypes[upg_slot].armor_id].cost
                     - Armor[VehPrototypes[init_slot].armor_id].cost;
    if (armor_diff < 0) {
        armor_diff = 0;
    }
    int chassis_diff = Chassis[VehPrototypes[upg_slot].chassis_id].speed
                       - Chassis[VehPrototypes[init_slot].chassis_id].speed;
    if (chassis_diff < 0) {
        chassis_diff = 0;
    }
    cost += weapon_diff + armor_diff + chassis_diff;
    int nano_factory_base = base_project(SP_NANO_FACTORY);
    if (nano_factory_base >= 0
        && Bases[nano_factory_base].faction_id_current == faction_id) {
        cost /= 2;
    }
    return cost;
}

// ORIGINAL: 0x004F0650 ?upgrade_any_prototypes@@YAXH@Z 0x004F0650-0x004F06D3 FILE
// LEVER: the four carried bindings are all modelled names - 0x0090EA3C is the
//        upgrade-confirmation flag (see UpgradePromptAnswered below),
//        0x009A6494 game.h's GameMorePreferences, 0x009A64E8 lock.h's
//        LockEnableMask read as its byte 0 (the is-human bitfield) and
//        0x009AB898 VehPrototypes[].flags - so no fixed address survives.
// TRIED: `int flags`, `uint16_t flags`, `(uint16_t)(flags & 0x100)` and a bare
//        `flags & 0x100` condition for the third bit test - the image spells it
//        `and ecx, 0x100 / test cx, cx` (two instructions) and this tree spells
//        every one of those source forms as `test ch, 1` (one). 46 of 47, one
//        instruction short, on all four spellings; the 16-bit `and`/`test` pair
//        is an idiom this source cannot be pushed to.
// size      131 bytes
// prototype void (__cdecl ?upgrade_any_prototypes@@YAXH@Z)(int factionID)
// callers   1   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004F0460
// Purpose: The turn-start automatic upgrade sweep. Clears the
//          upgrade-confirmation flag, then - for a faction that is human, or
//          whose "ask before upgrading" preference is off - walks all 128
//          prototype slots and upgrades every obsolete one still in service
//          (flags: in-use, bit 4, and 0x100).

void __cdecl upgrade_any_prototypes(int faction_id) {
    UpgradePromptAnswered = 0;
    if (((1 << faction_id) & (int)(unsigned char)GameDataStateStorage.fields_.lock_masks_.enable_mask_) == 0
        || (GameDataStateStorage.fields_.game_more_preferences_ & 0x100000) != 0) {
        int i = 0;
        do {
            int proto_id = i;
            if (proto_id >= 0x40) {
                proto_id = proto_id % 0x40 + faction_id * 0x40;
            }
            uint16_t flags = VehPrototypes[proto_id].flags;
            if ((flags & 1) != 0 && (flags & 4) != 0 && (flags & 0x100) != 0) {
                upgrade_prototypes(faction_id, proto_id);
            }
            ++i;
        } while (i < 0x80);
    }
}

// ORIGINAL: 0x004F0460 ?upgrade_prototypes@@YAXHH@Z 0x004F0460-0x004F064F FILE
// LEVER: every table the archive bound by address is a named model record -
//        0x009AB868+0x24..0x30 is VehPrototypes[] chassis/weapon/armor/plan/
//        unk_1/obsolete_factions/flags, 0x0094AE68 Weapon[].offense_rating,
//        0x0094F280 Armor[].defense_rating, 0x0094A378/.9 Chassis[]
//        .speed/.triad - and 0x0096D238 is veh.cpp's FactionProtoSeenCount.
// TRIED: spelling the `has_abil` argument as the five ABL_ bits the image's
//        0x408418 literal is the OR of (POLICE_2X | ARTILLERY | ANTIGRAV_STRUTS
//        | DROP_POD | AMPHIBIOUS); kept as the explicit OR of the enum.
// LEVER: the weapon guard is `!= 0` with the rating test INLINE and the
//        `== 0` weapon-id test as the ELSE arm - the image's `je` jumps
//        forward over the rating compare to the weapon-id compare. Spelling it
//        the archive's way (`== 0` first, else the rating test) inverted the
//        branch and cost four instructions. Swapping the two has_abil calls
//        (proto_id first, then the slot) fixed the last operand-order
//        divergence: 130/141 -> SHAPE_EXACT 140/141.
// TRIED: `flags &= ~0x100` as a word write - the image narrows it to a single
//        `and byte ptr [.. + 0x9ab899], 0xfe` (the flags field's HIGH byte
//        only); this tree keeps `and word ptr [.. + 0x30], 0xfeff`. That is
//        the one instruction between this and BYTE_EXACT, and reaching it
//        needs a byte-typed view of the field's second byte.
// size      495 bytes
// prototype void (__cdecl ?upgrade_prototypes@@YAXHH@Z)(int factionID, int protoID)
// callers   1   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004EFE80 0x004F0460 0x00625E50
// Purpose: Upgrade one prototype to its current-era replacement: clear the
//          prototype's 0x100 "obsolete" marker, then walk the faction's 128
//          prototype slots for a same-plan, same-chassis successor that is no
//          worse in weapon, armour, speed or triad, and upgrade every unit of
//          the old design to it.
// BUG IN THE ORIGINAL: the image calls the five-parameter
//          ?upgrade_prototype@@YAHHHHH@Z here with four pushes (add esp, 0x10
//          at 0x004F0620), so the callee's fifth parameter is read off the
//          caller's stack. Reproduced as a four-parameter call; see
//          pending_bodies.cpp for the forwarder that spells it the same way.

void __cdecl upgrade_prototypes(int faction_id, int proto_id) {
    int self_offset = proto_id * 0x34;
    VehPrototypes[proto_id].flags &= ~0x100;
    for (int i = 0; i < 0x80; i++) {
        int other = i;
        if (other >= 0x40) {
            other = other % 0x40 + faction_id * 0x40;
        }
        if (other == proto_id) {
            continue;
        }
        if ((VehPrototypes[other].flags & 1) == 0) {
            continue;
        }
        if ((VehPrototypes[other].obsolete_factions & (1 << faction_id)) == 0) {
            continue;
        }
        if (VehPrototypes[other].plan != VehPrototypes[proto_id].plan) {
            continue;
        }
        if (VehPrototypes[other].unk_1 != VehPrototypes[proto_id].unk_1) {
            continue;
        }
        if (Chassis[VehPrototypes[other].chassis_id].triad
            != Chassis[VehPrototypes[proto_id].chassis_id].triad) {
            continue;
        }
        if (FactionProtoSeenCount[faction_id * 0x20cc + other] == 0) {
            continue;
        }
        int8_t self_weapon = Weapon[VehPrototypes[proto_id].weapon_id].offense_rating;
        if (self_weapon != 0) {
            if (self_weapon > Weapon[VehPrototypes[other].weapon_id].offense_rating) {
                continue;
            }
        } else if (VehPrototypes[proto_id].weapon_id != VehPrototypes[other].weapon_id) {
            continue;
        }
        if (has_abil(proto_id, ABL_POLICE_2X | ABL_ARTILLERY | ABL_ANTIGRAV_STRUTS
                                  | ABL_DROP_POD | ABL_AMPHIBIOUS)
            != has_abil(other, ABL_POLICE_2X | ABL_ARTILLERY | ABL_ANTIGRAV_STRUTS
                             | ABL_DROP_POD | ABL_AMPHIBIOUS)) {
            continue;
        }
        if (Armor[VehPrototypes[proto_id].armor_id].defense_rating
            > Armor[VehPrototypes[other].armor_id].defense_rating) {
            continue;
        }
        if (Chassis[VehPrototypes[proto_id].chassis_id].speed
            > Chassis[VehPrototypes[other].chassis_id].speed) {
            continue;
        }
        if (VehPrototypes[proto_id].chassis_id != VehPrototypes[other].chassis_id) {
            continue;
        }
        upgrade_prototype(faction_id, proto_id, other, 1);
    }
}

/*
 * THE FOUR BASE-SITE CLAIM POINTS the AI records for a base it is founding.
 * BaseSiteClaimX[k] / BaseSiteClaimY[k] hold the x,y of the kth site
 * base_build() picked for the base - Y starts at 999999, meaning "none yet".
 * base_reset (0x004E5FE0) fills them with the base's state bits
 * BSTATE_UNK_1000000 / _2000000 / _4000000 / _8000000 set one at a time, and
 * base_claimed (0x004F8110) tests a tile against them.
 */
int BaseSiteClaimX[4];  // 0x0090EA44
int BaseSiteClaimY[4];  // 0x0090EA54

// ORIGINAL: 0x004F8110 ?base_claimed@@YAHHHHHHH@Z 0x004F8110-0x004F8195 FILE BYTE_EXACT
// LEVER: the scaffold's eight carried bindings are two four-int runs - the
//        base-site claim points base_reset writes and this function tests - so
//        they became the arrays BaseSiteClaimX[4] / BaseSiteClaimY[4] rather
//        than eight address-named scalars.
// LEVER: the six `aN` parameters are the tile and its four site-freedom flags;
//        naming them cleared the last scaffold names this body carried.
// size      133 bytes
// prototype int (__cdecl ?base_claimed@@YAHHHHHHH@Z)(int x, int y, int site1_free, int site2_free, int site3_free, int site4_free)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// Purpose: Is this tile one of the four sites the AI has claimed for a base?
//          A site counts only while its flag is still clear and only at or
//          below the recorded y - the AI claims southward from the site.
// Status:  transcription homed from src/unrecovered/004f8110.cpp

int __cdecl base_claimed(int x, int y, int site1_free, int site2_free,
                         int site3_free, int site4_free) {
    if (site1_free == 0 && x == BaseSiteClaimX[0] && y >= BaseSiteClaimY[0]) {
        return 1;
    }
    if (site2_free == 0 && x == BaseSiteClaimX[1] && y >= BaseSiteClaimY[1]) {
        return 1;
    }
    if (site3_free == 0 && x == BaseSiteClaimX[2] && y >= BaseSiteClaimY[2]) {
        return 1;
    }
    if (site4_free == 0 && x == BaseSiteClaimX[3] && y >= BaseSiteClaimY[3]) {
        return 1;
    }
    return 0;
}

// ORIGINAL: 0x005ABFF0 ?get_rating@@YAHHH@Z 0x005ABFF0-0x005AC051 FILE BYTE_EXACT
// LEVER: the archive's `g_faction_table[a1 * 2099]` is PlayersData's
//        diff_level - the image loads `[edx*4 + 0x96c9e8]` with edx =
//        faction * 0x833, which is PlayerData + 0x8 - and its `g_009a649c`
//        byte test is game.h's GameRules bit 0x40, RULES_INTENSE_RIVALRY.
//        Both named; nothing new declared.
// size      97 bytes
// prototype int (__cdecl ?get_rating@@YAHHH@Z)(int factionID, int)
// callers   2   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// Purpose: A faction's score weight: its difficulty level plus the triangular
//          series the higher levels add, one more when RULES_INTENSE_RIVALRY
//          is on, scaled by arg2 over 0xa0.
// Status:  transcription homed from src/unrecovered/005abff0.cpp

int __cdecl get_rating(int faction_id, int arg2) {
    int value = PlayersData[faction_id].diff_level;
    int rating = value;
    for (int i = 3; i <= value; i++) {
        rating += (i - 1) / 2;
    }
    if (*(unsigned char *)&GameDataStateStorage.fields_.game_rules_ & 0x40) {
        rating++;
    }
    return (rating + 6) * arg2 / 0xa0;
}

// ORIGINAL: 0x004EFB50 ?do_upgrade@@YAXHHH@Z 0x004EFB50-0x004EFD4A FILE
// LEVER: every fixed address the archive bound is a named model member -
//        0x0096D238 is veh.cpp's FactionProtoSeenCount, 0x0096CC00 is
//        faction.h's PlayersData[].energy_reserves, 0x009A64C8 is console.h's
//        ConsoleGroupCount, 0x0095284C/.+0x0A/.+0x0E/.+0x24 are Vehs[]
//        proto_id/faction_id/morale read through the record, 0x0097D08C/.-0x48
//        are Bases[] queue_size/faction_id_current, 0x009AB898 is
//        VehPrototypes[].flags, 0x009A64E8 is lock.h's LockEnableMask,
//        0x00939284 is game.h's LocalFaction and 0x009156B0 is console.h's
//        ConsoleGlobal. Nothing new declared but draw_map below.
// size      634 bytes
// prototype void (__cdecl ?do_upgrade@@YAXHHH@Z)(int factionID, int protoIDNew, int protoIDOld)
// callers   1   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004EFD50 0x005BF1F0 0x0064601D
// Purpose: Retire one prototype across the faction: pay energy_reerves for the
//          upgrade, re-equip every vehicle of the old design (and its entries
//          in every base's production queue), adjust morale for the Trained
//          ability moving either way, mark the new design in service and the
//          old one obsolete, then repaint if a human is watching.
// TRIED: hoisting the vehicle-loop counter into a named local `count` and
//        reading ConsoleGroupCount once; the image re-reads [0x9a64c8] at both
//        the test and the loop latch, so the local cost a second stack slot
//        (`sub esp, 8` against the image's single `push ecx`) and 80 agreeing
//        instructions. Reading it at both sites and reordering the prologue to
//        faction_offset, upgrade_cost, then the tally moved 3/169 -> 101/169.
// TRIED: reusing the `proto_id_new` parameter slot as the vehicle-loop counter
//        the way the image does (`mov dword ptr [ebp + 0xc], 0` at instruction
//        23 zeroes it, so the parameter is the counter and its original value
//        lives in a register) - the archive spells this as `a2_saved`, and
//        spelling it here is what is left between this body and the image.
// BUG IN THE ORIGINAL: the production-queue sweep tests queue_production_id
//          [0 .. queue_size] INCLUSIVE - one entry past the queue's own count
//          - because the loop condition is `k <= *countp`. Reproduced
//          deliberately; see has_fac's note at the top of this file for the
//          same defect in a different body.
// ?draw_map@@YAXH@Z (mapwin.h) - declared here rather than included, so this
// file's include graph and every claim that depends on it stay untouched.
void __cdecl draw_map(int draw_type);

void __cdecl do_upgrade(int faction_id, int proto_id_new, int proto_id_old) {
    int faction_offset = faction_id * 0x20cc;
    int cost = upgrade_cost(faction_id, proto_id_new, proto_id_old);
    int product = cost * FactionProtoSeenCount[faction_offset + proto_id_old] * 10;
    PlayersData[faction_id].energy_reserves -= product;

    if (*ConsoleGroupCount > 0) {
        int i = 0;
        do {
            if (Vehs[i].faction_id == faction_id && Vehs[i].proto_id == proto_id_old) {
                FactionProtoSeenCount[faction_offset + proto_id_old]--;
                FactionProtoSeenCount[faction_offset + proto_id_new]++;
                Vehs[i].proto_id = proto_id_new;
                if (has_abil(proto_id_new, ABL_TRAINED) != 0
                    && has_abil(proto_id_old, ABL_TRAINED) == 0) {
                    int morale = Vehs[i].morale + 1;
                    if (morale < 0) {
                        morale = 0;
                    } else if (morale > 6) {
                        morale = 6;
                    }
                    Vehs[i].morale = morale;
                }
                if (has_abil(proto_id_new, ABL_TRAINED) == 0
                    && has_abil(proto_id_old, ABL_TRAINED) != 0) {
                    int morale = Vehs[i].morale - 1;
                    if (morale < 0) {
                        morale = 0;
                    } else if (morale > 6) {
                        morale = 6;
                    }
                    Vehs[i].morale = morale;
                }
            }
            i++;
        } while (i < *ConsoleGroupCount);
    }

    if (GameDataStateStorage.fields_.base_current_count_ > 0) {
        Base *base = Bases;
        int j = GameDataStateStorage.fields_.base_current_count_;
        do {
            if (base->faction_id_current == faction_id) {
                int queue_size = base->queue_size;
                if (queue_size >= 0) {
                    int k = 0;
                    do {
                        if (base->queue_production_id[k] == proto_id_old) {
                            base->queue_production_id[k] = proto_id_new;
                        }
                        k++;
                    } while (k <= queue_size);
                }
            }
            base++;
            j--;
        } while (j != 0);
    }

    if (proto_id_new > 0x3f && product != 0) {
        VehPrototypes[proto_id_new].flags |= 4;
    }
    if (proto_id_old > 0x3f) {
        VehPrototypes[proto_id_old].flags &= 0xfe;
    }

    if ((*(unsigned char *)&GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1 << faction_id)) != 0) {
        draw_map(1);
    }
    if (faction_id == ConsoleGlobal.faction_id_) {
        ConsoleGlobal.update_data(0);
    }
}

// Original score calculation clears all eight weights, then publishes the
// aggregate after dividing the transcendence bonus among faction populations.
// Historical object identity moved to GameDataStateStorage; see the transport manifest.

// Historical object identity moved to GameDataStateStorage; see the transport manifest.


static int score_word_add(int left, int right) {
    return static_cast<int>(static_cast<uint32_t>(left) + static_cast<uint32_t>(right));
}
static int score_word_sub(int left, int right) {
    return static_cast<int>(static_cast<uint32_t>(left) - static_cast<uint32_t>(right));
}
static int score_word_mul(int left, int right) {
    return static_cast<int>(static_cast<uint32_t>(left) * static_cast<uint32_t>(right));
}

// ORIGINAL: 0x005ACBE0 ?compute_score@@YAXHPAHPAHH@Z 0x005ACBE0-0x005AD3AD
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/unrecovered/005acbe0.cpp", "source:src/faction.h", "image:complete original instructions and independent storage/lifecycle evidence"], "reason": "Recover all original faction-score branches, recursive updates and publication in the base/objective domain through genuine Base, PlayerData and Rules fields; original signed population, word wrapping, bonus order and malformed-state paths remain explicit."}
// symbol    ?compute_score@@YAXHPAH0H@Z
void __cdecl compute_score(int faction, int *total, int *items, int recursive) {
    if ((GameDataStateStorage.fields_.game_state_ & 8) && !recursive) return;
    if (!total) total = &PlayersData[faction].unk_109;
    if (!items) items = PlayersData[faction].score_items_;
    *total = 0;
    if (!recursive) PlayersData[faction].unk_107 = 0;
    for (int item = 0; item < 9; ++item) items[item] = 0;

    for (int base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
        const int owner = Bases[base].faction_id_current;
        if (owner == faction) {
            items[0] = score_word_add(items[0], Bases[base].population_size);
            set_base(base);
            base_compute(0);
            for (int other = 1; other < 8; ++other) {
                // Original tests raw rule flags, not ExpansionEnabled.
                if (!(Players[faction].rule_flags & 0x80) && !(Players[other].rule_flags & 0x80)) {
                    if (!PlayersData[faction].sanction_turns && !PlayersData[other].sanction_turns)
                        items[1] = score_word_add(items[1], BaseCommerceReceipts[other - 1]);
                    else items[1] = score_word_add(items[1], 0);
                }
            }
        } else if (!(PlayersData[faction].flags & 0x60000)) {
            const uint32_t treaty = PlayersData[owner].diplo_treaties[faction];
            if (treaty & 0x2000000) {
                if (treaty & 1) items[0] = score_word_add(items[0], Bases[base].population_size);
            } else if ((treaty & 1) && (GameDataStateStorage.fields_.game_rules_ & 0x1000)) {
                items[0] = score_word_add(items[0], Bases[base].population_size / 2);
            }
        } else if (!(PlayersData[faction].diplo_treaties[owner] & 1) || !(GameDataStateStorage.fields_.game_rules_ & 0x1000)) {
            items[0] = score_word_add(items[0], Bases[base].population_size / 2);
        } else items[0] = score_word_add(items[0], Bases[base].population_size);
    }
    for (int technology = 0; technology < 89; ++technology)
        if (has_tech(technology, faction)) items[2] = score_word_add(items[2], 1);
    items[3] = score_word_mul(PlayersData[faction].theory_of_everything, 10);
    int objective_points = static_cast<int>(ScenarioSave.objective_achieve_points);
    if (objective_points < 1) objective_points = 1;
    items[6] = score_word_mul(num_objectives(faction, GameDataStateStorage.fields_.game_rules_ & 0x1000), objective_points);
    for (int project = 0; project < 64; ++project) {
        const int base = base_project(project);
        if (base >= 0 && Bases[base].faction_id_current == faction)
            items[4] = score_word_add(items[4], 25);
    }

    GameDataStateStorage.fields_.transcendence_score_bonus_ = 0;
    for (int slot = 0; slot < 8; ++slot) GameDataStateStorage.fields_.transcendence_score_weights_[slot] = 0;
    const int transcendence_base = GameDataStateStorage.fields_.secret_project_.ascent_to_transcendence();
    if (transcendence_base >= 0) {
        GameDataStateStorage.fields_.transcendence_score_bonus_ = 300;
        const int scaled = score_word_mul(score_word_sub(1000, GameDataStateStorage.fields_.turn_current_num_), 2);
        if (scaled >= 300) GameDataStateStorage.fields_.transcendence_score_bonus_ = scaled;
        int own_weight = 0;
        int aggregate = 0;
        int leader = -1;
        for (int candidate = 1; candidate < 8; ++candidate) {
            if (Bases[transcendence_base].faction_id_current == candidate) {
                leader = candidate;
                aggregate = score_word_mul(PlayersData[candidate].pop_total, 2);
                GameDataStateStorage.fields_.transcendence_score_weights_[candidate] = aggregate;
                if (candidate == faction) own_weight = aggregate;
                break;
            }
        }
        for (int other = 1; other < 8; ++other) {
            // Invalid/no leader leaves index -1, as in the original. Do not
            // hide that malformed-state access or the possible zero divisor.
            if (other != leader && (PlayersData[other].diplo_treaties[leader] & 1)) {
                const int weight = PlayersData[other].pop_total;
                aggregate = score_word_add(aggregate, weight);
                GameDataStateStorage.fields_.transcendence_score_weights_[other] = weight;
                if (other == faction) own_weight = score_word_add(own_weight, weight);
            }
        }
        items[6] = score_word_mul(GameDataStateStorage.fields_.transcendence_score_bonus_, own_weight) / aggregate;
        // Original publishes aggregate only AFTER the signed division.
        GameDataStateStorage.fields_.transcendence_score_weights_[0] = aggregate;
    }

    int diplomatic_bonus = score_word_mul(score_word_sub(500, GameDataStateStorage.fields_.turn_current_num_), 2);
    items[7] = diplomatic_bonus < 0 ? 0 : diplomatic_bonus;
    if (!(GameDataStateStorage.fields_.game_rules_ & 2)) items[7] = 0;
    if ((GameDataStateStorage.fields_.game_state_ & 0x2000) && (GameDataStateStorage.fields_.game_rules_ & 2) && items[7]) {
        int matching_population = 0;
        int total_population = 0;
        for (int base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
            const int population = Bases[base].population_size;
            total_population = score_word_add(total_population, population);
            const int owner = Bases[base].faction_id_current;
            if (owner == faction || ((PlayersData[owner].diplo_treaties[faction] & 1)
                && (PlayersData[owner].diplo_treaties[faction] & 0x2000000)))
                matching_population = score_word_add(matching_population, population);
        }
        if (total_population)
            items[7] = score_word_mul(PlayersData[faction].score_items_[7], matching_population) / total_population;
    } else {
        items[7] = 0;
        const uint32_t flags = PlayersData[faction].flags;
        if (flags & 0x20000) {
            int bonus = score_word_mul(score_word_sub(600, GameDataStateStorage.fields_.turn_current_num_), 2);
            if (bonus < 200) bonus = 200;
            items[7] = bonus;
        } else if (flags & 0x40000) {
            int bonus = score_word_mul(score_word_sub(600, GameDataStateStorage.fields_.turn_current_num_), 2);
            if (bonus < 200) bonus = 200;
            items[7] = bonus;
        } else if (flags & 0x100000) {
            int bonus = score_word_mul(score_word_sub(1000, GameDataStateStorage.fields_.turn_current_num_), 2);
            if (bonus < 300) bonus = 300;
            items[7] = bonus;
        } else {
            for (int other = 1; other < 8; ++other) {
                if (PlayersData[other].flags & 0x20000) {
                    if ((PlayersData[faction].diplo_treaties[other] & 1) && (GameDataStateStorage.fields_.game_rules_ & 0x1000)) {
                        int bonus = score_word_sub(600, GameDataStateStorage.fields_.turn_current_num_);
                        if (bonus < 100) bonus = 100;
                        items[7] = bonus;
                    }
                } else if (PlayersData[other].flags & 0x40000) {
                    if ((PlayersData[faction].diplo_treaties[other] & 1) && (GameDataStateStorage.fields_.game_rules_ & 0x1000)) {
                        int bonus = score_word_sub(600, GameDataStateStorage.fields_.turn_current_num_);
                        if (bonus < 100) bonus = 100;
                        items[7] = bonus;
                    }
                }
            }
        }
    }
    if (!(factions_status_byte(1) & (1u << (faction & 31)))) {
        items[7] = 0;
        items[8] = 0;
    }

    int required = ScenarioSave.objective_req_victory;
    if ((required < 9000 || static_cast<int>(ScenarioSave.objectives_sudden_death_victory) < 9000)
        && !(GameDataStateStorage.fields_.game_state_ & 0x80000)) {
        // Intentionally clears the faction's GLOBAL score cache even when
        // caller supplied a separate local items array in recursive mode.
        for (int item = 0; item < 9; ++item)
            if (item != 6) PlayersData[faction].score_items_[item] = 0;
    }
    if (!PlayersData[faction].score_items_[7]) {
        if (!required) {
            int best = most_objectives(0, 0);
            if (best < 1) best = 1;
            else best = most_objectives(0, 0); // Original calls twice here.
            if (num_objectives(faction, GameDataStateStorage.fields_.game_rules_ & 0x1000) >= best)
                items[7] = static_cast<int>(ScenarioSave.victory_achieve_bonus_points);
        } else {
            const int mine = num_objectives(faction, GameDataStateStorage.fields_.game_rules_ & 0x1000);
            required = ScenarioSave.objective_req_victory; // Original re-reads after call.
            if (mine >= required) items[7] = static_cast<int>(ScenarioSave.victory_achieve_bonus_points);
        }
    }
    for (int total_item = 0; total_item < 9; ++total_item)
        *total = score_word_add(*total, items[total_item]);
    if (MapNativeLifeForms != 1 && !(GameDataStateStorage.fields_.game_state_ & 0x10)) {
        const int difference = score_word_mul(score_word_sub(static_cast<int>(MapNativeLifeForms), 1), *total) / 4;
        items[5] = difference;
        *total = score_word_add(*total, difference);
    }
    if (GameDataStateStorage.fields_.game_state_ & 0x80000) {
        if (recursive) return;
        int best = 0;
        int other_total;
        for (int other = 1; other < 8; ++other) {
            int other_items[9];
            compute_score(other, &other_total, other_items, 1);
            if (other_total > best) best = other_total;
        }
        if (*total >= best) {
            int bonus = static_cast<int>(ScenarioSave.victory_achieve_bonus_points);
            if (!bonus) bonus = 1000;
            items[7] = bonus; // Original does NOT add this late bonus to total.
        }
    } else if (recursive) return;
    PlayersData[faction].unk_107 = *total;
    if ((GameDataStateStorage.fields_.game_rules_ & 0x400) && !(GameDataStateStorage.fields_.game_state_ & 0x20))
        PlayersData[faction].unk_107 = score_word_mul(*total, 2);
}

// Base capture owns faction transfer, project notices and territorial updates.
// ORIGINAL: 0x0050C510 ?capture_base@@YAXHHH@Z 0x0050C510-0x0050DD8F
// RECOVERY_BINDING: v1 {"evidence": ["git:2fc3037a00903d280c784474ddfabc9116bba83e:src/unrecovered/0050c510.cpp: complete original identity, span and seed transcription", "src/base.h: canonical capture_base(base_id, faction_id, mode) caller contract", "Facility[70..133] owns project names; SecretProject/base_project owns the separate project base ids", "recovery/tasks/capture-base-owner-oct5.json", "recovery/tests/test_capture_base_source.py"], "reason": "Recover complete base capture through existing Base, PlayerData, Facility, SecretProject, Map, Vehicle, Console and network owners; preserve original callback ordering, arithmetic and error paths. Historical span and findings remain obligations, not a matching or runtime claim."}
// TRIED: full struct-based transcription (Base@0x97D040, PlayerData@0x96C9E0, Player@0x946A50, Map@0x94A30C, Veh@0x952828, RulesFacility@0x9A4B68, all cross-checked against base.h/faction.h/map.h/veh.h offsets and static_asserts) diverges at instruction #3 (prologue reg-save/stack layout order) - not yet matched at the frame-shape level.
void __cdecl capture_base(int base_id, int faction_id, int mode) {
    unsigned char bVar11;
    char cVar2, cVar4;
    int iVar5, iVar8, iVar10, iVar15, iVar17;
    unsigned int uVar13, uVar14;
    int uVar7, uVar20, uVar21;
    char *pcVar18;
    char *pcVar19;
    char *hqName;
    char *hqDirectionName;
    char local_174[256];
    char local_74[32];
    unsigned int local_5c;
    int local_58;
    int local_54;
    int local_50;
    unsigned int local_4c;
    int local_48;
    unsigned int local_44;
    int local_40;
    unsigned int local_3c;
    int local_38;
    int local_34;
    int local_30;
    unsigned int local_2c;
    int local_28;
    int local_1c;
    unsigned int local_18;
    int local_14;
    unsigned int local_10;
    unsigned int local_8;
    int wipeOut;
    int j, k, otherIdx, techIdx, fc, dirOff, i, f;
    int found;
    int idxTmp, maskTmp;
    int cnt3, popFactor, techVal;
    int doSkip;
    unsigned int owner2;
    int tileIdx, protoBase;
    Map *tile;
    int bestBase;

    log_say("Capture Base", Bases[base_id].name_string, base_id, faction_id, 0);
    local_8 = (unsigned int)(unsigned char)Bases[base_id].faction_id_current;
    local_1c = (int)Bases[base_id].y;
    local_3c = (unsigned int)(Bases[base_id].faction_id_former == (unsigned char)faction_id);
    local_28 = (int)Bases[base_id].x;
    local_44 = (unsigned int)MapTiles[(MapLongitude) * local_1c + (local_28 >> 1)].region;
    local_4c = (unsigned int)faction_id * 2099u;
    cVar4 = Bases[base_id].population_size;
    local_48 = (int)((local_4c + local_8) * 4u);
    PlayersData[faction_id].diplo_unk4[local_8] = PlayersData[faction_id].diplo_unk4[local_8] + cVar4 * 2;
    PlayersData[faction_id].diplo_unk3[local_8] = PlayersData[faction_id].diplo_unk3[local_8] + cVar4 * 2;
    strcpy(local_74, Bases[base_id].name_string);
    treaty_off(faction_id, (int)local_8, 0x40);
    bVar11 = (unsigned char)faction_id;
    local_5c = 1u << (bVar11 & 0x1f);
    if (((local_5c & GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xff) != 0) ||
        ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xff & (1u << (local_8 & 0x1f))) != 0)) {
        treaty_on(faction_id, (int)local_8, 0x2000);
    }

    uVar14 = local_8;
    for (uVar13 = 1; (int)uVar13 < 8; uVar13++) {
        if (uVar13 != (unsigned int)faction_id && uVar13 != uVar14 &&
            (PlayersData[uVar14].diplo_treaties[uVar13] & 1) != 0 &&
            (PlayersData[uVar13].diplo_treaties[faction_id] & 0x10) == 0) {
            treaty_on((int)uVar14, (int)uVar13, 0x2000);
            uVar14 = local_8;
        }
    }

    if (faction_id == ConsoleGlobal.faction_id_) {
        iVar5 = Bases[base_id].queue_production_id[0];
        if (iVar5 < -0x45) {
            if (iVar5 < 0) iVar5 = -iVar5;
            if (base_project(iVar5 - FAC_HUMAN_GENOME_PROJ) == -1) {
                found = 0;
                if (GameDataStateStorage.fields_.base_current_count_ > 0) {
                    for (otherIdx = 0; otherIdx < GameDataStateStorage.fields_.base_current_count_; otherIdx++) {
                        if (otherIdx != base_id &&
                            Bases[otherIdx].faction_id_current == uVar14 &&
                            Bases[otherIdx].queue_production_id[0] == Bases[base_id].queue_production_id[0]) {
                            local_18 = (unsigned int)Bases[otherIdx].queue_production_id[0];
                            parse_says(0, Players[uVar14].adj_name_faction, -1, -1);
                            parse_say(1, (int)Facility[(int)local_18].name, -1, -1);
                            popp(ScriptPopupFile, "SURVIVEPROJECT", 0, "secproj_sm.pcx", 0);
                            uVar14 = local_8;
                            found = 1;
                            break;
                        }
                    }
                }
                if (!found) {
                    PluralityDefault = 0;
                    GenderDefault = Players[uVar14].is_leader_female;
                    parse_says(0, Players[uVar14].title_leader, -1, -1);
                    GenderDefault = Players[uVar14].is_leader_female;
                    PluralityDefault = 0;
                    parse_says(1, Players[uVar14].name_leader, -1, -1);
                    GenderDefault = Players[uVar14].noun_gender;
                    PluralityDefault = Players[uVar14].is_noun_plural;
                    parse_says(2, Players[uVar14].noun_faction, -1, -1);
                    iVar5 = Bases[base_id].queue_production_id[0];
                    if (iVar5 < 0) iVar5 = -iVar5;
                    parse_say(3, (int)Facility[iVar5].name, -1, -1);
                    popp(ScriptPopupFile, "HALTPROJECT", 0, "secproj_sm.pcx", 0);
                    iVar5 = Bases[base_id].queue_production_id[0];
                    if (iVar5 < 0) iVar5 = -iVar5;
                    bitmask(iVar5 - 0x46, &local_54, &local_58);
                    PlayersData[local_8].secret_project_intel[local_54] &= ~(unsigned int)local_58;
                    uVar14 = local_8;
                }
            }
        }
    }

    if (faction_id == ConsoleGlobal.faction_id_ || uVar14 == (unsigned int)ConsoleGlobal.faction_id_) {
        for (int project_record_id = 0; project_record_id < 64; ++project_record_id) {
            if (base_project(project_record_id) == base_id) {
                parse_say(0, reinterpret_cast<int>(Facility[70 + project_record_id].name), -1, -1);
                if (faction_id == ConsoleGlobal.faction_id_) {
                    pcVar19 = "secproj_sm.pcx";
                    pcVar18 = "SEIZEPROJECT";
                } else {
                    pcVar19 = "secproj_sm.pcx";
                    pcVar18 = "LOSEPROJECT";
                }
                popp(ScriptPopupFile, pcVar18, 0, pcVar19, 0);
            }
        }
    }

    if (PlayersData[local_8].energy_reserves < 1) {
        local_40 = 0;
    } else {
        local_40 = (Bases[base_id].population_size * PlayersData[local_8].energy_reserves) /
                   (PlayersData[local_8].pop_total + 1);
    }

    bitmask(1, &idxTmp, &maskTmp);
    local_10 = (unsigned int)((Bases[base_id].facilities_built[idxTmp] & maskTmp) != 0);
    if (local_10 != 0 && 999 < PlayersData[local_8].energy_reserves) {
        local_34 = 9999;
        local_38 = -1;
        if (GameDataStateStorage.fields_.base_current_count_ > 0) {
            for (local_14 = 0; local_14 < GameDataStateStorage.fields_.base_current_count_; local_14++) {
                if (local_14 != base_id && Bases[local_14].faction_id_current == (unsigned char)local_8) {
                    int aBase, bBase, hi, lo;
                    local_30 = Bases[local_14].y;
                    uVar14 = (unsigned int)(unsigned short)Bases[local_14].x;
                    local_50 = 1;
                    local_2c = (unsigned int)MapTiles[(MapLongitude) * local_30 + ((int)uVar14 >> 1)].region;
                    local_10 = uVar14;
                    if (GameDataStateStorage.fields_.base_current_count_ > 0) {
                        for (j = 0; j < GameDataStateStorage.fields_.base_current_count_; j++) {
                            if (j != local_14 && j != base_id &&
                                Bases[j].faction_id_current == (unsigned char)local_8) {
                                iVar15 = abs((int)uVar14 - (int)Bases[j].x);
                                if ((MapIsFlat & 1) == 0 && MapLongitude < iVar15) {
                                    iVar15 = MapLongitudeBounds - iVar15;
                                }
                                iVar8 = abs(local_30 - (int)Bases[j].y);
                                aBase = abs(iVar15);
                                bBase = abs(iVar8);
                                hi = (aBase > bBase) ? aBase : bBase;
                                lo = (aBase < bBase) ? aBase : bBase;
                                iVar15 = hi - (((aBase + bBase >> 1) - lo) + 1 >> 1);
                                tile = &MapTiles[(MapLongitude) * (int)Bases[j].y + ((int)Bases[j].x >> 1)];
                                if ((unsigned int)tile->region != local_2c) {
                                    iVar15 = iVar15 * 2;
                                }
                                local_50 = local_50 + iVar15;
                                uVar14 = local_10;
                            }
                        }
                    }
                    iVar8 = (local_50 << 8) /
                            (int)((unsigned int)PlayersData[local_8].region_total_bases[local_2c] +
                                  (int)Bases[local_14].population_size * 2);
                    if (iVar8 < local_34) {
                        local_38 = local_14;
                        local_34 = iVar8;
                    }
                }
            }
            if (0 < local_38) {
                parse_says(0, local_74, -1, -1);
                parse_says(1, Bases[local_38].name_string, -1, -1);
                if (IsMultiplayerNet != 0 || local_8 != (unsigned int)ConsoleGlobal.faction_id_ ||
                    X_pop("ESCAPE", 0) != 0) {
                    uVar14 = local_8;
                    PlayersData[local_8].energy_reserves -= 1000;
                    set_fac(1, local_38, 1);
                    set_fac(1, base_id, 0);
                    if (uVar14 == (unsigned int)ConsoleGlobal.faction_id_) {
                        if (IsMultiplayerNet != 0) {
                            pcVar18 = "ESCAPED2";
                            NetMsgPrimary.pop(pcVar18, 5000, 0, 0);
                        }
                    } else {
                        GenderDefault = Players[uVar14].noun_gender;
                        PluralityDefault = Players[uVar14].is_noun_plural;
                        parse_says(2, Players[uVar14].noun_faction, -1, -1);
                        pcVar18 = "ESCAPED";
                        NetMsgPrimary.pop(pcVar18, 5000, 0, 0);
                    }
                    if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1u << (uVar14 & 0x1f))) == 0) {
                        base_reset(local_38, 0);
                    }
                }
            }
        }
    }

    bitmask(1, &idxTmp, &maskTmp);
    uVar14 = local_8;
    local_10 = (unsigned int)((Bases[base_id].facilities_built[idxTmp] & maskTmp) != 0);
    if (local_10 != 0 && GameDataStateStorage.fields_.turn_current_num_ < PlayersData[local_8].corner_market_turn &&
        0 < PlayersData[local_8].corner_market_active) {
        PlayersData[local_8].energy_reserves += PlayersData[local_8].corner_market_active;
        uVar13 = (unsigned int)ConsoleGlobal.faction_id_;
        local_40 = local_40 + PlayersData[local_8].corner_market_active / 2;
        PlayersData[local_8].corner_market_active = 0;
        PlayersData[local_8].corner_market_turn = 0;
        if (local_8 == uVar13) {
            popp(ScriptPopupFile, "CORNERFOILED", 0, "econwin_sm.pcx", 0);
        } else {
            parse_says(0, Players[local_8].adj_name_faction, -1, -1);
            popp(ScriptPopupFile, "CORNERTHEMFOILED", 0, "econwin_sm.pcx", 0);
        }
    }

    PlayersData[faction_id].energy_reserves += local_40;
    {
        int newVal = PlayersData[uVar14].energy_reserves - local_40;
        if (newVal < 0) newVal = 0;
        else if (9999999 < newVal) newVal = 9999999;
        PlayersData[uVar14].energy_reserves = newVal;
    }

    wipeOut = 0;
    if (mode == 0 && local_3c == 0 &&
        (Bases[base_id].state & 0x40000) == 0 &&
        is_objective(base_id) == 0 &&
        (Bases[base_id].population_size > 1 ||
         (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & Bases[base_id].visibility) != 0 ||
         IsMultiplayerNet != 0)) {
        cVar4 = Bases[base_id].population_size - 1;
        Bases[base_id].population_size = cVar4;
        wipeOut = (cVar4 == 0);
    }

    if (wipeOut) {
        base_kill(base_id);
        draw_map(1);
        base_id = -1;
    } else if (base_id >= 0) {
        Bases[base_id].faction_id_current = bVar11;
        for (f = 1; f < 8; f++) {
            if (PlayersData[f].base_id_atk_target == base_id) PlayersData[f].base_id_atk_target = -1;
        }
        Bases[base_id].nerve_staple_turns_left = 0;
        if (Bases[base_id].faction_id_former == (unsigned char)faction_id) {
            Bases[base_id].assimilation_turns_left = 0;
        } else {
            Bases[base_id].faction_id_former = (unsigned char)uVar14;
            Bases[base_id].assimilation_turns_left = 0x32;
        }
        PlayersData[faction_id].last_turn_new_base = GameDataStateStorage.fields_.turn_current_num_;
        PlayersData[uVar14].current_num_bases -= 1;
        PlayersData[faction_id].current_num_bases += 1;
        replay_base(1, local_28, local_1c, faction_id);
        Bases[base_id].minerals_accumulated = 0;
        Bases[base_id].unk_6 = 0;
        if ((int)Bases[base_id].queue_size >= 0) {
            int *item = &Bases[base_id].queue_production_id[0];
            int cnt = (int)Bases[base_id].queue_size + 1;
            do {
                if (*item >= 0) {
                    PlayersData[uVar14].proto_id_queue[*item] -= 1;
                }
                item++;
                cnt--;
            } while (cnt != 0);
        }
        Bases[base_id].queue_size = 0;
        Bases[base_id].production_id_last = -69;
        Bases[base_id].queue_production_id[0] = -69;

        local_34 = 0;
        local_38 = -1;
        if (GameDataStateStorage.fields_.base_current_count_ > 0) {
            for (j = 0; j < GameDataStateStorage.fields_.base_current_count_; j++) {
                if (j != base_id && Bases[j].faction_id_current == (unsigned char)faction_id) {
                    int val = (int)Bases[j].population_size;
                    bitmask(1, &idxTmp, &maskTmp);
                    if ((Bases[j].facilities_built[idxTmp] & maskTmp) != 0) val = val * 2;
                    if (local_34 < val) { local_38 = j; local_34 = val; }
                }
            }
            if (local_38 >= 0) {
                Bases[base_id].governor = Bases[local_38].governor;
            }
        }

        Bases[base_id].state &= 0xffdef7b1u;

        if (mode == 0 && local_3c == 0) {
            for (k = 1; k < 0x46; k++) {
                if (k != 0x1a && (rand() % 3) == 0) {
                    set_fac(k, base_id, 0);
                }
            }
        }
        set_fac(1, base_id, 0);
        if (local_3c == 0) {
            set_fac(6, base_id, 0);
            set_fac(3, base_id, 0);
        }

        {
            int mapW = MapLongitudeBounds;
            for (dirOff = 0; dirOff < 0x54; dirOff += 4) {
                int dx = RadiusOffsetX[dirOff / 4];
                int tx = dx + local_28;
                int dy, ty;
                if ((MapIsFlat & 1) == 0) {
                    if (tx < 0) tx += mapW;
                    else if (mapW <= tx) tx -= mapW;
                }
                dy = RadiusOffsetY[dirOff / 4];
                ty = dy + local_1c;
                if (ty >= 0 && ty < MapLatitudeBounds && tx >= 0 && tx < mapW) {
                    using_set(tx, ty, faction_id);
                    tile = &MapTiles[(MapLongitude) * ty + (tx >> 1)];
                    tile->visibility = (unsigned char)(tile->visibility | (1u << (bVar11 & 0x1f)));
                    synch_bit(tx, ty, faction_id);
                    iVar5 = veh_at(tx, ty);
                    mapW = MapLongitudeBounds;
                    if (iVar5 >= 0 && Vehs[iVar5].faction_id == (unsigned char)faction_id) {
                        while (iVar5 >= 0) {
                            Vehs[iVar5].state &= 0xfffbffffu;
                            iVar5 = Vehs[iVar5].next_veh_id_stack;
                            mapW = MapLongitudeBounds;
                        }
                    }
                }
            }
        }

        spot_base(base_id, (int)local_8);
        reset_territory();
        for (i = GameDataStateStorage.fields_.vehicle_current_count_ - 1; i >= 0; i--) {
            if (i < GameDataStateStorage.fields_.vehicle_current_count_ && Vehs[i].faction_id == (unsigned char)local_8 &&
                Vehs[i].home_base_id == base_id) {
                int vy, dx8, dy8, killIt;
                Bases[base_id].faction_id_current = 0xff;
                iVar5 = base_find(local_28, local_1c, (int)local_8);
                Bases[base_id].faction_id_current = bVar11;
                killIt = 1;
                if (iVar5 >= 0) {
                    vy = Vehs[i].y;
                    dx8 = abs(local_28 - Vehs[i].x);
                    if ((MapIsFlat & 1) == 0 && MapLongitude < dx8) {
                        dx8 = MapLongitudeBounds - dx8;
                    }
                    dy8 = abs(local_1c - vy);
                    if (((dx8 + dy8) & 0xfffffffe) != 0) {
                        Vehs[i].home_base_id = (short)iVar5;
                        killIt = 0;
                    }
                }
                if (killIt) {
                    kill(i);
                }
            }
        }

        for (fc = 0; fc < 2; fc++) {
            int fid = (fc == 0) ? faction_id : (int)local_8;
            int cnt2 = Players[fid].faction_bonus_count;
            for (k = 0; k < cnt2; k++) {
                if (Players[fid].faction_bonus_id[k] == 2 ||
                    (Players[fid].faction_bonus_id[k] == 0xc &&
                     has_tech(Facility[Players[fid].faction_bonus_val1[k]].preq_tech, faction_id) != 0)) {
                    int fv = Players[fid].faction_bonus_val1[k];
                    if (fv < 1) fv = 1;
                    else if (fv > 0x45) fv = 0x45;
                    set_fac(fv, BaseIDCurrentSelected, 1);
                }
            }
        }
        base_first(base_id);
    }

    iVar5 = local_1c;
    iVar15 = local_28;
    owner_set(local_28, local_1c, faction_id);
    draw_map(1);
    UnkBitfield1 |= 2;
    WorldClimateWorldWin.redraw();

    GenderDefault = Players[faction_id].noun_gender;
    PluralityDefault = Players[faction_id].is_noun_plural;
    parse_says(0, Players[faction_id].noun_faction, -1, -1);
    parse_says(1, local_74, -1, -1);
    GenderDefault = Players[local_8].noun_gender;
    PluralityDefault = Players[local_8].is_noun_plural;
    parse_says(2, Players[local_8].noun_faction, -1, -1);
    parse_num(0, local_40);
    local_174[0] = 0;
    pcVar18 = "LIBERATE";
    if (local_3c == 0) pcVar18 = "SEIZE";
    strcat(local_174, pcVar18);
    strcat(local_174, "BASE");

    local_10 = 1u << (local_8 & 0x1f);
    if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xff & local_10) != 0 &&
        (local_5c & GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xff) == 0 &&
        base_id >= 0 &&
        (Bases[base_id].state & 0x80) == 0 && local_3c == 0) {
        Bases[base_id].state |= 0x10000;
    }

    if (faction_id == ConsoleGlobal.faction_id_ || local_8 == (unsigned int)ConsoleGlobal.faction_id_) {
        iVar10 = ConsoleGlobal.focus(iVar15, iVar5, ConsoleGlobal.faction_id_);
        if (iVar10 == 0) {
            draw_tile_fixup(iVar15, iVar5, 1, 2);
        }
        ConsoleGlobal.update_data(0);
        pcVar18 = "liberate_sm.pcx";
        if (faction_id != ConsoleGlobal.faction_id_) pcVar18 = "seized_sm.pcx";
        popp(ScriptPopupFile, local_174, 0, pcVar18, 0);
        ambience(0xd5);
    } else {
        draw_tile_fixup(iVar15, iVar5, 1, 2);
        if ((PlayersData[ConsoleGlobal.faction_id_].diplo_treaties[faction_id] & 1) == 0 &&
            ((PlayersData[faction_id].diplo_treaties[ConsoleGlobal.faction_id_] & 0x80) == 0 ||
             (PlayersData[faction_id].diplo_treaties[local_8] & 0x40) == 0)) {
            if ((PlayersData[ConsoleGlobal.faction_id_].diplo_treaties[local_8] & 1) == 0 &&
                ((PlayersData[local_8].diplo_treaties[ConsoleGlobal.faction_id_] & 0x80) == 0 ||
                 (PlayersData[local_8].diplo_treaties[faction_id] & 0x40) == 0)) {
                int notSpying = 0;
                iVar15 = spying(faction_id);
                if (iVar15 == 0) {
                    iVar15 = spying((int)local_8);
                    if (iVar15 == 0) notSpying = 1;
                }
                if (!notSpying) {
                    strcat(local_174, "4");
                    pcVar18 = "seized_sm.pcx";
                    popp(ScriptPopupFile, local_174, 0, pcVar18, 0);
                }
            } else {
                strcat(local_174, "3");
                pcVar18 = "seized_sm.pcx";
                popp(ScriptPopupFile, local_174, 0, pcVar18, 0);
            }
        } else {
            strcat(local_174, "2");
            pcVar18 = "liberate_sm.pcx";
            popp(ScriptPopupFile, local_174, 0, pcVar18, 0);
        }
    }

    if (((Players[local_8].rule_flags ^ Players[faction_id].rule_flags) & 0x80) != 0 && base_id >= 0 &&
        Bases[base_id].population_size >= 2) {
        PluralityDefault = Players[faction_id].is_noun_plural;
        GenderDefault = Players[faction_id].noun_gender;
        uVar14 = (unsigned int)(Bases[base_id].population_size / 2);
        local_18 = uVar14;
        parse_says(0, Players[faction_id].noun_faction, -1, -1);
        parse_says(1, local_74, -1, -1);
        parse_num(0, (int)uVar14);
        Bases[base_id].population_size = 1;
        draw_tile((int)Bases[base_id].x, (int)Bases[base_id].y, 2);

        if ((Players[faction_id].rule_flags & 0x80) == 0) {
            if (faction_id == ConsoleGlobal.faction_id_) {
                if (uVar14 == 1) {
                    pcVar19 = "alienref_sm.pcx"; pcVar18 = "HOMELESSONE";
                    popp(ScriptPopupFile, pcVar18, 0, pcVar19, 0);
                } else if (uVar14 != 0) {
                    pcVar19 = "alienref_sm.pcx"; pcVar18 = "HOMELESS";
                    popp(ScriptPopupFile, pcVar18, 0, pcVar19, 0);
                }
                interlude(0x19, Bases[base_id].name_string, 1, 0);
            }
            if (local_8 == (unsigned int)ConsoleGlobal.faction_id_) {
                if (uVar14 == 1) {
                    pcVar19 = "alienref_sm.pcx"; pcVar18 = "HOMELESSONE";
                    popp(ScriptPopupFile, pcVar18, 0, pcVar19, 0);
                } else if (uVar14 != 0) {
                    pcVar19 = "alienref_sm.pcx"; pcVar18 = "HOMELESS";
                    popp(ScriptPopupFile, pcVar18, 0, pcVar19, 0);
                }
            }
            mode = 1;
            iVar5 = MapLongitudeBounds;
        } else {
            if (faction_id == ConsoleGlobal.faction_id_) {
                if (uVar14 == 1) {
                    pcVar19 = "humanref_sm.pcx"; pcVar18 = "HOMELESSONE";
                    popp(ScriptPopupFile, pcVar18, 0, pcVar19, 0);
                } else if (uVar14 != 0) {
                    pcVar19 = "humanref_sm.pcx"; pcVar18 = "HOMELESS";
                    popp(ScriptPopupFile, pcVar18, 0, pcVar19, 0);
                }
                interlude(0x25, Bases[base_id].name_string, 1, 0);
            }
            if (local_8 == (unsigned int)ConsoleGlobal.faction_id_) {
                if (uVar14 == 1) {
                    pcVar19 = "humanref_sm.pcx"; pcVar18 = "HOMELESSONE";
                    popp(ScriptPopupFile, pcVar18, 0, pcVar19, 0);
                } else if (uVar14 != 0) {
                    pcVar19 = "humanref_sm.pcx"; pcVar18 = "HOMELESS";
                    popp(ScriptPopupFile, pcVar18, 0, pcVar19, 0);
                }
                interlude(0x18, Bases[base_id].name_string, 1, 0);
            }
            mode = 1;
            iVar5 = MapLongitudeBounds;
        }

        do {
            iVar10 = mode;
            if (mode < 9) {
                uVar14 = (unsigned int)rand();
                uVar14 = uVar14 & 0x80000007u;
                if ((int)uVar14 < 0) {
                    uVar14 = (uVar14 - 1) | 0xfffffff8u;
                    uVar14 = uVar14 + 1;
                }
                iVar10 = (int)uVar14 + 1;
                iVar5 = MapLongitudeBounds;
            }
            iVar15 = RadiusOffsetX[iVar10] + (int)Bases[base_id].x;
            if ((MapIsFlat & 1) == 0) {
                if (iVar15 < 0) iVar15 += iVar5;
                else if (iVar5 <= iVar15) iVar15 -= iVar5;
            }
            iVar10 = RadiusOffsetY[iVar10] + (int)Bases[base_id].y;
            if (-1 < iVar10 && iVar10 < MapLatitudeBounds && -1 < iVar15 && iVar15 < iVar5) {
                tileIdx = (MapLongitude) * iVar10 + (iVar15 >> 1);
                tile = &MapTiles[tileIdx];
                doSkip = 0;
                if ((tile->bit & 3) != 0 && (unsigned int)(tile->val2 & 0xf) < 8) {
                    if ((tile->bit & 3) == 0 || (owner2 = (unsigned int)(tile->val2 & 0xf), owner2 > 7)) {
                        owner2 = 0xffffffff;
                    }
                    iVar5 = MapLongitudeBounds;
                    if (owner2 != local_8) {
                        doSkip = 1;
                    }
                }
                if (!doSkip) {
                    protoBase = ((tile->climate & 0xe0) < 0x60) ? 0x15 : 0;
                    veh_init(protoBase, (int)local_8, iVar15, iVar10);
                    draw_tile(iVar15, iVar10, 2);
                    local_18 = local_18 - 1;
                    iVar5 = MapLongitudeBounds;
                    if (local_18 == 0) break;
                }
            }
            mode = mode + 1;
        } while (mode < 0x15);
    }

    uVar14 = local_8;
    if ((GameDataStateStorage.fields_.game_rules_ & 0x4000) == 0 && (Players[faction_id].rule_flags & 0x10) == 0) {
        cnt3 = 0;
        for (techIdx = 0; techIdx < 0x59; techIdx++) {
            if (has_tech(techIdx, (int)local_8) != 0 && has_tech(techIdx, faction_id) == 0) {
                cnt3 = cnt3 + 1;
            }
        }
        if (base_id < 0) popFactor = 1;
        else popFactor = (int)Bases[base_id].population_size + 1;
        techVal = (PlayersData[faction_id].tech_ranking + 3) * cnt3 * popFactor;
        PlayersData[faction_id].tech_accumulated += (techVal + ((techVal >> 0x1f) & 7)) >> 3;
    } else {
        steal_tech(faction_id, (int)local_8, 1);
        if (base_id >= 0 && GameDataStateStorage.fields_.secret_project_.universal_translator() == base_id) {
            steal_tech(faction_id, (int)uVar14, 1);
        }
    }

    if (((unsigned char)local_10 & (unsigned char)GameDataStateStorage.fields_.lock_masks_.enable_mask_) == 0) {
        bases_reset((int)local_8, (int)local_44, 0);
    }
    mon_conquer_base(faction_id, local_74);

    if (faction_id == ConsoleGlobal.faction_id_ && base_id >= 0 &&
        (Bases[base_id].state & 0x40000) != 0 &&
        (GameDataStateStorage.fields_.game_preferences_ & 0x40000) == 0 && IsMultiplayerNet == 0) {
        parse_says(6, Players[ConsoleGlobal.faction_id_].assistant_city, -1, -1);
        hqName = Bases[base_id].name_string;
        parse_says(5, hqName, -1, -1);
        parse_says(7, Players[local_8].adj_name_faction, -1, -1);
        cVar4 = 0;
        if (GameDataStateStorage.fields_.base_current_count_ > 0) {
            bestBase = -1;
            for (iVar15 = 0; iVar15 < GameDataStateStorage.fields_.base_current_count_; iVar15++) {
                if (Bases[iVar15].faction_id_current == (unsigned char)faction_id && iVar15 != base_id) {
                    bitmask(1, &idxTmp, &maskTmp);
                    if ((Bases[iVar15].facilities_built[idxTmp] & maskTmp) != 0) {
                        parse_says(2, Bases[iVar15].name_string, -1, -1);
                        bestBase = iVar15;
                        break;
                    }
                    cVar2 = Bases[iVar15].population_size;
                    if (cVar4 < cVar2) {
                        parse_says(2, Bases[iVar15].name_string, -1, -1);
                        cVar4 = cVar2;
                    }
                }
            }
            (void)bestBase;
        }
        iVar15 = local_28;
        iVar5 = base_find(local_28, local_1c, (int)local_8);
        if (iVar5 < 0) {
            hqDirectionName = label_get(403);
        } else {
            iVar10 = (int)Bases[iVar5].x;
            iVar8 = iVar10 - iVar15;
            if (iVar8 < 0) iVar8 = iVar15 - iVar10;
            iVar17 = (int)Bases[iVar5].y;
            iVar5 = iVar17 - local_1c;
            if (iVar5 < 0) iVar5 = local_1c - iVar17;
            if ((iVar8 * 3) / 2 < iVar5) {
                if (iVar17 < local_1c) hqDirectionName = label_get(403);
                else hqDirectionName = label_get(405);
            } else {
                iVar15 = iVar15 - iVar10;
                if (MapLongitudeBounds / 2 < iVar15) iVar15 -= MapLongitudeBounds;
                if (iVar15 < -(MapLongitudeBounds / 2)) iVar15 += MapLongitudeBounds;
                if (iVar15 < 1) hqDirectionName = label_get(404);
                else hqDirectionName = label_get(406);
            }
        }
        uVar21 = -1; uVar20 = -1;
        parse_says(4, hqDirectionName, uVar20, uVar21);
        strcpy(hqName, Players[ConsoleGlobal.faction_id_].assistant_city);
        make_base_unique(base_id);
        interlude(7, 0, 1, 1);
        draw_map(1);
        text_close();
    }

    eliminate_player((int)local_8, faction_id);
    if (base_id >= 0) {
        set_base(base_id);
        base_compute(1);
        if (faction_id == ConsoleGlobal.faction_id_) {
            iVar15 = ConsoleGlobal.ready_search(-1);
            if (iVar15 < 0) {
                GameDataStateStorage.fields_.game_state_ |= 4;
                return;
            }
            TutWinBaseWindow.zoom(base_id, 0);
        }
    }
    return;
}

// ORIGINAL: 0x004EC3F0 ?base_connect@@YAHH@Z 0x004EC3F0-0x004ECB84
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004ec3f0.cpp original signature and complete span; independent pinned instruction review corrected magtube exception and destination population comparison; actual-source route and sensor controls."], "reason": "Base-domain connection planner follows same-region base paths and sensor fallback using existing typed Base, Map and Path owners."}
// TRIED: MISMATCH #0 'push' vs 'sub' (prologue diverges; framed build sim 0.63, first real divergence at #3 'mov'/'movsx'). Base-to-base connection pathfinder: transcribed from the Ghidra pseudocode's control flow, but re-derived every call's ARGUMENT LIST from the raw disassembly rather than trusting Ghidra's - two of its renderings were wrong (has_tech's 3rd arg 0xffffffff is a hallucinated leftover register, the real call pushes 2 args matching the 2-arg catalogue signature; and the sensor-distance comparison used `sensorX` where the source clearly reads Y). Simplified the `neg/sbb/and/add` "tech ? 0 : 4" guard-value idiom to a ternary for readability - MISMATCH-tier regardless of that choice given the size.
int __cdecl base_connect(int priority) {
    const int source_x = BaseCurrent->x;
    const int source_y = BaseCurrent->y;
    const int faction = BaseCurrent->faction_id_current;
    const Map *source_tile = map_loc(source_x, source_y);
    if ((source_tile->climate & 0xE0) < ALT_BIT_SHORE_LINE) return 0;

    int best_distance = 9999;
    int best_x = -1;
    int best_y = -1;
    bool found_connection = false;

    for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
        if (base_id == BaseIDCurrentSelected) continue;
        const Base &destination = Bases[base_id];
        const int destination_x = destination.x;
        const int destination_y = destination.y;
        if (map_loc(destination_x, destination_y)->region != source_tile->region)
            continue;

        int delta_x = abs(source_x - destination_x);
        if (!(MapIsFlat & 1) && static_cast<int>(MapLongitude) < delta_x)
            delta_x = MapLongitudeBounds - delta_x;
        const int delta_y = abs(source_y - destination_y);
        int distance = (delta_x + delta_y) >> 1;

        if (destination.faction_id_current == faction) {
            if (distance > 8 && !has_fac_built(FAC_HEADQUARTERS, base_id)) {
                bool outranked = false;
                for (int other_base_id = 0; other_base_id < GameDataStateStorage.fields_.base_current_count_;
                     ++other_base_id) {
                    const Base &other = Bases[other_base_id];
                    if (other.faction_id_current != faction ||
                        map_loc(other.x, other.y)->region != source_tile->region)
                        continue;
                    if (destination.population_size < other.population_size) {
                        outranked = true;
                        break;
                    }
                    if (has_fac_built(FAC_HEADQUARTERS, other_base_id)) {
                        outranked = true;
                        break;
                    }
                }
                if (outranked) continue;
            }
        } else {
            if (distance > 5 ||
                (PlayersData[faction].diplo_treaties[destination.faction_id_current]
                    & DTREATY_PACT) == 0)
                continue;
            distance += MapLongitudeBounds;
        }

        if (distance >= best_distance) continue;

        int path_x = source_x;
        int path_y = source_y;
        const int path_guard = has_tech(Rules.tech_build_roads_fungus, faction) ? 0 : 4;
        int direction = Paths.find(path_x, path_y, destination_x, destination_y,
                                   0, -1, path_guard, -1);
        int path_length = 0;
        while (direction >= 0) {
            path_x += RadiusBaseX[direction];
            if (!(MapIsFlat & 1)) {
                if (path_x < 0) path_x += MapLongitudeBounds;
                else if (path_x >= MapLongitudeBounds) path_x -= MapLongitudeBounds;
            }
            path_y += RadiusBaseY[direction];
            if (path_y < 0 || path_y >= MapLatitudeBounds || path_x < 0 ||
                path_x >= MapLongitudeBounds)
                break;
            if (path_x == destination_x && path_y == destination_y) {
                found_connection = true;
                break;
            }

            const Map *tile = map_loc(path_x, path_y);
            const unsigned int tile_owner = tile->val2 & 0x0F;
            const bool base_ok = (tile->bit & BIT_ROAD) == 0 ||
                ((tile->bit & BIT_MAGTUBE) != 0 &&
                 has_tech(Terraforming[TERRA_MAGTUBE].preq_tech, faction));
            const bool occupant_ok = (tile->bit & BIT_VEH_IN_TILE) == 0 ||
                tile_owner > 7 || tile_owner == static_cast<unsigned int>(faction) ||
                (PlayersData[faction].diplo_treaties[tile_owner] & DTREATY_PACT) != 0;
            const bool fungus_ok = (tile->bit & BIT_FUNGUS) == 0 ||
                (tile->climate & 0xE0) < ALT_BIT_OCEAN_SHELF ||
                has_tech(Rules.tech_build_roads_fungus, faction) ||
                has_tech(Terraforming[TERRA_REMOVE_FUNGUS].preq_tech, faction);
            const int territory = whose_territory(faction, path_x, path_y, NULL, FALSE);
            const bool territory_ok = territory < 0 || territory == faction ||
                (PlayersData[faction].diplo_treaties[territory] & DTREATY_PACT) != 0;

            if ((tile->bit & BIT_BASE_IN_TILE) == 0 || tile_owner > 7) {
                if (fungus_ok && occupant_ok && territory_ok && base_ok &&
                    zoc_sea(path_x, path_y, faction) <= 0) {
                    best_distance = distance;
                    best_x = path_x;
                    best_y = path_y;
                    break;
                }
            }

            ++path_length;
            if (path_length >= MapLongitudeBounds) break;
            const int live_guard = has_tech(Rules.tech_build_roads_fungus, faction) ? 0 : 4;
            direction = Paths.find(path_x, path_y, destination_x, destination_y,
                                   0, -1, live_guard, -1);
        }
    }

    int destination_x = source_x;
    int destination_y = source_y;
    if (best_y < 0 || best_y >= MapLatitudeBounds || best_x < 0 ||
        best_x >= MapLongitudeBounds) {
        if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1u << (faction & 0x1F))) != 0 &&
            (GameDataStateStorage.fields_.game_more_preferences_ & MPREF_AUTO_FORMER_BUILD_SENSORS) == 0)
            return 0;
        if (Paths.sensors(faction, &destination_x, &destination_y) != 0) return 0;
    } else {
        if (found_connection &&
            ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1u << (faction & 0x1F))) == 0 ||
             (GameDataStateStorage.fields_.game_more_preferences_ & MPREF_AUTO_FORMER_BUILD_SENSORS) != 0)) {
            int sensor_x;
            int sensor_y;
            if (Paths.sensors(faction, &sensor_x, &sensor_y) == 0) {
                int sensor_dx = abs(source_x - sensor_x);
                if (!(MapIsFlat & 1) && static_cast<int>(MapLongitude) < sensor_dx)
                    sensor_dx = MapLongitudeBounds - sensor_dx;
                const int sensor_dy = abs(source_y - sensor_y);
                const int sensor_max = sensor_dx > sensor_dy ? sensor_dx : sensor_dy;
                const int sensor_min = sensor_dx < sensor_dy ? sensor_dx : sensor_dy;
                const int sensor_distance = sensor_max -
                    (((sensor_dx + sensor_dy) / 2 - sensor_min + 1) / 2);

                int best_dx = abs(source_x - best_x);
                if (!(MapIsFlat & 1) && static_cast<int>(MapLongitude) < best_dx)
                    best_dx = MapLongitudeBounds - best_dx;
                const int best_dy = abs(source_y - best_y);
                const int best_max = best_dx > best_dy ? best_dx : best_dy;
                const int best_min = best_dx < best_dy ? best_dx : best_dy;
                const int best_approx_distance = best_max -
                    (((best_dx + best_dy) / 2 - best_min + 1) / 2);
                if (sensor_distance < best_approx_distance) {
                    add_goal(faction, AI_GOAL_TERRAFORM_LAND, priority, sensor_x, sensor_y,
                             BaseIDCurrentSelected);
                    return 1;
                }
            }
        }
        destination_x = best_x;
        destination_y = best_y;
    }

    add_goal(faction, AI_GOAL_TERRAFORM_LAND, priority, destination_x, destination_y,
             BaseIDCurrentSelected);
    return 1;
}

// ORIGINAL: 0x004ECBF0 ?base_terraform@@YAXH@Z 0x004ECBF0-0x004EEF61
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004ecbf0.cpp retained full native span", "Pinned 4ECBF0..4EEF61 instruction review establishes BaseIDCurrentSelected forced-improvement timer and pre-filter improvement-presence latches", "recovery/tests/test_base_terraform_behavior.py actual-source behavioral controls"], "reason": "Base-domain terraforming planner uses genuine Base, Map, Path and faction owners for tile scoring, improvement goals, forced actions and relocation. Historical seed and original extent remain preserved; source translation is not a byte-exact or runtime claim."}
// TRIED: Ghidra ported almost verbatim (DAT_0090ea30 recovered as a real `short *base16` on *BaseCurrent - offsets proven against the ASM at faction_id_current/population_size/state/governor/x/y/ mineral+energy_intake_1/unk_x/unk_y; the 15 FUN_ callees matched to their real, already-recovered names by prototype arity; two Ghidra call-site arg-grouping bugs fixed (abil_index/has_tech argument swap) - diverges at #6 'or eax,-1' vs 'xor eax,eax' register-idiom for the first of three `= 0xffffffff` locals, a compiler-idiom choice not obviously steerable from source.
void __cdecl base_terraform(int force_improve) {
    Base &base = *BaseCurrent;
    const int faction = base.faction_id_current;
    const uint32_t faction_bit = 1u << (faction & 0x1F);
    // The native tests only the low byte of the mask at both use sites.
    const bool human = (static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) &
                        static_cast<uint8_t>(faction_bit)) != 0;

    int growth_priority = 0;
    int wealth_priority = 0;
    int power_priority = 0;
    if (!human) {
        growth_priority = PlayersData[faction].ai_growth;
        wealth_priority = PlayersData[faction].ai_wealth;
        power_priority = PlayersData[faction].ai_power;
    } else {
        // The project has not named these Base::governor control bits; keep
        // the original masks and precedence instead of assigning semantics.
        const uint32_t governor = base.governor;
        if ((governor & 0x01000000u) == 0) {
            if ((governor & 0x02000000u) == 0) {
                if ((governor & 0x04000000u) == 0) {
                    if ((governor & 0x08000000u) != 0) power_priority = 1;
                } else {
                    wealth_priority = 1;
                }
            } else {
                growth_priority = 1;
            }
        }
    }

    int energy_need = (((wealth_priority - power_priority + 4 + growth_priority) *
                        static_cast<int>(base.population_size) + 3) / 5) -
                      base.energy_intake_1 + BaseSquareEnergy;
    if (energy_need < 0) energy_need = 0;
    else if (energy_need > 99) energy_need = 99;

    int mineral_need = (power_priority * 2 - base.mineral_intake_1) + 1 +
                       static_cast<int>(base.population_size);
    if (mineral_need < 0) mineral_need = 0;
    else if (mineral_need > 99) mineral_need = 99;

    if (energy_need == 0 || energy_need + 1 <= mineral_need)
        base.state &= ~BSTATE_UNK_1000;
    else
        base.state |= BSTATE_UNK_1000;
    if (mineral_need == 0 || mineral_need + 1 <= energy_need)
        base.state &= ~BSTATE_UNK_2000;
    else
        base.state |= BSTATE_UNK_2000;

    int need_gap = (3 - (energy_need < 3 ? energy_need : 2) -
                    (mineral_need < 3 ? mineral_need : 2));
    if (need_gap < 0) need_gap = 0;
    else if (need_gap > 3) need_gap = 3;
    int nutrient_need_state = 0;
    if (base.nutrient_surplus < need_gap) {
        nutrient_need_state = base.nutrient_surplus < 0 ? 2 : 1;
        base.state |= BSTATE_UNK_4000;
    } else {
        base.state &= ~BSTATE_UNK_4000;
    }

    int sensor_count = 0;
    int good_unworked_count = 0;
    int worked_bonus_count = 0;
    int worked_square_count = 0;
    int radius_index;
    for (radius_index = 0; radius_index < 21; ++radius_index) {
        int x = base.x + RadiusOffsetX[radius_index];
        if ((MapIsFlat & 1) == 0) {
            if (x < 0) x += MapLongitudeBounds;
            else if (x >= MapLongitudeBounds) x -= MapLongitudeBounds;
        }
        const int y = base.y + RadiusOffsetY[radius_index];
        if (y < 0 || y >= MapLatitudeBounds || x < 0 ||
            x >= MapLongitudeBounds)
            continue;

        const Map &tile = *map_loc(x, y);
        if ((tile.climate & 0xE0) < ALT_BIT_SHORE_LINE ||
            whose_territory(faction, x, y, NULL, FALSE) != faction)
            continue;

        if (is_sensor(x, y) != 0) ++sensor_count;
        const uint32_t tile_flags = tile.bit;
        const bool workable_terrain = (tile_flags & BIT_FOREST) == 0 &&
            ((tile_flags & BIT_FUNGUS) == 0 ||
             (tile.climate & 0xE0) < ALT_BIT_OCEAN_SHELF);
        const uint32_t worked_bit = 1u << (radius_index & 0x1F);
        if (workable_terrain && (base.resource_sqr_radius & worked_bit) == 0) {
            const int rainfall = (tile.climate & 0x18) >> 3;
            const int worked_modifier = (tile_flags >> 15) & 1;
            if (rainfall + worked_modifier > 1) {
                bool contested_by_neighbor = false;
                for (int other_id = 0; other_id < GameDataStateStorage.fields_.base_current_count_; ++other_id) {
                    if (Bases[other_id].faction_id_current != faction) continue;
                    int dx = abs(x - Bases[other_id].x);
                    if ((MapIsFlat & 1) == 0 && dx > MapLongitude)
                        dx = MapLongitudeBounds - dx;
                    const int dy = abs(y - Bases[other_id].y);
                    if (vector_dist(dx, dy) < 3) {
                        const int other_radius = radius_move(
                            Bases[other_id].x, Bases[other_id].y, x, y, 0x15);
                        if (other_radius >= 0 && other_radius < 32 &&
                            (Bases[other_id].resource_sqr_radius &
                             (1u << other_radius)) != 0) {
                            contested_by_neighbor = true;
                            break;
                        }
                    }
                }
                if (!contested_by_neighbor) {
                    ++good_unworked_count;
                    if ((tile.val3 & 0xC0) != 0 || bonus_at(x, y, 0) != 0)
                        ++worked_bonus_count;
                }
            }
        } else if ((base.resource_sqr_radius & worked_bit) != 0 &&
                   (tile_flags & BIT_FARM) == 0 &&
                   ((tile.val3 & 0xC0) < 0x80) &&
                   ((tile_flags & BIT_FUNGUS) == 0 ||
                    (tile.climate & 0xE0) < ALT_BIT_OCEAN_SHELF)) {
            if ((tile.climate & 0x18) < 0x10 ||
                has_tech(Rules.tech_three_nutrients_sqr, faction) != 0 ||
                bonus_at(x, y, 0) == 1)
                ++worked_square_count;
        }
    }

    // Expansion looks first for a larger connected land region. The saved
    // water-site objective is deliberately separate from the later former
    // action scoring and preserves the original early-exit behavior.
    int ocean_tile_count = 0;
    int expansion_region = -1;
    uint32_t expansion_region_size = 0;
    int expansion_score = 0;
    int expansion_x = -1;
    int expansion_y = -1;
    bool expansion_candidate_is_shelf = false;
    for (radius_index = 0; radius_index < 21; ++radius_index) {
        int x = base.x + RadiusOffsetX[radius_index];
        if ((MapIsFlat & 1) == 0) {
            if (x < 0) x += MapLongitudeBounds;
            else if (x >= MapLongitudeBounds) x -= MapLongitudeBounds;
        }
        const int y = base.y + RadiusOffsetY[radius_index];
        if (y < 0 || y >= MapLatitudeBounds || x < 0 ||
            x >= MapLongitudeBounds)
            continue;
        const Map &tile = *map_loc(x, y);
        const uint8_t altitude = tile.climate & 0xE0;
        if (altitude >= ALT_BIT_SHORE_LINE) continue;
        ++ocean_tile_count;
        if (radius_index == 0 || altitude != ALT_BIT_OCEAN_SHELF) continue;
        const bool region_can_compete = expansion_region < 0 ||
            tile.region == expansion_region ||
            Continents[tile.region].tile_count > expansion_region_size;
        if (!region_can_compete) continue;
        if (can_terraform(faction, x, y, TRUE, BaseIDCurrentSelected) == 0)
            return;
        expansion_region = tile.region;
        expansion_region_size = Continents[tile.region].tile_count;
        int score = 21 - radius_index;
        if (bonus_at(x, y, 0) != 0) score += 21;
        if (score > expansion_score) {
            expansion_score = score;
            expansion_x = x;
            expansion_y = y;
            expansion_candidate_is_shelf = true;
        }
    }
    if (expansion_candidate_is_shelf && expansion_x >= 0 &&
        map_loc(expansion_x, expansion_y)->region == map_loc(base.x, base.y)->region)
        add_goal(faction, AI_GOAL_TERRAFORM_WATER, 2, expansion_x, expansion_y,
                 BaseIDCurrentSelected);

    base.unk_x = -1;
    base.unk_y = -1;

    int edge_square_count = 0;
    int radius_rainfall = 0;
    int radius_temperature = 0;
    int special_land_square_count = 0;
    int worked_special_land_count = 0;
    int worked_special_square_count = 0;
    int map_improvement_count = 0;
    bool thermal_borehole_present = false;
    bool condenser_present = false;
    int nearest_open_square_count = 0;
    const int fungus_tech_total = PlayersData[faction].tech_fungus_nutrient +
                                  PlayersData[faction].tech_fungus_mineral +
                                  PlayersData[faction].tech_fungus_energy;
    for (radius_index = 0; radius_index < 21; ++radius_index) {
        int x = base.x + RadiusOffsetX[radius_index];
        if ((MapIsFlat & 1) == 0) {
            if (x < 0) x += MapLongitudeBounds;
            else if (x >= MapLongitudeBounds) x -= MapLongitudeBounds;
        }
        const int y = base.y + RadiusOffsetY[radius_index];
        if (y < 0 || y >= MapLatitudeBounds || x < 0 ||
            x >= MapLongitudeBounds) {
            ++edge_square_count;
            continue;
        }

        const Map &tile = *map_loc(x, y);
        const uint8_t climate = tile.climate;
        const uint8_t altitude = climate & 0xE0;
        const uint32_t flags = tile.bit;
        const int rainfall = (climate >> 3) & 3;
        const int rocky = (tile.val3 >> 6) & 3;
        if (altitude < ALT_BIT_SHORE_LINE &&
            !(force_improve != 0 && (Players[faction].rule_flags & RFLAG_AQUATIC)))
            continue;

        if ((flags & (BIT_CONDENSER | BIT_ECH_MIRROR | BIT_THERMAL_BORE)) != 0)
            ++map_improvement_count;
        const bool elevated_fungus = (flags & BIT_FUNGUS) != 0 &&
                                     altitude >= ALT_BIT_OCEAN_SHELF;
        const int altitude_level = (altitude >> 5) - 2;
        const int normalized_altitude = altitude_level < 0 ? 0 : altitude_level;
        if (radius_index != 0 &&
            (normalized_altitude + rainfall + rocky > 3 ||
             (flags & BIT_FOREST) != 0 ||
             (elevated_fungus && fungus_tech_total > 3))) {
            ++nearest_open_square_count;
            if ((base.resource_sqr_radius & (1u << (radius_index & 0x1F))) != 0)
                ++worked_special_land_count;

            const bool fungus_in_high_altitude = elevated_fungus;
            if (!fungus_in_high_altitude) {
                const bool forest = (flags & BIT_FOREST) != 0;
                const bool farm_or_rocky_mine = (flags & BIT_FARM) != 0 ||
                    (rocky > 1 && (flags & BIT_MINE) != 0);
                const bool terrain_kind_allowed = rocky > 1 ||
                    (flags & BIT_SOLAR_TIDAL) != 0 ||
                    ((flags & BIT_MINE) != 0 && (flags & BIT_FARM) != 0);
                if (forest || (farm_or_rocky_mine && terrain_kind_allowed)) {
                    ++special_land_square_count;
                    if ((base.resource_sqr_radius &
                         (1u << (radius_index & 0x1F))) != 0)
                        ++worked_special_square_count;
                }
            } else if (fungus_tech_total >= 4) {
                ++special_land_square_count;
                if ((base.resource_sqr_radius &
                     (1u << (radius_index & 0x1F))) != 0)
                    ++worked_special_square_count;
            } else if (has_tech(Terraforming[TERRA_REMOVE_FUNGUS].preq_tech,
                                faction) == 0) {
                --nearest_open_square_count;
            }
        }
        radius_rainfall += rainfall;
        radius_temperature += climate & 7;
    }
    const int radius_score_count = edge_square_count + radius_rainfall;
    const int high_edge_square_count = (radius_score_count > 14 ? 1 : 0) +
                                       (radius_score_count > 25 ? 1 : 0);

    // The native population adjustment uses the same low-byte human test as
    // the priority branch above.  It is applied after the complete 21-square
    // aggregate, before any individual candidate is scored.
    if (human && base.population_size > 3) --nearest_open_square_count;

    int best_action_score = 0;
    int best_action_radius = -1;
    int best_action = 0;
    int best_condenser_score = -1;
    int best_condenser_x = -1;
    int best_condenser_y = -1;
    int best_landing_score = -1;
    int best_landing_x = -1;
    int best_landing_y = -1;
    int best_thermal_score = -1;
    int best_thermal_x = -1;
    int best_thermal_y = -1;
    int high_elevation_candidate_count = 0;
    const int base_region = map_loc(base.x, base.y)->region;
    for (radius_index = 0; radius_index < 21; ++radius_index) {
        int x = base.x + RadiusOffsetX[radius_index];
        if ((MapIsFlat & 1) == 0) {
            if (x < 0) x += MapLongitudeBounds;
            else if (x >= MapLongitudeBounds) x -= MapLongitudeBounds;
        }
        const int y = base.y + RadiusOffsetY[radius_index];
        if (y < 0 || y >= MapLatitudeBounds || x < 0 ||
            x >= MapLongitudeBounds)
            continue;

        const Map &tile = *map_loc(x, y);
        const uint8_t entry_climate = tile.climate;
        const uint8_t entry_altitude = entry_climate & 0xE0;
        const uint32_t entry_flags = tile.bit;
        if ((entry_flags & BIT_MONOLITH) != 0) continue;
        // Native latches existing boreholes/condensers before excluding the
        // center square or applying the remaining candidate gates.
        if ((entry_flags & BIT_THERMAL_BORE) != 0) thermal_borehole_present = true;
        if ((entry_flags & BIT_CONDENSER) != 0) condenser_present = true;
        if (radius_index == 0) continue;
        if (entry_altitude < ALT_BIT_SHORE_LINE &&
            !(force_improve != 0 &&
              (Players[faction].rule_flags & RFLAG_AQUATIC) != 0))
            continue;
        if ((entry_flags & BIT_FUNGUS) != 0 &&
            entry_altitude >= ALT_BIT_OCEAN_SHELF &&
            !has_tech(Terraforming[TERRA_REMOVE_FUNGUS].preq_tech, faction))
            continue;
        if (tile.region != base_region) continue;

        const int zoc_owner = zoc_move(x, y, faction);
        if (zoc_owner >= 0) {
            if (zoc_owner == 1) continue;
            const int treaty_index = zoc_owner - 1;
            if (treaty_index >= 0 && treaty_index < MaxPlayerNum &&
                (PlayersData[faction].diplo_treaties[treaty_index] & 0x0C30) != 0)
                continue;
            const unsigned int occupant = tile.val2 & 0x0F;
            if ((tile.bit & 0x3u) != 0 && occupant <= 7) continue;
        }
        const int territory = whose_territory(faction, x, y, NULL, FALSE);
        if (territory >= 0 && territory != faction) continue;

        // These are the post-territory values the native scorer reloads after
        // the zoc/territory callbacks, before crop and bonus calculations.
        const uint8_t altitude_climate = tile.climate;
        const uint8_t altitude = altitude_climate & 0xE0;
        const uint32_t flags = tile.bit;
        const int altitude_level_raw = (altitude >> 5) - 2;
        const int altitude_level = altitude_level_raw < 0 ? 0 : altitude_level_raw;
        const int crop = crop_yield(faction, -1, x, y, TRUE);
        const int rocky = (tile.val3 >> 6) & 3;
        const int bonus = bonus_at(x, y, 0);
        const uint8_t climate = tile.climate;
        const int rainfall = (climate & 0x18) >> 3;
        const bool farm = (flags & BIT_FARM) != 0;
        bool forest = (flags & BIT_FOREST) != 0;
        const bool solar_or_bore = (flags & (BIT_SOLAR_TIDAL |
            BIT_THERMAL_BORE)) != 0;
        const bool mine_or_bore = (flags & (BIT_MINE | BIT_THERMAL_BORE)) != 0;
        bool special_improvement = (flags & (BIT_CONDENSER |
            BIT_ECH_MIRROR | BIT_THERMAL_BORE)) != 0;
        bool high_fungus = (flags & BIT_FUNGUS) != 0 &&
                           altitude >= ALT_BIT_OCEAN_SHELF;
        bool worked = (base.resource_sqr_radius &
            (1u << (radius_index & 0x1F))) != 0;
        const int sensor = is_sensor(x, y);
        const uint32_t post_sensor_flags = tile.bit;
        const uint8_t post_sensor_climate = tile.climate;
        forest = (post_sensor_flags & BIT_FOREST) != 0;
        special_improvement = (post_sensor_flags & (BIT_CONDENSER |
            BIT_ECH_MIRROR | BIT_THERMAL_BORE)) != 0;
        high_fungus = (post_sensor_flags & BIT_FUNGUS) != 0 &&
            (post_sensor_climate & 0xE0) >= ALT_BIT_OCEAN_SHELF;
        if (forest) {
            int forest_score = (sensor == 0 ? 1 : -1) + altitude_level +
                rocky + rainfall;
            if (has_tech(Rules.tech_three_nutrients_sqr, faction) != 0)
                ++forest_score;
            int facility_byte = 0;
            int facility_mask = 0;
            bitmask(0x0D, &facility_byte, &facility_mask);
            if ((Bases[BaseIDCurrentSelected].facilities_built[facility_byte] &
                 static_cast<uint8_t>(facility_mask)) != 0)
                --forest_score;
            bitmask(0x0E, &facility_byte, &facility_mask);
            if ((Bases[BaseIDCurrentSelected].facilities_built[facility_byte] &
                 static_cast<uint8_t>(facility_mask)) != 0)
                --forest_score;
            if (forest_score > 4) forest = false;
        }
        int fungus_score_factor = high_fungus ? 4 : 0;
        if (fungus_score_factor != 0) {
            const int fungus_yield_rank = (sensor == 0 ? 1 : 0) +
                altitude_level + rocky + rainfall;
            if (fungus_yield_rank < fungus_tech_total) continue;
            if (fungus_yield_rank == fungus_tech_total)
                fungus_score_factor = 1;
        }
        if (!worked && worked_special_square_count == worked_special_land_count &&
            worked_bonus_count == 0 && !forest && fungus_score_factor == 0 &&
            rainfall != 0 && rocky < 2)
            worked = true;

        // Native suppresses landing-site ranking after the second square
        // above altitude level four. The counter is cumulative across the
        // same eligible-square walk, not merely the current radius ring.
        if ((altitude >> 5) > 4 && ++high_elevation_candidate_count > 1)
            best_landing_score = -99999;

        // Native first adjusts the forest discriminator from local yield,
        // technology, and the selected base's facility bits. The sensor-array
        // candidate is then ranked before farm/solar.
        const uint32_t terrain_action_mask = flags & 0x01E00050u;
        const bool fungus_technology = has_tech(
            Rules.tech_improve_fungus_sqr, faction) != 0;
        const bool human_sensor_policy =
            (static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) &
             static_cast<uint8_t>(faction_bit)) != 0 &&
            (GameDataStateStorage.fields_.game_more_preferences_ & 0x20u) == 0;
        const bool sensor_site_allowed = sensor_count == 0 && sensor == 0 &&
            bonus == 0 && (terrain_action_mask == 0 ||
                           base.population_size > 7) &&
            !human_sensor_policy &&
            (worked_bonus_count == 0 ||
             base.population_size - 1 <= worked_square_count ||
             base.population_size > 1) &&
            (fungus_score_factor == 0 || fungus_technology) &&
            (rocky > 1 || forest || rainfall == 0 ||
             (fungus_score_factor != 0 && fungus_technology));
        if (sensor_site_allowed) {
            int score = (static_cast<int>(base.population_size) + 2 +
                PlayersData[faction].ai_fight * 2) *
                ((forest || fungus_score_factor != 0 || rainfall == 0) ? 8 : 4);
            bool shallow_neighbor = false;
            for (int sensor_direction = 0; sensor_direction < 8;
                 ++sensor_direction) {
                int neighbor_x = x + RadiusBaseX[sensor_direction];
                if ((MapIsFlat & 1) == 0) {
                    if (neighbor_x < 0) neighbor_x += MapLongitudeBounds;
                    else if (neighbor_x >= MapLongitudeBounds)
                        neighbor_x -= MapLongitudeBounds;
                }
                const int neighbor_y = y + RadiusBaseY[sensor_direction];
                if (neighbor_y < 0 || neighbor_y >= MapLatitudeBounds ||
                    neighbor_x < 0 || neighbor_x >= MapLongitudeBounds)
                    continue;
                if ((map_loc(neighbor_x, neighbor_y)->climate & 0xE0) <
                    ALT_BIT_SHORE_LINE) {
                    shallow_neighbor = true;
                    break;
                }
            }
            if (!shallow_neighbor) score *= 2;
            if (score > best_action_score) {
                best_action_score = score;
                best_action_radius = radius_index;
                best_action = 9;
            }
        }

        // Native farm/solar ranking. The two actions share their crop-yield
        // and worked-square score; the native tie-break prefers farm only
        // when the food/energy state allows it and no solar/borehole exists.
        if ((energy_need != 0 || (base.state & 0x8000) != 0) && !forest &&
            !special_improvement &&
            (sensor == 0 || (2 - (worked ? 1 : 0)) < sensor_count) &&
            (!mine_or_bore || (mineral_need == 0 && altitude_level > 1))) {
            const int reduced_need = energy_need < 2 || altitude_level < 2 ? 0 : 1;
            if ((static_cast<int>(farm) + 1 - reduced_need) <= crop &&
                altitude_level != 0) {
                int score = altitude_level * 3;
                if (bonus == 2 || rocky == 1) score += 2;
                if (bonus == 1) score += 2;
                else if (bonus == 3) score += farm ? -1 : 1;
                if (farm) ++score;
                if (rocky == 2) score -= 3;
                int scaled_need = energy_need * 2;
                if (scaled_need < 1) scaled_need = 1;
                else if (scaled_need > 99) scaled_need = 99;
                score *= scaled_need;
                if (worked) score *= 2;
                if (fungus_score_factor != 0)
                    score += base.eco_damage * fungus_score_factor * 2;

                if ((worked || energy_need > 1) && !farm &&
                    (bonus == 3 || energy_need == 0 || nutrient_need_state != 0) &&
                    rocky < 2) {
                    if (nutrient_need_state != 0) score *= 2;
                    if (score > best_action_score) {
                        best_action_score = score;
                        best_action_radius = radius_index;
                        best_action = 0;
                    }
                } else if (!solar_or_bore && score > best_action_score) {
                    best_action_score = score;
                    best_action_radius = radius_index;
                    best_action = 3;
                }
            }
        } else {
            int score = 0;
            if (rocky == 0) score = crop + 1;
            else if (rocky == 1) score = 3 - crop;
            else if (rocky == 2) score = 6;
            if (bonus == 1) score += 4;
            else if (bonus == 2) score *= 2;
            else if (bonus == 3) score += 2;
            if (farm) ++score;
            if ((flags & BIT_ROAD) != 0) ++score;
            if (forest) --score;
            score *= mineral_need;
            score *= worked ? 6 : 3;
            if (fungus_score_factor != 0)
                score += base.eco_damage * fungus_score_factor * 2;

            if (!mine_or_bore && !special_improvement) {
                if (score > best_action_score) {
                    best_action_score = score;
                    best_action_radius = radius_index;
                    best_action = 2;
                }
            } else if ((flags & BIT_ROAD) == 0 && (mine_or_bore ||
                       (flags & BIT_THERMAL_BORE) != 0)) {
                const bool suppress_mine_action =
                    (static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) &
                     static_cast<uint8_t>(faction_bit)) != 0 &&
                    (GameDataStateStorage.fields_.game_more_preferences_ & 0x00400000u) != 0;
                if (!suppress_mine_action && score > best_action_score) {
                    best_action_score = score;
                    best_action_radius = radius_index;
                    best_action = 5;
                }
            } else if ((flags & BIT_ROAD) != 0 && mine_or_bore &&
                       !special_improvement && !farm && rocky < 2 &&
                       (((climate & 0x18) != 0) || bonus == 1) &&
                       score > best_action_score) {
                best_action_score = score;
                best_action_radius = radius_index;
                best_action = 0;
            }
        }
        if (fungus_score_factor != 0) {
            const int fungus_score =
                base.eco_damage * fungus_score_factor * 2;
            if (fungus_score > best_action_score) {
                best_action_score = fungus_score;
                best_action_radius = radius_index;
                best_action = 10;
            }
        }

        if (!condenser_present) {
            int site_score = 21 - radius_index;
            if ((flags & 0x01E00050u) == 0) site_score += 8;
            if (!farm) site_score += 2;
            const int rainfall_delta = 2 - rainfall;
            site_score += (rainfall_delta * rainfall_delta + rocky * 2) * 2;
            if (MapCloudCover == 0 && rainfall < 2) site_score *= 2;
            if (site_score > best_condenser_score) {
                best_condenser_score = site_score;
                best_condenser_x = x;
                best_condenser_y = y;
            }
        }

        {
            int site_score = RadiusOffsetX[radius_index] *
                RadiusOffsetX[radius_index] * 2;
            const int altitude_code = altitude >> 5;
            if (altitude_code == 3) {
                if (rocky == 1) site_score += 3;
                if (bonus == 1) site_score += 6;
            } else {
                if (bonus != 1) site_score -= crop * 3;
                if (rocky == 1) site_score -= 4;
            }
            int supported_ocean_count = ocean_tile_count;
            for (int ring = 1; ring <= altitude_code && ring < 9; ++ring) {
                bool found_lower_neighbor = false;
                const int first = RadiusRange[ring - 1];
                const int last = RadiusRange[ring];
                const int altitude_limit = altitude_code - ring + 1;
                for (int neighbor_index = first; neighbor_index < last;
                     ++neighbor_index) {
                    int neighbor_x = x + RadiusOffsetX[neighbor_index];
                    if ((MapIsFlat & 1) == 0) {
                        if (neighbor_x < 0) neighbor_x += MapLongitudeBounds;
                        else if (neighbor_x >= MapLongitudeBounds)
                            neighbor_x -= MapLongitudeBounds;
                    }
                    const int neighbor_y = y + RadiusOffsetY[neighbor_index];
                    if (neighbor_y < 0 || neighbor_y >= MapLatitudeBounds ||
                        neighbor_x < 0 || neighbor_x >= MapLongitudeBounds)
                        continue;
                    const Map &neighbor = *map_loc(neighbor_x, neighbor_y);
                    if ((neighbor.climate >> 5) >= altitude_limit) continue;
                    found_lower_neighbor = true;
                    int dx = abs(neighbor_x - x);
                    if ((MapIsFlat & 1) == 0 && dx > MapLongitude)
                        dx = MapLongitudeBounds - dx;
                    const int dy = abs(neighbor_y - y);
                    const int distance = vector_dist(dx, dy);
                    if (distance < 3) {
                        ++site_score;
                        if (distance < 2 &&
                            (neighbor.climate & 0xE0) == ALT_BIT_OCEAN_SHELF)
                            --supported_ocean_count;
                    }
                }
                if (!found_lower_neighbor) break;
            }
            if ((high_edge_square_count == 0 &&
                 best_landing_score > -99999 &&
                 (best_landing_score > -9999 || bonus == 1)) &&
                (ocean_tile_count == 0 || supported_ocean_count > 0)) {
                if (mine_or_bore) site_score -= 4;
                if (solar_or_bore) site_score += 2;
                if (bonus == 1) site_score += 8;
                if (radius_index < 8) site_score /= 2;
                if (site_score > best_landing_score) {
                    best_landing_score = site_score;
                    best_landing_x = x;
                    best_landing_y = y;
                }
            }

        }

        if (!thermal_borehole_present) {
            int thermal_score = (rocky == 2 ? 0x0C : 0) -
                radius_index + 21 + ((rocky == 2 || rainfall == 0) ? 12 : 0);
            if ((flags & 0x01E00050u) != 0) thermal_score -= 8;
            for (int thermal_direction = 0; thermal_direction < 8;
                 ++thermal_direction) {
                int neighbor_x = x + RadiusBaseX[thermal_direction];
                if ((MapIsFlat & 1) == 0) {
                    if (neighbor_x < 0) neighbor_x += MapLongitudeBounds;
                    else if (neighbor_x >= MapLongitudeBounds)
                        neighbor_x -= MapLongitudeBounds;
                }
                const int neighbor_y = y + RadiusBaseY[thermal_direction];
                if (neighbor_y < 0 || neighbor_y >= MapLatitudeBounds ||
                    neighbor_x < 0 || neighbor_x >= MapLongitudeBounds)
                    continue;
                const Map &neighbor = *map_loc(neighbor_x, neighbor_y);
                const uint8_t neighbor_altitude = neighbor.climate & 0xE0;
                if (neighbor_altitude >= ALT_BIT_SHORE_LINE &&
                    (((neighbor_altitude >> 5) < (altitude >> 5)) ||
                     (neighbor.bit & BIT_THERMAL_BORE) != 0)) {
                    thermal_score = -1;
                    break;
                }
            }
            if (thermal_score > best_thermal_score) {
                best_thermal_score = thermal_score;
                best_thermal_x = x;
                best_thermal_y = y;
            }
        }
        (void)rainfall;
    }

    // Commit the action candidate through the native goal/connection path
    // before the special-improvement and relocation phases below.
    int connection_priority = has_tech(Terraforming[TERRA_MAGTUBE].preq_tech,
                                       faction)
        ? 2 : 1;
    const bool allow_connection_goal =
        (static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) &
         static_cast<uint8_t>(faction_bit)) == 0 ||
        (GameDataStateStorage.fields_.game_more_preferences_ & 0x00400000u) == 0;
    if (best_action_radius < 1) {
        if (allow_connection_goal) base_connect(2);
    } else {
        int selected_x = base.x + RadiusOffsetX[best_action_radius];
        if ((MapIsFlat & 1) == 0) {
            if (selected_x < 0) selected_x += MapLongitudeBounds;
            else if (selected_x >= MapLongitudeBounds)
                selected_x -= MapLongitudeBounds;
        }
        const int selected_y = base.y + RadiusOffsetY[best_action_radius];
        if (best_action == 9) {
            add_goal(faction, AI_GOAL_SENSOR_ARRAY, 2, selected_x, selected_y,
                     BaseIDCurrentSelected);
        } else if (best_action == 5 ||
                   can_terraform(faction, selected_x, selected_y, TRUE,
                                 BaseIDCurrentSelected) != 0) {
            add_goal(faction, AI_GOAL_TERRAFORM_LAND, 2, selected_x, selected_y,
                     BaseIDCurrentSelected);
        } else {
            connection_priority = 2;
        }
        if (allow_connection_goal) base_connect(connection_priority);

        if (force_improve != 0) {
            Map *selected_tile = map_loc(selected_x, selected_y);
            selected_tile->bit &= ~BIT_FUNGUS;
            int improvement = can_terraform(faction, selected_x, selected_y,
                TRUE, BaseIDCurrentSelected);
            if (ExpansionEnabled &&
                (Players[faction].rule_flags & RFLAG_AQUATIC) != 0) {
                uint32_t choice = (timeGetTime() +
                    static_cast<uint32_t>(BaseIDCurrentSelected) +
                    static_cast<uint32_t>(selected_y) +
                    static_cast<uint32_t>(selected_x)) & 3u;
                if (choice != 0) ++choice;
                improvement = static_cast<int>(choice + 4u);
            }
            switch (improvement) {
            case 4:
                selected_tile->bit |= BIT_FARM | BIT_ROAD;
                if ((selected_tile->climate & 7) == 0 &&
                    (selected_tile->climate & 0xE0) < 0x61)
                    selected_tile->bit |= BIT_MINE;
                else
                    selected_tile->bit |= BIT_SOLAR_TIDAL;
                break;
            case 6:
                selected_tile->bit |= BIT_MINE | BIT_ROAD;
                if ((selected_tile->val3 & 0xC0) < 0x80)
                    selected_tile->bit |= BIT_FARM;
                break;
            case 7:
                selected_tile->bit |= BIT_SOLAR_TIDAL | BIT_ROAD;
                break;
            case 8:
                selected_tile->bit |= BIT_FOREST;
                break;
            }
        }
    }

    const bool too_few_special_land_squares =
        special_land_square_count < static_cast<int>(base.population_size) + 1 &&
        special_land_square_count < nearest_open_square_count;
    bool thermal_site_allowed = false;
    if (!thermal_borehole_present && best_thermal_score > 0 &&
        base.eco_damage == 0) {
        const Map &candidate = *map_loc(best_thermal_x, best_thermal_y);
        const int rainfall_bits = candidate.climate & 0x18;
        const int rocky_bits = candidate.val3 & 0xC0;
        int terrain_pattern_count = rainfall_bits == 0 ? 1 : 0;
        if (rocky_bits == 0x80) ++terrain_pattern_count;
        if (rocky_bits == 0 && rainfall_bits < 0x10)
            ++terrain_pattern_count;
        if (terrain_pattern_count != 0) {
            if (base.population_size < 8 && energy_need < 2) {
                if (has_tech(Ability[1].preq_tech, faction) != 0 ||
                    GameDataStateStorage.fields_.turn_current_num_ >= 0xC9)
                    thermal_site_allowed = true;
            } else {
                thermal_site_allowed = true;
            }
        }
    }
    const bool thermal_goal_allowed = thermal_site_allowed &&
        terrain_avail(TERRA_THERMAL_BORE, FALSE, faction) != 0 &&
        ((static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) &
          static_cast<uint8_t>(faction_bit)) == 0 ||
         (faction == ConsoleGlobal.faction_id_ &&
          (GameDataStateStorage.fields_.game_preferences_ & 0x02000000u) != 0));

    bool condenser_site_allowed = false;
    if (!condenser_present && best_condenser_score > 0 &&
        base.eco_damage == 0) {
        if (high_edge_square_count == 0) {
            condenser_site_allowed = true;
        } else if (high_edge_square_count < 2 && map_improvement_count == 0) {
            if (nutrient_need_state != 0) {
                condenser_site_allowed = true;
            } else {
                const Map &candidate = *map_loc(best_condenser_x,
                                                best_condenser_y);
                const int rainfall_bits = candidate.climate & 0x18;
                const int rocky_bits = candidate.val3 & 0xC0;
                int terrain_pattern_count = rainfall_bits == 0 ? 1 : 0;
                if (rocky_bits == 0x80) ++terrain_pattern_count;
                if (rocky_bits == 0 && rainfall_bits < 0x10)
                    ++terrain_pattern_count;
                if (terrain_pattern_count != 0 &&
                    (has_tech(Ability[1].preq_tech, faction) != 0 ||
                     GameDataStateStorage.fields_.turn_current_num_ > 200))
                    condenser_site_allowed = true;
            }
        }
    }
    const bool condenser_goal_allowed = condenser_site_allowed &&
        terrain_avail(TERRA_CONDENSER, FALSE, faction) != 0 &&
        ((static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) &
          static_cast<uint8_t>(faction_bit)) == 0 ||
         (faction == ConsoleGlobal.faction_id_ &&
          (GameDataStateStorage.fields_.game_preferences_ & 0x02000000u) != 0));

    // The native tail chooses at most one objective in this family: thermal
    // bore first, condenser as its fallback, and the landing-site candidate
    // only if neither alt-site objective survives its gates. A shortage of
    // special squares bypasses the alt-site branch and rejects landing too.
    if (!too_few_special_land_squares) {
        if (thermal_goal_allowed) {
            add_goal(faction, AI_GOAL_THERMAL_BOREHOLE, 1, best_thermal_x,
                     best_thermal_y, BaseIDCurrentSelected);
        } else if (condenser_goal_allowed) {
            add_goal(faction, AI_GOAL_CONDENSER, 1, best_condenser_x,
                     best_condenser_y, BaseIDCurrentSelected);
        } else {
            const bool landing_site_rejected =
                (high_edge_square_count != 0 && energy_need < 3) ||
                best_landing_score < 1 ||
                (human && faction != ConsoleGlobal.faction_id_ &&
                 (GameDataStateStorage.fields_.game_more_preferences_ & 0x00020000u) == 0) ||
                terrain_avail(16, FALSE, faction) == 0;
            if (!landing_site_rejected)
                add_goal(faction, AI_GOAL_LANDING_SITE, 1, best_landing_x,
                         best_landing_y, BaseIDCurrentSelected);
        }
    }

    // Native's final base-relative relocation hint is the square eight map
    // columns west of the base. It is retained only when that square is
    // in-bounds, lies on land below the polar cap, and the local edge/rainfall
    // aggregate crossed at least one of the original thresholds.
    int relocation_x = base.x - 8;
    if ((MapIsFlat & 1) == 0) {
        if (relocation_x < 0) relocation_x += MapLongitudeBounds;
        else if (relocation_x >= MapLongitudeBounds)
            relocation_x -= MapLongitudeBounds;
    }
    const int relocation_y = base.y;
    if (relocation_y < 0 || relocation_y >= MapLatitudeBounds ||
        relocation_x < 0 || relocation_x >= MapLongitudeBounds ||
        high_edge_square_count == 0 ||
        (map_loc(relocation_x, relocation_y)->climate & 0xE0) <
            ALT_BIT_SHORE_LINE ||
        (map_loc(relocation_x, relocation_y)->climate & 0xE0) > 0xBF) {
        base.unk_x = -1;
        base.unk_y = -1;
    } else {
        base.unk_x = static_cast<int16_t>(relocation_x);
        base.unk_y = static_cast<int16_t>(relocation_y);
    }

    (void)best_action_score;
    (void)nutrient_need_state;
    (void)sensor_count;
    (void)good_unworked_count;
    (void)worked_bonus_count;
    (void)worked_square_count;
    (void)ocean_tile_count;
    (void)map_improvement_count;
    (void)special_land_square_count;
    (void)worked_special_land_count;
    (void)worked_special_square_count;
    (void)nearest_open_square_count;
    (void)radius_temperature;
    (void)high_edge_square_count;
    (void)best_condenser_score;
    (void)best_condenser_x;
    (void)best_condenser_y;
    (void)best_landing_score;
    (void)best_landing_x;
    (void)best_landing_y;
    (void)best_thermal_score;
    (void)best_thermal_x;
    (void)best_thermal_y;
    (void)thermal_borehole_present;

    // The native suffix after this hint begins an east-side scan only when
    // local_60 != 2. Its following `setge; dec; and 0x19; add 0x19; jne`
    // computes 25 or 50 on either outcome and branches directly to the epilogue,
    // so that scan has no reachable effects in the pinned executable.
}

// ORIGINAL: 0x004EF1C0 ?base_growth@@YAHXZ 0x004EF1C0-0x004EFB4E
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004ef1c0.cpp", "recovery/tasks/base-colony-economy-coverage-wave.json", "Pinned original01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; complete original function extent reviewed against native instructions and VC6 CodeView owners.", "recovery/tests/test_base_colony_production.py", "Native4EF3CE/4EF50E selects BaseIDCurrentSelected for has_fac, contradicting the retained TRIED owner-byte hypothesis. 9A6548 is SecretProject.ascetic_virtues(+34), not network_backbone; 97D046 is population_size(+6)."], "reason": "Recover starvation, population growth/limits and nutrient warnings through real Base/faction/facility/project/vehicle owners. Preserve callback reloads and native signed-byte population behavior; seeds remain, no matching or runtime claim."}
// symbol    ?base_growth@@YAHXZ
// TRIED: DAT_0090ea30 is Ghidra-typed `short*` throughout, so every `DAT_0090ea30[N]`/`DAT_0090ea30+N` is byte offset N*2 off the Base* (confirmed against raw disasm: index 0x1e/0x68 -> bytes 0x3c/0xd0, index 3 -> byte 6, the growth-stage counter); two has_fac() calls use the local `owner` byte, not DAT_00689370, per raw push order (Ghidra swapped these). Compiles; diverges at #3 (prologue field-read order).
int __cdecl base_growth() {
    const int x = BaseCurrent->x;
    const int y = BaseCurrent->y;
    const int owner = BaseCurrent->faction_id_current;

    if (BaseCurrent->nutrients_accumulated < 0) {
        if ((GameDataStateStorage.fields_.game_state_ & 1) == 0 || (GameDataStateStorage.fields_.game_state_ & 8) != 0) {
            if (BaseCurrentConvoyFrom[0] != 0 && GameDataStateStorage.fields_.vehicle_current_count_ > 0) {
                for (int vehicle_id = 0; vehicle_id < GameDataStateStorage.fields_.vehicle_current_count_; ++vehicle_id) {
                    Veh &vehicle = Vehs[vehicle_id];
                    if (vehicle.home_base_id == BaseIDCurrentSelected
                        && VehPrototypes[vehicle.proto_id].plan == 0xA
                        && vehicle.order == 3
                        && vehicle.order_auto_type == 0) {
                        const int tile_index = vehicle.y * static_cast<int>(MapLongitude)
                            + (vehicle.x >> 1);
                        Map *tile = &MapTiles[tile_index];
                        if ((tile->bit & 1) != 0 && (tile->val2 & 0xF) < 8) {
                            vehicle.order = 0;
                        }
                    }
                }
            }

            const bool alien = (Players[owner].rule_flags & RFLAG_ALIEN) != 0;
            popb("STARVE", 0x4000, 0xE,
                 alien ? "al_starv_sm.pcx" : "starv_sm.pcx", 0);
            if (is_objective(BaseIDCurrentSelected) == 0 || BaseCurrent->population_size > 1) {
                BaseCurrent->population_size = static_cast<int8_t>(static_cast<uint8_t>(
                    static_cast<int>(BaseCurrent->population_size) - 1));
                if (BaseCurrent->population_size < 1) {
                    base_kill(BaseIDCurrentSelected);
                    draw_map(1);
                    eliminate_player(owner, 0);
                    return 1;
                }
            }
        }
        BaseCurrent->nutrients_accumulated = BaseCurrent->nutrient_surplus;
        draw_tile(x, y, 2);
        return 0;
    }

    int growth = cost_factor(owner, 0, BaseIDCurrentSelected);
    growth = base_yield_mul(growth,
        base_yield_add(static_cast<int>(BaseCurrent->population_size), 1));
    const int faction = BaseIDCurrentSelected;

    if (((GameDataStateStorage.fields_.secret_project_.cloning_vats() >= 0
          && Bases[GameDataStateStorage.fields_.secret_project_.cloning_vats()].faction_id_current == owner)
         || BaseCurrentGrowthRate > 5)
        && Rules.nutrient_req_citizen != 0
        && static_cast<int>(Rules.nutrient_req_citizen) <= BaseCurrent->nutrient_surplus) {
        const bool same = GameDataStateStorage.fields_.secret_project_.ascetic_virtues() >= 0
            && Bases[GameDataStateStorage.fields_.secret_project_.ascetic_virtues()].faction_id_current
                == Bases[faction].faction_id_current;
        const int margin = same ? 2 : 0;
        const int total_population =
            Players[Bases[faction].faction_id_current].rule_population;
        if (((Bases[faction].population_size - margin)
                 + total_population
                 < static_cast<int>(Rules.pop_limit_sans_hab_complex)
             || has_fac(0x18, faction, 0) != 0)
            && ((Bases[faction].population_size - margin) + total_population
                 < static_cast<int>(Rules.pop_limit_sans_hab_dome)
                || has_fac(0x19, faction, 0) != 0)) {
            if (BaseCurrent->population_size < 0x7F) {
                BaseCurrent->population_size = static_cast<int8_t>(
                    static_cast<uint8_t>(static_cast<int>(BaseCurrent->population_size) + 1));
            }
            BaseComputedID = BaseIDCurrentSelected;
            base_support();
            base_yield();
            base_nutrient();
            base_minerals();
            base_energy();
            draw_tile(x, y, 2);
            if (BaseCurrent->nutrients_accumulated <= growth) return 0;
            BaseCurrent->nutrients_accumulated = growth;
            return 0;
        }
    }

    if (growth <= BaseCurrent->nutrients_accumulated) {
        if (BaseCurrentGrowthRate < -2) {
            BaseCurrent->nutrients_accumulated = growth;
            return 0;
        }
        const bool same = GameDataStateStorage.fields_.secret_project_.ascetic_virtues() >= 0
            && Bases[GameDataStateStorage.fields_.secret_project_.ascetic_virtues()].faction_id_current
                == Bases[faction].faction_id_current;
        const int margin = same ? 2 : 0;
        const int total_population =
            Players[Bases[faction].faction_id_current].rule_population;
        int limit_reason;
        if ((Bases[faction].population_size - margin) + total_population
                < static_cast<int>(Rules.pop_limit_sans_hab_complex)
            || has_fac(0x18, faction, 0) != 0) {
            if ((Bases[faction].population_size - margin) + total_population
                    < static_cast<int>(Rules.pop_limit_sans_hab_dome)
                || has_fac(0x19, faction, 0) != 0) {
                BaseCurrent->nutrients_accumulated = 0;
                BaseCurrent->population_size = static_cast<int8_t>(
                    static_cast<uint8_t>(static_cast<int>(BaseCurrent->population_size) + 1));
                BaseComputedID = BaseIDCurrentSelected;
                base_support();
                base_yield();
                base_nutrient();
                base_minerals();
                base_energy();
                draw_tile(x, y, 2);
                goto after_growth;
            }
            limit_reason = 0x19;
        } else {
            limit_reason = 0x18;
        }
        growth = cost_factor(BaseCurrent->faction_id_current, 0,
                             BaseIDCurrentSelected);
        BaseCurrent->nutrients_accumulated = base_yield_mul(growth,
            base_yield_add(static_cast<int>(BaseCurrent->population_size), 1)) / 2;
        parse_say(1, static_cast<int>(reinterpret_cast<uintptr_t>(
            Facility[limit_reason].name)), -1, -1);
        if ((Players[owner].rule_flags & RFLAG_ALIEN) == 0) {
            popb("POPULATIONLIMIT", 0x800, 0x10, "pop_sm.pcx", 0);
        } else {
            popb("POPULATIONLIMIT", 0x800, 0x10, "al_pop_sm.pcx", 0);
        }
    }

after_growth:
    BaseCurrent->nutrients_accumulated = base_yield_add(
        BaseCurrent->nutrients_accumulated, BaseCurrent->nutrient_surplus);
    {
        const int threshold = BaseCurrent->nutrient_surplus;
        if (threshold < 0
            && base_yield_sub(BaseCurrent->nutrients_accumulated, threshold) >= 0
            && base_yield_add(base_yield_add(threshold,
                BaseCurrent->nutrients_accumulated), base_yield_mul(threshold, 4)) < 0
            && (BaseCurrent->governor & 0x80000040u) != 0x80000040u) {
            int adjacent = 0;
            int bad_farms = 0;
            for (int direction = 1; direction < 0x15; ++direction) {
                int nx = x + RadiusOffsetX[direction];
                if ((MapIsFlat & 1) == 0) {
                    if (nx < 0) nx += MapLongitudeBounds;
                    else if (nx >= MapLongitudeBounds) nx -= MapLongitudeBounds;
                }
                const int ny = y + RadiusOffsetY[direction];
                if (ny < 0 || ny >= MapLatitudeBounds
                    || nx < 0 || nx >= MapLongitudeBounds) continue;
                Map &tile = MapTiles[(nx >> 1) + ny * MapLongitude];
                if ((tile.climate & 0xE0) < 0x60) {
                    if ((BaseCurrent->resource_sqr_radius & (1u << (direction & 0x1F))) != 0
                        && (tile.bit & 0x8000) == 0) ++bad_farms;
                    continue;
                }
                const uint32_t flags = tile.bit;
                if ((flags & 0x8000) != 0) continue;
                bool skip = false;
                if ((flags & 0x200000) != 0) {
                    int byte_offset;
                    int bit_mask;
                    bitmask(0xD, &byte_offset, &bit_mask);
                    if ((Bases[BaseIDCurrentSelected].facilities_built[byte_offset]
                         & static_cast<uint8_t>(bit_mask)) != 0) skip = true;
                }
                if (skip) continue;
                const uint8_t climate = tile.climate;
                const uint8_t rainfall = static_cast<uint8_t>(climate >> 3);
                const uint8_t rainfall_class = rainfall & 3;
                if ((flags & 0x20) == 0 || (climate & 0xE0) < 0x40
                    || PlayersData[owner].tech_fungus_nutrient < 2) {
                    const bool worked = (BaseCurrent->resource_sqr_radius
                        & (1u << (direction & 0x1F))) != 0;
                    if (!worked) {
                        if ((flags & 0x8000) == 0) {
                            const bool count = rainfall_class == 1
                                || has_tech(Rules.tech_three_nutrients_sqr, owner) != 0;
                            if (count) ++adjacent;
                        }
                    } else if (((flags & 0x20) != 0 && (climate & 0xE0) > 0x3F)
                               || (flags & 0x200000) != 0
                               || (tile.val3 & 0xC0) == 0x80
                               || rainfall_class == 0) {
                        ++bad_farms;
                    } else if ((flags & 0x8000) == 0) {
                        const bool count = rainfall_class == 1
                            || has_tech(Rules.tech_three_nutrients_sqr, owner) != 0;
                        if (count) ++adjacent;
                    }
                }
            }

            const bool alien = (Players[owner].rule_flags & RFLAG_ALIEN) != 0;
            if (alien) {
                if (adjacent != 0
                    && has_tech(VehPrototypes[1].preq_tech, owner) != 0) {
                    if (PlayersData[owner].proto_id_active[1] != 0) {
                    popb("LOWNUTRIENTFARMS", 0, -1, "al_starv_sm.pcx", 0);
                } else {
                    popb("LOWNUTRIENTFORMERS", 0, -1, "al_starv_sm.pcx", 0);
                    }
                    return 0;
                }
                if (BaseCurrent->specialist_total < 2
                    && (BaseCurrent->specialist_total == 0 || BaseCurrent->population_size > 2)) {
                    popb("LOWNUTRIENT", 0x80, 0x10,
                         "al_starv_sm.pcx", 0);
                    return 0;
                }
                if (BasePsychDrones[0] != 0) {
                    popb("LOWNUTRIENTSPEC1", 0, -1, "al_starv_sm.pcx", 0);
                    return 0;
                }
                popb("LOWNUTRIENTSPEC2", 0, -1, "al_starv_sm.pcx", 0);
                return 0;
            }
            if (adjacent != 0
                && has_tech(VehPrototypes[1].preq_tech, owner) != 0) {
                if (PlayersData[owner].proto_id_active[1] != 0) {
                    popb("LOWNUTRIENTFARMS", 0, -1, "starv_sm.pcx", 0);
                } else {
                    popb("LOWNUTRIENTFORMERS", 0, -1, "starv_sm.pcx", 0);
                }
                return 0;
            }
            if (BaseCurrent->specialist_total < 2
                && (BaseCurrent->specialist_total == 0 || BaseCurrent->population_size > 2)) {
                popb("LOWNUTRIENT", 0x80, 0x10, "starv_sm.pcx", 0);
                return 0;
            }
            if (BasePsychDrones[0] != 0) {
                popb("LOWNUTRIENTSPEC1", 0, -1, "starv_sm.pcx", 0);
                return 0;
            }
            popb("LOWNUTRIENTSPEC2", 0, -1, "starv_sm.pcx", 0);
            (void)bad_farms;
        }
    }
    return 0;
}

static int upgrade_arithmetic_shift_three(int value) {
    return value >= 0 ? value / 8 : -1 - ((-1 - value) / 8);
}

// ORIGINAL: 0x004EFE80 ?upgrade_prototype@@YAHHHHH@Z 0x004EFE80-0x004F045B
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004efe80.cpp", "recovery/tasks/base-colony-economy-coverage-wave.json", "Pinned original01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; complete original function extent reviewed against native instructions and VC6 CodeView owners.", "recovery/tests/test_base_colony_production.py", "Native4EFE8E..94 rejects statebit1 set with bit8 clear; 4F03A8/4F0414 choose UPGRADEPROTOCOST2 when prototypeflag4 is clear. Ability iterator9AB540 reads abbreviation(+8)."], "reason": "Recover complete prototype-upgrade decision, naming/prompt and network dispatch through typed owners. Four arguments follow the native caller and body; five H tokens include the int return. Replace central executable-address forwarder, preserve seed and historical obligations, no new byte claim."}
// symbol    ?upgrade_prototype@@YAHHHHH@Z
// TRIED: near-verbatim transcription of the (clean) Ghidra output, substituting catalogued globals/callees; the leading `if (cost==0) { if(a4!=0) return 0; if(!bit) return 0; }` three-branch guard was collapsed via De Morgan into one `if (!(cost != 0 || (a4 == 0 && bit))) return 0;` - logically equivalent (confirmed against raw asm) but restructures the control flow, so it does not reproduce the original's branch-by-branch codegen. The four `popp(...)` calls needed the 2nd/4th arguments swapped from Ghidra's printed order (confirmed via push-order tracing: 1st param dereferences g_00691b0c rather than using the address itself). Reaches MISMATCH #10 ('je' vs 'jne'); did not chase the exact branch-for-branch shape further.
int __cdecl upgrade_prototype(int faction_id, int proto_id_new,
                                 int proto_id_old, int set_flag) {
    const uint32_t faction_bit = 1u << (faction_id & 0x1f);
    if ((GameDataStateStorage.fields_.game_state_ & 1) != 0 && (GameDataStateStorage.fields_.game_state_ & 8) == 0) return 0;

    const int cost = base_yield_mul(base_yield_mul(
        upgrade_cost(faction_id, proto_id_new, proto_id_old),
        PlayersData[faction_id].proto_id_active[proto_id_old]), 10);
    if (cost == 0 && !(set_flag == 0 && (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & faction_bit) != 0)) {
        return 0;
    }

    if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & faction_bit) == 0) {
        const int ranking_factor = 8 - PlayersData[GameDataStateStorage.fields_.ranking_faction_id_unk1_].ranking;
        const int energy = PlayersData[faction_id].energy_reserves;
        int minerals = (base_yield_mul(GameDataStateStorage.fields_.diff_level_current_, cost) / 10)
            / ranking_factor;
        if (PlayersData[GameDataStateStorage.fields_.ranking_faction_id_unk2_].energy_reserves < energy) {
            if (energy > 999) minerals /= 2;
            if (energy > 1999) minerals = 0;
        }
        const int discount = base_yield_add(base_yield_mul(
            base_yield_add(base_yield_add(
                VehPrototypes[proto_id_new].plan == 2 ? 1 : 0,
                PlayersData[faction_id].ai_power),
                PlayersData[faction_id].ai_tech), 2), 1);
        const int discount_energy = base_yield_mul(discount, energy);
        const int threshold = upgrade_arithmetic_shift_three(base_yield_add(
            discount_energy, discount_energy < 0 ? 7 : 0));
        if (base_yield_sub(cost, minerals) <= threshold) {
            if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & faction_bit) == 0 || !IsMultiplayerNet) {
                do_upgrade(faction_id, proto_id_new, proto_id_old);
            } else {
                message_data(0x2411, 0, faction_id, proto_id_new,
                             proto_id_old, 0);
                int result = NetDaemonNet.receive();
                if (result != 0) {
                    do {
                        result = NetDaemonNet.receive();
                    } while (result != 0);
                    return 1;
                }
            }
            return 1;
        }
    } else if (faction_id == ConsoleGlobal.faction_id_) {
        if (cost != 0) {
            StringTemp[0] = '\0';
            strcat(StringTemp, VehPrototypes[proto_id_old].veh_name);
            strcat(StringTemp, " ");
            strcat(StringTemp, "(");
            say_stats_2(StringTemp, proto_id_old);
            if (VehPrototypes[proto_id_old].ability_flags != 0) {
                strcat(StringTemp, " ");
                for (int i = 0; i < MaxAbilityNum; ++i) {
                    if (has_abil(proto_id_old, 1 << (i & 0x1f)) != 0) {
                        strcat(StringTemp, StringTable.get(static_cast<int>(
                            reinterpret_cast<uintptr_t>(Ability[i].abbreviation))));
                    }
                }
            }
            strcat(StringTemp, ")");
            parse_says(0, StringTemp, -1, -1);

            StringTemp[0] = '\0';
            strcat(StringTemp, VehPrototypes[proto_id_new].veh_name);
            strcat(StringTemp, " ");
            strcat(StringTemp, "(");
            say_stats_2(StringTemp, proto_id_new);
            if (VehPrototypes[proto_id_new].ability_flags != 0) {
                strcat(StringTemp, " ");
                for (int i = 0; i < MaxAbilityNum; ++i) {
                    if (has_abil(proto_id_new, 1 << (i & 0x1f)) != 0) {
                        strcat(StringTemp, StringTable.get(static_cast<int>(
                            reinterpret_cast<uintptr_t>(Ability[i].abbreviation))));
                    }
                }
            }
            strcat(StringTemp, ")");
            parse_says(1, StringTemp, -1, -1);

            parse_num(0, cost);
            parse_num(1, PlayersData[faction_id].energy_reserves);
            parse_num(2, PlayersData[faction_id].proto_id_active[proto_id_old]);
            const bool flag_four_clear =
                (VehPrototypes[proto_id_new].flags & 4) == 0;
            const int energy = PlayersData[faction_id].energy_reserves;
            if (base_yield_sub(cost, energy) != 0 && energy <= cost) {
                if ((Players[faction_id].rule_flags & RFLAG_ALIEN) == 0) {
                    if (set_flag != 0 && (GameDataStateStorage.fields_.game_warnings_ & 4) == 0) return 0;
                    const int triad = Chassis[VehPrototypes[proto_id_new].chassis_id].triad;
                    const char *picture = triad == 2 ? "air_sm.pcx"
                        : triad == 1 ? "navun_sm.pcx" : "unit_sm.pcx";
                    popp(ScriptPopupFile,
                         flag_four_clear ? "UPGRADEPROTOCOST2" : "UPGRADEPROTOCOST",
                         0, picture, 0);
                    return 0;
                }
                if (set_flag != 0 && (GameDataStateStorage.fields_.game_warnings_ & 4) == 0) return 0;
                const int triad = Chassis[VehPrototypes[proto_id_new].chassis_id].triad;
                const char *picture = triad == 2 ? "alair_sm.pcx"
                    : triad == 1 ? "navun_sm.pcx" : "alunit_sm.pcx";
                popp(ScriptPopupFile,
                     flag_four_clear ? "UPGRADEPROTOCOST2" : "UPGRADEPROTOCOST",
                     0, picture, 0);
                return 0;
            }

            if (UpgradePromptAnswered == 0) {
                const int triad = Chassis[VehPrototypes[proto_id_new].chassis_id].triad;
                const bool alien = (Players[faction_id].rule_flags & RFLAG_ALIEN) != 0;
                const char *picture = alien
                    ? (triad == 2 ? "alair_sm.pcx"
                        : triad == 1 ? "navun_sm.pcx" : "alunit_sm.pcx")
                    : (triad == 2 ? "air_sm.pcx"
                        : triad == 1 ? "navun_sm.pcx" : "unit_sm.pcx");
                const char *label = flag_four_clear
                    ? "UPGRADEPROTO2" : "UPGRADEPROTO";
                const int result = popp(ScriptPopupFile, label, 0, picture, 0);
                if (result == 0) return 0;
                if (result > 1) UpgradePromptAnswered = 1;
            }
        }
        if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & faction_bit) == 0 || !IsMultiplayerNet) {
            do_upgrade(faction_id, proto_id_new, proto_id_old);
        } else {
            message_data(0x2411, 0, faction_id, proto_id_new,
                         proto_id_old, 0);
            int result = NetDaemonNet.receive();
            if (result != 0) {
                do {
                    result = NetDaemonNet.receive();
                } while (result != 0);
                return 1;
            }
        }
        return 1;
    }
    return 0;
}

#include "stdafx.h"
#include "base.h"
#include "alphamovie.h"
#include "buffer.h"
#include "console.h"
#include "faction.h"
#include "game.h"
#include "general.h"
#include "interlude.h"
#include "map.h"
#include "mapwin.h"
#include "popup.h"
#include "strings.h"
#include "temp.h"
#include "technology.h"
#include "veh.h"
#include "wave.h"
#include "win.h"
#include "worldwin.h"
#include <string.h>
#include <stdlib.h>

extern int BaseProductionZoomRequested;
extern int BaseProductionUnitCreationDisabled;
extern uint32_t UpgradePromptAnswered;
int __cdecl tech_advance(int faction_id);
void __cdecl mon_secret_project(int faction_id, int project_id);
void __cdecl help_project(int project_id);
void __cdecl set_dirty();
int __cdecl generators(int faction_id, int *count_out);

namespace {

int base_yield_add(int left, int right);
int base_yield_mul(int left, int right);

enum ProductionReportResume {
    PRODUCTION_REPORT_RESUME_274F,
    PRODUCTION_REPORT_RESUME_276B,
    PRODUCTION_REPORT_RESUME_2752
};

int production_abs(int value) {
    return value < 0 ? -value : value;
}

int production_project(int item) {
    return item <= -FacilitySPStart ? -item - FacilitySPStart : -1;
}

void production_set_project_owner(int project_id, int base_id) {
    switch (project_id) {
    case 0: GameDataStateStorage.fields_.secret_project_.human_genome_project() = base_id; break;
    case 1: GameDataStateStorage.fields_.secret_project_.command_nexus() = base_id; break;
    case 2: GameDataStateStorage.fields_.secret_project_.weather_paradigm() = base_id; break;
    case 3: GameDataStateStorage.fields_.secret_project_.merchant_exchange() = base_id; break;
    case 4: GameDataStateStorage.fields_.secret_project_.empath_guild() = base_id; break;
    case 5: GameDataStateStorage.fields_.secret_project_.citizens_defense_force() = base_id; break;
    case 6: GameDataStateStorage.fields_.secret_project_.virtual_world() = base_id; break;
    case 7: GameDataStateStorage.fields_.secret_project_.planetary_transit_system() = base_id; break;
    case 8: GameDataStateStorage.fields_.secret_project_.xenoempathy_dome() = base_id; break;
    case 9: GameDataStateStorage.fields_.secret_project_.neural_amplifier() = base_id; break;
    case 10: GameDataStateStorage.fields_.secret_project_.maritime_control_center() = base_id; break;
    case 11: GameDataStateStorage.fields_.secret_project_.planetary_datalinks() = base_id; break;
    case 12: GameDataStateStorage.fields_.secret_project_.supercollider() = base_id; break;
    case 13: GameDataStateStorage.fields_.secret_project_.ascetic_virtues() = base_id; break;
    case 14: GameDataStateStorage.fields_.secret_project_.longevity_vaccine() = base_id; break;
    case 15: GameDataStateStorage.fields_.secret_project_.hunter_seeker_algorithm() = base_id; break;
    case 16: GameDataStateStorage.fields_.secret_project_.pholus_mutagen() = base_id; break;
    case 17: GameDataStateStorage.fields_.secret_project_.cyborg_factory() = base_id; break;
    case 18: GameDataStateStorage.fields_.secret_project_.theory_of_everything() = base_id; break;
    case 19: GameDataStateStorage.fields_.secret_project_.dream_twister() = base_id; break;
    case 20: GameDataStateStorage.fields_.secret_project_.universal_translator() = base_id; break;
    case 21: GameDataStateStorage.fields_.secret_project_.network_backbone() = base_id; break;
    case 22: GameDataStateStorage.fields_.secret_project_.nano_factory() = base_id; break;
    case 23: GameDataStateStorage.fields_.secret_project_.living_refinery() = base_id; break;
    case 24: GameDataStateStorage.fields_.secret_project_.cloning_vats() = base_id; break;
    case 25: GameDataStateStorage.fields_.secret_project_.self_aware_colony() = base_id; break;
    case 26: GameDataStateStorage.fields_.secret_project_.clinical_immortality() = base_id; break;
    case 27: GameDataStateStorage.fields_.secret_project_.space_elevator() = base_id; break;
    case 28: GameDataStateStorage.fields_.secret_project_.singularity_inductor() = base_id; break;
    case 29: GameDataStateStorage.fields_.secret_project_.bulk_matter_transmitter() = base_id; break;
    case 30: GameDataStateStorage.fields_.secret_project_.telepathic_matrix() = base_id; break;
    case 31: GameDataStateStorage.fields_.secret_project_.voice_of_planet() = base_id; break;
    case 32: GameDataStateStorage.fields_.secret_project_.ascent_to_transcendence() = base_id; break;
    case 33: GameDataStateStorage.fields_.secret_project_.manifold_harmonics() = base_id; break;
    case 34: GameDataStateStorage.fields_.secret_project_.nethack_terminus() = base_id; break;
    case 35: GameDataStateStorage.fields_.secret_project_.cloudbase_academy() = base_id; break;
    case 36: GameDataStateStorage.fields_.secret_project_.planetary_energy_grid() = base_id; break;
    case 37: GameDataStateStorage.fields_.secret_project_.user_defined_38() = base_id; break;
    case 38: GameDataStateStorage.fields_.secret_project_.user_defined_39() = base_id; break;
    case 39: GameDataStateStorage.fields_.secret_project_.user_defined_40() = base_id; break;
    case 40: GameDataStateStorage.fields_.secret_project_.user_defined_41() = base_id; break;
    case 41: GameDataStateStorage.fields_.secret_project_.user_defined_42() = base_id; break;
    case 42: GameDataStateStorage.fields_.secret_project_.user_defined_43() = base_id; break;
    case 43: GameDataStateStorage.fields_.secret_project_.user_defined_44() = base_id; break;
    case 44: GameDataStateStorage.fields_.secret_project_.user_defined_45() = base_id; break;
    case 45: GameDataStateStorage.fields_.secret_project_.user_defined_46() = base_id; break;
    case 46: GameDataStateStorage.fields_.secret_project_.user_defined_47() = base_id; break;
    case 47: GameDataStateStorage.fields_.secret_project_.user_defined_48() = base_id; break;
    case 48: GameDataStateStorage.fields_.secret_project_.user_defined_49() = base_id; break;
    case 49: GameDataStateStorage.fields_.secret_project_.user_defined_50() = base_id; break;
    case 50: GameDataStateStorage.fields_.secret_project_.user_defined_51() = base_id; break;
    case 51: GameDataStateStorage.fields_.secret_project_.user_defined_52() = base_id; break;
    case 52: GameDataStateStorage.fields_.secret_project_.user_defined_53() = base_id; break;
    case 53: GameDataStateStorage.fields_.secret_project_.user_defined_54() = base_id; break;
    case 54: GameDataStateStorage.fields_.secret_project_.user_defined_55() = base_id; break;
    case 55: GameDataStateStorage.fields_.secret_project_.user_defined_56() = base_id; break;
    case 56: GameDataStateStorage.fields_.secret_project_.user_defined_57() = base_id; break;
    case 57: GameDataStateStorage.fields_.secret_project_.user_defined_58() = base_id; break;
    case 58: GameDataStateStorage.fields_.secret_project_.user_defined_59() = base_id; break;
    case 59: GameDataStateStorage.fields_.secret_project_.user_defined_60() = base_id; break;
    case 60: GameDataStateStorage.fields_.secret_project_.user_defined_61() = base_id; break;
    case 61: GameDataStateStorage.fields_.secret_project_.user_defined_62() = base_id; break;
    case 62: GameDataStateStorage.fields_.secret_project_.user_defined_63() = base_id; break;
    case 63: GameDataStateStorage.fields_.secret_project_.user_defined_64() = base_id; break;
    }
}

bool production_faction_human(int faction_id) {
    return (static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & (1u << faction_id)) != 0;
}

bool production_facility_built(int base_id, int facility_id) {
    return has_fac_built(facility_id, base_id);
}

int production_wrapped_subtract(int left, int right) {
    const uint32_t bits = static_cast<uint32_t>(left)
                        - static_cast<uint32_t>(right);
    int result;
    memcpy(&result, &bits, sizeof(result));
    return result;
}

int production_wrapped_add(int left, int right) {
    const uint32_t bits = static_cast<uint32_t>(left)
                        + static_cast<uint32_t>(right);
    int result;
    memcpy(&result, &bits, sizeof(result));
    return result;
}

int production_wrapped_multiply(int left, int right) {
    const uint32_t bits = static_cast<uint32_t>(left)
                        * static_cast<uint32_t>(right);
    int result;
    memcpy(&result, &bits, sizeof(result));
    return result;
}

bool production_colony_abandon_prompt(int owner, int prototype_id) {
    Base &base = *BaseCurrent;
    const bool human = production_faction_human(owner);
    if (!human) {
        if (GameDataStateStorage.fields_.diff_level_current_ > DLVL_LIBRARIAN) {
            base.nutrients_accumulated = production_wrapped_add(
                base.nutrients_accumulated, GameDataStateStorage.fields_.diff_level_current_ - DLVL_TALENT);
        }
        return false;
    }

    const uint32_t governor = base.governor;
    if (((governor & GOV_MANAGES_PRODUCTION) != 0
         && (governor & GOV_ACTIVE) != 0)
        || ((GameDataStateStorage.fields_.game_state_ & 1u) != 0 && (GameDataStateStorage.fields_.game_state_ & 8u) == 0)) {
        return false;
    }

    parse_says(0, base.name_string, -1, -1);
    parse_says(1, VehPrototypes[prototype_id].veh_name, -1, -1);
    StringTemp[0] = '\0';
    strcat(StringTemp, "ABANDONBASE");
    if (base.nutrient_surplus > 0) {
        strcat(StringTemp, "1");
        const int faction_cost = cost_factor(base.faction_id_current, 0,
                                             BaseIDCurrentSelected);
        const int population_cost = production_wrapped_multiply(
            faction_cost, base.population_size + 1);
        const int remaining_cost = production_wrapped_subtract(
            population_cost, base.nutrients_accumulated);
        int turns = remaining_cost / base.nutrient_surplus;
        if (turns < 0) turns = 0;
        if (turns > 9999) turns = 9999;
        parse_num(0, turns);
    }

    const char *picture = (Players[owner].rule_flags & RFLAG_ALIEN)
                        ? "alopdir.pcx" : "talent_sm.pcx";
    const int result = popp(ClimatePopupTitle[0], "ABANDONBASE", 0,
                            picture, 0);
    if (IsMultiplayerNet || result < 2) {
        if (result != 0) BaseProductionZoomRequested = 1;
        return false;
    }

    return true;
}

void production_spend(Base &base, int owner, int cost) {
    base.minerals_accumulated = production_wrapped_subtract(
        base.minerals_accumulated, cost);
    const int exemption = Rules.retool_exemption;
    const int cap = exemption < base.mineral_surplus
                  ? exemption : base.mineral_surplus;
    const unsigned char owner_bit = static_cast<unsigned char>(1u << owner);
    if ((static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & owner_bit) != 0
        && base.minerals_accumulated > cap) {
        base.minerals_accumulated = cap < 0 ? 0 : cap;
    }
    base.mineral_surplus_final = base.minerals_accumulated;
}

void production_refresh_base_if_needed(int selected_base_id) {
    if (BaseComputedID != selected_base_id) {
        base_support();
        base_yield();
        base_nutrient();
        base_minerals();
        base_energy();
    }
}

bool production_reveal_faction(int faction_id) {
    PlayerData &player_data = PlayersData[faction_id];
    if ((player_data.flags & PFLAG_MAP_REVEALED) != 0) return false;

    const uint8_t faction_mask = static_cast<uint8_t>(1u << faction_id);
    for (unsigned int tile_index = 0; tile_index < MapArea; ++tile_index) {
        Map &tile = MapTiles[tile_index];
        tile.visibility |= faction_mask;
        if (faction_id != 0) {
            tile.bit_visible[faction_id - 1] = tile.bit;
        }
    }
    for (int base_index = 0; base_index < GameDataStateStorage.fields_.base_current_count_; ++base_index) {
        Base &base = Bases[base_index];
        base.visibility |= faction_mask;
        base.faction_pop_size_intel[faction_id] =
            static_cast<uint8_t>(base.population_size);
    }
    player_data.flags |= PFLAG_MAP_REVEALED;
    return true;
}

void production_clear_artifact_orders() {
    BaseCurrent->state &=
        ~(BSTATE_ARTIFACT_LINKED | BSTATE_ARTIFACT_ALREADY_LINKED);
    int vehicle_id = stack_fix(veh_at(BaseCurrent->x, BaseCurrent->y));
    while (vehicle_id >= 0) {
        Veh &vehicle = Vehs[vehicle_id];
        if (VehPrototypes[vehicle.proto_id].plan == PLAN_ALIEN_ARTIFACT) {
            vehicle.order = ORDER_NONE;
        }
        vehicle_id = vehicle.next_veh_id_stack;
    }
    draw_tile(BaseCurrent->x, BaseCurrent->y, 2);
}

void production_voice_of_planet_spread() {
    int faction_already_handled[MaxPlayerNum];
    for (int faction_index = 0; faction_index < MaxPlayerNum;
         ++faction_index) {
        faction_already_handled[faction_index] = !is_alive(faction_index);
    }

    for (int base_index = 0; base_index < GameDataStateStorage.fields_.base_current_count_; ++base_index) {
        const Base &base = Bases[base_index];
        const int owner = base.faction_id_current;
        if (faction_already_handled[owner]) continue;

        const Map *base_tile = map_loc(base.x, base.y);
        if ((base_tile->climate & 0xe0) < 0x60) continue;
        faction_already_handled[owner] = 1;

        int center_x = 0;
        int center_y = 0;
        bool found_center = false;
        for (int offset_attempt = 0; offset_attempt < 1000;
             ++offset_attempt) {
            const int offset_index = rand() % 0x120;
            center_x = xrange(base.x + RadiusOffsetX[offset_index + 1]);
            center_y = base.y + RadiusOffsetY[offset_index + 1];
            if (center_y < 0
                || center_y >= static_cast<int>(MapLatitudeBounds)) {
                continue;
            }
            const Map *center_tile = map_loc(center_x, center_y);
            if ((center_tile->bit & BIT_BASE_IN_TILE) != 0
                && (center_tile->climate & 0x0f) < 8) {
                continue;
            }
            if (whose_territory(owner, center_x, center_y, 0, 0) != owner) {
                continue;
            }
            found_center = true;
            break;
        }
        if (!found_center) continue;

        if (owner == ConsoleGlobal.faction_id_) {
            ConsoleGlobal.focus(center_x, center_y,
                                ConsoleGlobal.faction_id_);
        }

        for (int point_index = 0; point_index < 9; ++point_index) {
            const int x_coord = xrange(center_x + RadiusOffsetX[point_index]);
            const int y_coord = center_y + RadiusOffsetY[point_index];
            if (y_coord < 0
                || y_coord >= static_cast<int>(MapLatitudeBounds)) {
                continue;
            }
            const Map *tile = map_loc(x_coord, y_coord);
            if ((tile->bit & BIT_BASE_IN_TILE) != 0
                && (tile->climate & 0x0f) < 8) {
                continue;
            }
            if (whose_territory(owner, x_coord, y_coord, 0, 0) != owner) {
                continue;
            }
            bit_set_call(x_coord, y_coord, BIT_FUNGUS, TRUE);
            bit_set_call(x_coord, y_coord, MapTileFlagClearMask, FALSE);
            synch_bit(x_coord, y_coord, ConsoleGlobal.faction_id_);
            draw_tile(x_coord, y_coord, 2);
        }
    }
}

int production_vehicle_morale(int selected_base_id, int owner,
                              int prototype_id, bool new_prototype) {
    const VehPrototype &prototype = VehPrototypes[prototype_id];
    int morale;
    if ((prototype_id == 8 || prototype_id < MaxVehProtoNum)
        && Weapon[prototype.weapon_id].offense_rating < 0) {
        morale = 0;
        if (has_project(SP_XENOEMPATYH_DOME, owner)) ++morale;
        if (has_project(SP_PHOLUS_MUTAGEN, owner)) ++morale;
        if (has_project(SP_VOICE_OF_PLANET, owner)) ++morale;
        if (has_fac(FAC_CENTAURI_PRESERVE, selected_base_id, 0)) ++morale;
        if (has_fac(FAC_TEMPLE_OF_PLANET, selected_base_id, 0)) ++morale;
        if (has_fac(FAC_BIOLOGY_LAB, selected_base_id, 0)) ++morale;
        if (has_fac(FAC_BIOENHANCEMENT_CENTER, selected_base_id, 0)
            || has_project(SP_CYBORG_FACTORY, owner)) {
            ++morale;
        }
    } else {
        morale = morale_mod(selected_base_id, owner,
                            Chassis[prototype.chassis_id].triad);
        if (prototype.weapon_id == 21) {
            if (has_fac(FAC_COVERT_OPS_CENTER, selected_base_id, 0)) morale += 2;
            if (has_project(SP_NETHACK_TERMINUS, owner)) ++morale;
        }
        if (has_abil(prototype_id, ABL_TRAINED)) ++morale;
        if (new_prototype) ++morale;
    }
    ++morale;
    if (morale < MORALE_VERY_GREEN) morale = MORALE_VERY_GREEN;
    if (morale > MORALE_ELITE) morale = MORALE_ELITE;
    return morale;
}

void production_vehicle_notice(int selected_base_id, int owner,
                              int prototype_id, bool new_prototype,
                              int queue_result) {
    Base &base = Bases[selected_base_id];
    const bool governor_full_auto =
        (base.governor & 0x80000001u) == 0x80000001u;
    bool route_to_warning = queue_result == 0 && !governor_full_auto;
    if (!route_to_warning) {
        route_to_warning = new_prototype
            ? (GameDataStateStorage.fields_.game_warnings_ & 4u) != 0
            : (GameDataStateStorage.fields_.game_warnings_ & 0x20000000u) != 0;
    }

    int popup_flags = 0;
    if (route_to_warning && new_prototype && (GameDataStateStorage.fields_.game_warnings_ & 4u) != 0) {
        const int faction_cost = cost_factor(owner, RSC_MINERALS, -1);
        const int unit_cost = veh_cost(prototype_id, selected_base_id, 0);
        parse_num(0, production_wrapped_multiply(faction_cost, unit_cost));
        StringTemp[0] = '\0';
        strcat(StringTemp, "PRODUCEPROTO");
        popup_flags = 4;
    } else if (route_to_warning && !new_prototype) {
        const VehPrototype &prototype = VehPrototypes[prototype_id];
        StringTemp[0] = '\0';
        strcat(StringTemp, "PRODUCE");
        popup_flags = prototype.plan < 8 ? 0x400 : 2;
    } else {
        StringTemp[0] = '\0';
        strcat(StringTemp, "PRODUCE");
        if (queue_result != 0) {
            parse_num(1, base.queue_size + 1);
            strcat(StringTemp, "Q");
        } else if (governor_full_auto) {
            strcat(StringTemp, "G");
        }
    }

    const VehPrototype &prototype = VehPrototypes[prototype_id];
    const bool alien = (Players[owner].rule_flags & RFLAG_ALIEN) != 0;
    const int triad = Chassis[prototype.chassis_id].triad;
    const bool native_life = prototype_id == 15
        || Weapon[prototype.weapon_id].offense_rating < 0;
    const char *picture;
    if (prototype_id >= 64 || native_life) {
        if (triad == TRIAD_AIR) {
            picture = "chiron_sm.pcx";
        } else if (triad == TRIAD_SEA && prototype_id == 14) {
            picture = "sealrk_sm.pcx";
        } else if (triad == TRIAD_SEA) {
            picture = "isle_sm.pcx";
        } else {
            picture = "native_sm.pcx";
        }
    } else if (triad == TRIAD_AIR) {
        picture = alien ? "alair_sm.pcx" : "air_sm.pcx";
    } else if (triad == TRIAD_SEA) {
        picture = "navun_sm.pcx";
    } else {
        picture = alien ? "alunit_sm.pcx" : "unit_sm.pcx";
    }
    popb(StringTemp, popup_flags, -1, picture, 0);
}

int production_report_signed_bits(uint32_t bits) {
    int value;
    memcpy(&value, &bits, sizeof(value));
    return value;
}

ProductionReportResume production_report_stage(
    Popup &popup, int owner, int facility_id, int project_id,
    int queue_result, bool orbital_report) {
    if (owner != ConsoleGlobal.faction_id_ && facility_id < 40) {
        return PRODUCTION_REPORT_RESUME_274F;
    }

    parse_say(1, reinterpret_cast<int>(Facility[facility_id].name), -1, -1);
    parse_says(2, Players[owner].adj_name_faction, -1, -1);
    if (owner != ConsoleGlobal.faction_id_) {
        return PRODUCTION_REPORT_RESUME_276B;
    }

    if (facility_id == 40) {
        interlude(0x26, 0, 1, 0);
        int under_size_generator_count = 0;
        const int active_generator_count =
            generators(owner, &under_size_generator_count);
        parse_num(0, active_generator_count);
        parse_num(1, under_size_generator_count);
        parse_num(2, production_report_signed_bits(
            Rules.subspace_generators_needed));
        parse_num(3, production_report_signed_bits(
            Rules.size_base_subspace_gen));
        parse_says(0, BaseCurrent->name_string, -1, -1);
        popp(ScriptPopupFile, "GENERATORME", 0, "subgen_sm.pcx", 0);
        return PRODUCTION_REPORT_RESUME_2752;
    }

    const bool facility_popup = facility_id < 70
        && !has_fac_announced(static_cast<uint32_t>(owner),
                              static_cast<uint32_t>(facility_id))
        && (GameDataStateStorage.fields_.game_state_ & 1u) == 0
        && (GameDataStateStorage.fields_.game_warnings_ & 1u) != 0;
    if (facility_popup) {
        popup.init(0, 0);
        popup.set_width(500);
        StringTemp[0] = '\0';
        if (project_id >= 0) {
            strcat(StringTemp, "PROJECT");
            say_num(project_id);
        } else {
            strcat(StringTemp, "FAC");
            say_num(facility_id);
            set_fac_announced(static_cast<uint32_t>(owner),
                              static_cast<uint32_t>(facility_id), TRUE);
        }
        popup.start(const_cast<char *>("BLURBS"), StringTemp, -1, 0,
                    0x80, 0);
        popup.string(const_cast<char *>("^"));
        popup.string(const_cast<char *>("^------"));
        popup.string(const_cast<char *>("^"));

        StringTemp[0] = '\0';
        strcat(StringTemp, "voices\\fac");
        if (facility_id >= 65 && facility_id <= 69) {
            say_num(facility_id - 31);
        } else {
            say_num(facility_id);
        }
        if (facility_id >= 34 && facility_id <= 41) say_num(0);
        strcat(StringTemp, ".mp3");

        const uint32_t owner_mask = 1u << owner;
        if ((GameDataStateStorage.fields_.game_more_preferences_ & 0x00400000u) != 0
            && owner == ConsoleGlobal.faction_id_
            && (static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & owner_mask) != 0) {
            g_WAVE_GENERAL.unload();
            g_WAVE_GENERAL.load(StringTemp);
            g_WAVE_GENERAL.play();
        }

        StringTemp[0] = '\0';
        strcat(StringTemp, "PRODUCEX");
        if (queue_result != 0) {
            parse_num(1, production_wrapped_add(
                production_report_signed_bits(BaseCurrent->queue_size), 1));
            strcat(StringTemp, "Q");
        } else if ((BaseCurrent->governor & 1u) != 0
                   && (BaseCurrent->governor & 0x80000000u) != 0) {
            strcat(StringTemp, "G");
        }
        const char *const image =
            (Players[owner].rule_flags & RFLAG_ALIEN) != 0
                ? "alfacblt_sm.pcx" : "facblt_sm.pcx";
        popb(StringTemp, 1, -1, image, 0);

        if ((GameDataStateStorage.fields_.game_more_preferences_ & 0x00400000u) != 0
            && (GameDataStateStorage.fields_.game_warnings_ & 0x00020000u) != 0) {
            g_WAVE_GENERAL.Sound::fade(2000);
        }
        return PRODUCTION_REPORT_RESUME_2752;
    }

    StringTemp[0] = '\0';
    strcat(StringTemp, "PRODUCE");
    if (queue_result != 0) {
        parse_num(1, production_wrapped_add(
            production_report_signed_bits(BaseCurrent->queue_size), 1));
        strcat(StringTemp, "Q");
    } else if ((BaseCurrent->governor & 1u) != 0
               && (BaseCurrent->governor & 0x80000000u) != 0) {
        strcat(StringTemp, "G");
    }

    const char *image;
    if (orbital_report) {
        image = "space_sm.pcx";
    } else if (facility_id >= 70) {
        image = "secproj_sm.pcx";
    } else if ((Players[owner].rule_flags & RFLAG_ALIEN) != 0) {
        image = "alfacblt_sm.pcx";
    } else {
        image = "facblt_sm.pcx";
    }
    const int show_popup = (GameDataStateStorage.fields_.game_warnings_ & 0x00200000u) != 0 ? 1 : 0;
    popb(StringTemp, show_popup, -1, image, 0);
    return PRODUCTION_REPORT_RESUME_2752;
}

// Corrected continuation for native 0x004F276B..0x004F2A59.
static void production_report_foreign_stage_v3(
    Popup &popup, int owner, int facility_id, int project_id,
    bool orbital_report) {
    if (project_id >= 0) {
        parse_says(0, BaseCurrent->name_string, -1, -1);
        popup.init(0, 0);
        popup.set_width(600);
        StringTemp[0] = '\0';
        strcat(StringTemp, "PROJECT");
        say_num(project_id);
        popup.start(const_cast<char *>("BLURBS"), StringTemp, -1, 0,
                    0x80, 0);
        if (WinScreenWidth != 800) {
            popup.string(const_cast<char *>("^"));
        }
        popup.string(const_cast<char *>("^------"));
        popup.string(const_cast<char *>("^"));
        popup.start(PopupStartCaption, "PRODUCE2", -1, 0, 0x80, 0);

        if ((GameDataStateStorage.fields_.game_state_ & 1u) == 0 || (GameDataStateStorage.fields_.game_state_ & 8u) != 0) {
            int width = 0;
            int height = 0;
            if (Buffer::get_pcx_dimensions("secproj_sm.pcx", &width,
                                           &height) == 0
                && popup.owned_picture().init(
                       const_cast<char *>("secproj_sm.pcx"), width,
                       height) == 0) {
                popup.select_owned_picture();
            }
            popup.exec(0, 0);
        }
        return;
    }

    if (facility_id == 40) {
        if ((GameDataStateStorage.fields_.game_state_ & 1u) != 0 && (GameDataStateStorage.fields_.game_state_ & 8u) == 0) return;
        if ((Players[owner].rule_flags & RFLAG_ALIEN) == 0) {
            interlude(0x26, BaseCurrent->name_string, 1, 0);
        }
        int under_size_generator_count = 0;
        const int active_generator_count =
            generators(owner, &under_size_generator_count);
        parse_num(0, active_generator_count);
        parse_num(1, under_size_generator_count);
        parse_num(2, production_report_signed_bits(
                         Rules.subspace_generators_needed));
        parse_num(3, production_report_signed_bits(
                         Rules.size_base_subspace_gen));
        parse_says(0, BaseCurrent->name_string, -1, -1);

        GenderDefault = Players[owner].is_leader_female;
        PluralityDefault = FALSE;
        parse_says(1, Players[owner].title_leader, -1, -1);
        GenderDefault = Players[owner].is_leader_female;
        PluralityDefault = FALSE;
        parse_says(2, Players[owner].name_leader, -1, -1);
        GenderDefault = Players[owner].noun_gender;
        PluralityDefault = Players[owner].is_noun_plural;
        parse_says(3, Players[owner].noun_faction, -1, -1);
        popp(ScriptPopupFile, "GENERATOR", 0, "subgen_sm.pcx", 0);
        return;
    }

    parse_says(0, BaseCurrent->name_string, -1, -1);
    if ((GameDataStateStorage.fields_.game_state_ & 1u) == 0 || (GameDataStateStorage.fields_.game_state_ & 8u) != 0) {
        if (orbital_report) {
            popp(ScriptPopupFile, "PRODUCE3", 0, "space_sm.pcx", 0);
        } else {
            popp(ScriptPopupFile, "PRODUCE2", 0, "secproj_sm.pcx", 0);
        }
    }
}

static ProductionReportResume production_report_stage_v3(
    Popup &popup, int owner, int facility_id, int project_id,
    int queue_result, bool orbital_report) {
    const ProductionReportResume continuation = production_report_stage(
        popup, owner, facility_id, project_id, queue_result, orbital_report);
    if (continuation == PRODUCTION_REPORT_RESUME_276B) {
        production_report_foreign_stage_v3(popup, owner, facility_id,
                                           project_id, orbital_report);
        return PRODUCTION_REPORT_RESUME_2752;
    }
    return continuation;
}

// Native duplicate project/facility notice branches. A return to the caller
// lets the one entry-owned Popup unwind through normal C++ lifetime.
void production_existing_project_notice(int owner, int facility_id,
                                        int project_id) {
    const uint32_t governor = BaseCurrent->governor;
    if (!production_faction_human(owner)
        || ((governor & GOV_MANAGES_PRODUCTION) != 0
            && (governor & GOV_ACTIVE) != 0)) {
        base_reset(BaseIDCurrentSelected, 0);
        return;
    }
    parse_say(1, static_cast<int>(reinterpret_cast<intptr_t>(
        Facility[facility_id].name)), -1, -1);
    StringTemp[0] = '\0';
    strcat(StringTemp, "ALREADYPROJECT");
    const int completed_at = base_project(project_id);
    if (completed_at >= 0) {
        Base &completed = Bases[completed_at];
        parse_says(2, completed.name_string, -1, -1);
        const int completed_owner = completed.faction_id_current;
        PluralityDefault = Players[completed_owner].is_noun_plural;
        GenderDefault = Players[completed_owner].noun_gender;
        parse_says(3, Players[completed_owner].noun_faction, -1, -1);
        strcat(StringTemp, completed_owner == owner ? "1" : "2");
    } else {
        strcat(StringTemp, "0");
    }
    popb(StringTemp, 0x200, -1, "secproj_sm.pcx", 0);
}

void production_existing_facility_notice(int owner, int facility_id) {
    const uint32_t governor = BaseCurrent->governor;
    if (!production_faction_human(owner)
        || ((governor & GOV_MANAGES_PRODUCTION) != 0
            && (governor & GOV_ACTIVE) != 0)) {
        base_reset(BaseIDCurrentSelected, 0);
        return;
    }
    parse_say(1, static_cast<int>(reinterpret_cast<intptr_t>(
        Facility[facility_id].name)), -1, -1);
    const char *const picture =
        (Players[owner].rule_flags & RFLAG_ALIEN) != 0
            ? "alfacblt_sm.pcx" : "facblt_sm.pcx";
    popb("ALREADYFACILITY", 0x200, -1, picture, 0);
}


void production_finish_vehicle(int selected_base_id, int owner,
                               int prototype_id, bool new_prototype) {
    Base &base = Bases[selected_base_id];
    base.state |= BSTATE_PRODUCTION_DONE;
    base.state &= 0x7fffffffu;
    if (!production_faction_human(owner)) {
        base_reset(selected_base_id, 0);
        BaseCurrent = &Bases[selected_base_id];
    } else {
        const int queue_result = base_queue(selected_base_id);
        BaseCurrent = &Bases[selected_base_id];
        production_vehicle_notice(selected_base_id, owner, prototype_id,
                                  new_prototype, queue_result);
    }

    if (new_prototype) mon_prototype_built(owner);
    if (IsMultiplayerNet && production_faction_human(owner)) {
        VehPrototypes[prototype_id].flags |= 0x100u;
    } else {
        UpgradePromptAnswered = 0;
        if (!production_faction_human(owner)
            || (GameDataStateStorage.fields_.game_more_preferences_ & 0x00100000u) != 0) {
            for (int prototype_index = 0; prototype_index < 128;
                 ++prototype_index) {
                const int candidate_id = prototype_index < 64
                    ? prototype_index
                    : prototype_index - 64 + owner * 64;
                const uint16_t flags = VehPrototypes[candidate_id].flags;
                if ((flags & 0x105u) == 0x105u) {
                    upgrade_prototypes(owner, candidate_id);
                }
            }
        }
    }

    const VehPrototype &prototype = VehPrototypes[prototype_id];
    if (prototype_id < 64
        && (prototype_id == 15
            || Weapon[prototype.weapon_id].offense_rating < 0)) {
        mon_native_life_bred(owner);
    }
    const int triad = Chassis[prototype.chassis_id].triad;
    if (triad == TRIAD_SEA) mon_naval_unit_built(owner);
    if (triad == TRIAD_AIR) mon_air_unit_built(owner);
}

void production_late_reconcile(int production_id, int project_id,
                               uint32_t faction_mask, int queue_result) {
    const uint32_t managed_active = GOV_MANAGES_PRODUCTION | GOV_ACTIVE;
    Base *current = BaseCurrent;
    const bool human_owner =
        (static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) &
         static_cast<uint8_t>(faction_mask)) != 0;
    const bool production_managed =
        (current->governor & managed_active) == managed_active;

    if (!human_owner || (production_managed && !queue_result)) {
        base_reset(BaseIDCurrentSelected, 0);
        current = BaseCurrent;
    }

    const bool reloaded_production_managed =
        (current->governor & managed_active) == managed_active;
    const bool human_owner_now =
        (static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) &
         static_cast<uint8_t>(faction_mask)) != 0;
    if (human_owner_now && !reloaded_production_managed &&
        current->queue_production_id[0] == -production_id &&
        current->queue_size == 0) {
        current->queue_production_id[0] = -FAC_STOCKPILE_ENERGY;
        draw_radius(current->x, current->y, 2, 2);
    }

    if (project_id < 0) return;

    const int selected_base_id = BaseIDCurrentSelected;
    for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
        if (base_id == selected_base_id) continue;

        Base &base = Bases[base_id];
        const uint32_t base_faction_mask = 1u << base.faction_id_current;
        const bool base_is_human =
            (static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) &
             static_cast<uint8_t>(base_faction_mask)) != 0;
        if (base_is_human &&
            (base.governor & managed_active) != managed_active) {
            continue;
        }
        if (base.queue_production_id[0] != -production_id) continue;

        const int faction = base.faction_id_current;
        const int base_score = base_yield_add(
            base.minerals_accumulated,
            base_yield_mul(base.mineral_surplus, 10));
        int best_other_base = -1;
        int best_other_score = base_score;
        for (int other_id = 0; other_id < GameDataStateStorage.fields_.base_current_count_; ++other_id) {
            if (other_id == base_id || other_id == selected_base_id) continue;
            Base &other = Bases[other_id];
            if (other.faction_id_current != faction) continue;
            if (other.queue_production_id[0] == -production_id ||
                other.queue_production_id[0] > -FacilitySPStart) {
                continue;
            }

            const int other_score = base_yield_add(
                other.minerals_accumulated,
                base_yield_mul(other.mineral_surplus, 10));
            if (other_score < best_other_score) {
                best_other_score = other_score;
                best_other_base = other_id;
            }
        }

        if (best_other_base < 0) {
            base_reset(base_id, 0);
        } else {
            const int displaced = Bases[base_id].queue_production_id[0];
            Bases[base_id].queue_production_id[0] =
                Bases[best_other_base].queue_production_id[0];
            Bases[best_other_base].queue_production_id[0] = displaced;
        }
    }

    BaseIDCurrentSelected = selected_base_id;
    BaseCurrent = &Bases[selected_base_id];
    if (BaseComputedID != selected_base_id) {
        BaseComputedID = selected_base_id;
        base_support();
        base_yield();
        base_nutrient();
        base_minerals();
        base_energy();
    }
}

// Native 0x4F2F61..0x4F33D2 project-event tail. The caller performs the
// initial project interlude/movie/help actions and Voice of Planet spread.
bool production_project_events_tail(int owner, int project_id) {
    if (project_id == SP_VOICE_OF_PLANET) {
        interlude(13, 0, 1, 0);
        PluralityDefault = 0;
        GenderDefault = Players[owner].is_leader_female;
        parse_says(5, Players[owner].name_leader, -1, -1);
        CharUpperA(ParseStrBuffer[5].str);
        interlude(14, 0, 0, 0);
    }

    mon_secret_project(owner, project_id);
    if (project_id == SP_ASCENT_TO_TRANSCENDENCE) {
        mon_winning_trans(owner, -1);
        if ((GameDataStateStorage.fields_.game_rules_ & RULES_VICTORY_COOPERATIVE) != 0) {
            for (int target = 1; target < MaxPlayerNum; ++target) {
                if (target == owner) continue;
                if ((PlayersData[owner].diplo_treaties[target]
                     & DTREATY_PACT) == 0) {
                    continue;
                }
                const uint32_t recipient_treaties =
                    PlayersData[target].diplo_treaties[owner];
                if ((recipient_treaties & DTREATY_HAVE_SURRENDERED) != 0) {
                    const uint8_t human = factions_status_byte(0);
                    if ((human & static_cast<uint8_t>(1u << target)) == 0) {
                        continue;
                    }
                }
                mon_winning_trans(target, owner);
            }
        }
        if (owner == ConsoleGlobal.faction_id_) {
            interlude(17, 0, 2, 0);
            EndgameExitReason = 1;
        } else {
            EndgameExitReason = 3;
            PluralityDefault = 0;
            GenderDefault = Players[owner].is_leader_female;
            parse_says(5, Players[owner].name_leader, -1, -1);
            const int year_turn = production_wrapped_add(GameDataStateStorage.fields_.turn_current_num_, 100);
            parse_num(2, game_year(year_turn));
            const int century_turn = production_wrapped_add(GameDataStateStorage.fields_.turn_current_num_, 100);
            parse_num(3, century_turn / 100);
            if ((Players[owner].rule_flags & RFLAG_ALIEN) != 0) {
                interlude(27, 0, 4, 0);
            } else {
                interlude(18, 0, 3, 0);
            }
        }
        GameDataStateStorage.fields_.game_state_ |= 1u;
        return true;
    }
    if (project_id == SP_UNIVERSAL_TRANSLATOR) {
        tech_advance(owner);
        tech_advance(owner);
    } else if (project_id == SP_PLANETARY_TRANS_SYS) {
        for (int base_index = 0; base_index < GameDataStateStorage.fields_.base_current_count_; ++base_index) {
            Base &base = Bases[base_index];
            if (base.faction_id_current != owner || base.population_size >= 3) {
                continue;
            }
            base.population_size = 3;
            if (owner == ConsoleGlobal.faction_id_) {
                draw_tile(base.x, base.y, 2);
            }
        }
    }
    return false;
}


} // namespace

// ORIGINAL: 0x004F07E0 ?base_production@@YAXXZ 0x004F07E0-0x004F3FD8;0x0065C8A0-0x0065D0D6
// RECOVERY_BINDING: v1 {"evidence": ["original:0x004F07E0-0x004F3FD8;0x0065C8A0-0x0065D0D6", "git:bb602081:src/unrecovered/004f07e0.cpp", "source:src/base.h", "source:src/faction.h", "source:src/popup.h"], "reason": "Recover the complete colony production operation with real Base, PlayerData, Popup and project lifetimes; helper decomposition preserves redispatch, reporting, unit/facility creation and cleanup paths, including native wrapping arithmetic."}
void __cdecl base_production() {
    Popup popup;

    BaseCurrent->state &= ~BSTATE_PRODUCTION_DONE;
    const int owner = BaseCurrent->faction_id_current;
    const int base_id = BaseIDCurrentSelected;
    PlayerData &player_data = PlayersData[owner];
    const int intake_goal = production_wrapped_add(
        BaseCurrent->mineral_intake_2,
        production_wrapped_multiply(2, BaseCurrent->mineral_surplus));
    if (player_data.unk_28 < intake_goal) player_data.unk_28 = intake_goal;

    // Negative nutrient accumulation takes the owned Popup cleanup return;
    // the native path does not show a starvation message or alter the base.
    if (BaseCurrent->nutrients_accumulated < 0) return;

    if (BaseCurrent->mineral_surplus >= 0
        && BaseCurrent->queue_production_id[0] != -FAC_STOCKPILE_ENERGY
        && (BaseCurrent->state & BSTATE_DRONE_RIOTS_ACTIVE) == 0) {
        BaseCurrent->minerals_accumulated = production_wrapped_add(
            BaseCurrent->minerals_accumulated, BaseCurrent->mineral_surplus);
    }

    const unsigned char owner_bit = static_cast<unsigned char>(1u << owner);
    const bool human = (static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & owner_bit)
                    != 0;
    const uint32_t base_state_before_reset = BaseCurrent->state;
    const uint32_t governor_before_reset = BaseCurrent->governor;
    bool reset_base = true;
    if (human && ((governor_before_reset & 1u) == 0
                  || (governor_before_reset & 0x80000000u) == 0
                  || (base_state_before_reset & 0x80000000u) != 0)) {
        reset_base = false;
    } else if ((base_state_before_reset & 8u) == 0) {
        const Map *base_tile = map_loc(BaseCurrent->x, BaseCurrent->y);
        if ((base_tile->bit & BIT_VEH_IN_TILE) != 0
            && (base_tile->val2 & 0x0f) < MaxPlayerNum) {
            reset_base = false;
        } else if (human) {
            reset_base = false;
        }
    }
    if (reset_base) base_reset(base_id, 0);
    BaseCurrent = &Bases[base_id];
    int production_id = BaseCurrent->queue_production_id[0];
    int completed_project_id = -1;
    int completed_facility_id = -1;
    int completed_production_cost = 0;

process_production_item:
    if (production_id == -FAC_STOCKPILE_ENERGY) {
        // Stockpile reaches the same owned cleanup return without production.
        return;
    }

    if (production_id >= 0) {
        // Vehicle production: the cost, prerequisite, queue accounting,
        // prototype side effects and all shortage branches are translated
        // in this proposal's subsequent section.
        const int faction_id = BaseCurrent->faction_id_current;
        const int cost = production_wrapped_multiply(
            cost_factor(faction_id, RSC_MINERALS, -1),
            veh_cost(production_id, base_id, 0));
        if (BaseCurrent->minerals_accumulated < cost) {
            // Native 0x4F0B74..0x4F0BE8 is only the entry-owned Popup
            // teardown followed by the common return; RAII supplies it.
            return;
        }
        const bool continue_colony =
            VehPrototypes[production_id].plan != PLAN_COLONIZATION
            || BaseCurrent->population_size != 1
            || player_data.diff_level <= DLVL_SPECIALIST
            || production_colony_abandon_prompt(owner, production_id);
        if (!continue_colony) {
            return;
        }
        bool new_prototype = false;
        if (production_id >= 64) {
            const uint16_t old_prototype_flags =
                VehPrototypes[production_id].flags;
            new_prototype = (old_prototype_flags & 4u) == 0;
            VehPrototypes[production_id].flags =
                static_cast<uint16_t>(old_prototype_flags | 4u);
        }
        if (new_prototype) {
            const int saved_selected_base = BaseIDCurrentSelected;
            prune_protos(owner, production_id, 0);
            BaseIDCurrentSelected = saved_selected_base;
            BaseCurrent = &Bases[saved_selected_base];
            production_refresh_base_if_needed(saved_selected_base);
        }
        production_spend(*BaseCurrent, faction_id, cost);
        if (BaseProductionUnitCreationDisabled != 0) {
            return;
        }
        const int vehicle_id = veh_init(production_id, faction_id,
                                        BaseCurrent->x, BaseCurrent->y);
        if (vehicle_id < 0) {
            return;
        }
        Veh &vehicle = Vehs[vehicle_id];
        if (production_faction_human(faction_id)
            && (BaseCurrent->governor
                & (GOV_NEW_VEH_FULLY_AUTO | GOV_ACTIVE))
            == (GOV_NEW_VEH_FULLY_AUTO | GOV_ACTIVE)) {
            vehicle.state |= VSTATE_UNK_200;
            vehicle.order_auto_type = 0;
        }
        if (VehPrototypes[production_id].plan == PLAN_COLONIZATION) {
            if ((BaseCurrent->state & 0x02000000u) != 0
                || Players[faction_id].ai_power == 0
                || player_data.pop_total > 8) {
                BaseCurrent->state |= 0x01000000u;
            }
            BaseCurrent->state |= 0x02000000u;
            BaseCurrent->nutrients_accumulated =
                BaseCurrent->nutrients_accumulated < 0
                ? 0 : BaseCurrent->nutrients_accumulated;
            if (BaseCurrent->population_size > 1
                || (player_data.diff_level > DLVL_SPECIALIST
                    && !is_objective(base_id))) {
                --BaseCurrent->population_size;
            }
            if (BaseCurrent->population_size <= 0) {
                vehicle.home_base_id = -1;
                base_kill(base_id);
                draw_map(1);
                eliminate_player(faction_id, 0);
                return;
            }
        }
        if (production_id == 8 && faction_id == ConsoleGlobal.faction_id_
            && (GameDataStateStorage.fields_.game_interlude_flags_ & 0x10u) == 0) {
            interlude(4, BaseCurrent->name_string, 1, 0);
            vehicle.state |= 0x00800000u;
        }
        vehicle.morale = static_cast<uint8_t>(production_vehicle_morale(
            base_id, faction_id, production_id, new_prototype));
        vehicle.state |= 0x00002000u;
        vehicle.flags |= 0x1000u;
        production_finish_vehicle(base_id, faction_id, production_id,
                                  new_prototype);
        return;
    }

    // Negative queue items are facilities or projects.
    {
        const int facility_id = production_abs(production_id);
        const int project_id = production_project(production_id);
        const int faction_id = BaseCurrent->faction_id_current;
        const int production_cost = production_wrapped_multiply(
            cost_factor(faction_id, RSC_MINERALS, -1),
            Facility[facility_id].cost);
        if (project_id >= 0) {
            if (project_id == SP_ASCENT_TO_TRANSCENDENCE
                && BaseCurrent->minerals_accumulated >= production_cost
                && !ascending(owner)) {
                popb("ASCENTNOTYET", 0x1000, -1, "asctran_sm.pcx", 0);
                return;
            }
            const int existing_project_base = base_project(project_id);
            if (existing_project_base != -1) {
                production_existing_project_notice(owner, facility_id,
                                                   project_id);
                return;
            }
            if (BaseCurrent->minerals_accumulated < production_cost) return;
            completed_project_id = project_id;
            completed_production_cost = production_cost;
        } else if (facility_id < FacilityRepStart) {
            if (production_facility_built(base_id, facility_id)) {
                if (!base_queue(BaseIDCurrentSelected)) {
                    production_existing_facility_notice(owner, facility_id);
                    return;
                }
                BaseCurrent = &Bases[BaseIDCurrentSelected];
                production_id = BaseCurrent->queue_production_id[0];
                if (production_id == -FAC_STOCKPILE_ENERGY) return;
                goto process_production_item;
            }
            if (BaseCurrent->minerals_accumulated < production_cost) return;
            completed_facility_id = facility_id;
            completed_production_cost = production_cost;
        } else {
            // 65..68 are the four orbital facilities; other IDs in this
            // reserved range are not ordinary facility-build completions.
            if (facility_id < FacilityRepStart + 4) {
                if (BaseCurrent->minerals_accumulated < production_cost) {
                    return;
                }
                completed_facility_id = facility_id;
                completed_production_cost = production_cost;
            } else {
                return;
            }
        }
    }

finalize_base_production:
    // Native 0x4F1FA5..0x4F2244 completes the negative queue item, updates its
    // typed owner records, and advances the queue before report construction.
    production_spend(*BaseCurrent, owner, completed_production_cost);
    BaseCurrent->state |= BSTATE_PRODUCTION_DONE;
    BaseCurrent->state &= 0x7fffffffu;

    if (completed_project_id >= 0) {
        production_set_project_owner(completed_project_id, base_id);
    } else if (completed_facility_id >= 0) {
        if (completed_facility_id == FAC_HEADQUARTERS) {
            int offset;
            int mask;
            bitmask(FAC_HEADQUARTERS, &offset, &mask);
            for (int base_index = 0; base_index < GameDataStateStorage.fields_.base_current_count_;
                 ++base_index) {
                Base &other_base = Bases[base_index];
                if (other_base.faction_id_current == owner) {
                    other_base.facilities_built[offset] =
                        static_cast<uint8_t>(
                            other_base.facilities_built[offset] & ~mask);
                }
            }
        }
        if (completed_facility_id >= FacilityRepStart
            && completed_facility_id < FacilityRepStart + 4) {
            ++PlayersData[owner].satellites_[completed_facility_id
                                             - FacilityRepStart];
        } else {
            set_fac(completed_facility_id, base_id, TRUE);
        }
        const bool revealed_map = production_reveal_faction(owner);
        if (revealed_map && owner == ConsoleGlobal.faction_id_
            && (GameDataStateStorage.fields_.game_state_ & 1u) == 0) {
            popp(ScriptPopupFile, "SEEMAP", 0, "space_sm.pcx", 0);
            set_dirty();
            draw_map(1);
            WorldClimateWorldWin.GraphicWin::redraw();
        }
    }
    if (completed_facility_id == 8) {
        production_clear_artifact_orders();
    }

    const int queue_result = base_queue(BaseIDCurrentSelected);
    const int completed_item_id = production_abs(production_id);
    const bool orbital_report = completed_item_id >= 65
                             && completed_item_id < 69;
    const ProductionReportResume report_resume = production_report_stage_v3(
        popup, owner, completed_item_id, completed_project_id,
        queue_result, orbital_report);

    // The native 274F, 276B, and 2752 paths converge before the monitor
    // dispatch at 2752; foreign-owner report paths are not monitor skips.
    (void)report_resume;
    if (orbital_report) {
        mon_first_in_space(owner);
    } else if (completed_item_id < FacilitySPStart) {
        mon_facility_built(owner, BaseCurrent->name_string);
    }

    if (completed_facility_id == FAC_CENTAURI_PRESERVE) {
        mon_built_preserve(owner);
        if (owner == ConsoleGlobal.faction_id_) {
            interlude((GameDataStateStorage.fields_.game_interlude_flags_ & 0x20u) ? 9 : 5, 0, 1, 0);
        }
    }
    if (completed_facility_id == 31 || completed_facility_id == 32
        || completed_facility_id == 13
        || completed_facility_id == 14) {
        if (PlayersData[owner].planet_ecology != 0) {
            PlayersData[owner].planet_ecology = production_wrapped_add(
                PlayersData[owner].planet_ecology, 1);
        }
        PlayersData[owner].flags &= ~0x4000u;
    }

    if (completed_project_id >= 0) {
        if (completed_project_id == SP_VOICE_OF_PLANET) {
            interlude(12, 0, 1, 0);
        }
        if ((GameDataStateStorage.fields_.game_preferences_ & PREF_AV_SECRET_PROJECT_MOVIES) != 0
            && !IsMultiplayerNet) {
            if (owner == ConsoleGlobal.faction_id_) {
                amovie_project(completed_project_id);
            } else if (completed_project_id == SP_ASCENT_TO_TRANSCENDENCE) {
                amovie_project("theyTranscend");
            }
        }
        if (owner == ConsoleGlobal.faction_id_) {
            help_project(completed_project_id);
        }
        if (completed_project_id == SP_VOICE_OF_PLANET) {
            production_voice_of_planet_spread();
        }
        if (production_project_events_tail(owner, completed_project_id)) {
            return;
        }
    }

    production_late_reconcile(production_id, completed_project_id,
                              owner_bit, queue_result);
}

// Production sets this request for the subsequent upkeep zoom. Four-byte
// loader-zero initial storage; the original identity is not a runtime address.
// RECOVERY_OBJECT: v2 0x0090E934 0x0090E938 ?BaseProductionZoomRequested@@3HA 0x004F07E0
int BaseProductionZoomRequested = 0;
// Nonzero skips unit creation before veh_init. The indirect producer remains
// unverified; do not infer a permanently false flag from loader-zero storage.
// RECOVERY_OBJECT: v2 0x0090EA40 0x0090EA44 ?BaseProductionUnitCreationDisabled@@3HA 0x004F07E0
int BaseProductionUnitCreationDisabled = 0;

static int base_upkeep_wrap_signed_bits(uint32_t bits) {
    return bits <= 0x7FFFFFFFu ? static_cast<int>(bits)
        : -1 - static_cast<int>(0xFFFFFFFFu - bits);
}

static int base_upkeep_wrap_add(int left, int right) {
    return base_upkeep_wrap_signed_bits(static_cast<uint32_t>(left) +
                                      static_cast<uint32_t>(right));
}

// ORIGINAL: 0x004F79C0 ?base_upkeep@@YAHH@Z 0x004F79C0-0x004F7FDA
// RECOVERY_BINDING: v1 {"evidence":["original:0x004F79C0-0x004F7FDA:pinned Base upkeep span","original:0x004F7D9E,0x004F7DDC,0x004F7E1D:BaseCurrent is reloaded at the native parser/climate callback boundaries","original:0x004F7BF6-0x004F7C01:actor and partner rule-flags use distinct faction indexes","source:src/base.h:Base, Bases, BaseCurrent, and the canonical Base-upkeep call surface","source:src/faction.h:Player and PlayerData arrays distinguish actor, partner, loans, energy and sanctions"],"reason":"Recover Base upkeep with native callback reloads and independently indexed actor/partner commerce state through the existing Base, Player and PlayerData owners."}
// TRIED: faithful port using the real Base struct (src/base.h offsets, cross-checked field-by-field against the disassembly: state@0x30, minerals_accumulated@0x40, production_id_last@0x44, resource_sqr_radius @0x78, minerals_accumulated_2@0x9C, economy_total@0x100, name_string@ 0x13, assimilation/nerve_staple/faction fields@4-8) plus raw pointer arithmetic for the five 0x59c-stride and 0x20cc-stride per-faction tables (946f58/9474f4/946d34/946d4c/946d50 and 96e35c/96cdbc/96d1d4/ 96cc00/96caa0/96cce8/96eb6c). Compiles and matches through instr #4 (prologue); diverges at #5 on register-save ordering, driven by local variable count/order. Not chased further; 1562-byte function with many distinct call sites.
int __cdecl base_upkeep(int base_id) {
    // Native event/log text remains inline: "al_col_sm.pcx" 0x689EA4..0x689EB2, "RENAMEBASE" 0x689EB4..0x689EBF and 0x689ED0..0x689EDB, "colfoun_sm.pcx" 0x689EC0..0x689ECF, "UPKEEP zoom-to-base" 0x689EDC..0x689EF0, and "FINISHED zoom-to-base" 0x689EF0..0x689F06.

    BaseIDCurrentSelected = base_id;
    BaseComputedID = base_id;
    BaseCurrent = &Bases[base_id];
    const int faction_id = BaseCurrent->faction_id_current;
    BaseProductionZoomRequested = 0;
    UnkGlobal0093A934 = 0;

    BaseCurrent->state &= 0xDFFFFDFFu;
    base_support();
    base_yield();
    base_nutrient();
    base_minerals();
    base_energy();
    base_production();
    base_hurry();

    BaseCurrent->minerals_accumulated_2 = BaseCurrent->minerals_accumulated;
    BaseCurrent->production_id_last = BaseCurrent->queue_production_id[0];
    if (base_growth() != 0) return 1;
    if (GameDataStateStorage.fields_.turn_current_num_ > 1) base_check_support();

    BaseUpkeepStage = 1;
    BaseComputedID = BaseIDCurrentSelected;
    base_support();
    base_yield();
    base_nutrient();
    base_minerals();
    base_energy();

    Base &stage_one_base = *BaseCurrent;
    const int tile_index = static_cast<int>(stage_one_base.y) *
        static_cast<int>(MapLongitude) + (static_cast<int>(stage_one_base.x) >> 1);
    const uint8_t resource_class = PlayersData[faction_id].unk_83[
        MapTiles[tile_index].region];
    const int tile_weight = resource_class == 7 &&
        PlayersData[faction_id].soc_effect_base.values_[SocialEffect::TALENT] < 1
        ? 4 : (resource_class != 2) + 1;
    PlayersData[faction_id].unk_47 = base_upkeep_wrap_add(
        PlayersData[faction_id].unk_47,
        bit_count(stage_one_base.resource_sqr_radius) * tile_weight);

    BaseUpkeepStage = 2;
    if (GameDataStateStorage.fields_.turn_current_num_ > 1) {
        base_drones();
        base_doctors();
        if (BaseCurrent->faction_id_current != faction_id) return 1;
        base_energy_costs();
        if (GameDataStateStorage.fields_.turn_current_num_ > 5 || IsMultiplayerNet != 0 ||
            (GameDataStateStorage.fields_.game_preferences_ & 0x20u) == 0) {
            base_research();
        }
    }

    BaseIDCurrentSelected = base_id;
    BaseCurrent = &Bases[base_id];
    if (BaseComputedID != base_id) {
        BaseComputedID = base_id;
        base_support();
        base_yield();
        base_nutrient();
        base_minerals();
        base_energy();
    }
    base_ecology();
    PlayersData[faction_id].energy_reserves = base_upkeep_wrap_add(
        PlayersData[faction_id].energy_reserves, BaseCurrent->economy_total);

    // The first flag read is indexed by ESI=faction_id (the actor). The
    // independent EAX pointer walk begins at Players[1].rule_flags and is
    // partner-indexed by the original loop. Keep both role tests separate.
    for (int other_faction = 1; other_faction < MaxPlayerNum; ++other_faction) {
        const int actor_rule_flags = Players[faction_id].rule_flags;
        const int partner_rule_flags = Players[other_faction].rule_flags;
        if (other_faction == faction_id ||
            (actor_rule_flags & 0x80) != 0 ||
            (partner_rule_flags & 0x80) != 0) {
            continue;
        }
        int commerce = 0;
        if (PlayersData[faction_id].sanction_turns == 0 &&
            PlayersData[other_faction].sanction_turns == 0) {
            commerce = BaseCommerceReceipts[other_faction - 1];
        }
        PlayersData[faction_id].turn_commerce_income = base_upkeep_wrap_add(
            PlayersData[faction_id].turn_commerce_income, commerce);
    }

    if ((Players[faction_id].rule_flags & 0x80) != 0) {
        int count = 0;
        for (int facility_id = 0; facility_id < 0x46; ++facility_id) {
            if ((BaseCurrent->facilities_built[facility_id >> 3] &
                 static_cast<uint8_t>(1u << (facility_id & 7))) != 0) {
                ++count;
            }
        }
        for (int project = 0; project < MaxSecretProjectNum; ++project) {
            if (base_project(project) == base_id) count += 5;
        }
        int alien_commerce = count / 2;
        if (PlayersData[faction_id].sanction_turns != 0) alien_commerce = 0;
        PlayersData[faction_id].turn_commerce_income = base_upkeep_wrap_add(
            PlayersData[faction_id].turn_commerce_income, alien_commerce);
    }

    if (GameDataStateStorage.fields_.turn_current_num_ > 1) base_maint();
    base_terraform(0);
    if ((BaseCurrent->state & 0x10000u) != 0 && (rand() % 3) == 0 &&
        (GameDataStateStorage.fields_.game_state_ & STATE_UNK_1) == 0) {
        // Keep each native BaseCurrent reload across its callback boundary.
        // 0x4F7D69 supplies focus coordinates; 0x4F7D86 reloads for draw_tiles.
        const int focus_x = BaseCurrent->x;
        const int focus_y = BaseCurrent->y;
        const Base &owner_before_focus = Bases[BaseIDCurrentSelected];
        const uint32_t local_faction_bit = 1u << (ConsoleGlobal.faction_id_ & 0x1F);
        if (owner_before_focus.faction_id_current == ConsoleGlobal.faction_id_ ||
            (owner_before_focus.visibility & local_faction_bit) != 0) {
            if (ConsoleGlobal.focus(focus_x, focus_y,
                                    ConsoleGlobal.faction_id_) == 0) {
                draw_tiles(BaseCurrent->x, BaseCurrent->y, 2);
            }
        }
        // 0x4F7D9E reloads after focus/draw callbacks before this phrase.
        parse_says(1, BaseCurrent->name_string, -1, -1);
        GenderDefault = Players[faction_id].noun_gender;
        PluralityDefault = Players[faction_id].is_noun_plural;
        parse_says(0, Players[faction_id].noun_faction, -1, -1);
        // 0x4F7DDC reloads after parse_says(0); use that same live Base for
        // the climate tile and name_base's name pointer.
        Base &climate_base = *BaseCurrent;
        const int event_tile = static_cast<int>(climate_base.y) *
            static_cast<int>(MapLongitude) + (static_cast<int>(climate_base.x) >> 1);
        const int final_name = (MapTiles[event_tile].climate & 0xE0) < 0x60;
        name_base(faction_id, climate_base.name_string, 1, final_name);
        // 0x4F7E1D reloads after name_base before parsing the renamed phrase.
        parse_says(2, BaseCurrent->name_string, -1, -1);
        const int local_faction = ConsoleGlobal.faction_id_;
        const Base &visible_base = Bases[BaseIDCurrentSelected];
        const uint32_t visible_faction_bit = 1u << (local_faction & 0x1F);
        if (visible_base.faction_id_current == local_faction ||
            (visible_base.visibility & visible_faction_bit) != 0) {
            draw_map(1);
            if ((Players[faction_id].rule_flags & 0x80) == 0) {
                popp("SCRIPT", "RENAMEBASE", 0,
                     "colfoun_sm.pcx", 0);
            } else {
                popp("SCRIPT", "RENAMEBASE", 0,
                     "al_col_sm.pcx", 0);
            }
        }
        BaseCurrent->state &= ~0x10000u;
        BaseCurrent->state |= 0x80u;
    }

    BaseComputedID = -1;
    if (BaseIDCurrentSelected == TutWinBaseWindow.focused_base_id()) {
        TutWinBaseWindow.on_redraw();
    }
    if (BaseProductionZoomRequested != 0) {
        const uint32_t preferences = GameDataStateStorage.fields_.game_preferences_;
        if ((preferences & 1u) == 0 || (preferences & 8u) != 0) {
            log_say("UPKEEP zoom-to-base", base_id, 0, 0);
            TutWinBaseWindow.zoom(base_id, 1);
            log_say("FINISHED zoom-to-base", base_id, 0, 0);
        }
    }

    BaseUpkeepStage = 0;
    if (BaseCurrent->assimilation_turns_left != 0) {
        --BaseCurrent->assimilation_turns_left;
        if (BaseCurrent->faction_id_former == BaseCurrent->faction_id_current &&
            BaseCurrent->assimilation_turns_left != 0) {
            --BaseCurrent->assimilation_turns_left;
        }
        if (BaseCurrent->assimilation_turns_left == 0) {
            BaseCurrent->faction_id_former = BaseCurrent->faction_id_current;
        }
    }
    if (BaseCurrent->nerve_staple_turns_left != 0) --BaseCurrent->nerve_staple_turns_left;
    BaseCurrent->state &= 0xFFDFFFF7u;
    if (((base_id + GameDataStateStorage.fields_.turn_current_num_) & 7) == 0) BaseCurrent->state &= 0xFFEFFFFFu;
    return 0;
}
// Native 0x009A658C is SecretProject.telepathic_matrix (SP_TELEPATHIC_MATRIX).
// The native call enters the mapped drone_riot body below.

// ORIGINAL: 0x004F5CE0 ?base_drones@@YAXXZ 0x004F5CE0-0x004F5F25
// RECOVERY_BINDING: v1 {"evidence":["original:0x004F5CE0-0x004F5F25:pinned base drone/riot lifecycle body","original:0x009A658C:SecretProject index 30 owns the telepathic-matrix faction id","source:src/alpha.h:SecretProject and SP_TELEPATHIC_MATRIX provide the typed project owner","source:src/base.h:BaseCurrent and Base-owned drone, talent, governor and state members","source:src/faction.h:Players owns faction rule_flags; LockEnableMask owns human-faction bits"],"reason":"Recover base drone state transitions using the existing Base, SecretProject, Player, and lock owners; retain native callback reloads and the separate drone-riot call."}
void __cdecl base_drones() {
    // Native popup literals remain ordinary C++ literals: "Alopdir.pcx" occurs at 0x689C78..0x689C84, 0x689CAC..0x689CB8, 0x689CE8..0x689CF4 and 0x689D04..0x689D10; the "GOLDENAGE", "GOLDENAGEOVER", "DRONERIOTSOVER" and "DRONERIOTSOVER2" keys and "talent_sm.pcx" have separate pinned call-site extents.

    Base *base = BaseCurrent;
    int faction_id = base->faction_id_current;
    const int drones = base->drone_total;

    if (drones == 0 && base->population_size > 2 &&
        base->talent_total >= (base->population_size + 1) / 2) {
        if ((base->state & BSTATE_GOLDEN_AGE_ACTIVE) == 0) {
            if ((Players[faction_id].rule_flags & RFLAG_ALIEN) == 0) {
                popb("GOLDENAGE", 0x20, -1,
                     "talent_sm.pcx", 0);
            } else {
                popb("GOLDENAGE", 0x20, -1,
                     "Alopdir.pcx", 0);
            }
            base = BaseCurrent;
        }
        base->state |= BSTATE_GOLDEN_AGE_ACTIVE;
    } else if ((base->state & BSTATE_GOLDEN_AGE_ACTIVE) != 0) {
        base->state &= ~BSTATE_GOLDEN_AGE_ACTIVE;
        base = BaseCurrent;
        if (base->drone_total <= base->talent_total) {
            faction_id = base->faction_id_current;
            if ((Players[faction_id].rule_flags & RFLAG_ALIEN) == 0) {
                popb("GOLDENAGEOVER", 0x40, -1,
                     "talent_sm.pcx", 0);
            } else {
                popb("GOLDENAGEOVER", 0x40, -1,
                     "Alopdir.pcx", 0);
            }
            base = BaseCurrent;
        }
    }

    if (base->drone_total > base->talent_total) {
        int exempt = 0;
        const int exemption_faction = GameDataStateStorage.fields_.secret_project_.telepathic_matrix();
        if (exemption_faction >= 0 &&
            Bases[exemption_faction].faction_id_current == base->faction_id_current) {
            exempt = 1;
        }
        if (!exempt && base->nerve_staple_turns_left == 0) {
            drone_riot();
            return;
        }
    }

    if ((base->state & BSTATE_DRONE_RIOTS_ACTIVE) != 0) {
        base->state &= ~BSTATE_DRONE_RIOTS_ACTIVE;
        base = BaseCurrent;
        if ((base->state & BSTATE_GOLDEN_AGE_ACTIVE) == 0) {
            faction_id = base->faction_id_current;
            const uint32_t governor = base->governor;
            const bool manages_citizens =
                (governor & GOV_MANAGES_CITIZENS_SPECS) != 0;
            const bool governor_active = (governor & GOV_ACTIVE) != 0;
            const bool alien =
                (Players[faction_id].rule_flags & RFLAG_ALIEN) != 0;

            if (alien) {
                if (!manages_citizens || !governor_active) {
                    popb("DRONERIOTSOVER", 0x33, 0x10,
                         "Alopdir.pcx", 0);
                } else {
                    popb("DRONERIOTSOVER2", 0x33, 0,
                         "Alopdir.pcx", 0);
                }
            } else if (!manages_citizens || !governor_active) {
                popb("DRONERIOTSOVER", 0x33, 0x10,
                     "talent_sm.pcx", 0);
            } else {
                popb("DRONERIOTSOVER2", 0x33, 0,
                     "talent_sm.pcx", 0);
            }
            base = BaseCurrent;
        }

        faction_id = base->faction_id_current;
        if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1u << faction_id)) == 0) {
            base_reset(BaseIDCurrentSelected, 0);
            base = BaseCurrent;
        }
        draw_tile(base->x, base->y, 2);
    }
}

// Facility indexing follows Base.facilities_built byte and bit indices.

static unsigned char faction_bit(int faction_id) {
    return static_cast<unsigned char>(1u << (faction_id & 7));
}

static void drone_riot_reassign_vehicles_on_tile(int x, int y, int actor,
                                                 int target, int &moved) {
    for (int vehicle_id = GameDataStateStorage.fields_.vehicle_current_count_ - 1;
         vehicle_id >= 0; --vehicle_id) {
        if (vehicle_id >= GameDataStateStorage.fields_.vehicle_current_count_) continue;
        Veh &vehicle = Vehs[vehicle_id];
        if (vehicle.x != x || vehicle.y != y) continue;
        if (vehicle.faction_id == target) {
            vehicle.state &= ~VSTATE_UNK_40000;
            continue;
        }
        if (vehicle.faction_id != actor || moved >= 0x24) continue;

        vehicle.faction_id = static_cast<uint8_t>(target);
        int prototype_id = vehicle.proto_id;
        --PlayersData[actor].proto_id_active[prototype_id];
        if (prototype_id > 0x40) {
            const VehPrototype &prototype = VehPrototypes[prototype_id];
            const int replacement = propose_proto(
                target, prototype.chassis_id, prototype.weapon_id,
                prototype.armor_id, static_cast<int>(prototype.ability_flags),
                prototype.reactor_id, -2,
                const_cast<char *>(prototype.veh_name));
            if (replacement >= 0) {
                vehicle.proto_id = static_cast<int16_t>(replacement);
                prototype_id = replacement;
            }
        }
        // The native word store at Veh+0x24 covers these two adjacent bytes.
        vehicle.morale = static_cast<uint8_t>(GameDataStateStorage.fields_.turn_current_num_);
        vehicle.terraforming_turns = static_cast<uint8_t>(
            static_cast<unsigned int>(GameDataStateStorage.fields_.turn_current_num_) >> 8);
        ++PlayersData[target].proto_id_active[prototype_id];
        owner_set(vehicle.x, vehicle.y, target);
        if (Chassis[VehPrototypes[prototype_id].chassis_id].range != 0)
            vehicle.terraforming_turns = 0;
        if (vehicle.order != ORDER_HOLD) vehicle.order = ORDER_NONE;
        vehicle.state &= static_cast<uint32_t>(0xfcffbdffu);
        ++moved;
    }
}

// ORIGINAL: 0x004F4FB0 ?drone_riot@@YAXXZ 0x004F4FB0-0x004F5CD6
// RECOVERY_BINDING: v1 {"evidence":["original:0x004F4FB0-0x004F5CD6:pinned drone-riot span and direct callers","original:0x0090EA30:BaseCurrent receiver; pinned offsets correlate to src/base.h Base fields","original:0x004F4FB0-0x004F5CD6:the receiver member displacements correlate with the typed Base fields","source:src/base.h:Base and BaseCurrent own callback-sensitive base state","source:src/veh.h:Veh and VehPrototype own vehicle transfer records and prototype data","source:src/map.h:Map, bit_set and visibility synchronization own tile state"],"reason":"Recover drone riot through current Base, Faction, Vehicle, Facility, and Map owners while preserving native callback reloads, candidate/radius ordering, and literal call-site distinctions."}
// TRIED: the first ~180 bytes (unrest flag check, draw_tile, the four popb dialog variants) are transcribed and argument-order checked directly against the raw disassembly. Past that, Ghidra typed `DAT_0090ea30` (BaseCurrent) as `short*`, so every offset it prints is HALF the real byte offset - doubled here and cross-checked against src/base.h's `Base` (0x38 governor, 0x4C queue_size, 0x50 queue_production_id, 0x7C specialist_total match exactly). Eight callees (owner_set/kill/best-target-transfer helpers at 0x46B1F0, 0x46B190, 0x580860, 0x591B10, 0x591C10, 0x5ADE80, 0x5B57D0, 0x5C0B00) have no catalogued name or signature in this scaffold and are called with no arguments; the base_energy/minerals/nutrient/support/yield call order is a guess. The nearest-rival-faction base-transfer arithmetic (distance metric, unit disband loop) follows Ghidra's shape but is not verified instruction-for-instruction.
void __cdecl drone_riot() {
    // Native popup literals remain ordinary C++ literals: "ALdrone_sm.pcx" 0x689B9C..0x689BAB, 0x689BD4..0x689BE3 and 0x689C24..0x689C33; "DRONERIOTS2" 0x689BAC..0x689BB8 and 0x689BC8..0x689BD4; "drone_sm.pcx" 0x689BB8..0x689BC5, 0x689BF0..0x689BFD and 0x689C40..0x689C4D; "DRONERIOTS" 0x689BE4..0x689BEF and 0x689C00..0x689C0B; the distinct "DRONERIOT", "baseobl_sm.pcx" and "DRONEREVOLT" extents remain distinct call-site literals.

    Base *base = BaseCurrent;
    const int faction_id = base->faction_id_current;
    // Native computes (~state >> 1) & 1: true means the old unrest bit was
    // clear, despite the misleading local name in the retained scaffold.
    const bool unrest_flag_was_clear =
        (base->state & BSTATE_DRONE_RIOTS_ACTIVE) == 0;
    base->state |= BSTATE_DRONE_RIOTS_ACTIVE;

    if (unrest_flag_was_clear) {
        draw_tile(base->x, base->y, 2);
        // Native re-reads BaseCurrent after the draw callback.
        base = BaseCurrent;
        if ((base->governor & 0x40u) != 0 &&
            (base->governor & 0x80000000u) != 0) {
            if (!is_alien_faction(faction_id)) {
                popb("DRONERIOTS2", 0, 0x11, "drone_sm.pcx", 0);
            } else {
                popb("DRONERIOTS2", 0, 0x11, "ALdrone_sm.pcx", 0);
            }
            goto after_popup;
        }
    }

    {
        const bool governor_allows = unrest_flag_was_clear &&
            (base->governor & 0x40u) != 0 &&
            (base->governor & 0x80000000u) != 0;
        if (is_alien_faction(faction_id)) {
            popb("DRONERIOTS", governor_allows ? 0 : 8, 0x11,
                 "ALdrone_sm.pcx", 0);
        } else {
            popb("DRONERIOTS", governor_allows ? 0 : 8, 0x11,
                 "drone_sm.pcx", 0);
        }
    }

after_popup:
    // Popup callbacks may replace/reselect the current base; the native body
    // explicitly reloads the selected Base pointer here.
    base = BaseCurrent;
    if ((base->state & BSTATE_DRONE_RIOTS_ACTIVE) == 0) return;

    if (unrest_flag_was_clear) {
        if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & faction_bit(faction_id)) != 0) return;
        base_reset(BaseIDCurrentSelected, 0);
        return;
    }

    const int base_id = BaseIDCurrentSelected;
    int offset = 0;
    int mask = 0;
    bitmask(0x23, &offset, &mask);
    int riot_total = PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::POLICE] + 7;
    if ((Bases[base_id].facilities_built[offset] & mask) != 0) riot_total += 2;
    if (riot_total != 1 && riot_total - 1 >= 0 &&
        rand() % riot_total != 0) return;

    bitmask(1, &offset, &mask);
    if (PlayersData[faction_id].current_num_bases > 1) {
        base = BaseCurrent;
        if (base->assimilation_turns_left < 0x2d) {
            if (base->population_size < 4 && rand() % 3 != 0) return;

            int total_weight = base->population_size -
                PlayersData[faction_id].ranking -
                PlayersData[faction_id].diff_level + 0x14;
            int tries = 0;
            bool converted = false;
            do {
                const int spec_type = rand() % 0x46;
                bool blocked = false;
                const Player &player = Players[faction_id];
                for (int k = 0; k < player.faction_bonus_count; ++k) {
                    const int bonus_id = player.faction_bonus_id[k];
                    const int bonus_value = player.faction_bonus_val1[k];
                    if (bonus_id == 2 ||
                        (bonus_id == 12 && has_tech_call(
                            Facility[bonus_value].preq_tech, faction_id))) {
                        if (spec_type == bonus_value) {
                            blocked = true;
                            break;
                        }
                    }
                }

                if (spec_type > 1 && spec_type < 0x41) {
                    bitmask(spec_type, &offset, &mask);
                    base = BaseCurrent;
                    if ((base->facilities_built[offset] & mask) != 0 && !blocked) {
                        if (tries <= total_weight) {
                            bitmask(spec_type, &offset, &mask);
                            base = BaseCurrent;
                            base->facilities_built[offset] &=
                                static_cast<uint8_t>(~mask);
                            parse_say(1, static_cast<int>(reinterpret_cast<intptr_t>(
                                Facility[spec_type].name)), -1, -1);
                            if (!is_alien_faction(faction_id)) {
                                popb("DRONERIOT", 8, -1,
                                     "drone_sm.pcx", 0);
                            } else {
                                popb("DRONERIOT", 8, -1,
                                     "ALdrone_sm.pcx", 0);
                            }
                            converted = true;
                            return;
                        }
                        break;
                    }
                }
                ++tries;
            } while (tries <= total_weight);
            (void)converted;

            bitmask(0x23, &offset, &mask);
            base = BaseCurrent;
            const bool has_pressure = (base->facilities_built[offset] & mask) != 0;
            const int severity = PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::POLICE] +
                                 (has_pressure ? 2 : 0);
            if (severity < -2 && !is_alien_faction(faction_id) &&
                (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & faction_bit(faction_id)) != 0) {
                int best_faction = -1;
                const int former_faction = static_cast<int8_t>(
                    BaseCurrent->faction_id_former);
                if (former_faction != faction_id) {
                    best_faction = former_faction;
                }
                int best_score = 0x270f;
                if (former_faction == faction_id) {
                    for (int rival_base = 0; rival_base < GameDataStateStorage.fields_.base_current_count_; ++rival_base) {
                        if (Bases[rival_base].faction_id_current == faction_id) continue;
                        int dx = abs(static_cast<int>(Bases[rival_base].x) - BaseCurrent->x);
                        if ((MapIsFlat & 1) == 0 && static_cast<int>(MapLongitude) < dx)
                            dx = static_cast<int>(MapLongitudeBounds) - dx;
                        const int dy = abs(static_cast<int>(Bases[rival_base].y) - BaseCurrent->y);
                        const int high = dx > dy ? dx : dy;
                        const int low = dx < dy ? dx : dy;
                        int score = high - (((dx + dy) / 2 - low + 1) / 2);
                        const int relation = best_faction == -1
                            ? PlayersData[faction_id].last_turn_new_base
                            : static_cast<int>(
                                PlayersData[faction_id].diplo_treaties[best_faction]);
                        if ((relation & DTREATY_PACT) != 0) score *= 2;
                        if ((relation & DTREATY_TREATY) != 0)
                            score = (3 * score) / 2;
                        if ((relation & DTREATY_TRUCE) != 0) score /= 2;
                        if (score < best_score) {
                            best_score = score;
                            best_faction = Bases[rival_base].faction_id_current;
                        }
                    }
                }

                {
                    // Native 0x004F5553..0x004F565C builds seven per-player
                    // weights from faction bonus id 13. The probability of
                    // retaining the nearest-base fallback is the product of
                    // (1 - weight/100); otherwise a time-derived weighted
                    // draw replaces it. Keep the signed divisions/modulo and
                    // the live timeGetTime sample used by the native code.
                    int target_weight[7];
                    // Native 4F5570..4F55B6 writes a slot only on the first
                    // type-13 match; 4F55D5..4F55F4 nevertheless reads all
                    // seven slots, skipping the product only for value -1.
                    // Preserve the untouched slots, not zero defaults. Their
                    // incidental native stack values remain a fidelity
                    // obligation: an indeterminate C++ read is not proof of
                    // the original compiler/runtime effects.
                    int total_target_weight = 0;
                    double fallback_probability = 1.0;
                    for (int candidate_player_id = 1; candidate_player_id < MaxPlayerNum; ++candidate_player_id) {
                        const Player &candidate = Players[candidate_player_id];
                        for (int k = 0; k < candidate.faction_bonus_count; ++k) {
                            if (candidate.faction_bonus_id[k] == 13) {
                                int weight = candidate.faction_bonus_val1[k];
                                if (weight < 0) weight = 0;
                                if (weight > 100) weight = 100;
                                total_target_weight += weight;
                                target_weight[candidate_player_id - 1] = weight;
                                break;
                            }
                        }
                    }
                    for (int i = 0; i < 7; ++i) {
                        const int weight = target_weight[i];
                        if (weight != -1)
                            fallback_probability *= 1.0 - weight * 0.01;
                    }
                    const int tick = static_cast<int>(timeGetTime());
                    const int fallback_cutoff =
                        static_cast<int>(fallback_probability * 100.0);
                    // Native skips weighted selection only when cutoff is
                    // strictly below the time residue; equality draws.
                    if (fallback_cutoff >= tick % 100) {
                        int draw = (tick / 100) % total_target_weight;
                        for (int i = 0; i < 7; ++i) {
                            if (draw < target_weight[i]) {
                                best_faction = i + 1;
                                break;
                            }
                            draw -= target_weight[i];
                        }
                    }
                    if (best_faction <= 0) return;
                    const unsigned char human_mask =
                        static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_);
                    if ((human_mask & faction_bit(faction_id)) == 0 ||
                        (human_mask & faction_bit(best_faction)) == 0 ||
                        is_alien_faction(best_faction)) return;

                    const int old_x = BaseCurrent->x;
                    const int old_y = BaseCurrent->y;
                    BaseCurrent->faction_id_current =
                        static_cast<uint8_t>(best_faction);
                    for (int attacker_player_id = 1; attacker_player_id < MaxPlayerNum; ++attacker_player_id) {
                        if (PlayersData[attacker_player_id].base_id_atk_target == base_id)
                            PlayersData[attacker_player_id].base_id_atk_target = -1;
                    }
                    BaseCurrent->assimilation_turns_left =
                        BaseCurrent->faction_id_former != best_faction ? 0x32 : 0;
                    BaseCurrent->minerals_accumulated_2 = 0;
                    BaseCurrent->minerals_accumulated = 0;
                    BaseCurrent->production_id_last = -69;
                    const int queue_last = static_cast<int>(BaseCurrent->queue_size);
                    if (queue_last >= 0) {
                        for (int q = 0; q <= queue_last; ++q) {
                            const int production = BaseCurrent->queue_production_id[q];
                            if (production >= 0)
                                --PlayersData[faction_id].proto_id_active[production];
                        }
                    }
                    BaseCurrent->queue_size = 0;
                    BaseCurrent->queue_production_id[0] = -69;
                    --PlayersData[faction_id].current_num_bases;
                    ++PlayersData[best_faction].current_num_bases;
                    PlayersData[best_faction].last_turn_new_base = GameDataStateStorage.fields_.turn_current_num_;
                    replay_base(1, old_x, old_y, best_faction);

                    // Native reuses the selected Base pointer after replay
                    // callbacks and copies Governor from the strongest
                    // base of a different owner (worked bit 1 doubles size).
                    int best_governor_base = -1;
                    int best_population_weight = 0;
                    for (int rival_base = 0;
                         rival_base < GameDataStateStorage.fields_.base_current_count_; ++rival_base) {
                        if (Bases[rival_base].faction_id_current == faction_id) continue;
                        int population_weight = Bases[rival_base].population_size;
                        bitmask_call(1, &offset, &mask);
                        if ((Bases[rival_base].facilities_built[offset] & mask) != 0)
                            population_weight *= 2;
                        if (population_weight > best_population_weight) {
                            best_population_weight = population_weight;
                            best_governor_base = rival_base;
                        }
                    }
                    if (best_governor_base >= 0)
                        BaseCurrent->governor = Bases[best_governor_base].governor;
                    BaseCurrent->state &= static_cast<uint32_t>(0xffdef7b1u);

                    // Native visits every valid tile in the 21-entry base
                    // radius and walks the live vehicle array for each tile.
                    int transferred_vehicle_count = 0;
                    for (int radius = 0; radius < 21; ++radius) {
                        int x = old_x + RadiusOffsetX[radius];
                        int y = old_y + RadiusOffsetY[radius];
                        if ((MapIsFlat & 1) == 0) {
                            if (x < 0) x += MapLongitudeBounds;
                            else if (MapLongitudeBounds <= x) x -= MapLongitudeBounds;
                        }
                        if (y >= 0 && y < MapLatitudeBounds &&
                            x >= 0 && x < MapLongitudeBounds) {
                            using_set(x, y, best_faction);
                            map_loc(x, y)->visibility |= faction_bit(best_faction);
                            drone_riot_reassign_vehicles_on_tile(
                                x, y, faction_id, best_faction,
                                transferred_vehicle_count);
                        }
                    }
                    spot_base(BaseIDCurrentSelected, best_faction);
                    reset_territory();

                    for (int vehicle_id = GameDataStateStorage.fields_.vehicle_current_count_ - 1;
                         vehicle_id >= 0; --vehicle_id) {
                        if (vehicle_id >= GameDataStateStorage.fields_.vehicle_current_count_) continue;
                        Veh &vehicle = Vehs[vehicle_id];
                        if (vehicle.faction_id == faction_id &&
                            vehicle.home_base_id == BaseIDCurrentSelected)
                            kill(vehicle_id);
                    }

                    // Native reapplies only actor and selected-recipient rows.
                    for (int bonus_pass = 0; bonus_pass < 2; ++bonus_pass) {
                        const int bonus_player_id =
                            bonus_pass == 0 ? faction_id : best_faction;
                        const Player &player = Players[bonus_player_id];
                        for (int k = 0; k < player.faction_bonus_count; ++k) {
                            const int bonus_id = player.faction_bonus_id[k];
                            const int bonus_value = player.faction_bonus_val1[k];
                            if (bonus_id == 2 ||
                                (bonus_id == 12 && has_tech_call(
                                    Facility[bonus_value].preq_tech, faction_id))) {
                                int facility_id = bonus_value;
                                if (facility_id < 1) facility_id = 1;
                                if (facility_id > 0x45) facility_id = 0x45;
                                bitmask_call(facility_id, &offset, &mask);
                                Bases[BaseIDCurrentSelected].facilities_built[offset] |=
                                    static_cast<uint8_t>(mask);
                            }
                        }
                    }
                    owner_set(old_x, old_y, best_faction);
                    BaseComputedID = BaseIDCurrentSelected;
                    base_support();
                    base_yield();
                    base_nutrient();
                    base_minerals();
                    base_energy();

                    const int reloaded_base_owner =
                        Bases[BaseIDCurrentSelected].faction_id_current;
                    const int console_faction = ConsoleGlobal.faction_id_;
                    if (faction_id == console_faction ||
                        best_faction == console_faction ||
                        reloaded_base_owner == console_faction ||
                        (Bases[BaseIDCurrentSelected].visibility &
                         faction_bit(console_faction)) != 0) {
                        MapWinTable[0]->set_center(old_x, old_y, 1);
                        draw_map(1);
                        parse_says(0, BaseCurrent->name_string, -1, -1);
                        parse_says(2, Players[best_faction].adj_name_faction, -1, -1);
                        if ((GameDataStateStorage.fields_.game_state_ & 1) == 0 || (GameDataStateStorage.fields_.game_state_ & 8) != 0)
                            popp("SCRIPT", "DRONEREVOLT", 0,
                                 "baseobl_sm.pcx", 0);
                    }
                }
            }
        }
    }
}

/*
// ORIGINAL: 0x004E7DB0 sub_4e7db0 0x004E7DB0-0x004E7DBD BYTE_EXACT
// symbol    _sub_4e7db0
// size      13 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: zero
Status: Complete
*/
extern "C" int __cdecl sub_4e7db0() {
    TileYieldRestricted = 0;
    return 0;
}
