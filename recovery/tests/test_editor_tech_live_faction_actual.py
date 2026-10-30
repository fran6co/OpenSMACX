"""Source-extracted controls for editor_tech's pinned live faction flow."""

import shutil
import subprocess

import pytest

from console_sources import console_method_source


def _body(source: str, signature: str) -> str:
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
    raise AssertionError("unterminated editor_tech")


HARNESS = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

int ScenarioTechFaction=-1, ScenarioTechSelection=-1, PopupModalState=0;
int GenderDefault=0, PluralityDefault=0;
char PopupStartCaption[16]="caption", StringTemp[64], StrEditorWhoListTitle[16];
const int MaxTechnologyNum=2, MaxPlayerNum=8, TechNone=-1;
std::vector<int> item_has_args, item_avail_args, selected_has_args;
std::vector<int> ranking_has_args, ranking_write_factions, opening_factions;
std::vector<int> who_preselect, outer_results, who_destruction_globals, grant_args;
std::vector<int> events;
int outer_index=0, who_result=0, phase=0, ranking_calls=0;
bool listing_mutates=false, popup_result_mutates=false, selected_owned=true;
bool who_constructor_mutates=false, who_destructor_mutates=false;
bool construct_who_next=false;

struct GraphicWin { void redraw() { events.push_back(70); ScenarioTechFaction=6; phase=13; } };
GraphicWin TutWinBaseWindow;
struct Dialogs { void item(char*,int) {} };
struct StringList { void sort(int) {} };
struct Popup {
  Dialogs list; StringList item_ids_; bool who=false; bool input=false;
  Popup() { events.push_back(1); if(construct_who_next) { construct_who_next=false; if(who_constructor_mutates) ScenarioTechFaction=3; } }
  ~Popup() { if(who) { events.push_back(20); if(who_destructor_mutates) { ScenarioTechFaction=4; who_destruction_globals.push_back(ScenarioTechFaction); } } }
  int start(char*,const char *name,int,char*,int) {
    who=std::strcmp(name,"EDITORWHO")==0;
    return 0;
  }
  void set_cancel_text(char*) {} void set_ok_text(char*) {}
  Dialogs& dialogs(){return list;} void sort_item_ids(int) {}
  void set_value(int v){ if(who) who_preselect.push_back(v); }
  unsigned input_poll_state() const { if(input) construct_who_next=true; return input ? 1u : 0u; }
  int exec(int,void*) {
    if(who) return who_result;
    const int result=outer_results.at(outer_index++);
    input=(outer_index==1 && result>=0 && phase==10);
    if(result>=0 && phase==0 && popup_result_mutates) ScenarioTechFaction=2;
    if(result>=0 && phase==10) ScenarioTechFaction=2;
    if(result>=0) phase=11;
    return result;
  }
};
struct Console:GraphicWin { int faction_id_=1; void editor_tech(); } ConsoleGlobal;
struct Player { int noun_gender=0,is_noun_plural=0; char noun_faction[16]="Faction"; } Players[8];
struct PlayerData { int tech_ranking=0; } PlayersData[8];
struct RulesTechnology { int preq_tech_1=-2,preq_tech_2=-2; } Technology[2];
struct GameDataStateFields { uint8_t game_tech_achieved_[2]={0,0}; };
struct GameDataStateFixture { GameDataStateFields fields_; } GameDataStateStorage;
char* label_get(int id) { assert((id>=246 && id<=248) || id==270 || id==1008); return const_cast<char*>("label"); }
void auto_undo() {}
void parse_says(int,char *name,int,int) {
  for(int i=0;i<MaxPlayerNum;++i) if(name==Players[i].noun_faction) opening_factions.push_back(i);
}
void say_tech(char *out,int,int) { std::strcpy(out,"T"); }
bool has_tech(int tech,int faction) {
  (void)tech;
  if(phase==0 && listing_mutates) {
    item_has_args.push_back(faction); ScenarioTechFaction=2; return false;
  }
  if(phase==0) { item_has_args.push_back(faction); return false; }
  if(phase==11) { selected_has_args.push_back(faction); return selected_owned; }
  if(phase==12) {
    ranking_has_args.push_back(faction);
    ++ranking_calls;
    ScenarioTechFaction=faction+1;
    return ranking_calls==1;
  }
  item_has_args.push_back(faction); return false;
}
int tech_avail(int,int faction) { item_avail_args.push_back(faction); return 0; }
void bitmask(int,int*,int*) { ScenarioTechFaction=3; phase=12; }
void tech_achieved(int faction,int,int,int) { grant_args.push_back(faction); ScenarioTechFaction=3; phase=12; }
void see_map_check() {}
'''


def test_editor_tech_uses_live_faction_reads_and_popup_lifetime(tmp_path):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    source = console_method_source("void Console::editor_tech()")
    method = _body(source, "void Console::editor_tech()")
    main = r'''
