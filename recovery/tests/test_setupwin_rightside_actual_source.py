"""Actual-source bounded host controls for the mapped SetupWin right-side body.

The definition and native source annotation are read from src/setupwin.cpp. It injects the extracted product body, never
the ignored candidate fragment, into a small host fixture.
"""
from pathlib import Path
import os
import shutil
import subprocess

import pytest
from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]
PRODUCT = ROOT / "src" / "setupwin.cpp"
HARNESS = Path(__file__).with_name("setupwin_rightside_harness.cpp")
SIGNATURE = "int SetupWin::do_menu_rightside(char *filename, int flags)"


def extract_body(source: str) -> str:
    start = source.find(SIGNATURE)
    assert start >= 0, "mapped product definition missing"
    opening = source.find("{", start)
    assert opening >= 0
    depth = 0
    state = "code"
    i = opening
    while i < len(source):
        ch = source[i]
        nxt = source[i + 1] if i + 1 < len(source) else ""
        if state == "code":
            if ch == "/" and nxt == "/":
                state = "line"
                i += 2
                continue
            if ch == "/" and nxt == "*":
                state = "block"
                i += 2
                continue
            if ch == '"':
                state = "string"
            elif ch == "'":
                state = "char"
            elif ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    return source[start:i + 1]
        elif state == "line":
            if ch == "\n":
                state = "code"
        elif state == "block":
            if ch == "*" and nxt == "/":
                state = "code"
                i += 2
                continue
        else:
            if ch == "\\":
                i += 2
                continue
            if (state == "string" and ch == '"') or (state == "char" and ch == "'"):
                state = "code"
        i += 1
    raise AssertionError("unterminated SetupWin::do_menu_rightside body")


def actual_body() -> str:
    records = [record for record in read_file(PRODUCT) if record.address == 0x004ADB70]
    assert len(records) == 1, "native owner identity must remain unique"
    assert records[0].state.value == "implemented"
    return extract_body(PRODUCT.read_text(encoding="utf-8"))


def run_host(tmp_path: Path, body: str):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if compiler is None:
        pytest.skip("a host C++ compiler is required for the extracted-body control")
    template = HARNESS.read_text(encoding="utf-8")
    marker = "// PRODUCT_BODY_INSERTION_POINT"
    assert template.count(marker) == 1
    source = tmp_path / "setupwin-rightside.cpp"
    binary = tmp_path / "setupwin-rightside"
    source.write_text(template.replace(marker, body), encoding="utf-8")
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
                    str(source), "-o", str(binary)], check=True,
                   capture_output=True, text=True, timeout=30)
    return subprocess.run([str(binary)], capture_output=True, text=True,
                          timeout=10,
                          env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))


def test_actual_setupwin_rightside_body(tmp_path):
    result = run_host(tmp_path, actual_body())
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("before,after,diagnostic", [
    ("flic.close();\n        return -1;", "return -1;",
     "early return explicitly closes Flic"),
    ("            text_close();", "            /* text-close mutant */",
     "script text closes before menu window initialization"),
    ("        opening_window_snapshot_->restore_clip();", "        /* restore mutant */",
     "parent clip restoration precedes Flic cleanup"),
    ("selected_row_ = static_cast<uint32_t>(menu_popup_->value());",
     "selected_row_ = 0;", "popup selection returns mapped dialog id"),
])
def test_order_and_lifetime_mutants_fail_with_specific_semantics(
        tmp_path, before, after, diagnostic):
    body = actual_body()
    assert before in body
    mutant = body.replace(before, after, 1)
    result = run_host(tmp_path, mutant)
    assert result.returncode == 97, (before, result.stdout, result.stderr)
    assert ("SEMANTIC97: " + diagnostic) in result.stderr
