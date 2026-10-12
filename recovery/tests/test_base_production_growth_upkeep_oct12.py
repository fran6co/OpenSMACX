"""Full actual Base production/growth/upkeep controls, not game acceptance.

Passive records are extracted from current owners. Engine/UI doubles expose
callback and Popup lifetime boundaries; host ABI and provider behavior are not
evidence of native layout, unwind selection, aliases or full-game runtime.
"""
from dataclasses import fields
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS
from match_recovery.source import read_file, read_text
from test_map_metadata_owner import definition
from test_base_economy_accounting_oct12 import declarations, helpers
from test_base_colony_production import PRODUCTION_HARNESS, helper, _assert_production_harness_constants

ROOT = Path(__file__).resolve().parents[2]
BASE = "908fd8cdcd7db78475f500bb03c4f143560bdb98"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
SUBJECTS = {
    0x4F07E0: ((0x4F07E0, 0x4F3FD8), (0x65C8A0, 0x65D0D6)),
    0x4EF1C0: ((0x4EF1C0, 0x4EFB4E),),
    0x4F79C0: ((0x4F79C0, 0x4F7FDA),),
}
SCOPED_DEFINITIONS = (
    "void __cdecl base_production()", "int __cdecl base_growth()", "int __cdecl base_upkeep(",
    "void production_late_reconcile(", "int production_vehicle_morale(",
    "void production_vehicle_notice(", "void production_refresh_base_if_needed(",
    "void production_finish_vehicle(",
    "bool production_colony_abandon_prompt(", "void production_clear_artifact_orders(",
    "ProductionReportResume production_report_stage(",
    "static void production_report_foreign_stage_v3(",
    "void production_voice_of_planet_spread(",
    "bool production_facility_built(", "void production_existing_project_notice(",
    "bool production_reveal_faction(",
    "int production_abs(", "int production_project(", "void production_set_project_owner(",
)


def source():
    return (ROOT / "src/base.cpp").read_text()


def actual(text, signature):
    body = definition(text, signature)
    assert "BODY GOES HERE" not in body
    return body


def tokens(text):
    result, cursor = [], 0
    for match in COMMENTS_OR_STRINGS.finditer(text):
        result.extend(text[cursor:match.start()].split())
        if not match[0].startswith(("//", "/*")):
            result.append(match[0])
        cursor = match.end()
    return result + text[cursor:].split()


def assert_group_continuity(before, current, path):
    """Exact 19-definition authorization; identities/history never exempted."""
    assert len(SCOPED_DEFINITIONS) == 19
    old = {r.address: r for r in read_text(before, path)}
    new = {r.address: r for r in read_text(current, path)}
    assert new.keys() == old.keys()
    for address, record in new.items():
        for field in fields(record):
            if field.name not in {"line", "region", "origin"}:
                assert getattr(record, field.name) == getattr(old[address], field.name), (hex(address), field.name)
        def lessons(row):
            return [(x.kind, x.key, x.prose, tuple(t for _, t in x.lines)) for x in row.origin or ()]
        assert lessons(record) == lessons(old[address])
        if address in SUBJECTS:
            assert record.image_spans == SUBJECTS[address]
    for signature in SCOPED_DEFINITIONS:
        before = before.replace(actual(before, signature), "SCOPED_OWNER_BODY", 1)
        current = current.replace(actual(current, signature), "SCOPED_OWNER_BODY", 1)
    assert tokens(before) == tokens(current)
    # Also retain outside-body source annotations/comments verbatim, including
    # object/member bindings not represented by the function-record parser.
    assert before == current, "outside-group source text changed"


def restore_body(text, current_body, previous_body):
    assert text.count(current_body) == 1
    return text.replace(current_body, previous_body, 1)


def restore_group_to_baseline(current, path):
    """Reverse only this reviewed group for the older economy scope guard.

    The predecessor remains 908fd; this neither resets the economy BASE58563
    nor permits unrelated body/mapping/claim/history changes. Host semantic
    and original-pin controls remain separate obligations for these bodies.
    """
    before = subprocess.check_output(["git", "show", BASE + ":src/base.cpp"], cwd=ROOT, text=True)
    assert_group_continuity(before, current, path)
    restored = current
    for signature in SCOPED_DEFINITIONS:
        restored = restore_body(restored, actual(current, signature), actual(before, signature))
    assert tokens(restored) == tokens(before), "restored body continuity failed"
    assert restored == before, "restored source text continuity failed"
    return restored


def test_original_identity_spans_claims_and_all_outside_scope_code_survive():
    path = ROOT / "src/base.cpp"
    before = subprocess.check_output(["git", "show", BASE + ":src/base.cpp"], cwd=ROOT, text=True)
    assert_group_continuity(before, source(), path)


def test_guarded_historical_group_restoration_matches_exact_predecessor():
    path = ROOT / "src/base.cpp"
    restored = restore_group_to_baseline(source(), path)
    before = subprocess.check_output(["git", "show", BASE + ":src/base.cpp"], cwd=ROOT, text=True)
    assert tokens(restored) == tokens(before)


@pytest.mark.parametrize("damage", ("outside_body", "outside_binding", "identity", "failed_restoration"))
def test_historical_group_bridge_rejects_damage(monkeypatch, damage):
    import sys
    text = source()
    if damage == "outside_body":
        old = "int base_yield_add("
        assert text.count(old) >= 1
        text = text.replace(old, "int unauthorized_base_yield_add(")
    elif damage == "outside_binding":
        old = "// RECOVERY_OBJECT: v2 0x0090E934 "
        assert text.count(old) == 1
        text = text.replace(old, "// RECOVERY_OBJECT: v2 0x0090E935 ", 1)
    elif damage == "identity":
        old = "// ORIGINAL: 0x004F07E0 "
        assert text.count(old) == 1
        text = text.replace(old, "// ORIGINAL: 0x004F07E1 ", 1)
    else:
        monkeypatch.setattr(sys.modules[__name__], "restore_body", lambda text, current, previous: text)
    with pytest.raises(AssertionError):
        restore_group_to_baseline(text, ROOT / "src/base.cpp")


def run_actual(tmp_path, text, succeeds=True):
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    assert any(compilers), "actual-source host compiler required"
    path = tmp_path / "base-owner.cpp"
    path.write_text(text)
    for compiler in filter(None, compilers):
        executable = tmp_path / Path(compiler).name.replace("+", "p")
        built = subprocess.run([compiler, "-std=c++11", "-O2", "-fsanitize=undefined", "-fno-sanitize-recover=undefined", str(path), "-o", str(executable)], capture_output=True, text=True, timeout=60)
        assert built.returncode == 0, built.stdout + built.stderr
        result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
        assert (result.returncode == 0) == succeeds, result.stdout + result.stderr


def passive_common():
    base = (ROOT / "src/base.h").read_text()
    faction = (ROOT / "src/faction.h").read_text()
    return (declarations() + "\n#include <stdio.h>\n#include <stdlib.h>\n"
            "#define TRUE 1\n#define FALSE 0\nuint32_t MapLongitude; int GenderDefault,PluralityDefault;\n"
            + helpers() + "\n" + definition(base, "enum GovernorBaseBitfield {") + ";\n"
            + definition(faction, "enum PlayerRuleFlagBitfield {") + ";\n")


def map_types_aliases():
    header = (ROOT / "src/map.h").read_text()
    aliases = "\n".join(line for line in header.splitlines() if line.startswith((
        "#define MapLongitudeBounds ", "#define MapLatitudeBounds ", "#define MapIsFlat ", "#define MapTiles ", "#define MapArea ")))
    return definition(header, "struct Map {") + ";\n" + aliases + "\n"


UPKEEP_PROVIDERS = r'''
static int zooms,logs,zoom_switch,renames;
static int test_rand(){return 0;}
#define rand test_rand
int BaseComputedID,BaseProductionZoomRequested,BaseUpkeepStage,UnkGlobal0093A934;
int IsMultiplayerNet,BaseCommerceReceipts[7];
char ScriptPopupFileText[]="LIVE_SCRIPT";char *ScriptPopupFile=ScriptPopupFileText;
struct TestConsole { int faction_id_; int focus(int,int,int){return 1;} } ConsoleGlobal;
struct TestTut {
  int focused_base_id(){return -1;} void on_redraw(){}
  void zoom(int,int){++zooms;if(zoom_switch){BaseIDCurrentSelected=1;BaseCurrent=&Bases[1];}}
} TutWinBaseWindow;
void base_support(){}void base_yield(){}void base_nutrient(){}void base_minerals(){}void base_energy(){}
void base_production(){BaseProductionZoomRequested=1;}void base_hurry(){}int base_growth(){return 0;}
void base_check_support(){}void base_drones(){}void base_doctors(){}void base_energy_costs(){}void base_research(){}
void base_ecology(){}void base_maint(){}void base_terraform(int){}
int bit_count(int x){int n=0;for(uint32_t b=static_cast<uint32_t>(x);b;b>>=1)n+=b&1;return n;}
void draw_tiles(int,int,int){}void draw_map(int){}
void parse_says(int,const char*,int,int){}int name_base(int,char*,int,int){return 0;}
void popp(const char *script,const char*,int,const char*,int){assert(script==ScriptPopupFile);++renames;}void log_say(const char*,int,int,int){++logs;}
'''
UPKEEP_MAIN = r'''
int main(){
  Map tiles[8]={};MapStorage.tiles_=tiles;MapLongitude=1;Bases[0].faction_id_current=1;
  UnkGlobal0093A934=17;
  GameDataStateStorage.fields_.game_state_=1;GameDataStateStorage.fields_.game_preferences_=0;
  assert(base_upkeep(0)==0 && zooms==0 && logs==0);
  assert(UnkGlobal0093A934==17);
  GameDataStateStorage.fields_.game_state_=0;GameDataStateStorage.fields_.game_preferences_=1;
  assert(base_upkeep(0)==0 && zooms==1 && logs==2);
  zooms=logs=0;GameDataStateStorage.fields_.game_state_=9;
  assert(base_upkeep(0)==0 && zooms==1 && logs==2);
  zooms=logs=0;zoom_switch=1;GameDataStateStorage.fields_.game_state_=0;
  GameDataStateStorage.fields_.game_preferences_=0;GameDataStateStorage.fields_.turn_current_num_=7;
  Bases[1].faction_id_current=1;Bases[1].state=0x100000;
  assert(base_upkeep(0)==0 && BaseIDCurrentSelected==1 && BaseCurrent==&Bases[1]);
  assert(Bases[1].state==0 && BaseUpkeepStage==0);
  // Native ADD keeps only low bits for this scheduling test, including wrap.
  GameDataStateStorage.fields_.turn_current_num_=INT_MAX;
  Bases[1].state=0x100000;assert(base_upkeep(0)==0 && Bases[1].state==0);
  char live_script[]="DYNAMIC_SCRIPT";ScriptPopupFile=live_script;
  ConsoleGlobal.faction_id_=1;
  Bases[0].state=0x10000;GameDataStateStorage.fields_.game_state_=0;
  assert(base_upkeep(0)==0 && renames==1 && UnkGlobal0093A934==17);
}
'''


