"""Shared physical-source guards for add-only annotation writers.

The owning writer validates typed records and their complete snapshot. This
module handles only safe placement into the caller's unchanged source bytes.
"""

from __future__ import annotations

import re
from typing import Sequence

from .reader import _block_state_after
from .writer import _SYMBOL_SOURCE_TOKENS


def insert_annotation_lines(raw: bytes, *, insertions: Sequence[tuple[int, bytes]],
                            marker_prefix: bytes, label: str) -> bytes:
    """Insert encoded marker lines before one-based source lines or at EOF.

    Existing marker bytes and all ordinary source bytes are preserved. The
    caller remains responsible for typed record validation and round trips.
    """
    if type(raw) is not bytes:
        raise ValueError(f"{label} source rewriting requires original bytes")
    _validate_marker_bytes(marker_prefix, "annotation marker prefix")
    text = raw.decode("utf-8", errors="surrogateescape")
    if re.search(r"[\v\f\x1c-\x1e\x85\u2028\u2029]", text):
        raise ValueError("unsupported source line separator")
    normalized = text.replace("\r\n", "\n").replace("\r", "\n")
    splice = re.search(r"(?:\\|\?\?/)[ \t]*\n", normalized)
    first_splice_line = (normalized.count("\n", 0, splice.start()) + 1
                         if splice else None)
    if raw.startswith(b"\xef\xbb\xbf"):
        raise ValueError(f"{label} writer does not support BOM-prefixed source")
    if any(re.fullmatch(r'(?:u8|u|U|L)?R"', token.group())
           for token in _SYMBOL_SOURCE_TOKENS.finditer(normalized)):
        raise ValueError(f"{label} writer does not support raw-string source")

    lines = raw.splitlines(keepends=True)
    line_count = len(lines)
    block = False
    boundaries = []
    for line in text.splitlines():
        boundaries.append(block)
        block = _block_state_after(line, block)
    boundaries.append(block)

    for line_number, marker in insertions:
        if type(line_number) is not int or not 1 <= line_number <= line_count + 1:
            raise ValueError(f"{label} insertion line is outside the source")
        _validate_marker_bytes(marker, "annotation marker")
        if not marker.startswith(marker_prefix):
            raise ValueError(f"{label} writer received an invalid annotation marker")
        if first_splice_line is not None and line_number >= first_splice_line:
            raise ValueError(f"{label} writer does not support insertion in line-spliced source")
        if boundaries[line_number - 1]:
            raise ValueError(f"{label} insertion anchor is inside a block comment")
        if (line_number <= line_count
                and lines[line_number - 1].lstrip().startswith(marker_prefix)):
            raise ValueError(f"{label} insertion would shadow an existing annotation")

    newline = next((line[len(line.rstrip(b"\r\n")):]
                    for line in lines if line.endswith((b"\r\n", b"\n", b"\r"))), b"\n")
    if newline not in (b"\n", b"\r", b"\r\n"):
        raise ValueError("unsupported source newline")
    inserted: dict[int, list[bytes]] = {}
    for line_number, marker in insertions:
        inserted.setdefault(line_number, []).append(marker + newline)

    output = []
    for line_number in range(1, line_count + 2):
        if (line_number == line_count + 1 and inserted.get(line_number)
                and raw and not raw.endswith((b"\r", b"\n"))):
            output.append(newline)
        output.extend(inserted.get(line_number, ()))
        if line_number <= line_count:
            output.append(lines[line_number - 1])
    return b"".join(output)


def _validate_marker_bytes(value: bytes, description: str) -> None:
    if type(value) is not bytes or not value:
        raise ValueError(f"{description} must be nonempty UTF-8 bytes")
    try:
        text = value.decode("utf-8")
    except UnicodeDecodeError as error:
        raise ValueError(f"{description} must be valid UTF-8 bytes") from error
    if any(ord(char) < 0x20 or 0x7F <= ord(char) <= 0x9F
           or ord(char) in (0x2028, 0x2029) for char in text):
        raise ValueError(f"{description} must contain exactly one physical line")
