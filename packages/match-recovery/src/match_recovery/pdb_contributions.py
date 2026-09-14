"""Image-bound V60 PDB module/contribution records, not storage permissions.

Format authorities: Microsoft's PDB/dbi/dbi.h (MODI_60_Persist, DBISCImpvV60),
PDB/include/dbicommon.h (SC), and PDB/dbi/dbi.cpp (persisted-module loading).
https://github.com/microsoft/microsoft-pdb/blob/master/PDB/dbi/dbi.h
https://github.com/microsoft/microsoft-pdb/blob/master/PDB/include/dbicommon.h

V60 SC has no original COFF section index. CRCs, module labels, first-module
contributions and absent rows never establish physical selection or discard.
"""

from dataclasses import asdict, dataclass
from hashlib import sha256
import struct

from .model import Result
from .msf import SmallMsf
from .pdb_fixups import _slice, bound_fixups


@dataclass(frozen=True)
class Contribution:
    section: int
    offset: int
    size: int
    flags: int
    module: int
    data_crc: int
    relocation_crc: int


@dataclass(frozen=True)
class Module:
    name: str
    object_name: str
    first: Contribution | None
    stream: int | None
    symbol_bytes: int
    line_bytes: int
    c13_bytes: int


@dataclass(frozen=True)
class BoundContributions:
    image_sha256: str
    pdb_sha256: str
    modules: tuple[Module, ...]
    contributions: tuple[Contribution, ...]

    def report(self):
        return {"check": "pdb_contribution_inventory", "enumeration": Result.PASS,
                "result": Result.UNKNOWN, "image_sha256": self.image_sha256, "pdb_sha256": self.pdb_sha256,
                "modules": [asdict(row) for row in self.modules],
                "contributions": [asdict(row) for row in self.contributions],
                "retained_storage_coverage": Result.UNKNOWN, "physical_object_binding": Result.UNKNOWN,
                "allocation_isolation": Result.UNKNOWN, "persisted": False, "ready_for_collection": False,
                "limitations": ["bound contribution intervals, not original COFF section identities or contents",
                                "duplicate/overlapping records and module names retain their multiplicity",
                                "module first contribution is not its complete contribution inventory",
                                "missing contributions do not establish discard or absence of storage"]}


def _contribution(raw, image, module_count, *, nil=False):
    # Padding is compiler layout, not a reserved semantic field or an address.
    section, offset, size, flags, module, data_crc, reloc_crc = struct.unpack("<H2xiiIH2xII", raw)
    if section == 0xFFFF and nil:
        # Microsoft's reader can invalidate only isect; the other old fields
        # are not a live region. Preserve no fabricated zero-length placement.
        return None
    if (not 1 <= section <= len(image.sections) or not 0 <= module < module_count
            or offset < 0 or size <= 0):
        raise ValueError("invalid PDB contribution section, module or signed extent")
    region = image.sections[section - 1][0]
    if region.start + offset + size > min(region.end, 0x100000000):
        raise ValueError("PDB contribution exceeds its bound PE section")
    return Contribution(section, offset, size, flags, module, data_crc, reloc_crc)


