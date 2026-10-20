"""Host arithmetic controls for the 0x0048D790 unsigned initializer.

These exercise the actual source RHS at high-bit and wraparound boundaries.
They establish modulo-32 behavior only, not VC6 load order, relocation identity,
CRT selection, or startup ordering.
"""
from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/prefwin_geometry.cpp").is_file())
SOURCE = ROOT / "src/prefwin_geometry.cpp"
DECLARATION = re.compile(
    r"^int32_t PrefWinGeometryValue_8577E0 = (.*);$", re.MULTILINE
)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_initializer_rhs_preserves_modulo_32_arithmetic(
        tmp_path: Path, compiler_name: str) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")

    source = SOURCE.read_text(encoding="utf-8")
    match = DECLARATION.search(source)
    assert match is not None
    rhs = match.group(1)
    test_source = tmp_path / "prefwin_reference_order.cpp"
    binary = tmp_path / "prefwin_reference_order"
    test_source.write_text(
        """#include <cassert>
#include <cstdint>
int32_t PrefWinGeometryValue_8578B0;
int32_t PrefWinGeometryValue_8577DC;
int32_t PrefWinGeometryConstant_686B78;
int32_t evaluate() { return """ + rhs + """; }
static void check(uint32_t a, uint32_t b, uint32_t c) {
    PrefWinGeometryValue_8578B0 = static_cast<int32_t>(a);
    PrefWinGeometryConstant_686B78 = static_cast<int32_t>(b);
    PrefWinGeometryValue_8577DC = static_cast<int32_t>(c);
    const uint32_t expected = a - c - b;
    assert(static_cast<uint32_t>(evaluate()) == expected);
}
int main() {
    check(0x80000000u, 0x7fffffffu, 0xffffffffu);
    check(0xffffffffu, 0x80000000u, 0x7fffffffu);
    check(0u, 0xffffffffu, 0x80000000u);
    check(0x7fffffffu, 0x80000000u, 1u);
}
""",
        encoding="utf-8",
    )
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         str(test_source), "-o", str(binary)],
        capture_output=True, text=True, timeout=60,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True,
                         timeout=30)
    assert ran.returncode == 0, ran.stdout + ran.stderr
