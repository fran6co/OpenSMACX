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

`recovery_state` IS NOT THE WHOLE TRUTH, AND THIS JOINS AGAINST `src/`.
`functions.csv` records what the catalogue exporter knew, and it does not know
about `src/recovered/` - the byte-exact proof units, which carry a `BYTE_EXACT`
annotation and no `source_locations`. Measured 2026-08-12: **252 of the 563
leaves this used to print were already proved**, and `0x00401000` sat fifth in
the queue while `src/recovered/00401000.cpp:1` had a BYTE_EXACT claim on it. A
queue that hands an agent finished work is worse than a short one, so every
population here is filtered by `annotation_scan` unless `--include-claimed`.

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


def load(functions_path: Path = None, callgraph_path: Path = None):
    """The inventory and the call edges, both read out of `src/`.

    `functions.csv` and `callgraph.json` are gone: every annotation carries its
    own facts and its own `calls` line, so the frontier is computed from the
    same store the ratchet measures rather than from an export that could drift
    from it. The arguments remain so a caller can point this at a regenerated
    export, which is how the projection is re-verified.
    """
    sys.path.insert(0, str(Path(__file__).resolve().parent))
    import project_catalogue
    rows = (project_catalogue.catalogue(functions_path) if functions_path
            else project_catalogue.from_source())
    # `recovery_state` was never a fact ABOUT a function - the exporter derived
    # it from `src/` and wrote it into the CSV. With `src/` as the store it is
    # read straight off the map instead, which also removes the round trip that
    # let the two disagree: an address the catalogue called `unrecovered` while
    # `src/` held a BYTE_EXACT proof for it, 651 of them at the last count.
    import annotation_scan
    states = {}
    for annotation in annotation_scan.scan_tree(
            Path(__file__).resolve().parent.parent / "src"):
        if annotation.state == annotation_scan.STATE_EXCLUDED:
            states[annotation.address] = "external_library"
        elif annotation.state == annotation_scan.STATE_IMPLEMENTED:
            states[annotation.address] = "source_complete"
        else:
            states.setdefault(annotation.address, "unrecovered")

    inventory, callees = {}, defaultdict(set)
    for address, row in rows.items():
        callees[address] = set(row.get("_calls") or ())
        inventory[address] = {
            "size": int(row.get("size") or 0),
            "name": row.get("name", ""),
            "state": states.get(address, "unrecovered"),
            "kind": row.get("binary_kind", "game"),
            "callers": int(row.get("caller_count") or 0),
            "targets": int(row.get("call_target_count") or 0),
        }
    return inventory, callees


def _legacy_load(functions_path: Path, callgraph_path: Path):
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


def fresh_set(fresh: list) -> set:
    return {address for address, _ in fresh}


def source_states(src: Path = None) -> tuple:
    """(claimed, excluded, implemented) as `src/` actually reports them.

    Read through `annotation_scan`, the tree's own map reader, so this sees
    `src/recovered/` and `src/recovered/units/` exactly as the ratchet does
    rather than through `source_locations` - which stored bodies deliberately do
    not set.

    THREE POPULATIONS, AND THE FIRST RUN OF THE SUBAGENT LOOP FOUND WHY IT NEEDS
    ALL THREE. Filtering only on BYTE_EXACT left a queue of 319 leaves in which
    41 were `EXCLUDED` - CRT bodies and EH funclets the tree has explicitly
    ruled out, offered to an agent as work - and 66 already carried a
    non-matching body under `src/recovered/units/`, so `--work` refused them
    with "only claims placeholders" after they had been handed out. Those 66 are
    real work, but they are IMPROVE work and need the incumbent body in the
    brief; the 41 are not work at all.
    """
    sys.path.insert(0, str(Path(__file__).resolve().parent))
    import annotation_scan
    root = src or (Path(__file__).resolve().parent.parent / "src")
    if not root.is_dir():
        return set(), set(), set()
    claimed, excluded, implemented = set(), set(), set()
    for a in annotation_scan.scan_tree(root):
        if a.matched:
            claimed.add(a.address)
        if a.state == annotation_scan.STATE_EXCLUDED:
            excluded.add(a.address)
        elif a.state == annotation_scan.STATE_IMPLEMENTED:
            implemented.add(a.address)
    return claimed, excluded, implemented


def claimed_addresses(src: Path = None) -> set:
    """Just the BYTE_EXACT set - kept because it reads clearly at call sites."""
    return source_states(src)[0]


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    # DEFAULT None: `src/` is the store, and passing a path forces the export
    # reader, which is how this kept opening a file that no longer exists.
    parser.add_argument("--functions", type=Path, default=None)
    parser.add_argument("--callgraph", type=Path, default=None)
    parser.add_argument("--queue", type=int, default=20,
                        help="how many of the leaf queue to print")
    parser.add_argument("--max-size", type=int, default=0,
                        help="only queue leaves at or below this many bytes")
    parser.add_argument("--fresh-only", action="store_true",
                        help="only leaves with NO body yet, so "
                             "`decomp_status.py --work` can claim them")
    parser.add_argument("--include-claimed", action="store_true",
                        help="do not filter out addresses src/ already proves "
                             "(the pre-2026-08-12 behaviour)")
    options = parser.parse_args(argv)

    inventory, callees = load(options.functions, options.callgraph)
    claimed, excluded, implemented = (
        (set(), set(), set()) if options.include_claimed else source_states())
    if claimed or excluded:
        hidden = [a for a in inventory
                  if a in claimed and inventory[a]["state"] == "unrecovered"]
        print(f"src/ already proves {len(claimed)} addresses; "
              f"{len(hidden)} of them are still `unrecovered` in the catalogue "
              f"and are filtered out below")
        for address in hidden:
            inventory[address]["state"] = "source_complete"
        # EXCLUDED is a DECISION (docs/DECOMP_MAP.md), and the one state this
        # project declares rather than measures. Offering one as work asks an
        # agent to undo it by accident.
        dropped = [a for a in inventory if a in excluded]
        for address in dropped:
            inventory.pop(address)
        if dropped:
            print(f"dropped {len(dropped)} address(es) src/ marks EXCLUDED - "
                  f"a decision, not a gap")
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
    if options.max_size:
        queue = [item for item in queue if item[1]["size"] <= options.max_size]
    fresh = [item for item in queue if item[0] not in implemented]
    if options.fresh_only:
        queue = fresh
    with_callers = [item for item in queue if item[1]["callers"] > 0]
    print(f"\nLEAF QUEUE: {len(queue)} leaves, "
          f"{sum(f['size'] for _, f in queue)} B; "
          f"{len(with_callers)} of them have callers")
    print("  ranked callers DESC, then size ASC. A zero-caller leaf unblocks")
    print("  nothing and does not count as progress.")
    if not options.fresh_only and len(fresh) != len(queue):
        print(f"  {len(queue) - len(fresh)} of them already carry a "
              f"non-matching body - that is IMPROVE work, and "
              f"`--work` refuses it. Use --fresh-only to exclude them.")
    for address, function in queue[:options.queue]:
        mark = " " if address in fresh_set(fresh) else "*"
        print(f"  {mark} 0x{address:08X}  {function['size']:5d} B  "
              f"{function['callers']:3d} caller(s)  {function['name'][:44]}")
    if not options.fresh_only and len(fresh) != len(queue):
        print("  (* already has a body: improve it, do not start it)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
