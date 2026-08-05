#!/usr/bin/env python3
"""Rewrite recovered signatures to the signedness their own mangled name states.

`tools/audit_export_signedness.py` reports the disagreements; this applies
them. The two are deliberately separate: the audit compares two independent
sources and must stay readable as evidence, while this edits the tree and is
only ever as trustworthy as the audit it is fed.

WHAT IT CHANGES, and nothing else. Every correction in the population is
`unsigned -> signed` on a scalar: measured over the 198 findings, 321 of 322
argument corrections are `uint -> int` and one is `uint* -> int*`, and 54
functions also carry the wrong RETURN type. So the rewrite is narrow enough to
do textually: in the declaration and the definition, the flagged parameter's
`uint32_t` becomes `int`, and the return type follows the mangled name. A
signature this tool cannot parse confidently is REFUSED and left alone rather
than guessed at - the refusals are the output that matters, because a silently
mangled signature is worse than an unfixed one.

WHY THE RETURN TYPE NEEDS SEPARATE WORK. The audit reports arguments only, so
fixing just what it lists leaves 54 decorations still wrong and still
unmatchable. The return type is read straight out of the original's mangled
name here rather than from the audit.

THE EXPORT ALIAS IS ADDITIVE. `tools/verify_def_append_only.py` records why:
the staged hybrid image carries a frozen import table naming 462 symbols, and
renaming an alias broke the game once already - on 2026-08-01, applying exactly
this correction to `?bitmask@@YAXIPAI0@Z`. So the old decoration keeps being
exported and is retargeted at the new GCC symbol, and the corrected decoration
is added beside it.

Usage:
    tools/fix_export_signedness.py --dry-run          # what would change
    tools/fix_export_signedness.py --limit 10         # apply the first 10
    tools/fix_export_signedness.py                    # apply all
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import audit_export_signedness as audit  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
SRC = REPO_ROOT / "src"
DEF_FILE = SRC / "OpenSMACX.def"

# The recovered spellings that mean "unsigned 32-bit" in this tree, and what
# each becomes. Anything else in a flagged position is a refusal.
UNSIGNED = ("uint32_t", "unsigned int", "unsigned", "uint")
SIGNED = "int"


class Refused(Exception):
    """This signature was left untouched."""


def demangled_name(decorated: str) -> str:
    """`?arm_val@@YAHHH@Z` -> `arm_val`; `?zoc_path@Path@@QAEHHHH@Z` -> `Path::zoc_path`."""
    body = decorated.lstrip("?").split("@@")[0]
    parts = [p for p in body.split("@") if p]
    if len(parts) == 1:
        return parts[0]
    return "::".join(reversed(parts))


def split_params(text: str) -> list:
    """Split a parameter list on top-level commas."""
    out, depth, current = [], 0, ""
    for ch in text:
        if ch in "(<[":
            depth += 1
        elif ch in ")>]":
            depth -= 1
        if ch == "," and depth == 0:
            out.append(current)
            current = ""
        else:
            current += ch
    if current.strip():
        out.append(current)
    return out


SIGNED_SPELLINGS = ("int32_t", "signed int", "int")
UNSIGNED_SPELLINGS = ("uint32_t", "unsigned int", "unsigned long", "unsigned", "uint")


def retype(param: str, want: str = "signed") -> str:
    """Rewrite one parameter's type to the wanted signedness, or refuse.

    Direction matters. The export audit only ever reported unsigned recoveries
    of signed originals, so the first version could only widen that way. The
    corpus-wide audit finds 35 argument corrections running the OTHER way -
    ?fade@Wave_Device@@QAEHI@Z and ?load@Wave@@QAEHPADK@Z take unsigned in the
    binary and int in the recovery - and fixing only one direction would leave
    those permanently wrong.
    """
    have = UNSIGNED_SPELLINGS if want == "signed" else SIGNED_SPELLINGS
    replacement = SIGNED if want == "signed" else "uint32_t"
    for spelling in have:
        pattern = rf"(^|[^A-Za-z0-9_]){re.escape(spelling)}(?![A-Za-z0-9_])"
        if re.search(pattern, param):
            return re.sub(pattern, rf"\g<1>{replacement}", param, count=1)
    raise Refused(f"no {'unsigned' if want == 'signed' else 'signed'} spelling "
                  f"in parameter {param.strip()!r}")


def rewrite_signature(line: str, wants: dict, want_return: str | None) -> str:
    """Rewrite the flagged parameters (and optionally the return type)."""
    open_paren = line.find("(")
    close_paren = line.rfind(")")
    if open_paren == -1 or close_paren == -1 or close_paren < open_paren:
        raise Refused("no parameter list on the signature line")

    head, params_text, tail = line[:open_paren], line[open_paren + 1:close_paren], line[close_paren:]
    params = split_params(params_text)
    if params and params[0].strip() in ("void", ""):
        params = []

    # A __thiscall method's catalogued body is its __fastcall redirect, which
    # spells out `self` and the empty slot. The audit strips that pair before
    # comparing, so its indices are two short of the source's.
    offset = 0
    if "__fastcall" in line and len(params) >= 2:
        first, second = params[0].replace(" ", ""), params[1].replace(" ", "")
        if first in ("void*", "void*self") and second == "void*":
            offset = 2

    for index, want in sorted(wants.items()):
        position = index + offset
        if position >= len(params):
            raise Refused(f"argument {index} beyond the {len(params) - offset} parameters declared")
        params[position] = retype(params[position], want)

    if want_return:
        head = retype(head, want_return)

    return head + "(" + ",".join(params) + tail


def rewrite_wrapped(lines: list, brace_index: int, wants: dict,
                    want_return: str | None) -> tuple:
    """Rewrite a signature that wraps across lines, preserving the line count.

    Collapsing the wrap into one line would be far simpler and is not safe:
    functions.csv records every body by `src/foo.cpp:LINE`, so removing lines
    shifts every catalogued location below it in that file. Each parameter is
    therefore edited in the line it actually occupies.

    Returns (first line index, rewritten lines).
    """
    # Find the signature block in BOTH directions. A definition anchors on its
    # opening brace and may have opened its parameter list lines earlier; a
    # header declaration anchors on the line holding `(` and may not close it
    # until lines later. Handling only the first case left every wrapped
    # DECLARATION refused while its definition was rewritten.
    start = brace_index
    while start > 0 and "(" not in lines[start]:
        start -= 1
    if "(" not in lines[start]:
        raise Refused("no parameter list near the signature")
    stop = max(start, brace_index)
    while stop < len(lines) - 1 and lines[start:stop + 1].count(")") == 0 and \
            "\n".join(lines[start:stop + 1]).count("(") > "\n".join(lines[start:stop + 1]).count(")"):
        stop += 1
    while stop < len(lines) - 1 and \
            "\n".join(lines[start:stop + 1]).count("(") > "\n".join(lines[start:stop + 1]).count(")"):
        stop += 1

    block = lines[start:stop + 1]
    joined = "\n".join(block)
    open_paren = joined.find("(")
    close_paren = joined.rfind(")")
    if close_paren <= open_paren:
        raise Refused("unbalanced parameter list across the wrap")

    # Walk the parameter list once, recording each parameter's character span
    # in `joined` so the edit lands in the right physical line.
    spans, depth, begin = [], 0, open_paren + 1
    for index in range(open_paren + 1, close_paren):
        ch = joined[index]
        if ch in "(<[":
            depth += 1
        elif ch in ")>]":
            depth -= 1
        elif ch == "," and depth == 0:
            spans.append((begin, index))
            begin = index + 1
    spans.append((begin, close_paren))

    offset = 0
    if "__fastcall" in joined and len(spans) >= 2:
        first = joined[spans[0][0]:spans[0][1]].replace(" ", "").replace("\n", "")
        second = joined[spans[1][0]:spans[1][1]].replace(" ", "").replace("\n", "")
        if first in ("void*", "void*self") and second == "void*":
            offset = 2

    edits = []
    for index, want in wants.items():
        position = index + offset
        if position >= len(spans):
            raise Refused(f"argument {index} beyond the {len(spans) - offset} parameters declared")
        low, high = spans[position]
        edits.append((low, high, retype(joined[low:high], want)))
    if want_return:
        edits.append((0, open_paren, retype(joined[:open_paren], want_return)))

    for low, high, replacement in sorted(edits, reverse=True):
        joined = joined[:low] + replacement + joined[high:]

    rewritten = joined.split("\n")
    if len(rewritten) != len(block):
        raise Refused("rewrite changed the line count")
    return start, rewritten


def definition_line(address: int) -> tuple:
    """(path, line index) of the definition, from the catalogue - not guessed.

    The first locator searched every header and .cpp for `name(` and took the
    first hit, which matched call sites (`parse_num(0, ...)`) and same-named
    methods on other classes (every `init(` in the tree resolved to
    autosound.h). functions.csv already records where each body is; using it
    removes the guessing entirely.
    """
    import byte_match_census as census

    row = _catalogue().get(address)
    if row is None:
        raise Refused("not in functions.csv")
    location = (row.get("source_locations") or "").split(";")[0].strip()
    if not location.startswith("src/"):
        raise Refused("no src/ source_locations")
    path, lines, start, end = census.body_span(location)
    offset = _definition_offset(lines[start:end + 1])
    return path, start + offset


def _definition_offset(span: list) -> int:
    """Index of the signature line inside a catalogued span, past the doc comment."""
    for index, line in enumerate(span):
        if "{" in line:
            return index
        if "*/" in line:
            return index + 1
    raise Refused("catalogued span has no opening brace")


_CATALOGUE = {}


def _catalogue() -> dict:
    if not _CATALOGUE:
        import csv
        with (REPO_ROOT / "docs" / "recovery" / "functions.csv").open(newline="") as handle:
            for row in csv.DictReader(handle):
                try:
                    _CATALOGUE[int(row["address"], 16)] = row
                except (KeyError, ValueError):
                    continue
    return _CATALOGUE


def declaration_line(name: str) -> tuple:
    """(path, line index) of the header declaration, or refuse when ambiguous."""
    simple = name.split("::")[-1]
    klass = name.split("::")[0] if "::" in name else None
    pattern = re.compile(rf"(^|[^A-Za-z0-9_]){re.escape(simple)}\s*\(")
    hits = []
    for path in sorted(SRC.glob("*.h")):
        text = path.read_text()
        if klass and f"class {klass}" not in text and f"struct {klass}" not in text:
            continue
        for index, line in enumerate(text.splitlines()):
            stripped = line.strip()
            # A declaration ends in `;` and is not a comment. That alone rules
            # out every call site, which is what the first version tripped on.
            if not stripped.endswith(";") or stripped.startswith("//"):
                continue
            if pattern.search(line):
                hits.append((path, index))
    if not hits:
        raise Refused(f"no header declaration for {simple}")
    if len(hits) > 1:
        raise Refused(f"{len(hits)} header declarations for {simple}; ambiguous")
    return hits[0]


def apply_finding(finding: dict, dry_run: bool) -> str:
    """finding: {address, name (decorated), wants {index: signedness}, want_return}."""
    name = demangled_name(finding["name"])
    address = int(finding["address"], 16)

    targets = [definition_line(address)]
    try:
        targets.append(declaration_line(name))
    except Refused:
        pass  # header-less statics are legitimate; the definition still counts

    changed = []
    for path, index in targets:
        lines = path.read_text().splitlines()
        try:
            block_start, block = rewrite_wrapped(lines, index, finding["wants"],
                                                 finding.get("want_return"))
        except Refused as error:
            raise Refused(f"{path.name}:{index + 1}: {error}")
        if block == lines[block_start:block_start + len(block)]:
            continue
        lines[block_start:block_start + len(block)] = block
        if not dry_run:
            path.write_text("\n".join(lines) + "\n")
        changed.append(f"{path.name}:{index + 1}")

    if not changed:
        raise Refused("signature already agrees, or no textual change needed")
    return ", ".join(changed)


def _wide_findings() -> list:
    """Normalise audit_recovered_signatures output, which covers 85% not 12%."""
    import audit_recovered_signatures as wide
    from mizuchi_declfix import decode_signature

    compared, findings, _ = wide.audit(wide.FUNCTIONS)
    out = []
    for finding in findings:
        wants = {}
        for argument in finding["arguments"]:
            sign = wide.signedness(argument["original"])
            if sign:
                wants[argument["index"]] = sign
        if not wants:
            continue
        # The audit reports arguments only; the return type is read here.
        want_return = None
        decoded = decode_signature(finding["name"])
        signature = wide.definition_signature(finding["location"])
        if decoded and signature:
            want = wide.signedness(decoded[0])
            head = signature[:signature.find("(")]
            got = wide.signedness(head.split()[-2] if len(head.split()) > 1 else "")
            if want and got and want != got:
                want_return = want
        out.append({"address": finding["address"], "name": finding["name"],
                    "wants": wants, "want_return": want_return})
    return compared, out


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--dry-run", action="store_true")
    parser.add_argument("--wide", action="store_true",
                        help="use audit_recovered_signatures (85% coverage, both directions)")
    parser.add_argument("--limit", type=int, default=0)
    arguments = parser.parse_args()

    if arguments.wide:
        compared, findings = _wide_findings()
    else:
        compared, raw = audit.audit(audit.DEFAULT_FUNCTIONS, audit.DEFAULT_DEF,
                                    audit.DEFAULT_EXE)
        findings = [{"address": f["address"], "name": f["original"],
                     "wants": {a["index"]: "signed" for a in f["arguments"]},
                     "want_return": "signed" if _return_differs(f["original"], f["alias"]) else None}
                    for f in raw]
    print(f"{compared} signatures compared, {len(findings)} disagreements")
    if arguments.limit:
        findings = findings[:arguments.limit]

    applied, refused = 0, []
    for finding in findings:
        try:
            where = apply_finding(finding, arguments.dry_run)
        except Refused as error:
            refused.append(f"{finding['name']}: {error}")
            continue
        applied += 1
        print(f"  {'would fix' if arguments.dry_run else 'fixed'} {demangled_name(finding['name']):32s} {where}")

    print(f"\n{applied} corrected, {len(refused)} refused")
    for line in refused[:20]:
        print(f"  REFUSED {line}")
    if len(refused) > 20:
        print(f"  ... and {len(refused) - 20} more")
    return 0


def _return_differs(original: str, alias: str) -> bool:
    def types(name):
        match = re.search(r"@@[A-Z]{2}(.*)@Z$", name)
        return match.group(1) if match else ""
    left, right = types(original), types(alias)
    return bool(left and right and left[0] != right[0])


if __name__ == "__main__":
    sys.exit(main())
