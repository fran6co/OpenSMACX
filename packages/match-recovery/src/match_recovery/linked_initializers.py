"""Bounded MSVC C++ initializer-table linkage, not startup equivalence.

The compiler puts dynamic-initializer pointers in .CRT$XCU; see
https://learn.microsoft.com/en-us/cpp/c-runtime-library/crt-initialization
This observes that named contribution, not every possible startup mechanism.
It never deduplicates registrations, assumes guard semantics or grants residual
ownership. Native callers keep both application builds alive throughout use.
"""

from collections import Counter
import hashlib
from pathlib import Path, PurePosixPath, PureWindowsPath

from .coff_archive import CoffArchive
from .linked_function import verify_function as _function
from .linked_code_graph import observe_code_graph, compare_code_graphs
from .linked_objects import SelectedObjects
from .link_file_evidence import parse_link_files
from .link_input_coverage import _input_inventory
from .model import Result, Span
from .msvc_link_map import parse_link_map
from .pe_coff import Coff, CoffRecords, PeImage


TABLE = ".CRT$XCU"


def _stamp(data):
    return {"sha256": hashlib.sha256(data).hexdigest(), "size": len(data)}


def _read(path, expected):
    path = Path(path)
    if (not path.is_absolute() or path.resolve() != path or not path.is_file()
            or path.stat().st_nlink != 1):
        raise ValueError("initializer evidence requires unaliased physical files")
    raw = path.read_bytes()
    if _stamp(raw) != expected:
        raise ValueError("initializer evidence file contents changed")
    return raw


def _artifacts(context):
    root, directory, report = Path(context.root), Path(context.directory), context.report
    if (root.resolve() != root or directory.resolve() != directory
            or not root.is_dir() or not directory.is_dir() or not directory.is_relative_to(root)
            or directory == root or report.get("result") != Result.PASS
            or report.get("compile_link", {}).get("result") != Result.PASS):
        raise ValueError("initializer observations require a successful live native application build")
    artifacts = {}
    for kind in ("pe32-i386", "msvc-map", "link-input-files", "msvc-link-log"):
        rows = [row for row in report["outputs"] if row["format"] == kind]
        if len(rows) != 1:
            raise ValueError("initializer observation requires one native artifact of format " + kind)
        row, = rows
        name = row["path"]
        if (not isinstance(name, str) or str(PurePosixPath(name)) != name
                or PurePosixPath(name).is_absolute() or ".." in PurePosixPath(name).parts):
            raise ValueError("initializer build artifact is not a relative path")
        artifacts[kind] = _read(directory / name, {key: row[key] for key in ("sha256", "size")})
    return artifacts


def _selected_objects(evidence, coverage, records):
    """Resolve final-pass owners through physical inputs, never map basenames alone."""
    selected, loaded, archives, has_table = {}, {}, {}, {}
    stamps = {row["path"]: row["after"] for row in evidence["files"]}
    stamps.update({row["path"]: row["after"] for row in evidence["generated_exports"]})

    def contents(record):
        path = record["path"]
        if path not in loaded:
            loaded[path] = _read(path, stamps[path])
        if "member" in record:
            if path not in archives:
                archives[path] = CoffArchive(loaded[path])
            raw = archives[path].members[record["member"]][record["occurrence"]]
        else:
            raw = loaded[path]
        if hashlib.sha256(raw).hexdigest() != record["sha256"]:
            raise ValueError("selected initializer object changed from its inventory")
        return raw

    def contributes(record):
        if record["format"] == "short_import":
            return False
        key = record["sha256"]
        if key not in has_table:
            obj = CoffRecords(contents(record))
            if any(section.name.startswith("/") for section in obj.sections):
                raise ValueError("unresolved COFF section names cannot establish initializer absence")
            has_table[key] = any(section.name == TABLE for section in obj.sections)
        return has_table[key]

    for final in coverage["inputs"]:
        if len(final["paths"]) != 1:
            raise ValueError("initializer input has an ambiguous physical owner")
        path, = final["paths"]
        candidates = [row for row in records if row["path"] == path and (
            "member" not in row if final["library"] is None else
            "member" in row and PurePosixPath(row["member"]).name.casefold()
            == PureWindowsPath(final["member"]).name.casefold())]
        # Repeated import member labels can have several ordinary COFF forms,
        # not only short imports. Independently inspect every candidate before
        # concluding it cannot contribute to this particular table. This is
        # absence of a named section, not a guess about which member won.
        if candidates and all(not contributes(row) for row in candidates):
            continue
        if final["selection"] != Result.PASS or len(candidates) != 1:
            raise ValueError("initializer input has an ambiguous selected object: " + final["label"])
        record, = candidates
        if "member" in record:
            owner = PureWindowsPath(final["library"]).stem + ":" + PureWindowsPath(final["member"]).name
        else:
            owner = PureWindowsPath(final["member"]).name
        raw = contents(record)
        identity = {key: record[key] for key in ("path", "member", "occurrence", "generated") if key in record}
        item = {"identity": identity, "sha256": record["sha256"], "raw": raw, "owner": owner}
        key = owner.casefold()
        if key in selected and selected[key] != item:
            raise ValueError("different initializer inputs share the same map owner label")
        selected[key] = item
    # The final-pass population is not itself a proof that explicit inputs were
    # all represented. An omitted initializer object needs discard/selection
    # evidence; it cannot disappear from a supposedly empty table observation.
    for record in records:
        if "member" in record or "generated" in record or not contributes(record):
            continue
        path = record["path"]
        matches = [row for row in coverage["inputs"] if row["library"] is None and row["paths"] == [path]]
        if (len(matches) != 1 or matches[0]["selection"] != Result.PASS
                or not any(item["identity"] == {"path": path} for item in selected.values())):
            raise ValueError("explicit initializer contributor lacks unique final-pass representation: " + path)
    return selected


