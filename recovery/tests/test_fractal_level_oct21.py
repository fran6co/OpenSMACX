"""Exercise the actual mapped Fractal::level body with controlled samples."""

from pathlib import Path
import shutil
import subprocess

import pytest

from test_buffer_typed_font_consumers_oct10 import extract_definition


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/world.cpp"
IDENTITY = ("0x005C1FC0", "int Fractal::level(int x_coord, int y_coord, int shift)")

FIXTURE = r'''#include <assert.h>

struct Fractal {
    int values[2];
    int sample_x[2];
    int sample_y[2];
    int calls;
    int sample(int x, int y) {
        assert(calls < 2);
        sample_x[calls] = x;
        sample_y[calls] = y;
        return values[calls++];
    }
    int level(int x_coord, int y_coord, int shift);
    void reset(int coarse, int fine) {
        values[0] = coarse;
        values[1] = fine;
        sample_x[0] = sample_x[1] = -1;
        sample_y[0] = sample_y[1] = -1;
        calls = 0;
    }
};
'''

MAIN = r'''
static void check_coordinates(const Fractal &fractal) {
    assert(fractal.calls == 2);
    assert(fractal.sample_x[0] == 32 && fractal.sample_y[0] == 48);
    assert(fractal.sample_x[1] == 64 && fractal.sample_y[1] == 96);
}

int main() {
    Fractal fractal;

    // The scripted samples verify call order, scaled-local reuse, and the
    // original *5, *2, *7, then right-shift arithmetic.
    fractal.reset(7, 13);
    assert(fractal.level(2, 3, 1) == 1);
    check_coordinates(fractal);

    // Exact zero and exact one-hundred are retained at the clamp boundaries.
    fractal.reset(0, 0);
    assert(fractal.level(2, 3, 1) == 0);
    check_coordinates(fractal);

    fractal.reset(0, 1833); // (1833 * 2 * 7) >> 8 == 100
    assert(fractal.level(2, 3, 1) == 100);
    check_coordinates(fractal);

    // Values outside both ends clamp without changing sample order.
    fractal.reset(1000, 1000);
    assert(fractal.level(2, 3, 1) == 100);
    check_coordinates(fractal);

    fractal.reset(-1, 0);
    assert(fractal.level(2, 3, 1) == 0);
    check_coordinates(fractal);
    return 0;
}
'''


def source_program(mutation=None):
    body = extract_definition(SOURCE.read_text(encoding="utf-8"), *IDENTITY)
    if mutation == "wrong_negative_clamp":
        old = "    return 0;\n}"
        assert body.count(old) == 1
        body = body.replace(old, "    return 1;\n}", 1)
    elif mutation == "wrong_second_octave":
        old = "int fine = sample(coarse_x << shift, coarse_y << shift);"
        new = "int fine = sample(x_coord << shift, y_coord << shift);"
        assert body.count(old) == 1
        body = body.replace(old, new, 1)
    elif mutation is not None:
        raise AssertionError("unknown mutation: " + mutation)
    return FIXTURE + "\n" + body + "\n" + MAIN


def compile_and_run(compiler, program, tmp_path, label):
    cpp = tmp_path / ("fractal_level_" + Path(compiler).name + "_" + label + ".cpp")
    exe = cpp.with_suffix("")
    cpp.write_text(program, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++03", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=45)
    if built.returncode != 0:
        return built, None
    return built, subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_fractal_level_samples_and_clamps(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    built, result = compile_and_run(compiler, source_program(), tmp_path, "actual")
    assert built.returncode == 0, built.stdout + built.stderr
    assert result is not None and result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("mutation", ("wrong_negative_clamp", "wrong_second_octave"))
def test_compile_success_damage_controls_fail(compiler_name, mutation, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    built, result = compile_and_run(compiler, source_program(mutation), tmp_path, mutation)
    assert built.returncode == 0, built.stdout + built.stderr
    assert result is not None and result.returncode != 0, mutation + " escaped the witness"
