#!/usr/bin/env python3
"""The source map: one grammar for every mapped byte of the binary in src/.

This is the reader for the single annotation convention the project is
consolidating on.  The map answers, for any address in the catalogue, WHERE
its decompilation lives and what STATE it is in - without any CSV:

    ORIGINAL: 0x00401640            the function definition that follows is
                                    the decompiled body; it is extracted and
                                    measured in generated VC6 scaffolding
    ORIGINAL: 0x00403B70 FILE       the WHOLE file is the translation unit
                                    for this address; compiled verbatim
    ORIGINAL: 0x0064A123 EXCLUDED S1   deliberately not decompiled; the token
                                    after EXCLUDED cites the ground in
                                    docs/EXCLUSIONS.md (section 1 here)

The marker is valid in any comment form, `//` or inside `/* */`, so the entire
map is `grep -rn "ORIGINAL: 0x" src/`.

STATE IS MEASURED, NOT CLAIMED. The only declared state is EXCLUDED, because
exclusion is a decision. Everything else is derived: a region that still holds
the `// BODY GOES HERE.` sentinel with no real code after it (the exact
`verify_wave.is_untouched` semantics) is a PLACEHOLDER; anything else is
IMPLEMENTED and belongs to the compiler. A file that claims a match in a
comment and does not hold one in bytes is not a match, and nothing here can
say otherwise.

DEPRECATED FORMS are still recognised read-only while the tree migrates, each
flagged `deprecated=True` so the migrator has an exact worklist and a scan can
assert when none remain:

  * `Original Offset: 00XXXXXX` doc-comment blocks (the 2,679 hand-written
    recoveries and generated thunk files);
  * the inline trailing form, `void f() {...} // 00XXXXXX`;
  * the two `src/recovered/` header styles - body-only proved files and
    preserved whole units, keyed by their 8-hex-digit filenames.

Once `tools/decomp_status.py --migrate` has rewritten the tree, recognition of
these forms is deleted, not maintained. No backward compatibility is owed.

Everything here is OFFLINE: no VC6, no Wine, no PE. The compiler belongs to
`tools/decomp_status.py`, which imports this module.
"""

from __future__ import annotations

import argparse
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

REPO_ROOT = Path(__file__).resolve().parent.parent
SRC_ROOT = REPO_ROOT / "src"
RECOVERED = SRC_ROOT / "recovered"
RECOVERED_UNITS = RECOVERED / "units"
UNRECOVERED = SRC_ROOT / "unrecovered"

# The one marker. 6-8 hex digits because the image spans 0x00401000 to
# 0x0066xxxx; `int(..., 16)` normalises. The optional trailing keyword is the
# mode: absent means the next definition is the body, FILE means the whole
# file is the unit, EXCLUDED cites the ground in docs/EXCLUSIONS.md.
# The lookbehind matters: `PROVEN-AGAINST-ORIGINAL: 0x...` provenance
# comments in the oracle files end in this spelling and are NOT map entries.
MARKER = re.compile(
    r"(?<![A-Za-z0-9_-])ORIGINAL:\s*0x(?P<addr>[0-9A-Fa-f]{6,8})"
    r"(?P<tail>[^\n*]*)")
# The keyword, read out of the marker's own tail rather than by a second
# alternation, so `FILE` and `BYTE_EXACT` may appear in either order.
MARKER_KEYWORD = re.compile(r"^\s*(?P<kw>FILE|EXCLUDED)\b(?P<rest>.*)$", re.S)
# A RATCHET CLAIM, NOT A STATUS FIELD. `// ORIGINAL: 0x0046FB10 BYTE_EXACT`
# means "this was proved to recompile to the shipped bytes; fail if it stops",
# and that is the ONLY tier ever written down. Every other verdict -
# MISMATCH, NO_COMPILE, MNEMONIC_ONLY - is measured on demand and recorded
# nowhere, because a status field in a comment goes stale the moment anyone
# edits the body and is believed anyway. The claim lives beside the body it
# constrains, so deleting the body deletes the claim.
MARKER_MATCHED = re.compile(r"(?<![A-Za-z0-9_])BYTE_EXACT(?![A-Za-z0-9_])")

