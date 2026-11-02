"""Actual-owner controls for the seven proposed game-logic recoveries.

After coordinator admission, this test reads only tracked src/ owner records,
extracts their real annotated definitions (and the real diplo_add helper),
then compiles those bodies against readable host adapters below. It does not
load ignored candidate fragments and is not ABI, native-match, or game proof.
"""
from pathlib import Path
import re
import shutil
import subprocess

import pytest

from match_recovery.source import Mode, read_file


ROOT = Path(__file__).resolve().parents[2]
OWNER_BY_ADDRESS = {
    0x0050C340: ("src/faction.cpp", "?steal_tech@@YAHHHH@Z", "int __cdecl steal_tech("),
    0x00522F00: ("src/veh.cpp", "?do_fungal_towers@@YAXXZ", "void __cdecl do_fungal_towers("),
    0x005B4600: ("src/faction.cpp", "?social_set@@YAXH@Z", "void __cdecl social_set("),
    0x00545D60: ("src/faction.cpp", "?propose_trade_maps@@YAXHH@Z", "void __cdecl propose_trade_maps("),
    0x0053E4C0: ("src/faction.cpp", "?demands_withdrawal@@YAXHH@Z", "void __cdecl demands_withdrawal("),
    0x005589E0: ("src/faction.cpp", "?commlink_attempter@@YAHHH@Z", "int __cdecl commlink_attempter("),
    0x00549270: ("src/faction.cpp", "?demand_withdrawal@@YAHHH@Z", "int __cdecl demand_withdrawal("),
}


def _masked_cpp(source: str) -> str:
    """Mask comments and literals while preserving offsets and braces."""
    out = list(source)
    i = 0
    state = "code"
    quote = ""
    while i < len(source):
        c = source[i]
        n = source[i + 1] if i + 1 < len(source) else ""
        if state == "code":
            if c == "/" and n == "/":
                out[i] = out[i + 1] = " "
                i += 2
                state = "line"
                continue
            if c == "/" and n == "*":
                out[i] = out[i + 1] = " "
                i += 2
                state = "block"
                continue
            if c in ('"', "'"):
                quote = c
                out[i] = " "
                state = "quoted"
        elif state == "line":
            if c == "\n":
                state = "code"
            else:
                out[i] = " "
        elif state == "block":
            if c == "*" and n == "/":
                out[i] = out[i + 1] = " "
                i += 2
                state = "code"
                continue
            if c != "\n":
                out[i] = " "
        else:
            if c == "\\":
                out[i] = " "
                if i + 1 < len(source) and source[i + 1] != "\n":
                    out[i + 1] = " "
                    i += 2
                    continue
            elif c == quote:
                out[i] = " "
                state = "code"
            elif c != "\n":
                out[i] = " "
        i += 1
    return "".join(out)


