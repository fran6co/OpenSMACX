#!/usr/bin/env python3
"""How big a class must be, read off the code that indexes into it.

The largest `[ecx+N]` a class's own methods reach, plus the width of that
access, is a hard LOWER bound on the class size: the object has to contain the
byte the code reads. This is the one direction the evidence already in the tree
does not cover. `derive_class_size_bounds.py` measures to the next constructed
global and gives an UPPER bound; the IDB sums its member table and gives a
lower one that is usually exact. Both of the IDB's known failures - `PullDown`
recording one member of 0xa14 against a true 0xf40, `Console` four bytes short
- UNDER-report, and an upper bound is structurally blind to that.

WHAT IT IS FOR. Not for pinning a size. Measured as a control, the pairing that
would pin one - lower meeting upper - holds for **0 of the 34 bounded classes**,
which is the same answer `docs/ORACLE_SESSIONS.md` recorded for the IDB's lower
bound against the same upper bound. That route is closed, and it is closed by
measurement rather than by argument.

What it does instead is REFUSE. A source that claims a class is 0xa14 bytes
while the code reads its 0xf35th byte is contradicted by the image, and no
amount of parsing rescues it - the members were never entered. On the four
classes where a source total is falsified this way, `PullDown` is one, which is
the control: the known-truncated entry is caught without being told about.

WHY IT CANNOT OVERSTATE. Three deliberate restrictions, each of which can only
lower the answer:

  * Only `__thiscall` non-static members are read. `recovery_symbols` reads
    both facts straight off the mangled name; a `QAA` or `QAG` member takes its
    receiver on the stack and a static member has none, so ECX there is not
    `this` and its displacements belong to something else entirely.
  * Only accesses BEFORE the first write to ECX. After `mov ecx, eax` the
    register is no longer the receiver, and following it would need dataflow
    this does not have.
  * Only non-negative displacements. A negative one is an adjusted `this`
    pointing into a base subobject, and its offsets are that base's.

So the bound is loose - 97 classes get one at all, and on the 40 whose size is
pinned it reaches the true size for none of them. It is never wrong, which is
the property that matters: measured against those 40, it exceeds the real size
**0 times**.
"""

from __future__ import annotations

import argparse
import collections
import csv
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from capstone import CS_ARCH_X86, CS_MODE_32, Cs  # noqa: E402
from capstone.x86 import (X86_INS_ADD, X86_INS_LEA, X86_INS_MOV,  # noqa: E402
                          X86_OP_MEM, X86_OP_REG, X86_OP_IMM)

import derive_class_layout  # noqa: E402
import recovery_symbols  # noqa: E402


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
IDB_MEMBERS = REPO_ROOT / "docs" / "recovery" / "idb-members.csv"
OUTPUT = REPO_ROOT / "docs" / "recovery" / "access-lower-bounds.csv"

# `?name@Class@@` and the special-name family `??0Class@@`, `??1`, `??_G`.
# The trailing `([\w_]+)@@` must match a NON-EMPTY scope, which is what refuses
# a free function: `?f@@YAXPAUGraphicWin@@@Z` has nothing between the name and
# the `@@`. A lazy `[~\w@]*?@(\w+)@@` does NOT refuse it - it consumes `f@`,
# then reads the convention code and the parameter type as a class, inventing
# scopes called `YAXPAUGraphicWin` and `YAHHHHHHPAUCaviar`.
#
# The two shapes differ in where the class sits. An ordinary method spells
# `?name@Class@@`, so the class follows the name. A SPECIAL name has no name
# segment at all - `??0Buffer@@` is `??`, the operator code `0`, and then the
# class - so a pattern written for the first shape silently matches neither.
RECEIVER_RE = re.compile(
    r"^(?:\?\?(?:_[A-Z]|[0-9A-Z])|\?[\w_]+@)"      # special code, or name@
    r"([\w_]+)(?:@[\w_]+)*@@"                       # class, innermost first
    r"([A-Z])([A-Z])([A-Z])")                       # access, CV, convention
# Every register spelling collapsed onto its 32-bit name, so that writing `cl`
# is seen to destroy `ecx`.
WHOLE_REGISTER = {}
for _wide, _parts in (("eax", "ax al ah"), ("ebx", "bx bl bh"),
                      ("ecx", "cx cl ch"), ("edx", "dx dl dh"),
                      ("esi", "si sil"), ("edi", "di dil"),
                      ("ebp", "bp bpl"), ("esp", "sp spl")):
    WHOLE_REGISTER[_wide] = _wide
    for _part in _parts.split():
        WHOLE_REGISTER[_part] = _wide
THISCALL = "__thiscall"
# The convention code sits after the access and CV codes. `E`/`F` are thiscall.
THISCALL_CODES = frozenset("EF")


