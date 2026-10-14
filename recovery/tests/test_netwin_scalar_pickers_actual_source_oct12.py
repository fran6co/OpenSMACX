"""Complete scalar pickers, real menu API/exec, allocated callback controls.

The pin establishes native branches and subobject lifetimes. Host dependency
shims test C++ ownership/order, not target ABI, linked preservation or gameplay.
Reserved packet bytes are deliberately neither initialized nor inspected.
"""
from pathlib import Path
import hashlib
import os
import shutil
import struct
import subprocess

import pytest

from test_netwin_owner_behavior import extract_function
from test_net_hotseat_owner_actual_source_oct6 import installed_packet_declarations
from test_netdaemon_early_routes_oct12 import declaration, PIN

ROOT = Path(__file__).resolve().parents[2]
NAMES = ("num_players", "global_diff", "time", "planet_size", "ocean",
         "erosion", "native", "clouds")


def material():
    source = (ROOT / "src/netwin.cpp").read_text()
    records = installed_packet_declarations(("NetWinSessionOptionsState",
                                               "NetWinSessionOptionsPacket"))
    menu = extract_function((ROOT / "src/popmenu.h").read_text(), "class PopMenu : public Popup, public PullDown") + ";"
    accessor = extract_function((ROOT / "src/basepop.h").read_text(), "Dialogs &dialogs()")
    owner = (ROOT / "src/popmenu.cpp").read_text()
    menu_methods = "\n".join(extract_function(owner, name) for name in (
        "PopMenu::~PopMenu()", "int PopMenu::init()", "int PopMenu::exec(int"))
    methods = {name: extract_function(source, "void NetWin::pick_" + name + "()") for name in NAMES}
    # Complete actual case-1 tail, including its gate, selector and send join.
    # The parent spot/other cases are unchanged by this bounded caller change.
    click = extract_function(source, "void NetWin::on_left_click(int")
    start = click.index("    case 1:\n        if (*NetWinRulesGate") + len("    case 1:\n")
    end = click.index("\n    case 2:\n        if (NetDaemonNet", start)
    methods["rule_toggle"] = "static void rule_toggle(int spot_position) {\n" + click[start:end] + "}\n"
    helpers = "\n".join(extract_function(source, name) for name in (
        "static int __cdecl pick_daemon_callback()", "static void pick_send_state()",
        "static void pick_send_state_with_captured_target()",
        "static int pick_answer(NetWin"))
    return records, menu, accessor, menu_methods, methods, helpers


