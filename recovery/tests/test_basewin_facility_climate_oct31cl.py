"""Actual recycling method controls, anchored to the three original tile loads.

Map and map_tiles come from the product header. Other classes model only the
collaborator surface used here; these host controls make no UI/ABI/runtime claim.
"""
from pathlib import Path
import os
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


def definition(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 1
    end = opening + 1
    while depth:
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    return source[start:end]


HOST = r'''
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
// MAP
static_assert(sizeof(Map)==44,"real map tile stride");
static_assert(offsetof(Map,climate)==0 && offsetof(Map,contour)==1,"real fields");
Map first_tiles[6], second_tiles[6];
struct { Map *tiles_; } MapStorage;
#define MapTiles (MapStorage.tiles_)
// MAP_ACCESSOR
int MapLongitude=3;
const unsigned STATE_OMNISCIENT_VIEW=0x80;
const unsigned BSTATE_FACILITY_SCRAPPED=0x200,BSTATE_ARTIFACT_LINKED=0x400;
struct Base { int16_t x,y; uint8_t faction_id_current; const char *name_string;
 unsigned state; uint8_t facilities_built[16]; } Bases[3],*BaseCurrent;
struct {int faction_id_;} ConsoleGlobal;
struct {struct {unsigned game_state_;int base_current_count_;} fields_;} GameDataStateStorage;
struct Player {unsigned rule_flags;int faction_bonus_count;
 int faction_bonus_id[4],faction_bonus_val1[4];} Players[2];
struct {int energy_reserves;} PlayersData[2];
struct {const char *name;int cost,preq_tech;} Facility[80];
int BaseIDCurrentSelected=0,ConsoleControlTurnActive=0,IsMultiplayerNet=0,BaseComputedID=0;
const char *ScriptPopupFile="script";char StringTemp[1032];
int menu_choice=3,confirm_choice=0,locks=0,unlocks=0,computes=0,redraws=0;
int numbers[2],confirmation_calls=0;bool swap_on_lock=false,swap_on_confirm=false;
bool swap_on_store=false,move_on_menu=false;std::vector<int> writes,synchs;
std::vector<std::string> popups;
void parse_says(int,const char*,int,int){}
void parse_say(int,const char*,int,int){}
void parse_num(int slot,int value){numbers[slot]=value;}
void help_topic(int,int){}
void X_pop(const char*,int){}
int has_tech(int,int){return 0;}
void bitmask(int facility,int *offset,int *mask){*offset=facility/8;*mask=1<<(facility%8);}
int popp(const char*,const char *key,int,const char*,int){
 popups.push_back(key);
 if(std::strcmp(key,"FACMENU")==0){if(move_on_menu)BaseCurrent=&Bases[1];return menu_choice;}
 if(std::strncmp(key,"RECYCLEME",9)==0){
  ++confirmation_calls;
  if(swap_on_confirm){MapStorage.tiles_=second_tiles;Bases[2].facilities_built[3]=0;
   Bases[2].state|=BSTATE_FACILITY_SCRAPPED;}
  return confirm_choice;
 }
 return 0;
}
struct Daemon{
 int lock_base(int,int,int,int){++locks;if(swap_on_lock)MapStorage.tiles_=second_tiles;return 0;}
 void unlock_base(int){++unlocks;}
}NetDaemonNet;
void set_fac(int facility,int base,int){
 writes.push_back(base);int offset,mask;bitmask(facility,&offset,&mask);
 Bases[base].facilities_built[offset]&=static_cast<uint8_t>(~mask);
 if(swap_on_store && base==0)MapStorage.tiles_=second_tiles;
}
void synch_base(int base){synchs.push_back(base);}
void log_say(const char*,int,int,int){}
void base_compute(int){++computes;}
struct GraphicWin{void redraw(){++redraws;}};
struct BaseWin{int facilities_click(int,int,int,int);void redraw(){++redraws;}};
struct Info{void set_redraw_phase(int){}void on_redraw(){++redraws;}}ConsoleInfoWin;
GraphicWin MainInterfaceGlobal;
// ACTUAL
void reset(){
 std::memset(Bases,0,sizeof(Bases));std::memset(Players,0,sizeof(Players));
 std::memset(first_tiles,0,sizeof(first_tiles));std::memset(second_tiles,0,sizeof(second_tiles));
 for(int i=0;i<3;++i){Bases[i].x=i*2;Bases[i].faction_id_current=1;
  Bases[i].name_string="base";Bases[i].facilities_built[3]=4;
  first_tiles[i].climate=first_tiles[i].contour=0x60;
  second_tiles[i].climate=second_tiles[i].contour=0x60;}
 BaseCurrent=Bases;BaseIDCurrentSelected=0;ConsoleGlobal.faction_id_=1;
 GameDataStateStorage.fields_.game_state_=0;GameDataStateStorage.fields_.base_current_count_=3;
 PlayersData[1].energy_reserves=100;Facility[26].name="pressure dome";Facility[26].cost=10;
 MapStorage.tiles_=first_tiles;menu_choice=3;confirm_choice=0;
 locks=unlocks=computes=redraws=confirmation_calls=0;numbers[0]=numbers[1]=-1;
 swap_on_lock=swap_on_confirm=swap_on_store=move_on_menu=false;
 popups.clear();writes.clear();synchs.clear();
}
void initial_gate(){
 BaseWin win;reset();first_tiles[0].climate=0x40;first_tiles[0].contour=0x60;
 assert(win.facilities_click(26,0,0,0)==0);
 assert(popups.back()=="PRESSURERECYCLE" && locks==0 && writes.empty());
 reset();first_tiles[0].climate=0x60;first_tiles[0].contour=0x40;
 win.facilities_click(26,0,0,0);assert(locks==1 && confirmation_calls==1 && writes.empty());
 // The menu can move the live current base; the first tile read follows it.
 reset();move_on_menu=true;first_tiles[1].climate=0x40;
 win.facilities_click(26,0,0,0);assert(popups.back()=="PRESSURERECYCLE" && locks==0);
}
void bulk_count(){
 BaseWin win;reset();
 first_tiles[1].climate=first_tiles[2].climate=0x40;
 win.facilities_click(26,0,0,0);
 assert(numbers[0]==50 && numbers[1]==1 && PlayersData[1].energy_reserves==100);
 assert(writes.empty() && unlocks==1 && computes==1);
 reset();first_tiles[1].contour=first_tiles[2].contour=0x40;
 Bases[2].state=BSTATE_FACILITY_SCRAPPED;
 win.facilities_click(26,0,0,0);
 // Original counts payable using BaseCurrent.state, not each candidate state.
 assert(numbers[0]==150 && numbers[1]==3 && writes.empty());
 reset();swap_on_lock=true;second_tiles[1].climate=second_tiles[2].climate=0x40;
 win.facilities_click(26,0,0,0);assert(numbers[0]==50 && numbers[1]==1);
}
void bulk_writes(){
 BaseWin win;reset();confirm_choice=1;swap_on_confirm=true;
 second_tiles[1].climate=0x40;second_tiles[1].contour=0x60;
 second_tiles[2].climate=0x60;second_tiles[2].contour=0x40;
 win.facilities_click(26,0,0,0);
 assert(numbers[0]==150 && numbers[1]==3);
 assert((writes==std::vector<int>{0,2}) && (synchs==std::vector<int>{2}));
 // Payment is the count-phase amount. Mutation deliberately does not retest
 // possession or the one-sale bit changed by the confirmation collaborator.
 assert(PlayersData[1].energy_reserves==250 && unlocks==1 && computes==1);
 assert((Bases[1].state&BSTATE_FACILITY_SCRAPPED)==0);
 reset();confirm_choice=1;swap_on_store=true;second_tiles[1].climate=0x40;
 win.facilities_click(26,0,0,0);
 assert((writes==std::vector<int>{0,2})); // next iteration reloads MapTiles
}
int main(int argc,char **argv){assert(argc==2);std::string which=argv[1];
 if(which=="initial")initial_gate();else if(which=="count")bulk_count();
 else if(which=="writes")bulk_writes();else assert(false);
}
'''


def program(mutant=None):
    source = (ROOT / "src/basewin.cpp").read_text()
    header = (ROOT / "src/map.h").read_text()
    method = definition(source, "int BaseWin::facilities_click(")
    assert method.count(".climate & 0xE0") == 3
    if mutant is not None:
        positions = [index for index in range(len(method))
                     if method.startswith(".climate & 0xE0", index)]
        index = positions[mutant]
        method = method[:index] + method[index:].replace(".climate", ".contour", 1)
    return (HOST.replace("// MAP\n", definition(header, "struct Map {") + ";\n")
            .replace("// MAP_ACCESSOR", definition(header, "inline Map *&map_tiles()"))
            .replace("// ACTUAL", method))


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("mutant", [None, 0, 1, 2])
def test_actual_complete_method_and_independent_field_damage(tmp_path, compiler_name, mutant):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    source = tmp_path / "facility.cpp"
    executable = tmp_path / "facility"
    source.write_text(program(mutant))
    built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                            "-fsanitize=undefined", str(source), "-o", str(executable)],
                           capture_output=True, text=True, timeout=60)
    assert built.returncode == 0, built.stdout + built.stderr
    cases = ["initial", "count", "writes"] if mutant is None else [["initial", "count", "writes"][mutant]]
    for case in cases:
        result = subprocess.run([str(executable), case], capture_output=True, text=True, timeout=20)
        if mutant is None:
            assert result.returncode == 0, result.stdout + result.stderr
        else:
            assert result.returncode != 0, "independent field damage survived: " + case
            # Require the intended behavioral oracle, not an unrelated crash.
            expected = ["PRESSURERECYCLE", "numbers[0]==50", "writes=="][mutant]
            assert "Assertion" in result.stderr and expected in result.stderr, result.stderr


