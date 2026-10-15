"""Descriptive recovery throughput from explicit paired measurements.

This module does not read history, source annotations, claims, or saved reports.
Callers provide an immutable original-identity denominator, before/after
per-function observations, implementation classifications, and timing receipts.
The result is descriptive only; it is never an acceptance or recovery proof.
"""

from collections import Counter
import math
from typing import Any, Mapping, Sequence

from .model import digest, require_digest, require_text


SCHEMA_VERSION = 1
MAX_IDENTITIES = 100_000
MAX_ATTEMPTS = 10_000
MAX_ATTEMPT_GROUPS = 1_000

_MEASUREMENTS = frozenset(("strict_exact", "masked_exact", "measured_nonexact", "mismatch",
                           "unknown", "stale", "noncomparable"))
_IMPLEMENTATIONS = frozenset(("genuine", "scaffold", "placeholder", "missing", "unknown"))
_TIMING_FIELDS = ("worker_elapsed_seconds", "wall_elapsed_seconds", "compile_elapsed_seconds")


def _text(value: Any, what: str) -> str:
    require_text(value, what)
    return value


def _mapping(value: Any, what: str) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        raise ValueError(f"{what} must be an object")
    return value


def _fingerprint(value: Any, what: str) -> str:
    value = _text(value, what)
    if value != value.strip():
        raise ValueError(f"{what} must not have surrounding whitespace")
    return value


def _identity_inventory(rows: Sequence[Mapping[str, Any]]) -> dict[str, str]:
    if not isinstance(rows, (list, tuple)) or len(rows) > MAX_IDENTITIES:
        raise ValueError("original identities must be a bounded list")
    inventory: dict[str, str] = {}
    for row in rows:
        row = _mapping(row, "original identity")
        identity = _text(row.get("identity"), "identity")
        fingerprint = _fingerprint(row.get("original_fingerprint"), "original fingerprint")
        if identity in inventory:
            raise ValueError(f"duplicate original identity: {identity}")
        inventory[identity] = fingerprint
    return inventory


def _batch(batch: Mapping[str, Any], name: str, inventory: Mapping[str, str]) -> dict[str, Any]:
    batch = _mapping(batch, f"{name} measurement batch")
    source_fp = _fingerprint(batch.get("source_fingerprint"), f"{name} source fingerprint")
    image_fp = _fingerprint(batch.get("original_image_fingerprint"), f"{name} original image fingerprint")
    toolchain_fp = _fingerprint(batch.get("toolchain_fingerprint"), f"{name} toolchain fingerprint")
    observations = batch.get("observations")
    if not isinstance(observations, (list, tuple)) or len(observations) > MAX_IDENTITIES:
        raise ValueError(f"{name} observations must be a bounded list")
    indexed: dict[str, dict[str, str]] = {}
    for row in observations:
        row = _mapping(row, f"{name} observation")
        identity = _text(row.get("identity"), "observation identity")
        if identity in indexed:
            raise ValueError(f"duplicate {name} observation: {identity}")
        if identity not in inventory:
            raise ValueError(f"{name} observation is outside the original identity denominator: {identity}")
        original_fp = _fingerprint(row.get("original_fingerprint"), f"{name} row original fingerprint")
        result = _text(row.get("result"), f"{name} measurement result")
        if result not in _MEASUREMENTS:
            raise ValueError(f"invalid {name} measurement result: {result}")
        row_source_fp = _fingerprint(row.get("source_fingerprint"), f"{name} row source fingerprint")
        if original_fp != inventory[identity]:
            result = "stale"
        elif row_source_fp != source_fp:
            result = "stale"
        indexed[identity] = {"result": result, "original_fingerprint": original_fp,
                             "source_fingerprint": row_source_fp}
    return {"source_fingerprint": source_fp, "original_image_fingerprint": image_fp,
            "toolchain_fingerprint": toolchain_fp, "observations": indexed,
            "receipt_fingerprint": batch.get("receipt_fingerprint"),
            "verification": batch.get("verification")}


