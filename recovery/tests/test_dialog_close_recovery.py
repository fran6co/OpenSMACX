"""Reusable Dialog cleanup, not full control/game acceptance."""

from test_application_link_recovery import run_recovered_harness
from test_application_link_recovery import references
from test_stringlist_recovery import objects
from test_application_link_recovery import ROOT, symbol_at
from match_recovery.compiler import Compiler
from match_recovery.measurement import Measurement
from match_recovery.pe_coff import Coff


def test_close_calls_reset_without_ending_member_lifetimes(objects):
    for obj in objects["src/dialog.cpp"]:
        targets = references(obj, "?close@Dialog@@QAEXXZ")
        assert targets.count("?reset@Heap@@QAEXXZ") == 1
        assert targets.count("?remove_all@StringStruct@@QAEXXZ") == 1
        assert not any(name.startswith(("??1", "??_D")) for name in targets)
        assert not any("Original" in name or "pending" in name.lower() for name in targets)


def test_close_resets_live_members_in_order_and_can_be_reused(tmp_path, monkeypatch):
    run_recovered_harness(tmp_path, monkeypatch, "dialog_close_harness.cpp", [
        ("// LIVE_DIALOG_CLOSE", "src/dialog.cpp", 0x608F50),
        ("// LIVE_HEAP_CONSTRUCTOR", "src/heap.cpp", 0x5D4560),
        ("// LIVE_HEAP_DESTRUCTOR", "src/heap.cpp", 0x5D4580)], declarations=[
        ("// LIVE_HEAP_DECLARATION", "src/heap.h", "class Heap {", "#if defined(_M_IX86)"),
        ("// LIVE_HEAP_RESET", "src/heap.cpp", "void Heap::reset() {", "/*\nPurpose: Deflate"),
        ("// LIVE_DIALOG_DECLARATION", "src/dialog.h", "class Dialog {", "static_assert(sizeof(Dialog)")])


def test_actual_destructor_and_generated_delete_own_member_lifetimes(tmp_path, monkeypatch):
    # This historical block contains quoted destructor bodies; select the
    # actual definition with unique anchors, as the list lifetime harness does.
    run_recovered_harness(tmp_path, monkeypatch, "dialog_lifetime_harness.cpp", [], declarations=[
        ("// LIVE_DIALOG_DECL", "src/dialog.h", "class Dialog {", "static_assert(sizeof(Dialog)"),
        ("// LIVE_DIALOG_DESTRUCTION", "src/dialog.cpp", "Dialog::~Dialog() {",
         "// Retained leaf callers still use this callback ABI."),
        ("// LIVE_DIALOG_CALLBACK", "src/dialog.cpp", "void __fastcall dialog_destructor_redirect(",
         "/*\nPurpose: The compiler-generated scalar deleting destructor:")])


def test_generated_scalar_entry_targets_actual_destructor(objects):
    for obj in objects["src/dialog.cpp"]:
        targets = references(obj, "??_GDialog@@UAEPAXI@Z")
        assert targets == ["??1Dialog@@UAE@XZ", "??3@YAXPAX@Z"]
        assert not any(s.name in {"?destroy@Dialog@@QAEXXZ",
                                 "?dialog_scalar_dtor_redirect@@YIPAXPAVDialog@@PAXI@Z"}
                       and s.section > 0 for s in obj.symbols.values())


def test_retained_callback_directly_destroys_the_named_base(objects):
    for obj in objects["src/dialog.cpp"]:
        assert references(obj, "?dialog_destructor_redirect@@YIXPAVDialog@@PAX@Z") == [
            "??1Dialog@@UAE@XZ"]


def test_retained_external_callers_reach_actual_dialog_destructor(objects):
    # Reuse the fixture's compiler-availability control. Original direct calls
    # are at 40711E and 407F45; other teardown in these callers is still pending.
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    for unit, address in (("src/dialogs.cpp", 0x407100), ("src/filewin.cpp", 0x407F00)):
        for flags in settings.flag_sets:
            obj = Coff(compiler(ROOT / unit, (), flags + settings.layout_flags))
            targets = references(obj, symbol_at(unit, address))
            # Check the explicit source call immediately after the enclosing
            # legacy stage. FileWin also emits automatic member destruction;
            # its pre-existing double teardown is a tracked recovery finding,
            # not a lifetime property this bounded caller repair can certify.
            previous = ("?destroy@Dialogs@@QAEIXZ" if address == 0x407100
                        else "?destroy@ListBox@@QAEIXZ")
            assert targets[targets.index(previous) + 1] == "??1Dialog@@UAE@XZ"
            assert "?destroy@Dialog@@QAEXXZ" not in targets
