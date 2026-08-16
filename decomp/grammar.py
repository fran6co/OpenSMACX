"""The annotation grammar, as data: every pattern the scanner recognises.

What a marker LOOKS LIKE lives here; what it MEANS lives in `annotation_scan`.
Keeping the two apart is what lets a reader audit the whole grammar in one
screenful, and what lets `python -m decomp` enumerate it exhaustively when it
holds this package's parse against the `tools/` originals.

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
# in `annotation_scan._is_placeholder_region`, because the emitter's own
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
