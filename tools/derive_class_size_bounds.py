#!/usr/bin/env python3
"""Publish an UPPER bound on each class's size, for staging oracle receivers.

The generated-signature route was gated on classes having a `sizeof` pinned by a
static_assert, and only 38 do. That gate asks for more than the oracle needs.
A pinned size is a claim about the RECOVERED class's layout; what staging needs
is a buffer the method cannot run off the end of, and the receiver is a fresh
`static uint8_t staged[ObjectSize]` rather than the real global - so
OVER-allocating is harmless and only UNDER-allocating corrupts.

Upper bounds are therefore usable where exact sizes are not, and they are the
safe direction by construction: an object cannot extend past the next
constructed global, because that global is there.

WHY A COMMITTED CSV. Deriving this reads the hash-bound executable, which is
uncommitted, so the generator cannot do it - `--check` runs in the test suite on
machines that have no copy. Same reasoning as every other catalogue under
docs/recovery: derive locally, publish addresses and integers, and let the
consumer read the file.

WHAT THIS IS NOT. A bound is not a layout. Nothing here may be turned into a
`static_assert`, and the generator records `size bounded` beside every oracle
built on one so the distinction survives into the generated source. Eight
mechanisms were tried for EXACT sizes and all failed their controls; this one
succeeds precisely because it answers a weaker question.

Control, replayed by --check: on the 40 classes whose size is pinned, the bound
was at or above the true size 13 times and BELOW it 0 times. Below is the only
direction that matters, and a single instance of it would retire the idea.
"""

from __future__ import annotations

import argparse
import csv
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import derive_class_layout as layout  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
BOUNDS_CSV = REPO_ROOT / "docs" / "recovery" / "class-size-bounds.csv"
FIELDS = ("class", "upper_bound", "evidence")

# Above this, three staged buffers cost more BSS than the oracle is worth;
# MainInterface's bound is 0x25098 and the suite would carry 456 KB for that one
# function. Recorded as a refusal rather than silently dropped.
MAX_STAGEABLE = 0x8000


def derive(exe: Path):
    """class -> (bound, evidence), for every class with a usable upper bound."""
    image = layout.Image(exe)
    by_address, _ = layout.load_functions()
    globals_ = layout.global_objects(image, by_address)
    pinned = layout.load_pinned()
    found = {}
    for name in sorted({n for n in _class_names(by_address)}):
        if name in pinned:
            continue          # a pinned size is better than a bound
        for ctor in layout.constructors(by_address, name):
            bound = layout.global_bound(image, ctor, globals_)
            if bound and 4 <= bound.size:
                found[name] = (bound.size, bound.detail)
                break
    return found, pinned


def _class_names(by_address):
    for row in by_address.values():
        found = layout.CTOR_RE.match(row.get("name", "") or "")
        if found:
            yield found.group(1)


def control(exe: Path) -> tuple[int, int]:
    """(at-or-above, BELOW) on the classes whose true size is known."""
    image = layout.Image(exe)
    by_address, _ = layout.load_functions()
    globals_ = layout.global_objects(image, by_address)
    pinned = layout.load_pinned()
    over = under = 0
    for name, truth in pinned.items():
        for ctor in layout.constructors(by_address, name):
            bound = layout.global_bound(image, ctor, globals_)
            if not bound:
                continue
            if bound.size < truth:
                under += 1
            else:
                over += 1
            break
    return over, under


def write(path: Path, found: dict) -> int:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", newline="", encoding="utf-8") as handle:
        # LF, because every other committed catalogue is LF and csv defaults to
        # CRLF; that mismatch has already cost one --check failure here.
        writer = csv.writer(handle, lineterminator="\n")
        writer.writerow(FIELDS)
        for name in sorted(found):
            size, evidence = found[name]
            writer.writerow([name, f"0x{size:X}", evidence])
    return len(found)


def load(path: Path = BOUNDS_CSV) -> dict:
    """class -> bound. Absent file is not an error.

    It fed the retired signature-oracle generator, which is why the import at
    the top of this file outlived the tool by one commit and failed the gate
    with ModuleNotFoundError rather than anything about a bound."""
    if not path.is_file():
        return {}
    with path.open(newline="", encoding="utf-8-sig") as handle:
        out = {}
        for row in csv.DictReader(handle):
            try:
                size = int(row["upper_bound"], 0)
            except (KeyError, ValueError):
                continue
            if 4 <= size <= MAX_STAGEABLE:
                out[row["class"]] = size
        return out


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--exe", type=Path, default=layout.DEFAULT_EXE)
    parser.add_argument("--out", type=Path, default=BOUNDS_CSV)
    parser.add_argument("--check", action="store_true",
                        help="fail if the committed file would change")
    args = parser.parse_args(argv)

    over, under = control(args.exe)
    print(f"control on pinned classes: bound at or above the true size {over}, "
          f"BELOW it {under}")
    if under:
        print("REFUSED: a bound below the true size would under-allocate a "
              "receiver, which is the one direction that corrupts memory.",
              file=sys.stderr)
        return 1

    found, pinned = derive(args.exe)
    stageable = {k: v for k, v in found.items() if v[0] <= MAX_STAGEABLE}
    print(f"{len(found)} class(es) with an upper bound; "
          f"{len(stageable)} within the {MAX_STAGEABLE:#x} staging cap")
    too_big = sorted(k for k in found if k not in stageable)
    if too_big:
        # Named, not silently dropped: a reader comparing counts should be able
        # to see where the difference went.
        print(f"{len(too_big)} refused as too large to stage: "
              + ", ".join(too_big[:8]) + (" ..." if len(too_big) > 8 else ""))

    if args.check:
        current = args.out.read_text(encoding="utf-8") if args.out.is_file() else ""
        import io
        buffer = io.StringIO()
        writer = csv.writer(buffer, lineterminator="\n")
        writer.writerow(FIELDS)
        for name in sorted(found):
            size, evidence = found[name]
            writer.writerow([name, f"0x{size:X}", evidence])
        if buffer.getvalue() != current:
            print(f"{args.out} is out of date", file=sys.stderr)
            return 1
        print(f"{args.out} is current")
        return 0

    print(f"wrote {write(args.out, found)} row(s) -> {args.out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
