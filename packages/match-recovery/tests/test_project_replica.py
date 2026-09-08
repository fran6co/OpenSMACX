"""Private generation context controls; source and original Git stay untouched."""

import os

import pytest

from match_recovery.driver import working_state
from match_recovery.project_replica import project_replica
from match_recovery.source_replica import replica_snapshot
from test_preservation import project
from test_task_source import git


def files(root):
    return {name: content for name, content in replica_snapshot(root, ("code",)).items() if content is not None}


def test_replica_has_current_policy_own_git_and_complete_historical_source(project):
    source = files(project)
    source.pop("code/model.h")
    source["code/old.h"] = b"historical header\n"
    before = working_state(project)
    (project / "build").mkdir()
    (project / "build/private.exe").write_bytes(b"not copied")
    with project_replica(project, ("code",), source) as replica:
        assert git(replica, "rev-parse", "HEAD") == before["head"].decode().strip()
        assert (replica / ".git/index").is_file()
        assert not (replica / "code/model.h").exists()
        assert (replica / "code/old.h").read_bytes() == source["code/old.h"]
        assert not (replica / "build/private.exe").exists()
        assert (replica / "recovery/project.json").read_bytes() == (project / "recovery/project.json").read_bytes()
        assert git(replica, "show", "HEAD:code/model.h") == "struct Model { int field; };"
        git(replica, "add", "code")
        git(replica, "-c", "user.name=Fixture", "-c", "user.email=fixture@example.invalid",
            "commit", "-m", "Private generation may inspect independent Git state")
        assert working_state(project) == before
    assert not replica.exists() and working_state(project) == before


def test_configured_checkout_filters_and_hooks_do_not_participate(project):
    (project / ".gitattributes").write_text("*.cc filter=damage\n")
    git(project, "add", ".gitattributes")
    git(project, "commit", "-m", "Declared external filter")
    git(project, "config", "filter.damage.smudge", "false")
    git(project, "config", "filter.damage.clean", "false")
    git(project, "config", "filter.damage.required", "true")
    hook = project / ".git/hooks/reference-transaction"
    hook.write_text("#!/bin/sh\nexit 1\n")
    hook.chmod(0o755)
    before = working_state(project)
    with project_replica(project, ("code",), files(project)) as replica:
        assert (replica / "code/owner.cc").read_bytes() == (project / "code/owner.cc").read_bytes()
        assert not (replica / ".git/hooks/reference-transaction").exists()
    assert working_state(project) == before


@pytest.mark.parametrize("kind", ["internal", "outside", "directory"])
def test_explicit_internal_file_aliases_only(project, kind):
    link = project / "recovery/alias"
    link.symlink_to("project.json" if kind == "internal" else "/etc/passwd" if kind == "outside" else "../code")
    before = working_state(project)
    if kind != "internal":
        with pytest.raises(ValueError, match="copied internal files"):
            with project_replica(project, ("code",), files(project)):
                pytest.fail("unsupported alias copied")
    else:
        with project_replica(project, ("code",), files(project)) as replica:
            assert os.readlink(replica / "recovery/alias") == "project.json"
            assert (replica / "recovery/alias").resolve() == replica / "recovery/project.json"
    assert working_state(project) == before


def test_original_source_changes_are_detected_and_preserved(project):
    with pytest.raises(ValueError, match="inputs or Git state changed"):
        with project_replica(project, ("code",), files(project)):
            (project / "code/model.h").write_text("concurrent user edit\n")
    assert (project / "code/model.h").read_text() == "concurrent user edit\n"


def test_declared_ignored_inputs_are_copied_but_not_other_caches(project):
    (project / "build").mkdir()
    (project / "build/sdk.h").write_bytes(b"explicit input\n")
    with project_replica(project, ("code",), files(project), inputs=("build/sdk.h",)) as replica:
        assert (replica / "build/sdk.h").read_bytes() == b"explicit input\n"


@pytest.mark.parametrize("variable", ["GIT_INDEX_FILE", "GIT_OBJECT_DIRECTORY", "GIT_ALTERNATE_OBJECT_DIRECTORIES", "GIT_NAMESPACE"])
def test_git_path_override_is_refused(project, monkeypatch, variable):
    monkeypatch.setenv(variable, str(project / ".git/index"))
    with pytest.raises(ValueError, match="Git path overrides"):
        with project_replica(project, ("code",), files(project)):
            pytest.fail("shared index override accepted")
