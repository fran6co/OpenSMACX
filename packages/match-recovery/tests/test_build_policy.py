"""Graph-policy classification is not scope permission or acceptance evidence."""

from dataclasses import replace
import json
import sys

import pytest

from match_recovery.build_check import Build
from match_recovery.driver import working_state
from match_recovery.measurement import Measurement
from match_recovery.migration_check import unchanged_build_policy
from match_recovery.tasks import TaskStore
from test_migration_check import migration, project
from test_task_source import git


GRAPH = "BuildGraph.txt"


def update(root, name, **changes):
    path = root / name
    value = json.loads(path.read_text())
    value.update(changes)
    path.write_text(json.dumps(value))


def baseline(root):
    git(root, "add", ".")
    git(root, "commit", "-m", "Synthetic policy predates this test task")
    # This separate task value names the new test baseline. Never rewrite the
    # persisted owner task or pretend that an existing baseline was reset.
    return replace(TaskStore(root).read("owner"), task_id="policy-test",
                   base_revision=git(root, "rev-parse", "HEAD"))


@pytest.fixture
def graph_policy(migration):
    root = migration
    (root / GRAPH).write_text("code/unit.cc\n")
    (root / "recovery/build-helper.py").write_text("# application helper\n")
    (root / "generator-data").mkdir()
    (root / "generator-data/model.txt").write_text("generator policy\n")
    (root / "recovery/generator.py").write_text("# preparation helper\n")
    update(root, "recovery/build.json", versioned_inputs=[GRAPH],
           inputs=["recovery/build-helper.py"])
    preparation = json.loads((root / "recovery/build.json").read_text())
    preparation.update(artifacts="build/preparation", versioned_inputs=[],
                       input_roots=["code", "generator-data"], inputs=["recovery/generator.py"])
    (root / "recovery/preparation.json").write_text(json.dumps(preparation))
    update(root, "recovery/project.json", preparation="recovery/preparation.json")
    return root, baseline(root)


def classify(root, task):
    from match_recovery.build_policy import build_policy_changes

    before = working_state(root)
    result = build_policy_changes(root, task, Measurement.read(root), Build.read(root))
    assert working_state(root) == before
    assert set(result) == {"baseline_revision", "unchanged", "graph_only", "changed_paths", "invalid_paths"}
    assert result["baseline_revision"] == task.base_revision
    assert type(result["unchanged"]) is bool and type(result["graph_only"]) is bool
    assert result["changed_paths"] == sorted(set(result["changed_paths"]))
    assert result["invalid_paths"] == sorted(set(result["invalid_paths"]))
    return result


def test_unchanged_policy_is_not_a_graph_transition(graph_policy):
    root, task = graph_policy
    assert classify(root, task) == {"baseline_revision": task.base_revision, "unchanged": True,
                                   "graph_only": False, "changed_paths": [], "invalid_paths": []}
    assert unchanged_build_policy(root, task, Measurement.read(root), Build.read(root))


def test_only_declared_graph_content_is_eligible_without_granting_task_scope(graph_policy):
    root, task = graph_policy
    stored = TaskStore(root).read("owner")
    assert GRAPH not in task.editable_files
    (root / GRAPH).write_text("code/unit.cc\ncode/new-unit.cc\n")
    assert classify(root, task) == {"baseline_revision": task.base_revision, "unchanged": False,
                                   "graph_only": True, "changed_paths": [GRAPH], "invalid_paths": []}
    assert not unchanged_build_policy(root, task, Measurement.read(root), Build.read(root))
    assert TaskStore(root).read("owner") == stored
    assert GRAPH not in task.editable_files


@pytest.mark.parametrize("name", ["recovery/build-helper.py", "recovery/compile.py", "recovery/build.json",
                                 "recovery/project.json", "recovery/preparation.json",
                                 "recovery/generator.py", "generator-data/model.txt"])
@pytest.mark.parametrize("with_graph", [False, True])
def test_non_graph_policy_changes_and_mixed_changes_are_ineligible(graph_policy, name, with_graph):
    root, task = graph_policy
    path = root / name
    path.write_bytes(path.read_bytes() + b"\n ")
    if with_graph:
        (root / GRAPH).write_text("changed graph\n")
    report = classify(root, task)
    assert not report["unchanged"] and not report["graph_only"]
    assert name in report["changed_paths"]
    if with_graph:
        assert GRAPH in report["changed_paths"]
    assert not unchanged_build_policy(root, task, Measurement.read(root), Build.read(root))


