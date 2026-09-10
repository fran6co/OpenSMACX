"""EditBox slot ownership and live input branches, not full UI acceptance."""

from test_application_link_recovery import run_recovered_harness
from test_control_lifetimes import control_objects
from match_recovery.pe_coff import weak_external


def table_targets(obj, name):
    symbol, = (s for s in obj.symbols.values() if s.name == name and s.section > 0)
    section = obj.sections[symbol.section - 1]
    return section, {offset // 4: obj.symbols[index].name
                     for offset, index, _ in section.relocations}


def test_real_editbox_overrides_occupy_existing_primary_slots(control_objects):
    expected = {0: "??_EEditBox@@UAEPAXI@Z", 12: "?vslot_12@EditBox@@UAEXXZ",
                17: "?vslot_17@EditBox@@UAEXHH@Z", 20: "?vslot_20@EditBox@@UAEXHH@Z",
                21: "?vslot_21@EditBox@@UAEXHH@Z", 25: "?vslot_25@EditBox@@UAEHHH@Z",
                26: "?vslot_26@EditBox@@UAEHH@Z", 51: "?vslot_51@EditBox@@UAEXH@Z",
                64: "?pass_dialog_focus@EditBox@@UAEXXZ"}
    for obj in control_objects["src/editbox.cpp"]:
        section, slots = table_targets(obj, "??_7EditBox@@6BWin@@@")
        assert len(section.data) == 90 * 4
        assert {i: slots[i] for i in expected} == expected
        assert {i for i, name in slots.items() if "EditBox@@" in name} == set(expected)
        # The mouse router is a different overload; drawing still enters the
        # inherited virtual redraw before its slot-12 callback.
        assert slots[16] == "?on_mouse_move@GraphicWin@@UAEXHHIH@Z"
        assert slots[62] == "?redraw@GraphicWin@@UAEXXZ"
        secondary, secondary_slots = table_targets(obj, "??_7EditBox@@6BBuffer@@@")
        assert len(secondary.data) == 8
        assert secondary_slots[0] == "??_EEditBox@@WEEE@AEPAXI@Z"
        assert secondary_slots[1] == "?get_graphic_win@GraphicWin@@WEEE@AEPAV1@XZ"
        entry, = (s for s in obj.symbols.values() if s.name == slots[0])
        fallback, mode = weak_external(obj, entry)
        assert fallback.name == "??_GEditBox@@UAEPAXI@Z"
        # SEARCH_LIBRARY permits a linked vector entry to override this
        # fallback; this object observation is deliberately not link proof.
        assert mode == 2


def test_text_input_selection_mouse_focus_and_rendering(tmp_path, monkeypatch):
    run_recovered_harness(tmp_path, monkeypatch, "editbox_input_harness.cpp", [
        ("// LIVE_FOCUS", "src/editbox.cpp", 0x615DF0),
        ("// LIVE_DOWN", "src/editbox.cpp", 0x615E60),
        ("// LIVE_UP", "src/editbox.cpp", 0x615EC0),
        ("// LIVE_MOVE", "src/editbox.cpp", 0x615F10),
        ("// LIVE_PASS_FOCUS", "src/editbox.cpp", 0x616140),
        ("// LIVE_POSITION", "src/editbox.cpp", 0x616030),
        ("// LIVE_FITS", "src/editbox.cpp", 0x615F50),
        ("// LIVE_CHARACTER", "src/editbox.cpp", 0x615630),
        ("// LIVE_KEY", "src/editbox.cpp", 0x615840),
        ("// LIVE_PAINT", "src/editbox.cpp", 0x615270)], declarations=[
        ("// LIVE_EDITBOX_DECL", "src/editbox.h", "class EditBox :", "static_assert(sizeof(EditBox)")])
