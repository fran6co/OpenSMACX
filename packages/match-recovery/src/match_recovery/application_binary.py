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
from match_recovery.generated_binary import check_generated, generated_coverage
from .compiler import Compiler
from .measurement import Measurement
from .import_source import read_imports, import_targets
from .shared_source import read_shared, referenced_shared_units, validate_shared
from .shared_binary import check_shared


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
            # A dependency annotation identifies work to verify; it is not
            # evidence that the current build uses the specified library code.
            # Reserve these names against product aliases too. Admission needs
            # fresh dependency/link evidence, not a prior component report.
            ambiguous.update(item["symbol"] for item in candidates if item["component"] is not None)
            if candidate is not None and candidate["component"] is None:
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
                   import_bindings=(), shared_bindings=()):
    """Same strict target resolution for ordinary measurements and source trials."""
    return _compiled_evidence(bindings, object_bindings, generated_bindings, objects, image, externals,
                              errors=errors, import_bindings=import_bindings, shared_bindings=shared_bindings)[0]


def _compiled_evidence(bindings, object_bindings, generated_bindings, objects, image, externals, *, errors=None,
                       import_bindings=(), shared_bindings=()):
    """Resolve targets from product objects, never from a retention probe."""
    data_targets, checked_objects = object_targets(object_bindings, objects)
    if {item.symbol for item in shared_bindings} & (set(externals) | {item.symbol for item in bindings}
            | {item.symbol for item in object_bindings} | {item.symbol for item in generated_bindings}
            | {item.symbol for item in import_bindings}):
        raise ValueError("shared symbol also has a function/object/generated/import binding")
    if {item.symbol for item in import_bindings} & (
            set(externals) | {item.symbol for item in object_bindings} | {item.symbol for item in generated_bindings}):
        raise ValueError("import symbol also has a function/object/generated binding")
    imports, checked_imports = import_targets(import_bindings, objects, image)
    generated_targets, checked_generated = check_generated(generated_bindings, objects, image, {**externals, **data_targets})
    shared_targets, checked_shared = check_shared(shared_bindings, objects, image,
        {**externals, **data_targets, **generated_targets}, import_targets=imports, errors=errors)
    externals = {**externals, **shared_targets}
    if shared_bindings:
        generated_targets, checked_generated = check_generated(generated_bindings, objects, image, {**externals, **data_targets})
    from .object_data import check_initial_data
    bounded = [item for item in object_bindings if item.span is not None
               and (item.unit in objects or item.unit in (errors or {}))]
    initial = check_initial_data(bounded, objects, image, {**externals, **data_targets, **generated_targets, **imports}, errors=errors)
    by_symbol = {item["symbol"]: item for item in initial}
    checked_objects = [by_symbol.pop(row["symbol"], row) for row in checked_objects] + list(by_symbol.values())
    targets = {**externals, **data_targets, **generated_targets}
    rows = check_objects(bindings, objects, image,
                         external_targets=targets,
                         import_targets=imports, errors=errors)
    return (rows, checked_objects, checked_generated, checked_imports, checked_shared), targets, imports


