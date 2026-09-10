"""Reservations derive from real scratch worktrees, not cached task reports."""

from dataclasses import replace
import json

import pytest

from match_recovery.driver import working_state
from match_recovery.locking import file_lock
from match_recovery.model import Origin
from match_recovery.planning import policy, select_task
from match_recovery.scheduling import reservations
from match_recovery.tasks import Task, TaskStore
from test_planning import answer, setup
from test_source_configuration import configured
from test_task_source import git, project


ORIGIN = Origin("a" * 64, "custom:offset", 16)
OTHER = Origin("a" * 64, "custom:offset", 32)


def task(root, name="parked", **changes):
    value = Task(name, "custom", "coordinator", "Preserve independently owned recovery work",
                 git(root, "rev-parse", "HEAD"), (ORIGIN,),
                 ("code/owner.cc", "code/owner.hpp"), ("Existing source-owned work",))
    value = replace(value, **changes)
    TaskStore(root).create(value)
    return value


def worker(root, name="parked"):
    path = root / "build" / name
    git(root, "worktree", "add", "-b", name, str(path))
    return path


def row(path, tasks=("parked",)):
    return {"path": str(path), "tasks": list(tasks), "evidence": ["Preserve this unrelated recovery checkout"]}


def checkpoint(root):
    git(root, "add", ".")
    git(root, "commit", "-m", "Synthetic task/source decision")


@pytest.mark.parametrize("change", [
    None, {}, "parked", ["parked"], [{"path": "build/parked", "tasks": ["parked"]}],
    [{"path": "../parked", "tasks": ["parked"], "evidence": ["Keep"]}],
    [{"path": "build/parked", "tasks": [], "evidence": ["Keep"]}],
    [{"path": "build/parked", "tasks": ["../task"], "evidence": ["Keep"]}],
    [{"path": "build/parked", "tasks": ["parked", "parked"], "evidence": ["Keep"]}],
    [{"path": "build/parked", "tasks": ["parked"], "evidence": []}],
    [{"path": "build/parked", "tasks": ["parked"], "evidence": [""]}],
    [{"path": "build/parked", "tasks": ["parked"], "evidence": ["Keep"], "editable_files": ["code/owner.cc"]}],
    [{"path": "build/parked", "tasks": ["parked"], "evidence": ["Keep"]}] * 2,
])
def test_reservation_policy_requires_only_explicit_paths_tasks_and_evidence(setup, change):
    with pytest.raises(ValueError):
        policy({**setup[1], "reserved_worktrees": change})


def test_omitted_or_empty_reservations_preserve_existing_policy(setup):
    assert policy(setup[1]) == setup[1]
    assert policy({**setup[1], "reserved_worktrees": []})["reserved_worktrees"] == []


@pytest.mark.parametrize("state", ["dirty", "staged", "committed", "assume_unchanged"])
def test_unreserved_worker_source_progress_refuses_before_planner(setup, state):
    root, delegation = setup
    parked = worker(root)
    path = parked / "code/owner.cc"
    if state == "assume_unchanged":
        git(parked, "update-index", "--assume-unchanged", "code/owner.cc")
    path.write_text(path.read_text() + "// preserved worker progress\n")
    if state in ("staged", "committed"):
        git(parked, "add", "code/owner.cc")
    if state == "committed":
        git(parked, "commit", "-m", "Worker-only recovery progress")
    before = working_state(parked)
    calls = []
    with pytest.raises(ValueError, match="unreserved"):
        select_task(root, "custom", delegation, execute=lambda *args: calls.append(args))
    assert not calls and working_state(parked) == before
    assert not (root / "recovery/tasks/owner.json").exists()


@pytest.mark.parametrize("committed", [False, True])
def test_restoring_workfile_cannot_hide_index_or_committed_worker_progress(setup, committed):
    root, delegation = setup
    parked = worker(root)
    path = parked / "code/owner.cc"
    original = path.read_bytes()
    path.write_bytes(original + b"// independently preserved progress\n")
    git(parked, "add", "code/owner.cc")
    if committed:
        git(parked, "commit", "-m", "Worker progress hidden by workfile reversal")
    path.write_bytes(original)
    before = working_state(parked)
    calls = []
    with pytest.raises(ValueError, match="unreserved"):
        select_task(root, "custom", delegation, execute=lambda *args: calls.append(args))
    assert not calls and working_state(parked) == before


