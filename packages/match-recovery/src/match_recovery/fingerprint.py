"""Content-based input identity; timestamps are never evidence of freshness."""

from __future__ import annotations

import hashlib
import json
import os
from pathlib import Path, PurePosixPath
from typing import Any, Iterable

from .model import canonical, digest


def _content_digest(stream, buffer: bytearray) -> str:
    """Hash a whole input, reusing storage but never another input's bytes."""
    value = hashlib.sha256()
    view = memoryview(buffer)
    while True:
        count = stream.readinto(buffer)
        if count is None:
            raise BlockingIOError("snapshot input would block")
        if count == 0:
            return value.hexdigest()
        value.update(view[:count])


def input_snapshot(root: Path, files: Iterable[str], configuration: dict[str, Any]) -> dict[str, Any]:
    root = os.fspath(root.resolve())
    inputs = {}
    buffer = bytearray(2**18)
    for name in sorted(set(files)):
        relative = PurePosixPath(name)
        if (not name or relative.is_absolute() or ".." in relative.parts
                or "\\" in name or str(relative) != name):
            raise ValueError(f"expected a normalized relative input path: {name}")
        path = os.path.join(root, name)
        # Resolve every input as before. Compare path components without
        # constructing a Path object for every possible ancestor; a string
        # prefix would incorrectly admit a sibling such as project-other.
        if os.path.commonpath((root, os.path.realpath(path))) != root:
            raise ValueError(f"input resolves outside the worktree: {name}")
        try:
            with open(path, "rb") as stream:
                inputs[name] = _content_digest(stream, buffer)
        except FileNotFoundError:
            # Deletion changes the fingerprint; it does not prove buildability.
            inputs[name] = None
    snapshot = {"schema_version": 1, "inputs": inputs,
                "configuration": json.loads(canonical(configuration))}
    # Validate that configuration is serializable before a caller starts work.
    digest(snapshot)
    return snapshot
