"""Owner handoffs survive serial collection order without blocking unrelated work."""

import json
from pathlib import Path

import pytest

import match_recovery.campaign as campaign
from match_recovery.driver import working_state
from match_recovery.model import Origin
from match_recovery.task_routes import route_task, target_reference
from match_recovery.tasks import Task, TaskStore
from test_coordination import setup
from test_source_configuration import configured
from test_task_source import git, project


@pytest.fixture
def owner_campaign(setup):
    root, store, owner, caller, delegation = setup
    other = Origin(caller.image_sha256, caller.address_space, 48)
    (root / "code/unrelated.cc").write_text(
        "// ORIGINAL: 0x00000030 _unrelated 0x00000030-0x00000038\nvoid unrelated() {}\n")
    git(root, "add", "code")
    git(root, "commit", "-m", "Synthetic unrelated original")
    base = git(root, "rev-parse", "HEAD")
    store.create(Task("source", owner.target, owner.author, "Recover caller owner", base,
                      (caller,), ("code/caller.cc",), ("Caller ownership",)))
    store.create(Task("unrelated", owner.target, owner.author, "Recover unrelated owner", base,
                      (other,), ("code/unrelated.cc",), ("Separate ownership",)))
    store.discover("source", request_id="relationship", author="worker",
                   hypothesis="Caller and existing owner need a coordinated structural repair",
                   affects=sorted([caller.entity_id, owner.subjects[0].entity_id]),
                   evidence=["Full finding includes both caller and owner"], expected_revision=0)
    delegation.update(subjects=[], editable_files=[], route_discoveries=True)
    source_config = json.loads((root / "recovery/project.json").read_text())
    source_config["compiler"] = {"environment": ["TEST_COMPILER_SLOT"]}
    (root / "recovery/project.json").write_text(json.dumps(source_config))
    slots = []
    for number in range(3):
        slot = root / f"build/owner-slot-{number}"
        slot.mkdir(parents=True)
        slots.append({"name": f"slot-{number}", "directory": str(slot),
                      "environment": {"TEST_COMPILER_SLOT": "{resource}"}})
    command = lambda name: {"argv": [name], "timeout": 10}
    profiles = {
        "planner": {"schema_version": 1, "author": "coordinator", "command": command("planner"),
                    "max_entities": 10, "max_editable_files": 4, "commit_decision": True, "evidence": ["Test planning"]},
        "driver": {"schema_version": 1, "target": owner.target, "acceptance_scope": "change",
                   **{name: command(name) for name in ("context", "check", "obligations", "migration_check")}},
        "worker": {"author": "worker", "command": command("worker")},
        "reviewer": {"author": "reviewer", "command": command("reviewer")},
        "checkpoint": {"author": "coordinator", "message": "Synthetic checkpoint", "allow_migration": False},
        "coordinator": delegation, "resources": {"schema_version": 1, "slots": slots},
    }
    for name, value in profiles.items():
        (root / f"recovery/{name}.json").write_text(json.dumps(value))
    configuration = {"schema_version": 1, "author": "coordinator", "destination_branch": "refs/heads/work",
                     **{name: f"recovery/{name}.json" for name in profiles}, "max_batches": 1, "max_parallel": 3,
                     "iterations": 1, "timeout": 30, "evidence": ["Explicit bounded owner handoff control"],
                     "publication": {"enabled": True, "allow_migration": False, "message": "Test publication", "cleanup": True}}
    return root, store, owner, configuration


def publish_help(source, target, *, leases=None):
    store = TaskStore(source)
    finding = store.journal.read("task:source")[0]
    return route_task(source, "source", {"request_id": "help-owner", "expected_revision": 1,
        "route": {"kind": "discovery_routed", "author": "coordinator", "discovery_id": "relationship",
                  "discovery_event_id": finding.event_id, "target": target_reference(TaskStore(target), "owner"),
                  "subjects": [TaskStore(target).read("owner").subjects[0].entity_id],
                  "evidence": ["Existing owner must repair its own part; retain the original full finding"]}},
        target_root=target, _held_leases=leases)


