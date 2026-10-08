"""Shared original string identity and actual measurement-block controls."""

from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
BASE = "9053d5d4a"


def test_join_owner_replacement_preserves_every_concat_operand_and_order():
    pattern = re.compile(r"strcat\([^;]+;")
    for name, count in (("popup", 1), ("designwin", 14)):
        path = f"src/{name}.cpp"
        old = subprocess.check_output(["git", "show", f"{BASE}:{path}"], cwd=ROOT, text=True)
        current = (ROOT / path).read_text()
        normalize = lambda text: text.replace(
            "reinterpret_cast<const char *>(SpaceText)", "BufferJoinSpace"
        ).replace("SpaceText", "BufferJoinSpace")
        assert [normalize(call) for call in pattern.findall(old)] == pattern.findall(current)
        assert len(re.findall(r"strcat\(StringTemp, (?:reinterpret_cast<const char \*>\(SpaceText\)|SpaceText)\);", old)) == count
        assert "SpaceText" not in current
    owner = (ROOT / "src/buffer.cpp").read_text()
    assert '// RECOVERY_OBJECT: v1 0x00682820 ?BufferJoinSpace@@3QBDB\nextern const char BufferJoinSpace[] = " ";' in owner
    assert '// RECOVERY_OBJECT: v1 0x00696CD8 ?BufferMeasureSpace@@3QBDB\nextern const char BufferMeasureSpace[] = " ";' in owner


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_buffer_prompt_measurement_block(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    text = (ROOT / "src/buffer.cpp").read_text()
    start = text.index("char *prompt = const_cast<char *>(BufferMeasureSpace);")
    end = text.index("\n            }", start) + len("\n            }")
    block = text[start:end]
    source = r'''
#include <cassert>
#include <cstring>
extern const char BufferMeasureSpace[] = " ";
static int BufferStrHeight = 5;
static int calls;
static int text_width(char *text, int length) {
    assert(text == BufferMeasureSpace);
    assert(length == 1 && text[0] == ' ' && text[1] == 0);
    ++calls;
    return 7;
}
int main() {
    MEASURE_BLOCK
    assert(calls == 1 && BufferStrHeight == 12);
}
'''.replace("MEASURE_BLOCK", block)
    path = tmp_path / "measure.cpp"
    binary = tmp_path / "measure"
    path.write_text(source)
    subprocess.run([compiler, "-std=c++03", "-Wall", "-Wextra", "-Werror",
                    str(path), "-o", str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
