"""Actual-source controls for SocialWin's typed state and rules owners."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/socialwin.cpp"


def body(text, signature):
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError(f"unterminated function {signature}")


def test_five_receivers_use_real_typed_owners_and_keep_string_validation():
    text = SOURCE.read_text()
    mouse = body(text, "void SocialWin::draw_mouseover()")
    info = body(text, "void SocialWin::draw_info()")
    init = body(text, "void SocialWin::init(")

    assert "static const int *const SocialCategoryRow" not in text
    assert "static const char *const SocialEffectRowBase" not in text
    assert "static int *const PlayerPowerBlock" not in text
    assert "static int *const PlayerUpheavalBlock" not in text
    assert "static int *const MainPanelHeight" not in text
    assert "reinterpret_cast<int>(SocialCategories[cat_idx].name[row_idx])" in mouse
    assert "PlayersData[self_faction].soc_effect_base.values_[i]" in mouse
    assert "SocialEffects[i].set2" in mouse
    assert "PlayersData[factionID_].soc_upheaval_cost_paid" in info
    assert "MainInterfaceGlobal.panel_height()" in init
    assert "StringTable.get(reinterpret_cast<int>(\n                    SocialCategories[row].name[col]))" in init
    assert "SocialEffects[row].set2" in init
    assert "while (row < 11)" in init and "if (row != 3)" in init
    assert "++row;\n            entry += 1;\n            button_offset += 0xB4C;" in init


def test_actual_owner_expressions_preserve_player_values_and_panel_metric(tmp_path):
    text = SOURCE.read_text()
    mouse = body(text, "void SocialWin::draw_mouseover()")
    info = body(text, "void SocialWin::draw_info()")
    init = body(text, "void SocialWin::init(")
    player_read = "v = PlayersData[self_faction].soc_effect_base.values_[i];"
    upheaval_read = "upheaval -= PlayersData[factionID_].soc_upheaval_cost_paid;"
    panel_read = "int cy = (WinScreenHeight - MainInterfaceGlobal.panel_height()) - 0x162;"
    assert player_read in mouse and upheaval_read in info and panel_read in init
    harness = r'''#include <cstdlib>
struct SocialEffect { int values_[11]; };
struct PlayerData { int soc_upheaval_cost_paid; int energy_reserves; SocialEffect soc_effect_base; };
PlayerData PlayersData[8];
struct MainInterface { int height; int panel_height() const { return height; } } MainInterfaceGlobal;
int selected_power() { int self_faction=4, i=6, v=0; __PLAYER_READ__ return v; }
int adjusted_upheaval() { int factionID_=4, upheaval=90; __UPHEAVAL_READ__ return upheaval; }
int panel_y() { int WinScreenHeight=600; __PANEL_READ__ return cy; }
int main() {
  PlayersData[4].soc_effect_base.values_[6]=37;
  PlayersData[4].soc_upheaval_cost_paid=12;
  MainInterfaceGlobal.height=300;
  if (selected_power()!=37 || adjusted_upheaval()!=78 || panel_y()!=-54) return 1;
  PlayersData[4].soc_effect_base.values_[6]=51;
  PlayersData[4].soc_upheaval_cost_paid=22;
  MainInterfaceGlobal.height=320;
  if (selected_power()!=51 || adjusted_upheaval()!=68 || panel_y()!=-74) return 2;
}
'''
    harness = harness.replace("__PLAYER_READ__", player_read)
    harness = harness.replace("__UPHEAVAL_READ__", upheaval_read)
    harness = harness.replace("__PANEL_READ__", panel_read)
    _compile_run(tmp_path, harness)
    for index, changed in enumerate((
            harness.replace("soc_effect_base.values_[i]", "soc_effect_base.values_[i + 1]", 1),
            harness.replace(
                "upheaval -= PlayersData[factionID_].soc_upheaval_cost_paid;",
                "upheaval -= PlayersData[factionID_].energy_reserves;", 1),
            harness.replace("MainInterfaceGlobal.panel_height()", "0", 1))):
        mutated = tmp_path / f"socialwin-owner-mutant-{index}.cpp"
        binary = tmp_path / f"socialwin-owner-mutant-{index}"
        mutated.write_text(changed)
        compiler = shutil.which("g++") or shutil.which("clang++")
        built = subprocess.run([compiler, "-std=c++17", str(mutated), "-o", str(binary)],
                               capture_output=True, text=True, timeout=30)
        assert built.returncode == 0, built.stderr
        run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
        assert run.returncode != 0


def test_actual_rules_names_keep_stringtable_lookup_and_skip_null_row(tmp_path):
    text = SOURCE.read_text()
    mouse = body(text, "void SocialWin::draw_mouseover()")
    init = body(text, "void SocialWin::init(")
    category_mouse = "table_index = reinterpret_cast<int>(SocialCategories[cat_idx].name[row_idx]);"
    category_init = "StringTable.get(reinterpret_cast<int>(\n                    SocialCategories[row].name[col]))"
    assert category_mouse in mouse and category_init in init
    loop_begin = init.index("        int row = 0;", init.index("buttonGroup_.init(2, 0);"))
    loop_end = init.index("\n    }\n\n    {\n        StringTemp[0] = 0;", loop_begin)
    loop = init[loop_begin:loop_end]
    category_harness = r'''#include <cstdlib>
#include <cstring>
typedef const char *LPSTR;
struct RulesSocialCategory { LPSTR type; int preq_tech[4]; LPSTR name[4]; };
RulesSocialCategory SocialCategories[4];
const char *expected[16]; int observed;
int host_handle(LPSTR value) {
  for (int i=0;i<16;++i) if (expected[i]==value) return i;
  return -1;
}
struct Strings { const char *get(int handle) { ++observed; return handle>=0?expected[handle]:nullptr; } } StringTable;
const char *mouse_title(int cat_idx,int row_idx) {
  int table_index=host_handle(SocialCategories[cat_idx].name[row_idx]);
  return StringTable.get(table_index);
}
const char *init_title(int row,int col) {
  return StringTable.get(host_handle(SocialCategories[row].name[col]));
}
int main() {
  char names[16][12];
  for (int i=0;i<16;++i) { std::strcpy(names[i],"category"); expected[i]=names[i]; }
  for (int row=0;row<4;++row) for(int col=0;col<4;++col)
    SocialCategories[row].name[col]=expected[row*4+col];
  if (mouse_title(2,3)!=expected[11] || init_title(2,3)!=expected[11] || observed!=2) return 1;
  if (mouse_title(0,1)!=expected[1] || init_title(0,1)!=expected[1] || observed!=4) return 2;
}
'''
    # Preserve the actual StringTable.get validation call; adapt only the
    # 32-bit pointer-to-int representation to a lookup handle for this host.
    category_harness = category_harness.replace(
        "host_handle(SocialCategories[cat_idx].name[row_idx])",
        category_mouse.split("= ", 1)[1].removesuffix(";").replace(
            "reinterpret_cast<int>(SocialCategories[cat_idx].name[row_idx])",
            "host_handle(SocialCategories[cat_idx].name[row_idx])"))
    actual_init_lookup = category_init.replace(
        "reinterpret_cast<int>(\n                    SocialCategories[row].name[col])",
        "host_handle(SocialCategories[row].name[col])")
    category_harness = category_harness.replace(
        "StringTable.get(host_handle(SocialCategories[row].name[col]))",
        actual_init_lookup)
    _compile_run(tmp_path, category_harness)
    effect_name_use = "strcat(StringTemp, SocialEffects[i].set2);"
    assert effect_name_use in mouse
    mouse_harness = r'''#include <cstdlib>
#include <cstdio>
#include <cstring>
struct RulesSocialEffect { char set1[24]; char set2[24]; char padding[56]; };
RulesSocialEffect SocialEffects[12]; char StringTemp[32];
int main() {
  for (int i=0;i<12;++i) std::sprintf(SocialEffects[i].set2,"name%d",i);
  for (int i=0;i<11;++i) {
    StringTemp[0]=0; __ACTUAL_APPEND__
    char expected[32]; std::sprintf(expected,"name%d",i);
    if (std::strcmp(StringTemp,expected)!=0) return 1;
  }
}
'''.replace("__ACTUAL_APPEND__", effect_name_use)
    _compile_run(tmp_path, mouse_harness)
    mouse_mutant = mouse_harness.replace(
        "strcat(StringTemp, SocialEffects[i].set2);",
        "strcat(StringTemp, SocialEffects[i + 1].set2);", 1)
    _expect_host_failure(tmp_path, "socialwin-mouse-name-mutant", mouse_mutant)
    # Exercise the actual row/name and mapping expressions in a compact host
    # projection; only pointer-to-int conversion is adapted for 64-bit hosts.
    name_expr = "const char *name = SocialEffects[row].set2;"
    mapping_start = loop.index("                int col;\n                int offset;")
    mapping_end = loop.index("                StringTemp[0] = 0;", mapping_start)
    mapping = loop[mapping_start:mapping_end]
    skip_statement = next(line.strip() for line in loop.splitlines()
                          if line.strip().startswith("if (row !="))
    harness = r'''#include <cstdlib>
#include <cstdio>
#include <cstring>
struct RulesSocialEffect { char set1[24]; char set2[24]; char padding[56]; };
RulesSocialEffect SocialEffects[11];
char expected_names[11][24];
int main() {
  for (int i=0;i<11;++i) { std::sprintf(expected_names[i],"effect%d",i); std::strcpy(SocialEffects[i].set2,expected_names[i]); }
  int row=0, entry=0, button_offset=0, emitted=0;
  int rows[10], cols[10], indices[10];
  while (row < 11) {
    __SKIP_STATEMENT__
      __NAME_EXPR__
      __MAP_EXPR__
      if (std::strcmp(name,expected_names[row])!=0) return 1;
      rows[emitted]=row; cols[emitted]=col;
      indices[emitted]=17 + offset / 0xB4C;
      ++emitted;
    }
    ++row; entry += 1; button_offset += 0xB4C;
  }
  if (emitted!=10 || rows[0]!=0 || rows[2]!=2 || rows[3]!=4 || rows[9]!=10) return 2;
  if (cols[0]!=0 || cols[2]!=8 || cols[3]!=12 || cols[9]!=36) return 3;
  if (indices[0]!=17 || indices[2]!=19 || indices[3]!=20 || indices[9]!=26) return 4;
}
'''.replace("__SKIP_STATEMENT__", skip_statement).replace(
        "__NAME_EXPR__", name_expr).replace("__MAP_EXPR__", mapping)
    _compile_run(tmp_path, harness)

    mutated = loop.replace("if (row != 3)", "if (row != 2)", 1)
    mutated_skip = next(line.strip() for line in mutated.splitlines()
                        if line.strip().startswith("if (row !="))
    mutated_harness = r'''#include <cstdlib>
#include <cstring>
struct RulesSocialEffect { char set1[24]; char set2[24]; char padding[56]; };
RulesSocialEffect SocialEffects[11];
int main() {
  for (int i=0;i<11;++i) std::strcpy(SocialEffects[i].set2,"effect");
  int row=0, entry=0, button_offset=0, emitted=0, saw_row3=0;
  while (row < 11) {
    __SKIP_STATEMENT__ __NAME_EXPR__ if (row==3) saw_row3=1; ++emitted; }
    ++row; entry += 1; button_offset += 0xB4C;
  }
  return emitted==10 && !saw_row3 ? 0 : 1;
}
'''.replace("__SKIP_STATEMENT__", mutated_skip).replace("__NAME_EXPR__", name_expr)
    compiler = shutil.which("g++") or shutil.which("clang++")
    cpp = tmp_path / "socialwin-skip-mutant.cpp"
    binary = tmp_path / "socialwin-skip-mutant"
    cpp.write_text(mutated_harness)
    built = subprocess.run([compiler, "-std=c++17", str(cpp), "-o", str(binary)],
                           capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stderr
    assert subprocess.run([str(binary)], capture_output=True, text=True,
                          timeout=10).returncode != 0

    name_mutant = harness.replace(name_expr,
                                  "const char *name = SocialEffects[row + 1].set2;", 1)
    _expect_host_failure(tmp_path, "socialwin-init-name-mutant", name_mutant)


def _compile_run(tmp_path, source):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    cpp = tmp_path / "socialwin-typed-state.cpp"
    binary = tmp_path / "socialwin-typed-state"
    cpp.write_text(source)
    build = subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                            str(cpp), "-o", str(binary)], capture_output=True,
                           text=True, timeout=30)
    assert build.returncode == 0, build.stderr
    run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
    assert run.returncode == 0, run.stderr


def _expect_host_failure(tmp_path, name, source):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    cpp = tmp_path / f"{name}.cpp"
    binary = tmp_path / name
    cpp.write_text(source)
    build = subprocess.run([compiler, "-std=c++17", str(cpp), "-o", str(binary)],
                           capture_output=True, text=True, timeout=30)
    assert build.returncode == 0, build.stderr
    run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
    assert run.returncode != 0
