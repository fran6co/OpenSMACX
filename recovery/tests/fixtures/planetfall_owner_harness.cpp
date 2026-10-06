#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define __cdecl
enum { RFLAG_ALIEN = 0x80, MaxSecretProjectNum = 64, FacilitySPStart = 70 };
struct Player {
    int is_leader_female; char filename[24]; char search_key[24];
    char name_leader[24]; char title_leader[24]; char adj_leader[128];
    char adj_insult_leader[128]; char adj_faction[128]; char adj_insult_faction[128];
    unsigned char padding1[128]; char noun_faction[24]; int noun_gender;
    int is_noun_plural; char adj_name_faction[128]; int rule_flags;
};
struct RulesFacility { const char *name; };
struct Base { int faction_id_current; };
struct BaseSecretProject { int project_base[64]; int &operator[](int i) { return project_base[i]; } };
struct GameDataStateWindowFields { int turn_current_num_; BaseSecretProject secret_project_; };
struct GameDataStateWindow { GameDataStateWindowFields fields_; };
Player Players[8]; RulesFacility Facility[134]; Base Bases[256];
GameDataStateWindow GameDataStateStorage;
int GenderDefault, PluralityDefault, ExpansionEnabled;
char StringTemp[1032];
static int event_count, parse_ids[8], parse_genders[8], parse_pluralities[8];
static int say_ids[8], say_inputs[8], num_ids[8], num_values[8];
static char events[16][8], popup_key[1032];
void record_event(const char *event) { strcpy(events[event_count++], event); }
int parse_says(int id, const char *, int gender, int plurality) {
    if (gender < 0) gender = GenderDefault;
    if (plurality < 0) plurality = PluralityDefault;
    parse_ids[id] = id; parse_genders[id] = gender; parse_pluralities[id] = plurality;
    char event[8]; sprintf(event, "S%d", id); record_event(event); return 0;
}
int parse_say(int id, int input, int, int) {
    say_ids[0] = id; say_inputs[0] = input; record_event("I4"); return 0;
}
int parse_num(int id, int value) {
    num_ids[0] = id; num_values[0] = value; record_event("N0"); return 0;
}
char *CharUpperA(char *text) {
    for (char *p = text; *p; ++p) if (*p >= 'a' && *p <= 'z') *p -= ('a' - 'A');
    return text;
}
int has_project(int project, int faction) {
    int base = GameDataStateStorage.fields_.secret_project_[project];
    return base < 0 ? 0 : Bases[base].faction_id_current == faction;
}
int X_pop(const char *label, int (*)()) {
    strcpy(popup_key, label); record_event("X"); return 0;
}

/* INSERT INSTALLED PLANETFALL BODY HERE */

