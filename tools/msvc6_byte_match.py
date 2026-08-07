#!/usr/bin/env python3
"""Recompile recovered leaf bodies with the ORIGINAL compiler and compare.

Every verification route in this repository is bounded by something. The
differential oracles are bounded by the fault wall - a body that walks a
pointer chain cannot be called with a staged receiver. The SMT prover is
bounded by decidability, and its cost grows with the body. Byte matching has
neither bound: if a recovered body recompiles to the original instruction
sequence, the cost of believing it is O(1) in the body's size, its call graph,
its loops, its COM and its SEH.

So this asks the question directly. SMACX shipped built by Microsoft's 32-bit
C/C++ Optimizing Compiler 12.00.8168 (Visual C++ 6.0 RTM). That compiler is
installed under ~/opt/vc6 and runs under Wine. This compiles five catalogued
recoveries with it and compares the result against the pinned executable at
the address each was recovered from.

WHAT IS COMPARED, precisely - over-normalising would manufacture a match:

  * Both sides are decoded with capstone in 32-bit mode and reduced to their
    MNEMONIC NAMES. Operands are discarded wholesale. That discards register
    allocation, absolute addresses, relocation targets, immediates, and branch
    displacements - none of which a fresh compile can be expected to
    reproduce, since the original's addresses do not exist in an object file.
  * It also discards the `ret` pop count, because capstone spells `ret 8` and
    `ret` with the same mnemonic. That is a real loss of resolution and the
    byte lengths are reported alongside so it cannot hide.
  * NOTHING ELSE IS MERGED. `lea` is not `mov`, `imul` is not `add`, `je` is
    not `jne`, `inc` is not `add`. A strength reduction the original performed
    and the recompile did not is a MISMATCH, which is the whole point: it is
    the signal that says the optimisation settings are not the original's.
  * On the object side, trailing `int3` and `nop` alignment padding is
    stripped. On the original side the body's catalogued spans are read and
    concatenated - `size` is their SUM, never `end - address`.

THE CANDIDATE RULE, fixed before any measurement: `source_complete`, LEAF (no
calls out), and NO STRUCT MEMBER ACCESS. The last is load-bearing. A wrong
`/Zp` packing and a wrong `/O` level produce the same symptom - an offset that
does not match - so a body that touches a struct field turns an interpretable
mismatch into an uninterpretable one.

THE FLAGS. The pre-registration fixed `/c /O2 /Gy /GR-` and that run is kept
reproducible under `--preregistered`. It scored 0/5, every case diverging at
instruction #0, `push` against `mov`: `/O2` implies `/Oy`, and the shipped
executable keeps its frame pointers. `/Oy-` is therefore part of the shipped
build's settings and is part of the default here. `/O1` was measured too and
is decisively NOT it - it leaves `* 10000` as an `imul` where the original
strength-reduced it to four `lea`s.

Usage:
    msvc6_byte_match.py            # the five cases, controls, diagnostics
    msvc6_byte_match.py --preregistered      # the run as pre-registered
    msvc6_byte_match.py --flags "/c /O1 /Gy /GR- /Oy-"   # flag sensitivity
    msvc6_byte_match.py --keep DIR # leave the units and objects behind

Output includes disassembly of proprietary bytes as MNEMONIC NAMES only: no
opcode byte ever leaves this tool, and nothing it prints is committed.
"""

from __future__ import annotations

import argparse
import csv
import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile
from dataclasses import dataclass, field
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"
FUNCTIONS_CSV = REPO_ROOT / "docs" / "recovery" / "functions.csv"

VC6_ROOT = Path(os.environ.get("VC6_ROOT", Path.home() / "opt" / "vc6"))
VC6_CL = VC6_ROOT / "BIN" / "CL.EXE"
VC6_PREFIX = VC6_ROOT / ".wineprefix"

# The flags the pre-registration fixed, kept verbatim so its run stays
# reproducible, and the flags this tool then measured the shipped build to
# have used. They differ in `/Oy-` alone.
PREREGISTERED_FLAGS = "/c /O2 /Gy /GR-"
MEASURED_FLAGS = "/c /O2 /Gy /GR- /Oy-"

STDINT = ("typedef int int32_t;\n"
          "typedef unsigned int uint32_t;\n"
          "typedef unsigned char uint8_t;\n")


