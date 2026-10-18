"""Pure, guarded insertion of associated-data source annotations.

This module rewrites only caller-provided bytes in memory. It does not read or
write files, grant claims, authorize interpretations or publish changes.
"""

from __future__ import annotations

from dataclasses import asdict
import re
from typing import Sequence

from match_recovery.components import relative_path
from match_recovery.model import require_digest, require_text
from match_recovery.source.reader import _block_state_after
from match_recovery.source.writer import _SYMBOL_SOURCE_TOKENS
from .associated_source import (AssociatedDataBinding, parse_associated,
                                validate_associated)


def _marker(binding: AssociatedDataBinding) -> bytes:
    return (
        "// RECOVERY_ASSOCIATED: v1 "
        f"{binding.kind} 0x{binding.owner.address:08X} "
        f"0x{binding.span.start:08X} 0x{binding.span.end:08X} "
        f"0x{binding.evidence.address:08X}"
    ).encode("ascii")


def _record(binding: AssociatedDataBinding, *, path: str, pin: str,
            address_space: str, line_limit: int) -> None:
    if type(binding) is not AssociatedDataBinding:
        raise ValueError("associated writer requires typed bindings")
    # Re-run the frozen model checks at the writing boundary.
    AssociatedDataBinding(binding.owner, binding.span, binding.kind,
                          binding.evidence, binding.unit, binding.line)
    if binding.unit != path:
        raise ValueError("associated binding belongs to another source unit")
    if ((binding.owner.image_sha256, binding.owner.address_space)
            != (pin, address_space)):
        raise ValueError("associated binding belongs to another original")
    if type(binding.line) is not int or not 1 <= binding.line <= line_limit:
        raise ValueError("associated insertion line is outside the source")


def _identity(binding: AssociatedDataBinding) -> tuple:
    return (binding.owner, binding.span, binding.kind, binding.evidence,
            binding.unit)


def write_associated_bytes(raw: bytes, *,
                           expected: Sequence[AssociatedDataBinding],
                           additions: Sequence[AssociatedDataBinding],
                           path: str, pin: str,
                           address_space: str) -> bytes:
    """Insert reviewed annotations before selected one-based source lines.

    ``expected`` is the complete parsed snapshot for ``path``. The operation
    is add-only and returns bytes; callers retain responsibility for guarded
    on-disk freshness and publication.
    """
    if type(raw) is not bytes:
        raise ValueError("associated source rewriting requires original bytes")
    if not additions:
        raise ValueError("associated writer requires at least one addition")
    relative_path(path)
    require_digest(pin)
    require_text(address_space, "address space")

    text = raw.decode("utf-8", errors="surrogateescape")
    if re.search(r"[\v\f\x1c-\x1e\x85\u2028\u2029]", text):
        raise ValueError("unsupported source line separator")
    normalized = text.replace("\r\n", "\n").replace("\r", "\n")
    splice = re.search(r"(?:\\|\?\?/)[ \t]*\n", normalized)
    first_splice_line = (normalized.count("\n", 0, splice.start()) + 1
                         if splice else None)
    if raw.startswith(b"\xef\xbb\xbf"):
        raise ValueError("associated writer does not support BOM-prefixed source")
    if any(re.fullmatch(r'(?:u8|u|U|L)?R"', token.group())
           for token in _SYMBOL_SOURCE_TOKENS.finditer(normalized)):
        raise ValueError("associated writer does not support raw-string source")

    lines = raw.splitlines(keepends=True)
    line_count = len(lines)
    parsed = parse_associated(text, path, pin, address_space=address_space)
    if tuple(asdict(row) for row in parsed) != tuple(asdict(row) for row in expected):
        raise ValueError("associated update requires a current complete source snapshot")
    expected = validate_associated(parsed)
    for row in expected:
        _record(row, path=path, pin=pin, address_space=address_space,
                line_limit=line_count)
    for row in additions:
        _record(row, path=path, pin=pin, address_space=address_space,
                line_limit=line_count + 1)
        if first_splice_line is not None and row.line >= first_splice_line:
            raise ValueError("associated writer does not support insertion in line-spliced source")

    all_rows = validate_associated((*expected, *additions))
    existing_ids = {_identity(row) for row in expected}
    addition_ids = {_identity(row) for row in additions}
    if len(addition_ids) != len(additions) or existing_ids & addition_ids:
        raise ValueError("new associated annotation conflicts with an existing identity")

    block = False
    boundaries = []
    for line in text.splitlines():
        boundaries.append(block)
        block = _block_state_after(line, block)
    boundaries.append(block)
    for row in additions:
        if boundaries[row.line - 1]:
            raise ValueError("associated insertion anchor is inside a block comment")
        if (row.line <= line_count
                and lines[row.line - 1].lstrip().startswith(
                    b"// RECOVERY_ASSOCIATED:")):
            raise ValueError("associated insertion would shadow an existing annotation")

    newline = next((line[len(line.rstrip(b"\r\n")):] for line in lines
                    if line.endswith((b"\r\n", b"\n", b"\r"))), b"\n")
    if newline not in (b"\n", b"\r", b"\r\n"):
        raise ValueError("unsupported source newline")
    inserted: dict[int, list[bytes]] = {}
    for row in additions:
        inserted.setdefault(row.line, []).append(_marker(row) + newline)

    output = []
    for line_number in range(1, line_count + 2):
        if (line_number == line_count + 1 and inserted.get(line_number)
                and raw and not raw.endswith((b"\r", b"\n"))):
            output.append(newline)
        output.extend(inserted.get(line_number, ()))
        if line_number <= line_count:
            output.append(lines[line_number - 1])
    result = b"".join(output)

    reread = parse_associated(result.decode("utf-8", errors="surrogateescape"),
                              path, pin, address_space=address_space)
    wanted = existing_ids | addition_ids
    if {_identity(row) for row in reread} != wanted:
        raise ValueError("associated annotation round-trip changed its identity")
    if len(reread) != len(all_rows):
        raise ValueError("associated annotation round-trip changed its population")
    return result
