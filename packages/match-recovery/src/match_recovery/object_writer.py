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
                                          array_slice_bounds,
                                          valid_member_path,
                                          validate_object_extent_ownership,
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
    if type(binding.target_local) is not bool:
        raise ValueError("object target scope must be a boolean")
    _symbol_token(binding.symbol)
    if binding.parent_symbol is not None:
        _symbol_token(binding.parent_symbol)
        if not valid_member_path(binding.member_path):
            raise ValueError("object member path must contain names and canonical fixed indices")
    if binding.target_symbol is not None:
        _symbol_token(binding.target_symbol)
        if binding.parent_symbol is not None:
            raise ValueError("direct target alias cannot be combined with a member owner")
    if binding.target_local and (binding.target_symbol is None or binding.parent_symbol is not None
                                 or binding.span is not None):
        raise ValueError("TU-local target requires an unbounded direct target alias")
    if binding.unit != path:
        raise ValueError("object binding belongs to another source unit")
    if (binding.origin.image_sha256, binding.origin.address_space) != (pin, address_space):
        raise ValueError("object binding belongs to another image or address space")
    if type(binding.line) is not int or not 1 <= binding.line <= line_limit:
        raise ValueError("object insertion line is outside the source")


def _unique(records: Sequence[ObjectBinding]) -> None:
    validate_object_symbol_ownership(records)
    if len({row.symbol for row in records}) != len(records):
        raise ValueError("duplicate original object symbol binding")
    validate_object_extent_ownership(records)
    for row in records:
        containing = [owner for owner in records if owner is not row and owner.span is not None
                      and owner.span.start < row.origin.address < owner.span.end]
        if containing and not (
                row.span is None and row.parent_symbol is not None
                and len(containing) == 1
                and containing[0].unit == row.unit
                and containing[0].symbol == row.parent_symbol):
            raise ValueError("object start lies inside another original object extent")


def _marker(binding: ObjectBinding) -> bytes:
    if array_slice_bounds(binding.member_path) is not None:
        assert binding.span is not None and binding.evidence is not None
        text = (f"// RECOVERY_OBJECT: v5 0x{binding.origin.address:08X} "
                f"0x{binding.span.end:08X} {binding.symbol} 0x{binding.evidence.address:08X} "
                f"member {binding.parent_symbol} {binding.member_path[0]}")
    elif binding.parent_symbol is not None and binding.span is None:
        text = (f"// RECOVERY_OBJECT: v3 0x{binding.origin.address:08X} {binding.symbol} "
                f"member {binding.parent_symbol} {'.'.join(binding.member_path)}")
    elif binding.parent_symbol is not None:
        assert binding.span is not None and binding.evidence is not None
        text = (f"// RECOVERY_OBJECT: v3 0x{binding.origin.address:08X} "
                f"0x{binding.span.end:08X} {binding.symbol} 0x{binding.evidence.address:08X} "
                f"member {binding.parent_symbol} {'.'.join(binding.member_path)}")
    elif binding.target_local:
        text = (f"// RECOVERY_OBJECT: v6 0x{binding.origin.address:08X} {binding.symbol} "
                f"target local {binding.target_symbol}")
    elif binding.target_symbol is not None and binding.span is None:
        text = (f"// RECOVERY_OBJECT: v4 0x{binding.origin.address:08X} {binding.symbol} "
                f"target {binding.target_symbol}")
    elif binding.target_symbol is not None:
        assert binding.span is not None and binding.evidence is not None
        text = (f"// RECOVERY_OBJECT: v4 0x{binding.origin.address:08X} "
                f"0x{binding.span.end:08X} {binding.symbol} 0x{binding.evidence.address:08X} "
                f"target {binding.target_symbol}")
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
    splice = re.search(r"(?:\\|\?\?/)[ \t]*\n", normalized)
    # A later untouched splice cannot change the lexical state of an earlier
    # insertion. We do not interpret the spliced suffix: all insertion anchors
    # must stay strictly before its first physical line, including batch peers.
    first_splice_line = normalized.count("\n", 0, splice.start()) + 1 if splice else None
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
        if first_splice_line is not None and binding.line >= first_splice_line:
            raise ValueError("object writer does not support insertion in line-spliced source")
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
               row.parent_symbol, row.member_path, row.target_symbol, row.target_local)
              for row in tuple(parsed) + tuple(additions)}
    got = {(row.origin.entity_id, row.symbol, row.span, row.evidence,
            row.parent_symbol, row.member_path, row.target_symbol, row.target_local)
           for row in reread}
    if got != wanted:
        raise ValueError("object annotation round-trip changed its identity")
    return result