def verify_task(root: Path, task_name: str, image_path: Path, *,
                compiler: Callable | None = None, command_for: Callable | None = None,
                debug_types: bool = False, retained_diagnostics: bool = False) -> dict:
    # Joint layout checks need configured debug flags. This is a measured configuration,
    # never an assumption that debug and non-debug objects are interchangeable.
    if type(debug_types) is not bool:
        raise ValueError("debug_types must be a boolean")
    if type(retained_diagnostics) is not bool:
        raise ValueError("retained_diagnostics must be a boolean")
    root = root.resolve()
    settings = Measurement.read(root)
    flag_sets = tuple(flags + (settings.layout_flags if debug_types else ()) for flags in settings.flag_sets)
    store = TaskStore(root)
    task = store.read(task_name)
    decisions = store.decisions(task_name)
    selected, _ = store._scope(task, decisions)
    if not selected:
        raise ValueError("refusing an empty task measurement")
    measurement_options = {"debug_types": debug_types,
                           **({"retained_diagnostics": True} if retained_diagnostics else {})}
    before = settings.snapshot(root, measurement_options)
    view, snapshot = project_view(root)
    if any(subject.image_sha256 != snapshot["image_sha256"] or subject.address_space != "pe:va"
           for subject in task.subjects):
        raise ValueError("task refers to a different original image/address space")
    image_data = image_path.read_bytes()
    image = PeImage(image_data, snapshot["image_sha256"])
    object_bindings = read_objects(root, snapshot["image_sha256"])
    if any(item.span is not None for item in object_bindings):
        flag_sets = tuple(flags + settings.layout_flags for flags in settings.flag_sets)
    generated_bindings = read_generated(root, snapshot["image_sha256"])
    import_bindings = read_imports(root)
    shared_bindings = read_shared(root, snapshot["image_sha256"])
    validate_shared(shared_bindings, view, reserved=[item.symbol for item in (*object_bindings, *generated_bindings, *import_bindings)])
    bindings, refused, externals = select_bindings(view, selected)
    # Build-only definitions are deliberately unfinished. Their emitted RET
    # may match an empty original, but cannot establish task completion until
    # the source body-slot sentinel is deliberately removed. Keep reference
    # identities available to callers; this is not a binding withdrawal.
    placeholders = {(row["entity_id"], row["path"]) for row in snapshot["records"]
                    if row["candidate_kind"] == "source" and row["legacy_state"] == "placeholder"}
    unfinished = [item for item in bindings if (item.origin.entity_id, item.unit) in placeholders]
    refused.extend({"entity_id": item.origin.entity_id, "result": Result.UNKNOWN, "symbol": item.symbol,
                    "reason": "unfinished source placeholder; implement its body before task acceptance"}
                   for item in unfinished)
    bindings = [item for item in bindings if item not in unfinished]
    unsupported = [item for item in bindings if Path(item.unit).suffix not in settings.unit_extensions]
    refused.extend({"entity_id": item.origin.entity_id, "result": Result.UNKNOWN, "symbol": item.symbol,
                    "reason": "source definition is not in a configured translation unit"} for item in unsupported)
    bindings = [item for item in bindings if item not in unsupported]
    if {item.symbol for item in object_bindings} & set(externals):
        raise ValueError("a source symbol is bound as both function and object")
    if {item.symbol for item in generated_bindings} & (set(externals) | {item.symbol for item in object_bindings}):
        raise ValueError("generated symbol also has a function/object binding")
    command_for = command_for if command_for is not None else lambda path: ()
    required_data = [item for item in object_bindings if item.evidence is not None and item.evidence.entity_id in selected]
    required_shared = [item for item in shared_bindings if item.evidence.entity_id in selected]
    units = sorted({item.unit for item in bindings} | {item.unit for item in required_data}
                   | {item.unit for item in required_shared})
    commands, command_errors = {}, {}
    for unit in units:
        try:
            if Path(unit).suffix not in settings.unit_extensions:
                raise ValueError("source owner is not a configured translation unit")
            commands[unit] = tuple(command_for(root / unit))
        except ValueError as error:
            command_errors[unit] = str(error)
    attempts, variants, object_checks = 0, defaultdict(list), []
    generated_checks, dependencies, import_checks, shared_checks = [], [], [], []
    generated_observations = []
    retained_observations = []
    generated_units = set(units)
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
        while pending := (referenced_object_units(object_bindings, objects)
                          | referenced_shared_units(shared_bindings, objects)) - objects.keys() - errors.keys():
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
        generated_units.update(dependency_units)
        active_shared = [item for item in shared_bindings if item.unit in objects or item.unit in errors]
        checked, product_targets, product_imports = _compiled_evidence(
            bindings, object_bindings, generated_bindings, objects, image, externals, errors=errors,
            import_bindings=import_bindings, shared_bindings=active_shared)
        checked_functions, checked_objects, checked_generated, checked_imports, checked_shared = checked
        if retained_diagnostics:
            from .function_witness import retained_definition
            for binding in bindings:
                product = objects.get(binding.unit)
                if product is None:
                    continue  # Failed compilation is not evidence of non-emission.
                exact_symbols = [symbol for symbol in product.symbols.values() if symbol.name == binding.symbol]
                if len(exact_symbols) > 1 or any(
                        symbol.section != 0 or symbol.storage != 2 or symbol.value != 0
                        or symbol.type not in (0, 0x20) or product.auxiliary.get(symbol.index)
                        for symbol in exact_symbols):
                    continue  # Never replace present, common, weak or malformed definitions.
                sources = [row for row in snapshot["records"] if row["candidate_kind"] == "source"
                           and row["entity_id"] == binding.origin.entity_id and row["path"] == binding.unit]
                if len(sources) != 1 or not sources[0]["body_path"]:
                    continue  # Explicit source-owned body binding, not a guessed header/name.
                header = sources[0]["body_path"]
                if (Path(header).suffix in settings.unit_extensions or settings.source.is_seed(header)
                        or Path(header).suffix not in settings.source.extensions
                        or not any(Path(header).is_relative_to(directory) for directory in settings.source.roots)):
                    continue
                probe, observation = retained_definition(root, binding, header, product, compiler,
                                                        commands[binding.unit], flags, settings.source.symbol_format)
                attempts += observation["compile_attempts"]
                observation = {**observation, "entity_id": binding.origin.entity_id,
                               "symbol": binding.symbol, "unit": binding.unit, "flags": list(flags)}
                if probe is not None:
                    observation["measurement"], = check_objects(
                        [binding], {binding.unit: probe}, image,
                        external_targets=product_targets, import_targets=product_imports)
                retained_observations.append(observation)
        shared_checks.extend({**item, "flags": list(flags)} for item in checked_shared)
        object_checks.extend({**item, "flags": list(flags)} for item in checked_objects)
        import_checks.extend({**item, "flags": list(flags)} for item in checked_imports)
        generated_checks.extend({**item, "flags": list(flags)} for item in checked_generated)
        generated_observations.extend({**item, "flags": list(flags)}
                                      for item in generated_coverage(generated_bindings, objects))
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
    if (settings.snapshot(root, measurement_options) != before or image_path.read_bytes() != image_data
            or store.read(task_name) != task or store.decisions(task_name) != decisions):
        raise ValueError("source, build inputs or task decisions changed during measurement; retry")
    counts = Counter(row["result"] for row in rows)
    overall = (Result.UNKNOWN if counts[Result.UNKNOWN] else Result.FAIL if counts[Result.FAIL] else Result.PASS)
    table_results = ({item["content"] for item in generated_checks}
                     | {item["content"] for item in object_checks if "content" in item}
                     | {item["result"] for item in shared_checks})
    if Result.UNKNOWN in table_results:
        overall = Result.UNKNOWN
    elif Result.FAIL in table_results and overall != Result.UNKNOWN:
        overall = Result.FAIL
    return {"task_id": task_name, "check": "scoped_binary_references", "result": overall,
            "input_fingerprint": digest(before), "functions": rows, "counts": dict(sorted(counts.items())),
            "compile_attempts": attempts, "selected_entities": len(selected), "translation_units": units,
            "flag_sets": [list(flags) for flags in flag_sets],
            "object_definitions": object_checks,
            "object_owners": [{"interpretation_id": item.interpretation_id, "unit": item.unit}
                              for item in object_bindings if item.span is not None],
            "object_dependencies": dependencies,
            "shared_functions": shared_checks,
            "shared_owners": [{"interpretation_id": item.interpretation_id, "unit": item.unit} for item in shared_bindings],
            "generated_tables": generated_checks,
            "generated_coverage": generated_observations,
            **({"retained_definitions": retained_observations,
                "retained_definition_policy": "advisory_only; never product emission, restoration or acceptance"}
               if retained_diagnostics else {}),
            "generated_units": sorted(generated_units),
            "import_bindings": import_checks,
            "unresolved_symbols": sorted({relocation["symbol"] for candidates in variants.values()
                                           for row in candidates for relocation in row.get("relocations", [])
                                           if relocation["result"] == Result.UNKNOWN}),
            "scope_complete": False, "ready_for_collection": False, "persisted": False,
            "build_verification": Result.UNKNOWN, "structure_verification": Result.UNKNOWN,
            "runtime_verification": Result.UNKNOWN,
            "limitations": ["task subjects only; dependent callers require a wider admission gate",
                            "unbounded object starts prove no initial data; bounded data needs standalone debug types and source extents",
                            "import slot identities require source DLL/export bindings; candidate DLL selection, ABI and delay imports remain unproved",
                            "no layout, lifetime, full link or independent review proof",
                            "fresh diagnostic only; fingerprint is not a reusable measurement cache key"]}
