"""Single-use parallel investigation grants never resolve or accept discoveries."""

import json
import shutil
from copy import deepcopy
from concurrent.futures import ThreadPoolExecutor
from threading import Barrier

import pytest

from match_recovery.driver import drive
from match_recovery.investigation_round import authorize
from match_recovery.tasks import TaskStore
from test_coordination import answer, discover, setup
from test_source_configuration import configured
from test_task_source import git, project


def driver_configuration(task, acceptance_scope="full_project"):
    config = {"schema_version": 1, "target": task.target, "acceptance_scope": acceptance_scope,
              **{name: {"argv": [name], "timeout": 10}
                 for name in ("context", "check", "build", "acceptance")}}
    if acceptance_scope == "change":
        config.update(obligations={"argv": ["inventory"], "timeout": 10},
                      migration_check={"argv": ["migration"], "timeout": 10})
    return config


def role(name):
    return {"author": name, "command": {"argv": [name], "timeout": 10}}


def grant_for(setup, *, action="investigate"):
    root, _, task, _, delegation = setup
    result = authorize(root, task.task_id, delegation,
                       execute=lambda *args: (0, answer(args[-1], action)))
    assert result["status"] == "authorized"
    return result["authorization"]


def run_grant(root, task, grant, execute, **kwargs):
    return drive(root, task.task_id, driver_configuration(task), role("worker"),
                 iterations=1, reviewer_configuration=role("reviewer"),
                 investigation_configuration=grant, execute=execute, **kwargs)


@pytest.mark.parametrize("acceptance_scope", ["change", "full_project"])
def test_two_isolated_grants_allow_parallel_rounds_but_all_pass_never_accepts(setup, tmp_path_factory, acceptance_scope):
    root, store, task, caller, delegation = setup
    discover(setup)
    second_root = tmp_path_factory.mktemp("peer") / "checkout"
    shutil.copytree(root, second_root)
    second_setup = (second_root, TaskStore(second_root), task, caller, delegation)
    grants = [grant_for(item, action="investigate_local") for item in (setup, second_setup)]
    barrier = Barrier(2, timeout=15)

    def round_for(item, grant):
        checkout, journal, _, _, _ = item
        before, scope = journal.decisions(task.task_id), journal.scope(task.task_id)
        calls = []

        def execute(command, root, name, payload):
            name = command.argv[0]
            calls.append(name)
            if name == "context": return 0, {}
            if name == "inventory": return 0, {"obligations": []}
            if name in ("check", "build"): return 0, {"result": "pass"}
            assert name == "worker", "Pending findings cannot reach coordinator, review or acceptance"
            assert payload["editable_files"] == ["code/owner.cc"]
            assert payload["context"]["pending_discoveries"][0]["discovery_id"] == "layout"
            assert payload["context"]["discovery_investigations"][0]["action"] == "investigate_local"
            barrier.wait()
            source = root / "code/owner.cc"
            source.write_text(source.read_text() + "// bounded local investigation\n")
            return 0, {"discoveries": []}

        result = drive(checkout, task.task_id, driver_configuration(task, acceptance_scope), role("worker"),
                       iterations=1, reviewer_configuration=role("reviewer"),
                       investigation_configuration=grant, execute=execute)
        assert calls.count("worker") == 1
        assert not result["ready_for_collection"]
        assert (journal.decisions(task.task_id), journal.scope(task.task_id)) == (before, scope)
        assert journal.read(task.task_id).base_revision == task.base_revision
        assert journal.status(task.task_id, scope[0])["pending_discoveries"]
        return result

    with ThreadPoolExecutor(max_workers=2) as pool:
        futures = [pool.submit(round_for, item, grant) for item, grant in zip((setup, second_setup), grants)]
        assert all(future.result()["worker_rounds"] == 1 for future in futures)


