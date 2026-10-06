"""Host control for the installed 0x402EE0 source body (not a proposal copy).

The host substitutes only the platform calling-convention spellings. The
loop is extracted from src/vector_teardown.cpp and its typed row bridge from
src/netwin.h on every run, then compiled and executed.
"""
from __future__ import annotations

from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest


SIGNATURE = 'extern "C" void __stdcall sub_402ee0('
BRIDGE_SIGNATURE = "static void __fastcall initialize_empty_slot_callback(void *element)"
CALL_SITE = "callback(element);"


def repo_root() -> Path:
    for parent in Path(__file__).resolve().parents:
        if (parent / "src/vector_teardown.cpp").is_file() and (parent / "CMakeLists.txt").is_file():
            return parent
    raise AssertionError("could not locate the OpenSMACX source tree")


def extract_function(source: str, signature: str) -> str:
    assert source.count(signature) == 1, f"expected exactly one installed definition for {signature}"
    start = source.index(signature)
    opening = source.index("{", start + len(signature))
    depth = 0
    state = "code"
    i = opening
    while i < len(source):
        char = source[i]
        following = source[i + 1] if i + 1 < len(source) else ""
        if state == "code":
            if char == "/" and following == "/":
                state = "line_comment"
                i += 1
            elif char == "/" and following == "*":
                state = "block_comment"
                i += 1
            elif char == '"':
                state = "string"
            elif char == "'":
                state = "character"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[start:i + 1]
        elif state == "line_comment":
            if char == "\n":
                state = "code"
        elif state == "block_comment":
            if char == "*" and following == "/":
                state = "code"
                i += 1
        elif state in ("string", "character"):
            if char == "\\":
                i += 1
            elif (state == "string" and char == '"') or (state == "character" and char == "'"):
                state = "code"
        i += 1
    raise AssertionError("unterminated installed helper definition")


@pytest.fixture(scope="module")
def host_binary(tmp_path_factory: pytest.TempPathFactory) -> Path:
    root = repo_root()
    owner = root / "src/vector_teardown.cpp"
    row_header = root / "src/netwin.h"
    function = extract_function(owner.read_text(encoding="utf-8"), SIGNATURE)
    assert function.count(CALL_SITE) == 1, "installed typed callback callsite changed"
    assert "original_method" not in function and "ORIGINAL(" not in function
    bridge = extract_function(row_header.read_text(encoding="utf-8"), BRIDGE_SIGNATURE)

    compiler = shutil.which("c++") or shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("no host C++ compiler is available")
    work = tmp_path_factory.mktemp("seed-402ee0-host")
    harness = work / "actual_source.cpp"
    binary = work / "actual_source"
    harness.write_text(
        r"""
#include <stdexcept>
typedef void (*func_vector_element_callback)(void *);
#define __stdcall
#define __fastcall
static void *seen[8];
static unsigned int seen_count;
static bool should_throw;
struct NetWinLobbyRow {
    unsigned char storage_[0x17C];
    void initialize_empty_slot() {
        seen[seen_count++] = this;
        if (should_throw) throw std::runtime_error("callback");
    }
""" + bridge + r"""
};
static_assert(sizeof(NetWinLobbyRow) == 0x17C, "host row stride");
""" + function + r"""
#include <cassert>
int main() {
    NetWinLobbyRow rows[3] = {};
    sub_402ee0(rows, sizeof(NetWinLobbyRow), 3,
               &NetWinLobbyRow::initialize_empty_slot_callback);
    assert(seen_count == 3);
    assert(seen[0] == rows);
    assert(seen[1] == rows + 1);
    assert(seen[2] == rows + 2);

    seen_count = 0;
    sub_402ee0(rows, sizeof(NetWinLobbyRow), 0,
               &NetWinLobbyRow::initialize_empty_slot_callback);
    sub_402ee0(rows, sizeof(NetWinLobbyRow), -1,
               &NetWinLobbyRow::initialize_empty_slot_callback);
    assert(seen_count == 0);

    should_throw = true;
    bool propagated = false;
    try {
        sub_402ee0(rows, sizeof(NetWinLobbyRow), 3,
                   &NetWinLobbyRow::initialize_empty_slot_callback);
    } catch (const std::runtime_error &) {
        propagated = true;
    }
    assert(propagated);
    assert(seen_count == 1); // no constructed-prefix cleanup is synthesized

    // DEC/JNS uses wrapped 32-bit subtraction: INT_MIN enters the loop,
    // unlike every other negative count. Throw on the first callback to
    // observe that branch without traversing an enormous synthetic array.
    seen_count = 0;
    propagated = false;
    try {
        sub_402ee0(rows, sizeof(NetWinLobbyRow), (-2147483647 - 1),
                   &NetWinLobbyRow::initialize_empty_slot_callback);
    } catch (const std::runtime_error &) {
        propagated = true;
    }
    assert(propagated && seen_count == 1);
}
""", encoding="utf-8")
    subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(harness), "-o", str(binary)], check=True)
    return binary


def test_installed_402ee0_body_uses_original_stride_and_count(host_binary: Path) -> None:
    subprocess.run([str(host_binary)], check=True)