@dataclass
class Case:
    """One catalogued recovery, and what it takes to compile it alone."""
    address: int
    label: str
    # WHERE the body lives is deliberately NOT recorded here. It was, as
    # "src/foo.cpp:LINE", and it went stale three times: every splice above one
    # of these five renumbers it, and a hand-copied line number cannot survive
    # a tool whose whole job is to insert lines. `functions.csv` is the one
    # place that tracks it and `mizuchi_writeback.py` keeps it current, so it
    # is read from there - see `case_location`. The ADDRESS is the identity.
    preamble: str = ""   # declarations the extracted body needs, nothing more
    # (old, new): a ONE-OPERATION edit that must change the mnemonic sequence.
    perturb: tuple[str, str] = ("", "")
    # (old, new, why): a hypothesis about a mismatch, run and reported apart
    # from the verdict. It NEVER counts as a match - the point of asking is to
    # separate "the compiler cannot reproduce this" from "the recovered source
    # says something the original did not", and those need different fixes.
    diagnostic: tuple[str, str, str] = ("", "", "")
    shape: str = ""
    body: str = field(default="", init=False)


# Five bodies that satisfy the pre-registered rule, chosen to span the three
# calling conventions and the optimisations that distinguish /O2 from /Od:
# constant-divisor strength reduction, constant-multiplier lea chains, a
# variable-divisor idiv, a shift, and a branch.
CASES = [
    Case(address=0x0042A020,
         label="?UNK1@Datalink@@QAEHHH@Z",
         preamble="class Datalink { public: int UNK1(int, int); };\n",
         perturb=("+ static_cast<uint32_t>(a2)", "- static_cast<uint32_t>(a2)"),
         shape="__thiscall; multiply by constant 10000, add"),
    Case(address=0x0042A040,
         label="?parse_id@Datalink@@QAEXHPAUDatalinkID@@PAH@Z",
         preamble=("struct DatalinkID;\n"
                   "class Datalink { public: "
                   "void parse_id(int, DatalinkID *, int *); };\n"),
         perturb=("static_cast<uint32_t>(id) - ", "static_cast<uint32_t>(id) + "),
         shape="__thiscall; signed divide by constant 10000, two stores"),
    Case(address=0x0050BA00,
         label="?bitmask@@YAXHPAHPAH@Z",
         perturb=("1 << (input & 7)", "1 >> (input & 7)"),
         diagnostic=("uint32_t input", "int input",
                     "the original divides SIGNED (cdq/and/add/sar); the "
                     "recovery declares the parameter uint32_t, and the "
                     "mangled name ?bitmask@@YAXHPAHPAH@Z says int"),
         shape="__cdecl; signed divide by 8, variable shift, two stores"),
    # This case USED to carry a diagnostic proposing that the body's ternary
    # be rewritten as an `if`, on the theory that the ternary's `jne` with
    # swapped arms was why it did not match. The hypothesis was right about
    # the ternary and wrong about the fix: neither spelling matches while a
    # `rounded` TEMPORARY exists, because the extra live value stops VC6
    # spilling the divisor to esi. Incrementing the quotient in place is
    # byte-exact, the body now does that, and the diagnostic is retired -
    # a hypothesis that has been settled is not a hypothesis.
    Case(address=0x00532A50,
         label="sub_532a50",
         perturb=("++quotient", "--quotient"),
         shape="__stdcall; variable idiv, remainder, conditional round up"),
    Case(address=0x00559210,
         label="sub_559210",
         perturb=("value / step * step", "value / step + step"),
         shape="__cdecl; variable idiv then multiply back"),
]


# ---------------------------------------------------------------- the original

def body_spans(row: dict) -> list[tuple[int, int]]:
    """Half-open [start, end) ranges the body occupies. `size` is their SUM."""
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


def load_rows() -> dict[int, dict]:
    with FUNCTIONS_CSV.open() as handle:
        return {int(row["address"], 16): row for row in csv.DictReader(handle)}


def original_bytes(pe, row: dict) -> bytes:
    import generator_support
    chunks = []
    for low, high in body_spans(row):
        chunk = generator_support.read_bytes(pe, low, high - low)
        if len(chunk) != high - low:
            raise ValueError(f"short read at 0x{low:08X}")
        chunks.append(chunk)
    return b"".join(chunks)


# ------------------------------------------------------------------ the source

def case_location(rows: dict, address: int) -> str:
    """Where `functions.csv` says this recovery lives, right now.

    The catalogue records `source_locations` as a `;`-separated list because a
    function can be spliced into more than one place; every case here is a
    single-site leaf, so the first entry is the body.
    """
    row = rows.get(address)
    if row is None:
        raise ValueError(f"0x{address:08X} is not in the catalogue")
    location = (row.get("source_locations") or "").split(";")[0].strip()
    if not location:
        raise ValueError(f"0x{address:08X} has no source_locations")
    return location


