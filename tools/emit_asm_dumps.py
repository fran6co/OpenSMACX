#!/usr/bin/env python3
"""Emit per-function x86 assembly dumps for the Mizuchi indexer.

Non-matching dumps: one .s file per catalogued function into
mizuchi/asm/nonmatching/, marked with the MANGLED name:

    func_start ?name@@...@Z
    0x00483310  push ebp
    ...
    func_end ?name@@...@Z

Matching dumps: for functions already recovered into src/ (catalog
source_locations), one .s file into mizuchi/asm/matchings/, marked with
the PLAIN name the C++ definition uses (method name, or Class / ~Class
for constructors / destructors), so the indexer can pair the definition
it finds in src/ with its assembly.

Call/jmp targets that are catalogued get a '; =name' annotation so the
indexer can build the call graph.

Output is derived from proprietary bytes: local analysis only, never
committed (the mizuchi/ tree is gitignored).
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import pefile  # noqa: E402
from capstone import CS_ARCH_X86, CS_MODE_32, Cs  # noqa: E402

from disasm import DEFAULT_EXE, annotate, load_functions, read_range  # noqa: E402
from generator_support import parse_body_ranges  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_OUT = REPO_ROOT / "mizuchi" / "asm" / "nonmatching"
DEFAULT_MATCHING_OUT = REPO_ROOT / "mizuchi" / "asm" / "matchings"

METHOD_NAME = re.compile(r"^\?([^@?]+)@")
CONSTRUCTOR_NAME = re.compile(r"^\?\?0([^@]+)@@")
DESTRUCTOR_NAME = re.compile(r"^\?\?1([^@]+)@@")


def plain_name(mangled: str) -> str | None:
    """The name a C++ definition of this symbol uses in src/, or None.

    ?method@Class@@... -> method, ??0Class@@... -> Class (constructor),
    ??1Class@@... -> ~Class (destructor). Operators and other specials
    (??2, ??_...) have no plain spelling this simple and are skipped.
    """
    match = METHOD_NAME.match(mangled)
    if match:
        return match.group(1)
    match = CONSTRUCTOR_NAME.match(mangled)
    if match:
        return match.group(1)
    match = DESTRUCTOR_NAME.match(mangled)
    if match:
        return f"~{match.group(1)}"
    return None


def dump_one(pe: pefile.PE, engine: Cs, address: int, row: dict[str, str],
             functions: dict[int, dict[str, str]], out_dir: Path,
             marker: str | None = None) -> None:
    name = marker or row["name"]
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
    parser.add_argument("--matching-out", type=Path, default=DEFAULT_MATCHING_OUT)
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    arguments = parser.parse_args()

    functions = load_functions()
    if not functions:
        print("no catalog: docs/recovery/functions.csv is missing", file=sys.stderr)
        return 1

    pe = pefile.PE(str(arguments.exe), fast_load=True)
    engine = Cs(CS_ARCH_X86, CS_MODE_32)
    arguments.out.mkdir(parents=True, exist_ok=True)
    arguments.matching_out.mkdir(parents=True, exist_ok=True)

    written = skipped = matching = 0
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

        # Recovered functions also get a matching dump keyed by the plain
        # name, so the indexer pairs the src/ definition with its assembly.
        source_location = (row.get("source_locations") or "").strip()
        if source_location.startswith("src/"):
            name = plain_name(row["name"])
            if name:
                try:
                    dump_one(pe, engine, address, row, functions,
                             arguments.matching_out, marker=name)
                    matching += 1
                except ValueError:
                    pass

    print(f"wrote {written} dump(s) into {arguments.out}, skipped {skipped}")
    print(f"wrote {matching} matching dump(s) into {arguments.matching_out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
