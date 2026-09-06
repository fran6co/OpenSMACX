"""Bounds-checked i386 PE/COFF inputs for the VC6 target adapter.

Format reference: https://learn.microsoft.com/en-us/windows/win32/debug/pe-format
Unsupported relocation/section forms are refused, never silently masked.
"""

from dataclasses import dataclass
import hashlib
import struct

from .binary import Relocation, RelocationKind
from .model import Span, require_digest


def _slice(data: bytes, offset: int, size: int) -> bytes:
    if offset < 0 or size < 0 or offset + size > len(data):
        raise ValueError("truncated PE/COFF input")
    return data[offset:offset + size]


def _unpack(fmt: str, data: bytes, offset: int):
    return struct.unpack(fmt, _slice(data, offset, struct.calcsize(fmt)))


@dataclass(frozen=True)
class Symbol:
    index: int
    name: str
    value: int
    section: int
    type: int
    storage: int


@dataclass(frozen=True)
class Section:
    data: bytes
    relocations: tuple[tuple[int, int, int], ...]
    flags: int

    @property
    def alignment(self) -> int:
        encoded = (self.flags >> 20) & 15
        if not 1 <= encoded <= 14:
            raise ValueError("missing or unsupported COFF section alignment")
        return 1 << (encoded - 1)


class Coff:
    def __init__(self, data: bytes):
        machine, count, _, sym_offset, sym_count, optional, _ = _unpack("<HHIIIHH", data, 0)
        if machine != 0x14C or optional:
            raise ValueError("expected an i386 COFF object without an optional header")
        _slice(data, 20, count * 40)
        _slice(data, sym_offset, sym_count * 18)
        strings_at = sym_offset + sym_count * 18
        strings_size, = _unpack("<I", data, strings_at)
        if strings_size < 4:
            raise ValueError("invalid COFF string table")
        strings = _slice(data, strings_at, strings_size)
        symbols, index = {}, 0
        while index < sym_count:
            at = sym_offset + index * 18
            raw_name = _slice(data, at, 8)
            if raw_name[:4] == bytes(4):
                offset = int.from_bytes(raw_name[4:], "little")
                if not 4 <= offset < len(strings):
                    raise ValueError("invalid COFF symbol string offset")
                end = strings.find(b"\0", offset)
                if end < 0:
                    raise ValueError("unterminated COFF symbol")
                name = strings[offset:end].decode("ascii")
            else:
                name = raw_name.rstrip(b"\0").decode("ascii")
            value, section, kind, storage, aux = _unpack("<IhHBB", data, at + 8)
            if index + aux >= sym_count or section > count:
                raise ValueError("invalid COFF symbol/auxiliary extent")
            symbols[index] = Symbol(index, name, value, section, kind, storage)
            index += aux + 1
        sections = []
        for index in range(count):
            at = 20 + index * 40
            size, offset, reloc_offset = _unpack("<III", data, at + 16)
            reloc_count, = _unpack("<H", data, at + 32)
            flags, = _unpack("<I", data, at + 36)
            if flags & 0x01000000:
                raise ValueError("extended COFF relocation tables are not supported")
            relocations = tuple(_unpack("<IIH", data, reloc_offset + j * 10) for j in range(reloc_count))
            if any(symbol not in symbols for _, symbol, _ in relocations):
                raise ValueError("COFF relocation references an absent or auxiliary symbol")
            raw = _slice(data, offset, size) if offset else b""
            sections.append(Section(raw, relocations, flags))
        self.symbols, self.sections = symbols, tuple(sections)

    def function(self, name: str) -> Symbol:
        matches = [symbol for symbol in self.symbols.values() if symbol.name == name
                   and symbol.section > 0 and symbol.type & 0x20 and symbol.storage in (2, 3)]
        if len(matches) != 1:
            raise ValueError(f"expected one defined COFF function named {name!r}")
        symbol = matches[0]
        section = self.sections[symbol.section - 1]
        if not section.flags & 0x20 or not section.flags & 0x1000:
            raise ValueError("function must belong to a code COMDAT")
        peers = [item for item in self.symbols.values() if item.section == symbol.section
                 and item.type & 0x20 and item.storage in (2, 3)]
        if len(peers) != 1 or symbol.value != 0:
            raise ValueError("function does not exclusively own its COMDAT")
        return symbol

    def subject(self, symbol: Symbol) -> tuple[bytes, tuple[Relocation, ...], int]:
        section = self.sections[symbol.section - 1]
        rows = []
        for offset, target, kind in section.relocations:
            if kind == 0:  # IMAGE_REL_I386_ABSOLUTE is explicitly a no-op.
                continue
            if kind not in (6, 20):
                raise ValueError(f"unsupported i386 relocation kind {kind}")
            rows.append(Relocation(offset, RelocationKind.ABSOLUTE32 if kind == 6
                                   else RelocationKind.RELATIVE32, str(target)))
        return section.data, tuple(rows), section.alignment


