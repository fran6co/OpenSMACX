"""Exercise the product say_vehicles body without libc overlap UB."""
from pathlib import Path
import shutil
import subprocess

import pytest

from recovery.tests.test_pact_ascent import ROOT, definition


FIXTURE = r"""
#include <cassert>
#include <cstring>
#define __cdecl

struct VehiclePrototype { char *veh_name; };
VehiclePrototype VehPrototypes[1];
char scratch[128];
char *StringTemp = scratch;
int strcat_calls;
int self_appends;

extern "C" char *strcat(char *destination, const char *source) {
    ++strcat_calls;
    if (destination == source) {
        ++self_appends;
        return destination;
    }
    char *end = destination;
    while (*end != 0) ++end;
    while (*source != 0) *end++ = *source++;
    *end = 0;
    return destination;
}

char *label_get(int index) {
    assert(index == 0x314 / 4);
    return const_cast<char *>("Label");
}

// @@FUNCTION@@

int main(int argc, char **argv) {
    assert(argc == 2);
    char output[128] = "unit";
    VehPrototypes[0].veh_name = const_cast<char *>("Scout");
    strcat_calls = self_appends = 0;
    if (strcmp(argv[1], "separate") == 0) {
        strcpy(scratch, "scratch");
        say_vehicles(output, 0, 1);
        assert(strcmp(output, "unitScout ()") == 0);
        assert(strcmp(scratch, "scratchLabel") == 0);
        assert(strcat_calls == 5 && self_appends == 0);
    } else if (strcmp(argv[1], "alias") == 0) {
        strcpy(scratch, "unit");
        say_vehicles(StringTemp, 0, 1);
        assert(strcmp(scratch, "unitScout (Label)") == 0);
        assert(strcat_calls == 5 && self_appends == 0);
    } else {
        assert(strcmp(argv[1], "plain") == 0);
        strcpy(output, "plain");
        strcpy(scratch, "unchanged");
        say_vehicles(output, 0, 0);
        assert(strcmp(output, "plainScout") == 0);
        assert(strcmp(scratch, "unchanged") == 0);
        assert(strcat_calls == 1 && self_appends == 0);
    }
}
"""


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_vehicle_label_output_and_mutant(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")

    source = (ROOT / "src/map.cpp").read_text()
    body = definition(source, r"void __cdecl say_vehicles\([^)]*\)")
    actual = FIXTURE.replace("// @@FUNCTION@@", body)
    assert "strcat(text, StringTemp);" not in body
    assert body.count("strcat(StringTemp, label_get(0x314 / 4));") == 1

    mutant_body = body.replace(
        'strcat(text, ")");',
        'strcat(text, StringTemp);\n        strcat(text, ")");',
        1,
    )
    assert mutant_body != body
    variants = {
        "actual": actual,
        "extra_scratch_copy": FIXTURE.replace("// @@FUNCTION@@", mutant_body),
    }
    for name, text in variants.items():
        cpp = tmp_path / (name + ".cpp")
        binary = tmp_path / name
        cpp.write_text(text)
        build = subprocess.run(
            [compiler, "-std=c++98", str(cpp), "-o", str(binary)],
            capture_output=True,
            text=True,
            timeout=45,
        )
        assert build.returncode == 0, name + build.stdout + build.stderr
        # Separate processes ensure an earlier nonalias assertion cannot hide
        # the alias-path damage control. The mock traps overlap before copying.
        for case in ("separate", "alias", "plain"):
            run = subprocess.run([str(binary), case], capture_output=True,
                                 text=True, timeout=15)
            expected_pass = name == "actual" or case == "plain"
            assert (run.returncode == 0) == expected_pass, (
                name + ":" + case + run.stdout + run.stderr
            )
