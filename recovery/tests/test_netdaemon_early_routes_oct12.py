"""Early process_message owner controls; host behavior is not native acceptance.

Definitions and passive records are read from the current product sources.
API shims expose callback boundaries using allocated packets and valid rows.
The pinned-image test independently checks the original branch/field/call
evidence. Neither host shims nor these bounded anchors prove full recovery,
Windows ABI, linked preservation, or game runtime behavior.
"""
from pathlib import Path
import os
import re
import shutil
import struct
import subprocess

import pytest

from test_netdaemon_synch_dispatch import _definition, _mask

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/netdaemon.cpp"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
HELPERS = (
    "netdaemon_process_sync_request", "netdaemon_handle_client_done_turn",
    "netdaemon_handle_server_turn_info", "netdaemon_handle_server_lock_status",
    "netdaemon_handle_client_done_upkeep", "netdaemon_handle_server_upkeep_info",
    "netdaemon_handle_diplo_cd_message", "netdaemon_handle_table_f",
    "netdaemon_run_contact_prompt", "netdaemon_handle_direct_diplomacy_complete",
)


def declaration(text, name):
    code = _mask(text)
    matches = list(re.finditer(r"\b(?:struct|union)\s+" + name + r"\s*\{", code))
    assert len(matches) == 1, name
    start, opening = matches[0].start(), matches[0].end() - 1
    depth = 0
    for end in range(opening, len(code)):
        depth += (code[end] == "{") - (code[end] == "}")
        if depth == 0:
            assert code[end + 1] == ";"
            return text[start:end + 2]
    raise AssertionError(name)


def source_material():
    source = SOURCE.read_text()
    functions = "\n".join(_definition(source, r"static\s+(?:void|bool|int)\s+" + h, h)
                          for h in HELPERS)
    net = (ROOT / "src/netdaemon.h").read_text()
    faction = (ROOT / "src/faction.h").read_text()
    alpha = (ROOT / "src/alpha.h").read_text()
    declarations = "\n".join((declaration(net, "PrimaryLockStateTail"),
                              declaration(net, "LockStateEntry"),
                              declaration(faction, "Player"),
                              declaration(alpha, "RulesTimeControl")))
    # The entire accessed game window is the actual passive source declaration.
    game = (ROOT / "src/game_data_state_window.h").read_text()
    base = (ROOT / "src/base.h").read_text()
    declarations += "\n" + declaration(base, "BaseSecretProject")
    for name in ("GameDataLockMaskBytes", "GameDataLockMaskView",
                 "GameDataStateWindowFields", "GameDataStateWindow"):
        declarations += "\n" + declaration(game, name)
    bonus = re.search(r"^\s*int time_bonus_count;", faction, re.M)
    assert bonus
    declarations += "\nstruct PlayerData {" + bonus.group(0) + " };"
    declarations += "\n" + declaration((ROOT / "src/alphanet.h").read_text(), "AlphaNetProcessSlot")
    # Exercise the real public field APIs, including the actual list member
    # used by picture_enabled, rather than replacing their result in a shim.
    pop = (ROOT / "src/basepop.h").read_text()
    methods = []
    for name in ("picture_enabled", "set_caller_picture"):
        match = re.search(r"(?:bool|void) " + name + r"\([^\n]*\)\s*(?:const\s*)?\{[^\n]*\}", pop)
        assert match, name
        methods.append(match.group(0))
    declarations += "\nstruct PictureAPIs { Dialogs dialogs_; Sprite *caller_sprite_;\n" + "\n".join(methods) + "\n};"
    diplo = (ROOT / "src/diplowin.h").read_text()
    state_fields = []
    for name in ("field_A24_", "field_A28_"):
        match = re.search(r"^\s*int32_t " + name + ";", diplo, re.M)
        assert match, name
        state_fields.append(match.group(0))
    setter = _definition(diplo, r"void\s+set_offer_state", "set_offer_state")
    assert "receive_no_deal" not in diplo
    assert "receive_no_deal" not in (ROOT / "src/diplowin.cpp").read_text()
    # These are actual private-owner scalar types and the actual inline setter;
    # the fixture's preceding storage below is not original layout evidence.
    return declarations, functions, "\n".join(state_fields), setter


