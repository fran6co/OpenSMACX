"""Standalone CodeView 5 data extents, bound by COFF debug relocations.

Format: Microsoft microsoft-pdb/include/cvinfo.h (DATASYM32, LF_*_ST).
Names are diagnostic only: the SECTION/SECREL pair identifies the emitted
definition. No source spelling sizes, next-symbol guesses or PCH type voting.
This observes compiler storage extent, not original extent or C++ lifetime.

One narrow exception: VC6 emits no S_LDATA32 record for a function-local
static, even with /Z7. Such a symbol's extent is taken from its own C++
decoration and its COFF neighbour only when the two agree unambiguously
(see `_local_static_layout`).
"""

import re

from .codeview import Cursor, _fields, _type_records, _unqualified_type
from .pe_coff import static_name_matches
from .object_source import _MEMBER_SEGMENT, _ROOT_ARRAY_SEGMENT, array_slice_bounds


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
    """Measure one TU's external or static definition through its debug identity.

    Static storage is not an external relocation target. Callers must retain the
    owning object; this does not qualify same-spelled symbols in other TUs.
    """
    from .debug_type_twin import attached_twin
    witness = attached_twin(obj)
    if witness is not None:
        # Only a proved debug-typed twin of this same object supplies types.
        return witness.remap(data_layout(witness.twin, name))
    definitions = [s for s in obj.symbols.values() if s.storage in (2, 3)
                   and static_name_matches(s.name, s.storage, name)
                   and s.section > 0 and not s.type & 0x20]
    if len(definitions) != 1:
        raise ValueError("requires one defined external or static COFF data symbol")
    symbol = definitions[0]
    section = obj.sections[symbol.section - 1]
    if symbol.storage == 3 and (symbol.name == section.name or symbol.index in obj.auxiliary):
        raise ValueError("static data requires an ordinary definition, not section/auxiliary metadata")
    if section.flags & 0x20 or not section.flags & (0x40 | 0x80) or section.name.startswith(".tls"):
        raise ValueError("unsupported COFF data storage section")
    records = _type_records(obj)
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
            if leaf not in (0x1007, 0x1008):  # S_LDATA32_ST, S_GDATA32_ST, not TLS/public records
                continue
            relocations = [(offset - start, index, kind) for offset, index, kind in debug.relocations
                           if start <= offset < cursor.at]
            if not any(index == symbol.index for _, index, _ in relocations):
                continue
            if leaf != (0x1007 if symbol.storage == 3 else 0x1008):
                raise ValueError("CodeView data linkage disagrees with its COFF definition")
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
    if not matches and not any(index == symbol.index for _, debug in debug_sections
                               for _, index, _ in debug.relocations):
        local = _local_static_layout(obj, symbol, section)
        if local is not None:
            return local
    if len(matches) != 1:
        raise ValueError("requires one directly bound CodeView data definition")
    return matches[0]


# MSVC decoration of a function-local static: `?name@?<scope>@<enclosing
# function's decoration>@4<type><cv>`; VC6 prefixes the COFF name with `_`.
# Storage class 4 is "function-local static" in the MSVC name grammar.
_LOCAL_STATIC = re.compile(r"_?\?([A-Za-z_$][\w$]*)@\?(?:[0-9]|[A-P]+@)\?\?")
_PRIMITIVE_SIZE = {"C": 1, "D": 1, "E": 1, "F": 2, "G": 2, "H": 4, "I": 4, "J": 4, "K": 4,
                   "M": 4, "N": 8, "O": 8, "_J": 8, "_K": 8, "_N": 1}


def _decoded_number(text, at):
    """MSVC encoded dimension: digit d is d+1, else hex digits A-P ended by `@`."""
    if at < len(text) and text[at].isdigit():
        return int(text[at]) + 1, at + 1
    end = text.find("@", at)
    if end <= at or any(not "A" <= c <= "P" for c in text[at:end]):
        return None, at
    return int("".join("0123456789ABCDEF"[ord(c) - 65] for c in text[at:end]), 16), end + 1


