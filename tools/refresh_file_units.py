#!/usr/bin/env python3
"""Re-scaffold a FILE-mode unit against the CURRENT emitter, keeping its body.

A FILE-mode landing is frozen at the moment it was written: the whole file is
the translation unit, so the scaffolding half is a snapshot of whatever the
emitter produced that day. The body ages well - it is a recovery - and the
scaffolding does not. Measured 2026-08-14, of 715 NO_COMPILE rows:

    137  C2079  'spot_' uses undefined class 'Spot'   - `class Buffer` holds a
                                                        `Spot` by value and
                                                        `Spot` is defined
                                                        AFTER it, an ordering
                                                        the emitter stopped
                                                        making in August
      7  C2065  'int8' : undeclared identifier        - the prelude gained
     26  C2061  syntax error : identifier 'uintptr_t'   these typedefs later

Every one of those is a defect in a file the emitter would no longer write.
The body is not at fault and re-running the agent on it would be paying twice
for the same recovery.

WHY THIS IS A RATCHET AND NOT A REWRITE. Regenerating scaffolding under a
committed body is exactly the move that can silently destroy evidence: a body
that compiled against the old shells may not against the new ones, and a
BYTE_EXACT claim rests on the unit as a whole. So the rewrite is provisional.
`--apply` writes it, the caller measures, and `--revert-regressions` restores
from git every file whose tier did not improve. A file can only move up the
tier order or go back to exactly what it was; there is no path where this tool
leaves a row worse than it found it, and none where it edits an annotation.

    tools/refresh_file_units.py --apply
    tools/decomp_status.py --addresses "$(tools/refresh_file_units.py --addresses)"
    tools/refresh_file_units.py --revert-regressions --before before.csv
"""
from __future__ import annotations

import argparse
import csv
import re
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import pefile                                                 # noqa: E402

import annotation_scan                                        # noqa: E402
import byte_match                                             # noqa: E402
import emit_translation_unit as emit                          # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
LEDGER = REPO_ROOT / ".opensmacx" / "byte-match.csv"

# Better is further down. A row that moves the other way is a regression and
# gets its file back; `UNSCOREABLE` and `REFUSED` are not on this scale at all,
# so a move into them is a regression by the same rule - they are absent.
# DERIVED FROM byte_match, NOT RESTATED. This file used to carry its own
# worst-first tuple, and it disagreed with the authority about one pair: it
# ranked MISMATCH ABOVE MNEMONIC_ONLY, where byte_match ranks MNEMONIC_ONLY
# above MISMATCH - correctly, since "every mnemonic agrees" is strictly
# stronger evidence than "they do not". A body that went MNEMONIC_ONLY ->
# MISMATCH was therefore KEPT as an improvement, which is the one outcome
# this tool promises cannot happen. It happened twice in the 2026-08-15
# re-scaffold, on 0x00476B70 and 0x00617620.
#
# A second copy of a ranking is a second answer to "which of these is better",
# and the disagreement is invisible until it decides something.
def rank(tier: str) -> int:
    """Higher is better. Unknown, unmeasured and UNSCOREABLE are all worst.

    `byte_match.UNSCOREABLE_TIERS` are walls rather than misses - SHARED_TAIL
    is a COMDAT-folded span up to thirteen functions claim, where a
    per-function verdict is undefined, and REFUSED is decided before a
    compiler is reached. Neither is a rung on this ladder, so moving onto one
    is never an improvement to keep. They sit inside byte_match's order, which
    is why deriving from it needs this line and not just an index.
    """
    order = byte_match.TIER_ORDER          # best-first
    if tier not in order or tier in byte_match.UNSCOREABLE_TIERS:
        return -1
    return len(order) - order.index(tier)


def normalise(address: str) -> str:
    """One spelling. `0x{:08X}` and `.upper()` disagree on the `x` itself, and
    a dict keyed both ways silently finds nothing - which reads as "no
    candidates" rather than as a bug."""
    return f"0x{int(address, 16):08X}"


def ledger_tiers(path: Path) -> dict:
    if not path.exists():
        return {}
    with path.open() as handle:
        return {normalise(row["address"]): row["tier"]
                for row in csv.DictReader(handle)}


def split_unit(text: str, subject: str):
    """`(header, body)` - the annotation block, and the subject's definition on.

    The subject is the LAST top-level definition in an emitted unit, so the
    search runs from the end: an earlier match is a callee declaration or a
    shell's member, and splitting there would swallow the scaffolding it still
    needs. Everything between the two is what this tool replaces.
    """
    lines = text.splitlines()
    header_end = 0
    for index, line in enumerate(lines):
        if line.startswith("//"):
            header_end = index + 1
        elif line.strip():
            break
    wanted = re.compile(rf"^\S.*\b{re.escape(subject)}\b\s*\(")
    for index in range(len(lines) - 1, header_end - 1, -1):
        if not wanted.match(lines[index]):
            continue
        tail = "\n".join(lines[index:]).lstrip()
        if not tail.rstrip().endswith("}"):
            continue          # a declaration, not the definition
        return "\n".join(lines[:header_end]), "\n".join(lines[index:])
    return None, None


