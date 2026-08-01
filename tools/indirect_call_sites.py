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
from collections import defaultdict
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from generator_support import parse_body_ranges, read_bytes  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_FUNCTIONS = REPO_ROOT / "docs" / "recovery" / "functions.csv"
DEFAULT_CALLGRAPH = REPO_ROOT / "docs" / "recovery" / "callgraph.json"
DEFAULT_EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"

RESOLVED = {"source_complete", "external_library", "thunk"}


def load_image(exe_path):
    """The image and a body reader, both from the shared implementation.

    This used to carry its own section walk, bounded by Misc_VirtualSize alone.
    That is a TIGHTER bound than generator_support.read_bytes uses - it stops at
    the virtual size where a section's raw data can extend past it - and it
    silently returned b"" for 14,063 bytes across six sections. No catalogued
    body lives there, so nothing was wrong today; it was one more copy of a read
    that exists in eight places with three incompatible semantics, and this is
    the one place the difference was pure loss.
    """
    import capstone
    import pefile
    image = pefile.PE(str(exe_path))

    def body(start, end):
        return read_bytes(image, start, end - start)

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


def body_spans(row):
    """Every span of this function, falling back to the contiguous read.

    416 catalogued functions are split and for all 416 `end_address` stops
    short of the real body - the cold blocks were outlined to 0x0065xxxx, up to
    2,102 bytes away. Scanning only the first span answers "does this function
    dispatch indirectly?" over part of it, and a miss there is a false LEAF,
    which is the exact error this tool exists to prevent.
    """
    spans = parse_body_ranges(row.get("body_ranges", ""))
    if spans:
        return spans
    return [(int(row["address"], 16), int(row["end_address"], 16))]


def scan_row(body, decoder, row):
    found = []
    for start, end in body_spans(row):
        found.extend(indirect_sites(body, decoder, start, end))
    return found


def read_rows(functions_path):
    rows = {}
    with functions_path.open(newline="", encoding="utf-8-sig") as handle:
        for row in csv.DictReader(handle):
            rows[row["address"]] = row
    return rows


def frontier_population(rows, callgraph_path):
    """Unrecovered game functions whose callees are all already resolved.

    THE CALLGRAPH IS AN EDGE LIST, NOT AN ADDRESS-KEYED MAP. This function used
    to ask it for `graph[address]["callees"]`, a key shape the file has never
    had, so every lookup returned [] and `all([])` was vacuously true. The seam
    filter admitted every row it was given: it reported 2,786 functions, which
    is exactly the unfiltered `unrecovered and game` count, where the answer is
    949. Two published percentages were computed over 2.9x too many functions.

    tools/recovery_frontier.py:65-68 already joined this correctly. This is that
    code, and the lesson is the cheaper one: the correct implementation was in
    the tree and a second was written instead of imported.

    Addresses are compared as integers because the CSV and the callgraph do not
    agree on spelling, and a string join between them fails silently in exactly
    the same direction as the bug above - matching nothing, and calling it clean.
    """
    graph = json.loads(callgraph_path.read_text(encoding="utf-8"))
    if "edges" not in graph:
        raise SystemExit(
            f"{callgraph_path} has no 'edges' key. Without it every callee list "
            f"is empty, the seam filter admits every row, and this tool reports "
            f"a population it never actually filtered.")
    by_source = defaultdict(set)
    for edge in graph["edges"]:
        by_source[int(edge["source"], 16)].add(int(edge["target"], 16))
    state_of = {int(address, 16): row.get("recovery_state")
                for address, row in rows.items()}
    out = []
    for address, row in rows.items():
        if row["recovery_state"] != "unrecovered" or row["binary_kind"] != "game":
            continue
        callees = by_source.get(int(address, 16), ())
        if all(state_of.get(one) in RESOLVED for one in callees):
            out.append(row)
    if len(out) == len(state_of):
        raise SystemExit(
            "the seam filter admitted every row, which means it filtered "
            "nothing - refusing to publish a 'zero-callee-seam' population "
            "that is really the whole catalogue.")
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
            sites = scan_row(body, decoder, row)
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
        spans = body_spans(row)
        # The real size, summed over the spans - not end_address - address,
        # which understates every split body.
        size = sum(end - start for start, end in spans)
        if not any(body(start, end) for start, end in spans):
            continue
        bytes_total += size
        if scan_row(body, decoder, row):
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
