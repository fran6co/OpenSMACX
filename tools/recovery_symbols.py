#!/usr/bin/env python3
"""The one place that decides what symbol a recovered function carries.

Mizuchi's objdiff stage looks up ONE name on both sides - `settings.yaml`'s
`functionName` is passed to `findSymbol` against the compiled object and
against the synthesised target alike. So the name the recovered source makes
CL emit and the name `emit_target_object.py` writes into the COFF have to be
the same string, or the two objects never pair and the run reports
"Symbol ... not found" instead of a diff.

For the 4,821 catalogued `?`-mangled functions they already agree, and by
construction: the prototype was DECODED from the mangled name, so CL re-encodes
that same name from that same signature. Nothing here touches them.

The other 1,179 have no such loop, and that is the bug this module closes. A
catalogued name like `sub_5e3650` is a label out of a disassembler, not a
linker symbol - MSVC has no declaration that emits it, because C++ mangles and
`extern "C"` prepends. Written into the target object verbatim it pairs with
nothing: the source side emits `?sub_5e3650@@YGHH@Z` and the objects sit side
by side with no symbol in common. Every one of those functions was unmatchable
for that reason alone, before a single instruction was compared.

WHAT THE CATALOGUED NAME IS depends on where it came from, and the three cases
have to be told apart because they constrain the source differently:

  `?foo@Bar@@QAEHH@Z`   a real C++ symbol. Keep it; the C++ compiler
                        reproduces it from the decoded signature.
  `_build_tree`         a real C symbol, already carrying MSVC's decoration.
  `_WinMain@16`         The source must spell the UNDECORATED identifier and
  `@fastcall_thing@8`   let CL decorate it back.
  `sub_5e3650`          a disassembler label - no decoration, so not a symbol
  `PopMenu::delete1`    any linker ever saw. A name is chosen here and both
  `nullsub_27`          sides are made to carry it.

WHY THE SYMBOL IS COMPUTED, NEVER COPIED. For the second group the catalogued
decoration is evidence, not an instruction: `_WinMain@16` says __stdcall with
16 bytes of arguments, and if the recovered prototype says 8, CL emits
`_WinMain@8` no matter what the target object claims. Copying the catalogued
name would pair nothing; computing both sides from the signature we actually
emit always pairs, and `disagreements()` reports the conflict rather than
hiding it. Nothing here is ever linked - the oracle compiles with `/c` - so a
symbol's SPELLING reaches no comparison. Only its agreement across the two
objects does.
"""

from __future__ import annotations

import re
from typing import NamedTuple, Sequence

# MSVC's C decoration on x86: `__cdecl` takes a leading underscore, `__stdcall`
# adds the argument byte count, `__fastcall` swaps the leading underscore for
# `@`. This is the whole rule and it is symmetric with the parsing below.
CDECL, STDCALL, FASTCALL, THISCALL = (
    "__cdecl", "__stdcall", "__fastcall", "__thiscall")

MANGLED = re.compile(r"^\?")
DECORATED_FASTCALL = re.compile(r"^@([A-Za-z_]\w*)@(\d+)$")
DECORATED_STDCALL = re.compile(r"^_([A-Za-z_]\w*)@(\d+)$")
DECORATED_CDECL = re.compile(r"^_([A-Za-z_]\w*)$")
PLAIN_IDENTIFIER = re.compile(r"^[A-Za-z]\w*$")

# 8 bytes on the argument stack, not 4. Anything else - including every
# pointer, reference and enum - is one slot.
WIDE_TYPE = re.compile(r"\b(double|__int64|long\s+long)\b")

# Names CL will not let a unit define while `/Oi` is on (which `/O2` implies):
# `error C2169: 'strlen' : intrinsic function, cannot be defined`. The catalogue
# holds the statically-linked CRT, so these ARE catalogued functions and their
# recovered source has to define them. Measured over the whole catalogue: this
# is every name that raised C2169, and `/Oi-` clears all of them but
# `_abnormal_termination`, which is a compiler-generated SEH helper rather than
# a library function and stays uncompilable.
#
# `#pragma function(...)` does NOT clear it - tried, with the exact CRT
# signature and without; C2169 is raised either way.
INTRINSIC = frozenset({
    "abs", "acos", "atan", "cos", "memcmp", "memcpy", "memset", "pow", "sin",
    "sqrt", "strcat", "strcmp", "strcpy", "strlen",
})