LEGACY_BLOCK = re.compile(r"Original Offset:\s*(?P<addr>[0-9A-Fa-f]{6,8})")
# The inline trailing form, e.g. src/random.cpp's
# `void __cdecl random_rand() {...} // 00625700`. Eight digits required: a
# shorter bare hex token is too easy to hit in ordinary prose.
LEGACY_TRAILING = re.compile(r"\}\s*//\s*(?P<addr>[0-9A-Fa-f]{8})\s*$")
# The opening-brace form in src/text.cpp: `int f(...) { // 00585120`. The
# census never scored these - a forward brace count from the marker line
# lands in the NEXT function - which is why they sit unlocated in the
# catalogue. Forward extraction from the marker line is correct here.
LEGACY_OPENING = re.compile(r"\{\s*//\s*(?P<addr>[0-9A-Fa-f]{8})\s*$")
# Preserved unit header, e.g. `// PRESERVED UNIT - measured BYTE_EXACT.`.
LEGACY_PRESERVED = re.compile(r"PRESERVED UNIT\s*-\s*measured\s+(?P<tier>\w+)")
# Proved body header, e.g. `// 0x00401000  ??0StringStruct@@QAE@H@Z -> ...`.
LEGACY_PROVED = re.compile(r"^//\s*0x(?P<addr>[0-9A-Fa-f]{8})\b")

# The citation token after EXCLUDED is a section reference, not free prose.
# Accept `S1`, `1`, or `§1` so the grammar is ASCII-safe and greppable.
EXCLUSION_TOKEN = re.compile(r"^\s*[§S]?\s*(?P<section>[0-9]+[a-z]?)")

MODE_BODY = "body"
MODE_FILE = "file"
STATE_IMPLEMENTED = "implemented"
STATE_PLACEHOLDER = "placeholder"
STATE_EXCLUDED = "excluded"


@dataclass
class Annotation:
    """One mapped piece of the binary, as the source tree declares it."""
    address: int
    mode: str                  # MODE_BODY | MODE_FILE
    state: str                 # STATE_IMPLEMENTED | STATE_PLACEHOLDER | STATE_EXCLUDED
    path: str                  # repo-relative, e.g. "src/stringstruct.cpp"
    line: int                  # 1-based line of the marker (0 for filename-derived)
    deprecated: bool = False   # recognised legacy spelling, pending migration
    exclusion: str = ""        # EXCLUSIONS.md citation for STATE_EXCLUDED
    region: str = ""           # the code this annotation claims ("" on error)
    extract_error: str = ""    # why the region could not be cut
    matched: bool = False      # carries a BYTE_EXACT ratchet claim: this piece
                               # was proved to recompile to the shipped bytes
                               # and must not stop. NOT a status field - see
                               # MARKER_MATCHED.
    recipe: str = "census"     # how the status tool must build the unit:
                               # "census" (extract + scaffolding), "writeback"
                               # (proved bodies, declfix recipe) or "verbatim"
                               # (FILE mode: compile the file as-is)
    levers: tuple = ()         # (fingerprint, prose) that MADE this match
    ruled_out: tuple = ()      # spellings tried on this body that did not

    @property
    def location(self) -> str:
        return f"{self.path}:{self.line}" if self.line else self.path

    @property
    def address_hex(self) -> str:
        return f"0x{self.address:08X}"


LESSON_LEVER = re.compile(r"^\s*(?://|\*)?\s*LEVER:\s*(?P<key>\S+)\s+(?P<prose>.+?)\s*$")
LESSON_RULED_OUT = re.compile(r"^\s*(?://|\*)?\s*RULED-OUT:\s*(?P<prose>.+?)\s*$")
LESSON_CONTINUED = re.compile(r"^\s*(?://|\*)\s{2,}(?P<prose>\S.*?)\s*$")


