#!/usr/bin/env python3
"""Which vtable belongs to which class, read off the constructors.

Declaring a class with REAL virtual functions instead of an explicit vptr
member needs one fact the tree did not have: each polymorphic class's own
vtable address, and the offsets its vptrs sit at.

`docs/recovery/vtables.csv` could not supply it. 44 rows, 23 classes, 11
distinct vtable addresses - and 0x006698C4 claimed by 15 classes at offset 0,
which is impossible, since distinct classes have distinct vtables. Two
readings of what it might mean instead were tested against the image and both
were refuted: it is not the base's vtable installed by an inlined base
constructor (`??0Win@@QAE@XZ` stores 0x0066FDD0 at +0x0, not 0x006698C4), and
taking "first store is the base's, last is the class's own" moved agreement
with the file DOWN, from 11 rows to 5.

THE CONSTRUCTORS WRITE IT DOWN, unambiguously, as an immediate operand:

    mov dword ptr [ecx], offset ??_7AlphaMovie@@6B@

Derived that way the answer has NO collisions at all: 70 classes, 70 distinct
primary vtables, 149 (class, offset) pairs.

    tools/derive_class_vtables.py            # the table
    tools/derive_class_vtables.py --check    # the gate's question

THREE RULES, EACH FROM A CASE THAT BREAKS THE SIMPLER VERSION.

A SPAN TWO ROWS CLAIM BELONGS TO NEITHER. `??0Sound@@`, `??0Wave@@`,
`??0Ambience@@`, `??0VoiceRx@@` and `??0VoiceTx@@` all list
`0x004C8450-0x004C8457` among their spans - seven bytes of COMDAT-folded tail
that installs a shared base's vptr. Scanning it once per claimant is the whole
of why six classes appeared to share 0x0066E444, and dropping shared spans is
what takes the collision count from one to zero.

THE LAST STORE TO AN OFFSET WINS. A constructor runs its base constructors
first, and when those are inlined their vptr stores are in this body too:
`??0AlphaNet@@` writes +0x144C twice, 0x00669408 then 0x0066EAE8. The derived
class installs its own last, which is exactly why the language guarantees a
base class sees its own virtuals during its own construction.

THE DESTRUCTOR IS NOT A SECOND WITNESS, and this contradicts what an earlier
scan of this tree recorded. Measured over the 57 classes that have both: the
destructor's first vptr store agrees with the constructor's for 39 and
disagrees for 18, and the disagreements are not noise - seven of them
(`FameWin`, `MonuWin`, `MultiWin`, `PickWin`, `PrefWin`, `QuayleWin`,
`ReportIf`) all give 0x00669754, one address, which is a base's. A destructor
adjusts `this` before touching a subobject, so its displacements are 0 for
every vptr it reaches and the order it reaches them in is the order of the
members it destroys. The agreement rate is reported here because it is
evidence about the derivation, and nothing is derived FROM it.
"""
from __future__ import annotations

import argparse
import collections
import re
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import emit_translation_unit as emit  # noqa: E402
from generator_support import read_bytes  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"

CONSTRUCTOR = re.compile(r"^\?\?0([A-Za-z_]\w*)@@")
DESTRUCTOR = re.compile(r"^\?\?1([A-Za-z_]\w*)@@")

# Floors, not targets. The classes come from the catalogue's `??0` names, so
# a rename in `catalogue_corrections` may legitimately add one; losing one is
# a regression in the scan or in the names. Measured 2026-08-14.
CLASS_FLOOR = 70
PAIR_FLOOR = 149
# Distinct classes have distinct vtables. This is not a measurement that could
# drift - it is the property whose failure made the retired CSV unusable, and
# any value above zero means the scan has started reading somebody else's
# bytes again.
COLLISIONS_ALLOWED = 0


class Image:
    """The pinned executable, with the two questions this asks of it."""

    def __init__(self, exe: Path):
        import pefile
        self.pe = pefile.PE(str(exe))
        origin = self.pe.OPTIONAL_HEADER.ImageBase
        self.sections = []
        for section in self.pe.sections:
            start = origin + section.VirtualAddress
            end = start + max(section.Misc_VirtualSize, section.SizeOfRawData)
            self.sections.append((start, end,
                                  section.Name.rstrip(b"\0").decode(errors="replace"),
                                  section.Characteristics))
        self.code = [(low, high) for low, high, _, flags in self.sections
                     if flags & 0x20000000]

    def section_of(self, address: int):
        for low, high, name, _ in self.sections:
            if low <= address < high:
                return name
        return None

    def is_code(self, address: int) -> bool:
        return any(low <= address < high for low, high in self.code)

    def is_vtable(self, address: int) -> bool:
        """A read-only datum whose first slot is a function.

        Cheap and sufficient: the alternative candidates for an immediate
        stored into an object are string literals and small constants, and
        neither begins with a pointer into `.text`.
        """
        if self.section_of(address) not in (".rdata", ".data"):
            return False
        raw = read_bytes(self.pe, address, 4)
        return len(raw) == 4 and self.is_code(struct.unpack("<I", raw)[0])


