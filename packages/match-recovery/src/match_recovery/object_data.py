"""Bounded original initialization versus compiled object data and references.

Only source-declared extents are checked. Compiler type metadata supplies the
candidate extent; no inferred next-symbol boundary or four-byte scalar guess.
Passing initial storage says nothing about later constructors or lifetime.
"""

import hashlib

from .binary import Relocation, RelocationKind, compare_bound_code
from .codeview_data import data_layout
from .model import Result


MAX_OBJECT_BYTES = 16 * 1024 * 1024


def referenced_initial_object_units(selected, bindings, objects):
    """Schedule only targets of requested initializers, not their TU's functions.

    A target needs its compiled definition, not proof of its own initializer
    unless independently selected. Missing bounds/types remain checker gaps.
    """
    owners = {binding.symbol: binding.unit for binding in bindings}
    if len(owners) != len(bindings):
        raise ValueError("duplicate original object symbol binding")
    required = set()
    for binding in selected:
        if binding.span is None or binding.unit not in objects:
            continue
        obj = objects[binding.unit]
        try:
            observed = data_layout(obj, binding.symbol)
        except ValueError:
            continue
        if observed["size"] != binding.span.end - binding.span.start:
            continue
        start, end = observed["offset"], observed["offset"] + observed["size"]
        for offset, index, kind in obj.sections[observed["section"] - 1].relocations:
            symbol = obj.symbols[index]
            if kind == 6 and start <= offset <= end - 4 and symbol.storage == 2 and symbol.name in owners:
                required.add(owners[symbol.name])
    return frozenset(required)


def check_initial_data(bindings, objects, image, targets, *, errors=None):
    pin = hashlib.sha256(image.data).hexdigest()
    if any(b.origin.image_sha256 != pin or b.origin.address_space != "pe:va" for b in bindings):
        raise ValueError("object data binding belongs to another image")
    rows = []
    for binding in bindings:
        row = {"symbol": binding.symbol, "unit": binding.unit, "address": f"0x{binding.origin.address:08X}",
               "source": {"path": binding.unit, "line": binding.line}, "definition": Result.UNKNOWN,
               "content": Result.UNKNOWN, "lifetime": Result.UNKNOWN}
        if binding.span is not None:
            row["interpretation_id"] = binding.interpretation_id
        try:
            if binding.unit not in objects:
                raise ValueError((errors or {}).get(binding.unit, "object owner was not compiled"))
            obj = objects[binding.unit]
            observed = data_layout(obj, binding.symbol)
            row.update(definition=Result.PASS, compiler_storage=observed)
            if binding.span is None:
                raise ValueError("original object extent is not declared; initial data remains unverified")
            size = binding.span.end - binding.span.start
            if observed["size"] != size:
                row.update(content=Result.FAIL, proof={"result": Result.FAIL, "boundary": Result.FAIL,
                           "bytes": Result.UNKNOWN, "references": Result.UNKNOWN,
                           "original_size": size, "candidate_size": observed["size"]})
                rows.append(row)
                continue
            if size > MAX_OBJECT_BYTES:
                raise ValueError("object exceeds the bounded initial-data comparison limit")
            start, end = observed["offset"], observed["offset"] + size
            section = obj.sections[observed["section"] - 1]
            if section.flags & 0x80:
                if section.flags & 0x40 or section.data or section.relocations:
                    raise ValueError("unsupported initialized or relocated COFF zero-fill section")
                candidate = bytes(size)
            else:
                if end > len(section.data):
                    raise ValueError("truncated compiled object initialization")
                candidate = section.data[start:end]
            relocations, resolved = [], {}
            for offset, index, kind in section.relocations:
                if kind == 0:
                    continue
                if kind not in (6, 20):
                    raise ValueError("unsupported relocation in object data section")
                if offset >= end or offset + 4 <= start:
                    continue
                if not start <= offset <= end - 4 or kind != 6:
                    raise ValueError("object extent cuts or contains an unsupported data relocation")
                symbol = obj.symbols[index]
                relocations.append(Relocation(offset - start, RelocationKind.ABSOLUTE32, str(index)))
                if symbol.storage == 2 and symbol.name in targets and symbol.section >= 0:
                    if symbol.section == 0 and symbol.value:
                        continue  # COFF common allocation is not a selected definition.
                    resolved[str(index)] = targets[symbol.name]
            proof = compare_bound_code(binding.span, image.read_initial(binding.span), candidate,
                                       tuple(relocations), resolved,
                                       original_absolute_sites=image.absolute_sites(binding.span))
            for relocation in proof.get("relocations", []):
                relocation["symbol"] = obj.symbols[int(relocation["target"])].name
            row.update(content=proof["result"], proof=proof)
        except ValueError as error:
            row["reason"] = str(error)
        rows.append(row)
    return rows