def lessons(lines: list, index: int) -> tuple:
    """(levers, ruled_out) recorded in the comment run after a marker at `index`.

    THE TWO TOKENS CARRY DIFFERENT DURABILITY, and that is the whole design.

      LEVER: <fingerprint> <what worked>   on a body that MATCHED. The
          divergence is gone, so the fingerprint is historical and has to be
          written down or the lesson cannot be filed against anything.
      RULED-OUT: <what did not work>       on a body that has NOT matched. The
          divergence is still live, so the key is MEASURED rather than written -
          and a key that is never written can never be stale.

    Both are read only from the comment run immediately after the marker, so a
    mention of either word in ordinary prose further down a file is not a claim.
    A continuation line is an indented comment line carrying no token of its own,
    which is how a long RULED-OUT list stays readable.
    """
    levers, ruled, current = [], [], None
    for line in lines[index + 1:]:
        stripped = line.strip()
        if not (stripped.startswith("//") or stripped.startswith("*")):
            break
        lever = LESSON_LEVER.match(line)
        out = LESSON_RULED_OUT.match(line)
        if lever:
            levers.append((lever.group("key"), lever.group("prose")))
            current = ("lever", len(levers) - 1)
            continue
        if out:
            ruled.append(out.group("prose"))
            current = ("ruled", len(ruled) - 1)
            continue
        joined = LESSON_CONTINUED.match(line)
        if joined and current:
            kind, position = current
            if kind == "lever":
                key, prose = levers[position]
                levers[position] = (key, prose + " " + joined.group("prose"))
            else:
                ruled[position] = ruled[position] + " " + joined.group("prose")
            continue
        current = None
    return tuple(levers), tuple(ruled)


@dataclass
class CrossRef:
    """The scan held against the catalogue. Drift, not failure."""
    matched: dict = field(default_factory=dict)       # address -> Annotation
    duplicates: dict = field(default_factory=dict)    # address -> [Annotation]
    catalog_only: list = field(default_factory=list)  # [address]
    uncatalogued: list = field(default_factory=list)  # [Annotation]


# --------------------------------------------------------------- extraction


def _rel(path: Path) -> str:
    """Repo-relative when possible; absolute only for out-of-tree fixtures."""
    resolved = Path(path).resolve()
    try:
        return str(resolved.relative_to(REPO_ROOT))
    except ValueError:
        return str(resolved)


def extract_forward_text(lines: list, marker_line: int) -> str:
    """The region claimed by a marker at `marker_line` (1-based), in LINES.

    The brace counter, the inclusive closing-brace line, and the `/*`
    re-opening when the extract starts mid-comment are the census rule -
    duplicated line for line here only so the scanner can run on in-memory
    text (the migrator's dry-run), and pinned to
    `byte_match_census.body_span` over the whole tree by
    test_annotation_scan so the copy cannot drift from the canonical
    extractor it mirrors.
    """
    start = marker_line - 1
    depth, opened = 0, False
    end = None
    for offset, line in enumerate(lines[start:]):
        depth += line.count("{") - line.count("}")
        if "{" in line:
            opened = True
        if opened and depth <= 0:
            end = start + offset
            break
    if end is None:
        raise ValueError("no closing brace within the file")
    text = "\n".join(lines[start:end + 1]) + "\n"
    if "*/" in text.split("{", 1)[0]:
        text = "/*\n" + text
    return text


def backward_start(lines: list, marker_index: int) -> int:
    """Line index where the body claimed by a trailing marker opens.

    The marker sits on the definition's CLOSING-brace line, so the body is
    found by walking backwards until the braces balance. Single-line
    definitions balance immediately; a multi-line body ends at the line whose
    opening brace closes the count.
    """
    depth = 0
    for index in range(marker_index, -1, -1):
        depth += lines[index].count("}") - lines[index].count("{")
        if depth <= 0 and "{" in lines[index]:
            return index
    raise ValueError("no balanced opening brace above the trailing marker")


def extract_backward(lines: list, marker_index: int) -> str:
    """The region claimed by a trailing marker, for the inline legacy form."""
    start = backward_start(lines, marker_index)
    return "\n".join(lines[start:marker_index + 1]) + "\n"


# The sentinel names the body slot. Its spelling is shared with the wave
# tools (verify_wave.PLACEHOLDER), but what counts as UNTOUCHED is defined
# HERE, because the emitter's own scaffold breaks verify_wave's rule: a
# non-void skeleton carries a placeholder return after the sentinel so it
# compiles before a body exists (C4716 otherwise), and `is_untouched`'s
# one-line allowance reads that pristine scaffold as worked. The residue of
# an untouched region is exactly: comments, the closing brace, and the
# placeholder return - anything else is a body.
SENTINEL = "// BODY GOES HERE."


def _is_placeholder_region(text: str) -> bool:
    if SENTINEL not in text:
        return False
    after = text.split(SENTINEL, 1)[1]
    for line in after.splitlines():
        stripped = line.strip()
        if not stripped or stripped.startswith("//"):
            continue
        if stripped.startswith("}"):
            continue
        if stripped.startswith("return") and "PLACEHOLDER" in stripped:
            continue
        return False
    return True