def test_original_three_loads_select_tile_byte_zero():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned image unavailable")
    image = PeImage(path.read_bytes(), image_pin(ROOT))
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    instructions = {item.address: (item.mnemonic, item.op_str)
                    for item in decoder.disasm(image.read(Span(0x40C070, 0x40C842)), 0x40C070)}
    for address, expected in {
        0x40C33E: ("mov", "al, byte ptr [ecx + edx]"),
        0x40C481: ("mov", "al, byte ptr [ecx + edx]"),
        0x40C710: ("mov", "dl, byte ptr [eax + ecx]"),
        0x40C333: ("mov", "edx, dword ptr [0x94a30c]"),
        0x40C476: ("mov", "edx, dword ptr [0x94a30c]"),
        0x40C705: ("mov", "ecx, dword ptr [0x94a30c]"),
        0x40C339: ("imul", "ecx, ecx, 0x2c"),
        0x40C47C: ("imul", "ecx, ecx, 0x2c"),
        0x40C70B: ("imul", "eax, eax, 0x2c"),
        0x40C414: ("call", "0x530d50"),
        0x40C647: ("call", "0x48c0a0"),
        0x40C692: ("call", "0x48c0a0"),
        0x40C72B: ("call", "0x4e48b0"),
    }.items():
        assert instructions[address] == expected
