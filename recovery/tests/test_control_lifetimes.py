"""Coupled member lifetime controls; not complete event dispatch/game proof."""

import shutil

import pytest

from match_recovery.codeview import class_layout
from match_recovery.compiler import Compiler
from match_recovery.measurement import Measurement
from match_recovery.pe_coff import Coff
from test_application_link_recovery import ROOT, references, run_recovered_harness, symbol_at


@pytest.fixture(scope="module")
def control_objects():
    from recovery.environment.wine import VC6_ROOT
    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine32") is None:
        pytest.skip("VC6 and Wine32 are required")
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    return {unit: [Coff(compiler(ROOT / unit, (), flags + settings.layout_flags))
                   for flags in settings.flag_sets]
            for unit in ("src/editbox.cpp", "src/listbox.cpp", "src/jackal/filewin.cpp", "src/flatbutton.cpp")}


def test_editbox_owns_real_base_timer_and_bounded_text(control_objects):
    for obj in control_objects["src/editbox.cpp"]:
        layout = class_layout(obj, "EditBox")
        assert layout["size"] == 0xB74
        assert [(b["class"], b["offset"], b["virtual"]) for b in layout["bases"]] == [
            ("GraphicWin", 0, False)]
        assert layout["members"]["text_"] == 0xA14
        assert layout["member_types"]["text_"]["count"] == 256
        assert layout["members"]["time_"] == 0xB4C
        assert layout["class_value_members"]["time_"] == "Time"
        ctor = references(obj, "??0EditBox@@QAE@XZ")
        assert ctor.count("??0GraphicWin@@QAE@XZ") == 1
        assert ctor.count("??0Time@@QAE@XZ") == 1
        dtor = references(obj, "??1EditBox@@UAE@XZ")
        assert [name for name in dtor if name in (
            "?close@EditBox@@QAEXXZ", "??1Time@@QAE@XZ", "??1GraphicWin@@UAE@XZ")] == [
            "?close@EditBox@@QAEXXZ", "??1Time@@QAE@XZ", "??1GraphicWin@@UAE@XZ"]


def test_listbox_constructs_virtual_bases_and_closes_derived_stage(control_objects):
    for obj in control_objects["src/listbox.cpp"]:
        layout = class_layout(obj, "ListBox")
        assert layout["size"] == 0xB54
        assert [(b["class"], b["virtual"]) for b in layout["bases"]] == [
            ("GraphicWin", True), ("Dialog", True)]
        ctor = references(obj, "??0ListBox@@QAE@XZ")
        assert ctor.count("??0GraphicWin@@QAE@XZ") == 1
        assert ctor.count("??0Dialog@@QAE@XZ") == 1
        assert references(obj, "??1ListBox@@UAE@XZ").count("?close@ListBox@@QAEIXZ") == 1
        assert not any(s.section > 0 and s.name == "?construct@ListBox@@QAEXH@Z"
                       for s in obj.symbols.values())


def test_filewin_preserves_member_layout_and_constructs_each_once(control_objects):
    for obj in control_objects["src/jackal/filewin.cpp"]:
        layout = class_layout(obj, "FileWin")
        assert layout["size"] == 0x33C4
        for name, offset in (("flat_button2_", 0x660), ("flat_button1_", 0x11AC),
                             ("edit_box_", 0x1CF8), ("list_box_", 0x286C)):
            assert layout["members"][name] == offset
            assert layout["member_type_kinds"][name] == "class_value"
        ctor = references(obj, "??0FileWin@@QAE@XZ")
        assert ctor.count("??0FlatButton@@QAE@XZ") == 2
        assert ctor.count("??0EditBox@@QAE@XZ") == 1
        assert ctor.count("??0ListBox@@QAE@XZ") == 1
        assert "?construct@ListBox@@QAEXH@Z" not in ctor
        dtor = references(obj, "??1FileWin@@QAE@XZ")
        assert dtor.count("??1ListBox@@UAE@XZ") == 1
        assert dtor.count("??1Dialog@@UAE@XZ") == 1
        assert dtor.count("??1EditBox@@UAE@XZ") == 1
        # The native binder owns the observed symbol after contract migration.
        # Do not fabricate an ABI spelling from the catalogue's old signature.
        assert dtor.count(symbol_at("src/flatbutton.cpp", 0x607DA0)) == 2
        assert not any(name in dtor for name in ("?destroy@ListBox@@QAEIXZ",
                                                "?destroy@BaseButton@@QAEXXZ"))


def test_member_order_unwind_and_editbox_single_byte_initialization(tmp_path, monkeypatch):
    run_recovered_harness(tmp_path, monkeypatch, "control_lifetime_harness.cpp", [
        ("// LIVE_FLATBUTTON_CTOR", "src/flatbutton.cpp", 0x607CF0),
        ("// LIVE_EDITBOX_CTOR", "src/editbox.cpp", 0x614E50),
        ("// LIVE_EDITBOX_DTOR", "src/editbox.cpp", 0x408010),
        ("// LIVE_FILEWIN_CTOR", "src/jackal/filewin.cpp", 0x613850),
        ("// LIVE_FILEWIN_DTOR", "src/jackal/filewin.cpp", 0x407F00)], declarations=[
        ("// LIVE_BASEBUTTON_CLOSE_DECL", "src/basebutton.h", "  virtual void close();", "  virtual void hide();"),
        ("// LIVE_EDITBOX_DECL", "src/editbox.h", "class EditBox :", "static_assert(sizeof(EditBox)"),
        ("// LIVE_FLATBUTTON_DECL", "src/flatbutton.h", "class FlatButton :", "static_assert(sizeof(FlatButton)"),
        ("// LIVE_FILEWIN_DECL", "src/jackal/filewin.h", "class FileWin {", "static_assert(sizeof(FileWin)")])


def test_flatbutton_constructs_base_once_and_keeps_indexed_runs(control_objects):
    for obj in control_objects["src/flatbutton.cpp"]:
        ctor = references(obj, "??0FlatButton@@QAE@XZ")
        assert ctor.count("??0BaseButton@@QAE@XZ") == 1
        assert "?construct@BaseButton@@QAEPAV1@XZ" not in ctor
        layout = class_layout(obj, "FlatButton")
        for name, offset in (("field_ABC_", 0xABC), ("border_styles_", 0xAC8), ("field_AD4_", 0xAD4)):
            assert layout["members"][name] == offset
            assert layout["member_types"][name]["count"] == 3
            assert layout["member_types"][name]["size"] == 12
