#include "actual_cases_7_8.hpp"

#include <cassert>
#include <cstring>
#include <vector>

struct PositionState {
    int multiplayer_event_guard_value;
    int base_x_value, base_y_value, faction_id_value;
    int longitude_bounds_value, latitude_bounds_value;
    int map_is_flat_value;
    unsigned int player_data_flags_value;
    int turn_current_num_value;
    int console_faction_id_value;

    int multiplayer_event_guard() const { return multiplayer_event_guard_value; }
    int base_x() const { return base_x_value; }
    int base_y() const { return base_y_value; }
    int faction_id() const { return faction_id_value; }
    int longitude_bounds() const { return longitude_bounds_value; }
    int latitude_bounds() const { return latitude_bounds_value; }
    int map_is_flat() const { return map_is_flat_value; }
    unsigned int player_data_flags() const { return player_data_flags_value; }
    int turn_current_num() const { return turn_current_num_value; }
    int console_faction_id() const { return console_faction_id_value; }
};

struct PositionServices {
    struct Cell {
        unsigned int visibility;
        unsigned char climate;
        int bonus;
        int owner;
    };
    Cell cells[32][32];
    std::vector<int> random_results;
    std::vector<int> random_highs;
    std::vector<int> calls;
    std::vector<int> bonus_results;
    std::vector<int> bit_values;
    std::vector<int> waits;
    std::vector<int> parsed_resources;
    std::vector<const char *> popup_keys;
    std::vector<int> order;
    std::vector<int> poll_results;
    int multiplayer_net_value;
    PositionState *mutable_state;
    bool change_local_after_draw;
    bool change_base_after_first_bonus_query;
    int bonus_query_count;

    PositionServices() {
        multiplayer_net_value = 0;
        mutable_state = 0;
        change_local_after_draw = false;
        change_base_after_first_bonus_query = false;
        bonus_query_count = 0;
        for (int y = 0; y < 32; ++y) {
            for (int x = 0; x < 32; ++x) {
                cells[y][x].visibility = 0;
                cells[y][x].climate = 0x80;
                cells[y][x].bonus = 0;
                cells[y][x].owner = -1;
            }
        }
    }

    int random(int low, int high) {
        assert(low == 0 && high == 20);
        assert(!random_results.empty());
        random_highs.push_back(high);
        const int result = random_results.front();
        random_results.erase(random_results.begin());
        calls.push_back(1);
        order.push_back(1);
        return result;
    }
    int bonus_at(int x, int y, int unused) {
        assert(unused == 0);
        calls.push_back(2);
        order.push_back(2);
        ++bonus_query_count;
        if (change_base_after_first_bonus_query && bonus_query_count == 1 &&
            mutable_state != 0) {
            mutable_state->base_x_value = 12;
        }
        if (!bonus_results.empty()) {
            const int result = bonus_results.front();
            bonus_results.erase(bonus_results.begin());
            return result;
        }
        return cells[y][x].bonus;
    }
    unsigned int visibility(int x, int y) {
        calls.push_back(3);
        return cells[y][x].visibility;
    }
    unsigned char climate(int x, int y) {
        calls.push_back(4);
        return cells[y][x].climate;
    }
    int whose_territory(int faction_id, int x, int y, int *base_id,
                        int ignore_comm) {
        assert(base_id == 0 && ignore_comm == 0);
        calls.push_back(5);
        (void)faction_id;
        return cells[y][x].owner;
    }
    void bit_set(int x, int y, unsigned int mask, int enabled) {
        assert(mask == 0x400u);
        (void)x; (void)y;
        bit_values.push_back(enabled);
        order.push_back(6);
    }
    void synch_bit(int x, int y, int faction_id) {
        (void)x; (void)y; (void)faction_id;
        order.push_back(7);
    }
    void draw_tile(int x, int y, int mode) {
        (void)x; (void)y;
        assert(mode == 2);
        order.push_back(8);
        if (change_local_after_draw && mutable_state != 0)
            mutable_state->console_faction_id_value = 3;
    }
    void center_map(int x, int y, int animate) {
        (void)x; (void)y;
        assert(animate == 1);
        order.push_back(9);
    }
    int poll_key(int key) {
        assert(key == 0x10);
        order.push_back(10);
        if (poll_results.empty()) return 0;
        const int result = poll_results.front();
        poll_results.erase(poll_results.begin());
        return result;
    }
    int is_multiplayer_net() {
        order.push_back(11);
        return multiplayer_net_value;
    }
    void wait(int milliseconds) {
        waits.push_back(milliseconds);
        order.push_back(12);
    }
    void parse_resource_say(int resource_index) {
        parsed_resources.push_back(resource_index);
        order.push_back(13);
    }
    void popup(const char *caption_key, const char *image_key, int argument) {
        assert(std::strcmp(caption_key, "NEWRESOURCE") == 0 && argument == -1);
        popup_keys.push_back(image_key);
        order.push_back(14);
    }
};

