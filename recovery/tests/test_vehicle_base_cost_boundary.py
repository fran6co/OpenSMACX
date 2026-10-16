"""Keep the mapped prototype base-cost call in its own source owner."""

from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest

from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]
BASE_SIGNATURE = "int __cdecl base_cost(int proto_id)"
VEH_COST_SIGNATURE = "int __cdecl veh_cost(int proto_id, int base_id, BOOL *has_proto_cost)"


def _body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[opening + 1:end]
    raise AssertionError(f"unterminated function: {signature}")


def test_base_cost_has_mapped_owner_and_veh_cost_calls_it_directly():
    header = (ROOT / "src/veh.h").read_text(encoding="utf-8")
    vehicle = (ROOT / "src/veh.cpp").read_text(encoding="utf-8")
    owner = (ROOT / "src/vehicle_cost.cpp").read_text(encoding="utf-8")
    build = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")

    assert header.count("int __cdecl base_cost(int proto_id);") == 1
    assert "MEASURED inline int __cdecl base_cost(" not in header
    assert not re.search(r"\bbase_cost_call\s*\(", header + vehicle)
    assert "src/vehicle_cost.cpp" in build
    assert "original object-file/TU assignment is unknown" in owner
    assert BASE_SIGNATURE in owner
    assert BASE_SIGNATURE not in vehicle
    cost = _body(vehicle, VEH_COST_SIGNATURE)
    assert re.search(r"base_cost\(proto_id\)", cost)
    assert "base_cost_call" not in cost

    mapped, = [row for row in read_file(ROOT / "src/vehicle_cost.cpp")
               if row.address == 0x005A5D00]
    assert mapped.image_spans == ((0x005A5D00, 0x005A5D3F),)
    assert mapped.extract_error == ""
    assert "int __cdecl base_cost(int proto_id)" in mapped.region
    assert "return proto_cost(VehPrototypes[proto_id].chassis_id" in mapped.region


def test_pinned_base_cost_and_nested_proto_cost_edges():
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

    veh_cost, = [row for row in read_file(ROOT / "src/veh.cpp")
                 if row.address == 0x005C1850]
    low, high = veh_cost.image_spans[0]
    instructions = list(decoder.disasm(image.read(Span(low, high)), low))
    cursor = low
    by_address = {}
    for instruction in instructions:
        assert instruction.address == cursor, f"undecoded byte at {cursor:#x}"
        by_address[instruction.address] = instruction
        cursor += instruction.size
    assert cursor == high
    call = by_address[0x005C198D]
    assert call.mnemonic == "call"
    assert call.operands[0].type == X86_OP_IMM
    assert call.operands[0].imm == 0x005A5D00

    base_cost, = [row for row in read_file(ROOT / "src/vehicle_cost.cpp")
                  if row.address == 0x005A5D00]
    low, high = base_cost.image_spans[0]
    instructions = list(decoder.disasm(image.read(Span(low, high)), low))
    cursor = low
    by_address = {}
    for instruction in instructions:
        assert instruction.address == cursor, f"undecoded byte at {cursor:#x}"
        by_address[instruction.address] = instruction
        cursor += instruction.size
    assert cursor == high
    # The call itself is at 0x5A5D35; the function's earlier field loads
    # begin at 0x5A5D13. Verify the actual instruction, not a nearby offset.
    call = by_address[0x005A5D35]
    assert call.mnemonic == "call"
    assert call.operands[0].type == X86_OP_IMM
    assert call.operands[0].imm == 0x005A5A60


def test_actual_base_cost_body_calls_proto_cost_with_the_original_fields(tmp_path):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    source = (ROOT / "src/vehicle_cost.cpp").read_text(encoding="utf-8")
    body = _body(source, BASE_SIGNATURE)
    program = r'''#include <cassert>
#define __cdecl
struct Prototype { int chassis_id, weapon_id, armor_id, reactor_id; };
static Prototype VehPrototypes[2];
static int got[5];
static int __cdecl proto_cost(int c, int w, int a, int ability, int r) {
  got[0] = c; got[1] = w; got[2] = a; got[3] = ability; got[4] = r;
  return 137;
}
int __cdecl base_cost(int proto_id) {
BODY
}
int main() {
  VehPrototypes[1] = {3, 5, 7, 11};
  assert(base_cost(1) == 137);
  assert(got[0] == 3 && got[1] == 5 && got[2] == 7);
  assert(got[3] == 0 && got[4] == 11);
}
'''.replace("BODY", body)
    cpp, executable = tmp_path / "base_cost.cpp", tmp_path / "base_cost"
    cpp.write_text(program, encoding="utf-8")
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    str(cpp), "-o", str(executable)], check=True,
                   capture_output=True, text=True)
    subprocess.run([str(executable)], check=True, capture_output=True, text=True)