def extract_body(source: str) -> str:
    """The committed recovery verbatim, from its catalogued line to its `}`.

    functions.csv points at a line inside the doc comment that precedes the
    definition, so the extract usually opens mid-comment. It is re-opened with
    `/*` rather than trimmed: the comment is part of what was recovered, and
    trimming it would mean this tool decides where a body starts.
    """
    path_part, _, line_part = source.rpartition(":")
    path = REPO_ROOT / path_part
    lines = path.read_text().splitlines()
    start = int(line_part) - 1
    depth, opened, out = 0, False, []
    for line in lines[start:]:
        out.append(line)
        depth += line.count("{") - line.count("}")
        if "{" in line:
            opened = True
        if opened and depth <= 0:
            break
    else:
        raise ValueError(f"{source}: no closing brace within the file")
    text = "\n".join(out) + "\n"
    if "*/" in text.split("{", 1)[0]:
        text = "/*\n" + text
    return text


# ------------------------------------------------------------------ the object

def coff_function(data: bytes) -> bytes:
    """The single external .text symbol's code, padding stripped.

    A COFF object is walked directly rather than through dumpbin: capstone has
    to see the same bytes the comparison claims to be about, and a text parse
    of another disassembler's output is a second decoder to keep honest.
    """
    machine, nsections, _, sym_ptr, nsyms, opt_size, _ = struct.unpack_from(
        "<HHIIIHH", data, 0)
    if machine != 0x14C:
        raise ValueError(f"not an i386 object (machine 0x{machine:04X})")

    sections = []
    offset = 20 + opt_size
    for _ in range(nsections):
        name = data[offset:offset + 8].rstrip(b"\0").decode("ascii", "replace")
        raw_size, raw_ptr = struct.unpack_from("<II", data, offset + 16)
        sections.append((name, raw_size, raw_ptr))
        offset += 40

    strings = sym_ptr + nsyms * 18
    found = []
    for index in range(nsyms):
        entry = data[sym_ptr + index * 18:sym_ptr + index * 18 + 18]
        if entry[:4] == b"\0\0\0\0":
            at = struct.unpack_from("<I", entry, 4)[0] + strings
            name = data[at:data.index(b"\0", at)].decode("ascii", "replace")
        else:
            name = entry[:8].rstrip(b"\0").decode("ascii", "replace")
        value, section, _, storage, _ = struct.unpack_from("<IhHBB", entry, 8)
        if storage != 2 or section <= 0 or section > len(sections):
            continue
        if not sections[section - 1][0].startswith(".text"):
            continue
        found.append((name, value, section))

    if len(found) != 1:
        raise ValueError(f"expected one external .text symbol, found {found}")
    _, value, section = found[0]
    _, raw_size, raw_ptr = sections[section - 1]
    code = data[raw_ptr + value:raw_ptr + raw_size]
    return code.rstrip(b"\xcc").rstrip(b"\x90")


def compile_unit(text: str, work: Path, stem: str, flags: str) -> bytes:
    source = work / f"{stem}.cpp"
    obj = work / f"{stem}.obj"
    source.write_text(text)
    command = ["wine", str(VC6_CL), "/nologo", *flags.split(),
               f"/Fo{obj.name}", source.name]
    environment = dict(os.environ,
                       WINEPREFIX=str(VC6_PREFIX),
                       WINEDEBUG="-all",
                       INCLUDE=r"Z:\home\fran6co\opt\vc6\INCLUDE",
                       LIB=r"Z:\home\fran6co\opt\vc6\LIB")
    result = subprocess.run(command, cwd=work, env=environment,
                            capture_output=True, text=True)
    if not obj.is_file():
        raise ValueError(f"CL produced no object: {result.stdout}{result.stderr}")
    return coff_function(obj.read_bytes())


# --------------------------------------------------------------- the comparison

def mnemonics(code: bytes, base: int = 0) -> list[str]:
    from capstone import CS_ARCH_X86, CS_MODE_32, Cs
    engine = Cs(CS_ARCH_X86, CS_MODE_32)
    return [i.mnemonic for i in engine.disasm(code, base)]


def first_divergence(left: list[str], right: list[str]) -> int | None:
    for index in range(min(len(left), len(right))):
        if left[index] != right[index]:
            return index
    return None if len(left) == len(right) else min(len(left), len(right))


def compare(original: list[str], rebuilt: list[str]) -> tuple[bool, str]:
    index = first_divergence(original, rebuilt)
    if index is None:
        return True, ""
    def at(seq, i):
        return seq[i] if i < len(seq) else "<end>"
    return False, (f"#{index}: original {at(original, index)!r} vs "
                   f"rebuilt {at(rebuilt, index)!r}")


# ---------------------------------------------------------------------- driving

def available() -> str:
    if not VC6_CL.is_file():
        return f"{VC6_CL} is absent"
    if shutil.which("wine") is None:
        return "wine is not on PATH"
    if not DEFAULT_EXE.is_file():
        return f"{DEFAULT_EXE} is absent"
    return ""


