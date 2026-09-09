"""Bounded lexical C/C++ recovery check, not a compiler or a structure proof.

Scan every branch of configured application source, ignoring comments and
literals. There is no macro expansion: absence of explicit assembly tokens is
necessary, not sufficient, evidence of genuine C++. Dependency declarations
classify exact files only; they do not prove upstream identity or dependency ABI.
"""

from bisect import bisect_right
from pathlib import Path
import re

from .component_source import parse_components
from .source_config import SourceConfiguration


_FORBIDDEN = {"asm", "_asm", "__asm", "__asm__", "_emit", "__emit"}
_IDENTIFIER = re.compile(r"[A-Za-z_$][A-Za-z_0-9$]*")
_NUMBER = re.compile(r"(?:[0-9]|\.[0-9])(?:[eEpP][+-]|[A-Za-z_0-9.'])*")
_RAW = re.compile(r'(?:u8|u|U|L)?R"([^ ()\\\t\r\n]{0,16})\(')
_HEADER = re.compile(r"^[ \t]*#[ \t]*include(?:_next)?[ \t]*<[^>\n]+>", re.MULTILINE)


def explicit_assembly(text: str) -> dict:
    """Return physical source lines for explicit tokens; lexical gaps are unknown."""
    # Translation-phase line splicing precedes comments and token recognition.
    pieces, starts, deltas, previous, length = [], [], [], 0, 0
    for match in re.finditer(r"\\\r?\n", text):
        piece = text[previous:match.start()]
        pieces.append(piece)
        length += len(piece)
        starts.append(length)
        deltas.append(match.end() - length)
        previous = match.end()
    source = "".join(pieces) + text[previous:]

    def line(position):
        index = bisect_right(starts, position) - 1
        original = position + (deltas[index] if index >= 0 else 0)
        return text.count("\n", 0, original) + 1

    findings, unsupported = [], []
    if re.search(r"\?\?[=/'()!<>-]", text):
        # Phase-one translation could change which later tokens are comments.
        return {"result": "unknown", "findings": [],
                "unsupported": [{"line": 1, "reason": "trigraph translation is unsupported"}]}
    position = 0
    while position < len(source):
        start = position
        if source.startswith("//", start):
            end = source.find("\n", start + 2)
            position = len(source) if end < 0 else end
        elif source.startswith("/*", start):
            end = source.find("*/", start + 2)
            if end < 0:
                unsupported.append({"line": line(start), "reason": "unterminated block comment"})
                break
            position = end + 2
        elif match := _RAW.match(source, start):
            end_token = ")" + match[1] + '"'
            end = source.find(end_token, match.end())
            if end < 0:
                unsupported.append({"line": line(start), "reason": "unterminated raw string"})
                break
            position = end + len(end_token)
        elif source[start] in "\"'":
            quote = source[start]
            position += 1
            while position < len(source) and source[position] not in (quote, "\n"):
                position += 2 if source[position] == "\\" else 1
            if position >= len(source) or source[position] != quote:
                unsupported.append({"line": line(start), "reason": "unterminated literal"})
                break
            position += 1
        elif match := _HEADER.match(source, start):
            position = match.end()
        elif match := _NUMBER.match(source, start):
            position = match.end()
        elif match := _IDENTIFIER.match(source, start):
            token = match[0]
            if token in _FORBIDDEN:
                findings.append({"line": line(start), "token": token})
            if token in ("R", "u8R", "uR", "UR", "LR") and source[match.end():].startswith('"'):
                unsupported.append({"line": line(start), "reason": "unsupported raw string delimiter"})
                break
            position = match.end()
        else:
            if source.startswith(("\\u", "\\U"), start):
                unsupported.append({"line": line(start), "reason": "escaped identifier is unsupported"})
            position += 1
    return {"result": "fail" if findings else "unknown" if unsupported else "pass",
            "findings": findings, "unsupported": unsupported}


def check_source_form(root: Path) -> dict:
    """Check application files, not configured seeds or exact upstream source files.

    Callers establishing acceptance must bind this observation to their source
    epoch, as they do compiler measurements; this API never stores a verdict.
    """
    root = root.resolve()
    config = SourceConfiguration.read(root)
    sources = {path.relative_to(root).as_posix(): path.read_text(encoding="utf-8")
               for path in config.files(root)}
    manifests = parse_components({name: value for name, value in sources.items() if not config.is_seed(name)})
    dependencies = {}
    from .components import VendorManifest
    for manifest in manifests:
        if not isinstance(manifest, VendorManifest):
            continue  # Precompiled inputs do not exempt any application source.
        for item in manifest.files:
            name = manifest.source_root + "/" + item.path
            if name in dependencies:
                raise ValueError("source file owned by multiple upstream components: " + name)
            dependencies[name] = manifest.component.component_id
    findings, unsupported, checked, excluded = [], [], [], []
    for name, value in sources.items():
        if config.is_seed(name) or name in dependencies:
            excluded.append({"path": name, "kind": "seed" if config.is_seed(name) else "upstream_source",
                             "component": dependencies.get(name)})
            continue
        report = explicit_assembly(value)
        checked.append(name)
        findings.extend({"path": name, **row} for row in report["findings"])
        unsupported.extend({"path": name, **row} for row in report["unsupported"])
    return {"check": "cpp_source_form", "result": "fail" if findings else "unknown" if unsupported or not checked else "pass",
            "findings": findings, "unsupported": unsupported, "checked": checked, "excluded": excluded,
            "limitations": ["lexical explicit assembly check; no macro expansion or semantic proof",
                            "dependency declarations classify files, not upstream identity or ABI"]}
