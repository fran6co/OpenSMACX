"""Private literal identities retain history and exact TU measurement obligations."""

from copy import deepcopy
from dataclasses import replace

import pytest

from match_recovery.literal_source import parse_literals
from match_recovery.preservation import _bindings, _outside_annotations, _unit_scope
from match_recovery.source_config import SourceConfiguration
from match_recovery.source_obligations import task_obligations
from match_recovery.structural_requirements import check_requirements, declared_outcome, structural_outcome
from test_generated_source_authority import observe, permission
from test_source_configuration import configured
from test_source_history import commit, task_project
from test_structural_requirements import FLAGS, inventory
from test_task_source import project


ANNOTATION = "// RECOVERY_LITERAL: v1 0x2000 0x2004 __real@4@3f800000 0x10\n"
PIN = "a" * 64


def literal(unit="code/owner.cc"):
    return parse_literals(ANNOTATION, unit, PIN, address_space="custom:offset")[0]


def candidate(item=None):
    item = item or literal()
    return {"checks": {"binary": {
        "translation_units": [item.unit, "code/caller.cc"], "flag_sets": deepcopy(FLAGS),
        "literal_owners": [{"interpretation_id": item.interpretation_id, "unit": item.unit}],
        "literal_definitions": [{"interpretation_id": item.interpretation_id, "unit": item.unit,
                                 "flags": flags, "definition": "pass", "content": "pass",
                                 "source": {"path": item.unit, "line": item.line}}
                                for flags in FLAGS]}}}


@pytest.mark.parametrize("change", ["remove", "rehome", "seed"])
def test_literal_history_retains_original_interpretation_and_evidence_scope(task_project, change):
    path = task_project / "code/owner.cc"
    original = path.read_text()
    path.write_text(ANNOTATION + original)
    commit(task_project, "Source-owned private literal interpretation")
    before = task_obligations(task_project, "owner")
    old, = before["interpretations"]
    assert old["current"] and old["entity_id"] == literal().evidence.entity_id
    path.write_text(original)
    if change != "remove":
        destination = "code/new.cc" if change == "rehome" else "hints/attempt.cc"
        (task_project / destination).write_text(ANNOTATION)
    after = task_obligations(task_project, "owner")
    assert {row["obligation_id"] for row in before["obligations"]} == {
        row["obligation_id"] for row in after["obligations"]}
    retained = next(row for row in after["interpretations"] if row["interpretation_id"] == old["interpretation_id"])
    assert not retained["current"]
    assert len(after["interpretations"]) == (2 if change == "rehome" else 1)
    if change == "rehome":
        current, = [row for row in after["interpretations"] if row["current"]]
        assert current["value"]["unit"] == "code/new.cc"
        assert current["interpretation_id"] != old["interpretation_id"]


def test_literal_outside_scope_preserves_home_and_multiplicity_without_body_waiver(tmp_path):
    configuration = SourceConfiguration("PIN", "custom:offset", roots=("code",))
    item = literal()
    files = {item.unit: ANNOTATION.encode()}
    baseline = _outside_annotations(files, configuration, PIN, frozenset())
    assert len(baseline) == 1
    assert baseline == _outside_annotations({item.unit: b"\n" + files[item.unit]}, configuration, PIN, frozenset())
    assert baseline != _outside_annotations({"code/new.cc": files[item.unit]}, configuration, PIN, frozenset())
    assert baseline == _outside_annotations(files, configuration, PIN, frozenset({item.origin.entity_id}))
    assert not _outside_annotations(files, configuration, PIN, frozenset({item.evidence.entity_id}))
    assert len(_outside_annotations({item.unit: files[item.unit] * 2}, configuration, PIN, frozenset())) == 2
    bindings = _bindings(tmp_path, files, configuration, PIN)
    assert bindings == {}
    assert _unit_scope(bindings, frozenset({item.evidence.entity_id}), item.unit) == ({}, set())


def test_literal_requires_owner_definition_and_content_in_every_trial():
    item = literal()
    report = candidate(item)
    row, = check_requirements(inventory((item, True)), report)
    assert row["result"] == declared_outcome([row], {})[0] == "pass"
    assert row["counts"] == {"pass": 2, "fail": 0, "unknown": 0}
    assert structural_outcome([row])[0] == "unknown"


@pytest.mark.parametrize("address", ["0x10", "0x2000"])
def test_literal_never_supplies_generated_registration_authority(tmp_path, address):
    from match_recovery.model import Origin

    entity = Origin(PIN, "test:va", int(address, 16)).entity_id
    with pytest.raises(ValueError, match="unique original source anchor"):
        observe(tmp_path, permission(entity_id=entity),
                versions={"current": {"code/owner.cc": ANNOTATION.encode()}}, scope=frozenset({entity}))


@pytest.mark.parametrize("damage", ["missing_trial", "missing_owners", "wrong_owner", "duplicate_owner",
                                    "foreign_measurements", "foreign_extra", "unrequested_flags",
                                    "missing_flags", "missing_definition", "missing_content"])
def test_literal_measurement_gaps_never_borrow_another_tu_or_trial(damage):
    item = literal()
    report = candidate(item)
    binary = report["checks"]["binary"]
    definitions = binary["literal_definitions"]
    if damage == "missing_trial": definitions.pop()
    elif damage == "missing_owners": binary.pop("literal_owners")
    elif damage == "wrong_owner": binary["literal_owners"][0]["unit"] = "code/caller.cc"
    elif damage == "duplicate_owner": binary["literal_owners"] *= 2
    elif damage == "foreign_measurements":
        for definition in definitions:
            definition["unit"] = "code/caller.cc"
    elif damage == "foreign_extra": definitions.append({**definitions[0], "unit": "code/caller.cc"})
    elif damage == "unrequested_flags": definitions[0]["flags"] = ["not requested"]
    elif damage == "missing_flags": binary.pop("flag_sets")
    elif damage == "missing_definition": definitions[0].pop("definition")
    elif damage == "missing_content": definitions[0].pop("content")
    row, = check_requirements(inventory((item, True)), report)
    assert row["result"] == declared_outcome([row], {})[0] == "unknown"


@pytest.mark.parametrize("dimension", ["definition", "content"])
def test_literal_counterexample_remains_actionable_with_a_missing_trial(dimension):
    item = literal()
    report = candidate(item)
    definitions = report["checks"]["binary"]["literal_definitions"]
    definitions[0][dimension] = "fail"
    row, = check_requirements(inventory((item, True)), report)
    assert declared_outcome([row], {})[0] == "fail"
    definitions.pop()
    row, = check_requirements(inventory((item, True)), report)
    assert row["result"] == "unknown" and structural_outcome([row])[0] == "fail"


def test_rehomed_literal_cannot_borrow_historical_measurements():
    item = literal()
    moved = replace(item, unit="code/new.cc")
    rows = check_requirements(inventory((item, False), (moved, True)), candidate(item))
    assert all(row["result"] == "unknown" for row in rows)
    old, = [row for row in rows if not row["current"]]
    assert old["next_action"] == "review_supersession_or_retraction" and not old["trials"]
