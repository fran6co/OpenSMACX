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

WHERE THE CORRELATION LIVES. On the assertion, as `// yitzi 0x134`. This tool
used to write `docs/recovery/yitzi-sizes.csv` and then check that the file it
wrote still matched what it would write - a comparison that can only fail if
somebody hand-edits a generated file, and that says nothing about the tree. The
half with content was always the CONTROL: an independent human's prose agreeing
with a size `src/` compiles. So the agreement is recorded where the claim is,
and the check now runs three ways - the note, the marker, and the `static_assert`
the compiler enforces - so no two of them can drift apart silently.

The marker carries ONE claim: an outside source states this size. That is what
keeps it clear of the trap that got the last `static_assert` migration reverted,
where one marker was silently both an image-derived size and a curated trust
flag `class_layouts.pinned_layouts()` read as permission.

FACTION HAS NO MARKER, and that is the finding. Yitzi states 0x20CC; the Thinker
layouts say 0x470; `src/` declares no `Faction` at all, only the generated
hypothesis struct. Two independent sources disagreeing by 0x1C5C on a class this
tree cannot compile is not an agreement to record - it is an open question, so
the check prints it every run instead of pinning either number.
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import correlate_thinker_layouts as thinker  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
NOTES = (REPO_ROOT / ".opensmacx" / "external-analysis" / "sources" /
         "Information" / "Yitzi" / "variables.txt")

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


MARKER_RE = re.compile(
    r"static_assert\(\s*sizeof\(\s*(?P<klass>\w+)\s*\)\s*==\s*"
    r"(?P<size>0x[0-9A-Fa-f]+|\d+)\s*,?[^\n]*?//\s*yitzi\s+"
    r"(?P<note>0x[0-9A-Fa-f]+|\d+)")


def markers(root: Path = None) -> dict:
    """{class: (asserted, noted)} for every `// yitzi` marker in src/."""
    root = root or (REPO_ROOT / "src")
    found = {}
    for header in sorted(root.rglob("*.h")):
        for match in MARKER_RE.finditer(header.read_text(errors="replace")):
            found[match.group("klass")] = (int(match.group("size"), 0),
                                           int(match.group("note"), 0))
    return found


def correlate(notes: dict, marked: dict) -> tuple:
    """(agreed, wrong, unmarked) - the three ways note and tree can relate.

    `wrong` is any disagreement at all: the note against the marker, or the
    marker against the size the compiler enforces. Both are the same defect
    from the reader's side - a number in a comment that is not the number in
    the tree - so neither gets to be the quiet one.
    """
    agreed, wrong = [], []
    for klass, (asserted, noted) in sorted(marked.items()):
        stated = notes.get(klass)
        if stated is None:
            wrong.append((klass, f"marked `// yitzi 0x{noted:X}` but the notes "
                                 f"state no size for it"))
        elif noted != stated:
            wrong.append((klass, f"marked 0x{noted:X}, notes say 0x{stated:X}"))
        elif noted != asserted:
            wrong.append((klass, f"notes say 0x{noted:X}, static_assert pins "
                                 f"0x{asserted:X}"))
        else:
            agreed.append(klass)
    unmarked = sorted(set(notes) - set(marked))
    return agreed, wrong, unmarked


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--notes", type=Path, default=NOTES)
    parser.add_argument("--src", type=Path, default=None)
    parser.add_argument("--check", action="store_true",
                        help="fail if a marker disagrees with the notes or "
                             "with the size the compiler pins")
    args = parser.parse_args(argv)

    notes = sizes(args.notes)
    marked = markers(args.src)

    if not notes:
        # The notes are fetched and ignored, so a checkout without them cannot
        # confirm a marker. It can still catch the defect that does not need
        # them: a marker whose number contradicts its own static_assert.
        contradictory = [(k, f"marked 0x{n:X}, static_assert pins 0x{a:X}")
                         for k, (a, n) in sorted(marked.items()) if a != n]
        for klass, why in contradictory:
            print(f"    {klass}: {why}", file=sys.stderr)
        if contradictory:
            return 1
        print(f"SKIP: {args.notes} is absent; it is a fetched, ignored "
              f"external note. {len(marked)} marker(s) are self-consistent.")
        return 0

    agreed, wrong, unmarked = correlate(notes, marked)

    for klass, why in wrong:
        print(f"    {klass}: {why}", file=sys.stderr)
    for klass in unmarked:
        # Not a failure. `src/` does not declare every record Yitzi names, and
        # an unmarked row is an open question rather than a broken claim.
        print(f"    {klass}: Yitzi states 0x{notes[klass]:X}; src/ pins no "
              f"size for it, so nothing is claimed")

    print(f"Yitzi sizes: {len(agreed)} agreed / {len(wrong)} wrong / "
          f"{len(unmarked)} unmarked, against {len(notes)} stated")
    return 1 if wrong else 0


if __name__ == "__main__":
    raise SystemExit(main())
