"""Source identity/lifetime attachment controls, not native EH matching proof."""
from pathlib import Path

import pytest

from match_recovery.source import Mode, read, read_text


ROOT = Path(__file__).resolve().parents[2]


def editor_record(text):
    rows = [row for row in read_text(text, ROOT / "src/console.cpp")
            if row.address == 0x004DFAD0]
    assert len(rows) == 1
    return rows[0]


def assert_complete_editor_region(text):
    record = editor_record(text)
    assert record.mode is Mode.BODY
    assert record.image_spans == ((0x004DFAD0, 0x004E0118),
                                  (0x0065C0BE, 0x0065C0C8))
    signature = "void Console::editor_tech()"
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for index in range(opening, len(text)):
        depth += (text[index] == "{") - (text[index] == "}")
        if depth == 0:
            definition = text[start:index + 1]
            break
    else:
        raise AssertionError("unterminated editor_tech definition")
    assert definition in record.region
    assert "Popup popup;" in definition and "Popup who;" in definition
    assert "TutWinBaseWindow.redraw();" in definition
    return record


def test_cleanup_identities_belong_to_real_owner_without_truncating_parent():
    text = (ROOT / "src/console.cpp").read_text()
    parent = assert_complete_editor_region(text)
    rows = list(read(ROOT / "src"))
    for address, end, name in ((0x0065BFBC, 0x0065BFC7, "sub_65bfbc"),
                                (0x0065C001, 0x0065C009, "sub_65c001")):
        selected = [row for row in rows if row.address == address]
        assert len(selected) == 1
        child = selected[0]
        assert child.path == ROOT / "src/console.cpp"
        assert child.line < parent.line
        assert child.image_spans == ((address, end),)
        assert child.name == name and child.kind == "game"
        assert child.mode is Mode.FILE and child.symbol == ""
        assert not child.byte_exact and not child.semantic
        assert not (ROOT / f"src/recovered/units/{address:08x}.cpp").exists()
        assert f'void {name}(' not in text


def test_nested_mapping_damage_cannot_hide_parent_body_truncation():
    text = (ROOT / "src/console.cpp").read_text()
    assert_complete_editor_region(text)
    # Move an existing marker into the nested scope. This is disposable damage,
    # not a production writer or a new source identity/claim.
    marker = next(line for line in text.splitlines()
                  if line.startswith("// ORIGINAL: 0x0065BFBC "))
    damaged = text.replace(marker + "\n", "", 1).replace(
        "            Popup who;", marker + "\n            Popup who;", 1)
    with pytest.raises(AssertionError):
        assert_complete_editor_region(damaged)


def test_cleanup_member_path_has_real_automatic_cpp_owners():
    assert "class Popup : public BasePop" in (ROOT / "src/popup.h").read_text()
    assert "Scroll scroll_;" in (ROOT / "src/popup.h").read_text()
    assert "FlatButton flat_button_left_;" in (ROOT / "src/scroll.h").read_text()
    assert "class FlatButton : public BaseButton" in (ROOT / "src/flatbutton.h").read_text()


def test_all_original_unwind_actions_keep_spans_in_parent_lifetime_owner():
    text = (ROOT / "src/console.cpp").read_text()
    parent = assert_complete_editor_region(text)
    # Independently bounded contiguous actions preceding the parent's handler.
    boundaries = (
        0x65BFB1, 0x65BFBC, 0x65BFC7, 0x65BFD2, 0x65BFDD, 0x65BFE5,
        0x65BFF3, 0x65C001, 0x65C009, 0x65C011, 0x65C01C, 0x65C027,
        0x65C032, 0x65C03D, 0x65C048, 0x65C053, 0x65C05E, 0x65C069,
        0x65C074, 0x65C07F, 0x65C08A, 0x65C092, 0x65C0A0, 0x65C0AE,
        0x65C0B6, 0x65C0BE,
    )
    records = list(read(ROOT / "src"))
    for start, end in zip(boundaries, boundaries[1:]):
        action, = [row for row in records if row.address == start]
        assert action.path == ROOT / "src/console.cpp" and action.line < parent.line
        assert action.name == f"sub_{start:x}"
        assert action.image_spans == ((start, end),)
        assert action.mode is Mode.FILE and action.kind == "game"
        assert not action.symbol and not action.byte_exact and not action.semantic
        assert not (ROOT / f"src/unrecovered/{start:08x}.cpp").exists()
        assert f"void {action.name}(" not in text
    # This is source/lifetime navigation, not one-to-one current EH matching.
    assert parent.image_spans[-1] == (boundaries[-1], 0x65C0C8)
