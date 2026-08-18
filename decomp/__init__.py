"""The source map, as a package: what `src/` says about the shipped image.

Two questions, one import. WHERE a piece of the binary is decompiled and
what STATE it is in comes from `reader`, which reads the `ORIGINAL: 0x...`
markers:

    from pathlib import Path
    from decomp import read, write, function_line

    records = read(Path("src"))             # every annotation under src/
    records[0].path                         # absolute Path, ready to open
    line = function_line(Path("src/win.cpp"), "Win::flip")

THE LAYOUT. Each module is one concern:

    model.py             the data structures a scan produces -
                         DecompilationState, Mode, State
    grammar.py           the annotation grammar, as data - every pattern,
                         including the fact-block patterns, because fact
                         blocks are part of the annotation layer the writer
                         protects
    reader.py            the parser - lessons, extraction, stores, the tree
                         walk, the entry points read (a file, a directory,
                         or an in-memory text) and read_file, and
                         function_line (a definition's line, by name)
    writer.py            the annotation writer - write, remove, and their
                         on-disk forms write_file, remove_file
    asm.py               the assembly behind a record - original_asm reads
                         the pinned image, compiled_asm compiles the record's
                         file with VC6 and pulls the function from the object

SELF-CONTAINED, BY REQUIREMENT. Nothing here imports from `tools/`, and
nothing here imports outside the standard library. The scripts in `tools/`
still carry their own copies of the reading machinery; on 2026-08-18 the
marker moved - it carries the piece's name and its image spans - and the
tools copies stayed on the old spelling by decision. `python -m decomp`
proves the package against `src/` alone: the parse and the read -> write ->
read loop. When the tools are refactored onto this package, their copies
are deleted.
"""

from __future__ import annotations

from . import grammar, model, reader, writer
from .model import DecompilationState, Mode, State
from .reader import function_line, read, read_file
from .writer import remove, remove_file, write, write_file

__all__ = [
    "grammar", "model", "reader", "writer",
    "DecompilationState", "Mode", "State",
    "read", "read_file", "function_line",
    "write", "remove", "write_file",
]
