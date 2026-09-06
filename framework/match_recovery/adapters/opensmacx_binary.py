"""Fresh task-scoped application measurements, using the strict shared checker.

No claim stamping, automatic demotion, missing-target inference or seed promotion.
This is a scoped diagnostic, not the full build/ownership/runtime admission gate.
"""

from collections import Counter, defaultdict
from pathlib import Path
from typing import Callable

from ..bound_objects import FunctionBinding, check_objects
from ..fingerprint import input_snapshot
from ..generated_source import read_generated
from ..model import Origin, Result, Span, digest
from ..object_source import read_objects, object_targets
from ..pe_coff import Coff, PeImage
from ..tasks import TaskStore
from .opensmacx_project import project_view
from .vc6 import FLAG_SETS, compile_object
from .vc6_generated import check_generated


def select_bindings(view: dict, selected: frozenset[str]) -> tuple[list, list, dict]:
    """Require a single product definition; archived interpretations remain seeds."""
    bindings, refused, targets = [], [], defaultdict(set)
    ambiguous = set()
    found = set()
    for group in view["groups"]:
        for entity in group["entities"]:
            key = entity["entity_id"]
            found.add(key)
            candidates = [item for item in entity["candidates"] if not item["seed"]]
            candidate = candidates[0] if len(candidates) == 1 and not entity["conflicts"] else None
            if candidate is None:
                ambiguous.update(item["symbol"] for item in candidates)
            if candidate is not None:
                # Exact source spelling only. Ambiguous external names are
                # removed below; static definitions never become external COFF
                # symbols and so cannot borrow this table during resolution.
                targets[candidate["symbol"]].add(entity["origin"]["address"])
            if key not in selected:
                continue
            reason = None
            if candidate is None:
                reason = "requires one unambiguous product definition; seeds are not production source"
            elif candidate["component"] is not None:
                reason = "dependency binding: use component verification, not application recovery"
            if reason is None:
                try:
                    bindings.append(FunctionBinding(Origin(**entity["origin"]),
                                                     tuple(Span(**span) for span in candidate["spans"]),
                                                     candidate["symbol"], candidate["path"]))
                except ValueError as error:
                    reason = str(error)
            if reason is not None:
                refused.append({"entity_id": key, "address": f'0x{entity["origin"]["address"]:08X}',
                                "result": Result.UNKNOWN, "reason": reason})
    refused.extend({"entity_id": key, "result": Result.UNKNOWN, "reason": "source mapping is missing"}
                   for key in sorted(selected - found))
    ambiguous.update(symbol for symbol, addresses in targets.items() if len(addresses) != 1)
    accepted = []
    for binding in bindings:
        if binding.symbol in ambiguous:
            refused.append({"entity_id": binding.origin.entity_id, "result": Result.UNKNOWN,
                            "symbol": binding.symbol, "reason": "ambiguous source symbol binding"})
        else:
            accepted.append(binding)
    return accepted, refused, {symbol: next(iter(addresses)) for symbol, addresses in targets.items()
                               if symbol not in ambiguous}


def _inputs(root: Path) -> tuple[str, ...]:
    # Deliberately broad until the dependency-cache cutover. Include newly added
    # files in the after-check too; a fixed list would miss a new shadowing header.
    paths = [path.relative_to(root).as_posix() for directory in ("src", "cmake")
             for path in (root / directory).rglob("*") if path.is_file()]
    return tuple(sorted(set(paths) | {"build/compile_commands.json", "tools/vc6_cl.py", "tools/vc6-cl"}))


