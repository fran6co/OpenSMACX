"""Actual borrowed Sprite receiver behavior for the sheet-icon helper.

Host controls execute the literal helper with narrow Sprite/Buffer doubles.
They check receiver and shared-buffer identity plus the original call sequence;
they do not prove the target ABI, receiver lifetime, allocation extent, or
pixel output.
"""

from pathlib import Path
from dataclasses import fields, replace
import shutil
import subprocess

import pytest

from match_recovery.includes import COMMENTS_OR_STRINGS
from match_recovery.source import read_text
from match_recovery.source.writer import write_bytes


ROOT = Path(__file__).resolve().parents[2]
BASE = "54f51003c1af9509f4c2abffc163c63b26df4e1e"
STEP_BASE = "f33ccd479ac454752a51a204c5f1fe40b5656268"
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
OLD_OFFSET_X = (
    "int off_x = (-static_cast<int>(((quadrant + 1) & 2) != 0) & 0x1d) + sheet_x;"
)
OLD_OFFSET_Y = "int off_y = (-static_cast<int>((quadrant & 2) != 0) & 0x1d) + sheet_y;"
NEW_OFFSET_X = "int off_x = sheet_x + (((quadrant + 1) & 2) ? 0x1d : 0);"
NEW_OFFSET_Y = "int off_y = sheet_y + ((quadrant & 2) ? 0x1d : 0);"
TARGET_MARKER = "// ORIGINAL: 0x00450FB0"
TARGET_CLAIM = "// ORIGINAL: 0x00450FB0 sub_450fb0 0x00450FB0-0x00451021 FILE BYTE_EXACT"


def _baseline_source() -> str:
    result = subprocess.run(
        ["git", "show", f"{BASE}:src/game.cpp"], cwd=ROOT,
        check=True, capture_output=True, text=True, timeout=30,
    )
    return result.stdout


def _step_source() -> str:
    result = subprocess.run(
        ["git", "show", f"{STEP_BASE}:src/game.cpp"], cwd=ROOT,
        check=True, capture_output=True, text=True, timeout=30,
    )
    return result.stdout


def _extract_function(source: str, signature: str = SIGNATURE) -> str:
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " " for char in match[0]),
        source,
    )
    start = masked.index(signature)
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


def _replace_once(source: str, old: str, new: str) -> str:
    assert source.count(old) == 1, f"expected one exact anchor: {old!r}"
    return source.replace(old, new, 1)


def _target_marker(source: str) -> str:
    matches = [line for line in source.splitlines() if line.startswith(TARGET_MARKER)]
    assert len(matches) == 1
    return matches[0]


def _with_current_target_marker(expected: str, current: str) -> str:
    return _replace_once(expected, _target_marker(expected), _target_marker(current))


def _assert_exact_source_transition(current: str, baseline: str) -> None:
    expected = _replace_once(baseline, OLD_SIGNATURE, NEW_SIGNATURE)
    expected = _replace_once(expected, OLD_RECEIVER, NEW_RECEIVER)
    expected = _replace_once(expected, OLD_OFFSET_X, NEW_OFFSET_X)
    expected = _replace_once(expected, OLD_OFFSET_Y, NEW_OFFSET_Y)
    expected = _with_current_target_marker(expected, current)
    assert _extract_function(current) == _extract_function(expected, SIGNATURE)
    assert _target_marker(current) == TARGET_CLAIM


def _assert_exact_step_transition(current: str, step_base: str) -> None:
    expected = _replace_once(step_base, OLD_OFFSET_X, NEW_OFFSET_X)
    expected = _replace_once(expected, OLD_OFFSET_Y, NEW_OFFSET_Y)
    rows = read_text(expected, SOURCE_PATH)
    target = [row for row in rows if row.address == 0x450FB0]
    assert len(target) == 1 and not target[0].byte_exact
    promoted = replace(target[0], byte_exact=True)
    expected_bytes = write_bytes(expected.encode("utf-8"), [promoted])
    assert current.encode("utf-8") == expected_bytes, (
        "game.cpp changed outside the two offsets and nativewriter target claim"
    )


