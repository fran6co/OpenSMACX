"""Reading an MSVC mangled name far enough to tell two overloads apart.

Not a demangler. The one question here is ARITY - how many arguments a name
declares - because that is what distinguishes an overload set when nothing
else does:

    ?init@Filemap@@QAEHPADH@Z     the catalogue's spelling: 2 arguments
    ?init@Filemap@@QAEPAV1@PBDH@Z what CL emits:            2 arguments
    ?init@Filemap@@QAEHPAD@Z                                1 argument

The catalogue is wrong about the return type (`H` against `PAV1@`) AND the
constness (`PAD` against `PBD`), so neither the whole name nor the argument
list matches textually. The COUNT does, on all ten overload sets this tree
has, which is why arity and not equality is what this answers.

WHY THAT IS ENOUGH AND NO MORE. A fuller reader would have to model MSVC's
back-reference tables, and `tools/recovery_symbols.py` records what that
costs: it carries measured bugs about the return type taking no slot, about
a callback's arguments sharing the enclosing table, and about `PAV1@` being
a name back-reference inside a written-out type. Every one of those is a way
to be confidently wrong. Counting types needs the tokeniser but none of the
tables, so this stops there - and returns None wherever it is unsure, which
every caller treats as "cannot tell".
"""

from __future__ import annotations

import re

# One-character argument codes. MSVC never back-references these.
_PRIMITIVE = set("XDCEFGHIJKMNOZ")
# `P`/`Q`/`R`/`S` are pointers, `A`/`B` references; each is followed by a CV
# code and then the type pointed at. `U`/`V`/`T` open a user-defined type.
_INDIRECTION = set("PQRSAB")
_USER_DEFINED = set("UVT")
_FUNCTION_POINTER = "P6"

# `?name@Class@@` or `?name@@` - everything up to the first `@@`, which is
# where the qualifier chain closes. NOT the last: a `PAUSprite@@` argument
# carries one too.
_HEAD = re.compile(r"^\?{1,2}[^@]*@(?:[^@]+@)*@")

# After the head: an access code, then for a NONSTATIC member a CV code, then
# the calling convention. Statics and free functions carry no CV code, so
# reading three characters there would swallow the return type.
_NONSTATIC = set("ABEFIJMNQRUV")
_STATIC = set("CDKLST")


# What a name is BUILT AROUND, under whatever decoration it carries.
# `?name@Class@@...` is named by `name`; `??0Class@@...`, `??1` and friends
# have no name of their own and are named by the CLASS; `_WinMain@16` and
# `@thunk@16` are a plain identifier under stdcall or fastcall decoration.
_IDENTIFIER = re.compile(
    r"^\?\?(?:_[A-Za-z]|[0-9A-Za-z])(?P<ctor>[A-Za-z_]\w*)@"
    r"|^\?(?P<member>[A-Za-z_]\w*)@"
    r"|^[_@]?(?P<plain>[A-Za-z_]\w*?)(?:@\d+)?$")


def identifier(mangled: str) -> str:
    """The name a person would use for this piece, or "".

    A LOOKUP NEEDS THIS because nobody types a mangled name. `WinMain` is
    `_WinMain@16` in this map - stdcall decoration, where the leading
    underscore is not part of the name and a word-boundary match therefore
    misses it - and `Buffer::set_font` is
    `?set_font@Buffer@@QAEHPAVFont@@000@Z`.

    Returns "" rather than guessing when the shape is unrecognised, which
    every caller reads as "cannot tell".
    """
    match = _IDENTIFIER.match((mangled or "").strip())
    if match is None:
        return ""
    return (match.group("ctor") or match.group("member")
            or match.group("plain") or "")


def _end_of_type(text: str, index: int) -> int | None:
    """The index just past ONE encoded type, or None if unrecognised."""
    if index >= len(text):
        return None
    if text[index:index + 2] == _FUNCTION_POINTER:
        # `P6<conv><return><arguments>@Z`, closing `XZ` when there are none.
        after = _end_of_type(text, index + 3)
        if after is None:
            return None
        if text[after:after + 2] == "XZ":
            return after + 2
        while after < len(text) and text[after] != "@":
            after = _end_of_type(text, after)
            if after is None:
                return None
        return after + 2 if text[after:after + 2] == "@Z" else None
    while index < len(text) and text[index] in _INDIRECTION:
        if text[index:index + 2] == _FUNCTION_POINTER:
            return _end_of_type(text, index)          # a POINTER to one
        index += 1                                   # P, PA, PAP, AAV ...
        if index < len(text) and text[index] in "ABCD":
            index += 1                               # the CV code
    if index >= len(text):
        return None
    code = text[index]
    if code.isdigit():                               # already back-referenced
        return index + 1
    if code in _USER_DEFINED:
        if text[index + 1:index + 2].isdigit():
            # `PAV1@`: the type is written out but its NAME is a
            # back-reference, so it closes on ONE `@`.
            return index + 3 if text[index + 2:index + 3] == "@" else None
        close = text.find("@@", index)
        return close + 2 if close != -1 else None
    if code in _PRIMITIVE:
        return index + 1
    if code == "_":                                  # _J, _K, _N, _W
        return index + 2
    return None


def _types(text: str) -> list[str] | None:
    """`text` split into whole encoded types, or None if it does not."""
    out, index = [], 0
    while index < len(text):
        end = _end_of_type(text, index)
        if end is None:
            return None
        out.append(text[index:end])
        index = end
    return out


def arity(mangled: str) -> int | None:
    """How many arguments `mangled` declares, or None if it cannot be read.

    `X` alone is `void`, which is no arguments rather than one of them.
    """
    mangled = (mangled or "").strip()
    head = _HEAD.match(mangled)
    if head is None or not mangled.endswith("Z"):
        return None
    tail = mangled[head.end():]
    if not tail:
        return None
    access = tail[0]
    if access in _NONSTATIC:
        skip = 3                       # access, CV, convention
    elif access in _STATIC or access == "Y":
        skip = 2                       # access, convention
    else:
        return None
    body = tail[skip:-1]               # drop the trailing `Z`
    if body.endswith("@"):             # `@Z` closes an argument list
        body = body[:-1]
    types = _types(body)
    if types is None or not types:
        return None
    arguments = types[1:]              # the first is the RETURN type
    if arguments == ["X"]:
        return 0                       # `void` is no arguments
    return len(arguments)
