#!/usr/bin/env python3
"""Fix callee declarations so VC6 re-mangles them to the catalogued names.

The scaffolding emits declarations from the catalogue's RECORDED prototypes,
which are IDA's spelling and lose mangling distinctions: both `char` and
`signed char` arrive as `int8`, both `unsigned int` and `unsigned long` as
`unsigned int`. VC6 mangles them differently (measured on cl 12.00.8168:
char=D, signed char=C, unsigned char=E, unsigned int=I, unsigned long=K), so
a call compiled against the scaffold can reference `...I@Z` where the
original image carries `...K@Z`, and objdiff pairs by symbol name.

This rewrites each callee declaration from the callee's MANGLED name - the
ground truth - decoding the parameter list and respelling every scalar so it
re-mangles identically. Declarations it cannot fully decode (function
pointers, struct pointers, member pointers) are left untouched: the mismatch
then stays visible in the diff, which is the honest signal.

C-linkage callees (no `?` prefix) get `extern "C"` and, for the common CRT
names, a real signature, because `int __cdecl _free();` both mangles as C++
and rejects the argument the original call passes.
"""

from __future__ import annotations

import re

import recovery_symbols

# Measured on cl 12.00.8168: the spelling that re-mangles to each code.
SCALAR = {
    "X": "void",
    "D": "char",
    "C": "signed char",
    "E": "unsigned char",
    "F": "short",
    "G": "unsigned short",
    "H": "int",
    "I": "unsigned int",
    "J": "long",
    "K": "unsigned long",
    "M": "float",
    "N": "double",
}

# C-linkage callees the catalogue records without a prototype. Signatures are
# the CRT's own; arity must match or VC6 rejects the call.
CRT_SIGNATURES = {
    "_free": "void _free(void *)",
    "_malloc": "void *_malloc(unsigned int)",
    "_calloc": "void *_calloc(unsigned int, unsigned int)",
    "_realloc": "void *_realloc(void *, unsigned int)",
    "_memcpy": "void *_memcpy(void *, const void *, unsigned int)",
    "_memmove": "void *_memmove(void *, const void *, unsigned int)",
    "_memset": "void *_memset(void *, int, unsigned int)",
    "_strlen": "unsigned int _strlen(const char *)",
    "_strcmp": "int _strcmp(const char *, const char *)",
    "_strcpy": "char *_strcpy(char *, const char *)",
    "_strcat": "char *_strcat(char *, const char *)",
    "_strncmp": "int _strncmp(const char *, const char *, unsigned int)",
    "_rand": "int _rand()",
    "_srand": "void _srand(unsigned int)",
    "_abs": "int _abs(int)",
    # The low-level IO trio. These are not `?`-mangled and appear in no
    # prototype source, so nothing could decode their arity and the emitter
    # declared them NULLARY. A body that calls one then fails `C2660: does not
    # take 3 parameters`, and declaring it in the body instead fails `C2733:
    # second C linkage of overloaded function` - so the function is
    # unreachable for a reason that has nothing to do with its own bytes.
    # `Caviar::vx_read` and `Caviar::vx_write` are thin forwarders onto these
    # and were blocked on exactly this.
    "_read": "int _read(int, void *, unsigned int)",
    "_write": "int _write(int, const void *, unsigned int)",
    "_close": "int _close(int)",
    # REPORTED BY AGENTS ON 2026-08-14, two independently, each having worked
    # around it by casting a function pointer to the right type - a correct
    # answer to the wrong problem, and one that changes the source form the
    # comparison is judging. The failure is the one this table already exists
    # for: the name is declared NULLARY, a real call is `C2660`, and
    # redeclaring it in the body is `C2733`, so the function is unreachable
    # for a reason that has nothing to do with its own bytes. `memcmp` blocked
    # 0x00422F20 and `isdigit`/`_itoa` blocked 0x005A9DB0.
    "_memcmp": "int _memcmp(const void *, const void *, unsigned int)",
    "_strncpy": "char *_strncpy(char *, const char *, unsigned int)",
    "_strchr": "char *_strchr(const char *, int)",
    "_strrchr": "char *_strrchr(const char *, int)",
    "_strstr": "char *_strstr(const char *, const char *)",
    "_stricmp": "int _stricmp(const char *, const char *)",
    "_strnicmp": "int _strnicmp(const char *, const char *, unsigned int)",
    "_atoi": "int _atoi(const char *)",
    "_atol": "long _atol(const char *)",
    "_itoa": "char *_itoa(int, char *, int)",
    "_ltoa": "char *_ltoa(long, char *, int)",
    "_isdigit": "int _isdigit(int)",
    "_isalpha": "int _isalpha(int)",
    "_isspace": "int _isspace(int)",
    "_isupper": "int _isupper(int)",
    "_islower": "int _islower(int)",
    "_toupper": "int _toupper(int)",
    "_tolower": "int _tolower(int)",
    # VARIADIC, and spelled so. A fixed arity here would be the same defect
    # one step along: right name, wrong call.
    "_sprintf": "int _sprintf(char *, const char *, ...)",
    "_printf": "int _printf(const char *, ...)",
    "_fprintf": "int _fprintf(void *, const char *, ...)",
    "_sscanf": "int _sscanf(const char *, const char *, ...)",
    "_qsort": "void _qsort(void *, unsigned int, unsigned int, "
              "int (__cdecl *)(const void *, const void *))",
}


