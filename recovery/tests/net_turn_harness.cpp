// Ignored host projection. Test drivers inject the actual annotated/draft
// definitions; this is not application ABI, native matching, or acceptance.
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <map>

#ifndef __cdecl
#define __cdecl
#endif
#ifndef nullptr
#define nullptr 0
#endif
typedef uint32_t DWORD;
static const int MaxPlayerNum = 8;
static const int MaxVehProtoNum = 512;
static const int NetTurnChecksumWords = 21;

struct Event { std::string name; int a,b,c,d; };
static std::vector<Event> events;
static void event(const char *n, int a=0, int b=0, int c=0, int d=0) {
    Event e = {n,a,b,c,d}; events.push_back(e);
}
static int count_event(const char *n) {
    int count=0; for(size_t i=0;i<events.size();++i) if(events[i].name==n) ++count; return count;
}
static int first_event(const char *n) {
    for(size_t i=0;i<events.size();++i) if(events[i].name==n) return (int)i; return -1;
}

struct PlayerLock {
    uint8_t active_;
    uint32_t entry_flags_[2];
    int active() const {
        return ((entry_flags_[0] | entry_flags_[1]) & 1u) != 0;
    }
} ;
struct AlphaNet { void close() { event("net_close"); } };
struct LockProjection {
    PlayerLock records_[8];
    int any_locks() { event("any_locks"); return 0; }
    void clear() { event("locks_clear"); }
};
struct LockEntry {
    uint32_t leading_words_[4];
    struct { uint32_t flags; } payload_;
    uint32_t trailing_words_[10];
    LockEntry() { memset(this,0,sizeof(*this)); }
};
struct NetDaemonProjection : AlphaNet {
    LockEntry lock_state_[3]; LockProjection locks_; int player_count_;
    int clear_playerlock_on_receive;
    NetDaemonProjection() : player_count_(1), clear_playerlock_on_receive(0) {}
    void net_tasks() { event("net_tasks"); }
    void hang_up() { event("hang_up"); }
    int receive_value;
    int receive() { event("receive"); if(clear_playerlock_on_receive){locks_.records_[1].active_=1;clear_playerlock_on_receive=0;} if(receive_value>0){--receive_value;return 1;} return 0; }
    void synch(int a,int b,int c,int d,int e,int f,int g) { event("synch",a,b,c,g); }
} NetDaemonNet;

