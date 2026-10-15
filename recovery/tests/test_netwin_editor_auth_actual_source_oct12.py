"""Complete coupled editors with real passive declarations and public menu APIs.

Dependency shims mutate valid allocated owners and trace C++ lifetimes/order.
The pin separately establishes native branches and subobjects. Neither host
ABI nor synthetic callbacks establish gameplay reachability/native emission.
Reserved packet bytes are not initialized, inspected or given expected values.
"""
from pathlib import Path
import hashlib
import os
import shutil
import subprocess

import pytest

from test_netwin_owner_behavior import extract_function
from test_netdaemon_early_routes_oct12 import declaration, PIN
from test_net_hotseat_owner_actual_source_oct6 import installed_packet_declarations

ROOT=Path(__file__).resolve().parents[2]


def material():
    source=(ROOT/"src/netwin.cpp").read_text()
    records=installed_packet_declarations()
    records+="\n"+declaration((ROOT/"src/faction.h").read_text(),"Player")
    menu=extract_function((ROOT/"src/popmenu.h").read_text(),"class PopMenu : public Popup, public PullDown")+";"
    accessor=extract_function((ROOT/"src/basepop.h").read_text(),"Dialogs &dialogs()")
    menu_source=(ROOT/"src/popmenu.cpp").read_text()
    owners="\n".join(extract_function(menu_source,n) for n in (
        "PopMenu::~PopMenu()","int PopMenu::init()","int PopMenu::exec(int"))
    helpers="\n".join(extract_function(source,n) for n in (
        "static int __cdecl pick_daemon_callback()","static int pick_answer(NetWin"))
    general=(ROOT/"src/general.cpp").read_text()
    checksum="\n".join(extract_function(general,n) for n in (
        "uint8_t __cdecl checksum(","uint32_t __cdecl checksum_password("))
    compat=extract_function((ROOT/"src/vc6_compat.h").read_text(),"inline int strcpy_s(char *destination, size_t,")
    bodies={n:extract_function(source,s) for n,s in (
        ("player","void NetWin::pick_player()"),
        ("faction","void NetWin::pick_faction(int"),
        ("pass","int NetWin::pick_pass(int"))}
    return records,menu,accessor,owners,helpers,compat+"\n"+checksum,bodies


