"""Bounded actual-source controls for the coupled vehicle action cohort.

Exercise transfer outcomes, tectonic/fungal sequencing and mutation-sensitive
vehicle-slot reads, plus the translated portions of orbital interception,
prototype naming and proposal state. Partial dependencies remain partial.
Host doubles do not validate the native ABI, full game UI or runtime behavior.
"""

from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def test_partial_dependencies_remain_unfinished_in_native_source_reader():
    from match_recovery.source import read_file

    partial = {0x57E040, 0x580860, 0x57F4B0}
    records = {r.address: r for r in read_file(ROOT / "src/veh.cpp")}
    assert partial <= records.keys()
    assert all(records[address].state.name == "PLACEHOLDER" for address in partial)
    assert records[0x4CDAF0].state.name == "IMPLEMENTED"
    assert records[0x57F1D0].state.name == "IMPLEMENTED"

def retire_harness(function):
    return r"""
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#define __cdecl
const int MaxVehProtoFactionNum=64, MaxVehProtoNum=512;
const uint16_t PROTO_ACTIVE=1;
const uint32_t BIT_BASE_IN_TILE=1;
struct VehPrototype { int8_t unk_1; uint8_t obsolete_factions; uint16_t flags; uint8_t cost; };
VehPrototype VehPrototypes[MaxVehProtoNum];
struct Veh { int16_t x,y; int16_t proto_id; uint8_t faction_id; uint8_t visibility; };
Veh Vehs[16]; int VehCurrentCount;
struct Map { uint8_t val2; uint32_t bit; } TestMap;
Map *map_loc(int,int) { return &TestMap; }
uint32_t LockEnableMask;
struct PlayerData { uint8_t proto_id_active[MaxVehProtoNum]; };
PlayerData PlayersData[8];
struct Base {
    int minerals_accumulated, minerals_accumulated_2, production_id_last;
    uint32_t queue_size; int queue_production_id[10];
} Bases[4];
int BaseCurrentCount;
int base_result, stack_result, killed_count, base_at_calls, stack_check_calls;
int rewritten_proto, shrink_count_after_stack, lock_mask_after_stack;
bool rewrite_proto_after_stack;
void proto_bung_put(int,int) {}
int base_at(int,int) { ++base_at_calls; return base_result; }
int stack_check(int vehicle_id,int,int,int,int) {
    ++stack_check_calls;
    if (rewrite_proto_after_stack) Vehs[vehicle_id].proto_id=rewritten_proto;
    if (shrink_count_after_stack >= 0) {
        VehCurrentCount=shrink_count_after_stack;
        shrink_count_after_stack=-1;
    }
    if (lock_mask_after_stack >= 0) {
        LockEnableMask=lock_mask_after_stack;
        lock_mask_after_stack=-1;
    }
    return stack_result;
}
void kill(int vehicle_id) {
    for (int i=vehicle_id; i<VehCurrentCount-1; ++i) Vehs[i]=Vehs[i+1];
    if (vehicle_id >= 0 && vehicle_id < VehCurrentCount) --VehCurrentCount;
    ++killed_count;
}
void check(bool ok,int n) {
    if (!ok) { std::fprintf(stderr,"retire host check %d failed\n",n); std::exit(42); }
}

LIVE_RETIRE_BODY

void reset() {
    std::memset(VehPrototypes,0,sizeof(VehPrototypes));
    std::memset(Vehs,0,sizeof(Vehs));
    std::memset(PlayersData,0,sizeof(PlayersData));
    std::memset(Bases,0,sizeof(Bases));
    std::memset(&TestMap,0,sizeof(TestMap));
    VehCurrentCount=BaseCurrentCount=0; base_result=-1;
    stack_result=killed_count=base_at_calls=stack_check_calls=0;
    rewritten_proto=0; shrink_count_after_stack=lock_mask_after_stack=-1;
    rewrite_proto_after_stack=false;
    LockEnableMask=0;
}
int main() {
    reset();
    VehCurrentCount=1; BaseCurrentCount=1; base_result=0; stack_result=1;
    VehPrototypes[70].unk_1=4; VehPrototypes[70].flags=PROTO_ACTIVE;
    Vehs[0].proto_id=70; Vehs[0].faction_id=2; Vehs[0].visibility=4;
    PlayersData[2].proto_id_active[70]=1;
    Bases[0].queue_size=2;
    Bases[0].queue_production_id[0]=11;
    Bases[0].queue_production_id[1]=70;
    Bases[0].queue_production_id[2]=22;
    retire_proto(70,2);
    check(Vehs[0].proto_id==2,1);
    check(PlayersData[2].proto_id_active[70]==0
          && PlayersData[2].proto_id_active[2]==1,2);
    check(Bases[0].queue_size==1 && Bases[0].queue_production_id[1]==22,3);
    check((VehPrototypes[70].flags & PROTO_ACTIVE)==0
          && (VehPrototypes[70].obsolete_factions & 4)==0,4);
    check(killed_count==0,5);

    reset();
    VehCurrentCount=1; BaseCurrentCount=1; base_result=0;
    VehPrototypes[70].flags=PROTO_ACTIVE; VehPrototypes[70].cost=9;
    Vehs[0].proto_id=70; Vehs[0].faction_id=2; Vehs[0].visibility=4;
    Bases[0].minerals_accumulated=10; Bases[0].minerals_accumulated_2=3;
    Bases[0].queue_production_id[0]=33;
    retire_proto(70,2);
    check(killed_count==1,6);
    check(Bases[0].minerals_accumulated==14
          && Bases[0].minerals_accumulated_2==14,7);
    check(Bases[0].production_id_last==33,8);

    reset();
    VehCurrentCount=1;
    VehPrototypes[70].flags=PROTO_ACTIVE;
    Vehs[0].proto_id=70; Vehs[0].faction_id=2; Vehs[0].visibility=0;
    retire_proto(70,2);
    check(killed_count==1 && Vehs[0].proto_id==70,9);

    reset();
    VehCurrentCount=1; VehPrototypes[70].unk_1=5;
    VehPrototypes[130].unk_1=5; VehPrototypes[130].flags=PROTO_ACTIVE;
    Vehs[0].proto_id=70; Vehs[0].faction_id=2; Vehs[0].visibility=2;
    Vehs[0].x=4; Vehs[0].y=6; TestMap.val2=1; TestMap.bit=1;
    LockEnableMask=2;
    PlayersData[2].proto_id_active[70]=1;
    retire_proto(70,2);
    check(Vehs[0].proto_id==130,10);
    check(PlayersData[2].proto_id_active[70]==0
          && PlayersData[2].proto_id_active[130]==1,11);

    reset();
    VehCurrentCount=1; BaseCurrentCount=1; base_result=0; stack_result=1;
    VehPrototypes[70].unk_1=5; VehPrototypes[130].unk_1=5;
    VehPrototypes[130].flags=PROTO_ACTIVE;
    Vehs[0].proto_id=70; Vehs[0].faction_id=2; Vehs[0].visibility=2;
    TestMap.val2=1; TestMap.bit=0; LockEnableMask=2;
    PlayersData[2].proto_id_active[70]=1;
    retire_proto(70,2);
    check(Vehs[0].proto_id==2 && Vehs[0].proto_id!=130,14);
    check(base_at_calls==1 && stack_check_calls==1,15);

    reset();
    VehCurrentCount=1; BaseCurrentCount=1; base_result=0;
    VehPrototypes[70].flags=PROTO_ACTIVE;
    Vehs[0].proto_id=70; Vehs[0].faction_id=2; Vehs[0].visibility=4;
    LockEnableMask=4;
    Bases[0].minerals_accumulated=10;
    retire_proto(70,2);
    check(killed_count==1 && Bases[0].minerals_accumulated==10,12);

    reset();
    VehCurrentCount=2;
    VehPrototypes[70].flags=PROTO_ACTIVE;
    Vehs[0].proto_id=Vehs[1].proto_id=70;
    Vehs[0].faction_id=Vehs[1].faction_id=2;
    retire_proto(70,2);
    check(killed_count==2 && VehCurrentCount==0,13);

    reset();
    VehCurrentCount=1; BaseCurrentCount=1; base_result=0; stack_result=1;
    VehPrototypes[70].unk_1=5; VehPrototypes[130].unk_1=5;
    VehPrototypes[130].flags=PROTO_ACTIVE;
    Vehs[0].proto_id=70; Vehs[0].faction_id=2; Vehs[0].visibility=2;
    TestMap.val2=1; TestMap.bit=0; LockEnableMask=2;
    retire_proto(70,2);
    check(Vehs[0].proto_id==2 && base_at_calls==1,16);

    reset();
    VehCurrentCount=1; BaseCurrentCount=1; base_result=0; stack_result=0;
    VehPrototypes[70].flags=PROTO_ACTIVE; VehPrototypes[70].cost=8;
    VehPrototypes[71].cost=15;
    Vehs[0].proto_id=70; Vehs[0].faction_id=2;
    rewrite_proto_after_stack=true; rewritten_proto=71;
    retire_proto(70,2);
    check(Bases[0].minerals_accumulated==7,17);

    reset();
    VehCurrentCount=2; BaseCurrentCount=1; base_result=0; stack_result=0;
    VehPrototypes[70].flags=PROTO_ACTIVE;
    Vehs[0].proto_id=Vehs[1].proto_id=70;
    Vehs[0].faction_id=Vehs[1].faction_id=2;
    lock_mask_after_stack=4;
    retire_proto(70,2);
    check(stack_check_calls==1 && killed_count==2,19);

    reset();
    VehCurrentCount=3; BaseCurrentCount=1; base_result=0; stack_result=0;
    VehPrototypes[70].flags=PROTO_ACTIVE;
    for (int i=0; i<3; ++i) {
        Vehs[i].proto_id=70; Vehs[i].faction_id=2;
    }
    shrink_count_after_stack=1;
    retire_proto(70,2);
    check(stack_check_calls==2 && VehCurrentCount==0,18);
}
""".replace("LIVE_RETIRE_BODY", function)

