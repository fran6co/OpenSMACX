#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#define __cdecl
static const int MaxPlayerNum = 8;
enum { RFLAG_ALIEN = 0x80 };

struct Player { char filename[24]; int rule_flags; };
struct PlayerData { uint32_t flags; int diff_level; int current_num_bases; };
struct Base { uint8_t faction_id_current; };
struct Veh { uint8_t faction_id; };
struct Console { int faction_id_; };

extern Player Players[MaxPlayerNum];
extern PlayerData PlayersData[MaxPlayerNum];
extern Base Bases[16];
extern int BaseCurrentCount;
extern Veh Vehs[16];
extern int VehCurrentCount;
extern Console ConsoleGlobal;
extern int TurnCurrentNum;
extern int ExpansionEnabled;
extern int IsMultiplayerPBEM;
extern int IsMultiplayerNet;
extern uint32_t LockEnableMask;
extern char StringTemp[1032];
char *CharUpperA(char *text);
void base_kill(int id);
void veh_kill(int id);
