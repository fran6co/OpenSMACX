#!/usr/bin/env python3
"""Hold every annotation's call-graph facts to the image, not to a copy of itself.

`src/` IS the catalogue. A recovery agent rebuilds its file's header when it
assembles a body, so the `// calls`, `// callers` and `// call targets` fields
are not comments about the evidence - they ARE the evidence, and rewriting
them edits what the body is scored against. Nothing noticed. Measured
2026-08-14 against the commit where `src/` became the store:

    0x0041E790  ?on_redraw@BaseWin@@QAEXXZ    16 call targets -> 6
    0x00636740  Video_unk2                     7 call targets -> (none)
    0x0043D120  ?on_status@DesignWin@@QAEXH@Z  7 -> 6
    0x005304A0  ?open_channel@NetDaemon@@       0x00534400 replaced by 0x00470F80
    0x005FB5A0  ?UNK10@Menu@@QAEHHPAD@Z         0x00645470 replaced by 0x006454E0
    0x00442590  ?on_key_click@DiploWin@@        gained 0x0066931C, an IAT slot
    0x006276A0  ?pops@@YAHPADPAD...             gained two addresses it never calls

None of it is a matter of opinion: a direct `call rel32` is in the bytes at a
fixed offset, and the three fields are derivable from the annotation's OWN
spans plus each other. So they are derived here and compared, which is the
difference between a rule an agent is asked to remember and a mechanism.

    tools/verify_call_edges.py            # the check the gate runs
    tools/verify_call_edges.py --repair   # rewrite the three fields from the image

WHAT IS NOT DERIVABLE. The sweep sees direct calls only. An indirect site -
`call dword ptr [eax+0x64]`, `call [0x669338]`, `call ebx` - has no target in
the bytes, and `// indirect` records those SITES separately. So a recorded
edge the sweep cannot see is not automatically wrong (IDA had the whole
program and this has one function's bytes); it is capped and named instead.
An edge the sweep DOES see and the annotation omits is always wrong, and that
is the half every one of the seven damaged rows failed.
"""
from __future__ import annotations

import argparse
import bisect
import collections
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import annotation_scan  # noqa: E402
import emit_translation_unit as emit  # noqa: E402
import project_catalogue  # noqa: E402
from generator_support import read_bytes  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"

# A RECORDED EDGE THE BYTES CANNOT CONFIRM. One row, and it predates `src/`
# becoming the store: `?filefind_init@@YAHPADH@Z` at 0x00600400 is catalogued
# as calling `?exec@BasePop@@QAEHHP6AHXZ@Z` at 0x00602600, and no `call` in its
# 453 bytes names that address. IDA saw the whole program; this sees one
# function. The number is pinned rather than the address ignored, so a row
# acquiring an unconfirmable edge has to be looked at - which is how
# 0x0066931C, an import slot an agent wrote into `// calls`, would have been
# caught the day it was written.
UNCONFIRMED_FLOOR = 1

CALLS_LINE = re.compile(r"^(//|\*) calls( +.*)?$")
CALLERS_LINE = re.compile(r"^(//|\*) callers +(\S+)\s+call targets\s+(\S+)\s*$")


def load_image(exe: Path):
    import capstone
    import pefile
    return (pefile.PE(str(exe)),
            capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32))


def executable_ranges(image) -> list:
    """Where code can be, from the section table.

    "Not a catalogued function" is the wrong test for an address that should
    not be in `// calls`: run against a one-row tree - which is what the
    damage case builds - every real callee is uncatalogued too, and the case
    would pass with the check it exists to prove deleted. Being outside every
    executable section is a fact about the IMAGE and holds whatever the tree
    contains. `0x0066931C`, the import slot an agent wrote into a call list,
    is in `.rdata` and fails it; a wrong address that is nonetheless code
    fails as an edge the bytes do not make, which is the other test.
    """
    base = image.OPTIONAL_HEADER.ImageBase
    out = []
    for section in image.sections:
        if section.Characteristics & 0x20000000:      # IMAGE_SCN_MEM_EXECUTE
            start = base + section.VirtualAddress
            out.append((start, start + max(section.Misc_VirtualSize,
                                           section.SizeOfRawData)))
    return out


def spans_of(row: dict) -> list:
    out = []
    for part in str(row.get("body_ranges") or "").split(";"):
        if "-" in part:
            low, _, high = part.partition("-")
            out.append((int(low, 16), int(high, 16)))
    return out


def ownership(rows: dict):
    """A raw target -> the catalogued row whose span contains it.

    The catalogue records the edge to the CALLEE'S ENTRY POINT; the bytes
    record whatever `call rel32` encodes, and MSVC's CRT float helpers are
    entered mid-function on purpose - `__CIsin` calls 0x006463ED, which is
    inside 0x006463E4. Comparing raw targets called nine such rows wrong
    before this existed.
    """
    spans = []
    for address, row in rows.items():
        for low, high in spans_of(row):
            spans.append((low, high, address))
    spans.sort()
    starts = [one[0] for one in spans]

    def owner(target: int) -> int:
        index = bisect.bisect_right(starts, target) - 1
        if index >= 0 and spans[index][0] <= target < spans[index][1]:
            return spans[index][2]
        return target

    return owner


