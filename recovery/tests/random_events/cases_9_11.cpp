#include <cassert>
#include <cstring>
#include <vector>

struct PlayerData {
    unsigned int satellites_[4];
};

struct RandomEventsFields {
    int global_energy_event_state_;
    int sunspot_duration_;
    int turn_current_num_;
};

struct RandomEventsGameState {
    RandomEventsFields fields_;
};

static RandomEventsGameState GameDataStateStorage;
static PlayerData PlayersData[8];
static const int MaxPlayerNum = 8;

static std::vector<int> random_results;
static std::vector<int> random_highs;
static std::vector<int> callback_order;
static int random_index;
static int parse_calls;
static int popup_calls;
static int popup_arg;
static char popup_caption[32];
static char popup_image[32];

static int random(int low, int high) {
    assert(low == 0);
    assert(random_index < static_cast<int>(random_results.size()));
    random_highs.push_back(high);
    callback_order.push_back(1);
    return random_results[static_cast<std::size_t>(random_index++)];
}

static int parse_num(int low, int high) {
    assert(low == 0 && high == 20);
    assert(GameDataStateStorage.fields_.sunspot_duration_ >= 10);
    assert(GameDataStateStorage.fields_.sunspot_duration_ <= 20);
    ++parse_calls;
    callback_order.push_back(2);
    return high;
}

static int POP2(const char *caption, const char *image, int argument) {
    assert(popup_calls == 0);
    if (std::strcmp(caption, "SOLARSTORM") == 0 ||
        std::strcmp(caption, "SOLARFLARE") == 0 ||
        std::strcmp(caption, "ASTEROID") == 0) {
        // The mapped cases set the global latch before invoking this callback.
        assert(GameDataStateStorage.fields_.global_energy_event_state_ == 1);
    }
    assert(GameDataStateStorage.fields_.sunspot_duration_ >= 10 ||
           std::strcmp(caption, "SUNSPOTS") != 0);
    std::strcpy(popup_caption, caption);
    std::strcpy(popup_image, image);
    popup_arg = argument;
    ++popup_calls;
    callback_order.push_back(3);
    return 0;
}

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
#include "actual_cases_9_11.hpp"
#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

static void reset() {
    std::memset(PlayersData, 0, sizeof(PlayersData));
    GameDataStateStorage.fields_.global_energy_event_state_ = 0;
    GameDataStateStorage.fields_.sunspot_duration_ = -40;
    GameDataStateStorage.fields_.turn_current_num_ = 50;
    random_results.clear();
    random_highs.clear();
    callback_order.clear();
    random_index = parse_calls = popup_calls = popup_arg = 0;
    popup_caption[0] = popup_image[0] = '\0';
}

static void test_case9_latch_gate_and_playable_faction_slots() {
    reset();
    PlayersData[0].satellites_[2] = 4; // Native starts at player record 1.
    PlayersData[1].satellites_[2] = 7;
    PlayersData[6].satellites_[3] = 9;

    GameDataStateStorage.fields_.global_energy_event_state_ = 2;
    random_events_native_cases_9_11(9);
    assert(random_highs.empty());
    assert(PlayersData[1].satellites_[2] == 7);

    GameDataStateStorage.fields_.global_energy_event_state_ = 0;
    random_results.push_back(1);
    random_events_native_cases_9_11(9);
    assert(random_highs.size() == 1 && random_highs[0] == 5);
    assert(PlayersData[1].satellites_[2] == 7);
    assert(GameDataStateStorage.fields_.global_energy_event_state_ == 0);
    assert(popup_calls == 0);

    random_results.push_back(0);
    random_events_native_cases_9_11(9);
    assert(GameDataStateStorage.fields_.global_energy_event_state_ == 1);
    assert(PlayersData[0].satellites_[2] == 4);
    assert(PlayersData[1].satellites_[2] == 0 &&
           PlayersData[1].satellites_[3] == 0);
    assert(PlayersData[6].satellites_[2] == 0 &&
           PlayersData[6].satellites_[3] == 0);
    assert(popup_calls == 1);
    assert(std::strcmp(popup_caption, "SOLARSTORM") == 0);
    assert(std::strcmp(popup_image, "sun_sm.pcx") == 0);
}

static void test_case9_sets_latch_and_uses_unchanged_popup() {
    reset();
    random_results.push_back(0);
    random_events_native_cases_9_11(9);
    assert(GameDataStateStorage.fields_.global_energy_event_state_ == 1);
    assert(popup_calls == 1);
    assert(std::strcmp(popup_caption, "SOLARFLARE") == 0);
    assert(std::strcmp(popup_image, "heat_sm.pcx") == 0);
    assert(callback_order.size() == 2 && callback_order[0] == 1 &&
           callback_order[1] == 3);
}

static void test_case10_clears_only_satellite_slot_one_and_latches() {
    reset();
    PlayersData[0].satellites_[1] = 3;
    PlayersData[2].satellites_[1] = 5;
    PlayersData[3].satellites_[2] = 8;
    random_results.push_back(0);
    random_events_native_cases_9_11(10);
    assert(GameDataStateStorage.fields_.global_energy_event_state_ == 1);
    assert(PlayersData[0].satellites_[1] == 3);
    assert(PlayersData[2].satellites_[1] == 0);
    assert(PlayersData[3].satellites_[2] == 8);
    assert(popup_calls == 1);
    assert(std::strcmp(popup_caption, "ASTEROID") == 0);
    assert(std::strcmp(popup_image, "astm_sm.pcx") == 0);

    reset();
    random_results.push_back(0);
    random_events_native_cases_9_11(10);
    assert(GameDataStateStorage.fields_.global_energy_event_state_ == 1);
    assert(popup_calls == 0); // Case 10 has no unchanged-result popup.
}

static void test_case11_gates_and_store_parse_popup_order() {
    reset();
    GameDataStateStorage.fields_.sunspot_duration_ = -39;
    GameDataStateStorage.fields_.turn_current_num_ = 49;
    random_events_native_cases_9_11(11);
    assert(random_highs.empty());
    assert(parse_calls == 0 && popup_calls == 0);

    GameDataStateStorage.fields_.turn_current_num_ = 80;
    GameDataStateStorage.fields_.sunspot_duration_ = -50;
    random_results.push_back(1); // Native retry gate rejects the event.
    random_events_native_cases_9_11(11);
    assert(GameDataStateStorage.fields_.sunspot_duration_ == -50);
    assert(popup_calls == 0 && parse_calls == 0);

    random_results.push_back(0); // Retry gate opens.
    random_results.push_back(6); // Stored duration becomes 16.
    random_events_native_cases_9_11(11);
    assert(GameDataStateStorage.fields_.sunspot_duration_ == 16);
    assert(random_highs.size() == 3);
    assert(random_highs[0] == 3 && random_highs[1] == 3 &&
           random_highs[2] == 11);
    assert(parse_calls == 1 && popup_calls == 1 && popup_arg == -1);
    assert(std::strcmp(popup_caption, "SUNSPOTS") == 0);
    assert(std::strcmp(popup_image, "solar_sm.pcx") == 0);
    assert(callback_order.size() == 5);
    assert(callback_order[2] == 1 && callback_order[3] == 2 &&
           callback_order[4] == 3);
}

int main() {
    test_case9_latch_gate_and_playable_faction_slots();
    test_case9_sets_latch_and_uses_unchanged_popup();
    test_case10_clears_only_satellite_slot_one_and_latches();
    test_case11_gates_and_store_parse_popup_order();
    return 0;
}
