"""Source-shape controls for the recovered src/probe.cpp body.

This file intentionally reads the product source only; it does not load any
ignored recovery candidate or packet fragment. The implementation under test lives in src/probe.cpp.
"""
from pathlib import Path
import re


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/probe.cpp"


def function_body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated product function")


def probe_body() -> str:
    source = SOURCE.read_text()
    return function_body(source, "int __cdecl probe(")


def test_probe_product_keeps_entry_warning_zero_exit_and_popup_scope():
    body = probe_body()
    assert re.search(r"Popup\s+\w+\s*;", body)
    assert re.search(
        r"if\s*\(entry_route\s*==\s*1\)\s*\{\s*"
        r"probe_native_entry_local_warning\([^;]+;\s*"
        r"goto\s+label_5a2cf4\s*;", body)
    trivia = r"(?:\s|//[^\n]*(?:\n|$))*"
    assert re.search(r"label_5a1e28\s*:" + trivia + r"return\s+1\s*;", body)
    assert re.search(r"label_5a5816\s*:" + trivia + r"goto\s+label_5a5821\s*;", body)
    assert re.search(r"label_5a5821\s*:" + trivia + r"return\s+1\s*;", body)
    assert re.search(r"label_5a2cf4\s*:\s*return\s+0\s*;", body)


def test_probe_product_keeps_selector_six_side_effect_order():
    body = probe_body()
    start = body.index("if (action_result == 6)")
    end = body.index("label_5a2012:", start)
    branch = body[start:end]
    ordered = (
        "mind_control(target_base_id, actor_faction_id, 1)",
        "if (control_chance < 0)",
        "Players[target_faction_id].rule_flags & 0x800",
        "Players[actor_faction_id].faction_bonus_count",
        "probe_native_nethack_terminus_adjust_amount(",
        "probe_native_social_amount_prefix(",
        'popup.start(PopupStartCaption, "THOUGHTCONTROL"',
        'popup.start(PopupStartCaption, "THOUGHTMENU"',
        "energy_reserves = static_cast<int32_t>(",
        "mind_control_total += 4",
        "diplo_mind_control[actor_faction_id] += 4",
    )
    positions = [branch.index(token) for token in ordered]
    assert positions == sorted(positions)
    assert "goto label_5a2574;" in branch
    assert "goto label_5a2ce9;" in branch
    assert "goto label_5a2dcd;" in branch


def test_probe_product_does_not_eagerly_initialize_native_roll_local():
    body = probe_body()
    declaration = re.search(r"\bint\s+(\w*native_probe_roll_threshold\w*)\s*;", body)
    assert declaration, "retain the native path-local stack value"
    name = declaration.group(1)
    before_first_native_store = body[:declaration.end()]
    assert not re.search(rf"\b{name}\s*=\s*", before_first_native_store)
    assert not re.search(rf"\b{name}\s*=\s*0\s*;", body[:declaration.end()])
