"""Vehicle alias controls with explicit ABI domains.

Negative-index product behavior and all3072 signed cases require the selected
Win32 lane. The host lane tests pointer representation only, including high64
bits; independent host arrays cannot qualify Vehs[-1]. Set
OPENSMACX_VEHICLE_ALIAS_BUILD_REPORT to a fresh current application build report.
OPENSMACX_VEHICLE_ALIAS_REQUIRED=1 refuses missing proof/resources. Without an
explicitly selected native lane, its tests skip rather than claim coverage.
"""
from pathlib import Path
import os
import shutil
import signal
import subprocess
import sys
import tempfile

import pytest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'recovery/tests'))
from vehicle_alias_runtime_support import definition, run_native_control

COMPILERS = [path for name in ('g++', 'clang++') if (path := shutil.which(name))]

CONTROL = r"""
static void check_pointer_word(uintptr_t bits, bool spore_word) {
    Reactor[0].name_short = reinterpret_cast<char *>(bits);
    Reactor[1].name_short = reinterpret_cast<char *>(uintptr_t(0x12340040));
    Weapon[0].mode = 0;
    Weapon[0].offense_rating = 4;
    const int offense_expected = spore_word ? 3 : 4;
    const int armor_expected = spore_word ? 7 : 1;
    assert(offense_proto(0, -1, 1) == offense_expected);
    assert(offense_proto(0, -1, 0) == offense_expected);
    assert(armor_proto(0, -1, 1) == armor_expected);
    assert(armor_proto(0, -1, 0) == 7); // The bombardment-only read is bypassed.
    assert(offense_proto(BSC_SPORE_LAUNCHER,-1,1) == 3);
    assert(armor_proto(BSC_SPORE_LAUNCHER,-1,1) == 7);
    // Ordinary IDs keep their actual vehicle member, regardless of pointer bits.
    Vehs[2].proto_id = BSC_SPORE_LAUNCHER;
    assert(offense_proto(0,2,1) == 88);
    assert(armor_proto(0,2,1) == 136);
    Vehs[2].proto_id = BSC_SCOUT_PATROL;
    assert(offense_proto(0,2,1) == 32);
    assert(armor_proto(0,2,1) == 8);
    // The first infowar gate still uses the genuine positive vehicle record.
    Weapon[0].mode = WPN_MODE_INFOWAR;
    VehPrototypes[BSC_SCOUT_PATROL].plan = PLAN_INFO_WARFARE;
    assert(offense_proto(0,2,1) == 16);
    assert(armor_proto(0,2,1) == 16);
    VehPrototypes[BSC_SCOUT_PATROL].plan = PLAN_COMBAT;
    Weapon[0].mode = 0;
}
int main() {
    // Bits16..31, not the low pointer word, determine the original comparison.
    check_pointer_word(uintptr_t(0x000f0040),true);
    check_pointer_word(uintptr_t(0x0010000f),false);
    check_pointer_word(uintptr_t(0x800f0040),false);
    check_pointer_word(uintptr_t(0xffff0040),false);
    check_pointer_word(uintptr_t(0),false);
    if (sizeof(uintptr_t) > 4)
        check_pointer_word(static_cast<uintptr_t>(UINT64_C(0xabcd000f0040)),true);
    char actual_string[16] = "reactor";
    // Independent representation oracle for a real, live host pointer. Native
    // code/layout review supplies the separate original Win32 offset evidence.
    char *actual_pointer = actual_string;
    unsigned short words[sizeof(char *) / sizeof(unsigned short)];
    std::memcpy(words,&actual_pointer,sizeof actual_pointer);
    check_pointer_word(reinterpret_cast<uintptr_t>(actual_pointer), words[1] == 15);
}
"""

SIGNED_RATING_CONTROL = r"""
int main() {
    Reactor[0].name_short = reinterpret_cast<char *>(uintptr_t(0x00100040));
    Weapon[0].mode = 0;
    VehPrototypes[0].weapon_id = 0;
    Vehs[0].proto_id = 0;
    Vehs[2].proto_id = 0;
    const int defenders[] = {-1, 0, 2};
    int cases = 0;
    for (int rating = -128; rating <= 127; ++rating) {
        Weapon[0].offense_rating = rating;
        prototype_offense_value = rating;
        for (int missile = 0; missile != 2; ++missile) {
            prototype_missile = missile;
            for (int bombard = 0; bombard != 2; ++bombard) {
                for (int index = 0; index != 3; ++index) {
                    const int defender = defenders[index];
                    int expected;
                    if (bombard != 0 || rating >= 0) {
                        int magnitude = rating < 0 ? -rating : rating;
                        if (missile != 0 && rating < 99) {
                            magnitude = (magnitude * 3) / 2;
                        }
                        expected = defender < 0 ? magnitude : magnitude * 8;
                    } else {
                        // The defender helper supplies nonnegative defense;
                        // a negative attacking rating takes the native PSI arm.
                        expected = defender < 0 ? 3 : 88;
                    }
                    const int actual = offense_proto(0, defender, bombard);
                    if (actual != expected) {
                        std::fprintf(stderr,
                            "pin_guard_case rating=%d missile=%d bombard=%d defender=%d actual=%d expected=%d\n",
                            rating, missile, bombard, defender, actual, expected);
                    }
                    assert(actual == expected);
                    ++cases;
                }
            }
        }
    }
    assert(cases == 3072);
}
"""


