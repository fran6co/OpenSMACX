#!/usr/bin/env python3
"""Derive a class's ABI layout from the pinned executable.

Class layouts are the gating knowledge for this recovery. A method cannot be
replaced until its field offsets are known, and roughly half the unrecovered
bytes sit in UI classes whose sizes have never been established - so each
`static_assert(sizeof(X) == N)` unblocks that class's whole method set. Doing
this by hand takes an hour per class and there are around ninety left.

Sizes come from two kinds of evidence, and the distinction is the whole point:

  exact   the image states the size. `operator new(N)` immediately feeding a
          construction, or a vbtable's virtual base offset plus an already
          pinned base size.
  upper   the image bounds the size. The next global object's address bounds a
          global instance from above, but only loosely - globals with no
          dynamic initialiser sit in the gap unseen, and measuring Font this
          way gives 0x718 against its true 0x28.

Only exact evidence can settle a size. Upper bounds are printed because they
are useful for sanity-checking a candidate, never to derive one.

Two traps make naive versions of this tool confidently wrong, and both are
guarded here. A constructor invoked while building a larger object sees that
object's allocation: every `Scroll::Scroll` call site in this image sits inside
`Popup`, `AlphaSave`, or `BaseWin`, so the adjacent `operator new(0x537C)`
describes the enclosing object and not Scroll's real 0x214C. Allocation
evidence is therefore taken only where the constructed pointer is the raw
allocation, unadjusted and unclaimed by another constructor. Separately, an
enclosing object whose base sits at offset zero is indistinguishable from the
base alone at a call site, so a size is reported only when nothing contradicts
it.

A layout guessed wrong compiles perfectly and produces methods that corrupt
memory at runtime, which is why silence is the designed-for outcome and
`--check-pinned` scores disagreements rather than coverage: replaying the
derivation against every hand-pinned class, the number that must stay zero is
the number of confident wrong answers.
"""

from __future__ import annotations

import argparse
import bisect
import csv
import re
import struct
import sys
from dataclasses import dataclass, field
from pathlib import Path

import pefile
from capstone import CS_ARCH_X86, CS_MODE_32, Cs
from capstone.x86 import X86_OP_IMM, X86_OP_MEM

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"
FUNCTIONS_CSV = REPO_ROOT / "docs" / "recovery" / "functions.csv"
SRC_DIR = REPO_ROOT / "src"

OPERATOR_NEW = 0x0064558A
PINNED_RE = re.compile(r"static_assert\(sizeof\((\w+)\)\s*==\s*(0x[0-9A-Fa-f]+|\d+)")
CTOR_RE = re.compile(r"^\?\?0(\w+)@@")
INITIALIZER_RE = re.compile(r"^\?\?__E")

# MSVC's EH vector iterators take the ELEMENT SIZE as a literal argument, so
# every array-constructed class states its own sizeof at each call site. This is
# exact evidence of the same standing as `operator new(N)`, and it reaches a
# population allocation evidence cannot: an object that is never heap-allocated
# but does appear as a global array.
#
# The argument counts are the signatures' own: the constructor iterators take
# (ptr, size, count, ctor, dtor) and the destructor iterator (ptr, size, count,
# dtor). Pushes are right-to-left, so in program order the size is always the
# second-to-last.
VECTOR_ITERATORS = {
    0x006457C2: 5,   # ??_L@YGXPAXIHP6EX0@Z1@Z  eh vector constructor iterator
    0x00645F8E: 5,   # ??_N@YGXPAXIHP6EX0@Z1@Z
    0x006456E4: 4,   # ??_M@YGXPAXIHP6EX0@Z@Z   eh vector destructor iterator
}
DTOR_RE = re.compile(r"^\?\?1(\w+)@@")


@dataclass
class Evidence:
    kind: str
    size: int
    detail: str
    exact: bool = True


