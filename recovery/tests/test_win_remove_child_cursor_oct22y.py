"""Callback-aware host controls for the actual Win::remove_child body.

These fixtures check source-level list effects only. They do not establish the
VC6 object layout, 32-bit ABI, native instruction match, or whole-game behavior.
"""
from __future__ import annotations

from dataclasses import fields, replace
from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery.source import Mode, read_text


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/win.cpp"
ADDRESS = 0x005F5270
SYMBOL = "?remove_child@Win@@QAEXPAV1@@Z"
SIGNATURE = "void Win::remove_child(Win *child)"
BASE_REVISION = "7fb7f072926c248045b4f70eaaf1cf8e3fd456ce"


def actual_definition() -> str:
    text = SOURCE.read_text(encoding="utf-8")
    records = [row for row in read_text(text, SOURCE) if row.address == ADDRESS]
    assert len(records) == 1
    row, = records
    assert row.mode is Mode.FILE
    assert row.symbol == SYMBOL
    assert row.region is not None
    assert text.count(SIGNATURE) == 1
    start = text.index(SIGNATURE)
    opening = text.index("{", start + len(SIGNATURE))
    depth = 0
    for end in range(opening, len(text)):
        if text[end] == "{":
            depth += 1
        elif text[end] == "}":
            depth -= 1
            if depth == 0:
                definition = text[start:end + 1]
                assert definition in row.region
                return definition
    raise AssertionError("unterminated actual Win::remove_child definition")


