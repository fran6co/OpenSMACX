"""Task-preserving worker setup and fresh recipe/resource failure controls."""

from dataclasses import replace
import json
from pathlib import Path
import subprocess
import sys

import pytest

from match_recovery.cli import main
from match_recovery.driver import working_state
from match_recovery.locking import file_lock
from match_recovery.processes import run_bounded
from match_recovery.resources import ACTIVE, LOCK
from match_recovery.tasks import TaskStore
from match_recovery.worker_worktree import prepare_worker
from test_preservation import project
from test_processes import _assert_stopped
from test_task_source import git


def permission(root, **changes):
    return {"author": "coordinator", "revision": git(root, "rev-parse", "HEAD"),
            "evidence": ["Authorize an isolated worker for the existing source task"], "prepare": False, **changes}


def prepare(root, **options):
    return prepare_worker(root, "owner", options.pop("authorization", permission(root)),
                          worktree=options.pop("worktree", "build/worker"),
                          branch=options.pop("branch", "worker"), **options)


def test_raw_worker_keeps_the_original_task_baseline_and_does_not_copy_caches(project):
    root = project
    (root / "build").mkdir()
    (root / "build/recipes.json").write_text("must not copy coordinator cache")
    before = working_state(root)
    report = prepare(root)
    child = Path(report["worktree"])
    assert report["worker_created"] and report["stop_reason"] == "checkout_prepared"
    assert report["preparation"] is None and not report["compiler_prepared"]
    assert report["verification"] == "not_measured" and not report["ready_for_collection"]
    assert git(child, "rev-parse", "HEAD") == git(root, "rev-parse", "HEAD")
    assert TaskStore(child).read("owner") == TaskStore(root).read("owner")
    assert report["task_base_revision"] != report["revision"]
    assert not (child / "build/recipes.json").exists()
    assert working_state(root) == before
    (child / "code/owner.cc").write_text("worker-owned draft")
    assert working_state(root) == before  # No hardlinks to coordinator files.


@pytest.mark.parametrize("damage", ["dirty", "staged", "untracked", "existing_path", "existing_branch", "unignored",
                                    "metadata", "source_root", "stale", "author", "pin", "subdirectory"])
def test_worker_refusals_never_erase_source_or_repurpose_branches(project, damage):
    root = project
    auth = permission(root)
    options = {}
    if damage in ("dirty", "staged"):
        (root / "code/owner.cc").write_text("user work")
        if damage == "staged":
            git(root, "add", "code/owner.cc")
    elif damage == "untracked":
        (root / "user-note").write_text("preserve")
    elif damage == "existing_path":
        (root / "build/worker").mkdir(parents=True)
        (root / "build/worker/user-note").write_text("preserve")
    elif damage == "existing_branch":
        git(root, "branch", "worker")
    elif damage == "unignored":
        options["worktree"] = "not-ignored"
    elif damage == "metadata":
        options["worktree"] = ".git/worker"
    elif damage == "source_root":
        (root / ".gitignore").write_text("build/\ncode/worker/\n")
        git(root, "add", ".gitignore")
        git(root, "commit", "-m", "Ignored path inside source is not a worker home")
        auth = permission(root)
        options["worktree"] = "code/worker"
    elif damage == "stale":
        git(root, "commit", "--allow-empty", "-m", "Coordinator advanced")
    elif damage == "author":
        auth["author"] = "worker"
    elif damage == "pin":
        (root / "original/PIN").write_text("sha256 " + "b" * 64 + "\n")
        git(root, "add", "original/PIN")
        git(root, "commit", "-m", "Different original")
        auth = permission(root)
    elif damage == "subdirectory":
        options["root"] = root / "code"
    before, branches = working_state(root), git(root, "show-ref")
    with pytest.raises(ValueError):
        prepare(options.pop("root", root), authorization=auth, **options)
    assert working_state(root) == before and git(root, "show-ref") == branches


@pytest.mark.parametrize("change", [{"prepare": 1}, {"evidence": []}, {"revision": "HEAD"}, {"unknown": True}])
def test_invalid_worker_authorization_refuses_before_creation(project, change):
    with pytest.raises(ValueError):
        prepare(project, authorization=permission(project, **change))
    assert not (project / "build/worker").exists()


def test_worker_setup_preserves_discoveries_and_expansion_decisions(project):
    store = TaskStore(project)
    task = store.read("owner")
    subject = replace(task.subjects[0], address=0x401010)
    store.discover("owner", request_id="caller", author="worker", hypothesis="Caller shares ownership",
                   affects=[subject.entity_id], evidence=["source:code/caller.cc"], expected_revision=0)
    store.expand("owner", request_id="scope", author=task.author, subjects=[subject],
                 editable_files=["code/caller.cc"], evidence=["Authorize caller recovery"], expected_revision=1)
    git(project, "add", "recovery/history")
    git(project, "commit", "-m", "Carry scope and unresolved discovery")
    child = Path(prepare(project)["worktree"])
    assert TaskStore(child).scope("owner") == store.scope("owner")
    assert TaskStore(child).decisions("owner") == store.decisions("owner")


