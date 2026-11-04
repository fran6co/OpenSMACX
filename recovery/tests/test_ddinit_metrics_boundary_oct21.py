"""Focused DDInit metrics-helper source, behavior and original-call controls.

This test verifies the helper's current coordinate owner and a single
definition, not historical TU
ownership, object extent, native ABI, or byte-match improvement.
"""

from hashlib import sha256
import os
from pathlib import Path
import shutil
import subprocess
import re

import pytest

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
OWNER_CANDIDATES = ("src/win.cpp",)
SIGNATURE = 'extern "C" int __cdecl DDInitRefreshScreenMetrics()'


def _extract_function(text, signature):
    match = re.search(r"(?m)^\s*" + re.escape(signature) + r"\s*\{", text)
    assert match, "missing source function " + signature
    opening = text.find("{", match.start())
    depth = 0
    state = "code"
    index = opening
    while index < len(text):
        char = text[index]
        following = text[index + 1] if index + 1 < len(text) else ""
        if state == "code":
            if char == "/" and following == "/":
                state = "line"
                index += 1
            elif char == "/" and following == "*":
                state = "block"
                index += 1
            elif char == '"':
                state = "string"
            elif char == "'":
                state = "char"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return text[match.start():index + 1]
        elif state == "line" and char == "\n":
            state = "code"
        elif state == "block" and char == "*" and following == "/":
            state = "code"
            index += 1
        elif state in ("string", "char"):
            if char == "\\":
                index += 1
            elif (state == "string" and char == '"') or (state == "char" and char == "'"):
                state = "code"
        index += 1
    raise AssertionError("unterminated source function " + signature)


def _actual_helper():
    definitions = []
    for relative in OWNER_CANDIDATES:
        path = ROOT / relative
        if not path.exists():
            continue
        text = path.read_text(encoding="utf-8")
        if re.search(r"(?m)^\s*" + re.escape(SIGNATURE) + r"\s*\{", text):
            definitions.append((relative, _extract_function(text, SIGNATURE)))
    assert len(definitions) == 1, "expected exactly one helper definition across the two plausible source owners"
    return definitions[0]


def _actual_body(function):
    opening = function.index("{")
    return function[opening + 1:function.rfind("}")]


def _function_body(text, signature):
    return _extract_function(text, signature)


def test_helper_has_one_definition_and_current_clients_keep_delegation():
    owner, _ = _actual_helper()
    assert owner == "src/win.cpp"
    header = (ROOT / "src/win.h").read_text(encoding="utf-8")
    assert "Defined in win.cpp as part of the inferred Win contribution;" in header
    assert 'extern "C" int __cdecl DDInitRefreshScreenMetrics();' in header
    source = (ROOT / "src/win.cpp").read_text(encoding="utf-8")
    init = _function_body(source, "int DDInit::init(int width, int height, int depth, int tgl)")
    teardown = _function_body(source, "void DDInit::teardown()")
    destructor = _function_body(source, "DDInit::~DDInit()")
    assert init.count("DDInitRefreshScreenMetrics();") == 1
    assert teardown.count("DDInitRefreshScreenMetrics();") == 1
    assert destructor.count("teardown();") == 1
    assert len(re.findall(r"(?m)^DDInit WinDisplayInit;", source)) == 1


def _host_harness(helper):
    return r'''#include <cassert>
#include <cstring>
#define __cdecl
enum { SM_CXSCREEN = 0, SM_CYSCREEN = 1 };
static int WinScreenWidth, WinScreenHeight;
static int calls[2], call_count, cx_result, cy_result;
int GetSystemMetrics(int metric) {
    assert(call_count < 2);
    calls[call_count++] = metric;
    return metric == SM_CXSCREEN ? cx_result : cy_result;
}
@@HELPER@@
static void check_refresh(int cx, int cy) {
    WinScreenWidth = -1; WinScreenHeight = -2;
    cx_result = cx; cy_result = cy;
    call_count = 0; calls[0] = calls[1] = -1;
    const int result = DDInitRefreshScreenMetrics();
    assert(call_count == 2);
    assert(calls[0] == SM_CXSCREEN && calls[1] == SM_CYSCREEN);
    assert(WinScreenWidth == cx && WinScreenHeight == cy && result == cy);
}
int main() {
    check_refresh(1024, 768);
    check_refresh(960, 720);
    return 0;
}
'''.replace("@@HELPER@@", helper)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_helper_order_stores_return_and_mutants(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")

    _, function = _actual_helper()
    helper = function
    mutations = {
        "wrong_first_metric": helper.replace(
            "GetSystemMetrics(SM_CXSCREEN)", "GetSystemMetrics(SM_CYSCREEN)", 1),
        "wrong_width_destination": helper.replace(
            "WinScreenWidth = GetSystemMetrics(SM_CXSCREEN);",
            "WinScreenHeight = GetSystemMetrics(SM_CXSCREEN);", 1),
        "returns_width": helper.replace(
            "return (WinScreenHeight = GetSystemMetrics(SM_CYSCREEN));",
            "WinScreenHeight = GetSystemMetrics(SM_CYSCREEN); return WinScreenWidth;", 1),
    }
    assert all(mutated != helper for mutated in mutations.values())

    harness = _host_harness(helper)
    source_path = tmp_path / "actual.cpp"
    binary = tmp_path / "actual"
    source_path.write_text(harness, encoding="utf-8")
    build = subprocess.run([compiler, "-std=c++98", "-fsanitize=undefined",
                            "-fno-sanitize-recover=all", str(source_path), "-o", str(binary)],
                           capture_output=True, text=True, timeout=45)
    assert build.returncode == 0, build.stderr
    run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
    assert run.returncode == 0, run.stderr

    for name, mutated in mutations.items():
        assert mutated.count("DDInitRefreshScreenMetrics") == 1, name
        source_path.write_text(_host_harness(mutated), encoding="utf-8")
        build = subprocess.run([compiler, "-std=c++98", "-fsanitize=undefined",
                                "-fno-sanitize-recover=all", str(source_path), "-o", str(binary)],
                               capture_output=True, text=True, timeout=45)
        assert build.returncode == 0, name + build.stderr
        run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
        assert run.returncode != 0, name + " did not violate the helper control"


def _pinned_image():
    configured = os.environ.get("OPENSMACX_IMAGE")
    path = Path(configured) if configured else ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.is_file():
        pytest.skip("optional pinned original image is absent")
    raw = path.read_bytes()
    assert sha256(raw).hexdigest() == PIN, "wrong image supplied for pinned caller evidence"
    return PeImage(raw, PIN)


@pytest.mark.parametrize("call_site", [0x006354C3, 0x006355D0, 0x006357BA],
                         ids=["global_cleanup", "ddinit_init", "ddinit_teardown"])
def test_pinned_callers_target_metrics_helper(call_site):
    image = _pinned_image()
    encoded = image.read(Span(call_site, call_site + 5))
    assert encoded[0] == 0xE8
    target = call_site + 5 + int.from_bytes(encoded[1:], "little", signed=True)
    assert target == 0x005EFD00
