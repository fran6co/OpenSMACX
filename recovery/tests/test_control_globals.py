"""Live control-global and keyboard regressions; not full UI/game acceptance."""

from dataclasses import replace
import os
from pathlib import Path
import shutil

import pytest
from capstone import CS_AC_READ, CS_AC_WRITE

from match_recovery.compiler import Compiler
from match_recovery.import_source import import_targets, read_imports
from match_recovery.measurement import Measurement
from match_recovery.pe_coff import Coff, PeImage
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
               for unit in ("src/win.cpp", "src/editbox.cpp")}
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