@pytest.mark.parametrize("committed", [False, True])
def test_worker_only_task_and_dirty_scope_history_supply_real_reservation(setup, committed):
    root, delegation = setup
    parked = worker(root)
    original = task(parked)
    if committed:
        checkpoint(parked)
    TaskStore(parked).expand("parked", request_id="extra", author="coordinator", subjects=(OTHER,),
                             editable_files=("code/extra.hpp",), evidence=("Shared member discovered",), expected_revision=0)
    (parked / "code/extra.hpp").write_text("void extra();\n")
    before = working_state(parked)
    observed = []

    def execute(command, current, name, payload):
        observed.extend(payload["reservations"])
        group, = payload["groups"]
        assert not group["eligible"] and "reserved_by_external_worktree" in group["reasons"]
        return 0, answer(payload, "defer")

    result = select_task(root, "custom", {**delegation, "reserved_worktrees": [row(parked)]}, execute=execute)
    assert result["action"] == "defer" and not result["ready_for_collection"]
    reservation, = observed
    assert set(reservation["subjects"]) == {ORIGIN.entity_id, OTHER.entity_id}
    assert set(reservation["editable_files"]) == {*original.editable_files, "code/extra.hpp"}
    assert len(reservation["state"]) == 64
    assert working_state(parked) == before
    assert not (root / "recovery/tasks/parked.json").exists()


def test_unreserved_worker_only_task_decision_is_not_a_clean_free_checkout(setup):
    root, delegation = setup
    parked = worker(root)
    task(parked)
    checkpoint(parked)
    with pytest.raises(ValueError, match="unreserved"):
        with reservations(root, []):
            pytest.fail("worker-only task was ignored")


@pytest.mark.parametrize("kind", ["source", "task"])
def test_git_ignored_recovery_inputs_are_not_invisible_to_reservations(setup, kind):
    root, _ = setup
    (root / ".gitignore").write_text("build/\ncode/hidden.hpp\nrecovery/tasks/\nrecovery/history/\n")
    checkpoint(root)
    parked = worker(root)
    if kind == "source":
        (parked / "code/hidden.hpp").write_text("void hidden();\n")
    else:
        task(parked)
    assert git(parked, "status", "--porcelain") == ""
    with pytest.raises(ValueError, match="unreserved"):
        with reservations(root, []):
            pytest.fail("ignored recovery input was treated as no work")


def test_ignored_append_only_task_history_still_expands_reserved_scope(setup):
    root, _ = setup
    (root / ".gitignore").write_text("build/\nrecovery/history/\n")
    task(root)
    checkpoint(root)
    parked = worker(root)
    TaskStore(parked).expand("parked", request_id="hidden", author="coordinator", subjects=(OTHER,),
                             editable_files=("code/hidden.hpp",), evidence=("Authorized scope",), expected_revision=0)
    assert git(parked, "status", "--porcelain") == ""
    with reservations(root, [row(parked)]) as view:
        assert OTHER.entity_id in view.subjects and "code/hidden.hpp" in view.files


def test_reservation_unions_destination_and_worker_task_scope_without_resetting_baseline(setup):
    root, _ = setup
    original = task(root)
    checkpoint(root)
    parked = worker(root)
    for checkout, filename in ((root, "code/destination.hpp"), (parked, "code/worker.hpp")):
        TaskStore(checkout).expand("parked", request_id="extra", author="coordinator", subjects=(),
                                  editable_files=(filename,), evidence=("Independent authorized expansion",), expected_revision=0)
    with reservations(root, [row(parked)]) as view:
        assert view.files == {*original.editable_files, "code/destination.hpp", "code/worker.hpp"}
        assert view.subjects == {ORIGIN.entity_id}
    assert TaskStore(root).read("parked").base_revision == original.base_revision
    assert TaskStore(parked).read("parked").base_revision == original.base_revision


@pytest.mark.parametrize("field,value", [("intent", "Conflicting task identity"), ("base_revision", "b" * 40),
                                         ("target", "different"), ("author", "different")])
def test_same_task_name_with_conflicting_identity_refuses(setup, field, value):
    root, _ = setup
    task(root)
    checkpoint(root)
    parked = worker(root)
    path = parked / "recovery/tasks/parked.json"
    data = json.loads(path.read_text())
    data[field] = value
    path.write_text(json.dumps(data))
    with pytest.raises(ValueError, match="identity|baseline|conflict"):
        with reservations(root, [row(parked)]):
            pytest.fail("conflicting task identity was admitted")


