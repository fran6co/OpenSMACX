"""Actual-source controls for the recovered first cursor diagnostic only.

The neighboring 0x939FF0 text view remains an opaque dependency. These host
checks establish neither its contents/extent nor native ABI/runtime acceptance.
"""
import hashlib
import os
from pathlib import Path
import re
import subprocess

import pytest

from match_recovery.source import read_text
from match_recovery.tasks import TaskStore
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from test_console_cursor_storage import braced


ROOT = Path(__file__).resolve().parents[2]
BASE = "44ca59b96f129c158e60e2cde7dee69a0925e49c"
TASK = "console-cursor-diagnostic-storage-owner-oct13af"
IMAGE = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
COLLATERAL_CPP_FILES = (
    "src/base.cpp", "src/designwin.cpp", "src/faction.cpp", "src/game.cpp",
    "src/mapwin.cpp", "src/socialwin.cpp", "src/veh.cpp",
)
EXPANDED_SUBJECTS = {
    0x00436090, 0x00454270, 0x00455610, 0x004632D0,
    0x004B2740, 0x004CF7C0, 0x004F67F0, 0x0050C510,
    0x0051BC70, 0x00524870, 0x0054FFD0, 0x0057A660,
}


def _tokens(source):
    token = re.compile(
        r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|'
        r'//[^\n]*|/\*.*?\*/|[A-Za-z_]\w*|0[xX][0-9A-Fa-f]+|\d+|'
        r'::|->|\+\+|--|&&|\|\||==|!=|<=|>=|<<|>>|'
        r'\+=|-=|\*=|/=|%=|&=|\|=|\^=|.', re.S)
    return [value for match in token.finditer(source)
            if (value := match.group(0)) and not value.isspace()
            and not value.startswith("//") and not value.startswith("/*")]


def _git(path):
    return subprocess.check_output(["git", "show", f"{BASE}:{path}"],
                                   cwd=ROOT, text=True, timeout=30)


def _git_bytes(path):
    return subprocess.check_output(["git", "show", f"{BASE}:{path}"],
                                   cwd=ROOT, timeout=30)


def _array_definition(source):
    matches = re.findall(r'(?m)^char CursorNextDangerMsg1\[16\] = "[^"]*";', source)
    assert len(matches) == 1
    return matches[0]


def _array_declaration(source):
    matches = re.findall(r"(?m)^extern char CursorNextDangerMsg1\[16\];", source)
    assert len(matches) == 1
    return matches[0]


def _cursor_next(source):
    return braced(source, "void Console::cursor_next(int x_coord, int y_coord) {")


def _source_rows(path, source):
    return {row.address: row for row in read_text(source, ROOT / path)}


def _history(row):
    return tuple((item.kind, item.key, item.prose,
                  tuple(line for _, line in item.lines)) for item in row.origin)


def _restore_array_tokens(header, cpp):
    header = header.replace(_array_declaration(header),
        "LPCSTR const CursorNextDangerMsg1 = (LPCSTR)0x0068A5D0;")
    cpp = cpp.replace(_array_definition(cpp), "")
    return header, cpp