def parse_spans(row: dict) -> list:
    out = []
    for part in str(row.get("body_ranges") or "").split(";"):
        if "-" in part:
            low, _, high = part.partition("-")
            out.append((int(low, 16), int(high, 16)))
    return out


def shared_spans(rows: dict) -> set:
    """Spans more than one catalogued row claims - COMDAT-folded tails.

    COMPARED AS NUMBERS, not as the text of the `// spans` line. Two rows can
    write one range two ways - a different hex case is enough - and a
    string-keyed count then sees two ranges where there is one, disowns
    neither, and hands the folded tail's vptr store to both claimants. That is
    the exact shape that made six sound classes appear to share 0x0066E444.
    """
    claims = collections.Counter()
    for row in rows.values():
        for span in parse_spans(row):
            claims[span] += 1
    return {span for span, count in claims.items() if count > 1}


def vptr_stores(image: Image, row: dict, shared: set) -> list:
    """(site, offset, vtable) for every `mov [reg+disp], <vtable>` in a body."""
    import capstone
    from capstone import x86
    decoder = getattr(vptr_stores, "_decoder", None)
    if decoder is None:
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        vptr_stores._decoder = decoder

    out = []
    for low, high in parse_spans(row):
        if (low, high) in shared:
            continue
        for instruction in decoder.disasm(read_bytes(image.pe, low, high - low),
                                          low):
            if instruction.mnemonic != "mov" or len(instruction.operands) != 2:
                continue
            target, value = instruction.operands
            if target.type != x86.X86_OP_MEM or value.type != x86.X86_OP_IMM:
                continue
            # A REGISTER PLUS A CONSTANT, which is what `this + offset` is. An
            # indexed store is walking an array, not installing a vptr.
            if target.mem.base == 0 or target.mem.index != 0:
                continue
            if not image.is_vtable(value.imm):
                continue
            out.append((instruction.address, target.mem.disp, value.imm))
    return out


class Derivation:
    def __init__(self):
        self.vtable = {}         # (class, offset) -> vtable
        self.site = {}           # (class, offset) -> the storing instruction
        self.destructor = {}     # class -> its destructor's first vtable

    @property
    def classes(self) -> set:
        return {name for name, _ in self.vtable}

    def primary(self) -> dict:
        return {name: self.vtable[(name, 0)] for name in self.classes
                if (name, 0) in self.vtable}

    def collisions(self) -> dict:
        owners = collections.defaultdict(set)
        for name, address in self.primary().items():
            owners[address].add(name)
        return {address: sorted(names) for address, names in owners.items()
                if len(names) > 1}

    def classify(self, name: str, offset: int) -> str:
        """What a vptr at `offset` in `name` is - as far as this can prove.

        `own primary` is the class's identity, and `subobject` is a store of
        ANOTHER class's primary, so an inlined base or embedded member
        constructor. Both are settled by the evidence.

        `unattributed` IS NOT "the class's second vtable", though most of them
        will be: no catalogued constructor stores that address at offset 0, so
        it is either a multiple-inheritance secondary of this class or an
        embedded member of a class whose constructor the catalogue does not
        name. The byte distance from this class's own primary is printed
        beside it because MSVC emits a class's vtable group adjacently -
        `AlphaMovie`'s two sit eight bytes apart - but adjacency is a hint and
        is deliberately not spent as a verdict.
        """
        if offset == 0:
            return "own primary"
        owners = self.primary()
        holder = [one for one, address in owners.items()
                  if address == self.vtable[(name, offset)]]
        if not holder or holder == [name]:
            return "unattributed"
        return "subobject"

    def distance(self, name: str, offset: int):
        """Bytes from this class's own primary vtable, or None."""
        mine = self.primary().get(name)
        if mine is None:
            return None
        return self.vtable[(name, offset)] - mine


