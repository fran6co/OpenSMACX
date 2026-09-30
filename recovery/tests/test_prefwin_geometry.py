"""Actual geometry startup with host ABI substitutions, not game acceptance."""
from pathlib import Path
import re
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


def source():
    header = (ROOT / "src/global_arith.h").read_text()
    unit = (ROOT / "src/global_arith.cpp").read_text()
    # The Win32 precompiled header supplies integer types and calling
    # conventions, not the geometry algorithm tested here.
    header = re.sub(r'^\s*#pragma once\s*$', '', header, flags=re.M)
    unit = re.sub(r'^#include "[^"]+"\s*$', '', unit, flags=re.M)
    return '#include <cstdint>\n#define __fastcall\n#define __cdecl\n' + header + unit


HARNESS = r'''
#include <cassert>
int main() {
    // Inspect before calling any helper: the runtime startup must do this.
    const int actual[] = {
        PrefWinGeometryValue_8577D8, PrefWinGeometryValue_8577DC,
        PrefWinGeometryValue_8577E0, PrefWinGeometryValue_8577E4,
        PrefWinGeometryValue_8577E8, PrefWinGeometryValue_8577EC,
        PrefWinGeometryValue_857884, PrefWinGeometryValue_857888,
        PrefWinGeometryValue_85788C, PrefWinGeometryValue_857890,
        PrefWinGeometryValue_857894, PrefWinGeometryValue_857898,
        PrefWinGeometryValue_85789C, PrefWinGeometryValue_8578A0,
        PrefWinGeometryValue_8578A4, PrefWinGeometryValue_8578A8,
        PrefWinGeometryValue_8578AC, PrefWinGeometryValue_8578B0,
        PrefWinGeometryValue_8578B4, PrefWinGeometryValue_8578B8,
        PrefWinGeometryValue_8578BC, PrefWinGeometryValue_8578C0,
        PrefWinGeometryValue_8578C4, PrefWinGeometryValue_8578C8,
        PrefWinGeometryValue_8578CC, PrefWinGeometryValue_8578D0
    };
    // Original ordered dword stores from the independently decoded
    // 48D550..48D820 initializer family and its initial input constants.
    const int expected[] = {
        256,287,92,457,4,27,7,30,33,10,214,7,253,457,
        376,463,692,382,259,328,290,376,10,256,382,288
    };
    static_assert(sizeof actual == sizeof expected, "all outputs tested");
    for (unsigned i = 0; i < sizeof actual / sizeof *actual; ++i)
        assert(actual[i] == expected[i]);
    PrefWinDisplayWidth = 1000;
    global_arith_0048d570_redirect(nullptr, nullptr);
    sub_48d650();
    assert(PrefWinGeometryValue_8578AC == 992);
    assert(PrefWinGeometryValue_8578A8 == 763);
}
'''


def execute(tmp_path, text, label):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    executable = tmp_path / label
    subprocess.run([compiler, "-std=c++17", "-x", "c++", "-", "-o", str(executable)],
                   input=text + HARNESS, text=True, capture_output=True, check=True)
    return subprocess.run([str(executable)], capture_output=True).returncode


def test_actual_geometry_is_initialized_before_use(tmp_path):
    assert execute(tmp_path, source(), "geometry") == 0


@pytest.mark.parametrize("before,after", [
    ("        sub_48d650();", "        // omitted initializer"),
    ("        sub_48d820();", "        // omitted initializer"),
    ("        global_arith_0048d590_redirect(nullptr, nullptr);",
     "        // omitted height producer"),
])
def test_missing_startup_producer_is_detected(tmp_path, before, after):
    text = source()
    assert before in text
    assert execute(tmp_path, text.replace(before, after, 1), "geometry-damaged") != 0
