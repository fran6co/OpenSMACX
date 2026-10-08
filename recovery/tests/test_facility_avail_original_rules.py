"""Whole-body host controls for the recovered facility_avail rules.

The complete function is extracted from src/base.cpp. Only boundary calls are
redirected to trace doubles; the function body is not transformed. These are selected-path host controls, not native ABI or game-runtime proof.
"""

from pathlib import Path
import re
import shutil
import subprocess

import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS
from match_recovery.source import read_file
from match_recovery.binding_source import parse as parse_bindings


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/base.cpp"
BASE_H = ROOT / "src/base.h"

HARNESS = r"""
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#define __cdecl
typedef int BOOL;
enum { DLVL_SPECIALIST = 1, RULES_SCN_NO_BUILDING_SP = 0x10000,
       RULES_VICTORY_TRANSCENDENCE = 0x800, RFLAG_FREEPROTO = 1 };
enum { SP_CITIZENS_DEFENSE_FORCE = 10, SP_VIRTUAL_WORLD = 11,
       SP_CLOUDBASE_ACADEMY = 12, SP_SPACE_ELEVATOR = 13,
       SP_VOICE_OF_PLANET = 31, SP_Unbuilt = -1 };

struct FacilityRecord { int preq_tech; };
struct Player { char filename[24]; uint32_t rule_flags; };
struct Base { int x, y; };
struct Fields { int diff_level_current_; uint32_t game_rules_; };
struct GameState { Fields fields_; };
FacilityRecord Facility[128];
Player Players[8];
Base Bases[8];
GameState GameDataStateStorage;
int ExpansionEnabled;
char StringTemp[1032];
bool built[128];
int queue_ids[10];
int project_owner[128];
std::vector<std::string> calls;

void reset_state() {
    std::memset(Facility, 0, sizeof(Facility));
    std::memset(Players, 0, sizeof(Players));
    std::memset(Bases, 0, sizeof(Bases));
    std::memset(&GameDataStateStorage, 0, sizeof(GameDataStateStorage));
    std::memset(StringTemp, 0, sizeof(StringTemp));
    std::memset(built, 0, sizeof(built));
    for (int i = 0; i < 10; ++i) queue_ids[i] = 0;
    for (int i = 0; i < 128; ++i) project_owner[i] = SP_Unbuilt;
    ExpansionEnabled = 1;
    GameDataStateStorage.fields_.diff_level_current_ = 3;
    std::strcpy(Players[0].filename, "GAIANS");
    calls.clear();
}
char *trace_strcat(char *destination, const char *source) {
    calls.push_back("strcat"); return std::strcat(destination, source);
}
int trace_strcmp(const char *left, const char *right) {
    calls.push_back("strcmp"); return std::strcmp(left, right);
}
char *trace_upper(char *text) {
    calls.push_back("CharUpperA");
    for (char *p = text; *p; ++p) if (*p >= 'a' && *p <= 'z') *p -= ('a' - 'A');
    return text;
}
int has_tech(int, int) { calls.push_back("has_tech"); return 1; }
int ascending(int faction) {
    (void)faction;
    calls.push_back("ascending"); return project_owner[SP_VOICE_OF_PLANET] != SP_Unbuilt;
}
int base_project(int project) {
    calls.push_back("base_project"); return project_owner[project];
}
int has_project(int project, int) { return project_owner[project] != SP_Unbuilt; }
int has_fac(int facility, int, int queue_count) {
    if (built[facility]) return 1;
    for (int i = 0; i < queue_count; ++i) if (queue_ids[i] == -facility) return 1;
    return 0;
}
int redundant(int, int) { return 0; }
int is_coast(int, int, int) { return 0; }
int is_alien_faction(int) { return 0; }

ACTUAL_FUNCTION

int test_garden() {
    reset_state(); built[FAC_PUNISHMENT_SPHERE] = true;
    if (!facility_avail(FAC_PARADISE_GARDEN, 0, 0, 0)) return 1;
    reset_state(); queue_ids[0] = -FAC_PUNISHMENT_SPHERE;
    if (!facility_avail(FAC_PARADISE_GARDEN, 0, 0, 1)) return 2;
    reset_state(); built[FAC_PARADISE_GARDEN] = true;
    if (facility_avail(FAC_PUNISHMENT_SPHERE, 0, 0, 0)) return 3;
    reset_state(); queue_ids[0] = -FAC_PARADISE_GARDEN;
    if (facility_avail(FAC_PUNISHMENT_SPHERE, 0, 0, 1)) return 4;
    return 0;
}
int test_ascent() {
    reset_state();
    project_owner[SP_VOICE_OF_PLANET] = 0;
    GameDataStateStorage.fields_.game_rules_ = RULES_VICTORY_TRANSCENDENCE;
    std::strcpy(Players[0].filename, "CARETAKE");
    if (!facility_avail(FAC_ASCENT_TO_TRANSCENDENCE, 0, 0, 0)) return 1;
    if (calls.size() != 1 || calls[0] != "ascending") return 2;
    reset_state();
    GameDataStateStorage.fields_.game_rules_ = RULES_VICTORY_TRANSCENDENCE;
    if (facility_avail(FAC_ASCENT_TO_TRANSCENDENCE, 0, 0, 0)) return 3;
    reset_state(); project_owner[SP_VOICE_OF_PLANET] = 0;
    if (facility_avail(FAC_ASCENT_TO_TRANSCENDENCE, 0, 0, 0)) return 4;
    return 0;
}
int test_voice() {
    reset_state(); std::strcpy(Players[0].filename, "caretake");
    std::strcpy(StringTemp, "stale");
    if (facility_avail(FAC_VOICE_OF_PLANET, 0, 0, 0)) return 1;
    if (std::strcmp(StringTemp, "CARETAKE") != 0) return 2;
    if (calls.size() != 4 || calls[0] != "has_tech" || calls[1] != "strcat"
        || calls[2] != "CharUpperA" || calls[3] != "strcmp") return 3;
    reset_state(); std::strcpy(Players[0].filename, "gaianS");
    if (!facility_avail(FAC_VOICE_OF_PLANET, 0, 0, 0)) return 4;
    if (std::strcmp(StringTemp, "GAIANS") != 0) return 5;
    if (calls.size() != 5 || calls[0] != "has_tech" || calls[1] != "strcat"
        || calls[2] != "CharUpperA" || calls[3] != "strcmp"
        || calls[4] != "base_project") return 6;
    return 0;
}
int main(int argc, char **argv) {
    if (argc != 2) return 90;
    if (std::strcmp(argv[1], "garden") == 0) return test_garden();
    if (std::strcmp(argv[1], "ascent") == 0) return test_ascent();
    if (std::strcmp(argv[1], "voice") == 0) return test_voice();
    return 91;
}
"""


