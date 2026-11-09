"""Complete bounded CV5/ST procedure blocks, not linked correspondence.

Supported records are PROCSYM32 (S_[GL]PROC32_ST), REGSYM
(S_REGISTER_ST), BPRELSYM32 (S_BPREL32_ST), UDTSYM (S_UDT_ST, a
function-scope typedef/class name with no address), and S_END, from Microsoft's
https://github.com/microsoft/microsoft-pdb/blob/master/include/cvinfo.h .
COFF associated blocks have no stream signature or record padding; the supported
PDB form has zero-padded four-byte records. Type indices and scope links remain
literal values: this decoder neither remaps types nor proves object ownership,
linked placement, discarded storage, or equivalence of two procedure blocks.
Opt-in decoding also retains S_LDATA32_ST and S_LABEL32_ST coordinates;
their relocation ownership must be proven separately. Default linked decoding
continues to refuse these address-bearing records.
"""

from dataclasses import dataclass

from .codeview import Cursor


S_END = 0x0006
S_REGISTER_ST = 0x1001
S_UDT_ST = 0x1003
S_BPREL32_ST = 0x1006
S_LPROC32_ST = 0x100A
S_GPROC32_ST = 0x100B
S_LDATA32_ST = 0x1007
S_LABEL32_ST = 0x0209
MAX_PROCEDURE_BYTES = 16 * 1024 * 1024
MAX_PROCEDURE_RECORDS = 100000


@dataclass(frozen=True)
class Record:
    """Absolute stream offset, exact encoded size, and validated padding."""

    offset: int
    size: int
    kind: int
    padding: bytes


@dataclass(frozen=True)
class Local:
    kind: int
    type_index: int
    name: str
    register: int | None = None
    frame_offset: int | None = None


@dataclass(frozen=True)
class LocalType:
    """A procedure-scope S_UDT_ST name; it owns no storage or address."""

    type_index: int
    name: str


@dataclass(frozen=True)
class LocalAddress:
    kind: int
    record_offset: int
    offset: int
    section: int
    name: str
    type_index: int | None = None
    flags: int | None = None


@dataclass(frozen=True)
class Procedure:
    kind: int
    parent: int
    end: int
    next: int
    length: int
    debug_start: int
    debug_end: int
    type_index: int
    offset: int
    section: int
    flags: int
    name: str
    locals: tuple[Local, ...]
    records: tuple[Record, ...]
    addresses: tuple[LocalAddress, ...] = ()
    types: tuple[LocalType, ...] = ()

    @property
    def end_record_offset(self):
        return self.records[-1].offset

    @property
    def end_offset(self):
        last = self.records[-1]
        return last.offset + last.size


def decode_procedure(data: bytes, *, alignment=1, base_offset=0, local_addresses=False) -> Procedure:
    """Decode exactly one procedure, its ordered locals and terminal S_END.

    ``alignment=1`` is the unpadded COFF form. ``alignment=4`` requires exact
    zero padding to four-byte record boundaries. ``base_offset`` is the block's
    position in its enclosing module symbol stream, not a code address. The
    caller must verify parent/end/next links against that complete stream and
    bind procedure types and coordinates independently. Unsupported records,
    nested procedures, absent ends and trailing bytes always refuse.
    ``local_addresses`` only enables complete syntactic decoding of local data
    and label records; it does not bind their coordinates or establish types.
    """
    if type(data) is not bytes or not 0 < len(data) <= MAX_PROCEDURE_BYTES:
        raise ValueError("CodeView procedure requires bounded immutable bytes")
    if type(alignment) is not int or alignment not in (1, 4):
        raise ValueError("unsupported CodeView procedure record alignment")
    if type(local_addresses) is not bool:
        raise ValueError("local address decoding requires an explicit boolean")
    if (type(base_offset) is not int or not 0 <= base_offset <= 0xFFFFFFFF
            or base_offset + len(data) > 0x100000000 or base_offset % alignment):
        raise ValueError("invalid CodeView procedure stream offset")
    stream, records, locals_, addresses, types = Cursor(data), [], [], [], []
    fields = None
    ended = False
    while stream.at < len(data):
        if len(records) >= MAX_PROCEDURE_RECORDS:
            raise ValueError("CodeView procedure exceeds its record bound")
        at = stream.at
        size = stream.number("<H")
        if size < 2:
            raise ValueError("invalid CodeView procedure record length")
        record = Cursor(stream.take(size))
        kind = record.number("<H")
        if fields is None:
            if kind not in (S_LPROC32_ST, S_GPROC32_ST):
                raise ValueError("CodeView block must begin with one supported procedure")
            fields = {"kind": kind}
            for name in ("parent", "end", "next", "length", "debug_start", "debug_end", "type_index", "offset"):
                fields[name] = record.number("<I")
            fields["section"] = record.number("<H")
            fields["flags"] = record.number("<B")
            fields["name"] = record.name()
            if (not fields["length"] or not 0 <= fields["debug_start"] <= fields["debug_end"] <= fields["length"]
                    or fields["offset"] + fields["length"] > 0x100000000):
                raise ValueError("invalid CodeView procedure code/debug bounds")
        elif kind == S_REGISTER_ST:
            index, register, name = record.number("<I"), record.number("<H"), record.name()
            locals_.append(Local(kind, index, name, register=register))
        elif kind == S_BPREL32_ST:
            offset, index, name = record.number("<i"), record.number("<I"), record.name()
            locals_.append(Local(kind, index, name, frame_offset=offset))
        elif kind == S_UDT_ST:
            index, name = record.number("<I"), record.name()
            if not index or not name:
                raise ValueError("CodeView local UDT lacks a type or name")
            types.append(LocalType(index, name))
        elif local_addresses and kind == S_LDATA32_ST:
            index, offset, section = record.number("<I"), record.number("<I"), record.number("<H")
            addresses.append(LocalAddress(kind, base_offset + at, offset, section,
                                          record.name(), type_index=index))
        elif local_addresses and kind == S_LABEL32_ST:
            offset, section, flags = record.number("<I"), record.number("<H"), record.number("<B")
            addresses.append(LocalAddress(kind, base_offset + at, offset, section,
                                          record.name(), flags=flags))
        elif kind == S_END:
            ended = True
        else:
            raise ValueError(f"unsupported CodeView procedure record {kind:#x}")
        # Padding is part of this record's declared length, not a gap between
        # records. Require precisely the supported encoding, never strip zeros.
        expected = (-(2 + record.at)) % alignment
        padding = record.take(len(record.data) - record.at)
        if len(padding) != expected or any(padding):
            raise ValueError("invalid CodeView procedure record padding")
        records.append(Record(base_offset + at, size + 2, kind, padding))
        if ended:
            if stream.at != len(data):
                raise ValueError("CodeView procedure has trailing records or bytes")
            break
    if not ended:
        raise ValueError("CodeView procedure lacks a terminal S_END")
    return Procedure(**fields, locals=tuple(locals_), records=tuple(records), addresses=tuple(addresses),
                     types=tuple(types))
