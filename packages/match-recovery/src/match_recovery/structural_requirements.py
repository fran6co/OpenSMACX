"""Join fresh native measurements to source/history structural interpretations.

Individual declared requirements can be measured without asserting complete
hierarchy, ownership or lifetime. Historical interpretations are review inputs,
not interchangeable spellings from which to select a convenient passing check.
"""

from collections import Counter, defaultdict

from .model import Result, require_digest
from .reviews import merge_interpretations, pending_interpretations


def _all(values):
    values = tuple(values)
    return (Result.UNKNOWN if not values or Result.UNKNOWN in values else
            Result.FAIL if Result.FAIL in values else Result.PASS)


def _grid(report):
    """Missing coverage metadata is a gap, never permission to use fewer trials."""
    units, flags = report.get("translation_units"), report.get("flag_sets")
    if units is None or flags is None:
        return (), (), "native coverage metadata is unavailable"
    if (not isinstance(units, list) or any(not isinstance(unit, str) or not unit for unit in units)
            or len(units) != len(set(units)) or not isinstance(flags, list)
            or any(not isinstance(trial, list) or any(not isinstance(word, str) for word in trial) for trial in flags)):
        raise ValueError("invalid structural measurement coverage")
    flag_sets = tuple(tuple(trial) for trial in flags)
    if len(flag_sets) != len(set(flag_sets)):
        raise ValueError("duplicate structural compiler trial")
    return tuple(units), flag_sets, None if units and flags else "no scoped compiler witnesses were requested"


def check_requirements(inventory, candidate):
    """Consume measurements made by the same fresh check, without recompilation.

    This is not a command-adapter attestation or independent acceptance verifier.
    The caller binds source/history and the candidate report to one snapshot.
    """
    known = {}
    merge_interpretations(known, inventory.get("interpretations", []), frozenset(inventory["scope"]))
    checks = candidate["checks"]
    reports = {"layout": checks.get("layout", {}), "binary": checks.get("binary", {})}
    grids = {name: _grid(report) for name, report in reports.items()}
    observations = defaultdict(dict)
    for backend, collection, kind in (("layout", "members", "direct_member_offset"),
                                       ("layout", "bases", "nonvirtual_base_offset"),
                                       ("binary", "generated_tables", "compiler_generated_vtable"),
                                       ("binary", "object_definitions", "object_initial_data")):
        for row in reports[backend].get(collection, []):
            key = row.get("interpretation_id")
            if key is None:
                continue  # Older/partial reports cannot attest an exact interpretation.
            require_digest(key)
            if key not in known or not known[key]["current"]:
                continue  # Other source requirements, or stale historical measurements.
            if known[key]["value"].get("kind") != kind:
                raise ValueError("structural measurement kind disagrees with source interpretation")
            unit, flags = row.get("unit"), row.get("flags")
            if (not isinstance(unit, str) or not unit or not isinstance(flags, list)
                    or any(not isinstance(flag, str) for flag in flags)):
                raise ValueError("structural measurement has no compiler witness identity")
            witness = (unit, tuple(flags))
            if witness in observations[key]:
                raise ValueError("ambiguous structural measurement for a compiler witness")
            if kind in ("compiler_generated_vtable", "object_initial_data"):
                result = _all((Result(row.get("definition", Result.UNKNOWN)),
                               Result(row.get("content", Result.UNKNOWN))))
                reason = row.get("reason", row.get("proof", {}).get("reason", "table definition and bounded contents compared"))
                source = row.get("source")
            else:
                result = Result(row["result"])
                reason = row.get("reason", "declared layout requirement compared")
                expectation = row.get("expectation", {})
                source = {name: expectation[name] for name in ("path", "line") if name in expectation}
            observations[key][witness] = {"unit": unit, "flags": flags, "result": result,
                                          "reason": reason, "source": source}
    requirements = []
    for key, item in sorted(known.items()):
        kind = item["value"].get("kind")
        backend = {"direct_member_offset": "layout", "nonvirtual_base_offset": "layout",
                   "compiler_generated_vtable": "binary", "object_initial_data": "binary"}.get(kind)
        row = {"interpretation_id": key, "entity_id": item["entity_id"], "kind": kind,
               "current": item["current"], "requirement": item["value"], "result": Result.UNKNOWN,
               "counts": {state.value: 0 for state in Result}, "trials": []}
        if not item["current"]:
            row.update(next_action="review_supersession_or_retraction",
                       reason="historical interpretation requires explicit snapshot review; current measurements do not replace it")
        elif backend is None:
            row.update(next_action="implement_requirement_verifier", reason="no native verifier for this interpretation kind")
        else:
            units, flag_sets, gap = grids[backend]
            observed = observations[key]
            if kind == "object_initial_data":
                owners = reports[backend].get("object_owners", [])
                if not isinstance(owners, list) or any(not isinstance(owner, dict) or set(owner) != {"interpretation_id", "unit"}
                        or not isinstance(owner["unit"], str) or not owner["unit"] for owner in owners):
                    raise ValueError("invalid object owner coverage")
                units = [owner["unit"] for owner in owners if owner["interpretation_id"] == key]
                if len(units) != 1:
                    gap = "requires one source-owned initial-data measurement host"
            # Include any extra compiled dependency witnesses as well. A bad
            # definition there must not disappear behind a passing scoped TU.
            units = sorted(set(units) | {unit for unit, _ in observed})
            wanted = {(unit, flags) for unit in units for flags in flag_sets}
            for witness in sorted(wanted | observed.keys()):
                trial = observed.get(witness)
                if trial is None:
                    trial = {"unit": witness[0], "flags": list(witness[1]), "result": Result.UNKNOWN,
                             "reason": "no measurement of this interpretation in the requested TU/trial", "source": None}
                elif witness[1] not in flag_sets:
                    trial = {**trial, "result": Result.UNKNOWN, "reason": "measurement uses an unrequested compiler trial"}
                row["trials"].append(trial)
            row["counts"].update(Counter(trial["result"] for trial in row["trials"]))
            result = Result.UNKNOWN if gap else _all(trial["result"] for trial in row["trials"])
            row.update(result=result, reason=gap or "all requested TU/trial witnesses compared independently",
                       next_action=("repair_source_or_review_interpretation" if row["counts"]["fail"] else
                                    "establish_measurement_coverage" if result == Result.UNKNOWN else
                                    "verify_remaining_structure_and_lifetime"))
        requirements.append(row)
    return requirements


