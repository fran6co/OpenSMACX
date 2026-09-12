"""Native source-qualified startup order, not callback or storage equivalence.

Only the enclosing live observer supplies observations here. A serialized report
does not authorize projection, and this bounded result is not graph acceptance.
"""

from collections import defaultdict
import hashlib
from itertools import zip_longest
from pathlib import Path

from .build_graph_commands import _context
from .generated_source import parse_generated
from .import_source import parse_imports
from .link_file_evidence import _stamp
from .linked_initializers import _artifacts, _read
from .migration_protocol import _generated_changes
from .model import Result, digest
from .msvc_link_map import parse_link_map
from .object_source import parse_objects
from .pe_coff import Coff, PeImage
from .pe_exports import exports
from .source.reader import read_text
from .source_config import SourceConfiguration
from .source_inventory import image_pin
from .shared_source import parse_shared


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
    return {"sequence": sequence, "covered": covered}, {"entries": entries, "objects": objects}


def _bound_names(root):
    configuration = SourceConfiguration.read(root)
    pin, names = image_pin(root, configuration), set()
    # Include headers and seeds conservatively: a source-owned identity is not
    # an anonymous startup root just because this particular TU emits it.
    for path in configuration.files(root):
        if path.resolve() != path or path.stat().st_nlink != 1:
            raise ValueError("slot-local source evidence requires unaliased source files")
        text = path.read_text(encoding="utf-8")
        for record in read_text(text, path):
            names.update((record.name, record.symbol))
        relative = path.relative_to(root).as_posix()
        for parse in (parse_objects, parse_shared, parse_generated):
            names.update(item.symbol for item in parse(text, relative, pin, address_space=configuration.address_space))
        names.update(item.symbol for item in parse_imports(text, relative))
    return names


class _LocalRoots:
    """Read-only, in-epoch eligibility, never a coverage or admission token."""

    def __init__(self, context, observed, projection):
        artifacts = _artifacts(context)
        # The enclosing observer and its final context guard own freshness.
        if {kind: {"sha256": hashlib.sha256(raw).hexdigest(), "size": len(raw)}
                for kind, raw in artifacts.items()} != observed["artifacts"]:
            raise ValueError("slot-local linkage differs from the native observation")
        raw = artifacts["pe32-i386"]
        self.image = PeImage(raw, hashlib.sha256(raw).hexdigest())
        self.mapping = parse_link_map(artifacts["msvc-map"])
        self.mapping.validate(self.image)
        self.exports = exports(self.image)
        table = observed["contribution"]
        table_spans = [(table["address"], table["address"] + table["size"])]
        for row in self.mapping.contributions or ():
            if row.name in (".CRT$XCA", ".CRT$XCZ"):
                start = self.image.sections[row.segment - 1][0].start + row.offset
                table_spans.append((start, start + row.size))
        if any(row.address is not None and any(start <= row.address < end for start, end in table_spans)
               for row in self.exports):
            raise ValueError("slot-local initializer table has an exported address")
        if any(row.segment and table["address"] <= row.address < table["address"] + table["size"]
               for row in self.mapping.publics):
            raise ValueError("slot-local initializer table has a public linked alias")
        self.names = _bound_names(Path(context.root))
        self.observed, self.projection = observed, projection

    def check(self, unit, name):
        from .startup_local import slot_local_root

        matches = [item["entry"] for item in self.projection["entries"]
                   if item["unit"] == unit and item["identity"]["symbol"] == name]
        if len(matches) != 1:
            raise ValueError("slot-local root lacks one exact compiled source occurrence")
        entry, = matches
        positions = [row for row in self.observed["sequence"] if row["target"] == entry["target"]]
        if len(positions) != 1:
            raise ValueError("slot-local root has repeated or shared linked occurrences")
        obj = self.projection["objects"][entry["input"]["path"]]
        symbol = obj.function(name)
        local = slot_local_root(obj, table=entry["section"], offset=entry["offset"], symbol=symbol)
        sections = {symbol.section, entry["section"],
                    *(row["section"] for row in local["recognized_debug_children"])}
        if any(item.name in self.names for item in obj.symbols.values() if item.section in sections):
            raise ValueError("slot-local root overlaps an original source binding")
        size = len(obj.sections[symbol.section - 1].data)
        if any(row.address is not None and entry["target"] <= row.address < entry["target"] + size
               for row in self.exports):
            raise ValueError("slot-local root has an exported entry or interior alias")
        matches = [row for row in (*self.mapping.publics, *(self.mapping.statics or ()))
                   if row.segment and entry["target"] <= row.address < entry["target"] + size]
        if (len(matches) != 1 or matches[0].symbol != name or not matches[0].function
                or matches[0].owner.casefold() != entry["function"]["owner"].casefold()
                or matches[0].address != entry["target"]
                or matches[0] not in (self.mapping.statics or ())):
            raise ValueError("slot-local root has linked aliases or another placed entry")
        # Retain the entire recognized root-relative metadata. This does not
        # prove its final linked placement or discard: those remain independent
        # unknown effects even when both versions' bounded order agrees.
        return [{"format": row["format"], "code_size": row["code_size"], "data": row["data"],
                 "reference": {"offset": row["relocation"][0], "kind": row["relocation"][2], "target": "root"}}
                for row in local["recognized_debug_children"]]


