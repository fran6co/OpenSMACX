"""Non-reproducible review decisions; freshness is recomputed from source.

Review identity is trusted coordinator attribution, not provider authentication.
Approving a structural interpretation never establishes binary/build correctness.
"""

from pathlib import Path

from .journal import Journal
from .model import digest, require_digest, require_text
from .tasks import task_name


def validate_review(value: dict) -> None:
    if not isinstance(value, dict) or set(value) != {"kind", "author", "reviewer", "snapshot", "decision", "evidence", "feedback"}:
        raise ValueError("invalid review decision fields")
    if value["kind"] != "review" or value["decision"] not in ("approve", "request_changes"):
        raise ValueError("invalid review disposition")
    for key in ("author", "reviewer", "feedback"):
        require_text(value[key], key)
    if value["author"] == value["reviewer"]:
        raise ValueError("reviewer must be independent of the worker")
    require_digest(value["snapshot"])
    if not isinstance(value["evidence"], list) or not value["evidence"]:
        raise ValueError("review needs evidence")
    for item in value["evidence"]:
        require_text(item, "review evidence")


class ReviewStore:
    def __init__(self, root: Path, task: str):
        self.root = root.resolve()
        self.stream = "review:" + task_name(task)
        self.journal = Journal(self.root / "recovery/history", lock_directory=self.root / "build/recovery/locks")

    def decisions(self) -> tuple[dict, ...]:
        values = tuple(event.payload for event in self.journal.read(self.stream))
        for value in values:
            validate_review(value)
        return values

    def append(self, value: dict, *, expected_revision: int) -> dict:
        validate_review(value)
        # Idempotent retry of exactly the same attributed decision.
        return self.journal.append(self.stream, expected_revision=expected_revision,
                                   request_id=digest(value), payload=value).payload

    def snapshot(self, working: dict, policy: dict) -> str:
        # The review record is an output of reviewing this source; hashing itself
        # would make it stale immediately. Only THIS review stream is omitted.
        # Task decisions, other histories, build scripts and all product source
        # remain inputs. HEAD/index are excluded so committing the decision alone
        # doesn't invalidate it. They are still guarded during command execution.
        directory = self.journal.stream_directory(self.stream).relative_to(self.root).as_posix() + "/"
        return digest({"files": {name: value for name, value in working["files"].items()
                                  if not name.startswith(directory)}, "policy": policy})


def acceptance_result(code: int, report: dict) -> str:
    """A separate full-project gate, never the scoped spelling-search check."""
    fields = {"scope", "result", "build", "structure", "references", "obligations", "runtime"}
    if set(report) != fields or report["scope"] != "full_project":
        raise ValueError("acceptance requires a full-project gate report")
    outcomes = [report[key] for key in fields - {"scope"}]
    if any(value not in ("pass", "fail", "unknown") for value in outcomes):
        raise ValueError("invalid acceptance outcome")
    dimensions = [report[key] for key in ("build", "structure", "references", "obligations", "runtime")]
    expected = "unknown" if "unknown" in dimensions else "fail" if "fail" in dimensions else "pass"
    if report["result"] != expected or code != {"pass": 0, "fail": 1, "unknown": 3}[expected]:
        raise ValueError("acceptance summary, dimensions and exit code disagree")
    return expected
