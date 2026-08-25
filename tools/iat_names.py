#!/usr/bin/env -S uv run python
r"""The image's import table, as names for `g_00669304`-shaped globals.

WHY. Every `g_<address>` in the 0x00669xxx band is a slot in terranx.exe's
IMPORT ADDRESS TABLE - a Win32 function the image calls indirectly. Those
addresses do not need naming from inference: the PE file says what each one
holds, and the answer is exact. On 2026-08-25 sixteen names were reasoned out
of their use sites (GetAsyncKeyState from a `fn(2)` returning short,
GetSystemMetrics from `fn(0x17)` = SM_SWAPBUTTON) and this table confirmed
all sixteen and supplied two more the use sites could not settle - 0x00669354
is ShowCursor, 0x006692C8 is IsWindow.

A guess that survives a check is still a guess; this is the check. Use it
before naming anything in the import band, and prefer `--map` over typing
names by hand - it emits exactly what tools/name_globals.py consumes.

    uv run tools/iat_names.py                       # every import, by address
    uv run tools/iat_names.py 0x669304 0x66927c     # just these
    uv run tools/iat_names.py --map src/win.cpp     # rename map for one file
"""
from __future__ import annotations

import json
import pathlib
import re
import struct
import sys

REPO = pathlib.Path(__file__).resolve().parent.parent
IMAGE = REPO / ".opensmacx/game/terranx_original.exe"


def imports(path: pathlib.Path = IMAGE) -> dict[int, tuple[str, str]]:
    """{absolute address: (dll, symbol)} for every named import."""
    d = path.read_bytes()
    pe = struct.unpack_from("<I", d, 0x3C)[0]
    if d[pe:pe + 4] != b"PE\0\0":
        raise SystemExit(f"{path} is not a PE image")
    nsec = struct.unpack_from("<H", d, pe + 6)[0]
    opt_size = struct.unpack_from("<H", d, pe + 20)[0]
    base = struct.unpack_from("<I", d, pe + 24 + 28)[0]
    imp_rva = struct.unpack_from("<I", d, pe + 24 + 96 + 8)[0]

    secs = []
    for i in range(nsec):
        o = pe + 24 + opt_size + i * 40
        vsz, va, _rsz, ra = struct.unpack_from("<IIII", d, o + 8)
        secs.append((va, vsz, ra))

    def off(rva):
        for va, vsz, ra in secs:
            if va <= rva < va + max(vsz, 1):
                return ra + (rva - va)
        return None

    def cstr(o):
        return d[o:d.index(b"\0", o)].decode("ascii", "replace")

    out, o = {}, off(imp_rva)
    while True:
        oft, _, _, namerva, first = struct.unpack_from("<IIIII", d, o)
        if namerva == 0:
            break
        dll = cstr(off(namerva))
        t, iat = off(oft or first), first
        while True:
            v = struct.unpack_from("<I", d, t)[0]
            if v == 0:
                break
            if not v & 0x80000000:            # by name, not by ordinal
                out[base + iat] = (dll, cstr(off(v) + 2))
            t += 4
            iat += 4
        o += 20
    return out


def main() -> int:
    args = [a for a in sys.argv[1:]]
    table = imports()
    if "--map" in args:
        args.remove("--map")
        target = pathlib.Path(args[0])
        found = {}
        for addr in sorted(set(re.findall(r"\bg_(00[0-9a-f]{6})\b",
                                          target.read_text(errors="replace")))):
            hit = table.get(int(addr, 16))
            if hit:
                found[addr] = "g_" + hit[1]
        print(json.dumps(found, indent=2))
        return 0
    if args:
        for a in args:
            n = int(a, 16)
            dll, sym = table.get(n, ("?", "NOT AN IMPORT"))
            print(f"0x{n:08X}  {sym:<28} {dll}")
        return 0
    for a, (dll, sym) in sorted(table.items()):
        print(f"0x{a:08X}  {sym:<28} {dll}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
