"""Actual-source controls for Datalink's per-category text inputs."""

from __future__ import annotations

import re
import shutil
import subprocess
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[2]
NAMES = (
    "DatalinkCategoryIdSeparator", "DatalinkChassisIdSeparator",
    "DatalinkReactorIdSeparator", "DatalinkWeaponIdSeparator",
    "DatalinkDefenseIdSeparator", "DatalinkAbilityIdSeparator",
    "DatalinkFacilityIdSeparator", "DatalinkProjectIdSeparator",
    "DatalinkTechIdSeparator", "DatalinkConceptIdSeparator",
    "DatalinkAdvConceptIdSeparator", "DatalinkSocialIdSeparator",
    "DatalinkEffectIdSeparator", "DatalinkFactionIdSeparator",
    "DatalinkTerraformIdSeparator", "DatalinkTerraformSecondSeparator",
    "DatalinkConceptArticle", "DatalinkConceptFile", "DatalinkConceptSection",
    "DatalinkAdvConceptFile",
)

CATEGORY_CONSUMERS = {
    "set_cat_unit": "DatalinkCategoryIdSeparator",
    "set_cat_chassis": "DatalinkChassisIdSeparator",
    "set_cat_reactor": "DatalinkReactorIdSeparator",
    "set_cat_weapon": "DatalinkWeaponIdSeparator",
    "set_cat_defense": "DatalinkDefenseIdSeparator",
    "set_cat_ability": "DatalinkAbilityIdSeparator",
    "set_cat_facility": "DatalinkFacilityIdSeparator",
    "set_cat_project": "DatalinkProjectIdSeparator",
    "set_cat_tech": "DatalinkTechIdSeparator",
    "set_cat_concept": "DatalinkConceptIdSeparator",
    "set_cat_adv_concept": "DatalinkAdvConceptIdSeparator",
    "set_cat_social": "DatalinkSocialIdSeparator",
    "set_cat_effect": "DatalinkEffectIdSeparator",
    "set_cat_faction": "DatalinkFactionIdSeparator",
    "set_cat_terraform": "DatalinkTerraformIdSeparator",
}


