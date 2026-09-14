"""Public residual authority against real task history and physical link inputs.

Only the synthetic compiler's receipt construction is supplied by fixtures.
No provider/compiler executes and no real project Git state is changed.
"""

from copy import deepcopy
from dataclasses import replace
import json
from pathlib import Path
import subprocess

import pytest

from match_recovery.build_graph import ApplicationGraph
from match_recovery import build_graph as graph_native
from match_recovery import generated_code_residual as native
from match_recovery.model import Origin
from match_recovery.source_history import revision_files
from match_recovery.source_replica import replica_snapshot
from match_recovery.tasks import Task, TaskStore
from test_slot_local_initializer_order import physical_context


UNIT = "src/unit0.cpp"
ORIGIN = Origin("a" * 64, "pe:va", 0x401100)


def git(root, *args):
    return subprocess.run(["git", "-C", str(root), *args], check=True, capture_output=True, text=True).stdout.strip()


def project_configuration():
    return {"schema_version": 1, "build": "recovery/build.json",
            "source": {"image_pin": "recovery/image.pin", "address_space": "pe:va",
                       "roots": ["src"], "seed_roots": ["src/seeds"]},
            "measurement": {"backend": "pe-coff-i386", "compiler_profile": "synthetic",
                            "flag_sets": [["/c", "/O2", "/Gy", "/GX"]], "layout_flags": [],
                            "unit_extensions": [".cpp"], "inputs": [], "input_roots": []}}


def source(value):
    return ("// ORIGINAL: 0x00401100 _anchor 0x00401100-0x00401110\n"
            f"int source_anchor() {{ return {value}; }}\n").encode()


@pytest.fixture
def authorized(monkeypatch, tmp_path):
    config = json.dumps(project_configuration()).encode()
    baseline = physical_context(monkeypatch, tmp_path / "baseline", code=b"\xe8" + bytes(4) + b"\xc3", reference="_dep",
        source_files={UNIT: source(1), "recovery/project.json": config})
    current = physical_context(monkeypatch, tmp_path / "current", code=b"\xe8" + bytes(4) + b"\x90\xc3", reference="_dep",
        source_files={UNIT: source(2), "recovery/project.json": config})
    root = tmp_path / "coordinator"
    root.mkdir()
    (root / "recovery").mkdir()
    (root / "recovery/project.json").write_bytes(config)
    (root / "recovery/image.pin").write_text("sha256 " + "a" * 64 + "\n")
    for path in (baseline.root / "src").rglob("*"):
        if path.is_file():
            target = root / path.relative_to(baseline.root)
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(path.read_bytes())
    git(root, "init", "-q")
    git(root, "config", "user.name", "Synthetic coordinator")
    git(root, "config", "user.email", "fixture@example.invalid")
    git(root, "add", "src", "recovery")
    git(root, "commit", "-qm", "Synthetic original source baseline")
    revision = git(root, "rev-parse", "HEAD")
    task = Task("generated-owner", "synthetic", "coordinator", "Authorize local initializer spelling change",
                revision, (ORIGIN,), (UNIT,), ("synthetic original source anchor",))
    store = TaskStore(root)
    store.create(task)
    (root / UNIT).write_bytes(source(2))
    sources = {"baseline": revision_files(root, revision, ("src",)),
               "current": {key: value for key, value in replica_snapshot(root, ("src",)).items() if value is not None}}
    changes = [{"entity_id": ORIGIN.entity_id, "unit": UNIT, "version": version,
                "symbol": "_$E1", "offset": 0, "evidence": ["independent synthetic local-root intent"]}
               for version in ("baseline", "current")]
    permission = {"migration_id": "generated-root-change", "author": task.author,
                  "evidence": ["synthetic coordinator decision"], "allowed": [], "generated_changes": changes}
    authority = {"task": task.to_dict(), "decisions": store.decisions(task.task_id), "integration": None, "permission": permission}
    expected_sources = deepcopy(sources["current"])
    expected_task = deepcopy(task.to_dict())
    state = {"alive": True, "checks": 0}

    def fresh():
        state["checks"] += 1
        if not state["alive"]:
            raise ValueError("synthetic application graph expired")
        baseline.check_current()
        current.check_current()
        if ({key: value for key, value in replica_snapshot(root, ("src",)).items() if value is not None} != expected_sources
                or store.read(task.task_id).to_dict() != expected_task or store.decisions(task.task_id)):
            raise ValueError("synthetic coordinator epoch changed")

    graph = ApplicationGraph(root, task.task_id, revision, ("src",), baseline, current, sources,
                             {"result": "unknown"}, authority, fresh)
    binding = graph_native._bound_graph(graph)
    binding.__enter__()
    try:
        yield graph, state
    finally:
        # Corruption cases deliberately leave their inputs damaged. Their
        # public consumption refusal is asserted in the test, not cleanup.
        try:
            binding.__exit__(None, None, None)
        except ValueError:
            pass


