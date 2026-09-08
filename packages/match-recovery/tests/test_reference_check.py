"""Instruction lint works on an installed package with project-owned paths."""

import json
import pytest

from match_recovery.cli import main
from match_recovery.reference_check import check_references, reference_snapshot


@pytest.fixture
def project(tmp_path):
    for name in ("engine", "notes", "scripts", "recovery"):
        (tmp_path / name).mkdir()
    (tmp_path / "engine/a.cc").write_text("int main() {}\n")
    (tmp_path / "recovery/project.json").write_text(json.dumps({
        "schema_version": 1,
        "source": {"image_pin": "engine/PIN", "address_space": "test:va", "roots": ["engine"],
                   "seed_roots": ["engine/seeds"], "include_paths": ["engine"]},
        "references": {"documentation_roots": ["notes", "recovery"],
                       "command_roots": ["scripts", "recovery"], "exclude_roots": ["notes/worktrees"]}}))
    return tmp_path


def policy(root, update):
    path = root / "recovery/project.json"
    config = json.loads(path.read_text())
    update(config["references"])
    path.write_text(json.dumps(config))


def test_live_source_and_nested_script_paths_across_instruction_roots(project):
    (project / "README.md").write_text("Run scripts/nested/Check-Names.py\n")
    (project / "recovery/WORKFLOW.md").write_text("Run recovery/missing.sh\n")
    (project / "engine/a.cc").write_text("// implementation in engine/missing.hh\n")
    report = check_references(project)
    assert report["result"] == "fail" and report["missing_paths"] == 3
    assert {r["target"] for r in report["findings"]} == {
        "scripts/nested/Check-Names.py", "recovery/missing.sh", "engine/missing.hh"}
    assert all(row["line"] == 1 for row in report["findings"])
    assert not report["persisted"] and not report["ready_for_collection"]


def test_existing_target_then_deletion_then_restoration_changes_snapshot(project):
    doc = project / "notes/start.md"
    doc.write_text("Run scripts/check.py\n")
    script = project / "scripts/check.py"
    script.write_text("pass\n")
    before = reference_snapshot(project)
    assert check_references(project)["result"] == "pass"
    script.unlink()
    assert reference_snapshot(project) != before
    assert check_references(project)["result"] == "fail"
    script.write_text("pass\n")
    assert reference_snapshot(project) == before


def test_history_remains_visible_but_distant_retraction_cannot_excuse_instruction(project):
    (project / "notes/old.md").write_text(
        "scripts/generator.py was deleted; do not regenerate.\n"
        "// homed from engine/seeds/gone.cc\n" + "\n" * 15 + "Run scripts/dead.py\n")
    report = check_references(project)
    assert [r["target"] for r in report["findings"]] == ["scripts/dead.py"]
    assert {r["target"] for r in report["history"]} == {"scripts/generator.py", "engine/seeds/gone.cc"}


def test_bare_script_mention_is_not_an_instruction_but_source_claim_is_live(project):
    (project / "notes/start.md").write_text("scripts/previous.py\nengine/current.cc\n")
    report = check_references(project)
    assert [r["target"] for r in report["findings"]] == ["engine/current.cc"]
    assert [r["target"] for r in report["history"]] == ["scripts/previous.py"]


def test_exclusions_are_whole_components_not_string_prefixes(project):
    for root in ("engine/seeds", "notes/worktrees", "engine/seeds-live"):
        (project / root).mkdir()
        (project / root / "old.md").write_text("Run scripts/absent.py\n")
    report = check_references(project)
    assert [r["path"] for r in report["findings"]] == ["engine/seeds-live/old.md"]


def test_noninput_compiler_launcher_alias_does_not_expand_prose_scope(project):
    (project / "recovery/launcher").symlink_to(project / "engine/a.cc")
    assert check_references(project)["result"] == "pass"


@pytest.mark.parametrize("name,target", [("notes/alias.md", "engine/a.cc"), ("notes/alias", "engine")])
def test_scan_aliases_cannot_hide_prose(project, name, target):
    (project / name).symlink_to(project / target)
    with pytest.raises(ValueError, match="symlinks"):
        check_references(project)


def test_aliased_target_is_not_accepted_as_canonical_source(project):
    (project / "scripts/check.py").symlink_to(project / "engine/a.cc")
    (project / "notes/start.md").write_text("Run scripts/check.py\n")
    assert check_references(project)["result"] == "fail"


def test_scan_cannot_follow_an_external_documentation_root(project, tmp_path_factory):
    external = tmp_path_factory.mktemp("outside")
    (project / "outside").symlink_to(external, target_is_directory=True)
    policy(project, lambda p: p.update(documentation_roots=["outside"]))
    with pytest.raises(ValueError, match="outside"):
        check_references(project)


@pytest.mark.parametrize("value", [None, ["notes", "notes"], ["../outside"], "notes"])
def test_invalid_scope_is_refused(project, value):
    policy(project, lambda p: p.update(documentation_roots=value))
    with pytest.raises(ValueError):
        check_references(project)


def test_missing_scan_root_is_unknown_not_empty_success(project, capsys):
    policy(project, lambda p: p.update(documentation_roots=["missing"]))
    assert main(["check-references", "--root", str(project)]) == 3
    assert "missing reference scan root" in capsys.readouterr().err


def test_changed_target_during_check_refuses_stale_success(project, monkeypatch):
    import match_recovery.reference_check as module
    target = project / "scripts/check.py"
    target.write_text("pass\n")
    (project / "notes/start.md").write_text("Run scripts/check.py\n")
    actual = module.reference_snapshot
    calls = 0
    def snapshot(root):
        nonlocal calls
        calls += 1
        if calls == 2:
            target.unlink()
        return actual(root)
    monkeypatch.setattr(module, "reference_snapshot", snapshot)
    with pytest.raises(ValueError, match="changed"):
        check_references(project)


def test_installed_cli_keeps_failure_locations_and_summarizes_history(project, capsys):
    (project / "notes/start.md").write_text("Run scripts/check.py\n")
    assert main(["check-references", "--root", str(project), "--summary"]) == 1
    report = json.loads(capsys.readouterr().out)
    assert report["findings"][0]["target"] == "scripts/check.py"
    assert "history" not in report and report["historical_mentions"] == 0
