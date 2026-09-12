"""Candidate PE loader-slot identities, not source intent or loaded behavior.

The enclosing live build check owns artifact freshness and independently
validates map coordinates. No symbol decoration or instruction operand supplies
an import name; identities come only from the actual PE import tables.
"""

from .model import Span


def import_slots(image):
    """Index ordinary imports with wholly contained, readable data slots.

    Slot addresses denote loader-owned four-byte storage, never function entry
    points or the prebound pointer stored there. Loaded implementation, import
    library selection and calling convention remain separate obligations.
    """
    slots = {}
    for imported in image.imports():
        span = Span(imported.address, imported.address + 4)
        matches = [(number, flags) for number, ((region, _, _), flags) in enumerate(
            zip(image.sections, image.section_flags, strict=True), 1)
            if region.start <= span.start and span.end <= region.end]
        if len(matches) != 1:
            raise ValueError("import slot must lie wholly inside one PE section")
        segment, flags = matches[0]
        if not flags & 0x40000000 or flags & (0x20000000 | 0x20):
            raise ValueError("import slot requires readable non-executable storage")
        image.read(span)  # Ordinary lookup/IAT evidence must remain file-backed.
        if image.absolute_sites(span):
            raise ValueError("import slot overlaps a PE loader base relocation")
        if any(imported.address + delta in slots for delta in range(-3, 4)):
            raise ValueError("import slots overlap or have duplicate addresses")
        slots[imported.address] = {"module": imported.module.casefold(),
            "name": imported.name, "ordinal": imported.ordinal,
            "address": imported.address, "segment": segment}
    return slots


def resolve_import_slot(symbol, placed, addend, slots):
    """Bind one map symbol to an independently parsed loader slot, if present.

    The caller must separately require an ABS32 relocation and check its actual
    linked operand/HIGHLOW. A COFF function type hint is permitted for an import
    pointer, but defined, common, static and weak symbols are never treated as
    ordinary imports. This is candidate correspondence, not a source binding.
    """
    slot = slots.get(placed.address)
    if slot is None:
        return None
    if symbol.storage != 2 or symbol.section != 0 or symbol.value != 0:
        raise ValueError("import slot requires an ordinary undefined external")
    if (placed.symbol != symbol.name or placed.function
            or not placed.segment or placed.segment != slot["segment"]):
        raise ValueError("import slot requires its exact non-function map placement")
    if type(addend) is not int or addend != 0:
        raise ValueError("import slot reference requires a zero addend")
    return {key: slot[key] for key in ("module", "name", "ordinal", "address")} | {"symbol": symbol.name}
