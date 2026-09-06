"""Admission is not verification; temporary regressions never erase obligations.

This pure policy layer accepts coordinator-produced measurements, not prose
claims from a worker. Persistence and execution adapters use the same policy.
"""

from __future__ import annotations

from dataclasses import dataclass
from enum import IntEnum
from typing import Mapping, Sequence

from .model import Measurement, Obligation, Result, require_digest, require_text


class GateExit(IntEnum):
    PASSED = 0
    FAILED = 1
    TRACKED_REGRESSIONS = 2
    UNMEASURABLE = 3


@dataclass(frozen=True)
class GateReport:
    exit_code: GateExit
    failed: tuple[str, ...] = ()
    unknown: tuple[str, ...] = ()
    tracked: tuple[str, ...] = ()


def check_obligations(obligations: Sequence[Obligation],
                      measurements: Sequence[Measurement],
                      current: Mapping[str, str],
                      allowed_regressions: frozenset[str] = frozenset()) -> GateReport:
    """Fresh FAIL may be tracked; missing/stale/UNKNOWN is never an allowed FAIL.

    `current` is keyed by obligation id, since distinct checks can have different
    dependencies. Unrelated new passes cannot make an old obligation disappear.
    """
    required = {item.obligation_id for item in obligations}
    if len(required) != len(obligations):
        raise ValueError("duplicate protected obligation")
    if not allowed_regressions <= required:
        raise ValueError("allowed regressions must name protected obligations")
    by_id = {item.obligation_id: item for item in measurements}
    if len(by_id) != len(measurements):
        raise ValueError("ambiguous measurements; select one current result per obligation")
    failed, unknown, tracked = [], [], []
    for key in sorted(required):
        item = by_id.get(key)
        if (item is None or current.get(key) != item.fingerprint
                or item.result == Result.UNKNOWN):
            unknown.append(key)
        elif item.result == Result.FAIL:
            (tracked if key in allowed_regressions else failed).append(key)
    exit_code = (GateExit.UNMEASURABLE if unknown else GateExit.FAILED if failed
                 else GateExit.TRACKED_REGRESSIONS if tracked else GateExit.PASSED)
    return GateReport(exit_code, tuple(failed), tuple(unknown), tuple(tracked))


@dataclass(frozen=True)
class Review:
    author: str
    reviewer: str
    snapshot: str
    result: Result
    evidence: tuple[str, ...]

    def __post_init__(self) -> None:
        require_text(self.author, "author")
        require_text(self.reviewer, "reviewer")
        require_digest(self.snapshot)
        if not isinstance(self.result, Result):
            raise ValueError("invalid review result")


@dataclass(frozen=True)
class Migration:
    migration_id: str
    author: str
    scope: frozenset[str]
    protected: tuple[Obligation, ...]
    allowed_regressions: frozenset[str]
    evidence: tuple[str, ...]

    def __post_init__(self) -> None:
        require_text(self.migration_id, "migration id")
        require_text(self.author, "author")
        if not self.scope or not self.evidence:
            raise ValueError("migration requires explicit scope and supporting evidence")
        keys = {item.obligation_id for item in self.protected}
        if len(keys) != len(self.protected):
            raise ValueError("duplicate protected obligation")
        if not self.allowed_regressions <= keys:
            raise ValueError("regressions must refer to protected obligations")
        matching = {item.obligation_id for item in self.protected
                    if item.dimension.value in ("bytes", "instruction_shape")}
        if not self.allowed_regressions <= matching:
            raise ValueError("only matching regressions may be temporarily admitted")
        if any(item.entity_id not in self.scope for item in self.protected):
            raise ValueError("protected entity is outside migration scope")

    def admit(self, *, snapshot: str, changed: frozenset[str], review: Review,
              build: Result, structural_checks: Result,
              measurements: Sequence[Measurement], current: Mapping[str, str]) -> GateReport:
        require_digest(snapshot)
        if not changed <= self.scope:
            raise ValueError("change exceeds migration scope; expand and review explicitly")
        if (review.author != self.author or review.reviewer == self.author
                or review.snapshot != snapshot or review.result != Result.PASS
                or not review.evidence):
            raise ValueError("a fresh, independent, evidence-backed review is required")
        if build != Result.PASS or structural_checks != Result.PASS:
            raise ValueError("build and declared structural checks must pass before admission")
        report = check_obligations(self.protected, measurements, current,
                                   self.allowed_regressions)
        if report.exit_code not in (GateExit.PASSED, GateExit.TRACKED_REGRESSIONS):
            raise ValueError(f"migration is not admissible: {report.exit_code.name}")
        return report

    def close(self, **checks) -> GateReport:
        report = self.admit(**checks)
        if report.exit_code != GateExit.PASSED:
            raise ValueError("restore every protected obligation before closing")
        return report
