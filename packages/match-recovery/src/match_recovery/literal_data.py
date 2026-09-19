"""Source-bound TU literal storage, not address inference or linker selection.

The i386 COFF backend supports dedicated independent literal COMDATs. Their
physical section extent includes embedded NULs and wide-character storage;
neither a first terminator nor the instruction being checked chooses a bound.
"""

import hashlib

from .binary import BoundedTarget, compare_bound_code
from .coff_comdat import comdat_key
from .literal_source import validate_literals
from .model import Result


MAX_LITERAL_BYTES = 1024 * 1024


def literal_storage(obj, name):
    """Observe a whole independently defined MSVC string-literal section.

    Names select the declared candidate; they do not establish original
    identity. Static/ordinary/associative or multi-entry storage needs another
    extent witness and remains unsupported, never truncated to a C string.
    """
    definitions = [symbol for symbol in obj.symbols.values() if symbol.name == name]
    if len(definitions) != 1:
        raise ValueError("literal requires one unambiguous same-TU symbol")
    symbol = definitions[0]
    if (not name.startswith("??_C@") or symbol.section <= 0 or symbol.storage != 2
            or symbol.type or symbol.value):
        raise ValueError("unsupported independent COFF literal definition")
    section = obj.sections[symbol.section - 1]
    if (section.flags & (0x20 | 0x20000000) or not section.flags & 0x40000000
            or section.name.startswith(".tls") or section.relocations
            or comdat_key(obj, symbol.section) != symbol):
        raise ValueError("literal requires readable, non-executable independent storage without relocations")
    peers = [item for item in obj.symbols.values() if item.section == symbol.section]
    if len(peers) != 2:  # exact section definition and key, not an inferred next-symbol extent
        raise ValueError("literal section contains additional definitions or labels")
    size = obj.section_sizes[symbol.section - 1]
    if not 0 < size <= MAX_LITERAL_BYTES:
        raise ValueError("literal exceeds bounded storage limit or is empty")
    storage = section.flags & (0x40 | 0x80)
    if storage == 0x80 and not section.data:
        contents = bytes(size)
    elif storage == 0x40 and len(section.data) == size:
        contents = section.data
    else:
        raise ValueError("unsupported literal raw/zero-fill storage extent")
    return contents, {"symbol": name, "section": symbol.section, "offset": 0,
                      "size": size, "flags": section.flags, "basis": "independent_literal_comdat"}


def check_literals(bindings, objects, image, *, errors=None):
    """Return TU-qualified targets (including unresolved declarations) and proofs.

    A failed declaration reserves its key with None: content-search fallback
    must not silently replace it. Targets must be recomputed for every object
    trial, including private retention probes. No target is exported globally.
    """
    bindings = validate_literals(bindings)
    pin = hashlib.sha256(image.data).hexdigest()
    if any(item.origin.image_sha256 != pin or item.origin.address_space != "pe:va" for item in bindings):
        raise ValueError("literal binding belongs to another original image/address space")
    targets, rows = {}, []
    for binding in bindings:
        targets[binding.unit, binding.symbol] = None
        row = {"interpretation_id": binding.interpretation_id, "symbol": binding.symbol,
               "unit": binding.unit, "address": f"0x{binding.origin.address:08X}",
               "source": {"path": binding.unit, "line": binding.line},
               "definition": Result.UNKNOWN, "content": Result.UNKNOWN}
        try:
            if binding.unit not in objects:
                raise ValueError((errors or {}).get(binding.unit, "literal owner was not compiled"))
            contents, storage = literal_storage(objects[binding.unit], binding.symbol)
            row.update(definition=Result.PASS, compiler_storage=storage)
            sections = [flags for (span, _, _), flags in zip(image.sections, image.section_flags)
                        if span.start <= binding.span.start < binding.span.end <= span.end]
            if (len(sections) != 1 or not sections[0] & 0x40000000
                    or sections[0] & (0x20 | 0x20000000)
                    or not sections[0] & (0x40 | 0x80)):
                raise ValueError("original literal requires one readable non-executable data section")
            if binding.span.end - binding.span.start > MAX_LITERAL_BYTES:
                raise ValueError("original literal exceeds bounded storage limit")
            proof = compare_bound_code(binding.span, image.read_initial(binding.span), contents, (), {},
                                       original_absolute_sites=image.absolute_sites(binding.span))
            row.update(content=proof["result"], proof=proof)
            if proof["result"] == Result.PASS:
                targets[binding.unit, binding.symbol] = BoundedTarget(binding.span)
        except ValueError as error:
            row["reason"] = str(error)
        rows.append(row)
    return targets, rows
