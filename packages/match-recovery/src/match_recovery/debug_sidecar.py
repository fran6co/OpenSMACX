"""ELF32/DWARF debugger hints from an explicitly supplied PE and MSVC link map.

This describes the linked candidate, not the original executable or recovery
acceptance. Incomplete CodeView coverage is reported; it never supplies layout
verification. The command definition belongs to the installed package CLI.
"""
from __future__ import annotations

import hashlib
import os
import tempfile
import re
import struct
from pathlib import Path

from . import debug_codeview as codeview
from .source.mangled import qualified_name
from .pe_coff import PeImage


# ----------------------------------------------------------------- the image

def pe_sections(exe: Path) -> tuple:
    """Image base, section bounds/execution flags, and actual PE entry point.

    The MAP names segments by ordinal - `0001:000a1300` - and only the PE
    itself says where segment one begins. This does not attest that the map
    was produced by the same link; the caller must supply corresponding inputs.
    """
    data = exe.read_bytes()
    parsed = PeImage(data, hashlib.sha256(data).hexdigest())
    out = [(name, span.start - parsed.base, span.end - span.start, bool(flags & 0x20000000))
           for name, (span, _, _), flags in zip(parsed.section_names, parsed.sections, parsed.section_flags)]
    if len({row[0] for row in out}) != len(out):
        raise ValueError("debug sidecar requires unique image section names")
    return parsed.base, out, parsed.entrypoint


# ------------------------------------------------------------------- the map

PUBLIC = re.compile(
    r"^\s*([0-9a-fA-F]{4}):([0-9a-fA-F]{8})\s+(\S+)\s+([0-9a-fA-F]{8})\s*(.*)$")
LINE_BLOCK = re.compile(r"^Line numbers for (\S+)\((.+)\) segment (\S+)")
LINE_ENTRY = re.compile(r"(\d+)\s+([0-9a-fA-F]{4}):([0-9a-fA-F]{8})")


def posix_path(text: str) -> str:
    """Normalize separators without inventing a host mapping for drive letters."""
    return text.replace("\\", "/")


def read_map(path: Path, path_maps=()) -> tuple:
    """`(publics, lines)` from a VC6 link map.

    A public is `(address, name, is_function, object)`. `lines` is
    `{object: [(source, line, segment, offset), ...]}` - KEYED BY OBJECT,
    because that is the unit DWARF calls a compile unit and the map does not:
    it emits one block per (object, source) pair, so a TU reports its
    own lines and its headers' separately. Grouped by source instead,
    a header's rows scatter across the whole image and the compile unit built
    from them spans and misattributes unrelated functions in between.

    `lines` is empty unless the link ran with `/MAPINFO:LINES`. A map without
    it parses fine and simply carries no line numbers, which is worth
    reporting rather than crashing over.
    """
    publics, lines = [], {}
    current, in_publics = None, False
    for raw in path.read_text(errors="replace").splitlines():
        block = LINE_BLOCK.match(raw.strip())
        if block:
            source = posix_path(block.group(2))
            for old, new in path_maps:
                if source.casefold().startswith(old.casefold()):
                    source = new + source[len(old):]
                    break
            current = (Path(posix_path(block.group(1))).name, source)
            lines.setdefault(current[0], [])
            in_publics = False
            continue
        if "Publics by Value" in raw:
            in_publics, current = True, None
            continue
        if current is not None:
            found = LINE_ENTRY.findall(raw)
            if found:
                for number, segment, offset in found:
                    lines[current[0]].append((current[1], int(number),
                                              int(segment, 16),
                                              int(offset, 16)))
                continue
            if raw.strip():
                current = None
            continue
        if not in_publics:
            continue
        public = PUBLIC.match(raw)
        if public:
            rest = public.group(5).split()
            owner = rest[-1].split(":")[-1] if rest else ""
            publics.append((int(public.group(4), 16), public.group(3),
                            "f" in rest[:2], owner))
    return publics, lines


# ------------------------------------------------------------------ encoding

def uleb(value: int) -> bytes:
    if value < 0:
        raise ValueError("negative unsigned DWARF value")
    out = bytearray()
    while True:
        byte = value & 0x7F
        value >>= 7
        out.append(byte | (0x80 if value else 0))
        if not value:
            return bytes(out)