def receiver_scope(mangled: str) -> str:
    """The class whose `this` arrives in ECX, or '' when there is none.

    `recovery_symbols.is_nonstatic_member` answers this for ordinary methods
    and refuses the special-name family: its MEMBER_ACCESS is `^\\?[\\w_]+@...`,
    and `??0Buffer@@QAE@XZ` begins `??0`. Constructors were therefore excluded
    from every bound below, which is the worst possible omission here - a
    constructor writes the WHOLE object, vtable and every field, so it reaches
    further into the class than anything else does.
    """
    found = RECEIVER_RE.match((mangled or "").strip())
    if not found:
        return ""
    scope, access, _cv, convention = found.groups()
    if access not in recovery_symbols.NONSTATIC_MEMBER:
        return ""
    if convention not in THISCALL_CODES:
        return ""
    return scope


def receivers() -> dict:
    """{class: [(address, size, mangled name)]} for its thiscall methods."""
    found = collections.defaultdict(list)
    # `src/` is the catalogue's store; the export is deleted. SORTED BY
    # ADDRESS, because the witness instruction recorded beside each bound is
    # whichever qualifying access is found first, and that made the output
    # depend on the store's iteration order: moving from a CSV to `src/` left
    # all 130 bounds identical and rewrote 36 of their witnesses, so the gate
    # reported the file stale without a single bound having moved.
    for row in sorted(_catalogue_rows(),
                      key=lambda r: int(r.get("address") or "0", 16)):
        name = row.get("name") or ""
        scope = receiver_scope(name)
        if not scope:
            continue
        try:
            address = int(row["address"], 16)
            size = int(row["size"])
        except (ValueError, KeyError, TypeError):
            continue
        if size > 0:
            found[scope].append((address, size, name))
    return found


def object_ceiling(image) -> int:
    """No object is bigger than the image that holds its code and its data.

    A crude ceiling, and it catches a real failure. `?check@MainMenu@@QAEXH@Z`
    is 4,782 bytes with a jump table inline in `.text`; a linear disassembly
    desynchronises on the table and everything after it is fiction. One such
    fiction was

        movsx eax, word ptr [ecx + 0x95282a]

    whose displacement is an ABSOLUTE ADDRESS with ECX holding an index into a
    global, not a member offset - and it put MainMenu at 9.7MB. That is past
    the end of a 6MB image, so it cannot be an offset into anything.
    """
    return image.pe.OPTIONAL_HEADER.SizeOfImage


def reach(image, engine, address: int, size: int, ceiling: int = 0) -> tuple:
    """(highest byte touched through `this`, and the instruction that did it).

    `this` arrives in ECX and almost never stays there. Every one of these
    bodies moves it out in the prologue:

        Patch::Patch      mov esi, ecx ; mov [esi+4], 0x7f
        Fractal::init     mov ebx, ecx ; lea esi, [ebx + 0x169]

    Reading ECX alone and stopping at the first write to it therefore saw
    nothing at all in those functions, and reported four classes as having no
    bound when the image proves otherwise. So `this` is followed through the
    copies that carry it: register-to-register moves, and `lea` of a member's
    address, which is how a by-value member is reached.

    The tracking is a linear walk with no notion of branches, which is safe in
    the only direction that matters: ANY write to a tracked register drops it,
    so a register reassigned on a path this does not model stops being trusted
    rather than being followed to a wrong offset. It can therefore still
    under-report, and the control - it must never exceed a pinned size -
    remains the check that it does not over-report.
    """
    start = address - image.code_start
    if start < 0 or start + size > len(image.code):
        return 0, ""
    # {register name: its offset from `this`}
    carries = {"ecx": 0}
    best, evidence = 0, ""

    for instruction in engine.disasm(image.code[start:start + size], address):
        # `lea` COMPUTES AN ADDRESS AND TOUCHES NO MEMORY, so its operand is
        # not evidence that the byte is inside the object - it is arithmetic as
        # often as it is a member's address. Counting it put `Win` at 0x8f8
        # against a real 0x444, off the back of `lea ecx, [esi + 0x8f4]`. It
        # still PROPAGATES below, so a real access through the result counts.
        for operand in ([] if instruction.id == X86_INS_LEA
                        else instruction.operands):
            if operand.type != X86_OP_MEM or operand.mem.index != 0:
                continue
            base = WHOLE_REGISTER.get(engine.reg_name(operand.mem.base) or "")
            if base not in carries:
                continue
            offset = carries[base] + operand.mem.disp
            if offset < 0:
                # An adjusted `this` pointing into a base subobject; those
                # offsets belong to the base, not to this class.
                continue
            end = offset + (operand.size or 4)
            if ceiling and end > ceiling:
                continue
            if end > best:
                best = end
                evidence = (f"{instruction.mnemonic} {instruction.op_str}"
                            f" at 0x{instruction.address:08X}")

        # What this instruction makes into a new carrier, computed BEFORE the
        # writes below retire the old one - `mov esi, ecx` reads and writes in
        # the same breath.
        gained = None
        operands = instruction.operands
        if len(operands) == 2 and operands[0].type == X86_OP_REG:
            destination = WHOLE_REGISTER.get(
                engine.reg_name(operands[0].reg) or "")
            if destination:
                if (instruction.id == X86_INS_MOV
                        and operands[1].type == X86_OP_REG):
                    source = WHOLE_REGISTER.get(
                        engine.reg_name(operands[1].reg) or "")
                    if source in carries:
                        gained = (destination, carries[source])
                elif (instruction.id == X86_INS_LEA
                        and operands[1].type == X86_OP_MEM
                        and operands[1].mem.index == 0):
                    source = WHOLE_REGISTER.get(
                        engine.reg_name(operands[1].mem.base) or "")
                    if source in carries:
                        gained = (destination,
                                  carries[source] + operands[1].mem.disp)
                # `add reg, imm` is deliberately NOT propagated. Capstone hands
                # back the immediate of `add esi, 0xFFFFB1C4` in a form that
                # made a subtraction read as a 4GB offset, and Console came out
                # at 0xFFFFB1C4 against a real 0x247A8. Dropping the register is
                # the conservative reading and costs nothing measurable.

        for register in instruction.regs_access()[1]:
            whole = WHOLE_REGISTER.get(engine.reg_name(register) or "")
            carries.pop(whole, None)
        if gained and gained[1] >= 0:
            carries[gained[0]] = gained[1]
        if not carries:
            break
    return best, evidence


