"""Actual-source controls for EditBox::text_position's pinned width scan."""

from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery.local_variants import _bound_function_body_bytes, _marker_identity
from match_recovery.source.reader import read_text


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/editbox.cpp"
ADDRESS = 0x00616030


def _actual_body() -> str:
    text = SOURCE.read_text()
    markers = read_text(text, SOURCE)
    target = next(row for row in markers if row.address == ADDRESS)
    identities = tuple(_marker_identity(row) for row in markers)
    low, high = _bound_function_body_bytes(
        text, "EditBox::text_position", SOURCE, target, identities)
    return text.encode("utf-8")[low:high].decode("utf-8")


def _harness(body: str) -> str:
    return r"""#include <string.h>

struct Buffer {
    static int calls;
    static int results[16];
    static int seen_count[16];
    static char seen_text[16][256];
    static int text_width(char *text, int count) {
        if (calls >= 16) return -1000;
        seen_count[calls] = count;
        strcpy(seen_text[calls], text);
        return results[calls++];
    }
};
int Buffer::calls;
int Buffer::results[16];
int Buffer::seen_count[16];
char Buffer::seen_text[16][256];

struct EditBox {
    unsigned int field_B34_;
    unsigned int field_B18_;
    unsigned int field_B3C_;
    unsigned int field_B44_;
    char text_[256];
    int text_position(int x);
};

int EditBox::text_position(int x) """ + body + r"""

static void reset(EditBox &edit, const char *text, int scroll,
                  int left_bound, int cursor) {
    strcpy(edit.text_, text);
    edit.field_B18_ = scroll;
    edit.field_B34_ = left_bound;
    edit.field_B3C_ = 0;
    edit.field_B44_ = cursor;
    Buffer::calls = 0;
    for (int i = 0; i != 16; ++i) {
        Buffer::results[i] = 0;
        Buffer::seen_count[i] = -1;
        Buffer::seen_text[i][0] = 0;
    }
}

int main() {
    EditBox edit;
    reset(edit, "abcdef", 2, 3, 2);
    if (edit.text_position(2) != 2 || Buffer::calls != 0) return 1;

    reset(edit, "abc", 3, 0, 7); // Empty scrolled suffix returns zero.
    if (edit.text_position(10) != 0 || Buffer::calls != 0) return 2;

    reset(edit, "a", 0, 0, 2);
    if (edit.text_position(100) != 0 || Buffer::calls != 0) return 3;
    edit.field_B44_ = 1;
    if (edit.text_position(100) != 1 || Buffer::calls != 0) return 4;

    // Width equal to x continues; the next strictly larger width stops.
    reset(edit, "abcd", 1, 0, 3);
    Buffer::results[0] = 8;
    Buffer::results[1] = 9;
    if (edit.text_position(8) != 2 || Buffer::calls != 2) return 5;
    if (Buffer::seen_count[0] != 1 || Buffer::seen_count[1] != 2) return 6;
    if (strcmp(Buffer::seen_text[0], "bcd") || strcmp(Buffer::seen_text[1], "bcd")) return 7;

    // The bounded scan calls widths only for positions 1..length-1.
    reset(edit, "abcde", 0, 0, 1);
    if (edit.text_position(10) != 5 || Buffer::calls != 4) return 8;
    for (int i = 0; i != 4; ++i)
        if (Buffer::seen_count[i] != i + 1) return 9;

    // The final split is against the saved cursor, including equality.
    reset(edit, "abc", 0, 0, 2);
    Buffer::results[0] = 99;
    if (edit.text_position(1) != 0 || Buffer::calls != 1) return 10;
    edit.field_B44_ = 1;
    Buffer::results[1] = 99;
    if (edit.text_position(1) != 1 || Buffer::calls != 2) return 11;

    // The masked path measures the asterisk copy of the full text.
    reset(edit, "abc", 1, 0, 0);
    edit.field_B3C_ = 2;
    Buffer::results[0] = 99;
    if (edit.text_position(1) != 2 || Buffer::calls != 1) return 12;
    if (strcmp(Buffer::seen_text[0], "***")) return 13;
    return 0;
}
"""


def _compile_run(tmp_path: Path, compiler: str, body: str, stem: str) -> subprocess.CompletedProcess:
    source = tmp_path / f"{stem}.cpp"
    binary = tmp_path / stem
    source.write_text(_harness(body))
    compiled = subprocess.run([compiler, "-std=c++98", "-Wall", "-Werror",
                               str(source), "-o", str(binary)],
                              capture_output=True, text=True)
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_text_position_actual_body_and_compile_success_mutants(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} is not installed")
    body = _actual_body()
    result = _compile_run(tmp_path, compiler, body, "actual")
    assert result.returncode == 0, result.stderr

    # Each control is syntactically valid and must compile before its runtime
    # failure demonstrates that the actual-source assertions are discriminating.
    mutants = {
        "non_strict_width": ("Buffer::text_width(text, count) > x",
                             "Buffer::text_width(text, count) >= x"),
        "inclusive_length": ("while (count < length)", "while (count <= length)"),
        "wrong_final_index": ("return static_cast<int>(field_B18_) + count;",
                              "return static_cast<int>(field_B18_) + count + 1;"),
        "unmasked_path": ("*p = '*'", "*p = *p"),
    }
    for stem, (before, after) in mutants.items():
        assert body.count(before) == 1
        mutant = body.replace(before, after, 1)
        result = _compile_run(tmp_path, compiler, mutant, stem)
        assert result.returncode != 0, f"{compiler_name} did not detect {stem}"
