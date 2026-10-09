"""Direct emitted-name aliases retain the original object identity."""

from dataclasses import replace
import hashlib
import struct

import pytest

from match_recovery.model import Origin, Span
from match_recovery.object_data import check_initial_data
from match_recovery.object_source import (ObjectBinding, object_targets,
                                          parse_objects, referenced_object_units,
                                          require_disjoint_object_symbols,
                                          validate_object_symbol_ownership)
from match_recovery.pe_coff import Coff, PeImage
from test_object_data import data_fixture, data_image
from test_object_dependencies import data_object


PIN = "a" * 64


def test_v4_parses_unbounded_and_bounded_roots_without_changing_interpretation():
    text = ("// RECOVERY_OBJECT: v4 0x00401000 _old target _new\n"
            "// RECOVERY_OBJECT: v4 0x00402000 0x00402004 _old_bounded 0x00403000 target _new_bounded\n")
    unbounded, bounded = parse_objects(text, "src/data.cpp", PIN, address_space="pe:va")
    assert (unbounded.symbol, unbounded.target_symbol, unbounded.span) == ("_old", "_new", None)
    assert (bounded.symbol, bounded.emitted_symbol) == ("_old_bounded", "_new_bounded")
    assert bounded.span == Span(0x402000, 0x402004)
    assert bounded.interpretation["symbol"] == "_old_bounded"
    assert "target_symbol" not in bounded.interpretation
    assert bounded.interpretation_id == replace(bounded, target_symbol="_another").interpretation_id


def test_member_and_direct_target_forms_are_mutually_exclusive():
    origin = Origin(PIN, "pe:va", 0x401000)
    with pytest.raises(ValueError, match="cannot be combined"):
        ObjectBinding(origin, "_old", "src/data.cpp", 1,
                      parent_symbol="_parent", member_path=("field",), target_symbol="_new")
    with pytest.raises(ValueError, match="invalid object annotation"):
        parse_objects("// RECOVERY_OBJECT: v4 0x00401000 _old member _parent field target _new\n",
                      "src/data.cpp", PIN, address_space="pe:va")


def test_current_coff_definition_and_relocations_use_only_direct_target_name():
    origin = Origin(PIN, "pe:va", 0x401000)
    binding = ObjectBinding(origin, "_legacy", "src/global.cpp", 1,
                            target_symbol="_actual")
    target_object = Coff(data_object(b"_actual"))
    targets, rows = object_targets((binding,), {binding.unit: target_object})
    assert targets == {"_actual": origin.address}
    assert rows[0]["symbol"] == "_legacy" and rows[0]["target_symbol"] == "_actual"

    old_reference = Coff(data_object(b"_caller", b"_legacy"))
    current_reference = Coff(data_object(b"_caller", b"_actual"))
    assert not referenced_object_units((binding,), {"src/caller.cpp": old_reference})
    assert referenced_object_units((binding,), {"src/caller.cpp": current_reference}) == {binding.unit}


def test_current_target_namespace_refuses_member_identity_and_peer_collisions():
    origin = Origin(PIN, "pe:va", 0x401000)
    direct = ObjectBinding(origin, "_legacy", "src/global.cpp", 1,
                           target_symbol="_child")
    child = ObjectBinding(Origin(PIN, "pe:va", 0x401004), "_child", "src/global.cpp", 2,
                         parent_symbol="_parent", member_path=("field",))
    with pytest.raises(ValueError, match="historical object identity"):
        validate_object_symbol_ownership((direct, child))
    with pytest.raises(ValueError, match="conflicts"):
        require_disjoint_object_symbols((direct,), {"_child"})
    other = replace(direct, origin=Origin(PIN, "pe:va", 0x402000), symbol="_other")
    with pytest.raises(ValueError, match="multiple original objects"):
        validate_object_symbol_ownership((direct, other))


def test_bounded_initial_data_resolves_current_definition_but_reports_historical_id():
    raw_image = data_image()
    image = PeImage(raw_image, hashlib.sha256(raw_image).hexdigest())
    origin = Origin(hashlib.sha256(raw_image).hexdigest(), "pe:va", 0x401000)
    binding = ObjectBinding(origin, "_legacy", "src/global.cpp", 1,
                            Span(0x401000, 0x401004), origin, target_symbol="_actual")
    obj = Coff(data_fixture(definition=b"_actual"))
    targets, _ = object_targets((binding,), {binding.unit: obj})
    row, = check_initial_data((binding,), {binding.unit: obj}, image, targets)
    assert row["symbol"] == "_legacy" and row["target_symbol"] == "_actual"
    assert row["definition"] == "pass" and row["content"] == "pass"


@pytest.mark.parametrize("target_map", [{}, {"_actual": 0x401001}])
def test_direct_target_alias_needs_the_unique_external_current_definition(target_map):
    raw_image = data_image()
    image = PeImage(raw_image, hashlib.sha256(raw_image).hexdigest())
    origin = Origin(hashlib.sha256(raw_image).hexdigest(), "pe:va", 0x401000)
    binding = ObjectBinding(origin, "_legacy", "src/global.cpp", 1,
                            Span(0x401000, 0x401004), origin, target_symbol="_actual")
    obj = Coff(data_fixture(definition=b"_actual"))
    row, = check_initial_data((binding,), {binding.unit: obj}, image, target_map)
    assert row["definition"] == "unknown" and row["content"] == "unknown"
    assert "validated external" in row["reason"]


@pytest.mark.parametrize("damage", ["function_type", "code_section", "undefined", "common", "duplicate"])
def test_alias_target_requires_one_external_ordinary_data_definition(damage):
    obj = Coff(data_fixture(definition=b"_actual"))
    current = obj.symbols[0]
    if damage == "function_type":
        obj.symbols[0] = replace(current, type=0x20)
    elif damage == "code_section":
        obj.sections = (replace(obj.sections[0], flags=obj.sections[0].flags | 0x20), *obj.sections[1:])
    elif damage == "undefined":
        obj.symbols[0] = replace(current, section=0, value=0)
    elif damage == "common":
        obj.symbols[0] = replace(current, section=0, value=4)
    elif damage == "duplicate":
        obj.symbols[2] = replace(current)
    origin = Origin(PIN, "pe:va", 0x401000)
    binding = ObjectBinding(origin, "_legacy", "src/global.cpp", 1,
                            target_symbol="_actual")
    targets, rows = object_targets((binding,), {binding.unit: obj})
    assert targets == {} and rows[0]["definition"] == "unknown"
