"""Signed continent counts follow the mapped original arithmetic and branches."""

from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
HOST_COMPILER = shutil.which("g++") or shutil.which("clang++")


def _function_body(text, signature):
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[opening + 1:end]
    raise AssertionError(f"unterminated function: {signature}")


def _continent_declaration(header):
    match = re.search(r"\bstruct\s+Continent\s*\{", header)
    assert match
    opening = header.index("{", match.start())
    depth = 0
    for end in range(opening, len(header)):
        depth += (header[end] == "{") - (header[end] == "}")
        if depth == 0:
            assert header[end + 1] == ";"
            return header[match.start():end + 2]
    raise AssertionError("unterminated Continent declaration")


def test_signed_count_types_and_consumers_are_in_actual_sources():
    header = (ROOT / "src/map.h").read_text(encoding="utf-8")
    continent = _continent_declaration(header)
    assert re.search(r"\bint\s+tile_count\s*;", continent)
    assert not re.search(r"\buint32_t\s+tile_count\s*;", continent)
    assert "signed: original uses signed division and ordering comparisons" in continent

    path = (ROOT / "src/path.cpp").read_text(encoding="utf-8")
    continent_body = _function_body(path, "void Path::continent(int x, int y, int region)")
    continents_body = _function_body(path, "void Path::continents()")
    assert re.search(r"\bint\s+fresh_water_count\s*=\s*0\s*;", continent_body)
    assert re.search(r"\bint\s+tiles\s*=\s*Continents\[region_current\]\.tile_count\s*;",
                     continents_body)

    world = (ROOT / "src/map.cpp").read_text(encoding="utf-8")
    fresh_body = _function_body(world, "void __cdecl world_fresh(int x, int y)")
    assert re.search(r"\bint\s+tile_search\s*=\s*0\s*;", fresh_body)
    assert re.search(r"\bint\s+tile_count\s*=\s*Continents\[i\]\.tile_count\s*;",
                     fresh_body)

    base = (ROOT / "src/base.cpp").read_text(encoding="utf-8")
    assert re.search(r"\bint\s+expansion_region_size\s*=\s*0\s*;", base)
    # This call site needs the original dword wrap before signed division.
    assert "(int)((uint32_t)Continents[region].tile_count + 199u) / 200" in base

    strategy = (ROOT / "src/faction_strategy.cpp").read_text(encoding="utf-8")
    assert "Continents[region].tile_count < 0x96u" not in strategy
    assert "Continents[region].tile_count >= 0x28u" not in strategy
    for relative in ("src/base.cpp", "src/enemy.cpp", "src/faction.cpp", "src/map.cpp"):
        source = (ROOT / relative).read_text(encoding="utf-8")
        assert not re.search(r"(?:static_cast<int>\s*\(\s*Continents\[[^]]+\]\.tile_count\s*\)|"
                             r"\(int\)\s*Continents\[[^]]+\]\.tile_count)", source), relative


@pytest.mark.parametrize("compiler", [HOST_COMPILER], ids=lambda value: Path(value).name if value else "no-host-cxx")
def test_actual_continent_declaration_and_source_expression_have_signed_behavior(
        tmp_path, compiler):
    if compiler is None:
        pytest.skip("host GCC or Clang required")
    header = (ROOT / "src/map.h").read_text(encoding="utf-8")
    declaration = _continent_declaration(header)
    path = (ROOT / "src/path.cpp").read_text(encoding="utf-8")
    match = re.search(
        r"BOOL\s+is_fresh_water\s*=\s*(fresh_water_count\s*>=\s*"
        r"\(\(Continents\[region\]\.tile_count\s*\*\s*3\)\s*/\s*4\))\s*;",
        path)
    assert match, "expected source-derived signed fresh-water expression"
    expression = match.group(1).replace("fresh_water_count", "fresh_count")
    expression = expression.replace("Continents[region]", "continent")

    base = (ROOT / "src/base.cpp").read_text(encoding="utf-8")
    base_body = _function_body(base, "int __cdecl base_build(")
    base_match = re.search(
        r"v11\s*=\s*(\(int\)\(\(uint32_t\)Continents\[region\]\.tile_count"
        r"\s*\+\s*199u\)\s*/\s*200)\s*;", base_body)
    assert base_match, "expected source-preserved dword-wrap and signed division"
    base_expression = base_match.group(1).replace(
        "Continents[region]", "continent")

    source = tmp_path / "continent_count.cpp"
    source.write_text("""
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <type_traits>
""" + declaration + """
static_assert(std::is_same<decltype(Continent::tile_count), int>::value,
              "tile_count must retain signed int type");
static_assert(offsetof(Continent, tile_count) == 0, "tile_count offset changed");
static_assert(sizeof(Continent) == 28, "Continent stride changed");
static bool fresh(int fresh_count, const Continent &continent) {
    return """ + expression + """;
}
static int build_count(const Continent &continent) {
    return """ + base_expression + """;
}
int main() {
    Continent continent = {};
    continent.tile_count = 8;
    assert(fresh(6, continent));
    assert(!fresh(5, continent));
    // A negative signed value is safe here (no overflow) and distinguishes
    // signed multiply/divide from the former uint32_t member behavior.
    continent.tile_count = -1;
    assert(fresh(1, continent));
    continent.tile_count = INT32_MAX;
    assert(build_count(continent) == -10737417);
    continent.tile_count = INT32_MAX - 198;
    assert(build_count(continent) == -10737418);
    continent.tile_count = -199;
    assert(build_count(continent) == 0);
    continent.tile_count = -200;
    assert(build_count(continent) == 0);
}
""", encoding="utf-8")
    binary = tmp_path / "continent_count"
    compiled = subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                               str(source), "-o", str(binary)], capture_output=True, text=True)
    assert compiled.returncode == 0, compiled.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True)
    assert ran.returncode == 0, ran.stdout + ran.stderr


