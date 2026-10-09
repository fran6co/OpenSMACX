"""Actual-source controls for Veh text currently borrowed from image addresses."""
from __future__ import annotations

from pathlib import Path
import re
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/veh.cpp"

EXPECTED = {
    "VehLogKillingVehicle": "Killing vehicle (v,who,v2)",
    "VehLabelMORALE": "MORALE",
    "VehIconBATWON_SM_PCX": "batwon_sm.pcx",
    "VehLabelMORALE2": "MORALE2",
    "VehIconBATWON2_SM_PCX": "batwon_sm.pcx",
    "VehLabelFEATURE": "FEATURE",
    # These addresses point at the complete phrase keys, not just ROAD/TUBE.
    "VehLabelROAD": "ROADTODONE",
    "VehLabelTUBE": "TUBETODONE",
    "VehLabelBOREHOLEMINE": "BOREHOLEMINE",
    "VehLabelHARVESTFOREST": "HARVESTFOREST",
    "VehLabelTERRAMINE": "TERRAMINE",
    "VehLabelTERRAYOURS": "TERRAYOURS",
    "VehLabelDROWNMINE": "DROWNMINE",
    "VehLabelDROWNYOURS": "DROWNYOURS",
    "VehLabelMADEFARM": "MADEFARM",
    "VehLabelMADESOIL": "MADESOIL",
    "VehLabelMADEMINE": "MADEMINE",
    "VehLabelMADESOLAR": "MADESOLAR",
    "VehLabelMADEFOREST": "MADEFOREST",
    "VehLabelMADEROAD": "MADEROAD",
    "VehLabelMADETUBE": "MADETUBE",
    "VehLabelMADESENSOR": "MADESENSOR",
    "VehLabelMADEAQUIFER": "MADEAQUIFER",
    "VehLabelMADECONDENSER": "MADECONDENSER",
    "VehLabelMADEBOREHOLE": "MADEBOREHOLE",
    "VehLabelMADESEAFARM": "MADESEAFARM",
    "VehLabelMADESEAMINE": "MADESEAMINE",
    "VehLabelMADESEASOLAR": "MADESEASOLAR",
    "VehIconBASEOBL_SM_PCX": "baseobl_sm.pcx",
    "VehLabelOBLITTED": "OBLITTED",
    "VehIconBASEOBL2_SM_PCX": "baseobl_sm.pcx",
    "VehLabelOBLITTED2": "OBLITTED2",
}

DECL = re.compile(r'^char (\w+)\[\] = "([^"\\]*)";\s*$', re.M)


def declarations(source: str) -> dict[str, str]:
    return dict(DECL.findall(source))


def body(source: str, name: str) -> str:
    match = re.search(r"\b" + re.escape(name) + r"\s*\([^;]*?\)\s*\{", source, re.S)
    assert match, f"missing actual source function {name}"
    start, depth = match.end(), 1
    cursor = start
    while depth:
        if source[cursor] == "{":
            depth += 1
        elif source[cursor] == "}":
            depth -= 1
        cursor += 1
    return source[start:cursor - 1]


def host_control(compiler: str, tmp_path: Path, source: str) -> subprocess.CompletedProcess:
    values = declarations(source)
    embedded = "\n".join(f'char {name}[] = "{value}";' for name, value in values.items())
    program = f'''#include <cassert>
#include <cstring>
{embedded}
int main() {{
  assert(std::strcmp(VehLogKillingVehicle, "Killing vehicle (v,who,v2)") == 0);
  assert(std::strcmp(VehLabelROAD, "ROADTODONE") == 0);
  assert(std::strcmp(VehLabelTUBE, "TUBETODONE") == 0);
  assert(std::strcmp(VehLabelBOREHOLEMINE, "BOREHOLEMINE") == 0);
  assert(std::strcmp(VehLabelMADESEASOLAR, "MADESEASOLAR") == 0);
  assert(std::strcmp(VehIconBASEOBL_SM_PCX, "baseobl_sm.pcx") == 0);
  assert(&VehIconBASEOBL_SM_PCX[0] != &VehIconBASEOBL2_SM_PCX[0]);
  assert(std::strcmp(VehIconBASEOBL2_SM_PCX, "baseobl_sm.pcx") == 0);
  assert(sizeof(VehLabelROAD) == sizeof("ROADTODONE"));
  assert(sizeof(VehLogKillingVehicle) == sizeof("Killing vehicle (v,who,v2)"));
}}
'''
    cpp, exe = tmp_path / "veh_text.cpp", tmp_path / "veh_text"
    cpp.write_text(program, encoding="utf-8")
    subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(cpp), "-o", str(exe)], check=True, timeout=60)
    return subprocess.run([str(exe)], timeout=10)


def test_source_owns_exact_distinct_text_arrays_and_real_consumers():
    source = SOURCE.read_text(encoding="utf-8")
    values = declarations(source)
    assert {name: values[name] for name in EXPECTED} == EXPECTED
    assert len([name for name in values if name in EXPECTED]) == len(EXPECTED)
    assert not re.search(r"char \w+\[\]\s*=\s*\(char\s*\*\)0x", source)

    assert "log_say(VehLogKillingVehicle" in body(source, "veh_kill")
    promote = body(source, "promote")
    assert "NetMsgPrimary.pop(VehLabelMORALE2" in promote
    assert "NetMsgPrimary.pop(VehLabelMORALE," in promote
    assert "NetMsgPrimary.pop(VehLabelFEATURE" in body(source, "action_feature")
    road = body(source, "action_road_to")
    assert "NetMsgPrimary.pop(VehLabelROAD" in road
    assert "NetMsgPrimary.pop(VehLabelTUBE" in road
    terraform = body(source, "action_terraform")
    assert "NetMsgPrimary.pop(VehLabelBOREHOLEMINE" in terraform
    assert "NetMsgPrimary.pop(VehLabelHARVESTFOREST" in terraform
    obliterate = body(source, "action_oblit")
    assert "VehLabelOBLITTED" in obliterate and "VehLabelOBLITTED2" in obliterate
    assert "VehIconBASEOBL_SM_PCX" in obliterate and "VehIconBASEOBL2_SM_PCX" in obliterate


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_source_arrays_compile_and_preserve_text_extent(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host compiler unavailable")
    source = SOURCE.read_text(encoding="utf-8")
    assert {name: declarations(source)[name] for name in EXPECTED} == EXPECTED
    assert host_control(compiler, tmp_path, source).returncode == 0


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_wrong_road_key_mutant_is_caught(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host compiler unavailable")
    source = SOURCE.read_text(encoding="utf-8")
    mutant = source.replace('VehLabelROAD[] = "ROADTODONE"',
                            'VehLabelROAD[] = "ROAD"', 1)
    assert mutant != source
    assert host_control(compiler, tmp_path, mutant).returncode != 0
