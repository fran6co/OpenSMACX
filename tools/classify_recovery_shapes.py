#!/usr/bin/env python3
"""Classify cached decompilations by recoverable shape.

The bottleneck in this effort is per-function manual work, so the question
worth answering is how much of the remaining backlog is mechanically shaped -
functions whose whole behaviour is a handful of field stores, a guarded
delegation, or a save/restore around one call.

This tool answers that by pattern-matching the *structure* of a cached
decompilation, never by interpreting it. That distinction is load-bearing.
Ghidra's output has been wrong about the things that matter most - it drops
the receiver on __thiscall calls and tail jumps, and its stack-local offsets
ignore mid-frame pushes - so nothing here treats the decompiler as an
authority on behaviour. A shape match says only "this function is small and
regular enough that a generated implementation is worth attempting", and the
differential oracle remains the sole judge of whether that implementation is
correct.

Shapes, in rough order of how safely they generate:

  constant-return   a single `return <literal>;`
  field-store       only stores of parameters or literals into `this`
  guarded-store     one guard, then field stores
  delegate          a single call, optionally guarded, result returned
  save-restore      globals saved, overwritten, one call, globals restored
  loop-store        a counted loop of stores at a fixed stride
  complex           anything else, including every call-bearing body that
                    does not fit one of the regular forms above

Nothing is emitted for `complex`; those stay manual.
"""

from __future__ import annotations

import argparse
import csv
import json
import re
import sys
from collections import Counter
from dataclasses import dataclass, field
from pathlib import Path

# The cache stores one file per function, named by canonical address.
CACHE_NAME_RE = re.compile(r"0x([0-9A-Fa-f]{8})-")

# Any call, not just Ghidra's synthetic FUN_ names: a body that calls
# IntersectRect or _strlen is no more mechanically generatable than one
# calling FUN_005de8f0, and treating named imports as call-free inflated
# every store category on the first run of this tool.
CALL_KEYWORDS = frozenset({
    "if", "while", "for", "switch", "return", "sizeof", "do", "else",
})
CALL_RE = re.compile(r"(?:\(\*\*\(code \*\*\))|\b([A-Za-z_]\w*)\s*\(")


def count_calls(line: str) -> int:
    """Count call sites, ignoring control keywords and cast-like forms."""
    total = 0
    for match in CALL_RE.finditer(line):
        name = match.group(1)
        if name is None:          # the indirect-call form
            total += 1
            continue
        if name in CALL_KEYWORDS:
            continue
        total += 1
    return total
LOOP_RE = re.compile(r"\b(?:do|while|for)\b")
RETURN_LITERAL_RE = re.compile(r"^\s*return\s+(?:0x[0-9A-Fa-f]+|\d+)\s*;\s*$")
STORE_RE = re.compile(r"^\s*\*?\(?[^=;]*\)?\s*=\s*[^=].*;\s*$")
GUARD_RE = re.compile(r"^\s*if\s*\(")


LVALUE_RE = re.compile(r"^\s*([^=;]+?)\s*=\s*[^=]")
RHS_RE = re.compile(r"=\s*([^=;][^;]*);\s*$")


def rhs_of(line: str) -> str:
    """The right-hand side of an assignment, whitespace collapsed."""
    match = RHS_RE.search(line)
    return re.sub(r"\s+", "", match.group(1)) if match else ""


def lvalue_of(line: str) -> str:
    """The assignment target of a statement, or empty for a non-assignment."""
    match = LVALUE_RE.match(line)
    return re.sub(r"\s+", "", match.group(1)) if match else ""


def is_recursive(text: str, body: list[str]) -> bool:
    """True when the body calls the function it belongs to.

    Self-recursion is never a mechanical shape, and it defeats the
    paired-write test below: Win::client_to_screen writes both out-parameters
    before recursing and adjusts them again afterwards, which looks exactly
    like a save followed by a restore while being nothing of the kind.
    """
    match = re.search(r"\b(FUN_[0-9a-f]+)\s*\(", text)
    if not match:
        return False
    own = match.group(1)
    return any(re.search(rf"\b{own}\s*\(", line) for line in body)


def is_save_restore(body: list[str]) -> bool:
    """True when a call is bracketed by writes to the same lvalues.

    Counting stores alone is not enough - any body that happens to assign
    several locals around one call matches that, which wrongly claimed
    Buffer::change_color's IntersectRect prologue as a save/restore. What
    actually defines the shape is that something written before the call is
    written again after it, which is what restoring means.
    """
    index = next((i for i, line in enumerate(body) if count_calls(line)), None)
    if index is None:
        return False
    # A restore reads back a local that captured the lvalue beforehand, so
    # require that pairing rather than merely a write on both sides.
    saved_into = {}
    for line in body[:index]:
        target, source = lvalue_of(line), rhs_of(line)
        if source and re.fullmatch(r"[A-Za-z_]\w*", source):
            saved_into.setdefault(source, target)
    for line in body[index + 1:]:
        target, source = lvalue_of(line), rhs_of(line)
        if source in saved_into and saved_into[source] == target:
            return True
    return False


