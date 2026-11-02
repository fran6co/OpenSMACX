"""Actual-source attack-score controls, with host-only deterministic stack data.

The original uninitialized array remains in production. Compiler auto-init here
provides a bounded diagnostic fixture, not proof of original stack/runtime effects.
"""
from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest
from match_recovery.source import read_file


ADDRESS = 0x0055BC80
SIGNATURE = 'BOOL __cdecl wants_to_attack(int faction_id, int faction_id_tgt, int faction_id_unk) {'


ROOT = Path(__file__).resolve().parents[2]

def _extract_actual_definition() -> str:
    source = ROOT / 'src/faction.cpp'
    record, = [row for row in read_file(source) if row.address == ADDRESS]
    region = record.region
    start = region.find(SIGNATURE)
    assert start >= 0, 'native source reader did not find the selected definition'
    brace = region.find('{', start)
    assert brace >= 0
    depth = 0
    i = brace
    state = 'code'
    while i < len(region):
        ch = region[i]
        following = region[i + 1] if i + 1 < len(region) else ''
        if state == 'code':
            if ch == '/' and following == '/':
                state = 'line'; i += 2; continue
            if ch == '/' and following == '*':
                state = 'block'; i += 2; continue
            if ch == '"': state = 'string'
            elif ch == "'": state = 'char'
            elif ch == '{': depth += 1
            elif ch == '}':
                depth -= 1
                if depth == 0:
                    body = region[start:i + 1]
                    assert 'BODY GOES HERE.' not in body
                    return body
        elif state == 'line':
            if ch == '\n': state = 'code'
        elif state == 'block':
            if ch == '*' and following == '/':
                state = 'code'; i += 2; continue
        else:
            if ch == '\\': i += 2; continue
            if state == 'string' and ch == '"': state = 'code'
            elif state == 'char' and ch == "'": state = 'code'
        i += 1
    raise AssertionError('unbalanced selected source definition')


def _host_compatibility(body: str) -> str:
    # VC6 extends this for-init declaration to the containing function block;
    # standard C++ scopes it only to the first loop. This mechanical test-only
    # adaptation preserves source operation/order and makes the later reuse
    # compile without rewriting the body as pseudocode.
    original = '    for (int region = 1; region < MaxRegionLandNum; region++) {'
    adapted = '    int region;\n    for (region = 1; region < MaxRegionLandNum; region++) {'
    assert body.count(original) == 1
    return body.replace(original, adapted, 1)