def _state_of(region: str, excluded: str) -> str:
    if excluded:
        return STATE_EXCLUDED
    if _is_placeholder_region(region):
        return STATE_PLACEHOLDER
    return STATE_IMPLEMENTED


def _parse_marker(line: str, in_block: bool):
    """(address, keyword, rest, matched) for a marker IN A COMMENT."""
    match = MARKER.search(line)
    if not match:
        return None
    # The marker only counts inside a comment: either the line opens `//`
    # before it, or the scanner is inside a `/* */` block. A hit in code or
    # data is prose, not a map entry.
    prefix = line[:match.start()]
    if not in_block and "//" not in prefix and "/*" not in prefix:
        return None
    tail = match.group("tail") or ""
    matched = bool(MARKER_MATCHED.search(tail))
    keyword_hit = MARKER_KEYWORD.match(MARKER_MATCHED.sub("", tail))
    keyword = keyword_hit.group("kw") if keyword_hit else None
    rest = keyword_hit.group("rest") if keyword_hit else ""
    return int(match.group("addr"), 16), keyword, rest, matched


def _exclusion_citation(rest: str) -> str:
    rest = rest.strip()
    match = EXCLUSION_TOKEN.match(rest)
    if match:
        return f"S{match.group('section')}"
    return rest or "unspecified"


# ------------------------------------------------------------------ scanning


def scan_file(path: Path) -> list:
    """Every annotation one file declares, in line order."""
    path = Path(path)
    return scan_text(path.read_text(), path)


def scan_text(text: str, path) -> list:
    """Scan TEXT attributed to `path` - the migrator's dry-run surface.

    An explicit marker always wins. The legacy-store adapters fire only when a
    file under `src/recovered/` carries NO marker at all, so a migrated file
    is read by the new grammar even before the adapter is retired.
    """
    path = Path(path)
    lines = text.splitlines()
    kind = store_kind(path)
    found = []

    in_block = False
    for index, line in enumerate(lines):
        # Parse with the state the line OPENS in, then advance the state, so
        # a marker on the line that opens a comment is still inside it.
        parsed = _parse_marker(line, in_block)
        in_block = _block_state_after(line, in_block)
        if parsed is None:
            continue
        address, keyword, rest, matched = parsed
        found_levers, found_ruled = lessons(lines, index)
        if keyword == "FILE":
            region = text
            found.append(Annotation(
                address=address, mode=MODE_FILE,
                state=_state_of(region, ""), path=_rel(path),
                line=index + 1, region=region, recipe="verbatim",
                matched=matched, levers=found_levers, ruled_out=found_ruled))
        elif keyword == "EXCLUDED":
            found.append(Annotation(
                address=address, mode=MODE_BODY, state=STATE_EXCLUDED,
                path=_rel(path), line=index + 1,
                exclusion=_exclusion_citation(rest), matched=matched,
                levers=found_levers, ruled_out=found_ruled))
        else:
            if kind == "proved":
                # Proved bodies keep the writeback semantics even once an
                # explicit marker lands in the header: the body is the
                # definition after the leading `//` run, never a brace count
                # from the marker line (the header prose could carry braces).
                region, error = _proved_body(lines), ""
            else:
                try:
                    region = extract_forward_text(lines, index + 1)
                    error = ""
                except ValueError as problem:
                    region, error = "", str(problem)
            recipe = "writeback" if kind == "proved" else "census"
            found.append(Annotation(
                address=address, mode=MODE_BODY,
                state=_state_of(region, ""), path=_rel(path),
                line=index + 1, region=region, extract_error=error,
                recipe=recipe, matched=matched,
                levers=found_levers, ruled_out=found_ruled))

    if not found:
        found.extend(_legacy_file_annotations(path, text, lines, kind))

    # A file may legitimately map several pieces (the thunk files do, once
    # migrated); but the SAME address twice in one file is a defect the
    # cross-reference reports, so nothing is dropped here.
    return found


def _block_state_after(line: str, in_block: bool) -> bool:
    """Cheap block-comment tracking: count the transitions on the line.

    Deliberately naive about strings containing comment marks - a false
    positive there only makes the scanner MORE willing to read a marker, and
    a marker at an address the catalogue does not know is reported as
    uncatalogued rather than believed, so the error cannot fabricate a map
    entry.
    """
    depth = 1 if in_block else 0
    index = 0
    while index < len(line):
        if depth and line.startswith("*/", index):
            depth -= 1
            index += 2
            continue
        if not depth and line.startswith("/*", index):
            depth += 1
            index += 2
            continue
        if not depth and line.startswith("//", index):
            break
        index += 1
    return depth > 0


