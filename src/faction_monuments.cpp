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
#include "game.h"
#include "game_data_state_window.h"
#include "faction.h"
#include "console.h"
#include <string.h>

void __cdecl monument(int from_load);

// Eight faction blocks begin at94CA08, with21records at38 stride and
// latest-type at+498. Founding/secrets/victory consumers establish the
// natural container; reset's previous94CA18 view began inside record0.
// RECOVERY_OBJECT: v2 0x0094CA08 0x0094F1A8 ?FactionMonuments@@3PAUMonumentFaction@@A 0x00476DA0
MonumentFaction FactionMonuments[MaxPlayerNum];

// ORIGINAL: 0x00476B30 ?clear_monuments@@YAXXZ 0x00476B30-0x00476B61 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned476B30 starts its writes at94CA18 with38stride in8faction rows4F4apart; founding476B70 and secrets476DA0 establish containing-record base94CA08", "recovery/tests/test_setup_dependencies_behavior.py checks everybyte against the exact original22word mutation footprint", "git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/00476b30.cpp"], "reason": "Reset the shared natural monument container: clear21achieved words atrecord+10 andlatest_type atrow+498, preserving all other bytes. Historical reset-only shifted interpretation remains in Git/task history; original seed and restoration obligations are intact. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
void __cdecl clear_monuments() {
    intptr_t latest = reinterpret_cast<intptr_t>(FactionMonuments)
                    + offsetof(MonumentFaction, latest_type_);
    const intptr_t end = latest + sizeof(FactionMonuments);
    const uintptr_t achieved_offset =
        offsetof(MonumentFaction, latest_type_)
        - offsetof(MonumentFaction, records_)
        - offsetof(MonumentRecord, achieved_);
    do {
        uintptr_t achieved = static_cast<uintptr_t>(latest) - achieved_offset;
        int slots = 21;
        do {
            *reinterpret_cast<int *>(achieved) = 0;
            achieved += sizeof(MonumentRecord);
        } while (--slots);
        *reinterpret_cast<int *>(latest) = -1;
        latest += sizeof(MonumentFaction);
    } while (latest < end);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00476B70 ?mon_colony_founded@@YAXHPAD@Z 0x00476B70-0x00476C84 FILE
// RECOVERY_BINDING: v1 {"evidence": ["image:terranx_original.exe:0x00476B70-0x00476C84", "source:src/recovered/units/00476b70.cpp", "Original faction stride4F4, record0 flag/first-owner words and name copy to4AC determine existing type ownership; preserve24byte strncpy behavior."], "reason": "Recover first colony founding in the shared genuine FactionMonuments owner, including first-achiever selection, the bounded faction-owned base-name field and live local notification."}
void __cdecl mon_colony_founded(int faction_id, char *base_name) {
    const int year = static_cast<int>(game_year(GameDataStateStorage.fields_.turn_current_num_));
    MonumentRecord &record = FactionMonuments[faction_id].records_[0];

    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0 || record.achieved_ != 0) {
        return;
    }

    record.word_14_ = 0;
    for (int other = 0; other < 8; ++other) {
        if (other != faction_id &&
            FactionMonuments[other].records_[0].achieved_ != 0) {
            ++record.word_14_;
        }
    }

    int owner_faction;
    if (record.word_14_ != 0) {
        record.word_14_ = 0;
        owner_faction = faction_id;
        for (int other = 0; other < 8; ++other) {
            const MonumentRecord &other_record =
                FactionMonuments[other].records_[0];
            if (other != faction_id && other_record.achieved_ != 0 &&
                other_record.word_14_ != 0) {
                owner_faction = other;
            }
        }
    } else {
        record.word_14_ = 1;
        owner_faction = faction_id;
    }

    record.owner_faction_ = owner_faction;
    record.achieved_ = 1;
    record.year_ = year;
    // Native strncpy copies exactly 24 bytes; a full-length name is not
    // forcibly terminated here. The field remains owned by this faction.
    strncpy(FactionMonuments[faction_id].founding_base_name_, base_name,
            sizeof(FactionMonuments[faction_id].founding_base_name_));
    const int local_faction = ConsoleGlobal.faction_id_;
    FactionMonuments[faction_id].latest_type_ = 0;
    if (faction_id == local_faction) {
        monument(1);
    }
}

