"""Live control-global and keyboard regressions; not full UI/game acceptance."""

from dataclasses import replace
import os
from pathlib import Path
import shutil

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32, CS_AC_READ, CS_AC_WRITE

from match_recovery.compiler import Compiler
from match_recovery.bound_objects import FunctionBinding, check_objects
from match_recovery.import_source import import_targets, read_imports
from match_recovery.measurement import Measurement
from match_recovery.model import Origin, Span
from match_recovery.object_source import object_targets, read_objects
from match_recovery.object_data import check_initial_data
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.source.reader import read_file
from match_recovery.source_inventory import image_pin
from test_application_link_recovery import ROOT, run_recovered_harness, symbol_at
from test_graphicwin_recovery import global_accesses
import test_application_link_recovery as harness_support


@pytest.fixture(scope="module")
def control_globals():
    from recovery.environment.wine import VC6_ROOT
    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine32") is None:
        pytest.skip("VC6 and Wine32 are required")
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    objects = {unit: [Coff(compiler(ROOT / unit, (), flags + settings.layout_flags))
                      for flags in settings.flag_sets]
               for unit in ("src/win.cpp", "src/editbox.cpp", "src/listbox.cpp")}
    return compiler, settings, objects


def test_keyboard_updates_pointer_storage_not_the_pointed_to_window(control_globals):
    _, _, objects = control_globals
    handler = symbol_at("src/win.cpp", 0x5F5FB0)
    assert handler == "?on_char@Win@@UAEXDH@Z"
    for obj in objects["src/win.cpp"]:
        for symbol in ("?WinPointerOwner4@@3PAVWin@@A", "?WinZOrderFlag@@3HA"):
            assert any(access & CS_AC_WRITE for access in global_accesses(obj, handler, symbol))
        for symbol in ("?WinRootCount@@3HA", "?JackalInitFlags@@3HA", "?WinZOrderWindow@@3PAVWin@@A"):
            assert any(access & CS_AC_READ for access in global_accesses(obj, handler, symbol))


def test_compiled_damage_control_cannot_hide_a_pointee_write(control_globals):
    compiler, settings, _ = control_globals
    path = ROOT / "src/win.cpp"
    source = path.read_bytes()
    good = b"if (WinPointerOwner4 == &WinBubbleWindow) {\n                WinPointerOwner4 = 0;"
    bad = b"if (*(void **)WinPointerOwner4 == &WinBubbleWindow) {\n                *(int *)WinPointerOwner4 = 0;"
    assert source.count(good) == 1
    for flags in settings.flag_sets:
        obj = Coff(compiler.overlay(path, {"src/win.cpp": source.replace(good, bad)},
                                    (), flags + settings.layout_flags))
        accesses = global_accesses(obj, symbol_at("src/win.cpp", 0x5F5FB0),
                                   "?WinPointerOwner4@@3PAVWin@@A")
        assert accesses and not any(access & CS_AC_WRITE for access in accesses)
    assert path.read_bytes() == source


def test_editbox_key_state_import_resolves_to_the_pinned_sdk_slot(control_globals):
    _, _, objects = control_globals
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned image is required")
    image = PeImage(path.read_bytes(), image_pin(ROOT))
    entry, = (entry for entry in image.imports() if entry.module.lower() == "user32.dll"
              and entry.name == "GetAsyncKeyState")
    for obj in objects["src/editbox.cpp"]:
        targets, _ = import_targets(read_imports(ROOT), {"src/editbox.cpp": obj}, image)
        assert targets["__imp__GetAsyncKeyState@4"] == entry.address


def test_live_keyboard_bubble_owners_and_mutable_z_order(tmp_path, monkeypatch):
    run_recovered_harness(tmp_path, monkeypatch, "win_keyboard_harness.cpp", [
        ("// LIVE_KEYBOARD", "src/win.cpp", 0x5F5FB0)])


