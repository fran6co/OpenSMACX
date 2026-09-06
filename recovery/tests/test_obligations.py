import subprocess
import json

import pytest

from match_recovery.source_obligations import task_obligations
from match_recovery.model import Origin
from match_recovery.source_history import source_versions
from match_recovery.tasks import Task, TaskStore


PIN = "a" * 64
ONE = Origin(PIN, "pe:va", 0x401000)
TWO = Origin(PIN, "pe:va", 0x402000)


def body(address=0x401000, claim="BYTE_EXACT"):
    return (f"// ORIGINAL: 0x{address:08X} ?f@@YAXXZ 0x{address:08X}-0x{address + 8:08X} {claim}\n"
            "void f() {}\n")


def git(root, *args):
    return subprocess.run(["git", *args], cwd=root, capture_output=True, check=True).stdout.decode().strip()


def commit(root, message):
    git(root, "add", ".")
    git(root, "commit", "-m", message)
    return git(root, "rev-parse", "HEAD")


@pytest.fixture
def project(tmp_path, source_configuration):
    git(tmp_path, "init", "-b", "main")
    git(tmp_path, "config", "user.name", "Synthetic")
    git(tmp_path, "config", "user.email", "synthetic@example.invalid")
    (tmp_path / "src").mkdir()
    (tmp_path / "src/IMAGE_PIN").write_text("sha256 " + PIN + "\n")
    (tmp_path / "src/f.cpp").write_text(body())
    base = commit(tmp_path, "synthetic baseline")
    task = Task("fixture", "opensmacx", "coordinator", "Recover structure", base,
                (ONE,), ("src/f.cpp",), ("synthetic source evidence",))
    TaskStore(tmp_path).create(task)
    commit(tmp_path, "task decision")
    return tmp_path, task


def report(project):
    root, task = project
    return task_obligations(root, task.task_id)


def test_removed_claim_survives_rehome_and_resume_without_a_ledger(project):
    root, task = project
    before = report(project)
    (root / "src/f.cpp").unlink()
    (root / "src/new.cpp").write_text(body(claim=""))
    commit(root, "structural checkpoint, matching not restored")
    status_before = git(root, "status", "--porcelain")
    after = report(project)
    assert len(after["obligations"]) == 1
    assert after["obligations"][0]["obligation_id"] == before["obligations"][0]["obligation_id"]
    assert after["obligations"][0]["source_evidence"][0]["revision"] == task.base_revision
    assert after["verification"] == "not_measured"
    assert after == report(project)
    assert git(root, "status", "--porcelain") == status_before == ""


def test_new_claim_committed_then_removed_is_still_required(project):
    root, task = project
    store = TaskStore(root)
    store.expand(task.task_id, request_id="expand", author=task.author, subjects=(TWO,),
                 editable_files=("src/g.cpp",), evidence=("synthetic member relationship",), expected_revision=0)
    (root / "src/g.cpp").write_text(body(0x402000))
    introduced = commit(root, "new required match")
    (root / "src/g.cpp").unlink()
    commit(root, "later structural checkpoint")
    rows = {row["entity_id"]: row for row in report(project)["obligations"]}
    assert set(rows) == {ONE.entity_id, TWO.entity_id}
    assert any(item["revision"] == introduced for item in rows[TWO.entity_id]["source_evidence"])


def test_merged_side_history_cannot_hide_removed_claims(project):
    root, task = project
    git(root, "switch", "-c", "side")
    (root / "src/f.cpp").write_text(body(claim="BYTE_EXACT SEMANTIC"))
    introduced = commit(root, "side matching evidence")
    (root / "src/f.cpp").write_text(body())
    commit(root, "side restores previous spelling")
    git(root, "switch", "main")
    git(root, "merge", "--no-ff", "side", "-m", "merge side checkpoint history")
    rows = report(project)["obligations"]
    assert {row["dimension"] for row in rows} == {"bytes", "instruction_shape"}
    semantic = next(row for row in rows if row["dimension"] == "instruction_shape")
    assert any(item["revision"] == introduced for item in semantic["source_evidence"])


def test_seeds_and_duplicate_claim_sites_do_not_duplicate_requirements(project):
    root, _ = project
    (root / "src/unrecovered").mkdir()
    (root / "src/unrecovered/seed.cpp").write_text(body())
    (root / "src/unrelated.cpp").write_text(body(0x402000))
    assert len(report(project)["obligations"]) == 1
    assert any("unrecovered" in item["path"] for item in report(project)["obligations"][0]["source_evidence"])


