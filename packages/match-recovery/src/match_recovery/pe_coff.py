"""Bounds-checked i386 PE/COFF inputs for binary verification.

Format reference: https://learn.microsoft.com/en-us/windows/win32/debug/pe-format
Unsupported relocation/section forms are refused, never silently masked.
"""

from bisect import bisect_left
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


def weak_external(obj, symbol: Symbol) -> tuple[Symbol, int]:
    """Decode a COFF fallback edge, without selecting its link-time definition.

    Microsoft PE/COFF, Auxiliary Format 3: Weak Externals. Search modes 1/2/3
    mean no-library/library/alias; this metadata is not a source-image binding.
    """
    raw = getattr(obj, "auxiliary", {}).get(symbol.index, b"")
    if symbol.storage != 105 or symbol.section != 0 or symbol.value != 0 or len(raw) != 18 or any(raw[8:]):
        raise ValueError("unsupported COFF weak-external metadata")
    target, mode = _unpack("<II", raw, 0)
    if target not in obj.symbols or mode not in (1, 2, 3):
        raise ValueError("invalid COFF weak-external target or search mode")
    return obj.symbols[target], mode


@dataclass(frozen=True)
class Section:
    data: bytes
    relocations: tuple[tuple[int, int, int], ...]
    flags: int
    name: str = ""

    @property
    def alignment(self) -> int:
        encoded = (self.flags >> 20) & 15
        if not 1 <= encoded <= 14:
            raise ValueError("missing or unsupported COFF section alignment")
        return 1 << (encoded - 1)


class CoffRecords:
    """Regular COFF records for inventory, without a code-ownership verdict.

    Import-descriptor objects can carry section-named symbols without the
    COMDAT auxiliary metadata required by the function verifier. Keep that
    distinction explicit: this reader exposes no function/subject API.
    """
    def __init__(self, data: bytes):
        data = bytes(data)
        machine, count, _, sym_offset, sym_count, optional, _ = _unpack("<HHIIIHH", data, 0)
        if machine not in (0, 0x14C):
            raise ValueError("expected i386 or unspecified-machine COFF records")
        # Retain immutable physical bytes for bounded format-specific metadata checks.
        self.raw_data = data
        self.machine = machine
        self.optional_header_size = optional
        if optional:
            header = _slice(data, 20, optional)
            if machine != 0x14C or optional < 96 or _unpack("<H", header, 0)[0] != 0x10B:
                raise ValueError("unsupported COFF record optional header")
            if _unpack("<I", header, 92)[0] > (optional - 96) // 8:
                raise ValueError("truncated COFF optional-header directory array")
        section_table = 20 + optional
        _slice(data, section_table, count * 40)
        _slice(data, sym_offset, sym_count * 18)
        strings_at = sym_offset + sym_count * 18
        strings_size, = _unpack("<I", data, strings_at)
        if strings_size < 4:
            raise ValueError("invalid COFF string table")
        strings = _slice(data, strings_at, strings_size)
        symbols, auxiliary, index = {}, {}, 0
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
            if aux:
                auxiliary[index] = _slice(data, at + 18, aux * 18)
            index += aux + 1
        sections = []
        for index in range(count):
            at = section_table + index * 40
            size, offset, reloc_offset = _unpack("<III", data, at + 16)
            reloc_count, = _unpack("<H", data, at + 32)
            flags, = _unpack("<I", data, at + 36)
            if flags & 0x01000000:
                raise ValueError("extended COFF relocation tables are not supported")
            relocations = tuple(_unpack("<IIH", data, reloc_offset + j * 10) for j in range(reloc_count))
            if any(symbol not in symbols for _, symbol, _ in relocations):
                raise ValueError("COFF relocation references an absent or auxiliary symbol")
            raw = _slice(data, offset, size) if offset else b""
            name = _slice(data, at, 8).rstrip(b"\0").decode("ascii")
            sections.append(Section(raw, relocations, flags, name))
        self.symbols, self.sections = symbols, tuple(sections)
        self.auxiliary = auxiliary
        self.section_sizes = tuple(_unpack("<I", data, section_table + index * 40 + 16)[0] for index in range(count))


