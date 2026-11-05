"""Fresh task-scoped application measurements, using the strict shared checker.

No claim stamping, automatic demotion, missing-target inference or seed promotion.
This is a scoped diagnostic, not the full build/ownership/runtime admission gate.
"""

from collections import Counter, defaultdict
from dataclasses import dataclass, replace
import hashlib
from pathlib import Path, PurePosixPath, PureWindowsPath
from typing import Callable

from match_recovery.bound_objects import (FunctionBinding, FunctionFragmentBinding,
                                          check_objects)
from match_recovery.generated_source import read_generated
from match_recovery.model import Origin, Result, Span, digest
from match_recovery.object_source import (emitted_object_symbols, object_targets, local_object_targets,
                                          require_disjoint_object_symbols,
                                          read_objects, referenced_object_units)
from .literal_source import read_literals, validate_literals
from .associated_source import read_associated
from .literal_data import check_literals
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.tasks import TaskStore
from match_recovery.source import read_file
from match_recovery.ownership_source import project_view
from match_recovery.generated_binary import check_generated, generated_coverage
from .compiler import Compiler
from .measurement import Measurement
from .source_inventory import (SourceInputs, assert_snapshot_current, image_pin,
                               source_snapshot)
from .import_source import read_imports, import_targets
from .shared_source import (read_shared_bindings, referenced_shared_units,
                            resolve_shared_contexts, validate_shared_bindings)
from .shared_binary import check_shared
from .selected_evidence import select_evidence
from .function_contexts import (ContextFunctionTarget, qualify_context_targets,
                                referenced_context_units)


@dataclass(frozen=True)
class CompiledSelectionObservation:
    """Body measurements and unresolved required evidence for one selection.

    This is an observation only. In particular, a passing body row does not
    establish that all source-owned references, data, or shared copies pass.
    """

    function_rows: tuple[dict, ...]
    required_evidence_checks: tuple[dict, ...]
    required_evidence_unknowns: tuple[dict, ...]
    body_result: str


def _validate_compiled_namespaces(bindings, object_bindings, generated_bindings,
                                  import_bindings, shared_bindings, externals=(),
                                  literal_bindings=()):
    """Validate every declared name before any selected-evidence filtering."""
    declared_functions = set(externals) | {item.symbol for item in bindings}
    object_symbols = set(require_disjoint_object_symbols(
        object_bindings, declared_functions,
        {item.symbol for item in generated_bindings},
        {item.symbol for item in import_bindings},
        {item.symbol for item in shared_bindings},
        {item.symbol for item in literal_bindings}))
    namespaces = [declared_functions, object_symbols,
                  {item.symbol for item in generated_bindings},
                  {item.symbol for item in import_bindings},
                  {item.symbol for item in shared_bindings}]
    if any(group & other for index, group in enumerate(namespaces) for other in namespaces[:index]):
        raise ValueError("conflicting function/object/generated/import/shared symbol also has another binding")
    literal_symbols = {item.symbol for item in literal_bindings}
    if literal_symbols & set.union(*namespaces):
        raise ValueError("literal target also has a global function/object/import binding")


