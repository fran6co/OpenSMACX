"""Focused host controls for the recovered signed vehicle arithmetic."""

from __future__ import annotations

import hashlib
import os
from pathlib import Path
import re
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
BEST_SPAN = (0x5044D0, 0x504A9B)
SPEED_SPAN = (0x5C1540, 0x5C1752)


def _definition(text: str, signature: str) -> str:
    match = re.search(signature, text)
    assert match, f"definition not found: {signature}"
    opening = text.index("{", match.start())
    depth = 0
    for end in range(opening, len(text)):
        if text[end] == "{":
            depth += 1
        elif text[end] == "}":
            depth -= 1
            if depth == 0:
                return text[match.start():end + 1]
    raise AssertionError(f"unterminated definition: {signature}")


def _source_bodies() -> tuple[str, str]:
    source = (ROOT / "src/veh.cpp").read_text(encoding="utf-8")
    return (
        _definition(source, r"int\s+__cdecl\s+best_defender\s*\("),
        _definition(source, r"int\s+__cdecl\s+speed\s*\("),
    )


def _road_rate_type() -> str:
    header = (ROOT / "src/alpha.h").read_text(encoding="utf-8")
    owner = re.search(r"struct\s+RulesBasic\s*\{(?P<body>.*?)\n\};", header, re.S)
    assert owner, "RulesBasic declaration not found"
    member = re.search(r"^\s*(int|uint32_t)\s+move_rate_roads\s*;",
                       owner.group("body"), re.M)
    assert member, "RulesBasic::move_rate_roads declaration not recognized"
    return member.group(1)


SUPPORT = r'''
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <type_traits>
#include <vector>
#define __cdecl
using BOOL = int;
enum { FALSE = 0, TRUE = 1, TRIAD_LAND = 0, TRIAD_SEA = 1, TRIAD_AIR = 2,
       PLAN_INFO_WARFARE = 40, PLAN_COLONIZATION = 1, PLAN_TERRAFORMING = 2,
       PLAN_ALIEN_ARTIFACT = 12, BSC_FUNGAL_TOWER = 19,
       SP_MARITIME_CONTROL_CENTER = 1, MORALE_ELITE = 6,
       ABL_AIR_SUPERIORITY = 1, ABL_CARRIER = 2, BIT_AIRBASE = 4,
       VSTATE_DESIGNATE_DEFENDER = 1, MaxVehProtoFactionNum = 64 };
struct Veh {
    int x = 0, y = 0;
    uint32_t proto_id = 0;
    int dmg_incurred = 0;
    int prev_veh_id_stack = -1, next_veh_id_stack = -1;
    uint32_t state = 0;
    int faction_id = 0;
};
struct VehPrototype { int plan = PLAN_COLONIZATION, reactor_id = 1; uint8_t triad = TRIAD_LAND; };
using RoadRateScalar = __ROAD_RATE_SCALAR__;
__ASSERT_SIGNED_TYPE__
std::vector<std::string> events;
static void event(const char *name) { events.emplace_back(name); }
struct RoadRate {
    int value = 2;
    int reads = 0;
    operator RoadRateScalar() { ++reads; event("road"); return static_cast<RoadRateScalar>(value); }
};
struct RuleSet { RoadRate move_rate_roads; };
Veh Vehs[8];
VehPrototype VehPrototypes[128];
RuleSet Rules;
int battle_defense[8], battle_offense_output[8];
int basic_offense = 80;
int speed_value = 6, morale_result = 0, morale_calls = 0;
int mutate_road_rate = 0;
int get_basic_offense(int, int, BOOL, BOOL, BOOL) { return basic_offense; }
int is_ocean(int, int) { return 0; }
int base_at(int, int) { return 0; }
int get_proto_triad(uint32_t p) { return VehPrototypes[p].triad; }
int get_triad(int) { return TRIAD_LAND; }
int can_arty(uint32_t, BOOL) { return 0; }
int has_abil(uint32_t, int) { return 0; }
int is_proto_missile(uint32_t) { return 0; }
int is_missile(int) { return 0; }
int get_offense_rating(int) { return 1; }
int get_defense_rating(int) { return 1; }
int get_proto_offense_rating(uint32_t) { return 0; }
int stack_check(int, int, int, int, int) { return 0; }
int bit_at(int, int) { return 0; }
int proto_power(int) { return 1; }
int range(int value, int low, int high) { return std::max(low, std::min(value, high)); }
void battle_compute(int, int defender, int *offense, int *defense, uint32_t) {
    *offense = battle_offense_output[defender];
    *defense = battle_defense[defender];
}
int speed_proto(int) { event("speed_proto"); return speed_value; }
int has_project(int, int) { event("project"); return 1; }
int morale_veh(int, BOOL, int) {
    event("morale"); ++morale_calls;
    if (mutate_road_rate) Rules.move_rate_roads.value = mutate_road_rate;
    return morale_result;
}
'''


