#!/usr/bin/env python3
"""Every annotated global must agree with the image about its INITIAL value.

A global written `int X;  // 0x00696D14` is zero at run time. If the image
carries 9 at that address, the recovered program is wrong - and NOTHING
else in this tree can tell, because both spellings compile the same load
and the byte comparison sees no difference. That is how `WinFillColour`
shipped as 0 when the image says 9 (2026-08-25), caught only because
tools/image_data.py had just been written.

Two directions, and only one of them is decidable here:

  ZERO-FILLED IN TREE, NON-ZERO IN IMAGE - a defect. The image's own file
  bytes carry a value at that address, so the variable IS initialised and
  the tree dropped it.

  INITIALISED IN TREE, ZERO IN IMAGE - reported but NOT ratcheted. The
  image may legitimately set it at run time, so a tree-side initialiser is
  a guess rather than a contradiction.
"""
from __future__ import annotations

import re
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from image_data import DEFAULT_IMAGE, NotBacked, read  # noqa: E402

# A file-scope definition, at column 0, carrying an address annotation.
DEFINITION = re.compile(
    r"^((?:const\s+)?(?:unsigned\s+|signed\s+)?[A-Za-z_][\w:]*)\s+(\*?)(\w+)"
    r"(\[\d*\])?\s*(?:=\s*([^;]+?))?\s*;[ \t]*//[ \t]*(0x[0-9A-Fa-f]{6,8})",
    re.M)

# Types whose first four bytes are the whole value.
SCALARS = {"int", "unsigned int", "long", "unsigned long", "uint32_t",
           "int32_t", "short", "uint16_t", "bool", "size_t", "DWORD"}

CEILING = 4
NEUTRAL = {"0", "nullptr", "NULL", "{}", "{0}", "0u", "false"}


def scan(src: Path = Path("src"), image: Path = DEFAULT_IMAGE):
    dropped, guessed = [], []
    for path in sorted(src.glob("*.cpp")):
        for m in DEFINITION.finditer(path.read_text()):
            ty, star, name, arr, init, addr = m.groups()
            ty = ty.strip()
            if not (star or arr or ty in SCALARS):
                continue
            va = int(addr, 16)
            try:
                raw = read(image, va, 4)
            except NotBacked:
                if init is not None and init.strip() not in NEUTRAL:
                    guessed.append((path.name, name, addr, init.strip()[:48]))
                continue
            if raw is None or len(raw) < 4:
                continue
            value, = struct.unpack("<I", raw)
            if value and (init is None or init.strip() in NEUTRAL):
                dropped.append((path.name, name, addr, value))
    return dropped, guessed


def main() -> int:
    check = "--check" in sys.argv
    if not DEFAULT_IMAGE.exists():
        print("no image; skipping the initial-value comparison")
        return 0
    dropped, guessed = scan()
    for name, sym, addr, value in dropped:
        print(f"  {addr}  {sym:<32} zero in tree, image holds "
              f"{value} ({value:#x})   in {name}")
    if guessed and not check:
        print(f"\n  {len(guessed)} initialised in the tree where the image is zero "
              f"(not ratcheted - the image may set them at run time):")
        for name, sym, addr, init in guessed:
            print(f"  {addr}  {sym:<32} = {init}   in {name}")
    n = len(dropped)
    if n > CEILING:
        print(f"\nDROPPED INITIAL VALUES GREW: {n}, above the ceiling of {CEILING}")
        return 1
    if n < CEILING:
        print(f"\ndropped initial values down: {n}, below the ceiling of "
              f"{CEILING} - set the ceiling to {n} in this commit")
        return 1
    print(f"\ndropped initial values: {n}, ceiling exact")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
