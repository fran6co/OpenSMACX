"""Bounded flag-read regression controls, not a full get_alt behavior proof."""

from hashlib import sha256
from pathlib import Path
import re
import shutil
import subprocess

import pytest

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage

ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def test_original_altitude_reads_flat_flag_value():
    path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.is_file():
        pytest.skip("pinned game unavailable")
    capstone = pytest.importorskip("capstone")
    data = path.read_bytes()
    assert sha256(data).hexdigest() == PIN
    image = PeImage(data, PIN)
    decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    instructions = list(decoder.disasm(image.read(Span(0x46FE70, 0x470414)), 0x46FE70))
    assert instructions[-1].address + instructions[-1].size == 0x470414
    reads = [(i.address, i.mnemonic, i.op_str) for i in instructions
             if "0x94988c" in i.op_str]
    assert reads == [(0x4700E1, "mov", "cl, byte ptr [0x94988c]"),
                     (0x4702A7, "mov", "al, byte ptr [0x94988c]")]
    for address, _, _ in reads:
        index = next(n for n, i in enumerate(instructions) if i.address == address)
        adjustment = instructions[index + 1]
        assert adjustment.mnemonic == "add"
        assert adjustment.op_str in ("eax, esi", "edx, esi")
        following = instructions[index + 2]
        assert following.mnemonic == "test"
        assert following.op_str in ("cl, 1", "al, 1")


def test_actual_altitude_flat_conditions(tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host compiler unavailable")
    source = (ROOT / "src/mapwin.cpp").read_text()
    body = source.split("int MapWin::get_alt(", 1)[1].split(
        "// ORIGINAL: 0x00462F00", 1)[0]
    conditions = re.findall(r"if\s*\(([^\n]*MapIsFlat[^\n]*)\)\s*\{", body)
    assert len(conditions) == 2
    checks = "\n".join(f"if (({condition}) != expected) return 1;"
                       for condition in conditions)
    fixture = tmp_path / "flag.cpp"
    fixture.write_text("int main() {\n"
                       "int values[] = {0,1,2,3,256,-1,-2};\n"
                       "for (int MapIsFlat : values) {\n"
                       "bool expected = (MapIsFlat & 1) == 0;\n"
                       + checks + "\n}\n}\n")
    executable = tmp_path / "flag"
    subprocess.run([compiler, "-std=c++11", "-Werror", "-Wint-to-pointer-cast",
                    str(fixture), "-o", str(executable)], check=True, capture_output=True)
    subprocess.run([str(executable)], check=True, capture_output=True)