def _definition(source: str, signature: str) -> str:
    masked = _masked_cpp(source)
    start = masked.find(signature)
    assert start >= 0, "missing actual definition: " + signature
    paren = masked.find("(", start)
    depth = 0
    close = -1
    for pos in range(paren, len(masked)):
        if masked[pos] == "(":
            depth += 1
        elif masked[pos] == ")":
            depth -= 1
            if depth == 0:
                close = pos
                break
    assert close >= 0
    brace = close + 1
    while brace < len(masked) and masked[brace].isspace():
        brace += 1
    assert brace < len(masked) and masked[brace] == "{"
    depth = 0
    for pos in range(brace, len(masked)):
        if masked[pos] == "{":
            depth += 1
        elif masked[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[start:pos + 1]
    raise AssertionError("unterminated actual definition: " + signature)


def actual_body(address: int) -> str:
    owner, decorated, signature = OWNER_BY_ADDRESS[address]
    records = [record for record in read_file(ROOT / owner)
               if record.address == address]
    assert len(records) == 1, f"{owner}: missing/duplicate source record {address:#x}"
    record = records[0]
    assert record.mode == Mode.BODY and record.name == decorated
    assert record.region, f"{owner}: source extraction failed: {record.extract_error}"
    assert signature in record.region
    assert "BODY GOES HERE." not in record.region
    # The source reader's region is already bounded by this mapped record and
    # the next one; keeping its complete text preserves writer metadata while
    # avoiding a second function-boundary parser.
    return record.region


def actual_helper(owner: str, signature: str) -> str:
    regions = [record.region for record in read_file(ROOT / owner)
               if record.region and signature in record.region]
    assert regions, f"{owner}: no source-reader region contains helper {signature}"
    return _definition(min(regions, key=len), signature)


HOST = r'''#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <limits.h>
#define __cdecl
enum { MaxFactions=8, BSC_FUNGAL_TOWER=19, VSTATE_UNK_4=4,
       BIT_FUNGUS=0x20, DTREATY_VENDETTA=0x10, STATE_DEBUG_MODE=0x1000,
       VK_CONTROL=0x11, VK_SHIFT=0x10 };
struct SocialCategory { int value; };
struct SocialEffect {
    enum EffectIndex { ECONOMY, EFFICIENCY, SUPPORT, TALENT, MORALE, POLICE,
                       GROWTH, PLANET, PROBE, INDUSTRY, RESEARCH };
    int values_[11];
};
struct PlayerData {
    SocialCategory soc_category_pending; SocialEffect soc_effect_pending;
    uint32_t diplo_treaties[8]; int diplo_spoke[8]; char diplo_patience[8];
};
struct Player {
    int is_leader_female,noun_gender,is_noun_plural;
    char title_leader[24],name_leader[24],noun_faction[24],adj_leader[64];
};
struct Base { int faction_id_current,nutrients_accumulated;
    int minerals_accumulated,minerals_accumulated_2; };
struct Veh { int proto_id,x,y; uint32_t state; uint8_t morale; };
struct Sprite {};
struct Console { int faction_id_; };
struct Window { int redraws,closes,execs; void redraw(){++redraws;}
    void close(){++closes;} void exec(int,int){++execs;} };
struct NetState { uint32_t leading_words_[4]; union Payload { uint32_t flags; int *vehicle_selection_follow; } payload_; uint32_t trailing_words_[10]; };
struct Net { NetState channels_[2]; struct PrimaryState { uint32_t leading_words_[3]; uint32_t flags_; int session_phase_; uint32_t trailing_words_[9]; } primary_state_; int waits,last_wait;
    uint32_t &factions_waiting_mask() { return primary_state_.trailing_words_[6]; }
    void await_diplo(int v){++waits;last_wait=v;} };
struct Message { int calls; const char *label; int topic;
    void pop(const char *s,int,int,const char*){++calls;label=s;} };
PlayerData PlayersData[8]; Player Players[8]; Base Bases[16]; Veh Vehs[16];
Sprite Portraits[8]; Sprite *FactionDiplomacyPortraits[8];
Console ConsoleGlobal; Window TutWinBaseWindow,DiploWindow; Net NetDaemonNet;
uint32_t &ConsoleFactionsWaitingMask=NetDaemonNet.factions_waiting_mask();
struct GameFields {
    union { uint16_t enable_mask_; struct { uint8_t alive_mask_, human_mask_; } bytes_; } lock_masks_;
    int vehicle_current_count_,base_current_count_,sunspot_duration_,turn_current_num_;
    uint32_t game_state_,net_turn_flags_;
};
struct { GameFields fields_; } GameDataStateStorage;
uint16_t &LockEnableMask=GameDataStateStorage.fields_.lock_masks_.enable_mask_;
int &BaseCurrentCount=GameDataStateStorage.fields_.base_current_count_;
int &VehCurrentCount=GameDataStateStorage.fields_.vehicle_current_count_;
int &SunspotDuration=GameDataStateStorage.fields_.sunspot_duration_;
int &TurnCurrentNum=GameDataStateStorage.fields_.turn_current_num_;
uint32_t &GameState=GameDataStateStorage.fields_.game_state_;
uint32_t &NetTurnFlags=GameDataStateStorage.fields_.net_turn_flags_;
Message NetMsgPrimary;
int IsMultiplayerNet,PopupDialogFactionID;
int DiploFriction,GenderDefault,PluralityDefault;
int CommlinkPending,CommlinkFaction,ConsoleControlTurnActive;
int DiploScratchFA68,DiploScratchFA70,DiploScratchFA74;
int DiploScratchFA78,DiploScratchFA98,DiploScratchFAA0,GameRules;
uint32_t DiploFrictionFactionID,DiploFrictionFactionIDWith;
int MapIsFlat,MapLongitudeBounds=8,MapLatitudeBounds=8,RadiusOffsetX[9],RadiusOffsetY[9];
int DiplomacyMapTransferChanged,tech_cost=-1,tech_calls,achieved_calls,reset_calls;
int social_phase,costs_before[2],costs_after[2],social_calc_calls,set_base_calls,compute_calls;
int parse_calls,popup_calls,log_calls,message_calls,trade_calls,commlink_result;
int withdrawal_calls,withdrawal_result,treaty_calls,double_cross_calls,territory_result,xpop_result,dont_result;
int withdrawal_args[8][2];
int bits[32][4],bit_calls,sync_calls,draw_calls,spot_calls,morale_calls;
int random_values[8],random_index,changed_topic;
int pick_faction,pick_target,pick_mode,pick_topic;
char StringTemp[1032];
static const char *last_parse;
int __cdecl pop_wait(){return 0;}
int __cdecl cost_factor(int,int kind,int){return social_phase?costs_after[kind]:costs_before[kind];}
void __cdecl social_calc(SocialCategory*,SocialEffect*,int,int,int){++social_calc_calls;social_phase=1;}
void __cdecl set_base(int){++set_base_calls;} void __cdecl base_compute(int){++compute_calls;}
int __cdecl tech_pick(int f,int m,int t,char *s){++tech_calls;pick_faction=f;pick_mode=m;pick_target=t;pick_topic=strcmp(s,"STEALTECH")==0;return tech_cost;}
void __cdecl say_tech(char *s,int,int){strcpy(s,"TECH");}
void __cdecl tech_achieved(int,int,int,int){++achieved_calls;}
void __cdecl bases_reset(int,int,int){++reset_calls;}
uint32_t __cdecl message_data(int topic,int,int,int,int,int){++message_calls;changed_topic=topic;return 0;}
int __cdecl parse_says(int,const char *s,int,int){++parse_calls;last_parse=s;return 0;}
int __cdecl X_pops(const char *s,int,Sprite*,int (__cdecl *callback)()){assert(callback==pop_wait);++popup_calls;NetMsgPrimary.label=s;return xpop_result;}
int __cdecl X_pop(const char*,int(*)()){++popup_calls;return 0;}
void __cdecl log_say(const char*,int,int,int){++log_calls;}
void __cdecl trade_maps(int,int){++trade_calls;}
int __cdecl diplomacy_caption(int,int){return 0;}
int __cdecl dont_withdrawal(int,int){return dont_result;}
int __cdecl territory(int,int,int,int*,int*){return territory_result;}
void __cdecl treaty_on(int,int,int){++treaty_calls;}
void __cdecl double_cross(int,int,int){++double_cross_calls;}
int __cdecl do_withdrawal(int f,int a){withdrawal_args[withdrawal_calls][0]=f;withdrawal_args[withdrawal_calls][1]=a;++withdrawal_calls;return withdrawal_result;}
char *__cdecl get_pact(int){return const_cast<char *>("PACT");}
short GetAsyncKeyState(int){return 0;}
int __cdecl rand_control(){int r=random_values[random_index++];return r;}
#define rand rand_control
void __cdecl bit_set(int x,int y,int mask,int value){int i=bit_calls++;bits[i][0]=x;bits[i][1]=y;bits[i][2]=mask;bits[i][3]=value;}
void __cdecl synch_bit(int,int,int){++sync_calls;}
void __cdecl draw_tile(int,int,int){++draw_calls;}
uint32_t __cdecl morale_alien(int,int){++morale_calls;return 53;}
void __cdecl spot_all(int,int){++spot_calls;}
int __cdecl steal_tech(int,int,int);
void __cdecl do_fungal_towers();
void __cdecl social_set(int);
void __cdecl propose_trade_maps(int,int);
void __cdecl demands_withdrawal(int,int);
int __cdecl commlink_attempter(int,int);
int __cdecl demand_withdrawal(int,int);
int main(){
    Players[1].noun_gender=3; Players[1].noun_faction[0]='F';
    tech_cost=9; assert(steal_tech(0,2,1)==0 && tech_calls==0);
    tech_cost=-1; assert(steal_tech(1,2,1)==1 && tech_calls==1 && achieved_calls==1);

    VehCurrentCount=1; Vehs[0].proto_id=BSC_FUNGAL_TOWER; Vehs[0].x=2;Vehs[0].y=3;
    random_values[0]=1; do_fungal_towers();
    assert(bit_calls==2 && bits[1][2]==0x0028805C && sync_calls==7 && spot_calls==1);

    BaseCurrentCount=2; PlayersData[1].soc_category_pending.value=1;
    costs_before[0]=4;costs_after[0]=8; costs_before[1]=2;costs_after[1]=2;
    Bases[0].faction_id_current=1;Bases[0].nutrients_accumulated=10;
    Bases[1].faction_id_current=2;Bases[1].nutrients_accumulated=5;
    ConsoleGlobal.faction_id_=1;social_set(1);
    assert(Bases[0].nutrients_accumulated==20 && Bases[1].nutrients_accumulated==5);
    assert(social_calc_calls==1 && TutWinBaseWindow.redraws==1);

    PlayersData[1].diplo_treaties[2]=1; DiploFriction=0; IsMultiplayerNet=1;
    propose_trade_maps(1,2);
    assert((unsigned char)PlayersData[2].diplo_patience[1]==2 && message_calls==1 && changed_topic==0x2448);

    IsMultiplayerNet=1; ConsoleGlobal.faction_id_=1;
    ConsoleFactionsWaitingMask=1u<<1; demands_withdrawal(1,2);
    assert(withdrawal_calls==0 && strcmp(NetMsgPrimary.label,"ALREADYDONE")==0);

    Players[2].title_leader[0]='T';Players[2].name_leader[0]='N';
    assert(commlink_attempter(1,2|DTREATY_VENDETTA)==1);
    assert(parse_calls==3 && strcmp(NetMsgPrimary.label,"ERADICATED")==0);

    // Direction discriminator: offer state [against][actor], response state
    // [actor][against]; native vendetta path returns before other callbacks.
    IsMultiplayerNet=0; DiploScratchFA70=1; SunspotDuration=1;
    GameRules=33; TurnCurrentNum=77;
    PlayersData[2].diplo_treaties[1]=0;
    assert(demand_withdrawal(1,2)==2 && treaty_calls==1);
    assert(PlayersData[1].diplo_spoke[2]==TurnCurrentNum);

    // The double-cross response has its own timestamp write at 0x54959B.
    IsMultiplayerNet=0; SunspotDuration=0; GameRules=33; TurnCurrentNum=77;
    DiploScratchFA70=0; dont_result=4;
    PlayersData[2].diplo_treaties[1]=0x200;
    PlayersData[1].diplo_treaties[2]=2;
    PlayersData[1].diplo_spoke[2]=0;
    assert(demand_withdrawal(1,2)==2 && double_cross_calls==1);
    assert(PlayersData[1].diplo_spoke[2]==TurnCurrentNum);

    // Native accepted territory route deliberately performs both directional
    // withdrawals; declining skips only the reversed, territory-specific one.
    DiploScratchFA70=0; SunspotDuration=0; GameRules=33; TurnCurrentNum=77;
    dont_result=0; PlayersData[1].diplo_treaties[2]=0;
    PlayersData[2].diplo_treaties[1]=0;
    territory_result=1; xpop_result=1; withdrawal_calls=0;
    assert(demand_withdrawal(1,2)==0);
    assert(withdrawal_calls==2 && withdrawal_args[0][0]==2 &&
           withdrawal_args[0][1]==1 && withdrawal_args[1][0]==1 &&
           withdrawal_args[1][1]==2);
    xpop_result=0; withdrawal_calls=0;
    assert(demand_withdrawal(1,2)==0);
    assert(withdrawal_calls==1 && withdrawal_args[0][0]==1 &&
           withdrawal_args[0][1]==2);

    // The native counter is a signed 32-bit wrapping add, not C++ signed ++.
    DiploFriction=INT_MAX; DiploFrictionFactionID=2;
    DiploFrictionFactionIDWith=1; DiploScratchFA98=1; DiploScratchFA78=10;
    territory_result=0; SunspotDuration=0;
    demand_withdrawal(1,2);
    assert(DiploFriction==INT_MIN);
    return 0;
}
'''


def compiler() -> str:
    path = shutil.which("clang++") or shutil.which("g++")
    if path is None:
        pytest.skip("host C++ compiler unavailable")
    return path


def test_actual_seven_bodies_compile_and_control(tmp_path: Path) -> None:
    bodies = "\n".join(actual_body(address) for address in OWNER_BY_ADDRESS)
    helper = actual_helper("src/faction.cpp", "static int diplo_add(int a, int b) {")
    source = HOST.replace("int main(){", helper + "\n" + bodies + "\nint main(){")
    cpp, exe = tmp_path / "actual_game_logic.cpp", tmp_path / "actual_game_logic"
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run([compiler(), "-std=c++98", "-Wall", "-Wextra", "-Werror",
                            "-fsanitize=undefined", "-fno-sanitize-recover=undefined",
                            str(cpp), "-o", str(exe)], cwd=tmp_path,
                           capture_output=True, text=True)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(exe)], cwd=tmp_path, capture_output=True, text=True)
    assert ran.returncode == 0, ran.stdout + ran.stderr


