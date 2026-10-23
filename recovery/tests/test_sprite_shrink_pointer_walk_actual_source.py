"""Actual-source behavioral controls for the paired Sprite shrink walks.

Host tests compare extracted complete C++ definitions against an independently
spelled index-walk oracle. They are not VC6 register-allocation, ABI, or game
runtime evidence.
"""

from __future__ import annotations

from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/sprite.cpp"
HEADER = ROOT / "src/sprite.h"
METHODS = (
    "get_clipped_x_table_shrink",
    "get_clipped_y_table_shrink",
)


HARNESS = r'''#include <assert.h>
#include <string.h>

struct RECT { int left, top, right, bottom; };
enum { SpriteScalingTableCount = 800 };
/* ACTUAL ARRAY DECLARATIONS */

struct Sprite {
/* ACTUAL METHOD DECLARATIONS */
};

int SpriteScalingSteps[SpriteScalingTableCount];
int SpriteClippedXSteps[SpriteScalingTableCount];
int SpriteClippedYSteps[SpriteScalingTableCount];

/* ACTUAL COMPLETE METHOD DEFINITIONS */

struct State {
    int scaling[SpriteScalingTableCount];
    int clipped_x[SpriteScalingTableCount];
    int clipped_y[SpriteScalingTableCount];
    int scalar[4];
};

static void seed(State &state) {
    for (int i = 0; i < SpriteScalingTableCount; ++i) {
        state.scaling[i] = 3 + ((i * 7) % 19);
        state.clipped_x[i] = 1001 + i;
        state.clipped_y[i] = 2001 + i;
    }
    for (int i = 0; i < 4; ++i) state.scalar[i] = 3001 + i;
}

static int *model_target(State &state, int region, int index) {
    if (region < 4) return &state.scalar[region];
    if (region == 4) return &state.scaling[index];
    if (region == 5) return &state.clipped_x[index];
    assert(region == 6);
    return &state.clipped_y[index];
}

static int *actual_target(int scalar[4], int region, int index) {
    if (region < 4) return &scalar[region];
    if (region == 4) return &SpriteScalingSteps[index];
    if (region == 5) return &SpriteClippedXSteps[index];
    assert(region == 6);
    return &SpriteClippedYSteps[index];
}

static int *index_model(bool vertical, const RECT &full, const RECT &clipped,
                        int *args[4], State &state) {
    const int skip = vertical ? clipped.top - full.top
                              : clipped.left - full.left;
    const int extent = vertical ? clipped.bottom - clipped.top
                                : clipped.right - clipped.left;
    if (extent == 0) return 0;
    *args[0] = skip;
    *args[2] = extent;
    *args[1] = 0;
    int index = 0;
    while (index < skip) {
        *args[1] += state.scaling[index];
        ++index;
    }
    *args[3] = 0;
    for (int i = 0; i < extent; ++i) {
        const int step = state.scaling[index + i];
        if (vertical) state.clipped_y[i] = step;
        else state.clipped_x[i] = step;
        *args[3] += step;
    }
    return vertical ? state.clipped_y : state.clipped_x;
}

static void run_case(bool vertical, int skip, int extent,
                     const int regions[4], const int indices[4]) {
    State expected;
    seed(expected);
    State actual = expected;
    int actual_args[4];
    for (int i = 0; i < 4; ++i) actual_args[i] = actual.scalar[i];
    int *model_args[4];
    int *call_args[4];
    for (int i = 0; i < 4; ++i) {
        model_args[i] = model_target(expected, regions[i], indices[i]);
        call_args[i] = actual_target(actual_args, regions[i], indices[i]);
    }

    RECT full = {0, 0, 0, 0};
    RECT clipped = {0, 0, 0, 0};
    if (vertical) {
        full.top = 10;
        clipped.top = full.top + skip;
        clipped.bottom = clipped.top + extent;
    } else {
        full.left = 10;
        clipped.left = full.left + skip;
        clipped.right = clipped.left + extent;
    }

    int *expected_result = index_model(vertical, full, clipped,
                                       model_args, expected);

    for (int i = 0; i < SpriteScalingTableCount; ++i) {
        SpriteScalingSteps[i] = actual.scaling[i];
        SpriteClippedXSteps[i] = actual.clipped_x[i];
        SpriteClippedYSteps[i] = actual.clipped_y[i];
    }
    Sprite sprite;
    int *actual_result = vertical
        ? sprite.get_clipped_y_table_shrink(&full, &clipped, call_args[0],
                                            call_args[1], call_args[2], call_args[3])
        : sprite.get_clipped_x_table_shrink(&full, &clipped, call_args[0],
                                            call_args[1], call_args[2], call_args[3]);
    int *expected_table = vertical ? SpriteClippedYSteps : SpriteClippedXSteps;
    assert(actual_result == (expected_result == 0 ? 0 : expected_table));
    for (int i = 0; i < 4; ++i) assert(actual_args[i] == expected.scalar[i]);
    for (int i = 0; i < SpriteScalingTableCount; ++i) {
        assert(SpriteScalingSteps[i] == expected.scaling[i]);
        assert(SpriteClippedXSteps[i] == expected.clipped_x[i]);
        assert(SpriteClippedYSteps[i] == expected.clipped_y[i]);
    }
}

int main() {
    // Ordinary positive horizontal and vertical walks, including a zero prefix.
    const int independent[4] = {0, 1, 2, 3};
    const int no_indices[4] = {0, 0, 0, 0};
    run_case(false, 3, 5, independent, no_indices);
    run_case(true, 0, 7, independent, no_indices);

    // Zero extent returns before all output-parameter or scratch-table writes.
    run_case(false, 2, 0, independent, no_indices);
    run_case(true, 4, 0, independent, no_indices);

    // Negative extent still performs the prefix work and scalar stores, but
    // must not form or advance either final-loop table pointer.
    run_case(false, 3, -2, independent, no_indices);
    run_case(true, 2, -3, independent, no_indices);

    // source_count aliases a future scaling entry: each iteration must observe
    // mutations caused by earlier loop iterations, not a cached source range.
    const int source_count_into_source[4] = {0, 1, 2, 4};
    const int future_source_index[4] = {0, 0, 0, 5};
    run_case(false, 2, 5, source_count_into_source, future_source_index);
    run_case(true, 2, 5, source_count_into_source, future_source_index);

    // source_count aliases the destination table; store-then-accumulate order
    // is observable for the matching cell.
    const int source_count_into_output[4] = {0, 1, 2, 5};
    const int first_output_cell[4] = {0, 0, 0, 0};
    run_case(false, 1, 4, source_count_into_output, first_output_cell);
    run_case(true, 1, 4, source_count_into_output, first_output_cell);

    // Prefix source_skip aliases the scaling table, exercising rereads after
    // the in-order prefix update; later output_count aliases a table as well.
    const int prefix_alias[4] = {0, 4, 2, 5};
    const int prefix_indices[4] = {0, 0, 0, 3};
    run_case(false, 3, 4, prefix_alias, prefix_indices);
    run_case(true, 3, 4, prefix_alias, prefix_indices);

    // Output parameters can alias each other. Assignment order and the later
    // source_count reset/increments remain explicit and independently checked.
    const int all_one_scalar[4] = {0, 0, 0, 0};
    run_case(false, 2, 3, all_one_scalar, no_indices);
    const int paired_scalar_alias[4] = {0, 1, 0, 1};
    run_case(true, 2, 3, paired_scalar_alias, no_indices);
}
'''


