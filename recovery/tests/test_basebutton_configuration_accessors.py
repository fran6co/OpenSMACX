"""Actual-header configuration controls, not VC6 ABI or image-match proof.

The minimal host projection intentionally has no redraw/parent/timer API: these
accessors replace direct application stores, not BaseButton::set notifications.
"""
from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def accessors():
    source = (ROOT / "src/basebutton.h").read_text()
    methods = []
    for name in ("selected_state", "set_selected_state", "set_pressed_visual",
                 "set_field_A44"):
        match = re.search(r"\b(?:int|void)\s+" + name
                          + r"\s*\([^)]*\)\s*(?:const\s*)?\{[^{}]*\}", source)
        assert match, name
        methods.append(match.group())
    return "\n".join(methods)


def projection(methods):
    return r'''
#include <stdint.h>
#include <assert.h>
struct Inspector;
class BaseButton {
    friend struct Inspector;
    uint32_t field_A18_, field_A9C_, field_A44_;
public:
    BaseButton() : field_A18_(17), field_A9C_(29), field_A44_(41) {}
    METHODS
};
struct Inspector {
    static void expect(const BaseButton &button, uint32_t selected,
                       uint32_t pressed, uint32_t unknown) {
        assert(button.field_A18_ == selected);
        assert(button.field_A9C_ == pressed);
        assert(button.field_A44_ == unknown);
    }
};
int main() {
    BaseButton button;
    assert(button.selected_state() == 17);
    button.set_selected_state(0);
    Inspector::expect(button, 0, 29, 41);
    button.set_selected_state(1);
    Inspector::expect(button, 1, 29, 41);
    button.set_pressed_visual(0);
    Inspector::expect(button, 1, 0, 41);
    button.set_field_A44(100);
    Inspector::expect(button, 1, 0, 100);
    button.set_selected_state(-1);
    Inspector::expect(button, UINT32_MAX, 0, 100);
}
'''.replace("METHODS", methods)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("damage", [None, "wrong_word", "notification"])
def test_configuration_is_only_the_requested_store(tmp_path, compiler_name, damage):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    methods = accessors()
    if damage == "wrong_word":
        changed = methods.replace("field_A18_ =", "field_A44_ =", 1)
        assert changed != methods
        methods = changed
    elif damage == "notification":
        changed = methods.replace("void set_selected_state(int value) {",
                                  "void set_selected_state(int value) { redraw();", 1)
        assert changed != methods
        methods = changed
    source, executable = tmp_path / "configuration.cpp", tmp_path / "configuration"
    source.write_text(projection(methods))
    compiled = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra",
                               "-Werror", str(source), "-o", str(executable)],
                              capture_output=True, text=True, timeout=45)
    if damage == "notification":
        assert compiled.returncode != 0, "unexpected notification compiled"
        return
    assert compiled.returncode == 0, compiled.stderr
    run = subprocess.run([str(executable)], capture_output=True, text=True, timeout=15)
    assert (run.returncode == 0) == (damage is None), run.stderr
