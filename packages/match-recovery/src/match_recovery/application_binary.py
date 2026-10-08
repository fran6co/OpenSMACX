"""Fresh task-scoped application measurements, using the strict shared checker.

No claim stamping, automatic demotion, missing-target inference or seed promotion.
This is a scoped diagnostic, not the full build/ownership/runtime admission gate.
"""

from collections import Counter, defaultdict
from dataclasses import replace
from pathlib import Path
from typing import Callable

from match_recovery.bound_objects import (FunctionBinding, FunctionFragmentBinding,
                                          check_objects)
from match_recovery.generated_source import read_generated
from match_recovery.model import Origin, Result, Span, digest
from match_recovery.object_source import (emitted_object_symbols, object_targets,
                                          require_disjoint_object_symbols,
                                          read_objects, referenced_object_units)
from .literal_source import read_literals
from .literal_data import check_literals
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.tasks import TaskStore
from match_recovery.ownership_source import project_view
from match_recovery.generated_binary import check_generated, generated_coverage
from .compiler import Compiler
from .measurement import Measurement
from .import_source import read_imports, import_targets
from .shared_source import read_shared, referenced_shared_units, validate_shared
from .shared_binary import check_shared


def select_bindings(view: dict, selected: frozenset[str], component_targets: dict | None = None) -> tuple[list, list, dict]:
    """Require a single product definition; archived interpretations remain seeds.

    Freshly verified toolchain components may publish relocation targets for
    their reserved names; the annotation alone never does, and a genuine
    product conflict keeps refusing resolution.
    """
    bindings, refused, targets = [], [], defaultdict(set)
    ambiguous = set()
    reserved = defaultdict(set)
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
            for item in candidates:
                if item["component"] is not None:
                    reserved[item["symbol"]].add(entity["origin"]["address"])
            if (candidate is not None and candidate["component"] is None
                    and candidate.get("function_fragment") is None):
                # Exact source spelling only. Ambiguous external names are
                # removed below; static definitions never become external COFF
                # symbols and so cannot borrow this table during resolution.
                targets[candidate["symbol"]].add(entity["origin"]["address"])
            if key not in selected:
                continue
            if candidate is not None and candidate.get("function_fragment") is not None:
                # This identity is verified as a slice of its parent COMDAT,
                # never selected as an independent COFF function symbol.
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
    product_ambiguous = set(ambiguous)
    ambiguous.update(reserved)
    accepted = []
    for binding in bindings:
        if binding.symbol in ambiguous:
            refused.append({"entity_id": binding.origin.entity_id, "result": Result.UNKNOWN,
                            "symbol": binding.symbol, "reason": "ambiguous source symbol binding"})
        else:
            accepted.append(binding)
    externals = {symbol: next(iter(addresses)) for symbol, addresses in targets.items()
                 if symbol not in ambiguous}
    if component_targets:
        for symbol, address in component_targets.items():
            if symbol in product_ambiguous:
                continue
            if symbol in externals:
                if externals[symbol] != address:
                    # A verified component origin contradicts the product
                    # binding: neither interpretation may resolve the name.
                    del externals[symbol]
                continue
            interpretations = targets.get(symbol, set()) | reserved.get(symbol, set())
            if interpretations and (type(address) is not int or interpretations != {address}):
                # A product or reserved image-address interpretation only
                # resolves when it coincides with the verified origin; an
                # absolute constant shares no address with any of them.
                continue
            externals[symbol] = address
    return accepted, refused, externals


