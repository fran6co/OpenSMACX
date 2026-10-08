"""Fresh declared evidence never substitutes for history or whole-class proof."""

from copy import deepcopy
from dataclasses import asdict, replace
from types import SimpleNamespace

import pytest

from match_recovery.generated_binary import check_generated, generated_coverage
from match_recovery.layout_source import parse_bases, parse_members
from match_recovery.model import Result
from match_recovery.object_source import parse_objects
from match_recovery.literal_source import parse_literals
from match_recovery.shared_source import parse_shared
from match_recovery.structural_requirements import check_requirements, declared_outcome, structural_outcome
from test_generated import fixture


MEMBER = parse_members("// RECOVERY_MEMBER: v3 Owner field_ Buffer* 0x8 0x10", "code/owner.hpp", "a" * 64,
                       address_space="custom:offset")[0]
FLAGS = [["trial-a"], ["trial-b"]]


def absent_coverage(unit, flags, name="Owner"):
    return {"class_name": name, "unit": unit, "flags": flags, "observation": "no_complete_definition",
            "witness": {"class_name": name, "basis": "same_tu_definition_absence", "result": "pass",
                        "replica_control": "pass", "compiler_layout": None, "probe_symbol": "_match_recovery_absence_test",
                        "definition_controls": [{"extent": 17, "result": "pass"}, {"extent": 29, "result": "pass"}]}}


def test_controlled_absence_covers_only_the_named_tu_trial_not_full_structure():
    report = candidate()
    layout = report["checks"]["layout"]
    layout["translation_units"].append("code/another.cc")
    layout["coverage"] = [absent_coverage("code/another.cc", flags) for flags in FLAGS]
    row, = check_requirements(inventory((MEMBER, True)), report)
    assert row["result"] == declared_outcome([row], {})[0] == "pass"
    assert len([trial for trial in row["trials"] if trial.get("coverage_only")]) == 2
    assert structural_outcome([row])[0] == "unknown"


def test_layout_absence_must_match_declared_graph_context_when_present():
    report = candidate()
    layout = report["checks"]["layout"]
    layout["unit_graph_trials"] = {"code/owner.cc": [
        {"flags": flags, "graph_flags": flags} for flags in FLAGS]}
    layout["unit_graph_receipts"] = {"code/owner.cc": [
        {"flags": flags, "graph_flags": flags, "outcome": "compiled"} for flags in FLAGS]}
    layout["members"] = [{**row, "graph_flags": row["flags"]} for row in layout["members"]
                         if row["flags"] != FLAGS[0]]
    coverage = absent_coverage("code/owner.cc", FLAGS[0])
    coverage["graph_flags"] = FLAGS[0]
    layout["coverage"] = [coverage]
    row, = check_requirements(inventory((MEMBER, True)), report)
    assert row["result"] == "unknown"  # Absence coverage cannot supply the required actual layout witness.
    coverage["graph_flags"] = ["unconfigured"]
    with pytest.raises(ValueError, match="outside its declared"):
        check_requirements(inventory((MEMBER, True)), report)


@pytest.mark.parametrize("missing", ["all", "one_trial"])
def test_absence_never_supplies_the_required_actual_definition_in_each_trial(missing):
    report = candidate()
    layout = report["checks"]["layout"]
    layout["members"] = [] if missing == "all" else layout["members"][:1]
    layout["coverage"] = [absent_coverage("code/owner.cc", flags) for flags in (FLAGS if missing == "all" else FLAGS[1:])]
    row, = check_requirements(inventory((MEMBER, True)), report)
    assert row["counts"]["pass"] == 2  # Coverage can be closed, without a declaration to check.
    assert row["result"] == declared_outcome([row], {})[0] == "unknown"


