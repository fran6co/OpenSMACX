"""Actual-source C++98 behavior and damage controls for game/vehicle owners.

The repository root is derived from the recovery/tests location. It embeds its own mock fixtures and never reads the
private candidate snippets or ignored fixture files at runtime.
"""
import pathlib
import shutil
import subprocess

import pytest


ROOT = pathlib.Path(__file__).resolve().parents[2]

BATCH_FIXTURE = r"""#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef int BOOL;
typedef const char *LPCSTR;
enum { BSTATE_ARTIFACT_LINKED = 0x400, BSTATE_ARTIFACT_ALREADY_LINKED = 0x800, RFLAG_ALIEN = 0x80 };
const int MaxTechnologyNum = 89;
struct Veh { short x,y; uint32_t state; uint16_t flags; short proto_id,unk; unsigned char faction_id,lurking,damage; signed char order; unsigned char waypoint_count,patrol; short waypoint_x[4],waypoint_y[4]; unsigned char morale; };
struct Base { short x,y; unsigned char faction_id_current,faction_id_former; signed char population_size; unsigned char assimilation,staple,unknown,visibility,faction_pop_size_intel[8]; char name_string[25]; short unk_x,unk_y; uint32_t state,event,governor; int nutrients,minerals,production,eco_damage,queue_size,queue_production_id[10],resource_sqr_radius,specialist_total,specialist_unknown; uint32_t spec[2]; unsigned char facilities_built[12]; };
struct PlayerData { uint32_t flags,ranking; int energy_reserves; };
struct Player { uint32_t rule_flags; char filename[24]; };
struct RulesTechnology { uint32_t flags; char *name; char id[8]; int pad,growth,value,wealth,power,preq_tech_1,preq_tech_2; };
struct BaseSecretProject { int fields[64]; int &universal_translator_; BaseSecretProject() : universal_translator_(fields[20]) {}
 int &universal_translator() { return universal_translator_; }
 const int &universal_translator() const { return universal_translator_; }
};
struct Console { int faction_id_; };
struct InfoWindow { void on_redraw(); };
struct NetMsg { void pop(const char *,int,int,const char *); };
struct FX { void play(int); void fade(int); };
struct GraphicWin { void redraw(); };
struct WorldWin : GraphicWin {};
Veh Vehs[2049]; Base Bases[512]; PlayerData PlayersData[8]; Player Players[8];
RulesTechnology Technology[MaxTechnologyNum]; BaseSecretProject SecretProject;
Console ConsoleGlobal; InfoWindow ConsoleInfoWin; NetMsg NetMsgPrimary; FX g_FX;
WorldWin WorldClimateWorldWin; int BaseCurrentCount,TurnCurrentNum; uint32_t MapLongitude;
struct Map { uint8_t pad[12]; uint32_t bit2; }; Map MapTiles[2048];
char StringTemp[1032]; uint32_t UnkBitfield1;
char ScriptPopupFileStorage[]="CLIMATE"; char *ScriptPopupFile=ScriptPopupFileStorage;
int tech_has[89][8],tech_is_available[89][8],tech_leap_valid[89][8];
int calls[32],call_count,deterministic_roll;
static void note(int id) { calls[call_count++]=id; }
int base_at(int x,int y) { note(1); for(int i=0;i<BaseCurrentCount;i++) if(Bases[i].x==x&&Bases[i].y==y)return i; return -1; }
void parse_says(int,LPCSTR,int,int) { note(2); }
void NetMsg::pop(const char *,int,int,const char *) { note(3); }
void veh_kill(int id) { note(4); for(int i=id;i<2048;i++)Vehs[i]=Vehs[i+1]; }
void draw_tile(int,int,int) { note(5); }
void FX::play(int) { note(6); } void FX::fade(int) { note(7); }
int popp(const char *,const char *,int,const char *,int(*)()) { note(8); return 0; }
BOOL has_tech(int t,int f) { return t>=0&&t<89&&tech_has[t][f]; }
BOOL tech_avail(int t,int f) { return t>=0&&t<89&&tech_is_available[t][f]; }
BOOL valid_tech_leap(int t,int f) { return tech_leap_valid[t][f]; }
void tech_achieved(int,int,int,int) { note(9); }
int veh_init(int p,int f,int x,int y) { note(10); Veh &v=Vehs[2047]; v.x=(short)x;v.y=(short)y;v.proto_id=(short)p;v.faction_id=(unsigned char)f;return 2047; }
void parse_num(int,int) { note(11); } void InfoWindow::on_redraw() { note(12); }
void bitmask(int,int *o,int *m) { *o=0;*m=1;note(13); }
int spot_count,grow_base_count_on_spot;
void spot_loc(int,int,int) { ++spot_count;note(14);if(grow_base_count_on_spot){++BaseCurrentCount;grow_base_count_on_spot=0;} }
void draw_map(int) { note(15); } void GraphicWin::redraw() { note(16); }
char *CharUpperA(char *s) { for(char *p=s;*p;++p)if(*p>='a'&&*p<='z')*p-=32;return s; }
int fake_rand() { return deterministic_roll; }
#define __cdecl
#define rand fake_rand
@@STUDY_ARTIFACT@@
@@DO_UNITY_CRASH@@
#undef rand
#undef __cdecl
static void reset_calls() { call_count=0;spot_count=0; }
static void study_linked_alert_preserves_gate_and_update_order() {
 reset_calls();BaseCurrentCount=1;ConsoleGlobal.faction_id_=2;Vehs[1].x=4;Vehs[1].y=6;Vehs[1].faction_id=2;Bases[0].x=4;Bases[0].y=6;strcpy(Bases[0].name_string,"ALPHA");Bases[0].state=BSTATE_ARTIFACT_LINKED;SecretProject.universal_translator()=-1;
 assert(study_artifact(1)==0);assert((Bases[0].state&BSTATE_ARTIFACT_ALREADY_LINKED)!=0);assert(call_count==3&&calls[0]==1&&calls[1]==2&&calls[2]==3);reset_calls();assert(study_artifact(1)==0);assert(call_count==1&&calls[0]==1);
}
static void study_rejects_translator_base_then_consumes_artifact_and_awards() {
 reset_calls();BaseCurrentCount=1;ConsoleGlobal.faction_id_=2;Vehs[1].x=4;Vehs[1].y=6;Vehs[1].faction_id=2;Bases[0].x=4;Bases[0].y=6;Bases[0].state=0;SecretProject.universal_translator()=0;
 assert(study_artifact(1)==1);assert(call_count==5);assert(calls[0]==1&&calls[1]==4&&calls[2]==5&&calls[3]==6&&calls[4]==8);
}
static void study_selects_only_an_available_leap_tech() {
 reset_calls();BaseCurrentCount=0;ConsoleGlobal.faction_id_=2;Vehs[1].x=4;Vehs[1].y=6;Vehs[1].faction_id=2;SecretProject.universal_translator()=-1;PlayersData[2].ranking=6;tech_has[1][2]=1;tech_is_available[0][2]=1;tech_leap_valid[0][2]=1;Technology[0].preq_tech_1=1;Technology[0].preq_tech_2=-1;deterministic_roll=3;
 assert(study_artifact(1)==1);assert(call_count==6&&calls[5]==9);assert((PlayersData[2].flags&0x10)!=0);tech_has[1][2]=0;tech_is_available[0][2]=0;tech_leap_valid[0][2]=0;deterministic_roll=0;
}
static void unity_event_gates_and_spawn() {
 reset_calls();MapLongitude=32;ConsoleGlobal.faction_id_=3;MapTiles[32*2+5].bit2=0;assert(do_unity_crash(10,2,3)==0&&call_count==0);MapTiles[32*2+5].bit2=0x14004000;assert(do_unity_crash(10,2,3)==0&&call_count==0);MapTiles[32*2+5].bit2=0x08004000;assert(do_unity_crash(10,2,3)==1);assert(call_count==4&&calls[0]==6&&calls[1]==3&&calls[2]==7&&calls[3]==10);assert(Vehs[2047].proto_id==12&&Vehs[2047].morale==2);
}
static void unity_energy_uses_turn_boundary_and_redraws_before_local_notice() {
 reset_calls();BaseCurrentCount=0;TurnCurrentNum=50;ConsoleGlobal.faction_id_=4;PlayersData[4].energy_reserves=10;MapTiles[0].bit2=0x0A004000;assert(do_unity_crash(0,0,4)==1);assert(PlayersData[4].energy_reserves==310);assert(call_count==5&&calls[0]==11&&calls[1]==12&&calls[2]==6&&calls[3]==3&&calls[4]==7);reset_calls();TurnCurrentNum=49;PlayersData[5].energy_reserves=0;assert(do_unity_crash(0,0,5)==1);assert(PlayersData[5].energy_reserves==150&&call_count==2);
}
static void unity_radar_reloads_base_count_and_filters_native_and_fungboy() {
 reset_calls();BaseCurrentCount=2;ConsoleGlobal.faction_id_=1;strcpy(Players[2].filename,"fungboy");strcpy(Players[3].filename,"spartans");Bases[0].faction_id_current=2;Bases[0].facilities_built[0]=1;Bases[1].faction_id_current=3;Bases[1].facilities_built[0]=1;Bases[2].faction_id_current=3;Bases[2].facilities_built[0]=1;grow_base_count_on_spot=1;MapTiles[0].bit2=0x13004000;
 assert(do_unity_crash(0,0,1)==1);assert(spot_count==2&&calls[0]==13&&calls[1]==13&&calls[2]==14);assert(calls[3]==13&&calls[4]==14&&calls[5]==15&&calls[6]==16&&calls[7]==6&&calls[8]==3&&calls[9]==7);assert((UnkBitfield1&4)!=0);
}
int main() { study_linked_alert_preserves_gate_and_update_order();study_rejects_translator_base_then_consumes_artifact_and_awards();study_selects_only_an_available_leap_tech();unity_event_gates_and_spawn();unity_energy_uses_turn_boundary_and_redraws_before_local_notice();unity_radar_reloads_base_count_and_filters_native_and_fungboy(); }
"""