CPP_MODEL = r'''#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#define __cdecl

namespace @NAMESPACE@ {
using uint8_t = std::uint8_t;
using uint16_t = std::uint16_t;
using uint32_t = std::uint32_t;
using BOOL = int;
enum { MaxPlayerNum = 8, MaxRegionLandNum = 64 };
enum { RFLAG_ALIEN = 0x80, PFLAG_TEAM_UP_VS_HUMAN = 0x800000,
       RULES_INTENSE_RIVALRY = 0x40, FAC_HEADQUARTERS = 1 };
enum { DTREATY_PACT = 0x1, DTREATY_TREATY = 0x2, DTREATY_COMMLINK = 0x8,
       DTREATY_VENDETTA = 0x10, DTREATY_WANT_REVENGE = 0x20,
       DTREATY_UNK_40 = 0x40, DTREATY_ATROCITY_VICTIM = 0x40000,
       DTREATY_HAVE_SURRENDERED = 0x2000000,
       DTREATY_UNK_4000000 = 0x4000000,
       DTREATY_UNK_20000000 = 0x20000000, DAGENDA_UNK_200 = 0x200 };
namespace SocialEffect { enum { MORALE = 0 }; }
struct Player { int rule_flags, rule_morale; };
struct PlayerData {
    int major_atrocities;
    int current_num_bases;
    uint32_t flags;
    int mil_strength_1, ai_fight, tech_commerce_bonus;
    int best_armor_value, best_weapon_value;
    uint16_t region_total_combat_vehs[128];
    uint8_t region_total_bases[128];
    uint16_t region_force_rating[128];
    struct { int values_[16]; } soc_effect_pending;
    int integrity_blemishes;
};
struct Base { uint8_t faction_id_current; int x, y; };
struct GameFields { int turn_current_num_; uint32_t game_rules_;
                     int faction_rankings_[8]; int base_current_count_; };
struct GameState { GameFields fields_; };
struct Result { int value; std::vector<std::string> calls; };
static Player Players[MaxPlayerNum];
static PlayerData PlayersData[MaxPlayerNum];
static Base Bases[128];
static GameState GameDataStateStorage;
static uint32_t treaties[MaxPlayerNum][MaxPlayerNum];
static bool human[MaxPlayerNum];
static std::vector<std::string> calls;
static void event(const char *name, int a=0, int b=0, int c=0) {
    calls.push_back(std::string(name)+":"+std::to_string(a)+":"+
                    std::to_string(b)+":"+std::to_string(c));
}
static void reset() {
    std::memset(Players,0,sizeof(Players));
    std::memset(PlayersData,0,sizeof(PlayersData));
    std::memset(Bases,0,sizeof(Bases));
    std::memset(&GameDataStateStorage,0,sizeof(GameDataStateStorage));
    std::memset(treaties,0,sizeof(treaties));
    std::memset(human,0,sizeof(human));
    calls.clear();
    PlayersData[1].current_num_bases=2;
}
static BOOL has_treaty(int a,int b,int mask) {
    event("treaty",a,b,mask);
    return (treaties[a][b] & static_cast<uint32_t>(mask)) != 0;
}
static BOOL is_human(int f) { event("human",f); return human[f]; }
static BOOL great_beelzebub(int f,BOOL a) { event("beelzebub",f,a); return 0; }
static BOOL great_satan(int f,BOOL a) { event("satan",f,a); return 0; }
static BOOL has_agenda(int a,int b,int g) { event("agenda",a,b,g); return 0; }
static BOOL has_fac_built(int f,int b) { event("facility",f,b); return 0; }
static int region_at(int x,int y) { event("region_at",x,y); return 0; }
static BOOL bad_reg(int r) { event("bad_reg",r); return 1; }
static int range(int v,int lo,int hi) { return std::max(lo,std::min(v,hi)); }

@BODY@

static Result run_case(int which) {
    reset();
    if (which == 0 || which == 1) {
        PlayersData[1].ai_fight = which == 0 ? 1 : 0;
        PlayersData[2].best_weapon_value = 3;
        PlayersData[2].best_armor_value = 1;
    } else if (which == 2) {
        Players[1].rule_flags=RFLAG_ALIEN; Players[2].rule_flags=RFLAG_ALIEN;
    } else if (which == 3) {
        treaties[1][2]=DTREATY_WANT_REVENGE;
    } else if (which == 4) {
        PlayersData[1].major_atrocities=1; PlayersData[2].major_atrocities=1;
    } else if (which == 5) {
        treaties[1][2]=DTREATY_UNK_4000000;
    } else if (which == 6) {
        PlayersData[1].current_num_bases=1;
    } else if (which == 7) {
        PlayersData[1].flags=PFLAG_TEAM_UP_VS_HUMAN;
        human[2]=false;
    }
    return Result{wants_to_attack(1,2,0),calls};
}
static std::string final_treaty_event() {
    return std::string("treaty:1:2:")+std::to_string(DTREATY_UNK_20000000);
}
} // namespace @NAMESPACE@
'''

