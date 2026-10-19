"""Reusable Dialog cleanup, not full control/game acceptance."""

from test_application_link_recovery import run_recovered_harness
from test_application_link_recovery import references
from test_stringlist_recovery import objects
from test_application_link_recovery import ROOT, symbol_at
from match_recovery.compiler import Compiler
from match_recovery.measurement import Measurement
from match_recovery.pe_coff import Coff
from capstone import Cs, CS_ARCH_X86, CS_MODE_32, CS_GRP_JUMP
from capstone.x86 import X86_OP_IMM, X86_OP_MEM


def test_close_inlines_list_removal_before_reset_without_ending_member_lifetimes(objects):
    for obj in objects["src/dialog.cpp"]:
        function = obj.function("?close@Dialog@@QAEXXZ")
        code, relocations, _ = obj.subject(function)
        reset_relocations = [reloc for reloc in relocations
                             if obj.symbols[int(reloc.target)].name == "?reset@Heap@@QAEXXZ"]
        assert len(reset_relocations) == 1
        reset_call_offset = reset_relocations[0].offset - 1  # call rel32 starts one byte before its displacement
        decoder = Cs(CS_ARCH_X86, CS_MODE_32)
        decoder.detail = True
        instructions = tuple(decoder.disasm(code, 0))
        reset_call, = (instruction for instruction in instructions
                       if instruction.address == reset_call_offset and instruction.mnemonic == "call")
        before_reset = tuple(instruction for instruction in instructions
                             if instruction.address < reset_call.address)

        # StringList::remove_all is inline here: its virtual removal hook,
        # payload delete and entry delete are indirect calls in a loop, not
        # a relocation to a standalone remove_all function.
        indirect_calls = [instruction for instruction in before_reset
                          if instruction.mnemonic == "call" and instruction.operands
                          and instruction.operands[0].type == X86_OP_MEM]
        assert len(indirect_calls) == 3
        backedges = [instruction for instruction in before_reset
                     if instruction.group(CS_GRP_JUMP) and instruction.operands
                     and instruction.mnemonic != "jmp"
                     and instruction.operands[0].type == X86_OP_IMM
                     and instruction.operands[0].imm <= indirect_calls[0].address
                     and instruction.address > indirect_calls[-1].address]
        assert backedges, "inlined list cleanup must retain its iteration backedge before Heap::reset"

        targets = [obj.symbols[int(reloc.target)].name for reloc in relocations]
        assert targets.count("?reset@Heap@@QAEXXZ") == 1
        assert not any(name.startswith(("??1", "??_D")) for name in targets)
        assert not any("Original" in name or "pending" in name.lower() for name in targets)


def test_close_resets_live_members_in_order_and_can_be_reused(tmp_path, monkeypatch):
    run_recovered_harness(tmp_path, monkeypatch, "dialog_close_harness.cpp", [
        ("// LIVE_DIALOG_CLOSE", "src/dialog.cpp", 0x608F50),
        ("// LIVE_HEAP_CONSTRUCTOR", "src/heap.cpp", 0x5D4560),
        ("// LIVE_HEAP_DESTRUCTOR", "src/heap.cpp", 0x5D45B0)], declarations=[
        ("// LIVE_DIALOG_DEFAULTS_HEADER", "src/dialog_defaults.h",
         "// The pinned image has an observed 148-byte", None),
        ("// LIVE_HEAP_DECLARATION", "src/heap.h", "class Heap {", "#if defined(_M_IX86)"),
        ("// LIVE_HEAP_RESET", "src/heap.cpp", "void Heap::reset() {", "/*\nPurpose: Shutdown"),
        ("// LIVE_DIALOG_DECLARATION", "src/dialog.h", "class Dialog {", "static_assert(sizeof(Dialog)")])


def test_actual_destructor_and_generated_delete_own_member_lifetimes(tmp_path, monkeypatch):
    # This historical block contains quoted destructor bodies; select the
    # actual definition with unique anchors, as the list lifetime harness does.
    run_recovered_harness(tmp_path, monkeypatch, "dialog_lifetime_harness.cpp", [], declarations=[
        ("// LIVE_DIALOG_DECL", "src/dialog.h", "class Dialog {", "static_assert(sizeof(Dialog)"),
        ("// LIVE_DIALOG_DESTRUCTION", "src/dialog.cpp", "Dialog::~Dialog() {",
         "/*\nPurpose: The compiler-generated scalar deleting destructor:")])


def test_generated_scalar_entry_targets_actual_destructor(objects):
    for obj in objects["src/dialog.cpp"]:
        targets = references(obj, "??_GDialog@@UAEPAXI@Z")
        assert targets == ["??1Dialog@@UAE@XZ", "??3@YAXPAX@Z"]
        assert not any(s.name in {"?destroy@Dialog@@QAEXXZ",
                                 "?dialog_scalar_dtor_redirect@@YIPAXPAVDialog@@PAXI@Z"}
                       and s.section > 0 for s in obj.symbols.values())


def test_obsolete_destructor_callback_is_not_emitted(objects):
    for obj in objects["src/dialog.cpp"]:
        assert not any(s.name == "?dialog_destructor_redirect@@YIXPAVDialog@@PAX@Z"
                       and s.section > 0 for s in obj.symbols.values())


def test_retained_external_callers_reach_actual_dialog_destructor(objects):
    # Reuse the fixture's compiler-availability control. The primary deleting
    # entry now belongs to the real compiler-generated Dialogs lifetime chain.
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    for unit, address in (("src/dialogs.cpp", 0x407100),):
        for flags in settings.flag_sets:
            obj = Coff(compiler(ROOT / unit, (), flags + settings.layout_flags))
            targets = references(obj, symbol_at(unit, address))
            assert targets == ["??1Dialogs@@UAE@XZ", "??1Dialog@@UAE@XZ",
                               "??1GraphicWin@@UAE@XZ", "??3@YAXPAX@Z"]
