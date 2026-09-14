"""Complete bounded CV5/ST procedure blocks, not linked correspondence.

Supported records are PROCSYM32 (S_[GL]PROC32_ST), REGSYM
(S_REGISTER_ST), BPRELSYM32 (S_BPREL32_ST), and S_END, from Microsoft's
https://github.com/microsoft/microsoft-pdb/blob/master/include/cvinfo.h .
COFF associated blocks have no stream signature or record padding; the supported
PDB form has zero-padded four-byte records. Type indices and scope links remain
literal values: this decoder neither remaps types nor proves object ownership,
linked placement, discarded storage, or equivalence of two procedure blocks.
"""

from dataclasses import dataclass

from .codeview import Cursor


S_END = 0x0006
S_REGISTER_ST = 0x1001
S_BPREL32_ST = 0x1006
S_LPROC32_ST = 0x100A
S_GPROC32_ST = 0x100B
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

    @property
    def end_record_offset(self):
        return self.records[-1].offset

    @property
    def end_offset(self):
        last = self.records[-1]
        return last.offset + last.size


def decode_procedure(data: bytes, *, alignment=1, base_offset=0) -> Procedure:
    """Decode exactly one procedure, its ordered locals and terminal S_END.

    ``alignment=1`` is the unpadded COFF form. ``alignment=4`` requires exact
    zero padding to four-byte record boundaries. ``base_offset`` is the block's
    position in its enclosing module symbol stream, not a code address. The
    caller must verify parent/end/next links against that complete stream and
    bind procedure types and coordinates independently. Unsupported records,
    nested procedures, absent ends and trailing bytes always refuse.
    """
    if type(data) is not bytes or not 0 < len(data) <= MAX_PROCEDURE_BYTES:
        raise ValueError("CodeView procedure requires bounded immutable bytes")
    if type(alignment) is not int or alignment not in (1, 4):
        raise ValueError("unsupported CodeView procedure record alignment")
    if (type(base_offset) is not int or not 0 <= base_offset <= 0xFFFFFFFF
            or base_offset + len(data) > 0x100000000 or base_offset % alignment):
        raise ValueError("invalid CodeView procedure stream offset")
    stream, records, locals_ = Cursor(data), [], []
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
    return Procedure(**fields, locals=tuple(locals_), records=tuple(records))
