#!/usr/bin/env python3
"""Measure the populations `docs/EXCLUSIONS.md` declares out of recovery scope.

An exclusion is a promise about a population, and a population is a number.
Written once into prose, that number rots: the counts on the `Recovery State`
line of `AGENTS.md` were all wrong for months, five of them by more than 60%,
because nothing re-derived them. So every figure in `docs/EXCLUSIONS.md` comes
from here, and `--check` re-derives them and fails when the document and the
image disagree.

Two of the measurements exist because the obvious way to take them is wrong.

`push -1; push imm32` looks like an SEH prologue and is not. Counting that byte
sequence over this image returns 1,072; only 385 of those are frame
registrations and the other 687 are ordinary argument pairs like
`push -1; push 0xa0; call sub_5DACB0`. The count that means something anchors on
the *store* - `mov fs:[0], esp` - and reads the handler back out of the nearest
preceding push, which finds 402 real frames. Subtracting one census from the
other invented ~685 `_except_handler3` frames that do not exist.

A linear sweep finds `out dx, al` inside jump tables. MSVC emits switch tables
in `.text`, and an address like `0x00422C20` decodes as `out 0x1f, eax` when a
sweep desynchronises into one. A hit is therefore only credited as code when
the dwords around it do NOT read as a table of addresses into the same
function, which distinguishes the one real port-I/O body in this image from the
data that impersonates it.
"""

from __future__ import annotations

import argparse
import csv
import json
import re
import struct
import sys
from collections import defaultdict
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import pefile  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"
FUNCTIONS_CSV = REPO_ROOT / "docs" / "recovery" / "functions.csv"
CALLGRAPH_JSON = REPO_ROOT / "docs" / "recovery" / "callgraph.json"
EXCLUSIONS_DOC = REPO_ROOT / "docs" / "EXCLUSIONS.md"

# The two handlers the frames in this image name. Everything else is a finding.
CXX_FRAME_HANDLER = 0x00644FD6
EXCEPT_HANDLER3 = 0x00646DF8

FS_ZERO_STORE_ESP = b"\x64\x89\x25\x00\x00\x00\x00"   # mov fs:[0], esp
PUSH_IMM32 = 0x68

# How far back from the store the handler push may sit. VC6 emits at most
# `push -1; push handler; mov eax, fs:[0]; push eax; mov eax, imm32` between
# them, which is 17 bytes; 40 is slack, and the search takes the LAST candidate
# so extra slack cannot pick up an earlier unrelated push.
HANDLER_PUSH_WINDOW = 40

PORT_IO_MNEMONICS = frozenset(
    ("in", "out", "insb", "insw", "insd", "outsb", "outsw", "outsd"))

# A dword window centred on a suspected jump table. Three entries pointing into
# the owning function is already past coincidence: a random dword lands inside a
# given function with probability under 1%.
JUMP_TABLE_WINDOW = 10
JUMP_TABLE_MIN_ENTRIES = 3

DIRECTX_DLLS = ("DDRAW.dll", "DSOUND.dll", "DPLAYX.dll", "MSVFW32.dll")

MEASURED_BLOCK = re.compile(r"```measured\n(.*?)```", re.DOTALL)


class Image:
    """Virtual-address reads over a loaded PE, and its section list.

    Wrapping pefile here is what lets the tests drive every classifier with a
    handful of synthetic bytes instead of a proprietary executable.
    """

    def __init__(self, chunks: list[tuple[int, bytes]], image_base: int = 0):
        self.chunks = sorted(chunks)
        self.image_base = image_base

    @classmethod
    def from_pe(cls, pe: pefile.PE) -> "Image":
        base = pe.OPTIONAL_HEADER.ImageBase
        return cls([(base + section.VirtualAddress, section.get_data())
                    for section in pe.sections], base)

    def read(self, address: int, length: int) -> bytes:
        for start, data in self.chunks:
            if start <= address < start + len(data):
                offset = address - start
                return data[offset:offset + length]
        return b""

    def dword(self, address: int) -> int | None:
        raw = self.read(address, 4)
        return struct.unpack("<I", raw)[0] if len(raw) == 4 else None


