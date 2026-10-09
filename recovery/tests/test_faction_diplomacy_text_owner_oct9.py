"""Actual-source controls for the diplomacy labels' source-owned storage."""
from __future__ import annotations

from pathlib import Path
import re
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]
HEADER = ROOT / "src/faction.h"
SOURCE = ROOT / "src/faction.cpp"

EXPECTED = {
    "DiploLabelSendingTREATYON": "Sending TREATY ON",
    "DiploLabelSendingTREATYOFF": "Sending TREATY OFF",
    "DiploLabelSendingSETTREATY": "Sending SET TREATY",
    "DiploLabelSendingAGENDAON": "Sending AGENDA ON",
    "DiploLabelSendingAGENDAOFF": "Sending AGENDA OFF",
    "DiploLabelSendingSETAGENDA": "Sending SET AGENDA",
    "DiploLabelSendingENERGY": "Sending ENERGY",
    "DiploLabelSendingLOAN": "Sending LOAN",
    "DiploLabelSendingTRADEMAPS": "Sending TRADE MAPS",
    "DiploLabelSendingTECH": "Sending TECH",
    "DiploLabelSendingPACTENDS": "Sending PACT ENDS",
    "DiploLabelSendingWITHDRAWAL": "Sending WITHDRAWAL",
    "DiploLabelSendingCEDEBASE": "Sending CEDE BASE",
    "DiploLabelSendingDOUBLECROSS": "Sending DOUBLE CROSS",
    "DiploLabelGettingDIPLOMATICLOCK": "Getting DIPLOMATIC LOCK",
    "DiploLabelNEEDGLOBAL": "NEEDGLOBAL",
    "DiploLabelReleasingDIPLOMATICLOCK": "Releasing DIPLOMATIC LOCK",
    "DiploLabelPACTATTACKS": "PACTATTACKS",
    "DiploLabelPACTUNPACT": "PACTUNPACT",
    "DiploLabelPACTTRUCE": "PACTTRUCE",
    "DiploLabelTEAM": "TEAM",
    "DiploLabelMAJORATROCITY": "MAJORATROCITY",
    "DiploLabelCouncilSmPcx": "council_sm.pcx",
    "DiploLabelBreakSmPcx": "break_sm.pcx",
    "DiploLabelBREAKINGPACT": "BREAKINGPACT",
    "DiploLabelBreakSmPcx_7C0": "break_sm.pcx",
    "DiploLabelBREAKINGTREATY": "BREAKINGTREATY",
    "DiploLabelBreakSmPcx_7E0": "break_sm.pcx",
    "DiploLabelBREAKINGTRUCE": "BREAKINGTRUCE",
    "DiploLabelBreakSmPcx_800": "break_sm.pcx",
    "DiploLabelBEGINVENDETTA": "BEGINVENDETTA",
}

DECL = re.compile(r'^char (DiploLabel\w+)\[\] = "([^"\\]*)";', re.M)


def definitions(source: str) -> dict[str, str]:
    return dict(DECL.findall(source))


def body(source: str, name: str) -> str:
    match = re.search(r"\b" + re.escape(name) + r"\s*\([^;]*?\)\s*\{", source, re.S)
    assert match, f"missing actual source function {name}"
    cursor, depth = match.end(), 1
    while depth:
        if source[cursor] == "{":
            depth += 1
        elif source[cursor] == "}":
            depth -= 1
        cursor += 1
    return source[match.end():cursor - 1]


