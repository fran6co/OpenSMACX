"""Image-bound NB10/DBI V60 XFIXUP observations, not relocation semantics.

Format sources: microsoft/microsoft-pdb PDB/dbi/{dbi.h,pdb1.h,dbi.cpp},
langapi/include/pdb.h (DBGTYPE), and pdbdump/pdbdump.cpp (XFIXUP).
No PDB path is opened, and no object ownership or target addend is inferred.
"""

from dataclasses import dataclass
from collections.abc import Mapping
import hashlib
import struct
from types import MappingProxyType

from .model import Span
from .msf import SmallMsf


@dataclass(frozen=True)
class Fixup:
    kind: int
    target_rva: int


@dataclass(frozen=True)
class BoundFixups(Mapping):
    image_sha256: str
    pdb_sha256: str
    _records: Mapping
    fpo_records: tuple[bytes, ...] = ()

    def __post_init__(self):
        object.__setattr__(self, "_records", MappingProxyType(dict(self._records)))
        if any(not isinstance(row, bytes) or len(row) != 16 for row in self.fpo_records):
            raise ValueError("invalid immutable PDB FPO records")
        object.__setattr__(self, "fpo_records", tuple(self.fpo_records))

    def __getitem__(self, site):
        return self._records[site]

    def __iter__(self):
        return iter(self._records)

    def __len__(self):
        return len(self._records)


def _slice(data, offset, size):
    if offset < 0 or size < 0 or offset + size > len(data):
        raise ValueError("truncated PDB binding metadata")
    return data[offset:offset + size]


def _mapped(image, rva, size):
    if not rva or size <= 0 or image.base + rva + size > 0x100000000:
        raise ValueError("invalid PDB binding RVA extent")
    span = Span(image.base + rva, image.base + rva + size)
    value = image.read(span)
    if image.absolute_sites(span):
        raise ValueError("PDB binding metadata has loader fixups")
    matches = [(region, offset) for region, offset, length in image.sections
               if region.start <= span.start and span.end <= region.start + length]
    if len(matches) != 1:
        raise ValueError("PDB binding metadata lacks unique physical backing")
    region, offset = matches[0]
    return value, offset + span.start - region.start


def _nb10(image):
    raw = image.data
    pe, = struct.unpack("<I", _slice(raw, 0x3C, 4))
    count, = struct.unpack("<H", _slice(raw, pe + 6, 2))
    optional, = struct.unpack("<H", _slice(raw, pe + 20, 2))
    header_size, = struct.unpack("<I", _slice(raw, pe + 84, 4))
    section_at = pe + 24 + optional
    sections = _slice(raw, section_at, count * 40)
    if not section_at + len(sections) <= header_size <= len(raw):
        raise ValueError("invalid PE header extent for PDB binding")
    if any(size and offset < header_size for _, offset, size in image.sections):
        raise ValueError("PE sections overlap PDB binding headers")
    rva, size = image.directories[6] if len(image.directories) > 6 else (0, 0)
    if not rva or not size or size % 28 or size > 28 * 4096:
        raise ValueError("missing or invalid PDB debug directory")
    directory, file_at = _mapped(image, rva, size)
    extents, entries = [(0, header_size), (file_at, file_at + size)], []
    for at in range(0, size, 28):
        flags, _, _, _, kind, length, address, offset = struct.unpack_from("<IIHHIIII", directory, at)
        if flags or length > 64 * 1024 * 1024:
            raise ValueError("unsupported PDB debug directory entry")
        if not length:
            if offset or address or kind == 2:
                raise ValueError("contradictory empty PDB debug entry")
            continue
        payload = _slice(raw, offset, length)
        if any(begin < offset + length and offset < end for begin, end in extents):
            raise ValueError("overlapping PDB debug payload")
        extents.append((offset, offset + length))
        if address:
            mapped, physical = _mapped(image, address, length)
            if physical != offset or mapped != payload:
                raise ValueError("PDB debug file pointer and RVA disagree")
        elif any(file_at < offset + length and offset < file_at + raw_size
                 for _, file_at, raw_size in image.sections):
            raise ValueError("PDB overlay overlaps mapped image bytes")
        if kind == 2:
            entries.append(payload)
    if len(entries) != 1:
        raise ValueError("expected exactly one NB10 PDB binding")
    payload = entries[0]
    magic, reserved, signature, age = struct.unpack("<4sIII", _slice(payload, 0, 16))
    path = payload[16:]
    if magic != b"NB10" or reserved or not age or not 2 <= len(path) <= 4096:
        raise ValueError("unsupported NB10 PDB binding")
    if path[-1:] != b"\0" or b"\0" in path[:-1] or any(byte < 32 for byte in path[:-1]):
        raise ValueError("invalid NB10 PDB path extent")
    return signature, age, sections