@dataclass
class Layout:
    name: str
    ctor: int | None = None
    evidence: list[Evidence] = field(default_factory=list)
    vbase_offset: int | None = None
    vbase_size: int | None = None
    global_address: int | None = None

    def settled(self) -> int | None:
        """The size, when exact evidence exists and agrees.

        Upper bounds never settle anything: they are consistent with every
        size below them, so treating one as an answer is how this tool would
        report 0x718 for a class that is really 0x28.
        """
        sizes = {e.size for e in self.evidence if e.exact}
        if len(sizes) != 1:
            return None
        size = sizes.pop()
        for item in self.evidence:
            if not item.exact and size > item.size:
                return None      # exceeds a bound it must respect
        return size


class Image:
    def __init__(self, path: Path):
        self.pe = pefile.PE(str(path), fast_load=True)
        self.base = self.pe.OPTIONAL_HEADER.ImageBase
        self.sections = [
            (self.base + s.VirtualAddress,
             self.base + s.VirtualAddress + max(s.Misc_VirtualSize, s.SizeOfRawData),
             s.PointerToRawData)
            for s in self.pe.sections]
        text = next(s for s in self.pe.sections
                    if s.Characteristics & 0x20000000)
        self.code = self.pe.__data__[
            text.PointerToRawData:text.PointerToRawData + text.SizeOfRawData]
        self.code_start = self.base + text.VirtualAddress
        self.engine = Cs(CS_ARCH_X86, CS_MODE_32)
        self.engine.detail = True

    def read(self, address: int, length: int) -> bytes:
        for begin, end, raw in self.sections:
            if begin <= address < end:
                offset = raw + (address - begin)
                return self.pe.__data__[offset:offset + length]
        return b""

    def dword(self, address: int) -> int | None:
        data = self.read(address, 4)
        return struct.unpack("<I", data)[0] if len(data) == 4 else None

    def disasm(self, address: int, length: int):
        return self.engine.disasm(self.read(address, length), address)


def load_functions() -> tuple[dict[int, dict], dict[str, int]]:
    by_address, by_name = {}, {}
    with FUNCTIONS_CSV.open() as handle:
        for row in csv.DictReader(handle):
            try:
                address = int(row["address"], 16)
            except (KeyError, ValueError):
                continue
            by_address[address] = row
            by_name[row["name"]] = address
    return by_address, by_name


def load_pinned() -> dict[str, int]:
    pinned = {}
    for header in sorted(SRC_DIR.glob("*.h")):
        for match in PINNED_RE.finditer(header.read_text()):
            pinned[match.group(1)] = int(match.group(2), 0)
    return pinned


def constructors(by_address: dict[int, dict], name: str) -> list[int]:
    """Constructor addresses for a class, most-referenced first."""
    found = []
    for address, row in by_address.items():
        match = CTOR_RE.match(row["name"])
        if match and match.group(1) == name:
            found.append((int(row.get("caller_count") or 0), address))
    return [address for _, address in sorted(found, reverse=True)]


def call_targets(image: Image, address: int, length: int) -> list[tuple[int, int]]:
    """(site, target) for every direct call in a range."""
    calls = []
    for instruction in image.disasm(address, length):
        if instruction.mnemonic == "call":
            for operand in instruction.operands:
                if operand.type == X86_OP_IMM:
                    calls.append((instruction.address, operand.imm & 0xFFFFFFFF))
    return calls


def scan_callers(image: Image, target: int) -> list[int]:
    sites = []
    for instruction in image.engine.disasm(image.code, image.code_start):
        if instruction.mnemonic in ("call", "jmp"):
            for operand in instruction.operands:
                if operand.type == X86_OP_IMM and (operand.imm & 0xFFFFFFFF) == target:
                    sites.append(instruction.address)
    return sites