@pytest.mark.parametrize("incoming_first", [False, True], ids=["reversed-owner-before-source", "incoming-without-local-finding"])
@pytest.mark.parametrize("checkpoint", [False, True], ids=["clean-noop", "committed-worker-progress"])
def test_serial_owner_is_preserved_before_retirement_or_integration(owner_campaign, monkeypatch, incoming_first, checkpoint):
    root, store, owner, configuration = owner_campaign
    if incoming_first:
        assert publish_help(root, root)["published"]
    git(root, "add", ".")
    git(root, "commit", "-m", "Synthetic three-owner routing campaign")
    before = working_state(root)
    finding = store.decisions("source")[0]
    histories = {name: store.decisions(name) for name in ("owner", "source", "unrelated")}
    tasks = {name: store.read(name) for name in histories}
    selected = iter(("owner", "source", "unrelated"))
    monkeypatch.setattr(campaign, "select_task", lambda *args, **kwargs:
                        {"action": "resume", "task_id": next(selected)})
    native_prepare = campaign.prepare_worker

    def prepare(current, name, authorization, **kwargs):
        # Native isolated checkouts, without claiming synthetic compiler proof.
        result = native_prepare(current, name, {**authorization, "prepare": False},
                                **{**kwargs, "resources": None})
        return {**result, "compiler_prepared": True}

    paths, driven, cleaned, owner_contexts = {}, [], [], []

    def dispatch(current, plan, **kwargs):
        paths.update({row["task"]: Path(row["root"]) for row in plan["jobs"]})
        if checkpoint:
            path = paths["owner"] / "code/owner.cc"
            path.write_text(path.read_text() + "// Synthetic scoped worker checkpoint.\n")
            git(paths["owner"], "add", "code/owner.cc")
            git(paths["owner"], "commit", "-m", "Synthetic owner progress")
        return {"coordinator_changed": False, "jobs": [{"task": row["task"], "scope_intact": True,
                "timed_out": False, "returncode": 3} for row in plan["jobs"]]}

    def drive(child, name, *args, **kwargs):
        driven.append(name)
        routing = kwargs["_routing"]
        if name == "owner":
            assert not TaskStore(child).status(name, TaskStore(child).scope(name)[0])["pending_discoveries"]
            context = routing.snapshot(child, name)
            owner_contexts.append(context)
            assert bool(context["incoming"]) is incoming_first
        if name == "source" and not incoming_first:
            # Owner ran first. Native publication now requires its checkout still
            # to exist and its peer lease still to be an actual held lease.
            assert driven == ["owner", "source"] and paths["owner"].exists()
            assert publish_help(child, paths["owner"], leases=routing.leases)["published"]
        return {"stop_reason": "no_source_progress", "ready_for_collection": False}

    native_cleanup = campaign._remove_integrated

    def cleanup(current, child, branch, directory):
        assert child == paths["unrelated"], "Neither endpoint of the pending handoff may be cleaned"
        cleaned.append(child)
        return native_cleanup(current, child, branch, directory)

    monkeypatch.setattr(campaign, "prepare_worker", prepare)
    monkeypatch.setattr(campaign, "dispatch", dispatch)
    monkeypatch.setattr(campaign, "drive", drive)
    monkeypatch.setattr(campaign, "_remove_integrated", cleanup)
    monkeypatch.setattr(campaign, "prepare_integration", lambda *args, **kwargs: pytest.fail("Pending handoff cannot integrate"))
    monkeypatch.setattr(campaign, "publish_integration", lambda *args, **kwargs: pytest.fail("Pending handoff cannot publish"))
    report = campaign.campaign(root, configuration, execute=lambda *args: pytest.fail("No real provider in scheduling control"))
    assert driven == ["owner", "source", "unrelated"] and cleaned == [paths["unrelated"]]
    assert paths["owner"].is_dir() and paths["source"].is_dir() and not paths["unrelated"].exists()
    batch, = report["batches"]
    pending = {row["task"]: row["reason"] for row in batch["pending"]}
    assert pending == {"owner": "owner_request_pending" if incoming_first else "peer_discovery_pending",
                       "source": "routed_discovery_pending"}
    assert not batch["integrations"] and not report["ready_for_collection"] and not report["full_project_accepted"]
    directory = Path(report["artifacts"])
    assert not list(directory.glob("*retired-owner.json")) and not list(directory.glob("*retired-source.json"))
    assert list(directory.glob("*retired-unrelated.json"))
    for name in ("owner", "source"):
        retained = TaskStore(paths[name])
        assert retained.read(name) == tasks[name] and retained.scope(name) == store.scope(name)
    assert TaskStore(paths["owner"]).decisions("owner") == histories["owner"] == ()
    source_store = TaskStore(paths["source"])
    assert source_store.status("source", source_store.scope("source")[0])["pending_discoveries"] == [finding]
    assert len(source_store.decisions("source")) == 2
    assert working_state(root) == before
