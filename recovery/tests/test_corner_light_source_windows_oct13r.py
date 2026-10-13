"""Bounded actual-source controls for the corner-light byte windows.

This compiles the real storage declarations, corner_light_value helper, the
corner-selection switch from get_brighting, and its OceanCornerShades byte
alias snippet. A host reference model checks all four corners and all 4-bit
nibble pairs. It is not a full get_brighting/game runtime or native ABI proof.
"""

from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src" / "mapwin.cpp"


def extract_definition(source: str, signature: str) -> str:
    if source.count(signature) != 1:
        raise AssertionError(f"expected one {signature!r}")
    start = source.index(signature)
    opening = source.find("{", start)
    if opening < 0:
        raise AssertionError(f"missing body for {signature}")
    depth = 0
    state = "code"
    i = opening
    while i < len(source):
        c = source[i]
        n = source[i + 1] if i + 1 < len(source) else ""
        if state == "code":
            if c == "/" and n == "/":
                state = "line"
                i += 2
                continue
            if c == "/" and n == "*":
                state = "block"
                i += 2
                continue
            if c == '"':
                state = "string"
            elif c == "'":
                state = "char"
            elif c == "{":
                depth += 1
            elif c == "}":
                depth -= 1
                if depth == 0:
                    return source[start : i + 1]
        elif state == "line":
            if c == "\n":
                state = "code"
        elif state == "block":
            if c == "*" and n == "/":
                state = "code"
                i += 2
                continue
        else:
            if c == "\\":
                i += 2
                continue
            if (state == "string" and c == '"') or (state == "char" and c == "'"):
                state = "code"
        i += 1
    raise AssertionError(f"unclosed body for {signature}")


def extract_braced(source: str, marker: str) -> str:
    if source.count(marker) != 1:
        raise AssertionError(f"expected one block marker {marker!r}")
    start = source.index(marker)
    opening = source.find("{", start)
    if opening < 0:
        raise AssertionError(f"missing block after {marker}")
    depth = 0
    state = "code"
    i = opening
    while i < len(source):
        c = source[i]
        n = source[i + 1] if i + 1 < len(source) else ""
        if state == "code":
            if c == "/" and n == "/":
                state = "line"
                i += 2
                continue
            if c == "/" and n == "*":
                state = "block"
                i += 2
                continue
            if c == '"':
                state = "string"
            elif c == "'":
                state = "char"
            elif c == "{":
                depth += 1
            elif c == "}":
                depth -= 1
                if depth == 0:
                    return source[start : i + 1]
        elif state == "line":
            if c == "\n":
                state = "code"
        elif state == "block":
            if c == "*" and n == "/":
                state = "code"
                i += 2
                continue
        else:
            if c == "\\":
                i += 2
                continue
            if (state == "string" and c == '"') or (state == "char" and c == "'"):
                state = "code"
        i += 1
    raise AssertionError(f"unclosed block {marker}")


def declaration(source: str, exact_line: str) -> str:
    if source.count(exact_line) != 1:
        raise AssertionError(f"expected one storage declaration: {exact_line}")
    return exact_line


def actual_fragments(source: str):
    declarations = "\n".join(
        declaration(source, line)
        for line in (
            "static signed char AltLightBit200[113];",
            "static int OceanCornerShades[16];",
            "signed char MapLightingTable[4 * 49];",
            "static signed char MapLightingLeadingReadWindow[4];",
            "static signed char MapLightingTrailingReadWindow[4];",
        )
    )
    helper = extract_definition(source, "static __forceinline signed char corner_light_value(int biased_index)")
    get_brighting = extract_definition(source, "int MapWin::get_brighting(int x, int y, int corner)")
    selection = extract_braced(get_brighting, "switch (corner)")
    ocean_start = get_brighting.index("const unsigned int byte =")
    ocean_end = get_brighting.index(";", get_brighting.index("result = byte <", ocean_start)) + 1
    ocean_alias = get_brighting[ocean_start:ocean_end]
    assert "OceanCornerShades[15]" in ocean_alias
    return declarations, helper, selection, ocean_alias


