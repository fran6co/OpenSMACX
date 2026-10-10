"""Host behavior controls for the actual Datalink faction-row body."""
from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/datalink.cpp").is_file())
SOURCE = ROOT / "src/datalink.cpp"
FACTION_HEADER = ROOT / "src/faction.h"
BODY_TOKEN = "// @@DATALINK_FACTION_BODY@@"
PLAYER_TOKEN = "// @@ACTUAL_PLAYER_DECLARATION@@"


def extract_body(text, signature):
    match = re.search(signature, text)
    assert match, "actual product definition not found"
    opening = text.index("{", match.end())
    depth = 0
    state = "code"
    index = opening
    while index < len(text):
        char = text[index]
        next_char = text[index + 1] if index + 1 < len(text) else ""
        if state == "code":
            if char == "/" and next_char == "/":
                state = "line"
                index += 2
                continue
            if char == "/" and next_char == "*":
                state = "block"
                index += 2
                continue
            if char == '"':
                state = "string"
            elif char == "'":
                state = "character"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    body = text[match.start():index + 1]
                    assert "BODY GOES HERE" not in body
                    return body
        elif state == "line" and char == "\n":
            state = "code"
        elif state == "block" and char == "*" and next_char == "/":
            state = "code"
            index += 2
            continue
        elif state in ("string", "character"):
            if char == "\\":
                index += 2
                continue
            if (state == "string" and char == '"') or (
                state == "character" and char == "'"
            ):
                state = "code"
        index += 1
    raise AssertionError("unterminated actual method definition")


def actual_player_declaration(text):
    start = text.index("struct Player {")
    end = text.index("\n};", start) + len("\n};")
    return text[start:end]