def test_public_wrapper_rederives_authorized_source_history_and_native_evidence(authorized):
    graph, state = authorized
    result = native.compare_generated_code_residual(graph)
    assert result["result"] == "pass", result
    assert result["omitted_bodies"] == {"baseline": 1, "current": 1}
    assert result["preserved_producers"] == 1  # The root's helper is still compared.
    assert result["remaining_linked_effects"] == "unknown" and not result["ready_for_collection"]
    assert graph.report == {"result": "unknown"}
    assert state["checks"] >= 2
    assert TaskStore(graph.root).decisions(graph.task_name) == ()


@pytest.mark.parametrize("damage", ["missing_permission", "permission_author", "permission_entity", "permission_unit",
                                    "permission_version", "permission_offset_bool", "authority_task", "authority_decisions",
                                    "baseline", "roots", "source_baseline", "source_current"])
def test_altered_authority_or_source_copies_cannot_grant_body_omission(authorized, damage):
    graph, _ = authorized
    if damage == "missing_permission": graph._authorization.pop("permission")
    elif damage == "permission_author": graph._authorization["permission"]["author"] = "worker"
    elif damage.startswith("permission_"):
        row = graph._authorization["permission"]["generated_changes"][0]
        key, value = {"permission_entity": ("entity_id", "entity-" + "b" * 64),
                      "permission_unit": ("unit", "src/dependency.cpp"),
                      "permission_version": ("version", "invented"),
                      "permission_offset_bool": ("offset", False)}[damage]
        row[key] = value
    elif damage == "authority_task": graph._authorization["task"]["intent"] = "different intent"
    elif damage == "authority_decisions": graph._authorization["decisions"] = ({"invented": True},)
    elif damage == "baseline": graph = replace(graph, baseline_revision="b" * 40)
    elif damage == "roots": graph = replace(graph, roots=("src", "unowned"))
    elif damage == "source_baseline": graph._sources["baseline"][UNIT] = source(99)
    else: graph._sources["current"][UNIT] = source(99)
    with pytest.raises(ValueError): native.compare_generated_code_residual(graph)


@pytest.mark.parametrize("damage", ["source", "task", "decision", "expired"])
def test_live_coordinator_changes_refuse_before_any_residual_can_pass(authorized, damage):
    graph, state = authorized
    if damage == "source": (graph.root / UNIT).write_bytes(source(99))
    elif damage == "task":
        path = graph.root / "recovery/tasks" / (graph.task_name + ".json")
        value = json.loads(path.read_text())
        value["intent"] = "changed task"
        path.write_text(json.dumps(value))
    elif damage == "decision":
        TaskStore(graph.root).discover(graph.task_name, request_id="new-finding", author="reviewer",
            hypothesis="New structural evidence", affects=(ORIGIN.entity_id,), evidence=("synthetic source finding",),
            expected_revision=0)
    else: state["alive"] = False
    with pytest.raises(ValueError): native.compare_generated_code_residual(graph)