static void reset_state() {
    memset(Players, 0, sizeof(Players)); memset(Facility, 0, sizeof(Facility));
    memset(Bases, 0, sizeof(Bases)); memset(StringTemp, 0, sizeof(StringTemp));
    memset(events, 0, sizeof(events)); memset(parse_ids, 0, sizeof(parse_ids));
    memset(parse_genders, 0, sizeof(parse_genders)); memset(parse_pluralities, 0, sizeof(parse_pluralities));
    memset(say_ids, 0, sizeof(say_ids)); memset(say_inputs, 0, sizeof(say_inputs));
    memset(num_ids, 0, sizeof(num_ids)); memset(num_values, 0, sizeof(num_values));
    memset(GameDataStateStorage.fields_.secret_project_.project_base, 0xFF,
           sizeof(GameDataStateStorage.fields_.secret_project_.project_base));
    GameDataStateStorage.fields_.turn_current_num_ = 0;
    GenderDefault = PluralityDefault = 77; ExpansionEnabled = 0;
    event_count = 0; popup_key[0] = 0;
}
static void assert_parser_sequence(int leader_gender, int noun_gender, int noun_plural) {
    assert(event_count >= 5);
    assert(!strcmp(events[0], "S2") && !strcmp(events[1], "S3")
           && !strcmp(events[2], "S0") && !strcmp(events[3], "S1"));
    assert(parse_genders[2] == leader_gender && parse_pluralities[2] == 0);
    assert(parse_genders[3] == leader_gender && parse_pluralities[3] == 0);
    assert(parse_genders[0] == leader_gender && parse_pluralities[0] == 0);
    assert(parse_genders[1] == noun_gender && parse_pluralities[1] == noun_plural);
}
static void parser_order_project_scan_and_suffix() {
    reset_state(); Player &p = Players[2];
    p.is_leader_female = 1; p.noun_gender = 3; p.is_noun_plural = 1;
    strcpy(p.filename, "usurper"); strcpy(p.title_leader, "TITLE");
    strcpy(p.name_leader, "LEADER"); strcpy(p.adj_name_faction, "ADJECTIVE");
    strcpy(p.noun_faction, "NOUN"); p.rule_flags = RFLAG_ALIEN;
    GameDataStateStorage.fields_.turn_current_num_ = 7;
    GameDataStateStorage.fields_.secret_project_[0] = 4;
    GameDataStateStorage.fields_.secret_project_[1] = 5;
    GameDataStateStorage.fields_.secret_project_[2] = -1;
    Bases[4].faction_id_current = 2; Bases[5].faction_id_current = 3;
    Facility[70].name = "PROJECT_A"; Facility[71].name = "PROJECT_B";
    planetfall(2);
    assert_parser_sequence(1, 3, 1);
    assert(event_count == 7 && !strcmp(events[4], "N0")
           && !strcmp(events[5], "I4") && !strcmp(events[6], "X"));
    assert(num_ids[0] == 0 && num_values[0] == 7);
    assert(say_ids[0] == 4 && say_inputs[0] == (int)(uintptr_t)Facility[70].name);
    assert(!strcmp(StringTemp, "PLANETFALLX2") && !strcmp(popup_key, StringTemp));
    assert(GenderDefault == 3 && PluralityDefault == 1);
}
static void exact_case_insensitive_input_is_uppercased_and_zero_turn_has_no_suffix() {
    reset_state(); Player &p = Players[1];
    p.is_leader_female = 0; p.noun_gender = 2; p.is_noun_plural = 0;
    strcpy(p.filename, "fungboy"); strcpy(p.title_leader, "T");
    strcpy(p.name_leader, "N"); strcpy(p.adj_name_faction, "A");
    strcpy(p.noun_faction, "F"); p.rule_flags = 0;
    planetfall(1);
    assert_parser_sequence(0, 2, 0);
    assert(event_count == 5 && !strcmp(events[4], "X"));
    assert(!strcmp(StringTemp, "PLANETFALLF") && !strcmp(popup_key, StringTemp));
}
static void alien_test_uses_raw_flag_and_exact_filename_not_helper_gate_or_substring() {
    reset_state(); Player &p = Players[3];
    p.is_leader_female = 1; p.noun_gender = 1; p.is_noun_plural = 0;
    strcpy(p.filename, "xusurper"); strcpy(p.title_leader, "T");
    strcpy(p.name_leader, "N"); strcpy(p.adj_name_faction, "A");
    strcpy(p.noun_faction, "F"); p.rule_flags = RFLAG_ALIEN;
    // ExpansionEnabled remains zero: pinned code tests rule_flags directly.
    planetfall(3);
    assert_parser_sequence(1, 1, 0);
    assert(!strcmp(StringTemp, "PLANETFALL") && !strcmp(popup_key, StringTemp));
}
int main() {
    parser_order_project_scan_and_suffix();
    exact_case_insensitive_input_is_uppercased_and_zero_turn_has_no_suffix();
    alien_test_uses_raw_flag_and_exact_filename_not_helper_gate_or_substring();
}