def _definition(source: str, method: str) -> str:
    signature = re.compile(r"int\s*\*\s*Sprite::" + re.escape(method) + r"\s*\(")
    found = list(signature.finditer(source))
    assert len(found) == 1, (method, len(found))
    start = found[0].start()
    opening = source.index("{", found[0].end())
    depth = 0
    for position in range(opening, len(source)):
        if source[position] == "{":
            depth += 1
        elif source[position] == "}":
            depth -= 1
            if depth == 0:
                return source[start:position + 1]
    raise AssertionError("unterminated definition: " + method)


def _source_fragments():
    source = SOURCE.read_text(encoding="utf-8")
    header = HEADER.read_text(encoding="utf-8")
    definitions = [_definition(source, method) for method in METHODS]
    declarations = []
    for method in METHODS:
        pattern = re.compile(r"  int \*" + re.escape(method) + r"\([^;]+;", re.S)
        found = pattern.findall(header)
        assert len(found) == 1, (method, len(found))
        declarations.append(found[0])
    array_declarations = []
    for name in ("SpriteScalingSteps", "SpriteClippedXSteps", "SpriteClippedYSteps"):
        pattern = re.compile(r"extern int " + name + r"\[SpriteScalingTableCount\];")
        found = pattern.findall(header)
        assert len(found) == 1, (name, len(found))
        array_declarations.append(found[0])
    return source, header, definitions, declarations, array_declarations


