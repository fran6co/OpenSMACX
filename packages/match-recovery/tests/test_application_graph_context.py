"""Live task graph sessions retain both artifacts and immutable authority."""

from dataclasses import FrozenInstanceError

import pytest

from match_recovery import build_graph as native
from match_recovery.driver import working_state
from match_recovery.tasks import TaskStore
from test_task_build_graph import GRAPH, change_graph, contexts, graph_task, migration, project


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
