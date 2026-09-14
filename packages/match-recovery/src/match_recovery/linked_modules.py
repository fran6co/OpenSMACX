"""PDB modules bound to independently selected physical linker inputs.

Debug labels do not select archive occurrences. Use actual command operands or
observed library spellings, complete physical candidates, and native final-pass
selection together. Module binding does not identify a V60 SC's COFF section.
"""

from collections import Counter, defaultdict
from pathlib import PureWindowsPath

from .build_graph_commands import _context
from .linked_objects import _owner
from .linked_storage_census import _identity
from .model import Result, digest
from .pdb_contributions import bound_contributions


def _label(value):
    # Compare target-platform spellings only. Never open a PDB path, translate
    # drives, collapse '..', or resolve archive-member paths on the host.
    return tuple(part.casefold() for part in PureWindowsPath(value).parts)


def _selected(inputs, record):
    identity = _identity(record)
    rows = [row for row in inputs.coverage["inputs"] if row["paths"] == [identity["path"]]
            and (row["library"] is not None) == ("member" in identity)
            and PureWindowsPath(row["member"]).name.casefold() ==
                PureWindowsPath(identity.get("member", identity["path"])).name.casefold()]
    if len(rows) != 1:
        raise ValueError("PDB module lacks a unique final-pass owner")
    selected = inputs.objects.resolve(_owner(rows[0]))
    if selected["identity"] != identity or selected["sha256"] != record["sha256"]:
        raise ValueError("PDB module candidate disagrees with independently selected object")
    return selected


def observe_modules(context, inputs):
    """Observe original native context/inputs, never supplied module reports."""
    before = digest(context.report)
    records, fresh = inputs.objects._constant_population()
    try:
        context.check_current()
        _, _, compilations, command = _context(context)
        if (list(command.arguments) != inputs.evidence["arguments"]
                or str(command.cwd) != inputs.evidence["cwd"]):
            raise ValueError("module command binding differs from actual link input evidence")
        pdb = inputs.artifacts.get("msvc-pdb")
        if pdb is None:
            raise ValueError("module binding requires this build's PDB sidecar")
        inventory = bound_contributions(inputs.image, pdb)
        standalone = defaultdict(list)
        explicit = set(inputs.evidence["explicit_inputs"])
        for operand in command.operands:
            if operand.kind != "input_file":
                continue
            if str(operand.path) not in explicit:
                raise ValueError("module input operand is not an observed explicit link input")
            if operand.start == 0 and operand.end == len(command.arguments[operand.argument]):
                standalone[command.arguments[operand.argument]].append(str(operand.path))
        libraries = defaultdict(set)
        for row in inputs.evidence["library_searches"]:
            if row["path"] is not None:
                libraries[_label(row["spelling"])].add(row["path"])
        compiled = {str(row["output"]): unit for unit, row in compilations.items()}
        modules = []
        for index, module in enumerate(inventory.modules):
            row = {"index": index, "name": module.name, "object_name": module.object_name,
                   "kind": "unresolved", "result": Result.UNKNOWN, "candidates": []}
            modules.append(row)
            candidates, problem = [], None
            if module.name == "* Linker *" and not module.object_name:
                row.update(kind="linker_generated", reason="synthetic module has no physical object identity")
                continue
            if module.name == module.object_name and module.name in standalone:
                paths = standalone[module.name]
                row["kind"] = "standalone_object"
                candidates = [record for record in records if "member" not in record
                              and record["path"] in paths and "generated" not in record]
                if len(paths) != 1:
                    problem = "PDB standalone label has multiple exact command operands"
            elif _label(module.object_name) in libraries:
                paths = libraries[_label(module.object_name)]
                row["kind"] = "archive_object"
                candidates = [record for record in records if record["path"] in paths
                              and "member" in record and _label(record["member"]) == _label(module.name)]
                if len(paths) != 1:
                    problem = "PDB archive label has multiple observed physical library paths"
            elif module.name == module.object_name:
                # Generated exports do not occur as explicit input operands.
                # Require the existing fresh-generation/final-pass inventory,
                # and its exact observed standalone label, not a basename guess.
                labels = [row for row in inputs.coverage["inputs"] if row["library"] is None
                          and row["member"] == module.name]
                paths = {path for item in labels for path in item["paths"]}
                candidates = [record for record in records if record.get("generated") == "msvc_export"
                              and record["path"] in paths]
                if candidates:
                    row["kind"] = "generated_export"
            row["candidates"] = [{"input": _identity(record), "object_sha256": record["sha256"]}
                                 for record in candidates]
            try:
                if problem:
                    raise ValueError(problem)
                if len(candidates) != 1:
                    raise ValueError("PDB module has no unique full physical candidate identity")
                record, = candidates
                selected = _selected(inputs, record)
                row.update(result=Result.PASS, input=dict(selected["identity"]),
                           object_sha256=selected["sha256"], owner=selected["owner"])
                if record["path"] in compiled and "member" not in record:
                    row["source"] = compiled[record["path"]]
            except ValueError as error:
                row["reason"] = str(error)
        # A second module may not silently borrow one physical selected object.
        identities = Counter(digest(row["input"]) for row in modules if row["result"] == Result.PASS)
        for row in modules:
            if row["result"] == Result.PASS and identities[digest(row["input"])] != 1:
                row.update(result=Result.UNKNOWN, reason="multiple PDB modules name the same physical selected input")
        return {"check": "linked_pdb_module_bindings", "result": Result.UNKNOWN,
                "modules": modules, "counts": dict(Counter(row["result"] for row in modules)),
                "image_sha256": inventory.image_sha256, "pdb_sha256": inventory.pdb_sha256,
                "retained_storage_coverage": Result.UNKNOWN, "allocation_isolation": Result.UNKNOWN,
                "persisted": False, "ready_for_collection": False,
                "limitations": ["native selected input identity, not a V60 contribution's original COFF section",
                                "ambiguous import families and linker-generated modules retain separate obligations",
                                "binding module labels does not prove contents, storage isolation or acceptance"]}
    finally:
        try:
            fresh()
            if digest(context.report) != before:
                raise ValueError("module application command report changed during observation")
        finally:
            context.check_current()
