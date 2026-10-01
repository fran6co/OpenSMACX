"""Live borrowed animation-buffer access; not native ABI/runtime acceptance."""
from pathlib import Path
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


def braced(source, signature):
    start = source.index(signature)
    depth = 0
    for end in range(source.index("{", start), len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated source")


def test_animation_view_observes_live_owned_slot(tmp_path):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    source = (ROOT / "src/palette.h").read_text()
    fields = braced(source, "struct PaletteInternal {") + " internal_[5];"
    getter = braced(source, "const void *animation_colours(int slot) const")
    harness = """
#include <cassert>
#include <cstdint>
struct Time;
struct Palette {
ACTUAL_FIELDS
ACTUAL_GETTER
};
int main() {
    Palette palette = {};
    const Palette &borrowed = palette;
    unsigned char first[8] = {}, second[8] = {};
    assert(borrowed.animation_colours(4) == nullptr);
    palette.internal_[4].colours = first;
    assert(borrowed.animation_colours(4) == first);
    palette.internal_[0].colours = second;
    assert(borrowed.animation_colours(0) == second);
    assert(borrowed.animation_colours(4) == first);
    palette.internal_[4].colours = second;
    assert(borrowed.animation_colours(4) == second);
    palette.internal_[4].colours = nullptr;
    assert(borrowed.animation_colours(4) == nullptr);
    for (int slot = 1; slot < 4; ++slot)
        assert(borrowed.animation_colours(slot) == nullptr);
}
""".replace("ACTUAL_FIELDS", fields).replace("ACTUAL_GETTER", getter)
    cpp = tmp_path / "palette_view.cpp"
    exe = tmp_path / "palette_view"
    cpp.write_text(harness)
    result = subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                             str(cpp), "-o", str(exe)], capture_output=True,
                            text=True, timeout=30)
    assert result.returncode == 0, result.stderr
    result = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)
    assert result.returncode == 0, result.stderr
