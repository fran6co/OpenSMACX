"""Actual-source controls for Veh links and per-faction prototype counters."""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
VEH_SOURCE = ROOT / "src/veh.cpp"
BASE_SOURCE = ROOT / "src/base.cpp"
FACTION_HEADER = ROOT / "src/faction.h"


def _body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[opening + 1:end]
    raise AssertionError(f"unterminated function {signature}")


def _between(source: str, start: str, end: str) -> str:
    first = source.index(start)
    last = source.index(end, first)
    return source[first:last]


def _line(source: str, needle: str) -> str:
    return next(line.strip() for line in source.splitlines() if needle in line)


def _source_fragments():
    veh = VEH_SOURCE.read_text(encoding="utf-8")
    base = BASE_SOURCE.read_text(encoding="utf-8")
    stack = _body(veh, "void __cdecl stack_kill(int a1)")
    kill = _body(veh, "void __cdecl veh_kill(int veh_id)")
    upgrade = _body(base, "void __cdecl upgrade_prototypes(int faction_id, int proto_id)")
    do_upgrade = _body(base, "void __cdecl do_upgrade(int faction_id, int proto_id_new, int proto_id_old)")
    kill_counts = _between(
        kill,
        "    if (VehPrototypes[proto_id].plan <= 7) {",
        "\n\n    veh_lift(veh_id);",
    )
    reindex = _between(
        kill,
        "    if (veh_id < GameDataStateStorage.fields_.vehicle_current_count_ - 1) {",
        "\n\n    if (ConsoleGlobal.selected_vehicle_ == veh_id)",
    )
    transfer = "\n".join(
        _line(do_upgrade, needle)
        for needle in (
            "PlayersData[faction_id].proto_id_active[proto_id_old]--;",
            "uint8_t new_count = PlayersData[faction_id].proto_id_active[proto_id_new];",
            "++new_count;",
            "Vehs[i].proto_id = proto_id_new;",
            "PlayersData[faction_id].proto_id_active[proto_id_new] = new_count;",
        )
    )
    product = _line(do_upgrade, "int product = cost * PlayersData[faction_id].proto_id_active")
    assert "PlayersData[faction_id].proto_id_active[other] == 0" in upgrade
    return veh, base, stack, kill_counts, reindex, transfer, product


def test_sources_use_existing_vehicle_and_playerdata_owners():
    veh, base, stack, kill_counts, reindex, transfer, product = _source_fragments()
    faction = FACTION_HEADER.read_text(encoding="utf-8")
    assert "Vehs[a1].prev_veh_id_stack" in stack
    assert "Vehs[a1].next_veh_id_stack" in stack
    assert "g_vehLinks" not in veh and "struct VehLink" not in veh
    assert "PlayersData[faction_id].total_mil_veh" in kill_counts
    assert "PlayersData[faction_id].proto_id_active[proto_id]" in kill_counts
    assert "FactionAliveUnitCount" not in veh
    assert "FactionProtoSeenCount" not in veh
    assert "FactionProtoSeenCount" not in _body(base, "void __cdecl do_upgrade(")
    assert "PlayersData[faction_id].proto_id_active[other] == 0" in base
    assert "PlayersData[faction_id].proto_id_active[proto_id_old]" in product
    assert transfer.index("proto_id_active[proto_id_old]--") < transfer.index("new_count =")
    assert transfer.index("new_count =") < transfer.index("++new_count")
    assert transfer.index("++new_count") < transfer.index("Vehs[i].proto_id")
    assert transfer.index("Vehs[i].proto_id") < transfer.index("= new_count")
    assert "static_assert(sizeof(PlayerData) == 0x20CC" in faction
    assert "static_assert(offsetof(PlayerData, proto_id_active) == 0x858" in faction
    # The two original words are the real Veh fields, not an alias record.
    assert "int16_t next_veh_id_stack;" in (ROOT / "src/veh.h").read_text(encoding="utf-8")
    assert "int16_t prev_veh_id_stack;" in (ROOT / "src/veh.h").read_text(encoding="utf-8")


