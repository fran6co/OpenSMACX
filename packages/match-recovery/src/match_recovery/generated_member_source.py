"""Source-owned bindings for implicit compiler-generated destructor members.

These records attach an emitted complete or scalar-deleting destructor to a
real class declaration without inventing a C++ function body or original
function identity. Evidence and carrier compilation remain independently
derived and verified.
"""

from __future__ import annotations

from dataclasses import asdict, dataclass, field
from pathlib import Path
import re

from .bound_objects import FunctionBinding
from .components import relative_path
from .model import Origin, Span, digest
from .source.mangled import qualified_name
from .source_config import SourceConfiguration


PREFIX = "// RECOVERY_GENERATED_MEMBER:"
PATTERN = re.compile(
    r"// RECOVERY_GENERATED_MEMBER: v1 "
    r"(complete_destructor|deleting_destructor) "
    r"(0x[0-9a-fA-F]+) (0x[0-9a-fA-F]+) (\S+) (0x[0-9a-fA-F]+)"
)
_CLASS = re.compile(r"^\s*(?:class|struct)\s+([A-Za-z_]\w*)\s*(?::|\{|final\b)")
_CONDITIONAL = re.compile(r"^\s*#\s*(if|ifdef|ifndef|elif|else|endif)\b")
_PREPROCESSOR = re.compile(r"^\s*#")
_CALL = re.compile(r"[A-Za-z_]\w*")


def _symbol_owner(symbol: str, kind: str) -> str:
    if kind == "complete_destructor":
        if not symbol.startswith("??1"):
            raise ValueError("complete destructor requires an exact ??1 symbol")
        name = qualified_name(symbol)
        owner, separator, method = name.rpartition("::")
        if not separator or method != "~" + owner.rsplit("::", 1)[-1]:
            raise ValueError("unsupported complete destructor ABI name")
        return owner
    if kind == "deleting_destructor":
        if not symbol.startswith("??_G"):
            raise ValueError("deleting destructor requires an exact ??_G symbol")
        name = qualified_name(symbol)
        owner, separator, method = name.rpartition("::")
        if not separator or method != "`scalar deleting destructor'":
            raise ValueError("unsupported scalar deleting destructor ABI name")
        return owner
    raise ValueError("unsupported generated member kind")


@dataclass(frozen=True)
class GeneratedMemberBinding:
    kind: str
    owner: str
    origin: Origin
    span: Span
    symbol: str
    evidence: Origin
    unit: str
    line: int
    compiler_unit: str | None = field(default=None, compare=False)
    context_error: str | None = field(default=None, compare=False)

    def __post_init__(self):
        if self.kind not in ("complete_destructor", "deleting_destructor"):
            raise ValueError("unsupported generated member kind")
        relative_path(self.unit)
        if not re.fullmatch(r"[A-Za-z_]\w*", self.owner):
            raise ValueError("unsupported generated member class owner")
        if _symbol_owner(self.symbol, self.kind) != self.owner:
            raise ValueError("generated member symbol owner disagrees with class declaration")
        FunctionBinding(self.origin, (self.span,), self.symbol, self.unit)
        if (self.evidence.image_sha256, self.evidence.address_space) != (
                self.origin.image_sha256, self.origin.address_space):
            raise ValueError("generated member evidence belongs to another original")
        if type(self.line) is not int or self.line < 1:
            raise ValueError("invalid generated member source location")
        if self.compiler_unit is not None:
            relative_path(self.compiler_unit)
        if self.context_error is not None and (not isinstance(self.context_error, str) or not self.context_error):
            raise ValueError("invalid generated member compiler-context diagnostic")

    @property
    def interpretation(self):
        return {"kind": "compiler_generated_" + self.kind, "owner": self.owner,
                "origin": asdict(self.origin), "span": asdict(self.span),
                "symbol": self.symbol, "unit": self.unit,
                "original_evidence": asdict(self.evidence)}

    @property
    def interpretation_id(self):
        return digest({"entity_id": self.evidence.entity_id, "value": self.interpretation})

    @property
    def body_error(self):
        return None

    @property
    def object_unit(self):
        if self.context_error:
            return None
        return self.compiler_unit