def observe_compiled_selection(bindings, selected, object_bindings, generated_bindings,
                               objects, image, externals, *, errors=None,
                               import_bindings=(), shared_bindings=(), literal_bindings=(),
                               fragments=(), associated_bindings=(), unit_extensions=None,
                               _weak_scope=None):
    """Observe selected function rows and their complete typed evidence closure.

    The returned object deliberately has no aggregate acceptance/result field.
    ``body_result`` summarizes only selected function rows. All required
    evidence outcomes, including failures, remain in ``required_evidence_checks``;
    ``required_evidence_unknowns`` contains every unmeasurable obligation.
    All name conflicts are validated against the full declared namespace before
    the selected closure is computed. The closure is recomputed for every call.
    """
    selected = frozenset(selected)
    if not selected:
        raise ValueError("refusing an empty compiled selection")
    bindings = tuple(bindings)
    object_bindings = tuple(object_bindings)
    generated_bindings = tuple(generated_bindings)
    import_bindings = tuple(import_bindings)
    shared_bindings = tuple(shared_bindings)
    literal_bindings = tuple(literal_bindings)
    literal_bindings = validate_literals(literal_bindings)
    _validate_compiled_namespaces(bindings, object_bindings, generated_bindings,
                                  import_bindings, shared_bindings, externals,
                                  literal_bindings=literal_bindings)

    selected_fragments = tuple(item for item in fragments if item.origin.entity_id in selected)
    parent_ids = {item.parent_entity_id for item in selected_fragments}
    evidence_roots = selected | parent_ids
    evidence = select_evidence(bindings, evidence_roots, objects, object_bindings,
                               generated_bindings, import_bindings, shared_bindings,
                               literal_bindings, unit_extensions=unit_extensions,
                               context_targets=externals)
    measurement_ids = selected | parent_ids
    selected_bindings = tuple(item for item in bindings
                              if item.origin.entity_id in measurement_ids)
    checked, _targets, _imports = _compiled_evidence(
        selected_bindings, object_bindings, evidence.generated, objects, image, externals,
        errors=errors, import_bindings=evidence.imports, shared_bindings=evidence.shared,
        literal_bindings=evidence.literals, fragments=selected_fragments,
        associated_bindings=associated_bindings,
        function_support_bindings=tuple(item for item in bindings
                                        if item.origin.entity_id not in measurement_ids),
        selected_object_targets=evidence.object_targets,
        selected_object_contents=evidence.object_contents,
        _weak_scope=_weak_scope)
    function_rows, object_rows, generated_rows, import_rows, shared_rows, literal_rows = checked
    rows_by_id = {row.get("entity_id"): row for row in function_rows
                  if row.get("entity_id") in selected}
    function_rows = [rows_by_id[key] for key in sorted(rows_by_id)]
    covered = set(rows_by_id)
    function_rows.extend({"entity_id": entity_id, "result": Result.UNKNOWN,
                          "reason": "selected source identity has no checked function row"}
                         for entity_id in sorted(selected - covered))

    checks = list(evidence.unknown)
    checks.extend(object_rows)
    checks.extend(generated_rows)
    checks.extend(import_rows)
    checks.extend(shared_rows)
    checks.extend(literal_rows)

    def evidence_outcome(row):
        values = []
        if "definition" in row:
            values.append(row["definition"])
        if "content" in row:
            values.append(row["content"])
        if "identity" in row:
            values.append(Result.PASS if row["identity"] == "pass" else Result.UNKNOWN)
        if "result" in row:
            values.append(row["result"])
        return values or [Result.UNKNOWN]

    unresolved = [row for row in checks
                  if Result.UNKNOWN in evidence_outcome(row)]
    body_values = [row.get("result", Result.UNKNOWN) for row in function_rows]
    body_result = (Result.UNKNOWN if Result.UNKNOWN in body_values else
                   Result.FAIL if Result.FAIL in body_values else Result.PASS)
    return CompiledSelectionObservation(tuple(function_rows), tuple(checks),
                                        tuple(unresolved), body_result)


def validated_context_units(root, snapshot, settings):
    """Re-evaluate explicit source contexts; invalid names remain refused.

    The mapping gate reports detailed invalid-context findings independently.
    This set authorizes no provider search or linked-provider inference.
    """
    from .source_selection import configured_compilation_context

    units = set()
    for name in sorted({row.get("context_path", "") for row in snapshot["records"]} - {""}):
        try:
            path = configured_compilation_context(root, name, settings)
        except (OSError, ValueError):
            continue
        units.add(path.relative_to(root).as_posix())
    return frozenset(units)


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