@pytest.mark.parametrize("damage", ["replica", "width", "result", "class", "unit", "flags", "duplicate", "definition"])
def test_invalid_absence_evidence_cannot_remove_a_missing_witness(damage):
    report = candidate()
    layout = report["checks"]["layout"]
    layout["translation_units"].append("code/another.cc")
    coverage = absent_coverage("code/another.cc", FLAGS[0])
    layout["coverage"] = [coverage]
    if damage == "replica": coverage["witness"]["replica_control"] = "unknown"
    elif damage == "width": coverage["witness"]["definition_controls"][1]["extent"] = 17
    elif damage == "result": coverage["witness"]["result"] = "unknown"
    elif damage == "class": coverage["witness"]["class_name"] = "Another"
    elif damage == "unit": coverage["unit"] = "code/not_checked.cc"
    elif damage == "flags": coverage["flags"] = ["not_checked"]
    elif damage == "duplicate": layout["coverage"].append(deepcopy(coverage))
    elif damage == "definition": coverage["witness"]["compiler_layout"] = {"size": 17}
    with pytest.raises(ValueError, match="absence"):
        check_requirements(inventory((MEMBER, True)), report)


def test_observed_declaration_and_absence_cannot_both_describe_one_trial():
    report = candidate()
    report["checks"]["layout"]["coverage"] = [absent_coverage("code/owner.cc", FLAGS[0])]
    with pytest.raises(ValueError, match="both measured"):
        check_requirements(inventory((MEMBER, True)), report)


def test_absence_of_another_class_cannot_cover_the_required_class():
    report = candidate(results=())
    report["checks"]["layout"]["coverage"] = [absent_coverage("code/owner.cc", flags, "Different") for flags in FLAGS]
    row, = check_requirements(inventory((MEMBER, True)), report)
    assert row["result"] == "unknown" and row["counts"]["unknown"] == 2


def test_object_initial_data_requires_owner_trials_not_duplicate_definitions_in_callers():
    item, = parse_objects("// RECOVERY_OBJECT: v2 0x2000 0x2004 _g 0x10", "code/data.cc", "a" * 64, address_space="custom:offset")
    report = candidate()
    binary = report["checks"]["binary"]
    binary["object_owners"] = [{"interpretation_id": item.interpretation_id, "unit": item.unit}]
    binary["object_definitions"] = [{"interpretation_id": item.interpretation_id, "unit": item.unit,
                                    "flags": flags, "definition": "pass", "content": "pass"} for flags in FLAGS]
    row, = check_requirements(inventory((item, True)), report)
    assert row["result"] == "pass" and row["counts"]["pass"] == 2


def test_ragged_structural_data_grid_requires_only_the_exact_owner_profiles():
    item, = parse_objects("// RECOVERY_OBJECT: v2 0x2000 0x2004 _g 0x2010", "code/data.cc", "a" * 64,
                          address_space="custom:offset")
    report = candidate()
    binary = report["checks"]["binary"]
    binary["translation_units"] = ["code/data.cc"]
    binary["unit_flag_sets"] = {"code/data.cc": [FLAGS[1]]}
    binary["unit_graph_trials"] = {"code/data.cc": [
        {"flags": FLAGS[1], "graph_flags": graph_flags} for graph_flags in FLAGS]}
    binary["unit_graph_receipts"] = {"code/data.cc": [
        {"flags": FLAGS[1], "graph_flags": graph_flags, "outcome": "compiled"}
        for graph_flags in FLAGS]}
    binary["object_owners"] = [{"interpretation_id": item.interpretation_id, "unit": item.unit}]
    binary["object_definitions"] = [{"interpretation_id": item.interpretation_id, "unit": item.unit,
                                    "flags": FLAGS[1], "graph_flags": graph_flags,
                                    "definition": "pass", "content": "pass"} for graph_flags in FLAGS]
    row, = check_requirements(inventory((item, True)), report)
    assert row["result"] == "pass" and row["counts"]["pass"] == 1

    binary["translation_units"].append("code/extra.cc")
    binary["unit_flag_sets"]["code/extra.cc"] = deepcopy(FLAGS)
    binary["unit_graph_trials"]["code/extra.cc"] = [
        {"flags": graph_flags, "graph_flags": graph_flags} for graph_flags in FLAGS]
    binary["unit_graph_receipts"]["code/extra.cc"] = [
        {"flags": graph_flags, "graph_flags": graph_flags, "outcome": "compiled"}
        for graph_flags in FLAGS]
    binary["object_definitions"].extend([
        {"interpretation_id": item.interpretation_id, "unit": "code/extra.cc", "flags": graph_flags,
         "graph_flags": graph_flags, "definition": "fail", "content": "pass"}
        for graph_flags in FLAGS])
    row, = check_requirements(inventory((item, True)), report)
    assert row["result"] == "fail" and row["counts"]["fail"] == 2
    binary["object_definitions"][3]["content"] = "fail"
    row, = check_requirements(inventory((item, True)), report)
    assert structural_outcome([row])[0] == "fail"
    binary["object_definitions"] = binary["object_definitions"][:2]
    binary["translation_units"].pop()
    binary["unit_flag_sets"].pop("code/extra.cc")
    binary["unit_graph_trials"].pop("code/extra.cc")
    binary["unit_graph_receipts"].pop("code/extra.cc")
    row, = check_requirements(inventory((item, True)), report)
    assert row["result"] == "pass" and row["counts"]["pass"] == 1
    binary["object_owners"] = []
    row, = check_requirements(inventory((item, True)), report)
    assert row["result"] == "unknown"


