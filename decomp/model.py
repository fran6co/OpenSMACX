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
    """How far one body reproduces another, best first.

    THE LAST TWO ARE NOT FAILURES, they are refusals: a verdict is not
    defined for those spans at all, and summing them into a denominator with
    the misses is what stops a reader telling a wall from a body nobody has
    written yet.
    """
    BYTE_EXACT = "BYTE_EXACT"        # every compared byte agrees
    SHAPE_EXACT = "SHAPE_EXACT"      # same instructions, same registers, same
                                     # addressing - a CONSTANT is wrong. The
                                     # most actionable miss there is: a field
                                     # offset, a vtable slot, a loop bound.
    MNEMONIC_ONLY = "MNEMONIC_ONLY"  # same instruction sequence, different
                                     # registers or addressing form
    MISMATCH = "MISMATCH"            # different instructions
    NO_COMPILE = "NO_COMPILE"        # the body did not build, so there is
                                     # nothing to compare. A TIER and not an
                                     # exception: getting a body from here to
                                     # MISMATCH is the most valuable single
                                     # move in a recovery pass, so a loop has
                                     # to be able to rank it.
    SHARED_TAIL = "SHARED_TAIL"      # `/Gy` folded this span onto another
                                     # function's; it belongs to no one body
    REFUSED = "REFUSED"              # nothing here can be scored - a
                                     # self-modifying span, or a record with
                                     # no primary span at all

    @property
    def rank(self) -> int:
        """Position in the ladder; lower is better."""
        return _TIER_ORDER.index(self)

    @property
    def scoreable(self) -> bool:
        """False where a verdict is not defined, rather than not reached."""
        return self not in (Tier.SHARED_TAIL, Tier.REFUSED)


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
    line: int                    # 1-based line of the marker
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
    kind: str = ""               # what the catalogue calls the piece:
                                 # `game` (5,575 of them), `library` (331 -
                                 # the statically linked CRT and zlib) or
                                 # `thunk` (35). The only fact that separates
                                 # Alpha Centauri's own code from what the
                                 # linker brought in, which is what a call
                                 # graph has to know to be worth reading.
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

    def __post_init__(self) -> None:
        # 1-BASED, AND CHECKED HERE. `writer` used to reject `line == 0` at
        # both of its entry points, describing it as a "filename-derived"
        # record - a kind nothing has ever produced. What the check was
        # really defending was indexing: `lines[record.line - 1]` on a zero
        # silently addresses the LAST line of the file. That is an invariant
        # of the field, so it belongs to the field, where it holds for every
        # consumer rather than for the two that remembered to ask.
        if self.line < 1:
            raise ValueError(
                f"0x{self.address:08X}: line {self.line} is not a 1-based "
                f"line number")

    @property
    def location(self) -> str:
        return f"{self.path}:{self.line}" if self.line else str(self.path)

    @property
    def address_hex(self) -> str:
        return f"0x{self.address:08X}"

    @property
    def size(self) -> int:
        """Bytes of the piece's PRIMARY span.

        Derived, not a fact: the marker already carries the spans, and the
        `// size` fact beside it is the catalogue's total ACROSS spans -
        which is a different number for the 417 records that have more than
        one, and the wrong one to compare an object against. Everything that
        ranks or reports wants this, so it is computed once here rather than
        spelled out at every call site.
        """
        if not self.image_spans:
            return 0
        low, high = self.image_spans[0]
        return high - low


# Declared after `Tier` so the members exist to be indexed.
_TIER_ORDER = (Tier.BYTE_EXACT, Tier.SHAPE_EXACT, Tier.MNEMONIC_ONLY,
               Tier.MISMATCH, Tier.NO_COMPILE, Tier.SHARED_TAIL,
               Tier.REFUSED)
