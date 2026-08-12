#!/usr/bin/env python3
"""File-scope declarations `src/` already carries, for the scaffolding preamble.

WHY THIS EXISTS. `byte_match_census.build_unit` compiles a recovered body in
generated scaffolding: the classes and globals the CATALOGUE knows about, and
nothing else. A body that calls a helper its own translation unit defines -
`forward_sound_device`, `store32`, `query_wrapped_device` - therefore does not
compile, and the piece is scored NO_COMPILE, which is not a verdict about the
recovery at all.

Measured on the live map before this module existed: **638 of the 2,925
hand-written pieces** were NO_COMPILE with `C2065: undeclared identifier`
naming ONLY things `src/` itself declares - 21.8% of the corpus, invisible to
the recovery state for want of a prototype. `COMPAT_DECLARATIONS` covered four
names by hand.

So the preamble is DERIVED from the tree instead of listed. `src/` is the
single record of the decompilation (docs/DECOMP_MAP.md); if a body names
something, the tree usually declares it, and this reads that declaration back
out.

WHY IT CANNOT BREAK A UNIT THAT COMPILES TODAY. A declaration is emitted only
when the body names an identifier that the scaffolding does NOT already
declare. A unit that compiles today has, by definition, a declaration for
every name it uses - so no candidate survives that test and its text is
unchanged, byte for byte, which also keeps its verdict cache entry valid. The
only units whose text moves are ones that were already failing.

WHAT IT REFUSES. Everything it cannot turn into a standalone declaration that
VC6 will accept with no headers:

  * a by-value parameter or return of a class type, which needs the full
    definition rather than a forward declaration;
  * a name declared more than once with different text, because choosing is
    guessing;
  * anything inside a class body, a function body, or a namespace - member
    names are the HEADER's job (an opaque shell is a layout defect, and
    papering over it here would hide it);
  * a declaration whose types it cannot classify.

A refusal costs one NO_COMPILE that was already NO_COMPILE. A wrong
declaration would change what is being measured, which is the one outcome that
matters here.
"""

from __future__ import annotations

import functools
import re
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
SRC = REPO_ROOT / "src"

# Spellings that are types in their own right and need no declaration.
SCALARS = frozenset({
    "void", "char", "short", "int", "long", "float", "double", "bool",
    "signed", "unsigned", "const", "volatile", "struct", "class", "enum",
    # `typedef` reads as an identifier to a regex, and leaving it out made
    # every function-pointer typedef require a type called "typedef" -
    # refusing the typedef and, with it, every declaration that names one.
    "union", "static", "extern", "inline", "typedef", "__cdecl", "__stdcall",
    "__fastcall", "__thiscall", "__declspec", "dllexport", "dllimport",
    "int8_t", "int16_t", "int32_t", "int64_t", "uint8_t", "uint16_t",
    "uint32_t", "uint64_t", "size_t", "intptr_t", "uintptr_t", "ptrdiff_t",
    "wchar_t", "int8", "int16", "int32", "int64", "uint8", "uint16",
    "uint32", "uint64", "uint",
})

# Reserved words that can open a file-scope statement but never name one.
NOT_A_DECLARATION = frozenset({
    "return", "if", "else", "for", "while", "do", "switch", "case",
    "default", "break", "continue", "goto", "sizeof", "new", "delete",
    "throw", "try", "catch", "using", "namespace", "template", "typename",
    "operator", "public", "private", "protected", "friend", "virtual",
    "explicit", "mutable", "register", "asm", "__asm",
})

IDENTIFIER = re.compile(r"[A-Za-z_]\w*")
TYPEDEF_SIMPLE = re.compile(r"^typedef\s+.*?\b(?P<name>\w+)\s*(?:\[[^\]]*\])?;$", re.S)
# `typedef void (__cdecl *MenuProc)(int);` - the name is inside the parens.
TYPEDEF_FUNCPTR = re.compile(
    r"^typedef\s+.*?\(\s*(?:__\w+\s+)?\*+\s*(?P<name>\w+)\s*\)\s*\(.*\)\s*;$", re.S)