SCENARIO_FIXTURE = r"""#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define __cdecl
#define rand scenario_rand

enum { TRIAD_LAND = 0, TRIAD_SEA = 1, TRIAD_AIR = 2 };
enum { VFLAG_START_RAND_LOCATION = 0x80,
       VFLAG_START_RAND_MONOLITH = 0x100,
       VFLAG_START_RAND_FUNGUS = 0x200 };
enum { BIT_FUNGUS = 0x20, BIT_MONOLITH = 0x2000 };

struct Veh {
    short x, y;
    unsigned state;
    unsigned short flags;
    short proto_id;
    unsigned short unk_1;
    unsigned char faction_id;
    unsigned char pad[0x34 - 15];
};
struct VehPrototype {
    unsigned char pad[0x20];
    unsigned char chassis_id;
    unsigned char tail[0x13];
};
struct RulesChassis {
    unsigned char pad[0x49];
    unsigned char triad;
    unsigned char tail[0x90 - 0x4A];
};
struct Map {
    unsigned char climate, contour, val2, region, visibility, val3, unk_1;
    signed char territory;
    unsigned bit, bit2, bit_visible[7];
};
typedef char VehSizeCheck[(sizeof(Veh) == 0x34) ? 1 : -1];
typedef char ProtoSizeCheck[(sizeof(VehPrototype) == 0x34) ? 1 : -1];
typedef char ChassisSizeCheck[(sizeof(RulesChassis) == 0x90) ? 1 : -1];
typedef char MapSizeCheck[(sizeof(Map) == 0x2C) ? 1 : -1];

static int VehCurrentCount, MapLongitudeBounds, MapLongitude;
static int MapLatitudeBounds, MapIsFlat, BaseFindDist;
static Veh Vehs[4];
static VehPrototype VehPrototypes[4];
static RulesChassis Chassis[4];
static int map_pointer_count, last_map_offset;
struct MapStorage {
    Map values[256];
    Map *operator+(int offset) {
        ++map_pointer_count;
        last_map_offset = offset;
        return values + offset;
    }
    Map &operator[](int offset) { return values[offset]; }
};
static MapStorage MapTiles;
static int lift_count, drop_count, kill_count, base_find_count, stack_count;
static int veh_at_count, occupancy_mode, scenario_rand_values[4], rand_index;
static int dropped_id, dropped_x, dropped_y;

int scenario_rand() { return scenario_rand_values[rand_index++ % 4]; }
int veh_lift(int) { ++lift_count; return 0; }
int veh_drop(int id, int x, int y) {
    ++drop_count;
    dropped_id = id;
    dropped_x = x;
    dropped_y = y;
    return 0;
}
int veh_at(int, int) { ++veh_at_count; return occupancy_mode ? 1 : -1; }
int stack_check(int, int, int, int, int) { ++stack_count; return 1; }
int base_find(int, int) { ++base_find_count; return BaseFindDist; }
void veh_kill(int) { ++kill_count; }

@@SCENARIO_SETUP@@

static void reset_case() {
    VehCurrentCount = 1;
    MapLongitudeBounds = 12;
    MapLongitude = 6; // Deliberately not the same as the x-coordinate bound.
    MapLatitudeBounds = 12;
    MapIsFlat = 0;
    BaseFindDist = 40;
    memset(Vehs, 0, sizeof Vehs);
    memset(VehPrototypes, 0, sizeof VehPrototypes);
    memset(Chassis, 0, sizeof Chassis);
    memset(&MapTiles, 0, sizeof MapTiles);
    map_pointer_count = last_map_offset = 0;
    lift_count = drop_count = kill_count = base_find_count = 0;
    stack_count = veh_at_count = occupancy_mode = rand_index = 0;
    dropped_id = dropped_x = dropped_y = -1;
    for (int i = 0; i < 4; ++i) scenario_rand_values[i] = 0;
    Vehs[0].flags = VFLAG_START_RAND_LOCATION;
    Vehs[0].faction_id = 1;
    Vehs[0].proto_id = 0;
    Chassis[0].triad = TRIAD_AIR;
}

static void unchanged_placement_and_air_map_access() {
    reset_case();
    scenario_setup();
    assert(lift_count == 1 && drop_count == 1 && kill_count == 0);
    assert(dropped_id == 0 && dropped_x == 0 && dropped_y == 2);
    assert(base_find_count == 1 && veh_at_count == 1);
    assert(map_pointer_count == 0); // Air + no terrain flags reads no Map tile.
}

static void map_rows_use_half_longitude_for_each_tile_read() {
    reset_case();
    Chassis[0].triad = TRIAD_LAND;
    MapTiles[12].climate = 0x80; // MapLongitude(6) * y(2) + x/2(0).
    MapTiles[24].climate = 0x20; // Bounds(12) stride would reject repeatedly.
    scenario_setup();
    assert(rand_index == 2 && last_map_offset == 12);

    reset_case();
    Vehs[0].flags |= VFLAG_START_RAND_MONOLITH;
    MapTiles[12].bit = BIT_MONOLITH;
    scenario_setup();
    assert(rand_index == 2 && last_map_offset == 12);

    reset_case();
    Vehs[0].flags |= VFLAG_START_RAND_FUNGUS;
    MapTiles[12].bit = BIT_FUNGUS;
    MapTiles[12].climate = 0x80;
    scenario_setup();
    assert(rand_index == 2 && last_map_offset == 12);
}

static void retries_occupancy_and_native_wrap_threshold() {
    reset_case();
    Vehs[0].flags = 0;
    scenario_setup();
    assert(lift_count == 0 && drop_count == 0 && kill_count == 0);

    reset_case();
    scenario_rand_values[0] = 1;
    scenario_rand_values[1] = 1;
    scenario_setup();
    assert(dropped_x == 1 && dropped_y == 3);

    reset_case();
    VehCurrentCount = 2;
    Vehs[1].faction_id = 2;
    occupancy_mode = 1;
    scenario_setup();
    assert(lift_count == 1 && drop_count == 1 && kill_count == 1);
    assert(veh_at_count == 10000 && base_find_count == 0);

    reset_case();
    VehCurrentCount = 2;
    Vehs[1].x = 8;
    Vehs[1].y = 2;
    BaseFindDist = 100;
    scenario_setup();
    // dx 8 wraps because MapLongitude is 6; subtract the full width 12.
    assert(rand_index == 6002);
}

int main() {
    unchanged_placement_and_air_map_access();
    map_rows_use_half_longitude_for_each_tile_read();
    retries_occupancy_and_native_wrap_threshold();
    return 0;
}
"""


