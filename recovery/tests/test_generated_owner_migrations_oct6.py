"""Source/history controls for genuine compiler-generated owner migrations.

These controls do not establish native body matching, linked selection or game acceptance.
"""
from functools import lru_cache
from pathlib import Path
import re
import subprocess

import pytest
from match_recovery.source import Mode, read_file, read_text
from match_recovery import binding_source

ROOT = Path(__file__).resolve().parents[2]
BASE = "8992788a853c19bca576c1a191021a8b00f300f9"
SPECS = (
    (0x004043A0, 'src/alphamovie.cpp', '??_GAlphaMovie@@UAEPAXI@Z'),
    (0x004068E0, 'src/flatbutton.cpp', '??_GFlatButton@@UAEPAXI@Z'),
    (0x00406B40, 'src/basepop.cpp', '??_GBasePop@@UAEPAXI@Z'),
    (0x00406BD0, 'src/popup.cpp', '??_GPopup@@UAEPAXI@Z'),
    (0x00408080, 'src/editbox.cpp', '??_GEditBox@@UAEPAXI@Z'),
    (0x00408140, 'src/alphasave.cpp', '??_GAlphaSave@@UAEPAXI@Z'),
    (0x004213A0, 'src/prodpicker.cpp', '??_GProdPicker@@UAEPAXI@Z'),
    (0x004213D0, 'src/basewin.cpp', '??_GBaseWin@@UAEPAXI@Z'),
    (0x00428710, 'src/councwin.cpp', '??_GCouncWin@@UAEPAXI@Z'),
    (0x00428E60, 'src/credits.cpp', '??_GCredits@@UAEPAXI@Z'),
    (0x00432770, 'src/stringbox.cpp', '??_GStringBox@@UAEPAXI@Z'),
    (0x00432800, 'src/datalink.cpp', '??_GDatalink@@UAEPAXI@Z'),
    (0x0043EDB0, 'src/designwin.cpp', '??_GDesignWin@@UAEPAXI@Z'),
    (0x00440EC0, 'src/diplopop.cpp', '??_GDiploPop@@UAEPAXI@Z'),
    (0x00445080, 'src/diplowin.cpp', '??_GDiploWin@@UAEPAXI@Z'),
    (0x00448350, 'src/fx.cpp', '??_GEffect@@UAEPAXI@Z'),
    (0x0044B300, 'src/famewin.cpp', '??_GFameWin@@UAEPAXI@Z'),
    (0x00456280, 'src/gamma.cpp', '??_GGamma@@UAEPAXI@Z'),
    (0x0045F1C0, 'src/maininterface.cpp', '??_GMainInterface@@UAEPAXI@Z'),
    (0x00477DD0, 'src/monuwin.cpp', '??_GMonuWin@@UAEPAXI@Z'),
    (0x0047A6C0, 'src/multiwin.cpp', '??_GMultiWin@@UAEPAXI@Z'),
    (0x0047AD70, 'src/netmsg.cpp', '??_GNetMsg@@UAEPAXI@Z'),
    (0x004837C0, 'src/netwin.cpp', '??_GNetWin@@UAEPAXI@Z'),
    (0x00484A80, 'src/newtechwin.cpp', '??_GNewTechWin@@UAEPAXI@Z'),
    (0x00488660, 'src/picktech.cpp', '??_GPickTech@@UAEPAXI@Z'),
    (0x0048ADC0, 'src/pickwin.cpp', '??_GPickWin@@UAEPAXI@Z'),
    (0x004923E0, 'src/prefwin.cpp', '??_GPrefWin@@UAEPAXI@Z'),
    (0x004968C0, 'src/quaylewin.cpp', '??_GQuayleWin@@UAEPAXI@Z'),
    (0x004AD840, 'src/reportwin.cpp', '??_GReportWin@@UAEPAXI@Z'),
    (0x004AE760, 'src/setupwin.cpp', '??_GSetupWin@@UAEPAXI@Z'),
    (0x004B3F20, 'src/checkbutton.cpp', '??_GCheckButton@@UAEPAXI@Z'),
    (0x004B3F50, 'src/socialwin.cpp', '??_GSocialWin@@UAEPAXI@Z'),
    (0x004BE9F0, 'src/tutwin.cpp', '??_GTutWin@@UAEPAXI@Z'),
    (0x004C9330, 'src/ambience.cpp', '??_GAmbience@@UAEPAXI@Z'),
    (0x005D7140, 'src/graphicwin.cpp', '??_GGraphicWin@@UAEPAXI@Z'),
    (0x005E3610, 'src/buffer.cpp', '??_GBuffer@@UAEPAXI@Z'),
    (0x005FA770, 'src/pulldown.cpp', '??_GPullDown@@UAEPAXI@Z'),
    (0x005FC6D0, 'src/menu.cpp', '??_GMenu@@UAEPAXI@Z'),
    (0x00607CC0, 'src/basebutton.cpp', '??_GBaseButton@@UAEPAXI@Z'),
    (0x006256D0, 'src/imagebutton.cpp', '??_GImageButton@@UAEPAXI@Z'),
    (0x0062C830, 'src/gamma.cpp', '??_GPushButton@@UAEPAXI@Z'),
    (0x0060CF50, 'src/listbox.cpp', '?on_redraw@ListBox@@$4PPPPPPPM@A@AEXXZ'),
    (0x0060CF70, 'src/listbox.cpp', '?on_mouse_move@ListBox@@$4PPPPPPPM@A@AEXHH@Z'),
    (0x0060CF90, 'src/listbox.cpp', '?on_left_down@ListBox@@$4PPPPPPPM@A@AEXHH@Z'),
    (0x0060CFA0, 'src/listbox.cpp', '?on_right_down@ListBox@@$4PPPPPPPM@A@AEXHH@Z'),
    (0x0060CFC0, 'src/listbox.cpp', '?on_left_double_click@ListBox@@$4PPPPPPPM@A@AEXHH@Z'),
    (0x0060D050, 'src/listbox.cpp', '?on_redraw@ListBox@@$4PPPPPPPM@KBI@AEXXZ'),
    (0x0060FE40, 'src/checkbox.cpp', '?on_redraw@CheckBox@@$4PPPPPPPM@A@AEXXZ'),
    (0x0060FE50, 'src/checkbox.cpp', '?on_mouse_move@CheckBox@@$4PPPPPPPM@A@AEXHH@Z'),
    (0x0060FE70, 'src/checkbox.cpp', '?on_left_down@CheckBox@@$4PPPPPPPM@A@AEXHH@Z'),
    (0x0060FE90, 'src/checkbox.cpp', '?on_left_double_click@CheckBox@@$4PPPPPPPM@A@AEXHH@Z'),
    (0x0060FEF0, 'src/checkbox.cpp', '?on_redraw@CheckBox@@$4PPPPPPPM@KBI@AEXXZ'),
    (0x00611820, 'src/spritebox.cpp', '?on_redraw@SpriteBox@@$4PPPPPPPM@A@AEXXZ'),
    (0x00611830, 'src/spritebox.cpp', '?on_mouse_move@SpriteBox@@$4PPPPPPPM@A@AEXHH@Z'),
    (0x00611860, 'src/spritebox.cpp', '?on_left_down@SpriteBox@@$4PPPPPPPM@A@AEXHH@Z'),
    (0x00611870, 'src/spritebox.cpp', '?on_left_up@SpriteBox@@$4PPPPPPPM@A@AEXHH@Z'),
    (0x00611890, 'src/spritebox.cpp', '?on_right_down@SpriteBox@@$4PPPPPPPM@A@AEXHH@Z'),
    (0x006118C0, 'src/spritebox.cpp', '?on_left_double_click@SpriteBox@@$4PPPPPPPM@A@AEXHH@Z'),
    (0x00611930, 'src/spritebox.cpp', '?on_redraw@SpriteBox@@$4PPPPPPPM@KBI@AEXXZ'),
)