def derive(rows: dict = None, exe: Path = DEFAULT_EXE) -> Derivation:
    rows = emit.load_functions() if rows is None else rows
    image = Image(exe)
    shared = shared_spans(rows)
    out = Derivation()
    for _, row in sorted(rows.items()):
        name = row.get("name") or ""
        built = CONSTRUCTOR.match(name)
        if built:
            for site, offset, vtable in vptr_stores(image, row, shared):
                # LAST WINS: see the module docstring on ??0AlphaNet@@.
                out.vtable[(built.group(1), offset)] = vtable
                out.site[(built.group(1), offset)] = site
            continue
        torn = DESTRUCTOR.match(name)
        if torn:
            found = vptr_stores(image, row, shared)
            if found:
                out.destructor.setdefault(torn.group(1), found[0][2])
    return out


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--check", action="store_true",
                        help="fail if the derivation has lost ground")
    # `--src` EXISTS SO THE DAMAGE CASE CAN RUN THIS EXACT TOOL, and the
    # floors below are skipped under it. A two-file tree has two classes, so
    # applying a floor of 70 would make the case "fail" for its size rather
    # than for the collision it was built to cause - and it would pass just as
    # well with the collision test deleted. This repository has published one
    # of those.
    parser.add_argument("--src", type=Path, default=None,
                        help="an annotation tree to read instead of src/")
    arguments = parser.parse_args(argv)

    if not arguments.exe.is_file():
        print(f"REFUSED: {arguments.exe} is not present. The vtable of a class "
              f"is an immediate in its constructor; there is nowhere else to "
              f"read it from.", file=sys.stderr)
        return 2

    rows = None
    if arguments.src is not None:
        import project_catalogue
        rows = project_catalogue.from_source(arguments.src)
    found = derive(rows, exe=arguments.exe)
    primary = found.primary()
    collisions = found.collisions()
    kinds = collections.Counter(found.classify(name, offset)
                                for name, offset in found.vtable)

    if not arguments.check:
        for name in sorted(found.classes):
            offsets = sorted(offset for one, offset in found.vtable
                             if one == name)
            print(f"{name}")
            for offset in offsets:
                gap = found.distance(name, offset)
                near = "" if not gap else f"  {gap:+d} from its primary"
                print(f"    +0x{offset:<6X} 0x{found.vtable[(name, offset)]:08X}"
                      f"  {found.classify(name, offset):<13}"
                      f"  stored at 0x{found.site[(name, offset)]:08X}{near}")

    print(f"{len(found.classes)} class(es), {len(found.vtable)} "
          f"(class, offset) pair(s), {len(primary)} primary vtable(s)")
    for kind, count in sorted(kinds.items()):
        print(f"  {kind:<14} {count}")

    loose = [(name, offset) for name, offset in found.vtable
             if found.classify(name, offset) == "unattributed"]
    grouped = sum(1 for name, offset in loose
                  if (found.distance(name, offset) or 0x1000) in
                  range(-0x40, 0x41))
    print(f"  {grouped} of the {len(loose)} unattributed sit within 0x40 of "
          f"their class's own primary, which is what an MSVC vtable group "
          f"looks like; the rest are further off and unsettled")
    common = collections.Counter(offset for _, offset in found.vtable if offset)
    for offset, count in common.most_common(3):
        print(f"  +0x{offset:X} appears {count} time(s)")

    both = set(primary) & set(found.destructor)
    agree = sum(1 for name in both
                if primary[name] == found.destructor[name])
    print(f"  the destructor's first store agrees on {agree} of {len(both)} "
          f"class(es) that have one; it is evidence, not a source")

    if not arguments.check:
        return 0

    failures = []
    # A RUN THAT DERIVED NOTHING PROVES NOTHING, whatever tree it was pointed
    # at. Found while building the damage case: giving two constructors the
    # same span string makes `shared_spans` disown both, the derivation comes
    # back empty, and with the floors skipped under `--src` an empty answer
    # exited 0.
    if not found.classes:
        failures.append("no class carries a vtable store at all, so this "
                        "compared nothing")
    if collisions:
        for address, names in sorted(collisions.items()):
            failures.append(f"0x{address:08X} is claimed at offset 0 by "
                            f"{', '.join(names)}; distinct classes have "
                            f"distinct vtables")
    if arguments.src is None:
        if len(found.classes) < CLASS_FLOOR:
            failures.append(f"{len(found.classes)} classes, below the pinned "
                            f"{CLASS_FLOOR}")
        if len(found.vtable) < PAIR_FLOOR:
            failures.append(f"{len(found.vtable)} (class, offset) pairs, below "
                            f"the pinned {PAIR_FLOOR}")
    for line in failures:
        print(f"  {line}", file=sys.stderr)
    if failures:
        print("FAIL: the class-to-vtable derivation has lost ground. It is "
              "read from constructor immediates, so this is the scan or the "
              "catalogued names changing, never the image.", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
