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
    # ``qualified`` for an out-of-class/free definition spelled with its full
    # name; ``in_class`` for a member defined inside its unique global class
    # body (the form the ``body`` header pointer exists for). ``implicit``
    # (status ``implicit``, no body) is a destructor the class never
    # declares: the compiler generates it, and line/end_line span the class.
    form: str = "qualified"


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
        if str(error) == f"{name}: 0 definitions found":
            in_class = _in_class_definition(text, name)
            if isinstance(in_class, BodyResolution):
                return replace(in_class, path=target)
            if in_class:
                error = ValueError(f"{error}; in-class: {in_class}")
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


_CLASS_HEAD = re.compile(r"^\s*(?:class|struct)\s+([A-Za-z_]\w*)\b(?:\s*:[^;{]*)?\s*(?:\{.*)?$")


def _in_class_definition(text: str, name: str):
    """One member definition written inside its unique global class body.

    ``Owner::member`` (exactly one scope level) resolves only when ``Owner``
    has exactly one global-scope class/struct definition in ``text`` and
    ``member`` occurs exactly once at member level there, as a definition.
    A member that is merely declared, overloaded, nested in another scope or
    absent stays unresolved. Returns a candidate, or a refusal string, or ""
    when no class of that name is defined here.
    A destructor with no member-level occurrence at all, in a class free of
    preprocessor directives and macro-like member declarations, resolves as
    ``implicit``: no user declaration exists, so the compiler generates it.
    """
    owner, separator, member = name.partition("::")
    if not separator or "::" in member or not owner.isidentifier():
        return ""
    destructor = member.startswith("~")
    spelled = member[1:] if destructor else member
    if not spelled.isidentifier() or (destructor and spelled != owner):
        return ""
    pattern = re.compile((r"(?<![\w:])~\s*" if destructor else r"(?<![\w:~])")
                         + re.escape(spelled) + r"\s*\(")
    lines = text.splitlines()
    depth, in_block, classes, current = 0, False, [], None
    for index, line in enumerate(lines):
        state = in_block
        code = reader._brace_delta(line, state)[3]
        in_block = reader._block_state_after(line, state)
        if code.lstrip().startswith("#"):
            if current is not None and current["opened"]:
                current["preprocessor"] = True
            continue
        if current is None and depth == 0:
            head = _CLASS_HEAD.match(code)
            if head is not None and head.group(1) == owner and not code.rstrip().endswith(";"):
                current = {"start": index, "opened": False, "members": [], "code": [],
                           "preprocessor": False}
        for position, char in enumerate(code):
            if current is not None and current["opened"] and depth == 1:
                match = pattern.match(code, position)
                if match is not None:
                    current["members"].append((index, state, match))
            if char == "{":
                depth += 1
                if current is not None:
                    current["opened"] = True
            elif char == "}":
                depth -= 1
                if current is not None and current["opened"] and depth == 0:
                    current["end"] = index
                    classes.append(current)
                    current = None
        if current is not None:
            current["code"].append(code)
        elif classes and classes[-1].get("end") == index:
            classes[-1]["code"].append(code)
        if current is not None and not current["opened"] and code.rstrip().endswith(";"):
            current = None
    if not classes:
        return ""
    if len(classes) != 1:
        return f"{owner}: {len(classes)} class definitions found"
    members = classes[0]["members"]
    if destructor and not members:
        from ..generated_member_source import _unsupported_member_macro
        if classes[0]["preprocessor"]:
            return f"{name}: class body has preprocessor-controlled members"
        if _unsupported_member_macro(classes[0]["code"], owner):
            return f"{name}: class body has macro-like member declarations"
        start, end = classes[0]["start"], classes[0]["end"]
        return BodyResolution(
            "implicit", "", start + 1, end + 1, "",
            "class declares no destructor; compiler-generated member, emitted symbol unproven",
            "compile-context/include closure unproven",
            "the unique owner class declares no destructor, so the compiler generates it; independent compiler evidence is still required",
            "implicit",
        )
    if len(members) != 1:
        return f"{name}: {len(members)} member-level occurrences found"
    index, state, match = members[0]
    if index == classes[0]["start"]:
        return f"{name}: member shares its class head line"
    if not reader._is_definition(lines, index, state, match):
        return f"{name}: declared in its class without an in-class definition"
    end_index = reader._region_end(lines, index, name="")
    if end_index is None or end_index < index:
        return f"{name}: in-class definition has no bounded body"
    return BodyResolution(
        "candidate", "", index + 1, end_index + 1, "\n".join(lines[index:end_index + 1]),
        "in-class member spelling only; emitted symbol and full signature unproven",
        "compile-context/include closure unproven",
        "one in-class member definition matches the supported source spelling; independent compiler and context evidence is still required",
        "in_class",
    )
