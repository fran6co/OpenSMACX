#!/usr/bin/env python3
"""Read the struct sizes out of Yitzi's notes.

`.opensmacx/external-analysis/sources/Information/Yitzi/variables.txt` has been
fetched and catalogued since the external-analysis pipeline was built, and
nothing has ever read it. It is a human's working notes on the same binary -
absolute addresses of globals, and, in a handful of places, the SIZE of a
record:

    Factions: Size 20CC.  Faction 0 would have:
    Bases: Size is 134, begins at 97D040
    Units: At 952828, size 34
    Facilities: Size 30, facility 0 would have cost at 9A4B74

WHY THIS ONE IS WORTH READING when PRACX was not. `docs/EXCLUSIONS.md` 7a
records that PRACX and Thinker share 26 structs with identical member names, so
pairing them would count one source twice. Yitzi's notes are a different
lineage entirely - prose written while patching the game, not a header - and
that independence is the whole value. `derive_agreed_sizes.py` admits a size
when two sources that fail differently land on the same number.

It is a small source and its plural English names are not the tree's class
names, so the mapping is written out by hand below rather than guessed at.
Every size is confirmed by compiling `static_assert(sizeof(X) == N)` before it
is believed, which is what makes a hand-written mapping safe: a wrong pairing
fails the build instead of pinning the wrong class.
"""

from __future__ import annotations

import argparse
import csv
import io
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import correlate_thinker_layouts as thinker  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
NOTES = (REPO_ROOT / ".opensmacx" / "external-analysis" / "sources" /
         "Information" / "Yitzi" / "variables.txt")
OUTPUT = REPO_ROOT / "docs" / "recovery" / "yitzi-sizes.csv"

# What Yitzi calls a record, and what this tree calls the class. Written out
# because the notes use plural English ("Bases", "Unit types") and a guess here
# would pin the wrong class - which the compile check would catch, but as a
# build failure rather than as a decision anybody made.
RECORD_CLASS = {
    "Bases": "Base",
    "Units": "Veh",
    "Unit types": "VehPrototype",
    "Facilities": "RulesFacility",
    "Factions": "Faction",
}

SIZE_RE = re.compile(
    r"^(?P<record>[A-Za-z][A-Za-z ]*?)\s*:.*?\bsize (?:is )?(?P<size>[0-9A-Fa-f]+)\b",
    re.IGNORECASE)


def sizes(path: Path = NOTES) -> dict:
    """{class: size} for every record the notes state a size for."""
    found = {}
    if not path.is_file():
        return found
    for line in path.read_text(errors="replace").splitlines():
        match = SIZE_RE.match(line.strip())
        if not match:
            continue
        record = match.group("record").strip()
        klass = RECORD_CLASS.get(record)
        if not klass:
            continue
        try:
            found[klass] = int(match.group("size"), 16)
        except ValueError:
            continue
    return found


def render(found: dict) -> str:
    buffer = io.StringIO()
    writer = csv.writer(buffer, lineterminator="\n")
    writer.writerow(["class", "size", "evidence"])
    for name in sorted(found):
        writer.writerow([name, f"0x{found[name]:X}",
                         "stated in Yitzi's variables.txt"])
    return buffer.getvalue()


def control(found: dict) -> tuple:
    """(right, wrong) against the classes `src/` already pins."""
    pinned = thinker.pinned_sizes()
    right = [n for n in found if n in pinned and found[n] == pinned[n]]
    wrong = [(n, found[n], pinned[n]) for n in found
             if n in pinned and found[n] != pinned[n]]
    return right, wrong


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--notes", type=Path, default=NOTES)
    parser.add_argument("--out", type=Path, default=OUTPUT)
    parser.add_argument("--check", action="store_true",
                        help="fail if the committed CSV is stale")
    args = parser.parse_args(argv)

    found = sizes(args.notes)
    if not found:
        print(f"SKIP: {args.notes} is absent; it is a fetched, ignored "
              f"external note.")
        return 0
    text = render(found)
    right, wrong = control(found)

    if args.check:
        current = args.out.read_text() if args.out.is_file() else ""
        if current != text:
            print(f"{args.out.name} is stale; regenerate it", file=sys.stderr)
            return 1
        if wrong:
            for name, claimed, true in wrong:
                print(f"    {name}: Yitzi says 0x{claimed:X}, src/ pins "
                      f"0x{true:X}", file=sys.stderr)
            return 1
        print(f"Yitzi sizes: {len(found)} class(es), control {len(right)} "
              f"right / 0 wrong (up to date)")
        return 0

    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(text)
    print(f"{len(found)} class size(s) -> {args.out}")
    print(f"    control against the pinned sizes: {len(right)} right, "
          f"{len(wrong)} wrong")
    for name, claimed, true in wrong:
        print(f"        WRONG {name}: 0x{claimed:X} against 0x{true:X}")
    return 1 if wrong else 0


if __name__ == "__main__":
    raise SystemExit(main())
