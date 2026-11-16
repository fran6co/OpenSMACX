"""Observed-link qualification of VC6 `??_E` weak vftable slots."""

from dataclasses import replace
import struct
from types import SimpleNamespace

import pytest

from match_recovery.generated_binary import check_generated
from match_recovery.msvc_link_map import Public
from match_recovery.pe_coff import Symbol
from match_recovery.weak_selection import LinkObservation, qualify_weak_fallbacks
from test_generated import fixture

WEAK, FALLBACK = "??_EWidget@@UAEPAXI@Z", "??_GWidget@@UAEPAXI@Z"


def weak_object(*, mode=2, fallback=FALLBACK, strong=False):
    symbols = {0: Symbol(0, WEAK, 0, 0, 0x20, 105), 1: Symbol(1, fallback, 0, 0, 0x20, 2)}
    if strong:
        symbols[2] = Symbol(2, WEAK, 0, 1, 0x20, 2)
    return SimpleNamespace(symbols=symbols, auxiliary={0: struct.pack("<II10x", 1, mode)})


def observation(*, weak_address=0x402000, weak_owner="widget.cpp.obj", rows=1, function=True):
    publics = {FALLBACK: (Public(FALLBACK, 1, 0x1000, 0x402000, True, "widget.cpp.obj"),)}
    if rows:
        publics[WEAK] = tuple(Public(WEAK, 1, 0x1000, weak_address, function, weak_owner)
                              for _ in range(rows))
    return LinkObservation(publics, {"map_sha256": "0" * 64})


def test_map_placing_the_weak_name_on_its_default_selects_the_fallback():
    result = qualify_weak_fallbacks({WEAK}, {"src/a.cpp": weak_object(), "src/b.cpp": weak_object()},
                                    observation())
    assert result.selected == {WEAK: FALLBACK} and not result.refused
    assert result.evidence == {"map_sha256": "0" * 64}


@pytest.mark.parametrize("damage,reason", [
    ("no_observation", "no observed link"),
    ("strong_definition", "defines the weak name strongly"),
    ("separate_address", "separate strong definition"),
    ("separate_owner", "separate strong definition"),
    ("missing_row", "lacks one placement"),
    ("duplicate_row", "lacks one placement"),
    ("data_row", "not function code"),
    ("mode", "uniformly default"),
    ("other_fallback", "uniformly default"),
    ("mixed_objects", "uniformly default"),
])
def test_weak_selection_refuses_without_complete_link_evidence(damage, reason):
    objects = {"src/a.cpp": weak_object()}
    link = observation()
    if damage == "no_observation":
        link = None
    elif damage == "strong_definition":
        objects["src/b.cpp"] = weak_object(strong=True)
    elif damage == "separate_address":
        link = observation(weak_address=0x403000)
    elif damage == "separate_owner":
        link = observation(weak_owner="vector.cpp.obj")
    elif damage == "missing_row":
        link = observation(rows=0)
    elif damage == "duplicate_row":
        link = observation(rows=2)
    elif damage == "data_row":
        link = observation(function=False)
    elif damage == "mode":
        objects = {"src/a.cpp": weak_object(mode=3)}
    elif damage == "other_fallback":
        objects = {"src/a.cpp": weak_object(fallback="??_GOther@@UAEPAXI@Z")}
    else:
        objects["src/b.cpp"] = weak_object(mode=1)
    result = qualify_weak_fallbacks({WEAK}, objects, link)
    assert result.selected == {} and reason in result.refused[WEAK]


def test_only_vector_deleting_names_are_qualified():
    obj = weak_object()
    obj.symbols[0] = replace(obj.symbols[0], name="_weak")
    result = qualify_weak_fallbacks({"_weak"}, {"src/a.cpp": obj}, observation())
    assert "only ??_E" in result.refused["_weak"]


def table_with_weak_slot(*, mode=2):
    binding, objects, image = fixture()
    obj = objects["src/widget.cpp"]
    obj.symbols[1] = Symbol(1, WEAK, 0, 0, 0x20, 105)
    obj.symbols[2] = Symbol(2, FALLBACK, 0, 0, 0x20, 2)
    obj.auxiliary = {1: struct.pack("<II10x", 2, mode)}
    return binding, objects, image


def test_selected_weak_slot_resolves_through_its_bound_fallback():
    binding, objects, image = table_with_weak_slot()
    selections = qualify_weak_fallbacks({WEAK}, objects, observation())
    _, rows = check_generated((binding,), objects, image, {FALLBACK: 0x402000}, weak_selections=selections)
    row, = rows
    assert row["content"] == "pass"
    entry, = row["diagnostic"]["entries"]
    assert entry["observation"] == "target_word_agrees"
    assert entry["relocations"][0]["linker_fallback"]["selection"] == "observed_link"
    relocation, = row["proof"]["relocations"]
    assert relocation["weak_selection"]["fallback"] == FALLBACK


def test_selected_weak_slot_still_fails_a_wrong_fallback_target():
    binding, objects, image = table_with_weak_slot()
    selections = qualify_weak_fallbacks({WEAK}, objects, observation())
    _, rows = check_generated((binding,), objects, image, {FALLBACK: 0x403000}, weak_selections=selections)
    assert rows[0]["content"] == "fail"
    assert rows[0]["diagnostic"]["entries"][0]["observation"] == "target_word_differs"


@pytest.mark.parametrize("damage", ["refused", "unbound_fallback", "mode"])
def test_unqualified_weak_slot_remains_a_linker_selection(damage):
    binding, objects, image = table_with_weak_slot(mode=3 if damage == "mode" else 2)
    link = observation(weak_address=0x403000) if damage == "refused" else observation()
    selections = qualify_weak_fallbacks({WEAK}, objects, link)
    if damage == "mode":
        # A forged selection still needs the object's own mode-2 record.
        selections = replace(selections, selected={WEAK: FALLBACK})
    external = {} if damage == "unbound_fallback" else {FALLBACK: 0x402000}
    _, rows = check_generated((binding,), objects, image, external, weak_selections=selections)
    entry, = rows[0]["diagnostic"]["entries"]
    assert entry["observation"] == "linker_selection_required"
    assert rows[0]["content"] == "unknown"
    if damage == "refused":
        assert "separate strong definition" in entry["relocations"][0]["linker_fallback"]["refusal"]


def _configured(tmp_path, value):
    import json
    from match_recovery.measurement import Measurement
    from match_recovery.project import initialize
    initialize(tmp_path, "link-observation", "unused-compiler")
    path = tmp_path / "recovery/project.json"
    config = json.loads(path.read_text())
    config["measurement"]["link_observation"] = value
    path.write_text(json.dumps(config))
    return Measurement.read(tmp_path)


PATHS = {"map": "build/app.map", "image": "build/app.exe",
         "inputs": "build/app.link.inputs.json", "log": "build/app.link.log"}


def test_link_observation_configuration_is_exact_and_relative(tmp_path):
    assert dict(_configured(tmp_path, PATHS).link_observation) == PATHS


@pytest.mark.parametrize("value", [{**PATHS, "pdb": "build/app.pdb"}, {"map": "build/app.map"},
                                   {**PATHS, "map": "/abs/app.map"}, {**PATHS, "map": "../app.map"}, []])
def test_link_observation_configuration_refuses_partial_or_escaping_paths(tmp_path, value):
    with pytest.raises(ValueError):
        _configured(tmp_path, value)


def test_missing_observed_link_files_refuse(tmp_path):
    from match_recovery.weak_selection import read_link_observation
    with pytest.raises(ValueError, match="observed link map is missing"):
        read_link_observation(tmp_path, PATHS)
