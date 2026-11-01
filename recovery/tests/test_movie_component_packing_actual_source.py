"""Exercise the actual packed-component leaf with original-derived vectors.

The original clamps negative C to 1-(1<<width), propagates signed borrows,
and sets the format bits from the three channel widths. Incoming stack +8
is unused. These checks cover behavior, not native instruction equality.
"""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("optimization", ["-O0", "-O2"])
def test_original_component_saturation_borrows_and_format(
    tmp_path, compiler, optimization
):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"host {compiler} required")

    source = (ROOT / "src/movie_media.cpp").read_text()
    start = source.index('extern "C" int __fastcall sub_5cbc00(')
    end = source.index("\n}\n", start) + 3
    actual_body = source[start:end]
    harness = r"""
#include <cstdint>
#include <cstdio>
#include <climits>
#define __fastcall
static_assert(sizeof(int) == 4 && sizeof(unsigned int) == 4,
              "the recovered leaf uses 32-bit integer arithmetic");
ACTUAL_BODY
int main() {
    struct Case {
        int sample, a, b;
        int widths[3], shifts[3];
        uint32_t expected;
    };
    const Case cases[] = {
        {0, 0, 0, {5, 6, 5}, {11, 5, 0}, 0x00000000u},
        {0, 0, 0, {5, 5, 5}, {10, 5, 0}, 0x80008000u},
        {0, 0, -256, {5, 6, 5}, {11, 5, 0}, 0xffffffe1u},
        {0, 0, -255, {5, 6, 5}, {11, 5, 0}, 0xffe0ffe1u},
        {0, 0, -252, {5, 6, 5}, {11, 5, 0}, 0xffe0ffe1u},
        {0, 0, -251, {5, 6, 5}, {11, 5, 0}, 0xffe0ffe1u},
        {0, 0, 255, {5, 6, 5}, {11, 5, 0}, 0x001f001fu},
        {255, 255, 255, {5, 6, 5}, {11, 5, 0}, 0xffffffffu},
    };
    const int unused_values[] = {0, 5, 6, 16, 255, -1, INT_MIN, INT_MAX};
    for (unsigned i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        const Case &c = cases[i];
        for (unsigned j = 0; j < sizeof(unused_values) / sizeof(unused_values[0]); ++j) {
            const uint32_t result = static_cast<uint32_t>(sub_5cbc00(
                c.sample, c.a, c.b, unused_values[j],
                c.widths[0], c.widths[1], c.widths[2],
                c.shifts[0], c.shifts[1], c.shifts[2]));
            if (result != c.expected) {
                std::fprintf(stderr, "case %u selector %u: %08x != %08x\n",
                             i, j, result, c.expected);
                return 1;
            }
        }
    }
    return 0;
}
""".replace("ACTUAL_BODY", actual_body)
    path = tmp_path / "component_packing.cpp"
    path.write_text(harness)
    output = tmp_path / "component_packing"
    built = subprocess.run(
        [executable, "-std=c++17", optimization, "-Wall", "-Wextra",
         "-Werror", "-Wno-unused-parameter", str(path), "-o", str(output)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stderr
    result = subprocess.run([str(output)], capture_output=True, text=True, timeout=10)
    assert result.returncode == 0, result.stderr
