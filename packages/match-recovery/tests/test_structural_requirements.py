"""Fresh declared evidence never substitutes for history or whole-class proof."""

from copy import deepcopy
from dataclasses import asdict, replace

import pytest

from match_recovery.generated_binary import check_generated
from match_recovery.layout_source import parse_bases, parse_members
from match_recovery.model import Result
from match_recovery.object_source import parse_objects
from match_recovery.structural_requirements import check_requirements, structural_outcome
from test_generated import fixture


MEMBER = parse_members("// RECOVERY_MEMBER: v3 Owner field_ Buffer* 0x8 0x10", "code/owner.hpp", "a" * 64,
                       address_space="custom:offset")[0]
FLAGS = [["trial-a"], ["trial-b"]]


def test_object_initial_data_requires_owner_trials_not_duplicate_definitions_in_callers():
    item, = parse_objects("// RECOVERY_OBJECT: v2 0x2000 0x2004 _g 0x10", "code/data.cc", "a" * 64, address_space="custom:offset")
    report = candidate()
    binary = report["checks"]["binary"]
    binary["object_owners"] = [{"interpretation_id": item.interpretation_id, "unit": item.unit}]
    binary["object_definitions"] = [{"interpretation_id": item.interpretation_id, "unit": item.unit,
                                    "flags": flags, "definition": "pass", "content": "pass"} for flags in FLAGS]
    row, = check_requirements(inventory((item, True)), report)
    assert row["result"] == "pass" and row["counts"]["pass"] == 2
    binary["object_definitions"][1]["content"] = "fail"
    row, = check_requirements(inventory((item, True)), report)
    assert structural_outcome([row])[0] == "fail"
    binary["object_definitions"].pop()
    row, = check_requirements(inventory((item, True)), report)
    assert row["result"] == "unknown" and row["counts"]["unknown"] == 1
    binary["object_owners"] = []
    row, = check_requirements(inventory((item, True)), report)
    assert row["result"] == "unknown"


def inventory(*items):
    return {"scope": sorted({item.evidence.entity_id for item, current in items}),
            "interpretations": [{"interpretation_id": item.interpretation_id, "entity_id": item.evidence.entity_id,
                                 "value": item.interpretation, "current": current} for item, current in items]}


def candidate(item=MEMBER, results=("pass", "pass"), *, units=("code/owner.cc",), collection="members"):
    rows = [{"interpretation_id": item.interpretation_id, "expectation": asdict(item),
             "unit": unit, "flags": flags, "result": result}
            for unit in units for flags, result in zip(FLAGS, results)]
    return {"checks": {"layout": {"translation_units": list(units), "flag_sets": deepcopy(FLAGS), collection: rows},
                       "binary": {"translation_units": list(units), "flag_sets": deepcopy(FLAGS)}}}


def test_all_declared_witnesses_pass_without_proving_complete_structure():
    row, = check_requirements(inventory((MEMBER, True)), candidate())
    assert row["result"] == "pass" and row["counts"] == {"pass": 2, "fail": 0, "unknown": 0}
    assert all(trial["source"] == {"path": "code/owner.hpp", "line": 1} for trial in row["trials"])
    assert structural_outcome([row])[0] == Result.UNKNOWN
    assert row["next_action"] == "verify_remaining_structure_and_lifetime"


@pytest.mark.parametrize("results, expected", [(("pass", "fail"), "fail"), (("pass",), "unknown"),
                                            (("pass", "unknown"), "unknown"), ((), "unknown")])
def test_no_passing_trial_hides_a_failure_or_missing_witness(results, expected):
    row, = check_requirements(inventory((MEMBER, True)), candidate(results=results))
    assert row["result"] == expected and len(row["trials"]) == 2
    assert structural_outcome([row])[0] == (Result.FAIL if expected == "fail" else Result.UNKNOWN)


def test_a_counterexample_stays_actionable_despite_another_missing_tu():
    report = candidate(results=("pass", "fail"))
    report["checks"]["layout"]["translation_units"].append("code/another.cc")
    row, = check_requirements(inventory((MEMBER, True)), report)
    assert row["result"] == "unknown" and row["counts"] == {"pass": 1, "fail": 1, "unknown": 2}
    assert structural_outcome([row])[0] == Result.FAIL
    assert row["next_action"] == "repair_source_or_review_interpretation"


@pytest.mark.parametrize("change", ["member", "offset", "class", "type", "evidence", "image", "address_space"])
def test_different_interpretations_cannot_borrow_measurements(change):
    mutations = {"member": {"member": "other_"}, "offset": {"offset": 12}, "class": {"class_name": "Other"},
                 "type": {"class_pointer_type": "Other"}, "evidence": {"evidence": replace(MEMBER.evidence, address=32)},
                 "image": {"evidence": replace(MEMBER.evidence, image_sha256="b" * 64)},
                 "address_space": {"evidence": replace(MEMBER.evidence, address_space="other:offset")}}
    changed = replace(MEMBER, **mutations[change])
    row, = check_requirements(inventory((changed, True)), candidate())
    assert row["result"] == "unknown" and row["counts"]["pass"] == 0


def test_rehoming_does_not_change_interpretation_identity():
    moved = replace(MEMBER, path="code/new.hpp", line=22)
    assert moved.interpretation_id == MEMBER.interpretation_id
    row, = check_requirements(inventory((moved, True)), candidate(moved))
    assert row["result"] == "pass"
    assert all(trial["source"] == {"path": "code/new.hpp", "line": 22} for trial in row["trials"])


