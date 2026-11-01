"""Source-extracted controls for the original mutable-input trimming loop."""
from pathlib import Path
import shutil
import subprocess

import pytest

from recovery.tests.test_pact_ascent import definition


ROOT = Path(__file__).resolve().parents[2]


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("damage", [False, True])
def test_editbox_text_limit(tmp_path, compiler_name, damage):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    body = definition((ROOT / "src/editbox.cpp").read_text(),
                      r"void\s+EditBox::set_text\s*\(char\s*\*text\)")
    if damage:
        guard = "if (strlen(text) <= field_B14_)"
        assert body.count(guard) == 1
        body = body.replace(guard, "if (false)", 1)
    harness = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
using std::strlen;
using std::strcat;
struct EditBox {
    char text_[256];
    uint32_t field_B14_, field_B18_, field_B40_, field_B44_;
    int redraws;
    void redraw() { ++redraws; }
    void set_text(char *text);
};
@@BODY@@
int main() {
    EditBox box = {};
    strcpy(box.text_, "previous");
    box.field_B14_ = 3;
    box.field_B18_ = box.field_B40_ = 17;
    char over[] = "abcdef";
    box.set_text(over);
    assert(strcmp(over, "abc") == 0);
    assert(strcmp(box.text_, "abc") == 0);
    assert(box.field_B18_ == 0 && box.field_B40_ == 0);
    assert(box.field_B44_ == 3 && box.redraws == 1);
    char exact[] = "xyz";
    box.set_text(exact);
    assert(strcmp(exact, "xyz") == 0 && strcmp(box.text_, exact) == 0);
    box.field_B14_ = 0;
    char zero_limit[] = "input";
    box.set_text(zero_limit);
    assert(zero_limit[0] == 0 && box.text_[0] == 0);
    assert(box.field_B44_ == 0);
    box.field_B14_ = 9;
    char short_input[] = "short";
    box.set_text(short_input);
    assert(strcmp(short_input, "short") == 0);
    assert(strcmp(box.text_, short_input) == 0 && box.field_B44_ == 5);
    box.field_B18_ = box.field_B40_ = 99;
    box.set_text(nullptr);
    assert(box.text_[0] == 0 && box.field_B44_ == 0);
    assert(box.field_B18_ == 0 && box.field_B40_ == 0);
    assert(box.redraws == 5);
    char empty[] = "";
    box.set_text(empty);
    assert(empty[0] == 0 && box.text_[0] == 0 && box.redraws == 6);
}
'''.replace("@@BODY@@", body)
    source, executable = tmp_path / "control.cpp", tmp_path / "control"
    source.write_text(harness)
    built = subprocess.run([compiler, "-std=c++11", str(source), "-o", str(executable)],
                           capture_output=True, text=True, timeout=45)
    assert built.returncode == 0, built.stdout + built.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=15)
    if damage:
        assert result.returncode != 0, "the previous erase-to-empty error escaped"
    else:
        assert result.returncode == 0, result.stdout + result.stderr
