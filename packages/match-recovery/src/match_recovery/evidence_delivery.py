"""Bounded, explicit routing for oversized worker diagnostics.

The authoritative request remains on disk unchanged.  Only diagnostic values
under the worker's context/check payload are replaced in the provider prompt;
the replacement carries enough information for a filesystem-capable provider
to retrieve the exact value from that immutable request.
"""

from __future__ import annotations

from copy import deepcopy
import hashlib
from typing import Any

from .model import canonical


REQUEST_THRESHOLD = 128 * 1024
EVIDENCE_THRESHOLD = 8 * 1024
REFERENCE_KEY = "__match_recovery_on_demand_evidence__"


def _token(value: str) -> str:
    """Encode one RFC 6901 JSON Pointer token."""
    return value.replace("~", "~0").replace("/", "~1")


def _reference(pointer: str, value: list[Any] | str, request_path: str,
              request_sha256: str, manifest: list[dict[str, Any]] | None) -> dict[str, Any]:
    metadata = {
        "pointer": pointer,
        "request_path": request_path,
        "request_sha256": request_sha256,
        "value_type": "array" if isinstance(value, list) else "string",
        "item_count": len(value) if isinstance(value, list) else 1,
        "byte_count": len(canonical(value)),
    }
    if manifest is not None:
        manifest.append(metadata)
    return {REFERENCE_KEY: {
        "schema": "match-recovery/evidence-reference/v1",
        "request_path": request_path,
        "request_sha256": request_sha256,
        "json_pointer": pointer,
        "value_type": metadata["value_type"],
        "item_count": metadata["item_count"],
        "byte_count": metadata["byte_count"],
        "omitted_evidence_is_not": "a passing or empty result",
        "instruction": "Read the full request, resolve this JSON Pointer, and inspect the omitted evidence before editing.",
    }}


def _compact(value: Any, pointer: str, request_path: str,
             request_sha256: str, manifest: list[dict[str, Any]] | None) -> Any:
    if isinstance(value, (list, str)):
        if len(canonical(value)) > EVIDENCE_THRESHOLD:
            return _reference(pointer, value, request_path, request_sha256, manifest)
        if isinstance(value, list):
            return [_compact(item, pointer + "/" + str(index), request_path, request_sha256, manifest)
                    for index, item in enumerate(value)]
        return value
    if isinstance(value, dict):
        return {_key: _compact(item, pointer + "/" + _token(_key), request_path, request_sha256, manifest)
                for _key, item in value.items()}
    return value


def compact_worker_request(request: dict[str, Any], request_path: str,
                           request_sha256: str,
                           manifest: list[dict[str, Any]] | None = None) -> dict[str, Any]:
    """Return the worker prompt view while retaining the request authority.

    This function deliberately only visits the three diagnostic roots.  Task,
    scope, round, instructions and every authorization/review field therefore
    remain byte-for-byte equivalent to the original request.
    """
    result = deepcopy(request)
    context = result.get("context")
    if isinstance(context, dict):
        for name in ("impact", "restoration"):
            if name in context:
                context[name] = _compact(context[name], "/context/" + name,
                                         request_path, request_sha256, manifest)
    if isinstance(result.get("check"), dict):
        result["check"] = _compact(result["check"], "/check", request_path, request_sha256, manifest)
    return result


def request_sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()
