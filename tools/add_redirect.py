#!/usr/bin/env python3
"""Wire a recovered function's runtime redirect in one verified step.

Installing a redirect touches four places that must agree: a row in
docs/recovery-redirects.csv, a regenerated signature macro in
src/redirect_signatures.h, a spec entry in the dllmain.cpp table, and the
RedirectCount that a static_assert checks the table against. Done by hand this
is pure clerical work, and it is the step this repository's history shows
going wrong most often - a spec inserted at the wrong sorted position, or a
count left stale, both of which surface much later as a confusing gate
failure rather than an obvious mistake.

Every edit here is checked rather than assumed: the address must not already
be wired, the replacement symbol must exist in the sources, the signature
macro must appear after regeneration, and the resulting table length must
match the new count. Any failure restores every file this touched.
"""

from __future__ import annotations

import argparse
import re
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
REDIRECT_CSV = REPO_ROOT / "docs" / "recovery-redirects.csv"
DLLMAIN = REPO_ROOT / "src" / "dllmain.cpp"
SIGNATURES = REPO_ROOT / "src" / "redirect_signatures.h"
GENERATOR = REPO_ROOT / "tools" / "generate_redirect_signatures.py"
SRC_DIR = REPO_ROOT / "src"

COUNT_RE = re.compile(r"constexpr size_t RedirectCount = (\d+);")
SPEC_RE = re.compile(r"^        \{\n            0x([0-9A-Fa-f]{8}),\n", re.M)
CALL_TABLE = "const RedirectSpec call_specs[]"
JUMP_TABLE_END = "    };\n    static_assert(sizeof(specs)"


@dataclass
class Edit:
    path: Path
    original: str


def parse_address(text: str) -> int:
    value = int(text, 16)
    if not 0x00400000 <= value < 0x00800000:
        raise ValueError(f"{text} is outside the image's code range")
    return value


def symbol_defined(symbol: str) -> bool:
    """True when a source file defines this replacement.

    A redirect naming a symbol that does not exist still compiles the table
    only if some header declares it, and then fails at link with a message
    that points at dllmain rather than at the omission.
    """
    pattern = re.compile(rf"\b{re.escape(symbol)}\s*\(")
    for path in sorted(SRC_DIR.glob("*.cpp")):
        text = path.read_text()
        for match in pattern.finditer(text):
            # A definition is followed by a body, not a semicolon.
            tail = text[match.end():]
            depth = 1
            for index, char in enumerate(tail):
                if char == "(":
                    depth += 1
                elif char == ")":
                    depth -= 1
                    if depth == 0:
                        rest = tail[index + 1:].lstrip()
                        if rest.startswith("{"):
                            return True
                        break
    return False


def declaring_header(symbol: str) -> str | None:
    """The header that declares this replacement, if any."""
    pattern = re.compile(rf"\b{re.escape(symbol)}\s*\(")
    for path in sorted(SRC_DIR.glob("*.h")):
        if pattern.search(path.read_text()):
            return path.name
    return None


def visible_to_dllmain(symbol: str) -> tuple[bool, str]:
    """Whether the spec table can actually name this replacement.

    The four files agreeing is not enough: a symbol declared in a header
    dllmain.cpp does not include compiles into a spec table that cannot see
    it. Wiring eighteen redirects to a new header once produced eighteen
    identical 'not declared in this scope' errors well after the tool had
    reported success on every one.
    """
    header = declaring_header(symbol)
    if header is None:
        return False, f"no header in src/ declares {symbol}"
    if f'#include "{header}"' not in DLLMAIN.read_text():
        return False, (f"{symbol} is declared in {header}, which "
                       f"{DLLMAIN.name} does not include")
    return True, header


def add_csv_row(address: int, kind: str) -> Edit:
    original = REDIRECT_CSV.read_text()
    token = f"0x{address:08X}"
    if token in original:
        raise ValueError(f"{token} is already listed in {REDIRECT_CSV.name}")
    lines = original.strip().splitlines()
    header, rows = lines[0], lines[1:]
    rows.append(f"{token},{kind}")
    rows = sorted(set(rows), key=lambda row: int(row.split(",")[0], 16))
    REDIRECT_CSV.write_text(header + "\n" + "\n".join(rows) + "\n")
    return Edit(REDIRECT_CSV, original)


