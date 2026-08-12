#!/usr/bin/env python3
"""Every object a class builds inside itself, and whether src/ declares it.

A constructor and a destructor between them name the whole by-value structure
of a class. `??0Flic@@QAE@XZ` calling `??0Palette@@QAE@XZ` on `this + 0x5BC` is
not a hint that a palette is somewhere nearby - it is a statement that a
`Palette` OBJECT begins at 0x5BC, because a constructor's receiver is the
object it constructs. `src/flic.h` said `Palette *palette_;` there, four bytes
where 0x454 belong.

This is the third independent reading of a layout in this tree, and the only
one that recovers TYPES rather than extents:

  derive_base_edges.py    checks the sixteen inheritance edges Thinker states
  verify_member_offsets   compares declared boundaries with access widths
  this                    enumerates every subobject the image constructs or
                          tears down, at any offset, whether or not any
                          outside source mentioned it

WHAT AN OFFSET OF ZERO MEANS. A base subobject is built on an unadjusted
`this`, so a call at offset 0 is either a base or a first member - and MSVC
cannot tell them apart, because it lays them out identically. The verdict is
therefore `base` when `src/` already derives from that class, and `at-zero`
when it does not, which is a question for a human: `PopMenu` derives from
`BasePop` in `src/` and its destructor calls `?close@Popup@@QAEXXZ` on `this`,
so the real edge is one level deeper than the tree says.

THE OUTERMOST TYPE IS THE MEMBER. A destructor for a derived member runs its
base's destructor on the same pointer, so `AlphaSave + 0x1BC0` shows up as both
`FlatButton` and its base `BaseButton`. Only the largest is the member; the
rest are its own bases doing their job.

WHAT IT CANNOT SEE. A member whose class has no constructor or destructor at
all - a plain aggregate like `RECT` - is never built by a call and never
appears here. Absence is not evidence, and every verdict this prints is about
a site the image DOES name.
"""

from __future__ import annotations

import argparse
import collections
import csv
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import class_layouts  # noqa: E402
import verify_member_offsets as offsets  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
SRC = REPO_ROOT / "src"
CATALOGUE = REPO_ROOT / "docs" / "recovery" / "subobject-sites.csv"

# `??0X@@QAE...` and `??1X@@QAE...` - a constructor or destructor whose
# receiver is the subobject. `?close@X@@QAE` joins them because MSVC inlines a
# destructor whose body is one `close()` call often enough that the teardown is
# the only witness left: BaseWin's third Scroll is reached that way and no
# other.
BUILDER = re.compile(r"^\?\?[01](\w+)@@QAE")
CLOSER = re.compile(r"^\?close@(\w+)@@QAE")


def sites(exe: Path) -> dict:
    """{class: {offset: {built class: evidence}}}."""
    from capstone import CS_ARCH_X86, CS_MODE_32, Cs
    from capstone.x86 import (X86_INS_LEA, X86_INS_MOV, X86_OP_IMM,
                              X86_OP_MEM, X86_OP_REG)

    import derive_access_bounds as bounds
    import derive_class_layout as layout

    image = layout.Image(exe)
    engine = Cs(CS_ARCH_X86, CS_MODE_32)
    engine.detail = True
    by_address, _ = layout.load_functions()
    whole = bounds.WHOLE_REGISTER

    found = collections.defaultdict(lambda: collections.defaultdict(dict))
    for klass, methods in sorted(bounds.receivers().items()):
        for address, size, mangled in methods:
            start = address - image.code_start
            if start < 0 or start + size > len(image.code):
                continue
            carries = {"ecx": 0}
            for ins in engine.disasm(image.code[start:start + size], address):
                if ins.mnemonic == "call" and "ecx" in carries:
                    for operand in ins.operands:
                        if operand.type != X86_OP_IMM:
                            continue
                        row = by_address.get(operand.imm & 0xFFFFFFFF)
                        name = (row or {}).get("name") or ""
                        hit = BUILDER.match(name) or CLOSER.match(name)
                        if hit and hit.group(1) != klass:
                            found[klass][carries["ecx"]].setdefault(
                                hit.group(1),
                                f"{mangled}: call {name} on this+"
                                f"0x{carries['ecx']:X} at 0x{ins.address:08X}")
                gained = None
                operands = ins.operands
                if len(operands) == 2 and operands[0].type == X86_OP_REG:
                    destination = whole.get(engine.reg_name(operands[0].reg)
                                            or "")
                    if destination:
                        if (ins.id == X86_INS_MOV
                                and operands[1].type == X86_OP_REG):
                            source = whole.get(
                                engine.reg_name(operands[1].reg) or "")
                            if source in carries:
                                gained = (destination, carries[source])
                        elif (ins.id == X86_INS_LEA
                                and operands[1].type == X86_OP_MEM
                                and operands[1].mem.index == 0):
                            source = whole.get(
                                engine.reg_name(operands[1].mem.base) or "")
                            if source in carries:
                                gained = (destination,
                                          carries[source]
                                          + operands[1].mem.disp)
                for register in ins.regs_access()[1]:
                    carries.pop(whole.get(engine.reg_name(register) or ""),
                                None)
                if gained and gained[1] >= 0:
                    carries[gained[0]] = gained[1]
                if not carries:
                    break
    return {klass: dict(at) for klass, at in found.items()}


