"""Exercise the actual map setter bodies with their original operand widths."""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
MAP_SOURCE = ROOT / "src/map.cpp"


def definition(source: str, signature: str) -> str:
    starts = [i for i in range(len(source)) if source.startswith(signature, i)]
    assert len(starts) == 1, f"expected one definition for {signature!r}"
    opening = source.index("{", starts[0])
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[starts[0]:index + 1]
    raise AssertionError(f"unterminated definition for {signature!r}")


def actual_bodies():
    source = MAP_SOURCE.read_text(encoding="utf-8")
    return {
        "site": definition(source, "void __cdecl site_set(int x, int y, int site)"),
        "rocky": definition(source, "void __cdecl rocky_set(int x, int y, int rocky)"),
        "code": definition(source, "void __cdecl code_set(int x, int y, int code)"),
    }


HARNESS_PREFIX = r"""
#include <assert.h>
#include <limits.h>
#include <stdint.h>
#define __cdecl

struct Map {
    uint8_t val2;
    uint8_t val3;
    uint32_t bit2;
};

static Map tile;
static int map_calls;
static int last_x;
static int last_y;
uint32_t UnkBitfield1;

Map *map_loc(int x, int y) {
    ++map_calls;
    last_x = x;
    last_y = y;
    return &tile;
}

void __cdecl site_set(int x, int y, int site);
void __cdecl rocky_set(int x, int y, int rocky);
void __cdecl code_set(int x, int y, int code);

static void check_map_loc() {
    assert(map_calls == 1);
    assert(last_x == -17);
    assert(last_y == 29);
}

static void reset_call(int x, int y) {
    map_calls = 0;
    last_x = x;
    last_y = y;
}

static void test_site(int value, uint8_t initial, uint32_t flags) {
    tile.val2 = initial;
    tile.val3 = 0xA5;
    tile.bit2 = 0x87654321u;
    UnkBitfield1 = flags;
    reset_call(-17, 29);
    site_set(-17, 29, value);
    const uint8_t expected = static_cast<uint8_t>((initial & 0x0Fu) |
        ((static_cast<uint8_t>(value) & 0x0Fu) * 16u));
    assert(tile.val2 == expected);
    assert(tile.val3 == 0xA5);
    assert(tile.bit2 == 0x87654321u);
    assert(UnkBitfield1 == flags);
    check_map_loc();
}

static void test_rocky(int value, uint8_t initial, uint32_t flags) {
    tile.val2 = 0x5A;
    tile.val3 = initial;
    const uint8_t expected = static_cast<uint8_t>((initial & 0x3Fu) |
        ((static_cast<uint8_t>(value) & 0x03u) * 64u));
    const uint32_t bit2_initials[] = {0x87254321u, 0x87654321u};
    for (unsigned i = 0; i < sizeof(bit2_initials) / sizeof(bit2_initials[0]); ++i) {
        tile.bit2 = bit2_initials[i];
        UnkBitfield1 = flags;
        reset_call(-17, 29);
        rocky_set(-17, 29, value);
        assert(tile.val2 == 0x5A);
        assert(tile.val3 == expected);
        assert(tile.bit2 == (bit2_initials[i] | 0x00400000u));
        assert(UnkBitfield1 == (flags | 1u));
        check_map_loc();
    }
}

static void test_code(int value, uint32_t initial, uint32_t flags) {
    tile.val2 = 0x5A;
    tile.val3 = 0xA5;
    tile.bit2 = initial;
    UnkBitfield1 = flags;
    reset_call(-17, 29);
    code_set(-17, 29, value);
    const uint32_t expected = (initial & 0x00FFFFFFu) |
        ((static_cast<uint32_t>(value) & 0xFFu) * 0x01000000u);
    assert(tile.val2 == 0x5A);
    assert(tile.val3 == 0xA5);
    assert(tile.bit2 == expected);
    assert(UnkBitfield1 == (flags | 4u));
    check_map_loc();
}
"""


