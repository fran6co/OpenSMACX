#!/usr/bin/env python3
"""Generate hybrid-runtime oracles for functions whose signature is derivable.

`unproven_recovered` says 96.9% of recovered bytes have never been executed
against the original. Every proof that exists was hand-written, which is why
there are 49 of them against 2,501 unproven functions. This turns the cheap
cases into a build step.

WHY THIS NEEDS NO SYMBOL LOOKUP, which is the part that makes it possible.

The recovered source is compiled by MinGW with Itanium mangling; the names in
the inventory are MSVC names read out of the original binary. There is no link
between them, so a generated oracle cannot call the recovered function by name.
It does not have to. In the hybrid the recovered body is PATCHED OVER the
original's address, so one call site serves both: with the redirect suspended
the address runs the original, with it active the same address runs the
recovery. `suspend_redirect_at` / `resume_redirect_at` in dllmain.cpp are the
switch, and src/sprite_oracle.cpp uses them by hand already.

WHAT IS COMPARED, and why a return value alone would be worthless. The
candidates here take no arguments - that is what makes their signature
derivable - and they are not pure: `base_minerals` and `base_nutrient` compute
into global arrays and return nothing. So the comparison snapshots .data/.bss,
runs one side, captures the result AND the memory it produced, restores, runs
the other side, and compares both. A recovery that returns the right value and
writes the wrong globals fails here, which is the whole point.

The process is left exactly as it was found: the snapshot is restored after the
second call too, so running an oracle cannot perturb the game that follows it.

WHAT IS DELIBERATELY NOT GENERATED:

  * anything taking arguments - staging them needs types this cannot infer
    safely, and a wrong argument proves nothing while looking like a proof;
  * `sub_*` functions, whose arity is unknown; `ret N` is the only statement of
    it and reading that is a separate job;
  * anything already proven, which would inflate the count without adding
    evidence.
"""

from __future__ import annotations

import argparse
import csv
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_FUNCTIONS = REPO_ROOT / "docs" / "recovery" / "functions.csv"
DEFAULT_PROVEN = REPO_ROOT / "docs" / "recovery" / "proven.csv"
DEFAULT_OUTPUT = REPO_ROOT / "src" / "generated_signature_oracle.cpp"
DEFAULT_DLLMAIN = REPO_ROOT / "src" / "dllmain.cpp"

# MSVC mangling: ?name@@<scope><conv><return><args>Z. `Y` is a free function;
# the character after it is the calling convention, then the return type, then
# the parameter list. `X` alone as the parameter list means (void).
# A NON-EMPTY parameter list is terminated by "@Z"; "(void)" is the bare "XZ"
# form. Missing that cost a measurement: `?energy_limit@@YAHH@Z` matched
# nothing, so a whole class of one-argument functions read as absent.
FREE_WITH_ARGS = re.compile(r"^\?([A-Za-z_][\w]*)@@Y([A-Z])([A-Z])([A-Z]+)@Z$")
FREE_NO_ARG = re.compile(r"^\?([A-Za-z_][\w]*)@@Y([A-Z])([A-Z])XZ$")

# Argument types that can be passed as a plain integer. A pointer or a struct
# would need a staged object, and guessing one proves nothing while looking
# like a proof.
INT_ARGS = {"H": "int", "I": "unsigned int", "J": "long", "K": "unsigned long"}

# Seeds every argument is driven with. Small, boundary and negative values,
# which is where a transcription slip shows up; the same reasoning as the
# lifted oracle's case list.
ARGUMENT_SEEDS = (0, 1, -1, 2, 7, 0x7FFFFFFF, -0x80000000, 0x55555555)