// ORIGINAL: 0x00476C90 ?mon_tech_discovered@@YAXHH@Z 0x00476C90-0x00476D93
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original00476C90 and seed git:91b86557299ab66a61f1184540338d569e780e7e:src/recovered/units/00476c90.cpp; record1 at factionstride4F4, year0, payload8, firstownerC, achieved10, firstflag14 and latesttype498 are independently established by native stores.", "The original calls game_year before alien/duplicate guards and includes all eight peer records regardless of peer flags; later qualifying first-achiever ties win."], "reason": "Recover the complete native milestone producer against genuine shared monument records, with typed peer selection and live local notification rather than executable-address table views."}
void __cdecl mon_tech_discovered(int faction_id, int tech_id) {
    const int year = static_cast<int>(game_year(GameDataStateStorage.fields_.turn_current_num_));
    MonumentRecord &record = FactionMonuments[faction_id].records_[1];

    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0 || record.achieved_ != 0) {
        return;
    }

    record.word_14_ = 0;
    for (int other = 0; other < 8; ++other) {
        if (other != faction_id &&
            FactionMonuments[other].records_[1].achieved_ != 0) {
            ++record.word_14_;
        }
    }

    int owner_faction;
    if (record.word_14_ != 0) {
        record.word_14_ = 0;
        owner_faction = faction_id;
        for (int other = 0; other < 8; ++other) {
            const MonumentRecord &other_record =
                FactionMonuments[other].records_[1];
            if (other != faction_id && other_record.achieved_ != 0 &&
                other_record.word_14_ != 0) {
                owner_faction = other;
            }
        }
    } else {
        record.word_14_ = 1;
        owner_faction = faction_id;
    }

    record.achieved_ = 1;
    record.year_ = year;
    record.owner_faction_ = owner_faction;
    record.word_8_ = tech_id;
    FactionMonuments[faction_id].latest_type_ = 1;
    if (faction_id == ConsoleGlobal.faction_id_) {
        monument(1);
    }
}

// ORIGINAL: 0x00477100 ?mon_secret_project@@YAXHH@Z 0x00477100-0x00477207
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original00477100 and seed git:91b86557299ab66a61f1184540338d569e780e7e:src/recovered/units/00477100.cpp; record5 at factionstride4F4, year0, payload8, firstownerC, achieved10, firstflag14 and latesttype498 are independently established by native stores.", "The original calls game_year before alien/duplicate guards and includes all eight peer records regardless of peer flags; later qualifying first-achiever ties win."], "reason": "Recover the complete native milestone producer against genuine shared monument records, with typed peer selection and live local notification rather than executable-address table views."}
void __cdecl mon_secret_project(int faction_id, int project_id) {
    const int year = static_cast<int>(game_year(GameDataStateStorage.fields_.turn_current_num_));
    MonumentRecord &record = FactionMonuments[faction_id].records_[5];

    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0 || record.achieved_ != 0) {
        return;
    }

    record.word_14_ = 0;
    for (int other = 0; other < 8; ++other) {
        if (other != faction_id &&
            FactionMonuments[other].records_[5].achieved_ != 0) {
            ++record.word_14_;
        }
    }

    int owner_faction;
    if (record.word_14_ != 0) {
        record.word_14_ = 0;
        owner_faction = faction_id;
        for (int other = 0; other < 8; ++other) {
            const MonumentRecord &other_record =
                FactionMonuments[other].records_[5];
            if (other != faction_id && other_record.achieved_ != 0 &&
                other_record.word_14_ != 0) {
                owner_faction = other;
            }
        }
    } else {
        record.word_14_ = 1;
        owner_faction = faction_id;
    }

    record.achieved_ = 1;
    record.year_ = year;
    record.owner_faction_ = owner_faction;
    record.word_8_ = project_id;
    FactionMonuments[faction_id].latest_type_ = 5;
    if (faction_id == ConsoleGlobal.faction_id_) {
        monument(1);
    }
}

// ORIGINAL: 0x00477840 ?mon_built_preserve@@YAXH@Z 0x00477840-0x0047793E
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original00477840 and seed git:91b86557299ab66a61f1184540338d569e780e7e:src/unrecovered/00477840.cpp; record12 at factionstride4F4, year0, firstownerC, achieved10, firstflag14 and latesttype498 are independently established by native stores.", "The original calls game_year before alien/duplicate guards and includes all eight peer records regardless of peer flags; later qualifying first-achiever ties win."], "reason": "Recover the complete native milestone producer against genuine shared monument records, with typed peer selection and live local notification rather than executable-address table views."}
// TRIED: single `if(!(flag)&&active==0)` short-circuit form vs hoisting `winner`/`count=0` before the scan loop - both land the same MISMATCH #3 (original reserves the stack slot for `year` with `push ecx` ahead of the ebx/esi pushes; every C form tried here has the compiler push ebx/esi first instead).
void __cdecl mon_built_preserve(int faction_id) {
    const int year = static_cast<int>(game_year(GameDataStateStorage.fields_.turn_current_num_));
    MonumentRecord &record = FactionMonuments[faction_id].records_[12];

    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0 || record.achieved_ != 0) {
        return;
    }

    record.word_14_ = 0;
    for (int other = 0; other < 8; ++other) {
        if (other != faction_id &&
            FactionMonuments[other].records_[12].achieved_ != 0) {
            ++record.word_14_;
        }
    }

    int owner_faction;
    if (record.word_14_ != 0) {
        record.word_14_ = 0;
        owner_faction = faction_id;
        for (int other = 0; other < 8; ++other) {
            const MonumentRecord &other_record =
                FactionMonuments[other].records_[12];
            if (other != faction_id && other_record.achieved_ != 0 &&
                other_record.word_14_ != 0) {
                owner_faction = other;
            }
        }
    } else {
        record.word_14_ = 1;
        owner_faction = faction_id;
    }

    record.achieved_ = 1;
    record.owner_faction_ = owner_faction;
    record.year_ = year;
    FactionMonuments[faction_id].latest_type_ = 12;
    if (faction_id == ConsoleGlobal.faction_id_) {
        monument(1);
    }
}

