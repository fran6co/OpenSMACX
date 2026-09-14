"""Complete native final-pass candidate families, not retained-section proof.

An unchanged family inventory is weaker than unique member selection, COMDAT
selection, storage isolation or acceptance. Ambiguous import families remain
explicit; unrelated members selected outside initializer reachability are kept.
"""

from collections import Counter
from copy import deepcopy
from pathlib import Path

from .components import relative_path
from .linked_code_ownership import _portable, _contents
from .linked_input_epoch import input_epoch, _borrow_epoch
from .linked_objects import _owner
from .linked_storage_census import _candidates
from .model import Result, digest, require_digest


def _candidate(state, record):
    identity = {key: record[key] for key in ("path", "member", "occurrence", "generated") if key in record}
    compiled = [unit for unit, row in state.compiled.items() if identity == {"path": row["path"]}]
    if compiled:
        if len(compiled) != 1 or state.compiled[compiled[0]]["sha256"] != record["sha256"]:
            raise ValueError("input family compiled candidate differs from its exact native receipt")
        producer = {"kind": "compiled", "unit": compiled[0]}
    else:
        producer = {"kind": "dependency", "input": _portable(Path(identity["path"]), state.root, state.directory),
                    "object_sha256": record["sha256"],
                    **{key: identity[key] for key in ("member", "occurrence", "generated") if key in identity}}
    return {"producer": producer, "input": identity, "format": record["format"], "object_sha256": record["sha256"]}


def observe_input_selection(context, objects, *, epoch=None):
    """Observe all native families through fresh physical inputs, never a report.

    A shared authentic input epoch avoids reparsing the complete archive corpus.
    The returned diagnostic is defensive and grants no member/scope permission.
    """
    if epoch is None:
        with input_epoch(context, objects) as owned:
            return observe_input_selection(context, objects, epoch=owned)
    with _borrow_epoch(epoch, context, objects) as state:
        records = state.records
        if len(records) > 1000000 or len(state.coverage["inputs"]) > 1000000:
            raise ValueError("native input family observation exceeds its resource bound")
        inventory = [_candidate(state, record) for record in records]
        owners = Counter(_owner(row).casefold() for row in state.coverage["inputs"])
        families = []
        for index, row in enumerate(state.coverage["inputs"]):
            candidates = _candidates(row, records)
            if (not candidates or type(row["candidate_count"]) is not int
                    or len(candidates) != row["candidate_count"]
                    or set(row["paths"]) != {records[number]["path"] for number in candidates}
                    or row["candidate_coverage"] != Result.PASS):
                raise ValueError("native final-pass family differs from its complete physical candidates")
            owner = _owner(row)
            selection = (Result.PASS if row["selection"] == Result.PASS and len(candidates) == 1
                         and owners[owner.casefold()] == 1 else Result.UNKNOWN)
            if selection == Result.PASS:
                candidate = inventory[candidates[0]]
                if candidate["format"] == "short_import":
                    # Complete native family membership and physical bytes bind
                    # this import input, not its thunk, ABI or retained sections.
                    _contents(candidate["input"], candidate["object_sha256"], state.inputs)
                else:
                    actual = state.native.resolve(owner)
                    if actual["identity"] != candidate["input"] or actual["sha256"] != candidate["object_sha256"]:
                        raise ValueError("unique native family differs from independently selected physical input")
            families.append({"index": index, "label": row["label"], "owner": owner,
                             "selection": selection, "candidate_coverage": Result.PASS,
                             "candidates": [deepcopy(inventory[number]) for number in candidates]})
        ambiguous = [row["index"] for row in families if row["selection"] != Result.PASS]
        return {"check": "linked_native_input_selection", "result": Result.UNKNOWN,
                "candidate_coverage": Result.PASS, "input_consistency": Result.PASS,
                "families": families, "inventory": deepcopy(inventory), "ambiguous_families": ambiguous,
                "selected_input_identity": Result.UNKNOWN if ambiguous else Result.PASS,
                "policy_gaps": deepcopy(objects._constant_policy_gaps),
                "retained_section_selection": Result.UNKNOWN, "allocation_isolation": Result.UNKNOWN,
                "remaining_linked_effects": Result.UNKNOWN, "persisted": False, "ready_for_collection": False}


