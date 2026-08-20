#!/usr/bin/env python3
"""Read the CodeView VC6 puts in every object, so gdb can be given it.

`/Z7` is on for all 127 translation units, which means each `.obj` carries its
own debug information rather than deferring to the PDB: `.debug$S` holds the
symbols - one `S_GPROC32` per function and an `S_BPREL32` per local, with the
name and the frame offset - and `.debug$T` holds the types those indices point
into. This module turns both into plain Python; `gdb_sidecar.py` turns that
into DWARF.

WHY NOT THE PDB, which has the same content already linked: nothing outside
wine can read it. It is version 2.0, the `JG` container, and `llvm-pdbutil`
answers "not a supported file type". The objects are ordinary COFF.

THE FORMAT IS CV5-SHAPED, not the CV8 that modern documentation describes, and
every offset here was read off this tree's own objects rather than assumed:
type indices are 4 bytes, `LF_MEMBER` is 0x1405 (not 0x150D), and a structure
is `count(2) property(2) field(4) derived(4) vshape(4) size(numeric)
name(pascal)`.
"""
from __future__ import annotations

import struct
from pathlib import Path

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
# takes the rest of the class with it - `Palette` came back with no members at
# all until `LF_ONEMETHOD`, which VC6 emits FIRST, was among these.
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
# a 32-bit pointer to it. Only the kinds this tree's objects actually use are
# named; anything else is reported as unknown rather than guessed at.
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
    if mode in (4, 5, 6):                      # near/far 32-bit pointer
        return dict(kind="pointer", target=index & 0xFF, size=4)
    return base


# ------------------------------------------------------------------ decoding

def pascal(data: bytes, at: int) -> tuple:
    """`(text, next_offset)` for CodeView's length-prefixed names."""
    length = data[at]
    return data[at + 1:at + 1 + length].decode("latin-1"), at + 1 + length


def numeric(data: bytes, at: int) -> tuple:
    """`(value, next_offset)` for a numeric leaf.

    Under 0x8000 the value IS the halfword; at or above it, the halfword names
    the encoding of what follows.
    """
    value = struct.unpack_from("<H", data, at)[0]
    if value < 0x8000:
        return value, at + 2
    widths = {0x8000: ("<b", 1), 0x8001: ("<h", 2), 0x8002: ("<H", 2),
              0x8003: ("<i", 4), 0x8004: ("<I", 4), 0x8009: ("<q", 8),
              0x800A: ("<Q", 8)}
    if value not in widths:
        return 0, at + 2
    form, width = widths[value]
    return struct.unpack_from(form, data, at + 2)[0], at + 2 + width


# ---------------------------------------------------------------------- COFF

IMAGE_SCN_LNK_COMDAT = 0x00001000


