"""The original group-relative list view is a typed, trivial subobject."""
from __future__ import annotations

import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]


def test_group_list_is_typed_at_the_original_receiver_offset(tmp_path: Path) -> None:
    header = (ROOT / "src/wave_device.h").read_text()
    source = (ROOT / "src/wave_device.cpp").read_text()
    group_types = header[header.index("struct WaveGroupNode {"):
                         header.index(" /*\n  * Wave_Device class")]
    assert "RECOVERY_MEMBER: v3 WaveGroupList head WaveGroupNode* 0x0" in group_types
    assert "RECOVERY_MEMBER: v3 WaveGroupList tail WaveGroupNode* 0x4" in group_types
    assert "RECOVERY_MEMBER: v3 WaveGroupList cursor WaveGroupNode* 0x8" in group_types
    assert "RECOVERY_MEMBER: v1 WaveGroupList count 0xC" in group_types
    assert "RECOVERY_MEMBER: v2 WaveControlGroup list_ WaveGroupList 0x8" in group_types
    assert "WaveGroupList();" not in group_types
    assert "~WaveGroupList();" not in group_types

    for signature in (
        "int Wave_Device::add_to_group(",
        "int Wave_Device::pull_from_group(",
        "int Wave_Device::set_group_volume(",
        "int Wave_Device::enable_group(",
        "int Wave_Device::disable_group(",
    ):
        begin = source.index(signature)
        opening = source.index("{", begin)
        depth = 0
        for end in range(opening, len(source)):
            if source[end] == "{":
                depth += 1
            elif source[end] == "}":
                depth -= 1
                if depth == 0:
                    body = source[opening:end + 1]
                    break
        assert "reinterpret_cast<WaveGroupList" not in body, signature
        assert ".list_" in body, signature

    compilers = [compiler for compiler in (shutil.which("g++"), shutil.which("clang++"))
                 if compiler]
    assert compilers
    harness = r"""
#include <cstddef>
#include <type_traits>
#include <cstdint>
using std::uint32_t;
class Wave;
""" + group_types + r"""
#define STATIC_ASSERT(name, expression) typedef char name[(expression) ? 1 : -1]
STATIC_ASSERT(group_list_offset, offsetof(WaveControlGroup, list_) == 0x8);
STATIC_ASSERT(head_offset, offsetof(WaveGroupList, head) == 0x0);
STATIC_ASSERT(tail_offset, offsetof(WaveGroupList, tail) == sizeof(WaveGroupNode *));
STATIC_ASSERT(cursor_offset, offsetof(WaveGroupList, cursor) == 2 * sizeof(WaveGroupNode *));
STATIC_ASSERT(count_offset, offsetof(WaveGroupList, count) == 3 * sizeof(WaveGroupNode *));
STATIC_ASSERT(list_size, sizeof(WaveGroupList) ==
              ((3 * sizeof(WaveGroupNode *) + sizeof(uint32_t) + sizeof(WaveGroupNode *) - 1) /
               sizeof(WaveGroupNode *)) * sizeof(WaveGroupNode *));
static_assert(std::is_trivially_default_constructible<WaveGroupList>::value,
              "list has no added construction behavior");
static_assert(std::is_trivially_destructible<WaveGroupList>::value,
              "list has no added destruction behavior");
int main() {}
"""
    for compiler in compilers:
        with tempfile.TemporaryDirectory(prefix="osmx-wave-list-member-") as directory:
            binary = Path(directory) / "layout"
            built = subprocess.run(
                [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", "-x", "c++", "-", "-o", str(binary)],
                input=harness, text=True, capture_output=True, timeout=45,
            )
            assert built.returncode == 0, built.stdout + built.stderr
            ran = subprocess.run([str(binary)], check=False, timeout=10)
            assert ran.returncode == 0, (compiler, ran.returncode)
