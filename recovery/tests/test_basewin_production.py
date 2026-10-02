"""Strict host controls for the actual production helper source.

The test extracts the real product helpers from their owning translation unit; it never
substitutes an oracle implementation. The mock ABI uses integer IDs for the
engine's pointer-sized address-token interfaces and is not product compilation.
"""
import os
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
ACTION = ROOT / "src/basewin.cpp"
COMBINED = ROOT / "src/basewin.cpp"


def actual_fragment(path: Path, start: str, stop: str) -> str:
    text = path.read_text()
    begin = text.find(start)
    end = text.find(stop, begin + len(start))
    if begin < 0 or end < 0:
        raise AssertionError("required actual source fragment is absent")
    return text[begin:end]


PRELUDE = r'''
#include <stdint.h>
#include <string.h>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#define VSTATE_UNK_200 0x200u
struct Base { int16_t x,y; uint32_t state,governor,queue_size; int queue_production_id[10]; int minerals_accumulated_2,minerals_accumulated; uint8_t faction_id_current; };
struct Player { int diff_level; uint8_t proto_id_queue[512]; };
struct RulesType { int retool_pct_pen_prod_chg,retool_exemption,retool_strictness; };
struct RuleText { struct { int retool_pct_pen_prod_chg; } dummy; };
struct Proto { char veh_name[32]; uint32_t flags; uint8_t plan; };
struct Vehicle { int16_t x,y; uint32_t state; int16_t proto_id; uint16_t unk_1; uint8_t faction_id,year_end_lurking; int16_t next_veh_id_stack; };
typedef Vehicle Veh;
struct Popup { int result; int starts,items; void start(int,const char*,int,int,int,int) { ++starts; } struct Dialog { void item(int,int) {} }; Dialog d; Dialog &dialogs(){return d;} int exec(){return result;} };
struct BaseWin { int displayed; int displayed_production_id() const {return displayed;} int focused_base_id() const {return 0;} };
Base Bases[8]; Base *BaseCurrent=&Bases[0]; int BaseIDCurrentSelected=0; int BaseCurrentCount=2;
Player PlayersData[8]; Proto VehPrototypes[512]; Vehicle Vehs[8];
RulesType Rules; int IsMultiplayerNet=0; char StringTemp[256]; int PopupStartCaption=0;
static int log_count, wave_count, radius_count, synch_count;
static int callback_queue=-9999, callback_base=0, veh_calls, last_x, last_y;
static BaseWin *callback_window;
static int making_calls, mutate_exemption, parsed_numbers[3];
static int label_get(int n){return n;}
static void parse_num(int slot,int value){parsed_numbers[slot]=value;if(slot==0&&mutate_exemption)Rules.retool_exemption=7;}
static void log_say(const char*,int,int,int){++log_count;}
static void wave_it(int n){if(n!=0x24) abort(); ++wave_count;}
static void draw_radius(int,int,int,int){++radius_count; if(callback_queue!=-9999){BaseCurrent=&Bases[callback_base];BaseCurrent->queue_production_id[0]=callback_queue;}}
static int veh_at(int x,int y){++veh_calls;last_x=x;last_y=y;return 0;}
static void synch_veh(int){++synch_count;}
static int base_making(int id,int){++making_calls;if(callback_window&&making_calls==3)callback_window->displayed=9;return id==9?1:2;}
static int _itoa(int n,char *p,int){sprintf(p,"%d",n);return 0;}
struct FacilityType { int name; }; FacilityType Facility[128];
'''


