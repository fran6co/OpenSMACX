#!/usr/bin/env python3
"""The data members `src/` declares for a class, for the scaffolding emitter.

WHY THIS EXISTS. `emit_translation_unit` declares every class as an opaque
shell with no members, and an agent that needs a field therefore reaches it
by offset - or, worse, declares a shadow struct beside the function, which
`mizuchi_writeback.splice` cannot carry into `src/` because it splices the
definition alone. Measured on the first real run: 4 of 10 writebacks lost
that way, and the ones that landed put `self[2]` into `src/` where the file
already said `head_`.

`src/` knows the layout for 48 of the 109 classes that own unrecovered work
- 907 functions - and pins 21 of them with a `sizeof` static_assert. Handing
the agent the real members makes the recovered body readable, splices into
`src/` cleanly, and costs nothing: the emitter's own docstring records that
the same function written with real members and with offset casts compiles
to BYTE-IDENTICAL objects.

WHY IT IS GATED. A layout `src/` gets WRONG is worse than no layout at all:
the offsets silently move and a body that used to match stops. Only classes
whose size is pinned by a static_assert are emitted, because that assertion
is the tree's own statement that the layout was checked against the image.

WHAT IS DELIBERATELY NOT DONE. Nothing here parses C++ properly. It reads
declarations that look like data members out of a class body, and refuses
the class on anything it does not recognise - a nested type, a bitfield, an
array whose bound is an expression. A layout guessed wrong is the one
failure mode that matters, so every uncertainty refuses.
"""

from __future__ import annotations

import functools
import re
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
SRC = REPO_ROOT / "src"

CLASS_HEAD = re.compile(
    r"^(?:class|struct)\s+(?:DLLEXPORT\s+)?(?P<name>\w+)\s*"
    r"(?P<bases>:[^{]*)?\{", re.M)
SIZE_ASSERT = re.compile(r"static_assert\(\s*sizeof\(\s*(\w+)\s*\)\s*==\s*"
                         r"(0x[0-9A-Fa-f]+|\d+)")
# `uint32_t primary_abi_word_;` / `StringStructEntry *head_;` / `int a[4];`
# `uint32_t a_;`, `StringStructEntry *head_;`, `int grid_[4]`. The stars bind
# to the NAME in this tree's style, so they are captured separately rather
# than as part of the type - matching `type *` then whitespace then a name
# finds nothing at all, which is how this first came back with 11 classes
# instead of 45.
MEMBER = re.compile(
    r"^\s*(?P<type>(?:const\s+)?[A-Za-z_]\w*)\s*(?P<stars>\*+)?\s*"
    r"(?P<name>[A-Za-z_]\w*)\s*(?P<array>\[[^\]]*\])?\s*;\s*(?://.*)?$")
# `void (*callback)(int);` - a whole line that declares a function pointer,
# as opposed to a METHOD taking one, which is `void init(void (*cb)(int));`
# and has an identifier before the first paren.
FUNCTION_POINTER_MEMBER = re.compile(
    r"^\s*(?:const\s+)?[\w:]+\s*\(\s*(?:__\w+\s+)?\*+\s*\w+\s*\)\s*\([^;]*\)\s*;")
# Anything in a class body that means "stop, this is not a plain layout".
REFUSE = re.compile(r"\b(virtual|union|enum|typedef|template|operator|friend|"
                    r"static|:\s*\d+\s*;)\b")


def class_bodies(text: str):
    """(name, bases, body) for each class or struct defined in `text`."""
    for head in CLASS_HEAD.finditer(text):
        depth, index = 1, head.end()
        while index < len(text) and depth:
            if text[index] == "{":
                depth += 1
            elif text[index] == "}":
                depth -= 1
            index += 1
        yield head.group("name"), head.group("bases"), text[head.end():index - 1]


