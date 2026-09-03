#!/usr/bin/env -S uv run python
"""Bodies the image could not have got from this compiler.

Some of the shipped code was inline assembly in the 1999 source. No spelling of
C++ reaches it, so an agent handed one of those addresses grinds until it gives
up - `Buffer::vline` cost a pass before its `loop` instruction was noticed.

The tells, in decreasing confidence:

  loop/loope/loopne  VC6 does not emit these. It counts down with `dec`/`jne`.
  lodsb/stosb/scasb  without a `rep` prefix - the compiler only uses the string
                     instructions for block moves, and always with `rep`.
  sahf/lahf/pushf/popf, xchg, bswap, xlat, cbw/cwd variants used bare, and any
  segment prefix. Compilers do emit some of these; they are reported separately
  as WEAK so a real `loop` is not buried under them.

Writing a high-byte register (`ah`, `bh`, `ch`, `dh`) as a destination is a
fourth tell, listed WEAK: VC6 does use them, but rarely as a store source.

    uv run tools/handwritten_asm.py [--weak]

This does NOT say a body is impossible - it says the shipped bytes are not this
compiler's output for any source, so the honest ceiling is semantic equivalence.
"""

from __future__ import annotations

import concurrent.futures
import os
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from decomp import read
from decomp.asm import original_asm

REPO_ROOT = Path(__file__).resolve().parent.parent
IMAGE = Path(os.environ.get(
    "OPENSMACX_IMAGE",
    REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"))

STRONG = {"loop", "loope", "loopne", "loopz", "loopnz",
          "lodsb", "lodsw", "lodsd", "scasb", "scasw", "scasd", "xlatb", "xlat",
          # SAVING THE FLAGS is something a compiler never needs to do: it owns
          # the flags between the instruction that sets them and the one that
          # reads them. `pushf`/`popf` means a human wrote a block and defended
          # it. These were filed WEAK at first and `Buffer::fill` slipped
          # through - its image body is an `__asm` block at 0x005DFC43 that
          # does `pushf`, `cld`, `rep stosd`, `popf` and clobbers ebx, a
          # callee-saved register, without saving it. The body's own comment
          # said so; the tool did not.
          # NOT `sahf`/`lahf`: those ARE standard in floating-point compares
          # and promoting them buried the real hits under twenty CRT math
          # helpers - `__powhlp`, `__hypothlp`, `__fpclass` and friends, where
          # Microsoft's own hand-written assembly is not this recovery's
          # problem. They stay weak.
          "pushf", "pushfd", "popf", "popfd",
          # THE CARRY-CHAIN BIT COUNT, added 2026-09-03 on general.cpp's
          # bit_count: its `shr ecx,1 / adc eax,0` loop sums the carry flag
          # straight into the count, and cl 12.00.8168 CANNOT reach it -
          # twelve probe spellings all emit `mov edx,ecx; and edx,1; add
          # eax,edx; shr ecx,1` instead, because C has no name for the flag
          # register. `adc` with an immediate zero operand is the tell: the
          # compiler only emits adc with a register or memory operand it
          # computed itself. Not bare `adc` STRONG - only the immediate-0
          # form - and `sbb` likewise, so real adds-with-carry in codegen
          # (multiword arithmetic, `neg/sbb` idioms) are not buried.
          "adc-eax-0", "sbb-eax-0"}
WEAK = {"sahf", "lahf",
        "xchg", "bswap", "cbw", "cwde", "cdq", "cwd", "salc", "aaa", "aad"}


# ONE read of the tree, not one per body. The first draft called `read()`
# inside the worker, so scanning 3,000 bodies re-parsed 1,000 files 3,000
# times and never finished. Disassembly is the cheap half.
def scan(record):
    address = record.address
    try:
        listing = original_asm(record, IMAGE)
    except Exception as error:                       # noqa: BLE001
        return (address, record.name, "ERROR", str(error)[:60])
    strong = sorted({m for m in listing.mnemonics if m in STRONG})
    weak = sorted({m for m in listing.mnemonics if m in WEAK})
    # THE CARRY-CHAIN TELL, whose STRONG entry is a shape rather than a
    # mnemonic: `adc <anything>, 0` (and `sbb ..., 0`) reads the flag
    # register C has no name for, so cl 12.00.8168 cannot emit it. The
    # mnemonics view carries no operands, so this reads the rendered lines.
    lines = listing.lines
    for index, line in enumerate(lines):
        parts = line.split()
        # THE DUFF'S-DEVICE DISPATCH, found 2026-09-03 on Buffer::copy_mask
        # (0x005E2B00): a `lea reg, <case>` immediately followed by a jump
        # through that SAME register is a computed dispatch VC6 never emits
        # - its switches use jump tables addressed directly, and nothing it
        # synthesises is a two-instruction computed goto. The shipped
        # binary carries the FOLDED form: a dead lea of the target with a
        # direct jmp beside it. copy_mask keeps
        # its case counter and row remainders in fixed .data beside it. The
        # lea-then-jmp-through-it window is what keeps ordinary address
        # arithmetic out.
        if index + 1 < len(lines):
            here = lines[index].split()
            nxt = lines[index + 1].split()
            if "lea" in here and "jmp" in nxt:
                # THE FOLDED FORM the shipped binary actually carries: the
                # lea computes an address and the next instruction jumps to
                # that SAME address directly - a dead lea VC6 never emits
                # (its address computations always feed something). The
                # unfolded `jmp reg` form matches too, for bodies whose
                # dispatch survived folding.
                lea_at = here.index("lea")
                jmp_at = nxt.index("jmp")
                if (lea_at + 2 < len(here) and jmp_at + 1 < len(nxt)):
                    target = nxt[jmp_at + 1]
                    folded = (here[lea_at + 2].lstrip("[").rstrip("]") == target)
                    through = (here[lea_at + 1].rstrip(",") == target)
                    if folded or through:
                        strong.append("lea/jmp-dispatch")
        # THE MNEMONIC SITS AFTER THE ADDRESS AND THE HEX-BYTE TOKENS, which
        # split into one token each - so index into it by membership, not
        # position: `adc eax, 0` is ['0xADDR', '83', 'D0', '00', 'adc',
        # 'eax,', '0'].
        for carry in ("adc", "sbb"):
            if carry in parts and parts[-1] == "0":
                strong.append(carry + " ...,0")
    strong = sorted(set(strong))
    return (address, record.name, ",".join(strong), ",".join(weak))


if __name__ == "__main__":
    records = read(REPO_ROOT / "src")
    # A body with no spans has nothing to disassemble.
    subjects = [r for r in records if r.image_spans and not r.byte_exact]
    print(f"scanning {len(subjects):,} unclaimed bodies", flush=True)
    rows = [scan(r) for r in subjects]

    strong = [r for r in rows if r[2] and r[2] != "ERROR"]
    weak = [r for r in rows if not r[2] and r[3]]
    for address, name, tells, _ in sorted(strong):
        print(f"  0x{address:08X}  {tells:20s} {name}")
    if "--weak" in sys.argv:
        print("\nWEAK - compilers do emit these; check before believing:")
        for address, name, _, tells in sorted(weak):
            print(f"  0x{address:08X}  {tells:20s} {name}")
    print(f"\n{len(strong)} body(s) with a STRONG tell, {len(weak)} with only weak ones")
