"""Compile the production PrefWin redraw lifecycle and meaningful mutants.

The doubles distinguish the public GraphicWin-style redraw wrapper from the
control's on_redraw handler, and track the seven trailing FlatButton calls.
This is an actual-source ordering control, not native ABI or GUI proof.
"""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/prefwin.cpp"
SIGNATURE = "void PrefWin::on_redraw()"


def extract_function(source: str) -> str:
    start = source.index(SIGNATURE)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError("unterminated production PrefWin::on_redraw")


FIXTURE = r'''
#include <cassert>
#include <vector>
std::vector<int> events;
struct CheckBox {
    int event;
    void on_redraw() { events.push_back(event); }
    void redraw() { events.push_back(event + 100); on_redraw(); }
};
struct FlatButton {
    int event;
    void redraw() { events.push_back(event); }
};
struct PrefWin {
    CheckBox bscPrefCheckBox_{5}, advPrefCheckBox_{6}, warnPrefCheckBox_{7};
    CheckBox warnContPrefCheckBox_{8}, autoPrefCheckBox_{9};
    CheckBox audioVisPrefCheckBox_{10}, mapPrefCheckBox_{11};
    FlatButton volumeMainToggle_{12}, volumeVoiceToggle_{13};
    FlatButton volumeSFXToggle_{14}, volumeMusicToggle_{15};
    FlatButton okButton_{16}, cancelButton_{17}, restoreDefaultButton_{18};
    void redraw_option() { events.push_back(1); }
    void redraw_volume() { events.push_back(2); }
    void hide_option_buttons() { events.push_back(3); }
    void update_pref_buttons() { events.push_back(4); }
    void on_redraw();
};
BODY
int main() {
    PrefWin window;
    window.on_redraw();
    const std::vector<int> expected{
        1,2,3,4,
        105,5,106,6,107,7,108,8,109,9,110,10,111,11,
        12,13,14,15,16,17,18
    };
    assert(events == expected);
}
'''


def compile_and_run(tmp_path, compiler: str, body: str, stem: str):
    source = tmp_path / f"{stem}.cpp"
    executable = tmp_path / stem
    source.write_text(FIXTURE.replace("BODY", body))
    compiled = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
         str(source), "-o", str(executable)],
        capture_output=True,
        text=True,
    )
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_prefwin_redraw_lifecycle_and_mutants(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")

    body = extract_function(SOURCE.read_text(encoding="utf-8"))
    result = compile_and_run(tmp_path, compiler, body, "prefwin-redraw-actual")
    assert result.returncode == 0, result.stderr

    bypass_wrapper = body.replace(
        "bscPrefCheckBox_.redraw();", "bscPrefCheckBox_.on_redraw();", 1
    )
    assert bypass_wrapper != body
    result = compile_and_run(
        tmp_path, compiler, bypass_wrapper, "prefwin-redraw-bypass-wrapper"
    )
    assert result.returncode != 0

    omit_button = body.replace("  okButton_.redraw();\n", "", 1)
    assert omit_button != body
    result = compile_and_run(
        tmp_path, compiler, omit_button, "prefwin-redraw-omit-button"
    )
    assert result.returncode != 0