@lru_cache(None)
def old_records():
    rows = []
    for unit in ("src/deleting_thunks.cpp", "src/adjustor_thunks.cpp"):
        raw = subprocess.check_output(["git", "show", BASE + ":" + unit], cwd=ROOT).decode()
        rows.extend(read_text(raw, ROOT / unit))
    return {r.address: r for r in rows}

@lru_cache(None)
def current_records(unit):
    return {r.address: r for r in read_file(ROOT / unit)}

@pytest.mark.parametrize("address,owner,symbol", SPECS)
def test_original_identity_history_and_generated_owner(address, owner, symbol):
    before = old_records()[address]
    after = current_records(owner)[address]
    assert after.mode is Mode.FILE and after.symbol == symbol
    for field in ("name", "image_spans", "byte_exact", "semantic", "levers",
                  "ruled_out", "unrecoverable", "deferred"):
        assert getattr(after, field) == getattr(before, field), field
    assert address not in current_records("src/deleting_thunks.cpp")
    assert address not in current_records("src/adjustor_thunks.cpp")

def test_unmigrated_original_rows_are_retained():
    moved = {a for a, _owner, _symbol in SPECS}
    current = {**current_records("src/deleting_thunks.cpp"),
               **current_records("src/adjustor_thunks.cpp")}
    assert set(current) == set(old_records()) - moved
    for address, after in current.items():
        before = old_records()[address]
        for field in ("name", "image_spans", "byte_exact", "semantic"):
            assert getattr(after, field) == getattr(before, field), field

