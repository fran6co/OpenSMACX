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
          "lodsb", "lodsw", "lodsd", "scasb", "scasw", "scasd", "xlatb", "xlat"}
WEAK = {"sahf", "lahf", "pushf", "pushfd", "popf", "popfd",
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