BODY_CASES = (
    ("study_artifact", ROOT / "src" / "veh.cpp", "int __cdecl study_artifact(int veh_id)"),
    ("do_unity_crash", ROOT / "src" / "game.cpp", "int __cdecl do_unity_crash(int x, int y, int faction_id)"),
    ("scenario_setup", ROOT / "src" / "game.cpp", "void __cdecl scenario_setup()"),
)


def extract_body(text, signature):
    start = text.find(signature)
    assert start >= 0, "actual product body is not integrated: " + signature
    brace = text.find("{", start)
    assert brace >= 0, "definition has no body: " + signature
    depth, state, escaped, cursor = 0, "code", False, brace
    while cursor < len(text):
        ch = text[cursor]
        nxt = text[cursor + 1] if cursor + 1 < len(text) else ""
        if state == "line":
            if ch == "\n": state = "code"
        elif state == "block":
            if ch == "*" and nxt == "/": state = "code"; cursor += 1
        elif state in ("string", "char"):
            if escaped: escaped = False
            elif ch == "\\": escaped = True
            elif (state == "string" and ch == '"') or (state == "char" and ch == "'"): state = "code"
        elif ch == "/" and nxt == "/": state = "line"; cursor += 1
        elif ch == "/" and nxt == "*": state = "block"; cursor += 1
        elif ch == '"': state = "string"
        elif ch == "'": state = "char"
        elif ch == "{": depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0: return text[start:cursor + 1]
        cursor += 1
    raise AssertionError("unbalanced definition: " + signature)


