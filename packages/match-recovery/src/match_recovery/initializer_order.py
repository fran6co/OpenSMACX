"""Native source-qualified startup order, not callback or storage equivalence.

Only the enclosing live observer supplies observations here. A serialized report
does not authorize projection, and this bounded result is not graph acceptance.
"""

from collections import defaultdict
from pathlib import Path

from .build_graph_commands import _context
from .link_file_evidence import _stamp
from .linked_initializers import _read
from .migration_protocol import _generated_changes
from .model import Result, digest
from .pe_coff import Coff


def _input_identity(path, root, directory):
    path = Path(path)
    if not path.is_absolute() or path.resolve() != path or not path.is_file():
        raise ValueError("initializer owner requires a canonical physical input")
    if path.is_relative_to(directory):
        return {"kind": "build", "path": path.relative_to(directory).as_posix()}
    if path.is_relative_to(root):
        return {"kind": "project", "path": path.relative_to(root).as_posix()}
    return {"kind": "external", "path": str(path)}


def _project(context, observed, changes):
    root, directory, compilations, _ = _context(context)
    compiled = {str(row["output"]): unit for unit, row in compilations.items()}
    receipts = {row["source"]: row["compilation"]["output"]["after"]
                for row in context.report["compile_link"]["compilations"]}
    entries, by_target, objects = [], defaultdict(list), {}
    for row in observed["entries"]:
        source = row["input"]
        path, unit = source["path"], None
        if set(source) == {"path"} and path in compiled:
            unit = compiled[path]
            receipt = receipts[str(root / unit)]
            _stamp(receipt)
            if receipt is None or receipt["sha256"] != row["object_sha256"]:
                raise ValueError("initializer input differs from its exact native compilation receipt")
            if path not in objects:
                objects[path] = Coff(_read(path, receipt))
            identity = {"kind": "compiled", "unit": unit, "symbol": row["function"]["symbol"]}
        else:
            identity = {"kind": "dependency", "input": _input_identity(path, root, directory),
                        "member": {key: value for key, value in source.items() if key != "path"},
                        "object_sha256": row["object_sha256"], "symbol": row["function"]["symbol"]}
        entry = {"unit": unit, "identity": identity, "entry": row}
        entries.append(entry)
        by_target[row["target"]].append(entry)

    positions = defaultdict(list)
    for index, row in enumerate(observed["sequence"]):
        positions[row["target"]].append(index)
    covered, omitted = [], set()
    for change in changes:
        matches = [item for item in entries if item["unit"] == change["unit"]
                   and item["entry"]["offset"] == change["offset"]
                   and item["entry"]["function"]["symbol"] == change["symbol"]]
        if len(matches) != 1:
            raise ValueError("generated change lacks one exact source/table initializer entry")
        item, = matches
        entry = item["entry"]
        target = entry["target"]
        if len(by_target[target]) != 1 or len(positions[target]) != 1:
            raise ValueError("selected initializer has ambiguous linked occurrence placement")
        obj = objects[entry["input"]["path"]]
        tables = [number for number, section in enumerate(obj.sections, 1) if section.name == ".CRT$XCU"]
        symbol = obj.function(change["symbol"])
        if (tables != [entry["section"]] or symbol.storage != 3
                or (change["offset"], symbol.index, 6) not in obj.sections[tables[0] - 1].relocations):
            raise ValueError("generated change lacks its unique ordinary table and static root")
        index, = positions[target]
        if index in omitted:
            raise ValueError("generated changes select the same linked occurrence")
        omitted.add(index)
        covered.append({**change, "slot": observed["sequence"][index]["slot"], "sequence_index": index})

    sequence = []
    for index, row in enumerate(observed["sequence"]):
        candidates = by_target[row["target"]]
        # Repeated unselected registrations retain every list occurrence, but
        # distinct source owners may not borrow one another's linked placement.
        if (not candidates or len({digest(item["identity"]) for item in candidates}) != 1
                or any(item["entry"]["function"] != row["function"] for item in candidates)
                or len(candidates) != len(positions[row["target"]])):
            raise ValueError("initializer sequence lacks unambiguous source-qualified membership")
        if index not in omitted:
            sequence.append({"identity": candidates[0]["identity"], "body_fingerprint": row["body_fingerprint"],
                             "slot": row["slot"], "offsets": sorted(item["entry"]["offset"] for item in candidates)})
    return {"sequence": sequence, "covered": covered}


def _compare_order(baseline, current, old, new, changes):
    """Consume fresh observations only inside compare_build_initializers."""
    result = {"check": "application_projected_initializer_order", "result": Result.UNKNOWN,
              "remaining_linked_effects": Result.UNKNOWN, "persisted": False, "ready_for_collection": False,
              "limitations": ["exact source-qualified root order only; callbacks, storage and other effects remain independent",
                              "decisions identify intended changes, not equivalence or source ownership proof",
                              "static root spellings are retained; compiler renumbering is not inferred from equal bytes"]}
    try:
        if not isinstance(changes, list):
            raise ValueError("generated order selections must be an array")
        if changes:
            # Syntax only: true task/source authority belongs to the enclosing
            # application graph. This reporter cannot grant it to itself.
            scope = frozenset(row.get("entity_id") for row in changes
                              if isinstance(row, dict) and isinstance(row.get("entity_id"), str))
            _generated_changes(changes, scope)
        if any(row["result"] != Result.PASS for row in (old, new)):
            result["result"] = Result.FAIL if any(row["result"] == Result.FAIL for row in (old, new)) else Result.UNKNOWN
            result["reason"] = "projected order requires two complete consistent initializer observations"
            return result
        for version, context, observed in (("baseline", baseline, old), ("current", current, new)):
            result[version] = _project(context, observed, [row for row in changes if row["version"] == version])
        def logical(version):
            return [{key: row[key] for key in ("identity", "body_fingerprint")} for row in result[version]["sequence"]]
        equal = logical("baseline") == logical("current")
        result.update(result=Result.PASS if equal else Result.FAIL,
                      baseline_fingerprint=digest(logical("baseline")), current_fingerprint=digest(logical("current")))
    except (OSError, ValueError, KeyError, TypeError, AttributeError) as error:
        result.update(result=Result.UNKNOWN, reason=str(error))
    return result
