"""Identity-based source obligations, without a count ledger or compiler."""

import json
from pathlib import Path
import subprocess

import pytest

from match_recovery.cli import main
from match_recovery.claim_integrity import check_claims
from match_recovery import claim_integrity


FIRST = "// ORIGINAL: 0x00000010 _first 0x00000010-0x00000018 BYTE_EXACT\nint first() {return 1;}\n"
SECOND = "// ORIGINAL: 0x00000020 _second 0x00000020-0x00000028\nint second() {return 2;}\n"


def git(root, *args):
    return subprocess.run(["git", *args], cwd=root, check=True, capture_output=True, text=True).stdout.strip()


def commit(root, message):
    git(root, "add", ".")
    git(root, "commit", "-m", message)
    return git(root, "rev-parse", "HEAD")


def configuration(root, change):
    path = root / "recovery/project.json"
    value = json.loads(path.read_text())
    change(value)
    path.write_text(json.dumps(value))


@pytest.fixture
def project(tmp_path):
    git(tmp_path, "init", "-b", "main")
    git(tmp_path, "config", "user.name", "Control")
    git(tmp_path, "config", "user.email", "control@example.test")
    for name in ("recovery", "engine", "seeds", "original"):
        (tmp_path / name).mkdir()
    (tmp_path / "original/PIN").write_text("sha256 " + "a" * 64 + "\n")
    (tmp_path / "engine/first.cc").write_text(FIRST)
    (tmp_path / "engine/second.cc").write_text(SECOND)
    (tmp_path / "seeds/keep.hpp").write_text("// seed evidence\n")
    (tmp_path / "recovery/project.json").write_text(json.dumps({
        "schema_version": 1,
        "source": {"image_pin": "original/PIN", "address_space": "custom:offset",
                   "roots": ["engine", "seeds"], "extensions": [".cc", ".hpp"], "seed_roots": ["seeds"]}}))
    base = commit(tmp_path, "Source baseline")
    configuration(tmp_path, lambda value: value.update(source_history={
        "baseline_revision": base, "evidence": ["Existing source obligations at recovery start"]}))
    commit(tmp_path, "Declare immutable source history baseline")
    return tmp_path


def snapshot(root):
    return {path.relative_to(root): (path.read_bytes(), path.stat().st_mtime_ns)
            for path in root.rglob("*") if path.is_file() and ".git" not in path.parts}


def test_native_check_is_readonly_and_needs_no_compiler(project, capsys):
    before = snapshot(project)
    assert main(["check-claims", "--root", str(project), "--summary"]) == 0
    result = json.loads(capsys.readouterr().out)
    assert result["historical_requirements"] == result["claims"] == 1
    assert result["missing_requirements"] == 0 and result["verification"] == "source_coverage_only"
    assert not result["persisted"] and not result["ready_for_collection"]
    assert snapshot(project) == before


@pytest.mark.parametrize("damage", ["remove-token", "rename-head", "delete-file", "remove-all", "weaken", "same-count-swap"])
def test_lost_identity_fails_even_when_count_is_unchanged(project, damage):
    first = project / "engine/first.cc"
    second = project / "engine/second.cc"
    if damage == "rename-head":
        first.write_text(FIRST.replace("ORIGINAL", "ORIGINAl"))
    elif damage == "delete-file":
        first.unlink()
    elif damage == "remove-all":
        first.unlink()
        second.unlink()
    else:
        first.write_text(FIRST.replace("BYTE_EXACT", "SEMANTIC" if damage == "weaken" else ""))
        if damage == "same-count-swap":
            second.write_text(SECOND.replace("0x00000020-0x00000028", "0x00000020-0x00000028 BYTE_EXACT"))
    result = check_claims(project)
    assert result["result"] == "fail"
    missing, = result["missing"]
    assert missing["address"] == "0x00000010" and missing["dimension"] == "bytes"
    if damage == "same-count-swap":
        assert result["claims"] == 1


def test_committing_a_lost_claim_does_not_make_it_disappear(project):
    (project / "engine/first.cc").write_text(FIRST.replace(" BYTE_EXACT", ""))
    commit(project, "Structural migration retains outstanding restoration")
    assert check_claims(project)["result"] == "fail"


def test_rehoming_and_renaming_keep_the_same_obligation(project):
    source = project / "engine/first.cc"
    source.rename(project / "seeds/old.cc")
    (project / "seeds/old.cc").write_text(FIRST.replace(" BYTE_EXACT", ""))
    (project / "engine/new.cc").write_text(FIRST.replace("_first", "_renamed"))
    result = check_claims(project)
    assert result["result"] == "pass" and result["claims"] == 1


def test_duplicate_seed_deletion_does_not_require_a_counter_repair(project):
    seed = project / "seeds/duplicate.cc"
    seed.write_text(FIRST)
    commit(project, "Preserve seed alternative")
    assert check_claims(project)["claims"] == 2
    seed.unlink()
    result = check_claims(project)
    assert result["result"] == "pass" and result["claims"] == 1 and result["historical_requirements"] == 1