def test_one_use_grant_is_consumed_even_without_source_progress(setup):
    root, store, task, _, _ = setup
    discover(setup, task.subjects[0].entity_id)
    grant = grant_for(setup)
    calls = []

    def execute(command, root, name, payload):
        name = command.argv[0]
        calls.append(name)
        if name == "context": return 0, {}
        if name in ("check", "build"): return 0, {"result": "pass"}
        assert name == "worker"
        return 0, {"discoveries": []}

    assert not run_grant(root, task, grant, execute)["ready_for_collection"]
    assert calls.count("worker") == 1
    with pytest.raises(ValueError):
        run_grant(root, task, grant, lambda *args: pytest.fail("Consumed grant must not execute again"))
    assert len(store.decisions(task.task_id)) == 1


@pytest.mark.parametrize("damage", ["source", "decision", "head", "index", "task", "root", "grant"])
def test_grant_refuses_changed_source_decisions_git_task_or_checkout(setup, tmp_path_factory, damage):
    root, store, task, _, _ = setup
    discover(setup, task.subjects[0].entity_id)
    grant = grant_for(setup)
    if damage == "source":
        path = root / "code/owner.cc"
        path.write_text(path.read_text() + "// unrelated concurrent edit retained\n")
    elif damage == "decision":
        store.discover(task.task_id, request_id="second", author="worker", hypothesis="New finding",
                       affects=[task.subjects[0].entity_id], evidence=["source"], expected_revision=1)
    elif damage == "head":
        git(root, "commit", "--allow-empty", "-m", "Moved committed baseline")
    elif damage == "index":
        (root / "extra").write_text("staged concurrent work")
        git(root, "add", "extra")
    elif damage == "task":
        path = root / "recovery/tasks/owner.json"
        value = json.loads(path.read_text())
        value["intent"] += " changed scope rationale"
        path.write_text(json.dumps(value))
    elif damage == "root":
        destination = tmp_path_factory.mktemp("copied-grant") / "checkout"
        shutil.copytree(root, destination)
        root = destination
    else:
        grant = {**grant, "extra_authority": True}
    with pytest.raises(ValueError):
        run_grant(root, task, grant, lambda *args: pytest.fail("Stale or altered grant reached execution"))
    if damage == "source":
        assert "concurrent edit retained" in (root / "code/owner.cc").read_text()


@pytest.mark.parametrize("action", ["expand", "incorporate", "reject"])
def test_read_only_grant_authorization_rejects_publishing_actions_before_journal_mutation(setup, action):
    root, store, task, _, delegation = setup
    discover(setup)
    before, scope = store.decisions(task.task_id), store.scope(task.task_id)
    with pytest.raises(ValueError):
        authorize(root, task.task_id, delegation, execute=lambda *args: (0, answer(args[-1], action)))
    assert (store.decisions(task.task_id), store.scope(task.task_id)) == (before, scope)


@pytest.mark.parametrize("action", ["investigate", "investigate_local"])
def test_unknown_originals_cannot_receive_an_investigation_grant(setup, action):
    root, store, task, _, delegation = setup
    discover(setup, "entity-" + "b" * 64)
    with pytest.raises(ValueError):
        authorize(root, task.task_id, delegation, execute=lambda *args: (0, answer(args[-1], action)))
    assert len(store.decisions(task.task_id)) == 1


def test_partial_authorization_followed_by_deferral_produces_no_grant(setup):
    root, store, task, _, delegation = setup
    discover(setup, task.subjects[0].entity_id)
    store.discover(task.task_id, request_id="second", author="worker", hypothesis="Second concern",
                   affects=[task.subjects[0].entity_id], evidence=["owner"], expected_revision=1)
    before, calls = store.decisions(task.task_id), []

    def execute(command, root, name, payload):
        calls.append(payload)
        selected = "layout" if len(calls) == 1 else "second"
        return 0, answer(payload, "investigate" if len(calls) == 1 else "defer", discovery_id=selected)

    result = authorize(root, task.task_id, delegation, execute=execute)
    assert result["status"] == "deferred" and "authorization" not in result
    assert len(calls) == 2 and store.decisions(task.task_id) == before


