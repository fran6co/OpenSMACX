"""Actual-source controls for the secret-project suffix of Facility."""
from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS


def root_dir() -> Path:
    for candidate in Path(__file__).resolve().parents:
        if (candidate / "CMakeLists.txt").is_file() and (candidate / "src/IMAGE_PIN").is_file():
            return candidate
    raise RuntimeError("repository root not found")


def _blank(match):
    return "".join("\n" if char == "\n" else " " for char in match.group(0))


def code_only(text: str) -> str:
    return COMMENTS_OR_STRINGS.sub(_blank, text)


def extract_function(text: str, signature: str) -> str:
    masked = code_only(text)
    assert masked.count(signature) == 1, signature
    start = masked.index(signature)
    opening = masked.index("{", start + len(signature))
    depth = 0
    for pos in range(opening, len(masked)):
        if masked[pos] == "{":
            depth += 1
        elif masked[pos] == "}":
            depth -= 1
            if depth == 0:
                return text[start:pos + 1]
    raise AssertionError(f"unterminated function {signature}")


SCAFFOLD = r'''#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#define __cdecl
// The source is a 32-bit game. Use its 32-bit name-handle slot in this host
// fixture so the exact original `(int)record.name` expressions are portable.
typedef unsigned int LPSTR;
struct RulesFacility {
  LPSTR name; LPSTR effect; unsigned int pad; int cost;
  unsigned int maint; int preq_tech; int free_tech;
  int sp_ai_fight, sp_ai_growth, sp_ai_tech, sp_ai_wealth, sp_ai_power;
};
enum { MaxFacilityNum=134, MaxSecretProjectNum=64, FacilitySPStart=70 };
typedef char facility_record_size[(sizeof(RulesFacility)==0x30)?1:-1];
RulesFacility Facility[MaxFacilityNum] = {};
char StringTemp[1024];
char ProjectNames[MaxSecretProjectNum][32];
char *label_get(int) { return (char*)"COST_LABEL"; }
struct Strings { char *get(int id) { return (char*)ProjectNames[id]; } };
Strings StringTable;
int _itoa(int value, char *out, int radix) {
  if (radix == 10) std::sprintf(out, "%d", value);
  else std::sprintf(out, "%x", value);
  return 0;
}
int Language = 0;
const char *ConceptArticle = "The ";
const char *CategoryIdSeparator = "|";
struct GameDataState { unsigned int game_state_; };
struct GameDataStateRoot { GameDataState fields_; };
GameDataStateRoot GameDataStateStorage;
struct Rect { int left, top, right, bottom; };
typedef Rect RECT;
struct Font {};
struct ListBox {
  std::vector<std::string> texts; std::vector<int> ids;
  void item(char *text, int id) { texts.push_back(text); ids.push_back(id); }
};
struct Datalink {
  ListBox listBox_; RECT category_rect_; Font font3_;
  void set_cat_project();
};
int InterfaceBoxStyles[7];
void box_sprite(RECT *, int *) {}
void set_text_color(int, int, int, int) {}
void set_font(Font *, int, int, int) {}
const char *category_label(int) { return "CATEGORY"; }
#define CATEGORY_LABEL(id) category_label(id)
void write_cent_l(char *, RECT *, int) {}
void soft_update(RECT *) {}
__ACTUAL_SAY_PROJECT__
__ACTUAL_SET_CAT_PROJECT__
int main() {
  for (int i=0; i<MaxSecretProjectNum; ++i) {
    Facility[FacilitySPStart+i].preq_tech=-2;
    std::sprintf(ProjectNames[i], "Row%d", i);
    Facility[FacilitySPStart+i].name=(unsigned int)i;
  }
  Facility[69].name=999;
  Facility[70].name=0;
  std::strcpy(ProjectNames[0], "The Alpha");
  std::strcpy(ProjectNames[1], "Skip");
  std::strcpy(ProjectNames[2], "Beta");
  std::strcpy(ProjectNames[63], "Last");
  Facility[70].preq_tech=-1;
  Facility[71].preq_tech=-2;
  Facility[72].preq_tech=0;
  Facility[133].preq_tech=1;

  // Formatter behavior: only name is looked up/appended. The label is
  // appended to StringTemp, not output; keep this odd original behavior.
  char output[256]="prefix";
  say_project(output, 0, 1);
  assert(std::strcmp(output, "prefixThe Alpha ()")==0);
  assert(std::strcmp(StringTemp, "COST_LABEL")==0);
  char last[256]="";
  say_project(last, 63, 0);
  assert(std::strcmp(last, "Last")==0);

  Datalink dl = {};
  Language=0;
  GameDataStateStorage.fields_.game_state_=0;
  dl.set_cat_project();
  assert(dl.listBox_.ids.size()==3);
  assert(dl.listBox_.ids[0]==68534 && dl.listBox_.texts[0]=="Alpha");
  assert(dl.listBox_.ids[1]==68536 && dl.listBox_.texts[1]=="Beta");
  assert(dl.listBox_.ids[2]==68597 && dl.listBox_.texts[2]=="Last");

  Datalink with_ids = {};
  Language=1;
  GameDataStateStorage.fields_.game_state_=0x1000;
  with_ids.set_cat_project();
  assert(with_ids.listBox_.ids.size()==3);
  assert(with_ids.listBox_.texts[0]=="The Alpha|68534");
  assert(with_ids.listBox_.texts[1]=="Beta|68536");
  assert(with_ids.listBox_.texts[2]=="Last|68597");
  return 0;
}
'''