static const int radius_x[22] = {
    0, 1, 2, 1, 0, -1, -2, -1, 0,
    2, 2, -2, -2, 1, 3, 3, 1, -1, -3, -3, -1, 4
};
static const int radius_y[22] = {
    0, -1, 0, 1, 2, 1, 0, -1, -2,
    -2, 2, 2, -2, -3, -1, 1, 3, 3, 1, -1, -3, 0
};

static PositionState make_state() {
    PositionState s;
    s.multiplayer_event_guard_value = 0;
    s.base_x_value = 10;
    s.base_y_value = 10;
    s.faction_id_value = 2;
    s.longitude_bounds_value = 20;
    s.latitude_bounds_value = 20;
    s.map_is_flat_value = 1;
    s.player_data_flags_value = 0;
    s.turn_current_num_value = 100;
    s.console_faction_id_value = 2;
    return s;
}

static void test_guard_skips_random_and_all_queries() {
    PositionState state = make_state();
    state.multiplayer_event_guard_value = 1;
    PositionServices services;
    const random_events_cases::PositionSelection result =
        random_events_cases::select_case_7_8_position(
            state, services, radius_x, radius_y);
    assert(!result.valid && result.offset_index == -1);
    assert(services.calls.empty());
}

static void test_scan_uses_faction_visibility_and_nearby_bonus() {
    PositionState state = make_state();
    PositionServices services;
    services.random_results.push_back(19); // Fallback index would be 20.
    // Offset 0 has a bonus but is invisible to faction 2, so scan continues.
    services.cells[10][10].bonus = 1;
    services.cells[10][10].visibility = 1u << 1;
    // Offset 1 is visible to faction 2 and owned by nobody; it is selected.
    services.cells[9][11].bonus = 3;
    services.cells[9][11].visibility = 1u << 2;
    services.cells[9][11].climate = 0x40; // no territory query below shore

    const random_events_cases::PositionSelection result =
        random_events_cases::select_case_7_8_position(
            state, services, radius_x, radius_y);
    assert(result.valid && result.found_nearby_bonus);
    assert(result.offset_index == 1 && result.x == 11 && result.y == 9);
    assert(services.random_highs.size() == 1 && services.random_highs[0] == 20);
    // Native performs the final visibility check again after recomputing x/y.
    int visible_calls = 0;
    for (std::size_t i = 0; i < services.calls.size(); ++i)
        if (services.calls[i] == 3) ++visible_calls;
    assert(visible_calls == 3);
}

static void test_fallback_offset_twenty_uses_radius_index_twenty_one() {
    PositionState state = make_state();
    PositionServices services;
    services.random_results.push_back(20);
    services.cells[10][14].visibility = 1u << 2;

    const random_events_cases::PositionSelection result =
        random_events_cases::select_case_7_8_position(
            state, services, radius_x, radius_y);
    assert(result.valid && !result.found_nearby_bonus);
    assert(result.offset_index == 21 && result.x == 14 && result.y == 10);
}

static void test_scan_reloads_base_coordinates_after_service_calls() {
    PositionState state = make_state();
    PositionServices services;
    services.mutable_state = &state;
    services.change_base_after_first_bonus_query = true;
    services.random_results.push_back(19);
    services.cells[9][13].bonus = 1;
    services.cells[9][13].visibility = 1u << 2;

    const random_events_cases::PositionSelection result =
        random_events_cases::select_case_7_8_position(
            state, services, radius_x, radius_y);
    assert(result.valid && result.found_nearby_bonus);
    assert(result.offset_index == 1 && result.x == 13 && result.y == 9);
}

