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
from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_OP_REG
from owner_transfer_test_support import assert_current_owner, assert_leaf_collection_retired


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
                         "src/net_class.cpp", "src/alphanet.cpp",
                         "src/alphanet_records.cpp")}


def test_erased_hooks_use_payload_fields_and_keep_list_layout(objects):
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


def test_real_tables_dispatch_to_both_erased_hooks(objects):
    # Check every emitted copy; an inline class lifetime can move its provider.
    for name in ("??_7StringStruct@@6B0@@", "??_7StringList@@6BStringStruct@@@"):
        owner = "StringStruct" if name.startswith("??_7StringStruct@") else "StringList"
        providers = [(obj, symbol) for group in objects.values() for obj in group
                     for symbol in obj.symbols.values()
                     if symbol.name == name and symbol.section > 0]
        assert providers, name
        for obj, table in providers:
            section = obj.sections[table.section - 1]
            assert len(section.data) == 8
            assert [obj.symbols[index].name for _, index, _ in section.relocations] == [
                f"?on_insert@{owner}@@UAEHPAX@Z", f"?on_remove@{owner}@@UAEXPAX@Z"]


def test_complete_destructor_is_generated_and_publishes_virtual_base_owner(objects):
    assert_leaf_collection_retired(ROOT)
    assert_current_owner(ROOT, 0x00401520, "src/stringstruct.cpp")
    record, = (r for r in read_file(ROOT / "src/dialog.cpp") if r.address == 0x406820)
    assert record.mode is Mode.FILE
    for obj in objects["src/dialog.cpp"]:
        code, relocations, _ = obj.subject(obj.function(record.symbol))
        decoder = Cs(CS_ARCH_X86, CS_MODE_32)
        decoder.detail = True
        instructions = list(decoder.disasm(code, 0))
        assert references(obj, record.symbol) == [
            "??1StringList@@UAE@XZ", "??_7StringAllocationBase@@6B@",
            "?StringAllocationHeap@@3PAVHeap@@A"]
        def at(instruction, symbol):
            return any(instruction.address <= r.offset < instruction.address + instruction.size
                       and obj.symbols[int(r.target)].name == symbol for r in relocations)
        call, = [i for i in instructions if i.mnemonic == "call"]
        assert at(call, "??1StringList@@UAE@XZ")
        view, = [i for i in instructions if i.mnemonic == "lea"
                 and i.operands[1].type == X86_OP_MEM and i.operands[1].mem.disp == 0x28]
        view_reg = view.operands[0].reg
        owner_load, = [i for i in instructions if i.address > call.address
                       and i.mnemonic == "mov" and i.operands[0].type == X86_OP_REG
                       and i.operands[1].type == X86_OP_MEM
                       and i.operands[1].mem.base == view_reg and i.operands[1].mem.disp == 4]
        stage, = [i for i in instructions if at(i, "??_7StringAllocationBase@@6B@")]
        publication, = [i for i in instructions if at(i, "?StringAllocationHeap@@3PAVHeap@@A")]
        assert call.address < owner_load.address < stage.address < publication.address
        assert stage.operands[0].type == X86_OP_MEM and stage.operands[0].mem.base == view_reg
        assert publication.operands[0].type == X86_OP_MEM
        assert publication.operands[1].type == X86_OP_REG
        assert publication.operands[1].reg == owner_load.operands[0].reg
    for unit in objects.values():
        for obj in unit:
            assert not any(s.section > 0 and s.name == "?destroy@StringList@@QAEIXZ"
                           for s in obj.symbols.values())



