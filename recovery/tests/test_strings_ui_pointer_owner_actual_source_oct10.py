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


def caption_harness(caption_body: str) -> str:
    return r'''#include <cstdio>
#include <cstring>
typedef char *LPSTR;
struct LabelBlock { LPSTR *strings_ptr; } Labels;
char StringTemp[128];
struct StringTableType {
    LPSTR input;
    LPSTR output;
    int calls;
    StringTableType() : input(0), output(0), calls(0) {}
    LPSTR get(LPSTR text) { input = text; ++calls; return output; }
} StringTable;
class Win {
public:
    char caption[128];
    void set_caption(LPSTR text) { std::strcpy(caption, text); }
};
class MapWin : public Win {
public:
    unsigned int field_1DD70_;
    void standard_caption();
};
''' + caption_body + r'''
int main() {
    char detail_label[] = "detail source";
    char world_label[] = "world source";
    char detail_text[] = "resolved detail";
    char world_text[] = "resolved world";
    LPSTR slots[0x102] = {};
    slots[0x100] = detail_label;
    slots[0x101] = world_label;
    Labels.strings_ptr = slots;
    MapWin window;

    window.field_1DD70_ = 0x40000000U;
    StringTable.output = detail_text;
    window.standard_caption();
    if (StringTable.calls != 1 || StringTable.input != detail_label ||
        std::strcmp(window.caption, detail_text) != 0) {
        std::fprintf(stderr, "FAIL: detail label flows through get into caption\n");
        return 1;
    }

    window.field_1DD70_ = 0;
    StringTable.output = world_text;
    window.standard_caption();
    if (StringTable.calls != 2 || StringTable.input != world_label ||
        std::strcmp(window.caption, world_text) != 0) {
        std::fprintf(stderr, "FAIL: world label flows through get into caption\n");
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


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_mapwin_caption_uses_label_pointer_and_get_result(
        compiler_name: str, tmp_path: Path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is not installed")
    source = (ROOT / "src/mapwin.cpp").read_text()
    body = extract_definition(source, "void MapWin::standard_caption()")
    assert "Labels.strings_ptr[0x100]" in body
    assert "Labels.strings_ptr[0x101]" in body
    assert "StringTable.get(text)" in body
    assert "reinterpret_cast<LPSTR *>" not in body
    ran = compile_and_run(compiler, caption_harness(body), tmp_path,
                          f"mapwin_caption_{compiler_name}")
    assert ran.returncode == 0, ran.stdout + ran.stderr


@pytest.mark.parametrize(("before", "after", "failure"), [
    ("Labels.strings_ptr[0x100]", "Labels.strings_ptr[0x101]",
     "detail label flows through get into caption"),
    ("StringTable.get(text)", "text",
     "detail label flows through get into caption"),
])
@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_compiling_mapwin_caption_mutants_are_detected(
        compiler_name: str, tmp_path: Path, before: str, after: str,
        failure: str):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is not installed")
    source = (ROOT / "src/mapwin.cpp").read_text()
    body = extract_definition(source, "void MapWin::standard_caption()")
    mutant = body.replace(before, after, 1)
    assert mutant != body
    ran = compile_and_run(compiler, caption_harness(mutant), tmp_path,
                          f"mapwin_caption_mutant_{compiler_name}")
    assert ran.returncode == 1
    assert failure in ran.stderr
