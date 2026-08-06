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
from capstone.x86 import X86_OP_MEM, X86_REG_ECX  # noqa: E402

import derive_class_layout  # noqa: E402
import recovery_symbols  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"
FUNCTIONS = REPO_ROOT / "docs" / "recovery" / "functions.csv"
IDB_MEMBERS = REPO_ROOT / "docs" / "recovery" / "idb-members.csv"
OUTPUT = REPO_ROOT / "docs" / "recovery" / "access-lower-bounds.csv"

SCOPE_RE = re.compile(r"^\?{1,2}[~\w@]*?@(\w+)@@")
# Every spelling of ECX. Writing any of them ends its life as the receiver.
ECX_NAMES = frozenset({"ecx", "cx", "cl", "ch"})
THISCALL = "__thiscall"


def receivers() -> dict:
    """{class: [(address, size, mangled name)]} for its thiscall methods."""
    found = collections.defaultdict(list)
    if not FUNCTIONS.is_file():
        return found
    with FUNCTIONS.open(newline="", encoding="utf-8-sig") as handle:
        for row in csv.DictReader(handle):
            name = row.get("name") or ""
            match = SCOPE_RE.match(name)
            if not match:
                continue
            if not recovery_symbols.is_nonstatic_member(name):
                continue
            if recovery_symbols.convention_of(name) != THISCALL:
                continue
            try:
                address = int(row["address"], 16)
                size = int(row["size"])
            except (ValueError, KeyError, TypeError):
                continue
            if size > 0:
                found[match.group(1)].append((address, size, name))
    return found


def reach(image, engine, address: int, size: int) -> tuple:
    """(highest byte touched through ECX, the instruction that touched it)."""
    start = address - image.code_start
    if start < 0 or start + size > len(image.code):
        return 0, ""
    best, evidence = 0, ""
    for instruction in engine.disasm(image.code[start:start + size], address):
        for operand in instruction.operands:
            if (operand.type != X86_OP_MEM
                    or operand.mem.base != X86_REG_ECX
                    or operand.mem.index != 0
                    or operand.mem.disp < 0):
                continue
            end = operand.mem.disp + (operand.size or 4)
            if end > best:
                best = end
                evidence = (f"{instruction.mnemonic} {instruction.op_str}"
                            f" at 0x{instruction.address:08X}")
        written = {engine.reg_name(register)
                   for register in instruction.regs_access()[1]}
        if written & ECX_NAMES:
            break
    return best, evidence


def bounds(exe: Path) -> dict:
    image = derive_class_layout.Image(exe)
    engine = Cs(CS_ARCH_X86, CS_MODE_32)
    engine.detail = True
    found = {}
    for name, methods in receivers().items():
        best, evidence = 0, ""
        for address, size, _ in methods:
            reached, why = reach(image, engine, address, size)
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
    """Source totals the image contradicts."""
    out = []
    for name, total in sorted(idb_totals().items()):
        if name in found and found[name][0] > total:
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
