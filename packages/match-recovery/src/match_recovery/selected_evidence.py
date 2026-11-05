"""Dependency closure for one source-owned native measurement selection.

The task checker intentionally follows the application-wide object graph.
This module builds the narrower evidence set for explicit source records:
selected code, its associated spans, annotations rooted at those records, and
typed owners reached by actual COFF relocations from those roots.
"""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path

from .components import relative_path
from .model import Result


@dataclass(frozen=True)
class EvidenceSelection:
    object_targets: tuple
    object_contents: tuple
    generated: tuple
    imports: tuple
    shared: tuple
    literals: tuple
    units: frozenset[str]
    unknown: tuple[dict, ...]
    references: tuple[dict, ...]


def _relocation_names(unit, obj, section_numbers, *, span=None):
    names = set()
    for number in section_numbers:
        if not 1 <= number <= len(obj.sections):
            raise ValueError("selected evidence names a missing COFF section")
        section = obj.sections[number - 1]
        for offset, index, kind in section.relocations:
            if kind == 0:
                continue
            if span is not None:
                overlaps = offset < span[1] and span[0] < offset + 4
                if overlaps and not (span[0] <= offset and offset + 4 <= span[1]):
                    raise ValueError("selected object extent cuts through a COFF relocation")
                if not overlaps:
                    continue
            symbol = obj.symbols.get(index)
            if symbol is None:
                raise ValueError("selected COFF relocation has no symbol")
            names.add((unit, symbol.name, symbol.storage))
    return names


def _function_references(binding, objects):
    obj = objects.get(binding.unit)
    if obj is None:
        return set()
    subject = obj.function(binding.symbol)
    # A function's root COMDAT and compiler-associated code children are the
    # exact section scope already used by check_objects. Unclaimed bytes or
    # references in that COMDAT remain subject to the native body checker.
    return _relocation_names(binding.unit, obj, obj.code_sections(subject))


def _object_reference_span(binding, obj):
    if binding.span is None:
        raise ValueError("selected object dependency has no independently bounded original extent")
    if binding.parent_symbol is not None:
        from .codeview_data import member_data_layout
        observed = member_data_layout(obj, binding.parent_symbol, binding.member_path)
    else:
        from .codeview_data import data_layout
        observed = data_layout(obj, binding.emitted_symbol)
    expected = binding.span.end - binding.span.start
    if observed["size"] != expected:
        raise ValueError("selected object dependency lacks a matching COFF extent")
    start, end = observed["offset"], observed["offset"] + observed["size"]
    section = obj.sections[observed["section"] - 1]
    if not 0 <= start < end <= obj.section_sizes[observed["section"] - 1]:
        raise ValueError("selected object dependency exceeds its COFF section")
    return observed["section"], (start, end), section


def _object_references(binding, obj):
    number, span, _section = _object_reference_span(binding, obj)
    return _relocation_names(binding.unit, obj, (number,), span=span)


def _generated_references(binding, objects):
    matches = [(unit, obj, symbol) for unit, obj in objects.items()
               for symbol in obj.symbols.values()
               if symbol.name == binding.symbol and symbol.storage == 2 and symbol.section > 0]
    if len(matches) != 1:
        return set()
    unit, obj, symbol = matches[0]
    return _relocation_names(unit, obj, (symbol.section,))


def _closure_object_group(binding, all_objects):
    """Keep source-declared member/parent relationships intact for targets."""
    if binding.parent_symbol is None:
        siblings = [item for item in all_objects if item.unit == binding.unit
                    and item.parent_symbol == binding.emitted_symbol]
        return (binding, *siblings)
    return tuple(item for item in all_objects
                 if item.unit == binding.unit
                 and (item.parent_symbol == binding.parent_symbol
                      or (item.parent_symbol is None
                          and item.emitted_symbol == binding.parent_symbol)))