def _assert_native_scalar_delete_flow(obj, symbol):
    code, relocations, _ = obj.subject(obj.function(symbol))
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    instructions = list(decoder.disasm(code, 0))
    targets = {
        row.offset: obj.symbols[int(row.target)].name
        for row in relocations
    }

    def relocated_targets(instruction):
        return [name for offset, name in targets.items()
                if instruction.address <= offset < instruction.address + instruction.size]

    heap_symbol = "?StringAllocationHeap@@3PAVHeap@@A"
    base_vtable = "??_7StringAllocationBase@@6B@"
    target_names = references(obj, symbol)
    base_stage = target_names.index(base_vtable)
    tail = target_names[base_stage + 1:]
    if tail == [heap_symbol, "??3NetAllocationNode@@SAXPAX@Z"]:
        # The alternate configured profile keeps the class-specific operator
        # and placement cleanup out of line. Verify those real edges as well
        # as the scalar flag, rather than requiring their inline spelling.
        operator = tail[-1]
        cleanup, = references(obj, operator)
        assert "cleanup_failed_net_construction" in cleanup
        assert references(obj, cleanup) == [heap_symbol, "_free", heap_symbol]
        publication, = [i for i in instructions if heap_symbol in relocated_targets(i)
                         and i.mnemonic == "mov" and i.operands[0].type == X86_OP_MEM]
        flag_test, = [i for i in instructions if i.mnemonic == "test"
                      and i.operands[1].type == X86_OP_IMM and i.operands[1].imm == 1]
        deletion, = [i for i in instructions if operator in relocated_targets(i)]
        flag_branch = next(i for i in instructions if i.address > flag_test.address
                           and i.mnemonic in ("je", "jz"))
        assert publication.address < flag_branch.address < deletion.address
        assert flag_branch.operands[0].imm > deletion.address
        return
    assert tail == [heap_symbol, "_free", heap_symbol]

    heap_stores = [instruction for instruction in instructions
                   if heap_symbol in relocated_targets(instruction)
                   and instruction.mnemonic == "mov"
                   and instruction.operands[0].type == X86_OP_MEM]
    assert len(heap_stores) == 2
    owner_publish = next(instruction for instruction in heap_stores
                         if instruction.operands[1].type == X86_OP_REG)
    handoff_clear = next(instruction for instruction in heap_stores
                         if instruction.operands[1].type == X86_OP_IMM
                         and instruction.operands[1].imm == 0)
    owner_reg = owner_publish.operands[1].reg

    owner_loads = [instruction for instruction in instructions
                   if instruction.mnemonic == "mov"
                   and instruction.operands[0].type == X86_OP_REG
                   and instruction.operands[0].reg == owner_reg
                   and instruction.operands[1].type == X86_OP_MEM
                   and instruction.operands[1].mem.disp in (4, 0x20)
                   and instruction.address < owner_publish.address]
    assert owner_loads

    flag_test, = [instruction for instruction in instructions
                  if instruction.mnemonic == "test"
                  and instruction.operands[0].type == X86_OP_REG
                  and instruction.operands[1].type == X86_OP_IMM
                  and instruction.operands[1].imm == 1]

    def conditional_after(instruction, mnemonics):
        return next(candidate for candidate in instructions
                    if candidate.address > instruction.address
                    and candidate.mnemonic in mnemonics)

    flag_branch = conditional_after(flag_test, ("je", "jz"))
    self_tests = [instruction for instruction in instructions
                  if instruction.address > flag_branch.address
                  and instruction.address < handoff_clear.address
                  and instruction.mnemonic == "test"
                  and len(instruction.operands) == 2
                  and instruction.operands[0].type == X86_OP_REG
                  and instruction.operands[1].type == X86_OP_REG
                  and instruction.operands[0].reg == instruction.operands[1].reg]
    pointer_test = self_tests[0]
    owner_test, = [instruction for instruction in self_tests[1:]
                   if instruction.operands[0].reg == owner_reg]
    pointer_branch = conditional_after(pointer_test, ("je", "jz"))
    owner_branch = conditional_after(owner_test, ("jne", "jnz"))

    free_call, = [instruction for instruction in instructions
                  if instruction.mnemonic == "call"
                  and "_free" in relocated_targets(instruction)]
    assert flag_test.address < owner_publish.address < flag_branch.address
    assert flag_branch.operands[0].imm > handoff_clear.address
    assert pointer_test.address < pointer_branch.address < owner_test.address
    assert pointer_branch.operands[0].imm > handoff_clear.address
    assert owner_test.address < owner_branch.address < free_call.address
    assert owner_branch.operands[0].imm == handoff_clear.address
    assert free_call.address < handoff_clear.address


@pytest.mark.parametrize("unit,owner", [
    ("src/stringstruct.cpp", "StringAllocationBase"),
    ("src/alphanet.cpp", "StringStruct"),
    ("src/dialog.cpp", "StringList"),
    ("src/net_class.cpp", "NetAllocationNode"),
    ("src/net_class.cpp", "NetUnk72C"),
    ("src/net_class.cpp", "JackalVoiceRx"),
    ("src/alphanet.cpp", "AlphaNetStringStruct"),
    ("src/alphanet.cpp", "AlphaNetLedger"),
])
def test_scalar_deleting_entries_publish_owner_and_guard_complete_storage(objects, unit, owner):
    symbol = f"??_G{owner}@@UAEPAXI@Z"
    for obj in objects[unit]:
        _assert_native_scalar_delete_flow(obj, symbol)
    # The generated entry keeps its canonical original identity in the source.
    record, = (r for r in read_file(ROOT / "src/stringstruct.cpp")
               if r.address == 0x401520)
    assert record.mode is Mode.FILE
    assert record.symbol == "??_GStringAllocationBase@@UAEPAXI@Z"