def test_internal_compiler_symlinks_and_raw_bytes_are_preserved_without_hooks(project):
    (project / "recovery/compiler.py").write_bytes(b"# compiler wrapper\n")
    (project / "recovery/compiler.py").chmod(0o755)
    (project / "recovery/compiler-link").symlink_to("compiler.py")
    (project / ".gitattributes").write_text("*.csv text=auto\n")
    (project / "notes.csv").write_bytes(b"field,offset\nmember,4\n")
    git(project, "add", ".")
    git(project, "commit", "-m", "Committed wrapper links and normalized evidence")
    (project / "notes.csv").write_bytes(b"field,offset\r\nmember,4\r\n")
    hook = project / ".git/hooks/post-checkout"
    hook.write_text("#!/bin/sh\nexit 87\n")
    hook.chmod(0o755)
    before = working_state(project)
    child = Path(prepare(project)["worktree"])
    assert (child / "recovery/compiler-link").is_symlink()
    assert (child / "recovery/compiler-link").read_bytes() == b"# compiler wrapper\n"
    assert (child / "recovery/compiler.py").stat().st_mode & 0o111
    assert (child / "notes.csv").read_bytes() == b"field,offset\nmember,4\n"
    assert working_state(project) == before


def test_worker_refuses_filters_without_running_them(project):
    (project / ".gitattributes").write_text("code/owner.cc filter=untrusted\n")
    git(project, "add", ".gitattributes")
    git(project, "commit", "-m", "Active filter must be resolved explicitly")
    git(project, "config", "filter.untrusted.process", "false")
    git(project, "config", "filter.untrusted.required", "true")
    with pytest.raises(ValueError, match="active checkout filters"):
        prepare(project)
    assert not (project / "build/worker").exists()


@pytest.fixture
def compiler_project(project):
    root = project
    config = json.loads((root / "recovery/project.json").read_text())
    config.update(preparation="recovery/prepare.json", mappings={"compile_commands": "build/recipes.json"},
                  compiler={"environment": ["TEST_COMPILER_SLOT"]})
    (root / "recovery/project.json").write_text(json.dumps(config))
    code = ("from pathlib import Path; import json,os,sys; root=Path(sys.argv[1]); out=Path(sys.argv[2]); "
            "assert not list(out.iterdir()); assert Path(os.environ['TEST_COMPILER_SLOT']).is_dir(); "
            "(out/'commands.json').write_text(json.dumps([dict(directory=str(out),file=str(root/'code/owner.cc'),"
            "arguments=['compiler','/c',str(root/'code/owner.cc')])]))")
    settings = {"schema_version": 1, "artifacts": "build/preparation", "inputs": [], "input_roots": ["code"],
                "environment": ["TEST_COMPILER_SLOT"], "outputs": [{"path": "commands.json", "format": "compilation-database"}],
                "commands": [{"argv": [sys.executable, "-c", code, "{root}", "{build}"], "timeout": 5}]}
    (root / "recovery/prepare.json").write_text(json.dumps(settings))
    slot = root / "build/compiler-slot"
    slot.mkdir(parents=True)
    resources = {"schema_version": 1, "slots": [{"name": "compiler", "directory": str(slot),
                                               "environment": {"TEST_COMPILER_SLOT": "{resource}"}}]}
    (root / "recovery/resources.json").write_text(json.dumps(resources))
    git(root, "add", ".")
    git(root, "commit", "-m", "Synthetic recipe generator and compiler slot")
    return root, resources


def test_worker_prepares_fresh_native_recipes_under_a_lease(compiler_project):
    root, resources = compiler_project
    before = working_state(root)
    report = prepare(root, authorization=permission(root, prepare=True), resources=resources)
    child = Path(report["worktree"])
    assert report["compiler_prepared"] and report["stop_reason"] == "worker_prepared"
    row, = json.loads((child / "build/recipes.json").read_text())
    assert Path(row["file"]) == child / "code/owner.cc"
    assert Path(row["directory"]).is_relative_to(child)
    assert not (root / "build/recipes.json").exists()
    assert working_state(root) == before and not report["ready_for_collection"]
    slot = Path(resources["slots"][0]["directory"])
    assert not (slot / ACTIVE).exists()
    with file_lock(slot / LOCK, timeout=0):
        pass


def test_busy_compiler_refuses_before_creating_worker(compiler_project):
    root, resources = compiler_project
    slot = Path(resources["slots"][0]["directory"])
    with file_lock(slot / LOCK):
        with pytest.raises(TimeoutError):
            prepare(root, authorization=permission(root, prepare=True), resources=resources)
    assert not (root / "build/worker").exists() and not git(root, "branch", "--list", "worker")


