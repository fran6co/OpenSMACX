"""Actual-source controls for the measured int predicate and byte storage."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def definition(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for index in range(opening, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[start:index + 1]
    raise AssertionError(f"unterminated definition: {signature}")


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_facility_predicates_keep_zero_one_results(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")

    header = (ROOT / "src/base.h").read_text()
    source = (ROOT / "src/base.cpp").read_text()
    signature = "int __cdecl has_fac_built(int facility_id)"
    assert header.count(signature + ";") == 1, "selected-base facility declaration missing"
    assert "inline " + signature not in header
    one_arg = definition(source, signature)
    two_arg = definition(
        header, "inline int __cdecl has_fac_built(int facility_id, int base_id)"
    )
    assert "inline int __cdecl has_fac_built(int facility_id, int base_id)" in two_arg
    assert "int base_id = BaseIDCurrentSelected;" in one_arg
    assert one_arg.index("int base_id = BaseIDCurrentSelected;") < one_arg.index(
        "facility_id >= FacilityRepStart"
    )
    assert "bitmask(facility_id, &facility_id, &mask);" in one_arg

    fixture = r'''
#include <assert.h>
#include <stdint.h>
#include <type_traits>
#define __cdecl
struct Base { uint8_t facilities_built[12]; };
static Base Bases[3];
static int selected_reads;
static int read_selected_base() { ++selected_reads; return 1; }
#define BaseIDCurrentSelected read_selected_base()
static const int FacilityRepStart = 0x41;
static void bitmask(int input, int *offset, int *mask) {
    *offset = input / 8;
    *mask = 1 << (input & 7);
}
''' + two_arg + "\n" + one_arg + r'''
int main() {
    static_assert(std::is_same<decltype(has_fac_built(1, 1)), int>::value,
                  "two-argument helper contract must remain unchanged");
static_assert(std::is_same<decltype(has_fac_built(1)), int>::value,
              "one-argument helper returns the original full-width predicate");

    Bases[1].facilities_built[2] = 1u << 3;
    selected_reads = 0;
    assert(has_fac_built(19) == 1);
    assert(selected_reads == 1);

    selected_reads = 0;
    assert(has_fac_built(18) == 0);
    assert(selected_reads == 1);

    // The selected-base read precedes the facility-ID guard in the source.
    selected_reads = 0;
    assert(has_fac_built(FacilityRepStart) == 0);
    assert(selected_reads == 1);

    assert(has_fac_built(19, 1) == 1);
    assert(has_fac_built(18, 1) == 0);
    assert(selected_reads == 1); // explicit-base overload has no global read
}
'''
    generated = tmp_path / f"facility_result_width_{compiler_name}.cpp"
    generated.write_text(fixture)
    executable = tmp_path / f"facility-result-width-{compiler_name}"
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(generated),
         "-o", str(executable)],
        capture_output=True, text=True, timeout=30, check=False,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(executable)], capture_output=True, text=True,
                         timeout=10, check=False)
    assert ran.returncode == 0, ran.stdout + ran.stderr