HARNESS = r'''#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#define __cdecl
#define LPCSTR const char *
extern char CursorNextDangerMsg1[16];
static const char *const opaque_second_argument =
    reinterpret_cast<const char *>(static_cast<uintptr_t>(0x939FF0));
const char *const CursorNextDangerMsg2 = opaque_second_argument;
struct GameDataStateStorage { struct { uint32_t game_state_; } fields_; };
GameDataStateStorage GameDataStateStorage;
const uint32_t STATE_DEBUG_MODE = 0x100;
struct Console { int32_t cursor_history_index_, cursor_history_head_;
                 int32_t cursor_history_x_[32], cursor_history_y_[32];
                 void cursor_next(int, int); };
Console ConsoleGlobal, receiver;
struct StatusWin { void set_loc(int, int); };
StatusWin ConsoleStatusWin;
static int danger_calls, status_calls;
static int expected_x, expected_y, normalized_x, normalized_y;
static void require(bool value, const char *reason) {
    if (!value) { std::fprintf(stderr, "cursor-diagnostic: %s\n", reason); std::exit(98); }
}
void danger(const char *first, const char *second, int x, int y, int final_arg) {
    ++danger_calls;
    require(first == CursorNextDangerMsg1, "danger-first-pointer-identity");
    require(std::strcmp(first, "Bad Cursor Move") == 0, "danger-first-text");
    require(second == opaque_second_argument, "danger-second-opaque-identity");
    require(x == expected_x && y == expected_y, "danger-original-coordinates");
    require(final_arg == 0, "danger-final-argument");
}
void StatusWin::set_loc(int x, int y) {
    ++status_calls;
    require(danger_calls == 1, "danger-before-status");
    require(x == normalized_x && y == normalized_y, "status-normalized-coordinates");
}
ACTUAL_DEFINITION
ACTUAL_CURSOR_NEXT
int main() {
    static_assert(sizeof(CursorNextDangerMsg1) == 16, "first text storage extent");
    require(CursorNextDangerMsg1[15] == '\0', "first-text-terminator");
    expected_x = 2; expected_y = 3;
    normalized_x = 3; normalized_y = 3;
    GameDataStateStorage.fields_.game_state_ = STATE_DEBUG_MODE;
    receiver.cursor_history_head_ = 0;
    ConsoleGlobal.cursor_history_index_ = 0;
    ConsoleGlobal.cursor_history_x_[0] = -100;
    ConsoleGlobal.cursor_history_y_[0] = -100;
    receiver.cursor_next(expected_x, expected_y);
    require(danger_calls == 1 && status_calls == 1, "one-danger-and-status-call");
    std::puts("actual cursor diagnostic source controls passed");
}
'''


def _compile_run(tmp_path, definition, body, *, expected):
    compiler = __import__("shutil").which("g++")
    if compiler is None:
        pytest.skip("host g++ required")
    source = tmp_path / "cursor_diagnostic.cpp"
    executable = tmp_path / "cursor_diagnostic"
    source.write_text(HARNESS.replace("ACTUAL_DEFINITION", definition)
                      .replace("ACTUAL_CURSOR_NEXT", body))
    compiled = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                               "-fsanitize=undefined", "-fno-sanitize-recover=all",
                               str(source), "-o", str(executable)], capture_output=True,
                              text=True, timeout=30)
    assert compiled.returncode == 0, compiled.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
    if expected is None:
        assert result.returncode == 0, result.stderr
        assert "actual cursor diagnostic source controls passed" in result.stdout
    else:
        assert result.returncode == 98, result.stdout + result.stderr
        assert f"cursor-diagnostic: {expected}" in result.stderr


def test_actual_console_cursor_diagnostic_source_and_immutable_base_tokens(tmp_path):
    task = TaskStore(ROOT).read(TASK)
    assert task.base_revision == BASE
    header = (ROOT / "src/console.h").read_text(encoding="utf-8")
    cpp = (ROOT / "src/console.cpp").read_text(encoding="utf-8")
    old_header = _git("src/console.h")
    old_cpp = _git("src/console.cpp")

    assert _array_declaration(header) == "extern char CursorNextDangerMsg1[16];"
    assert _array_definition(cpp) == 'char CursorNextDangerMsg1[16] = "Bad Cursor Move";'
    assert "CursorNextDangerMsg2 = (LPCSTR)0x00939FF0;" in header
    assert ("// RECOVERY_OBJECT: v2 0x0068A5D0 0x0068A5E0 "
            "?CursorNextDangerMsg1@@3PADA 0x005109B0") in cpp
    assert _tokens(_cursor_next(cpp)) == _tokens(_cursor_next(old_cpp))
    restored_header, restored_cpp = _restore_array_tokens(header, cpp)
    assert _tokens(restored_header) == _tokens(old_header)
    assert _tokens(restored_cpp) == _tokens(old_cpp)

    for path, before, after in (("src/console.h", old_header, header),
                                ("src/console.cpp", old_cpp, cpp)):
        old_rows = _source_rows(path, before)
        new_rows = _source_rows(path, after)
        assert set(new_rows) == set(old_rows), (path, "function identity set")
        for address, old in old_rows.items():
            new = new_rows[address]
            for field in old.__dataclass_fields__:
                if field not in {"line", "region", "origin"}:
                    assert getattr(new, field) == getattr(old, field), (path, address, field)
            assert _history(new) == _history(old), (path, address, "history")
    assert 0x005109B0 in _source_rows("src/console.cpp", cpp)

    definition = _array_definition(cpp)
    body = _cursor_next(cpp)
    _compile_run(tmp_path, definition, body, expected=None)


