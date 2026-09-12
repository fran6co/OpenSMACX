"""Task graph orchestration controls; synthetic builds are not recovery proof."""

from contextlib import contextmanager
from copy import deepcopy
from dataclasses import replace
import json
from pathlib import Path
from tempfile import TemporaryDirectory
from types import SimpleNamespace

import pytest

from match_recovery import build_graph as native
from match_recovery.driver import working_state
from match_recovery.tasks import TaskStore
from test_migration_check import migration, project
from test_task_source import git


GRAPH = "CMakeLists.txt"


@pytest.fixture
def graph_task(migration, monkeypatch):
    root = migration
    (root / GRAPH).write_bytes(b"add_executable(app code/unit.cc code/old.cc)\n")
    (root / "code/old.cc").write_bytes(b"old source\n")
    path = root / "recovery/build.json"
    config = json.loads(path.read_text())
    config.update(versioned_inputs=[GRAPH], compile_link={
        "database": "compile_commands.json", "source_extensions": [".cc"],
        "directory": "observations", "suffix": ".compile.json", "link_files": "link.json",
        "link_log": "link.log", "command_stages": {"compile": "compile", "link": "link"}})
    config["outputs"] += [
        {"path": "compile_commands.json", "format": "compilation-database"},
        {"path": "link.json", "format": "link-input-files"},
        {"path": "link.log", "format": "msvc-link-log"}]
    path.write_text(json.dumps(config))
    git(root, "add", ".")
    git(root, "commit", "-m", "Declare graph before independent graph task")
    store = TaskStore(root)
    task = replace(store.read("owner"), task_id="graph-task", base_revision=git(root, "rev-parse", "HEAD"),
                   editable_files=(GRAPH, "code/unit.cc", "code/unit.hpp", "code/old.cc", "code/new.cc"))
    store.create(task)
    git(root, "add", ".")
    git(root, "commit", "-m", "Authorize independent graph task")
    monkeypatch.setattr(native, "application_build", lambda *a, **k: pytest.fail("unexpected private build"))
    monkeypatch.setattr(native, "compare_build_commands", lambda *a, **k: pytest.fail("unexpected command comparison"))
    return root, task


def change_graph(root):
    (root / GRAPH).write_bytes(b"add_executable(app code/unit.cc code/new.cc)\n")


def new_task(root, task, **changes):
    replacement = replace(task, task_id=task.task_id + "-variant", **changes)
    TaskStore(root).create(replacement)
    return replacement


def contexts(monkeypatch, *, outcome="pass", mutate=None, compare_mutate=None):
    calls, living, comparisons = [], [], []

    @contextmanager
    def build(root, roots, files, *, versioned_files):
        index = len(calls)
        with TemporaryDirectory(prefix="task-graph-test-") as temporary:
            private = Path(temporary)
            for name, data in {**files, **versioned_files}.items():
                path = private / name
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(data)
            directory = private / "build/product"
            directory.mkdir(parents=True)
            report = {"result": "pass", "directory": "build/product", "test_version": index}
            def fresh():
                assert index in living and directory.is_dir()
            context = SimpleNamespace(root=private, directory=directory, report=report, check_current=fresh)
            calls.append({"root": root, "roots": roots, "files": deepcopy(files),
                          "versioned": deepcopy(versioned_files), "context": context})
            living.append(index)
            try:
                if mutate is not None:
                    mutate(index)
                yield context
            finally:
                living.remove(index)

    def compare(old, new, *, editable):
        assert living == [0, 1]
        old.check_current()
        new.check_current()
        assert old is calls[0]["context"] and new is calls[1]["context"]
        for call in calls:
            for name, data in {**call["files"], **call["versioned"]}.items():
                assert (call["context"].root / name).read_bytes() == data
        comparisons.append(tuple(editable))
        if compare_mutate is not None:
            compare_mutate()
        return {"result": outcome, "test_evidence": "synthetic command comparison only"}

    monkeypatch.setattr(native, "application_build", build)
    monkeypatch.setattr(native, "compare_build_commands", compare)
    return calls, living, comparisons


