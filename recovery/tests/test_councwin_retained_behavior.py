"""Current-source callback test retained from the CouncWin worktree."""
import importlib.util
from pathlib import Path


FIXTURE = Path(__file__).with_name("councwin_owner_control_fixture.py")
spec = importlib.util.spec_from_file_location("retained_councwin_fixture", FIXTURE)
retained = importlib.util.module_from_spec(spec)
spec.loader.exec_module(retained)


import pytest


@pytest.mark.parametrize("control", [
    "test_on_redraw_uses_live_typed_owner_state_and_preserves_draw_order",
    "test_set_vote_rereads_lock_masks_after_ai_vote_callbacks",
    "test_on_key_click_fills_from_live_lock_state_and_closes_in_native_order",
    "test_draw_info_counts_votes_from_live_typed_faction_records",
    "test_make_resolution_retains_tally_only_and_resolution_side_effect_order",
    "test_init_builds_typed_council_controls_and_restores_shared_windows",
    "test_on_button_clicked_uses_typed_popup_vote_and_key_dispatch",
    "test_on_left_click_routes_hit_treaty_duplicate_and_vote_paths",
])
def test_retained_councwin_current_source_control(tmp_path, control):
    getattr(retained, control)(tmp_path)
