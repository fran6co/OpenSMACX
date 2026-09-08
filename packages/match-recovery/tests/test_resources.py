"""Portable resource allocation controls; no installed compiler or provider."""

import json
import os
from pathlib import Path
import subprocess
import sys
import threading
import time

import pytest

from match_recovery import cli
from match_recovery.locking import file_lock
from match_recovery.resources import ACTIVE, INHERITED, LOCK, resource_environment


@pytest.fixture
def pool(tmp_path):
    root = tmp_path / "project"
    (root / "recovery").mkdir(parents=True)
    (root / "recovery/project.json").write_text(json.dumps({"compiler": {"environment": ["TEST_COMPILER_SLOT"]}}))
    slots = []
    for index in range(2):
        directory = tmp_path / f"slot-{index}"
        directory.mkdir()
        slots.append({"name": f"slot-{index}", "directory": str(directory),
                      "environment": {"TEST_COMPILER_SLOT": "{resource}"}})
    config = {"schema_version": 1, "slots": slots}
    (root / "recovery/resources.json").write_text(json.dumps(config))
    return root, config


def test_environment_is_inherited_by_children_and_restored_on_exception(pool, monkeypatch):
    root, config = pool
    monkeypatch.setenv("TEST_COMPILER_SLOT", "previous")
    with pytest.raises(ValueError, match="control"):
        with resource_environment(config, [root]) as name:
            assert name == "slot-0"
            slot = Path(config["slots"][0]["directory"])
            assert json.loads((slot / ACTIVE).read_text())["pid"] == os.getpid()
            child = subprocess.run([sys.executable, "-c", "import os; print(os.environ['TEST_COMPILER_SLOT'])"],
                                   capture_output=True, text=True, check=True)
            assert child.stdout.strip() == str(slot)
            with pytest.raises(TimeoutError):
                with file_lock(slot / LOCK, timeout=0):
                    pytest.fail("allocated resource was not locked")
            raise ValueError("control")
    assert os.environ["TEST_COMPILER_SLOT"] == "previous" and INHERITED not in os.environ
    assert not (slot / ACTIVE).exists()
    with resource_environment(config, [root]):
        pass


def test_busy_first_slot_uses_next_without_changing_parent_or_unset_environment(pool, monkeypatch):
    root, config = pool
    monkeypatch.delenv("TEST_COMPILER_SLOT", raising=False)
    first = Path(config["slots"][0]["directory"])
    with file_lock(first / LOCK):
        with resource_environment(config, [root]) as name:
            assert name == "slot-1"
    assert "TEST_COMPILER_SLOT" not in os.environ


def test_interrupted_marker_is_not_reaped_from_pid_or_empty_content(pool):
    root, config = pool
    for slot in config["slots"]:
        (Path(slot["directory"]) / ACTIVE).write_text("")
    with pytest.raises(TimeoutError, match="busy or interrupted"):
        with resource_environment(config, [root]):
            pytest.fail("quarantined slots must not be reused")
    assert all((Path(slot["directory"]) / ACTIVE).read_text() == "" for slot in config["slots"])


def test_wait_can_acquire_a_released_slot_and_expires_when_all_busy(pool):
    root, config = pool
    first, second = (Path(slot["directory"]) for slot in config["slots"])
    ready, release = threading.Event(), threading.Event()
    def holder():
        with file_lock(first / LOCK):
            ready.set()
            release.wait(5)
    thread = threading.Thread(target=holder)
    thread.start()
    try:
        assert ready.wait(2)
        with file_lock(second / LOCK):
            before = time.monotonic()
            with pytest.raises(TimeoutError):
                with resource_environment(config, [root], timeout=0.03):
                    pass
            assert time.monotonic() - before >= 0.03
            timer = threading.Timer(0.03, release.set)
            timer.start()
            with resource_environment(config, [root], timeout=2) as name:
                assert name == "slot-0"
            timer.join(timeout=2)
    finally:
        release.set()
        thread.join(timeout=5)
    assert not thread.is_alive()


