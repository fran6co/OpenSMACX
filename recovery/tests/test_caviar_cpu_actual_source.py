"""Exercise the real CPU owner with controlled original flags/CPUID effects.

These host controls do not prove entry-register or final condition-flag ABI,
physical processor behavior, linked selection, or original byte matching.
"""
from pathlib import Path
import json
import os
import shutil
import subprocess
import sys

import pytest

ROOT = Path(__file__).resolve().parents[2]

HARNESS = r'''
#include <cassert>
#include <vector>
#define OPENSMACX_CPU_TEST_HARDWARE
#include "caviar_cpu.h"
static unsigned int saved, ac_read, id_read;
static int reads, maximum, family, mmx, manufacturer;
static std::vector<unsigned int> writes;
static std::vector<int> leaves, subleaves;
struct CaviarCpuHardware {
    unsigned int read_flags() {
        ++reads;
        if (reads == 1) return saved;
        if (reads == 2) {
            assert(CaviarVoxelAlgoFlag == 99 && vox_cpu_vendor == 99);
            return ac_read;
        }
        assert(CaviarVoxelAlgoFlag == 1 && vox_cpu_vendor == 0);
        return id_read;
    }
    void write_flags(unsigned int flags) { writes.push_back(flags); }
    void identify(int *words, int leaf, int ecx) {
        leaves.push_back(leaf); subleaves.push_back(ecx);
        if (leaf == 0) {
            assert(ecx == static_cast<int>(saved));
            words[0] = maximum;
            words[1] = manufacturer == 1 ? 0x756e6547 : manufacturer == 2 ? 0x68747541 : 7;
            words[2] = manufacturer == 1 ? 0x6c65746e : manufacturer == 2 ? 0x444d4163 : 8;
            words[3] = manufacturer == 1 ? 0x49656e69 : manufacturer == 2 ? 0x69746e65 : 9;
        } else {
            assert(leaf == 1);
            assert(ecx == (manufacturer == 1 ? 0x6c65746e : manufacturer == 2 ? 0x444d4163 : 8));
            words[0] = (family << 8) | 0x100000;
            words[1] = words[2] = 0;
            words[3] = mmx ? 0x800000 : 0;
        }
    }
};
#include "caviar_cpu.cpp"
static void reset() {
    saved = 0x202; ac_read = saved ^ 0x40000; id_read = saved ^ 0x200000;
    reads = 0; maximum = 1; family = 5; mmx = 0; manufacturer = 1;
    writes.clear(); leaves.clear(); subleaves.clear();
    CaviarVoxelAlgoFlag = vox_cpu_vendor = 99;
}
int main() {
    reset(); ac_read = saved; vox_detect_cpu();
    assert(CaviarVoxelAlgoFlag == 0 && vox_cpu_vendor == 0);
    assert(reads == 2 && leaves.empty() && writes.size() == 1);
    assert(writes[0] == (saved ^ 0x40000));
    reset(); id_read = saved; vox_detect_cpu();
    assert(CaviarVoxelAlgoFlag == 1 && vox_cpu_vendor == 0);
    assert(reads == 3 && leaves.empty() && writes.size() == 3);
    for (int vendor = 0; vendor != 3; ++vendor)
    for (int base_family = 4; base_family != 8; ++base_family)
    for (int feature = 0; feature != 2; ++feature) {
        reset(); manufacturer = vendor; family = base_family; mmx = feature;
        vox_detect_cpu();
        const int expected = family == 5 ? 2 + mmx : family == 6 ? 4 + mmx : 0;
        assert(CaviarVoxelAlgoFlag == expected && vox_cpu_vendor == vendor);
        assert(leaves.size() == 2 && leaves[0] == 0 && leaves[1] == 1);
        assert(writes.size() == 3 && writes[0] == (saved ^ 0x40000));
        assert(writes[1] == saved && writes[2] == (saved ^ 0x200000));
    }
    for (int limit = -1; limit != 1; ++limit) {
        reset(); maximum = limit; vox_detect_cpu();
        assert(CaviarVoxelAlgoFlag == 1 && vox_cpu_vendor == 1);
        assert(leaves.size() == 1 && writes.size() == 3);
    }
    // Full-word gates: a different non-AC/non-ID bit still opens each gate.
    reset(); ac_read = saved ^ 0x40; id_read = saved ^ 0x80;
    vox_detect_cpu(); assert(CaviarVoxelAlgoFlag == 2 && leaves.size() == 2);
}
'''


def compile_and_run(tmp_path, source_dir):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    harness = tmp_path / "cpu.cpp"
    harness.write_text(HARNESS)
    output = tmp_path / "cpu"
    compiled = subprocess.run(
        [compiler, "-std=c++11", "-O0", "-I", str(source_dir), str(harness), "-o", str(output)],
        capture_output=True, text=True, timeout=60,
    )
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(output)], capture_output=True, timeout=10)


def test_actual_owner_preserves_original_hardware_sequence(tmp_path):
    result = compile_and_run(tmp_path, ROOT / "src")
    assert result.returncode == 0, result.stderr