def upkeep_program():
    src = source()
    header = (ROOT / "src/base.h").read_text()
    arithmetic = "\n".join(actual(src, "static int " + name + "(") for name in ("base_upkeep_wrap_signed_bits", "base_upkeep_wrap_add"))
    return (passive_common() + map_types_aliases() + UPKEEP_PROVIDERS + arithmetic
            + definition(header, "MEASURED inline int __cdecl base_project(")
            + actual(src, "int __cdecl base_upkeep(") + UPKEEP_MAIN)


def test_complete_upkeep_state_gate_live_selected_id_and_wrapped_schedule(tmp_path):
    run_actual(tmp_path, upkeep_program())


@pytest.mark.parametrize("damage", ("preferences_gate", "captured_selected", "unwrapped_schedule", "invented_energy_reset", "hardcoded_rename_script"))
def test_upkeep_runtime_damage_controls(tmp_path, damage):
    text = upkeep_program()
    if damage == "preferences_gate":
        old,new = "const uint32_t state_for_zoom = GameDataStateStorage.fields_.game_state_;", "const uint32_t state_for_zoom = GameDataStateStorage.fields_.game_preferences_;"
    elif damage == "captured_selected":
        old,new = "base_upkeep_wrap_add(BaseIDCurrentSelected,", "base_upkeep_wrap_add(base_id,"
    elif damage == "invented_energy_reset":
        old,new="    BaseProductionZoomRequested = 0;", "    BaseProductionZoomRequested = 0;\n    UnkGlobal0093A934 = 0;"
    elif damage == "hardcoded_rename_script":
        old,new='popp(ScriptPopupFile, "RENAMEBASE"', 'popp("SCRIPT", "RENAMEBASE"'
    else:
        old,new = "base_upkeep_wrap_add(BaseIDCurrentSelected,\n            GameDataStateStorage.fields_.turn_current_num_)", "(BaseIDCurrentSelected + GameDataStateStorage.fields_.turn_current_num_)"
    assert text.count(old)==(2 if damage == "hardcoded_rename_script" else 1)
    run_actual(tmp_path,text.replace(old,new,1),False)


GROWTH_PROVIDERS = r'''
RulesBasic Rules;Veh Vehs[8];VehPrototype VehPrototypes[512];
int BaseCurrentGrowthRate,BaseComputedID,BasePsychDrones[5],RadiusOffsetX[289],RadiusOffsetY[289];
int change_population_rule,change_base_faction,growth_select_switch,fac_calls,limit_calls,support_calls;
int has_fac(int id,int selected,int){assert(selected==0);++fac_calls;
  if(growth_select_switch){BaseIDCurrentSelected=1;BaseCurrent=&Bases[1];return 0;}
  if(id==24){if(change_population_rule)Players[1].rule_population=100;
    if(change_base_faction)Bases[0].faction_id_current=2;return 1;}return 0;}
int cost_factor(int,int,int){return 10;}int has_tech(int,int){return 0;}int is_objective(int){return 0;}
void popb(const char*label,int,int,const char*,void*){if(strcmp(label,"POPULATIONLIMIT")==0)++limit_calls;}
void base_kill(int){}void draw_map(int){}void eliminate_player(int,int){}void draw_tile(int,int,int){}
void base_support(){++support_calls;}void base_yield(){}void base_nutrient(){}void base_minerals(){}void base_energy(){}
void parse_say(int,const char*,int,int){}void bitmask(int,int*o,int*m){*o=0;*m=1;}
'''
GROWTH_MAIN = r'''
static void reset(){
  memset(Bases,0,sizeof Bases);memset(Players,0,sizeof Players);memset(PlayersData,0,sizeof PlayersData);
  memset(&Rules,0,sizeof Rules);memset(&GameDataStateStorage,0,sizeof GameDataStateStorage);
  BaseCurrent=&Bases[0];BaseIDCurrentSelected=0;Bases[0].faction_id_current=1;Bases[0].population_size=2;
  Bases[0].nutrients_accumulated=30;GameDataStateStorage.fields_.secret_project_.cloning_vats()=-1;
  GameDataStateStorage.fields_.secret_project_.ascetic_virtues()=-1;
  Rules.pop_limit_sans_hab_complex=2;Rules.pop_limit_sans_hab_dome=10;
  BaseCurrentGrowthRate=change_population_rule=change_base_faction=fac_calls=limit_calls=support_calls=0;
  growth_select_switch=0;
}
int main(){
  Map tiles[8]={};MapStorage.tiles_=tiles;MapLongitude=1;MapLongitudeBounds=MapLatitudeBounds=4;
  reset();change_population_rule=1;assert(base_growth()==0);
  assert(Bases[0].population_size==2 && Bases[0].nutrients_accumulated==15 && limit_calls==1 && support_calls==0);
  // The second read uses the same Player row, not a newly selected faction.
  reset();change_population_rule=change_base_faction=1;Players[2].rule_population=-100;
  assert(base_growth()==0 && Bases[0].population_size==2 && limit_calls==1);
  reset();BaseCurrentGrowthRate=6;Rules.nutrient_req_citizen=1;Bases[0].nutrient_surplus=1;
  change_population_rule=1;assert(base_growth()==0);
  assert(Bases[0].population_size==2 && limit_calls==1 && support_calls==0);
  reset();Rules.pop_limit_sans_hab_complex=Rules.pop_limit_sans_hab_dome=100;
  assert(base_growth()==0 && Bases[0].population_size==3 && support_calls==1);
  reset();Players[1].rule_population=INT_MAX;
  assert(base_growth()==0 && Bases[0].population_size==3); // wrapped signed population limit
  reset();Bases[0].nutrients_accumulated=-1;GameDataStateStorage.fields_.game_state_=1;Bases[0].nutrient_surplus=3;
  assert(base_growth()==0 && Bases[0].population_size==2 && Bases[0].nutrients_accumulated==3);
  reset();BaseCurrentGrowthRate=6;Rules.nutrient_req_citizen=1;Bases[0].nutrient_surplus=1;
  growth_select_switch=1;Bases[1].faction_id_current=1;Bases[1].population_size=1;Bases[1].nutrients_accumulated=30;
  assert(base_growth()==0 && Bases[1].population_size==2 && Bases[0].population_size==2 && support_calls==1);
}
'''


def growth_program():
    veh = (ROOT / "src/veh.h").read_text()
    alpha = (ROOT / "src/alpha.h").read_text()
    declarations_ = "\n".join(definition(text, signature)+";" for text,signature in (
        (veh,"struct Veh {"),(veh,"struct VehPrototype {"),(alpha,"struct RulesBasic {")))
    return passive_common()+map_types_aliases()+declarations_+GROWTH_PROVIDERS+actual(source(),"int __cdecl base_growth()")+GROWTH_MAIN


def test_complete_growth_live_rule_row_signed_limits_and_original_starvation(tmp_path):
    run_actual(tmp_path,growth_program())


@pytest.mark.parametrize("damage",("cached_rule","wrong_rule_owner","unwrapped_limit","shared_selected_capture"))
def test_growth_runtime_damage_controls(tmp_path,damage):
    text=growth_program()
    if damage=="cached_rule":
        text=text.replace("const Player &population_rules = Players[Bases[faction].faction_id_current];", "const Player &population_rules = Players[Bases[faction].faction_id_current];\n        const int cached_population_rule = population_rules.rule_population;")
        old,new="population_rules.rule_population)","cached_population_rule)"
        assert text.count(old)==4
        text=text.replace(old,new)
    elif damage=="wrong_rule_owner":
        old,new="population_rules.rule_population)","Players[Bases[faction].faction_id_current].rule_population)"
        assert text.count(old)==4
        text=text.replace(old,new)
    elif damage=="shared_selected_capture":
        text=text.replace("    int growth = cost_factor", "    const int captured_selected = BaseIDCurrentSelected;\n    int growth = cost_factor",1)
        old="const int faction = BaseIDCurrentSelected;";assert text.count(old)==2
        text=text.replace(old,"const int faction = captured_selected;")
    else:
        old="base_yield_add(base_yield_sub(Bases[faction].population_size, margin),\n                population_rules.rule_population)"
        new="(Bases[faction].population_size - margin + population_rules.rule_population)"
        assert text.count(old)==1
        text=text.replace(old,new,1)
    run_actual(tmp_path,text,False)


