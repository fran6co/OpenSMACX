"""Actual borrowed Sprite receiver behavior for the sheet-icon helper.

Host controls execute the literal helper with narrow Sprite/Buffer doubles.
They check receiver and shared-buffer identity plus the original call sequence;
they do not prove the target ABI, receiver lifetime, allocation extent, or
pixel output.
"""

from pathlib import Path
from dataclasses import fields
import shutil
import subprocess

import pytest

from match_recovery.includes import COMMENTS_OR_STRINGS
from match_recovery.source import read_text


ROOT = Path(__file__).resolve().parents[2]
BASE = "54f51003c1af9509f4c2abffc163c63b26df4e1e"
SOURCE_PATH = ROOT / "src/game.cpp"
SIGNATURE = "int __cdecl extract_sheet_icon(Sprite *sprite,"
OLD_SIGNATURE = (
    'extern "C" int __cdecl extract_sheet_icon(int sprite, int sheet_x, int sheet_y,\n'
    "                                         int quadrant, int recolor) {"
)
NEW_SIGNATURE = (
    'extern "C" int __cdecl extract_sheet_icon(Sprite *sprite, int sheet_x, int sheet_y,\n'
    "                                         int quadrant, int recolor) {"
)
OLD_RECEIVER = "return reinterpret_cast<Sprite *>(sprite)->extract("
NEW_RECEIVER = "return sprite->extract("


def _baseline_source() -> str:
    result = subprocess.run(
        ["git", "show", f"{BASE}:src/game.cpp"], cwd=ROOT,
        check=True, capture_output=True, text=True, timeout=30,
    )
    return result.stdout