def structural_outcome(requirements):
    """A declared counterexample refutes a model; passing parts do not prove it."""
    failed = [row for row in requirements if row["current"] and row["counts"]["fail"]]
    if failed:
        return Result.FAIL, "current source interpretations have measured counterexamples; inspect structural_requirements"
    return Result.UNKNOWN, "declared requirements do not prove complete structure or lifetime; inspect structural_requirements and historical review needs"


def declared_outcome(requirements, interpretations, *, review=None):
    """Prove only bounded declarations, with explicit historical dispositions.

    The caller must bind an optional independent review to its current input
    snapshot. Review never supplies a missing compiler witness, waives a current
    counterexample, or establishes complete structure/lifetime. Removing every
    requirement remains unknown even if a reviewer retracts the old model.
    """
    rows = tuple(requirements)
    current = [row for row in rows if row["current"]]
    if any(row["counts"]["fail"] for row in current):
        return Result.FAIL, "current declared requirements have measured counterexamples"
    if not current or any(row["result"] != Result.PASS for row in current):
        return Result.UNKNOWN, "current declared requirements lack complete compiler witnesses"
    historical = {row["interpretation_id"] for row in rows if not row["current"]}
    if historical:
        if review is None or review.get("decision") != "approve":
            return Result.UNKNOWN, "historical interpretations require independent snapshot review"
        if historical.intersection(pending_interpretations(review, interpretations)):
            return Result.UNKNOWN, "historical interpretations lack explicit reviewed dispositions"
    return Result.PASS, "all current bounded declarations measured; historical dispositions checked; not full structure/lifetime proof"