def host_control(compiler: str, tmp_path: Path, source: str) -> subprocess.CompletedProcess:
    values = definitions(source)
    embedded = "\n".join(f'char {name}[] = "{value}";' for name, value in values.items())
    program = f'''#include <cassert>
#include <cstring>
{embedded}
int main() {{
  assert(std::strcmp(DiploLabelSendingTREATYON, "Sending TREATY ON") == 0);
  assert(std::strcmp(DiploLabelPACTATTACKS, "PACTATTACKS") == 0);
  assert(std::strcmp(DiploLabelBEGINVENDETTA, "BEGINVENDETTA") == 0);
  assert(std::strcmp(DiploLabelBreakSmPcx, "break_sm.pcx") == 0);
  assert(std::strcmp(DiploLabelBreakSmPcx_7C0, "break_sm.pcx") == 0);
  assert(std::strcmp(DiploLabelBreakSmPcx_7E0, "break_sm.pcx") == 0);
  assert(std::strcmp(DiploLabelBreakSmPcx_800, "break_sm.pcx") == 0);
  assert(&DiploLabelBreakSmPcx[0] != &DiploLabelBreakSmPcx_7C0[0]);
  assert(&DiploLabelBreakSmPcx_7C0[0] != &DiploLabelBreakSmPcx_7E0[0]);
  assert(&DiploLabelBreakSmPcx_7E0[0] != &DiploLabelBreakSmPcx_800[0]);
  DiploLabelSendingTREATYON[0] = 's';
  assert(DiploLabelSendingTREATYON[0] == 's');
  assert(sizeof(DiploLabelSendingTREATYON) == sizeof("Sending TREATY ON"));
}}
'''
    cpp, exe = tmp_path / "faction_diplomacy_text.cpp", tmp_path / "faction_diplomacy_text"
    cpp.write_text(program, encoding="utf-8")
    subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(cpp), "-o", str(exe)], check=True, timeout=60)
    return subprocess.run([str(exe)], timeout=10)


def test_source_owns_all_live_labels_and_preserves_consumers_and_address_notes():
    header, source = HEADER.read_text(encoding="utf-8"), SOURCE.read_text(encoding="utf-8")
    assert definitions(source) == EXPECTED
    for name in EXPECTED:
        assert re.search(r"extern char " + re.escape(name) + r"\[\];", header)

    # The retained old address inventory is documentation only; no active
    # declarations keep an executable-address dependency.
    active_header = re.sub(r"/\*.*?\*/|//[^\n]*", "", header, flags=re.S)
    assert not re.search(r"char\s*\*const\s+DiploLabel\w+\s*=\s*\(char\s*\*\)0x", active_header)
    assert "0x0068D444" in header and '"Sending TREATY ON"' in header
    assert "0x0068E7A0" in header and "0x0068E7C0" in header

    net_treaty = body(source, "net_treaty_on")
    assert "log_say(DiploLabelSendingTREATYON" in net_treaty
    atrocity = body(source, "major_atrocity")
    assert "DiploLabelMAJORATROCITY" in atrocity
    broke = body(source, "break_treaty")
    for name in ("DiploLabelBREAKINGPACT", "DiploLabelBreakSmPcx",
                 "DiploLabelBREAKINGTREATY", "DiploLabelBreakSmPcx_7C0",
                 "DiploLabelBREAKINGTRUCE", "DiploLabelBreakSmPcx_7E0",
                 "DiploLabelBEGINVENDETTA", "DiploLabelBreakSmPcx_800"):
        assert name in broke

    all_sources = "\n".join(path.read_text(encoding="utf-8")
                              for path in (ROOT / "src").rglob("*.cpp"))
    used = set(re.findall(r"\bDiploLabel\w+\b", all_sources))
    assert used == set(EXPECTED)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_writable_arrays_and_distinct_original_duplicate_copies(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host compiler unavailable")
    source = SOURCE.read_text(encoding="utf-8")
    assert host_control(compiler, tmp_path, source).returncode == 0


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_wrong_treaty_key_mutant_is_caught(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host compiler unavailable")
    source = SOURCE.read_text(encoding="utf-8")
    mutant = source.replace('DiploLabelSendingTREATYON[] = "Sending TREATY ON"',
                            'DiploLabelSendingTREATYON[] = "Sending TREATY OFF"', 1)
    assert mutant != source
    assert host_control(compiler, tmp_path, mutant).returncode != 0