# `typedef void (OriginalObject::*func_menu_repaint)();` - a member pointer.
TYPEDEF_MEMBERPTR = re.compile(
    r"^typedef\s+.*?\(\s*\w+::\s*\*+\s*(?P<name>\w+)\s*\)\s*\(.*\)\s*;$", re.S)
TYPE_HEAD = re.compile(r"^(?:class|struct|union)\s+(?:DLLEXPORT\s+)?(?P<name>\w+)\b")
ENUM_HEAD = re.compile(r"^enum\s+(?P<name>\w+)?\s*\{")
# `int __cdecl foo(int a, char *b)` / `Buffer *bar(void)` - a function.
FUNCTION = re.compile(
    r"^(?P<head>[\w:\*&<>,\s]*?)\b(?P<name>\w+)\s*\((?P<params>[^;{]*)\)"
    r"\s*(?:const\s*)?$", re.S)
# `extern Console *ConsoleState;` / `uint32_t Foo[4];`
VARIABLE = re.compile(
    r"^(?P<head>[\w:\*&<>,\s]*?)\b(?P<name>\w+)\s*(?P<array>(?:\[[^\]]*\])*)\s*$")


# A block whose contents are still FILE SCOPE. `src/sound.cpp` puts its
# helpers - `forward_sound_device` and the rest - inside an anonymous
# namespace, which is how a translation unit spells "static" in this tree.
# Skipping those bodies is why the first version of this module indexed 26
# names from that file and none of the four the bodies actually call.
TRANSPARENT_BLOCK = re.compile(r'^(?:namespace\b|extern\s+"C(?:\+\+)?"\s*$)')


def _statements(text: str):
    """(file-scope statement, opened_a_block) for a whole file.

    Comments and preprocessor lines are dropped; a statement runs to its `;`
    or to the `{` that opens a body. A class or function body is skipped
    whole - which is what makes a class member impossible to mistake for a
    global - while a `namespace` or `extern "C"` block is TRANSPARENT, because
    what it contains is file scope by any definition that matters here.
    """
    text = re.sub(r"/\*.*?\*/", " ", text, flags=re.S)
    # depth of nested OPAQUE blocks; zero means the scanner is at file scope.
    opaque, stack, pending = 0, [], []
    for raw in text.splitlines():
        line = re.sub(r"//.*$", "", raw).strip()
        if not line or line.startswith("#"):
            continue
        index = 0
        while index < len(line):
            char = line[index]
            if opaque:
                if char == "{":
                    opaque += 1
                elif char == "}":
                    opaque -= 1
                index += 1
                continue
            if char == "{":
                statement = " ".join(pending).strip()
                pending = []
                index += 1
                if TRANSPARENT_BLOCK.match(statement):
                    stack.append("transparent")
                    continue
                stack.append("opaque")
                opaque = 1
                if statement:
                    yield statement, True
                continue
            if char == "}":
                if stack:
                    stack.pop()
                pending = []
                index += 1
                continue
            if char == ";":
                statement = " ".join(pending).strip()
                pending = []
                if statement:
                    yield statement, False
                index += 1
                continue
            end = index
            while end < len(line) and line[end] not in "{};":
                end += 1
            pending.append(line[index:end])
            index = end
        pending.append(" ")


def _named_types(text: str) -> set:
    """Identifiers in a type expression that are not built in."""
    return {name for name in IDENTIFIER.findall(text)
            if name not in SCALARS and name not in NOT_A_DECLARATION}


# `int vtable_offset` -> `int`. THE PARAMETER NAME IS NOT A TYPE, and reading
# it as one is not a cosmetic error: every declaration whose parameters are
# named was refused, because the "types" it appeared to need - `a1`,
# `vtable_offset` - are declared nowhere. That was the whole of `src/sound.cpp`
# on the first run.
PARAM_NAME = re.compile(r"\b[A-Za-z_]\w*\s*(?:\[[^\]]*\])*\s*$")


def _param_type(part: str) -> str:
    """The TYPE of one parameter, with its name and array bound removed."""
    part = re.sub(r"=\s*[^,]+$", " ", part).strip()
    if not part or part in ("void", "..."):
        return ""
    if part.endswith(("*", "&")) or part.split()[-1] in SCALARS:
        return part                      # unnamed: `int`, `char *`
    return PARAM_NAME.sub(" ", part).strip() or part


