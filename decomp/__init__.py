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
    mangled.py           reading an MSVC mangled name far enough to tell
                         two overloads apart - `arity`, and no more
    asm.py               the assembly behind a record, and the verdict -
                         original_asm reads the pinned image, compiled_asm
                         compiles the record's file with VC6 and pulls the
                         function from the object, compare_record asks every
                         flag set and keeps the best answer
    record.py            what a measurement does to an annotation - stamped
                         says what it should become, record_match writes it

`asm` and `record` are NOT imported here: they need capstone, and reading
an annotation must not.

SELF-CONTAINED, BY REQUIREMENT. Nothing here imports from `tools/`.

STANDARD LIBRARY TO READ, CAPSTONE TO COMPARE. `model`, `grammar`, `reader`
and `writer` import nothing that has to be installed, so `import decomp`
and everything above this line costs only Python; `asm` disassembles both
sides of a byte match and needs capstone, which is why it is not imported
here and why it is a declared dependency rather than a hopeful lazy import.
`decomp/tests/test_roundtrip.py` CHECKS that split rather than restating it - see
`layering` - because the package carried the broader claim in prose for as
long as it was already false. The scripts in `tools/`
still carry their own copies of the reading machinery; on 2026-08-18 the
marker moved - it carries the piece's name and its image spans - and the
tools copies stayed on the old spelling by decision. The test suite
proves the package against `src/` alone: the parse and the read -> write ->
read loop. When the tools are refactored onto this package, their copies
are deleted.
"""

from __future__ import annotations

from . import grammar, mangled, model, reader, writer
from .model import DecompilationState, Mode, Recipe, State, Tier
from .reader import function_line, read, read_file, read_text
from .writer import remove, remove_file, write, write_file

__all__ = [
    "grammar", "mangled", "model", "reader", "writer",
    "DecompilationState", "Mode", "Recipe", "State", "Tier",
    "read", "read_file", "read_text", "function_line",
    "write", "remove", "write_file", "remove_file",
]