@pytest.fixture(scope='module')
def target32_control(tmp_path_factory):
    build_report = os.environ.get('OPENSMACX_VEHICLE_ALIAS_BUILD_REPORT')
    required = os.environ.get('OPENSMACX_VEHICLE_ALIAS_REQUIRED') == '1'
    if not build_report:
        if required:
            pytest.fail('required target32 vehicle alias lane needs a fresh build report')
        pytest.skip('target32 vehicle alias lane not selected; host representation is not a substitute')
    if not Path(build_report).is_file():
        pytest.fail('selected target32 build report is missing: ' + build_report)
    artifact_parent = os.environ.get('OPENSMACX_VEHICLE_ALIAS_ARTIFACTS')
    if artifact_parent:
        artifact_parent = Path(artifact_parent).resolve()
        artifact_parent.mkdir(parents=True, exist_ok=True)
        directory = Path(tempfile.mkdtemp(prefix='vehicle-alias-', dir=artifact_parent))
    else:
        artifact_parent = ROOT / 'build/recovery/vehicle-alias-runtime'
        artifact_parent.mkdir(parents=True, exist_ok=True)
        directory = Path(tempfile.mkdtemp(prefix='control-', dir=artifact_parent)).resolve()
    assert directory.is_relative_to(ROOT), 'native reproducible control artifacts must be inside ROOT'
    report = run_native_control(
        ROOT, build_report, directory, CONTROL, SIGNED_RATING_CONTROL,
        os.environ.get('OPENSMACX_VEHICLE_ALIAS_RESOURCE_FILE'))
    print('VEHICLE_ALIAS_ARTIFACTS=' + str(directory))
    return report


@pytest.mark.parametrize('mode,code,message', [
    ('actual', 0, 'CONTROL_PASS guarded+product'),
    ('wrong-member', 41, 'offense_under_test(0, -1, 1) == offense_expected'),
    ('wrong-owner', 42, 'offense_under_test(0, -1, 1) == offense_expected'),
    ('old-magnitude', 51,
     'pin_guard_case rating=-128 missile=1 bombard=1 defender=-1 actual=128 expected=192'),
])
def test_required_target32_actual_ratings_and_causal_controls(target32_control, mode, code, message):
    result = target32_control['outcomes'][mode]
    assert result['returncode'] == code, result
    assert message in result['stdout'] + result['stderr'], result
    if code:
        assert 'CONTROL_FAILURE mode=' + mode + ' code=' + str(code) in result['stderr']
    else:
        assert 'signed_cases=3072_each,total6144' in result['stdout']
        assert target32_control['signed_cases_each'] == 3072
    assert target32_control['compiler_calls'] == target32_control['linker_calls'] == 1
    assert target32_control['runtime_calls'] == 4
    assert target32_control['source_settings_unchanged']


@pytest.mark.parametrize('compiler', COMPILERS)
@pytest.mark.parametrize('wrong_owner', [False, True], ids=['representation', 'wrong-owner-control'])
def test_host_pointer_representation_only(tmp_path, compiler, wrong_owner):
    """No rating bodies or out-of-array access execute in this host64 lane."""
    reactor = definition((ROOT / 'src/veh.h').read_text(), 'struct RulesReactor {') + ';'
    source = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
typedef char *LPSTR;
''' + reactor + r'''
static RulesReactor reactors[2];
static void check(uintptr_t bits, bool spore_word) {
    reactors[0].name_short = reinterpret_cast<char *>(bits);
    reactors[1].name_short = reinterpret_cast<char *>(uintptr_t(0x12340040));
    const int selected = @OWNER@;
    const int16_t word = static_cast<int16_t>(
        reinterpret_cast<uintptr_t>(reactors[selected].name_short) >> 16);
    assert((word == 15) == spore_word);
}
int main() {
    check(uintptr_t(0x000f0040), true);
    check(uintptr_t(0x0010000f), false);
    check(uintptr_t(0x800f0040), false);
    check(uintptr_t(0xffff0040), false);
    check(uintptr_t(0), false);
    if (sizeof(uintptr_t) > 4)
        check(static_cast<uintptr_t>(UINT64_C(0xabcd000f0040)), true);
    char actual_string[16] = "reactor";
    char *actual_pointer = actual_string;
    unsigned short words[sizeof(char *) / sizeof(unsigned short)];
    std::memcpy(words, &actual_pointer, sizeof actual_pointer);
    check(reinterpret_cast<uintptr_t>(actual_pointer), words[1] == 15);
}
'''
    source = source.replace('@OWNER@', '1' if wrong_owner else '0')
    unit, binary = tmp_path / 'representation.cpp', tmp_path / 'representation'
    unit.write_text(source)
    subprocess.run([compiler, '-std=c++17', str(unit), '-o', str(binary)], check=True, timeout=60)
    ran = subprocess.run([str(binary)], capture_output=True, timeout=10)
    if wrong_owner:
        assert ran.returncode == -signal.SIGABRT
        assert b'spore_word' in ran.stderr
    else:
        assert ran.returncode == 0, ran.stderr.decode(errors='replace')
