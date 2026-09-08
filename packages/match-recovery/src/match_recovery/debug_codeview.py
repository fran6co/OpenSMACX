"""CV5 debugger types and procedures, not recovery layout verification.

COFF boundaries and variable-length values use the shared checked readers.
PCH imports require matching signatures and index ranges, never type-index votes.
Unsupported debug records remain diagnostics, not empty complete class proofs.
Format: https://github.com/microsoft/microsoft-pdb/blob/master/include/cvinfo.h
"""
from __future__ import annotations

import struct
from pathlib import Path

from .codeview import Cursor
from .pe_coff import Coff

# ------------------------------------------------------------------- symbols

S_END = 0x0006
S_BPREL32 = 0x1006
S_LPROC32 = 0x100A
S_GPROC32 = 0x100B

# --------------------------------------------------------------------- types

LF_MODIFIER = 0x1001
LF_POINTER = 0x1002
LF_ARRAY = 0x1003
LF_CLASS = 0x1004
LF_STRUCTURE = 0x1005
LF_UNION = 0x1006
LF_ENUM = 0x1007
LF_PROCEDURE = 0x1008
LF_MFUNCTION = 0x1009
LF_FIELDLIST = 0x1203
LF_BITFIELD = 0x1205

# The field-list leaves, which are their own 0x140x family. Each entry's
# length is implied by its shape, so ONE unrecognised leaf ends the walk and
# takes the rest of the class with it. Unknown leaves invalidate that field list
# rather than returning a partial class as though every member had been read.
LF_BCLASS = 0x1400
LF_VBCLASS = 0x1401
LF_IVBCLASS = 0x1402
LF_FRIENDFCN = 0x1403
LF_INDEX = 0x1404
LF_MEMBER = 0x1405
LF_STMEMBER = 0x1406
LF_METHOD = 0x1407
LF_NESTTYPE = 0x1408
LF_VFUNCTAB = 0x1409
LF_FRIENDCLS = 0x140A
LF_ONEMETHOD = 0x140B
LF_VFUNCOFF = 0x140C
LF_ENUMERATE = 0x1502

FIRST_USER_TYPE = 0x1000

# DW_ATE_*, so the caller does not have to re-derive what a CV primitive means.
SIGNED, UNSIGNED, SIGNED_CHAR, UNSIGNED_CHAR, FLOAT, BOOLEAN = 5, 7, 6, 8, 4, 2

# CV primitive types are `(mode << 8) | kind`: mode 0 is the type itself, mode 4
# a 32-bit pointer to it. Unsupported kinds remain unknown.
PRIMITIVE = {
    0x00: ("void", 0, SIGNED),
    0x03: ("void", 0, SIGNED),
    0x08: ("HRESULT", 4, SIGNED),
    0x10: ("signed char", 1, SIGNED_CHAR),
    0x11: ("short", 2, SIGNED),
    0x12: ("long", 4, SIGNED),
    0x13: ("long long", 8, SIGNED),
    0x20: ("unsigned char", 1, UNSIGNED_CHAR),
    0x21: ("unsigned short", 2, UNSIGNED),
    0x22: ("unsigned long", 4, UNSIGNED),
    0x23: ("unsigned long long", 8, UNSIGNED),
    0x30: ("bool", 1, BOOLEAN),
    0x40: ("float", 4, FLOAT),
    0x41: ("double", 8, FLOAT),
    0x42: ("long double", 10, FLOAT),
    0x70: ("char", 1, SIGNED_CHAR),
    0x71: ("wchar_t", 2, UNSIGNED),
    0x74: ("int", 4, SIGNED),
    0x75: ("unsigned int", 4, UNSIGNED),
}


def primitive(index: int) -> dict:
    """What a type index below 0x1000 means, or None if it is not one."""
    if index >= FIRST_USER_TYPE:
        return None
    kind, mode = index & 0xFF, (index >> 8) & 0x7
    named = PRIMITIVE.get(kind)
    if named is None:
        return dict(kind="unknown", name=f"cv_{index:#06x}")
    name, size, encoding = named
    base = dict(kind="void") if size == 0 and name == "void" else \
        dict(kind="base", name=name, size=size, encoding=encoding)
    if mode == 4:                            # near 32-bit pointer
        return dict(kind="pointer", target=index & 0xFF, size=4)
    if mode:
        return dict(kind="unknown", name=f"cv_{index:#06x}")
    return base