@pytest.mark.parametrize("path", ["code/outside.hpp", "hints/attempt.cc", "original/identity", "recovery/tasks/unrelated.json"])
def test_reservation_does_not_authorize_outside_source_or_decisions(setup, path):
    root, _ = setup
    parked = worker(root)
    task(parked)
    destination = parked / path
    destination.write_text((destination.read_text() if destination.exists() else "") + "\n// outside progress\n")
    before = working_state(parked)
    with pytest.raises(ValueError, match="outside"):
        with reservations(root, [row(parked)]):
            pytest.fail("outside source/history was adopted")
    assert working_state(parked) == before


def test_invalid_dirty_history_refuses_instead_of_using_a_saved_scope(setup):
    root, _ = setup
    parked = worker(root)
    task(parked)
    store = TaskStore(parked)
    store.expand("parked", request_id="extra", author="coordinator", subjects=(),
                 editable_files=("code/extra.hpp",), evidence=("Authorized",), expected_revision=0)
    checkpoint(parked)
    journal = next(store.journal.stream_directory("task:parked").glob("*.json"))
    journal.write_text(journal.read_text().replace("Authorized", "Tampered"))
    with pytest.raises(ValueError):
        with reservations(root, [row(parked)]):
            pytest.fail("tampered committed task history was accepted")


def test_named_history_directory_does_not_authorize_unvalidated_extra_files(setup):
    root, _ = setup
    parked = worker(root)
    task(parked)
    store = TaskStore(parked)
    store.expand("parked", request_id="extra", author="coordinator", subjects=(),
                 editable_files=("code/extra.hpp",), evidence=("Authorized",), expected_revision=0)
    (store.journal.stream_directory("task:parked") / "unvalidated.txt").write_text("Not a journal decision\n")
    with pytest.raises(ValueError, match="outside|history"):
        with reservations(root, [row(parked)]):
            pytest.fail("journal directory membership substituted for validated event provenance")


@pytest.mark.parametrize("kind", ["source", "scope", "head", "inventory"])
def test_external_changes_during_planner_refuse_even_a_defer_response(setup, kind):
    root, delegation = setup
    parked = worker(root)
    task(parked)
    before = working_state(root)

    def execute(command, current, name, payload):
        if kind == "source":
            path = parked / "code/owner.cc"
            path.write_text(path.read_text() + "// concurrent progress\n")
        elif kind == "scope":
            TaskStore(parked).expand("parked", request_id="race", author="coordinator", subjects=(),
                                     editable_files=("code/new.hpp",), evidence=("Concurrent decision",), expected_revision=0)
        elif kind == "head":
            git(parked, "commit", "--allow-empty", "-m", "Concurrent worker checkpoint")
        else:
            worker(root, "another")
        return 0, answer(payload, "defer")

    with pytest.raises(ValueError, match="changed|replan"):
        select_task(root, "custom", {**delegation, "reserved_worktrees": [row(parked)]}, execute=execute)
    assert working_state(root) == before
    if kind == "source":
        assert "concurrent progress" in (parked / "code/owner.cc").read_text()


def test_new_ignored_source_during_planning_invalidates_external_snapshot(setup):
    root, delegation = setup
    (root / ".gitignore").write_text("build/\ncode/hidden.hpp\n")
    checkpoint(root)
    parked = worker(root)

    def execute(command, current, name, payload):
        assert payload["reservations"] == []
        (parked / "code/hidden.hpp").write_text("void concurrent_hidden();\n")
        return 0, answer(payload, "defer")

    with pytest.raises(ValueError, match="changed|replan"):
        select_task(root, "custom", delegation, execute=execute)
    assert (parked / "code/hidden.hpp").read_text() == "void concurrent_hidden();\n"


def test_active_external_worker_lease_refuses_planning_without_adopting_it(setup):
    root, delegation = setup
    parked = worker(root)
    task(parked)
    calls = []
    with file_lock(parked / "build/recovery/driver.lock"):
        with pytest.raises(TimeoutError, match="leased"):
            select_task(root, "custom", {**delegation, "reserved_worktrees": [row(parked)]},
                        execute=lambda *args: calls.append(args))
    assert not calls


