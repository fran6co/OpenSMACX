#!/usr/bin/env python3
"""Check that every wired redirect has room for the jump that installs it.

`install_redirect` in src/dllmain.cpp does an unconditional
`memcpy(original, patch, PatchSize)` with a five-byte `E9 rel32`. Nothing in
that path looks at how long the function being replaced actually is. 280 of the
1,986 wired redirects are SHORTER than five bytes - the shortest are a single
`ret` - and they work only because MSVC pads each function out to a sixteen-byte
slot, so the overrun lands in padding.

That is a property of the image, not of the mechanism, and nothing was checking
it. This does: for each wired redirect, the five bytes starting at its entry
must not reach the next function's first byte.

The failure it prevents is not a crash at the patch site. It is four bytes of
some unrelated function silently rewritten, which shows up much later as that
other function misbehaving.
"""

from __future__ import annotations

import argparse
import bisect
import csv
import re
import sys
from pathlib import Path

SPEC_ADDRESS = re.compile(r"\{\s*0x([0-9A-Fa-f]{8}),")
PATCH_SIZE = re.compile(r"constexpr\s+(?:size_t|std::size_t)\s+PatchSize\s*=\s*(\d+)\s*;")


def patch_size(dllmain: str) -> int:
    """The width of the jump, read from the source that installs it.

    Hardcoding five here would let the two drift: someone widening the patch to
    a fourteen-byte absolute jump would silently invalidate every check below
    while this file kept reporting success.
    """
    found = PATCH_SIZE.search(dllmain)
    if not found:
        raise SystemExit(
            "cannot find `constexpr size_t PatchSize = N;` in dllmain.cpp; "
            "refusing to guess the width of the patch being checked")
    return int(found.group(1))


def wired_addresses(dllmain: str) -> list[int]:
    """The canonical addresses in the RedirectSpec table.

    Scoped to the table rather than the whole file: address-shaped literals
    appear elsewhere in dllmain.cpp, and counting those would report checks
    that never happened.
    """
    try:
        start = dllmain.index("RedirectSpec specs[]")
        end = dllmain.index("static_assert(sizeof(specs)", start)
    except ValueError as problem:
        raise SystemExit(f"cannot locate the RedirectSpec table: {problem}")
    return sorted(int(one, 16) for one in SPEC_ADDRESS.findall(dllmain[start:end]))


def function_starts(catalogue: Path) -> list[int]:
    with catalogue.open(newline="") as handle:
        starts = sorted(int(row["address"], 16) for row in csv.DictReader(handle))
    if not starts:
        raise SystemExit(f"{catalogue} lists no functions")
    return starts


def overruns(addresses, starts, width):
    """Redirects whose patch reaches into the next function.

    The gap that matters is to the next function START, not the end of this
    function: the bytes in between are inter-function padding, and clobbering
    padding is exactly what the short redirects already rely on.
    """
    found = []
    for address in addresses:
        after = bisect.bisect_right(starts, address)
        if after >= len(starts):
            continue                    # nothing catalogued after it to damage
        following = starts[after]
        if address + width > following:
            found.append((address, following, following - address))
    return found


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dllmain", type=Path, default=Path("src/dllmain.cpp"))
    parser.add_argument("--functions", type=Path,
                        default=Path("docs/recovery/functions.csv"))
    options = parser.parse_args()

    dllmain = options.dllmain.read_text()
    width = patch_size(dllmain)
    addresses = wired_addresses(dllmain)
    starts = function_starts(options.functions)
    if not addresses:
        raise SystemExit("the RedirectSpec table is empty; nothing was checked")

    bad = overruns(addresses, starts, width)
    for address, following, gap in bad:
        print(f"{address:#010x}: the {width}-byte patch overruns {width - gap} "
              f"byte(s) into {following:#010x}", file=sys.stderr)
    if bad:
        raise SystemExit(f"{len(bad)} redirect(s) would clobber the next function")

    tight = sum(1 for address in addresses
                if (after := bisect.bisect_right(starts, address)) < len(starts)
                and starts[after] - address < width * 2)
    print(f"verify-redirect-patch-fit: {len(addresses)} redirect(s), "
          f"{width}-byte patch, none overrun ({tight} with less than "
          f"{width * 2} bytes of room)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
