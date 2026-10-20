"""Exercise get_phrase's real source body and its phrase iteration bounds."""
from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/game.cpp"
SIGNATURE = "char *__cdecl get_phrase(int faction_id, int phrase_id)"


def extract_definition(source: str) -> str:
    assert source.count(SIGNATURE) == 1
    start = source.index(SIGNATURE)
    opening = source.index("{", start + len(SIGNATURE))
    depth = 0
    quote = None
    escaped = False
    line_comment = False
    block_comment = False
    for position in range(opening, len(source)):
        char = source[position]
        following = source[position + 1] if position + 1 < len(source) else ""
        if line_comment:
            if char == "\n":
                line_comment = False
            continue
        if block_comment:
            if char == "*" and following == "/":
                block_comment = False
            continue
        if quote:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == quote:
                quote = None
            continue
        if char in "\"'":
            quote = char
        elif char == "/" and following == "/":
            line_comment = True
        elif char == "/" and following == "*":
            block_comment = True
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return source[start : position + 1]
    raise AssertionError("get_phrase definition is not brace-balanced")


HARNESS = r"""
#include <cassert>
#include <cstring>
#include <string>
#include <vector>
#define __cdecl

static std::vector<std::string> events;

struct Player { char *search_key; char *filename; };
static Player Players[1];
static char *TextBufferGetPtr;
struct Strings {
    char *fallback;
    int gets;
    char *get(int index) { assert(index == 0); events.push_back("get"); ++gets; return fallback; }
} StringTable;

static int open_results[2];
static int open_count;
static char *open_src[2];
static char *open_section[2];
static int reads;
static int parsed[2];
static char *parsed_text[2];
static int pops;
static char *pop_caption;
static int pop_value;
static char returned_text[] = "last text buffer";
static char fallback_text[] = "fallback";

int text_open(const char *src, const char *section) {
    events.push_back("open");
    assert(open_count < 2);
    open_src[open_count] = const_cast<char *>(src);
    open_section[open_count] = const_cast<char *>(section);
    return open_results[open_count++];
}
char *text_get() {
    events.push_back("read");
    ++reads;
    assert(reads <= 32);
    TextBufferGetPtr = returned_text;
    return returned_text;
}
int parse_says(int id, const char *text, int gender, int plurality) {
    assert(id == 0 || id == 1);
    assert(gender == -1 && plurality == -1);
    events.push_back(id == 0 ? "parse0" : "parse1");
    parsed[id]++;
    parsed_text[id] = const_cast<char *>(text);
    return 0;
}
int X_pop(const char *caption, int value) {
    events.push_back("popup");
    ++pops;
    pop_caption = const_cast<char *>(caption);
    pop_value = value;
    return 0;
}

__FUNCTION__

static void reset(char *search, char *file, int first, int second) {
    events.clear();
    Players[0].search_key = search;
    Players[0].filename = file;
    open_results[0] = first;
    open_results[1] = second;
    open_count = reads = pops = 0;
    parsed[0] = parsed[1] = 0;
    parsed_text[0] = parsed_text[1] = 0;
    pop_caption = 0;
    pop_value = -1;
    StringTable.fallback = fallback_text;
    StringTable.gets = 0;
    TextBufferGetPtr = returned_text;
}

int main() {
    char search[] = "SEARCH";
    char file[] = "PLAYER";

    reset(search, file, 0, 0);
    assert(get_phrase(0, -9) == returned_text);
    assert(open_count == 1 && open_src[0] == file && open_section[0] == search);
    assert(reads == 0 && StringTable.gets == 0);

    reset(search, file, 0, 0);
    assert(get_phrase(0, -8) == returned_text);
    assert(reads == 1 && StringTable.gets == 0);

    reset(search, file, 0, 0);
    assert(get_phrase(0, -10) == returned_text);
    assert(reads == 0);

    reset(search, file, 0, 0);
    assert(get_phrase(0, 0) == returned_text);
    assert(reads == 9);

    reset(search, file, 0, 0);
    assert(get_phrase(0, 3) == returned_text);
    assert(reads == 12);

    reset(search, file, 1, 0);
    assert(get_phrase(0, -8) == returned_text);
    assert(open_count == 2 && open_src[1] == file && open_section[1] == file);
    assert(reads == 1 && StringTable.gets == 0);

    reset(search, file, 1, 1);
    assert(get_phrase(0, 4) == fallback_text);
    assert(open_count == 2 && reads == 0 && StringTable.gets == 1);
    assert(parsed[0] == 1 && parsed[1] == 1);
    assert(parsed_text[0] == search && parsed_text[1] == file);
    assert(pops == 1 && std::strcmp(pop_caption, "PLAYERFILE") == 0);
    assert(pop_value == 0);
    const char *expected[] = {"open", "open", "parse0", "parse1", "popup", "get"};
    assert(events.size() == sizeof(expected) / sizeof(expected[0]));
    for (unsigned int i = 0; i < events.size(); ++i) assert(events[i] == expected[i]);
    return 0;
}
"""


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_get_phrase_bounds_fallback_and_result(compiler: str, tmp_path: Path):
    compiler_path = shutil.which(compiler)
    if compiler_path is None:
        pytest.skip(f"{compiler} unavailable")
    definition = extract_definition(SOURCE.read_text(encoding="utf-8"))
    harness = HARNESS.replace("__FUNCTION__", definition)
    source_file = tmp_path / "get_phrase_actual.cpp"
    executable = tmp_path / "get_phrase_actual"
    source_file.write_text(harness, encoding="utf-8")
    built = subprocess.run(
        [compiler_path, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(source_file), "-o", str(executable)],
        capture_output=True, text=True, check=False,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    run = subprocess.run([str(executable)], capture_output=True, text=True, check=False)
    assert run.returncode == 0, run.stdout + run.stderr

    damaged = harness.replace("while (remaining > 0)", "while (remaining > 1)")
    assert damaged != harness, "damage anchor missing from actual get_phrase"
    source_file.write_text(damaged, encoding="utf-8")
    damaged_build = subprocess.run(
        [compiler_path, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(source_file), "-o", str(executable)],
        capture_output=True, text=True, check=False,
    )
    assert damaged_build.returncode == 0, damaged_build.stdout + damaged_build.stderr
    rejected = subprocess.run([str(executable)], capture_output=True, text=True, check=False)
    assert rejected.returncode != 0, "actual-source control missed under-iteration mutant"