def production_program():
    _assert_production_harness_constants()
    src=source();base=(ROOT/"src/base.h").read_text();faction=(ROOT/"src/faction.h").read_text();veh=(ROOT/"src/veh.h").read_text();alpha=(ROOT/"src/alpha.h").read_text();mapping=(ROOT/"src/map.h").read_text()
    game_state=(ROOT/"src/game_data_state_window.h").read_text()
    text=PRODUCTION_HARNESS[:PRODUCTION_HARNESS.index("int main()")]
    text=text.replace("#define __cdecl", "#define __cdecl\ntypedef int BOOL;typedef char *LPSTR;")
    for header,signature in ((base,"struct Base {"),(faction,"struct Player {"),(veh,"struct VehPrototype {"),(veh,"struct Veh {")):
        text=text.replace(definition(text,signature),definition(header,signature),1)
    text=text.replace(definition(text,"struct RuleFacility {"),definition(base,"struct RulesFacility {"),1)
    text=text.replace(definition(text,"struct RulesType {"),definition(alpha,"struct RulesBasic {"),1)
    text=text.replace(definition(text,"struct Map {"),definition(mapping,"struct Map {"),1)
    player_data="\n".join(definition(faction,sig)+";" for sig in ("struct SocialCategory {","struct SocialEffect {","struct Goal {"))+definition(faction,"struct PlayerData {")
    text=text.replace(definition(text,"struct PlayerData {"),player_data,1)
    state_types=definition(base,"struct BaseSecretProject {")+";\n"+definition(game_state,"struct GameDataLockMaskBytes {")+";\n"+definition(game_state,"union GameDataLockMaskView {")
    text=text.replace(definition(text,"struct GameDataLockMaskView {"),state_types,1)
    text=text.replace(definition(text,"struct GameDataStateWindowFields {"),definition(game_state,"struct GameDataStateWindowFields {"),1)
    text=text.replace("VehPrototypes[128]", "VehPrototypes[512]")
    text=text.replace("PRODUCTION_ARITHMETIC_HELPERS","\n".join(helper(src,name) for name in ("production_wrapped_subtract","production_wrapped_add","production_wrapped_multiply")))
    text=text.replace("PRODUCTION_COLONY_PROMPT_HELPER",helper(src,"production_colony_abandon_prompt"))
    text=text.replace(definition(text,"int production_abs("),"int base_yield_sub(int,int);\n"+actual(src,"int production_abs("),1)
    text=text.replace(definition(text,"int production_project("),actual(src,"int production_project("),1)
    text=text.replace("PRODUCTION_BODY",actual(src,"void __cdecl base_production()"))
    text=text.replace(definition(text,"bool production_facility_built("),"bool production_facility_built(int,int,int&);",1)
    text=text.replace("void production_voice_of_planet_spread()", "void production_voice_of_planet_spread(int&)")
    text=text.replace(definition(text,"void draw_tile("), "void draw_tile(int,int,int){call_order.push_back(90);}",1)
    # Full actual reconciliation participates in the entry, not a tail stub.
    text=text.replace(definition(text,"void production_late_reconcile("),"void production_late_reconcile(int,int,uint32_t,int);",1)
    text += "\n"+helpers()+"\nvoid draw_radius(int,int,int,int){call_order.push_back(91);}\n"+actual(src,"void production_late_reconcile(")+actual(src,"bool production_facility_built(")
    # Observe service order and a live selected id published by cost_factor.
    text=text.replace("int cost_factor(int, int, int) { return production_test_cost_factor; }", "int cost_callback_switch,live_cost_faction,live_cost_base,cost_mutate_facility;\nint cost_factor(int faction,int,int){live_cost_faction=faction;call_order.push_back(1);if(cost_callback_switch){BaseIDCurrentSelected=1;BaseCurrent=&Bases[1];}if(cost_mutate_facility)Facility[cost_mutate_facility].cost=200;return production_test_cost_factor;}")
    text=text.replace("int veh_cost(int, int, int) { return production_test_vehicle_cost; }", "int cost_throw;\nint veh_cost(int,int selected,int){live_cost_base=selected;call_order.push_back(2);if(cost_throw)throw 17;return production_test_vehicle_cost;}")
    text=text.replace("int production_vehicle_morale(int, int, int, bool) { return 4; }", "int production_vehicle_morale(int,int,int,bool){call_order.push_back(22);return 4;}")
    text=text.replace("void base_reset(int, int) { ++base_reset_calls; call_order.push_back(5); }", "int reset_callback_switch;\nvoid base_reset(int,int){++base_reset_calls;call_order.push_back(5);if(reset_callback_switch){BaseIDCurrentSelected=1;BaseCurrent=&Bases[1];}}")
    text=text.replace("call_order.push_back(101); return 0;", "call_order.push_back(101); return 2;")
    text=text.replace("bool ascending(int) { return production_test_ascending != 0; }", "int ascent_rival_block;\nbool ascending(int owner){return production_test_ascending!=0 && !(ascent_rival_block && owner==2);}")
    text=text.replace("int base_queue(int) {", "int queue_callback_switch,queue_callback_count;\nint base_queue(int) {\n    if(queue_callback_switch){assert(++queue_callback_count==1);BaseCurrent=&Bases[1];return 1;}")
    text=text.replace("void amovie_project(int) { call_order.push_back(82); }", "int movie_switch;\nvoid amovie_project(int){call_order.push_back(82);if(movie_switch)ConsoleGlobal.faction_id_=2;if(movie_switch==2)GameDataStateStorage.fields_.game_preferences_=0;if(movie_switch==3)IsMultiplayerNet=1;}")
    # A queue boundary may intentionally retain the completed item.
    return text+PRODUCTION_MAIN