def test_planner_cannot_append_reserved_file_to_an_unrelated_eligible_group(setup):
    root, delegation = setup
    (root / "code/second.cc").write_text('// ORIGINAL: 0x00000020 _second 0x00000020-0x00000028\nvoid second() {}\n')
    checkpoint(root)
    parked = worker(root)
    task(parked)
    before = working_state(root)

    def execute(command, current, name, payload):
        group = next(item for item in payload["groups"] if item["selector"] == "source:code/second.cc")
        assert group["eligible"]
        return 0, answer(payload, task_id="second", group=group["selector"], editable_files=["code/owner.hpp"])

    with pytest.raises(ValueError, match="scope|reservation"):
        select_task(root, "custom", {**delegation, "reserved_worktrees": [row(parked)]}, execute=execute)
    assert working_state(root) == before and not (root / "recovery/tasks/second.json").exists()


def test_planner_cannot_reuse_a_worker_only_task_id_for_a_disjoint_group(setup):
    root, delegation = setup
    (root / "code/second.cc").write_text('// ORIGINAL: 0x00000020 _second 0x00000020-0x00000028\nvoid second() {}\n')
    checkpoint(root)
    parked = worker(root)
    task(parked)
    before = working_state(root)

    def execute(command, current, name, payload):
        group = next(item for item in payload["groups"] if item["selector"] == "source:code/second.cc")
        assert group["eligible"] and not payload["tasks"]
        return 0, answer(payload, task_id="parked", group=group["selector"])

    with pytest.raises(ValueError, match="reserved|overlap"):
        select_task(root, "custom", {**delegation, "reserved_worktrees": [row(parked)]}, execute=execute)
    assert working_state(root) == before and not (root / "recovery/tasks/parked.json").exists()


@pytest.mark.parametrize("destination_change", ["source", "decision", "none"])
def test_clean_ancestor_worker_does_not_reserve_destination_only_changes(setup, destination_change):
    root, delegation = setup
    parked = worker(root)
    original = working_state(parked)
    if destination_change == "source":
        path = root / "code/owner.cc"
        path.write_text(path.read_text() + "// destination-only checkpoint\n")
        checkpoint(root)
    elif destination_change == "decision":
        task(root)
        checkpoint(root)
    observed = []

    def execute(command, current, name, payload):
        observed.append(payload)
        assert payload["reservations"] == []
        assert all("reserved_by_external_worktree" not in group["reasons"] for group in payload["groups"])
        return 0, answer(payload, "defer")

    assert select_task(root, "custom", delegation, execute=execute)["action"] == "defer"
    assert observed and working_state(parked) == original


def test_unrelated_worker_commit_does_not_turn_master_only_source_edits_into_worker_scope(setup):
    root, delegation = setup
    parked = worker(root)
    (parked / "notes.txt").write_text("Tooling notes, no recovery source change\n")
    checkpoint(parked)
    path = root / "code/owner.cc"
    path.write_text(path.read_text() + "// source changed only on destination\n")
    checkpoint(root)
    with reservations(root, []) as view:
        assert view.records == [] and not view.files and not view.subjects


@pytest.mark.parametrize("committed", [False, True])
def test_framework_only_recovery_test_and_documentation_work_is_not_application_reservation(setup, committed):
    root, _ = setup
    parked = worker(root)
    (parked / "recovery/tests").mkdir()
    (parked / "recovery/tests/test_framework.py").write_text("def test_example():\n    assert True\n")
    (parked / "recovery/README.md").write_text("Framework-only documentation\n")
    if committed:
        checkpoint(parked)
    before = working_state(parked)
    with reservations(root, []) as view:
        assert view.records == []
    assert working_state(parked) == before


def test_relative_reservation_resolves_to_registered_worktree_without_mutation(setup):
    root, _ = setup
    parked = worker(root)
    task(parked)
    before = working_state(parked)
    with reservations(root, [row("build/parked")]) as view:
        assert view.records[0]["path"] == str(parked)
        assert view.tasks == {"parked"}
    assert working_state(parked) == before


@pytest.mark.parametrize("kind", ["unregistered", "root", "symlink", "missing-task", "managed"])
def test_reservations_must_name_existing_distinct_canonical_unmanaged_worktrees(setup, kind):
    root, _ = setup
    parked = worker(root)
    task(parked)
    path, names, managed = parked, ("parked",), set()
    if kind == "unregistered":
        path = root / "build/not-a-worktree"
        path.mkdir()
    elif kind == "root":
        path = root
    elif kind == "symlink":
        path = root / "build/alias"
        path.symlink_to(parked, target_is_directory=True)
    elif kind == "missing-task":
        names = ("missing",)
    else:
        managed = {parked}
    with pytest.raises((ValueError, FileNotFoundError)):
        with reservations(root, [row(path, names)], managed=managed):
            pytest.fail("invalid reservation accepted")