def candidates(only_no_compile: bool = True, tiers: dict = None):
    """Every FILE-mode unit whose scaffolding this tool could replace.

    `tiers` DEFAULTS TO THE LIVE LEDGER AND MUST NOT, once the units have been
    rewritten and re-measured: a unit that improved is no longer NO_COMPILE,
    so re-deriving the set from the live ledger drops exactly the units the
    run succeeded on. The revert pass did that and reported "0 improved, 72
    restored" for a run that improved 44 - it reverted the right files and
    then described the opposite outcome, which is the more dangerous half.
    """
    if tiers is None:
        tiers = ledger_tiers(LEDGER)
    out = []
    for path in annotation_scan.sources(REPO_ROOT / "src"):
        for note in annotation_scan.scan_file(path):
            if note.recipe != "verbatim":
                continue
            # A PLACEHOLDER HAS NO BODY TO KEEP, so re-scaffolding it is not
            # this tool's job - `decomp_status --work` already materialises a
            # fresh scaffold over one, and doing it here would present 112
            # untouched stubs as refusals.
            if note.state != annotation_scan.STATE_IMPLEMENTED:
                continue
            address = normalise(hex(note.address))
            if only_no_compile and tiers.get(address) != "NO_COMPILE":
                continue
            lines = note.region.splitlines()
            subject = annotation_scan.subject_identifier(lines, 0)
            if subject is None:
                continue
            out.append((address, path, subject))
    return out


def refreshed(path: Path, address: str, subject: str, context):
    functions, derived, callees, pe = context
    text = path.read_text(errors="ignore")
    header, body = split_unit(text, subject)
    if body is None:
        return None, f"{path}: cannot find the definition of {subject}"
    try:
        scaffolding = emit.emit(int(address, 16), functions, derived, callees,
                                pe, scaffolding_only=True, body=body)
    except emit.Unsettled as error:
        return None, f"{path}: no scaffolding: {error}"
    # The emitter's own banner would be the second one in the file; the
    # annotation block above is the one the map reads, and it stays first.
    scaffolding = "\n".join(
        line for line in scaffolding.splitlines()
        if not line.startswith("// ORIGINAL:"))
    return f"{header}\n{scaffolding.rstrip()}\n\n{body.rstrip()}\n", None


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--apply", action="store_true",
                        help="rewrite the scaffolding of every candidate")
    parser.add_argument("--addresses", action="store_true",
                        help="print the candidate addresses, comma separated")
    parser.add_argument("--revert-regressions", action="store_true")
    parser.add_argument("--before", type=Path,
                        help="ledger snapshot taken before --apply")
    parser.add_argument("--all-tiers", action="store_true",
                        help="consider units that already compile")
    args = parser.parse_args()

    found = candidates(only_no_compile=not args.all_tiers)

    if args.addresses:
        print(",".join(address for address, _, _ in found), end="")
        return 0

    if args.revert_regressions:
        if args.before is None:
            parser.error("--revert-regressions needs --before")
        was, now = ledger_tiers(args.before), ledger_tiers(LEDGER)
        # The set as it was BEFORE the rewrite, for the reason in
        # `candidates`.
        found = candidates(only_no_compile=not args.all_tiers, tiers=was)
        reverted, kept = [], []
        for address, path, _ in found:
            before, after = was.get(address, ""), now.get(address, "")
            if rank(after) > rank(before):
                kept.append((address, before, after))
            else:
                reverted.append((address, before, after))
                subprocess.run(["git", "checkout", "--",
                                str(path.relative_to(REPO_ROOT))],
                               cwd=REPO_ROOT, check=True)
        for address, before, after in kept:
            print(f"  kept     {address}  {before} -> {after}")
        for address, before, after in reverted:
            print(f"  reverted {address}  {before} -> {after or 'unmeasured'}")
        print(f"{len(kept)} improved, {len(reverted)} restored")
        return 0

    context = (emit.load_functions(), emit.load_derived(),
               emit.load_callees(),
               pefile.PE(str(emit.DEFAULT_EXE), fast_load=True))
    written, refused = 0, []
    for address, path, subject in found:
        text, why = refreshed(path, address, subject, context)
        if text is None:
            refused.append(why)
            continue
        if args.apply:
            path.write_text(text)
        written += 1
    print(f"{written} unit(s) {'rewritten' if args.apply else 'would change'}, "
          f"{len(refused)} refused")
    for why in refused[:20]:
        print(f"  {why}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