PRODUCTION_MAIN=r'''
int main(){
  reset_case();ConsoleGlobal.faction_id_=2;VehPrototypes[3].plan=8;
  Players[1].ai_power=0;PlayersData[1].ai_power=1;PlayersData[1].pop_total=4;
  base_production();assert((Bases[0].state&0x01000000u)==0 && Vehs[0].flags==0);
  assert(Vehs[0].morale==4 && (Vehs[0].state&0x2000)!=0);check_popup_lifetime();
  reset_case();ConsoleGlobal.faction_id_=1;VehPrototypes[3].plan=8;
  Bases[0].population_size=1;PlayersData[1].diff_level=3;
  base_production();assert(base_kill_calls==1 && Vehs[0].home_base_id==-1);
  assert(Vehs[0].morale==4 && (Vehs[0].state&0x2000)!=0 && (Vehs[0].flags&0x1000)!=0);
  assert(order_index(22)<order_index(30));check_popup_lifetime();
  reset_case();ConsoleGlobal.faction_id_=1;Bases[0].queue_production_id[0]=-71;Facility[71].cost=20;
  base_production();assert(Bases[0].queue_production_id[0]==-69 && reveal_calls==0);
  assert(order_index(90)>=0 && order_index(90)<order_index(51));check_popup_lifetime();
  reset_case();ConsoleGlobal.faction_id_=1;Bases[0].queue_production_id[0]=-65;Facility[65].cost=20;
  PlayersData[1].satellites_[0]=0xFFFFFFFFu;base_production();
  assert(PlayersData[1].satellites_[0]==0);check_popup_lifetime();
  reset_case();ConsoleGlobal.faction_id_=1;Bases[0].queue_production_id[0]=3;
  cost_callback_switch=1;Bases[1].faction_id_current=2;Bases[1].minerals_accumulated=100;
  base_production();assert(live_cost_faction==1 && live_cost_base==1);
  assert(order_index(1)<order_index(2) && last_vehicle_faction==1 && last_vehicle_x==Bases[1].x);
  check_popup_lifetime();cost_callback_switch=0;
  reset_case();ConsoleGlobal.faction_id_=1;GameDataStateStorage.fields_.lock_masks_.enable_mask_=0;
  Bases[0].state=8;reset_callback_switch=1;Bases[1].faction_id_current=2;
  Bases[1].queue_production_id[0]=3;Bases[1].minerals_accumulated=100;
  base_production();assert(live_cost_faction==2 && last_vehicle_faction==1);
  assert(last_vehicle_x==Bases[1].x);check_popup_lifetime();reset_callback_switch=0;
  reset_case();ConsoleGlobal.faction_id_=1;Bases[0].nutrients_accumulated=-1;
  base_production();assert(production_spend_calls==0 && veh_init_calls==0);check_popup_lifetime();
  reset_case();ConsoleGlobal.faction_id_=1;BaseProductionUnitCreationDisabled=1;
  base_production();assert(production_spend_calls==1 && veh_init_calls==0);check_popup_lifetime();
  reset_case();ConsoleGlobal.faction_id_=1;production_test_vehicle_id=-1;
  base_production();assert(production_spend_calls==1 && veh_init_calls==1 && production_finish_calls==0);check_popup_lifetime();
  reset_case();ConsoleGlobal.faction_id_=1;Bases[0].queue_production_id[0]=-32;
  Facility[32].cost=20;cost_mutate_facility=32;
  base_production();assert(production_spend_calls==0 && set_fac_calls==0);check_popup_lifetime();
  cost_mutate_facility=0;
  reset_case();Bases[0].queue_production_id[0]=-32;Facility[32].cost=20;
  cost_callback_switch=1;Bases[1].faction_id_current=2;Bases[1].minerals_accumulated=100;
  Bases[1].queue_production_id[0]=-32;Bases[1].facilities_built[4]=1;
  base_production();assert(existing_facility_notice_calls==1 && production_spend_calls==0);check_popup_lifetime();
  reset_case();Bases[0].queue_production_id[0]=-32;Facility[32].cost=20;
  Bases[1].minerals_accumulated=100;Bases[1].queue_production_id[0]=-32;
  base_production();assert((Bases[1].facilities_built[4]&1)!=0 && Bases[0].facilities_built[4]==0);
  check_popup_lifetime();
  reset_case();Bases[0].queue_production_id[0]=-71;Facility[71].cost=20;
  Bases[1].minerals_accumulated=100;Bases[1].queue_production_id[0]=-71;
  base_production();assert(last_project_owner_base==1);check_popup_lifetime();cost_callback_switch=0;
  // Native queue redispatch retains the pointer published by base_queue, even
  // if that pointer and the selected index do not identify the same Base.
  reset_case();Bases[0].queue_production_id[0]=-32;Facility[32].cost=20;
  Bases[0].facilities_built[4]=1;Bases[1].queue_production_id[0]=3;Bases[1].minerals_accumulated=100;
  queue_callback_switch=1;base_production();assert(last_vehicle_x==Bases[1].x && veh_init_calls==1);
  check_popup_lifetime();queue_callback_switch=0;
  // Nonhuman project completion yields to a human rival using the original
  // nutrient-surplus quirk, not the more intuitive mineral-surplus ledger.
  reset_case();GameDataStateStorage.fields_.lock_masks_.enable_mask_=4;
  Bases[0].queue_production_id[0]=-71;Facility[71].cost=20;Bases[1].faction_id_current=2;
  Bases[1].queue_production_id[0]=-71;Bases[1].nutrient_surplus=15;
  Bases[1].minerals_accumulated=5;Bases[1].mineral_surplus=-100;
  base_production();assert(production_spend_calls==0 && project_owner_calls==0);check_popup_lifetime();
  reset_case();GameDataStateStorage.fields_.lock_masks_.enable_mask_=4;
  Bases[0].queue_production_id[0]=-71;Facility[71].cost=20;Bases[1].faction_id_current=2;
  Bases[1].queue_production_id[0]=-71;Bases[1].nutrient_surplus=INT_MAX;Bases[1].minerals_accumulated=1;
  base_production();assert(production_spend_calls==1 && project_owner_calls==1);check_popup_lifetime();
  reset_case();GameDataStateStorage.fields_.lock_masks_.enable_mask_=4;
  Bases[0].queue_production_id[0]=-102;Facility[102].cost=20;Bases[1].faction_id_current=2;
  Bases[1].queue_production_id[0]=-102;Bases[1].nutrient_surplus=100;ascent_rival_block=1;
  base_production();assert(production_spend_calls==1 && project_owner_calls==1);check_popup_lifetime();
  ascent_rival_block=0;
  reset_case();ConsoleGlobal.faction_id_=1;Bases[0].queue_production_id[0]=-32;
  Facility[32].cost=20;production_test_reveal=1;
  base_production();assert(reveal_calls==0 && draw_map_calls==0 && order_index(90)>=0 && order_index(90)<order_index(51));check_popup_lifetime();
  reset_case();ConsoleGlobal.faction_id_=1;Bases[0].queue_production_id[0]=-65;
  Facility[65].cost=20;production_test_reveal=1;
  base_production();assert(reveal_calls==1 && PlayersData[1].satellites_[0]==1 && draw_map_calls==1);
  assert(order_index(90)>=0 && order_index(90)<order_index(51));check_popup_lifetime();
  for(int mode=1;mode<=3;++mode){
    reset_case();ConsoleGlobal.faction_id_=1;Bases[0].queue_production_id[0]=-102;Facility[102].cost=20;
    GameDataStateStorage.fields_.game_preferences_=PREF_AV_SECRET_PROJECT_MOVIES;movie_switch=mode;
    base_production();assert(order_index(82)>=0 && (order_index(83)>=0)==(mode==1));check_popup_lifetime();
    IsMultiplayerNet=0;
  }
  movie_switch=0;
  // The entry-owned Popup is destroyed through a throwing provider boundary,
  // as well as all of the ordinary shortage/failure returns above. This host
  // control does not prove native unwind registration or provider exceptions.
  reset_case();cost_throw=1;bool caught=false;
  try{base_production();}catch(int value){caught=value==17;}
  assert(caught && production_spend_calls==0 && veh_init_calls==0);check_popup_lifetime();
}
'''


def test_complete_production_entry_records_callbacks_reconciliation_and_lifetime(tmp_path):
    run_actual(tmp_path,production_program())


@pytest.mark.parametrize("damage",("wrong_ai_owner","unconditional_flag","wrong_completed_domain","wrong_creation_actor","captured_cost_base","population_before_morale","reversed_cost_callbacks","early_facility_cost","leaked_popup","stale_facility_base","stale_project_base","stale_set_fac_base","recenter_redispatch","missing_rival_holdback","mineral_rival_ledger","unwrapped_rival_stock","missing_rival_ascent_gate","missing_completion_draw","ordinary_reveal","exclusive_movies","stale_movie_preferences","stale_movie_network"))
def test_production_entry_runtime_damage_controls(tmp_path,damage):
    text=production_program()
    changes={
        "wrong_ai_owner":("player_data.ai_power == 0","Players[faction_id].ai_power == 0"),
        "unconditional_flag":("if (faction_id == ConsoleGlobal.faction_id_) vehicle.flags |= 0x1000u;","vehicle.flags |= 0x1000u;"),
        "wrong_completed_domain":("production_late_reconcile(completed_item_id, completed_project_id,","production_late_reconcile(production_id, completed_project_id,"),
        "wrong_creation_actor":("const int faction_id = owner;","const int faction_id = BaseCurrent->faction_id_current;"),
        "captured_cost_base":("veh_cost(production_id, BaseIDCurrentSelected, 0)","veh_cost(production_id, 0, 0)"),
        "reversed_cost_callbacks":("        const int faction_cost = cost_factor(BaseCurrent->faction_id_current,\n                                             RSC_MINERALS, -1);\n        const int unit_cost = veh_cost(production_id, BaseIDCurrentSelected, 0);", "        const int unit_cost = veh_cost(production_id, BaseIDCurrentSelected, 0);\n        const int faction_cost = cost_factor(BaseCurrent->faction_id_current, RSC_MINERALS, -1);"),
        "early_facility_cost":("        const int faction_cost = cost_factor(faction_id, RSC_MINERALS, -1);\n        const int production_cost = production_wrapped_multiply(\n            faction_cost, Facility[facility_id].cost);", "        const int early_cost = Facility[facility_id].cost;\n        const int faction_cost = cost_factor(faction_id, RSC_MINERALS, -1);\n        const int production_cost = production_wrapped_multiply(faction_cost, early_cost);"),
        "leaked_popup":("    Popup popup;", "    Popup &popup = *new Popup;"),
        "stale_facility_base":("production_facility_built(BaseIDCurrentSelected, facility_id,","production_facility_built(0, facility_id,"),
        "stale_project_base":("production_set_project_owner(completed_project_id, BaseIDCurrentSelected)","production_set_project_owner(completed_project_id, 0)"),
        "stale_set_fac_base":("set_fac(completed_facility_id, BaseIDCurrentSelected, TRUE)","set_fac(completed_facility_id, 0, TRUE)"),
        "recenter_redispatch":("                production_id = BaseCurrent->queue_production_id[0];", "                BaseCurrent = &Bases[BaseIDCurrentSelected];\n                production_id = BaseCurrent->queue_production_id[0];"),
        "missing_rival_holdback":("if (completed_project_id >= 0 && !production_faction_human(owner))", "if (false)"),
        "mineral_rival_ledger":("rival.nutrient_surplus, rival.minerals_accumulated", "rival.mineral_surplus, rival.minerals_accumulated"),
        "unwrapped_rival_stock":("production_wrapped_add(\n                rival.nutrient_surplus, rival.minerals_accumulated)", "(rival.nutrient_surplus + rival.minerals_accumulated)"),
        "missing_rival_ascent_gate":("&& !ascending(rival_owner)","&& false"),
        "missing_completion_draw":("    draw_tile(BaseCurrent->x, BaseCurrent->y, 2);", "    (void)0;"),
        "ordinary_reveal":("            set_fac(completed_facility_id, BaseIDCurrentSelected, TRUE);", "            set_fac(completed_facility_id, BaseIDCurrentSelected, TRUE);\n            production_reveal_faction(owner);"),
        "exclusive_movies":("            }\n            if ((GameDataStateStorage.fields_.game_preferences_ & PREF_AV_SECRET_PROJECT_MOVIES)", "            } else if ((GameDataStateStorage.fields_.game_preferences_ & PREF_AV_SECRET_PROJECT_MOVIES)"),
        "stale_movie_preferences":("            if ((GameDataStateStorage.fields_.game_preferences_ & PREF_AV_SECRET_PROJECT_MOVIES) != 0\n                && !IsMultiplayerNet", "            if (true\n                && !IsMultiplayerNet"),
        "stale_movie_network":("                && !IsMultiplayerNet && owner != ConsoleGlobal.faction_id_", "                && owner != ConsoleGlobal.faction_id_"),
    }
    if damage=="population_before_morale":
        stores=re.search(r"        vehicle\.morale = static_cast<uint8_t>\(production_vehicle_morale\(.*?vehicle\.flags \|= 0x1000u;",text,re.S)[0]
        assert text.count(stores)==1
        text=text.replace(stores,"",1)
        anchor="        production_finish_vehicle(BaseIDCurrentSelected, faction_id, production_id,"
        assert text.count(anchor)==1
        run_actual(tmp_path,text.replace(anchor,stores+"\n"+anchor,1),False)
    else:
        old,new=changes[damage];assert text.count(old)==1
        run_actual(tmp_path,text.replace(old,new,1),False)


