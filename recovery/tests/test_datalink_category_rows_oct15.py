"""Actual-source controls for Datalink's typed technology/social row walks."""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        depth += (source[index] == "{") - (source[index] == "}")
        if depth == 0:
            return source[start:index + 1]
    raise AssertionError(f"unterminated body: {signature}")


def owners():
    source = (ROOT / "src/datalink.cpp").read_text()
    return (
        body(source, "void Datalink::set_cat_tech()"),
        body(source, "void Datalink::set_cat_social()"),
    )


def run_host(tmp_path: Path, tech: str, social: str):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host g++ required")
    program = r'''
#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
typedef char *LPSTR;
typedef unsigned int uint32_t;
struct RECT { int left, top, right, bottom; };
struct RulesTechnology { int preq_tech_1, preq_tech_2; LPSTR name; };
struct RulesSocialCategory { LPSTR type; int preq_tech[4]; LPSTR name[4]; };
struct GameFields { uint32_t game_state_; };
struct { GameFields fields_; } GameDataStateStorage = {{0x1000u}};
static char StringTemp[256];
static char tech_names[89][16];
static char social_types[4][16];
static char social_names[4][4][24];
RulesTechnology Technology[89];
RulesSocialCategory SocialCategories[4];
struct Entry { std::string text; int id; };
static std::vector<Entry> entries;
static void (*on_item)(int) = 0;
struct ListBox {
  void item(const char *text, int id) {
    entries.push_back(Entry{text, id});
    if (on_item) on_item(id);
  }
};
struct Strings { char *get(char *s) { return s; } } StringTable;
static char *decimal(int n, char *out, int) { std::sprintf(out, "%d", n); return out; }
#define _itoa decimal
static int InterfaceBoxStyles[8];
static const char *category_label(int) { return "category"; }
#define CATEGORY_LABEL(n) category_label(n)
static void box_sprite(RECT *, int *) {}
static void set_text_color(int,int,int,int) {}
static void set_font(int *,int,int,int) {}
static void write_cent_l(char *,RECT *,unsigned int) {}
static void soft_update(RECT *) {}
static const char DatalinkTechIdSeparator[] = "|";
static const char DatalinkSocialIdSeparator[] = "|";
struct Datalink {
  ListBox listBox_; RECT category_rect_; int font3_;
  Datalink() : category_rect_{0,0,100,80}, font3_(0) {}
  void set_cat_tech(); void set_cat_social();
};
TECH_BODY
SOCIAL_BODY
static void reset() {
  entries.clear(); on_item = 0; StringTemp[0] = 0;
  GameDataStateStorage.fields_.game_state_ = 0x1000u;
  for (int i=0; i<89; ++i) {
    std::sprintf(tech_names[i], "tech%d", i);
    Technology[i] = RulesTechnology{-1, -1, tech_names[i]};
  }
  for (int i=0; i<4; ++i) {
    std::sprintf(social_types[i], "type%d", i);
    SocialCategories[i].type = social_types[i];
    for (int j=0; j<4; ++j) {
      std::sprintf(social_names[i][j], "social%d_%d", i, j);
      SocialCategories[i].name[j] = social_names[i][j];
    }
  }
}
static void hide_second_tech(int id) {
  if (id == 140000) Technology[1].preq_tech_1 = -2;
}
static char changed_name[] = "changed-after-header";
static void change_first_social_name(int id) {
  if (id == 120000) SocialCategories[0].name[0] = changed_name;
}
int main() {
  Datalink link;
  reset();
  Technology[88].preq_tech_2 = 88;
  on_item = hide_second_tech;
  link.set_cat_tech();
  assert(entries.size() == 88);
  assert(entries.front().id == 140000 && entries.front().text == "tech0|140000");
  assert(entries[1].id == 140002 && entries[1].text == "tech2|140002");
  assert(entries.back().id == 140088 && entries.back().text == "tech88|140088");

  reset();
  on_item = change_first_social_name;
  link.set_cat_social();
  assert(entries.size() == 20);
  assert(entries[0].id == 120000 && entries[0].text == "type0");
  assert(entries[1].id == 120001 && entries[1].text == "changed-after-header|120001");
  assert(entries[4].id == 120004 && entries[4].text == "social0_3|120004");
  assert(entries[5].id == 120005 && entries[5].text == "type1");
  assert(entries[19].id == 120019 && entries[19].text == "social3_3|120019");
  return 0;
}
'''.replace("TECH_BODY", tech).replace("SOCIAL_BODY", social)
    cpp = tmp_path / "category_rows.cpp"
    exe = tmp_path / "category_rows"
    cpp.write_text(program)
    built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                            str(cpp), "-o", str(exe)], capture_output=True, text=True,
                           timeout=30)
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)


def test_actual_technology_and_social_cursor_walks_keep_ids_and_live_reads(tmp_path):
    tech, social = owners()
    result = run_host(tmp_path, tech, social)
    assert result.returncode == 0, result.stderr


def test_technology_filter_mutant_is_rejected(tmp_path):
    tech, social = owners()
    assert "record->preq_tech_1 >= -1" in tech
    bad_tech = tech.replace("record->preq_tech_1 >= -1", "true")
    result = run_host(tmp_path, bad_tech, social)
    assert result.returncode != 0


def test_social_five_id_stride_mutant_is_rejected(tmp_path):
    tech, social = owners()
    assert "category += 5" in social
    bad_social = social.replace("category += 5", "category += 4")
    result = run_host(tmp_path, tech, bad_social)
    assert result.returncode != 0


def test_social_cached_name_before_header_callback_is_rejected(tmp_path):
    tech, social = owners()
    anchor = "listBox_.item(StringTable.get(row->type), category - 1);"
    assert social.count(anchor) == 1
    cached = social.replace(anchor, "LPSTR cached_name = row->name[0];\n        " + anchor)
    cached = cached.replace("StringTable.get(row->name[choice])",
                            "StringTable.get(cached_name)")
    result = run_host(tmp_path, tech, cached)
    assert result.returncode != 0