def _harness(definitions, declarations, array_declarations):
    methods = "\n".join(declarations)
    arrays = "\n".join(array_declarations)
    bodies = "\n\n".join(definitions)
    return (HARNESS.replace("/* ACTUAL ARRAY DECLARATIONS */", arrays)
            .replace("/* ACTUAL METHOD DECLARATIONS */", methods)
            .replace("/* ACTUAL COMPLETE METHOD DEFINITIONS */", bodies))


def _compilers():
    available = [path for name in ("g++", "clang++")
                 if (path := shutil.which(name))]
    if not available:
        pytest.skip("g++ and clang++ unavailable")
    return available


def _compile_run(tmp_path: Path, compiler: str, definitions, declarations,
                 arrays, *, should_pass: bool):
    cpp = tmp_path / "sprite_shrink_actual.cpp"
    binary = tmp_path / "sprite_shrink_actual"
    cpp.write_text(_harness(definitions, declarations, arrays), encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-O2", "-Wall", "-Wextra", "-Werror",
         str(cpp), "-o", str(binary)],
        capture_output=True, text=True, timeout=40,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=15)
    if should_pass:
        assert run.returncode == 0, run.stdout + run.stderr
    else:
        assert run.returncode != 0, "behaviorally damaged extracted source unexpectedly passed"


def test_actual_source_extraction_covers_both_complete_methods_and_typed_tables():
    source, header, definitions, declarations, arrays = _source_fragments()
    assert len(definitions) == len(declarations) == 2 and len(arrays) == 3
    for method, definition in zip(METHODS, definitions):
        assert "Sprite::" + method in definition
        assert "return SpriteClipped" in definition
    assert "if (extent == 0) return 0;" in source
    assert "enum { SpriteScalingTableCount = 800 };" in header
    assert ["SpriteScalingSteps", "SpriteClippedXSteps", "SpriteClippedYSteps"] == [
        re.search(r"extern int (\w+)", declaration).group(1)
        for declaration in arrays
    ]


@pytest.mark.parametrize("compiler", _compilers())
def test_extracted_actual_shrink_methods_match_index_oracle_with_aliases(
    compiler, tmp_path,
):
    _, _, definitions, declarations, arrays = _source_fragments()
    _compile_run(tmp_path, compiler, definitions, declarations, arrays, should_pass=True)


@pytest.mark.parametrize("compiler", _compilers())
@pytest.mark.parametrize("method_index", (0, 1))
def test_compiling_table_store_damage_fails_only_after_actual_witness(
    compiler, method_index, tmp_path,
):
    _, _, definitions, declarations, arrays = _source_fragments()
    _compile_run(tmp_path, compiler, definitions, declarations, arrays, should_pass=True)
    damaged = list(definitions)
    output_table = "SpriteClippedYSteps" if method_index else "SpriteClippedXSteps"
    old = output_table + "[i] = step;"
    assert damaged[method_index].count(old) == 1
    damaged[method_index] = damaged[method_index].replace(
        old, output_table + "[i] = step + 1;", 1)
    _compile_run(tmp_path, compiler, damaged, declarations, arrays, should_pass=False)
