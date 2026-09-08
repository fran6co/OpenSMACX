"""Non-reproducible review decisions; freshness is recomputed from source.

Review identity is trusted coordinator attribution, not provider authentication.
Approving a structural interpretation never establishes binary/build correctness.
"""

from pathlib import Path

from .journal import Journal
from .model import Dimension, Measurement, Obligation, Result, digest, require_digest, require_text
from .tasks import entity_id, task_name


def validate_review(value: dict) -> None:
    required = {"kind", "author", "reviewer", "snapshot", "decision", "evidence", "feedback"}
    if not isinstance(value, dict) or not required <= set(value) or set(value) - required - {"supersessions", "retractions"}:
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
    retractions = value.get("retractions", [])
    if not isinstance(retractions, list):
        raise ValueError("review retractions must be an array")
    for item in retractions:
        if not isinstance(item, dict) or set(item) != {"interpretation", "evidence", "reason"}:
            raise ValueError("invalid interpretation retraction")
        require_digest(item["interpretation"])
        if item["interpretation"] in seen:
            raise ValueError("duplicate interpretation disposition")
        seen.add(item["interpretation"])
        require_text(item["reason"], "retraction reason")
        if not isinstance(item["evidence"], list) or not item["evidence"]:
            raise ValueError("retraction needs evidence")
        for evidence in item["evidence"]:
            require_text(evidence, "retraction evidence")
    if retractions and value["decision"] != "approve":
        raise ValueError("only an approved review can retract an interpretation")


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
    for item in review.get("retractions", []):
        previous = interpretations.get(item["interpretation"])
        if previous is None or previous["current"]:
            raise ValueError("retraction requires a known historical interpretation")


def pending_interpretations(review: dict, interpretations: dict) -> list[str]:
    """Removed/revised interpretations need explicit disposition before admission."""
    validate_supersessions(review, interpretations)
    addressed = {item["previous"] for item in review.get("supersessions", [])}
    addressed.update(item["interpretation"] for item in review.get("retractions", []))
    return sorted(key for key, item in interpretations.items() if not item["current"] and key not in addressed)


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


def acceptance_outcome(code: int, report: dict) -> str:
    """Validate report syntax for diagnostics, NOT admission or freshness."""
    fields = {"scope", "result", "build", "structure", "references", "obligations", "runtime"}
    receipt = {"request_fingerprint", "measurements", "current"}
    if (not isinstance(report, dict) or not (fields | receipt) <= set(report)
            or set(report) - fields - receipt - {"findings"} or report["scope"] != "full_project"):
        raise ValueError("acceptance requires a full-project gate report")
    require_digest(report["request_fingerprint"])
    if not isinstance(report["measurements"], list) or not isinstance(report["current"], dict):
        raise ValueError("invalid acceptance measurements")
    seen = set()
    for row in report["measurements"]:
        if not isinstance(row, dict) or set(row) != {"obligation_id", "fingerprint", "result", "evidence"}:
            raise ValueError("invalid acceptance measurement")
        item = Measurement(row["obligation_id"], row["fingerprint"], Result(row["result"]), row["evidence"])
        if item.obligation_id in seen:
            raise ValueError("duplicate acceptance measurement")
        seen.add(item.obligation_id)
    for key, fingerprint in report["current"].items():
        require_text(key, "obligation id")
        require_digest(fingerprint)
    outcomes = [report[key] for key in fields - {"scope"}]
    if any(value not in ("pass", "fail", "unknown") for value in outcomes):
        raise ValueError("invalid acceptance outcome")
    dimensions = [report[key] for key in ("build", "structure", "references", "obligations", "runtime")]
    expected = "fail" if "fail" in dimensions else "unknown" if "unknown" in dimensions else "pass"
    if report["result"] != expected or code != {"pass": 0, "fail": 1, "unknown": 3}[expected]:
        raise ValueError("acceptance summary, dimensions and exit code disagree")
    findings = report.get("findings", [])
    if not isinstance(findings, list):
        raise ValueError("acceptance findings must be an array")
    for finding in findings:
        if (not isinstance(finding, dict) or set(finding) != {"dimension", "message", "evidence"}
                or not isinstance(finding["dimension"], str)
                or finding["dimension"] not in fields - {"scope", "result"}):
            raise ValueError("invalid acceptance finding")
        if report[finding["dimension"]] == "pass":
            raise ValueError("acceptance finding contradicts a passing dimension")
        require_text(finding["message"], "acceptance finding message")
        if not isinstance(finding["evidence"], list) or not finding["evidence"]:
            raise ValueError("acceptance finding needs evidence")
        for evidence in finding["evidence"]:
            require_text(evidence, "acceptance finding evidence")
    return expected


def acceptance_result(code: int, report: dict, *, request: dict) -> str:
    """Bind a trusted verifier's report to this invocation and protected set.

    Neither the receipt nor textual evidence makes an untrusted verifier sound.
    The verifier still owns image/compiler freshness and the actual full checks.
    Missing/stale protected measurements may explain unknown, never a pass.
    """
    outcome = acceptance_outcome(code, report)
    fields = {"task", "snapshot", "protected", "review", "interpretations", "verification_id"}
    if not isinstance(request, dict) or set(request) != fields:
        raise ValueError("invalid acceptance request")
    require_digest(request["snapshot"])
    require_text(request["verification_id"], "verification id")
    validate_review(request["review"])
    if request["review"]["decision"] != "approve" or request["review"]["snapshot"] != request["snapshot"]:
        raise ValueError("acceptance requires the reviewed snapshot")
    if report["request_fingerprint"] != digest(request):
        raise ValueError("acceptance report refers to a stale or different request")
    if not isinstance(request["protected"], list):
        raise ValueError("invalid acceptance protected obligations")
    protected = set()
    for row in request["protected"]:
        if not isinstance(row, dict) or set(row) != {"entity_id", "dimension", "baseline", "obligation_id"}:
            raise ValueError("invalid acceptance protected obligation")
        entity_id(row["entity_id"])
        item = Obligation(row["entity_id"], Dimension(row["dimension"]), row["baseline"])
        if item.obligation_id != row["obligation_id"] or item.obligation_id in protected:
            raise ValueError("invalid or duplicate acceptance obligation identity")
        protected.add(item.obligation_id)
    measured = {row["obligation_id"]: row for row in report["measurements"]}
    if set(measured) - protected or set(report["current"]) - protected:
        raise ValueError("acceptance measurements name unrequested obligations")
    results = []
    for key in protected:
        row = measured.get(key)
        results.append(Result.UNKNOWN if row is None or row["fingerprint"] != report["current"].get(key)
                       else Result(row["result"]))
    if Result.UNKNOWN in results and report["obligations"] == "pass":
        raise ValueError("acceptance obligations omit unknown or stale measurements")
    if Result.FAIL in results and report["obligations"] == "pass":
        raise ValueError("acceptance obligations contradict a failed measurement")
    return outcome
