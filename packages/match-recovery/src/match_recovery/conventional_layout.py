"""Conventional member-offset hints, not original-layout proof or a C++ ABI model.

Read existing trailing offset comments and field_<hex>_ names. The compiler,
never a table of guessed type sizes, supplies the corresponding actual offsets.
Unsupported declarations remain diagnostics; this is deliberately not a C++ parser.
"""

import base64
from bisect import bisect_right
from dataclasses import dataclass
from pathlib import PurePosixPath
import re

from .component_source import parse_components
from .includes import COMMENTS_OR_STRINGS
from .model import Result, digest


SCOPE = re.compile(r"\b(?P<kind>class|struct|union|namespace)\s+(?P<name>[A-Za-z_]\w*(?:::\w+)*)"
                   r"\s*(?:final\s*)?(?::[^;{}]+)?\{")
MEMBER = re.compile(r"\s*(?!(?:static|typedef|using|friend|virtual|return)\b)"
                    r"[A-Za-z_]\w*(?:::\w+)*(?:\s+[A-Za-z_]\w*)*\s+"
                    r"[*&]*\s*(?P<name>[A-Za-z_]\w*)\s*(?:\[[^\]\n]+\]\s*)*;\s*")
NOTE = re.compile(r"\s*//\s*(?:[A-Za-z_]\w*\+)?(?P<offset>0x[0-9A-Fa-f]+|\(\+\d+\))(?!\w)")
SELF_NAME = re.compile(r"\bfield_([0-9A-Fa-f]+)_\b")


@dataclass(frozen=True)
class OffsetHint:
    class_name: str
    member: str
    offset: int
    path: str
    line: int
    basis: str

    @property
    def interpretation_id(self):
        return digest(self.__dict__)


def parse_offsets(text: str, path: str):
    """Return directly declared hints and explicit unsupported-source diagnostics."""
    hints, errors = [], []
    def problem(line, reason):
        errors.append({"path": path, "line": line, "reason": reason})
    uncommented = COMMENTS_OR_STRINGS.sub(
        lambda m: re.sub(r"[^\n]", " ", m[0]) if m[0].startswith(("//", "/*")) else m[0], text)
    if re.search(r'\b(?:u8|u|U|L)?R"|\?\?[=/\'()!<>-]', uncommented) or "\\\n" in uncommented:
        problem(1, "raw strings, trigraphs or continued lines require a full C++ lexer")
        return hints, errors
    code = COMMENTS_OR_STRINGS.sub(lambda m: re.sub(r"[^\n]", " ", m[0]), text)
    named = {m.end() - 1: (m["kind"], m["name"]) for m in SCOPE.finditer(code)}
    unsupported = {m.end() - 1 for m in re.finditer(r"\b(?:namespace|class|struct|union)\s*\{", code)}
    for match in SCOPE.finditer(code):
        prefix = code[:match.start()]
        if match["kind"] == "union" or re.search(r"(?:template\s*<[^{};]*>|\benum)\s*$", prefix):
            unsupported.add(match.end() - 1)
    stack, positions, owners, names = [], [-1], [None], {}
    for token in re.finditer(r"[{}]", code):
        at = token.start()
        if token[0] == "{":
            parent = stack[-1] if stack else None
            if at in named and at not in unsupported and (parent is None or parent in names):
                names[at] = ((names[parent][0] + "::") if parent is not None else "") + named[at][1], named[at][0]
            elif at in named:
                unsupported.add(at)
            stack.append(at)
        elif not stack:
            problem(code.count("\n", 0, at) + 1, "unbalanced source braces")
        else:
            stack.pop()
        positions.append(at)
        owners.append(stack[-1] if stack else None)
    if stack:
        problem(1, "unclosed source braces")
    for opening in named:
        end = code.find("\n", opening)
        end = len(code) if end < 0 else end
        tail = code[opening + 1:end]
        if SELF_NAME.search(tail) or re.search(r";\s*//\s*(?:0x[0-9A-Fa-f]+|\(\+\d+\))", text[opening + 1:end]):
            problem(code.count("\n", 0, opening) + 1, "same-line class members require a full declaration parser")
    parens, paren_positions, paren_owners = [], [-1], [None]
    for token in re.finditer(r"[()]", code):
        if token[0] == "(":
            parens.append(token.start())
        elif parens:
            parens.pop()
        paren_positions.append(token.start())
        paren_owners.append(parens[0] if parens else None)
    at = 0
    for line, raw in enumerate(text.splitlines(keepends=True), 1):
        masked = code[at:at + len(raw)]
        first = at + len(masked) - len(masked.lstrip())
        owner = owners[bisect_right(positions, first) - 1]
        at += len(raw)
        if owner in unsupported and (SELF_NAME.search(masked) or re.search(r";\s*//\s*(?:0x|\(\+)", raw)):
            problem(line, "offset hint belongs to an unsupported class/namespace/union scope")
            continue
        if owner not in names or names[owner][1] not in ("class", "struct"):
            continue
        if not masked.strip() or masked.lstrip().startswith("#"):
            continue
        opening = paren_owners[bisect_right(paren_positions, first) - 1]
        if opening is not None:
            prefix = code[code.rfind("\n", 0, opening) + 1:opening]
            if re.fullmatch(r"[^();{}=\[\]]+", prefix) and not code[opening + 1:].lstrip().startswith(("*", "&", "__")):
                continue
        # Method entry addresses and field references inside inline methods are
        # not data-member offsets. Do not discard function-pointer declarators.
        if re.match(r"^[^(;{}=\[\]]+\((?!\s*[*&])", masked):
            continue
        match = MEMBER.fullmatch(masked.rstrip("\r\n"))
        semicolon = masked.find(";")
        note = NOTE.match(raw[semicolon + 1:]) if semicolon >= 0 else None
        self_name = SELF_NAME.search(masked)
        if not note and not self_name:
            continue
        if not match:
            problem(line, "offset hint has an unsupported direct-member declaration")
            continue
        class_name, member = names[owner][0], match["name"]
        if note:
            value = note["offset"]
            offset = int(value[2:-1], 10) if value.startswith("(+") else int(value, 16)
            hints.append(OffsetHint(class_name, member, offset, path, line, "trailing_offset_comment"))
        if SELF_NAME.fullmatch(member):
            hints.append(OffsetHint(class_name, member, int(SELF_NAME.fullmatch(member)[1], 16),
                                    path, line, "self_naming_member"))
    return hints, errors


def read_offsets(snapshot, source):
    hints, errors = [], []
    sources = {path: base64.b64decode(item["content_base64"]).decode("utf-8")
               for path, item in snapshot["files"].items()}
    from .components import VendorManifest
    dependencies = sorted({item.source_root for item in parse_components(sources) if isinstance(item, VendorManifest)})
    for path, text in sorted(sources.items()):
        if (source.is_seed(path) or PurePosixPath(path).suffix not in source.interface_extensions
                or any(PurePosixPath(path).is_relative_to(parent) for parent in dependencies)):
            continue
        parsed, diagnostics = parse_offsets(text, path)
        hints.extend(parsed)
        errors.extend(diagnostics)
    return tuple(hints), errors, dependencies


def check_offset(item, layout):
    actual = layout["members"].get(item.member)
    return {"actual_offset": actual,
            "result": Result.UNKNOWN if actual is None else Result.PASS if actual == item.offset else Result.FAIL,
            "reason": "conventional hint compared with compiler member offset; not original-layout proof"}