static void test_visibility_exception_and_foreign_territory_rejection() {
    PositionState state = make_state();
    state.player_data_flags_value = 0x200;
    PositionServices services;
    services.random_results.push_back(0);
    services.cells[10][10].bonus = 2;
    services.cells[10][10].climate = 0x60;
    services.cells[10][10].owner = 3; // reject as foreign owned
    services.cells[9][11].bonus = 2;
    services.cells[9][11].climate = 0x40; // accept without territory call

    const random_events_cases::PositionSelection result =
        random_events_cases::select_case_7_8_position(
            state, services, radius_x, radius_y);
    assert(result.valid && result.offset_index == 1);
    assert(result.x == 11 && result.y == 9);
    // Visibility is bypassed by PlayerData.flags & 0x200; owner is queried
    // for the first shore-or-higher candidate, then not for the accepted one.
    int visibility_calls = 0, owner_calls = 0;
    for (std::size_t i = 0; i < services.calls.size(); ++i) {
        if (services.calls[i] == 3) ++visibility_calls;
        if (services.calls[i] == 5) ++owner_calls;
    }
    assert(visibility_calls == 0);
    assert(owner_calls == 1);
}

static void test_single_wrap_and_fallback_offsets_9_through_20() {
    PositionState state = make_state();
    state.base_x_value = 0;
    state.base_y_value = 10;
    state.map_is_flat_value = 0; // wrap enabled
    PositionServices services;
    services.random_results.push_back(8); // fallback index 9 if scan fails
    // Radius slot 5 is x=-1 from the base; native adds width once, then
    // accepts it only after ordinary visibility/terrain tests.
    services.cells[11][19].bonus = 2;
    services.cells[11][19].visibility = 1u << 2;
    services.cells[11][19].climate = 0x40;

    const random_events_cases::PositionSelection result =
        random_events_cases::select_case_7_8_position(
            state, services, radius_x, radius_y);
    assert(result.valid && result.found_nearby_bonus);
    assert(result.offset_index == 5 && result.x == 19 && result.y == 11);

    // A visible fallback point may be accepted without a bonus lookup. The
    // fallback uses offset 9; final eligibility is rechecked, not scanned.
    services.cells[11][19].bonus = 0;
    services.random_results.push_back(8);
    services.cells[8][2].visibility = 1u << 2;
    services.cells[8][2].climate = 0x40;
    const random_events_cases::PositionSelection fallback =
        random_events_cases::select_case_7_8_position(
            state, services, radius_x, radius_y);
    assert(fallback.valid && !fallback.found_nearby_bonus);
    assert(fallback.offset_index == 9 && fallback.x == 2 && fallback.y == 8);
}

static void test_foreign_territory_final_gate_and_flat_map_bounds() {
    PositionState state = make_state();
    PositionServices services;
    services.random_results.push_back(0);
    services.cells[10][10].bonus = 1;
    services.cells[10][10].visibility = 1u << 2;
    services.cells[10][10].climate = 0x60;
    services.cells[10][10].owner = 4;
    // No nearby candidate; fallback index 1 points here but fails final owner.
    services.random_results.push_back(0);
    services.cells[9][11].visibility = 1u << 2;
    services.cells[9][11].climate = 0x60;
    services.cells[9][11].owner = 4;
    const random_events_cases::PositionSelection result =
        random_events_cases::select_case_7_8_position(
            state, services, radius_x, radius_y);
    assert(!result.valid);
}

static void test_nine_candidate_scan_then_random_offset_twenty() {
    PositionState state = make_state();
    PositionServices services;
    services.random_results.push_back(19); // Native +1 gives offset index 20.
    services.cells[7][9].visibility = 1u << 2;
    services.cells[7][9].climate = 0x40;

    const random_events_cases::PositionSelection result =
        random_events_cases::select_case_7_8_position(
            state, services, radius_x, radius_y);
    assert(result.valid && !result.found_nearby_bonus);
    assert(result.offset_index == 20 && result.x == 9 && result.y == 7);
    int bonus_queries = 0;
    for (std::size_t i = 0; i < services.calls.size(); ++i)
        if (services.calls[i] == 2) ++bonus_queries;
    assert(bonus_queries == 9);
}