FIXTURE = r'''
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>

typedef int BOOL;
// @@ACTUAL_PLAYER_DECLARATION@@
typedef char Player_size_is_59c[(sizeof(Player) == 0x59c) ? 1 : -1];
typedef char Player_gender_offset_is_2fc[(offsetof(Player, noun_gender) == 0x2fc) ? 1 : -1];
typedef char Player_plural_offset_is_300[(offsetof(Player, is_noun_plural) == 0x300) ? 1 : -1];
typedef char Player_formal_name_offset_is_384[(offsetof(Player, formal_name_faction) == 0x384) ? 1 : -1];

static const int MaxPlayerNum = 8;
Player Players[MaxPlayerNum];
char StringTemp[512];
const char *events[32];
int event_count;
void fail(const char *message);
struct PublishSlot {
    int value;
    const char *name;
    PublishSlot(const char *label) : value(0), name(label) {}
    PublishSlot &operator=(int next) {
        if (next == -77 || next == -88) { value = next; return *this; }
        if (StringTemp[0] != '\0') fail("grammar published before buffer clear");
        events[event_count++] = name;
        value = next;
        return *this;
    }
    operator int() const { return value; }
};
PublishSlot GenderDefault("gender");
PublishSlot PluralityDefault("plurality");
char DatalinkFactionIdSeparator[] = "|";
struct GameFields { unsigned int game_state_; };
struct GameState { GameFields fields_; } GameDataStateStorage;
struct RECT { int left, top, right, bottom; };
RECT category_rect_ = {10, 20, 110, 70};
int InterfaceBoxStyles[8];
int font3_;
std::vector<std::string> item_texts;
std::vector<int> item_ids;
std::vector<int> item_genders;
std::vector<int> item_pluralities;
bool mutate_next_row = true;
const char *label_key = "FACTION_LABEL_KEY";

struct LabelTable { const char *strings_ptr[728]; } Labels;
struct Strings {
    const char *get(const char *key) {
        if (key != label_key) fail("wrong category label key");
        events[event_count++] = "label-get";
        return "Faction category";
    }
} StringTable;

void fail(const char *message) {
    fprintf(stderr, "%s\n", message);
    __builtin_trap();
}
char *_itoa(int value, char *buffer, int radix) {
    if (radix != 10) fail("unexpected radix");
    sprintf(buffer, "%d", value);
    return buffer;
}
void strcat_checked(char *dest, const char *src) { strcat(dest, src); }

struct ListBox {
    void item(const char *text, int id) {
        events[event_count++] = "item";
        item_texts.push_back(text);
        item_ids.push_back(id);
        item_genders.push_back(GenderDefault);
        item_pluralities.push_back(PluralityDefault);
        // This callback mutates the next Player row after the previous row's
        // callback. The following iteration must reload its live fields.
        if (mutate_next_row && id == 150001) {
            strcpy(Players[2].formal_name_faction, "Changed second faction");
            Players[2].noun_gender = 44;
            Players[2].is_noun_plural = 7;
            GenderDefault = -77;
            PluralityDefault = -88;
        }
    }
} listBox_;

struct Datalink {
    ListBox &listBox_;
    RECT &category_rect_;
    int &font3_;
    Datalink(ListBox &list, RECT &rect, int &font)
        : listBox_(list), category_rect_(rect), font3_(font) {}
    void box_sprite(RECT *rect, int *style) {
        if (style != &InterfaceBoxStyles[6]) fail("wrong box style");
        if (rect->left != 10 || rect->right != 110 ||
            rect->top != 20 || rect->bottom != 70) fail("wrong frame rect");
        events[event_count++] = "box";
    }
    void set_text_color(int a, int b, int c, int d) {
        if (a != 0xe0 || b != -1 || c != 1 || d != 1) fail("wrong text color");
        events[event_count++] = "color";
    }
    void set_font(int *font, int a, int b, int c) {
        if (font != &font3_ || a != 0 || b != 0 || c != 0) fail("wrong font");
        events[event_count++] = "font";
    }
    void write_cent_l(char *text, RECT *rect, int length) {
        if (strcmp(text, "Faction category") != 0 ||
            length != (int)strlen(text)) fail("wrong category text");
        if (rect->left != 13 || rect->right != 107 ||
            rect->top != 23 || rect->bottom != 67) fail("wrong label rect");
        events[event_count++] = "write";
    }
    void soft_update(RECT *rect) {
        if (rect != &category_rect_) fail("wrong update rect");
        events[event_count++] = "update";
    }
    void set_cat_faction();
};

#define CATEGORY_LABEL(slot) StringTable.get(Labels.strings_ptr[(slot)])
// @@DATALINK_FACTION_BODY@@

void reset_case(bool suffix) {
    event_count = 0;
    StringTemp[0] = '\0';
    item_texts.clear(); item_ids.clear();
    item_genders.clear(); item_pluralities.clear();
    mutate_next_row = true;
    category_rect_ = RECT{10, 20, 110, 70};
    GenderDefault.value = 101; PluralityDefault.value = 102;
    GameDataStateStorage.fields_.game_state_ = suffix ? 0x1000 : 0;
    Labels.strings_ptr[727] = label_key;
    for (int faction = 1; faction <= 7; ++faction) {
        Player &p = Players[faction];
        p.noun_gender = 10 + faction;
        p.is_noun_plural = 20 + faction;
        sprintf(p.formal_name_faction, "Faction %d", faction);
    }
}

void check_case(bool suffix) {
    if (item_ids.size() != 7) fail("wrong number of faction rows");
    for (int index = 0; index < 7; ++index) {
        const int faction = index + 1;
        if (item_ids[index] != 150001 + index) fail("wrong faction id order");
        if (item_genders[index] != (index == 1 ? 44 : 10 + faction))
            fail("gender publication/order or callback reread wrong");
        if (item_pluralities[index] != (index == 1 ? 7 : 20 + faction))
            fail("plurality publication/order or callback reread wrong");
        const char *name = index == 1 ? "Changed second faction" : 0;
        char expected[128];
        if (name == 0) sprintf(expected, "Faction %d", faction);
        else strcpy(expected, name);
        if (suffix) {
            char digits[32];
            _itoa(150001 + index, digits, 10);
            strcat(expected, "|"); strcat(expected, digits);
        }
        if (item_texts[index] != expected) fail("wrong name/suffix read");
    }
    if (event_count != 27) fail("wrong grammar/category/render publication count");
    const char *tail[] = {"box", "color", "font", "label-get", "write", "update"};
    for (int i = 0; i < 6; ++i)
        if (events[21 + i] != tail[i]) fail("wrong render/publication order");
    for (int index = 0; index < 7; ++index) {
        if (strcmp(events[index * 3], "gender") != 0 ||
            strcmp(events[index * 3 + 1], "plurality") != 0 ||
            strcmp(events[index * 3 + 2], "item") != 0)
            fail("wrong grammar publication order");
    }
}

int main() {
    Datalink link(listBox_, category_rect_, font3_);
    reset_case(true);
    link.set_cat_faction();
    check_case(true);
    reset_case(false);
    link.set_cat_faction();
    check_case(false);
    return 0;
}
'''


