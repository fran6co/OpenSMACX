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


def exports(image) -> tuple[PeExport, ...]:
    """Read every export slot or refuse malformed/unsupported evidence.

    Empty means an explicitly absent directory or a valid zero-entry table,
    never missing name coverage. Direct targets must belong to one PE section;
    unplaced absolute exports and header targets remain unsupported. Metadata
    must be raw-backed and must not be modified by loader base relocations.
    Limits refuse the whole inventory rather than returning a partial result.
    """
    rva, size = image.directories[0] if image.directories else (0, 0)
    if (rva, size) == (0, 0):
        return ()
    if not rva or size < 40 or rva + size > 0x100000000:
        raise ValueError("invalid PE export directory extent")

    def read(at, length):
        if not at or length <= 0 or image.base + at + length > 0x100000000:
            raise ValueError("invalid PE export metadata RVA or extent")
        span = Span(image.base + at, image.base + at + length)
        raw = image.read(span)
        if image.absolute_sites(span):
            raise ValueError("PE export metadata has loader base relocations")
        return raw

    directory = read(rva, size)
    flags, _, _, _, module, base, count, name_count, eat, name_table, ordinal_table = struct.unpack_from(
        "<IIHHIIIIIII", directory)
    if flags or count > 65536 or name_count > 65536 or base + count > 0x100000000:
        raise ValueError("unsupported PE export flags, counts or ordinal extent")
    strings, string_bytes = {}, 0

    def string(at, *, forwarder=False):
        nonlocal string_bytes
        key = (at, forwarder)
        if key in strings:
            return strings[key]
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
        raw = read(at, end - file_at + 1)[:-1]
        if any(value < 32 or value >= 127 for value in raw):
            raise ValueError("unsupported non-ASCII or control character in PE export string")
        string_bytes += len(raw) + 1
        if string_bytes > 8 * 1024 * 1024:
            raise ValueError("PE export string inventory exceeds resource limit")
        text = raw.decode("ascii")
        strings[key] = text
        return text

    string(module)
    if not count:
        if name_count or eat or name_table or ordinal_table:
            raise ValueError("empty PE export table has contradictory arrays")
        return ()
    values = struct.unpack("<" + "I" * count, read(eat, count * 4))
    names = [[] for _ in values]
    if name_count:
        pointers = struct.unpack("<" + "I" * name_count, read(name_table, name_count * 4))
        ordinals = struct.unpack("<" + "H" * name_count, read(ordinal_table, name_count * 2))
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
            forwarder = string(target, forwarder=True)
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
    return tuple(result)
