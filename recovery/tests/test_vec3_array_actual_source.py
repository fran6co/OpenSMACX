"""Actual free-vector bodies and aliasing controls; not native floating-point proof."""
from pathlib import Path
import shutil
import subprocess

import pytest
from match_recovery.source import read_file
from test_game_logic_seed_batch_behavior import _definition

ROOT = Path(__file__).resolve().parents[2]
SPECS = (
    (0x628180, "void __cdecl vec3_subtract(const float *left, const float *right,"),
    (0x6281B0, "void __cdecl vec3_scale(const float *source, float scale,"),
    (0x6281E0, "float __cdecl vec3_length(const float *vector)"),
)
MAIN = r'''
int main() {
    const float a[3] = {9, -2, 7}, b[3] = {4, 3, -5};
    float out[3];
    vec3_subtract(a, b, out);
    assert(out[0] == 5 && out[1] == -5 && out[2] == 12);
    float in_place[3] = {2, -3, 4};
    vec3_scale(in_place, 2.5f, in_place);
    assert(in_place[0] == 5 && in_place[1] == -7.5f && in_place[2] == 10);
    float overlap[4] = {1, 2, 3, 4};
    vec3_scale(overlap, 2, overlap + 1);
    assert(overlap[1] == 2 && overlap[2] == 4 && overlap[3] == 8);
    const float length[3] = {3, 4, 12}, negative[3] = {-3, -4, -12};
    assert(vec3_length(length) == 13 && vec3_length(negative) == 13);
}
'''


def test_array_mapping_regions_keep_their_own_definitions():
    rows = {r.address: r for r in read_file(ROOT / "src/vector.cpp")}
    specs = ((0x628150, "void __cdecl vector_add("),) + SPECS
    for address, signature in specs:
        region = rows[address].region
        assert signature in region
        for other_address, other_signature in specs:
            if other_address != address:
                assert other_signature not in region


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_array_operations_and_mutants(tmp_path, compiler):
    if not shutil.which(compiler):
        pytest.skip(compiler + " unavailable")
    rows = {r.address: r for r in read_file(ROOT / "src/vector.cpp")}
    bodies = "\n\n".join(_definition(rows[a].region, signature)
                           for a, signature in SPECS)
    source = '#include <cassert>\n#include <cmath>\n#define __cdecl\n' + bodies + MAIN
    variants = [source,
                source.replace("left[0] - right[0]", "left[0] + right[0]", 1),
                source.replace("result[1] = y * scale;", "result[1] = y + scale;", 1),
                source.replace("+ static_cast<long double>(vector[0]) * vector[0];", ";", 1)]
    assert all(v != source for v in variants[1:])
    for index, variant in enumerate(variants):
        cpp, exe = tmp_path / f"variant{index}.cpp", tmp_path / f"variant{index}"
        cpp.write_text(variant)
        built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                                str(cpp), "-o", str(exe)], capture_output=True, text=True)
        assert built.returncode == 0, built.stderr
        ran = subprocess.run([str(exe)], capture_output=True, text=True)
        assert (ran.returncode == 0) == (index == 0), ran.stderr