def _include_guard_lines(lines: list[str]) -> set[int]:
    """Recognize only the conventional file-wide #ifndef/#define/#endif guard."""
    from .source.reader import _brace_delta

    state, code_lines = False, []
    for index, line in enumerate(lines):
        code = _brace_delta(line, state)[3]
        state = _brace_delta(line, state)[1]
        if code.strip():
            code_lines.append((index, code.strip()))
    directives = [(index, re.match(r"#\s*(\w+)(?:\s+(\w+))?", code))
                  for index, code in code_lines if code.startswith("#")]
    if not directives or any(match is None for _, match in directives):
        return set()
    first_code_pos = 0
    if directives[0][1].group(1) == "pragma" and directives[0][1].group(2) == "once":
        first_code_pos = 1
    if first_code_pos >= len(directives):
        return set()
    first_index, first = directives[first_code_pos]
    if first.group(1) != "ifndef" or not first.group(2) or first_code_pos + 1 >= len(directives):
        return set()
    second_index, second = directives[first_code_pos + 1]
    # The define must be the next code line after #ifndef, allowing blank and
    # comment lines but not an intervening include or other directive.
    next_code = next((index for index, _ in code_lines if index > first_index), None)
    if (next_code != second_index or second.group(1) != "define"
            or second.group(2) != first.group(2)):
        return set()
    last_index, last = directives[-1]
    if last.group(1) != "endif" or code_lines[-1][0] != last_index:
        return set()
    # There may be nested conditional directives within a guard. The parser
    # ignores only the outer #ifndef/#define/#endif; inner conditions remain
    # counted and therefore refuse an anchored class under their branches.
    return {first_index, second_index, last_index}


def _preprocessor_depth(lines: list[str], target: int, *, in_block: bool) -> int:
    from .source.reader import _brace_delta

    depth = 0
    guard_lines = _include_guard_lines(lines)
    for index, line in enumerate(lines[:target]):
        code = _brace_delta(line, in_block)[3]
        if 'R"' in code:
            raise ValueError("raw-string context is unsupported for generated member declarations")
        in_block = _brace_delta(line, in_block)[1]
        if index in guard_lines:
            continue
        directive = _CONDITIONAL.match(code)
        if directive:
            word = directive.group(1)
            if word in ("if", "ifdef", "ifndef"):
                depth += 1
            elif word == "endif":
                depth -= 1
                if depth < 0:
                    raise ValueError("unbalanced preprocessor conditional")
    if depth:
        raise ValueError("generated member declaration is inside a preprocessor conditional")
    return depth