def _compile(tmp_path: Path, best: str, speed: str, road_type: str,
             require_signed_type: bool = True) -> Path:
    compiler = os.environ.get("CXX", "c++")
    source = tmp_path / "veh_signed.cpp"
    binary = tmp_path / "veh_signed"
    main = r'''
int best_defender(int, int, BOOL);
int speed(int, BOOL);
static void reset() {
    for (auto &v : Vehs) v = Veh{};
    for (auto &p : VehPrototypes) p = VehPrototype{};
    for (auto &v : battle_defense) v = 0;
    for (auto &v : battle_offense_output) v = 1;
    Rules.move_rate_roads.value = 2; Rules.move_rate_roads.reads = 0;
    basic_offense = 80; speed_value = 6; morale_result = 0;
    morale_calls = 0; mutate_road_rate = 0; events.clear();
}
static void signed_defender_case() {
    reset();
    Vehs[0].proto_id = 0; Vehs[0].next_veh_id_stack = 1;
    Vehs[1].proto_id = 1; Vehs[1].next_veh_id_stack = -1;
    Vehs[2].proto_id = 2;
    VehPrototypes[0].triad = VehPrototypes[1].triad = TRIAD_LAND;
    battle_defense[0] = 1; battle_offense_output[0] = 10;
    battle_defense[1] = -1; battle_offense_output[1] = -1;
    // Candidate zero scores one; signed candidate one scores ten. With the
    // original unsigned coercion candidate one's computed modifier is zero.
    assert(best_defender(0, 2, TRUE) == 1);
}
static void speed_live_read_case() {
    reset();
    Vehs[0].proto_id = 1; Vehs[0].dmg_incurred = 1;
    VehPrototypes[1].triad = TRIAD_SEA;
    speed_value = 6; morale_result = MORALE_ELITE; mutate_road_rate = 5;
    assert(speed(0, FALSE) == 15);
    assert(Rules.move_rate_roads.reads == 4);
    assert((events == std::vector<std::string>{
        "speed_proto", "project", "road", "morale", "road", "road", "road"}));
    assert(morale_calls == 1);
}
static void fungal_late_result_case() {
    reset();
    Vehs[0].proto_id = BSC_FUNGAL_TOWER; Vehs[0].dmg_incurred = 1;
    VehPrototypes[BSC_FUNGAL_TOWER].triad = TRIAD_LAND;
    speed_value = 0; morale_result = MORALE_ELITE;
    assert(speed(0, TRUE) == 0);
    assert(morale_calls == 1);
    assert((events == std::vector<std::string>{
        "speed_proto", "morale", "road", "road"}));
}
static void negative_speed_division_case() {
    reset();
    Vehs[0].proto_id = 1; Vehs[0].dmg_incurred = 1;
    VehPrototypes[1].triad = TRIAD_LAND;
    speed_value = -1; // bounded: divisor 2, no signed overflow or zero divisor
    assert(speed(0, TRUE) == 2);
}
static void negative_road_rate_case() {
    reset();
    Vehs[0].proto_id = 1; Vehs[0].dmg_incurred = 1;
    VehPrototypes[1].triad = TRIAD_LAND;
    Rules.move_rate_roads.value = -2; speed_value = -6;
    assert(speed(0, TRUE) == -6);
}
int main(int argc, char **argv) {
    if (argc > 1 && std::string(argv[1]) == "--negative-road") {
        negative_road_rate_case();
        return 0;
    }
    signed_defender_case();
    speed_live_read_case();
    fungal_late_result_case();
    negative_speed_division_case();
    negative_road_rate_case();
}
'''
    signed_assertion = ("static_assert(std::is_signed<RoadRateScalar>::value, "
                        "\"the pinned road-rate field must preserve signed values\");"
                        if require_signed_type else "")
    support = SUPPORT.replace("__ROAD_RATE_SCALAR__", road_type)
    support = support.replace("__ASSERT_SIGNED_TYPE__", signed_assertion)
    source.write_text(support + "\n" + best + "\n" + speed + "\n" + main,
                      encoding="utf-8")
    subprocess.run([compiler, "-std=c++17", "-O0", str(source), "-o", str(binary)],
                   check=True, capture_output=True, text=True)
    return binary


