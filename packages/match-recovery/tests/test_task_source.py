"""Configuration-only ownership tasks and source freshness damage controls."""

import json
from pathlib import Path
import subprocess
import sys

import pytest

from match_recovery.cli import main
from match_recovery.model import Origin
from match_recovery.task_source import task_context
from match_recovery.tasks import TaskStore
from test_source_configuration import configured


def git(root, *args):
    return subprocess.run(["git", "-c", "user.name=Test", "-c", "user.email=test@invalid.test",
                           "-c", "commit.gpgsign=false", "-c", "core.hooksPath=/dev/null", *args],
                          cwd=root, capture_output=True, check=True).stdout.decode().strip()


@pytest.fixture
def project(configured):
    git(configured, "init", "--initial-branch=work")
    git(configured, "add", ".")
    git(configured, "commit", "-m", "Synthetic baseline")
    return configured


def create(root, *extra):
    return main(["tasks", "create", "owner", "--root", str(root), "--group", "source:code/owner.cc",
                 "--author", "coordinator", "--intent", "Recover ownership", "--evidence", "source:code/owner.hpp",
                 *extra])


def test_native_task_lifecycle_has_no_project_code_or_registered_target(project, capsys):
    assert create(project, "--target", "custom-game") == 0
    task = json.loads(capsys.readouterr().out)["task"]
    assert task["target"] == "custom-game"
    assert task["base_revision"] == git(project, "rev-parse", "HEAD")
    assert task["editable_files"] == ["code/owner.cc", "code/owner.hpp"]
    subject = Origin("a" * 64, "custom:offset", 32)
    (project / "code/caller.cc").write_text('// ORIGINAL: 0x00000020 _caller 0x00000020-0x00000028\nvoid caller() {}\n')
    common = ["--root", str(project)]
    assert main(["tasks", "discover", "owner", *common, "--author", "worker", "--request-id", "finding",
                 "--hypothesis", "Caller shares ownership", "--affects", subject.entity_id,
                 "--evidence", "source:code/caller.cc", "--expected-revision", "0"]) == 0
    capsys.readouterr()
    assert main(["tasks", "expand", "owner", *common, "--author", "coordinator", "--request-id", "scope",
                 "--subject", subject.entity_id, "--edit-file", "code/caller.cc",
                 "--evidence", "source:code/caller.cc", "--expected-revision", "1"]) == 0
    capsys.readouterr()
    # Discovery decisions work even when there is no source provider configured.
    (project / "recovery/project.json").unlink()
    assert main(["tasks", "resolve", "owner", *common, "--author", "reviewer", "--request-id", "resolve",
                 "--discovery-id", "finding", "--disposition", "incorporated",
                 "--source-evidence", "source:code/caller.cc", "--expected-revision", "2"]) == 0
    capsys.readouterr()
    assert main(["tasks", "discover", "owner", *common, "--author", "worker", "--request-id", "next",
                 "--hypothesis", "Ownership needs review", "--affects", subject.entity_id,
                 "--evidence", "source:code/caller.cc", "--expected-revision", "3"]) == 0
    capsys.readouterr()
    assert len(TaskStore(project).decisions("owner")) == 4


def test_task_target_can_come_from_driver_configuration(project, capsys):
    (project / "recovery/driver.json").write_text(json.dumps({"schema_version": 1, "target": "configured-game"}))
    assert create(project) == 0
    assert json.loads(capsys.readouterr().out)["task"]["target"] == "configured-game"


def test_baseline_is_explicit_not_inferred_from_branch_names(project, capsys):
    base = git(project, "rev-parse", "HEAD")
    (project / "README.md").write_text("Tooling-only checkpoint\n")
    git(project, "add", "README.md")
    git(project, "commit", "-m", "Unrelated change")
    assert create(project, "--target", "custom-game", "--base", base) == 0
    assert json.loads(capsys.readouterr().out)["task"]["base_revision"] == base


