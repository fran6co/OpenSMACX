"""Native ownership selection/authorization; synthetic checks are not acceptance."""

from dataclasses import replace
import json
import subprocess
import sys

import pytest

from match_recovery.driver import working_state
from match_recovery.component_source import render_component
from match_recovery.model import Origin, Span
from match_recovery.planning import policy, select_task, start
from match_recovery.tasks import TaskStore
from test_source_configuration import configured, vendor_manifest
from test_task_source import git, project


@pytest.fixture
def setup(project):
    root = project
    git(root, "config", "user.name", "Planner test")
    git(root, "config", "user.email", "planner@example.invalid")
    (root / ".gitignore").write_text("build/\n")
    delegation = {"schema_version": 1, "author": "coordinator", "command": {"argv": ["planner"], "timeout": 10},
                  "max_entities": 10, "max_editable_files": 4, "commit_decision": True,
                  "evidence": ["Authorize source-owned task selection and task-only checkpoints"]}
    (root / "recovery/planner.json").write_text(json.dumps(delegation))
    git(root, "add", ".")
    git(root, "commit", "-m", "Planner authorization")
    return root, delegation


def answer(payload, action="create", **changes):
    return {"snapshot": payload["snapshot"], "action": action, "task_id": "owner" if action != "defer" else "",
            "group": "source:code/owner.cc" if action == "create" else "",
            "intent": "Recover ownership, not just byte spellings" if action == "create" else "",
            "editable_files": [], "evidence": ["code/owner.cc and its interface"], **changes}


def test_native_planner_checkpoints_only_task_decision_then_can_resume_same_baseline(setup):
    root, delegation = setup
    baseline = git(root, "rev-parse", "HEAD")
    source = (root / "code/owner.cc").read_bytes()
    def execute(command, root, name, payload):
        group, = payload["groups"]
        assert "entities" not in group  # Deep context is on demand, not an entire-image prompt.
        assert group["eligible"] and group["editable_files"] == ["code/owner.cc", "code/owner.hpp"]
        assert payload["create_available"] and not payload["tasks"]
        return 0, answer(payload)
    result = select_task(root, "custom", delegation, execute=execute)
    assert result["action"] == "create" and not result["ready_for_collection"]
    assert result["checkpoint"] == git(root, "rev-parse", "HEAD")
    assert git(root, "diff-tree", "--no-commit-id", "--name-only", "-r", "HEAD") == "recovery/tasks/owner.json"
    assert git(root, "rev-parse", "HEAD^") == baseline
    task = TaskStore(root).read("owner")
    assert task.base_revision == baseline and task.editable_files == ("code/owner.cc", "code/owner.hpp")
    assert (root / "code/owner.cc").read_bytes() == source
    (root / "code/owner.cc").write_bytes(source + b"// unfinished structural progress\n")
    before = working_state(root)
    def resume(command, root, name, payload):
        assert not payload["create_available"]
        assert payload["groups"][0]["reasons"] == ["reserved_by_existing_task"]
        assert payload["tasks"][0]["eligible"]
        return 0, answer(payload, "resume")
    result = select_task(root, "custom", delegation, execute=resume)
    assert result["action"] == "resume" and result["checkpoint"] is None
    assert working_state(root) == before and TaskStore(root).read("owner") == task


@pytest.mark.parametrize("damage", ["stale", "unknown_group", "scope", "seed", "pin", "dirty", "limit", "write", "git_write", "exception_write", "unknown_resume", "bad_array"])
def test_invalid_planning_cannot_publish_or_erase_source(setup, damage):
    root, delegation = setup
    if damage == "dirty":
        (root / "unrelated.txt").write_text("preserve user work")
    if damage == "limit":
        delegation = {**delegation, "max_editable_files": 1}
    def execute(command, root, name, payload):
        response = answer(payload)
        if damage == "stale": response["snapshot"] = "b" * 64
        if damage == "unknown_group": response["group"] = "class:Invented"
        if damage == "scope": response["editable_files"] = ["recovery/project.json"]
        if damage == "seed": response["editable_files"] = ["hints/attempt.cc"]
        if damage == "pin": response["editable_files"] = ["original/identity"]
        if damage == "bad_array": response["editable_files"] = [{}]
        if damage == "unknown_resume": response = answer(payload, "resume")
        if damage in ("write", "exception_write"):
            (root / "code/owner.cc").write_text("unauthorized source edit")
        if damage == "exception_write": raise subprocess.TimeoutExpired("planner", 1)
        if damage == "git_write": git(root, "commit", "--allow-empty", "-m", "Unauthorized Git mutation")
        return 0, response
    with pytest.raises(ValueError):
        select_task(root, "custom", delegation, execute=execute)
    assert not (root / "recovery/tasks/owner.json").exists()
    if damage in ("write", "exception_write"):
        assert (root / "code/owner.cc").read_text() == "unauthorized source edit"


