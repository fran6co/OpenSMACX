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
}


USER_DEFINED_NAME = re.compile(r"[A-Za-z_]\w*")


def _decode_type(text: str, index: int, seen: list | None = None):
    """One encoded type starting at `index`; returns (spelling, next index)
    or (None, index) when the encoding is out of scope.

    `seen` is the back-reference table - the composite argument types already
    written out, in order. MSVC writes the second occurrence of one as its
    index, so without the table `?f@@YAXPAD0@Z` decodes one parameter and a
    dead `0`. Passing None disables them, which is what a caller decoding a
    lone type outside an argument list wants.
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
        found = USER_DEFINED_NAME.match(text, index + 1)
        if not found or not text.startswith("@@", found.end()):
            return None, index
        return found.group(0), found.end() + 2
    if char in "PQA":
        # Pointer (P/Q) or reference (A), a CV code, then the pointee. `B` is
        # the const one, and dropping it emitted `char *` where the target
        # holds `PBD` - 25 rows, all of them string arguments.
        next_index = index + 1
        const = ""
        if next_index < len(text) and text[next_index] in "ABCD":
            const = "const " if text[next_index] in "BD" else ""
            next_index += 1
        base, next_index = _decode_type(text, next_index, seen)
        if base is None:
            return None, index
        return f"{const}{base} {'&' if char == 'A' else '*'}", next_index
    # Function pointers (P6), member pointers, templates: still out of scope.
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
    skip = 2 if tail.startswith("Y") else 3
    if len(tail) <= skip:
        return None
    body = tail[skip:]

    returns, index = _decode_type(body, 0)
    if returns is None:
        return None

    # The back-reference table covers ARGUMENTS only; the return type takes no
    # slot, which is why it is decoded above with no table.
    params, seen = [], []
    while index < len(body) and body[index] != "@":
        if body[index] == "X" and (index + 1 >= len(body) or body[index + 1] in "@Z"):
            break  # (void)
        start = index
        param, index = _decode_type(body, index, seen)
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
        if stripped.startswith("struct ") and stripped.endswith("{"):
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