def _dont_withdrawal_program(old_waypoint: bool) -> str:
    """Original 548EE0/548EE4 read slot zero from the saved proto pointer.

    Veh is the real scalar layout. Other records/services model only consumed
    fields and calls; this host control does not claim their original ABI.
    """
    header = (ROOT / "src/veh.h").read_text()
    begin = header.index("struct Veh {")
    end = header.index("\n};", begin) + 3
    veh = header[begin:end]
    order, = re.findall(r"\bORDER_MOVE_TO\s*=\s*(\d+)", header)
    body = _definition((ROOT / "src/faction.cpp").read_text(),
                       "int __cdecl dont_withdrawal(")
    if old_waypoint:
        for axis in ("x", "y"):
            old = "veh.waypoint_" + axis + "[0]"
            assert body.count(old) == 1
            body = body.replace(old, "veh.waypoint_" + axis + "[1]", 1)
    return r'''
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>
#define __cdecl
ACTUAL_VEH
static_assert(sizeof(Veh) == 0x34, "real vehicle stride");
static_assert(offsetof(Veh, proto_id) == 0xA, "original saved receiver");
static_assert(offsetof(Veh, waypoint_x) == 0x14, "original first x");
static_assert(offsetof(Veh, waypoint_y) == 0x1C, "original first y");
const int ORDER_MOVE_TO = ACTUAL_ORDER;
Veh Vehs[1];
struct PrototypeService { uint8_t plan; } VehPrototypes[1];
struct PlayerService { uint32_t diplo_treaties[8]; } PlayersData[8];
struct BaseService { int16_t x, y; uint8_t faction_id_current; } Bases[3];
struct MapService { uint8_t climate; } tiles[64];
typedef MapService Map;
struct GameFields { int vehicle_current_count_; };
struct GameState { GameFields fields_; } GameDataStateStorage;
int MapLongitude = 8;
int PopupDialogFactionID;
int DiplomacyProposalGateCount = 1;
bool slot_zero_absent;
int observed_x, observed_y, base_at_calls;
std::vector<int> abs_arguments;
Map *map_tiles() { return tiles; }
int base_find(int x, int y, int faction) {
    assert(x == 2 && y == 2 && faction == 1);
    return 0;
}
int whose_territory(int faction, int x, int y, int a, int b) {
    assert(faction == 1 && x == 2 && y == 2 && a == 0 && b == 0);
    return 2;
}
int base_at(int x, int y) {
    observed_x = x; observed_y = y; ++base_at_calls;
    if (x == 14 && y == 8) return slot_zero_absent ? -1 : 1;
    if (x == 20 && y == 10) return 2;
    assert(false && "unexpected waypoint coordinates");
    return -1;
}
int fixture_abs(int value) {
    abs_arguments.push_back(value);
    return value < 0 ? -value : value;
}
#define abs fixture_abs
ACTUAL_BODY
#undef abs
void reset(bool missing) {
    std::memset(Vehs, 0, sizeof(Vehs));
    std::memset(PlayersData, 0, sizeof(PlayersData));
    Vehs[0].x = 2; Vehs[0].y = 2;
    Vehs[0].faction_id = 2;
    Vehs[0].order = ORDER_MOVE_TO;
    Vehs[0].waypoint_x[0] = 14; Vehs[0].waypoint_y[0] = 8;
    Vehs[0].waypoint_x[1] = 20; Vehs[0].waypoint_y[1] = 10;
    Bases[0].x = 4; Bases[0].y = 4;
    Bases[1].faction_id_current = 3;
    Bases[2].faction_id_current = 4;
    PlayersData[1].diplo_treaties[2] = 0x800000;
    PlayersData[2].diplo_treaties[3] = 0x10;
    PlayersData[2].diplo_treaties[4] = 0x10;
    PlayersData[1].diplo_treaties[3] = 1;
    PlayersData[1].diplo_treaties[4] = 1;
    for (int i = 0; i < 64; ++i) tiles[i].climate = 0x60;
    GameDataStateStorage.fields_.vehicle_current_count_ = 1;
    slot_zero_absent = missing;
    observed_x = observed_y = -1; base_at_calls = 0;
    abs_arguments.clear();
}
int main() {
    reset(false);
    int result = dont_withdrawal(1, 2);
    assert(observed_x == 14 && observed_y == 8 && base_at_calls == 1);
    assert(result == 3 && PopupDialogFactionID == 2);
    assert(abs_arguments.size() == 2 && abs_arguments[0] == 2 && abs_arguments[1] == 2);
    reset(true);
    result = dont_withdrawal(1, 2);
    assert(observed_x == 14 && observed_y == 8 && base_at_calls == 1);
    assert(result == 0); // Original failed-base lookup skips the vehicle.
}
'''.replace("ACTUAL_VEH", veh).replace("ACTUAL_ORDER", order).replace("ACTUAL_BODY", body)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("old_waypoint", [False, True])
def test_actual_dont_withdrawal_reads_original_first_waypoint(
        tmp_path: Path, compiler_name: str, old_waypoint: bool) -> None:
    host_compiler = shutil.which(compiler_name)
    if host_compiler is None:
        pytest.skip(compiler_name + " unavailable")
    source = tmp_path / "actual_dont_withdrawal.cpp"
    executable = tmp_path / "actual_dont_withdrawal"
    source.write_text(_dont_withdrawal_program(old_waypoint))
    built = subprocess.run(
        [host_compiler, "-std=c++11", "-O2", "-Wall", "-Wextra", "-Werror",
         str(source), "-o", str(executable)], capture_output=True, text=True)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(executable)], cwd=tmp_path, capture_output=True, text=True)
    assert (ran.returncode == 0) == (not old_waypoint), ran.stdout + ran.stderr
