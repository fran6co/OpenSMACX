"""Bounded actual-source controls for Datalink rules-table ownership."""

from __future__ import annotations

import shutil
import re
import subprocess
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[2]


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
    raise AssertionError(f"unterminated body: {signature}")


def test_datalink_rules_consumers_use_typed_records(tmp_path: Path) -> None:
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")

    source = (ROOT / "src/datalink.cpp").read_text()
    bodies = "\n".join(
        _body(source, signature)
        for signature in (
            "void Datalink::set_cat_tech()",
            "void Datalink::set_cat_social()",
            "void Datalink::set_cat_effect()",
            "void Datalink::set_cat_terraform()",
        )
    )
    for retired in (
        "TechRowBase", "SocialCategoryRow", "SocialEffectRowBase",
        "SocialEffectNullRow", "SocialEffectRowEnd", "TerraformFlagBase",
        "TerraformNameBase",
    ):
        assert retired not in bodies
        assert not re.search(rf"static const [^;]*\b{retired}\b", source)

    harness = r'''
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
typedef unsigned int uint32_t;
typedef char *LPSTR;
struct RECT { int left, top, right, bottom; };
struct BoxSpriteParams {};
static BoxSpriteParams InterfaceBoxStyles[32];
static char StringTemp[1024];
static const char DatalinkTechIdSeparator[] = "|";
static const char DatalinkSocialIdSeparator[] = "|";
static const char DatalinkEffectIdSeparator[] = "|";
static const char DatalinkTerraformIdSeparator[] = "|";
static const char DatalinkTerraformSecondSeparator[] = "|";
struct GameFields { uint32_t game_state_; };
struct { GameFields fields_; } GameDataStateStorage = {{0x1000u}};

struct RulesTechnology {
    uint32_t flags; LPSTR name; char id[8]; int padding;
    int growth_value, tech_value, wealth_value, power_value;
    int preq_tech_1, preq_tech_2;
};
const int MaxTechnologyNum = 89;
RulesTechnology Technology[MaxTechnologyNum] = {};
struct SocialEffect { int values_[11]; };
struct RulesSocialEffect { char set1[24], set2[24], padding[56]; };
const int MaxSocialEffectNum = 11;
struct RulesSocialCategory {
    LPSTR type; int preq_tech[4]; LPSTR name[4]; SocialEffect model_effect[4];
};
const int MaxSocialCatNum = 4;
RulesSocialCategory SocialCategories[MaxSocialCatNum] = {};
RulesSocialEffect SocialEffects[MaxSocialEffectNum] = {};
struct RulesTerraforming {
    LPSTR name, name_sea; int preq_tech, preq_tech_sea;
    uint32_t bit, bit_incompatible; int rate; LPSTR shortcuts;
};
const int MaxTerrainNum = 20;
RulesTerraforming Terraforming[MaxTerrainNum] = {};
struct RulesOrder { LPSTR order, order_sea, letter; };
RulesOrder Order[30] = {};
struct StringTableType {
    char *get(char *name) {
        assert(name != 0);
        return name;
    }
} StringTable;
static char *label(int id) {
    static char text[32]; std::sprintf(text, "label%d", id); return text;
}
#define CATEGORY_LABEL(slot) label(slot)
static char * _itoa(int value, char *out, int radix) {
    assert(radix == 10); std::sprintf(out, "%d", value); return out;
}
struct Item { std::string text; int id; };
struct ListBox {
    std::vector<Item> items;
    void item(const char *text, int id) { items.push_back(Item{text, id}); }
};
struct Datalink {
    ListBox listBox_;
    RECT category_rect_ = {0, 0, 100, 80};
    int font3_ = 0;
    void box_sprite(RECT *, BoxSpriteParams *) {}
    void set_text_color(int, int, int, int) {}
    void set_font(void *, void *, void *, int) {}
    int write_cent_l(char *, RECT *, int) { return 0; }
    void soft_update(RECT *) {}
    void set_cat_tech(); void set_cat_social(); void set_cat_effect();
    void set_cat_terraform();
};
''' + bodies + r'''

static void clear(Datalink &d) { d.listBox_.items.clear(); }
int main() {
    Datalink d;
    char tech_names[MaxTechnologyNum][20] = {};
    char social_types[MaxSocialCatNum][20] = {};
    char social_names[MaxSocialCatNum][4][32] = {};
    char land_names[MaxTerrainNum][20] = {};
    char sea_names[MaxTerrainNum][20] = {};

    // Only the real prerequisite pair admits a technology. The first record's
    // negative power field must not be mistaken for either gate.
    for (int i = 0; i < MaxTechnologyNum; ++i) {
        Technology[i].preq_tech_1 = -2;
        Technology[i].preq_tech_2 = -2;
        Technology[i].power_value = -9;
        std::sprintf(tech_names[i], "tech%d", i);
        Technology[i].name = tech_names[i];
    }
    Technology[0].preq_tech_1 = -1;
    Technology[0].preq_tech_2 = -1;
    Technology[1].preq_tech_1 = 2;
    Technology[1].preq_tech_2 = 3;
    d.set_cat_tech();
    assert(d.listBox_.items.size() == 2);
    assert(d.listBox_.items[0].id == 140000 && d.listBox_.items[0].text == "tech0|140000");
    assert(d.listBox_.items[1].id == 140001 && d.listBox_.items[1].text == "tech1|140001");

    clear(d);
    for (int category = 0; category < MaxSocialCatNum; ++category) {
        std::sprintf(social_types[category], "category%d", category);
        SocialCategories[category].type = social_types[category];
        for (int choice = 0; choice < 4; ++choice) {
            std::sprintf(social_names[category][choice], "choice%d.%d", category, choice);
            SocialCategories[category].name[choice] = social_names[category][choice];
        }
    }
    d.set_cat_social();
    assert(d.listBox_.items.size() == 20);
    for (int category = 0; category < 4; ++category) {
        const int first = category * 5;
        assert(d.listBox_.items[first].id == 120000 + category * 5);
        assert(d.listBox_.items[first].text == "category" + std::to_string(category));
        for (int choice = 0; choice < 4; ++choice) {
            const Item &item = d.listBox_.items[first + choice + 1];
            const int id = 120001 + category * 5 + choice;
            assert(item.id == id);
            assert(item.text == "choice" + std::to_string(category) + "." + std::to_string(choice) + "|" + std::to_string(id));
        }
    }

    clear(d);
    for (int i = 0; i < MaxSocialEffectNum; ++i)
        std::sprintf(SocialEffects[i].set2, "effect%d", i);
    d.set_cat_effect();
    assert(d.listBox_.items.size() == 10);
    for (int i = 0, out = 0; i < MaxSocialEffectNum; ++i) {
        if (i == 3) continue;
        assert(d.listBox_.items[out].id == 130000 + i);
        assert(d.listBox_.items[out].text == "effect" + std::to_string(i) + "|" + std::to_string(130000 + i));
        ++out;
    }

    clear(d);
    for (int i = 0; i < MaxTerrainNum; ++i) {
        Terraforming[i].preq_tech = -2;
        Terraforming[i].preq_tech_sea = -2;
        std::sprintf(land_names[i], "land%d", i);
        std::sprintf(sea_names[i], "sea%d", i);
        Order[i + 4].order = land_names[i];
        Order[i + 4].order_sea = sea_names[i];
    }
    Terraforming[0].preq_tech = -2;
    Terraforming[0].preq_tech_sea = -1;
    Terraforming[1].preq_tech = -1;
    Terraforming[1].preq_tech_sea = -2;
    Terraforming[16].preq_tech = -1;
    Terraforming[16].preq_tech_sea = -1;
    Terraforming[17].preq_tech = -2;
    Terraforming[17].preq_tech_sea = -1;
    d.set_cat_terraform();
    assert(d.listBox_.items.size() == 3);
    assert(d.listBox_.items[0].id == 90020 && d.listBox_.items[0].text == "sea0|90020");
    assert(d.listBox_.items[1].id == 90001 && d.listBox_.items[1].text == "land1|90001");
    assert(d.listBox_.items[2].id == 90016 && d.listBox_.items[2].text == "land16|90016");
    return 0;
}
'''
    generated = tmp_path / "datalink_rules_table_owners.cpp"
    executable = tmp_path / "datalink_rules_table_owners"
    generated.write_text(harness)
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", str(generated), "-o", str(executable)],
        capture_output=True, text=True,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(executable)], capture_output=True, text=True)
    assert ran.returncode == 0, ran.stdout + ran.stderr
