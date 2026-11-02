"""Observable call/order supplement to the existing stale-tail tests."""
import os
from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery.source import read_file

_HERE = Path(__file__).resolve()
_DEFAULT_ROOT = next(parent for parent in _HERE.parents
                     if (parent / "src/designwin.cpp").is_file())
ROOT = Path(os.environ.get("OPENSMACX_SOURCE_ROOT", _DEFAULT_ROOT))
SOURCE = ROOT / "src/designwin.cpp"
SIGNATURE = "void DesignWin::suggest_abil(int ability)"


def actual_body(*, cached_results=False):
    records = read_file(SOURCE)
    row, = [item for item in records if item.address == 0x00439D30]
    assert Path(row.path).resolve() == SOURCE.resolve()
    assert row.region and SIGNATURE in row.region
    text = SOURCE.read_text()
    start = text.index(SIGNATURE)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            body = text[start:end + 1]
            break
    else:
        raise AssertionError("unterminated actual DesignWin::suggest_abil")
    if cached_results:
        first = "    if (!has_tech(Ability[abil_index(ability)].preq_tech,"
        assert body.count(first) == 1
        body = body.replace(first,
            "    const int ability_id = abil_index(ability);\n"
            "    if (!has_tech(Ability[ability_id].preq_tech,", 1)
        fresh_cond = "abil_cond(abil_index(ability), chassis_id, weapon_id, armor_id)"
        cached_cond = "abil_cond(ability_id, chassis_id, weapon_id, armor_id)"
        assert body.count(fresh_cond) == 1
        body = body.replace(fresh_cond, cached_cond, 1)
        fresh_scan = "const int bonus_ability_id = abil_index(ability);"
        cached_scan = "const int bonus_ability_id = ability_id;"
        assert body.count(fresh_scan) == 1
        body = body.replace(fresh_scan, cached_scan, 1)
    return body