int main() {
  Technology[0].preq_tech_1=-1; Technology[0].preq_tech_2=-1;
  Technology[1].preq_tech_1=-2; Technology[1].preq_tech_2=-2;

  // A negative initial global falls back to the Console faction and persists it.
  ScenarioTechFaction=-1; ConsoleGlobal.faction_id_=2; phase=0; outer_index=0;
  outer_results={-1}; listing_mutates=false; popup_result_mutates=false;
  item_has_args.clear(); opening_factions.clear();
  ConsoleGlobal.editor_tech();
  assert(ScenarioTechFaction==2);
  assert((opening_factions==std::vector<int>{2}));

  // The tech-status and availability calls sample the global separately.
  ScenarioTechFaction=1; listing_mutates=true; phase=0; outer_index=0;
  outer_results={-1}; item_has_args.clear(); item_avail_args.clear();
  ConsoleGlobal.editor_tech();
  assert((item_has_args==std::vector<int>{1}));
  assert((item_avail_args==std::vector<int>{2}));

  // A callback after bitmask changes the faction used for both the clear and
  // initial ranking row. Ranking callbacks then change the row target after
  // has_tech, and redraw changes the next opening faction.
  ScenarioTechFaction=1; listing_mutates=false; popup_result_mutates=true; selected_owned=true; phase=0; outer_index=0;
  outer_results={0,-1}; GameDataStateStorage.fields_.game_tech_achieved_[0]=0xff; ranking_calls=0;
  item_has_args.clear(); selected_has_args.clear(); ranking_has_args.clear();
  opening_factions.clear(); PlayersData[3].tech_ranking=0;
  PlayersData[4].tech_ranking=0; PlayersData[5].tech_ranking=0;
  ConsoleGlobal.editor_tech();
  assert((selected_has_args==std::vector<int>{2}));
  assert((ranking_has_args==std::vector<int>{3,4}));
  assert(GameDataStateStorage.fields_.game_tech_achieved_[0]==0xf7); // bitmask callback changed the clear faction to 3
  assert(PlayersData[4].tech_ranking==2); // first lookup's post-call faction
  assert(PlayersData[5].tech_ranking==0);
  assert((opening_factions==std::vector<int>{1,6})); // redraw mutation is reread

  // The technology-grant callback can replace the faction; ranking starts
  // from the post-callback value.
  ScenarioTechFaction=1; listing_mutates=false; popup_result_mutates=true;
  selected_owned=false; phase=0; outer_index=0; outer_results={0,-1};
  ranking_calls=0; selected_has_args.clear(); grant_args.clear(); ranking_has_args.clear(); opening_factions.clear();
  PlayersData[4].tech_ranking=0;
  ConsoleGlobal.editor_tech();
  assert((selected_has_args==std::vector<int>{2}));
  assert((grant_args==std::vector<int>{2}));
  assert((ranking_has_args==std::vector<int>{3,4}));
  assert(PlayersData[4].tech_ranking==2);

  // Who preselection is captured before Popup construction. Positive choice
  // is stored only after destruction, then enters the next body as chosen.
  ScenarioTechFaction=1; phase=10; outer_index=0; outer_results={0,-1};
  who_result=5; who_preselect.clear(); opening_factions.clear();
  who_constructor_mutates=true; who_destructor_mutates=true;
  who_destruction_globals.clear();
  ConsoleGlobal.editor_tech();
  assert((who_preselect==std::vector<int>{2}));
  assert((who_destruction_globals==std::vector<int>{4}));
  assert((opening_factions==std::vector<int>{1,5}));
  assert(ScenarioTechFaction==5);

  // A nonpositive result keeps the destructor's mutation and reloads after
  // Popup destruction rather than carrying the pre-construction snapshot.
  ScenarioTechFaction=1; phase=10; outer_index=0; outer_results={0,-1};
  who_result=0; who_preselect.clear(); opening_factions.clear();
  who_destruction_globals.clear();
  ConsoleGlobal.editor_tech();
  assert((who_preselect==std::vector<int>{2}));
  assert((who_destruction_globals==std::vector<int>{4}));
  assert((opening_factions==std::vector<int>{1,4}));
  assert(ScenarioTechFaction==4);
}
'''
    unit = tmp_path / "editor_tech_live_faction.cpp"
    unit.write_text(HARNESS + method + main)
    executable = tmp_path / "editor_tech_live_faction"
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    str(unit), "-o", str(executable)], check=True,
                   capture_output=True, text=True)
    subprocess.run([str(executable)], check=True, capture_output=True, text=True)
