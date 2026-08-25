#!/usr/bin/env -S uv run python
r"""Turn `*reinterpret_cast<T *>(self + 0x13c)` into `outer_rect_.left`.

WHY. `class_debt`'s "raw self-access" shape counts every body that walks its
own object through `reinterpret_cast<char *>(this)` and a raw offset while
the class DECLARES the member. Homing imports the form by the hundred, and
the remedy is not style: a divergence after the rewrite is a LAYOUT finding,
which is the whole reason the shape is ratcheted.

WHERE THE MAP COMES FROM. Not from inference - from the header, which
already records every offset this tool needs:

  - `win.h:277-278` puts `iFlags_` at 0x98 (immediately after AutoSound's
    0x98 bytes) and `iSomeFlag_` at 0x9C;
  - `win.h:50-63` annotates every WinNodeList field with its Win offset,
    so 0xCC..0xDC are `list_.head_` .. `list_.external_`;
  - `outer_rect_` at 0x13C and `client_rect_` at 0x14C are RECTs, which is
    why IDA's member table lists their four dwords as separate `field_140`,
    `field_144`... entries. 0x140 is `outer_rect_.top`, not a field.

That last one is the reason to prefer the header over
`docs/recovery/idb-members.csv`: the CSV cannot see a struct member, so it
names the inside of one.

    uv run tools/name_offsets.py src/win.cpp map.json          # dry run
    uv run tools/name_offsets.py src/win.cpp map.json --apply

THE ACCESS WIDTH IS PART OF THE ACCESS. `*reinterpret_cast<unsigned char *>
(self + 0xa14)` is a ONE-BYTE store; the member at 0xA14 is declared
`uint32_t`, so rewriting it to a bare `field_A14_ = 0` silently widens it to
four bytes. That cost EditBox::close (0x00614F30) its BYTE_EXACT claim on
2026-08-26 - image `mov byte ptr [esi + 0xa14], al`, this tree
`mov dword ptr`. When the cast is narrower than the member, keep it:
`*reinterpret_cast<unsigned char *>(&field_A14_)`.

TWO THINGS A TREE-WIDE SWEEP HITS, measured 2026-08-26 by trying one:

  `self` IS NOT ALWAYS `this`. spritebox.cpp defines it BOTH ways in one
  file - `reinterpret_cast<char *>(this)` at line 67 and
  `reinterpret_cast<char *>(this) - 0x8C` at line 234, an adjustor thunk
  reaching a base subobject. Offsets off the second belong to ANOTHER
  class, and naming them from this class's map produces undeclared
  identifiers. Check every `self` definition in a file before sweeping it.

  A CORRECT NAME CAN STILL NOT COMPILE. basebutton.cpp reads a `char *`
  member through `*(int *)(self + N)` into an `int` local, and statuswin.cpp
  assigns 4 to what is declared `Font *`. The naming is right and the
  surrounding code is written in ints; each site needs a decision about
  which side to change, which is not a sweep.

Every rewrite is a candidate for a byte change, unlike a rename - re-measure
the touched claims. Palette's copy_from measured identical (19/19) in the
member form, which is the precedent, not a guarantee.
"""
from __future__ import annotations

import collections
import json
import pathlib
import re
import sys

# `self` ONLY, and the restriction is load-bearing: `self` is this body's own
# `reinterpret_cast<char *>(this)`, so a bare member name resolves. A base
# like `wc` or `winb` points at ANOTHER object, where the same offset needs
# `obj->member` - rewriting those to bare names compiled as "undeclared
# identifier" in the free functions homed into win.cpp, which is how this
# restriction was found rather than reasoned.
# `*reinterpret_cast<unsigned int *>(self + 0x98)` and the C-style twin.
# The INLINE form, where no `self` local exists: `*(int *)(reinterpret_cast<
# char *>(this) + 0x98)`. Same access, same remedy - the tool used to see
# only the `self` spelling and left these behind.
INLINE = re.compile(
    r"\*\s*(?:reinterpret_cast<\s*[\w ]+\*+\s*>|\(\s*[\w ]+\*+\s*\))\s*"
    r"\(\s*(?:reinterpret_cast<\s*char\s*\*\s*>\s*\(\s*this\s*\)"
    r"|\(\s*char\s*\*\s*\)\s*this)\s*\+\s*0x(?P<off>[0-9A-Fa-f]+)\s*\)")
INLINE_RECT = re.compile(
    r"reinterpret_cast<\s*RECT\s*\*\s*>\s*\(\s*(?:reinterpret_cast<\s*char"
    r"\s*\*\s*>\s*\(\s*this\s*\)|\(\s*char\s*\*\s*\)\s*this)\s*\+\s*"
    r"0x(?P<off>[0-9A-Fa-f]+)\s*\)")

