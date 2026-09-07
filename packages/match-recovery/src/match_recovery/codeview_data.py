"""Standalone CodeView 5 data extents, bound by COFF debug relocations.

Format: Microsoft microsoft-pdb/include/cvinfo.h (DATASYM32, LF_*_ST).
Names are diagnostic only: the SECTION/SECREL pair identifies the emitted
definition. No source spelling sizes, next-symbol guesses or PCH type voting.
This observes compiler storage extent, not original extent or C++ lifetime.
"""

from .codeview import Cursor, _type_records, _unqualified_type


def _class_record(leaf, record):
    cursor = Cursor(record.data[2:])
    count, properties, field = cursor.number("<H"), cursor.number("<H"), cursor.number("<I")
    derived, shape = (cursor.number("<I"), cursor.number("<I")) if leaf != 0x1006 else (0, 0)
    size, name = cursor.numeric(), cursor.name()
    cursor.padding()
    if cursor.at != len(cursor.data) or not name:
        raise ValueError("unsupported CodeView class storage record")
    if properties & 0x80 and any((count, field, derived, shape, size)):
        raise ValueError("inconsistent CodeView forward storage record")
    return properties, field, size, name


def storage_size(index, records, seen=frozenset()):
    index = _unqualified_type(index, records)
    if index is None or index in seen or len(seen) >= 32:
        raise ValueError("cyclic or invalid CodeView storage type")
    if 0 < index < 0x1000:
        mode, kind = index >> 8, index & 255
        scalar = {0x08: 4, 0x10: 1, 0x11: 2, 0x12: 4, 0x13: 8,
                  0x20: 1, 0x21: 2, 0x22: 4, 0x23: 8,
                  0x30: 1, 0x31: 2, 0x32: 4, 0x33: 8,
                  0x40: 4, 0x41: 8, 0x42: 10,
                  0x70: 1, 0x71: 2, 0x72: 2, 0x73: 2,
                  0x74: 4, 0x75: 4, 0x76: 8, 0x77: 8}
        if mode == 4 and (kind in scalar or kind == 3):
            return 4  # near32, not far32 or near64
        if mode == 0 and kind in scalar:
            return scalar[kind]
        raise ValueError("unsupported CodeView primitive storage type")
    if not 0 <= index - 0x1000 < len(records):
        raise ValueError("missing standalone CodeView storage type")
    leaf, record = records[index - 0x1000]
    cursor = Cursor(record.data[2:])
    if leaf == 0x1002:  # LF_POINTER: ordinary near32 pointers/references only
        target, attributes = cursor.number("<I"), cursor.number("<I")
        if (attributes & ~0xFFF or attributes & 31 != 0xA
                or (attributes >> 5) & 7 not in (0, 1)
                or not 0 < target < 0x1000 + len(records)):
            raise ValueError("unsupported CodeView pointer storage")
        size = 4
    elif leaf == 0x1003:  # LF_ARRAY_ST
        element, subscript = cursor.number("<I"), cursor.number("<I")
        size, _ = cursor.numeric(), cursor.name()
        width = storage_size(element, records, seen | {index})
        if not subscript or size <= 0 or size % width:
            raise ValueError("invalid CodeView array storage extent")
    elif leaf in (0x1004, 0x1005, 0x1006):  # class, struct, union
        properties, field, size, name = _class_record(leaf, record)
        if properties & 0x80:
            # CodeView identifies a forward UDT by its qualified name. Resolve
            # only one complete same-kind record in this standalone stream;
            # never borrow another TU's definition or vote among layouts.
            # https://lists.llvm.org/pipermail/llvm-dev/2015-October/091847.html
            complete = []
            for candidate, (kind, other) in enumerate(records):
                if kind != leaf:
                    continue
                options, _, _, other_name = _class_record(kind, other)
                if not options & 0x80 and other_name == name:
                    complete.append(candidate + 0x1000)
            if len(complete) != 1:
                raise ValueError("requires one complete CodeView forward storage definition")
            return storage_size(complete[0], records, seen | {index})
        if (size <= 0 or (field and
                (not 0 <= field - 0x1000 < len(records) or records[field - 0x1000][0] != 0x1203))):
            raise ValueError("requires complete CodeView object storage definition")
        return size
    elif leaf == 0x1007:  # enum: its underlying type determines storage
        cursor.take(4)  # count, properties
        target, field = cursor.number("<I"), cursor.number("<I")
        cursor.name()
        if field and (not 0 <= field - 0x1000 < len(records) or records[field - 0x1000][0] != 0x1203):
            raise ValueError("invalid CodeView enum field list")
        size = storage_size(target, records, seen | {index})
    else:
        raise ValueError(f"unsupported CodeView storage leaf {leaf:#x}")
    cursor.padding()
    if cursor.at != len(cursor.data):
        raise ValueError("unsupported CodeView storage record tail")
    return size