struct ConsoleProjection {
    int faction_id_;
    uint32_t field_23BE4_;
    uint32_t field_23BE8_;
    ConsoleProjection() : faction_id_(1) {}
    void set_view(int x) { event("set_view",x); }
    void set_center(int x,int y,int z) { event("set_center",x,y,z); }
    void set_cursor(int x,int y) { event("set_cursor",x,y); }
    void human_turn() { event("human_turn"); }
    void set_ai(int f) { event("set_ai",f); }
} ConsoleGlobal;
struct Base {
    int16_t x, y;
    uint8_t faction_id_current, faction_id_former;
    char through_state[0x2A];
    uint32_t state;
} Bases[128];
struct Vehicle { uint32_t state; uint8_t faction_id; int8_t order; int x,y; } Vehs[2049];
struct VehPrototype {
    char through_obsolete[0x2C];
    uint8_t obsolete_factions;
    uint8_t combat_factions, icon_offset, padding;
    uint16_t flags;
    int16_t preq_tech;
} VehPrototypes[MaxVehProtoNum];
typedef char VerifyVehPrototypeNativeStride[sizeof(VehPrototype)==0x34 ? 1 : -1];
struct Player { char *name_leader; } Players[MaxPlayerNum+1];
struct PlayerData { int earned_techs_saved, tech_accumulated, tech_id_researching; uint32_t flags; } PlayersData[MaxPlayerNum+1];
struct DeletionProjection { int cleared[24]; void clear(int i) { assert(i>=0&&i<24);++cleared[i];event("delete_clear",i); } } VehDeletionList;
struct FX { void play(int i) { event("fx",i); } } g_FX;
struct Dialogs { void item(char*,int) {} };
struct MultiWin { void draw(int i) { event("multi_draw",i); } } MultiWindow;
struct MessageWin { int clear_count; void clear(){++clear_count;event("message_clear");} } InterfaceMessageWin;
struct MessageWinNetworkProjection { int count; };
struct Diplo { void close(){event("diplo_close");} } DiploWindow;
struct TutWindow { int visible; int is_visible(){return visible;} void exit(){event("tut_exit");} } TutWinBaseWindow;
struct NetMsg { void pop(const char*,int,int,...){event("msg_pop");} void close(){event("msg_close");} } NetMsgPrimary;
struct StringTableProjection { char *get(int i) { static char value[]="label"; event("string_get",i); return value; } } StringTable;
static char *label_get(int i) { return StringTable.get(i); }
static char StringTemp[1032];
static char NetControlTurnClearTurnSemaphore[]="(Clearing turn semaphore)";
static char NetControlTurnMultiDebugPreference[]="Multi Debug";
static char NetControlTurnMultiDebugPopup[]="MULTIDEBUG";
static char NetControlTurnTurnLogDirectory[]="k:\\breynolds\\log";
static char NetControlTurnTextFileSuffix[]=".txt";
static char NetControlTurnFallbackTextFileSuffix[]=".txt";
static char NetControlTurnFallbackLogPrefix[]="log";
static char NetControlTurnTextFileMode[]="wt";
static char NetEndTurnEnterEndTurnLog[]="Entering 'end of turn' loop";
static char NetEndTurnFinishingTurnLabel[]="FINISHINGTURN";
static char NetEndTurnReadyToProceedLog[]="Ready to proceed";
static char NetEndTurnProceedingToNextTurnLog[]="--- Proceeding to next turn ---";
static char NetEndTurnSaveDirectory[]="k:\\breynolds\\multisaves\\";
static char NetEndTurnSaveGamePrefix[]="AC";
static char NetEndTurnSaveGameSuffix[]="end";
static char NetEndTurnSaveFileLog[]="Writing save file";
static char NetUpkeepSkippingUpkeepSynch[]="Skipping upkeep synch";
static char NetUpkeepSkippingResearchSynch[]="Skipping upkeep synch";
static char NetUpkeepSkippingPrototypeSynch[]="Skipping upkeep synch";
static char NetUpkeepUpkeepAdvance[]="Upkeep Advance";
static char NetUpkeepSetResearch[]="Set Research";
static char NetUpkeepUpkeep4[]="UPKEEP4";
static char NetUpkeepUpkeep5[]="UPKEEP5";
static char NetUpkeepEnemyTurn[]="Enemy turn ";
static char NetUpkeepUpkeep6[]="UPKEEP6";
static char NetUpkeepSaveDirectory[]="k:\\breynolds\\multisaves\\";
static char NetUpkeepSavePrefix[]="AC";
static char NetUpkeepSaveSuffix[]="multi";
static char NetUpkeepSaveLog[]="Writing save file";
static char NetUpkeepUpkeep7[]="UPKEEP7";
static char NetUpkeepGameChecksum[]="Failed GAME checksum - resynch";
static char NetUpkeepLeadersChecksum[]="Failed LEADERS checksum - resynch";
static char NetUpkeepMapChecksum[]="Failed MAP checksum - resynch";
static char NetUpkeepVehiclesChecksum[]="Failed VEHICLES checksum - resynch";
static char NetUpkeepBasesChecksum[]="Failed BASES checksum - resynch";
static char NetUpkeepMapSectionChecksum[]="Failed MAP section chcksum - resynch";
static char NetUpkeepSkippingUpkeepPhase[]="Skipping upkeep phase";
static int NetTurnFlags, NetTurnFaction, NextTurnReady, NetUpkeepPhase, TurnUpkeepPhase;
static int TurnCurrentNum, TurnControlDurationMilliseconds, GameState, GamePreferences, GameRules, GameMorePreferences;
static int ConsoleControlTurnActive, ConsoleFactionsWaitingMask, ControlTurnPhase, MapBaseDisplayOverride;
static int BaseCurrentCount, VehCurrentCount, ExitTurnLoop, IsMultiplayerNet, MessageWinNetworkMessageCount;
static int NetDaemonWaitTicks, LockEnableMask, MapRandSeed, MapLatitudeBounds=256, MapLongitudeBounds=256;
static int BaseAtDebugPopupFlag, CommlinkPending, SpockHelpActive, SpockNotifyPending;
static int DiploDefaultFlag93A968;
static int TurnControlWord93A954;
static uint32_t UpgradePromptAnswered;
static int TurnRedrawPending, CouncilPendingMessageArgs[3]={-1,-1,-1};
static int NetTurnChecksums[NetTurnChecksumWords];
static int NetPeerTurnChecksums[MaxPlayerNum][NetTurnChecksumWords];
static int g_time_ms;
static uint32_t timeGetTime() { return (uint32_t)g_time_ms; }
static int _itoa(int n,char *b,int) { return sprintf(b,"%d",n); }
static int my_itoa(int n,char *b,int) { return sprintf(b,"%d",n); }
static void draw_map(int n){event("draw_map",n);} static void my_srand(int n){event("srand",n);}
static int prefs_get(const char*,int,int){event("prefs_get");return 0;}
static int X_pop(const char*,int){event("x_pop");return 0;}
struct Logger { void init(char*){event("log_init");} void reset(){event("log_reset");} } Logging;
static void log_set_state(int n){event("log_state",n);}
static void log_say(const char*, ...){event("log_say");}
static void do_checksums(int n){event("checksums",n);}
static void turn_upkeep(){event("turn_upkeep");}
static void set_time_controls(){event("time_controls");TurnControlDurationMilliseconds=1000;}
static int EndGameResult;
static int end_of_game(int){event("end_game");return EndGameResult;}
static int net_upkeep_phase_result[32]; static int net_upkeep_phase_calls;
static int __cdecl net_upkeep_phase(int phase){event("upkeep_phase",phase);int i=net_upkeep_phase_calls++;return net_upkeep_phase_result[i];}
static void message_data(int id,int,int=0,int=0,int=0,int=0){event("message",id);}
static void message_big_data(int id,int,int*,int){event("big_message",id);}
static void synch_radius(int i){event("synch_radius",i);} static void synch_base(int i){event("synch_base",i);}
static void synch_leader(int i){event("synch_leader",i);} static void do_all_tasks(){event("all_tasks");}
static void veh_promote(int i){event("promote",i);} static void faction_upkeep(int i){event("faction_upkeep",i);}
static void tech_advance(int i){event("tech_advance",i);} static int tech_selection(int){event("tech_selection");return 2;}
static void consider_designs(unsigned int){event("consider_designs");}
static void upgrade_prototypes(int,int i){event("upgrade",i);} static void enemy_turn(int i){event("enemy_turn",i);}
static void auto_save(){event("auto_save");} static void save_daemon(char*){event("save");}
static void wait_task(){event("wait");NetDaemonNet.locks_.records_[1].active_=1;} static void go_reset(){event("go_reset");} static void mash_planes(){event("mash_planes");}
static int next_player_turn(){event("next_player_turn");return 0;} static void net_upkeep(); static void net_end_of_turn(); static void set_time_controls();
static int __cdecl game_logic_dummy();

