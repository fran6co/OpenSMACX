"""Actual-source host controls, not native ABI or game acceptance."""
from pathlib import Path
import re
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]
TESTS = ROOT / "recovery/tests/random_events"
from match_recovery.source import Mode, read_file

COMPILERS = [c for c in ("g++", "clang++") if shutil.which(c)]
FIXTURES = (
    "cases_0_5.cpp", "case6.cpp", "cases_7_8.cpp", "cases_9_11.cpp",
    "cases_12_13.cpp", "cases_15_16.cpp", "cases_17_18.cpp",
    "cases_19_21.cpp", "case14.cpp", "native_services.cpp",
)


def masked(source):
    chars = list(source); i = 0; state = "code"; quote = ""
    while i < len(source):
        c = source[i]; n = source[i + 1] if i + 1 < len(source) else ""
        if state == "code":
            if c == "/" and n == "/":
                chars[i] = chars[i + 1] = " "; i += 2; state = "line"; continue
            if c == "/" and n == "*":
                chars[i] = chars[i + 1] = " "; i += 2; state = "block"; continue
            if c in ("'", '"'):
                quote = c; chars[i] = " "; state = "quote"
        elif state == "line":
            if c == "\n": state = "code"
            else: chars[i] = " "
        elif state == "block":
            if c == "*" and n == "/":
                chars[i] = chars[i + 1] = " "; i += 2; state = "code"; continue
            if c != "\n": chars[i] = " "
        else:
            if c == "\\":
                chars[i] = " "
                if i + 1 < len(source) and source[i + 1] != "\n":
                    chars[i + 1] = " "; i += 2; continue
            elif c == quote: chars[i] = " "; state = "code"
            elif c != "\n": chars[i] = " "
        i += 1
    return "".join(chars)


def block(source, start, op="{", cl="}"):
    code = masked(source); opening = code.find(op, start); depth = 0
    assert opening >= 0
    for pos in range(opening, len(code)):
        if code[pos] == op: depth += 1
        elif code[pos] == cl:
            depth -= 1
            if not depth:
                end = pos + 1
                if cl == "}" and source[end:end + 1] == ";": end += 1
                return source[start:end]
    raise AssertionError("unclosed source block")


def function(source, name):
    code = masked(source)
    for match in re.finditer(r"\b" + re.escape(name) + r"\s*\(", code):
        p = code.find("(", match.start()); depth = 0
        for q in range(p, len(code)):
            if code[q] == "(": depth += 1
            elif code[q] == ")":
                depth -= 1
                if not depth: break
        brace = q + 1
        while brace < len(code) and code[brace].isspace(): brace += 1
        if brace == len(code) or code[brace] != "{": continue
        start = source.rfind("\n", 0, match.start()) + 1
        prior = source[source.rfind("\n", 0, start - 1) + 1:start].strip()
        if prior.startswith("template <"):
            start = source.rfind("\n", 0, start - 1) + 1
        return block(source, start)
    raise AssertionError("source definition missing: " + name)


def struct(source, name):
    code = masked(source)
    match = re.search(r"\bstruct\s+" + re.escape(name) + r"\s*\{", code)
    assert match, name
    return block(source, match.start())


def guarded(source, macro):
    lines = source.splitlines(True)
    start = next(i for i, line in enumerate(lines)
                 if line.strip() == "#ifndef " + macro)
    depth = 0
    for i in range(start, len(lines)):
        item = lines[i].lstrip()
        if item.startswith(("#if ", "#ifdef ", "#ifndef ")): depth += 1
        elif item.startswith("#endif"):
            depth -= 1
            if depth == 0: return "".join(lines[start:i + 1])
    raise AssertionError("unterminated guard: " + macro)


