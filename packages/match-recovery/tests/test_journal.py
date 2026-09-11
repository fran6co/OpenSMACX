from concurrent.futures import ThreadPoolExecutor
import os
import json
import shutil

import pytest

from match_recovery.fingerprint import input_snapshot
from match_recovery.journal import ConcurrentUpdate, Journal, JournalCorruption
from match_recovery.model import digest


def test_reading_absent_history_does_not_create_files(tmp_path):
    root = tmp_path / "absent"
    assert Journal(root).read("task") == ()
    assert not root.exists()


def test_history_survives_reopening_and_retry_is_idempotent(tmp_path):
    journal = Journal(tmp_path)
    first = journal.append("migration", expected_revision=0, request_id="start",
                           payload={"kind": "opened", "obligations": ["first", "second"]})
    journal.append("migration", expected_revision=1, request_id="pause",
                   payload={"kind": "paused", "reason": "quota", "lost_matches": ["first"]})
    reopened = Journal(tmp_path)
    assert reopened.read("migration")[0] == first
    assert reopened.append("migration", expected_revision=0, request_id="start",
                           payload=first.payload) == first
    assert len(reopened.read("migration")) == 2


def test_retry_with_different_payload_is_refused(tmp_path):
    journal = Journal(tmp_path)
    journal.append("task", expected_revision=0, request_id="one", payload={"kind": "open"})
    with pytest.raises(ValueError, match="different event"):
        journal.append("task", expected_revision=1, request_id="one", payload={"kind": "closed"})
    assert journal.read("task")[0].payload == {"kind": "open"}


def test_stale_writer_does_not_overwrite_concurrent_discovery(tmp_path):
    journal = Journal(tmp_path)
    journal.append("task", expected_revision=0, request_id="one", payload={"kind": "discovered"})
    with pytest.raises(ConcurrentUpdate):
        journal.append("task", expected_revision=0, request_id="two", payload={"kind": "closed"})
    assert len(journal.read("task")) == 1
    journal.append("task", expected_revision=1, request_id="two", payload={"kind": "paused"})
    assert [event.sequence for event in journal.read("task")] == [1, 2]


def test_pinned_append_detects_a_valid_same_length_history_rewrite(tmp_path):
    journal = Journal(tmp_path)
    original = journal.append("task", expected_revision=0, request_id="one", payload={"kind": "open"})
    alternate = Journal(tmp_path / "alternate")
    alternate.append("task", expected_revision=0, request_id="one", payload={"kind": "different"})
    shutil.copy2(alternate.stream_directory("task") / "00000001.json",
                 journal.stream_directory("task") / "00000001.json")
    with pytest.raises(ConcurrentUpdate, match="observed head"):
        journal.append("task", expected_revision=1, expected_head=original.event_id,
                       request_id="two", payload={"kind": "paused"})
    assert len(journal.read("task")) == 1


def test_pinned_append_and_exact_retry_require_the_current_observed_head(tmp_path):
    journal = Journal(tmp_path)
    first = journal.append("task", expected_revision=0, request_id="one", payload={"kind": "open"})
    second = journal.append("task", expected_revision=1, expected_head=first.event_id,
                            request_id="two", payload={"kind": "paused"})
    assert journal.append("task", expected_revision=1, expected_head=second.event_id,
                          request_id="two", payload=second.payload) == second
    with pytest.raises(ConcurrentUpdate, match="observed head"):
        journal.append("task", expected_revision=1, expected_head=first.event_id,
                       request_id="two", payload=second.payload)
    assert len(journal.read("task")) == 2


def test_concurrent_publishers_produce_one_event(tmp_path):
    def append(index):
        try:
            return Journal(tmp_path).append("task", expected_revision=0,
                                            request_id=str(index), payload={"kind": "opened"})
        except ConcurrentUpdate:
            return None
    with ThreadPoolExecutor(max_workers=8) as workers:
        events = list(workers.map(append, range(8)))
    assert sum(event is not None for event in events) == 1
    assert len(Journal(tmp_path).read("task")) == 1


