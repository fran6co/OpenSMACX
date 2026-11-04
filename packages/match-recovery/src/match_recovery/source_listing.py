"""Strict helpers for binding MSVC assembly listings to source text.

These helpers establish evidence for one configured compiler listing. They do
not prove historical translation-unit identity or linked-object selection.
"""

from __future__ import annotations

from pathlib import Path
import re
from typing import Mapping


def listing_lines(text: str, symbol: str, code: bytes) -> dict[int, int | None]:
    """Read MSVC /FAcs source anchors, checking each listed byte against COFF.

    Wrapped byte lines continue the preceding instruction. Unsupported listings
    refuse attribution rather than guessing source positions from instructions.
    """
    active = False
    line = None
    result = {}
    seen = bytearray(len(code))
    cursor = None
    found = 0
    segment = None
    root_segment = None
    for row in text.splitlines():
        section = re.match(r"(\S+)\s+SEGMENT\b", row)
        if section:
            segment = section[1]
        if re.match(r"\S+\s+ENDS\b", row):
            segment = None
        if re.match(re.escape(symbol) + r"\s+PROC\b", row):
            active = True
            found += 1
            root_segment = segment
            continue
        if active and re.match(re.escape(symbol) + r"\s+ENDP\b", row):
            active = False
        if not active:
            continue
        if segment != root_segment:
            continue
        anchor = re.match(r";\s*(\d+)\s*:", row)
        if anchor:
            line = int(anchor[1])
            continue
        start = re.match(r"\s+([0-9a-fA-F]{5,8})\t", row)
        if start:
            cursor = int(start[1], 16)
            rest = row[start.end():]
            if line is not None:
                result[cursor] = line
        elif row.startswith("\t") and cursor is not None:
            rest = row[1:]
        else:
            continue
        raw = rest.split("\t", 1)[0].strip()
        if not re.fullmatch(r"[0-9a-fA-F]{2}(?: +[0-9a-fA-F]{2})*", raw):
            continue
        data = bytes.fromhex(raw)
        if cursor is None or code[cursor:cursor + len(data)] != data:
            raise ValueError("compiler listing bytes do not match the fresh object")
        seen[cursor:cursor + len(data)] = b"\1" * len(data)
        cursor += len(data)
    covered = next((i for i, value in enumerate(seen) if not value), len(seen))
    # VC6 omits the section's terminal alignment bytes from /FAcs. Keep these
    # bytes visible in the assembly, with no fabricated C++ line attribution.
    padding = code[covered:]
    valid_tail = (covered > 0 and len(padding) <= 15
                  and all(value in (0x90, 0xCC) for value in padding)
                  and not any(seen[covered:]))
    if found != 1 or not result or (not all(seen) and not valid_tail):
        raise ValueError("listing does not completely cover one selected function")
    if padding:
        result[covered] = None
    return result


def _selected_quotes(text: str, symbol: str, anchors: set[int]) -> dict[int, str]:
    active = False
    found = 0
    quotes: dict[int, str] = {}
    for row in text.splitlines():
        if re.match(re.escape(symbol) + r"\s+PROC\b", row):
            if active:
                raise ValueError("nested selected procedures in compiler listing")
            active = True
            found += 1
            continue
        if active and re.match(re.escape(symbol) + r"\s+ENDP\b", row):
            active = False
            continue
        if active:
            match = re.match(r";\s*(\d+)\s*:(.*)", row)
            if match and int(match[1]) in anchors and match[2].strip():
                number, quote = int(match[1]), match[2].strip()
                previous = quotes.get(number)
                if previous is not None and previous != quote:
                    raise ValueError("compiler listing has conflicting source quotes for one line")
                quotes[number] = quote
    if found != 1 or active:
        raise ValueError("compiler listing does not contain one complete selected procedure")
    if not anchors or set(quotes) != anchors:
        raise ValueError("compiler listing lacks source quotes for every selected anchor")
    return quotes


def _source_path(path: str | Path) -> Path:
    candidate = Path(path)
    normalized = Path(candidate.as_posix())
    if (not normalized.as_posix() or normalized.as_posix() == "."
            or any(part in ("", ".", "..") for part in normalized.parts)):
        raise ValueError("source path is not canonical")
    # Captured labels must not be resolved against the live filesystem.
    # Physical-source validation belongs to the snapshot's owner.
    return normalized


def listing_source_texts(text: str, symbol: str,
                         source_texts: Mapping[str | Path, str],
                         anchors: set[int]) -> tuple[Path, str]:
    """Match selected procedure quotes to exactly one captured source file.

    Paths are canonicalized before returning. The caller should pass immutable
    captured source text, not use this helper to reopen arbitrary live files.
    A listing does not carry a source path token, so identical text in multiple
    files is deliberately ambiguous and refused.
    """
    quotes = _selected_quotes(text, symbol, anchors)
    matches: dict[Path, str] = {}
    for raw_path, source in source_texts.items():
        path = _source_path(raw_path)
        rows = source.splitlines()
        if all(1 <= number <= len(rows) and rows[number - 1].strip() == quote
               for number, quote in quotes.items()):
            matches[path] = source
    if len(matches) != 1:
        raise ValueError("compiler source lines do not identify one current source file")
    return next(iter(matches.items()))