def _validate(report):
    if (not isinstance(report, dict) or report.get("check") != "linked_native_input_selection"
            or report.get("candidate_coverage") != Result.PASS or report.get("input_consistency") != Result.PASS
            or not isinstance(report.get("inventory"), list) or not report["inventory"]
            or not isinstance(report.get("families"), list) or not report["families"]
            or not isinstance(report.get("policy_gaps"), list)
            or len(report["inventory"]) > 1000000 or len(report["families"]) > 1000000):
        raise ValueError("selection comparison requires complete native candidate-family observations")
    candidates = {}
    for row in report["inventory"]:
        if not isinstance(row, dict) or set(row) != {"producer", "input", "format", "object_sha256"}:
            raise ValueError("invalid native input candidate observation")
        require_digest(row["object_sha256"])
        producer = row["producer"]
        if row["format"] not in ("coff", "short_import") or not isinstance(producer, dict):
            raise ValueError("unsupported native input candidate producer")
        if producer.get("kind") == "compiled":
            if set(producer) != {"kind", "unit"} or relative_path(producer["unit"]) != producer["unit"]:
                raise ValueError("invalid compiled input candidate identity")
        elif (producer.get("kind") != "dependency" or producer.get("object_sha256") != row["object_sha256"]
              or not isinstance(producer.get("input"), dict)):
            raise ValueError("invalid dependency input candidate identity")
        key = digest(row)
        if key in candidates:
            raise ValueError("duplicate native physical candidate observation")
        candidates[key] = row
    owners = Counter()
    for family in report["families"]:
        if not isinstance(family, dict) or not isinstance(family.get("owner"), str) or not family["owner"]:
            raise ValueError("invalid native family owner")
        owners[family["owner"].casefold()] += 1
    for index, family in enumerate(report["families"]):
        if (not isinstance(family, dict) or type(family.get("index")) is not int or family["index"] != index
                or family.get("candidate_coverage") != Result.PASS
                or family.get("selection") not in (Result.PASS, Result.UNKNOWN)
                or not isinstance(family.get("candidates"), list) or not family["candidates"]
                or any(digest(row) not in candidates for row in family["candidates"])
                or len({digest(row) for row in family["candidates"]}) != len(family["candidates"])
                or family["selection"] == Result.PASS and
                    (len(family["candidates"]) != 1 or owners[family["owner"].casefold()] != 1)):
            raise ValueError("invalid ordered native candidate family")
    ambiguous = report.get("ambiguous_families")
    if (not isinstance(ambiguous, list) or any(type(index) is not int for index in ambiguous)
            or ambiguous != [row["index"] for row in report["families"] if row["selection"] != Result.PASS]):
        raise ValueError("inconsistent native family ambiguity diagnostic")


def _difference(old, new):
    indexed = {digest(row): row for row in (*old, *new)}
    a, b = Counter(map(digest, old)), Counter(map(digest, new))
    return [{"change": change, "count": count, "family": deepcopy(indexed[key])}
            for change, counts in (("removed", a - b), ("added", b - a)) for key, count in sorted(counts.items())]


def compare_selection(old, new):
    """Pure diagnostic comparison; even equality supplies no native authority."""
    _validate(old)
    _validate(new)
    compiled, dependencies, inventory, sequences = {}, {}, {}, {}
    for version, report in (("baseline", old), ("current", new)):
        compiled[version] = {}
        inventory[version] = []
        for row in report["inventory"]:
            producer = row["producer"]
            if producer["kind"] == "compiled":
                unit = producer["unit"]
                if unit in compiled[version]:
                    raise ValueError("one source TU has multiple physical candidate records")
                compiled[version][unit] = {"object_sha256": row["object_sha256"], "format": row["format"]}
            else:
                inventory[version].append({"producer": producer, "format": row["format"]})
        # Compiled producer identity is source-qualified, not its mutable output
        # bytes. Full dependency identity pins its bytes/member occurrence.
        # Preserve interleaving; equality is still not COMDAT retention proof.
        sequences[version] = [
            {"candidates": sorted(({"producer": row["producer"], "format": row["format"]}
                                   for row in family["candidates"]), key=digest),
             "selection": family["selection"]}
            for family in report["families"]]
        dependencies[version] = [family for family in sequences[version]
                                if any(row["producer"]["kind"] == "dependency" for row in family["candidates"])]
    a, b = compiled["baseline"], compiled["current"]
    changes = [{"unit": unit, "baseline": a.get(unit), "current": b.get(unit)}
               for unit in sorted(a.keys() | b.keys()) if a.get(unit) != b.get(unit)]
    difference = _difference(dependencies["baseline"], dependencies["current"])
    order = dependencies["baseline"] != dependencies["current"]
    policy = bool(old["policy_gaps"] or new["policy_gaps"])
    return {"check": "native_input_selection_comparison", "result": Result.UNKNOWN,
            "candidate_coverage": Result.PASS, "dependency_families": Result.FAIL if order else
                Result.UNKNOWN if policy else Result.PASS,
            "compiled_changes": changes, "dependency_changes": difference,
            "family_sequence": deepcopy(sequences),
            "family_sequence_changed": sequences["baseline"] != sequences["current"],
            "dependency_order_changed": order,
            "dependency_candidate_changes": _difference(inventory["baseline"], inventory["current"]),
            "ambiguous_families": {"baseline": list(old["ambiguous_families"]), "current": list(new["ambiguous_families"])},
            "policy_gaps": {"baseline": deepcopy(old["policy_gaps"]), "current": deepcopy(new["policy_gaps"])},
            "retained_section_selection": Result.UNKNOWN, "allocation_isolation": Result.UNKNOWN,
            "remaining_linked_effects": Result.UNKNOWN, "persisted": False, "ready_for_collection": False}
