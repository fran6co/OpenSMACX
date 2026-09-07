"""Fresh task-scoped application measurements, using the strict shared checker.

No claim stamping, automatic demotion, missing-target inference or seed promotion.
This is a scoped diagnostic, not the full build/ownership/runtime admission gate.
"""

from collections import Counter, defaultdict
from pathlib import Path
from typing import Callable

from match_recovery.bound_objects import FunctionBinding, check_objects
from match_recovery.generated_source import read_generated
from match_recovery.model import Origin, Result, Span, digest
from match_recovery.object_source import read_objects, object_targets, referenced_object_units
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.tasks import TaskStore
from match_recovery.ownership_source import project_view
from match_recovery.generated_binary import check_generated
from .compiler import Compiler
from .measurement import Measurement
from .import_source import read_imports, import_targets


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


def check_compiled(bindings, object_bindings, generated_bindings, objects, image, externals, *, errors=None,
                   import_bindings=()):
    """Same strict target resolution for ordinary measurements and source trials."""
    data_targets, checked_objects = object_targets(object_bindings, objects)
    if {item.symbol for item in import_bindings} & (
            set(externals) | {item.symbol for item in object_bindings} | {item.symbol for item in generated_bindings}):
        raise ValueError("import symbol also has a function/object/generated binding")
    imports, checked_imports = import_targets(import_bindings, objects, image)
    generated_targets, checked_generated = check_generated(generated_bindings, objects, image, {**externals, **data_targets})
    rows = check_objects(bindings, objects, image,
                         external_targets={**externals, **data_targets, **generated_targets},
                         import_targets=imports, errors=errors)
    return rows, checked_objects, checked_generated, checked_imports


def verify_task(root: Path, task_name: str, image_path: Path, *,
                compiler: Callable | None = None, command_for: Callable | None = None,
                debug_types: bool = False) -> dict:
    # Joint layout checks need configured debug flags. This is a measured configuration,
    # never an assumption that debug and non-debug objects are interchangeable.
    if type(debug_types) is not bool:
        raise ValueError("debug_types must be a boolean")
    root = root.resolve()
    settings = Measurement.read(root)
    flag_sets = tuple(flags + (settings.layout_flags if debug_types else ()) for flags in settings.flag_sets)
    store = TaskStore(root)
    task = store.read(task_name)
    decisions = store.decisions(task_name)
    selected, _ = store._scope(task, decisions)
    if not selected:
        raise ValueError("refusing an empty task measurement")
    before = settings.snapshot(root, {"flags": flag_sets})
    view, snapshot = project_view(root)
    if any(subject.image_sha256 != snapshot["image_sha256"] or subject.address_space != "pe:va"
           for subject in task.subjects):
        raise ValueError("task refers to a different original image/address space")
    image_data = image_path.read_bytes()
    image = PeImage(image_data, snapshot["image_sha256"])
    object_bindings = read_objects(root, snapshot["image_sha256"])
    generated_bindings = read_generated(root, snapshot["image_sha256"])
    import_bindings = read_imports(root)
    bindings, refused, externals = select_bindings(view, selected)
    unsupported = [item for item in bindings if Path(item.unit).suffix not in settings.unit_extensions]
    refused.extend({"entity_id": item.origin.entity_id, "result": Result.UNKNOWN, "symbol": item.symbol,
                    "reason": "source definition is not in a configured translation unit"} for item in unsupported)
    bindings = [item for item in bindings if item not in unsupported]
    if {item.symbol for item in object_bindings} & set(externals):
        raise ValueError("a source symbol is bound as both function and object")
    if {item.symbol for item in generated_bindings} & (set(externals) | {item.symbol for item in object_bindings}):
        raise ValueError("generated symbol also has a function/object binding")
    command_for = command_for if command_for is not None else lambda path: ()
    units = sorted({item.unit for item in bindings})
    commands, command_errors = {}, {}
    for unit in units:
        try:
            commands[unit] = tuple(command_for(root / unit))
        except ValueError as error:
            command_errors[unit] = str(error)
    attempts, variants, object_checks = 0, defaultdict(list), []
    generated_checks, dependencies, import_checks = [], [], []
    for flags in flag_sets:
        objects, errors = {}, dict(command_errors)
        for unit, command in commands.items():
            attempts += 1
            try:
                if compiler is None:
                    compiler = Compiler(root, profile=settings.compiler_profile)
                objects[unit] = Coff(compiler(root / unit, command, flags))
            except (OSError, ValueError) as error:
                errors[unit] = str(error)
        # A real global may live in a TU without any mapped task function.
        # Follow source-declared ownership, never resolve it from the original
        # operand or reuse an object compiled under another flag trial.
        dependency_units = set()
        while pending := referenced_object_units(object_bindings, objects) - objects.keys() - errors.keys():
            dependency_units.update(pending)
            for unit in sorted(pending):
                try:
                    if Path(unit).suffix not in settings.unit_extensions:
                        raise ValueError("object owner is not a configured translation unit")
                    command = tuple(command_for(root / unit))
                    attempts += 1
                    if compiler is None:
                        compiler = Compiler(root, profile=settings.compiler_profile)
                    objects[unit] = Coff(compiler(root / unit, command, flags))
                except (OSError, ValueError) as error:
                    errors[unit] = str(error)
        dependencies.append({"flags": list(flags), "units": sorted(dependency_units),
                             "errors": {unit: errors[unit] for unit in sorted(dependency_units) if unit in errors}})
        checked_functions, checked_objects, checked_generated, checked_imports = check_compiled(
            bindings, object_bindings, generated_bindings, objects, image, externals, errors=errors,
            import_bindings=import_bindings)
        object_checks.extend({**item, "flags": list(flags)} for item in checked_objects)
        import_checks.extend({**item, "flags": list(flags)} for item in checked_imports)
        generated_checks.extend({**item, "flags": list(flags)} for item in checked_generated)
        for row in checked_functions:
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
    if (settings.snapshot(root, {"flags": flag_sets}) != before or image_path.read_bytes() != image_data
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
            "flag_sets": [list(flags) for flags in flag_sets],
            "object_definitions": object_checks,
            "object_dependencies": dependencies,
            "generated_tables": generated_checks,
            "import_bindings": import_checks,
            "unresolved_symbols": sorted({relocation["symbol"] for candidates in variants.values()
                                           for row in candidates for relocation in row.get("relocations", [])
                                           if relocation["result"] == Result.UNKNOWN}),
            "scope_complete": False, "ready_for_collection": False, "persisted": False,
            "build_verification": Result.UNKNOWN, "structure_verification": Result.UNKNOWN,
            "runtime_verification": Result.UNKNOWN,
            "limitations": ["task subjects only; dependent callers require a wider admission gate",
                            "object starts require compiled definitions; extents remain unproved",
                            "import slot identities require source DLL/export bindings; candidate DLL selection, ABI and delay imports remain unproved",
                            "no layout, lifetime, full link or independent review proof",
                            "fresh diagnostic only; fingerprint is not a reusable measurement cache key"]}
