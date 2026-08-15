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
# `typedef int(__cdecl func_game_atexit)(func_atexit_callback *);` - a
# FUNCTION TYPE, with no star. Neither pattern above matches it: the name is
# inside parentheses so `TYPEDEF_SIMPLE` never sees it before the `;`, and
# `TYPEDEF_FUNCPTR` requires the star. The typedef therefore went unindexed,
# and every declaration whose type named one was dropped whole rather than
# emitted - `extern func_game_atexit *GameAtexit;` among them, which is the
# seam every generated atexit thunk calls through. 388 already-written bodies
# could not compile for want of these two lines, the single largest entry in
# the emitter's defect histogram.
TYPEDEF_FUNCTYPE = re.compile(
    r"^typedef\s+.*?\(\s*(?:__\w+\s+)?(?P<name>\w+)\s*\)\s*\(.*\)\s*;$", re.S)
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


# `LPSTR UNUSED(input) input` - `src/stdafx.h` defines UNUSED(x) as nothing,
# so it is not part of the declaration at all. Left in, the parameter appears
# to name a type called `UNUSED` and one called `input`, neither of which is
# declared anywhere, and the whole function is dropped: `rnd` is the one that
# shows up most, on 22 bodies.
UNUSED_MACRO = re.compile(r"\bUNUSED\s*\(\s*\w*\s*\)")


def _declaration_for(statement: str, opened_block: bool, origin: str):
    """A Declaration for one file-scope statement, or None."""
    statement = UNUSED_MACRO.sub(" ", statement).strip()
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
               or TYPEDEF_FUNCTYPE.match(body) or TYPEDEF_SIMPLE.match(body))
        if not hit:
            return None
        name = hit.group("name")
        # THE PARAMETER NAMES OF A FUNCTION TYPEDEF ARE NOT TYPES, exactly as
        # they are not for a function - this file already records that lesson
        # above `PARAM_NAME` and applied it only to `FUNCTION`. Reading every
        # identifier here made `func_game_atexit` claim to need a type called
        # `callback`, which is declared nowhere, so the typedef was dropped and
        # `GameAtexit` with it. Named parameters are the norm in this tree's
        # headers, so this cost every function typedef that has one.
        arguments = re.search(r"\)\s*\((?P<params>.*)\)\s*;$", body, re.S)
        if arguments:
            head = body[:arguments.start()]
            types = _named_types(head)
            for spelling in _parameter_types(arguments.group("params")):
                types |= _named_types(spelling)
        else:
            types = _named_types(body)
        return Declaration(name, body, "typedef", types - {name}, set(),
                           origin)

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


def _signature_params(text: str) -> str:
    """The parameter TYPES of a declaration, as a comparable string."""
    match = FUNCTION.match(text.rstrip(";").strip())
    if not match:
        return ""
    return "|".join(_parameter_types(match.group("params")))


def _merge_overloads(existing, declaration):
    """One Declaration carrying BOTH, or None when they truly conflict.

    AN OVERLOAD IS NOT AN AMBIGUITY. `has_fac_built` is declared twice in
    `src/base.h`, once taking a facility and once taking a facility and a
    base, which is ordinary C++ and exactly what the image has. The indexer
    treated any second spelling of a name as a guess it refused to make, so
    the name was dropped entirely and every body calling either overload
    failed `C2065` - 31 bodies on this name alone, 12 more on `vector_dist`.

    The test is the PARAMETER TYPES. Different lists are overloads and both
    declarations are emitted, which is what the header itself does. The same
    list with a different return or convention is a real disagreement between
    two headers, and that is still refused rather than resolved by a coin
    flip - emitting both would be `C2556` and would take down a unit that
    compiles today.
    """
    if existing.kind != "function" or declaration.kind != "function":
        return None
    if _signature_params(existing.text) == _signature_params(declaration.text):
        return None
    texts = sorted({existing.text.rstrip(";") + ";",
                    declaration.text.rstrip(";") + ";"})
    return Declaration(existing.name, "\n".join(texts), "function",
                       existing.types | declaration.types,
                       existing.by_value | declaration.by_value,
                       existing.origin)


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
            merged = _merge_overloads(existing, declaration)
            if merged is not None:
                out[name] = merged
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


