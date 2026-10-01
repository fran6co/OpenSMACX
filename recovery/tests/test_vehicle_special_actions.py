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

    partial = {0x57E040}
    records = {r.address: r for r in read_file(ROOT / "src/veh.cpp")}
    assert partial <= records.keys()
    assert all(records[address].state.name == "PLACEHOLDER" for address in partial)
    assert records[0x4CDAF0].state.name == "IMPLEMENTED"
    assert records[0x57F1D0].state.name == "IMPLEMENTED"
    assert records[0x580860].state.name == "IMPLEMENTED"
    assert records[0x57F4B0].state.name == "IMPLEMENTED"

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


def prune_harness(function):
    return r"""
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#define __cdecl
const int MaxVehProtoFactionNum=64;
const int MaxVehProtoNum=512;
const int MaxAbilityNum=29;
const int PROTO_ACTIVE=1, PROTO_TYPED_COMPLETE=4;
const int TRIAD_LAND=0, TRUE=1, FALSE=0;
const int MPREF_BSC_AUTO_PRUNE_OBS_VEH=0x100000;
const int RFLAG_ALIEN=0x80;
typedef char *LPSTR;
struct VehPrototype {
    char veh_name[32]; uint32_t ability_flags;
    uint8_t chassis_id, weapon_id, armor_id, reactor_id;
    uint8_t carry_capacity, cost, plan; int8_t unk_1;
    uint8_t obsolete_factions; int8_t combat_factions, icon_offset, padding;
    uint16_t flags; int16_t preq_tech;
};
VehPrototype VehPrototypes[512];
struct RulesWeapon { int offense_rating; int mode; } Weapon[26];
struct RulesArmor { int defense_rating; int mode; } Armor[14];
struct RulesChassis { int triad, speed, range; } Chassis[19];
struct RulesAbility { LPSTR name, description, abbreviation; } Ability[29];
struct StringTableStub { const char *get(int) { return ""; } } StringTable;
const char *prototype_rule_name(LPSTR text) { return text; }
char StringTemp[1032];
int say_stats_2(char *,int) { return 0; }
int parsed_report_count=0;
char parsed_reports[16][1032];
int manual_flag_target=-1;
bool manual_flag_on_parse=false;
bool manual_flag_on_prompt=false;
bool multiplayer_on_prompt=false;
bool manual_flag_on_sync=false;
bool enable_lock_on_sync=false;
int parse_says(int slot,const char *text,int,int) {
    std::strncpy(parsed_reports[slot],text,sizeof(parsed_reports[slot])-1);
    ++parsed_report_count;
    if (manual_flag_on_parse && manual_flag_target >= 0)
        VehPrototypes[manual_flag_target].flags |= 0x0200;
    return 0;
}
const char *ScriptPopupFile="SCRIPT";
int popup_calls=0;
const char *popup_caption, *popup_label, *popup_picture;
int popp(const char *caption,const char *label,int,const char *picture,
         int (__cdecl *)()) {
    ++popup_calls; popup_caption=caption; popup_label=label;
    popup_picture=picture; return 0;
}
int production_upgrade_choice=0, production_upgrade_prompts=0;
extern int IsMultiplayerNet;
int X_pop(const char *label,int (__cdecl *)()) {
    if (std::strcmp(label,"ASKPRODUPGRADE")==0) ++production_upgrade_prompts;
    if (manual_flag_on_prompt && manual_flag_target >= 0)
        VehPrototypes[manual_flag_target].flags |= 0x0200;
    if (multiplayer_on_prompt) IsMultiplayerNet=1;
    return production_upgrade_choice;
}
struct Player { uint32_t rule_flags; } Players[8];
struct ConsoleStub { int faction_id_; } ConsoleGlobal={2};
uint32_t *queue_size_after_lock_target=0;
uint32_t queue_size_after_lock_value=0;
struct NetDaemonStub {
    int locks, unlocks, lock_result;
    int lock_base(int,int,int,int) {
        ++locks;
        if (queue_size_after_lock_target != 0) {
            *queue_size_after_lock_target=queue_size_after_lock_value;
        }
        return lock_result;
    }
    void unlock_base(int) { ++unlocks; }
} NetDaemonNet;
struct Base {
    uint8_t faction_id_current;
    uint32_t queue_size;
    int queue_production_id[10];
} Bases[8];
int BaseCurrentCount=0;
struct PlayerData {
    int enemy_best_armor_value, enemy_best_weapon_value;
    uint8_t proto_id_active[512], proto_id_queue[512];
} PlayersData[8];
struct RulesStub {
    int psi_combat_ratio_atk[3]={1,1,1};
    int psi_combat_ratio_def[3]={1,1,1};
} Rules;
int ExpansionEnabled=1;
int psi_factor_calls=0;
int psi_factor(int value,int,int,int) { ++psi_factor_calls; return value; }
int IsMultiplayerNet=0;
uint32_t GameMorePreferences=0;
uint32_t LockEnableMask;
int has_tech_result=1, has_tech_calls=0;
int has_tech(int,int) { ++has_tech_calls; return has_tech_result; }
int bit_count(int bits) {
    int count=0;
    for (; bits; bits &= bits-1) ++count;
    return count;
}
int synchronized_prototypes[16], synchronized_count;
void synch_obs(int prototype) {
    synchronized_prototypes[synchronized_count++]=prototype;
    if (manual_flag_on_sync && manual_flag_target >= 0)
        VehPrototypes[manual_flag_target].flags |= 0x0200;
    if (enable_lock_on_sync) LockEnableMask=4;
}
void check(bool ok,int n) {
    if (!ok) { std::fprintf(stderr,"prune host check %d failed\n",n); std::exit(42); }
}
void setup_prune_queue_case(int multiplayer) {
    GameMorePreferences=MPREF_BSC_AUTO_PRUNE_OBS_VEH;
    LockEnableMask=4;
    IsMultiplayerNet=multiplayer;
    BaseCurrentCount=1;
    Bases[0].faction_id_current=2;
    Bases[0].queue_size=0;
    Bases[0].queue_production_id[0]=129;
    VehPrototypes[128].flags=PROTO_ACTIVE;
    VehPrototypes[129].flags=PROTO_ACTIVE;
    VehPrototypes[128].plan=VehPrototypes[129].plan=3;
    VehPrototypes[128].chassis_id=VehPrototypes[129].chassis_id=2;
    VehPrototypes[128].weapon_id=5;
    VehPrototypes[129].weapon_id=6;
    VehPrototypes[128].armor_id=5;
    VehPrototypes[129].armor_id=6;
    VehPrototypes[128].reactor_id=2;
    VehPrototypes[129].reactor_id=1;
    Weapon[5].offense_rating=2;
    Weapon[6].offense_rating=1;
    Armor[5].defense_rating=3;
    Armor[6].defense_rating=2;
    Chassis[2].triad=Chassis[2].speed=Chassis[2].range=1;
}

LIVE_PRUNE_HELPERS
LIVE_PRUNE_BODY

void reset() {
    std::memset(VehPrototypes,0,sizeof(VehPrototypes));
    VehPrototypes[64].chassis_id=2;
    std::memset(Weapon,0,sizeof(Weapon));
    std::memset(Armor,0,sizeof(Armor));
    std::memset(Chassis,0,sizeof(Chassis));
    std::memset(Ability,0,sizeof(Ability));
    std::memset(Players,0,sizeof(Players));
    std::memset(PlayersData,0,sizeof(PlayersData));
    std::memset(Bases,0,sizeof(Bases));
    BaseCurrentCount=0;
    synchronized_count=0;
    LockEnableMask=0;
    IsMultiplayerNet=0;
    GameMorePreferences=0;
    ExpansionEnabled=1;
    psi_factor_calls=0;
    NetDaemonNet.locks=NetDaemonNet.unlocks=0;
    NetDaemonNet.lock_result=0;
    queue_size_after_lock_target=0;
    queue_size_after_lock_value=0;
    has_tech_result=1;
    has_tech_calls=0;
    popup_calls=parsed_report_count=0;
    std::memset(parsed_reports,0,sizeof(parsed_reports));
    production_upgrade_choice=production_upgrade_prompts=0;
    manual_flag_target=-1;
    manual_flag_on_parse=manual_flag_on_prompt=multiplayer_on_prompt=false;
    manual_flag_on_sync=false;
    enable_lock_on_sync=false;
    popup_caption=popup_label=popup_picture=nullptr;
}
int main() {
    reset();
    VehPrototypes[64].flags=PROTO_TYPED_COMPLETE;
    VehPrototypes[0].flags=PROTO_ACTIVE;
    VehPrototypes[128].flags=PROTO_ACTIVE;
    VehPrototypes[129].flags=PROTO_ACTIVE;
    prune_protos(2,64,0);
    check((VehPrototypes[128].flags & (PROTO_TYPED_COMPLETE|0x100))
          == (PROTO_TYPED_COMPLETE|0x100),1);

    reset();
    VehPrototypes[64].flags=PROTO_TYPED_COMPLETE;
    VehPrototypes[0].flags=PROTO_ACTIVE;
    VehPrototypes[128].flags=PROTO_ACTIVE;
    VehPrototypes[128].obsolete_factions=4;
    VehPrototypes[129].flags=PROTO_ACTIVE|PROTO_TYPED_COMPLETE;
    prune_protos(2,64,0);
    check((VehPrototypes[128].flags & (PROTO_TYPED_COMPLETE|0x100))
          == (PROTO_TYPED_COMPLETE|0x100),2);

    reset();
    VehPrototypes[64].flags=PROTO_TYPED_COMPLETE;
    VehPrototypes[128].flags=PROTO_ACTIVE;
    VehPrototypes[129].flags=PROTO_ACTIVE;
    prune_protos(2,64,0);
    check((VehPrototypes[128].flags & PROTO_TYPED_COMPLETE)==0,3);

    reset();
    VehPrototypes[128].flags=PROTO_ACTIVE;
    VehPrototypes[129].flags=PROTO_ACTIVE;
    VehPrototypes[128].plan=VehPrototypes[129].plan=3;
    VehPrototypes[128].unk_1=VehPrototypes[129].unk_1=4;
    VehPrototypes[128].chassis_id=VehPrototypes[129].chassis_id=2;
    VehPrototypes[128].weapon_id=VehPrototypes[129].weapon_id=5;
    VehPrototypes[128].armor_id=VehPrototypes[129].armor_id=6;
    VehPrototypes[128].reactor_id=VehPrototypes[129].reactor_id=2;
    VehPrototypes[128].carry_capacity=VehPrototypes[129].carry_capacity=1;
    VehPrototypes[128].ability_flags=VehPrototypes[129].ability_flags=0x408418;
    Chassis[2].triad=1;
    prune_protos(2,64,1);
    check((VehPrototypes[129].obsolete_factions & 4)!=0,4);
    check(synchronized_count==1 && synchronized_prototypes[0]==129,5);

    reset();
    VehPrototypes[128].flags=PROTO_ACTIVE;
    VehPrototypes[129].flags=PROTO_ACTIVE;
    VehPrototypes[128].plan=VehPrototypes[129].plan=3;
    VehPrototypes[128].unk_1=VehPrototypes[129].unk_1=4;
    VehPrototypes[128].chassis_id=VehPrototypes[129].chassis_id=2;
    VehPrototypes[128].weapon_id=VehPrototypes[129].weapon_id=5;
    VehPrototypes[128].armor_id=VehPrototypes[129].armor_id=6;
    VehPrototypes[128].reactor_id=2; VehPrototypes[129].reactor_id=1;
    VehPrototypes[128].carry_capacity=VehPrototypes[129].carry_capacity=1;
    VehPrototypes[128].ability_flags=VehPrototypes[129].ability_flags=0x408418;
    Chassis[2].triad=1;
    prune_protos(2,64,0);
    check((VehPrototypes[129].obsolete_factions & 4)!=0,6);
    check(synchronized_count==0,7);

    reset();
    VehPrototypes[128].flags=PROTO_ACTIVE;
    VehPrototypes[129].flags=PROTO_ACTIVE;
    VehPrototypes[128].plan=VehPrototypes[129].plan=3;
    VehPrototypes[128].unk_1=VehPrototypes[129].unk_1=4;
    VehPrototypes[128].chassis_id=VehPrototypes[129].chassis_id=2;
    VehPrototypes[128].weapon_id=VehPrototypes[129].weapon_id=5;
    VehPrototypes[128].armor_id=VehPrototypes[129].armor_id=6;
    VehPrototypes[128].reactor_id=VehPrototypes[129].reactor_id=2;
    VehPrototypes[128].carry_capacity=1; VehPrototypes[129].carry_capacity=0;
    VehPrototypes[128].ability_flags=VehPrototypes[129].ability_flags=0x408418;
    Chassis[2].triad=1;
    prune_protos(2,64,0);
    check((VehPrototypes[129].obsolete_factions & 4)!=0,8);

    reset();
    VehPrototypes[128].flags=PROTO_ACTIVE;
    VehPrototypes[129].flags=PROTO_ACTIVE;
    VehPrototypes[128].plan=3; VehPrototypes[129].plan=4;
    VehPrototypes[128].unk_1=VehPrototypes[129].unk_1=4;
    VehPrototypes[128].chassis_id=VehPrototypes[129].chassis_id=2;
    VehPrototypes[128].weapon_id=VehPrototypes[129].weapon_id=5;
    VehPrototypes[128].armor_id=VehPrototypes[129].armor_id=6;
    VehPrototypes[128].reactor_id=VehPrototypes[129].reactor_id=2;
    VehPrototypes[128].carry_capacity=VehPrototypes[129].carry_capacity=1;
    VehPrototypes[128].ability_flags=VehPrototypes[129].ability_flags=0x408418;
    Chassis[2].triad=1;
    prune_protos(2,64,0);
    check((VehPrototypes[129].obsolete_factions & 4)==0,9);
    check(synchronized_count==0,10);

    reset();
    VehPrototypes[128].flags=PROTO_ACTIVE|PROTO_TYPED_COMPLETE;
    VehPrototypes[5].flags=PROTO_ACTIVE;
    VehPrototypes[128].plan=VehPrototypes[5].plan=3;
    VehPrototypes[128].unk_1=VehPrototypes[5].unk_1=4;
    VehPrototypes[128].chassis_id=VehPrototypes[5].chassis_id=2;
    VehPrototypes[128].weapon_id=6; VehPrototypes[5].weapon_id=5;
    VehPrototypes[128].armor_id=6; VehPrototypes[5].armor_id=5;
    VehPrototypes[128].reactor_id=VehPrototypes[5].reactor_id=2;
    VehPrototypes[128].carry_capacity=VehPrototypes[5].carry_capacity=1;
    VehPrototypes[128].ability_flags=0x409418;
    VehPrototypes[5].ability_flags=0x408418;
    Weapon[5].offense_rating=1; Weapon[6].offense_rating=2;
    Weapon[5].mode=Weapon[6].mode=1;
    Armor[5].defense_rating=2; Armor[6].defense_rating=3;
    Chassis[2].triad=1;
    prune_protos(2,64,1);
    check((VehPrototypes[5].obsolete_factions & 4)!=0,11);
    check(synchronized_count==1 && synchronized_prototypes[0]==5,12);

    reset();
    VehPrototypes[128].flags=PROTO_ACTIVE|PROTO_TYPED_COMPLETE;
    VehPrototypes[5].flags=PROTO_ACTIVE;
    VehPrototypes[128].plan=VehPrototypes[5].plan=3;
    VehPrototypes[128].unk_1=VehPrototypes[5].unk_1=4;
    VehPrototypes[128].chassis_id=VehPrototypes[5].chassis_id=2;
    VehPrototypes[128].weapon_id=VehPrototypes[5].weapon_id=5;
    VehPrototypes[128].armor_id=VehPrototypes[5].armor_id=6;
    VehPrototypes[128].reactor_id=VehPrototypes[5].reactor_id=2;
    VehPrototypes[128].carry_capacity=VehPrototypes[5].carry_capacity=1;
    VehPrototypes[128].ability_flags=0x408418;
    VehPrototypes[5].ability_flags=0x418;
    Chassis[2].triad=1;
    prune_protos(2,64,0);
    check((VehPrototypes[5].obsolete_factions & 4)==0,13);

    reset();
    VehPrototypes[128].flags=PROTO_ACTIVE;
    VehPrototypes[129].flags=PROTO_ACTIVE;
    VehPrototypes[128].plan=VehPrototypes[129].plan=3;
    VehPrototypes[128].unk_1=VehPrototypes[129].unk_1=4;
    VehPrototypes[128].chassis_id=VehPrototypes[129].chassis_id=2;
    VehPrototypes[128].weapon_id=VehPrototypes[129].weapon_id=5;
    VehPrototypes[128].armor_id=VehPrototypes[129].armor_id=6;
    VehPrototypes[128].reactor_id=2; VehPrototypes[129].reactor_id=1;
    VehPrototypes[128].carry_capacity=VehPrototypes[129].carry_capacity=1;
    VehPrototypes[128].ability_flags=VehPrototypes[129].ability_flags=0x408418;
    Chassis[2].triad=1; LockEnableMask=4;
    prune_protos(2,64,0);
    check((VehPrototypes[129].obsolete_factions & 4)==0,14);

    reset();
    GameMorePreferences=MPREF_BSC_AUTO_PRUNE_OBS_VEH;
    LockEnableMask=4;
    BaseCurrentCount=1;
    Bases[0].faction_id_current=2;
    Bases[0].queue_size=1;
    Bases[0].queue_production_id[0]=129;
    Bases[0].queue_production_id[1]=128;
    Bases[1].faction_id_current=3;
    Bases[1].queue_size=0;
    Bases[1].queue_production_id[0]=129;
    BaseCurrentCount=2;
    VehPrototypes[128].flags=PROTO_ACTIVE;
    VehPrototypes[129].flags=PROTO_ACTIVE;
    VehPrototypes[128].plan=VehPrototypes[129].plan=3;
    VehPrototypes[128].chassis_id=VehPrototypes[129].chassis_id=2;
    VehPrototypes[128].weapon_id=5; VehPrototypes[129].weapon_id=6;
    VehPrototypes[128].armor_id=5; VehPrototypes[129].armor_id=6;
    VehPrototypes[128].reactor_id=2; VehPrototypes[129].reactor_id=1;
    VehPrototypes[128].ability_flags=VehPrototypes[129].ability_flags=0;
    Weapon[5].offense_rating=2; Weapon[6].offense_rating=1;
    Armor[5].defense_rating=3; Armor[6].defense_rating=2;
    Chassis[2].triad=Chassis[2].speed=Chassis[2].range=1;
    prune_protos(2,64,1);
    check((VehPrototypes[129].obsolete_factions & 4)!=0,15);
    check(synchronized_count==1 && synchronized_prototypes[0]==129,16);
    check(Bases[0].queue_production_id[0]==128,17);
    check(Bases[0].queue_production_id[1]==128,18);
    check(Bases[1].queue_production_id[0]==129,20);

    reset();
    GameMorePreferences=MPREF_BSC_AUTO_PRUNE_OBS_VEH;
    LockEnableMask=4;
    VehPrototypes[128].flags=PROTO_ACTIVE;
    VehPrototypes[129].flags=PROTO_ACTIVE;
    VehPrototypes[128].plan=VehPrototypes[129].plan=3;
    VehPrototypes[128].chassis_id=VehPrototypes[129].chassis_id=2;
    VehPrototypes[128].weapon_id=5; VehPrototypes[129].weapon_id=6;
    VehPrototypes[128].armor_id=5; VehPrototypes[129].armor_id=6;
    VehPrototypes[128].reactor_id=2; VehPrototypes[129].reactor_id=1;
    VehPrototypes[128].ability_flags=1; VehPrototypes[129].ability_flags=0;
    Weapon[5].offense_rating=1; Weapon[6].offense_rating=2;
    Armor[5].defense_rating=2; Armor[6].defense_rating=3;
    Chassis[2].triad=Chassis[2].speed=Chassis[2].range=1;
    PlayersData[2].proto_id_active[129]=1;
    prune_protos(2,64,0);
    check((VehPrototypes[129].obsolete_factions & 4)==0,19);

    reset();
    IsMultiplayerNet=1;
    LockEnableMask=4;
    BaseCurrentCount=1;
    Bases[0].faction_id_current=2;
    Bases[0].queue_size=0;
    Bases[0].queue_production_id[0]=129;
    VehPrototypes[128].flags=PROTO_ACTIVE;
    VehPrototypes[129].flags=PROTO_ACTIVE;
    VehPrototypes[128].plan=VehPrototypes[129].plan=3;
    VehPrototypes[128].chassis_id=VehPrototypes[129].chassis_id=2;
    VehPrototypes[128].weapon_id=5; VehPrototypes[129].weapon_id=6;
    VehPrototypes[128].armor_id=5; VehPrototypes[129].armor_id=6;
    VehPrototypes[128].reactor_id=2; VehPrototypes[129].reactor_id=1;
    Weapon[5].offense_rating=2; Weapon[6].offense_rating=1;
    Armor[5].defense_rating=3; Armor[6].defense_rating=2;
    Chassis[2].triad=Chassis[2].speed=Chassis[2].range=1;
    Players[2].rule_flags=RFLAG_ALIEN;
    prune_protos(2,64,1);
    check(Bases[0].queue_production_id[0]==128,21);
    check(NetDaemonNet.locks==1 && NetDaemonNet.unlocks==1,22);
    check(popup_calls==1 && std::strcmp(popup_caption,"SCRIPT")==0
          && std::strcmp(popup_label,"PRODUPGRADE")==0
          && std::strcmp(popup_picture,"alunit_sm.pcx")==0,23);
    check(parsed_report_count==2,24);

    // The native network path skips this base when locking fails; it neither
    // mutates its production queue nor unlocks a lock it never acquired.
    reset();
    IsMultiplayerNet=1;
    LockEnableMask=4;
    BaseCurrentCount=1;
    Bases[0].faction_id_current=2;
    Bases[0].queue_size=0;
    Bases[0].queue_production_id[0]=129;
    NetDaemonNet.lock_result=1;
    VehPrototypes[128].flags=PROTO_ACTIVE;
    VehPrototypes[129].flags=PROTO_ACTIVE;
    VehPrototypes[128].plan=VehPrototypes[129].plan=3;
    VehPrototypes[128].chassis_id=VehPrototypes[129].chassis_id=2;
    VehPrototypes[128].weapon_id=5; VehPrototypes[129].weapon_id=6;
    VehPrototypes[128].armor_id=5; VehPrototypes[129].armor_id=6;
    VehPrototypes[128].reactor_id=2; VehPrototypes[129].reactor_id=1;
    Weapon[5].offense_rating=2; Weapon[6].offense_rating=1;
    Armor[5].defense_rating=3; Armor[6].defense_rating=2;
    Chassis[2].triad=Chassis[2].speed=Chassis[2].range=1;
    prune_protos(2,64,1);
    check(Bases[0].queue_production_id[0]==129,25);
    check(NetDaemonNet.locks==1 && NetDaemonNet.unlocks==0,26);
    check(popup_calls==0,27);

    // The local prompt first publishes both design reports. A choice greater
    // than one still permits queue substitution but avoids the manual-obsolete
    // flag transition, matching the native X_pop branch.
    reset();
    GameMorePreferences=MPREF_BSC_AUTO_PRUNE_OBS_VEH;
    LockEnableMask=4;
    BaseCurrentCount=1;
    Bases[0].faction_id_current=2;
    Bases[0].queue_size=0;
    Bases[0].queue_production_id[0]=129;
    VehPrototypes[128].flags=PROTO_ACTIVE;
    VehPrototypes[129].flags=PROTO_ACTIVE;
    VehPrototypes[128].plan=VehPrototypes[129].plan=3;
    VehPrototypes[128].chassis_id=VehPrototypes[129].chassis_id=2;
    VehPrototypes[128].weapon_id=5; VehPrototypes[129].weapon_id=6;
    VehPrototypes[128].armor_id=5; VehPrototypes[129].armor_id=6;
    VehPrototypes[128].reactor_id=2; VehPrototypes[129].reactor_id=1;
    Weapon[5].offense_rating=2; Weapon[6].offense_rating=1;
    Armor[5].defense_rating=3; Armor[6].defense_rating=2;
    Chassis[2].triad=Chassis[2].speed=Chassis[2].range=1;
    production_upgrade_choice=2;
    std::strcpy(VehPrototypes[129].veh_name,"Obsolete");
    std::strcpy(VehPrototypes[128].veh_name,"Replacement");
    VehPrototypes[129].ability_flags=1;
    Ability[0].abbreviation=const_cast<char *>("ABL");
    prune_protos(2,64,0);
    check(production_upgrade_prompts==1 && parsed_report_count==2,28);
    check(std::strcmp(parsed_reports[0],"Obsolete ( ABL)")==0,40);
    check(std::strcmp(parsed_reports[1],"Replacement ()")==0,41);
    check(Bases[0].queue_production_id[0]==128,29);
    check((VehPrototypes[129].flags & 0x0208)==0,30);

    // The local report/prompt is selected by the faction-base walk, not by
    // finding this prototype in a queue; a nonmatching queue remains intact.
    reset();
    GameMorePreferences=MPREF_BSC_AUTO_PRUNE_OBS_VEH;
    LockEnableMask=4;
    BaseCurrentCount=1;
    Bases[0].faction_id_current=2;
    Bases[0].queue_size=0;
    Bases[0].queue_production_id[0]=130;
    BaseCurrentCount=2;
    Bases[1].faction_id_current=2;
    Bases[1].queue_size=0;
    Bases[1].queue_production_id[0]=129;
    VehPrototypes[128].flags=PROTO_ACTIVE;
    VehPrototypes[129].flags=PROTO_ACTIVE;
    VehPrototypes[128].plan=VehPrototypes[129].plan=3;
    VehPrototypes[128].chassis_id=VehPrototypes[129].chassis_id=2;
    VehPrototypes[128].weapon_id=5; VehPrototypes[129].weapon_id=6;
    VehPrototypes[128].armor_id=5; VehPrototypes[129].armor_id=6;
    VehPrototypes[128].reactor_id=2; VehPrototypes[129].reactor_id=1;
    Weapon[5].offense_rating=2; Weapon[6].offense_rating=1;
    Armor[5].defense_rating=3; Armor[6].defense_rating=2;
    Chassis[2].triad=Chassis[2].speed=Chassis[2].range=1;
    production_upgrade_choice=2;
    prune_protos(2,64,0);
    check(production_upgrade_prompts==1 && parsed_report_count==2,31);
    check(Bases[0].queue_production_id[0]==130,32);

    reset();
    GameMorePreferences=MPREF_BSC_AUTO_PRUNE_OBS_VEH;
    LockEnableMask=4;
    BaseCurrentCount=1;
    Bases[0].faction_id_current=2;
    Bases[0].queue_size=0;
    Bases[0].queue_production_id[0]=129;
    VehPrototypes[128].flags=PROTO_ACTIVE;
    VehPrototypes[129].flags=PROTO_ACTIVE;
    VehPrototypes[128].plan=VehPrototypes[129].plan=3;
    VehPrototypes[128].chassis_id=VehPrototypes[129].chassis_id=2;
    VehPrototypes[128].weapon_id=5; VehPrototypes[129].weapon_id=6;
    VehPrototypes[128].armor_id=5; VehPrototypes[129].armor_id=6;
    VehPrototypes[128].reactor_id=2; VehPrototypes[129].reactor_id=1;
    VehPrototypes[129].flags|=0x0008;
    Weapon[5].offense_rating=2; Weapon[6].offense_rating=1;
    Armor[5].defense_rating=3; Armor[6].defense_rating=2;
    Chassis[2].triad=Chassis[2].speed=Chassis[2].range=1;
    production_upgrade_choice=1;
    prune_protos(2,64,0);
    check(production_upgrade_prompts==1,33);
    check((VehPrototypes[129].obsolete_factions & 4)==0,34);
    check((VehPrototypes[129].flags & 0x0208)==0x0208,35);
    check(Bases[0].queue_production_id[0]==129,36);

    reset();
    GameMorePreferences=MPREF_BSC_AUTO_PRUNE_OBS_VEH;
    VehPrototypes[128].flags=PROTO_ACTIVE;
    VehPrototypes[129].flags=PROTO_ACTIVE;
    VehPrototypes[128].plan=VehPrototypes[129].plan=3;
    VehPrototypes[128].unk_1=1; VehPrototypes[129].unk_1=2;
    VehPrototypes[128].chassis_id=VehPrototypes[129].chassis_id=2;
    VehPrototypes[128].weapon_id=5; VehPrototypes[129].weapon_id=6;
    VehPrototypes[128].armor_id=VehPrototypes[129].armor_id=5;
    VehPrototypes[128].reactor_id=2; VehPrototypes[129].reactor_id=1;
    Weapon[5].offense_rating=Weapon[6].offense_rating=0;
    Armor[5].defense_rating=1;
    Chassis[2].triad=Chassis[2].speed=Chassis[2].range=1;
    prune_protos(2,64,0);
    check((VehPrototypes[129].obsolete_factions & 4)==0,37);

    reset();
    GameMorePreferences=MPREF_BSC_AUTO_PRUNE_OBS_VEH;
    LockEnableMask=4;
    ExpansionEnabled=0;
    VehPrototypes[128].flags=PROTO_ACTIVE;
    VehPrototypes[129].flags=PROTO_ACTIVE;
    VehPrototypes[128].plan=VehPrototypes[129].plan=3;
    VehPrototypes[128].unk_1=1; VehPrototypes[129].unk_1=2;
    VehPrototypes[128].chassis_id=VehPrototypes[129].chassis_id=2;
    VehPrototypes[128].weapon_id=10; VehPrototypes[129].weapon_id=5;
    VehPrototypes[128].armor_id=VehPrototypes[129].armor_id=5;
    VehPrototypes[128].reactor_id=2; VehPrototypes[129].reactor_id=1;
    Weapon[10].offense_rating=5; Weapon[5].offense_rating=1;
    Armor[5].defense_rating=1;
    Chassis[2].triad=Chassis[2].speed=Chassis[2].range=1;
    prune_protos(2,64,0);
    check((VehPrototypes[129].obsolete_factions & 4)!=0,38);

    // Native dominance compares chassis triad and range, not chassis IDs.
    // The late replacement pass rejects this higher-reactor target, leaving
    // the earlier cross-prototype comparison to obsolete the lower-reactor peer.
    reset();
    VehPrototypes[128].flags=PROTO_ACTIVE;
    VehPrototypes[129].flags=PROTO_ACTIVE;
    VehPrototypes[128].plan=VehPrototypes[129].plan=3;
    VehPrototypes[128].unk_1=VehPrototypes[129].unk_1=2;
    VehPrototypes[128].chassis_id=2; VehPrototypes[129].chassis_id=3;
    VehPrototypes[128].weapon_id=5; VehPrototypes[129].weapon_id=6;
    VehPrototypes[128].armor_id=VehPrototypes[129].armor_id=5;
    VehPrototypes[128].reactor_id=1; VehPrototypes[129].reactor_id=2;
    Weapon[5].offense_rating=1; Weapon[6].offense_rating=2;
    Armor[5].defense_rating=1;
    Chassis[2].triad=Chassis[3].triad=1;
    Chassis[2].range=Chassis[3].range=1;
    Chassis[2].speed=Chassis[3].speed=1;
    prune_protos(2,64,0);
    check((VehPrototypes[128].obsolete_factions & 4)!=0,39);

    // The native first dominance pass takes its lower-reactor early mark
    // after triad equality and before the later range check.
    reset();
    VehPrototypes[10].flags=PROTO_ACTIVE;
    VehPrototypes[129].flags=PROTO_ACTIVE|PROTO_TYPED_COMPLETE;
    VehPrototypes[10].plan=VehPrototypes[129].plan=3;
    VehPrototypes[10].unk_1=VehPrototypes[129].unk_1=2;
    VehPrototypes[10].chassis_id=2; VehPrototypes[129].chassis_id=3;
    VehPrototypes[10].weapon_id=5; VehPrototypes[129].weapon_id=6;
    VehPrototypes[10].armor_id=VehPrototypes[129].armor_id=5;
    VehPrototypes[10].reactor_id=1; VehPrototypes[129].reactor_id=2;
    Weapon[5].offense_rating=1; Weapon[6].offense_rating=2;
    Armor[5].defense_rating=1;
    Chassis[2].triad=Chassis[3].triad=1;
    Chassis[2].range=1; Chassis[3].range=2;
    Chassis[2].speed=Chassis[3].speed=1;
    prune_protos(2,64,0);
    check((VehPrototypes[10].obsolete_factions & 4)!=0,42);

    // Native 57FA95 marks a lower-reactor peer before carry, ratings, and
    // ability dominance.
    reset();
    VehPrototypes[128].flags=PROTO_ACTIVE;
    VehPrototypes[129].flags=PROTO_ACTIVE;
    VehPrototypes[128].plan=VehPrototypes[129].plan=3;
    VehPrototypes[128].unk_1=VehPrototypes[129].unk_1=2;
    VehPrototypes[128].chassis_id=VehPrototypes[129].chassis_id=2;
    VehPrototypes[128].weapon_id=5; VehPrototypes[129].weapon_id=6;
    VehPrototypes[128].armor_id=VehPrototypes[129].armor_id=5;
    VehPrototypes[128].reactor_id=0; VehPrototypes[129].reactor_id=1;
    VehPrototypes[128].carry_capacity=0;
    VehPrototypes[129].carry_capacity=1;
    VehPrototypes[128].ability_flags=2;
    Weapon[5].offense_rating=9; Weapon[6].offense_rating=1;
    Armor[5].defense_rating=1;
    Chassis[2].triad=Chassis[2].range=1;
    prune_protos(2,64,0);
    check((VehPrototypes[128].obsolete_factions & 4)!=0,43);

    // Native 57FA95 excludes typed-complete peers when the target is not
    // typed-complete (the local_30 path has not enabled that comparison).
    reset();
    VehPrototypes[128].flags=PROTO_ACTIVE|PROTO_TYPED_COMPLETE;
    VehPrototypes[129].flags=PROTO_ACTIVE;
    VehPrototypes[128].plan=VehPrototypes[129].plan=3;
    VehPrototypes[128].unk_1=VehPrototypes[129].unk_1=2;
    VehPrototypes[128].chassis_id=VehPrototypes[129].chassis_id=2;
    VehPrototypes[128].carry_capacity=0;
    VehPrototypes[129].carry_capacity=1;
    Chassis[2].triad=Chassis[2].range=1;
    prune_protos(2,64,0);
    check((VehPrototypes[128].obsolete_factions & 4)==0,44);

    // Native automatic pruning is directional: a replacement with lower raw
    // offense cannot retire the stronger target.
    reset();
    LockEnableMask=4;
    GameMorePreferences=MPREF_BSC_AUTO_PRUNE_OBS_VEH;
    VehPrototypes[128].flags=PROTO_ACTIVE;
    VehPrototypes[129].flags=PROTO_ACTIVE;
    VehPrototypes[128].plan=VehPrototypes[129].plan=3;
    VehPrototypes[128].chassis_id=VehPrototypes[129].chassis_id=2;
    VehPrototypes[128].weapon_id=6; VehPrototypes[129].weapon_id=5;
    VehPrototypes[128].armor_id=VehPrototypes[129].armor_id=5;
    VehPrototypes[128].reactor_id=2; VehPrototypes[129].reactor_id=1;
    VehPrototypes[128].carry_capacity=1;
    Weapon[5].offense_rating=2; Weapon[6].offense_rating=1;
    Armor[5].defense_rating=1;
    Chassis[2].triad=Chassis[2].range=Chassis[2].speed=1;
    prune_protos(2,64,0);
    check((VehPrototypes[129].obsolete_factions & 4)==0,45);

    // Ability differences may bypass live/queued protections when the
    // replacement has a reactor advantage; allowed-bit differences remain
    // subject to the native mask.
    reset();
    LockEnableMask=4;
    GameMorePreferences=MPREF_BSC_AUTO_PRUNE_OBS_VEH;
    VehPrototypes[128].flags=PROTO_ACTIVE;
    VehPrototypes[129].flags=PROTO_ACTIVE;
    VehPrototypes[128].plan=VehPrototypes[129].plan=3;
    VehPrototypes[128].chassis_id=VehPrototypes[129].chassis_id=2;
    VehPrototypes[128].weapon_id=VehPrototypes[129].weapon_id=5;
    VehPrototypes[128].armor_id=VehPrototypes[129].armor_id=5;
    VehPrototypes[128].reactor_id=2; VehPrototypes[129].reactor_id=1;
    VehPrototypes[128].ability_flags=0;
    VehPrototypes[129].ability_flags=2;
    Weapon[5].offense_rating=1;
    Armor[5].defense_rating=1;
    Chassis[2].triad=Chassis[2].range=Chassis[2].speed=1;
    PlayersData[2].proto_id_active[129]=1;
    PlayersData[2].proto_id_queue[129]=1;
    prune_protos(2,64,0);
    check((VehPrototypes[129].obsolete_factions & 4)!=0,46);

    // When the target is the function prototype ID, native sets flag 0x8
    // and synchronizes it without setting its ordinary obsolete bit.
    reset();
    LockEnableMask=4;
    GameMorePreferences=MPREF_BSC_AUTO_PRUNE_OBS_VEH;
    VehPrototypes[128].flags=PROTO_ACTIVE;
    VehPrototypes[129].flags=PROTO_ACTIVE;
    VehPrototypes[128].plan=VehPrototypes[129].plan=3;
    VehPrototypes[128].chassis_id=VehPrototypes[129].chassis_id=2;
    VehPrototypes[128].weapon_id=VehPrototypes[129].weapon_id=5;
    VehPrototypes[128].armor_id=VehPrototypes[129].armor_id=5;
    VehPrototypes[128].reactor_id=2; VehPrototypes[129].reactor_id=1;
    Weapon[5].offense_rating=1;
    Armor[5].defense_rating=1;
    Chassis[2].triad=Chassis[2].range=Chassis[2].speed=1;
    prune_protos(2,129,1);
    check((VehPrototypes[129].flags & 0x0008)!=0
          && (VehPrototypes[129].obsolete_factions & 4)==0,47);
    check(synchronized_count==1 && synchronized_prototypes[0]==129,48);

    // A predefined target is marked first; its prerequisite is tested only
    // after that native mark, not as an inner-loop candidate gate.
    reset();
    LockEnableMask=4;
    GameMorePreferences=MPREF_BSC_AUTO_PRUNE_OBS_VEH;
    has_tech_result=0;
    VehPrototypes[128].flags=PROTO_ACTIVE|PROTO_TYPED_COMPLETE;
    VehPrototypes[5].flags=PROTO_ACTIVE;
    VehPrototypes[128].plan=VehPrototypes[5].plan=3;
    VehPrototypes[128].chassis_id=VehPrototypes[5].chassis_id=2;
    VehPrototypes[128].weapon_id=6; VehPrototypes[5].weapon_id=5;
    VehPrototypes[128].armor_id=VehPrototypes[5].armor_id=5;
    VehPrototypes[128].reactor_id=2; VehPrototypes[5].reactor_id=1;
    VehPrototypes[128].carry_capacity=VehPrototypes[5].carry_capacity=1;
    Weapon[5].offense_rating=1; Weapon[6].offense_rating=2;
    Armor[5].defense_rating=1;
    Chassis[2].triad=Chassis[2].range=Chassis[2].speed=1;
    prune_protos(2,64,0);
    check((VehPrototypes[5].obsolete_factions & 4)!=0,49);

    // With options enabled, a non-MP/nonlocal base path still locks. Queue
    // size is reread after the lock callback, so newly-live slot 1 is changed.
    reset();
    ConsoleGlobal.faction_id_=3;
    LockEnableMask=4;
    GameMorePreferences=MPREF_BSC_AUTO_PRUNE_OBS_VEH;
    BaseCurrentCount=2;
    Bases[0].faction_id_current=2;
    Bases[0].queue_size=0;
    Bases[0].queue_production_id[0]=130;
    Bases[0].queue_production_id[1]=129;
    Bases[1].faction_id_current=2;
    Bases[1].queue_size=0;
    Bases[1].queue_production_id[0]=129;
    queue_size_after_lock_target=&Bases[0].queue_size;
    queue_size_after_lock_value=1;
    VehPrototypes[128].flags=PROTO_ACTIVE;
    VehPrototypes[129].flags=PROTO_ACTIVE;
    VehPrototypes[128].plan=VehPrototypes[129].plan=3;
    VehPrototypes[128].chassis_id=VehPrototypes[129].chassis_id=2;
    VehPrototypes[128].weapon_id=6; VehPrototypes[129].weapon_id=5;
    VehPrototypes[128].armor_id=VehPrototypes[129].armor_id=5;
    VehPrototypes[128].reactor_id=2; VehPrototypes[129].reactor_id=1;
    Weapon[5].offense_rating=1; Weapon[6].offense_rating=2;
    Armor[5].defense_rating=1;
    Chassis[2].triad=Chassis[2].range=Chassis[2].speed=1;
    prune_protos(2,64,1);
    check(Bases[0].queue_production_id[1]==128,50);
    check(NetDaemonNet.locks==2 && NetDaemonNet.unlocks==0,51);
    ConsoleGlobal.faction_id_=2;

    // A predefined outer replacement is rejected by its own prerequisite
    // check, while inner predefined targets are checked only after marking.
    reset();
    LockEnableMask=4;
    GameMorePreferences=MPREF_BSC_AUTO_PRUNE_OBS_VEH;
    has_tech_result=0;
    VehPrototypes[5].flags=PROTO_ACTIVE;
    VehPrototypes[129].flags=PROTO_ACTIVE;
    VehPrototypes[5].plan=VehPrototypes[129].plan=3;
    VehPrototypes[5].chassis_id=VehPrototypes[129].chassis_id=2;
    VehPrototypes[5].weapon_id=6; VehPrototypes[129].weapon_id=5;
    VehPrototypes[5].armor_id=VehPrototypes[129].armor_id=5;
    VehPrototypes[5].reactor_id=2; VehPrototypes[129].reactor_id=1;
    VehPrototypes[5].carry_capacity=1;
    Weapon[5].offense_rating=1; Weapon[6].offense_rating=2;
    Armor[5].defense_rating=1;
    Chassis[2].triad=Chassis[2].range=Chassis[2].speed=1;
    prune_protos(2,64,0);
    check((VehPrototypes[129].obsolete_factions & 4)==0
          && has_tech_calls!=0,52);

    // A disallowed ability delta fails before any activity/queue exception.
    reset();
    LockEnableMask=4;
    GameMorePreferences=MPREF_BSC_AUTO_PRUNE_OBS_VEH;
    VehPrototypes[128].flags=PROTO_ACTIVE;
    VehPrototypes[129].flags=PROTO_ACTIVE;
    VehPrototypes[128].plan=VehPrototypes[129].plan=3;
    VehPrototypes[128].chassis_id=VehPrototypes[129].chassis_id=2;
    VehPrototypes[128].weapon_id=VehPrototypes[129].weapon_id=5;
    VehPrototypes[128].armor_id=VehPrototypes[129].armor_id=5;
    VehPrototypes[128].reactor_id=2; VehPrototypes[129].reactor_id=1;
    VehPrototypes[129].ability_flags=4;
    Weapon[5].offense_rating=1;
    Armor[5].defense_rating=1;
    Chassis[2].triad=Chassis[2].range=Chassis[2].speed=1;
    prune_protos(2,64,0);
    check((VehPrototypes[129].obsolete_factions & 4)==0,53);

    // The local prompt and every base side effect require a target ID found
    // by the global owned-base queue scan.
    reset();
    LockEnableMask=4;
    GameMorePreferences=MPREF_BSC_AUTO_PRUNE_OBS_VEH;
    BaseCurrentCount=1;
    Bases[0].faction_id_current=2;
    Bases[0].queue_size=0;
    Bases[0].queue_production_id[0]=130;
    VehPrototypes[128].flags=PROTO_ACTIVE;
    VehPrototypes[129].flags=PROTO_ACTIVE;
    VehPrototypes[128].plan=VehPrototypes[129].plan=3;
    VehPrototypes[128].chassis_id=VehPrototypes[129].chassis_id=2;
    VehPrototypes[128].weapon_id=6; VehPrototypes[129].weapon_id=5;
    VehPrototypes[128].armor_id=VehPrototypes[129].armor_id=5;
    VehPrototypes[128].reactor_id=2; VehPrototypes[129].reactor_id=1;
    Weapon[5].offense_rating=1; Weapon[6].offense_rating=2;
    Armor[5].defense_rating=1;
    Chassis[2].triad=Chassis[2].range=Chassis[2].speed=1;
    prune_protos(2,64,0);
    check(production_upgrade_prompts==0
          && Bases[0].queue_production_id[0]==130,54);

    // Exact part duplicates crossing the predefined/faction boundary are
    // skipped by the native late walk.
    reset();
    LockEnableMask=4;
    GameMorePreferences=MPREF_BSC_AUTO_PRUNE_OBS_VEH;
    VehPrototypes[5].flags=PROTO_ACTIVE;
    VehPrototypes[129].flags=PROTO_ACTIVE;
    VehPrototypes[5].plan=VehPrototypes[129].plan=3;
    VehPrototypes[5].chassis_id=VehPrototypes[129].chassis_id=2;
    VehPrototypes[5].weapon_id=VehPrototypes[129].weapon_id=5;
    VehPrototypes[5].armor_id=VehPrototypes[129].armor_id=5;
    VehPrototypes[5].reactor_id=VehPrototypes[129].reactor_id=1;
    Weapon[5].offense_rating=1;
    Armor[5].defense_rating=1;
    Chassis[2].triad=Chassis[2].range=Chassis[2].speed=1;
    prune_protos(2,64,0);
    check((VehPrototypes[129].obsolete_factions & 4)==0,55);

    // The same exact design pair within faction-owned slots is not rejected
    // by that predefined-boundary exception.
    reset();
    LockEnableMask=4;
    GameMorePreferences=MPREF_BSC_AUTO_PRUNE_OBS_VEH;
    VehPrototypes[128].flags=PROTO_ACTIVE;
    VehPrototypes[129].flags=PROTO_ACTIVE;
    VehPrototypes[128].plan=VehPrototypes[129].plan=3;
    VehPrototypes[128].chassis_id=VehPrototypes[129].chassis_id=2;
    VehPrototypes[128].weapon_id=VehPrototypes[129].weapon_id=5;
    VehPrototypes[128].armor_id=VehPrototypes[129].armor_id=5;
    VehPrototypes[128].reactor_id=VehPrototypes[129].reactor_id=1;
    Weapon[5].offense_rating=1;
    Armor[5].defense_rating=1;
    Chassis[2].triad=Chassis[2].range=Chassis[2].speed=1;
    prune_protos(2,64,0);
    check((VehPrototypes[129].obsolete_factions & 4)!=0,56);

    // The native first predefined comparison has a target prerequisite gate,
    // not a per-candidate has_tech call.
    reset();
    has_tech_result=0;
    VehPrototypes[5].flags=PROTO_ACTIVE;
    VehPrototypes[5].preq_tech=99;
    VehPrototypes[128].flags=PROTO_ACTIVE|PROTO_TYPED_COMPLETE;
    VehPrototypes[5].plan=VehPrototypes[128].plan=3;
    VehPrototypes[5].chassis_id=VehPrototypes[128].chassis_id=2;
    VehPrototypes[5].reactor_id=1; VehPrototypes[128].reactor_id=2;
    Chassis[2].triad=Chassis[2].range=Chassis[2].speed=1;
    prune_protos(2,64,0);
    check((VehPrototypes[5].obsolete_factions & 4)!=0
          && has_tech_calls==0,57);

    // Faction zero aliases the faction bank with predefined IDs. Native first
    // pass prerequisite checking applies to the full target ID, and the target
    // itself is skipped as a predefined candidate.
    reset();
    VehPrototypes[1].flags=PROTO_ACTIVE;
    VehPrototypes[1].preq_tech=99;
    VehPrototypes[1].plan=3;
    VehPrototypes[1].chassis_id=2;
    Chassis[2].triad=Chassis[2].range=Chassis[2].speed=1;
    prune_protos(0,64,0);
    check((VehPrototypes[1].obsolete_factions & 1)==0
          && has_tech_calls==1,58);

    reset();
    has_tech_result=0;
    VehPrototypes[1].flags=PROTO_ACTIVE;
    VehPrototypes[1].preq_tech=99;
    prune_protos(0,64,0);
    check((VehPrototypes[1].obsolete_factions & 1)==0
          && has_tech_calls==1,59);

    // Reports are parsed before the local path reloads a manual flag set by
    // the parser callback, so that flag suppresses X_pop and retains the item.
    reset();
    setup_prune_queue_case(0);
    manual_flag_target=129;
    manual_flag_on_parse=true;
    prune_protos(2,64,1);
    check(parsed_report_count==2 && production_upgrade_prompts==0,60);
    check((VehPrototypes[129].flags & 0x0208)==0x0208
          && (VehPrototypes[129].obsolete_factions & 4)==0,61);
    check(Bases[0].queue_production_id[0]==129,62);

    // A >1 prompt result jumps straight to queue handling even if X_pop sets
    // the manual flag; if it also flips multiplayer on, native still unlocks
    // and uses the live multiplayer value for the final popup.
    reset();
    setup_prune_queue_case(0);
    manual_flag_target=129;
    manual_flag_on_prompt=true;
    multiplayer_on_prompt=true;
    production_upgrade_choice=2;
    prune_protos(2,64,1);
    check(parsed_report_count==4 && production_upgrade_prompts==1,63);
    check(Bases[0].queue_production_id[0]==128
          && (VehPrototypes[129].obsolete_factions & 4)!=0,64);
    check(NetDaemonNet.locks==0 && NetDaemonNet.unlocks==1
          && popup_calls==1,65);

    // In the native multiplayer branch a sync callback may set flag 0x200,
    // but this path does not parse reports early or enter manual retention.
    reset();
    setup_prune_queue_case(1);
    manual_flag_target=129;
    manual_flag_on_sync=true;
    prune_protos(2,64,1);
    check((VehPrototypes[129].flags & 0x0200)!=0
          && (VehPrototypes[129].obsolete_factions & 4)!=0,66);
    check(Bases[0].queue_production_id[0]==128
          && production_upgrade_prompts==0,67);
    check(parsed_report_count==2 && NetDaemonNet.locks==1
          && NetDaemonNet.unlocks==1 && popup_calls==1,68);

    // 57FEC2..57FED0 accepts matching non-positive armor IDs without
    // entering either normalization/psi call, even for unequal reactors.
    reset();
    setup_prune_queue_case(1);
    VehPrototypes[129].armor_id=5;
    Armor[5].defense_rating=-1;
    prune_protos(2,64,0);
    check(psi_factor_calls==0,69);
    check(Bases[0].queue_production_id[0]==128
          && (VehPrototypes[129].obsolete_factions & 4)!=0,70);

    // The predefined target's saved typed-peer permission is true even
    // without flag 4. A lower-reactor typed peer is therefore retired.
    reset();
    VehPrototypes[1].flags=PROTO_ACTIVE;
    VehPrototypes[2].flags=PROTO_ACTIVE|PROTO_TYPED_COMPLETE;
    VehPrototypes[1].reactor_id=2;
    VehPrototypes[2].reactor_id=1;
    Weapon[0].offense_rating=-1; // Exclude the first predefined walk.
    prune_protos(0,64,0);
    check((VehPrototypes[2].obsolete_factions & 1)!=0,71);

    // The unlocked mode's native epilogue cannot fall through to automatic
    // pruning when a sync callback enables locks and multiplayer is live.
    reset();
    setup_prune_queue_case(1);
    LockEnableMask=0;
    enable_lock_on_sync=true;
    VehPrototypes[128].reactor_id=1;
    VehPrototypes[129].reactor_id=2;
    prune_protos(2,64,1);
    check(synchronized_count>0 && LockEnableMask==4,72);
    check(Bases[0].queue_production_id[0]==129
          && NetDaemonNet.locks==0 && parsed_report_count==0,73);
}
""".replace(
        "LIVE_PRUNE_HELPERS",
        actual_function("prototype_armor_strength")
        + "\n"
        + actual_function("parse_prototype_report"),
    ).replace("LIVE_PRUNE_BODY", function)