def source_fragments():
    path = ROOT / "src/game.cpp"
    text = path.read_text(encoding="utf-8")
    rows = [r for r in read_file(path) if r.address == 0x51F2E0]
    assert len(rows) == 1 and rows[0].mode is Mode.BODY
    assert "BODY GOES HERE." not in rows[0].region
    assert "random_events_candidate" not in rows[0].region
    result = {
        "actual_cases_0_5.inc": function(text, "random_events_cases_0_5"),
        "actual_case6.hpp": guarded(text, "RANDOM_EVENTS_CASE6_HPP"),
        "actual_cases_7_8.hpp": guarded(text, "RANDOM_EVENTS_CASES_7_8_POSITION_HPP"),
        "actual_cases_9_11.hpp": guarded(text, "RANDOM_EVENTS_CASES_9_11_HPP"),
        "actual_cases_12_13.inc": function(text, "random_events_cases_12_13"),
        "actual_blight_cache.inc": function(text, "random_events_blight_cache_word"),
        "actual_cases_15_16.inc": function(text, "random_events_cases_15_16"),
        "actual_cases_17_18.inc": function(text, "random_events_cases_17_18"),
        "actual_ceil_quarter.inc": function(text, "random_events_ceil_quarter"),
        "actual_cases_19_21.inc": function(text, "random_events_cases_19_21"),
        "actual_case14.inc": function(text, "random_events_case14"),
        "actual_native_services.inc": "\n".join([*(struct(text, n) for n in (
            "RandomEventsNativePlayerView", "RandomEventsNativeState9_11",
            "RandomEventsNativeServices9_11", "RandomEventsNativePositionServices",
            "RandomEventsNativeCase6State", "RandomEventsNativeCase6Services")),
            function(text, "random_events_native_cases_9_11")]),
        "actual_dispatch_input.inc": struct(text, "RandomEventsDispatchInput"),
        "actual_entry_prepare.inc": function(text, "random_events_entry_prepare"),
        "actual_dispatch.inc": function(text, "random_events_dispatch_cases"),
        "actual_wrapper.inc": struct(text, "RandomEventsNativeCases"),
    }
    return text, rows[0].region, result


def compile_run(tmp_path, compiler, source_text, name):
    source = tmp_path / (name + ".cpp")
    binary = tmp_path / (name + ".exe")
    source.write_text(source_text)
    subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                    "-pedantic", "-I", str(tmp_path), str(source), "-o", str(binary)],
                   check=True, capture_output=True, text=True)
    subprocess.run([str(binary)], check=True, timeout=20, capture_output=True, text=True)


@pytest.mark.parametrize("compiler", COMPILERS)
@pytest.mark.parametrize("fixture", FIXTURES)
def test_reviewed_case_bodies_are_extracted_from_current_owner(tmp_path, compiler, fixture):
    _, _, fragments = source_fragments()
    for name, body in fragments.items():
        (tmp_path / name).write_text(body + "\n")
    if fixture == "cases_17_18.cpp":
        (tmp_path / "actual_cases_17_18.inc").write_text(
            fragments["actual_ceil_quarter.inc"] + "\n" + fragments["actual_cases_17_18.inc"])
    compile_run(tmp_path, compiler, (TESTS / fixture).read_text(), fixture)


