"""Exact independent COFF COMDAT keys, never link-time selection authority.

Microsoft PE/COFF: the first symbol for a COMDAT section is its section
definition, and the second is its COMDAT key. A later same-named entry or
ordinary symbol cannot substitute. The native caller must bind parsed records
to fresh physical inputs and independently prove the retained winner.
https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#comdat-sections-object-only
"""

import struct

from .pe_coff import Coff, _slice, _unpack


def _function_auxiliary(obj, key):
    """Validate auxiliary formats 1/2 and their original COFF line table.

    This is format coherence, not a function extent/placement permission.
    Alignment padding remains part of the physical COMDAT in native proofs.
    Microsoft PE/COFF: Auxiliary Format 1: Function Definitions; Auxiliary
    Format 2: .bf and .ef Symbols; COFF Line Numbers (Deprecated).
    """
    from .parsed_state import same

    data = getattr(obj, "raw_data", None)
    if type(data) is not bytes or not same(obj.__dict__, Coff(data).__dict__):
        raise ValueError("function auxiliary parsed state differs from physical COFF")
    raw = obj.auxiliary[key.index]
    section = obj.sections[key.section - 1]
    if (len(raw) != 18 or key.type != 0x20 or not section.flags & 0x20
            or len(section.data) != obj.section_sizes[key.section - 1]
            or obj.function(key.name) != key):
        raise ValueError("unsupported COMDAT function-definition auxiliary")
    tag, size, lines, next_function, unused = struct.unpack("<IIIIH", raw)
    if unused or not 0 < size <= len(section.data):
        raise ValueError("invalid COMDAT function auxiliary size or unused fields")
    markers = sorted((item for item in obj.symbols.values()
                      if item.section == key.section and item.storage == 101),
                     key=lambda item: item.index)
    if (len(markers) != 3 or [item.name for item in markers] != [".bf", ".lf", ".ef"]
            or any(item.type != 0 for item in markers)):
        raise ValueError("function auxiliary requires exact .bf/.lf/.ef records")
    begin, count, end = markers
    if (tag != begin.index or not key.index < begin.index < count.index < end.index
            or begin.value != 0 or end.value != size or obj.auxiliary.get(count.index)):
        raise ValueError("function auxiliary markers contradict original function")
    begin_raw = obj.auxiliary.get(begin.index, b"")
    end_raw = obj.auxiliary.get(end.index, b"")
    for record in (begin_raw, end_raw):
        if (len(record) != 18 or any(record[:4]) or any(record[6:12])
                or any(record[16:]) or not _unpack("<H", record, 4)[0]):
            raise ValueError("unsupported function boundary auxiliary")
    if any(end_raw[12:16]):
        raise ValueError("end-function auxiliary has nonzero unused pointer")
    following = sorted((item for item in obj.symbols.values()
                        if item.index > key.index and item.section > 0
                        and item.storage == 2 and item.type == 0x20),
                       key=lambda item: item.index)
    expected_next = following[0].index if following else 0
    if next_function != expected_next:
        raise ValueError("function auxiliary next-function pointer is not the next definition")
    next_begin = _unpack("<I", begin_raw, 12)[0]
    if following:
        target = following[0]
        target_raw = obj.auxiliary.get(target.index, b"")
        if len(target_raw) != 18 or end.index >= target.index:
            raise ValueError("next function has unsupported auxiliary or overlapping symbol scope")
        target_tag = _unpack("<I", target_raw, 0)[0]
        target_marker = obj.symbols.get(target_tag)
        target_begins = [item for item in obj.symbols.values()
                         if item.section == target.section and item.storage == 101
                         and item.name == ".bf"]
        if (next_begin != target_tag or target_marker is None
                or target_marker.name != ".bf" or target_marker.storage != 101
                or target_marker.type != 0 or target_marker.section != target.section
                or target_marker.index <= target.index or target_begins != [target_marker]
                or obj.function(target.name) != target):
            raise ValueError("next begin-function pointer is not independently bound")
    elif next_begin:
        raise ValueError("last begin-function has a nonzero next pointer")
    _function_lines(obj, key, size, lines)