def test_pinned_original_probe_control_flow_and_vendor_words():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    import struct
    path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.is_file():
        pytest.skip("pinned original unavailable")
    image = PeImage(path.read_bytes(),
        "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf")
    instructions = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(
        image.read(Span(0x63E860, 0x63E94E)), 0x63E860))
    assert instructions[-1].address + instructions[-1].size == 0x63E94E
    anchors = {i.address: (i.mnemonic, i.op_str) for i in instructions}
    expected = {
        0x63E86D: ("xor", "eax, 0x40000"),
        0x63E876: ("xor", "eax, ecx"),
        0x63E878: ("mov", "byte ptr [0x9c0830], 0"),
        0x63E87F: ("mov", "byte ptr [0x9c0b70], 0"),
        0x63E886: ("je", "0x63e946"),
        0x63E88D: ("popfd", ""),
        0x63E897: ("xor", "eax, 0x200000"),
        0x63E8A0: ("xor", "eax, ecx"),
        0x63E8A2: ("je", "0x63e946"),
        0x63E8AD: ("cpuid", ""),
        0x63E8EF: ("cmp", "eax, 1"),
        0x63E8F2: ("jl", "0x63e946"),
        0x63E8F9: ("cpuid", ""),
        0x63E8FE: ("and", "eax, 0xf"),
        0x63E914: ("test", "edx, 0x800000"),
        0x63E92E: ("test", "edx, 0x800000"),
        0x63E94B: ("pop", "eax"),
        0x63E94D: ("ret", ""),
    }
    for address, instruction in expected.items():
        assert anchors[address] == instruction
    assert sum(i.mnemonic == "popfd" for i in instructions) == 3
    assert struct.unpack("<6I", image.read(Span(0x698DD8, 0x698DF0))) == (
        0x756e6547, 0x49656e69, 0x6c65746e,
        0x68747541, 0x69746e65, 0x444d4163)


def test_actual_i386_owner_register_saves_and_observed_object(tmp_path):
    if shutil.which("clang-cl") is None:
        pytest.skip("registered intrinsic compiler unavailable")
    from recovery.environment.wine import VC6_ROOT
    if not (VC6_ROOT / "BIN/CL.EXE").is_file():
        pytest.skip("project wrapper configuration unavailable")
    from match_recovery.pe_coff import Coff
    from match_recovery.compile_file_evidence import verify_compile_files
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    output = tmp_path / "cpu.obj"
    receipt = tmp_path / "cpu.compile.json"
    env = os.environ.copy()
    for name in ("CL", "_CL_", "INCLUDE", "CPATH", "C_INCLUDE_PATH", "CPLUS_INCLUDE_PATH"):
        env.pop(name, None)
    result = subprocess.run([
        sys.executable, "-I", "-B", str(ROOT / "recovery/environment/wine.py"),
        "--tool=cl", "--frontend=intrinsics-i386-no-debug",
        "--repro-root=" + str(ROOT), "--compile-diagnostics=" + str(receipt),
        "/nologo", "/c", "/O2", "/Gy", "/GR-", "/GX",
        "/FI" + str(ROOT / "src/vc6_compat.h"), "/Fo" + str(output),
        str(ROOT / "src/caviar_cpu.cpp"),
    ], cwd=ROOT, env=env, capture_output=True, timeout=60)
    assert result.returncode == 0, result.stderr.decode(errors="replace")
    obj = Coff(output.read_bytes())
    symbol = obj.function("?vox_detect_cpu@@YAXXZ")
    body, relocations, _ = obj.subject(symbol)
    instructions = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(body, 0))
    assert sum(i.size for i in instructions) == len(body)
    assert sum(i.mnemonic == "cpuid" for i in instructions) == 2
    assert not any(i.mnemonic == "call" for i in instructions)
    for register in ("eax", "ebx", "ecx", "edx", "esi", "edi", "ebp"):
        assert any(i.mnemonic == "push" and i.op_str == register for i in instructions)
        assert any(i.mnemonic == "pop" and i.op_str == register for i in instructions)
    assert instructions[-1].mnemonic == "ret"
    assert {obj.symbols[int(r.target)].name for r in relocations} == {
        "?CaviarVoxelAlgoFlag@@3EA", "?vox_cpu_vendor@@3EA"}
    report = json.loads(receipt.read_text())
    assert report["schema_version"] == 2
    assert "/clang:-march=i386" in report["arguments"]
    assert "/clang:-mgeneral-regs-only" in report["arguments"]
    assert verify_compile_files(receipt.read_bytes(), object_path=output)["result"] == "pass"


@pytest.mark.parametrize("before,after", [
    ("observed == saved", "((observed ^ saved) & 0x40000u) == 0"),
    ("hardware.read_flags() == saved", "((hardware.read_flags() ^ saved) & 0x200000u) == 0"),
    ("vendor = 0;", "vendor = 99;"),
    ("static_cast<int>(saved)", "0"),
    ("hardware.identify(words, 1, words[2]);", "hardware.identify(words, 1, 0);"),
    ("words[0] < 1", "static_cast<unsigned int>(words[0]) < 1u"),
    ("0x800000", "0x400000"),
    ("rank = selected;", "rank = selected; hardware.write_flags(saved);"),
])
def test_controls_detect_original_behavior_regressions(tmp_path, before, after):
    owner = tmp_path / "owner"
    owner.mkdir()
    header = (ROOT / "src/caviar_cpu.h").read_text()
    assert header.count(before) == 1
    (owner / "caviar_cpu.h").write_text(header.replace(before, after))
    (owner / "caviar_cpu.cpp").write_bytes((ROOT / "src/caviar_cpu.cpp").read_bytes())
    assert compile_and_run(tmp_path, owner).returncode != 0