@pytest.mark.parametrize("outcome", ["budget", "unavailable"])
def test_budget_or_provider_failure_cannot_grant_a_partial_round(setup, outcome):
    root, store, task, _, delegation = setup
    discover(setup, task.subjects[0].entity_id)
    store.discover(task.task_id, request_id="second", author="worker", hypothesis="Second concern",
                   affects=[task.subjects[0].entity_id], evidence=["owner"], expected_revision=1)
    delegation["max_decisions"] = 1
    calls = []

    def execute(command, root, name, payload):
        calls.append(payload)
        return (3, {"error": "provider unavailable"}) if outcome == "unavailable" else (0, answer(payload, "investigate"))

    result = authorize(root, task.task_id, delegation, execute=execute)
    assert result["status"] == "deferred" and "authorization" not in result
    assert len(calls) == 1 and len(store.decisions(task.task_id)) == 2


@pytest.mark.parametrize("damage", ["author", "task", "root", "epoch", "coverage", "duplicate",
                                    "resolution", "published", "acceptance", "measured"])
def test_grant_fields_cannot_widen_authority_or_turn_investigation_into_acceptance(setup, damage):
    root, store, task, _, _ = setup
    discover(setup, task.subjects[0].entity_id)
    grant = deepcopy(grant_for(setup))
    if damage in ("author", "task", "root"):
        grant[damage] = "another"
    elif damage == "epoch": grant["epoch"] = "f" * 64
    elif damage == "coverage": grant["investigations"] = []
    elif damage == "duplicate": grant["investigations"] *= 2
    else:
        field, value = {"resolution": ("action", "incorporate"), "published": ("published", True),
                        "acceptance": ("ready_for_collection", True), "measured": ("verification", "pass")}[damage]
        grant["investigations"][0][field] = value
    with pytest.raises(ValueError):
        run_grant(root, task, grant, lambda *args: pytest.fail("Damaged grant reached execution"))
    assert len(store.decisions(task.task_id)) == 1


def test_no_pending_findings_need_no_provider_or_grant(setup):
    root, _, task, _, delegation = setup
    result = authorize(root, task.task_id, delegation,
                       execute=lambda *args: pytest.fail("No finding requires no provider call"))
    assert result["status"] == "no_pending" and "authorization" not in result


@pytest.mark.parametrize("options", [{"iterations": 2}, {"verify_only": True},
                                      {"coordinator_configuration": {}}, {"integration_configuration": {}}])
def test_grant_cannot_be_repurposed_for_multiple_rounds_coordination_or_publication(setup, options):
    root, _, task, _, _ = setup
    discover(setup, task.subjects[0].entity_id)
    grant = grant_for(setup)
    kwargs = {"iterations": 1, **options}
    with pytest.raises(ValueError):
        drive(root, task.task_id, driver_configuration(task), role("worker"),
              investigation_configuration=grant, execute=lambda *args: pytest.fail("Invalid grant mode executed"), **kwargs)


def test_late_coordinator_source_mutation_is_retained_but_never_authorized(setup, monkeypatch):
    import match_recovery.investigation_round as implementation

    root, store, task, _, delegation = setup
    discover(setup, task.subjects[0].entity_id)
    original = implementation.coordinate

    def late_mutation(*args, **kwargs):
        result = original(*args, **kwargs)
        path = root / "code/owner.cc"
        path.write_text(path.read_text() + "// late unauthorized change retained\n")
        return result

    monkeypatch.setattr(implementation, "coordinate", late_mutation)
    with pytest.raises(ValueError):
        authorize(root, task.task_id, delegation,
                  execute=lambda *args: (0, answer(args[-1], "investigate")))
    assert "late unauthorized change retained" in (root / "code/owner.cc").read_text()
    assert len(store.decisions(task.task_id)) == 1


