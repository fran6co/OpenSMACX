"""Actual StringList hook bodies and emitted ABI, not full list/game acceptance."""

from pathlib import Path
import shutil

import pytest

from match_recovery.codeview import class_layout
from match_recovery.compiler import Compiler
from match_recovery.measurement import Measurement
from match_recovery.pe_coff import Coff
from match_recovery.source import read_file
from match_recovery.source.model import Mode
from test_application_link_recovery import run_recovered_harness, references
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86 import X86_OP_IMM


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
            for unit in ("src/stringstruct.cpp", "src/dialog.cpp",
                         "src/net_class.cpp", "src/alphanet.cpp")}


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
    # Standalone base construction now owns its emission in stringstruct.cpp;
    # callers need not emit a duplicate base table to reference that constructor.
    for unit, owner, name in (
            ("src/stringstruct.cpp", "StringStruct", "??_7StringStruct@@6B0@@"),
            ("src/dialog.cpp", "StringList", "??_7StringList@@6BStringStruct@@@")):
        for obj in objects[unit]:
            table, = (symbol for symbol in obj.symbols.values() if symbol.name == name and symbol.section > 0)
            section = obj.sections[table.section - 1]
            # The neighboring BasePop secondary table independently bounds
            # the derived table; its destructor thunk is not a third hook.
            assert len(section.data) == 8
            assert [obj.symbols[index].name for _, index, _ in section.relocations[:2]] == [
                f"?on_insert@{owner}@@UAEHPAVStringStructPayload@@@Z",
                f"?on_remove@{owner}@@UAEXPAVStringStructPayload@@@Z"]


def test_complete_destructor_is_generated_and_destroys_virtual_base(objects):
    record, = (r for r in read_file(ROOT / "src/dialog.cpp") if r.address == 0x406820)
    assert record.mode is Mode.FILE
    for obj in objects["src/dialog.cpp"]:
        assert references(obj, record.symbol) == [
            "??1StringList@@UAE@XZ", "??1StringAllocationBase@@UAE@XZ"]
        code, _, _ = obj.subject(obj.function(record.symbol))
        decoder = Cs(CS_ARCH_X86, CS_MODE_32)
        decoder.detail = True
        calls = [i for i in decoder.disasm(code, 0) if i.mnemonic == "call"]
        assert len(calls) == 2
        assert all(i.operands[0].type == X86_OP_IMM for i in calls)
    for unit in objects.values():
        for obj in unit:
            assert not any(s.section > 0 and s.name == "?destroy@StringList@@QAEIXZ"
                           for s in obj.symbols.values())


@pytest.mark.parametrize("unit,owner", [
    ("src/stringstruct.cpp", "StringAllocationBase"),
    ("src/stringstruct.cpp", "StringStruct"),
    # Real Dialog construction emits this deleting entry; the allocation
    # base TU need not emit every derived class's generated destructor.
    ("src/dialog.cpp", "StringList"),
    ("src/net_class.cpp", "NetAllocationNode"),
    ("src/net_class.cpp", "NetUnk72C"),
    ("src/net_class.cpp", "JackalVoiceRx"),
    ("src/alphanet.cpp", "AlphaNetStringStruct"),
    ("src/alphanet.cpp", "AlphaNetLedger"),
])
def test_scalar_deleting_entries_inherit_owner_aware_deallocation(objects, unit, owner):
    for obj in objects[unit]:
        targets = references(obj, f"??_G{owner}@@UAEPAXI@Z")
        publication = targets.index("??1StringAllocationBase@@UAE@XZ")
        deallocation = targets[publication + 1:]
        if owner == "NetAllocationNode" and any(
                name.startswith("??3NetAllocationNode@@") for name in deallocation):
            # The real context-specific node operators share the source-owned
            # constructor-failure cleanup contract. Follow actual emitted calls
            # instead of requiring its former inherited inline spelling.
            operator, = (name for name in deallocation
                         if name.startswith("??3NetAllocationNode@@"))
            assert deallocation == [operator]
            operator_targets = references(obj, operator)
            cleanup, = (name for name in operator_targets
                        if "cleanup_failed_net_construction" in name)
            assert operator_targets == [cleanup]
            deallocation = references(obj, cleanup)
        assert deallocation == [
            "?StringAllocationHeap@@3PAVHeap@@A", "_free",
            "?StringAllocationHeap@@3PAVHeap@@A"]
        assert "??3@YAXPAX@Z" not in targets
    # The generated entry owns the existing identity; no copied assembly
    # or extra wrapper is needed to force emission.
    record, = (r for r in read_file(ROOT / "src/stringstruct.cpp") if r.address == 0x401520)
    assert record.mode is Mode.FILE
    assert record.symbol == "??_GStringAllocationBase@@UAEPAXI@Z"
    assert not any(r.address == 0x401520 for r in read_file(ROOT / "src/leaf_recoveries.cpp"))


