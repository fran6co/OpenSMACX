"""Actual-source checks for byte-width human/alive mask consumers."""
from pathlib import Path
import shutil
import subprocess
import re

import pytest


ROOT = Path(__file__).resolve().parents[2]
HUMAN = "GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_"
ALIVE = "GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_"


def extract_body(path: Path, signature: str) -> str:
    source = path.read_text()
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start : end + 1]
    raise AssertionError(f"unterminated production body: {signature}")


def extract_braced_declaration(path: Path, signature: str) -> str:
    source = path.read_text()
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                declaration_end = end + 1
                if declaration_end < len(source) and source[declaration_end] == ";":
                    declaration_end += 1
                return source[start:declaration_end]
    raise AssertionError(f"unterminated declaration: {signature}")


BASE = ROOT / "src/basewin.cpp"
MAP = ROOT / "src/mapwin.cpp"
POPUP = ROOT / "src/popup.cpp"
BODIES = {
    "farm_click": extract_body(BASE, "void BaseWin::farm_click("),
    "rename": extract_body(BASE, "void BaseWin::rename()"),
    "gov_on": extract_body(BASE, "void BaseWin::gov_on("),
    "update_gov_buttons": extract_body(BASE, "void BaseWin::update_gov_buttons()"),
    "draw_vehicles": extract_body(MAP, "void MapWin::draw_vehicles("),
    "draw_base_names": extract_body(MAP, "void MapWin::draw_base_names()"),
    "POP3": extract_body(POPUP, "void __cdecl POP3(const char *section, const char *unused"),
    "popb": extract_body(POPUP, "int __cdecl popb("),
}
MASK_DECLARATIONS = "\n".join(
    extract_braced_declaration(ROOT / "src/game_data_state_window.h", signature)
    for signature in ("struct GameDataLockMaskBytes", "union GameDataLockMaskView")
)
GOVERNOR_DECLARATION = extract_braced_declaration(
    ROOT / "src/base.h", "enum GovernorBaseBitfield"
)


def assert_member_contract(name: str, body: str) -> None:
    if name == "POP3":
        assert ALIVE in body
        assert HUMAN in body
        assert "faction_mask << 8" not in body
        assert "lock_masks_.enable_mask_" not in body
    else:
        assert HUMAN in body
        assert "lock_masks_.enable_mask_" not in body
        assert ALIVE not in body


def test_all_eight_bodies_use_evidenced_byte_members():
    header = (ROOT / "src/game_data_state_window.h").read_text()
    assert "uint8_t human_mask_;" in header
    assert "uint8_t alive_mask_;" in header
    assert "offsetof(GameDataLockMaskBytes, human_mask_) == 0" in header
    assert "offsetof(GameDataLockMaskBytes, alive_mask_) == 1" in header
    for name, body in BODIES.items():
        assert_member_contract(name, body)


@pytest.mark.parametrize("name", BODIES)
def test_wrong_byte_or_dword_alias_is_rejected(name):
    body = BODIES[name]
    assert_member_contract(name, body)
    if name == "POP3":
        damaged = body.replace(ALIVE, HUMAN, 1)
    else:
        damaged = body.replace(HUMAN, "GameDataStateStorage.fields_.lock_masks_.enable_mask_", 1)
    assert damaged != body
    with pytest.raises(AssertionError):
        assert_member_contract(name, damaged)


UPDATE_HUMAN_READ = f"({HUMAN} & (1u << faction)) != 0"


UPDATE_HARNESS = r'''
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

/* MASK_DECLARATIONS */
struct GameDataStateWindowFields { GameDataLockMaskView lock_masks_; };
struct GameDataStorage { GameDataStateWindowFields fields_; } GameDataStateStorage;
struct Base { int faction_id_current; unsigned int governor; } BaseValue, BaseReplacement, *BaseCurrent = &BaseValue;
struct PlayerData { int ai_growth, ai_tech, ai_wealth, ai_power; } PlayersData[8];
int CallbackOrder[8];
int CallbackCount;
int Scenario;
int CallbackTrigger;

/* GOVERNOR_DECLARATION */

struct FlatButton {
    int id_;
    int value_;
    int auxiliary_;
    void set(int value) { value_ = value; }
    void clear_auxiliary_state_for_disabled_button() {
        if (auxiliary_ != 0) {
            auxiliary_ = 0;
            CallbackOrder[CallbackCount++] = id_;
            if (id_ == CallbackTrigger && Scenario != 0) {
                GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ =
                    Scenario == 1 ? 0 : static_cast<unsigned char>(1u << 2);
                if (Scenario == 1) {
                    BaseReplacement.governor = 0;
                } else if (CallbackTrigger == 4) {
                    BaseReplacement.governor = GOV_ACTIVE | GOV_PRIORITY_DISCOVER;
                } else if (CallbackTrigger == 5) {
                    BaseReplacement.governor = GOV_ACTIVE | GOV_PRIORITY_BUILD;
                } else {
                    BaseReplacement.governor = GOV_ACTIVE | GOV_PRIORITY_CONQUER;
                }
                BaseCurrent = &BaseReplacement;
            }
        }
    }
};
class BaseWin {
public:
    FlatButton flatButtons_[20];
    void update_gov_buttons();
};
#define require(condition) do { if (!(condition)) { fprintf(stderr, "failed:%d\\n", __LINE__); exit(1); } } while (0)
'''