// ORIGINAL: 0x00476FE0 ?mon_facility_built@@YAXHPAD@Z 0x00476FE0-0x004770F2
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original 00476FE0 and seed git:fc8112d1ca80fbe743dd051511ad7e0af8454e94:src/recovered/units/00476fe0.cpp; faction stride4F4, record4, name field4C4 and24byte strncpy are established by the complete native body.", "Native first-achiever selection includes all eight peer factions; alien target and duplicate events return without updates. Local notification reads the live ConsoleGlobal owner after game_year and name copying."], "reason": "Recover the native milestone and its bounded base-name storage in the genuine shared FactionMonuments owner, not a duplicate raw table view."}
void __cdecl mon_facility_built(int faction_id, char *base_name) {
    const int year = static_cast<int>(game_year(GameDataStateStorage.fields_.turn_current_num_));
    MonumentRecord &record = FactionMonuments[faction_id].records_[4];

    if (record.achieved_ != 0 || (Players[faction_id].rule_flags & RFLAG_ALIEN) != 0) {
        return;
    }

    record.word_14_ = 0;
    for (int other = 0; other < 8; ++other) {
        if (other != faction_id &&
            FactionMonuments[other].records_[4].achieved_ != 0) {
            ++record.word_14_;
        }
    }

    int owner_faction;
    if (record.word_14_ != 0) {
        record.word_14_ = 0;
        owner_faction = faction_id;
        for (int other = 0; other < 8; ++other) {
            const MonumentRecord &other_record =
                FactionMonuments[other].records_[4];
            if (other != faction_id && other_record.achieved_ != 0 &&
                other_record.word_14_ != 0) {
                owner_faction = other;
            }
        }
    } else {
        record.word_14_ = 1;
        owner_faction = faction_id;
    }

    record.owner_faction_ = owner_faction;
    record.achieved_ = 1;
    record.year_ = year;
    // Native copies exactly 24 bytes, including strncpy's zero padding but
    // without forcing a terminator for a full-length name.
    strncpy(FactionMonuments[faction_id].facility_base_name_, base_name,
            sizeof(FactionMonuments[faction_id].facility_base_name_));
    const int local_faction = ConsoleGlobal.faction_id_;
    FactionMonuments[faction_id].latest_type_ = 4;
    if (faction_id == local_faction) {
        monument(1);
    }
}

// ORIGINAL: 0x00477320 ?mon_conquer_base@@YAXHPAD@Z 0x00477320-0x00477434
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original 00477320 and seed git:fc8112d1ca80fbe743dd051511ad7e0af8454e94:src/recovered/units/00477320.cpp; faction stride4F4, record7, name field4DC and24byte strncpy are established by the complete native body.", "Native first-achiever selection includes all eight peer factions; alien target and duplicate events return without updates. Local notification reads the live ConsoleGlobal owner after game_year and name copying."], "reason": "Recover the native milestone and its bounded base-name storage in the genuine shared FactionMonuments owner, not a duplicate raw table view."}
void __cdecl mon_conquer_base(int faction_id, char *base_name) {
    const int year = static_cast<int>(game_year(GameDataStateStorage.fields_.turn_current_num_));
    MonumentRecord &record = FactionMonuments[faction_id].records_[7];

    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0 || record.achieved_ != 0) {
        return;
    }

    record.word_14_ = 0;
    for (int other = 0; other < 8; ++other) {
        if (other != faction_id &&
            FactionMonuments[other].records_[7].achieved_ != 0) {
            ++record.word_14_;
        }
    }

    int owner_faction;
    if (record.word_14_ != 0) {
        record.word_14_ = 0;
        owner_faction = faction_id;
        for (int other = 0; other < 8; ++other) {
            const MonumentRecord &other_record =
                FactionMonuments[other].records_[7];
            if (other != faction_id && other_record.achieved_ != 0 &&
                other_record.word_14_ != 0) {
                owner_faction = other;
            }
        }
    } else {
        record.word_14_ = 1;
        owner_faction = faction_id;
    }

    record.owner_faction_ = owner_faction;
    record.achieved_ = 1;
    record.year_ = year;
    strncpy(FactionMonuments[faction_id].conquered_base_name_, base_name,
            sizeof(FactionMonuments[faction_id].conquered_base_name_));
    const int local_faction = ConsoleGlobal.faction_id_;
    FactionMonuments[faction_id].latest_type_ = 7;
    if (faction_id == local_faction) {
        monument(1);
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00476DA0 ?mon_secrets_of_tech@@YAXH@Z 0x00476DA0-0x00476EDD
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00476da0.cpp preserved original identity/extent; pinned instructions and independent family review establish317byte body and fields; actual-source milestone/reset controls verify shared ownership and native branches"], "reason": "Record first/repeated technology secrets in genuine shared monument record2, retain first-achiever peer interpretation and notify the live local faction."}
void __cdecl mon_secrets_of_tech(int faction_id) {
    const int year = static_cast<int>(game_year(GameDataStateStorage.fields_.turn_current_num_));
    MonumentRecord &record = FactionMonuments[faction_id].records_[2];
    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0)
        return;

    if (record.achieved_ != 0) {
        if (record.word_8_ + 1 != 0) {
            if (record.word_4_ + 1 == 0)
                record.word_4_ = year;
        } else {
            record.word_8_ = year;
        }
    } else {
        record.word_14_ = 0;
        int peer;
        for (peer = 0; peer < MaxPlayerNum; ++peer) {
            if (peer != faction_id &&
                FactionMonuments[peer].records_[2].achieved_ != 0)
                ++record.word_14_;
        }

        int first_faction = faction_id;
        if (record.word_14_ != 0) {
            record.word_14_ = 0;
            for (peer = 0; peer < MaxPlayerNum; ++peer) {
                const MonumentRecord &other = FactionMonuments[peer].records_[2];
                if (peer != faction_id && other.achieved_ != 0 &&
                    other.word_14_ != 0)
                    first_faction = peer;
            }
        } else {
            record.word_14_ = 1;
        }
        record.owner_faction_ = first_faction;
        record.year_ = year;
        record.achieved_ = 1;
        record.word_8_ = -1;
        record.word_4_ = -1;
    }
    FactionMonuments[faction_id].latest_type_ = 2;
    if (faction_id == ConsoleGlobal.faction_id_)
        monument(1);
}