def select_bindings(view: dict, selected: frozenset[str], component_targets: dict | None = None,
                    *, context_units: frozenset[str] | None = None) -> tuple[list, list, dict]:
    """Require a single product definition; archived interpretations remain seeds.

    Freshly verified toolchain components may publish relocation targets for
    their reserved names; the annotation alone never does, and a genuine
    product conflict keeps refusing resolution.
    """
    bindings, refused, targets = [], [], defaultdict(set)
    context_targets = {}
    context_units = frozenset(context_units or ())
    # Context/fragment composition has no supported independent body boundary.
    # Quarantine the parent before constructing external targets: the later
    # fragment pass may refuse a parent body but cannot withdraw a published
    # target from callers or data initializers.
    fragment_parents = {
        (relation["parent_address"], relation["parent_symbol"])
        for group in view["groups"] for entity in group["entities"]
        for item in entity["candidates"] if not item["seed"]
        if (relation := item.get("function_fragment")) is not None
    }
    local_symbols = _local_function_symbols(view, component_targets)
    ambiguous = set()
    product_conflicts = set()
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
            context_supported = (candidate is not None and candidate["component"] is None
                                 and candidate.get("function_fragment") is None
                                 and (entity["origin"]["address"], candidate["symbol"]) not in fragment_parents
                                 and bool(candidate.get("context_path"))
                                 and candidate["context_path"] in context_units)
            # Size-free entry aliases in one dependency companion do not
            # compete with product definitions. They remain reserved below;
            # only freshly verified component targets can resolve them.
            dependency_aliases = (
                len(candidates) > 1 and not entity["conflicts"] and not unbound
                and len({item["symbol"] for item in candidates}) == len(candidates)
                and all(item["component"] is not None and not item.get("spans")
                        and not item.get("context_path") and item.get("function_fragment") is None
                        for item in candidates)
                and len({(item["component"], item["path"]) for item in candidates}) == 1)
            if candidate is None:
                ambiguous.update(item["symbol"] for item in candidates)
                if not dependency_aliases:
                    product_conflicts.update(item["symbol"] for item in candidates)
            elif (candidate.get("context_path") and not context_supported
                  or candidate.get("function_fragment") is not None):
                # These interpretations cannot safely publish a cross-TU
                # external address for a same-named ordinary candidate.
                ambiguous.add(candidate["symbol"])
                product_conflicts.add(candidate["symbol"])
            # A dependency annotation identifies work to verify; it is not
            # evidence that the current build uses the specified library code.
            # Reserve these names against product aliases too. Admission needs
            # fresh dependency/link evidence, not a prior component report.
            for item in candidates:
                if item["component"] is not None:
                    reserved[item["symbol"]].add(entity["origin"]["address"])
            if (candidate is not None and candidate["component"] is None
                    and (not candidate.get("context_path") or context_supported)
                    and candidate.get("function_fragment") is None
                    and candidate["symbol"] not in local_symbols):
                # Exact source spelling only. Ambiguous external names are
                # removed below; static definitions never become external COFF
                # symbols and so cannot borrow this table during resolution.
                targets[candidate["symbol"]].add(entity["origin"]["address"])
                if context_supported:
                    context_targets[candidate["symbol"]] = ContextFunctionTarget(
                        Origin(**entity["origin"]), candidate["symbol"],
                        candidate["context_path"], candidate["path"])
            if key not in selected:
                continue
            if candidate is not None and candidate.get("function_fragment") is not None:
                # This identity is verified as a slice of its parent COMDAT,
                # never selected as an independent COFF function symbol.
                continue
            reason = None
            if unbound:
                reason = "unbound original identity has no current C++ implementation; review and bind a genuine owner"
            elif dependency_aliases:
                reason = "dependency binding: use component verification, not application recovery"
            elif candidate is None:
                reason = "requires one unambiguous product definition; seeds are not production source"
            elif candidate["component"] is not None:
                reason = "dependency binding: use component verification, not application recovery"
            elif candidate.get("context_path") and not context_supported:
                reason = "compile_context requires a validated configured product translation unit"
            if reason is None:
                try:
                    bindings.append(FunctionBinding(Origin(**entity["origin"]),
                                                     tuple(Span(**span) for span in candidate["spans"]),
                                                     candidate["symbol"],
                                                     candidate["context_path"] if context_supported else candidate["path"],
                                                     local_only=candidate["symbol"] in local_symbols,
                                                     source_unit=candidate["path"] if context_supported else ""))
                except ValueError as error:
                    reason = str(error)
            if reason is not None:
                refused.append({"entity_id": key, "address": f'0x{entity["origin"]["address"]:08X}',
                                "result": Result.UNKNOWN, "reason": reason})
    refused.extend({"entity_id": key, "result": Result.UNKNOWN, "reason": "source mapping is missing"}
                   for key in sorted(selected - found))
    conflicting_targets = {symbol for symbol, addresses in targets.items() if len(addresses) != 1}
    ambiguous.update(conflicting_targets)
    product_ambiguous = product_conflicts | conflicting_targets | local_symbols
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
    # These declarations are not addresses yet. Only the exact fresh context
    # object may qualify them at the compiled-evidence boundary.
    externals.update({symbol: target for symbol, target in context_targets.items()
                      if symbol in externals})
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
                   associated_bindings=(), _weak_scope=None):
    """Same strict target resolution for ordinary measurements and source trials."""
    return _compiled_evidence(bindings, object_bindings, generated_bindings, objects, image, externals,
                              errors=errors, import_bindings=import_bindings, shared_bindings=shared_bindings,
                              literal_bindings=literal_bindings, fragments=fragments,
                              associated_bindings=associated_bindings, _weak_scope=_weak_scope)[0]


def _compiled_evidence(bindings, object_bindings, generated_bindings, objects, image, externals, *, errors=None,
                       import_bindings=(), shared_bindings=(), literal_bindings=(), fragments=(),
                       associated_bindings=(), function_support_bindings=(), selected_object_targets=None,
                       selected_object_contents=None, _weak_scope=None):
    """Resolve targets from product objects, never from a retention probe."""
    _validate_compiled_namespaces((*bindings, *function_support_bindings), object_bindings, generated_bindings,
                                  import_bindings, shared_bindings, externals)
    externals, context_checks = qualify_context_targets(externals, objects)
    target_bindings = (object_bindings if selected_object_targets is None
                       else selected_object_targets)
    content_bindings = (object_bindings if selected_object_contents is None
                        else selected_object_contents)
    data_targets, checked_objects = object_targets(target_bindings, objects)
    local_data_targets, checked_local_objects = local_object_targets(target_bindings, objects)
    explicit_local_data_targets = {
        (item.unit, item.emitted_symbol): local_data_targets[(item.unit, item.emitted_symbol)]
        for item in target_bindings if item.target_local
        and (item.unit, item.emitted_symbol) in local_data_targets}
    local_reports = {(row["unit"], row["symbol"]): row for row in checked_local_objects
                     if row["definition"] == "pass"}
    checked_objects = [local_reports.get((row["unit"], row["symbol"]), row) for row in checked_objects]
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
                                 literal_targets=literal_targets,
                                 local_data_targets=explicit_local_data_targets, errors=errors)
    by_symbol = {item["symbol"]: item for item in initial}
    checked_objects = [by_symbol.pop(row["symbol"], row) for row in checked_objects] + list(by_symbol.values())
    targets = {**externals, **data_targets, **generated_targets}
    rows = check_objects(bindings, objects, image,
                         external_targets=targets,
                         local_data_targets=local_data_targets,
                         explicit_local_data_names=tuple(
                             (item.unit, item.emitted_symbol) for item in target_bindings
                             if item.target_local),
                         import_targets=imports, literal_targets=literal_targets, errors=errors,
                         fragments=fragments, associated_bindings=associated_bindings,
                         support_bindings=function_support_bindings, _weak_scope=_weak_scope)
    if context_checks:
        for row in rows:
            names = {item["symbol"] for item in row.get("diagnostic", {}).get("emitted_references", ())}
            if row.get("source_unit"):
                names.add(row["symbol"])
            relevant = [item for item in context_checks if item["symbol"] in names]
            if relevant:
                row["context_target_checks"] = relevant
    return (rows, checked_objects, checked_generated, checked_imports, checked_shared, checked_literals), targets, imports


