"""Pure, guarded insertion of source-owned original object annotations.

No file IO, authorization, compiler inference, claim grants or publication.
Callers provide independently reviewed identities and compiled symbol evidence;
this writer checks only annotation-snapshot consistency, safety and round trips.
On-disk freshness must be checked by the caller at guarded publication.
"""

from __future__ import annotations

from dataclasses import asdict
import re
from typing import Mapping, Sequence

from match_recovery.model import Span
from match_recovery.object_source import (ObjectBinding, parse_objects,
                                          validate_object_symbol_ownership)
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
    if binding.parent_symbol is not None:
        _symbol_token(binding.parent_symbol)
        if not binding.member_path or any(not re.fullmatch(r"[A-Za-z_][A-Za-z_0-9]*", part)
                                          for part in binding.member_path):
            raise ValueError("object member path must contain identifier names")
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
    if binding.parent_symbol is not None and binding.span is None:
        text = (f"// RECOVERY_OBJECT: v3 0x{binding.origin.address:08X} {binding.symbol} "
                f"member {binding.parent_symbol} {'.'.join(binding.member_path)}")
    elif binding.parent_symbol is not None:
        assert binding.span is not None and binding.evidence is not None
        text = (f"// RECOVERY_OBJECT: v3 0x{binding.origin.address:08X} "
                f"0x{binding.span.end:08X} {binding.symbol} 0x{binding.evidence.address:08X} "
                f"member {binding.parent_symbol} {'.'.join(binding.member_path)}")
    elif binding.span is None:
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
    wanted = {(row.origin.entity_id, row.symbol, row.span, row.evidence,
               row.parent_symbol, row.member_path)
              for row in tuple(parsed) + tuple(additions)}
    got = {(row.origin.entity_id, row.symbol, row.span, row.evidence,
            row.parent_symbol, row.member_path)
           for row in reread}
    if got != wanted:
        raise ValueError("object annotation round-trip changed its identity")
    return result


