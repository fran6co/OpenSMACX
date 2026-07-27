#!/usr/bin/env python3
"""Whole-image static recompilation: emit every game function as C++.

The incremental recovery path cannot resolve indirect control flow. 5,064
indirect call sites and 373 indirect jumps live in 1,542 of the 5,673 game
functions (27.2%); `call eax` has no answer while only a fraction of the image
exists in source. Lifting *everything* inverts that: once every original
address has a lifted counterpart, an indirect call is a lookup in a table that
is total by construction. The same move deletes the data problem - map the
image at its canonical base and absolute operands simply work, with no
per-global seam - and reduces the ABI surface from 5,673 hand-derived
prototypes to the 221 imports at the boundary.

This tool runs in two modes.

`--mode skeleton` is a SCALE PROBE, and it is the first thing to run. It emits
the real skeleton - memory model, dispatch table, import shims, one TU shard
per group of functions - but fills each function body with placeholder
statements at a realistic density rather than correct lowerings. The question
it answers is not "is the translation right" but "does an artifact this size
compile and link at all", which is the assumption every later phase rests on
and the cheapest one to falsify. Emitting correct code first and discovering
at 5 million lines that the toolchain will not link it is the expensive
ordering.

`--mode lower` is the real translation and is not implemented yet; the
skeleton's statement budget is deliberately generous so that a real lowering
landing inside it is a fact already measured rather than a hope.

Nothing here is distributable. Output is mechanically derived from the user's
own executable and lands under build/, which is ignored, exactly as
docs/STATIC_RECOMPILATION.md and docs/LEGACY_ISLANDS.md require.
"""

from __future__ import annotations

import argparse
import csv
import json
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import pefile  # noqa: E402
from capstone import CS_ARCH_X86, CS_MODE_32, Cs  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"
FUNCTIONS_CSV = REPO_ROOT / "docs" / "recovery" / "functions.csv"

# Sections that are not ordinary code and must not be lifted. _SELFMOD is
# executable AND writable and holds two copies of one four-function stub that
# rewrites itself; a static translation of self-modifying code is a
# contradiction, so those addresses are emitted as traps and the decision about
# what replaces them is deferred to the point where the game is booting and can
# say whether it needs them at all.
UNLIFTABLE_SECTIONS = {"_selfmod"}

# Statements emitted per decoded instruction in skeleton mode. A real lowering
# of a mid-range instruction - operand fetch, the operation, flag updates,
# writeback - lands around three, so this is the honest figure rather than a
# flattering one. Raise it to see how much headroom the toolchain has.
STATEMENTS_PER_INSTRUCTION = 3


def section_of(pe: pefile.PE, address: int) -> str:
    base = pe.OPTIONAL_HEADER.ImageBase
    for section in pe.sections:
        begin = base + section.VirtualAddress
        end = begin + max(section.Misc_VirtualSize, section.SizeOfRawData)
        if begin <= address < end:
            return section.Name.decode("ascii", "replace").rstrip("\0").lower()
    return ""


def read_bytes(pe: pefile.PE, address: int, length: int) -> bytes:
    base = pe.OPTIONAL_HEADER.ImageBase
    for section in pe.sections:
        begin = base + section.VirtualAddress
        end = begin + max(section.Misc_VirtualSize, section.SizeOfRawData)
        if begin <= address < end:
            offset = section.PointerToRawData + (address - begin)
            return pe.__data__[offset:offset + length]
    return b""


def image_span(pe: pefile.PE) -> tuple[int, int]:
    """(base, size) covering every section, so one flat array models memory."""
    base = pe.OPTIONAL_HEADER.ImageBase
    end = base
    for section in pe.sections:
        section_end = (base + section.VirtualAddress
                       + max(section.Misc_VirtualSize, section.SizeOfRawData))
        end = max(end, section_end)
    return base, ((end - base) + 0xFFF) & ~0xFFF