# NARROW TYPES KEEP THEIR CAST. `*(unsigned char *)(self + 0xa14) = 0` is a
# ONE-BYTE store; the member at 0xA14 is `uint32_t`, so returning the bare
# name widens it to four bytes and changes the emitted instruction from
# `mov byte ptr` to `mov dword ptr`. That cost EditBox::close (0x00614F30)
# its BYTE_EXACT claim on 2026-08-26.
NARROW = {"char", "unsigned char", "signed char", "uint8_t", "int8_t",
          "short", "unsigned short", "uint16_t", "int16_t", "BYTE", "WORD"}

ACCESS = re.compile(
    r"\*\s*(?:reinterpret_cast<\s*(?P<ty>[\w ]+?)\s*\*+\s*>"
    r"|\(\s*(?P<ty2>[\w ]+?)\s*\*+\s*\))\s*"
    # `self` bare, or wrapped in a redundant `(char *)` the artifact emitted
    # even where `self` is already `char *`.
    r"\(\s*(?:\(\s*char\s*\*\s*\)\s*)?(?P<base>self)\s*\+\s*"
    r"0x(?P<off>[0-9A-Fa-f]+)\s*\)")
# a RECT taken by address rather than dereferenced
RECT_PTR = re.compile(
    r"reinterpret_cast<\s*RECT\s*\*\s*>\s*\(\s*(?P<base>self)\s*\+\s*"
    r"0x(?P<off>[0-9A-Fa-f]+)\s*\)")


def main() -> int:
    if len(sys.argv) < 3:
        print(__doc__)
        return 2
    src = pathlib.Path(sys.argv[1])
    table = {k.lower(): v for k, v in json.loads(
        pathlib.Path(sys.argv[2]).read_text()).items()}
    apply = "--apply" in sys.argv
    text = src.read_text(errors="replace")

    counts = collections.Counter()

    def rect(m):
        off = f"0x{int(m.group('off'), 16):x}"
        name = table.get(off)
        # only a `.left` entry names the RECT itself
        if not name or not name.endswith(".left"):
            return m.group(0)
        counts[off] += 1
        return f"&{name[:-len('.left')]}"

    def scalar(m):
        off = f"0x{int(m.group('off'), 16):x}"
        name = table.get(off)
        if not name:
            return m.group(0)
        counts[off] += 1
        cast = (m.groupdict().get("ty") or m.groupdict().get("ty2") or "").strip()
        if cast in NARROW:
            # keep the width, name the member
            return f"*reinterpret_cast<{cast} *>(&{name})"
        return name

    # REFUSE THE WHOLE FILE, not just the body. `self` is not always
    # `this`: spritebox.cpp defines it eight times, two of them adjusted -
    # `(char *)this - 0x8C` at line 234 and `- 0x28` at 728, adjustor thunks
    # reaching a base subobject, where an offset belongs to ANOTHER class.
    #
    # A per-body skip was tried first and IS NOT SAFE. Finding a body's end
    # by scanning for `}` at column 0 stops early when the body contains a
    # nested class definition - which 728's does - so the skip ended before
    # the accesses and rewrote them anyway. Measured by watching it break
    # the build after the guard was in place.
    #
    # A file-level refusal cannot be fooled that way. The cost is that such
    # a file needs its class pass rather than a sweep, which is true of
    # spritebox regardless.
    ADJUSTED = re.compile(
        r"^\s*(?:const\s+)?char\s*\*(?:\s*const)?\s+self\s*=\s*"
        r"reinterpret_cast<char \*>\(this\)\s*[-+]")
    if any(ADJUSTED.match(line) for line in text.split("\n")):
        print(f"  REFUSED {src.name}: `self` is defined as an ADJUSTED `this` "
              f"somewhere in this file, so an offset here may belong to "
              f"another class. Needs the class pass, not a rewrite.")
        return 1

    text = INLINE_RECT.sub(rect, text)
    text = RECT_PTR.sub(rect, text)
    text = INLINE.sub(scalar, text)
    text = ACCESS.sub(scalar, text)

    for off, n in sorted(counts.items(), key=lambda kv: -kv[1]):
        print(f"  {off:<8} x{n:<3} -> {table[off]}")
    print(f"{sum(counts.values())} access(es) over {len(counts)} offset(s)"
          f"{'' if apply else '  (dry run; pass --apply)'}")
    if apply:
        src.write_text(text)
    return 0


if __name__ == "__main__":
    sys.exit(main())
