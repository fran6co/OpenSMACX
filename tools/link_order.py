#!/usr/bin/env -S uv run python
"""Report how well src/*.cpp lines up with the image's translation units.

The image's .text is laid out in LINK order, which is object-file order. That
is not an assumption - it is checked here every run, from the CRT dynamic
initialiser table (.CRT$XCU, 434 entries): those pointers are emitted in
object order, so if .text were not also in object order the table would
descend. It does not. If it ever does, this tool fails rather than reporting.

Given that, a translation unit is a CONTIGUOUS RANGE of addresses, so a source
file that is one TU claims one unbroken run of them. Files that claim several
runs are either several original TUs merged, or one TU split across our files.

Collector files (init_thunks.cpp and friends) gather one KIND of body from all
over the image, so they are anti-TU by construction and are treated as
transparent - they never break another file's run.
"""
import argparse, collections, pathlib, re, struct, sys

REPO = pathlib.Path(__file__).resolve().parent.parent
IMAGE = REPO / ".opensmacx/game/terranx_original.exe"
MARKER = re.compile(r"//\s*ORIGINAL:\s*(0x[0-9A-Fa-f]{8})")
COLLECTOR = re.compile(
    r"(_thunks|leaf_recoveries|field_accessors|guarded_teardowns|pending_bodies)")


def init_table(data):
    """The .CRT$XCU pointers, in link order. Located, not hardcoded."""
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    nsec = struct.unpack_from("<H", data, pe + 6)[0]
    opt = struct.unpack_from("<H", data, pe + 20)[0]
    base = struct.unpack_from("<I", data, pe + 24 + 28)[0]
    secs = []
    for i in range(nsec):
        o = pe + 24 + opt + 40 * i
        vsize, va, rsize, raw = struct.unpack_from("<IIII", data, o + 8)
        secs.append((data[o:o + 8].rstrip(b"\0").decode(), va, vsize, raw, rsize))
    text = next(s for s in secs if s[0] == ".text")
    lo, hi = base + text[1], base + text[1] + text[2]
    # __xc_a and __xc_z bracket the table with NULL dwords, and it lives in
    # .data. Both conditions matter: .rdata holds far longer runs of code
    # pointers (the vtables), and taking the longest run picks one of those.
    best = []
    for name, va, vsize, raw, rsize in secs:
        if name != ".data":
            continue
        end = min(vsize, rsize) - 3
        run, start = [], None
        for off in range(0, end, 4):
            value = struct.unpack_from("<I", data, raw + off)[0]
            if lo <= value < hi:
                if not run:
                    start = off
                run.append(value)
                continue
            if run and value == 0 and start >= 4 and \
                    struct.unpack_from("<I", data, raw + start - 4)[0] == 0 and \
                    len(run) > len(best):
                best = run
            run = []
    return best


def claims():
    owner = {}
    for path in sorted(REPO.joinpath("src").rglob("*")):
        if path.suffix not in (".cpp", ".h"):
            continue
        if "recovered" in path.parts or "unrecovered" in path.parts:
            continue
        rel = str(path.relative_to(REPO))
        for addr in MARKER.findall(path.read_text(errors="replace")):
            owner.setdefault(int(addr, 16), rel)
    return owner


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--check", action="store_true",
                    help="exit 1 if the link-order premise no longer holds")
    ap.add_argument("--worst", type=int, default=12)
    ap.add_argument("--image", type=pathlib.Path, default=IMAGE,
                    help="alternate image; used to positive-control --check")
    args = ap.parse_args()

    if not args.image.exists():
        print(f"SKIP: {args.image} absent")
        return 0

    table = init_table(args.image.read_bytes())
    descents = sum(1 for a, b in zip(table, table[1:]) if b < a)
    if descents:
        print(f"PREMISE BROKEN: CRT initialiser table descends {descents} times "
              f"in {len(table) - 1} steps; .text is NOT in link order and every "
              f"run count below is meaningless.")
        return 1
    print(f"premise holds: {len(table)} CRT initialisers, 0 descents in "
          f"{len(table) - 1} steps -> .text is in link order")
    if args.check:
        return 0

    owner = claims()
    collected = sum(1 for f in owner.values() if COLLECTOR.search(f))
    print(f"{len(owner)} claimed bodies; {collected} "
          f"({100 * collected / len(owner):.0f}%) in collector files (transparent)")

    seq = [(a, owner[a]) for a in sorted(owner) if not COLLECTOR.search(owner[a])]
    runs = collections.defaultdict(list)
    cur = [seq[0]]
    for addr, f in seq[1:]:
        if f == cur[-1][1]:
            cur.append((addr, f))
        else:
            runs[cur[0][1]].append(len(cur))
            cur = [(addr, f)]
    runs[cur[0][1]].append(len(cur))

    rows = sorted((max(r) / sum(r), len(r), max(r), sum(r), f)
                  for f, r in runs.items())
    single = sum(1 for r in rows if r[1] == 1)
    print(f"{single}/{len(rows)} game files are ONE unbroken run of the image")
    print(f"{sum(1 for r in rows if r[0] >= 0.8)}/{len(rows)} keep >=80% in their "
          f"biggest run; {sum(r[1] for r in rows)} runs total\n")
    print("least TU-like (share in biggest run, runs, biggest/bodies):")
    for share, nruns, big, total, f in rows[:args.worst]:
        print(f"  {share:4.0%} {nruns:3d} runs {big:4d}/{total:<4d} {f}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
