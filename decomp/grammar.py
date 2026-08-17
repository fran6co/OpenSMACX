"""The annotation grammar: every pattern, and where a marker counts.

What a marker LOOKS LIKE is the patterns below; WHERE IT COUNTS is the
recognition half at the bottom - a marker only counts inside a comment,
and `parse_marker` and `block_state_after` are the ONE implementation of
that rule: the reader parses with them, and nothing re-implements them.
What a marker MEANS - regions, states, lessons - lives in `reader`, and
deciding between records lives in `annotation_scan`. Keeping the grammar
apart is what lets a reader audit it in one screenful, and what lets
`python -m decomp` enumerate it exhaustively when it holds this package's
parse against the `tools/` originals.

A COPY, AND KNOWINGLY SO. `tools/annotation_scan.py` and the reading half of
`tools/project_catalogue.py` hold the same patterns, because the 61 scripts in
`tools/` still import them and this package must not depend on `tools/`.
`python -m decomp` compares the two parses and fails if they stop agreeing -
that check is the whole defence against the copies drifting, so do not delete
it while both exist. **A pattern edit lands in `decomp/grammar.py` AND in the
`tools/` originals** until the tools are refactored onto this package and the
originals are deleted.
"""

from __future__ import annotations

import re

# ------------------------------------------------------------- the one marker

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

# ------------------------------------------------- the deprecated spellings

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

# -------------------------------------------------------- exclusion citations

# The citation token after EXCLUDED is a section reference, not free prose.
# Accept `S1`, `1`, or `§1` so the grammar is ASCII-safe and greppable.
EXCLUSION_TOKEN = re.compile(r"^\s*[§S]?\s*(?P<section>[0-9]+[a-z]?)")

# --------------------------------------------------------- the lesson tokens

LESSON_LEVER = re.compile(r"^\s*(?://|\*)?\s*LEVER:\s*(?P<key>\S+)\s+(?P<prose>.+?)\s*$")
LESSON_RULED_OUT = re.compile(r"^\s*(?://|\*)?\s*RULED-OUT:\s*(?P<prose>.+?)\s*$")
LESSON_CONTINUED = re.compile(r"^\s*(?://|\*)\s{2,}(?P<prose>\S.*?)\s*$")
# The third token, and the only one that BELONGS on a placeholder.
LESSON_UNRECOVERABLE = re.compile(
    r"^\s*(?://|\*)?\s*UNRECOVERABLE:\s*(?P<prose>.+?)\s*$")
# The FOURTH, and it exists because the third was used for something it does
# not mean. See `lessons`.
LESSON_DEFERRED = re.compile(
    r"^\s*(?://|\*)?\s*DEFERRED:\s*(?P<prose>.+?)\s*$")

# ------------------------------------------------------- region boundaries

# A marker owns everything up to the NEXT marker. Matched loosely on purpose:
# the point is only to stop before another piece's text, so any spelling of the
# marker ends the region.
NEXT_MARKER = re.compile(r"^\s*(?://|\*)?\s*ORIGINAL:\s*0x[0-9A-Fa-f]{8}\b")

# The sentinel names the body slot. Its spelling is shared with the wave
# tools (verify_wave.PLACEHOLDER), but what counts as UNTOUCHED is defined
# in `reader._is_placeholder_region`, because the emitter's own
# scaffold breaks verify_wave's rule: a non-void skeleton carries a
# placeholder return after the sentinel so it compiles before a body exists
# (C4716 otherwise), and `is_untouched`'s one-line allowance reads that
# pristine scaffold as worked. The residue of an untouched region is exactly:
# comments, the closing brace, and the placeholder return - anything else is
# a body.
SENTINEL = "// BODY GOES HERE."

# The identifier a mangled name is built around. `?base@Class@@...` is named
# by `base`; `??0Class@@...`, `??1`, `??_G` and friends have no base name of
# their own and are named by the CLASS. A plain `sub_63ffe0` is itself.
_MANGLED_BASE = re.compile(r"^\?\?(?:_[A-Za-z]|[0-9A-Za-z])([A-Za-z_]\w*)@"
                           r"|^\?([A-Za-z_]\w*)@"
                           r"|^([A-Za-z_]\w*)$")
_NAME_FIELD = re.compile(r"^\s*(?://+|\*)?\s*name\s+(\S+)\s*$")

# ----------------------------------------------------- the catalogue facts