def replace_once(text, old, new):
    assert text.count(old) == 1, "expected one mutant anchor: " + old
    return text.replace(old, new, 1)


def compile_and_run(tmp_path, compiler, body, player_decl, should_pass, name):
    generated = FIXTURE.replace(PLAYER_TOKEN, player_decl).replace(BODY_TOKEN, body)
    assert PLAYER_TOKEN not in generated and BODY_TOKEN not in generated
    source = tmp_path / (name + ".cpp")
    binary = tmp_path / name
    source.write_text(generated)
    build = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", "-pedantic",
         str(source), "-o", str(binary)],
        capture_output=True, text=True, timeout=60,
    )
    assert build.returncode == 0, build.stdout + build.stderr
    run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=20)
    if should_pass:
        assert run.returncode == 0, run.stdout + run.stderr
    else:
        assert run.returncode != 0, name + " damage escaped the host control"


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_set_cat_faction_and_mutants(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " host compiler unavailable")
    source = SOURCE.read_text()
    header = FACTION_HEADER.read_text()
    body = extract_body(source, r"void\s+Datalink::set_cat_faction\s*\(\s*\)")
    ordered_accesses = (
        "const int gender = player.noun_gender;",
        "const BOOL plural = player.is_noun_plural;",
        "StringTemp[0] = '\\0';",
        "GenderDefault = gender;",
        "PluralityDefault = plural;",
    )
    positions = [body.index(fragment) for fragment in ordered_accesses]
    assert positions == sorted(positions), "pinned grammar load/clear/publish order changed"
    assert "index < 7" in body, "pinned faction loop must emit exactly seven rows"
    player_decl = actual_player_declaration(header)
    compile_and_run(tmp_path, compiler, body, player_decl, True, "actual")

    mutants = {
        "wrong_stride": replace_once(
            body, "Players[index + 1]", "Players[index + 1 + index]"),
        "wrong_member": replace_once(
            body, "player.formal_name_faction", "player.adj_name_faction"),
        "publish_order": replace_once(
            body,
            "GenderDefault = gender;\n        PluralityDefault = plural;",
            "PluralityDefault = plural;\n        GenderDefault = gender;"),
        "extra_eighth_row": replace_once(body, "index < 7", "index < 8"),
        "publish_before_clear": replace_once(
            body, "StringTemp[0] = '\\0';\n        GenderDefault = gender;",
            "GenderDefault = gender;\n        StringTemp[0] = '\\0';"),
        "wrong_id": replace_once(
            body, "listBox_.item(StringTemp, index + 150001)",
            "listBox_.item(StringTemp, index + 150000)"),
    }
    for name, mutant in mutants.items():
        compile_and_run(tmp_path, compiler, mutant, player_decl, False, name)