def add_spec(address: int, symbol: str) -> Edit:
    original = DLLMAIN.read_text()
    token = f"0x{address:08X}"
    if f"            {token},\n" in original:
        raise ValueError(f"{token} already has a spec entry in {DLLMAIN.name}")
    match = COUNT_RE.search(original)
    if not match:
        raise ValueError("could not find RedirectCount")
    count = int(match.group(1))

    entry = ("        {\n"
             f"            {token},\n"
             f"            reinterpret_cast<uintptr_t>(&{symbol}),\n"
             f"            OPENSMACX_SIGNATURE_{address:08X},\n"
             "        },\n")
    # Keep the table's mostly-ascending order by inserting before the first
    # higher address. This is a tidiness convention rather than a correctness
    # one - the array is only ever iterated, so order does not affect
    # behaviour, and the committed table is in fact already out of order in
    # two dozen places. What does matter is landing in the jump table rather
    # than in the call table below it, which holds interior call sites and not
    # function starts.
    position = None
    for spec in SPEC_RE.finditer(original[:original.index(CALL_TABLE)]):
        if int(spec.group(1), 16) > address:
            position = spec.start()
            break
    if position is None:
        # Higher than everything present: append at the end of the jump table,
        # which is unambiguous because the array's terminator is.
        position = original.index(JUMP_TABLE_END)
    updated = original[:position] + entry + original[position:]
    updated = COUNT_RE.sub(
        f"constexpr size_t RedirectCount = {count + 1};", updated, count=1)
    DLLMAIN.write_text(updated)
    return Edit(DLLMAIN, original)


def regenerate_signatures() -> None:
    python = REPO_ROOT / ".opensmacx" / "venv" / "bin" / "python"
    interpreter = str(python) if python.is_file() else sys.executable
    result = subprocess.run([interpreter, str(GENERATOR)],
                            capture_output=True, text=True)
    if result.returncode != 0:
        raise ValueError(
            "signature regeneration failed:\n" + (result.stderr or result.stdout))


def verify(address: int, symbol: str) -> None:
    """Confirm the four files now agree, before anything is built."""
    macro = f"OPENSMACX_SIGNATURE_{address:08X}"
    if macro not in SIGNATURES.read_text():
        raise ValueError(
            f"{macro} is missing after regeneration; the address may not be a "
            "recognised function start in the pinned executable")
    visible, detail = visible_to_dllmain(symbol)
    if not visible:
        raise ValueError(detail)
    text = DLLMAIN.read_text()
    match = COUNT_RE.search(text)
    declared = int(match.group(1))
    # Count only the jump table, which ends where the call table begins.
    cutoff = text.index(CALL_TABLE)
    actual = len(SPEC_RE.findall(text[:cutoff]))
    if actual != declared:
        raise ValueError(
            f"RedirectCount is {declared} but the table holds {actual} entries")


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("address", help="canonical address, e.g. 0x005EE130")
    parser.add_argument("symbol", help="the __fastcall/__cdecl replacement")
    parser.add_argument("--kind", default="jump", choices=("jump", "call"))
    parser.add_argument("--allow-missing-symbol", action="store_true",
                        help="skip the check that a source file defines the "
                             "replacement, for a symbol added in the same edit")
    args = parser.parse_args()

    try:
        address = parse_address(args.address)
    except ValueError as error:
        print(f"error: {error}", file=sys.stderr)
        return 2

    if not args.allow_missing_symbol and not symbol_defined(args.symbol):
        print(f"error: no source file defines {args.symbol}; pass "
              "--allow-missing-symbol if it is being added in this same change",
              file=sys.stderr)
        return 2

    edits: list[Edit] = []
    try:
        edits.append(add_csv_row(address, args.kind))
        edits.append(add_spec(address, args.symbol))
        signatures_before = SIGNATURES.read_text()
        edits.append(Edit(SIGNATURES, signatures_before))
        regenerate_signatures()
        verify(address, args.symbol)
    except Exception as error:                      # noqa: BLE001
        for edit in reversed(edits):
            edit.path.write_text(edit.original)
        print(f"error: {error}", file=sys.stderr)
        print("all files restored", file=sys.stderr)
        return 1

    print(f"wired 0x{address:08X} -> {args.symbol} ({args.kind})")
    print("  docs/recovery-redirects.csv, src/dllmain.cpp, "
          "src/redirect_signatures.h updated and cross-checked")
    return 0


if __name__ == "__main__":
    sys.exit(main())
