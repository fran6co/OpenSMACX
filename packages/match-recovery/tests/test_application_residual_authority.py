"""Application section coverage cannot borrow serialized task/build authority."""

from dataclasses import replace

import pytest

from match_recovery.application_residual import compare_application_residual
from test_generated_residual_authority import authorized


def test_live_task_rederivation_keeps_full_physical_obligations(authorized):
    graph, state = authorized
    report = compare_application_residual(graph)
    assert report["result"] == report["outside_scope_verification"] == "unknown"
    assert not report["scope_complete"] and not report["ready_for_collection"]
    assert report["commands"]["result"] == "pass", report
    assert set(report["scoped_code"]["physical_sections"]) == {"baseline", "current"}
    for version in ("baseline", "current"):
        assert report["scoped_code"]["physical_sections"][version]["enumeration"] == "pass"
    assert state["checks"] > 0


@pytest.mark.parametrize("replacement", ["copy", "report", "source", "authorization", "guard"])
def test_copied_or_replaced_graph_cannot_supply_scope(authorized, replacement):
    graph, _ = authorized
    changes = {
        "copy": {}, "report": {"_report": {"result": "pass"}},
        "source": {"_sources": {"baseline": {}, "current": {}}},
        "authorization": {"_authorization": {}}, "guard": {"check_current": lambda: None},
    }
    with pytest.raises(ValueError, match="original live native authority"):
        compare_application_residual(replace(graph, **changes[replacement]))


def test_saved_diagnostic_is_never_an_application_capability(authorized):
    graph, _ = authorized
    with pytest.raises(ValueError, match="original live native authority"):
        compare_application_residual(compare_application_residual(graph))


def test_expired_graph_refuses_even_when_its_files_still_exist(authorized):
    graph, state = authorized
    state["alive"] = False
    with pytest.raises(ValueError, match="expired"):
        compare_application_residual(graph)
