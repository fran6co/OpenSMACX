"""Host controls for stack_fix's entry-coordinate snapshots.

The fixture extracts the production function and stubs only its external
callbacks. The pinned body saves Vehs[veh_id].x/y before the loop, then
re-reads that same subject after callbacks and compares it to those saved
values. It deliberately does not compare the loop iterator's coordinates.
"""

from __future__ import annotations

import os
from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/veh.cpp"
SIGNATURE = "int __cdecl stack_fix(int veh_id)"


def _actual_definition() -> str:
    from match_recovery.source import read_file
    from match_recovery.source.reader import function_line
    from match_recovery.local_variants import (
        _bound_function_body_bytes,
        _marker_identity,
    )

    raw = SOURCE.read_bytes()
    text = raw.decode("utf-8")
    records = read_file(SOURCE)
    target, = [row for row in records if row.address == 0x005B8E10]
    markers = tuple(_marker_identity(row) for row in records)
    _, high = _bound_function_body_bytes(
        text, "stack_fix", SOURCE, target, markers
    )
    line = function_line(text, "stack_fix", after_line=target.line)
    start = len("".join(text.splitlines(keepends=True)[:line - 1]).encode("utf-8"))
    definition = raw[start:high].decode("utf-8")
    assert definition.startswith(SIGNATURE), definition[:100]
    return definition


HARNESS = r'''#include <assert.h>
#include <stdint.h>
#include <vector>

#ifndef __cdecl
#define __cdecl
#endif

struct Veh {
    int16_t x;
    int16_t y;
    int16_t next_veh_id_stack;
    int16_t prev_veh_id_stack;
};
Veh Vehs[8];
int IsMultiplayerNet;
struct GameDataFields { int vehicle_current_count_; };
struct GameData { GameDataFields fields_; };
GameData GameDataStateStorage;

enum CallbackMode {
    CALLBACKS_STABLE,
    MUTATE_SUBJECT_AFTER_FIRST_PROMOTE,
    GROW_COUNT_AFTER_FIRST_PROMOTE,
    SHRINK_COUNT_AFTER_FIRST_PROMOTE
};
static CallbackMode callback_mode;
static int subject_id;
static bool callback_applied;
static std::vector<int> events;

void __cdecl veh_promote(int id) {
    events.push_back(1000 + id);
    if (id != 0 || callback_applied) return;
    callback_applied = true;
    if (callback_mode == MUTATE_SUBJECT_AFTER_FIRST_PROMOTE) {
        ++Vehs[subject_id].x;
    } else if (callback_mode == GROW_COUNT_AFTER_FIRST_PROMOTE) {
        GameDataStateStorage.fields_.vehicle_current_count_ = 3;
    } else if (callback_mode == SHRINK_COUNT_AFTER_FIRST_PROMOTE) {
        GameDataStateStorage.fields_.vehicle_current_count_ = 1;
    }
}

void __cdecl stack_sort(int id) {
    events.push_back(2000 + id);
}

ACTUAL_STACK_FIX

static void expect_events(const int *expected, int count) {
    assert(static_cast<int>(events.size()) == count);
    for (int i = 0; i < count; ++i) assert(events[i] == expected[i]);
}

static void reset(int id, int count, CallbackMode mode) {
    for (int i = 0; i < 8; ++i) {
        Vehs[i].x = static_cast<int16_t>(i * 3);
        Vehs[i].y = static_cast<int16_t>(i * 5);
        Vehs[i].next_veh_id_stack = -1;
        Vehs[i].prev_veh_id_stack = -1;
    }
    subject_id = id;
    if (id >= 0) {
        Vehs[id].next_veh_id_stack = 2;
        Vehs[id].prev_veh_id_stack = 3;
    }
    Vehs[3].prev_veh_id_stack = 1;
    Vehs[1].prev_veh_id_stack = -1;
    IsMultiplayerNet = 1;
    GameDataStateStorage.fields_.vehicle_current_count_ = count;
    callback_mode = mode;
    callback_applied = false;
    events.clear();
}

int main() {
    // Early guards return the input and invoke neither callback.
    reset(4, 3, CALLBACKS_STABLE);
    assert(stack_fix(-1) == -1);
    expect_events(0, 0);
    IsMultiplayerNet = 0;
    assert(stack_fix(4) == 4);
    expect_events(0, 0);
    IsMultiplayerNet = 1;
    Vehs[4].next_veh_id_stack = -1;
    Vehs[4].prev_veh_id_stack = -1;
    assert(stack_fix(4) == 4);
    expect_events(0, 0);

    // Stable subject coordinates permit every live-count iteration. The
    // returned value is the subject's final stack top, reached after callbacks.
    reset(4, 3, CALLBACKS_STABLE);
    {
        const int expected[] = {1000, 2004, 1001, 2004, 1002, 2004};
        assert(stack_fix(4) == 1);
        expect_events(expected, 6);
    }

    // A callback changes the subject coordinates after iteration 0 has
    // entered its body. stack_sort still runs for that iteration; subsequent
    // iterator values must compare against the entry snapshots and be skipped.
    reset(4, 3, MUTATE_SUBJECT_AFTER_FIRST_PROMOTE);
    {
        const int expected[] = {1000, 2004};
        assert(stack_fix(4) == 1);
        expect_events(expected, 2);
    }

    // The loop reloads vehicle_current_count_ after callbacks: growth admits
    // new rows, while shrinkage prevents the next row from being visited.
    reset(4, 1, GROW_COUNT_AFTER_FIRST_PROMOTE);
    {
        const int expected[] = {1000, 2004, 1001, 2004, 1002, 2004};
        assert(stack_fix(4) == 1);
        expect_events(expected, 6);
    }
    reset(4, 3, SHRINK_COUNT_AFTER_FIRST_PROMOTE);
    {
        const int expected[] = {1000, 2004};
        assert(stack_fix(4) == 1);
        expect_events(expected, 2);
    }
    return 0;
}
'''


