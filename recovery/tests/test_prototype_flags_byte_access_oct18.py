"""Actual-source controls for the two byte-width VehPrototype flag updates.

The host harness checks behavior and object-neighbor preservation. Pinned
instruction controls establish original operand widths. Neither is a
substitute for the coordinator's configured native compiler measurement.
"""

from __future__ import annotations

import os
from pathlib import Path
import re
import shutil
import subprocess

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32, x86_const
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage


ROOT = Path(__file__).resolve().parents[2]
GAME_SOURCE = ROOT / "src/game.cpp"
BASE_SOURCE = ROOT / "src/base.cpp"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def _definition(path: Path, signature: str) -> str:
    source = path.read_text(encoding="utf-8")
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError(f"unterminated definition {signature}")


def _body(definition: str) -> str:
    opening = definition.index("{")
    return definition[opening + 1:-1]


def _actual_prototype_type() -> str:
    source = (ROOT / "src/veh.h").read_text(encoding="utf-8")
    start = source.index("struct VehPrototype {")
    end = source.index("\n};", start) + 3
    return source[start:end]


def _reset_definition() -> str:
    source = GAME_SOURCE.read_text(encoding="utf-8")
    signature = "int __cdecl sub_52a980()"
    start = source.index(signature)
    marker = source.rfind("// ORIGINAL:", 0, start)
    assert source[marker:start].strip().startswith("// ORIGINAL: 0x0052A980")
    definition = _definition(GAME_SOURCE, signature)
    return definition


def _upgrade_definition() -> str:
    source = BASE_SOURCE.read_text(encoding="utf-8")
    signature = "void __cdecl upgrade_prototypes(int faction_id, int proto_id)"
    start = source.index(signature)
    marker = source.rfind("// ORIGINAL:", 0, start)
    assert source[marker:start].strip().startswith("// ORIGINAL: 0x004F0460")
    definition = _definition(BASE_SOURCE, signature)
    return definition


DECLARATIONS = r'''#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <tuple>
#include <vector>
#define __cdecl
'''


RESET_SUPPORT = r'''
static_assert(sizeof(VehPrototype) == 0x34, "native prototype stride");
static_assert(offsetof(VehPrototype, flags) == 0x30, "flags offset");
static_assert(offsetof(VehPrototype, preq_tech) == 0x32, "prerequisite offset");
static_assert(offsetof(VehPrototype, preq_tech) == offsetof(VehPrototype, flags) + 2,
              "neighboring field is distinct");
constexpr int MaxVehProtoNum = 512;
VehPrototype VehPrototypes[MaxVehProtoNum];
struct GameDataStateFields { int vehicle_current_count_, base_current_count_; };
struct GameDataState { GameDataStateFields fields_; };
GameDataState GameDataStateStorage;
'''


UPGRADE_SUPPORT = r'''
static_assert(sizeof(VehPrototype) == 0x34, "native prototype stride");
static_assert(offsetof(VehPrototype, flags) == 0x30, "flags offset");
static_assert(offsetof(VehPrototype, preq_tech) == 0x32, "prerequisite offset");
VehPrototype VehPrototypes[512];
struct PlayerData { uint8_t proto_id_active[512]; };
PlayerData PlayersData[8];
struct ChassisRow { int triad; int speed; } Chassis[256];
struct WeaponRow { int8_t offense_rating; } Weapon[256];
struct ArmorRow { int defense_rating; } Armor[256];
enum {
  ABL_POLICE_2X = 1,
  ABL_ARTILLERY = 2,
  ABL_ANTIGRAV_STRUTS = 4,
  ABL_DROP_POD = 8,
  ABL_AMPHIBIOUS = 16
};
int ability_queries[512];
std::vector<std::tuple<int, int, int, int, uint16_t>> upgrades;
int has_abil(int prototype_id, int) {
  ++ability_queries[prototype_id];
  return 0;
}
void upgrade_prototype(int faction_id, int old_id, int new_id, int mode) {
  upgrades.emplace_back(faction_id, old_id, new_id, mode,
                        VehPrototypes[old_id].flags);
}
'''


