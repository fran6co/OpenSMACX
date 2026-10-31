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
#include "general.h"
#include "veh.h"
#include "faction.h"
#include "game.h"

// The private prototype-history owner keeps its storage and readers/writers
// together. The original source filename is unknown. The predicate is called
// out of line from propose_proto5808B3, which tests full EAX at5808BB; both
// original predicate exits define full EAX.

// The faction prototype-design list. 0x0094B558, eight factions x 64 entries x
// 8 bytes - one entry is VehPrototype's ability_flags dword followed by its
// chassis/weapon/armor/reactor bytes, which is exactly the 8 bytes
// 0x0057DF30 copies in. clear_bunglist (0x0057DF00) fills a faction's 64 with
// -1 then 0xFF, which is why the comparison bytes sit at +4..+7 and the dword
// at +0.
struct ProtoBung {
    uint32_t ability_flags;
    uint8_t chassis_id;
    uint8_t weapon_id;
    uint8_t armor_id;
    uint8_t reactor_id;
};
static_assert(sizeof(ProtoBung) == 8, "Prototype history record stride");
static_assert(offsetof(ProtoBung, chassis_id) == 4,
              "Prototype history component offset");
// Eight faction histories, 64 records each. Initial loaded storage is zero.
// Native clear57DF00 uses base94B558+5 then stores at -5..+2; the former
// address alias started five bytes too early and corrupted the preceding data.
// RECOVERY_OBJECT: v2 0x0094B558 0x0094C558 _BungList 0x0057DF00
static ProtoBung BungList[8][64];

bool veh_bung_copy_chunk(size_t offset, unsigned char *packet, size_t count,
                         bool packet_to_owner) {
    if (count == 0) return true;
    unsigned char *owner = reinterpret_cast<unsigned char *>(&BungList);
    const uint32_t native_offset = static_cast<uint32_t>(offset);
    if (packet_to_owner) memmove(owner + native_offset, packet, count);
    else memmove(packet, owner + native_offset, count);
    return true;
}

/*
Purpose: Empty one faction's prototype-design run.
// ORIGINAL: 0x0057DF00 ?clear_bunglist@@YAXH@Z 0x0057DF00-0x0057DF2F BYTE_EXACT
// size      47 bytes
// prototype void (__cdecl ?clear_bunglist@@YAXH@Z)(int factionID)
// callers   3   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// HOMED from src/recovered/0057df00.cpp, 2026-09-04 - the first of this
// file's archive-only claims to move into the build. The scaffold's
// `entry[-5]` / `entry[-1]` byte arithmetic expects a pointer to the weapon
// byte (+5), not the record base. Typed members express the actual stores.
Return Value: n/a
Historical byte claim remains a restoration obligation. The typed owner fixes
the former reference-target error; a masked-byte match did not detect it.
*/
void __cdecl clear_bunglist(int faction_id) {
    uintptr_t cursor = reinterpret_cast<uintptr_t>(&BungList[faction_id][0].weapon_id);
    int count = 0x40;
    do {
        *reinterpret_cast<uint32_t *>(cursor - (offsetof(ProtoBung, weapon_id) - offsetof(ProtoBung, ability_flags))) = 0xffffffffu;
        *reinterpret_cast<uint8_t *>(cursor - (offsetof(ProtoBung, weapon_id) - offsetof(ProtoBung, chassis_id))) = 0xff;
        *reinterpret_cast<uint8_t *>(cursor) = 0xff;
        *reinterpret_cast<uint8_t *>(cursor + (offsetof(ProtoBung, armor_id) - offsetof(ProtoBung, weapon_id))) = 0xff;
        *reinterpret_cast<uint8_t *>(cursor + (offsetof(ProtoBung, reactor_id) - offsetof(ProtoBung, weapon_id))) = 0xff;
        cursor += sizeof(ProtoBung);
    } while (--count);
}

int __cdecl veh_game_data_bunglist(FILE *file, int format_version) {
    if (format_version >= 6) {
        return game_data_transfer_block(file, &BungList[0][0], sizeof(BungList));
    }
    for (int faction = 0; faction < 8; ++faction) clear_bunglist(faction);
    return 1;
}

/*
Purpose: Record a prototype design in the faction's run.
// ORIGINAL: 0x0057DF30 sub_57df30 0x0057DF30-0x0057DFB2 BYTE_EXACT
// size      130 bytes
// prototype void (__cdecl sub_57df30)(int factionID, int protoID)
// symbol    ?proto_bung_put@@YAXHH@Z
//           The image's catalogue name is an uninformative `sub_`; this tree
//           names what the body does. Renames update this fact, not the bytes.
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// HOMED from src/unrecovered/0057df30.cpp, 2026-09-04.
// TRIED: a separate pointer variable for the RMW counter slot ('mov' then a
//        stray 'lea' to materialise the address) - indexing the array
//        directly drops the divergence from #10 to #22. MISMATCH remains open.
Return Value: n/a
Status: MISMATCH.
*/
void __cdecl proto_bung_put(int faction_id, int proto_id) {
    const int slot = PlayersData[faction_id].prototype_history_cursor;
    PlayersData[faction_id].prototype_history_cursor = (slot + 1) % 64;
    BungList[faction_id][slot].ability_flags = VehPrototypes[proto_id].ability_flags;
    BungList[faction_id][slot].chassis_id = VehPrototypes[proto_id].chassis_id;
    BungList[faction_id][slot].weapon_id = VehPrototypes[proto_id].weapon_id;
    BungList[faction_id][slot].armor_id = VehPrototypes[proto_id].armor_id;
    BungList[faction_id][slot].reactor_id = VehPrototypes[proto_id].reactor_id;
}

/*
Purpose: Was this exact prototype design already recorded for this faction?
// ORIGINAL: 0x0057DFC0 ?is_bunged@@YA_NHHHHHH@Z 0x0057DFC0-0x0057E037 BYTE_EXACT
// symbol    ?is_bunged@@YAHHHHHHH@Z
// size      119 bytes
// prototype bool (__cdecl ?is_bunged@@YA_NHHHHHH@Z)(int factionID, int chassisID, int weaponID, int armorID, int abilityFlags, int reactorID)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// HOMED from src/unrecovered/0057dfc0.cpp, 2026-09-04.
Return Value: 1 when the faction's design run already holds the tuple.
Status: MISMATCH.
*/
int __cdecl is_bunged(int faction_id, int chassis_id, int weapon_id,
                       int armor_id, int ability_flags, int reactor_id) {
    if ((1u << faction_id) & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_) {
        int i = 0;
        for (; i < 0x40; ++i) {
            const ProtoBung &entry = BungList[faction_id][i];
            if (entry.chassis_id == chassis_id && entry.weapon_id == weapon_id &&
                entry.armor_id == armor_id &&
                entry.ability_flags == (uint32_t)ability_flags &&
                entry.reactor_id == reactor_id) {
                return true;
            }
        }
    }
    return false;
}
