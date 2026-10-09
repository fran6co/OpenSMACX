"""Source identity and callback-order checks for Console AI priority editors."""
from __future__ import annotations

import re
from pathlib import Path

from match_recovery.includes import COMMENTS_OR_STRINGS

ROOT = next(path for path in Path(__file__).resolve().parents
            if (path / "src/IMAGE_PIN").is_file())


def body_from(source: str, start: int) -> str:
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " " for char in match[0]),
        source,
    )
    opening = masked.find("{", start)
    assert opening >= 0
    depth = 0
    for pos in range(opening, len(masked)):
        if masked[pos] == "{":
            depth += 1
        elif masked[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[start:pos + 1]
    raise AssertionError("unterminated source body")


def test_both_original_priority_paths_use_named_player_members() -> None:
    source = (ROOT / "src/console.cpp").read_text(encoding="utf-8")
    player_header = (ROOT / "src/faction.h").read_text(encoding="utf-8")
    setter_start = source.index("void Console::set_ai(int faction_id)")
    setter = body_from(source, setter_start)
    key_start = source.index("case 'R':", source.index("int Console::on_key_click"))
    key_route = body_from(source, key_start)

    fields = ("ai_growth", "ai_tech", "ai_wealth", "ai_power")
    for field in fields:
        assert re.search(rf"\bBOOL\s+{field}\s*;", player_header)
        assert f"PlayersData[faction_id].{field}" in setter
        assert f"PlayersData[faction].{field}" in key_route
    assert "unk_38" not in setter and "unk_38" not in key_route

    popup = setter.index("x_pop_short(ConsoleStrTechRandom")
    answer = setter.index("const int answer = PopupWriteCheckState;")
    sync = setter.index("synch_ai(faction_id)")
    assert setter.index("PopupWriteCheckState = static_cast<int>(flags);") < popup < answer < sync
    key_popup = key_route.index("x_pop_short(ConsoleStrTechRandom")
    key_answer = key_route.index("const int answer = PopupWriteCheckState;")
    key_sync = key_route.index("synch_ai(faction)")
    assert key_route.index("popup_set_write_check_bit(2,") < key_route.index("popup_set_write_check_bit(3,") < key_popup
    assert key_popup < key_answer < key_sync
    assert "social_select(faction_id_)" in key_route
