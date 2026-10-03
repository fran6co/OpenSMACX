#include "actual_cases_9_11.hpp"

#include <cassert>
#include <vector>

struct HostPlayerData {
    unsigned int satellites_[4];
    HostPlayerData() {
        satellites_[0] = satellites_[1] = satellites_[2] = satellites_[3] = 0;
    }
};

struct HostState {
    int energy_event_state;
    int sunspot_duration;
    int turn_current_num;
    int player_count;
    HostPlayerData *players;
};

struct HostServices {
    std::vector<int> random_results;
    std::vector<int> random_highs;
    std::vector<int> notifications;
    std::vector<int> notification_args;
    std::vector<int> order;
    int parse_result;
    HostState *observed_state;

    HostServices() : parse_result(0), observed_state(0) {}

    int random(int low, int high) {
        assert(low == 0);
        random_highs.push_back(high);
        order.push_back(1);
        assert(!random_results.empty());
        int result = random_results.front();
        random_results.erase(random_results.begin());
        return result;
    }

    void notify(int notification, int argument) {
        if (observed_state != 0) {
            if (notification == random_events_cases::CASE9_STATE_CHANGED
                || notification == random_events_cases::CASE9_STATE_UNCHANGED
                || notification == random_events_cases::CASE10_STATE_CLEARED) {
                assert(observed_state->energy_event_state == 1);
            } else if (notification == random_events_cases::CASE11_DURATION_SET) {
                assert(observed_state->sunspot_duration >= 10);
                assert(observed_state->sunspot_duration <= 20);
            }
        }
        notifications.push_back(notification);
        notification_args.push_back(argument);
        order.push_back(3);
    }

    int parse_num(int low, int high) {
        assert(low == 0 && high == 20);
        if (observed_state != 0) {
            assert(observed_state->sunspot_duration >= 10);
            assert(observed_state->sunspot_duration <= 20);
        }
        order.push_back(2);
        return parse_result;
    }
};

static void test_case9_gate_and_two_counters() {
    HostPlayerData players[8];
    players[0].satellites_[2] = 4; // Native excludes record zero.
    players[1].satellites_[2] = 7;
    players[6].satellites_[3] = 9;
    HostState state = {0, 0, 0, 8, players};
    HostServices services;
    services.observed_state = &state;

    state.energy_event_state = 2;
    random_events_cases::case9(state, services);
    assert(services.random_highs.empty());
    state.energy_event_state = 0;
    services.random_results.push_back(1);
    random_events_cases::case9(state, services);
    assert(services.notifications.empty());
    assert(players[1].satellites_[2] == 7);

    services.random_results.push_back(0);
    random_events_cases::case9(state, services);
    assert(state.energy_event_state == 1);
    assert(players[0].satellites_[2] == 4);
    assert(players[1].satellites_[2] == 0 && players[1].satellites_[3] == 0);
    assert(players[6].satellites_[2] == 0 && players[6].satellites_[3] == 0);
    assert(services.notifications.size() == 1);
    assert(services.notifications[0] == random_events_cases::CASE9_STATE_CHANGED);
    assert(services.notification_args[0] == -1);
}

static void test_case9_sets_latch_on_empty_records() {
    HostPlayerData players[8];
    HostState state = {0, 0, 0, 8, players};
    HostServices services;
    services.observed_state = &state;
    services.random_results.push_back(0);

    random_events_cases::case9(state, services);
    assert(state.energy_event_state == 1);
    assert(services.notifications.size() == 1);
    assert(services.notifications[0] == random_events_cases::CASE9_STATE_UNCHANGED);
}

static void test_case10_clears_only_slot_one_and_latches_even_empty() {
    HostPlayerData players[8];
    players[0].satellites_[1] = 3;
    players[2].satellites_[1] = 5;
    players[3].satellites_[2] = 8;
    HostState state = {0, 0, 0, 8, players};
    HostServices services;
    services.observed_state = &state;
    services.random_results.push_back(0);

    random_events_cases::case10(state, services);
    assert(state.energy_event_state == 1);
    assert(players[0].satellites_[1] == 3);
    assert(players[2].satellites_[1] == 0);
    assert(players[3].satellites_[2] == 8);
    assert(services.notifications.size() == 1);
    assert(services.notifications[0] == random_events_cases::CASE10_STATE_CLEARED);

    state.energy_event_state = 0;
    services.random_results.push_back(0);
    random_events_cases::case10(state, services);
    assert(state.energy_event_state == 1);
    assert(services.notifications.size() == 1);
}

static void test_case11_native_gates_and_callback_order() {
    HostPlayerData players[8];
    HostState state = {0, -39, 49, 8, players};
    HostServices services;
    services.observed_state = &state;
    random_events_cases::case11(state, services);
    assert(services.random_highs.empty());

    state.turn_current_num = 80;
    state.sunspot_duration = -50;
    services.random_results.push_back(1); // The >=80 retry gate.
    random_events_cases::case11(state, services);
    assert(state.sunspot_duration == -50);
    assert(services.notifications.empty());

    services.random_results.push_back(0); // Retry gate opens.
    services.random_results.push_back(6); // Stored duration becomes 16.
    random_events_cases::case11(state, services);
    assert(state.sunspot_duration == 16);
    assert(services.random_highs.size() == 3);
    assert(services.random_highs[0] == 3);
    assert(services.random_highs[1] == 3);
    assert(services.random_highs[2] == 11);
    assert(services.notifications.size() == 1);
    assert(services.notifications[0] == random_events_cases::CASE11_DURATION_SET);
    assert(services.order[services.order.size() - 3] == 1);
    assert(services.order[services.order.size() - 2] == 2);
    assert(services.order[services.order.size() - 1] == 3);
}

int main() {
    test_case9_gate_and_two_counters();
    test_case9_sets_latch_on_empty_records();
    test_case10_clears_only_slot_one_and_latches_even_empty();
    test_case11_native_gates_and_callback_order();
    return 0;
}

