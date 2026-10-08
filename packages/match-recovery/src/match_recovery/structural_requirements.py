"""Join fresh native measurements to source/history structural interpretations.

Individual declared requirements can be measured without asserting complete
hierarchy, ownership or lifetime. Historical interpretations are review inputs,
not interchangeable spellings from which to select a convenient passing check.
"""

from collections import Counter, defaultdict

from .model import Result, require_digest
from .reviews import merge_interpretations, pending_interpretations
from .layout_witness import declaration_absent
from .generated_binary import definition_absent


def _all(values):
    values = tuple(values)
    return (Result.UNKNOWN if not values or Result.UNKNOWN in values else
            Result.FAIL if Result.FAIL in values else Result.PASS)


def _aggregate_graph_results(values):
    """Aggregate one physical TU/profile observed under multiple graph trials."""
    values = tuple(values)
    return (Result.FAIL if Result.FAIL in values else
            Result.UNKNOWN if not values or Result.UNKNOWN in values else Result.PASS)


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
    suffix = report.get("layout_flags", [])
    if not isinstance(suffix, list) or any(not isinstance(flag, str) for flag in suffix):
        raise ValueError("invalid common compiler flag suffix")
    canonical_trials = set(flag_sets) | {trial + tuple(suffix) for trial in flag_sets}
    unit_flags = report.get("unit_flag_sets")
    if unit_flags is None:
        pairs = tuple((unit, trial) for unit in units for trial in flag_sets)
    else:
        if not isinstance(unit_flags, dict) or set(unit_flags) != set(units):
            raise ValueError("per-translation-unit compiler coverage must exactly match declared units")
        pairs = []
        for unit in units:
            trials = unit_flags[unit]
            if (not isinstance(trials, list) or not trials
                    or any(not isinstance(trial, list) or any(not isinstance(word, str) for word in trial)
                           for trial in trials)):
                raise ValueError("invalid per-translation-unit compiler trials")
            canonical = tuple(tuple(trial) for trial in trials)
            if len(canonical) != len(set(canonical)):
                raise ValueError("duplicate per-translation-unit compiler trial")
            if any(trial not in canonical_trials for trial in canonical):
                raise ValueError("per-translation-unit trial is not a configured compiler profile")
            pairs.extend((unit, trial) for trial in canonical)
    return tuple(units), tuple(pairs), None if units and pairs else "no scoped compiler witnesses were requested"


def _graph_trial_grid(report, units, pairs, *, required=False):
    """Validate logical graph-context receipts separately from physical profiles."""
    raw = report.get("unit_graph_trials")
    if raw is None:
        if required:
            raise ValueError("per-unit binary profile policy requires logical graph-trial metadata")
        return None
    if not isinstance(raw, dict) or set(raw) != set(units):
        raise ValueError("per-unit graph-trial receipts must exactly match declared units")
    declared_graphs = tuple(tuple(flags) for flags in report.get("flag_sets", []))
    physical_pairs = set(pairs)
    result = {}
    for unit in units:
        rows = raw[unit]
        if not isinstance(rows, list) or not rows:
            raise ValueError("each translation unit needs graph-trial receipts")
        seen = set()
        for row in rows:
            if (not isinstance(row, dict) or set(row) != {"flags", "graph_flags"}
                    or not isinstance(row["flags"], list) or not isinstance(row["graph_flags"], list)
                    or any(not isinstance(flag, str) for flag in (*row["flags"], *row["graph_flags"]))):
                raise ValueError("invalid per-unit graph-trial receipt")
            identity = (tuple(row["flags"]), tuple(row["graph_flags"]))
            if identity in seen:
                raise ValueError("duplicate per-unit graph-trial receipt")
            if (unit, identity[0]) not in physical_pairs or identity[1] not in declared_graphs:
                raise ValueError("per-unit graph trial is outside declared physical/logical compiler profiles")
            seen.add(identity)
        if {(unit, flags) for flags, _ in seen} != {(name, flags) for name, flags in physical_pairs if name == unit}:
            raise ValueError("per-unit graph receipts do not cover the declared physical compiler profiles")
        if {graph_flags for _, graph_flags in seen} != set(declared_graphs):
            raise ValueError("each translation unit must declare every logical graph compiler trial")
        result[unit] = rows
    return result