def test_interrupt_quarantines_and_changed_marker_is_preserved(pool):
    root, config = pool
    first, second = (Path(slot["directory"]) for slot in config["slots"])
    with pytest.raises(KeyboardInterrupt):
        with resource_environment(config, [root]):
            raise KeyboardInterrupt()
    assert (first / ACTIVE).is_file() and INHERITED not in os.environ
    with pytest.raises(ValueError, match="marker changed"):
        with resource_environment(config, [root]) as name:
            assert name == "slot-1"
            (second / ACTIVE).write_text("preserve changed ownership")
    assert (second / ACTIVE).read_text() == "preserve changed ownership"


def test_cross_process_exclusion_and_abrupt_exit_quarantine(pool):
    root, config = pool
    source = """
import json, os, sys
from pathlib import Path
from match_recovery.resources import resource_environment
root = Path(sys.argv[1])
config = json.loads((root / 'recovery/resources.json').read_text())
with resource_environment(config, [root]) as slot:
    print(slot, flush=True)
    sys.stdin.readline()
    os._exit(23)
"""
    environment = dict(os.environ)
    child = None
    try:
        # communicate is bounded; this independent parent lease proves the child
        # cannot choose the same slot while the parent is executing.
        with resource_environment(config, [root]) as slot:
            child = subprocess.Popen([sys.executable, "-c", source, str(root)], stdin=subprocess.PIPE,
                                     stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, env=environment)
            stdout, stderr = child.communicate("exit\n", timeout=10)
            assert slot == "slot-0" and stdout.strip() == "slot-1", stderr
        assert child.returncode == 23
        assert (Path(config["slots"][1]["directory"]) / ACTIVE).exists()
        with file_lock(Path(config["slots"][0]["directory"]) / LOCK):
            with pytest.raises(TimeoutError):
                with resource_environment(config, [root]):
                    pytest.fail("kernel unlock does not prove descendants terminated")
    finally:
        if child is not None:
            if child.poll() is None:
                child.kill()
            child.communicate(timeout=10)


@pytest.mark.parametrize("kind", ["symlink", "hardlink"])
def test_lock_alias_refuses_without_modifying_aliased_file(pool, kind):
    root, config = pool
    first = Path(config["slots"][0]["directory"])
    target = first / "untouched"
    target.write_bytes(b"keep")
    if kind == "symlink":
        (first / LOCK).symlink_to(target)
    else:
        os.link(target, first / LOCK)
    with pytest.raises((ValueError, OSError)):
        with resource_environment(config, [root]):
            pytest.fail("aliased lock accepted")
    assert target.read_bytes() == b"keep" and not (first / ACTIVE).exists()


@pytest.mark.parametrize("damage", ["missing", "relative", "duplicate", "nested", "directory_alias",
                                    "unknown_variable", "unequal_variables", "bad_schema", "empty", "extra",
                                    "placeholder", "invalid_variable", "duplicate_name", "reserved"])
def test_invalid_configuration_refuses_before_leasing(pool, damage):
    root, config = pool
    first, second = config["slots"]
    if damage == "missing":
        second["directory"] += "/missing"
    elif damage == "relative":
        second["directory"] = "slot-1"
    elif damage == "duplicate":
        second["directory"] = first["directory"]
    elif damage == "nested":
        child = Path(first["directory"]) / "nested"
        child.mkdir()
        second["directory"] = str(child)
    elif damage == "directory_alias":
        alias = root / "alias"
        alias.symlink_to(first["directory"], target_is_directory=True)
        first["directory"] = str(alias)
    elif damage == "unknown_variable":
        for slot in config["slots"]:
            slot["environment"] = {"UNKNOWN_COMPILER_INPUT": "value"}
    elif damage == "unequal_variables":
        second["environment"] = {"PATH": "value"}
    elif damage == "bad_schema":
        config["schema_version"] = True
    elif damage == "empty":
        config["slots"] = []
    elif damage == "extra":
        config["implicit_install"] = True
    elif damage == "placeholder":
        first["directory"] = "{bad}"
    elif damage == "invalid_variable":
        first["environment"] = {"A=B": "bad"}
    elif damage == "duplicate_name":
        second["name"] = first["name"]
    elif damage == "reserved":
        first["environment"] = {INHERITED: "value"}
    with pytest.raises(ValueError):
        with resource_environment(config, [root]):
            pytest.fail("invalid configuration was admitted")
    assert not list(root.parent.rglob(ACTIVE))
    assert not list(root.parent.rglob(LOCK))