# A FACT LINE IS `// key value` WITH THE KEY AT COLUMN 3, exactly as
# `facts` in `tools/project_catalogue.py` writes it. The loose form - any
# indentation, any comment marker, the key anywhere `\s*` could reach - also
# matched RULED-OUT prose, because a
# continuation line is `//            calls need, so this is landed FILE-mode`
# and "calls" is a word people write about a function. `stamped` let the last
# match win, so 0x00402DD0's one call edge became that sentence; 13
# annotations had a fact overwritten by prose this way, 10 of them `calls`.
#
# Measured over all 6,000: tightening this changes four values, every one of
# them from prose back to the fact, and takes no `name` or `spans` away from
# any row. The value is optional so a bare `// prototype`, which is how a row
# with no recorded prototype is spelled once an editor strips the trailing
# space, still registers as present-and-empty rather than absent.
FACT_LINE = re.compile(
    r"^(?://|\*) (name|size|spans|prototype|callers|kind|flags|calls|notes"
    r"|indirect)"
    r"(?: +(.*?))?\s*$")

# A WRAPPED `calls` OR `indirect` VALUE. Long edge lists are re-flowed onto
# continuation lines indented to the value's own column, and reading only the
# first line silently shortened them. Held to two things at once - the column
# the fact's own value starts at, and a value made ENTIRELY of catalogue
# addresses. Either alone admits prose.
CONTINUED = re.compile(r"^(?://|\*)( +)"
                       r"((?:0x[0-9A-Fa-f]{8}(?: \(\d+x\))?(?: |$))+)$")
CONTINUABLE = ("calls", "indirect")

# ------------------------------------------------------------------ inventory

# Every pattern above, named, so a checker can compare the grammar
# EXHAUSTIVELY instead of curating a list. The two copies keep these names in
# different homes: SCAN_PATTERNS live in `tools/annotation_scan.py`,
# CATALOGUE_PATTERNS in `tools/project_catalogue.py`.
SCAN_PATTERNS = (
    "MARKER", "MARKER_KEYWORD", "MARKER_MATCHED",
    "LEGACY_BLOCK", "LEGACY_TRAILING", "LEGACY_OPENING", "LEGACY_PRESERVED",
    "LEGACY_PROVED", "EXCLUSION_TOKEN",
    "LESSON_LEVER", "LESSON_RULED_OUT", "LESSON_CONTINUED",
    "LESSON_UNRECOVERABLE", "LESSON_DEFERRED",
    "NEXT_MARKER", "_MANGLED_BASE", "_NAME_FIELD",
)
CATALOGUE_PATTERNS = ("FACT_LINE", "CONTINUED")

# --------------------------------------------------------------- recognition

# The patterns above are the grammar as DATA; these three functions are the
# grammar APPLIED - the rule for where a marker is valid, shared by every
# consumer. A hit in code or data is prose, not a map entry, and this is the
# ONE implementation of that rule: the reader parses with it, and the writer
# finds existing markers with it, so the two can never disagree about what
# counts as a marker.
def block_state_after(line: str, in_block: bool) -> bool:
    """Cheap block-comment tracking: count the transitions on the line.

    Deliberately naive about strings containing comment marks - a false
    positive there only makes the scanner MORE willing to read a marker, and
    a marker at an address the catalogue does not know is reported as
    uncatalogued rather than believed, so the error cannot fabricate a map
    entry.
    THE SCAN IS BY `find`, NOT BY CHARACTER. This ran once per line of every
    file under `src/`, and every tool in the loop reads the tree at
    startup: the catalogue, the brief, the verifier and the ratchet all pay
    it. Measured 2026-08-14, the character walk was 730,000 calls making 30
    MILLION `startswith` probes - 8.8 s of the 12 s `load_functions` took, and
    the largest single cost in the whole critical path. `find` does the same
    search in C and skips everything between the marks.

    The states are the same three: outside, inside a block, and stopped at a
    line comment. Depth never exceeds one - the character version only ever
    incremented from zero - so `/*` inside a block is text, exactly as before.
    """
    index = 0
    if in_block:
        end = line.find("*/")
        if end < 0:
            return True
        index = end + 2
    while True:
        start = line.find("/*", index)
        if start < 0:
            return False
        line_comment = line.find("//", index)
        if 0 <= line_comment < start:
            return False
        end = line.find("*/", start + 2)
        if end < 0:
            return True
        index = end + 2


def parse_marker(line: str, in_block: bool) -> tuple | None:
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
