#!/usr/bin/env python3
"""Check EVERY recovered signature against the decoration the binary carries.

`tools/audit_export_signedness.py` answers the same question for 314 exports,
because it reads the GCC symbol each `src/OpenSMACX.def` alias maps to. That is
12% of the recovered corpus. The other 88% - 2,209 functions with a committed
body and a catalogued mangled name - have never been checked against the binary
at all, and the export population that WAS checked came back 63% wrong.

This closes the gap by comparing two things that are both already in the
repository and neither derived from the other:

  original  - `decode_signature` over the catalogue's decorated name, which
              MSVC wrote mechanically and the linker relies on being exact.
  recovered - the parameter list of the committed definition, read at the
              `source_locations` line functions.csv records.

WHAT COUNTS AS A DISAGREEMENT. Only signedness and only on integer scalars.
That is the defect this exists to find - `bitmask` divided signed and was
recovered as `uint32_t`, which every differential oracle here agrees with on
non-negative inputs - and it is the one comparison that is safe to make
textually. A mangled name says `int`; it never says `factionID`, and it cannot
distinguish `BOOL` from `int` or `LPSTR` from `char *`. Those are spelled
differently on purpose and are NOT reported.

WHAT IT IS NOT. It is not a compile. A signature it cannot parse is counted as
UNREADABLE and named, not quietly skipped: an audit whose denominator moves
without saying so is worse than no audit, because the total reads as coverage.
"""

from __future__ import annotations

import argparse
import collections
import csv
import json
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import byte_match_census as census  # noqa: E402
from mizuchi_declfix import decode_signature  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
FUNCTIONS = REPO_ROOT / "docs" / "recovery" / "functions.csv"

# How the tree spells each signedness. Only these are compared; anything else
# in either position makes the parameter UNCOMPARABLE rather than wrong.
SIGNED = {"int", "int32_t", "long", "signed int", "short", "int16_t", "char", "int8_t"}
UNSIGNED = {"unsigned int", "uint32_t", "unsigned long", "uint", "unsigned",
            "unsigned short", "uint16_t", "unsigned char", "uint8_t", "BYTE", "DWORD", "UINT"}


def signedness(spelling: str) -> str | None:
    """'signed' / 'unsigned' / None when the spelling is not a plain integer."""
    text = spelling.strip().rstrip("*&").strip()
    text = re.sub(r"\b(const|volatile)\b", "", text).strip()
    if text in SIGNED:
        return "signed"
    if text in UNSIGNED:
        return "unsigned"
    return None


def parameter_types(signature: str) -> list | None:
    """The declared type of each parameter, or None when unparseable."""
    open_paren = signature.find("(")
    close_paren = signature.rfind(")")
    if open_paren == -1 or close_paren <= open_paren:
        return None
    inner = signature[open_paren + 1:close_paren].strip()
    if inner in ("", "void"):
        return []
    out, depth, current = [], 0, ""
    for ch in inner:
        if ch in "(<[":
            depth += 1
        elif ch in ")>]":
            depth -= 1
        if ch == "," and depth == 0:
            out.append(current)
            current = ""
        else:
            current += ch
    out.append(current)
    # Drop the parameter NAME: the type is everything but the trailing
    # identifier, keeping any `*` with the type.
    types = []
    for param in out:
        param = param.strip()
        if not param:
            return None
        match = re.match(r"^(.*?)([A-Za-z_][A-Za-z0-9_]*)\s*(\[\s*\])?$", param)
        types.append((match.group(1) if match and match.group(1).strip() else param).strip())
    return types


def definition_signature(location: str) -> str | None:
    """The signature line of the committed definition at `src/foo.cpp:123`."""
    try:
        _, lines, start, end = census.body_span(location)
    except (ValueError, OSError):
        return None
    span = lines[start:end + 1]
    for index, line in enumerate(span):
        if "{" in line:
            # Signatures can wrap; join back to the previous blank/comment edge.
            head = index
            while head > 0 and span[head - 1].strip() and "*/" not in span[head - 1]:
                head -= 1
            return " ".join(part.strip() for part in span[head:index + 1])
        if "*/" in line:
            continue
    return None


def _strip_thiscall_shim(signature: str, params: list | None) -> list | None:
    """Drop the `self` pair a __fastcall redirect spells out.

    A __thiscall method's catalogued source location points at its redirect
    thunk, not at the method: `int __fastcall x_redirect(void *self, void *,
    int a1, int a2)`. The mangled name counts neither `self` nor the empty
    slot __fastcall requires, so every one of these read as a parameter-count
    mismatch — the single largest bucket of "unreadable", and consistently +2.
    """
    if params is None or len(params) < 2:
        return params
    if "__fastcall" not in signature:
        return params
    first, second = params[0].replace(" ", ""), params[1].replace(" ", "")
    if first in ("void*", "void*self") and second == "void*":
        return params[2:]
    return params


def audit(functions_path: Path) -> tuple:
    findings, unreadable, compared = [], [], 0
    with functions_path.open(newline="", encoding="utf-8-sig") as handle:
        rows = list(csv.DictReader(handle))

    for row in rows:
        name = (row.get("name") or "").strip()
        location = (row.get("source_locations") or "").split(";")[0].strip()
        if not name.startswith("?") or not location.startswith("src/"):
            continue
        decoded = decode_signature(name)
        if decoded is None:
            continue
        _, original_params = decoded

        signature = definition_signature(location)
        if signature is None:
            unreadable.append((row["address"], name, "no signature at source_locations"))
            continue
        recovered_params = parameter_types(signature)
        recovered_params = _strip_thiscall_shim(signature, recovered_params)
        if recovered_params is None:
            unreadable.append((row["address"], name, "unparseable parameter list"))
            continue
        if len(recovered_params) != len(original_params):
            unreadable.append((row["address"], name,
                               f"{len(original_params)} params in the name, "
                               f"{len(recovered_params)} in the source"))
            continue

        compared += 1
        mismatches = []
        for index, (want, got) in enumerate(zip(original_params, recovered_params)):
            want_sign, got_sign = signedness(want), signedness(got)
            if want_sign and got_sign and want_sign != got_sign:
                mismatches.append({"index": index, "original": want.strip(),
                                   "recovered": got.strip()})
        if mismatches:
            findings.append({"address": row["address"], "name": name,
                             "location": location, "arguments": mismatches})
    return compared, findings, unreadable


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--functions", type=Path, default=FUNCTIONS)
    parser.add_argument("--json", action="store_true")
    parser.add_argument("--limit", type=int, default=25)
    arguments = parser.parse_args()

    compared, findings, unreadable = audit(arguments.functions)
    if arguments.json:
        print(json.dumps({"compared": compared, "findings": findings,
                          "unreadable": [{"address": a, "name": n, "why": w}
                                         for a, n, w in unreadable]}, indent=2))
        return 0

    print(f"signatures compared : {compared}")
    print(f"disagreements       : {len(findings)}")
    print(f"unreadable          : {len(unreadable)}")
    if unreadable:
        why = collections.Counter(w for _, _, w in unreadable)
        for reason, count in why.most_common(5):
            print(f"    {count:5d}  {reason}")
    print()
    for finding in findings[:arguments.limit]:
        args = ", ".join(f"arg{a['index']} {a['recovered']}->{a['original']}"
                         for a in finding["arguments"])
        print(f"  {finding['address']} {finding['name']}")
        print(f"      {finding['location']}: {args}")
    if len(findings) > arguments.limit:
        print(f"  ... and {len(findings) - arguments.limit} more")
    return 0


if __name__ == "__main__":
    sys.exit(main())
