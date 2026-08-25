#!/usr/bin/env python3
"""Report what each hand-called CRT vector iterator says the member IS.

    VectorCtorIterator(self + 0x2C60, 0x2C, 0xA, SomeCtor, SomeDtor);

is the compiler's array construction written out by hand, and it carries
its own proof of what the member is: the STRIDE is the element's sizeof and
the COUNT is the extent, so the raw `uint8_t[N]` sitting at that offset must
be `Element[COUNT]` with `sizeof(Element) * COUNT == N`. Declaring the real
array makes VC6 emit the iterators itself, and the `const void *` ctor/dtor
constants - which are the element's own constructor and destructor, and
which C++ cannot spell as function references - go with them.

    uv run tools/vector_iterators.py            # every site, with its arithmetic

WHAT THE THREE MEASUREMENTS SO FAR SAY, because the transformation is NOT
uniformly a win and this tool should not be read as a to-do list:

  FontQueue   ONE array, tiny ctor      21/21 and 6/6, BYTE_EXACT both ways
  SocialWin   FIVE arrays, large ctor   20/121 -> 16/121
  ReportIf    SIX arrays, large ctor    34/132 -> 22/132

The arithmetic was exact in all three - 0x2C * 0xA == 0x1B8, 7 * 0xB4C, and
so on - so the DECLARATIONS are right in all three. What differs is whether
VC6 emits its generated iterator calls in the image's order, and with many
arrays it does not: the class is still missing members BETWEEN the arrays,
so the generated construction order cannot line up yet. Expect a single
array to hold its claim and a multi-array constructor to score worse until
the rest of its layout is declared.

The debt is still worth taking: reportif's function-address bindings went
4 -> 0, and those `const void *` constants are a class's own constructor
and destructor, which C++ cannot spell as function references at all.
"""
from __future__ import annotations

import argparse
import re
from pathlib import Path

CALL = re.compile(
    r"Vector(Ctor|Dtor)Iterator\(\s*([^,]+?),\s*(0x[0-9A-Fa-f]+|\d+),\s*"
    r"(0x[0-9A-Fa-f]+|\d+)\s*,\s*(\w+)")
SIZEOF = re.compile(r"static_assert\(sizeof\((\w+)\)\s*==\s*(0x[0-9A-Fa-f]+|\d+)")
ADDR_CONST = re.compile(r"^\s*const void \*const (\w+) = \(const void \*\)(0x[0-9A-Fa-f]+);",
                        re.M)
MEMBER = re.compile(r"^\s*uint8_t (\w+)\[(0x[0-9A-Fa-f]+|\d+)\];\s*//\s*(0x[0-9A-Fa-f]+)",
                    re.M)


def sizes(root: Path) -> dict[str, int]:
    out = {}
    for h in root.glob("*.h"):
        for name, size in SIZEOF.findall(h.read_text(errors="replace")):
            out[name] = int(size, 0)
    return out


def owners(root: Path) -> dict[int, str]:
    """address -> the symbol a marker gives it."""
    out = {}
    marker = re.compile(r"^// ORIGINAL: (0x[0-9A-Fa-f]+) (\S+)", re.M)
    for path in list(root.rglob("*.cpp")):
        for addr, sym in marker.findall(path.read_text(errors="replace")):
            out.setdefault(int(addr, 16), sym)
    return out


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--root", type=Path, default=Path("src"))
    ap.add_argument("--skip", nargs="*", default=["init_thunks.cpp", "atexit_thunks.cpp"],
                    help="the CRT archives, where the iterator call IS the body")
    args = ap.parse_args()

    size_of = sizes(args.root)
    symbol_at = owners(args.root)
    total = matched = 0
    for src in sorted(args.root.glob("*.cpp")):
        if src.name in args.skip:
            continue
        text = src.read_text(errors="replace")
        consts = {n: int(a, 16) for n, a in ADDR_CONST.findall(text)}
        header = src.with_suffix(".h")
        members = {}
        if header.exists():
            for name, extent, off in MEMBER.findall(header.read_text(errors="replace")):
                members[int(off, 16)] = (name, int(extent, 0))
        hits = list(CALL.finditer(text))
        if not hits:
            continue
        print(f"\n{src.name}")
        for m in hits:
            total += 1
            base, stride, count, ctor = m.group(2), int(m.group(3), 0), int(m.group(4), 0), m.group(5)
            off = None
            hit = re.search(r"\+\s*(0x[0-9A-Fa-f]+)", base)
            if hit:
                off = int(hit.group(1), 16)
            addr = consts.get(ctor)
            sym = symbol_at.get(addr, "") if addr else ""
            # THE CONSTRUCTOR'S OWN NAME IS AUTHORITATIVE. Matching the stride
            # against sizeof() is ambiguous - sizeof(Map) and sizeof(Sprite)
            # are both 0x2C, and the size lookup reported `Map[3]` for an
            # array the symbol plainly calls Sprite's. Only fall back to the
            # size when there is no symbol to read.
            elem = None
            named = re.match(r"\?\?[01](\w+)@@", sym)
            if named:
                elem = named.group(1)
            elif stride:
                candidates = [k for k, v in size_of.items() if v == stride]
                elem = candidates[0] if len(candidates) == 1 else None
            want = stride * count
            member = members.get(off) if off is not None else None
            ok = member and member[1] == want
            if ok:
                matched += 1
            line = (f"  {m.group(1):<4} +{off:#07x} " if off is not None
                    else f"  {m.group(1):<4} {base.strip()[:18]:<18} ")
            line += f"stride {stride:#x} x {count:#x} = {want:#x}"
            if elem:
                line += f"  -> {elem}[{count:#x}]"
            if member:
                line += f"  member {member[0]}[{member[1]:#x}]" + ("  MATCHES" if ok else "  MISMATCH")
            if sym:
                line += f"   ctor={sym}"
            print(line)
    print(f"\n{total} hand-called iterator(s); {matched} where the arithmetic "
          f"matches a raw uint8_t member exactly")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