HARNESS = r'''
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#define __forceinline inline

STORAGE_DECLARATIONS

HELPER_DEFINITION

static signed char ocean_alias_value(int index) {
    int result = 0;
    OCEAN_ALIAS
    return static_cast<signed char>(result);
}

static signed char source_corner_value(int corner, int v1, int v2, int v3, int v4) {
    int v0 = 0x5A;
    int result = 0;
    CORNER_SWITCH
    return static_cast<signed char>(result);
}

static signed char reference_byte(int index) {
    if (index < 4) {
        const unsigned char byte =
            reinterpret_cast<const unsigned char *>(&OceanCornerShades[15])[index];
        return byte < 0x80U ? static_cast<signed char>(byte)
                            : static_cast<signed char>(static_cast<int>(byte) - 0x100);
    }
    if (index < 8) return MapLightingLeadingReadWindow[index - 4];
    if (index < 204) return MapLightingTable[index - 8];
    if (index < 208) return MapLightingTrailingReadWindow[index - 204];
    return AltLightBit200[index - 208];
}

static int expected_index(int corner, int v1, int v2, int v3, int v4) {
    switch (corner) {
    case 0: return v1 * 7 + v2;
    case 1: return 0x31 + v2 * 7 + v3;
    case 2: return 0x62 + v3 * 7 + v4;
    default: return 0x93 + v4 * 7 + v1;
    }
}

static void check_all_nibbles() {
    for (int corner = 0; corner < 4; ++corner) {
        for (int first = 0; first < 16; ++first) {
            for (int second = 0; second < 16; ++second) {
                int v1 = 0, v2 = 0, v3 = 0, v4 = 0;
                if (corner == 0) { v1 = first; v2 = second; }
                if (corner == 1) { v2 = first; v3 = second; }
                if (corner == 2) { v3 = first; v4 = second; }
                if (corner == 3) { v4 = first; v1 = second; }
                const int index = expected_index(corner, v1, v2, v3, v4);
                assert(index >= 0 && index <= 267);
                assert(source_corner_value(corner, v1, v2, v3, v4) ==
                       reference_byte(index));
            }
        }
    }
}

int main() {
    assert(sizeof(AltLightBit200) == 113);
    assert(sizeof(OceanCornerShades) == 64);
    assert(sizeof(MapLightingTable) == 196);
    assert(sizeof(MapLightingLeadingReadWindow) == 4);
    assert(sizeof(MapLightingTrailingReadWindow) == 4);
    for (int i = 0; i < 4; ++i) {
        assert(MapLightingLeadingReadWindow[i] == 0);
        assert(MapLightingTrailingReadWindow[i] == 0);
    }

    // Fill each actual source window with distinct signed values. The ocean
    // alias is written through the byte representation of its real final int.
    unsigned char *ocean_tail = reinterpret_cast<unsigned char *>(&OceanCornerShades[15]);
    ocean_tail[0] = 0x80; ocean_tail[1] = 0xFF;
    ocean_tail[2] = 0x7F; ocean_tail[3] = 0x01;
    OceanCornerShades[14] = 0x10203040;
    for (int i = 0; i < 4; ++i) {
        MapLightingLeadingReadWindow[i] = static_cast<signed char>(i - 128);
        MapLightingTrailingReadWindow[i] = static_cast<signed char>(i - 128);
    }
    for (int i = 0; i < 196; ++i) MapLightingTable[i] = static_cast<signed char>((i % 255) - 128);
    for (int i = 0; i < 60; ++i) AltLightBit200[i] = static_cast<signed char>((i % 255) - 128);
    assert(MapLightingLeadingReadWindow[0] == -128);
    assert(MapLightingLeadingReadWindow[1] == -127);
    assert(MapLightingTrailingReadWindow[0] == -128);
    assert(MapLightingTable[1] == -127);
    assert(AltLightBit200[1] == -127);
    for (int i = 0; i < 4; ++i) assert(ocean_alias_value(i) == reference_byte(i));
    check_all_nibbles();

    // A later write must be observed on a subsequent lookup; these are views,
    // not cached or copied table values.
    MapLightingLeadingReadWindow[0] = 73;
    MapLightingTrailingReadWindow[0] = -1;
    MapLightingTable[92] = -1;
    AltLightBit200[0] = -128;
    assert(source_corner_value(0, 0, 4, 0, 0) == 73);
    assert(source_corner_value(3, 1, 0, 0, 8) == -1);
    assert(source_corner_value(2, 0, 0, 0, 2) == -1);
    assert(source_corner_value(3, 5, 0, 0, 8) == -128);
    ocean_tail[0] = 0xFF;
    assert(ocean_alias_value(0) == reference_byte(0));
    assert(source_corner_value(0, 0, 0, 0, 0) == -1);
    return 0;
}
'''


def make_harness(source: str, mutation: str = "") -> str:
    declarations, helper, selection, ocean_alias = actual_fragments(source)
    if mutation == "wrong_bias":
        assert "0x31 + v2 * 7 + v3" in selection
        selection = selection.replace("0x31 + v2 * 7 + v3", "0x30 + v2 * 7 + v3", 1)
    elif mutation == "stale_zero":
        target = "return MapLightingLeadingReadWindow[biased_index - 4];"
        assert target in helper
        helper = helper.replace(target, "return 0;", 1)
    elif mutation == "wrong_ocean_tail":
        assert "OceanCornerShades[15]" in selection
        selection = selection.replace("OceanCornerShades[15]", "OceanCornerShades[14]", 1)
    return (
        HARNESS.replace("STORAGE_DECLARATIONS", declarations)
        .replace("HELPER_DEFINITION", helper)
        .replace("OCEAN_ALIAS", ocean_alias)
        .replace("CORNER_SWITCH", selection)
    )


def compile_and_run(tmp_path: Path, compiler: str, source: str, expect_success: bool):
    source_path = tmp_path / "corner_light_windows.cpp"
    executable = tmp_path / "corner_light_windows"
    source_path.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", "-pedantic",
         str(source_path), "-o", str(executable)],
        text=True, capture_output=True, timeout=45, check=False,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(executable)], text=True, capture_output=True,
                         timeout=15, check=False)
    if expect_success:
        assert ran.returncode == 0, ran.stdout + ran.stderr
    else:
        assert ran.returncode != 0, "the source mutation escaped the byte-window oracle"


def compilers():
    found = [name for name in ("g++", "clang++") if shutil.which(name)]
    assert len(found) == 2, "both GCC and Clang are required for this host control"
    return found


@pytest.mark.parametrize("compiler", compilers())
def test_actual_corner_lookup_windows_cover_every_corner_and_nibble(tmp_path, compiler):
    compile_and_run(tmp_path, compiler, make_harness(SOURCE.read_text(encoding="utf-8")), True)


@pytest.mark.parametrize("compiler", compilers())
@pytest.mark.parametrize("mutation", ("wrong_bias", "stale_zero", "wrong_ocean_tail"))
def test_corner_window_mutations_are_detected(tmp_path, compiler, mutation):
    compile_and_run(tmp_path, compiler,
                    make_harness(SOURCE.read_text(encoding="utf-8"), mutation), False)