@pytest.mark.parametrize("outcome", ["pass", "fail", "unknown"])
def test_exact_source_graph_versions_live_together_without_acceptance(graph_task, monkeypatch, outcome):
    root, task = graph_task
    stored = TaskStore(root).read("owner")
    prior = {path.relative_to(root).as_posix(): path.read_bytes() for path in (root / "code").rglob("*") if path.is_file()}
    old_graph = (root / GRAPH).read_bytes()
    change_graph(root)
    (root / "code/old.cc").unlink()
    (root / "code/new.cc").write_bytes(b"new candidate source\n")
    (root / "code/unit.cc").write_bytes(prior["code/unit.cc"] + b"// candidate spelling\n")
    current = {path.relative_to(root).as_posix(): path.read_bytes() for path in (root / "code").rglob("*") if path.is_file()}
    before = working_state(root)
    calls, living, comparisons = contexts(monkeypatch, outcome=outcome)
    report = native.check_build_graph(root, task.task_id)
    assert len(calls) == 2 and living == []
    assert len(comparisons) == 1 and set(comparisons[0]) == set(task.editable_files)
    assert all(call["root"] == root and call["roots"] == ("code",) for call in calls)
    assert [call["files"] for call in calls] == [prior, current]
    assert [call["versioned"] for call in calls] == [{GRAPH: old_graph}, {GRAPH: (root / GRAPH).read_bytes()}]
    assert all(not call["context"].root.exists() for call in calls)
    assert report["baseline_revision"] == task.base_revision
    assert report["graph_inputs"] == [GRAPH]
    assert report["source_changes"] == ["code/new.cc", "code/old.cc", "code/unit.cc"]
    assert report["result"] == ("fail" if outcome == "fail" else "unknown")
    assert report["linked_effects"] == "unknown"
    assert not report["persisted"] and not report["ready_for_collection"]
    assert report["current_build"] == calls[1]["context"].report
    assert report["baseline_build"] == calls[0]["context"].report
    assert TaskStore(root).read(task.task_id) == task and TaskStore(root).read("owner") == stored
    assert working_state(root) == before


def test_cmake_graph_requires_explicit_editable_scope(graph_task):
    root, task = graph_task
    task = new_task(root, task, editable_files=tuple(name for name in task.editable_files if name != GRAPH))
    change_graph(root)
    with pytest.raises(ValueError, match="graph inputs exceed authorized editable"):
        native.check_build_graph(root, task.task_id)


@pytest.mark.parametrize("outcome", ["pass", "fail", "unknown"])
def test_initializer_comparison_is_consumed_live_without_waiving_remaining_effects(graph_task, monkeypatch, outcome):
    root, task = graph_task
    change_graph(root)
    calls, living, _ = contexts(monkeypatch)
    observations = []
    def initializers(old, new):
        assert living == [0, 1]
        assert old is calls[0]["context"] and new is calls[1]["context"]
        old.check_current()
        new.check_current()
        observations.append(True)
        return {"result": outcome, "remaining_linked_effects": "unknown"}
    monkeypatch.setattr(native, "compare_build_initializers", initializers)
    before = working_state(root)
    report = native.check_build_graph(root, task.task_id)
    assert observations == [True] and living == []
    assert report["initializers"]["result"] == outcome
    assert report["result"] == report["linked_effects"] == "unknown"
    assert not report["ready_for_collection"] and working_state(root) == before


@pytest.mark.parametrize("consistency", ["pass", "fail", "unknown"])
def test_input_corruption_is_distinct_from_cross_build_initializer_change(graph_task, monkeypatch, consistency):
    root, task = graph_task
    change_graph(root)
    _, living, _ = contexts(monkeypatch)
    monkeypatch.setattr(native, "compare_build_initializers", lambda *args: {
        "result": "fail", "input_consistency": consistency, "remaining_linked_effects": "unknown"})
    report = native.check_build_graph(root, task.task_id)
    assert living == []
    assert report["result"] == ("fail" if consistency == "fail" else "unknown")
    assert report["linked_effects"] == "unknown" and not report["ready_for_collection"]


