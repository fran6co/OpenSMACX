"""Host controls for the pinned raw alien-rule test in actual social_calc."""
from __future__ import annotations

import os
import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest


ROOT = Path(os.environ["OPENSMACX_ROOT"]).resolve() if os.environ.get("OPENSMACX_ROOT") else Path(__file__).resolve().parents[2]
FACTION_CPP = ROOT / "src/faction.cpp"
FACTION_H = ROOT / "src/faction.h"


def _definition(text: str, signature: str, marker: str) -> str:
    marker_at = text.find(marker)
    assert marker_at >= 0, "actual source marker missing: " + marker
    start = text.find(signature, marker_at)
    assert start >= 0, "actual source definition missing: " + signature
    opening = text.find("{", start)
    assert opening >= 0, "actual source body missing: " + signature

    depth = 0
    i = opening
    state = "code"
    while i < len(text):
        ch = text[i]
        following = text[i + 1] if i + 1 < len(text) else ""
        if state == "code":
            if ch == "/" and following == "/":
                state = "line_comment"
                i += 2
                continue
            if ch == "/" and following == "*":
                state = "block_comment"
                i += 2
                continue
            if ch == '"':
                state = "string"
            elif ch == "'":
                state = "character"
            elif ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    body = text[start : i + 1]
                    assert "BODY GOES HERE." not in body
                    return body
        elif state == "line_comment":
            if ch == "\n":
                state = "code"
        elif state == "block_comment":
            if ch == "*" and following == "/":
                state = "code"
                i += 2
                continue
        elif state in ("string", "character"):
            if ch == "\\":
                i += 2
                continue
            if (state == "string" and ch == '"') or (state == "character" and ch == "'"):
                state = "code"
        i += 1
    raise AssertionError("unbalanced actual source definition: " + signature)


def _actual_definitions() -> tuple[str, str]:
    faction_source = FACTION_CPP.read_text(encoding="utf-8")
    faction_header = FACTION_H.read_text(encoding="utf-8")
    social_calc = _definition(
        faction_source,
        "void __cdecl social_calc(SocialCategory *category, SocialEffect *effect, int faction_id,",
        "// ORIGINAL: 0x005B4210 ?social_calc@@YAXPAHPAHHHH@Z",
    )
    alien_helper = _definition(
        faction_header,
        "inline BOOL __cdecl is_alien_faction(uint32_t faction_id)",
        "inline BOOL __cdecl is_alien_faction(uint32_t faction_id)",
    )
    assert "if (Players[faction_id].rule_flags & RFLAG_ALIEN)" in social_calc
    assert "effect->values_[SocialEffect::RESEARCH]++" in social_calc
    assert "ExpansionEnabled &&" in alien_helper
    return social_calc, alien_helper


