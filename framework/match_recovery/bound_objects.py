"""Shared application/dependency checks over already compiled COFF objects.

Placement comes from source bindings, never from the operands under test. This
checks bounded code and references, not ownership, class layout or runtime.
"""

from dataclasses import dataclass
import hashlib
from typing import Mapping, Sequence

from .binary import compare_bound_code
from .components import relative_path
from .model import Origin, Result, Span, require_text
from .pe_coff import Coff, PeImage


@dataclass(frozen=True)
class FunctionBinding:
    origin: Origin
    spans: tuple[Span, ...]
    symbol: str
    unit: str

    def __post_init__(self):
        require_text(self.symbol, "emitted symbol")
        relative_path(self.unit)
        if not self.spans or self.spans[0].start != self.origin.address:
            raise ValueError("function requires an independently mapped entry span")


def check_objects(bindings: Sequence[FunctionBinding], objects: Mapping[str, Coff],
                  image: PeImage, *, external_targets: Mapping[str, int] | None = None,
                  errors: Mapping[str, str] | None = None) -> list[dict]:
    """One report per selected binding, including missing objects and symbols.

    External targets must be unambiguous source-owned mappings supplied by the
    adapter. Static symbol names never leak across translation units. Defined
    symbols resolve through their own mappings too, not only undefined imports.
    """
    if len({item.origin.entity_id for item in bindings}) != len(bindings):
        raise ValueError("ambiguous original function selection")
    pin = hashlib.sha256(image.data).hexdigest()
    if any(item.origin.image_sha256 != pin or item.origin.address_space != "pe:va" for item in bindings):
        raise ValueError("function binding refers to a different image/address space")
    externals = dict(external_targets or {})
    if any(not isinstance(key, str) or not key or type(value) is not int
           or not 0 <= value <= 0xFFFFFFFF for key, value in externals.items()):
        raise ValueError("invalid explicit external target binding")
    sections, subjects = {}, {}
    for binding in bindings:
        obj = objects.get(binding.unit)
        if obj is None:
            continue
        try:
            symbol = obj.function(binding.symbol)
        except ValueError as error:
            subjects[binding.origin.entity_id] = str(error)
            continue
        key = (binding.unit, symbol.section)
        base = binding.origin.address - symbol.value
        if key in sections and sections[key] != base:
            raise ValueError("conflicting original placement of a COFF section")
        sections[key] = base
        if symbol.storage == 2:
            if symbol.name in externals and externals[symbol.name] != binding.origin.address:
                raise ValueError("conflicting external symbol bindings")
            externals[symbol.name] = binding.origin.address
        subjects[binding.origin.entity_id] = symbol
    rows = []
    for binding in bindings:
        row = {"entity_id": binding.origin.entity_id, "address": f"0x{binding.origin.address:08X}",
               "symbol": binding.symbol, "unit": binding.unit}
        try:
            if binding.unit not in objects:
                raise ValueError((errors or {}).get(binding.unit, "translation unit was not compiled"))
            subject = subjects[binding.origin.entity_id]
            if isinstance(subject, str):
                raise ValueError(subject)
            if len(binding.spans) != 1:
                raise ValueError("multi-span placement requires an explicit section layout")
            obj, targets = objects[binding.unit], {}
            for index, symbol in obj.symbols.items():
                if symbol.section > 0:
                    base = sections.get((binding.unit, symbol.section))
                    if base is not None:
                        targets[str(index)] = base + symbol.value
                    elif symbol.storage == 2 and symbol.name in externals:
                        # A global defined in this TU still needs its own source
                        # binding. Never place an entire data section from it.
                        targets[str(index)] = externals[symbol.name]
                elif symbol.section == 0 and symbol.value == 0 and symbol.storage == 2:
                    if symbol.name in externals:
                        targets[str(index)] = externals[symbol.name]
            code, relocations, alignment = obj.subject(subject)
            span = binding.spans[0]
            proof = compare_bound_code(span, image.read(span), code, relocations, targets,
                                       original_absolute_sites=image.absolute_sites(span), alignment=alignment)
            for relocation in proof.get("relocations", []):
                relocation["symbol"] = obj.symbols[int(relocation["target"])].name
            row.update(proof)
        except ValueError as error:
            row.update(result=Result.UNKNOWN, reason=str(error))
        rows.append(row)
    return rows
