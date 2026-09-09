"""Validate emitted vftable identities and independently bounded table contents.

Bindings may resolve references to table starts without proving table contents.
Every such table retains a separate content result; missing extents stay UNKNOWN.
No original operand or candidate section length supplies a missing source fact.
"""

from collections import defaultdict
import hashlib

from match_recovery.binary import Relocation, RelocationKind, compare_bound_code
from match_recovery.model import Result, Span
from match_recovery.pe_coff import weak_external


MAX_ENTRY_DIAGNOSTICS = 128


def definition_absent(witness):
    """Only an inspected symbol table can establish non-emission in one object.

    Ordinary unresolved externals are declarations, not definitions. Common,
    absolute, static and weak/alias symbols need separate handling; never call
    them absence just because the ordinary external-definition scan skips them.
    This evidence is consumed within the same fresh native check, not replayed.
    """
    if (not isinstance(witness, dict) or witness.get("basis") != "compiled_coff_symbols"
            or not isinstance(witness.get("symbol"), str) or not witness["symbol"].startswith("??_7")
            or not isinstance(witness.get("records"), list)):
        return False
    return all(isinstance(row, dict) and set(row) == {"storage", "section", "value", "type"}
               and all(type(value) is int for value in row.values())
               and row == {"storage": 2, "section": 0, "value": 0, "type": 0}
               for row in witness["records"])


def generated_coverage(bindings, objects):
    """Observe non-emission without requiring every TU to emit every vtable.

    Every actual external definition still goes through check_generated. No row
    is created for a missing/failed compilation. Unsupported symbols remain
    visible even in dependency objects; they cannot borrow a passing owner.
    """
    rows = []
    for binding in bindings:
        for unit, obj in sorted(objects.items()):
            matches = [symbol for symbol in obj.symbols.values() if symbol.name == binding.symbol]
            if any(symbol.storage == 2 and symbol.section > 0 for symbol in matches):
                continue
            witness = {"basis": "compiled_coff_symbols", "symbol": binding.symbol,
                       "records": [{key: getattr(symbol, key) for key in ("storage", "section", "value", "type")}
                                   for symbol in matches]}
            rows.append({"interpretation_id": binding.interpretation_id, "unit": unit,
                         "observation": "not_emitted" if definition_absent(witness) else "unsupported_symbol",
                         "witness": witness})
    return rows


def _entries(binding, obj, section, image, external, source_symbols):
    """Position-based observations, never an inferred original table extent.

    For an unbounded table, a word read after its start may already belong to
    another object. Expose that uncertainty even if every observed word agrees.
    Neither the original word nor a reverse-name correlation resolves a symbol.
    """
    by_offset = defaultdict(list)
    for offset, index, kind in section.relocations:
        by_offset[offset].append((index, kind))
    entries = []
    count = len(section.data) // 4
    for slot in range(min(count, MAX_ENTRY_DIAGNOSTICS)):
        offset = slot * 4
        address = binding.origin.address + offset
        inside = None if binding.span is None else address + 4 <= binding.span.end
        row = {"slot": slot, "offset": offset, "original_word_address": f"0x{address:08X}",
               "within_declared_extent": inside, "original_word": None,
               "original_absolute_relocation": None, "source_symbols_at_original_word": [],
               "candidate_word": None, "observation": "unavailable", "relocations": []}
        if inside is False:
            row["reason"] = "emitted slot lies beyond the declared original extent"
        else:
            try:
                span = Span(address, address + 4)
                original = int.from_bytes(image.read(span), "little")
                row["original_word"] = f"0x{original:08X}"
                row["original_absolute_relocation"] = 0 in image.absolute_sites(span)
                row["source_symbols_at_original_word"] = source_symbols.get(original, [])
            except ValueError as error:
                row["reason"] = str(error)
        relocations = by_offset.get(offset, [])
        for index, kind in relocations:
            symbol = obj.symbols[index]
            target = external.get(symbol.name) if symbol.storage == 2 else None
            detail = {"symbol": symbol.name, "coff_type": kind,
                      "source_address": f"0x{target:08X}" if type(target) is int else None}
            if symbol.storage == 105:
                try:
                    fallback, mode = weak_external(obj, symbol)
                    detail["linker_fallback"] = {"symbol": fallback.name,
                        "search": {1: "no_library", 2: "library", 3: "alias"}[mode], "selection": "unverified"}
                except ValueError as error:
                    detail["metadata_error"] = str(error)
            row["relocations"].append(detail)
        raw = int.from_bytes(section.data[offset:offset + 4], "little")
        if not relocations:
            row.update(observation="literal_without_relocation", candidate_word=f"0x{raw:08X}")
        elif len(relocations) != 1 or relocations[0][1] != 6 or any(
                by_offset.get(position) for position in range(offset + 1, offset + 4)):
            row["observation"] = "unsupported_relocation_layout"
        else:
            symbol = obj.symbols[relocations[0][0]]
            target = external.get(symbol.name) if symbol.storage == 2 else None
            if symbol.storage == 105:
                row["observation"] = "linker_selection_required"
            elif type(target) is not int or not 0 <= target <= 0xFFFFFFFF:
                row["observation"] = "unresolved_source_target"
            else:
                row["candidate_word"] = f"0x{(target + raw) & 0xFFFFFFFF:08X}"
                if row["original_word"] is not None:
                    row["observation"] = ("target_word_agrees" if row["candidate_word"] == row["original_word"]
                                          else "target_word_differs")
        entries.append(row)
    layout_error = None
    try:
        _validate_pointer_slots(binding, section)
    except ValueError as error:
        layout_error = str(error)
    return {"original_extent": ({"start": binding.span.start, "end": binding.span.end}
                                if binding.span is not None else None),
            "emitted_size": len(section.data), "entries": entries,
            "omitted_entries": max(0, count - len(entries)),
            "trailing_bytes": len(section.data) % 4,
            "relocation_layout_error": layout_error,
            "limitations": ["position-based diagnostic only; no original extent is inferred from emitted size",
                            "words outside an unknown extent may belong to another object",
                            "reverse source-symbol correlations are hints, never target bindings",
                            "word agreement alone does not prove relocation form, whole-table contents or hierarchy"]}