def bound_contributions(image, pdb_bytes, *, max_modules=65535, max_contributions=1000000):
    """Parse this exact NB10/SmallMSF/V60 pair with existing strict PDB binding.

    The supported boundary requires the same fixup/section-header sidecar as
    native linked verification. No module path is opened. A present empty SC
    table differs from a missing table; neither proves complete linked storage.
    Returned immutable observations are not live acceptance capabilities.
    """
    if (type(max_modules) is not int or not 1 <= max_modules <= 65535
            or type(max_contributions) is not int or not 1 <= max_contributions <= 1000000):
        raise ValueError("PDB contribution limits must be exact bounded positive integers")
    # Reuse NB10 signature/age, DBI extent/stream alias/remapping and exact PE
    # section-header validation. Do not create a weaker parallel PDB binding.
    bound_fixups(image, pdb_bytes)
    msf = SmallMsf(pdb_bytes)
    dbi = msf.stream(3)
    fields = struct.unpack("<IIIHHHHHHiiiiiiiiHHI", _slice(dbi, 0, 64))
    module_bytes, contribution_bytes = fields[9:11]
    if module_bytes % 4 or contribution_bytes < 4 or (contribution_bytes - 4) % 28:
        raise ValueError("missing or malformed V60 PDB contribution substream")
    if (contribution_bytes - 4) // 28 > max_contributions:
        raise ValueError("PDB contribution table exceeds its record resource bound")
    begin = 64 + module_bytes
    version, = struct.unpack("<I", _slice(dbi, begin, 4))
    if version != 0xF12EBA2D:
        raise ValueError("unsupported PDB contribution version; expected V60 SC")
    module_data = _slice(dbi, 64, module_bytes)
    debug_at = 64 + sum(fields[index] for index in (9, 10, 11, 12, 13, 16))
    optional = struct.unpack(f"<{fields[15] // 2}H", dbi[debug_at:])
    reserved_streams = {0, 1, 2, 3, *(index for index in optional if index != 0xFFFF),
                        *(fields[index] for index in (3, 5, 7) if fields[index] != 0xFFFF)}
    used_streams, modules, firsts = set(), [], []
    at = 0
    while at < len(module_data):
        if len(modules) >= max_modules:
            raise ValueError("PDB module table exceeds its record resource bound")
        header = _slice(module_data, at, 64)
        flags, stream = struct.unpack_from("<HH", header, 32)
        lengths = struct.unpack_from("<iii", header, 36)
        # fWritten is a historical writer bit. EC/TSM interpretation is not
        # supplied by this bounded reader; refuse rather than ignoring it.
        if flags & ~1 or any(value < 0 for value in lengths) or any(header[56:64]):
            raise ValueError("unsupported PDB module flags, EC metadata or signed stream sizes")
        if stream == 0xFFFF:
            if any(lengths):
                raise ValueError("absent PDB module stream has nonempty declared contents")
            stream = None
        else:
            if stream in reserved_streams or stream in used_streams:
                raise ValueError("aliased or reserved PDB module stream")
            raw = msf.stream(stream)
            if raw is None or sum(lengths) > len(raw):
                raise ValueError("PDB module stream lacks declared backing")
            used_streams.add(stream)
        cursor, names = at + 64, []
        for _ in range(2):
            end = module_data.find(b"\0", cursor, min(cursor + 4097, len(module_data)))
            if end < 0:
                raise ValueError("unterminated or oversized PDB module name")
            name = module_data[cursor:end]
            if any(byte < 32 or byte >= 127 for byte in name):
                raise ValueError("unsupported PDB module name encoding")
            names.append(name.decode("ascii"))
            cursor = end + 1
        if not names[0]:
            raise ValueError("empty PDB module identity")
        end = (cursor + 3) & ~3
        _slice(module_data, cursor, end - cursor)  # Alignment padding need not be zero.
        modules.append(Module(names[0], names[1], None, stream, *lengths))
        firsts.append(header[4:32])
        at = end
    # Retain every first-contribution observation without replacing the full
    # table. A nil first entry does not exclude this module's other records.
    count = len(modules)
    modules = tuple(Module(row.name, row.object_name,
        _contribution(first, image, count, nil=True), row.stream,
        row.symbol_bytes, row.line_bytes, row.c13_bytes) for row, first in zip(modules, firsts))
    for index, row in enumerate(modules):
        if row.first is not None and row.first.module != index:
            raise ValueError("PDB module first contribution names another module")
    raw = _slice(dbi, begin + 4, contribution_bytes - 4)
    contributions = tuple(_contribution(raw[at:at + 28], image, count)
                          for at in range(0, len(raw), 28))
    return BoundContributions(sha256(image.data).hexdigest(), sha256(pdb_bytes).hexdigest(), modules, contributions)