def _verify_selection(root: Path, selected: frozenset[str], image_path: Path, *,
                      compiler: Callable | None = None, command_for: Callable | None = None,
                      flag_sets=None, debug_types: bool = False, retained_diagnostics: bool = False,
                      selection_guard=None, selection_label=None, expected_image_sha256=None,
                      record_evidence: bool = False, source_inputs: SourceInputs | None = None) -> dict:
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
    if source_inputs is None:
        source_inputs = SourceInputs.from_snapshot(root, source_snapshot(root))
    elif source_inputs.root != root:
        raise ValueError("captured source inputs belong to a different root")
    snapshot = source_inputs.snapshot
    view, snapshot = project_view(root, snapshot=snapshot)
    from .body_attribution import BodyAttribution
    body_attribution = BodyAttribution(root, snapshot)
    if expected_image_sha256 is not None and expected_image_sha256 != snapshot["image_sha256"]:
        raise ValueError("records refer to a different configured original image")
    image_data = image_path.read_bytes()
    image = PeImage(image_data, snapshot["image_sha256"])
    object_bindings = read_objects(root, snapshot["image_sha256"], inputs=source_inputs)
    literal_bindings = read_literals(root, snapshot["image_sha256"], inputs=source_inputs)
    associated_bindings = read_associated(root, snapshot["image_sha256"], inputs=source_inputs)
    if any(item.span is not None for item in object_bindings) and not explicit_flag_sets:
        flag_sets = tuple(flags + settings.layout_flags for flags in settings.flag_sets)
    generated_bindings = read_generated(root, snapshot["image_sha256"], inputs=source_inputs)
    import_bindings = read_imports(root, inputs=source_inputs)
    shared_bindings = resolve_shared_contexts(
        root, read_shared_bindings(root, snapshot["image_sha256"], inputs=source_inputs),
        settings=settings, inputs=source_inputs)
    object_symbols = set(emitted_object_symbols(object_bindings))
    validate_shared_bindings(shared_bindings, view, reserved=[*object_symbols,
        *[item.symbol for item in (*generated_bindings, *import_bindings)]])
    from .component_targets import component_targets
    try:
        dependency_targets, component_reports = component_targets(root, snapshot, image_data)
    except ValueError as error:
        raise ValueError(f"component dependency verification refused: {error}") from error
    context_units = validated_context_units(root, snapshot, settings)
    bindings, refused, externals = select_bindings(
        view, selected, component_targets=dependency_targets, context_units=context_units)
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
    for binding in bindings:
        entity_id = binding.origin.entity_id
        if body_attribution.has_reference(entity_id):
            record = body_attribution.record(entity_id)
            if body_attribution.is_placeholder(record):
                placeholders.add((entity_id, record.path.relative_to(root).as_posix()))
    unfinished = [item for item in bindings
                  if (item.origin.entity_id, item.source_unit or item.unit) in placeholders]
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
                                        unit_extensions=settings.unit_extensions,
                                        context_targets=externals)
                       if record_evidence else None)
    if record_evidence:
        units = sorted(initial_evidence.units)
    else:
        required_data = [item for item in object_bindings if item.evidence is not None and item.evidence.entity_id in selected]
        required_shared = [item for item in shared_bindings if item.evidence.entity_id in selected]
        required_literals = [item for item in literal_bindings if item.evidence.entity_id in selected]
        units = sorted({item.unit for item in bindings} | {item.unit for item in required_data}
                       | {item.object_unit for item in required_shared if item.object_unit is not None}
                       | {item.unit for item in required_literals})
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
                                       unit_extensions=settings.unit_extensions,
                                       context_targets=externals)
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
                                           unit_extensions=settings.unit_extensions,
                                           context_targets=externals)
        else:
            # Full task-scoped behavior: follow every declared object/shared
            # relocation in the selected application object graph.
            while pending := (referenced_object_units(object_bindings, objects)
                              | referenced_shared_units(shared_bindings, objects)
                              | referenced_context_units(externals, objects)) - objects.keys() - errors.keys():
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
                         else [item for item in shared_bindings
                               if item.object_unit in objects or item.object_unit in errors
                               or item.context_error is not None])
        active_literals = active_objects.literals if active_objects is not None else literal_bindings
        checked, product_targets, product_imports = _compiled_evidence(
            bindings, object_bindings, active_generated, objects, image, externals, errors=errors,
            import_bindings=active_imports, shared_bindings=active_shared, literal_bindings=active_literals,
            fragments=fragment_bindings, associated_bindings=associated_bindings,
            selected_object_targets=(active_objects.object_targets if active_objects is not None else None),
            selected_object_contents=(active_objects.object_contents if active_objects is not None else None))
        checked_functions, checked_objects, checked_generated, checked_imports, checked_shared, checked_literals = checked
        # An explicit header/body path is an independent source attribution
        # obligation. Exact emitted bytes cannot validate a pointer to a
        # different overload, namespace, header, or a marker-local caller.
        binding_by_id = {binding.origin.entity_id: binding for binding in bindings}
        for row in checked_functions:
            entity_id = row.get('entity_id')
            if not body_attribution.has_reference(entity_id):
                continue
            row['compiled_comparison_result'] = row.get('result', Result.UNKNOWN)
            if row['compiled_comparison_result'] != Result.PASS:
                row['body_attribution'] = {'result': 'not_checked',
                                          'reason': 'ordinary comparison did not pass; no admission'}
                continue
            binding = binding_by_id.get(entity_id)
            if binding is None or binding.unit not in objects:
                continue  # Missing ordinary compilation already refuses.
            try:
                attribution = body_attribution.qualify(
                    body_attribution.record(entity_id), objects[binding.unit],
                    unit=binding.unit, arguments=commands[binding.unit],
                    flags=effective_by_unit[binding.unit], compiler=compiler)
                row['body_attribution'] = attribution
            except (OSError, ValueError) as error:
                row['body_attribution'] = {'result': 'unknown', 'reason': str(error)}
                row['result'] = Result.UNKNOWN
                row['reason'] = 'declared source body attribution refused: ' + str(error)
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
        rows.append({**{name: candidates[0][name] for name in
                        ("entity_id", "address", "symbol", "unit", "source_unit", "compilation_context")
                        if name in candidates[0]},
                     "result": result, "variants": candidates})
    if (settings.snapshot(root, measurement_options) != before or image_path.read_bytes() != image_data):
        raise ValueError("source, build inputs or image changed during measurement; retry")
    body_attribution.assert_current()
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
            "compile_attempts": attempts + body_attribution.compile_attempts,
            "body_attribution_compile_attempts": body_attribution.compile_attempts,
            "selected_entities": len(selected), "translation_units": sorted(generated_units),
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
            "shared_owners": [{"interpretation_id": item.interpretation_id,
                               "unit": item.object_unit or item.unit}
                              for item in shared_bindings],
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
    image_path = image_path if image_path.is_absolute() else root / image_path
    image_path = image_path.resolve()
    snapshot = source_snapshot(root, image_path)
    from .source_config import SourceConfiguration
    if SourceConfiguration.from_dict(snapshot["source_configuration"]) != settings.source:
        raise ValueError("source configuration changed while starting record verification")
    source_inputs = SourceInputs.from_snapshot(root, snapshot)
    pin = snapshot["image_sha256"]
    selected_records = tuple(records)
    if not selected_records:
        raise ValueError("record verification requires an explicit nonempty selection")
    by_key = defaultdict(list)
    selected_ids, selected_keys, selected_hosts = [], [], {}
    for record in selected_records:
        try:
            relative = record.path.resolve().relative_to(root).as_posix()
            key = (record.path.resolve(), record.address, record.line)
        except (AttributeError, OSError, ValueError) as error:
            raise ValueError("selected record is not a configured current source annotation") from error
        if relative not in snapshot["files"]:
            raise ValueError("selected record is not a configured current source annotation")
        selected_hosts[relative] = record.path.resolve()
        selected_keys.append(key)
        selected_ids.append(Origin(pin, settings.source.address_space, record.address).entity_id)
    from match_recovery.source import read_text
    for relative, path in selected_hosts.items():
        for current in read_text(source_inputs.text(relative), path):
            by_key[(current.path.resolve(), current.address, current.line)].append(current)
    for key, selected_record in zip(selected_keys, selected_records):
        candidates = by_key.get(key, ())
        if len(candidates) != 1 or candidates[0] != selected_record:
            raise ValueError("selected record differs from the current configured source annotation")
    if len(set(selected_ids)) != len(selected_ids):
        raise ValueError("record verification selection contains duplicate original identities")
    selected = frozenset(selected_ids)

    def guard():
        assert_snapshot_current(root, snapshot)
        if image_pin(root, settings.source) != pin:
            raise ValueError("source records or configured image pin changed during verification; retry")

    return _verify_selection(root, selected, image_path, compiler=compiler,
                              command_for=command_for, flag_sets=flag_sets,
                              retained_diagnostics=retained_diagnostics,
                              selection_guard=guard, selection_label=None,
                              expected_image_sha256=pin, record_evidence=True,
                              source_inputs=source_inputs)


