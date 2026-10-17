"""Actual-source controls for Win child-pointer identity and cursor cells.

The host executable uses the real extracted Win definitions with tiny field
doubles. It checks source behavior only, not VC6 ABI or application layout.
"""
from __future__ import annotations

from pathlib import Path
import shutil
import subprocess

import pytest
from match_recovery.binding_source import parse as parse_binding_evidence
from match_recovery.source import Mode, read_text


ROOT = Path(__file__).resolve().parents[2]
BASELINE = "423eb0250f4e59a19410add7c996af91582c9ed1"
IS_CHILD_ADDRESS = 0x005ECE80
SET_CURSOR_ADDRESS = 0x005EC780
IS_CHILD = "int Win::is_child(Win *child)"
SET_CURSOR = "int Win::set_cursor(HCURSOR *name)"


def _extract(text: str, signature: str) -> str:
    assert text.count(signature) == 1, f"expected one definition: {signature}"
    start = text.index(signature)
    opening = text.index("{", start + len(signature))
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError(f"unterminated definition: {signature}")


def _actual_bodies() -> tuple[str, str]:
    path = ROOT / "src/win.cpp"
    source = path.read_text(encoding="utf-8")
    rows = read_text(source, path)
    expected_modes = {IS_CHILD_ADDRESS: Mode.BODY, SET_CURSOR_ADDRESS: Mode.FILE}
    expected_spans = {
        IS_CHILD_ADDRESS: ((IS_CHILD_ADDRESS, 0x005ECEB6),),
        SET_CURSOR_ADDRESS: ((SET_CURSOR_ADDRESS, 0x005EC7BC),),
    }
    for address, signature in ((IS_CHILD_ADDRESS, IS_CHILD),
                               (SET_CURSOR_ADDRESS, SET_CURSOR)):
        row, = [row for row in rows if row.address == address]
        assert row.mode is expected_modes[address]
        assert row.image_spans == expected_spans[address]
    return _extract(source, IS_CHILD), _extract(source, SET_CURSOR)


PRELUDE = r'''
#include <cassert>
#define __cdecl
struct CursorObject {};
using HCURSOR = CursorObject *;
static int sprite_sentinel;
static HCURSOR cursor_cell_sentinel;
struct Win {
    Win *children_[150]{};
    int child_count_ = 0;
    int identity = 0;
    void *cursor_sprite_ = &sprite_sentinel;
    HCURSOR *cursor_handle_ = nullptr;
    int cursor_name_ = 99;
    static int update_calls;
    static int observed_null_window;
    static int observed_toggle;
    static void *observed_sprite;
    static HCURSOR *observed_handle;
    static int observed_name;
    static int update_cursor(Win *window, int toggle) {
        ++update_calls;
        observed_null_window = (window == nullptr);
        observed_toggle = toggle;
        observed_sprite = active->cursor_sprite_;
        observed_handle = active->cursor_handle_;
        observed_name = active->cursor_name_;
        return 0;
    }
    static Win *active;
    int is_child(Win *child);
    int set_cursor(HCURSOR *name);
};
int Win::update_calls = 0;
int Win::observed_null_window = 0;
int Win::observed_toggle = 0;
void *Win::observed_sprite = nullptr;
HCURSOR *Win::observed_handle = nullptr;
int Win::observed_name = -1;
Win *Win::active = nullptr;
'''

