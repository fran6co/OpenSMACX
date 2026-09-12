"""Bounded embedded COFF debug symbols, not object ownership or code proof.

Formats: https://learn.microsoft.com/en-us/windows/win32/debug/pe-format
https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-image_coff_symbols_header
The caller owns immutable image provenance. Overlay file offsets are not RVAs;
linked symbol values are RVAs, not offsets within their numbered PE section.
"""

import struct

from .model import Span
from .pe_coff import Symbol


MAX_DEBUG_ENTRIES = 4096
MAX_DEBUG_BYTES = 64 * 1024 * 1024
MAX_SYMBOLS = 1_000_000


def _bytes(raw, offset, size):
    if offset < 0 or size < 0 or offset + size > len(raw):
        raise ValueError("truncated embedded COFF debug evidence")
    return raw[offset:offset + size]


def _mapped(image, rva, size):
    if not rva or size <= 0 or image.base + rva + size > 0x100000000:
        raise ValueError("invalid mapped debug metadata extent")
    span = Span(image.base + rva, image.base + rva + size)
    raw = image.read(span)
    if image.absolute_sites(span):
        raise ValueError("debug metadata has loader base relocations")
    matches = [(region, offset) for region, offset, length in image.sections
               if region.start <= span.start and span.end <= region.start + length]
    if len(matches) != 1:
        raise ValueError("debug metadata lacks unique physical section backing")
    region, offset = matches[0]
    return raw, offset + span.start - region.start