def select_fragment_bindings(view: dict, selected: frozenset[str], bindings: list[FunctionBinding]):
    """Resolve validated source relations into parent support + child slices.

    Parent bindings are expanded to the exact contiguous union. A child-only
    task compiles that parent TU as support but only returns the child's mapped
    result as selected work. Ambiguous/missing source parents remain UNKNOWN.
    """
    entities = {entity["entity_id"]: entity for group in view["groups"]
                for entity in group["entities"]}
    existing = {binding.origin.entity_id: binding for binding in bindings}
    fragments, support, refused = [], set(), []
    relation_rows = []
    untrusted_relation_children = set()
    component_relation_children = set()
    for child_id, entity in entities.items():
        current = [row for row in entity["candidates"] if not row["seed"]]
        relations = [row for row in entity["candidates"]
                     if not row["seed"] and row.get("function_fragment") is not None]
        if relations and (len(current) != 1 or entity["conflicts"]):
            untrusted_relation_children.add(child_id)
        for candidate in relations:
            if candidate.get("component") is not None:
                # A fragment identity cannot bypass the separate component
                # verifier or become a callable dependency binding.
                untrusted_relation_children.add(child_id)
                component_relation_children.add(child_id)
            relation = candidate["function_fragment"]
            relation_rows.append((child_id, entity, candidate, relation))
    expanded = dict(existing)
    relation_counts = Counter(relation.get("parent_address")
                              for _, _, _candidate, relation in relation_rows)
    ambiguous_relations = untrusted_relation_children | {
        child_id for child_id, _, _candidate, relation in relation_rows
        if relation_counts[relation.get("parent_address")] != 1}
    ambiguous_parent_ids = set()
    for child_id, _entity, candidate, relation in relation_rows:
        potential = [(parent_id, parent) for parent_id, parent in entities.items()
                     if parent["origin"]["address"] == relation.get("parent_address")]
        qualified = []
        for parent_id, parent in potential:
            current = [row for row in parent["candidates"] if not row["seed"]]
            if (not parent["conflicts"] and len(current) == 1
                    and current[0].get("function_fragment") is None
                    and current[0]["path"] == candidate["path"]
                    and current[0]["symbol"] == relation.get("parent_symbol")
                    and current[0]["component"] is None):
                qualified.append(parent_id)
        if len(potential) != 1 or len(qualified) != 1:
            ambiguous_relations.add(child_id)
            ambiguous_parent_ids.update(parent_id for parent_id, _ in potential)
    fragment_ids = {child_id for child_id, *_ in relation_rows}
    ambiguous_relations.update(fragment_ids & ambiguous_parent_ids)
    for child_id, _entity, candidate, relation in relation_rows:
        if (relation_counts[relation.get("parent_address")] == 1
                and child_id not in untrusted_relation_children):
            continue
        parent_ids = [parent_id for parent_id, parent in entities.items()
                      if parent["origin"]["address"] == relation.get("parent_address")]
        ambiguous_parent_ids.update(parent_ids)
        if child_id in selected:
            refused.append({"entity_id": child_id, "result": Result.UNKNOWN,
                            "reason": ("component-owned mapping cannot be a non-callable fragment"
                                       if child_id in component_relation_children else
                                       "multiple fragment relations claim one parent extent"),
                            "fragment": True})
        for parent_id in parent_ids:
            if parent_id in selected:
                refused.append({"entity_id": parent_id, "result": Result.UNKNOWN,
                                "reason": "parent extent has multiple competing fragment relations",
                                "fragment_parent": True})
    refused_ids = {row["entity_id"] for row in refused}
    for child_id in ambiguous_relations:
        if child_id in selected and child_id not in refused_ids:
            refused.append({"entity_id": child_id, "result": Result.UNKNOWN,
                            "reason": ("component-owned mapping cannot be a non-callable fragment"
                                       if child_id in component_relation_children else
                                       "fragment relation has an ambiguous or conflicted parent"),
                            "fragment": True})
            refused_ids.add(child_id)
    for parent_id in ambiguous_parent_ids:
        if parent_id in selected and parent_id not in refused_ids:
            refused.append({"entity_id": parent_id, "result": Result.UNKNOWN,
                            "reason": "fragment parent has conflicting source candidates",
                            "fragment_parent": True})
            refused_ids.add(parent_id)
    for parent_id in ambiguous_parent_ids:
        expanded.pop(parent_id, None)
        support.discard(parent_id)
    for child_id, child_entity, child_candidate, relation in relation_rows:
        if child_id in ambiguous_relations:
            continue
        parent_address = relation.get("parent_address")
        parent_symbol = relation.get("parent_symbol")
        child_spans = child_candidate.get("spans", [])
        parents = []
        for parent_id, parent_entity in entities.items():
            if parent_entity["origin"]["address"] != parent_address:
                continue
            current = [row for row in parent_entity["candidates"] if not row["seed"]]
            if parent_entity["conflicts"] or len(current) != 1:
                continue
            candidate = current[0]
            if (candidate.get("function_fragment") is None
                    and candidate["path"] == child_candidate["path"]
                    and candidate["symbol"] == parent_symbol
                    and candidate["component"] is None):
                parents.append((parent_id, parent_entity, candidate))
        is_selected = child_id in selected
        selected_parent_ids = [parent_id for parent_id, parent in entities.items()
                               if parent_id in selected
                               and parent["origin"]["address"] == parent_address]
        parent_selected = bool(selected_parent_ids)
        if not is_selected and not parent_selected:
            continue
        reason = None
        if (len(child_spans) != 1 or len(parents) != 1
                or relation.get("reason") is None or not relation.get("evidence")):
            reason = "fragment relation has no unique same-file parent or contiguous child span"
        if reason is None:
            parent_id, parent_entity, parent_candidate = parents[0]
            parent_spans = parent_candidate.get("spans", [])
            parent_span = parent_spans[0] if parent_spans else None
            child_span = child_spans[0] if child_spans else None
            pstart, pend = ((parent_span["start"], parent_span["end"])
                            if isinstance(parent_span, dict) else parent_span or (None, None))
            child_start, child_end = ((child_span["start"], child_span["end"])
                                      if isinstance(child_span, dict) else child_span or (None, None))
            if (len(parent_spans) != 1 or pstart != parent_entity["origin"]["address"]
                    or pend != child_start or child_end is None or child_start is None
                    or child_end <= child_start):
                reason = "fragment relation does not describe one adjacent parent/child extent"
        if reason is not None:
            if is_selected:
                refused.append({"entity_id": child_id, "result": Result.UNKNOWN,
                                "reason": reason, "fragment": True})
            refused_ids = {row["entity_id"] for row in refused}
            for selected_parent_id in selected_parent_ids:
                expanded.pop(selected_parent_id, None)
                support.discard(selected_parent_id)
                if selected_parent_id not in refused_ids:
                    refused.append({"entity_id": selected_parent_id, "result": Result.UNKNOWN,
                                    "reason": reason, "fragment_parent": True})
            continue
        parent_origin = Origin(**parent_entity["origin"])
        pstart, pend = ((parent_spans[0]["start"], parent_spans[0]["end"])
                        if isinstance(parent_spans[0], dict) else parent_spans[0])
        combined = Span(pstart, child_end)
        parent_binding = expanded.get(parent_id)
        if parent_binding is not None:
            if (parent_binding.unit != parent_candidate["path"]
                    or parent_binding.symbol != parent_symbol):
                expanded.pop(parent_id, None)
                support.discard(parent_id)
                if is_selected:
                    refused.append({"entity_id": child_id, "result": Result.UNKNOWN,
                                    "reason": "selected parent binding disagrees with fragment relation",
                                    "fragment": True})
                refused_ids = {row["entity_id"] for row in refused}
                for selected_parent_id in selected_parent_ids:
                    if selected_parent_id not in refused_ids:
                        refused.append({"entity_id": selected_parent_id, "result": Result.UNKNOWN,
                                        "reason": "selected parent binding disagrees with fragment relation",
                                        "fragment_parent": True})
                continue
            source_spans = parent_binding.source_spans or parent_binding.spans
            expanded[parent_id] = replace(parent_binding, spans=(combined,),
                                          source_spans=source_spans)
        else:
            expanded[parent_id] = FunctionBinding(
                parent_origin, (combined,), parent_symbol, parent_candidate["path"],
                source_spans=(Span(pstart, pend),))
            support.add(parent_id)
        if is_selected:
            fragments.append(FunctionFragmentBinding(
                Origin(**child_entity["origin"]), Span(child_start, child_end),
                parent_id, parent_symbol, Span(pstart, pend), child_candidate["path"]))
    return list(expanded.values()), fragments, support, refused