def test_failed_generator_retains_the_worker_without_publishing_recipes(compiler_project):
    root, resources = compiler_project
    path = root / "recovery/prepare.json"
    settings = json.loads(path.read_text())
    settings["commands"] = [{"argv": [sys.executable, "-c", "raise SystemExit(1)", "{build}"], "timeout": 5}]
    path.write_text(json.dumps(settings))
    git(root, "add", "recovery/prepare.json")
    git(root, "commit", "-m", "Broken generator control")
    report = prepare(root, authorization=permission(root, prepare=True), resources=resources)
    assert report["worker_created"] and report["stop_reason"] == "compiler_preparation_failed"
    assert not report["compiler_prepared"] and not report["ready_for_collection"]
    assert not (Path(report["worktree"]) / "build/recipes.json").exists()
    assert git(root, "branch", "--list", "worker")


def test_installed_cli_prepares_with_the_same_resource_options(compiler_project, capsys):
    root, _ = compiler_project
    path = root / "build/authorization.json"
    path.write_text(json.dumps(permission(root, prepare=True)))
    assert main(["worktree", "prepare", "owner", "--root", str(root), "--authorization", str(path),
                 "--worktree", "build/worker", "--branch", "worker", "--resources", "recovery/resources.json"]) == 0
    report = json.loads(capsys.readouterr().out)
    assert report["compiler_prepared"] and not report["ready_for_collection"]


def test_existing_worker_can_refresh_recipes_without_recreating_or_resetting_it(compiler_project, capsys):
    root, resources = compiler_project
    report = prepare(root, authorization=permission(root, prepare=True), resources=resources)
    child = Path(report["worktree"])
    before = working_state(child)
    old = (child / "build/recipes.json").read_bytes()
    command = ["prepare", "--root", str(child), "--resources", str(root / "recovery/resources.json")]
    with file_lock(child / "build/recovery/driver.lock"):
        assert main(command) == 3
        assert "leased" in capsys.readouterr().err
        assert (child / "build/recipes.json").read_bytes() == old
    assert main(command) == 0
    refreshed = json.loads(capsys.readouterr().out)
    assert refreshed["published"] and refreshed["directory"] != report["preparation"]["directory"]
    assert (child / "build/recipes.json").read_bytes() != old
    assert working_state(child) == before


def test_generator_source_damage_is_preserved_and_never_reported_prepared(compiler_project):
    root, resources = compiler_project
    path = root / "recovery/prepare.json"
    settings = json.loads(path.read_text())
    code = "from pathlib import Path; import sys; (Path(sys.argv[1])/'code/owner.cc').write_text('generator damaged source')"
    settings["commands"].append({"argv": [sys.executable, "-c", code, "{root}"], "timeout": 5})
    path.write_text(json.dumps(settings))
    git(root, "add", "recovery/prepare.json")
    git(root, "commit", "-m", "Generator source damage control")
    before = working_state(root)
    with pytest.raises(ValueError, match="preserve and inspect"):
        prepare(root, authorization=permission(root, prepare=True), resources=resources)
    assert working_state(root) == before
    assert (root / "build/worker/code/owner.cc").read_text() == "generator damaged source"
    assert not (root / "build/worker/build/recipes.json").exists()


@pytest.mark.skipif(sys.platform != "linux", reason="Linux process liveness control")
def test_interrupted_setup_keeps_worktree_and_quarantines_compiler_slot(compiler_project):
    root, resources = compiler_project
    slot = Path(resources["slots"][0]["directory"])
    path = root / "recovery/prepare.json"
    settings = json.loads(path.read_text())
    code = ("import os,signal,time; from pathlib import Path; "
            "signal.signal(signal.SIGTERM,signal.SIG_IGN); "
            "(Path(os.environ['TEST_COMPILER_SLOT'])/'compiler.pid').write_text(str(os.getpid())); time.sleep(60)")
    settings["commands"] = [{"argv": [sys.executable, "-c", code, "{build}"], "timeout": 30}]
    path.write_text(json.dumps(settings))
    git(root, "add", "recovery/prepare.json")
    git(root, "commit", "-m", "Compiler interruption control")
    auth = root / "build/authorization.json"
    auth.write_text(json.dumps(permission(root, prepare=True)))
    before = working_state(root)
    with pytest.raises(subprocess.TimeoutExpired):
        run_bounded([sys.executable, "-I", "-m", "match_recovery", "worktree", "prepare", "owner",
                     "--root", str(root), "--authorization", str(auth), "--worktree", "build/worker",
                     "--branch", "worker", "--resources", "recovery/resources.json"], cwd=root, timeout=1)
    _assert_stopped(int((slot / "compiler.pid").read_text()))
    assert (slot / ACTIVE).exists()
    assert git(root / "build/worker", "rev-parse", "HEAD") == git(root, "rev-parse", "HEAD")
    assert working_state(root) == before
    with file_lock(slot / LOCK, timeout=0):
        pass
