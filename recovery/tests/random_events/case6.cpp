#include <assert.h>
#include <string.h>
#include "actual_case6.hpp"

struct TestBase {
    int x, y, faction, population, visibility;
    bool facilities[40];
};
struct State {
    int turn, local, pbem, longitude, half, flat, count;
    TestBase bases[8];
    int turn_current_num() const { return turn; }
    int local_faction() const { return local; }
    int multiplayer_pbem() const { return pbem; }
    int longitude_bounds() const { return longitude; }
    int longitude_half() const { return half; }
    int map_is_flat() const { return flat; }
    int base_count() const { return count; }
};
struct Services {
    State &s;
    int ranking[8], projects[32];
    int notices[8], notice_args[8], notice_count;
    int parses, parsed_facility, parsed_gender, draws, focuses, parsed_low, parsed_high;
    Services(State &state) : s(state), notice_count(0), parses(0),
        parsed_facility(-1), parsed_gender(-2), draws(0), focuses(0),
        parsed_low(0), parsed_high(0) {
        memset(ranking, 0, sizeof(ranking));
        memset(projects, 0xFF, sizeof(projects));
        memset(notices, 0, sizeof(notices));
        memset(notice_args, 0, sizeof(notice_args));
    }
    bool base_has_facility(int id, int facility) const {
        return s.bases[id].facilities[facility];
    }
    int secret_project_owner(int slot) const { return projects[slot]; }
    int base_faction(int id) const { return s.bases[id].faction; }
    int faction_ranking(int f) const { return ranking[f]; }
    int base_population(int id) const { return s.bases[id].population; }
    int base_visibility(int id) const { return s.bases[id].visibility; }
    int base_x(int id) const { return s.bases[id].x; }
    int base_y(int id) const { return s.bases[id].y; }
    int parse_num(int low, int high) {
        ++parses; parsed_low = low; parsed_high = high; return low;
    }
    void parse_facility_say(int facility, int gender) {
        parsed_facility = facility; parsed_gender = gender;
    }
    void focus(int, int, int) { ++focuses; }
    void draw_base(int) { ++draws; }
    void set_base_population(int id, int value) { s.bases[id].population = value; }
    void notify(int notice, int argument) {
        notices[notice_count] = notice;
        notice_args[notice_count++] = argument;
    }
};

static void initialize(State &s) {
    memset(&s, 0, sizeof(s));
    s.turn = 100; s.local = 1; s.longitude = 100; s.half = 50;
    s.flat = 1; s.count = 3;
    s.bases[0].x = 10; s.bases[0].y = 10;
    s.bases[0].faction = 2; s.bases[0].population = 6;
    s.bases[0].visibility = 1u << 1;
    s.bases[1].x = 12; s.bases[1].y = 10;
    s.bases[1].faction = 1; s.bases[1].population = 7;
    s.bases[2].x = 18; s.bases[2].y = 10;
    s.bases[2].faction = 3; s.bases[2].population = 5;
}

static void test_population_radius_and_local_focus() {
    State s; initialize(s); Services ops(s); ops.ranking[2] = 4;
    random_events_cases::case6(s, ops, 2, 0, 9);
    assert(ops.parses == 1 && ops.parsed_low == 1 && ops.parsed_high == 3);
    assert(s.bases[0].population == 3); // selected base is also scanned
    assert(s.bases[1].population == 4); // ceil(7 / 2)
    assert(s.bases[2].population == 5); // outside the selected radius
    assert(ops.draws == 2 && ops.focuses == 2);
    assert(ops.notice_count == 1);
    assert(ops.notices[0] == random_events_cases::CASE6_NEARBY_BASES_AFFECTED);
    assert(ops.notice_args[0] == 0);
}

static void test_rank_threshold_and_round_wrap() {
    State s; initialize(s); Services ops(s); ops.ranking[2] = 4;
    s.turn = 150;
    random_events_cases::case6(s, ops, 2, 0, 9);
    assert(ops.parses == 0 && ops.draws == 0);

    initialize(s); Services round_ops(s); round_ops.ranking[2] = 4;
    s.flat = 0; s.longitude = 80; s.half = 40;
    s.bases[1].x = 98; s.bases[1].y = 10; // wrapped dx = 12, in range
    random_events_cases::case6(s, round_ops, 2, 0, 9);
    assert(s.bases[0].population == 3 && s.bases[1].population == 4 &&
           round_ops.draws == 2);
}

static void test_facility_and_project_immunity() {
    State s; initialize(s); Services ops(s); ops.ranking[2] = 4;
    s.bases[1].facilities[17] = true;
    ops.projects[0] = 1; // same candidate faction owns project
    random_events_cases::case6(s, ops, 2, 0, 9);
    assert(s.bases[1].population == 7 && ops.draws == 1);

    initialize(s); Services facility_ops(s);
    s.local = 2;
    s.bases[0].facilities[18] = true;
    random_events_cases::case6(s, facility_ops, 2, 0, 9);
    assert(facility_ops.parsed_facility == 28);
    assert(facility_ops.notices[0] == random_events_cases::CASE6_FACILITY_LOCAL);
}

static void test_project_precedence_and_pbem_notification() {
    State s; initialize(s); s.local = 1; s.pbem = 1;
    Services ops(s); ops.projects[14] = 0;
    random_events_cases::case6(s, ops, 2, 0, 9);
    assert(ops.parsed_facility == 84);
    assert(ops.notices[0] == random_events_cases::CASE6_PROJECT_PBEM);
    assert(ops.parses == 0); // direct project branch bypasses radius parse
}

int main() {
    test_population_radius_and_local_focus();
    test_rank_threshold_and_round_wrap();
    test_facility_and_project_immunity();
    test_project_precedence_and_pbem_notification();
}