USER_DEFINED_NAME = re.compile(r"[A-Za-z_]\w*")

# `P6<conv>`: the convention a function-pointer type carries.
FUNCTION_POINTER_CONVENTION = {"A": "__cdecl", "G": "__stdcall",
                              "I": "__fastcall",
                              "E": "__thiscall"}


def _decode_type(text: str, index: int, seen: list | None = None,
                 names: list | None = None):
    """One encoded type starting at `index`; returns (spelling, next index)
    or (None, index) when the encoding is out of scope.

    `seen` is the back-reference table - the composite argument types already
    written out, in order. MSVC writes the second occurrence of one as its
    index, so without the table `?f@@YAXPAD0@Z` decodes one parameter and a
    dead `0`. Passing None disables them, which is what a caller decoding a
    lone type outside an argument list wants.

    `names` is the OTHER back-reference table, and MSVC keeps it separately:
    it holds identifiers rather than whole types, seeded with the function's
    own qualification chain. `?init@Filemap@@QAEPAV1@PBDH@Z` returns
    `PAV1@` - a pointer to name 1, which is `Filemap`, the class it is
    declared in. Without it the whole signature came back None and two
    `Filemap::init` overloads could not be told apart at all.
    """
    if index >= len(text):
        return None, index
    char = text[index]
    if char.isdigit():
        slot = int(char)
        if seen is None or slot >= len(seen):
            return None, index
        return seen[slot], index + 1
    if char in SCALAR:
        return SCALAR[char], index + 1
    if char == "_":
        if index + 1 < len(text) and text[index + 1] == "N":
            return "bool", index + 2
        return None, index
    if char in "UVT":
        # A struct, class or union, named up to `@@`. Both keys decode to the
        # bare name: the emitted unit declares every one of them `struct`,
        # which is what `PAU` asks for and what the catalogue holds 8 to 1.
        if text[index + 1:index + 2].isdigit() and \
                text[index + 2:index + 3] == "@":
            slot = int(text[index + 1])
            if names is None or slot >= len(names):
                return None, index
            return names[slot], index + 3
        found = USER_DEFINED_NAME.match(text, index + 1)
        if not found or not text.startswith("@@", found.end()):
            return None, index
        if names is not None and found.group(0) not in names:
            names.append(found.group(0))
        return found.group(0), found.end() + 2
    if char == "P" and text[index + 1:index + 2] == "6":
        # A FUNCTION POINTER: `P6<conv><ret><args>@Z`. Returning None here
        # made `decode_signature` give up on the WHOLE signature, so 15 rows
        # that pass a callback kept the prototype's spelling of every OTHER
        # argument - and the prototype writes `char *` where the name says
        # `PBD`, a different symbol. They only surfaced once the units began
        # compiling; before that they failed earlier and never got this far.
        convention = FUNCTION_POINTER_CONVENTION.get(text[index + 2:index + 3])
        if convention is None:
            return None, index
        inner = index + 3
        returns, inner = _decode_type(text, inner, None, names)
        if returns is None:
            return None, index
        # A nested argument list keeps its own back-reference table.
        params, nested = [], []
        while inner < len(text) and text[inner] != "@":
            if text[inner] == "X" and text[inner + 1:inner + 2] in ("@", "Z"):
                inner += 1
                break                        # (void)
            start = inner
            param, inner = _decode_type(text, inner, nested, names)
            if param is None:
                return None, index
            if inner - start > 1 and len(nested) < 10:
                nested.append(param)
            params.append(param)
        if text[inner:inner + 1] == "@":
            inner += 1
        if text[inner:inner + 1] != "Z":
            return None, index
        return f"{returns} ({convention} *)({', '.join(params)})", inner + 1
    if char in "PQA":
        # Pointer (P/Q) or reference (A), a CV code, then the pointee. `B` is
        # the const one, and dropping it emitted `char *` where the target
        # holds `PBD` - 25 rows, all of them string arguments.
        next_index = index + 1
        const = ""
        if next_index < len(text) and text[next_index] in "ABCD":
            const = "const " if text[next_index] in "BD" else ""
            next_index += 1
        base, next_index = _decode_type(text, next_index, seen, names)
        if base is None:
            return None, index
        # `Q` is the POINTER itself being const, which is a different type
        # from `PB` (a pointer to const) and mangles differently. Measured
        # against VC6: `Palette *const` gives `QAV1@` and `const Palette *`
        # gives `PBV1@`. Decoding both as `*` emitted `PAV1@` for the one row
        # that takes a const pointer, which pairs with neither.
        indirection = {"A": "&", "Q": "*const"}.get(char, "*")
        return f"{const}{base} {indirection}", next_index
    # Member pointers and templates: still out of scope.
    return None, index