def _source_receipt_object_owners(objects, receipt_outputs, archive_receipts=()):
    """Resolve final-pass owners bound to direct or exact archive receipts.

    ``archive_receipts`` are already passing compile/link observations. The
    native selected-object resolver supplies the physical archive-member
    identity and bytes; this function binds those observations to the unique
    final-pass map owner without basename-only object selection.
    """
    archive_by_key = {}
    for receipt in archive_receipts:
        if (not isinstance(receipt, dict)
                or set(receipt) != {"archive", "member", "object", "source"}
                or not all(isinstance(receipt[key], str) and receipt[key]
                           for key in ("archive", "member", "object", "source"))):
            raise ValueError("invalid passing archive source receipt")
        key = (receipt["archive"], receipt["member"], receipt["object"])
        if key in archive_by_key:
            raise ValueError("duplicate passing archive source receipt")
        archive_by_key[key] = receipt
    owner_paths = {}
    for owner_rows in objects._owners.values():
        for owner_row in owner_rows:
            relevant = bool(receipt_outputs.intersection(owner_row["paths"]))
            archive_candidates = [receipt for (archive, member, _output), receipt in archive_by_key.items()
                                 if archive in owner_row["paths"]
                                 and PurePosixPath(member).name.casefold()
                                 == PureWindowsPath(owner_row["member"]).name.casefold()]
            if not relevant and not archive_candidates:
                continue
            owner = owner_row["owner"]
            selected_object = objects.resolve(owner)
            identity = selected_object["identity"]
            if set(identity) == {"path"}:
                if identity["path"] in receipt_outputs:
                    owner_paths.setdefault(identity["path"], []).append((owner, selected_object))
                continue
            if set(identity) != {"path", "member", "occurrence"}:
                continue
            if type(identity["occurrence"]) is not int or identity["occurrence"] < 0:
                continue
            matching = [receipt for (archive, member, _output), receipt in archive_by_key.items()
                        if archive == identity["path"] and member == identity["member"]]
            if len(matching) == 1:
                receipt, = matching
                owner_paths.setdefault(receipt["object"], []).append((owner, selected_object))
    return owner_paths