COMMENT_OR_STRING = re.compile(
    r'//[^\n]*'                      # line comment
    r'|/\*.*?\*/'                    # block comment
    r'|"(?:\\.|[^"\\])*"'            # string literal
    r"|'(?:\\.|[^'\\])*'",           # character literal
    re.S)


@functools.lru_cache(maxsize=8)
def _code_only(text: str) -> str:
    """`text` with its comments blanked out.

    A NAME IN A COMMENT IS NOT A DECLARATION, and `_present` used to count it
    as one. `src/original_seam.h` documents itself with

        typedef void(__thiscall func_buffer_line)(Buffer *, int, int, int, int);

    inside a `/* ... */`, and the day that header started being spliced into
    the unit, `_present("Buffer", scaffolding)` answered yes for every unit in
    the tree. `GraphicWin`'s definition was then emitted with `Buffer buffer_;`
    and no `Buffer` anywhere - `C2079: uses undefined class` - which took
    `??1TextureStore@@QAE@XZ` from BYTE_EXACT to NO_COMPILE. The ratchet
    caught it; nothing else would have.

    ONE LEFT-TO-RIGHT PASS, because the forms are mutually exclusive and a
    per-form pass is not. Stripping `/*...*/` first and `//...` second reads
    a `/*` that only appears INSIDE a line comment as a real comment opener
    and swallows everything up to the next `*/` - real declarations included.
    This is not hypothetical: the scaffolding for 0x00483E80 holds 8 `/*`
    against 6 `*/`, so the naive order deleted live code and the unit gained
    a SECOND definition of six classes, `C2011: type redefinition`. String and
    character literals join the alternation for the same reason - a `//`
    inside one opens no comment.
    """
    return COMMENT_OR_STRING.sub(
        lambda hit: "" if hit.group()[0] == "/" else hit.group(), text)


def _present(name: str, text: str) -> bool:
    return re.search(rf"\b{re.escape(name)}\b", _code_only(text)) is not None


@functools.lru_cache(maxsize=1)
def _class_keys() -> dict:
    """{class: 'struct'|'class'} - the emitter's own choice, not a second one.

    `U` and `V` mangle differently, so a definition emitted here has to use
    the same key the emitter and the target object use, or a function taking
    the type as a parameter decorates to a symbol no target object holds.
    """
    try:
        import emit_translation_unit as emit
        return emit.class_keys(emit.load_functions())
    except Exception:                      # no catalogue: fall back to struct
        return {}


@functools.lru_cache(maxsize=1)
def _windows_typedefs() -> dict:
    """Win32 spellings that are a primitive, not a struct."""
    try:
        import emit_translation_unit as emit
        return dict(emit.NOT_A_STRUCT)
    except Exception:
        return {}


BY_VALUE_MEMBER = re.compile(r"^\s*(?:const\s+)?([A-Za-z_]\w*)\s+\w+\s*(?:\[[^\]]*\])?\s*;\s*$")
POINTER_MEMBER = re.compile(r"^\s*(?:const\s+)?([A-Za-z_]\w*)\s*\*+")


def _member_types(lines) -> tuple:
    """({by-value type}, {pointed-at type}) over a class's member lines.

    BOTH are needed, and only counting the first is how
    `??1TextureStore@@QAE@XZ` - byte-exact for weeks - became NO_COMPILE the
    moment definitions started being emitted: `Buffer` holds
    `const BITMAPINFO *bitmap_info_;`, a POINTER, so nothing asked for
    `BITMAPINFO` and VC6 stopped at `syntax error : missing ';' before '*'`.
    A pointer needs the name DECLARED even though it does not need the type
    complete.
    """
    by_value, pointed = set(), set()
    for line in lines:
        stripped = line.strip()
        if "(" in stripped:
            continue
        if "*" in stripped:
            hit = POINTER_MEMBER.match(stripped)
            if hit and hit.group(1) not in SCALARS:
                pointed.add(hit.group(1))
            continue
        hit = BY_VALUE_MEMBER.match(stripped)
        if hit and hit.group(1) not in SCALARS:
            by_value.add(hit.group(1))
    return by_value, pointed


