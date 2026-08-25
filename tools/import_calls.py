#!/usr/bin/env -S uv run python
r"""Which IMPORT does each body call - the image's answer against this tree's.

WHY THE BYTE RATCHET CANNOT DO THIS. `call dword ptr [0x669310]` and
`call dword ptr [0x669284]` differ only in a relocation, and
`decomp/asm.py` masks relocation-determined bytes out of the comparison.
So a body that calls SetCursorPos where the image calls GetCursorPos is
BYTE_EXACT. Measured 2026-08-25: win.cpp reached through one binding named
`g` for EIGHT different Win32 imports, and every caller was and stayed
BYTE_EXACT.

`tools/call_diff.py` does not cover it either, and says so: it compares
DIRECT calls only, because "an indirect call through a vtable or a bound
slot is a runtime fact neither side can resolve". That is true of a vtable
dispatch. It is NOT true of an import slot: `call dword ptr [0x669310]`
names an absolute address, `decomp/calls.py` already records it as
`form="indirect", slot=...`, and the PE import table says what lives there.

So this compares the SET OF IMPORT SLOTS each side calls, and names them.
A body whose slots differ is calling the wrong function, whatever its
bytes say.

    uv run tools/import_calls.py                 # every claimed body
    uv run tools/import_calls.py src/win.cpp     # one file
"""
from __future__ import annotations

import collections
import concurrent.futures
import os
import pathlib
import sys

REPO = pathlib.Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO))

from decomp import read                                        # noqa: E402
from decomp.asm import (build_command, original_asm,           # noqa: E402
                        compiled_asm)
from decomp.calls import call_sites                            # noqa: E402
from tools.iat_names import imports                            # noqa: E402

IMAGE = REPO / ".opensmacx/game/terranx_original.exe"
COMPILE_COMMANDS = REPO / "build" / "compile_commands.json"
FLAGS = "/c /O2 /Gy /GR- /GX"


def _slots(listing) -> collections.Counter:
    """The absolute addresses this body calls through, one count each."""
    out: collections.Counter = collections.Counter()
    for site in call_sites(listing):
        if site.form == "indirect" and site.slot:
            out[site.slot] += 1
    return out


def _one_file(job: tuple) -> tuple[list[str], int]:
    """Report lines for one file, plus how many bodies it could not cover."""
    path, group, command = job
    table = imports()
    rows: list[str] = []
    missed = 0
    for record in group:
        try:
            theirs = _slots(original_asm(record, IMAGE))
            ours = _slots(compiled_asm(record, command, FLAGS))
        except Exception as problem:                   # noqa: BLE001
            # SAID, not swallowed: a body this cannot compile or locate is a
            # body this check did not cover, and a silent skip is how a
            # checker reports zero while looking at nothing.
            rows.append(f"  {record.address_hex} {path.name}: skipped ({problem})")
            missed += 1
            continue
        theirs = collections.Counter(
            {s: n for s, n in theirs.items() if s in table})
        ours = collections.Counter(
            {s: n for s, n in ours.items() if s in table})
        if not ours:
            continue          # calls its imports by name; nothing to say
        missing, extra = theirs - ours, ours - theirs
        if not missing and not extra:
            continue
        rows.append(f"\n  {record.address_hex}  {path.name}  {record.name}")
        for slot, n in sorted(missing.items()):
            who = table.get(slot, ("", "not an import"))[1]
            rows.append(f"      image calls [0x{slot:06x}] {who} x{n}, "
                        f"this tree does not")
        for slot, n in sorted(extra.items()):
            who = table.get(slot, ("", "not an import"))[1]
            rows.append(f"      this tree calls [0x{slot:06x}] {who} x{n}, "
                        f"the image does not")
    return rows, missed


def main() -> int:
    only = pathlib.Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else None
    table = imports()
    records = [r for r in read(REPO / "src")
               if r.address and (r.byte_exact or r.semantic)
               and (only is None or r.path.resolve() == only)]
    by_file: dict[pathlib.Path, list] = collections.defaultdict(list)
    for r in records:
        by_file[r.path].append(r)

    # ONE WORKER PER FILE, capped like the gate's own compiles. Serially this
    # is 912 files of `cl` back to back - a run that outlives its usefulness.
    # The cap is the wine prefix's, not the CPU's: several sweeps against one
    # prefix queue and a queued compile times out as a compile FAILURE.
    jobs = max(1, min(os.cpu_count() or 1, 8))
    work = []
    for path, group in sorted(by_file.items()):
        try:
            work.append((path, group, build_command(COMPILE_COMMANDS, path)))
        except Exception as problem:                   # noqa: BLE001
            print(f"  {path.name}: no compile command ({problem})", flush=True)
    print(f"{len(records):,} claimed bodies in {len(work):,} files, "
          f"{jobs} at a time", flush=True)

    disagree = skipped = 0
    with concurrent.futures.ProcessPoolExecutor(max_workers=jobs) as pool:
        for rows, missed in pool.map(_one_file, work):
            skipped += missed
            for line in rows:
                disagree += line.startswith("\n")
                print(line, flush=True)

    print(f"\n{disagree} claimed body(s) call a different import than the "
          f"image; {skipped} skipped, {len(records)} examined")
    return 1 if disagree else 0


if __name__ == "__main__":
    sys.exit(main())
