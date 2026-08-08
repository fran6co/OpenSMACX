#!/usr/bin/env python3
"""Move every worked unit into the tree, stamped with the tier it measured.

`harvest_proven_units.py` rescues a recovery from
`build/byte-match/<address>/unit.cpp` into `src/recovered/` - but only when it
is BYTE_EXACT *and* its body can be separated from the scaffolding it was
proved against. Both conditions are right for that store: `src/recovered/`
means "proved", and `byte_match_fanout.py --collect` re-scaffolds and
re-verifies everything in it.

Both conditions also leave real work in a gitignored directory. Measured
today: 120 units are BYTE_EXACT but refuse extraction, because the agent tuned
the emitted scaffolding - the `VCall` vtable shim invites exactly that - so
body and scaffolding are one artefact. 77 more are near misses: a body was
written and the harness measured MISMATCH at one instruction, or
MNEMONIC_ONLY, or a NO_COMPILE that names precisely what is absent. None of
that is a proof. All of it is the best starting point that exists for those
addresses, and all of it dies with the next `git clean`.

COVERAGE FIRST. Getting a body into the tree for every function comes before
getting every body byte-exact; the passes that make them exact come after, and
they need something to work from. So each unit is kept WHOLE, one file per
address, under

    src/recovered/units/<address>.cpp

with its measured tier stamped at the top. Whole rather than split, because
for the 120 the tuned scaffolding IS the recovery, and because a whole unit
stays reproducible - copy it back and score it.

NOTHING HERE MAY EVER RAISE THE RATCHET. A file in this directory is not a
claim, it is a record of one. `CMakeLists.txt` names every compiled file by
hand, and the two globs that scan the proved store - `byte_match_fanout.collect`
and `tools/test_collect_ownership.py` - are non-recursive, so a subdirectory
is invisible to both. The tier written into the header comment is the tier the
harness measured, never the tier an agent reported.

    tools/preserve_near_misses.py            # report
    tools/preserve_near_misses.py --apply    # keep them
"""

from __future__ import annotations

import argparse
import csv
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import emit_translation_unit as emit  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
WORK_ROOT = REPO_ROOT / "build" / "byte-match"
STORE = REPO_ROOT / "src" / "recovered" / "units"
PROVED = REPO_ROOT / "src" / "recovered"
LEDGER = REPO_ROOT / "docs" / "recovery" / "byte-match.csv"

PLACEHOLDER = "// BODY GOES HERE."

# A body has to be big enough to BE a body. Measured on a haiku trial wave:
# 57% of its units held three code lines or fewer for functions averaging 786
# bytes - empty stubs that compile, score MISMATCH at instruction 0, and would
# have been preserved here and counted as covered. Sonnet's median body over
# the same size band is 241-340 code lines.
#
# A stub is worse than an untouched placeholder, and not by a little. A
# placeholder is honestly uncovered and stays in the queue. A stub LOOKS
# covered, leaves the queue, and the next pass opens it expecting a starting
# point and finds `return;`. The only reason coverage counts a MISMATCH at all
# is that a real attempt is worth something to whoever comes next.
#
# Deliberately crude, because the failure is crude: only bodies that cannot
# possibly implement their function are refused, and anything near the
# boundary is kept.
MIN_BODY_LINES = 6
STUB_ABOVE_BYTES = 200


def body_lines(text: str) -> int:
    """Code lines after the emitted scaffolding, comments and blanks aside."""
    tail = text.rsplit(PLACEHOLDER, 1)[-1] if PLACEHOLDER in text else text
    return len([line for line in tail.splitlines()
                if line.strip() and not line.lstrip().startswith("//")])


def ledger_rows() -> dict:
    if not LEDGER.is_file():
        return {}
    with LEDGER.open(newline="", encoding="utf-8-sig") as handle:
        return {row["address"].upper(): row for row in csv.DictReader(handle)}


