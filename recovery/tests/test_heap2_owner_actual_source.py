"""Permanent-test candidate: exercise only OpenSMACX's current source files.

When promoted to recovery/tests this resolves its root from the checkout and
never opens build/recovery or a proposal overlay.
"""
from __future__ import annotations

import os
from pathlib import Path
import re
import shlex
import shutil
import subprocess
import tempfile

import pytest

from match_recovery.binding_source import parse as parse_bindings
from match_recovery.source import read


ADDRESS = 0x005D4540
OLD_REDIRECT_SYMBOL = "?field_accessor_005d4540_redirect@@YIXPAX0@Z"
EXPECTED_STORE_ORDER = ["err_flags_", "current_", "base_", "free_size_", "base_size_"]


def _root() -> Path:
    for parent in Path(__file__).resolve().parents:
        if ((parent / "CMakeLists.txt").is_file()
                and (parent / "src/heap.cpp").is_file()
                and (parent / "src/field_accessors.cpp").is_file()):
            return parent
    raise AssertionError("could not locate OpenSMACX source root")


def _method(source: str) -> str:
    matches = list(re.finditer(r"(?m)^void\s+Heap::Heap2\s*\(\)\s*\{", source))
    assert len(matches) == 1, "Heap::Heap2 must be one real product definition"
    start = matches[0].start()
    brace = source.find("{", matches[0].start())
    depth = 0
    for position in range(brace, len(source)):
        if source[position] == "{":
            depth += 1
        elif source[position] == "}":
            depth -= 1
            if depth == 0:
                return source[start:position + 1]
    raise AssertionError("unterminated Heap::Heap2")


def _assignments(method: str) -> list[str]:
    return re.findall(r"\b(err_flags_|current_|base_|free_size_|base_size_)\s*=", method)


def _compile_run(method: str, expected_success: bool) -> None:
    compiler = shlex.split(os.environ.get("CXX", "g++"))
    if not compiler or shutil.which(compiler[0]) is None:
        pytest.skip("host C++ compiler unavailable")
    source = r'''#include <cassert>
#include <cstdint>
struct Heap {
  int8_t err_flags_;
  void *base_;
  void *current_;
  uint32_t base_size_;
  uint32_t free_size_;
  uint32_t outside_guard_;
  void Heap2();
};
struct GuardedHeap {
  uint32_t before_;
  Heap heap_;
  uint32_t after_;
};
''' + method + r'''
int main() {
  GuardedHeap value;
  value.before_ = 0x12345678u;
  value.after_ = 0x87654321u;
  value.heap_.err_flags_ = 0x5a;
  value.heap_.base_ = reinterpret_cast<void *>(uintptr_t(0x11112222u));
  value.heap_.current_ = reinterpret_cast<void *>(uintptr_t(0x33334444u));
  value.heap_.base_size_ = 0xaabbccddu;
  value.heap_.free_size_ = 0x55667788u;
  value.heap_.outside_guard_ = 0x13579bdfu;
  value.heap_.Heap2();
  assert(value.before_ == 0x12345678u);
  assert(value.after_ == 0x87654321u);
  assert(value.heap_.outside_guard_ == 0x13579bdfu);
  assert(value.heap_.err_flags_ == 0);
  assert(value.heap_.base_ == nullptr);
  assert(value.heap_.current_ == nullptr);
  assert(value.heap_.base_size_ == 0);
  assert(value.heap_.free_size_ == 0);
  return 0;
}
'''
    with tempfile.TemporaryDirectory(prefix="heap2-owner-host-") as temp:
        cpp = Path(temp) / "owner.cpp"
        exe = Path(temp) / "owner"
        cpp.write_text(source)
        built = subprocess.run(
            compiler + ["-std=c++11", "-Wall", "-Wextra", "-Werror",
                        str(cpp), "-o", str(exe)],
            capture_output=True, text=True)
        if built.returncode != 0:
            assert not expected_success, built.stdout + built.stderr
            return
        ran = subprocess.run([str(exe)], capture_output=True, text=True)
        assert (ran.returncode == 0) is expected_success, ran.stdout + ran.stderr


def test_heap2_mapping_moves_with_history_and_binding_to_real_owner():
    root = _root()
    heap_cpp_path = root / "src/heap.cpp"
    heap_header_path = root / "src/heap.h"
    old_cpp_path = root / "src/field_accessors.cpp"
    old_header_path = root / "src/field_accessors.h"
    heap_cpp = heap_cpp_path.read_text()
    heap_header = heap_header_path.read_text()
    old_cpp = old_cpp_path.read_text()
    old_header = old_header_path.read_text()

    old_records = [row for row in read(old_cpp_path) if row.address == ADDRESS]
    records = [row for row in read(heap_cpp_path) if row.address == ADDRESS]
    assert old_records == []
    assert len(records) == 1
    record = records[0]
    assert record.name == "Heap::Heap2"
    assert record.image_spans == ((ADDRESS, ADDRESS + 0x11),)
    assert record.kind == "game"
    assert record.byte_exact is True and record.semantic is False
    assert record.mode.value == "body" and record.state.value == "implemented"
    assert record.source_role.value == "unclassified"
    assert OLD_REDIRECT_SYMBOL not in record.symbol
    assert "field_accessor_005d4540_redirect" not in old_header
    assert "field_accessor_005d4540_redirect" not in old_cpp

    binding = parse_bindings(heap_cpp, read(heap_cpp_path))[record.line]
    assert "member ownership only" in binding["reason"]
    assert "constructor" in binding["reason"] and "lifetime" in binding["reason"]
    assert any("src/heap.h" in evidence for evidence in binding["evidence"])
    assert any("0x005D4560" in evidence for evidence in binding["evidence"])

    for required_catalog_fact in (
            "Purpose: Set 5 field(s) to constants.",
            "Emitted by tools/generate_field_accessors.py",
            "The adapter declares 0 stack dword(s)",
            "size      17 bytes", "// prototype", "// callers   0   call targets   0",
            "// kind      game", "// flags     sp_ready;purged_ok",
            "// calls     (none)", "Return Value: n/a", "Status: Complete",
            "HISTORICAL RECOVERY_ROLE (seed): SCAFFOLD",
            "historical scaffold emitted-symbol fact", OLD_REDIRECT_SYMBOL):
        assert required_catalog_fact in heap_cpp

    assert "  void Heap2();" in heap_header
    assert "Heap::Heap()" in heap_cpp and "0x005D4560" in heap_cpp


def test_actual_heap2_member_preserves_values_store_order_and_bounds():
    root = _root()
    method = _method((root / "src/heap.cpp").read_text())
    assert _assignments(method) == EXPECTED_STORE_ORDER
    _compile_run(method, True)


def test_mutation_controls_reject_wrong_order_missing_store_bits_and_outside_write():
    method = _method((_root() / "src/heap.cpp").read_text())

    reordered = method.replace(
        "free_size_ = 0;\n    base_size_ = 0;",
        "base_size_ = 0;\n    free_size_ = 0;")
    assert reordered != method
    assert _assignments(reordered) != EXPECTED_STORE_ORDER

    missing_store = method.replace("    free_size_ = 0;\n", "")
    assert missing_store != method
    _compile_run(missing_store, False)

    wrong_bits = method.replace("    err_flags_ = 0;", "    err_flags_ = -1;")
    assert wrong_bits != method
    _compile_run(wrong_bits, False)

    outside_write = method.replace(
        "    base_size_ = 0;", "    base_size_ = 0;\n    outside_guard_ = 0;")
    assert outside_write != method
    _compile_run(outside_write, False)
