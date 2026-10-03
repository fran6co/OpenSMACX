#include "fixture.h"
#include <stdio.h>

Player Players[MaxPlayerNum];
PlayerData PlayersData[MaxPlayerNum];
Base Bases[16];
int BaseCurrentCount;
Veh Vehs[16];
int VehCurrentCount;
Console ConsoleGlobal;
int TurnCurrentNum;
int ExpansionEnabled;
int IsMultiplayerPBEM;
int IsMultiplayerNet;
uint32_t LockEnableMask;
char StringTemp[1032];

static int base_kills[16];
static int base_kill_count;
static int veh_kills[16];
static int veh_kill_count;

char *CharUpperA(char *text) {
    for (char *p = text; *p; ++p) {
        if (*p >= 'a' && *p <= 'z') *p = static_cast<char>(*p - 'a' + 'A');
    }
    return text;
}

void base_kill(int id) {
    base_kills[base_kill_count++] = id;
    for (int i = id; i + 1 < BaseCurrentCount; ++i) Bases[i] = Bases[i + 1];
    --BaseCurrentCount;
}

void veh_kill(int id) {
    veh_kills[veh_kill_count++] = id;
    for (int i = id; i + 1 < VehCurrentCount; ++i) Vehs[i] = Vehs[i + 1];
    --VehCurrentCount;
}

#include "actual.inc"

static void reset() {
    memset(Players, 0, sizeof Players);
    memset(PlayersData, 0, sizeof PlayersData);
    memset(Bases, 0, sizeof Bases);
    memset(Vehs, 0, sizeof Vehs);
    memset(base_kills, 0, sizeof base_kills);
    memset(veh_kills, 0, sizeof veh_kills);
    base_kill_count = veh_kill_count = 0;
    BaseCurrentCount = VehCurrentCount = 0;
    ConsoleGlobal.faction_id_ = 0;
    TurnCurrentNum = 0;
    ExpansionEnabled = 1;
    IsMultiplayerPBEM = IsMultiplayerNet = 0;
    LockEnableMask = 0;
    strcpy(Players[0].filename, "HUMAN");
}

int main() {
    reset();
    // Ineligible calls must not mutate faction or world state.
    TurnCurrentNum = 1;
    Players[2].rule_flags = RFLAG_ALIEN;
    alien_start();
    if (PlayersData[2].flags != 0 || LockEnableMask != 0) return 1;

    reset();
    strcpy(Players[0].filename, "FUNGBOY");
    Players[2].rule_flags = RFLAG_ALIEN;
    alien_start();
    if (PlayersData[2].flags != 0 || LockEnableMask != 0) return 2;

    reset();
    ExpansionEnabled = 0;
    Players[2].rule_flags = RFLAG_ALIEN;
    alien_start();
    if (PlayersData[2].flags != 0 || LockEnableMask != 0) return 9;

    reset();
    Players[0].rule_flags = RFLAG_ALIEN;
    Players[2].rule_flags = RFLAG_ALIEN;
    alien_start();
    if (PlayersData[2].flags != 0 || LockEnableMask != 0) return 10;

    reset();
    Players[2].rule_flags = RFLAG_ALIEN;
    strcpy(Players[4].filename, "FUNGBOY");
    Players[3].rule_flags = RFLAG_ALIEN; // human mask suppresses faction 3
    LockEnableMask = 0x1C08U;
    PlayersData[2].flags = 0x1234;
    PlayersData[2].diff_level = 4;
    PlayersData[2].current_num_bases = 9;
    PlayersData[4].flags = 0x5678;
    PlayersData[4].diff_level = 5;
    PlayersData[4].current_num_bases = 7;
    BaseCurrentCount = 4;
    Bases[0].faction_id_current = 2;
    Bases[1].faction_id_current = 2;
    Bases[2].faction_id_current = 3;
    Bases[3].faction_id_current = 4;
    VehCurrentCount = 4;
    Vehs[0].faction_id = 2;
    Vehs[1].faction_id = 2;
    Vehs[2].faction_id = 3;
    Vehs[3].faction_id = 4;
    alien_start();
    if (base_kill_count != 3 || base_kills[0] != 1 ||
        base_kills[1] != 0 || base_kills[2] != 1) {
        fprintf(stderr, "base count=%d ids=%d,%d,%d remaining=%d mask=%x\n",
                base_kill_count, base_kills[0], base_kills[1], base_kills[2],
                BaseCurrentCount, LockEnableMask);
        return 3;
    }
    if (veh_kill_count != 3 || veh_kills[0] != 1 ||
        veh_kills[1] != 0 || veh_kills[2] != 1) return 4;
    if (PlayersData[2].flags != 0xFFFFFFFFU ||
        PlayersData[2].diff_level != -1 ||
        PlayersData[2].current_num_bases != 0) return 5;
    if (PlayersData[4].flags != 0xFFFFFFFFU ||
        PlayersData[4].diff_level != -1 ||
        PlayersData[4].current_num_bases != 0) return 6;
    if (LockEnableMask != 0x0808U) return 7;

    reset();
    IsMultiplayerNet = 1;
    Players[2].rule_flags = RFLAG_ALIEN;
    BaseCurrentCount = 1;
    Bases[0].faction_id_current = 2;
    alien_start();
    if (BaseCurrentCount != 1 || PlayersData[2].flags != 0) return 8;

    reset();
    IsMultiplayerPBEM = 1;
    Players[2].rule_flags = RFLAG_ALIEN;
    BaseCurrentCount = 1;
    Bases[0].faction_id_current = 2;
    alien_start();
    if (BaseCurrentCount != 1 || PlayersData[2].flags != 0) return 11;
    return 0;
}
