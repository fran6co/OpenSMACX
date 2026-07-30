#!/usr/bin/env python3
"""The recoverable frontier, the wave structure behind it, and the leaf queue.

WHY THIS METRIC AND NOT THE TWO OBVIOUS ONES.

Raw function count is gameable. 550 of the 2,808 unrecovered functions are under
64 bytes and total 19,350 B - about 1% of the unrecovered mass. "Recovered 550
functions" would be true and would move the project almost not at all.

Byte coverage is honest but it cannot be worked directly: 481 functions of
>=1024 B hold 73% of the unrecovered bytes, and most of them CANNOT be recovered
cleanly yet because their callees do not exist. `?enemy_move@@YAHH@Z`, the
largest at 56,408 B, calls 106 functions of which 33 are unrecovered - so
recovering it means 33 seams to the original, which is a different and worse
kind of debt.

So the number that says whether the project is being unblocked is the FRONTIER:
unrecovered functions whose callees are all resolved, and which can therefore be
recovered today with no seams. It GROWS as leaves land - each finished function
promotes its callers - and that growth is the evidence. A session that recovers
easy functions off the frontier without growing it has skimmed; a session that
grows it has moved the project.

WHAT "RESOLVED" MEANS. A callee is resolved if it is `source_complete`,
`external_library` or `thunk` - i.e. there is something to call. `unrecovered`,
`source_in_progress` and `original_dependency` are NOT resolved: the first has no
body, and the other two still lean on the original, so a caller built on them
inherits that lean.

CYCLES ARE A CEILING, NOT A DETAIL. Peeling waves only terminates if the graph
is layered. A mutually recursive cluster has no leaf, so no amount of
leaves-first work enters it and a seam is unavoidable there regardless of
ordering. This prints those clusters explicitly, because discovering them after
committing to a leaves-first plan is expensive.
"""

from __future__ import annotations

import argparse
import csv
import json
import sys
from collections import defaultdict
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_FUNCTIONS = REPO_ROOT / "docs" / "recovery" / "functions.csv"
DEFAULT_CALLGRAPH = REPO_ROOT / "docs" / "recovery" / "callgraph.json"

RESOLVED_STATES = frozenset({"source_complete", "external_library", "thunk"})


def load(functions_path: Path, callgraph_path: Path):
    inventory = {}
    with functions_path.open(newline="") as handle:
        for row in csv.DictReader(handle):
            inventory[int(row["address"], 16)] = {
                "size": int(row["size"]),
                "name": row["name"],
                "state": row["recovery_state"],
                "kind": row["binary_kind"],
                "callers": int(row["caller_count"] or 0),
                "targets": int(row["call_target_count"] or 0),
            }
    callees = defaultdict(set)
    graph = json.loads(callgraph_path.read_text())
    for edge in graph["edges"]:
        callees[int(edge["source"], 16)].add(int(edge["target"], 16))
    return inventory, callees


def waves(inventory, callees, limit=1000):
    """Peel functions whose callees are all resolved, repeatedly.

    Only `game` functions in a non-resolved state are candidates. Each pass is
    one wave: what could be recovered if every prior wave had landed.

    THE LIMIT IS DELIBERATELY HIGHER THAN ANY REAL DEPTH. An earlier version
    stopped at 8, which conflated the two reasons a function can still be
    blocked: it is deeper than the limit, or it sits in a cycle. Those demand
    opposite responses - wait versus cut a seam - so peeling runs to
    exhaustion. Whatever remains is then blocked for a STRUCTURAL reason, and
    cycles() can say so without a caveat.
    """
    pending = {a for a, f in inventory.items()
               if f["kind"] == "game" and f["state"] not in RESOLVED_STATES}
    out = []
    for _ in range(limit):
        ready = {a for a in pending
                 if not (callees.get(a, set()) & pending)}
        if not ready:
            break
        out.append(sorted(ready))
        pending -= ready
    return out, pending