def _observe(context, artifacts):
    image = PeImage(artifacts["pe32-i386"], hashlib.sha256(artifacts["pe32-i386"]).hexdigest())
    mapping = parse_link_map(artifacts["msvc-map"])
    mapping.validate(image)
    if mapping.contributions is None:
        raise ValueError("initializer table lacks explicit linker contribution evidence")
    coverage, records = _input_inventory(artifacts["link-input-files"], artifacts["msvc-link-log"])
    if coverage["result"] != Result.PASS:
        raise ValueError("initializer table requires complete linked input candidate coverage")
    evidence = parse_link_files(artifacts["link-input-files"])
    selected = _selected_objects(evidence, coverage, records)
    functions, entries = {}, []
    for item in selected.values():
        obj = CoffRecords(item["raw"])
        tables = [(index, section) for index, section in enumerate(obj.sections, 1) if section.name == TABLE]
        if not tables:
            continue
        obj = Coff(item["raw"])
        for index, section in tables:
            if (len(section.data) != obj.section_sizes[index - 1] or len(section.data) % 4
                    or section.flags & (0x20 | 0x80 | 0x1000 | 0x20000000)
                    or section.alignment != 4):
                raise ValueError("unsupported initializer table object contribution")
            relocations = {offset: (target, kind) for offset, target, kind in section.relocations}
            if (len(relocations) != len(section.relocations)
                    or set(relocations) != set(range(0, len(section.data), 4)) or any(section.data)):
                raise ValueError("initializer entries require exactly one zero-addend pointer relocation per slot")
            for offset in range(0, len(section.data), 4):
                target, kind = relocations[offset]
                symbol = obj.symbols[target]
                if kind != 6 or symbol.section <= 0 or symbol.storage not in (2, 3) or symbol.value != 0:
                    raise ValueError("initializer entry does not target a defined zero-offset function")
                key = (item["owner"].casefold(), symbol.name)
                if key not in functions:
                    functions[key] = _function(obj, symbol, item["owner"], mapping, image)
                function = functions[key]
                entries.append({"input": item["identity"], "object_sha256": item["sha256"],
                                "section": index, "offset": offset, "target": function["address"],
                                "function": {"owner": item["owner"], "symbol": symbol.name}})
    contributions = [row for row in mapping.contributions if row.name == TABLE]
    if not entries and not contributions:
        return {"result": Result.PASS, "table": TABLE, "entries": [], "sequence": [],
                "contribution": None, "functions": [], "input_coverage": Result.PASS,
                "code_dependencies": observe_code_graph(SelectedObjects(evidence, coverage, records), mapping, image, [])}
    if len(contributions) != 1:
        raise ValueError("initializer table requires one independently bounded linked contribution")
    table, = contributions
    if table.kind != "DATA" or not table.size or table.size % 4:
        raise ValueError("initializer table is not a nonempty pointer-sized DATA contribution")
    start = image.sections[table.segment - 1][0].start + table.offset
    if start % 4 or image.section_flags[table.segment - 1] & 0x20000000:
        raise ValueError("initializer table has unsupported alignment or executable storage")
    span = Span(start, start + table.size)
    raw, sites = image.read_initial(span), image.absolute_sites(span)
    values = [int.from_bytes(raw[index:index + 4], "little") for index in range(0, len(raw), 4)]
    expected = Counter(entry["target"] for entry in entries)
    table_ok = (Counter(values) == expected and sites == frozenset(range(0, len(raw), 4)))
    by_address = {}
    for function in functions.values():
        by_address.setdefault(function["address"], []).append(function)
    if any(len(rows) != 1 for rows in by_address.values()):
        raise ValueError("initializer functions have ambiguous folded linked placements")
    sequence = [{"slot": start + index * 4, "target": value,
                 "body_fingerprint": by_address[value][0]["body_fingerprint"] if value in by_address else None,
                 "function": {key: by_address[value][0][key] for key in ("owner", "symbol")} if value in by_address else None}
                for index, value in enumerate(values)]
    results = [row["result"] for row in functions.values()]
    result = (Result.FAIL if not table_ok or Result.FAIL in results else
              Result.UNKNOWN if Result.UNKNOWN in results else Result.PASS)
    dependencies = (observe_code_graph(SelectedObjects(evidence, coverage, records), mapping, image,
                                      [row["function"] for row in sequence]) if table_ok else
                    {"result": Result.UNKNOWN, "reason": "initializer root table is not verified"})
    return {"result": result, "table": TABLE, "entries": entries, "sequence": sequence,
            "contribution": {"address": start, "size": table.size, "segment": table.segment},
            "table_correspondence": Result.PASS if table_ok else Result.FAIL,
            "functions": list(functions.values()), "input_coverage": Result.PASS,
            "code_dependencies": dependencies}