def _anchor_class(lines: list[str], index: int, in_block: bool):
    """Resolve one adjacent class declaration and reject ambiguous syntax."""
    from .source.reader import _block_state_after, _brace_delta

    depth = _preprocessor_depth(lines, index, in_block=False)
    if depth:
        raise ValueError("generated member declaration is conditionally enabled")
    declaration = None
    state = in_block
    # An anchor may have adjacent generated-member annotations and ordinary
    # comments, but no intervening source declaration or directive.
    for cursor in range(index + 1, len(lines)):
        line = lines[cursor]
        if line.lstrip().startswith(PREFIX):
            continue
        code = _brace_delta(line, state)[3]
        if 'R"' in code:
            raise ValueError("raw-string context is unsupported for generated member declarations")
        delta, state, saw_open, code = _brace_delta(line, state)
        if code.strip().startswith("#") and not code.strip().startswith("#pragma once"):
            raise ValueError("preprocessor context is unsupported for generated member declarations")
        if not code.strip():
            continue
        match = _CLASS.match(code)
        if match:
            declaration = (cursor, match.group(1), state)
            break
        raise ValueError("generated member annotation is not adjacent to a class declaration")
    if declaration is None:
        raise ValueError("generated member class declaration is missing")
    declaration_line, owner, _ = declaration

    # The supported ABI owner parser currently yields unqualified names only.
    # Reject declarations nested in namespaces, classes, or function bodies so
    # a same-spelled class cannot stand in for the emitted symbol's owner.
    outer_depth, outer_block = 0, False
    for line in lines[:declaration_line]:
        delta, outer_block, _, code = _brace_delta(line, outer_block)
        if code.lstrip().startswith("#"):
            continue
        outer_depth += delta
        if outer_depth < 0:
            raise ValueError("generated member class has unsupported surrounding scope")
    if outer_depth != 0:
        raise ValueError("namespaced or nested generated member class is unsupported")

    # Parse the class body with the same comment/string-aware brace scanner as
    # source annotations. Conditional or macro-generated members are unknown.
    brace_depth, opened, body_lines, state = 0, False, [], False
    explicit_destructor = False
    end = None
    for cursor in range(declaration_line, len(lines)):
        line = lines[cursor]
        if 'R"' in _brace_delta(line, state)[3]:
            raise ValueError("raw-string class context is unsupported for generated member declarations")
        delta, state, saw_open, code = _brace_delta(line, state)
        if _PREPROCESSOR.match(code) and not code.strip().startswith("#pragma once"):
            raise ValueError("preprocessor-controlled class members are unsupported")
        before = brace_depth
        brace_depth += delta
        opened = opened or saw_open
        body_lines.append(code)
        if ((before >= 1 or brace_depth >= 1 or cursor == declaration_line and saw_open)
                and re.search(rf"~\s*{re.escape(owner)}\s*\(", code)):
            explicit_destructor = True
        if opened and brace_depth == 0:
            if not line.rstrip().endswith("};"):
                raise ValueError("class declaration has unsupported closing syntax")
            end = cursor
            break
    if end is None:
        raise ValueError("class declaration is incomplete")
    if _unsupported_member_macro(body_lines, owner):
        raise ValueError("macro-generated class members are unsupported")
    # A second definition of the same global class in one source file makes
    # this lexical anchor ambiguous. Forward declarations do not count.
    duplicates = 0
    current_block = False
    for number, line in enumerate(lines):
        code = _brace_delta(line, current_block)[3]
        current_block = _block_state_after(line, current_block)
        if _CLASS.match(code) and _CLASS.match(code).group(1) == owner:
            duplicates += 1
    if duplicates != 1:
        raise ValueError("generated member class declaration is ambiguous")
    return owner, explicit_destructor


def _unsupported_member_macro(code_lines: list[str], owner: str) -> bool:
    """Reject function-like tokens at class-member level unless declarators.

    Function calls inside inline method bodies are below class-member depth
    and do not affect whether the compiler synthesizes a destructor. At member
    depth, only ordinary declarations with a visible type prefix and the
    class's own constructor/destructor spellings are admitted; macro-like
    declaration forms remain unsupported.
    """
    depth, segment = 0, ""
    for line in code_lines:
        index = 0
        while index < len(line):
            char = line[index]
            if char == "{":
                depth += 1
                if depth == 1:
                    segment = ""
                index += 1
                continue
            if char == "}":
                if depth == 1 and re.fullmatch(r"[A-Za-z_]\w*", segment.strip()):
                    return True
                depth -= 1
                if depth == 1:
                    segment = ""
                index += 1
                continue
            if depth == 1 and char == ";":
                if re.fullmatch(r"[A-Za-z_]\w*", segment.strip()):
                    return True
                segment = ""
                index += 1
                continue
            if depth == 1 and char == ":" and not line[index:index + 2] == "::":
                if segment.strip() in ("public", "protected", "private"):
                    segment = ""
                else:
                    segment += char
                index += 1
                continue
            if depth == 1 and (char.isalpha() or char == "_"):
                match = _CALL.match(line, index)
                if match:
                    end = match.end()
                    after = end
                    while after < len(line) and line[after].isspace():
                        after += 1
                    if after < len(line) and line[after] == "(":
                        name = match.group(0)
                        prefix = segment.strip()
                        if name == "static_assert":
                            pass
                        elif name == owner and prefix in ("", "explicit", "constexpr", "inline", "explicit constexpr"):
                            pass
                        elif name == owner and re.fullmatch(r"(?:virtual\s+)?~", prefix):
                            pass
                        elif not prefix or prefix in ("virtual", "static", "inline", "friend", "explicit"):
                            return True
                    segment += line[index:end]
                    index = end
                    continue
            if depth == 1:
                segment += char
            index += 1
    return False