ENTRY_HOST = r'''#define __cdecl
#include <cassert>
#include <cstring>
struct Base { short x,y; unsigned char faction_id_current; signed char population_size;
 unsigned int event; int random_event_turns; char name_string[25]; };
struct PlayerData { unsigned int current_num_bases; unsigned int diplo_treaties[8];
 unsigned int diff_level,ranking,energy_reserves,flags,satellites_[4]; };
static int GlobalEnergyEventState,SunspotDuration,DustCloudDuration,LockEnableMask;
static int BaseCurrentCount,TurnCurrentNum,DiffLevelCurrent,StartingMissionYear;
static unsigned int GameState,GameRules,MapRandSeed;
static Base Bases[8]; static PlayerData PlayersData[8]; static char StringTemp[1032];
static int random_values[8],random_count,random_index,reseed_count,reseed_value;
static int parser_order_count,mutate_base_on_parse;
static int dispatch_calls,dispatch_bucket; static RandomEventsDispatchInput dispatch_input;
static int treaty_calls,treaty_row,treaty_column,popup_calls;
static char popup_captions[4][32],popup_images[4][32];
static int game_year(int turn){return StartingMissionYear+turn;}
static int random(int low,int high){assert(low==0);(void)high;return random_values[random_index++];}
static void random_reseed(unsigned int seed){++reseed_count;reseed_value=(int)seed;}
static int parse_num(int low,int high){assert(low==0&&high==10);++parser_order_count;
 if(mutate_base_on_parse){Bases[0].x=19;Bases[0].y=20;}return 0;}
static int parse_says(int,const char *name,int,int){assert(strcmp(name,"NATIVE BASE")==0);++parser_order_count;return 0;}
static void set_treaty(int row,int column,int treaty,int enabled){assert(treaty==0xC00&&enabled==0);++treaty_calls;treaty_row=row;treaty_column=column;}
static int POP2(const char *caption,const char *image,int){assert(popup_calls<4);strcpy(popup_captions[popup_calls],caption);strcpy(popup_images[popup_calls],image);++popup_calls;return 0;}
struct Recorder {
 void save(int b,const RandomEventsDispatchInput &i){++dispatch_calls;dispatch_bucket=b;dispatch_input=i;}
 void cases_0_5(const RandomEventsDispatchInput&i){save(5,i);} void case6(const RandomEventsDispatchInput&i){save(6,i);}
 void cases_7_8(const RandomEventsDispatchInput&i){save(78,i);} void cases_9_11(const RandomEventsDispatchInput&i){save(911,i);}
 void cases_12_13(const RandomEventsDispatchInput&i){save(1213,i);} void case14(const RandomEventsDispatchInput&i){save(14,i);}
 void cases_15_16(const RandomEventsDispatchInput&i){save(1516,i);} void cases_17_18(const RandomEventsDispatchInput&i){save(1718,i);}
 void cases_19_21(const RandomEventsDispatchInput&i){save(1921,i);}
};
struct RandomEventsNativeCases : Recorder {};
static void reset(){memset(Bases,0,sizeof(Bases));memset(PlayersData,0,sizeof(PlayersData));
 GlobalEnergyEventState=SunspotDuration=DustCloudDuration=LockEnableMask=0;BaseCurrentCount=2;
 TurnCurrentNum=100;DiffLevelCurrent=0;StartingMissionYear=0;GameState=GameRules=MapRandSeed=0;
 StringTemp[0]='\0';random_count=random_index=reseed_count=parser_order_count=0;mutate_base_on_parse=0;
 reseed_value=treaty_calls=popup_calls=0;treaty_row=treaty_column=-1;
 dispatch_calls=0;dispatch_bucket=-1;Bases[0].x=4;Bases[0].y=5;Bases[0].population_size=4;
 Bases[0].faction_id_current=2;strcpy(Bases[0].name_string,"NATIVE BASE");PlayersData[2].current_num_bases=2;}
'''

ENTRY_MAIN = r'''
int main(){const int buckets[22]={5,5,5,5,5,5,6,78,78,911,911,911,1213,1213,14,1516,1516,1718,1718,1921,1921,1921};
 for(int e=0;e<22;++e){reset();random_values[random_count++]=0;random_values[random_count++]=e;
 mutate_base_on_parse=1;random_events(1);assert(parser_order_count==2&&StringTemp[0]=='\0');
 assert(dispatch_calls==1&&dispatch_bucket==buckets[e]&&dispatch_input.event_kind==e);
 assert(dispatch_input.faction_id==2&&dispatch_input.selected_base_id==0&&dispatch_input.base_roll==0);
 assert(dispatch_input.base_x_snapshot==4&&dispatch_input.base_y_snapshot==5&&Bases[0].x==19&&Bases[0].y==20);}
 reset();random_values[random_count++]=0;random_values[random_count++]=22;random_events(1);
 assert(parser_order_count==2&&dispatch_calls==0);
 reset();GlobalEnergyEventState=1;SunspotDuration=1;DustCloudDuration=1;LockEnableMask=1u<<1;
 for(int row=3;row<8;++row)PlayersData[row].diplo_treaties[1]=0x30;
 Bases[1].event=0x200;Bases[1].random_event_turns=1;MapRandSeed=1000;
 random_values[random_count++]=0;random_values[random_count++]=3;random_events(0);
 assert(GlobalEnergyEventState==2&&SunspotDuration==0&&DustCloudDuration==0);
 assert(treaty_calls==1&&treaty_row==2&&treaty_column==1);
 assert(Bases[1].event==0&&Bases[1].random_event_turns==0);
 assert(popup_calls==2&&strcmp(popup_captions[0],"NOMORESPOTS")==0&&strcmp(popup_captions[1],"NOMOREDUST")==0);
 assert(reseed_count==1&&reseed_value==1000+13*TurnCurrentNum);
 reset();GameRules=0x8000;random_events(0);assert(random_count==0&&reseed_count==0&&dispatch_calls==0);
 reset();TurnCurrentNum=74;random_events(1);assert(random_count==0&&dispatch_calls==0);
 reset();StartingMissionYear=0x8A2 - TurnCurrentNum;GameState=0x100000;random_events(0);
 assert(popup_calls==1&&strcmp(popup_captions[0],"PERIHELIONENDS")==0&&random_count==0);
 reset();StartingMissionYear=0x88E - TurnCurrentNum;random_events(0);
 assert((GameState&0x100000)!=0&&popup_calls==1&&strcmp(popup_captions[0],"PERIHELION")==0&&random_count==0);
 return 0;}
'''

