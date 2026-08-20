#!/usr/bin/env python3
"""Give gdb the symbols VC6 cannot hand it.

gdb reads DWARF, stabs and old-style COFF. VC6 writes a PDB (2.0, the `JG`
container - LLVM's reader refuses it), CodeView in `.debug$S` (signature 2,
where LLVM handles 4), and a COFF symbol table that segfaults gdb 15.1 on
`file` alone. Every byte of debug information exists; the one consumer we want
cannot parse any of it.

So this translates. It reads the linker's MAP - which carries every public with
its final address, and, with `/MAPINFO:LINES`, a line-number table per source
file - and writes an ELF32 whose only content is symbols and DWARF describing
the addresses the VC6 executable actually runs at:

    winedbg --gdb --port 12345 --no-start build/OpenSMACX.exe
    gdb -ex 'file build/OpenSMACX.sym' -ex 'target remote localhost:12345'

THE SIDECAR DESCRIBES THE REAL BINARY. That is the whole reason it exists
rather than a gcc build of the same sources: the questions this project asks -
why does the recovered code fault at 0x0062D3C2 - are about addresses, and a
different compiler moves every one of them.

WHAT IS NOT HERE YET: locals and types. Those live in the CodeView the compiler
already emits into each `.obj` (`/Z7` is on for all 127 translation units) -
`S_BPREL32` for a local's name and frame offset, `.debug$T` for the layouts.
This stage gets names, addresses and lines, which is what `break`, `step`,
`list` and a readable backtrace need.
"""
from __future__ import annotations

import argparse
import re
import struct
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO_ROOT))

from decomp.mangled import qualified_name  # noqa: E402


# ----------------------------------------------------------------- the image

def pe_sections(exe: Path) -> tuple:
    """`(image_base, [(name, virtual_address, virtual_size), ...])`.

    The MAP names segments by ordinal - `0001:000a1300` - and only the PE
    itself says where segment one begins. Reading it here means the sidecar
    cannot disagree with the executable it describes.
    """
    data = exe.read_bytes()
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    if data[pe:pe + 4] != b"PE\0\0":
        raise SystemExit(f"{exe} is not a PE image")
    sections = struct.unpack_from("<H", data, pe + 6)[0]
    optional = struct.unpack_from("<H", data, pe + 20)[0]
    base = struct.unpack_from("<I", data, pe + 24 + 28)[0]
    table = pe + 24 + optional
    out = []
    for i in range(sections):
        offset = table + i * 40
        name = data[offset:offset + 8].rstrip(b"\0").decode("latin-1")
        virtual_size, virtual_address = struct.unpack_from("<II", data, offset + 8)
        out.append((name, virtual_address, virtual_size))
    return base, out


# ------------------------------------------------------------------- the map

PUBLIC = re.compile(
    r"^\s*([0-9a-fA-F]{4}):([0-9a-fA-F]{8})\s+(\S+)\s+([0-9a-fA-F]{8})\s*(.*)$")
LINE_BLOCK = re.compile(r"^Line numbers for (\S+)\((.+)\) segment (\S+)")
LINE_ENTRY = re.compile(r"(\d+)\s+([0-9a-fA-F]{4}):([0-9a-fA-F]{8})")


def posix_path(text: str) -> str:
    """`Z:\\home\\x\\y.cpp` back to `/home/x/y.cpp`."""
    if len(text) > 2 and text[1] == ":":
        text = text[2:]
    return text.replace("\\", "/")


