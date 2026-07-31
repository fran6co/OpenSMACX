#!/usr/bin/env python3
"""Turn IDA's exported C header into verified class sizes, via the compiler.

IDA Free 9.4 ships no IDAPython - its plugins directory has the decompiler and
no `idapython.so`, so a `.py` handed to File > Script file is parsed as IDC and
dies on the shebang as a "bad or ill formed preprocessor command". Rewriting the
export in IDC would mean guessing which struct API IDA 9 kept, since IDA 9
folded structures into the type system and IDC has no way to test for a function
before calling it.

This needs no scripting at all. `File > Produce file > Create C header file`
writes every local type as C declarations; from there the SIZE is computed by
compiling the declarations and asking the compiler, which is exact by
construction and does not require this tool to model a single alignment rule.

  1. In IDA:  File > Produce file > Create C header file...
  2. Here:    tools/sizes_from_ida_header.py types.h --out sizes.csv
  3. Then:    tools/derive_class_layout.py --score-csv sizes.csv

Step 3 is not optional. The output is a hypothesis like any other, and two
sources have already failed that gate - Thinker's headers at 5 right and 11
wrong because its same-named structs are different types, and the readable IDB
at 31 right and 2 wrong because some of its structs are incomplete hand
reconstructions. A wrong layout compiles, links, and corrupts memory only at
run time, so nothing here is believed until it scores zero wrong against the 40
classes whose sizes are already pinned.

-m32 IS NOT OPTIONAL EITHER. The subject is a 32-bit PE. Compiling its types as
64-bit silently doubles every pointer member and changes the alignment of
everything that follows one, which would produce sizes that look plausible and
are wrong for most classes carrying a pointer.
"""

from __future__ import annotations

import argparse
import csv
import re
import subprocess
import sys
import tempfile
from pathlib import Path

# `struct NAME`, `struct __cppobj NAME`, `struct __attribute__((aligned(N))) X`
STRUCT_RE = re.compile(
    r"^\s*struct\s+(?:__cppobj\s+)?(?:__unaligned\s+)?"
    r"(?:__attribute__\s*\(\([^)]*\)\)\s*)?"
    r"(?P<name>[A-Za-z_]\w*)\s*(?::[^{]*)?\{", re.M)


def struct_names(header: str) -> list[str]:
    """Every struct the header defines a body for, in order, without duplicates."""
    seen: dict[str, None] = {}
    for found in STRUCT_RE.finditer(header):
        seen.setdefault(found.group("name"), None)
    return list(seen)


def probe_source(header_path: Path, names: list[str]) -> str:
    """A program that prints `name,size` for each struct the header declares."""
    lines = [f'#include "{header_path.name}"', "#include <stdio.h>", "",
             "int main(void) {"]
    for name in names:
        # Printed one per line and parsed back, rather than computed here: the
        # COMPILER is the authority on layout, which is the whole point.
        lines.append(f'    printf("{name},0x%zx\\n", sizeof(struct {name}));')
    lines += ["    return 0;", "}"]
    return "\n".join(lines) + "\n"


def measure(header_path: Path, names: list[str], compiler: str,
            extra: list[str]) -> tuple[dict, list[str]]:
    """Compile and run; returns (sizes, names the compiler refused).

    A struct that will not compile - an incomplete type, a member whose own
    type IDA did not export - is REPORTED, never guessed at. Dropping it
    silently would make a partial export look complete.
    """
    with tempfile.TemporaryDirectory() as directory:
        work = Path(directory)
        (work / header_path.name).write_text(
            header_path.read_text(encoding="utf-8", errors="replace"),
            encoding="utf-8")
        remaining = list(names)
        refused: list[str] = []
        # Drop whatever the compiler rejects and retry, so one bad struct does
        # not cost the whole export.
        for _ in range(len(names) + 1):
            source = work / "probe.c"
            source.write_text(probe_source(header_path, remaining),
                              encoding="utf-8")
            binary = work / "probe"
            done = subprocess.run(
                [compiler, "-w", "-std=gnu99", *extra, str(source),
                 "-o", str(binary)],
                capture_output=True, text=True, cwd=work)
            if done.returncode == 0:
                break
            broken = set()
            for line in done.stderr.splitlines():
                for name in remaining:
                    if f"struct {name}" in line or f"'{name}'" in line:
                        broken.add(name)
            if not broken:
                raise SystemExit(
                    "the probe will not compile and no struct can be blamed:\n"
                    + done.stderr[:1500])
            refused.extend(sorted(broken))
            remaining = [n for n in remaining if n not in broken]
            if not remaining:
                return {}, refused
        run = subprocess.run([str(binary)], capture_output=True, text=True)
        sizes = {}
        for line in run.stdout.splitlines():
            name, _, value = line.partition(",")
            if value:
                sizes[name] = int(value, 16)
        return sizes, refused


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("header", type=Path,
                        help="the .h IDA wrote via Produce file")
    parser.add_argument("--out", type=Path, default=Path("ida-sizes.csv"))
    parser.add_argument("--compiler", default="gcc")
    parser.add_argument("--bits", type=int, default=32, choices=(32, 64),
                        help="32 unless you know why not; the subject is a "
                             "32-bit PE and 64-bit doubles every pointer")
    args = parser.parse_args(argv)

    if not args.header.is_file():
        print(f"error: {args.header} not found", file=sys.stderr)
        return 2
    text = args.header.read_text(encoding="utf-8", errors="replace")
    names = struct_names(text)
    if not names:
        print(f"error: {args.header} declares no struct bodies; was it the "
              f"C header export?", file=sys.stderr)
        return 2
    print(f"{args.header.name}: {len(names)} struct(s) declared")

    sizes, refused = measure(args.header, names, args.compiler,
                             [f"-m{args.bits}"])
    with args.out.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.writer(handle, lineterminator="\n")
        writer.writerow(["struct", "size"])
        for name in sorted(sizes):
            writer.writerow([name, f"0x{sizes[name]:X}"])
    print(f"measured {len(sizes)} at {args.bits}-bit -> {args.out}")
    if refused:
        print(f"{len(refused)} struct(s) the compiler refused, so they carry "
              f"NO size here rather than a guessed one:")
        print("   " + ", ".join(refused[:16])
              + (" ..." if len(refused) > 16 else ""))
    print("\nNOT YET USABLE. Score it before believing any of it:")
    print(f"  tools/derive_class_layout.py --score-csv {args.out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
