"""The mapped vehicle sleep operation remains a real call owner."""

from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest

from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]
CALLERS = (
    ("src/faction_strategy.cpp", 0x00561080, 0x00561DE5),
    ("src/basewin.cpp", 0x0040B140, 0x0040BCB9),
    ("src/game.cpp", 0x005AEDE0, 0x005AFC58),
    ("src/game.cpp", 0x00522180, 0x00522829),
    ("src/console.cpp", 0x004D2220, 0x004D2443),
    ("src/veh.cpp", 0x005B8EE0, 0x005B9312),
    ("src/veh.cpp", 0x0057A660, 0x0057BDA1),
    ("src/veh.cpp", 0x005947C0, 0x00599789),
)
SLEEP_SIGNATURE = "void __cdecl sleep(int veh_id)"
STACK_SIGNATURE = "int __cdecl stack_veh(int veh_id, int mode)"


def _body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[opening + 1:end]
    raise AssertionError(f"unterminated function: {signature}")


def test_sleep_is_declared_normally_and_mapped_to_vehicle_orders_owner():
    header = (ROOT / "src/veh.h").read_text(encoding="utf-8")
    source = (ROOT / "src/veh.cpp").read_text(encoding="utf-8")
    owner_source = (ROOT / "src/vehicle_orders.cpp").read_text(encoding="utf-8")
    build = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
    assert header.count("void __cdecl sleep(int veh_id);") == 1
    assert "MEASURED inline void __cdecl sleep(" not in header
    assert "void __cdecl sleep_call(" not in header
    assert "void __cdecl sleep_call(" not in source
    assert "src/vehicle_orders.cpp" in build
    assert "original object-file/TU assignment is\n// unknown" in owner_source
    assert SLEEP_SIGNATURE in owner_source
    assert SLEEP_SIGNATURE not in source

    owner, = [row for row in read_file(ROOT / "src/vehicle_orders.cpp")
              if row.address == 0x005C01A0]
    assert owner.image_spans == ((0x005C01A0, 0x005C01CA),)
    assert owner.extract_error == ""
    assert "void __cdecl sleep(int veh_id)" in owner.region
    assert "Vehs[veh_id].order = ORDER_SENTRY_BOARD;" in owner.region
    assert "Vehs[veh_id].waypoint_x[0] = -1;" in owner.region
    assert "Vehs[veh_id].waypoint_y[0] = 0;" in owner.region

    stack = _body(source, STACK_SIGNATURE)
    assert re.search(
        r"sleep\(i\);\s*Vehs\[i\]\.waypoint_x\[0\] = \(int16_t\)veh_id;",
        stack)


def test_pinned_all_eight_original_sleep_calls_and_distinct_delay_target():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from capstone.x86 import X86_OP_IMM
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin

    image_path = Path(os.environ.get(
        "OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not image_path.is_file():
        pytest.skip("pinned original required for call-target verification")
    image = PeImage(image_path.read_bytes(), image_pin(ROOT))
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    for relative_path, owner_address, call_address in CALLERS:
        owner, = [row for row in read_file(ROOT / relative_path)
                  if row.address == owner_address]
        low, high = owner.image_spans[0]
        instructions = list(decoder.disasm(image.read(Span(low, high)), low))
        cursor = low
        by_address = {}
        for instruction in instructions:
            assert instruction.address == cursor, f"undecoded byte at {cursor:#x}"
            by_address[instruction.address] = instruction
            cursor += instruction.size
        assert cursor == high, f"incomplete mapped owner {owner_address:#x}"
        call = by_address[call_address]
        assert call.mnemonic == "call"
        assert call.operands[0].type == X86_OP_IMM
        assert call.operands[0].imm == 0x005C01A0

    order, = [row for row in read_file(ROOT / "src/veh.cpp")
              if row.address == 0x005947C0]
    low, high = order.image_spans[0]
    instructions = list(decoder.disasm(image.read(Span(low, high)), low))
    delay_call, = [instruction for instruction in instructions
                   if instruction.address == 0x00598DA9]
    assert delay_call.mnemonic == "call"
    assert delay_call.operands[0].type == X86_OP_IMM
    assert delay_call.operands[0].imm == 0x0050DD90

    sleep_owner, = [row for row in read_file(ROOT / "src/vehicle_orders.cpp")
                    if row.address == 0x005C01A0]
    low, high = sleep_owner.image_spans[0]
    stores = list(decoder.disasm(image.read(Span(low, high)), low))
    by_address = {instruction.address: instruction for instruction in stores}
    assert (by_address[0x005C01AF].mnemonic,
            by_address[0x005C01AF].op_str) == (
                "mov", "byte ptr [eax + 0x952839], 1")
    assert (by_address[0x005C01B6].mnemonic,
            by_address[0x005C01B6].op_str) == (
                "mov", "word ptr [eax + 0x95283c], 0xffff")
    assert (by_address[0x005C01BF].mnemonic,
            by_address[0x005C01BF].op_str) == (
                "mov", "word ptr [eax + 0x952844], 0")


def test_actual_sleep_body_and_stack_overwrite_run_in_a_host_control(tmp_path):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    source = (ROOT / "src/vehicle_orders.cpp").read_text(encoding="utf-8")
    stack_source = (ROOT / "src/veh.cpp").read_text(encoding="utf-8")
    sleep_body = _body(source, SLEEP_SIGNATURE)
    stack_body = _body(stack_source, STACK_SIGNATURE)
    overwrite, = re.findall(
        r"sleep\(i\);\s*(Vehs\[i\]\.waypoint_x\[0\] = \(int16_t\)veh_id;)",
        stack_body)
    overwrite = overwrite.replace("Vehs[i]", "Vehs[1]").replace("veh_id", "parent_id")
    program = r'''#include <cassert>
#include <cstdint>
#define __cdecl
enum { ORDER_SENTRY_BOARD = 1 };
struct Veh { int order; int16_t waypoint_x[4]; int16_t waypoint_y[4]; };
static Veh Vehs[4];
void __cdecl sleep(int veh_id) {
BODY
}
void accepted_stack_store(int parent_id) {
  const int i = 1;
  sleep(i);
  STORE
}
int main() {
  Vehs[1] = {0, {12, 13, 14, 15}, {20, 21, 22, 23}};
  Vehs[2] = {4, {5, 6, 7, 8}, {9, 10, 11, 12}};
  sleep(2); // exercise all three actual source stores before the stack overwrite
  assert(Vehs[2].order == ORDER_SENTRY_BOARD);
  assert(Vehs[2].waypoint_x[0] == -1);
  assert(Vehs[2].waypoint_y[0] == 0);
  assert(Vehs[2].waypoint_x[1] == 6 && Vehs[2].waypoint_y[1] == 10);
  assert(Vehs[1].order == 0 && Vehs[1].waypoint_x[0] == 12);
  accepted_stack_store(3);
  assert(Vehs[1].order == ORDER_SENTRY_BOARD);
  assert(Vehs[1].waypoint_x[0] == 3); // stack_veh immediately replaces -1
  assert(Vehs[1].waypoint_y[0] == 0);
}
'''.replace("BODY", sleep_body).replace("STORE", overwrite)
    cpp, executable = tmp_path / "sleep.cpp", tmp_path / "sleep"
    cpp.write_text(program, encoding="utf-8")
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    str(cpp), "-o", str(executable)], check=True,
                   capture_output=True, text=True)
    subprocess.run([str(executable)], check=True, capture_output=True, text=True)
