#!/usr/bin/env python3
"""Emit the module definition file the VC6 build links with.

WHY THIS IS NOT generate_mingw_exports.py WITH A FLAG. That tool exists to
BRIDGE two manglings: the patched executable imports MSVC names, MinGW emits
Itanium names, and nothing connects them, so it has to build the DLL first,
read its symbols back with nm, demangle both sides and pair them by signature
- which is why every line it writes is an alias, `"?init@Heap@@QAEHI@Z" =
_ZN4Heap4initEj`.

cl 12.00.8168 has no such gap. It emits the MSVC name directly, because it is
the compiler that produced the names in the first place. So the export list
is just the import list, there is nothing to pair, and this tool does not
need the DLL to exist - which also removes the chicken-and-egg where the
export list could only be regenerated from a build that already linked.

The `.def` is still worth having rather than relying on `__declspec(dllexport)`
alone: it PINS the set. A body that stops being exported - renamed, made
static, dropped from the target - becomes an unresolved external at link time
instead of a DLL that loads and then cannot be bound to the patched image.
"""

from __future__ import annotations

import argparse
from pathlib import Path

import pefile


def load_imports(path: Path) -> list:
    """The names the patched executable expects OpenSMACX.dll to export."""
    pe = pefile.PE(str(path))
    try:
        entry = next(
            entry for entry in pe.DIRECTORY_ENTRY_IMPORT
            if entry.dll.lower() == b"opensmacx.dll")
        return [item.name.decode("ascii") for item in entry.imports]
    finally:
        pe.close()


def render(names) -> str:
    lines = ["LIBRARY OpenSMACX", "EXPORTS"]
    # Sorted and de-duplicated so the file is stable across regenerations and
    # a diff shows a real change rather than a reordering.
    lines += [f'    "{name}"' for name in sorted(set(names))]
    return "\n".join(lines) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--patched-exe", required=True,
                        help="Executable whose OpenSMACX.dll imports pin the export set")
    parser.add_argument("--output", required=True,
                        help="Module definition file to write")
    parser.add_argument("--check", action="store_true",
                        help="Exit non-zero if --output is not what would be written")
    arguments = parser.parse_args()

    wanted = render(load_imports(Path(arguments.patched_exe)))
    output = Path(arguments.output)

    if arguments.check:
        actual = output.read_text(encoding="ascii") if output.is_file() else ""
        if actual != wanted:
            print(f"{output} is stale; regenerate without --check")
            return 1
        print(f"{output}: up to date ({wanted.count(chr(10)) - 2} exports)")
        return 0

    output.write_text(wanted, encoding="ascii", newline="\n")
    print(f"wrote {wanted.count(chr(10)) - 2} exports to {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