def _extract_function(source: str) -> str:
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " " for char in match[0]),
        source,
    )
    start = masked.index(SIGNATURE)
    opening = masked.index("{", start)
    depth = 0
    for end in range(opening, len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError("unterminated extract_sheet_icon definition")


def _continuity(row):
    return tuple(
        (field.name, getattr(row, field.name))
        for field in fields(row)
        if field.name not in {"path", "line", "region"}
    )


def _assert_exact_source_transition(current: str, baseline: str) -> None:
    assert baseline.count(OLD_SIGNATURE) == 1
    expected = baseline.replace(OLD_SIGNATURE, NEW_SIGNATURE, 1)
    assert expected.count(OLD_RECEIVER) == 1
    expected = expected.replace(OLD_RECEIVER, NEW_RECEIVER, 1)
    assert current == expected, "game.cpp changed outside the two authorized receiver edits"


def test_original_identity_and_history_are_preserved():
    before = _baseline_source()
    current = SOURCE_PATH.read_text(encoding="utf-8")
    _assert_exact_source_transition(current, before)

    old_rows = read_text(before, SOURCE_PATH)
    new_rows = read_text(current, SOURCE_PATH)
    assert len(old_rows) == len(new_rows)
    assert [_continuity(row) for row in old_rows] == [
        _continuity(row) for row in new_rows
    ]
    old_icon = [row for row in old_rows if row.address == 0x450FB0]
    new_icon = [row for row in new_rows if row.address == 0x450FB0]
    assert len(old_icon) == len(new_icon) == 1
    assert old_icon[0].image_spans == ((0x450FB0, 0x451021),)
    assert old_icon[0].symbol == new_icon[0].symbol == "_extract_sheet_icon"
    assert old_icon[0].byte_exact == new_icon[0].byte_exact
    assert _extract_function(current).count("sprite->extract(") == 1
    assert "reinterpret_cast<Sprite *>(sprite)" not in _extract_function(current)


HOST_PREFIX = r"""
#include <cstdio>
#include <cstdlib>
#define __cdecl

struct Event { int kind; int first; int second; };
static Event events[8];
static int event_count;
static int append_event(int kind, int first, int second) {
    if (event_count >= 8) return 98;
    events[event_count].kind = kind;
    events[event_count].first = first;
    events[event_count].second = second;
    ++event_count;
    return 0;
}

struct Buffer {
    void change_color(int from, int to) { append_event(1, from, to); }
};
static Buffer g_PCX_PARSE_TEMP_BUFFER1;
static Buffer wrong_buffer;

class Sprite {
public:
    int extract(Buffer *buffer, int transparent, int x, int y,
                int width, int height, int flags) {
        seen_receiver = this;
        seen_buffer = buffer;
        seen_transparent = transparent;
        seen_x = x;
        seen_y = y;
        seen_width = width;
        seen_height = height;
        seen_flags = flags;
        append_event(2, transparent, flags);
        return extract_result;
    }
    static Sprite *seen_receiver;
    static Buffer *seen_buffer;
    static int seen_transparent, seen_x, seen_y, seen_width, seen_height, seen_flags;
    static int extract_result;
};
Sprite *Sprite::seen_receiver = 0;
Buffer *Sprite::seen_buffer = 0;
int Sprite::seen_transparent = 0;
int Sprite::seen_x = 0;
int Sprite::seen_y = 0;
int Sprite::seen_width = 0;
int Sprite::seen_height = 0;
int Sprite::seen_flags = 0;
int Sprite::extract_result = 0;
static Sprite *wrong_sprite;

static int control_failure(const char *name) {
    std::fprintf(stderr, "CONTROL_FAIL:%s\n", name);
    return 98;
}
"""


def _harness(body: str, mutation: str | None = None) -> str:
    if mutation == "wrong_receiver":
        old = "return sprite->extract("
        assert body.count(old) == 1
        body = body.replace(old, "return wrong_sprite->extract(", 1)
    elif mutation == "wrong_buffer":
        old = "return sprite->extract(\n        &g_PCX_PARSE_TEMP_BUFFER1,"
        assert body.count(old) == 1
        body = body.replace(old, "return sprite->extract(\n        &wrong_buffer,", 1)
    elif mutation == "recolor_order":
        first = "g_PCX_PARSE_TEMP_BUFFER1.change_color(0x107, 0x109);"
        second = "g_PCX_PARSE_TEMP_BUFFER1.change_color(0x106, 0x109);"
        assert body.count(first) == body.count(second) == 1
        body = body.replace(first, "TEMP_COLOR_CALL;", 1)
        body = body.replace(second, first, 1)
        body = body.replace("TEMP_COLOR_CALL;", second, 1)
    elif mutation == "wrong_offset":
        old = "((quadrant + 1) & 2)"
        assert body.count(old) == 1
        body = body.replace(old, "(quadrant & 2)", 1)
    elif mutation == "wrong_result":
        old = "return sprite->extract("
        assert body.count(old) == 1
        body = body.replace(old, "sprite->extract(", 1)
        tail = "0x1c, 0x1c, 0);"
        assert body.count(tail) == 1
        body = body.replace(tail, tail + "\n    return 0;", 1)

    main = r"""
static int exercise(Sprite *receiver, int quadrant, int recolor,
                    int result_value, int sheet_x, int sheet_y) {
    event_count = 0;
    Sprite::seen_receiver = 0;
    Sprite::seen_buffer = 0;
    Sprite::extract_result = result_value;
    int returned = extract_sheet_icon(receiver, sheet_x, sheet_y, quadrant, recolor);
    if (Sprite::seen_receiver != receiver) return control_failure("extract-receiver-identity");
    if (Sprite::seen_buffer != &g_PCX_PARSE_TEMP_BUFFER1)
        return control_failure("extract-shared-buffer-identity");
    const int recolor_count = recolor != 0 ? 2 : 0;
    if (event_count != recolor_count + 1)
        return control_failure("event-count-and-call-order");
    if (recolor != 0 &&
        (events[0].kind != 1 || events[0].first != 0x107 || events[0].second != 0x109 ||
         events[1].kind != 1 || events[1].first != 0x106 || events[1].second != 0x109))
        return control_failure("recolor-event-order");
    if (events[recolor_count].kind != 2)
        return control_failure("extract-after-recolor");
    const int expected_x = sheet_x + ((((quadrant + 1) & 2) != 0) ? 0x1d : 0);
    const int expected_y = sheet_y + (((quadrant & 2) != 0) ? 0x1d : 0);
    if (Sprite::seen_transparent != 0x109 || Sprite::seen_x != expected_x ||
        Sprite::seen_y != expected_y || Sprite::seen_width != 0x1c ||
        Sprite::seen_height != 0x1c || Sprite::seen_flags != 0)
        return control_failure("extract-offsets-and-constants");
    if (returned != result_value) return control_failure("extract-return-value");
    return 0;
}

int main() {
    Sprite actual, decoy;
    wrong_sprite = &decoy;
    for (int quadrant = -8; quadrant <= 15; ++quadrant) {
        int result = exercise(&actual, quadrant, quadrant & 1,
                              quadrant * 17 - 9, 101 + quadrant, -47 + quadrant);
        if (result != 0) return result;
    }
    return 0;
}
"""
    function = body + "\n"
    return HOST_PREFIX + "\n" + function + "\n" + main


def _compile_run(source: str, compiler: str, tmp_path: Path, stem: str):
    cpp = tmp_path / f"{stem}.cpp"
    binary = tmp_path / stem
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-no-pie", str(cpp), "-o", str(binary)],
        capture_output=True, text=True, timeout=45,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_sheet_helper_receiver_behavior(compiler_name: str, tmp_path: Path) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    body = _extract_function(SOURCE_PATH.read_text(encoding="utf-8"))
    result = _compile_run(_harness(body), compiler, tmp_path, "sprite_sheet_receiver")
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize(("mutation", "diagnostic"), (
    ("wrong_receiver", "extract-receiver-identity"),
    ("wrong_buffer", "extract-shared-buffer-identity"),
    ("recolor_order", "recolor-event-order"),
    ("wrong_offset", "extract-offsets-and-constants"),
    ("wrong_result", "extract-return-value"),
))
def test_compile_success_damages_are_observable(
    mutation: str, diagnostic: str, compiler_name: str, tmp_path: Path,
) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    body = _extract_function(SOURCE_PATH.read_text(encoding="utf-8"))
    result = _compile_run(_harness(body, mutation), compiler, tmp_path,
                          f"sprite_sheet_{mutation}")
    assert result.returncode == 98, result.stdout + result.stderr
    assert f"CONTROL_FAIL:{diagnostic}" in result.stderr