def check_compiled(bindings, object_bindings, generated_bindings, objects, image, externals, *, errors=None,
                   import_bindings=(), shared_bindings=(), literal_bindings=(), fragments=()):
    """Same strict target resolution for ordinary measurements and source trials."""
    return _compiled_evidence(bindings, object_bindings, generated_bindings, objects, image, externals,
                              errors=errors, import_bindings=import_bindings, shared_bindings=shared_bindings,
                              literal_bindings=literal_bindings, fragments=fragments)[0]


def _compiled_evidence(bindings, object_bindings, generated_bindings, objects, image, externals, *, errors=None,
                       import_bindings=(), shared_bindings=(), literal_bindings=(), fragments=()):
    """Resolve targets from product objects, never from a retention probe."""
    data_targets, checked_objects = object_targets(object_bindings, objects)
    object_symbols = set(require_disjoint_object_symbols(
        object_bindings, set(externals) | {item.symbol for item in bindings},
        {item.symbol for item in generated_bindings}, {item.symbol for item in import_bindings},
        {item.symbol for item in shared_bindings}))
    namespaces = [set(externals) | {item.symbol for item in bindings}, object_symbols,
                  {item.symbol for item in generated_bindings}, {item.symbol for item in import_bindings},
                  {item.symbol for item in shared_bindings}]
    if any(group & other for index, group in enumerate(namespaces) for other in namespaces[:index]):
        raise ValueError("conflicting function/object/generated/import/shared symbol also has another binding")
    imports, checked_imports = import_targets(import_bindings, objects, image)
    active_literals = [item for item in literal_bindings if item.unit in objects or item.unit in (errors or {})]
    literal_targets, checked_literals = check_literals(active_literals, objects, image, errors=errors)
    generated_targets, checked_generated = check_generated(generated_bindings, objects, image, {**externals, **data_targets})
    shared_targets, checked_shared = check_shared(shared_bindings, objects, image,
        {**externals, **data_targets, **generated_targets}, import_targets=imports,
        literal_targets=literal_targets, errors=errors)
    externals = {**externals, **shared_targets}
    if shared_bindings:
        generated_targets, checked_generated = check_generated(generated_bindings, objects, image, {**externals, **data_targets})
    from .object_data import check_initial_data
    bounded = [item for item in object_bindings if item.span is not None
               and (item.unit in objects or item.unit in (errors or {}))]
    initial = check_initial_data(bounded, objects, image, {**externals, **data_targets, **generated_targets, **imports},
                                 literal_targets=literal_targets, errors=errors)
    by_symbol = {item["symbol"]: item for item in initial}
    checked_objects = [by_symbol.pop(row["symbol"], row) for row in checked_objects] + list(by_symbol.values())
    targets = {**externals, **data_targets, **generated_targets}
    rows = check_objects(bindings, objects, image,
                         external_targets=targets,
                         import_targets=imports, literal_targets=literal_targets, errors=errors,
                         fragments=fragments)
    return (rows, checked_objects, checked_generated, checked_imports, checked_shared, checked_literals), targets, imports


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
    literal_bindings = read_literals(root, snapshot["image_sha256"])
    if any(item.span is not None for item in object_bindings):
        flag_sets = tuple(flags + settings.layout_flags for flags in settings.flag_sets)
    generated_bindings = read_generated(root, snapshot["image_sha256"])
    import_bindings = read_imports(root)
    shared_bindings = read_shared(root, snapshot["image_sha256"])
    object_symbols = set(emitted_object_symbols(object_bindings))
    validate_shared(shared_bindings, view, reserved=[*object_symbols,
        *[item.symbol for item in (*generated_bindings, *import_bindings)]])
    from .component_targets import component_targets
    try:
        dependency_targets, component_reports = component_targets(root, snapshot, image_data)
    except ValueError as error:
        raise ValueError(f"component dependency verification refused: {error}") from error
    bindings, refused, externals = select_bindings(view, selected, component_targets=dependency_targets)
    bindings, fragment_bindings, support_entities, fragment_refused = select_fragment_bindings(
        view, selected, bindings)
    refused.extend(fragment_refused)
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
    valid_binding_ids = {item.origin.entity_id for item in bindings}
    active_fragments = []
    for fragment in fragment_bindings:
        if fragment.parent_entity_id not in valid_binding_ids:
            refused.append({"entity_id": fragment.origin.entity_id, "result": Result.UNKNOWN,
                            "reason": "fragment parent support binding is unavailable",
                            "fragment": True})
        else:
            active_fragments.append(fragment)
    fragment_bindings = active_fragments
    require_disjoint_object_symbols(object_bindings,
        set(externals) | {item.symbol for item in bindings},
        {item.symbol for item in generated_bindings}, {item.symbol for item in import_bindings},
        {item.symbol for item in shared_bindings})
    if {item.symbol for item in generated_bindings} & (set(externals) | {item.symbol for item in bindings} | object_symbols):
        raise ValueError("generated symbol also has a function/object binding")
    command_for = command_for if command_for is not None else lambda path: ()
    required_data = [item for item in object_bindings if item.evidence is not None and item.evidence.entity_id in selected]
    required_shared = [item for item in shared_bindings if item.evidence.entity_id in selected]
    required_literals = [item for item in literal_bindings if item.evidence.entity_id in selected]
    units = sorted({item.unit for item in bindings} | {item.unit for item in required_data}
                   | {item.unit for item in required_shared} | {item.unit for item in required_literals})
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
    literal_checks = []
    generated_observations = []
    retained_observations = []
    generated_units = set(units)
    object_cache = {}
    observed_grid = {}
    graph_trial_grid = {}

    def effective(unit, requested):
        suffix = settings.layout_flags if settings.layout_flags and requested[-len(settings.layout_flags):] == settings.layout_flags else ()
        base = requested[:-len(suffix)] if suffix else requested
        return settings.effective_flags(unit, base) + suffix

    def record_graph_trial(unit, unit_flags, graph_flags, outcome):
        rows = graph_trial_grid.setdefault(unit, [])
        receipt = {"flags": list(unit_flags), "graph_flags": list(graph_flags), "outcome": outcome}
        if receipt not in rows:
            rows.append(receipt)

    for flags in flag_sets:
        objects, errors = {}, dict(command_errors)
        effective_by_unit = {}
        for unit, command in commands.items():
            unit_flags = effective(unit, flags)
            effective_by_unit[unit] = list(unit_flags)
            observed_grid.setdefault(unit, [])
            if list(unit_flags) not in observed_grid[unit]:
                observed_grid[unit].append(list(unit_flags))
            try:
                cache_key = (unit, command, unit_flags)
                if cache_key not in object_cache:
                    attempts += 1
                    if compiler is None:
                        compiler = Compiler(root, profile=settings.compiler_profile)
                    try:
                        object_cache[cache_key] = (Coff(compiler(root / unit, command, unit_flags)), None)
                    except (OSError, ValueError) as error:
                        object_cache[cache_key] = (None, str(error))
                obj, error = object_cache[cache_key]
                if error is not None:
                    errors[unit] = error
                    record_graph_trial(unit, unit_flags, flags, "failed")
                else:
                    objects[unit] = obj
                    record_graph_trial(unit, unit_flags, flags, "compiled")
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
                    unit_flags = effective(unit, flags)
                    command = tuple(command_for(root / unit))
                    effective_by_unit[unit] = list(unit_flags)
                    observed_grid.setdefault(unit, [])
                    if list(unit_flags) not in observed_grid[unit]:
                        observed_grid[unit].append(list(unit_flags))
                    cache_key = (unit, command, unit_flags)
                    if cache_key not in object_cache:
                        attempts += 1
                        if compiler is None:
                            compiler = Compiler(root, profile=settings.compiler_profile)
                        try:
                            object_cache[cache_key] = (Coff(compiler(root / unit, command, unit_flags)), None)
                        except (OSError, ValueError) as error:
                            object_cache[cache_key] = (None, str(error))
                    obj, error = object_cache[cache_key]
                    if error is not None:
                        errors[unit] = error
                        record_graph_trial(unit, unit_flags, flags, "failed")
                    else:
                        objects[unit] = obj
                        record_graph_trial(unit, unit_flags, flags, "compiled")
                except (OSError, ValueError) as error:
                    errors[unit] = str(error)
        dependencies.append({"flags": list(flags), "units": sorted(dependency_units),
                             "unit_flag_sets": {unit: effective_by_unit[unit] for unit in sorted(effective_by_unit)},
                             "errors": {unit: errors[unit] for unit in sorted(dependency_units) if unit in errors}})
        generated_units.update(dependency_units)
        active_shared = [item for item in shared_bindings if item.unit in objects or item.unit in errors]
        checked, product_targets, product_imports = _compiled_evidence(
            bindings, object_bindings, generated_bindings, objects, image, externals, errors=errors,
            import_bindings=import_bindings, shared_bindings=active_shared, literal_bindings=literal_bindings,
            fragments=fragment_bindings)
        checked_functions, checked_objects, checked_generated, checked_imports, checked_shared, checked_literals = checked
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
                                                        commands[binding.unit], effective(binding.unit, flags), settings.source.symbol_format)
                attempts += observation["compile_attempts"]
                observation = {**observation, "entity_id": binding.origin.entity_id,
                               "symbol": binding.symbol, "unit": binding.unit,
                               "flags": list(effective(binding.unit, flags)), "graph_flags": list(flags)}
                if probe is not None:
                    # Probe COFF is not the product object: re-establish every
                    # same-TU literal against these exact new emitted bytes.
                    probe_literals, _ = check_literals(
                        [item for item in literal_bindings if item.unit == binding.unit],
                        {binding.unit: probe}, image)
                    observation["measurement"], = check_objects(
                        [binding], {binding.unit: probe}, image,
                        external_targets=product_targets, import_targets=product_imports,
                        literal_targets=probe_literals)
                retained_observations.append(observation)
        def stamped(item):
            unit = item.get("unit")
            return {**item, "flags": list(effective(unit, flags)) if unit is not None else list(flags),
                    "graph_flags": list(flags)}

        shared_checks.extend(stamped(item) for item in checked_shared)
        literal_checks.extend(stamped(item) for item in checked_literals)
        object_checks.extend(stamped(item) for item in checked_objects)
        import_checks.extend(stamped(item) for item in checked_imports)
        generated_checks.extend(stamped(item) for item in checked_generated)
        generated_observations.extend({**item, "flags": list(effective(item["unit"], flags)), "graph_flags": list(flags)}
                                      for item in generated_coverage(generated_bindings, objects))
        for row in checked_functions:
            if row["entity_id"] in support_entities:
                continue
            variants[row["entity_id"]].append({**row, "flags": list(effective(row["unit"], flags)),
                                                "graph_flags": list(flags)})
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
                     | {item["content"] for item in literal_checks}
                     | {item["result"] for item in shared_checks})
    if Result.UNKNOWN in table_results:
        overall = Result.UNKNOWN
    elif Result.FAIL in table_results and overall != Result.UNKNOWN:
        overall = Result.FAIL
    return {"task_id": task_name, "check": "scoped_binary_references", "result": overall,
            "input_fingerprint": digest(before), "functions": rows, "counts": dict(sorted(counts.items())),
            "compile_attempts": attempts, "selected_entities": len(selected), "translation_units": sorted(generated_units),
            "flag_sets": [list(flags) for flags in flag_sets],
            "unit_flag_sets": {unit: [list(profile) for profile in dict.fromkeys(
                tuple(effective(unit, flags)) for flags in flag_sets)]
                for unit in sorted(generated_units)},
            "unit_graph_trials": {unit: [{"flags": list(effective(unit, flags)),
                                           "graph_flags": list(flags)} for flags in flag_sets]
                                  for unit in sorted(generated_units)},
            "unit_graph_receipts": {unit: graph_trial_grid.get(unit, [])
                                    for unit in sorted(generated_units)},
            "component_verification": component_reports,
            "object_definitions": object_checks,
            "literal_definitions": literal_checks,
            "literal_owners": [{"interpretation_id": item.interpretation_id, "unit": item.unit}
                               for item in literal_bindings],
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