def source_measurement_batch(report: Mapping[str, Any],
                             identity_bindings: Mapping[tuple[str, str], Mapping[str, str]], *,
                             source_fingerprint: str, original_image_fingerprint: str,
                             toolchain_fingerprint: str) -> dict[str, Any]:
    """Adapt a native ``measure_source`` report without upgrading masked bytes.

    ``identity_bindings`` maps ``(path, address)`` rows to the caller's stable
    original identity and its fingerprint. The source fingerprint is supplied
    by the caller's current source snapshot; ``report.input_fingerprint`` is
    separately retained as the receipt binding. A pass at ``BYTE_EXACT`` from
    this API is classified as ``masked_exact`` only. It never creates a
    ``strict_exact`` observation.
    """
    report = _mapping(report, "native source measurement report")
    if report.get("operation") != "measure" or report.get("schema_version") != 1:
        raise ValueError("unsupported native source measurement report")
    receipt = report.get("input_fingerprint")
    try:
        require_digest(receipt)
    except ValueError as exc:
        raise ValueError("native source report lacks a valid input fingerprint") from exc
    verification = report.get("verification")
    if verification not in ("historical_masked_bytes_only", "source_masked_bytes_and_component_binary",
                            "source_and_system_import_original_thunks"):
        raise ValueError("native source report has an unsupported verification mode")
    if not isinstance(identity_bindings, Mapping):
        raise ValueError("identity bindings must be a mapping")
    source_fingerprint = _fingerprint(source_fingerprint, "source fingerprint")
    image_fingerprint = _fingerprint(original_image_fingerprint, "original image fingerprint")
    toolchain_fingerprint = _fingerprint(toolchain_fingerprint, "toolchain fingerprint")
    rows = report.get("measurements")
    if not isinstance(rows, (list, tuple)) or len(rows) > MAX_IDENTITIES:
        raise ValueError("native source measurements must be a bounded list")
    observations = []
    seen_keys = set()
    result_counts: Counter[str] = Counter()
    for row in rows:
        row = _mapping(row, "native measurement row")
        path = _text(row.get("path"), "measurement path")
        address = _text(row.get("address"), "measurement address")
        key = (path, address)
        if key in seen_keys:
            raise ValueError(f"duplicate native measurement row: {path} {address}")
        seen_keys.add(key)
        binding = identity_bindings.get(key)
        if binding is None:
            raise ValueError(f"native measurement has no original identity binding: {path} {address}")
        binding = _mapping(binding, "original identity binding")
        identity = _text(binding.get("identity"), "bound identity")
        original_fp = _fingerprint(binding.get("original_fingerprint"), "bound original fingerprint")
        result = row.get("result")
        verdict = row.get("verdict")
        if result not in ("pass", "fail", "unknown"):
            raise ValueError("native measurement row has an unsupported result")
        result_counts[result] += 1
        if result == "pass" and verdict == "BYTE_EXACT":
            normalized = "masked_exact"
        elif result == "fail":
            normalized = "mismatch"
        elif result == "unknown":
            normalized = "unknown"
        elif result == "pass":
            if not isinstance(verdict, str) or not verdict or verdict == "UNRESOLVED":
                raise ValueError("passing native row lacks a measured comparison tier")
            normalized = "measured_nonexact"
        else:
            raise ValueError("native measurement row has an unsupported result/verdict pair")
        observations.append({"identity": identity, "original_fingerprint": original_fp,
                            "source_fingerprint": source_fingerprint, "result": normalized})
    if set(identity_bindings) != seen_keys:
        raise ValueError("native report and identity bindings do not cover the same selected rows")
    aggregate_result = ("fail" if result_counts["fail"] else
                        "unknown" if result_counts["unknown"] else "pass")
    if report.get("result") != aggregate_result:
        raise ValueError("native aggregate result disagrees with per-function observations")
    if report.get("selected") != len(rows) or report.get("counts") != dict(result_counts):
        raise ValueError("native selection/count summaries disagree with per-function observations")
    return {"source_fingerprint": source_fingerprint,
            "original_image_fingerprint": image_fingerprint,
            "toolchain_fingerprint": toolchain_fingerprint,
            "receipt_fingerprint": receipt,
            "verification": verification,
            "observations": observations}


