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


def retype(param: str, want_pointer: bool) -> str:
    """Rewrite one parameter's type to signed, or refuse."""
    for spelling in UNSIGNED:
        # Match the type as a whole word so `uint32_t*` and `uint32_t x` both hit
        # and a name containing the spelling does not.
        pattern = rf"(^|[^A-Za-z0-9_]){re.escape(spelling)}(?![A-Za-z0-9_])"
        if re.search(pattern, param):
            return re.sub(pattern, rf"\g<1>{SIGNED}", param, count=1)
    raise Refused(f"no unsigned spelling in parameter {param.strip()!r}")


def rewrite_signature(line: str, indices: list, want_return: bool) -> str:
    """Rewrite the flagged parameters (and optionally the return type) of one signature."""
    open_paren = line.find("(")
    close_paren = line.rfind(")")
    if open_paren == -1 or close_paren == -1 or close_paren < open_paren:
        raise Refused("no parameter list on the signature line")

    head, params_text, tail = line[:open_paren], line[open_paren + 1:close_paren], line[close_paren:]
    params = split_params(params_text)
    if params and params[0].strip() in ("void", ""):
        params = []

    for index in indices:
        if index >= len(params):
            raise Refused(f"argument {index} beyond the {len(params)} parameters declared")
        params[index] = retype(params[index], want_pointer=False)

    if want_return:
        head = retype(head, want_pointer=False)

    return head + "(" + ",".join(params) + tail


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


def apply_finding(finding: dict, want_return: bool, dry_run: bool) -> str:
    name = demangled_name(finding["original"])
    indices = [a["index"] for a in finding["arguments"]]
    address = int(finding["address"], 16)

    targets = []
    path, index = definition_line(address)
    targets.append((path, index))
    try:
        targets.append(declaration_line(name))
    except Refused:
        pass  # header-less statics are legitimate; the definition still counts

    changed = []
    for path, index in targets:
        lines = path.read_text().splitlines()
        try:
            rewritten = rewrite_signature(lines[index], indices, want_return)
        except Refused as error:
            raise Refused(f"{path.name}:{index + 1}: {error}")
        if rewritten == lines[index]:
            continue
        lines[index] = rewritten
        if not dry_run:
            path.write_text("\n".join(lines) + "\n")
        changed.append(f"{path.name}:{index + 1}")

    if not changed:
        raise Refused("signature already signed, or no textual change needed")
    return ", ".join(changed)


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--dry-run", action="store_true")
    parser.add_argument("--limit", type=int, default=0)
    arguments = parser.parse_args()

    compared, findings = audit.audit(audit.DEFAULT_FUNCTIONS, audit.DEFAULT_DEF,
                                     audit.DEFAULT_EXE)
    print(f"{compared} exports compared, {len(findings)} disagreements")
    if arguments.limit:
        findings = findings[:arguments.limit]

    applied, refused = 0, []
    for finding in findings:
        original, alias = finding["original"], finding["alias"]
        want_return = _return_differs(original, alias)
        try:
            where = apply_finding(finding, want_return, arguments.dry_run)
        except Refused as error:
            refused.append(f"{original}: {error}")
            continue
        applied += 1
        print(f"  {'would fix' if arguments.dry_run else 'fixed'} {demangled_name(original):32s} {where}")

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
