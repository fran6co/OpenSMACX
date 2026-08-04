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


def _decode_type(text: str, index: int):
    """One encoded type starting at `index`; returns (spelling, next index)
    or (None, index) when the encoding is out of scope."""
    if index >= len(text):
        return None, index
    char = text[index]
    if char in SCALAR:
        return SCALAR[char], index + 1
    if char == "_":
        if index + 1 < len(text) and text[index + 1] == "N":
            return "bool", index + 2
        return None, index
    if char == "P":
        # Pointer: P, an optional near/const qualifier, then the pointee.
        next_index = index + 1
        if next_index < len(text) and text[next_index] in "AB":
            next_index += 1
        base, next_index = _decode_type(text, next_index)
        if base is None:
            return None, index
        return base + " *", next_index
    # Struct/class pointers (PAU/PAV), references, function pointers (P6),
    # member pointers: out of scope for the MVP.
    return None, index


def decode_signature(mangled: str):
    """(return type, [param types]) from a decorated name, or None.

    Handles the two shapes this image contains: free functions
    (?name@@YA<ret><params>@Z) and class methods
    (?name@Class@@<3 qualifier chars><ret><params>@Z).
    """
    if not mangled.startswith("?"):
        return None
    parts = mangled.split("@@")
    if len(parts) < 2:
        return None
    tail = parts[-1]
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

    params = []
    while index < len(body) and body[index] != "@":
        if body[index] == "X" and (index + 1 >= len(body) or body[index + 1] in "@Z"):
            break  # (void)
        param, index = _decode_type(body, index)
        if param is None:
            return None
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

    return "\n".join(lines)


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