WRAPPER_HOST = r'''#include <cassert>
struct RandomEventsDispatchInput { int event_kind,faction_id,selected_base_id,base_roll,base_x_snapshot,base_y_snapshot; };
static int calls,bucket,observed[6];
static void note(int b,int e,int f,int base,int roll,int x,int y){++calls;bucket=b;observed[0]=e;observed[1]=f;observed[2]=base;observed[3]=roll;observed[4]=x;observed[5]=y;}
struct RandomEventsNativeCase6State {}; struct RandomEventsNativeCase6Services {};
struct RandomEventsNativePositionServices {}; struct RandomEventsCase14NativeOps {};
namespace random_events_cases { template<class S,class V> inline void case6(S&,V&,int f,int b,int r){note(6,6,f,b,r,-1,-1);} }
static void random_events_native_cases_7_8(const RandomEventsDispatchInput&i){note(78,i.event_kind,i.faction_id,i.selected_base_id,i.base_roll,i.base_x_snapshot,i.base_y_snapshot);}
static void random_events_native_cases_9_11(int e){note(911,e,-1,-1,-1,-1,-1);}
static void random_events_cases_0_5(int e,int f,int b,int r){note(5,e,f,b,r,-1,-1);}
static void random_events_cases_12_13(int e,int f,int b,int r,int x,int y){note(1213,e,f,b,r,x,y);}
static void random_events_case14(RandomEventsCase14NativeOps&,int f,int b){note(14,14,f,b,-1,-1,-1);}
static void random_events_cases_15_16(int e,int f,int b,int r,int x,int y){note(1516,e,f,b,r,x,y);}
static void random_events_cases_17_18(int e,int f,int b,int r,int x,int y){note(1718,e,f,b,r,x,y);}
static void random_events_cases_19_21(int e,int f,int b,int r,int x,int y){note(1921,e,f,b,r,x,y);}
'''
WRAPPER_MAIN = r'''
int main(){const int expected[22]={5,5,5,5,5,5,6,78,78,911,911,911,1213,1213,14,1516,1516,1718,1718,1921,1921,1921};
 for(int event=0;event<22;++event){RandomEventsDispatchInput input={event,31,47,59,71,73};RandomEventsNativeCases cases;calls=0;
 random_events_dispatch_cases(input,cases);assert(calls==1&&bucket==expected[event]);
 if(bucket==78||bucket==1213||bucket==1516||bucket==1718||bucket==1921){assert(observed[0]==event&&observed[1]==31&&observed[2]==47&&observed[3]==59&&observed[4]==71&&observed[5]==73);}
 else if(bucket==5){assert(observed[0]==event&&observed[1]==31&&observed[2]==47&&observed[3]==59);}
 else if(bucket==6){assert(observed[1]==31&&observed[2]==47&&observed[3]==59);}
 else if(bucket==911){assert(observed[0]==event);}else if(bucket==14){assert(observed[1]==31&&observed[2]==47);}}
 return 0;}
'''


