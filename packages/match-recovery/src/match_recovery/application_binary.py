"""Fresh task-scoped application measurements, using the strict shared checker.

No claim stamping, automatic demotion, missing-target inference or seed promotion.
This is a scoped diagnostic, not the full build/ownership/runtime admission gate.
"""

from collections import Counter, defaultdict
from dataclasses import replace
import hashlib
from pathlib import Path
from typing import Callable

from match_recovery.bound_objects import (FunctionBinding, FunctionFragmentBinding,
                                          check_objects)
from match_recovery.generated_source import read_generated
from match_recovery.model import Origin, Result, Span, digest
from match_recovery.object_source import (emitted_object_symbols, object_targets, local_object_targets,
                                          require_disjoint_object_symbols,
                                          read_objects, referenced_object_units)
from .literal_source import read_literals
from .associated_source import read_associated
from .literal_data import check_literals
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.tasks import TaskStore
from match_recovery.source import read_file
from match_recovery.ownership_source import project_view
from match_recovery.generated_binary import check_generated, generated_coverage
from .compiler import Compiler
from .measurement import Measurement
from .source_inventory import image_pin
from .import_source import read_imports, import_targets
from .shared_source import read_shared, referenced_shared_units, validate_shared
from .shared_binary import check_shared
from .selected_evidence import select_evidence


def _has_unbound_candidate(entity: dict) -> bool:
    """Pending originals are evidence, never a callable definition or alias."""
    return any(item.get("unbound", False) for item in entity["candidates"])


def _local_function_symbols(view: dict, component_targets: dict | None) -> set[str]:
    """Find only cross-TU duplicate names whose every source owner is ordinary.

    This is a tentative scheduling classification. Physical COFF storage is
    checked later by ``check_objects`` before any local-only binding is used.
    """
    by_symbol = defaultdict(list)
    for group in view["groups"]:
        for entity in group["entities"]:
            candidates = [item for item in entity["candidates"] if not item["seed"]]
            unbound = _has_unbound_candidate(entity)
            candidate = (candidates[0] if len(candidates) == 1
                         and not entity["conflicts"] and not unbound else None)
            ordinary = (candidate is not None and candidate["component"] is None
                        and not candidate.get("context_path")
                        and candidate.get("function_fragment") is None)
            for item in candidates:
                by_symbol[item["symbol"]].append((entity, item, ordinary and item is candidate))

    local_symbols = set()
    component_targets = component_targets or {}
    for symbol, entries in by_symbol.items():
        units = [candidate["path"] for _entity, candidate, eligible in entries]
        addresses = [entity["origin"]["address"] for entity, _candidate, _eligible in entries]
        entity_ids = [entity["entity_id"] for entity, _candidate, _eligible in entries]
        if (len(entries) > 1 and len(set(units)) == len(entries)
                and len(set(addresses)) == len(entries)
                and len(set(entity_ids)) == len(entries)
                and all(eligible for _entity, _candidate, eligible in entries)
                and symbol not in component_targets):
            local_symbols.add(symbol)
    return local_symbols


