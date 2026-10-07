"""Source/history controls for genuine compiler-generated owner migrations.

These controls do not establish native body matching, linked selection or game acceptance.
"""
from functools import lru_cache
from pathlib import Path
import re
import subprocess

import pytest
from test_virtual_adjustor_owners_oct6 import SPECS as FOLLOW_ON_VIRTUAL_SPECS
from test_mapwin_adjustor_owner_history_oct6 import MAPWIN_MIGRATED_ADDRESSES
from test_virtual_adjustor_owners_oct6 import FOLLOW_ON_MIGRATED_ADDRESSES
from match_recovery.source import Mode, Recipe, read_file, read_text
from match_recovery import binding_source
from test_remaining_adjustor_actual_source_oct7 import assert_archived_history

ROOT = Path(__file__).resolve().parents[2]
BASE = "8992788a853c19bca576c1a191021a8b00f300f9"
SPECS = (
    (0x00408390, 'src/alphasave.cpp', '??_EAlphaSave@@WEEE@AEPAXI@Z'),
    (0x004083A0, 'src/editbox.cpp', '??_EEditBox@@WEEE@AEPAXI@Z'),
    (0x0044B330, 'src/famewin.cpp', '??_EFameWin@@WEEE@AEPAXI@Z'),
    (0x00477E00, 'src/monuwin.cpp', '??_EMonuWin@@WEEE@AEPAXI@Z'),
    (0x0047A760, 'src/multiwin.cpp', '??_EMultiWin@@WEEE@AEPAXI@Z'),
    (0x00488740, 'src/picktech.cpp', '??_EPickTech@@WEEE@AEPAXI@Z'),
    (0x00492410, 'src/prefwin.cpp', '??_EPrefWin@@WEEE@AEPAXI@Z'),
    (0x004968F0, 'src/quaylewin.cpp', '??_EQuayleWin@@WEEE@AEPAXI@Z'),
    (0x004070B0, 'src/basepop.cpp', '??_EBasePop@@WEEE@AEPAXI@Z'),
    (0x00428F80, 'src/credits.cpp', '??_ECredits@@WEEE@AEPAXI@Z'),
    (0x004AE9A0, 'src/setupwin.cpp', '??_ESetupWin@@WEEE@AEPAXI@Z'),
    (0x005AE0E0, 'src/replaywin.cpp', '??_EReplayWin@@WEEE@AEPAXI@Z'),
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
    (0x00448390, 'src/ambience.cpp', '??_GAmbience@@UAEPAXI@Z'),
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
    (0x005F8610, 'src/win.cpp', '??_GWin@@UAEPAXI@Z'),
    (0x005AE0B0, 'src/replaywin.cpp', '??_GReplayWin@@UAEPAXI@Z'),
    (0x0051DC90, 'src/alphamenu.cpp', '??_GAlphaMenu@@UAEPAXI@Z'),
    (0x0051DF70, 'src/alphamenu.cpp', '??_EAlphaMenu@@WEEE@AEPAXI@Z'),
    (0x00421640, 'src/popmenu.cpp', '??_GPopMenu@@UAEPAXI@Z'),
    (0x00421980, 'src/popmenu.cpp', '??_EPopMenu@@WEEE@AEPAXI@Z'),
    (0x00421990, 'src/popmenu.cpp', '??_EPopMenu@@WFDHM@AEPAXI@Z'),
    (0x004219A0, 'src/popmenu.cpp', '??_EPopMenu@@WFHMA@AEPAXI@Z'),
    (0x0048BF10, 'src/planwin.cpp', '??_EPlanWin@@$4PPPPPPPM@A@AEPAXI@Z'),
)