def test_deleting_owner_rationale_uses_constructor_virtual_address_not_file_offset():
    for address, owner, symbol in SPECS:
        if not symbol.startswith("??_G"):
            continue
        rows = list(current_records(owner).values())
        rationale = binding_source.parse((ROOT / owner).read_text(), rows)[current_records(owner)[address].line]
        evidence = "\n".join(rationale["evidence"])
        witness = re.search(
            r"stores original class/view table VA (0x[0-9A-Fa-f]+) at "
            r"0x[0-9A-Fa-f]+ \(mov .*?, (0x[0-9A-Fa-f]+)\);", evidence)
        assert witness is not None, owner
        assert int(witness.group(1), 16) == int(witness.group(2), 16), owner

def test_unused_wrappers_are_removed_and_live_adjustor_dependencies_remain():
    source = (ROOT / "src/deleting_thunks.cpp").read_text()
    header = (ROOT / "src/deleting_thunks.h").read_text()
    removed = ('scalar_delete_alpha_movie', 'scalar_delete_ambience', 'scalar_delete_base_button', 'scalar_delete_base_pop', 'scalar_delete_base_win', 'scalar_delete_buffer', 'scalar_delete_check_button', 'scalar_delete_counc_win', 'scalar_delete_credits', 'scalar_delete_datalink', 'scalar_delete_design_win', 'scalar_delete_diplo_pop', 'scalar_delete_diplo_win', 'scalar_delete_effect', 'scalar_delete_flat_button', 'scalar_delete_gamma', 'scalar_delete_graphic_win', 'scalar_delete_image_button', 'scalar_delete_main_interface', 'scalar_delete_menu', 'scalar_delete_net_msg', 'scalar_delete_net_win', 'scalar_delete_new_tech_win', 'scalar_delete_pick_win', 'scalar_delete_popup', 'scalar_delete_prod_picker', 'scalar_delete_pull_down', 'scalar_delete_push_button', 'scalar_delete_report_win', 'scalar_delete_setup_win', 'scalar_delete_social_win', 'scalar_delete_string_box', 'scalar_delete_tut_win')
    retained = ('scalar_delete_alpha_save', 'scalar_delete_edit_box', 'scalar_delete_fame_win', 'scalar_delete_monu_win', 'scalar_delete_multi_win', 'scalar_delete_pick_tech', 'scalar_delete_pref_win', 'scalar_delete_quayle_win')
    # Comments preserve historical names; only executable definitions/prototypes count.
    source = re.sub(r"/\*.*?\*/|//[^\n]*", "", source, flags=re.S)
    header = re.sub(r"/\*.*?\*/|//[^\n]*", "", header, flags=re.S)
    for helper in removed:
        assert not re.search(r"\b" + re.escape(helper) + r"\s*\(", source)
        assert not re.search(r"\b" + re.escape(helper) + r"\s*\(", header)
    for helper in retained:
        assert len(re.findall(r"\b" + re.escape(helper) + r"\s*\(", source)) >= 2
        assert re.search(r"\b" + re.escape(helper) + r"\s*\(", header)