def test_original_identity_and_history_are_preserved():
    before = _baseline_source()
    step_before = _step_source()
    current = SOURCE_PATH.read_text(encoding="utf-8")
    _assert_exact_source_transition(current, before)
    _assert_exact_step_transition(current, step_before)

    old_rows = read_text(before, SOURCE_PATH)
    step_rows = read_text(step_before, SOURCE_PATH)
    new_rows = read_text(current, SOURCE_PATH)
    old_icon = [row for row in old_rows if row.address == 0x450FB0]
    step_icon = [row for row in step_rows if row.address == 0x450FB0]
    new_icon = [row for row in new_rows if row.address == 0x450FB0]
    assert len(old_icon) == len(step_icon) == len(new_icon) == 1
    old_fields = tuple(
        (name, value) for name, value in _continuity(old_icon[0])
        if name in {"address", "image_spans", "symbol"}
    )
    new_fields = _continuity(new_icon[0])
    new_identity = tuple(
        (name, value) for name, value in new_fields
        if name in {"address", "image_spans", "symbol"}
    )
    assert old_fields == new_identity
    assert len(step_rows) == len(new_rows)
    for prior, recorded in zip(step_rows, new_rows):
        prior_fields = _continuity(prior)
        recorded_fields = _continuity(recorded)
        if prior.address == 0x450FB0:
            prior_fields = tuple(item for item in prior_fields if item[0] != "byte_exact")
            recorded_fields = tuple(item for item in recorded_fields if item[0] != "byte_exact")
        assert prior_fields == recorded_fields
    assert old_icon[0].image_spans == ((0x450FB0, 0x451021),)
    assert old_icon[0].symbol == new_icon[0].symbol == "_extract_sheet_icon"
    assert step_icon[0].byte_exact is False
    assert new_icon[0].byte_exact is True
    assert _target_marker(current) == TARGET_CLAIM
    assert _extract_function(current).count("sprite->extract(") == 1
    assert "reinterpret_cast<Sprite *>(sprite)" not in _extract_function(current)


@pytest.mark.parametrize("mutation", ("unrelated_source", "mapping", "claim", "history"))
def test_inverse_rejects_unrelated_source_or_evidence_loss(mutation: str) -> None:
    current = SOURCE_PATH.read_text(encoding="utf-8")
    before = _baseline_source()
    if mutation == "unrelated_source":
        damaged = _replace_once(current, "void __cdecl load_cursors()", "void __cdecl load_cursors_mutated()")
    elif mutation == "mapping":
        damaged = _replace_once(current, TARGET_MARKER, "// ORIGINAL: 0x00450FB1")
    elif mutation == "claim":
        damaged = _replace_once(current, TARGET_CLAIM, TARGET_CLAIM.removesuffix(" BYTE_EXACT"))
    else:
        damaged = _replace_once(
            current,
            "// LEVER: the fixed-address binding renamed to buffer.h's",
            "// NOTE: the fixed-address binding renamed to buffer.h's",
        )
    with pytest.raises((AssertionError, ValueError)):
        _assert_exact_source_transition(damaged, before)
        _assert_exact_step_transition(damaged, _step_source())


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
    elif mutation == "wrong_sign":
        old = "sheet_x + (((quadrant + 1) & 2) ? 0x1d : 0)"
        assert body.count(old) == 1
        body = body.replace(
            old, "sheet_x - (((quadrant + 1) & 2) ? 0x1d : 0)", 1
        )
    elif mutation == "wrong_constant":
        old = "sheet_x + (((quadrant + 1) & 2) ? 0x1d : 0)"
        assert body.count(old) == 1
        body = body.replace(
            old, "sheet_x + (((quadrant + 1) & 2) ? 0x1c : 0)", 1
        )
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
    ("wrong_sign", "extract-offsets-and-constants"),
    ("wrong_constant", "extract-offsets-and-constants"),
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