def bound_fixups(image, pdb_bytes):
    """Return immutable site-RVA -> Fixup after exact image/PDB binding.

    Missing/unsupported evidence raises ValueError, never an empty success.
    A present empty FIXUP stream is allowed. Types 6, 7 and 20 are preserved
    verbatim; callers must establish their operand, addend and rebasing rules.
    The caller separately owns fresh native build and sidecar provenance.
    """
    if not isinstance(image.data, bytes):
        raise ValueError("PDB binding requires immutable image bytes")
    signature, age, sections = _nb10(image)
    msf = SmallMsf(pdb_bytes)

    def stream(index):
        value = msf.stream(index)
        if value is None:
            raise ValueError("required PDB stream is absent")
        return value

    info = stream(1)
    version, info_signature, info_age = struct.unpack("<III", _slice(info, 0, 12))
    if version != 19970604 or (info_signature, info_age) != (signature, age):
        raise ValueError("PDB information and NB10 signature/age disagree")
    dbi = stream(3)
    fields = struct.unpack("<IIIHHHHHHiiiiiiiiHHI", _slice(dbi, 0, 64))
    if fields[:3] != (0xFFFFFFFF, 19970606, age):
        raise ValueError("unsupported DBI version or mismatched age")
    modules, contributions, section_map, files, servers, _, debug_size, ec = fields[9:17]
    flags, machine, reserved = fields[17:]
    lengths = (modules, contributions, section_map, files, servers, ec)
    if any(size < 0 for size in lengths) or not 12 <= debug_size <= 22 or debug_size % 2:
        raise ValueError("invalid DBI substream extents")
    if flags or machine not in (0, 0x14C) or reserved:
        raise ValueError("unsupported DBI flags or machine")
    debug_at = 64 + sum(lengths)
    if debug_at + debug_size != len(dbi):
        raise ValueError("DBI substream lengths disagree with stream extent")
    optional = struct.unpack(f"<{debug_size // 2}H", dbi[debug_at:])
    indices = [index for index in optional if index != 0xFFFF]
    if len(indices) != len(set(indices)) or any(index < 4 for index in indices):
        raise ValueError("aliased or reserved DBI optional stream indices")
    for index in indices:
        stream(index)
    symbol_indices = [index for index in (fields[3], fields[5], fields[7]) if index != 0xFFFF]
    if len(symbol_indices) != len(set(symbol_indices)):
        raise ValueError("aliased DBI symbol stream indices")
    for index in symbol_indices:
        if index != 0xFFFF:
            if index < 4 or index in indices:
                raise ValueError("aliased DBI symbol stream index")
            stream(index)
    if optional[2] == 0xFFFF or optional[5] == 0xFFFF:
        raise ValueError("missing PDB fixup or section-header stream")
    if any(optional[index] != 0xFFFF for index in (3, 4)) or (len(optional) > 10 and optional[10] != 0xFFFF):
        raise ValueError("PDB address remapping is unsupported")
    if stream(optional[5]) != sections:
        raise ValueError("PDB and image section headers disagree")
    fixups = stream(optional[2])
    if len(fixups) % 12 or len(fixups) > 12 * 1_000_000:
        raise ValueError("invalid PDB XFIXUP stream extent")
    result = {}
    for kind, extra, site, target in struct.iter_unpack("<HHII", fixups):
        if kind not in (6, 7, 20) or extra:
            raise ValueError("unsupported PDB XFIXUP type or extra flags")
        if not site or image.base + site + 4 > 0x100000000:
            raise ValueError("invalid PDB XFIXUP site RVA")
        image.read(Span(image.base + site, image.base + site + 4))
        if not target or image.base + target > 0xFFFFFFFF or sum(
                region.start <= image.base + target < region.end for region, _, _ in image.sections) != 1:
            raise ValueError("PDB XFIXUP target lacks unique image section")
        if site in result:
            raise ValueError("duplicate PDB XFIXUP site")
        result[site] = Fixup(kind, target)
    ordered = sorted(result)
    if any(right < left + 4 for left, right in zip(ordered, ordered[1:])):
        raise ValueError("overlapping PDB XFIXUP sites")
    fpo = stream(optional[0]) if optional[0] != 0xFFFF else b""
    if len(fpo) % 16 or len(fpo) > 16 * 1_000_000:
        raise ValueError("invalid PDB FPO stream extent")
    # Multiplicity is evidence: preserve even identical rows for the consumer
    # to refuse ambiguous root metadata, never deduplicate it here.
    records = tuple(fpo[at:at + 16] for at in range(0, len(fpo), 16))
    return BoundFixups(hashlib.sha256(image.data).hexdigest(), hashlib.sha256(pdb_bytes).hexdigest(), result, records)
