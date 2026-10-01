"""Actual-source teardown controls, not native ABI or game acceptance."""
from pathlib import Path
import re
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


def test_diplomacy_teardown_preserves_lifecycle_and_callback_order(tmp_path):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    source = (ROOT / "src/faction.cpp").read_text()
    signature = "void __cdecl diplomacy_ends("
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            body = source[start:end + 1]
            break
    else:
        raise AssertionError("unterminated teardown")

    names = ("DiplomacyActive", "DiplomacyAuxiliaryState", "DiplomacyEndState",
             "DiplomacyContactResult", "DiploFocusX", "DiploFocusY")
    definitions = []
    for name in names:
        matches = re.findall(r"^int " + name + r"(?: = -?\d+)?;", source, re.M)
        assert len(matches) == 1, name
        definitions.append(matches[0])
    harness = r'''
#include <cassert>
#define __cdecl
'''
    harness += "\n".join(definitions)
    harness += r'''
int stage;
struct DiploPop {
    void stop() {
        assert(stage++ == 0);
        assert(DiplomacyActive == 0 && DiplomacyAuxiliaryState == 0);
        assert(DiplomacyEndState == 0 && DiplomacyContactResult == 0);
        assert(DiploFocusX == 12 && DiploFocusY == 34);
        // The real services may publish fresh state; teardown must not
        // repeat the initial clears after callbacks have returned.
        DiplomacyContactResult = 9;
        DiploFocusX = 56;
    }
} DiplomacyPopup;
void synch_diplo(int faction, int other) {
    assert(stage++ == 1 && faction == 2 && other == 7);
    assert(DiplomacyContactResult == 9 && DiploFocusX == 56);
    DiplomacyEndState = 8;
    DiploFocusY = 78;
}
struct NetDaemon {
    void close_channel() {
        assert(stage++ == 2);
        assert(DiplomacyEndState == 8 && DiplomacyContactResult == 9);
        assert(DiploFocusX == 56 && DiploFocusY == 78);
        DiplomacyAuxiliaryState = 11;
    }
} NetDaemonNet;
'''
    harness += body
    harness += r'''
int main() {
    assert(DiplomacyActive == 0 && DiplomacyAuxiliaryState == 0);
    assert(DiplomacyEndState == 0 && DiplomacyContactResult == 0);
    assert(DiploFocusX == -1 && DiploFocusY == -1);
    DiplomacyActive = 1; DiplomacyAuxiliaryState = 2;
    DiplomacyEndState = 3; DiplomacyContactResult = 4;
    DiploFocusX = 12; DiploFocusY = 34;
    diplomacy_ends(2, 7);
    assert(stage == 3 && DiploFocusX == -1 && DiploFocusY == -1);
    assert(DiplomacyActive == 0 && DiplomacyAuxiliaryState == 11);
    assert(DiplomacyEndState == 8 && DiplomacyContactResult == 9);
}
'''
    cpp = tmp_path / "diplomacy_end.cpp"
    executable = tmp_path / "diplomacy_end"
    cpp.write_text(harness)
    compiled = subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra",
                               "-Werror", str(cpp), "-o", str(executable)],
                              capture_output=True, text=True, timeout=30)
    assert compiled.returncode == 0, compiled.stderr
    ran = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
    assert ran.returncode == 0, ran.stderr
