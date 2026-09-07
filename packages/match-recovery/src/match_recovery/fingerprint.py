"""Content-based input identity; timestamps are never evidence of freshness."""

from __future__ import annotations

import hashlib
import json
import os
from pathlib import Path, PurePosixPath
from typing import Any, Iterable

from .model import canonical, digest


def input_snapshot(root: Path, files: Iterable[str], configuration: dict[str, Any]) -> dict[str, Any]:
    root = root.resolve()
    inputs = {}
    for name in sorted(set(files)):
        relative = PurePosixPath(name)
        if (not name or relative.is_absolute() or ".." in relative.parts
                or "\\" in name or str(relative) != name):
            raise ValueError(f"expected a normalized relative input path: {name}")
        path = root / name
        # Resolve every input as before. Compare path components without
        # constructing a Path object for every possible ancestor; a string
        # prefix would incorrectly admit a sibling such as project-other.
        if os.path.commonpath((root, path.resolve())) != str(root):
            raise ValueError(f"input resolves outside the worktree: {name}")
        try:
            with path.open("rb") as stream:
                inputs[name] = hashlib.file_digest(stream, "sha256").hexdigest()
        except FileNotFoundError:
            # Deletion changes the fingerprint; it does not prove buildability.
            inputs[name] = None
    snapshot = {"schema_version": 1, "inputs": inputs,
                "configuration": json.loads(canonical(configuration))}
    # Validate that configuration is serializable before a caller starts work.
    digest(snapshot)
    return snapshot
