"""Reusable Dialog cleanup, not full control/game acceptance."""

from test_application_link_recovery import run_recovered_harness
from test_application_link_recovery import references
from test_stringlist_recovery import objects


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
