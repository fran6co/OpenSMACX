"""Route freshness and explicit replica lineage cannot resurrect resolved work."""

from copy import deepcopy
from dataclasses import replace
import json

import pytest

from match_recovery.model import digest
from match_recovery.task_routes import incoming_routes, route_task
from match_recovery.tasks import TaskStore
from test_coordination import setup
from test_source_configuration import configured
from test_task_source import project
from test_task_routes import routed


def replica(root, store, task):
    result = TaskStore(root)
    result.create(task)
    for event in store.journal.read("task:" + task.task_id):
        result.journal.append(event.stream, request_id=event.request_id,
                              expected_revision=event.sequence - 1, payload=event.payload)
    return result


def rewrite_discovery(store, name):
    path = store.journal.stream_directory("task:" + name) / "00000001.json"
    record = json.loads(path.read_text())
    record["payload"]["hypothesis"] = "New valid source evidence at the same revision"
    record.pop("event_id")
    record["event_id"] = digest(record)
    path.write_text(json.dumps(record))


@pytest.mark.parametrize("damage", ["source_task", "source_history"])
def test_last_target_observation_cannot_publish_against_changed_source_authority(routed, monkeypatch, damage):
    import match_recovery.task_routes as module
    root, store, source, target, _, decision = routed
    real = module.target_reference
    calls, changed = [], []
    def observe(selected, name):
        result = real(selected, name)
        if name == target.task_id:
            calls.append(name)
            # Target validation reads twice, then route_task retains its owner
            # reference and finally observes it again before source publication.
            if len(calls) == 4:
                if damage == "source_task":
                    path = root / "recovery/tasks" / (source.task_id + ".json")
                    path.write_text(json.dumps(replace(source, author="changed-coordinator").to_dict()))
                else:
                    rewrite_discovery(store, source.task_id)
                changed.append(True)
        return result
    monkeypatch.setattr(module, "target_reference", observe)
    with pytest.raises(ValueError):
        route_task(root, source.task_id, decision)
    assert changed == [True]
    events = store.journal.read("task:" + source.task_id)
    assert len(events) == 1 and events[0].payload["kind"] == "discovery"
    assert store.decisions(target.task_id)[0]["kind"] == "discovery"


def test_source_history_pin_is_rechecked_inside_append_not_only_before_it(routed, monkeypatch):
    from match_recovery.journal import Journal
    root, store, source, _, _, decision = routed
    real = Journal.append
    changed = []
    def append(journal, stream, **kwargs):
        if stream == "task:" + source.task_id:
            rewrite_discovery(store, source.task_id)
            changed.append(True)
        return real(journal, stream, **kwargs)
    monkeypatch.setattr(Journal, "append", append)
    with pytest.raises(ValueError):
        route_task(root, source.task_id, decision)
    assert changed == [True]
    assert len(store.journal.read("task:" + source.task_id)) == 1


@pytest.mark.parametrize("newest_first", [False, True])
def test_resolved_descendant_overrides_stale_pending_replica_in_either_order(routed, tmp_path_factory, newest_first):
    root, store, source, target, _, decision = routed
    route_task(root, source.task_id, decision)
    stale = replica(tmp_path_factory.mktemp("stale-route"), store, source)
    newer = replica(tmp_path_factory.mktemp("resolved-route"), store, source)
    newer.resolve(source.task_id, discovery_id="relationship", request_id="resolved",
                  author=source.author, disposition="rejected", source_evidence=["Explicit newer review"],
                  expected_revision=2)
    sources = [newer, stale] if newest_first else [stale, newer]
    assert incoming_routes(store, target.task_id, sources=sources) == []
    assert len(stale.decisions(source.task_id)) == 2 and len(newer.decisions(source.task_id)) == 3
    assert store.status(source.task_id, store.scope(source.task_id)[0])["pending_discoveries"]


def test_identical_explicit_source_replicas_produce_only_one_request(routed, tmp_path_factory):
    root, store, source, target, _, decision = routed
    route_task(root, source.task_id, decision)
    a = replica(tmp_path_factory.mktemp("route-copy-a"), store, source)
    b = replica(tmp_path_factory.mktemp("route-copy-b"), store, source)
    result = incoming_routes(store, target.task_id, sources=[a, b, a])
    assert len(result) == 1 and result[0]["route"] == decision["route"]


@pytest.mark.parametrize("reverse", [False, True])
def test_divergent_source_route_tails_refuse_instead_of_duplicate_requests(routed, tmp_path_factory, reverse):
    _, store, source, target, _, decision = routed
    branches = [replica(tmp_path_factory.mktemp("route-fork"), store, source) for _ in range(2)]
    for index, branch in enumerate(branches):
        payload = deepcopy(decision["route"])
        payload["evidence"] = [f"Conflicting branch decision {index}"]
        branch.journal.append("task:" + source.task_id, request_id=f"branch-route-{index}",
                              expected_revision=1, payload=payload)
        assert len(branch.decisions(source.task_id)) == 2
    with pytest.raises(ValueError):
        incoming_routes(store, target.task_id, sources=branches[::-1] if reverse else branches)


def test_same_source_task_id_with_conflicting_immutable_task_definition_refuses(routed, tmp_path_factory):
    _, store, source, target, _, _ = routed
    a = replica(tmp_path_factory.mktemp("source-definition-a"), store, source)
    b = replica(tmp_path_factory.mktemp("source-definition-b"), store, source)
    path = b.root / "recovery/tasks" / (source.task_id + ".json")
    path.write_text(json.dumps(replace(source, intent="Conflicting immutable task definition").to_dict()))
    with pytest.raises(ValueError):
        incoming_routes(store, target.task_id, sources=[a, b])


@pytest.mark.parametrize("damage", ["resolution", "new_task"])
def test_later_source_observation_cannot_hide_changes_to_an_earlier_store(routed, tmp_path_factory, monkeypatch, damage):
    import match_recovery.task_routes as module
    root, store, source, target, _, decision = routed
    route_task(root, source.task_id, decision)
    earlier = replica(tmp_path_factory.mktemp("earlier-source"), store, source)
    later = TaskStore(tmp_path_factory.mktemp("later-source"))
    other = replace(source, task_id="another-source", intent="Independent context observer")
    later.create(other)
    real = module.target_reference
    changed = []
    def observe(selected, name):
        result = real(selected, name)
        if selected.root == later.root and not changed:
            if damage == "resolution":
                earlier.resolve(source.task_id, discovery_id="relationship", request_id="during-observation",
                                author=source.author, disposition="rejected", expected_revision=2,
                                source_evidence=["Explicit resolution after earlier observation"])
            else:
                earlier.create(replace(source, task_id="newly-visible-task"))
            changed.append(True)
        return result
    monkeypatch.setattr(module, "target_reference", observe)
    with pytest.raises(ValueError):
        incoming_routes(store, target.task_id, sources=[earlier, later])
    assert changed == [True]
    assert len(store.decisions(source.task_id)) == 2
