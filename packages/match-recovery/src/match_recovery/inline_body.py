"""Bounded lexical damage probes for explicitly identified source bodies.

This is not a C++ parser or ownership proof. A successful probe only identifies
one ordinary definition to perturb. Callers must freshly compile the unchanged
and damaged header in both the canonical and candidate translation units and
observe changes to the intended function in each object.
"""

import re

from .includes import COMMENTS_OR_STRINGS
from .source.mangled import arity, identifier


_ID = r"[A-Za-z_][A-Za-z_0-9]*"
_DAMAGE = b"{ for (;;) {} }"


def _masked(raw: bytes) -> str:
    # Latin-1 is a byte-preserving view, not a claim about the source encoding.
    # Non-ASCII content in comments/literals can remain opaque to this probe.
    text = raw.decode("latin-1")
    if (re.search(r"\\(?:\r\n?|\n)|\?\?[=/'()!<>-]", text) or 'R"' in text):
        raise ValueError("unsupported continued line, trigraph or raw string")
    parts, start, index = [], 0, 0
    while index < len(text):
        if text[index] in "\"'" or text.startswith(("/*", "//"), index):
            token = COMMENTS_OR_STRINGS.match(text, index)
            if token is None:
                raise ValueError("unterminated source comment or literal")
            if text[index] in "\"'" and any(c in token[0] for c in "\r\n"):
                raise ValueError("newline in source literal")
            parts.extend((text[start:index], re.sub(r"[^\r\n]", " ", token[0])))
            index = start = token.end()
        else:
            index += 1
    parts.append(text[start:])
    masked = "".join(parts)
    if re.search(r"<%|%>|<:|:>|%:", masked):
        raise ValueError("unsupported source digraph")
    return masked


def _name(symbol: str, symbol_format: str) -> str:
    if not isinstance(symbol, str) or not symbol or symbol != symbol.strip():
        raise ValueError("probe requires an ordinary function symbol")
    if symbol_format == "none":
        name = symbol if re.fullmatch(_ID, symbol) else ""
    elif symbol_format == "msvc":
        if symbol.startswith("?"):
            ordinary = re.fullmatch(r"\?" + _ID + r"(?:@" + _ID + r")*@@[A-Za-z_0-9@]+", symbol)
            if ordinary is None or arity(symbol) is None:
                raise ValueError("unsupported MSVC function decoration")
        elif not re.fullmatch(r"[_@]?" + _ID + r"(?:@\d+)?", symbol):
            raise ValueError("unsupported MSVC function decoration")
        name = identifier(symbol)
    else:
        raise ValueError("unsupported source symbol format")
    if not name or name in {"operator", "if", "for", "while", "switch", "catch", "sizeof"}:
        raise ValueError("probe requires an ordinary function identifier")
    return name


def _end(masked: str, start: int) -> int:
    """Return one past a balanced parenthesis/brace span, or refuse it."""
    stack = []
    pairs = {")": "(", "]": "[", "}": "{"}
    for index in range(start, len(masked)):
        char = masked[index]
        if char in "([{":
            stack.append(char)
        elif char in ")]}":
            if not stack or stack.pop() != pairs[char]:
                raise ValueError("unbalanced source definition")
            if not stack:
                return index + 1
    raise ValueError("unterminated source definition")


def _ordinary_prefix(masked: str, offset: int) -> bool:
    start = max(masked.rfind(token, 0, offset) for token in ";{}") + 1
    prefix = masked[start:offset]
    # Directives preceding the declaration are outside the selected body.
    # A directive on the declaration's own line must not become a definition.
    prefix = re.sub(r"\A(?:\s*#[^\r\n]*(?:\r?\n))*", "", prefix)
    prefix = re.sub(r"^\s*(?:public|protected|private)\s*:\s*", "", prefix)
    prefix = re.sub(r"(?:" + _ID + r"\s*::\s*)+$", "", prefix)
    prefix = prefix.strip()
    if not re.fullmatch(_ID + r"(?:\s+" + _ID + r"|\s*::\s*" + _ID + r"|\s*[*&])*", prefix):
        return False
    # Exclude expression/declarator forms outside this deliberately small set.
    return not set(re.findall(_ID, prefix)) & {
        "return", "throw", "new", "delete", "typedef", "using", "friend",
        "class", "struct", "union", "enum", "operator", "template", "case",
        "else", "co_return", "co_await", "if", "while", "for", "switch",
    }


def damage_body(raw: bytes, symbol: str, symbol_format: str) -> bytes:
    """Damage exactly one unambiguous ordinary definition in a body header.

    The caller supplies the bytes of its explicit ``// body`` source. In-class
    and qualified out-of-class definitions are supported; overload sets,
    constructors, operators, complex declarators and preprocessing inside the
    selected definition are refused. No filesystem or compiler work occurs here.
    """
    if not isinstance(raw, bytes) or not raw.strip():
        raise ValueError("probe requires nonempty source bytes")
    name, masked = _name(symbol, symbol_format), _masked(raw)
    if re.search(r"\b" + re.escape(name) + r"\s*<", masked):
        raise ValueError("unsupported template function spelling")
    definitions, declarations = [], 0
    for match in re.finditer(r"\b" + re.escape(name) + r"\s*\(", masked):
        ordinary = _ordinary_prefix(masked, match.start())
        opening = masked.index("(", match.start(), match.end())
        end = _end(masked, opening)
        suffix = re.match(r"\s*(?:(?:const|volatile)\b\s*)*", masked[end:])
        body = end + suffix.end()
        if not ordinary:
            if body < len(masked) and masked[body] == "{":
                raise ValueError("unsupported function definition prefix")
            continue
        if body >= len(masked):
            raise ValueError("incomplete function declaration")
        if masked[body] == ";":
            declarations += 1
        elif masked[body] == "{":
            stop = _end(masked, body)
            if "#" in masked[match.start():stop]:
                raise ValueError("preprocessor directive inside selected definition")
            definitions.append((body, stop))
        else:
            raise ValueError("unsupported function declarator")
    if len(definitions) != 1 or declarations:
        raise ValueError("expected one unambiguous ordinary function definition")
    start, stop = definitions[0]
    damaged = raw[:start] + _DAMAGE + raw[stop:]
    if damaged == raw:
        raise ValueError("function body already equals the damage probe")
    return damaged
