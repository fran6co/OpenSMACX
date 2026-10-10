"""Actual StringBox staging, inherited insertion and fixup controls."""
from pathlib import Path
import shutil
import subprocess

import pytest
from match_recovery.source import read_file
from test_game_logic_seed_batch_behavior import _definition

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/stringbox.cpp"

PREFIX = r'''
#include <cassert>
#include <cstdint>
char *expected_text;
uint32_t expected_flag;
int expected_index, insertion_result, calls, fixups, hooks;
struct StringStruct {
    virtual void on_insert() = 0;
    int add(int index) {
        assert(index == expected_index);
        ++calls;
        on_insert();
        return insertion_result;
    }
};
struct StringList : StringStruct {
    char *pending_text_;
    uint32_t field_20_, field_24_;
    void on_insert() {
        assert(pending_text_ == expected_text);
        assert(field_20_ == expected_flag && field_24_ == 0);
        ++hooks;
    }
};
struct StringBox {
    StringList stringList_;
    void add(char *, int, int);
    void add_fixup() {
        assert(calls == hooks && calls > 0);
        assert(stringList_.pending_text_ == expected_text);
        assert(stringList_.field_20_ == expected_flag);
        assert(stringList_.field_24_ == 0);
        ++fixups;
    }
};
'''
MAIN = r'''
int main() {
    StringBox box;
    char text[] = "TEXT";
    const int results[] = {0, 1, -1};
    const int flags[] = {0, 7, -1, (-2147483647-1)};
    int expected_fixups = 0;
    for (int r = 0; r != 3; ++r) {
        for (int f = 0; f != 4; ++f) {
            insertion_result = results[r];
            expected_text = f == 0 ? nullptr : text;
            expected_flag = static_cast<uint32_t>(flags[f]);
            expected_index = f == 0 ? -1 : f * 19;
            box.stringList_.pending_text_ = text + 1;
            box.stringList_.field_20_ = 123;
            box.stringList_.field_24_ = 456;
            box.add(expected_text, expected_index, flags[f]);
            if (insertion_result == 0) ++expected_fixups;
            assert(fixups == expected_fixups);
        }
    }
    assert(calls == 12 && hooks == 12 && fixups == 4);
}
'''


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None, "text", "flag", "zero", "index",
                                    "omit_fixup", "wrong_condition"))
def test_actual_typed_add(tmp_path, compiler_name, damage):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    record, = (r for r in read_file(SOURCE) if r.address == 0x00629710)
    assert record.image_spans == ((0x00629710, 0x00629749),)
    body = _definition(record.region, "void StringBox::add(")
    assert "reinterpret_cast" not in body and "memcpy" not in body
    changes = {
        "text": ("list.pending_text_ = text;", "list.pending_text_ = nullptr;"),
        "flag": ("static_cast<uint32_t>(flag)", "static_cast<uint32_t>(0)"),
        "zero": ("list.field_24_ = 0;", "list.field_24_ = 1;"),
        "index": ("list.add(index)", "list.add(index + 1)"),
        "omit_fixup": ("StringBox::add_fixup();", ""),
        "wrong_condition": ("list.add(index) == 0", "list.add(index) != 0"),
    }
    if damage is not None:
        old, new = changes[damage]
        assert body.count(old) == 1
        body = body.replace(old, new)
    source = tmp_path / "control.cpp"
    source.write_text(PREFIX + body + MAIN)
    executable = tmp_path / "control"
    subprocess.run([compiler, "-std=c++11", str(source), "-o", str(executable)],
                   check=True, capture_output=True, text=True)
    result = subprocess.run([str(executable)], capture_output=True)
    assert (result.returncode == 0) == (damage is None)