def cycles(pending, callees):
    """Tarjan SCCs over the still-pending set. Any SCC of size > 1 is a knot.

    A self-loop counts too: a directly recursive function never becomes a leaf.
    """
    index = {}
    low = {}
    stack = []
    on_stack = set()
    found = []
    counter = [0]

    def strong(root):
        work = [(root, iter(sorted(callees.get(root, set()) & pending)))]
        index[root] = low[root] = counter[0]
        counter[0] += 1
        stack.append(root)
        on_stack.add(root)
        while work:
            node, children = work[-1]
            advanced = False
            for child in children:
                if child not in index:
                    index[child] = low[child] = counter[0]
                    counter[0] += 1
                    stack.append(child)
                    on_stack.add(child)
                    work.append(
                        (child, iter(sorted(callees.get(child, set()) & pending))))
                    advanced = True
                    break
                if child in on_stack:
                    low[node] = min(low[node], index[child])
            if advanced:
                continue
            work.pop()
            if work:
                low[work[-1][0]] = min(low[work[-1][0]], low[node])
            if low[node] == index[node]:
                component = []
                while True:
                    member = stack.pop()
                    on_stack.discard(member)
                    component.append(member)
                    if member == node:
                        break
                if len(component) > 1 or node in callees.get(node, set()):
                    found.append(sorted(component))

    for address in sorted(pending):
        if address not in index:
            strong(address)
    return found


def leaf_queue(inventory, callees):
    """Unrecovered game leaves, ranked callers DESC then size ASC.

    A leaf with no callers unblocks nothing - it is still listed, because it is
    still recoverable, but it sorts last and must not be counted as progress.
    """
    rows = [(a, f) for a, f in inventory.items()
            if f["kind"] == "game" and f["state"] == "unrecovered"
            and not callees.get(a)]
    rows.sort(key=lambda item: (-item[1]["callers"], item[1]["size"]))
    return rows


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--functions", type=Path, default=DEFAULT_FUNCTIONS)
    parser.add_argument("--callgraph", type=Path, default=DEFAULT_CALLGRAPH)
    parser.add_argument("--queue", type=int, default=20,
                        help="how many of the leaf queue to print")
    options = parser.parse_args(argv)

    inventory, callees = load(options.functions, options.callgraph)
    wave_list, pending = waves(inventory, callees)

    unrecovered = [(a, f) for a, f in inventory.items()
                   if f["kind"] == "game" and f["state"] == "unrecovered"]
    print(f"unrecovered game functions: {len(unrecovered)}, "
          f"{sum(f['size'] for _, f in unrecovered)} B")

    if wave_list:
        frontier = wave_list[0]
        print(f"\nRECOVERABLE FRONTIER: {len(frontier)} functions, "
              f"{sum(inventory[a]['size'] for a in frontier)} B")
        print("  (unrecovered, every callee resolved - no seams needed today)")
    else:
        print("\nRECOVERABLE FRONTIER: 0 - nothing is unblocked, see cycles below")

    print("\nwave structure:")
    for number, wave in enumerate(wave_list, start=1):
        print(f"  wave {number}: {len(wave):5d} fn  "
              f"{sum(inventory[a]['size'] for a in wave):8d} B")
    if pending:
        print(f"  still blocked after {len(wave_list)} waves: {len(pending)} fn  "
              f"{sum(inventory[a]['size'] for a in pending):8d} B")

    # Peeling ran to exhaustion, so `pending` is blocked structurally, not by
    # depth. Every member is in a cycle or reachable only through one.
    knots = cycles(pending, callees) if pending else []
    if knots:
        total = sum(len(k) for k in knots)
        print(f"\nCYCLES WITH NO LEAF: {len(knots)} cluster(s), {total} functions.")
        print("  These cannot be entered by leaves-first work at any depth; a seam")
        print("  is unavoidable in each. This is a CEILING on the strategy.")
        for knot in sorted(knots, key=len, reverse=True)[:5]:
            names = ", ".join(inventory[a]["name"][:28] for a in knot[:3])
            print(f"    {len(knot):4d} fn  {names}"
                  f"{' ...' if len(knot) > 3 else ''}")
    elif pending:
        # Peeling ran to exhaustion, so this should be unreachable: anything
        # still pending must sit in a cycle. If it prints, the SCC pass is
        # wrong, not the graph.
        print(f"\n{len(pending)} functions still blocked but NO cycle found - "
              "that is contradictory; suspect cycles(), not the data")
    else:
        print("\nno cycles: the graph is fully layered, and leaves-first "
              "reaches every function")

    queue = leaf_queue(inventory, callees)
    with_callers = [item for item in queue if item[1]["callers"] > 0]
    print(f"\nLEAF QUEUE: {len(queue)} leaves, "
          f"{sum(f['size'] for _, f in queue)} B; "
          f"{len(with_callers)} of them have callers")
    print("  ranked callers DESC, then size ASC. A zero-caller leaf unblocks")
    print("  nothing and does not count as progress.")
    for address, function in queue[:options.queue]:
        print(f"    0x{address:08X}  {function['size']:5d} B  "
              f"{function['callers']:3d} caller(s)  {function['name'][:44]}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