def bounds(exe: Path) -> dict:
    image = derive_class_layout.Image(exe)
    engine = Cs(CS_ARCH_X86, CS_MODE_32)
    engine.detail = True
    ceiling = object_ceiling(image)
    found = {}
    for name, methods in receivers().items():
        best, evidence = 0, ""
        for address, size, _ in methods:
            reached, why = reach(image, engine, address, size, ceiling)
            if reached > best:
                best, evidence = reached, why
        if best:
            found[name] = (best, evidence)
    return found


def idb_totals() -> dict:
    """{class: the size its member table sums to}."""
    totals = collections.defaultdict(int)
    if not IDB_MEMBERS.is_file():
        return totals
    with IDB_MEMBERS.open(newline="", encoding="utf-8-sig") as handle:
        for row in csv.DictReader(handle):
            try:
                end = int(row["offset"], 16) + int(row["size"])
            except (ValueError, KeyError, TypeError):
                continue
            totals[row["class"]] = max(totals[row["class"]], end)
    return totals


def render(found: dict) -> str:
    import io

    buffer = io.StringIO()
    writer = csv.writer(buffer, lineterminator="\n")
    writer.writerow(["class", "lower_bound", "evidence"])
    for name in sorted(found):
        size, evidence = found[name]
        writer.writerow([name, f"0x{size:X}", evidence])
    return buffer.getvalue()


def control(found: dict) -> int:
    """Does this ever exceed a size the tree already knows? It must not."""
    pinned = derive_class_layout.load_pinned()
    over = [(name, found[name][0], pinned[name]) for name in pinned
            if name in found and found[name][0] > pinned[name]]
    checked = [name for name in pinned if name in found]
    print(f"control: {len(checked)} of {len(pinned)} pinned classes have a "
          f"bound; {len(over)} exceed the real size")
    for name, low, true in over:
        print(f"    WRONG {name}: reaches 0x{low:X}, real size 0x{true:X}")
    return len(over)


def falsified(found: dict) -> list:
    """Source totals the image contradicts.

    THE COMPARISON IS AGAINST THE ALIGNED TOTAL. The IDB's member table sums to
    the last member's end, not to `sizeof`, and MSVC rounds a class up to its
    alignment - `Buffer` sums to 0x585 and really is 0x588. Comparing against
    the raw sum reported every 4-byte-aligned class in the database as
    "contradicted", which is trailing padding rather than a missing member.
    Four bytes of tail padding is the most any of these can hide, so the
    comparison allows exactly that and no more.
    """
    out = []
    for name, total in sorted(idb_totals().items()):
        if name not in found:
            continue
        aligned = (total + 3) & ~3
        if found[name][0] > aligned:
            out.append((name, total, found[name][0], found[name][1]))
    return out


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--out", type=Path, default=OUTPUT)
    parser.add_argument("--check", action="store_true",
                        help="fail if the committed CSV is stale")
    parser.add_argument("--falsify", action="store_true",
                        help="report source totals the image contradicts")
    args = parser.parse_args(argv)

    if not args.exe.is_file():
        print(f"SKIP: {args.exe} is absent. This reads the pinned image.")
        return 0

    found = bounds(args.exe)
    text = render(found)

    if args.check:
        current = args.out.read_text() if args.out.is_file() else ""
        if current != text:
            print(f"{args.out.name} is stale; regenerate it", file=sys.stderr)
            return 1
        # A bound that exceeds a known size is a defect in this tool, and
        # checking staleness without checking soundness would gate the wrong
        # thing.
        if control(found):
            return 1
        print(f"access lower bounds: {len(found)} classes (up to date)")
        return 0

    if args.falsify:
        wrong = falsified(found)
        print(f"{len(wrong)} source total(s) contradicted by the image:")
        for name, total, low, evidence in wrong:
            print(f"    {name}: the IDB sums to 0x{total:X}, the code reaches "
                  f"0x{low:X}")
            print(f"        {evidence}")
        return 0

    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(text)
    print(f"{len(found)} classes bounded below -> {args.out}")
    control(found)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
