"""Exercise the pinned zero-name path in Datalink's terraforming filler."""

from __future__ import annotations

import shutil
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


HARNESS = r'''
#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
typedef unsigned int uint32_t;
typedef char *LPSTR;
struct RECT { int left, top, right, bottom; };
struct BoxSpriteParams {};
static BoxSpriteParams InterfaceBoxStyles[8];
static char StringTemp[256];
static const char DatalinkTerraformIdSeparator[] = "|";
static const char DatalinkTerraformSecondSeparator[] = "|";
struct GameFields { uint32_t game_state_; };
static struct { GameFields fields_; } GameDataStateStorage = {{0}};

struct RulesTerraforming {
    LPSTR name, name_sea;
    int preq_tech, preq_tech_sea;
    uint32_t bit, bit_incompatible;
    int rate;
    LPSTR shortcuts;
};
struct RulesOrder { LPSTR order, order_sea, letter; };
static RulesTerraforming Terraforming[20] = {};
static RulesOrder Order[30] = {};
static int null_name_lookups = 0;
struct StringTableType {
    char *get(char *name) {
        if (name == 0) {
            ++null_name_lookups;
            return const_cast<char *>("null-order");
        }
        return name;
    }
} StringTable;
static char *_itoa(int value, char *out, int radix) {
    assert(radix == 10);
    std::sprintf(out, "%d", value);
    return out;
}
static const char *category_label(int) { return "Terraforming"; }
#define CATEGORY_LABEL(slot) category_label(slot)
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
    void set_font(void *, int, int, int) {}
    int write_cent_l(char *, RECT *, unsigned int) { return 0; }
    void soft_update(RECT *) {}
    void set_cat_terraform();
};
''' + "\n" + "__DATALINK_BODY__" + r'''

int main() {
    Datalink datalink;
    for (int i = 0; i < 20; ++i) {
        Terraforming[i].preq_tech = -2;
        Terraforming[i].preq_tech_sea = -2;
    }
    // The image gates this land entry only on its prerequisite word. It still
    // resolves and inserts the zero order-name pointer through Strings::get.
    Terraforming[0].preq_tech = -1;
    Order[4].order = 0;
    datalink.set_cat_terraform();
    assert(null_name_lookups == 1);
    assert(datalink.listBox_.items.size() == 1);
    assert(datalink.listBox_.items[0].id == 90000);
    assert(datalink.listBox_.items[0].text == "null-order");
    return 0;
}
'''


def _compile_and_run(compiler: str, source: str, output: Path) -> subprocess.CompletedProcess[str]:
    generated = output.with_suffix(".cpp")
    generated.write_text(source)
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", str(generated), "-o", str(output)],
        capture_output=True,
        text=True,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(output)], capture_output=True, text=True)


def test_terraforming_zero_order_name_still_resolves_and_inserts(tmp_path: Path) -> None:
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")

    source = (ROOT / "src/datalink.cpp").read_text()
    body = _body(source, "void Datalink::set_cat_terraform()")
    assert "if (terraform.preq_tech >= -1) {" in body
    assert "order.order != 0" not in body
    actual = _compile_and_run(
        compiler,
        HARNESS.replace("__DATALINK_BODY__", body),
        tmp_path / "terraforming_zero_order",
    )
    assert actual.returncode == 0, actual.stdout + actual.stderr

    # A mutant reinstating the former safety guard must be caught by the
    # executed zero-name case, not merely by a source-text assertion.
    mutant_body = body.replace(
        "if (terraform.preq_tech >= -1) {",
        "if (terraform.preq_tech >= -1 && order.order != 0) {",
        1,
    )
    assert mutant_body != body
    mutant = _compile_and_run(
        compiler,
        HARNESS.replace("__DATALINK_BODY__", mutant_body),
        tmp_path / "terraforming_zero_order_mutant",
    )
    assert mutant.returncode != 0