def decode_signature(mangled: str):
    """(return type, [param types]) from a decorated name, or None.

    Handles the two shapes this image contains: free functions
    (?name@@YA<ret><params>@Z) and class methods
    (?name@Class@@<3 qualifier chars><ret><params>@Z).
    """
    if not mangled.startswith("?"):
        return None
    # The qualifier chain closes at the FIRST `@@`, not the last. Splitting on
    # the last worked only while no argument was a user-defined type, because
    # `PAUSprite@@` carries one too - so every signature with a struct
    # parameter decoded its own argument list as the class qualifier and came
    # back None.
    split = mangled.find("@@")
    if split == -1:
        return None
    tail = mangled[split + 2:]
    if not tail.endswith("Z"):
        return None
    tail = tail[:-1]

    # Free functions spell the convention as Y<letter> (?name@@YA...); class
    # methods carry a 3-char qualifier+convention prefix (?name@Class@@QAE...).
    # A STATIC member has no CV code either, so it is two like a free one:
    # `??3AlphaMovie@@SAXPAXI@Z` is `SA` + `X` + `PAX` + `I`, and reading
    # three swallowed the void return and dropped the first argument, so all
    # 47 of them were emitted returning `void *` and taking one parameter.
    #
    # THE SAME BUG BIT THE SAME 47 ROWS A SECOND TIME, from the other end. On
    # 2026-08-13 `catalogue_corrections` renamed them to the this-adjusting
    # thunk spellings their bodies carry, and a thunk's kind is NOT adjacent to
    # its cv slot - the displacement it applies to `this` sits between them. A
    # hardcoded 2-or-3 read `??_GAlphaMovie@@WEEE@AEPAXI@Z` as `unsigned char
    # ()` and `??_GPlanWin@@$4PPPPPPPM@A@AEPAXI@Z` as `float ******()`, and
    # RETURNED THEM rather than None, so nothing downstream could tell a guess
    # from an answer. `infix_length` owns the arithmetic for all three decoders
    # in this tree, and refuses instead of guessing when it cannot read the
    # adjustment.
    skip = recovery_symbols.infix_length(tail)
    if skip is None or len(tail) <= skip:
        return None
    body = tail[skip:]

    # The NAME back-reference table, seeded with this symbol's own
    # qualification chain in the order MSVC records it: the base name first,
    # then each enclosing scope innermost-first, which is the order they are
    # written in. `?init@Filemap@@` therefore seeds ['init', 'Filemap'], and
    # `V1@` in the signature resolves to `Filemap`.
    qualification = mangled[1:split]
    if qualification.startswith("?"):
        # AN OPERATOR CODE IS NOT A NAME and takes no slot. `??4Filemap@@`
        # spells `operator=` as `?4` glued to the class with no separator, so
        # splitting on `@` yields one part `?4Filemap` - and `V0@` in its
        # signature means `Filemap`, slot 0. `?_G` and the rest of the
        # underscore forms are two characters after the `?`.
        qualification = qualification[3:] if qualification[1:2] == "_" \
            else qualification[2:]
    names = [part for part in qualification.split("@") if part]

    returns, index = _decode_type(body, 0, None, names)
    if returns is None:
        return None

    # The TYPE back-reference table covers ARGUMENTS only; the return type
    # takes no slot, which is why it is decoded above with no table. The NAME
    # table is not the same table and does cover it - a class named in the
    # return type is recorded and can be referred back to from an argument.
    params, seen = [], []
    while index < len(body) and body[index] != "@":
        if body[index] == "X" and (index + 1 >= len(body) or body[index + 1] in "@Z"):
            break  # (void)
        start = index
        param, index = _decode_type(body, index, seen, names)
        if param is None:
            return None
        # A slot is taken by any type written as more than one character, and
        # never by a back-reference standing in for one already recorded.
        if index - start > 1 and len(seen) < 10:
            seen.append(param)
        params.append(param)

    return returns, params


