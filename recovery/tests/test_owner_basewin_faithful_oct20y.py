"""Pinned instruction and source-layout evidence for BaseWin's LABS total.

The separate draw_energy_alloc host suite exercises actual rendering and
mutants. This file independently proves the original's BaseCurrent+0x108
def-use and the corresponding adjacent Base member/source expression.
"""
from pathlib import Path
import re

from match_recovery.inspection import inspect_source
from match_recovery.source import Mode, read_file


ROOT = Path(__file__).resolve().parents[2]
IMAGE = Path("/home/fran6co/code/OpenSMACX/.opensmacx/game/terranx_original.exe")
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
ADDRESS = "0x00411980"
EXPECTED_SPANS = (
    [0x00411980, 0x00412A96],
    [0x00651B4C, 0x00651B61],
)


def _original():
    report = inspect_source(ROOT, (ADDRESS,), image=IMAGE)
    assert report["image_verified"]
    assert report["result"] == "pass"
    row, = report["subjects"]
    assert row["source"]["origin"]["image_sha256"] == PIN
    assert row["source"]["path"] == "src/basewin.cpp"
    assert row["source"]["catalogued_symbol"] == "?draw_energy_alloc@BaseWin@@QAEXH@Z"
    assert row["source"]["spans"] == [EXPECTED_SPANS[0], EXPECTED_SPANS[1]]
    spans = row["original"]["spans"]
    assert [span["span"] for span in spans] == list(EXPECTED_SPANS)
    assert all(span["result"] == "pass" for span in spans)
    return spans[0]["disassembly"]


def test_pinned_labs_total_def_use_and_format_argument():
    listing = _original()
    by_address = {int(line.split()[0], 16): line for line in listing}
    expected = {
        0x00411C26: "mov edx, dword ptr [0x90ea30]",
        0x00411C4B: "mov eax, dword ptr [edx + 0x108]",
        0x00411C57: "mov dword ptr [ebp - 0x60], eax",
        0x004125D0: "mov edx, dword ptr [ebp - 0x60]",
        0x004125D3: "lea ecx, [ebp - 0xf4]",
        0x004125D9: "push 0xa",
        0x004125DB: "push ecx",
        0x004125DC: "push edx",
        0x004125DD: "mov byte ptr [0x9b86a0], 0",
        0x004125E4: "call 0x64fc88",
    }
    for address, instruction in expected.items():
        assert address in by_address
        assert by_address[address].split("  ", 2)[-1].strip() == instruction
    addresses = list(expected)
    assert addresses == sorted(addresses)


def test_base_member_declaration_and_production_format_source_match_evidence():
    header = (ROOT / "src" / "base.h").read_text()
    base_member_sequence = re.search(
        r"// RECOVERY_MEMBER: v1 Base psych_total 0x104 [^\n]*\n"
        r"\s*int psych_total;\s*\n\s*int labs_total;",
        header,
    )
    assert base_member_sequence, "labs_total must immediately follow the +0x104 int"
    assert 0x104 + 4 == 0x108

    rows = [row for row in read_file(ROOT / "src" / "basewin.cpp")
            if row.address == 0x00411980]
    row, = rows
    assert row.mode == Mode.FILE
    assert "_itoa(labs_total, number, 10);" in row.region
    assert "_itoa(saved_clip.top, number, 10);" not in row.region
