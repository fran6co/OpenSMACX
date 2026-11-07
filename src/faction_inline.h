#pragma once

// Header-visible diplomacy helpers shared by faction.cpp and faction_treaty.cpp.
// has_treaty/has_agenda are expanded in place in both units while the image
// keeps one out-of-line copy of each for the callers that do not expand them;
// their original markers stay in faction.cpp. Include after faction.h.

// Native i386 word arithmetic; preserve wrapping before signed division.
static int diplo_add(int a, int b) { return static_cast<int>(static_cast<uint32_t>(a) + static_cast<uint32_t>(b)); }
static int diplo_sub(int a, int b) { return static_cast<int>(static_cast<uint32_t>(a) - static_cast<uint32_t>(b)); }
static int diplo_mul(int a, int b) { return static_cast<int>(static_cast<uint32_t>(a) * static_cast<uint32_t>(b)); }

// vector_dist's doubled numerator: x_dist plus the y delta, floor kept.
static inline int territory_dist(int x_point_a, int y_point_a, int x_point_b, int y_point_b) {
    int x_distance = x_dist(x_point_a, x_point_b);
    int y_distance = abs(y_point_a - y_point_b);
    return (y_distance + x_distance) & ~1;
}

static void faction_parse_leader_fields(int faction_id, int title_slot,
                                        int name_slot, int noun_slot,
                                        int adj_slot) {
    GenderDefault = Players[faction_id].is_leader_female;
    PluralityDefault = FALSE;
    parse_says(title_slot, Players[faction_id].title_leader, -1, -1);
    GenderDefault = Players[faction_id].is_leader_female;
    PluralityDefault = FALSE;
    parse_says(name_slot, Players[faction_id].name_leader, -1, -1);
    GenderDefault = Players[faction_id].noun_gender;
    PluralityDefault = Players[faction_id].is_noun_plural;
    parse_says(noun_slot, Players[faction_id].noun_faction, -1, -1);
    if (adj_slot >= 0) {
        parse_says(adj_slot, get_he_she(faction_id, 0), -1, -1);
    }
}

// Historical placement rationale: the image writes this helper out at some
// call sites and calls it at others. Both diplomacy units expand it in place;
// base_build, communicate, probe and battle_fight call the out-of-line copy.
MEASURED inline int __cdecl has_treaty(int faction_id, int faction_id_with, int treaty) {
    return PlayersData[faction_id].diplo_treaties[faction_id_with] & treaty;
}

MEASURED inline int __cdecl has_agenda(int faction_id, int faction_id_with, int agenda) {
    return PlayersData[faction_id].diplo_agenda[faction_id_with] & agenda;
}
