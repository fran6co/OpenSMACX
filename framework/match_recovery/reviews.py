"""Non-reproducible review decisions; freshness is recomputed from source.

Review identity is trusted coordinator attribution, not provider authentication.
Approving a structural interpretation never establishes binary/build correctness.
"""

from pathlib import Path

from .journal import Journal
from .model import digest, require_digest, require_text
from .tasks import task_name


def validate_review(value: dict) -> None:
    required = {"kind", "author", "reviewer", "snapshot", "decision", "evidence", "feedback"}
    if not isinstance(value, dict) or not required <= set(value) or set(value) - required - {"supersessions"}:
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
    supersessions = value.get("supersessions", [])
    if not isinstance(supersessions, list):
        raise ValueError("review supersessions must be an array")
    seen = set()
    for item in supersessions:
        if not isinstance(item, dict) or set(item) != {"previous", "replacement", "evidence", "reason"}:
            raise ValueError("invalid interpretation supersession")
        for key in ("previous", "replacement"):
            require_digest(item[key])
        if item["previous"] == item["replacement"] or item["previous"] in seen:
            raise ValueError("duplicate or self-replacing interpretation")
        seen.add(item["previous"])
        require_text(item["reason"], "supersession reason")
        if not isinstance(item["evidence"], list) or not item["evidence"]:
            raise ValueError("supersession needs evidence")
        for evidence in item["evidence"]:
            require_text(evidence, "supersession evidence")
    if supersessions and value["decision"] != "approve":
        raise ValueError("only an approved review can supersede an interpretation")


def merge_interpretations(known: dict, rows: list, scope: frozenset[str]) -> None:
    """Keep encountered interpretations, but refresh which ones are current."""
    if not isinstance(rows, list):
        raise ValueError("interpretation inventory must be an array")
    additions = {}
    for row in rows:
        if not isinstance(row, dict) or set(row) != {"interpretation_id", "entity_id", "value", "current"}:
            raise ValueError("invalid interpretation inventory row")
        if (not isinstance(row["entity_id"], str) or row["entity_id"] not in scope
                or not isinstance(row["value"], dict) or not row["value"] or type(row["current"]) is not bool):
            raise ValueError("invalid interpretation scope, value or current status")
        key = digest({"entity_id": row["entity_id"], "value": row["value"]})
        if key != row["interpretation_id"] or key in additions:
            raise ValueError("invalid or duplicate interpretation identity")
        additions[key] = row
    known.update({key: {**row, "current": False} for key, row in known.items()})
    known.update(additions)


def validate_supersessions(review: dict, interpretations: dict) -> None:
    """A decision can replace an old interpretation, never erase proof duties."""
    validate_review(review)
    for item in review.get("supersessions", []):
        previous = interpretations.get(item["previous"])
        replacement = interpretations.get(item["replacement"])
        if previous is None or replacement is None:
            raise ValueError("supersession names an unknown source interpretation")
        if previous["current"] or not replacement["current"]:
            raise ValueError("supersession must replace a historical interpretation with a current one")
        if previous["entity_id"] != replacement["entity_id"]:
            raise ValueError("supersession must preserve the original evidence anchor")


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