def fix_declarations(scaffold: str, callees: list[dict]) -> str:
    """Rewrite callee declarations in the scaffold, guided by mangled names.

    `callees` are catalogue rows (with `name`) for every function the subject
    calls. Returns the scaffold with corrections applied.
    """
    lines = scaffold.splitlines()

    for row in callees:
        name = row.get("name") or ""
        if not name:
            continue

        if not name.startswith("?"):
            _fix_c_linkage(lines, name)
            continue

        decoded = decode_signature(name)
        if decoded is None:
            continue
        returns, params = decoded

        # Base name is between the leading ? and the first @.
        base = name[1:name.index("@")]
        _fix_declaration(lines, base, returns, params)

    return "\n".join(_drop_duplicate_members(lines))


def _drop_duplicate_members(lines: list[str]) -> list[str]:
    """Remove a member declaration this pass turned into a twin of another.

    Rewriting is per-callee and each callee is decoded independently, so two
    overloads the catalogue spells differently - `PAD` against `PBD`, `H`
    against `J` - can both land on the same text. C++ calls that
    `error C2535: member function already defined or declared` and refuses the
    whole unit. That is how it was found: a unit that used to compile stopped
    the moment the emitter began decoding types out of mangled names.

    Only exact duplicates, and only inside one struct body, so a genuine
    overload set is left alone.
    """
    out, seen, depth = [], set(), 0
    for line in lines:
        stripped = line.strip()
        if stripped.startswith(("struct ", "class ")) and "{" in stripped:
            # `class X { public:` as well as `struct X {`: a type with methods
            # is emitted as a class, and matching only the struct spelling let
            # the C2535 this exists to prevent back into every one of them.
            depth, seen = depth + 1, set()
        elif stripped == "};" and depth:
            depth -= 1
        elif depth and stripped.endswith(";"):
            if stripped in seen:
                continue
            seen.add(stripped)
        out.append(line)
    return out


def _fix_declaration(lines: list[str], base: str, returns: str,
                     params: list[str]) -> None:
    pattern = re.compile(
        rf"^(\s*)(?:[\w\s\*]+?\s+)?{re.escape(base)}\s*\(([^)]*)\)\s*;$")
    new_params = ", ".join(params)
    for index, line in enumerate(lines):
        match = pattern.match(line)
        if not match:
            continue
        indent = match.group(1)
        old_params = [p.strip() for p in match.group(2).split(",")
                      if p.strip()]
        if len(old_params) != len(params):
            continue  # arity disagrees; leave the honest mismatch visible
        if "{" in line or "public" in line:
            continue
        lines[index] = f"{indent}{returns} {base}({new_params});"
        return


def _fix_c_linkage(lines: list[str], name: str) -> None:
    signature = CRT_SIGNATURES.get(name)
    pattern = re.compile(
        rf"^(\s*)(?:[\w\s\*]+?__cdecl\s+)?{re.escape(name)}\s*\([^)]*\)\s*;$")
    for index, line in enumerate(lines):
        match = pattern.match(line)
        if not match:
            continue
        indent = match.group(1)
        if signature:
            lines[index] = f'{indent}extern "C" {signature};'
        else:
            # Keep the guessed shape but fix the linkage: without extern "C"
            # VC6 mangles the callee as C++, which never matches the image.
            body = line.strip()
            lines[index] = f'{indent}extern "C" {body}'
        return