def parse_generated_members(text: str, path: str, pin: str, *, address_space: str):
    from .source.reader import _role_lexical_state_after

    relative_path(path)
    lines = text.splitlines()
    locations, in_block, raw = [], False, None
    for index, line in enumerate(lines):
        if line.lstrip().startswith(PREFIX):
            if in_block or raw is not None:
                raise ValueError(f"generated member annotation must be an active line comment at {path}:{index + 1}")
            locations.append((index, in_block))
        in_block, raw, saw_raw = _role_lexical_state_after(line, in_block, raw)
        if line.lstrip().startswith(PREFIX) and saw_raw:
            raise ValueError(f"generated member annotation cannot occur in a raw string at {path}:{index + 1}")
    if locations and any(re.search(r"(?:\\|\?\?/)$", line.rstrip("\r\n")) for line in lines):
        raise ValueError("line-spliced source is unsupported for generated member declarations")
    bindings = []
    for index, in_block in locations:
        match = PATTERN.fullmatch(lines[index].strip())
        if match is None:
            raise ValueError(f"invalid generated member binding at {path}:{index + 1}")
        kind, start, end, symbol, evidence = match.groups()
        owner = _symbol_owner(symbol, kind)
        if "::" in owner:
            raise ValueError("namespaced generated member class declarations are unsupported")
        declared_owner, explicit_destructor = _anchor_class(lines, index, in_block)
        if owner != declared_owner:
            raise ValueError("generated member symbol owner disagrees with anchored class")
        if kind == "complete_destructor" and explicit_destructor:
            raise ValueError("complete destructor binding requires an implicit destructor")
        origin = Origin(pin, address_space, int(start, 16))
        bindings.append(GeneratedMemberBinding(kind, owner, origin, Span(origin.address, int(end, 16)),
                          symbol, Origin(pin, address_space, int(evidence, 16)), path, index + 1))
    return tuple(bindings)


def read_generated_members(root: Path, pin: str, *, contents=None):
    root = root.resolve()
    configuration = SourceConfiguration.read(root)
    bindings = []
    for path in configuration.files(root):
        relative = path.relative_to(root).as_posix()
        if configuration.is_seed(relative):
            continue
        raw = (contents or {}).get(relative)
        text = path.read_text(encoding="utf-8") if raw is None else raw.decode("utf-8")
        bindings.extend(parse_generated_members(text, relative, pin, address_space=configuration.address_space))
    symbols = [binding.symbol for binding in bindings]
    if len(set(symbols)) != len(symbols):
        raise ValueError("duplicate generated member symbol binding")
    owners = {}
    for binding in bindings:
        owners.setdefault(binding.owner, set()).add(binding.unit)
    if any(len(paths) != 1 for paths in owners.values()):
        raise ValueError("generated member class owner has multiple source declarations")
    return tuple(bindings)


def validate_generated_members(bindings, view):
    entities = {item["entity_id"]: item for group in view["groups"] for item in group["entities"]}
    for binding in bindings:
        evidence = entities.get(binding.evidence.entity_id)
        if evidence is None or evidence["conflicts"]:
            raise ValueError("generated member evidence has no unambiguous mapped constructor")
        constructors = [candidate for candidate in evidence["candidates"]
                        if not candidate["seed"] and not candidate["unbound"]
                        and candidate["symbol"].startswith("??0")
                        and qualified_name(candidate["symbol"]).rsplit("::", 1)[0] == binding.owner]
        if len(constructors) != 1:
            raise ValueError("generated member evidence is not one mapped constructor of the same class")

