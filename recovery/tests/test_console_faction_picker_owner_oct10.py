"""Actual-body controls for Console's faction picker and editor dialogs."""

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

from match_recovery.includes import COMMENTS_OR_STRINGS


ROOT = next(path for path in Path(__file__).resolve().parents
            if (path / "src/IMAGE_PIN").is_file())
CONSOLE = ROOT / "src/console.cpp"


def _mask(source: str) -> str:
    return COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " "
                              for char in match[0]), source)


def _body(source: str, signature: str) -> str:
    masked = _mask(source)
    start = masked.index(signature)
    opening = masked.index("{", start)
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{":
            depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError("unbalanced function body: " + signature)


def _fragments() -> dict[str, str]:
    source = CONSOLE.read_text(encoding="utf-8")
    return {
        "reload": _body(source, "void Console::editor_reload()"),
        "eliminate": _body(source, "void Console::editor_eliminate()"),
        "picker": _body(source, "void Console::custom_my_faction()"),
    }


HARNESS = r'''#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum { MaxPlayerNum = 8 };
typedef int BOOL;
typedef char *LPSTR;

struct Player {
    BOOL is_leader_female;
    char filename[24];
    char search_key[24];
    char name_leader[24];
    char title_leader[24];
    char adj_leader[128];
    char adj_insult_leader[128];
    char adj_faction[128];
    char adj_insult_faction[128];
    uint8_t padding1[128];
    char noun_faction[24];
    int noun_gender;
    BOOL is_noun_plural;
    char adj_name_faction[128];
    char formal_name_faction[40];
    char insult_leader[24];
    char desc_name_faction[24];
    char assistant_name[24];
    char scientist_name[24];
    char assistant_city[24];
    uint8_t padding2[176];
    int rule_tech_selected;
    int rule_morale;
    int rule_research;
    int rule_drone;
    int rule_talent;
    int rule_energy;
    int rule_interest;
    int rule_population;
    int rule_hurry;
    int rule_techcost;
    int rule_psi;
    int rule_sharetech;
    int rule_commerce;
    int rule_flags;
    int faction_bonus_count;
    int faction_bonus_id[8];
    int faction_bonus_val1[8];
    int faction_bonus_val2[8];
    int ai_fight;
    BOOL ai_growth;
    BOOL ai_tech;
    BOOL ai_wealth;
    BOOL ai_power;
    int soc_ideology_category;
    int soc_anti_ideology_category;
    int soc_ideology_model;
    int soc_anti_ideology_model;
    int soc_ideology_effect;
    int soc_anti_ideology_effect;
};
static_assert(offsetof(Player, noun_faction) == 0x2E4, "noun field layout");
static_assert(offsetof(Player, noun_gender) == 0x2FC, "gender field layout");
static_assert(offsetof(Player, is_noun_plural) == 0x300, "plural field layout");
static_assert(sizeof(Player) == 0x59C, "Player stride");
Player Players[MaxPlayerNum];

struct Console { int faction_id_; } ConsoleGlobal;
struct GameDataLockMaskBytes {
    uint8_t human_mask_;
    uint8_t alive_mask_;
    uint8_t unassigned_[2];
};
union GameDataLockMaskView {
    uint32_t enable_mask_;
    GameDataLockMaskBytes bytes_;
};
static_assert(sizeof(GameDataLockMaskBytes) == 4, "lock-mask byte view");
struct GameFields {
    int game_state_;
    GameDataLockMaskView lock_masks_;
    int base_current_count_;
} fields;
struct GameDataState { GameFields fields_; } GameDataStateStorage;
struct Base { int faction_id_current; } Bases[64];
struct ParseBuffer { char str[4096]; } ParseStrBuffer[8];
struct LabelPool { LPSTR *strings_ptr; } Labels;
struct StringTableType {
    int calls;
    LPSTR get(LPSTR value) { ++calls; return value; }
} StringTable;
struct NetType {
    int calls, op, faction, a3, a4, a6, a7;
    void synch(int operation, int faction_id, int third, int fourth,
               void *, int sixth, int seventh) {
        ++calls; op = operation; faction = faction_id;
        a3 = third; a4 = fourth; a6 = sixth; a7 = seventh;
    }
} NetDaemonNet;

int PopupModalState;
int PluralityDefault;
int GenderDefault;
char PopupStartCaption[] = "CAPTION";
char ConsoleStrEditorWhoListTitle[] = "EDITORWHO";
char ConsoleStrCustomNames[] = "CUSTOMNAMES";
int XPopAnswer;
int XPopCalls;
int ReadFactionCalls;
int ComputeModifierCalls;
int AutoUndoCalls;
int BaseKillCalls;
int DrawMapCalls;
int EliminateCalls;
int LastKilledBase;
int LastEliminatedFaction;
int LastEliminateMode;
int MapDrawArgument;
int EditorQuestionFaction;

int X_pop_ask(const char *, int, char *, char *, int) {
    ++XPopCalls;
    return XPopAnswer;
}
void read_faction(int) { ++ReadFactionCalls; }
void compute_faction_modifiers(int) { ++ComputeModifierCalls; }
void auto_undo() { ++AutoUndoCalls; }
void base_kill(int base) { ++BaseKillCalls; LastKilledBase = base; }
void draw_map(int value) { ++DrawMapCalls; MapDrawArgument = value; }
void eliminate_player(int faction, int mode) {
    ++EliminateCalls; LastEliminatedFaction = faction; LastEliminateMode = mode;
}

int ctor_mutation = -1;
int start_mutation = -1;
int item_mutation = -1;
int picker_result = 2;
int edit_result = 0;
int generic_result = -1;
int ctor_count;
int destructor_count;
int close_count;
int start_count;
int start_modal[8];
int start_value[8];
int exec_count;
int set_value_count;
int set_value_seen;
int expected_saved_selection;
int expect_saved_selection;
int item_count;
char *item_text[8];
int item_id[8];
int item_plurality[8];
int item_gender[8];
int item_modal[8];
int edit_count;
char *edit_text[8];
int edit_limit[8];

struct Dialogs {
    class Popup *owner;
    explicit Dialogs(Popup *popup) : owner(popup) {}
    void item(char *text, int id);
    void edit(char *label, char *text, int limit);
};

class Popup {
public:
    int id;
    Dialogs dialog;
    Popup() : id(++ctor_count), dialog(this) {
        if (id == 2 && ctor_mutation >= 0)
            ConsoleGlobal.faction_id_ = ctor_mutation;
    }
    ~Popup() { ++destructor_count; }
    Dialogs &dialogs() { return dialog; }
    void close() { ++close_count; }
    void set_value(int value) {
        assert(item_count == 7);
        assert(PopupModalState == 6);
        assert(expect_saved_selection);
        assert(value == expected_saved_selection);
        ++set_value_count;
        set_value_seen = value;
    }
    void start(char *, const char *, int value, char *, int) {
        start_common(value);
    }
    void start(char *, const char *, int value, char *, int, void *) {
        start_common(value);
    }
    void start_common(int value) {
        ++start_count;
        if (id < 8) {
            start_value[id] = value;
            start_modal[id] = PopupModalState;
        }
        if (id == 2 && start_mutation >= 0)
            ConsoleGlobal.faction_id_ = start_mutation;
    }
    int exec(int, void *) {
        ++exec_count;
        if (id == 2) {
            assert(item_count == 7);
            if (expect_saved_selection) {
                assert(set_value_count == 1);
                assert(set_value_seen == expected_saved_selection);
            } else {
                assert(set_value_count == 0);
            }
            return picker_result;
        }
        if (edit_count == 7 && edit_result >= 0) {
            strcpy(edit_text[0], "LeaderX");
            strcpy(edit_text[1], "FactionX");
            strcpy(edit_text[2], "NounX");
            strcpy(edit_text[3], "AdjX");
            strcpy(edit_text[4], "TitleX");
            strcpy(edit_text[5], "LeaderAdjX");
            strcpy(edit_text[6], "FemaleWord");
        }
        if (edit_count == 7) return edit_result;
        return generic_result;
    }
};

void Dialogs::item(char *text, int id) {
    if (item_count < 8) {
        item_text[item_count] = text;
        item_id[item_count] = id;
        item_plurality[item_count] = PluralityDefault;
        item_gender[item_count] = GenderDefault;
        item_modal[item_count] = PopupModalState;
    }
    ++item_count;
    if (owner->id == 2 && item_mutation >= 0)
        ConsoleGlobal.faction_id_ = item_mutation;
}
void Dialogs::edit(char *label, char *text, int limit) {
    if (edit_count < 8) {
        edit_text[edit_count] = text;
        edit_limit[edit_count] = limit;
    }
    ++edit_count;
}

int _strnicmp(const char *left, const char *right, size_t count) {
    for (size_t index = 0; index < count; ++index) {
        const unsigned char a = static_cast<unsigned char>(left[index]);
        const unsigned char b = static_cast<unsigned char>(right[index]);
        const int lower_a = (a >= 'A' && a <= 'Z') ? a + ('a' - 'A') : a;
        const int lower_b = (b >= 'A' && b <= 'Z') ? b + ('a' - 'A') : b;
        if (lower_a != lower_b || !a || !b) return lower_a - lower_b;
    }
    return 0;
}

__RELOAD__
__ELIMINATE__
__PICKER__

static void reset_common(int state, int faction) {
    memset(&GameDataStateStorage, 0, sizeof(GameDataStateStorage));
    GameDataStateStorage.fields_.game_state_ = state;
    GameDataStateStorage.fields_.base_current_count_ = 3;
    memset(Bases, 0, sizeof(Bases));
    Bases[0].faction_id_current = 2;
    Bases[1].faction_id_current = 3;
    Bases[2].faction_id_current = 3;
    ConsoleGlobal.faction_id_ = faction;
    PopupModalState = 0;
    PluralityDefault = GenderDefault = -99;
    ctor_mutation = start_mutation = item_mutation = -1;
    picker_result = 2;
    edit_result = generic_result = 0;
    ctor_count = destructor_count = close_count = start_count = 0;
    exec_count = set_value_count = set_value_seen = item_count = 0;
    expected_saved_selection = faction;
    expect_saved_selection = (state & 0xC0) == 0xC0 && faction >= 0;
    edit_count = XPopCalls = ReadFactionCalls = ComputeModifierCalls = 0;
    AutoUndoCalls = BaseKillCalls = DrawMapCalls = EliminateCalls = 0;
    NetDaemonNet.calls = StringTable.calls = 0;
    memset(start_modal, 0, sizeof(start_modal));
    memset(start_value, 0, sizeof(start_value));
    memset(item_text, 0, sizeof(item_text));
    memset(item_id, 0, sizeof(item_id));
    memset(item_plurality, 0, sizeof(item_plurality));
    memset(item_gender, 0, sizeof(item_gender));
    memset(item_modal, 0, sizeof(item_modal));
    memset(edit_text, 0, sizeof(edit_text));
    memset(edit_limit, 0, sizeof(edit_limit));
    memset(ParseStrBuffer, 0, sizeof(ParseStrBuffer));
    memset(Players, 0, sizeof(Players));
    for (int faction_id = 0; faction_id < MaxPlayerNum; ++faction_id) {
        sprintf(Players[faction_id].noun_faction, "Noun%d", faction_id);
        Players[faction_id].noun_gender = 10 + faction_id;
        Players[faction_id].is_noun_plural = 20 + faction_id;
        strcpy(Players[faction_id].filename, "file.txt");
        strcpy(Players[faction_id].search_key, "search.txt");
        strcpy(Players[faction_id].name_leader, "Leader");
        strcpy(Players[faction_id].formal_name_faction, "Faction");
        strcpy(Players[faction_id].adj_name_faction, "Adj");
        strcpy(Players[faction_id].title_leader, "Title");
        strcpy(Players[faction_id].adj_leader, "LeaderAdj");
    }
    Players[2].is_leader_female = 0;
}

static void assert_who_rows() {
    assert(item_count == 7);
    for (int index = 0; index != 7; ++index) {
        const int faction_id = index + 1;
        assert(item_id[index] == faction_id);
        assert(item_text[index] == Players[faction_id].noun_faction);
        assert(item_plurality[index] == Players[faction_id].is_noun_plural);
        assert(item_gender[index] == Players[faction_id].noun_gender);
        assert(item_modal[index] == 6);
    }
}

static void test_saved_selection_and_full_edit() {
    reset_common(0xC0, 3);
    expected_saved_selection = 3;
    expect_saved_selection = 1;
    ctor_mutation = 4;
    start_mutation = 5;
    item_mutation = 6;
    custom_my_faction();
    assert(ctor_count == 2 && destructor_count == 2);
    assert(start_count == 2 && start_value[2] == -1 && start_modal[2] == 6);
    assert_who_rows();
    assert(set_value_count == 1 && set_value_seen == 3);
    assert(exec_count == 2 && PopupModalState == 0);
    assert(edit_count == 7);
    assert(strcmp(Players[2].name_leader, "LeaderX") == 0);
    assert(strcmp(Players[2].formal_name_faction, "FactionX") == 0);
    assert(strcmp(Players[2].noun_faction, "NounX") == 0);
    assert(strcmp(Players[2].adj_name_faction, "AdjX") == 0);
    assert(strcmp(Players[2].title_leader, "TitleX") == 0);
    assert(strcmp(Players[2].adj_leader, "LeaderAdjX") == 0);
    assert(strcmp(ParseStrBuffer[0].str, "LeaderX") == 0);
    assert(strcmp(ParseStrBuffer[5].str, "LeaderAdjX") == 0);
    assert(Players[2].is_leader_female == 1);
    assert(NetDaemonNet.calls == 1 && NetDaemonNet.op == 7);
    assert(NetDaemonNet.faction == 2 && NetDaemonNet.a3 == 0 &&
           NetDaemonNet.a4 == 0 && NetDaemonNet.a6 == 1 &&
           NetDaemonNet.a7 == 0x2101);
    assert(close_count == 1);
}

static void test_negative_cached_faction_and_picker_cancel() {
    reset_common(0xC0, -3);
    custom_my_faction();
    assert(ctor_count == 2 && destructor_count == 2);
    assert(start_value[2] == -1 && start_modal[2] == 6);
    assert_who_rows();
    assert(set_value_count == 0);
    assert(edit_count == 7 && NetDaemonNet.calls == 1);

    reset_common(0xC0, 3);
    expected_saved_selection = 3;
    expect_saved_selection = 1;
    picker_result = -1;
    custom_my_faction();
    assert(ctor_count == 2 && destructor_count == 2);
    assert(close_count == 1);
    assert(start_count == 1 && exec_count == 1);
    assert(item_count == 7 && edit_count == 0 && NetDaemonNet.calls == 0);
    assert(PopupModalState == 0);
}

static void test_edit_cancel_and_non_editor_path() {
    reset_common(0xC0, 3);
    edit_result = -1;
    custom_my_faction();
    assert(ctor_count == 2 && destructor_count == 2);
    assert(close_count == 1 && edit_count == 7);
    assert(NetDaemonNet.calls == 0);
    assert(strcmp(Players[2].name_leader, "Leader") == 0);

    reset_common(0x40, 2);
    custom_my_faction();
    assert(ctor_count == 1 && destructor_count == 1);
    assert(item_count == 0 && set_value_count == 0);
    assert(edit_count == 7 && NetDaemonNet.calls == 1);
    assert(NetDaemonNet.faction == 2);
}

static void test_reload_and_eliminate_picker_rows_and_paths() {
    reset_common(0, 0);
    generic_result = -1;
    editor_reload();
    assert(ctor_count == 1 && destructor_count == 1);
    assert(item_count == 7 && start_modal[1] == 6);
    assert_who_rows();
    assert(XPopCalls == 0 && PopupModalState == 0);

    reset_common(0, 0);
    generic_result = 3;
    XPopAnswer = 0;
    strcpy(ParseStrBuffer[0].str, "replacement.txt");
    editor_reload();
    assert(strcmp(Players[3].filename, "replacement.txt") == 0);
    assert(strcmp(Players[3].search_key, "replacement.txt") == 0);
    assert(ReadFactionCalls == 1 && ComputeModifierCalls == 1);

    reset_common(0, 0);
    generic_result = 3;
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0xFFFFFFFFu;
    editor_eliminate();
    assert(AutoUndoCalls == 1 && item_count == 7);
    assert_who_rows();
    assert(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ == 0xF7);
    assert(GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ == 0xF7);
    assert(GameDataStateStorage.fields_.lock_masks_.bytes_.unassigned_[0] == 0xFF);
    assert(GameDataStateStorage.fields_.lock_masks_.bytes_.unassigned_[1] == 0xFF);
    assert(BaseKillCalls == 2 && LastKilledBase == 1);
    assert(DrawMapCalls == 1 && MapDrawArgument == 1);
    assert(EliminateCalls == 1 && LastEliminatedFaction == 3 &&
           LastEliminateMode == -2);
}

int main() {
    char *label_slots[0x300] = {};
    char neutral[] = "LeaderWord";
    char female[] = "FemaleWord";
    char field_caption[] = "Field";
    for (int index = 0; index != 0x300; ++index) label_slots[index] = field_caption;
    label_slots[0x2E2] = neutral;
    label_slots[0x2E3] = female;
    Labels.strings_ptr = label_slots;
    test_saved_selection_and_full_edit();
    test_negative_cached_faction_and_picker_cancel();
    test_edit_cancel_and_non_editor_path();
    test_reload_and_eliminate_picker_rows_and_paths();
    return 0;
}
'''


def _compile_and_run(fragments: dict[str, str]) -> subprocess.CompletedProcess[str]:
    compiler = shutil.which("c++") or shutil.which("g++") or shutil.which("clang++")
    assert compiler, "host C++ compiler is required for the actual-body control"
    source = HARNESS
    for key, body in fragments.items():
        for method in ("editor_reload", "editor_eliminate", "custom_my_faction"):
            body = body.replace("void Console::" + method + "()",
                                "void " + method + "()", 1)
        source = source.replace("__" + key.upper() + "__", body)
    import tempfile
    with tempfile.TemporaryDirectory(prefix="console-faction-picker-") as temp:
        unit = Path(temp) / "console_faction_picker.cpp"
        binary = Path(temp) / "console_faction_picker"
        unit.write_text(source, encoding="utf-8")
        built = subprocess.run(
            [compiler, "-std=c++11", "-Wall", "-Wextra", str(unit), "-o", str(binary)],
            capture_output=True, text=True)
        assert built.returncode == 0, built.stdout + built.stderr
        return subprocess.run([str(binary)], capture_output=True, text=True)


def test_typed_picker_and_actual_editor_bodies_reject_mutants() -> None:
    fragments = _fragments()
    picker = fragments["picker"]
    assert "int *entry =" not in picker
    assert "reinterpret_cast<char *>" not in picker
    assert "const int saved_faction = ConsoleGlobal.faction_id_;" in picker
    assert picker.index("const int saved_faction") < picker.index("Popup popup2;")
    assert picker.index("PopupModalState = 6;") < picker.index("popup2.start(")
    assert "popup2.start(PopupStartCaption, ConsoleStrEditorWhoListTitle,\n                         -1" in picker
    assert picker.index("for (int idx = 1;") < picker.index("popup2.set_value(saved_faction)")
    assert picker.index("popup2.set_value(saved_faction)") < picker.index("popup2.exec(")
    for body in (fragments["reload"], fragments["eliminate"], picker):
        assert "Player &row = Players[" in body
        assert body.index("const int plurality = row.is_noun_plural;") < body.index(
            "const int gender = row.noun_gender;")
        assert body.index("PluralityDefault = plurality;") < body.index("GenderDefault = gender;")
    eliminate = fragments["eliminate"]
    assert "reinterpret_cast<uint8_t *>" not in eliminate
    assert eliminate.index("const uint8_t human_mask") < eliminate.index("const uint8_t alive_mask")
    assert eliminate.index("const uint8_t alive_mask") < eliminate.index("status.human_mask_ =")
    assert eliminate.index("status.human_mask_ =") < eliminate.index("status.alive_mask_ =")

    result = _compile_and_run(fragments)
    assert result.returncode == 0, result.stdout + result.stderr

    mutants: list[tuple[str, str, str]] = []
    moved_snapshot = picker.replace(
        "const int saved_faction = ConsoleGlobal.faction_id_;\n        {\n            Popup popup2;",
        "{\n            Popup popup2;\n            const int saved_faction = ConsoleGlobal.faction_id_;", 1)
    mutants.append(("picker", moved_snapshot, "snapshot-after-constructor"))

    live_default = picker.replace(
        "popup2.set_value(saved_faction);",
        "popup2.set_value(ConsoleGlobal.faction_id_);", 1)
    mutants.append(("picker", live_default, "live-global-default"))

    wrong_start = picker.replace(
        "popup2.start(PopupStartCaption, ConsoleStrEditorWhoListTitle,\n                         -1, nullptr, 0x40);",
        "popup2.start(PopupStartCaption, ConsoleStrEditorWhoListTitle,\n                         saved_faction, nullptr, 0x40);", 1)
    mutants.append(("picker", wrong_start, "start-value"))

    set_before_items = picker.replace(
        "            for (int idx = 1; idx < MaxPlayerNum; ++idx) {",
        "            if (saved_faction >= 0) popup2.set_value(saved_faction);\n"
        "            for (int idx = 1; idx < MaxPlayerNum; ++idx) {", 1)
    mutants.append(("picker", set_before_items, "set-value-before-items"))

    modal_after_start = picker.replace(
        "            PopupModalState = 6;\n"
        "            popup2.start(PopupStartCaption, ConsoleStrEditorWhoListTitle,\n"
        "                         -1, nullptr, 0x40);",
        "            popup2.start(PopupStartCaption, ConsoleStrEditorWhoListTitle,\n"
        "                         -1, nullptr, 0x40);\n"
        "            PopupModalState = 6;", 1)
    mutants.append(("picker", modal_after_start, "modal-after-start"))

    wrong_member = picker.replace("row.noun_faction, idx", "row.filename, idx", 1)
    mutants.append(("picker", wrong_member, "wrong-row-member"))

    wrong_count = picker.replace("idx < MaxPlayerNum; ++idx", "idx < MaxPlayerNum - 1; ++idx", 1)
    mutants.append(("picker", wrong_count, "six-row-bound"))

    wrong_branch = picker.replace("(state & 0x80) != 0", "(state & 0x80) == 0", 1)
    mutants.append(("picker", wrong_branch, "wrong-editor-branch"))

    missing_cancel_close = picker.replace(
        "if (chosen < 0) {\n            popup.close();\n            return;",
        "if (chosen < 0) {\n            return;", 1)
    mutants.append(("picker", missing_cancel_close, "missing-cancel-close"))

    wrong_lock_byte = fragments["eliminate"].replace(
        "status.alive_mask_ = alive_mask &", "status.human_mask_ = alive_mask &", 1)
    mutants.append(("eliminate", wrong_lock_byte, "wrong-lock-byte-member"))

    for key, mutant_body, name in mutants:
        assert mutant_body != fragments[key], name + " mutation did not apply"
        changed = dict(fragments)
        changed[key] = mutant_body
        rejected = _compile_and_run(changed)
        assert rejected.returncode != 0, name + " mutant unexpectedly passed"
