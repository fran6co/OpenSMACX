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