HELPER_PROVIDERS=r'''
RulesWeapon Weapon[26];RulesChassis Chassis[19];VehPrototype VehPrototypes[512];
struct {int faction_id_;} ConsoleGlobal;
char StringTemp[1024];int BaseComputedID;
static int ordinary_morale=2,morale_switch,parser_switch,parsed_slot,parsed_value;
static int popup_flags,cache_expected=-1;static const char *popup_picture;
static int finish_switch,finish_result=1,IsMultiplayerNet,UpgradePromptAnswered;
static char order[32];static unsigned order_count;
static void event(char c){assert(order_count<31);order[order_count++]=c;order[order_count]=0;}
static void bitmask_call(int id,int*offset,int*mask){*offset=id>>3;*mask=1<<(id&7);}
static void bitmask(int id,int*offset,int*mask){bitmask_call(id,offset,mask);}
static int has_fac(int id,int selected,int){return (Bases[selected].facilities_built[id>>3]&(1u<<(id&7)))!=0;}
static int morale_mod(int,int,int){if(morale_switch)BaseIDCurrentSelected=1;return ordinary_morale;}
static int has_abil(int id,int mask){return (VehPrototypes[id].ability_flags&mask)!=0;}
static int cost_factor(int faction,int,int){assert(faction==BaseCurrent->faction_id_current);event('C');return 3;}
static int veh_cost(int,int selected,int){assert(selected==BaseIDCurrentSelected);event('V');return 10;}
static void parse_num(int slot,int value){parsed_slot=slot;parsed_value=value;event('N');}
static void parse_says(int slot,const char *name,int,int){assert(slot==1 && name==VehPrototypes[64].veh_name);event('P');if(parser_switch){BaseCurrent=&Bases[1];BaseIDCurrentSelected=1;}}
static void popb(const char*,int flags,int,const char*picture,void*){popup_flags=flags;popup_picture=picture;event('O');}
static void base_support(){assert(BaseComputedID==cache_expected);event('S');}
static void base_yield(){assert(BaseComputedID==cache_expected);event('Y');}
static void base_nutrient(){assert(BaseComputedID==cache_expected);event('N');}
static void base_minerals(){assert(BaseComputedID==cache_expected);event('M');}
static void base_energy(){assert(BaseComputedID==cache_expected);event('E');}
static bool production_faction_human(int owner){return (GameDataStateStorage.fields_.lock_masks_.enable_mask_&0xFFu)&(1u<<owner);}
static int base_queue(int selected){assert(selected==BaseIDCurrentSelected);event('Q');if(finish_switch){BaseCurrent=&Bases[1];BaseIDCurrentSelected=1;}return finish_result;}
static void base_reset(int selected,int){assert(selected==BaseIDCurrentSelected);event('B');if(finish_switch){BaseCurrent=&Bases[1];BaseIDCurrentSelected=1;}}
static int prototype_notices,upgrade_calls,native_notices,naval_notices,air_notices;
static void mon_prototype_built(int){++prototype_notices;}static void upgrade_prototypes(int,int){++upgrade_calls;}
static void mon_native_life_bred(int){++native_notices;}static void mon_naval_unit_built(int){++naval_notices;}static void mon_air_unit_built(int){++air_notices;}
'''
HELPER_MAIN=r'''
static void reset(){
  memset(Bases,0,sizeof Bases);memset(Players,0,sizeof Players);memset(PlayersData,0,sizeof PlayersData);
  memset(VehPrototypes,0,sizeof VehPrototypes);memset(Weapon,0,sizeof Weapon);memset(Chassis,0,sizeof Chassis);
  memset(&GameDataStateStorage,0,sizeof GameDataStateStorage);
  for(int i=0;i<64;++i)GameDataStateStorage.fields_.secret_project_[i]=-1;
  Bases[0].faction_id_current=1;Bases[1].faction_id_current=2;BaseCurrent=&Bases[0];BaseIDCurrentSelected=0;
  ordinary_morale=2;morale_switch=parser_switch=0;parsed_slot=parsed_value=popup_flags=0;popup_picture=0;
  order_count=0;order[0]=0;ConsoleGlobal.faction_id_=1;
  finish_switch=IsMultiplayerNet=0;finish_result=1;
  prototype_notices=upgrade_calls=native_notices=naval_notices=air_notices=0;UpgradePromptAnswered=17;
}
int main(){
  assert(sizeof(Base)==0x134 && sizeof(PlayerData)==0x20CC && offsetof(PlayerData,ai_power)==0x3EC);
  reset();Weapon[0].offense_rating=-1;
  assert(production_vehicle_morale(0,1,1,false)==1);
  assert(production_vehicle_morale(0,1,64,false)==3); // custom negative-offense is ordinary morale
  assert(production_vehicle_morale(0,1,64,true)==4);
  reset();ordinary_morale=INT_MAX;assert(production_vehicle_morale(0,1,64,false)==0);
  reset();VehPrototypes[64].weapon_id=21;morale_switch=1;
  Bases[1].facilities_built[34>>3]=1u<<(34&7);
  assert(production_vehicle_morale(0,1,64,false)==5 && BaseIDCurrentSelected==1);
  // Complete notice uses the prototype name and the live Base after parser.
  reset();BaseCurrent->governor=0x80000001u;GameDataStateStorage.fields_.game_warnings_=0x2000;
  BaseCurrent->queue_size=2;production_vehicle_notice(0,1,64,false,1);
  assert(strcmp(StringTemp,"PRODUCEQ")==0 && popup_flags==0x400 && strcmp(popup_picture,"unit_sm.pcx")==0);
  assert(strcmp(order,"PNO")==0 && parsed_slot==1 && parsed_value==3);
  reset();BaseCurrent->governor=0x80000001u;GameDataStateStorage.fields_.game_warnings_=0x20000000;
  production_vehicle_notice(0,1,64,false,1);assert(popup_flags==0);
  reset();GameDataStateStorage.fields_.game_warnings_=4;BaseCurrent->queue_size=0xFFFFFFFFu;
  production_vehicle_notice(0,1,64,true,1);
  assert(strcmp(StringTemp,"PRODUCEPROTOQ")==0 && popup_flags==4 && parsed_value==0);
  assert(strcmp(order,"PCVNNO")==0);
  reset();production_vehicle_notice(0,1,64,true,0);assert(popup_flags==0x400);
  reset();Weapon[0].offense_rating=-1;
  production_vehicle_notice(0,1,64,false,0);assert(strcmp(popup_picture,"unit_sm.pcx")==0);
  Players[1].rule_flags=RFLAG_ALIEN;
  production_vehicle_notice(0,1,64,false,0);assert(strcmp(popup_picture,"alunit_sm.pcx")==0);
  reset();parser_switch=1;Bases[1].governor=0x80000001u;
  production_vehicle_notice(0,1,64,false,0);
  assert(BaseCurrent==&Bases[1] && popup_flags==0 && strcmp(StringTemp,"PRODUCEG")==0);
  reset();BaseComputedID=0;cache_expected=2;
  production_refresh_base_if_needed(2);assert(BaseComputedID==2 && strcmp(order,"SYNME")==0);
  order_count=0;order[0]=0;production_refresh_base_if_needed(2);assert(order_count==0);
  reset();GameDataStateStorage.fields_.lock_masks_.enable_mask_=2;finish_switch=1;
  Bases[1].governor=0x80000001u;production_finish_vehicle(0,1,64,false);
  assert(BaseCurrent==&Bases[1] && BaseIDCurrentSelected==1 && strcmp(StringTemp,"PRODUCEQ")==0);
  assert((Bases[0].state&BSTATE_PRODUCTION_DONE)!=0);
  reset();finish_switch=1;production_finish_vehicle(0,1,64,false);
  assert(BaseCurrent==&Bases[1] && BaseIDCurrentSelected==1 && strcmp(order,"B")==0);
  assert(UpgradePromptAnswered==17 && upgrade_calls==0 && prototype_notices==0);
  reset();GameDataStateStorage.fields_.lock_masks_.enable_mask_=2;IsMultiplayerNet=1;
  production_finish_vehicle(0,1,64,false);assert(VehPrototypes[64].flags==0 && UpgradePromptAnswered==17);
  order_count=0;production_finish_vehicle(0,1,64,true);
  assert(VehPrototypes[64].flags==0x100 && prototype_notices==1);
  reset();GameDataStateStorage.fields_.lock_masks_.enable_mask_=2;finish_result=0;
  BaseCurrent->governor=0x80000001u;production_finish_vehicle(0,1,64,false);
  assert(strcmp(order,"QPOB")==0); // post-notice full-auto reset
  reset();Chassis[0].triad=TRIAD_SEA;Weapon[0].offense_rating=-1;
  production_finish_vehicle(0,1,1,false);assert(native_notices==1 && naval_notices==0 && air_notices==0);
  reset();Chassis[0].triad=TRIAD_SEA;
  production_finish_vehicle(0,1,64,false);assert(naval_notices==0);
  Weapon[0].offense_rating=1;production_finish_vehicle(0,1,64,false);assert(naval_notices==1);
}
'''