@pytest.mark.parametrize("kind", ["object_initial_data", "tu_private_literal", "shared_function_body"])
def test_pinned_data_and_body_owners_require_every_declared_graph_context(kind):
    pin = "a" * 64
    if kind == "object_initial_data":
        item, = parse_objects("// RECOVERY_OBJECT: v2 0x2000 0x2004 _g 0x2010", "code/data.cc", pin,
                              address_space="custom:offset")
        collection, owner_key = "object_definitions", "object_owners"
    elif kind == "tu_private_literal":
        item, = parse_literals("// RECOVERY_LITERAL: v1 0x2000 0x2004 _literal 0x2010", "code/data.cc", pin,
                               address_space="custom:offset")
        collection, owner_key = "literal_definitions", "literal_owners"
    else:
        item, = parse_shared("// RECOVERY_SHARED: v1 0x2000 0x2004 _shared 0x2010\nvoid _shared() {}",
                             "code/data.cc", pin, address_space="custom:offset")
        collection, owner_key = "shared_functions", "shared_owners"
    graph_flags = [FLAGS[0], FLAGS[1]]
    report = {"checks": {"layout": {}, "binary": {
        "translation_units": ["code/data.cc"], "generated_units": ["code/data.cc"],
        "flag_sets": deepcopy(FLAGS), "unit_flag_sets": {"code/data.cc": [FLAGS[0]]},
        "unit_graph_trials": {"code/data.cc": [
            {"flags": FLAGS[0], "graph_flags": flags} for flags in graph_flags]},
        "unit_graph_receipts": {"code/data.cc": [
            {"flags": FLAGS[0], "graph_flags": flags, "outcome": "compiled"}
            for flags in graph_flags]},
        owner_key: [{"interpretation_id": item.interpretation_id, "unit": item.unit}],
        collection: []}}}
    if kind == "shared_function_body":
        report["checks"]["binary"][collection] = [
            {"interpretation_id": item.interpretation_id, "unit": item.unit,
             "flags": FLAGS[0], "graph_flags": flags, "result": "pass"}
            for flags in graph_flags]
    else:
        report["checks"]["binary"][collection] = [
            {"interpretation_id": item.interpretation_id, "unit": item.unit,
             "flags": FLAGS[0], "graph_flags": flags, "definition": "pass", "content": "pass"}
            for flags in graph_flags]
    row, = check_requirements(inventory((item, True)), report)
    assert row["result"] == "pass" and row["counts"]["pass"] == 1
    report["checks"]["binary"][collection].pop()
    row, = check_requirements(inventory((item, True)), report)
    assert row["result"] == "unknown" and row["counts"]["unknown"] == 1


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
    assert declared_outcome([row], {})[0] == Result.PASS


def test_ragged_unit_profile_grid_requires_each_exact_unit_trial():
    report = candidate(units=("code/owner.cc", "code/dependency.cc"))
    layout = report["checks"]["layout"]
    layout["unit_flag_sets"] = {"code/owner.cc": [FLAGS[0]],
                                "code/dependency.cc": deepcopy(FLAGS)}
    layout["members"] = [row for row in layout["members"]
                         if row["unit"] != "code/owner.cc" or row["flags"] == FLAGS[0]]
    report["checks"]["binary"]["unit_flag_sets"] = deepcopy(layout["unit_flag_sets"])
    row, = check_requirements(inventory((MEMBER, True)), report)
    assert row["result"] == "pass" and row["counts"]["pass"] == 3

    layout["members"] = [item for item in layout["members"]
                         if not (item["unit"] == "code/dependency.cc" and item["flags"] == FLAGS[1])]
    row, = check_requirements(inventory((MEMBER, True)), report)
    assert row["result"] == "unknown" and row["counts"]["unknown"] == 1

    layout["members"].append({"interpretation_id": MEMBER.interpretation_id,
                              "expectation": asdict(MEMBER), "unit": "code/dependency.cc",
                              "flags": ["unexpected"], "result": "pass"})
    row, = check_requirements(inventory((MEMBER, True)), report)
    assert row["result"] == "unknown" and row["counts"]["unknown"] >= 1


