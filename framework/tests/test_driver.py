from dataclasses import replace
import subprocess
import sys

import pytest

from match_recovery.driver import Command, drive, invoke
from match_recovery.model import Origin
from match_recovery.tasks import Task, TaskStore


CONFIG = {"schema_version": 1, "target": "synthetic",
          "context": {"argv": ["context"], "timeout": 10},
          "check": {"argv": ["check"], "timeout": 10}}
WORKER = {"author": "worker", "command": {"argv": ["worker"], "timeout": 10}}


@pytest.fixture
def project(tmp_path):
    def git(*args):
        return subprocess.run(["git", *args], cwd=tmp_path, check=True, capture_output=True).stdout
    git("init")
    git("config", "user.name", "Synthetic")
    git("config", "user.email", "synthetic@example.invalid")
    (tmp_path / ".gitignore").write_text("build/\n")
    (tmp_path / "src").mkdir()
    (tmp_path / "src/f.cpp").write_text("unfinished\n")
    git("add", ".")
    git("commit", "-m", "synthetic baseline")
    task = Task("fixture", "synthetic", "coordinator", "Recover fixture", git("rev-parse", "HEAD").decode().strip(),
                (Origin("a" * 64, "fixture", 16),), ("src/f.cpp",), ("synthetic evidence",))
    TaskStore(tmp_path).create(task)
    git("add", ".")
    git("commit", "-m", "task decision")
    return tmp_path, task


def run(project, worker, *, rounds=3, checker=None):
    root, task = project
    def execute(command, root, task_name, payload):
        name = command.argv[0]
        if name == "context":
            return 0, {"source": "src/f.cpp"}
        if name == "check":
            if checker:
                return checker(root)
            passed = (root / "src/f.cpp").read_text() == "finished\n"
            return (0 if passed else 1), {"result": "pass" if passed else "fail"}
        return worker(root, payload)
    return drive(root, task.task_id, CONFIG, WORKER, iterations=rounds, execute=execute)


def test_driver_really_edits_rechecks_and_stops_before_collection(project):
    def worker(root, payload):
        assert payload["check"]["result"] == "fail"
        assert payload["editable_files"] == ["src/f.cpp"]
        (root / "src/f.cpp").write_text("finished\n")
        return 0, {"discoveries": []}
    result = run(project, worker)
    assert result["stop_reason"] == "independent_review_required"
    assert result["worker_rounds"] == 1 and result["check"]["result"] == "pass"
    assert not result["ready_for_collection"]


def test_multiple_rounds_and_budget_exhaustion_preserve_source(project):
    def worker(root, payload):
        (root / "src/f.cpp").write_text(f"attempt {payload['round']}\n")
        return 0, {"discoveries": []}
    result = run(project, worker, rounds=2)
    assert result["stop_reason"] == "iteration_budget_exhausted" and result["worker_rounds"] == 2
    assert (project[0] / "src/f.cpp").read_text() == "attempt 2\n"
    assert not (project[0] / "recovery/history").exists()  # no measurement ledger


def test_no_change_does_not_burn_the_remaining_budget(project):
    result = run(project, lambda *_: (0, {"discoveries": []}))
    assert result["stop_reason"] == "no_source_progress" and result["worker_rounds"] == 1


def test_cross_function_discovery_is_preserved_and_stops_for_scope_review(project):
    root, task = project
    other = replace(task.subjects[0], address=32)
    def worker(*_):
        return 0, {"discoveries": [{"hypothesis": "The owner has another member", "affects": [other.entity_id],
                                    "evidence": ["human-supplied constructor note"]}]}
    result = run(project, worker)
    assert result["stop_reason"] == "review_scope_expansion"
    assert not result["check_is_current"]
    assert TaskStore(root).decisions(task.task_id)[0]["hypothesis"] == "The owner has another member"


@pytest.mark.parametrize("interrupted", [False, True])
def test_scope_violation_is_detected_even_on_interruption_and_never_reverted(project, interrupted):
    def worker(root, payload):
        (root / "src/other.cpp").write_text("outside scope")
        if interrupted:
            raise subprocess.TimeoutExpired("worker", 10)
        return 0, {"discoveries": []}
    with pytest.raises(ValueError, match="exceeded editing scope"):
        run(project, worker)
    assert (project[0] / "src/other.cpp").read_text() == "outside scope"


def test_readonly_checker_cannot_modify_source(project):
    def checker(root):
        (root / "src/f.cpp").write_text("finished\n")
        return 0, {"result": "pass"}
    with pytest.raises(ValueError, match="read-only"):
        run(project, lambda *_: pytest.fail("worker must not run"), checker=checker)


def test_false_checker_exit_and_worker_verdict_are_refused(project):
    with pytest.raises(ValueError, match="disagree"):
        run(project, lambda *_: None, checker=lambda _: (0, {"result": "unknown"}))
    with pytest.raises(ValueError, match="discoveries array"):
        run(project, lambda *_: (0, {"result": "pass"}))


def test_unrelated_dirty_work_refuses_dispatch(project):
    (project[0] / "unrelated.txt").write_text("user work")
    with pytest.raises(ValueError, match="unrelated"):
        run(project, lambda *_: pytest.fail("worker must not run"))


def test_worker_cannot_stage_edits_instead_of_returning_them(project):
    def worker(root, payload):
        (root / "src/f.cpp").write_text("finished\n")
        subprocess.run(["git", "add", "src/f.cpp"], cwd=root, check=True)
        return 0, {"discoveries": []}
    with pytest.raises(ValueError, match="HEAD or index"):
        run(project, worker)


def test_interrupted_worker_keeps_edits_but_does_not_relabel_old_check_as_fresh(project):
    def worker(root, payload):
        (root / "src/f.cpp").write_text("partial\n")
        return 3, {}
    result = run(project, worker)
    assert result["stop_reason"] == "worker_interrupted" and not result["check_is_current"]
    assert (project[0] / "src/f.cpp").read_text() == "partial\n"


def test_real_command_timeout_is_bounded(tmp_path):
    command = Command((sys.executable, "-c", "import time; time.sleep(30)"), 1)
    with pytest.raises(subprocess.TimeoutExpired):
        invoke(command, tmp_path, "fixture", {})


def test_real_command_protocol_uses_stdin_json_not_shell(tmp_path):
    command = Command((sys.executable, "-c", "import sys,json; x=json.load(sys.stdin); print(json.dumps(x))"), 10)
    code, result = invoke(command, tmp_path, "fixture", {"literal": "$(not-a-shell)"})
    assert code == 0 and result == {"literal": "$(not-a-shell)"}


def test_real_subprocess_worker_drives_an_alternate_target(project):
    root, task = project
    config = {**CONFIG,
              "context": {"argv": [sys.executable, "-c", "print('{}')"], "timeout": 10},
              "check": {"argv": [sys.executable, "-c", "from pathlib import Path; import json,sys; p=Path('src/f.cpp').read_text()=='finished'; print(json.dumps({'result':'pass' if p else 'fail'})); sys.exit(0 if p else 1)"], "timeout": 10}}
    worker = {"author": "synthetic-worker", "command": {"argv": [sys.executable, "-c", "from pathlib import Path; Path('src/f.cpp').write_text('finished'); print('{\"discoveries\":[]}')"], "timeout": 10}}
    result = drive(root, task.task_id, config, worker, iterations=2)
    assert result["stop_reason"] == "independent_review_required" and result["worker_rounds"] == 1