def test_live_hooks_preserve_null_failure_and_heap_ownership(tmp_path, monkeypatch):
    run_recovered_harness(tmp_path, monkeypatch, "stringlist_hooks_harness.cpp", [
        ("// LIVE_INSERT", "src/stringlist.cpp", 0x6086F0),
        ("// LIVE_REMOVE", "src/stringlist.cpp", 0x608770)], declarations=[
        ("// LIVE_TEXT_RELEASE", "src/stringlist.cpp", "namespace {\ninline void release_text",
         "// ORIGINAL: 0x006086F0"),
        ("// LIVE_ALLOCATION_BASE", "src/stringstruct.h", "class StringAllocationBase {",
         "class ListPayloadBase :"),
        ("// LIVE_PAYLOAD_BASE", "src/stringstruct.h", "class ListPayloadBase :", "struct StringStructEntry :"),
        ("// LIVE_PAYLOAD", "src/stringstruct.h", "class StringStructPayload :", "static_assert(sizeof(StringStructPayload)"),
        ("// LIVE_ENTRY", "src/stringstruct.h", "struct StringStructEntry :", "static_assert(sizeof(StringStructEntry)"),
        ("// LIVE_LIST_BASE", "src/stringstruct.h", "class StringStruct :", "static_assert(sizeof(StringStruct)"),
        ("// LIVE_LIST", "src/stringstruct.h", "class StringList :", "static_assert(sizeof(StringList)"),
        ("// LIVE_CONSTRUCTOR", "src/stringstruct.h", "StringStruct::StringStruct() {",
         "class StringList : public StringStruct")])


def test_actual_node_lifetime_uses_complete_allocations_and_heap_ownership(tmp_path, monkeypatch):
    run_recovered_harness(tmp_path, monkeypatch, "stringlist_lifetime_harness.cpp", [
        ("// LIVE_REMOVE", "src/stringlist.cpp", 0x608770)], declarations=[
        ("// LIVE_ALLOCATION_BASE", "src/stringstruct.h", "class StringAllocationBase {",
         "class ListPayloadBase :"),
        ("// LIVE_PAYLOAD_BASE", "src/stringstruct.h", "class ListPayloadBase :", "struct StringStructEntry :"),
        ("// LIVE_CONSTRUCTOR", "src/stringstruct.h", "StringStruct::StringStruct() {",
         "class StringList : public StringStruct"),
        ("// LIVE_PAYLOAD", "src/stringstruct.h", "class StringStructPayload :", "static_assert(sizeof(StringStructPayload)"),
        ("// LIVE_ENTRY", "src/stringstruct.h", "struct StringStructEntry :", "static_assert(sizeof(StringStructEntry)"),
        ("// LIVE_LIST_BASE", "src/stringstruct.h", "class StringStruct :", "static_assert(sizeof(StringStruct)"),
        ("// LIVE_LIST", "src/stringstruct.h", "class StringList :", "static_assert(sizeof(StringList)"),
        ("// LIVE_TEXT_RELEASE", "src/stringlist.cpp", "namespace {\ninline void release_text",
         "// ORIGINAL: 0x006086F0"),
        ])


def test_actual_construction_preserves_allocator_and_partial_failure_contracts(tmp_path, monkeypatch):
    run_recovered_harness(tmp_path, monkeypatch, "stringlist_construction_harness.cpp", [
        ("// LIVE_ADD", "src/stringstruct.cpp", 0x401100)], declarations=[
        ("// LIVE_ALLOCATION_BASE", "src/stringstruct.h", "class StringAllocationBase {",
         "class ListPayloadBase :"),
        ("// LIVE_PAYLOAD_BASE", "src/stringstruct.h", "class ListPayloadBase :", "struct StringStructEntry :"),
        ("// LIVE_CONSTRUCTOR", "src/stringstruct.h", "StringStruct::StringStruct() {",
         "class StringList : public StringStruct"),
        ("// LIVE_PAYLOAD", "src/stringstruct.h", "class StringStructPayload :", "static_assert(sizeof(StringStructPayload)"),
        ("// LIVE_ENTRY", "src/stringstruct.h", "struct StringStructEntry :", "static_assert(sizeof(StringStructEntry)"),
        ("// LIVE_LIST_BASE", "src/stringstruct.h", "class StringStruct :", "static_assert(sizeof(StringStruct)")])