def select_bindings(view: dict, selected: frozenset[str], component_targets: dict | None = None) -> tuple[list, list, dict]:
    """Require a single product definition; archived interpretations remain seeds.

    Freshly verified toolchain components may publish relocation targets for
    their reserved names; the annotation alone never does, and a genuine
    product conflict keeps refusing resolution.
    """
    bindings, refused, targets = [], [], defaultdict(set)
    local_symbols = _local_function_symbols(view, component_targets)
    ambiguous = set()
    reserved = defaultdict(set)
    found = set()
    unbound_addresses = set()
    for group in view["groups"]:
        for entity in group["entities"]:
            key = entity["entity_id"]
            found.add(key)
            candidates = [item for item in entity["candidates"] if not item["seed"]]
            unbound = _has_unbound_candidate(entity)
            if unbound:
                unbound_addresses.add(entity["origin"]["address"])
            candidate = (candidates[0] if len(candidates) == 1
                         and not entity["conflicts"] and not unbound else None)
            if candidate is None:
                ambiguous.update(item["symbol"] for item in candidates)
            elif candidate.get("context_path") or candidate.get("function_fragment") is not None:
                # These interpretations cannot safely publish a cross-TU
                # external address for a same-named ordinary candidate.
                ambiguous.add(candidate["symbol"])
            # A dependency annotation identifies work to verify; it is not
            # evidence that the current build uses the specified library code.
            # Reserve these names against product aliases too. Admission needs
            # fresh dependency/link evidence, not a prior component report.
            for item in candidates:
                if item["component"] is not None:
                    reserved[item["symbol"]].add(entity["origin"]["address"])
            if (candidate is not None and candidate["component"] is None
                    and not candidate.get("context_path")
                    and candidate.get("function_fragment") is None
                    and candidate["symbol"] not in local_symbols):
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
            if unbound:
                reason = "unbound original identity has no current C++ implementation; review and bind a genuine owner"
            elif candidate is None:
                reason = "requires one unambiguous product definition; seeds are not production source"
            elif candidate["component"] is not None:
                reason = "dependency binding: use component verification, not application recovery"
            elif candidate.get("context_path"):
                reason = "strict application measurement does not yet support compile_context mappings"
            if reason is None:
                try:
                    bindings.append(FunctionBinding(Origin(**entity["origin"]),
                                                     tuple(Span(**span) for span in candidate["spans"]),
                                                     candidate["symbol"], candidate["path"],
                                                     local_only=candidate["symbol"] in local_symbols))
                except ValueError as error:
                    reason = str(error)
            if reason is not None:
                refused.append({"entity_id": key, "address": f'0x{entity["origin"]["address"]:08X}',
                                "result": Result.UNKNOWN, "reason": reason})
    refused.extend({"entity_id": key, "result": Result.UNKNOWN, "reason": "source mapping is missing"}
                   for key in sorted(selected - found))
    ambiguous.update(symbol for symbol, addresses in targets.items() if len(addresses) != 1)
    product_ambiguous = set(ambiguous) | local_symbols
    ambiguous.update(reserved)
    accepted = []
    for binding in bindings:
        if binding.local_only and binding.symbol in local_symbols and binding.symbol not in reserved:
            accepted.append(binding)
        elif binding.symbol in ambiguous or binding.symbol in local_symbols:
            refused.append({"entity_id": binding.origin.entity_id, "result": Result.UNKNOWN,
                            "symbol": binding.symbol, "reason": "ambiguous source symbol binding"})
        else:
            accepted.append(binding)
    externals = {symbol: next(iter(addresses)) for symbol, addresses in targets.items()
                 if symbol not in ambiguous and symbol not in local_symbols}
    if component_targets:
        for symbol, address in component_targets.items():
            if address in unbound_addresses:
                # Even independently measured dependency bytes do not silently
                # resolve a competing pending source interpretation. Retire or
                # migrate that interpretation explicitly before publishing a
                # relocation target for the identity.
                continue
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
    context_relation_children = set()
    context_parent_ids = set()
    for child_id, entity in entities.items():
        current = [row for row in entity["candidates"] if not row["seed"]]
        relations = [row for row in entity["candidates"]
                     if not row["seed"] and row.get("function_fragment") is not None]
        if relations and (len(current) != 1 or entity["conflicts"]
                          or _has_unbound_candidate(entity)):
            untrusted_relation_children.add(child_id)
        for candidate in relations:
            if candidate.get("context_path"):
                untrusted_relation_children.add(child_id)
                context_relation_children.add(child_id)
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
            if len(current) == 1 and current[0].get("context_path"):
                context_relation_children.add(child_id)
                context_parent_ids.add(parent_id)
            if (not parent["conflicts"] and not _has_unbound_candidate(parent)
                    and len(current) == 1
                    and current[0].get("function_fragment") is None
                    and not current[0].get("context_path")
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
                            "reason": ("compile_context fragment mappings are unsupported by strict application measurement"
                                       if child_id in context_relation_children else
                                       "component-owned mapping cannot be a non-callable fragment"
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
                            "reason": ("compile_context fragment mappings are unsupported by strict application measurement"
                                       if child_id in context_relation_children else
                                       "component-owned mapping cannot be a non-callable fragment"
                                       if child_id in component_relation_children else
                                       "fragment relation has an ambiguous or conflicted parent"),
                            "fragment": True})
            refused_ids.add(child_id)
    for parent_id in ambiguous_parent_ids:
        if parent_id in selected and parent_id not in refused_ids:
            refused.append({"entity_id": parent_id, "result": Result.UNKNOWN,
                            "reason": ("compile_context fragment mappings are unsupported by strict application measurement"
                                       if parent_id in context_parent_ids else
                                       "fragment parent has conflicting source candidates"),
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
            if (parent_entity["conflicts"] or len(current) != 1
                    or _has_unbound_candidate(parent_entity)):
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
            # A child-only fragment selection may need a parent solely as
            # support. Do not infer TU-local eligibility here: this phase has
            # no component-target reservations from select_bindings. A
            # duplicate source spelling (including a same-TU duplicate) must
            # therefore remain unresolved unless the parent was already
            # selected and qualified by the ordinary binding path above.
            duplicate_parent_symbol = sum(
                1 for entity in entities.values()
                for row in entity["candidates"]
                if not row["seed"] and row["symbol"] == parent_symbol) > 1
            if duplicate_parent_symbol:
                if is_selected:
                    refused.append({"entity_id": child_id, "result": Result.UNKNOWN,
                                    "reason": "ambiguous fragment support parent symbol",
                                    "fragment": True})
                continue
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
                   import_bindings=(), shared_bindings=(), literal_bindings=(), fragments=(),
                   associated_bindings=()):
    """Same strict target resolution for ordinary measurements and source trials."""
    return _compiled_evidence(bindings, object_bindings, generated_bindings, objects, image, externals,
                              errors=errors, import_bindings=import_bindings, shared_bindings=shared_bindings,
                              literal_bindings=literal_bindings, fragments=fragments,
                              associated_bindings=associated_bindings)[0]


def _compiled_evidence(bindings, object_bindings, generated_bindings, objects, image, externals, *, errors=None,
                       import_bindings=(), shared_bindings=(), literal_bindings=(), fragments=(),
                       associated_bindings=(), selected_object_targets=None,
                       selected_object_contents=None):
    """Resolve targets from product objects, never from a retention probe."""
    target_bindings = (object_bindings if selected_object_targets is None
                       else selected_object_targets)
    content_bindings = (object_bindings if selected_object_contents is None
                        else selected_object_contents)
    data_targets, checked_objects = object_targets(target_bindings, objects)
    local_data_targets, checked_local_objects = local_object_targets(target_bindings, objects)
    local_reports = {(row["unit"], row["symbol"]): row for row in checked_local_objects
                     if row["definition"] == "pass"}
    checked_objects = [local_reports.get((row["unit"], row["symbol"]), row) for row in checked_objects]
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
    bounded = [item for item in content_bindings if item.span is not None
               and (item.unit in objects or item.unit in (errors or {}))]
    initial = check_initial_data(bounded, objects, image, {**externals, **data_targets, **generated_targets, **imports},
                                 literal_targets=literal_targets, errors=errors)
    by_symbol = {item["symbol"]: item for item in initial}
    checked_objects = [by_symbol.pop(row["symbol"], row) for row in checked_objects] + list(by_symbol.values())
    targets = {**externals, **data_targets, **generated_targets}
    rows = check_objects(bindings, objects, image,
                         external_targets=targets,
                         local_data_targets=local_data_targets,
                         import_targets=imports, literal_targets=literal_targets, errors=errors,
                         fragments=fragments, associated_bindings=associated_bindings)
    return (rows, checked_objects, checked_generated, checked_imports, checked_shared, checked_literals), targets, imports


def _verify_selection(root: Path, selected: frozenset[str], image_path: Path, *,
                      compiler: Callable | None = None, command_for: Callable | None = None,
                      flag_sets=None, debug_types: bool = False, retained_diagnostics: bool = False,
                      selection_guard=None, selection_label=None, expected_image_sha256=None,
                      record_evidence: bool = False) -> dict:
    # Joint layout checks need configured debug flags. This is a measured configuration,
    # never an assumption that debug and non-debug objects are interchangeable.
    if type(debug_types) is not bool:
        raise ValueError("debug_types must be a boolean")
    if type(retained_diagnostics) is not bool:
        raise ValueError("retained_diagnostics must be a boolean")
    if type(record_evidence) is not bool:
        raise ValueError("record_evidence must be a boolean")
    root = root.resolve()
    settings = Measurement.read(root)
    explicit_flag_sets = flag_sets is not None
    if flag_sets is None:
        flag_sets = settings.flag_sets
    else:
        if (not isinstance(flag_sets, (tuple, list)) or not flag_sets
                or any(not isinstance(flags, (tuple, list))
                       or any(not isinstance(arg, str) or "\0" in arg for arg in flags)
                       for flags in flag_sets)):
            raise ValueError("explicit native flag sets must be a nonempty array of string arrays")
        flag_sets = tuple(tuple(flags) for flags in flag_sets)
    flag_sets = tuple(flags + (settings.layout_flags if debug_types else ()) for flags in flag_sets)
    if not selected:
        raise ValueError("refusing an empty native selection")
    measurement_options = {"debug_types": debug_types,
                           **({"retained_diagnostics": True} if retained_diagnostics else {})}
    before = settings.snapshot(root, measurement_options)
    view, snapshot = project_view(root)
    if expected_image_sha256 is not None and expected_image_sha256 != snapshot["image_sha256"]:
        raise ValueError("records refer to a different configured original image")
    image_data = image_path.read_bytes()
    image = PeImage(image_data, snapshot["image_sha256"])
    object_bindings = read_objects(root, snapshot["image_sha256"])
    literal_bindings = read_literals(root, snapshot["image_sha256"])
    associated_bindings = read_associated(root, snapshot["image_sha256"])
    if any(item.span is not None for item in object_bindings) and not explicit_flag_sets:
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
    if record_evidence:
        source_extract_errors = {row["entity_id"]: row["extract_error"] for row in snapshot["records"]
                                 if row["entity_id"] in selected and row.get("extract_error")}
        unextractable = [item for item in bindings if item.origin.entity_id in source_extract_errors]
        refused.extend({"entity_id": item.origin.entity_id, "result": Result.UNKNOWN,
                        "symbol": item.symbol,
                        "reason": "source definition could not be extracted: "
                                  + source_extract_errors[item.origin.entity_id]}
                       for item in unextractable)
        bindings = [item for item in bindings if item not in unextractable]
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
    initial_evidence = (select_evidence(bindings, selected, {}, object_bindings,
                                        generated_bindings, import_bindings,
                                        shared_bindings, literal_bindings,
                                        unit_extensions=settings.unit_extensions)
                       if record_evidence else None)
    if record_evidence:
        units = sorted(initial_evidence.units)
    else:
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
    evidence_checks = []
    selection_trials = []
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
        dependency_units = set()
        if record_evidence:
            # Record verification follows only dependencies named by the
            # selected COFF bodies, associated data and explicitly selected
            # evidence. Task verification below retains the full application
            # object/shared graph.
            evidence = select_evidence(bindings, selected, objects, object_bindings,
                                       generated_bindings, import_bindings,
                                       shared_bindings, literal_bindings,
                                       unit_extensions=settings.unit_extensions)
            while pending := evidence.units - objects.keys() - errors.keys():
                dependency_units.update(pending)
                for unit in sorted(pending):
                    try:
                        if Path(unit).suffix not in settings.unit_extensions:
                            raise ValueError("selected evidence owner is not a configured translation unit")
                        unit_flags = effective(unit, flags)
                        command = commands.get(unit)
                        if command is None:
                            command = tuple(command_for(root / unit))
                            commands[unit] = command
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
                evidence = select_evidence(bindings, selected, objects, object_bindings,
                                           generated_bindings, import_bindings,
                                           shared_bindings, literal_bindings,
                                           unit_extensions=settings.unit_extensions)
        else:
            # Full task-scoped behavior: follow every declared object/shared
            # relocation in the selected application object graph.
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
        active_objects = evidence if record_evidence else None
        active_generated = active_objects.generated if active_objects is not None else generated_bindings
        active_imports = active_objects.imports if active_objects is not None else import_bindings
        active_shared = (list(active_objects.shared) if active_objects is not None
                         else [item for item in shared_bindings if item.unit in objects or item.unit in errors])
        active_literals = active_objects.literals if active_objects is not None else literal_bindings
        checked, product_targets, product_imports = _compiled_evidence(
            bindings, object_bindings, active_generated, objects, image, externals, errors=errors,
            import_bindings=active_imports, shared_bindings=active_shared, literal_bindings=active_literals,
            fragments=fragment_bindings, associated_bindings=associated_bindings,
            selected_object_targets=(active_objects.object_targets if active_objects is not None else None),
            selected_object_contents=(active_objects.object_contents if active_objects is not None else None))
        checked_functions, checked_objects, checked_generated, checked_imports, checked_shared, checked_literals = checked
        if active_objects is not None:
            evidence_checks.extend({**row, "flags": list(flags), "graph_flags": list(flags)}
                                   for row in active_objects.unknown)
            function_results = [row.get("result", Result.UNKNOWN) for row in checked_functions
                                if row.get("entity_id") in selected
                                and row.get("entity_id") not in support_entities]
            refused_selected = [row for row in refused if row.get("entity_id") in selected]
            function_rows_by_id = Counter(
                row.get("entity_id") for row in checked_functions
                if row.get("entity_id") in selected and row.get("entity_id") not in support_entities)
            refused_rows_by_id = Counter(row.get("entity_id") for row in refused_selected)
            coverage_unknown = []
            for entity_id in sorted(selected):
                count = function_rows_by_id[entity_id] + refused_rows_by_id[entity_id]
                if count != 1:
                    coverage_unknown.append({
                        "result": Result.UNKNOWN, "entity_id": entity_id,
                        "reason": ("selected source identity has no checked or refused result"
                                   if count == 0 else
                                   "selected source identity has multiple checked/refused results"),
                    })
            refused_results = [Result.UNKNOWN for _row in refused_selected]
            evidence_results = [row.get("definition", Result.UNKNOWN) for row in checked_objects]
            evidence_results.extend(row.get("content", Result.UNKNOWN) for row in checked_objects
                                    if "content" in row)
            evidence_results.extend(row.get("content", Result.UNKNOWN) for row in checked_generated)
            evidence_results.extend(Result.PASS if row.get("identity") == "pass" else Result.UNKNOWN
                                    for row in checked_imports)
            evidence_results.extend(row.get("result", Result.UNKNOWN) for row in checked_shared)
            evidence_results.extend(row.get("content", Result.UNKNOWN) for row in checked_literals)
            evidence_results.extend(row.get("result", Result.UNKNOWN) for row in active_objects.unknown)
            evidence_results.extend(refused_results)
            evidence_results.extend(row["result"] for row in coverage_unknown)
            evidence_checks.extend({**row, "flags": list(flags), "graph_flags": list(flags)}
                                   for row in coverage_unknown)
            evidence_checks.extend({**row, "result": Result.UNKNOWN,
                                    "flags": list(flags), "graph_flags": list(flags)}
                                   for row in refused_selected)
            trial_values = [*function_results, *evidence_results]
            trial_result = (Result.UNKNOWN if Result.UNKNOWN in trial_values else
                            Result.FAIL if Result.FAIL in trial_values else Result.PASS)
            selection_trials.append({"flags": list(flags), "result": trial_result,
                                     "function_results": function_results,
                                     "evidence_results": evidence_results,
                                     "unknown": [*active_objects.unknown, *coverage_unknown,
                                                 *({**row, "result": Result.UNKNOWN}
                                                   for row in refused_selected)]})
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
                        literal_targets=probe_literals, associated_bindings=associated_bindings)
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
                                      for item in generated_coverage(active_generated, objects))
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
    if (settings.snapshot(root, measurement_options) != before or image_path.read_bytes() != image_data):
        raise ValueError("source, build inputs or image changed during measurement; retry")
    if selection_guard is not None:
        selection_guard()
    counts = Counter(row["result"] for row in rows)
    if record_evidence:
        trial_results = {item["result"] for item in selection_trials}
        overall = (Result.PASS if Result.PASS in trial_results else
                   Result.UNKNOWN if Result.UNKNOWN in trial_results else Result.FAIL)
    else:
        overall = (Result.UNKNOWN if counts[Result.UNKNOWN] else Result.FAIL if counts[Result.FAIL] else Result.PASS)
        table_results = ({item["content"] for item in generated_checks}
                         | {item["content"] for item in object_checks if "content" in item}
                         | {item["content"] for item in literal_checks}
                         | {item["result"] for item in shared_checks})
        if Result.UNKNOWN in table_results:
            overall = Result.UNKNOWN
        elif Result.FAIL in table_results and overall != Result.UNKNOWN:
            overall = Result.FAIL
    return {"task_id": selection_label, "check": "scoped_binary_references", "result": overall,
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
            **({"selected_evidence": {"checks": evidence_checks,
                                      "trials": selection_trials}}
               if record_evidence else {}),
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
            "limitations": ["selected subjects only; dependent callers require a wider admission gate",
                            "unbounded object starts prove no initial data; bounded data needs standalone debug types and source extents",
                            "import slot identities require source DLL/export bindings; candidate DLL selection, ABI and delay imports remain unproved",
                            "no layout, lifetime, full link or independent review proof",
                            "fresh diagnostic only; fingerprint is not a reusable measurement cache key"]}