def sleb(value: int) -> bytes:
    out = bytearray()
    while True:
        byte = value & 0x7F
        value >>= 7
        done = (value == 0 and not byte & 0x40) or (value == -1 and byte & 0x40)
        out.append(byte | (0 if done else 0x80))
        if done:
            return bytes(out)


# DWARF 2. The line program's standard opcodes and their argument counts, and
# the special-opcode parameters - `line_base`/`line_range` decide which
# (address, line) steps fit in one byte, and nothing here depends on them
# being tight, so they are the conventional ones.
OPCODE_BASE = 10
STANDARD_LENGTHS = bytes([0, 1, 1, 1, 1, 0, 0, 0, 1])
LINE_BASE, LINE_RANGE = -5, 14
DW_LNS_copy, DW_LNS_advance_pc, DW_LNS_advance_line, DW_LNS_set_file = 1, 2, 3, 4
DW_LNE_end_sequence, DW_LNE_set_address = 1, 2


def line_program(sources: list, sequences: list) -> bytes:
    """One DWARF 2 line-number program for one object file.

    `sources` is the file table - a translation unit's lines come from its
    `.cpp` AND every header it defines a body in, and the map reports those as
    separate blocks. `sequences` is `[(rows, end), ...]` with rows
    `[(address, file_index, line), ...]`, ONE SEQUENCE PER FUNCTION: the state
    machine attributes every address from one row to the next, so a single
    sequence spanning the unit would claim the gaps between its functions -
    which the linker fills with other units' COMDATs. Bounding each function
    closes that. `end` is the function's end address: end_sequence marks the
    first address OUTSIDE the sequence, so ending on the last row's own
    address makes its range empty - a function with a single row (a
    header-defined body like `MainInterface::MainInterface`, whose whole body
    is one map row) then has no line info at all.
    """
    body = bytearray()
    for rows, end in sequences:
        body += b"\x00" + uleb(5) + bytes([DW_LNE_set_address])
        body += struct.pack("<I", rows[0][0])
        address, line, source_file = rows[0][0], 1, 0
        for row_address, row_file, row_line in rows:
            if row_file != source_file:
                body += bytes([DW_LNS_set_file]) + uleb(row_file)
                source_file = row_file
            if row_address > address:
                body += bytes([DW_LNS_advance_pc]) + uleb(row_address - address)
                address = row_address
            if row_line != line:
                body += bytes([DW_LNS_advance_line]) + sleb(row_line - line)
                line = row_line
            body += bytes([DW_LNS_copy])
        if end > address:
            body += bytes([DW_LNS_advance_pc]) + uleb(end - address)
        body += b"\x00" + uleb(1) + bytes([DW_LNE_end_sequence])

    prologue = bytearray()
    prologue += bytes([1])                      # minimum_instruction_length
    prologue += bytes([1])                      # default_is_stmt
    prologue += struct.pack("<b", LINE_BASE)
    prologue += bytes([LINE_RANGE, OPCODE_BASE])
    prologue += STANDARD_LENGTHS
    prologue += b"\x00"                         # include_directories: none
    for source in sources:
        prologue += source.encode() + b"\x00" + uleb(0) + uleb(0) + uleb(0)
    prologue += b"\x00"                         # file_names terminator

    out = bytearray()
    out += struct.pack("<HI", 2, len(prologue))
    out += prologue
    out += body
    return struct.pack("<I", len(out)) + bytes(out)


# DWARF tags, attributes and forms - only what is emitted here.
TAG = dict(compile_unit=0x11, subprogram=0x2E, variable=0x34,
           formal_parameter=0x05, base_type=0x24, pointer_type=0x0F,
           const_type=0x26, volatile_type=0x35, structure_type=0x13,
           class_type=0x02, union_type=0x17, member=0x0D, inheritance=0x1C,
           array_type=0x01, subrange_type=0x21, enumeration_type=0x04,
           enumerator=0x28, subroutine_type=0x15)
