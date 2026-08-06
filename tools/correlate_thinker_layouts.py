#!/usr/bin/env python3
"""Extract struct-layout and global-address hypotheses from Thinker headers.

The Thinker mod's engine headers describe packed struct layouts and absolute
global addresses reverse engineered from the same binary. Under the repository
policy they are hypothesis sources only: this tool reduces fetched local
copies to two ignored CSVs of offsets and addresses so the layout campaign can
consume leads without any header text entering the repository. Every lead
still requires independent verification against the hash-bound canonical
executable before source, tests, or metadata are committed.
"""

import argparse
import csv
from pathlib import Path
import re
import sys

from local_artifact import require_local_artifact_path


REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_SOURCE_DIR = (
    REPO_ROOT / ".opensmacx" / "external-analysis" / "sources" / "thinker")
DEFAULT_OUTPUT_DIR = REPO_ROOT / ".opensmacx" / "external-analysis"

STRUCT_RE = re.compile(r"^\s*struct\s+(?P<name>\w+)\s*\{")
FIELD_RE = re.compile(
    r"^\s*(?P<type>[A-Za-z_]\w*(?:\s*\*+)?)\s+"
    r"(?P<name>\w+)\s*(?:\[(?P<count>\w+)\])?\s*;")
ADDRESS_RE = re.compile(r"0x0{0,2}[0-9A-Fa-f]{6,8}\b")
BINDING_RE = re.compile(
    r"^\s*\w[\w\s\*&:<>,]*?[\s\*&](?P<name>\w+)\s*="
    r"[^;]*?(?P<address>0x[0-9A-Fa-f]{6,8})\b")

PRIMITIVE_SIZES = {
    "int8_t": 1, "uint8_t": 1, "char": 1, "bool": 1,
    "int16_t": 2, "uint16_t": 2, "short": 2,
    "int32_t": 4, "uint32_t": 4, "int": 4, "float": 4, "long": 4,
    "int64_t": 8, "uint64_t": 8, "double": 8,
}


def field_size(type_name, count):
    if "*" in type_name:
        base = 4  # 32-bit target pointers
    elif type_name in PRIMITIVE_SIZES:
        base = PRIMITIVE_SIZES[type_name]
    else:
        return None
    if count is None:
        return base
    try:
        return base * int(count, 0)
    except ValueError:
        return None


def parse_struct_layouts(text, source_name):
    """Walk packed struct bodies accumulating byte offsets for primitive
    fields; offsets become unknown after the first unsized field."""
    rows = []
    struct_name = None
    offset = 0
    known = True
    for line_number, line in enumerate(text.splitlines(), 1):
        if struct_name is None:
            match = STRUCT_RE.match(line)
            if match:
                struct_name = match.group("name")
                offset = 0
                known = True
            continue
        if re.match(r"^\s*\};", line):
            rows.append({
                "source": source_name, "line": line_number,
                "struct": struct_name, "field": "(sizeof)",
                "offset": f"0x{offset:X}" if known else "",
                "size": "",
            })
            struct_name = None
            continue
        match = FIELD_RE.match(line)
        if not match:
            # A DECLARATION THIS REGEX CANNOT READ IS NOT A LINE TO SKIP. It
            # occupies storage, so every offset after it moves, and skipping it
            # silently reports the REST of the struct too low by its width.
            #
            # Measured on Thinker's own Buffer, which declares
            #
            #     int (__cdecl *pfcnScrollText)(char *, int, int, int);
            #
            # at 0x8. FIELD_RE does not match a function pointer, so `iFlags`
            # came out at 0x18 against a true 0x1C. The struct's own names give
            # it away: the fields Thinker calls dwordC, dword10 and dword14
            # were reported at 0x8, 0xC and 0x10.
            #
            # Anything ending in `;` inside a struct body is a member, so from
            # here on the honest answer is that the offset is unknown - which
            # this function already knows how to say.
            if re.sub(r"//.*", "", line).strip().endswith(";"):
                known = False
            continue
        size = field_size(match.group("type").strip(), match.group("count"))
        rows.append({
            "source": source_name, "line": line_number,
            "struct": struct_name, "field": match.group("name"),
            "offset": f"0x{offset:X}" if known else "",
            "size": size if size is not None else "",
        })
        if size is None:
            known = False
        elif known:
            offset += size
    return rows


def parse_address_bindings(text, source_name):
    rows = []
    for line_number, line in enumerate(text.splitlines(), 1):
        match = BINDING_RE.match(line)
        if match:
            rows.append({
                "source": source_name, "line": line_number,
                "name": match.group("name"),
                "address": f"0x{int(match.group('address'), 16):08X}",
            })
    return rows


def write_rows(path, fieldnames, rows):
    with path.open("w", newline="", encoding="utf-8") as file:
        writer = csv.DictWriter(file, fieldnames=fieldnames, lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)


def main():
    parser = argparse.ArgumentParser(
        description="Reduce Thinker headers to local layout/address hypotheses")
    parser.add_argument("--source-dir", type=Path, default=DEFAULT_SOURCE_DIR)
    parser.add_argument("--output-dir", type=Path, default=DEFAULT_OUTPUT_DIR)
    args = parser.parse_args()

    output_dir = require_local_artifact_path(
        args.output_dir, "hypothesis output directory")
    source_dir = args.source_dir.resolve()
    if not source_dir.is_dir():
        raise SystemExit(
            f"fetched Thinker sources not found: {source_dir}; run "
            "tools/fetch_external_analysis.py first")

    layout_rows = []
    binding_rows = []
    for source in sorted(source_dir.rglob("*")):
        if source.suffix not in (".h", ".cpp"):
            continue
        text = source.read_text(encoding="utf-8", errors="replace")
        name = source.relative_to(source_dir).as_posix()
        if source.suffix == ".h":
            layout_rows.extend(parse_struct_layouts(text, name))
        binding_rows.extend(parse_address_bindings(text, name))

    output_dir.mkdir(parents=True, exist_ok=True)
    write_rows(output_dir / "thinker-layout-hypotheses.csv",
               ["source", "line", "struct", "field", "offset", "size"],
               layout_rows)
    write_rows(output_dir / "thinker-address-hypotheses.csv",
               ["source", "line", "name", "address"],
               binding_rows)
    structs = len({row["struct"] for row in layout_rows})
    print(
        f"correlate-thinker-layouts: {structs} structs, "
        f"{len(layout_rows)} field rows, {len(binding_rows)} address rows")
    return 0


if __name__ == "__main__":
    sys.exit(main())
