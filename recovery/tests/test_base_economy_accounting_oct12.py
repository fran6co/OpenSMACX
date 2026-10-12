"""Whole actual Base economy bodies; host controls are not native acceptance.

Passive declarations, globals, arithmetic helpers and inline owner queries are
read from current source. Service doubles make call boundaries observable; they
do not establish provider equivalence, native ABI, aliasing or full-game runtime.
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

ROOT = Path(__file__).resolve().parents[2]
BASE = "58563bc456781d3224dca06e4d52c5175691a29e"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
SUBJECTS = {
    0x4E9CB0: (0x4EA1EF, "void __cdecl base_minerals()"),
    0x4F6510: (0x4F65E7, "int __cdecl fac_maint("),
    0x4F65F0: (0x4F67E3, "void __cdecl base_maint()"),
}


def tokens(text):
    result, cursor = [], 0
    for match in COMMENTS_OR_STRINGS.finditer(text):
        result.extend(text[cursor:match.start()].split())
        if not match[0].startswith(("//", "/*")):
            result.append(match[0])
        cursor = match.end()
    return result + text[cursor:].split()


def test_exact_scope_and_all_historical_mapping_fields_survive():
    path = ROOT / "src/base.cpp"
    before = subprocess.check_output(["git", "show", BASE + ":src/base.cpp"], cwd=ROOT, text=True)
    current = path.read_text()
    # The separately reviewed production/growth/upkeep group is reversed only
    # against its exact 908fd predecessor. Its bridge preserves all mappings,
    # history and outside-group code; this older BASE/economy3 guard stays live.
    from test_base_production_growth_upkeep_oct12 import restore_group_to_baseline
    current = restore_group_to_baseline(current, path)
    old = {r.address: r for r in read_text(before, path)}
    new = {r.address: r for r in read_file(path)}
    assert new.keys() == old.keys()
    for address, record in new.items():
        for field in fields(record):
            if field.name not in {"line", "region", "origin"}:
                assert getattr(record, field.name) == getattr(old[address], field.name), (hex(address), field.name)
        def lessons(row):
            return [(x.kind, x.key, x.prose, tuple(t for _, t in x.lines)) for x in row.origin or ()]
        assert lessons(record) == lessons(old[address])
    # Only the explicitly authorized complete bodies may change. This does
    # not exempt mappings, history, other functions, globals or helpers.
    for _, signature in SUBJECTS.values():
        before = before.replace(definition(before, signature), "AUTHORIZED_ECONOMY_BODY", 1)
        current = current.replace(definition(current, signature), "AUTHORIZED_ECONOMY_BODY", 1)
    assert tokens(before) == tokens(current)


def helpers():
    source = (ROOT / "src/base.cpp").read_text()
    return "\n".join(definition(source, "int " + name + "(") for name in (
        "base_yield_signed_bits", "base_yield_add", "base_yield_sub", "base_yield_mul", "base_yield_shift"))


def declarations():
    base = (ROOT / "src/base.h").read_text()
    faction = (ROOT / "src/faction.h").read_text()
    game = (ROOT / "src/game.h").read_text()
    state = (ROOT / "src/game_data_state_window.h").read_text()
    mapping = (ROOT / "src/map.h").read_text()
    alpha = (ROOT / "src/alpha.h").read_text()
    selected = [(base, x) for x in ("enum FacilityId {", "enum  SecretProjectId {", "struct Base {", "struct RulesFacility {", "struct BaseSecretProject {")]
    selected += [(faction, x) for x in ("enum FactionSpecialRules {", "enum PlayerFlagsBitfield {", "struct SocialCategory {", "struct SocialEffect {", "struct Goal {", "struct Player {", "struct PlayerData {")]
    selected += [(game, x) for x in ("enum GameStateBitfield {", "enum GameRulesBitfield {", "enum GameDifficultyLevel {")]
    selected += [(state, x) for x in ("struct GameDataLockMaskBytes {", "union GameDataLockMaskView {", "struct GameDataStateWindowFields {", "struct GameDataStateWindow {")]
    selected += [(alpha, "struct RulesWorldbuilder {")]
    # The Map's actual passive window and aliases retain shared storage.
    selected += [(mapping, x) for x in ("enum ResourceType {", "struct Landmark {")]
    constants = "\n".join(re.search(r"^static const int " + name + r" = [^;]+;", base if name != "MaxPlayerNum" else faction, re.M)[0]
                            for name in ("MaxBaseNum", "MaxFacilityNum", "MaxSecretProjectNum", "FacilitySPStart", "FacilityRepStart", "SP_Unbuilt", "MaxPlayerNum"))
    types = "\n".join(definition(text, sig) + ";" for text, sig in selected)
    types += "\nconst int MaxLandmarkNum=64; struct Map;\n"
    types += "\n".join(definition(mapping, sig) + ";" for sig in ("struct MapMetadataFields {", "struct MapStorageWindow {"))
    aliases = "\n".join(re.search(r"^#define " + name + r" .*", mapping, re.M)[0] for name in ("MapSeaLevel", "MapNativeLifeForms"))
    globals_ = []
    for relative, names in {
        "src/alpha.cpp": ("WorldBuilder", "Players", "PlayersData", "Facility", "Bases"),
        "src/base.cpp": ("BaseCurrent", "BaseIDCurrentSelected", "BaseCurrentConvoyTo", "BaseCurrentConvoyFrom", "BaseCurrentForcesMaintCost"),
        "src/game.cpp": ("GameDataStateStorage", "TectonicDetonationCount"),
        "src/map.cpp": ("MapStorage",),
    }.items():
        source = (ROOT / relative).read_text()
        for name in names:
            globals_.append(re.search(r"^[A-Za-z][^\n;]*\b" + name + r"(?:\[[^\]]+\])?[^\n;]*;", source, re.M)[0])
    return ("#include <stdint.h>\n#include <stddef.h>\n#include <assert.h>\n#include <string.h>\n#include <limits.h>\n"
            "#define __cdecl\n#define MEASURED\n#define UNUSED(x)\ntypedef int BOOL; typedef char *LPSTR;\n" + constants + "\n" + types + "\n" + aliases + "\n" + "\n".join(globals_) + "\n")


PROVIDERS = r'''
// These service doubles expose boundaries, not original provider/runtime proof.
static int facility_calls[8], facility_call_count, human_reads;
static int switch_base_on_facility, drop_human_after_first;
static int reactor_result, reactor_mutates_difficulty, tech_result, tech_mutates_bonus;
static int reactor_calls, tech_calls, clear_calls, parse_calls, popup_calls;
static int cost_faction, cost_result, switch_on_clear;
static char events[32]; static unsigned event_count;
static void event(char c) { assert(event_count<31); events[event_count++]=c; events[event_count]=0; }
static int is_human(uint32_t faction) {
    ++human_reads;
    const int result=(GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFFu) & (1u<<faction);
    if (drop_human_after_first && human_reads==1) GameDataStateStorage.fields_.lock_masks_.enable_mask_=0;
    return result;
}
static void bitmask_call(int id,int *offset,int *mask) { *offset=id/8; *mask=1<<(id&7); }
static void bitmask(int id,int *offset,int *mask) { bitmask_call(id,offset,mask); }
static int has_fac(int id,int base_id,int queue) {
    assert(queue==0); facility_calls[facility_call_count++]=id;
    if (switch_base_on_facility && facility_call_count==switch_base_on_facility) BaseCurrent=&Bases[1];
    return (Bases[base_id].facilities_built[id>>3] & (1u<<(id&7)))!=0;
}
static int best_reactor(int faction) {
    ++reactor_calls; event('R');
    if (reactor_mutates_difficulty) PlayersData[faction].diff_level=100;
    return reactor_result;
}
static int has_tech(int technology,int faction) {
    ++tech_calls; event('T'); assert(technology==100 && faction==1);
    if (tech_mutates_bonus==1) Players[faction].faction_bonus_val1[0]=2;
    if (tech_mutates_bonus==2) Players[faction].faction_bonus_count=2;
    return tech_result;
}
static void set_fac(int id,int base_id,int value) {
    assert(value==0); event('C'); ++clear_calls;
    Bases[base_id].facilities_built[id>>3] &= ~(1u<<(id&7));
    if (switch_on_clear) { BaseCurrent=&Bases[1]; BaseIDCurrentSelected=1; }
}
static int cost_factor(int faction,int resource,int base_id) {
    assert(resource==1 && base_id==-1); event('V'); cost_faction=faction; return cost_result;
}
static void parse_say(int slot,char *name,int a,int b) {
    assert(slot==1 && name==Facility[1].name && a==-1 && b==-1); event('P'); ++parse_calls;
}
static void popb(const char *label,int flags,int icon,const char *picture,void *sprite) {
    assert(strcmp(label,"POWERSHORT")==0 && flags==0x10000 && icon==14 &&
           strcmp(picture,"genwarning_sm.pcx")==0 && sprite==0);
    event('O'); ++popup_calls;
}
'''


MAIN = r'''
static void built(int base,int facility) { Bases[base].facilities_built[facility>>3] |= 1u<<(facility&7); }
static void project(int id) { GameDataStateStorage.fields_.secret_project_[id]=2; Bases[2].faction_id_current=1; }
static void reset() {
    memset(Bases,0,sizeof Bases); memset(PlayersData,0,sizeof PlayersData);
    memset(Players,0,sizeof Players); memset(Facility,0,sizeof Facility);
    memset(&GameDataStateStorage,0,sizeof GameDataStateStorage); memset(&MapStorage,0,sizeof MapStorage);
    memset(&WorldBuilder,0,sizeof WorldBuilder); memset(TectonicDetonationCount,0,sizeof TectonicDetonationCount);
    memset(BaseCurrentConvoyTo,0,sizeof BaseCurrentConvoyTo); memset(BaseCurrentConvoyFrom,0,sizeof BaseCurrentConvoyFrom);
    BaseCurrent=&Bases[0]; BaseIDCurrentSelected=0; BaseCurrent->faction_id_current=1; BaseCurrent->population_size=20;
    Bases[1].faction_id_current=2; PlayersData[1].tech_ranking=12; PlayersData[1].diff_level=3;
    GameDataStateStorage.fields_.diff_level_current_=3; GameDataStateStorage.fields_.lock_masks_.enable_mask_=2;
    for (int i=0;i<64;++i) GameDataStateStorage.fields_.secret_project_[i]=-1;
    WorldBuilder.sea_level_rises=1; BaseCurrentForcesMaintCost=0;
    facility_call_count=human_reads=switch_base_on_facility=drop_human_after_first=0;
    reactor_result=5; reactor_mutates_difficulty=reactor_calls=tech_calls=0;
    tech_result=tech_mutates_bonus=clear_calls=parse_calls=popup_calls=0;
    cost_faction=-1; cost_result=7; switch_on_clear=0; event_count=0; events[0]=0;
    Facility[1].name=(char *)"headquarters"; Facility[1].cost=5;
}
static void minerals() {
    reset(); BaseCurrent->mineral_intake_2=16; BaseCurrentForcesMaintCost=2; BaseCurrentConvoyFrom[1]=1;
    base_minerals(); assert(BaseCurrent->mineral_intake_2==16 && BaseCurrent->mineral_consumption==3);
    assert(BaseCurrent->mineral_surplus==13 && BaseCurrent->mineral_surplus_final==13 && BaseCurrent->eco_damage==0);
    const int order[]={21,19,22,20,20}; assert(facility_call_count==5 && memcmp(facility_calls,order,sizeof order)==0);
    assert(human_reads==3);
    // Ordinary, bounded rounding discriminator: native final 0, merged source 1.
    reset(); BaseCurrent->mineral_intake_2=16; PlayersData[1].major_atrocities=1;
    TectonicDetonationCount[1]=1; MapSeaLevel=2; base_minerals(); assert(BaseCurrent->eco_damage==0);
    reset(); BaseCurrent->mineral_intake_2=-1; base_minerals(); assert(BaseCurrent->mineral_intake_2==-1 && BaseCurrent->mineral_surplus==-1);
    reset(); BaseCurrent->mineral_intake_2=INT_MAX; base_minerals(); assert(BaseCurrent->mineral_intake_2==-1);
    reset(); BaseCurrent->mineral_intake_2=INT_MAX; BaseCurrentConvoyTo[1]=1;
    BaseCurrentForcesMaintCost=0xFFFFFFFFu; BaseCurrentConvoyFrom[1]=1;
    base_minerals(); assert(BaseCurrent->mineral_intake_2==0 && BaseCurrent->mineral_consumption==0);
    reset(); BaseCurrentForcesMaintCost=INT_MAX; BaseCurrentConvoyFrom[1]=1;
    base_minerals(); assert(BaseCurrent->mineral_consumption==INT_MIN && BaseCurrent->mineral_surplus==INT_MIN);
    reset(); BaseCurrent->mineral_intake_2=17; BaseCurrent->eco_damage=-1; PlayersData[1].tech_ranking=36;
    base_minerals(); assert(BaseCurrent->eco_damage==0);
    reset(); BaseCurrent->mineral_intake_2=17; BaseCurrent->eco_damage=INT_MIN;
    base_minerals(); assert(BaseCurrent->eco_damage==0);
    // The pre-scaling clamp is not a post-scaling clamp; negative stays negative.
    reset(); BaseCurrent->mineral_intake_2=17; PlayersData[1].tech_ranking=1;
    PlayersData[1].soc_effect_pending.values_[SocialEffect::PLANET]=2; MapNativeLifeForms=INT_MAX;
    base_minerals(); assert(BaseCurrent->eco_damage==-3579138);
    reset(); BaseCurrent->mineral_intake_2=17; PlayersData[1].tech_ranking=INT_MIN;
    PlayersData[1].theory_of_everything=INT_MAX;
    PlayersData[1].soc_effect_pending.values_[SocialEffect::PLANET]=2;
    base_minerals(); assert(BaseCurrent->eco_damage==0); // wrapped technology difference is one
    reset(); PlayersData[1].planet_ecology=INT_MAX; PlayersData[1].tech_ranking=0;
    base_minerals(); assert(BaseCurrent->eco_damage==0);
    reset(); BaseCurrent->mineral_intake_2=17; GameDataStateStorage.fields_.lock_masks_.enable_mask_=0;
    GameDataStateStorage.fields_.diff_level_current_=INT_MIN;
    PlayersData[1].tech_ranking=200; PlayersData[1].soc_effect_pending.values_[SocialEffect::PLANET]=2;
    base_minerals(); assert(BaseCurrent->eco_damage==0); // wrapped6-difficulty clamps to one
    reset(); BaseCurrent->mineral_intake_2=10; BaseCurrentForcesMaintCost=2;
    project(SP_SPACE_ELEVATOR); BaseCurrent->queue_production_id[0]=-FAC_SKY_HYDRO_LAB;
    base_minerals(); assert(BaseCurrent->mineral_intake_2==20 && BaseCurrent->mineral_surplus==18 && BaseCurrent->mineral_surplus_final==8);
    reset(); BaseCurrent->mineral_intake_2=10; BaseCurrent->eco_damage=128;
    built(0,21); built(0,19); built(0,22); built(0,20); built(0,31); built(0,32);
    project(SP_BULK_MATTER_TRANSMITTER); project(SP_PHOLUS_MUTAGEN); project(SP_VOICE_OF_PLANET);
    PlayersData[1].satellites_[1]=2; BaseCurrent->population_size=3;
    GameDataStateStorage.fields_.game_rules_=RULES_VICTORY_TRANSCENDENCE;
    GameDataStateStorage.fields_.game_state_=STATE_PERIHELION_ACTIVE;
    base_minerals(); assert(BaseCurrent->mineral_intake_2==35 && BaseCurrent->eco_damage==4);
    // Instrumented human boundary makes the two source reevaluations observable.
    reset(); BaseCurrent->mineral_intake_2=16; PlayersData[1].major_atrocities=1;
    TectonicDetonationCount[1]=100; MapSeaLevel=2; PlayersData[1].tech_ranking=100;
    drop_human_after_first=1; base_minerals(); assert(human_reads==3 && BaseCurrent->eco_damage==2);
    // Callbacks publish another Base; the faction remains the captured actor.
    reset(); BaseCurrent->mineral_intake_2=7; BaseCurrentConvoyTo[1]=1;
    Bases[1].mineral_intake_2=10; switch_base_on_facility=1;
    base_minerals(); assert(BaseCurrent==&Bases[1] && Bases[0].mineral_intake_2==8 && Bases[1].mineral_intake_2==10);
}
static void facility_maintenance() {
    reset(); PlayersData[1].diff_level=4; reactor_mutates_difficulty=1;
    assert(fac_maint(FAC_COMMAND_CENTER,1)==2 && PlayersData[1].diff_level==100 && reactor_calls==1);
    reset(); PlayersData[1].diff_level=INT_MAX; assert(fac_maint(FAC_COMMAND_CENTER,1)==0 && reactor_calls==1);
    reset(); PlayersData[1].diff_level=-4; assert(fac_maint(FAC_COMMAND_CENTER,1)==0);
    reset(); reactor_result=-1; assert(fac_maint(FAC_COMMAND_CENTER,1)==0);
    reset(); reactor_result=1; assert(fac_maint(FAC_COMMAND_CENTER,1)==1);
    reset(); Facility[1].maint=0xFFFFFFFDu; assert(fac_maint(1,1)==-3);
    Players[1].faction_bonus_count=1; Players[1].faction_bonus_id[0]=RULE_FACILITY;
    Players[1].faction_bonus_val1[0]=1; assert(fac_maint(1,1)==0 && tech_calls==0);
    reset(); Facility[1].maint=3; Players[1].faction_bonus_count=1;
    Players[1].faction_bonus_id[0]=RULE_FREEFAC; Players[1].faction_bonus_val1[0]=1; Facility[1].preq_tech=100;
    assert(fac_maint(1,1)==3 && tech_calls==1);
    tech_result=1; tech_mutates_bonus=1; assert(fac_maint(1,1)==3); // live bonus value after has_tech
    reset(); Players[1].faction_bonus_count=1; Players[1].faction_bonus_id[0]=RULE_FREEFAC;
    Players[1].faction_bonus_val1[0]=1; Facility[1].preq_tech=100;
    Players[1].faction_bonus_id[1]=RULE_FACILITY; Players[1].faction_bonus_val1[1]=2;
    Facility[2].maint=9; tech_result=1; tech_mutates_bonus=2;
    assert(fac_maint(2,1)==0 && tech_calls==1); // live count after callback
}
static void accounting() {
    reset(); built(0,1); built(0,2); Facility[1].maint=3; Facility[2].maint=2;
    PlayersData[1].energy_reserves=100; PlayersData[1].flags=0x100; project(SP_SELF_AWARE_COLONY);
    base_maint(); assert(PlayersData[1].energy_reserves==98 && PlayersData[1].maint_cost_total==2);
    assert(PlayersData[1].flags==0x120 && clear_calls==0);
    reset(); built(0,1); Facility[1].maint=0xFFFFFFFDu; PlayersData[1].energy_reserves=100; project(SP_SELF_AWARE_COLONY);
    base_maint(); assert(PlayersData[1].energy_reserves==101 && PlayersData[1].maint_cost_total==0xFFFFFFFFu);
    assert((PlayersData[1].flags&0x20)!=0);
    reset(); built(0,1); Facility[1].maint=INT_MAX; PlayersData[1].energy_reserves=100;
    PlayersData[1].flags=0x20; project(SP_SELF_AWARE_COLONY); base_maint();
    assert(PlayersData[1].energy_reserves==1073741924 && PlayersData[1].maint_cost_total==0xC0000000u && (PlayersData[1].flags&0x20)==0);
    reset(); built(0,1); Facility[1].maint=1; PlayersData[1].energy_reserves=INT_MIN;
    base_maint(); assert(PlayersData[1].energy_reserves==INT_MAX && clear_calls==0);
    reset(); built(0,1); Facility[1].maint=1; BaseCurrent->queue_production_id[0]=-1;
    base_maint(); assert(PlayersData[1].energy_reserves==0 && clear_calls==0 && (Bases[0].facilities_built[0]&2));
    reset(); built(0,1); Facility[1].maint=1; PlayersData[1].diff_level=1;
    base_maint(); assert(PlayersData[1].energy_reserves==0 && clear_calls==0);
    reset(); built(0,1); Facility[1].maint=1; switch_on_clear=1;
    base_maint(); assert(strcmp(events,"CVPO")==0 && cost_faction==2);
    assert(clear_calls==1 && parse_calls==1 && popup_calls==1 && (Bases[0].facilities_built[0]&2)==0);
    assert(BaseCurrent==&Bases[1] && PlayersData[1].energy_reserves==35 && PlayersData[1].maint_cost_total==1);
    assert(PlayersData[2].energy_reserves==0 && PlayersData[2].maint_cost_total==0);
    // Repeatables never acquire installed-facility maintenance.
    reset(); built(0,65); Facility[65].maint=100; base_maint(); assert(PlayersData[1].maint_cost_total==0);
}
int main() { minerals(); facility_maintenance(); accounting(); }
'''


def program():
    source = (ROOT / "src/base.cpp").read_text()
    header = (ROOT / "src/base.h").read_text()
    general = (ROOT / "src/general.h").read_text()
    queries = "\n".join(definition(header, sig) for sig in (
        "MEASURED inline int __cdecl base_project(", "MEASURED inline BOOL __cdecl has_project(",
        "inline bool __cdecl has_fac_built(int facility_id, int base_id)",
        "inline bool __cdecl has_fac_built(int facility_id)",
        "static __forceinline int has_fac_built_call(", "MEASURED inline BOOL __cdecl ascending("))
    queries = queries.replace("__forceinline", "inline")
    bodies = "\n".join(definition(source, sig) for _, sig in SUBJECTS.values())
    return declarations() + helpers() + "\n" + PROVIDERS + queries + "\n" + definition(general, "MEASURED inline int __cdecl range(") + "\n" + bodies + "\n" + MAIN


def run_actual(tmp_path, text, succeeds=True):
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    assert any(compilers), "whole actual-source host compiler required"
    path = tmp_path / "economy.cpp"
    path.write_text(text)
    for compiler in filter(None, compilers):
        executable = tmp_path / Path(compiler).name.replace("+", "p")
        built = subprocess.run([compiler, "-std=c++11", "-O2", "-fsanitize=undefined", "-fno-sanitize-recover=undefined", str(path), "-o", str(executable)], capture_output=True, text=True, timeout=60)
        assert built.returncode == 0, built.stdout + built.stderr
        result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
        assert (result.returncode == 0) == succeeds, result.stderr


def test_complete_current_bodies_signed_wrapped_accounting_and_live_owners(tmp_path):
    run_actual(tmp_path, program())


MUTANTS = {
    "unsigned_minerals": ("base_yield_mul(\n        BaseCurrent->mineral_intake_2, mineral_bonus + 2) / 2", "static_cast<uint32_t>(base_yield_mul(\n        BaseCurrent->mineral_intake_2, mineral_bonus + 2)) / 2"),
    "truncating_eco": ("raw_eco_damage >= 0 ? raw_eco_damage / 8\n        : -1 - (-1 - raw_eco_damage) / 8", "raw_eco_damage / 8"),
    "merged_atrocities": ("base_yield_mul(PlayersData[faction_id].major_atrocities, 5)", "base_yield_mul(base_yield_add(PlayersData[faction_id].major_atrocities, base_yield_signed_bits(TectonicDetonationCount[faction_id])), 5)"),
    "unsigned_scale": ("BaseCurrent->eco_damage = scaled_damage / 6;", "BaseCurrent->eco_damage = static_cast<uint32_t>(scaled_damage) / 6;"),
    "unwrapped_life": ("base_yield_add(MapNativeLifeForms, 1)", "(MapNativeLifeForms + 1)"),
    "unwrapped_product": ("base_yield_mul(\n        BaseCurrent->mineral_intake_2, mineral_bonus + 2) / 2", "(BaseCurrent->mineral_intake_2 * (mineral_bonus + 2)) / 2"),
    "unwrapped_technology": ("base_yield_sub(PlayersData[faction_id].tech_ranking,\n            PlayersData[faction_id].theory_of_everything)", "(PlayersData[faction_id].tech_ranking - PlayersData[faction_id].theory_of_everything)"),
    "unwrapped_planet_ecology": ("base_yield_add(PlayersData[faction_id].planet_ecology, 16)", "(PlayersData[faction_id].planet_ecology + 16)"),
    "unwrapped_nonhuman_difficulty": ("base_yield_sub(6, GameDataStateStorage.fields_.diff_level_current_)", "(6 - GameDataStateStorage.fields_.diff_level_current_)"),
    "post_scale_clamp": ("BaseCurrent->eco_damage = scaled_damage / 6;", "BaseCurrent->eco_damage = scaled_damage < 0 ? 0 : scaled_damage / 6;"),
    "unsigned_halving": ("maint /= 2;", "maint = static_cast<uint32_t>(maint) / 2;"),
    "removed_carry": ("maint = base_yield_add(maint, 1);", "maint = maint;"),
    "cached_sale_faction": ("cost_factor(BaseCurrent->faction_id_current,", "cost_factor(faction_id,"),
    "unwrapped_difficulty": ("base_yield_add(PlayersData[faction_id].diff_level, 1) / 2", "(PlayersData[faction_id].diff_level + 1) / 2"),
    "cached_bonus_count": ("i < Players[faction_id].faction_bonus_count", "i < 1"),
    "updated_final_surplus": ("// doesn't update mineral_surplus_final?", "BaseCurrent->mineral_surplus_final = BaseCurrent->mineral_intake_2 - BaseCurrent->mineral_consumption; // incorrect synchronization"),
}


@pytest.mark.parametrize("damage", MUTANTS)
def test_executed_controls_reject_accounting_and_owner_damage(tmp_path, damage):
    text = program()
    old, new = MUTANTS[damage]
    assert text.count(old) == 1, (damage, text.count(old))
    changed = text.replace(old, new, 1)
    if damage == "merged_atrocities":
        old_second = "base_yield_mul(base_yield_signed_bits(TectonicDetonationCount[faction_id]), 5)"
        assert changed.count(old_second) == 1
        changed = changed.replace(old_second, "0", 1)
    run_actual(tmp_path, changed, False)


def test_pinned_complete_native_branches_and_signed_arithmetic_anchors():
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    capstone = pytest.importorskip("capstone")
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.exists() and "OPENSMACX_IMAGE" not in os.environ:
        pytest.skip("pinned image unavailable; set OPENSMACX_IMAGE")
    image = PeImage(path.read_bytes(), PIN)
    decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    decoder.detail = True
    all_rows = {}
    for address, (end, _) in SUBJECTS.items():
        rows = list(decoder.disasm(image.read(Span(address, end)), address))
        assert sum(x.size for x in rows) == end - address
        starts = {x.address for x in rows}
        branches = [x for x in rows if x.group(capstone.CS_GRP_JUMP)]
        assert len(branches) == {0x4E9CB0: 55, 0x4F6510: 11, 0x4F65F0: 12}[address]
        assert all(x.operands[0].type == capstone.CS_OP_IMM and x.operands[0].imm in starts for x in branches)
        all_rows.update({x.address: (x.mnemonic, x.op_str) for x in rows})
    for address, expected in {
        0x4E9D94: ("imul", "eax, dword ptr [ecx + 0xc4]"),
        0x4E9D9E: ("sar", "eax, 1"), 0x4E9E19: ("sar", "ebx, 3"),
        0x4E9FF1: ("idiv", "esi"), 0x4EA056: ("idiv", "esi"),
        0x4EA140: ("inc", "edx"), 0x4EA150: ("imul", "ecx"),
        0x4EA154: ("shr", "eax, 0x1f"), 0x4F66C8: ("cdq", ""),
        0x4F66CB: ("sar", "eax, 1"), 0x4F6772: ("mov", "eax, dword ptr [0x90ea30]"),
        0x4F6779: ("mov", "dl, byte ptr [eax + 4]"), 0x4F6539: ("inc", "eax"),
    }.items():
        assert all_rows[address] == expected