def test_live_keyboard_control_rejects_the_old_extra_indirection(tmp_path, monkeypatch):
    reader = harness_support.read_file

    def damaged(path):
        result = []
        for record in reader(path):
            if record.address == 0x5F5FB0:
                good = "if (WinPointerOwner4 == &WinBubbleWindow) {\n                WinPointerOwner4 = 0;"
                bad = "if (*(void **)WinPointerOwner4 == &WinBubbleWindow) {\n                *(int *)WinPointerOwner4 = 0;"
                assert record.region.count(good) == 1
                record = replace(record, region=record.region.replace(good, bad))
            result.append(record)
        return result

    monkeypatch.setattr(harness_support, "read_file", damaged)
    with pytest.raises(AssertionError, match="Assertion failed"):
        test_live_keyboard_bubble_owners_and_mutable_z_order(tmp_path, monkeypatch)


@pytest.mark.parametrize("good,bad", [
    ("debug_window.win_parent_->is_visible() == 0",
     "debug_window.win_parent_->is_visible() != 0"),
    ("} while (i < debug_window.child_count_);", "} while (i < 2);"),
    ("Win **list = debug_window.children_;",
     "Win *snapshot[4]; for (int k = 0; k < 4; ++k) "
     "snapshot[k] = debug_window.children_[k]; Win **list = snapshot;"),
    ("Win *w = debug_window.val_4_;", "Win *w = val_4_;"),
])
def test_live_keyboard_controls_require_owned_members_and_fresh_children(
        tmp_path, monkeypatch, good, bad):
    reader = harness_support.read_file

    def damaged(path):
        result = []
        for record in reader(path):
            if record.address == 0x5F5FB0:
                assert record.region.count(good) == 1
                record = replace(record, region=record.region.replace(good, bad))
            result.append(record)
        return result

    monkeypatch.setattr(harness_support, "read_file", damaged)
    with pytest.raises(AssertionError, match="Assertion failed"):
        test_live_keyboard_bubble_owners_and_mutable_z_order(tmp_path, monkeypatch)


def test_live_hide_reads_flag_value_and_mutable_window_state(tmp_path, monkeypatch):
    run_recovered_harness(tmp_path, monkeypatch, "win_hide_harness.cpp", [
        ("// LIVE_HIDE", "src/win.cpp", 0x5EDCD0)])


def test_live_cursor_uses_shared_storage_and_observable_callback_state(tmp_path, monkeypatch):
    run_recovered_harness(tmp_path, monkeypatch, "win_cursor_harness.cpp", [
        ("// LIVE_CURSOR", "src/win.cpp", 0x5F1820)], declarations=[
        ("// LIVE_RESTORE", "src/win.cpp", "void __cdecl restore_flip_sprite(int do_flip) {",
         "\n\n\n/*\nPurpose: Run the teardown step"),
        ("// LIVE_RECT_HELPERS", "src/win.cpp", "static void BuildRectDirect(RECT &r) {",
         "\nnamespace {\n// SCROLL'S SLOT 25")])


