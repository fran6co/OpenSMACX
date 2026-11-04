"""Pure, snapshot-backed discovery of a mapping's declared source body.

This module returns lexical candidates only. A unique C++ spelling does not
prove the complete emitted-symbol identity or the compiler/include context;
native callers need independent evidence for those facts before admission.
"""

from __future__ import annotations

from dataclasses import dataclass, replace
from pathlib import PurePosixPath
import re
from typing import Mapping

from . import reader
from .mangled import identifier, qualified_name


@dataclass(frozen=True)
class BodyResolution:
    """A body candidate cut from caller-supplied source text.

    ``status`` is ``candidate`` only when one supported qualified source
    definition was found. It is never an authoritative symbol/context proof.
    Unresolved results have no body or line range.
    """

    status: str
    path: str
    line: int | None
    end_line: int | None
    body: str
    identity_evidence: str
    context_evidence: str
    reason: str


def has_placeholder_body(record, *, marker_path, source_texts, symbol_format='msvc'):
    """A declared location cannot erase a matching marker-owned body slot."""
    candidates = [resolve_body(record, marker_path=marker_path,
                               source_texts=source_texts, symbol_format=symbol_format)]
    if record.body != marker_path:
        candidates.append(resolve_body(replace(record, body=''), marker_path=marker_path,
                                       source_texts=source_texts, symbol_format=symbol_format))
    return any(candidate.status == 'candidate' and reader._is_placeholder_region(candidate.body)
               for candidate in candidates)


def _canonical(path: str) -> str:
    parsed = PurePosixPath(path)
    if (not path or parsed.is_absolute() or path != parsed.as_posix()
            or any(part in ("", ".", "..") for part in parsed.parts)):
        raise ValueError("source path must be canonical and project-relative")
    return path


def resolve_body(record, *, marker_path: str,
                 source_texts: Mapping[str, str],
                 symbol_format: str = "msvc") -> BodyResolution:
    """Find one lexical definition candidate without reading the filesystem.

    ``source_texts`` is an immutable/current source snapshot supplied by the
    caller, keyed by canonical project-relative paths. No path fallback or
    disk lookup is attempted. Full MSVC signature, emitted-symbol and
    compilation-context identity remain explicitly unproven.
    """
    try:
        marker = _canonical(marker_path)
    except (TypeError, ValueError) as error:
        return BodyResolution("unresolved", "", None, None, "", "unproven",
                              "unproven", str(error))
    target = record.body or marker
    try:
        target = _canonical(target)
    except (TypeError, ValueError) as error:
        return BodyResolution("unresolved", str(target), None, None, "", "unproven",
                              "unproven", f"invalid declared body path: {error}")
    text = source_texts.get(target)
    if not isinstance(text, str):
        return BodyResolution("unresolved", target, None, None, "", "unproven",
                              "unproven", "body path is absent from the supplied source snapshot")

    symbol = record.symbol or record.name
    if symbol_format == "msvc":
        name = qualified_name(symbol)
    elif symbol_format == "none":
        # This mode carries a source identifier (possibly with ordinary
        # platform decoration), not a namespace/signature proof.
        name = identifier(symbol) or (symbol if symbol.isidentifier() else "")
    else:
        name = ""
    if not name:
        return BodyResolution("unresolved", target, None, None, "", "unproven",
                              "unproven", "mapping symbol has no supported source spelling")

    try:
        first = reader.function_line(text + "\n", name)
    except ValueError as error:
        return BodyResolution("unresolved", target, None, None, "", "unproven",
                              "unproven", f"unique lexical definition unresolved: {error}")
    lines = text.splitlines()
    if "::" not in name:
        # An unqualified global symbol cannot be placed inside a namespace,
        # class or other braced scope based on spelling alone. Reject those
        # candidates rather than treating a same-named member as the target.
        depth, in_block = 0, False
        for line in lines[:first - 1]:
            delta, _, _, _ = reader._brace_delta(line, in_block)
            depth += delta
            in_block = reader._block_state_after(line, in_block)
        code = reader._brace_delta(lines[first - 1], in_block)[3]
        found_name = re.search(rf"(?<![\w:]){re.escape(name)}\s*\(", code)
        if found_name is None:
            return BodyResolution("unresolved", target, None, None, "", "unproven",
                                  "unproven", "unqualified symbol location could not be isolated")
        depth += reader._brace_delta(code[:found_name.start()], False)[0]
        if depth != 0:
            return BodyResolution("unresolved", target, None, None, "", "unproven",
                                  "unproven", "unqualified symbol candidate is nested in a C++ scope")
    end_index = reader._region_end(lines, first - 1, name="")
    if end_index is None or end_index < first - 1:
        return BodyResolution("unresolved", target, first, None, "", "unproven",
                              "unproven", "candidate definition has no bounded body")
    body = "\n".join(lines[first - 1:end_index + 1])
    return BodyResolution(
        "candidate", target, first, end_index + 1, body,
        "qualified-name-only; emitted symbol and full signature unproven",
        "compile-context/include closure unproven",
        "one lexical definition matches the supported source spelling; independent compiler and context evidence is still required",
    )