class PeImage:
    def __init__(self, data: bytes, pin: str):
        require_digest(pin)
        if hashlib.sha256(data).hexdigest() != pin:
            raise ValueError("original image digest mismatch")
        if _slice(data, 0, 2) != b"MZ":
            raise ValueError("expected a DOS/PE image")
        pe, = _unpack("<I", data, 0x3C)
        if _slice(data, pe, 4) != b"PE\0\0":
            raise ValueError("invalid PE signature")
        machine, count, _, _, _, optional, _ = _unpack("<HHIIIHH", data, pe + 4)
        if machine != 0x14C or optional < 96:
            raise ValueError("expected an i386 PE32 image")
        header = _slice(data, pe + 24, optional)
        if _unpack("<H", header, 0)[0] != 0x10B:
            raise ValueError("expected a PE32 optional header")
        self.base, = _unpack("<I", header, 28)
        sections = []
        for index in range(count):
            at = pe + 24 + optional + index * 40
            virtual_size, rva, size, offset = _unpack("<IIII", data, at + 8)
            _slice(data, offset, size)
            sections.append((Span(self.base + rva, self.base + rva + max(virtual_size, size)), offset, size))
        ordered = sorted(span for span, _, _ in sections)
        if any(left.end > right.start for left, right in zip(ordered, ordered[1:])):
            raise ValueError("overlapping PE sections")
        self.data, self.sections = data, tuple(sections)
        directories, = _unpack("<I", header, 92)
        self.relocations = frozenset()
        if directories > 5:
            rva, size = _unpack("<II", header, 96 + 5 * 8)
            if size:
                table = self.read(Span(self.base + rva, self.base + rva + size))
                sites, at = set(), 0
                while at < len(table):
                    page, block_size = _unpack("<II", table, at)
                    if block_size < 8 or block_size % 2:
                        raise ValueError("invalid PE base relocation block")
                    entries = _slice(table, at + 8, block_size - 8)
                    for (entry,) in struct.iter_unpack("<H", entries):
                        kind, offset = entry >> 12, entry & 0xFFF
                        if kind == 0:
                            continue
                        if kind != 3:
                            raise ValueError(f"unsupported PE base relocation kind {kind}")
                        site = self.base + page + offset
                        if site in sites:
                            raise ValueError("duplicate PE base relocation")
                        sites.add(site)
                    at += block_size
                self.relocations = frozenset(sites)

    def read(self, span: Span) -> bytes:
        matches = [(region, offset, size) for region, offset, size in self.sections
                   if region.start <= span.start and span.end <= region.start + size]
        if len(matches) != 1:
            raise ValueError("original span is not uniquely backed by image file bytes")
        region, offset, _ = matches[0]
        return _slice(self.data, offset + span.start - region.start, span.end - span.start)

    def absolute_sites(self, span: Span) -> frozenset[int]:
        sites = {site for site in self.relocations if site < span.end and site + 4 > span.start}
        if any(not span.start <= site <= span.end - 4 for site in sites):
            raise ValueError("original span cuts through a relocation")
        return frozenset(site - span.start for site in sites)