@pytest.mark.parametrize("path", ["code/owner.hpp", "hints/attempt.cc", "original/identity", "recovery/project.json"])
def test_dirty_baseline_inputs_are_refused(project, capsys, path):
    selected = project / path
    selected.write_text(selected.read_text() + "\n")
    assert create(project, "--target", "custom-game") == 3
    assert "checkpoint source" in capsys.readouterr().err
    assert not (project / "recovery/tasks/owner.json").exists()


def test_git_assume_unchanged_cannot_hide_a_dirty_source_baseline(project, capsys):
    git(project, "update-index", "--assume-unchanged", "code/owner.hpp")
    (project / "code/owner.hpp").write_text("void changed();\n")
    assert create(project, "--target", "custom-game") == 3
    assert "checkpoint source bytes" in capsys.readouterr().err


def test_historical_baseline_with_different_source_is_refused(project, capsys):
    base = git(project, "rev-parse", "HEAD")
    (project / "code/owner.hpp").write_text("void owner(); // changed interpretation\n")
    git(project, "add", "code/owner.hpp")
    git(project, "commit", "-m", "Change source")
    assert create(project, "--target", "custom-game", "--base", base) == 3
    assert "differs" in capsys.readouterr().err


@pytest.mark.parametrize("base", ["HEAD", "--help", "b" * 40])
def test_invalid_or_unrelated_baselines_are_refused(project, capsys, base):
    assert create(project, "--target", "custom-game", "--base=" + base) == 3
    capsys.readouterr()
    assert not (project / "recovery/tasks/owner.json").exists()


def test_untracked_ignored_source_cannot_enter_a_committed_task_baseline(project, capsys):
    (project / ".gitignore").write_text("code/ignored.hpp\n")
    (project / "code/ignored.hpp").write_text("void hidden();\n")
    assert create(project, "--target", "custom-game") == 3
    assert "checkpoint source" in capsys.readouterr().err


def test_context_preserves_restoration_provider_and_refuses_racing_source(project, capsys):
    assert create(project, "--target", "custom-game") == 0
    capsys.readouterr()
    restoration = {"obligations": [{"synthetic": "retained"}], "verification": "not_measured"}
    output = task_context(project, "owner", restoration_for=lambda root, name: restoration)
    assert output["restoration"] is restoration
    assert output["impact"]["root_files"] == ["code/owner.cc", "code/owner.hpp"]
    assert not output["persisted"]
    def changing(root, name):
        (root / "code/owner.hpp").write_text("void raced();\n")
        return restoration
    with pytest.raises(ValueError, match="source changed"):
        task_context(project, "owner", restoration_for=changing)


def test_context_refuses_a_concurrent_scope_decision(project, capsys):
    assert create(project, "--target", "custom-game") == 0
    capsys.readouterr()
    def changing(root, name):
        TaskStore(root).expand(name, request_id="scope", author="coordinator", subjects=(),
                               editable_files=("code/new.cc",), evidence=("synthetic",), expected_revision=0)
        return {"verification": "not_measured"}
    with pytest.raises(ValueError, match="task decisions changed"):
        task_context(project, "owner", restoration_for=changing)


def test_context_refuses_another_original_even_with_a_custom_provider(project, capsys):
    assert create(project, "--target", "custom-game") == 0
    capsys.readouterr()
    (project / "original/identity").write_text("sha256 " + "b" * 64 + "\n")
    with pytest.raises(ValueError, match="task original"):
        task_context(project, "owner", restoration_for=lambda root, name: {})


def test_native_task_creation_runs_without_site_packages(project):
    import match_recovery
    package_root = Path(match_recovery.__file__).resolve().parent.parent
    script = ('import sys; sys.path.insert(0, sys.argv[1]); '
              'from match_recovery.cli import main; raise SystemExit(main(sys.argv[2:]))')
    result = subprocess.run([sys.executable, "-I", "-S", "-c", script, str(package_root),
                             "tasks", "create", "owner", "--root", str(project),
                             "--group", "source:code/owner.cc", "--target", "isolated-game",
                             "--author", "coordinator", "--intent", "Recover ownership", "--evidence", "synthetic"],
                            cwd=project, capture_output=True, text=True, timeout=30)
    assert result.returncode == 0, result.stderr
    assert json.loads(result.stdout)["task"]["target"] == "isolated-game"
