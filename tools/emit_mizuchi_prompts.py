#!/usr/bin/env python3
"""Generate Mizuchi prompt folders from the recovery catalog.

For each chosen function this writes one Mizuchi prompt directory:

    <out>/<hex>/prompt.md      the decompilation task
    <out>/<hex>/settings.yaml  functionName, targetObjectPath, asm,
                               compilerFlags

`functionName` is the SYMBOL, which is not always the catalogued name:
Mizuchi looks that one string up in the compiled object and in the target
object alike, and the 1,179 functions a disassembler labelled had no symbol
at all until one was computed for both sides. tools/recovery_symbols.py
computes it.

The target object is the COFF synthesised by tools/emit_target_object.py
(run it first). compilerFlags carries the per-function flag variant the
original image measurably used: the prologue `push ebp; mov ebp, esp`
(55 8B EC) means the function kept a frame pointer, so it compiled with
/Oy-; otherwise /O2 already omitted the frame and no flag is added. /O1
variants are not detectable from the prologue; retry with /O1 if a body
will not match under /O2. `/Oi-` is added for the 14 CRT intrinsics the
catalogue holds, which CL otherwise refuses to let a unit define at all.

The Ghidra decompilation, when the headless cache has one, is included as
a hypothesis to confirm against the disassembly - AGENTS.md's rule that
decompiler output is a hypothesis applies unchanged.

Output is derived from proprietary bytes: local analysis only, never
committed.
"""

from __future__ import annotations

import argparse
import csv
import json
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import pefile  # noqa: E402
from capstone import CS_ARCH_X86, CS_MODE_32, Cs  # noqa: E402

import recovery_symbols  # noqa: E402

from disasm import DEFAULT_EXE, annotate, load_functions, read_range  # noqa: E402
from emit_target_object import SymbolResolver  # noqa: E402
from emit_translation_unit import Unsettled, emit, load_callees, load_derived  # noqa: E402
from generator_support import parse_body_ranges  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
GHIDRA_DIR = REPO_ROOT / "build" / "ghidra-decompile"
LEDGER = REPO_ROOT / "docs" / "recovery" / "byte-match.csv"
MIZUCHI_DB = REPO_ROOT / "mizuchi-db.json"

FRAME_PROLOGUE = b"\x55\x8b\xec"


def load_ledger() -> dict[int, dict[str, str]]:
    """byte-match.csv keyed by address (int)."""
    if not LEDGER.is_file():
        return {}
    out = {}
    for row in csv.DictReader(LEDGER.open()):
        try:
            out[int(row["address"], 16)] = row
        except (KeyError, ValueError):
            continue
    return out


def load_existing_code() -> dict[str, str]:
    """Recovered C++ code keyed by mangled name, from the Mizuchi index."""
    if not MIZUCHI_DB.is_file():
        return {}
    db = json.loads(MIZUCHI_DB.read_text())
    return {f["name"]: f["cCode"] for f in db.get("decompFunctions", []) if f.get("cCode")}


def disassemble(pe: pefile.PE, address: int, body_ranges: str,
                functions: dict[int, dict[str, str]]) -> str:
    engine = Cs(CS_ARCH_X86, CS_MODE_32)
    lines = []
    for lo, hi in parse_body_ranges(body_ranges):
        code = read_range(pe, lo, hi - lo)
        for instruction in engine.disasm(code, lo):
            note = annotate(instruction.mnemonic, instruction.op_str, functions)
            operand = f" {instruction.op_str}" if instruction.op_str else ""
            lines.append(f"0x{instruction.address:08X}  "
                         f"{instruction.mnemonic}{operand}{note}")
    return "\n".join(lines)


def signature_head(address: int, functions: dict, derived: dict,
                   callees: dict, pe: pefile.PE) -> str:
    """The subject's definition line, as the skeleton emits it."""
    text = emit(address, functions, derived, callees, pe)
    lines = text.splitlines()
    for index, line in enumerate(lines):
        if line.strip() == "// BODY GOES HERE." and index > 0:
            return lines[index - 1]
    raise Unsettled(f"0x{address:08X} produced no body slot")


