#!/usr/bin/env -S uv run python
"""One address, every place it appears - and the boundaries between them.

Three partial checks each guarded ONE boundary and the audit found every
boundary leaking into the next: `duplicated_markers` sees only
within-one-file pairs; `redundant_artifacts` sees only claimed-vs-claimed
across directories; `orphan_artifacts` sees one directory pair. None of them
could see an address annotated in TWO artifact files (10 such), a claim
contradicted by the PENDING_BODY forwarder the build actually links (21),
or a scaffold file nobody ratchets (81 markers in two files at the time).

This builds the whole map once - for every address, its marker(s) per file,
whether each is claimed, and whether pending_bodies.cpp also defines it -
and reports every crossing:

  DUPLICATE      the address has markers in more than one file
  CONTRADICTED   a claimed record somewhere AND a PENDING_BODY forwarder -
                 the build still calls through to the original, so the
                 claim lives outside everything the gate can see
  UNKNOWN HOST   a marker-hosting file that is neither a build input nor a
                 known scaffold - a hand-list leak, finding 5's shape

    tools/address_index.py            # report
    tools/address_index.py --check    # floors: counts may only fall

Floors start at today's measured counts and are ceilings in disguise: the
report is a defect list, so zero retires a floor in its own commit.
"""

from __future__ import annotations

import collections
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from decomp import read
from decomp.asm import build_inputs

REPO = Path(__file__).resolve().parent.parent
SRC = REPO / "src"
PENDING = SRC / "pending_bodies.cpp"

# Files whose markers are RATCHETED as scaffolds (compiler_work.py owns the
# ceilings); plus the two archive families. Anything else hosting markers
# outside the build is this check's third finding.
KNOWN_SCAFFOLDS = ("init_thunks.cpp", "atexit_thunks.cpp",
                   "adjustor_thunks.cpp", "deleting_thunks.cpp",
                   "delegation_thunks.cpp", "field_accessors.cpp",
                   "leaf_recoveries.cpp", "nullsub_thunks.cpp",
                   "guarded_teardowns.cpp")
ARCHIVES = ("src/recovered/", "src/unrecovered/")

PENDING_AT = re.compile(r"at\s+(0x[0-9A-Fa-f]{8})")

# Floors, measured 2026-08-23. Each may only fall.
FLOORS = {
    "duplicate": 4,
    "contradicted": 9,
    "unknown-host-files": 0,
}


def build():
    """(by_address, pending_defs, hosts) from one pass over the tree."""
    built = build_inputs(REPO / "build" / "compile_commands.json")
    by_address: dict = collections.defaultdict(list)
    for record in read(SRC):
        by_address[record.address].append(record)

    pending_defs: dict = set()
    pending_text = PENDING.read_text(errors="replace")
    for match in PENDING_AT.finditer(pending_text):
        pending_defs.add(int(match.group(1), 16))

    hosts: dict = collections.Counter()
    for path in sorted(SRC.glob("*.cpp")):
        rel = str(path.relative_to(REPO)).replace("\\", "/")
        if path.name in KNOWN_SCAFFOLDS:
            continue
        if rel.startswith("src/recovered") or rel.startswith("src/unrecovered"):
            continue
        if path.resolve() in built:
            continue  # product TUs host markers legitimately
        text = path.read_text(errors="replace")
        n = len(re.findall(r"^// ORIGINAL: 0x", text, re.M))
        if n:
            hosts[path.name] += n
    return by_address, pending_defs, hosts


def main() -> int:
    check = "--check" in sys.argv
    by_address, pending_defs, hosts = build()

    duplicates = []
    contradicted = []
    for address, records in sorted(by_address.items()):
        files = {r.path.name for r in records}
        if len(files) > 1:
            duplicates.append((address, records))
        if any(r.byte_exact or r.semantic for r in records) \
                and address in pending_defs:
            contradicted.append((address, records))

    print(f"{len(by_address)} distinct addresses indexed")
    for address, records in duplicates:
        places = ", ".join(f"{r.path.name}:{r.line}"
                           for r in sorted(records, key=lambda r: r.path.name))
        state = ",".join(sorted({"CLAIMED" if r.byte_exact else
                                 ("SEMANTIC" if r.semantic else "open")
                                 for r in records}))
        print(f"  DUPLICATE 0x{address:08X} [{state}]  {places}")
    for address, records in contradicted:
        where = sorted({r.path.name for r in records})[0]
        print(f"  CONTRADICTED 0x{address:08X}  claim in {where}, "
              f"PENDING_BODY defines it too")
    if hosts:
        for name, n in sorted(hosts.items()):
            print(f"  UNKNOWN HOST {name}: {n} marker(s)")

    print(f"\n{len(duplicates)} duplicate group(s), "
          f"{len(contradicted)} claim-vs-forwarder contradiction(s), "
          f"{len(hosts)} unknown host file(s)")

    if check:
        live = {"duplicate": len(duplicates),
                "contradicted": len(contradicted),
                "unknown-host-files": len(hosts)}
        grew = [k for k, v in live.items() if v > FLOORS[k]]
        shrank = [(k, v) for k, v in live.items() if v < FLOORS[k]]
        if grew:
            print(f"ADDRESS INDEX GREW: {', '.join(grew)} above their floors")
            return 1
        for k, v in shrank:
            print(f"address index down: {k} is {v}, below its floor of "
                  f"{FLOORS[k]} - lower it in this same commit")
        return 0
    return 0


if __name__ == "__main__":
    sys.exit(main())