class Object:
    """One `.obj`: its sections, its symbols, and the CodeView in them."""

    def __init__(self, path: Path):
        self.path = Path(path)
        self.data = self.path.read_bytes()
        count = struct.unpack_from("<H", self.data, 2)[0]
        symbol_pointer, symbol_count = struct.unpack_from("<II", self.data, 8)
        self._symbols = symbol_pointer
        self._strings = symbol_pointer + symbol_count * 18
        self.sections = []
        for index in range(count):
            at = 20 + index * 40
            name = self.data[at:at + 8].rstrip(b"\0").decode("latin-1")
            (_, _, size, pointer, relocations, _, relocation_count,
             _, flags) = struct.unpack_from("<IIIIIIHHI", self.data, at + 8)
            self.sections.append(dict(
                name=name, size=size, pointer=pointer, flags=flags,
                relocations=relocations, relocation_count=relocation_count))

    def symbol_name(self, index: int) -> str:
        at = self._symbols + index * 18
        raw = self.data[at:at + 8]
        if raw[:4] == b"\0\0\0\0":
            offset = struct.unpack_from("<I", raw, 4)[0]
            end = self.data.index(b"\0", self._strings + offset)
            return self.data[self._strings + offset:end].decode("latin-1")
        return raw.rstrip(b"\0").decode("latin-1")

    def relocations(self, section: dict) -> dict:
        """`{offset_within_section: mangled_name}`."""
        found = {}
        for index in range(section["relocation_count"]):
            at = section["relocations"] + index * 10
            address, symbol, _ = struct.unpack_from("<IIH", self.data, at)
            found[address] = self.symbol_name(symbol)
        return found

    # ----------------------------------------------------------------- types

    def type_table(self) -> tuple:
        """`(records, base)` for this object's own type section.

        THE FIRST INDEX IS NOT 0x1000 when a precompiled header is in use, and
        nothing in the section says what it is: the PCH's types occupy the low
        indices and the object's own table continues above them. This tree's
        objects start at 0x22C3, and the PCH that supplies 0x1000 upward is
        `cmake_pch.cxx.obj`, which carries its table in `.debug$P` rather than
        `.debug$T`.

        The base is VOTED FOR rather than assumed, because it is recoverable
        from the data: VC6 emits a class's `LF_FIELDLIST` immediately before
        the class, so `field - (position - 1)` is the base for every one of
        them, and a few hundred structures agree to the index.
        """
        section = next((s for s in self.sections
                        if s["name"] in (".debug$T", ".debug$P")), None)
        if section is None:
            return [], FIRST_USER_TYPE
        blob = self.data[section["pointer"]:section["pointer"] + section["size"]]
        records, at = [], 4
        while at + 4 <= len(blob):
            length = struct.unpack_from("<H", blob, at)[0]
            if not length:
                break
            leaf = struct.unpack_from("<H", blob, at + 2)[0]
            records.append(self._type_record(leaf, blob[at + 4:at + 2 + length]))
            at += 2 + length

        votes = {}
        for position, record in enumerate(records):
            if (record and record["kind"] in ("struct", "union")
                    and not record.get("forward") and record.get("field")):
                candidate = record["field"] - (position - 1)
                votes[candidate] = votes.get(candidate, 0) + 1
        base = max(votes, key=votes.get) if votes else FIRST_USER_TYPE
        return records, base

    def types(self, inherited: dict = None) -> dict:
        """`{index: record}`, forward references resolved.

        `inherited` is the precompiled header's table, which supplies every
        index below this object's base. A forward reference carries the name
        and nothing else; the definition appears elsewhere in the table, and
        left unresolved every `this` would point at an empty class.
        """
        records, base = self.type_table()
        table = dict(inherited or {})
        table = {index: record for index, record in table.items()
                 if index < base}
        for position, record in enumerate(records):
            if record is not None:
                table[base + position] = record

        defined = {r["name"]: i for i, r in table.items()
                   if r.get("name") and not r.get("forward")}
        for record in table.values():
            if record.get("forward") and record.get("name") in defined:
                record["definition"] = defined[record["name"]]
        return table

    def _type_record(self, leaf: int, body: bytes) -> dict:
        try:
            if leaf == LF_MODIFIER:
                target, attribute = struct.unpack_from("<IH", body, 0)
                return dict(kind="modifier", target=target,
                            const=bool(attribute & 1),
                            volatile=bool(attribute & 2))
            if leaf == LF_POINTER:
                target = struct.unpack_from("<I", body, 0)[0]
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
                target = struct.unpack_from("<I", body, 0)[0]
                return dict(kind="modifier", target=target, const=False,
                            volatile=False)
        except (struct.error, IndexError):
            return None
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
                elif leaf == LF_ENUMERATE:
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
                elif leaf in (LF_VFUNCTAB, LF_FRIENDCLS, LF_INDEX, LF_VFUNCOFF):
                    at += 6
                else:
                    break
            except (struct.error, IndexError):
                break
            while at < len(body) and body[at] >= 0xF0:   # LF_PAD
                at += 1
        return members

    # --------------------------------------------------------------- symbols

    def procedures(self) -> list:
        """Every function in this object, with its locals.

        The mangled name comes from the section's RELOCATION rather than the
        record: CodeView spells a procedure the way a human reads it -
        `Palette::set_from_dib` - and this tree has overloads that share that
        spelling. The relocation at the record's `offset` field names the COFF
        symbol, which is what the linker's map is keyed by.
        """
        found = []
        for section in self.sections:
            if section["name"] != ".debug$S":
                continue
            blob = self.data[section["pointer"]:
                             section["pointer"] + section["size"]]
            comdat = bool(section["flags"] & IMAGE_SCN_LNK_COMDAT)
            relocations = self.relocations(section)
            at = 0 if comdat else 4
            current = None
            while at + 4 <= len(blob):
                length = struct.unpack_from("<H", blob, at)[0]
                if not length:
                    break
                kind = struct.unpack_from("<H", blob, at + 2)[0]
                body = blob[at + 4:at + 2 + length]
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
                at += 2 + length
        return found

    def _procedure(self, body: bytes, at: int, relocations: dict,
                   external: bool) -> dict:
        try:
            length = struct.unpack_from("<I", body, 12)[0]
            type_index, offset = struct.unpack_from("<II", body, 24)
            name, _ = pascal(body, 35)
        except (struct.error, IndexError):
            return None
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
    while record.get("forward") and record.get("definition") in table:
        record = table[record["definition"]]
    return record


def precompiled(path: Path) -> dict:
    """The PCH object's type table, to be handed to every object it covers."""
    obj = Object(path)
    records, base = obj.type_table()
    return {base + position: record
            for position, record in enumerate(records) if record is not None}


def read(path: Path, inherited: dict = None) -> dict:
    """`{"types": {...}, "procedures": [...]}` for one object file."""
    obj = Object(path)
    return dict(types=obj.types(inherited), procedures=obj.procedures())