def _implementation_statuses(rows: Mapping[str, str] | None, inventory: Mapping[str, str], name: str) -> dict[str, str]:
    if rows is None:
        rows = {}
    rows = _mapping(rows, f"{name} implementation statuses")
    statuses: dict[str, str] = {}
    for identity in inventory:
        status = rows.get(identity, "unknown")
        if status not in _IMPLEMENTATIONS:
            raise ValueError(f"invalid {name} implementation status for {identity}: {status}")
        statuses[identity] = status
    if set(rows) - set(inventory):
        raise ValueError(f"{name} implementation status is outside the original identity denominator")
    return statuses


def _timings(timings: Mapping[str, Any] | None) -> dict[str, float | None]:
    if timings is None:
        timings = {}
    timings = _mapping(timings, "timings")
    if set(timings) - set(_TIMING_FIELDS):
        raise ValueError("timings contain unsupported fields")
    values: dict[str, float | None] = {}
    for name in _TIMING_FIELDS:
        value = timings.get(name)
        if value is None:
            values[name] = None
            continue
        if isinstance(value, bool) or not isinstance(value, (int, float)):
            raise ValueError(f"{name} must be a nonnegative finite number or null")
        value = float(value)
        if not math.isfinite(value) or value < 0:
            raise ValueError(f"{name} must be a nonnegative finite number or null")
        values[name] = value
    return values


def summarize_attempts(attempts: Sequence[Mapping[str, Any]], *,
                       max_attempts: int = MAX_ATTEMPTS,
                       max_groups: int = MAX_ATTEMPT_GROUPS) -> dict[str, Any]:
    """Group attempts only when source, toolchain, and hypothesis fingerprints all agree."""
    if (not isinstance(attempts, (list, tuple)) or type(max_attempts) is not int or max_attempts < 0
            or type(max_groups) is not int or max_groups < 1 or len(attempts) > max_attempts):
        raise ValueError("attempt history exceeds its explicit bounds")
    grouped: dict[tuple[str, str, str], Counter[str]] = {}
    for attempt in attempts:
        attempt = _mapping(attempt, "attempt")
        key = tuple(_fingerprint(attempt.get(field), field) for field in
                    ("source_fingerprint", "toolchain_fingerprint", "hypothesis_fingerprint"))
        outcome = _text(attempt.get("outcome"), "attempt outcome")
        counts = grouped.setdefault(key, Counter())
        counts[outcome] += 1
        if len(grouped) > max_groups:
            raise ValueError("attempt history has too many distinct fingerprint groups")
    groups = []
    for (source_fp, toolchain_fp, hypothesis_fp), outcomes in sorted(grouped.items()):
        count = sum(outcomes.values())
        groups.append({"source_fingerprint": source_fp,
                       "toolchain_fingerprint": toolchain_fp,
                       "hypothesis_fingerprint": hypothesis_fp,
                       "attempt_count": count,
                       "repeat_attempt_count": max(0, count - 1),
                       "outcomes": dict(sorted(outcomes.items()))})
    return {"attempt_count": len(attempts), "fingerprint_group_count": len(groups),
            "groups": groups,
            "grouping_basis": ["source_fingerprint", "toolchain_fingerprint", "hypothesis_fingerprint"]}