def helper_program():
    src=source();veh=(ROOT/"src/veh.h").read_text();base=(ROOT/"src/base.h").read_text()
    declarations_="\n".join(definition(veh,sig)+";" for sig in ("struct RulesWeapon {","struct RulesChassis {","struct VehPrototype {","enum VehMorale {","enum VehTriad {","enum VehAbilitiesBitfield {"))
    declarations_+=definition(base,"enum BaseStateBitfield {")+";"
    constants="\n".join(re.search(r"^static const int "+name+r" = [^;]+;",veh,re.M)[0] for name in ("MaxVehProtoNum","MaxVehProtoFactionNum"))
    queries="\n".join(definition(base,sig) for sig in ("MEASURED inline int __cdecl base_project(","MEASURED inline BOOL __cdecl has_project(","static __forceinline int has_fac_built_call("))
    queries=queries.replace("__forceinline","inline")
    return (passive_common()+declarations_+constants+HELPER_PROVIDERS+queries
            +helper(src,"production_wrapped_multiply")
            +actual(src,"int production_vehicle_morale(")+actual(src,"void production_vehicle_notice(")
            +actual(src,"void production_refresh_base_if_needed(")
            +actual(src,"void production_finish_vehicle(")
            +HELPER_MAIN)


def test_complete_production_morale_notice_and_cache_helpers(tmp_path):
    run_actual(tmp_path,helper_program())


@pytest.mark.parametrize("damage",("allocation_cutoff","wrong_spy_base","unwrapped_morale","wrong_notice_mask","native_custom_image","missing_cache_publish","missing_prototype_parser","warning_suffix_skip","recenter_after_queue","unguarded_upgrade_block","missing_second_reset","native_naval_fallthrough","zero_offense_naval"))
def test_production_helper_runtime_damage_controls(tmp_path,damage):
    text=helper_program()
    changes={
        "allocation_cutoff":("prototype_id < MaxVehProtoFactionNum\n        &&", "prototype_id < MaxVehProtoNum\n        &&"),
        "wrong_spy_base":("has_fac_built_call(FAC_COVERT_OPS_CENTER, BaseIDCurrentSelected)","has_fac_built_call(FAC_COVERT_OPS_CENTER, selected_base_id)"),
        "unwrapped_morale":("    morale = base_yield_add(morale, 1);\n    if (morale <", "    ++morale;\n    if (morale <"),
        "wrong_notice_mask":("(warnings & 0x2000u)","(warnings & 0x20000000u)"),
        "native_custom_image":("prototype_id < MaxVehProtoFactionNum && native_life","prototype_id >= 64 || native_life"),
        "missing_cache_publish":("        BaseComputedID = selected_base_id;","        (void)selected_base_id;"),
        "missing_prototype_parser":("    parse_says(1, VehPrototypes[prototype_id].veh_name, -1, -1);","    (void)prototype_id;"),
        "warning_suffix_skip":("    if (queue_result != 0) {\n        parse_num(1, base_yield_add", "    if (queue_result != 0 && !route_to_warning) {\n        parse_num(1, base_yield_add"),
        "recenter_after_queue":("        const int queue_result = base_queue(BaseIDCurrentSelected);", "        const int queue_result = base_queue(BaseIDCurrentSelected);\n        BaseCurrent = &Bases[selected_base_id];"),
        "unguarded_upgrade_block":("    if (new_prototype) {", "    if (true) {"),
        "missing_second_reset":("        if (queue_result == 0", "        if (false"),
        "native_naval_fallthrough":("        mon_native_life_bred(owner);\n    } else {", "        mon_native_life_bred(owner);\n    } {"),
        "zero_offense_naval":("triad == TRIAD_SEA && Weapon[prototype.weapon_id].offense_rating != 0", "triad == TRIAD_SEA"),
    }
    old,new=changes[damage];assert text.count(old)==1,(damage,text.count(old))
    run_actual(tmp_path,text.replace(old,new,1),False)


DUPLICATE_MAIN=r'''
int main(){
  BaseCurrent=&Bases[0];Bases[0].faction_id_current=1;
  GameDataStateStorage.fields_.lock_masks_.enable_mask_=2;
  GameDataStateStorage.fields_.secret_project_[0]=2;Bases[2].faction_id_current=1;
  Players[1].is_noun_plural=1;Players[1].noun_gender=2;
  production_existing_project_notice(1,70,0);
  assert(strcmp(StringTemp,"ALREADYPROJECT2")==0 && GenderDefault==2 && PluralityDefault==1);
}
'''


def duplicate_program():
    text=helper_program().split(HELPER_MAIN)[0]
    text=text.replace(definition(text,"static void parse_says("), "static void parse_says(int slot,const char*,int,int){if(slot==3)Bases[2].faction_id_current=2;}")
    return text+"static void parse_say(int,const char*,int,int){}\n"+actual(source(),"void production_existing_project_notice(")+DUPLICATE_MAIN


def test_complete_duplicate_project_notice_reloads_same_completed_base_after_phrase(tmp_path):
    run_actual(tmp_path,duplicate_program())


def test_duplicate_project_cached_owner_damage_control(tmp_path):
    text=duplicate_program();old="completed.faction_id_current == owner ? \"1\" : \"2\""
    assert text.count(old)==1
    run_actual(tmp_path,text.replace(old,"completed_owner == owner ? \"1\" : \"2\"",1),False)


PROMPT_MAIN=r'''
int main(){
  BaseCurrent=&Bases[0];Bases[0].faction_id_current=1;Bases[0].population_size=3;
  GameDataStateStorage.fields_.lock_masks_.enable_mask_=2;prompt_result=2;
  assert(production_colony_abandon_prompt(1,64));
  assert(strcmp(order,"01Co")==0 && prompt_calls==1 && parsed_value==0);
  order_count=0;order[0]=0;IsMultiplayerNet=1;prompt_calls=0;
  assert(!production_colony_abandon_prompt(1,64) && prompt_calls==0 && strcmp(order,"01C")==0);
  assert(BaseProductionZoomRequested==0);IsMultiplayerNet=0;
  order_count=0;order[0]=0;prompt_parse_switch=prompt_cost_switch=1;
  Bases[1].faction_id_current=2;Bases[2].faction_id_current=3;Bases[2].population_size=5;
  Bases[2].nutrients_accumulated=2;Bases[2].nutrient_surplus=2;Players[3].rule_flags=RFLAG_ALIEN;
  assert(production_colony_abandon_prompt(1,64));
  assert(prompt_cost_owner==2 && BaseCurrent==&Bases[2] && parsed_slot==0 && parsed_value==8);
  assert(strcmp(prompt_picture,"alopdir.pcx")==0 && strcmp(order,"01CNo")==0);
}
'''


def prompt_program():
    text=helper_program().split(HELPER_MAIN)[0]
    text="static int prompt_parse_switch,prompt_cost_switch,prompt_cost_owner,prompt_calls,prompt_result,BaseProductionZoomRequested;\nchar prompt_script[] = \"PROMPT\";char *ScriptPopupFile=prompt_script;\n"+text
    replacement=r'''
static const char *prompt_picture;
static void parse_says(int slot,const char*,int,int){event(slot==0?'0':'1');if(slot==0 && prompt_parse_switch){BaseCurrent=&Bases[1];BaseIDCurrentSelected=1;}}
static int popp(const char*,const char*,int,const char *picture,int){++prompt_calls;prompt_picture=picture;event('o');return prompt_result;}
'''
    text=text.replace(definition(text,"static void parse_says("),replacement,1)
    text=text.replace(definition(text,"static int cost_factor("),"static int cost_factor(int owner,int,int){prompt_cost_owner=owner;event('C');if(prompt_cost_switch){BaseCurrent=&Bases[2];BaseIDCurrentSelected=2;}return 3;}",1)
    return text+helper(source(),"production_wrapped_add")+helper(source(),"production_wrapped_subtract")+actual(source(),"bool production_colony_abandon_prompt(")+PROMPT_MAIN


def test_complete_colony_prompt_unconditional_cost_live_base_and_network_skip(tmp_path):
    run_actual(tmp_path,prompt_program())


@pytest.mark.parametrize("damage",("skip_nonpositive_cost","popup_in_network","captured_picture_owner","precallback_population"))
def test_colony_prompt_runtime_damage_controls(tmp_path,damage):
    text=prompt_program()
    changes={
        "skip_nonpositive_cost":("const int faction_cost = cost_factor(BaseCurrent->faction_id_current, 0,", "const int faction_cost = BaseCurrent->nutrient_surplus <= 0 ? 0 : cost_factor(BaseCurrent->faction_id_current, 0,"),
        "popup_in_network":("const int result = IsMultiplayerNet ? 0\n        : popp", "const int result = popp"),
        "captured_picture_owner":("Players[BaseCurrent->faction_id_current].rule_flags", "Players[owner].rule_flags"),
        "precallback_population":("faction_cost, production_wrapped_add(BaseCurrent->population_size, 1)", "faction_cost, production_wrapped_add(Bases[0].population_size, 1)"),
    }
    old,new=changes[damage];assert text.count(old)==1
    run_actual(tmp_path,text.replace(old,new,1),False)