HARNESS = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <deque>
#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#define __cdecl
static std::vector<std::string> events;
static std::function<void(const std::string&)> callback;
static void event(const std::string&s) { events.push_back(s); if(callback)callback(s); }
static int answer, dialog_count, pull_count, sends, player_count, NetWinPlanetSizeLabelCount;
static int recipient_storage=39;
static bool trace_copy_reads=false;
static int &destination() { if(trace_copy_reads)event("destination_read"); return recipient_storage; }
static int *NetWinPlayerCount=&player_count;
static int rules_gate=1;
static int *NetWinRulesGate=&rules_gate;
static char StringTemp[256];
static int Difficulty[]={10,11,12,13,14,15};
/* ACTUAL RECORDS */
static NetWinSessionOptionsState options_storage, sent_options;
// Instrumented lvalue evaluation witnesses source order only. The real POD
// and native rep movsd acquire no artificial callback or provider semantics.
static NetWinSessionOptionsState &options() {
    if(trace_copy_reads)event("options_read");
    return options_storage;
}
#define NetWinSessionOptions options()
static int sent_target;
struct Payload { char *text; };
struct Node {
    int id; std::string text; Payload payload;
    Payload *string_payload() { payload.text=&text[0]; return &payload; }
};
struct StringList {
    std::deque<Node> rows;
    Node *head_=nullptr, *current_=nullptr;
    void seek_pos(int i) { current_=&rows.at(i); }
    void add(const char *text,int id) {
        rows.push_back({id,text,{nullptr}}); head_=&rows.front(); current_=&rows.back();
    }
};
struct Dialogs {
    StringList item_list_;
    Dialogs() { ++dialog_count; event("dialogs+"); }
    ~Dialogs() { event("dialogs-"); }
    int get_num_items() { return static_cast<int>(item_list_.rows.size()); }
    void item(const char *text,int id) { item_list_.add(text,id); event("item:"+std::string(text)); }
};
class BasePop {
    Dialogs dialogs_;
public:
    /* ACTUAL ACCESSOR */
    int init(int a,int b) { assert(a==0 && b==0); event("init"); return 0; }
};
struct Popup : BasePop {
    Popup() { event("popup+"); }
    ~Popup() { event("popup-"); }
};
static std::vector<std::pair<int,std::string>> visible;
struct PullDown {
    std::vector<std::pair<int,std::string>> items;
    PullDown() { ++pull_count; event("pull+"); }
    ~PullDown() { event("pull-"); }
    void init(void *p) { assert(!p); items.clear(); event("pull_init"); }
    void add_item(int id,const char *text) { items.emplace_back(id,text); }
    void add_separator(int id) { items.emplace_back(id,"*"); }
    void show(int x,int y,int (*poll)()) {
        assert(x==110 && y==220 && poll); visible=items; event("modal"); poll();
    }
    int get_selected() { return answer; }
    int get_selected_id() { return answer<0 ? answer : items.at(answer).first; }
};
/* ACTUAL MENU */
/* ACTUAL MENU METHODS */
static char *label_get(int id) {
    static char names[1100][24]; assert(id>=0 && id<1100);
    std::snprintf(names[id],sizeof names[id],"label:%x",id*4); return names[id];
}
static struct Strings {
    char *get(int id) { static char names[32][24]; assert(id>=0 && id<32);
        std::snprintf(names[id],sizeof names[id],"difficulty:%d",id); return names[id]; }
} StringTable;
static char *_itoa(int n,char *out,int base) { assert(base==10); std::sprintf(out,"%d",n); return out; }
static void time_controls_dialog(Popup *popup) {
    event("time_builder");
    for(int i=0;i<3;++i) popup->dialogs().item(label_get(i),i);
}
struct NetDaemon {
    uint32_t local_player_id_ = 27;
    uint32_t local_player_id() const { return local_player_id_; }
    uint32_t host_player_id() const { return static_cast<uint32_t>(destination()); }
    int pid_2_idx(uint32_t id) {
        assert(id==local_player_id()); event("identity"); trace_copy_reads=true; return 2;
    }
    void send(void *data,unsigned long size,int target,int flags) {
        assert(size==0x28 && flags==1); auto *p=static_cast<NetWinSessionOptionsPacket*>(data);
        assert(p->opcode_==0x2f02); sent_options=p->options_; sent_target=target;
        trace_copy_reads=false; ++sends; event("send");
    }
} NetDaemonNet;
class NetWin {
public:
    char world_size_labels_[8][128];
    int modal_callback_daemon() { event("daemon"); return 0; }
    void get_mouse_pos(int *x,int *y) { *x=10; *y=20; event("mouse"); }
    void client_to_screen(int *x,int *y) { *x+=100; *y+=200; event("screen"); }
    void pick_num_players(); void pick_global_diff(); void pick_time(); void pick_planet_size();
    void pick_ocean(); void pick_erosion(); void pick_native(); void pick_clouds();
};
static NetWin NetWinDialog;
/* ACTUAL HELPERS */
/* ACTUAL PICKERS */
static void invoke(NetWin&w,int kind) {
    void (NetWin::*methods[])()={&NetWin::pick_num_players,&NetWin::pick_global_diff,&NetWin::pick_time,
        &NetWin::pick_planet_size,&NetWin::pick_ocean,&NetWin::pick_erosion,&NetWin::pick_native,&NetWin::pick_clouds};
    (w.*methods[kind])();
}
static void reset(NetWin&w,int selected,int first=4) {
    callback={}; trace_copy_reads=false; events.clear(); visible.clear(); answer=selected; player_count=first;
    NetWinPlanetSizeLabelCount=2; dialog_count=pull_count=sends=0; recipient_storage=39;
    std::memset(&NetWinSessionOptions,0x5a,sizeof NetWinSessionOptions);
    std::memset(&sent_options,0,sizeof sent_options); sent_target=-1;
    for(int i=0;i<8;++i)std::sprintf(w.world_size_labels_[i],"world%d",i);
}
static int position(const char *e) {
    auto p=std::find(events.begin(),events.end(),e); assert(p!=events.end()); return int(p-events.begin());
}
static void lifetime() {
    assert(dialog_count==1 && pull_count==1 && sends==1);
    assert(position("dialogs+")<position("popup+") && position("popup+")<position("pull+"));
    assert(position("mouse")<position("screen") && position("screen")<position("modal"));
    assert(position("modal")<position("daemon") && position("daemon")<position("identity"));
    assert(position("identity")<position("send") && position("send")<position("pull-"));
    assert(position("pull-")<position("popup-") && position("popup-")<position("dialogs-"));
}
static void check_items(int kind,int first=4,int worlds=2) {
    int sizes[]={std::max(0,8-first),6,3,worlds+1,4,4,4,4}; assert(int(visible.size())==sizes[kind]);
    for(int i=0;i<int(visible.size());++i) {
        assert(visible[i].first==(kind==0 ? first+i : i));
        if(kind==0)assert(visible[i].second==std::to_string(first+i));
        if(kind==1)assert(visible[i].second=="difficulty:"+std::to_string(10+i));
        if(kind==3 && i>0)assert(visible[i].second=="world"+std::to_string(i-1));
    }
    if(kind>=3)assert(visible[0].second=="label:fe4");
}
int main() {
    static_assert(sizeof(NetWinSessionOptionsState)==0x18 && sizeof(NetWinSessionOptionsPacket)==0x28,"wire extent");
    static_assert(offsetof(NetWinSessionOptionsPacket,options_)==0x10,"wire offset");
    NetWin w;
    const int offsets[]={1,2,3,6,7,8,9,10};
    for(int kind=0;kind<8;++kind)for(int selected : {-1,0,1,255,256,2147483647}) {
        reset(w,selected); NetWinSessionOptionsState expected=NetWinSessionOptions;
        callback=[&](const std::string&e) {
            if(e=="modal") { player_count=7; NetWinSessionOptions.rule_bits_a_=0x11223344;
                expected.rule_bits_a_=0x11223344; }
            if(e=="identity") { NetWinSessionOptions.rule_bits_b_=0x55667788;
                expected.rule_bits_b_=0x55667788; recipient_storage=99; }
        };
        if(selected>=0) {
            int added=static_cast<int>(static_cast<uint32_t>(selected)+4U);
            int value=kind==0 ? std::min(7,std::max(2,added)) : kind<=2 ? selected : selected-1;
            reinterpret_cast<unsigned char*>(&expected)[offsets[kind]]=static_cast<unsigned char>(value);
        }
        invoke(w,kind); lifetime(); check_items(kind);
        assert((position("destination_read")<position("options_read"))==(kind==0 || kind==3));
        assert(std::memcmp(&sent_options,&expected,sizeof expected)==0 && sent_target==99);
    }
    // Native number lower bound is captured after init, before modal callbacks.
    for(int first : {-3,1,2,6,8,10}) {
        reset(w,0,first); invoke(w,0); lifetime();
        int captured=std::max(first,2); check_items(0,captured);
        assert(sent_options.player_count_==std::min(7,captured));
    }
    reset(w,0,2); callback=[](const std::string&e){if(e=="init")player_count=6;};
    invoke(w,0); lifetime(); check_items(0,6); assert(sent_options.player_count_==6);
    reset(w,0); callback=[](const std::string&e){if(e=="item:4")player_count=7;};
    invoke(w,0); lifetime(); check_items(0,4); assert(sent_options.player_count_==4);
    reset(w,1,2147483647); invoke(w,0); lifetime(); check_items(0,2147483647);
    assert(sent_options.player_count_==2);
    for(int final_count : {1,4}) {
        reset(w,-1); callback=[&](const std::string&e) {
            if(e=="item:world0")NetWinPlanetSizeLabelCount=final_count;
        };
        invoke(w,3); lifetime(); check_items(3,4,final_count);
    }
    for(int initial_count : {-3,0}) {
        reset(w,-1); NetWinPlanetSizeLabelCount=initial_count;
        invoke(w,3); lifetime(); check_items(3,4,0);
    }
    reset(w,-1); callback=[](const std::string&e) {
        if(e=="item:label:fe4")NetWinPlanetSizeLabelCount=3;
    };
    invoke(w,3); lifetime(); check_items(3,4,3);
    const uint32_t masks[]={0x800,2,8,4,0x1000,1,0x10,0x20,0x4000,0x200,0x40,0x100,0x2000,0x8000,0x80};
    for(int selected : {-2147483647-1,-1,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,99,100,101,2147483647}) {
        reset(w,0); rules_gate=1; NetWinSessionOptionsState expected=NetWinSessionOptions;
        if(selected>=0 && selected<=14)expected.rule_bits_a_^=masks[selected];
        else if(selected==16)expected.rule_bits_b_^=0x800000;
        else if(selected==17)expected.rule_bits_b_^=0x1000000;
        else if(selected==100)expected.rule_bits_c_^=0x10;
        callback=[&](const std::string&e) {
            if(e=="identity") { expected.rule_bits_c_^=0x800;
                NetWinSessionOptions.rule_bits_c_^=0x800; recipient_storage=99; }
        };
        rule_toggle(selected);
        assert(sends==1 && dialog_count==0 && pull_count==0 && sent_target==99);
        assert(std::memcmp(&sent_options,&expected,sizeof expected)==0);
        assert(position("identity")<position("destination_read") && position("destination_read")<position("options_read"));
        reset(w,0); rules_gate=0; rule_toggle(selected);
        assert(sends==0 && events.empty());
    }
    // Public owner accessors and the actual exec must expose the same list.
    reset(w,-1); {
        PopMenu menu; Popup *base=&menu; assert(&menu.dialogs()==&base->dialogs());
        menu.dialogs().item("owned",7); menu.dialogs().item("*",9);
        menu.exec(110,220,pick_daemon_callback);
        assert(visible.size()==2 && visible[0].first==7 && visible[0].second=="owned");
        assert(visible[1].first==9 && visible[1].second=="*");
    }
    return 0;
}
'''


def compile_run(tmp_path, compiler, methods=None):
    records, menu, accessor, menu_methods, real, helpers = material()
    text = HARNESS
    for marker, value in (("RECORDS", records), ("MENU", menu), ("ACCESSOR", accessor),
                          ("MENU METHODS", menu_methods), ("HELPERS", helpers),
                          ("PICKERS", "\n".join((methods or real).values()))):
        text = text.replace("/* ACTUAL " + marker + " */", value)
    cpp=tmp_path/"scalar.cpp"; exe=tmp_path/"scalar"
    cpp.write_text(text)
    built=subprocess.run([compiler,"-std=c++17","-Wall","-Wextra","-Werror",str(cpp),"-o",str(exe)],capture_output=True,text=True)
    assert built.returncode==0, built.stdout+built.stderr
    return subprocess.run([str(exe)],capture_output=True,text=True)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_complete_actual_source_and_public_menu_api(tmp_path, compiler):
    compiler=shutil.which(compiler)
    if not compiler: pytest.skip("host compiler unavailable")
    result=compile_run(tmp_path,compiler)
    assert result.returncode==0, result.stdout+result.stderr


MUTANTS = [
    ("detached_dialog", "global_diff", "Dialogs &dialogs = pop_menu.dialogs();", "Dialogs dialogs;"),
    ("extra_pulldown", "time", "PopMenu pop_menu;", "PopMenu pop_menu; PullDown extra;"),
    ("diff_default", "global_diff", "Dialogs &dialogs = pop_menu.dialogs();", "Dialogs &dialogs = pop_menu.dialogs(); dialogs.item(label_get(0xFE4 / 4),0);"),
    ("diff_off_by_one", "global_diff", "static_cast<unsigned char>(answer)", "static_cast<unsigned char>(answer - 1)"),
    ("time_off_by_one", "time", "static_cast<unsigned char>(answer)", "static_cast<unsigned char>(answer - 1)"),
    ("number_old_result", "num_players", "static_cast<uint32_t>(first)", "static_cast<uint32_t>(-1)"),
    ("number_live_first", "num_players", "static_cast<uint32_t>(first)", "static_cast<uint32_t>(*NetWinPlayerCount)"),
    ("number_no_clamp", "num_players", "else if (count > 7) count = 7;", "else if (count > 7) count = count;"),
    ("number_no_lower_clamp", "num_players", "if (count < 2) count = 2;", "if (count < 2) count = count;"),
    ("cached_planet_count", "planet_size", "for (int index = 0; index < NetWinPlanetSizeLabelCount; ++index)", "int cached=NetWinPlanetSizeLabelCount; for (int index = 0; index < cached; ++index)"),
] + [("cancel_"+n,n,"    "+("pick_send_state_with_captured_target" if n in ("num_players","planet_size") else "pick_send_state")+"();",
      "    if (answer >= 0) "+("pick_send_state_with_captured_target" if n in ("num_players","planet_size") else "pick_send_state")+"();") for n in NAMES]
MUTANTS += [("early_target_order", "num_players", "pick_send_state_with_captured_target();", "pick_send_state();"),
            ("late_target_order", "global_diff", "pick_send_state();", "pick_send_state_with_captured_target();")]
MUTANTS += [("default_"+n,n,"static_cast<unsigned char>(answer - 1)","static_cast<unsigned char>(answer)")
            for n in ("planet_size","ocean","erosion","native","clouds")]
MUTANTS += [("rule_click_late_destination","rule_toggle","pick_send_state_with_captured_target();","pick_send_state();"),
            ("rule_click_wrong_toggle","rule_toggle","rule_bits_a_ ^= 0x800;","rule_bits_a_ ^= 0x400;"),
            ("rule_click_default_return","rule_toggle","default: break;","default: return;")]


@pytest.mark.parametrize("label,name,old,new", MUTANTS, ids=[x[0] for x in MUTANTS])
def test_executed_damage_controls(tmp_path,label,name,old,new):
    compiler=shutil.which("g++")
    if not compiler: pytest.skip("host compiler unavailable")
    methods=material()[4]; assert methods[name].count(old)==1
    methods[name]=methods[name].replace(old,new)
    result=compile_run(tmp_path,compiler,methods)
    assert result.returncode!=0, label+" survived"


# Complete mapped primary/cold extents; offsets are evidence, not product APIs.
ROUTES = (
    (0x47E010,0x47E33B,0x656D9D,0x656E75,0x47E111,0x47E132,0x90E8E1,0x47E137,0x47E142,0x47E170),
    (0x47E340,0x47E640,0x656E75,0x656F4D,0x47E418,0x47E421,0x90E8E2,0x47E426,0x47E431,0x47E45F),
    (0x47E640,0x47E924,0x656F4D,0x657025,0x47E6FD,0x47E706,0x90E8E3,0x47E70B,0x47E716,0x47E744),
    (0x47E930,0x47EC60,0x657025,0x6570FD,0x47EA37,0x47EA42,0x90E8E6,0x47EA47,0x47EA52,0x47EA80),
    (0x47EC60,0x47EF8C,0x6570FD,0x6571D5,0x47ED62,0x47ED6D,0x90E8E7,0x47ED72,0x47ED7D,0x47EDAB),
    (0x47EF90,0x47F2BC,0x6571D5,0x6572AD,0x47F092,0x47F09D,0x90E8E8,0x47F0A2,0x47F0AD,0x47F0DB),
    (0x47F2C0,0x47F5EC,0x6572AD,0x657385,0x47F3C2,0x47F3CD,0x90E8E9,0x47F3D2,0x47F3DD,0x47F40B),
    (0x47F5F0,0x47F91C,0x657385,0x65745D,0x47F6F2,0x47F6FD,0x90E8EA,0x47F702,0x47F70D,0x47F73B),
)


def test_pinned_complete_native_selection_cancel_and_owner_lifetimes():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    path=Path(os.environ.get("OPENSMACX_IMAGE", "/home/fran6co/code/OpenSMACX/.opensmacx/game/terranx_original.exe"))
    assert path.is_absolute() and hashlib.sha256(path.read_bytes()).hexdigest()==PIN
    image=PeImage(path.read_bytes(),PIN); decoder=Cs(CS_ARCH_X86,CS_MODE_32)
    all_ins={}
    for start,end,cold,cold_end,modal,store,dest,join,identity,send in ROUTES:
        ins=list(decoder.disasm(image.read(Span(start,end)),start))
        unwind=list(decoder.disasm(image.read(Span(cold,cold_end)),cold))
        assert ins[-1].address+ins[-1].size==end and ins[-1].mnemonic=="ret"
        assert unwind[-1].address+unwind[-1].size==cold_end
        indexed={i.address:(i.mnemonic,i.op_str) for i in ins}; all_ins.update(indexed)
        assert indexed[modal]==("call","0x59d250")
        assert indexed[store]==("mov",f"byte ptr [{hex(dest)}], al")
        assert indexed[join]==("mov","eax, dword ptr [0x93d4f0]")
        assert indexed[identity]==("call","0x4e25e0")
        assert indexed[send]==("call","0x630080")
        # Every cancel branch joins identity and send, never the destructor.
        between=[i for i in ins if modal<i.address<store]
        assert any(i.mnemonic=="jl" and i.op_str==hex(join) for i in between)
        base=0x6328 if start==0x47E010 else 0x6300
        pull=base-0x537C
        ctors=[i for i in ins if i.mnemonic=="call" and i.op_str in ("0x4048a0","0x5f8800")]
        assert len(ctors)==2
        for call,offset in zip(ctors,(base,pull)):
            # The second construction has state initialization between LEA/call.
            assert any(i.mnemonic=="lea" and i.op_str==f"ecx, [ebp - {hex(offset)}]"
                       for i in ins[max(0,ins.index(call)-4):ins.index(call)])
        assert any(i.mnemonic=="jmp" and i.op_str=="0x421400" for i in unwind)
        assert any(i.mnemonic=="lea" and i.op_str==f"ecx, [ebp - {hex(base-0x21D0)}]" for i in unwind)
        # Source menu item receiver is the same embedded Dialogs, not a local.
        if start!=0x47E640:
            items=[i for i in ins if i.mnemonic=="call" and i.op_str=="0x612a70"]
            assert items
            for call in items:
                assert any(i.mnemonic=="lea" and i.op_str==f"ecx, [ebp - {hex(base-0x21D0)}]"
                           for i in ins[max(0,ins.index(call)-4):ins.index(call)])
        assert any(i.mnemonic=="call" and i.op_str=="0x5f88a0" and i.address>send for i in ins)
    pins={
        0x47E094:("cmp","edi, 2"),0x47E099:("mov","edi, 2"),
        0x47E11A:("add","eax, edi"),0x47E11C:("cmp","eax, 2"),
        0x47E121:("cmp","eax, 7"),0x47E126:("mov","eax, 7"),0x47E12D:("mov","eax, 2"),
        0x47E11F:("jl","0x47e12d"),0x47E124:("jle","0x47e132"),
        0x47E3BC:("mov","esi, 0x96c85c"),0x47E3DF:("cmp","esi, 0x96c874"),
        0x47E6C4:("call","0x589330"),0x47E9F4:("mov","ecx, dword ptr [0x80a6f0]"),
        0x47EA40:("dec","al"),0x47ED6B:("dec","al"),0x47F09B:("dec","al"),
        0x47F3CB:("dec","al"),0x47F6FB:("dec","al"),
    }
    for address,expected in pins.items(): assert all_ins[address]==expected
    # Source early/late helpers preserve this observed load/copy ordering.
    copy_order=((0x47E147,0x47E15D,0x47E16A,"edx"),
                (0x47E447,0x47E445,0x47E459,"ecx"),
                (0x47E72C,0x47E72A,0x47E73E,"ecx"),
                (0x47EA57,0x47EA6D,0x47EA7A,"edx"),
                (0x47ED93,0x47ED91,0x47EDA5,"ecx"),
                (0x47F0C3,0x47F0C1,0x47F0D5,"ecx"),
                (0x47F3F3,0x47F3F1,0x47F405,"ecx"),
                (0x47F723,0x47F721,0x47F735,"ecx"))
    for target,copy,opcode,register in copy_order:
        assert all_ins[target]==("mov",register+", dword ptr [0x93d4f4]")
        assert all_ins[copy]==("rep movsd","dword ptr es:[edi], dword ptr [esi]")
        assert all_ins[opcode]==("mov","word ptr [ebp - 0x44], 0x2f02")
    native_exec={i.address:(i.mnemonic,i.op_str) for i in decoder.disasm(image.read(Span(0x59D250,0x59D392)),0x59D250)}
    assert native_exec[0x59D25B]==("lea","esi, [ebx + 0x537c]")
    assert native_exec[0x59D270]==("lea","ecx, [ebx + 0x21d0]")
    assert native_exec[0x59D359]==("add","ecx, 0x21d0")
    click=list(decoder.disasm(image.read(Span(0x47B760,0x47BAF6)),0x47B760))
    assert click[-1].address+click[-1].size==0x47BAF6 and click[-1].mnemonic=="ret"
    click_ins={i.address:(i.mnemonic,i.op_str) for i in click}
    click_pins={0x47B8A7:("mov","eax, dword ptr [0x93e8c0]"),
                0x47B8AE:("je","0x47baee"),
                0x47B9A4:("mov","edx, dword ptr [0x93d4f0]"),
                0x47B9B0:("call","0x4e25e0"),
                0x47B9B5:("mov","eax, dword ptr [0x93d4f4]"),
                0x47B9C7:("rep movsd","dword ptr es:[edi], dword ptr [esi]"),
                0x47B9D7:("mov","word ptr [ebp - 0x28], 0x2f02"),
                0x47B9DD:("call","0x630080")}
    for address,expected in click_pins.items(): assert click_ins[address]==expected
    assert click_ins[0x47B8B7]==("cmp","eax, 0x64")
    assert click_ins[0x47B8BA]==("ja","0x47b9a4")
    assert click_ins[0x47B8C2]==("mov","cl, byte ptr [eax + 0x47bb80]")
    assert click_ins[0x47B8C8]==("jmp","dword ptr [ecx*4 + 0x47bb34]")
    targets=struct.unpack("<19I",image.read(Span(0x47BB34,0x47BB80)))
    selectors=image.read(Span(0x47BB80,0x47BBE5))
    assert len(selectors)==101 and targets[18]==0x47B9A4
    expected_targets=(0x47B8DB,0x47B8EA,0x47B8F6,0x47B902,0x47B90E,
                      0x47B91D,0x47B926,0x47B92F,0x47B938,0x47B944,
                      0x47B950,0x47B959,0x47B965,0x47B971,0x47B97D,
                      0x47B989,0x47B995,0x47B8CF,0x47B9A4)
    assert targets==expected_targets
    masks=(0x800,2,8,4,0x1000,1,0x10,0x20,0x4000,0x200,0x40,0x100,0x2000,0x8000,0x80)
    for target,mask in zip(targets,masks):
        assert click_ins[target]==("xor",f"dword ptr [0x90e8ec], {hex(mask) if mask>9 else mask}")
    assert click_ins[0x47B98E]==("xor","eax, 0x800000")
    assert click_ins[0x47B99A]==("xor","eax, 0x1000000")
    assert click_ins[0x47B99F]==("mov","dword ptr [0x90e8f0], eax")
    assert click_ins[0x47B8CF]==("xor","dword ptr [0x90e8f4], 0x10")
    for position,selector in enumerate(selectors):
        expected=position if position<=14 else 15 if position==16 else 16 if position==17 else 17 if position==100 else 18
        assert selector==expected

