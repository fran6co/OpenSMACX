"""Check veh_lift's live body against its pinned native coordinate order."""
from __future__ import annotations

import hashlib
import os
import shutil
import subprocess
import tempfile
from pathlib import Path

from capstone import CS_ARCH_X86, CS_MODE_32, Cs
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage

ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
SOURCE = ROOT / "src/veh.cpp"
SPAN = Span(0x005BFFA0, 0x005C007F)

PREFIX = r'''#include <cstdint>
#include <cstdlib>
#include <cstring>
#define __cdecl
typedef int BOOL;
ACTUAL_TILE_ENUM
ACTUAL_VEH_DECLARATION
Veh Vehs[8];
int VehDropLiftVehID, VehLiftX, VehLiftY;
int on_map_calls, bit_set_calls, bit_set_contract_failed;
int callback_x, callback_y;
BOOL on_map(int x, int y) {
    ++on_map_calls;
    return x >= 0 && x < 64 && y >= 0 && y < 64;
}
void bit_set(int x, int y, int bit, BOOL clear) {
    ++bit_set_calls;
    if (x != Vehs[2].x || y != Vehs[2].y || bit != BIT_VEH_IN_TILE || clear != 0
        || VehDropLiftVehID != -99 || Vehs[2].next_veh_id_stack != -1
        || Vehs[2].prev_veh_id_stack != -1) bit_set_contract_failed=1;
    Vehs[2].x = static_cast<int16_t>(callback_x);
    Vehs[2].y = static_cast<int16_t>(callback_y);
}
'''

MAIN = r'''
void reset(int16_t x, int16_t y, int16_t prev, int16_t next) {
    std::memset(Vehs, 0, sizeof(Vehs));
    Vehs[2].x=x; Vehs[2].y=y;
    Vehs[2].prev_veh_id_stack=prev; Vehs[2].next_veh_id_stack=next;
    if (prev >= 0) Vehs[prev].next_veh_id_stack=2;
    if (next >= 0) Vehs[next].prev_veh_id_stack=2;
    VehDropLiftVehID=-99; VehLiftX=-99; VehLiftY=-99;
    on_map_calls=bit_set_calls=bit_set_contract_failed=0;
    callback_x=31; callback_y=-7;
}
int main() {
    // Both linked neighbors are spliced around the lifted vehicle.
    reset(12, 23, 1, 3); if (veh_lift(2)!=2) return 90;
    if (Vehs[1].next_veh_id_stack!=3 || Vehs[3].prev_veh_id_stack!=1
        || VehLiftX!=12 || VehLiftY!=23 || VehDropLiftVehID!=2
        || Vehs[2].x!=-1 || Vehs[2].y!=-1
        || Vehs[2].next_veh_id_stack!=-1 || Vehs[2].prev_veh_id_stack!=-1
        || bit_set_calls!=0) return 1;

    // No previous neighbor but a next neighbor: update its back-link, no map clear.
    reset(14, 25, -1, 3); if (veh_lift(2)!=2) return 91;
    if (Vehs[3].prev_veh_id_stack!=-1 || VehLiftX!=14 || VehLiftY!=25
        || bit_set_calls!=0) return 2;

    // Previous neighbor only: update its forward-link without map removal.
    reset(15, 26, 1, -1); if (veh_lift(2)!=2) return 92;
    if (Vehs[1].next_veh_id_stack!=-1 || VehLiftX!=15 || VehLiftY!=26
        || bit_set_calls!=0) return 3;

    // With no neighbors on-map, bit_set runs; the following native coordinate
    // loads observe the post-call record values (a timing control, not a claim
    // that the real bit_set routine mutates Vehs).
    reset(16, 27, -1, -1); if (veh_lift(2)!=2) return 93;
    if (on_map_calls!=1 || bit_set_calls!=1 || VehLiftX!=callback_x
        || VehLiftY!=callback_y || Vehs[2].x!=-1 || Vehs[2].y!=-1
        || bit_set_contract_failed) return 4;

    // No neighbors off-map: no bit_set call, and signed negative x survives.
    reset(-4, 70, -1, -1); if (veh_lift(2)!=2) return 94;
    if (on_map_calls!=1 || bit_set_calls!=0 || VehLiftX!=-4 || VehLiftY!=70
        || Vehs[2].x!=-1 || Vehs[2].y!=-1) return 5;
    return 0;
}
'''


def _body(source: str) -> str:
    signature = "int __cdecl veh_lift(int veh_id) {"
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated veh_lift definition")


def _braced(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1] + ";"
    raise AssertionError("unterminated fixture declaration")


def _fixture_prefix() -> str:
    veh_header = (ROOT / "src/veh.h").read_text()
    map_header = (ROOT / "src/map.h").read_text()
    veh_decl = _braced(veh_header, "struct Veh {")
    terrain_decl = _braced(map_header, "enum TerrainBit {")
    assert "int16_t x;" in veh_decl and "int16_t y;" in veh_decl
    assert "int16_t next_veh_id_stack;" in veh_decl
    assert "int16_t prev_veh_id_stack;" in veh_decl
    assert "BIT_VEH_IN_TILE = 0x2" in terrain_decl
    return PREFIX.replace("ACTUAL_TILE_ENUM", terrain_decl).replace(
        "ACTUAL_VEH_DECLARATION", veh_decl)