@pytest.mark.parametrize("damage", ["source", "expired", "physical_object", "valid_permission", "permission_bool_offset"])
def test_final_graph_guard_refuses_changes_after_native_comparison(authorized, monkeypatch, damage):
    graph, state = authorized
    operation = native._compare_contexts
    def changed(*args, **kwargs):
        result = operation(*args, **kwargs)
        assert result["result"] == "pass", result
        if damage == "source": (graph.root / UNIT).write_bytes(source(99))
        elif damage == "expired": state["alive"] = False
        elif damage == "valid_permission": graph._authorization["permission"]["evidence"] = ["new valid decision"]
        elif damage == "permission_bool_offset": graph._authorization["permission"]["generated_changes"][0]["offset"] = False
        else:
            path = graph.current.objects[0]
            path.write_bytes(path.read_bytes() + b"changed")
        return result
    monkeypatch.setattr(native, "_compare_contexts", changed)
    with pytest.raises(ValueError): native.compare_generated_code_residual(graph)


def test_serialized_report_is_not_a_live_authority(authorized):
    graph, _ = authorized
    for value in (graph.report, graph.authorization, {"result": "pass", "ready_for_collection": True}):
        with pytest.raises(ValueError, match="live native"):
            native.compare_generated_code_residual(value)


@pytest.mark.parametrize("replacement", ["same_fields", "valid_permission", "baseline", "current", "guard"])
def test_replaced_graph_cannot_borrow_another_instances_live_binding(authorized, replacement):
    graph, _ = authorized
    kwargs = {}
    if replacement == "valid_permission":
        authority = graph.authorization
        authority["permission"]["migration_id"] = "another-valid-decision"
        authority["permission"]["evidence"] = ["different but schema-valid coordinator intent"]
        kwargs["_authorization"] = authority
    elif replacement == "baseline": kwargs["baseline"] = graph.current
    elif replacement == "current": kwargs["current"] = graph.baseline
    elif replacement == "guard": kwargs["check_current"] = lambda: None
    forged = replace(graph, **kwargs)
    assert forged is not graph
    with pytest.raises(ValueError): native.compare_generated_code_residual(forged)


@pytest.mark.parametrize("field", ["migration_id", "evidence", "selection_evidence", "allowed"])
def test_still_valid_permission_mutation_invalidates_its_original_binding(authorized, field):
    graph, _ = authorized
    permission = graph._authorization["permission"]
    if field == "migration_id": permission[field] = "another-valid-decision"
    elif field == "evidence": permission[field] = ["different valid evidence"]
    elif field == "selection_evidence": permission["generated_changes"][0]["evidence"] = ["new selection intent"]
    else: permission["allowed"] = [{"entity_id": ORIGIN.entity_id, "dimension": "bytes"}]
    # These remain legal schema; refusal must come from epoch identity rather
    # than the ordinary permission parser rejecting malformed data.
    from match_recovery.migration_protocol import permissions
    task = TaskStore(graph.root).read(graph.task_name)
    permissions(permission, task, frozenset({ORIGIN.entity_id}))
    with pytest.raises(ValueError): native.compare_generated_code_residual(graph)


@pytest.mark.parametrize("field", ["baseline", "current", "check_current"])
def test_mutating_bound_context_or_guard_identity_cannot_refresh_authority(authorized, field):
    graph, _ = authorized
    value = (lambda: None) if field == "check_current" else graph.current if field == "baseline" else graph.baseline
    object.__setattr__(graph, field, value)
    with pytest.raises(ValueError): native.compare_generated_code_residual(graph)


def test_expired_registry_binding_refuses_with_a_still_live_underlying_build(authorized):
    graph, _ = authorized
    # Construct a separate test-owned native epoch. Merely copying the session
    # grants nothing; only this explicit private factory boundary binds it.
    other = replace(graph)
    with graph_native._bound_graph(other):
        assert native.compare_generated_code_residual(other)["result"] == "pass"
    other.check_current()  # Original build/source guards still pass.
    with pytest.raises(ValueError): native.compare_generated_code_residual(other)
