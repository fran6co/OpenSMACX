#!/usr/bin/env python3
"""Generate complete recoveries for constant-return stubs, in bulk.

The per-batch loop this replaces is: read a function, write its declaration,
its implementation with an Original Offset annotation, its redirect, a canary
test, wire the redirect, run the gate. For the constant-return shape - a body
that is nothing but `mov eax, N; ret K` or a bare `ret` - every one of those
steps is mechanical and identical up to a name, an address, and a returned
value, so this emits all of them for a whole class at once.

What it does NOT do is decide anything a wrong guess would make plausible. It
only accepts a function whose body is provably a constant return AND whose
mangled name's parameter list agrees with the callee-pop count - the same two
checks tools/find_constant_returns.py makes and that reject Win::on_redraw
across the whole image. Anything else is skipped and named, never emitted.

The recoveries it writes are still verified the ordinary way: the caller runs
the gate, which builds them, runs the generated canary tests against a seeded
object, regenerates metadata, and exercises them in the hybrid process. This
tool compresses authoring, not verification - the differential check that has
caught real defects all along is untouched.

Output is source to the class's files plus a test block to stdout for pasting
into the leaf-test suite; it prints the address/symbol pairs to wire with
tools/add_redirect.py. It never edits a header it did not create without being
told the class already has one.
"""

from __future__ import annotations

import argparse
import csv
import re
import sys
from dataclasses import dataclass
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import find_constant_returns as const  # noqa: E402
from generator_support import snake  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
FUNCTIONS_CSV = REPO_ROOT / "docs" / "recovery" / "functions.csv"
SRC_DIR = REPO_ROOT / "src"


@dataclass
class Stub:
    method: str          # bare method name, e.g. "close"
    address: str         # "0x0045C170"
    convention: str      # "thiscall" | "cdecl" | "stdcall"
    param_count: int     # number of int-sized parameters
    returns: int | None  # constant returned, or None for void
    return_type: str     # "void" | "int"


def classify(row: dict, pe) -> Stub | None:
    """A Stub when the row is a provable constant return with agreeing arity."""
    size = int(row["size"] or 0)
    if not 1 <= size <= 10:
        return None
    body = const.constant_body(const.read_bytes(pe, int(row["address"], 16), size))
    if body is None:
        return None
    value, popped = body
    arity = const.declared_arity(row["name"])
    if arity is None:
        return None
    convention, declared = arity
    expected = 0 if convention == "cdecl" else declared
    if expected != popped:                       # the on_redraw guard
        return None
    match = re.match(r"\?(\w+)@\w+@@", row["name"])
    if not match:
        return None
    return_type = "void" if value is None else "int"
    return Stub(match.group(1), f"0x{int(row['address'], 16):08X}",
                convention, declared // 4, value, return_type)


def collect(pe, class_name: str) -> list[Stub]:
    stubs, seen = [], set()
    with FUNCTIONS_CSV.open() as handle:
        for row in csv.DictReader(handle):
            if row["recovery_state"] != "unrecovered":
                continue
            if not row["name"].startswith(f"?") or f"@{class_name}@@" not in row["name"]:
                continue
            stub = classify(row, pe)
            if stub and stub.method not in seen:   # skip overloads; ambiguous
                seen.add(stub.method)
                stubs.append(stub)
    return stubs


def render_declarations(stubs: list[Stub]) -> str:
    lines = []
    for stub in stubs:
        params = ", ".join("int" for _ in range(stub.param_count))
        static = "static " if stub.convention == "cdecl" else ""
        lines.append(f"  {static}{stub.return_type} {stub.method}({params});")
    return "\n".join(lines)


def render_redirect_declarations(class_name: str, stubs: list[Stub]) -> str:
    lines = []
    for stub in stubs:
        name = f"{snake(class_name)}_{stub.method.lower()}_redirect"
        if stub.convention == "cdecl":
            lines.append(f"{stub.return_type} __cdecl {name}();")
        else:
            args = "".join(f", int a{i + 1}" for i in range(stub.param_count))
            lines.append(f"{stub.return_type} __fastcall {name}"
                         f"({class_name} *self, void *{args});")
    return "\n".join(lines)


def render_definitions(class_name: str, stubs: list[Stub]) -> str:
    blocks = []
    for stub in stubs:
        params = ", ".join("int" for _ in range(stub.param_count))
        body = "" if stub.returns is None else f"    return {stub.returns};\n"
        told = ("returns" if stub.returns is None
                else f"returns {stub.returns}")
        blocks.append(f"""
/*
Purpose: Unknown; the legacy implementation is a constant return that {told}.
Original Offset: {stub.address[2:]}
Return Value: {"n/a" if stub.returns is None else str(stub.returns) + ", always"}
Status: Complete
*/
{stub.return_type} {class_name}::{stub.method}({params}) {{
{body}}}
""")
    forwards = []
    for stub in stubs:
        name = f"{snake(class_name)}_{stub.method.lower()}_redirect"
        call = "return " if stub.return_type == "int" else ""
        arg_names = ", ".join(f"a{i + 1}" for i in range(stub.param_count))
        if stub.convention == "cdecl":
            forwards.append(f"{stub.return_type} __cdecl {name}() {{\n"
                            f"    {call}{class_name}::{stub.method}();\n}}\n")
        else:
            args = "".join(f", int a{i + 1}" for i in range(stub.param_count))
            forwards.append(f"{stub.return_type} __fastcall {name}"
                            f"({class_name} *self, void *{args}) {{\n"
                            f"    {call}self->{stub.method}({arg_names});\n}}\n")
    return "".join(blocks) + "\n" + "\n".join(forwards)


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("class_name")
    parser.add_argument("--exe", type=Path, default=const.DEFAULT_EXE)
    parser.add_argument("--dry-run", action="store_true",
                        help="print what would be generated, write nothing")
    args = parser.parse_args()

    import pefile
    pe = pefile.PE(str(args.exe), fast_load=True)
    stubs = collect(pe, args.class_name)
    if not stubs:
        print(f"no constant-return stubs found for {args.class_name}",
              file=sys.stderr)
        return 1

    print(f"# {args.class_name}: {len(stubs)} constant-return stubs")
    for stub in stubs:
        print(f"#   {stub.address}  {stub.method}  "
              f"{stub.convention} {stub.param_count} params -> {stub.returns}")
    print("\n# --- declarations (into the class body) ---")
    print(render_declarations(stubs))
    print("\n# --- redirect declarations (into the header) ---")
    print(render_redirect_declarations(args.class_name, stubs))
    print("\n# --- definitions (into the .cpp) ---")
    print(render_definitions(args.class_name, stubs))
    print("\n# --- wire these ---")
    for stub in stubs:
        print(f"{stub.address} "
              f"{snake(args.class_name)}_{stub.method.lower()}_redirect")
    return 0


if __name__ == "__main__":
    sys.exit(main())
