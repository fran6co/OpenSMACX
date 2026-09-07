"""History controls for configuration-only projects and structural migrations."""

import json

import pytest

from match_recovery.cli import main
from match_recovery.layout_source import read_members
from match_recovery.generated_source import read_generated
from match_recovery.source_obligations import task_obligations
from match_recovery.tasks import Task, TaskStore
from match_recovery.model import Origin
from test_task_source import project, git, create
from test_source_configuration import configured


def commit(root, message):
    git(root, "add", ".")
    git(root, "commit", "-m", message)
    return git(root, "rev-parse", "HEAD")


def config(root, **changes):
    path = root / "recovery/project.json"
    value = json.loads(path.read_text())
    value["source"].update(changes)
    path.write_text(json.dumps(value))
    return value


@pytest.fixture
def task_project(project, capsys):
    assert create(project, "--target", "custom-game") == 0
    capsys.readouterr()
    commit(project, "Task decision")
    return project


def report(root):
    return task_obligations(root, "owner")


MEMBER = "// RECOVERY_MEMBER: v1 Owner field_ 0x4 0x10\n"
GENERATED = "// RECOVERY_GENERATED: v1 vtable 0x100000000 ? _table 0x10\n"


def test_native_context_and_obligations_require_no_adapter(task_project, capsys):
    root = task_project
    (root / "code/owner.hpp").write_text(MEMBER + GENERATED)
    for action in ("obligations", "context"):
        assert main(["tasks", action, "owner", "--root", str(root)]) == 0
        output = json.loads(capsys.readouterr().out)
        inventory = output if action == "obligations" else output["restoration"]
        assert {row["dimension"] for row in inventory["obligations"]} == {"bytes", "structure"}
        assert len(inventory["interpretations"]) == 2
        assert inventory["verification"] == "not_measured"
    member, = read_members(root, "a" * 64)
    table, = read_generated(root, "a" * 64)
    assert member.evidence.address_space == table.origin.address_space == "custom:offset"
    assert table.origin.address == 0x100000000


def test_root_and_pin_relocation_keep_deleted_claims(task_project):
    root = task_project
    before = report(root)
    (root / "code").rename(root / "engine")
    (root / "original/identity").rename(root / "engine/PIN")
    config(root, roots=["engine", "hints"], include_paths=["engine"], image_pin="engine/PIN")
    source = root / "engine/owner.cc"
    source.write_text(source.read_text().replace("BYTE_EXACT", ""))
    commit(root, "Move source and pin while restoring matching")
    after = report(root)
    assert [row["obligation_id"] for row in before["obligations"]] == [row["obligation_id"] for row in after["obligations"]]
    assert any(item["path"] == "code/owner.cc" for item in after["obligations"][0]["source_evidence"])


def test_pointer_target_refinement_and_removal_preserve_review_obligations(task_project):
    path = task_project / "code/owner.hpp"
    path.write_text(MEMBER)
    commit(task_project, "Original offset interpretation")
    initial = report(task_project)
    path.write_text(MEMBER.replace("v1", "v3").replace("field_ 0x4", "field_ Buffer* 0x4"))
    commit(task_project, "Recover pointer target interpretation")
    refined = report(task_project)
    assert {row["obligation_id"] for row in initial["obligations"]} == {row["obligation_id"] for row in refined["obligations"]}
    assert len(refined["interpretations"]) == 2
    assert any(row["current"] and row["value"].get("class_pointer_type") == "Buffer" for row in refined["interpretations"])
    path.unlink()
    commit(task_project, "Remove declaration during migration")
    removed = report(task_project)
    assert {row["obligation_id"] for row in refined["obligations"]} == {row["obligation_id"] for row in removed["obligations"]}
    assert len(removed["interpretations"]) == 2 and not any(row["current"] for row in removed["interpretations"])


def test_reclassifying_product_as_seed_does_not_erase_structural_history(task_project):
    root = task_project
    (root / "code/owner.hpp").write_text(MEMBER)
    commit(root, "Original product interpretation")
    first = report(root)
    config(root, seed_roots=["hints", "code"])
    commit(root, "Treat existing attempts as seeds")
    after = report(root)
    assert {row["obligation_id"] for row in first["obligations"]} == {row["obligation_id"] for row in after["obligations"]}
    interpretation, = after["interpretations"]
    assert not interpretation["current"]