def test_actual_vehicle_bodies_preserve_signed_and_late_effects(tmp_path: Path):
    best, speed = _source_bodies()
    binary = _compile(tmp_path, best, speed, _road_rate_type())
    subprocess.run([str(binary)], check=True, capture_output=True, text=True)


@pytest.mark.parametrize("mutation", ["early_fungal", "unsigned_divisions"])
def test_damage_controls_reject_early_or_unsigned_mutants(tmp_path: Path, mutation: str):
    best, speed = _source_bodies()
    if mutation == "early_fungal":
        speed = speed.replace(
            "    int speed_val = speed_proto(proto_id);",
            "    if (proto_id == BSC_FUNGAL_TOWER) return 0;\n"
            "    int speed_val = speed_proto(proto_id);",
            1,
        )
    else:
        best = best.replace("int proto_pwr_def = proto_power(i);",
                            "uint32_t proto_pwr_def = proto_power(i);", 1)
        speed = speed.replace("int speed_val = speed_proto(proto_id);",
                              "uint32_t speed_val = speed_proto(proto_id);", 1)
        speed = speed.replace("int moves = speed_val / Rules.move_rate_roads;",
                              "uint32_t moves = speed_val / static_cast<uint32_t>(Rules.move_rate_roads);", 1)
        speed = speed.replace("int reactor_fac;", "uint32_t reactor_fac;", 1)
    binary = _compile(tmp_path, best, speed, _road_rate_type())
    result = subprocess.run([str(binary)], capture_output=True, text=True)
    assert result.returncode != 0


def test_unsigned_road_rate_storage_mutant_changes_negative_route(tmp_path: Path):
    best, speed = _source_bodies()
    binary = _compile(tmp_path, best, speed, "uint32_t", require_signed_type=False)
    result = subprocess.run([str(binary), "--negative-road"], capture_output=True, text=True)
    assert result.returncode != 0
    assert "speed(0, TRUE) == -6" in result.stderr


def test_pinned_original_uses_signed_divides_and_late_fungal_branch():
    image_path = Path(os.environ.get(
        "OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not image_path.exists():
        pytest.skip("pinned image unavailable; set OPENSMACX_IMAGE")
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage

    raw = image_path.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == PIN
    image = PeImage(raw, PIN)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoded = {}
    for start, end in (BEST_SPAN, SPEED_SPAN):
        decoded.update({ins.address: ins for ins in decoder.disasm(
            image.read(Span(start, end)), start)})
    assert decoded[0x5047F7].mnemonic == "idiv"
    assert decoded[0x504813].mnemonic == "idiv"
    assert decoded[0x50481E].mnemonic == "sar"
    assert decoded[0x5C16A1].mnemonic == "cdq"
    assert decoded[0x5C16A2].mnemonic == "idiv"
    assert "0x949738" in decoded[0x5C16A2].op_str
    assert decoded[0x5C1705].mnemonic == "cdq"
    assert decoded[0x5C1706].mnemonic == "idiv"
    assert decoded[0x5C15CB].mnemonic == "call"
    assert "0x5c0e40" in decoded[0x5C15CB].op_str
    assert decoded[0x5C15D3].mnemonic == "cmp"
    assert decoded[0x5C15D8].mnemonic == "mov"  # skip_morale read follows callback
    assert decoded[0x5C173B].mnemonic == "mov"  # fungal prototype check is at tail
    assert decoded[0x5C1743].mnemonic == "sub" and decoded[0x5C1743].op_str.endswith("0x13")
    assert decoded[0x5C1748].mnemonic == "neg"
    assert decoded[0x5C174B].mnemonic == "sbb"
    assert decoded[0x5C174D].mnemonic == "and"
    assert decoded[0x5C1751].mnemonic == "ret"
    branch = {ins.address: ins for ins in decoder.disasm(
        image.read(Span(0x507410, 0x507480)), 0x507410)}
    assert branch[0x507455].mnemonic == "mov"
    assert "0x949738" in branch[0x507455].op_str
    assert branch[0x50745B].mnemonic == "test"
    assert branch[0x50745D].mnemonic == "jl"
    assert "0x50746b" in branch[0x50745D].op_str