def select_evidence(functions, selected, objects, object_bindings, generated_bindings,
                    import_bindings, shared_bindings, literal_bindings, *, unit_extensions=None,
                    context_targets=None):
    """Return the typed evidence closure visible from selected current functions.

    The closure is recomputed within each actual flag trial from its COFF
    objects. It never borrows target addresses from an operand or compiles
    units because an unrelated function in the same TU references them.
    """
    all_objects = tuple(object_bindings)
    all_generated = tuple(generated_bindings)
    all_imports = tuple(import_bindings)
    all_shared = tuple(shared_bindings)
    all_literals = tuple(literal_bindings)
    selected = frozenset(selected)
    refs = set()
    diagnostics = []
    for binding in functions:
        if binding.origin.entity_id not in selected:
            continue
        try:
            refs.update(_function_references(binding, objects))
        except ValueError as error:
            diagnostics.append({"result": Result.UNKNOWN, "unit": binding.unit,
                                "symbol": binding.symbol,
                                "reason": f"selected code references could not be bounded: {error}"})

    target_objects = {}
    content_objects = {}
    generated = {}
    imports = {}
    shared = {}
    literals = {}
    scanned_data = set()
    scanned_shared = set()
    scanned_generated = set()
    reference_rows = []

    def add_ref(unit, symbol, storage, source):
        refs.add((unit, symbol, storage))
        reference_rows.append({"unit": unit, "symbol": symbol,
                               "storage_class": storage, "source": source})

    def object_matches(unit, symbol, storage):
        return [item for item in all_objects if item.emitted_symbol == symbol
                and ((storage == 2 and not item.target_local)
                     or (storage == 3 and item.unit == unit))]

    content_owners = {}

    def content_owner(item):
        if item.span is not None or item.parent_symbol is None:
            return item
        key = (item.unit, item.parent_symbol)
        if key not in content_owners:
            # An unbounded member remains a target/layout obligation. Only
            # an independently bounded direct parent, qualified against this
            # actual object, can cover its contents. Never synthesize a child
            # span from current types or a neighboring original address.
            group = _closure_object_group(item, all_objects)
            parents = [row for row in group if row.parent_symbol is None
                       and row.span is not None]
            owner = None
            obj = objects.get(item.unit)
            if len(parents) == 1 and obj is not None:
                parent = parents[0]
                try:
                    from .object_source import object_targets, local_object_targets
                    targets, _ = object_targets(group, {item.unit: obj})
                    local_targets, _ = local_object_targets(group, {item.unit: obj})
                    addresses = {targets.get(parent.emitted_symbol),
                                 local_targets.get((item.unit, parent.emitted_symbol))} - {None}
                    if (addresses == {parent.origin.address}
                            and all((row.origin.image_sha256, row.origin.address_space)
                                    == (parent.origin.image_sha256, parent.origin.address_space)
                                    for row in group)):
                        _object_reference_span(parent, obj)
                        owner = parent
                except ValueError:
                    pass  # Keep the original member's unknown prerequisite.
            content_owners[key] = owner
        return content_owners[key] or item

    def merge_object(item, *, content):
        for group_item in _closure_object_group(item, all_objects):
            target_objects[(group_item.unit, group_item.symbol, group_item.line)] = group_item
        if content:
            item = content_owner(item)
            content_objects[(item.unit, item.symbol, item.line)] = item
        return item

    pending_refs = list(refs)
    processed_refs = set()

    def process_refs():
        while pending_refs:
            unit, symbol, storage = pending_refs.pop()
            ref_key = (unit, symbol, storage)
            if ref_key in processed_refs:
                continue
            processed_refs.add(ref_key)
            matches = object_matches(unit, symbol, storage)
            for item in matches:
                item = merge_object(item, content=True)
                key = (item.unit, item.symbol, item.line)
                if key in scanned_data:
                    continue
                scanned_data.add(key)
                obj = objects.get(item.unit)
                if obj is None:
                    continue
                try:
                    for ref_unit, ref_symbol, ref_storage in _object_references(item, obj):
                        found = (ref_unit, ref_symbol, ref_storage)
                        if found not in refs:
                            add_ref(ref_unit, ref_symbol, ref_storage,
                                    f"object:{item.unit}:{item.symbol}")
                            pending_refs.append(found)
                except ValueError as error:
                    diagnostics.append({"result": Result.UNKNOWN, "unit": item.unit,
                                        "symbol": item.symbol, "reason": str(error)})

            for item in all_generated:
                if storage == 2 and item.symbol == symbol:
                    generated[item.symbol] = item
                    key = (item.path, item.symbol)
                    if key not in scanned_generated:
                        scanned_generated.add(key)
                        try:
                            for ref_unit, ref_symbol, ref_storage in _generated_references(item, objects):
                                found = (ref_unit, ref_symbol, ref_storage)
                                if found not in refs:
                                    add_ref(ref_unit, ref_symbol, ref_storage,
                                            f"generated:{item.path}:{item.symbol}")
                                    pending_refs.append(found)
                        except ValueError as error:
                            diagnostics.append({"result": Result.UNKNOWN, "unit": item.path,
                                                "symbol": item.symbol, "reason": str(error)})
            for item in all_imports:
                if storage == 2 and item.symbol == symbol:
                    imports[item.symbol] = item
            for item in all_shared:
                if storage == 2 and item.symbol == symbol:
                    shared[item.symbol] = item
                    unit = item.object_unit
                    if unit is None:
                        diagnostics.append({"result": Result.UNKNOWN, "unit": item.unit,
                                            "symbol": item.symbol,
                                            "reason": item.context_error or "shared definition has no resolved compiler context"})
                        continue
                    for copy_unit, obj in objects.items():
                        definitions = [row for row in obj.symbols.values()
                                       if row.name == item.symbol and row.storage == 2 and row.section > 0]
                        key = (copy_unit, item.symbol)
                        if not definitions or key in scanned_shared:
                            continue
                        scanned_shared.add(key)
                        try:
                            subject = obj.function(item.symbol)
                            for ref_unit, ref_symbol, ref_storage in _relocation_names(
                                    copy_unit, obj, obj.code_sections(subject)):
                                found = (ref_unit, ref_symbol, ref_storage)
                                if found not in refs:
                                    add_ref(ref_unit, ref_symbol, ref_storage,
                                            f"shared:{copy_unit}:{item.symbol}")
                                    pending_refs.append(found)
                        except ValueError as error:
                            diagnostics.append({"result": Result.UNKNOWN, "unit": copy_unit,
                                                "symbol": item.symbol,
                                                "reason": f"selected shared references could not be bounded: {error}"})
            for item in all_literals:
                if item.unit == unit and item.symbol == symbol:
                    literals[(item.unit, item.symbol)] = item

    # Explicitly rooted evidence is required even when the selected body does
    # not itself contain a relocation to it.
    for item in all_objects:
        if item.evidence is not None and item.evidence.entity_id in selected:
            merge_object(item, content=True)
            for storage in (2, 3):
                found = (item.unit, item.emitted_symbol, storage)
                if found not in refs:
                    refs.add(found)
                    pending_refs.append(found)
    for item in all_generated:
        if item.evidence.entity_id in selected:
            generated[item.symbol] = item
            found = (item.path, item.symbol, 2)
            if found not in refs:
                refs.add(found)
                pending_refs.append(found)
    for item in all_shared:
        if item.evidence.entity_id in selected:
            shared[item.symbol] = item
            if item.object_unit is None:
                diagnostics.append({"result": Result.UNKNOWN, "unit": item.unit,
                                    "symbol": item.symbol,
                                    "reason": item.context_error or "shared definition has no resolved compiler context"})
                continue
            found = (item.object_unit, item.symbol, 2)
            if found not in refs:
                refs.add(found)
                pending_refs.append(found)
    for item in all_literals:
        if item.evidence.entity_id in selected:
            literals[(item.unit, item.symbol)] = item

    process_refs()

    for item in generated.values():
        definitions = [symbol for obj in objects.values() for symbol in obj.symbols.values()
                       if symbol.name == item.symbol and symbol.storage == 2 and symbol.section > 0]
        if not definitions:
            diagnostics.append({"result": Result.UNKNOWN, "unit": item.path,
                                "symbol": item.symbol,
                                "reason": "selected generated table has no observed COFF definition in the selected evidence closure"})

    # Data/static definitions are TU-local unless the COFF relocation itself
    # is external. The key is source-unit-qualified above for that reason.
    for item in content_objects.values():
        if item.span is None:
            diagnostics.append({"result": Result.UNKNOWN, "unit": item.unit,
                                "symbol": item.symbol,
                                "reason": "selected object dependency has no independently bounded original extent"})

    units = {item.unit for item in functions if item.origin.entity_id in selected}
    units.update(item.unit for item in content_objects.values())
    units.update(item.path for item in generated.values()
                 if unit_extensions is None or Path(item.path).suffix in unit_extensions)
    units.update(item.object_unit for item in shared.values() if item.object_unit is not None)
    units.update(unit for unit, _symbol in literals)
    # Include target support units for a selected member/parent layout group.
    units.update(item.unit for item in target_objects.values())
    from .function_contexts import ContextFunctionTarget
    units.update(target.unit for _unit, symbol, storage in refs
                 if storage == 2
                 and isinstance(target := (context_targets or {}).get(symbol), ContextFunctionTarget))
    for unit in units:
        relative_path(unit)
    return EvidenceSelection(
        tuple(target_objects.values()), tuple(content_objects.values()),
        tuple(generated.values()), tuple(imports.values()), tuple(shared.values()),
        tuple(literals.values()), frozenset(units), tuple(diagnostics),
        tuple(reference_rows))