# Names CL declares ITSELF, with no header included: `atexit` compiles in a
# bare unit, and a second `extern "C"` declaration of it is
# `error C2733: second C linkage of overloaded function 'atexit' not allowed`.
# So the emitter must leave it alone rather than declare it.
COMPILER_DECLARED = frozenset({"atexit"})

# A subject CL will not let this emitter define AT ALL, under any flags. Both
# are measured, and both are named here rather than left to fail as a compile
# error nobody reads:
#
#   atexit                  CL's own declaration is
#                           `int __cdecl atexit(void (__cdecl *)(void))`, and
#                           the emitter has no arity for the row, so it writes
#                           `atexit()` and collides (C2733). Defining it needs
#                           the real CRT signature, which is knowledge this
#                           tool does not carry.
#   _abnormal_termination   C2169 with `/Oi` AND with `/Oi-`: it is a compiler
#                           -generated SEH helper, not a library function.
#
# Refusing beats emitting: before, both compiled to a symbol no target object
# held, so an agent could work on either indefinitely and never pair.
UNDEFINABLE = {
    "atexit": "CL declares it itself and the catalogue has no arity for it "
              "(C2733)",
    "_abnormal_termination": "an SEH helper CL refuses to let a unit define, "
                             "under /Oi and /Oi- alike (C2169)",
}


class Spelling(NamedTuple):
    """What the catalogued name obliges the recovered source to say."""

    linkage: str           # 'c++' | 'c'
    identifier: str        # the name to write in source ('' when unspellable)
    convention: str        # forced by the decoration, or '' when free
    argument_bytes: int    # forced by the decoration, or -1 when free


def spelling(name: str) -> Spelling:
    """Read a catalogued name as a constraint on the source that recreates it.

    `_` is stripped exactly once, never repeatedly: zlib's `_tr_align` is
    catalogued `__tr_align` and the CRT's `_CxxFrameHandler` is catalogued
    `___CxxFrameHandler`, so a greedy strip would invent `tr_align` and emit a
    symbol one underscore short of every reference to it.
    """
    name = (name or "").strip()
    if MANGLED.match(name):
        return Spelling("c++", "", "", -1)

    found = DECORATED_FASTCALL.match(name)
    if found:
        return Spelling("c", found.group(1), FASTCALL, int(found.group(2)))
    found = DECORATED_STDCALL.match(name)
    if found:
        return Spelling("c", found.group(1), STDCALL, int(found.group(2)))
    found = DECORATED_CDECL.match(name)
    if found:
        return Spelling("c", found.group(1), CDECL, -1)
    if PLAIN_IDENTIFIER.match(name):
        # A disassembler label: undecorated, so no convention is implied.
        return Spelling("c", name, "", -1)
    # `PopMenu::delete1`, `j_??1Ambience@@QAE@XZ`, `_$I10_OUTPUT`. Not
    # spellable; the caller supplies an identifier (the emitter already
    # synthesises `fn_<address>` for exactly these).
    return Spelling("c", "", "", -1)


def argument_bytes(parameters: Sequence[str]) -> int:
    """Stack bytes MSVC counts into a __stdcall/__fastcall decoration."""
    total = 0
    for text in parameters:
        if "*" in text or "&" in text:
            total += 4                       # a pointer is a slot, `double *`
        elif WIDE_TYPE.search(text):         # included
            total += 8
        else:
            total += 4
    return total


def decorate(identifier: str, convention: str,
             parameters: Sequence[str] = ()) -> str:
    """The symbol CL emits for `extern "C" ... convention identifier(...)`."""
    if convention == STDCALL:
        return f"_{identifier}@{argument_bytes(parameters)}"
    if convention == FASTCALL:
        return f"@{identifier}@{argument_bytes(parameters)}"
    return f"_{identifier}"                  # __cdecl, and the default


def fallback_identifier(name: str, address: int) -> str:
    """An identifier for a catalogued name that is not one.

    Matches what `emit_translation_unit` already substitutes, so a function
    declared through the emitter's unsettled-callee path and the same function
    emitted as a target object land on the same symbol.
    """
    chosen = spelling(name).identifier
    return chosen or f"fn_{address:08x}"


