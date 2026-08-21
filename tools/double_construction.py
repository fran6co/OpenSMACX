#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# dependencies = []
# ///
"""Members that are constructed twice - once by the compiler, once by hand.

A class declares `Font font_;` and the compiler builds it. The constructor
body then ALSO does `new (&font_) Font();`, so it is built again on top of
itself. The program mostly survives it - the second construction overwrites
a freshly-built object with an identical one - which is exactly why it goes
unnoticed.

It is not free. `SocialWin::SocialWin` was doing this to seventeen members at
once and compiled to 232 instructions against an image of 121; removing the
redundant construction dropped it to 133. Measured 2026-08-21.

    uv run tools/double_construction.py

ONLY INSIDE A REAL CONSTRUCTOR. Placement-new in a `construct()` METHOD is
intentional and correct - the object already exists, its members were never
implicitly built, and building them by hand is the whole job. Without that
distinction this check reported five instances in
`GraphicWin::construct`, `BaseButton::construct` and `WorldWin::construct`,
every one of them deliberate. Flagging those would have sent someone to break
working code, which is worse than not having the check at all.
"""
import pathlib
import re
import sys

REPO_ROOT = pathlib.Path(__file__).resolve().parent.parent

# `new (&member_) Type()`, the spelling that names the member outright.
BY_NAME = re.compile(r"new\s*\(\s*&\s*(\w+_)\s*\)\s*(\w+)\s*\(")
# `new (self + 0xA20) Type()`, the spelling that does NOT - and this is the
# one that mattered. SocialWin's seventeen double constructions were all
# written at raw offsets, so a check that only understood `&member_` could
# not see the very case it was built for. Its own control caught that.
BY_OFFSET = re.compile(
    r"new\s*\(\s*\w+\s*\+\s*(0[xX][0-9a-fA-F]+)\s*\)\s*(\w+)\s*\(")
# A member declaration carrying its offset: `Spot spot_;  // 0xA20`.
MEMBER = re.compile(
    r"^\s*(?:const\s+)?([A-Z]\w*)\s+(\w+_)\s*(\[[^\]]*\])?\s*;"
    r"(?:\s*//\s*(0[xX][0-9a-fA-F]+))?")
# A definition at column zero: `Ret Class::name(` or `Class::Class(`.
# The optional group is the RETURN TYPE, and it has to be optional as a whole
# rather than a leading character class: written `^[A-Za-z_][\w:*& ]*?(\w+)::`
# the first letter is eaten by the anchor, so `SocialWin::SocialWin()` parsed
# as class `ocialWin` method `SocialWin` - which never compares equal, so no
# constructor was ever recognised and the check reported zero for everything.
DEFINITION = re.compile(r"^(?:[A-Za-z_][\w:*&<>, ]*?\s+\*?)?(\w+)::(~?\w+)\s*\(")


def members_of(header: pathlib.Path) -> dict:
    """Every `Type name_;` the header declares, by name."""
    found = {}
    try:
        text = header.read_text(errors="replace")
    except OSError:
        return found
    for line in text.splitlines():
        hit = MEMBER.match(line)
        if hit:
            offset = int(hit.group(4), 16) if hit.group(4) else None
            found[hit.group(2)] = (hit.group(1), header.name, offset)
    return found


if __name__ == "__main__":
    rows = []
    for source in sorted((REPO_ROOT / "src").glob("*.cpp")):
        header = source.with_suffix(".h")
        if not header.exists():
            continue
        declared = members_of(header)
        if not declared:
            continue
        try:
            text = source.read_text(errors="replace")
        except OSError:
            continue
        enclosing = None
        for number, line in enumerate(text.splitlines(), 1):
            start = DEFINITION.match(line)
            if start:
                cls, method = start.groups()
                # A REAL CONSTRUCTOR is `Class::Class(`. Anything else - and
                # `construct()` in particular - has no implicit member
                # construction to duplicate.
                enclosing = (cls, method) if method == cls else None
            if line.lstrip().startswith("//") or not enclosing:
                continue
            named = BY_NAME.search(line)
            target = built = None
            if named:
                target, built = named.group(1), named.group(2)
                if target not in declared:
                    continue                  # raw storage, which is the point
            else:
                at = BY_OFFSET.search(line)
                if not at:
                    continue
                offset, built = int(at.group(1), 16), at.group(2)
                for name_, (kind_, _, off_) in declared.items():
                    if off_ == offset:
                        target = name_
                        break
                if target is None:
                    continue                  # an offset no member claims
            kind, where, _ = declared[target]
            rows.append((source.name, number, target, kind, built,
                         where, f"{enclosing[0]}::{enclosing[1]}"))

    for name, number, target, kind, built, where, func in rows:
        print(f"  {name}:{number}  in {func}(), `{target}` is a declared "
              f"`{kind}` in {where} and is placement-new'd as `{built}` again")
    classes = {r[0] for r in rows}
    print(f"\n{len(rows)} member(s) constructed twice, across "
          f"{len(classes)} file(s)")
    sys.exit(0)