def _parameter_types(params: str) -> list:
    return [t for t in (_param_type(p) for p in params.split(",")) if t]


def _by_value_types(params: str) -> set:
    """Type names a parameter list holds BY VALUE - the ones needing a body."""
    wanted = set()
    for spelling in _parameter_types(params):
        if "*" in spelling or "&" in spelling:
            continue
        wanted |= _named_types(spelling)
    return wanted


class Declaration:
    """One file-scope name, and the text that declares it standalone."""

    __slots__ = ("name", "text", "kind", "types", "by_value", "origin")

    def __init__(self, name, text, kind, types, by_value, origin):
        self.name = name
        self.text = text
        self.kind = kind          # typedef | type | variable | function
        self.types = types        # every type name the declaration mentions
        self.by_value = by_value  # those it holds by value, needing a body
        self.origin = origin

    def __repr__(self):  # pragma: no cover - debugging aid
        return f"<Declaration {self.name} {self.kind} from {self.origin}>"


def _declaration_for(statement: str, opened_block: bool, origin: str):
    """A Declaration for one file-scope statement, or None."""
    statement = statement.strip()
    if not statement:
        return None
    first = IDENTIFIER.match(statement)
    if first and first.group(0) in NOT_A_DECLARATION:
        return None

    if statement.startswith("typedef"):
        if opened_block:
            # `typedef struct { ... } Name;` - the body was skipped, so the
            # name never reached here and the width is not stated.
            return None
        body = statement + ";"
        hit = (TYPEDEF_FUNCPTR.match(body) or TYPEDEF_MEMBERPTR.match(body)
               or TYPEDEF_SIMPLE.match(body))
        if not hit:
            return None
        name = hit.group("name")
        return Declaration(name, body, "typedef",
                           _named_types(body) - {name}, set(), origin)

    head = TYPE_HEAD.match(statement)
    if head:
        # A class or struct. Only the FORWARD declaration is offered: the
        # definition is the emitter's business and its layout is gated.
        name = head.group("name")
        keyword = "struct" if statement.startswith("struct") else "class"
        return Declaration(name, f"{keyword} {name};", "type", set(), set(),
                           origin)
    if ENUM_HEAD.match(statement) or statement.startswith("enum"):
        return None

    if opened_block or statement.endswith(")"):
        function = FUNCTION.match(statement)
        if function:
            head_text = function.group("head")
            name = function.group("name")
            params = function.group("params")
            if "::" in head_text or "::" in name or "operator" in statement:
                return None
            if not head_text.strip():
                return None       # a call, not a declaration
            # `static` would promise a definition this unit does not have.
            head_text = re.sub(r"\b(?:static|inline|DLLEXPORT)\b", " ",
                               head_text).strip()
            if not head_text:
                return None
            text = f"{head_text} {name}({params});"
            # A return type held BY VALUE needs the definition just as much as
            # a by-value parameter does.
            returns_by_value = set() if ("*" in head_text or "&" in head_text) \
                else _named_types(head_text)
            mentioned = _named_types(head_text)
            for spelling in _parameter_types(params):
                mentioned |= _named_types(spelling)
            return Declaration(name, text, "function", mentioned,
                               _by_value_types(params) | returns_by_value,
                               origin)
        return None

    # A variable. `Type name;`, `Type name = init;`, `Type name[4];`
    body = statement.split("=", 1)[0].strip()
    variable = VARIABLE.match(body)
    if not variable:
        return None
    head_text = variable.group("head")
    name = variable.group("name")
    if not head_text.strip() or "::" in head_text or "(" in body:
        return None
    if name in SCALARS:
        return None
    array = variable.group("array") or ""
    if "[" in array and not re.fullmatch(r"(?:\[\s*\d+\s*\]|\[\s*0x[0-9A-Fa-f]+\s*\])+", array):
        return None
    head_text = re.sub(r"\b(?:static|extern|DLLEXPORT)\b", " ", head_text).strip()
    if not head_text:
        return None
    by_value = set() if "*" in head_text or "&" in head_text \
        else _named_types(head_text) - {"const", "volatile"}
    return Declaration(name, f"extern {head_text} {name}{array};", "variable",
                       _named_types(head_text), by_value, origin)


