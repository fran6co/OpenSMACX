#!/usr/bin/env python3
"""Rewrite the `__thiscall` seams into pointers-to-member.

cl 12.00.8168 reserves `__thiscall` and refuses it, and silencing that
warning silently downgrades the call to __cdecl - see src/original_seam.h for
the measurement. A pointer-to-member is thiscall in every compiler without
naming the convention, so each seam becomes:

    typedef void(__thiscall f)(Buffer *, int, int);   ->  typedef void (OriginalObject::*f)(int, int);
    extern f *Ptr;                                    ->  extern f Ptr;
    f *Ptr = (f *)0xADDR;                             ->  f Ptr = original_method<f>(0xADDR);
    Ptr(obj, a, b);                                   ->  (ORIGINAL(obj)->*Ptr)(a, b);

The receiver moves out of the argument list, which is what makes the call
sites read as ordinary C++ afterwards.

Argument splitting is PAREN AND ANGLE AWARE. A regex on commas gets
`Ptr(this, f(a, b), c)` wrong, and it gets it wrong quietly - the seam would
still compile with the arguments shifted by one.
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
SRC = REPO_ROOT / "src"

# `typedef RET(__thiscall NAME)(params);` and the `*NAME` pointer spelling.
TYPEDEF = re.compile(
    r"typedef\s+(?P<ret>[\w\s:*&]+?)\s*\(\s*__thiscall\s*(?P<star>\*?)\s*"
    r"(?P<name>\w+)\s*\)\s*\((?P<params>[^;]*?)\)\s*;",
    re.S)


def split_arguments(text: str) -> list:
    """Top-level comma split that respects (), [] and <>."""
    out, depth, current = [], 0, []
    for character in text:
        if character in "([<":
            depth += 1
        elif character in ")]>":
            depth -= 1
        if character == "," and depth == 0:
            out.append("".join(current).strip())
            current = []
            continue
        current.append(character)
    tail = "".join(current).strip()
    if tail:
        out.append(tail)
    return out


def call_arguments(text: str, start: int):
    """The argument text of a call whose `(` is at `start`, and the index
    just past its `)`. None when the parentheses do not close."""
    depth = 0
    for index in range(start, len(text)):
        if text[index] == "(":
            depth += 1
        elif text[index] == ")":
            depth -= 1
            if depth == 0:
                return text[start + 1:index], index + 1
    return None, None


def convert_typedefs(text: str) -> tuple:
    """Rewrite every seam typedef; returns (text, {name: was_function_type})."""
    seams = {}

    def replace(match):
        name = match.group("name")
        params = split_arguments(match.group("params"))
        # The first parameter is the receiver and becomes `this`.
        rest = ", ".join(params[1:]) if len(params) > 1 else ""
        seams[name] = match.group("star") == ""
        return (f"typedef {match.group('ret').strip()} "
                f"(OriginalObject::*{name})({rest});")

    return TYPEDEF.sub(replace, text), seams


def convert_declarations(text: str, function_typed: set) -> str:
    """`f *Ptr` -> `f Ptr` for the seams declared as a function TYPE.

    The pointer-spelled ones (`typedef RET(__thiscall *f)(...)`) already
    declare `f Ptr`, and must not lose a star they never had.
    """
    for name in function_typed:
        text = re.sub(rf"\b{re.escape(name)}\s*\*\s*(\w+)", rf"{name} \1", text)
    return text


def convert_bindings(text: str, seams: set) -> str:
    """`f Ptr = (f *)0xADDR;` -> `f Ptr = original_method<f>(0xADDR);`"""
    for name in seams:
        text = re.sub(
            rf"(\b{re.escape(name)}\s+\w+\s*=\s*)"
            rf"\(\s*{re.escape(name)}\s*\*?\s*\)\s*(0[xX][0-9A-Fa-f]+)",
            rf"\1original_method<{name}>(\2)", text)
    return text


def convert_calls(text: str, variables: set) -> tuple:
    """`Ptr(obj, a)` -> `(ORIGINAL(obj)->*Ptr)(a)`, innermost-last."""
    converted = 0
    for variable in sorted(variables, key=len, reverse=True):
        pattern = re.compile(rf"(?<![\w>.])\b{re.escape(variable)}\s*\(")
        index = 0
        while True:
            found = pattern.search(text, index)
            if not found:
                break
            arguments, end = call_arguments(text, found.end() - 1)
            if arguments is None:
                index = found.end()
                continue
            parts = split_arguments(arguments)
            if not parts:
                index = found.end()
                continue
            receiver, rest = parts[0], ", ".join(parts[1:])
            replacement = f"(ORIGINAL({receiver})->*{variable})({rest})"
            text = text[:found.start()] + replacement + text[end:]
            index = found.start() + len(replacement)
            converted += 1
    return text, converted


def convert_vtable_calls(text: str, seams: set) -> tuple:
    """`reinterpret_cast<f *>(slot)(obj, a)` -> a pointer-to-member call.

    A virtual of the original is reached by casting the vtable entry to the
    seam type and calling it. Once the seam IS a pointer-to-member, that cast
    produces a pointer to a pointer-to-member and the call stops being a call
    at all - `error C2064: term does not evaluate to a function`. 198 sites,
    and the converter's first pass did not see them because they go through a
    cast rather than a declared variable.
    """
    converted = 0
    for name in sorted(seams, key=len, reverse=True):
        pattern = re.compile(rf"reinterpret_cast\s*<\s*{re.escape(name)}\s*\*?\s*>\s*\(")
        index = 0
        while True:
            found = pattern.search(text, index)
            if not found:
                break
            slot, after = call_arguments(text, found.end() - 1)
            if slot is None:
                index = found.end()
                continue
            # The call must follow immediately, or this is a cast used for
            # something else and is left alone.
            rest_of_text = text[after:]
            offset = len(rest_of_text) - len(rest_of_text.lstrip())
            if not rest_of_text[offset:offset + 1] == "(":
                index = after
                continue
            arguments, end = call_arguments(text, after + offset)
            if arguments is None:
                index = after
                continue
            parts = split_arguments(arguments)
            if not parts:
                index = after
                continue
            receiver, rest = parts[0], ", ".join(parts[1:])
            replacement = (f"(ORIGINAL({receiver})->*original_method<{name}>("
                           f"reinterpret_cast<unsigned long>({slot.strip()})))({rest})")
            text = text[:found.start()] + replacement + text[end:]
            index = found.start() + len(replacement)
            converted += 1
    return text, converted


def seam_variables(text: str, seams: set) -> set:
    """Every variable declared with a seam type, however it is spelled."""
    found = set()
    for name in seams:
        found |= set(re.findall(rf"\b{re.escape(name)}\s*\*?\s*(\w+)\s*[=;,)]", text))
    return found - {"const"}


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("files", nargs="*", type=Path,
                        help="default: every src/*.h and src/*.cpp")
    parser.add_argument("--apply", action="store_true",
                        help="write the files (default: report only)")
    arguments = parser.parse_args()

    # The seam header DEFINES the vocabulary this rewrites into, and its
    # documentation quotes the old spelling on purpose. Converting it rewrites
    # the explanation into the thing being explained, and makes it include
    # itself.
    excluded = {"original_seam.h", "vc6_compat.h"}
    paths = arguments.files or [p for p in
                                sorted(SRC.glob("*.h")) + sorted(SRC.glob("*.cpp"))
                                if p.name not in excluded]

    # Pass 1: every seam typedef in the tree, so a call site in one file can
    # be rewritten against a typedef declared in another. BOTH spellings are
    # collected - the original `__thiscall` one and the converted
    # pointer-to-member - because the vtable-cast sites are found in a later
    # run, by which time the typedefs themselves are already converted.
    seams, function_typed = set(), set()
    for path in paths:
        text = path.read_text(errors="ignore")
        _, found = convert_typedefs(text)
        seams |= set(found)
        function_typed |= {n for n, is_function in found.items() if is_function}
        seams |= set(re.findall(
            r"typedef\s+[\w\s:*&]+?\(\s*OriginalObject::\*(\w+)\s*\)", text))

    variables = set()
    for path in paths:
        variables |= seam_variables(path.read_text(errors="ignore"), seams)

    print(f"{len(seams)} seam typedefs, {len(variables)} bound variables")

    touched = calls = 0
    for path in paths:
        original = path.read_text(errors="ignore")
        text, _ = convert_typedefs(original)
        text = convert_declarations(text, function_typed)
        text = convert_bindings(text, seams)
        text, n = convert_calls(text, variables)
        text, vtable = convert_vtable_calls(text, seams)
        n += vtable
        if text == original:
            continue
        if "original_seam.h" not in text and (n or "OriginalObject::" in text):
            text = re.sub(r'(#include "stdafx.h"\n)', r'\1#include "original_seam.h"\n',
                          text, count=1)
            if "original_seam.h" not in text:
                text = re.sub(r"(#pragma once\n)", r'\1\n#include "original_seam.h"\n',
                              text, count=1)
        touched += 1
        calls += n
        if arguments.apply:
            path.write_text(text)
        else:
            print(f"   would change {path.name} ({n} call sites)")
    print(f"{touched} files{' changed' if arguments.apply else ' would change'}, "
          f"{calls} call sites")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