def replace_object_records(current: Mapping[str, bytes], *,
                           expected: Sequence[ObjectBinding],
                           replacements: Sequence[ObjectBinding],
                           destination: str, pin: str,
                           address_space: str,
                           direct_owner_promotions: frozenset[str] = frozenset(),
                           local_target_promotions: frozenset[str] = frozenset()) -> dict[str, bytes]:
    """Rehome existing identities as member records or direct target aliases.

    ``current`` must contain every affected source unit and ``expected`` must
    be the complete parsed object snapshot for those units. Replacements are
    one-for-one with selected old rows, preserve their old identity and any
    bounded initializer obligation exactly, and may change only current target,
    owner/path and source location. Dropping a member owner in favor of a direct
    target requires that identity's explicit ``direct_owner_promotions`` grant.
    Promoting an unbounded v4 direct alias to v6 ``target local`` requires its
    exact entity ID in ``local_target_promotions`` and retains the target name.
    This grant authorizes only the pure annotation transport; it proves no ABI,
    compiled layout, target equivalence or runtime behavior. All non-replaced
    records survive byte-for-byte. Selected records and destination anchors must
    precede the first line splice; an untouched spliced suffix may contain other
    records. Callers remain responsible for independent
    native review and guarded publication.
    """
    if type(direct_owner_promotions) is not frozenset or any(
            type(identity) is not str or not identity
            for identity in direct_owner_promotions):
        raise ValueError("direct owner promotions must be a frozenset of nonempty entity IDs")
    if type(local_target_promotions) is not frozenset or any(
            type(identity) is not str or not identity
            for identity in local_target_promotions):
        raise ValueError("local target promotions must be a frozenset of nonempty entity IDs")
    if not current or destination not in current or not replacements:
        raise ValueError("object replacement requires affected units and a destination")
    if any(not isinstance(path, str) or not isinstance(raw, bytes)
           for path, raw in current.items()):
        raise ValueError("object replacement requires source paths and original bytes")
    parsed_by_unit = {}
    lines_by_unit = {}
    newline_by_unit = {}
    boundaries_by_unit = {}
    first_splice_by_unit = {}
    for path, raw in current.items():
        text = raw.decode("utf-8", errors="surrogateescape")
        if re.search(r"[\v\f\x1c-\x1e\x85\u2028\u2029]", text):
            raise ValueError("unsupported source line separator")
        normalized = text.replace("\r\n", "\n").replace("\r", "\n")
        splice = re.search(r"(?:\\|\?\?/)[ \t]*\n", normalized)
        first_splice_by_unit[path] = (
            normalized.count("\n", 0, splice.start()) + 1 if splice else None)
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
    consumed_promotions = set()
    consumed_local_promotions = set()
    for row in replacements:
        old = by_identity.get(row.origin.entity_id)
        if old is None or row.origin.entity_id in selected:
            raise ValueError("object replacement identity is missing or duplicated")
        old_splice = first_splice_by_unit[old.unit]
        if old_splice is not None and old.line >= old_splice:
            raise ValueError("object writer does not support replacement of records in line-spliced source")
        row_interpretation = row.interpretation_id if row.span is not None else None
        old_interpretation = old.interpretation_id if old.span is not None else None
        if (row.symbol, row.origin, row.span, row.evidence, row_interpretation) != (
                old.symbol, old.origin, old.span, old.evidence, old_interpretation):
            raise ValueError("object replacement changed historical identity or initializer obligation")
        identity = row.origin.entity_id
        is_promotion = old.parent_symbol is not None and row.parent_symbol is None
        granted = identity in direct_owner_promotions
        if granted and not is_promotion:
            raise ValueError("direct owner promotion grant does not select a member-to-target transition")
        if is_promotion:
            if not granted:
                raise ValueError("member object replacement cannot discard its current owner")
            if row.member_path != () or not row.target_symbol:
                raise ValueError("member promotion requires an empty member path and nonempty direct target")
            consumed_promotions.add(identity)
        elif old.parent_symbol is not None and row.target_symbol is not None:
            raise ValueError("member object replacement cannot add a direct target alias")
        if old.parent_symbol is None and row.parent_symbol is not None and row.target_symbol is not None:
            raise ValueError("object replacement cannot combine member and direct target owners")
        local_promotion = identity in local_target_promotions
        if local_promotion:
            if not (not old.target_local and row.target_local
                    and old.parent_symbol is None and row.parent_symbol is None
                    and old.target_symbol is not None and row.target_symbol == old.target_symbol
                    and old.span is None and row.span is None):
                raise ValueError("local target promotion must preserve an unbounded direct alias")
            consumed_local_promotions.add(identity)
        elif old.target_local != row.target_local:
            raise ValueError("changing TU-local target scope requires its exact explicit promotion grant")
        _record(row, path=destination, pin=pin, address_space=address_space,
                line_limit=len(lines_by_unit[destination]) + 1)
        if (first_splice_by_unit[destination] is not None
                and row.line >= first_splice_by_unit[destination]):
            raise ValueError("object writer does not support replacement in line-spliced source")
        if boundaries_by_unit[destination][row.line - 1]:
            raise ValueError("object replacement anchor is inside a block comment")
        selected[row.origin.entity_id] = row
    if len(selected) != len(replacements):
        raise ValueError("object replacement identity set is inconsistent")
    if consumed_promotions != direct_owner_promotions:
        raise ValueError("direct owner promotion grants must match member-to-target replacements exactly")
    if consumed_local_promotions != local_target_promotions:
        raise ValueError("local target promotion grants must match scope promotions exactly")
    selected_old = {identity: by_identity[identity] for identity in selected}
    remaining = [row for row in expected if row.origin.entity_id not in selected]
    _unique(tuple(remaining) + tuple(replacements))
    validate_object_symbol_ownership(tuple(remaining) + tuple(replacements))
    # A moved member parent cannot conflict with a different direct/member
    # binding under the same emitted symbol.
    owner_identity = {}
    for row in tuple(remaining) + tuple(replacements):
        emitted = row.emitted_symbol
        # Same-unit direct parent and historical children share one emitted
        # owner. Native object_targets must still prove their original base;
        # this pure writer preserves identities, not compiled layout claims.
        key = ("local", row.unit, emitted) if row.target_local else ("global", emitted)
        identity = ("owner", row.unit, emitted)
        prior = owner_identity.setdefault(key, identity)
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
                row.parent_symbol, row.member_path, row.target_symbol,
                row.target_local, interpretation_id)

    wanted = {continuity(row)
              for row in tuple(remaining) + tuple(replacements)}
    got = {continuity(row) for row in reread}
    if got != wanted:
        raise ValueError("object replacement changed source identity continuity")
    return output
