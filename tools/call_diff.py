#!/usr/bin/env -S uv run python
"""Bodies that call something the image does not, or miss one it makes.

WHY THIS IS NOT A BYTE QUESTION. A body that reaches a helper the shipped code
writes out inline is not merely encoded differently - it executes a `call` the
original never executes, and its callee's own claim is being satisfied by a
different program. Three faction.cpp bodies were found this way in one pass:
`great_satan` called `great_beelzebub`, `climactic_battle` called `ascending`,
`del_site` called `vector_dist`, and the image open-codes all three.

The reverse is worse: a call the IMAGE makes and this tree does not is a body
that skips work.

Both are visible without any byte comparison - disassemble each side, collect
the direct call targets, and subtract. This reports them, ranked by how many
edges disagree, and names the callee where the catalogue can.

    uv run tools/call_diff.py [<addr> ...] [--all]

Given addresses, ONLY those bodies are examined, and only their files are
compiled. This mattered: the first version silently ignored an address and
scanned the whole tree, so the four agents the brief had sent here each paid a
full-tree compile and then read a 40-row report that did not mention the body
they asked about.

Only DIRECT calls, on both sides. An indirect call through a vtable or a bound
slot is a runtime fact neither side can resolve, so it is counted and excluded
rather than guessed at.
"""

from __future__ import annotations

import collections
import concurrent.futures
import os
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from decomp import read
from decomp.asm import (build_command, build_inputs, call_symbols,
                        compile_unit, original_asm, shared_spans,
                        span_refusal, subject_asm)
from decomp.calls import call_sites

