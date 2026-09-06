"""Strict direct-member layout observations from standalone VC6 /Z7 objects.

The CV5 shapes are shared with the project's debugger reader, but no type-base
voting, partial field lists, silent numeric defaults or PCH inheritance is used.
An unsupported class is unmeasurable, not an empty or partially verified class.
This proves compiler placement only; original layout expectations need evidence.
"""

import struct

from ..pe_coff import Coff


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


def direct_members(data: bytes) -> dict[str, int]:
    cursor, members = Cursor(data), {}
    while cursor.at < len(data):
        leaf = cursor.number("<H")
        if leaf == 0x1405:  # LF_MEMBER
            cursor.take(6)  # attributes, type index
            offset, name = cursor.numeric(), cursor.name()
            if offset < 0 or name in members:
                raise ValueError("invalid or duplicate direct member")
            members[name] = offset
        elif leaf == 0x1400:  # nonvirtual base
            cursor.take(6)
            cursor.numeric()
        elif leaf in (0x1401, 0x1402):  # virtual bases: do not claim their offsets
            cursor.take(10)
            cursor.numeric()
            cursor.numeric()
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
    return members


def class_layout(obj: Coff, name: str) -> dict:
    """Caller must compile a standalone /Z7 TU, explicitly without PCH use."""
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
        records.append((record.number("<H"), record))
    definitions = []
    for leaf, record in records:
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
        if class_name != name or properties & 0x80:
            continue
        index = field - 0x1000
        if size < 0 or not 0 <= index < len(records) or records[index][0] != 0x1203:
            raise ValueError("invalid standalone class field-list reference")
        members = direct_members(records[index][1].data[2:])
        if any(offset >= size for offset in members.values()):
            raise ValueError("member offset outside compiler class extent")
        definitions.append({"class": name, "size": size, "members": members})
    if len(definitions) != 1:
        raise ValueError("requires one complete compiler class definition")
    return definitions[0]
