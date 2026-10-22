"""Boundary checks for the one source-owned random_events body.

These assertions verify source/mapping continuity and branch selection shape;
the neighboring fixture suite executes extracted bodies with host shims and is
not proof of VC6 ABI, the complete game, or runtime equivalence.
"""
from pathlib import Path
import re

from match_recovery.source import Mode, read_file
from test_random_events_behavior import _mapped_case_block, function, masked

ROOT = Path(__file__).resolve().parents[2]


def test_all_twenty_two_cases_are_inside_the_unique_mapped_owner():
    source = (ROOT / "src/game.cpp").read_text(encoding="utf-8")
    rows = [row for row in read_file(ROOT / "src/game.cpp")
            if row.address == 0x51F2E0]
    assert len(rows) == 1
    assert rows[0].mode is Mode.BODY
    assert "void __cdecl random_events(int mode)" in rows[0].region

    owner = function(source, "random_events")
    code = masked(owner)
    labels = [int(value) for value in re.findall(r"\bcase\s+(\d+)\s*:", code)]
    # Cases 0..6 occur a second time in the nested blight-cache switch inside
    # the outer 15/16 branch; retain that distinction rather than deduping.
    assert labels == [0, 1, 2, 3, 4, 5, 9, 10, 11, 12, 13,
                      15, 16, 0, 1, 2, 3, 4, 5, 6, 17, 18, 19, 20, 21]
    assert "RandomEventsDispatchInput" not in owner
    assert "random_events_dispatch_cases" not in owner
    assert "random_events_entry_prepare" not in owner


def test_each_extracted_outer_branch_is_the_mapped_owner_branch():
    source = (ROOT / "src/game.cpp").read_text(encoding="utf-8")
    owner = function(source, "random_events")
    _, _, fragments = __import__("test_random_events_behavior").source_fragments()
    conditions = {
        "actual_cases_0_5.inc": "event_kind <= 5",
        "actual_case6.hpp": "event_kind == 6",
        "actual_cases_7_8.hpp": "event_kind == 7 || event_kind == 8",
        "actual_cases_9_11.hpp": "event_kind >= 9 && event_kind <= 11",
        "actual_cases_12_13.inc": "event_kind == 12 || event_kind == 13",
        "actual_case14.inc": "event_kind == 14",
        "actual_cases_15_16.inc": "event_kind == 15 || event_kind == 16",
        "actual_cases_17_18.inc": "event_kind == 17 || event_kind == 18",
        "actual_cases_19_21.inc": "event_kind >= 19 && event_kind <= 21",
    }
    for filename, condition in conditions.items():
        assert _mapped_case_block(owner, condition) in fragments[filename]


def test_entry_snapshots_precede_callbacks_and_preserve_real_order():
    _, _, fragments = __import__("test_random_events_behavior").source_fragments()
    prefix = fragments["actual_entry_prefix.inc"]
    assert prefix.index("const int base_x_snapshot = selected_base.x;") < prefix.index("parse_num(0, 10)")
    assert prefix.index("const int base_y_snapshot = selected_base.y;") < prefix.index("parse_says(0, selected_base.name_string")
    assert prefix.index("parse_num(0, 10)") < prefix.index("parse_says(0, selected_base.name_string")
    assert prefix.index("StringTemp[0] = '\\0';") < prefix.index("event_kind) > 0x15u")
