"""Actual draw body callback/state controls; doubles do not prove class ABI."""

from dataclasses import replace

import pytest

from test_application_link_recovery import run_recovered_harness
import test_application_link_recovery as harness_support


def test_live_control_draw_preserves_callback_mutation_and_geometry(tmp_path, monkeypatch):
    run_recovered_harness(tmp_path, monkeypatch, "control_draw_harness.cpp", [
        ("// LIVE_CHECKBOX", "src/checkbox.cpp", 0x60F030),
        ("// LIVE_RADIO", "src/radiobutton.cpp", 0x60D750)], declarations=[
        ("// LIVE_ACCESSOR", "src/stringstruct.h", "  char *current_text() {", "  // 0x006087A0")])


@pytest.mark.parametrize("address", [0x60F030, 0x60D750])
@pytest.mark.parametrize("old,new", [
    ("Dialog::field_64_, item_height);", "column_width, item_height);"),
    ("area.bottom - checked_sprite_->iHeight_", "area.bottom - checked_sprite_->iTopOffset_"),
    ("if (!Dialog::item_list_.seek_pos(position))", "if (false && !Dialog::item_list_.seek_pos(position))"),
    ("Dialog::field_F0_ == static_cast<uint32_t>(position)", "Win::is_dialog_focus() != 0"),
])
def test_live_controls_detect_stale_width_wrong_dimension_and_missing_seek(
        tmp_path, monkeypatch, address, old, new):
    reader = harness_support.read_file
    def damaged(path):
        result = []
        for record in reader(path):
            if record.address == address:
                assert record.region.count(old) == 1
                record = replace(record, region=record.region.replace(old, new))
            result.append(record)
        return result
    monkeypatch.setattr(harness_support, "read_file", damaged)
    with pytest.raises(AssertionError, match="Assertion failed"):
        test_live_control_draw_preserves_callback_mutation_and_geometry(tmp_path, monkeypatch)
