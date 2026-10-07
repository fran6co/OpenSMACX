"""Actual-source behavioral controls for the bounded byte-fill utility."""
from __future__ import annotations

import re
import shutil
import subprocess
import tempfile
import os
from pathlib import Path

from match_recovery import source

ROOT = Path(__file__).resolve().parents[2]


def function(source: str, signature: str) -> str:
    hits = list(re.finditer(re.escape(signature), source))
    assert len(hits) == 1, signature
    start = hits[0].start()
    brace = source.find("{", hits[0].end())
    assert brace >= 0
    depth = 0
    for pos in range(brace, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[start:pos + 1]
    raise AssertionError(f"unterminated source body: {signature}")


def test_bounded_byte_fill_matches_actual_source():
    utility_path = ROOT / "src/byte_utilities.cpp"
    assert utility_path.is_file(), "integrate byte utility source migration first"
    utility = utility_path.read_text(encoding="utf-8")
    actual = function(utility, "void __cdecl fill_eight_bytes_ff")

    requested = os.environ.get("CXX")
    compiler = (shutil.which(requested) if requested else None)
    compiler = compiler or shutil.which("clang++") or shutil.which("g++") or shutil.which("c++")
    assert compiler, "host C++ compiler required for actual-source controls"
    macros = """#include <cstdint>
#include <cstring>
#include <cassert>
#include <climits>
#define __cdecl
#define __stdcall
#define __fastcall
"""
    harness = macros + actual + r'''
int main() {
    alignas(4) unsigned char fill[16];
    std::memset(fill, 0x31, sizeof(fill));
    fill_eight_bytes_ff(fill + 4);
    for (int i = 0; i != 4; ++i) assert(fill[i] == 0x31);
    for (int i = 4; i != 12; ++i) assert(fill[i] == 0xFF);
    for (int i = 12; i != 16; ++i) assert(fill[i] == 0x31);

}
'''
    with tempfile.TemporaryDirectory() as temp:
        cpp = Path(temp) / "actual_source.cpp"
        exe = Path(temp) / "actual_source"
        cpp.write_text(harness, encoding="utf-8")
        result = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra",
                                 "-Werror", str(cpp), "-o", str(exe)],
                                text=True, capture_output=True)
        assert result.returncode == 0, result.stderr
        run = subprocess.run([str(exe)], text=True, capture_output=True)
        assert run.returncode == 0, run.stderr

        # A concrete damaged-owner witness: omission of the last byte write
        # must fail the same behavioral oracle, not merely a lexical check.
        mutant = harness.replace("bytes[7] = fill;", "bytes[7] = 0;")
        assert mutant != harness
        cpp.write_text(mutant, encoding="utf-8")
        damaged = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra",
                                 "-Werror", str(cpp), "-o", str(exe)],
                                text=True, capture_output=True)
        assert damaged.returncode == 0, damaged.stderr
        rejected = subprocess.run([str(exe)], text=True, capture_output=True)
        assert rejected.returncode != 0


def test_identity_claims_and_history_survive_the_owner_transfer():
    utility_path = ROOT / "src/byte_utilities.cpp"
    leaf_path = ROOT / "src/leaf_recoveries.cpp"
    utility_text = utility_path.read_text(encoding="utf-8")
    leaf_text = leaf_path.read_text(encoding="utf-8")
    rows = {row.address: row for row in source.read_text(utility_text, utility_path)}
    expected = {0x0057DEE0: ("sub_57dee0", ((0x0057DEE0, 0x0057DEFC),))}
    assert set(rows) == set(expected)
    for address, (name, spans) in expected.items():
        row = rows[address]
        assert row.name == name
        assert row.image_spans == spans
        assert row.byte_exact is True and row.semantic is False
        assert row.source_role is source.SourceRole.UNCLASSIFIED
        assert row.symbol == "?fill_eight_bytes_ff@@YAXPAX@Z"
        assert "leaf_0057dee0_redirect" not in row.symbol
        assert "RECOVERY_BINDING: v1" in utility_text[
            utility_text.index(f"// ORIGINAL: 0x{address:08X}"):
            utility_text.index(f"// ORIGINAL: 0x{address:08X}") + 1000]
        assert f"// historical original marker: 0x{address:08X}" in utility_text
        assert f"// ORIGINAL: 0x{address:08X}" not in leaf_text
    assert utility_text.count("// HISTORICAL scaffold role: SCAFFOLD") == 1
    leaf_rows = {row.address for row in source.read_text(leaf_text, leaf_path)}
    assert not (set(expected) & leaf_rows)
    assert {0x00408470, 0x005D7A10}.issubset(leaf_rows)
    assert all(next(row for row in source.read_text(leaf_text, leaf_path)
                    if row.address == address).source_role is source.SourceRole.SCAFFOLD
               for address in (0x00408470, 0x005D7A10))
    assert 0x005D5470 in leaf_rows  # keep the receiver-shaped thunk unresolved