REPORT_PROVIDERS=r'''
RulesBasic Rules;struct {int faction_id_;} ConsoleGlobal;
char StringTemp[1024];char script_text[]="SCRIPT",caption_text[]="CAPTION";
char *ScriptPopupFile=script_text,*PopupStartCaption=caption_text;int WinScreenWidth=800;
static int clear_warning_on_width,fade_prefs_on_popup,starts,strings,announcements,audio_loads,audio_plays,fades;
static int parsed_base_names,popup_flags,interlude_calls,executions,picture_selections;
static bool announced;static char events[128];static unsigned event_count;
static void event(char c){assert(event_count<127);events[event_count++]=c;events[event_count]=0;}
struct Sound{void fade(int value){assert(value==2000);++fades;event('F');}};
struct Wave:Sound{void unload(){event('U');}void load(const char *p){assert(strncmp(p,"voices\\fac",10)==0);++audio_loads;event('L');}void play(){++audio_plays;event('A');}} g_WAVE_GENERAL;
struct Picture{int init(char*,int,int){return 0;}};
struct Popup{
  Picture picture;
  void init(int,int){event('I');}void set_width(int){event('W');if(clear_warning_on_width)GameDataStateStorage.fields_.game_warnings_=0;}
  void start(char*,const char*,int,int,int,int){++starts;event('S');}void string(char*){++strings;event('T');}
  Picture &owned_picture(){return picture;}void select_owned_picture(){++picture_selections;}void exec(int,int){++executions;}
};
struct Buffer{static int get_pcx_dimensions(const char*,int*w,int*h){*w=*h=32;return 0;}};
static int has_fac_announced(uint32_t,uint32_t){return announced;}
static void set_fac_announced(uint32_t,uint32_t,int){++announcements;}
static void say_num(int value){char buffer[32];sprintf(buffer,"%d",value);strcat(StringTemp,buffer);}
static void parse_say(int,const char*,int,int){event('f');}
static void parse_says(int slot,const char *p,int,int){if(slot==0){assert(p==BaseCurrent->name_string);++parsed_base_names;event('b');}else event('p');}
static void parse_num(int,int){}
static void popb(const char*,int flags,int,const char*,void*){popup_flags=flags;event('O');if(fade_prefs_on_popup)GameDataStateStorage.fields_.game_more_preferences_|=0x20000;}
static int popp(const char *script,const char*,int,const char*,int){assert(script==ScriptPopupFile);event('o');return 0;}
static void interlude(int id,const char*,int,int){assert(id==0x26);++interlude_calls;}
static int generators(int owner,int *small){assert(owner==1);*small=2;return 3;}
'''
REPORT_MAIN=r'''
static void reset(){
  memset(Bases,0,sizeof Bases);memset(Players,0,sizeof Players);memset(&GameDataStateStorage,0,sizeof GameDataStateStorage);
  BaseCurrent=&Bases[0];BaseIDCurrentSelected=0;ConsoleGlobal.faction_id_=1;
  Bases[0].faction_id_current=1;GameDataStateStorage.fields_.lock_masks_.enable_mask_=2;
  clear_warning_on_width=fade_prefs_on_popup=starts=strings=announcements=audio_loads=audio_plays=fades=0;
  parsed_base_names=popup_flags=interlude_calls=executions=picture_selections=0;announced=false;event_count=0;events[0]=0;
}
int main(){
  Popup popup;
  reset();GameDataStateStorage.fields_.game_warnings_=1;GameDataStateStorage.fields_.game_more_preferences_=0x4000;
  clear_warning_on_width=fade_prefs_on_popup=1;
  assert(production_report_stage_v3(popup,1,32,-1,0,false)==PRODUCTION_REPORT_RESUME_2752);
  assert(parsed_base_names==1 && starts==0 && strings==0 && announcements==0);
  assert(audio_loads==1 && audio_plays==1 && fades==1 && strcmp(events,"fpbIWULAOF")==0);
  reset();GameDataStateStorage.fields_.game_warnings_=1;
  production_report_stage_v3(popup,1,32,-1,0,false);
  assert(starts==1 && strings==3 && announcements==1 && parsed_base_names==1);
  reset();production_report_stage_v3(popup,1,32,-1,0,false);assert(popup_flags==1);
  reset();production_report_stage_v3(popup,1,32,-1,1,false);assert(popup_flags==0);
  reset();GameDataStateStorage.fields_.game_warnings_=0x2000;
  production_report_stage_v3(popup,1,32,-1,1,false);assert(popup_flags==1);
  reset();BaseCurrent->governor=0x80000001u;
  production_report_stage_v3(popup,1,32,-1,0,false);assert(popup_flags==0);
  reset();ConsoleGlobal.faction_id_=2;Players[1].rule_flags=RFLAG_ALIEN;
  production_report_stage_v3(popup,1,40,-1,0,false);assert(interlude_calls==1 && parsed_base_names==1);
  reset();ConsoleGlobal.faction_id_=2;Players[2].rule_flags=RFLAG_ALIEN;
  production_report_stage_v3(popup,1,40,-1,0,false);assert(interlude_calls==0);
  reset();ConsoleGlobal.faction_id_=2;
  production_report_stage_v3(popup,1,101,31,0,false);
  assert(starts==2 && strings==2 && executions==1 && picture_selections==1 && parsed_base_names==1);
}
'''


def report_program():
    src=source();base=(ROOT/"src/base.cpp").read_text();alpha=(ROOT/"src/alpha.h").read_text()
    return (passive_common()+definition(alpha,"struct RulesBasic {")+";"
            +definition(base,"enum ProductionReportResume {")+";"+REPORT_PROVIDERS
            +helper(src,"production_wrapped_add")+actual(src,"int production_report_signed_bits(")
            +actual(src,"ProductionReportResume production_report_stage(")
            +actual(src,"static void production_report_foreign_stage_v3(")
            +actual(src,"static ProductionReportResume production_report_stage_v3(")+REPORT_MAIN)


def test_complete_report_callbacks_audio_warning_flags_foreign_owners_and_picture(tmp_path):
    run_actual(tmp_path,report_program())


@pytest.mark.parametrize("damage",("warning_not_rechecked","missing_live_name","wrong_audio_bit","wrong_fade_owner","ordinary_warning_mask","foreign_producer_alien"))
def test_report_runtime_damage_controls(tmp_path,damage):
    text=report_program()
    changes={
        "warning_not_rechecked":("        if ((GameDataStateStorage.fields_.game_warnings_ & 1u) != 0) {", "        if (true) {"),
        "missing_live_name":("        parse_says(0, BaseCurrent->name_string, -1, -1);\n        popup.init(0, 0);", "        popup.init(0, 0);"),
        "wrong_audio_bit":("game_more_preferences_ & 0x00004000u", "game_more_preferences_ & 0x00400000u"),
        "wrong_fade_owner":("game_more_preferences_ & 0x00020000u", "game_warnings_ & 0x00020000u"),
        "ordinary_warning_mask":("game_warnings_ & 0x00002000u", "game_warnings_ & 0x00200000u"),
        "foreign_producer_alien":("Players[ConsoleGlobal.faction_id_].rule_flags", "Players[owner].rule_flags"),
    }
    old,new=changes[damage]
    assert text.count(old)==(2 if damage in {"wrong_audio_bit","missing_live_name"} else 1)
    run_actual(tmp_path,text.replace(old,new,1),False)


VOICE_PROVIDERS=r'''
struct {int faction_id_;int focus(int,int,int){return 0;}} ConsoleGlobal;
const int RadiusOffsetX[289]={},RadiusOffsetY[289]={};uint32_t MapTileFlagClearMask=0x800;
static int random_values[8],random_calls,territory_calls,draw_calls,fungus_calls,clear_calls,last_x,last_y;
static int test_rand(){assert(random_calls<8);return random_values[random_calls++];}
#define rand test_rand
static int is_alive(int faction){return faction==1 || faction==2;}
static Map *map_loc(int x,int y){assert(x>=0 && x<(int)MapLongitudeBounds && y>=0 && y<(int)MapLatitudeBounds);return &MapTiles[(x>>1)+y*MapLongitude];}
static int whose_territory(int owner,int,int,int,int){++territory_calls;return owner;}
static void bit_set_call(int x,int y,uint32_t mask,int value){last_x=x;last_y=y;if(value){assert(mask==BIT_FUNGUS);++fungus_calls;}else{assert(mask==MapTileFlagClearMask);++clear_calls;}}
static void synch_bit(int,int,int faction){assert(faction==ConsoleGlobal.faction_id_);}
static void draw_tile(int,int,int){++draw_calls;}
static void bitmask(int id,int*offset,int*mask){*offset=id>>3;*mask=1<<(id&7);}
'''
VOICE_MAIN=r'''
static Map tiles[16];
static void reset(){
  memset(Bases,0,sizeof Bases);memset(PlayersData,0,sizeof PlayersData);memset(tiles,0,sizeof tiles);
  memset(random_values,0,sizeof random_values);memset(&MapStorage,0,sizeof MapStorage);
  MapStorage.tiles_=tiles;MapLongitude=4;MapLongitudeBounds=8;MapLatitudeBounds=4;MapIsFlat=1;MapArea=10;
  GameDataStateStorage.fields_.base_current_count_=2;ConsoleGlobal.faction_id_=1;
  Bases[0].x=2;Bases[0].y=2;Bases[0].faction_id_current=1;
  Bases[1].x=4;Bases[1].y=2;Bases[1].faction_id_current=2;
  tiles[9].climate=tiles[10].climate=0x60;tiles[9].bit=tiles[10].bit=BIT_BASE_IN_TILE;
  tiles[9].val2=tiles[10].val2=8;random_calls=territory_calls=draw_calls=fungus_calls=clear_calls=last_x=last_y=0;
}
int main(){
  // Positive initialized redispatch storage: the same ordinary-facility
  // bitmask output feeds the complete actual Voice helper. Never run the
  // direct entry's indeterminate scratch through a host execution control.
  reset();Bases[0].facilities_built[4]=1;int counter=777;
  assert(production_facility_built(0,32,counter) && counter==4);
  production_voice_of_planet_spread(counter);
  assert(counter==6 && territory_calls==2 && fungus_calls==19 && clear_calls==19 && draw_calls==19);
  reset();counter=998;random_values[2]=3;random_values[3]=1;
  production_voice_of_planet_spread(counter);
  assert(counter==1000 && territory_calls==1 && fungus_calls==10 && clear_calls==10);
  assert(random_calls==4 && last_x==3 && last_y==1);
  reset();counter=999;production_voice_of_planet_spread(counter);
  assert(counter==1001 && territory_calls==0 && fungus_calls==1);
  reset();GameDataStateStorage.fields_.base_current_count_=1;counter=INT_MAX;
  production_voice_of_planet_spread(counter);assert(counter==INT_MIN && territory_calls==1 && fungus_calls==10);
  reset();MapArea=0xFFFFFFFFu;counter=999;
  assert(production_reveal_faction(1)); // signed negative area skips tile walk
  assert(tiles[9].visibility==0 && Bases[0].visibility==2 && (PlayersData[1].flags&PFLAG_MAP_REVEALED)!=0);
  assert(!production_reveal_faction(1));
}
'''