def body_spans(row: dict) -> list[tuple[int, int]]:
    """The half-open [start, end) ranges a function's code actually occupies.

    `size` is the SUM of these spans, not `end_address - address`, and reading
    that many bytes from the entry point is wrong for every function MSVC
    outlined. 402 of the 5,673 game functions (7.1%) carry a second span in the
    0x0065xxxx-0x0066xxxx cold region, and for all 402 a contiguous read runs
    past the first span into the NEXT function - by up to 2,102 bytes. The
    functions it hits hardest are the largest in the game: probe (28,292 B),
    base_production (16,430 B), tech_trade, terraform, upgrade.

    In skeleton mode that error only inflated an instruction count. Under real
    lowering it would translate a neighbour's bytes as part of this body, so
    the spans are authoritative here and `size` is used only to cross-check
    them. The same confusion was fixed once already for adjustor thunks in
    commit 5e4c362.
    """
    spans = []
    for part in (row.get("body_ranges") or "").split(";"):
        if "-" not in part:
            continue
        low, high = part.split("-", 1)
        try:
            spans.append((int(low, 16), int(high, 16)))
        except ValueError:
            continue
    if not spans:
        address = int(row["address"], 16)
        spans = [(address, address + int(row["size"] or 0))]
    spans.sort()
    return spans


def load_functions(pe: pefile.PE, functions_csv: Path) -> list[dict]:
    with functions_csv.open() as handle:
        rows = list(csv.DictReader(handle))
    functions = []
    for row in rows:
        # external_library rows are CRT and Windows code that the lift must
        # call, never translate: they are supplied by the host toolchain.
        if row["recovery_state"] == "external_library":
            continue
        size = int(row["size"] or 0)
        if not size:
            continue
        address = int(row["address"], 16)
        spans = body_spans(row)
        functions.append({
            "address": address,
            "size": size,
            "spans": spans,
            "span_bytes": sum(high - low for low, high in spans),
            "name": row["name"],
            "section": section_of(pe, address),
            "state": row["recovery_state"],
        })
    functions.sort(key=lambda item: item["address"])
    return functions


