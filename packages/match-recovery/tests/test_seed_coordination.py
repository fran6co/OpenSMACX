"""Coordinator-controlled rehoming of an existing seed identity."""

import hashlib
import json

import pytest

from match_recovery.coordination import coordinate, validate_response
from match_recovery.model import Origin
from match_recovery.source_inventory import source_snapshot
from match_recovery.tasks import Task, TaskStore
from match_recovery.binding import bind
from match_recovery.agent_protocol import response_schema
from test_binding import parameters, project
from test_task_source import git


def _seed_marker(address=0x401000, end=0x401001, name="_seed"):
    return (f"// ORIGINAL: 0x{address:08X} {name} "
            f"0x{address:08X}-0x{end:08X}\nvoid seed() {{}}\n")


def _prepare(project, *, product_first=False, conflicting_seed=False):
    args = parameters(project)
    if product_first:
        # Exercise the real binder so the later coordinator call sees a
        # genuine product/seed identity conflict.
        bind(project, **args)

    config_path = project / "recovery/project.json"
    config = json.loads(config_path.read_text())
    config["source"]["seed_roots"] = ["src/seeds"]
    config_path.write_text(json.dumps(config))
    (project / "src/seeds").mkdir()
    (project / "src/seeds/seed.cpp").write_text(_seed_marker())
    if conflicting_seed:
        (project / "src/seeds/other.cpp").write_text(_seed_marker(end=0x401002))
    git(project, "add", ".")
    git(project, "commit", "-m", "Add selected seed identity")

    snapshot = source_snapshot(project)
    seed = next(row for row in snapshot["records"] if row["path"] == "src/seeds/seed.cpp")
    origin = Origin(**seed["origin"])
    task = Task("owner", "binding-control", "coordinator", "Rehome selected seed", git(project, "rev-parse", "HEAD"),
                (origin,), ("src/f.cpp",), ("seed identity evidence",))
    store = TaskStore(project)
    store.create(task)
    store.discover(task.task_id, request_id="seed", author="worker",
                   hypothesis="The selected seed belongs to the existing product TU",
                   affects=[origin.entity_id], evidence=["seed/src/seeds/seed.cpp"], expected_revision=0)
    delegation = {
        "schema_version": 1, "author": "coordinator",
        "command": {"argv": ["coordinate"], "timeout": 10},
        "subjects": [origin.entity_id], "editable_files": ["src/f.cpp"],
        "evidence": ["Coordinator is authorized to bind this selected seed identity"],
        "max_decisions": 1, "seed_bindings": {"image": "original.exe"},
    }
    return project, store, task, origin, delegation


def _response(payload, origin, project, **binding_overrides):
    proposal = {
        "entity_id": origin.entity_id,
        "source": "src/f.cpp",
        "line": 1,
        "source_sha256": hashlib.sha256((project / "src/f.cpp").read_bytes()).hexdigest(),
        "name": "_f",
        "reason": "The worker body is in the existing product translation unit.",
        "evidence": ["image:0x00401000-0x00401001; src/seeds/seed.cpp maps the product function _f"],
    }
    proposal.update(binding_overrides)
    return {
        "snapshot": payload["snapshot"], "action": "bind_seed", "discovery_id": "seed",
        "subjects": [], "editable_files": [],
        "evidence": ["worker evidence: selected seed spans match the product body"],
        "binding": proposal,
    }


def test_bind_seed_uses_real_binder_and_preserves_seed_pending_baseline(project):
    root, store, task, origin, delegation = _prepare(project)
    seed_path = root / "src/seeds/seed.cpp"
    seed_before = seed_path.read_bytes()
    head_before = git(root, "rev-parse", "HEAD")
    task_before = store.read(task.task_id)

    result = coordinate(root, task.task_id, delegation,
                        execute=lambda command, root, name, payload:
                        (0, _response(payload, origin, root)))

    assert result["action"] == "bind_seed" and result["published"]
    assert result["ready_for_collection"] is False
    assert result["verification"] == "not_measured"
    assert result["binding"]["entity_id"] == origin.entity_id
    assert result["binding"]["binding_added"] is True
    assert result["binding"]["verification"] == "not_measured"
    assert root.joinpath("src/f.cpp").read_text().startswith("// ORIGINAL: 0x00401000 _f")
    assert seed_path.read_bytes() == seed_before
    assert git(root, "rev-parse", "HEAD") == head_before
    assert store.read(task.task_id) == task_before
    assert len(store.decisions(task.task_id)) == 1
    assert store.status(task.task_id, {origin.entity_id})["pending_discoveries"]
    product = next(row for row in source_snapshot(root)["records"] if row["path"] == "src/f.cpp")
    assert product["entity_id"] == origin.entity_id
    assert product["binding_evidence"]["evidence"] == _response({"snapshot": "unused"}, origin, root)["binding"]["evidence"]
    assert product["binding_evidence"]["evidence"] != result["evidence"]