def body_spans(row: dict) -> list[tuple[int, int]]:
    """The half-open ranges a function's code occupies.

    `size` is the sum of these, not `end_address - address`: MSVC outlined cold
    blocks to `0x0065xxxx` and 402 functions carry a second span.
    """
    spans = []
    for span in (row.get("body_ranges") or "").split(";"):
        if "-" not in span:
            continue
        low, high = span.split("-")
        spans.append((int(low, 16), int(high, 16)))
    return spans or [(int(row["address"], 16), int(row["end_address"], 16))]


def load_rows(functions_csv: Path) -> list[dict]:
    with functions_csv.open() as handle:
        return list(csv.DictReader(handle))


def seh_registrations(image: Image) -> list[tuple[int, int]]:
    """Every `mov fs:[0], esp` paired with the handler pushed just before it.

    This is the census that means something. See the module docstring for the
    byte-pattern count that does not.
    """
    found = []
    for start, data in image.chunks:
        offset = data.find(FS_ZERO_STORE_ESP)
        while offset >= 0:
            window_start = max(0, offset - HANDLER_PUSH_WINDOW)
            window = data[window_start:offset]
            handler = None
            for index in range(len(window) - 5, -1, -1):
                if window[index] != PUSH_IMM32:
                    continue
                candidate = struct.unpack("<I", window[index + 1:index + 5])[0]
                if image.read(candidate, 1):
                    handler = candidate
                    break
            found.append((start + offset, handler))
            offset = data.find(FS_ZERO_STORE_ESP, offset + 1)
    found.sort()
    return found


def resolve_handler(image: Image, handler: int | None) -> int | None:
    """Follow VC6's per-function `__ehhandler` thunk to the real handler.

    Each thunk is `mov eax, <FuncInfo>; jmp __CxxFrameHandler`, so the pushed
    address is unique per function and only the jump target identifies the
    mechanism. 387 distinct thunks exist and none of them is shared.
    """
    if handler is None:
        return None
    body = image.read(handler, 10)
    if len(body) == 10 and body[0] == 0xB8 and body[5] == 0xE9:
        return (handler + 10 + struct.unpack("<i", body[6:10])[0]) & 0xFFFFFFFF
    return handler


def owner_index(rows: list[dict]) -> list[tuple[int, int, dict]]:
    spans = []
    for row in rows:
        for low, high in body_spans(row):
            spans.append((low, high, row))
    spans.sort(key=lambda item: (item[0], item[1]))
    return spans


def owner_of(spans: list[tuple[int, int, dict]], address: int) -> dict | None:
    low = 0
    high = len(spans)
    while low < high:
        middle = (low + high) // 2
        if spans[middle][0] <= address:
            low = middle + 1
        else:
            high = middle
    if low == 0:
        return None
    start, end, row = spans[low - 1]
    return row if address < end else None


