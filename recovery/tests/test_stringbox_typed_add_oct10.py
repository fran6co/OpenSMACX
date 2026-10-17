"""Actual StringBox staging, inherited insertion and result/fixup controls."""
from hashlib import sha256
import os
from pathlib import Path
import shutil
import subprocess

import pytest
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source import read_file
from match_recovery.source.asm import Listing
from test_game_logic_seed_batch_behavior import _definition

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/stringbox.cpp"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
ADD = (0x00629710, 0x00629749)

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
    int add(char *, int, int);
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
    const int results[] = {0, 4, 1, -1, 7, (-2147483647-1), 2147483647};
    const int flags[] = {0, 7, -1, (-2147483647-1), 2147483647};
    int expected_fixups = 0;
    for (int r = 0; r != 7; ++r) {
        for (int f = 0; f != 5; ++f) {
            insertion_result = results[r];
            expected_text = f == 0 ? nullptr : text;
            expected_flag = static_cast<uint32_t>(flags[f]);
            expected_index = f == 0 ? -1 : f * 19;
            box.stringList_.pending_text_ = text + 1;
            box.stringList_.field_20_ = 123;
            box.stringList_.field_24_ = 456;
            const int result = box.add(expected_text, expected_index, flags[f]);
            assert(result == insertion_result);
            if (insertion_result == 0) ++expected_fixups;
            assert(fixups == expected_fixups);
        }
    }
    assert(calls == 35 && hooks == 35 && fixups == 5);
}
'''


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (
    None, "text", "flag", "zero", "index", "omit_fixup", "wrong_condition",
    "return_zero", "failure_normalization", "success_nonzero",
))
def test_actual_typed_add(tmp_path, compiler_name, damage):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    record, = (r for r in read_file(SOURCE) if r.address == ADD[0])
    assert record.image_spans == (ADD,)
    body = _definition(record.region, "int StringBox::add(")
    assert "reinterpret_cast" not in body and "memcpy" not in body
    changes = {
        "text": ("list.pending_text_ = text;", "list.pending_text_ = nullptr;"),
        "flag": ("static_cast<uint32_t>(flag)", "static_cast<uint32_t>(0)"),
        "zero": ("list.field_24_ = 0;", "list.field_24_ = 1;"),
        "index": ("list.add(index)", "list.add(index + 1)"),
        "omit_fixup": ("StringBox::add_fixup();", ""),
        "wrong_condition": ("result != 0", "result == 0"),
        "return_zero": ("return result;", "return 0;"),
        "failure_normalization": ("return result;", "return 1;"),
        "success_nonzero": ("return 0;", "return 1;"),
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


def test_pinned_add_result_contract_and_shared_return():
    image_path = Path(os.environ.get(
        "OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not image_path.is_file():
        pytest.skip("pinned original image is unavailable")
    raw = image_path.read_bytes()
    assert sha256(raw).hexdigest() == PIN
    image = PeImage(raw, PIN)

    assert ADD[1] - ADD[0] == 57
    code = image.read(Span(*ADD))
    assert len(code) == 57
    instructions = Listing(code, ADD[0]).instructions
    assert sum(instruction.size for instruction in instructions) == 57
    assert instructions[0].address == ADD[0]
    assert instructions[-1].address + instructions[-1].size == ADD[1]
    by_address = {instruction.address: instruction for instruction in instructions}

    assert (by_address[0x00629733].mnemonic,
            by_address[0x00629733].op_str) == ("call", "0x401100")
    assert (by_address[0x00629738].mnemonic,
            by_address[0x00629738].op_str) == ("test", "eax, eax")
    assert (by_address[0x0062973A].mnemonic,
            by_address[0x0062973A].op_str) == ("jne", "0x629745")
    assert (by_address[0x0062973E].mnemonic,
            by_address[0x0062973E].op_str) == ("call", "0x629490")
    assert (by_address[0x00629743].mnemonic,
            by_address[0x00629743].op_str) == ("xor", "eax, eax")
    assert [(instruction.mnemonic, instruction.op_str)
            for instruction in instructions[-2:]] == [
                ("pop", "esi"), ("ret", "0xc")]