def _compile_run(compiler: str, source: str, tmp_path: Path,
                 optimization: str = "-O0") -> subprocess.CompletedProcess:
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} is unavailable")
    tmp_path.mkdir(parents=True, exist_ok=True)
    cpp = tmp_path / "actual_prototype_flags.cpp"
    binary = tmp_path / "actual_prototype_flags"
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [executable, "-std=c++17", "-Wall", "-Wextra", "-Werror",
         "-Wno-unused-variable", optimization, str(cpp), "-o", str(binary)],
        capture_output=True, text=True, timeout=45,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=45)


def _require_success(compiler: str, source: str, tmp_path: Path,
                     optimization: str) -> None:
    ran = _compile_run(compiler, source, tmp_path, optimization)
    assert ran.returncode == 0, ran.stdout + ran.stderr


def _require_mutant_rejected(compiler: str, source: str, tmp_path: Path,
                             optimization: str) -> None:
    ran = _compile_run(compiler, source, tmp_path, optimization)
    assert ran.returncode != 0, "damage control accepted a deliberate source mutant"


def test_actual_prototype_declaration_keeps_flags_word_and_neighbor() -> None:
    declaration = _actual_prototype_type()
    assert "uint16_t flags;" in declaration
    assert "int16_t preq_tech;" in declaration
    assert declaration.index("uint16_t flags;") < declaration.index("int16_t preq_tech;")


def _reset_program(definition: str) -> str:
    return DECLARATIONS + _actual_prototype_type() + RESET_SUPPORT + definition + r'''
int main() {
    unsigned char expected[MaxVehProtoNum][sizeof(VehPrototype)];
    for (int i = 0; i < MaxVehProtoNum; ++i) {
        std::memset(&VehPrototypes[i], 0xA5, sizeof(VehPrototype));
        VehPrototypes[i].flags = static_cast<uint16_t>(0xA503u ^ (i * 13u));
        const uint16_t tech = static_cast<uint16_t>(0xD000u ^ (i * 31u));
        std::memcpy(&VehPrototypes[i].preq_tech, &tech, sizeof(tech));
        VehPrototypes[i].icon_offset = static_cast<int8_t>(i & 0x7F);
        std::memcpy(expected[i], &VehPrototypes[i], sizeof(VehPrototype));
        expected[i][offsetof(VehPrototype, flags)] &= 0xFE;
        expected[i][offsetof(VehPrototype, icon_offset)] = 0xFF;
    }
    GameDataStateStorage.fields_.vehicle_current_count_ = 1234;
    GameDataStateStorage.fields_.base_current_count_ = 4321;
    assert(sub_52a980() == 0);
    assert(GameDataStateStorage.fields_.vehicle_current_count_ == 0);
    assert(GameDataStateStorage.fields_.base_current_count_ == 0);
    for (int i = 0; i < MaxVehProtoNum; ++i) {
        assert(std::memcmp(&VehPrototypes[i], expected[i], sizeof(VehPrototype)) == 0);
    }
    return 0;
}
'''


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("optimization", ["-O0", "-O2"])
def test_actual_low_byte_reset_preserves_all_512_rows_and_neighbor_bytes(
        compiler, optimization, tmp_path):
    _require_success(compiler, _reset_program(_reset_definition()), tmp_path, optimization)


