"""The annotation grammar, as data: every pattern the reader recognises.

What a marker LOOKS LIKE lives here; what it MEANS - regions, states,
lessons - lives in `reader`, and deciding between records lives in
`annotation_scan`. Keeping the grammar apart is what lets a reader audit
it in one screenful.

`tools/annotation_scan.py` and the reading half of
`tools/project_catalogue.py` still carry their own copies of these
patterns for the 61 scripts in `tools/`. On 2026-08-18 the marker moved -
it carries the piece's name and its image spans - and the tools copies
stayed on the old spelling by decision; the package proves itself against
`src/` alone from then on.
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
# The tail's structure: the marker names the piece before saying how to
# read it - mangled name, then the image spans, then the keywords.
MARKER_TAIL = re.compile(
    r"^\s*(?P<name>\S+)\s+"
    r"(?P<spans>0x[0-9A-Fa-f]{6,8}-0x[0-9A-Fa-f]{6,8}"
    r"(?:;0x[0-9A-Fa-f]{6,8}-0x[0-9A-Fa-f]{6,8})*)"
    r"\s*(?P<rest>.*)$", re.S)
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
# THE SECOND RATCHET CLAIM, and it is a claim for exactly the same reason:
# `SEMANTIC` means "this was proved to compile to the same INSTRUCTIONS as the
# shipped bytes, differing only in register allocation or scheduling; fail if
# it stops". It is re-proved by every run like BYTE_EXACT, so it is not the
# stale status field this grammar refuses to carry - and it is counted apart
# from BYTE_EXACT, so it can never inflate the ratchet it sits beside.
MARKER_SEMANTIC = re.compile(r"(?<![A-Za-z0-9_])SEMANTIC(?![A-Za-z0-9_])")

# -------------------------------------------------------- exclusion citations

# The citation token after EXCLUDED is a section reference, not free prose.
# Accept `S1`, `1`, or `§1` so the grammar is ASCII-safe and greppable.
EXCLUSION_TOKEN = re.compile(r"^\s*[§S]?\s*(?P<section>[0-9]+[a-z]?)")

# --------------------------------------------------------- the lesson tokens

# THE PARENTHETICAL IS OPTIONAL AND IS PART OF THE TOKEN. People write
# `RULED-OUT (still open):`, `LEVER (partial):`, `LEVER (2026-08-21):` - a
# qualifier before the colon - and without `(?:\([^)]*\))?` here the line
# matches NOTHING and the lesson is invisible: the body reads as untouched and
# the next pass re-derives it. 15 such lines were sitting in five files when
# this was found. The qualifier is captured into the prose so nothing is lost
# when a lesson is rewritten.
LESSON_LEVER = re.compile(
    r"^\s*(?://|\*)?\s*LEVER\s*(?P<qualifier>\([^)]*\))?:\s*"
    r"(?P<key>\S+)\s+(?P<prose>.+?)\s*$")
LESSON_RULED_OUT = re.compile(
    r"^\s*(?://|\*)?\s*RULED-OUT\s*(?P<qualifier>\([^)]*\))?:\s*"
    r"(?P<prose>.+?)\s*$")
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

# The identifier a C++ DEFINITION HEAD declares: the last name before a
# parameter list. Used to find what this tree calls a piece when that is not
# what the image calls it - see `reader.region_identifiers`.
DEFINITION_HEAD = re.compile(
    r"(?:^|[\s*&:~])(?P<identifier>[A-Za-z_]\w*)\s*\(", re.M)
# Words that open a parenthesis without declaring anything.
DEFINITION_KEYWORDS = frozenset((
    "if", "for", "while", "switch", "return", "sizeof", "catch", "throw",
    "do", "else", "case", "new", "delete", "and", "or", "not"))

# The identifier a mangled name is built around. `?base@Class@@...` is named
# by `base`; `??0Class@@...`, `??1`, `??_G` and friends have no base name of
# their own and are named by the CLASS. A plain `sub_63ffe0` is itself.
_MANGLED_BASE = re.compile(r"^\?\?(?:_[A-Za-z]|[0-9A-Za-z])([A-Za-z_]\w*)@"
                           r"|^\?([A-Za-z_]\w*)@"
                           r"|^([A-Za-z_]\w*)$")

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
# `symbol` joined the keys when the measurement needed a fact the marker does
# not carry: what the COMPILER emits for this piece, which is not always what
# the image calls it. See `model.DecompilationState.symbol`. `body` joined
# them for the 199 pieces DEFINED IN A HEADER - constructors and destructors
# written in-class so the image's own inlining reproduces - whose marker has
# to sit in a `.cpp` because that is what a reader globs and a compiler
# compiles. Without it the claim points at a file with no body in it.
FACT_LINE = re.compile(
    r"^(?://|\*) (name|size|spans|symbol|body|prototype|callers|kind|flags"
    r"|calls|notes|indirect)"
    r"(?: +(.*?))?\s*$")

# A WRAPPED `calls` OR `indirect` VALUE. Long edge lists are re-flowed onto
# continuation lines indented to the value's own column, and reading only the
# first line silently shortened them. Held to two things at once - the column
# the fact's own value starts at, and a value made ENTIRELY of catalogue
# addresses. Either alone admits prose.
CONTINUED = re.compile(r"^(?://|\*)( +)"
                       r"((?:0x[0-9A-Fa-f]{8}(?: \(\d+x\))?(?: |$))+)$")
CONTINUABLE = ("calls", "indirect")