def _rename_roots(contexts, observations, projections, before, after):
    local, renamed, refused, metadata = {}, [], [], []
    for index, (left, right) in enumerate(zip(before, after)):
        a, b = left["identity"], right["identity"]
        if (a.get("kind") != "compiled" or b.get("kind") != "compiled"
                or a["unit"] != b["unit"] or a["symbol"] == b["symbol"]
                or left["body_fingerprint"] != right["body_fingerprint"]):
            continue
        try:
            details = {}
            for version, identity in (("baseline", a), ("current", b)):
                if version not in local:
                    local[version] = _LocalRoots(contexts[version], observations[version], projections[version])
                details[version] = local[version].check(identity["unit"], identity["symbol"])
            if details["baseline"] != details["current"]:
                raise ValueError("slot-local roots have different associated metadata")
        except (OSError, ValueError, KeyError, TypeError, AttributeError) as error:
            if len(refused) < 16:
                refused.append({"index": index, "reason": str(error)})
            continue
        # Only a proven slot-local root can use positional occurrence identity.
        # Distinct slots stay distinct; owners or equal-byte functions are never
        # searched/reordered to manufacture a pairing.
        identity = {"kind": "slot_local", "unit": a["unit"], "occurrence": index,
                    "metadata": details["baseline"]}
        renamed.append({"index": index, "unit": a["unit"], "baseline_symbol": a["symbol"],
                        "current_symbol": b["symbol"], "body_fingerprint": left["body_fingerprint"]})
        if details["baseline"]:
            metadata.append({"index": index, "result": Result.PASS, "metadata": details["baseline"],
                             "linked_treatment": Result.UNKNOWN})
        left["identity"], right["identity"] = identity, dict(identity)
    return renamed, refused, metadata


def _compare_order(baseline, current, old, new, changes):
    """Consume fresh observations only inside compare_build_initializers."""
    result = {"check": "application_projected_initializer_order", "result": Result.UNKNOWN,
              "remaining_linked_effects": Result.UNKNOWN, "persisted": False, "ready_for_collection": False,
              "limitations": ["exact source-qualified root order only; callbacks, storage and other effects remain independent",
                              "decisions identify intended changes, not equivalence or source ownership proof",
                              "root renaming requires native slot-local references and equal rooted metadata, not naming patterns",
                              "symbolic incoming-reference checks are not a proof against arbitrary address computation or external address identity",
                              "retained typed frame metadata does not establish final-link debug treatment or address-identity effects"]}
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
        projections = {}
        for version, context, observed in (("baseline", baseline, old), ("current", current, new)):
            result[version], projections[version] = _project(
                context, observed, [row for row in changes if row["version"] == version])
        def logical(version):
            return [{key: row[key] for key in ("identity", "body_fingerprint")} for row in result[version]["sequence"]]
        old_sequence, new_sequence = logical("baseline"), logical("current")
        renamed, refused, metadata = _rename_roots({"baseline": baseline, "current": current},
                                                   {"baseline": old, "current": new}, projections,
                                                   old_sequence, new_sequence)
        if renamed:
            result["root_renamings"] = renamed
        if refused:
            result["root_renaming_refusals"] = refused
        if metadata:
            result["root_metadata_comparisons"] = metadata
        equal = old_sequence == new_sequence
        result.update(result=Result.PASS if equal else Result.FAIL,
                      baseline_fingerprint=digest(old_sequence), current_fingerprint=digest(new_sequence))
        if not equal:
            mismatches, count = [], 0
            for index, (before, after) in enumerate(zip_longest(old_sequence, new_sequence)):
                if before != after:
                    count += 1
                    if len(mismatches) < 16:
                        mismatches.append({"index": index, "baseline": before, "current": after})
            result.update(mismatch_count=count, mismatches=mismatches)
    except (OSError, ValueError, KeyError, TypeError, AttributeError) as error:
        result.update(result=Result.UNKNOWN, reason=str(error))
    return result