@dataclass
class Shape:
    address: str
    name: str
    size: int
    callers: int
    shape: str
    body_lines: int
    calls: int


def body_of(text: str) -> list[str]:
    """Return the statement lines of the function body.

    Declarations and blank lines are dropped: they describe the decompiler's
    locals, not the function's behaviour.
    """
    lines = text.splitlines()
    try:
        open_index = next(i for i, l in enumerate(lines) if l.strip() == "{")
    except StopIteration:
        return []
    body = []
    for line in lines[open_index + 1:]:
        stripped = line.strip()
        if not stripped or stripped == "}":
            continue
        # A bare declaration: a type followed by a name and nothing else.
        if re.match(r"^[A-Za-z_]\w*(?:\s*\*)?\s+[A-Za-z_]\w*(?:\s*\[[^\]]*\])?;$",
                    stripped):
            continue
        body.append(stripped)
    return body


def classify(text: str) -> tuple[str, int, int]:
    """Return (shape, body statement count, call count)."""
    body = body_of(text)
    calls = sum(count_calls(line) for line in body)
    meaningful = [l for l in body if l not in ("return;",)]

    if not meaningful:
        return "empty", 0, 0

    if len(meaningful) == 1 and RETURN_LITERAL_RE.match(meaningful[0]):
        return "constant-return", len(meaningful), calls

    if is_recursive(text, body):
        return "complex", len(meaningful), calls

    has_loop = any(LOOP_RE.search(l) for l in body)
    guards = [l for l in body if GUARD_RE.match(l)]
    stores = [l for l in body if STORE_RE.match(l) and not GUARD_RE.match(l)]

    if has_loop:
        # A counted loop whose body is only stores is still regular.
        if calls == 0 and stores:
            return "loop-store", len(meaningful), calls
        return "complex", len(meaningful), calls

    if calls == 0:
        if not guards and stores:
            return "field-store", len(meaningful), calls
        if guards and stores:
            return "guarded-store", len(meaningful), calls
        return "complex", len(meaningful), calls

    if calls == 1:
        if is_save_restore(body):
            return "save-restore", len(meaningful), calls
        if len(guards) <= 2 and len(stores) <= 3:
            return "delegate", len(meaningful), calls

    return "complex", len(meaningful), calls


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--cache-dir", type=Path,
                        default=Path("build/ghidra-decompile"))
    parser.add_argument("--functions", type=Path,
                        default=Path("docs/recovery/functions.csv"))
    parser.add_argument("--max-size", type=int, default=0,
                        help="only consider functions at or below this size")
    parser.add_argument("--shape", action="append", default=[],
                        help="restrict the listing to these shapes")
    parser.add_argument("--json", type=Path,
                        help="write the full classification here")
    parser.add_argument("--limit", type=int, default=25)
    args = parser.parse_args()

    inventory: dict[str, dict] = {}
    with args.functions.open() as handle:
        for row in csv.DictReader(handle):
            inventory[row["address"].lower()] = row

    results: list[Shape] = []
    unmatched = 0
    for path in sorted(args.cache_dir.glob("*.c")):
        match = CACHE_NAME_RE.match(path.name)
        if not match:
            continue
        address = "0x" + match.group(1).lower()
        row = inventory.get(address)
        if row is None:
            unmatched += 1
            continue
        if row["recovery_state"] != "unrecovered":
            continue
        size = int(row["size"] or 0)
        if args.max_size and size > args.max_size:
            continue
        shape, body_lines, calls = classify(path.read_text())
        results.append(Shape(
            address=address,
            name=row["name"],
            size=size,
            callers=int(row["caller_count"] or 0),
            shape=shape,
            body_lines=body_lines,
            calls=calls,
        ))

    counts = Counter(r.shape for r in results)
    total = len(results)
    mechanical = sum(v for k, v in counts.items() if k != "complex")
    print(f"classified {total} unrecovered functions from the decompile cache")
    if unmatched:
        print(f"  ({unmatched} cached files had no inventory row)")
    print()
    for shape, count in counts.most_common():
        share = 100.0 * count / total if total else 0.0
        print(f"  {shape:<16} {count:>5}  {share:5.1f}%")
    print()
    print(f"mechanically shaped: {mechanical} of {total} "
          f"({100.0 * mechanical / total if total else 0:.1f}%)")

    wanted = set(args.shape) if args.shape else None
    listing = [r for r in results
               if r.shape != "complex" and (not wanted or r.shape in wanted)]
    listing.sort(key=lambda r: (-r.callers, r.size))
    if listing:
        print()
        print(f"highest fan-in candidates:")
        for entry in listing[:args.limit]:
            print(f"  callers={entry.callers:>4} {entry.size:>5}B "
                  f"{entry.shape:<15} {entry.name[:50]}  {entry.address}")

    if args.json:
        args.json.write_text(json.dumps(
            [vars(r) for r in results], indent=2, sort_keys=True) + "\n")
        print(f"\nwrote {args.json}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