def observe_build_initializers(context):
    """Verify one actual table and its code; no source/graph ownership inference."""
    try:
        context.check_current()
        try:
            artifacts = _artifacts(context)
            report = _observe(context, artifacts)
            report["artifacts"] = {kind: _stamp(raw) for kind, raw in artifacts.items()}
        except (OSError, ValueError, KeyError, TypeError, AttributeError) as error:
            report = {"result": Result.UNKNOWN, "reason": str(error)}
        return {"check": "linked_initializer_table", **report,
                "remaining_linked_effects": Result.UNKNOWN, "persisted": False, "ready_for_collection": False,
                "limitations": ["one named MSVC C++ initializer contribution, not complete startup or runtime proof",
                                "target placements do not prove callback/data contents, guard effects or ownership",
                                "no registration deduplication, residual waiver or original application binding"]}
    finally:
        context.check_current()


def compare_build_initializers(baseline, current):
    def fresh():
        try:
            baseline.check_current()
        finally:
            current.check_current()
    try:
        fresh()
        old, new = observe_build_initializers(baseline), observe_build_initializers(current)
        equal = [row["body_fingerprint"] for row in old.get("sequence", [])] == [
            row["body_fingerprint"] for row in new.get("sequence", [])]
        results = [old["result"], new["result"]]
        observations_complete = all(value == Result.PASS for value in results)
        ordered = (Result.FAIL if Result.FAIL in results else Result.UNKNOWN if Result.UNKNOWN in results else
                   Result.PASS if equal else Result.FAIL)
        dependencies = compare_code_graphs(
            old.get("code_dependencies", {"result": Result.UNKNOWN}),
            new.get("code_dependencies", {"result": Result.UNKNOWN}))
        results = [ordered, dependencies["result"]]
        result = (Result.FAIL if Result.FAIL in results else Result.UNKNOWN if Result.UNKNOWN in results else Result.PASS)
        summary = {"baseline_slots": len(old.get("sequence", [])), "current_slots": len(new.get("sequence", [])),
                   "observations_complete": observations_complete}
        if summary["observations_complete"]:
            left = Counter(row["body_fingerprint"] for row in old["sequence"])
            right = Counter(row["body_fingerprint"] for row in new["sequence"])
            summary.update(added_registrations=sum((right - left).values()),
                           removed_registrations=sum((left - right).values()), ordered_equal=equal)
        return {"check": "application_linked_initializers", "result": result, "baseline": old, "current": new,
                "summary": summary,
                "ordered_code_correspondence": ordered, "code_dependencies": dependencies,
                "remaining_linked_effects": Result.UNKNOWN, "persisted": False, "ready_for_collection": False,
                "limitations": ["exact ordered code/relocation identities retain every registration occurrence",
                                "changed sequence requires scope/effect proof; it is not automatically an outside-scope regression",
                                "selected relocation-reachable helper/callback code is checked, not just initializer names or addresses",
                                "matching table code alone cannot discharge callback/storage/selection or other residual obligations"]}
    finally:
        fresh()