def read_map(path: Path) -> tuple:
    """`(publics, lines)` from a VC6 link map.

    A public is `(address, name, is_function, object)`. `lines` is
    `{object: [(source, line, segment, offset), ...]}` - KEYED BY OBJECT,
    because that is the unit DWARF calls a compile unit and the map does not:
    it emits one block per (object, source) pair, so `win.cpp.obj` reports its
    own lines and `original_seam.h`'s separately. Grouped by source instead,
    a header's rows scatter across the whole image and the compile unit built
    from them spans - and mis-attributes - everything in between. That was a
    real defect here: gdb placed `WinMain` in `buffer.h:100`.

    `lines` is empty unless the link ran with `/MAPINFO:LINES`. A map without
    it parses fine and simply carries no line numbers, which is worth
    reporting rather than crashing over.
    """
    publics, lines = [], {}
    current, in_publics = None, False
    for raw in path.read_text(errors="replace").splitlines():
        block = LINE_BLOCK.match(raw.strip())
        if block:
            current = (Path(block.group(1)).name, posix_path(block.group(2)))
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
    separate blocks. `sequences` is `[[(address, file_index, line), ...], ...]`,
    ONE SEQUENCE PER FUNCTION: the state machine attributes every address from
    one row to the next, so a single sequence spanning the unit would claim the
    gaps between its functions - which the linker fills with other units'
    COMDATs. Bounding each function closes that.
    """
    body = bytearray()
    for rows in sequences:
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


ABBREV = bytes(
    # 1: compile_unit, has children
    uleb(1) + uleb(0x11) + b"\x01"
    + uleb(0x25) + uleb(0x08)   # producer  string
    + uleb(0x13) + uleb(0x0B)   # language  data1
    + uleb(0x03) + uleb(0x08)   # name      string
    + uleb(0x1B) + uleb(0x08)   # comp_dir  string
    + uleb(0x11) + uleb(0x01)   # low_pc    addr
    + uleb(0x12) + uleb(0x01)   # high_pc   addr
    + uleb(0x10) + uleb(0x06)   # stmt_list data4
    + uleb(0) + uleb(0)
    # 2: subprogram, no children
    + uleb(2) + uleb(0x2E) + b"\x00"
    + uleb(0x03) + uleb(0x08)   # name      string
    + uleb(0x11) + uleb(0x01)   # low_pc    addr
    + uleb(0x12) + uleb(0x01)   # high_pc   addr
    + uleb(0x3F) + uleb(0x0C)   # external  flag
    + uleb(0) + uleb(0)
    + uleb(0))


def compile_unit(source: str, comp_dir: str, functions: list,
                 low: int, high: int, stmt_list: int) -> bytes:
    die = bytearray()
    die += uleb(1)
    die += b"opensmacx gdb_sidecar\x00"
    die += bytes([0x04])                        # DW_LANG_C_plus_plus
    die += source.encode() + b"\x00"
    die += comp_dir.encode() + b"\x00"
    die += struct.pack("<II", low, high)
    die += struct.pack("<I", stmt_list)
    for name, start, end in functions:
        die += uleb(2)
        die += name.encode() + b"\x00"
        die += struct.pack("<II", start, end)
        die += bytes([1])
    die += uleb(0)                              # end of children
    header = struct.pack("<HIB", 2, 0, 4)       # version, abbrev offset, addr size
    return struct.pack("<I", len(header) + len(die)) + header + bytes(die)


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

def build(exe: Path, map_path: Path, output: Path, comp_dir: Path) -> dict:
    image_base, pe = pe_sections(exe)
    publics, lines = read_map(map_path)
    publics.sort()

    sections = [(name, image_base + address, size, name.startswith(".text"))
                for name, address, size in pe]
    segment_base = {index: image_base + address
                    for index, (_, address, _) in enumerate(pe, start=1)}

    def owning_section(address: int) -> str:
        for name, start, size, _ in sections:
            if start <= address < start + size:
                return name
        return ".text"

    # A public's extent is the next public's address: VC6's map records where
    # each one starts and nothing about how long it is.
    symbols, by_object = [], {}
    for index, (address, mangled, is_function, owner) in enumerate(publics):
        following = publics[index + 1][0] if index + 1 < len(publics) else address
        size = max(following - address, 1) if is_function else 1
        pretty = qualified_name(mangled) or mangled
        symbols.append((pretty, address, size, owning_section(address),
                        is_function))
        if is_function:
            by_object.setdefault(owner, []).append((pretty, address,
                                                    address + size))

    debug_line, debug_info = bytearray(), bytearray()
    units = attributed = 0
    for owner, entries in sorted(lines.items()):
        functions = sorted(by_object.get(owner, []), key=lambda item: item[1])
        if not functions or not entries:
            continue
        sources = sorted({source for source, _, _, _ in entries})
        index_of = {source: number for number, source in enumerate(sources, 1)}
        rows = sorted({(segment_base[segment] + offset, index_of[source], number)
                       for source, number, segment, offset in entries})

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
                sequences.append(taken)
        if not sequences:
            continue
        attributed += sum(len(sequence) for sequence in sequences)

        stmt_list = len(debug_line)
        debug_line += line_program(sources, sequences)
        # The unit is named for its own translation unit, which is the source
        # whose stem matches the object - `win.cpp.obj` is `win.cpp`, not the
        # first header it happens to define a body in.
        stem = Path(owner).name.replace(".obj", "")
        primary = next((s for s in sources if Path(s).name == stem), sources[0])
        debug_info += compile_unit(
            primary, str(comp_dir), functions,
            min(low for _, low, _ in functions),
            max(high for _, _, high in functions), stmt_list)
        units += 1

    payload = {".debug_info": bytes(debug_info),
               ".debug_abbrev": ABBREV,
               ".debug_line": bytes(debug_line)}
    entry = next((address for address, name, _, _ in publics
                  if name in ("_WinMain@16",
                              "?WinMain@@YGHPAUHINSTANCE__@@0PADH@Z")),
                 image_base)
    output.write_bytes(elf(sections, symbols, payload, entry))
    return dict(symbols=len(symbols),
                functions=sum(len(v) for v in by_object.values()),
                units=units, sources=len(lines),
                rows=sum(len(v) for v in lines.values()),
                attributed=attributed)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--exe", type=Path,
                        default=REPO_ROOT / "build" / "OpenSMACX.exe")
    parser.add_argument("--map", type=Path,
                        default=REPO_ROOT / "build" / "OpenSMACX.map")
    parser.add_argument("--out", type=Path,
                        default=REPO_ROOT / "build" / "OpenSMACX.sym")
    parser.add_argument("--comp-dir", type=Path, default=REPO_ROOT)
    arguments = parser.parse_args()

    for path in (arguments.exe, arguments.map):
        if not path.is_file():
            print(f"gdb-sidecar: {path} is absent"
                  + (" - the link needs /MAP /MAPINFO:LINES"
                     if path is arguments.map else ""), file=sys.stderr)
            return 2

    counted = build(arguments.exe, arguments.map, arguments.out,
                    arguments.comp_dir)
    print(f"{arguments.out}: {counted['symbols']:,} symbols "
          f"({counted['functions']:,} functions), {counted['units']:,} units, "
          f"{counted['attributed']:,} of {counted['rows']:,} line rows "
          f"attributed to a function")
    if not counted["rows"]:
        print("gdb-sidecar: no line numbers in the map - relink with "
              "/MAPINFO:LINES for source-level stepping", file=sys.stderr)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