@pytest.mark.parametrize("good,bad", [
    ("int destX = screen_x - WinDragOffsetX;", "int destX = hit_x - WinDragOffsetX;"),
    ("if (tgl == 0)", "if (rect.bottom == 0)"),
    ("ScreenBuffer.copy(&g_WIN_BUFFER, destX, destY, 0, 0, width, height);",
     "g_WIN_BUFFER.copy(&ScreenBuffer, 0, 0, destX, destY, width, height);"),
    ("SetCursor(*win->cursor_handle_)",
     "SetCursor(LoadCursorA(0, reinterpret_cast<const char *>(*win->cursor_handle_)))"),
    ("rect.left = WinFlipSpriteY;", "rect.left = destX;"),
    ("result = WinDirtyRect.right;", "result = 0;"),
    ("int result = UnionRect(&WinDirtyRect, &rect, &WinDirtyRect);",
     "RECT merged; int result = UnionRect(&merged, &rect, &WinDirtyRect);"),
    ("if (WinMouseDirect != 0 || win->cursor_sprite_ != 0)", "if (false)"),
    ("g_WIN_BUFFER.init(WinClipWidth, WinClipHeight, 0, 0);",
     "g_WIN_BUFFER.init(reinterpret_cast<int>(win->cursor_sprite_), 0, 0, 0);"),
    ("} while (result <= 0);\n        return result;", "} while (result <= 0);\n        return 0;"),
    ("WinFlipSprite = 0;\n            Win::flip(&WinDirtyRect);",
     "Win::flip(&WinDirtyRect);\n            WinFlipSprite = 0;"),
])
def test_cursor_controls_reject_recovered_dataflow_defects(tmp_path, monkeypatch, good, bad):
    reader = harness_support.read_file

    def damaged(path):
        result = []
        for record in reader(path):
            if record.address == 0x5F1820:
                assert record.region.count(good) == 1
                record = replace(record, region=record.region.replace(good, bad))
            result.append(record)
        return result

    monkeypatch.setattr(harness_support, "read_file", damaged)
    with pytest.raises(AssertionError, match="Assertion failed"):
        test_live_cursor_uses_shared_storage_and_observable_callback_state(tmp_path, monkeypatch)


def test_root_registry_and_cursor_rectangle_have_independent_bounded_storage(control_globals):
    compiler, settings, objects = control_globals
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned image is required")
    pin = image_pin(ROOT)
    image = PeImage(path.read_bytes(), pin)
    symbols = {"?WinRootWindows@@3PAPAVWin@@A", "?WinDirtyRect@@3UtagRECT@@A"}
    bindings = [b for b in read_objects(ROOT, pin) if b.symbol in symbols]
    assert len(bindings) == 2

    def check(obj):
        return check_initial_data(bindings, {"src/win.cpp": obj}, image, {})

    source = ROOT / "src/win.cpp"
    before = source.read_bytes()
    old = b"Win *WinRootWindows[40];"
    assert before.count(old) == 1
    for flags, obj in zip(settings.flag_sets, objects["src/win.cpp"]):
        rows = check(obj)
        assert all(r["content"] == r["proof"]["boundary"] == "pass" for r in rows)
        assert {r["symbol"]: r["compiler_storage"]["size"] for r in rows} == {
            "?WinRootWindows@@3PAPAVWin@@A": 160, "?WinDirtyRect@@3UtagRECT@@A": 16}
        assert not any("WinModalResult" in s.name or "WinSavedArea" in s.name
                       for s in obj.symbols.values())
        damaged = Coff(compiler.overlay(source, {"src/win.cpp": before.replace(
            old, b"Win *WinRootWindows[64];")}, (), flags + settings.layout_flags))
        row, = (r for r in check(damaged) if "WinRootWindows" in r["symbol"])
        assert row["content"] == row["proof"]["boundary"] == "fail"
    assert source.read_bytes() == before


def test_live_hide_control_rejects_flag_value_as_address(tmp_path, monkeypatch):
    reader = harness_support.read_file

    def damaged(path):
        result = []
        for record in reader(path):
            if record.address == 0x5EDCD0:
                good = "if ((JackalInitFlags & 1) == 0) {"
                bad = "if (!(*reinterpret_cast<uint8_t *>(JackalInitFlags) & 1)) {"
                assert record.region.count(good) == 1
                record = replace(record, region=record.region.replace(good, bad))
            result.append(record)
        return result

    monkeypatch.setattr(harness_support, "read_file", damaged)
    with pytest.raises(AssertionError, match="Assertion failed"):
        test_live_hide_reads_flag_value_and_mutable_window_state(tmp_path, monkeypatch)


def test_live_root_order_uses_receiver_free_lift_and_mutable_registry(tmp_path, monkeypatch):
    run_recovered_harness(tmp_path, monkeypatch, "win_root_order_harness.cpp", [
        ("// LIVE_ROOT_LIFT", "src/win.cpp", 0x5F5140),
        ("// LIVE_SHOW", "src/win.cpp", 0x5ED9D0)], declarations=[
        ("// LIVE_ROOT_DECLARATION", "src/win.h",
         "  static void __cdecl bring_parent_to_top(Win * window);",
         "  void draw_rect_border(")])