def throughput_report(original_identities: Sequence[Mapping[str, Any]],
                      before: Mapping[str, Any], after: Mapping[str, Any], *,
                      implementation_before: Mapping[str, str] | None = None,
                      implementation_after: Mapping[str, str] | None = None,
                      timings: Mapping[str, Any] | None = None,
                      attempts: Sequence[Mapping[str, Any]] = ()) -> dict[str, Any]:
    """Summarize exact-function movement and implementation coverage descriptively.

    Inputs are explicit measurement receipts, not historical claim markers. Each
    observation's source fingerprint must match its containing batch. Cross-batch
    exactness movement is compared only when original image and toolchain
    fingerprints are identical. Missing rows remain in the original denominator.
    """
    inventory = _identity_inventory(original_identities)
    old = _batch(before, "before", inventory)
    new = _batch(after, "after", inventory)
    old_impl = _implementation_statuses(implementation_before, inventory, "before")
    new_impl = _implementation_statuses(implementation_after, inventory, "after")
    times = _timings(timings)
    attempt_summary = summarize_attempts(attempts)
    batch_compatible = (old["original_image_fingerprint"] == new["original_image_fingerprint"]
                        and old["toolchain_fingerprint"] == new["toolchain_fingerprint"])

    buckets: Counter[str] = Counter()
    row_statuses: dict[str, str] = {}
    newly_strict_exact: list[str] = []
    newly_masked_exact: list[str] = []
    strict_to_masked: list[str] = []
    lost_exact: list[str] = []
    baseline_strict_exact: list[str] = []
    current_strict_exact: list[str] = []
    baseline_masked_exact: list[str] = []
    current_masked_exact: list[str] = []
    current_genuine_exact: list[str] = []
    stale: list[str] = []
    unknown: list[str] = []
    noncomparable: list[str] = []
    missing: list[str] = []

    for identity in inventory:
        old_row = old["observations"].get(identity)
        new_row = new["observations"].get(identity)
        old_result = old_row["result"] if old_row else "missing"
        new_result = new_row["result"] if new_row else "missing"
        if old_result == "strict_exact":
            baseline_strict_exact.append(identity)
        if old_result == "masked_exact":
            baseline_masked_exact.append(identity)
        if new_result == "strict_exact":
            current_strict_exact.append(identity)
            if new_impl[identity] == "genuine":
                current_genuine_exact.append(identity)
        if new_result == "masked_exact":
            current_masked_exact.append(identity)

        if old_result == "stale" or new_result == "stale":
            status = "stale"
            stale.append(identity)
        elif old_result in ("unknown", "missing") or new_result in ("unknown", "missing"):
            status = "unknown" if "unknown" in (old_result, new_result) else "missing"
            (unknown if status == "unknown" else missing).append(identity)
        elif old_result == "noncomparable" or new_result == "noncomparable" or not batch_compatible:
            status = "noncomparable"
            noncomparable.append(identity)
        else:
            status = "comparable"
            if old_result != "strict_exact" and new_result == "strict_exact" and new_impl[identity] == "genuine":
                newly_strict_exact.append(identity)
            if old_result == "strict_exact" and new_result == "masked_exact":
                strict_to_masked.append(identity)
            elif old_result in ("mismatch", "measured_nonexact") and new_result == "masked_exact" and new_impl[identity] == "genuine":
                newly_masked_exact.append(identity)
            if old_result == "strict_exact" and new_result in ("mismatch", "measured_nonexact", "masked_exact"):
                lost_exact.append(identity)
        row_statuses[identity] = status
        buckets[status] += 1

    implementation_summary = {}
    for label, statuses in (("before", old_impl), ("after", new_impl)):
        counts = Counter(statuses.values())
        scaffold_ids = sorted(identity for identity, status in statuses.items() if status == "scaffold")
        genuine = counts["genuine"]
        implementation_summary[label] = {
            "genuine_implemented": genuine,
            "all_original_identity_denominator": len(inventory),
            "coverage_fraction_of_all_original_identities": genuine / len(inventory) if inventory else None,
            "status_counts": {status: counts[status] for status in sorted(_IMPLEMENTATIONS)},
            "scaffolds_remaining": {"count": len(scaffold_ids), "identities": scaffold_ids},
        }
    implementation_summary["change"] = {
        "genuine_implemented_delta": (implementation_summary["after"]["genuine_implemented"]
                                       - implementation_summary["before"]["genuine_implemented"]),
        "coverage_fraction_of_all_original_identities_delta": (
            None if implementation_summary["before"]["coverage_fraction_of_all_original_identities"] is None
            or implementation_summary["after"]["coverage_fraction_of_all_original_identities"] is None
            else implementation_summary["after"]["coverage_fraction_of_all_original_identities"]
            - implementation_summary["before"]["coverage_fraction_of_all_original_identities"]),
    }

    rate_denominators = {"worker_elapsed_seconds": times["worker_elapsed_seconds"],
                         "wall_elapsed_seconds": times["wall_elapsed_seconds"]}
    rates = {}
    for key, seconds in rate_denominators.items():
        rates[key.replace("_elapsed_seconds", "_seconds")] = (
            len(newly_strict_exact) / seconds if seconds is not None and seconds > 0 else None)

    fingerprint_payload = {
        "original_identities": [{"identity": identity, "original_fingerprint": fp}
                                for identity, fp in inventory.items()],
        "before": {**{key: old[key] for key in ("source_fingerprint", "original_image_fingerprint", "toolchain_fingerprint",
                                                   "receipt_fingerprint", "verification")},
                   "observations": old["observations"], "implementation": old_impl},
        "after": {**{key: new[key] for key in ("source_fingerprint", "original_image_fingerprint", "toolchain_fingerprint",
                                                  "receipt_fingerprint", "verification")},
                  "observations": new["observations"], "implementation": new_impl},
        "timings": times,
        "attempts": attempt_summary,
    }
    return {
        "schema_version": SCHEMA_VERSION,
        "operation": "recovery_throughput_observation",
        "authority": "descriptive_only_not_acceptance_or_recovery_proof",
        "input_fingerprint": digest(fingerprint_payload),
        "denominator": {"original_identity_count": len(inventory),
                        "original_identities": list(inventory)},
        "comparison": {"before_source_fingerprint": old["source_fingerprint"],
                       "after_source_fingerprint": new["source_fingerprint"],
                       "original_image_comparable": old["original_image_fingerprint"] == new["original_image_fingerprint"],
                       "toolchain_comparable": old["toolchain_fingerprint"] == new["toolchain_fingerprint"],
                       "batches_comparable": batch_compatible,
                       "row_counts": {key: buckets[key] for key in
                                      ("comparable", "unknown", "stale", "noncomparable", "missing")},
                       "row_statuses": row_statuses,
                       "unknown_identities": sorted(unknown),
                       "stale_identities": sorted(stale),
                       "noncomparable_identities": sorted(noncomparable),
                       "missing_measurement_identities": sorted(missing)},
        "measurements": {"baseline_strict_exact_count": len(baseline_strict_exact),
                         "baseline_strict_exact_identities": sorted(baseline_strict_exact),
                         "current_strict_exact_count": len(current_strict_exact),
                         "current_strict_exact_identities": sorted(current_strict_exact),
                         "current_strict_exact_genuine_count": len(current_genuine_exact),
                         "current_strict_exact_genuine_identities": sorted(current_genuine_exact),
                         "newly_verified_exact_genuine_count": len(newly_strict_exact),
                         "newly_verified_exact_genuine_identities": sorted(newly_strict_exact),
                         "baseline_masked_exact_count": len(baseline_masked_exact),
                         "baseline_masked_exact_identities": sorted(baseline_masked_exact),
                         "current_masked_exact_count": len(current_masked_exact),
                         "current_masked_exact_identities": sorted(current_masked_exact),
                         "newly_measured_masked_exact_genuine_count": len(newly_masked_exact),
                         "newly_measured_masked_exact_genuine_identities": sorted(newly_masked_exact),
                         "strict_to_masked_exact_identities": sorted(strict_to_masked),
                         "lost_exact_count": len(lost_exact),
                         "lost_exact_identities": sorted(lost_exact)},
        "implementation_coverage": implementation_summary,
        "timing": times,
        "rates": {"newly_verified_exact_genuine_per_worker_second": rates["worker_seconds"],
                  "newly_verified_exact_genuine_per_wall_second": rates["wall_seconds"],
                  "newly_measured_masked_exact_genuine_per_worker_second": (
                      len(newly_masked_exact) / times["worker_elapsed_seconds"]
                      if times["worker_elapsed_seconds"] not in (None, 0) else None),
                  "newly_measured_masked_exact_genuine_per_wall_second": (
                      len(newly_masked_exact) / times["wall_elapsed_seconds"]
                      if times["wall_elapsed_seconds"] not in (None, 0) else None),
                  "basis": "explicit elapsed seconds only; compile time is separate and is not substituted"},
        "repeat_attempts": attempt_summary,
        "persisted": False,
        "ready_for_collection": False,
        "limitations": [
            "descriptive throughput is not acceptance, semantic correctness, or game recovery proof",
            "historical BYTE_EXACT claims are not current measurements and are not consumed",
            "missing per-function rows remain in the original-identity denominator as missing",
            "stale source fingerprints and changed image/toolchain fingerprints refuse exactness movement",
            "scaffolds remain unresolved identities in the implementation denominator and are listed explicitly; they are not genuine-function numerators",
            "strict-to-masked transitions are reported as strict-evidence losses, not newly measured masked gains",
            "rates are omitted when the corresponding explicit elapsed time is missing or zero",
            "repeat attempts coalesce only on identical source, toolchain, and hypothesis fingerprints",
        ],
    }
