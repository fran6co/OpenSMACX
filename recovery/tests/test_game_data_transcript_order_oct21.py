"""Bounded actual-source controls for game_data's diplomacy transcript walk.

The native walk is verified only for these 21 list calls. This host control
does not initialize or inspect the serializer's intentionally indeterminate
write header and is not full serializer/runtime evidence.
"""
from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery.source import Mode, read_file


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/game.cpp"
MAPPED_ID = 0x005A6AC0
START = "    // 5A8A39/4A begins at [1][1]"
END = "    if (game_data_candidate_transfer(\n            file, &CouncilWindow.field_A1C_, 4) == 0)"


def actual_transcript_walk() -> str:
    source = SOURCE.read_text(encoding="utf-8")
    rows = [row for row in read_file(SOURCE) if row.address == MAPPED_ID]
    assert len(rows) == 1 and rows[0].mode is Mode.BODY
    assert rows[0].name == "?game_data@@YAHPAUFILE@@H@Z"
    assert "DEFERRED:" not in rows[0].region
    start = source.find(START)
    end = source.find(END, start)
    assert start >= 0 and end > start
    walk = source[start:end]
    assert "DiplomacyTranscriptLists[row][column]" in walk
    return walk


HARNESS = r'''
#include <assert.h>
#include <stdio.h>
#include <vector>
struct StringList { int id; };
StringList DiplomacyTranscriptLists[8][8];
static std::vector<int> seen;
static std::vector<int> seen_modes;
static int fail_at = -1;
static int list_call(FILE *, StringList &list, int mode) {
    int index = static_cast<int>(seen.size());
    seen.push_back(list.id);
    seen_modes.push_back(mode);
    return index == fail_at ? 1 : 0;
}
int walk(int write_mode) {
    FILE *file = 0;
    WALK_SOURCE
    return 0;
}
int seen_expected(int position);
int main() {
    for (int row = 0; row < 8; ++row)
        for (int column = 0; column < 8; ++column)
            DiplomacyTranscriptLists[row][column].id = row * 8 + column;
    for (int mode = 0; mode <= 1; ++mode) {
        seen.clear(); seen_modes.clear(); fail_at = -1;
        assert(walk(mode) == 0);
        int index = 0;
        for (int column = 2; column < 8; ++column)
            for (int row = 1; row < column; ++row) {
                assert(seen[index] == row * 8 + column);
                assert(seen_modes[index] == mode);
                ++index;
            }
        assert(index == 21 && seen.size() == 21);
        // These forbidden cells include row zero, every diagonal, the
        // initial [1][1] cell, and the [2][0] exclusive-stop cell.
        for (size_t i = 0; i < seen.size(); ++i) {
            assert(seen[i] != 1 * 8 + 1);
            assert(seen[i] != 2 * 8 + 0);
            assert(seen[i] / 8 != 0);
            assert(seen[i] / 8 != seen[i] % 8);
        }
        for (int failure = 0; failure < 21; ++failure) {
            seen.clear(); seen_modes.clear(); fail_at = failure;
            assert(walk(mode) == 1);
            assert(seen.size() == static_cast<size_t>(failure + 1));
            assert(seen.back() == seen_expected(failure));
        }
    }
    return 0;
}
int seen_expected(int position) {
    int index = 0;
    for (int column = 2; column < 8; ++column)
        for (int row = 1; row < column; ++row)
            if (index++ == position) return row * 8 + column;
    return -1;
}
'''


def harness_source(walk: str) -> str:
    # Keep the mock's function name aligned with the real helper call without
    # rewriting any control flow from the extracted production fragment.
    return HARNESS.replace("WALK_SOURCE", walk).replace(
        "game_data_candidate_string_list(", "list_call(")


def compile_and_run(compiler: str, tmp_path: Path, walk: str) -> subprocess.CompletedProcess:
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} is unavailable")
    cpp = tmp_path / "transcript_walk.cpp"
    binary = tmp_path / "transcript_walk"
    cpp.write_text(harness_source(walk), encoding="utf-8")
    built = subprocess.run(
        [executable, "-std=c++98", "-Wall", "-Wextra", str(cpp), "-o", str(binary)],
        capture_output=True, text=True, timeout=45)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=45)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_transcript_walk_order_modes_and_each_failure(compiler, tmp_path):
    ran = compile_and_run(compiler, tmp_path, actual_transcript_walk())
    assert ran.returncode == 0, ran.stdout + ran.stderr


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("mutation", ["old_stride", "inclusive", "transposed", "ignore_error"])
def test_compile_success_transcript_walk_mutants_are_rejected(
        compiler, mutation, tmp_path):
    walk = actual_transcript_walk()
    if mutation == "old_stride":
        changed = walk.replace(
            "    for (int column = 2; column < 8; ++column) {\n"
            "        for (int row = 1; row < column; ++row) {\n"
            "            if (game_data_candidate_string_list(\n"
            "                    file, DiplomacyTranscriptLists[row][column],\n"
            "                    write_mode) != 0)\n"
            "                return 1;\n"
            "        }\n"
            "    }\n",
            "    StringList *outer = &DiplomacyTranscriptLists[1][1];\n"
            "    StringList *const stop = &DiplomacyTranscriptLists[2][0];\n"
            "    int inner_count = 1;\n"
            "    while (outer < stop) {\n"
            "        StringList *const row_begin = outer;\n"
            "        StringList *inner = row_begin;\n"
            "        for (int pair = 0; pair < inner_count; ++pair) {\n"
            "            if (game_data_candidate_string_list(\n"
            "                    file, *inner, write_mode) != 0)\n"
            "                return 1;\n"
            "            inner += 2;\n"
            "        }\n"
            "        outer = row_begin + 1;\n"
            "        ++inner_count;\n"
            "    }\n", 1)
    elif mutation == "inclusive":
        changed = walk.replace("row < column", "row <= column", 1)
    elif mutation == "transposed":
        changed = walk.replace(
            "DiplomacyTranscriptLists[row][column]",
            "DiplomacyTranscriptLists[column][row]", 1)
    else:
        changed = walk.replace(
            "if (game_data_candidate_string_list(\n"
            "                    file, DiplomacyTranscriptLists[row][column],\n"
            "                    write_mode) != 0)\n"
            "                return 1;",
            "(void)game_data_candidate_string_list(\n"
            "                file, DiplomacyTranscriptLists[row][column],\n"
            "                write_mode);", 1)
    assert changed != walk, f"{mutation} mutant anchor must remain present"
    ran = compile_and_run(compiler, tmp_path, changed)
    assert ran.returncode != 0, f"{compiler} accepted behavioral mutant {mutation}"