static void test_outcome_remove_resource_with_native_animation_order() {
    PositionState state = make_state();
    PositionServices services;
    services.bonus_results.push_back(2); // initial presence check
    services.bonus_results.push_back(2); // parse_say table selection
    services.bonus_results.push_back(0); // post-clear check
    random_events_cases::PositionSelection position = {true, true, 1, 11, 9};

    const random_events_cases::Case78Result result =
        random_events_cases::apply_case_7_8_outcome(state, services, position);
    assert(result.outcome == random_events_cases::CASE78_NEARBY_BONUS_REMOVED);
    assert(result.resource_index == 1 && result.parsed_resource_say);
    assert(result.popup_emitted
           && std::strcmp(result.popup_image_key, "rdminl_sm.pcx") == 0);
    assert(services.parsed_resources.size() == 1
           && services.parsed_resources[0] == 1);
    assert(services.bit_values.size() == 21);
    assert(services.bit_values[0] == 0);
    assert(services.bit_values[1] == 1 && services.bit_values[2] == 0);
    assert(services.waits.size() == 21);
    assert(services.waits[0] == 0x14 && services.waits[20] == 0x64);
    assert(services.popup_keys.size() == 1
           && std::strcmp(services.popup_keys[0], "rdminl_sm.pcx") == 0);
}

static void test_outcome_reloads_local_faction_after_mutating_draw_callback() {
    PositionState state = make_state();
    PositionServices services;
    services.mutable_state = &state;
    services.change_local_after_draw = true;
    services.bonus_results.push_back(2);
    services.bonus_results.push_back(2);
    services.bonus_results.push_back(0);
    random_events_cases::PositionSelection position = {true, true, 1, 11, 9};

    const random_events_cases::Case78Result result =
        random_events_cases::apply_case_7_8_outcome(state, services, position);
    assert(state.console_faction_id_value == 3);
    assert(result.outcome == random_events_cases::CASE78_NO_OUTCOME);
    assert(!result.popup_emitted && services.popup_keys.empty());
}

static void test_outcome_add_resource_key_skip_and_turn_gate() {
    PositionState state = make_state();
    state.turn_current_num_value = 74;
    PositionServices services;
    services.bonus_results.push_back(3);
    random_events_cases::PositionSelection position = {true, true, 0, 10, 10};
    random_events_cases::Case78Result result =
        random_events_cases::apply_case_7_8_outcome(state, services, position);
    assert(result.outcome == random_events_cases::CASE78_NO_OUTCOME);
    assert(services.bonus_results.size() == 0);
    assert(services.bit_values.empty());

    // A fallback selection sets the bit before looking up the resource. A
    // pressed key skips the flash loop; parsing and popup still follow.
    state.turn_current_num_value = 100;
    services.bonus_results.push_back(3);
    services.poll_results.push_back(0x0100);
    position.found_nearby_bonus = false;
    position.x = 10;
    position.y = 10;
    result = random_events_cases::apply_case_7_8_outcome(
        state, services, position);
    assert(result.outcome == random_events_cases::CASE78_FALLBACK_BONUS_SET);
    assert(result.resource_index == 2 && result.parsed_resource_say);
    assert(result.popup_emitted
           && std::strcmp(result.popup_image_key, "rdminl_sm.pcx") == 0);
    assert(services.bit_values.size() == 1 && services.bit_values[0] == 1);
    assert(services.waits.empty());
}

static void test_outcome_animation_continues_but_suppresses_network_waits() {
    PositionState state = make_state();
    PositionServices services;
    services.multiplayer_net_value = 1;
    services.bonus_results.push_back(2);
    // Initial key poll, then a mid-animation key press. The native still
    // completes all ten bit alternations; it merely skips waits while busy.
    services.poll_results.push_back(0);
    services.poll_results.push_back(0x0100);
    random_events_cases::PositionSelection position = {true, false, 9, 12, 8};

    const random_events_cases::Case78Result result =
        random_events_cases::apply_case_7_8_outcome(state, services, position);
    assert(result.outcome == random_events_cases::CASE78_FALLBACK_BONUS_SET);
    assert(services.bit_values.size() == 21);
    assert(services.poll_results.empty());
    assert(services.waits.empty());
}

int main() {
    test_guard_skips_random_and_all_queries();
    test_scan_uses_faction_visibility_and_nearby_bonus();
    test_scan_reloads_base_coordinates_after_service_calls();
    test_fallback_offset_twenty_uses_radius_index_twenty_one();
    test_visibility_exception_and_foreign_territory_rejection();
    test_single_wrap_and_fallback_offsets_9_through_20();
    test_foreign_territory_final_gate_and_flat_map_bounds();
    test_nine_candidate_scan_then_random_offset_twenty();
    test_outcome_remove_resource_with_native_animation_order();
    test_outcome_reloads_local_faction_after_mutating_draw_callback();
    test_outcome_add_resource_key_skip_and_turn_gate();
    test_outcome_animation_continues_but_suppresses_network_waits();
    return 0;
}