// ORIGINAL: 0x00477210 ?mon_enemy_destroyed@@YAXHH@Z 0x00477210-0x00477320 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:monument-enemy-destroyed-owner original identity, spans and historical claim retained from git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/00477210.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:477210..477320 complete body; record6 stores at 94CB58/60/64/68/6C, latest_type at 94CEA0 and notification after updates.", "source:src/game.h MonumentRecord stride38, MonumentFaction stride4F4 and source bindings; game_year before all guards; Players rule_flags bit80 is alien, not unrelated game state.", "git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/00477210.cpp"], "reason": "Recover complete enemy-destruction milestone with genuine FactionMonuments, Players and live ConsoleGlobal ownership. Preserve target-faction fallback when prior achievements lack a first flag; later qualifying peer ties win. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// symbol    ?mon_enemy_destroyed@@YAXHH@Z
// kind      game
void __cdecl mon_enemy_destroyed(int faction_id, int target_faction) {
    const int year = static_cast<int>(game_year(GameDataStateStorage.fields_.turn_current_num_));
    MonumentRecord &record = FactionMonuments[faction_id].records_[6];

    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0 ||
        record.achieved_ != 0 || target_faction == 0) {
        return;
    }

    record.word_14_ = 0;
    for (int other = 0; other < 8; ++other) {
        if (other != faction_id &&
            FactionMonuments[other].records_[6].achieved_ != 0) {
            ++record.word_14_;
        }
    }

    int owner_faction;
    if (record.word_14_ != 0) {
        record.word_14_ = 0;
        owner_faction = target_faction;
        for (int other = 0; other < 8; ++other) {
            const MonumentRecord &other_record =
                FactionMonuments[other].records_[6];
            if (other != faction_id && other_record.achieved_ != 0 &&
                other_record.word_14_ != 0) {
                owner_faction = other;
            }
        }
    } else {
        record.word_14_ = 1;
        owner_faction = faction_id;
    }

    record.achieved_ = 1;
    record.year_ = year;
    record.owner_faction_ = owner_faction;
    record.word_8_ = target_faction;
    FactionMonuments[faction_id].latest_type_ = 6;
    if (faction_id == ConsoleGlobal.faction_id_) {
        monument(1);
    }
}

// These first-built triggers select individual records in FactionMonuments.
// Their latest-type word belongs to the faction block, not to a fabricated
// per-type overlay extending across unrelated records.
// The same event owner also receives victory and faction-loss notifications.

/*
Purpose: Record the local faction's first built prototype (the mon type-3
         trigger): latch the year, count the rival factions that already
         hold this monument, decide the builder, then open the monument
         window when it was the local faction's own build.
// size      254 bytes
Status: Coverage transcription of the retained seed
        (git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00476ee0.cpp, plain FILE). The seed's TRIED wall
        stands: the original reserves the year local with `push ecx` ahead
        of the callee-save pushes, and the C forms tried all save
        ebx/esi first. rule_flags is read as the typed dword here where
        the image tests the byte at 0x00946F58 + f*0x59C; bit 0x80 is the
        same test. game_year expands from its MEASURED inline in game.h
        where the image calls 0x005C89A0 - the accepted mon_winning_*
        house form above.
*/
// ORIGINAL: 0x00476EE0 ?mon_prototype_built@@YAXH@Z 0x00476EE0-0x00476FDE
// RECOVERY_BINDING: v1 {"evidence":["Pinned original entry 0x00476EE0 and its complete recorded span identify mon_prototype_built.","src/game.h defines MonumentRecord/MonumentFaction and the shared FactionMonuments[8] object; the body uses those typed record/faction owners rather than a second table view.","Existing src/game.cpp milestone producers already own neighboring records 0,1,2,4,5,6,7,12; this event family fills record types 3,8,9,10,11,13,14,15..20 on the same owner.","Actual callers remain in src/base.cpp (build/turn events), src/faction.cpp (faction death), and src/game.cpp (unification); notification calls resolve through the existing monument UI API.","Historical seed comments and unclaimed/non-byte-exact status are retained; no new byte or semantic claim is made."],"reason":"Keep this milestone producer with the existing source-owned FactionMonuments event family. Retain the exact branch order, record writes, unchecked faction indices, current-local-faction notification timing, and original matching/restoration obligations."}
// TRIED: MonRecord struct + reinterpret_cast<MonRecord*>(base)+a1 array indexing reproduces every instruction from the flag/active check onward byte-for-byte; only the prologue diverges at MISMATCH #6 (original reserves the `year` local with `push ecx` ahead of the ebx/esi callee-save pushes - same wall as 0x00477840's sibling, and every C form tried here has the compiler push ebx/esi first instead).
void __cdecl mon_prototype_built(int faction_id) {
    const int year = static_cast<int>(game_year(GameDataStateStorage.fields_.turn_current_num_));
    MonumentRecord &record = FactionMonuments[faction_id].records_[3];

    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0 || record.achieved_ != 0) {
        return;
    }

    record.word_14_ = 0;
    for (int other = 0; other < 8; ++other) {
        if (other != faction_id &&
            FactionMonuments[other].records_[3].achieved_ != 0) {
            ++record.word_14_;
        }
    }

    int owner_faction;
    if (record.word_14_ != 0) {
        record.word_14_ = 0;
        owner_faction = faction_id;
        for (int other = 0; other < 8; ++other) {
            const MonumentRecord &other_record =
                FactionMonuments[other].records_[3];
            if (other != faction_id && other_record.achieved_ != 0 &&
                other_record.word_14_ != 0) {
                owner_faction = other;
            }
        }
    } else {
        record.word_14_ = 1;
        owner_faction = faction_id;
    }

    record.achieved_ = 1;
    record.owner_faction_ = owner_faction;
    record.year_ = year;
    FactionMonuments[faction_id].latest_type_ = 3;
    if (faction_id == ConsoleGlobal.faction_id_) {
        monument(1);
    }
}

