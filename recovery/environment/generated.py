"""Validate emitted vftable identities and independently bounded table contents.

Bindings may resolve references to table starts without proving table contents.
Every such table retains a separate content result; missing extents stay UNKNOWN.
No original operand or candidate section length supplies a missing source fact.
"""

import hashlib

from match_recovery.binary import Relocation, RelocationKind, compare_bound_code
from match_recovery.model import Result, Span


def check_generated(bindings, objects, image, external_targets):
    pin = hashlib.sha256(image.data).hexdigest()
    targets, rows, emitted = {}, [], []
    for binding in bindings:
        if binding.origin.image_sha256 != pin or binding.origin.address_space != "pe:va":
            raise ValueError("generated binding belongs to another image")
        if not binding.symbol.startswith("??_7"):
            raise ValueError("VC6 vtable binding requires an exact compiler vftable symbol")
        image.read(Span(binding.origin.address, binding.origin.address + 4))
        found = []
        for unit, obj in objects.items():
            matches = [symbol for symbol in obj.symbols.values() if symbol.name == binding.symbol
                       and symbol.storage == 2 and symbol.section > 0]
            if not matches:
                continue
            if len(matches) != 1:
                raise ValueError("ambiguous generated table definition")
            symbol = matches[0]
            section = obj.sections[symbol.section - 1]
            peers = [s for s in obj.symbols.values() if s.storage == 2 and s.section == symbol.section]
            valid = (not symbol.type & 0x20 and symbol.value == 0 and len(peers) == 1
                     and section.flags & 0x1000 and section.flags & 0x40
                     and not section.flags & (0x20 | 0x80000000))
            if not valid:
                raise ValueError("vtable requires an isolated read-only data COMDAT at offset zero")
            found.append((unit, obj, section))
        if not found:
            continue  # Uncompiled owners do not create reference bindings.
        if binding.symbol in external_targets and external_targets[binding.symbol] != binding.origin.address:
            raise ValueError("conflicting generated reference binding")
        targets[binding.symbol] = binding.origin.address
        emitted.extend((binding, *item) for item in found)
    external = {**external_targets, **targets}
    for binding, unit, obj, section in emitted:
        row = {"symbol": binding.symbol, "unit": unit, "address": f"0x{binding.origin.address:08X}",
               "definition": Result.PASS, "content": Result.UNKNOWN}
        if binding.span is None:
            row["reason"] = "original table extent is not declared; content remains unverified"
        else:
            try:
                relocations, resolved = [], {}
                for offset, index, kind in section.relocations:
                    if kind != 6:
                        raise ValueError("unsupported vtable relocation")
                    symbol = obj.symbols[index]
                    relocations.append(Relocation(offset, RelocationKind.ABSOLUTE32, str(index)))
                    if symbol.storage == 2 and symbol.name in external:
                        resolved[str(index)] = external[symbol.name]
                proof = compare_bound_code(binding.span, image.read(binding.span), section.data,
                                           tuple(relocations), resolved,
                                           original_absolute_sites=image.absolute_sites(binding.span))
                for relocation in proof.get("relocations", []):
                    relocation["symbol"] = obj.symbols[int(relocation["target"])].name
                row.update(content=proof["result"], proof=proof)
            except ValueError as error:
                row["reason"] = str(error)
        rows.append(row)
    return targets, rows
