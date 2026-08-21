#!/usr/bin/env -S uv run python
"""Write the `symbol` fact for claims whose name the object does not define.

WHY. `measure` refuses a record whose catalogued mangled name is not among the
object's `.text` symbols, and says so precisely: the fix is a `symbol` fact
recording what THIS TREE emits for the piece. It refuses rather than guesses on
purpose - a lookup that guesses returns a neighbouring function's assembly and
reads as a mismatch nobody can explain.

So the guessing belongs here, in a migration that writes the fact once and can
be argued with, and not in what measures. Two things make a claim unverifiable
this way:

  * the catalogue carries an IDA placeholder - `sub_402ee0` - where the source
    defines a real C++ name;
  * the catalogue spells a type out where MSVC back-references it:
    `?swap@@YAXPAHPAH@Z` against the emitted `?swap@@YAXPAH0@Z`.

WHAT IT REFUSES. Anything it cannot pin to ONE symbol. A file defining several
functions is only answered when exactly one candidate matches the record's
identifier and argument count, and a file whose single symbol serves several
records is refused outright - two claims cannot share one body.
"""

from __future__ import annotations

import concurrent.futures
import os
import re
import sys
from dataclasses import replace
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from decomp import read
from decomp.asm import build_command, compile_unit, text_symbols
from decomp.mangled import arity, qualified_name

REPO_ROOT = Path(__file__).resolve().parent.parent
COMPILE_COMMANDS = REPO_ROOT / "build" / "compile_commands.json"
BORROW = "src/buffer.cpp"
FLAGS = "/c /O2 /Gy /GR- /GX"


def _symbols_of(path: Path) -> list[str] | str:
    """The object's `.text` symbols, or the reason there are none."""
    try:
        try:
            command = build_command(COMPILE_COMMANDS, path)
        except ValueError:
            command = build_command(COMPILE_COMMANDS, REPO_ROOT / BORROW)
        return [name for name, _v, _s
                in text_symbols(compile_unit(path, command, FLAGS))]
    except Exception as problem:                # noqa: BLE001 - reported
        return str(problem).splitlines()[0]


def candidates(record, symbols: list[str]) -> list[str]:
    """The object symbols that could be this record, best evidence first."""
    if record.name in symbols:
        return [record.name]
    wanted, count = qualified_name(record.name), arity(record.name)
    same = [s for s in symbols
            if qualified_name(s) == wanted and wanted]
    if len(same) == 1:
        return same
    if count is not None:
        narrowed = [s for s in same if arity(s) == count]
        if len(narrowed) == 1:
            return narrowed
    return same


def _insert(facts: list[tuple]) -> None:
    """Write each `symbol` fact under its marker.

    NOT THROUGH `write_file`. The writer rewrites the marker line and the
    lesson run and passes the rest of the fact block through untouched, so a
    `symbol` the model carries has no way back to disk through it. Adding
    emission there would mean the writer deciding where a fact belongs in a
    block it deliberately does not parse; this is a migration, and putting the
    line under the marker is its job.
    """
    by_path: dict[Path, list[tuple]] = {}
    for record, symbol in facts:
        by_path.setdefault(Path(record.path), []).append((record, symbol))
    for path, group in by_path.items():
        lines = path.read_text().splitlines(keepends=True)
        for record, symbol in sorted(group, key=lambda g: -g[0].line):
            index = record.line - 1
            marker = lines[index]
            prefix = "// " if marker.lstrip().startswith("//") else " * "
            indent = marker[:len(marker) - len(marker.lstrip())]
            # An existing (stale) fact is REPLACED, never doubled.
            for offset in range(index + 1, min(index + 20, len(lines))):
                if re.match(r"\s*(?://|\*)\s*symbol\s", lines[offset]):
                    lines[offset] = f"{indent}{prefix}symbol    {symbol}\n"
                    break
                if not re.match(r"\s*(?://|\*)\s*\w+\s", lines[offset]):
                    lines.insert(index + 1,
                                 f"{indent}{prefix}symbol    {symbol}\n")
                    break
            else:
                lines.insert(index + 1,
                             f"{indent}{prefix}symbol    {symbol}\n")
        path.write_text("".join(lines))


def main(apply: bool) -> int:
    records = read(REPO_ROOT / "src")
    stuck: dict[Path, list] = {}
    for record in records:
        if record.symbol or not record.name or not record.byte_exact:
            continue
        stuck.setdefault(record.path, []).append(record)

    # ONE COMPILE PER FILE, IN PARALLEL. The stuck claims live one per
    # artifact file, so this is a thousand wine invocations; serially it does
    # not finish inside any timeout worth setting.
    jobs = max(1, min(os.cpu_count() or 1, 8))
    print(f"{sum(len(v) for v in stuck.values()):,} claim(s) without a "
          f"`symbol` fact in {len(stuck):,} files, {jobs} at a time",
          flush=True)
    pool = concurrent.futures.ProcessPoolExecutor(max_workers=jobs)
    with pool:
        compiled = dict(zip(sorted(stuck),
                            pool.map(_symbols_of, sorted(stuck))))

    updated, written = [], 0
    for path, mine in sorted(stuck.items()):
        symbols = compiled[path]
        if isinstance(symbols, str):
            print(f"  ? {path.name}: {symbols[:90]}")
            continue
        # THE SAME RULE THE LOOKUP USES, or this proposes a fact for records
        # that already measure: `_coff_function_masked` also accepts the
        # `extern "C"` form, so `sub_401be0` finding `_sub_401be0` is not a
        # failure and writing it down would be 125 lines of noise.
        missing = [r for r in mine
                   if r.name not in symbols and "_" + r.name not in symbols]
        if not missing:
            continue
        # A SINGLE-SYMBOL FILE answers a single claim, and no more: if two
        # records point at one body, the catalogue is wrong about one of them
        # and naming both would hide that.
        for record in missing:
            found = candidates(record, symbols)
            if len(symbols) == 1 and len(missing) == 1:
                found = symbols
            if len(found) != 1:
                print(f"  - {record.address_hex} {record.name} "
                      f"({path.name}): {len(found)} candidate(s) among "
                      f"{len(symbols)} symbol(s)")
                continue
            print(f"  + {record.address_hex} {record.name} -> {found[0]}")
            updated.append((record, found[0]))
            written += 1
    if apply and updated:
        _insert(updated)
    return written


if __name__ == "__main__":
    count = main("--apply" in sys.argv)
    print(f"{count} `symbol` fact(s) "
          f"{'written' if '--apply' in sys.argv else 'writable'}")
