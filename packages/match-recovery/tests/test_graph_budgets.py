"""Explicit larger capacity preserves every node; exhaustion never truncates."""

from copy import deepcopy

import pytest

from match_recovery.graph_limits import DEFAULT_NODES, MAX_NODES, node_limit
from match_recovery.linked_code_residual import normalize_residual, compare_residual
from match_recovery.linked_initializers import observe_build_initializers, compare_build_initializers
from test_linked_code_residual import body, edge, graph
from test_linked_initializers import build, update_artifact
from test_storage_section_seeds import paired, unreferenced, observed


@pytest.mark.parametrize("value", [True, False, 0, -1, 100001, 4096.0, "4096", None])
def test_budget_requires_exact_bounded_integer(value):
    with pytest.raises(ValueError):
        node_limit(value)
    with pytest.raises(ValueError):
        normalize_residual(graph([]), {}, set(), max_nodes=value)


def test_large_pure_residual_retains_every_node_edge_and_last_body_damage():
    count = DEFAULT_NODES + 3
    identities = {index: {"unit": f"unit{index}.cpp", "symbol": "f"} for index in range(count)}
    bodies = [body(index, [edge({"node": (index + 1) % count})]) for index in range(count)]
    value = graph(bodies, roots=[0])
    with pytest.raises(ValueError, match="bounded"):
        normalize_residual(value, identities, set())
    before = normalize_residual(value, identities, set(), max_nodes=count)
    assert len(before["canonical"]["nodes"]) == count
    assert before["result"] == "pass" and before["remaining_linked_effects"] == "unknown"
    assert not before["ready_for_collection"]
    assert compare_residual(before, normalize_residual(value, identities, set(), max_nodes=MAX_NODES))["result"] == "pass"
    changed = deepcopy(value)
    changed["canonical_nodes"][-1]["code_sha256"] = "last-node damage"
    assert compare_residual(before, normalize_residual(changed, identities, set(), max_nodes=count))["result"] == "fail"
    changed["canonical_nodes"][-1]["edges"][0]["target"]["node"] = count
    with pytest.raises(ValueError, match="unknown node"):
        normalize_residual(changed, identities, set(), max_nodes=count)


def two_nodes(root):
    return build(root, (b"\xe8" + bytes(4) + b"\xc3",), references=("_dep",))


def test_tiny_native_initializer_propagates_budget_to_reachable_helper(tmp_path):
    context = two_nodes(tmp_path / "native")
    small = observe_build_initializers(context, max_functions=1)
    assert small["result"] == "pass"  # Actual table still verified.
    assert small["code_dependencies"]["result"] == "unknown"
    assert any("bound" in row["reason"] for row in small["code_dependencies"]["gaps"])
    exact = observe_build_initializers(context, max_functions=2)
    assert exact["code_dependencies"]["result"] == "pass", exact
    assert len(exact["code_dependencies"]["nodes"]) == 2


def test_paired_initializer_budget_reaches_both_native_builds(tmp_path):
    old, new = two_nodes(tmp_path / "old"), two_nodes(tmp_path / "new")
    small = compare_build_initializers(old, new, max_functions=1)
    assert small["code_dependencies"]["result"] == "unknown"
    exact = compare_build_initializers(old, new, max_functions=2)
    assert exact["code_dependencies"]["result"] == "pass", exact
    for version in ("baseline", "current"):
        assert len(exact[version]["code_dependencies"]["nodes"]) == 2


def test_empty_native_initializer_still_validates_explicit_budget(tmp_path):
    context = build(tmp_path / "native", ())
    mapping = (context.directory / "app.map").read_text()
    update_artifact(context, "app.map", "\n".join(row for row in mapping.split("\n") if ".CRT$XCU" not in row).encode())
    assert observe_build_initializers(context, max_functions=True)["result"] == "unknown"
    report = observe_build_initializers(context, max_functions=MAX_NODES)
    assert report["result"] == report["code_dependencies"]["result"] == "pass", report
    assert report["code_dependencies"]["nodes"] == []


def test_native_storage_accepts_explicit_larger_budget_without_extra_coverage_claim(tmp_path):
    with paired(unreferenced(tmp_path / "old"), unreferenced(tmp_path / "new"), limit=MAX_NODES) as (closure, live):
        assert closure.limit == MAX_NODES
        closure.require_sections("baseline", [{"owner": "unit0.obj", "section": 1}])
        result = observed(closure, live)
        assert result["result"] == "pass" and result["sections"] == 1
        assert result["remaining_linked_effects"] == result["allocation_isolation"] == "unknown"