@pytest.mark.parametrize("change", [{"commit_decision": False}, {"commit_decision": 1}, {"max_entities": True},
                                    {"max_editable_files": 0}, {"evidence": []}, {"unknown": True}])
def test_task_selection_needs_explicit_bounded_commit_authority(setup, change):
    with pytest.raises(ValueError):
        policy({**setup[1], **change})


@pytest.mark.parametrize("action", ["defer", "unavailable"])
def test_planner_deferral_or_unavailability_leaves_no_task_or_git_write(setup, action):
    root, delegation = setup
    before = working_state(root)
    result = select_task(root, "custom", delegation,
                         execute=lambda *args: (3, {"error": "unavailable"}) if action == "unavailable" else (0, answer(args[-1], "defer")))
    assert result["action"] == action and not result["ready_for_collection"]
    assert before == working_state(root)


def test_start_selects_then_drives_without_manual_task_creation_or_checkpoint(setup):
    root, delegation = setup
    config = {"schema_version": 1, "target": "custom", "context": {"argv": ["context"], "timeout": 10},
              "check": {"argv": ["check"], "timeout": 10}}
    worker = {"author": "worker", "command": {"argv": ["worker"], "timeout": 10}}
    calls = []
    def execute(command, root, name, payload):
        role = command.argv[0]
        calls.append(role)
        if role == "planner": return 0, answer(payload)
        if role == "context": return 0, {}
        if role == "check":
            passed = "// improved" in (root / "code/owner.cc").read_text()
            return (0 if passed else 1), {"result": "pass" if passed else "fail"}
        assert role == "worker" and name == "owner"
        path = root / "code/owner.cc"
        path.write_text(path.read_text() + "// improved\n")
        return 0, {"discoveries": []}
    report = start(root, config, worker, delegation, iterations=1, execute=execute)
    assert calls == ["planner", "context", "check", "worker", "context", "check"]
    assert report["stop_reason"] == "independent_review_required" and not report["ready_for_collection"]
    assert report["selection"]["action"] == "create"


@pytest.mark.parametrize("scope,author", [("full_project", "coordinator"), ("change", "worker")])
def test_invalid_source_publication_permission_refuses_before_planning(setup, scope, author):
    root, delegation = setup
    before = working_state(root)
    def execute(*args):
        pytest.fail("invalid publication must be rejected before any planner or worker invocation")
    with pytest.raises(ValueError, match="checkpoint"):
        start(root, {"target": "custom", "acceptance_scope": scope},
              {"author": "worker", "command": {"argv": ["worker"], "timeout": 10}}, delegation,
              iterations=1, checkpoint_configuration={"author": author, "message": "source checkpoint",
                                                       "allow_migration": False}, execute=execute)
    assert working_state(root) == before


def test_installed_planner_agent_bridge_selects_without_a_task_input(setup):
    root, delegation = setup
    script = ("import json,sys; from pathlib import Path; p=json.loads(Path(sys.argv[1]).read_text()); sys.stdin.read(); "
              "print(json.dumps(dict(snapshot=p['snapshot'], action='create', task_id='owner', group='source:code/owner.cc', "
              "intent='Recover ownership', editable_files=[], evidence=['source interface inspected'])))")
    role = {"command": {"argv": [sys.executable, "-c", script, "{request}"], "timeout": 10},
            "instructions": [], "response": "stdout"}
    (root / "recovery/agents.json").write_text(json.dumps({"schema_version": 1, "artifacts": "build/agents",
                                                          **{name: role for name in ("planner", "worker", "reviewer")}}))
    git(root, "add", ".")
    git(root, "commit", "-m", "Local planner command")
    delegation = {**delegation, "command": {"argv": [sys.executable, "-m", "match_recovery.cli", "agent", "planner"], "timeout": 20}}
    report = select_task(root, "custom", delegation)
    assert report["action"] == "create" and TaskStore(root).read("owner").target == "custom"


