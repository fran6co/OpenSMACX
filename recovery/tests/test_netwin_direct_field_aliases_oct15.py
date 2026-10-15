"""Guard the eight NetWin aliases as direct references to mapped owners.

The editor/scalar suites execute extracted-body reload, wrong-field and order
mutants. These checks pin the field identities without asserting native ABI or
reference-target proof.
"""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def _function(text, signature):
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for index in range(opening, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[start:index + 1]
    raise AssertionError(signature)


def test_netwin_aliases_are_direct_owner_fields_at_original_sites():
    source = (ROOT / "src/netwin.cpp").read_text()
    for alias in ("NetWinPlayerCount", "NetWinEighthSlotWord", "NetWinScenarioText",
                  "NetWinNinthSlot", "NetWinRuleBitsA", "NetWinRuleBitsB",
                  "NetWinRuleBitsC", "NetWinRulesGate"):
        assert alias not in source

    assert "NetDaemonNet.player_count_" in source
    assert "NetWinSessionData.mode_" in source
    assert "NetWinSessionData.scenario_text_wire_tail_[0]" in source
    assert "strcpy(NetWinSessionData.scenario_text_wire_tail_, scenario_name);" in source
    assert "NetWinSessionOptions.time_control_" in source
    for field in ("rule_bits_a_", "rule_bits_b_", "rule_bits_c_"):
        assert f"NetWinSessionOptions.{field}" in source
    gate = "NetDaemonNet.primary_state_.leading_words_[0]"
    assert source.count(gate) >= 10
    assert "NetDaemonNet.lock_state_" not in source
    assert "scenario_text_wire_tail_[0] = 0;" in source


def test_live_player_count_reload_and_time_control_byte_have_negative_controls():
    source = (ROOT / "src/netwin.cpp").read_text()
    faction = _function(source, "void NetWin::pick_faction(int")
    captured = faction.index("int player_count = count > 0 ? NetDaemonNet.player_count_ : 0;")
    reload = faction.index("player_count = NetDaemonNet.player_count_;")
    loop = faction.index("for (int player = 1; player <= player_count; ++player)")
    assert captured < loop < reload

    time_owner = "NetWinSessionOptions.time_control_"
    assert source.count(time_owner) >= 2
    assert "NetWinSessionOptions.rule_bits_a_" in source

    # Mutation controls: a cached local, another byte, or a neighboring state
    # word would no longer satisfy the actual owner/site guards above.
    assert "player_count = count > 0 ? first : 0;" not in faction
    pick_time = _function(source, "void NetWin::pick_time()")
    assert "NetWinSessionOptions.time_control_ =" in pick_time
    assert "NetWinSessionOptions.option_4_ =" not in pick_time
    assert "NetWinSessionData.scenario_text_wire_tail_[3]" not in source
    assert "NetDaemonNet.primary_state_.leading_words_[1]" not in source
