"""Synthetic explanation controls; no native compiler or acceptance proof."""

from copy import deepcopy

import pytest

from match_recovery.linked_code_residual import compare_residual, normalize_residual
from match_recovery.linked_residual_diagnostics import MAX_DIAGNOSTIC_BYTES, MAX_DIFFERENCES
from match_recovery.model import canonical, digest


def normalized(nodes, **extra):
    value = {"nodes": nodes, **extra}
    return {"result": "pass", "canonical": value, "fingerprint": digest(value)}


def node(identity="unit.cpp:_f", **body):
    return {"identity": identity, "body": body}


def check(old, new):
    snapshot = deepcopy((old, new))
    report = compare_residual(old, new)
    assert (old, new) == snapshot
    assert report["equal"] is (canonical(old["canonical"]) == canonical(new["canonical"]))
    assert report["result"] == ("pass" if report["equal"] else "fail")
    assert report["remaining_linked_effects"] == "unknown"
    assert report["ready_for_collection"] is False
    diagnostics = report["diagnostics"]
    assert diagnostics["advisory_only"] is True
    assert (diagnostics["difference_count"] == 0) is report["equal"]
    assert diagnostics["omitted_difference_count"] == (
        diagnostics["difference_count"] - len(diagnostics["differences"]))
    assert diagnostics["truncated"] is (diagnostics["omitted_difference_count"] > 0)
    assert len(diagnostics["differences"]) <= MAX_DIFFERENCES
    assert len(canonical(diagnostics)) <= MAX_DIAGNOSTIC_BYTES
    return diagnostics


def test_equal_values_ignore_dictionary_insertion_order():
    old = normalized([node(size=8, flags=32)])
    new = normalized([{"body": {"flags": 32, "size": 8}, "identity": "unit.cpp:_f"}])
    assert check(old, new)["differences"] == []


def test_equal_json_scalar_subclass_does_not_invent_a_difference():
    class Count(int):
        pass
    assert check(normalized([node(size=Count(1))]), normalized([node(size=1)]))["difference_count"] == 0


def test_changed_producer_and_nested_field_are_identified():
    identity = {"unit": "src/unit.cpp", "symbol": "_f"}
    old = normalized([node(identity, edges=[{"target": {"addend": 0}}])])
    new = normalized([node(identity, edges=[{"target": {"addend": 4}}])])
    assert check(old, new)["differences"] == [{
        "kind": "value_changed", "identity": {"value": identity},
        "path": {"value": ["body", "edges", 0, "target", "addend"]}}]


@pytest.mark.parametrize("old,new,kind", [
    (True, 1, "type_changed"), (1, 1.0, "type_changed"),
    (0.0, -0.0, "value_changed"), (None, {}, "type_changed"),
    ({"a": 1}, {}, "removed"), ({}, {"a": 1}, "added"),
    ([1], [], "removed"), ([], [1], "added")])
def test_json_types_and_missing_subtrees_count_once(old, new, kind):
    report = check(normalized([node(value=old)]), normalized([node(value=new)]))
    assert report["difference_count"] == 1
    assert report["differences"][0]["kind"] == kind


def test_canonical_node_reordering_is_reported_without_changing_comparison():
    before = [node("a", size=8), node("b", size=9)]
    report = check(normalized(before), normalized(list(reversed(before))))
    assert report["differences"] == [{"kind": "order_changed", "path": {"value": ["nodes"]}}]


def test_observed_node_reordering_still_normalizes_equal():
    def graph(values):
        return {"result": "pass", "gaps": [], "canonical_nodes": values,
                "nodes": [{"id": i, "result": "pass"} for i in range(len(values))], "roots": [0, 1]}
    old = normalize_residual(graph([{"size": 8}, {"size": 9}]), {0: "a", 1: "b"}, set())
    new = normalize_residual(graph([{"size": 9}, {"size": 8}]), {0: "b", 1: "a"}, set())
    assert check(old, new)["difference_count"] == 0


def test_missing_node_does_not_shift_matched_node_field_paths():
    old = normalized([node("a", size=1), node("b", size=2), node("c", size=3)])
    new = normalized([node("b", size=2), node("c", size=4), node("d", size=5)])
    report = check(old, new)
    assert report["difference_count"] == 3
    assert [(row["identity"]["value"], row["kind"]) for row in report["differences"]] == [
        ("a", "node_removed"), ("c", "value_changed"), ("d", "node_added")]


def test_late_difference_after_many_equal_nodes_is_not_missed():
    old = normalized([node(f"node{i:04d}", value=0) for i in range(500)])
    new = deepcopy(old)
    new["canonical"]["nodes"][-1]["body"]["value"] = 1
    new["fingerprint"] = digest(new["canonical"])
    report = check(old, new)
    assert report["difference_count"] == 1
    assert report["differences"][0]["identity"] == {"value": "node0499"}


def test_entry_bound_does_not_bound_difference_count_or_mutate_inputs():
    old = normalized([node(values=[0] * 400)])
    new = normalized([node(values=[1] * 400)])
    report = check(old, new)
    assert report["difference_count"] == 400
    assert len(report["differences"]) == MAX_DIFFERENCES
    assert report == check(old, new)


def test_long_identities_and_paths_have_bounded_detached_previews():
    identity = {"unit": "unicode_\u2603" * 1000, "symbols": ["f"]}
    field = "very_long_\u2603" * 1000
    old = normalized([node(identity, **{field: [0] * 100})])
    new = normalized([node(identity, **{field: [1] * 100})])
    report = check(old, new)
    assert report["difference_count"] == 100
    row = report["differences"][0]
    assert row["identity"]["truncated"] is row["path"]["truncated"] is True
    assert row["identity"]["sha256"] == digest(identity)
    assert len(report["differences"]) < MAX_DIFFERENCES


def test_identity_in_report_does_not_alias_canonical_value():
    identity = {"unit": "f.cpp", "symbols": ["f"]}
    old, new = normalized([node(identity, size=1)]), normalized([node(identity, size=2)])
    check(old, new)["differences"][0]["identity"]["value"]["symbols"].append("other")
    assert identity["symbols"] == ["f"]


@pytest.mark.parametrize("old,new", [
    ({}, {"nodes": []}), ({"nodes": [1]}, {"nodes": [2]}),
    ({"nodes": [node("a"), node("a")]}, {"nodes": [node("a"), node("b")]}),
    ({"nodes": [], "extra": 1}, {"nodes": [], "extra": 2}),
    ({"nodes": [node(None, size=1)]}, {"nodes": [node(None, size=2)]})])
def test_existing_nonstandard_canonical_inputs_remain_comparable(old, new):
    wrap = lambda value: {"result": "pass", "canonical": value, "fingerprint": digest(value)}
    assert check(wrap(old), wrap(new))["difference_count"] > 0


def test_stale_fingerprint_still_refuses_before_diagnostics():
    old = normalized([node(size=1)])
    new = deepcopy(old)
    new["canonical"]["nodes"][0]["body"]["size"] = 2
    with pytest.raises(ValueError, match="changed after normalization"):
        compare_residual(old, new)