/*
Purpose: The type-8 trigger: the faction's first naval unit built.
// size      254 bytes
Status: Coverage transcription of the retained seed
        (git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00477440.cpp, plain FILE); same shape and walls
        as mon_prototype_built above, over the naval record table.
*/
// ORIGINAL: 0x00477440 ?mon_naval_unit_built@@YAXH@Z 0x00477440-0x0047753E
// RECOVERY_BINDING: v1 {"evidence":["Pinned original entry 0x00477440 and its complete recorded span identify mon_naval_unit_built.","src/game.h defines MonumentRecord/MonumentFaction and the shared FactionMonuments[8] object; the body uses those typed record/faction owners rather than a second table view.","Existing src/game.cpp milestone producers already own neighboring records 0,1,2,4,5,6,7,12; this event family fills record types 3,8,9,10,11,13,14,15..20 on the same owner.","Actual callers remain in src/base.cpp (build/turn events), src/faction.cpp (faction death), and src/game.cpp (unification); notification calls resolve through the existing monument UI API.","Historical seed comments and unclaimed/non-byte-exact status are retained; no new byte or semantic claim is made."],"reason":"Keep this milestone producer with the existing source-owned FactionMonuments event family. Retain the exact branch order, record writes, unchecked faction indices, current-local-faction notification timing, and original matching/restoration obligations."}
// TRIED: MonRecord struct + reinterpret_cast<MonRecord*>(base)+a1 array indexing reproduces every instruction from the flag/active check onward byte-for-byte; only the prologue diverges at MISMATCH #6 (original reserves the `year` local with `push ecx` ahead of the ebx/esi callee-save pushes - same wall as 0x00477840's sibling, and every C form tried here has the compiler push ebx/esi first instead).
void __cdecl mon_naval_unit_built(int faction_id) {
    const int year = static_cast<int>(game_year(GameDataStateStorage.fields_.turn_current_num_));
    MonumentRecord &record = FactionMonuments[faction_id].records_[8];

    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0 || record.achieved_ != 0) {
        return;
    }

    record.word_14_ = 0;
    for (int other = 0; other < 8; ++other) {
        if (other != faction_id &&
            FactionMonuments[other].records_[8].achieved_ != 0) {
            ++record.word_14_;
        }
    }

    int owner_faction;
    if (record.word_14_ != 0) {
        record.word_14_ = 0;
        owner_faction = faction_id;
        for (int other = 0; other < 8; ++other) {
            const MonumentRecord &other_record =
                FactionMonuments[other].records_[8];
            if (other != faction_id && other_record.achieved_ != 0 &&
                other_record.word_14_ != 0) {
                owner_faction = other;
            }
        }
    } else {
        record.word_14_ = 1;
        owner_faction = faction_id;
    }

    record.achieved_ = 1;
    record.owner_faction_ = owner_faction;
    record.year_ = year;
    FactionMonuments[faction_id].latest_type_ = 8;
    if (faction_id == ConsoleGlobal.faction_id_) {
        monument(1);
    }
}