def compiler_controls():
    found = [shutil.which(name) for name in ("g++", "clang++")]
    return [path for path in found if path]


@pytest.mark.parametrize("name,owner,signature", BODY_CASES)
def test_actual_product_owner_body_cpp98(tmp_path, name, owner, signature):
    compilers = compiler_controls()
    if not compilers:
        pytest.skip("neither g++ nor clang++ is installed")
    damage_variants = ()
    if name == "scenario_setup":
        body = extract_body(owner.read_text(), signature)
        source_text = SCENARIO_FIXTURE.replace("@@SCENARIO_SETUP@@", body)
        stride_before = "MapTiles + MapLongitude * y"
        threshold_before = "dx > MapLongitude)"
        assert stride_before in body, "scenario_setup row-stride expression changed; refresh damage control"
        assert threshold_before in body, "scenario_setup wrap-threshold expression changed; refresh damage control"
        damage_variants = (
            ("old-stride", source_text.replace(stride_before,
                                                "MapTiles + MapLongitudeBounds * y")),
            ("old-wrap-threshold", source_text.replace(threshold_before,
                                                       "dx > MapLongitudeBounds)")),
        )
    else:
        # The batch mock fixture exercises both bodies together.
        artifact = extract_body((ROOT / "src" / "veh.cpp").read_text(), BODY_CASES[0][2])
        unity = extract_body((ROOT / "src" / "game.cpp").read_text(), BODY_CASES[1][2])
        source_text = BATCH_FIXTURE.replace("@@STUDY_ARTIFACT@@", artifact).replace("@@DO_UNITY_CRASH@@", unity)
    assert "@@" not in source_text, "fixture body placeholder was not replaced"
    source = tmp_path / (name + "_actual_body.cpp")
    source.write_text(source_text)
    for compiler in compilers:
        binary = tmp_path / (name + "-" + pathlib.Path(compiler).name)
        subprocess.run([compiler,"-std=c++98","-Wall","-Wextra","-pedantic",str(source),"-o",str(binary)],check=True,timeout=30)
        subprocess.run([str(binary)],check=True,timeout=30)
        for damage_name, damaged_text in damage_variants:
            damaged_source = tmp_path / (name + "-" + damage_name + ".cpp")
            damaged_source.write_text(damaged_text)
            damaged_binary = tmp_path / (name + "-" + damage_name + "-" + pathlib.Path(compiler).name)
            subprocess.run([compiler,"-std=c++98","-Wall","-Wextra","-pedantic",str(damaged_source),"-o",str(damaged_binary)],check=True,timeout=30)
            result = subprocess.run([str(damaged_binary)],check=False,timeout=30)
            assert result.returncode != 0, damage_name + " damage escaped scenario_setup controls"