def _upgrade_program(definition: str) -> str:
    return DECLARATIONS + _actual_prototype_type() + UPGRADE_SUPPORT + definition + r'''
int main() {
    uint8_t before[512][sizeof(VehPrototype)];
    std::memset(VehPrototypes, 0, sizeof(VehPrototypes));
    std::memset(PlayersData, 0, sizeof(PlayersData));
    VehPrototype &old_proto = VehPrototypes[1];
    VehPrototype &replacement = VehPrototypes[129];
    old_proto.flags = 0xABFF;
    old_proto.preq_tech = static_cast<int16_t>(0x5A6B);
    old_proto.plan = 3;
    old_proto.unk_1 = -1;
    old_proto.chassis_id = 1;
    old_proto.weapon_id = 0;
    old_proto.armor_id = 0;
    replacement.flags = 1;
    replacement.obsolete_factions = 1u << 2;
    replacement.plan = old_proto.plan;
    replacement.unk_1 = old_proto.unk_1;
    replacement.chassis_id = old_proto.chassis_id;
    replacement.weapon_id = old_proto.weapon_id;
    replacement.armor_id = old_proto.armor_id;
    PlayersData[2].proto_id_active[129] = 1;
    Chassis[1].triad = 0;
    Chassis[1].speed = 2;
    Weapon[0].offense_rating = 0;
    Armor[0].defense_rating = 3;
    for (int i = 0; i < 512; ++i) {
        std::memcpy(before[i], &VehPrototypes[i], sizeof(VehPrototype));
    }

    // This test runs on little-endian hosts, as does the pinned x86 target.
    const uint16_t endian_probe = 1;
    assert(*reinterpret_cast<const unsigned char *>(&endian_probe) == 1);
    upgrade_prototypes(2, 1);

    assert(old_proto.flags == 0xAAFF);
    assert(old_proto.preq_tech == static_cast<int16_t>(0x5A6B));
    assert(ability_queries[1] == 1 && ability_queries[129] == 1);
    assert(upgrades.size() == 1);
    assert(std::get<0>(upgrades[0]) == 2);
    assert(std::get<1>(upgrades[0]) == 1);
    assert(std::get<2>(upgrades[0]) == 129);
    assert(std::get<3>(upgrades[0]) == 1);
    assert(std::get<4>(upgrades[0]) == 0xAAFF); // clear precedes the callback

    for (int i = 0; i < 512; ++i) {
        unsigned char expected[sizeof(VehPrototype)];
        std::memcpy(expected, before[i], sizeof(expected));
        if (i == 1) expected[offsetof(VehPrototype, flags) + 1] &= 0xFE;
        assert(std::memcmp(&VehPrototypes[i], expected, sizeof(expected)) == 0);
    }
    return 0;
}
'''


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("optimization", ["-O0", "-O2"])
def test_actual_upgrade_clears_high_flag_byte_before_scan_and_preserves_neighbors(
        compiler, optimization, tmp_path):
    _require_success(compiler, _upgrade_program(_upgrade_definition()), tmp_path, optimization)


def _replace_byte_access(body: str, index: int, statement: str) -> str:
    # Both positive masks are exactly byte-equivalent on the supported x86
    # target; the configured native compiler decides which one emits the pin.
    pattern = re.compile(
        rf"flags_bytes\s*\[\s*{index}\s*\]\s*&=\s*"
        rf"(?:0x0*FE[uU]?|~\s*1[uU]?)\s*;"
    )
    changed, count = pattern.subn(statement, body, count=1)
    assert count == 1, f"expected one byte-{index} flags access in actual source body"
    return changed


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("optimization", ["-O0", "-O2"])
@pytest.mark.parametrize("mutation", ["wrong-bit", "other-flag-byte", "full-word-zero"])
def test_low_byte_reset_damage_controls(compiler, optimization, mutation, tmp_path):
    body = _body(_reset_definition())
    if mutation == "wrong-bit":
        body = _replace_byte_access(body, 0, "flags_bytes[0] &= 0xFD;")
    elif mutation == "other-flag-byte":
        body = _replace_byte_access(body, 0, "flags_bytes[1] &= 0xFE;")
    else:
        body = _replace_byte_access(body, 0, "prototype.flags = 0;")
    definition = _reset_definition().split("{", 1)[0] + "{" + body + "}"
    _require_mutant_rejected(compiler, _reset_program(definition), tmp_path, optimization)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("optimization", ["-O0", "-O2"])