HARNESS = r'''
static void reset_update() {
    Win::update_calls = 0;
    Win::observed_null_window = 0;
    Win::observed_toggle = 0;
    Win::observed_sprite = nullptr;
    Win::observed_handle = nullptr;
    Win::observed_name = -1;
}
static void child_cases() {
    Win parent, first, same_contents, last;
    first.identity = same_contents.identity = last.identity = 41;
    parent.children_[0] = &first;
    parent.children_[1] = &last;
    parent.child_count_ = 0;
    assert(parent.is_child(&first) == 0);
    parent.child_count_ = -3;
    assert(parent.is_child(&first) == 0);
    parent.child_count_ = 2;
    assert(parent.is_child(&last) == 1); // final occupied entry is included
    assert(parent.is_child(&same_contents) == 0); // equal contents, distinct address
    assert(parent.is_child(nullptr) == 0);
    parent.children_[0] = nullptr;
    parent.child_count_ = 1;
    assert(parent.is_child(nullptr) == 0);
}
static void cursor_cases() {
    Win window;
    CursorObject object;
    HCURSOR live = &object;
    Win::active = &window;

    // Null cell pointer is permitted and publishes the null handle cell.
    reset_update();
    window.cursor_sprite_ = &sprite_sentinel;
    window.cursor_handle_ = &cursor_cell_sentinel;
    window.cursor_name_ = 5;
    assert(window.set_cursor(nullptr) == 0);
    assert(window.cursor_sprite_ == nullptr && window.cursor_handle_ == nullptr && window.cursor_name_ == 0);
    assert(Win::update_calls == 1 && Win::observed_null_window == 1 && Win::observed_toggle == 1);
    assert(Win::observed_sprite == nullptr && Win::observed_handle == nullptr && Win::observed_name == 0);

    // A nonnull pointer to a null HCURSOR cell returns 7 before any mutation.
    reset_update();
    HCURSOR empty = nullptr;
    window.cursor_sprite_ = &sprite_sentinel;
    window.cursor_handle_ = &cursor_cell_sentinel;
    window.cursor_name_ = 5;
    assert(window.set_cursor(&empty) == 7);
    assert(window.cursor_sprite_ == &sprite_sentinel);
    assert(window.cursor_handle_ == &cursor_cell_sentinel && window.cursor_name_ == 5);
    assert(Win::update_calls == 0);

    // A nonnull HCURSOR value publishes the original cell address, not a copy.
    reset_update();
    assert(window.set_cursor(&live) == 0);
    assert(window.cursor_sprite_ == nullptr && window.cursor_handle_ == &live && window.cursor_name_ == 0);
    assert(Win::update_calls == 1 && Win::observed_handle == &live);
    assert(Win::observed_sprite == nullptr && Win::observed_name == 0);
}
int main() { child_cases(); cursor_cases(); }
'''


def _compile_run(tmp_path: Path, compiler: str, bodies: tuple[str, str]) -> subprocess.CompletedProcess[str]:
    source = tmp_path / "actual_win_child_cursor.cpp"
    binary = tmp_path / "actual_win_child_cursor"
    source.write_text(PRELUDE + "\n".join(bodies) + HARNESS, encoding="utf-8")
    subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(source), "-o", str(binary)], check=True, timeout=30,
                   capture_output=True, text=True)
    return subprocess.run([str(binary)], timeout=10, capture_output=True, text=True)


def test_header_exposes_only_the_evidence_backed_current_types():
    header = (ROOT / "src/win.h").read_text(encoding="utf-8")
    assert "int is_child(Win *child);" in header
    assert "int set_cursor(HCURSOR *cursor);" in header
    assert "Win *children_[150];" in header
    assert "HCURSOR *cursor_handle_;" in header
    cpp = (ROOT / "src/win.cpp").read_text(encoding="utf-8")
    assert "reinterpret_cast<int *>(name)" not in _extract(cpp, SET_CURSOR)
    assert "reinterpret_cast<int>(children_[index])" not in _extract(cpp, IS_CHILD)