def _passing_archive_source_receipts(compile_rows, command_by_output):
    """Bind archive correspondence rows to their unique successful command receipt."""
    receipts = []
    for row in compile_rows:
        output = row.get("output")
        commands = command_by_output.get(output, ())
        if (len(commands) != 1 or commands[0].get("result") != Result.PASS
                or commands[0].get("source") != row.get("source")
                or row.get("result") != Result.PASS
                or row.get("link_correspondence") != Result.PASS):
            continue
        archive = row.get("archive_correspondence")
        if (not isinstance(archive, dict) or archive.get("result") != Result.PASS
                or archive.get("object") != output
                or not all(isinstance(archive.get(key), str) and archive[key]
                           for key in ("archive", "member"))):
            continue
        receipts.append({"archive": archive["archive"], "member": archive["member"],
                         "object": output, "source": row["source"]})
    return receipts


def _source_receipt_object_matches(selected_object, observed):
    """Require compiler-receipt content identity to match resolved physical bytes."""
    return observed == {"sha256": selected_object["sha256"],
                       "size": len(selected_object["raw"])}


def verify_linked_selection(root: Path, selected, image_path: Path) -> dict:
    """Compare selected original rows using one fresh, receipt-bound live app build.

    This explicit diagnostic path consumes the exact source objects selected by
    the current linker. It does not bank claims, qualify preservation, or turn
    a graph observation into acceptance.
    """
    from .application_context import application_build
    from .build_check import Build
    from .linked_context import linked_build
    from .source_replica import replica_roots, replica_snapshot

    root = Path(root).resolve()
    selected = frozenset(selected)
    if not selected:
        raise ValueError("linked source comparison refuses an empty selection")
    settings = Measurement.read(root)
    image_path = image_path if image_path.is_absolute() else root / image_path
    image_path = image_path.resolve()
    pin = image_pin(root, settings.source)
    raw_image = image_path.read_bytes()
    if hashlib.sha256(raw_image).hexdigest() != pin:
        raise ValueError("linked source comparison requires the configured pinned original image")
    image = PeImage(raw_image, pin)
    source_roots = replica_roots(settings.source.roots)
    source_files = {name: data for name, data in replica_snapshot(root, source_roots).items()
                    if data is not None}
    build = Build.read(root)
    versioned = {name: (root / name).read_bytes() for name in build.versioned_inputs}
    before = settings.snapshot(root, {"linked_current_image": pin,
                                      "linked_current_selection": sorted(selected)})
    view, snapshot = project_view(root)
    from .component_targets import component_targets
    dependency_targets, component_reports = component_targets(root, snapshot, raw_image)
    context_units = validated_context_units(root, snapshot, settings)
    all_ids = frozenset(row["entity_id"] for row in snapshot["records"])
    bindings, refused, externals = select_bindings(
        view, all_ids, component_targets=dependency_targets, context_units=context_units)
    bindings, fragments, _support, fragment_refused = select_fragment_bindings(view, all_ids, bindings)
    refused.extend(fragment_refused)
    selected_refused = [row for row in refused if row.get("entity_id") in selected]
    bindings = [item for item in bindings if item.origin.entity_id not in {
        row.get("entity_id") for row in selected_refused}]
    from .object_source import read_objects
    from .literal_source import read_literals
    from .associated_source import read_associated
    from .generated_source import read_generated
    from .import_source import read_imports
    from .shared_source import read_shared_bindings, resolve_shared_contexts
    object_bindings = read_objects(root, pin)
    literal_bindings = read_literals(root, pin)
    associated_bindings = read_associated(root, pin)
    generated_bindings = read_generated(root, pin)
    import_bindings = read_imports(root)
    shared_bindings = resolve_shared_contexts(
        root, read_shared_bindings(root, pin), settings=settings)

    with application_build(root, source_roots, source_files,
                           versioned_files=versioned) as application:
        with linked_build(application) as native:
            report = application.report.get("compile_link")
            if (not isinstance(report, dict) or report.get("result") != Result.PASS
                    or report.get("command_bindings", {}).get("result") != Result.PASS):
                raise ValueError("linked source comparison requires complete source/flag/output compile receipts")
            command_rows = report["command_bindings"]["compilations"]
            command_by_output = defaultdict(list)
            for row in command_rows:
                command_by_output[row["output"]].append(row)
            compile_rows = [row for row in report.get("compilations", ())
                            if row.get("result") == Result.PASS
                            and row.get("link_correspondence") == Result.PASS]
            receipt_outputs = {row["output"] for row in compile_rows
                               if len(command_by_output.get(row["output"], ())) == 1
                               and command_by_output[row["output"]][0].get("result") == Result.PASS
                               and command_by_output[row["output"]][0].get("source") == row.get("source")}
            archive_receipts = _passing_archive_source_receipts(compile_rows, command_by_output)
            outputs = {}
            owner_paths = _source_receipt_object_owners(native.inputs.objects, receipt_outputs,
                                                        archive_receipts)
            compile_by_unit = {}
            for row in compile_rows:
                output = row["output"]
                commands_for_output = command_by_output.get(output, ())
                if len(commands_for_output) != 1:
                    continue
                command, = commands_for_output
                if command.get("result") != Result.PASS or command.get("source") != row.get("source"):
                    continue
                source = Path(row["source"])
                try:
                    unit = source.relative_to(application.root).as_posix()
                except ValueError:
                    continue
                exact = owner_paths.get(output, ())
                if len(exact) != 1:
                    continue
                owner, selected_object = exact[0]
                observed = row.get("compilation", {}).get("output", {}).get("after")
                if not _source_receipt_object_matches(selected_object, observed):
                    continue
                if unit in compile_by_unit:
                    raise ValueError("linked source compile receipts duplicate one translation unit")
                outputs[unit] = (owner, Coff(selected_object["raw"]))
                compile_by_unit[unit] = {"owner": owner, "output": output,
                                         "sha256": selected_object["sha256"],
                                         "command": command}
            objects = {unit: item[1] for unit, item in outputs.items()}
            errors = {unit: "no unique selected physical object matches the source/command receipt"
                      for unit in sorted({binding.unit for binding in bindings}) if unit not in objects}
            weak_roots, weak_root_refusals = [], []
            for binding in bindings:
                if binding.origin.entity_id not in selected:
                    continue
                physical = objects.get(binding.unit)
                owner_row = compile_by_unit.get(binding.unit)
                if physical is None or owner_row is None:
                    weak_root_refusals.append({"owner": binding.unit, "symbol": binding.symbol,
                                               "source_unit": binding.unit, "result": Result.UNKNOWN,
                                               "entity_id": binding.origin.entity_id,
                                               "blocks_original_qualification": True,
                                               "reason": "selected root has no unique receipt-bound physical source object"})
                    continue
                try:
                    subject = physical.function(binding.symbol, comdat=True)
                    has_weak = any(physical.symbols[int(relocation.target)].storage == 105
                                   for number in physical.code_sections(subject)
                                   for relocation in physical.section_subject(number)[1])
                    if has_weak:
                        map_rows = [row for row in native.inputs.mapping.publics
                                    if row.symbol == binding.symbol and row.owner.casefold() == owner_row["owner"].casefold()
                                    and row.function and row.segment]
                        if len(map_rows) != 1:
                            weak_root_refusals.append({"owner": owner_row["owner"], "symbol": binding.symbol,
                                                       "source_unit": binding.unit, "result": Result.UNKNOWN,
                                                       "entity_id": binding.origin.entity_id,
                                                       "blocks_original_qualification": True,
                                                       "reason": "selected weak root lacks one unique owner-qualified map placement"})
                            continue
                        weak_roots.append({"owner": owner_row["owner"], "symbol": binding.symbol,
                                           "unit": binding.unit, "entity_id": binding.origin.entity_id})
                except ValueError:
                    weak_root_refusals.append({"owner": owner_row["owner"], "symbol": binding.symbol,
                                               "source_unit": binding.unit, "result": Result.UNKNOWN,
                                               "entity_id": binding.origin.entity_id,
                                               "blocks_original_qualification": True,
                                               "reason": "selected root is not a uniquely bounded weak caller"})
                    continue
            weak_scope = native._weak_scope()
            try:
                current_graphs = []
                current_graph_result = Result.UNKNOWN
                current_root_regions = [*native._source_regions_in_scope(weak_roots, weak_scope),
                                        *weak_root_refusals]
                current_root_result = (Result.UNKNOWN if not current_root_regions or
                                       any(item.get("result") == Result.UNKNOWN
                                           for item in current_root_regions) else
                                       Result.FAIL if any(item.get("result") == Result.FAIL
                                                          for item in current_root_regions) else Result.PASS)
                observed = observe_compiled_selection(
                    bindings, selected, object_bindings, generated_bindings, objects, image, externals,
                    errors=errors, import_bindings=import_bindings, shared_bindings=shared_bindings,
                    literal_bindings=literal_bindings, fragments=fragments,
                    associated_bindings=associated_bindings, unit_extensions=settings.unit_extensions,
                    _weak_scope=weak_scope)
            finally:
                weak_scope.expire()
            native.check_current()
    if (settings.snapshot(root, {"linked_current_image": pin,
                                 "linked_current_selection": sorted(selected)}) != before
            or image_path.read_bytes() != raw_image):
        raise ValueError("source, linked build inputs or pinned image changed during linked comparison")
    rows = list(observed.function_rows)
    for row in rows:
        row["linked_current_code_graph"] = current_graph_result
        matching_regions = [item for item in current_root_regions
                            if item.get("entity_id") == row.get("entity_id")
                            or (item.get("entity_id") is None and item.get("symbol") == row.get("symbol")
                                and (row.get("source_unit") is None
                                     or item.get("source_unit") == row.get("source_unit")))]
        if row.get("entity_id") is None and row.get("source_unit") is None and len(matching_regions) != 1:
            matching_regions = []
        row["linked_current_root_regions"] = matching_regions
        if any(item.get("result") == Result.FAIL for item in matching_regions):
            row["source_row_result_before_current_root"] = row.get("result", Result.UNKNOWN)
            row["result"] = Result.FAIL
        elif any(item.get("blocks_original_qualification") for item in matching_regions):
            row["source_row_result_before_current_root"] = row.get("result", Result.UNKNOWN)
            if row.get("result") != Result.FAIL:
                row["result"] = Result.UNKNOWN
    weak_rows = [relocation for row in rows for relocation in row.get("relocations", ())
                 if relocation.get("storage_class") == 105]
    result_values = [row.get("result", Result.UNKNOWN) for row in rows]
    result_values.extend(Result.UNKNOWN for _ in selected_refused)
    result_values.extend(Result.FAIL for item in current_root_regions
                         if item.get("result") == Result.FAIL)
    result_values.extend(Result.UNKNOWN for item in current_root_regions
                         if item.get("blocks_original_qualification")
                         and item.get("result") != Result.FAIL)
    for check in observed.required_evidence_checks:
        values = []
        if "definition" in check:
            values.append(check["definition"])
        if "content" in check:
            values.append(check["content"])
        if "identity" in check:
            values.append(Result.PASS if check["identity"] == "pass" else Result.UNKNOWN)
        if "result" in check:
            values.append(check["result"])
        result_values.extend(values or [Result.UNKNOWN])
    overall = (Result.UNKNOWN if Result.UNKNOWN in result_values else
               Result.FAIL if Result.FAIL in result_values else Result.PASS)
    return {"check": "linked_current_source_comparison", "result": overall,
            "functions": rows, "selected_entities": len(selected),
            "counts": dict(sorted(Counter(row.get("result", Result.UNKNOWN) for row in rows).items())),
            "current_code_graphs": current_graphs, "current_code_graph": current_graph_result,
            "current_code_graph_reason": "transitive callees were not traversed; selected source regions are reported separately",
            "current_root_regions": current_root_regions,
            "current_root_regions_result": current_root_result,
            "current_root_regions_reason": ("no selected weak-reference root was observed"
                                             if not current_root_regions else None),
            "weak_original_references": weak_rows,
            "required_evidence_checks": list(observed.required_evidence_checks),
            "required_evidence_unknowns": list(observed.required_evidence_unknowns),
            "source_object_receipts": {unit: compile_by_unit[unit]
                                       for unit in sorted(compile_by_unit)
                                       if unit in {binding.unit for binding in bindings}},
            "unavailable_source_objects": errors,
            "compile_link_result": Result.PASS, "component_verification": component_reports,
            "limitations": ["diagnostic original body comparison only; no claims, acceptance or preservation authority",
                            "only one receipt-bound physical object per source unit is eligible; archive members need exact source/output correspondence and ambiguous owners remain unknown",
                            "weak original targets require one current linked edge and a strict same-object fallback source PASS",
                            "unsupported layouts, extra weak references and unrelated unresolved dependencies remain unknown"]}


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
