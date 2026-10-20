"""Actual-source controls for Dialog's shared ID traversal."""

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[2]


def _body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError(f"unterminated body: {signature}")


HARNESS = r'''
#include <cassert>

#define MEASURED
struct StringStructEntry {
    int id;
    StringStructEntry *next;
};
struct ItemList {
    StringStructEntry *head_;
    StringStructEntry *current_;
    int entry_count_;
    int current_position_;
};
struct Dialog {
    ItemList item_list_;
    MEASURED int id_to_pos(int id);
};

__DIALOG_BODY__

int main() {
    StringStructEntry first = {11, 0};
    StringStructEntry second = {22, 0};
    StringStructEntry third = {33, 0};
    first.next = &second;
    second.next = &third;

    Dialog dialog;
    dialog.item_list_.head_ = &first;
    dialog.item_list_.current_ = 0;
    dialog.item_list_.entry_count_ = 3;
    dialog.item_list_.current_position_ = 8;
    assert(dialog.id_to_pos(22) == 1);
    assert(dialog.item_list_.current_position_ == 1);
    assert(dialog.item_list_.current_ == &second);

    // A miss walks the configured count, leaving current_ at the end and the
    // cursor at count; it does not stop at the last non-null node.
    assert(dialog.id_to_pos(99) == 3);
    assert(dialog.item_list_.current_position_ == 3);
    assert(dialog.item_list_.current_ == 0);

    // Null head preserves the prior cursor and current node.
    dialog.item_list_.head_ = 0;
    dialog.item_list_.current_ = &third;
    dialog.item_list_.current_position_ = 7;
    assert(dialog.id_to_pos(11) == 7);
    assert(dialog.item_list_.current_position_ == 7);
    assert(dialog.item_list_.current_ == &third);

    // Non-null head with a zero configured count resets the cursor and
    // installs head before the bounded loop runs zero iterations.
    dialog.item_list_.head_ = &first;
    dialog.item_list_.current_ = &third;
    dialog.item_list_.entry_count_ = 0;
    dialog.item_list_.current_position_ = 7;
    assert(dialog.id_to_pos(11) == 0);
    assert(dialog.item_list_.current_position_ == 0);
    assert(dialog.item_list_.current_ == &first);
    return 0;
}
'''


def _run(compiler: str, body: str, output: Path) -> subprocess.CompletedProcess[str]:
    generated = output.with_suffix(".cpp")
    generated.write_text(HARNESS.replace("__DIALOG_BODY__", body))
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", str(generated), "-o", str(output)],
        capture_output=True,
        text=True,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(output)], capture_output=True, text=True)


def test_id_to_pos_owner_traversal_and_miss_semantics(tmp_path: Path) -> None:
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")

    source = (ROOT / "src/dialog.cpp").read_text()
    header = (ROOT / "src/dialog.h").read_text()
    assert "MEASURED int id_to_pos(int id);" in header
    body = _body(source, "MEASURED int Dialog::id_to_pos(int id)")
    assert "entry_count_" in body and "entry->next" in body
    actual = _run(compiler, body, tmp_path / "dialog_id_to_pos")
    assert actual.returncode == 0, actual.stdout + actual.stderr

    mutant_body = body.replace(
        "traversed < count",
        "traversed < count - 1",
        1,
    )
    assert mutant_body != body
    mutant = _run(compiler, mutant_body, tmp_path / "dialog_id_to_pos_mutant")
    assert mutant.returncode != 0
