#!/usr/bin/env python3
"""Derive `sizeof(T)` from how OTHER classes embed T, and propose the assert.

A class whose header carries no `static_assert(sizeof(T))` stops every offset
walk that reaches an embedded T - `header_offsets.py` prints "unknown size"
and the containing class becomes underivable. Font was the first case solved
by hand: statuswin.h embeds three consecutive Fonts, and the stride between
them IS sizeof(Font).

The general rule is stronger than the stride, because it needs only ONE
embedding: a member annotated `// 0xA` followed by a member annotated
`// 0xB` proves the first member occupies 0xB - 0xA bytes. When the first
member has a class type, that span IS that class's size. Two consecutive
embeddings of the same type are the special case where both annotations
belong to T itself.

Every proposal is corroborated across all the evidence in the tree and then
checked against T's OWN declared members: if T's walk already closes on the
derived size, the assert is safe to write and unlocks the walk. If the walk
falls SHORT, the difference is an undeclared tail - a layout finding, and the
assert would simply fail to compile. This tool never writes a size it cannot
corroborate, and never papers over a shortfall.
"""
from __future__ import annotations
import argparse, collections, re, sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import header_offsets as ho

# `Type name_;  // 0xNN...` and `Type name_[N];  // 0xNN...`
# THE SAME TWO NOTATIONS header_offsets reads - shared, not re-spelled, so a
# second parser cannot drift into knowing only one of the tree's dialects.
MEMBER = re.compile(
    r"^\s{2,}([A-Za-z_][\w:\s*]*?)\s+(\*?)\s*([A-Za-z_]\w*)\s*"
    r"(\[([^\]]*)\])?\s*;\s*//\s*" + ho.NOTE)
SCALARS = set(ho.SIZES)


def evidence(root: Path):
    """Every (type, span) a header's own annotations prove, with provenance."""
    spans: dict[str, list[tuple[int, Path, int, str]]] = collections.defaultdict(list)
    for header in sorted(root.glob("*.h")):
        text = header.read_text(errors="replace")
        lines = text.splitlines()
        prev = None                     # (type, star, name, offset, lineno)
        for n, line in enumerate(lines, 1):
            m = MEMBER.match(line)
            if not m:
                # A blank or comment line does not break adjacency; anything
                # else does - `};` between two members means they are not
                # neighbours and their gap proves nothing.
                s = line.strip()
                if s and not s.startswith("//"):
                    prev = None
                continue
            ty, star, name, _arr, count, note = m.groups()
            ty = " ".join(ty.split())
            here = ho.parse_note(note)
            if prev is not None:
                pty, pstar, pname, poff, pline = prev
                # Only an un-starred, un-arrayed class-typed member proves a
                # size, and only when the next annotation is AFTER it.
                if (not pstar and pty not in SCALARS and here > poff):
                    spans[pty].append((here - poff, header, pline, pname))
            prev = (ty, star, name, here, n) if not count else None
    return spans


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--root", type=Path, default=Path("src"))
    ap.add_argument("--apply", action="store_true",
                    help="write the corroborated asserts into their headers")
    args = ap.parse_args()

    known = ho.sizes(args.root)
    spans = evidence(args.root)

    proposed, conflicting, short, already = [], [], [], 0
    for ty, obs in sorted(spans.items()):
        if ty in known or ty in SCALARS:
            already += 1
            continue
        header = args.root / f"{ty[0].lower()}{ty[1:]}.h"
        widths = {w for w, *_ in obs}
        if len(widths) > 1:
            conflicting.append((ty, sorted(widths), obs))
            continue
        width = widths.pop()
        # Corroborate against T's OWN members: derive its map with the
        # candidate size in hand and see where its declared walk ends.
        decl = None
        for cand in args.root.glob("*.h"):
            if ho.class_body(cand.read_text(errors="replace"), ty):
                decl = cand
                break
        if decl is None:
            continue
        table, dis, _seen, _body, end = ho.derive(decl, ty, {**known, ty: width})
        if dis:
            continue
        # `end` is where the LAST member finishes, not where it starts. A
        # class that closes exactly on the derived width corroborates it from
        # both directions - the containers agree on the span, and T's own
        # members fill it - and that is the only case safe to write.
        if end is not None and end != width:
            short.append((ty, width, decl, end, obs))
            continue
        proposed.append((ty, width, decl, end, obs))

    for ty, widths, obs in conflicting:
        print(f"CONFLICT  {ty}: the tree's annotations prove "
              f"{', '.join(hex(w) for w in widths)} - one of them is wrong")
        for w, h, ln, nm in obs:
            print(f"            {hex(w):>8}  {h.name}:{ln} `{nm}`")

    for ty, width, decl, end, obs in short:
        gap = width - end
        print(f"FINDING   {ty}: containers prove {width:#x}, but {decl.name}'s "
              f"members close at {end:#x}")
        print(f"            {abs(gap):#x} bytes {'undeclared at the tail' if gap > 0 else 'OVER - a member is too wide'}")

    wrote = 0
    for ty, width, decl, end, obs in proposed:
        src = obs[0]
        where = f"{src[1].name}:{src[2]} `{src[3]}`"
        print(f"{ty}: sizeof == {width:#x}   (from {where}"
              f"{f', +{len(obs) - 1} more' if len(obs) > 1 else ''})")
        print(f"    declared members walk to {end:#x}" if end is not None
              else "    no annotated members")
        if not args.apply:
            continue
        text = decl.read_text()
        line = (f'static_assert(sizeof({ty}) == {width:#x},\n'
                f'              "{ty} size is the span {src[3]} occupies in '
                f'{src[1].name}");\n')
        # Place it after the class's closing brace, where the tree puts every
        # other sizeof assert.
        m = re.search(rf"^\}};\s*$", text[text.index(f"class {ty}"):], re.M)
        if not m:
            print(f"    REFUSED: cannot find `{ty}`'s closing brace")
            continue
        at = text.index(f"class {ty}") + m.end()
        decl.write_text(text[:at] + "\n\n" + line.rstrip("\n") + text[at:])
        wrote += 1

    print(f"\n{len(proposed)} size(s) derivable, {len(conflicting)} conflicting, "
          f"{already} already asserted"
          + (f", {wrote} written" if args.apply else "  (dry run; pass --apply)"))
    return 1 if conflicting else 0


if __name__ == "__main__":
    raise SystemExit(main())