AT = dict(name=0x03, byte_size=0x0B, encoding=0x3E, type=0x49, low_pc=0x11,
          high_pc=0x12, external=0x3F, frame_base=0x40, location=0x02,
          data_member_location=0x38, const_value=0x1C, upper_bound=0x2F,
          declaration=0x3C, producer=0x25, language=0x13, comp_dir=0x1B,
          stmt_list=0x10, prototyped=0x27)
FORM = dict(addr=0x01, block1=0x0A, data2=0x05, data4=0x06, string=0x08,
            block=0x09, data1=0x0B, flag=0x0C, sdata=0x0D, udata=0x0F,
            ref4=0x13)

DW_OP_fbreg, DW_OP_reg5 = 0x91, 0x55   # reg5 is ebp in the i386 numbering


class Die:
    """One debugging information entry, with references left as objects.

    A DIE's size does not depend on what it points AT - `DW_FORM_ref4` is four
    bytes either way - so sizing and offset assignment finish before any
    reference is resolved, and a type graph with cycles needs no special case.
    """

    def __init__(self, tag: str, attributes: list = None, children: list = None):
        self.tag = tag
        self.attributes = attributes or []
        self.children = children if children is not None else []
        self.offset = 0

    def add(self, child: "Die") -> "Die":
        self.children.append(child)
        return child

    def shape(self) -> tuple:
        return (TAG[self.tag], bool(self.children),
                tuple((AT[name], FORM[form]) for name, form, _
                      in self.attributes))

    def walk(self):
        yield self
        for child in self.children:
            yield from child.walk()


def encoded(form: str, value) -> bytes:
    """One attribute value, with references left to the caller to resolve."""
    if form == "string":
        return value.encode("latin-1", "replace") + b"\x00"
    if form == "addr" or form == "data4":
        return struct.pack("<I", value & 0xFFFFFFFF)
    if form == "data2":
        return struct.pack("<H", value & 0xFFFF)
    if form in ("data1", "flag"):
        return bytes([value & 0xFF])
    if form == "udata":
        return uleb(value)
    if form == "sdata":
        return sleb(value)
    if form == "block1":
        return bytes([len(value)]) + value
    if form == "ref4":
        return struct.pack("<I", value)
    raise ValueError(form)


class Abbreviations:
    """The shared abbreviation table every unit points at offset zero of."""

    def __init__(self):
        self.codes, self.order = {}, []

    def code(self, shape: tuple) -> int:
        if shape not in self.codes:
            self.codes[shape] = len(self.order) + 1
            self.order.append(shape)
        return self.codes[shape]

    def encode(self) -> bytes:
        out = bytearray()
        for index, (tag, has_children, attributes) in enumerate(self.order, 1):
            out += uleb(index) + uleb(tag) + bytes([1 if has_children else 0])
            for attribute, form in attributes:
                out += uleb(attribute) + uleb(form)
            out += uleb(0) + uleb(0)
        return bytes(out + uleb(0))


def serialise(root: Die, abbreviations: Abbreviations) -> bytes:
    """One compile unit: assign offsets, then write.

    The same walk twice, which is what lets a member's type point forwards to a
    class that points back at it. The end-of-children marker is a byte no DIE
    owns, so it has to be counted in the offset pass as well as written in the
    second - a member DIE landing one byte early is a type gdb reads as
    garbage.
    """
    header = 11                        # length(4) version(2) abbrev(4) size(1)
    at = header

    def assign(die: Die) -> None:
        nonlocal at
        die.offset = at
        at += len(uleb(abbreviations.code(die.shape())))
        for _, form, value in die.attributes:
            at += 4 if form == "ref4" else len(encoded(form, value))
        for child in die.children:
            assign(child)
        if die.children:
            at += 1

    assign(root)
    total = at

    def write(die: Die, out: bytearray) -> None:
        out += uleb(abbreviations.code(die.shape()))
        for _, form, value in die.attributes:
            if form == "ref4":
                out += struct.pack("<I", value.offset if isinstance(value, Die)
                                   else value)
            else:
                out += encoded(form, value)
        for child in die.children:
            write(child, out)
        if die.children:
            out += b"\x00"

    body = bytearray()
    write(root, body)
    return struct.pack("<IHIB", total - 4, 2, 0, 4) + bytes(body)


