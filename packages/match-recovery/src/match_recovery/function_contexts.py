"""Explicit compile-context function identities for object-level checks.

These bindings qualify a definition in its declared COFF object. They do not
prove which provider a linker selected or retained in a final image.
"""

from dataclasses import dataclass
from typing import Mapping

from .components import relative_path
from .model import Origin, Result, require_text
from .pe_coff import Coff


@dataclass(frozen=True)
class ContextFunctionTarget:
    """A function identity whose body is compiled in an explicit context TU."""

    origin: Origin
    symbol: str
    unit: str
    source_unit: str

    def __post_init__(self):
        if not isinstance(self.origin, Origin):
            raise ValueError("context function target requires an original identity")
        require_text(self.symbol, "context function symbol")
        relative_path(self.unit)
        relative_path(self.source_unit)


def qualify_context_targets(targets: Mapping[str, object], objects: Mapping[str, Coff]):
    """Resolve explicit context targets only from their named COFF objects.

    Ordinary target values are copied unchanged. A context target is published
    only after the named object proves one external function in an independent
    code COMDAT. Each refusal is local to that target and remains UNKNOWN.
    """
    resolved = {}
    reports = []
    for name, target in targets.items():
        if not isinstance(target, ContextFunctionTarget):
            resolved[name] = target
            continue

        reason = None
        if name != target.symbol:
            reason = "context target key does not match its exact COFF symbol"
        elif target.unit not in objects:
            reason = "declared context object is missing"
        else:
            obj = objects[target.unit]
            try:
                definition = obj.function(target.symbol, comdat=True)
                if definition.storage != 2:
                    raise ValueError("context function definition is not external COFF storage")
                if definition.section in obj.associations:
                    raise ValueError("context function must own an independent root, not an associative COMDAT child")
                # `function` proves a unique function symbol owns the root
                # COMDAT. `subject` checks relocation kinds; explicitly check
                # their physical extent and symbol-table identity as well.
                data, _relocations, _alignment = obj.subject(definition)
                if not data:
                    raise ValueError("context function code COMDAT is empty")
                for offset, symbol_index, kind in obj.sections[definition.section - 1].relocations:
                    if kind == 0:
                        continue
                    if offset < 0 or offset + 4 > len(data):
                        raise ValueError("context relocation extends outside its code COMDAT")
                    if symbol_index not in obj.symbols:
                        raise ValueError("context relocation has no COFF symbol-table identity")
            except (AttributeError, IndexError, KeyError, TypeError, ValueError) as error:
                reason = str(error) or "context function COFF definition is malformed"

        if reason is None:
            resolved[name] = target.origin.address
            reports.append({
                "symbol": target.symbol,
                "unit": target.unit,
                "source_unit": target.source_unit,
                "origin": target.origin.entity_id,
                "result": Result.PASS,
                "evidence": "object_level_definition_identity",
                "linked_selection": False,
                "retained_equivalence": False,
            })
        else:
            reports.append({
                "symbol": target.symbol,
                "unit": target.unit,
                "source_unit": target.source_unit,
                "origin": target.origin.entity_id,
                "result": Result.UNKNOWN,
                "reason": reason,
                "evidence": "object_level_definition_identity",
                "linked_selection": False,
                "retained_equivalence": False,
            })
    return resolved, reports


def referenced_context_units(targets: Mapping[str, object], objects: Mapping[str, Coff]) -> set[str]:
    """Return contexts named by external relocations in runtime sections.

    This inspects physical COFF relocation records in code and runtime data
    sections, including vtables and function-pointer tables. Debug and linker
    directive metadata are excluded. It does not search for a same-named
    provider; static names and no-op relocations are ignored.
    """
    contexts = {name: target for name, target in targets.items()
                if isinstance(target, ContextFunctionTarget) and name == target.symbol}
    if not contexts:
        return set()

    referenced = set()
    for obj in objects.values():
        for section in obj.sections:
            if section.name.startswith(".debug") or section.name == ".drectve":
                continue
            for _offset, symbol_index, kind in section.relocations:
                if kind == 0:  # IMAGE_REL_I386_ABSOLUTE: no-op
                    continue
                symbol = obj.symbols.get(symbol_index)
                if symbol is None or symbol.storage != 2:
                    continue
                target = contexts.get(symbol.name)
                if target is not None:
                    referenced.add(target.unit)
    return referenced
