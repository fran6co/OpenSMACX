"""The source map, as a package: what `src/` says about the shipped image.

Two questions, one import. WHERE a piece of the binary is decompiled and what
STATE it is in comes from `annotation_scan`, which reads the `ORIGINAL: 0x...`
markers. WHAT that piece is - its name, size, spans, prototype, call edges -
comes from `project_catalogue.from_source`, which reads the `// key value`
fact block stamped under each marker:

    from decomp import from_source, scan_tree, resolve

    rows = from_source()                      # {address: row}
    rows[0x005D7210]["name"]                  # '??0Buffer@@QAE@XZ'
    records, duplicates = resolve(scan_tree())
    records[0].path                           # absolute Path, ready to open
    records[0].state is State.IMPLEMENTED     # typed vocabulary, not strings

THE LAYOUT. Each module is one concern:

    model.py             the data structures a scan produces -
                         DecompilationState, Mode, State
    grammar.py           the annotation grammar as data - every pattern the
                         scanner recognises, nothing else
    annotation_scan.py   the engine - marker recognition, extraction,
                         resolution, cross-reference
    reader.py            the reading entry points - read_file, read, scan_tree
    writer.py            the annotation writer - write, write_file
    project_catalogue.py the catalogue reader - the facts stamped under markers

SELF-CONTAINED, BY REQUIREMENT. Nothing here imports from `tools/`, and
nothing here imports outside the standard library. The grammar and the engine
are COPIES of `tools/annotation_scan.py` and the reading half of
`tools/project_catalogue.py`, which the 61 scripts in `tools/` still import
directly.

That means two parsers for one grammar, which is a real cost and is paid on
purpose: the package owes nothing to the scripts and can be lifted out whole.
The cost is bounded by `python -m decomp`, which parses `src/` and proves the
result directly against the tree, then holds its parse against the `tools/`
originals and fails if the answers diverge - so a grammar edit landing in one
copy and not the other is caught by the next run rather than by a wrong
number months later. When the tools are refactored onto this package,
`tools/annotation_scan.py` and the reader in `tools/project_catalogue.py` are
deleted and that comparison goes with them.
"""

from __future__ import annotations

from . import (annotation_scan, grammar, model, project_catalogue, reader,
               writer)
from .annotation_scan import cross_reference, resolve
from .grammar import FACT_LINE
from .model import DecompilationState, Mode, State
from .project_catalogue import from_source, stamped
from .reader import read, read_file, scan_tree, tree_stamp
from .writer import write, write_file

__all__ = [
    "annotation_scan", "grammar", "model", "project_catalogue", "reader",
    "writer",
    "DecompilationState", "Mode", "State",
    "read", "read_file", "scan_tree", "tree_stamp", "resolve",
    "cross_reference", "write", "write_file",
    "from_source", "stamped", "FACT_LINE",
]