/*
Purpose: The type-9 trigger: the faction's first air unit built.
// size      254 bytes
Status: Coverage transcription of the retained seed
        (git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00477540.cpp, plain FILE); same shape and walls
        as mon_prototype_built above, over the air record table.
*/
// ORIGINAL: 0x00477540 ?mon_air_unit_built@@YAXH@Z 0x00477540-0x0047763E
// RECOVERY_BINDING: v1 {"evidence":["Pinned original entry 0x00477540 and its complete recorded span identify mon_air_unit_built.","src/game.h defines MonumentRecord/MonumentFaction and the shared FactionMonuments[8] object; the body uses those typed record/faction owners rather than a second table view.","Existing src/game.cpp milestone producers already own neighboring records 0,1,2,4,5,6,7,12; this event family fills record types 3,8,9,10,11,13,14,15..20 on the same owner.","Actual callers remain in src/base.cpp (build/turn events), src/faction.cpp (faction death), and src/game.cpp (unification); notification calls resolve through the existing monument UI API.","Historical seed comments and unclaimed/non-byte-exact status are retained; no new byte or semantic claim is made."],"reason":"Keep this milestone producer with the existing source-owned FactionMonuments event family. Retain the exact branch order, record writes, unchecked faction indices, current-local-faction notification timing, and original matching/restoration obligations."}
// TRIED: MonRecord struct + reinterpret_cast<MonRecord*>(base)+a1 array indexing reproduces every instruction from the flag/active check onward byte-for-byte; only the prologue diverges at MISMATCH #6 (original reserves the `year` local with `push ecx` ahead of the ebx/esi callee-save pushes - same wall as 0x00477840's sibling, and every C form tried here has the compiler push ebx/esi first instead).
void __cdecl mon_air_unit_built(int faction_id) {
    const int year = static_cast<int>(game_year(GameDataStateStorage.fields_.turn_current_num_));
    MonumentRecord &record = FactionMonuments[faction_id].records_[9];

    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0 || record.achieved_ != 0) {
        return;
    }

    record.word_14_ = 0;
    for (int other = 0; other < 8; ++other) {
        if (other != faction_id &&
            FactionMonuments[other].records_[9].achieved_ != 0) {
            ++record.word_14_;
        }
    }

    int owner_faction;
    if (record.word_14_ != 0) {
        record.word_14_ = 0;
        owner_faction = faction_id;
        for (int other = 0; other < 8; ++other) {
            const MonumentRecord &other_record =
                FactionMonuments[other].records_[9];
            if (other != faction_id && other_record.achieved_ != 0 &&
                other_record.word_14_ != 0) {
                owner_faction = other;
            }
        }
    } else {
        record.word_14_ = 1;
        owner_faction = faction_id;
    }

    record.achieved_ = 1;
    record.owner_faction_ = owner_faction;
    record.year_ = year;
    FactionMonuments[faction_id].latest_type_ = 9;
    if (faction_id == ConsoleGlobal.faction_id_) {
        monument(1);
    }
}

/*
Purpose: The type-10 trigger: the faction's first native life bred.
// size      254 bytes
Status: Coverage transcription of the retained seed
        (git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00477640.cpp, plain FILE); same shape and walls
        as mon_prototype_built above, over the native-life record table.
*/
// ORIGINAL: 0x00477640 ?mon_native_life_bred@@YAXH@Z 0x00477640-0x0047773E
// RECOVERY_BINDING: v1 {"evidence":["Pinned original entry 0x00477640 and its complete recorded span identify mon_native_life_bred.","src/game.h defines MonumentRecord/MonumentFaction and the shared FactionMonuments[8] object; the body uses those typed record/faction owners rather than a second table view.","Existing src/game.cpp milestone producers already own neighboring records 0,1,2,4,5,6,7,12; this event family fills record types 3,8,9,10,11,13,14,15..20 on the same owner.","Actual callers remain in src/base.cpp (build/turn events), src/faction.cpp (faction death), and src/game.cpp (unification); notification calls resolve through the existing monument UI API.","Historical seed comments and unclaimed/non-byte-exact status are retained; no new byte or semantic claim is made."],"reason":"Keep this milestone producer with the existing source-owned FactionMonuments event family. Retain the exact branch order, record writes, unchecked faction indices, current-local-faction notification timing, and original matching/restoration obligations."}
// TRIED: MonRecord struct + reinterpret_cast<MonRecord*>(base)+a1 array indexing reproduces every instruction from the flag/active check onward byte-for-byte; only the prologue diverges at MISMATCH #6 (original reserves the `year` local with `push ecx` ahead of the ebx/esi callee-save pushes - same wall as 0x00477840's sibling, and every C form tried here has the compiler push ebx/esi first instead).
void __cdecl mon_native_life_bred(int faction_id) {
    const int year = static_cast<int>(game_year(GameDataStateStorage.fields_.turn_current_num_));
    MonumentRecord &record = FactionMonuments[faction_id].records_[10];

    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0 || record.achieved_ != 0) {
        return;
    }

    record.word_14_ = 0;
    for (int other = 0; other < 8; ++other) {
        if (other != faction_id &&
            FactionMonuments[other].records_[10].achieved_ != 0) {
            ++record.word_14_;
        }
    }

    int owner_faction;
    if (record.word_14_ != 0) {
        record.word_14_ = 0;
        owner_faction = faction_id;
        for (int other = 0; other < 8; ++other) {
            const MonumentRecord &other_record =
                FactionMonuments[other].records_[10];
            if (other != faction_id && other_record.achieved_ != 0 &&
                other_record.word_14_ != 0) {
                owner_faction = other;
            }
        }
    } else {
        record.word_14_ = 1;
        owner_faction = faction_id;
    }

    record.achieved_ = 1;
    record.owner_faction_ = owner_faction;
    record.year_ = year;
    FactionMonuments[faction_id].latest_type_ = 10;
    if (faction_id == ConsoleGlobal.faction_id_) {
        monument(1);
    }
}