def store_kind(path: Path) -> str:
    """Which legacy store a file sits in, by directory SHAPE, not repo path.

    "units" when the parent is `recovered/units`, "proved" when the parent is
    `recovered`, and "" anywhere else - so the mechanism works in a fixture
    tree as well as in the real one, and a store relocated during the
    migration keeps its semantics. The file must be named for an address.
    """
    resolved = Path(path).resolve()
    if not re.fullmatch(r"[0-9a-f]{8}", resolved.stem):
        return ""
    if resolved.parent.name == "units" \
            and resolved.parent.parent.name == "recovered":
        return "units"
    if resolved.parent.name == "recovered":
        return "proved"
    return ""


def _legacy_file_annotations(path: Path, text: str, lines: list,
                             kind: str) -> list:
    """Read-only recognition of the pre-migration stores and spellings."""
    found = []
    resolved = Path(path).resolve()

    # The two src/recovered/ stores, keyed by filename.
    if kind == "units":
        found.append(Annotation(
            address=int(resolved.stem, 16), mode=MODE_FILE,
            state=_state_of(text, ""), path=_rel(path), line=0,
            deprecated=True, region=text, recipe="verbatim"))
        return found
    if kind == "proved":
        body = _proved_body(lines)
        found.append(Annotation(
            address=int(resolved.stem, 16), mode=MODE_BODY,
            state=_state_of(body, ""), path=_rel(path), line=0,
            deprecated=True, region=body, recipe="writeback"))
        return found

    # The two deprecated inline spellings, anywhere under src/.
    in_block = False
    for index, line in enumerate(lines):
        block_now = _block_state_after(line, in_block)
        if in_block or line.lstrip().startswith("/*"):
            match = LEGACY_BLOCK.search(line)
            if match and "// ORIGINAL:" not in line:
                address = int(match.group("addr"), 16)
                try:
                    region = extract_forward_text(lines, index + 1)
                    error = ""
                except ValueError as problem:
                    region, error = "", str(problem)
                found.append(Annotation(
                    address=address, mode=MODE_BODY,
                    state=_state_of(region, ""), path=_rel(path),
                    line=index + 1, deprecated=True, region=region,
                    extract_error=error, recipe="census"))
        trailing = LEGACY_TRAILING.search(line)
        if trailing:
            address = int(trailing.group("addr"), 16)
            try:
                region = extract_backward(lines, index)
                error = ""
            except ValueError as problem:
                region, error = "", str(problem)
            found.append(Annotation(
                address=address, mode=MODE_BODY,
                state=_state_of(region, ""), path=_rel(path),
                line=index + 1, deprecated=True, region=region,
                extract_error=error, recipe="census"))
            in_block = block_now
            continue
        opening = LEGACY_OPENING.search(line)
        if opening:
            address = int(opening.group("addr"), 16)
            try:
                region = extract_forward_text(lines, index + 1)
                error = ""
            except ValueError as problem:
                region, error = "", str(problem)
            found.append(Annotation(
                address=address, mode=MODE_BODY,
                state=_state_of(region, ""), path=_rel(path),
                line=index + 1, deprecated=True, region=region,
                extract_error=error, recipe="census"))
        in_block = block_now
    return found


def _proved_body(lines: list) -> str:
    """The definition out of a proved file, sans header - writeback semantics.

    Mirrors `writeback.read_matched_body`: the header is the leading
    run of `//` lines and blanks, and everything from the first other line is
    the body. Kept local because importing writeback drags in the PE
    and writeback machinery a scanner must not need.
    """
    index = 0
    while index < len(lines) and (not lines[index].strip()
                                  or lines[index].lstrip().startswith("//")):
        index += 1
    return "\n".join(lines[index:]).strip() + "\n"


def scan_tree(root: Path = SRC_ROOT) -> list:
    """Every annotation under `root`, deterministic order."""
    found = []
    for path in sorted(Path(root).rglob("*.cpp")):
        found.extend(scan_file(path))
    found.sort(key=lambda ann: (ann.path, ann.line, ann.address))
    return found


# ----------------------------------------------------------------- resolving


