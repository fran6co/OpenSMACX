#!/usr/bin/env python3
"""Take the members PRACX has and Thinker does not.

`docs/EXCLUSIONS.md` 7a records that PRACX and Thinker share 26 structs under
identical member names - `CSprite`/`Sprite` 12 of 12, `CImage`/`Texture` 28 of
28 - so they are one lineage, and pairing them as two votes would be counting a
copy as corroboration.

THAT RULES PRACX OUT OF AGREEMENT, NOT OUT OF COVERAGE. The two uses are
different and only the first needs independence:

  * AGREEMENT decides a size, and `derive_agreed_sizes.py` needs its two
    sources to fail differently. PRACX is barred from that and always will be.
  * COVERAGE decides whether a member has a name at all. The IDA database and
    Thinker are both already used this way, side by side, because a name is
    checked downstream by objdiff rather than by a vote.

Where the two overlap PRACX is redundant. Where it does not, 19 of its 38
structs map confidently and carry 35 members Thinker never mentions, 17 of
them named.

THE NAME MAP IS THE DANGEROUS PART, so it is measured rather than assumed, and
the metric took three attempts - see `name_map` for what each one got wrong.
Whatever is refused is PRINTED with its score, because a pair this cannot
justify is a lead for a person rather than a thing to drop silently:
`CCanvas`/`Buffer` scores 0.53 on 32 shared member names and is very probably
right, and it is still not carried, because the threshold is not there to be
argued past one struct at a time.
"""

from __future__ import annotations

import argparse
import collections
import csv
import io
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import correlate_thinker_layouts as thinker  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
SOURCE = (REPO_ROOT / ".opensmacx" / "external-analysis" / "sources" /
          "pracx" / "shared" / "terran.h")
THINKER_MEMBERS = REPO_ROOT / "docs" / "recovery" / "thinker-members.csv"
OUTPUT = REPO_ROOT / "docs" / "recovery" / "pracx-members.csv"

STRUCT_RE = re.compile(r"^struct (?P<name>\w+)\s*$")
# PRACX writes Win32 spellings and `__int32`; the member is the last identifier
# before the semicolon.
#
# THE STAR BINDS TO THE NAME, NOT THE TYPE. `void *ppszFileName;` has no space
# between the `*` and the identifier, so a pattern demanding whitespace after
# the type matches nothing here - and PRACX declares most of its interesting
# members as pointers, so that silently dropped them and made the whole source
# look thin.
FIELD_RE = re.compile(
    r"^\s*(?P<type>[A-Za-z_]\w*)\s*(?P<stars>\*+)?\s*"
    r"(?P<name>\w+)\s*(?:\[(?P<count>\w+)\])?\s*;")
# A pair below this share too little of the smaller declaration to be the
# same type, and a pair sharing fewer names than MIN_SHARED is coincidence -
# two structs that both begin with a vtable pointer share one name and mean
# nothing by it.
CONFIDENT = 0.70
MIN_SHARED = 4


def parse(text: str) -> dict:
    """{struct: [(member, type, count)]} in declaration order."""
    found = {}
    for block in re.finditer(r"^struct (\w+)\s*\n\{(.*?)^\};", text,
                             re.S | re.M):
        members = []
        for line in block.group(2).splitlines():
            match = FIELD_RE.match(line)
            if match:
                members.append((
                    match.group("name"),
                    match.group("type") + (match.group("stars") or ""),
                    match.group("count")))
        if members:
            found[block.group(1)] = members
    return found


def thinker_members() -> dict:
    found = collections.defaultdict(list)
    if not THINKER_MEMBERS.is_file():
        return found
    with THINKER_MEMBERS.open(newline="", encoding="utf-8-sig") as handle:
        for row in csv.DictReader(handle):
            found[row["struct"]].append(row["field"])
    return found


def name_map(pracx: dict, twins: dict) -> tuple:
    """({pracx: our name}, [(pracx, best, score) that were refused]).

    Scored by CONTAINMENT - how much of the smaller struct the two share -
    rather than by count or by Jaccard, and both of the others were tried:

      * COUNT rewards a struct for being large. `CCity` and `CUnit` both
        matched `Caviar` on it, and `Caviar` is neither a city nor a unit.
      * JACCARD divides by the union, so it punishes the very case worth
        finding: a source that knows MORE members scores lower for it.
        `CSprite`/`Sprite` came out at 0.77 and was refused with 12 of its 12
        members matching, and `CWinBase`/`Win` at 0.52 with 66 shared.

    Containment says what fraction of the smaller declaration the larger one
    accounts for, so extra members on either side cost nothing. It still
    refuses the spurious pairs, because `CCity` shares 14 of its 76 members
    with `Caviar` and that is 0.18 however it is framed.
    """
    mapped, refused = {}, []
    for name, members in sorted(pracx.items()):
        mine = {member for member, _, _ in members}
        best, score = "", 0.0
        for other, fields in twins.items():
            theirs = set(fields)
            smaller = min(len(mine), len(theirs))
            if not smaller:
                continue
            shared = len(mine & theirs)
            if shared < MIN_SHARED:
                continue
            overlap = shared / smaller
            if overlap > score:
                best, score = other, overlap
        if score >= CONFIDENT:
            mapped[name] = best
        elif best:
            refused.append((name, best, score))
    return mapped, refused


def additions(pracx: dict, twins: dict, mapped: dict) -> list:
    """[(our class, member, type)] PRACX has that its twin does not."""
    rows = []
    for name, ours in sorted(mapped.items()):
        known = set(twins.get(ours, ()))
        for member, type_name, _ in pracx[name]:
            if member not in known:
                rows.append((ours, member, type_name))
    return rows


def render(rows: list) -> str:
    buffer = io.StringIO()
    writer = csv.writer(buffer, lineterminator="\n")
    writer.writerow(["class", "field", "type", "evidence"])
    for klass, member, type_name in sorted(rows):
        writer.writerow([klass, member, type_name,
                         "declared by PRACX and absent from Thinker"])
    return buffer.getvalue()


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--source", type=Path, default=SOURCE)
    parser.add_argument("--out", type=Path, default=OUTPUT)
    parser.add_argument("--check", action="store_true",
                        help="fail if the committed CSV is stale")
    args = parser.parse_args(argv)

    if not args.source.is_file():
        print(f"SKIP: {args.source} is absent; it is a fetched, ignored "
              f"external header.")
        return 0

    pracx = parse(args.source.read_text(errors="replace"))
    twins = thinker_members()
    mapped, refused = name_map(pracx, twins)
    rows = additions(pracx, twins, mapped)
    text = render(rows)

    if args.check:
        current = args.out.read_text() if args.out.is_file() else ""
        if current != text:
            print(f"{args.out.name} is stale; regenerate it", file=sys.stderr)
            return 1
        print(f"PRACX members: {len(rows)} beyond Thinker across "
              f"{len(mapped)} mapped struct(s) (up to date)")
        return 0

    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(text)
    print(f"{len(pracx)} PRACX struct(s); {len(mapped)} map confidently "
          f"-> {args.out}")
    print(f"    {len(rows)} member(s) PRACX has and Thinker does not")
    for name, best, score in sorted(refused, key=lambda row: -row[2])[:8]:
        print(f"    refused {name} -> {best} at {score:.2f}, below "
              f"{CONFIDENT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
