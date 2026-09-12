"""Coordinator intent syntax grants neither source nor linked ownership."""

from copy import deepcopy
from types import SimpleNamespace

import pytest

from match_recovery.migration_protocol import permissions


ENTITY = "entity-" + "a" * 64
OTHER = "entity-" + "b" * 64
TASK = SimpleNamespace(author="coordinator")
SCOPE = frozenset({ENTITY})


def permission():
    return {"migration_id": "initializer-migration", "author": TASK.author,
            "evidence": ["Scoped lifecycle restructuring"], "allowed": []}


def occurrence(**changes):
    return {"entity_id": ENTITY, "unit": "src/lifecycle.cpp", "version": "current",
            "symbol": "_$E21", "offset": 0, "evidence": ["Scoped original lifecycle evidence"],
            **changes}


def generated(*rows):
    return {**permission(), "generated_changes": list(rows)}


def test_legacy_permission_is_unchanged_and_not_expanded():
    value = permission()
    original = deepcopy(value)
    assert permissions(value, TASK, SCOPE) is value
    assert value == original and "generated_changes" not in value


@pytest.mark.parametrize("dimension", ["bytes", "instruction_shape"])
def test_generated_intent_does_not_change_existing_matching_permissions(dimension):
    value = generated(occurrence())
    value["allowed"] = [{"entity_id": ENTITY, "dimension": dimension}]
    original = deepcopy(value)
    assert permissions(value, TASK, SCOPE) is value
    assert value == original


def test_explicit_baseline_and_current_occurrences_are_distinct():
    value = generated(occurrence(version="baseline"), occurrence())
    assert permissions(value, TASK, SCOPE) is value


def test_symbols_and_offsets_are_local_to_exact_unit():
    value = generated(occurrence(), occurrence(unit="src/second.cpp"))
    assert permissions(value, TASK, SCOPE) is value


def test_parser_does_not_claim_editable_file_or_emission_ownership():
    # The parser has no source or compiler inputs. The live consumer must reject
    # this if the unit is not editable or the actual emitted root is not owned.
    value = generated(occurrence(unit="not-an-editable-unit.cpp"))
    assert permissions(value, TASK, SCOPE) is value


@pytest.mark.parametrize("value", [None, {}, (), "changes", [], [None], ["entry"], [[]]])
def test_invalid_generated_change_container_or_row(value):
    with pytest.raises(ValueError):
        permissions({**permission(), "generated_changes": value}, TASK, SCOPE)


@pytest.mark.parametrize("field", ["entity_id", "unit", "version", "symbol", "offset", "evidence"])
def test_missing_generated_change_field(field):
    row = occurrence()
    del row[field]
    with pytest.raises(ValueError, match="fields"):
        permissions(generated(row), TASK, SCOPE)


def test_extra_generated_change_field():
    with pytest.raises(ValueError, match="fields"):
        permissions(generated(occurrence(whole_unit=True)), TASK, SCOPE)


@pytest.mark.parametrize("field,value", [
    ("entity_id", OTHER), ("entity_id", "a" * 64), ("entity_id", None),
    ("unit", "../src/a.cpp"), ("unit", "/src/a.cpp"), ("unit", "src/./a.cpp"),
    ("unit", "src//a.cpp"), ("unit", "src\\a.cpp"), ("unit", "."),
    ("unit", "C:/src/a.cpp"), ("unit", ""), ("unit", None),
    ("version", "both"), ("version", "Baseline"), ("version", None), ("version", []),
    ("symbol", ""), ("symbol", " "), ("symbol", "_$E21 "),
    ("symbol", "_$E\n21"), ("symbol", "_$E\t21"), ("symbol", "_$E\x0021"),
    ("symbol", "_$E\x7f21"), ("symbol", "_$E\u00a021"), ("symbol", None),
    ("offset", -4), ("offset", 1), ("offset", 2), ("offset", 3),
    ("offset", 16 * 1024 * 1024), ("offset", 16 * 1024 * 1024 + 4),
    ("offset", True), ("offset", False), ("offset", 0.0), ("offset", "0"),
    ("offset", None), ("evidence", []), ("evidence", "reason"),
    ("evidence", [""]), ("evidence", [" "]), ("evidence", [None]),
])
def test_invalid_generated_change_value(field, value):
    with pytest.raises(ValueError):
        permissions(generated(occurrence(**{field: value})), TASK, SCOPE)


@pytest.mark.parametrize("offset", [0, 4, 16 * 1024 * 1024 - 4])
def test_valid_bounded_slot(offset):
    assert permissions(generated(occurrence(offset=offset)), TASK, SCOPE)


@pytest.mark.parametrize("symbol", ["_$E21", "?initializer@@YAXXZ", "initializer", "_entry@0"])
def test_exact_linker_identifiers(symbol):
    assert permissions(generated(occurrence(symbol=symbol)), TASK, SCOPE)


@pytest.mark.parametrize("other", [occurrence(), occurrence(symbol="_$E22"), occurrence(offset=4)])
def test_duplicate_occurrence_or_root_refused(other):
    with pytest.raises(ValueError, match="duplicate"):
        permissions(generated(occurrence(), other), TASK, SCOPE)


def test_scope_anchor_cannot_hide_duplicate_occurrence():
    with pytest.raises(ValueError, match="duplicate"):
        permissions(generated(occurrence(), occurrence(entity_id=OTHER)), TASK, SCOPE | {OTHER})


def test_generated_change_limit():
    rows = [occurrence(symbol="_$E" + str(index), offset=4 * index) for index in range(256)]
    assert permissions(generated(*rows), TASK, SCOPE)
    with pytest.raises(ValueError, match="256"):
        permissions(generated(*rows, occurrence(symbol="_$E256", offset=1024)), TASK, SCOPE)


@pytest.mark.parametrize("dimension", ["structure", "references", "outside_scope", "generated_changes"])
def test_generated_permission_does_not_expand_matching_regression_dimensions(dimension):
    value = generated(occurrence())
    value["allowed"] = [{"entity_id": ENTITY, "dimension": dimension}]
    with pytest.raises(ValueError, match="matching regressions"):
        permissions(value, TASK, SCOPE)


def test_generated_permission_still_requires_task_coordinator():
    value = generated(occurrence())
    value["author"] = "worker"
    with pytest.raises(ValueError, match="coordinator"):
        permissions(value, TASK, SCOPE)


def test_unrecognized_top_level_permission_is_not_accepted():
    value = {**generated(occurrence()), "generated_prefix": "_$E"}
    with pytest.raises(ValueError, match="fields"):
        permissions(value, TASK, SCOPE)