def _validate_pointer_slots(binding, section):
    if len(section.data) % 4 or (binding.span is not None and (binding.span.end - binding.span.start) % 4):
        raise ValueError("i386 vtable extent must contain complete four-byte slots")
    offsets = []
    for offset, _, kind in section.relocations:
        if kind != 6 or offset % 4 or not 0 <= offset <= len(section.data) - 4:
            raise ValueError("unsupported vtable relocation layout")
        offsets.append(offset)
    if len(offsets) != len(set(offsets)):
        raise ValueError("overlapping vtable relocations")


def check_generated(bindings, objects, image, external_targets):
    pin = hashlib.sha256(image.data).hexdigest()
    targets, rows, emitted = {}, [], []
    for binding in bindings:
        if binding.origin.image_sha256 != pin or binding.origin.address_space != "pe:va":
            raise ValueError("generated binding belongs to another image")
        if not binding.symbol.startswith("??_7"):
            raise ValueError("MSVC vtable binding requires an exact compiler vftable symbol")
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
    source_symbols = defaultdict(list)
    for symbol, address in sorted(external.items()):
        if type(address) is int:
            source_symbols[address].append(symbol)
    for binding, unit, obj, section in emitted:
        row = {"symbol": binding.symbol, "unit": unit, "address": f"0x{binding.origin.address:08X}",
               "interpretation_id": binding.interpretation_id,
               "source": {"path": binding.path, "line": binding.line},
               "definition": Result.PASS, "content": Result.UNKNOWN,
               "diagnostic": _entries(binding, obj, section, image, external, source_symbols)}
        if binding.span is None:
            row["reason"] = "original table extent is not declared; content remains unverified"
        else:
            try:
                _validate_pointer_slots(binding, section)
                original = image.read(binding.span)
                if len(section.data) != len(original):
                    row.update(content=Result.FAIL, proof={
                        "result": Result.FAIL, "boundary": Result.FAIL,
                        "bytes": Result.UNKNOWN, "references": Result.UNKNOWN,
                        "original_size": len(original), "candidate_size": len(section.data),
                        "reason": "candidate exceeds or misses the independently recorded table extent"})
                    rows.append(row)
                    continue
                relocations, resolved = [], {}
                for offset, index, kind in section.relocations:
                    if kind != 6:
                        raise ValueError("unsupported vtable relocation")
                    symbol = obj.symbols[index]
                    relocations.append(Relocation(offset, RelocationKind.ABSOLUTE32, str(index)))
                    if symbol.storage == 2 and symbol.name in external:
                        resolved[str(index)] = external[symbol.name]
                proof = compare_bound_code(binding.span, original, section.data,
                                           tuple(relocations), resolved,
                                           original_absolute_sites=image.absolute_sites(binding.span))
                for relocation in proof.get("relocations", []):
                    relocation["symbol"] = obj.symbols[int(relocation["target"])].name
                row.update(content=proof["result"], proof=proof)
            except ValueError as error:
                row["reason"] = str(error)
        rows.append(row)
    return targets, rows