def _local_static_type(text):
    """(element size, is_array) for a decorated variable type with storage cv.

    Only primitive scalars and `P<cv>Y<dims><primitive>` multidimensional
    arrays are decoded; MSVC drops the outer array bound, so an array yields
    its outer element size. A plain `P<cv><type>` is either a pointer or a
    one-dimensional array and stays undecided (None).
    """
    at, array, element = 0, False, 1
    if text[:1] == "P" and text[1:2] in ("A", "B", "C", "D") and text[2:3] == "Y":
        count, at = _decoded_number(text, 3)
        if not count:
            return None
        for _ in range(count):
            dim, at = _decoded_number(text, at)
            if not dim:
                return None
            element *= dim
        array = True
    code = text[at:at + 2] if text[at:at + 1] == "_" else text[at:at + 1]
    if code not in _PRIMITIVE_SIZE or text[at + len(code):] not in ("A", "B", "C", "D"):
        return None
    return element * _PRIMITIVE_SIZE[code], array


def _local_static_layout(obj, symbol, section):
    """COFF-observed extent of a VC6 function-local static without CodeView.

    Accepted only for a storage-class-3 definition in an ordinary (non-COMDAT,
    non-associative) data section whose decoration marks a function-local
    static of a decodable type, with no alias at its offset. The extent runs to
    the next definition in the section (or the section end); padding before
    that neighbour is bounded by the neighbour's offset alignment A, so the
    extent is unique only when a scalar fits with less than A of padding, or
    an array's outer element is at least A bytes and divides the span.
    """
    match = _LOCAL_STATIC.match(symbol.name)
    if (symbol.storage != 3 or match is None or section.flags & 0x1000
            or symbol.section in obj.associations):
        return None
    decoded = [found for found in (_local_static_type(symbol.name[at + 2:])
                                   for at in range(match.end(), len(symbol.name) - 2)
                                   if symbol.name[at:at + 2] == "@4") if found is not None]
    if len(decoded) != 1:
        return None
    element, array = decoded[0]
    others = [s for s in obj.symbols.values() if s.index != symbol.index and s.section == symbol.section
              and s.storage in (2, 3) and not (s.name == section.name and s.index in obj.auxiliary)]
    if any(s.value == symbol.value for s in others):
        return None
    following = [s.value for s in others if s.value > symbol.value]
    end = min(following) if following else obj.section_sizes[symbol.section - 1]
    gap = end - symbol.value
    alignment = min(16, end & -end) if following else 1
    if array:
        if element < alignment or gap <= 0 or gap % element:
            return None
        size = gap
    else:
        if not 0 <= gap - element < alignment:
            return None
        size = element
    return {"symbol": symbol.name, "source_name": match.group(1), "section": symbol.section,
            "offset": symbol.value, "size": size, "type_index": None,
            "extent_basis": "coff_local_static"}


def _complete_udt(index, records, seen=frozenset()):
    index = _unqualified_type(index, records)
    if index is None or index in seen or not 0 <= index - 0x1000 < len(records):
        raise ValueError("member path requires a complete embedded CodeView UDT")
    leaf, record = records[index - 0x1000]
    if leaf not in (0x1004, 0x1005, 0x1006):
        raise ValueError("member path crosses a non-UDT value")
    properties, field, size, name = _class_record(leaf, record)
    if properties & 0x80:
        complete = []
        for ordinal, (candidate_leaf, candidate) in enumerate(records):
            if candidate_leaf != leaf:
                continue
            candidate_properties, _, _, candidate_name = _class_record(candidate_leaf, candidate)
            if not candidate_properties & 0x80 and candidate_name == name:
                complete.append(ordinal + 0x1000)
        if len(complete) != 1:
            raise ValueError("member path requires one complete same-stream UDT")
        return _complete_udt(complete[0], records, seen | {index})
    if size <= 0 or (field and (not 0 <= field - 0x1000 < len(records)
                                or records[field - 0x1000][0] != 0x1203)):
        raise ValueError("member path requires a complete direct field list")
    members, member_types, bases = _fields(records[field - 0x1000][1].data[2:]) if field else ({}, {}, [])
    return index, size, members, member_types, bases


