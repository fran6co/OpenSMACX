"""Live task graph sessions retain both artifacts and immutable authority."""

from dataclasses import FrozenInstanceError

import pytest

from match_recovery import build_graph as native
from match_recovery.driver import working_state
from match_recovery.tasks import TaskStore
from test_task_build_graph import GRAPH, change_graph, contexts, graph_task, migration, project


@pytest.mark.parametrize("initializer_result", ["pass", "fail", "unknown"])
def test_ordinary_scoped_body_uses_original_paired_sources_without_generated_waiver(
        graph_task, monkeypatch, initializer_result):
    # Real task/history/policy selection with controlled private build contexts;
    # this exercises eligibility, not a live compiler or completed linked proof.
    root, task = graph_task
    source = root / "code/unit.cc"
    original = source.read_bytes()
    candidate = original.replace(b"void f() {}", b"void f() { return; }")
    assert candidate != original
    source.write_bytes(candidate)
    graph = (root / GRAPH).read_bytes()
    before = working_state(root)
    calls, living, _ = contexts(monkeypatch)
    observations = []

    def initializers(old, new, **kwargs):
        assert living == [0, 1] and not kwargs
        observations.append(True)
        return {"result": initializer_result, "remaining_linked_effects": "unknown"}

    monkeypatch.setattr(native, "compare_build_initializers", initializers)
    monkeypatch.setattr(native, "generated_sources", lambda *a, **k: pytest.fail("unexpected generated waiver"))
    from match_recovery import generated_code_residual
    monkeypatch.setattr(generated_code_residual, "compare_generated_code_residual",
                        lambda *a, **k: pytest.fail("unexpected generated residual exception"))
    with native.application_graph(root, task.task_id) as session:
        assert session.baseline_revision == task.base_revision
        assert session.source_versions["baseline"]["code/unit.cc"] == original
        assert session.source_versions["current"]["code/unit.cc"] == candidate
        assert [call["versioned"] for call in calls] == [{GRAPH: graph}, {GRAPH: graph}]
        assert session.authorization == {"task": task.to_dict(),
                                         "decisions": TaskStore(root).decisions(task.task_id), "integration": None}
        assert session.report["graph_inputs"] == []
        assert session.report["source_changes"] == ["code/unit.cc"]
        assert session.report["result"] == session.report["linked_effects"] == "unknown"
        assert "generated_code_residual" not in session.report
        session.check_current()
    assert len(calls) == 2 and observations == [True] and living == []
    assert working_state(root) == before


@pytest.mark.parametrize("name", ["recovery/compile.py", "recovery/build.json", "recovery/project.json"])
def test_ordinary_body_route_still_refuses_changed_compiler_policy(graph_task, name):
    root, task = graph_task
    source = root / "code/unit.cc"
    source.write_bytes(source.read_bytes().replace(b"void f() {}", b"void f() { return; }"))
    path = root / name
    path.write_bytes(path.read_bytes() + b"\n ")
    with pytest.raises(ValueError, match="unchanged compiler/build policy"):
        native.check_build_graph(root, task.task_id)


def test_unchanged_policy_does_not_authorize_outside_task_source(graph_task):
    root, task = graph_task
    (root / "code/unowned.cc").write_bytes(b"void unrelated() {}\n")
    with pytest.raises(ValueError, match="source changes exceed authorized editable"):
        native.check_build_graph(root, task.task_id)


def test_unchanged_policy_does_not_turn_matching_permission_into_generated_permission(graph_task):
    root, task = graph_task
    permission = {"migration_id": "ordinary-body", "author": task.author,
                  "evidence": ["Scoped body review, not generated-code authorization"], "allowed": []}
    with pytest.raises(ValueError, match="explicit generated changes"):
        with native.application_graph(root, task.task_id, permission=permission):
            pytest.fail("matching permission must not grant a generated-code exception")


def prepared(graph_task, monkeypatch):
    root, task = graph_task
    change_graph(root)
    (root / "code/old.cc").unlink()
    (root / "code/new.cc").write_bytes(b"current new source\n")
    calls, living, comparisons = contexts(monkeypatch)
    monkeypatch.setattr(native, "compare_build_initializers", lambda *a: {
        "result": "pass", "remaining_linked_effects": "unknown"})
    return root, task, calls, living, comparisons


def test_live_graph_session_keeps_both_versions_and_expires_without_rebuilding(graph_task, monkeypatch):
    root, task, calls, living, comparisons = prepared(graph_task, monkeypatch)
    before = working_state(root)
    with native.application_graph(root, task.task_id) as session:
        assert session.root == root and session.task_name == task.task_id
        assert session.baseline_revision == task.base_revision and session.roots == ("code",)
        assert session.baseline is calls[0]["context"] and session.current is calls[1]["context"]
        assert living == [0, 1] and len(calls) == 2 and len(comparisons) == 1
        assert session.source_versions == {"baseline": calls[0]["files"], "current": calls[1]["files"]}
        assert "code/old.cc" in session.source_versions["baseline"]
        assert "code/old.cc" not in session.source_versions["current"]
        assert session.authorization == {"task": task.to_dict(), "decisions": TaskStore(root).decisions(task.task_id),
                                         "integration": None}
        session.check_current()
        assert session.report["result"] == session.report["linked_effects"] == "unknown"
        assert session.report["initializers"]["result"] == "pass"
    assert living == [] and len(calls) == 2
    assert not session.baseline.root.exists() and not session.current.root.exists()
    with pytest.raises(ValueError, match="expired"):
        session.check_current()
    assert working_state(root) == before