def redirected_addresses(dllmain_path: Path) -> set[int]:
    """Addresses the hybrid actually patches, read from the table itself.

    NOT the inventory's `redirect_exports` column, which does not agree with it:
    selecting on that column produced 39 oracles that every one failed with
    "cannot suspend redirect". The proof calls one address twice - redirect
    suspended for the original, restored for the recovery - so an address
    absent from this table has no second body to reach.
    """
    text = dllmain_path.read_text()
    start = text.index("const RedirectSpec specs[] = {")
    end = text.index("\n    };", start)
    return {int(value, 16) for value in
            re.findall(r"^\s*(0x[0-9A-Fa-f]{8}),\s*$", text[start:end], re.M)}

# Return types this can express. Anything else - a pointer, a struct, a class
# reference - is skipped rather than guessed at.
RETURN_TYPES = {
    "X": ("void", "void"),
    "H": ("int", "int"),
    "I": ("unsigned int", "unsigned"),
    "J": ("long", "long"),
    "K": ("unsigned long", "unsigned long"),
    "_N": ("bool", "bool"),
    "E": ("unsigned char", "unsigned char"),
    "D": ("char", "char"),
    "F": ("short", "short"),
    "G": ("unsigned short", "unsigned short"),
    "M": ("float", "float"),
    "N": ("double", "double"),
}

CONVENTIONS = {"A": "__cdecl", "G": "__stdcall", "I": "__fastcall"}


def candidates(functions_path: Path, proven_path: Path,
               redirected: set) -> list[dict]:
    proven = set()
    if proven_path.exists():
        with proven_path.open(newline="") as handle:
            proven = {int(row["address"], 16) for row in csv.DictReader(handle)}
    found = []
    with functions_path.open(newline="") as handle:
        for row in csv.DictReader(handle):
            address = int(row["address"], 16)
            if row["recovery_state"] != "source_complete":
                continue
            if row["binary_kind"] != "game" or address in proven:
                continue
            if address not in redirected:
                continue
            name = row["name"]
            match = FREE_WITH_ARGS.match(name)
            if match:
                symbol, convention, ret, args = match.groups()
                if any(code not in INT_ARGS for code in args):
                    continue
                arg_types = [INT_ARGS[code] for code in args]
            else:
                match = FREE_NO_ARG.match(name)
                if not match:
                    continue
                symbol, convention, ret = match.groups()
                arg_types = []
            if convention not in CONVENTIONS or ret not in RETURN_TYPES:
                continue
            found.append({
                "address": address,
                "name": name,
                "symbol": symbol,
                "convention": CONVENTIONS[convention],
                "return": RETURN_TYPES[ret][0],
                "args": arg_types,
                "size": int(row["size"]),
                "source": row["source_locations"],
            })
    return sorted(found, key=lambda item: item["address"])