PLAYER_DATA_FIXTURE = r'''
struct PlayerData {
  unsigned char prefix[0x858];
  uint8_t proto_id_active[512];
  uint8_t proto_id_queue[512];
  int16_t proto_id_lost[512];
  int total_mil_veh;
  unsigned char tail[0x20CC - 0x105C];
};
static_assert(offsetof(PlayerData, proto_id_active) == 0x858, "byte row offset");
static_assert(offsetof(PlayerData, total_mil_veh) == 0x1058, "count dword offset");
static_assert(sizeof(PlayerData) == 0x20CC, "faction row stride");
static PlayerData PlayersData[8];
struct Prototype { int plan; };
static Prototype VehPrototypes[512];
'''


def _compile_run(tmp_path: Path, compiler: str, stack: str, kill_counts: str,
                 reindex: str, transfer: str, product: str):
    tmp_path.mkdir(parents=True, exist_ok=True)
    # Veh has its native 0x34 stride and signed link words at +0x30/+0x32.
    program = r'''#include <cassert>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstring>
using std::memcpy;
struct Veh {
  unsigned char original_id;
  unsigned char prefix[0x09];
  int16_t proto_id;
  unsigned char proto_tail[0x05];
  int8_t order;
  unsigned char waypoint_prefix[2];
  int16_t waypoint_x[4];
  unsigned char tail_prefix[0x14];
  int16_t next_veh_id_stack;
  int16_t prev_veh_id_stack;
};
static_assert(offsetof(Veh, order) == 0x11, "order offset");
static_assert(offsetof(Veh, waypoint_x) == 0x14, "waypoint offset");
static_assert(offsetof(Veh, next_veh_id_stack) == 0x30, "next link offset");
static_assert(offsetof(Veh, prev_veh_id_stack) == 0x32, "previous link offset");
static_assert(sizeof(Veh) == 0x34, "vehicle stride");
static Veh Vehs[16];
static int killed[16], killed_count;
static struct { struct { int vehicle_current_count_; } fields_; } GameDataStateStorage;
''' + PLAYER_DATA_FIXTURE + r'''
static void veh_kill(int veh_id) {
  killed[killed_count++] = static_cast<int>(Vehs[veh_id].original_id);
''' + reindex + r'''
}
static void stack_kill(int);
''' + "void stack_kill(int a1) {" + stack + "}\n" + r'''
static void source_kill_counts(int faction_id, int proto_id) {
''' + kill_counts + r'''
}
static int source_upgrade_product(int faction_id, int proto_id_old, int cost) {
  ''' + product + r'''
  return product;
}
static void source_transfer_counts(int faction_id, int proto_id_old, int proto_id_new, int i) {
''' + transfer + r'''
}
int main() {
  // Tail-to-head traversal followed by deletion and link-ID reindexing. The
  // selected stack is old IDs 0 -> 2 -> 4, with unrelated records between.
  GameDataStateStorage.fields_.vehicle_current_count_ = 5;
  for (int i=0; i<5; ++i) Vehs[i].original_id = static_cast<unsigned char>(100+i);
  for (int i=0; i<5; ++i) Vehs[i].next_veh_id_stack = Vehs[i].prev_veh_id_stack = -1;
  Vehs[0].next_veh_id_stack=2; Vehs[2].prev_veh_id_stack=0;
  Vehs[2].next_veh_id_stack=4; Vehs[4].prev_veh_id_stack=2;
  stack_kill(4);
  assert(killed_count==3 && killed[0]==100 && killed[1]==102 && killed[2]==104);
  assert(GameDataStateStorage.fields_.vehicle_current_count_==2);
  assert(Vehs[0].original_id==101 && Vehs[1].original_id==103);

  // The negative sentinel and a one-element stack take their original exits.
  killed_count=0; stack_kill(-1); assert(killed_count==0);
  GameDataStateStorage.fields_.vehicle_current_count_=1;
  Vehs[0].original_id=77; Vehs[0].next_veh_id_stack=-1; Vehs[0].prev_veh_id_stack=-1;
  stack_kill(0);
  assert(killed_count==1 && killed[0]==77);
  assert(GameDataStateStorage.fields_.vehicle_current_count_==0);

  // Byte and dword updates land in the requested nonzero PlayerData row.
  PlayersData[3].total_mil_veh=12; PlayersData[3].proto_id_active[9]=4;
  PlayersData[2].total_mil_veh=30; PlayersData[2].proto_id_active[9]=8;
  VehPrototypes[9].plan=7;
  source_kill_counts(3,9);
  assert(PlayersData[3].total_mil_veh==11 && PlayersData[3].proto_id_active[9]==3);
  assert(PlayersData[2].total_mil_veh==30 && PlayersData[2].proto_id_active[9]==8);
  PlayersData[3].total_mil_veh=INT_MIN;
  source_kill_counts(3,9);
  assert(PlayersData[3].total_mil_veh==INT_MAX);
  PlayersData[3].total_mil_veh=0;
  source_kill_counts(3,9);
  assert(PlayersData[3].total_mil_veh==-1);
  PlayersData[3].total_mil_veh=11;
  PlayersData[3].proto_id_active[9]=3;
  VehPrototypes[9].plan=8;
  source_kill_counts(3,9);
  assert(PlayersData[3].total_mil_veh==11 && PlayersData[3].proto_id_active[9]==2);

  PlayersData[3].proto_id_active[9]=4;
  assert(source_upgrade_product(3,9,7)==280);
  PlayersData[3].proto_id_active[10]=1;
  source_transfer_counts(3,9,10,0);
  assert(PlayersData[3].proto_id_active[9]==3 && PlayersData[3].proto_id_active[10]==2);
  assert(PlayersData[2].proto_id_active[9]==8);
  Vehs[0].proto_id=9;
  PlayersData[3].proto_id_active[9]=4;
  source_transfer_counts(3,9,9,0);
  assert(Vehs[0].proto_id==9 && PlayersData[3].proto_id_active[9]==4);
  PlayersData[3].proto_id_active[10]=255;
  source_transfer_counts(3,9,10,0);
  assert(Vehs[0].proto_id==10 && PlayersData[3].proto_id_active[10]==0);
}
'''
    cpp, exe = tmp_path / "vehicle_storage.cpp", tmp_path / "vehicle_storage"
    cpp.write_text(program, encoding="utf-8")
    built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                            str(cpp), "-o", str(exe)], capture_output=True,
                           text=True, timeout=60)
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_vehicle_storage_fragments_preserve_rows_and_stack_order(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host compiler unavailable")
    _, _, stack, kill_counts, reindex, transfer, product = _source_fragments()
    result = _compile_run(tmp_path, compiler, stack, kill_counts, reindex,
                          transfer, product)
    assert result.returncode == 0, result.stdout + result.stderr


def test_stack_link_and_faction_row_mutants_are_caught(tmp_path):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host compiler unavailable")
    _, _, stack, kill_counts, reindex, transfer, product = _source_fragments()
    bad_stack = stack.replace("Vehs[a1].prev_veh_id_stack", "Vehs[a1].next_veh_id_stack", 1)
    assert bad_stack != stack
    failed = _compile_run(tmp_path / "stack-mutant", compiler, bad_stack,
                          kill_counts, reindex, transfer, product)
    assert failed.returncode != 0

    bad_counts = kill_counts.replace(
        "PlayersData[faction_id].proto_id_active[proto_id]",
        "PlayersData[0].proto_id_active[proto_id]", 1)
    assert bad_counts != kill_counts
    failed = _compile_run(tmp_path / "counter-mutant", compiler, stack,
                          bad_counts, reindex, transfer, product)
    assert failed.returncode != 0
