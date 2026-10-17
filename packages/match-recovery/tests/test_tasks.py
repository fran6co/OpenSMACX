from dataclasses import asdict, replace
import json
import os

import pytest

from match_recovery.journal import ConcurrentUpdate
from match_recovery.model import Origin, canonical, digest
from match_recovery.tasks import Task, TaskStore, task_name


ONE = Origin("a" * 64, "pe:va", 0x401000)
OTHER = Origin("a" * 64, "pe:va", 0x402000)


@pytest.fixture
def task():
    return Task("widget-layout", "synthetic", "coordinator", "Recover Widget ownership",
                "b" * 40, (ONE,), ("src/widget.cpp", "src/widget.h"), ("source:widget-evidence",))


def test_task_is_a_scope_decision_not_a_stored_measurement(task, tmp_path):
    store = TaskStore(tmp_path)
    assert store.create(task)
    assert not store.create(task)
    assert store.read(task.task_id) == task
    assert "measurements" not in task.to_dict()
    assert "status" not in task.to_dict()
    assert Task.from_dict(json.loads(canonical(task.to_dict()))) == task
    with pytest.raises(FileExistsError):
        store.create(replace(task, subjects=(ONE, OTHER)))


def test_task_store_reuses_only_unchanged_bytes_and_does_not_reparse(task, tmp_path, monkeypatch):
    import match_recovery.tasks as tasks_module

    store = TaskStore(tmp_path)
    store.create(task)
    path = tmp_path / "recovery/tasks/widget-layout.json"
    parse = tasks_module.parse_json
    calls = []

    def counted(payload):
        calls.append(payload)
        return parse(payload)

    monkeypatch.setattr(tasks_module, "parse_json", counted)
    first = store.read(task.task_id)
    assert store.read(task.task_id) is first
    assert len(calls) == 1

    before = path.stat()
    changed = path.read_bytes().replace(b'"target": "synthetic"', b'"target": "synthetix"')
    assert len(changed) == before.st_size
    path.write_bytes(changed)
    os.utime(path, ns=(before.st_atime_ns, before.st_mtime_ns))
    second = store.read(task.task_id)
    assert second is not first
    assert second.target == "synthetix"
    assert store.read(task.task_id) is second
    assert len(calls) == 2


@pytest.mark.parametrize("old, new, expected", [
    (b'"author": "coordinator",', b'"author": "coordinator", "author": "attacker",', "duplicate JSON key"),
    (b'"schema_version": 1', b'"schema_version": 2', "unsupported task schema"),
    (b'"task_id": "widget-layout"', b'"task_id": "other-task"', "filename disagrees"),
])
def test_task_store_never_uses_cached_value_after_invalid_or_mismatched_bytes(task, tmp_path, old, new, expected):
    store = TaskStore(tmp_path)
    store.create(task)
    path = tmp_path / "recovery/tasks/widget-layout.json"
    store.read(task.task_id)
    raw = path.read_bytes().replace(old, new)
    assert raw != path.read_bytes()
    path.write_bytes(raw)
    with pytest.raises(ValueError, match=expected):
        store.read(task.task_id)


def test_task_store_never_uses_cached_value_after_invalid_json(task, tmp_path):
    store = TaskStore(tmp_path)
    store.create(task)
    path = tmp_path / "recovery/tasks/widget-layout.json"
    store.read(task.task_id)
    path.write_bytes(b"{")
    with pytest.raises(json.JSONDecodeError):
        store.read(task.task_id)


def test_task_store_observes_deletion_after_cache(task, tmp_path):
    store = TaskStore(tmp_path)
    store.create(task)
    path = tmp_path / "recovery/tasks/widget-layout.json"
    store.read(task.task_id)
    path.unlink()
    with pytest.raises(FileNotFoundError):
        store.read(task.task_id)


def test_task_store_cache_is_per_instance_and_task_data_is_immutable(task, tmp_path):
    first_store, second_store = TaskStore(tmp_path), TaskStore(tmp_path)
    first_store.create(task)
    first = first_store.read(task.task_id)
    again = first_store.read(task.task_id)
    independent = second_store.read(task.task_id)
    assert again is first
    assert independent == first and independent is not first
    assert isinstance(first.subjects, tuple) and isinstance(first.evidence, tuple)
    with pytest.raises((AttributeError, TypeError)):
        first.subjects[0].address = 7
    with pytest.raises(AttributeError):
        first.subjects += (OTHER,)


def test_origin_entity_id_cache_does_not_change_identity_serialization():
    origin = Origin("a" * 64, "pe:va", 0x401000)
    encoded_before = canonical(origin)
    fields_before = asdict(origin)
    digest_before = digest(origin)
    expected = "entity-" + digest_before
    assert origin.entity_id == expected
    assert origin.entity_id == expected
    assert canonical(origin) == encoded_before
    assert asdict(origin) == fields_before
    assert digest(origin) == digest_before


@pytest.mark.parametrize("name", ["../outside", "a/b", "", "a b", "Task", "x" * 81])
def test_task_names_do_not_escape_the_store(name):
    with pytest.raises(ValueError):
        task_name(name)


def test_schema_rejects_boolean_versions_and_derived_report_fields(task):
    data = json.loads(canonical(task.to_dict()))
    with pytest.raises(ValueError):
        Task.from_dict({**data, "schema_version": True})
    with pytest.raises(ValueError):
        Task.from_dict({**data, "match_count": 100})