def direct_calls(image, decoder, row: dict, owner):
    """(targets, decoded) - the direct call edges in a row's own spans.

    `decoded` is False when the linear sweep does not reach the end of a span:
    a body carrying data (a jump table, `_memcpy`'s inlined constants) is not
    a body this can read, and "I could not read it" is not evidence that an
    edge is missing.
    """
    found, decoded = set(), True
    for low, high in spans_of(row):
        data = read_bytes(image, low, high - low)
        if len(data) != high - low:
            return found, False
        covered = 0
        for instruction in decoder.disasm(data, low):
            covered = instruction.address + instruction.size - low
            if instruction.mnemonic == "call" and \
                    instruction.op_str.startswith("0x"):
                found.add(owner(int(instruction.op_str, 16)))
        if covered != high - low:
            decoded = False
    return found, decoded


class Finding:
    def __init__(self, address, name, omitted, unconfirmed, uncatalogued,
                 targets_said, targets_are, callers_said, callers_are):
        self.address = address
        self.name = name
        self.omitted = omitted
        self.unconfirmed = unconfirmed
        self.uncatalogued = uncatalogued
        self.targets_said = targets_said
        self.targets_are = targets_are
        self.callers_said = callers_said
        self.callers_are = callers_are

    @property
    def undercounts_callers(self) -> bool:
        # A FLOOR, NOT AN EQUALITY. `// callers` is IDA's xref count over the
        # whole program - it includes the indirect and data references no
        # sweep of one function's bytes can reproduce - so a row with MORE
        # callers than direct edges is the normal case, not a defect. Fewer is
        # impossible: an edge exists that the count denies. `?` means the
        # catalogue never recorded one and is left alone.
        return self.callers_said >= 0 and self.callers_said < self.callers_are

    @property
    def wrong(self) -> bool:
        return bool(self.omitted or self.uncatalogued
                    or self.targets_said != self.targets_are
                    or self.undercounts_callers)


def survey(rows: dict, image, decoder) -> tuple:
    """(findings, undecodable) over every catalogued row."""
    owner = ownership(rows)
    code = executable_ranges(image)

    def is_code(address: int) -> bool:
        return any(low <= address < high for low, high in code)

    inbound = collections.Counter()
    for row in rows.values():
        for target in row["_calls"]:
            inbound[target] += 1

    findings, undecodable = [], []
    for address, row in sorted(rows.items()):
        if not spans_of(row):
            continue
        seen, decoded = direct_calls(image, decoder, row, owner)
        if not decoded:
            undecodable.append(address)
            continue
        recorded = set(row["_calls"])
        said_targets = row.get("call_target_count")
        said_callers = row.get("caller_count")
        finding = Finding(
            address, row.get("name", ""),
            omitted=sorted(seen - recorded),
            unconfirmed=sorted(recorded - seen),
            uncatalogued=sorted(one for one in recorded if not is_code(one)),
            targets_said=int(said_targets) if str(said_targets).isdigit() else -1,
            targets_are=len(recorded),
            callers_said=int(said_callers) if str(said_callers).isdigit() else -1,
            callers_are=inbound[address])
        findings.append(finding)
    return findings, undecodable


HEAD = "// calls     "
CONTINUATION = "//" + " " * (len(HEAD) - 2)


def reflow(tokens: list) -> list:
    """`// calls` and its continuation lines, wrapped at the value column.

    The tokens are TEXT, not integers, because a call list carries more than
    addresses: `0x005CEB12 (16x)` records sixteen call sites to one target,
    which an agent measured and which rebuilding the line from a set would
    throw away.
    """
    out, current = [], HEAD
    for token in tokens or ["(none)"]:
        if len(current) > len(HEAD) and len(current) + 1 + len(token) > 78:
            out.append(current)
            current = CONTINUATION
        current += (" " if len(current) > len(HEAD) else "") + token
    out.append(current)
    return out


