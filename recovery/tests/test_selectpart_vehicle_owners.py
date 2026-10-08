"""Compile host controls from the actual SelectPartWin source selection bodies."""

from __future__ import annotations

import re
import shutil
import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


def _definition(source: str, signature: str) -> str:
    matches = list(re.finditer(rf"(?m)^{re.escape(signature)}\s*\{{", source))
    assert len(matches) == 1, f"expected one installed definition: {signature}"
    start = matches[0].start()
    opening = source.index("{", start)
    depth, state, i = 0, "code", opening
    while i < len(source):
        ch = source[i]
        nxt = source[i + 1] if i + 1 < len(source) else ""
        if state == "line":
            if ch == "\n": state = "code"
        elif state == "block":
            if ch == "*" and nxt == "/": state = "code"; i += 1
        elif state in ("string", "char"):
            if ch == "\\": i += 1
            elif (state == "string" and ch == '"') or (state == "char" and ch == "'"):
                state = "code"
        elif ch == "/" and nxt == "/": state = "line"; i += 1
        elif ch == "/" and nxt == "*": state = "block"; i += 1
        elif ch == '"': state = "string"
        elif ch == "'": state = "char"
        elif ch == "{": depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                return source[start:i + 1]
        i += 1
    raise AssertionError(f"unclosed definition: {signature}")


def _actual_bodies() -> tuple[str, str, str]:
    source = (ROOT / "src/selectpartwin.cpp").read_text(encoding="utf-8")
    chassis = _definition(source, "int SelectPartWin::draw_chassis(int a1, RECT *a2, int a3)")
    start = chassis.index("const RulesChassis *entry = Chassis;")
    end = chassis.index("\n\n    if (slot == 9)", start)
    chassis_loop = chassis[start:end]
    armor = _definition(source, "int SelectPartWin::draw_armor(int a1, RECT *a2, int a3)")
    start = armor.index("    int slot;\n")
    end = armor.index("\n    if (slot == 0xE)", start)
    armor_choice = armor[start:end]
    body = _definition(source,
        "int SelectPartWin::exec(int x, int y, int value0, int kind, int selected,\n"
        "                        int chassis, int weapon, int armor, int abilities,\n"
        "                        int reactor, unsigned char *palette)")
    vector_lines = "\n".join(line for line in body.splitlines()
                             if "caviar_.light_vector()" in line or
                             "caviar_.shadow_vector()" in line)
    assert vector_lines.count("VehLightDirMap") == 2
    return chassis_loop, armor_choice, vector_lines


HOST_SUPPORT = r'''#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

struct RECT { int left, top, right, bottom; };

struct RulesChassis {
  unsigned char prefix[0x48];
  unsigned char speed;
  unsigned char triad;
  unsigned char flags[3];
  unsigned char missile;
  unsigned char tail[0x40];
  int16_t preq_tech;
};
struct RulesArmor {
  unsigned char prefix[0x0C];
  int16_t preq_tech;
  unsigned char tail[2];
};
static_assert(sizeof(RulesChassis) == 0x90);
static_assert(offsetof(RulesChassis, speed) == 0x48);
static_assert(offsetof(RulesChassis, triad) == 0x49);
static_assert(offsetof(RulesChassis, missile) == 0x4D);
static_assert(offsetof(RulesChassis, preq_tech) == 0x8E);
static_assert(sizeof(RulesArmor) == 0x10);
static_assert(offsetof(RulesArmor, preq_tech) == 0x0C);
static constexpr int MaxChassisNum = 9;
static constexpr int ChassisStorageCount = 19;
static constexpr int MaxArmorNum = 14;
RulesChassis Chassis[ChassisStorageCount]{};
RulesArmor Armor[MaxArmorNum]{};
int field_2B68_ = 0;
int field_2B88_ = 0;
int tech_result = 1;
struct TechCall { int tech; int faction; };
std::vector<TechCall> tech_calls;
int has_tech(int tech, int faction) {
  tech_calls.push_back({tech, faction});
  return tech_result;
}
struct ChassisResult { int slot, idx, no_tech; bool end; };
struct ArmorResult { int slot, no_tech; bool end; };
struct VOX_Vect { float x, y, z; };
VOX_Vect VehLightDirMap{1.25f, -2.5f, 3.75f};
struct Caviar {
  VOX_Vect light{}, shadow{};
  VOX_Vect &light_vector() { return light; }
  VOX_Vect &shadow_vector() { return shadow; }
} caviar_;
'''