/*
Purpose: The type-11 trigger: the faction's first craft in space.
// size      254 bytes
Status: Coverage transcription of the retained seed
        (git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00477740.cpp, plain FILE); same shape and walls
        as mon_prototype_built above, over the space record table.
*/
// ORIGINAL: 0x00477740 ?mon_first_in_space@@YAXH@Z 0x00477740-0x0047783E
// RECOVERY_BINDING: v1 {"evidence":["Pinned original entry 0x00477740 and its complete recorded span identify mon_first_in_space.","src/game.h defines MonumentRecord/MonumentFaction and the shared FactionMonuments[8] object; the body uses those typed record/faction owners rather than a second table view.","Existing src/game.cpp milestone producers already own neighboring records 0,1,2,4,5,6,7,12; this event family fills record types 3,8,9,10,11,13,14,15..20 on the same owner.","Actual callers remain in src/base.cpp (build/turn events), src/faction.cpp (faction death), and src/game.cpp (unification); notification calls resolve through the existing monument UI API.","Historical seed comments and unclaimed/non-byte-exact status are retained; no new byte or semantic claim is made."],"reason":"Keep this milestone producer with the existing source-owned FactionMonuments event family. Retain the exact branch order, record writes, unchecked faction indices, current-local-faction notification timing, and original matching/restoration obligations."}
// TRIED: MonRecord struct + reinterpret_cast<MonRecord*>(base)+a1 array indexing reproduces every instruction from the flag/active check onward byte-for-byte; only the prologue diverges at MISMATCH #6 (original reserves the `year` local with `push ecx` ahead of the ebx/esi callee-save pushes - same wall as 0x00477840's sibling, and every C form tried here has the compiler push ebx/esi first instead).
void __cdecl mon_first_in_space(int faction_id) {
    const int year = static_cast<int>(game_year(GameDataStateStorage.fields_.turn_current_num_));
    MonumentRecord &record = FactionMonuments[faction_id].records_[11];

    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0 || record.achieved_ != 0) {
        return;
    }

    record.word_14_ = 0;
    for (int other = 0; other < 8; ++other) {
        if (other != faction_id &&
            FactionMonuments[other].records_[11].achieved_ != 0) {
            ++record.word_14_;
        }
    }

    int owner_faction;
    if (record.word_14_ != 0) {
        record.word_14_ = 0;
        owner_faction = faction_id;
        for (int other = 0; other < 8; ++other) {
            const MonumentRecord &other_record =
                FactionMonuments[other].records_[11];
            if (other != faction_id && other_record.achieved_ != 0 &&
                other_record.word_14_ != 0) {
                owner_faction = other;
            }
        }
    } else {
        record.word_14_ = 1;
        owner_faction = faction_id;
    }

    record.achieved_ = 1;
    record.owner_faction_ = owner_faction;
    record.year_ = year;
    FactionMonuments[faction_id].latest_type_ = 11;
    if (faction_id == ConsoleGlobal.faction_id_) {
        monument(1);
    }
}

/*
Purpose: Record the unification victory monument for one faction: skip
         factions already flagged, take the faction's unify record slot
         when its live word is clear, stamp the year/counterpart/faction
         and the two live words, publish 0xD as the faction's latest
         monument type, and open the monument window for the local faction.

// size      135 bytes
// prototype
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005C89A0 0x00476A50
Return Value: n/a
Status: Coverage transcription - unclaimed body
*/
// ORIGINAL: 0x00477940 ?mon_winning_unify@@YAXHH@Z 0x00477940-0x004779C7
// RECOVERY_BINDING: v1 {"evidence":["Pinned original entry 0x00477940 and its complete recorded span identify mon_winning_unify.","src/game.h defines MonumentRecord/MonumentFaction and the shared FactionMonuments[8] object; the body uses those typed record/faction owners rather than a second table view.","Existing src/game.cpp milestone producers already own neighboring records 0,1,2,4,5,6,7,12; this event family fills record types 3,8,9,10,11,13,14,15..20 on the same owner.","Actual callers remain in src/base.cpp (build/turn events), src/faction.cpp (faction death), and src/game.cpp (unification); notification calls resolve through the existing monument UI API.","Historical seed comments and unclaimed/non-byte-exact status are retained; no new byte or semantic claim is made."],"reason":"Keep this milestone producer with the existing source-owned FactionMonuments event family. Retain the exact branch order, record writes, unchecked faction indices, current-local-faction notification timing, and original matching/restoration obligations."}
// kind game
void __cdecl mon_winning_unify(int faction_id, int counterpart) {
    const int year = static_cast<int>(game_year(GameDataStateStorage.fields_.turn_current_num_));
    MonumentRecord &record = FactionMonuments[faction_id].records_[13];
    if ((Players[faction_id].rule_flags & 0x80) != 0) {
        return;
    }
    if (record.achieved_ != 0) {
        return;
    }
    record.word_14_ = 1;
    record.achieved_ = 1;
    record.year_ = year;
    record.owner_faction_ = faction_id;
    record.word_8_ = counterpart;
    FactionMonuments[faction_id].latest_type_ = 0xD;
    if (faction_id == ConsoleGlobal.faction_id_) {
        monument(1);
    }
}

