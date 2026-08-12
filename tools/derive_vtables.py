#!/usr/bin/env python3
"""Where each class's vtable pointers live, and nothing about how long they are.

A constructor installs the vtable by storing its address into the object, so
`mov dword ptr [ecx + N], 0x6698c4` says three things at once: this class is
polymorphic, its vtable pointer sits at offset N, and the table is at that
address. All three are read straight off the instruction and none of them needs
the table to be walked.

WHAT THIS DELIBERATELY DOES NOT DO IS WALK IT. The obvious next step - follow
the table while each dword is a known function start, then transfer names
between classes by slot index - is not sound here, and the reason is worth
writing down rather than rediscovering:

  * The image carries no `??_7` vftable symbols. It carries no symbols at all;
    every mangled name in `functions.csv` comes from external analysis. So
    there is no marker for where a table ends.
  * `docs/recovery/functions.csv` catalogues functions, not data, so `.rdata`
    offers no boundary either.
  * Walking until a dword stops being a function start therefore runs off the
    end of one table into the next whenever they abut. Measured: 44 tables,
    7,105 slots, a median of 92 and a maximum of 401 - and 401 is where the
    walk was capped, not where the table stopped.

A name transferred to the wrong slot is a wrong name written into `src/`, and
the whole campaign around this file has been about not doing that. So the
extent is left unclaimed and only the first slot is recorded, because the first
slot is the one the constructor's store already proves is a function.

WHAT IS RECOVERED ANYWAY. The offsets are structural: `BaseWin`, `MessageWin`
and `ImageButton` each install a SECOND vtable at 0x444, which is sizeof(Win),
so those classes carry a second polymorphic base beginning there. That is
multiple inheritance, read off the constructors, and it is the kind of thing a
flat layout cannot say.
"""

from __future__ import annotations

import argparse
import collections
import csv
import io
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from capstone import CS_ARCH_X86, CS_MODE_32, Cs  # noqa: E402
from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_REG_ECX  # noqa: E402

import derive_access_bounds  # noqa: E402
import derive_class_layout  # noqa: E402


def _catalogue_rows():
    """Every catalogued row, from `src/`.

    `docs/recovery/functions.csv` is deleted: every `ORIGINAL:` annotation
    carries its own name, size, spans, prototype, kind, flags and call
    edges, and `emit.load_functions()` reads them back. This tool opened
    the CSV directly, so it broke the moment the store moved - which is
    how five layout gates went red at once.
    """
    import sys as _sys
    from pathlib import Path as _Path
    _sys.path.insert(0, str(_Path(__file__).resolve().parent))
    import emit_translation_unit as _emit
    return list(_emit.load_functions().values())

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"
FUNCTIONS = REPO_ROOT / "docs" / "recovery" / "functions.csv"
OUTPUT = REPO_ROOT / "docs" / "recovery" / "vtables.csv"

# `??0` constructor, `??1` destructor - the two places a vtable pointer is
# installed.
INSTALLERS = ("??0", "??1")


def vtables(exe: Path) -> list:
    """[(class, offset, table, first slot name)] - one row per install."""
    image = derive_class_layout.Image(exe)
    engine = Cs(CS_ARCH_X86, CS_MODE_32)
    engine.detail = True
    by_address, _ = derive_class_layout.load_functions()
    starts = set(by_address)
    ceiling = image.pe.OPTIONAL_HEADER.ImageBase + \
        image.pe.OPTIONAL_HEADER.SizeOfImage

    found = collections.defaultdict(set)
    # `src/` is the catalogue's store; the export is deleted.
    for row in _catalogue_rows():
        name = row.get("name") or ""
        if not name.startswith(INSTALLERS):
            continue
        klass = derive_access_bounds.receiver_scope(name)
        if not klass:
            continue
        try:
            address, size = int(row["address"], 16), int(row["size"])
        except (ValueError, KeyError, TypeError):
            continue
        if not 0 < size <= 4000:
            continue
        start = address - image.code_start
        if start < 0 or start + size > len(image.code):
            continue
        for instruction in engine.disasm(
                image.code[start:start + size], address):
            if instruction.mnemonic != "mov" or len(
                    instruction.operands) != 2:
                continue
            destination, source = instruction.operands
            if (destination.type != X86_OP_MEM
                    or destination.mem.base != X86_REG_ECX
                    or destination.mem.index != 0
                    or source.type != X86_OP_IMM):
                continue
            table = source.imm & 0xFFFFFFFF
            if not image.pe.OPTIONAL_HEADER.ImageBase < table < ceiling:
                continue
            # The store is only a vtable install if what it points at is a
            # function. An ordinary constant assigned to a member is not.
            slot = image.dword(table)
            if slot in starts:
                found[(klass, destination.mem.disp, table)].add(slot)

    rows = []
    for (klass, offset, table), slots in found.items():
        slot = sorted(slots)[0]
        rows.append((klass, offset, table,
                     by_address[slot].get("name") or f"sub_{slot:x}"))
    return rows


def render(rows: list) -> str:
    buffer = io.StringIO()
    writer = csv.writer(buffer, lineterminator="\n")
    writer.writerow(["class", "pointer_offset", "vtable", "first_slot"])
    for klass, offset, table, first in sorted(rows):
        writer.writerow([klass, f"0x{offset:X}", f"0x{table:08X}", first])
    return buffer.getvalue()


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--out", type=Path, default=OUTPUT)
    parser.add_argument("--check", action="store_true",
                        help="fail if the committed CSV is stale")
    args = parser.parse_args(argv)

    if not args.exe.is_file():
        print(f"SKIP: {args.exe} is absent. Every row here is read out of "
              f"the image.")
        return 0

    rows = vtables(args.exe)
    text = render(rows)

    if args.check:
        current = args.out.read_text() if args.out.is_file() else ""
        if current != text:
            print(f"{args.out.name} is stale; regenerate it", file=sys.stderr)
            return 1
        classes = len({row[0] for row in rows})
        print(f"vtables: {len(rows)} install(s) across {classes} classes "
              f"(up to date)")
        return 0

    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(text)
    classes = {row[0] for row in rows}
    secondary = [row for row in rows if row[1]]
    print(f"{len(rows)} vtable install(s) across {len(classes)} classes "
          f"-> {args.out}")
    print(f"    {len(secondary)} at a nonzero offset, so a second polymorphic "
          f"base begins there")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