# Transfers from mainline commits newer than BASE. Keep each identity and
# positive FILE owner explicit so the exact residual-set assertion cannot
# silently excuse vanished history.
PRIOR_MAINLINE_MIGRATIONS = {
    0x00406FF0: "src/radiobutton.cpp", 0x00407000: "src/radiobutton.cpp",
    0x00407020: "src/radiobutton.cpp", 0x00407030: "src/radiobutton.cpp",
    0x00407040: "src/radiobutton.cpp", 0x004070A0: "src/radiobutton.cpp",
    0x00407150: "src/dialogs.cpp", 0x00407160: "src/dialogs.cpp",
    0x00407170: "src/dialogs.cpp", 0x00407180: "src/dialogs.cpp",
    0x00407190: "src/dialogs.cpp", 0x004071A0: "src/dialogs.cpp",
    0x004071B0: "src/dialogs.cpp", 0x004071C0: "src/dialogs.cpp",
    0x004071D0: "src/dialogs.cpp", 0x004071E0: "src/dialogs.cpp",
    0x004071F0: "src/dialogs.cpp", 0x00407200: "src/dialogs.cpp",
    0x00407210: "src/dialogs.cpp", 0x00407220: "src/dialogs.cpp",
    0x00407230: "src/dialogs.cpp", 0x00407240: "src/dialogs.cpp",
    0x00407250: "src/dialogs.cpp", 0x00407260: "src/dialogs.cpp",
    0x00407270: "src/dialogs.cpp", 0x004072B0: "src/dialogs.cpp",
    0x004072C0: "src/dialogs.cpp", 0x00421820: "src/mapwin.cpp",
    0x00421920: "src/mapwin.cpp", 0x00421950: "src/mapwin.cpp",
    0x0048C010: "src/planwin.cpp", 0x0048C040: "src/planwin.cpp",
    0x0051DDE0: "src/console.cpp", 0x0051DF00: "src/console.cpp",
    0x0051DF50: "src/console.cpp", 0x0060CFD0: "src/listbox.cpp",
    0x0051DEF0: "src/console.cpp", 0x0051DF10: "src/console.cpp",
    0x0051DF30: "src/console.cpp",
    0x0060CFE0: "src/listbox.cpp", 0x0060CFF0: "src/listbox.cpp",
    0x006118A0: "src/spritebox.cpp", 0x006118D0: "src/spritebox.cpp",
    0x004070F0: "src/dialogs.cpp", 0x00407280: "src/dialogs.cpp",
    0x004072A0: "src/dialogs.cpp",
}
HISTORICAL_FIELDS = (
    "name", "image_spans", "byte_exact", "semantic", "levers", "ruled_out",
    "unrecoverable", "deferred", "kind", "exclusion", "origin",
)

@lru_cache(None)
def old_records():
    rows = []
    for unit in ("src/deleting_thunks.cpp", "src/adjustor_thunks.cpp", "src/pending_bodies.cpp"):
        raw = subprocess.check_output(["git", "show", BASE + ":" + unit], cwd=ROOT).decode()
        rows.extend(read_text(raw, ROOT / unit))
    return {r.address: r for r in rows}

@lru_cache(None)
def current_records(unit):
    path = ROOT / unit
    return {r.address: r for r in read_file(path)} if path.is_file() else {}

@lru_cache(None)
def all_current_source_records():
    rows = {}
    for path in (ROOT / "src").rglob("*.cpp"):
        for row in read_file(path):
            rows.setdefault(row.address, []).append((path.relative_to(ROOT).as_posix(), row))
    return rows

@pytest.mark.parametrize("address,owner,symbol", SPECS)
def test_original_identity_history_and_generated_owner(address, owner, symbol):
    before = old_records()[address]
    after = current_records(owner)[address]
    assert after.mode is Mode.FILE and after.symbol == symbol
    for field in ("name", "image_spans", "byte_exact", "semantic", "levers",
                  "ruled_out", "unrecoverable", "deferred"):
        assert getattr(after, field) == getattr(before, field), field
    assert not (ROOT / "src/adjustor_thunks.cpp").exists()
    assert not (ROOT / "src/adjustor_thunks.h").exists()

def test_unmigrated_original_rows_are_retained_by_real_owners():
    assert not (ROOT / "src/deleting_thunks.cpp").exists()
    assert not (ROOT / "src/deleting_thunks.h").exists()
    assert not (ROOT / "src/adjustor_thunks.cpp").exists()
    assert not (ROOT / "src/adjustor_thunks.h").exists()
    moved = ({a for a, _owner, _symbol in SPECS + FOLLOW_ON_VIRTUAL_SPECS}
             | MAPWIN_MIGRATED_ADDRESSES | FOLLOW_ON_MIGRATED_ADDRESSES
             | set(PRIOR_MAINLINE_MIGRATIONS))
    current = all_current_source_records()
    for address in set(old_records()) - moved:
        hits = current.get(address, [])
        assert len(hits) == 1, f"{address:08X} lost or duplicated after owner transfer"
        _path, after = hits[0]
        before = old_records()[address]
        for field in HISTORICAL_FIELDS:
            assert getattr(after, field) == getattr(before, field), (hex(address), field)

@pytest.mark.parametrize("address,owner", sorted(PRIOR_MAINLINE_MIGRATIONS.items()))
def test_prior_mainline_migrations_have_positive_file_owners_and_full_history(address, owner):
    hits = all_current_source_records().get(address, [])
    assert len(hits) == 1, f"{address:08X} must have exactly one current source owner"
    actual_owner, after = hits[0]
    assert actual_owner == owner
    assert after.mode is Mode.FILE
    before = old_records()[address]
    # The reader derives the recipe from source placement: FILE resolves the
    # compiler-generated owner definition, not the retired BODY census adapter.
    assert before.recipe is Recipe.CENSUS
    assert after.recipe is Recipe.VERBATIM
    for field in HISTORICAL_FIELDS:
        assert getattr(after, field) == getattr(before, field), field