def symbol_for(name: str, address: int, convention: str = CDECL,
               parameters: Sequence[str] = (),
               identifier: str = "") -> str:
    """The symbol both objects must carry for this catalogued function.

    `?`-mangled names are returned untouched: the C++ compiler reproduces them
    from the decoded signature, which is the loop this module exists to give
    everything else.
    """
    if MANGLED.match((name or "").strip()):
        return compress_backrefs(empty_destructor_arguments(name.strip()))
    return decorate(identifier or fallback_identifier(name, address),
                    convention, parameters)


# One-character argument codes. These are never back-referenced by MSVC.
PRIMITIVE = set("XDCEFGHIJKMNOZ")
# `P`/`Q`/`R`/`S` are pointers, `A`/`B` references; each is followed by a CV
# code and then the pointee. `U`/`V`/`T` open a user-defined type that runs to
# `@@`.
INDIRECTION = set("PQRSAB")
USER_DEFINED = set("UVT")


def _argument_tokens(text: str):
    """Split a mangled argument list into whole type tokens, or None.

    None means "not recognised", and every caller treats that as "change
    nothing". A tokeniser that guesses would produce a symbol that is wrong in
    a new way rather than leaving one that is wrong in a known way.
    """
    tokens, index = [], 0
    while index < len(text):
        start = index
        while index < len(text) and text[index] in INDIRECTION:
            index += 1                       # P, PA, PAP, AAV ...
            if index < len(text) and text[index] in "ABCD":
                index += 1                   # the CV code
        if index >= len(text):
            return None
        code = text[index]
        if code.isdigit():                   # already a back-reference
            index += 1
        elif code in USER_DEFINED:
            if index + 1 < len(text) and text[index + 1].isdigit():
                # `PAV1@`: the TYPE is written out but its NAME is a back-
                # reference, so it closes on ONE `@`. Requiring two made the
                # whole tokeniser return None for any name already in this
                # form - which is most of the catalogue's class parameters -
                # and every caller then changed nothing at all.
                index += 3
                if text[index - 1] != "@":
                    return None
            else:
                close = text.find("@@", index)
                if close == -1:
                    return None
                index = close + 2
        elif code in PRIMITIVE:
            index += 1
        elif code == "_":                    # _J, _K, _N, _W
            index += 2
        else:
            return None
        tokens.append(text[start:index])
    return tokens


DESTRUCTOR = re.compile(r"^\?\?1[A-Za-z_]\w*@@[A-Z]{3}@")


def empty_destructor_arguments(mangled: str) -> str:
    """A destructor's argument list, as C++ can actually spell it.

    `??1StringStruct@@QAE@H@Z` is catalogued with a parameter. A destructor
    has none, CL writes `@XZ`, and the target object held `@H@Z` - so the one
    row in this state could never pair however the body was written.
    """
    mangled = (mangled or "").strip()
    if not DESTRUCTOR.match(mangled) or not mangled.endswith("Z"):
        return mangled
    return f"{mangled[:DESTRUCTOR.match(mangled).end()]}XZ"


UDT_NAME = re.compile(r"([UVT])([A-Za-z_]\w*)@@")
# `fn_00483810` / `m_004483c0`: what the emitter mints when a
# catalogued name is not a C identifier.
SYNTHETIC_IDENTIFIER = re.compile(r"^(?:fn|m)_([0-9a-f]{8})$")


def substitute_name(mangled: str, identifier: str) -> str:
    """Re-point a mangled name at the identifier the source will actually use.

    14 catalogued names are not C identifiers at all - `??__Eg_BOOM_BUFFERS1`
    is a dynamic initialiser and `??__F...` an atexit thunk, both minted by
    the compiler for a global it constructs. The emitter already substitutes
    `fn_<address>` for them, because refusing cost 60+ rows and the spelling
    reaches no comparison. The TARGET object kept the original, so the two
    objects had no name in common and none of the 14 could ever pair.

    Only the name is replaced; the type encoding after the qualifier chain is
    the part that has to stay exact, and it does.
    """
    mangled = (mangled or "").strip()
    if not MANGLED.match(mangled) \
            or not SYNTHETIC_IDENTIFIER.match(identifier or ""):
        # ONLY a synthesised identifier, because only those were substituted.
        # Accepting any name rewrote every constructor and destructor: the
        # emitter calls a ctor's method `StringStruct`, and splicing that in
        # turned `??0StringStruct@@QAE@H@Z` into `?StringStruct@@QAE@H@Z`,
        # which is not a constructor and pairs with nothing. 51 rows that had
        # been pairing stopped, which is how it was caught.
        return mangled
    split = mangled.find("@@")
    if split == -1:
        return mangled
    qualifiers = mangled[1:split].split("@")
    if qualifiers and qualifiers[0] == identifier:
        return mangled
    qualifiers[0] = identifier
    return f"?{'@'.join(qualifiers)}{mangled[split:]}"