def run(flags: str, work: Path, verbose: bool) -> list[dict]:
    import pefile
    pe = pefile.PE(str(DEFAULT_EXE), fast_load=True)
    rows = load_rows()
    results = []
    for case in CASES:
        row = rows[case.address]
        case.body = extract_body(case_location(rows, case.address))
        original = original_bytes(pe, row)
        unit = STDINT + case.preamble + "\n" + case.body
        rebuilt = compile_unit(unit, work, f"f{case.address:08x}", flags)
        left, right = mnemonics(original, case.address), mnemonics(rebuilt)
        matched, note = compare(left, right)
        results.append(dict(case=case, original=original, rebuilt=rebuilt,
                            left=left, right=right, matched=matched, note=note))
        if verbose:
            print(f"    original: {' '.join(left)}")
            print(f"    rebuilt : {' '.join(right)}")
    return results


def control_perturbation(result: dict, work: Path, flags: str) -> str:
    """A one-operation edit to a MATCHING body must be reported as a mismatch."""
    case = result["case"]
    old, new = case.perturb
    if old not in case.body:
        return f"perturbation {old!r} not present in the body"
    unit = STDINT + case.preamble + "\n" + case.body.replace(old, new, 1)
    rebuilt = compile_unit(unit, work, f"p{case.address:08x}", flags)
    matched, note = compare(result["left"], mnemonics(rebuilt))
    if matched:
        return f"FAILED: {old!r} -> {new!r} still compared equal"
    return f"OK: {old!r} -> {new!r} reported a mismatch at {note}"


def diagnose(result: dict, work: Path, flags: str) -> str:
    """Ask why a mismatch mismatched. Never counted, never a verdict."""
    case = result["case"]
    old, new, why = case.diagnostic
    if not old:
        return "no hypothesis recorded"
    if old not in case.body:
        return f"hypothesis text {old!r} not present in the body"
    unit = STDINT + case.preamble + "\n" + case.body.replace(old, new, 1)
    rebuilt = compile_unit(unit, work, f"d{case.address:08x}", flags)
    matched, note = compare(result["left"], mnemonics(rebuilt))
    outcome = ("the variant reproduces the original exactly"
               if matched else f"the variant still differs at {note}")
    return f"{why} -> {outcome}"


def control_identity(result: dict) -> str:
    """Handed the same sequence twice, the comparator must say match."""
    matched, _ = compare(result["right"], list(result["right"]))
    return "OK: identical objects compare equal" if matched else "FAILED"


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--flags", default=MEASURED_FLAGS,
                        help=f"CL flags; default {MEASURED_FLAGS!r}")
    parser.add_argument("--preregistered", action="store_true",
                        help=f"reproduce the pre-registered run, "
                             f"{PREREGISTERED_FLAGS!r}")
    parser.add_argument("--keep", type=Path,
                        help="directory to leave the units and objects in")
    parser.add_argument("--verbose", action="store_true",
                        help="print both mnemonic sequences per function")
    args = parser.parse_args()
    if args.preregistered:
        args.flags = PREREGISTERED_FLAGS

    reason = available()
    if reason:
        print(f"SKIP: {reason}. This needs Visual C++ 6.0 (12.00.8168) under "
              f"Wine and the pinned executable; set VC6_ROOT to relocate it.")
        return 0

    holder = None
    if args.keep:
        args.keep.mkdir(parents=True, exist_ok=True)
        work = args.keep
    else:
        holder = tempfile.TemporaryDirectory()
        work = Path(holder.name)

    try:
        results = run(args.flags, work, args.verbose)
        print(f"cl {args.flags}")
        matches = 0
        for result in results:
            case = result["case"]
            verdict = "MATCH" if result["matched"] else "mismatch"
            matches += bool(result["matched"])
            print(f"0x{case.address:08X}  {case.label}")
            print(f"    {case.shape}")
            print(f"    original {len(result['original']):3d} B / "
                  f"{len(result['left']):2d} mnemonics    "
                  f"rebuilt {len(result['rebuilt']):3d} B / "
                  f"{len(result['right']):2d} mnemonics")
            print(f"    {verdict}" + (f"    first divergence {result['note']}"
                                      if result["note"] else ""))
        print(f"{matches}/{len(results)} match")

        print("controls:")
        matched = [r for r in results if r["matched"]]
        if matched:
            print(f"  perturbation: "
                  f"{control_perturbation(matched[0], work, args.flags)}")
        else:
            print("  perturbation: not run - nothing matched to perturb")
        print(f"  identity: {control_identity(results[0])}")

        missed = [r for r in results if not r["matched"] and r["case"].diagnostic[0]]
        if missed:
            print("diagnostics - NOT part of the verdict above, and the "
                  "committed source is not edited:")
            for result in missed:
                print(f"  0x{result['case'].address:08X}  "
                      f"{diagnose(result, work, args.flags)}")
        return 0
    finally:
        if holder:
            holder.cleanup()


if __name__ == "__main__":
    sys.exit(main())