def test_pinned_signed_arithmetic_and_branch_mnemonics_use_complete_mapped_spans():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source import read_file
    from match_recovery.source_inventory import image_pin

    image_path = Path(os.environ.get(
        "OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not image_path.is_file():
        pytest.skip("pinned original required for instruction verification")
    image = PeImage(image_path.read_bytes(), image_pin(ROOT))
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True

    witness_groups = (
        ("src/path.cpp", 0x0059C520, {
            0x0059C700: None,
            0x0059C713: "setge",
        }),
        ("src/path.cpp", 0x0059C790, {0x0059C8C8: "jge"}),
        ("src/map.cpp", 0x005C6C40, {
            0x005C6C84: "jl", 0x005C6C89: "jg", 0x005C6C8D: "jl",
        }),
        ("src/map.cpp", 0x005C2CB0, {0x005C3030: "jl", 0x005C303B: "jle"}),
        ("src/base.cpp", 0x004ECBF0, {0x004ED1CB: "jle"}),
        ("src/base.cpp", 0x004F81A0, {
            0x004FABC3: None, 0x004FABCA: None, 0x004FABD0: None,
            0x004FABD2: None, 0x004FABD7: None, 0x004FABDA: None,
        }),
        ("src/faction_strategy.cpp", 0x00561080, {
            0x005630F1: "jge", 0x005633CA: "jl",
        }),
        ("src/technology.cpp", 0x005BCBE0, {0x005BDBBB: "jge"}),
    )
    for relative_path, owner_address, expected in witness_groups:
        records = read_file(ROOT / relative_path)
        record, = [row for row in records if row.address == owner_address]
        assert len(record.image_spans) == 1
        low, high = record.image_spans[0]
        assert low == owner_address
        instructions = list(decoder.disasm(image.read(Span(low, high)), low))
        by_address = {}
        cursor = low
        for instruction in instructions:
            assert instruction.address == cursor, f"undecoded byte at {cursor:#x}"
            by_address[instruction.address] = instruction
            cursor += instruction.size
        assert cursor == high
        for address, mnemonic in expected.items():
            instruction = by_address[address]
            if mnemonic is not None:
                assert instruction.mnemonic == mnemonic
        if owner_address == 0x0059C520:
            assert [(by_address[address].mnemonic, by_address[address].op_str)
                    for address in (0x0059C700, 0x0059C703, 0x0059C704,
                                    0x0059C707, 0x0059C709)] == [
                ("lea", "eax, [eax + eax*2]"),
                ("cdq", ""),
                ("and", "edx, 3"),
                ("add", "eax, edx"),
                ("sar", "eax, 2"),
            ]
        if owner_address == 0x004F81A0:
            assert [(by_address[address].mnemonic, by_address[address].op_str)
                    for address in (0x004FABC3, 0x004FABCA, 0x004FABD0,
                                    0x004FABD2, 0x004FABD7, 0x004FABDA)] == [
                ("mov", "ecx, dword ptr [ecx*4 + 0x9aa730]"),
                ("add", "ecx, 0xc7"),
                ("imul", "ecx"),
                ("sar", "edx, 6"),
                ("shr", "eax, 0x1f"),
                ("add", "edx, eax"),
            ]