def data_layout(obj, name):
    """Require a uniquely defined external, with directly relocated debug data."""
    definitions = [s for s in obj.symbols.values() if s.name == name and s.storage == 2
                   and s.section > 0 and not s.type & 0x20]
    if len(definitions) != 1:
        raise ValueError("requires one defined external COFF data symbol")
    symbol = definitions[0]
    section = obj.sections[symbol.section - 1]
    if section.flags & 0x20 or not section.flags & (0x40 | 0x80) or section.name.startswith(".tls"):
        raise ValueError("unsupported COFF data storage section")
    records = _type_records(obj)
    if any(leaf in (0x0013, 0x0014, 0x0016, 0x100E, 0x1501, 0x1509, 0x1515) for leaf, _ in records):
        raise ValueError("requires standalone CodeView storage types, not PCH or type servers")
    debug_sections = [(i + 1, s) for i, s in enumerate(obj.sections) if s.name == ".debug$S"]
    roots = [number for number, section in debug_sections if not section.flags & 0x1000]
    if len(roots) != 1:
        raise ValueError("requires one standalone CodeView symbol stream")
    matches = []
    for number, debug in debug_sections:
        cursor = Cursor(debug.data)
        if number in roots:
            if cursor.number("<I") != 2:
                raise ValueError("unsupported CodeView symbol signature")
        elif number in obj.associations:
            # VC6 /Gy emits signatureless symbol-record fragments associated
            # with each code COMDAT. The validated section ownership, not a
            # guessed first word, distinguishes them from the signed root.
            parent = obj.associations[number]
            while parent in obj.associations:
                parent = obj.associations[parent]
            if not obj.sections[parent - 1].flags & 0x20:
                raise ValueError("unsupported CodeView symbol fragment owner")
        else:
            raise ValueError("unsupported CodeView symbol signature")
        while cursor.at < len(cursor.data):
            start = cursor.at
            size = cursor.number("<H")
            if size < 2:
                raise ValueError("invalid CodeView symbol record length")
            record = Cursor(cursor.take(size))
            leaf = record.number("<H")
            if leaf != 0x1008:  # S_GDATA32_ST, not thread/local/public records
                continue
            relocations = [(offset - start, index, kind) for offset, index, kind in debug.relocations
                           if start <= offset < cursor.at]
            if not any(index == symbol.index for _, index, _ in relocations):
                continue
            if sorted(relocations) != [(8, symbol.index, 11), (12, symbol.index, 10)]:
                raise ValueError("data debug record requires its exact SECTION/SECREL symbol pair")
            type_index, offset, segment = record.number("<I"), record.number("<I"), record.number("<H")
            source_name = record.name()
            if offset or segment or any(record.take(len(record.data) - record.at)):
                raise ValueError("unsupported data debug addends or record tail")
            extent = storage_size(type_index, records)
            if symbol.value + extent > obj.section_sizes[symbol.section - 1]:
                raise ValueError("data type extends beyond its COFF section")
            if any(s.index != symbol.index and s.section == symbol.section and s.storage in (2, 3)
                   and s.name != section.name and symbol.value <= s.value < symbol.value + extent
                   for s in obj.symbols.values()):
                raise ValueError("data type overlaps another COFF definition")
            matches.append({"symbol": name, "source_name": source_name, "section": symbol.section,
                            "offset": symbol.value, "size": extent, "type_index": type_index})
    if len(matches) != 1:
        raise ValueError("requires one directly bound CodeView data definition")
    return matches[0]