def _lookup_udt_member(index, name, records, active=frozenset()):
    """Resolve one direct or uniquely inherited nonvirtual member.

    Base offsets and extents come only from the same standalone CodeView
    stream. An unresolved base on a searched path prevents a uniqueness claim.
    """
    owner_type, owner_size, members, member_types, bases = _complete_udt(index, records)
    if owner_type in active:
        raise ValueError("cyclic CodeView base/member path")
    if name in members and name in member_types:
        offset, member_type = members[name], member_types[name]
        extent = storage_size(member_type, records)
        if offset < 0 or extent <= 0 or offset + extent > owner_size:
            raise ValueError("CodeView member lies outside its owning UDT")
        return offset, member_type

    matches = []
    next_active = active | {owner_type}
    for base in bases:
        if base["virtual"]:
            raise ValueError("inherited member lookup cannot resolve a virtual base offset")
        base_type, base_size, _, _, _ = _complete_udt(base["type_index"], records)
        base_offset = base["offset"]
        if base_offset < 0 or base_size <= 0 or base_offset + base_size > owner_size:
            raise ValueError("CodeView base lies outside its owning UDT")
        found = _lookup_udt_member(base_type, name, records, next_active)
        if found is not None:
            matches.append((base_offset + found[0], found[1]))
    if len(matches) > 1:
        raise ValueError("inherited CodeView member is ambiguous")
    return matches[0] if matches else None


def _integral_array_subscript(index, records, seen=frozenset()):
    """Accept only supported integer primitives or enums as array indices."""
    index = _unqualified_type(index, records)
    if index is None or index in seen:
        return False
    if 0 < index < 0x1000:
        return (index >> 8) == 0 and (index & 0xFF) in {
            0x08, 0x10, 0x11, 0x12, 0x13,
            0x20, 0x21, 0x22, 0x23,
            0x30, 0x31, 0x32, 0x33,
            0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
        }
    if not 0 <= index - 0x1000 < len(records):
        return False
    leaf, record = records[index - 0x1000]
    if leaf != 0x1007:
        return False
    cursor = Cursor(record.data[2:])
    try:
        cursor.take(4)  # enum count and properties
        underlying, _field = cursor.number("<I"), cursor.number("<I")
        cursor.name()
        cursor.padding()
        if cursor.at != len(cursor.data):
            return False
        storage_size(index, records, seen)
        return _integral_array_subscript(underlying, records, seen | {index})
    except ValueError:
        return False


def _array_element_layout(index, records):
    """Return the element shape of one complete LF_ARRAY_ST type."""
    index = _unqualified_type(index, records)
    if index is None or not 0 <= index - 0x1000 < len(records):
        raise ValueError("fixed member index requires a complete CodeView array")
    leaf, record = records[index - 0x1000]
    if leaf != 0x1003:
        raise ValueError("fixed member index crosses a non-array value")
    cursor = Cursor(record.data[2:])
    element, subscript = cursor.number("<I"), cursor.number("<I")
    size, _name = cursor.numeric(), cursor.name()
    cursor.padding()
    if cursor.at != len(cursor.data) or not subscript:
        raise ValueError("invalid CodeView array storage record")
    element_size = storage_size(element, records, frozenset({index}))
    # The declared byte extent, rather than source spelling or an inferred
    # element count, establishes both the stride and available fixed indices.
    storage_size(subscript, records)
    if not _integral_array_subscript(subscript, records):
        raise ValueError("CodeView array subscript type is not integral")
    if size <= 0 or element_size <= 0 or size % element_size:
        raise ValueError("invalid CodeView array storage extent")
    return element, size, element_size, size // element_size


