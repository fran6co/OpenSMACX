"""Pinned receiver-call shapes constrain AutoSound views, not ownership."""

from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


def test_sound_interface_views_and_context_mixed_words_remain_explicit() -> None:
    auto = (ROOT / "src/autosound.h").read_text()
    win = (ROOT / "src/win.cpp").read_text()
    base = (ROOT / "src/basebutton.cpp").read_text()
    listing = (ROOT / "src/listbox.cpp").read_text()
    scroll = (ROOT / "src/scroll.cpp").read_text()
    menu = (ROOT / "src/menu.cpp").read_text()

    for field in (3, 4, 6, 7, 9, 13, 14, 15, 16, 17, 18, 19, 21,
                  22, 23, 25, 26, 27, 28, 29, 30, 33, 35, 36, 37):
        assert f"Sound *val_{field}_;" in auto
    assert "Sound *val_8_;" in auto
    assert "int val_10_;" in auto and "int val_11_;" in auto
    assert "void *val_12_;" in auto
    assert "void *val_31_;" in auto
    assert "int val_20_;" in auto
    assert "Sound *event_window_;" in auto
    assert "Sound *event_window() const" in auto

    # Sound 23/7 receiver calls are represented by actual Sound methods.
    assert "Sound *const owner = parent->legacy_sound_28();" in base
    assert "parent->legacy_value_31()" in base
    assert "parent->legacy_sound_36()" in listing
    assert "legacy_sound_37()" in listing
    assert "legacy_sound_33()" in scroll
    assert "Sound *target = parent->event_window();" in menu
    assert "Sound *target = win_parent_->event_window();" in menu

    # The same numbered storage retains contextual views where original call
    # shapes differ; these are intentionally not global Win/Sound renames.
    assert "reinterpret_cast<Win *>(val_31_)->on_listbox_scroll_create()" in listing
    assert "reinterpret_cast<Win *>(val_31_)->on_listbox_scroll_delete(scroll)" in listing
    assert "Sound *child = reinterpret_cast<Sound *>(val_20_);" in win
    assert "Sound *child = val_8_;" in win
    assert "reinterpret_cast<Sound *>(flags_)" not in win
    assert "Sound *child = reinterpret_cast<Sound *>(val_10_);" in win
    assert "reinterpret_cast<Sound *>(val_12_)" in win