def test_facility_suffix_identity_and_actual_consumers():
    root = root_dir()
    base_h = (root / "src/base.h").read_text()
    alpha = (root / "src/alpha.cpp").read_text()
    map_cpp = (root / "src/map.cpp").read_text()
    datalink = (root / "src/datalink.cpp").read_text()
    assert "MaxFacilityNum = 134" in base_h
    assert "MaxSecretProjectNum = 64" in base_h
    assert "FacilitySPStart = 70" in base_h
    assert "sizeof(RulesFacility) == 0x30" in base_h
    assert "RulesFacility Facility[MaxFacilityNum]" in alpha
    assert "for (i = 1; i < MaxFacilityNum; i++)" in alpha
    assert "if (i >= FacilitySPStart)" in alpha
    assert "SecretProjectRules" not in code_only(map_cpp)
    assert "SecretProjectRules" not in code_only(datalink)
    assert "0x009A5888" not in code_only(map_cpp)
    assert "0x009A5888" not in code_only(datalink)

    say = extract_function(map_cpp, "void __cdecl say_project(void *text, int project_id, int with_cost)")
    cat = extract_function(datalink, "void Datalink::set_cat_project()")
    say_code = code_only(say)
    cat_code = code_only(cat)
    assert "Facility[FacilitySPStart + project_id].name" in say_code
    assert "strcat(StringTemp, label_get(179));" in say_code
    assert cat_code.count("record->preq_tech >= -1") == 1
    assert cat_code.count("record->name") == 1
    assert "&Facility[FacilitySPStart]" in cat_code
    assert "++record" in cat_code or "record++" in cat_code
    assert "index < 0x40" in cat_code


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_formatter_and_datalink_bodies(compiler_name: str, tmp_path: Path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} is unavailable")
    root = root_dir()
    say = extract_function(
        (root / "src/map.cpp").read_text(),
        "void __cdecl say_project(void *text, int project_id, int with_cost)")
    cat = extract_function(
        (root / "src/datalink.cpp").read_text(),
        "void Datalink::set_cat_project()")
    source = SCAFFOLD.replace("__ACTUAL_SAY_PROJECT__", say).replace(
        "__ACTUAL_SET_CAT_PROJECT__", cat)
    exe = tmp_path / "secret_project_rules_owner"
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-x", "c++",
         "-o", str(exe), "-"], input=source, text=True, capture_output=True)
    assert built.returncode == 0, built.stderr
    ran = subprocess.run([str(exe)], text=True, capture_output=True)
    assert ran.returncode == 0, ran.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("damage", ("formatter_start", "datalink_start",
                                     "datalink_stride", "datalink_preq"))
def test_host_fixture_rejects_wrong_owner_mapping(
    compiler_name: str, damage: str, tmp_path: Path
):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} is unavailable")
    root = root_dir()
    say = extract_function(
        (root / "src/map.cpp").read_text(),
        "void __cdecl say_project(void *text, int project_id, int with_cost)")
    cat = extract_function(
        (root / "src/datalink.cpp").read_text(),
        "void Datalink::set_cat_project()")
    if damage == "formatter_start":
        old = "Facility[FacilitySPStart + project_id].name"
        new = "Facility[FacilitySPStart + 1 + project_id].name"
        say = say.replace(old, new, 1)
        assert old not in say
    elif damage == "datalink_start":
        old = "&Facility[FacilitySPStart]"
        cat = cat.replace(old, "&Facility[FacilitySPStart + 1]", 1)
        assert old not in cat
    elif damage == "datalink_stride":
        old = "++record;"
        cat = cat.replace(old, "/* lost row advance */", 1)
        assert old not in cat
    else:
        old = "record->preq_tech >= -1"
        cat = cat.replace(old, "record->preq_tech >= 0", 1)
        assert old not in cat
    source = SCAFFOLD.replace("__ACTUAL_SAY_PROJECT__", say).replace(
        "__ACTUAL_SET_CAT_PROJECT__", cat)
    exe = tmp_path / f"secret_project_damage_{damage}"
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-x", "c++",
         "-o", str(exe), "-"], input=source, text=True, capture_output=True)
    assert built.returncode == 0, built.stderr
    ran = subprocess.run([str(exe)], text=True, capture_output=True)
    assert ran.returncode != 0, f"{damage} damage escaped the behavioral controls"