HARNESS = r'''
int main(int argc,char **argv){
  (void)argc;
  const int which=atoi(argv[1]);
  BaseCurrent=&Bases[0]; BaseIDCurrentSelected=0;
  Bases[0].queue_size=2; Bases[0].queue_production_id[0]=3;
  Bases[0].queue_production_id[1]=4; Bases[0].queue_production_id[2]=5;
  Bases[0].minerals_accumulated_2=20; Bases[0].minerals_accumulated=100;
  PlayersData[0].diff_level=2; Rules.retool_pct_pen_prod_chg=50;
  Rules.retool_exemption=2; Rules.retool_strictness=1;
  BaseWin w; w.displayed=8; Popup p; int slot=0;
  if(which>=10 && which<=13){
    // Force the eligible popup path and exercise the real result arms.
    Bases[0].minerals_accumulated_2=20; p.result=which-10;
    const bool cancelled=apply_retool_cost_prompt(w,p,9,0,slot,0);
    if(which==10){if(!cancelled || log_count || Bases[0].minerals_accumulated!=100) return 11;}
    if(which==11){if(cancelled || Bases[0].minerals_accumulated!=11) return 12;}
    if(which==12){if(cancelled || slot!=1 || Bases[0].queue_production_id[1]!=9 || Bases[0].queue_production_id[2]!=4) return 13;}
    if(which==13){if(cancelled || slot!=3 || Bases[0].queue_production_id[3]!=9) return 14;}
    return 0;
  }
  if(which==20){
    slot=1; callback_queue=70; callback_base=1; Bases[1].x=31; Bases[1].y=47;
    Bases[1].faction_id_current=2; Vehs[0].faction_id=2; Vehs[0].proto_id=6;
    Vehs[0].next_veh_id_stack=-1; VehPrototypes[6].plan=12;
    VehPrototypes[70].flags=0;
    finish_ordinary_selection(w,p,70,0,slot,1);
    if(wave_count||radius_count||veh_calls||log_count!=1) return 21;
    return 0;
  }
  if(which==21){
    slot=0; callback_queue=70; callback_base=1; Bases[1].x=31; Bases[1].y=47;
    Bases[1].faction_id_current=2; Vehs[0].faction_id=2; Vehs[0].proto_id=6;
    Vehs[0].next_veh_id_stack=-1; VehPrototypes[6].plan=12;
    VehPrototypes[70].flags=0;
    finish_ordinary_selection(w,p,70,0,slot,1);
    if(wave_count||radius_count!=1||veh_calls!=1||last_x!=31||last_y!=47||synch_count) return 22;
    if(Bases[1].queue_production_id[0]!=70) return 23;
    return 0;
  }
  if(which==22){
    slot=0; callback_queue=70; callback_base=1; Bases[1].x=31; Bases[1].y=47;
    Bases[1].faction_id_current=2; Vehs[0].faction_id=2; Vehs[0].proto_id=6;
    Vehs[0].year_end_lurking=3; Vehs[0].state=VSTATE_UNK_200;
    Vehs[0].next_veh_id_stack=-1; VehPrototypes[6].plan=10;
    VehPrototypes[70].flags=0; IsMultiplayerNet=1;
    finish_ordinary_selection(w,p,70,0,slot,1);
    if(Vehs[0].year_end_lurking || (Vehs[0].state&VSTATE_UNK_200) || synch_count!=1) return 24;
    return 0;
  }
  if(which==23){
    slot=0; callback_queue=-70; callback_base=1; Bases[1].x=31; Bases[1].y=47;
    Vehs[0].next_veh_id_stack=-1;
    p.result=0; // a1!=0 bypasses popup; this value must still control post-callback branch
    finish_ordinary_selection(w,p,-70,0,slot,1);
    if(wave_count!=1||radius_count!=1||veh_calls!=1||last_x!=31||last_y!=47) return 25;
    return 0;
  }
  if(which==30){
    // Saved displayed ID precedes the third base_making callback.
    callback_window=&w; p.result=1;
    apply_retool_cost_prompt(w,p,9,0,slot,0);
    if(making_calls!=4||w.displayed!=9||p.starts!=1||Bases[0].minerals_accumulated!=11) return 31;
    return 0;
  }
  if(which==31){
    // Native parse_num(1) reloads exemption after parse_num(0).
    mutate_exemption=1;p.result=1;
    apply_retool_cost_prompt(w,p,9,0,slot,0);
    if(parsed_numbers[1]!=7||Bases[0].minerals_accumulated!=11) return 32;
    return 0;
  }
  if(which==32){
    // A full signed strictness value requires more than the draft's ten bytes.
    Rules.retool_strictness=(-2147483647-1);p.result=1;
    apply_retool_cost_prompt(w,p,9,0,slot,0);
    if(strcmp(StringTemp,"RETOOLPENALTY-2147483648A")) return 33;
    return 0;
  }
  if(which==33){
    Bases[0].minerals_accumulated=(-2147483647-1);p.result=1;
    apply_retool_cost_prompt(w,p,9,0,slot,0);
    if(parsed_numbers[2]!=2147483637) return 34;
    return 0;
  }
  return 90;
}
'''


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_current_actual_helper_fragment_controls(tmp_path, compiler):
    exe = shutil.which(compiler)
    if not exe:
        pytest.skip(compiler + " unavailable")
    helpers = actual_fragment(ACTION, "static int production_signed_bits", "// Native success label")
    # set_production_parse_name is a product helper whose pointer ABI is not
    # modeled by this host fixture; replace only that external call contract.
    helpers = helpers.replace(
        "static void set_production_parse_name(int slot, int production_id) {\n"
        "    if (production_id >= 0) {\n"
        "        parse_says(slot, VehPrototypes[production_id].veh_name, -1, -1);\n"
        "    } else {\n"
        "        parse_say(slot,\n"
        "                  reinterpret_cast<int>(Facility[-production_id].name),\n"
        "                  -1, -1);\n"
        "    }\n"
        "}\n\n", "static void set_production_parse_name(int, int) {}\n\n")
    source = PRELUDE + "\n" + helpers + HARNESS
    cpp = tmp_path / "production-controls.cpp"
    out = tmp_path / "production-controls"
    cpp.write_text(source)
    subprocess.run([exe, "-std=c++03", "-Wall", "-Wextra", "-Werror", "-pedantic",
                    "-fsanitize=address,undefined", "-fno-omit-frame-pointer",
                    str(cpp), "-o", str(out)], check=True)
    test_env = dict(os.environ)
    test_env["ASAN_OPTIONS"] = "detect_leaks=0"
    for case in range(10, 14):
        subprocess.run([str(out), str(case)], check=True, env=test_env)
    for case in range(20, 24):
        subprocess.run([str(out), str(case)], check=True, env=test_env)
    for case in range(30, 34):
        subprocess.run([str(out), str(case)], check=True, env=test_env)


def test_combined_actual_method_keeps_result_and_picker_routes():
    text = COMBINED.read_text()
    # Bound to actual current method text: result-zero propagates into its
    # unlock/restore/return path, while picker visibility is distinct from the
    # selected-ID 9999 full-setup restart.
    assert "const bool cancelled_retool = finish_ordinary_selection(" in text
    assert "if (cancelled_retool) {" in text
    assert "return; // native 0x4181FD result-table entry 0" in text
    assert "if (prodPicker_.is_visible()) {\n            run_picker_setup = false;\n            continue; // native 0x417DAE -> picker body 0x417751" in text
    assert "} else if (selected_id == 9999) {\n                run_picker_setup = true;\n                continue; // native 0x417DE6 -> full setup 0x41763D" in text
    assert "// native 0x417F7A..0x417F75" not in text