def test_existing_scope_cannot_be_evaded_by_creating_another_task(setup):
    root, delegation = setup
    select_task(root, "custom", delegation, execute=lambda *args: (0, answer(args[-1])))
    before = working_state(root)
    with pytest.raises(ValueError, match="eligible unreserved"):
        select_task(root, "custom", delegation, execute=lambda *args: (0, answer(args[-1], task_id="duplicate")))
    assert working_state(root) == before


@pytest.mark.parametrize("kind,reason", [("component", "verify_dependency"), ("conflict", "resolve_conflict"),
                                       ("unassigned", "identify_owner")])
def test_nonapplication_or_ambiguous_groups_remain_visible_but_cannot_be_selected(setup, kind, reason):
    root, delegation = setup
    if kind == "component":
        (root / "code/codec.recovery.h").write_text(render_component(vendor_manifest()))
    elif kind == "conflict":
        path = root / "hints/attempt.cc"
        path.write_text(path.read_text().replace("0x00000018", "0x00000019"))
    else:
        (root / "code/owner.cc").unlink()  # Only the seed remains; no invented product owner.
    git(root, "add", ".")
    git(root, "commit", "-m", "Nonapplication or unresolved evidence")
    def execute(command, root, name, payload):
        group, = payload["groups"]
        assert not group["eligible"] and reason in group["reasons"]
        return 0, answer(payload, group=group["selector"])
    with pytest.raises(ValueError, match="eligible unreserved"):
        select_task(root, "custom", delegation, execute=execute)
    assert not (root / "recovery/tasks/owner.json").exists()


def test_extra_editing_files_cannot_smuggle_in_a_vendor_header(setup):
    root, delegation = setup
    manifest = vendor_manifest()
    manifest = replace(manifest, bindings=(replace(manifest.bindings[0], origin=Origin("a" * 64, "custom:offset", 32),
                                                  spans=(Span(32, 40),), symbol="_codec"),))
    (root / "code/codec.recovery.h").write_text(render_component(manifest))
    (root / "code/vendor").mkdir()
    (root / "code/vendor/api.hpp").write_text("void codec();\n")
    git(root, "add", ".")
    git(root, "commit", "-m", "Independent vendor dependency")
    with pytest.raises(ValueError, match="delegated file scope"):
        select_task(root, "custom", delegation,
                    execute=lambda *args: (0, answer(args[-1], editable_files=["code/vendor/api.hpp"])))


def test_checkpoint_failure_preserves_decision_and_refuses_worker_dispatch(setup, monkeypatch):
    import match_recovery.planning as planning
    root, delegation = setup
    baseline = git(root, "rev-parse", "HEAD")
    original = planning._git
    def unavailable(root, *args):
        if args[0] == "commit":
            raise subprocess.CalledProcessError(1, "git commit", stderr=b"controlled unavailable signing identity")
        return original(root, *args)
    monkeypatch.setattr(planning, "_git", unavailable)
    with pytest.raises(subprocess.CalledProcessError):
        select_task(root, "custom", delegation, execute=lambda *args: (0, answer(args[-1])))
    assert (root / "recovery/tasks/owner.json").is_file()
    assert git(root, "rev-parse", "HEAD") == baseline
    assert git(root, "diff", "--cached", "--name-only") == "recovery/tasks/owner.json"


@pytest.mark.parametrize("authority", ["other", "foreign_target"])
def test_planner_cannot_resume_another_author_or_target_task(setup, authority):
    root, delegation = setup
    select_task(root, "custom", delegation, execute=lambda *args: (0, answer(args[-1])))
    if authority == "other":
        delegation = {**delegation, "author": "another-coordinator"}
    with pytest.raises(ValueError, match="within delegated authority"):
        select_task(root, "foreign" if authority == "foreign_target" else "custom", delegation,
                    execute=lambda *args: (0, answer(args[-1], "resume")))
