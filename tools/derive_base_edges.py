#!/usr/bin/env python3
"""Which classes derive from which, stated by one source and checked by another.

Inheritance has been the least-recovered thing about these classes. `src/`
declares a base for 39 of 239, the image emits a destructor family for over a
hundred, and the layouts have had to be written flat - a derived class's base
subobject showing up as an anonymous run of `field_<hex>_` members with nothing
saying what those bytes are.

Thinker states the edges outright: `struct BasePop : GraphicWin`. Sixteen of
them, and they cost nothing to read once the parser stopped dropping derived
structs. But a stated edge is a claim, and `AGENTS.md` records what happens to
an unchecked one - `FileWin`, `StatusWin`, `InfoWin` and `BattleWin` were each
declared as deriving from the class their constructor calls into, when in every
case it is a MEMBER at a nonzero offset (`FlatButton` at 0x660, `Caviar` at
0x30, `Time` at 0x30). Declaring a member as a base puts it at offset zero and
moves every field after it.

So every edge is checked against the image, using the discriminator that
already exists: `derive_class_layout.first_constructed()`. A base is
constructed on an unadjusted `this`; a member is constructed on `this` plus its
offset, and the `lea ecx, [reg + N]` in between is what tells them apart.

Measured on the first run: **10 agree, 0 disagree, 6 silent.** The six are
silent because no constructor for them is catalogued under that name, which is
an absence of evidence and recorded as one.
"""

from __future__ import annotations

import argparse
import csv
import io
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import correlate_thinker_layouts as thinker  # noqa: E402
import derive_class_layout  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"
THINKER_SOURCES = (REPO_ROOT / ".opensmacx" / "external-analysis" /
                   "sources" / "thinker" / "src")
OUTPUT = REPO_ROOT / "docs" / "recovery" / "base-edges.csv"

AGREED = "agreed"
UNCHECKED = "unchecked"


def claimed_edges() -> dict:
    """{derived: base} as Thinker's headers declare it."""
    found = {}
    if not THINKER_SOURCES.is_dir():
        return found
    for header in sorted(THINKER_SOURCES.glob("*.h")):
        for line in header.read_text(errors="ignore").splitlines():
            match = thinker.STRUCT_RE.match(line)
            if match and match.group("base"):
                found[match.group("name")] = match.group("base")
    return found


def check(edges: dict, exe: Path) -> list:
    """[(derived, base, verdict, evidence)] - one row per claimed edge."""
    image = derive_class_layout.Image(exe)
    by_address, _ = derive_class_layout.load_functions()
    rows = []
    for derived, base in sorted(edges.items()):
        constructors = derive_class_layout.constructors(by_address, derived)
        if not constructors:
            rows.append((derived, base, UNCHECKED,
                         "no constructor catalogued for this class"))
            continue
        address = constructors[0]
        built = derive_class_layout.first_constructed(
            image, address, int(by_address[address]["size"]), by_address)
        if built is None:
            rows.append((derived, base, UNCHECKED,
                         f"the constructor at 0x{address:08X} builds nothing "
                         f"this catalogue names"))
            continue
        name, offset, is_member = built
        if name == base and not is_member:
            rows.append((derived, base, AGREED,
                         f"the constructor at 0x{address:08X} builds a {base} "
                         f"on an unadjusted this"))
        elif is_member:
            # The failure AGENTS.md records: a member read as a base.
            rows.append((derived, base, "refuted",
                         f"the constructor at 0x{address:08X} builds {name} at "
                         f"0x{offset:X}, so it is a member, not a base"))
        else:
            rows.append((derived, base, "refuted",
                         f"the constructor at 0x{address:08X} builds {name} "
                         f"first, not {base}"))
    return rows


def render(rows: list) -> str:
    buffer = io.StringIO()
    writer = csv.writer(buffer, lineterminator="\n")
    writer.writerow(["derived", "base", "verdict", "evidence"])
    for row in sorted(rows):
        writer.writerow(row)
    return buffer.getvalue()


def agreed(path: Path = OUTPUT) -> dict:
    """{derived: base} for the edges the image confirms. The only ones any
    other tool may act on."""
    found = {}
    if not path.is_file():
        return found
    with path.open(newline="", encoding="utf-8-sig") as handle:
        for row in csv.DictReader(handle):
            if row.get("verdict") == AGREED:
                found[row["derived"]] = row["base"]
    return found


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--out", type=Path, default=OUTPUT)
    parser.add_argument("--check", action="store_true",
                        help="fail if the committed CSV is stale")
    args = parser.parse_args(argv)

    edges = claimed_edges()
    if not edges:
        print("SKIP: the fetched Thinker sources are absent.")
        return 0
    if not args.exe.is_file():
        print(f"SKIP: {args.exe} is absent. Every edge here is checked "
              f"against the image.")
        return 0

    rows = check(edges, args.exe)
    text = render(rows)
    counts = {}
    for _, _, verdict, _ in rows:
        counts[verdict] = counts.get(verdict, 0) + 1

    if args.check:
        current = args.out.read_text() if args.out.is_file() else ""
        if current != text:
            print(f"{args.out.name} is stale; regenerate it", file=sys.stderr)
            return 1
        if counts.get("refuted"):
            print(f"the image refutes {counts['refuted']} claimed edge(s)",
                  file=sys.stderr)
            return 1
        print(f"base edges: {counts.get(AGREED, 0)} agreed, "
              f"{counts.get(UNCHECKED, 0)} unchecked, 0 refuted (up to date)")
        return 0

    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(text)
    print(f"{len(rows)} claimed edge(s) -> {args.out}")
    for verdict in (AGREED, "refuted", UNCHECKED):
        if counts.get(verdict):
            print(f"    {verdict}: {counts[verdict]}")
    for derived, base, verdict, evidence in rows:
        if verdict == "refuted":
            print(f"        REFUTED {derived} : {base} - {evidence}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
