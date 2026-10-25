"""Actual-source regression for base_first's queue and live faction reads.

The production definition is used unchanged. Two independently compiled
damage variants remove its initial queue publication or cache the faction
before the callback. This host control observes effects and selection; it is
not a native runtime or game-data reachability proof.
"""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/base.cpp").is_file())
SOURCE = ROOT / "src/base.cpp"
ADDRESS = 0x004E4AA0
SIGNATURE = "void __cdecl base_first(int base_id)"
EARLY_STORE = "    Bases[base_id].queue_production_id[0] = proto_id;\n"
LOOP = "    for (int i = 0; i < MaxVehProtoNum; i++) {"
LIVE_CALL = "veh_avail(i, Bases[base_id].faction_id_current, base_id)"


def _actual_body() -> str:
    from match_recovery.source import read_text
    from match_recovery.local_variants import _bound_function_body_bytes, _marker_identity

    text = SOURCE.read_text()
    records = read_text(text, SOURCE)
    record, = [row for row in records if row.address == ADDRESS]
    start, end = _bound_function_body_bytes(
        text, "base_first", SOURCE, record,
        tuple(_marker_identity(row) for row in records))
    return SIGNATURE + " " + text.encode()[start:end].decode()


PREAMBLE = r"""
#include <cstdint>
#include <cstring>
#define __cdecl
using std::int8_t;
using std::uint8_t;
using std::uint16_t;
using std::uint32_t;

enum { BSC_SCOUT_PATROL = 2, PROTO_TYPED_COMPLETE = 0x4 };
enum { PLAN_COMBAT = 1, PLAN_DEFENSIVE = 2, PLAN_RECONNAISANCE = 3 };
enum { TRIAD_LAND = 0, TRIAD_SEA = 1 };
static const int MaxVehProtoNum = 4;
static const int MaxVehProtoFactionNum = 2;

// The accessed members retain production widths/signs: an 8-bit base
// faction and signed queue IDs; prototype flags/plan/cost are 16/8/8-bit.
struct BaseForTest { uint8_t faction_id_current; int queue_production_id[10]; };
struct PrototypeForTest { uint16_t flags; uint8_t plan; uint8_t cost; };
BaseForTest Bases[1];
PrototypeForTest VehPrototypes[MaxVehProtoNum];
int rating[MaxVehProtoNum];
int triad[MaxVehProtoNum];
int availability_count;
int availability_ids[MaxVehProtoNum];
int availability_factions[MaxVehProtoNum];
int queue_during_first_availability;
int rating_count;
int rating_ids[MaxVehProtoNum];

int veh_avail(int proto_id, int faction_id, int base_id);
int get_proto_defense_rating(int proto_id);
int get_proto_triad(int proto_id);
"""

HARNESS = r"""
int veh_avail(int proto_id, int faction_id, int base_id) {
    const int call = availability_count++;
    availability_ids[call] = proto_id;
    availability_factions[call] = faction_id;
    if (call == 0) {
        queue_during_first_availability = Bases[base_id].queue_production_id[0];
        Bases[base_id].faction_id_current = 7;
    }
    if (proto_id == 0) return faction_id == 1;
    return faction_id == 7;
}

int get_proto_defense_rating(int proto_id) {
    rating_ids[rating_count++] = proto_id;
    return rating[proto_id];
}
int get_proto_triad(int proto_id) { return triad[proto_id]; }

void reset_fixture() {
    std::memset(Bases, 0, sizeof(Bases));
    std::memset(VehPrototypes, 0, sizeof(VehPrototypes));
    std::memset(rating, 0, sizeof(rating));
    std::memset(triad, 0, sizeof(triad));
    availability_count = rating_count = 0;
    queue_during_first_availability = -1;
    Bases[0].faction_id_current = 1;
    Bases[0].queue_production_id[0] = 99;

    // Score 1=47, 2=192, 3=192. The final >= rule selects 3, exercising
    // the combat/defensive/recon plan multipliers, nonland quartering,
    // costs, typed-prototype gate and later-wins tie behavior.
    rating[0] = 0;
    rating[1] = 2; VehPrototypes[1].plan = PLAN_COMBAT;
    triad[1] = TRIAD_SEA; VehPrototypes[1].cost = 1;
    rating[2] = 2; VehPrototypes[2].plan = PLAN_DEFENSIVE;
    VehPrototypes[2].flags = PROTO_TYPED_COMPLETE;
    triad[2] = TRIAD_LAND; VehPrototypes[2].cost = 64;
    rating[3] = 3; VehPrototypes[3].plan = PLAN_RECONNAISANCE;
    VehPrototypes[3].flags = PROTO_TYPED_COMPLETE;
    triad[3] = TRIAD_LAND; VehPrototypes[3].cost = 0;
}

int main() {
    reset_fixture();
    base_first(0);
    if (queue_during_first_availability != BSC_SCOUT_PATROL) return 11;
    if (Bases[0].faction_id_current != 7) return 12;
    if (availability_count != 4 || rating_count != 4) return 13;
    for (int i = 0; i < 4; ++i) {
        if (availability_ids[i] != i || rating_ids[i] != i) return 14;
        if (availability_factions[i] != (i == 0 ? 1 : 7)) return 15;
    }
    if (Bases[0].queue_production_id[0] != 3) return 16;
    return 0;
}
"""


def _build_run(compiler: str, body: str, directory: Path) -> subprocess.CompletedProcess:
    directory.mkdir(parents=True, exist_ok=True)
    source = directory / "base_first.cpp"
    executable = directory / "base_first"
    source.write_text(PREAMBLE + "\n" + body + "\n" + HARNESS,
                      encoding="utf-8")
    compile_result = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
         str(source), "-o", str(executable)],
        capture_output=True, text=True, check=False,
    )
    assert compile_result.returncode == 0, compile_result.stdout + compile_result.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True,
                          check=False)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_production_body_preserves_queue_publication_and_live_faction(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    production = _actual_body()
    result = _build_run(compiler, production, tmp_path / "production")
    assert result.returncode == 0, result.stdout + result.stderr

    # Damage 1: retain the final store but remove only the pre-callback store.
    no_early_store = production.replace(EARLY_STORE, "", 1)
    assert no_early_store != production
    early_damage = _build_run(compiler, no_early_store, tmp_path / "no_early_store")
    assert early_damage.returncode != 0

    # Damage 2: keep the early store but capture the 8-bit faction before the
    # first callback. The callback's owner change must affect the next query.
    snapshot = "    uint8_t faction_id = Bases[base_id].faction_id_current;\n"
    snapshot_mutant = production.replace(LOOP, snapshot + LOOP, 1)
    snapshot_mutant = snapshot_mutant.replace(
        LIVE_CALL, "veh_avail(i, faction_id, base_id)", 1)
    assert snapshot_mutant != production
    faction_damage = _build_run(compiler, snapshot_mutant,
                                tmp_path / "cached_faction")
    assert faction_damage.returncode != 0
