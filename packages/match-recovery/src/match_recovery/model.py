"""Portable identities and proof vocabulary; no compiler or agent dependencies.

An observation is not a current verdict. A component classification is not a
verification exemption. Names, source paths and function extents may change
without changing the identity of the original entry point.
"""

from __future__ import annotations

from dataclasses import asdict, dataclass, is_dataclass
from enum import StrEnum
import hashlib
import json
import re
from typing import Any


def canonical(value: Any) -> bytes:
    """The common on-disk and fingerprint encoding (reject NaN and Infinity)."""
    return (json.dumps(_json_value(value), sort_keys=True, ensure_ascii=False,
                       separators=(",", ":"), allow_nan=False) + "\n").encode("utf-8")


def _json_value(value: Any) -> Any:
    if is_dataclass(value) and not isinstance(value, type):
        value = asdict(value)
    if isinstance(value, dict):
        if any(not isinstance(key, str) for key in value):
            raise ValueError("JSON object keys must be strings")
        return {key: _json_value(item) for key, item in value.items()}
    if isinstance(value, (list, tuple)):
        return [_json_value(item) for item in value]
    if isinstance(value, frozenset):
        return sorted((_json_value(item) for item in value), key=canonical)
    return value


def digest(value: Any) -> str:
    return hashlib.sha256(canonical(value)).hexdigest()


def require_digest(value: str) -> None:
    if not isinstance(value, str) or not re.fullmatch(r"[0-9a-f]{64}", value):
        raise ValueError("expected a lowercase SHA-256 digest")


def require_text(value: str, what: str) -> None:
    if not isinstance(value, str) or not value.strip():
        raise ValueError(f"{what} must be nonempty text")


@dataclass(frozen=True)
class Origin:
    image_sha256: str
    address_space: str
    address: int

    def __post_init__(self) -> None:
        require_digest(self.image_sha256)
        require_text(self.address_space, "address space")
        if type(self.address) is not int or self.address < 0:
            raise ValueError("address must be a nonnegative integer")

    @property
    def entity_id(self) -> str:
        return "entity-" + digest(self)


@dataclass(frozen=True, order=True)
class Span:
    start: int
    end: int

    def __post_init__(self) -> None:
        if (type(self.start) is not int or type(self.end) is not int
                or not 0 <= self.start < self.end):
            raise ValueError("span must be a nonempty half-open address range")


class Ownership(StrEnum):
    APPLICATION = "application"
    THIRD_PARTY = "third_party"
    RUNTIME = "runtime"
    SYSTEM = "system"
    GENERATED = "generated"
    UNKNOWN = "unknown"


class Delivery(StrEnum):
    RECOVERED_SOURCE = "recovered_source"
    UPSTREAM_SOURCE = "upstream_source"
    TOOLCHAIN = "toolchain"
    SYSTEM_IMPORT = "system_import"
    GENERATED = "generated"
    UNRESOLVED = "unresolved"


@dataclass(frozen=True)
class Component:
    component_id: str
    ownership: Ownership
    delivery: Delivery
    evidence: tuple[str, ...]
    version: str | None = None

    def __post_init__(self) -> None:
        require_text(self.component_id, "component id")
        if not isinstance(self.ownership, Ownership) or not isinstance(self.delivery, Delivery):
            raise ValueError("component ownership and delivery must use the typed vocabulary")
        if self.ownership != Ownership.UNKNOWN and not self.evidence:
            raise ValueError("component classification requires evidence")
        for item in self.evidence:
            require_text(item, "evidence reference")

    @property
    def workflow(self) -> str:
        # A coarse imported `library` label deliberately routes to identification.
        # Neither unknown ownership nor unresolved delivery silently drops work.
        if self.ownership == Ownership.UNKNOWN:
            return "identify"
        if self.ownership == Ownership.APPLICATION:
            return "recover"
        if self.ownership == Ownership.GENERATED:
            return "recover_owner"
        return "resolve_dependency" if self.delivery == Delivery.UNRESOLVED else "verify_dependency"


class Result(StrEnum):
    PASS = "pass"
    FAIL = "fail"
    UNKNOWN = "unknown"


class Dimension(StrEnum):
    BYTES = "bytes"
    INSTRUCTION_SHAPE = "instruction_shape"
    REFERENCES = "references"
    DECLARED_STRUCTURE = "declared_structure"
    STRUCTURE = "structure"
    RUNTIME = "runtime"


@dataclass(frozen=True)
class Obligation:
    """A protected requirement, independent of current path or component owner."""

    entity_id: str
    dimension: Dimension
    baseline: str

    def __post_init__(self) -> None:
        require_text(self.entity_id, "entity id")
        require_text(self.baseline, "baseline attestation")
        if not isinstance(self.dimension, Dimension):
            raise ValueError("invalid obligation dimension")

    @property
    def obligation_id(self) -> str:
        return "obligation-" + digest(self)


@dataclass(frozen=True)
class Measurement:
    """One coordinator-observed check on an exact input/configuration snapshot."""

    obligation_id: str
    fingerprint: str
    result: Result
    evidence: str

    def __post_init__(self) -> None:
        require_text(self.obligation_id, "obligation id")
        require_digest(self.fingerprint)
        require_text(self.evidence, "measurement evidence")
        if not isinstance(self.result, Result):
            raise ValueError("invalid measurement result")
