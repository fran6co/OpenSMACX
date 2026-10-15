"""Focused controls for Menu's signed selected-entry contract."""

from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


def test_menu_selection_member_and_actual_sentinel_are_signed():
    header = (ROOT / "src/menu.h").read_text(encoding="utf-8")
    source = (ROOT / "src/menu.cpp").read_text(encoding="utf-8")
    harness = (ROOT / "recovery/tests/menu-owner-harness.cpp").read_text(encoding="utf-8")

    assert "int field_A24_;  // selected entry index; -1 means no selected menu" in header
    assert "int field_A24_;" in harness
    assert "field_A24_(0xffffffffu)" not in harness
    assert "field_A24_ = 0xFFFFFFFF" not in source
    assert "field_A24_ = 0xFFFFFFFFu" not in source
    assert "static_cast<int>(field_A24_)" not in source
    assert "static_cast<unsigned int>(new_index)" not in source
    assert "static_cast<unsigned int>(index)" not in source

    for signature in (
        "void Menu::menu_event(int id)",
        "void Menu::UNK1()",
        "Menu::Menu()",
        "Menu::~Menu()",
        "void Menu::hide_menus()",
    ):
        start = source.index(signature)
        opening = source.index("{", start)
        assert "field_A24_ = -1;" in source[opening : source.index("\n}", opening)]

    assert "if (field_A24_ != -1)" in source
    assert "entries_[field_A24_]" in source


def test_menu_selection_layout_remains_four_bytes_at_native_offset():
    header = (ROOT / "src/menu.h").read_text(encoding="utf-8")
    assert "static_assert(sizeof(Menu) == 0xB64" in header
    assert header.index("int field_A24_;") < header.index("uint32_t field_A28_;")
