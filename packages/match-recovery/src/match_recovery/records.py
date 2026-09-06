"""Strict, readable JSON records and atomic non-overwriting publication."""

import json
import os
from pathlib import Path
import tempfile

from .model import canonical


def _unique_pairs(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError(f"duplicate JSON key: {key}")
        result[key] = value
    return result


def _reject_constant(value):
    raise ValueError(f"non-finite JSON constant: {value}")


def read_json(path: Path) -> dict:
    return parse_json(path.read_bytes())


def parse_json(text: str | bytes) -> dict:
    value = json.loads(text, object_pairs_hook=_unique_pairs,
                       parse_constant=_reject_constant)
    if not isinstance(value, dict):
        raise ValueError("record must be a JSON object")
    return value


def publish_json(path: Path, value: dict, *, allow_identical: bool = False) -> bool:
    """True when created; never overwrite an existing record, even after a race."""
    encoded = canonical(value)
    payload = json.loads(encoded)
    if not isinstance(payload, dict):
        raise ValueError("record must be a JSON object")
    text = json.dumps(payload, indent=2, sort_keys=True, ensure_ascii=False, allow_nan=False) + "\n"
    return publish_text(path, text, allow_identical=allow_identical)


def publish_text(path: Path, text: str, *, allow_identical: bool = False) -> bool:
    """Atomically create a complete source/record file; never replace user edits."""
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, temporary = tempfile.mkstemp(prefix=".publish-", dir=path.parent)
    try:
        with os.fdopen(fd, "w", encoding="utf-8", newline="\n") as output:
            output.write(text)
            output.flush()
            os.fsync(output.fileno())
        try:
            os.link(temporary, path)
        except FileExistsError:
            if allow_identical and path.read_bytes() == text.encode("utf-8"):
                return False
            raise
        if os.name != "nt":
            directory_fd = os.open(path.parent, os.O_RDONLY)
            try:
                os.fsync(directory_fd)
            finally:
                os.close(directory_fd)
        return True
    finally:
        Path(temporary).unlink(missing_ok=True)