HARNESS = r'''#include <cassert>
#include <cstring>
#include <vector>
typedef unsigned int uint32_t;
struct AbilityRecord { int preq_tech; };
struct Player {
  int faction_bonus_count;
  // Host backing is widened to keep the source's original 0x29-word walk
  // defined while observing its loop and callback order.
  int faction_bonus_id[0x2A];
  int faction_bonus_val1[0x2A];
};
struct Console { int faction_id_; };
struct GameData { struct Fields { unsigned int game_preferences_; } fields_; };
struct RulesRecord { int tech_two_spec_abilities; };
static Player Players[2];
static Console ConsoleGlobal;
static GameData GameDataStateStorage;
static RulesRecord Rules;
static AbilityRecord Ability[29];
struct Event {
  int kind, a, b, c, d;
  Event(int kind_, int a_ = 0, int b_ = 0, int c_ = 0, int d_ = 0)
      : kind(kind_), a(a_), b(b_), c(c_), d(d_) {}
  bool operator==(const Event &other) const {
    return kind == other.kind && a == other.a && b == other.b &&
           c == other.c && d == other.d;
  }
};
static std::vector<Event> events;
static int mode, ability_calls, bit_count_calls;
static void after_has_tech(int tech);
static void after_ability_call();
static void after_bit_count();
static int has_tech(int tech, int faction) {
  events.push_back(Event(2, tech, faction));
  after_has_tech(tech);
  return 1;
}
static int abil_cond(int ability, int chassis, int weapon, int armor) {
  events.push_back(Event(3, ability, chassis, weapon, armor));
  return 1;
}
static int bit_count(int value) {
  events.push_back(Event(4, value));
  unsigned int bits = static_cast<unsigned int>(value);
  int count = 0;
  for (; bits; bits &= bits - 1u) ++count;
  after_bit_count();
  return count;
}
static int abil_index(int value) {
  events.push_back(Event(1));
  after_ability_call();
  int index = 0;
  while ((value & 1) == 0) { value >>= 1; ++index; }
  return index;
}
class DesignWin {
public:
  uint32_t protoAbilityFlags_;
  int protoAbilityIDs_[2];
  int factionID_, protoChassisType_, protoWeaponType_, protoArmorType_;
  void check_abil() { events.push_back(Event(0)); }
  void suggest_abil(int ability);
};
static DesignWin *active;
static void after_has_tech(int tech) {
  if (mode == 1 && tech == 55) active->factionID_ = 6;
}
static void after_ability_call() {
  ++ability_calls;
  // The value-row base has already been captured from faction0, but the
  // next ID row must follow the standing faction changed by this callback.
  if (mode == 3 && ability_calls == 3) ConsoleGlobal.faction_id_ = 1;
}
static void after_bit_count() {
  ++bit_count_calls;
  if (mode == 2 && bit_count_calls == 1) active->protoAbilityFlags_ = 0;
}
/* ACTUAL_SUGGEST_ABIL */
int main() {
  DesignWin win;
  active = &win;
  win.protoAbilityFlags_ = 1u << 2;
  win.protoAbilityIDs_[0] = 2;
  win.protoAbilityIDs_[1] = -1;
  win.factionID_ = 4;
  win.protoArmorType_ = 1;
  win.protoWeaponType_ = 2;
  win.protoChassisType_ = 3;
  ConsoleGlobal.faction_id_ = 0;
  GameDataStateStorage.fields_.game_preferences_ = 0x4000;
  Rules.tech_two_spec_abilities = 99;
  Ability[5].preq_tech = 55;
  Players[0].faction_bonus_count = 0;
  std::memset(Players[0].faction_bonus_id, 0, sizeof(Players[0].faction_bonus_id));
  std::memset(Players[0].faction_bonus_val1, 0, sizeof(Players[0].faction_bonus_val1));
  // First matching-ID row is not this ability; the next one is. The original
  // recalculates the requested index once per ID-15 row and returns at row 1.
  Players[0].faction_bonus_id[0] = 15;
  Players[0].faction_bonus_val1[0] = 99;
  Players[0].faction_bonus_id[1] = 15;
  Players[0].faction_bonus_val1[1] = 5;
  win.suggest_abil(1u << 5);

  const Event expected[] = {
    Event(0), Event(1), Event(2,55,4), Event(1), Event(3,5,3,2,1),
    Event(4,4), Event(4,4), Event(2,99,4), Event(1), Event(1)
  };
  assert(events.size() == sizeof(expected) / sizeof(expected[0]));
  for (unsigned int i = 0; i < events.size(); ++i) assert(events[i] == expected[i]);
  assert(win.protoAbilityFlags_ == (1u << 2));
  assert(win.protoAbilityIDs_[0] == 2 && win.protoAbilityIDs_[1] == -1);

  // A later has_tech must read the member after intervening calls, rather
  // than use the faction captured for the first technology gate.
  mode = 1; ability_calls = bit_count_calls = 0; events.clear();
  win.suggest_abil(1u << 5);
  assert(events.size() == 10);
  assert(events[2] == Event(2,55,4));
  assert(events[7] == Event(2,99,6));

  // Both bit_count calls are real and take fresh flag values. The first
  // reports one bit, then clears the member before the second read.
  mode = 2; ability_calls = bit_count_calls = 0; events.clear();
  win.factionID_ = 4; win.protoAbilityFlags_ = 4;
  win.suggest_abil(1u << 5);
  assert(bit_count_calls == 2 && events.size() == 9);
  assert(events[5] == Event(4,4) && events[6] == Event(4,0));
  assert(win.protoAbilityFlags_ == 0);

  mode = 3; ability_calls = bit_count_calls = 0; events.clear();
  win.protoAbilityFlags_ = 4; ConsoleGlobal.faction_id_ = 0;
  Players[0].faction_bonus_val1[1] = 99;
  std::memset(Players[1].faction_bonus_id, 0, sizeof(Players[1].faction_bonus_id));
  std::memset(Players[1].faction_bonus_val1, 0, sizeof(Players[1].faction_bonus_val1));
  Players[1].faction_bonus_id[1] = 15;
  Players[1].faction_bonus_val1[0] = 5; // wrong post-callback row must not win early
  Players[1].faction_bonus_val1[1] = 5;
  win.suggest_abil(1u << 5);
  assert(ability_calls == 4 && bit_count_calls == 2);
  assert(win.protoAbilityFlags_ == 4);
  assert(win.protoAbilityIDs_[0] == 2 && win.protoAbilityIDs_[1] == -1);
  return 0;
}
'''


def compile_and_run(tmp_path, compiler, *, cached_results=False, expect_failure=False):
    executable = shutil.which(compiler)
    if not executable:
        pytest.skip(compiler + " unavailable")
    source = tmp_path / ("suggest-cached.cpp" if cached_results else "suggest-live.cpp")
    binary = tmp_path / source.stem
    source.write_text(HARNESS.replace(
        "/* ACTUAL_SUGGEST_ABIL */", actual_body(cached_results=cached_results)))
    built = subprocess.run(
        [executable, "-std=c++03", "-O1", "-Wall", "-Wextra", "-Werror",
         "-pedantic", str(source), "-o", str(binary)],
        capture_output=True, text=True, timeout=60)
    assert built.returncode == 0, built.stdout + built.stderr
    run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)
    if expect_failure:
        assert run.returncode != 0, "cached-index/collapsed-call mutant survived"
        assert "events.size()" in run.stderr
    else:
        assert run.returncode == 0, run.stdout + run.stderr


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_actual_suggest_abil_recomputes_ids_at_each_pinned_call_site(
        tmp_path, compiler):
    compile_and_run(tmp_path, compiler)


def test_cached_ability_id_mutant_collapses_pinned_call_sites(tmp_path):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("no host C++ compiler")
    compile_and_run(tmp_path, compiler, cached_results=True, expect_failure=True)