def _run(body: str, compiler: str, optimization: str, expected: int) -> None:
    with tempfile.TemporaryDirectory(prefix="vehicle-lift-contract-") as directory:
        cpp = Path(directory) / "veh_lift.cpp"
        exe = Path(directory) / "veh_lift"
        cpp.write_text(_fixture_prefix() + "\n" + body + "\n" + MAIN)
        subprocess.run([
            compiler, "-std=c++11", optimization, "-Wall", "-Wextra", "-Werror",
            str(cpp), "-o", str(exe)
        ], check=True)
        result = subprocess.run([str(exe)], check=False)
        assert result.returncode == expected, (
            f"{compiler} {optimization}: expected {expected}, got {result.returncode}"
        )


def test_actual_veh_lift_preserves_native_capture_and_publication_order() -> None:
    body = _body(SOURCE.read_text())
    compilers = [compiler for compiler in ("g++", "clang++") if shutil.which(compiler)]
    assert compilers, "neither g++ nor clang++ is available"
    for compiler in compilers:
        for optimization in ("-O0", "-O2"):
            _run(body, compiler, optimization, expected=0)


def test_post_clear_read_damage_fails_coordinate_fixture() -> None:
    body = _body(SOURCE.read_text())
    assert body.count("VehLiftX = lift_x;") == 1
    assert body.count("VehLiftY = lift_y;") == 1
    mutant = body.replace("VehLiftX = lift_x;", "VehLiftX = Vehs[veh_id].x;", 1)
    mutant = mutant.replace("VehLiftY = lift_y;", "VehLiftY = Vehs[veh_id].y;", 1)
    captures = "    const int lift_x = Vehs[veh_id].x;\n    const int lift_y = Vehs[veh_id].y;\n"
    assert mutant.count(captures) == 1
    mutant = mutant.replace(captures, "", 1)
    for compiler in ("g++", "clang++"):
        if shutil.which(compiler):
            for optimization in ("-O0", "-O2"):
                _run(mutant, compiler, optimization, expected=1)


def test_early_capture_mutant_fails_callback_reload_case() -> None:
    body = _body(SOURCE.read_text())
    captures = "    const int lift_x = Vehs[veh_id].x;\n    const int lift_y = Vehs[veh_id].y;\n"
    marker = "    int16_t next_veh_id = Vehs[veh_id].next_veh_id_stack;\n"
    assert body.count(captures) == 1 and body.count(marker) == 1
    early = body.replace(captures, "", 1)
    early = early.replace(marker, captures + marker, 1)
    for compiler in ("g++", "clang++"):
        if shutil.which(compiler):
            for optimization in ("-O0", "-O2"):
                _run(early, compiler, optimization, expected=4)


def test_pinned_native_lift_capture_clear_and_publication_order() -> None:
    image_value = os.environ.get("OPENSMACX_IMAGE")
    assert image_value, "OPENSMACX_IMAGE must explicitly identify the pinned original"
    image_path = Path(image_value)
    assert image_path.is_absolute() and image_path.is_file()
    image_data = image_path.read_bytes()
    assert hashlib.sha256(image_data).hexdigest() == PIN
    image = PeImage(image_data, PIN)
    instructions = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(image.read(SPAN), SPAN.start))
    assert instructions and instructions[0].address == SPAN.start
    assert sum(instruction.size for instruction in instructions) == SPAN.end - SPAN.start
    native = {instruction.address: (instruction.mnemonic, instruction.op_str)
              for instruction in instructions}
    assert native[0x005C003A] == ("movsx", "ecx, word ptr [esi + 0x952828]")
    assert native[0x005C0041] == ("movsx", "edx, word ptr [esi + 0x95282a]")
    assert native[0x005C004B] == ("mov", "dword ptr [0x9b2280], edi")
    assert native[0x005C0051] == ("mov", "word ptr [esi + 0x952828], ax")
    assert native[0x005C0058] == ("mov", "word ptr [esi + 0x95282a], ax")
    assert native[0x005C005F] == ("mov", "word ptr [esi + 0x952858], ax")
    assert native[0x005C0066] == ("mov", "word ptr [esi + 0x95285a], ax")
    assert native[0x005C006D] == ("mov", "eax, edi")
    assert native[0x005C0070] == ("mov", "dword ptr [0x9b2278], ecx")
    assert native[0x005C0076] == ("mov", "dword ptr [0x9b2284], edx")
    addresses = [instruction.address for instruction in instructions]
    assert addresses.index(0x005C003A) < addresses.index(0x005C0041)
    assert addresses.index(0x005C0041) < addresses.index(0x005C004B)
    assert addresses.index(0x005C004B) < addresses.index(0x005C0051)
    assert addresses.index(0x005C0066) < addresses.index(0x005C0070)
    assert addresses.index(0x005C0070) < addresses.index(0x005C0076)