def _current_source_records(root: Path, settings: Measurement):
    records = []
    for path in settings.source.files(root):
        records.extend(read_file(path))
    return records


def verify_records(root: Path, records, image_path: Path, *, flag_sets,
                   compiler: Callable | None = None, command_for: Callable | None = None,
                   retained_diagnostics: bool = False) -> dict:
    """Strictly verify current source records without creating a task or ledger.

    The caller supplies the actual invocation's flag trials. Every selected
    record must equal a freshly parsed annotation from a configured source
    file, and the image must match the configured pin. This is the shared
    native path used for source-associated code tails.
    """
    root = root.resolve()
    settings = Measurement.read(root)
    if settings.source.address_space != "pe:va":
        raise ValueError("record verification requires the configured PE virtual-address space")
    pin = image_pin(root, settings.source)
    image_path = image_path if image_path.is_absolute() else root / image_path
    image_path = image_path.resolve()
    actual = hashlib.sha256(image_path.read_bytes()).hexdigest()
    if actual != pin:
        raise ValueError("record verification requires the configured pinned original image")
    selected_records = tuple(records)
    if not selected_records:
        raise ValueError("record verification requires an explicit nonempty selection")

    def fresh_records():
        current = _current_source_records(root, settings)
        by_key = defaultdict(list)
        for row in current:
            by_key[(row.path.resolve(), row.address, row.line)].append(row)
        selected_ids = []
        for record in selected_records:
            try:
                relative = record.path.resolve().relative_to(root).as_posix()
                key = (record.path.resolve(), record.address, record.line)
            except (AttributeError, OSError, ValueError) as error:
                raise ValueError("selected record is not a configured current source annotation") from error
            if (relative not in {path.relative_to(root).as_posix() for path in settings.source.files(root)}
                    or len(by_key.get(key, ())) != 1 or by_key[key][0] != record):
                raise ValueError("selected record differs from the current configured source annotation")
            selected_ids.append(Origin(pin, settings.source.address_space, record.address).entity_id)
        if len(set(selected_ids)) != len(selected_ids):
            raise ValueError("record verification selection contains duplicate original identities")
        return frozenset(selected_ids)

    selected = fresh_records()

    def guard():
        if fresh_records() != selected or image_pin(root, settings.source) != pin:
            raise ValueError("source records or configured image pin changed during verification; retry")

    return _verify_selection(root, selected, image_path, compiler=compiler,
                              command_for=command_for, flag_sets=flag_sets,
                              retained_diagnostics=retained_diagnostics,
                              selection_guard=guard, selection_label=None,
                              expected_image_sha256=pin, record_evidence=True)


def verify_task(root: Path, task_name: str, image_path: Path, *,
                compiler: Callable | None = None, command_for: Callable | None = None,
                debug_types: bool = False, retained_diagnostics: bool = False) -> dict:
    """Verify task-scoped native selection, retaining task freshness guards."""
    root = root.resolve()
    settings = Measurement.read(root)
    pin = image_pin(root, settings.source)
    store = TaskStore(root)
    task = store.read(task_name)
    decisions = store.decisions(task_name)
    selected, _ = store._scope(task, decisions)
    if not selected:
        raise ValueError("refusing an empty task measurement")
    if any(subject.image_sha256 != pin or subject.address_space != "pe:va"
           for subject in task.subjects):
        raise ValueError("task refers to a different original image/address space")

    def guard():
        if store.read(task_name) != task or store.decisions(task_name) != decisions:
            raise ValueError("task scope or decisions changed during measurement; retry")

    return _verify_selection(root, selected, image_path, compiler=compiler,
                              command_for=command_for, debug_types=debug_types,
                              retained_diagnostics=retained_diagnostics,
                              selection_guard=guard, selection_label=task_name,
                              expected_image_sha256=pin)
