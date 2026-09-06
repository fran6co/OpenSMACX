"""Versionable JSON history with optimistic concurrency and retry safety.

Every event contains its payload: a fresh checkout needs no database or ignored
object store to read durable decisions. Large/private artifacts are referenced by
digest, not embedded. The coordinator alone publishes history; cross-branch
conflicts are explicit Git conflicts, never a last-writer-wins merge.
"""

from __future__ import annotations

from dataclasses import dataclass
import json
import os
from pathlib import Path
import tempfile
from typing import Any

from .locking import file_lock
from .model import canonical, digest, require_text


class ConcurrentUpdate(ValueError):
    pass


class JournalCorruption(ValueError):
    pass


@dataclass(frozen=True)
class Event:
    stream: str
    sequence: int
    request_id: str
    payload_id: str
    payload: dict[str, Any]
    event_id: str


class Journal:
    def __init__(self, directory: Path, *, lock_directory: Path | None = None):
        self.directory = Path(directory)
        self.lock_directory = (Path(lock_directory) if lock_directory is not None
                               else self.directory / ".locks")

    def stream_directory(self, stream: str) -> Path:
        require_text(stream, "stream")
        return self.directory / digest(stream)

    def read(self, stream: str, *, expected_head: str | None = None) -> tuple[Event, ...]:
        directory = self.stream_directory(stream)
        events, requests = [], set()
        previous = None
        # Writers publish only complete files. Reading an older complete prefix is
        # valid; the expected revision prevents acting on it after a new append.
        for sequence, path in enumerate(sorted(directory.glob("*.json")), 1):
            if path.name != f"{sequence:08d}.json":
                raise JournalCorruption(f"event stream has missing or extra entries: {stream}")
            try:
                record = json.loads(path.read_bytes())
                if not isinstance(record, dict) or record.get("schema_version") != 1:
                    raise JournalCorruption("unsupported journal schema")
                expected_fields = {"schema_version", "stream", "sequence", "request_id",
                                   "previous_event", "payload", "event_id"}
                if set(record) != expected_fields:
                    raise JournalCorruption("invalid event fields")
                event_id = record.pop("event_id")
                if digest(record) != event_id:
                    raise JournalCorruption("event content hash mismatch")
                if (record["stream"] != stream or type(record["sequence"]) is not int
                        or record["sequence"] != sequence or record["previous_event"] != previous):
                    raise JournalCorruption("event chain mismatch")
                request_id, payload = record["request_id"], record["payload"]
                require_text(request_id, "request id")
                if not isinstance(payload, dict):
                    raise JournalCorruption("event payload must be an object")
                require_text(payload.get("kind"), "event kind")
                if request_id in requests:
                    raise JournalCorruption("duplicate event request id")
                requests.add(request_id)
                events.append(Event(stream, sequence, request_id, digest(payload), payload, event_id))
                previous = event_id
            except (ValueError, TypeError, KeyError) as error:
                if isinstance(error, JournalCorruption):
                    raise
                raise JournalCorruption(f"invalid event {path.name}: {error}") from error
        # A committed migration checkpoint can pin its history head. Without such
        # a reference, deleting the final event is detectable by Git, not a guess
        # about how many events ought to exist.
        if expected_head is not None and previous != expected_head:
            raise JournalCorruption("history no longer reaches its checkpoint head")
        return tuple(events)

    def append(self, stream: str, *, expected_revision: int, request_id: str,
               payload: dict[str, Any]) -> Event:
        directory = self.stream_directory(stream)
        require_text(request_id, "request id")
        if type(expected_revision) is not int or expected_revision < 0:
            raise ValueError("expected revision must be a nonnegative integer")
        if not isinstance(payload, dict):
            raise ValueError("event payload must be a JSON object")
        require_text(payload.get("kind"), "event kind")
        payload = json.loads(canonical(payload))
        payload_id = digest(payload)
        with file_lock(self.lock_directory / (digest(stream) + ".lock")):
            events = self.read(stream)
            for event in events:
                if event.request_id == request_id:
                    if event.payload_id != payload_id:
                        raise ValueError("request id already belongs to a different event")
                    return event
            if len(events) != expected_revision:
                raise ConcurrentUpdate(f"{stream}: expected revision {expected_revision}, got {len(events)}")
            sequence = len(events) + 1
            record = {"schema_version": 1, "stream": stream, "sequence": sequence,
                      "request_id": request_id, "previous_event": events[-1].event_id if events else None,
                      "payload": payload}
            event_id = digest(record)
            record["event_id"] = event_id
            directory.mkdir(parents=True, exist_ok=True)
            target = directory / f"{sequence:08d}.json"
            fd, temporary = tempfile.mkstemp(prefix=".publish-", dir=directory)
            try:
                with os.fdopen(fd, "w", encoding="utf-8", newline="\n") as output:
                    json.dump(record, output, indent=2, sort_keys=True, ensure_ascii=False, allow_nan=False)
                    output.write("\n")
                    output.flush()
                    os.fsync(output.fileno())
                os.link(temporary, target)  # publish without overwriting existing history
                if os.name != "nt":
                    directory_fd = os.open(directory, os.O_RDONLY)
                    try:
                        os.fsync(directory_fd)
                    finally:
                        os.close(directory_fd)
            finally:
                Path(temporary).unlink(missing_ok=True)
            return Event(stream, sequence, request_id, payload_id, payload, event_id)
