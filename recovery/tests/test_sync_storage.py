"""Actual retained storage/reset controls, not full synchronization acceptance.

The native protocol/save paths remain unfinished. These tests cover only the
declared owners, initial representation and setup's single-byte filename reset.
"""
import os
from pathlib import Path
import re
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]
OWNERS = [
    ("uint8_t", "GameWireOpaque_9A64A0", 4, 0x9A64A0, 0x9A64A4),
    ("uint8_t", "GameWireOpaque_9A64A8", 4, 0x9A64A8, 0x9A64AC),
    ("char", "LegacyGameSaveName", 256, 0x9A66C9, 0x9A67C9),
    ("uint8_t", "GameWireOpaque_9A67C9", 3, 0x9A67C9, 0x9A67CC),
]


def declarations():
    source = (ROOT / "src/game.cpp").read_text()
    header = (ROOT / "src/game.h").read_text()
    result = []
    for kind, name, size, start, end in OWNERS:
        declaration = f"{kind} {name}[{size}];"
        assert source.count(declaration) == 1
        assert header.count("extern " + declaration) == 1
        symbol_type = "PADA" if kind == "char" else "PAEA"
        marker = f"// RECOVERY_OBJECT: v2 0x{start:08X} 0x{end:08X} ?{name}@@3{symbol_type} "
        assert source.count(marker) == 1
        assert end - start == size
        result.append(declaration)
    vehicle = (ROOT / "src/veh.cpp").read_text()
    assert "int *const VehSelectionFollowLimit" not in vehicle
    assert "*VehSelectionFollowLimit" not in vehicle
    assert re.findall(r"(?m)^int VehSelectionFollowLimit;", vehicle) == [
        "int VehSelectionFollowLimit;"]
    assert "extern int VehSelectionFollowLimit;" in (ROOT / "src/veh.h").read_text()
    result.append("int VehSelectionFollowLimit;")
    setup = source[source.index("void __cdecl setup_game(int reload) {"):]
    setup = setup[:setup.index("\n}")]
    accesses = [line.strip() for line in setup.splitlines() if "LegacyGameSaveName" in line]
    assert accesses == ["LegacyGameSaveName[0] = 0;"]
    return "\n".join(result), accesses[0]


@pytest.mark.parametrize("damage", [False, True])
def test_actual_retained_storage_and_prefix_only_reset(tmp_path, damage):
    owners, reset = declarations()
    if damage:
        reset = "memset(LegacyGameSaveName, 0, sizeof LegacyGameSaveName);"
    program = """
#include <stdint.h>
#include <string.h>
#include <assert.h>
OWNERS
int main() {
    assert(sizeof GameWireOpaque_9A64A0 == 4);
    assert(sizeof GameWireOpaque_9A64A8 == 4);
    assert(sizeof LegacyGameSaveName == 256);
    assert(sizeof GameWireOpaque_9A67C9 == 3);
    assert(sizeof VehSelectionFollowLimit == 4);
    assert(VehSelectionFollowLimit == 0);
    for (unsigned i = 0; i < sizeof LegacyGameSaveName; ++i)
        assert(LegacyGameSaveName[i] == 0);
    memset(GameWireOpaque_9A64A0, 0x41, sizeof GameWireOpaque_9A64A0);
    memset(GameWireOpaque_9A64A8, 0x52, sizeof GameWireOpaque_9A64A8);
    memset(GameWireOpaque_9A67C9, 0x63, sizeof GameWireOpaque_9A67C9);
    memset(LegacyGameSaveName, 0x74, sizeof LegacyGameSaveName);
    VehSelectionFollowLimit = -3;
    RESET
    assert(LegacyGameSaveName[0] == 0);
    for (unsigned i = 1; i < sizeof LegacyGameSaveName; ++i)
        assert(LegacyGameSaveName[i] == 0x74);
    for (unsigned i = 0; i < 4; ++i) {
        assert(GameWireOpaque_9A64A0[i] == 0x41);
        assert(GameWireOpaque_9A64A8[i] == 0x52);
    }
    for (unsigned i = 0; i < 3; ++i) assert(GameWireOpaque_9A67C9[i] == 0x63);
    assert(VehSelectionFollowLimit == -3);
}
""".replace("OWNERS", owners).replace("RESET", reset)
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    assert any(compilers), "a host compiler is required"
    for index, compiler in enumerate(compilers):
        if compiler is None:
            continue
        cpp, binary = tmp_path / f"storage-{index}.cpp", tmp_path / f"storage-{index}"
        cpp.write_text(program)
        build = subprocess.run([compiler, "-std=c++03", "-Wall", "-Wextra", "-Werror",
            "-pedantic", "-fsanitize=address,undefined", str(cpp), "-o", str(binary)],
            capture_output=True, text=True, timeout=60)
        assert build.returncode == 0, build.stderr
        run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30,
            env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))
        assert (run.returncode != 0) if damage else (run.returncode == 0), run.stderr


def test_pinned_native_initial_storage_and_legacy_copy_bounds():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin

    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned original required for native evidence")
    image = PeImage(path.read_bytes(), image_pin(ROOT))
    for _, _, size, start, end in OWNERS:
        assert image.read_initial(Span(start, end)) == bytes(size)
    assert image.read_initial(Span(0x9A64DC, 0x9A64E0)) == bytes(4)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    reset = list(decoder.disasm(image.read(Span(0x5B3935, 0x5B393C)), 0x5B3935))
    assert reset[0].mnemonic == "mov" and reset[0].op_str == "byte ptr [0x9a66c9], 0"
    copy = list(decoder.disasm(image.read(Span(0x5A6BA3, 0x5A6BBB)), 0x5A6BA3))
    assert any(i.mnemonic == "mov" and i.op_str == "ecx, 0x40" for i in copy)
    assert any(i.mnemonic == "mov" and i.op_str == "edi, 0x9a66c9" for i in copy)
    assert any(i.mnemonic == "rep movsd" for i in copy)
