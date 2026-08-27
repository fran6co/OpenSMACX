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

# A FORWARDER IS A `PENDING_BODY(0xADDR, ...)` CALL, NOT THE WORDS "at 0x...".
# The old pattern scanned the whole file INCLUDING PROSE, so a comment reading
# "?X_pop@@... at 0x005BF480 - the two full bodies" (pending_bodies.cpp:439)
# was counted as a forwarder definition and reported as a live contradiction
# against src/xpops.cpp. It was wrong in the other direction too: it matched
# only the addresses that happen to be written after the word "at", so real
# forwarders whose comment is phrased differently were invisible. Match the
# macro invocation, which is the definition itself.
PENDING_AT = re.compile(r"PENDING_BODY\s*\(\s*(0x[0-9A-Fa-f]{8})")

# Floors, measured 2026-08-23. Each may only fall.
# TWO SHAPES, AND ONLY ONE OF THEM IS A FAULT. A claim whose body is
# COMPILED INTO THE BUILD, with a forwarder still routed to a raw image
# address, is a live landmine: any caller reaching the forwarder jumps into
# nothing and faults, though the real body is linked into the same binary.
# A claim that lives only in src/recovered/ or src/unrecovered/ is not that
# at all - those files are not build inputs, so the forwarder is the ONLY
# definition and deleting it breaks the link. Conflating the two is what
# made this floor read 9 and then be left alone: five of the nine could not
# be fixed the way the other three had to be, so nobody fixed any.
FLOORS = {
    # 1, not the 4 an earlier commit recorded: the four documented below are
    # down to the single live one - 0x00428550, CouncWin::~CouncWin - whose
    # defusal belongs to CouncWin's class pass (empty hypothesis-layout
    # destructor; see the long note above).
    "duplicate": 1,
    # TWO LIVE LANDMINES REMAIN, named so the number cannot become scenery:
    #   0x00428550 - claimed in BOTH 00428550.cpp and councwin.cpp (it is
    #     also a duplicate), so which definition the build links has to be
    #     settled before the forwarder can go.
    #   0x004C5BF0 - the byte-exact body is in wave_device.cpp but spelled as
    #     a free function; retiring the forwarder means giving it the member
    #     identity its caller already uses, which moves its symbol and must
    #     be re-measured. That belongs to the Wave pass, not to this one.
    # The two that WERE mechanical - 0x005F04E0 Win::close_class and
    # 0x005FECF0 Palette::close_palette_class - are gone, and jackal_close
    # calls the members directly now.
    # 1, not 2: 0x004C5BF0 was defused on 2026-08-25 by pointing
    # WaveGroupList::insert at `wave_group_insert_redirect`, the BYTE_EXACT
    # body already linked in wave_device.cpp, instead of jumping to the raw
    # image address beside it.
    # The one that remains is 0x00428550, CouncWin::~CouncWin, and it cannot
    # be defused the same way: its artifact destructor is EMPTY, and the
    # 312 bytes the image has are the compiler destroying members of a
    # hypothesis layout (a Spot, six Fonts, six FlatButtons). Homing it needs
    # councwin.h to declare that layout, which is CouncWin's class pass, not
    # a forwarder edit. A floor above 0 still means the gate passes on a live
    # fault - see P0.5 - so this stays a stated debt with an owner, not a
    # number nobody remembers choosing.
    # 0 as of the 2026-08-27 WIN pass: every reference pointing at a
    # no-longer-existing layout is gone, so nothing contradicts its host any
    # more; zero is where this floor now stands.
    "contradicted": 0,
    "archive-only-forwarder": 20,
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

    built = build_inputs(REPO / "build" / "compile_commands.json")
    duplicates = []
    contradicted = []
    archive_only = []
    for address, records in sorted(by_address.items()):
        files = {r.path.name for r in records}
        if len(files) > 1:
            duplicates.append((address, records))
        if any(r.byte_exact or r.semantic for r in records) \
                and address in pending_defs:
            # Is the claiming body actually COMPILED? That is the whole
            # difference between a fault and an archive waiting to be homed.
            if any(r.path.resolve() in built for r in records):
                contradicted.append((address, records))
            else:
                archive_only.append((address, records))

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
        print(f"  LANDMINE 0x{address:08X}  claim in {where} IS COMPILED, "
              f"and a PENDING_BODY forwarder still jumps to the raw address")
    for address, records in archive_only:
        where = sorted({r.path.name for r in records})[0]
        print(f"  archive-only 0x{address:08X}  claim in {where}, which the "
              f"build does not compile - the forwarder is the only definition")
    if hosts:
        for name, n in sorted(hosts.items()):
            print(f"  UNKNOWN HOST {name}: {n} marker(s)")

    print(f"\n{len(duplicates)} duplicate group(s), "
          f"{len(contradicted)} live landmine(s), "
          f"{len(archive_only)} archive-only forwarder(s), "
          f"{len(hosts)} unknown host file(s)")

    if check:
        live = {"duplicate": len(duplicates),
                "contradicted": len(contradicted),
                "archive-only-forwarder": len(archive_only),
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