def test_live_prune_protos_typed_completion_marking(tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host g++ required")
    cpp = tmp_path / "prune_protos.cpp"
    executable = tmp_path / "prune_protos"
    cpp.write_text(prune_harness(actual_function("prune_protos")))
    result = subprocess.run(
        [compiler, "-std=c++11", "-O1", "-fsanitize=undefined",
         "-fno-sanitize-recover=undefined", str(cpp), "-o", str(executable)],
        capture_output=True, text=True, timeout=30)
    assert result.returncode == 0, result.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
    assert result.returncode == 0, result.stderr

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
    veh_header = (ROOT / "src/veh.h").read_text()
    types = "\n".join(
        braced(veh_header, veh_header.index(marker)) + ";"
        for marker in (
            "struct RulesAbility {", "struct RulesChassis {",
            "struct RulesWeapon {", "struct RulesArmor {",
            "struct RulesReactor {", "struct VehPrototype {",
        )
    )
    source_body = actual_function("name_proto")
    source = (ROOT / "src/veh.cpp").read_text()
    name_helpers = source[
        source.index("static const char *prototype_rule_name"):
        source.index("void __cdecl name_proto")
    ]
    harness = r"""
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#define __cdecl
const int MaxAbilityNum=29;
const int TRIAD_LAND=0, TRIAD_SEA=1, TRIAD_AIR=2;
const int MaxVehProtoFactionNum=64;
const int MaxVehProtoNum=512;
typedef char *LPSTR; typedef int BOOL;
const int ABL_POLICE_2X=0x400000, ABL_TRAINED=0x4000;
const int ABL_ARTILLERY=0x8000;
const int ABL_AIR_SUPERIORITY=0x20, ABL_AMPHIBIOUS=0x8;
const uint16_t PROTO_ACTIVE=1;
SOURCE_TYPES
RulesAbility Ability[29]; RulesChassis Chassis[19];
RulesWeapon Weapon[26]; RulesArmor Armor[14]; RulesReactor Reactor[4];
VehPrototype VehPrototypes[512];
uint32_t LockEnableMask;
int has_abil(int prototype, int ability) {
    return (VehPrototypes[prototype].ability_flags & static_cast<uint32_t>(ability)) != 0;
}
const char *label_get(int slot) {
    if(slot==0x87c/4) return "Artillery1";
    if(slot==0x880/4) return "Artillery2";
    if(slot==0x998/4) return "Marine1";
    if(slot==0x99c/4) return "Marine2";
    return "Unit";
}
int palette_string_value=9, palette_slot_requested=-1;
struct PaletteStub { const void *animation_colours(int slot) const {
    palette_slot_requested=slot;
    return reinterpret_cast<const void *>(static_cast<intptr_t>(palette_string_value));
} } g_PALETTE2;
struct StringTableStub { const char *get(int index) {
    switch(index) {
      case 1: return "Cloak"; case 2: return "Deep Radar";
      case 3: return "Scout"; case 4: return "Impact"; case 5: return "Fission";
      case 6: return "Air"; case 7: return "Trained"; case 8: return "Amph";
      case 9: return "Palette Reactor A";
      case 10: return "Palette Reactor B";
      case 11: return "Rover";
      case 12: return "Shield";
      case 13: return "Guard1";
      case 14: return "Guard2";
      default: return "";
    }
} } StringTable;
NAME_HELPERS
LIVE_BODY
int main() {
    if(std::strcmp(prototype_reactor_name(0),"Palette Reactor A")!=0
        || palette_slot_requested!=4) {
      std::fprintf(stderr,"reactor zero did not read palette animation slot 4\n"); return 47;
    }
    palette_string_value=10;
    if(std::strcmp(prototype_reactor_name(0),"Palette Reactor B")!=0
        || palette_slot_requested!=4) {
      std::fprintf(stderr,"reactor zero did not observe the live palette pointer\n"); return 48;
    }
    palette_string_value=9;
    VehPrototypes[0].reactor_id=1;
    Weapon[0].mode=0; Weapon[0].offense_rating=1;
    Armor[0].defense_rating=2;
    char palette_name[256];
    name_proto(palette_name,0,1,0,0,0,0,0);
    if(palette_slot_requested!=4) {
      std::fprintf(stderr,"name_proto did not use the reactor-zero palette path\n"); return 52;
    }
    palette_string_value=10;
    name_proto(palette_name,0,1,0,0,0,0,0);
    if(palette_slot_requested!=4) {
      std::fprintf(stderr,"name_proto cached the reactor-zero palette pointer\n"); return 53;
    }
    Ability[0].abbreviation=reinterpret_cast<char *>(1);
    Ability[3].abbreviation=reinterpret_cast<char *>(2);
    Chassis[0].offsv_1_name=reinterpret_cast<char *>(3);
    Weapon[0].name_short=reinterpret_cast<char *>(4);
    Armor[0].name_short=reinterpret_cast<char *>(4);
    Reactor[0].name_short=reinterpret_cast<char *>(5);
    Chassis[0].triad=TRIAD_SEA;
    char name[256];
    name_proto(name, 0, 1, 0, 0, 0, (1<<0)|(1<<3), 0);
    if (std::strncmp(name,"Cloak ",6) != 0) {
      std::fprintf(stderr,"unexpected name: %s\n",name); return 42;
    }
    Ability[5].abbreviation=reinterpret_cast<char *>(6);
    Chassis[1].triad=TRIAD_AIR;
    name_proto(name,0,1,1,0,0,1<<5,1);
    if(std::strncmp(name,"Air",3) == 0) { std::fprintf(stderr,"air-triad air-superiority abbreviation was not omitted\n"); return 43; }
    if(std::strstr(name,"Impact")==nullptr) {
      std::fprintf(stderr,"air-superiority distinct name path was not taken: %s\n",name); return 54;
    }

    Ability[15].abbreviation=reinterpret_cast<char *>(7);
    name_proto(name,0,1,0,0,0,1<<15,1);
    if(std::strstr(name,"Trained") != nullptr) { std::fprintf(stderr,"trained abbreviation was not omitted\n"); return 44; }

    Ability[3].abbreviation=reinterpret_cast<char *>(8);
    Chassis[0].triad=TRIAD_LAND;
    Chassis[0].speed=1;
    Weapon[0].offense_rating=2; Armor[0].defense_rating=2;
    name_proto(name,0,1,0,0,0,1<<3,1);
    if(std::strncmp(name,"Amph ",5) == 0) { std::fprintf(stderr,"balanced land amphibious abbreviation was not omitted\n"); return 45; }
    Weapon[0].offense_rating=1;
    name_proto(name,0,1,0,0,0,1<<3,1);
    if(std::strncmp(name,"Amph ",5) != 0) { std::fprintf(stderr,"unexpected amphibious prefix: %s\n",name); return 46; }
    Chassis[0].speed=0;
    name_proto(name,0,1,0,0,0,1<<3,1);
    if(std::strncmp(name,"Amph ",5)!=0) {
      std::fprintf(stderr,"speed-zero ability abbreviation was omitted: %s\n",name); return 55;
    }

    Chassis[3].triad=TRIAD_LAND;
    Weapon[2].name_short=reinterpret_cast<char *>(4);
    Weapon[2].mode=9; Weapon[2].offense_rating=2;
    name_proto(name,0,1,3,2,0,1,1);
    if(std::strncmp(name,"Cloak Impact",12)!=0) {
      std::fprintf(stderr,"mode-nine land prefix skip mismatch: %s\n",name); return 56;
    }

    Chassis[2].triad=TRIAD_LAND; Chassis[2].speed=2;
    Weapon[0].offense_rating=1; Armor[0].defense_rating=1;
    VehPrototypes[0].ability_flags=ABL_POLICE_2X;
    name_proto(name,0,1,2,0,0,1,1);
    if(std::strncmp(name,"Cloa",4) != 0) { std::fprintf(stderr,"Police 2X prefix trim mismatch: %s\n",name); return 49; }

    VehPrototypes[0].ability_flags=0;
    Chassis[0].triad=TRIAD_LAND; Chassis[0].speed=0;
    Weapon[0].offense_rating=2; Weapon[0].mode=0;
    Armor[0].defense_rating=2;
    char duplicate_name[64]="Impact Scout";
    bool suffix_exhausted=false;
    add_prototype_name_suffix(duplicate_name,"Unit",suffix_exhausted);
    if(std::strcmp(duplicate_name,"Impact Scout Unit2") != 0) return 50;
    add_prototype_name_suffix(duplicate_name,"Unit",suffix_exhausted);
    if(std::strcmp(duplicate_name,"Impact Scout Unit3") != 0) return 51;

    // 57EA8C tests the artillery bit, not the distinct trained bit. The
    // alternate label is appended once on each special suffix route.
    Chassis[0].offsv_2_name=reinterpret_cast<char *>(11);
    name_proto(name,0,0,0,0,0,ABL_TRAINED,1);
    if(std::strcmp(name,"Impact Scout")) return 57;
    name_proto(name,0,0,0,0,0,ABL_ARTILLERY,1);
    if(std::strcmp(name,"Impact Artillery1")) return 58;
    name_proto(name,0,1,0,0,0,ABL_ARTILLERY,1);
    if(std::strcmp(name,"Impact Artillery2")) return 59;

    // 57EAEA tests speed==1 independently of triad. No abbreviation is
    // emitted for this stronger weapon's amphibious flag at speed one.
    Chassis[0].speed=1; Chassis[0].triad=TRIAD_SEA;
    Weapon[0].offense_rating=3;
    name_proto(name,0,0,0,0,0,ABL_AMPHIBIOUS,1);
    if(std::strcmp(name,"Impact Marine2")) return 60;

    // The missile terminals append their alternate weapon/chassis once;
    // mode>=3 exits without adding a defensive chassis suffix.
    Chassis[0].missile=1;
    Weapon[24].name_short=reinterpret_cast<char *>(4);
    Weapon[24].offense_rating=98; Weapon[24].mode=3;
    name_proto(name,0,1,0,24,0,0,1);
    if(std::strcmp(name,"Impact Fission Scout")) return 61;
    Weapon[24].offense_rating=99;
    name_proto(name,0,1,0,24,0,0,1);
    if(std::strcmp(name,"Fission Impact")) return 62;

    // 57E4F4 uses Armor.name_short, not Weapon.name_short. Reactor
    // selection scans the live named prototype's reactor, not the argument.
    Chassis[0].missile=0; Chassis[0].speed=0;
    Chassis[0].triad=TRIAD_LAND;
    Chassis[0].defsv_1_name=reinterpret_cast<char *>(13);
    Chassis[0].defsv_2_name=reinterpret_cast<char *>(14);
    Armor[0].name_short=reinterpret_cast<char *>(12);
    Armor[0].defense_rating=4;
    Weapon[0].offense_rating=2;
    VehPrototypes[0].reactor_id=2;
    VehPrototypes[64].flags=PROTO_ACTIVE;
    VehPrototypes[64].reactor_id=1;
    name_proto(name,0,1,0,0,0,0,1);
    if(std::strcmp(name,"Fission Guard2")) return 63;
    VehPrototypes[64].reactor_id=2;
    name_proto(name,0,1,0,0,0,0,1);
    if(std::strcmp(name,"Shield Guard2")) return 64;

    // 57E5F5 skips the component for a unique chassis form. 57E6C5
    // chooses the alternate form using prototype parity, not faction parity.
    VehPrototypes[64].flags=0;
    Chassis[1].triad=TRIAD_AIR; Chassis[1].speed=2;
    Chassis[1].defsv_1_name=reinterpret_cast<char *>(13);
    Chassis[1].defsv_2_name=reinterpret_cast<char *>(14);
    Armor[0].defense_rating=1;
    VehPrototypes[0].unk_1=10;
    name_proto(name,0,1,1,0,0,ABL_AIR_SUPERIORITY,1);
    if(std::strcmp(name,"Unit Guard2")) return 65;
    VehPrototypes[1].unk_1=10;
    name_proto(name,1,1,1,0,0,ABL_AIR_SUPERIORITY,1);
    if(std::strcmp(name,"Unit Guard1")) return 66;
}
""".replace("SOURCE_TYPES", types).replace("NAME_HELPERS", name_helpers).replace(
        "LIVE_BODY", source_body)
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
void name_proto(char *out,int,int,int,int,int,int,int) { std::strcpy(out,"Generated"); }
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

    // The next inactive faction-local slot outranks ordinary predefined
    // candidates. Queue rewriting covers the active slot through queue_size
    // inclusively and replaces every obsolete prototype in the category.
    DesignConsiderOverride=0; DesignConsiderPrototype=-1;
    Chassis[1].speed=2; Armor[1].defense_rating=2; Weapon[1].offense_rating=2;
    BaseCurrentCount=1; Bases[0].faction_id_current=1; Bases[0].queue_size=3;
    Bases[0].queue_production_id[0]=70;
    Bases[0].queue_production_id[1]=-69;
    Bases[0].queue_production_id[2]=71;
    Bases[0].queue_production_id[3]=72;
    for(int slot=70;slot<=72;++slot) {
      VehPrototypes[slot].flags=PROTO_ACTIVE;
      VehPrototypes[slot].obsolete_factions=2;
      VehPrototypes[slot].unk_1=4;
      PlayersData[1].proto_id_queue[slot]=1;
    }
    const int allocated=propose_proto(1,1,1,1,0,1,4,nullptr);
    if(allocated!=64 || Bases[0].queue_production_id[0]!=64
       || Bases[0].queue_production_id[1]!=-69
       || Bases[0].queue_production_id[2]!=64
       || Bases[0].queue_production_id[3]!=64
       || (VehPrototypes[64].flags & 0x40)==0) {
      std::fprintf(stderr,"prototype allocation/queue rewrite mismatch: %d\n",allocated);
      return 44;
    }

    std::memset(VehPrototypes,0,sizeof(VehPrototypes));
    BaseCurrentCount=0; FirstProposedPrototype=-1;
    std::strcpy(VehPrototypes[2].veh_name,"Scout");
    char requested_name[]="Scout";
    const int named=propose_proto(1,1,1,1,0,1,4,requested_name);
    if(named!=64 || std::strcmp(VehPrototypes[64].veh_name,"Scout*")!=0) {
      std::fprintf(stderr,"duplicate custom-name suffix mismatch: %d %s\n",
                   named,VehPrototypes[64].veh_name);
      return 47;
    }

    // The native obsolete slot with no live or queued units gets a category
    // score (same-category 0x2000, other-category 0x1000). With a queued
    // reference instead, the separate obsolete/zero-active branch scores
    // 0x400, independent of category.
    std::memset(VehPrototypes,0,sizeof(VehPrototypes));
    std::memset(PlayersData,0,sizeof(PlayersData));
    BaseCurrentCount=0; FirstProposedPrototype=-1;
    for(int slot=0;slot<128;++slot) {
      VehPrototypes[slot].flags=PROTO_ACTIVE;
      PlayersData[1].proto_id_active[slot]=1;
      PlayersData[1].proto_id_queue[slot]=1;
    }
    VehPrototypes[64].obsolete_factions=2;
    VehPrototypes[64].unk_1=7;
    PlayersData[1].proto_id_active[64]=0;
    const int obsolete_queued=propose_proto(1,1,1,1,0,1,4,nullptr);
    if(obsolete_queued!=64) {
      std::fprintf(stderr,"obsolete queued-slot score mismatch: %d\n",obsolete_queued);
      return 45;
    }

    std::memset(VehPrototypes,0,sizeof(VehPrototypes));
    std::memset(PlayersData,0,sizeof(PlayersData));
    for(int slot=0;slot<128;++slot) {
      VehPrototypes[slot].flags=PROTO_ACTIVE;
      PlayersData[1].proto_id_active[slot]=1;
      PlayersData[1].proto_id_queue[slot]=1;
    }
    VehPrototypes[64].obsolete_factions=2;
    VehPrototypes[64].unk_1=7;
    PlayersData[1].proto_id_active[64]=0;
    PlayersData[1].proto_id_queue[64]=0;
    VehPrototypes[65].obsolete_factions=2;
    VehPrototypes[65].unk_1=4;
    PlayersData[1].proto_id_active[65]=0;
    PlayersData[1].proto_id_queue[65]=0;
    const int category_score=propose_proto(1,1,1,1,0,1,4,nullptr);
    if(category_score!=65) {
      std::fprintf(stderr,"obsolete category-score ordering mismatch: %d\n",category_score);
      return 46;
    }
    if((VehPrototypes[65].flags & 0x40)!=0) {
      std::fprintf(stderr,"active reused prototype incorrectly gained flag 0x40\n"); return 49;
    }

    // The similarity fast path compares the proposed offense factor, not the
    // faction id that shares the original stack slot before rating setup.
    std::memset(VehPrototypes,0,sizeof(VehPrototypes));
    std::memset(PlayersData,0,sizeof(PlayersData));
    DesignConsiderOverride=0; DesignConsiderPrototype=-1;
    Chassis[1].speed=2; Weapon[1].offense_rating=2; Armor[1].defense_rating=2;
    Weapon[2].offense_rating=3; Armor[2].defense_rating=1;
    VehPrototypes[64].flags=PROTO_ACTIVE;
    VehPrototypes[64].chassis_id=1; VehPrototypes[64].weapon_id=2;
    VehPrototypes[64].armor_id=2; VehPrototypes[64].reactor_id=1;
    PlayersData[1].proto_id_active[64]=1;
    PlayersData[1].proto_id_queue[64]=1;
    const int similar=propose_proto(1,1,1,1,0,1,4,nullptr);
    if(similar!=64) {
      std::fprintf(stderr,"proposal similarity-factor comparison mismatch: %d\n",similar);
      return 48;
    }

    // Queued obsolete designs still take compatible reuse before the 0x400
    // allocation score. Reuse clears obsolescence without make/name/prune.
    std::memset(VehPrototypes,0,sizeof(VehPrototypes));
    std::memset(PlayersData,0,sizeof(PlayersData));
    VehPrototypes[64].flags=PROTO_ACTIVE;
    VehPrototypes[64].obsolete_factions=2;
    VehPrototypes[64].chassis_id=1; VehPrototypes[64].weapon_id=1;
    VehPrototypes[64].armor_id=1; VehPrototypes[64].reactor_id=1;
    PlayersData[1].proto_id_queue[64]=1;
    const int compatible=propose_proto(1,1,1,1,0,1,4,nullptr);
    if(compatible!=64 || VehPrototypes[64].flags!=PROTO_ACTIVE
       || VehPrototypes[64].obsolete_factions!=0) return 50;

    // All faction slots are occupied by other categories. Active predefined
    // designs cannot become allocation candidates even with a matching
    // category and no active/queued instances.
    std::memset(VehPrototypes,0,sizeof(VehPrototypes));
    std::memset(PlayersData,0,sizeof(PlayersData));
    for(int slot=0;slot<128;++slot) {
      VehPrototypes[slot].flags=PROTO_ACTIVE;
      VehPrototypes[slot].unk_1=slot<64?4:7;
      if(slot>=64) {
        PlayersData[1].proto_id_active[slot]=1;
        PlayersData[1].proto_id_queue[slot]=1;
      }
    }
    const int unavailable=propose_proto(1,1,1,1,0,1,4,nullptr);
    if(unavailable!=-1) return 51;
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