CPP_MAIN = r'''int main() {
    // Valid bounded state: ai_fight=1 (-2), target weapon 3 > 2*armor 2
    // (-1), no pair vendetta (+1), no pair pact (+1): signed score -1.
    auto old_negative=old_unsigned::run_case(0);
    auto signed_negative=signed_candidate::run_case(0);
    assert(old_negative.value==0 && signed_negative.value==1);
    assert(old_negative.calls.size()+1==signed_negative.calls.size());
    assert(std::equal(old_negative.calls.begin(),old_negative.calls.end(),
                      signed_negative.calls.begin()));
    assert(signed_negative.calls.back()==signed_candidate::final_treaty_event());

    // Positive control: only ai_fight changes to 0, score becomes +1.
    auto old_positive=old_unsigned::run_case(1);
    auto signed_positive=signed_candidate::run_case(1);
    assert(old_positive.value==0 && signed_positive.value==0);
    assert(old_positive.calls==signed_positive.calls);

    // Early guard return and call-order controls; types are declared after
    // these branches, so both body variants must match on all six cases.
    const int expected[]={1,1,1,0,0,0};
    for(int mode=2;mode!=8;++mode) {
        auto a=old_unsigned::run_case(mode);
        auto b=signed_candidate::run_case(mode);
        assert(a.value==expected[mode-2] && b.value==expected[mode-2]);
        assert(a.calls==b.calls);
    }
    assert(old_unsigned::run_case(2).calls.empty());
    assert(old_unsigned::run_case(3).calls.size()==1);
    assert(old_unsigned::run_case(5).calls.size()==2);
    auto team=old_unsigned::run_case(7);
    assert(team.calls.back()=="human:2:0:0");
    return 0;
}
'''


def _harness(old_body: str, candidate_body: str) -> str:
    old_namespace = CPP_MODEL.replace('@NAMESPACE@', 'old_unsigned').replace('@BODY@', old_body)
    new_namespace = CPP_MODEL.replace('@NAMESPACE@', 'signed_candidate').replace('@BODY@', candidate_body)
    return old_namespace + '\n' + new_namespace + '\n' + CPP_MAIN


def _compile_and_run(compiler: str, harness: str, directory: Path, label: str) -> subprocess.CompletedProcess[str]:
    source = directory / f'{label}.cpp'
    binary = directory / label
    source.write_text(harness)
    command = [compiler, '-std=c++17', '-O0', '-UNDEBUG',
               '-ftrivial-auto-var-init=zero', str(source), '-o', str(binary)]
    built = subprocess.run(command, capture_output=True, text=True, timeout=60)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=20)


def _supports_deterministic_stack(compiler: str, directory: Path) -> bool:
    source = directory / 'stack_init_probe.cpp'
    source.write_text('int main() { int value; return value; }\n')
    result = subprocess.run(
        [compiler, '-std=c++17', '-ftrivial-auto-var-init=zero',
         str(source), '-o', str(directory / 'stack_init_probe')],
        capture_output=True, text=True, timeout=30)
    return result.returncode == 0


@pytest.mark.parametrize('compiler_name', ['g++', 'clang++'])
def test_wants_to_attack_signed_score_actual_source(compiler_name: str, tmp_path: Path) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f'{compiler_name} is unavailable')
    if not _supports_deterministic_stack(compiler, tmp_path):
        pytest.skip(f'{compiler_name} lacks the test-only deterministic automatic-storage option')
    candidate = _extract_actual_definition()
    # Old behavior is derived from the live candidate by exactly the three
    # authorized declaration reversions; no stored historical function copy.
    old_body = candidate
    for declaration in ('    int want_to_attack = 0;',
                        '    int factor_force_rating = 0;',
                        '    int factor_unk = 1;'):
        assert candidate.count(declaration) == 1
        old_body = old_body.replace(declaration, declaration.replace('int ', 'uint32_t '), 1)
    normal = _compile_and_run(compiler, _harness(_host_compatibility(old_body),
                                                 _host_compatibility(candidate)),
                              tmp_path, 'normal')
    assert normal.returncode == 0, normal.stdout + normal.stderr

    # Causal mutant: restore only the unsigned score local. The negative-score
    # oracle must fail even though the two factor locals remain signed.
    score_mutant = candidate.replace('    int want_to_attack = 0;',
                                     '    uint32_t want_to_attack = 0;', 1)
    score_result = _compile_and_run(compiler, _harness(_host_compatibility(old_body),
                                                       _host_compatibility(score_mutant)),
                                    tmp_path, 'score_mutant')
    assert score_result.returncode != 0, 'unsigned-score mutant escaped the negative witness'

    # Order mutant: move the final treaty predicate before the score predicate.
    # The positive control must observe the extra query and fail its trace oracle.
    old_gate = ('if ((factor_count || want_to_attack > 0\n'
                '        || has_treaty(faction_id, faction_id_tgt, DTREATY_UNK_20000000)) &&')
    new_gate = ('if ((factor_count || has_treaty(faction_id, faction_id_tgt, DTREATY_UNK_20000000)\n'
                '        || want_to_attack > 0) &&')
    assert candidate.count(old_gate) == 1
    order_mutant = candidate.replace(old_gate, new_gate, 1)
    order_result = _compile_and_run(compiler, _harness(_host_compatibility(old_body),
                                                       _host_compatibility(order_mutant)),
                                    tmp_path, 'order_mutant')
    assert order_result.returncode != 0, 'gate-order mutant escaped callback trace controls'


