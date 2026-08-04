#!/usr/bin/env python3
"""Context emitter for Mizuchi's getContextScript.

Prints the scaffolding-only translation unit for one catalogued function:
everything tools/emit_translation_unit.py generates EXCEPT the subject's own
definition. Mizuchi concatenates this context with the code the agent
submits, so the agent defines the subject function against these class
shells, callee declarations and fixed-address globals.

The subject may be given as a hex address or a mangled name, because
Mizuchi's {{functionName}} template variable carries the mangled name.

Output is derived from proprietary bytes: local analysis only, never
committed.
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import pefile  # noqa: E402

from disasm import DEFAULT_EXE, resolve  # noqa: E402
from emit_translation_unit import (  # noqa: E402
    Unsettled,
    emit,
    load_callees,
    load_derived,
    load_functions,
)
from mizuchi_declfix import fix_declarations  # noqa: E402


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("subject", help="hex address or mangled name")
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    arguments = parser.parse_args()

    functions = load_functions()
    try:
        address, _ = resolve(arguments.subject, functions)
    except ValueError as error:
        print(f"REFUSED: {error}", file=sys.stderr)
        return 2

    pe = pefile.PE(str(arguments.exe), fast_load=True)
    callees = load_callees()
    try:
        text = emit(address, functions, load_derived(), callees, pe,
                    scaffolding_only=True)
    except Unsettled as error:
        print(f"REFUSED: {error}", file=sys.stderr)
        return 2

    callee_rows = [functions[target] for target in callees.get(address, [])
                   if target in functions]
    text = fix_declarations(text, callee_rows)

    print(text)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
