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
    annotation claims, the name and spans the fact block records, and the
    lesson vocabulary beside it. No parsing metadata leaks into the record -
    how the marker was spelled is the parser's concern, not the result's.
    """
    address: int
    mode: Mode
    state: State
    path: Path                   # absolute location of the declaring file
    line: int                    # 1-based line of the marker (0 for filename-derived)
    name: str = ""               # the fact block's mangled name
    spans: tuple = ()            # the fact block's byte spans, (low, high)
                                 # pairs; the first is the body, any further
                                 # span is cold code the image lays elsewhere
    exclusion: str = ""          # EXCLUSIONS.md citation for State.EXCLUDED
    region: str = ""             # the code this annotation claims ("" on error)
    extract_error: str = ""      # why the region could not be cut
    byte_exact: bool = False     # carries a BYTE_EXACT ratchet claim: this
                                 # piece was proved to recompile to the shipped
                                 # bytes and must not stop. NOT a state - the
                                 # claim lives beside the body it constrains
                                 # and is re-proved by every ratchet run,
                                 # while states are derived from the region.
    recipe: str = "census"       # how the status tool must build the unit:
                                 # "census" (extract + scaffolding), "writeback"
                                 # (proved bodies, declfix recipe) or "verbatim"
                                 # (FILE mode: compile the file as-is)
    levers: tuple = ()           # (fingerprint, prose) that MADE this match
    ruled_out: tuple = ()        # spellings tried on this body that did not
    unrecoverable: tuple = ()    # prose: no C body CAN exist for this piece
    deferred: tuple = ()         # prose: a body can exist, nobody has written it

    @property
    def location(self) -> str:
        return f"{self.path}:{self.line}" if self.line else str(self.path)

    @property
    def address_hex(self) -> str:
        return f"0x{self.address:08X}"