def ghidra_hypothesis(address: int) -> str | None:
    matches = sorted(GHIDRA_DIR.glob(f"0x{address:08X}-*.c"))
    return matches[0].read_text() if matches else None


def yaml_block(text: str, indent: str = "  ") -> str:
    return "\n".join(f"{indent}{line}" if line else indent.rstrip()
                     for line in text.splitlines())


PROMPT_TEMPLATE = """# Decompile `{name}` (0x{address:08X}, {size} bytes)

You are decompiling a function from a Windows x86 game binary compiled with
Microsoft Visual C++ 6.0 into C++ that recompiles to a byte-identical
instruction stream. This is matching decompilation: functional equivalence
is insufficient, the generated machine code must be identical.

## Contract

- The context file declares opaque class shells, the callees, fixed-address
  globals and VCall shims. Read it. Do not redeclare any of it.
- Define exactly one function, with exactly this definition head:

```cpp
{head}
```

- The class this method belongs to is declared in the context; write the
  out-of-line definition (`Ret Class::method(args) {{ ... }}`).
- C runtime callees (free, malloc, memcpy, ...) are C symbols: if you add
  declarations for any, declare them `extern "C"` so the mangling matches.
- Callee signatures must match the catalog exactly (signedness and pointer
  types are visible in the mangled name).

## Target disassembly (Intel syntax, original bytes)

```asm
{asm}
```

{existing_section}{ghidra_section}
## Rules

- Reproduce the instruction stream: source FORM matters (ternary vs if,
  loop shape, counting direction, signedness). The known VC6 source-form
  rules are in the system prompt.
- Output the complete function definition in a single ```cpp block.
- SHOW THE ENTIRE CODE WITHOUT CROPPING.
"""

EXISTING_SECTION = """## Existing recovery (NOT byte-exact — fix it)

This function has already been reversed into src/, but it does not yet
recompile to a byte-identical instruction stream (last verdict: {tier}).
Start from this code and repair it rather than starting from scratch. Keep
what is correct; change only what the diff demands.

```cpp
{code}
```

"""

GHIDRA_SECTION = """## Ghidra decompilation (hypothesis only)

A decompiler's guess, not ground truth: it drops `this` on __thiscall,
invents arities and normalises callee-pop. Confirm every claim against the
disassembly above before trusting it.

```c
{ghidra}
```

"""