def replace_object_records(current: Mapping[str, bytes], *,
                           expected: Sequence[ObjectBinding],
                           replacements: Sequence[ObjectBinding],
                           destination: str, pin: str,
                           address_space: str) -> dict[str, bytes]:
    """Rehome a complete set of existing object identities as member records.

    ``current`` must contain every affected source unit and ``expected`` must
    be the complete parsed object snapshot for those units. Replacements are
    one-for-one with selected old rows, preserve their old identity and any
    bounded initializer obligation exactly, and may change only owner/path and
    source location. All non-replaced records survive byte-for-byte. This is a
    pure transformation; callers remain responsible for native review and
    guarded publication.
    """
    if not current or destination not in current or not replacements:
        raise ValueError("object replacement requires affected units and a destination")
    if any(not isinstance(path, str) or not isinstance(raw, bytes)
           for path, raw in current.items()):
        raise ValueError("object replacement requires source paths and original bytes")
    parsed_by_unit = {}
    lines_by_unit = {}
    newline_by_unit = {}
    boundaries_by_unit = {}
    for path, raw in current.items():
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
        if raw and not lines:
            raise ValueError("source line parsing failed")
        parsed = parse_objects(text, path, pin, address_space=address_space)
        parsed_by_unit[path] = parsed
        lines_by_unit[path] = lines
        newline_by_unit[path] = next((line[len(line.rstrip(b"\r\n")):] for line in lines
                                      if line.endswith((b"\r\n", b"\n", b"\r"))), b"\n")
        block = False
        boundaries = []
        for line in text.splitlines():
            boundaries.append(block)
            block = _block_state_after(line, block)
        boundaries.append(block)
        boundaries_by_unit[path] = boundaries
    observed = tuple(row for path in current for row in parsed_by_unit[path])
    if sorted((row.unit, row.line, asdict(row)) for row in observed) != sorted(
            (row.unit, row.line, asdict(row)) for row in expected):
        raise ValueError("object replacement requires a complete current source snapshot")
    if len({row.unit for row in expected}) != len({row.unit for row in observed}):
        raise ValueError("object replacement snapshot is incomplete")
    _unique(expected)
    for row in expected:
        if row.unit not in current:
            raise ValueError("object replacement omitted an affected source unit")
        _record(row, path=row.unit, pin=pin, address_space=address_space,
                line_limit=len(lines_by_unit[row.unit]))

    by_identity = {row.origin.entity_id: row for row in expected}
    selected = {}
    for row in replacements:
        old = by_identity.get(row.origin.entity_id)
        if old is None or row.origin.entity_id in selected:
            raise ValueError("object replacement identity is missing or duplicated")
        row_interpretation = row.interpretation_id if row.span is not None else None
        old_interpretation = old.interpretation_id if old.span is not None else None
        if (row.symbol, row.origin, row.span, row.evidence, row_interpretation) != (
                old.symbol, old.origin, old.span, old.evidence, old_interpretation):
            raise ValueError("object replacement changed historical identity or initializer obligation")
        if row.parent_symbol is None:
            raise ValueError("object replacement must resolve to a named member owner")
        _record(row, path=destination, pin=pin, address_space=address_space,
                line_limit=len(lines_by_unit[destination]) + 1)
        if boundaries_by_unit[destination][row.line - 1]:
            raise ValueError("object replacement anchor is inside a block comment")
        selected[row.origin.entity_id] = row
    if len(selected) != len(replacements):
        raise ValueError("object replacement identity set is inconsistent")
    selected_old = {identity: by_identity[identity] for identity in selected}
    remaining = [row for row in expected if row.origin.entity_id not in selected]
    _unique(tuple(remaining) + tuple(replacements))
    validate_object_symbol_ownership(tuple(remaining) + tuple(replacements))
    # A moved member parent cannot conflict with a different direct/member
    # binding under the same emitted symbol.
    owner_identity = {}
    for row in tuple(remaining) + tuple(replacements):
        emitted = row.parent_symbol or row.symbol
        # Same-unit direct parent and historical children share one emitted
        # owner. Native object_targets must still prove their original base;
        # this pure writer preserves identities, not compiled layout claims.
        identity = ("owner", row.unit, emitted)
        prior = owner_identity.setdefault(emitted, identity)
        if prior != identity:
            raise ValueError("object replacement conflicts with an emitted owner binding")

    remove_lines = {path: {row.line for row in selected_old.values() if row.unit == path}
                    for path in current}
    insertions = {}
    for row in replacements:
        if (row.line <= len(lines_by_unit[destination])
                and lines_by_unit[destination][row.line - 1].lstrip().startswith(b"// RECOVERY_OBJECT:")
                and row.line not in remove_lines[destination]):
            raise ValueError("object replacement anchor would shadow an existing object marker")
        insertions.setdefault(row.line, []).append(_marker(row))
    output = {}
    for path, raw in current.items():
        lines = lines_by_unit[path]
        newline = newline_by_unit[path]
        if newline not in (b"\n", b"\r", b"\r\n"):
            raise ValueError("unsupported source newline")
        chunks = []
        for number in range(1, len(lines) + 2):
            if path == destination:
                if (number == len(lines) + 1 and insertions.get(number)
                        and raw and not raw.endswith((b"\r", b"\n"))):
                    chunks.append(newline)
                for marker in insertions.get(number, ()):
                    chunks.extend((marker, newline))
            if number <= len(lines) and number not in remove_lines[path]:
                chunks.append(lines[number - 1])
        output[path] = b"".join(chunks)
    reread = tuple(row for path, raw in output.items()
                   for row in parse_objects(raw.decode("utf-8", errors="surrogateescape"),
                                            path, pin, address_space=address_space))
    _unique(reread)
    def continuity(row):
        interpretation_id = row.interpretation_id if row.span is not None else None
        return (row.origin.entity_id, row.symbol, row.span, row.evidence,
                row.parent_symbol, row.member_path, interpretation_id)

    wanted = {continuity(row)
              for row in tuple(remaining) + tuple(replacements)}
    got = {continuity(row) for row in reread}
    if got != wanted:
        raise ValueError("object replacement changed source identity continuity")
    return output