def _definition(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError(f"unterminated definition: {signature}")


def test_datalink_text_arrays_and_actual_consumers(tmp_path: Path) -> None:
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    source = (ROOT / "src/datalink.cpp").read_text()
    faction_header = (ROOT / "src/faction.h").read_text()
    veh_header = (ROOT / "src/veh.h").read_text()
    player_declaration = re.search(r"struct Player\s*\{.*?\n\};", faction_header, re.DOTALL)
    order_declaration = re.search(r"struct RulesOrder\s*\{.*?\n\};", veh_header, re.DOTALL)
    assert player_declaration and order_declaration
    declarations = []
    for name in NAMES:
        match = re.search(rf"char {name}\[\] = \"[^\"]*\";", source)
        assert match, f"missing writable source-owned array {name}"
        declarations.append(match.group(0))
    expected_values = ("|",) * 16 + ("The ", "TITLES", "CONCEPTSX", "ADVTITLES")
    for name, value in zip(NAMES, expected_values):
        assert f"char {name}[] = \"{value}\";" in source
    assert "folds to the bare immediate" not in source

    bodies = "\n".join(_definition(source, f"void Datalink::{name}()")
                        for name in CATEGORY_CONSUMERS)
    harness = r'''
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
typedef unsigned int uint32_t;
typedef int BOOL;
typedef char *LPSTR;
struct RECT { int left,top,right,bottom; };
static char StringTemp[1024];
int Language=0;
struct GameFields { uint32_t game_state_; };
struct { GameFields fields_; } GameDataStateStorage={{0x1000u}};
struct Entry { std::string text; int id; };
struct ListBox { std::vector<Entry> entries;
  void item(const char *s,int n) { Entry e={s,n}; entries.push_back(e); }
} list;
struct Datalink {
  ListBox &listBox_; RECT category_rect_; int font3_;
  Datalink() : listBox_(list), category_rect_({0,0,100,80}), font3_(0) {}
  void set_cat_unit(); void set_cat_chassis(); void set_cat_reactor();
  void set_cat_weapon(); void set_cat_defense(); void set_cat_ability();
  void set_cat_facility(); void set_cat_project(); void set_cat_tech();
  void set_cat_concept(); void set_cat_adv_concept(); void set_cat_social();
  void set_cat_effect(); void set_cat_faction(); void set_cat_terraform();
};
struct VehPrototype { unsigned flags; const char *veh_name; };
VehPrototype VehPrototypes[100];
struct RulesTechnology {
  uint32_t flags; char *name; char id[8]; int padding;
  int growth_value, tech_value, wealth_value, power_value;
  int preq_tech_1, preq_tech_2;
};
const int MaxTechnologyNum=89; RulesTechnology Technology[MaxTechnologyNum];
struct SocialEffect { int values_[11]; };
struct RulesSocialCategory {
  char *type; int preq_tech[4]; char *name[4]; SocialEffect model_effect[4];
};
const int MaxSocialCatNum=4; RulesSocialCategory SocialCategories[MaxSocialCatNum];
struct RulesSocialEffect { char set1[24], set2[24], padding[56]; };
const int MaxSocialEffectNum=11; RulesSocialEffect SocialEffects[MaxSocialEffectNum];
struct RulesTerraforming {
  char *name, *name_sea; int preq_tech, preq_tech_sea;
  uint32_t bit, bit_incompatible; int rate; char *shortcuts;
};
const int MaxTerrainNum=20; RulesTerraforming Terraforming[MaxTerrainNum];
''' + player_declaration.group(0) + r'''
static_assert(sizeof(Player)==0x59C, "actual Player declaration layout");
const int MaxPlayerNum=8;
// Oversized host-only backing permits the original inclusive Player[8] read;
// it does not establish that production has a ninth Players row.
Player Players[MaxPlayerNum];
''' + order_declaration.group(0) + r'''
RulesOrder Order[30];
struct RulesEntry { int preq_tech; int name; };
struct RulesChassis { int preq_tech; int offsv_1_name; };
RulesChassis Chassis[64]; int MaxChassisNum=1;
struct RulesReactor { int preq_tech; int name; };
RulesReactor Reactor[5];
RulesEntry Weapon[64]; int MaxWeaponNum=1;
RulesEntry Armor[64]; int MaxArmorNum=1;
RulesEntry Ability[64]; int MaxAbilityNum=1;
struct RulesFacility { int preq_tech; int name; };
RulesFacility Facility[64]; int FacilitySPStart=0;
int GenderDefault=0, PluralityDefault=0;
static int name_storage[61];
const int *TerraformNameBase=name_storage+1;
struct Strings { char *get(int id) {
  static char value[80];
  if(id==5) std::strcpy(value,"The Lighthouse");
  else if(id==10) std::strcpy(value,"Land action");
  else if(id==11) std::strcpy(value,"Sea action");
  else std::sprintf(value,"key%d",id);
  return value;
} char *get(char *text) {
  return text;
} } StringTable;
static const char *text_lines[4]; static int text_line=0;
char *TextBufferGetPtr=0; int open_result=0,open_calls=0,get_calls=0,close_calls=0;
char opened_section[32],opened_file[32];
int text_open(char *section,char *file) {
  ++open_calls; std::strcpy(opened_section,section); std::strcpy(opened_file,file);
  text_line=0; return open_result;
}
void text_get() { ++get_calls; TextBufferGetPtr=const_cast<char *>(text_lines[text_line++]); }
void text_close() { ++close_calls; }
int InterfaceBoxStyles[8];
void box_sprite(RECT *,int *) {}
void set_text_color(int,int,int,int) {}
void set_font(int *,int,int,int) {}
const char *category_label(int) { return "category"; }
#define CATEGORY_LABEL(n) category_label(n)
void write_cent_l(char *,RECT *,unsigned int) {}
void soft_update(RECT *) {}
char *decimal(int n,char *out,int) { std::sprintf(out,"%d",n); return out; }
#define _itoa decimal
'''+"\n".join(declarations)+"\n"+bodies+r'''
static void reset() {
  list.entries.clear(); StringTemp[0]=0; open_result=0;
  open_calls=get_calls=close_calls=text_line=0;
  opened_section[0]=opened_file[0]=0;
  text_lines[0]=const_cast<char *>("First concept");
  text_lines[1]=const_cast<char *>("Second concept");
  text_lines[2]=const_cast<char *>(""); text_lines[3]=0;
  std::memset(Facility,0,sizeof(Facility));
  for(int i=0;i<64;++i) Facility[i].preq_tech=-2;
  std::memset(name_storage,0,sizeof(name_storage));
  std::memset(Technology,0,sizeof(Technology));
  std::memset(SocialCategories,0,sizeof(SocialCategories));
  std::memset(SocialEffects,0,sizeof(SocialEffects));
  std::memset(Terraforming,0,sizeof(Terraforming));
  for(int i=0;i<MaxTerrainNum;++i) Terraforming[i].preq_tech_sea=-2;
  std::memset(Order,0,sizeof(Order));
  std::memset(Players,0,sizeof(Players));
  GameDataStateStorage.fields_.game_state_=0x1000u; Language=0;
}
int main() {
  assert(sizeof(DatalinkCategoryIdSeparator)==2 && std::strcmp(DatalinkCategoryIdSeparator,"|")==0);
  assert(sizeof(DatalinkTerraformSecondSeparator)==2 && std::strcmp(DatalinkTerraformSecondSeparator,"|")==0);
  assert(sizeof(DatalinkConceptArticle)==5 && std::strcmp(DatalinkConceptArticle,"The ")==0);
  assert(sizeof(DatalinkConceptFile)==7 && std::strcmp(DatalinkConceptFile,"TITLES")==0);
  assert(sizeof(DatalinkConceptSection)==10 && std::strcmp(DatalinkConceptSection,"CONCEPTSX")==0);
  assert(sizeof(DatalinkAdvConceptFile)==10 && std::strcmp(DatalinkAdvConceptFile,"ADVTITLES")==0);
  assert(&DatalinkCategoryIdSeparator[0]!=&DatalinkTerraformSecondSeparator[0]);
  assert(&DatalinkConceptSection[0]!=&DatalinkConceptFile[0] && &DatalinkConceptFile[0]!=&DatalinkAdvConceptFile[0]);
  char *owned_arrays[] = {''' + ",".join(f"{name}" for name in NAMES) + r''' };
  for(unsigned i=0;i<16;++i) assert(std::strcmp(owned_arrays[i],"|")==0);
  for(unsigned i=0;i<sizeof(owned_arrays)/sizeof(owned_arrays[0]);++i)
    for(unsigned j=i+1;j<sizeof(owned_arrays)/sizeof(owned_arrays[0]);++j)
      assert(&owned_arrays[i][0]!=&owned_arrays[j][0]);
  DatalinkCategoryIdSeparator[0]=':'; assert(DatalinkTerraformSecondSeparator[0]=='|');
  DatalinkCategoryIdSeparator[0]='|';

  Datalink win;
  reset(); text_lines[0]=const_cast<char *>("First concept");
  text_lines[1]=const_cast<char *>("Second concept");
  win.set_cat_concept();
  assert(open_calls==1 && std::strcmp(opened_section,"CONCEPTSX")==0 &&
         std::strcmp(opened_file,"TITLES")==0);
  assert(get_calls==3 && close_calls==1 && list.entries.size()==2);
  assert(list.entries[0].text=="First concept|0" && list.entries[0].id==0);
  assert(list.entries[1].text=="Second concept|1" && list.entries[1].id==1);

  reset(); win.set_cat_adv_concept();
  assert(open_calls==1 && std::strcmp(opened_section,"CONCEPTSX")==0 &&
         std::strcmp(opened_file,"ADVTITLES")==0);
  assert(list.entries.size()==2 && list.entries[0].text=="First concept|10000" &&
         list.entries[0].id==10000);
  assert(list.entries[1].text=="Second concept|10001" && list.entries[1].id==10001);

  reset(); open_result=1; win.set_cat_concept();
  assert(get_calls==0 && close_calls==0 && list.entries.empty());
  reset(); open_result=1; win.set_cat_adv_concept();
  assert(get_calls==0 && close_calls==0 && open_calls==1);

  reset(); Facility[0].preq_tech=-1; Facility[0].name=5;
  win.set_cat_project();
  assert(list.entries.size()==1 && list.entries[0].text=="Lighthouse|68534" &&
         list.entries[0].id==68534);
  reset();
  Order[4].order=const_cast<char *>("Land action");
  Order[4].order_sea=const_cast<char *>("Sea action");
  Terraforming[0].preq_tech=-1;
  Terraforming[0].preq_tech_sea=-1;
  DatalinkCategoryIdSeparator[0]=';';
  DatalinkTerraformIdSeparator[0]=':';
  win.set_cat_terraform();
  assert(list.entries.size()>=2);
  assert(list.entries[0].text=="Land action:90000" && list.entries[0].id==90000);
  assert(list.entries[1].text=="Sea action|90020" && list.entries[1].id==90020);
  DatalinkCategoryIdSeparator[0]='|';
  DatalinkTerraformIdSeparator[0]='|';

  reset();
  for(int faction=1;faction<=7;++faction) {
    Players[faction].noun_gender=100+faction;
    Players[faction].is_noun_plural=200+faction;
    std::sprintf(Players[faction].formal_name_faction,"Faction %d",faction);
  }
  win.set_cat_faction();
  assert(list.entries.size()==7);
  for(int i=0;i<7;++i) {
    assert(list.entries[i].id==150001+i);
    char expected[80];
    std::sprintf(expected,"Faction %d|%d",i+1,150001+i);
    assert(list.entries[i].text==expected);
  }
}
'''
    cpp = tmp_path / "datalink_text.cpp"
    exe = tmp_path / "datalink_text"
    cpp.write_text(harness)
    built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                            str(cpp), "-o", str(exe)], capture_output=True, text=True)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(exe)], capture_output=True, text=True)
    assert ran.returncode == 0, ran.stdout + ran.stderr

    mutants = (
        ("concept-file-swapped", "text_open(DatalinkConceptSection, DatalinkConceptFile)",
         "text_open(DatalinkConceptSection, DatalinkAdvConceptFile)"),
        ("article-prefix-swapped", "strncmp(StringTemp, DatalinkConceptArticle, 4)",
         "strncmp(StringTemp, DatalinkConceptFile, 4)"),
        ("terraform-separator-aliased", "strcat(StringTemp, DatalinkTerraformSecondSeparator)",
         "strcat(StringTemp, DatalinkCategoryIdSeparator)"),
    )
    for name, old, new in mutants:
        assert old in bodies, name
        mutant_bodies = bodies.replace(old, new, 1)
        mutant_harness = harness.replace(bodies, mutant_bodies, 1)
        mutant_cpp, mutant_exe = tmp_path / (name + ".cpp"), tmp_path / name
        mutant_cpp.write_text(mutant_harness)
        mutant_build = subprocess.run(
            [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
             str(mutant_cpp), "-o", str(mutant_exe)], capture_output=True, text=True)
        assert mutant_build.returncode == 0, mutant_build.stdout + mutant_build.stderr
        mutant_run = subprocess.run([str(mutant_exe)], capture_output=True, text=True)
        assert mutant_run.returncode != 0, name


def test_consumers_keep_each_proved_source_identity() -> None:
    source = (ROOT / "src/datalink.cpp").read_text()
    assert "strncmp(StringTemp, DatalinkConceptArticle, 4)" in source
    assert "text_open(DatalinkConceptSection, DatalinkConceptFile)" in source
    assert "text_open(DatalinkConceptSection, DatalinkAdvConceptFile)" in source
    assert "strcat(StringTemp, DatalinkTerraformSecondSeparator)" in source
    for function, variable in CATEGORY_CONSUMERS.items():
        body = _definition(source, f"void Datalink::{function}()")
        assert body.count(f"strcat(StringTemp, {variable})") == 1, function
    for address in ("0x0042A630", "0x0042A790", "0x0042A900", "0x0042AA60",
                    "0x0042ABC0", "0x0042AD20", "0x0042AE80", "0x0042AFE0",
                    "0x0042B190", "0x0042B300", "0x0042B490", "0x0042B630",
                    "0x0042B7E0", "0x0042B940", "0x0042BAB0"):
        assert address in source