def _precedence(annotation: Annotation) -> tuple:
    """Which claimant owns an address when several annotate it.

    Lower wins. The new grammar beats every legacy spelling; within one
    grammar a proved body (byte-exact, re-verified by every collect) beats
    an inline block; a preserved unit is a RECORD of a measurement, so it
    yields to any live claim on the same address. The second key matters
    once both stores carry explicit markers: the tie is decided by what the
    file IS, not by the spelling it used to carry.
    """
    base = 0 if not annotation.deprecated else 1
    if annotation.recipe == "writeback":
        return (base, 0)
    if annotation.recipe == "census":
        return (base, 1)
    return (base, 2)


def _code_only(region: str) -> str:
    """The region with comments and blank lines removed.

    Two stacked doc comments above one definition extract DIFFERENT raw
    regions - each starts at its own marker - but they are one piece of
    code, and comparing code rather than text is what says so.
    """
    kept, in_block = [], False
    for line in region.splitlines():
        text = line.strip()
        if in_block:
            if "*/" in text:
                in_block = False
                text = text.split("*/", 1)[1].strip()
            else:
                continue
        # Inline comments are cut before comparison: the migrator rewrites
        # them, so an identity check that kept them would flag its own work.
        # A `//` inside a string is cut on BOTH sides of that comparison, so
        # the cut cannot manufacture a difference.
        if "//" in text:
            text = text.split("//", 1)[0].strip()
        if text.startswith("/*"):
            if "*/" not in text:
                in_block = True
            continue
        if not text:
            continue
        kept.append(text)
    return "\n".join(kept)


def resolve(annotations: list) -> tuple:
    """One annotation per address; only GENUINE conflicts survive.

    Two spellings whose regions hold the SAME code are one piece commented
    twice - a shape the tree actually contains (two stacked doc comments
    above one definition) - and collapse to the first silently. Different
    code for one address is resolved by precedence; a tie at the same
    precedence is a conflict the caller must surface, never decide.
    """
    by_address: dict = {}
    for annotation in annotations:
        by_address.setdefault(annotation.address, []).append(annotation)
    resolved, conflicts = [], {}
    for address, group in sorted(by_address.items()):
        if len(group) == 1:
            resolved.append(group[0])
            continue
        if len({_code_only(a.region) for a in group}) == 1:
            resolved.append(group[0])
            continue
        group.sort(key=_precedence)
        best = group[0]
        tied = [a for a in group if _precedence(a) == _precedence(best)]
        if len(tied) > 1:
            conflicts[address] = group
        else:
            resolved.append(best)
    return resolved, conflicts


# ------------------------------------------------------------- cross-reference


def cross_reference(annotations: list, catalog: dict) -> CrossRef:
    """Hold the map against the catalogue; report drift, never fail on it.

    `catalog` is `{int_address: row}` as loaded by
    `emit_translation_unit.load_functions()` (catalogue corrections applied).
    """
    result = CrossRef()
    seen: dict = {}
    for annotation in annotations:
        if annotation.address in seen:
            result.duplicates.setdefault(annotation.address,
                                         [seen[annotation.address]])
            result.duplicates[annotation.address].append(annotation)
            continue
        seen[annotation.address] = annotation
        if annotation.address in catalog:
            result.matched[annotation.address] = annotation
        else:
            result.uncatalogued.append(annotation)
    for address in seen:
        if address in result.duplicates:
            result.matched.pop(address, None)
    result.catalog_only = sorted(set(catalog) - set(seen))
    return result


# ------------------------------------------------------------------- CLI


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("paths", nargs="*", type=Path,
                        help="files to scan (default: all of src/)")
    arguments = parser.parse_args(argv)

    if arguments.paths:
        annotations = []
        for path in arguments.paths:
            annotations.extend(scan_file(path))
    else:
        annotations = scan_tree()

    for annotation in annotations:
        flags = " ".join(filter(None, [
            "deprecated" if annotation.deprecated else "",
            annotation.mode, annotation.state,
            annotation.exclusion, annotation.extract_error]))
        print(f"{annotation.address_hex}  {annotation.location:60s} {flags}")

    states: dict = {}
    for annotation in annotations:
        states[annotation.state] = states.get(annotation.state, 0) + 1
    print(f"\n{len(annotations)} annotation(s): "
          + ", ".join(f"{state} {count}"
                      for state, count in sorted(states.items())))
    deprecated = sum(1 for ann in annotations if ann.deprecated)
    print(f"{deprecated} on deprecated spellings, pending migration")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
