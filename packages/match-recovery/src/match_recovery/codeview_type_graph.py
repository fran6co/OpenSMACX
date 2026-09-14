"""Exact rooted CV5 type correspondence, including cycles and shared edges.

Only explicitly decoded type-index fields are renumbered. All remaining record
bytes, including names, properties, offsets and padding, are retained. Forward
declarations are not resolved to definitions; folding distinct type records is
not supported. This is type correspondence, not PDB/image or source authority.

TPI format: Microsoft's PDB/dbi/tpi.h (HDR/TpiHash), tpi.cpp
(acslValidateHdr), and include/cvinfo.h in microsoft/microsoft-pdb:
https://github.com/microsoft/microsoft-pdb/blob/master/PDB/dbi/tpi.h
https://github.com/microsoft/microsoft-pdb/blob/master/PDB/dbi/tpi.cpp
https://github.com/microsoft/microsoft-pdb/blob/master/include/cvinfo.h
"""

from collections.abc import Mapping
import struct
from types import MappingProxyType

from .codeview import _type_records
from .codeview_identity import reference_fields


MAX_RECORDS = 100000
MAX_BYTES = 16 * 1024 * 1024


def _index(value):
    if type(value) is not int or not 0 <= value < 0xFFFFFFFF:
        raise ValueError("invalid CodeView type index")
    return value


def _snapshot(records):
    if not isinstance(records, Mapping) or len(records) > MAX_RECORDS:
        raise ValueError("CodeView types require a bounded record mapping")
    result = dict(records)
    size = 0
    for index, data in result.items():
        if _index(index) < 0x1000 or type(data) is not bytes or not 2 <= len(data) <= 65535:
            raise ValueError("CodeView records require nonprimitive indices and immutable record bytes")
        leaf, = struct.unpack_from("<H", data)
        if leaf in (0x0013, 0x0014, 0x0016, 0x0200, 0x100E, 0x1200, 0x1501, 0x1509, 0x1515):
            raise ValueError("CodeView type mapping requires standalone indices, not skips, PCH or type servers")
        size += len(data)
    if size > MAX_BYTES:
        raise ValueError("CodeView type records exceed the byte bound")
    return result


def coff_type_records(obj):
    """Snapshot one standalone physical COFF type stream, without PCH imports."""
    records = {0x1000 + index: record.data for index, (_, record) in enumerate(_type_records(obj))}
    return MappingProxyType(_snapshot(records))


def tpi_type_records(data):
    """Decode the exact VC50 (19961031), 56-byte TPI header and record region.

    Input is stream 2, not the PDB container. Hash indices never supply type
    identity: their supported header ranges are checked, but their separate
    backing stream is not consumed or claimed verified. The native caller owns
    the container, image binding and physical lifetime. Other TPI versions and
    remapping/type-server records refuse instead of guessing a conversion.
    """
    if type(data) is not bytes or not 56 <= len(data) <= MAX_BYTES:
        raise ValueError("TPI requires bounded immutable stream bytes")
    version, header, first, end, size, stream, auxiliary, key_size, buckets = struct.unpack_from(
        "<IIIIIHHII", data)
    if (version != 19961031 or header != 56 or first != 0x1000
            or not first <= end <= first + MAX_RECORDS or size != len(data) - header):
        raise ValueError("unsupported or inconsistent VC50 TPI header/range")
    if key_size != 2 or buckets != 0x1000 or auxiliary != 0xFFFF:
        raise ValueError("unsupported VC50 TPI hash metadata")
    ranges = [struct.unpack_from("<ii", data, at) for at in (32, 40, 48)]
    if (any(offset < 0 or length < 0 or offset + length > 0x7FFFFFFF for offset, length in ranges)
            or ranges[0][1] != (end - first) * key_size or ranges[1][1] % 8
            or stream == 0xFFFF and any(length for _, length in ranges)
            or stream != 0xFFFF and stream < 4):
        raise ValueError("invalid or unsupported VC50 TPI hash ranges")
    occupied = sorted((offset, offset + length) for offset, length in ranges if length)
    if any(left[1] > right[0] for left, right in zip(occupied, occupied[1:])):
        raise ValueError("overlapping VC50 TPI hash ranges")
    records, at = {}, header
    for index in range(first, end):
        if at + 2 > len(data):
            raise ValueError("truncated TPI type record length")
        length, = struct.unpack_from("<H", data, at)
        if length < 2 or (length + 2) % 4 or at + 2 + length > len(data):
            raise ValueError("invalid TPI type record extent or alignment")
        record = data[at + 2:at + 2 + length]
        reference_fields(record)  # Refuse unsupported global index-context forms.
        records[index] = record
        at += length + 2
    if at != len(data):
        raise ValueError("TPI record count does not cover its complete record region")
    for record in records.values():
        if any(target >= 0x1000 and target not in records for _, target in reference_fields(record)):
            raise ValueError("TPI type reference is outside its record region")
    return MappingProxyType(records)


def compare_type_records(left, left_index, right, right_index):
    """Compare one rooted graph; use joint roots for procedures with locals."""
    return compare_type_roots(left, (left_index,), right, (right_index,))


def compare_type_roots(left, left_indices, right, right_indices):
    """Return a numbering-independent graph after exact bijective pairing.

    Root order and repetitions, edge order and sharing all survive. Iterative
    traversal handles cycles without bounded hash refinement or recursion. The
    returned JSON graph owns no references to input dictionaries, and primitive
    indices remain literal. No mismatched, absent or unsupported type is omitted.
    """
    left, right = _snapshot(left), _snapshot(right)
    if (not isinstance(left_indices, (tuple, list)) or not isinstance(right_indices, (tuple, list))
            or not 0 < len(left_indices) <= MAX_RECORDS or len(left_indices) != len(right_indices)):
        raise ValueError("CodeView correspondence requires equally sized nonempty root lists")
    left_indices, right_indices = tuple(left_indices), tuple(right_indices)
    left_ids, right_ids, pending, nodes = {}, {}, [], []

    def pair(a, b):
        _index(a)
        _index(b)
        if a < 0x1000 or b < 0x1000:
            if a != b:
                raise ValueError("CodeView primitive types differ")
            return {"primitive": a}
        if a not in left or b not in right:
            raise ValueError("CodeView type reference is outside its record mapping")
        if a in left_ids or b in right_ids:
            if a not in left_ids or b not in right_ids or left_ids[a] != right_ids[b]:
                raise ValueError("CodeView type sharing has inconsistent/nonbijective pairing")
            return {"node": left_ids[a]}
        index = len(pending)
        left_ids[a] = right_ids[b] = index
        pending.append((a, b))
        return {"node": index}

    roots = [pair(a, b) for a, b in zip(left_indices, right_indices)]
    at = 0
    while at < len(pending):
        a, b = pending[at]
        raw_a, raw_b = bytearray(left[a]), bytearray(right[b])
        edges_a, edges_b = reference_fields(left[a]), reference_fields(right[b])
        if tuple(offset for offset, _ in edges_a) != tuple(offset for offset, _ in edges_b):
            raise ValueError("CodeView type reference fields differ")
        for offset, _ in edges_a:
            raw_a[offset:offset + 4] = bytes(4)
            raw_b[offset:offset + 4] = bytes(4)
        if raw_a != raw_b:
            raise ValueError("CodeView type record payload differs")
        edges = [{"offset": offset, "target": pair(target, peer)}
                 for (offset, target), (_, peer) in zip(edges_a, edges_b)]
        nodes.append({"id": at, "payload": raw_a.hex(), "references": edges})
        at += 1
    return {"format": "codeview5_type_graph", "roots": roots, "nodes": nodes}