def test_undeclared_graph_added_inside_preparation_inputs_is_not_eligible(graph_policy):
    root, task = graph_policy
    name = "generator-data/undeclared-graph.txt"
    (root / name).write_text("code/new-unit.cc\n")
    report = classify(root, task)
    assert not report["unchanged"] and not report["graph_only"]
    assert name in report["changed_paths"]


def test_ordinary_source_edit_is_not_a_build_policy_change(graph_policy):
    root, task = graph_policy
    path = root / "code/unit.cc"
    path.write_text(path.read_text() + "// candidate source\n")
    report = classify(root, task)
    assert report["unchanged"] and not report["graph_only"]
    assert report["changed_paths"] == report["invalid_paths"] == []


@pytest.mark.parametrize("damage", ["missing_baseline", "missing_current", "mode", "symlink", "hardlink"])
def test_graph_membership_kind_and_mode_changes_are_not_content_transitions(graph_policy, damage):
    root, task = graph_policy
    path = root / GRAPH
    original = path.read_bytes()
    if damage == "missing_baseline":
        path.unlink()
        task = baseline(root)
        path.write_bytes(original)
    elif damage == "missing_current":
        path.unlink()
    elif damage == "mode":
        path.chmod(0o755)
    else:
        target = root / "other-graph.txt"
        target.write_bytes(original)
        path.unlink()
        if damage == "symlink":
            path.symlink_to(target.name)
        else:
            path.hardlink_to(target)
    try:
        report = classify(root, task)
    except ValueError as error:
        # Build.snapshot rejects malformed current declared files before policy
        # classification; that remains a refusal, never an eligible transition.
        assert damage in {"missing_current", "symlink", "hardlink"}
        assert "build input" in str(error) or "versioned" in str(error)
        return
    assert not report["unchanged"] and not report["graph_only"]
    assert GRAPH in report["invalid_paths"]


def test_graph_declaration_added_after_baseline_does_not_authorize_the_change(graph_policy):
    root, task = graph_policy
    update(root, "recovery/build.json", versioned_inputs=[])
    task = baseline(root)
    update(root, "recovery/build.json", versioned_inputs=[GRAPH])
    (root / GRAPH).write_text("changed graph\n")
    report = classify(root, task)
    assert not report["unchanged"] and not report["graph_only"]
    assert "recovery/build.json" in report["changed_paths"]


@pytest.mark.parametrize("owner", ["compiler", "toolchain"])
def test_declared_graph_cannot_substitute_compiler_or_toolchain_policy(graph_policy, owner):
    root, task = graph_policy
    project_path = root / "recovery/project.json"
    value = json.loads(project_path.read_text())
    if owner == "compiler":
        value["compiler"]["inputs"].append(GRAPH)
    else:
        description = {"schema_version": 1, "files": {"tool": str(root / "original.exe")}, "directories": {}}
        value["toolchain"] = {"command": {"argv": [sys.executable, "-c", f"import json; print(json.dumps({description!r}))"],
                                            "timeout": 5}, "inputs": [GRAPH], "environment": []}
    project_path.write_text(json.dumps(value))
    task = baseline(root)
    (root / GRAPH).write_text("changed graph and tool policy\n")
    report = classify(root, task)
    assert not report["unchanged"] and not report["graph_only"]
    assert GRAPH in report["changed_paths"]


def test_source_root_does_not_hide_declared_compiler_helper_changes(graph_policy):
    root, task = graph_policy
    helper = root / "code/compiler-helper.py"
    helper.write_text("# original compiler helper\n")
    project_path = root / "recovery/project.json"
    value = json.loads(project_path.read_text())
    value["compiler"]["inputs"].append("code/compiler-helper.py")
    project_path.write_text(json.dumps(value))
    task = baseline(root)
    helper.write_text("# changed compiler helper\n")
    (root / GRAPH).write_text("changed graph\n")
    report = classify(root, task)
    assert not report["unchanged"] and not report["graph_only"]
    assert "code/compiler-helper.py" in report["changed_paths"]