def _split_signature(mangled: str):
    """`(head, convention prefix, type list, terminator)`, or None.

    `?name@Class@@<qualifier+convention><return><args>@Z`. The qualifier chain
    closes at the FIRST `@@`, not the last: a `PAUSprite@@` ARGUMENT contains
    one too, and splitting on the last put the class qualifier inside the
    argument list.

    None means "not recognised", and every caller treats that as "change
    nothing".
    """
    mangled = (mangled or "").strip()
    if not MANGLED.match(mangled) or not mangled.endswith("Z"):
        return None
    split = mangled.find("@@")
    if split == -1:
        return None
    head, tail = mangled[:split + 2], mangled[split + 2:]
    prefix_length = 2 if tail.startswith("Y") else 3
    if len(tail) <= prefix_length + 1:
        return None
    body = tail[prefix_length:-1]            # drop the trailing `Z`
    terminator = ""
    if body.endswith("@"):                   # `@Z` closes an argument list
        body, terminator = body[:-1], "@"
    return head, tail[:prefix_length], body, terminator


def _name_table(head: str) -> list:
    """The identifiers already written when the type list starts.

    Slot 0 is the function's own name and slot 1 its class, which is why
    `?update@GraphicWin@@QAEXHHHHPAU1@@Z` says `1`.
    """
    return [part for part in head[1:-2].split("@") if part]


# `U`/`V` in a TYPE: `PAUSprite@@` written out, `PAV1@` back-referenced.
CLASS_KEY = re.compile(r"([UV])(?:([A-Za-z_]\w*)@@|(\d)@)")
KEY_CHARACTER = {"struct": "U", "class": "V"}


def _rewrite_class_keys(mangled: str, keys: dict):
    """`(name with the keys rewritten, class names used as types)`.

    The scan is per TOKEN and never over the whole name: an access qualifier
    is a bare run of letters, so `?f@X@@UAEXPAUFoo@@@Z` offers `U` +
    `AEXPAUFoo` + `@@` to the same pattern and it matches - swallowing the
    real use and inventing a class called `AEXPAUFoo`. Classes whose own name
    starts with `U` or `V` self-match in the head for the same reason
    (`Vector@@` reads as `V` + `ector`).
    """
    parsed = _split_signature(mangled)
    if parsed is None:
        return mangled, []
    head, prefix, body, terminator = parsed
    tokens = _argument_tokens(body)
    if not tokens:
        return mangled, []
    names, subjects = _name_table(head), []

    def _key(match):
        key, written, index = match.groups()
        if written:
            subject = written
            if len(names) < 10:
                names.append(written)
        elif int(index) < len(names):
            subject = names[int(index)]
        else:
            return match.group(0)
        subjects.append(subject)
        return KEY_CHARACTER.get(keys.get(subject, ""), key) + match.group(0)[1:]

    rebuilt = "".join(CLASS_KEY.sub(_key, token) for token in tokens)
    if rebuilt == body:
        return mangled, subjects
    return f"{head}{prefix}{rebuilt}{terminator}Z", subjects


def class_key_uses(mangled: str) -> list:
    """Every class named by a TYPE in this signature, back-references resolved.

    `?draw@Buffer@@QAEHPAV1@HHH@Z` passes `Buffer *`, and slot 1 is the
    enclosing class, so reading only the written-out `V<Name>@@` form misses
    most real uses.
    """
    return _rewrite_class_keys(mangled, {})[1]


def canonicalise_class_keys(mangled: str, keys: dict) -> str:
    """Spell every struct/class key the way the SOURCE will declare it.

    MSVC mangles a struct `U` and a class `V`, so the two objects have to
    agree - and six classes disagree with THEMSELVES in the catalogue
    (`Buffer` is `U` 49 times and `V` 17), which no tally can resolve because
    the linker had exactly one answer and some of those rows are simply
    wrong. The original's answer is also unrecoverable: the image carries no
    RTTI and no embedded mangled strings, so nothing in it says which key
    `Buffer` was declared with.

    It does not have to. BOTH objects are ours - the target object's symbol
    is already a computed choice rather than a quotation, the same way the
    1,179 disassembler labels get a decoration - so the key only has to be
    the same on both sides. `emit_translation_unit.class_keys` picks it, this
    writes it into the target, and the disagreement stops mattering.

    Only the key character changes; the identifier, the indirection and the
    back-reference indices are untouched.
    """
    return _rewrite_class_keys(mangled, keys or {})[0]


