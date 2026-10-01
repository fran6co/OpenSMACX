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
#include "spying_recovery.h"
#include "general.h"
#include "netmsg.h"
#include "replaywin.h"
#include "messagewin.h"
#include "tutwin.h"
#include "path.h"
#include "terraforming.h"

void __cdecl atrocity(int faction_id, int victim, int kind, int unknown);
void __cdecl kill(int vehicle_id);
void __cdecl interlude(int id, char *text, int mode, int flag);

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
            NetMsgPopup->pop("NERVESTAPLETHEM", 5000, 0, 0);
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
        || (SecretProject.citizens_defense_force >= 0
            && Bases[SecretProject.citizens_defense_force].faction_id_current == faction)) {
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
            || (GameState & STATE_OMNISCIENT_VIEW)
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
// base_compute publishes the selected id before running the resource passes.
// Window refreshes invalidate this cache without changing the selected base.
// RECOVERY_OBJECT: v2 0x00689374 0x00689378 ?BaseComputedID@@3HA 0x004EC3B0
int BaseComputedID = -1;
// RECOVERY_OBJECT: v2 0x009A64CC 0x009A64D0 ?BaseCurrentCount@@3HA 0x004E68B0
int BaseCurrentCount;  // 0x009A64CC
int BaseFindDist;  // 0x0090EA04
uint32_t ScnVictFacilityObj;  // 0x009A6814
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

// The seed shows a selected-base energy pass: convoys and inefficiency first,
// then commerce, allocation, specialists and facility multipliers. The first
// and last stages below use their owning C++ objects. Pending: faction trade
// credits, Planetary Energy Grid, specialist upgrades, research allocation
// effects and the final energy-production special case still need translation
// from the preserved seed before this is a complete behavioral recovery.
// ORIGINAL: 0x004EB560 ?base_energy@@YAXXZ 0x004EB560-0x004EC3A6
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004eb560.cpp", "image:0x004EB560-0x004EC3A6 ranks the selected base and publishes seven commerce pairs at 0x0090EA10/0x0090E964; its commerce base doubles when 0x009A6628 is below 0x009A6624", "source:src/base.h", "source:src/faction.h"], "reason": "Recover selected-base convoy, commerce rank/ledger and rate modifier, inefficiency, allocation, specialist and facility stages through typed owners. Several late modifiers remain pending; this binding makes no exactness claim."}
void __cdecl base_energy() {
    Base &base = *BaseCurrent;
    const int faction = base.faction_id_current;
    base.energy_intake_2 += BaseCurrentConvoyTo[2];
    base.energy_consumption = BaseCurrentConvoyFrom[2];
    int available = base.energy_intake_2 - base.energy_consumption;
    base.energy_inefficiency = black_market(available);
    base.energy_surplus = available - base.energy_inefficiency;

    // Rank the selected base among its own faction by pre-commerce energy.
    // base_rank sorts that same signed key in descending order and returns a
    // partner's base at the corresponding rank (or -1 if it has fewer bases).
    int commerce_rank = 0;
    for (int base_id = 0; base_id < BaseCurrentCount; ++base_id) {
        const Base &candidate = Bases[base_id];
        if (candidate.faction_id_current == faction &&
            base.energy_intake_1 * 512 + BaseIDCurrentSelected <
                candidate.energy_intake_1 * 512 + base_id) {
            ++commerce_rank;
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
        if (CouncilResolutionCount[PROP_REPEAL_GLOBAL_TRADE_PACT] <
            CouncilResolutionCount[PROP_GLOBAL_TRADE_PACT]) commerce_base *= 2;
        const int divisor = TechCommerceCount + 1;
        if (divisor <= 0) continue;
        int partner_receipt = ((PlayersData[partner].tech_commerce_bonus + 1) *
                               commerce_base + divisor / 2) / divisor;
        int receipt = ((PlayersData[faction].tech_commerce_bonus + 1) *
                       commerce_base + divisor / 2) / divisor;
        if ((PlayersData[faction].diplo_treaties[partner] & DTREATY_PACT) == 0) {
            receipt /= 2;
            partner_receipt /= 2;
        }
        if (GovernorFaction == faction) ++receipt;
        if (GovernorFaction == partner) ++partner_receipt;
        if (PlayersData[faction].sanction_turns != 0 ||
            PlayersData[partner].sanction_turns != 0 ||
            (Players[faction].rule_flags & RFLAG_ALIEN) != 0 ||
            (Players[partner].rule_flags & RFLAG_ALIEN) != 0) {
            receipt = 0;
            partner_receipt = 0;
        }
        BaseCommerceReceipts[slot] = receipt;
        BaseCommercePartnerReceipts[slot] = partner_receipt;
        base.energy_intake_2 += receipt;
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
        // Pending: obsolete specialists are promoted according to the seed's
        // technology and yield comparison, updating the stored nibble.
        base.economy_total += Citizen[specialist].ops_bonus;
        base.psych_total += Citizen[specialist].psych_bonus;
        base.labs_total += Citizen[specialist].research_bonus;
    }

    base.labs_total += Players[faction].rule_research;
    if (has_fac(FAC_BIOLOGY_LAB, BaseIDCurrentSelected, 0)) base.labs_total += 2;

    int psych_multiplier = 4;
    if (has_fac(FAC_HOLOGRAM_THEATRE, BaseIDCurrentSelected, 0)) psych_multiplier = 6;
    int labs_multiplier = has_fac(FAC_NETWORK_NODE, BaseIDCurrentSelected, 0) ? 6 : 4;
    int economy_multiplier = has_fac(FAC_ENERGY_BANK, BaseIDCurrentSelected, 0) ? 6 : 4;
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

    // Pending: Secret Project, faction efficiency, Network Backbone and
    // research-allocation modifiers follow these multipliers in the seed.
    base_psych();
}

// ORIGINAL: 0x004E80B0 ?base_yield@@YAXXZ 0x004E80B0-0x004E954B
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004e80b0.cpp", "source:src/base.h", "image:0x004E80CC-0x004E80D9 signed population plus one publishes90E938;0x004E86D9 resets90E900 only on allocation, whereas0x004E8456 jumps to0x004E923D and retains it.", "image:0x004E83D9-0x004E8426 four resource calls per admitted tile;0x004E8E0C-0x004E923D performs one replacement with fresh-call undo;0x004E930C-0x004E954B publishes mask/specialists/intakes and accounts for all21valid radius positions."], "reason": "Recover the complete selected-base worker and specialist allocation pass with typed resource fields, real shared counters, original selection and replacement behavior, and final ecology accounting. Retain its seed identity and evidence; the former link-only placeholder is replaced, not accepted by a source binding."}
void __cdecl base_yield() {
    // Original domain: live selected base/map, valid specialist encodings and
    // population counts, and a positive nutrient requirement for IDIV. The
    // original does not guard malformed data or the zero-divisor rules case.
    farm_compute();
    Base *base = BaseCurrent;
    const int faction = base->faction_id_current;
    const uint32_t faction_bit = 1u << faction;
    BasePopulationUnassigned = base->population_size + 1;
    const bool automatic = (static_cast<unsigned char>(LockEnableMask) & faction_bit) == 0
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
            if (!full_yield && SecretProject.space_elevator >= 0) {
                full_yield = Bases[SecretProject.space_elevator].faction_id_current == faction;
            }
            if (!full_yield && SecretProject.cloudbase_academy >= 0) {
                full_yield = Bases[SecretProject.cloudbase_academy].faction_id_current == faction;
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
        if ((static_cast<unsigned char>(LockEnableMask) & faction_bit) != 0) {
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
                    || max_nutrients == 0) && TurnCurrentNum > 100) {
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
int base_doctors_best_specialist(const Base &base) {
    int best = CITIZEN_TECHNICIAN;
    int best_score = -999;
    for (int type = 0; type < MaxSpecialistNum; ++type) {
        if (!has_tech(Citizen[type].preq_tech, base.faction_id_current)) continue;
        int score = Citizen[type].psych_bonus * 3;
        if (base.population_size >= static_cast<int>(Rules.min_base_size_specialists)) {
            score += Citizen[type].ops_bonus + Citizen[type].research_bonus;
        }
        if (score > best_score) {
            best = type;
            best_score = score;
        }
    }
    return best;
}

void base_doctors_add_specialist(Base &base) {
    const unsigned int slot = static_cast<unsigned int>(base.specialist_total++);
    if (slot < 16) {
        const unsigned int shift = (slot & 7u) * 4;
        uint32_t &word = base.spec[slot >> 3];
        word = (word & ~(15u << shift))
             | (static_cast<uint32_t>(base_doctors_best_specialist(base)) << shift);
    }
}

void base_doctors_recompute() {
    BaseComputedID = BaseIDCurrentSelected;
    base_support();
    base_yield();
    base_nutrient();
    base_minerals();
    base_energy();
}

bool base_doctors_project_owned(int project, int faction) {
    const int owner = base_project(project);
    return owner >= 0 && Bases[owner].faction_id_current == faction;
}

} // namespace

// ORIGINAL: 0x004F5F30 ?base_doctors@@YAXXZ 0x004F5F30-0x004F650C
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004f5f30.cpp", "source:src/base.h"], "reason": "Recover the selected Base's specialist/worker adjustment and repeated resource recomputation through typed Base, citizen and project state. Riot surcharge and warning popup paths remain unresolved; this is not a byte or behavioral claim."}
void __cdecl base_doctors() {
    Base &base = *BaseCurrent;
    const int faction = base.faction_id_current;
    const bool matrix = base_doctors_project_owned(SP_TELEPATHIC_MATRIX, faction);

    // A human governor only changes citizen allocation when both its citizen
    // option and active state are set. The nutrient flag is published by
    // base_yield and can survive a cached allocation.
    if ((static_cast<unsigned char>(LockEnableMask) & (1u << faction))
        && (base.governor & (GOV_MANAGES_CITIZENS_SPECS | GOV_ACTIVE))
            != (GOV_MANAGES_CITIZENS_SPECS | GOV_ACTIVE)) return;

    for (int attempt = 0; attempt < 9; ++attempt) {
        const bool needs_psych = BaseNutrientAdequate != 0
            && base.drone_total > base.talent_total && !matrix;
        if (needs_psych) {
            base_doctors_add_specialist(base);
            base_doctors_recompute();
            if (base.drone_total <= base.talent_total || matrix) return;
            continue;
        }

        // A non-psych specialist can return to the fields only if there is
        // an allocated slot beyond the protected specialist count. Recompute
        // the yields after each removal, as the original does.
        if (base.specialist_total <= base.specialist_unk_1
            || (base.state & BSTATE_DRONE_RIOTS_ACTIVE)) return;
        --base.specialist_total;
        base_doctors_recompute();
        if (base.drone_total > base.talent_total && !matrix) {
            base_doctors_add_specialist(base);
            return;
        }
    }
}

// ORIGINAL: 0x004E4B80 ?base_init@@YAHHHH@Z 0x004E4B80-0x004E5250
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004e4b80.cpp: canonical identity, original spans and retained seed history", "source:src/base.h: real Base storage and lifecycle contracts", "source:recovery/tasks/base-lifecycle-owner-closure.json: bounded coordinator authorization"], "reason": "The original base creation operation owns initialization of Bases records and faction base counts; source expresses real Base/PlayerData fields, facility bitmaps, technology and named UI/replay dependencies."}
int __cdecl base_init(int faction_id, int x, int y) {
    if (BaseCurrentCount >= 512) return -1;
    Map *tile = map_loc(x, y);
    const unsigned int region = tile->region;
    bit_set(x, y, 1, 1);
    owner_set(x, y, faction_id);
    synch_bit(x, y, faction_id);
    const int base_id = BaseCurrentCount++;
    PlayerData &faction = PlayersData[faction_id];
    ++faction.current_num_bases;
    ++faction.region_total_bases[region];
    replay_base(0, x, y, faction_id);
    Base &base = Bases[base_id];
    base.x = static_cast<int16_t>(x);
    base.y = static_cast<int16_t>(y);
    base.faction_id_current = base.faction_id_former = static_cast<uint8_t>(faction_id);
    base.nerve_staple_turns_left = base.assimilation_turns_left = base.unk_1 = 0;
    base.population_size = SecretProject.planetary_transit_system >= 0
        && Bases[SecretProject.planetary_transit_system].faction_id_current == faction_id ? 3 : 1;
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
    if (TurnCurrentNum == 0) {
        const int production = has_tech(VehPrototypes[1].preq_tech, faction_id)
            ? 1 : (faction.diff_level > 1 ? 2 : 0);
        base.queue_production_id[0] = production;
        base.minerals_accumulated = cost_factor(faction_id, 1, -1)
            * VehPrototypes[production].cost / 2;
    } else {
        base.minerals_accumulated = faction.soc_effect_active.support < -1
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
            base.population_size = static_cast<int8_t>(max(1, min(5, (TurnCurrentNum + 49) / 50)));
            if (TurnCurrentNum > 74) set_fac(3, base_id, TRUE);
            if (TurnCurrentNum > 99) set_fac(6, base_id, TRUE);
            if (TurnCurrentNum >= 150) set_fac(7, base_id, TRUE);
        } else {
            base.population_size = 3;
            if (Players[faction_id].rule_talent || faction.soc_effect_base.planet > 0)
                base.population_size = 4;
            if (Players[faction_id].rule_drone || Players[faction_id].rule_population)
                --base.population_size;
            set_fac(3, base_id, TRUE);
            if (DiffLevelCurrent > 2) set_fac(6, base_id, TRUE);
            if (faction.soc_effect_base.economy >= 1) set_fac(7, base_id, TRUE);
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
        &SecretProject.human_genome_project,
        &SecretProject.command_nexus,
        &SecretProject.weather_paradigm,
        &SecretProject.merchant_exchange,
        &SecretProject.empath_guild,
        &SecretProject.citizens_defense_force,
        &SecretProject.virtual_world,
        &SecretProject.planetary_transit_system,
        &SecretProject.xenoempathy_dome,
        &SecretProject.neural_amplifier,
        &SecretProject.maritime_control_center,
        &SecretProject.planetary_datalinks,
        &SecretProject.supercollider,
        &SecretProject.ascetic_virtues,
        &SecretProject.longevity_vaccine,
        &SecretProject.hunter_seeker_algorithm,
        &SecretProject.pholus_mutagen,
        &SecretProject.cyborg_factory,
        &SecretProject.theory_of_everything,
        &SecretProject.dream_twister,
        &SecretProject.universal_translator,
        &SecretProject.network_backbone,
        &SecretProject.nano_factory,
        &SecretProject.living_refinery,
        &SecretProject.cloning_vats,
        &SecretProject.self_aware_colony,
        &SecretProject.clinical_immortality,
        &SecretProject.space_elevator,
        &SecretProject.singularity_inductor,
        &SecretProject.bulk_matter_transmitter,
        &SecretProject.telepathic_matrix,
        &SecretProject.voice_of_planet,
        &SecretProject.ascent_to_transcendence,
        &SecretProject.manifold_harmonics,
        &SecretProject.nethack_terminus,
        &SecretProject.cloudbase_academy,
        &SecretProject.planetary_energy_grid,
        &SecretProject.user_defined_38,
        &SecretProject.user_defined_39,
        &SecretProject.user_defined_40,
        &SecretProject.user_defined_41,
        &SecretProject.user_defined_42,
        &SecretProject.user_defined_43,
        &SecretProject.user_defined_44,
        &SecretProject.user_defined_45,
        &SecretProject.user_defined_46,
        &SecretProject.user_defined_47,
        &SecretProject.user_defined_48,
        &SecretProject.user_defined_49,
        &SecretProject.user_defined_50,
        &SecretProject.user_defined_51,
        &SecretProject.user_defined_52,
        &SecretProject.user_defined_53,
        &SecretProject.user_defined_54,
        &SecretProject.user_defined_55,
        &SecretProject.user_defined_56,
        &SecretProject.user_defined_57,
        &SecretProject.user_defined_58,
        &SecretProject.user_defined_59,
        &SecretProject.user_defined_60,
        &SecretProject.user_defined_61,
        &SecretProject.user_defined_62,
        &SecretProject.user_defined_63,
        &SecretProject.user_defined_64,
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
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004e5250.cpp: canonical identity, original spans and retained seed history", "source:src/base.h: real Base storage and lifecycle contracts", "source:recovery/tasks/base-lifecycle-owner-closure.json: bounded coordinator authorization"], "reason": "The original base destruction operation belongs to the Base owner and repairs real faction/project/base/vehicle references and count/selection state rather than forwarding into the original executable."}
void __cdecl base_kill(int base_id) {
    Base &base = Bases[base_id];
    const int x = base.x, y = base.y;
    const int faction_id = base.faction_id_current;
    PlayerData &faction = PlayersData[faction_id];
    if (has_fac(1, base_id, 0) && TurnCurrentNum < faction.corner_market_turn
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
    for (vehicle = VehCurrentCount - 1; vehicle >= 0; --vehicle) {
        if (vehicle >= VehCurrentCount || Vehs[vehicle].home_base_id != base_id) continue;
        if (Vehs[vehicle].faction_id != faction_id) {
            Vehs[vehicle].home_base_id = -1;
            continue;
        }
        base.faction_id_current = 255;
        int nearest = -1, distance = 9999;
        for (int candidate = 0; candidate < BaseCurrentCount; ++candidate) {
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
    if (base_id < BaseCurrentCount - 1)
        memcpy(&Bases[base_id], &Bases[base_id + 1],
            (BaseCurrentCount - base_id - 1) * sizeof(Base));
    --BaseCurrentCount;
    if (BaseComputedID == base_id) BaseComputedID = -1;
    for (vehicle = 0; vehicle < VehCurrentCount; ++vehicle)
        if (Vehs[vehicle].home_base_id > base_id) --Vehs[vehicle].home_base_id;
    int remaining;
    for (remaining = 0; remaining < BaseCurrentCount; ++remaining) {
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
                messages.on_remove(entry->payload);
                delete entry->payload;
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
        for (vehicle = 0; vehicle < VehCurrentCount; ++vehicle)
            if (VehPrototypes[Vehs[vehicle].proto_id].plan == 7
                && ((x_dist(Vehs[vehicle].x, x) + abs(Vehs[vehicle].y - y)) & ~1) == 0)
                stack_veh(vehicle, 0);
    }
    // Original terrain cleanup begins at the one-past active vehicle slot.
    // Do not silently repair that stale-slot/error-path read with a new guard.
    for (vehicle = VehCurrentCount; vehicle >= 0; --vehicle) {
        if (get_triad(vehicle) == (ocean ? 0 : 1)
            && ((x_dist(Vehs[vehicle].x, x) + abs(Vehs[vehicle].y - y)) & ~1) == 0
            && (!ocean || Vehs[vehicle].order != 1 || Vehs[vehicle].waypoint_x[0] < 0))
            kill(vehicle);
    }
    TutWinBaseWindow.base_killed(base_id);
    for (remaining = 0; remaining < BaseCurrentCount; ++remaining) base_mark(remaining);
    reset_territory();
    UnkBitfield1 |= 2;
}

// These dependencies retain their complete historical seeds under
// src/unrecovered/004f81a0.cpp and src/recovered/units/004e5a60.cpp. Their
// owning-TU bodies remain explicitly unfinished while those independent
// recoveries are pending.
// ORIGINAL: 0x004F81A0 ?base_build@@YAHHPAH00@Z 0x004F81A0-0x004FFFB8
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004f81a0.cpp", "source:src/base.cpp:base_reset"], "reason": "Explicitly unfinished Base-owned dependency required to link the recovered base_reset caller; full 72-arm build-selection behavior remains in its seed"}
int __cdecl base_build(int, int *, int *, int *) {
    // BODY GOES HERE.
    return -1; // Seed's no-candidate sentinel; selection is still unfinished.
}

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
        && (static_cast<unsigned char>(LockEnableMask)
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
    if ((!new_unbuilt && !old_relevant) || ((GameState & 1) && !(GameState & 8))) return;
    const int facility = abs(next);
    const int project = facility - 70;
    int word, mask;
    bitmask(new_unbuilt ? project : abs(previous) - 70, &word, &mask);
    int old_count = 0, new_count = 0;
    int other;
    for (other = 0; other < BaseCurrentCount; ++other) {
        if (Bases[other].faction_id_current == base.faction_id_current && other != base_id) {
            if (Bases[other].queue_production_id[0] == previous) ++old_count;
            if (Bases[other].queue_production_id[0] == base.queue_production_id[0]) ++new_count;
        }
    }
    if (!new_unbuilt || new_count) return;
    Player &player = Players[owner];
    if (old_relevant && (!old_count || base_project(-previous - 70) != -1)) {
        for (other = 0; other < BaseCurrentCount; ++other)
            if (Bases[other].faction_id_current == owner
                && Bases[other].queue_production_id[0] == previous)
                Bases[other].queue_production_id[0] = base.queue_production_id[0];
        if (SunspotDuration > 0) return;
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
        if (SunspotDuration > 0 && project != 31) return;
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
        GameInterludeFlags |= 0x400;
    }
}

// ORIGINAL: 0x004E5FE0 ?base_reset@@YAXHH@Z 0x004E5FE0-0x004E62FB
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004e5fe0.cpp", "source:src/base.h", "source:src/base.cpp:BaseSiteClaimX/BaseSiteClaimY are the four 0x0090EA44/54 site-output arrays", "source:src/base.h:Base.location is the 0x130 field read as 0x0097D090 + base_id*0x134", "pinned image:0x00689484 contains secproj_sm.pcx;0x00689494 contains GOVPROJECT"], "reason": "Recover the base reset/site selection operation using the typed Base and faction state, four source-owned site claims, and source-owned popup text. Preserve the existing seed and mismatch obligation; base_build/base_change remain explicit link-only dependencies."}
void __cdecl base_reset(int base_id, int force) {
    const uint8_t faction = Bases[base_id].faction_id_current;
    const uint8_t faction_bit = static_cast<uint8_t>(1u << faction);
    const uint8_t active_mask = static_cast<uint8_t>(LockEnableMask);
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

    const uint8_t current_active_mask = static_cast<uint8_t>(LockEnableMask);
    if ((faction_bit & current_active_mask) == 0) return;
    if ((Bases[base_id].state & BSTATE_UNK_8000000000) == 0) return;
    if (location <= -0x46) return;
    if (Bases[base_id].location > -0x46) return;
    if (IsMultiplayerNet != 0) return;
    if ((GameState & STATE_UNK_1) != 0) return;

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

    for (int other_id = 0; other_id < BaseCurrentCount; ++other_id) {
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
// RECOVERY_OBJECT: v2 0x009A6614 0x009A6618 ?GovernorFaction@@3HA 0x00424410
int GovernorFaction;  // 0x009A6614
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
int GlobalEnergyEventState;  // 0x009A6818

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
int BaseAtKeyPollFlag;  // 0x009BC070

int __cdecl base_at(int x, int y) {
    if (!on_map(x, y) || !(bit_at(x, y) & BIT_BASE_IN_TILE)) {
        return -1;
    }
    Base *base = Bases;
    for (int base_id = 0; base_id < BaseCurrentCount; base_id++, base++) {
        if (base->x == x && base->y == y) {
            return base_id;
        }
    }
    if (!BaseAtErrorGuard) {
        log_say("Base Bits Error  (x, y)", x, y, 0);
    }
    if (!(GameState & STATE_DEBUG_MODE)) {
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
    if (!BaseAtKeyPollFlag) {
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
    for (int i = 0; i < BaseCurrentCount; i++) {
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
    if (BaseCurrentCount <= 0) {
        return -1;
    }
    int proximity = 9999;
    int base_id = -1;
    for (int i = 0; i < BaseCurrentCount; i++) {
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
    for (int i = 0; i < BaseCurrentCount; i++, base++) {
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
    int factor = is_human(faction_id) ? 10 : diff_cost_base[DiffLevelCurrent]
        - great_satan(FactionRankings[7], false)
        - (!IsMultiplayerNet && is_human(FactionRankings[7]));
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
            switch (PlayersData[faction_id].soc_effect_pending.industry) {
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
        int growth_factor = PlayersData[faction_id].soc_effect_pending.growth;
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
    if (PlayersData[faction_id].soc_effect_pending.morale < -1) {
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
            && PlayersData[faction_id].soc_effect_pending.planet > 0) {
            crop++;
        }
    } else if ((bit & BIT_FUNGUS) && !assume_improved
               && (tile->climate & 0xE0) >= ALT_BIT_OCEAN_SHELF) {
        int planet = PlayersData[faction_id].soc_effect_pending.planet;
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
            && PlayersData[faction_id].soc_effect_pending.planet > 1) {
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
        int planet = PlayersData[faction_id].soc_effect_pending.planet;
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
        int economy = PlayersData[faction_id].soc_effect_pending.economy
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
        if (GovernorFaction == faction_id) {
            energy++;
        }
    } else if (bit & BIT_MONOLITH) {
        energy = ResourceInfo[RSCINFO_MONOLITH].energy;
        if (has_project(SP_MANIFOLD_HARMONICS, faction_id)) {
            int planet = PlayersData[faction_id].soc_effect_pending.planet;
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
        int planet = PlayersData[faction_id].soc_effect_pending.planet;
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
            if (SecretProject.merchant_exchange == base_id) {
                energy++;
            }
            if (Bases[base_id].state & BSTATE_GOLDEN_AGE_ACTIVE) {
                golden_age = 1;
            }
        }
        if (PlayersData[faction_id].soc_effect_pending.economy + golden_age
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
    if (DustCloudDuration && energy) {
        energy--;
    }
    int event_active = UnkGlobal0093A934 ? (GlobalEnergyEventState & 2)
                                          : (GlobalEnergyEventState & 1);
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
    for (int veh_id = 0; veh_id < VehCurrentCount; veh_id++) {
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
                int support = PlayersData[faction_id].soc_effect_pending.support;
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
        int police = PlayersData[faction_id].soc_effect_pending.police
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
    BaseCurrentGrowthRate = PlayersData[faction_id].soc_effect_pending.growth;
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
    if (ascending(faction_id) && GameRules & RULES_VICTORY_TRANSCENDENCE) {
        BaseCurrent->eco_damage *= 2;
    }
    if (GameState & STATE_PERIHELION_ACTIVE) {
        BaseCurrent->eco_damage *= 2;
    }
    uint32_t diff_factor;
    if (is_human(faction_id)) {
        int diff_lvl = PlayersData[faction_id].diff_level;
        diff_factor = !diff_lvl ? DLVL_TALENT
            : ((diff_lvl <= DLVL_LIBRARIAN) ? DLVL_LIBRARIAN : DLVL_TRANSCEND);
    } else {
        diff_factor = range(6 - DiffLevelCurrent, DLVL_SPECIALIST, DLVL_LIBRARIAN);
    }
    BaseCurrent->eco_damage = ((PlayersData[faction_id].tech_ranking
        - PlayersData[faction_id].theory_of_everything)
        * (3 - range(PlayersData[faction_id].soc_effect_pending.planet, -3, 2))
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
    for (int i = 0; i < BaseCurrentCount; i++) { // modified version of vulnerable()
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
    int ineffciency = 4 - PlayersData[faction_id].soc_effect_pending.efficiency;
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
    const bool human = (static_cast<unsigned char>(LockEnableMask) & (1u << faction)) != 0;
    const int difficulty = human ? PlayersData[faction].diff_level : 3;
    const int content = base_yield_sub(6, difficulty);
    const int efficiency = PlayersData[faction].soc_effect_pending.efficiency;
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
         && base_psych_project_owned(SecretProject.virtual_world, faction))
        || has_fac(FAC_HOLOGRAM_THEATRE, BaseIDCurrentSelected, 0)) {
        facility_drones = base_yield_sub(facility_drones, 2);
    }
    if (base_psych_project_owned(SecretProject.planetary_transit_system, faction)
        && BaseCurrent->population_size <= 3) facility_drones = base_yield_sub(facility_drones, 1);
    const int facility_talents = has_fac(FAC_PARADISE_GARDEN, BaseIDCurrentSelected, 0) ? 2 : 0;
    if (base_yield_has_facility(FAC_RESEARCH_HOSPITAL)) facility_drones = base_yield_sub(facility_drones, 1);
    if (base_yield_has_facility(FAC_NANOHOSPITAL)) facility_drones = base_yield_sub(facility_drones, 1);
    int project_talents = base_psych_project_owned(SecretProject.human_genome_project, faction) ? 1 : 0;
    if (base_psych_project_owned(SecretProject.clinical_immortality, faction)) project_talents += 2;
    const bool punishment = base_yield_has_facility(FAC_PUNISHMENT_SPHERE);

    for (int talent_case = 8; talent_case >= 0; --talent_case) {
        if (talent_case != 0 && BaseUpkeepStage != 1) continue;
        const int talent = talent_case != 0 ? talent_case - 4 : PlayersData[faction].soc_effect_pending.talent;
        for (int police_case = 9; police_case >= 0; --police_case) {
            int police;
            if (police_case == 0) {
                // This query also occurs at the end of each hypothetical row.
                const bool brood_pit = base_yield_has_facility(FAC_BROOD_PIT);
                police = base_yield_add(PlayersData[faction].soc_effect_pending.police, brood_pit ? 2 : 0);
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
                if (base_psych_project_owned(SecretProject.self_aware_colony, faction)) {
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
            if (base_psych_project_owned(SecretProject.longevity_vaccine, faction)) {
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
    for (int i = 0; i < BaseCurrentCount; i++) {
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
    if (BaseCurrent->energy_surplus >= 0 || VehCurrentCount <= 0) {
        return;
    }
    uint32_t faction_id = BaseCurrent->faction_id_current;
    for (int i = 0; i < VehCurrentCount; i++) {
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
    for (int i = 0; i < BaseCurrentCount; i++) {
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
    for (int i = 0; i < BaseCurrentCount; i++) {
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
    for (int i = 0; i < BaseCurrentCount; i++) {
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
    int support = PlayersData[faction_id].soc_effect_pending.support;
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
    for (int i = 0; i < BaseCurrentCount; i++) {
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
    if (GameRules & RULES_SCN_VICT_ALL_BASE_COUNT_OBJ) {
        return true;
    }
    if (Bases[base_id].event & BEVENT_OBJECTIVE) {
        return true;
    }
    if (GameRules & RULES_SCN_VICT_SP_COUNT_OBJ) {
        for (int i = 0; i < MaxSecretProjectNum; i++) {
            if (base_project(i) == base_id) {
                return true;
            }
        }
    }
    if (GameState & STATE_SCN_VICT_BASE_FACIL_COUNT_OBJ) {
        // Open-coded has_fac(ScnVictFacilityObj, base_id, 0): with
        // queue_count == 0 it reduces to has_fac's own early-out guard
        // followed by has_fac_built(), and the image inlines exactly that
        // (its `cmp eax, 0x41 / jge` IS has_fac's `facility_id >=
        // FacilityRepStart` guard) rather than calling has_fac out of line.
        if ((int)ScnVictFacilityObj < FacilityRepStart
            && has_fac_built_call(ScnVictFacilityObj, base_id)) {
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
    for (int veh_id = 0; veh_id < VehCurrentCount; veh_id++) {
        Veh *veh = &Vehs[veh_id];
        if (veh->faction_id != faction_id || !(veh->flags & VFLAG_IS_OBJECTIVE)) {
            continue;
        }
        int base_id;
        if (GameRules & RULES_SCN_VICT_OBJ_UNITS_REACH_FRIEND_OBJ_BASE) {
            base_id = base_at(veh->x, veh->y);
            if (base_id < 0 || !is_objective(base_id)) {
                continue;
            }
        } else {
            if (!(GameRules & RULES_SCN_VICT_OBJ_UNITS_REACH_FRIEND_HQ_BASE)) {
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
            && (GameRules & RULES_VICTORY_COOPERATIVE)) {
            total++; // a pact partner's base counts under 'One For All'
        }
    }
    // Population, and bases that are objectives in their own right.
    for (int base_id = 0; base_id < BaseCurrentCount; base_id++) {
        if (Bases[base_id].faction_id_current != faction_id) {
            continue;
        }
        if (GameState & STATE_SCN_VICT_POPULATION_COUNT_OBJ) {
            total += Bases[base_id].population_size;
        }
        if (!(GameRules & RULES_SCN_VICT_ALL_BASE_COUNT_OBJ)
            && !(Bases[base_id].event & BEVENT_OBJECTIVE)) {
            continue;
        }
        total++;
    }
    if (GameState & STATE_SCN_VICT_TECH_COUNT_OBJ) {
        for (int tech_id = 0; tech_id < MaxTechnologyNum; tech_id++) {
            if (has_tech(tech_id, faction_id)) {
                total++;
            }
        }
        total += PlayersData[faction_id].theory_of_everything;
    }
    if (GameState & STATE_SCN_VICT_CREDITS_COUNT_OBJ) {
        total += PlayersData[faction_id].energy_reserves;
    }
    if (GameState & STATE_SCN_VICT_BASE_FACIL_COUNT_OBJ) {
        for (int base_id = 0; base_id < BaseCurrentCount; base_id++) {
            if (Bases[base_id].faction_id_current == faction_id
                && (int)ScnVictFacilityObj < FacilityRepStart
                && has_fac_built(ScnVictFacilityObj, base_id)) {
                total++;
            }
        }
    }
    /*
    * Both map walks step one Map along per tile visited rather than indexing,
    * because a row holds only every other x and the rows pack together; the
    * pointer is not reset between rows.
    */
    if (GameState & STATE_SCN_VICT_TERRAIN_ENH_COUNT_OBJ) {
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
    if (GameState & STATE_SCN_VICT_TERRITORY_COUNT_OBJ) {
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
    if (GameRules & RULES_SCN_VICT_SP_COUNT_OBJ) {
        for (int i = 0; i < MaxSecretProjectNum; i++) {
            int base_id = base_project(i);
            if (base_id >= 0 && Bases[base_id].faction_id_current == faction_id) {
                total++;
            }
        }
    }
    // One recursion deep only: the pact partners are asked with the flag off.
    if (count_pact_factions && (GameRules & RULES_VICTORY_COOPERATIVE)) {
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
    for (int i = 0; i < BaseCurrentCount; i++) {
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
    if (!facility_id || (facility_id == FAC_SKUNKWORKS && DiffLevelCurrent <= DLVL_SPECIALIST)
        || (facility_id >= FAC_HUMAN_GENOME_PROJ && GameRules & RULES_SCN_NO_BUILDING_SP)) {
        return false; // Skunkworks removed if there are no prototype costs
    }
    if (facility_id == FAC_ASCENT_TO_TRANSCENDENCE) { // at top since anyone can build it
        return ascending(faction_id) && GameRules & RULES_VICTORY_TRANSCENDENCE
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
        int count = num_objectives(faction_id, GameRules & RULES_VICTORY_COOPERATIVE);
        if (count >= best_count) {
            if (pact_count_out != 0) {
                if (count > best_count) {
                    *pact_count_out = 0;
                } else if ((PlayersData[faction_id].diplo_treaties[best_faction] & DTREATY_PACT) == 0
                           || (GameRules & RULES_VICTORY_COOPERATIVE) == 0) {
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
    if (((1 << faction_id) & (int)(unsigned char)LockEnableMask) == 0
        || (GameMorePreferences & 0x100000) != 0) {
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
    if (*(unsigned char *)&GameRules & 0x40) {
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

    if (BaseCurrentCount > 0) {
        Base *base = Bases;
        int j = BaseCurrentCount;
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

    if ((*(unsigned char *)&LockEnableMask & (1 << faction_id)) != 0) {
        draw_map(1);
    }
    if (faction_id == ConsoleGlobal.faction_id_) {
        ConsoleGlobal.update_data(0);
    }
}

// Original score calculation clears all eight weights, then publishes the
// aggregate after dividing the transcendence bonus among faction populations.
// RECOVERY_OBJECT: v2 0x009A67DC 0x009A67E0 ?TranscendenceScoreBonus@@3HA 0x005ACBE0
int TranscendenceScoreBonus;
// RECOVERY_OBJECT: v2 0x009A67E0 0x009A6800 ?TranscendenceScoreWeights@@3PAHA 0x005ACBE0
int TranscendenceScoreWeights[8];

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
    if ((GameState & 8) && !recursive) return;
    if (!total) total = &PlayersData[faction].unk_109;
    if (!items) items = PlayersData[faction].score_items_;
    *total = 0;
    if (!recursive) PlayersData[faction].unk_107 = 0;
    for (int item = 0; item < 9; ++item) items[item] = 0;

    for (int base = 0; base < BaseCurrentCount; ++base) {
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
            } else if ((treaty & 1) && (GameRules & 0x1000)) {
                items[0] = score_word_add(items[0], Bases[base].population_size / 2);
            }
        } else if (!(PlayersData[faction].diplo_treaties[owner] & 1) || !(GameRules & 0x1000)) {
            items[0] = score_word_add(items[0], Bases[base].population_size / 2);
        } else items[0] = score_word_add(items[0], Bases[base].population_size);
    }
    for (int technology = 0; technology < 89; ++technology)
        if (has_tech(technology, faction)) items[2] = score_word_add(items[2], 1);
    items[3] = score_word_mul(PlayersData[faction].theory_of_everything, 10);
    int objective_points = static_cast<int>(ObjectiveAchievePts);
    if (objective_points < 1) objective_points = 1;
    items[6] = score_word_mul(num_objectives(faction, GameRules & 0x1000), objective_points);
    for (int project = 0; project < 64; ++project) {
        const int base = base_project(project);
        if (base >= 0 && Bases[base].faction_id_current == faction)
            items[4] = score_word_add(items[4], 25);
    }

    TranscendenceScoreBonus = 0;
    for (int slot = 0; slot < 8; ++slot) TranscendenceScoreWeights[slot] = 0;
    const int transcendence_base = SecretProject.ascent_to_transcendence;
    if (transcendence_base >= 0) {
        TranscendenceScoreBonus = 300;
        const int scaled = score_word_mul(score_word_sub(1000, TurnCurrentNum), 2);
        if (scaled >= 300) TranscendenceScoreBonus = scaled;
        int own_weight = 0;
        int aggregate = 0;
        int leader = -1;
        for (int candidate = 1; candidate < 8; ++candidate) {
            if (Bases[transcendence_base].faction_id_current == candidate) {
                leader = candidate;
                aggregate = score_word_mul(PlayersData[candidate].pop_total, 2);
                TranscendenceScoreWeights[candidate] = aggregate;
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
                TranscendenceScoreWeights[other] = weight;
                if (other == faction) own_weight = score_word_add(own_weight, weight);
            }
        }
        items[6] = score_word_mul(TranscendenceScoreBonus, own_weight) / aggregate;
        // Original publishes aggregate only AFTER the signed division.
        TranscendenceScoreWeights[0] = aggregate;
    }

    int diplomatic_bonus = score_word_mul(score_word_sub(500, TurnCurrentNum), 2);
    items[7] = diplomatic_bonus < 0 ? 0 : diplomatic_bonus;
    if (!(GameRules & 2)) items[7] = 0;
    if ((GameState & 0x2000) && (GameRules & 2) && items[7]) {
        int matching_population = 0;
        int total_population = 0;
        for (int base = 0; base < BaseCurrentCount; ++base) {
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
            int bonus = score_word_mul(score_word_sub(600, TurnCurrentNum), 2);
            if (bonus < 200) bonus = 200;
            items[7] = bonus;
        } else if (flags & 0x40000) {
            int bonus = score_word_mul(score_word_sub(600, TurnCurrentNum), 2);
            if (bonus < 200) bonus = 200;
            items[7] = bonus;
        } else if (flags & 0x100000) {
            int bonus = score_word_mul(score_word_sub(1000, TurnCurrentNum), 2);
            if (bonus < 300) bonus = 300;
            items[7] = bonus;
        } else {
            for (int other = 1; other < 8; ++other) {
                if (PlayersData[other].flags & 0x20000) {
                    if ((PlayersData[faction].diplo_treaties[other] & 1) && (GameRules & 0x1000)) {
                        int bonus = score_word_sub(600, TurnCurrentNum);
                        if (bonus < 100) bonus = 100;
                        items[7] = bonus;
                    }
                } else if (PlayersData[other].flags & 0x40000) {
                    if ((PlayersData[faction].diplo_treaties[other] & 1) && (GameRules & 0x1000)) {
                        int bonus = score_word_sub(600, TurnCurrentNum);
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

    int required = ObjectiveReqVictory;
    if ((required < 9000 || static_cast<int>(ObjectivesSuddenDeathVictory) < 9000)
        && !(GameState & 0x80000)) {
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
            if (num_objectives(faction, GameRules & 0x1000) >= best)
                items[7] = static_cast<int>(VictoryAchieveBonusPts);
        } else {
            const int mine = num_objectives(faction, GameRules & 0x1000);
            required = ObjectiveReqVictory; // Original re-reads after call.
            if (mine >= required) items[7] = static_cast<int>(VictoryAchieveBonusPts);
        }
    }
    for (int total_item = 0; total_item < 9; ++total_item)
        *total = score_word_add(*total, items[total_item]);
    if (MapNativeLifeForms != 1 && !(GameState & 0x10)) {
        const int difference = score_word_mul(score_word_sub(static_cast<int>(MapNativeLifeForms), 1), *total) / 4;
        items[5] = difference;
        *total = score_word_add(*total, difference);
    }
    if (GameState & 0x80000) {
        if (recursive) return;
        int best = 0;
        int other_total;
        for (int other = 1; other < 8; ++other) {
            int other_items[9];
            compute_score(other, &other_total, other_items, 1);
            if (other_total > best) best = other_total;
        }
        if (*total >= best) {
            int bonus = static_cast<int>(VictoryAchieveBonusPts);
            if (!bonus) bonus = 1000;
            items[7] = bonus; // Original does NOT add this late bonus to total.
        }
    } else if (recursive) return;
    PlayersData[faction].unk_107 = *total;
    if ((GameRules & 0x400) && !(GameState & 0x20))
        PlayersData[faction].unk_107 = score_word_mul(*total, 2);
}

// Unfinished dependency of the airdrop recovery. The complete original seed
// remains in src/unrecovered/0050c510.cpp; this body provides link continuity
// only and must not be counted as recovered capture behavior.
// ORIGINAL: 0x0050C510 ?capture_base@@YAXHHH@Z 0x0050C510-0x0050DD8F
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/0050c510.cpp retained original signature and extent; full seed preserved", "Coordinator user authorizes explicit unfinished empty bodies for new referenced functions, never replacement of recovered behavior"], "reason": "Explicit unfinished owning-TU link-continuity dependency for full action_airdrop authoring; BODY GOES HERE preserves unknown behavior and is not a recovery or matching claim."}
void __cdecl capture_base(int base_id, int faction_id, int mode) {
    // BODY GOES HERE.
}

// ORIGINAL: 0x004EC3F0 ?base_connect@@YAHH@Z 0x004EC3F0-0x004ECB84
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/004ec3f0.cpp original signature and complete span; independent pinned instruction review corrected magtube exception and destination population comparison; actual-source route and sensor controls."], "reason": "Base-domain connection planner follows same-region base paths and sensor fallback using existing typed Base, Map and Path owners."}
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

    for (int base_id = 0; base_id < BaseCurrentCount; ++base_id) {
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
                for (int other_base_id = 0; other_base_id < BaseCurrentCount;
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
        if ((LockEnableMask & (1u << (faction & 0x1F))) != 0 &&
            (GameMorePreferences & MPREF_AUTO_FORMER_BUILD_SENSORS) == 0)
            return 0;
        if (Paths.sensors(faction, &destination_x, &destination_y) != 0) return 0;
    } else {
        if (found_connection &&
            ((LockEnableMask & (1u << (faction & 0x1F))) == 0 ||
             (GameMorePreferences & MPREF_AUTO_FORMER_BUILD_SENSORS) != 0)) {
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
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/004ecbf0.cpp retained full native span", "Pinned 4ECBF0..4EEF61 instruction review establishes BaseIDCurrentSelected forced-improvement timer and pre-filter improvement-presence latches", "recovery/tests/test_base_terraform_behavior.py actual-source behavioral controls"], "reason": "Base-domain terraforming planner uses genuine Base, Map, Path and faction owners for tile scoring, improvement goals, forced actions and relocation. Historical seed and original extent remain preserved; source translation is not a byte-exact or runtime claim."}
void __cdecl base_terraform(int force_improve) {
    Base &base = *BaseCurrent;
    const int faction = base.faction_id_current;
    const uint32_t faction_bit = 1u << (faction & 0x1F);
    // The native tests only the low byte of the mask at both use sites.
    const bool human = (static_cast<uint8_t>(LockEnableMask) &
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
                for (int other_id = 0; other_id < BaseCurrentCount; ++other_id) {
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
            (static_cast<uint8_t>(LockEnableMask) &
             static_cast<uint8_t>(faction_bit)) != 0 &&
            (GameMorePreferences & 0x20u) == 0;
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
                    (static_cast<uint8_t>(LockEnableMask) &
                     static_cast<uint8_t>(faction_bit)) != 0 &&
                    (GameMorePreferences & 0x00400000u) != 0;
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
        (static_cast<uint8_t>(LockEnableMask) &
         static_cast<uint8_t>(faction_bit)) == 0 ||
        (GameMorePreferences & 0x00400000u) == 0;
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
                    TurnCurrentNum >= 0xC9)
                    thermal_site_allowed = true;
            } else {
                thermal_site_allowed = true;
            }
        }
    }
    const bool thermal_goal_allowed = thermal_site_allowed &&
        terrain_avail(TERRA_THERMAL_BORE, FALSE, faction) != 0 &&
        ((static_cast<uint8_t>(LockEnableMask) &
          static_cast<uint8_t>(faction_bit)) == 0 ||
         (faction == ConsoleGlobal.faction_id_ &&
          (GamePreferences & 0x02000000u) != 0));

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
                     TurnCurrentNum > 200))
                    condenser_site_allowed = true;
            }
        }
    }
    const bool condenser_goal_allowed = condenser_site_allowed &&
        terrain_avail(TERRA_CONDENSER, FALSE, faction) != 0 &&
        ((static_cast<uint8_t>(LockEnableMask) &
          static_cast<uint8_t>(faction_bit)) == 0 ||
         (faction == ConsoleGlobal.faction_id_ &&
          (GamePreferences & 0x02000000u) != 0));

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
                 (GameMorePreferences & 0x00020000u) == 0) ||
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