# ------------------------------------------------------------------ decoding

def pascal(data: bytes, at: int) -> tuple:
    """`(text, next_offset)` for CodeView's length-prefixed names."""
    cursor = Cursor(data)
    cursor.take(at)
    name = cursor.name()
    return name, cursor.at


def numeric(data: bytes, at: int) -> tuple:
    """`(value, next_offset)` for a numeric leaf.

    Under 0x8000 the value IS the halfword; at or above it, the halfword names
    the encoding of what follows.
    """
    cursor = Cursor(data)
    cursor.take(at)
    value = cursor.numeric()
    return value, cursor.at


# ---------------------------------------------------------------------- COFF

IMAGE_SCN_LNK_COMDAT = 0x00001000


class Object:
    """One `.obj`: its sections, its symbols, and the CodeView in them."""

    def __init__(self, path: Path):
        self.path = Path(path)
        self.obj = Coff(self.path.read_bytes())
        self.sections = self.obj.sections
        self.diagnostics = []

    def relocations(self, section) -> dict:
        """`{offset_within_section: mangled_name}`."""
        found = {}
        for address, symbol, kind in section.relocations:
            if address in found:
                raise ValueError("duplicate debug relocation site")
            if kind == 0xB:  # IMAGE_REL_I386_SECREL, the procedure offset
                found[address] = self.obj.symbols[symbol].name
        return found

    # ----------------------------------------------------------------- types

    def type_table(self) -> tuple:
        """Indexed local records plus explicit import/export metadata.

        LF_PRECOMP_ST supplies start/count/signature. LF_ENDPRECOMP marks the
        provider's exported prefix. The import is replaced by that prefix;
        ENDPRECOMP occupies an untranslated local index in the provider. A
        provider may contain further, non-exported local types after its end.
        """
        sections = [s for s in self.sections if s.name in (".debug$T", ".debug$P")]
        if not sections:
            self.diagnostics.append("no CodeView type stream")
            return {}, None, None
        if len(sections) != 1:
            raise ValueError("ambiguous CodeView type streams")
        cursor = Cursor(sections[0].data)
        if cursor.number("<I") != 2:
            raise ValueError("unsupported CodeView type signature")
        table, imported, exported, index = {}, None, None, FIRST_USER_TYPE
        while cursor.at < len(cursor.data):
            size = cursor.number("<H")
            if size < 2:
                raise ValueError("invalid CodeView type length")
            row = Cursor(cursor.take(size))
            leaf = row.number("<H")
            if leaf == 0x100E:
                if table or imported is not None or exported is not None:
                    raise ValueError("misplaced precompiled type import")
                start, count, signature = row.number("<I"), row.number("<I"), row.number("<I")
                name = row.name()
                row.padding()
                if row.at != len(row.data) or start != FIRST_USER_TYPE or count <= 0 or start + count > 0xFFFFFFFF:
                    raise ValueError("unsupported precompiled type range")
                imported = {"start": start, "count": count, "signature": signature, "name": name}
                index = start + count
                continue
            if leaf == 0x0014:
                if imported is not None or exported is not None or sections[0].name != ".debug$P":
                    raise ValueError("misplaced precompiled type export")
                signature = row.number("<I")
                row.padding()
                if row.at != len(row.data):
                    raise ValueError("invalid precompiled type signature record")
                exported = {"start": FIRST_USER_TYPE, "count": index - FIRST_USER_TYPE,
                            "signature": signature, "types": dict(table)}
                index += 1
                continue
            if leaf in (0x0013, 0x0016, 0x1501, 0x1509, 0x1515, 0x1200):
                raise ValueError("unsupported CodeView external type context")
            table[index] = self._type_record(leaf, row.data[2:])
            index += 1
        if sections[0].name == ".debug$P" and exported is None:
            raise ValueError("precompiled type stream has no export signature")
        return table, imported, exported

    def types(self, providers=()) -> dict:
        table, imported, _ = self.type_table()
        if imported:
            matches = [p for p in providers if all(p[key] == imported[key]
                       for key in ("start", "count", "signature"))]
            if len(matches) != 1:
                raise ValueError("precompiled types require one matching signature and index range")
            table = {**matches[0]["types"], **table}
        table = {i: dict(r) for i, r in table.items() if r is not None}
        defined = {}
        for i, record in table.items():
            if record.get("name") and not record.get("forward"):
                defined.setdefault((record["kind"], record["name"]), []).append(i)
        for record in table.values():
            matches = defined.get((record["kind"], record.get("name")), [])
            if record.get("forward") and len(matches) == 1:
                record["definition"] = matches[0]
        return table

    def _type_record(self, leaf: int, body: bytes) -> dict:
        try:
            if leaf == LF_MODIFIER:
                target, attribute = struct.unpack_from("<IH", body, 0)
                return dict(kind="modifier", target=target,
                            const=bool(attribute & 1),
                            volatile=bool(attribute & 2))
            if leaf == LF_POINTER:
                target, attribute = struct.unpack_from("<II", body, 0)
                if attribute & 0x1F != 0xA or (attribute >> 5) & 7:
                    raise ValueError("unsupported pointer representation")
                return dict(kind="pointer", target=target, size=4)
            if leaf == LF_ARRAY:
                element, _ = struct.unpack_from("<II", body, 0)
                size, _ = numeric(body, 8)
                return dict(kind="array", element=element, size=size)
            if leaf in (LF_CLASS, LF_STRUCTURE, LF_UNION):
                count, property_bits = struct.unpack_from("<HH", body, 0)
                if leaf == LF_UNION:
                    field = struct.unpack_from("<I", body, 4)[0]
                    size, at = numeric(body, 8)
                else:
                    field = struct.unpack_from("<I", body, 4)[0]
                    size, at = numeric(body, 16)
                name, _ = pascal(body, at)
                return dict(kind="union" if leaf == LF_UNION else "struct",
                            name=name, size=size, field=field, count=count,
                            forward=bool(property_bits & 0x80))
            if leaf == LF_ENUM:
                count, property_bits = struct.unpack_from("<HH", body, 0)
                underlying, field = struct.unpack_from("<II", body, 4)
                name, _ = pascal(body, 12)
                return dict(kind="enum", name=name, underlying=underlying,
                            field=field, count=count,
                            forward=bool(property_bits & 0x80))
            if leaf == LF_FIELDLIST:
                return dict(kind="fieldlist", members=self._field_list(body))
            if leaf in (LF_PROCEDURE, LF_MFUNCTION):
                returns = struct.unpack_from("<I", body, 0)[0]
                return dict(kind="function", returns=returns)
            if leaf == LF_BITFIELD:
                raise ValueError("bitfield location requires bit-sized DWARF support")
        except (struct.error, IndexError, ValueError) as error:
            self.diagnostics.append(f"type {leaf:#x}: {error}")
            return None
        self.diagnostics.append(f"unsupported type leaf {leaf:#x}")
        return None

    def _field_list(self, body: bytes) -> list:
        """The members of one `LF_FIELDLIST`.

        Data members, base classes and enumerators are kept - what a debugger
        can show. Methods and nested types are walked past rather than
        collected: gdb gets the functions from the symbols, where they carry
        addresses. WALKED PAST, NOT SKIPPED OVER - every leaf's length has to
        be known even to reach the next one.
        """
        members, at = [], 0
        while at + 2 <= len(body):
            leaf = struct.unpack_from("<H", body, at)[0]
            at += 2
            try:
                if leaf == LF_MEMBER:
                    _, index = struct.unpack_from("<HI", body, at)
                    offset, at = numeric(body, at + 6)
                    name, at = pascal(body, at)
                    members.append(dict(kind="member", name=name, type=index,
                                        offset=offset))
                elif leaf == LF_BCLASS:
                    _, index = struct.unpack_from("<HI", body, at)
                    offset, at = numeric(body, at + 6)
                    members.append(dict(kind="base", type=index, offset=offset))
                elif leaf in (LF_VBCLASS, LF_IVBCLASS):
                    # A virtual base is reached through the vbtable rather than
                    # at a fixed offset, so it is walked past: DWARF would need
                    # a location expression this has no way to write.
                    _, _, _ = struct.unpack_from("<HII", body, at)
                    _, at = numeric(body, at + 10)
                    _, at = numeric(body, at)
                    self.diagnostics.append("virtual-base location omitted")
                elif leaf == 0x0403:  # LF_ENUMERATE_ST: length-prefixed name
                    value, at = numeric(body, at + 2)
                    name, at = pascal(body, at)
                    members.append(dict(kind="enumerator", name=name,
                                        value=value))
                elif leaf == LF_ONEMETHOD:
                    attribute = struct.unpack_from("<H", body, at)[0]
                    at += 6
                    if (attribute >> 2) & 7 in (4, 6):   # introducing virtual
                        at += 4
                    _, at = pascal(body, at)
                elif leaf == LF_METHOD:
                    at += 6
                    _, at = pascal(body, at)
                elif leaf in (LF_STMEMBER, LF_NESTTYPE, LF_FRIENDFCN):
                    at += 6
                    _, at = pascal(body, at)
                elif leaf in (LF_VFUNCTAB, LF_FRIENDCLS):
                    at += 6
                else:
                    raise ValueError(f"unsupported field leaf {leaf:#x}")
            except (struct.error, IndexError) as error:
                raise ValueError("truncated CodeView field") from error
            cursor = Cursor(body)
            cursor.take(at)
            cursor.padding()
            at = cursor.at
        if at != len(body):
            raise ValueError("truncated CodeView field leaf")
        return members

    # --------------------------------------------------------------- symbols

    def procedures(self) -> list:
        """Every function in this object, with its locals.

        The mangled name comes from the section's RELOCATION rather than the
        record: CodeView spells a procedure the way a human reads it -
        `Widget::update` - and overloads can share that spelling.
        The relocation at the record's `offset` field names the COFF
        symbol, which is what the linker's map is keyed by.
        """
        found = []
        for section in self.sections:
            if section.name != ".debug$S":
                continue
            blob = section.data
            comdat = bool(section.flags & IMAGE_SCN_LNK_COMDAT)
            relocations = self.relocations(section)
            cursor = Cursor(blob)
            if not comdat and cursor.number("<I") != 2:
                raise ValueError("unsupported CodeView symbol signature")
            current = None
            while cursor.at < len(blob):
                at = cursor.at
                length = cursor.number("<H")
                if length < 2:
                    raise ValueError("invalid CodeView symbol length")
                record = Cursor(cursor.take(length))
                kind = record.number("<H")
                body = record.data[2:]
                if kind in (S_GPROC32, S_LPROC32):
                    current = self._procedure(body, at, relocations,
                                              kind == S_GPROC32)
                    if current:
                        found.append(current)
                elif kind == S_BPREL32 and current is not None:
                    offset, index = struct.unpack_from("<iI", body, 0)
                    name, _ = pascal(body, 8)
                    current["locals"].append(
                        dict(name=name, offset=offset, type=index))
                elif kind == S_END:
                    current = None
        return found

    def _procedure(self, body: bytes, at: int, relocations: dict,
                   external: bool) -> dict:
        try:
            length = struct.unpack_from("<I", body, 12)[0]
            type_index, offset = struct.unpack_from("<II", body, 24)
            name, _ = pascal(body, 35)
        except (struct.error, IndexError, ValueError) as error:
            raise ValueError("truncated CodeView procedure") from error
        # `off` sits 32 bytes into the record, and the relocation that fills it
        # in is the only place the mangled name appears.
        mangled = relocations.get(at + 32)
        return dict(name=name, mangled=mangled, offset=offset, length=length,
                    type=type_index, external=external, locals=[])


def definition(table: dict, index: int) -> dict:
    """The record for `index`, with a forward reference followed to its body.

    A `this` pointer usually names the forward declaration - the compiler saw
    the class declared before it saw it defined - and stopping there shows an
    empty class with size 0.
    """
    record = table.get(index)
    if record is None:
        return primitive(index)
    seen = set()
    while record.get("forward") and record.get("definition") in table:
        target = record["definition"]
        if target in seen:
            return None
        seen.add(target)
        record = table[record["definition"]]
    if record.get("forward"):
        return {"kind": record["kind"], "name": record.get("name"), "forward": True}
    return record


def precompiled(path: Path) -> dict:
    """The PCH object's type table, to be handed to every object it covers."""
    obj = Object(path)
    _, _, exported = obj.type_table()
    if exported is not None:
        exported["diagnostics"] = obj.diagnostics
    return exported


def read(path: Path, providers=()) -> dict:
    """`{"types": {...}, "procedures": [...]}` for one object file."""
    obj = Object(path)
    try:
        types = obj.types(providers)
    except ValueError as error:
        types = {}
        obj.diagnostics.append(str(error))
    return dict(types=types, procedures=obj.procedures(), diagnostics=sorted(set(obj.diagnostics)))