def verify_task(root: Path, task_name: str, image_path: Path, *,
                compiler: Callable = compile_object, command_for: Callable | None = None) -> dict:
    root = root.resolve()
    store = TaskStore(root, private=".opensmacx/recovery")
    task = store.read(task_name)
    if task.target != "opensmacx":
        raise ValueError("this target adapter only measures OpenSMACX tasks")
    decisions = store.decisions(task_name)
    selected, _ = store._scope(task, decisions)
    if not selected:
        raise ValueError("refusing an empty task measurement")
    before = input_snapshot(root, _inputs(root), {"flags": FLAG_SETS})
    view, snapshot = project_view(root)
    if any(subject.image_sha256 != snapshot["image_sha256"] or subject.address_space != "pe:va"
           for subject in task.subjects):
        raise ValueError("task refers to a different original image/address space")
    image = PeImage(image_path.read_bytes(), snapshot["image_sha256"])
    object_bindings = read_objects(root, snapshot["image_sha256"])
    generated_bindings = read_generated(root, snapshot["image_sha256"])
    bindings, refused, externals = select_bindings(view, selected)
    if {item.symbol for item in object_bindings} & set(externals):
        raise ValueError("a source symbol is bound as both function and object")
    if {item.symbol for item in generated_bindings} & (set(externals) | {item.symbol for item in object_bindings}):
        raise ValueError("generated symbol also has a function/object binding")
    if command_for is None:
        from decomp.asm import build_command
        command_for = lambda path: tuple(build_command(root / "build/compile_commands.json", path))
    units = sorted({item.unit for item in bindings})
    commands, command_errors = {}, {}
    for unit in units:
        try:
            commands[unit] = tuple(command_for(root / unit))
        except ValueError as error:
            command_errors[unit] = str(error)
    attempts, variants, object_checks = 0, defaultdict(list), []
    generated_checks = []
    for flags in FLAG_SETS:
        objects, errors = {}, dict(command_errors)
        for unit, command in commands.items():
            attempts += 1
            try:
                objects[unit] = Coff(compiler(root / unit, command, flags))
            except (OSError, ValueError) as error:
                errors[unit] = str(error)
        data_targets, checked_objects = object_targets(object_bindings, objects)
        object_checks.extend({**item, "flags": list(flags)} for item in checked_objects)
        generated_targets, checked_generated = check_generated(generated_bindings, objects, image, {**externals, **data_targets})
        generated_checks.extend({**item, "flags": list(flags)} for item in checked_generated)
        for row in check_objects(bindings, objects, image, external_targets={**externals, **data_targets, **generated_targets}, errors=errors):
            variants[row["entity_id"]].append({**row, "flags": list(flags)})
    rows = list(refused)
    for key, candidates in sorted(variants.items()):
        results = {item["result"] for item in candidates}
        # Flags are alternatives. A failed trial cannot hide an unmeasurable
        # trial that might match, and two partial trials cannot be combined.
        result = (Result.PASS if Result.PASS in results else Result.UNKNOWN
                  if Result.UNKNOWN in results else Result.FAIL)
        rows.append({**{name: candidates[0][name] for name in ("entity_id", "address", "symbol", "unit")},
                     "result": result, "variants": candidates})
    if (input_snapshot(root, _inputs(root), {"flags": FLAG_SETS}) != before
            or store.read(task_name) != task or store.decisions(task_name) != decisions):
        raise ValueError("source, build inputs or task decisions changed during measurement; retry")
    counts = Counter(row["result"] for row in rows)
    overall = (Result.UNKNOWN if counts[Result.UNKNOWN] else Result.FAIL if counts[Result.FAIL] else Result.PASS)
    table_results = {item["content"] for item in generated_checks}
    if Result.UNKNOWN in table_results:
        overall = Result.UNKNOWN
    elif Result.FAIL in table_results and overall != Result.UNKNOWN:
        overall = Result.FAIL
    return {"task_id": task_name, "check": "scoped_binary_references", "result": overall,
            "input_fingerprint": digest(before), "functions": rows, "counts": dict(sorted(counts.items())),
            "compile_attempts": attempts, "selected_entities": len(selected), "translation_units": units,
            "object_definitions": object_checks,
            "generated_tables": generated_checks,
            "unresolved_symbols": sorted({relocation["symbol"] for candidates in variants.values()
                                           for row in candidates for relocation in row.get("relocations", [])
                                           if relocation["result"] == Result.UNKNOWN}),
            "scope_complete": False, "ready_for_collection": False, "persisted": False,
            "build_verification": Result.UNKNOWN, "structure_verification": Result.UNKNOWN,
            "runtime_verification": Result.UNKNOWN,
            "limitations": ["task subjects only; dependent callers require a wider admission gate",
                            "object starts require compiled definitions; extents and import bindings remain unproved",
                            "no layout, lifetime, full link or independent review proof",
                            "fresh diagnostic only; fingerprint is not a reusable measurement cache key"]}
