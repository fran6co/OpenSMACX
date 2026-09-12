"""Exact COFF startup-slot projection, not authorization or linked proof.

Only a native live producer may supply slots. This internal algebra has no
authority to infer ownership, accept saved reports or establish startup effects.
All unprojected records retain the ordinary strict residual comparison.
"""

from copy import copy
from dataclasses import replace
import struct

from .coff_replica import _object_evidence
from .pe_coff import weak_external


def project_startup(obj, slots, *, excluded=frozenset(), section_keys=None, entry_names=None):
    """Return residual evidence and an exact, non-authoritative coverage report.

    A slot is ``(table_section, byte_offset, root_symbol_index)``. Root sections
    are removed only when no other code/data references any symbol in them.
    Associative children, undefined dependencies and other storage stay strict.
    """
    if not isinstance(slots, (tuple, list)) or len(slots) > 4096:
        raise ValueError("startup projection requires a bounded slot sequence")
    if any(not isinstance(row, tuple) or len(row) != 3 or any(type(value) is not int for value in row)
           for row in slots):
        raise ValueError("invalid startup slot identity")
    if len(set(slots)) != len(slots):
        raise ValueError("duplicate startup projection slot")
    if not slots:
        return _object_evidence(obj, excluded=excluded, section_keys=section_keys, entry_names=entry_names), {
            "slots": [], "removed_root_sections": [], "retained_root_sections": [], "removed_symbols": []}
    tables = [number for number, section in enumerate(obj.sections, 1) if section.name == ".CRT$XCU"]
    if len(tables) != 1:
        raise ValueError("startup projection requires one ordinary initializer table")
    table, = tables
    section = obj.sections[table - 1]
    if (table in excluded or table in obj.associations or table in obj.associations.values()
            or len(section.data) != obj.section_sizes[table - 1] or not section.data
            or len(section.data) % 4 or section.alignment != 4
            or section.flags & (0x20 | 0x80 | 0x1000 | 0x20000000) or not section.flags & 0x40
            or any(section.data)):
        raise ValueError("unsupported startup table storage")
    relocations = {offset: (target, kind) for offset, target, kind in section.relocations}
    if (len(relocations) != len(section.relocations)
            or set(relocations) != set(range(0, len(section.data), 4))
            or any(kind != 6 for target, kind in relocations.values())):
        raise ValueError("startup table requires complete zero-addend ABS32 slots")
    chosen = {}
    for number, offset, index in slots:
        if number != table or offset not in relocations or relocations[offset] != (index, 6):
            raise ValueError("startup slot differs from the actual table relocation")
        if offset in chosen:
            raise ValueError("duplicate startup projection offset")
        symbol = obj.symbols[index]
        if (symbol.storage != 3 or symbol.type != 0x20 or symbol.section <= 0
                or obj.function(symbol.name).index != index or symbol.section in excluded):
            raise ValueError("startup root requires an unscoped exclusive static code COMDAT")
        chosen[offset] = index

    # Validate the original complete evidence before changing any record. In
    # particular, malformed/ambiguous identities must not vanish in projection.
    _object_evidence(obj, excluded=excluded, section_keys=section_keys, entry_names=entry_names)
    incoming = {}
    for number, item in enumerate(obj.sections, 1):
        if item.name == ".debug$S":
            continue
        for offset, target, kind in item.relocations:
            incoming.setdefault(target, []).append((number, offset))
    # Weak aliases are linker references even without an instruction operand.
    for symbol in obj.symbols.values():
        if symbol.storage == 105:
            target, _ = weak_external(obj, symbol)
            incoming.setdefault(target.index, []).append((0, symbol.index))

    projected = copy(obj)
    projected.symbols, projected.auxiliary = dict(obj.symbols), dict(obj.auxiliary)
    removed_symbols, definitions = [], []
    remaining = sorted(set(relocations) - chosen.keys())
    shifted = {offset: number * 4 for number, offset in enumerate(remaining)}
    for index, symbol in obj.symbols.items():
        if symbol.section != table:
            continue
        raw = obj.auxiliary.get(index, b"")
        if symbol.name == section.name and symbol.storage == 3 and symbol.type == 0 and symbol.value == 0:
            if len(raw) != 18 or any(raw[12:]):
                raise ValueError("startup table has unsupported section metadata")
            length, count = struct.unpack_from("<IH", raw)
            if length != len(section.data) or count != len(relocations):
                raise ValueError("startup table section metadata differs from contents")
            if incoming.get(index):
                raise ValueError("startup table section has an unowned reference")
            definitions.append(index)
            projected.auxiliary[index] = struct.pack("<IH", len(remaining) * 4, len(remaining)) + raw[6:]
            continue
        if (symbol.storage != 3 or symbol.type != 0 or raw or symbol.value not in relocations):
            raise ValueError("startup table has an external or unsupported cell symbol")
        if incoming.get(index):
            # Even an unchanged cell reference could carry an addend to a
            # removed neighbour. No operand rewriting is inferred here.
            raise ValueError("startup table cell has an unowned reference")
        if symbol.value in chosen:
            del projected.symbols[index]
            removed_symbols.append(index)
        else:
            projected.symbols[index] = replace(symbol, value=shifted[symbol.value])
    if len(definitions) > 1:
        raise ValueError("startup table has ambiguous section metadata")

    roots = sorted({obj.symbols[index].section for index in chosen.values()})
    removed, retained = [], []
    preserve = set()
    for number in roots:
        symbols = [symbol for symbol in obj.symbols.values() if symbol.section == number]
        shared = any(origin != number and (origin != table or offset not in chosen)
                     for symbol in symbols for origin, offset in incoming.get(symbol.index, ()))
        # Opaque local auxiliary metadata and exposed aliases are not silently
        # absorbed into an otherwise exclusive generated function.
        unsupported = any(symbol.storage == 2 or (obj.auxiliary.get(symbol.index) and
                          not (symbol.storage == 3 and symbol.type == 0 and
                               symbol.name == obj.sections[number - 1].name)) for symbol in symbols)
        if shared or unsupported:
            retained.append(number)
            continue
        removed.append(number)
        preserve.update(target for _, target, _ in obj.sections[number - 1].relocations)
    projected.sections = tuple(replace(item, data=bytes(4 * len(remaining)),
                                      relocations=tuple((shifted[offset], *relocations[offset]) for offset in remaining))
                               if number == table else item
                               for number, item in enumerate(obj.sections, 1))
    projected.section_sizes = tuple(4 * len(remaining) if number == table else size
                                    for number, size in enumerate(obj.section_sizes, 1))
    omitted = set(excluded) | set(removed) | ({table} if not remaining else set())
    value = _object_evidence(projected, excluded=frozenset(omitted), section_keys=section_keys,
                             entry_names=entry_names, preserve_references=frozenset(preserve))
    consumed = set(removed) | ({table} if not remaining else set())
    removed_symbols.extend(index for index, symbol in obj.symbols.items()
                           if symbol.section in consumed and symbol.storage != 103)
    return value, {"slots": [list(row) for row in sorted(slots)], "removed_root_sections": removed,
                   "retained_root_sections": retained, "removed_symbols": sorted(set(removed_symbols)),
                   "remaining_slots": len(remaining), "table_removed": not remaining,
                   "preserved_reference_symbols": sorted(preserve),
                   "covered_root_references": [{"section": number, "offset": offset,
                                                 "target": target, "kind": kind}
                                                for number in removed
                                                for offset, target, kind in obj.sections[number - 1].relocations]}