def repair(findings: list, rows: dict) -> int:
    """Rewrite `// calls` and the two counts from what was just derived."""
    by_file = collections.defaultdict(list)
    resolved, _ = annotation_scan.resolve(annotation_scan.scan_tree())
    where = {one.address: (REPO_ROOT / one.path, one.line - 1)
             for one in resolved}
    for finding in findings:
        if finding.wrong and finding.address in where:
            path, index = where[finding.address]
            by_file[path].append((index, finding))

    written = 0
    for path, entries in by_file.items():
        lines = path.read_text().splitlines()
        # BOTTOM UP, so replacing a wrapped `// calls` with a differently
        # wrapped one does not move the annotation below it out from under
        # its own recorded line number.
        for index, finding in sorted(entries, reverse=True):
            recorded = str(rows[finding.address].get("_calls_text", ""))
            tokens, skip = [], {f"0x{one:08X}" for one in finding.uncatalogued}
            for token in recorded.split():
                if token.startswith("0x") and token.upper() not in \
                        {one.upper() for one in skip}:
                    tokens.append(token)
                elif token.startswith("(") and tokens:
                    tokens[-1] += " " + token      # a `(16x)` multiplicity
            tokens += [f"0x{one:08X}" for one in finding.omitted]
            tokens.sort()
            replacement = reflow(tokens)

            first, last = None, None
            for offset in range(index + 1, len(lines)):
                stripped = lines[offset].strip()
                if not (stripped.startswith("//") or stripped.startswith("*")):
                    break
                if first is None and CALLS_LINE.match(lines[offset]):
                    first = last = offset
                    continue
                if first is not None and last == offset - 1 and \
                        project_catalogue.CONTINUED.match(lines[offset]):
                    last = offset
                    continue
                match = CALLERS_LINE.match(lines[offset])
                if match:
                    # RAISED TO THE FLOOR, NEVER LOWERED. The recorded count
                    # is IDA's whole-program xref total and legitimately
                    # exceeds the direct edges this can see; `?` means it was
                    # never recorded and stays that way.
                    said = match.group(2)
                    callers = (said if not said.isdigit()
                               else str(max(int(said), finding.callers_are)))
                    lines[offset] = (f"// callers   {callers}   "
                                     f"call targets   {len(tokens)}")
            if first is not None:
                lines[first:last + 1] = replacement
            written += 1
        path.write_text("\n".join(lines) + "\n")
    return written


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--src", type=Path, default=None,
                        help="a tree to read instead of src/, for the damage case")
    parser.add_argument("--repair", action="store_true",
                        help="rewrite the fields from the image")
    arguments = parser.parse_args(argv)

    if not arguments.exe.is_file():
        print(f"REFUSED: {arguments.exe} is not present. The edges are read "
              f"out of the image; there is nothing to hold the catalogue to "
              f"without it.", file=sys.stderr)
        return 2

    rows = (emit.load_functions() if arguments.src is None
            else project_catalogue.from_source(arguments.src))
    image, decoder = load_image(arguments.exe)
    findings, undecodable = survey(rows, image, decoder)

    omitting = [one for one in findings if one.omitted]
    inventing = [one for one in findings if one.uncatalogued]
    miscounted = [one for one in findings
                  if one.targets_said != one.targets_are
                  or one.undercounts_callers]
    unconfirmed = [one for one in findings if one.unconfirmed]

    # DISTINCT ROWS. Summing the three lists counts a row twice when it fails
    # two ways, and 0x00636740 fails all three - which printed "-2 of 1 row(s)
    # agree" the first time the damage case ran it.
    agreeing = len(findings) - len({one.address for one in
                                    omitting + inventing + miscounted})
    print(f"call edges held to {arguments.exe.name}: "
          f"{agreeing} of {len(findings)} row(s) agree")
    print(f"  {len(undecodable)} row(s) carry data the linear sweep cannot "
          f"decode, and are not scored")
    print(f"  {len(unconfirmed)} row(s) record an edge the bytes do not show "
          f"(pinned at {UNCONFIRMED_FLOOR})")

    if arguments.repair:
        print(f"repaired {repair(findings, rows)} row(s)")
        return 0

    failed = False
    for finding in omitting:
        print(f"  0x{finding.address:08X} {finding.name}: `// calls` omits "
              f"{', '.join(f'0x{one:08X}' for one in finding.omitted)}, which "
              f"its own bytes call directly", file=sys.stderr)
        failed = True
    for finding in inventing:
        print(f"  0x{finding.address:08X} {finding.name}: `// calls` names "
              f"{', '.join(f'0x{one:08X}' for one in finding.uncatalogued)}, "
              f"which no catalogued function owns", file=sys.stderr)
        failed = True
    for finding in miscounted:
        if finding.targets_said != finding.targets_are:
            # NOT LENIENT ON AN UNSTATED COUNT, unlike `callers`. This one is
            # the length of the list on the very next line, so `?` is not a
            # measurement that was never taken - it is a refusal to count.
            said = ("no number" if finding.targets_said < 0
                    else f"`call targets {finding.targets_said}`")
            print(f"  0x{finding.address:08X} {finding.name}: says {said} "
                  f"beside a `// calls` line naming {finding.targets_are}",
                  file=sys.stderr)
        if finding.undercounts_callers:
            print(f"  0x{finding.address:08X} {finding.name}: says "
                  f"`callers {finding.callers_said}` while "
                  f"{finding.callers_are} catalogued row(s) call it directly",
                  file=sys.stderr)
        failed = True
    if len(unconfirmed) > UNCONFIRMED_FLOOR:
        print(f"  {len(unconfirmed)} row(s) record an edge no `call` in their "
              f"own bytes makes, above the pinned {UNCONFIRMED_FLOOR}: "
              + ", ".join(f"0x{one.address:08X}" for one in unconfirmed[:8]),
              file=sys.stderr)
        failed = True

    if failed:
        print("FAIL: an annotation's call-graph facts disagree with the bytes "
              "it points at. src/ is the catalogue, so this is the evidence "
              "moving, not a comment going stale - repair with --repair.",
              file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