def update_harness(cached_human_mutant: bool = False,
                   cached_governor_mutant: bool = False) -> str:
    body = BODIES["update_gov_buttons"]
    if cached_human_mutant:
        body = re.sub(
            re.escape(HUMAN) + r"\s*&\s*\(1u << faction\)\) != 0",
            "cached_human",
            body,
        )
        body = body.replace("if ((cached_human) {", "if (cached_human) {")
    if cached_human_mutant or cached_governor_mutant:
        body = body.replace(
            "    const unsigned int initial_governor = BaseCurrent->governor;",
            "    const unsigned int initial_governor = BaseCurrent->governor;\n"
            + (f"    const bool cached_human = {UPDATE_HUMAN_READ};\n" if cached_human_mutant else "")
            + ("    const unsigned int cached_governor = BaseCurrent->governor;\n" if cached_governor_mutant else ""),
        )
    if cached_governor_mutant:
        body = body.replace(
            "const unsigned int governor = BaseCurrent->governor;",
            "const unsigned int governor = cached_governor;",
        )
    harness = UPDATE_HARNESS.replace("/* MASK_DECLARATIONS */", MASK_DECLARATIONS)
    harness = harness.replace("/* GOVERNOR_DECLARATION */", GOVERNOR_DECLARATION)
    return harness + "\n" + body + r'''

int run_scenario(int scenario, int target_button) {
    BaseCurrent = &BaseValue;
    BaseValue.faction_id_current = 2;
    BaseValue.governor = GOV_ACTIVE;
    BaseReplacement.faction_id_current = 5;
    BaseReplacement.governor = 0;
    Scenario = scenario;
    CallbackTrigger = target_button - 1;
    GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ = 1u << 2;
    GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ = 0xA5;
    PlayersData[2].ai_growth = 0;
    PlayersData[2].ai_tech = 0;
    PlayersData[2].ai_wealth = 0;
    PlayersData[2].ai_power = 0;
    PlayersData[5].ai_growth = 0;
    PlayersData[5].ai_tech = 0;
    PlayersData[5].ai_wealth = 0;
    PlayersData[5].ai_power = 0;
    if (scenario == 1) {
        if (target_button == 5) PlayersData[2].ai_tech = 1;
        if (target_button == 6) PlayersData[2].ai_wealth = 1;
        if (target_button == 7) PlayersData[2].ai_power = 1;
    }
    CallbackCount = 0;

    BaseWin win;
    for (int i = 0; i < 20; ++i) {
        win.flatButtons_[i].id_ = i;
        win.flatButtons_[i].value_ = -1;
        win.flatButtons_[i].auxiliary_ = 0;
    }
    win.flatButtons_[4].auxiliary_ = 1;
    win.flatButtons_[5].auxiliary_ = 1;
    win.flatButtons_[6].auxiliary_ = 1;
    win.flatButtons_[7].auxiliary_ = 1;
    win.update_gov_buttons();
    require(win.flatButtons_[8].value_ == 1);
    require(CallbackCount == 3);
    int order = 0;
    for (int button = 4; button <= 7; ++button) {
        if (button != target_button) require(CallbackOrder[order++] == button);
    }
    require(order == 3);
    return win.flatButtons_[target_button].value_;
}

int main() {
    require(run_scenario(1, 5) == 1);
    require(run_scenario(1, 6) == 1);
    require(run_scenario(1, 7) == 1);
    require(run_scenario(2, 5) == 1);
    require(run_scenario(2, 6) == 1);
    require(run_scenario(2, 7) == 1);
    return 0;
}
'''


def compile_and_run(compiler: str, source: str, tmp_path: Path):
    cpp = tmp_path / "ui_mask_update_gov_buttons.cpp"
    executable = tmp_path / "ui_mask_update_gov_buttons"
    cpp.write_text(source)
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(executable)],
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)


COMPILERS = [compiler for compiler in (shutil.which("g++"), shutil.which("clang++")) if compiler]


@pytest.mark.parametrize("compiler", COMPILERS or [None])
def test_update_gov_buttons_reloads_human_byte_after_auxiliary_redraws(compiler, tmp_path):
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    result = compile_and_run(compiler, update_harness(), tmp_path)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler", COMPILERS or [None])
@pytest.mark.parametrize("mutation", ("cached_human", "cached_governor"))
def test_cached_human_or_governor_mutant_compiles_then_fails(compiler, mutation, tmp_path):
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    result = compile_and_run(
        compiler,
        update_harness(
            cached_human_mutant=mutation == "cached_human",
            cached_governor_mutant=mutation == "cached_governor",
        ),
        tmp_path,
    )
    assert result.returncode != 0


BYTE_VIEW_HARNESS = rf'''
#include <stdlib.h>
#include <stdint.h>
/* MASK_DECLARATIONS */
struct Fields {{ GameDataLockMaskView lock_masks_; }};
struct Storage {{ Fields fields_; }} GameDataStateStorage;
void require(bool value) {{ if (!value) exit(1); }}
int main() {{
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0xD2A50000u | (0x29u << 8) | 0x49u;
    const unsigned int bit = 1u << 3;
    require((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & bit) != 0);
    require((GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ & bit) != 0);
    require((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1u << 1)) == 0);
    require((GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ & (1u << 1)) == 0);
    return 0;
}}
'''


@pytest.mark.parametrize("compiler", COMPILERS or [None])
def test_human_and_alive_members_are_distinct_byte_lanes(compiler, tmp_path):
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    harness = BYTE_VIEW_HARNESS.replace("/* MASK_DECLARATIONS */", MASK_DECLARATIONS)
    result = compile_and_run(compiler, harness, tmp_path)
    assert result.returncode == 0, result.stdout + result.stderr