@pytest.mark.parametrize("timeout", [-1, float("inf"), float("nan"), True, 3601])
def test_invalid_timeouts_refuse(pool, timeout):
    root, config = pool
    with pytest.raises(ValueError, match="timeout"):
        with resource_environment(config, [root], timeout=timeout):
            pass


def test_env_root_expansion_and_nested_allocation_refusal(pool, monkeypatch):
    root, config = pool
    monkeypatch.setenv("TEST_POOL_ROOT", str(root.parent))
    config["slots"][0]["directory"] = "{env:TEST_POOL_ROOT}/slot-0"
    with resource_environment(config, [root]):
        with pytest.raises(ValueError, match="inherit"):
            with resource_environment(config, [root]):
                pass
        child = subprocess.run([sys.executable, "-c", "from match_recovery.resources import resource_environment; "
                                "from pathlib import Path; "
                                "resource_environment({}, [Path('.')]).__enter__()"], capture_output=True, text=True, timeout=10)
        assert child.returncode != 0 and "inherit" in child.stderr


@pytest.mark.parametrize("command", [["run", "owner"], ["start", "--planner", "planner.json"],
                                    ["integrate", "publish", "owner", "--candidate", ".", "--authorization", "auth.json", "--reviewer", "reviewer.json"]])
def test_top_level_cli_wraps_entire_dispatch_and_preserves_result(pool, monkeypatch, command):
    root, config = pool
    seen = []
    def dispatch(args):
        seen.append(os.environ["TEST_COMPILER_SLOT"])
        assert (Path(seen[-1]) / ACTIVE).exists()
        return 2
    monkeypatch.setattr(cli, "_dispatch", dispatch)
    assert cli.main([*command, "--root", str(root), "--resources", "recovery/resources.json"]) == 2
    assert seen == [config["slots"][0]["directory"]]
    assert not (Path(seen[0]) / ACTIVE).exists()


def test_candidate_compiler_must_declare_allocated_variables_too(pool, monkeypatch, capsys):
    root, config = pool
    candidate = root / "candidate"
    (candidate / "recovery").mkdir(parents=True)
    (candidate / "recovery/project.json").write_text('{"compiler": {"environment": []}}')
    monkeypatch.setattr(cli, "_dispatch", lambda args: pytest.fail("dispatch before candidate validation"))
    assert cli.main(["integrate", "publish", "owner", "--root", str(root), "--candidate", "candidate",
                     "--authorization", "auth.json", "--reviewer", "reviewer.json",
                     "--resources", "recovery/resources.json"]) == 3
    assert "each checkout" in capsys.readouterr().err
    assert not list(root.parent.rglob(LOCK))


def test_cli_busy_pool_never_dispatches_and_timeout_requires_pool(pool, monkeypatch, capsys):
    root, config = pool
    monkeypatch.setattr(cli, "_dispatch", lambda args: pytest.fail("unexpected dispatch"))
    assert cli.main(["run", "owner", "--resource-timeout", "1"]) == 3
    assert "requires --resources" in capsys.readouterr().err
    for slot in config["slots"]:
        (Path(slot["directory"]) / ACTIVE).write_text("interrupted")
    assert cli.main(["run", "owner", "--root", str(root), "--resources", "recovery/resources.json"]) == 3
    assert "busy or interrupted" in capsys.readouterr().err
