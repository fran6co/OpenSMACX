"""Actual-source controls for DesignWin's fixed-owner data flows.

LPSTR is represented by a 32-bit token so RulesAbility remains the image's
0x1C-byte record. Host pointer members (including BoxSpriteParams::mask_) still
use host width, so these controls verify field/index flow, not full ABI layout,
the game's pointer ABI or complete DesignWin behavior.
"""

from __future__ import annotations

import re
import shutil
import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
DESIGN = ROOT / "src/designwin.cpp"


def definition(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for offset in range(opening, len(source)):
        if source[offset] == "{":
            depth += 1
        elif source[offset] == "}":
            depth -= 1
            if depth == 0:
                return source[start : offset + 1]
    raise AssertionError(f"unterminated definition: {signature}")


def struct_definition(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for offset in range(opening, len(source)):
        if source[offset] == "{":
            depth += 1
        elif source[offset] == "}":
            depth -= 1
            if depth == 0:
                end = source.index(";", offset) + 1
                return source[start:end]
    raise AssertionError(f"unterminated declaration: {signature}")


def owner_fragments() -> dict[str, str]:
    faction = (ROOT / "src/faction.h").read_text(encoding="utf-8")
    veh = (ROOT / "src/veh.h").read_text(encoding="utf-8")
    buffer = (ROOT / "src/buffer.h").read_text(encoding="utf-8")
    return {
        "player": struct_definition(faction, "struct Player {"),
        "weapon": struct_definition(veh, "struct RulesWeapon {"),
        "ability": struct_definition(veh, "struct RulesAbility {"),
        "box": struct_definition(buffer, "struct BoxSpriteParams {"),
    }


def source_fragments() -> dict[str, str]:
    source = DESIGN.read_text(encoding="utf-8")
    on_status = definition(source, "void DesignWin::on_status(int mode)")
    stack = definition(source, "void DesignWin::draw_stack()")
    design_it = definition(source, "int DesignWin::design_it()")
    flash = definition(source, "void DesignWin::draw_flash()")
    special = definition(source, "void DesignWin::select_special(int which)")

    loop_start = flash.index("int ability_bit = 0;")
    for_start = flash.index("for (", loop_start)
    opening = flash.index("{", for_start)
    depth = 0
    for offset in range(opening, len(flash)):
        if flash[offset] == "{":
            depth += 1
        elif flash[offset] == "}":
            depth -= 1
            if depth == 0:
                ability_loop = flash[loop_start : offset + 1]
                break
    else:
        raise AssertionError("unterminated draw_flash ability loop")

    game_state = re.search(
        r"const uint32_t game_state\s*=\s*"
        r"GameDataStateStorage\.fields_\.game_state_;\s*"
        r"if\s*(\(\(game_state & 0x40\) != 0 && "
        r"\(game_state & 0x1000\) != 0\))", stack)
    assert game_state, "draw_stack must snapshot the original DWORD once"
    human_condition = re.search(
        r"if\s*(\(\(Players\[factionID_\]\.rule_flags & RFLAG_ALIEN\) == 0\))",
        design_it)
    special_expression = re.search(
        r"if\s*(\(\(Players\[faction\]\.rule_flags & RFLAG_ALIEN\) != 0\))",
        special)
    offense_expression = re.search(
        r"Weapon\[VehPrototypes\[proto_id\]\.weapon_id\]\.offense_rating",
        stack)
    style_expression = re.search(r"InterfaceBoxStyles\[9\]\.mask_", stack)
    color_calls = re.findall(
        r"canvas->set_text_color\(TextColourTable\[\d+\],\s*"
        r"TextColourTable\[\d+\],\s*1,\s*1\);", flash)
    preq_read = re.search(
        r"const int16_t preq_tech = Ability\[ability\]\.preq_tech;", special)
    scan_start = special.index("const uint8_t *const player_bytes =")
    scan_end = special.index("            for (int slot = 0; slot < 0x29; ++slot)", scan_start)
    loop_open = special.index("{", scan_end)
    depth = 0
    for offset in range(loop_open, len(special)):
        if special[offset] == "{":
            depth += 1
        elif special[offset] == "}":
            depth -= 1
            if depth == 0:
                scan_loop = special[scan_end : offset + 1]
                break
    else:
        raise AssertionError("unterminated select_special build-slot scan")
    assert human_condition and special_expression and offense_expression
    assert style_expression and len(color_calls) == 2 and preq_read
    assert design_it.count("Players[factionID_].rule_flags & RFLAG_ALIEN") == 4
    return {
        "on_status": on_status,
        "stack": stack,
        "design_it": design_it,
        "flash": flash,
        "special": special,
        "ability_loop": ability_loop,
        "game_state_condition": game_state.group(1),
        "human_condition": human_condition.group(1),
        "special_expression": special_expression.group(1),
        "offense_expression": offense_expression.group(0),
        "style_expression": style_expression.group(0),
        "title_color_call": color_calls[0],
        "text_color_call": color_calls[1],
        "preq_read": preq_read.group(0),
        "scan_start_declarations": special[scan_start:scan_end],
        "scan_loop": scan_loop,
    }


HARNESS = r'''
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>

using BOOL = int;
using LPSTR = uint32_t; // 32-bit image pointer token for host layout controls
using SocialCategory = int;
using SocialEffect = int;
struct Sprite {};
struct Buffer {
    int text_colors[4] = {};
    void set_text_color(int a, int b, int c, int d) {
        text_colors[0] = a; text_colors[1] = b;
        text_colors[2] = c; text_colors[3] = d;
    }
};

__PLAYER__
__WEAPON__
__ABILITY__
__BOX__

static_assert(offsetof(Player, rule_flags) == 0x508, "pinned Player flag offset");
static_assert(offsetof(Player, faction_bonus_id) == 0x510, "pinned bonus ID start");
static_assert(offsetof(Player, faction_bonus_val1) == 0x530, "pinned bonus value start");
static_assert(sizeof(Player) == 0x59C, "pinned Player faction stride");
static_assert(offsetof(RulesWeapon, offense_rating) == 8, "pinned weapon field");
static_assert(sizeof(RulesWeapon) == 0x10, "pinned weapon stride");
static_assert(offsetof(RulesAbility, abbreviation) == 8, "pinned ability label field");
static_assert(offsetof(RulesAbility, preq_tech) == 0x18, "signed prereq field");
static_assert(sizeof(RulesAbility) == 0x1C, "pinned ability stride");
static_assert(std::is_signed<decltype(RulesAbility::preq_tech)>::value,
              "Ability prerequisite remains signed");

constexpr int RFLAG_ALIEN = 0x80;
constexpr int MaxAbilityNum = 29;
Player Players[8] = {};
RulesWeapon Weapon[26] = {};
RulesAbility Ability[MaxAbilityNum] = {};
BoxSpriteParams InterfaceBoxStyles[51] = {};
int TextColourTable[195] = {};
char StringTemp[256] = {};

struct StringTableMock {
    char *rows[64] = {};
    int calls = 0;
    char *get(uint32_t id) { ++calls; return rows[id]; }
} StringTable;

struct ObservedWords {
    int words[41] = {};
    mutable int reads = 0;
    int operator[](int index) const { ++reads; return words[index]; }
};

struct VehPrototype { uint8_t weapon_id = 0; uint32_t ability_flags = 0; };
VehPrototype VehPrototypes[1] = {};

void bonus_scan_starts(int faction, const int **out_kind, const int **out_id) {
    __SCAN_START_DECLARATIONS__
    *out_kind = built_kind;
    *out_id = built_id;
}

bool scan_already_built(const ObservedWords &built_kind,
                        const ObservedWords &built_id, int ability) {
    int already_built = 0;
    __SCAN_LOOP__
    return already_built != 0;
}

bool faction_human(int factionID_) { return __HUMAN_CONDITION__; }
bool selected_faction_alien(int faction) {
    return __SPECIAL_EXPRESSION__;
}

bool stack_game_state_visible(uint32_t state) {
    const uint32_t game_state = state;
    return __GAME_STATE_CONDITION__;
}

bool weapon_unofferable(int proto_id) {
    return __OFFENSE_EXPRESSION__ < 0;
}

Buffer *stack_tile_source() { return __STYLE_EXPRESSION__; }

void draw_title_colors(Buffer *canvas) { __TITLE_COLOR_CALL__ }
void draw_stat_colors(Buffer *canvas) { __TEXT_COLOR_CALL__ }
int special_prerequisite(int ability) {
    __PREQ_READ__
    return preq_tech;
}

void draw_ability_labels(int prototype_id) {
    __ABILITY_LOOP__
}

int main() {
    // Check source-derived starts against real members without executing the
    // original 41-dword scans or their unsafe endpoint.
    const int *built_kind = nullptr;
    const int *built_id = nullptr;
    bonus_scan_starts(3, &built_kind, &built_id);
    assert(built_kind == &Players[3].faction_bonus_id[0]);
    assert(built_id == &Players[3].faction_bonus_val1[0]);

    // Exercise the extracted original scan with bounded, instrumented words.
    // The real Players endpoint is intentionally never executed here.
    const int match_positions[] = {0, 12, 40};
    for (int match_at : match_positions) {
        ObservedWords kinds;
        ObservedWords ids;
        kinds.words[match_at] = 0xF;
        ids.words[match_at] = 7;
        assert(scan_already_built(kinds, ids, 7));
        assert(kinds.reads == match_at + 1);
        assert(ids.reads == 1);
    }
    ObservedWords no_match_kinds;
    ObservedWords no_match_ids;
    assert(!scan_already_built(no_match_kinds, no_match_ids, 7));
    assert(no_match_kinds.reads == 0x29);
    assert(no_match_ids.reads == 0);

    // A faction other than zero controls all four live design_it reads.
    Players[0].rule_flags = 0;
    Players[3].rule_flags = RFLAG_ALIEN;
    assert(!faction_human(3));
    assert(selected_faction_alien(3));

    // The original DWORD load's AL/AH tests are bits 0x40 and 0x1000.
    assert(stack_game_state_visible(0x1040));
    assert(!stack_game_state_visible(0x0040));
    assert(!stack_game_state_visible(0x1000));

    VehPrototypes[0].weapon_id = 7;
    Weapon[7].offense_rating = -3;
    assert(weapon_unofferable(0));
    Weapon[7].offense_rating = 0;
    assert(!weapon_unofferable(0));

    Buffer expected_tile;
    InterfaceBoxStyles[9].mask_ = &expected_tile;
    assert(stack_tile_source() == &expected_tile);

    TextColourTable[82] = 82;
    TextColourTable[83] = 83;
    TextColourTable[14] = 14;
    TextColourTable[15] = 15;
    Buffer canvas;
    draw_title_colors(&canvas);
    assert(canvas.text_colors[0] == 82 && canvas.text_colors[1] == 83);
    draw_stat_colors(&canvas);
    assert(canvas.text_colors[0] == 14 && canvas.text_colors[1] == 15);

    StringTable.rows[1] = const_cast<char *>("first");
    StringTable.rows[2] = const_cast<char *>("second");
    StringTable.rows[29] = const_cast<char *>("last");
    Ability[1].abbreviation = 1;
    Ability[2].abbreviation = 2;
    Ability[28].abbreviation = 29;
    Ability[1].name = 30;
    Ability[2].name = 32;
    Ability[28].name = 31;
    Ability[0].abbreviation = 3;
    Ability[2].preq_tech = -2;
    StringTable.rows[30] = const_cast<char *>("wrong-name-one");
    StringTable.rows[32] = const_cast<char *>("wrong-name-two");
    StringTable.rows[31] = const_cast<char *>("wrong-name-last");
    StringTable.rows[3] = const_cast<char *>("wrong-stride");
    assert(special_prerequisite(2) == -2);
    VehPrototypes[0].ability_flags = (1u << 1) | (1u << 2) | (1u << 28);
    draw_ability_labels(0);
    assert(std::strcmp(StringTemp, ", first, second, last") == 0);
    assert(StringTable.calls == 6); // two gets per present nonempty label
    return 0;
}
'''


def compile_and_run(tmp_path: Path, fragments: dict[str, str]) -> subprocess.CompletedProcess[str]:
    compiler = shutil.which("c++") or shutil.which("clang++") or shutil.which("g++")
    assert compiler, "a host C++ compiler is required for these actual-source controls"
    source = HARNESS
    for key, value in owner_fragments().items():
        source = source.replace(f"__{key.upper()}__", value)
    for key, value in fragments.items():
        source = source.replace(f"__{key.upper()}__", value)
    unit = tmp_path / "designwin_fixed_owner_flow.cpp"
    binary = tmp_path / "designwin_fixed_owner_flow"
    unit.write_text(source, encoding="utf-8")
    built = subprocess.run([compiler, "-std=c++17", "-O0", str(unit), "-o", str(binary)],
                           text=True, capture_output=True)
    assert built.returncode == 0, "actual-source control/mutant must compile: " + built.stderr
    return subprocess.run([str(binary)], text=True, capture_output=True)


def source_fragments_for_harness() -> dict[str, str]:
    return source_fragments()


def test_five_bodies_use_the_pinned_source_owners_and_access_order() -> None:
    parts = source_fragments()
    assert "GameDataStateStorage.fields_.game_state_ & 0x40" in parts["on_status"]
    assert "InterfaceBoxStyles[9].mask_" in parts["stack"]
    assert "Weapon[VehPrototypes[proto_id].weapon_id].offense_rating < 0" in parts["stack"]
    assert "const uint32_t game_state =" in parts["stack"]
    assert parts["stack"].count("GameDataStateStorage.fields_.game_state_") == 1
    assert "(game_state & 0x40) != 0 && (game_state & 0x1000) != 0" in parts["stack"]
    assert parts["stack"].index("say_stats_3(StringTemp, Vehs[unit_id].proto_id);") < parts["stack"].index("const uint32_t game_state =") < parts["stack"].index("canvas->write_cent_l(StringTemp, &stats_rect")
    assert parts["flash"].count("TextColourTable[82]") == 1
    assert parts["flash"].count("TextColourTable[83]") == 1
    assert parts["flash"].count("TextColourTable[14]") == 1
    assert parts["flash"].count("TextColourTable[15]") == 1
    assert parts["design_it"].count("Players[factionID_].rule_flags & RFLAG_ALIEN") == 4
    assert "PlayerFlagBlocks" not in parts["design_it"]
    assert parts["special_expression"] in parts["special"]
    assert "ability < MaxAbilityNum" in parts["special"]
    assert "ability < 39" not in parts["special"]
    assert "const int16_t preq_tech = Ability[ability].preq_tech;" in parts["special"]
    assert parts["special"].index("if (!has_tech(preq_tech, faction))") < parts["special"].index("abil_cond(ability,")
    assert parts["special"].index("for (int ability = 0; ability < MaxAbilityNum;") < parts["special"].index("const uint8_t *const player_bytes")
    assert "reinterpret_cast<const uint8_t *>(&Players)" in parts["scan_start_declarations"]
    assert "sizeof(Player) * faction" in parts["scan_start_declarations"]
    assert "offsetof(Player, faction_bonus_id)" in parts["scan_start_declarations"]
    assert "offsetof(Player, faction_bonus_val1)" in parts["scan_start_declarations"]
    assert "for (int slot = 0; slot < 0x29; ++slot)" in parts["special"]
    assert "built_kind[slot] == 0xF && built_id[slot] == ability" in parts["special"]
    assert "break;" in parts["scan_loop"]
    assert "0x29 dwords" in parts["special"]
    # The second 41-dword window runs 0x38 bytes past one Player, and the
    # final faction extends beyond Players[8]. Do not execute that endpoint.
    assert parts["flash"].count("StringTable.get(Ability[ability_bit].abbreviation)") == 2
    entire_source = DESIGN.read_text(encoding="utf-8")
    for retired_alias in (
        "InterfaceFlagByte", "InterfaceFlagTail", "StackTileBufferSlot",
        "StackTitleColorMain", "StackTitleColorShadow", "StackTextColorMain",
        "StackTextColorShadow", "WeaponClassRows", "AbilityLabelIds",
        "AbilityLabelIdsEnd",
    ):
        assert retired_alias not in entire_source
    assert "PlayerFlagBlocks" not in entire_source


def test_actual_owner_flow_compiles_and_rejects_behavior_mutants(tmp_path: Path) -> None:
    parts = source_fragments_for_harness()
    baseline = compile_and_run(tmp_path, parts)
    assert baseline.returncode == 0, baseline.stdout + baseline.stderr

    mutants = (
        ("HUMAN_CONDITION", "(Players[0].rule_flags & RFLAG_ALIEN) == 0"),
        ("SPECIAL_EXPRESSION", "(Players[0].rule_flags & RFLAG_ALIEN) != 0"),
        ("GAME_STATE_CONDITION", "(game_state & 0x40) != 0 && (game_state & 0x10) != 0"),
        ("OFFENSE_EXPRESSION", "static_cast<uint8_t>(Weapon[VehPrototypes[proto_id].weapon_id].offense_rating)"),
        ("STYLE_EXPRESSION", "InterfaceBoxStyles[8].mask_"),
        ("TITLE_COLOR_CALL", "canvas->set_text_color(TextColourTable[80], TextColourTable[83], 1, 1);"),
        ("TEXT_COLOR_CALL", "canvas->set_text_color(TextColourTable[14], TextColourTable[13], 1, 1);"),
        ("ABILITY_LOOP", parts["ability_loop"].replace("MaxAbilityNum", "MaxAbilityNum - 1")),
        ("ABILITY_LOOP", parts["ability_loop"].replace("ability_bit].abbreviation", "ability_bit / 7].abbreviation")),
        ("ABILITY_LOOP", parts["ability_loop"].replace(".abbreviation", ".name")),
        ("PREQ_READ", parts["preq_read"].replace("int16_t", "uint16_t")),
    )
    for key, mutant in mutants:
        changed = dict(parts)
        changed[key.lower()] = mutant
        rejected = compile_and_run(tmp_path, changed)
        assert rejected.returncode != 0, f"{key} behavior mutant unexpectedly passed"

    no_break = dict(parts)
    no_break["scan_loop"] = parts["scan_loop"].replace("break;", "")
    rejected = compile_and_run(tmp_path, no_break)
    assert rejected.returncode != 0, "no-break scan mutant unexpectedly passed"


def test_source_owner_mutants_are_rejected_before_running_unsafe_reads() -> None:
    source = DESIGN.read_text(encoding="utf-8")
    parts = source_fragments()
    stack = definition(source, "void DesignWin::draw_stack()")
    flash = definition(source, "void DesignWin::draw_flash()")
    assert "InterfaceBoxStyles[9].mask_" in stack
    assert "Weapon[VehPrototypes[proto_id].weapon_id].offense_rating < 0" in stack
    assert "TextColourTable[82], TextColourTable[83]" in flash
    assert "TextColourTable[14], TextColourTable[15]" in flash
    assert "Ability[ability_bit].abbreviation" in flash
    assert "AbilityLabelIds" not in flash

    # Deliberately bounded lexical/source controls for owner, signedness and
    # palette mapping. The original unchecked 41-dword scans remain in place,
    # now rooted at their source-owned member offsets.
    for wrong_owner in ("InterfaceBoxStyles[8].mask_", "WeaponClassRows[",
                        "StackTitleColor", "StackTextColor", "AbilityLabelIds"):
        assert wrong_owner not in stack + flash
    special = definition(source, "void DesignWin::select_special(int which)")
    assert "Players[faction].rule_flags & RFLAG_ALIEN" in special
    assert "offsetof(Player, faction_bonus_id)" in special
    assert "offsetof(Player, faction_bonus_val1)" in special
    assert "Pinned 0x0043AF9E exits this scan on its first match" in special
    assert "break;" in parts["scan_loop"]
