#ifndef OPENSMACX_ENEMY_PB_POLICY_H
#define OPENSMACX_ENEMY_PB_POLICY_H

// Vehicle-owned planet-buster admission and blast-scoring operations.
namespace enemy_pb_policy {

inline int charter_balance(int repeal_count, int reinstate_count) {
    return reinstate_count < repeal_count ? 4 : 0;
}

template <class ClimaxQuery>
inline bool treaty_shape_allows_target(unsigned long treaty, ClimaxQuery &query) {
    if (treaty & 0x1UL) return false;             // DTREATY_PACT
    if ((treaty & 0x7UL) == 0) return true;       // no pact/treaty/truce
    return (treaty & 0xC20UL) != 0 && query();    // revenge/betrayal pressure
}

inline int remember_reachable_launch_base(int previous, int candidate,
                                         bool reachable) {
    return reachable ? candidate : previous;
}

inline bool city_blocks_blast(unsigned int city_faction, unsigned int attacker_faction,
                              unsigned long treaty) {
    return city_faction == attacker_faction || (treaty & 0x7UL) != 0;
}

inline int add_city_pressure(int score, int city_rank, int population,
                             bool human_headquarters, bool corner_pressure,
                             bool ascent_race, int ascent_minerals,
                             bool hunter_seeker, bool cloning_vats,
                             int faction_rank, int owned_projects) {
    score += city_rank * population;
    if (human_headquarters && corner_pressure) score += 2000;
    if (ascent_race) score += ascent_minerals * 2;
    if (hunter_seeker) score += 1000;
    if (cloning_vats) score += 500;
    score += faction_rank * 60 * owned_projects;
    return score;
}

inline int region_stack_pressure(int score, int stack_rating, int region_plan,
                                 int region_base_count) {
    score += stack_rating;
    if (region_plan == 2) return score * 2;
    if (region_plan != 0) {
        score /= 2;
        if (region_base_count == 0) score /= 2;
    }
    return score;
}

inline int ranked_stack_pressure(int stack_rating, int faction_rank) {
    return stack_rating * faction_rank;
}

inline int discount_for_orbital_defense(int score, int carried_heavy,
                                         int defense_available,
                                         int friendly_heavy_production) {
    if (carried_heavy >= defense_available + 1) return score;
    if (friendly_heavy_production != 0) return -9999;
    score /= 2;
    if (carried_heavy < defense_available / 2) score /= 2;
    return score;
}

} // namespace enemy_pb_policy
#endif