@pytest.mark.parametrize(("damage", "expected"), [
    ("wrong-first-text", "danger-first-text"),
    ("swapped-danger-coordinates", "danger-original-coordinates"),
])
def test_named_compiling_cursor_diagnostic_mutants_exit_98(tmp_path, damage, expected):
    cpp = (ROOT / "src/console.cpp").read_text(encoding="utf-8")
    definition = _array_definition(cpp)
    body = _cursor_next(cpp)
    if damage == "wrong-first-text":
        definition = definition.replace("Bad Cursor Move", "Bad Cursor Moxe", 1)
    else:
        before = "danger(CursorNextDangerMsg1, CursorNextDangerMsg2, x_coord,\n                  y_coord, 0);"
        after = "danger(CursorNextDangerMsg1, CursorNextDangerMsg2, y_coord,\n                  x_coord, 0);"
        assert body.count(before) == 1
        body = body.replace(before, after, 1)
    _compile_run(tmp_path, definition, body, expected=expected)


def test_pinned_first_diagnostic_bytes_match_the_bounded_source_literal():
    image_path = Path(os.environ.get(
        "OPENSMACX_IMAGE", ROOT.parents[2] / ".opensmacx/game/terranx_original.exe"))
    if not image_path.is_file():
        pytest.skip("pinned original unavailable; set OPENSMACX_IMAGE")
    raw = image_path.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == IMAGE
    image = PeImage(raw, IMAGE)
    assert image.read(Span(0x0068A5D0, 0x0068A5E0)) == b"Bad Cursor Move\0"


def test_expanded_header_collateral_cpp_units_and_subject_histories_are_preserved():
    store = TaskStore(ROOT)
    task = store.read(TASK)
    decisions = store.decisions(TASK)
    expansions = [decision for decision in decisions if decision["kind"] == "scope_expanded"]
    assert len(expansions) == 1
    expansion = expansions[0]
    expanded_cpp = {path for path in expansion["editable_files"] if path.endswith(".cpp")}
    assert expanded_cpp == set(COLLATERAL_CPP_FILES) | {"src/console.cpp"}
    assert EXPANDED_SUBJECTS == {subject["address"] for subject in expansion["subjects"]}
    assert all(subject["image_sha256"] == IMAGE and subject["address_space"] == "pe:va"
               for subject in expansion["subjects"])

    for path in COLLATERAL_CPP_FILES:
        assert (ROOT / path).read_bytes() == _git_bytes(path), (
            path, "header-induced collateral source must remain byte-exact")

    expected_subjects = EXPANDED_SUBJECTS | {subject.address for subject in task.subjects}
    assert expected_subjects == EXPANDED_SUBJECTS | {0x005109B0}
    source_paths = (*COLLATERAL_CPP_FILES, "src/console.cpp")
    old_rows, new_rows = {}, {}
    for path in source_paths:
        old_rows.update((row.address, (path, row))
                            for row in _source_rows(path, _git(path)).values())
        new_rows.update((row.address, (path, row))
                            for row in _source_rows(
                                path, (ROOT / path).read_text(encoding="utf-8")).values())
    assert expected_subjects <= set(old_rows) and expected_subjects <= set(new_rows)
    for address in expected_subjects:
        old_path, old = old_rows[address]
        new_path, new = new_rows[address]
        assert new_path == old_path, (address, "source identity owner")
        for field in old.__dataclass_fields__:
            if field not in {"line", "region", "origin"}:
                assert getattr(new, field) == getattr(old, field), (address, field)
        assert _history(new) == _history(old), (address, "history")