def test_binding_keeps_decision_diagnostics_out_of_source(project):
    root, store, task, origin, delegation = _prepare(project)
    decision = ["request hash " + "a" * 64, "2844 impacted entities; compiler failed; permission checked"]
    binding_evidence = ["image:0x00401000-0x00401001", "src/seeds/seed.cpp: original _seed corresponds to product _f"]

    def execute(command, root, name, payload):
        response = _response(payload, origin, root, evidence=binding_evidence)
        response["evidence"] = decision
        return 0, response

    result = coordinate(root, task.task_id, delegation, execute=execute)
    product = next(row for row in source_snapshot(root)["records"] if row["path"] == "src/f.cpp")
    assert product["binding_evidence"]["evidence"] == binding_evidence
    assert result["evidence"] == decision
    assert all(item not in (root / "src/f.cpp").read_text() for item in decision)
    assert store.status(task.task_id, {origin.entity_id})["pending_discoveries"]


@pytest.mark.parametrize("evidence", [None, [], "image:0x401000", [1], [""], ["same", "same"],
                                      ["line\nbreak"], ["ORIGINAL: injected"]])
def test_invalid_binding_evidence_refuses_before_publication(project, evidence):
    root, store, task, origin, delegation = _prepare(project)
    before = (root / "src/f.cpp").read_bytes()
    decisions = store.decisions(task.task_id)
    with pytest.raises(ValueError):
        coordinate(root, task.task_id, delegation,
                   execute=lambda *args: (0, _response(args[-1], origin, root, evidence=evidence)))
    assert (root / "src/f.cpp").read_bytes() == before
    assert store.decisions(task.task_id) == decisions


def test_binding_requires_both_evidence_channels(project):
    root, store, task, origin, delegation = _prepare(project)
    response = _response({"snapshot": "a" * 64}, origin, root)
    response["binding"].pop("evidence")
    with pytest.raises(ValueError, match="separate binding evidence"):
        validate_response(response, "a" * 64)
    response = _response({"snapshot": "a" * 64}, origin, root)
    response["evidence"] = []
    with pytest.raises(ValueError, match="decision needs evidence"):
        validate_response(response, "a" * 64)


def test_coordinator_schema_requires_separate_binding_evidence():
    properties = response_schema("coordinator")["properties"]
    binding = next(item for item in properties["binding"]["anyOf"] if item["type"] == "object")
    assert "evidence" in binding["required"]
    assert binding["properties"]["evidence"]["minItems"] == 1
    assert properties["evidence"]["minItems"] == 1


@pytest.mark.parametrize("damage", ["missing_policy", "unknown_identity", "outside_editable", "stale_digest"])
def test_bind_seed_refusals_leave_source_task_and_baseline_unchanged(project, damage):
    root, store, task, origin, delegation = _prepare(project)
    if damage == "missing_policy":
        delegation.pop("seed_bindings")
    before = {path: path.read_bytes() for path in (root / "src").rglob("*") if path.is_file()}
    task_before = store.decisions(task.task_id)
    head_before = git(root, "rev-parse", "HEAD")

    def execute(command, root, name, payload):
        if damage == "unknown_identity":
            return 0, _response(payload, Origin(origin.image_sha256, origin.address_space, 0x401010), root)
        if damage == "outside_editable":
            return 0, _response(payload, origin, root, source="src/not-editable.cpp")
        if damage == "stale_digest":
            return 0, _response(payload, origin, root, source_sha256="b" * 64)
        return 0, _response(payload, origin, root)

    with pytest.raises(ValueError):
        coordinate(root, task.task_id, delegation, execute=execute)
    assert before == {path: path.read_bytes() for path in (root / "src").rglob("*") if path.is_file()}
    assert task_before == store.decisions(task.task_id)
    assert store.read(task.task_id).base_revision == task.base_revision
    assert git(root, "rev-parse", "HEAD") == head_before


