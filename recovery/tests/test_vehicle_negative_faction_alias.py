"""Actual-source controls for the original empty-neighbor Reactor byte alias."""
from pathlib import Path
import shutil
import signal
import subprocess
import sys

import pytest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'recovery/tests'))
from test_movie_runtime_decoder_ownership_actual_source_oct9 import _definition

COMPILERS = [path for name in ('g++', 'clang++') if (path := shutil.which(name))]

PRELUDE = r'''
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>
#define __cdecl
typedef char *LPSTR;
'''

FIXTURE = r'''
static const int MaxVehProtoFactionNum = 64;
static Veh Vehs[3];
static VehPrototype VehPrototypes[512];
static RulesReactor Reactor[4];
struct ChassisRow { int triad; };
static ChassisRow Chassis[3];
struct WeaponRow { int offense_rating; };
static WeaponRow Weapon[2];
struct PlayerRow { int region_base_plan[2]; unsigned int flags; int proto_id_active[512]; };
static PlayerRow PlayersData[8];
struct Tile { int region; };
static Tile tile;
static int VehMoveDeltaX[8] = {1,2,3,4,5,6,7,8};
static int VehMoveDeltaY[8];
static std::vector<int> stack_ids;
static std::vector<int> events;
static int lookup_count;
static bool change_alias_after_lookup;
static int base_at(int,int) { assert(false); return -1; }
static Tile *map_loc(int,int) { return &tile; }
static int at_goal(int,int,int,int) { return 0; }
static int is_ocean(int,int) { return 0; }
static int is_proto_missile(int) { return 0; }
static int course_xrange(int x) { return x; }
static int on_map(int,int) { events.push_back(1); return 1; }
static int has_abil(int,int) { return 0; }
static int veh_at(int,int) {
    events.push_back(2);
    const int slot = lookup_count++;
    if (change_alias_after_lookup) Reactor[0].power = slot % 2 ? 0x0102 : 0x0201;
    return slot % 2 ? 2 : -1;
}
static int stack_check(int id, int type, int, int, int) {
    if (type == 3) { events.push_back(3); stack_ids.push_back(id); return 0; }
    if (type == 1 || type == 16) return 1;
    return 0;
}
'''

CONTROL = r'''
static void run_case(int triad, unsigned short power, int faction, bool live) {
    std::memset(Vehs,0,sizeof Vehs);
    std::memset(VehPrototypes,0,sizeof VehPrototypes);
    std::memset(PlayersData,0,sizeof PlayersData);
    std::memset(Reactor,0,sizeof Reactor);
    stack_ids.clear(); events.clear(); lookup_count = 0;
    change_alias_after_lookup = live;
    Reactor[0].power = power;
    Reactor[1].power = faction == 1 ? 2 : 1; // A different, valid storage target.
    Vehs[0].proto_id = 0;
    Vehs[1].proto_id = 1;
    Vehs[2].proto_id = 2;
    Vehs[2].faction_id = static_cast<uint8_t>(faction);
    VehPrototypes[0].chassis_id = 0;
    VehPrototypes[0].plan = PLAN_NAVAL_TRANSPORT;
    VehPrototypes[0].cost = 10;
    VehPrototypes[1].plan = PLAN_COMBAT;
    VehPrototypes[1].cost = 10;
    Chassis[0].triad = triad;
    const int result = compute_odds(10,faction,0,1,0);
    assert(result == 20);
    assert(lookup_count == 8);
    std::vector<int> expected_ids, expected_events;
    for (int slot = 0; slot != 8; ++slot) {
        expected_events.push_back(1); expected_events.push_back(2);
        const int id = slot % 2 ? 2 : -1;
        const int seen = id == -1 ? (live ? 1 : (power & 0xff)) : faction;
        if (seen == faction) {
            expected_ids.push_back(id); expected_events.push_back(3);
        }
    }
    assert(stack_ids == expected_ids);
    assert(events == expected_events);
    // Reactor value and all unrelated vehicle data retain their owner storage.
    assert(Reactor[1].power == (faction == 1 ? 2 : 1));
    assert(Vehs[2].faction_id == faction);
}
int main() {
    static_assert(sizeof(Veh) == 0x34, "actual vehicle stride");
    static_assert(offsetof(Veh,faction_id) == 0xe, "actual vehicle faction member");
    for (int triad = TRIAD_LAND; triad <= TRIAD_AIR; triad += 2) {
        run_case(triad,0x0201,1,false);
        run_case(triad,0x0201,2,false);
        run_case(triad,0x0102,2,false);
        run_case(triad,0x0102,1,false);
        run_case(triad,0,0,false); // Preserve the original call with -1, not a skip.
        run_case(triad,0x0102,1,true); // Read after each live lookup, not once before.
    }
}
'''


@pytest.mark.parametrize('compiler', COMPILERS)
@pytest.mark.parametrize('wrong_storage', [False, True], ids=['actual', 'wrong-owner-control'])
def test_actual_compute_odds_original_storage(tmp_path, compiler, wrong_storage):
    header = (ROOT / 'src/veh.h').read_text()
    faction = (ROOT / 'src/faction.h').read_text()
    enums = ('VehTriad', 'VehAbilitiesBitfield', 'VehPrototypeFlagBitfield',
             'VehOrders', 'VehFlagsBitfield', 'VehPlan')
    declarations = '\n'.join(_definition(header, 'enum ' + name + ' {') + ';' for name in enums)
    declarations += '\n' + _definition(faction, 'enum AiGoalTypes {') + ';'
    for name in ('Veh', 'VehPrototype', 'RulesReactor'):
        declarations += '\n' + _definition(header, 'struct ' + name + ' {') + ';'
    body = _definition((ROOT / 'src/veh.cpp').read_text(), 'int __cdecl compute_odds(')
    if wrong_storage:
        assert body.count('static_cast<uint8_t>(Reactor[0].power)') == 2
        body = body.replace('static_cast<uint8_t>(Reactor[0].power)',
                            'static_cast<uint8_t>(Reactor[1].power)')
    unit = tmp_path / 'odds.cpp'
    binary = tmp_path / 'odds'
    unit.write_text(PRELUDE + declarations + FIXTURE + body + CONTROL)
    subprocess.run([compiler,'-std=c++17',str(unit),'-o',str(binary)],check=True,timeout=60)
    result = subprocess.run([str(binary)],capture_output=True,timeout=10)
    if wrong_storage:
        assert result.returncode == -signal.SIGABRT
        assert b'stack_ids == expected_ids' in result.stderr
    else:
        assert result.returncode == 0, result.stderr.decode(errors='replace')
