"""Controls for both actual-source Council notification reset overloads."""
from pathlib import Path
import shutil
import subprocess

import pytest


def test_notification_reset_uses_shared_faction_arrays(tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    source = (Path(__file__).resolve().parents[2] / "src/general.cpp").read_text()
    bodies = []
    for signature in ("void __cdecl clear_council_notify()",
                      "void __cdecl clear_council_notify(int a1)"):
        start = source.index(signature)
        opening = source.index("{", start)
        depth = 0
        for end in range(opening, len(source)):
            depth += (source[end] == "{") - (source[end] == "}")
            if depth == 0:
                bodies.append(source[start:end + 1])
                break
        else:
            raise AssertionError(f"unterminated production body: {signature}")
    harness = "#include <cassert>\n#include <cstring>\n#define __cdecl\n"
    harness += "int CouncilNotifyFormSlots[8], CouncilNotifyPluralSlots[8];\n"
    harness += "\n".join(bodies)
    harness += r'''
int main() {
    for(int i=0;i<8;++i) { CouncilNotifyFormSlots[i]=i+1; CouncilNotifyPluralSlots[i]=i+9; }
    clear_council_notify(3);
    for(int i=0;i<8;++i) {
        assert(CouncilNotifyFormSlots[i]==(i==3?0:i+1));
        assert(CouncilNotifyPluralSlots[i]==(i==3?0:i+9));
    }
    clear_council_notify();
    for(int i=0;i<8;++i) assert(!CouncilNotifyFormSlots[i] && !CouncilNotifyPluralSlots[i]);
}
'''
    unit = tmp_path / "notifications.cpp"
    unit.write_text(harness)
    executable = tmp_path / "notifications"
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    str(unit), "-o", str(executable)], check=True,
                   capture_output=True, text=True)
    subprocess.run([str(executable)], check=True, capture_output=True, text=True)