def first_constructed(image: Image, ctor: int, length: int,
                      by_address: dict[int, dict]) -> tuple[str, int, bool] | None:
    """(class, offset, is_member) for the first object this constructor builds.

    A base class is constructed on an unadjusted `this`; a member is
    constructed on `this` plus its offset. The instruction that distinguishes
    them is the `lea ecx, [reg + N]` in between, and reading only which
    constructor runs first cannot tell them apart.

    That mattered: FileWin, StatusWin, InfoWin, and BattleWin were all
    declared here as deriving from the class their constructor calls into,
    when in every case it is a member sitting at a nonzero offset -
    FlatButton at 0x660, Caviar at 0x30, Time at 0x30. Declaring a member as a
    base puts it at offset zero and moves every field after it.
    """
    offset = 0
    for instruction in image.disasm(ctor, length):
        if instruction.mnemonic == "lea" and instruction.op_str.startswith("ecx,"):
            match = re.match(r"ecx, \[e\w\w \+ (0x[0-9a-f]+)\]", instruction.op_str)
            offset = int(match.group(1), 16) if match else offset
        elif instruction.mnemonic == "call":
            for operand in instruction.operands:
                if operand.type != X86_OP_IMM:
                    continue
                row = by_address.get(operand.imm & 0xFFFFFFFF)
                if not row:
                    continue
                match = CTOR_RE.match(row["name"])
                if match:
                    return match.group(1), offset, offset != 0
    return None


def owning_function(by_address: dict[int, dict], site: int) -> dict | None:
    starts = sorted(by_address)
    index = bisect.bisect_right(starts, site) - 1
    if index < 0:
        return None
    owner = by_address[starts[index]]
    size = int(owner.get("size") or 0)
    return owner if site < starts[index] + size else None


def allocation_size(image: Image, ctor: int,
                    by_address: dict[int, dict]) -> Evidence | None:
    """A `push N` feeding operator new whose result this constructor builds.

    The size is only this class's when the pointer being constructed is the
    allocation itself. Two things break that, and both are rejected: a call
    site inside another constructor or allocator, where the `new` belongs to
    the object under construction rather than to this one, and an adjusted
    `this` (a member subobject at a nonzero offset).
    """
    for site in scan_callers(image, ctor):
        owner = owning_function(by_address, site)
        if owner is None:
            continue
        name = owner["name"]
        if name.startswith("??0") or name.startswith("?alloc@"):
            # The allocation here is the enclosing object's, not ours.
            continue

        window = max(image.code_start, site - 0x40)
        pushed = size = None
        adjusted = False
        for instruction in image.disasm(window, site - window):
            if instruction.mnemonic == "push":
                for operand in instruction.operands:
                    if operand.type == X86_OP_IMM:
                        pushed = operand.imm & 0xFFFFFFFF
            elif instruction.mnemonic == "lea" and \
                    instruction.op_str.startswith("ecx,"):
                # `this` is being offset, so this is a subobject.
                adjusted = True
            elif instruction.mnemonic == "call":
                target = None
                for operand in instruction.operands:
                    if operand.type == X86_OP_IMM:
                        target = operand.imm & 0xFFFFFFFF
                if target == OPERATOR_NEW and pushed:
                    size, adjusted = pushed, False
                elif target is not None and size is not None:
                    other = by_address.get(target, {}).get("name", "")
                    if CTOR_RE.match(other):
                        # Another constructor claimed the allocation first.
                        size = None
        if size and not adjusted and 0 < size < 0x400000:
            return Evidence("allocation", size,
                            f"operator new({size}) constructed directly by "
                            f"0x{ctor:08X} in {name[:40]}")
    return None


def global_objects(image: Image, by_address: dict[int, dict]) -> list[tuple[int, str, int]]:
    """(address, initializer name, constructor) for every global with a ctor."""
    found = []
    for address, row in by_address.items():
        if not INITIALIZER_RE.match(row["name"]):
            continue
        target, this = None, None
        for instruction in image.disasm(address, int(row.get("size") or 64)):
            if instruction.mnemonic == "mov" and instruction.op_str.startswith("ecx,"):
                for operand in instruction.operands:
                    if operand.type == X86_OP_IMM:
                        this = operand.imm & 0xFFFFFFFF
            elif instruction.mnemonic == "call":
                for operand in instruction.operands:
                    if operand.type == X86_OP_IMM:
                        target = operand.imm & 0xFFFFFFFF
                break
        if this and target:
            found.append((this, row["name"], target))
    return sorted(found)


def global_bound(image: Image, ctor: int,
                 globals_: list[tuple[int, str, int]]) -> Evidence | None:
    for index, (address, name, target) in enumerate(globals_):
        if target != ctor:
            continue
        if index + 1 >= len(globals_):
            return None
        following, next_name, _ = globals_[index + 1]
        return Evidence("global", following - address,
                        f"{name} at 0x{address:08X}, bounded above by "
                        f"{next_name} at 0x{following:08X}",
                        exact=False)
    return None


