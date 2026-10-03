"""Pure, guarded insertion of source-owned original object annotations.

No file IO, authorization, compiler inference, claim grants or publication.
Callers provide independently reviewed identities and compiled symbol evidence;
this writer checks only annotation-snapshot consistency, safety and round trips.
On-disk freshness must be checked by the caller at guarded publication.
"""

from __future__ import annotations

from dataclasses import asdict
import re
from typing import Sequence

from match_recovery.model import Span
from match_recovery.object_source import ObjectBinding, parse_objects
from match_recovery.source.reader import _block_state_after
from match_recovery.source.writer import _SYMBOL_SOURCE_TOKENS


def _symbol_token(symbol: str) -> None:
    if (not isinstance(symbol, str) or not symbol
            or any(c.isspace() or ord(c) < 32 or ord(c) == 127 for c in symbol)
            or any(token in symbol for token in ("/*", "*/", "//", "\\", "??/"))):
        raise ValueError("object symbol must be one annotation-safe token")


def _record(binding: ObjectBinding, *, path: str, pin: str,
            address_space: str, line_limit: int) -> None:
    _symbol_token(binding.symbol)
    if binding.unit != path:
        raise ValueError("object binding belongs to another source unit")
    if (binding.origin.image_sha256, binding.origin.address_space) != (pin, address_space):
        raise ValueError("object binding belongs to another image or address space")
    if type(binding.line) is not int or not 1 <= binding.line <= line_limit:
        raise ValueError("object insertion line is outside the source")


def _unique(records: Sequence[ObjectBinding]) -> None:
    if len({row.symbol for row in records}) != len(records):
        raise ValueError("duplicate original object symbol binding")
    if len({row.origin.entity_id for row in records}) != len(records):
        raise ValueError("multiple objects claim the same original start address")
    spans = sorted(row.span for row in records if row.span is not None)
    if any(left.end > right.start for left, right in zip(spans, spans[1:])):
        raise ValueError("original object extents overlap")
    for row in records:
        if any(span.start < row.origin.address < span.end for span in spans):
            raise ValueError("object start lies inside another original object extent")


def _marker(binding: ObjectBinding) -> bytes:
    if binding.span is None:
        text = f"// RECOVERY_OBJECT: v1 0x{binding.origin.address:08X} {binding.symbol}"
    else:
        assert binding.evidence is not None
        text = (f"// RECOVERY_OBJECT: v2 0x{binding.origin.address:08X} "
                f"0x{binding.span.end:08X} {binding.symbol} "
                f"0x{binding.evidence.address:08X}")
    return text.encode("utf-8")


def write_object_bytes(raw: bytes, *, expected: Sequence[ObjectBinding],
                       additions: Sequence[ObjectBinding], path: str, pin: str,
                       address_space: str) -> bytes:
    """Insert object records before caller-selected one-based source lines.

    ``expected`` must be the complete current ``parse_objects`` snapshot for
    this source unit. Each new binding's ``line`` is an insertion anchor: the
    marker is placed immediately before that current one-based line (or at
    EOF as ``line_count + 1``). Existing records are add-only and preserved.
    The returned bytes are reparsed and checked; no file is read or written.
    """
    if not isinstance(raw, bytes):
        raise ValueError("object source rewriting requires original source bytes")
    if not additions:
        raise ValueError("object writer requires at least one addition")

    text = raw.decode("utf-8", errors="surrogateescape")
    if re.search(r"[\v\f\x1c-\x1e\x85\u2028\u2029]", text):
        raise ValueError("unsupported source line separator")
    normalized = text.replace("\r\n", "\n").replace("\r", "\n")
    if re.search(r"(?:\\|\?\?/)[ \t]*\n", normalized):
        raise ValueError("object writer does not support line-spliced source")
    if raw.startswith(b"\xef\xbb\xbf"):
        raise ValueError("object writer does not support BOM-prefixed source")
    if any(re.fullmatch(r'(?:u8|u|U|L)?R"', token.group())
           for token in _SYMBOL_SOURCE_TOKENS.finditer(normalized)):
        raise ValueError("object writer does not support raw-string source")
    lines = raw.splitlines(keepends=True)
    line_count = len(lines)
    parsed = parse_objects(text, path, pin, address_space=address_space)
    if tuple(asdict(row) for row in parsed) != tuple(asdict(row) for row in expected):
        raise ValueError("object update requires a current complete source snapshot")
    _unique(parsed)
    for binding in parsed:
        _record(binding, path=path, pin=pin, address_space=address_space,
                line_limit=line_count)
    block = False
    boundaries = []
    for line in text.splitlines():
        boundaries.append(block)
        block = _block_state_after(line, block)
    boundaries.append(block)

    for binding in additions:
        _record(binding, path=path, pin=pin, address_space=address_space,
                line_limit=line_count + 1)
        if boundaries[binding.line - 1]:
            raise ValueError("object insertion anchor is inside a block comment")
        if any(row.origin.entity_id == binding.origin.entity_id or
               row.symbol == binding.symbol for row in parsed):
            raise ValueError("new object conflicts with an existing object")
    _unique(tuple(parsed) + tuple(additions))

    if any(binding.line <= line_count and
           lines[binding.line - 1].lstrip().startswith(b"// RECOVERY_OBJECT:")
           for binding in additions):
        raise ValueError("object insertion would shadow an existing object marker")

    newline = next((line[len(line.rstrip(b"\r\n")):] for line in lines
                    if line.endswith((b"\r\n", b"\n", b"\r"))), b"\n")
    if newline not in (b"\n", b"\r", b"\r\n"):
        raise ValueError("unsupported source newline")

    inserted_by_line: dict[int, list[bytes]] = {}
    for binding in additions:
        inserted_by_line.setdefault(binding.line, []).append(_marker(binding) + newline)

    output = []
    for line_number in range(1, line_count + 2):
        if line_number == line_count + 1 and inserted_by_line.get(line_number) and raw and not raw.endswith((b"\r", b"\n")):
            output.append(newline)
        output.extend(inserted_by_line.get(line_number, ()))
        if line_number <= line_count:
            output.append(lines[line_number - 1])
    result = b"".join(output)
    reread = parse_objects(result.decode("utf-8", errors="surrogateescape"),
                           path, pin, address_space=address_space)
    _unique(reread)
    wanted = {(row.origin.entity_id, row.symbol, row.span, row.evidence)
              for row in tuple(parsed) + tuple(additions)}
    got = {(row.origin.entity_id, row.symbol, row.span, row.evidence)
           for row in reread}
    if got != wanted:
        raise ValueError("object annotation round-trip changed its identity")
    return result
