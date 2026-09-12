"""Reviewed generated authority reaches native graph and residual boundaries."""

from copy import deepcopy
from dataclasses import replace

import pytest

from match_recovery import build_graph as graph_native
from match_recovery import migration_check as migration_native
from match_recovery.tasks import TaskStore
from test_migration_check import generated_request, measure, migration, project
from test_task_build_graph import change_graph, contexts, graph_task
from test_preservation_graph_consumption import application
from test_preservation import Compiler, check, project as preservation_project


def authority(task, **changes):
    return {"migration_id": "startup", "author": task.author, "allowed": [],
            "evidence": ["Intentional scoped lifecycle migration"], "generated_changes": [{
                "entity_id": task.subjects[0].entity_id, "unit": "code/unit.cc", "version": "current",
                "symbol": "init", "offset": 0, "evidence": ["Exact source-owned generated occurrence"],
                **changes}]}


@pytest.mark.parametrize("changed_graph", [False, True])
def test_generated_graph_keeps_exact_authority_without_granting_linked_acceptance(graph_task, monkeypatch, changed_graph):
    root, task = graph_task
    if changed_graph:
        change_graph(root)
    calls, living, _ = contexts(monkeypatch)
    monkeypatch.setattr(graph_native, "compare_build_initializers", lambda *a, **k: {"result": "pass"})
    permission = authority(task)
    before = deepcopy(permission)
    with graph_native.application_graph(root, task.task_id, permission=permission) as session:
        assert session.authorization["permission"] == before
        permission["generated_changes"][0]["offset"] = 4
        copy = session.authorization
        copy["permission"]["generated_changes"][0]["offset"] = 8
        assert session.authorization["permission"] == before
        assert session.report["result"] == session.report["linked_effects"] == "unknown"
        session.check_current()
    assert len(calls) == 2 and living == []


@pytest.mark.parametrize("order", ["pass", "fail", "unknown", None])
@pytest.mark.parametrize("consistency", ["pass", "fail"])
def test_generated_graph_consumes_order_without_waiving_other_linked_effects(graph_task, monkeypatch, order, consistency):
    root, task = graph_task
    contexts(monkeypatch)
    permission = authority(task)
    calls = []

    def observe(old, new, **kwargs):
        calls.append(kwargs)
        return {"result": "fail", "input_consistency": consistency,
                **({"projected_order": {"result": order}} if order is not None else {})}

    monkeypatch.setattr(graph_native, "compare_build_initializers", observe)
    with graph_native.application_graph(root, task.task_id, permission=permission) as session:
        assert calls == [{"generated_changes": permission["generated_changes"]}]
        assert session.report["result"] == ("fail" if order == "fail" or consistency == "fail" else "unknown")
        assert session.report["linked_effects"] == "unknown"


@pytest.mark.parametrize("changes", [{"unit": "code/old.cc"}, {"unit": "code/unit.hpp"},
                                    {"unit": "code/unknown.cc"}])
def test_graph_requires_source_owned_exact_unit_before_private_build(graph_task, changes):
    root, task = graph_task
    with pytest.raises(ValueError, match="source anchor|product translation unit"):
        with graph_native.application_graph(root, task.task_id, permission=authority(task, **changes)):
            pytest.fail("unowned generated source entered live graph")


def test_changed_compiler_policy_still_refuses_generated_authority(graph_task):
    root, task = graph_task
    path = root / "recovery/build.json"
    path.write_text(path.read_text() + "\n")
    with pytest.raises(ValueError, match="unchanged compiler/build policy"):
        with graph_native.application_graph(root, task.task_id, permission=authority(task)):
            pytest.fail("generated permission waived compiler policy")


def test_failed_generated_graph_cannot_inherit_unchanged_policy_acceptance(migration, monkeypatch):
    calls = []
    payload = generated_request(migration)

    def refused(root, task, **kwargs):
        calls.append(kwargs)
        raise ValueError("synthetic missing native startup evidence")

    monkeypatch.setattr(migration_native, "application_graph", refused)
    monkeypatch.setattr(migration_native, "check_preservation", lambda *a, **k: {"result": "pass"})
    report = measure(migration, payload)
    assert calls == [{"integration": None, "permission": payload["permission"]}]
    assert report["build"] == "pass" and report["outside_scope"] == "unknown"
    assert any(row["check"] == "build-graph" and "missing native startup" in row["evidence"][0]
               for row in report["diagnostics"])


@pytest.mark.parametrize("diagnostic", [False, True])
def test_preservation_permission_requires_live_graph_and_complete_controls(preservation_project, diagnostic):
    root = preservation_project
    task = TaskStore(root).read("owner")
    compiler = Compiler(root)
    with pytest.raises(ValueError, match="live application graph and complete controls"):
        check(root, compiler=compiler, permission=authority(task, unit="code/owner.cc"), diagnostic=diagnostic)
    assert compiler.calls == []


def test_preservation_cannot_use_a_graph_with_different_generated_authority(preservation_project):
    root = preservation_project
    task = TaskStore(root).read("owner")
    compiler = Compiler(root)
    with pytest.raises(ValueError, match="differs from preservation task or authorization"):
        check(root, compiler=compiler, application=application(root),
              permission=authority(task, unit="code/owner.cc"))
    assert compiler.calls == []


def test_graph_authority_is_not_an_implicit_preservation_permission(preservation_project):
    root = preservation_project
    task = TaskStore(root).read("owner")
    graph = application(root)
    authorization = graph.authorization
    authorization["permission"] = authority(task, unit="code/owner.cc")
    compiler = Compiler(root)
    with pytest.raises(ValueError, match="differs from preservation task or authorization"):
        check(root, compiler=compiler, application=replace(graph, _authorization=authorization))
    assert compiler.calls == []