def virtual_base(image: Image, ctor: int, length: int,
                 pinned: dict[str, int],
                 by_address: dict[int, dict]) -> tuple[int | None, Evidence | None, str]:
    """Read the vbtable a constructor installs, and add a pinned base size.

    MSVC stores the virtual base offset in vbtable[1]; the constructor loads
    the vbtable pointer into the object's first slot, so the immediate moved
    to [this] is the vbtable address.
    """
    vbtable = None
    for instruction in image.disasm(ctor, length):
        if instruction.mnemonic != "mov":
            continue
        operands = instruction.operands
        if len(operands) == 2 and operands[1].type == X86_OP_IMM:
            candidate = operands[1].imm & 0xFFFFFFFF
            destination = operands[0]
            if destination.type == X86_OP_MEM and destination.mem.disp == 0:
                probe = image.dword(candidate + 4)
                if image.dword(candidate) == 0 and probe and 0 < probe < 0x400000:
                    vbtable = candidate
                    break
    if vbtable is None:
        return None, None, ""

    offset = image.dword(vbtable + 4)
    # The base is whichever pinned class the constructor delegates to; its
    # constructor is called with the most-derived flag before anything else.
    base_name = ""
    for _, target in call_targets(image, ctor, length):
        row = by_address.get(target)
        if not row:
            continue
        match = CTOR_RE.match(row["name"])
        if match and match.group(1) in pinned:
            base_name = match.group(1)
            break
    if not base_name:
        return offset, None, ""
    size = pinned[base_name]
    total = offset + size

    # Adding the base size to its offset is only the whole object when the
    # virtual base is genuinely last. ListBox constructs a Dialog member at
    # 0xA60, past the 0xA5C that sum gives, so for that class the sum is not a
    # size at all. Anything the constructor touches at or beyond the computed
    # end proves the object continues, and no size is reported.
    for instruction in image.disasm(ctor, length):
        match = re.search(r"\[e(?:si|bx|di|cx|ax) \+ (0x[0-9a-f]+)\]",
                          instruction.op_str)
        if match and int(match.group(1), 16) >= total:
            return offset, None, ""

    return offset, Evidence(
        "virtual", total,
        f"vbtable 0x{vbtable:08X} puts the virtual base at +0x{offset:X}, "
        f"plus pinned sizeof({base_name}) == 0x{size:X}, and the constructor "
        f"touches nothing at or past 0x{total:X}"), base_name


def vector_iterator_sizes(image: Image,
                          by_address: dict[int, dict]) -> dict[str, set[int]]:
    """class -> element sizes read off every vector-iterator call site.

    Returns a SET per class on purpose. Two call sites naming the same class
    with different sizes means one of them is describing something else, and
    the caller must refuse rather than pick; settled() already enforces that,
    but keeping the ambiguity visible here is what makes it enforceable.
    """
    sizes: dict[str, set[int]] = {}
    # PER FUNCTION, not one linear sweep of .text. A linear decode
    # desynchronises on data and jump tables: it finds 24 of the 212 iterator
    # call sites in this image and resolves none of them, because the `push`
    # run before each call is misdecoded. Every function's start and length is
    # already known, so decode from those.
    instructions = []
    for start in sorted(by_address):
        length = int(by_address[start].get("size") or 0)
        if length:
            instructions.extend(image.disasm(start, length))
    for index, instruction in enumerate(instructions):
        if instruction.mnemonic != "call" or not instruction.operands:
            continue
        operand = instruction.operands[0]
        if operand.type != X86_OP_IMM:
            continue
        arity = VECTOR_ITERATORS.get(operand.imm & 0xFFFFFFFF)
        if arity is None:
            continue
        pushed: list[int | None] = []
        for previous in reversed(instructions[max(0, index - 12):index]):
            if previous.mnemonic != "push":
                break
            source = previous.operands[0]
            pushed.append(source.imm & 0xFFFFFFFF
                          if source.type == X86_OP_IMM else None)
        pushed.reverse()
        if len(pushed) < arity:
            continue          # a register-computed argument; say nothing
        arguments = pushed[-arity:]
        size = arguments[-2]
        if not size:
            continue
        # The ctor is third from the end for the 5-argument form; the dtor is
        # first in both. Either names the class.
        for candidate in {arguments[0], arguments[-4] if arity == 5 else None}:
            if not candidate:
                continue
            name = (by_address.get(candidate) or {}).get("name", "")
            found = CTOR_RE.match(name) or DTOR_RE.match(name)
            if found:
                sizes.setdefault(found.group(1), set()).add(size)
    return sizes