def test_actual_picker_loops_and_vector_assignments_on_gcc_and_clang(tmp_path: Path) -> None:
    chassis_loop, armor_choice, vector_lines = _actual_bodies()
    unit = HOST_SUPPORT + "\n"
    unit += "ChassisResult choose_chassis(int a1) {\n" + chassis_loop
    unit += "\nreturn {slot, idx, has_all_tech_flag, slot == 9};\n}\n"
    unit += "ArmorResult choose_armor(int a1, int chassis_value) {\n"
    unit += "int field_2B88_ = chassis_value;\nint field_2B68_ = 41;\n"
    unit += armor_choice
    unit += "\nreturn {slot, noTechFlag, slot == 0xE};\n}\n"
    unit += "void copy_camera_vectors() {\n" + vector_lines + "\n}\n"
    unit += r'''
int main() {
  field_2B68_ = 41;
  for (int i = 0; i < ChassisStorageCount; ++i) Chassis[i].preq_tech = -2;
  Chassis[1].preq_tech = -1;
  Chassis[3].preq_tech = 7;
  tech_calls.clear(); tech_result = 1;
  ChassisResult c = choose_chassis(0);
  assert(c.slot == 1 && c.idx == 0 && c.no_tech == 0 && !c.end);
  assert(tech_calls.size() == 1 && tech_calls[0].tech == -1 && tech_calls[0].faction == 41);
  tech_calls.clear(); c = choose_chassis(1);
  assert(c.slot == 3 && c.idx == 1 && !c.end);
  assert(tech_calls.size() == 1 && tech_calls[0].tech == 7);
  for (int i = 0; i < MaxChassisNum; ++i) Chassis[i].preq_tech = -1;
  tech_calls.clear(); c = choose_chassis(8);
  assert(c.slot == 8 && c.idx == 8 && !c.end);
  tech_calls.clear(); c = choose_chassis(9);
  assert(c.slot == 9 && c.idx == 9 && c.end && tech_calls.empty());

  Chassis[2].missile = 1;
  tech_calls.clear();
  ArmorResult a = choose_armor(0, 2);
  assert(a.slot == 0 && a.no_tech == 0 && !a.end && tech_calls.empty());
  Chassis[2].missile = 0;
  for (int i = 0; i < MaxArmorNum; ++i) Armor[i].preq_tech = -2;
  tech_calls.clear(); a = choose_armor(0, 2);
  assert(a.slot == 14 && a.no_tech == 0 && a.end && tech_calls.empty());
  Armor[0].preq_tech = -1; Armor[2].preq_tech = 3;
  tech_result = 0; tech_calls.clear(); a = choose_armor(0, 2);
  assert(a.slot == 0 && a.no_tech == 1 && !a.end);
  assert(tech_calls.size() == 1 && tech_calls[0].tech == -1 && tech_calls[0].faction == 41);
  a = choose_armor(1, 2);
  assert(a.slot == 2 && a.no_tech == 1 && !a.end);
  for (int i = 0; i < MaxArmorNum; ++i) Armor[i].preq_tech = -1;
  tech_calls.clear(); a = choose_armor(13, 2);
  assert(a.slot == 13 && !a.end);
  tech_calls.clear(); a = choose_armor(14, 2);
  assert(a.slot == 14 && a.end && tech_calls.empty());

  copy_camera_vectors();
  assert(caviar_.light.x == 1.25f && caviar_.light.y == -2.5f && caviar_.light.z == 3.75f);
  assert(caviar_.shadow.x == 1.25f && caviar_.shadow.y == -2.5f && caviar_.shadow.z == 3.75f);
  return 0;
}
'''
    source_path = tmp_path / "selectpart_owner_host.cpp"
    source_path.write_text(unit, encoding="utf-8")
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    assert compilers, "GCC or Clang required for extracted host control"
    for index, compiler in enumerate(compilers):
        binary = tmp_path / f"selectpart_owner_host_{index}"
        built = subprocess.run([compiler, "-std=c++17", "-O0", str(source_path), "-o", str(binary)],
                               text=True, capture_output=True)
        assert built.returncode == 0, f"{compiler}: {built.stderr}"
        ran = subprocess.run([str(binary)], text=True, capture_output=True)
        assert ran.returncode == 0, f"{compiler}: {ran.stderr}"