def test_old_interpretation_is_not_auto_superseded_by_a_passing_replacement():
    old = replace(MEMBER, class_pointer_type=None)
    rows = check_requirements(inventory((old, False), (MEMBER, True)), candidate())
    previous = next(row for row in rows if not row["current"])
    assert previous["result"] == "unknown" and previous["trials"] == []
    assert previous["next_action"] == "review_supersession_or_retraction"
    assert next(row for row in rows if row["current"])["result"] == "pass"
    assert structural_outcome(rows)[0] == Result.UNKNOWN


def test_even_stale_failing_rows_do_not_measure_a_historical_interpretation():
    row, = check_requirements(inventory((MEMBER, False)), candidate(results=("fail", "fail")))
    assert row["counts"]["fail"] == 0 and row["trials"] == []
    assert structural_outcome([row])[0] == Result.UNKNOWN


def test_missing_measurement_identity_never_falls_back_to_spelling_or_offset():
    report = candidate()
    for row in report["checks"]["layout"]["members"]:
        del row["interpretation_id"]
    row, = check_requirements(inventory((MEMBER, True)), report)
    assert row["result"] == "unknown" and row["counts"]["pass"] == 0


def test_extra_compiled_dependency_witness_cannot_hide_a_counterexample():
    report = candidate()
    extra = {**report["checks"]["layout"]["members"][0], "unit": "code/dependency.cc", "result": "fail"}
    report["checks"]["layout"]["members"].append(extra)
    row, = check_requirements(inventory((MEMBER, True)), report)
    assert row["counts"] == {"pass": 2, "fail": 1, "unknown": 1}
    assert structural_outcome([row])[0] == Result.FAIL


def test_unrequested_flags_do_not_supply_a_passing_witness():
    report = candidate()
    report["checks"]["layout"]["members"][0]["flags"] = ["unrequested"]
    row, = check_requirements(inventory((MEMBER, True)), report)
    assert row["result"] == "unknown" and row["counts"] == {"pass": 1, "fail": 0, "unknown": 2}


@pytest.mark.parametrize("key", ["translation_units", "flag_sets"])
def test_missing_coverage_metadata_never_implies_complete_proof(key):
    report = candidate()
    del report["checks"]["layout"][key]
    row, = check_requirements(inventory((MEMBER, True)), report)
    assert row["result"] == "unknown" and "coverage" in row["reason"]


def test_empty_native_report_is_a_coverage_gap_not_an_empty_pass():
    row, = check_requirements(inventory((MEMBER, True)), {"checks": {}})
    assert row["result"] == "unknown" and row["next_action"] == "establish_measurement_coverage"


@pytest.mark.parametrize("damage", ["duplicate-row", "duplicate-unit", "duplicate-flags", "wrong-kind", "bad-result", "bad-id"])
def test_ambiguous_or_malformed_measurements_are_refused(damage):
    report = candidate()
    layout = report["checks"]["layout"]
    if damage == "duplicate-row":
        layout["members"].append(deepcopy(layout["members"][0]))
    elif damage == "duplicate-unit":
        layout["translation_units"] *= 2
    elif damage == "duplicate-flags":
        layout["flag_sets"] *= 2
    elif damage == "wrong-kind":
        layout["bases"] = layout.pop("members")
    elif damage == "bad-result":
        layout["members"][0]["result"] = "looks good"
    else:
        layout["members"][0]["interpretation_id"] = "bad"
    with pytest.raises(ValueError):
        check_requirements(inventory((MEMBER, True)), report)


def test_base_requirements_use_the_same_source_identity_contract():
    base, = parse_bases("// RECOVERY_BASE: v1 Owner Base 0x0 0x10", "code/owner.hpp", "a" * 64, address_space="custom:offset")
    row, = check_requirements(inventory((base, True)), candidate(base, collection="bases"))
    assert row["result"] == "pass" and row["kind"] == "nonvirtual_base_offset"
    assert structural_outcome([row])[0] == Result.UNKNOWN


@pytest.mark.parametrize("bounded, target, expected", [(True, 0x402000, "pass"), (True, 0x403000, "fail"),
                                                      (False, 0x402000, "unknown")])
def test_generated_tables_require_content_not_just_a_start_binding(bounded, target, expected):
    binding, objects, image = fixture(bounded=bounded)
    _, measured = check_generated((binding,), objects, image, {"_method": target})
    report = {"checks": {"binary": {"translation_units": list(objects), "flag_sets": FLAGS,
               "generated_tables": [{**row, "flags": flags} for row in measured for flags in FLAGS]}}}
    row, = check_requirements(inventory((binding, True)), report)
    assert row["result"] == expected
    assert structural_outcome([row])[0] == (Result.FAIL if expected == "fail" else Result.UNKNOWN)
    assert measured[0]["interpretation_id"] == binding.interpretation_id


def test_no_emitted_table_does_not_erase_its_declared_requirement():
    binding, objects, _ = fixture()
    report = {"checks": {"binary": {"translation_units": list(objects), "flag_sets": FLAGS, "generated_tables": []}}}
    row, = check_requirements(inventory((binding, True)), report)
    assert row["result"] == "unknown" and row["counts"]["unknown"] == 2
