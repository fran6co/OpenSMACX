#!/usr/bin/env -S uv run python
"""The WinMain-reachable call graph, in depth-first order, with verdicts.

WHY. The standing goal is to walk depth first from WinMain until everything
reachable is byte exact or semantically equivalent, and nothing in the tree
answers "what is reachable and not yet exact, nearest the root first". `check`
re-measures claims, `sweep` measures unclaimed bodies - both are flat lists
over the whole tree, so neither says which of 1,900 candidates is ON THE PATH.

The edges come from the shipped bytes, not from any annotation's `calls` fact,
which is a sorted set produced by a pass that no longer runs.

WHAT "REACHABLE" MEANS HERE. Direct and tail calls only. An indirect call
through a vtable or a bound slot is a runtime fact this cannot resolve, so a
node reached ONLY that way does not appear - the frontier is a lower bound on
the work, never an upper one, and the count is printed so that is visible.
"""

from __future__ import annotations

import json
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from decomp import read
from decomp.asm import build_inputs
from decomp.asm import original_asm, shared_spans, span_refusal
from decomp.calls import call_sites


REPO_ROOT = Path(__file__).resolve().parent.parent
IMAGE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"
ROOT = 0x0045F950  # _WinMain@16


def walk(records: list, exe: Path, root: int) -> tuple[list, int, int]:
    """Depth-first from `root`; returns (order, edges seen, unnamed edges)."""
    by_address = {}
    for record in records:
        by_address.setdefault(record.address, record)
    shared = shared_spans(records)

    order, seen, unnamed, edges = [], set(), 0, 0
    stack = [root]
    while stack:
        address = stack.pop()
        if address in seen:
            continue
        seen.add(address)
        record = by_address.get(address)
        if record is None:
            unnamed += 1
            continue
        order.append(record)
        if span_refusal(record, exe, shared) is not None:
            continue
        try:
            listing = original_asm(record, exe)
        except (ValueError, KeyError):
            continue
        targets = []
        for site in call_sites(listing):
            edges += 1
            if site.form in ("direct", "tail") and site.target is not None:
                targets.append(site.target)
        # Reversed, so `pop` takes the FIRST call the image makes: the order
        # of the walk is the order of the code.
        stack.extend(reversed(targets))
    return order, edges, unnamed


if __name__ == "__main__":
    records = read(REPO_ROOT / "src")
    order, edges, unnamed = walk(records, IMAGE, ROOT)
    built = build_inputs(REPO_ROOT / "build" / "compile_commands.json")
    pending = [r for r in order if not r.byte_exact]
    if "--all" not in sys.argv:
        # A record whose file the build does not compile cannot be MEASURED,
        # so it is not work anyone can finish today - it is work that first
        # needs a home in a translation unit.
        pending = [r for r in pending if r.path in built]
    if "--json" in sys.argv:
        print(json.dumps([{"address": r.address_hex, "name": r.name,
                           "file": str(r.path)} for r in pending], indent=2))
        sys.exit(0)
    for depth, record in enumerate(pending):
        print(f"{depth:4d}  {record.address_hex}  {record.path.name:24s} "
              f"{record.name}")
    print(f"{len(order):,} reachable and catalogued, {len(pending):,} not yet "
          f"byte exact; {unnamed:,} direct edges name nothing in the catalogue "
          f"({edges:,} edges walked, indirect ones not followed)")
    print("  --all also lists reachable bodies the build does not compile")