def test_actual_win_definitions_pass_host_controls_with_both_compilers(tmp_path):
    bodies = _actual_bodies()
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    if not compilers:
        pytest.skip("neither g++ nor clang++ is available for host controls")
    for index, compiler in enumerate(compilers):
        location = tmp_path / str(index)
        location.mkdir()
        result = _compile_run(location, compiler, bodies)
        assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("damage", (
    "compare_contents", "negative_count", "accept_null_needle",
    "reject_null_cell_pointer", "skip_null_cell_guard", "mutate_before_guard",
    "update_before_publication",
))
def test_host_controls_reject_damaged_pointer_or_cursor_semantics(tmp_path, damage):
    child, cursor = _actual_bodies()
    if damage == "compare_contents":
        child = child.replace("children_[index] == child",
                              "children_[index]->identity == child->identity")
    elif damage == "negative_count":
        child = child.replace("index < child_count_",
                              "index < (child_count_ < 0 ? 1 : child_count_)")
    elif damage == "accept_null_needle":
        child = child.replace("    if (child == nullptr) {\n        return 0;\n    }\n", "")
    elif damage == "reject_null_cell_pointer":
        cursor = cursor.replace("name != 0 && *name == 0", "name == 0 || *name == 0")
    elif damage == "skip_null_cell_guard":
        cursor = cursor.replace("name != 0 && *name == 0", "name != 0 && *name != 0")
    elif damage == "mutate_before_guard":
        cursor = cursor.replace("    if (name != 0 && *name == 0) {",
                                "    cursor_sprite_ = 0;\n    if (name != 0 && *name == 0) {")
    else:
        cursor = cursor.replace(
            "    cursor_sprite_ = 0;\n    cursor_handle_ = name;\n    cursor_name_ = 0;\n    Win::update_cursor(nullptr, 1);",
            "    Win::update_cursor(nullptr, 1);\n    cursor_sprite_ = 0;\n    cursor_handle_ = name;\n    cursor_name_ = 0;")
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("no host C++ compiler is available")
    tmp_path.mkdir(exist_ok=True)
    result = _compile_run(tmp_path, compiler, (child, cursor))
    assert result.returncode != 0, f"damage control survived: {damage}"


def _history(row):
    return tuple((lesson.kind, lesson.key, lesson.prose,
                  tuple(line for _, line in lesson.lines))
                 for lesson in (row.origin or ()))


def test_mapped_identity_claims_spans_and_history_are_continuous():
    path = ROOT / "src/win.cpp"
    current_text = path.read_text(encoding="utf-8")
    baseline_text = subprocess.run(
        ["git", "show", f"{BASELINE}:src/win.cpp"], cwd=ROOT,
        check=True, capture_output=True, text=True,
    ).stdout
    current = {row.address: row for row in read_text(current_text, path)}
    baseline = {row.address: row for row in read_text(baseline_text, path)}
    for address in (IS_CHILD_ADDRESS, SET_CURSOR_ADDRESS):
        before, after = baseline[address], current[address]
        assert (after.address, after.name, after.mode, after.image_spans,
                after.byte_exact, after.semantic, after.kind, after.exclusion,
                after.levers, after.ruled_out, after.unrecoverable, after.deferred,
                after.source_role, after.source_role_reason) == (
                before.address, before.name, before.mode, before.image_spans,
                before.byte_exact, before.semantic, before.kind, before.exclusion,
                before.levers, before.ruled_out, before.unrecoverable, before.deferred,
                before.source_role, before.source_role_reason)
        assert _history(after) == _history(before)


def test_observed_current_symbol_and_cursor_binding_are_parser_backed():
    path = ROOT / "src/win.cpp"
    text = path.read_text(encoding="utf-8")
    rows = read_text(text, path)
    current = {row.address: row for row in rows}
    rationale = parse_binding_evidence(text, rows)
    child = current[IS_CHILD_ADDRESS]
    cursor = current[SET_CURSOR_ADDRESS]

    # This is the coordinator's fresh native COFF observation, not an
    # authenticated original debug name. Assert the parsed source binding.
    assert child.symbol == "?is_child@Win@@QAEHPAV1@@Z"
    assert child.line in rationale
    assert "no authenticated original C++ prototype or name is established." in " ".join(
        rationale[child.line]["evidence"])

    # Cursor's existing emitted binding was not part of the native rename.
    baseline_text = subprocess.run(
        ["git", "show", f"{BASELINE}:src/win.cpp"], cwd=ROOT,
        check=True, capture_output=True, text=True,
    ).stdout
    baseline_rows = {row.address: row for row in read_text(baseline_text, path)}
    assert cursor.symbol == baseline_rows[SET_CURSOR_ADDRESS].symbol
    assert cursor.symbol == "?set_cursor@Win@@QAEHPAPAUHICON__@@@Z"
    assert cursor.line in rationale