def _index_text(text: str, origin: str, out: dict, rejected: set) -> None:
    for statement, opened_block in _statements(text):
        declaration = _declaration_for(statement, opened_block, origin)
        if declaration is None:
            continue
        name = declaration.name
        existing = out.get(name)
        if existing is None:
            out[name] = declaration
        elif existing.text != declaration.text:
            # Two different declarations of one name. `class Foo;` beside the
            # real definition is the same claim; anything else is ambiguous
            # and choosing between them would be a guess.
            if {existing.kind, declaration.kind} == {"type"}:
                continue
            if existing.kind == "type" or declaration.kind == "type":
                out[name] = existing if existing.kind == "type" else declaration
                continue
            rejected.add(name)
            out.pop(name, None)


@functools.lru_cache(maxsize=8)
def index(src: Path = SRC) -> dict:
    """{name: Declaration} for every file-scope name `src/*.h` declares.

    Headers only. A `.cpp`'s own file-scope helpers are added per unit by
    `for_body`, because a static in one file must not be offered to a body in
    another - two files may spell the same helper differently, and the
    ambiguity would be resolved silently and wrongly.
    """
    out, rejected = {}, set()
    for header in sorted(Path(src).glob("*.h")):
        _index_text(header.read_text(errors="ignore"), header.name, out,
                    rejected)
    for name in rejected:
        out.pop(name, None)
    return out


@functools.lru_cache(maxsize=256)
def file_scope(path: Path) -> dict:
    """{name: Declaration} for one `.cpp`'s own file-scope declarations."""
    try:
        text = Path(path).read_text(errors="ignore")
    except OSError:
        return {}
    out, rejected = {}, set()
    _index_text(text, Path(path).name, out, rejected)
    for name in rejected:
        out.pop(name, None)
    return out


def _present(name: str, text: str) -> bool:
    return re.search(rf"\b{re.escape(name)}\b", text) is not None


def for_body(body: str, scaffolding: str, source_path=None,
             src: Path = SRC) -> str:
    """Declarations `body` needs that `scaffolding` does not already carry.

    The ORDER is forward type declarations, then typedefs, then variables and
    functions, so a prototype never precedes the type it names.
    """
    known = dict(index(src))
    if source_path is not None:
        known.update(file_scope(Path(source_path)))

    wanted, seen = [], set()

    def take(name: str, depth: int = 0) -> bool:
        """Queue `name` and everything it needs; False when it cannot be."""
        if name in seen:
            return True
        declaration = known.get(name)
        if declaration is None or depth > 3:
            return False
        seen.add(name)
        for required in sorted(declaration.by_value):
            # A by-value class parameter needs the definition, which only the
            # emitter can supply. Refuse rather than emit a unit that says
            # `C2079: uses undefined class`.
            if required in SCALARS or _present(required, scaffolding):
                continue
            other = known.get(required)
            if other is None or other.kind != "typedef":
                seen.discard(name)
                return False
            if not take(required, depth + 1):
                seen.discard(name)
                return False
        for required in sorted(declaration.types - declaration.by_value):
            if required in SCALARS or _present(required, scaffolding):
                continue
            other = known.get(required)
            if other is None:
                seen.discard(name)
                return False
            take(required, depth + 1)
        wanted.append(declaration)
        return True

    for name in sorted(set(IDENTIFIER.findall(body))):
        if name in SCALARS or name in NOT_A_DECLARATION:
            continue
        if _present(name, scaffolding):
            continue
        take(name)

    rank = {"type": 0, "typedef": 1, "variable": 2, "function": 3}
    ordered, emitted = [], set()
    for declaration in sorted(wanted, key=lambda d: (rank[d.kind], d.name)):
        if declaration.text in emitted:
            continue
        emitted.add(declaration.text)
        ordered.append(declaration.text)
    return ("\n".join(ordered) + "\n") if ordered else ""
