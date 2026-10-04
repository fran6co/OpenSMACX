// Standalone behavioral fixture. Product definitions are inserted verbatim by
// test_pact_ascent.py; this file contains only external owners and
// observable callback/state doubles.
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifndef __cdecl
#define __cdecl
#endif

typedef int BOOL;
enum { MaxPlayerNum = 8, FAC_BIOLOGY_LAB = 9 };

struct Player {
    int is_leader_female;
    char filename[24];
    char search_key[24];
    char name_leader[24];
    char title_leader[24];
    char adj_leader[128];
    char adj_insult_leader[128];
    char adj_faction[128];
    char adj_insult_faction[128];
    char padding1[128];
    char noun_faction[24];
    int noun_gender;
    int is_noun_plural;
    char padding2[0x59c - 0x304];
};
typedef char PlayerStride[(sizeof(Player) == 0x59c) ? 1 : -1];

struct PlayerData {
    uint32_t flags;
    unsigned char rest[0x20cc - sizeof(uint32_t)];
};
typedef char PlayerDataStride[(sizeof(PlayerData) == 0x20cc) ? 1 : -1];

struct FacilityRecord { int preq_tech; };
struct ConsoleRecord { int faction_id_; };
struct NetMessage {
    int calls;
    const char *label;
    int duration;
    int channel;
    const char *target;
    void pop(const char *message, int delay, int mode, const char *destination) {
        ++calls;
        label = message;
        duration = delay;
        channel = mode;
        target = destination;
    }
};

Player Players[MaxPlayerNum];
PlayerData PlayersData[MaxPlayerNum];
FacilityRecord Facility[32];
ConsoleRecord ConsoleGlobal;
NetMessage NetMsgPrimary;
int TurnCurrentNum;
int GenderDefault;
BOOL PluralityDefault;
char ScriptPopupTitleText[] = "CLIMATE";
char *ScriptPopupTitle = ScriptPopupTitleText;
char **const ClimatePopupTitle = &ScriptPopupTitle;

struct ParserEvent { int slot, gender, plurality; const char *text; };
static ParserEvent ParserEvents[8];
static int ParserCount;
static int EventLog[32];
static int EventCount;
static int TechResult;
static int AscendingResult;
static int ExpectedTech;
static int ExpectedTechFaction;
static int ExpectedAscendFaction;
static int ChangeFactionAfterTech;
static int FactionAfterTech;
static int ChangeFactionAfterAscending;
static int FactionAfterAscending;
static int ChangeFactionAfterTitle;
static int FactionAfterTitle;
static int InterludeCount;
static int PopupCount;
static const char *PopupTitle;
static const char *PopupText;
static const char *PopupImage;
static int PronounCalls;
static const char PronounResult[] = "pronoun-result";

static void reset_observations() {
    ParserCount = 0;
    EventCount = 0;
    TechResult = 0;
    AscendingResult = 0;
    ExpectedTech = -1;
    ExpectedTechFaction = -1;
    ExpectedAscendFaction = -1;
    ChangeFactionAfterTech = 0;
    FactionAfterTech = 0;
    ChangeFactionAfterAscending = 0;
    FactionAfterAscending = 0;
    ChangeFactionAfterTitle = 0;
    FactionAfterTitle = 0;
    InterludeCount = 0;
    PopupCount = 0;
    PopupTitle = 0;
    PopupText = 0;
    PopupImage = 0;
    PronounCalls = 0;
    NetMsgPrimary.calls = 0;
}

BOOL __cdecl has_tech(int tech, int faction) {
    assert(tech == ExpectedTech);
    assert(faction == ExpectedTechFaction);
    EventLog[EventCount++] = 1;
    if (ChangeFactionAfterTech) {
        ConsoleGlobal.faction_id_ = FactionAfterTech;
    }
    return TechResult;
}

int __cdecl ascending(int faction) {
    assert(faction == ExpectedAscendFaction);
    EventLog[EventCount++] = 2;
    if (ChangeFactionAfterAscending) {
        ConsoleGlobal.faction_id_ = FactionAfterAscending;
    }
    return AscendingResult;
}

void __cdecl interlude(int id, char *, int mode, int extra) {
    assert(id == 1 && mode == 1 && extra == 0);
    ++InterludeCount;
    EventLog[EventCount++] = 3;
}

int __cdecl parse_says(int slot, const char *text, int, int) {
    assert(ParserCount < 8);
    ParserEvents[ParserCount].slot = slot;
    ParserEvents[ParserCount].gender = GenderDefault;
    ParserEvents[ParserCount].plurality = PluralityDefault;
    ParserEvents[ParserCount].text = text;
    ++ParserCount;
    EventLog[EventCount++] = 4 + slot;
    if (slot == 0 && ChangeFactionAfterTitle) {
        ConsoleGlobal.faction_id_ = FactionAfterTitle;
    }
    return 0;
}

char *__cdecl get_he_she(int faction, int form) {
    assert(faction == 2 && form == 0);
    ++PronounCalls;
    EventLog[EventCount++] = 9;
    return (char *)PronounResult;
}

void __cdecl popp(const char *title, const char *text, int mode,
                  const char *image, int extra) {
    assert(mode == 0 && extra == 0);
    ++PopupCount;
    PopupTitle = title;
    PopupText = text;
    PopupImage = image;
    EventLog[EventCount++] = 10;
}