/*EXTRACTED_NET_CONTROL_TURN*/
/*EXTRACTED_NET_END_OF_TURN*/
/*EXTRACTED_NET_UPKEEP*/

static void reset_events() {
    events.clear(); net_upkeep_phase_calls=0; memset(net_upkeep_phase_result,0,sizeof(net_upkeep_phase_result));
}
static int main_control_end_game() {
    reset_events(); LockEnableMask=0x0202; EndGameResult=1; net_upkeep_phase_result[0]=1;
    NetTurnFlags=0; ControlTurnPhase=0; ConsoleGlobal.faction_id_=1;
    IsMultiplayerNet=1; ConsoleFactionsWaitingMask=0;
    net_control_turn();
    assert(first_event("draw_map") < first_event("checksums"));
    assert(first_event("checksums") < first_event("turn_upkeep"));
    assert(first_event("turn_upkeep") < first_event("upkeep_phase"));
    assert(first_event("upkeep_phase") < first_event("end_game"));
    assert(count_event("hang_up")==1 && count_event("locks_clear")==1);
    assert(count_event("net_close")==1 && IsMultiplayerNet==0);
    assert(NetDaemonNet.lock_state_[0].leading_words_[1]==0);
    assert(NetDaemonNet.lock_state_[0].payload_.flags==0);
    return 0;
}
static int main_end_turn_network() {
    reset_events(); NetTurnFlags=0x10; IsMultiplayerNet=1; ExitTurnLoop=0;
    NextTurnReady=0; GameState=0; GamePreferences=0; ConsoleGlobal.faction_id_=2;
    ConsoleGlobal.field_23BE4_=42;
    NetDaemonNet.receive_value=0; NetUpkeepPhase=9; ConsoleControlTurnActive=0;
    net_end_of_turn();
    assert(count_event("message")==1 && first_event("message") < first_event("fx"));
    assert(GameState==2 && ConsoleGlobal.field_23BE4_==0);
    assert(first_event("tut_exit")<first_event("go_reset"));
    assert(first_event("go_reset")<first_event("mash_planes"));
    assert(first_event("mash_planes")<first_event("checksums"));
    assert(NetUpkeepPhase==0 && ConsoleControlTurnActive==1);
    assert(NetDaemonNet.lock_state_[0].leading_words_[0]==1);
    return 0;
}
static int main_end_turn_lock_mask() {
    reset_events(); NetTurnFlags=0; IsMultiplayerNet=0; ExitTurnLoop=0;
    NextTurnReady=0; GameState=0; GamePreferences=0; ConsoleGlobal.faction_id_=1;
    LockEnableMask=2; ConsoleFactionsWaitingMask=2;
    NetDaemonNet.player_count_=2; NetDaemonNet.lock_state_[0].leading_words_[1]=1;
    NetDaemonNet.locks_.records_[1].active_=2;
    NetDaemonNet.locks_.records_[1].entry_flags_[0]=1;
    NetDaemonNet.clear_playerlock_on_receive=1; NetDaemonNet.receive_value=0;
    net_end_of_turn();
    assert(count_event("wait")==1);
    assert(count_event("receive")>=2);
    return 0;
}
static int main_upkeep_clean() {
    reset_events(); NetTurnFlags=0; NetUpkeepPhase=0; TurnUpkeepPhase=0;
    LockEnableMask=0x0606; ConsoleGlobal.faction_id_=1; TurnCurrentNum=1;
    BaseCurrentCount=1; Bases[0].faction_id_current=1; Bases[0].state=0x10000010;
    VehCurrentCount=1; Vehs[0].faction_id=1; Vehs[0].state=0;
    NetDaemonNet.lock_state_[0].trailing_words_[9]=0;
    GamePreferences=0; GameMorePreferences=0x100000; GameRules=0;
    BaseAtDebugPopupFlag=0; CouncilPendingMessageArgs[0]=-1;
    UpgradePromptAnswered=99; TurnRedrawPending=0;
    for(int i=0;i<NetTurnChecksumWords;++i) { NetTurnChecksums[i]=i*17; NetPeerTurnChecksums[2][i]=i*17; }
    VehPrototypes[64].flags=0x101; VehPrototypes[64].obsolete_factions=0;
    VehPrototypes[511].flags=0x101; VehPrototypes[511].obsolete_factions=0;
    net_upkeep();
    assert(UpgradePromptAnswered==0);
    assert((VehPrototypes[64].flags&0x100)==0);
    assert((VehPrototypes[511].flags&0x100)==0);
    assert(count_event("promote")==1 && count_event("synch_radius")==1);
    assert(count_event("big_message")==1 && count_event("msg_close")==1);
    assert(count_event("delete_clear")==24);
    assert(count_event("upkeep_phase")>=5);
    return 0;
}
static int main_upkeep_checksum_cursor() {
    reset_events(); NetUpkeepPhase=0; LockEnableMask=0x0606;
    ConsoleGlobal.faction_id_=1; GamePreferences=0; GameMorePreferences=0;
    NetDaemonNet.lock_state_[0].leading_words_[1]=1;
    NetDaemonNet.lock_state_[0].leading_words_[1]=1;
    for(int i=0;i<NetTurnChecksumWords;++i) { NetTurnChecksums[i]=i+100; NetPeerTurnChecksums[2][i]=i+100; }
    NetTurnChecksums[20]^=1;
    net_upkeep();
    int found=0;
    for(size_t i=0;i<events.size();++i) if(events[i].name=="synch" && events[i].a==0x30 && events[i].b==15) found=1;
    assert(found); // word 20 is the last checksum: map section index 20-5.
    assert(count_event("msg_close")==1);
    return 0;
}
int main(int argc,char **argv) {
    assert(argc==2);
    if(strcmp(argv[1],"control")==0) return main_control_end_game();
    if(strcmp(argv[1],"end-turn")==0) return main_end_turn_network();
    if(strcmp(argv[1],"end-turn-lock-mask")==0) return main_end_turn_lock_mask();
    if(strcmp(argv[1],"upkeep-clean")==0) return main_upkeep_clean();
    if(strcmp(argv[1],"upkeep-checksum")==0) return main_upkeep_checksum_cursor();
    return 2;
}
