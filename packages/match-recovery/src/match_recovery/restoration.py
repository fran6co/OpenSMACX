"""Remeasure source/history obligations without stamping or waiving them.

This is the measurement side of migration admission, not an admission decision.
In particular, a declared offset check cannot discharge a structural obligation,
and an unresolved reference cannot become a permitted spelling regression.
"""

from collections import Counter
from dataclasses import asdict
import hashlib
from pathlib import Path

from .application_check import check_task
from .measurement import Measurement as Settings
from .migration import check_obligations
from .migration_protocol import extend_inventory
from .model import Dimension, Measurement, Result, digest
from .source_obligations import task_obligations
from .structural_requirements import check_requirements, declared_outcome, structural_outcome
from .reviews import merge_interpretations, validate_review
from .tasks import TaskStore


def _all(results):
    values = tuple(results)
    return (Result.UNKNOWN if not values or Result.UNKNOWN in values else
            Result.FAIL if Result.FAIL in values else Result.PASS)


def _binary_outcome(row):
    """Select one complete trial, never stitch bytes and references across flags.

    Length differences are known matching failures but do not verify references.
    The separate reference prerequisite keeps those failures out of admission.
    A row can contain known differing bytes *and* unresolved targets; that is
    unknown for restoration, regardless of the diagnostic's aggregate FAIL.
    """
    if row is None or not row.get("variants"):
        return Result.UNKNOWN, Result.UNKNOWN, (row or {}).get("reason", "no measured product definition")
    trials = []
    for trial in row["variants"]:
        refs = trial.get("references", Result.UNKNOWN)
        fragments = trial.get("spans", [trial])
        is_fragment = ("fragment_result" in trial or trial.get("fragment") is True
                       or trial.get("callable") is False)
        parent_result = trial.get("parent_whole_body_result",
                                  Result.UNKNOWN if is_fragment else Result.PASS)
        fragment_result = trial.get("fragment_result",
                                    Result.UNKNOWN if is_fragment else Result.PASS)
        unresolved = any(relocation["result"] == Result.UNKNOWN
                         for fragment in fragments for relocation in fragment.get("relocations", []))
        if parent_result != Result.PASS or fragment_result != Result.PASS:
            # A passing child slice cannot discharge its mapped obligation
            # unless the complete containing source body/section also passed.
            result = Result.UNKNOWN
        elif unresolved or any(fragment.get("bytes", Result.UNKNOWN) == Result.UNKNOWN
                             and fragment.get("boundary") != Result.FAIL for fragment in fragments):
            result = Result.UNKNOWN
        elif trial.get("boundary") == Result.FAIL:
            result = Result.FAIL
        else:
            result = _all((trial.get("boundary", Result.UNKNOWN), trial.get("bytes", Result.UNKNOWN), refs))
        missing = sorted({relocation.get("symbol", relocation.get("target", "unresolved target"))
                          for fragment in fragments for relocation in fragment.get("relocations", [])
                          if relocation["result"] == Result.UNKNOWN})
        if missing:
            reason = "unresolved targets: " + ", ".join(missing)
        elif trial.get("reason"):
            reason = trial["reason"]
        elif refs != Result.PASS:
            reason = "reference mismatch" if refs == Result.FAIL else "reference proof unavailable"
        else:
            reason = {Result.FAIL: "known byte mismatch", Result.PASS: "strict bounded byte/reference agreement",
                      Result.UNKNOWN: "incomplete byte/boundary evidence"}[result]
        trials.append((result, refs, reason + "; single configured trial: " + repr(trial["flags"])))
    # A fully proved alternative suffices. Otherwise unresolved alternatives
    # stay unknown: they could still be the correct compiler configuration.
    wanted = (Result.PASS if any(item[0] == Result.PASS for item in trials) else
              Result.UNKNOWN if any(item[0] == Result.UNKNOWN for item in trials) else Result.FAIL)
    eligible = [item for item in trials if item[0] == wanted]
    # Prefer a reference-safe known regression, but never borrow its references
    # for a different trial's bytes. All returned dimensions share this witness.
    return min(eligible, key=lambda item: {Result.PASS: 0, Result.UNKNOWN: 1, Result.FAIL: 2}[item[1]])


