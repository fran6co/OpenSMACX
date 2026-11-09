"""Source-bound TU literal storage, not address inference or linker selection.

The i386 COFF backend supports dedicated independent literal COMDATs. Their
physical section extent includes embedded NULs and wide-character storage;
neither a first terminator nor the instruction being checked chooses a bound.
"""

import hashlib
import re

from .binary import BoundedTarget, compare_bound_code
from .coff_comdat import comdat_key, comdat_selection
from .literal_source import validate_literals
from .model import Result


MAX_LITERAL_BYTES = 1024 * 1024
_VC6_REAL8 = re.compile(r"__real@8@[0-9a-f]{20}", re.ASCII)
_VC6_STATIC_STRING = re.compile(r"\$SG[0-9]+", re.ASCII)
_MAX_STATIC_ALIGNMENT = 8  # VC6 default /Zp8 natural alignment limit


def _static_string_storage(obj, symbol, declared_size):
    """Observe one unpooled VC6 ``$SG`` string inside its TU's ``.data``.

    Without ``/Gf`` VC6 emits each string literal as a static ``$SGn`` label
    in the ordinary multi-entry data section (an empty one in ``.bss``). The label value is the start.
    Its extent ends at the next label in that section (or the section end).
    Only an all-zero alignment tail may follow the declared extent, so the
    declared bound covers every content byte. The tail must end exactly at
    the next start rounded to a power-of-two alignment. A relocation inside
    the extent, an alias at the same start or a COMDAT section refuses.
    """
    section = obj.sections[symbol.section - 1]
    storage = section.flags & (0x40 | 0x80)
    if (section.flags & (0x20 | 0x20000000 | 0x1000) or not section.flags & 0x40000000
            or section.name.startswith(".tls") or storage not in (0x40, 0x80)
            or section.name.split("$")[0] != (".data", ".bss")[storage == 0x80]):
        raise ValueError("static string literal requires ordinary non-COMDAT .data/.bss storage")
    size = obj.section_sizes[symbol.section - 1]
    # An empty "" literal is emitted as one zero-filled .bss byte.
    data = bytes(size) if storage == 0x80 and not section.data else section.data
    if len(data) != size:
        raise ValueError("unsupported static string literal raw/zero-fill storage extent")
    starts = [item.value for item in obj.symbols.values()
              if item.section == symbol.section and item.index != symbol.index
              and not (item.name == section.name and item.storage == 3 and item.value == 0)]
    if symbol.value in starts:
        raise ValueError("static string literal shares its start with another definition or label")
    following = [value for value in starts if value > symbol.value]
    end = min(following) if following else size
    start = symbol.value
    if (type(declared_size) is not int or not 0 < declared_size <= end - start
            or end - start > MAX_LITERAL_BYTES):
        raise ValueError("declared static string literal extent exceeds its emitted label extent")
    stored = start + declared_size
    if any(offset < end and start < offset + 4 for offset, _target, _kind in section.relocations):
        raise ValueError("static string literal storage contains relocations")
    contents, tail = data[start:stored], data[stored:end]
    if contents[-1:] != b"\0" or any(tail):
        raise ValueError("static string literal extent must include its terminator and leave only zero padding")
    if tail:
        alignment = 1
        while alignment < _MAX_STATIC_ALIGNMENT and end % (alignment * 2) == 0:
            alignment *= 2
        if -(-stored // alignment) * alignment != end:
            raise ValueError("static string literal tail is not alignment padding before the next label")
    return contents, {"symbol": symbol.name, "section": symbol.section, "offset": start,
                      "size": declared_size, "label_extent": end - start, "flags": section.flags,
                      "basis": "static_string_label"}


def literal_storage(obj, name, declared_size=None):
    """Observe a whole independent MSVC string or VC6 double literal.

    Names select the declared candidate; they do not establish original
    identity. A static ``$SGn`` string label in multi-entry data is bounded
    by its declared size, checked against the label extent (see
    ``_static_string_storage``). Other static/ordinary/associative storage
    needs another extent witness and remains unsupported, never truncated to
    a C string.
    """
    definitions = [symbol for symbol in obj.symbols.values() if symbol.name == name]
    if len(definitions) != 1:
        raise ValueError("literal requires one unambiguous same-TU symbol")
    symbol = definitions[0]
    if _VC6_STATIC_STRING.fullmatch(name):
        if (symbol.section <= 0 or symbol.storage != 3 or symbol.type
                or obj.auxiliary.get(symbol.index)):
            raise ValueError("unsupported static string literal definition")
        return _static_string_storage(obj, symbol, declared_size)
    numeric = _VC6_REAL8.fullmatch(name) is not None
    if (not (name.startswith("??_C@") or numeric)
            or symbol.section <= 0 or symbol.storage != 2
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
    if numeric and (size != 8 or storage != 0x40 or section.flags & 0x80000000
                    or comdat_selection(obj, symbol.section) != 2):
        raise ValueError("VC6 double literal requires eight initialized read-only bytes in a selection-2 COMDAT")
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
            contents, storage = literal_storage(objects[binding.unit], binding.symbol,
                                                binding.span.end - binding.span.start)
            row.update(definition=Result.PASS, compiler_storage=storage)
            numeric = _VC6_REAL8.fullmatch(binding.symbol) is not None
            sections = [flags for (span, _, _), flags in zip(image.sections, image.section_flags)
                        if span.start <= binding.span.start < binding.span.end <= span.end]
            if (len(sections) != 1 or not sections[0] & 0x40000000
                    or sections[0] & (0x20 | 0x20000000)
                    or not sections[0] & (0x40 | 0x80)):
                raise ValueError("original literal requires one readable non-executable data section")
            if (numeric
                    and (binding.span.end - binding.span.start != 8
                         or sections[0] & 0x80000000
                         or sections[0] & (0x40 | 0x80) != 0x40)):
                raise ValueError("original VC6 double literal requires eight initialized read-only bytes")
            if binding.span.end - binding.span.start > MAX_LITERAL_BYTES:
                raise ValueError("original literal exceeds bounded storage limit")
            original = image.read(binding.span) if numeric else image.read_initial(binding.span)
            proof = compare_bound_code(binding.span, original, contents, (), {},
                                       original_absolute_sites=image.absolute_sites(binding.span))
            row.update(content=proof["result"], proof=proof)
            if proof["result"] == Result.PASS:
                targets[binding.unit, binding.symbol] = BoundedTarget(binding.span)
        except ValueError as error:
            row["reason"] = str(error)
        rows.append(row)
    return targets, rows
