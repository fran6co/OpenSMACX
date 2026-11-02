"""Observe the actual yearmotize body's promoted ctype arguments safely.

The host observer never calls the host CRT ctype routine, because negative
arguments are outside its portable domain. It records values and classifies
only ASCII digits so this control can verify the image's signed-char calls.
"""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = next(p for p in Path(__file__).resolve().parents if (p / "src/game.cpp").is_file())
SOURCE = ROOT / "src/game.cpp"
SIGNATURE = "void __cdecl yearmotize(char *text)"

PREAMBLE = r"""
#include <cassert>
#include <cstdio>
#include <cstring>
#include <vector>

#define __cdecl
static std::vector<int> observed_digit_arguments;
static int host_digit_observer(int value) {
    observed_digit_arguments.push_back(value);
    return value >= '0' && value <= '9';
}
#define isdigit host_digit_observer
#define _itoa host_itoa

struct GameDataFixture {
    struct Fields { int turn_current_num_; } fields_;
} GameDataStateStorage = {{0}};

static int game_year(int turn) { return 2140 + turn; }
static char *host_itoa(int value, char *buffer, int radix) {
    assert(radix == 10);
    std::snprintf(buffer, 80, "%d", value);
    return buffer;
}
"""

LOOP_MAIN = r"""
int main() {
    // Native5A9DC7 increments once before the loop;5A9EA4 returns to
    //5A9DC8, so candidate starts advance one byte, including odd offsets.
    char odd[] = "x1234X";
    yearmotize(odd);
    assert(std::strcmp(odd, "x2140X") == 0);
    char even[] = "xx1234X";
    yearmotize(even);
    assert(std::strcmp(even, "xx2140X") == 0);

    char longer_run[] = "x12345X";
    yearmotize(longer_run);
    assert(std::strcmp(longer_run, "x12345X") == 0);
    char short_tail[] = "x123";
    yearmotize(short_tail);
    assert(std::strcmp(short_tail, "x123") == 0);
    return 0;
}
"""

MAIN = r"""
static void check_digit_offset(int offset) {
    char text[] = {'1', '1', '1', '1', '1', 'X', '\0'};
    text[offset] = static_cast<char>(0xE9);
    observed_digit_arguments.clear();
    yearmotize(text);
    const int expected = static_cast<int>(static_cast<signed char>(0xE9));
    assert(static_cast<int>(static_cast<char>(0xE9)) == expected);
    assert(observed_digit_arguments.size() > static_cast<unsigned>(offset));
    assert(observed_digit_arguments[offset] == expected);
}

int main() {
    for (int offset = 0; offset <= 4; ++offset) {
        check_digit_offset(offset);
    }

    // The image also classifies the byte immediately before a later run.
    char previous[] = {'x', static_cast<char>(0xE9), '1', '2', '3', '4', 'X', '\0'};
    observed_digit_arguments.clear();
    yearmotize(previous);
    const int expected = static_cast<int>(static_cast<signed char>(0xE9));
    assert(observed_digit_arguments.size() >= 8);
    assert(observed_digit_arguments[1] == expected);
    assert(observed_digit_arguments[7] == expected);
    return 0;
}
"""


def extract_body(source: str) -> str:
    start = source.find(SIGNATURE)
    assert start >= 0, "yearmotize product definition is missing"
    brace = source.find("{", start)
    assert brace >= 0, "yearmotize product definition has no body"
    depth = 0
    for cursor in range(brace, len(source)):
        if source[cursor] == "{":
            depth += 1
        elif source[cursor] == "}":
            depth -= 1
            if depth == 0:
                return source[start:cursor + 1]
    raise AssertionError("yearmotize product body is unbalanced")


def compile_and_run(compiler: str, body: str, tmp_path: Path, name: str, main: str = MAIN) -> int:
    cpp = tmp_path / f"{name}.cpp"
    binary = tmp_path / name
    cpp.write_text(PREAMBLE + "\n" + body + "\n" + main)
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(binary)],
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], capture_output=True, timeout=30).returncode


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_yearmotize_passes_signed_byte_values_to_ctype(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")

    body = extract_body(SOURCE.read_text())
    assert "isdigit(static_cast<unsigned char>(run[" not in body
    assert compile_and_run(compiler, body, tmp_path, f"yearmotize-{compiler_name}") == 0

    # Damage control: restoring the old unsigned conversion must be observable.
    mutant = body
    for index in ("0", "1", "2", "3", "4", "-1"):
        old = f"isdigit(run[{index}])"
        new = f"isdigit(static_cast<unsigned char>(run[{index}]))"
        assert old in mutant
        mutant = mutant.replace(old, new, 1)
    assert compile_and_run(compiler, mutant, tmp_path, f"yearmotize-mutant-{compiler_name}") != 0

    assert compile_and_run(compiler, body, tmp_path, f"yearmotize-scan-{compiler_name}", LOOP_MAIN) == 0
    old = "    ++p;\n    do {\n"
    assert body.count(old) == 1
    double_increment = body.replace(old, "    do {\n        ++p;\n", 1)
    assert compile_and_run(
        compiler, double_increment, tmp_path, f"yearmotize-double-increment-{compiler_name}", LOOP_MAIN
    ) != 0