// @@PACT_TREATY_DEFINITION@@
// @@ASCENT_DEFINITION@@
// @@TURN_UPKEEP_ASCENT_FRAGMENT@@

static void initialize_players() {
    memset(Players, 0, sizeof(Players));
    memset(PlayersData, 0, sizeof(PlayersData));
    for (int i = 0; i != MaxPlayerNum; ++i) {
        sprintf(Players[i].name_leader, "name%d", i);
        sprintf(Players[i].title_leader, "title%d", i);
        sprintf(Players[i].noun_faction, "noun%d", i);
    }
}

static void check_pact_treaty() {
    initialize_players();
    reset_observations();
    Players[2].is_leader_female = 1;
    Players[2].noun_gender = 12;
    Players[2].is_noun_plural = 1;
    Players[5].noun_gender = 15;
    Players[5].is_noun_plural = 0;
    pact_treaty(2, 5);

    assert(ParserCount == 5 && PronounCalls == 1);
    assert(ParserEvents[0].slot == 0 && strcmp(ParserEvents[0].text, "title2") == 0);
    assert(ParserEvents[1].slot == 1 && strcmp(ParserEvents[1].text, "name2") == 0);
    assert(ParserEvents[2].slot == 2 && strcmp(ParserEvents[2].text, "noun2") == 0);
    assert(ParserEvents[3].slot == 3 && ParserEvents[3].text == PronounResult);
    assert(ParserEvents[4].slot == 4 && strcmp(ParserEvents[4].text, "noun5") == 0);
    assert(ParserEvents[0].gender == 1 && ParserEvents[0].plurality == 0);
    assert(ParserEvents[1].gender == 1 && ParserEvents[1].plurality == 0);
    assert(ParserEvents[2].gender == 12 && ParserEvents[2].plurality == 1);
    assert(ParserEvents[3].gender == 12 && ParserEvents[3].plurality == 1);
    assert(ParserEvents[4].gender == 15 && ParserEvents[4].plurality == 0);
    assert(NetMsgPrimary.calls == 1);
    assert(strcmp(NetMsgPrimary.label, "PACTTREATY") == 0);
    assert(NetMsgPrimary.duration == 5000 && NetMsgPrimary.channel == 0);
    assert(NetMsgPrimary.target == 0);
}

static void set_ascent_case(int year, int faction) {
    initialize_players();
    reset_observations();
    TurnCurrentNum = year;
    ConsoleGlobal.faction_id_ = faction;
    Facility[FAC_BIOLOGY_LAB].preq_tech = 33;
    Players[faction].is_leader_female = 1;
    Players[4].is_leader_female = 0;
    ExpectedTech = 33;
    ExpectedTechFaction = faction;
    ExpectedAscendFaction = faction;
}

static void check_ascent_body(void (*body)()) {
    set_ascent_case(74, 2);
    AscendingResult = 0;
    ExpectedTechFaction = 2;
    ExpectedAscendFaction = 2;
    body();
    assert(InterludeCount == 0 && ParserCount == 0 && PopupCount == 0);

    set_ascent_case(75, 2);
    TechResult = 1;
    AscendingResult = 0;
    body();
    assert(InterludeCount == 1 && ParserCount == 0 && PopupCount == 0);
    assert(EventLog[0] == 1 && EventLog[1] == 3 && EventLog[2] == 2);

    set_ascent_case(76, 2);
    AscendingResult = 1;
    PlayersData[2].flags = 0x2000;
    body();
    assert(ParserCount == 0 && PopupCount == 0);

    set_ascent_case(75, 2);
    TechResult = 1;
    AscendingResult = 1;
    ChangeFactionAfterTech = 1;
    FactionAfterTech = 1;
    ExpectedAscendFaction = 1;
    ChangeFactionAfterAscending = 1;
    FactionAfterAscending = 3;
    ChangeFactionAfterTitle = 1;
    FactionAfterTitle = 4;
    PlayersData[2].flags = 0x200;
    Players[3].is_leader_female = 1;
    Players[4].is_leader_female = 0;
    body();
    assert(PlayersData[2].flags == 0x200);
    assert((PlayersData[3].flags & 0x2000) != 0);
    assert((PlayersData[3].flags & 0x200) == 0);
    assert(ParserCount == 2 && PopupCount == 1);
    assert(ParserEvents[0].slot == 0 && strcmp(ParserEvents[0].text, "title3") == 0);
    assert(ParserEvents[1].slot == 1 && strcmp(ParserEvents[1].text, "name4") == 0);
    assert(ParserEvents[0].gender == 1 && ParserEvents[0].plurality == 0);
    assert(ParserEvents[1].gender == 0 && ParserEvents[1].plurality == 0);
    assert(strcmp(PopupTitle, "CLIMATE") == 0);
    assert(strcmp(PopupText, "ASCENT") == 0);
    assert(strcmp(PopupImage, "asctran_sm.pcx") == 0);
    assert(EventLog[0] == 1 && EventLog[1] == 3 && EventLog[2] == 2);
}

int main() {
    check_pact_treaty();
    check_ascent_body(&ascent);
    check_ascent_body(&turn_upkeep_ascent_control);
    return 0;
}