def test_graph_context_aggregation_preserves_failure_and_unknown_details():
    report = candidate(results=("fail", "unknown"))
    layout = report["checks"]["layout"]
    layout["unit_flag_sets"] = {"code/owner.cc": [FLAGS[0]]}
    for index, row in enumerate(layout["members"]):
        row["flags"] = FLAGS[0]
        row["graph_flags"] = FLAGS[index]
    row, = check_requirements(inventory((MEMBER, True)), report)
    assert row["result"] == "fail" and row["counts"] == {"pass": 0, "fail": 1, "unknown": 0}
    contexts = row["trials"][0]["graph_contexts"]
    assert [context["result"] for context in contexts] == ["fail", "unknown"]


def test_malformed_per_unit_grid_is_rejected_not_treated_as_a_waiver():
    report = candidate()
    report["checks"]["layout"]["unit_flag_sets"] = {"code/extra.cc": deepcopy(FLAGS)}
    with pytest.raises(ValueError, match="exactly match declared units"):
        check_requirements(inventory((MEMBER, True)), report)


def test_reviewed_replacement_requires_its_own_complete_measurements():
    old = replace(MEMBER, offset=12)
    facts = inventory((old, False), (MEMBER, True))
    known = {row["interpretation_id"]: row for row in facts["interpretations"]}
    review = {"kind": "review", "author": "worker", "reviewer": "independent",
              "snapshot": "b" * 64, "decision": "approve", "feedback": "Correct offset from original accesses",
              "evidence": ["source:original accesses"], "supersessions": [
                  {"previous": old.interpretation_id, "replacement": MEMBER.interpretation_id,
                   "evidence": ["source:original accesses"], "reason": "Old offset counted from wrong base"}]}
    rows = check_requirements(facts, candidate())
    assert declared_outcome(rows, known)[0] == Result.UNKNOWN
    assert declared_outcome(rows, known, review={**review, "supersessions": []})[0] == Result.UNKNOWN
    assert declared_outcome(rows, known, review=review)[0] == Result.PASS
    assert structural_outcome(rows)[0] == Result.UNKNOWN  # Whole-model proof is still separate.
    for results, expected in ((("fail", "pass"), Result.FAIL), (("unknown", "pass"), Result.UNKNOWN)):
        rows = check_requirements(facts, candidate(results=results))
        assert declared_outcome(rows, known, review=review)[0] == expected


def test_retracting_every_interpretation_does_not_prove_a_replacement():
    facts = inventory((MEMBER, False))
    known = {row["interpretation_id"]: row for row in facts["interpretations"]}
    rows = check_requirements(facts, candidate())
    review = {"kind": "review", "author": "worker", "reviewer": "independent",
              "snapshot": "b" * 64, "decision": "approve", "feedback": "Wrong model",
              "evidence": ["source:contradiction"], "retractions": [
                  {"interpretation": MEMBER.interpretation_id, "evidence": ["source:contradiction"],
                   "reason": "Not a direct member"}]}
    assert declared_outcome(rows, known, review=review)[0] == Result.UNKNOWN


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


def generated_candidate():
    binding, objects, image = fixture()
    objects["src/caller.cpp"] = SimpleNamespace(symbols={}, sections=())
    _, measured = check_generated((binding,), objects, image, {"_method": 0x402000})
    report = {"checks": {"binary": {"translation_units": list(objects), "flag_sets": deepcopy(FLAGS),
              "generated_units": list(objects),
              "generated_tables": [{**row, "flags": flags} for row in measured for flags in FLAGS],
              "generated_coverage": [{**row, "flags": flags} for row in generated_coverage((binding,), objects)
                                     for flags in FLAGS]}}}
    return binding, report