def _function_lines(obj, key, size, pointer):
    data = obj.raw_data
    header = 20 + (key.section - 1) * 40
    table = _unpack("<I", data, header + 28)[0]
    count = _unpack("<H", data, header + 34)[0]
    definition = min((item for item in obj.symbols.values()
                      if item.section == key.section), key=lambda item: item.index)
    declared = _unpack("<H", obj.auxiliary[definition.index], 6)[0]
    if count != declared or pointer != table or bool(pointer) != bool(count):
        raise ValueError("function line table contradicts section/definition metadata")
    if not count:
        return
    _slice(data, table, count * 6)
    begin, end = table, table + count * 6
    protected = [(0, 20 + len(obj.sections) * 40)]
    symbols, symbols_count = _unpack("<II", data, 8)
    strings = symbols + symbols_count * 18
    protected.append((symbols, strings + _unpack("<I", data, strings)[0]))
    for number in range(1, len(obj.sections) + 1):
        at = 20 + (number - 1) * 40
        raw_size, raw_pointer, reloc_pointer, line_pointer, relocs, lines = _unpack("<IIIIHH", data, at + 16)
        if raw_pointer and raw_size:
            protected.append((raw_pointer, raw_pointer + raw_size))
        if relocs:
            protected.append((reloc_pointer, reloc_pointer + relocs * 10))
        if number != key.section and lines:
            protected.append((line_pointer, line_pointer + lines * 6))
    if any(begin < stop and start < end for start, stop in protected):
        raise ValueError("function line table aliases another physical COFF structure")
    if _unpack("<IH", data, table) != (key.index, 0):
        raise ValueError("function line table does not start at its original function symbol")
    for index in range(1, count):
        address, line = _unpack("<IH", data, table + index * 6)
        if not line or address >= size:
            raise ValueError("function line entry has an unsupported function/offset")


def _definition(obj, number):
    if (type(obj) is not Coff or type(number) is not int
            or not 1 <= number <= len(obj.sections)):
        raise ValueError("COMDAT metadata requires a strict COFF object and exact section")
    section = obj.sections[number - 1]
    if not section.flags & 0x1000 or section.name.startswith("/"):
        raise ValueError("COMDAT metadata requires an explicitly named COMDAT section")
    symbols = sorted((symbol for symbol in obj.symbols.values() if symbol.section == number),
                     key=lambda symbol: symbol.index)
    if not symbols:
        raise ValueError("COMDAT section has no section-definition symbol")
    definition = symbols[0]
    raw = obj.auxiliary.get(definition.index, b"")
    if (definition.name != section.name or definition.value != 0
            or definition.type != 0 or definition.storage != 3 or len(raw) != 18):
        raise ValueError("first COMDAT section symbol is not its exact section definition")
    size, relocations, _, _, parent, selection = struct.unpack_from("<IHHIHB", raw)
    if (size != obj.section_sizes[number - 1] or relocations != len(section.relocations)
            or selection not in range(1, 7) or any(raw[15:])):
        raise ValueError("COMDAT section definition is inconsistent")
    if selection == 5:
        if obj.associations.get(number) != parent:
            raise ValueError("COMDAT association differs from its parsed parent")
    elif parent or number in obj.associations:
        raise ValueError("independent COMDAT has unexplained parent metadata")
    return symbols, selection


def comdat_selection(obj, section):
    """Read the exact section policy; no winner or discard is inferred."""
    return _definition(obj, section)[1]


def comdat_key(obj, section):
    """Return the exact external key of an independent code/data COMDAT.

    Associative children follow their parent; they do not get independent keys
    through a convenient symbol spelling. Unsupported local-key forms refuse.
    """
    symbols, selection = _definition(obj, section)
    if selection == 5 or len(symbols) < 2:
        raise ValueError("COMDAT key requires an independent section and its second symbol")
    key = symbols[1]
    if (key.storage != 2 or key.value != 0 or key.type not in (0, 0x20)
            or not key.name or key.name == obj.sections[section - 1].name):
        raise ValueError("second COMDAT section symbol is not a supported external key")
    # A removed parsed auxiliary must not silently become an auxiliary-less
    # supported key. Original symbol-table framing still decides this branch.
    data = getattr(obj, "raw_data", None)
    if type(data) is not bytes:
        raise ValueError("COMDAT key lacks immutable physical COFF bytes")
    symbol_table, symbol_count = _unpack("<II", data, 8)
    if not 0 <= key.index < symbol_count:
        raise ValueError("COMDAT key index lies outside physical symbol records")
    physical_aux = _slice(data, symbol_table + key.index * 18 + 17, 1)[0]
    if obj.auxiliary.get(key.index) or physical_aux:
        _function_auxiliary(obj, key)
    return key