@pytest.mark.parametrize("change", ["replace", "remove"])
def test_pin_changed_and_restored_in_history_is_refused(project, change):
    root, _ = project
    pin = root / "src/IMAGE_PIN"
    if change == "replace":
        pin.write_text("sha256 " + "b" * 64 + "\n")
    else:
        pin.unlink()
    commit(root, "wrong original")
    pin.write_text("sha256 " + PIN + "\n")
    commit(root, "restore pin")
    with pytest.raises(ValueError, match="pin|lacks required"):
        report(project)


def test_unreachable_baseline_is_not_silently_replaced(project):
    root, task = project
    with pytest.raises(ValueError, match="source history"):
        source_versions(root, "b" * 40, git(root, "rev-parse", "HEAD"))
    git(root, "switch", "--orphan", "unrelated")
    (root / "src").mkdir(exist_ok=True)
    (root / "src/f.cpp").write_text(body())
    unrelated = commit(root, "unrelated root")
    with pytest.raises(ValueError, match="source history"):
        source_versions(root, task.base_revision, unrelated)


def test_current_claim_changes_fingerprint_without_claiming_verification(project):
    root, _ = project
    before = report(project)
    (root / "src/f.cpp").write_text(body(claim="BYTE_EXACT SEMANTIC"))
    after = report(project)
    assert before["input_fingerprint"] != after["input_fingerprint"]
    assert len(after["obligations"]) == 2
    assert after["verification"] == "not_measured"


def test_cli_context_exposes_same_requirements(project, capsys):
    from match_recovery.cli import main
    root, task = project
    assert main(["tasks", "obligations", task.task_id, "--root", str(root)]) == 0
    standalone = json.loads(capsys.readouterr().out)
    assert main(["tasks", "context", task.task_id, "--root", str(root)]) == 0
    context = json.loads(capsys.readouterr().out)
    assert context["restoration"] == standalone


def test_concurrent_working_source_change_is_refused(project, monkeypatch):
    import match_recovery.source_obligations as adapter
    root, _ = project
    original = adapter.source_snapshot
    def racing_snapshot(*args):
        snapshot = original(*args)
        (root / "src/f.cpp").write_text(body(claim=""))
        return snapshot
    monkeypatch.setattr(adapter, "source_snapshot", racing_snapshot)
    with pytest.raises(ValueError, match="source changed"):
        report(project)


MEMBER = "// RECOVERY_MEMBER: v1 Widget slot_ 0x4 0x00401000\n"


def structural(project):
    return [row for row in report(project)["obligations"] if row["dimension"] == "structure"]


@pytest.mark.parametrize("annotation", [MEMBER, "// RECOVERY_BASE: v1 Widget Base 0x4 0x00401000\n",
                                        "// RECOVERY_MEMBER: v2 Widget slot_ Base 0x4 0x00401000\n"])
def test_layout_review_obligation_survives_rehome_and_deletion(project, annotation):
    root, task = project
    path = root / "src/widget.h"
    path.write_text(annotation)
    introduced = commit(root, "source-owned layout interpretation")
    first = structural(project)[0]
    path.rename(root / "src/renamed.h")
    commit(root, "rehome declaration")
    (root / "src/renamed.h").unlink()
    commit(root, "remove interpretation during structural work")
    after = structural(project)
    assert len(after) == 1 and after[0]["obligation_id"] == first["obligation_id"]
    evidence = after[0]["source_evidence"]
    assert any(item["revision"] == introduced and item["path"] == "src/widget.h" for item in evidence)
    assert any(item["path"] == "src/renamed.h" for item in evidence)
    assert evidence[0]["interpretation"]["offset"] == 4
    assert git(root, "status", "--porcelain") == ""


def test_revised_layout_preserves_interpretations_without_freezing_old_spelling(project):
    root, _ = project
    path = root / "src/widget.h"
    path.write_text(MEMBER)
    commit(root, "initial interpretation")
    first = structural(project)[0]
    path.write_text(MEMBER.replace("Widget slot_ 0x4", "Owner member_ 0x8"))
    after = structural(project)
    # A single structural review obligation per original evidence anchor, not
    # two immutable and potentially contradictory member-layout claims.
    assert len(after) == 1 and after[0]["obligation_id"] == first["obligation_id"]
    interpretations = [item["interpretation"] for item in after[0]["source_evidence"]]
    assert {(item["class_name"], item["member"], item["offset"]) for item in interpretations} == {
        ("Widget", "slot_", 4), ("Owner", "member_", 8)}
    assert report(project)["verification"] == "not_measured"

    indexed = report(project)["interpretations"]
    assert len(indexed) == 2
    assert {(item["value"]["offset"], item["current"]) for item in indexed} == {(4, False), (8, True)}