@pytest.mark.parametrize("address,owner,symbol", SPECS)
def test_owner_transfers_preserve_catalogue_kind(address, owner, symbol):
    assert current_records(owner)[address].kind == old_records()[address].kind

def test_deleting_owner_rationale_uses_constructor_virtual_address_not_file_offset():
    for address, owner, symbol in SPECS:
        if not symbol.startswith("??_G"):
            continue
        rows = list(current_records(owner).values())
        rationale = binding_source.parse((ROOT / owner).read_text(), rows)[current_records(owner)[address].line]
        evidence = "\n".join(rationale["evidence"])
        if owner == 'src/ambience.cpp':
            # Require the final derived table, not the intermediate Midi view.
            assert re.search(
                r"final (?:Ambience )?table 0x66E664 at 0x004C850F", evidence), owner
            assert re.search(
                r"(?:intermediate base table 0x66E538 at 0x004C84EF|"
                r"0x004C84EF stores intermediate base table 0x66E538)", evidence), owner
            assert re.search(
                r"slot 2 at 0x66E66C selects (?:the original deleting entry )?0x00448390",
                evidence), owner
            continue
        if address in (0x005F8610, 0x005AE0B0, 0x0051DC90, 0x0051DF70,
                       0x00421640):
            # These class-specific table/constructor proofs are asserted
            # phrase-by-phrase in the dedicated control below.
            continue
        witness = re.search(
            r"stores original class/view table VA (0x[0-9A-Fa-f]+) at "
            r"0x[0-9A-Fa-f]+ \(mov .*?, (0x[0-9A-Fa-f]+)\);", evidence)
        assert witness is not None, owner
        assert int(witness.group(1), 16) == int(witness.group(2), 16), owner



def test_win_replay_alpha_have_explicit_pinned_view_proofs():
    expected = {
        0x005F8610: (
            "pinned Win constructor 0x005EB402 installs primary table 0x0066FDD0",
            "slot 0 selects 0x005F8610"),
        0x005AE0B0: (
            "pinned ReplayWin primary table 0x0066F8F0 has slot 0 = 0x005AE0B0",),
        0x0051DC90: (
            "pinned Console constructor 0x0050F52F stores primary AlphaMenu table 0x0066ED88",
            "pinned table slot 0 is 0x0051DC90"),
        0x0051DF70: (
            "pinned Console constructor 0x0050F535 stores AlphaMenu Buffer-view table 0x0066ED80",
            "at alpha_menu_+0x444; table slot 0 is exactly 0x0051DF70",
            "mode-2 fallback is exactly ??_GAlphaMenu@@UAEPAXI@Z"),
    }
    for address, phrases in expected.items():
        owner = next(owner for item, owner, _symbol in SPECS if item == address)
        row = current_records(owner)[address]
        rationale = binding_source.parse((ROOT / owner).read_text(),
                                         list(current_records(owner).values()))[row.line]
        evidence = "\n".join(rationale["evidence"])
        for phrase in phrases:
            assert phrase in evidence, (hex(address), phrase)
    secondary = current_records("src/alphamenu.cpp")[0x0051DF70]
    assert secondary.symbol == "??_EAlphaMenu@@WEEE@AEPAXI@Z"
    assert secondary.kind == "game"


def test_popmenu_mi_views_and_remaining_adjustors_are_not_waived():
    header = (ROOT / "src/popmenu.h").read_text()
    body = re.search(r"class PopMenu\b.*?^};", header, re.S | re.M)
    assert body is not None
    assert "class PopMenu : public Popup, public PullDown" in body.group()
    assert "PullDown pull_down_" not in body.group()
    assert not re.search(r"\bprivate\s*:", body.group())
    assert "pull_down_." not in (ROOT / "src/popmenu.cpp").read_text()
    assert "BaseWin::queue_click's constructor sequence" in header
    normalized_header = re.sub(r"\s+", " ",
                              re.sub(r"(?m)^\s*\*\s?", "", header))
    for proof in ("LEA at 0x00409228 begins Popup setup", "actual constructor CALL at 0x0040922E",
                  "LEA at 0x00409233 begins PullDown setup", "CALL 0x00409240 constructs PullDown",
                  "0x00409245", "0x0040924F", "0x00409259", "0x00409263", "0x0040926D",
                  "each store is 10 bytes", "0x00421640", "0x00421980",
                  "0x00421990", "0x004219A0"):
        assert proof in normalized_header
    popmenu = current_records("src/popmenu.cpp")
    assert {0x00421640, 0x00421980, 0x00421990, 0x004219A0} <= set(popmenu)
    for address, owner, symbol in SPECS[-5:-1]:
        assert owner == "src/popmenu.cpp" and popmenu[address].symbol == symbol


