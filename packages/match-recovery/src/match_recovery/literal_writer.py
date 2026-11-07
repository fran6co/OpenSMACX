"""Pure, guarded insertion of source-owned literal annotations.

The writer adds reviewed bindings to a complete current source snapshot. It
does not infer symbols, alter literal contents or publish files.
"""

from __future__ import annotations

from dataclasses import asdict
import re
from typing import Sequence

from match_recovery.components import relative_path
from match_recovery.model import Origin, Span, require_digest, require_text
from match_recovery.literal_source import (LiteralBinding, parse_literals,
                                           validate_literals)
from match_recovery.source.annotation_writer import insert_annotation_lines


_SAFE_COMPILER_SYMBOL = re.compile(r"[A-Za-z_?$@.][A-Za-z0-9_?$@.]*\Z", re.ASCII)
_PREFIX = b"// RECOVERY_LITERAL:"


def _symbol_token(symbol: str) -> None:
    if (not isinstance(symbol, str) or not symbol.isascii()
            or _SAFE_COMPILER_SYMBOL.fullmatch(symbol) is None):
        raise ValueError("literal symbol must be one ASCII compiler-symbol token")


def _record(binding: LiteralBinding, *, path: str, pin: str,
            address_space: str, line_limit: int) -> None:
    if type(binding) is not LiteralBinding:
        raise ValueError("literal writer requires typed bindings")
    _symbol_token(binding.symbol)
    if (type(binding.origin) is not Origin or type(binding.span) is not Span
            or type(binding.evidence) is not Origin):
        raise ValueError("literal binding requires native origin, span and evidence values")
    # Re-run frozen model validation at the write boundary.
    origin = Origin(binding.origin.image_sha256, binding.origin.address_space,
                    binding.origin.address)
    span = Span(binding.span.start, binding.span.end)
    evidence = Origin(binding.evidence.image_sha256,
                      binding.evidence.address_space, binding.evidence.address)
    LiteralBinding(origin, span, binding.symbol,
                   binding.unit, binding.line, evidence, binding.merged)
    if binding.unit != path:
        raise ValueError("literal binding belongs to another source unit")
    if ((binding.origin.image_sha256, binding.origin.address_space)
            != (pin, address_space)):
        raise ValueError("literal binding belongs to another original")
    if type(binding.line) is not int or not 1 <= binding.line <= line_limit:
        raise ValueError("literal insertion line is outside the source")


def _identity(binding: LiteralBinding) -> tuple:
    return (binding.origin, binding.span, binding.symbol, binding.unit,
            binding.evidence, binding.merged)


def _marker(binding: LiteralBinding) -> bytes:
    return (
        ("// RECOVERY_LITERAL: v2 merged " if binding.merged else "// RECOVERY_LITERAL: v1 ")
        + f"0x{binding.span.start:08X} 0x{binding.span.end:08X} "
        f"{binding.symbol} 0x{binding.evidence.address:08X}"
    ).encode("ascii")


def write_literal_bytes(raw: bytes, *, expected: Sequence[LiteralBinding],
                        additions: Sequence[LiteralBinding], path: str,
                        pin: str, address_space: str) -> bytes:
    """Insert literal records before selected one-based source lines.

    ``expected`` must be the complete ``parse_literals`` snapshot for ``path``.
    The operation is add-only and returns bytes; freshness and guarded disk
    publication remain the caller's responsibility.
    """
    if type(raw) is not bytes:
        raise ValueError("literal source rewriting requires original bytes")
    if not additions:
        raise ValueError("literal writer requires at least one addition")
    relative_path(path)
    require_digest(pin)
    require_text(address_space, "address space")

    text = raw.decode("utf-8", errors="surrogateescape")
    line_count = len(raw.splitlines(keepends=True))
    parsed = parse_literals(text, path, pin, address_space=address_space)
    if tuple(asdict(row) for row in parsed) != tuple(asdict(row) for row in expected):
        raise ValueError("literal update requires a current complete source snapshot")
    expected = validate_literals(parsed)
    for row in expected:
        _record(row, path=path, pin=pin, address_space=address_space,
                line_limit=line_count)
    for row in additions:
        _record(row, path=path, pin=pin, address_space=address_space,
                line_limit=line_count + 1)

    existing_ids = {_identity(row) for row in expected}
    addition_ids = {_identity(row) for row in additions}
    if len(addition_ids) != len(additions) or existing_ids & addition_ids:
        raise ValueError("new literal annotation conflicts with an existing identity")
    all_rows = validate_literals((*expected, *additions))

    result = insert_annotation_lines(
        raw, insertions=tuple((row.line, _marker(row)) for row in additions),
        marker_prefix=_PREFIX, label="literal")
    reread = parse_literals(result.decode("utf-8", errors="surrogateescape"),
                            path, pin, address_space=address_space)
    wanted = existing_ids | addition_ids
    if {_identity(row) for row in reread} != wanted:
        raise ValueError("literal annotation round-trip changed its identity")
    if len(reread) != len(all_rows):
        raise ValueError("literal annotation round-trip changed its population")
    return result