def test_root_lift_callers_use_static_cdecl_and_reclaim_one_argument(control_globals):
    _, _, objects = control_globals
    helper = symbol_at("src/win.cpp", 0x5F5140)
    assert helper == "?bring_parent_to_top@Win@@SAXPAV1@@Z"
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    for obj in objects["src/win.cpp"]:
        code, _, _ = obj.subject(obj.function(helper))
        returns = [op for op in decoder.disasm(code, 0) if op.mnemonic == "ret"]
        assert returns and all(not op.op_str for op in returns)
        for address in (0x5ED9D0, 0x5F6710):
            code, relocations, _ = obj.subject(obj.function(symbol_at("src/win.cpp", address)))
            offset, = (row.offset for row in relocations if obj.symbols[int(row.target)].name == helper)
            instructions = list(decoder.disasm(code, 0))
            index, = (i for i, op in enumerate(instructions)
                      if op.address <= offset < op.address + op.size)
            assert instructions[index].mnemonic == "call"
            cleanup = instructions[index + 1]
            assert (cleanup.mnemonic, cleanup.op_str) == ("add", "esp, 4")


@pytest.mark.parametrize("address,good,bad", [
    (0x5ED9D0, "} while (i < WinRootCount);", "} while (i < count);"),
    (0x5F5140, "count = WinRootCount;\n                    cur = WinZOrderWindow;",
     "count = WinRootCount;"),
])
def test_live_root_order_rejects_stale_callback_state(tmp_path, monkeypatch, address, good, bad):
    reader = harness_support.read_file

    def damaged(path):
        result = []
        for record in reader(path):
            if record.address == address:
                assert record.region.count(good) == 1
                record = replace(record, region=record.region.replace(good, bad))
            result.append(record)
        return result

    monkeypatch.setattr(harness_support, "read_file", damaged)
    with pytest.raises(AssertionError, match="Assertion failed"):
        test_live_root_order_uses_receiver_free_lift_and_mutable_registry(tmp_path, monkeypatch)


def test_listbox_close_uses_actual_default_targets_and_rejects_swapped_field(control_globals):
    compiler, settings, objects = control_globals
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned image is required")
    pin = image_pin(ROOT)
    image = PeImage(path.read_bytes(), pin)
    record, = (r for r in read_file(ROOT / "src/listbox.cpp") if r.address == 0x609F20)
    binding = FunctionBinding(Origin(pin, "pe:va", record.address),
                              tuple(Span(a, b) for a, b in record.image_spans),
                              record.symbol or record.name, "src/listbox.cpp")
    # Independently mapped callee identities. This verifies this caller's
    # targets, not the callees' behavior or a full ListBox lifecycle.
    callees = {symbol_at("src/graphicwin.cpp", 0x5D4E40): 0x5D4E40,
               symbol_at("src/dialog.cpp", 0x608F50): 0x608F50}

    def verify(obj):
        units = {"src/listbox.cpp": obj}
        targets, _ = object_targets(read_objects(ROOT, pin), units)
        return check_objects([binding], units, image, external_targets=targets | callees)[0]

    source = ROOT / "src/listbox.cpp"
    before = source.read_bytes()
    old = b"field_24_ = ListBoxDefault24;"
    assert before.count(old) == 2  # close and constructor
    for flags, obj in zip(settings.flag_sets, objects["src/listbox.cpp"]):
        result = verify(obj)
        assert result["boundary"] == result["bytes"] == result["references"] == "pass"
        damaged = Coff(compiler.overlay(source, {"src/listbox.cpp": before.replace(
            old, b"field_24_ = ListBoxDefault2C;", 1)}, (), flags + settings.layout_flags))
        bad = verify(damaged)
        assert bad["boundary"] == "pass"
        assert bad["bytes"] == bad["references"] == bad["result"] == "fail"
    assert source.read_bytes() == before
