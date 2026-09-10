"""Actual StringList hook bodies and emitted ABI, not full list/game acceptance."""

from pathlib import Path
import shutil

import pytest

from match_recovery.codeview import class_layout
from match_recovery.compiler import Compiler
from match_recovery.measurement import Measurement
from match_recovery.pe_coff import Coff
from test_application_link_recovery import run_recovered_harness


ROOT = Path(__file__).resolve().parents[2]


@pytest.fixture(scope="module")
def objects():
    from recovery.environment.wine import VC6_ROOT
    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine32") is None:
        pytest.skip("VC6 and Wine32 are required")
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    return {unit: [Coff(compiler(ROOT / unit, (), flags + settings.layout_flags))
                   for flags in settings.flag_sets]
            for unit in ("src/stringstruct.cpp", "src/dialog.cpp")}


def test_hooks_use_payload_fields_and_keep_list_layout(objects):
    for obj in objects["src/stringstruct.cpp"]:
        payload = class_layout(obj, "StringStructPayload")
        assert payload["size"] == 0x18
        assert {name: payload["members"][name] for name in ("text", "field_8_", "field_C_")} == {
            "text": 4, "field_8_": 8, "field_C_": 12}
        base = class_layout(obj, "StringStruct")
        derived = class_layout(obj, "StringList")
        assert base["size"] == 0x24 and derived["size"] == 0x30
        assert base["members"]["allocator_"] == 0x18
        assert derived["members"]["pending_text_"] == 0x1C
        assert base["member_type_kinds"]["allocator_"] == "pointer"
        # The CV5 reader currently classifies primitive pointers only as
        # non-class values. The live C++ harness checks the exact char* member
        # type through a pointer-to-member, not a size/offset inference.


def test_real_tables_dispatch_to_both_typed_hooks(objects):
    for obj in objects["src/dialog.cpp"]:
        for owner, name in (("StringStruct", "??_7StringStruct@@6B0@@"),
                            ("StringList", "??_7StringList@@6BStringStruct@@@")):
            table, = (symbol for symbol in obj.symbols.values() if symbol.name == name and symbol.section > 0)
            section = obj.sections[table.section - 1]
            # The neighboring BasePop secondary table independently bounds
            # the derived table; its destructor thunk is not a third hook.
            assert len(section.data) == 8
            assert [obj.symbols[index].name for _, index, _ in section.relocations[:2]] == [
                f"?on_insert@{owner}@@UAEHPAVStringStructPayload@@@Z",
                f"?on_remove@{owner}@@UAEXPAVStringStructPayload@@@Z"]


def test_live_hooks_preserve_null_failure_and_heap_ownership(tmp_path, monkeypatch):
    run_recovered_harness(tmp_path, monkeypatch, "stringlist_hooks_harness.cpp", [
        ("// LIVE_INSERT", "src/stringstruct.cpp", 0x6086F0),
        ("// LIVE_REMOVE", "src/stringstruct.cpp", 0x608770)], declarations=[
        ("// LIVE_TEXT_RELEASE", "src/stringstruct.cpp", "namespace {\n// The cleanup path",
         "// The original primary table uses these shared defaults."),
        ("// LIVE_ALLOCATION_BASE", "src/stringstruct.h", "class StringAllocationBase {",
         "// REAL VIRTUAL INHERITANCE, like StringStructPayload"),
        ("// LIVE_PAYLOAD", "src/stringstruct.h", "class StringStructPayload :", "static_assert(sizeof(StringStructPayload)"),
        ("// LIVE_ENTRY", "src/stringstruct.h", "struct StringStructEntry :", "static_assert(sizeof(StringStructEntry)"),
        ("// LIVE_LIST_BASE", "src/stringstruct.h", "class StringStruct :", "static_assert(sizeof(StringStruct)"),
        ("// LIVE_LIST", "src/stringstruct.h", "class StringList :", "static_assert(sizeof(StringList)")])


def test_actual_node_lifetime_uses_complete_allocations_and_heap_ownership(tmp_path, monkeypatch):
    run_recovered_harness(tmp_path, monkeypatch, "stringlist_lifetime_harness.cpp", [
        ("// LIVE_CLEAR", "src/stringstruct.cpp", 0x402970),
        ("// LIVE_REMOVE", "src/stringstruct.cpp", 0x608770)], declarations=[
        ("// LIVE_ALLOCATION_BASE", "src/stringstruct.h", "class StringAllocationBase {",
         "// REAL VIRTUAL INHERITANCE, like StringStructPayload"),
        ("// LIVE_OWNER_PUBLICATION", "src/stringstruct.cpp", "StringAllocationBase::~StringAllocationBase() {",
         "namespace {\n// The cleanup path"),
        ("// LIVE_PAYLOAD", "src/stringstruct.h", "class StringStructPayload :", "static_assert(sizeof(StringStructPayload)"),
        ("// LIVE_ENTRY", "src/stringstruct.h", "struct StringStructEntry :", "static_assert(sizeof(StringStructEntry)"),
        ("// LIVE_LIST_BASE", "src/stringstruct.h", "class StringStruct :", "static_assert(sizeof(StringStruct)"),
        ("// LIVE_LIST", "src/stringstruct.h", "class StringList :", "static_assert(sizeof(StringList)"),
        ("// LIVE_TEXT_RELEASE", "src/stringstruct.cpp", "namespace {\n// The cleanup path",
         "// The original primary table uses these shared defaults."),
        ("// LIVE_BASE_DESTRUCTOR", "src/stringstruct.cpp", "StringStruct::~StringStruct() {",
         "/*\nPurpose: Return the current string-list item ID.")])


def test_actual_construction_preserves_allocator_and_partial_failure_contracts(tmp_path, monkeypatch):
    run_recovered_harness(tmp_path, monkeypatch, "stringlist_construction_harness.cpp", [
        ("// LIVE_ADD", "src/stringstruct.cpp", 0x401100),
        ("// LIVE_CLEAR", "src/stringstruct.cpp", 0x402970)], declarations=[
        ("// LIVE_ALLOCATION_BASE", "src/stringstruct.h", "class StringAllocationBase {",
         "// REAL VIRTUAL INHERITANCE, like StringStructPayload"),
        ("// LIVE_OWNER_PUBLICATION", "src/stringstruct.cpp", "StringAllocationBase::~StringAllocationBase() {",
         "namespace {\n// The cleanup path"),
        ("// LIVE_PAYLOAD", "src/stringstruct.h", "class StringStructPayload :", "static_assert(sizeof(StringStructPayload)"),
        ("// LIVE_ENTRY", "src/stringstruct.h", "struct StringStructEntry :", "static_assert(sizeof(StringStructEntry)"),
        ("// LIVE_LIST_BASE", "src/stringstruct.h", "class StringStruct :", "static_assert(sizeof(StringStruct)"),
        ("// LIVE_BASE_DESTRUCTOR", "src/stringstruct.cpp", "StringStruct::~StringStruct() {",
         "/*\nPurpose: Return the current string-list item ID."),
        ("// LIVE_NODE_ALLOCATION", "src/stringstruct.cpp", "namespace {\n// 0x00401100 selects",
         "// Append before the circular head.")])