def test_popmenu_generated_views_keep_pinned_and_current_table_binding_evidence():
    expected = {
        0x00421640: ("??_7PopMenu@@6BWin@@Popup@@@", "tests [ebp+8]&1", "ret 4"),
        0x00421980: ("??_7PopMenu@@6BBuffer@@Popup@@@", "subtracts 0x444", "tail-jumps"),
        0x00421990: ("??_7PopMenu@@6BWin@@PullDown@@@", "subtracts 0x537C", "tail-jumps"),
        0x004219A0: ("??_7PopMenu@@6BBuffer@@PullDown@@@", "subtracts 0x57C0", "tail-jumps"),
    }
    all_rows = list(current_records("src/popmenu.cpp").values())
    source = (ROOT / "src/popmenu.cpp").read_text()
    for address, (table, operation, return_fact) in expected.items():
        row = current_records("src/popmenu.cpp")[address]
        evidence = "\n".join(binding_source.parse(source, all_rows)[row.line]["evidence"])
        assert table in evidence and operation in evidence and return_fact in evidence
        assert "Pinned source-mapped BaseWin::queue_click ?queue_click@BaseWin@@QAEXHHHH@Z at 0x00409200" in evidence
        assert "actual Popup constructor CALL is at 0x0040922E" in evidence
        assert "actual PullDown constructor CALL at 0x00409240" in evidence
        assert "vptrs at 0x00409245, 0x0040924F, 0x00409259 and 0x00409263" in evidence
        assert "fourth ends at 0x0040926D" in evidence
        assert "mode 2" in evidence


def test_deleting_adapter_translation_unit_is_retired_and_only_dialogs_target_remains():
    assert not (ROOT / "src/deleting_thunks.cpp").exists()
    assert not (ROOT / "src/deleting_thunks.h").exists()
    for path in (ROOT / "src").rglob("*"):
        if path.suffix not in (".cpp", ".h"):
            continue
        text = path.read_text()
        assert '#include "deleting_thunks.h"' not in text, path
        assert "PopupDtorTarget" not in text, path
        assert "func_deleting_dtor" not in text, path

    assert not (ROOT / "src/adjustor_thunks.h").exists()
    assert not (ROOT / "src/adjustor_thunks.cpp").exists()
    obsolete = ("ConsoleScalarDeleteTarget", "MapWinScalarDeleteTarget",
                "PlanWinScalarDeleteTarget", "DialogsScalarDeleteTarget")
    for path in (ROOT / "src").rglob("*"):
        if path.suffix not in (".cpp", ".h"):
            continue
        active = re.sub(r"/\*\n \* RECOVERY_HISTORY: v1 .*?\n \*/", "",
                        path.read_text(errors="surrogateescape"), flags=re.S)
        for symbol in obsolete:
            assert symbol not in active, (path, symbol)
    for address in (0x004070F0, 0x00407280, 0x004072A0):
        row = current_records("src/dialogs.cpp")[address]
        assert row.mode is Mode.FILE and row.symbol
        assert address not in current_records("src/adjustor_thunks.cpp")
        assert address in (0x004070F0, 0x00407280, 0x004072A0)

    planwin_row = current_records("src/planwin.cpp")[0x0048C040]
    assert planwin_row.mode is Mode.FILE
    assert planwin_row.symbol == "??_EPlanWin@@$4PPPPPLLI@EEE@AEPAXI@Z"
    before = old_records()[0x0048C040]
    for field in HISTORICAL_FIELDS:
        assert getattr(planwin_row, field) == getattr(before, field), field
    assert_archived_history(ROOT / "src/planwin.cpp", 0x0048C040)


def test_planwin_vtordisp_deleting_identity_and_live_target_dependency():
    row = current_records("src/planwin.cpp")[0x0048BF10]
    assert row.mode is Mode.FILE
    assert row.symbol == "??_EPlanWin@@$4PPPPPPPM@A@AEPAXI@Z"
    rows = list(current_records("src/planwin.cpp").values())
    binding = binding_source.parse((ROOT / "src/planwin.cpp").read_text(), rows)[row.line]
    evidence = " ".join(binding["evidence"])
    for phrase in (
        "PlanWin constructor 0x0048BCD0 installs vbtable 0x0066D414",
        "builds virtual GraphicWin at +0x22050",
        "stores its vtordisp immediately before that subobject at 0x0048BD5D",
        "0x0048BF10 executes sub ecx,[ecx-4] and tail-jumps to 0x0048BF20",
    ):
        assert phrase in evidence, phrase
    assert "compiler-generated PlanWin Win-view thunk now owns that adjustment" in binding["reason"]