@pytest.mark.parametrize("compiler", COMPILERS)
def test_mapped_entry_dispatches_all_kinds_and_snapshots_before_callbacks(tmp_path, compiler):
    _, body, fragments = source_fragments()
    for name, text in fragments.items(): (tmp_path / name).write_text(text + "\n")
    source = (fragments["actual_dispatch_input.inc"] + "\n" + ENTRY_HOST + "\n" +
              fragments["actual_entry_prepare.inc"] + "\n" +
              fragments["actual_dispatch.inc"] + "\n" + body + ENTRY_MAIN)
    compile_run(tmp_path, compiler, source, "mapped-entry")


@pytest.mark.parametrize("compiler", COMPILERS)
def test_concrete_wrapper_and_dispatch_route_all_cases(tmp_path, compiler):
    _, _, fragments = source_fragments()
    for name, text in fragments.items(): (tmp_path / name).write_text(text + "\n")
    compile_run(tmp_path, compiler, WRAPPER_HOST + fragments["actual_wrapper.inc"] +
                fragments["actual_dispatch.inc"] + WRAPPER_MAIN, "concrete-wrapper")


def test_blight_cache_uses_seven_repeated_live_faction_reads():
    text, _, _ = source_fragments()
    body = function(text, "random_events_cases_15_16")
    start = body.index("for (int clear_pass = 0; clear_pass < 7;")
    end = body.index("\n            }", start)
    clear_loop = body[start:end]
    assert "random_events_blight_cache_word(\n                    tile, ConsoleGlobal.faction_id_)" in clear_loop


@pytest.mark.parametrize("compiler", COMPILERS)
def test_blight_cache_selects_real_members_including_factions_zero_and_seven(tmp_path, compiler):
    _, _, fragments = source_fragments()
    host = r'''#include <cassert>
typedef unsigned int uint32_t;
struct Map { uint32_t bit2, bit_visible[7]; };
'''
    main = r'''
int main() {
    Map tile = {0, {1,2,3,4,5,6,7}};
    for (int faction = 0; faction < 8; ++faction) {
        uint32_t *expected = faction == 0 ? &tile.bit2 : &tile.bit_visible[faction-1];
        assert(&random_events_blight_cache_word(tile, faction) == expected);
        random_events_blight_cache_word(tile, faction) = 100u + faction;
        assert(*expected == 100u + faction);
    }
}
'''
    compile_run(tmp_path, compiler, host + fragments["actual_blight_cache.inc"] + main, "cache-members")


@pytest.mark.parametrize("damage", ["snapshot", "reseed", "wrapper_coordinates"])
def test_controls_reject_damaged_actual_source(tmp_path, damage):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ is unavailable")
    _, body, fragments = source_fragments()
    if damage == "wrapper_coordinates":
        wrapper = fragments["actual_wrapper.inc"]
        old = "input.base_x_snapshot, input.base_y_snapshot"
        assert old in wrapper
        wrapper = wrapper.replace(old, "input.base_y_snapshot, input.base_x_snapshot", 1)
        source = WRAPPER_HOST + wrapper + fragments["actual_dispatch.inc"] + WRAPPER_MAIN
    else:
        entry = fragments["actual_entry_prepare.inc"]
        old, new = (("const int base_x_snapshot = selected_base.x;",
                     "const int base_x_snapshot = selected_base.x + 1;") if damage == "snapshot" else
                    ("13u * static_cast<unsigned int>(current_turn)",
                     "14u * static_cast<unsigned int>(current_turn)"))
        assert entry.count(old) == 1
        entry = entry.replace(old, new)
        source = "\n".join((fragments["actual_dispatch_input.inc"], ENTRY_HOST, entry,
                            fragments["actual_dispatch.inc"], body, ENTRY_MAIN))
    with pytest.raises(subprocess.CalledProcessError) as failure:
        compile_run(tmp_path, compiler, source, "damaged-control")
    assert failure.value.returncode < 0, "damage must be detected at execution, not by compile failure"
