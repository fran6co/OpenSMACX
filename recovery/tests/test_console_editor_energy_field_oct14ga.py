"""Pinned-field and actual-source controls for Console::editor_energy."""
from __future__ import annotations

import shutil
import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src" / "console.cpp"


def body(text: str) -> str:
    signature = "void Console::editor_energy()"
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for offset in range(opening, len(text)):
        if text[offset] == "{":
            depth += 1
        elif text[offset] == "}":
            depth -= 1
            if depth == 0:
                return text[start : offset + 1]
    raise AssertionError("unterminated Console::editor_energy body")


PRELUDE = r"""
#include <cassert>
#include <vector>
static std::vector<int> events;
static std::vector<int> parsed_ids;
static std::vector<int> parsed_values;
static std::vector<int> say_ids;
static std::vector<int> gender_seen;
static std::vector<int> plurality_seen;
static int chosen_faction = 3;
static int popup_answer = 1;
static int redraws = 0;
static int updates = 0;
static int ParseNumTable[4] = {777, 0, 0, 0};
static const int MaxPlayerNum = 8;
static int GenderDefault = -7;
static int PluralityDefault = -8;
static const char ConsoleStrEditorEnergy[] = "EDITORENERGY";
typedef int (*NullCallback)();

struct Player {
  int is_leader_female;
  char adj_name_faction[8];
  char name_leader[8];
};
struct PlayerData {
  int energy_reserves;
  int unk_48;
};
static Player Players[8];
static PlayerData PlayersData[8];
struct Console {
  int faction_id_;
  void update_data(int value) { assert(value == 0); ++updates; events.push_back(90); }
  void editor_energy();
} ConsoleGlobal;
struct BaseWin { void on_redraw() { ++redraws; events.push_back(80); } } TutWinBaseWindow;

static void auto_undo() { events.push_back(1); }
static int editor_who(int faction, int mode) {
  assert(faction == ConsoleGlobal.faction_id_ && mode == 1);
  events.push_back(2);
  return chosen_faction;
}
static int parse_says(int id, const char *text, int gender, int plural) {
  assert(text != 0 && gender == -1 && plural == -1);
  say_ids.push_back(id);
  gender_seen.push_back(GenderDefault);
  plurality_seen.push_back(PluralityDefault);
  events.push_back(10 + id);
  return 0;
}
static int parse_num(int id, int value) {
  parsed_ids.push_back(id);
  parsed_values.push_back(value);
  events.push_back(20 + id);
  return 0;
}
static int X_pop_ask_number(const char *label, int value, NullCallback callback) {
  assert(label == ConsoleStrEditorEnergy);
  assert(value == PlayersData[chosen_faction].energy_reserves);
  assert(callback == 0);
  events.push_back(70);
  return popup_answer;
}
"""


def program(method: str, main: str) -> str:
    return PRELUDE + "\n" + method + "\n" + main


def compile_run(tmp_path: Path, text: str) -> subprocess.CompletedProcess[str]:
    compiler = shutil.which("g++")
    if compiler is None:
        raise AssertionError("g++ is required for this actual-source host control")
    tmp_path.mkdir(parents=True, exist_ok=True)
    cpp = tmp_path / "control.cpp"
    exe = tmp_path / "control"
    cpp.write_text(text)
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
        capture_output=True, text=True, check=False, timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(exe)], capture_output=True, text=True, check=False, timeout=10)


def test_actual_body_parses_each_factions_energy_and_preserves_cancel_and_accept(tmp_path: Path) -> None:
    source = SOURCE.read_text()
    method = body(source)
    assert "PlayersData[loop_faction].energy_reserves" in method
    assert "PlayersData[loop_faction].unk_48" not in method
    main = r"""
static void reset_fixture() {
  events.clear(); parsed_ids.clear(); parsed_values.clear(); say_ids.clear();
  gender_seen.clear(); plurality_seen.clear();
  chosen_faction=3; popup_answer=1; redraws=updates=0;
  ConsoleGlobal.faction_id_=6;
  for (int i=0;i!=8;++i) {
    Players[i].is_leader_female=i%2;
    Players[i].adj_name_faction[0]='A'+i; Players[i].adj_name_faction[1]=0;
    Players[i].name_leader[0]='L'+i; Players[i].name_leader[1]=0;
    PlayersData[i].energy_reserves=1000+i*17;
    PlayersData[i].unk_48=5000+i*31;
  }
}
static void assert_loop_values() {
  assert(parsed_ids.size()==7 && parsed_values.size()==7);
  assert(say_ids.size()==8 && say_ids[0]==0);
  for (int i=1;i!=8;++i) {
    assert(say_ids[i]==i);
    assert(parsed_ids[i-1]==i);
    assert(parsed_values[i-1]==1000+i*17);
    assert(gender_seen[i]==i%2 && plurality_seen[i]==0);
  }
}
int main() {
  Console console;
  reset_fixture();
  console.editor_energy();
  assert_loop_values();
  assert(events.size()==18 && events[0]==1 && events[1]==2 && events[2]==10);
  for (int i=1;i!=8;++i) {
    assert(events[3+(i-1)*2]==10+i);
    assert(events[4+(i-1)*2]==20+i);
  }
  assert(events[17]==70 && redraws==0 && updates==0);
  for (int i=0;i!=8;++i) {
    assert(PlayersData[i].energy_reserves==1000+i*17);
    assert(PlayersData[i].unk_48==5000+i*31);
  }

  reset_fixture(); popup_answer=0; ParseNumTable[0]=4321;
  console.editor_energy();
  assert_loop_values();
  assert(PlayersData[3].energy_reserves==4321);
  assert(PlayersData[3].unk_48==5093);
  for (int i=0;i!=8;++i) if(i!=3) assert(PlayersData[i].energy_reserves==1000+i*17);
  assert(redraws==1 && updates==1 && events[18]==80 && events[19]==90);

  reset_fixture(); chosen_faction=-1;
  console.editor_energy();
  assert(events.size()==2 && events[0]==1 && events[1]==2);
  assert(parsed_ids.empty() && say_ids.empty() && redraws==0 && updates==0);
  return 0;
}
"""
    result = compile_run(tmp_path / "correct", program(method, main))
    assert result.returncode == 0, result.stdout + result.stderr


def test_old_member_damage_control_fails(tmp_path: Path) -> None:
    method = body(SOURCE.read_text())
    damaged = method.replace(
        "PlayersData[loop_faction].energy_reserves",
        "PlayersData[loop_faction].unk_48",
        1,
    )
    assert damaged != method
    main = r"""
int main() {
  Console console;
  ConsoleGlobal.faction_id_=2; chosen_faction=3; popup_answer=1;
  for (int i=0;i!=8;++i) {
    Players[i].is_leader_female=0;
    Players[i].adj_name_faction[0]='A'; Players[i].adj_name_faction[1]=0;
    Players[i].name_leader[0]='L'; Players[i].name_leader[1]=0;
    PlayersData[i].energy_reserves=100+i;
    PlayersData[i].unk_48=900+i;
  }
  console.editor_energy();
  assert(parsed_values.size()==7);
  for (int i=1;i!=8;++i) assert(parsed_values[i-1]==100+i);
  return 0;
}
"""
    result = compile_run(tmp_path / "mutant", program(damaged, main))
    assert result.returncode != 0