@pytest.mark.parametrize("mutation", ["wrong-bit", "low-byte", "neighbor-byte", "full-word-zero"])
def test_high_byte_upgrade_damage_controls(compiler, optimization, mutation, tmp_path):
    body = _body(_upgrade_definition())
    if mutation == "wrong-bit":
        body = _replace_byte_access(body, 1, "flags_bytes[1] &= 0xFD;")
    elif mutation == "low-byte":
        body = _replace_byte_access(body, 1, "flags_bytes[0] &= 0xFE;")
    elif mutation == "neighbor-byte":
        body = _replace_byte_access(body, 1, "VehPrototypes[proto_id].preq_tech = 0;")
    else:
        body = _replace_byte_access(body, 1, "VehPrototypes[proto_id].flags = 0;")
    definition = _upgrade_definition().split("{", 1)[0] + "{" + body + "}"
    _require_mutant_rejected(compiler, _upgrade_program(definition), tmp_path, optimization)


def _image_path() -> Path | None:
    configured = os.environ.get("OPENSMACX_IMAGE")
    if configured:
        path = Path(configured).resolve()
        if path.is_file():
            return path
    for parent in (ROOT, *ROOT.parents):
        path = parent / ".opensmacx/game/terranx_original.exe"
        if path.is_file():
            return path
    return None


def _decode_exact(image: PeImage, start: int, end: int):
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    decoded = list(decoder.disasm(image.read(Span(start, end)), start))
    assert sum(instruction.size for instruction in decoded) == end - start
    assert decoded and decoded[0].address == start
    assert decoded[-1].address + decoded[-1].size == end
    return {instruction.address: instruction for instruction in decoded}


def _assert_byte_and(instruction, *, base: int | None, displacement: int,
                     immediate: int) -> None:
    assert instruction.mnemonic == "and"
    assert len(instruction.operands) == 2
    memory, value = instruction.operands
    assert memory.type == x86_const.X86_OP_MEM
    assert memory.size == 1
    assert memory.mem.base == (base or 0)
    assert memory.mem.index == 0
    assert memory.mem.disp == displacement
    assert value.type == x86_const.X86_OP_IMM
    assert value.imm == immediate


def test_pinned_original_has_exact_low_and_high_byte_flag_operands() -> None:
    image_path = _image_path()
    if image_path is None:
        pytest.skip("pinned original unavailable for instruction-width evidence")
    image = PeImage(image_path.read_bytes(), PIN)
    low = _decode_exact(image, 0x0052A980, 0x0052A9A5)
    _assert_byte_and(low[0x0052A988], base=x86_const.X86_REG_EAX,
                     displacement=0, immediate=0xFE)
    assert (low[0x0052A98B].mnemonic, low[0x0052A98B].operands[0].size) == ("mov", 1)

    high = _decode_exact(image, 0x004F0460, 0x004F064F)
    _assert_byte_and(high[0x004F047B], base=x86_const.X86_REG_EDX,
                     displacement=0x009AB899, immediate=0xFE)


# do_upgrade's original low-byte clear preserves all eight upper flag bits.
# The real complete product body runs here; service records below deliberately
# model only called APIs/fields, not original Base/Vehicle/Console ABI layouts.
def _do_upgrade_definition() -> str:
    return _definition(BASE_SOURCE,
                       "void __cdecl do_upgrade(int faction_id, int proto_id_new, int proto_id_old)")