def write_prompt(out_dir: Path, address: int, row: dict[str, str],
                 pe: pefile.PE, functions: dict, derived: dict,
                 callees: dict, existing: tuple[str, str] | None = None,
                 symbols: SymbolResolver | None = None) -> None:
    name = row["name"]
    # `functionName` is looked up in BOTH objects, so it is the symbol rather
    # than the catalogued label - they differ for every function a
    # disassembler named. tools/recovery_symbols.py settles which.
    symbol = symbols.of(address, row) if symbols else name
    size = int(row["size"])
    asm = disassemble(pe, address, row["body_ranges"], functions)
    head = signature_head(address, functions, derived, callees, pe)

    first_bytes = read_range(pe, address, min(3, size))
    flags = ["/Oy-"] if first_bytes == FRAME_PROLOGUE else []
    if recovery_symbols.spelling(name).identifier in recovery_symbols.INTRINSIC:
        # The catalogue holds the statically-linked CRT, so `strlen` and its
        # siblings are functions to RECOVER - and `/O2` implies `/Oi`, under
        # which CL refuses to let a unit define one at all (C2169). Nothing
        # else in these units expands to an intrinsic, so switching them off
        # changes no other byte.
        flags.append("/Oi-")

    ghidra = ghidra_hypothesis(address)
    ghidra_section = GHIDRA_SECTION.format(ghidra=ghidra.rstrip()) if ghidra else ""

    existing_section = ""
    if existing:
        tier, code = existing
        existing_section = EXISTING_SECTION.format(tier=tier, code=code.rstrip())

    prompt = PROMPT_TEMPLATE.format(name=name, address=address, size=size,
                                    head=head, asm=asm,
                                    existing_section=existing_section,
                                    ghidra_section=ghidra_section)

    prompt_dir = out_dir / f"{address:08x}"
    prompt_dir.mkdir(parents=True, exist_ok=True)
    (prompt_dir / "prompt.md").write_text(prompt)

    settings = [
        f'functionName: "{symbol}"',
        f'targetObjectPath: "build/target-objects/{address:08x}.obj"',
    ]
    if flags:
        settings.append(f'compilerFlags: "{" ".join(flags)}"')
    settings.append("asm: |")
    settings.append(yaml_block(asm))
    (prompt_dir / "settings.yaml").write_text("\n".join(settings) + "\n")


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("subjects", nargs="*",
                        help="hex addresses or mangled names")
    parser.add_argument("--unrecovered", type=int, default=0, metavar="N",
                        help="pick the first N unrecovered functions instead")
    parser.add_argument("--not-byte-exact", action="store_true",
                        help="pick recovered functions whose last byte-match verdict "
                             "is not BYTE_EXACT (seeded with their existing src/ code)")
    parser.add_argument("--tier", default="",
                        help="with --not-byte-exact: comma-separated ledger tiers to "
                             "include (e.g. MISMATCH or MISMATCH,NO_COMPILE); default all")
    parser.add_argument("--limit", type=int, default=0, metavar="N",
                        help="with --not-byte-exact: cap the number of prompts")
    parser.add_argument("--out", type=Path, default=REPO_ROOT / "mizuchi" / "prompts")
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    arguments = parser.parse_args()

    functions = load_functions()
    ledger = load_ledger()
    existing_code = load_existing_code() if arguments.not_byte_exact else {}

    if arguments.subjects:
        addresses = []
        for text in arguments.subjects:
            try:
                address = int(text, 16)
            except ValueError:
                for candidate, row in functions.items():
                    if row.get("name") == text:
                        address = candidate
                        break
                else:
                    print(f"{text} is neither a hex address nor a known name",
                          file=sys.stderr)
                    return 1
            addresses.append(address)
    elif arguments.not_byte_exact:
        wanted = {t.strip() for t in arguments.tier.split(",") if t.strip()}
        picked = []
        for address, row in sorted(functions.items()):
            if not (row.get("source_locations") or "").startswith("src/"):
                continue
            tier = (ledger.get(address) or {}).get("tier") or "NOT_IN_LEDGER"
            if tier == "BYTE_EXACT":
                continue
            if wanted and tier not in wanted:
                continue
            picked.append(address)
            if arguments.limit and len(picked) >= arguments.limit:
                break
        addresses = picked
    elif arguments.unrecovered:
        addresses = [address for address, row in sorted(functions.items())
                     if row.get("recovery_state") == "unrecovered"
                     and row.get("name")][:arguments.unrecovered]
    else:
        parser.error("give subjects, --unrecovered N, or --not-byte-exact")

    pe = pefile.PE(str(arguments.exe), fast_load=True)
    derived = load_derived()
    callees = load_callees()
    symbols = SymbolResolver(derived, pe)

    written = 0
    for address in addresses:
        row = functions.get(address)
        if not row or not row.get("name") or not row.get("body_ranges"):
            print(f"skip 0x{address:08X}: not catalogued with a body", file=sys.stderr)
            continue
        existing = None
        code = existing_code.get(row["name"])
        if code:
            tier = (ledger.get(address) or {}).get("tier") or "NOT_IN_LEDGER"
            existing = (tier, code)
        try:
            write_prompt(arguments.out, address, row, pe, functions, derived,
                         callees, existing, symbols)
        except (Unsettled, ValueError) as error:
            print(f"skip 0x{address:08X} {row.get('name')}: {error}", file=sys.stderr)
            continue
        written += 1
        print(f"wrote {arguments.out / f'{address:08x}'}"
              + ("  [seeded with existing recovery]" if existing else ""))

    print(f"{written} prompt(s) written to {arguments.out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
