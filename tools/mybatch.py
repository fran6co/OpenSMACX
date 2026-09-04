#!/usr/bin/env -S uv run python
"""Scratch: list the archive bodies routed to a given set of product TUs."""
import sys, collections
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
sys.path.insert(0, str(Path(__file__).resolve().parent))

from decomp import read
import home_bodies as hb

SRC = Path(__file__).resolve().parent.parent / "src"
records = read(SRC)
MINE = {"net_class.cpp", "netdaemon.cpp"}

flat = []
for r in records:
    p = str(r.path)
    if "recovered" in p or "unrecovered" in p:
        continue
    if r.path.name in hb.NEVER_HOME:
        continue
    flat.append((r.address, r.path.name))
flat.sort()
addrs = [a for a, _ in flat]
names = [n for _, n in flat]

tu_cache = {}
rows = collections.defaultdict(list)
for r in records:
    p = str(r.path)
    if "recovered" not in p and "unrecovered" not in p:
        continue
    klass = hb.owner_of(r)
    claim = "CLAIM" if (r.byte_exact or r.semantic) else "free"
    if klass:
        if klass not in tu_cache:
            tu_cache[klass] = hb.product_tu(klass, records)
        tu = tu_cache[klass]
        if tu is None:
            continue
        name = tu.name
    else:
        name = hb.nearest_product(r.address, addrs, names)
    if name in MINE:
        rows[name].append((r.address, claim, klass or "", r.name or "",
                           r.path.name))

for name in sorted(rows):
    print("=====", name, len(rows[name]))
    for a, c, k, n, f in sorted(rows[name]):
        print(f"  0x{a:08X} {c:5s} {k:22s} {n}  {f}")
