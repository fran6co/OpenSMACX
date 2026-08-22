#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# dependencies = ["capstone==5.0.6"]
# ///
r"""Bodies the image enters on a SUBOBJECT, and whether the source agrees.

A method reached through a base's vtable is entered with `this` pointing at
that base subobject, so its first act is to walk back:

    0x0060FB90  mov eax, dword ptr [ecx - 0x1c]     <- CheckBox's GraphicWin at 0x1C

That opening is a fingerprint, and it answers two questions nothing else here
answers.

WHICH METHODS ARE VIRTUAL. The mangled names in this tree are census
RECONSTRUCTIONS - the image is stripped - so `QAE` versus `UAE` proves nothing.
The entry convention does: on 2026-08-22 exactly three CheckBox bodies opened
`mov eax, [ecx - 0x1c]` (~CheckBox, on_dialog_focus, on_mouse_leave), which is
how GraphicWin's real virtuals were identified, while `close()` opened
`mov esi, ecx` on an unadjusted receiver and was correctly ruled out as an
override.

WHETHER A MARKER IS BOUND TO THE RIGHT FUNCTION. 0x00609EC0's marker sat on
`ListBox::destroy()`, a front-relative entry, while the image's body there is
the ADJUSTED one entered at `this = L + 0x48`. Every measurement had been
comparing two different functions, which is why it read as an unrecoverable
stub; re-binding it took it to BYTE_EXACT immediately. That is a wrong BINDING,
not a wrong body, and no other check in this tree looks for it.

    uv run tools/adjusted_entry.py            # every adjusted entry
    uv run tools/adjusted_entry.py --suspect  # only those whose source
                                              # looks front-relative

Exit 0 always: this reports a shape, not a fault.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO_ROOT))

from decomp.asm import original_asm  # noqa: E402
from decomp.reader import read  # noqa: E402

EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"
MARKER = re.compile(r"^// ORIGINAL: (0x[0-9A-Fa-f]{8}) (\S+)", re.M)
# `mov <reg>, [ecx - N]` / `lea <reg>, [ecx - N]` as the FIRST instruction.
ADJUST = re.compile(r"\[ecx - (0x[0-9a-f]+)\]")
# A source function that already knows it is an adjusted entry says so: it is a
# redirect/thunk, or it subtracts the offset itself.
KNOWS = re.compile(r"redirect|thunk|adjust|_entry\b")


# READ THE IMAGE ONCE, not `osmx show` per marker. The first draft shelled out
# for each of ~2,800 markers and would have taken twenty minutes; a tool nobody
# waits for is a tool nobody runs.
def first_instruction(record) -> str:
    try:
        listing = original_asm(record, EXE)
    except (ValueError, KeyError, OSError):
        return ""
    # `lines` renders on demand - the Listing keeps BYTES, deliberately, so
    # that the text is never the record of what a body is.
    for line in listing.lines:
        return line
    return ""


if __name__ == "__main__":
    only_suspect = "--suspect" in sys.argv
    sources = {p.name: p.read_text(errors="replace")
               for p in (REPO_ROOT / "src").glob("*.cpp")}
    records = [r for r in read(REPO_ROOT / "src")
               if r.path.parent.name == "src"]
    print(f"  scanning {len(records):,} marker(s) for an adjusted entry...\n")
    hits = 0
    for record in records:
        name, address = record.path.name, record.address_hex
        symbol = record.name or "(unnamed)"
        text = sources.get(name, "")
        found = ADJUST.search(first_instruction(record))
        if not found:
            continue
        hits += 1
        # Does the source at this marker look like it knows? A redirect or a
        # body that subtracts the same offset is bound correctly.
        at = text.find(f"// ORIGINAL: {address}")
        block = text[at:at + 2400] if at >= 0 else ""
        # A BODY THAT REPRODUCES IS BOUND CORRECTLY, whatever it looks like.
        # That is the only discriminator that cannot be fooled by spelling:
        # the prose heuristic below flagged CheckBox::on_mouse_leave, which is
        # BYTE_EXACT and therefore self-evidently bound to the right function -
        # it simply lets the compiler perform the adjustment instead of writing
        # it out, which is what the conversion to real virtual bases enabled.
        aware = (record.byte_exact or record.semantic
                 or bool(KNOWS.search(block)) or found.group(1) in block)
        if only_suspect and aware:
            continue
        flag = "" if aware else "   <- SOURCE LOOKS FRONT-RELATIVE"
        print(f"  {address}  enters on +{found.group(1):<7} {name:20} "
              f"{symbol[:38]}{flag}")
    print(f"\n  {hits} body(ies) entered on a subobject")
