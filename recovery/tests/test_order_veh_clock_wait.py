"""Keep order_veh's cooperative delay distinct from vehicle-state sleep."""

from pathlib import Path
import os
import shutil
import subprocess

import pytest

from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]
SIGNATURE = "static void order_veh_combat_and_turn_prelude("


def _function_body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[opening + 1:end]
    raise AssertionError(f"unterminated function: {signature}")


def _host_program(body: str) -> str:
    return r'''#include <cassert>
#include <cstdint>
struct Veh { int proto_id; int dmg_incurred; };
struct WeaponData { int offense_rating; };
struct Prototype { int weapon_id; };
struct Map { unsigned bit; unsigned climate; };
struct ConsoleData {
  int faction_id_;
  int ui_state_;
  int vehicle_order_ui_state() const { return ui_state_; }
};
struct StateFields {
  struct { unsigned enable_mask_; } lock_masks_;
  unsigned game_state_;
};
struct { StateFields fields_; } GameDataStateStorage;
static Veh Vehs[128];
static WeaponData Weapon[2];
static Prototype VehPrototypes[2];
static ConsoleData ConsoleGlobal;
static int IsMultiplayerNet;
static int shift_state;
static int clock_delays[4], clock_count, sleep_count, spots, demotions;
static constexpr unsigned BIT_FUNGUS = 1u;
static constexpr unsigned STATE_OMNISCIENT_VIEW = 2u;
static constexpr int VK_SHIFT = 0x10;
int GetAsyncKeyState(int key) { assert(key == VK_SHIFT); return shift_state; }
void clock_wait(int milliseconds) { clock_delays[clock_count++] = milliseconds; }
void sleep(int vehicle_id) {
  assert(vehicle_id >= 0);
  ++sleep_count;
  Vehs[vehicle_id].dmg_incurred = -999; // the distinct state-changing operation
}
void veh_demote(int) { ++demotions; }
void spot_all(int, int) { ++spots; }
void order_veh_combat_and_turn_prelude(
    int veh_id, int vehicle_faction_snapshot, int original_action_mode,
    uint8_t vehicle_faction_mask, const Map &destination_tile,
    bool native_local_24_nonzero, bool native_local_30_nonzero) {
BODY
}
void invoke(bool local, bool locked, bool omniscient, bool local24,
            bool local30, bool shift, bool multiplayer) {
  Vehs[0] = {0, 0}; VehPrototypes[0] = {0}; Weapon[0] = {1};
  ConsoleGlobal = {0, 0};
  GameDataStateStorage.fields_.lock_masks_.enable_mask_ = locked ? 1u : 0u;
  GameDataStateStorage.fields_.game_state_ = omniscient ? STATE_OMNISCIENT_VIEW : 0u;
  shift_state = shift ? 0x0100 : 0;
  IsMultiplayerNet = multiplayer ? 1 : 0;
  clock_count = sleep_count = spots = demotions = 0;
  order_veh_combat_and_turn_prelude(0, local ? 0 : 1, 0,
                                    locked ? 1 : 0, Map{0, 0},
                                    local24, local30);
}
int main() {
  invoke(false, false, false, true, false, false, false);
  assert(clock_count == 1 && clock_delays[0] == 5 && sleep_count == 0);
  invoke(false, false, false, true, true, false, false);
  assert(clock_count == 1 && clock_delays[0] == 100 && sleep_count == 0);
  invoke(true, false, false, true, true, false, false);
  assert(clock_count == 0 && sleep_count == 0);
  invoke(false, true, false, true, true, false, false);
  assert(clock_count == 0 && sleep_count == 0);
  invoke(false, false, true, true, true, false, false);
  assert(clock_count == 0 && sleep_count == 0);
  invoke(false, false, false, false, true, false, false);
  assert(clock_count == 0 && sleep_count == 0);
  invoke(false, false, false, true, true, true, false);
  assert(clock_count == 0 && sleep_count == 0);
  invoke(false, false, false, true, true, false, true);
  assert(clock_count == 0 && sleep_count == 0);
}
'''.replace("BODY", body)


def _compile_run(tmp_path: Path, compiler: str, body: str, name: str):
    unit = tmp_path / f"{name}.cpp"
    executable = tmp_path / name
    unit.write_text(_host_program(body), encoding="utf-8")
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    str(unit), "-o", str(executable)], check=True,
                   capture_output=True, text=True)
    return subprocess.run([str(executable)], capture_output=True, text=True)


def test_pinned_order_veh_span_uses_clock_wait_for_the_5_or_100_delay():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from capstone.x86 import X86_OP_IMM, X86_OP_MEM
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin

    image_path = Path(os.environ.get(
        "OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not image_path.is_file():
        pytest.skip("pinned original required for call-target verification")
    owner, = [row for row in read_file(ROOT / "src/veh.cpp")
              if row.address == 0x005947C0]
    assert owner.image_spans[0] == (0x005947C0, 0x0059A1AB)
    image = PeImage(image_path.read_bytes(), image_pin(ROOT))
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    low, high = owner.image_spans[0]
    instructions = list(decoder.disasm(image.read(Span(low, high)), low))
    by_address = {instruction.address: instruction for instruction in instructions}
    assert len(by_address) == len(instructions)
    cursor = low
    for instruction in instructions:
        assert instruction.address == cursor, f"undecoded byte at {cursor:#x}"
        cursor += instruction.size
    assert cursor == high, "the complete mapped order_veh span must decode"

    # Shift is queried before the multiplayer gate; delay is 100 for the true
    # local gate and 5 otherwise. The resulting call targets mapped clock_wait.
    shift_call = by_address[0x00598D77]
    assert shift_call.mnemonic == "call"
    assert shift_call.operands[0].type == X86_OP_MEM
    assert shift_call.operands[0].mem.disp == 0x00669330
    assert by_address[0x00598D9C].mnemonic == "mov"
    assert by_address[0x00598D9C].operands[1].imm == 100
    assert by_address[0x00598DA3].mnemonic == "mov"
    assert by_address[0x00598DA3].operands[1].imm == 5
    delay_call = by_address[0x00598DA9]
    assert delay_call.mnemonic == "call"
    assert delay_call.operands[0].type == X86_OP_IMM
    assert delay_call.operands[0].imm == 0x0050DD90

    # This same owner has a separate, genuine game-state sleep call; do not
    # conflate it with the cooperative delay above.
    vehicle_sleep = by_address[0x00599789]
    assert vehicle_sleep.mnemonic == "call"
    assert vehicle_sleep.operands[0].type == X86_OP_IMM
    assert vehicle_sleep.operands[0].imm == 0x005C01A0


def test_extracted_prelude_waits_cooperatively_and_wrong_callee_is_rejected(tmp_path):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    source = (ROOT / "src/veh.cpp").read_text(encoding="utf-8")
    body = _function_body(source, SIGNATURE)
    assert "clock_wait(delay);" in body
    assert "sleep(delay);" not in body
    result = _compile_run(tmp_path, compiler, body, "clock_wait_control")
    assert result.returncode == 0, result.stderr

    wrong_callee = body.replace("clock_wait(delay);", "sleep(delay);")
    assert wrong_callee != body
    damaged = _compile_run(tmp_path, compiler, wrong_callee, "wrong_callee_mutant")
    assert damaged.returncode != 0, "wrong-callee mutant unexpectedly passed behavior assertions"