def _definition_lines(name: str, scaffolding: str, provided: set = None,
                      depth: int = 0):
    """`struct X { ... };` for a class `src/` has a VERIFIED layout for.

    Returns the lines its dependencies need FIRST, then the definition, or
    None when the layout is not supplyable or names a type this cannot put in
    the same unit. A forward declaration is the fallback, and it is a correct
    one: it costs the unit `C2036 unknown size` where it already had
    `C2065 undeclared identifier`, which is the same NO_COMPILE.

    THE GATE IS `class_layouts.supplyable`, which is `verified-layouts.txt`
    plus the recursive by-value closure. Nothing unverified is written into a
    unit here - a layout the tree has not proved against the real size would
    move every offset after it and the body would still compile.
    """
    import class_layouts

    if provided is None:
        provided = set()
    if depth > 4 or not class_layouts.supplyable(name):
        return None
    members = class_layouts.declaration_for(name)
    if not members:
        return None

    # `provided` is what this PREAMBLE has already written, and it is not the
    # same question as what the scaffolding wrote. Without it `RECT` was
    # defined once as Buffer's dependency and again as Win's, and the unit
    # died on `C2011: 'RECT' : 'struct' type redefinition` - a unit that had
    # compiled before definitions were emitted at all.
    before, windows = [], _windows_typedefs()
    by_value, pointed = _member_types(members)
    for required in sorted(by_value | pointed):
        if _present(required, scaffolding) or required in provided:
            continue
        if required in windows:
            provided.add(required)
            before.append(f"typedef {windows[required]} {required};")
            continue
        if required in class_layouts.WINDOWS_STRUCT:
            provided.add(required)
            nested = class_layouts.declaration_for(required)
            before.append(f"struct {required} {{")
            before.extend(nested)
            before.append("};")
            continue
        if required in pointed and required not in by_value:
            # A pointer needs the NAME, not the layout. Forward declarations
            # may legally repeat, but tracking them keeps the unit readable.
            provided.add(required)
            before.append(f"{_class_keys().get(required, 'struct')} "
                          f"{required};")
            continue
        deeper = _definition_lines(required, scaffolding, provided, depth + 1)
        if deeper is None:
            return None
        before.extend(deeper)

    # THE METHODS `src/` DECLARES, not only the layout. A definition with the
    # right fields and no methods is `C2039: is not a member` the moment the
    # body calls one, and that was 240 already-written bodies - 209 of them on
    # `Sprite::close` and `CaviarData::close`, two one-line declarations that
    # have been sitting in `src/sprite.h` and `src/caviar.h` the whole time.
    # A declaration is never inlined and occupies no bytes, so importing one
    # cannot move the comparison; only its absence can.
    methods = []
    for text, _, types in class_layouts.methods_of(name):
        for required in sorted(types):
            if _present(required, scaffolding) or required in provided:
                continue
            provided.add(required)
            if required in windows:
                before.append(f"typedef {windows[required]} {required};")
            else:
                # A NAME is all a parameter needs, even by value: C++ requires
                # a complete type at the call and at the definition, neither
                # of which this declaration is.
                before.append(f"{_class_keys().get(required, 'struct')} "
                              f"{required};")
        methods.append(text)

    provided.add(name)
    key = _class_keys().get(name, "struct")
    head = f"{key} {name} {{ public:" if key == "class" else f"{key} {name} {{"
    return before + [head] + list(members) + methods + ["};"]



def code_only(text: str) -> str:
    """`text` with comments and string literals blanked out.

    THE SCAFFOLDER READ TYPE NAMES OUT OF COMMENTS. `for_body` collects every
    identifier in the body and supplies a declaration for each one `src/`
    knows, and it collected them from the RAW text - so a comment that
    mentions a class pulled that class's full definition into the unit.

    Found on 0x006077F0 (`BaseButton::on_key_click`) when its annotation
    gained the note "the sibling class BasePop declares the same handler":
    the word `BasePop` in prose grew the unit from 1,972 lines to 2,386 and
    broke it with `C2086: 'heap_' : redefinition`. Three BYTE_EXACT claims
    went NO_COMPILE on a comment.

    Blanked rather than deleted so nothing shifts - every offset stays where
    it was, which keeps any position a caller reports honest. Same treatment
    `classes_defined_in` in the emitter and `code_only` in
    `verify_recovered_function` already apply, for the same reason.
    """
    def blank(match):
        return re.sub(r"\S", " ", match.group(0))

    # Block comments, then line comments, then literals: a `//` inside a
    # string is not a comment, and a quote inside a comment opens nothing.
    stripped = re.sub(r"/\*.*?\*/", blank, text, flags=re.S)
    stripped = re.sub(r"//[^\n]*", blank, stripped)
    return re.sub(r'"(?:\\.|[^"\\])*"' r"|'(?:\\.|[^'\\])*'", blank, stripped)


