"""Idle siblings cannot acquire conflicting ownership during serial coordination."""

from pathlib import Path

import pytest

from match_recovery.campaign_peers import peer_epoch
from match_recovery.driver import working_state
from match_recovery.locking import file_lock
from match_recovery.model import Origin
from match_recovery.scheduling import _state as reservation_state
from match_recovery.tasks import TaskStore
from test_preservation import project
from test_task_source import git


@pytest.fixture
def peers(project):
    root = project
    child = root / "build/peer"
    git(root, "worktree", "add", "-b", "peer", str(child))
    jobs = [{"task": "current", "root": str(root / "build/current")},
            {"task": "owner", "root": str(child)}]
    return root, child, jobs


def test_preserved_wip_is_read_only_and_peer_lease_is_held(peers):
    root, child, jobs = peers
    path = child / "code/model.h"
    path.write_text(path.read_text() + "// Keep structural WIP\n")
    before, source = reservation_state(child), working_state(root)
    with peer_epoch(root, "current", jobs) as fresh:
        fresh()
        assert reservation_state(child) == before
        with pytest.raises(TimeoutError):
            with file_lock(child / "build/recovery/driver.lock", timeout=0):
                pytest.fail("sibling lease must remain held")
    assert reservation_state(child) == before and working_state(root) == source
    with file_lock(child / "build/recovery/driver.lock", timeout=0):
        pass


def test_leased_sibling_refuses_without_touching_wip(peers):
    root, child, jobs = peers
    before = reservation_state(child)
    with file_lock(child / "build/recovery/driver.lock", timeout=0):
        with pytest.raises(TimeoutError):
            with peer_epoch(root, "current", jobs):
                pytest.fail("leased sibling must refuse the epoch")
    assert reservation_state(child) == before


@pytest.mark.parametrize("ignored", [False, True])
@pytest.mark.parametrize("check", ["explicit", "exit", "exception"])
def test_late_scope_decisions_are_detected_even_when_ignored(peers, ignored, check):
    root, child, jobs = peers
    if ignored:
        path = child / ".gitignore"
        path.write_text(path.read_text() + "recovery/history/\n")
    store = TaskStore(child)
    task = store.read("owner")
    original = task.subjects[0]
    acquired = Origin(original.image_sha256, original.address_space, original.address + 16)
    with pytest.raises(ValueError, match="peer state or scope changed"):
        with peer_epoch(root, "current", jobs) as fresh:
            store.expand("owner", request_id="late", author=task.author, subjects=[acquired],
                         editable_files=["code/caller.cc"], evidence=["Controlled late ownership"], expected_revision=0)
            if check == "explicit": fresh()
            if check == "exception": raise RuntimeError("Provider failure does not skip freshness")
    assert acquired.entity_id in store.scope("owner")[0]  # Preserve the conflicting decision.


def test_unchanged_peer_does_not_mask_original_failure(peers):
    root, child, jobs = peers
    with pytest.raises(RuntimeError, match="original failure"):
        with peer_epoch(root, "current", jobs):
            raise RuntimeError("original failure")
    with file_lock(child / "build/recovery/driver.lock", timeout=0):
        pass


def test_absent_collected_sibling_is_not_recreated(project):
    missing = project / "build/collected"
    jobs = [{"task": "owner", "root": str(missing)}]
    with peer_epoch(project, "current", jobs) as fresh:
        fresh()
    assert not missing.exists()
    with pytest.raises(ValueError, match="reappeared"):
        with peer_epoch(project, "current", jobs):
            missing.mkdir(parents=True)
    assert missing.exists()


@pytest.mark.parametrize("damage", ["staged", "branch", "operation", "source"])
def test_late_peer_checkout_changes_refuse(peers, damage):
    root, child, jobs = peers
    with pytest.raises(ValueError):
        with peer_epoch(root, "current", jobs):
            if damage in ("staged", "source"):
                path = child / "code/model.h"
                path.write_text(path.read_text() + "// Different source\n")
                if damage == "staged": git(child, "add", "code/model.h")
            if damage == "branch": git(child, "branch", "-m", "moved-peer")
            if damage == "operation":
                Path(git(child, "rev-parse", "--path-format=absolute", "--git-path", "MERGE_HEAD")).write_text(
                    git(child, "rev-parse", "HEAD"))


def test_symlinked_peer_is_not_followed(peers):
    root, child, jobs = peers
    alias = root / "build/peer-alias"
    alias.symlink_to(child, target_is_directory=True)
    jobs[1]["root"] = str(alias)
    before = reservation_state(child)
    with pytest.raises(ValueError, match="canonical"):
        with peer_epoch(root, "current", jobs):
            pytest.fail("peer aliases must refuse")
    assert reservation_state(child) == before