def linked_coff_symbols(image) -> tuple[Symbol, ...]:
    """Read one complete embedded COFF table, or refuse malformed evidence.

    Symbols retain actual table indices and duplicate names. ``value`` remains
    the linked RVA for positive-section records. No adjacent .file or compiler
    record supplies an owner. Mapped records with auxiliaries are unsupported;
    non-placement auxiliaries are bounded and skipped, never read as symbols.
    Absent COFF debug information yields an empty tuple, not placement evidence.
    """
    raw = image.data
    pe, = struct.unpack("<I", _bytes(raw, 0x3C, 4))
    file_table, file_count = struct.unpack("<II", _bytes(raw, pe + 12, 8))
    if bool(file_table) != bool(file_count):
        raise ValueError("incomplete PE COFF symbol-table identity")
    rva, size = image.directories[6] if len(image.directories) > 6 else (0, 0)
    if (rva, size) == (0, 0):
        if file_table or file_count:
            raise ValueError("PE COFF symbols lack an embedded debug directory")
        return ()
    if not rva or not size or size % 28 or size // 28 > MAX_DEBUG_ENTRIES:
        raise ValueError("invalid PE debug directory extent")
    section_count, = struct.unpack("<H", _bytes(raw, pe + 6, 2))
    optional_size, = struct.unpack("<H", _bytes(raw, pe + 20, 2))
    header_size, = struct.unpack("<I", _bytes(raw, pe + 24 + 60, 4))
    header_end = pe + 24 + optional_size + section_count * 40
    if not header_end <= header_size <= len(raw):
        raise ValueError("invalid PE declared header extent")
    if any(raw_size and file_at < header_size for _, file_at, raw_size in image.sections):
        raise ValueError("PE section bytes overlap declared headers")
    directory, directory_at = _mapped(image, rva, size)
    tables, extents = [], [(0, header_size), (directory_at, directory_at + size)]
    for at in range(0, size, 28):
        flags, _, _, _, kind, length, address, offset = struct.unpack_from("<IIHHIIII", directory, at)
        if flags or length > MAX_DEBUG_BYTES:
            raise ValueError("unsupported PE debug entry flags or size")
        if not length:
            if offset or address or kind == 1:
                raise ValueError("empty PE debug entry has contradictory metadata")
            continue
        if not offset:
            raise ValueError("PE debug payload lacks a physical file offset")
        contents = _bytes(raw, offset, length)
        if any(begin < offset + length and offset < end for begin, end in extents):
            raise ValueError("overlapping PE debug metadata")
        extents.append((offset, offset + length))
        if address:
            mapped, physical = _mapped(image, address, length)
            if physical != offset or mapped != contents:
                raise ValueError("PE debug file offset disagrees with its mapped RVA")
        elif any(file_at < offset + length and offset < file_at + raw_size
                 for _, file_at, raw_size in image.sections):
            raise ValueError("unmapped debug payload overlaps mapped section bytes")
        if kind == 1:
            tables.append((offset, contents))
    if len(tables) > 1:
        raise ValueError("ambiguous multiple embedded COFF tables")
    if not tables:
        if file_table or file_count:
            raise ValueError("PE COFF symbols lack an embedded COFF debug entry")
        return ()
    payload_at, payload = tables[0]
    count, symbols_at, line_count, lines_at, *_ = struct.unpack("<8I", _bytes(payload, 0, 32))
    if count > MAX_SYMBOLS or line_count > MAX_SYMBOLS:
        raise ValueError("embedded COFF table exceeds resource limits")
    if (file_table or file_count) and (file_table != payload_at + symbols_at or file_count != count):
        raise ValueError("PE header and embedded COFF symbol tables disagree")
    if not count and not symbols_at:
        if line_count or lines_at or len(payload) != 32:
            raise ValueError("empty embedded COFF table has contradictory metadata")
        return ()
    if symbols_at < 32:
        raise ValueError("embedded COFF symbols overlap their header")
    records = _bytes(payload, symbols_at, count * 18)
    strings_at = symbols_at + len(records)
    strings_size, = struct.unpack("<I", _bytes(payload, strings_at, 4))
    if strings_size < 4:
        raise ValueError("invalid embedded COFF string table")
    strings = _bytes(payload, strings_at, strings_size)
    if line_count:
        if lines_at < 32 or (lines_at < strings_at + strings_size and symbols_at < lines_at + line_count * 6):
            raise ValueError("COFF line records overlap other debug metadata")
        lines = _bytes(payload, lines_at, line_count * 6)
    elif lines_at:
        raise ValueError("empty COFF line table has a nonzero pointer")
    else:
        lines = b""
    result, indices, index = [], set(), 0
    while index < count:
        at = index * 18
        name = records[at:at + 8]
        if name[:4] == bytes(4):
            offset, = struct.unpack_from("<I", name, 4)
            if not 4 <= offset < len(strings):
                raise ValueError("invalid embedded COFF symbol string offset")
            end = strings.find(b"\0", offset)
            if end < 0:
                raise ValueError("unterminated embedded COFF symbol name")
            name = strings[offset:end]
        else:
            name, separator, padding = name.partition(b"\0")
            if separator and any(padding):
                raise ValueError("embedded COFF short symbol has invalid padding")
        # Linkers emit ASCII control-prefixed names (including DEL for import
        # terminators). Preserve names losslessly; spelling is not ownership.
        if not name or len(name) > 4096 or any(byte >= 128 for byte in name):
            raise ValueError("unsupported embedded COFF symbol name")
        value, section, kind, storage, auxiliaries = struct.unpack_from("<IhHBB", records, at + 8)
        if index + auxiliaries >= count or not -2 <= section <= len(image.sections):
            raise ValueError("invalid embedded COFF symbol or auxiliary extent")
        if section > 0:
            if auxiliaries:
                raise ValueError("mapped COFF symbol auxiliary metadata is unsupported")
            region = image.sections[section - 1][0]
            if not region.start <= image.base + value < region.end or image.base + value > 0xFFFFFFFF:
                raise ValueError("linked COFF symbol RVA lies outside its declared PE section")
        indices.add(index)
        result.append(Symbol(index, name.decode("ascii"), value, section, kind, storage))
        index += 1 + auxiliaries
    for at in range(0, len(lines), 6):
        value, number = struct.unpack_from("<IH", lines, at)
        if not number and value not in indices:
            raise ValueError("COFF line record references an absent or auxiliary symbol")
        if number and sum(region.start <= image.base + value < region.end for region, _, _ in image.sections) != 1:
            raise ValueError("COFF line RVA lacks a unique image section")
    return tuple(result)
