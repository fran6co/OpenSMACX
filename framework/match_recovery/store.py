"""Immutable JSON observations, with atomic publication and integrity checks.

The caller chooses a versioned knowledge directory or an ignored run directory.
This store does not decide whether data is publishable; raw binary derivatives
must only be placed in ignored run storage by the target adapter.
"""

from __future__ import annotations

import json
import hashlib
import os
from pathlib import Path
import tempfile
from typing import Any

from .model import canonical, digest, require_digest


class CorruptObject(ValueError):
    pass


class ObjectStore:
    def __init__(self, root: Path):
        self.root = Path(root)

    def _path(self, object_id: str) -> Path:
        require_digest(object_id)
        return self.root / object_id[:2] / (object_id[2:] + ".json")

    def put(self, value: Any) -> str:
        payload = canonical(value)
        object_id = hashlib.sha256(payload).hexdigest()
        target = self._path(object_id)
        target.parent.mkdir(parents=True, exist_ok=True)
        if target.exists():
            self.get(object_id)
            return object_id
        fd, temporary = tempfile.mkstemp(prefix=".publish-", dir=target.parent)
        try:
            with os.fdopen(fd, "wb") as stream:
                stream.write(payload)
                stream.flush()
                os.fsync(stream.fileno())
            # O_EXCL-style publication: never overwrite an existing observation.
            try:
                os.link(temporary, target)
            except FileExistsError:
                self.get(object_id)
            directory_fd = os.open(target.parent, os.O_RDONLY)
            try:
                os.fsync(directory_fd)
            finally:
                os.close(directory_fd)
        finally:
            Path(temporary).unlink(missing_ok=True)
        return object_id

    def get(self, object_id: str) -> Any:
        path = self._path(object_id)
        try:
            value = json.loads(path.read_bytes())
            actual = digest(value)
        except (ValueError, UnicodeError, TypeError) as error:
            raise CorruptObject(f"unreadable observation {object_id}") from error
        if actual != object_id:
            raise CorruptObject(f"observation hash mismatch: {object_id}")
        return value