def measure_restoration(root: Path, task_name: str, image: Path, *, compiler=None, command_for=None,
                        review=None, review_snapshot=None) -> dict:
    root = root.resolve()
    store = TaskStore(root)
    scope, _ = store.scope(task_name)
    inventory = task_obligations(root, task_name)
    if review is not None:
        validate_review(review)
        if review_snapshot is None or review["snapshot"] != review_snapshot:
            raise ValueError("restoration review requires the caller's current reviewed snapshot")
    elif review_snapshot is not None:
        raise ValueError("restoration review snapshot has no review")
    interpretations = {}
    merge_interpretations(interpretations, inventory.get("interpretations", []), scope)
    protected = {}
    extend_inventory(protected, inventory, scope)
    settings = Settings.read(root)
    def snapshot(value):
        with image.open("rb") as stream:
            image_sha = hashlib.file_digest(stream, "sha256").hexdigest()
        return settings.snapshot(root, {"inventory": value, "image_sha256": image_sha,
                                        "review": review, "review_snapshot": review_snapshot})
    before = snapshot(inventory)
    candidate = check_task(root, task_name, image, compiler=compiler, command_for=command_for)
    # Re-read history as well as source. A commit, scope expansion or removed
    # annotation during compilation must not produce a smaller protected set.
    after_inventory = task_obligations(root, task_name)
    if inventory != after_inventory or before != snapshot(after_inventory):
        raise ValueError("source, history, policy or task changed during restoration measurement")
    fingerprint = digest({"inputs": before, "candidate": candidate["input_fingerprint"]})
    requirements = check_requirements(inventory, candidate)
    rows = candidate["checks"]["binary"].get("functions", [])
    by_entity = {}
    for row in rows:
        if row["entity_id"] in by_entity or row["entity_id"] not in scope:
            raise ValueError("ambiguous or out-of-scope restoration measurement")
        by_entity[row["entity_id"]] = row
    outcomes = {entity: _binary_outcome(by_entity.get(entity)) for entity in scope}
    measurements, current, details = [], {}, []
    for key, obligation in sorted(protected.items()):
        exact, references, evidence = outcomes[obligation.entity_id]
        if obligation.dimension == Dimension.BYTES:
            result = exact
        elif obligation.dimension == Dimension.REFERENCES:
            result = references
        elif obligation.dimension == Dimension.INSTRUCTION_SHAPE and exact == Result.PASS:
            result = Result.PASS
            evidence = "strict byte/reference agreement also proves instruction shape; " + evidence
        elif obligation.dimension == Dimension.DECLARED_STRUCTURE:
            result, evidence = declared_outcome(
                (row for row in requirements if row["entity_id"] == obligation.entity_id),
                interpretations, review=review)
        elif obligation.dimension == Dimension.STRUCTURE:
            result, evidence = structural_outcome(row for row in requirements if row["entity_id"] == obligation.entity_id)
        else:
            result = Result.UNKNOWN
            evidence = "no native verifier for this obligation; byte mismatch is not a shape/runtime failure"
        current[key] = digest({"measurement": fingerprint, "obligation": key})
        measurements.append(Measurement(key, current[key], result, evidence))
        details.append({"obligation_id": key, "entity_id": obligation.entity_id,
                        "dimension": obligation.dimension, "result": result, "reason": evidence,
                        "references": references,
                        **({"requirement_ids": [row["interpretation_id"] for row in requirements
                                               if row["entity_id"] == obligation.entity_id]}
                           if obligation.dimension in (Dimension.STRUCTURE, Dimension.DECLARED_STRUCTURE) else {}),
                        **{name: by_entity[obligation.entity_id][name] for name in ("address", "symbol", "unit")
                           if name in by_entity.get(obligation.entity_id, {})}})
    gate = check_obligations(tuple(protected.values()), measurements, current)
    restoration = (Result.UNKNOWN if gate.unknown else Result.FAIL if gate.failed else Result.PASS)
    references = _all(value[1] for value in outcomes.values())
    unknown_dimensions = {row["dimension"] for row in details if row["result"] == Result.UNKNOWN}
    # More spelling edits cannot by themselves supply independent structural
    # review. Hand off only after the actual scoped measurements agree; missing
    # declaration witnesses or byte/reference failures still require recovery.
    review_structure = (candidate["result"] == Result.PASS and references == Result.PASS
                        and not gate.failed and unknown_dimensions
                        and unknown_dimensions <= {Dimension.STRUCTURE, Dimension.DECLARED_STRUCTURE}
                        and _all(row["result"] for row in requirements if row["current"]) == Result.PASS)
    return {"task_id": task_name, "check": "scoped_recovery_and_restoration",
            "result": _all((candidate["result"], restoration, references)),
            "next_action": "review_structure" if review_structure else "recover",
            "candidate": candidate, "restoration": restoration,
            "references": references, "protected": [asdict(item) | {"obligation_id": key}
                                                       for key, item in sorted(protected.items())],
            "measurements": [asdict(item) for item in measurements], "current": current,
            "details": details, "counts": dict(Counter(item.result for item in measurements)),
            "structural_requirements": requirements,
            "unrestored": asdict(gate), "input_fingerprint": fingerprint,
            "scope_complete": False, "ready_for_collection": False, "persisted": False,
            "limitations": ["fresh scoped obligation measurements, not migration admission or full acceptance",
                            "declared_structure covers bounded source requirements and reviewed historical dispositions, not complete structure/lifetime",
                            "non-exact instruction-shape obligations remain unknown",
                            "no full build, outside-scope preservation or runtime verification"]}
