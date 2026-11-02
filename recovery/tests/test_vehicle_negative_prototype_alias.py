"""Execute the actual rating bodies with the original signed pointer-word alias.

Opaque pointer tokens exercise representation reads, never pointer dereferences.
These host controls do not establish Win32 heap addresses or original layout.
"""
from pathlib import Path
import shutil
import signal
import subprocess
import sys

import pytest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'recovery/tests'))
from test_movie_runtime_decoder_ownership_actual_source_oct9 import _definition

COMPILERS = [path for name in ('g++','clang++') if (path := shutil.which(name))]
PRELUDE = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <cstdio>
#define __cdecl
typedef char *LPSTR;
typedef int BOOL;
'''
FIXTURE = r'''
static Veh Vehs[3];
static VehPrototype VehPrototypes[32];
static RulesReactor Reactor[4];
struct WeaponRow { int mode; int offense_rating; };
static WeaponRow Weapon[1];
struct RatioFixture { int psi_combat_ratio_atk[3]; int psi_combat_ratio_def[3]; };
static RatioFixture Rules = {{3,11,13},{7,16,17}};
static int prototype_offense_value = -4;
static int prototype_missile;
static int get_proto_offense_rating(int) { return prototype_offense_value; }
static int get_proto_defense_rating(int) { return -2; }
static int get_offense_rating(int id) { assert(id >= 0); return 4; }
static int get_defense_rating(int id) { assert(id >= 0); return 1; }
static int get_triad(int id) { assert(id >= 0); return TRIAD_SEA; }
static int get_proto_triad(int) { return TRIAD_AIR; }
static int is_proto_missile(int) { return prototype_missile; }
static int range(int value, int lower, int upper) {
    return value < lower ? lower : value > upper ? upper : value;
}
'''
CONTROL = r'''
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
'''


@pytest.mark.parametrize('compiler',COMPILERS)
@pytest.mark.parametrize('wrong_storage',[False,True],ids=['actual','wrong-owner-control'])
def test_actual_negative_prototype_pointer_word(tmp_path,compiler,wrong_storage):
    header = (ROOT/'src/veh.h').read_text()
    declarations = '\n'.join(_definition(header,'enum '+name+' {')+';' for name in
                              ('VehTriad','VehBasic','VehPlan','VehWeaponMode'))
    declarations += '\n' + '\n'.join(_definition(header,'struct '+name+' {')+';' for name in
                                       ('Veh','VehPrototype','RulesReactor'))
    source = (ROOT/'src/veh.cpp').read_text()
    bodies = '\n'.join(_definition(source,'int __cdecl '+name+'(') for name in
                        ('offense_proto','armor_proto'))
    if wrong_storage:
        anchor = 'reinterpret_cast<uintptr_t>(Reactor[0].name_short)'
        assert bodies.count(anchor) == 2
        bodies = bodies.replace(anchor,'reinterpret_cast<uintptr_t>(Reactor[1].name_short)')
    unit,binary = tmp_path/'ratings.cpp',tmp_path/'ratings'
    unit.write_text(PRELUDE+declarations+FIXTURE+bodies+CONTROL)
    subprocess.run([compiler,'-std=c++17',str(unit),'-o',str(binary)],check=True,timeout=60)
    result = subprocess.run([str(binary)],capture_output=True,timeout=10)
    if wrong_storage:
        assert result.returncode == -signal.SIGABRT
        assert b'offense_expected' in result.stderr
    else:
        assert result.returncode == 0,result.stderr.decode(errors='replace')


# The expected values follow native 5C1200..5C1244: signed raw byte controls
# the <99 threshold, while the separately negated DWORD supplies magnitude.
# This is a body-level oracle; the existing controls retain the distinct
# pointer-word alias, infowar, and unsafe-path qualification.
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


@pytest.mark.parametrize('compiler', COMPILERS)
@pytest.mark.parametrize('old_magnitude_guard', [False, True],
                         ids=['actual-raw-byte', 'old-magnitude-guard'])
def test_actual_signed_offense_missile_guard(tmp_path, compiler, old_magnitude_guard):
    header = (ROOT/'src/veh.h').read_text()
    declarations = '\n'.join(_definition(header, 'enum '+name+' {')+';' for name in
                              ('VehTriad', 'VehBasic', 'VehPlan', 'VehWeaponMode'))
    declarations += '\n' + '\n'.join(_definition(header, 'struct '+name+' {')+';' for name in
                                       ('Veh', 'VehPrototype', 'RulesReactor'))
    source = (ROOT/'src/veh.cpp').read_text()
    body = _definition(source, 'int __cdecl offense_proto(')
    if old_magnitude_guard:
        anchor = 'is_proto_missile(proto_id) && off_rating_byte < 99'
        assert body.count(anchor) == 1
        body = body.replace(anchor, 'is_proto_missile(proto_id) && off_rating < 99')
    unit, binary = tmp_path/'signed-ratings.cpp', tmp_path/'signed-ratings'
    unit.write_text(PRELUDE+declarations+FIXTURE+body+SIGNED_RATING_CONTROL)
    subprocess.run([compiler, '-std=c++17', '-fsanitize=undefined',
                    '-fno-sanitize-recover=all', str(unit), '-o', str(binary)],
                   check=True, timeout=60)
    result = subprocess.run([str(binary)], capture_output=True, timeout=10)
    assert b'runtime error:' not in result.stderr
    if old_magnitude_guard:
        assert result.returncode == -signal.SIGABRT
        assert b'pin_guard_case rating=-128 missile=1 bombard=1 defender=-1 actual=128 expected=192' in result.stderr
        assert b'actual == expected' in result.stderr
    else:
        assert result.returncode == 0, result.stderr.decode(errors='replace')