/*
Purpose: The transcendence twin of mon_winning_unify: same gates and
         record shape over the faction's trans record slot, publishing
         0xE as the latest monument type instead.

// size      135 bytes
// prototype
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005C89A0 0x00476A50
Return Value: n/a
Status: Coverage transcription - unclaimed body
*/
// ORIGINAL: 0x004779D0 ?mon_winning_trans@@YAXHH@Z 0x004779D0-0x00477A57
// RECOVERY_BINDING: v1 {"evidence":["Pinned original entry 0x004779D0 and its complete recorded span identify mon_winning_trans.","src/game.h defines MonumentRecord/MonumentFaction and the shared FactionMonuments[8] object; the body uses those typed record/faction owners rather than a second table view.","Existing src/game.cpp milestone producers already own neighboring records 0,1,2,4,5,6,7,12; this event family fills record types 3,8,9,10,11,13,14,15..20 on the same owner.","Actual callers remain in src/base.cpp (build/turn events), src/faction.cpp (faction death), and src/game.cpp (unification); notification calls resolve through the existing monument UI API.","Historical seed comments and unclaimed/non-byte-exact status are retained; no new byte or semantic claim is made."],"reason":"Keep this milestone producer with the existing source-owned FactionMonuments event family. Retain the exact branch order, record writes, unchecked faction indices, current-local-faction notification timing, and original matching/restoration obligations."}
// TRIED: `extern "C"` on the already-declared plain-linkage game_year/monument callees conflicts as C2732; a byte-table extern for 0x94CD18 compiles but the `lea` vs `sub` MISMATCH stayed open.
// kind game
void __cdecl mon_winning_trans(int faction_id, int counterpart) {
    const int year = static_cast<int>(game_year(GameDataStateStorage.fields_.turn_current_num_));
    MonumentRecord &record = FactionMonuments[faction_id].records_[14];
    if ((Players[faction_id].rule_flags & 0x80) != 0) {
        return;
    }
    if (record.achieved_ != 0) {
        return;
    }
    record.word_14_ = 1;
    record.achieved_ = 1;
    record.year_ = year;
    record.owner_faction_ = faction_id;
    record.word_8_ = counterpart;
    FactionMonuments[faction_id].latest_type_ = 0xE;
    if (faction_id == ConsoleGlobal.faction_id_) {
        monument(1);
    }
}

/*
Purpose: Place a faction-killed faction's shared monument: scan the six
         free 0x38-byte slots for the first clear achieved word, take the
         matching 0xF+i index, stamp the finished-record fields, publish
         the chosen index as the latest monument type, and open the
         monument window for the unflagged local faction.

// size      175 bytes
// prototype
// callers   0   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005C89A0 0x00476A50
Return Value: n/a
Status: Coverage transcription - unclaimed body
*/
// ORIGINAL: 0x00477A60 ?mon_killed_faction@@YAXHH@Z 0x00477A60-0x00477B0F BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["Pinned original entry 0x00477A60 and its complete recorded span identify mon_killed_faction.","src/game.h defines MonumentRecord/MonumentFaction and the shared FactionMonuments[8] object; the body uses those typed record/faction owners rather than a second table view.","Existing src/game.cpp milestone producers already own neighboring records 0,1,2,4,5,6,7,12; this event family fills record types 3,8,9,10,11,13,14,15..20 on the same owner.","Actual callers remain in src/base.cpp (build/turn events), src/faction.cpp (faction death), and src/game.cpp (unification); notification calls resolve through the existing monument UI API.","Historical seed comments and unclaimed/non-byte-exact status are retained; no new byte or semantic claim is made."],"reason":"Keep this milestone producer with the existing source-owned FactionMonuments event family. Retain the exact branch order, record writes, unchecked faction indices, current-local-faction notification timing, and original matching/restoration obligations."}
// TRIED: the flag byte needs its own extern beside the scaffold's int-typed binding (C2373), and the three table bases need masked relocations; the spill of `year` to an extra frame slot stayed a 0.86-similarity MISMATCH.
// kind game
void __cdecl mon_killed_faction(int faction_id, int counterpart) {
    // The image tests the -1 sentinel as `chosen + 1` (lea/test).
    int chosen = -1;
    const int year = static_cast<int>(game_year(GameDataStateStorage.fields_.turn_current_num_));
    for (int index = 0; index < 6; ++index) {
        if (FactionMonuments[faction_id].records_[index + 0xF].achieved_ == 0
            && chosen + 1 == 0) {
            chosen = index + 0xF;
        }
    }
    if (chosen + 1 == 0) {
        return;
    }
    MonumentRecord &record = FactionMonuments[faction_id].records_[chosen];
    record.year_ = year;
    record.word_8_ = counterpart;
    record.word_14_ = 1;
    record.achieved_ = 1;
    record.owner_faction_ = faction_id;
    FactionMonuments[faction_id].latest_type_ = chosen;
    if ((Players[faction_id].rule_flags & 0x80) == 0
        && faction_id == ConsoleGlobal.faction_id_) {
        monument(1);
    }
}