HARNESS=r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <cctype>
#include <deque>
#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#define __cdecl
typedef int BOOL;
typedef char *LPSTR;
typedef const char *LPCSTR;
static std::vector<std::string> events;
static std::function<void(const std::string&)> callback;
static void event(const std::string&e) { events.push_back(e); if(callback)callback(e); }
/* ACTUAL RECORDS */
static NetWinLobbyRow NetWinLobbyRows[8],sent_row;
static NetWinSessionBlock NetWinSessionData;
static NetWinFactionRecord table_a[8],table_b[8];
static Player Players[8];
static struct Text { char str[256]; } ParseStrBuffer[10];
static char PopupStartCaption[]="caption", ScriptPopupFile[]="script";
static char NetWinPasswordPromptInitialText,NetWinPasswordVerifyInitialText;
static int local_slot=2,answer=0,poll_state=0,editor_result=0;
static int popup_ctors,popup_dtors,pull_ctors,pull_dtors,sends,copies,identity_calls,ask_index;
static int sent_slot,sent_target;
static std::vector<std::string> ask_texts;
static std::vector<int> ask_results;
static std::string edited_gender="Female";
static bool fresh_default_labels=false;
static bool long_gender_label=false;
static int default_lookups=0;
static bool observe_owner=false;
static int owner_accesses=0;
static uint32_t &owner_access(NetWinLobbyRow &row) {
    if(observe_owner)++owner_accesses;
    return row.owner_;
}
static void *memcpy_call(void *dst,const void *src,unsigned size) {
    assert(size==0x17c); ++copies; std::memcpy(dst,src,size); event("copy"); return dst;
}
struct Payload { char *text; };
struct Node {
    int id; std::string text; Payload payload;
    Payload *string_payload() { payload.text=&text[0]; return &payload; }
};
struct StringList {
    std::deque<Node> rows; Node *head_=nullptr,*current_=nullptr;
    void seek_pos(int i) { current_=&rows.at(i); }
    void add(const char *text,int id) {
        rows.push_back({id,text,{nullptr}}); head_=&rows.front(); current_=&rows.back();
    }
};
struct EditGroup {
    std::vector<char*> inputs;
    void edit(char *,char *initial,int limit) {
        assert(limit==(inputs.size()==1 ? 0x27 : 0x17)); inputs.push_back(initial);
    }
};
struct Dialogs : EditGroup {
    StringList item_list_;
    int get_num_items() { return int(item_list_.rows.size()); }
    void item(char *text,int id) { item_list_.add(text,id); event("item:"+std::string(text)); }
};
struct BasePop {
    Dialogs dialogs_;
    /* ACTUAL ACCESSOR */
    int init(int a,int b) { assert(a==0 && b==0); event("init"); return 0; }
};
struct Popup : BasePop {
    Popup() { ++popup_ctors; event("popup+"); }
    ~Popup() { ++popup_dtors; event("popup-"); }
    void start(char*,const char *label,int a,char *text,int flags,void *extra) {
        assert(std::string(label)=="CUSTOMNAMES" && a==-1 && !text && flags==0x200044 && !extra);
        event("start");
    }
    void set_loc(int x,int y) { assert(x==0x1000 && y==0x1000); }
    int exec(int flags,int(*poll)()) {
        assert(flags==0 && poll && dialogs_.inputs.size()==7);
        if(long_gender_label)assert(std::strlen(dialogs_.inputs[6])==127);
        std::strcpy(dialogs_.inputs[6],edited_gender.c_str()); event("editor"); poll(); return editor_result;
    }
    int input_poll_state() { return poll_state; }
};
static std::vector<std::pair<int,std::string>> visible;
struct PullDown {
    std::vector<std::pair<int,std::string>> items;
    PullDown() { ++pull_ctors; event("pull+"); }
    ~PullDown() { ++pull_dtors; event("pull-"); }
    void init(void *p) { assert(!p); items.clear(); }
    void add_item(int id,char *text) { items.emplace_back(id,text); }
    void add_separator(int id) { items.emplace_back(id,"*"); }
    void show(int x,int y,int(*poll)()) { assert(x==110 && y==220 && poll); visible=items; event("menu"); poll(); }
    int get_selected() { return answer; }
    int get_selected_id() { return answer>=0 && answer<64 ? items.at(answer).first : -1; }
};
/* ACTUAL MENU */
/* ACTUAL MENU METHODS */
static char *label_get(int id) {
    static char male[]="Male",female[]="Female",def[]="Default",other[]="label";
    static char long_female[128];
    if(id==0xb8c/4 && long_gender_label) {
        std::memset(long_female,'F',sizeof long_female-1); long_female[sizeof long_female-1]=0;
        return long_female;
    }
    static char names[6][24]={"default0","default1","default2","default3","default4","default5"};
    if(id==0xfe4/4 && fresh_default_labels) {
        assert(default_lookups<6); event("default_lookup"); return names[default_lookups++];
    }
    return id==0xb88/4 ? male : id==0xb8c/4 ? female : id==0xfe4/4 ? def : other;
}
static int _strnicmp(const char *a,const char *b,unsigned n) {
    for(unsigned i=0;i<n;++i) {
        int aa=std::tolower(static_cast<unsigned char>(a[i])),bb=std::tolower(static_cast<unsigned char>(b[i]));
        if(aa!=bb || !aa || !bb)return aa-bb;
    }
    return 0;
}
static char *CharUpper(char *text) {
    for(char*p=text;*p;++p)*p=std::toupper(static_cast<unsigned char>(*p));
    event("upper"); return text;
}
/* ACTUAL CHECKSUM */
static void parse_says(int index,char *text,int a,int b) {
    assert(index==0 && a==-1 && b==-1); event("parse:"+std::string(text));
}
static int X_pop_ask(const char *label,int value,char *initial,int(*poll)(),int flags) {
    assert(value==0x50 && !poll); assert(ask_index<int(ask_texts.size()));
    assert(initial==(std::string(label)=="VERIFYPASSWORD" ? &NetWinPasswordVerifyInitialText : &NetWinPasswordPromptInitialText));
    assert(flags==(std::string(label)=="VERIFYPASSWORD" ? 0x800000 : 0x800040));
    std::strcpy(ParseStrBuffer[0].str,ask_texts[ask_index].c_str());
    int result=ask_results.at(ask_index++); event("ask:"+std::string(label)); return result;
}
static void X_pop(const char *label,void *text) { assert(!text); event("pop:"+std::string(label)); }
static void X_pop(char *file,const char *label,void *text) { assert(file==ScriptPopupFile && !text); event("pop:"+std::string(label)); }
struct Net {
    uint32_t local_player_id_ = 27, host_player_id_ = 39;
    uint32_t local_player_id() const { return local_player_id_; }
    uint32_t host_player_id() const { return host_player_id_; }
    void send(void *data,unsigned long size,int target,int flags) {
        assert(size==0x18c && flags==1); auto *p=static_cast<NetWinLobbySinglePacket*>(data);
        assert(p->header_.opcode_==4 && p->header_.flags_==0x2f && p->header_.field_8_==0);
        sent_slot=p->header_.slot_; sent_row=p->row_; sent_target=target; ++sends; event("send");
    }
};
struct NetDaemon : Net {
    int player_count_=2;
    int pid_2_idx(uint32_t id) { assert(id==local_player_id()); ++identity_calls; event("identity"); return local_slot; }
} NetDaemonNet;
static int &player_count=NetDaemonNet.player_count_;
struct NetWin {
    void *faction_records_;
    void pick_player(); void pick_faction(int); int pick_pass(int);
    int modal_callback_daemon() { event("daemon"); return 0; }
    void get_mouse_pos(int *x,int*y) { *x=10; *y=20; event("mouse"); }
    void client_to_screen(int *x,int*y) { *x+=100; *y+=200; event("screen"); }
};
static NetWin NetWinDialog;
/* ACTUAL HELPERS */
static char *copied_string(char *dst,const char *src) {
    std::strcpy(dst,src); event("strcpy"); return dst;
}
#define strcpy copied_string
/* ACTUAL BODIES */
#undef strcpy
static void reset(NetWin&w) {
    callback={}; events.clear(); visible.clear(); edited_gender="Female";
    fresh_default_labels=false; long_gender_label=false; default_lookups=0;
    std::memset(NetWinLobbyRows,0x5a,sizeof NetWinLobbyRows);
    for(auto&row:NetWinLobbyRows) {
        row.faction_or_color_=-1;
        std::strcpy(row.leader_name_,"row-leader"); std::strcpy(row.formal_faction_name_,"row-formal");
        std::strcpy(row.noun_faction_,"row-noun"); std::strcpy(row.adjective_faction_,"row-adj");
        std::strcpy(row.leader_title_,"row-title"); std::strcpy(row.adjective_leader_,"row-leaderadj");
    }
    std::memset(table_a,0,sizeof table_a); std::memset(table_b,0,sizeof table_b);
    table_a[0].record_count_=4; table_b[0].record_count_=7;
    for(int i=0;i<8;++i) {
        table_a[i].active_=table_b[i].active_=1;
        std::sprintf(table_a[i].noun_faction_,"a%d",i); std::sprintf(table_b[i].noun_faction_,"b%d",i);
    }
    w.faction_records_=table_a; NetWinSessionData.mode_=0; player_count=2;
    local_slot=2; answer=0; editor_result=0; poll_state=0; NetDaemonNet.host_player_id_=39;
    popup_ctors=popup_dtors=pull_ctors=pull_dtors=sends=copies=identity_calls=ask_index=0;
    observe_owner=false; owner_accesses=0; ask_texts.clear(); ask_results.clear();
    std::memset(ParseStrBuffer,0,sizeof ParseStrBuffer); std::memset(Players,0,sizeof Players);
    for(int i=0;i<6;++i)std::sprintf(ParseStrBuffer[i].str,"custom%d",i);
    std::strcpy(Players[3].name_leader,"leader"); std::strcpy(Players[3].formal_name_faction,"formal");
    std::strcpy(Players[3].noun_faction,"noun"); std::strcpy(Players[3].adj_name_faction,"adj");
    std::strcpy(Players[3].title_leader,"title"); std::strcpy(Players[3].adj_leader,"leaderadj");
    Players[3].is_leader_female=1;
}
static void ask(std::initializer_list<const char*> texts,std::initializer_list<int> results) {
    ask_texts.clear(); for(auto*t:texts)ask_texts.emplace_back(t); ask_results=results;
}
static int pos(const char *s) { auto p=std::find(events.begin(),events.end(),s); assert(p!=events.end()); return int(p-events.begin()); }
static void lifetime(int popup,int pull=0) {
    assert(popup_ctors==popup && popup_dtors==popup && pull_ctors==pull && pull_dtors==pull);
    assert(events.front()=="popup+" && events.back()=="popup-");
    if(sends)assert(pos("send")<int(events.size())-1);
}
static void items(std::initializer_list<const char*> texts) {
    assert(visible.size()==texts.size()); unsigned i=0;
    for(auto*t:texts)assert(visible[i++].second==t);
}
int main() {
    (void)&owner_access;
    static_assert(sizeof(NetWinLobbyRow)==0x17c && sizeof(NetWinFactionRecord)==0x190 && sizeof(Player)==0x59c,"actual passive widths");
    NetWin w;
    // Complete player branches and the constructor-before-mode early path.
    for(int mode : {2,3}) { reset(w); NetWinSessionData.mode_=mode; w.pick_player(); lifetime(1); assert(!identity_calls && !sends); }
    reset(w); NetWinSessionData.mode_=2;
    callback=[](const std::string&e){if(e=="popup+")NetWinSessionData.mode_=0;};
    w.pick_player(); lifetime(1); assert(sends==1 && identity_calls==1);
    for(int faction : {-1,3})for(int state : {0,1})for(int slot : {0,2})for(int result : {-1,0}) {
        reset(w); local_slot=slot; editor_result=result; poll_state=state;
        NetWinLobbyRows[slot].faction_or_color_=static_cast<int8_t>(faction);
        NetWinLobbyRow before=NetWinLobbyRows[slot];
        w.pick_player(); lifetime(1); assert(std::memcmp(&before,&NetWinLobbyRows[slot],sizeof before)==0);
        assert(sends==(result>=0 && slot>=1));
        if(sends) {
            assert(sent_slot==slot && sent_target==39);
            if(state==1 && faction==-1) {
                assert(sent_row.tail_[0]==10 && std::string(sent_row.leader_name_)=="Default");
            } else if(state==1) {
                assert(sent_row.tail_[0]==1 && std::string(sent_row.adjective_faction_)=="adj");
            } else {
                assert(sent_row.tail_[0]==1 && std::string(sent_row.leader_name_)=="custom0");
            }
            assert(sent_row.owner_==before.owner_ && sent_row.tail_[1]==before.tail_[1]);
        }
    }
    reset(w); poll_state=1; fresh_default_labels=true; w.pick_player(); lifetime(1);
    assert(default_lookups==6 && sent_row.tail_[0]==10);
    assert(std::string(sent_row.leader_name_)=="default0" && std::string(sent_row.formal_faction_name_)=="default1");
    assert(std::string(sent_row.noun_faction_)=="default2" && std::string(sent_row.adjective_faction_)=="default3");
    assert(std::string(sent_row.leader_title_)=="default4" && std::string(sent_row.adjective_leader_)=="default5");
    int lookup_start=pos("default_lookup");
    for(int i=0;i<12;++i)assert(events.at(lookup_start+i)==(i%2 ? "strcpy" : "default_lookup"));
    // Safe host witness inside the native 100h window, beyond editlen17h.
    // Do not run a narrowed-buffer mutation or infer the native overrun tail.
    reset(w); long_gender_label=true; w.pick_player(); lifetime(1);
    assert(sends==1 && sent_row.tail_[0]==1);
    // Count is captured after default insertion, allocation is live thereafter.
    reset(w); answer=-1; callback=[&](const std::string&e){if(e=="item:a0")w.faction_records_=table_b;};
    w.pick_faction(1); lifetime(1,1); items({"Default","a0","b1","b2","b3"}); assert(!sends);
    reset(w); answer=-1; table_b[0].record_count_=2;
    callback=[&](const std::string&e){if(e=="item:Default")w.faction_records_=table_b;};
    w.pick_faction(1); lifetime(1,1); items({"Default","b0","b1"});
    for(int mode : {2,3})for(int count : {-3,0,2}) {
        reset(w); answer=-1; NetWinSessionData.mode_=mode; player_count=count;
        NetWinLobbyRows[1].faction_or_color_=1; NetWinLobbyRows[2].faction_or_color_=3;
        w.pick_faction(1); lifetime(1,1);
        if(count<0)items({"Default"}); else if(count==0)items({"Default","a0","a1","a2","a3"}); else items({"Default","a0","a2"});
    }
    reset(w); answer=-1; NetWinSessionData.mode_=2; player_count=0;
    callback=[](const std::string&e){if(e=="item:a0")player_count=-1;};
    w.pick_faction(1); lifetime(1,1); items({"Default","a0"});
    reset(w); answer=-1; table_a[0].record_count_=0;
    w.pick_faction(1); lifetime(1,1); items({"Default"});
    // Native out-of-range selected slot yields ID -1 then faction -2.
    // New-mode payload1 bypasses authentication, so this remains allocated.
    reset(w); answer=64; w.pick_faction(1); lifetime(2,1);
    assert(sends==1 && sent_row.faction_or_color_==-2 && NetWinLobbyRows[1].faction_or_color_==-1);
    // Actual caller and authentication callee keep distinct requested/local rows.
    reset(w); NetWinSessionData.mode_=2; player_count=0; answer=1;
    NetWinLobbyRows[1].faction_or_color_=2; NetWinLobbyRows[2].faction_or_color_=3;
    NetWinLobbyRows[2].owner_=0; table_a[3].owner_=checksum_password("old"); table_b[3].owner_=checksum_password("new");
    events.clear(); ask({"new"},{0});
    callback=[&](const std::string&e) {
        if(e=="identity")assert(NetWinLobbyRows[1].faction_or_color_==0);
        if(e=="ask:ENTERPASSWORDFACTION") { w.faction_records_=table_b; NetDaemonNet.host_player_id_=99; NetWinLobbyRows[1].faction_or_color_=6; }
    };
    w.pick_faction(1); lifetime(2,1);
    assert(sends==1 && sent_slot==1 && sent_target==99 && sent_row.faction_or_color_==0);
    assert(NetWinLobbyRows[1].faction_or_color_==2 && NetWinLobbyRows[2].owner_==table_b[3].owner_);
    reset(w); NetWinSessionData.mode_=2; player_count=0; answer=1;
    NetWinLobbyRows[1].faction_or_color_=2; NetWinLobbyRows[2].faction_or_color_=3;
    NetWinLobbyRows[2].owner_=0; table_a[3].owner_=checksum_password("right");
    events.clear(); ask({"wrong"},{0}); w.pick_faction(1); lifetime(2,1);
    assert(!sends && !copies && NetWinLobbyRows[1].faction_or_color_==2 && NetWinLobbyRows[2].owner_==0);
    // Password success, cancel, bad hash, empty hash, and live table validation.
    for(int payload : {0,1})for(int type : {0,1,2,3,4,5,6,7}) {
        reset(w); NetWinSessionData.mode_=2; NetWinLobbyRows[2].faction_or_color_=3;
        NetWinLobbyRows[2].owner_=77;
        uint32_t right=checksum_password("right"),wrong=checksum_password("wrong"); events.clear();
        table_a[3].owner_=type==0 ? 0 : type==1 ? 77 : right;
        ask({type==5 ? "" : "right"},{type==2 ? -1 : type==7 ? -2 : 0});
        callback=[&](const std::string&e) {
            if(e=="ask:ENTERPASSWORDFACTION" && type>=4 && type<=6) {
                w.faction_records_=table_b; table_b[3].owner_=type==4 ? right : type==5 ? 0 : wrong;
            }
        };
        if(type==3)ask({"wrong"},{0});
        observe_owner=type==0;
        int result=w.pick_pass(payload); observe_owner=false; lifetime(1);
        assert(result==((type<=1 || type==4 || type==5 || type==7) ? 1 : 0)); assert(!sends && !copies);
        if(type==0)assert(owner_accesses==0);
        if(type<=1)assert(ask_index==0);
        if(type==4)assert(NetWinLobbyRows[2].owner_==right);
        if(type==5)assert(NetWinLobbyRows[2].owner_==0);
        if(type==3 || type==6)assert(pos("pop:BADPASSWORD")>=0);
    }
    reset(w); NetWinSessionData.mode_=3; NetWinLobbyRows[2].faction_or_color_=-1;
    assert(w.pick_pass(0)==1 && NetWinLobbyRows[2].owner_==0); lifetime(1); assert(!sends && !ask_index);
    // Identity-boundary callbacks are source order controls, not a claim that
    // the actual native pid_2_idx leaf mutates the mode.
    reset(w); NetWinLobbyRows[2].faction_or_color_=3; ask({"cancel"},{-1});
    callback=[](const std::string&e){if(e=="identity")NetWinSessionData.mode_=2;};
    assert(w.pick_pass(0)==1); lifetime(1); assert(!ask_index && !sends);
    assert(pos("pop:PASSWORDOK")>=0);
    reset(w); NetWinSessionData.mode_=2; NetWinLobbyRows[2].faction_or_color_=3;
    NetWinLobbyRows[2].owner_=0; table_a[3].owner_=checksum_password("old"); table_b[3].owner_=checksum_password("new");
    events.clear(); ask({"new"},{0});
    callback=[&](const std::string&e){if(e=="upper") { w.faction_records_=table_b; NetWinSessionData.mode_=0; }};
    assert(w.pick_pass(0)==1); lifetime(1); assert(NetWinLobbyRows[2].owner_==table_b[3].owner_ && !sends);
    for(int slot : {0,2})for(int sequence : {0,1,2,3,4}) {
        reset(w); local_slot=slot; NetWinLobbyRows[slot].faction_or_color_=3;
        if(sequence==0)ask({""},{0});
        if(sequence==1)ask({"right","right"},{0,0});
        if(sequence==2)ask({"wrong","different","right","right"},{0,0,0,0});
        if(sequence==3)ask({"right","right"},{0,-1}); // verification return ignored
        if(sequence==4)ask({"right","","right","right"},{0,0,0,0});
        assert(w.pick_pass(0)==1); lifetime(1); assert(copies==1 && sends==(slot>=1));
        assert(NetWinLobbyRows[slot].owner_==(sequence==0 ? 0 : checksum_password("right")));
        if(sends)assert(std::memcmp(&sent_row,&NetWinLobbyRows[slot],sizeof sent_row)==0);
    }
    reset(w); NetWinLobbyRows[2].faction_or_color_=3;
    ask({"cancel"},{-1}); assert(w.pick_pass(0)==0); lifetime(1); assert(!sends && !copies);
    reset(w); assert(w.pick_pass(1)==1); lifetime(1); assert(!ask_index && !sends);
    // Default faction acceptance snapshots before the slot-zero send guard.
    reset(w); w.pick_faction(0); lifetime(2,1); assert(copies==1 && !sends);
    return 0;
}
'''


def compile_run(tmp_path,compiler,bodies=None,instrument=False):
    records,menu,accessor,owners,helpers,checksum,real=material()
    bodies=dict(bodies or real)
    if instrument:
        bodies["pass"]=bodies["pass"].replace("row.owner_","owner_access(row)")
    text=HARNESS
    for name,value in (("RECORDS",records),("MENU",menu),("ACCESSOR",accessor),
                       ("MENU METHODS",owners),("HELPERS",helpers),("CHECKSUM",checksum),
                       ("BODIES","\n".join(bodies.values()))):
        text=text.replace("/* ACTUAL "+name+" */",value)
    cpp=tmp_path/"editors.cpp";exe=tmp_path/"editors";cpp.write_text(text)
    build=subprocess.run([compiler,"-std=c++17","-Wall","-Wextra","-Werror",str(cpp),"-o",str(exe)],capture_output=True,text=True)
    assert build.returncode==0,build.stdout+build.stderr
    return subprocess.run([str(exe)],capture_output=True,text=True)


@pytest.mark.parametrize("compiler",("g++","clang++"))
@pytest.mark.parametrize("instrument",(False,True),ids=("ordinary","allocated-read-evaluation"))
def test_complete_coupled_actual_source(tmp_path,compiler,instrument):
    compiler=shutil.which(compiler)
    if not compiler:pytest.skip("host compiler unavailable")
    result=compile_run(tmp_path,compiler,instrument=instrument)
    assert result.returncode==0,result.stdout+result.stderr


MUTANTS=(
    ("late_popup","player","    Popup popup;",""),
    ("gender_boolean","player","updated.tail_[0] = 10;","updated.tail_[0] = 0;"),
    ("old_gender","player","updated.tail_[0] = 10;","updated.tail_[0] = updated.tail_[0];"),
    ("cached_table","faction","    for (int faction = 0; faction < count; ++faction) {\n        // Native 47C618 reloads the allocation, but not the captured count.\n        NetWinFactionRecord *const records =\n            static_cast<NetWinFactionRecord *>(faction_records_);",
     "    NetWinFactionRecord *const records = static_cast<NetWinFactionRecord *>(faction_records_);\n    for (int faction = 0; faction < count; ++faction) {"),
    ("live_record_count","faction","faction < count;","faction < static_cast<NetWinFactionRecord *>(faction_records_)[0].record_count_;"),
    ("negative_count_normalized","faction","already_selected = player_count < 0;","already_selected = false;"),
    ("missing_player_reload","faction","            player_count = NetDaemonNet.player_count_;",""),
    ("no_faction_snapshot_slot_zero","faction","memcpy_call(&updated, &NetWinLobbyRows[slot], 0x17C);","if(slot>=1) memcpy_call(&updated, &NetWinLobbyRows[slot], 0x17C);"),
    ("no_restore_faction","faction","NetWinLobbyRows[slot].faction_or_color_ = previous;","(void)previous;"),
    ("invalid_selection_normalized","faction","const int chosen_faction = pop_menu.get_selected_id() - 1;",
     "const int selected_id=pop_menu.get_selected_id(); const int chosen_faction=selected_id<0 ? -1 : selected_id-1;"),
    ("entry_hash_validation","pass","if (records[faction].owner_ != entered_hash)","(void)records; if (table_a[faction].owner_ != entered_hash)"),
    ("bad_hash_accepted","pass","if (records[faction].owner_ != entered_hash)","if (records[faction].owner_ == entered_hash)"),
    ("eager_row_owner","pass","        if (expected_hash == 0 || expected_hash == row.owner_)","        const uint32_t stored_hash = row.owner_;\n        if (expected_hash == 0 || expected_hash == stored_hash)"),
    ("verify_return_checked","pass","        X_pop_ask(\"VERIFYPASSWORD\", 0x50,\n                  &NetWinPasswordVerifyInitialText, nullptr, 0x800000);",
     "        if(X_pop_ask(\"VERIFYPASSWORD\", 0x50,\n                  &NetWinPasswordVerifyInitialText, nullptr, 0x800000)==-1)return 0;"),
    ("no_password_snapshot_slot_zero","pass","memcpy_call(&updated, &row, 0x17C);","if(slot>=1) memcpy_call(&updated, &row, 0x17C);"),
    ("mode_before_identity","pass","    const int slot = NetDaemonNet.pid_2_idx(NetDaemonNet.local_player_id());\n    const bool faction_mode = NetWinSessionData.mode_ == 2 ||\n                              NetWinSessionData.mode_ == 3;",
     "    const bool faction_mode = NetWinSessionData.mode_ == 2 ||\n                              NetWinSessionData.mode_ == 3;\n    const int slot = NetDaemonNet.pid_2_idx(NetDaemonNet.local_player_id());"),
    ("negative_result_normalized","pass","nullptr, 0x800040) == -1)","nullptr, 0x800040) < 0)"),
    ("table_before_checksum","pass","if (records[faction].owner_ != entered_hash)","(void)records; if (checked_records[faction].owner_ != entered_hash)"),
    ("cached_default_lookup","player","strcpy(updated.leader_name_, label_get(0xFE4 / 4));",
     "char *const cached_default=label_get(0xFE4 / 4); strcpy(updated.leader_name_, cached_default);"),
    ("caller_failure_sends","faction","if (accepted == 0)","if (accepted == 2)"),
)


@pytest.mark.parametrize("label,name,old,new",MUTANTS,ids=[m[0] for m in MUTANTS])
def test_compiled_executed_damage_controls(tmp_path,label,name,old,new):
    compiler=shutil.which("g++")
    if not compiler:pytest.skip("host compiler unavailable")
    bodies=material()[-1];assert bodies[name].count(old)==1
    if label=="late_popup":
        bodies[name]=bodies[name].replace(old,new).replace("    popup.start(","    Popup popup;\n    popup.start(")
    else:bodies[name]=bodies[name].replace(old,new)
    if label=="table_before_checksum":
        bodies[name]=bodies[name].replace("        if (have_password) {",
            "        NetWinFactionRecord *const checked_records = static_cast<NetWinFactionRecord *>(faction_records_);\n        if (have_password) {")
    if label=="cached_default_lookup":
        for field in ("formal_faction_name_","noun_faction_","adjective_faction_","leader_title_","adjective_leader_"):
            bodies[name]=bodies[name].replace("strcpy(updated."+field+", label_get(0xFE4 / 4));",
                "strcpy(updated."+field+", cached_default);")
    result=compile_run(tmp_path,compiler,bodies,instrument=label=="eager_row_owner")
    assert result.returncode!=0,label+" survived"


def test_pinned_complete_native_control_and_lifetime_contracts():
    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    path=Path(os.environ.get("OPENSMACX_IMAGE","/home/fran6co/code/OpenSMACX/.opensmacx/game/terranx_original.exe"))
    assert path.is_absolute() and hashlib.sha256(path.read_bytes()).hexdigest()==PIN
    image=PeImage(path.read_bytes(),PIN);decoder=Cs(CS_ARCH_X86,CS_MODE_32);all_ins={}
    for start,end,cold,cold_end,base in ((0x47BBF0,0x47C527,0x656740,0x6568F9,0x5798),
        (0x47C530,0x47C965,0x6568F9,0x6569C4,0x65E4),
        (0x47C970,0x47CF32,0x6569C4,0x656AED,0x56A0)):
        ins=list(decoder.disasm(image.read(Span(start,end)),start))
        unwind=list(decoder.disasm(image.read(Span(cold,cold_end)),cold))
        assert ins[-1].address+ins[-1].size==end and ins[-1].mnemonic=="ret"
        assert unwind[-1].address+unwind[-1].size==cold_end
        all_ins.update({i.address:(i.mnemonic,i.op_str) for i in ins})
        assert unwind[0].mnemonic=="lea" and unwind[0].op_str==f"ecx, [ebp - {hex(base)}]"
        assert unwind[1].mnemonic=="jmp" and unwind[1].op_str=="0x406c00"
        assert any(i.mnemonic=="call" and i.op_str=="0x404900" for i in ins)
    pins={
        0x47BC1B:("call","0x4048a0"),0x47BC20:("mov","eax, dword ptr [0x90e778]"),
        0x47BC51:("je","0x47c485"),0x47BC5A:("je","0x47c485"),
        0x47BC6B:("call","0x4e25e0"),0x47BE21:("call","0x602600"),
        0x47BCA5:("lea","edx, [ebp - 0x41c]"),0x47BCAD:("call","0x645460"),
        0x47BDEB:("lea","edx, [ebp - 0x41c]"),0x47BDF1:("push","0x17"),
        0x47C231:("mov","word ptr [ebp - 0x31c], 0x2f04"),
        0x47C254:("lea","edx, [ebp - 0x31c]"),
        0x47BE28:("jge","0x47bf97"),0x47BFAA:("call","0x645930"),
        0x47BFBB:("jne","0x47c14f"),0x47BFC1:("movsx","eax, byte ptr [ebp - 0x18d]"),
        0x47BFCB:("je","0x47c062"),0x47C146:("mov","byte ptr [ebp - 0x1b], 0xa"),
        0x47C051:("mov","dl, byte ptr [esi + 0x946a50]"),0x47C1D8:("call","0x64fdf0"),
        0x47C1E8:("cmp","edi, 1"),0x47C212:("jl","0x47c26d"),0x47C268:("call","0x630080"),
        0x47C603:("mov","al, byte ptr [edx + 1]"),0x47C608:("mov","dword ptr [ebp - 0x20], eax"),
        0x47C60D:("mov","edx, dword ptr [0x93d4f8]"),0x47C618:("mov","eax, dword ptr [ebx + 0x772c]"),
        0x47C625:("je","0x47c673"),0x47C638:("test","edx, edx"),0x47C63A:("jle","0x47c656"),
        0x47C656:("cmp","eax, edx"),0x47C658:("jne","0x47c673"),
        0x47C668:("call","0x612a70"),0x47C66D:("mov","edx, dword ptr [0x93d4f8]"),
        0x47C676:("mov","eax, dword ptr [ebp - 0x20]"),0x47C6B8:("call","0x59d250"),
        0x47C6BF:("jge","0x47c719"),0x47C74B:("mov","byte ptr [esi + 0x90db9b], bl"),
        0x47C751:("call","0x47c970"),0x47C75B:("mov","byte ptr [esi + 0x90db9b], dl"),
        0x47C761:("jne","0x47c7fe"),0x47C811:("call","0x645930"),0x47C848:("jl","0x47c8a3"),
        0x47C89E:("call","0x630080"),0x47C9C9:("call","0x4048a0"),0x47C9DC:("call","0x4e25e0"),
        0x47C9E3:("mov","eax, dword ptr [0x90e778]"),0x47CAAB:("cmp","esi, -1"),
        0x47CAB4:("mov","dword ptr [ecx + 0x90dd10], ebx"),
        0x47CB4D:("mov","eax, dword ptr [edi + 0x772c]"),0x47CB5A:("cmp","eax, ebx"),
        0x47CB5C:("je","0x47cb6a"),0x47CB5E:("cmp","eax, dword ptr [ecx + 0x90dd10]"),
        0x47CC1E:("mov","edx, dword ptr [ecx + 0x772c]"),0x47CC52:("call","0x5bfab0"),
        0x47CC5D:("je","0x47ccd5"),0x47CC65:("je","0x47cd31"),0x47CC70:("call","0x5390c0"),
        0x47CC97:("call","0x5bfab0"),0x47CCAD:("call","0x5390c0"),
        0x47CCBA:("cmp","edi, eax"),0x47CCBC:("je","0x47ce5f"),0x47CCD0:("jmp","0x47cc08"),
        0x47CD43:("mov","eax, dword ptr [edx + 0x772c]"),0x47CD4C:("cmp","dword ptr [eax + ecx + 4], edi"),
        0x47CD50:("je","0x47ce0a"),0x47CE21:("mov","dword ptr [edx + 0x90dd10], edi"),
        0x47CE6D:("mov","dword ptr [eax + 0x90dd10], edi"),0x47CE7B:("call","0x645930"),
        0x47CEAD:("jl","0x47cf08"),0x47CF03:("call","0x630080"),
        0x47CF1F:("mov","eax, 1"),
    }
    for address,expected in pins.items():assert all_ins[address]==expected,(hex(address),all_ins.get(address))
    assert 0x5798-0x537C==0x41C and 0x41C-0x31C==0x100
    assert "char gender_text[0x100];" in material()[-1]["player"]
    lookups=(0x47C073,0x47C099,0x47C0BF,0x47C0E5,0x47C10B,0x47C131)
    copies=(0x47C080,0x47C0A6,0x47C0CC,0x47C0F2,0x47C118,0x47C13E)
    for lookup,copy in zip(lookups,copies):
        assert all_ins[lookup]==("call","0x6169a0")
        assert all_ins[copy]==("call","0x645460")
    # Actual compatibility wrapper is unchecked, not a mistaken safety fix.
    compat=material()[5];assert "strcpy(destination, source);" in compat