def test_same_blob_promoted_from_seed_becomes_a_review_obligation(task_project):
    root = task_project
    (root / "hints/attempt.cc").write_text(MEMBER)
    commit(root, "Seed hypothesis")
    assert report(root)["interpretations"] == []
    config(root, seed_roots=[])
    introduced = commit(root, "Adopt source interpretation without changing its bytes")
    after = report(root)
    assert len(after["interpretations"]) == 1
    structural = next(row for row in after["obligations"] if row["dimension"] == "structure")
    assert any(row["revision"] == introduced for row in structural["source_evidence"])


def test_merged_side_history_with_different_roots_is_not_pruned(task_project):
    root = task_project
    git(root, "switch", "-c", "side")
    (root / "code").rename(root / "side_code")
    config(root, roots=["side_code", "hints"], include_paths=["side_code"])
    (root / "side_code/owner.hpp").write_text(MEMBER)
    introduced = commit(root, "Side branch ownership interpretation")
    (root / "side_code/owner.hpp").write_text("void owner();\n")
    (root / "side_code").rename(root / "code")
    config(root, roots=["code", "hints"], include_paths=["code"])
    commit(root, "Return source home while migration remains unfinished")
    git(root, "switch", "work")
    git(root, "merge", "--no-ff", "side", "-m", "Merge side history")
    structure = next(row for row in report(root)["obligations"] if row["dimension"] == "structure")
    assert any(item["revision"] == introduced and item["path"] == "side_code/owner.hpp" for item in structure["source_evidence"])


@pytest.mark.parametrize("change", ["space", "missing_config", "pin", "malformed_config"])
def test_damaged_historical_inputs_cannot_be_hidden_by_restoring_them(task_project, change):
    root = task_project
    path = root / "recovery/project.json"
    original = path.read_text()
    pin = root / "original/identity"
    if change == "space":
        config(root, address_space="wrong:space")
    elif change == "missing_config":
        path.unlink()
    elif change == "malformed_config":
        path.write_text('{"schema_version":1,"source":null}')
    else:
        pin.write_text("sha256 " + "b" * 64 + "\n")
    commit(root, "Damaged historical input")
    path.write_text(original)
    pin.write_text("sha256 " + "a" * 64 + "\n")
    commit(root, "Restore inputs")
    with pytest.raises(ValueError):
        report(root)


def test_early_history_requires_explicit_evidenced_configuration(project):
    root = project
    path = root / "recovery/project.json"
    original = json.loads(path.read_text())
    path.unlink()
    base = commit(root, "Pre-framework source baseline")
    task = Task("owner", "custom-game", "coordinator", "Recover ownership", base,
                (Origin("a" * 64, "custom:offset", 16),), ("code/owner.cc",), ("synthetic",))
    TaskStore(root).create(task)
    path.write_text(json.dumps(original))
    commit(root, "Introduce source configuration")
    with pytest.raises(ValueError, match="declare evidenced"):
        report(root)
    original["source_history"] = {"before_configuration": original["source"], "evidence": ["synthetic:" + base]}
    path.write_text(json.dumps(original))
    assert len(report(root)["obligations"]) == 1
    commit(root, "Declare legacy source interpretation")
    # The fallback cannot excuse deleting configuration once established.
    path.unlink()
    commit(root, "Remove configuration")
    path.write_text(json.dumps(original))
    commit(root, "Restore configuration")
    with pytest.raises(ValueError, match="configuration removed"):
        report(root)


def test_history_configuration_race_is_refused(task_project, monkeypatch):
    from match_recovery import source_obligations
    root = task_project
    versions = source_obligations.source_versions
    def changed(*args, **kwargs):
        rows = versions(*args, **kwargs)
        path = root / "recovery/project.json"
        value = json.loads(path.read_text())
        value["source_history"] = {"before_configuration": value["source"], "evidence": ["concurrent decision"]}
        path.write_text(json.dumps(value))
        return rows
    monkeypatch.setattr(source_obligations, "source_versions", changed)
    with pytest.raises(ValueError, match="source history changed"):
        report(root)


@pytest.mark.parametrize("value", [None, {}, {"before_configuration": {}, "evidence": []}])
def test_invalid_fallback_is_not_treated_as_missing(task_project, value):
    path = task_project / "recovery/project.json"
    project = json.loads(path.read_text())
    project["source_history"] = value
    path.write_text(json.dumps(project))
    with pytest.raises(ValueError, match="historical source configuration"):
        report(task_project)


def test_historical_root_symlink_is_not_an_empty_source_tree(task_project):
    root = task_project
    (root / "code").rename(root / "saved_code")
    (root / "code").symlink_to("saved_code", target_is_directory=True)
    commit(root, "Unsupported historical root link")
    (root / "code").unlink()
    (root / "saved_code").rename(root / "code")
    commit(root, "Restore root")
    with pytest.raises(ValueError, match="root is not a directory"):
        report(root)
