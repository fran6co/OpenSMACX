#!/usr/bin/env python3
"""Read the shipped image's DATA at a virtual address.

The tree can already ask the image what CODE is at an address; it could not
ask what a byte, a string or a table holds. That gap is why fixed-address
scaffolds for strings survived: giving `WinMsgTooManyChildren` real storage
needs the text, and nothing could read it.

    uv run tools/image_data.py 0x00696D80 --string
    uv run tools/image_data.py 0x00696D34 --dwords 8
    uv run tools/image_data.py 0x00696D14 --bytes 4
"""
from __future__ import annotations

import argparse
import struct
import sys
from pathlib import Path

DEFAULT_IMAGE = Path(".opensmacx/game/terranx_original.exe")


def _map(exe: Path):
    """(data, [(va_start, va_end, raw_offset, raw_size, name)]) per section.

    `va_end` is the VIRTUAL end and `raw_size` the number of bytes the file
    actually carries. An address between the two is real, addressable memory
    that the file does not back: it is ZERO at run time. Keeping the two
    apart is the whole point - it is what tells `const int X = 9;` from
    `int X;`, and the byte ratchet cannot see the difference.
    """
    data = exe.read_bytes()
    e_lfanew, = struct.unpack_from("<I", data, 0x3C)
    n_sections, = struct.unpack_from("<H", data, e_lfanew + 6)
    opt_size, = struct.unpack_from("<H", data, e_lfanew + 20)
    image_base, = struct.unpack_from("<I", data, e_lfanew + 24 + 28)
    table = e_lfanew + 24 + opt_size
    spans = []
    for i in range(n_sections):
        off = table + i * 40
        name = data[off:off + 8].rstrip(b"\0").decode("latin-1")
        vsize, vaddr, rsize, raw = struct.unpack_from("<IIII", data, off + 8)
        spans.append((image_base + vaddr, image_base + vaddr + max(vsize, rsize),
                      raw, rsize, name))
    return data, spans


class NotBacked(Exception):
    """The address is real memory the file does not carry: zero at run time."""

    def __init__(self, section: str):
        super().__init__(section)
        self.section = section


def read(exe: Path, va: int, length: int) -> bytes | None:
    data, spans = _map(exe)
    for start, end, raw, rsize, name in spans:
        if start <= va < end:
            delta = va - start
            if delta >= rsize:
                raise NotBacked(name)
            return data[raw + delta:raw + min(delta + length, rsize)]
    return None


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("address", help="virtual address, e.g. 0x00696D80")
    ap.add_argument("--image", type=Path, default=DEFAULT_IMAGE)
    ap.add_argument("--string", action="store_true", help="read a NUL-terminated C string")
    ap.add_argument("--bytes", type=int, metavar="N", help="read N raw bytes")
    ap.add_argument("--dwords", type=int, metavar="N", help="read N little-endian dwords")
    args = ap.parse_args()

    if not args.image.exists():
        print(f"no image at {args.image}", file=sys.stderr)
        return 2
    va = int(args.address, 0)

    def fetch(n: int) -> bytes | None:
        try:
            return read(args.image, va, n)
        except NotBacked as exc:
            print(f"{va:#010x} is in {exc.section}, past the bytes the file "
                  f"carries: uninitialised, ZERO at run time", file=sys.stderr)
            raise SystemExit(3)

    if args.string:
        raw = fetch(4096)
        if raw is None:
            print(f"{va:#010x} is not inside any section", file=sys.stderr)
            return 1
        end = raw.find(b"\0")
        if end < 0:
            print(f"{va:#010x}: no NUL within 4096 bytes", file=sys.stderr)
            return 1
        print(repr(raw[:end].decode("latin-1")))
        return 0

    n = args.bytes or (args.dwords * 4 if args.dwords else 16)
    raw = fetch(n)
    if raw is None:
        print(f"{va:#010x} is not inside any section", file=sys.stderr)
        return 1
    if args.dwords:
        if len(raw) < args.dwords * 4:
            print(f"only {len(raw)} byte(s) available at {va:#010x}", file=sys.stderr)
            return 1
        for i in range(args.dwords):
            word, = struct.unpack_from("<I", raw, i * 4)
            print(f"{va + i*4:#010x}  {word:#010x}  {word}")
    else:
        print(" ".join(f"{b:02x}" for b in raw))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
