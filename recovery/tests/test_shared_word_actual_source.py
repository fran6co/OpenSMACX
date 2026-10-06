"""Actual shared-word bodies with a host compiler, not linked layout proof."""
from pathlib import Path
import shutil
import subprocess

import pytest
from match_recovery.source import read_text


ROOT = Path(__file__).resolve().parents[2]


def test_setter_and_toggle_use_the_same_zero_initialized_word(tmp_path):
    compiler = shutil.which("c++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    text = (ROOT / "src/shared_word.cpp").read_text()
    records = {r.address: r for r in read_text(text, "src/shared_word.cpp")}
    setter, toggle = records[0x44C860], records[0x44C870]
    assert setter.image_spans == ((0x44C860, 0x44C86D),)
    assert toggle.image_spans == ((0x44C870, 0x44C878),)
    assert "int32_t SharedWordValue;" in text
    source = tmp_path / "shared_word.cpp"
    source.write_text(
        "#include <cstdint>\n#include <cassert>\nint32_t SharedWordValue;\n"
        + setter.region + "\n" + toggle.region + "\n"
        + "int main() { assert(SharedWordValue == 0); "
          "toggle_shared_word(); assert(SharedWordValue == 1); "
          "toggle_shared_word(); assert(SharedWordValue == 0); "
          "set_shared_word(0x12345678); toggle_shared_word(); "
          "assert(SharedWordValue == 0x12345679); "
          "set_shared_word(-1); toggle_shared_word(); "
          "assert(SharedWordValue == -2); }\n"
    )
    binary = tmp_path / "shared_word"
    subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(source), "-o", str(binary)], check=True, timeout=30)
    subprocess.run([str(binary)], check=True, timeout=30)
