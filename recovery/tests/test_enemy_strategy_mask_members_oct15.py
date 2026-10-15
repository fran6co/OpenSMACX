"""Operand-width controls for enemy_strategy's original byte mask reads."""
from __future__ import annotations

import os
import re
import shutil
import subprocess
import tempfile
from pathlib import Path


ROOT = Path(os.environ.get("OPENSMACX_SOURCE_ROOT", Path(__file__).resolve().parents[2])).resolve()
SOURCE = ROOT / "src/faction_strategy.cpp"
MASK_HEADER = ROOT / "src/game_data_state_window.h"
MASK_FIELD = "GameDataStateStorage.fields_.lock_masks_.enable_mask_"
BYTE_READ = "GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_"


def production_lock_mask_declarations() -> str:
    header = MASK_HEADER.read_text()
    start = header.index("struct GameDataLockMaskBytes {")
    union_start = header.index("union GameDataLockMaskView {", start)
    end = header.index("};", union_start) + 2
    return header[start:end]


def strategy_body(source: str) -> str:
    start = source.index("void __cdecl enemy_strategy(int faction_id)")
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError("unclosed enemy_strategy body")


def test_source_has_twelve_byte_operands_and_retains_dword_snapshot() -> None:
    body = strategy_body(SOURCE.read_text())
    assert body.count(BYTE_READ) == 12
    assert body.count(MASK_FIELD) == 1
    assert ("static_cast<uint32_t>(" + MASK_FIELD + ") & 0xFFu & faction_bit") in body


def operand_control_source(operands: list[str]) -> str:
    assignments = "\n".join(
        f"const uint32_t lane_{index} = ({operand} & probe_bit);"
        for index, operand in enumerate(operands)
    )
    checks = "\n".join(
        f"if (lane_{index} != expected) return {index + 1};"
        for index in range(len(operands))
    )
    source = f'''#include <stdint.h>
{production_lock_mask_declarations()}
struct Fields {{ GameDataLockMaskView lock_masks_; }};
struct Storage {{ Fields fields_; }};
static Storage GameDataStateStorage;
int main() {{
  {{
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0xA50000ADu;
    uint32_t probe_bit = 0x08u;
    uint32_t expected = 0x08u;
    {assignments}
    {checks}
  }}
  {{
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0x01000000u;
    uint32_t probe_bit = 0x01000000u;
    uint32_t expected = 0u;
    {assignments}
    {checks}
  }}
    return 0;
}}
'''
    return source


def test_extracted_byte_operands_use_low_lane() -> None:
    """Exercise source-owned member reads; high probe bits are expression-only."""
    body = strategy_body(SOURCE.read_text())
    operands = re.findall(re.escape(BYTE_READ), body)
    assert len(operands) == 12
    compilers = [compiler for compiler in (shutil.which("g++"), shutil.which("clang++")) if compiler]
    if not compilers:
        import pytest
        pytest.skip("no host C++ compiler available")
    for compiler in compilers:
        with tempfile.TemporaryDirectory(prefix="enemy-strategy-mask-") as directory:
            cpp = Path(directory) / "mask.cpp"
            exe = Path(directory) / "mask"
            cpp.write_text(operand_control_source(operands))
            subprocess.run(
                [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
                check=True,
                timeout=60,
            )
            subprocess.run([str(exe)], check=True, timeout=30)

            wide_operands = [operand.replace(".bytes_.human_mask_", ".enable_mask_")
                             for operand in operands]
            cpp.write_text(operand_control_source(wide_operands))
            subprocess.run(
                [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
                check=True,
                timeout=60,
            )
            mutant = subprocess.run([str(exe)], timeout=30)
            assert mutant.returncode != 0, "dword operand mutant unexpectedly passed expression-level control"