class Coff(CoffRecords):
    def __init__(self, data: bytes):
        machine, _, _, _, _, optional, _ = _unpack("<HHIIIHH", data, 0)
        if machine != 0x14C or optional:
            raise ValueError("expected an i386 COFF object without an optional header")
        super().__init__(data)
        symbols, sections, auxiliary = self.symbols, self.sections, self.auxiliary
        count = len(sections)
        # Section ownership is explicit COFF metadata, not section order, a
        # compiler-local label spelling or a match against original operands.
        definitions, associations = {}, {}
        for index, symbol in symbols.items():
            if symbol.section <= 0 or symbol.storage != 3 or symbol.type != 0:
                continue
            section = sections[symbol.section - 1]
            if symbol.name != section.name:
                continue
            raw = auxiliary.get(index, b"")
            if symbol.value != 0 or len(raw) != 18 or symbol.section in definitions:
                raise ValueError("invalid or duplicate COFF section definition")
            length, reloc_count, _, _, parent, selection = _unpack("<IHHIHB", raw, 0)
            header_at = 20 + (symbol.section - 1) * 40
            declared_size, = _unpack("<I", data, header_at + 16)
            if length != declared_size or reloc_count != len(section.relocations) or any(raw[15:]):
                raise ValueError("inconsistent COFF section definition")
            if selection not in range(7) or bool(selection) != bool(section.flags & 0x1000):
                raise ValueError("unsupported COFF COMDAT selection")
            definitions[symbol.section] = selection
            if selection == 5:
                if not 1 <= parent <= count or not sections[parent - 1].flags & 0x1000:
                    raise ValueError("invalid associative COMDAT parent")
                associations[symbol.section] = parent
        for child in associations:
            seen, current = set(), child
            while current in associations:
                if current in seen:
                    raise ValueError("cyclic associative COMDAT ownership")
                seen.add(current)
                current = associations[current]
            if current not in definitions:
                raise ValueError("associative COMDAT root lacks a section definition")
        self.associations = associations

    def code_sections(self, symbol: Symbol) -> tuple[int, ...]:
        """Entry plus explicitly associated code; never guess among siblings."""
        children = []
        for child in self.associations:
            current = child
            while current in self.associations:
                current = self.associations[current]
                if current == symbol.section:
                    if self.sections[child - 1].flags & 0x20:
                        children.append(child)
                    break
        return (symbol.section, *sorted(children))

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
        return self.section_subject(symbol.section)

    def section_subject(self, number: int) -> tuple[bytes, tuple[Relocation, ...], int]:
        if not 1 <= number <= len(self.sections):
            raise ValueError("invalid COFF subject section")
        section = self.sections[number - 1]
        rows = []
        for offset, target, kind in section.relocations:
            if kind == 0:  # IMAGE_REL_I386_ABSOLUTE is explicitly a no-op.
                continue
            if kind not in (6, 20):
                raise ValueError(f"unsupported i386 relocation kind {kind}")
            rows.append(Relocation(offset, RelocationKind.ABSOLUTE32 if kind == 6
                                   else RelocationKind.RELATIVE32, str(target)))
        return section.data, tuple(rows), section.alignment


