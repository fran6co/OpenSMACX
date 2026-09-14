"""Complete bounded PE32 export-address inventory, not runtime resolution.

Format: https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#export-address-table
All EAT slots are retained, including ordinal-only exports and holes. Forwarders
remain strings, never guessed local addresses. The caller owns image freshness;
this parser neither authorizes recovery nor proves exported behavior.
"""

from dataclasses import dataclass
import struct

from .model import Span


@dataclass(frozen=True)
class PeExport:
    index: int
    ordinal: int
    names: tuple[str, ...]
    rva: int
    address: int | None
    forwarder: str | None


@dataclass(frozen=True)
class ExportRange:
    rva: int
    size: int
    kind: str


@dataclass(frozen=True)
class ExportMetadata:
    entries: tuple[PeExport, ...]
    ranges: tuple[ExportRange, ...]
    timestamp_rva: int | None
    timestamp: int | None


def export_metadata(image) -> ExportMetadata:
    """Read immutable entries and every metadata use, or refuse completely.

    Empty entries mean an absent directory or a valid zero-entry table,
    never missing name coverage. Direct targets must belong to one PE section;
    unplaced absolute exports and header targets remain unsupported. Metadata
    must be raw-backed and must not be modified by loader base relocations.
    Limits refuse the whole inventory rather than returning a partial result.
    Ranges retain overlapping uses, including cached strings used in different
    roles. Consumers must not turn the broad directory span into permission to
    project bytes that also belong to a table or string.
    """
    rva, size = image.directories[0] if image.directories else (0, 0)
    if (rva, size) == (0, 0):
        return ExportMetadata((), (), None, None)
    if not rva or size < 40 or rva + size > 0x100000000:
        raise ValueError("invalid PE export directory extent")

    ranges = []

    def read(at, length, kind=None):
        if not at or length <= 0 or image.base + at + length > 0x100000000:
            raise ValueError("invalid PE export metadata RVA or extent")
        span = Span(image.base + at, image.base + at + length)
        raw = image.read(span)
        if image.absolute_sites(span):
            raise ValueError("PE export metadata has loader base relocations")
        if kind is not None:
            ranges.append(ExportRange(at, length, kind))
        return raw

    directory = read(rva, size, "export_directory")
    ranges.append(ExportRange(rva, 40, "export_header"))
    flags, timestamp, _, _, module, base, count, name_count, eat, name_table, ordinal_table = struct.unpack_from(
        "<IIHHIIIIIII", directory)
    if flags or count > 65536 or name_count > 65536 or base + count > 0x100000000:
        raise ValueError("unsupported PE export flags, counts or ordinal extent")
    strings, string_bytes = {}, 0

    def string(at, *, forwarder=False, kind="export_name"):
        nonlocal string_bytes
        key = (at, forwarder)
        if key in strings:
            text = strings[key]
            ranges.append(ExportRange(at, len(text) + 1, kind))
            return text
        start = image.base + at
        matches = [(region, offset, raw_size) for region, offset, raw_size in image.sections
                   if region.start <= start < region.start + raw_size]
        if not at or start > 0xFFFFFFFF or len(matches) != 1:
            raise ValueError("unbacked PE export string")
        region, file_offset, raw_size = matches[0]
        length = min(region.start + raw_size - start, 4096)
        if forwarder:
            if not rva <= at < rva + size:
                raise ValueError("PE forwarder is outside export directory")
            length = min(length, rva + size - at)
        file_at = file_offset + start - region.start
        end = image.data.find(b"\0", file_at, file_at + length)
        if end <= file_at:
            raise ValueError("empty or unterminated PE export string")
        raw = read(at, end - file_at + 1, kind)[:-1]
        if any(value < 32 or value >= 127 for value in raw):
            raise ValueError("unsupported non-ASCII or control character in PE export string")
        string_bytes += len(raw) + 1
        if string_bytes > 8 * 1024 * 1024:
            raise ValueError("PE export string inventory exceeds resource limit")
        text = raw.decode("ascii")
        strings[key] = text
        return text

    string(module, kind="export_module_name")
    if not count:
        if name_count or eat or name_table or ordinal_table:
            raise ValueError("empty PE export table has contradictory arrays")
        return ExportMetadata((), tuple(ranges), rva + 4, timestamp)
    values = struct.unpack("<" + "I" * count, read(eat, count * 4, "export_address_table"))
    names = [[] for _ in values]
    if name_count:
        pointers = struct.unpack("<" + "I" * name_count, read(name_table, name_count * 4, "export_name_pointers"))
        ordinals = struct.unpack("<" + "H" * name_count, read(ordinal_table, name_count * 2, "export_ordinals"))
        previous = None
        for pointer, index in zip(pointers, ordinals, strict=True):
            if index >= count:
                raise ValueError("PE export name ordinal is outside address table")
            name = string(pointer)
            if previous is not None and name <= previous:
                raise ValueError("PE export names must be unique and lexically ordered")
            previous = name
            if not values[index]:
                raise ValueError("PE export name points to an empty address slot")
            names[index].append(name)
    elif name_table or ordinal_table:
        raise ValueError("ordinal-only PE exports have contradictory name arrays")

    result = []
    for index, target in enumerate(values):
        address, forwarder = None, None
        if rva <= target < rva + size:
            forwarder = string(target, forwarder=True, kind="export_forwarder")
            module_name, dot, target_name = forwarder.partition(".")
            if not dot or not module_name or not target_name:
                raise ValueError("unsupported PE export forwarder syntax")
            if target_name.startswith("#") and (not target_name[1:].isascii()
                    or not target_name[1:].isdigit() or not 0 <= int(target_name[1:]) <= 65535):
                raise ValueError("invalid PE export forwarder ordinal")
        elif target:
            address = image.base + target
            if (address > 0xFFFFFFFF or sum(region.start <= address < region.end
                                          for region, _, _ in image.sections) != 1):
                raise ValueError("PE export target is outside actual image sections")
        result.append(PeExport(index, base + index, tuple(names[index]), target, address, forwarder))
    return ExportMetadata(tuple(result), tuple(ranges), rva + 4, timestamp)


def exports(image) -> tuple[PeExport, ...]:
    """Return complete export entries; metadata parsing and bounds have one owner."""
    return export_metadata(image).entries