def _do_upgrade_program(definition: str) -> str:
    return DECLARATIONS + _actual_prototype_type() + r'''
static_assert(sizeof(VehPrototype) == 0x34, "prototype stride");
static_assert(offsetof(VehPrototype, flags) == 0x30, "flags offset");
static_assert(offsetof(VehPrototype, preq_tech) == 0x32, "neighbor offset");
VehPrototype VehPrototypes[512];
struct PlayerService { int energy_reserves; uint8_t proto_id_active[512]; };
PlayerService PlayersData[8];
struct VehicleService { uint8_t faction_id; int16_t proto_id; int8_t morale; };
VehicleService Vehs[1];
struct Base { int faction_id_current; int queue_size; int queue_production_id[10]; };
Base Bases[1];
struct LockBytes { uint8_t human_mask_; };
struct LockMasks { LockBytes bytes_; };
struct GameFields { int vehicle_current_count_, base_current_count_; LockMasks lock_masks_; };
struct GameState { GameFields fields_; } GameDataStateStorage;
struct ConsoleService {
    int faction_id_;
    int updates;
    void update_data(int mode) { assert(mode == 0); ++updates; }
} ConsoleGlobal;
const int ABL_TRAINED = 1;
int upgrade_cost(int, int, int) { return 3; }
int has_abil(int, int) { return 0; }
int draws;
void draw_map(int mode) { assert(mode == 1); ++draws; }
''' + definition + r'''
int main() {
    std::memset(VehPrototypes, 0xA5, sizeof(VehPrototypes));
    std::memset(PlayersData, 0, sizeof(PlayersData));
    ConsoleGlobal.faction_id_ = -1;
    unsigned char before[512][sizeof(VehPrototype)];
    std::memcpy(before, VehPrototypes, sizeof(before));
    for (unsigned value = 0; value < 65536; ++value) {
        VehPrototypes[64].flags = static_cast<uint16_t>(value);
        unsigned char expected[sizeof(VehPrototype)];
        std::memcpy(expected, &VehPrototypes[64], sizeof(expected));
        expected[offsetof(VehPrototype, flags)] &= 0xFE;
        do_upgrade(2, 65, 64);
        assert(VehPrototypes[64].flags == (value & 0xFFFEu));
        assert(std::memcmp(expected, &VehPrototypes[64], sizeof(expected)) == 0);
    }
    for (int i = 0; i < 512; ++i) {
        if (i != 64) assert(std::memcmp(before[i], &VehPrototypes[i], sizeof(VehPrototype)) == 0);
    }
    // Original old-id cutoff is exclusive: row63 remains completely unchanged.
    VehPrototypes[63].flags = 0xABFF;
    unsigned char fixed_row[sizeof(VehPrototype)];
    std::memcpy(fixed_row, &VehPrototypes[63], sizeof(fixed_row));
    do_upgrade(2, 65, 63);
    assert(std::memcmp(fixed_row, &VehPrototypes[63], sizeof(fixed_row)) == 0);
    // Nonzero paid upgrade sets only bit2 on the new row before the old clear.
    VehPrototypes[64].flags = 0xABFF;
    VehPrototypes[65].flags = 0xABF0;
    PlayersData[2].proto_id_active[64] = 1;
    PlayersData[2].energy_reserves = 100;
    GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ = 1u << 2;
    ConsoleGlobal.faction_id_ = 2;
    do_upgrade(2, 65, 64);
    assert(VehPrototypes[64].flags == 0xABFE);
    assert(VehPrototypes[65].flags == 0xABF4);
    assert(VehPrototypes[64].preq_tech == static_cast<int16_t>(0xA5A5));
    assert(VehPrototypes[65].preq_tech == static_cast<int16_t>(0xA5A5));
    assert(PlayersData[2].energy_reserves == 70);
    assert(draws == 1 && ConsoleGlobal.updates == 1);
}
'''


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("optimization", ["-O0", "-O2"])
def test_actual_do_upgrade_low_clear_preserves_every_word_value_and_neighbors(
        compiler, optimization, tmp_path):
    _require_success(compiler, _do_upgrade_program(_do_upgrade_definition()),
                     tmp_path, optimization)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("optimization", ["-O0", "-O2"])
def test_do_upgrade_old_word_mask_loses_poisoned_upper_bits(
        compiler, optimization, tmp_path):
    definition = _do_upgrade_definition()
    pattern = re.compile(r"^.*VehPrototypes\[proto_id_old\]\.flags.*&=.*;$", re.MULTILINE)
    if "flags_bytes[0]" in definition:
        mutant = _replace_byte_access(
            definition, 0, "VehPrototypes[proto_id_old].flags &= 0xFE;")
    else:
        mutant, count = pattern.subn("        VehPrototypes[proto_id_old].flags &= 0xFE;",
                                     definition)
        assert count == 1
    _require_mutant_rejected(compiler, _do_upgrade_program(mutant), tmp_path,
                            optimization)