PREAMBLE = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <functional>
#include <string>
#include <vector>
typedef int BOOL;
typedef char *LPSTR;
static std::vector<std::string> events;
static std::function<void(const std::string&)> callback;
static void event(const std::string &s) { events.push_back(s); if(callback) callback(s); }
static int GenderDefault, PluralityDefault;
struct Sprite { int id; };
struct SpriteBox {
    struct List { int entry_count_; } list_{};
    int sprite(Sprite *s, char *, int) { event("sprite:"+std::to_string(s?s->id:-1)); return 0; }
};
struct Dialogs : SpriteBox {};
/* ACTUAL DECLARATIONS */
static Player Players[8];
static PlayerData PlayersData[8];
static RulesTimeControl TimeControl[3];
static GameDataStateWindow GameDataStateStorage;
static int NetDaemonWaitTicks, NetUpkeepPhase, NextTurnReady;
static int TurnControlWord93A954, ConsoleControlTurnActive, SpockNotifyPending;
static unsigned int NetDaemonProgressInterval, NetDaemonProgressLastTick;
static int CommlinkPending, CommlinkFaction, Language, DiplomacyActive, PopupWaitActive;
struct { int faction_id_; } ConsoleGlobal;
static char PopupStartCaption[256];
static Sprite portraits[8];
static Sprite *FactionDiplomacyPortraits[8];
static int last_log[3], last_send[6], last_num, diplo_sender, who_arg, name_arg;
static std::vector<int> parsed_factions, parsed_genders, parsed_pluralities;
static int pop_wait() { return 0; }
static int pop_expire() { return 0; }
static void log_say(const char *s, int a, int b, int c) {
    last_log[0]=a; last_log[1]=b; last_log[2]=c; event(std::string("log:")+s);
}
static void message_data(int a,int b,int c,int d,int e,int f) {
    int x[]={a,b,c,d,e,f}; std::memcpy(last_send,x,sizeof x); event("send:"+std::to_string(a));
}
static void parse_says(int slot,const char *text,int,int) {
    int faction=-1;
    for(int i=0;i<8;++i) if(text==Players[i].title_leader || text==Players[i].name_leader || text==Players[i].noun_faction) faction=i;
    parsed_factions.push_back(faction); parsed_genders.push_back(GenderDefault);
    parsed_pluralities.push_back(PluralityDefault); event("parse:"+std::to_string(slot));
}
static void parse_num(int,int n) { last_num=n; event("num"); }
static void my_srand(int) { event("seed"); }
static unsigned int timeGetTime() { event("time"); return 123; }
static int bit_count(int n) { return __builtin_popcount(static_cast<unsigned int>(n)); }
static void say_year(char *s) { std::strcpy(s,"year"); event("year"); }
static void X_pop(const char *s,int(*)()) { event(std::string("xpop:")+s); }
static void diplo(int,int faction) { diplo_sender=faction; event("diplo"); }
static void report_intel(int) { event("intel"); }
static void prepare_contact_dialog_arguments(int faction) { event("prepare:"+std::to_string(faction)); }
struct GraphicWin {
    virtual void redraw() { event("redraw"); }
};
struct Multi : GraphicWin {
    void draw(int) { event("draw"); }
    void redraw() override { event("virtual_redraw"); }
} MultiWindow;
struct MessagePopup {
    void pop(const char *s,int,int,void*) { event(std::string("pop:")+s); }
} NetMsgPrimary;
struct Popup : PictureAPIs {
    int response=2;
    void start(char*,const char*,int,int,int,int) { event("start"); }
    Dialogs &dialogs() { return dialogs_; }
    int exec(int,int(*)()) { event("exec"); return response; }
};
struct Diplo : GraphicWin {
    int32_t preceding_storage_[4]{};
 private:
    /* ACTUAL DIPLO STATE */
 public:
    /* ACTUAL DIPLO SETTER */
    int state_for_test(int side) const { return side == 0 ? field_A24_ : field_A28_; }
    unsigned char *alias_packet_for_test() {
        unsigned char *begin = reinterpret_cast<unsigned char *>(this);
        const size_t offset = reinterpret_cast<unsigned char *>(&field_A24_) - begin;
        assert(offset>=0x10 && reinterpret_cast<unsigned char *>(&field_A28_) == begin+offset+4);
        return begin+offset-0x10;
    }
    int visible=0, maybe_sender=-1;
    int is_visible() { event("visible"); return visible; }
    void release_modal() { event("release"); }
    void add_offer(int,int,int) { event("offer"); }
    void kill_offer(int,int) { event("kill"); }
    void redraw() { event("diplo_redraw"); }
    void receive_clear_offer() { event("clear"); }
    void receive_channel_status(int) { event("status"); }
    void do_it() { event("do_it"); }
    void done_deal() { event("done_deal"); }
    void receive_maybe_deal(int n) { maybe_sender=n; event("maybe"); }
} DiploWindow;
struct Net {
    void *get(unsigned long *a,unsigned long *b) { *a=1001; *b=32; event("get"); return nullptr; }
};
struct NetDaemon : Net {
    PrimaryLockStateTail primary_state_{};
    LockStateEntry channels_[2]{};
    AlphaNetProcessSlot process_slots_[8]{};
    int player_count_=0;
    struct Locks { int field_E8_=0; } locks_;
    uint32_t &network_message_count() { return channels_[1].trailing_words_[7]; }
    void synch(int16_t,int,int,int,char*,int,int16_t) { event("synch"); }
    void process_message(char*,unsigned long,int) { event("process"); }
    char *get_player_name(unsigned long n) { name_arg=n; event("name"); return process_slots_[1].name_; }
    int who_2_pid(int n) { who_arg=n; event("who"); return n+100; }
} NetDaemonNet;
enum { NetTurnChecksumWords=21 };
static int NetPeerTurnChecksums[8][NetTurnChecksumWords];
static int turn_reads;
static int observe_turn_read(unsigned char *message) {
    ++turn_reads;
    return *reinterpret_cast<int *>(message+0x14);
}
/* ACTUAL HELPERS */
static void put(unsigned char *p,int at,int n) { std::memcpy(p+at,&n,4); }
static void reset() {
    callback={}; events.clear(); parsed_factions.clear(); parsed_genders.clear(); parsed_pluralities.clear();
    NetDaemonNet=NetDaemon{}; GameDataStateStorage=GameDataStateWindow{};
    GameDataStateStorage.fields_.turn_current_num_=9;
    GameDataStateStorage.fields_.lock_masks_.enable_mask_=0x0e0e;
    ConsoleGlobal.faction_id_=1; NetUpkeepPhase=3;
    NextTurnReady=0; TurnControlWord93A954=0; ConsoleControlTurnActive=0;
    NetDaemonWaitTicks=0; NetDaemonProgressInterval=0; NetDaemonProgressLastTick=0;
    CommlinkPending=0; CommlinkFaction=0; Language=0; DiplomacyActive=0; PopupWaitActive=0;
    DiploWindow=Diplo{};
    for(int i=0;i<8;++i) { Players[i]=Player{}; PlayersData[i]=PlayerData{}; Players[i].is_leader_female=i+10;
        Players[i].noun_gender=i+20; Players[i].is_noun_plural=i+30; portraits[i].id=i; FactionDiplomacyPortraits[i]=&portraits[i]; }
    TimeControl[0].event=4; TimeControl[1].event=7; TimeControl[2].event=11;
    std::memset(last_log,0,sizeof last_log); std::memset(last_send,0,sizeof last_send);
    std::memset(NetPeerTurnChecksums,0,sizeof NetPeerTurnChecksums);
}
static int position(const std::string &s) {
    for(size_t i=0;i<events.size();++i) if(events[i]==s) return static_cast<int>(i);
    return -1;
}
int main() {
    alignas(4) unsigned char packet[128]{};
    // The three host guards read the singleton but write the passed receiver.
    reset(); NetDaemon receiver; receiver.primary_state_.leading_words_[0]=1;
    put(packet,0x10,2); netdaemon_handle_server_upkeep_info(&receiver,packet);
    assert(receiver.channels_[0].leading_words_[1]==4 && NetDaemonNet.channels_[0].leading_words_[1]==0);
    reset(); receiver=NetDaemon{}; NetDaemonNet.primary_state_.leading_words_[0]=1;
    netdaemon_handle_server_upkeep_info(&receiver,packet); assert(receiver.channels_[0].leading_words_[1]==0 && position("draw")==-1);
    reset(); receiver=NetDaemon{}; receiver.primary_state_.leading_words_[0]=1;
    put(packet,0x10,2); put(packet,0x14,1); put(packet,0x18,9);
    netdaemon_handle_server_turn_info(&receiver,packet); assert(receiver.channels_[0].leading_words_[0]==4);
    reset(); receiver=NetDaemon{}; NetDaemonNet.primary_state_.leading_words_[0]=1;
    netdaemon_handle_server_turn_info(&receiver,packet); assert(receiver.channels_[0].leading_words_[0]==0 && position("draw")==-1);
    reset(); receiver=NetDaemon{}; GameDataStateStorage.fields_.net_turn_flags_=0x10;
    GameDataStateStorage.fields_.net_turn_faction_=1; NetDaemonNet.primary_state_.leading_words_[0]=1;
    netdaemon_handle_client_done_turn(&receiver,1); assert(last_send[0]==0x4309 && last_send[2]==2);
    reset(); receiver=NetDaemon{}; receiver.primary_state_.leading_words_[0]=1;
    GameDataStateStorage.fields_.net_turn_flags_=0x10;
    netdaemon_handle_client_done_turn(&receiver,1); assert(last_send[0]==0);

    // Logging mutates all three TURN INFO words before the branch reads them.
    reset(); receiver=NetDaemon{}; put(packet,0x10,1); put(packet,0x14,0); put(packet,0x18,0);
    callback=[&](const std::string&s) { if(s=="log:From Server: TURN INFO") { put(packet,0x10,2); put(packet,0x14,1); put(packet,0x18,9); } };
    netdaemon_handle_server_turn_info(&receiver,packet); assert(receiver.channels_[0].leading_words_[0]==4);
    // Slowpoke uses fresh grammar before name and pops before the server draw.
    reset(); receiver=NetDaemon{}; receiver.channels_[0].leading_words_[0]=6;
    put(packet,0x10,2); put(packet,0x14,1); put(packet,0x18,9);
    callback=[&](const std::string&s) { if(s=="parse:0") { GenderDefault=99; PluralityDefault=99; } };
    netdaemon_handle_server_turn_info(&receiver,packet);
    assert(parsed_factions[0]==3 && parsed_factions[1]==3 && parsed_genders[1]==13 && parsed_pluralities[1]==0);
    assert(position("pop:SLOWPOKE")>=0 && position("pop:SLOWPOKE")<position("draw"));
    reset(); receiver=NetDaemon{}; receiver.channels_[0].leading_words_[0]=6;
    callback=[&](const std::string&s) { if(s=="parse:0") { GenderDefault=99; PluralityDefault=99; } };
    netdaemon_handle_client_done_turn(&receiver,2);
    assert(parsed_genders[1]==13 && parsed_pluralities[1]==0 && position("draw")<position("pop:SLOWPOKE"));

    // NEXT UPKEEP resets the ACK notification word, preserving base-ready.
    reset(); receiver=NetDaemon{}; receiver.primary_state_.trailing_words_[9]=0x1234;
    put(packet,0x10,0); put(packet,0x14,0);
    callback=[&](const std::string&s) { if(s=="log:From Server: NEXT UPKEEP") { put(packet,0x10,3); put(packet,0x14,9); } };
    netdaemon_handle_server_lock_status(&receiver,0x4303,packet,2);
    assert(receiver.primary_state_.trailing_words_[8]==255 && receiver.primary_state_.trailing_words_[9]==0x1234);
    assert(position("log:(Upkeep instruction for wrong phase!)")==-1 && position("log:(Upkeep instruction for wrong turn!)")==-1);
    // Preserve wrong phase/turn paths: log, seed and masks still execute.
    reset(); receiver=NetDaemon{}; put(packet,0x10,4); put(packet,0x14,8);
    netdaemon_handle_server_lock_status(&receiver,0x4303,packet,2);
    assert(position("log:(Upkeep instruction for wrong phase!)")>=0 && position("seed")>=0);
    reset(); receiver=NetDaemon{}; put(packet,0x10,3);
    netdaemon_handle_server_lock_status(&receiver,0x4303,packet,2);
    assert(position("log:(Upkeep instruction for wrong turn!)")>=0 && receiver.primary_state_.trailing_words_[8]==255);
    // EXEC DONE logs old remaining as argument one, then tests live remaining.
    reset(); receiver=NetDaemon{}; receiver.channels_[1].trailing_words_[2]=7; receiver.channels_[1].leading_words_[3]=55;
    callback=[&](const std::string&s) { if(s=="log:From Server: EXEC DONE") { assert(last_log[0]==7 && last_log[1]==0 && last_log[2]==0); receiver.channels_[1].trailing_words_[2]=0; } };
    netdaemon_handle_server_lock_status(&receiver,0x1206,packet,2); assert(last_send[0]==0 && receiver.channels_[1].leading_words_[3]==0);
    reset(); receiver=NetDaemon{}; receiver.channels_[1].leading_words_[3]=55;
    callback=[&](const std::string&s) { if(s=="log:From Server: EXEC DONE") receiver.channels_[1].trailing_words_[2]=7;
        if(s=="send:8724") assert(receiver.channels_[1].trailing_words_[2]==0); };
    netdaemon_handle_server_lock_status(&receiver,0x1206,packet,2);
    assert(last_send[0]==0x2214 && receiver.channels_[1].leading_words_[3]==55);
    reset(); receiver=NetDaemon{};
    netdaemon_handle_server_lock_status(&receiver,0x1207,packet,2);
    assert(receiver.channels_[0].leading_words_[2]==1 && position("log:From Server: ADD LOCK OK")>=0);
    netdaemon_handle_server_lock_status(&receiver,0x1208,packet,2);
    assert(receiver.channels_[0].leading_words_[2]==2 && position("log:From Server: ADD LOCK FAIL")>=0);

    // DONE UPKEEP's final phase is read after send/draw, not entry-captured.
    reset(); receiver=NetDaemon{}; receiver.player_count_=1; receiver.process_slots_[1].identity_=2;
    put(packet,0x10,3); put(packet,0x14,9);
    callback=[&](const std::string&s) { if(s=="draw") put(packet,0x10,6); };
    netdaemon_handle_client_done_upkeep(&receiver,packet,2); assert(last_log[0]==2 && last_log[1]==6 && last_log[2]==4);
    // The old-pending early return remains distinct from completion polling.
    reset(); receiver=NetDaemon{}; put(packet,4,2); receiver.channels_[1].trailing_words_[0]=55;
    callback=[&](const std::string&s) { if(s=="synch") { put(packet,4,1); receiver.channels_[1].trailing_words_[1]=0; } };
    netdaemon_process_sync_request(&receiver,packet); assert(receiver.channels_[1].trailing_words_[0]==0);
    reset(); receiver=NetDaemon{}; put(packet,4,2); receiver.channels_[1].trailing_words_[0]=55; receiver.channels_[1].trailing_words_[1]=1;
    callback=[&](const std::string&s) { if(s=="synch") { put(packet,4,1); receiver.channels_[1].trailing_words_[1]=0; } };
    netdaemon_process_sync_request(&receiver,packet); assert(receiver.channels_[1].trailing_words_[0]==55);

    // Table F carries resolved sender for logs and transport sender for names.
    reset(); receiver=NetDaemon{}; netdaemon_handle_table_f(&receiver,0x4304,packet,1001,2);
    assert(last_log[0]==2 && receiver.primary_state_.trailing_words_[9]==1);
    put(packet,0x10,4); NetDaemonNet.network_message_count()=5;
    callback=[&](const std::string&s) { if(s=="log:From Server: MSG_EVENTS") { assert(last_log[0]==2 && last_log[1]==4); put(packet,0x10,7); } };
    netdaemon_handle_table_f(&receiver,0x4305,packet,1001,2); assert(NetDaemonNet.network_message_count()==7);
    callback={}; put(packet,0x10,-1); netdaemon_handle_table_f(&receiver,0x4305,packet,1001,2); assert(NetDaemonNet.network_message_count()==7);
    reset(); receiver=NetDaemon{}; put(packet,4,2);
    for(int i=0;i<21;++i) put(packet,0x10+i*4,100+i);
    netdaemon_handle_table_f(&receiver,0x4307,packet,1001,2);
    for(int i=0;i<21;++i) assert(NetPeerTurnChecksums[2][i]==100+i);
    // A byte view within the whole existing array allocation. First write
    // changes faction; later stores use the new row. Preserve the overlap
    // where the next-to-last payload has already been written by this loop.
    reset(); receiver=NetDaemon{};
    NetPeerTurnChecksums[2][0]=2; NetPeerTurnChecksums[2][3]=3; NetPeerTurnChecksums[2][4]=17;
    unsigned char *checksum_packet = reinterpret_cast<unsigned char *>(&NetPeerTurnChecksums) + (2*21-1)*sizeof(int);
    netdaemon_handle_table_f(&receiver,0x4307,checksum_packet,1001,2);
    assert(NetPeerTurnChecksums[2][0]==3 && NetPeerTurnChecksums[3][1]==17 && NetPeerTurnChecksums[2][1]==0);
    assert(NetPeerTurnChecksums[3][19]==17);
    for(int n : {0,-2}) { reset(); receiver=NetDaemon{}; put(packet,4,2); PlayersData[2].time_bonus_count=n;
        netdaemon_handle_table_f(&receiver,0x4306,packet,1001,2); assert(NetDaemonWaitTicks==0 && events.size()==1 && PlayersData[2].time_bonus_count==n); }
    reset(); receiver=NetDaemon{}; put(packet,4,1); PlayersData[2].time_bonus_count=1;
    callback=[&](const std::string&s) { if(s=="log:From Server: MSG_TIME_BONUS") put(packet,4,2);
        if(s=="redraw") GameDataStateStorage.fields_.time_controls_selection_=1;
        if(s=="parse:0") GameDataStateStorage.fields_.time_controls_selection_=2; };
    netdaemon_handle_table_f(&receiver,0x4306,packet,1001,2);
    assert(PlayersData[2].time_bonus_count==0 && NetDaemonWaitTicks==4000 && GameDataStateStorage.fields_.turn_control_duration_milliseconds_==4000);
    assert(name_arg==1001 && last_num==11 && position("redraw")>=0 && position("draw")==-1 && position("pop:HEBONUSED")>=0);
    reset(); receiver=NetDaemon{}; put(packet,4,2); PlayersData[2].time_bonus_count=1; ConsoleControlTurnActive=1;
    netdaemon_handle_table_f(&receiver,0x4306,packet,1001,2); assert(PlayersData[2].time_bonus_count==1 && NetDaemonWaitTicks==0);

    // NEXT PLAYER TURN fixes the local/remote branch before parse callbacks.
    for(bool local : {false,true}) {
        reset(); receiver=NetDaemon{}; put(packet,0x10,7); TurnControlWord93A954=42;
        callback=[&](const std::string&s) {
            if(s=="log:From Server: NEXT PLAYER TURN") put(packet,0x10,local?1:2);
            if(s=="parse:0") { assert(TurnControlWord93A954==(local?0:42)); GameDataStateStorage.fields_.net_turn_faction_=3; ConsoleGlobal.faction_id_=local?7:2; }
            if(s=="parse:1") GameDataStateStorage.fields_.net_turn_faction_=4;
            if(s=="parse:2") GameDataStateStorage.fields_.net_turn_faction_=5;
        };
        netdaemon_handle_table_f(&receiver,0x4309,packet,1001,2);
        assert(parsed_factions[0]==(local?1:2) && parsed_factions[1]==3 && parsed_factions[2]==4 && who_arg==5 && name_arg==105);
        assert(position(local?"xpop:SIMULYOU":"pop:SIMULWHOSE")>=0);
    }

    // Real public picture APIs: empty list retains caller portrait; a populated
    // list clears the caller picture while both paths still insert the sprite.
    for(int count : {0,2}) { reset(); Popup popup{}; popup.dialogs_.list_.entry_count_=count; popup.caller_sprite_=&portraits[7];
        callback=[&](const std::string&s) { if(s=="sprite:2") assert(popup.caller_sprite_==(count?nullptr:&portraits[2])); };
        assert(netdaemon_run_contact_prompt(popup,2)==2);
        assert(popup.caller_sprite_==(count?nullptr:&portraits[2])); assert(position("start")<position("sprite:2") && position("sprite:2")<position("exec")); }
    // Receiver's busy word, post-log sender capture, and fixed sender in prompt.
    reset(); receiver=NetDaemon{}; Popup popup{}; int mask=0; put(packet,4,7); receiver.channels_[1].leading_words_[2]=1;
    callback=[&](const std::string&s) { if(s=="log:Received OPEN CHAN") put(packet,4,2); };
    netdaemon_handle_direct_diplomacy_complete(receiver,0x1501,packet,popup,mask); assert(last_send[0]==0x1504 && last_send[1]==2);
    reset(); receiver=NetDaemon{}; NetDaemonNet.channels_[1].leading_words_[2]=1; put(packet,4,2); popup=Popup{}; mask=0;
    netdaemon_handle_direct_diplomacy_complete(receiver,0x1501,packet,popup,mask); assert(position("start")>=0 && last_send[0]==0x1503 && last_send[1]==2 && last_send[2]==1);
    reset(); receiver=NetDaemon{}; put(packet,4,2); popup=Popup{}; mask=1<<2;
    netdaemon_handle_direct_diplomacy_complete(receiver,0x1501,packet,popup,mask); assert(last_send[0]==0x1503 && position("start")==-1);
    reset(); receiver=NetDaemon{}; put(packet,4,2); popup=Popup{}; mask=0; CommlinkPending=1; CommlinkFaction=2; DiploWindow.visible=1;
    callback=[&](const std::string&s) { if(s=="visible") CommlinkPending=0; };
    netdaemon_handle_direct_diplomacy_complete(receiver,0x1501,packet,popup,mask); assert(position("start")>=0);

    // OPEN ACK reloads sender after log, visibility and message callbacks.
    reset(); receiver=NetDaemon{}; put(packet,4,7); popup=Popup{}; mask=0; CommlinkPending=1; CommlinkFaction=2;
    callback=[&](const std::string&s) { if(s=="log:Received OPEN ACKNOWLEDGE") put(packet,4,2);
        if(s=="visible") put(packet,4,3); if(s=="send:9281") put(packet,4,4); };
    netdaemon_handle_direct_diplomacy_complete(receiver,0x1502,packet,popup,mask);
    assert(last_send[0]==0x2441 && last_send[3]==3 && diplo_sender==4);
    // Refuse/busy/close/vendetta parse each faction afresh, with unchecked rows
    // kept valid in the fixture. Response payload is reread after parsing too.
    for(int opcode : {0x1503,0x1504,0x1505,0x150f}) {
        reset(); receiver=NetDaemon{}; put(packet,4,7); put(packet,0x10,2); popup=Popup{}; mask=0; CommlinkPending=1;
        callback=[&](const std::string&s) { if(s.find("log:Received")==0) put(packet,4,2);
            if(s=="parse:0") put(packet,4,3);
            if(s=="parse:1") { put(packet,4,4); put(packet,0x10,1); } };
        if(opcode==0x1505 || opcode==0x150f) netdaemon_handle_diplo_cd_message(opcode,packet);
        else netdaemon_handle_direct_diplomacy_complete(receiver,opcode,packet,popup,mask);
        assert(parsed_factions[0]==2 && parsed_factions[1]==3 && parsed_genders[1]==13 && parsed_pluralities[1]==0);
        if(opcode==0x150f) assert(parsed_factions[2]==4 && parsed_genders[2]==24 && parsed_pluralities[2]==34);
        if(opcode==0x1503) assert(position("pop:DIPLOREFUSE1")>=0);
        if(opcode==0x1504) assert(position("pop:DIPLOBUSY1")>=0);
    }
    reset(); receiver=NetDaemon{}; put(packet,4,7); popup=Popup{}; mask=0;
    callback=[&](const std::string&s) { if(s=="log:Received MAYBE DEAL") put(packet,4,2); };
    netdaemon_handle_direct_diplomacy_complete(receiver,0x150a,packet,popup,mask); assert(DiploWindow.maybe_sender==2);
    // NO DEAL's packet is a valid byte view of the existing owner allocation.
    // Actual private-owner setters interleave stores with reads; snapshotting
    // both payloads would produce7,2 instead of7,7.
    reset(); DiploWindow.set_offer_state(0,2); DiploWindow.set_offer_state(1,7);
    unsigned char *owner_packet=DiploWindow.alias_packet_for_test();
    callback=[&](const std::string&s) { if(s=="redraw") assert(DiploWindow.state_for_test(0)==7 && DiploWindow.state_for_test(1)==7); };
    netdaemon_handle_diplo_cd_message(0x150c,owner_packet);
    assert(DiploWindow.state_for_test(0)==7 && DiploWindow.state_for_test(1)==7);
    assert(position("redraw")>=0 && position("diplo_redraw")==-1);
    /* CONDITIONAL READ CONTROLS */
    return 0;
}
'''


def compile_and_run(tmp_path, compiler, declarations, functions, state_fields, setter, controls=""):
    cpp = tmp_path / "early.cpp"
    exe = tmp_path / "early"
    cpp.write_text(PREAMBLE.replace("/* ACTUAL DECLARATIONS */", declarations)
                   .replace("/* ACTUAL HELPERS */", functions)
                   .replace("/* ACTUAL DIPLO STATE */", state_fields)
                   .replace("/* ACTUAL DIPLO SETTER */", setter)
                   .replace("/* CONDITIONAL READ CONTROLS */", controls))
    build = subprocess.run([compiler, "-std=c++11", "-fsanitize=undefined", "-fno-sanitize-recover=all",
                            str(cpp), "-o", str(exe)], capture_output=True, text=True, timeout=60)
    assert build.returncode == 0, build.stdout + build.stderr
    return subprocess.run([str(exe)], capture_output=True, text=True, timeout=20)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_current_owner_callbacks(tmp_path, compiler):
    compiler = shutil.which(compiler)
    if not compiler:
        pytest.skip("host compiler unavailable")
    declarations, functions, state_fields, setter = source_material()
    result = compile_and_run(tmp_path, compiler, declarations, functions, state_fields, setter)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("damage", ["upkeep_word", "bonus_exit", "global_host", "portrait", "completion_reload", "exec_reload", "server_ui_order", "direct_sender", "table_sender", "grammar", "local_clear", "phase_reload", "bonus_redraw", "bonus_virtual_redraw", "bonus_selection", "assigned_faction", "pending_recheck", "add_reply", "checksum_row_cache", "no_deal_preload", "no_deal_virtual_redraw"])
def test_behavior_damage_controls(tmp_path, damage):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host compiler unavailable")
    declarations, functions, state_fields, setter = source_material()
    mutations = {
        "upkeep_word": ("daemon->primary_state_.trailing_words_[8] = 0xFF;", "daemon->primary_state_.trailing_words_[9] = 0xFF;"),
        "bonus_exit": ("if (PlayersData[faction_id].time_bonus_count <= 0) return true;", "if (false) return true;"),
        "global_host": ("if (NetDaemonNet.primary_state_.leading_words_[0] != 0) return;", "if (daemon->primary_state_.leading_words_[0] != 0) return;"),
        "portrait": ("popup.set_caller_picture(popup.picture_enabled() ? 0 : portrait);", "popup.set_caller_picture(portrait);"),
        "completion_reload": ("if (*reinterpret_cast<uint32_t *>(message + 4) ==\n        static_cast<uint32_t>(ConsoleGlobal.faction_id_))", "if (sender == static_cast<uint32_t>(ConsoleGlobal.faction_id_))"),
        "exec_reload": ("if (daemon->channels_[1].trailing_words_[2] != 0)", "if (remaining != 0)"),
        "server_ui_order": ('NetMsgPrimary.pop("SLOWPOKE", 5000, 0, 0);\n    MultiWindow.draw(0);', 'MultiWindow.draw(0);\n    NetMsgPrimary.pop("SLOWPOKE", 5000, 0, 0);'),
        "direct_sender": ('log_say("Received OPEN CHAN",\n                *reinterpret_cast<int *>(message + 4), 0, 0);\n        const int sender = *reinterpret_cast<int *>(message + 4);', 'const int sender = *reinterpret_cast<int *>(message + 4);\n        log_say("Received OPEN CHAN", sender, 0, 0);'),
        "table_sender": ('log_say("From Server: UPKEEP_BASE", sender_identity, 0, 0);', 'log_say("From Server: UPKEEP_BASE", 0, 0, 0);'),
        "grammar": ('GenderDefault = faction_data.is_leader_female;\n    PluralityDefault = 0;\n    parse_says(1,', 'parse_says(1,'),
        "local_clear": ('if (local_turn) TurnControlWord93A954 = 0;', 'if (false) TurnControlWord93A954 = 0;'),
        "phase_reload": ('log_say("From Client: DONE UPKEEP", sender,\n            *reinterpret_cast<int *>(message + 0x10),', 'log_say("From Client: DONE UPKEEP", sender, phase,'),
        "bonus_redraw": ('MultiWindow.GraphicWin::redraw();', 'MultiWindow.draw(0);'),
        "bonus_virtual_redraw": ('MultiWindow.GraphicWin::redraw();', 'MultiWindow.redraw();'),
        "bonus_selection": ('parse_num(0, TimeControl[\n            GameDataStateStorage.fields_.time_controls_selection_].event);', 'parse_num(0, TimeControl[selection].event);'),
        "assigned_faction": ('Player &name_player = Players[\n            GameDataStateStorage.fields_.net_turn_faction_];', 'Player &name_player = Players[faction_id];'),
        "pending_recheck": ('if (CommlinkPending != 0) {\n                message_data(0x1504, sender, 0, 0, 0, 0);', 'if (true) {\n                message_data(0x1504, sender, 0, 0, 0, 0);'),
        "add_reply": ('case 0x1207:\n        log_say("From Server: ADD LOCK OK", 0, 0, 0);\n        daemon->channels_[0].leading_words_[2] = 1;', 'case 0x1207:\n        log_say("From Server: ADD LOCK OK", 0, 0, 0);\n        daemon->channels_[0].leading_words_[2] = 2;'),
        "checksum_row_cache": ('for (int index = 0; index < NetTurnChecksumWords; ++index) {\n            const int faction = *reinterpret_cast<int *>(message + 4);', 'const int faction = *reinterpret_cast<int *>(message + 4);\n        for (int index = 0; index < NetTurnChecksumWords; ++index) {'),
        "no_deal_preload": ('DiploWindow.set_offer_state(0, *reinterpret_cast<int *>(message + 0x14));\n        DiploWindow.set_offer_state(1, *reinterpret_cast<int *>(message + 0x10));', '{ const int first = *reinterpret_cast<int *>(message + 0x14);\n        const int second = *reinterpret_cast<int *>(message + 0x10);\n        DiploWindow.set_offer_state(0, first);\n        DiploWindow.set_offer_state(1, second); }'),
        "no_deal_virtual_redraw": ('DiploWindow.GraphicWin::redraw();', 'DiploWindow.redraw();'),
    }
    before, after = mutations[damage]
    assert before in functions, damage
    functions = functions.replace(before, after, 1)
    result = compile_and_run(tmp_path, compiler, declarations, functions, state_fields, setter)
    assert result.returncode != 0, "control did not detect " + damage


@pytest.mark.parametrize("eager_damage", [False, True])
def test_done_upkeep_conditional_cpp_evaluation(tmp_path, eager_damage):
    """Allocated instrumentation proves C++ evaluation only, not whether a
    previous native compiler emitted a load or native code would fault. The
    separate pinned-image assertions establish the original branch.
    """
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host compiler unavailable")
    declarations, functions, state_fields, setter = source_material()
    body = _definition(SOURCE.read_text(), r"static\s+void\s+netdaemon_handle_client_done_upkeep", "done_upkeep")
    instrumented = body
    read = "        const int turn = *reinterpret_cast<int *>(message + 0x14);"
    assert instrumented.count(read)==1
    if eager_damage:
        instrumented = instrumented.replace(read, "", 1).replace("    if (phase != NetUpkeepPhase)", read.strip()+"\n    if (phase != NetUpkeepPhase)", 1)
    instrumented = instrumented.replace("*reinterpret_cast<int *>(message + 0x14)", "observe_turn_read(message)")
    functions = functions.replace(body, instrumented, 1)
    controls = r'''
    reset(); receiver=NetDaemon{}; put(packet,0x10,4); put(packet,0x14,9); turn_reads=0;
    netdaemon_handle_client_done_upkeep(&receiver,packet,2); assert(turn_reads==0);
    reset(); receiver=NetDaemon{}; put(packet,0x10,3); put(packet,0x14,9); turn_reads=0;
    netdaemon_handle_client_done_upkeep(&receiver,packet,2); assert(turn_reads==1);
    reset(); receiver=NetDaemon{}; put(packet,0x14,8); turn_reads=0;
    netdaemon_handle_client_done_upkeep(&receiver,packet,2); assert(turn_reads==1);
    assert(position("log:(Upkeep ack for wrong turn!)")>=0);
