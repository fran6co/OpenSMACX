"""Actual project-help forwarding controls; host double is not game proof."""
from pathlib import Path
import shutil
import subprocess
import pytest
from match_recovery.source import read_file

ROOT = Path(__file__).resolve().parents[2]
HARNESS = r"""
#include <assert.h>

struct Datalink {
    unsigned int last_topic;
    int last_index;
    int calls;

    Datalink() : last_topic(0), last_index(0), calls(0) {}

    void exec(unsigned int topic, int index) {
        last_topic = topic;
        last_index = index;
        ++calls;
    }
};

Datalink DatalinkMain;

#define __cdecl
BODY

int main() {
    for (int project_id = 0; project_id <= 36; ++project_id) {
        const int before = DatalinkMain.calls;
        help_project(project_id);
        assert(DatalinkMain.calls == before + 1);
        assert(DatalinkMain.last_topic == 11u);
        assert(DatalinkMain.last_index == project_id + 70);
    }
    assert(DatalinkMain.calls == 37);
    return 0;
}

"""

@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_project_help(compiler, tmp_path):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(compiler + " unavailable")
    path = ROOT / "src/datalink.cpp"
    record, = (r for r in read_file(path) if r.address == 0x44C9E0)
    assert record.image_spans == ((0x44C9E0, 0x44C9F8),)
    assert record.byte_exact and not record.extract_error
    assert "BODY GOES HERE" not in record.region
    assert "DatalinkMain.exec" in record.region
    text = path.read_text()
    start = text.index("void __cdecl help_project(")
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            body = text[start:end + 1]
            break
    else:
        raise AssertionError("unterminated project helper")
    source = tmp_path / "project-help.cpp"
    source.write_text(HARNESS.replace("BODY", body))
    binary = tmp_path / "project-help"
    subprocess.run([executable, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                    "-fsanitize=undefined", "-fno-sanitize-recover=all",
                    str(source), "-o", str(binary)], check=True,
                   capture_output=True, text=True)
    subprocess.run([str(binary)], check=True, capture_output=True, text=True)