def looks_like_jump_table(image: Image, address: int,
                          spans: list[tuple[int, int]]) -> bool:
    """True when the dwords around `address` read as a switch table.

    MSVC puts switch tables in `.text` immediately after the indirect jump that
    reads them, so a disassembler that walks straight through decodes the
    entries as instructions. The entries all point into the same function,
    which nothing in a real instruction stream does.
    """
    inside = lambda value: any(low <= value < high for low, high in spans)
    aligned = address & ~3
    entries = 0
    for step in range(-(JUMP_TABLE_WINDOW // 2), JUMP_TABLE_WINDOW // 2):
        value = image.dword(aligned + 4 * step)
        if value is not None and inside(value):
            entries += 1
    return entries >= JUMP_TABLE_MIN_ENTRIES


def port_io_bodies(image: Image, rows: list[dict], disassembler
                   ) -> tuple[list[dict], list[dict]]:
    """Functions containing a real port-I/O instruction, and the impostors."""
    real = []
    table_data = []
    for row in rows:
        spans = body_spans(row)
        code_hits = []
        data_hits = []
        for low, high in spans:
            body = image.read(low, high - low)
            for instruction in disassembler.disasm(body, low):
                if instruction.mnemonic not in PORT_IO_MNEMONICS:
                    continue
                target = (data_hits
                          if looks_like_jump_table(image, instruction.address,
                                                   spans)
                          else code_hits)
                target.append("%s %s" % (instruction.mnemonic,
                                         instruction.op_str))
        if code_hits:
            real.append({"row": row, "sites": code_hits})
        elif data_hits:
            table_data.append({"row": row, "sites": data_hits})
    return real, table_data


def import_slots(pe: pefile.PE, dll_names) -> set[int]:
    wanted = {name.lower() for name in dll_names}
    slots = set()
    for entry in getattr(pe, "DIRECTORY_ENTRY_IMPORT", []) or []:
        if entry.dll.decode().lower() not in wanted:
            continue
        for imported in entry.imports:
            slots.add(imported.address)
    return slots


def slot_referrers(image: Image, slots: set[int],
                   spans: list[tuple[int, int, dict]]) -> set[int]:
    """Functions holding a four-byte reference to one of the IAT slots.

    Deliberately a raw byte search rather than a decode: a reference is a
    reference whether it is reached by `call [slot]` or by loading the slot's
    address, and over-collecting here can only widen the closure.
    """
    seeds = set()
    for start, data in image.chunks:
        for slot in slots:
            pattern = struct.pack("<I", slot)
            offset = data.find(pattern)
            while offset >= 0:
                row = owner_of(spans, start + offset)
                if row is not None:
                    seeds.add(int(row["address"], 16))
                offset = data.find(pattern, offset + 1)
    return seeds


def transitive_callers(edges: list[dict], seeds: set[int]) -> set[int]:
    """Every function that can reach a seed along direct call edges.

    A LOWER bound, and known to be one: the call graph carries no indirect
    edges, and the DirectDraw surface is reached almost entirely by
    `call [vtable+n]` on a returned COM pointer.
    """
    callers = defaultdict(set)
    for edge in edges:
        callers[int(edge["target"], 16)].add(int(edge["source"], 16))
    reached = set(seeds)
    pending = list(seeds)
    while pending:
        current = pending.pop()
        for caller in callers[current]:
            if caller not in reached:
                reached.add(caller)
                pending.append(caller)
    return reached


def measure(exe: Path, functions_csv: Path, callgraph_json: Path) -> dict:
    import capstone

    pe = pefile.PE(str(exe))
    image = Image.from_pe(pe)
    rows = load_rows(functions_csv)
    spans = owner_index(rows)
    disassembler = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)

    external = [row for row in rows
                if row["recovery_state"] == "external_library"]
    measured = {
        "external_library.functions": len(external),
        "external_library.bytes": sum(int(row["size"]) for row in external),
        "external_library.underscore_names":
            sum(1 for row in external if row["name"].startswith("_")),
    }

    registrations = seh_registrations(image)
    kinds = defaultdict(list)
    for site, handler in registrations:
        kinds[resolve_handler(image, handler)].append(site)
    handler3_owners = {
        int(owner_of(spans, site)["address"], 16)
        for site in kinds.get(EXCEPT_HANDLER3, [])
        if owner_of(spans, site) is not None}
    handler3_game = [address for address in handler3_owners
                     if owner_of(spans, address)["binary_kind"] == "game"]
    measured.update({
        "seh.frames": len(registrations),
        "seh.cxx_frame_handler": len(kinds.get(CXX_FRAME_HANDLER, [])),
        "seh.except_handler3": len(kinds.get(EXCEPT_HANDLER3, [])),
        "seh.except_handler3_functions": len(handler3_owners),
        "seh.except_handler3_game_functions": len(handler3_game),
    })

    real, impostors = port_io_bodies(image, rows, disassembler)
    measured.update({
        "port_io.functions": len(real),
        "port_io.bytes": sum(int(item["row"]["size"]) for item in real),
        "port_io.jump_table_impostors": len(impostors),
    })

    seeds = slot_referrers(image, import_slots(pe, DIRECTX_DLLS), spans)
    with callgraph_json.open() as handle:
        callgraph = json.load(handle)
    reached = transitive_callers(callgraph["edges"], seeds)
    by_address = {int(row["address"], 16): row for row in rows}
    reached_bytes = sum(int(by_address[address]["size"]) for address in reached)
    catalogued_bytes = sum(int(row["size"]) for row in rows)
    measured.update({
        "directx.functions": len(reached),
        "directx.bytes": reached_bytes,
        "directx.percent_of_catalogued_bytes":
            round(100.0 * reached_bytes / catalogued_bytes, 1),
        # A list of addresses in the graph, and the reason every seam count in
        # this repository is a lower bound: none of these has a known target.
        "indirect_call_sites": len(callgraph["indirect_call_sites"]),
    })

    netdaemon = [row for row in rows if "NetDaemon" in row["name"]]
    carried = [row for row in netdaemon
               if row["recovery_state"] == "original_dependency"]
    measured.update({
        "netdaemon.functions": len(netdaemon),
        "netdaemon.bytes": sum(int(row["size"]) for row in netdaemon),
        "netdaemon.original_dependency_bytes":
            sum(int(row["size"]) for row in carried),
    })

    measured["_detail"] = {
        "except_handler3_owners": sorted(
            owner_of(spans, site)["name"]
            for site in kinds.get(EXCEPT_HANDLER3, [])
            if owner_of(spans, site) is not None),
        "other_handlers": {
            hex(handler): len(sites) for handler, sites in kinds.items()
            if handler not in (CXX_FRAME_HANDLER, EXCEPT_HANDLER3)},
        "port_io_functions": [item["row"]["name"] for item in real],
        "port_io_sites": {item["row"]["name"]: item["sites"] for item in real},
        "port_io_impostors": [item["row"]["name"] for item in impostors],
        "directx_seed_functions": sorted(
            by_address[address]["name"] for address in seeds),
    }
    return measured


def parse_measured_block(text: str) -> dict:
    """Read the ```measured fence out of the exclusions document."""
    match = MEASURED_BLOCK.search(text)
    if match is None:
        raise ValueError("no ```measured block found")
    declared = {}
    for line in match.group(1).splitlines():
        line = line.split("#", 1)[0].strip()
        if not line:
            continue
        key, _, value = line.partition("=")
        key = key.strip()
        value = value.strip()
        if not key or not _:
            raise ValueError("not a `key = value` line: %r" % line)
        declared[key] = float(value) if "." in value else int(value)
    return declared


def compare(measured: dict, declared: dict) -> list[str]:
    problems = []
    for key in sorted(set(declared) | set(measured) - {"_detail"}):
        if key not in declared:
            problems.append("%s: measured %s, absent from the document"
                            % (key, measured[key]))
        elif key not in measured:
            problems.append("%s: declared %s, no longer measured"
                            % (key, declared[key]))
        elif measured[key] != declared[key]:
            problems.append("%s: document says %s, image says %s"
                            % (key, declared[key], measured[key]))
    return problems


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--functions", type=Path, default=FUNCTIONS_CSV)
    parser.add_argument("--callgraph", type=Path, default=CALLGRAPH_JSON)
    parser.add_argument("--document", type=Path, default=EXCLUSIONS_DOC)
    parser.add_argument("--check", action="store_true",
                        help="fail when the document disagrees with the image")
    parser.add_argument("--detail", action="store_true",
                        help="also print the names behind each population")
    args = parser.parse_args(argv)

    if not args.exe.exists():
        # Reporting nothing is a fine answer to "measure this"; it is never an
        # answer to "does the document still agree with the image". Without the
        # separation --check exits 0 having compared nothing, and the exe is a
        # gitignored proprietary artifact so that is the ordinary case on any
        # checkout without the game. Latent today - nothing wires --check yet -
        # and fixed before it is wired rather than after.
        if args.check:
            print("measure-exclusions: the pinned executable is not present at "
                  "%s, so the document could not be compared against anything. "
                  "This check verified NOTHING." % args.exe, file=sys.stderr)
            return 2
        print("the pinned executable is not present at %s; nothing measured"
              % args.exe)
        return 0

    measured = measure(args.exe, args.functions, args.callgraph)
    detail = measured.pop("_detail")
    for key in sorted(measured):
        print("%-46s %s" % (key, measured[key]))
    if args.detail:
        print()
        print(json.dumps(detail, indent=2, sort_keys=True))

    if not args.check:
        return 0

    declared = parse_measured_block(args.document.read_text())
    problems = compare(measured, declared)
    if problems:
        print()
        print("%s disagrees with the image:" % args.document)
        for problem in problems:
            print("  " + problem)
        return 1
    print()
    print("%s agrees with the image on all %d measured figures"
          % (args.document, len(declared)))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
