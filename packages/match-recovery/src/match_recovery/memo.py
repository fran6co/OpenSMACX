"""Disposable JSON memoization. Never use this for verification verdicts."""

import os
from pathlib import Path
import tempfile
from typing import Callable

from .model import canonical, digest, require_digest
from .records import read_json


def memoized(root: Path | None, key: str, compute: Callable[[], dict]) -> dict:
    """Recompute on missing/corrupt/unavailable storage; failures are not cached.

    Callers must key every semantic input, including implementation versions.
    The checksum detects accidental corruption, not a malicious cache writer.
    """
    require_digest(key)
    if root is None:
        return compute()
    path = root / key[:2] / (key[2:] + ".json")
    try:
        entry = read_json(path)
        if (set(entry) == {"key", "value", "checksum"} and entry["key"] == key
                and isinstance(entry["value"], dict)
                and digest(entry["value"]) == entry["checksum"]):
            return entry["value"]
    except (OSError, ValueError, RecursionError):
        pass
    value = compute()
    if not isinstance(value, dict):
        raise TypeError("memo values must be JSON objects")
    payload = canonical({"key": key, "value": value, "checksum": digest(value)})
    temporary = None
    try:
        path.parent.mkdir(parents=True, exist_ok=True)
        fd, temporary = tempfile.mkstemp(prefix=".memo-", dir=path.parent)
        with os.fdopen(fd, "wb") as stream:
            stream.write(payload)
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temporary, path)
    except OSError:
        pass  # A read-only/full cache must not prevent recomputation.
    finally:
        if temporary is not None:
            try:
                Path(temporary).unlink(missing_ok=True)
            except OSError:
                pass
    return value