def fixture(bodies, main: str) -> str:
    return "\n".join((
        HARNESS_PREFIX,
        bodies["site"],
        bodies["rocky"],
        bodies["code"],
        main,
    ))


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_setters_preserve_low_byte_and_modulo32_semantics(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")

    bodies = actual_bodies()
    values = """
    int main() {
        for (int value = 0; value < 256; ++value) {
            test_site(value, static_cast<uint8_t>(value * 73), 0x80000011u);
            test_rocky(value, static_cast<uint8_t>(value * 29), 0x80000010u);
            test_code(value, 0xA5C39E71u ^ static_cast<uint32_t>(value), 0x80000020u);
        }
        const int representatives[] = {
            -1, -2, -127, -128, -255, -256, -257,
            256, 257, 0x12345678, INT_MIN, INT_MAX
        };
        for (unsigned i = 0; i < sizeof(representatives) / sizeof(representatives[0]); ++i) {
            const int value = representatives[i];
            test_site(value, static_cast<uint8_t>(0x93u + i), 0x40000001u);
            test_rocky(value, static_cast<uint8_t>(0xE3u - i), 0x20000008u);
            test_code(value, 0x5A783D91u + i, 0x10000002u);
        }
        return 0;
    }
    """


    cpp = tmp_path / "map_setter_widths_actual.cpp"
    binary = tmp_path / "map_setter_widths_actual"
    cpp.write_text(fixture(bodies, values), encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         "-fsanitize=undefined", "-fno-sanitize-recover=undefined",
         str(cpp), "-o", str(binary)],
        capture_output=True, text=True, timeout=60,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=20)
    assert ran.returncode == 0, ran.stdout + ran.stderr


ROCKY_BIT2_HARNESS = r"""
int main() {
    tile.val2 = 0x5A;
    tile.val3 = 0xA5;
    tile.bit2 = 0x87254321u;
    UnkBitfield1 = 0x20000008u;
    reset_call(-17, 29);
    rocky_set(-17, 29, 3);
    assert(tile.bit2 == 0x87654321u);
    assert(UnkBitfield1 == 0x20000009u);
    assert(map_calls == 1 && last_x == -17 && last_y == 29);
    return 0;
}
"""


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("setter", ("site", "rocky", "code"))
def test_actual_signed_shift_mutants_are_rejected_by_ubsan(compiler_name, setter, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")

    bodies = actual_bodies()
    if setter == "site":
        bodies["site"] = bodies["site"].replace(
            "    const uint8_t site_byte = static_cast<uint8_t>(site);\n"
            "    *field = static_cast<uint8_t>((*field & 0x0F) | (site_byte << 4));",
            "    *field = static_cast<uint8_t>((*field & 0x0F) | (site << 4));",
            1,
        )
        call = "site_set(-17, 29, -1);"
    elif setter == "rocky":
        bodies["rocky"] = bodies["rocky"].replace(
            "    const uint8_t rocky_byte = static_cast<uint8_t>(rocky);\n"
            "    *field = static_cast<uint8_t>(masked | (rocky_byte << 6));",
            "    *field = masked | (rocky << 6);",
            1,
        )
        call = "rocky_set(-17, 29, -1);"
    else:
        bodies["code"] = bodies["code"].replace(
            "(static_cast<uint32_t>(code) << 24)", "(code << 24)", 1
        )
        call = "code_set(-17, 29, -1);"

    assert bodies[setter] != actual_bodies()[setter], "expected the source mutant to apply"
    cpp = tmp_path / f"map_setter_{setter}_signed_mutant.cpp"
    binary = tmp_path / f"map_setter_{setter}_signed_mutant"
    cpp.write_text(fixture(bodies, f"int main() {{ {call} return 0; }}"), encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-fsanitize=undefined",
         "-fno-sanitize-recover=undefined", str(cpp), "-o", str(binary)],
        capture_output=True, text=True, timeout=60,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=20)
    assert ran.returncode != 0
    assert "runtime error" in ran.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_rocky_bit2_mutant_is_rejected(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")

    bodies = actual_bodies()
    bodies["rocky"] = bodies["rocky"].replace(
        "tile->bit2 |= 0x400000; // TODO: identify value",
        "/* mutant removes the original bit2 OR-set */",
        1,
    )
    assert bodies["rocky"] != actual_bodies()["rocky"], "expected bit2 mutant to apply"
    cpp = tmp_path / "map_setter_rocky_bit2_mutant.cpp"
    binary = tmp_path / "map_setter_rocky_bit2_mutant"
    cpp.write_text(fixture(bodies, ROCKY_BIT2_HARNESS), encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", str(cpp), "-o", str(binary)],
        capture_output=True, text=True, timeout=60,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=20)
    assert ran.returncode != 0