def test_live_hooks_preserve_null_failure_and_heap_ownership(tmp_path, monkeypatch):
    run_recovered_harness(tmp_path, monkeypatch, "stringlist_hooks_harness.cpp", [
        ("// LIVE_INSERT", "src/stringlist.cpp", 0x6086F0),
        ("// LIVE_REMOVE", "src/stringlist.cpp", 0x608770)], declarations=[
        ("// LIVE_TEXT_RELEASE", "src/stringlist.cpp", "namespace {\ninline void release_text",
         "// ORIGINAL: 0x006086F0"),
        ("// LIVE_ALLOCATION_BASE", "src/stringstruct.h", "class StringAllocationBase {",
         "// REAL VIRTUAL INHERITANCE, like StringStructPayload"),
        ("// LIVE_PAYLOAD", "src/stringstruct.h", "class StringStructPayload :", "static_assert(sizeof(StringStructPayload)"),
        ("// LIVE_ENTRY", "src/stringstruct.h", "struct StringStructEntry :", "static_assert(sizeof(StringStructEntry)"),
        ("// LIVE_LIST_BASE", "src/stringstruct.h", "class StringStruct :", "static_assert(sizeof(StringStruct)"),
        ("// LIVE_LIST", "src/stringstruct.h", "class StringList :", "static_assert(sizeof(StringList)"),
        ("// LIVE_CONSTRUCTOR", "src/stringstruct.cpp", "StringStruct::StringStruct() {",
         "// The original primary table uses these shared defaults.")])


def test_actual_node_lifetime_uses_complete_allocations_and_heap_ownership(tmp_path, monkeypatch):
    run_recovered_harness(tmp_path, monkeypatch, "stringlist_lifetime_harness.cpp", [
        ("// LIVE_CLEAR", "src/stringstruct.cpp", 0x402970),
        ("// LIVE_REMOVE", "src/stringlist.cpp", 0x608770)], declarations=[
        ("// LIVE_ALLOCATION_BASE", "src/stringstruct.h", "class StringAllocationBase {",
         "// REAL VIRTUAL INHERITANCE, like StringStructPayload"),
        ("// LIVE_OWNER_PUBLICATION", "src/stringstruct.cpp", "StringAllocationBase::~StringAllocationBase() {",
         "// ORIGINAL: 0x00401000"),
        ("// LIVE_CONSTRUCTOR", "src/stringstruct.cpp", "StringStruct::StringStruct() {",
         "// The original primary table uses these shared defaults."),
        ("// LIVE_PAYLOAD", "src/stringstruct.h", "class StringStructPayload :", "static_assert(sizeof(StringStructPayload)"),
        ("// LIVE_ENTRY", "src/stringstruct.h", "struct StringStructEntry :", "static_assert(sizeof(StringStructEntry)"),
        ("// LIVE_LIST_BASE", "src/stringstruct.h", "class StringStruct :", "static_assert(sizeof(StringStruct)"),
        ("// LIVE_LIST", "src/stringstruct.h", "class StringList :", "static_assert(sizeof(StringList)"),
        ("// LIVE_CLEANUP_HELPER", "src/stringstruct.cpp", "namespace {\n// Share the original entry walk",
         "/*\nPurpose: Return the current string-list item ID."),
        ("// LIVE_TEXT_RELEASE", "src/stringlist.cpp", "namespace {\ninline void release_text",
         "// ORIGINAL: 0x006086F0"),
        ("// LIVE_BASE_DESTRUCTOR", "src/stringstruct.cpp", "StringStruct::~StringStruct() {",
         "// StringStructDerivedVtable / StringStructDerivedVirtualBaseVtable"),
        ("// LIVE_DERIVED_DESTRUCTOR", "src/stringlist.cpp", "StringList::~StringList() {",
         None)])


def test_actual_construction_preserves_allocator_and_partial_failure_contracts(tmp_path, monkeypatch):
    run_recovered_harness(tmp_path, monkeypatch, "stringlist_construction_harness.cpp", [
        ("// LIVE_ADD", "src/stringstruct.cpp", 0x401100),
        ("// LIVE_CLEAR", "src/stringstruct.cpp", 0x402970)], declarations=[
        ("// LIVE_ALLOCATION_BASE", "src/stringstruct.h", "class StringAllocationBase {",
         "// REAL VIRTUAL INHERITANCE, like StringStructPayload"),
        ("// LIVE_OWNER_PUBLICATION", "src/stringstruct.cpp", "StringAllocationBase::~StringAllocationBase() {",
         "// ORIGINAL: 0x00401000"),
        ("// LIVE_CONSTRUCTOR", "src/stringstruct.cpp", "StringStruct::StringStruct() {",
         "// The original primary table uses these shared defaults."),
        ("// LIVE_PAYLOAD", "src/stringstruct.h", "class StringStructPayload :", "static_assert(sizeof(StringStructPayload)"),
        ("// LIVE_ENTRY", "src/stringstruct.h", "struct StringStructEntry :", "static_assert(sizeof(StringStructEntry)"),
        ("// LIVE_LIST_BASE", "src/stringstruct.h", "class StringStruct :", "static_assert(sizeof(StringStruct)"),
        ("// LIVE_CLEANUP_HELPER", "src/stringstruct.cpp", "namespace {\n// Share the original entry walk",
         "/*\nPurpose: Return the current string-list item ID."),
        ("// LIVE_BASE_DESTRUCTOR", "src/stringstruct.cpp", "StringStruct::~StringStruct() {",
         "// StringStructDerivedVtable / StringStructDerivedVirtualBaseVtable"),
        ("// LIVE_NODE_ALLOCATION", "src/stringstruct.cpp", "namespace {\n// 0x00401100 selects",
         "// Append before the circular head.")])
