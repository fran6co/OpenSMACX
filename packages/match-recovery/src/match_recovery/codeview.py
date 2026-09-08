"""Strict direct-member layout observations from standalone CodeView 5 objects.

No type-base voting, partial field lists, silent numeric defaults or PCH
inheritance is used.
An unsupported class is unmeasurable, not an empty or partially verified class.
This proves compiler placement only; original layout expectations need evidence.
"""

import struct

from match_recovery.pe_coff import Coff


class ClassNotEmitted(ValueError):
    """Valid standalone type stream, but no complete definition of this class.

    This is missing coverage, not evidence that the TU cannot depend on its ABI.
    """


class Cursor:
    def __init__(self, data: bytes):
        self.data, self.at = data, 0

    def take(self, size):
        if size < 0 or self.at + size > len(self.data):
            raise ValueError("truncated CodeView record")
        value = self.data[self.at:self.at + size]
        self.at += size
        return value

    def number(self, fmt):
        return struct.unpack(fmt, self.take(struct.calcsize(fmt)))[0]

    def numeric(self):
        value = self.number("<H")
        if value < 0x8000:
            return value
        formats = {0x8000: "<b", 0x8001: "<h", 0x8002: "<H", 0x8003: "<i",
                   0x8004: "<I", 0x8009: "<q", 0x800A: "<Q"}
        if value not in formats:
            raise ValueError("unsupported CodeView numeric encoding")
        return self.number(formats[value])

    def name(self):
        return self.take(self.number("<B")).decode("latin-1")

    def padding(self):
        if self.at < len(self.data) and self.data[self.at] >= 0xF0:
            count = self.data[self.at] - 0xF0
            if not count or self.take(count) != bytes(range(0xF0 + count, 0xF0, -1)):
                raise ValueError("invalid CodeView padding")


def _fields(data: bytes) -> tuple[dict, dict, list]:
    cursor, members, member_types, bases = Cursor(data), {}, {}, []
    while cursor.at < len(data):
        leaf = cursor.number("<H")
        if leaf == 0x1405:  # LF_MEMBER
            cursor.take(2)  # attributes
            member_type = cursor.number("<I")
            offset, name = cursor.numeric(), cursor.name()
            if offset < 0 or name in members:
                raise ValueError("invalid or duplicate direct member")
            members[name] = offset
            member_types[name] = member_type
        elif leaf == 0x1400:  # nonvirtual base
            cursor.take(2)
            base_type, offset = cursor.number("<I"), cursor.numeric()
            if offset < 0:
                raise ValueError("negative nonvirtual base offset")
            bases.append({"type_index": base_type, "virtual": False, "offset": offset})
        elif leaf in (0x1401, 0x1402):  # virtual bases: do not claim their offsets
            cursor.take(2)
            base_type, vbptr_type = cursor.number("<I"), cursor.number("<I")
            bases.append({"type_index": base_type, "virtual": True, "offset": None,
                          "indirect": leaf == 0x1402, "vbptr_type_index": vbptr_type,
                          "vbptr_offset": cursor.numeric(), "vbtable_index": cursor.numeric()})
        elif leaf == 0x140B:  # one method, optionally introducing virtual
            attribute = cursor.number("<H")
            cursor.take(4)
            if ((attribute >> 2) & 7) in (4, 6):
                cursor.take(4)
            cursor.name()
        elif leaf in (0x1403, 0x1406, 0x1407, 0x1408):
            cursor.take(6)
            cursor.name()
        elif leaf in (0x1409, 0x140A):
            cursor.take(6)
        else:
            # In particular LF_INDEX continuations are not silently skipped.
            raise ValueError(f"unsupported CodeView field leaf {leaf:#x}")
        cursor.padding()
    return members, member_types, bases


def direct_members(data: bytes) -> dict[str, int]:
    return _fields(data)[0]


def _unqualified_type(type_index: int, records: list) -> int | None:
    """Follow only validated const/volatile wrappers, never pointer targets."""
    visited = set()
    while type_index not in visited:
        visited.add(type_index)
        index = type_index - 0x1000
        if not 0 <= index < len(records) or records[index][0] != 0x1001:
            return type_index
        cursor = Cursor(records[index][1].data[2:])
        try:
            target, attributes = cursor.number("<I"), cursor.number("<H")
            cursor.padding()
            if attributes & ~3 or cursor.at != len(cursor.data):
                return None
        except ValueError:
            return None
        type_index = target
    return None  # A cycle cannot establish an embedded class type.