@dataclass(frozen=True)
class PeImport:
    module: str
    name: str | None
    ordinal: int | None
    address: int  # Address of the loader-owned slot, never its loaded pointer.


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
        entry_rva, = _unpack("<I", header, 16)
        self.entrypoint = self.base + entry_rva if entry_rva else None
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
        self.section_names = tuple(_slice(data, pe + 24 + optional + index * 40, 8).rstrip(b"\0").decode("latin1")
                                   for index in range(count))
        self.section_flags = tuple(_unpack("<I", data, pe + 24 + optional + index * 40 + 36)[0] for index in range(count))
        directories, = _unpack("<I", header, 92)
        if directories > (len(header) - 96) // 8:
            raise ValueError("truncated PE data directory array")
        self.directories = tuple(_unpack("<II", header, 96 + index * 8) for index in range(directories))
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
        self._relocation_sites = tuple(sorted(self.relocations))

    def read(self, span: Span) -> bytes:
        matches = [(region, offset, size) for region, offset, size in self.sections
                   if region.start <= span.start and span.end <= region.start + size]
        if len(matches) != 1:
            raise ValueError("original span is not uniquely backed by image file bytes")
        region, offset, _ = matches[0]
        return _slice(self.data, offset + span.start - region.start, span.end - span.start)

    def absolute_sites(self, span: Span) -> frozenset[int]:
        # Include sites beginning before the span when their four-byte operand
        # overlaps it. Table diagnostics query individual words, so rescanning
        # the entire image's relocation set for every slot is needlessly costly.
        left = bisect_left(self._relocation_sites, span.start - 3)
        right = bisect_left(self._relocation_sites, span.end)
        sites = self._relocation_sites[left:right]
        if any(not span.start <= site <= span.end - 4 for site in sites):
            raise ValueError("original span cuts through a relocation")
        return frozenset(site - span.start for site in sites)

    def read_initial(self, span: Span) -> bytes:
        """Image storage before CRT initialization, including virtual zero fill.

        Relocations are still encoded for the preferred base; callers must
        independently verify their targets and forms. This is not runtime state.
        """
        matches = [(region, offset, size, flags) for (region, offset, size), flags in zip(self.sections, self.section_flags)
                   if region.start <= span.start and span.end <= region.end]
        if len(matches) != 1:
            raise ValueError("initial data span is not inside one image section")
        region, offset, size, flags = matches[0]
        if flags & 0x20 or not flags & (0x40 | 0x80):
            raise ValueError("original initial data requires a non-code data section")
        begin = span.start - region.start
        backed = max(0, min(span.end - span.start, size - begin))
        raw = _slice(self.data, offset + begin, backed) if backed else b""
        return raw + bytes(span.end - span.start - backed)

    def _import_bytes(self, rva: int, size: int) -> bytes:
        if not rva or self.base + rva + size > 0x100000000:
            raise ValueError("invalid PE import RVA")
        return self.read(Span(self.base + rva, self.base + rva + size))

    def _import_string(self, rva: int) -> str:
        # Strings must end within their raw-backed section, not somewhere in
        # the rest of the file. A resource limit refuses, never truncates names.
        start = self.base + rva
        matches = [(region, offset, size) for region, offset, size in self.sections
                   if region.start <= start < region.start + size]
        if not rva or len(matches) != 1 or start > 0xFFFFFFFF:
            raise ValueError("unbacked PE import name")
        region, offset, size = matches[0]
        at = offset + start - region.start
        end = self.data.find(b"\0", at, min(offset + size, at + 4096))
        if end <= at:
            raise ValueError("empty or unterminated PE import name")
        try:
            return self.data[at:end].decode("ascii")
        except UnicodeDecodeError as error:
            raise ValueError("non-ASCII PE import name") from error

    def imports(self) -> tuple[PeImport, ...]:
        """Read ordinary PE32 imports independently of code operands/symbols.

        Bound IAT values are not identities. Without the original lookup table
        a bound descriptor cannot supply names. Delay imports are not decoded.
        Malformed or unsupported input raises rather than returning a prefix.
        """
        rva, size = self.directories[1] if len(self.directories) > 1 else (0, 0)
        if not rva and not size:
            return ()
        table = self._import_bytes(rva, size)
        imports, slots = [], set()
        for at in range(0, min(len(table), 4096 * 20), 20):
            lookup, timestamp, chain, name, iat = _unpack("<IIIII", table, at)
            if not any((lookup, timestamp, chain, name, iat)):
                return tuple(imports)
            module = self._import_string(name)
            if not iat:
                raise ValueError("missing PE import address table")
            if not lookup and timestamp:
                raise ValueError("bound PE import has no original lookup table")
            for index in range(65536):
                entry, = struct.unpack("<I", self._import_bytes((lookup or iat) + index * 4, 4))
                stored, = struct.unpack("<I", self._import_bytes(iat + index * 4, 4))
                address = self.base + iat + index * 4
                if any(address + delta in slots for delta in range(-3, 4)):
                    raise ValueError("overlapping PE import address tables")
                slots.add(address)  # Include each terminator in overlap checks.
                if not entry:
                    if stored:
                        raise ValueError("PE import tables have different termination")
                    break
                if not timestamp and entry != stored:
                    raise ValueError("unbound PE import tables disagree")
                if len(imports) >= 65536:
                    raise ValueError("PE import count exceeds verification limit")
                if entry & 0x80000000:
                    if entry & 0x7FFF0000:
                        raise ValueError("reserved bits in PE ordinal import")
                    imports.append(PeImport(module, None, entry & 0xFFFF, address))
                else:
                    self._import_bytes(entry, 2)  # Hint exists, but is not an ordinal.
                    imports.append(PeImport(module, self._import_string(entry + 2), None, address))
            else:
                raise ValueError("unterminated PE import lookup table")
        raise ValueError("unterminated or oversized PE import directory")


def unaddressed_end_label(obj, symbol: Symbol) -> bool:
    """Recognize inert label metadata exactly at a bounded code section's end.

    This never extends the section or supplies a relocation target. All object
    sections participate in the incoming-reference check, including no-op and
    debug relocations. Unsupported weak metadata cannot prove non-reference.
    Callers must still retain the label's offset, type, storage and identity.
    """
    if (not isinstance(obj, Coff) or not isinstance(symbol, Symbol)
            or obj.symbols.get(symbol.index) != symbol or symbol.storage != 6
            or symbol.type != 0 or obj.auxiliary.get(symbol.index)
            or not 1 <= symbol.section <= len(obj.sections)):
        return False
    section = obj.sections[symbol.section - 1]
    if (not section.data or len(section.data) != obj.section_sizes[symbol.section - 1]
            or symbol.value != len(section.data) or not section.flags & 0x20
            or not section.flags & 0x20000000 or section.flags & 0x80):
        return False
    if any(target == symbol.index for section in obj.sections
           for _, target, _ in section.relocations):
        return False
    for candidate in obj.symbols.values():
        if candidate.storage == 105:
            try:
                target, _ = weak_external(obj, candidate)
            except ValueError:
                return False
            if target.index == symbol.index:
                return False
    return True
