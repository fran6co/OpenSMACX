"""The data structures of the source map, apart from the code that parses it.

What a scan PRODUCES lives here: the Annotation record, the cross-reference
summary, and the mode and state vocabulary. The patterns that recognise
markers live in `grammar`; the algorithms that walk, extract and resolve live
in `annotation_scan`. A reader that only wants to know what an annotation IS
imports this module and nothing else.

SELF-CONTAINED, BY REQUIREMENT - standard library only, like the rest of the
package.
"""

from __future__ import annotations

from dataclasses import dataclass, field

# ------------------------------------------------------------- the vocabulary

MODE_BODY = "body"
MODE_FILE = "file"
STATE_IMPLEMENTED = "implemented"
STATE_PLACEHOLDER = "placeholder"
STATE_EXCLUDED = "excluded"


# --------------------------------------------------------------- the records

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
    unrecoverable: tuple = ()  # prose: no C body CAN exist for this piece
    deferred: tuple = ()       # prose: a body can exist, nobody has written it

    @property
    def location(self) -> str:
        return f"{self.path}:{self.line}" if self.line else self.path

    @property
    def address_hex(self) -> str:
        return f"0x{self.address:08X}"


@dataclass
class CrossRef:
    """The scan held against the catalogue. Drift, not failure."""
    matched: dict = field(default_factory=dict)       # address -> Annotation
    duplicates: dict = field(default_factory=dict)    # address -> [Annotation]
    catalog_only: list = field(default_factory=list)  # [address]
    uncatalogued: list = field(default_factory=list)  # [Annotation]