class Types:
    """CodeView type indices to DIEs, for one compile unit.

    Memoised BEFORE recursing, because the graph has cycles: a class holds a
    pointer to itself, and its members cannot be filled in until the class DIE
    exists to point at.
    """

    def __init__(self, table: dict, unit: Die):
        self.table, self.unit, self.cache = table, unit, {}

    def size(self, index: int, seen=frozenset()) -> int:
        if index in seen:
            return 0
        record = codeview.definition(self.table, index) or {}
        if record.get("kind") == "pointer":
            return 4
        if record.get("kind") in ("base", "struct", "union", "array"):
            return record.get("size", 0)
        if record.get("kind") == "modifier":
            return self.size(record["target"], seen | {index})
        if record.get("kind") == "enum":
            return self.size(record["underlying"], seen | {index})
        return 0

    def die(self, index: int) -> Die:
        """The DIE for a type index, or None for void and the unreadable."""
        if index in self.cache:
            return self.cache[index]
        record = codeview.definition(self.table, index)
        if record is None or record.get("kind") in ("void", "unknown"):
            self.cache[index] = None
            return None
        built = self._build(index, record)
        return built

    def _build(self, index: int, record: dict) -> Die:
        kind = record["kind"]
        if kind == "base":
            die = Die("base_type", [
                ("name", "string", record["name"]),
                ("encoding", "data1", record["encoding"]),
                ("byte_size", "data1", record["size"])])
            return self._keep(index, die)
        if kind == "pointer":
            die = self._keep(index, Die("pointer_type",
                                        [("byte_size", "data1", 4)]))
            target = self.die(record["target"])
            if target is not None:
                die.attributes.append(("type", "ref4", target))
            return die
        if kind == "modifier":
            if not record.get("const") and not record.get("volatile"):
                self.cache[index] = None
                self.cache[index] = self.die(record["target"])
                return self.cache[index]
            tag = "const_type" if record.get("const") else "volatile_type"
            die = self._keep(index, Die(tag))
            target = self.die(record["target"])
            if record.get("const") and record.get("volatile"):
                volatile = self.unit.add(Die("volatile_type"))
                if target is not None:
                    volatile.attributes.append(("type", "ref4", target))
                target = volatile
            if target is not None:
                die.attributes.append(("type", "ref4", target))
            return die
        if kind in ("struct", "union"):
            return self._aggregate(index, record)
        if kind == "array":
            return self._array(index, record)
        if kind == "enum":
            return self._enumeration(index, record)
        if kind == "function":
            die = self._keep(index, Die("subroutine_type",
                                        [("prototyped", "flag", 1)]))
            returns = self.die(record.get("returns", 0))
            if returns is not None:
                die.attributes.append(("type", "ref4", returns))
            return die
        self.cache[index] = None
        return None

    def _keep(self, index: int, die: Die) -> Die:
        self.cache[index] = die
        self.unit.add(die)
        return die

    def _aggregate(self, index: int, record: dict) -> Die:
        tag = "union_type" if record["kind"] == "union" else "structure_type"
        fields = self.table.get(record.get("field", 0)) or {}
        incomplete = record.get("forward") or (record.get("field") and fields.get("kind") != "fieldlist")
        attributes = ([("declaration", "flag", 1)] if incomplete else
                      [("byte_size", "udata", record.get("size", 0))])
        if record.get("name"):
            attributes.insert(0, ("name", "string", record["name"]))
        die = self._keep(index, Die(tag, attributes))
        if incomplete:
            return die
        for member in fields.get("members", []):
            if member["kind"] == "member":
                target = self.die(member["type"])
                if target is None:
                    continue
                die.add(Die("member", [
                    ("name", "string", member["name"]),
                    ("type", "ref4", target),
                    ("data_member_location", "block1",
                     bytes([0x23]) + uleb(member["offset"]))]))
            elif member["kind"] == "base":
                target = self.die(member["type"])
                if target is None:
                    continue
                die.add(Die("inheritance", [
                    ("type", "ref4", target),
                    ("data_member_location", "block1",
                     bytes([0x23]) + uleb(member["offset"]))]))
        return die

    def _array(self, index: int, record: dict) -> Die:
        die = self._keep(index, Die("array_type"))
        element = self.die(record["element"])
        if element is None:
            self.cache[index] = None
            return None
        die.attributes.append(("type", "ref4", element))
        stride = self.size(record["element"])
        size = record.get("size", 0)
        if stride and size > 0 and size % stride == 0:
            die.add(Die("subrange_type", [
                ("upper_bound", "udata",
                 size // stride - 1)]))
        return die

    def _enumeration(self, index: int, record: dict) -> Die:
        size = self.size(record["underlying"])
        attributes = [("byte_size", "data1", size)] if size else []
        if record.get("name"):
            attributes.insert(0, ("name", "string", record["name"]))
        die = self._keep(index, Die("enumeration_type", attributes))
        fields = self.table.get(record.get("field", 0)) or {}
        for member in fields.get("members", []):
            if member["kind"] == "enumerator":
                die.add(Die("enumerator", [
                    ("name", "string", member["name"]),
                    ("const_value", "sdata", member["value"])]))
        return die


# --------------------------------------------------------------------- ELF32

SHT_PROGBITS, SHT_SYMTAB, SHT_STRTAB, SHT_NOBITS = 1, 2, 3, 8
SHF_WRITE, SHF_ALLOC, SHF_EXECINSTR = 1, 2, 4


def elf(sections: list, symbols: list, debug: dict, entry: int) -> bytes:
    """An ET_EXEC ELF32/i386 carrying addresses, symbols and DWARF only.

    The program's sections are NOBITS: the bytes live in the target gdb is
    attached to, and duplicating them here would only invite the two copies
    to disagree.
    """
    names = bytearray(b"\x00")

    def name_offset(text: str) -> int:
        offset = len(names)
        names.extend(text.encode() + b"\x00")
        return offset

    strtab = bytearray(b"\x00")

    def string_offset(text: str) -> int:
        offset = len(strtab)
        strtab.extend(text.encode() + b"\x00")
        return offset

    # Section index 0 is SHN_UNDEF; the program's sections follow in map order
    # so a symbol can name the one that holds it.
    entries = [dict(name="", type=0, flags=0, addr=0, size=0, data=b"", link=0,
                    info=0, align=0, entsize=0)]
    section_index = {}
    for index, (name, address, size, executable) in enumerate(sections, start=1):
        section_index[name] = index
        entries.append(dict(
            name=name, type=SHT_NOBITS,
            flags=SHF_ALLOC | (SHF_EXECINSTR if executable else SHF_WRITE),
            addr=address, size=size, data=b"", link=0, info=0, align=4,
            entsize=0))

    symbol_bytes = bytearray(struct.pack("<IIIBBH", 0, 0, 0, 0, 0, 0))
    local_count = 1
    for name, address, size, section, is_function in symbols:
        symbol_bytes += struct.pack(
            "<IIIBBH", string_offset(name), address, size,
            (1 << 4) | (2 if is_function else 1), 0,
            section_index.get(section, 0))

    for label, payload in debug.items():
        entries.append(dict(name=label, type=SHT_PROGBITS, flags=0, addr=0,
                            size=len(payload), data=payload, link=0, info=0,
                            align=1, entsize=0))

    symtab_index = len(entries)
    entries.append(dict(name=".symtab", type=SHT_SYMTAB, flags=0, addr=0,
                        size=len(symbol_bytes), data=bytes(symbol_bytes),
                        link=symtab_index + 1, info=local_count, align=4,
                        entsize=16))
    entries.append(dict(name=".strtab", type=SHT_STRTAB, flags=0, addr=0,
                        size=len(strtab), data=bytes(strtab), link=0, info=0,
                        align=1, entsize=0))
    entries.append(dict(name=".shstrtab", type=SHT_STRTAB, flags=0, addr=0,
                        size=0, data=b"", link=0, info=0, align=1, entsize=0))

    for entry_record in entries:
        entry_record["name_offset"] = name_offset(entry_record["name"]) \
            if entry_record["name"] else 0
    entries[-1]["data"] = bytes(names)
    entries[-1]["size"] = len(names)

    header_size, section_header_size = 52, 40
    offset = header_size
    for entry_record in entries:
        if entry_record["type"] in (SHT_NOBITS, 0):
            entry_record["offset"] = offset
            continue
        align = entry_record["align"] or 1
        offset += (-offset) % align
        entry_record["offset"] = offset
        offset += len(entry_record["data"])
    section_headers = offset

    out = bytearray()
    out += b"\x7fELF" + bytes([1, 1, 1, 0]) + bytes(8)
    out += struct.pack("<HHIIIIIHHHHHH", 2, 3, 1, entry, 0, section_headers,
                       0, header_size, 32, 0, section_header_size,
                       len(entries), len(entries) - 1)
    for entry_record in entries:
        if entry_record["type"] in (SHT_NOBITS, 0):
            continue
        out += bytes((-len(out)) % (entry_record["align"] or 1))
        out += entry_record["data"]
    for entry_record in entries:
        out += struct.pack(
            "<IIIIIIIIII", entry_record["name_offset"], entry_record["type"],
            entry_record["flags"], entry_record["addr"], entry_record["offset"],
            entry_record["size"], entry_record["link"], entry_record["info"],
            entry_record["align"], entry_record["entsize"])
    return bytes(out)


# ---------------------------------------------------------------------- main

def objects(root: Path) -> dict:
    """`{basename: path}` for every object the build produced.

    Keyed by basename because that is how the map names them, both in the
    publics table (`alpha.cpp.obj`) and in the line blocks.
    """
    found = {}
    for path in sorted(root.rglob("*.obj")):
        if path.name in found:
            raise ValueError(f"ambiguous debug object basename: {path.name}")
        found[path.name] = path
    return found


def render(exe: Path, map_path: Path, comp_dir: Path, build_root: Path,
           *, frame_base="none", path_maps=()) -> tuple[bytes, dict]:
    if frame_base not in ("none", "ebp"):
        raise ValueError("unsupported debug frame base")
    image_base, pe, entrypoint = pe_sections(exe)
    publics, lines = read_map(map_path, path_maps)
    publics.sort()
    if not publics:
        raise ValueError("link map has no public symbols")
    diagnostics = []

    sections = [(name, image_base + address, size, executable)
                for name, address, size, executable in pe]
    segment_base = {index: image_base + address
                    for index, (_, address, _, _) in enumerate(pe, start=1)}

    def owning_section(address: int) -> str:
        for name, start, size, _ in sections:
            if start <= address < start + size:
                return name
        return None

    # A public's extent is the next public's address: VC6's map records where
    # each one starts and nothing about how long it is.
    symbols, by_object, address_of = [], {}, {}
    ends, following = {}, None
    for address in sorted({row[0] for row in publics}, reverse=True):
        ends[address] = following if following is not None else address + 1
        following = address
    for index, (address, mangled, is_function, owner) in enumerate(publics):
        section = owning_section(address)
        if section is None:
            diagnostics.append(f"public outside image sections: {mangled}")
            continue
        _, low, length, executable = next(row for row in sections if row[0] == section)
        if is_function and not executable:
            raise ValueError(f"function public outside executable section: {mangled}")
        following = ends[address]
        size = min(following, low + length) - address if is_function else 1
        pretty = qualified_name(mangled) or mangled
        symbols.append((pretty, address, size, section,
                        is_function))
        if is_function:
            by_object.setdefault(owner, []).append((pretty, address,
                                                    address + size))
            key = owner, mangled
            if key in address_of and address_of[key] != (address, address + size):
                raise ValueError(f"ambiguous linked debug symbol: {mangled}")
            address_of[key] = address, address + size

    available = objects(build_root)
    providers = []
    for name, path in available.items():
        # The section identity, not a project-specific filename, identifies PCH.
        try:
            obj = codeview.Object(path)
            if any(s.name == ".debug$P" for s in obj.sections):
                provider = codeview.precompiled(path)
                providers.append(provider)
                diagnostics.extend(f"{name}: {note}" for note in provider["diagnostics"])
        except (OSError, ValueError) as error:
            diagnostics.append(f"{name}: {error}")

    abbreviations = Abbreviations()
    debug_line, debug_info = bytearray(), bytearray()
    units = attributed = described = located = 0
    for owner in sorted(set(lines) | set(by_object)):
        functions = sorted(by_object.get(owner, []), key=lambda item: item[1])
        if not functions:
            continue
        entries = lines.get(owner, [])
        sources = sorted({source for source, _, _, _ in entries})
        index_of = {source: number for number, source in enumerate(sources, 1)}
        rows = []
        for source, number, segment, offset in entries:
            if segment not in segment_base or not 0 <= offset < pe[segment - 1][2]:
                diagnostics.append(f"{owner}: line outside image section")
                continue
            rows.append((segment_base[segment] + offset, index_of[source], number))
        rows = sorted(set(rows))

        # Every row belongs to the function that contains it; rows outside all
        # of them - the map does emit a few - describe addresses this unit does
        # not own, and attributing them would be worse than dropping them.
        sequences, cursor = [], 0
        for _, low, high in functions:
            taken = []
            while cursor < len(rows) and rows[cursor][0] < low:
                cursor += 1
            while cursor < len(rows) and rows[cursor][0] < high:
                taken.append(rows[cursor])
                cursor += 1
            if taken:
                sequences.append((taken, high))
        attributed += sum(len(taken) for taken, _ in sequences)

        stem = Path(owner).name.replace(".obj", "")
        primary = next((s for s in sources if Path(s).name == stem),
                       sources[0] if sources else stem)
        unit = Die("compile_unit", [
            ("producer", "string", "match-recovery debug-sidecar"),
            ("language", "data1", 0x04),
            ("name", "string", primary),
            ("comp_dir", "string", str(comp_dir)),
            ("low_pc", "addr", min(low for _, low, _ in functions)),
            ("high_pc", "addr", max(high for _, _, high in functions))])
        if sequences:
            unit.attributes.append(("stmt_list", "data4", len(debug_line)))
            debug_line += line_program(sources, sequences)

        # The CodeView in the object is what carries locals and types; the map
        # carries where the linker put them. Neither alone is enough.
        described_here = set()
        source = available.get(owner)
        if source is not None:
            try:
                read = codeview.read(source, providers)
            except (OSError, ValueError, struct.error, IndexError) as error:
                diagnostics.append(f"{owner}: {error}")
                read = None
            if read:
                diagnostics.extend(f"{owner}: {note}" for note in read["diagnostics"])
                types = Types(read["types"], unit)
                for procedure in read["procedures"]:
                    placed = address_of.get((owner, procedure.get("mangled") or ""))
                    if placed is None:
                        continue
                    low, high = placed
                    described_here.add(low)
                    described += 1
                    located += _subprogram(unit, types, procedure, low, high, frame_base=frame_base)
        else:
            diagnostics.append(f"{owner}: no unique local object for debug types/locals")

        for name, low, high in functions:
            if low in described_here:
                continue
            unit.add(Die("subprogram", [
                ("name", "string", name),
                ("low_pc", "addr", low),
                ("high_pc", "addr", high),
                ("external", "flag", 1)]))
        debug_info += serialise(unit, abbreviations)
        units += 1

    payload = {".debug_info": bytes(debug_info),
               ".debug_abbrev": abbreviations.encode(),
               ".debug_line": bytes(debug_line)}
    result = elf(sections, symbols, payload, entrypoint or 0)
    return result, dict(symbols=len(symbols),
                functions=sum(len(v) for v in by_object.values()),
                units=units, sources=len(lines),
                rows=sum(len(v) for v in lines.values()),
                attributed=attributed, described=described, located=located,
                diagnostics=sorted(set(diagnostics)), frame_base=frame_base,
                verification="debug_hints_only", ready_for_collection=False,
                limitations=["public extents bounded by next distinct public and image section, not proven function extents",
                             "link map/object correspondence supplied by caller; no final-link identity attestation",
                             "unsupported debug types and locals remain incomplete; virtual-base locations are not emitted",
                             "frame locations omitted unless caller explicitly guarantees EBP-relative frames"])


def _subprogram(unit: Die, types: "Types", procedure: dict, low: int,
                high: int, *, frame_base="none") -> int:
    """One function DIE with its locals; returns how many locals were placed.

    EBP-relative locations require the caller's explicit frame-base contract.
    A toolchain may omit EBP, so the backend cannot infer this from a build name.
    """
    attributes = [("name", "string", procedure["name"]),
                  ("low_pc", "addr", low), ("high_pc", "addr", high),
                  ("external", "flag", 1 if procedure["external"] else 0)]
    if frame_base == "ebp":
        attributes.append(("frame_base", "block1", bytes([DW_OP_reg5])))
    die = unit.add(Die("subprogram", attributes))
    signature = types.table.get(procedure.get("type", 0)) or {}
    if signature.get("kind") == "function":
        returns = types.die(signature.get("returns", 0))
        if returns is not None:
            die.attributes.append(("type", "ref4", returns))
    placed = 0
    if frame_base != "ebp":
        return placed
    for local in procedure["locals"]:
        target = types.die(local["type"])
        tag = "formal_parameter" if local["offset"] > 0 else "variable"
        entry = [("name", "string", local["name"]),
                 ("location", "block1",
                  bytes([DW_OP_fbreg]) + sleb(local["offset"]))]
        if target is not None:
            entry.insert(1, ("type", "ref4", target))
        die.add(Die(tag, entry))
        placed += 1
    return placed


def build(exe: Path, map_path: Path, output: Path, comp_dir: Path, build_root: Path,
          *, frame_base="none", path_maps=()) -> dict:
    """Publish one disposable sidecar after checking input/output freshness.

    The caller supplies an output artifact path. Existing source, image, map and
    object inputs cannot be overwritten, even through aliases. Unsupported debug
    information is reported, while malformed primary inputs refuse publication.
    """
    exe, map_path, output, comp_dir, build_root = (
        Path(p).absolute() for p in (exe, map_path, output, comp_dir, build_root))
    if any(not isinstance(pair, (list, tuple)) or len(pair) != 2 or not pair[0]
           or any(not isinstance(word, str) or "\0" in word for word in pair) for pair in path_maps):
        raise ValueError("path maps require nonempty source prefixes and replacement strings")
    if not comp_dir.is_dir() or not build_root.is_dir():
        raise ValueError("debug source and object directories must exist")
    if output.resolve() != output or output.suffix != ".sym":
        raise ValueError("sidecar output must be a non-aliased .sym artifact")
    if not output.parent.is_dir():
        raise ValueError("sidecar output directory must exist")

    def snapshot():
        paths = {exe, map_path, *objects(build_root).values()}
        if output in paths:
            raise ValueError("sidecar output overlaps an input")
        result = {}
        for path in sorted(paths):
            if path.resolve() != path or not path.is_file() or path.stat().st_nlink != 1:
                raise ValueError(f"unsafe debug input: {path}")
            result[str(path)] = hashlib.sha256(path.read_bytes()).hexdigest()
        return result

    def previous():
        if output.is_symlink() or (output.exists() and (not output.is_file() or output.stat().st_nlink != 1)):
            raise ValueError("unsafe sidecar destination")
        return output.read_bytes() if output.exists() else None

    before, destination = snapshot(), previous()
    data, report = render(exe, map_path, comp_dir, build_root, frame_base=frame_base, path_maps=path_maps)
    if snapshot() != before or previous() != destination:
        raise ValueError("debug inputs or destination changed during sidecar generation")
    temporary = None
    try:
        with tempfile.NamedTemporaryFile(dir=output.parent, prefix=".debug-sidecar-", delete=False) as stream:
            temporary = Path(stream.name)
            stream.write(data)
            stream.flush()
            os.fsync(stream.fileno())
        if output.parent.resolve() != output.parent or previous() != destination:
            raise ValueError("sidecar destination changed before publication")
        os.replace(temporary, output)
    finally:
        if temporary is not None and temporary.exists():
            temporary.unlink()
    return {"operation": "debug_sidecar", "output": str(output), "output_sha256": hashlib.sha256(data).hexdigest(),
            "debug_coverage": "incomplete", "artifact_written": True, **report}