def verdicts(built: dict, layouts: dict, sizes: dict) -> list:
    """[(verdict, class, offset, type, detail, evidence)].

      base        offset 0, and src/ already derives from it
      at-zero     offset 0, and src/ does not - a base edge, or a first member
      agreed      a member of exactly that size is declared there
      mistyped    a member IS declared there and it is the wrong size
      absent      nothing is declared at that offset
    """
    out = []
    for klass, at in sorted(built.items()):
        rows = layouts.get(klass)
        if rows is None:
            continue
        widest = {}
        for name, offset, size, spelled in rows:
            if offset not in widest or size > widest[offset][1]:
                widest[offset] = (name, size, spelled)
        bases = set(class_layouts.base_chain(klass))
        for offset, types in sorted(at.items()):
            # The outermost type is the member; the rest are its own bases,
            # torn down on the same pointer.
            kind = max(types, key=lambda name: sizes.get(name, 0))
            evidence = types[kind]
            want = sizes.get(kind)
            got = widest.get(offset)
            if offset == 0:
                if kind in bases:
                    out.append(("base", klass, offset, kind,
                                f"src/ bases: {sorted(bases)}", evidence))
                elif got and want and got[1] == want:
                    # A FIRST MEMBER, not a base. MSVC lays the two out
                    # identically, so the only thing that tells them apart
                    # here is whether src/ already declares something of
                    # exactly that size at zero - `Win` opens with
                    # `AutoSound auto_sound_` and its 0x98 match, which is a
                    # correct declaration rather than a missing edge.
                    out.append(("agreed", klass, offset, kind,
                                f"{got[0]} 0x{got[1]:X}, first member",
                                evidence))
                else:
                    out.append(("at-zero", klass, offset, kind,
                                f"src/ bases: {sorted(bases)}", evidence))
                continue
            if got is None:
                out.append(("absent", klass, offset, kind,
                            f"0x{want:X} bytes" if want else "size unknown",
                            evidence))
            elif want and got[1] == want:
                out.append(("agreed", klass, offset, kind,
                            f"{got[0]} 0x{got[1]:X}", evidence))
            elif want and got[2].split()[-1] == kind and got[1] % want == 0:
                # AN ARRAY OF THEM. `ButtonGroup buttonGroups_[4]` is 0x250
                # against a 0x94 ButtonGroup, and the constructor building
                # element zero on `this + 0x1EB84` is that array behaving
                # correctly, not a member of the wrong size.
                out.append(("agreed", klass, offset, kind,
                            f"{got[0]}, {got[1] // want} element(s)",
                            evidence))
            else:
                out.append(("mistyped", klass, offset, kind,
                            f"declared `{got[2]} {got[0]}`, 0x{got[1]:X} bytes"
                            f" against 0x{want:X}" if want
                            else f"declared `{got[2]} {got[0]}`", evidence))
    order = {"mistyped": 0, "at-zero": 1, "absent": 2, "agreed": 3, "base": 4}
    return sorted(out, key=lambda row: (order[row[0]], row[1], row[2]))


def render(rows: list) -> str:
    import io

    buffer = io.StringIO()
    writer = csv.writer(buffer, lineterminator="\n")
    writer.writerow(["verdict", "class", "offset", "built", "detail",
                     "evidence"])
    for verdict, klass, offset, kind, detail, evidence in sorted(
            rows, key=lambda row: (row[1], row[2])):
        writer.writerow([verdict, klass, f"0x{offset:X}", kind, detail,
                         evidence])
    return buffer.getvalue()


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path)
    parser.add_argument("--publish", action="store_true")
    parser.add_argument("--class", dest="only")
    parser.add_argument("--verdict", choices=("base", "at-zero", "agreed",
                                              "mistyped", "absent"))
    parser.add_argument("--src", type=Path, default=SRC)
    args = parser.parse_args(argv)

    import byte_match as bm
    import derive_class_layout as layout

    exe = args.exe or layout.DEFAULT_EXE
    if not exe.is_file():
        print(f"SKIP: {exe} is absent")
        return 0
    reason = bm.available()
    if reason:
        print(f"SKIP: {reason}")
        return 0

    layouts, sizes, refused = offsets.declared(args.src)
    for line in refused:
        print(f"  refused {line}")

    rows = verdicts(sites(exe), layouts, sizes)
    if args.publish:
        CATALOGUE.write_text(render(rows))
        print(f"wrote {CATALOGUE.relative_to(REPO_ROOT)}: {len(rows)} sites")

    tally = collections.Counter(row[0] for row in rows)
    print("subobject sites: " + "  ".join(
        f"{name} {count}" for name, count in sorted(tally.items())))
    shown = [row for row in rows
             if (not args.only or row[1] == args.only)
             and (not args.verdict or row[0] == args.verdict)]
    for verdict, klass, offset, kind, detail, evidence in shown:
        if verdict in ("agreed", "base") and not args.verdict:
            continue
        print(f"  {verdict:<8} {klass}+0x{offset:X} builds {kind}: {detail}\n"
              f"           {evidence}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