def extract_facility_enum():
    text = BASE_H.read_text()
    start = text.index("enum FacilityId {")
    end = text.index("};", start) + 2
    return text[start:end]


def extract_function():
    text = SOURCE.read_text()
    records = read_file(SOURCE)
    owner, = [r for r in records if r.address == 0x005BA0E0]
    assert owner.line in parse_bindings(text, records)
    masked = COMMENTS_OR_STRINGS.sub(lambda m: re.sub(r"[^\n]", " ", m[0]), text)
    signature = "BOOL __cdecl facility_avail(int facility_id, int faction_id, int base_id,"
    start = masked.index(signature)
    opening = masked.index("{", start)
    depth = 0
    for pos in range(opening, len(masked)):
        if masked[pos] == "{":
            depth += 1
        elif masked[pos] == "}":
            depth -= 1
            if depth == 0:
                return text[start:pos + 1]
    raise AssertionError("facility_avail body is unterminated")


def instrument_boundary_calls(body):
    return re.sub(r"\bstrcat\s*\(", "trace_strcat(",
                  re.sub(r"\bstrcmp\s*\(", "trace_strcmp(",
                         re.sub(r"\bCharUpperA\s*\(", "trace_upper(", body)))


def mutated_function(mutation=None):
    body = instrument_boundary_calls(extract_function())
    if mutation == "garden_gate":
        body = body.replace(
            "      case FAC_PUNISHMENT_SPHERE:",
            "      case FAC_PARADISE_GARDEN:\n"
            "        return !has_fac(FAC_PUNISHMENT_SPHERE, base_id, queue_count);\n"
            "      case FAC_PUNISHMENT_SPHERE:", 1)
    elif mutation == "ascent_caretaker":
        anchor = "return ascending(faction_id) && GameDataStateStorage.fields_.game_rules_ & RULES_VICTORY_TRANSCENDENCE;"
        assert body.count(anchor) == 1
        body = body.replace(anchor,
            "return ascending(faction_id) && GameDataStateStorage.fields_.game_rules_ & RULES_VICTORY_TRANSCENDENCE && trace_strcmp(Players[faction_id].filename, \"CARETAKE\");", 1)
    elif mutation == "missing_clear":
        assert body.count("    StringTemp[0] = 0;\n") == 1
        body = body.replace("    StringTemp[0] = 0;\n", "", 1)
    elif mutation == "missing_upper":
        assert body.count("    trace_upper(StringTemp);\n") == 1
        body = body.replace("    trace_upper(StringTemp);\n", "", 1)
    elif mutation is not None:
        raise AssertionError(f"unknown mutation: {mutation}")
    return body


def compile_actual(tmp_path, mutation=None, compiler_name="g++"):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"host {compiler_name} required")
    unit = tmp_path / "facility_avail_original_rules.cpp"
    executable = tmp_path / "facility_avail_original_rules"
    unit.write_text(HARNESS.replace("ACTUAL_FUNCTION", mutated_function(mutation))
                    .replace("enum { DLVL_SPECIALIST", extract_facility_enum()
                             + "\nenum { DLVL_SPECIALIST"))
    result = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", "-pedantic",
         str(unit), "-o", str(executable)], capture_output=True, text=True, timeout=30)
    assert result.returncode == 0, result.stderr
    return executable


@pytest.fixture(scope="module", params=("g++", "clang++"))
def actual_function_binary(request, tmp_path_factory):
    return compile_actual(tmp_path_factory.mktemp("facility-avail-actual"),
                          compiler_name=request.param)


@pytest.mark.parametrize("scenario", ("garden", "ascent", "voice"))
def test_actual_full_body_original_rules(actual_function_binary, scenario):
    result = subprocess.run([str(actual_function_binary), scenario], capture_output=True,
                            text=True, timeout=10)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("mutation,scenario", (
    ("garden_gate", "garden"),
    ("ascent_caretaker", "ascent"),
    ("missing_clear", "voice"),
    ("missing_upper", "voice"),
))
def test_damage_mutants_fail(tmp_path, mutation, scenario, compiler_name):
    executable = compile_actual(tmp_path, mutation, compiler_name)
    result = subprocess.run([str(executable), scenario], capture_output=True,
                            text=True, timeout=10)
    assert result.returncode != 0, f"{mutation} escaped the behavioral control"