def test_live_retire_proto_replacement_fallback_and_queue_paths(tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host g++ required")
    cpp = tmp_path / "retire_proto.cpp"
    executable = tmp_path / "retire_proto"
    cpp.write_text(retire_harness(actual_function("retire_proto")))
    result = subprocess.run(
        [compiler, "-std=c++11", "-O1", "-fsanitize=undefined",
         "-fno-sanitize-recover=undefined", str(cpp), "-o", str(executable)],
        capture_output=True, text=True, timeout=30)
    assert result.returncode == 0, result.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
    assert result.returncode == 0, result.stderr


def braced(source, start):
    brace = source.index("{", start)
    depth = 0
    for end in range(brace, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated source declaration")


def actual_function(name):
    source = (ROOT / "src/veh.cpp").read_text()
    source = re.sub(r"/\*.*?\*/|//[^\n]*", "", source, flags=re.S)
    match = re.search(r"(?:int|void)\s+(?:__cdecl\s+)?" + name + r"\([^;{]*\)\s*\{", source)
    assert match, f"missing live {name} definition"
    brace = source.index("{", match.start())
    depth = 0
    for end in range(brace, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[match.start():end + 1]
    raise AssertionError(f"unterminated live {name} definition")


def harness(function):
    return r"""
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#define __cdecl
int scripted_random[8];
int scripted_count, scripted_index;
bool mutate_inventory_on_first_base, mutate_deployed_on_first_random;
int base_at_call_count;
int test_rand();
#define rand test_rand
const int MaxPlayerNum = 8;
const int FAC_FLECHETTE_DEFENSE_SYS = 39;
const int RadiusRange[3] = {1, 9, 25};
int RadiusOffsetX[25], RadiusOffsetY[25];
int MapLongitudeBounds = 20, MapLatitudeBounds = 20;
int xrange(int x) { x %= MapLongitudeBounds; return x < 0 ? x + MapLongitudeBounds : x; }
bool on_map(int x, int y) {
    return x >= 0 && x < MapLongitudeBounds && y >= 0 && y < MapLatitudeBounds;
}
struct PlayerData { uint32_t satellites_[4]; uint32_t sat_odp_deployed; };
PlayerData PlayersData[MaxPlayerNum];
int test_rand() {
    if (scripted_index >= scripted_count) std::exit(91);
    const int value = scripted_random[scripted_index++];
    if (mutate_deployed_on_first_random && scripted_index == 1) {
        PlayersData[2].sat_odp_deployed = 6;
    }
    return value;
}
struct Base { int x, y; uint8_t faction_id_current; uint8_t facilities_built[12]; char name_string[25]; };
Base Bases[4];
int BaseCount;
int base_at(int x, int y) {
    ++base_at_call_count;
    if (mutate_inventory_on_first_base && base_at_call_count == 1) {
        PlayersData[2].satellites_[3] = 2;
        PlayersData[2].sat_odp_deployed = 0;
    }
    for (int i = 0; i < BaseCount; ++i)
        if (Bases[i].x == x && Bases[i].y == y) return i;
    return -1;
}
bool has_fac_built(int facility, int base_id) {
    return (Bases[base_id].facilities_built[facility / 8] & (1u << (facility & 7))) != 0;
}
int base_find(int, int) { return 0; }
int base_find(int, int, int) { return 0; }
struct Player {
    int noun_gender, is_noun_plural; char noun_faction[24];
    char adj_name_faction[128];
} Players[8];
int GenderDefault, PluralityDefault;
int ConsoleFaction=2, IsMultiplayerNet, LockEnableMask;
struct Console { int faction_id_; } ConsoleGlobal;
char PopupTitle[]="EVENT"; char *PopupTitlePointer=PopupTitle;
char **ClimatePopupTitle=&PopupTitlePointer;
int parse_count, parse_num_count, popup_count, popup_result;
int parse_slots[16], parsed_numbers[8]; const char *parse_text[16], *popup_label;
bool mutate_inventory_callbacks, zero_inventory_on_sacrifice;
int base_name_parse_count, observed_pods_before_summary, observed_attempts_before_summary;
int parse_says(int slot,const char *text,int,int) {
    parse_slots[parse_count]=slot; parse_text[parse_count++]=text;
    if (mutate_inventory_callbacks && slot==0 && text==Bases[0].name_string) {
        ++base_name_parse_count;
        if (base_name_parse_count==2) {
            observed_pods_before_summary=PlayersData[2].satellites_[3];
            observed_attempts_before_summary=PlayersData[2].sat_odp_deployed;
            PlayersData[2].satellites_[3]=7;
            PlayersData[2].sat_odp_deployed=3;
        }
    }
    return 0;
}
int parse_num(int slot,int value) {
    parsed_numbers[parse_num_count++]=value; return slot;
}
int popp(const char *,const char *label,int,const char *,int(__cdecl *)()) {
    ++popup_count; popup_label=label;
    if (mutate_inventory_callbacks && std::strcmp(label,"TECTONICSAC")==0) {
        PlayersData[2].satellites_[3]=2;
        PlayersData[2].sat_odp_deployed=5;
    }
    if (zero_inventory_on_sacrifice
        && std::strcmp(label,"TECTONICSAC")==0) {
        PlayersData[2].satellites_[3]=0;
        PlayersData[2].sat_odp_deployed=0;
    }
    return popup_result;
}
int rand();
int check_number;
void check(bool condition) {
    ++check_number;
    if (!condition) { std::fprintf(stderr, "host check %d failed\n", check_number); std::exit(42); }
}

LIVE_BODY

void reset() {
    std::memset(PlayersData, 0, sizeof(PlayersData));
    std::memset(Bases, 0, sizeof(Bases));
    std::memset(Players, 0, sizeof(Players));
    std::memset(RadiusOffsetX, 0, sizeof(RadiusOffsetX));
    std::memset(RadiusOffsetY, 0, sizeof(RadiusOffsetY));
    RadiusOffsetX[0] = 1;
    BaseCount = scripted_count = scripted_index = 0;
    mutate_inventory_on_first_base=mutate_deployed_on_first_random=false;
    base_at_call_count=0;
    ConsoleGlobal.faction_id_=ConsoleFaction; IsMultiplayerNet=0;
    LockEnableMask=1u<<ConsoleFaction; popup_result=0;
    parse_count=parse_num_count=popup_count=0; popup_label=nullptr;
    mutate_inventory_callbacks=zero_inventory_on_sacrifice=false;
    base_name_parse_count=0;
    observed_pods_before_summary=observed_attempts_before_summary=-1;
}
void push_random(int value) { scripted_random[scripted_count++] = value; }
int main() {
    // No orbital inventory and no defense base: the body returns before RNG.
    reset();
    check(shoot_it(1, 2, 5, 5, 0) == 0);
    check(scripted_index == 0);

    // A successful orbital interception spends one attempt; the native
    // completion branch leaves the pod count untouched.
    reset();
    PlayersData[2].satellites_[3] = 2;
    push_random(75);
    check(shoot_it(1, 2, 5, 5, 0) == 1);
    check(PlayersData[2].sat_odp_deployed == 1);
    check(PlayersData[2].satellites_[3] == 2);
    check(std::strcmp(popup_label,"TWESHOTIT")==0 && parsed_numbers[0]==2);
    check(parse_num_count==2 && parsed_numbers[1]==1 && popup_count==1);

    // The first orbital inventory reads happen after the full worked-square
    // scan, and each attempt increments the live counter after RNG callbacks.
    reset();
    mutate_inventory_on_first_base=true;
    mutate_deployed_on_first_random=true;
    push_random(20); push_random(75);
    check(shoot_it(1,2,5,5,0)==1);
    check(base_at_call_count==25 && PlayersData[2].sat_odp_deployed==7);

    // A Flechette system can intercept without orbital inventory; a failed
    // facility roll consumes neither the faction counter nor a pod.
    reset();
    BaseCount = 1;
    Bases[0].x = 6; Bases[0].y = 5; Bases[0].faction_id_current = 2;
    Bases[0].facilities_built[FAC_FLECHETTE_DEFENSE_SYS / 8] |=
        1u << (FAC_FLECHETTE_DEFENSE_SYS & 7);
    push_random(50);
    check(shoot_it(1, 2, 5, 5, 1) == 0);
    check(scripted_index == 1);
    check(PlayersData[2].satellites_[3] == 0);
    check(PlayersData[2].sat_odp_deployed == 0);
    check(std::strcmp(popup_label,"WEFAILEDFLECHETTE")==0);

    // Even without an initial Flechette facility, an unsuccessful orbital
    // roll performs the full second neighboring-base scan.
    reset(); PlayersData[2].satellites_[3]=1; push_random(20);
    check(shoot_it(1,2,5,5,0)==1);
    check(base_at_call_count==50);

    reset();
    BaseCount = 1;
    Bases[0].x = 6; Bases[0].y = 5; Bases[0].faction_id_current = 2;
    Bases[0].facilities_built[FAC_FLECHETTE_DEFENSE_SYS / 8] |=
        1u << (FAC_FLECHETTE_DEFENSE_SYS & 7);
    push_random(49);
    check(shoot_it(1, 2, 5, 5, 1) == 1);
    check(scripted_index == 1);
    check(std::strcmp(popup_label,"FWESHOTIT2")==0);
    check(base_at_call_count==2); // reuse the hit base; no third lookup

    // A non-intercepting orbital roll still spends the pod; attempts are
    // clamped to the remaining inventory as it reaches zero.
    reset();
    PlayersData[2].satellites_[3] = 1;
    push_random(20);
    check(shoot_it(1, 2, 5, 5, 0) == 1);
    check(PlayersData[2].satellites_[3] == 0);
    check(PlayersData[2].sat_odp_deployed == 0);
    check(std::strcmp(popup_label,"TWESHOTIT")==0);
    check(popup_count==2 && parse_num_count==3 && parsed_numbers[2]==0);

    // Dismissing the typed sacrifice prompt takes the native early-return
    // path before decrementing the defender's satellite count.
    reset();
    PlayersData[2].satellites_[3]=1; popup_result=1;
    push_random(20);
    check(shoot_it(1,2,5,5,0)==0);
    check(PlayersData[2].satellites_[3]==1);
    check(PlayersData[2].sat_odp_deployed==1);
    check(popup_count==1 && std::strcmp(popup_label,"TECTONICSAC")==0);

    // Both the post-popup decrement/clamp and post-base-parser count read
    // observe callback mutations to the live faction records.
    reset();
    PlayersData[2].satellites_[3]=1; push_random(20);
    mutate_inventory_callbacks=true;
    check(shoot_it(1,2,5,5,0)==1);
    check(observed_pods_before_summary==1 && observed_attempts_before_summary==1);
    check(parsed_numbers[1]==7 && PlayersData[2].satellites_[3]==7);

    // The native failed-interception path decrements the live inventory
    // unconditionally after the popup, even if its callback zeroed it.
    reset(); PlayersData[2].satellites_[3]=1; push_random(20);
    zero_inventory_on_sacrifice=true;
    check(shoot_it(1,2,5,5,0)==1);
    check(PlayersData[2].satellites_[3]==0xffffffffu);
    check(PlayersData[2].sat_odp_deployed==0xffffffffu);

    // The native THEYMUSTSAC parser uses the noun field, while the later
    // combat-result sentence uses the distinct adj_name_faction field.
    reset();
    std::strcpy(Players[1].noun_faction,"noun-field");
    std::strcpy(Players[1].adj_name_faction,"adjective-field");
    IsMultiplayerNet=1; LockEnableMask=0;
    PlayersData[1].satellites_[3]=1; push_random(20);
    check(shoot_it(2,1,5,5,0)==1);
    check(parse_text[0]==Players[1].noun_faction);
    check(parse_text[2]==Players[1].adj_name_faction);

    // The fungal variant selects its own sacrifice phrase before the shared
    // completion phrase, while retaining the same typed counter updates.
    reset();
    PlayersData[2].satellites_[3]=1;
    push_random(20);
    check(shoot_it(1,2,5,5,1)==1);
    check(PlayersData[2].satellites_[3]==0);
    check(popup_count==2 && std::strcmp(popup_label,"FWESHOTIT")==0);
}
""".replace("LIVE_BODY", function)


def test_live_shoot_it_orbital_and_flechette_paths(tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host g++ required")
    cpp = tmp_path / "vehicle_special_actions.cpp"
    executable = tmp_path / "vehicle_special_actions"
    cpp.write_text(harness(actual_function("shoot_it")))
    result = subprocess.run(
        [compiler, "-std=c++11", "-O1", "-fsanitize=undefined",
         "-fno-sanitize-recover=undefined", str(cpp), "-o", str(executable)],
        capture_output=True, text=True, timeout=30)
    assert result.returncode == 0, result.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
    assert result.returncode == 0, result.stderr


def give_harness(function):
    veh_header = (ROOT / "src/veh.h").read_text()
    veh_type = braced(veh_header, veh_header.index("struct Veh {")) + ";"
    prototype_type = braced(veh_header, veh_header.index("struct VehPrototype {")) + ";"
    chassis_type = braced(veh_header, veh_header.index("struct RulesChassis {")) + ";"
    return r"""
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#define __cdecl
const int MaxVehProtoFactionNum = 64;
const int ORDER_NONE = 0, ORDER_SENTRY_BOARD = 1;
const int TRIAD_SEA = 1, BSC_TRANSPORT_FOIL = 3, BSC_SCOUT_PATROL = 2;
typedef char *LPSTR;
typedef int BOOL;
SOURCE_TYPES
struct Map { uint32_t pad[2]; uint32_t bit; uint16_t val2; uint8_t region; };
struct Base { char name_string[25]; } Bases[4];
Veh Vehs[32]; VehPrototype VehPrototypes[512]; RulesChassis Chassis[16]; Map Tile;
struct Player {
    int is_leader_female; char title_leader[24], name_leader[24];
    int noun_gender, is_noun_plural; char noun_faction[24];
};
Player Players[8]; int GenderDefault; int PluralityDefault;
struct PlayerData { uint8_t unk_83[128]; } PlayersData[8];
uint32_t LockEnableMask, Language;
int popup_count;
const char *last_popup;
struct NetMsg { int pop(const char *label, int, int, const char *) {
    ++popup_count; last_popup=label; return 0;
} };
NetMsg Message; NetMsg *NetMsgPopup = &Message;
const char *last_xpop; int xpop_count;
int X_pop(const char *label, int (__cdecl *)()) {
    ++xpop_count; last_xpop=label; return 0;
}
int base_result, base_x, base_y, base_faction, base3_calls, base2_calls;
int owner_calls, owner_x, owner_y, owner_faction;
int radius_calls, radius_x, radius_y, focus_calls, focus_x, focus_y, focus_faction;
int parse_count, event_count, parse_slots[32], focus_order;
int propose_calls, proposed_faction, proposed_chassis, proposed_weapon, proposed_armor,
    proposed_abilities, proposed_reactor, proposed_category;
Map *map_loc(int x, int y) { (void)x; (void)y; return &Tile; }
int base_find(int x, int y, int faction) {
    ++base3_calls; base_x=x; base_y=y; base_faction=faction; return base_result;
}
int base_find(int x, int y) { ++base2_calls; base_x=x; base_y=y; return base_result; }
void owner_set(int x, int y, int faction) {
    ++owner_calls; owner_x=x; owner_y=y; owner_faction=faction;
}
void draw_radius(int x, int y, int, int) { ++radius_calls; radius_x=x; radius_y=y; }
struct Console { int faction_id_; void focus(int x,int y,int faction) {
    ++focus_calls; focus_order=++event_count; focus_x=x; focus_y=y; focus_faction=faction;
} } ConsoleGlobal;
int parse_says(int slot, const char *, int, int) {
    parse_slots[parse_count++] = slot; ++event_count; return 0;
}
int __cdecl propose_proto(int faction, int chassis, int weapon, int armor,
                          int abilities, int reactor, int category, char *) {
    ++propose_calls; proposed_faction=faction; proposed_chassis=chassis;
    proposed_weapon=weapon; proposed_armor=armor; proposed_abilities=abilities;
    proposed_reactor=reactor; proposed_category=category; return -1;
}
int give_check_number;
void check(bool condition) {
    ++give_check_number;
    if (!condition) { std::fprintf(stderr, "give host check %d failed\n", give_check_number); std::exit(42); }
}

LIVE_BODY

int main() {
    std::memset(Vehs, 0, sizeof(Vehs));
    std::memset(VehPrototypes, 0, sizeof(VehPrototypes));
    std::memset(Players, 0, sizeof(Players));
    Vehs[4].x=7; Vehs[4].y=9; Vehs[4].faction_id=1; Vehs[4].proto_id=120;
    Vehs[4].prev_veh_id_stack=-1; Vehs[4].next_veh_id_stack=5;
    Vehs[4].state=0xffffffffu; Vehs[4].home_base_id=-1;
    Vehs[5].x=7; Vehs[5].y=9; Vehs[5].faction_id=1; Vehs[5].proto_id=7;
    Vehs[5].prev_veh_id_stack=4; Vehs[5].next_veh_id_stack=6;
    Vehs[5].order=ORDER_SENTRY_BOARD; Vehs[5].waypoint_x[0]=4;
    Vehs[5].state=0xffffffffu;
    Vehs[6].x=7; Vehs[6].y=9; Vehs[6].faction_id=1; Vehs[6].proto_id=122;
    Vehs[6].prev_veh_id_stack=5; Vehs[6].next_veh_id_stack=-1;
    Vehs[6].state=0xffffffffu;
    VehPrototype &source=VehPrototypes[120];
    source.ability_flags=0x12345678u; source.chassis_id=2; source.weapon_id=3;
    source.armor_id=4; source.reactor_id=2; source.unk_1=7;
    VehPrototype &match=VehPrototypes[130];
    match.obsolete_factions=1; match.ability_flags=source.ability_flags;
    match.chassis_id=source.chassis_id; match.weapon_id=source.weapon_id;
    match.armor_id=source.armor_id; match.reactor_id=source.reactor_id;
    Tile.bit=0; Tile.val2=0; base_result=3; ConsoleGlobal.faction_id_=2;
    LockEnableMask=1u<<1;
    action_give(4,2);
    check(Vehs[4].faction_id==2 && Vehs[4].proto_id==130 && Vehs[4].order==ORDER_NONE);
    check(Vehs[5].faction_id==2 && Vehs[5].proto_id==7 && Vehs[5].order==ORDER_NONE);
    check(Vehs[6].faction_id==1 && Vehs[6].proto_id==122);
    check(Vehs[4].state==0xfcffbdffu && Vehs[5].state==0xfcffbdffu);
    check(Vehs[4].home_base_id==3 && Vehs[5].home_base_id==3);
    check(base_x==7 && base_y==9 && base_faction==2 && base3_calls==2 && base2_calls==1);
    check(owner_calls==1 && owner_x==7 && owner_y==9 && owner_faction==2);
    check(radius_calls==1 && radius_x==7 && radius_y==9);
    check(focus_calls==1 && focus_x==7 && focus_y==9 && focus_faction==1);
    check(parse_count==5 && focus_order==5 && event_count==6);
    check(popup_count==1 && std::strcmp(last_popup,"GOTUNIT")==0);
    check(parse_slots[0]==0 && parse_slots[1]==1 && parse_slots[2]==2
          && parse_slots[3]==3 && parse_slots[4]==4);
    check(propose_calls==0);

    Vehs[11].x=3; Vehs[11].y=4; Vehs[11].faction_id=1; Vehs[11].proto_id=7;
    Vehs[11].prev_veh_id_stack=-1; Vehs[11].next_veh_id_stack=-1;
    Tile.region=5; PlayersData[2].unk_83[5]=0; VehPrototypes[7].plan=0;
    LockEnableMask=0; Language=0;
    action_give(11,2);
    check(xpop_count==1 && std::strcmp(last_xpop,"NITOFFEND")==0);
    Vehs[12].x=3; Vehs[12].y=4; Vehs[12].faction_id=1; Vehs[12].proto_id=7;
    Vehs[12].prev_veh_id_stack=-1; Vehs[12].next_veh_id_stack=-1;
    PlayersData[2].unk_83[5]=2;
    action_give(12,2);
    check(xpop_count==2 && std::strcmp(last_xpop,"NITDEFEND")==0);

    // When the human is the original owner, the recipient's faction parser
    // state precedes the exact original GAVEUNIT popup key.
    Vehs[10].x=2; Vehs[10].y=3; Vehs[10].faction_id=2; Vehs[10].proto_id=7;
    Vehs[10].prev_veh_id_stack=-1; Vehs[10].next_veh_id_stack=-1;
    base_result=-1; Language=0; LockEnableMask=0;
    action_give(10,1);
    check(popup_count==2 && std::strcmp(last_popup,"GAVEUNIT")==0);
}
""".replace("SOURCE_TYPES", "\n".join((veh_type, prototype_type, chassis_type))).replace(
    "LIVE_BODY", function)


def test_live_action_give_transfers_root_and_embarked_units(tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host g++ required")
    cpp = tmp_path / "vehicle_give.cpp"
    executable = tmp_path / "vehicle_give"
    cpp.write_text(give_harness(actual_function("action_give")))
    result = subprocess.run(
        [compiler, "-std=c++11", "-O1", "-fsanitize=undefined",
         "-fno-sanitize-recover=undefined", str(cpp), "-o", str(executable)],
        capture_output=True, text=True, timeout=30)
    assert result.returncode == 0, result.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
    assert result.returncode == 0, result.stderr


def test_live_name_proto_composes_rule_strings(tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host g++ required")
    source_body = actual_function("name_proto")
    harness = r"""
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#define __cdecl
const int MaxAbilityNum=29;
const int TRIAD_LAND=0, TRIAD_SEA=2;
struct RulesAbility { char *abbreviation; } Ability[29];
struct RulesChassis { char *offsv_1_name; uint8_t triad; } Chassis[19];
struct RulesWeapon { char *name_short; int8_t offense_rating; } Weapon[26];
struct RulesArmor { char *name_short; int8_t defense_rating; } Armor[14];
struct RulesReactor { char *name_short; } Reactor[4];
struct StringTableStub { const char *get(int index) {
    switch(index) {
      case 1: return "Cloak"; case 2: return "Deep Radar";
      case 3: return "Scout"; case 4: return "Impact"; case 5: return "Fission";
      default: return "";
    }
} } StringTable;
LIVE_BODY
int main() {
    Ability[0].abbreviation=reinterpret_cast<char *>(1);
    Ability[3].abbreviation=reinterpret_cast<char *>(2);
    Chassis[0].offsv_1_name=reinterpret_cast<char *>(3);
    Weapon[0].name_short=reinterpret_cast<char *>(4);
    Armor[0].name_short=reinterpret_cast<char *>(4);
    Reactor[0].name_short=reinterpret_cast<char *>(5);
    char name[256];
    name_proto(name, 0, 1, 0, 0, 0, (1<<0)|(1<<3), 0);
    if (std::strcmp(name,"Cloak ") != 0) {
      std::fprintf(stderr,"unexpected name: %s\n",name); return 42;
    }
}
""".replace("LIVE_BODY", source_body)
    cpp = tmp_path / "vehicle_name_proto.cpp"
    executable = tmp_path / "vehicle_name_proto"
    cpp.write_text(harness)
    result = subprocess.run(
        [compiler, "-std=c++11", "-O1", "-fsanitize=undefined",
         "-fno-sanitize-recover=undefined", str(cpp), "-o", str(executable)],
        capture_output=True, text=True, timeout=30)
    assert result.returncode == 0, result.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
    assert result.returncode == 0, result.stderr


def test_live_propose_proto_keeps_basic_unit_special_case(tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host g++ required")
    veh_header = (ROOT / "src/veh.h").read_text()
    types = "\n".join(
        braced(veh_header, veh_header.index(marker)) + ";"
        for marker in (
            "struct VehPrototype {", "struct RulesChassis {",
            "struct RulesWeapon {", "struct RulesArmor {",
            "struct RulesReactor {",
        )
    )
    function = actual_function("propose_proto")
    harness = r"""
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#define __cdecl
typedef char *LPSTR; typedef int BOOL;
const BOOL TRUE=1, FALSE=0;
const uint32_t PrototypeAbilityCompatibilityMask=0x0044AE1Bu;
const uint32_t PrototypeAbilitySimilarityMask=0x0048841Au;
SOURCE_TYPES
const int MaxVehProtoFactionNum=64, MaxVehProtoNum=512, MaxAbilityNum=29;
const int MaxPlayerNum=8, TRIAD_LAND=0;
const uint16_t PROTO_ACTIVE=1, PROTO_CUSTOM_NAME_SET=2, PROTO_TYPED_COMPLETE=4;
struct PlayerData {
    int enemy_best_weapon_value, enemy_best_armor_value;
    uint8_t proto_id_active[512], proto_id_queue[512];
} PlayersData[8];
struct Player { char filename[24]; } Players[8];
struct RulesBasic { uint32_t psi_combat_ratio_atk[3], psi_combat_ratio_def[3]; } Rules;
VehPrototype VehPrototypes[512]; RulesChassis Chassis[19];
RulesWeapon Weapon[26]; RulesArmor Armor[14]; RulesReactor Reactor[4];
struct Base { uint8_t faction_id_current; uint32_t queue_size; int queue_production_id[10]; };
Base Bases[512]; int BaseCurrentCount;
int DesignConsiderOverride, DesignConsiderPrototype, DesignConsiderWeapon,
    DesignConsiderChassis, DesignConsiderArmor, DesignConsiderReactor;
uint32_t DesignConsiderAbilities; int DiffLevelCurrent, ExpansionEnabled,
    IsMultiplayerNet, VehCurrentCount, LockEnableMask;
int FirstProposedPrototype=-1;
bool is_bunged(int,int,int,int,int,int) { return false; }
bool has_tech(int,int) { return true; }
int psi_factor(int value,int,BOOL,BOOL) { return value; }
void make_proto(int,int,int,int,int,int) {}
void retire_proto(int,int) {}
void prune_protos(int,int,int) {}
void name_proto(char *,int,int,int,int,int,int,int) {}
LIVE_BODY
int main() {
    Chassis[0].speed=1; Armor[0].defense_rating=1; Weapon[0].offense_rating=1;
    const int result=propose_proto(1,0,0,0,0,0,0,nullptr);
    if(result!=2 || FirstProposedPrototype!=-1) {
      std::fprintf(stderr,"unexpected special-case result %d\n",result); return 42;
    }
    DesignConsiderOverride=1; DesignConsiderPrototype=-1;
    const int staged=propose_proto(1,1,1,1,0,1,4,nullptr);
    if(staged!=-1 || DesignConsiderOverride!=3 || DesignConsiderPrototype!=4
       || DesignConsiderChassis!=1 || DesignConsiderWeapon!=1
       || DesignConsiderArmor!=1 || DesignConsiderReactor!=1
       || DesignConsiderAbilities!=0) {
      std::fprintf(stderr,"proposal design staging mismatch\n"); return 43;
    }
}
""".replace("SOURCE_TYPES", types).replace("LIVE_BODY", function)
    cpp = tmp_path / "vehicle_propose_proto.cpp"
    executable = tmp_path / "vehicle_propose_proto"
    cpp.write_text(harness)
    result = subprocess.run(
        [compiler, "-std=c++11", "-O1", "-fsanitize=undefined",
         "-fno-sanitize-recover=undefined", str(cpp), "-o", str(executable)],
        capture_output=True, text=True, timeout=30)
    assert result.returncode == 0, result.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
    assert result.returncode == 0, result.stderr


def action_harness(function, is_fungal):
    veh_header = (ROOT / "src/veh.h").read_text()
    veh_type = braced(veh_header, veh_header.index("struct Veh {")) + ";"
    prototype_type = braced(veh_header, veh_header.index("struct VehPrototype {")) + ";"
    map_header = (ROOT / "src/map.h").read_text()
    map_type = braced(map_header, map_header.index("struct Map {")) + ";"
    test_path = (r"""
        rand_script[0]=0; rand_script[1]=0; rand_script[2]=0; rand_script[3]=0;
        rand_script[4]=0; rand_script[5]=0; rand_script[6]=0; rand_count=7;
        territory_result=3;
        action_fungal(0,8,9);
        if(kill_count!=1 || raises!=0 || climates!=0 || maps_drawn!=0) return 40;
        if((Tile.bit&BIT_FUNGUS)==0 || (UnkBitfield1&4)==0 || rand_index!=7) return 41;
        if(focus_count!=4 || popup_count!=2 || std::strcmp(last_label,"FUNGMOTIZED")!=0
           || std::strcmp(last_picture,"fungpayld_sm.pcx")!=0) return 42;
        if(parse_text[0]!=Players[1].title_leader) return 47;
        if(parse_text[5]!=Players[5].title_leader) return 48;
        if(last_shoot_attacker!=1) return 49;
""" if is_fungal else r"""
        territory_result=-1; action_tectonic(0,8,9);
        if(kill_count!=1 || raises!=1 || climates!=1 || maps_drawn!=1) return 43;
        if(TectonicDetonationCount[4]!=1 || TectonicDetonationCount[1]!=0
           || (UnkBitfield1&4)==0) return 44;
        if(focus_count!=1 || popup_count!=2 || std::strcmp(last_label,"TECMOFIED")!=0
           || std::strcmp(last_picture,"tectonic_sm.pcx")!=0) return 45;
        if(parse_count!=8) return 46;
        if(parse_text[0]!=Players[3].title_leader
           || parse_text[5]!=Players[5].title_leader) return 47;
""")
    failure_path = (r"""
        std::memset(&Tile,0,sizeof(Tile)); Tile.visibility=1u<<ConsoleFaction;
        Tile.climate=0; PlayersData[ConsoleFaction].flags=PFLAG_MAP_REVEALED;
        compact_faction=1; Vehs[1].faction_id=ConsoleFaction;
        whom_at=1; scripted_shot=1;
        kill_count=focus_count=popup_count=parse_count=0;
        rand_count=rand_index=0; UnkBitfield1=0;
        action_fungal(0,8,9);
        if((Tile.bit&BIT_FUNGUS)!=0 || (UnkBitfield1&4)!=0 || rand_index!=0) return 50;
        if(kill_count!=1 || focus_count!=0 || popup_count!=1
           || std::strcmp(last_label,"FUNGALMISSILE")!=0) return 51;
""" if is_fungal else r"""
        std::memset(&Tile,0,sizeof(Tile)); Tile.visibility=1u<<ConsoleFaction;
        PlayersData[ConsoleFaction].flags=PFLAG_MAP_REVEALED;
        compact_faction=1; Vehs[1].faction_id=ConsoleFaction;
        whom_at=1; scripted_shot=1;
        kill_count=focus_count=popup_count=parse_count=0;
        raises=climates=maps_drawn=0; UnkBitfield1=0;
        TectonicDetonationCount[1]=0; TectonicDetonationCount[4]=0;
        action_tectonic(0,8,9);
        if(raises!=0 || climates!=0 || maps_drawn!=0 || TectonicDetonationCount[1]!=0
           || (UnkBitfield1&4)!=0) return 52;
        if(kill_count!=1 || focus_count!=1 || popup_count!=1
           || std::strcmp(last_label,"TECTONICMISSILE")!=0) return 53;
""")
    return r"""
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#define __cdecl
typedef char *LPSTR; typedef int BOOL;
SOURCE_TYPES
const int MaxPlayerNum=8, PFLAG_MAP_REVEALED=0x200, DTREATY_PACT=1;
const int BIT_FUNGUS=0x20, BIT_ROAD=0x4, BIT_MAGTUBE=0x8, BIT_MINE=0x10;
const int BIT_SOLAR_TIDAL=0x40, BIT_FARM=0x80, BIT_SOIL_ENRICHER=0x100;
const int BIT_FOREST=0x200, MapTileFlagClearMask=0x28805c;
const int ALT_BIT_OCEAN_SHELF=0x40, BSC_MIND_WORMS=8, BSC_FUNGAL_TOWER=19;
const int RadiusRange[3]={1,2,25};
int RadiusOffsetX[25], RadiusOffsetY[25];
int MapLongitudeBounds=100, MapLatitudeBounds=100, MapIsFlat=1, GameState=0x80;
Veh Vehs[32]; VehPrototype VehPrototypes[512]; Map Tile;
struct Base { uint8_t faction_id_current; char name_string[25]; } Bases[4];
struct PlayerData { uint32_t flags; uint32_t diplo_treaties[8]; } PlayersData[8];
struct Player {
    int is_leader_female; char title_leader[24], name_leader[24];
    int noun_gender, is_noun_plural; char noun_faction[24];
} Players[8];
int ConsoleFaction=2, focus_count, kill_count, raises, climates, maps_drawn;
int parse_slots[32], parse_count, popup_count; const char *parse_text[32];
const char *last_label, *last_picture;
int compact_faction=3, compact_proto=11;
int last_shoot_attacker=-1;
int rand_script[8], rand_count, rand_index, whom_at=-1, territory_result=-1, scripted_shot;
int UnkBitfield1; uint32_t TectonicDetonationCount[8];
int GenderDefault, PluralityDefault; char PopupTitleText[]="EVENT";
char *PopupTitle=PopupTitleText; char **ClimatePopupTitle=&PopupTitle;
int rand() { if(rand_index>=rand_count) std::exit(91); return rand_script[rand_index++]; }
bool on_map(int x,int y) { return x>=0&&x<MapLongitudeBounds&&y>=0&&y<MapLatitudeBounds; }
int xrange(int x) { return x; }
Map *map_loc(int,int) { return &Tile; }
int base_find(int,int) { return 0; }
int whose_territory(int,int,int,void *,int) { return territory_result; }
int veh_at(int,int) { return whom_at; }
int stack_fix(int v) { return v; }
void kill(int vehicle_id) {
    ++kill_count;
    Vehs[vehicle_id].faction_id=compact_faction;
    Vehs[vehicle_id].proto_id=compact_proto;
}
struct Fx { void play(int,int,int) {} } g_FX;
struct Console { int faction_id_; void focus(int,int,int) {
    ++focus_count; Vehs[0].faction_id=kill_count ? 4 : 3;
} } ConsoleGlobal;
int parse_says(int slot,const char *text,int,int) {
    parse_slots[parse_count]=slot; parse_text[parse_count++]=text;
    if(slot==0 && text==Bases[0].name_string) Vehs[0].faction_id=5;
    return 0;
}
int popp(const char *,const char *label,int,const char *picture,int(__cdecl *)()) {
    ++popup_count; last_label=label; last_picture=picture;
    if(std::strcmp(label,"TECMOFIED")==0 || std::strcmp(label,"FUNGMOTIZED")==0)
        Vehs[0].faction_id=4;
    return 0;
}
void world_raise_alt(int,int) { ++raises; }
void world_climate() { ++climates; }
void draw_map(int) { ++maps_drawn; }
void veh_init(int,int,int,int) { std::exit(92); }
void bit_set(int,int,uint32_t mask,int enabled) {
    if(enabled) Tile.bit|=mask; else Tile.bit&=~mask;
}
void world_alt_set(int,int,int,int) {}
void synch_bit(int,int,int) {}
void draw_tile(int,int,int) {}
int __cdecl shoot_it(int attacker,int,int,int,int) {
    last_shoot_attacker=attacker; return scripted_shot;
}
BODY
int main() {
    std::memset(Vehs,0,sizeof(Vehs)); std::memset(VehPrototypes,0,sizeof(VehPrototypes));
    std::memset(&Tile,0,sizeof(Tile)); std::memset(Bases,0,sizeof(Bases));
    RadiusOffsetX[1]=1; RadiusOffsetY[1]=0;
    ConsoleGlobal.faction_id_=ConsoleFaction;
    Vehs[0].x=8; Vehs[0].y=9; Vehs[0].faction_id=1; Vehs[0].proto_id=10;
    VehPrototypes[10].reactor_id=1; VehPrototypes[11].reactor_id=2;
    Bases[0].faction_id_current=ConsoleFaction;
    PlayersData[ConsoleFaction].flags=PFLAG_MAP_REVEALED;
    Tile.visibility=1u<<ConsoleFaction;
    TEST_PATH
    FAILURE_PATH
}
""".replace("SOURCE_TYPES", "\n".join((veh_type, prototype_type, map_type))).replace(
        "BODY", function).replace("TEST_PATH", test_path).replace(
            "FAILURE_PATH", failure_path)


@pytest.mark.parametrize(("name", "is_fungal"), [
    ("action_tectonic", False), ("action_fungal", True),
])
def test_live_terraforming_action_effect_and_event_paths(tmp_path, name, is_fungal):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host g++ required")
    cpp = tmp_path / f"{name}.cpp"
    executable = tmp_path / name
    cpp.write_text(action_harness(actual_function(name), is_fungal))
    result = subprocess.run(
        [compiler, "-std=c++11", "-O1", "-fsanitize=undefined",
         "-fno-sanitize-recover=undefined", str(cpp), "-o", str(executable)],
        capture_output=True, text=True, timeout=30)
    assert result.returncode == 0, result.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
    assert result.returncode == 0, result.stderr
