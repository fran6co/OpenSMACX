#!/usr/bin/env python3
"""Emit per-function x86 assembly dumps for the Mizuchi indexer.

Writes one .s file per catalogued function into mizuchi/asm/nonmatching/
using the marker format Mizuchi's x86 parser expects:

    func_start ?name@@...@Z
    0x00483310  push ebp
    ...
    func_end ?name@@...@Z

Call/jmp targets that are catalogued get a '; =name' annotation so the
indexer can build the call graph.

Every function goes to the NONMATCHING folder on purpose: the indexer's
matched-C scan resolves assembly through compiled objects this project
does not have, and a function it cannot resolve fails the whole scan.
Recovered functions therefore appear assembly-only in the Atlas until a
matching-asm flow is added.

Output is derived from proprietary bytes: local analysis only, never
committed.
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import pefile  # noqa: E402
from capstone import CS_ARCH_X86, CS_MODE_32, Cs  # noqa: E402

from disasm import DEFAULT_EXE, annotate, load_functions, read_range  # noqa: E402
from generator_support import parse_body_ranges  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_OUT = REPO_ROOT / "mizuchi" / "asm" / "nonmatching"


def dump_one(pe: pefile.PE, engine: Cs, address: int, row: dict[str, str],
             functions: dict[int, dict[str, str]], out_dir: Path) -> None:
    name = row["name"]
    lines = [f"func_start {name}"]
    for lo, hi in parse_body_ranges(row["body_ranges"]):
        code = read_range(pe, lo, hi - lo)
        for instruction in engine.disasm(code, lo):
            note = annotate(instruction.mnemonic, instruction.op_str, functions)
            # '; name' -> '; =name': the '=' marks a function reference for the
            # indexer's call-graph extraction, matching the ARM '@ =' convention.
            if note:
                note = note.replace("; ", "; =", 1)
            operand = f" {instruction.op_str}" if instruction.op_str else ""
            lines.append(f"0x{instruction.address:08X}  "
                         f"{instruction.mnemonic}{operand}{note}")
    lines.append(f"func_end {name}")
    (out_dir / f"{address:08x}.s").write_text("\n".join(lines) + "\n")


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--out", type=Path, default=DEFAULT_OUT)
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    arguments = parser.parse_args()

    functions = load_functions()
    if not functions:
        print("no catalog: docs/recovery/functions.csv is missing", file=sys.stderr)
        return 1

    pe = pefile.PE(str(arguments.exe), fast_load=True)
    engine = Cs(CS_ARCH_X86, CS_MODE_32)
    arguments.out.mkdir(parents=True, exist_ok=True)

    written = skipped = 0
    for address, row in sorted(functions.items()):
        if not row.get("name") or not row.get("body_ranges"):
            skipped += 1
            continue
        try:
            dump_one(pe, engine, address, row, functions, arguments.out)
        except ValueError as error:
            print(f"skip 0x{address:08X} {row.get('name')}: {error}", file=sys.stderr)
            skipped += 1
            continue
        written += 1

    print(f"wrote {written} dump(s) into {arguments.out}, skipped {skipped}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
