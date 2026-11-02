"""Actual destination half helper over negative/odd and 32-bit boundary values."""
from pathlib import Path
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


def actual_half():
    source = (ROOT / "src/mapwin.cpp").read_text()
    start = source.index("int map_dest_half(int value)")
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated actual half helper")


HARNESS = r'''
#include <cassert>
#include <cstdint>
#include <limits>
ACTUAL_HELPER
void check(int value) {
    // Wider division plus a negative-remainder correction is independent of
    // the target's signed-shift expression and cannot overflow at INT_MIN.
    const int64_t wide=value;
    const int64_t expected=wide/2-((wide<0 && wide%2!=0)?1:0);
    assert(map_dest_half(value)==expected);
}
int main() {
    const int values[]={std::numeric_limits<int>::min(),
        std::numeric_limits<int>::min()+1,-1025,-1024,-17,-16,-5,-4,-3,-2,-1,
        0,1,2,3,4,5,16,17,1024,1025,std::numeric_limits<int>::max()};
    for(int value:values)check(value);
    uint32_t bits=0x80000001u;
    for(int i=0;i<10000;++i) {
        bits=bits*1664525u+1013904223u;
        const int64_t value=bits<=0x7fffffffu?static_cast<int64_t>(bits)
                                                 :static_cast<int64_t>(bits)-0x100000000ll;
        check(static_cast<int>(value));
    }
}
'''


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("truncate", [False, True])
def test_actual_half_floor_domain(tmp_path, compiler_name, truncate):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    body = actual_half()
    if truncate:
        changed = body.replace("return value >> 1;", "return value / 2;", 1)
        assert changed != body
        body = changed
    source, binary = tmp_path / "half.cpp", tmp_path / "half"
    source.write_text(HARNESS.replace("ACTUAL_HELPER", body))
    built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                            str(source), "-o", str(binary)],
                           capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
    assert (ran.returncode == 0) == (not truncate), ran.stdout + ran.stderr