def test_bind_seed_refuses_conflicting_seed_spans(project):
    root, store, task, origin, delegation = _prepare(project, conflicting_seed=True)
    before = {path: path.read_bytes() for path in (root / "src").rglob("*") if path.is_file()}

    with pytest.raises(ValueError, match="agreeing existing original spans"):
        coordinate(root, task.task_id, delegation,
                   execute=lambda command, root, name, payload:
                   (0, _response(payload, origin, root)))
    assert before == {path: path.read_bytes() for path in (root / "src").rglob("*") if path.is_file()}

def test_bind_seed_refuses_existing_product(project):
    # A real product binding causes the native seed-only guard to refuse.
    root, store, task, origin, delegation = _prepare(project, product_first=True)
    before = {path: path.read_bytes() for path in (root / "src").rglob("*") if path.is_file()}
    with pytest.raises(ValueError, match="seed-only identity"):
        coordinate(root, task.task_id, delegation,
                   execute=lambda command, root, name, payload:
                   (0, _response(payload, origin, root)))
    assert before == {path: path.read_bytes() for path in (root / "src").rglob("*") if path.is_file()}


@pytest.mark.parametrize("action", ["expand", "incorporate", "reject", "defer", "investigate"])
def test_binding_proposal_cannot_hide_in_another_action(project, action):
    root, store, task, origin, delegation = _prepare(project)
    response = _response({"snapshot": "a" * 64}, origin, root)
    response["action"] = action
    with pytest.raises(ValueError, match="only bind_seed"):
        validate_response(response, "a" * 64)
    response["binding"] = None
    validate_response(response, "a" * 64)


@pytest.mark.parametrize("damage", ["delegated_only", "not_affected"])
def test_binding_requires_current_selected_and_affected_identity(project, damage):
    root, store, task, origin, delegation = _prepare(project)
    (root / "src/seeds/other.cpp").write_text(_seed_marker(address=0x401001, end=0x401002, name="_other"))
    other = Origin(origin.image_sha256, origin.address_space, 0x401001)
    delegation["subjects"].append(other.entity_id)
    if damage == "not_affected":
        store.expand(task.task_id, request_id="other", author=task.author, subjects=[other], editable_files=[],
                     evidence=["Authorize another known identity"], expected_revision=1)
    before = (root / "src/f.cpp").read_bytes()
    with pytest.raises(ValueError, match="existing selected discovery identity"):
        coordinate(root, task.task_id, delegation, execute=lambda *args: (0, _response(args[-1], other, root)))
    assert (root / "src/f.cpp").read_bytes() == before


@pytest.mark.parametrize("damage", [None, "missing", "wrong_image", "invalid_placeholder"])
def test_seed_binding_image_delegation_is_explicit_and_pinned(project, monkeypatch, damage):
    root, store, task, origin, delegation = _prepare(project)
    delegation["seed_bindings"]["image"] = "{env:BINDING_TEST_IMAGE}"
    monkeypatch.setenv("BINDING_TEST_IMAGE", str(root / "original.exe"))
    if damage == "missing":
        monkeypatch.delenv("BINDING_TEST_IMAGE")
    elif damage == "wrong_image":
        (root / "original.exe").write_bytes((root / "original.exe").read_bytes() + b"changed")
    elif damage == "invalid_placeholder":
        delegation["seed_bindings"]["image"] = "{env:BINDING_TEST_IMAGE}/nested"
    execute = lambda *args: (0, _response(args[-1], origin, root))
    before = (root / "src/f.cpp").read_bytes()
    if damage:
        with pytest.raises(ValueError):
            coordinate(root, task.task_id, delegation, execute=execute)
        assert (root / "src/f.cpp").read_bytes() == before
    else:
        assert coordinate(root, task.task_id, delegation, execute=execute)["binding"]["binding_added"]
