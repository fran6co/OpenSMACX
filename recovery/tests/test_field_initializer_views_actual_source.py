"""Bounded field controls, not original class or whole-object layout proof."""
from pathlib import Path
import shutil

import pytest
from test_leaf_neutral_views_actual_source import (
    PREAMBLE, compile_run, extract, extract_struct,
)

ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/IMAGE_PIN").is_file())
SOURCE = ROOT / "src/field_accessors.cpp"

MAIN = r'''int main() {
  assert(sizeof(Fields4C8090View) == 0x24);
  assert(offsetof(Fields4C8090View, dword_00_) == 0);
  assert(offsetof(Fields4C8090View, dword_04_) == 4);
  assert(offsetof(Fields4C8090View, dword_08_) == 8);
  assert(offsetof(Fields4C8090View, dword_0C_) == 12);
  assert(offsetof(Fields4C8090View, dword_10_) == 16);
  assert(offsetof(Fields4C8090View, dword_14_) == 20);
  assert(offsetof(Fields4C8090View, dword_18_) == 24);
  assert(offsetof(Fields4C8090View, dword_1C_) == 28);
  assert(offsetof(Fields4C8090View, dword_20_) == 32);
  assert(sizeof(Fields4C8100View) == 12);
  assert(offsetof(Fields4C8100View, dword_04_) == 4);
  assert(offsetof(Fields4C8100View, dword_08_) == 8);
  struct Guarded9 { uint32_t before; Fields4C8090View view; uint32_t after; };
  Guarded9 a = {0xAABBCCDDu, {1,2,3,4,5,6,7,8,9}, 0x11223344u};
  assert(field_accessor_004c8090_redirect(&a.view, 0) == &a.view);
  assert(a.view.dword_00_ == 0 && a.view.dword_04_ == 0);
  assert(a.view.dword_08_ == 0 && a.view.dword_0C_ == 0);
  assert(a.view.dword_10_ == 0 && a.view.dword_14_ == 0);
  assert(a.view.dword_18_ == 0 && a.view.dword_1C_ == 0);
  assert(a.view.dword_20_ == 0xFFFFFFFFu);
  assert(a.before == 0xAABBCCDDu && a.after == 0x11223344u);
  struct Guarded2 { uint32_t before; Fields4C8100View view; uint32_t after; };
  Guarded2 b = {0x10203040u, {0xFFFFFFFFu, 7, 8}, 0x50607080u};
  assert(field_accessor_004c8100_redirect(&b.view, 0) == &b.view);
  assert(b.view.untouched_00_ == 0xFFFFFFFFu);
  assert(b.view.dword_04_ == 0 && b.view.dword_08_ == 0);
  assert(b.before == 0x10203040u && b.after == 0x50607080u);
  return 0;
}
'''


def test_current_views_keep_observed_store_order_and_scaffold_status():
    from match_recovery.source import read_file
    source = SOURCE.read_text()
    rows = {row.address: row for row in read_file(SOURCE)}
    for address in (0x4C8090, 0x4C8100):
        assert rows[address].source_role == "scaffold"
        assert rows[address].byte_exact
    first = extract(source, "void * __fastcall field_accessor_004c8090_redirect(")
    stores = ("04", "08", "0C", "10", "00", "14", "18", "1C", "20")
    indices = [first.index("receiver->dword_" + offset + "_ =") for offset in stores]
    assert indices == sorted(indices)
    second = extract(source, "void * __fastcall field_accessor_004c8100_redirect(")
    assert second.index("receiver->dword_04_") < second.index("receiver->dword_08_")
    for body in (first, second):
        assert "reinterpret_cast" not in body
        assert "return self;" in body


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None, "last_constant", "omit_store", "clear_prefix"))
def test_actual_field_initializer_bodies(compiler_name, damage):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " is unavailable")
    source = SOURCE.read_text()
    first = extract(source, "void * __fastcall field_accessor_004c8090_redirect(")
    second = extract(source, "void * __fastcall field_accessor_004c8100_redirect(")
    if damage == "last_constant":
        first = first.replace("= 0xFFFFFFFFU;", "= 0;", 1)
    elif damage == "omit_store":
        first = first.replace("receiver->dword_14_ = 0;", "", 1)
    elif damage == "clear_prefix":
        second = second.replace("receiver->dword_04_ = 0;",
                                "receiver->untouched_00_ = 0;", 1)
    translation = PREAMBLE + extract_struct(source, "Fields4C8090View") + "\n"
    translation += extract_struct(source, "Fields4C8100View") + "\n"
    compile_run(compiler, translation + first + "\n" + second + MAIN,
                expected=damage is None)