def _member_type(type_index, records, classes, seen=frozenset()):
    """Observe unqualified storage shape, not ownership or full C++ type equality.

    LF_POINTER also represents references and member pointers. A leaf number
    alone is not proof of ordinary pointer storage. CV5 near32 pointers and
    lvalue references are supported; other models/modes remain unknown.
    Format: Microsoft's microsoft-pdb/include/cvinfo.h, lfPointerBody.
    """
    unknown = {"kind": "unknown"}
    type_index = _unqualified_type(type_index, records)
    if type_index is None or type_index in seen or len(seen) >= 16:
        return unknown
    if type_index in classes:
        return {"kind": "class_value", "class": classes[type_index]["name"]}
    if 0 < type_index < 0x1000:
        return {"kind": "non_class_value"}
    index = type_index - 0x1000
    if not 0 <= index < len(records):
        return unknown
    leaf, record = records[index]
    cursor = Cursor(record.data[2:])
    try:
        if leaf == 0x1002:
            target, attributes = cursor.number("<I"), cursor.number("<I")
            mode = (attributes >> 5) & 7
            # CV5's near32 model supplies width; later size/restrict/managed
            # encodings are not silently interpreted as this older format.
            if attributes & ~0xFFF or attributes & 31 != 0xA or mode not in (0, 1):
                return unknown
            cursor.padding()
            if cursor.at != len(cursor.data):
                return unknown
            target_type = _member_type(target, records, classes, seen | {type_index})
            if target_type["kind"] == "unknown":
                return unknown
            return {"kind": "pointer" if mode == 0 else "reference", "width": 4, "target": target_type}
        if leaf == 0x1003:
            target, index_type = cursor.number("<I"), cursor.number("<I")
            size = cursor.numeric()
            cursor.name()
            cursor.padding()
            if size < 0 or not index_type or cursor.at != len(cursor.data):
                return unknown
            target_type = _member_type(target, records, classes, seen | {type_index})
            if target_type["kind"] == "unknown":
                return unknown
            # The array leaf declares total bytes, not the element count.
            # Resolve element storage within this same standalone stream;
            # names alone (especially forward UDTs) cannot supply a stride.
            from .codeview_data import storage_size
            try:
                width = storage_size(target, records, seen | {type_index})
            except ValueError:
                width = None
            if width is not None and (width <= 0 or size <= 0 or size % width):
                return unknown
            return {"kind": "array", "target": target_type, "size": size,
                    "element_size": width, "count": size // width if width else None}
    except ValueError:
        return unknown
    return unknown


def _type_records(obj: Coff):
    """Read the standalone type stream shared by class and object observers."""
    sections = [section for section in obj.sections if section.name == ".debug$T"]
    if len(sections) != 1 or any(section.name == ".debug$P" for section in obj.sections):
        raise ValueError("requires one standalone CodeView type section")
    cursor = Cursor(sections[0].data)
    if cursor.number("<I") != 2:
        raise ValueError("unsupported CodeView type signature")
    records = []
    while cursor.at < len(cursor.data):
        size = cursor.number("<H")
        if size < 2:
            raise ValueError("invalid CodeView type length")
        record = Cursor(cursor.take(size))
        leaf = record.number("<H")
        # Imported/precompiled types change the type-index context. A local
        # definition beside such a record is not standalone evidence. Apply
        # this boundary to both class and data observers, before returning an
        # index that either could mistake for a complete type stream.
        if leaf in (0x0013, 0x0014, 0x0016, 0x100E, 0x1501, 0x1509, 0x1515):
            raise ValueError("requires standalone CodeView types, not PCH or type servers")
        records.append((leaf, record))
    return records


def _class_records(obj: Coff):
    """Validate and index class headers once, without guessing field lists."""
    records = _type_records(obj)
    classes = {}
    for index, (leaf, record) in enumerate(records):
        if leaf not in (0x1004, 0x1005):
            continue
        record.take(2)  # declared field count includes methods/bases
        properties = record.number("<H")
        field = record.number("<I")
        record.take(8)
        size, class_name = record.numeric(), record.name()
        record.padding()
        if record.at != len(record.data):
            raise ValueError("unsupported class record tail")
        classes[index + 0x1000] = {"name": class_name, "properties": properties, "field": field, "size": size}
    return records, classes


class ClassLayouts:
    """An invocation-local index of one supplied standalone object's types.

    No objects or verdicts are shared across compilations. Class field lists
    are checked on lookup, so one unsupported class does not erase observable
    siblings. Each result is newly constructed and safe for its caller to edit.
    """

    def __init__(self, obj: Coff):
        self._records, self._classes = _class_records(obj)

    def layout(self, name: str) -> dict:
        return _class_layout(self._records, self._classes, name)


def _class_layout(records, classes, name):
    definitions = []
    for cls in classes.values():
        class_name, properties, field, size = (cls[key] for key in ("name", "properties", "field", "size"))
        if class_name != name or properties & 0x80:
            continue
        index = field - 0x1000
        if size < 0 or not 0 <= index < len(records) or records[index][0] != 0x1203:
            raise ValueError("invalid standalone class field-list reference")
        members, member_types, bases = _fields(records[index][1].data[2:])
        if any(offset >= size for offset in members.values()):
            raise ValueError("member offset outside compiler class extent")
        named_bases = []
        for base in bases:
            base_class = classes.get(base["type_index"])
            if base_class is None:
                raise ValueError("base type does not reference a standalone class record")
            if not base["virtual"] and base["offset"] >= size:
                raise ValueError("base offset outside compiler class extent")
            named_bases.append({**base, "class": base_class["name"]})
        observed_types = {member: _member_type(type_index, records, classes)
                          for member, type_index in member_types.items()}
        if any(members[member] + value.get("width", value.get("size", 0)) > size
               for member, value in observed_types.items()):
            raise ValueError("member storage outside compiler class extent")
        class_members = {member: value["class"] for member, value in observed_types.items()
                         if value["kind"] == "class_value"}
        type_kinds = {member: value["kind"] for member, value in observed_types.items()}
        definitions.append({"class": name, "size": size, "members": members,
                            "bases": named_bases, "class_value_members": class_members,
                            "member_type_kinds": type_kinds, "member_types": observed_types})
    if not definitions:
        raise ClassNotEmitted("requires one complete compiler class definition; none emitted")
    if len(definitions) != 1:
        raise ValueError("requires one complete compiler class definition")
    return definitions[0]


def class_layout(obj: Coff, name: str) -> dict:
    """Single lookup; use ClassLayouts for several names in the same object.

    Caller must compile a standalone /Z7 TU, explicitly without PCH use.
    """
    return ClassLayouts(obj).layout(name)