def _fixture(definition: str) -> str:
    marker = "ACTUAL_STACK_FIX"
    assert HARNESS.count(marker) == 1
    return HARNESS.replace(marker, definition, 1)


def _old_self_comparison_mutant(definition: str) -> str:
    predicate = re.compile(
        r"if\s*\(\s*Vehs\[veh_id\]\.x\s*==\s*[A-Za-z_]\w*"
        r"\s*&&\s*Vehs\[veh_id\]\.y\s*==\s*[A-Za-z_]\w*\s*\)"
    )
    mutant, replacements = predicate.subn(
        "if (Vehs[veh_id].x == Vehs[veh_id].x && "
        "Vehs[veh_id].y == Vehs[veh_id].y)",
        definition,
        count=1,
    )
    assert replacements == 1, "snapshot predicate damage anchor must match once"
    assert mutant != definition
    return mutant


def _compile_and_run(tmp_path: Path, compiler: str, definition: str,
                     name: str, *, damage: bool = False) -> subprocess.CompletedProcess[str]:
    cpp = tmp_path / f"{name}.cpp"
    exe = tmp_path / name
    cpp.write_text(_fixture(definition), encoding="utf-8")
    # The deliberately damaged predicate warns about tautology and leaves the
    # snapshots unused. Compile it so the behavior assertion, rather than a
    # warning promoted to an error, demonstrates the lost callback semantics.
    damage_flags = ["-Wno-error=tautological-compare", "-Wno-error=unused-variable"] if damage else []
    subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         *damage_flags, str(cpp), "-o", str(exe)],
        check=True,
        capture_output=True,
        text=True,
        timeout=60,
    )
    return subprocess.run([str(exe)], capture_output=True, text=True, timeout=30)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_stack_fix_entry_snapshots_and_live_callbacks(
    compiler_name: str, tmp_path: Path
) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    result = _compile_and_run(tmp_path, compiler, _actual_definition(), "actual")
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_old_self_comparison_damage_control_is_rejected(
    compiler_name: str, tmp_path: Path
) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    mutant = _old_self_comparison_mutant(_actual_definition())
    result = _compile_and_run(tmp_path, compiler, mutant, "self-comparison-mutant", damage=True)
    assert result.returncode != 0, "old always-true self-comparison mutant passed"