def count_instructions(pe: pefile.PE, functions: list[dict]) -> None:
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    for function in functions:
        total = 0
        covered = 0
        for low, high in function["spans"]:
            instructions = list(md.disasm(read_bytes(pe, low, high - low), low))
            total += len(instructions)
            covered += sum(one.size for one in instructions)
        function["instructions"] = total
        # Capstone stopping early means the range holds data, or an encoding it
        # does not know. Either way the byte count is the honest denominator,
        # so fall back to it rather than silently under-budgeting the shard.
        function["fully_decoded"] = covered >= function["span_bytes"] * 0.95
        if not function["fully_decoded"]:
            function["instructions"] = max(total, function["span_bytes"] // 3)


def symbol_for(function: dict) -> str:
    return f"lifted_{function['address']:08x}"


RUNTIME_HEADER = """\
#ifndef OPENSMACX_LIFTED_RUNTIME_H
#define OPENSMACX_LIFTED_RUNTIME_H

#include <cstdint>

#include "static_recompile_runtime.h"

// The whole original image lives in one flat array based at its canonical
// address, so an absolute operand needs no relocation, no per-global binding
// and no class layout: it is an index. This is the single largest
// simplification the whole-image approach buys over per-function recovery,
// where every global costs a hand-declared seam.
extern unsigned char opensmacx_image[];

constexpr uint32_t OpensmacxImageBase = %(base)#010xU;
constexpr uint32_t OpensmacxImageSize = %(size)#010xU;

inline unsigned char *opensmacx_at(uint32_t address) {
    return opensmacx_image + (address - OpensmacxImageBase);
}

inline uint32_t opensmacx_mem32(uint32_t address) {
    uint32_t value;
    __builtin_memcpy(&value, opensmacx_at(address), sizeof(value));
    return value;
}

inline void opensmacx_store32(uint32_t address, uint32_t value) {
    __builtin_memcpy(opensmacx_at(address), &value, sizeof(value));
}

typedef void (*OpensmacxLiftedFunction)(OpensmacxStaticRecompileState &);

// Total by construction: every catalogued game address has an entry, which is
// what makes an indirect call expressible at all.
OpensmacxLiftedFunction opensmacx_dispatch(uint32_t address);

// Reached when a lifted body meets something not yet translated. In skeleton
// mode that is every body; once lowering starts, each trap is a stack trace
// naming exactly one address, which is the whole debugging loop for phase 5.
[[noreturn]] void opensmacx_trap(uint32_t address, const char *reason);

#endif
"""


def write_runtime(out: Path, base: int, size: int) -> None:
    (out / "lifted_runtime.h").write_text(
        RUNTIME_HEADER % {"base": base, "size": size}, encoding="utf-8")


def write_image(out: Path, pe: pefile.PE, base: int, size: int) -> None:
    """Emit the image as initialized data.

    Committing the bytes of the original executable into a C array is exactly
    the derived-code problem the project's policy is about, so the array is
    zero-filled here and populated at startup from the user's own file. The
    generated tree therefore carries the image's SHAPE, never its contents.
    """
    source = out / "lifted_image.cpp"
    source.write_text(
        "#include \"lifted_runtime.h\"\n\n"
        f"// {size} bytes spanning {base:#010x}..{base + size:#010x}.\n"
        "// Zero-filled on purpose: the bytes are loaded at startup from the\n"
        "// user's own executable so no original content enters this tree.\n"
        "unsigned char opensmacx_image[OpensmacxImageSize];\n",
        encoding="utf-8")


def body_statements(function: dict) -> list[str]:
    """Placeholder statements at the density a real lowering would produce."""
    lines = []
    address = function["address"]
    total = function["instructions"] * STATEMENTS_PER_INSTRUCTION
    for index in range(total):
        slot = index % 6
        if slot == 0:
            lines.append(f"    state.eax = opensmacx_mem32(state.esp + {index % 64});")
        elif slot == 1:
            lines.append(f"    state.ecx = state.eax + {index % 251}U;")
        elif slot == 2:
            lines.append("    opensmacx_cmp32(state, state.eax, state.ecx);")
        elif slot == 3:
            lines.append(f"    opensmacx_store32(state.esp + {index % 32}, state.ecx);")
        elif slot == 4:
            lines.append("    state.edx ^= state.ecx;")
        else:
            lines.append(f"    state.esi = opensmacx_mem32({address:#010x}U);")
    return lines


def write_shards(out: Path, functions: list[dict], shards: int) -> list[Path]:
    buckets: list[list[dict]] = [[] for _ in range(shards)]
    for index, function in enumerate(functions):
        buckets[index % shards].append(function)

    paths = []
    for number, bucket in enumerate(buckets):
        path = out / f"lifted_{number:03d}.cpp"
        parts = ["#include \"lifted_runtime.h\"\n\n"]
        for function in bucket:
            symbol = symbol_for(function)
            ranges = ";".join(f"{low:#010x}-{high:#010x}"
                              for low, high in function["spans"])
            parts.append(
                f"// {function['name'][:70]}\n"
                f"// {ranges}  {function['span_bytes']} bytes, "
                f"{function['instructions']} instructions\n"
                f"void {symbol}(OpensmacxStaticRecompileState &state) {{\n")
            if function["section"] in UNLIFTABLE_SECTIONS:
                parts.append(
                    "    (void)state;\n"
                    f"    opensmacx_trap({function['address']:#010x}U,"
                    " \"self-modifying section\");\n}\n\n")
                continue
            parts.append("\n".join(body_statements(function)))
            parts.append(
                f"\n    opensmacx_trap({function['address']:#010x}U,"
                " \"not lowered\");\n}\n\n")
        path.write_text("".join(parts), encoding="utf-8")
        paths.append(path)
    return paths


def write_dispatch(out: Path, functions: list[dict]) -> None:
    declarations = "\n".join(
        f"void {symbol_for(function)}(OpensmacxStaticRecompileState &);"
        for function in functions)
    entries = "\n".join(
        f"    {{{function['address']:#010x}U, &{symbol_for(function)}}},"
        for function in functions)
    (out / "lifted_dispatch.cpp").write_text(
        f"""#include "lifted_runtime.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>

{declarations}

namespace {{

struct Entry {{
    uint32_t address;
    OpensmacxLiftedFunction function;
}};

// Sorted by address at generation time, so the lookup is a binary search over
// a read-only table rather than a hash built at startup.
const Entry Table[] = {{
{entries}
}};

}}  // namespace

OpensmacxLiftedFunction opensmacx_dispatch(uint32_t address) {{
    const Entry *const end = Table + (sizeof(Table) / sizeof(Table[0]));
    const Entry *const found = std::lower_bound(
        Table, end, address,
        [](const Entry &entry, uint32_t key) {{ return entry.address < key; }});
    if (found == end || found->address != address) {{
        // An indirect call landing between catalogued functions means the
        // catalogue is wrong or the target is computed in a way not yet
        // modelled. Failing loudly here is the point.
        opensmacx_trap(address, "indirect call to an address with no function");
    }}
    return found->function;
}}

void opensmacx_trap(uint32_t address, const char *reason) {{
    std::fprintf(stderr, "opensmacx: trap at %#010x: %s\\n",
                 static_cast<unsigned>(address), reason);
    std::abort();
}}
""", encoding="utf-8")


def write_main(out: Path, functions: list[dict]) -> None:
    """A driver whose only job is to keep the linker from discarding the lift."""
    probes = "\n".join(
        f"    resolved += (opensmacx_dispatch({function['address']:#010x}U) != nullptr);"
        for function in functions[:64])
    (out / "lifted_main.cpp").write_text(
        f"""#include "lifted_runtime.h"

#include <cstdio>

int main() {{
    int resolved = 0;
{probes}
    std::printf("resolved %d of 64 probes across %u bytes of image\\n",
                resolved, static_cast<unsigned>(OpensmacxImageSize));
    return resolved == 64 ? 0 : 1;
}}
""", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--functions", type=Path, default=FUNCTIONS_CSV)
    parser.add_argument("--out", type=Path,
                        default=REPO_ROOT / "build" / "lifted")
    parser.add_argument("--mode", choices=("skeleton", "lower"),
                        default="skeleton")
    parser.add_argument("--shards", type=int, default=64,
                        help="translation units, so the build parallelises")
    args = parser.parse_args()

    if args.mode == "lower":
        print("--mode lower is phase 2; run --mode skeleton first and read "
              "the numbers it prints.", file=sys.stderr)
        return 2

    started = time.time()
    pe = pefile.PE(str(args.exe), fast_load=True)
    base, size = image_span(pe)
    functions = load_functions(pe, args.functions)
    count_instructions(pe, functions)

    args.out.mkdir(parents=True, exist_ok=True)
    write_runtime(args.out, base, size)
    write_image(args.out, pe, base, size)
    write_dispatch(args.out, functions)
    write_main(args.out, functions)
    shards = write_shards(args.out, functions, args.shards)

    lines = sum(
        len(path.read_text(encoding="utf-8").splitlines()) for path in shards)
    instructions = sum(function["instructions"] for function in functions)
    undecoded = sum(
        1 for function in functions if not function["fully_decoded"])
    unliftable = sum(1 for function in functions
                     if function["section"] in UNLIFTABLE_SECTIONS)
    outlined = sum(1 for function in functions if len(function["spans"]) > 1)

    manifest = {
        "mode": args.mode,
        "image_base": f"{base:#010x}",
        "image_size": size,
        "functions": len(functions),
        "outlined_functions": outlined,
        "code_bytes": sum(function["span_bytes"] for function in functions),
        "instructions": instructions,
        "statements_per_instruction": STATEMENTS_PER_INSTRUCTION,
        "generated_lines": lines,
        "shards": len(shards),
        "not_fully_decoded": undecoded,
        "unliftable_section_functions": unliftable,
        "seconds": round(time.time() - started, 1),
    }
    (args.out / "manifest.json").write_text(
        json.dumps(manifest, indent=2) + "\n", encoding="utf-8")

    print(f"functions            {len(functions)}")
    print(f"instructions         {instructions}")
    print(f"generated lines      {lines} across {len(shards)} shards")
    print(f"image span           {base:#010x} + {size} bytes")
    print(f"not fully decoded    {undecoded}")
    print(f"unliftable section   {unliftable}")
    print(f"generation seconds   {manifest['seconds']}")
    print(f"\nwrote {args.out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
