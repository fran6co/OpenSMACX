"""Pinned receiver-call shapes constrain AutoSound views, not ownership."""

from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


def test_sound_views_and_separate_win_notification_members_remain_explicit() -> None:
    auto = (ROOT / "src/autosound.h").read_text()
    win = (ROOT / "src/win.cpp").read_text()
    base = (ROOT / "src/basebutton.cpp").read_text()
    listing = (ROOT / "src/listbox.cpp").read_text()
    scroll = (ROOT / "src/scroll.cpp").read_text()
    menu = (ROOT / "src/menu.cpp").read_text()

    for field in (3, 4, 6, 7, 9, 13, 14, 15, 16, 17, 18, 19, 20, 21,
                  22, 23, 25, 26, 27, 28, 29, 30, 31, 33, 35, 36, 37):
        assert f"Sound *val_{field}_;" in auto
    assert "Sound *val_8_;" in auto
    for field in (10, 11, 12):
        assert f"Sound *val_{field}_;" in auto
    assert "Sound *val_31_;" in auto
    assert "Sound *legacy_sound_31() const { return val_31_; }" in auto
    assert "Sound *val_20_;" in auto
    assert "Sound *event_window_;" in auto
    assert "Sound *event_window() const" in auto

    # Sound 23/7 receiver calls are represented by actual Sound methods.
    assert "Sound *const owner = parent->legacy_sound_28();" in base
    assert "parent->legacy_sound_31()" in base
    assert "parent->legacy_sound_36()" in listing
    assert "legacy_sound_37()" in listing
    assert "legacy_sound_33()" in scroll
    assert "Sound *target = parent->event_window();" in menu
    assert "Sound *target = win_parent_->event_window();" in menu

    # Win's Sound words and its separately inherited notification state have
    # different owners. ListBox's scroll callbacks use Win parent/flags,
    # while the Win button paths use its AutoSound Sound fields.
    assert "win_parent_->on_listbox_scroll_create()" in listing
    assert "win_parent_->on_listbox_scroll_delete(scroll)" in listing
    assert "val_20_" not in listing and "val_31_" not in listing
    assert "(iFlags_ & 0x40)" in listing
    assert "Sound *child = val_20_;" in win
    assert "Sound *child = val_8_;" in win
    assert "reinterpret_cast<Sound *>(flags_)" not in win
    assert "Sound *child = val_10_;" in win
    assert "Sound *child = val_12_;" in win
    for field in (10, 11, 12):
        assert f"reinterpret_cast<Sound *>(val_{field}_)" not in win