def discover(store, task, *, affects=(ONE.entity_id,), revision=0, request="discovery-1"):
    return store.discover(task.task_id, request_id=request, author="worker", hypothesis="This global is a member",
                          affects=affects, evidence=("source:widget.h:field",), expected_revision=revision)


def test_discoveries_survive_resumption_and_never_disappear_at_a_function_boundary(task, tmp_path):
    store = TaskStore(tmp_path)
    store.create(task)
    discovery = discover(store, task, affects=(ONE.entity_id, OTHER.entity_id))
    assert discover(store, task, affects=(ONE.entity_id, OTHER.entity_id)) == discovery
    reopened = TaskStore(tmp_path)
    status = reopened.status(task.task_id, (ONE.entity_id, OTHER.entity_id))
    assert status["next_action"] == "review_scope_expansion"
    assert status["scope_expansion_required"] == [OTHER.entity_id]
    assert len(status["pending_discoveries"]) == status["decision_revision"] == 1
    assert not status["ready_for_collection"]
    assert status["verification"] == "not_measured"
    with pytest.raises(ConcurrentUpdate):
        discover(store, task, request="stale-worker", revision=0)


def test_discovery_cannot_be_marked_incorporated_outside_authorized_scope(task, tmp_path):
    store = TaskStore(tmp_path)
    store.create(task)
    discover(store, task, affects=(OTHER.entity_id,))
    with pytest.raises(ValueError, match="beyond"):
        store.resolve(task.task_id, discovery_id="discovery-1", request_id="resolve-1", author="reviewer",
                      disposition="incorporated", source_evidence=("source:other.cpp",), expected_revision=1)


def test_resolving_a_hypothesis_does_not_declare_the_task_verified(task, tmp_path):
    store = TaskStore(tmp_path)
    store.create(task)
    discover(store, task)
    store.resolve(task.task_id, discovery_id="discovery-1", request_id="resolve-1", author="reviewer",
                  disposition="incorporated", source_evidence=("source:widget.h",), expected_revision=1)
    status = store.status(task.task_id, (ONE.entity_id,))
    assert not status["pending_discoveries"]
    assert status["next_action"] == "evaluate_current_source_and_checks"
    assert not status["ready_for_collection"]
    assert status["verification"] == "not_measured"
    store.resolve(task.task_id, discovery_id="discovery-1", request_id="resolve-1", author="reviewer",
                  disposition="incorporated", source_evidence=("source:widget.h",), expected_revision=1)
    with pytest.raises(ValueError, match="already resolved"):
        store.resolve(task.task_id, discovery_id="discovery-1", request_id="rewrite", author="reviewer",
                      disposition="rejected", source_evidence=("different guess",), expected_revision=2)


def test_disappearing_source_subject_is_not_hidden_by_a_task_record(task, tmp_path):
    store = TaskStore(tmp_path)
    store.create(task)
    status = store.status(task.task_id, ())
    assert status["missing_subjects"] == [ONE.entity_id]
    assert status["next_action"] == "repair_missing_source_mapping"


def test_scope_expansion_keeps_the_discovery_pending_and_does_not_relax_checks(task, tmp_path):
    store = TaskStore(tmp_path)
    store.create(task)
    discover(store, task, affects=(ONE.entity_id, OTHER.entity_id))
    store.expand(task.task_id, request_id="expansion-1", author=task.author, subjects=(OTHER,),
                 editable_files=("src/caller.cpp",), evidence=("source:caller.cpp:field-write",), expected_revision=1)
    status = store.status(task.task_id, (ONE.entity_id, OTHER.entity_id))
    assert status["next_action"] == "evaluate_structural_discoveries"
    assert len(status["pending_discoveries"]) == 1
    assert "src/caller.cpp" in status["editable_files"]
    assert not status["scope_expansion_required"]
    assert not status["ready_for_collection"]
    assert status["verification"] == "not_measured"
    assert TaskStore(tmp_path).scope(task.task_id)[0] == {ONE.entity_id, OTHER.entity_id}
    store.resolve(task.task_id, discovery_id="discovery-1", request_id="resolve-1", author="reviewer",
                  disposition="incorporated", source_evidence=("source:widget.h", "source:caller.cpp"), expected_revision=2)
    assert not store.status(task.task_id, (ONE.entity_id, OTHER.entity_id))["pending_discoveries"]


def test_worker_cannot_self_expand_and_another_image_cannot_be_smuggled_into_scope(task, tmp_path):
    store = TaskStore(tmp_path)
    store.create(task)
    with pytest.raises(ValueError, match="coordinator"):
        store.expand(task.task_id, request_id="expansion", author="worker", subjects=(OTHER,),
                     editable_files=(), evidence=("note",), expected_revision=0)
    with pytest.raises(ValueError, match="unrelated"):
        store.expand(task.task_id, request_id="expansion", author=task.author,
                     subjects=(replace(OTHER, image_sha256="c" * 64),), editable_files=(),
                     evidence=("note",), expected_revision=0)


def test_valid_json_cannot_smuggle_derived_claims_into_task_history(task, tmp_path):
    store = TaskStore(tmp_path)
    store.create(task)
    store.journal.append("task:" + task.task_id, expected_revision=0, request_id="fake-result",
                         payload={"kind": "measurement", "passed": True})
    with pytest.raises(ValueError, match="decision fields"):
        store.status(task.task_id, (ONE.entity_id,))