def for_body(body: str, scaffolding: str, source_path=None,
             src: Path = SRC) -> str:
    """Declarations `body` needs that `scaffolding` does not already carry.

    The ORDER IS TOPOLOGICAL, not by kind: a declaration is appended after
    everything it needs, because `Buffer` holds a `Spot` by value and a class
    definition cannot name a class declared below it. Sorting by kind put the
    typedefs a definition uses after the definition.

    A referenced CLASS is emitted as its full definition when
    `class_layouts.supplyable` says `src/` has a verified layout for it, and
    as a forward declaration otherwise. That is the join between header
    recovery and measurement: `Vehs[index].field_` needs `sizeof(Veh)`, so
    every class the tree learns to declare turns some NO_COMPILE into a
    verdict, and every class it cannot stays exactly as unmeasurable as it was.
    """
    known = dict(index(src))
    if source_path is not None:
        known.update(file_scope(Path(source_path)))

    wanted, seen = [], set()
    # Every type name this preamble has already written. One set for the whole
    # call, so a type two different classes depend on is defined once.
    provided = set()

    def take(name: str, depth: int = 0) -> bool:
        """Queue `name` and everything it needs; False when it cannot be."""
        if name in seen:
            return True
        declaration = known.get(name)
        if declaration is None or depth > 3:
            return False
        seen.add(name)
        if declaration.kind == "type":
            if name in provided:
                return True
            definition = _definition_lines(name, scaffolding, provided)
            if definition is not None:
                wanted.append(Declaration(name, "\n".join(definition), "type",
                                          set(), set(), declaration.origin))
                return True

        def supply(required: str, complete: bool) -> bool:
            """Get `required` into the unit ahead of `name`.

            `complete` is the difference between a member held BY VALUE, which
            needs the whole type, and one reached through a pointer, which a
            forward declaration satisfies.
            """
            if required in SCALARS or _present(required, scaffolding):
                return True
            if required in provided:
                return True
            windows = _windows_typedefs()
            if required in windows:
                provided.add(required)
                wanted.append(Declaration(
                    required, f"typedef {windows[required]} {required};",
                    "typedef", set(), set(), "NOT_A_STRUCT"))
                seen.add(required)
                return True
            definition = _definition_lines(required, scaffolding, provided)
            if definition is not None:
                seen.add(required)
                wanted.append(Declaration(required, "\n".join(definition),
                                          "type", set(), set(), "layout"))
                return True
            other = known.get(required)
            if other is None:
                return False
            if complete and other.kind not in ("typedef",):
                # A forward declaration cannot satisfy a by-value member, and
                # emitting one anyway trades C2065 for C2079 while putting a
                # claim in the preamble that is not true.
                return False
            return take(required, depth + 1)

        for required in sorted(declaration.by_value):
            if not supply(required, complete=True):
                seen.discard(name)
                return False
        for required in sorted(declaration.types - declaration.by_value):
            if not supply(required, complete=False):
                seen.discard(name)
                return False
        wanted.append(declaration)
        return True

    for name in sorted(set(IDENTIFIER.findall(code_only(body)))):
        if name in SCALARS or name in NOT_A_DECLARATION:
            continue
        if _present(name, scaffolding):
            continue
        take(name)

    ordered, emitted = [], set()
    for declaration in wanted:
        if declaration.text in emitted:
            continue
        emitted.add(declaration.text)
        ordered.append(declaration.text)
    return ("\n".join(ordered) + "\n") if ordered else ""
