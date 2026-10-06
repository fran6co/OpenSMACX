"""Host control for the installed 0x496610 source body.

The function is extracted from src/quaylewin.cpp on each run. This test never imports or copies an ignored recovery proposal's body.
"""
from __future__ import annotations

from pathlib import Path
import shutil
import subprocess

import pytest


SIGNATURE = 'extern "C" void __cdecl sub_496610(int page)'


def repo_root() -> Path:
    for parent in Path(__file__).resolve().parents:
        if (parent / "src/quaylewin.cpp").is_file() and (parent / "CMakeLists.txt").is_file():
            return parent
    raise AssertionError("could not locate the OpenSMACX source tree")


def extract_function(source: str) -> str:
    assert source.count(SIGNATURE) == 1, "expected one installed direct-page function"
    start = source.index(SIGNATURE)
    opening = source.index("{", start + len(SIGNATURE))
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
    raise AssertionError("unterminated installed function definition")


@pytest.fixture(scope="module")
def host_binary(tmp_path_factory: pytest.TempPathFactory) -> Path:
    root = repo_root()
    function = extract_function((root / "src/quaylewin.cpp").read_text(encoding="utf-8"))
    for forbidden in ("0x00872CB0", "0x00874DF4", "0x00874DF8", "reinterpret_cast", "original_method"):
        assert forbidden not in function, f"fixed-address seam returned to owner body: {forbidden}"
    assert "QuayleWinGlobal.init()" in function
    assert "QuayleWinGlobal.set_page(page)" in function
    assert "QuayleWinGlobal.set_chapter(-1)" in function
    assert "QuayleWinGlobal.set_modal(0, 0, 0)" in function

    compiler = shutil.which("c++") or shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("no host C++ compiler is available")
    work = tmp_path_factory.mktemp("seed-496610-host")
    harness = work / "actual_source.cpp"
    binary = work / "actual_source"
    harness.write_text(
        r"""
#include <cassert>
#define __cdecl
static int IsMultiplayerNet;
static int events[16];
static int event_count;
struct QuayleWinMock {
    int init_result;
    int init_calls;
    int page;
    int chapter;
    int modal_flag;
    int modal_poll_was_null;
    int modal_owner_was_null;
    int modal_calls;
    int init() { ++init_calls; events[event_count++] = 1; return init_result; }
    void set_page(int value) { page = value; events[event_count++] = 2; }
    void set_chapter(int value) { chapter = value; events[event_count++] = 3; }
    int set_modal(int flag, int (*poll)(), void *owner) {
        modal_flag = flag;
        modal_poll_was_null = poll == 0;
        modal_owner_was_null = owner == 0;
        ++modal_calls;
        events[event_count++] = 4;
        return 17;
    }
};
static QuayleWinMock QuayleWinGlobal = {};
""" + function + r"""
int main() {
    IsMultiplayerNet = 1;
    sub_496610(7);
    assert(QuayleWinGlobal.init_calls == 0);
    assert(QuayleWinGlobal.modal_calls == 0);

    IsMultiplayerNet = 0;
    QuayleWinGlobal.init_result = 1;
    sub_496610(7);
    assert(QuayleWinGlobal.init_calls == 1);
    assert(QuayleWinGlobal.modal_calls == 0);
    assert(event_count == 1 && events[0] == 1);

    event_count = 0;
    QuayleWinGlobal.init_result = 0;
    sub_496610(25);
    assert(QuayleWinGlobal.page == 24);
    assert(QuayleWinGlobal.chapter == -1);
    assert(QuayleWinGlobal.modal_flag == 0);
    assert(QuayleWinGlobal.modal_poll_was_null);
    assert(QuayleWinGlobal.modal_owner_was_null);
    assert(QuayleWinGlobal.modal_calls == 1);
    assert(event_count == 4);
    assert(events[0] == 1 && events[1] == 2 && events[2] == 3 && events[3] == 4);

    event_count = 0;
    sub_496610(-2);
    assert(QuayleWinGlobal.page == -1);
    assert(QuayleWinGlobal.modal_calls == 2);
    assert(event_count == 4);
    assert(events[0] == 1 && events[1] == 2 && events[2] == 3 && events[3] == 4);
    sub_496610(-1);
    assert(QuayleWinGlobal.page == -1);
    assert(QuayleWinGlobal.modal_calls == 3);
}
""", encoding="utf-8")
    subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(harness), "-o", str(binary)], check=True)
    return binary


def test_installed_496610_preserves_page_and_modal_behavior(host_binary: Path) -> None:
    subprocess.run([str(host_binary)], check=True)