HARNESS = r'''
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define __cdecl
#define UNUSED(value) value
typedef int BOOL;
enum { MaxSocialCatNum = 4, MaxSocialEffectNum = 11 };
enum { SOCIAL_CAT_VALUES = 2, SOCIAL_CAT_FUTURE = 3 };
enum { SE_CYBERNETIC = 1, SE_THOUGHT_CONTROL = 2, SE_POWER = 3 };
enum { SP_NETWORK_BACKBONE = 21, SP_CLONING_VATS = 24,
       SP_ASCETIC_VIRTUES = 34, SP_LIVING_REFINERY = 48 };
enum { RULE_IMMUNITY = 4, RULE_IMPUNITY = 5, RULE_PENALTY = 6, RULE_ROBUST = 10 };
enum { RFLAG_ALIEN = 0x80 };

struct SocialEffect {
    enum { ECONOMY, EFFICIENCY, SUPPORT, TALENT, MORALE, POLICE,
           GROWTH, PLANET, PROBE, INDUSTRY, RESEARCH };
    int values_[MaxSocialEffectNum];
};
struct SocialCategory { uint32_t models_[MaxSocialCatNum]; };
struct RulesSocialEffect { SocialEffect model_effect[4]; };
struct Player {
    int faction_bonus_count;
    int faction_bonus_id[8];
    int faction_bonus_val1[8];
    int faction_bonus_val2[8];
    uint32_t rule_flags;
};
struct PlayerData { SocialEffect soc_effect_base; };

RulesSocialEffect SocialCategories[MaxSocialCatNum];
Player Players[8];
PlayerData PlayersData[8];
BOOL ExpansionEnabled;
BOOL temple_result;

void ZeroMemory(void *data, size_t size) { std::memset(data, 0, size); }
BOOL has_project(int, int) { return 0; }
BOOL has_temple(int) { return temple_result; }
int range(int value, int low, int high) {
    return value < low ? low : (value > high ? high : value);
}

/*ACTUAL_ALIEN_HELPER*/
/*ACTUAL_SOCIAL_CALC*/

static void require(bool ok, const char *case_name, const char *field) {
    if (!ok) {
        std::fprintf(stderr, "%s: %s\n", case_name, field);
        std::exit(86);
    }
}

static void setup(SocialCategory &category, int expansion, int alien, int temple) {
    std::memset(SocialCategories, 0, sizeof(SocialCategories));
    std::memset(Players, 0, sizeof(Players));
    std::memset(PlayersData, 0, sizeof(PlayersData));
    std::memset(&category, 0, sizeof(category));
    ExpansionEnabled = expansion;
    temple_result = temple;
    Players[0].rule_flags = alien ? RFLAG_ALIEN : 0;

    // Exercise unaffected category accumulation, immunity cancellation,
    // base-effect addition, quick-calculation behavior, and temple ordering.
    SocialCategories[0].model_effect[0].values_[SocialEffect::ECONOMY] = 2;
    SocialCategories[0].model_effect[0].values_[SocialEffect::EFFICIENCY] = -3;
    Players[0].faction_bonus_count = 1;
    Players[0].faction_bonus_id[0] = RULE_IMPUNITY;
    Players[0].faction_bonus_val1[0] = 0;
    Players[0].faction_bonus_val2[0] = 0;
    for (int i = 0; i < MaxSocialEffectNum; ++i)
        PlayersData[0].soc_effect_base.values_[i] = i + 10;
}

static void run_case(const char *name, int expansion, int alien, int temple,
                     int quick, int expected_research, int expected_planet) {
    SocialCategory category;
    SocialEffect effect;
    setup(category, expansion, alien, temple);
    social_calc(&category, &effect, 0, 0, quick);
    if (quick) {
        require(effect.values_[SocialEffect::ECONOMY] == 2, name, "quick economy");
        require(effect.values_[SocialEffect::EFFICIENCY] == 0, name, "quick immunity");
        require(effect.values_[SocialEffect::SUPPORT] == 0, name, "quick skips base");
    } else {
        require(effect.values_[SocialEffect::ECONOMY] == 12, name, "base plus category economy");
        require(effect.values_[SocialEffect::EFFICIENCY] == 11, name, "immunity then base effect");
        require(effect.values_[SocialEffect::SUPPORT] == 12, name, "base support");
    }
    require(effect.values_[SocialEffect::PLANET] == expected_planet, name, "temple planet");
    require(effect.values_[SocialEffect::RESEARCH] == expected_research, name, "alien research");
}

int main() {
    run_case("raw alien bit without expansion", 0, 1, 1, 0, 21, 18);
    run_case("no alien bit", 0, 0, 1, 0, 20, 18);
    run_case("expansion enabled", 1, 1, 1, 0, 21, 18);
    run_case("no temple", 0, 1, 0, 0, 20, 17);
    run_case("quick calculation", 0, 1, 1, 1, 0, 0);
    return 0;
}
'''


def _build_and_run(tmp_path: Path, compiler: str, source: str, name: str) -> subprocess.CompletedProcess[str]:
    cpp = tmp_path / (name + ".cpp")
    binary = tmp_path / name
    cpp.write_text(source, encoding="utf-8")
    build = subprocess.run(
        [compiler, "-std=c++17", "-O0", "-Wall", "-Wextra", "-Werror",
         "-Wno-unused-parameter", str(cpp), "-o", str(binary)],
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        timeout=60,
    )
    assert build.returncode == 0, build.stdout
    return subprocess.run([str(binary)], text=True, stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT, timeout=20)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_social_calc_raw_alien_flag_and_old_predicate_control(tmp_path: Path, compiler_name: str) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " is unavailable")

    social_calc, alien_helper = _actual_definitions()
    current = HARNESS.replace("/*ACTUAL_ALIEN_HELPER*/", alien_helper).replace(
        "/*ACTUAL_SOCIAL_CALC*/", social_calc
    )
    with tempfile.TemporaryDirectory(prefix="social-calc-alien-current-", dir=tmp_path) as directory:
        result = _build_and_run(Path(directory), compiler, current, "actual-current")
        assert result.returncode == 0, result.stdout

    # Recreate the prior source predicate while retaining the actual helper
    # definition. The exact same real social_calc body and oracle must reject it.
    baseline_body = social_calc.replace(
        "if (Players[faction_id].rule_flags & RFLAG_ALIEN)",
        "if (is_alien_faction(faction_id))",
        1,
    )
    assert baseline_body != social_calc
    baseline = HARNESS.replace("/*ACTUAL_ALIEN_HELPER*/", alien_helper).replace(
        "/*ACTUAL_SOCIAL_CALC*/", baseline_body
    )
    with tempfile.TemporaryDirectory(prefix="social-calc-alien-baseline-", dir=tmp_path) as directory:
        result = _build_and_run(Path(directory), compiler, baseline, "actual-baseline")
        assert result.returncode != 0, "old ExpansionEnabled-gated predicate escaped the actual-source control"
        assert "raw alien bit without expansion: alien research" in result.stdout, result.stdout
