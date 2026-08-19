"""The data structures the source map produces, apart from the parsing code.

What a scan PRODUCES lives here: the DecompilationState record and the mode
and state vocabulary. The patterns that recognise markers live in `grammar`;
the algorithms that walk, extract and resolve live in `annotation_scan`. A
reader that only wants to know what a decompilation record IS imports this
module and nothing else.

SELF-CONTAINED, BY REQUIREMENT - standard library only, like the rest of the
package.
"""

from __future__ import annotations

from dataclasses import dataclass
from enum import StrEnum
from pathlib import Path


class Mode(StrEnum):
    """How an annotation claims its piece of the binary."""
    BODY = "body"   # the definition after the marker is the decompiled body
    FILE = "file"   # the WHOLE file is the translation unit, compiled as-is


class Recipe(StrEnum):
    """How a scoreable translation unit is built for this piece.

    Derived from where the record lives, not declared: see `reader`. It is a
    vocabulary rather than a free string because only these three exist and a
    typo in a fourth would read as a recipe nobody implements.
    """
    CENSUS = "census"        # extract the body and wrap it in scaffolding
    WRITEBACK = "writeback"  # a proved body in the store, declfix recipe
    VERBATIM = "verbatim"    # FILE mode: the file already IS the unit


class Tier(StrEnum):
    """How far one assembly listing reproduces another, best first.

    The ladder this package measures, and no more of it: `tools/byte_match.py`
    also reports SHAPE_EXACT, SHARED_TAIL and REFUSED, which need span
    classification and an operand-level comparison that live there. A verdict
    named here is one `compare_asm` can actually reach.
    """
    BYTE_EXACT = "BYTE_EXACT"        # every compared byte agrees
    MNEMONIC_ONLY = "MNEMONIC_ONLY"  # same instructions, different constants
    MISMATCH = "MISMATCH"            # different instructions

    @property
    def rank(self) -> int:
        """Position in the ladder; lower is better."""
        return _TIER_ORDER.index(self)


class State(StrEnum):
    """What condition the claimed piece is in.

    MEASURED, NOT CLAIMED. The only declared state is EXCLUDED, because
    exclusion is a decision; the others are derived from the region itself -
    see `reader._state_of`.
    """
    IMPLEMENTED = "implemented"
    PLACEHOLDER = "placeholder"
    EXCLUDED = "excluded"


@dataclass
class DecompilationState:
    """One mapped piece of the binary, ready to work with after parsing.

    Everything a consumer needs to act on the piece directly: an absolute
    `path` it can open, typed `mode` and `state`, the `region` of source the
    annotation claims, and the lesson vocabulary beside it. `name` and
    `image_spans` are the piece's identity and extent in the image; the reader
    fills them from the fact block, and a fresh annotation must supply them
    - an annotation that cannot say what it names and where it ends points
    at nothing verifiable. No parsing metadata leaks into the record - how
    the marker was spelled is the parser's concern, not the result's.
    """
    address: int
    mode: Mode
    state: State
    path: Path                   # absolute location of the declaring file
    line: int                    # 1-based line of the marker (0 for filename-derived)
    name: str                    # the mangled name ("" until a fact block
                                 # records it; required to ADD an annotation)
    image_spans: tuple[tuple[int, int], ...]
                                 # the byte spans, (low, high) pairs; the
                                 # first is the body, any further span is
                                 # cold code the image lays elsewhere
    symbol: str = ""             # the symbol THIS TREE emits for the piece,
                                 # when it is not `name`. `name` is what the
                                 # IMAGE calls it and stays the catalogue's
                                 # key; a redirect shim carries a symbol of
                                 # its own, so the two are different facts and
                                 # neither replaces the other. Empty means
                                 # "the compiler emits `name`".
    exclusion: str = ""          # EXCLUSIONS.md citation for State.EXCLUDED
    region: str = ""             # the code this annotation claims ("" on error)
    extract_error: str = ""      # why the region could not be cut
    byte_exact: bool = False     # carries a BYTE_EXACT ratchet claim: this
                                 # piece was proved to recompile to the shipped
                                 # bytes and must not stop. NOT a state - the
                                 # claim lives beside the body it constrains
                                 # and is re-proved by every ratchet run,
                                 # while states are derived from the region.
    recipe: Recipe = Recipe.CENSUS
                                 # how a scoreable unit is built - see Recipe
    levers: tuple[tuple[str, str], ...] = ()
                                 # (fingerprint, prose) that MADE this match
    ruled_out: tuple[str, ...] = ()
                                 # spellings tried on this body that did not
    unrecoverable: tuple[str, ...] = ()
                                 # prose: no C body CAN exist for this piece
    deferred: tuple[str, ...] = ()
                                 # prose: a body can exist, nobody has written it

    @property
    def location(self) -> str:
        return f"{self.path}:{self.line}" if self.line else str(self.path)

    @property
    def address_hex(self) -> str:
        return f"0x{self.address:08X}"


# Declared after `Tier` so the members exist to be indexed.
_TIER_ORDER = (Tier.BYTE_EXACT, Tier.MNEMONIC_ONLY, Tier.MISMATCH)