def voice_program():
    src=source();mapping=(ROOT/"src/map.h").read_text()
    return (passive_common()+map_types_aliases()+definition(mapping,"enum TerrainBit {")+";"+VOICE_PROVIDERS
            +definition(mapping,"MEASURED inline int __cdecl xrange(")
            +helper(src,"production_wrapped_add")+actual(src,"bool production_facility_built(")
            +actual(src,"void production_voice_of_planet_spread(")+actual(src,"bool production_reveal_faction(")
            +VOICE_MAIN)


def test_complete_voice_initialized_redispatch_persistent_counter_cluster_global_pass_and_reveal(tmp_path):
    run_actual(tmp_path,voice_program())


@pytest.mark.parametrize("damage",("counter_reset","unwrapped_counter","late_cutoff","cluster_territory","climate_occupant","missing_global_pass","unsigned_reveal_walk"))
def test_voice_initialized_runtime_damage_controls(tmp_path,damage):
    text=voice_program()
    changes={
        "counter_reset":("        int center_x = 0;", "        offset_attempt = 0;\n        int center_x = 0;"),
        "unwrapped_counter":("offset_attempt = production_wrapped_add(offset_attempt, 1)", "++offset_attempt"),
        "late_cutoff":("if (offset_attempt >= 1000) break;", "if (offset_attempt > 1000) break;"),
        "cluster_territory":("            bit_set_call(x_coord, y_coord, BIT_FUNGUS, TRUE);", "            if (whose_territory(owner, x_coord, y_coord, 0, 0) != owner) continue;\n            bit_set_call(x_coord, y_coord, BIT_FUNGUS, TRUE);"),
        "climate_occupant":("center_tile->val2 & 0x0f", "center_tile->climate & 0x0f"),
        "missing_global_pass":("const int attempts = base_yield_signed_bits(MapArea) / 10;", "const int attempts = 0;"),
        # Observe the wrong walk on the negative-area case without overrunning
        # this fixture's sixteen allocated tiles. The final visibility assertion
        # rejects these in-bounds writes; a sanitizer fault/timeout is not proof.
        "unsigned_reveal_walk":("tile_index < tile_count", "static_cast<uint32_t>(tile_index) < MapArea && tile_index < 16"),
    }
    old,new=changes[damage];assert text.count(old)==1
    run_actual(tmp_path,text.replace(old,new,1),False)


def test_direct_voice_indeterminate_entry_is_retained_not_host_normalized():
    main=actual(source(),"void __cdecl base_production()")
    voice=actual(source(),"void production_voice_of_planet_spread(")
    assert re.search(r"\bint production_bitmask_offset;",main)
    assert not re.search(r"production_bitmask_offset\s*=",main)
    assert "production_voice_of_planet_spread(production_bitmask_offset)" in main
    assert "offset_attempt = production_wrapped_add(offset_attempt, 1)" in voice
    assert not re.search(r"offset_attempt\s*=\s*0",voice)
    assert "native compiler/runtime effect remains unproved" in main


def project_domain_program():
    src=source()
    return (passive_common()+actual(src,"int production_abs(")+actual(src,"int production_project(")
            +actual(src,"void production_set_project_owner(")+r'''
int main(){
  assert(production_abs(INT_MIN)==INT_MIN && production_abs(-71)==71);
  assert(production_project(INT_MIN)==2147483578 && production_project(-71)==1 && production_project(-69)==-1);
  production_set_project_owner(0,3);production_set_project_owner(63,4);
  production_set_project_owner(-1,99);
  assert(GameDataStateStorage.fields_.secret_project_.human_genome_project()==3);
  assert(GameDataStateStorage.fields_.secret_project_.user_defined_64()==4);
}
''')


def test_complete_project_queue_domains_wrap_and_valid_unchecked_owner_storage(tmp_path):
    run_actual(tmp_path,project_domain_program())
    setter=actual(source(),"void production_set_project_owner(")
    # Invalid >=64 indexed stores are retained as pin-only unsafe evidence.
    # Never invent an enlarged host member or execute that member overrun.
    assert "secret_project_[project_id] = base_id" in setter
    assert "switch" not in setter and not re.search(r"project_id\s*(?:<|<=)\s*",setter)


@pytest.mark.parametrize("damage",("signed_abs_negation","signed_project_negation"))
def test_project_domain_runtime_damage_controls(tmp_path,damage):
    text=project_domain_program()
    old,new=("value < 0 ? base_yield_sub(0, value)","value < 0 ? -value") if damage=="signed_abs_negation" else ("base_yield_sub(base_yield_sub(0, item), FacilitySPStart)","base_yield_sub(-item, FacilitySPStart)")
    assert text.count(old)==1
    run_actual(tmp_path,text.replace(old,new,1),False)


def test_independent_pinned_complete_spans_and_native_owner_operands():
    import struct
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    capstone=pytest.importorskip("capstone")
    path=Path(os.environ.get("OPENSMACX_IMAGE",ROOT/".opensmacx/game/terranx_original.exe"))
    if not path.exists() and "OPENSMACX_IMAGE" not in os.environ:
        pytest.skip("pinned image unavailable; set OPENSMACX_IMAGE")
    image=PeImage(path.read_bytes(),PIN);decoder=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32)
    rows={}
    for spans in SUBJECTS.values():
        for lo,hi in spans:
            decoded=list(decoder.disasm(image.read(Span(lo,hi)),lo))
            assert sum(x.size for x in decoded)==hi-lo
            rows.update({x.address:(x.mnemonic,x.op_str) for x in decoded})
    for address,expected in {
        0x4F09B0:("neg","ecx"),0x4F09B5:("mov","dword ptr [ebp - 0x14], ecx"),
        0x4F3435:("neg","ecx"),0x4F3870:("neg","eax"),
        0x4F113D:("mov","esi, dword ptr [edx + 0x96cdcc]"),
        0x4F11EB:("cmp","eax, 0x40"),0x4F13D9:("cmp","edi, eax"),
        0x4F13C5:("mov","byte ptr [esi + 0x95284c], al"),0x4F1441:("dec","cl"),
        0x4EF469:("mov","ecx, dword ptr [eax]"),0x4EF5AA:("mov","esi, dword ptr [ecx]"),
        0x4F7F05:("mov","cl, byte ptr [0x9a64c0]"),0x4F7FB8:("mov","ecx, dword ptr [0x689370]"),
        0x4F0B4D:("call","0x4e4430"),0x4F0B52:("mov","edx, dword ptr [0x689370]"),
        0x4F0B5E:("call","0x5c1850"),0x4F09FE:("imul","edi, dword ptr [edx + 0x9a4b74]"),
        0x4EF50E:("mov","edi, dword ptr [0x689370]"),
        0x4F0FEE:("mov","dword ptr [0x689374], esi"),
        0x4F15AC:("test","dh, 0x20"),0x4F23BB:("test","byte ptr [0x9a6498], 1"),
        0x4F0A52:("mov","esi, dword ptr [0x689370]"),
        0x4F0A67:("lea","ecx, [ebp - 0x10]"),
        0x4F2083:("mov","edx, dword ptr [0x689370]"),
        0x4F2193:("mov","esi, dword ptr [0x689370]"),
        0x4F2239:("call","0x46af40"),0x4F20E7:("jle","0x4f2108"),
        # The entry bypasses the starvation cleanup pointer assignment;
        # direct Voice reaches the reused slot without a proved initializer.
        0x4F0871:("jge","0x4f08ec"),0x4F08A9:("mov","dword ptr [ebp - 0x10], eax"),
        0x4F2C5C:("mov","eax, dword ptr [ebp - 0x10]"),
        0x4F2C5F:("inc","eax"),0x4F2C60:("cmp","eax, 0x3e8"),
        0x4F2C68:("jge","0x4f2dd1"),0x65D0CC:("mov","eax, 0x67b308"),
    }.items():
        assert rows[address]==expected
    info=struct.unpack("<8I",image.read(Span(0x67B308,0x67B328)))
    assert info[:3]==(0x19930520,151,0x67B328)
    # These are native mapping/unwind-table anchors, not proof of the current
    # C++ compiler's selected states or host Popup provider ABI.
    unwind=image.read(Span(info[2],info[2]+info[1]*8))
    states=struct.unpack("<"+"II"*info[1],unwind)
    assert states[0]==0xFFFFFFFF and states[1]==0x65C8A0
    assert all(action==0 or 0x65C8A0<=action<0x65D0CC for action in states[1::2])