def member_data_layout(obj, parent_symbol, member_path):
    """Resolve direct/unique nonvirtual inherited members and fixed elements.

    The emitted parent remains a real COFF data definition. Offsets and extents
    come only from its directly bound standalone CodeView records. Inherited
    lookup refuses unresolved base graphs rather than guessing offsets.
    """
    from .debug_type_twin import attached_twin
    witness = attached_twin(obj)
    if witness is not None:
        return witness.remap(member_data_layout(witness.twin, parent_symbol, member_path))
    bounds = array_slice_bounds(member_path)
    if bounds is not None:
        parent = data_layout(obj, parent_symbol)
        if parent["type_index"] is None:
            raise ValueError("array slice requires a CodeView-typed parent")
        records = _type_records(obj)
        element_type, array_size, element_size, element_count = _array_element_layout(
            parent["type_index"], records)
        begin, end = bounds
        if end > element_count:
            raise ValueError("array slice lies outside CodeView array extent")
        relative, size = begin * element_size, (end - begin) * element_size
        if relative + size > parent["size"]:
            raise ValueError("array slice lies outside parent storage")
        selection = {"begin": begin, "end": end, "offset": relative, "size": size,
                     "array_size": array_size, "element_size": element_size,
                     "element_count": element_count, "element_type_index": element_type,
                     "array_type_index": parent["type_index"]}
        return {"symbol": parent_symbol, "source_name": parent["source_name"],
                "section": parent["section"], "symbol_offset": parent["offset"],
                "size": size, "type_index": None, "selection_kind": "array_slice",
                "offset": parent["offset"] + relative, "member_offset": relative,
                "parent_size": parent["size"], "member_path": [selection]}
    if (not isinstance(member_path, tuple) or not member_path
            or any(not isinstance(part, str) or not (
                _MEMBER_SEGMENT.fullmatch(part)
                or (index == 0 and _ROOT_ARRAY_SEGMENT.fullmatch(part)))
                   for index, part in enumerate(member_path))):
        raise ValueError("member path must contain names and canonical fixed indices")
    parent = data_layout(obj, parent_symbol)
    if parent["type_index"] is None:
        raise ValueError("member path requires a CodeView-typed parent")
    records = _type_records(obj)
    current_type = parent["type_index"]
    relative = 0
    path_rows = []
    visited_udts = set()
    selected_size = None
    for path_index, part in enumerate(member_path):
        root_index = _ROOT_ARRAY_SEGMENT.fullmatch(part)
        if root_index is not None:
            if path_index != 0:
                raise ValueError("root array index is only valid as the first member path segment")
            element_type, array_size, element_size, element_count = _array_element_layout(
                current_type, records)
            element_index = int(root_index.group(1), 10)
            if element_index >= element_count:
                raise ValueError("fixed member index lies outside CodeView array extent")
            selected_size = element_size
            relative += element_index * element_size
            current_type = element_type
            path_rows.append({"index": element_index, "offset": element_index * element_size,
                              "size": element_size, "array_size": array_size,
                              "element_size": element_size, "type_index": element_type})
            continue
        match = _MEMBER_SEGMENT.fullmatch(part)
        assert match is not None
        name, raw_index = match.groups()
        owner_type = _complete_udt(current_type, records)[0]
        if owner_type in visited_udts:
            raise ValueError("cyclic embedded CodeView member path")
        visited_udts.add(owner_type)
        resolved = _lookup_udt_member(current_type, name, records)
        if resolved is None:
            raise ValueError("named CodeView member is missing or ambiguous")
        member_offset, member_type = resolved
        extent = storage_size(member_type, records)
        normalized_member = _unqualified_type(member_type, records)
        if normalized_member in visited_udts:
            raise ValueError("cyclic embedded CodeView member type")
        if extent <= 0:
            raise ValueError("CodeView member has no measurable storage extent")
        selected_type, selected_size, selected_offset = member_type, extent, member_offset
        if raw_index is not None:
            element_type, array_size, element_size, element_count = _array_element_layout(
                member_type, records)
            element_index = int(raw_index, 10)
            if element_index >= element_count:
                raise ValueError("fixed member index lies outside CodeView array extent")
            selected_type, selected_size = element_type, element_size
            selected_offset += element_index * element_size
            path_rows.append({"name": name, "offset": member_offset, "size": extent,
                              "type_index": member_type, "index": element_index,
                              "array_size": array_size, "element_size": element_size})
        else:
            path_rows.append({"name": name, "offset": member_offset, "size": extent,
                              "type_index": member_type})
        relative += selected_offset
        current_type = selected_type
    if selected_size is None:
        raise ValueError("member path did not select a storage element")
    if relative + selected_size > parent["size"]:
        raise ValueError("resolved member path lies outside parent storage")
    return {"symbol": parent_symbol, "source_name": parent["source_name"],
            "section": parent["section"], "symbol_offset": parent["offset"],
            "size": selected_size, "type_index": current_type,
            "offset": parent["offset"] + relative, "member_offset": relative,
            "parent_size": parent["size"], "member_path": path_rows}