@pytest.mark.parametrize('compiler_name', ['g++', 'clang++'])
def test_surrendered_partner_requires_pact(compiler_name: str, tmp_path: Path) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f'{compiler_name} is unavailable')
    if not _supports_deterministic_stack(compiler, tmp_path):
        pytest.skip(f'{compiler_name} lacks the existing host fixture compiler option')
    body = _host_compatibility(_extract_actual_definition())
    main = r'''int main() {
        using namespace treaty_candidate;
        // Original 55BDC7..55BDD5 requires both surrender and pact before
        // remembering a peace partner. Its vendetta then forces an attack.
        // Otherwise the negative ai_fight guard returns before the original
        // uninitialized region array is reached; these cases need no stack model.
        const uint32_t masks[] = {0, DTREATY_PACT, DTREATY_HAVE_SURRENDERED,
                                 DTREATY_PACT | DTREATY_HAVE_SURRENDERED};
        for (int i = 0; i != 4; ++i) {
            reset();
            PlayersData[1].ai_fight = -1;
            treaties[1][3] = masks[i] | DTREATY_COMMLINK;
            treaties[2][3] = DTREATY_VENDETTA;
            assert(wants_to_attack(1, 2, 0) == (i == 3));
        }
        // Original 55BF2F is a distinct ANY(pact,treaty) predicate. Either
        // agreement with the established partner must prevent an attack.
        const uint32_t agreements[] = {DTREATY_PACT, DTREATY_TREATY};
        for (int i = 0; i != 2; ++i) {
            reset();
            PlayersData[1].ai_fight = 1;
            PlayersData[2].best_weapon_value = 3;
            PlayersData[2].best_armor_value = 1;
            treaties[1][3] = DTREATY_HAVE_SURRENDERED | DTREATY_PACT;
            treaties[2][3] = agreements[i];
            assert(wants_to_attack(1, 2, 0) == 0);
        }
        return 0;
    }'''

    def run(candidate: str, label: str) -> subprocess.CompletedProcess[str]:
        harness = CPP_MODEL.replace('@NAMESPACE@', 'treaty_candidate').replace('@BODY@', candidate)
        return _compile_and_run(compiler, harness + '\n' + main, tmp_path, label)

    normal = run(body, 'partner_normal')
    assert normal.returncode == 0, normal.stdout + normal.stderr
    required = ('has_treaty(faction_id, i, DTREATY_HAVE_SURRENDERED)\n'
                '                && has_treaty(faction_id, i, DTREATY_PACT)')
    assert body.count(required) == 1
    any_bit = body.replace(required,
                           'has_treaty(faction_id, i, DTREATY_HAVE_SURRENDERED | DTREATY_PACT)', 1)
    damaged = run(any_bit, 'partner_any_bit_mutant')
    assert damaged.returncode != 0, 'single-bit partner mutant escaped the attack decision oracle'


"""Compile with -ftrivial-auto-var-init=zero only to make the
function's documented, intentionally uninitialized region_top_base_count
diagnostic reproducible. No initialization is inserted into either extracted
body. This cannot model the original arbitrary prior stack contents or establish
game/runtime behavior. Exact current_num_bases/mil_strength_1/region field widths
are copied from faction.h. The one for-scope adaptation mirrors VC6's old
extension in a standard-host syntax; no body logic is recoded."""
