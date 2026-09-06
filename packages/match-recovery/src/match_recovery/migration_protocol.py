"""Trusted target/coordinator contracts for protected structural checkpoints.

Permissions are decisions. Inventories and measurement reports are disposable
outputs of source readers and verifiers, never worker-authored proof records.
"""

from .migration import Migration, Review
from .model import Dimension, Measurement, Obligation, Result, require_digest, require_text
from .tasks import entity_id


def permissions(value: dict, task, scope: frozenset[str]) -> dict:
    if not isinstance(value, dict) or set(value) != {"migration_id", "author", "evidence", "allowed"}:
        raise ValueError("invalid migration permission fields")
    require_text(value["migration_id"], "migration id")
    if value["author"] != task.author:
        raise ValueError("migration permission requires the task coordinator")
    if not isinstance(value["evidence"], list) or not value["evidence"]:
        raise ValueError("migration permission needs evidence")
    for item in value["evidence"]:
        require_text(item, "migration evidence")
    if not isinstance(value["allowed"], list):
        raise ValueError("allowed regressions must be an array")
    seen = set()
    for item in value["allowed"]:
        if not isinstance(item, dict) or set(item) != {"entity_id", "dimension"}:
            raise ValueError("invalid allowed regression")
        entity_id(item["entity_id"])
        require_text(item["dimension"], "regression dimension")
        key = (item["entity_id"], item["dimension"])
        if item["entity_id"] not in scope or item["dimension"] not in ("bytes", "instruction_shape"):
            raise ValueError("only scoped matching regressions may be allowed")
        if key in seen:
            raise ValueError("duplicate allowed regression")
        seen.add(key)
    return value


def extend_inventory(frozen: dict[str, Obligation], report: dict, scope: frozenset[str]) -> None:
    """Never replace the protected set with a later, smaller source inventory."""
    if not isinstance(report, dict) or not isinstance(report.get("obligations"), list):
        raise ValueError("invalid restoration inventory")
    additions = {}
    for row in report["obligations"]:
        fields = {"entity_id", "dimension", "baseline", "obligation_id"}
        if not isinstance(row, dict) or not fields <= set(row) or set(row) - fields - {"source_evidence"}:
            raise ValueError("invalid restoration obligation")
        entity_id(row["entity_id"])
        require_text(row["dimension"], "obligation dimension")
        item = Obligation(row["entity_id"], Dimension(row["dimension"]), row["baseline"])
        if item.entity_id not in scope or item.obligation_id != row["obligation_id"]:
            raise ValueError("restoration obligation identity or scope disagrees")
        if item.obligation_id in additions:
            raise ValueError("duplicate restoration obligation")
        additions[item.obligation_id] = item
    frozen.update(additions)


def admit(permission: dict, protected: tuple[Obligation, ...], scope: frozenset[str],
          decision: dict, snapshot: str, code: int, report: dict):
    fields = {"snapshot", "changed", "build", "structure", "references", "outside_scope", "measurements", "current"}
    if not isinstance(report, dict) or set(report) != fields or report["snapshot"] != snapshot:
        raise ValueError("invalid or stale migration check report")
    if code != 0:
        raise ValueError("migration verifier could not complete its checks")
    require_digest(snapshot)
    if not isinstance(report["changed"], list):
        raise ValueError("invalid changed entity inventory")
    for entity in report["changed"]:
        entity_id(entity)
    if len(set(report["changed"])) != len(report["changed"]):
        raise ValueError("duplicate changed entity")
    if Result(report["outside_scope"]) != Result.PASS:
        raise ValueError("outside-scope preservation must be verified before admission")
    if not isinstance(report["measurements"], list) or not isinstance(report["current"], dict):
        raise ValueError("invalid migration measurements")
    measurements = []
    for row in report["measurements"]:
        if not isinstance(row, dict) or set(row) != {"obligation_id", "fingerprint", "result", "evidence"}:
            raise ValueError("invalid migration measurement")
        measurements.append(Measurement(row["obligation_id"], row["fingerprint"], Result(row["result"]), row["evidence"]))
    for fingerprint in report["current"].values():
        require_digest(fingerprint)
    keys = {item.obligation_id for item in protected}
    if set(report["current"]) - keys or {item.obligation_id for item in measurements} - keys:
        raise ValueError("migration measurements name unrequested obligations")
    allowed_pairs = {(row["entity_id"], row["dimension"]) for row in permission["allowed"]}
    available_pairs = {(item.entity_id, item.dimension.value) for item in protected}
    if not allowed_pairs <= available_pairs:
        raise ValueError("permission names an obligation absent from protected source")
    migration = Migration(permission["migration_id"], decision["author"], scope, protected,
                          frozenset(item.obligation_id for item in protected
                                    if (item.entity_id, item.dimension.value) in allowed_pairs),
                          tuple(permission["evidence"]))
    review = Review(decision["author"], decision["reviewer"], decision["snapshot"],
                    Result.PASS if decision["decision"] == "approve" else Result.FAIL,
                    tuple(decision["evidence"]))
    return migration.admit(snapshot=snapshot, changed=frozenset(report["changed"]), review=review,
                           build=Result(report["build"]), structural_checks=Result(report["structure"]),
                           references=Result(report["references"]),
                           measurements=measurements, current=report["current"])