def test_generated_nonemission_covers_a_compiled_tu_without_claiming_table_content():
    binding, report = generated_candidate()
    row, = check_requirements(inventory((binding, True)), report)
    assert row["result"] == declared_outcome([row], {})[0] == "pass"
    assert row["counts"] == {"pass": 4, "fail": 0, "unknown": 0}
    assert sum(trial.get("coverage_only", False) for trial in row["trials"]) == 2
    assert structural_outcome([row])[0] == "unknown"


@pytest.mark.parametrize("missing", ["coverage", "trial", "failed_compile", "failed_dependency", "definition",
                                     "definition_trial", "compilation_grid"])
def test_generated_absence_never_waives_a_missing_compile_or_all_real_definitions(missing):
    binding, report = generated_candidate()
    binary = report["checks"]["binary"]
    if missing == "coverage": binary.pop("generated_coverage")
    elif missing == "trial": binary["generated_coverage"].pop()
    elif missing == "failed_compile": binary["translation_units"].append("src/failed.cpp")
    elif missing == "failed_dependency": binary["generated_units"].append("src/failed_dependency.cpp")
    elif missing == "compilation_grid": binary.pop("generated_units")
    else:
        absent = deepcopy(binary["generated_coverage"])
        for row in absent:
            row["unit"] = "src/widget.cpp"
        if missing == "definition":
            binary["generated_tables"] = []
            binary["generated_coverage"].extend(absent)
        else:
            binary["generated_tables"].pop()
            binary["generated_coverage"].append(absent[1])
    row, = check_requirements(inventory((binding, True)), report)
    assert row["result"] == declared_outcome([row], {})[0] == "unknown"


@pytest.mark.parametrize("damage", ["symbol", "basis", "records", "common", "weak", "boolean",
                                   "flags", "unit", "duplicate", "contradiction", "kind"])
def test_generated_absence_requires_unambiguous_interpretation_and_object_evidence(damage):
    binding, report = generated_candidate()
    binary = report["checks"]["binary"]
    row = binary["generated_coverage"][0]
    if damage == "symbol": row["witness"]["symbol"] = "??_7Other@@6B@"
    elif damage == "basis": row["witness"]["basis"] = "source_search"
    elif damage == "records": row["witness"].pop("records")
    elif damage in ("common", "weak", "boolean"):
        row["witness"]["records"] = [{"storage": 105 if damage == "weak" else 2, "section": 0,
                                      "value": 4 if damage == "common" else False if damage == "boolean" else 0,
                                      "type": 0}]
    elif damage == "flags": row["flags"] = ["not_requested"]
    elif damage == "unit": row["unit"] = "src/not_compiled.cpp"
    elif damage == "duplicate": binary["generated_coverage"].append(deepcopy(row))
    elif damage == "contradiction": row["unit"] = "src/widget.cpp"
    else: row["interpretation_id"] = MEMBER.interpretation_id
    with pytest.raises(ValueError, match="(absence|both measured)"):
        check_requirements(inventory((binding, True), (MEMBER, True)), report)


def test_unsupported_generated_dependency_is_not_hidden_by_a_passing_owner():
    binding, report = generated_candidate()
    row = deepcopy(report["checks"]["binary"]["generated_coverage"][0])
    row.update(unit="src/dependency.cpp", observation="unsupported_symbol")
    row["witness"]["records"] = [{"storage": 105, "section": 0, "value": 0, "type": 0}]
    report["checks"]["binary"]["generated_units"].append("src/dependency.cpp")
    report["checks"]["binary"]["generated_coverage"].append(row)
    requirement, = check_requirements(inventory((binding, True)), report)
    assert requirement["result"] == "unknown" and requirement["counts"]["unknown"] == 2


def test_nonemission_elsewhere_cannot_hide_an_incorrect_emitted_table():
    binding, report = generated_candidate()
    row = deepcopy(report["checks"]["binary"]["generated_tables"][0])
    row.update(unit="src/dependency.cpp", content="fail")
    report["checks"]["binary"]["generated_tables"].append(row)
    requirement, = check_requirements(inventory((binding, True)), report)
    assert requirement["counts"]["fail"] == 1 and declared_outcome([requirement], {})[0] == "fail"
