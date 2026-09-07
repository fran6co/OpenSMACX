"""Exact contents of supported acyclic CodeView 5 types, modulo numbering.

Only fields declared to be type indices are replaced with referenced contents.
All other bytes (including names, attributes, sizes, offsets and padding) remain
in the identity. Duplicate records retain their multiplicity. This is compiler
output comparison, not proof of the recovered program's class model.

Record layouts: Microsoft's microsoft-pdb/include/cvinfo.h (32-bit ST leaves).
https://github.com/microsoft/microsoft-pdb/blob/master/include/cvinfo.h
"""

from collections import defaultdict, deque

from .codeview import Cursor, _type_records
from .model import digest


def reference_fields(data: bytes) -> tuple[tuple[int, int], ...]:
    """Decode index positions; never search raw bytes for index-like integers."""
    cursor = Cursor(data)
    leaf = cursor.number("<H")
    references = []

    def ref():
        at = cursor.at
        references.append((at, cursor.number("<I")))

    def method():
        attr = cursor.number("<H")
        if cursor.number("<H") != 0:
            raise ValueError("nonzero CodeView method-list padding")
        ref()
        if ((attr >> 2) & 7) in (4, 6):
            cursor.take(4)  # Introducing virtual: vtable offset, NOT a type.

    if leaf == 0x000A:  # LF_VTSHAPE
        cursor.take((cursor.number("<H") + 1) // 2)
    elif leaf == 0x1001:  # LF_MODIFIER
        ref()
        cursor.take(2)
    elif leaf == 0x1002:  # LF_POINTER, including member pointers.
        ref()
        attributes = cursor.number("<I")
        mode = (attributes >> 5) & 7
        if attributes & ~0xFFF or attributes & 31 != 0xA or mode not in (0, 1, 2, 3):
            raise ValueError("unsupported CodeView pointer model for replica identity")
        if mode in (2, 3):
            ref()  # Containing class, not the member target.
            cursor.take(2)  # Member-pointer representation.
    elif leaf == 0x1003:  # LF_ARRAY_ST
        ref()
        ref()
        cursor.numeric()
        cursor.name()
    elif leaf in (0x1004, 0x1005, 0x1006, 0x1007):  # Class, structure, union, enum.
        cursor.take(4)  # Count and properties.
        ref()
        if leaf in (0x1004, 0x1005):
            ref()
            ref()
        elif leaf == 0x1007:
            ref()
        if leaf != 0x1007:
            cursor.numeric()
        cursor.name()
    elif leaf in (0x1008, 0x1009):  # LF_PROCEDURE / LF_MFUNCTION
        ref()
        if leaf == 0x1009:
            ref()
            ref()
        cursor.take(4)  # Calling convention, attributes and parameter count.
        ref()
        if leaf == 0x1009:
            cursor.take(4)  # this adjustment, NOT a type index.
    elif leaf in (0x1201, 0x1204):  # LF_ARGLIST / LF_DERIVED
        count = cursor.number("<I")
        if count > (len(data) - cursor.at) // 4:
            raise ValueError("truncated CodeView type-index list")
        for _ in range(count):
            ref()
    elif leaf == 0x1205:  # LF_BITFIELD
        ref()
        cursor.take(2)
    elif leaf == 0x1206:  # LF_METHODLIST
        while len(data) - cursor.at > 3:
            method()
    elif leaf == 0x1203:  # LF_FIELDLIST
        while cursor.at < len(data):
            kind = cursor.number("<H")
            attr = cursor.number("<H")
            if kind == 0x0403:  # LF_ENUMERATE_ST: numeric value then name.
                cursor.numeric()
                cursor.name()
            elif kind in (0x1400, 0x1401, 0x1402):
                ref()
                if kind != 0x1400:
                    ref()
                    cursor.numeric()
                cursor.numeric()
            elif kind in (0x1403, 0x1404, 0x1405, 0x1406, 0x1407,
                          0x1408, 0x1409, 0x140A, 0x140B, 0x140C, 0x140D):
                ref()
                if kind == 0x1405:
                    cursor.numeric()
                if kind == 0x140C or (kind == 0x140B and ((attr >> 2) & 7) in (4, 6)):
                    cursor.take(4)
                if kind in (0x1403, 0x1405, 0x1406, 0x1407, 0x1408, 0x140B, 0x140D):
                    cursor.name()
            else:
                raise ValueError(f"unsupported CodeView replica field leaf {kind:#x}")
            cursor.padding()
    else:
        raise ValueError(f"unsupported CodeView replica type leaf {leaf:#x}")
    cursor.padding()
    if cursor.at != len(data):
        raise ValueError("unsupported CodeView replica record tail")
    return tuple(references)


def type_identity(obj) -> tuple[str, ...]:
    """Full type definitions, not compiler emission order or type-index values.

    Forward UDT declarations keep their names/properties and are distinct from
    definitions. Cycles and unsupported forms are refused, not approximated by
    a fixed number of hash rounds. Callers may retain exact raw-stream comparison
    as a conservative fallback; they must never omit an unsupported stream.
    """
    records = _type_records(obj)
    nodes, users, dependencies = [], defaultdict(set), {}
    for index, (_, record) in enumerate(records):
        fields = reference_fields(record.data)
        payload = bytearray(record.data)
        dependencies[index] = set()
        for at, target in fields:
            if target >= 0x1000:
                target_index = target - 0x1000
                if target_index >= len(records):
                    raise ValueError("CodeView replica type index is outside the stream")
                dependencies[index].add(target_index)
                users[target_index].add(index)
            payload[at:at + 4] = bytes(4)
        nodes.append((payload.hex(), fields))
    ready = deque(index for index, targets in dependencies.items() if not targets)
    identities = {}
    while ready:
        index = ready.popleft()
        payload, fields = nodes[index]
        identities[index] = digest({"payload": payload, "references": [
            (at, ("primitive", target) if target < 0x1000 else ("record", identities[target - 0x1000]))
            for at, target in fields]})
        for user in users[index]:
            dependencies[user].remove(index)
            if not dependencies[user]:
                ready.append(user)
    if len(identities) != len(records):
        raise ValueError("cyclic CodeView types require an explicit identity verifier")
    return tuple(sorted(identities.values()))