def test_missing_journal_event_is_detected(tmp_path):
    journal = Journal(tmp_path)
    journal.append("task", expected_revision=0, request_id="one", payload={"kind": "open"})
    journal.append("task", expected_revision=1, request_id="two", payload={"kind": "paused"})
    (journal.stream_directory("task") / "00000001.json").unlink()
    with pytest.raises(JournalCorruption, match="missing"):
        journal.read("task")


def test_future_schema_is_not_silently_rewritten(tmp_path):
    journal = Journal(tmp_path)
    journal.append("task", expected_revision=0, request_id="one", payload={"kind": "open"})
    path = journal.stream_directory("task") / "00000001.json"
    record = json.loads(path.read_bytes())
    record["schema_version"] = 2
    path.write_text(json.dumps(record))
    with pytest.raises(JournalCorruption, match="schema"):
        journal.read("task")
    with pytest.raises(JournalCorruption, match="schema"):
        journal.append("task", expected_revision=1, request_id="two", payload={"kind": "closed"})


def test_history_is_readable_without_database_or_private_artifacts(tmp_path):
    first = Journal(tmp_path / "first")
    event = first.append("task", expected_revision=0, request_id="one",
                         payload={"kind": "discovered", "reason": "field is part of an instance"})
    second = Journal(tmp_path / "fresh-checkout")
    destination = second.stream_directory("task")
    destination.mkdir(parents=True)
    shutil.copy2(first.stream_directory("task") / "00000001.json", destination)
    assert second.read("task", expected_head=event.event_id) == (event,)
    assert not list(destination.glob("*.sqlite*"))
    assert not (second.directory / "objects").exists()


def test_checkpoint_head_detects_deleted_tail(tmp_path):
    journal = Journal(tmp_path)
    event = journal.append("task", expected_revision=0, request_id="one", payload={"kind": "open"})
    (journal.stream_directory("task") / "00000001.json").unlink()
    with pytest.raises(JournalCorruption, match="checkpoint head"):
        journal.read("task", expected_head=event.event_id)


def test_history_detects_edited_payload(tmp_path):
    journal = Journal(tmp_path)
    journal.append("task", expected_revision=0, request_id="one", payload={"kind": "open"})
    path = journal.stream_directory("task") / "00000001.json"
    record = json.loads(path.read_bytes())
    record["payload"]["kind"] = "verified"
    path.write_text(json.dumps(record))
    with pytest.raises(JournalCorruption, match="hash mismatch"):
        journal.read("task")


def test_changed_content_invalidates_even_with_same_size_and_timestamp(tmp_path):
    header = tmp_path / "class.h"
    header.write_text("int a;")
    before_stat = header.stat()
    first = input_snapshot(tmp_path, ["class.h"], {"compiler": "vc6"})
    header.write_text("int b;")
    os.utime(header, ns=(before_stat.st_atime_ns, before_stat.st_mtime_ns))
    second = input_snapshot(tmp_path, ["class.h"], {"compiler": "vc6"})
    assert header.stat().st_size == before_stat.st_size
    assert digest(first) != digest(second)


def test_fingerprints_are_worktree_portable_and_configuration_sensitive(tmp_path):
    first, second = tmp_path / "one", tmp_path / "two"
    first.mkdir()
    second.mkdir()
    for root in (first, second):
        (root / "class.h").write_text("int member;")
    config = {"compiler": "vc6", "dependency_lock": "old", "verifier": "1"}
    snapshot = input_snapshot(first, ["class.h"], config)
    assert digest(snapshot) == digest(input_snapshot(second, ["class.h"], config))
    for key in config:
        changed = {**config, key: "different"}
        assert digest(snapshot) != digest(input_snapshot(first, ["class.h"], changed))
    assert digest(snapshot) != digest(input_snapshot(first, ["class.h", "added.h"], config))
    config["compiler"] = "mutated after capture"
    assert snapshot["configuration"]["compiler"] == "vc6"


@pytest.mark.parametrize("name", ["../outside", "/absolute", "a/../b", "", "a\\b"])
def test_fingerprint_rejects_nonlocal_paths(tmp_path, name):
    with pytest.raises(ValueError):
        input_snapshot(tmp_path, [name], {})