def members_of(body: str):
    """[(type, name, array)] for a body of nothing but plain data members.

    None when the body holds anything this cannot account for, because a
    member missed is a member whose absence moves every offset after it.
    """
    found = []
    for line in body.splitlines():
        stripped = line.strip()
        if not stripped or stripped.startswith(("//", "/*", "*", "#")):
            continue
        if stripped in ("public:", "private:", "protected:"):
            continue
        if REFUSE.search(stripped):
            # A method, a static, a nested type: not layout, but also not a
            # reason to refuse - only DATA is counted, and a method declares
            # none. A `virtual` is different: it adds a vtable pointer this
            # cannot see, so it refuses below.
            if re.search(r"\bvirtual\b|\bunion\b", stripped):
                return None
            continue
        if FUNCTION_POINTER_MEMBER.match(stripped):
            # `void (*callback)(int);` IS storage, and it contains a `(`, so
            # the method test below would skip it and every offset after it
            # would move. `sizeof` catches that only when padding does not
            # absorb the four bytes, so it is refused explicitly rather than
            # left to a check that might not fire. No pinned class has one
            # today; this is here so the next one cannot pass quietly.
            return None
        if "(" in stripped:            # a method declaration
            continue
        member = MEMBER.match(stripped)
        if not member:
            return None
        type_ = member.group("type").strip()
        stars = member.group("stars")
        if not stars and type_.replace("const", "").strip() not in SCALAR:
            # A member held BY VALUE needs a complete type, and the emitted
            # unit only forward-declares - `AutoSound auto_sound_;` is
            # `error C2079: uses undefined class`. Supplying such a layout
            # broke 675 of the 2,783 units before this refused it. A POINTER
            # to an incomplete type is fine, which is most of them.
            return None
        found.append((type_ + (" " + stars if stars else ""),
                      member.group("name"), member.group("array") or ""))
    return found


@functools.lru_cache(maxsize=1)
def pinned_layouts() -> dict:
    """{class name: [(type, name, array)]} for every class `src/` pins.

    Only classes carrying `static_assert(sizeof(X) == ...)`, and only those
    with no base class - a base contributes members this does not see, and
    the offsets would all be wrong by its size.
    """
    layouts = {}
    for header in sorted(SRC.glob("*.h")):
        text = header.read_text(errors="ignore")
        pinned = {name for name, _ in SIZE_ASSERT.findall(text)}
        for name, bases, body in class_bodies(text):
            if name not in pinned or bases:
                continue
            members = members_of(body)
            if members:
                layouts[name] = members
    return layouts


VERIFIED = REPO_ROOT / "docs" / "recovery" / "verified-layouts.txt"


@functools.lru_cache(maxsize=1)
def verified_names() -> frozenset:
    """Classes whose extracted layout compiles to the real class's size.

    Written by tools/verify_class_layouts.py. The regexes above are enough to
    be useful and not enough to be trusted - a member they skip moves every
    offset after it, and a body indexing through the wrong offset still
    COMPILES, failing later as a byte mismatch nobody traces back here. So a
    layout is supplied only once it has been proved.
    """
    if not VERIFIED.is_file():
        return frozenset()
    return frozenset(line.strip() for line in VERIFIED.read_text().splitlines()
                     if line.strip() and not line.startswith("#"))


def declaration_for(name: str) -> list:
    """The member lines to emit inside a class shell, or []."""
    if name not in verified_names():
        return []
    members = pinned_layouts().get(name)
    if not members:
        return []
    return [f"    {type_} {member}{array};" for type_, member, array in members]


def unverified_declaration_for(name: str) -> list:
    """The extracted members WITHOUT the verification gate, for the verifier."""
    members = pinned_layouts().get(name)
    return [f"    {type_} {member}{array};" for type_, member, array in members or []]


# Spelled here rather than imported so this module stays independent of the
# emitter; `emit_translation_unit.BUILTIN` is the same idea for the same
# reason. Forward-declaring one of these as a struct is `error C2371:
# 'uint32_t' : redefinition; different basic types`.
SCALAR = frozenset({
    "void", "bool", "char", "signed", "unsigned", "short", "int", "long",
    "float", "double", "const", "size_t", "wchar_t",
    "int8_t", "uint8_t", "int16_t", "uint16_t", "int32_t", "uint32_t",
    "int64_t", "uint64_t", "intptr_t", "uintptr_t",
})


def referenced_types(name: str) -> set:
    """Types a layout mentions, which the unit has to declare before it."""
    out = set()
    for type_, _, _ in pinned_layouts().get(name, ()):
        bare = type_.replace("*", " ").replace("const", " ").split()
        if bare and bare[-1] not in SCALAR:
            out.add(bare[-1])
    return out


if __name__ == "__main__":
    layouts = pinned_layouts()
    print(f"{len(layouts)} classes with a pinned layout src/ can supply")
    for name in sorted(layouts):
        print(f"   {name:22} {len(layouts[name]):3} members")
