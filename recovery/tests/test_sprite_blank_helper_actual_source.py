"""Host behavior controls for the actual source-owned 0x63CDC0 body.

This test intentionally has no copied-body fallback: until the helper mapping
and definition are installed in src/sprite.cpp, it refuses to extract a body.
"""

from pathlib import Path
import shutil
import subprocess

import pytest
from match_recovery.source import Mode, read_text


ROOT = Path(__file__).resolve().parents[2]
PRODUCT = ROOT / "src/sprite.cpp"
SIGNATURE = 'extern "C" void __cdecl sub_63cdc0('

FIXTURE_PREFIX = r'''#include <stdint.h>
#include <string.h>
#include <stdio.h>
#ifndef __cdecl
#define __cdecl
#endif
struct Buffer { int tag; };
struct Win { unsigned char bytes[0x444]; };
struct GraphicWin : Win, Buffer { };
struct Sprite {
    int h;
    unsigned char key;
    int calls;
    Buffer *last_destination;
    int last_key, last_x, last_y;
    int result;
    int height() const { return h; }
    unsigned char transparent_index() const { return key; }
    int draw(Buffer *destination, int color, int x, int y) {
        ++calls; last_destination = destination; last_key = color;
        last_x = x; last_y = y;
        return result;
    }
};
Sprite &blank_sprite();
Sprite test_sprite = {8, 0xE5, 0, 0, 0, 0, 0, -17};
Sprite &blank_sprite() { return test_sprite; }
'''

FIXTURE_SUFFIX = r'''
static int fail(int n) {
    fprintf(stderr, "FAIL: sprite-helper control %d\n", n);
    return n;
}
int main() {
    unsigned char context[12] = {};
    GraphicWin window = {};
    test_sprite.calls = 0;
    sub_63cdc0(0, 0, 0, 3, 20, 5);
    if (test_sprite.calls != 0) return fail(1); // null opaque context

    uint32_t word = 0;
    memcpy(context + 8, &word, sizeof(word));
    sub_63cdc0(0, context, 0, 3, 20, 5);
    if (test_sprite.calls != 0) return fail(2); // zero full DWORD

    word = 0x80000000u;
    memcpy(context + 8, &word, sizeof(word));
    sub_63cdc0(0, context, -99, 3, 20, 5);
    if (test_sprite.calls != 1 || test_sprite.last_destination != 0 ||
        test_sprite.last_key != 0xE5 || test_sprite.last_x != 3 ||
        test_sprite.last_y != 19) return fail(3); // high-bit gate, null window, signed odd divide

    test_sprite.key = 0x7A;
    sub_63cdc0(&window, context, 12345, -6, -4, 11);
    if (test_sprite.calls != 2 ||
        test_sprite.last_destination != static_cast<Buffer *>(&window) ||
        test_sprite.last_key != 0x7A || test_sprite.last_x != -6 ||
        test_sprite.last_y != -3) return fail(4); // typed secondary base, live key, positive odd divide

    puts("sprite helper actual-source controls PASS");
    return 0;
}
'''


def extract_body(text: str) -> str:
    assert text.count(SIGNATURE) == 1, "actual src/sprite.cpp must map one sub_63cdc0 definition"
    starts = [row for row in read_text(text, PRODUCT.resolve()) if row.address == 0x0063CDC0]
    assert len(starts) == 1, "actual source reader must find one 0x63CDC0 owner"
    record = starts[0]
    assert record.mode is Mode.BODY and record.byte_exact
    start = text.index(SIGNATURE)
    opening = text.index("{", start)
    depth = 0
    state = "code"
    index = opening
    while index < len(text):
        char = text[index]
        following = text[index + 1] if index + 1 < len(text) else ""
        if state == "line_comment":
            if char == "\n":
                state = "code"
        elif state == "block_comment":
            if char == "*" and following == "/":
                state = "code"
                index += 1
        elif state == "string":
            if char == "\\":
                index += 1
            elif char == '"':
                state = "code"
        elif state == "char":
            if char == "\\":
                index += 1
            elif char == "'":
                state = "code"
        else:
            if char == "/" and following == "/":
                state = "line_comment"
                index += 1
            elif char == "/" and following == "*":
                state = "block_comment"
                index += 1
            elif char == '"':
                state = "string"
            elif char == "'":
                state = "char"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return text[start:index + 1]
        index += 1
    raise AssertionError("actual helper body has unbalanced braces")


def compile_run(compiler: str, function: str, tmp_path: Path, name: str):
    source = tmp_path / f"{name}.cpp"
    binary = tmp_path / name
    source.write_text(FIXTURE_PREFIX + function + FIXTURE_SUFFIX)
    result = subprocess.run(
        [compiler, "-std=c++17", "-O0", "-Wall", "-Wextra", str(source), "-o", str(binary)],
        text=True, capture_output=True, check=False,
    )
    assert result.returncode == 0, result.stdout + result.stderr
    return subprocess.run([str(binary)], text=True, capture_output=True, check=False)


def run_behavior_controls(source_text: str, tmp_path: Path, compilers=None):
    body = extract_body(source_text)
    if compilers is None:
        compilers = [shutil.which(name) for name in ("g++", "clang++")]
        compilers = [compiler for compiler in compilers if compiler]
    if not compilers:
        return False

    mutants = {
        "signed-gate": (
            "*reinterpret_cast<const uint32_t *>(\n          static_cast<const unsigned char *>(gate_context) + 8) != 0",
            "*reinterpret_cast<const int32_t *>(\n          static_cast<const unsigned char *>(gate_context) + 8) > 0",
        ),
        "untyped-window-offset": (
            "static_cast<Buffer *>(window)",
            "reinterpret_cast<Buffer *>(window)",
        ),
        "wrong-live-key": ("blank_sprite().transparent_index()", "blank_sprite().height()"),
        "negative-odd-rounding": ("region_height - blank_sprite().height()) / 2",
            "region_height - blank_sprite().height()) >> 1"),
    }
    for index, compiler in enumerate(compilers):
        baseline = compile_run(compiler, body, tmp_path, f"baseline-{index}")
        assert baseline.returncode == 0, baseline.stdout + baseline.stderr
        assert "sprite helper actual-source controls PASS" in baseline.stdout
        for name, (old, new) in mutants.items():
            assert body.count(old) == 1, f"mutant anchor must be unique: {name}"
            mutant = compile_run(compiler, body.replace(old, new, 1), tmp_path,
                                 f"{name}-{index}")
            assert mutant.returncode != 0 and "FAIL:" in mutant.stderr, (
                f"{compiler} control missed {name}: {mutant.stdout}{mutant.stderr}"
            )
    return True


def test_sprite_helper_actual_source_controls(tmp_path):
    source_text = PRODUCT.read_text(encoding="utf-8")
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    if not compilers:
        pytest.skip("host GCC/Clang required for actual-source behavior controls")
    assert run_behavior_controls(source_text, tmp_path, compilers)