@pytest.mark.parametrize("damage", ["modify", "add", "delete"])
def test_source_changes_outside_editable_scope_refuse_before_builds(graph_task, damage):
    root, task = graph_task
    task = new_task(root, task, editable_files=(GRAPH,))
    change_graph(root)
    path = root / ("code/unowned.cc" if damage == "add" else "code/unit.cc")
    if damage == "delete":
        path.unlink()
    else:
        path.write_bytes(b"outside authorized source change\n")
    before = working_state(root)
    with pytest.raises(ValueError, match="source changes exceed authorized editable"):
        native.check_build_graph(root, task.task_id)
    assert working_state(root) == before


@pytest.mark.parametrize("name", ["recovery/compile.py", "recovery/build.json", "recovery/project.json"])
def test_other_changed_policy_cannot_borrow_graph_path(graph_task, name):
    root, task = graph_task
    change_graph(root)
    path = root / name
    path.write_bytes(path.read_bytes() + b"\n ")
    with pytest.raises(ValueError, match="exclusively versioned graph changes"):
        native.check_build_graph(root, task.task_id)


def test_unchanged_graph_is_not_a_graph_transition(graph_task):
    root, task = graph_task
    with pytest.raises(ValueError, match="exclusively versioned graph changes"):
        native.check_build_graph(root, task.task_id)


def test_missing_command_stages_refuses_without_building(graph_task):
    root, task = graph_task
    path = root / "recovery/build.json"
    config = json.loads(path.read_text())
    del config["compile_link"]["command_stages"]
    path.write_text(json.dumps(config))
    git(root, "add", ".")
    git(root, "commit", "-m", "Synthetic baseline with no command grammar")
    task = new_task(root, task, base_revision=git(root, "rev-parse", "HEAD"))
    change_graph(root)
    with pytest.raises(ValueError, match="declared command stages"):
        native.check_build_graph(root, task.task_id)


def test_historical_image_pin_cannot_be_replaced_by_current_pin(graph_task):
    root, task = graph_task
    pin = root / "code/PIN"
    original = pin.read_bytes()
    pin.write_bytes(b"sha256 " + b"0" * 64 + b"\n")
    git(root, "add", ".")
    git(root, "commit", "-m", "Synthetic incompatible historical image pin")
    task = new_task(root, task, base_revision=git(root, "rev-parse", "HEAD"),
                    editable_files=(*task.editable_files, "code/PIN"))
    pin.write_bytes(original)
    change_graph(root)
    with pytest.raises(ValueError, match="baseline must retain the task's pinned original image"):
        native.check_build_graph(root, task.task_id)


@pytest.mark.parametrize("damage", ["task", "graph", "source"])
@pytest.mark.parametrize("stage", ["baseline", "current", "compare"])
def test_context_time_mutation_is_preserved_and_refused(graph_task, monkeypatch, damage, stage):
    root, task = graph_task
    change_graph(root)
    def mutate():
        if damage == "task":
            path = root / f"recovery/tasks/{task.task_id}.json"
            value = json.loads(path.read_text())
            value["intent"] = "changed during observation"
            path.write_text(json.dumps(value))
        else:
            path = root / (GRAPH if damage == "graph" else "code/unit.cc")
            path.write_bytes(path.read_bytes() + b"\n// newer edit\n")
    def on_enter(index):
        if (stage == "baseline" and index == 0) or (stage == "current" and index == 1):
            mutate()
    calls, living, comparisons = contexts(monkeypatch, mutate=on_enter,
                                          compare_mutate=mutate if stage == "compare" else None)
    with pytest.raises(ValueError, match="task application graph inputs or authorization changed"):
        native.check_build_graph(root, task.task_id)
    assert living == []
    assert len(calls) == (1 if stage == "baseline" else 2)
    assert len(comparisons) == (1 if stage == "compare" else 0)
    if damage == "task":
        assert TaskStore(root).read(task.task_id).intent == "changed during observation"
    else:
        assert (root / (GRAPH if damage == "graph" else "code/unit.cc")).read_bytes().endswith(b"// newer edit\n")
    assert TaskStore(root).read(task.task_id).base_revision == task.base_revision