def _graph_receipt_grid(report, units, graph_grid):
    """Validate compiler-attempt receipts as a subset of the expected grid."""
    raw = report.get("unit_graph_receipts")
    if raw is None:
        raise ValueError("logical graph-trial metadata requires compiler outcome receipts")
    if not isinstance(raw, dict) or set(raw) != set(units):
        raise ValueError("per-unit graph receipts must exactly match declared units")
    result = {}
    for unit in units:
        rows = raw[unit]
        if not isinstance(rows, list):
            raise ValueError("invalid per-unit compiler graph receipts")
        expected = {(tuple(row["flags"]), tuple(row["graph_flags"]))
                    for row in graph_grid[unit]}
        seen = {}
        for row in rows:
            if (not isinstance(row, dict) or set(row) != {"flags", "graph_flags", "outcome"}
                    or not isinstance(row["flags"], list) or not isinstance(row["graph_flags"], list)
                    or row["outcome"] not in ("compiled", "failed")
                    or any(not isinstance(flag, str) for flag in (*row["flags"], *row["graph_flags"]))):
                raise ValueError("invalid per-unit compiler graph receipt")
            identity = (tuple(row["flags"]), tuple(row["graph_flags"]))
            if identity in seen or identity not in expected:
                raise ValueError("compiler graph receipt is duplicated or outside its expected profile")
            seen[identity] = row["outcome"]
        result[unit] = seen
    return result


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
    binary_graph_required = ("unit_flag_sets" in reports["binary"] and any(
        item["current"] and item["value"].get("kind") in {
            "compiler_generated_vtable", "object_initial_data", "tu_private_literal", "shared_function_body"}
        for item in known.values()))
    graph_grids = {
        name: _graph_trial_grid(report, grids[name][0], grids[name][1],
                                required=(name == "binary" and binary_graph_required))
        for name, report in reports.items()}
    graph_receipts = {name: _graph_receipt_grid(report, grids[name][0], graph_grids[name])
                      if graph_grids[name] is not None else None
                      for name, report in reports.items()}

    def require_graph_context(backend, unit, flags, graph_flags):
        graph_grid = graph_grids[backend]
        if graph_grid is None:
            return
        if not isinstance(graph_flags, list) or any(not isinstance(flag, str) for flag in graph_flags):
            raise ValueError("structural observation lacks its logical graph compiler trial")
        expected = {(tuple(context["flags"]), tuple(context["graph_flags"]))
                    for context in graph_grid.get(unit, [])}
        identity = (tuple(flags), tuple(graph_flags))
        if identity not in expected:
            raise ValueError("structural observation is outside its declared TU/profile/graph context")
        receipts = graph_receipts[backend]
        if receipts is not None and receipts.get(unit, {}).get(identity) != "compiled":
            raise ValueError("structural observation has no successful matching compiler graph receipt")
    generated_report = {"translation_units": reports["binary"].get("generated_units"),
                        "flag_sets": reports["binary"].get("flag_sets")}
    if "unit_flag_sets" in reports["binary"]:
        generated_report["unit_flag_sets"] = reports["binary"]["unit_flag_sets"]
    if "layout_flags" in reports["binary"]:
        generated_report["layout_flags"] = reports["binary"]["layout_flags"]
    generated_grid = (_grid(generated_report)
                      if "generated_coverage" in reports["binary"] else grids["binary"])
    absences = {}
    for row in reports["layout"].get("coverage", []):
        if row.get("observation") not in ("no_complete_definition", "not_emitted"):
            continue
        name, unit, flags, witness = (row.get(key) for key in ("class_name", "unit", "flags", "witness"))
        units, trials, _ = grids["layout"]
        if (not isinstance(name, str) or not name or unit not in units or not isinstance(flags, list)
                or any(not isinstance(flag, str) for flag in flags) or (unit, tuple(flags)) not in trials
                or not declaration_absent(witness) or witness.get("class_name") != name):
            raise ValueError("invalid compiler declaration-absence coverage")
        require_graph_context("layout", unit, flags, row.get("graph_flags"))
        key = (name, unit, tuple(flags))
        if key in absences:
            raise ValueError("ambiguous compiler declaration-absence coverage")
        absences[key] = row
    generated_absences = defaultdict(dict)
    for row in reports["binary"].get("generated_coverage", []):
        key = row.get("interpretation_id")
        require_digest(key)
        if key not in known or not known[key]["current"]:
            continue
        item = known[key]["value"]
        unit, flags, observation, witness = (row.get(name) for name in ("unit", "flags", "observation", "witness"))
        generated_units, generated_trials, coverage_gap = generated_grid
        if (item.get("kind") != "compiler_generated_vtable" or not isinstance(unit, str) or not unit
                or not isinstance(flags, list) or any(not isinstance(flag, str) for flag in flags)
                or (not coverage_gap and (unit, tuple(flags)) not in generated_trials)
                or (coverage_gap and tuple(flags) not in tuple(tuple(value) for value in reports["binary"].get("flag_sets", [])))
                or observation not in ("not_emitted", "unsupported_symbol")
                or (not coverage_gap and unit not in generated_units)
                or not isinstance(witness, dict) or witness.get("symbol") != item.get("symbol")
                or (observation == "not_emitted" and not definition_absent(witness))):
            raise ValueError("invalid generated definition-absence coverage")
        require_graph_context("binary", unit, flags, row.get("graph_flags"))
        identity = (unit, tuple(flags))
        previous = generated_absences[key].get(identity)
        if previous is not None:
            if (previous.get("observation"), previous.get("witness")) != (observation, witness):
                raise ValueError("conflicting generated definition-absence coverage for one effective TU profile")
            graph_flags = row.get("graph_flags", flags)
            if graph_flags in previous.get("graph_flags", []):
                raise ValueError("duplicate generated definition-absence graph witness")
            previous.setdefault("graph_flags", []).append(graph_flags)
            continue
        generated_absences[key][identity] = {**row, "graph_flags": [row.get("graph_flags", flags)]}
    observations = defaultdict(dict)
    for backend, collection, kind in (("layout", "members", "direct_member_offset"),
                                       ("layout", "bases", "nonvirtual_base_offset"),
                                       ("binary", "generated_tables", "compiler_generated_vtable"),
                                       ("binary", "object_definitions", "object_initial_data"),
                                       ("binary", "literal_definitions", "tu_private_literal"),
                                       ("binary", "shared_functions", "shared_function_body")):
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
            require_graph_context(backend, unit, flags, row.get("graph_flags"))
            witness = (unit, tuple(flags))
            if kind in ("compiler_generated_vtable", "object_initial_data", "tu_private_literal"):
                result = _all((Result(row.get("definition", Result.UNKNOWN)),
                               Result(row.get("content", Result.UNKNOWN))))
                reason = row.get("reason", row.get("proof", {}).get("reason", "table definition and bounded contents compared"))
                source = row.get("source")
            elif kind == "shared_function_body":
                result = Result(row["result"])
                reason = row.get("reason", "shared body bytes, extent and references compared independently")
                source = row.get("source")
            else:
                result = Result(row["result"])
                reason = row.get("reason", "declared layout requirement compared")
                expectation = row.get("expectation", {})
                source = {name: expectation[name] for name in ("path", "line") if name in expectation}
            previous = observations[key].get(witness)
            if previous is not None:
                if (previous["source"], previous.get("layout_basis")) != (source, row.get("layout_basis")):
                    raise ValueError("conflicting structural measurement identity for one effective compiler witness")
                graph_flags = row.get("graph_flags", flags)
                if graph_flags in [context["flags"] for context in previous.get("graph_contexts", [])]:
                    raise ValueError("duplicate structural graph witness for one effective compiler profile")
                previous["result"] = _aggregate_graph_results((previous["result"], result))
                previous.setdefault("graph_contexts", []).append(
                    {"flags": graph_flags, "result": result, "reason": reason})
                previous["reason"] = "same effective TU/profile observed in multiple graph contexts"
            else:
                observations[key][witness] = {"unit": unit, "flags": flags, "result": result,
                                              "reason": reason, "source": source,
                                              "graph_contexts": [{"flags": row.get("graph_flags", flags),
                                                                  "result": result, "reason": reason}],
                                              **({"layout_basis": row["layout_basis"]} if "layout_basis" in row else {})}
    requirements = []
    for key, item in sorted(known.items()):
        kind = item["value"].get("kind")
        backend = {"direct_member_offset": "layout", "nonvirtual_base_offset": "layout",
                   "compiler_generated_vtable": "binary", "object_initial_data": "binary",
                   "tu_private_literal": "binary",
                   "shared_function_body": "binary"}.get(kind)
        row = {"interpretation_id": key, "entity_id": item["entity_id"], "kind": kind,
               "current": item["current"], "requirement": item["value"], "result": Result.UNKNOWN,
               "counts": {state.value: 0 for state in Result}, "trials": []}
        if not item["current"]:
            row.update(next_action="review_supersession_or_retraction",
                       reason="historical interpretation requires explicit snapshot review; current measurements do not replace it")
        elif backend is None:
            row.update(next_action="implement_requirement_verifier", reason="no native verifier for this interpretation kind")
        else:
            units, expected_pairs, gap = grids[backend]
            observed = observations[key]
            if kind == "compiler_generated_vtable":
                generated_units, _, generated_gap = generated_grid
                units = sorted(set(units) | set(generated_units))
                gap = gap or generated_gap
            if kind in ("object_initial_data", "shared_function_body", "tu_private_literal"):
                wanted = set(expected_pairs)
                owner_collection = {"object_initial_data": "object_owners", "shared_function_body": "shared_owners",
                                    "tu_private_literal": "literal_owners"}[kind]
                owners = reports[backend].get(owner_collection, [])
                if not isinstance(owners, list) or any(not isinstance(owner, dict) or set(owner) != {"interpretation_id", "unit"}
                        or not isinstance(owner["unit"], str) or not owner["unit"] for owner in owners):
                    raise ValueError("invalid source owner coverage")
                units = [owner["unit"] for owner in owners if owner["interpretation_id"] == key]
                if len(units) != 1:
                    gap = "requires one source-owned measurement host"
                elif kind == "shared_function_body" and units != [item["value"].get("unit")]:
                    gap = "shared body measurement host disagrees with source interpretation"
                if kind == "tu_private_literal":
                    owner = item["value"].get("unit")
                    if units != [owner] or any(unit != owner for unit, _ in observed):
                        gap = "literal measurements require the exact source-owned translation unit"
                    # Private symbols are meaningful only inside this TU. A
                    # matching symbol in another object cannot fill its trials.
                    units = [owner]
                owner_unit = units[0] if len(units) == 1 else None
                if owner_unit is not None:
                    if "unit_flag_sets" in reports[backend]:
                        wanted = {(unit, flags) for unit, flags in expected_pairs if unit == owner_unit}
                        if not wanted:
                            gap = gap or "source owner has no declared compiler trial receipts"
                    else:
                        legacy_trials = tuple(tuple(flags) for flags in reports[backend].get("flag_sets", []))
                        wanted = {(owner_unit, flags) for flags in legacy_trials}
                    # Dependency observations are never discarded; an extra
                    # owner/profile or conflicting dependency can still refute.
                    wanted.update(observed.keys())
            else:
                wanted = set(expected_pairs)
            # Include any extra compiled dependency witnesses as well. A bad
            # definition there must not disappear behind a passing scoped TU.
            table_coverage = generated_absences[key]
            units = sorted(set(units) | {unit for unit, _ in observed} | {unit for unit, _ in table_coverage})
            if "unit_flag_sets" not in reports[backend]:
                legacy_trials = tuple(tuple(flags) for flags in reports[backend].get("flag_sets", []))
                if kind not in ("object_initial_data", "shared_function_body", "tu_private_literal"):
                    wanted.update((unit, flags) for unit in units for flags in legacy_trials)
            for witness in sorted(wanted | observed.keys()):
                trial = observed.get(witness)
                absence = absences.get((item["value"].get("class_name"), *witness)) if backend == "layout" else None
                if absence is not None and trial is not None:
                    raise ValueError("class declaration is both measured and reported absent in one TU/trial")
                table_absence = table_coverage.get(witness)
                if table_absence is not None and trial is not None:
                    raise ValueError("generated definition is both measured and reported absent in one TU/trial")
                if table_absence is not None:
                    absent = table_absence["observation"] == "not_emitted"
                    trial = {"unit": witness[0], "flags": list(witness[1]),
                             "result": Result.PASS if absent else Result.UNKNOWN,
                             "reason": ("compiled object has no definition of this table; not table-content proof" if absent
                                        else "unsupported generated symbol requires definition/linker analysis"),
                             "source": None, "coverage_only": True, "emission_witness": table_absence["witness"],
                             "graph_flags": table_absence.get("graph_flags", [])}
                elif absence is not None:
                    trial = {"unit": witness[0], "flags": list(witness[1]), "result": Result.PASS,
                             "reason": "controlled absence of a complete declaration; no member/base layout to compare in this TU",
                             "source": None, "coverage_only": True, "layout_basis": "same_tu_definition_absence",
                             "graph_contexts": [{"flags": absence.get("graph_flags", list(witness[1])),
                                                 "result": Result.PASS,
                                                 "reason": "controlled declaration absence in this graph context"}]}
                elif trial is None:
                    trial = {"unit": witness[0], "flags": list(witness[1]), "result": Result.UNKNOWN,
                             "reason": "no measurement of this interpretation in the requested TU/trial", "source": None}
                elif witness not in wanted:
                    trial = {**trial, "result": Result.UNKNOWN, "reason": "measurement uses an unrequested compiler trial"}
                else:
                    trial = {**trial, "graph_contexts": trial.get("graph_contexts", [])}
                graph_grid = graph_grids[backend]
                expected_graphs = ({tuple(context["graph_flags"]) for context in graph_grid.get(witness[0], [])
                                    if tuple(context["flags"]) == witness[1]}
                                   if graph_grid is not None else set())
                if expected_graphs:
                    actual_contexts = list(trial.get("graph_contexts", []))
                    if table_absence is not None and witness in table_coverage:
                        actual_contexts = [{"flags": flags, "result": trial["result"],
                                            "reason": trial["reason"]}
                                           for flags in table_absence.get("graph_flags", [])]
                    actual_graphs = {tuple(context["flags"]) for context in actual_contexts}
                    missing_graphs = sorted(expected_graphs - actual_graphs)
                    if missing_graphs:
                        trial = {**trial,
                                 "result": _aggregate_graph_results((trial["result"], Result.UNKNOWN)),
                                 "reason": "one or more expected logical graph contexts lack a structural witness",
                                 "graph_contexts": actual_contexts + [
                                     {"flags": list(flags), "result": Result.UNKNOWN,
                                      "reason": "no structural measurement in this expected graph context"}
                                     for flags in missing_graphs]}
                row["trials"].append(trial)
            row["counts"].update(Counter(trial["result"] for trial in row["trials"]))
            if backend == "layout" and any(
                    not any(tuple(trial["flags"]) == flags and not trial.get("coverage_only")
                            and (trial["unit"], flags) in observed
                            for trial in row["trials"])
                    for flags in {flags for _, flags in expected_pairs}):
                gap = gap or "each declared requirement needs an actual layout witness in every configured trial; absence alone is insufficient"
            if kind == "compiler_generated_vtable" and any(
                    not any(trial["flags"] == list(flags)
                            and not trial.get("coverage_only") and (trial["unit"], flags) in observed
                            for trial in row["trials"])
                    for flags in {flags for _, flags in expected_pairs}):
                gap = gap or "each generated table needs an actual definition witness in every configured trial; non-emission alone is insufficient"
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
