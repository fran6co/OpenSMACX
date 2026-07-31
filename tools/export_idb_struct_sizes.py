#!/usr/bin/env python3
"""IDAPython: dump every struct's size to CSV. Run inside IDA, not here.

WHY THIS IS A SEPARATE SCRIPT. `.opensmacx/game/terranx.exe.i64` is IDA2 format
(IDA 7.0 and later) and `python-idb` cannot read it - it raises MemoryError on
open, because it only implements the older IDA 6.x container. The other
database, `.opensmacx/analysis/terranx_ORIG_200_v3_7.5.SP3.idb`, IS readable
that way, but its struct definitions are hand reconstructions that are
sometimes incomplete: `PullDown` declares one member of 0xa14 against a true
0xf40, and `Console` sums four bytes short of 0x247a8. Scored against the 40
classes whose size is already pinned, that route reads 31 right, 2 wrong.

Two wrong is two too many. A wrong class layout compiles perfectly and corrupts
memory at runtime, which is why `derive_class_layout.py` is built to abstain
rather than answer, and why its `--check-pinned` counts wrong answers instead
of coverage.

So the reliable path is to let IDA itself report the sizes it holds, from
whichever database is better maintained. Inside IDA:

    File > Script file...  ->  tools/export_idb_struct_sizes.py

or headless:

    idat64 -A -S"tools/export_idb_struct_sizes.py" terranx.exe.i64

It writes `idb-struct-sizes.csv` beside the database. Nothing here reads the
executable or emits any of its content: only struct names and integer sizes,
which are facts about layout rather than copied material.

WHAT TO DO WITH THE RESULT. It is a hypothesis source like any other and gets
the same treatment the repository already applies to Thinker: verified against
the 40 known sizes before anything is believed. Thinker failed exactly that
check at 5 right and 7 wrong, because its same-named structs are different
types. Run `--check-pinned` against this CSV and require ZERO wrong before a
single size is used to stage an oracle receiver.
"""

try:
    import idaapi
    import idautils
    import idc
    try:
        import ida_typeinf          # IDA 7.4+; absent on older builds
    except ImportError:
        ida_typeinf = None
except ImportError:  # pragma: no cover - only importable inside IDA
    raise SystemExit(
        "This script runs INSIDE IDA. It is checked in so that whoever has the "
        "database can export sizes without needing anything else installed; "
        "python-idb cannot read the IDA2 (.i64) container."
    )

import csv
import os


def _type_library():
    """The local type library, across three generations of the API."""
    for module in (globals().get("ida_typeinf"), idaapi):
        if module is None:
            continue
        getter = getattr(module, "get_idati", None)
        if getter:
            return module, getter()
    return None, None


def _ordinal_limit(module, library):
    """IDA 9 renamed get_ordinal_qty to get_ordinal_limit."""
    for name in ("get_ordinal_limit", "get_ordinal_qty"):
        call = getattr(module, name, None)
        if call:
            try:
                return call(library)
            except TypeError:
                return call()
    return 0


def structures():
    """(name, size) for every structure, across IDA's eras of API.

    IDA 9 folded structures into the type system and removed
    `idautils.Structs`, so the type-library walk is tried FIRST and the old
    struct API is only a fallback. Both are attempted rather than one being
    chosen from the version string, because a version test would be one more
    thing to get wrong on a machine this was never run on.
    """
    found = []
    module, library = _type_library()
    if library is not None:
        tinfo = getattr(module, "tinfo_t", None) or idaapi.tinfo_t
        for ordinal in range(1, _ordinal_limit(module, library) + 1):
            tif = tinfo()
            if not tif.get_numbered_type(library, ordinal):
                continue
            if not tif.is_struct():
                continue
            name = tif.get_type_name()
            size = tif.get_size()
            # BADSIZE comes back as -1 or a huge unsigned; either way it is
            # not an answer and must not be written as one.
            if name and size and size != idaapi.BADSIZE and size < (1 << 31):
                found.append((name, size))
    if not found and hasattr(idautils, "Structs"):
        for _, sid, name in idautils.Structs():
            size = idc.get_struc_size(sid)
            if size:
                found.append((name, size))
    return found


def main():
    where = os.path.dirname(idc.get_idb_path()) or "."
    out = os.path.join(where, "idb-struct-sizes.csv")
    rows = structures()
    with open(out, "w", newline="") as handle:
        writer = csv.writer(handle, lineterminator="\n")
        writer.writerow(["struct", "size"])
        for name, size in sorted(rows):
            # A zero-size struct is a forward declaration with no members, and
            # writing it would look like an answer.
            if size:
                writer.writerow([name, f"0x{size:X}"])
    print("wrote %d struct size(s) to %s" % (len(rows), out))


if __name__ == "__main__":
    main()