def test_session_report_sources_and_authority_are_defensive_copies(graph_task, monkeypatch):
    root, task, calls, _, _ = prepared(graph_task, monkeypatch)
    with native.application_graph(root, task.task_id) as session:
        report, sources, authorization = session.report, session.source_versions, session.authorization
        report["commands"]["result"] = "pass"
        report["linked_effects"] = report["result"] = "pass"
        report["current_build"].clear()
        sources["baseline"].clear()
        sources["current"]["code/forged.cc"] = b"forged source"
        authorization["task"]["base_revision"] = "0" * 40
        authorization["integration"] = {"target_revision": "0" * 40}
        assert session.report["result"] == "unknown" and session.report["current_build"] == calls[1]["context"].report
        assert session.source_versions["baseline"] and "code/forged.cc" not in session.source_versions["current"]
        assert session.authorization["task"]["base_revision"] == task.base_revision
        assert session.authorization["integration"] is None
        with pytest.raises(FrozenInstanceError):
            session.baseline_revision = "0" * 40
        session.check_current()


@pytest.mark.parametrize("damage", ["source", "graph", "task", "build", "measurement", "pin", "ignored_source"])
@pytest.mark.parametrize("explicit", [False, True])
def test_original_input_mutations_refuse_on_demand_or_exit_and_preserve_work(graph_task, monkeypatch, damage, explicit):
    root, task, _, living, _ = prepared(graph_task, monkeypatch)
    if damage == "ignored_source":
        ignore = root / ".gitignore"
        ignore.write_text(ignore.read_text() + "\ncode/session-ignored.bin\n")
    target = root / {"source": "code/unit.cc", "graph": GRAPH,
                     "task": f"recovery/tasks/{task.task_id}.json", "build": "recovery/build.json",
                     "measurement": "recovery/project.json", "pin": "code/PIN",
                     "ignored_source": "code/session-ignored.bin"}[damage]
    with pytest.raises(ValueError):
        with native.application_graph(root, task.task_id) as session:
            target.write_bytes((target.read_bytes() if target.exists() else b"") + b"\n ")
            if explicit:
                session.check_current()
    assert living == [] and target.read_bytes().endswith(b"\n ")
    assert TaskStore(root).read(task.task_id).base_revision == task.base_revision
    with pytest.raises(ValueError, match="expired"):
        session.check_current()


@pytest.mark.parametrize("failed_version", ["baseline", "current"])
@pytest.mark.parametrize("explicit", [False, True])
def test_both_underlying_artifact_guards_run_even_when_one_refuses(graph_task, monkeypatch, failed_version, explicit):
    root, task, _, living, _ = prepared(graph_task, monkeypatch)
    calls = []
    with pytest.raises(ValueError, match="artifact changed"):
        with native.application_graph(root, task.task_id) as session:
            for name in ("baseline", "current"):
                context = getattr(session, name)
                original = context.check_current
                def check(name=name, original=original):
                    original()
                    calls.append(name)
                    if name == failed_version:
                        raise ValueError("artifact changed")
                context.check_current = check
            if explicit:
                with pytest.raises(ValueError, match="artifact changed"):
                    session.check_current()
                assert calls == ["baseline", "current"]
    assert len(calls) >= 2 * (2 if explicit else 1)
    assert calls == ["baseline", "current"] * (len(calls) // 2)
    assert living == []
    with pytest.raises(ValueError, match="expired"):
        session.check_current()


def test_consumer_exception_closes_both_builds_and_expires_session(graph_task, monkeypatch):
    root, task, _, living, _ = prepared(graph_task, monkeypatch)
    with pytest.raises(RuntimeError, match="consumer failed"):
        with native.application_graph(root, task.task_id) as session:
            raise RuntimeError("consumer failed")
    assert living == []
    with pytest.raises(ValueError, match="expired"):
        session.check_current()


def test_reporting_wrapper_uses_one_session_and_returns_only_defensive_report(graph_task, monkeypatch):
    root, task, calls, living, comparisons = prepared(graph_task, monkeypatch)
    report = native.check_build_graph(root, task.task_id)
    assert len(calls) == 2 and living == [] and len(comparisons) == 1
    assert report["result"] == "unknown"
    report["current_build"].clear()
    assert calls[1]["context"].report["result"] == "pass"


def test_decision_history_is_captured_defensively_and_new_decisions_invalidate_session(graph_task, monkeypatch):
    root, task, _, living, _ = prepared(graph_task, monkeypatch)
    store = TaskStore(root)
    store.expand(task.task_id, request_id="initial-graph-context-scope", author=task.author,
                 subjects=(), editable_files=("code/extra.cc",), evidence=("independent coordinator decision",),
                 expected_revision=0)
    original = store.decisions(task.task_id)
    with pytest.raises(ValueError, match="changed"):
        with native.application_graph(root, task.task_id) as session:
            assert session.authorization["decisions"] == original
            copied = session.authorization
            copied["decisions"][0]["editable_files"].append("code/forged.cc")
            assert session.authorization["decisions"] == original
            store.discover(task.task_id, request_id="new-graph-context-discovery", author="worker",
                           hypothesis="new ownership information", affects=(task.subjects[0].entity_id,),
                           evidence=("fresh source observation",), expected_revision=1)
            session.check_current()
    assert len(store.decisions(task.task_id)) == 2 and living == []
    with pytest.raises(ValueError, match="expired"):
        session.check_current()