REPO_ROOT = Path(__file__).resolve().parent.parent
# HONOURS `OPENSMACX_IMAGE`, because `.opensmacx/` is gitignored and a
# worktree does not have it - an agent working in one must be able to
# point every tool back at the root copy. osmx.py has always taken it;
# these did not, and an agent had to symlink the image to work around it.
IMAGE = Path(os.environ.get(
    "OPENSMACX_IMAGE",
    REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"))
COMPILE_COMMANDS = Path(os.environ.get(
    "OPENSMACX_COMPILE_COMMANDS", REPO_ROOT / "build" / "compile_commands.json"))
BORROW = REPO_ROOT / "src" / "buffer.cpp"
# EVERY SET, AND FROM THE ONE PLACE THEY LIVE. `memset` and `strcat` are
# INTRINSICS under `/Oi`, so a body the image calls them from reads as "makes
# no calls" under the default set and as a false positive here. A disagreement
# is only real when NO invocation reproduces the image's count - which means
# this has to search the SAME ten sets `measure` does.
#
# It searched six. A hand-copied subset omitted `/O2 /Ob0` without `/Oy-`
# among others, so `Win::init_class` was reported as calling MORE than the
# image while the flag set `measure` actually picks agreed with it exactly.
# An agent had to disassemble the object's relocation table to disprove it.
from osmx import FLAG_SETS


def _targets(listing) -> collections.Counter:
    out: collections.Counter = collections.Counter()
    for site in call_sites(listing):
        if site.form in ("direct", "tail") and site.target is not None:
            out[site.target] += 1
    return out


def _one(job: tuple) -> list[tuple]:
    # `named` is passed IN, not read from module scope: `_one` runs in a worker
    # process and cannot see what `__main__` built.
    path, records, command, named, by_symbol = job
    objects = []
    for flags in FLAG_SETS:
        try:
            objects.append((flags, compile_unit(path, command, flags)))
        except Exception:                       # noqa: BLE001 - try the next
            continue
    if not objects:
        return []
    out = []
    for record in records:
        try:
            image = original_asm(record, IMAGE)
        except Exception:                       # noqa: BLE001 - skip this one
            continue
        best = None
        for flags, obj in objects:
            try:
                candidate = subject_asm(obj, record, flags)
            except Exception:                   # noqa: BLE001 - next set
                continue
            gap = abs(sum(_targets(candidate).values())
                      - sum(_targets(image).values()))
            if best is None or gap < best[0]:
                best = (gap, candidate, obj)
        if best is None:
            continue
        _gap, here, obj = best
        # RELOCATED TARGETS ARE UNKNOWABLE on the object side: the linker
        # writes them. So the tree's call COUNT is comparable but not its
        # addresses, and the useful statement is about how many.
        theirs, mine = _targets(image), _targets(here)
        # SAME COUNT, DIFFERENT CALLEE, which counting alone cannot see. A body
        # calling ITSELF where the image calls a base-class method has exactly
        # one call on each side and is unboundedly recursive - `Wave::load` was
        # written `this->load(a1)` inside `Wave::load` and resolved to itself,
        # while the image calls `?load@Sound@@QAEHPBD@Z`. A byte comparison
        # cannot catch it either: a call target is a relocation on both sides
        # and is discounted, so `call rel32` matches `call rel32` whatever it
        # aims at. Compare the NAMES the catalogue gives the image's targets
        # against the symbols our object calls.
        if sum(theirs.values()) == sum(mine.values()) and sum(mine.values()):
            try:
                calling = set(call_symbols(obj, record.name, record.symbol))
            except Exception:                   # noqa: BLE001
                calling = set()
            # RESOLVE OUR SYMBOLS BACK TO ADDRESSES and compare ADDRESSES.
            # Comparing names cannot work: this tree renames what it recovers -
            # `scalar_delete_popup` IS `??_GPopup`, `..._redirect` IS the method
            # it forwards - and it declares parameters differently, so
            # `?attach@RadioButton@@QAEHPAUGraphicWin@@HHH@Z` and
            # `?attach@RadioButton@@QAEHPAXHHH@Z` are one method. Both were
            # false positives, and between them they were most of a 140-row
            # report. The catalogue already maps every symbol it knows - the
            # marker's own name and its `symbol` fact - back to an address, so
            # that is the comparison that means something.
            ours = {by_symbol[x] for x in calling if x in by_symbol}
            unknown = [x for x in calling if x not in by_symbol]
            # THE RESOLVED SUBSET, not all-or-nothing. Requiring EVERY called
            # symbol to resolve made this silent whenever one CRT call was
            # unmapped - and `stack_sort` slipped through exactly that way: six
            # calls on each side, and the image's real callees are
            # `veh_cargo`/`has_abil`/`veh_lift`/`veh_drop` where the tree called
            # `veh_top`/`stack_put`. An agent found it by reading the
            # disassembly, which is the work this check exists to save.
            #
            # The unresolved names are PRINTED with the row, because they are
            # the reason to doubt it: if one of them is the image's callee under
            # a name the catalogue does not know, the row is a false positive
            # and the reader can see that.
            if ours and theirs and not (set(theirs) & ours):
                out.append((record, sum(theirs.values()), sum(mine.values()),
                            list(theirs), sorted(calling),
                            "TARGETS" if not unknown
                            else f"TARGETS/{len(unknown)}-unresolved"))
                continue
        if sum(theirs.values()) != sum(mine.values()):
            # WHICH ONES, on the tree's side. The object's call target is a
            # relocation, so the SYMBOL is the only thing that names it - and
            # "you call `vector_dist` and the image does not" is the sentence
            # worth printing.
            try:
                calling = call_symbols(obj, record.name, record.symbol)
            except Exception:               # noqa: BLE001 - names are a bonus
                calling = []
            out.append((record, sum(theirs.values()), sum(mine.values()),
                        sorted(theirs), calling))
    return out


if __name__ == "__main__":
    records = read(REPO_ROOT / "src")
    shared = shared_spans(records)
    built = build_inputs(COMPILE_COMMANDS)
    named = {}
    by_symbol: dict = {}
    for record in records:
        named.setdefault(record.address, record.name)
        for spelling in (record.name, record.symbol):
            if spelling:
                by_symbol.setdefault(spelling, record.address)

    wanted = set()
    for argument in sys.argv[1:]:
        if argument.startswith("--"):
            continue
        try:
            wanted.add(int(argument, 16))
        except ValueError:
            raise SystemExit(f"call_diff: {argument!r} is not an address")

    grouped: dict = {}
    for record in records:
        if wanted and record.address not in wanted:
            continue
        if record.byte_exact or record.path not in built or not record.name:
            continue
        if span_refusal(record, IMAGE, shared):
            continue
        grouped.setdefault(record.path, []).append(record)

    # An address that survives to nothing is a QUESTION ANSWERED WRONG, not an
    # empty report: it is claimed byte-exact already, or its file is not a
    # build input, or its span is shared. Say which, rather than printing zero
    # rows and letting the caller read that as "no disagreement".
    missing = wanted - {r.address for group in grouped.values() for r in group}
    for address in sorted(missing):
        known = [r for r in records if r.address == address]
        if not known:
            reason = "not in the catalogue"
        elif known[0].byte_exact:
            reason = "already claimed BYTE_EXACT, so there is nothing to diff"
        elif known[0].path not in built:
            reason = f"{known[0].path.name} is not a build input"
        elif not known[0].name:
            reason = "has no `symbol` fact, so its callee cannot be resolved"
        else:
            reason = "span is shared with another body (COMDAT or jump table)"
        print(f"  0x{address:08X}  SKIPPED: {reason}")

    work = []
    for path, mine in sorted(grouped.items()):
        try:
            command = build_command(COMPILE_COMMANDS, path)
        except ValueError:
            command = build_command(COMPILE_COMMANDS, BORROW)
        work.append((path, mine, command, named, by_symbol))

    jobs = max(1, min(os.cpu_count() or 1, 8))
    print(f"{sum(len(v) for v in grouped.values()):,} unclaimed bodies in "
          f"{len(grouped):,} files, {jobs} at a time", flush=True)
    pool = concurrent.futures.ProcessPoolExecutor(max_workers=jobs)
    with pool:
        batches = list(pool.map(_one, work))

    rows = [row for batch in batches for row in batch]
    # Wrong-callee rows first: a body calling the wrong thing is a semantic
    # defect, and a count disagreement is only evidence of one.
    rows.sort(key=lambda row: (len(row) < 6, -abs(row[1] - row[2]),
                               row[0].address))
    shown = rows if ("--all" in sys.argv or wanted) else rows[:40]
    for row in shown:
        record, theirs, mine, targets, calling = row[:5]
        kind = row[5] if len(row) > 5 else None
        if kind and kind.startswith("TARGETS"):
            names = ", ".join(named.get(t, f"0x{t:08X}") or f"0x{t:08X}"
                              for t in targets[:4])
            doubt = ("" if kind == "TARGETS"
                     else f"  ({kind.split('/')[1]}, see below)")
            print(f"  {record.address_hex}  {mine} call(s) on each side, but "
                  f"NONE of the image's callees - WRONG CALLEE{doubt}")
            print(f"      {record.path.name:22s} {record.name}")
            print(f"      image calls: {names}")
            print(f"      this tree calls: {', '.join(calling[:4])}")
            continue
        where = "MORE" if mine > theirs else "FEWER"
        names = ", ".join(named.get(t, f"0x{t:08X}") or f"0x{t:08X}"
                          for t in targets[:4])
        print(f"  {record.address_hex}  this tree makes {mine} call(s), the "
              f"image {theirs} - {where}")
        print(f"      {record.path.name:22s} {record.name}")
        if names:
            print(f"      image calls: {names}")
        if calling and mine > theirs:
            extra = ", ".join(sorted(set(calling))[:5])
            print(f"      this tree calls: {extra}")
    print(f"{len(rows):,} body(s) disagree with the image about how many "
          f"calls they make")
