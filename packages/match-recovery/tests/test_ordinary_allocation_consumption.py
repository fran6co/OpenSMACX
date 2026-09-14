"""Physical allocation observations and explicit application failure routing."""

import pytest

from match_recovery import application_residual, build_graph, generated_code_residual
from test_generated_residual_authority import authorized
from test_ordinary_allocation_coverage import fixture
from test_task_build_graph import contexts, graph_task, migration, project


@pytest.mark.parametrize("damage", [None, "corrupt", "overlap", "missing_sc"])
def test_application_path_consumes_both_native_allocation_domains(tmp_path, damage):
    old = fixture(tmp_path / "old")
    new = fixture(tmp_path / "new", **({damage: True} if damage else {}))
    report = generated_code_residual._compare_contexts(old, new, [],
                            application_scope={"baseline": {}, "current": {}})
    assert set(report["ordinary_allocations"]) == {"baseline", "current"}, report
    assert report["ordinary_allocations"]["baseline"]["result"] == "pass"
    current = report["ordinary_allocations"]["current"]
    assert current["result"] == ("fail" if damage in ("corrupt", "overlap") else
                                  "unknown" if damage else "pass"), report
    if damage in ("corrupt", "overlap"):
        assert report["result"] == "fail"
        assert "storage_residual" not in report  # Earlier physical evidence must survive.
    assert report["remaining_linked_effects"] == "unknown"


@pytest.mark.parametrize("dimension", ["input_consistency", "allocation_isolation"])
@pytest.mark.parametrize("outcome", ["pass", "fail", "unknown"])
def test_application_wrapper_propagates_counterexamples_without_granting_coverage(
        authorized, monkeypatch, dimension, outcome):
    # Routing control only. Native physical observations are exercised above;
    # this fixture retains actual task/source authority while controlling the
    # private dimension result, never supplying an external acceptance receipt.
    graph, _ = authorized
    monkeypatch.setattr(generated_code_residual, "_compare_contexts", lambda *a, **k: {
        "result": "fail", "ordinary_allocations": {"current": {dimension: outcome}}})
    report = application_residual.compare_application_residual(graph)
    assert report["result"] == ("fail" if outcome == "fail" else "unknown")
    assert report["outside_scope_verification"] == "unknown" and not report["scope_complete"]


@pytest.mark.parametrize("dimension", ["input_consistency", "allocation_isolation"])
@pytest.mark.parametrize("outcome", ["pass", "fail", "unknown"])
def test_build_graph_consumes_allocation_counterexamples_but_not_positive_scope_grants(
        graph_task, monkeypatch, dimension, outcome):
    root, task = graph_task
    _, living, _ = contexts(monkeypatch)
    monkeypatch.setattr(build_graph, "compare_build_initializers", lambda *a, **k: {"result": "pass"})

    def observe(session):
        assert living == [0, 1]
        session.check_current()
        return {"result": outcome, dimension: outcome}

    monkeypatch.setattr(application_residual, "compare_application_residual", observe)
    with build_graph.application_graph(root, task.task_id) as session:
        assert session.report["result"] == ("fail" if outcome == "fail" else "unknown")
        assert session.report["linked_effects"] == "unknown"