def candidates(functions: dict, rows: dict) -> tuple:
    """Worked units with no committed copy anywhere, and the stubs refused."""
    proved = {int(path.stem, 16) for path in PROVED.glob("*.cpp")}
    found, stubs = [], []
    for unit in sorted(WORK_ROOT.glob("*/unit.cpp")):
        try:
            address = int(unit.parent.name, 16)
        except ValueError:
            continue
        text = unit.read_text()
        if PLACEHOLDER in text:
            continue                       # nobody has worked on it
        if address in proved:
            continue                       # the proved store already has it
        # A function `src/` owns has an authoritative body there already; a
        # copy beside it is a second answer for one address, which is the
        # confusion `harvest_proven_units` documents at length.
        if (functions.get(address, {}).get("source_locations") or "").strip():
            continue
        size = int(functions.get(address, {}).get("size") or 0)
        if size >= STUB_ABOVE_BYTES and body_lines(text) <= MIN_BODY_LINES:
            stubs.append((address, size, body_lines(text)))
            continue
        row = rows.get(f"0X{address:08X}") or {}
        found.append((address, unit, row, text))
    return found, stubs


def preamble(address: int, row: dict, function: dict) -> str:
    """What the next attempt needs, above the unit it starts from."""
    tier = row.get("tier") or "UNMEASURED"
    lines = [
        f"// PRESERVED UNIT - measured {tier}.",
        "//",
        "// Kept for COVERAGE, not as a claim. Nothing reads this directory:",
        "// it is on no ratchet, in no build, and scored by no collect.",
        "//",
        f"// address        0x{address:08X}",
        f"// name           {function.get('name') or row.get('name') or ''}",
        f"// size           {function.get('size') or row.get('size') or ''}"
        " bytes",
        f"// measured tier  {tier}",
    ]
    divergence = (row.get("first_divergence") or "").strip()
    if divergence:
        lines.append(f"// divergence     {divergence[:160]}")
    reason = (row.get("refusal_reason") or "").strip().replace("\n", " ")
    if reason:
        lines.append(f"// refusal        {reason[:160]}")
    lines += [
        "//",
        "// The WHOLE unit as measured, scaffolding included: for the units",
        "// that are byte-exact yet refuse extraction, the agent tuned the",
        "// emitted scaffolding and the body alone will not reproduce the",
        "// verdict. To resume, copy everything below back over",
        f"//   build/byte-match/{address:08x}/unit.cpp",
        "// and score it with tools/agent_brief.py.",
        "",
    ]
    return "\n".join(lines)


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--apply", action="store_true",
                        help="write the units; without it, only report")
    arguments = parser.parse_args(argv)

    functions = emit.load_functions()
    rows = ledger_rows()
    found, stubs = candidates(functions, rows)
    print(f"{len(found)} worked units have no committed copy")
    if stubs:
        # Never silent: a refused stub is an address that still needs work,
        # and saying so is the difference between a queue and a guess.
        print(f"REFUSED {len(stubs)} stub(s) - too small to be a body, and "
              f"they stay uncovered:")
        for address, size, lines in sorted(stubs)[:8]:
            print(f"  0x{address:08X}  {size:>6} B function, "
                  f"{lines} code line(s)")
        if len(stubs) > 8:
            print(f"  ... and {len(stubs) - 8} more")
    if not found:
        return 0

    tiers: dict = {}
    for _address, _unit, row, _text in found:
        tier = row.get("tier") or "UNMEASURED"
        tiers[tier] = tiers.get(tier, 0) + 1
    print("  " + ", ".join(f"{tier} {n}" for tier, n in sorted(tiers.items())))

    if not arguments.apply:
        return 0

    STORE.mkdir(parents=True, exist_ok=True)
    written = 0
    for address, _unit, row, text in found:
        destination = STORE / f"{address:08x}.cpp"
        kept = preamble(address, row, functions.get(address, {})) + text
        if destination.is_file() and destination.read_text() == kept:
            continue
        destination.write_text(kept)
        written += 1
    print(f"kept {written} unit(s) under {STORE.relative_to(REPO_ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