FIXTURE = r'''
#include <assert.h>

struct Win {
    Win *children_[8];
    int child_count_;
    int hide_calls_;
    static Win *callback_owner_;
    static Win *callback_replacement_;
    static Win *callback_observed_slots_[8];
    static int callback_observed_count_;
    static int callback_mode_;

    Win() : child_count_(0), hide_calls_(0) {
        for (int i = 0; i != 8; ++i) children_[i] = 0;
    }
    virtual void hide() {
        ++hide_calls_;
        callback_observed_count_ = callback_owner_->child_count_;
        for (int i = 0; i != 8; ++i)
            callback_observed_slots_[i] = callback_owner_->children_[i];
        if (callback_mode_ == 1) {
            // Re-grow the live bound and replace entries during the virtual
            // callback. The original shift starts from the current list and
            // continues against the callback-updated count.
            callback_owner_->children_[1] = callback_replacement_;
            callback_owner_->children_[2] = callback_replacement_;
            callback_owner_->child_count_ = 4;
        } else if (callback_mode_ == 2) {
            // Shrinking below the found index suppresses the post-callback
            // shift; the callback's list/count writes remain observable.
            callback_owner_->children_[1] = callback_replacement_;
            callback_owner_->child_count_ = 1;
        } else if (callback_mode_ == 3) {
            callback_owner_->children_[2] = callback_replacement_;
        }
    }
    void remove_child(Win *child);
};

Win *Win::callback_owner_ = 0;
Win *Win::callback_replacement_ = 0;
Win *Win::callback_observed_slots_[8] = {0,0,0,0,0,0,0,0};
int Win::callback_observed_count_ = -1;
int Win::callback_mode_ = 0;

// ACTUAL_REMOVE_CHILD

static void reset_callback(Win *owner, Win *replacement, int mode) {
    Win::callback_owner_ = owner;
    Win::callback_replacement_ = replacement;
    Win::callback_mode_ = mode;
    Win::callback_observed_count_ = -1;
    for (int i = 0; i != 8; ++i) Win::callback_observed_slots_[i] = 0;
}

static void ordinary_cases() {
    Win parent, a, b, c, tail, absent;
    parent.children_[0] = &a;
    parent.children_[1] = &b;
    parent.children_[2] = &c;
    parent.children_[3] = &tail;
    parent.child_count_ = 4;
    reset_callback(&parent, 0, 0);
    parent.remove_child(0);
    assert(parent.child_count_ == 4 && parent.hide_calls_ == 0);
    assert(parent.children_[0] == &a && parent.children_[1] == &b);

    parent.child_count_ = 0;
    parent.remove_child(&a);
    assert(parent.child_count_ == 0 && a.hide_calls_ == 0);
    parent.child_count_ = -2;
    parent.remove_child(&a);
    assert(parent.child_count_ == -2 && a.hide_calls_ == 0);

    parent.child_count_ = 4;
    parent.remove_child(&absent);
    assert(parent.child_count_ == 4 && absent.hide_calls_ == 0);
    assert(parent.children_[0] == &a && parent.children_[1] == &b);

    reset_callback(&parent, 0, 0);
    parent.remove_child(&b);
    assert(parent.child_count_ == 3 && b.hide_calls_ == 1);
    assert(Win::callback_observed_count_ == 3);
    assert(Win::callback_observed_slots_[0] == &a);
    assert(Win::callback_observed_slots_[1] == &b);
    assert(parent.children_[0] == &a && parent.children_[1] == &c);
    assert(parent.children_[2] == &tail);

    reset_callback(&parent, 0, 0);
    parent.remove_child(&tail);
    assert(parent.child_count_ == 2 && tail.hide_calls_ == 1);
    assert(parent.children_[0] == &a && parent.children_[1] == &c);
}

static void callback_growth_and_live_shift() {
    Win parent, a, target, old_c, old_d, replacement, sentinel;
    parent.children_[0] = &a;
    parent.children_[1] = &target;
    parent.children_[2] = &old_c;
    parent.children_[3] = &old_d;
    parent.children_[4] = &sentinel;
    parent.child_count_ = 4;
    reset_callback(&parent, &replacement, 1);
    parent.remove_child(&target);
    assert(target.hide_calls_ == 1);
    assert(Win::callback_observed_count_ == 3); // decrement before hide
    assert(Win::callback_observed_slots_[1] == &target); // no early shift
    assert(parent.child_count_ == 4); // callback's growth is preserved
    assert(parent.children_[0] == &a);
    assert(parent.children_[1] == &replacement);
    assert(parent.children_[2] == &old_d);
    assert(parent.children_[3] == &sentinel);
}

static void callback_shrink_and_live_list() {
    Win parent, a, target, old_c, replacement;
    parent.children_[0] = &a;
    parent.children_[1] = &target;
    parent.children_[2] = &old_c;
    parent.child_count_ = 3;
    reset_callback(&parent, &replacement, 2);
    parent.remove_child(&target);
    assert(target.hide_calls_ == 1);
    assert(Win::callback_observed_count_ == 2);
    assert(parent.child_count_ == 1);
    assert(parent.children_[1] == &replacement); // no shift after shrink

    // With an unchanged count, shifting must reload the callback-mutated
    // successor rather than a stale/snapshotted array value.
    parent.children_[0] = &a;
    parent.children_[1] = &target;
    parent.children_[2] = &old_c;
    parent.child_count_ = 3;
    reset_callback(&parent, &replacement, 3);
    parent.remove_child(&target);
    assert(parent.child_count_ == 2);
    assert(parent.children_[1] == &replacement);
}

int main() {
    ordinary_cases();
    callback_growth_and_live_shift();
    callback_shrink_and_live_list();
}
'''


def harness(definition: str) -> str:
    marker = "// ACTUAL_REMOVE_CHILD"
    assert FIXTURE.count(marker) == 1
    return FIXTURE.replace(marker, definition, 1)


def run(compiler: str, tmp_path: Path, definition: str, label: str):
    source = tmp_path / (label + ".cpp")
    binary = tmp_path / label
    source.write_text(harness(definition), encoding="utf-8")
    subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                    str(source), "-o", str(binary)], check=True, timeout=30,
                   capture_output=True, text=True)
    return subprocess.run([str(binary)], timeout=10, capture_output=True, text=True)


def compilers():
    found = [compiler for compiler in (shutil.which("g++"), shutil.which("clang++"))
             if compiler]
    assert found, "actual-source controls require g++ or clang++"
    return found