'''
    result = compile_and_run(tmp_path, compiler, declarations, functions, state_fields, setter, controls)
    assert (result.returncode == 0) == (not eager_damage), result.stdout + result.stderr


def test_pinned_branch_storage_and_callback_evidence():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    image_path = Path(os.environ.get("OPENSMACX_IMAGE", "/home/fran6co/code/OpenSMACX/.opensmacx/game/terranx_original.exe"))
    image = PeImage(image_path.read_bytes(), PIN)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    # Opcode selectors independently establish each table entry's branch,
    # including Table F's forwarding hole, rather than inferring opcode IDs
    # from the adjacent bodies or their labels.
    for address, targets in (
        (0x5388E8, (0x534E23, 0x534E50, 0x534E7E, 0x534E9D,
                    0x534EFA, 0x534F1C, 0x534F71, 0x534F4F)),
        (0x538908, (0x535818, 0x5358DA, 0x535916, 0x53594C, 0x53597A)),
        (0x53891C, (0x535A73, 0x535A3D, 0x535A97, 0x535ABB, 0x535AD5)),
        (0x538944, (0x53662D, 0x53664E, 0x53667D, 0x53675D, 0x537F7A, 0x536414)),
    ):
        assert struct.unpack("<" + "I" * len(targets),
                             image.read(Span(address, address + 4 * len(targets)))) == targets
    selectors = {}
    for start,end in ((0x534DFD,0x534E23),(0x535803,0x535818),
                      (0x535A17,0x535A3D),(0x5363EE,0x536414)):
        selectors.update({i.address:(i.mnemonic,i.op_str) for i in decoder.disasm(image.read(Span(start,end)),start)})
    for address,instruction in {
        0x534E0E:("add","eax, 0xffffedff"), 0x534E13:("cmp","eax, 7"),
        0x534E1C:("jmp","dword ptr [eax*4 + 0x5388e8]"),
        0x535803:("add","eax, 0xffffeafb"), 0x535811:("jmp","dword ptr [eax*4 + 0x538908]"),
        0x535A28:("add","eax, 0xffffeaf5"), 0x535A36:("jmp","dword ptr [eax*4 + 0x53891c]"),
        0x5363FF:("add","eax, 0xffffbcfc"), 0x536404:("cmp","eax, 5"),
        0x53640D:("jmp","dword ptr [eax*4 + 0x538944]"),
    }.items():
        assert selectors[address] == instruction, hex(address)
    windows = ((0x534F1C,0x534F93), (0x535048,0x535097), (0x53511E,0x53533C),
               (0x53533C,0x535600), (0x535612,0x535803), (0x535818,0x5358DA),
               (0x5359B3,0x535B9F), (0x536037,0x53611D), (0x53624A,0x5362E4),
               (0x536369,0x5363EE), (0x536414,0x536781), (0x536D78,0x536DA5),
               (0x537FD0,0x538266))
    instructions = {}
    for start,end in windows:
        decoded = list(decoder.disasm(image.read(Span(start,end)),start))
        assert decoded and decoded[-1].address + decoded[-1].size == end
        instructions.update({i.address:(i.mnemonic,i.op_str) for i in decoded})
    expected = {
        0x534F1C:("mov","eax, dword ptr [esi + 0x1bc0]"),
        0x534F24:("push","edi"), 0x534F25:("push","edi"), 0x534F26:("push","eax"),
        0x534F2C:("call","0x6262f0"), 0x534F31:("mov","eax, dword ptr [esi + 0x1bc0]"),
        0x534F3C:("je","0x536d9a"), 0x536D78:("call","0x6262f0"),
        0x534F62:("mov","dword ptr [esi + 0x1b70], 2"),
        0x534F84:("mov","dword ptr [esi + 0x1b70], 1"),
        0x536D87:("mov","dword ptr [esi + 0x1bc0], edi"), 0x536D8D:("call","0x592ee0"),
        0x536D9A:("mov","dword ptr [esi + 0x1bb0], edi"),
        0x53505E:("mov","eax, dword ptr [0x93e8c0]"),
        0x53506E:("mov","ecx, dword ptr [ebx + 0x10]"),
        0x535082:("mov","dword ptr [esi + 0x1b6c], ecx"),
        0x535144:("mov","eax, dword ptr [0x93e8c0]"),
        0x535151:("mov","edx, dword ptr [ebx + 0x18]"),
        0x535161:("mov","eax, dword ptr [ebx + 0x14]"),
        0x53516C:("mov","ecx, dword ptr [ebx + 0x10]"),
        0x535247:("mov","eax, dword ptr [edi + 0x946a50]"),
        0x53525A:("mov","dword ptr [0x9bbfec], eax"),
        0x53525F:("mov","dword ptr [0x9bbff0], 0"),
        0x5352FB:("call","0x47a890"), 0x535307:("call","0x479330"),
        0x538087:("mov","eax, dword ptr [0x93e8c0]"),
        0x5381AE:("mov","eax, dword ptr [edi + 0x946a50]"),
        0x5381B6:("mov","dword ptr [0x9bbfec], eax"),
        0x5381C6:("mov","dword ptr [0x9bbff0], 0"),
        0x536105:("mov","ecx, dword ptr [ebx + 0x10]"),
        0x536037:("mov","eax, dword ptr [ebx + 0x10]"),
        0x536042:("je","0x53604f"), 0x53604D:("jmp","0x536065"),
        0x53604F:("mov","eax, dword ptr [ebx + 0x14]"),
        0x536110:("call","0x6262f0"),
        0x53627D:("call","0x532e00"), 0x536284:("jne","0x538266"),
        0x5362B5:("call","0x534400"), 0x5362C4:("mov","eax, dword ptr [ebx + 4]"),
        0x5362D5:("mov","dword ptr [esi + 0x1bb8], 0"),
        0x536379:("call","0x6262f0"), 0x53637E:("mov","eax, dword ptr [ebx + 0x10]"),
        0x536398:("mov","eax, dword ptr [ebx + 0x14]"),
        0x5363CB:("mov","dword ptr [esi + 0x1b6c], eax"),
        0x5363D2:("mov","dword ptr [esi + 0x1b60], eax"),
        0x53662D:("push","0"), 0x53662F:("push","0"), 0x536631:("push","edi"),
        0x53663F:("mov","dword ptr [esi + 0x1b64], 1"),
        0x536651:("push","0"), 0x536653:("push","eax"), 0x536654:("push","edi"),
        0x53665A:("call","0x6262f0"), 0x53665F:("mov","eax, dword ptr [ebx + 0x10]"),
        0x53666D:("jg","0x538266"),
        0x536696:("mov","edi, dword ptr [ebx + 4]"),
        0x5366B4:("mov","ecx, dword ptr [eax*4 + 0x96ea34]"),
        0x5366BD:("jle","0x538266"), 0x5366C3:("dec","ecx"),
        0x536706:("call","0x5d5a70"), 0x53671D:("call","0x625ec0"),
        0x536722:("mov","eax, dword ptr [0x9a64e0]"), 0x536733:("call","0x625e30"),
        0x536762:("mov","edx, dword ptr [ebx + 4]"),
        0x536765:("mov","edi, dword ptr [ecx]"),
        0x53676D:("add","ecx, 4"),
        0x536773:("mov","dword ptr [edx*4 + 0x93a510], edi"),
        0x53677A:("jl","0x536762"),
        0x536421:("call","0x6262f0"), 0x536426:("mov","ecx, dword ptr [ebx + 0x10]"),
        0x536431:("cmp","ecx, eax"), 0x536433:("mov","dword ptr [0x9a6820], ecx"),
        0x536439:("jne","0x53654d"), 0x536446:("mov","dword ptr [0x93a954], edi"),
        0x536476:("mov","ecx, dword ptr [0x9a6820]"),
        0x5364AE:("mov","ecx, dword ptr [0x9a6820]"),
        0x5364EC:("mov","edx, dword ptr [0x9a6820]"),
        0x53657E:("mov","ecx, dword ptr [0x9a6820]"),
        0x5365B6:("mov","ecx, dword ptr [0x9a6820]"),
        0x5365F4:("mov","edx, dword ptr [0x9a6820]"),
        0x535349:("call","0x6262f0"), 0x53534E:("mov","edi, dword ptr [ebx + 4]"),
        0x535388:("mov","eax, dword ptr [esi + 0x1bac]"),
        0x5353AD:("mov","eax, dword ptr [0x7492cc]"),
        0x5353B6:("cmp","edi, dword ptr [0x73b790]"),
        0x5353C3:("call","0x5f7e90"), 0x5353CA:("jne","0x5353fa"),
        0x5353FA:("mov","eax, dword ptr [0x7492cc]"),
        0x5353FF:("test","eax, eax"), 0x535401:("jne","0x535392"),
        0x535507:("mov","ecx, dword ptr [ebp - 0x329c]"),
        0x535512:("mov","eax, dword ptr [eax*4 + 0x6846d8]"),
        0x535524:("mov","dword ptr [ebp - 0x33cc], ecx"),
        0x535530:("call","0x6108e0"),
        0x5356E4:("mov","eax, dword ptr [ebx + 4]"),
        0x53571E:("mov","edx, dword ptr [ebx + 4]"),
        0x535737:("mov","ecx, dword ptr [ebx + 4]"),
        0x53562A:("mov","ecx, dword ptr [ebx + 4]"), 0x53565E:("mov","ecx, dword ptr [ebx + 4]"),
        0x535767:("mov","ecx, dword ptr [ebx + 4]"), 0x53579B:("mov","ecx, dword ptr [ebx + 4]"),
        0x53585B:("mov","ecx, dword ptr [ebx + 4]"), 0x535891:("mov","ecx, dword ptr [ebx + 4]"),
        0x535AE7:("mov","ecx, dword ptr [ebx + 4]"),
        0x535B1B:("mov","ecx, dword ptr [ebx + 4]"), 0x535B50:("mov","ecx, dword ptr [ebx + 4]"),
        0x5359DD:("mov","ecx, dword ptr [ebx + 4]"),
        0x5359FB:("mov","edx, dword ptr [ebx + 4]"),
        0x535A4F:("mov","ecx, dword ptr [ebx + 0x14]"),
        0x535A55:("mov","dword ptr [0x73b6fc], ecx"),
        0x535A5B:("mov","edx, dword ptr [ebx + 0x10]"),
        0x535A5E:("mov","ecx, 0x73acd8"),
        0x535A63:("mov","dword ptr [0x73b700], edx"),
        0x535A69:("call","0x5d5a70"),
    }
    for address, instruction in expected.items():
        assert instructions[address] == instruction, hex(address)
    # Validate conditional caller-picture mask and the known local offsets;
    # this is object evidence, not an adjacency or full-frame allocation claim.
    assert 0x5510 - 0x33cc == 0x2144 and 0x5510 - 0x329c == 0x2274
    assert [instructions[a] for a in (0x535510,0x53551B,0x53551F,0x535521)] == [
        ("neg","ecx"), ("sbb","ecx, ecx"), ("not","ecx"), ("and","ecx, eax")]
    assert 0x93CD90 + 0x1B30 == 0x93E8C0


def test_dispatcher_preserves_identity_and_owner_calls():
    body = _definition(SOURCE.read_text(), r"void\s+NetDaemon::process_message", "process_message")
    assert "*this, type, msg, popup, MultiWindowContactStatusMask" in body
    assert "netdaemon_handle_table_f(this, type, msg, sender_id,\n                                    entry_sender_identity)" in body
    assert body.count("pid_2_who(") == 1