def emit(rows: list[dict], claim_proofs: bool = False) -> str:
    out: list[str] = []
    w = out.append
    w("// GENERATED by tools/generate_signature_oracles.py - do not edit.")
    w("//")
    w("// Each function below is executed in its ORIGINAL form, at its canonical")
    w("// address inside the hybrid process, and compared against the recovered")
    w("// implementation reached through the same address with the redirect")
    w("// restored. Return value AND the globals each side produces are compared;")
    w("// these functions return void as often as not and do their work in .data.")
    w("//")
    w("// The markers below are the input to `unproven_recovered` in")
    w("// docs/recovery/summary.json; tools/export_proven_functions.py decides")
    w("// what counts.")
    w("//")
    # THE MARKERS ARE WITHHELD, and that is deliberate.
    #
    # `export_proven_functions.py` reads `PROVEN-AGAINST-ORIGINAL:` and lowers
    # `unproven_recovered` for every one it finds. Emitting them for oracles
    # that cannot be run to completion would be exactly the flattering figure
    # this project keeps guarding against.
    #
    # They cannot be run to completion yet. The comparison snapshots .data/.bss
    # and restores it between the two calls, which is correct for a function
    # whose effects live there - and wrong for one that allocates or touches a
    # file. `?load_deswin_sprites@@YAXXZ` loads sprites and `?auto_save@@YAXXZ`
    # writes one; calling either twice moves the heap on while the restore puts
    # back the pointers from before it. Three runs out of three, the game then
    # died with an unhandled division by zero at 0x004991DD, inside
    # ?draw_labs@ReportWin@@QAEXXZ - UNRECOVERED original code reading state the
    # restore had made inconsistent.
    #
    # What is missing is a way to know which functions confine themselves to
    # .data/.bss, or a harness whose process does not have to survive the
    # oracle. Until one exists these run and report, and claim nothing.
    if claim_proofs:
        for row in rows:
            w(f"// PROVEN-AGAINST-ORIGINAL: 0x{row['address']:08X}  {row['name']}")
    else:
        w("// PROOFS NOT CLAIMED: see the note above emit() in")
        w("// tools/generate_signature_oracles.py. These oracles run and report;")
        w("// they do not yet lower unproven_recovered.")
    w("")
    w('#include "stdafx.h"')
    w('#include "generated_signature_oracle.h"')
    w("")
    w('#include "runtime_oracle.h"')
    w("")
    w("#include <cstdio>")
    w("#include <cstdint>")
    w("#include <cstring>")
    w("#include <vector>")
    w("")
    w("namespace {")
    w("")
    w("// .data and .bss of the original image. The recovered bodies write here")
    w("// and so do the originals; comparing it is what makes a void-returning")
    w("// function testable at all.")
    w("constexpr uintptr_t GlobalsBegin = 0x00682000U;")
    w("constexpr uintptr_t GlobalsEnd   = 0x009C21F8U;")
    w("constexpr size_t GlobalsSize = GlobalsEnd - GlobalsBegin;")
    w("")
    w("void snapshot(std::vector<uint8_t> &into) {")
    w("    into.resize(GlobalsSize);")
    w("    std::memcpy(into.data(), reinterpret_cast<const void *>(GlobalsBegin),")
    w("                GlobalsSize);")
    w("}")
    w("")
    w("void restore(const std::vector<uint8_t> &from) {")
    w("    std::memcpy(reinterpret_cast<void *>(GlobalsBegin), from.data(),")
    w("                GlobalsSize);")
    w("}")
    w("")
    w("// Where the two images first disagree, for a message that names an address")
    w("// rather than saying only that something differed.")
    w("bool same_globals(const std::vector<uint8_t> &a,")
    w("                  const std::vector<uint8_t> &b, uintptr_t *where) {")
    w("    for (size_t index = 0; index < GlobalsSize; ++index) {")
    w("        if (a[index] != b[index]) {")
    w("            *where = GlobalsBegin + index;")
    w("            return false;")
    w("        }")
    w("    }")
    w("    return true;")
    w("}")
    w("")
    w("}  // namespace")
    w("")

    for row in rows:
        ret = row["return"]
        conv = row["convention"]
        addr = f"0x{row['address']:08X}"
        fn = f"verify_{row['symbol']}_{row['address']:08x}"
        args = row["args"]
        params = ", ".join(args) if args else ""
        w(f"// {row['name']}")
        w(f"// recovered in {row['source']}")
        w(f"static bool {fn}() {{")
        w(f"    typedef {ret} ({conv} *Callable)({params});")
        w(f"    Callable target = reinterpret_cast<Callable>({addr}U);")
        w("    std::vector<uint8_t> before, after_original, after_recovered;")
        w("    bool passed = true;")
        if args:
            # One row per case, so a failure names the arguments that caused it.
            # long long, not the argument type: a seed of -1 or INT_MIN is a
            # narrowing conversion into an unsigned parameter and the build
            # refuses it. The call site casts each value back to its real type.
            w(f"    static const long long cases[][{len(args)}] = {{")
            for index in range(len(ARGUMENT_SEEDS)):
                values = ", ".join(
                    str(ARGUMENT_SEEDS[(index + position) % len(ARGUMENT_SEEDS)])
                    for position in range(len(args)))
                w(f"        {{{values}}},")
            w("    };")
            w("    for (const auto &argv : cases) {")
            call = "target(" + ", ".join(
                f"({args[i]})argv[{i}]" for i in range(len(args))) + ")"
            indent = "        "
        else:
            w("    {")
            call = "target()"
            indent = "        "
        w(f"{indent}snapshot(before);")
        w(f"{indent}if (!suspend_redirect_at({addr}U)) {{")
        w(f'{indent}    std::printf("  {row["name"]}: cannot suspend redirect\\n");')
        w(f"{indent}    return false;")
        w(f"{indent}}}")
        if ret == "void":
            w(f"{indent}{call};")
        else:
            w(f"{indent}const {ret} original_result = {call};")
        w(f"{indent}snapshot(after_original);")
        w(f"{indent}restore(before);")
        w(f"{indent}if (!resume_redirect_at({addr}U)) {{")
        w(f'{indent}    std::printf("  {row["name"]}: cannot resume redirect\\n");')
        w(f"{indent}    return false;")
        w(f"{indent}}}")
        if ret == "void":
            w(f"{indent}{call};")
        else:
            w(f"{indent}const {ret} recovered_result = {call};")
        w(f"{indent}snapshot(after_recovered);")
        w(f"{indent}restore(before);   // leave the process as it was found")
        if ret != "void":
            w(f"{indent}if (original_result != recovered_result) {{")
            w(f'{indent}    std::printf("  {row["name"]}: return value differs\\n");')
            w(f"{indent}    passed = false;")
            w(f"{indent}}}")
        w(f"{indent}uintptr_t where = 0;")
        w(f"{indent}if (!same_globals(after_original, after_recovered, &where)) {{")
        w(f'{indent}    std::printf("  {row["name"]}: globals differ, first at %p\\n",')
        w(f"{indent}                reinterpret_cast<void *>(where));")
        w(f"{indent}    passed = false;")
        w(f"{indent}}}")
        w("    }")
        w("    return passed;")
        w("}")
        w("")

    w("bool run_generated_signature_oracles() {")
    w("    bool passed = true;")
    for row in rows:
        fn = f"verify_{row['symbol']}_{row['address']:08x}"
        w(f"    passed &= {fn}();")
    w(f"    std::printf(\"generated signature oracles: %d function(s)\\n\", {len(rows)});")
    w("    return passed;")
    w("}")
    return "\n".join(out) + "\n"


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--functions", type=Path, default=DEFAULT_FUNCTIONS)
    parser.add_argument("--proven", type=Path, default=DEFAULT_PROVEN)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--dllmain", type=Path, default=DEFAULT_DLLMAIN)
    parser.add_argument("--claim-proofs", action="store_true",
                        help="emit PROVEN-AGAINST-ORIGINAL markers. Only once "
                             "the oracles can run without destabilising the "
                             "process - see the note in emit().")
    parser.add_argument("--check", action="store_true",
                        help="fail if the output would differ; write nothing")
    parser.add_argument("--list", action="store_true",
                        help="print the candidates and stop")
    options = parser.parse_args(argv)

    rows = candidates(options.functions, options.proven,
                      redirected_addresses(options.dllmain))
    if options.list:
        for row in rows:
            print(f"0x{row['address']:08X}  {row['size']:5d} B  {row['name']}")
        print(f"{len(rows)} candidate(s), "
              f"{sum(row['size'] for row in rows)} bytes")
        return 0

    text = emit(rows, options.claim_proofs)
    if options.check:
        current = options.output.read_text() if options.output.exists() else ""
        if current != text:
            print(f"error: {options.output} is stale; regenerate it",
                  file=sys.stderr)
            return 1
        print(f"generated signature oracles: {len(rows)} (up to date)")
        return 0
    options.output.write_text(text)
    print(f"generated {len(rows)} oracle(s) -> {options.output}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