def compress_backrefs(mangled: str) -> str:
    """The argument list as CL writes it: a repeated type becomes its index.

    MSVC never spells the same composite argument type twice. The second
    `PAH` in `?f@@QAEXPAHPAH@Z` is written `0`, and the catalogue does not do
    this - its names come from a demangle/remangle round trip that expands
    them - so 37 rows had a target object holding a name the compiler will
    never emit. Primitives (`H`, `D`, `X`) are one character and are never
    back-referenced; only tokens longer than that take a slot.

    Anything this cannot parse is returned unchanged.
    """
    mangled = (mangled or "").strip()
    parsed = _split_signature(mangled)
    if parsed is None:
        return mangled
    head, prefix, body, terminator = parsed
    tokens = _argument_tokens(body)
    if not tokens:
        return mangled

    # TWO tables, both counting from 0 and both read off a real VC6 object.
    #
    #   types  a whole argument type written before becomes its index:
    #          `?f4@@YAXPAD0PAH1@Z` for (char*, char*, int*, int*).
    #   names  an identifier written before - the function's own name is 0,
    #          its class 1, then each new type name - becomes its index
    #          INSIDE the type: `?update@GraphicWin@@QAEXHHHHPAU1@@Z`.
    #
    # The type table wins when both could apply, which
    # `?f3@@YAXPAUSprite@@PAUGraphicWin@@0@Z` shows: the repeat is the whole
    # type, so it is `0` rather than a name back-reference.
    names = _name_table(head)
    seen, out = [], []
    for position, token in enumerate(tokens):
        if position and len(token) > 1 and token in seen:
            out.append(str(seen.index(token)))
            continue
        original = token

        def _name(match):
            key, identifier = match.group(1), match.group(2)
            if identifier in names:
                return f"{key}{names.index(identifier)}@"
            if len(names) < 10:
                names.append(identifier)
            return match.group(0)

        token = UDT_NAME.sub(_name, token)
        if position and len(original) > 1 and len(seen) < 10:
            seen.append(original)
        out.append(token)
    rebuilt = "".join(out)
    if rebuilt == body:
        return mangled
    return f"{head}{prefix}{rebuilt}{terminator}Z"




def undecorate(symbol: str) -> str:
    """The identifier inside a C decoration - `_sub_5e3650@4` -> `sub_5e3650`.

    The inverse of `decorate`, for the tools that are HANDED a symbol and have
    to find the catalogue row again: Mizuchi's `{{functionName}}` now carries
    the symbol rather than the catalogued label, and the context emitter and
    the integrator both resolve an address from it.
    """
    symbol = (symbol or "").strip()
    if MANGLED.match(symbol):
        return symbol
    for pattern in (DECORATED_FASTCALL, DECORATED_STDCALL, DECORATED_CDECL):
        found = pattern.match(symbol)
        if found:
            return found.group(1)
    return symbol


def address_in(symbol: str) -> int | None:
    """The address a synthesised identifier carries, when it carries one.

    `fn_00401c80` and `m_004483c0` are minted from an address precisely
    because the catalogued name (`SessionStruct::SessionStruct`) is not an
    identifier, so the address is the only way back to the row.
    """
    found = SYNTHETIC_IDENTIFIER.match(undecorate(symbol))
    return int(found.group(1), 16) if found else None


def disagreement(name: str, symbol: str) -> str:
    """Why a computed symbol differs from a catalogued decoration, or ''.

    Only meaningful for the names that already ARE symbols. A difference means
    the recovered signature contradicts the binary's own decoration - a wrong
    arity or a wrong convention - and the two objects still pair with each
    other, so nothing fails; it just quietly stops describing the original.
    """
    name = (name or "").strip()
    if MANGLED.match(name) or not name or symbol == name:
        return ""
    if spelling(name).identifier and name[0] in "_@":
        return f"{name} decorates as {symbol}"
    return ""