@pytest.mark.parametrize("compiler", compilers())
def test_actual_remove_child_callback_and_list_controls(compiler, tmp_path):
    result = run(compiler, tmp_path, actual_definition(), "actual-remove-child")
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("compiler", compilers())
@pytest.mark.parametrize("before,after,label", (
    ("        child_count_ = count - 1;\n        child->hide();",
     "        child->hide();\n        child_count_ = count - 1;", "callback_before_decrement"),
    ("            } while (i < child_count_);",
     "            } while (i < count - 1);", "cached_post_callback_bound"),
    ("                *slot = slot[1];", "                *slot = *slot;", "skipped_shift"),
))
def test_compile_success_mutants_fail_callback_controls(
        compiler, tmp_path, before, after, label):
    definition = actual_definition()
    assert definition.count(before) == 1
    damaged = definition.replace(before, after, 1)
    result = run(compiler, tmp_path, damaged, label)
    assert result.returncode != 0, f"mutant escaped actual-source control: {label}"


def _metadata(row):
    return tuple((item.name, getattr(row, item.name)) for item in fields(row)
                 if item.name not in {"line", "region", "origin",
                                      "byte_exact", "semantic"})


def _history(row):
    return tuple((item.kind, item.key, item.prose,
                  tuple(line for _, line in item.lines)) for item in (row.origin or ()))


def test_mapped_identity_claims_and_history_remain_monotonic():
    current_text = SOURCE.read_text(encoding="utf-8")
    current = {row.address: row for row in read_text(current_text, SOURCE)}
    base_text = subprocess.check_output(
        ["git", "show", f"{BASE_REVISION}:src/win.cpp"], cwd=ROOT,
        text=True, timeout=30)
    base = {row.address: row for row in read_text(base_text, SOURCE)}
    # These original identities moved to genuine owners after BASE_REVISION.
    # Verify their current mappings and history rather than dropping them from
    # the baseline continuity check or requiring obsolete Win duplicates.
    relocated_owners = {
        0x005ED240: "src/win_point_coordinates.cpp",
        0x005ED2D0: "src/win_point_coordinates.cpp",
        0x005ED3F0: "src/win_point_coordinates.cpp",
        0x005FA7A0: "src/win_coordinates.cpp",
        0x005FA7E0: "src/win_coordinates.cpp",
        0x005FD120: "src/win_input.cpp",
    }
    assert set(base) - set(current) == set(relocated_owners)
    assert set(current) <= set(base)
    owners = {
        name: read_text((ROOT / name).read_text(encoding="utf-8"), ROOT / name)
        for name in set(relocated_owners.values())
    }
    # A prior ABI recovery corrected this empty virtual callback to void;
    # retain its original catalogue identity while checking the exact emitted
    # symbol transition, rather than freezing the old return-type hypothesis.
    corrected_symbols = {
        0x005F6A40: ("?on_sys_command@Win@@UAEHIHH@Z",
                     "?on_sys_command@Win@@UAEXIHH@Z"),
    }
    assert "void Win::on_sys_command(unsigned int, int, int)" in current_text
    for address, old in base.items():
        expected = old
        if address in corrected_symbols:
            before, after = corrected_symbols[address]
            assert old.symbol == before
            expected = replace(old, symbol=after)
        if address in relocated_owners:
            owner = ROOT / relocated_owners[address]
            candidates = [row for row in owners[relocated_owners[address]]
                          if row.address == address]
            assert len(candidates) == 1
            now, = candidates
            assert now.path == owner
            # The proven move changes the host path, preserving all other
            # original mapping metadata, claims and recorded source lessons.
            assert _metadata(replace(now, path=old.path)) == _metadata(expected)
        else:
            now = current[address]
            assert _metadata(now) == _metadata(expected)
        assert _history(now) == _history(old)
        if old.byte_exact:
            assert now.byte_exact
        if old.semantic:
            assert now.semantic
    row = current[ADDRESS]
    assert row.symbol == SYMBOL and row.mode is Mode.FILE
    # The candidate changes only executable source within the already mapped
    # body; mappings, claims, and recorded lessons remain intact.
    assert "// ORIGINAL: 0x005F5270" in row.region
    assert row.byte_exact is False and row.semantic is False
    assert row.image_spans == ((ADDRESS, 0x005F52D1),)