def test_new_claims_are_measured_elsewhere_not_waived_or_count_reconciled(project):
    (project / "engine/second.cc").write_text(SECOND.replace("0x00000020-0x00000028", "0x00000020-0x00000028 BYTE_EXACT"))
    result = check_claims(project)
    assert result["result"] == "pass" and result["claims"] == 2 and result["historical_requirements"] == 1
    commit(project, "Bank measured second obligation")
    (project / "engine/second.cc").write_text(SECOND)
    assert len(check_claims(project)["missing"]) == 1


def test_exact_upgrade_preserves_historical_shape_requirement(project):
    first = project / "engine/first.cc"
    first.write_text(FIRST.replace("BYTE_EXACT", "SEMANTIC"))
    commit(project, "Historical shape claim")
    first.write_text(FIRST)
    result = check_claims(project)
    assert result["result"] == "pass" and result["historical_requirements"] == 2


@pytest.mark.parametrize("committed", [False, True])
def test_baseline_cannot_advance_to_hide_a_loss(project, committed):
    (project / "engine/first.cc").write_text(FIRST.replace(" BYTE_EXACT", ""))
    later = commit(project, "Remove old claim")
    configuration(project, lambda value: value["source_history"].update(baseline_revision=later))
    if committed:
        commit(project, "Attempt to reset obligations")
    with pytest.raises(ValueError, match="baseline changed"):
        check_claims(project)


def test_committed_baseline_change_then_restoration_cannot_hide_side_history(project):
    original = json.loads((project / "recovery/project.json").read_text())
    configuration(project, lambda value: value["source_history"].update(baseline_revision=git(project, "rev-parse", "HEAD")))
    commit(project, "Alter baseline")
    (project / "recovery/project.json").write_text(json.dumps(original))
    commit(project, "Restore current spelling")
    with pytest.raises(ValueError, match="baseline changed"):
        check_claims(project)


def test_merged_baseline_change_and_restoration_is_not_hidden(project):
    git(project, "switch", "-c", "side")
    original = (project / "recovery/project.json").read_bytes()
    configuration(project, lambda value: value["source_history"].update(baseline_revision=git(project, "rev-parse", "HEAD")))
    commit(project, "Side branch changes anchor")
    (project / "recovery/project.json").write_bytes(original)
    commit(project, "Side branch restores anchor")
    git(project, "switch", "main")
    git(project, "merge", "--no-ff", "side", "-m", "Merge side history")
    with pytest.raises(ValueError, match="baseline changed"):
        check_claims(project)


@pytest.mark.parametrize("committed", [False, True])
def test_preconfiguration_interpretation_cannot_be_changed_after_anchor(project, committed):
    configuration(project, lambda value: value["source_history"].update(before_configuration=value["source"]))
    if committed:
        commit(project, "Change historical interpretation")
    with pytest.raises(ValueError, match="pre-configuration source interpretation changed"):
        check_claims(project)


def test_shallow_history_refuses_even_when_declared_baseline_is_present(project, tmp_path_factory):
    clone = tmp_path_factory.mktemp("shallow") / "checkout"
    git(project, "clone", "--depth", "2", project.as_uri(), str(clone))
    assert git(clone, "rev-parse", "--is-shallow-repository") == "true"
    with pytest.raises(ValueError, match="complete Git history"):
        check_claims(clone)


def test_merged_claim_introduced_and_removed_on_side_branch_stays_required(project):
    git(project, "switch", "-c", "side")
    second = project / "engine/second.cc"
    second.write_text(SECOND.replace("0x00000020-0x00000028", "0x00000020-0x00000028 BYTE_EXACT"))
    commit(project, "Side branch claim")
    second.write_text(SECOND)
    commit(project, "Side branch removed claim")
    git(project, "switch", "main")
    git(project, "merge", "--no-ff", "side", "-m", "Integrate structural work")
    result = check_claims(project)
    assert result["missing"][0]["address"] == "0x00000020"


def test_source_and_pin_relocation_keep_history(project):
    (project / "engine").rename(project / "product")
    (project / "original/PIN").rename(project / "product/PIN")
    configuration(project, lambda value: value["source"].update(roots=["product", "seeds"], image_pin="product/PIN"))
    commit(project, "Move source inputs")
    assert check_claims(project)["result"] == "pass"


def test_source_change_during_history_scan_is_refused(project, monkeypatch):
    real = claim_integrity.source_versions
    def changing(*args, **kwargs):
        versions = real(*args, **kwargs)
        (project / "engine/first.cc").write_text(FIRST.replace(" BYTE_EXACT", ""))
        return versions
    monkeypatch.setattr(claim_integrity, "source_versions", changing)
    with pytest.raises(ValueError, match="source changed"):
        check_claims(project)


def test_missing_baseline_is_unknown_not_empty_success(project, capsys):
    configuration(project, lambda value: value["source_history"].pop("baseline_revision"))
    assert main(["check-claims", "--root", str(project)]) == 3
    assert capsys.readouterr().err
