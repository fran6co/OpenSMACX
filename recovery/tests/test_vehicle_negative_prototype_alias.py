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
static int get_proto_offense_rating(int) { return -4; }
static int get_proto_defense_rating(int) { return -2; }
static int get_offense_rating(int id) { assert(id >= 0); return 4; }
static int get_defense_rating(int id) { assert(id >= 0); return 1; }
static int get_triad(int id) { assert(id >= 0); return TRIAD_SEA; }
static int get_proto_triad(int) { return TRIAD_AIR; }
static int is_proto_missile(int) { return 0; }
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