@pytest.mark.parametrize("stage", ["context", "check", "build"])
@pytest.mark.parametrize("damage", ["discovery", "resolution", "expansion", "source"])
def test_ignored_input_mutation_during_prechecks_expires_grant_before_worker(setup, stage, damage):
    from match_recovery.driver import working_state

    root, store, task, caller, _ = setup
    ignore = root / ".gitignore"
    ignore.write_text(ignore.read_text() + "recovery/history/\ncode/ignored.hpp\n")
    git(root, "add", ".gitignore")
    git(root, "commit", "-m", "Ignore synthetic decision and source inputs")
    source = root / "code/ignored.hpp"
    source.write_text("void ignored_input();\n")
    discover(setup, task.subjects[0].entity_id)
    grant = grant_for(setup)
    initial_scope = store.scope(task.task_id)
    before = working_state(root)
    calls = []

    def execute(command, checkout, name, payload):
        command_name = command.argv[0]
        calls.append(command_name)
        assert command_name in ("context", "check", "build"), "Stale grant reached worker or acceptance"
        if command_name == stage:
            if damage == "discovery":
                store.discover(task.task_id, request_id="late-finding", author="worker",
                               hypothesis="Late finding invalidates prior investigation",
                               affects=[task.subjects[0].entity_id], evidence=["source"], expected_revision=1)
            elif damage == "resolution":
                store.resolve(task.task_id, request_id="late-resolution", author=task.author,
                              discovery_id="layout", disposition="incorporated",
                              source_evidence=["Late source interpretation"], expected_revision=1)
            elif damage == "expansion":
                store.expand(task.task_id, request_id="late-expansion", author=task.author,
                             subjects=[caller], editable_files=["code/caller.cc"],
                             evidence=["Late ownership interpretation"], expected_revision=1)
            else:
                source.write_text("void changed_ignored_input();\n")
            # Ordinary Git working-state checks cannot detect this mutation.
            assert working_state(root) == before
        return (0, {}) if command_name == "context" else (0, {"result": "pass"})

    with pytest.raises(ValueError):
        run_grant(root, task, grant, execute)
    assert calls and calls[-1] in ("context", "check", "build")
    assert stage in calls and "worker" not in calls
    assert working_state(root) == before
    assert store.read(task.task_id).base_revision == task.base_revision
    assert (root / "build/recovery/investigation-uses" / (grant["request_id"] + ".json")).is_file()
    if damage == "source":
        assert source.read_text() == "void changed_ignored_input();\n"
        assert len(store.decisions(task.task_id)) == 1
    else:
        assert len(store.decisions(task.task_id)) == 2  # Preserve the late decision, never roll it back.
    assert (store.scope(task.task_id) != initial_scope) == (damage == "expansion")


@pytest.mark.parametrize("acceptance_scope", ["change", "full_project"])
@pytest.mark.parametrize("collision", ["worker", "reviewer"])
def test_investigation_coordinator_is_independent_before_consumption(setup, monkeypatch, acceptance_scope, collision):
    import match_recovery.investigation_round as implementation

    root, store, task, _, _ = setup
    discover(setup, task.subjects[0].entity_id)
    grant = grant_for(setup)
    before = store.decisions(task.task_id)
    worker = role(task.author if collision == "worker" else "worker")
    reviewer = role(task.author if collision == "reviewer" else "reviewer")
    monkeypatch.setattr(implementation, "consume", lambda *args:
                        pytest.fail("Conflicting coordinator identity consumed investigation permission"))
    with pytest.raises(ValueError):
        drive(root, task.task_id, driver_configuration(task, acceptance_scope), worker,
              iterations=1, reviewer_configuration=reviewer, investigation_configuration=grant,
              execute=lambda *args: pytest.fail("Conflicting coordinator identity reached execution"))
    assert not (root / "build/recovery/investigation-uses").exists()
    assert store.decisions(task.task_id) == before
