"""Source-owned original mappings, annotations and measured legacy claims.

Read/write APIs use only the standard library and take caller-owned paths:

    from pathlib import Path
    from match_recovery.source import read, write, function_line

    records = read(Path("src"))

The reader deliberately has no persistent cache. Callers may memoize parsing
with complete input keys; annotations remain authoritative. The writer preserves
unmodified mapping facts and seed notes rather than regenerating their prose.

The optional ``asm`` and ``calls`` modules retain the legacy i386
PE/COFF/MSVC measurement contract and require the package's ``x86`` extra.
They are not imported here. Their masked-byte agreement is not a proof of
relocation targets, ownership or runtime correctness. Strict verification is
provided by the framework's separate binary, generated-table and layout APIs.
The ``record`` module only prepares historical claim transitions in memory;
it neither measures nor writes source and needs no optional dependencies.

No repository root, original image path or compiler flag sets are selected here.
"""

from __future__ import annotations

from . import grammar, mangled, model, reader, writer
from .model import DecompilationState, Mode, Recipe, SourceRole, State, Tier
from .reader import function_line, read, read_file, read_text
from .writer import remove, remove_file, write, write_file, write_kind, write_symbol

__all__ = [
    "grammar", "mangled", "model", "reader", "writer",
    "DecompilationState", "Mode", "Recipe", "SourceRole", "State", "Tier",
    "read", "read_file", "read_text", "function_line",
    "write", "remove", "write_file", "remove_file", "write_symbol", "write_kind",
]
