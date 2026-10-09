"""Compile actual UI string pointer helpers and catch a slot-index mutation."""
from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]


def extract_definition(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError(f"unterminated definition: {signature}")


def harness(label_helper: str, order_helper: str) -> str:
    return r'''#include <cstdio>
typedef char *LPSTR;
struct LabelBlock { LPSTR *strings_ptr; };
static LabelBlock Labels;
struct StringTableType {
    LPSTR get(LPSTR text) { return text; }
} StringTable;
''' + label_helper + "\n" + order_helper + r'''
int main() {
    char first[] = "first";
    char next[] = "next";
    LPSTR slots[0x3B2] = {};
    slots[0x36C / 4] = first;
    slots[0x36C / 4 + 1] = next;
    Labels.strings_ptr = slots;
    if (tutorial_button_label(0x36C) != first) {
        std::fprintf(stderr, "FAIL: byte offset selects pointer slot 0x36C/4\\n");
        return 1;
    }
    if (vehicle_order_rule_text(next) != next) {
        std::fprintf(stderr, "FAIL: RulesOrder pointer reaches Strings::get unchanged\\n");
        return 2;
    }
    return 0;
}
'''


def compile_and_run(compiler: str, source: str, tmp_path: Path, name: str):
    unit = tmp_path / f"{name}.cpp"
    executable = tmp_path / name
    unit.write_text(source)
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-pedantic",
         str(unit), "-o", str(executable)],
        capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True,
                          timeout=10)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_label_slot_and_rule_pointer_flow(compiler_name: str,
                                                 tmp_path: Path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is not installed")
    tutwin = (ROOT / "src/tutwin.cpp").read_text()
    statuswin = (ROOT / "src/statuswin.cpp").read_text()
    label_helper = extract_definition(
        tutwin, "static LPSTR tutorial_button_label(unsigned int byte_offset)")
    order_helper = extract_definition(
        statuswin, "static const char *vehicle_order_rule_text(LPSTR encoded_id)")
    assert "[byte_offset / 4]" in label_helper
    assert "StringTable.get(encoded_id)" in order_helper
    ran = compile_and_run(compiler, harness(label_helper, order_helper),
                          tmp_path, f"ui_pointer_flow_{compiler_name}")
    assert ran.returncode == 0, ran.stdout + ran.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_compiling_wrong_label_slot_mutant_is_detected(compiler_name: str,
                                                        tmp_path: Path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is not installed")
    tutwin = (ROOT / "src/tutwin.cpp").read_text()
    statuswin = (ROOT / "src/statuswin.cpp").read_text()
    label_helper = extract_definition(
        tutwin, "static LPSTR tutorial_button_label(unsigned int byte_offset)")
    order_helper = extract_definition(
        statuswin, "static const char *vehicle_order_rule_text(LPSTR encoded_id)")
    mutant = label_helper.replace("[byte_offset / 4]", "[byte_offset / 4 + 1]", 1)
    assert mutant != label_helper
    ran = compile_and_run(compiler, harness(mutant, order_helper), tmp_path,
                          f"ui_pointer_flow_mutant_{compiler_name}")
    assert ran.returncode == 1
    assert "FAIL: byte offset selects pointer slot 0x36C/4" in ran.stderr
