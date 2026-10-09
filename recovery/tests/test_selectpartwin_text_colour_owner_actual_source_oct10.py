"""Bounded actual-source controls for SelectPartWin's palette-color pairs."""

from __future__ import annotations

import re
import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]
SELECTPART = ROOT / "src/selectpartwin.cpp"
PALETTE_OWNER = ROOT / "src/interface_palette.cpp"

OWNERS = (
    ("draw_chassis", "int SelectPartWin::draw_chassis(int a1, RECT *a2, int a3)"),
    ("draw_reactor", "int SelectPartWin::draw_reactor(int a1, RECT *a2, int a3)"),
    ("draw_armor", "int SelectPartWin::draw_armor(int a1, RECT *a2, int a3)"),
    ("draw_weapon", "int SelectPartWin::draw_weapon(int a1, RECT *a2, int a3)"),
)

COLOR_CALL = re.compile(
    r"(?:\w+)->set_text_color\(\s*TextColourTable\[(\d+)\],\s*"
    r"TextColourTable\[(\d+)\],\s*1,\s*1\s*\);"
)


def extract_definition(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for offset in range(opening, len(text)):
        if text[offset] == "{":
            depth += 1
        elif text[offset] == "}":
            depth -= 1
            if depth == 0:
                return text[start : offset + 1]
    raise AssertionError(f"unterminated definition: {signature}")


def actual_owner_calls() -> dict[str, list[str]]:
    source = SELECTPART.read_text(encoding="utf-8")
    assert "PartLabelColorA1" not in source
    assert "PartLabelColorA2" not in source
    assert "PartLabelColorB1" not in source
    assert "PartLabelColorB2" not in source

    calls_by_owner: dict[str, list[str]] = {}
    for name, signature in OWNERS:
        body = extract_definition(source, signature)
        matches = list(COLOR_CALL.finditer(body))
        pairs = [(int(match.group(1)), int(match.group(2))) for match in matches]
        assert pairs == [(158, 159), (160, 161)], (name, pairs)
        calls_by_owner[name] = [match.group(0) for match in matches]
    return calls_by_owner


def table_definition() -> str:
    source = PALETTE_OWNER.read_text(encoding="utf-8")
    definitions = re.findall(r"^int TextColourTable\[\d+\];$", source, re.MULTILINE)
    assert len(definitions) == 1
    return definitions[0]


HARNESS = r'''
#include <cassert>

__TABLE_DEFINITION__

struct BufferSpy {
    int call_count;
    int actual[2][4];

    void reset() { call_count = 0; }

    void set_text_color(int first, int second, int flag1, int flag2) {
        assert(call_count < 2);
        actual[call_count][0] = first;
        actual[call_count][1] = second;
        actual[call_count][2] = flag1;
        actual[call_count][3] = flag2;
        ++call_count;
        // Model a consumer/callback changing the live table between the two
        // original calls. The later call must read the updated owner values.
        if (call_count == 1) {
            TextColourTable[158] = 501;
            TextColourTable[159] = -602;
            TextColourTable[160] = 703;
            TextColourTable[161] = -804;
        }
    }
};

BufferSpy spy;
BufferSpy *buf = &spy;
BufferSpy *buf444 = &spy;
BufferSpy *panel_buf = &spy;

void test_draw_chassis() {
    __CHASSIS_CALL_0__
    __CHASSIS_CALL_1__
}
void test_draw_reactor() {
    __REACTOR_CALL_0__
    __REACTOR_CALL_1__
}
void test_draw_armor() {
    __ARMOR_CALL_0__
    __ARMOR_CALL_1__
}
void test_draw_weapon() {
    __WEAPON_CALL_0__
    __WEAPON_CALL_1__
}

void check_pair(int call, int first, int second) {
    assert(spy.actual[call][0] == first);
    assert(spy.actual[call][1] == second);
    assert(spy.actual[call][2] == 1);
    assert(spy.actual[call][3] == 1);
}

void run_owner(void (*owner)()) {
    TextColourTable[158] = -101;
    TextColourTable[159] = 202;
    TextColourTable[160] = -303;
    TextColourTable[161] = 404;
    spy.reset();
    owner();
    assert(spy.call_count == 2);
    check_pair(0, -101, 202);
    check_pair(1, 703, -804);
}

int main() {
    run_owner(test_draw_chassis);
    run_owner(test_draw_reactor);
    run_owner(test_draw_armor);
    run_owner(test_draw_weapon);
    return 0;
}
'''


def compile_and_run(tmp_path: Path, calls_by_owner: dict[str, list[str]]):
    compiler = shutil.which("c++") or shutil.which("clang++") or shutil.which("g++")
    assert compiler, "a host C++ compiler is required for actual-source controls"
    substitutions = {"__TABLE_DEFINITION__": table_definition()}
    for owner, calls in calls_by_owner.items():
        assert len(calls) == 2
        marker = owner.removeprefix("draw_").upper()
        substitutions[f"__{marker}_CALL_0__"] = calls[0]
        substitutions[f"__{marker}_CALL_1__"] = calls[1]
    source = HARNESS
    for placeholder, statement in substitutions.items():
        source = source.replace(placeholder, statement)

    unit = tmp_path / "selectpartwin_text_colour_actual.cpp"
    binary = tmp_path / "selectpartwin_text_colour_actual"
    unit.write_text(source, encoding="utf-8")
    compiled = subprocess.run(
        [compiler, "-std=c++17", "-O0", str(unit), "-o", str(binary)],
        text=True,
        capture_output=True,
        check=False,
    )
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    return subprocess.run([str(binary)], text=True, capture_output=True, check=False)


def test_all_four_actual_owner_pairs_read_live_table_in_original_order(tmp_path: Path):
    result = compile_and_run(tmp_path, actual_owner_calls())
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize(
    ("label", "mutate"),
    [
        (
            "first pair reversed",
            lambda calls: {
                **calls,
                "draw_chassis": [
                    calls["draw_chassis"][0].replace(
                        "TextColourTable[158], TextColourTable[159]",
                        "TextColourTable[159], TextColourTable[158]",
                    ),
                    calls["draw_chassis"][1],
                ],
            },
        ),
        (
            "second pair uses cached values",
            lambda calls: {
                **calls,
                "draw_reactor": [
                    calls["draw_reactor"][0],
                    calls["draw_reactor"][1].replace(
                        "TextColourTable[160], TextColourTable[161]", "-303, 404"
                    ),
                ],
            },
        ),
        (
            "second pair uses wrong indices",
            lambda calls: {
                **calls,
                "draw_armor": [
                    calls["draw_armor"][0],
                    calls["draw_armor"][1].replace(
                        "TextColourTable[160], TextColourTable[161]",
                        "TextColourTable[158], TextColourTable[159]",
                    ),
                ],
            },
        ),
    ],
)
def test_compiling_source_mutants_are_rejected(tmp_path: Path, label, mutate):
    actual = actual_owner_calls()
    mutant = mutate(actual)
    assert mutant != actual, f"mutant did not change extracted source: {label}"
    result = compile_and_run(tmp_path, mutant)
    assert result.returncode != 0, f"behavioral control accepted mutant: {label}"
