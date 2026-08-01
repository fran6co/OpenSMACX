#!/usr/bin/env python3
"""Report the call sites `docs/recovery/callgraph.json` cannot see.

The callgraph records direct `call rel32` edges. An indirect dispatch -
`call dword ptr [eax+0x64]`, `call [0x669338]`, `call ebx` - has no edge at
all, so a function that dispatches through a vtable slot or a member function
pointer reads as a PERFECT LEAF: `call_target_count` 0, every callee resolved,
apparently zero-seam.

Three were selected as leaves on exactly that basis before this existed:
`?hline@Buffer@@`, `?vline@Buffer@@` (both `call dword ptr [edx+0x64]`) and
`?fill@Buffer@@` (`call dword ptr [0x669338]`).

An indirect site is NOT automatically a seam - it may dispatch to a vtable the
recovery itself builds. It is a call site that must be LOOKED AT, and the
callgraph gives no reason to look.

    tools/indirect_call_sites.py --address 0x005E10C0     # one function
    tools/indirect_call_sites.py --frontier               # the population
"""
import argparse
import csv
import json
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_FUNCTIONS = REPO_ROOT / "docs" / "recovery" / "functions.csv"
DEFAULT_CALLGRAPH = REPO_ROOT / "docs" / "recovery" / "callgraph.json"
DEFAULT_EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"

RESOLVED = {"source_complete", "external_library", "thunk"}


def load_image(exe_path):
    import capstone
    import pefile
    image = pefile.PE(str(exe_path))
    base = image.OPTIONAL_HEADER.ImageBase

    def body(start, end):
        for section in image.sections:
            begin = base + section.VirtualAddress
            if begin <= start < begin + section.Misc_VirtualSize:
                offset = start - begin
                return section.get_data()[offset:offset + (end - start)]
        return b""

    return body, capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)


def indirect_sites(body, decoder, start, end):
    """Every call whose target is not an immediate address."""
    found = []
    data = body(start, end)
    if not data:
        return found
    for one in decoder.disasm(data, start):
        if one.mnemonic != "call":
            continue
        operand = one.op_str
        # A direct call is `call 0x4012a0`; anything else is computed.
        if operand.startswith("0x"):
            continue
        found.append((one.address, f"{one.mnemonic} {operand}"))
    return found


def read_rows(functions_path):
    rows = {}
    with functions_path.open(newline="", encoding="utf-8-sig") as handle:
        for row in csv.DictReader(handle):
            rows[row["address"]] = row
    return rows


def frontier_population(rows, callgraph_path):
    graph = json.loads(callgraph_path.read_text(encoding="utf-8"))
    out = []
    for address, row in rows.items():
        if row["recovery_state"] != "unrecovered" or row["binary_kind"] != "game":
            continue
        callees = (graph.get(address) or {}).get("callees", [])
        if all(rows.get(one, {}).get("recovery_state") in RESOLVED
               for one in callees):
            out.append(row)
    return out


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--address", action="append", default=[],
                        type=lambda v: int(v, 0))
    parser.add_argument("--frontier", action="store_true",
                        help="summarise the whole zero-callee-seam population")
    parser.add_argument("--functions", type=Path, default=DEFAULT_FUNCTIONS)
    parser.add_argument("--callgraph", type=Path, default=DEFAULT_CALLGRAPH)
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    arguments = parser.parse_args()

    if not arguments.address and not arguments.frontier:
        parser.error("give --address or --frontier")
    if not arguments.exe.is_file():
        print(f"executable absent: {arguments.exe}", file=sys.stderr)
        return 2

    rows = read_rows(arguments.functions)
    body, decoder = load_image(arguments.exe)

    if arguments.address:
        worst = 0
        for wanted in arguments.address:
            key = f"0x{wanted:08X}"
            row = rows.get(key) or rows.get(f"0x{wanted:08x}")
            if row is None:
                print(f"{key}: not in the catalogue", file=sys.stderr)
                worst = 2
                continue
            sites = indirect_sites(body, decoder, int(row["address"], 16),
                                   int(row["end_address"], 16))
            print(f"{row['address']} {row['name']}  "
                  f"callgraph callees {row.get('call_target_count', '?')}")
            if not sites:
                print("    no indirect call sites")
                continue
            worst = max(worst, 1)
            for address, text in sites:
                print(f"    0x{address:08X}  {text}")
        return worst

    population = frontier_population(rows, arguments.callgraph)
    with_indirect = without = 0
    bytes_with = bytes_total = 0
    for row in population:
        start, end = int(row["address"], 16), int(row["end_address"], 16)
        size = end - start
        if not body(start, end):
            continue
        bytes_total += size
        if indirect_sites(body, decoder, start, end):
            with_indirect += 1
            bytes_with += size
        else:
            without += 1
    total = with_indirect + without
    print(f"zero-callee-seam functions examined : {total}")
    print(f"  contain an indirect call site     : {with_indirect}"
          f"  ({bytes_with} B)")
    print(f"  direct calls only                 : {without}"
          f"  ({bytes_total - bytes_with} B)")
    if total:
        print(f"  share of functions                : "
              f"{100 * with_indirect / total:.1f}%")
        print(f"  share of bytes                    : "
              f"{100 * bytes_with / bytes_total:.1f}%")
    print("\nAn indirect site is a call site to LOOK AT, not automatically a "
          "seam.\nThe callgraph counts none of them, so its seam counts are a "
          "lower bound.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