def test_layout_hints_in_seeds_and_outside_scope_do_not_become_scoped_obligations(project):
    root, _ = project
    (root / "src/unrecovered").mkdir()
    (root / "src/unrecovered/seed.cpp").write_text(MEMBER)
    (root / "src/unrelated.h").write_text(MEMBER.replace("00401000", "00402000"))
    assert structural(project) == []


def test_malformed_historical_layout_cannot_disappear_from_review(project):
    root, _ = project
    path = root / "src/widget.h"
    path.write_text("// RECOVERY_MEMBER: malformed\n")
    commit(root, "invalid historical interpretation")
    path.unlink()
    commit(root, "remove malformed source")
    with pytest.raises(ValueError, match="invalid original member expectation"):
        report(project)


def test_structural_review_requirements_enter_driver_protected_inventory(project):
    from match_recovery.migration_protocol import extend_inventory

    root, task = project
    (root / "src/widget.h").write_text(MEMBER)
    inventory = report(project)
    protected = {}
    extend_inventory(protected, inventory, frozenset({ONE.entity_id}))
    assert {item.dimension.value for item in protected.values()} == {"bytes", "structure"}
    (root / "src/widget.h").unlink()
    extend_inventory(protected, report(project), frozenset({ONE.entity_id}))
    assert {item.dimension.value for item in protected.values()} == {"bytes", "structure"}


def test_strengthening_offset_with_type_preserves_old_interpretation_for_review(project):
    root, task = project
    path = root / "src/widget.h"
    path.write_text(MEMBER)
    commit(root, "offset interpretation")
    first = structural(project)[0]["obligation_id"]
    path.write_text("// RECOVERY_MEMBER: v2 Widget slot_ Base 0x4 0x00401000\n")
    after = report(project)
    assert structural(project)[0]["obligation_id"] == first
    assert len(after["interpretations"]) == 2
    assert any(item["current"] and item["value"]["class_type"] == "Base" for item in after["interpretations"])
    assert any(not item["current"] and "class_type" not in item["value"] for item in after["interpretations"])


GENERATED = "// RECOVERY_GENERATED: v1 vtable 0x00601000 ? ??_7Widget@@6B@ 0x00401000\n"


def test_generated_identity_survives_rehome_refinement_and_removal(project):
    root, _ = project
    path = root / "src/widget.h"
    path.write_text(GENERATED)
    commit(root, "original table identity with unknown extent")
    first = report(project)
    obligation = structural(project)[0]
    assert obligation["entity_id"] == ONE.entity_id  # evidence, not table address
    initial, = first["interpretations"]
    assert initial["value"]["span"] is None and initial["current"]
    path.rename(root / "src/renamed.h")
    path = root / "src/renamed.h"
    assert report(project)["interpretations"] == first["interpretations"]
    path.write_text(GENERATED.replace(" ? ", " 0x00601008 ").replace("Widget", "Owner"))
    commit(root, "refine table extent and owning class")
    rows = report(project)["interpretations"]
    assert len(rows) == 2
    assert any(not row["current"] and row["interpretation_id"] == initial["interpretation_id"] for row in rows)
    assert any(row["current"] and row["value"]["span"] == {"start": 0x601000, "end": 0x601008}
               and row["value"]["symbol"] == "??_7Owner@@6B@" for row in rows)
    path.unlink()
    commit(root, "remove table mapping during hierarchy rewrite")
    after = report(project)
    assert len(after["interpretations"]) == 2 and not any(row["current"] for row in after["interpretations"])
    assert structural(project)[0]["obligation_id"] == obligation["obligation_id"]
    assert after["verification"] == "not_measured"


def test_generated_seed_hints_and_other_evidence_are_not_scoped_obligations(project):
    root, _ = project
    (root / "src/unrecovered").mkdir()
    (root / "src/unrecovered/seed.cpp").write_text(GENERATED)
    (root / "src/unrelated.h").write_text(GENERATED.replace("00401000", "00402000"))
    commit(root, "seed and unrelated table evidence")
    assert structural(project) == []


def test_malformed_generated_history_cannot_be_hidden_by_deletion(project):
    root, _ = project
    path = root / "src/widget.h"
    path.write_text("// RECOVERY_GENERATED: invalid\n")
    commit(root, "malformed table interpretation")
    path.unlink()
    commit(root, "remove malformed table")
    with pytest.raises(ValueError, match="invalid generated binding"):
        report(project)