def derive(image: Image, name: str, by_address: dict[int, dict],
           pinned: dict[str, int],
           globals_: list[tuple[int, str, int]],
           strides: dict[str, set[int]] | None = None) -> Layout:
    layout = Layout(name)
    found = constructors(by_address, name)
    if not found:
        return layout
    layout.ctor = found[0]
    length = int(by_address[layout.ctor].get("size") or 256)

    allocation = allocation_size(image, layout.ctor, by_address)
    if allocation:
        layout.evidence.append(allocation)

    stride = (strides or {}).get(name)
    if stride and len(stride) == 1:
        size = next(iter(stride))
        layout.evidence.append(Evidence(
            "array", size,
            "element size passed to an EH vector iterator"))

    for candidate in found:
        bound = global_bound(image, candidate, globals_)
        if bound:
            layout.evidence.append(bound)
            break

    offset, evidence, base = virtual_base(
        image, layout.ctor, length, pinned, by_address)
    layout.vbase_offset = offset
    if evidence:
        layout.evidence.append(evidence)
        layout.vbase_size = pinned.get(base)
    return layout


def report(layout: Layout, pinned: dict[str, int]) -> None:
    print(f"\n{layout.name}")
    if layout.ctor is None:
        print("  no constructor found; the class may be abstract or inlined")
        return
    print(f"  constructor 0x{layout.ctor:08X}")
    if layout.vbase_offset is not None:
        print(f"  virtual base at +0x{layout.vbase_offset:X}")
    if not layout.evidence:
        print("  no size evidence found")
        return
    for item in layout.evidence:
        kind = item.kind if item.exact else f"{item.kind} <="
        print(f"  [{kind:12}] 0x{item.size:X} ({item.size})  {item.detail}")
    settled = layout.settled()
    known = pinned.get(layout.name)
    if settled is None:
        print("  no exact evidence; not enough to pin")
    else:
        print(f"  => sizeof({layout.name}) == 0x{settled:X}")
    if known is not None:
        verdict = "matches" if settled == known else "DIFFERS FROM"
        print(f"  {verdict} the pinned 0x{known:X}")


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("classes", nargs="*", help="class names, e.g. Console")
    parser.add_argument("--check-pinned", action="store_true",
                        help="replay the derivation against every hand-pinned "
                             "class and summarise agreement")
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    args = parser.parse_args()

    image = Image(args.exe)
    by_address, _ = load_functions()
    pinned = load_pinned()
    globals_ = global_objects(image, by_address)
    # Scanned once: the sweep is over the whole .text and every class asks the
    # same question of it.
    strides = vector_iterator_sizes(image, by_address)

    names = args.classes
    if args.check_pinned:
        names = sorted(pinned)
    if not names:
        parser.error("name at least one class, or pass --check-pinned")

    agree = differ = silent = 0
    for name in names:
        layout = derive(image, name, by_address, pinned, globals_, strides)
        if args.check_pinned:
            settled = layout.settled()
            if settled is None:
                # Abstaining is a correct outcome, not a failure: the tool is
                # only dangerous when it answers, and answers wrongly.
                silent += 1
                continue
            if settled == pinned.get(name):
                agree += 1
                continue
            differ += 1
        report(layout, pinned)

    if args.check_pinned:
        total = agree + differ + silent
        print(f"\n{total} pinned classes replayed: {agree} reproduced, "
              f"{silent} abstained, {differ} answered wrongly")
        if differ:
            print("a wrong answer is the one failure mode that matters here")
        return 1 if differ else 0
    return 0


if __name__ == "__main__":
    sys.exit(main())
