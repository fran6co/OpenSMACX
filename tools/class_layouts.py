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
    r"(?P<name>[A-Za-z_]\w*)\s*(?P<array>\[[^\]]*\])?"
    # A default member initialiser - `uint8_t *base_ = nullptr;` - is C++11
    # and changes no layout. Not allowing for it refused `Heap`, and with it
    # every class that holds one.
    r"(?:\s*=\s*[^;]+)?\s*;\s*(?://.*)?$")
# `void (*callback)(int);` - a whole line that declares a function pointer,
# as opposed to a METHOD taking one, which is `void init(void (*cb)(int));`
# and has an identifier before the first paren.
FUNCTION_POINTER_MEMBER = re.compile(
    r"^\s*(?:const\s+)?[\w:]+\s*\(\s*(?:__\w+\s+)?\*+\s*(?P<name>\w+)\s*\)"
    r"\s*\([^;]*\)\s*;")
# `typedef int32_t Dib;` inside a class body. A nested alias for a type the
# parser already knows is layout it can account for exactly, and refusing it
# cost Buffer - whose `Dib dib_[256];` is 1 KB of int32_t - and through
# Buffer, GraphicWin and the eight classes holding one by value.
NESTED_TYPEDEF = re.compile(
    r"^\s*typedef\s+(?P<type>[A-Za-z_]\w*)\s+(?P<name>[A-Za-z_]\w*)\s*;")
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


# Win32 spellings that are typedefs, not classes. Holding one BY VALUE needs
# no definition - the emitter writes `typedef void * PVOID;` for any of these
# it sees - but they are not in SCALAR, deliberately: SCALAR controls what is
# left out of `referenced_types`, and these must stay IN it so that typedef
# gets emitted. Two different questions, and conflating them refused
# `AutoSound` for holding a `PVOID`.
# Kept in step with `emit_translation_unit.NOT_A_STRUCT` by hand rather than
# imported, because the emitter imports this module.
WINDOWS_TYPEDEF = frozenset({
    "BOOL", "CHAR", "WCHAR", "BYTE", "WORD", "UINT", "DWORD", "LONG", "ULONG",
    "LCID", "LPARAM", "WPARAM", "LRESULT", "UINT_PTR", "ULONG_PTR", "SIZE_T",
    "HRESULT", "LPSTR", "LPCSTR", "PVOID", "LPVOID", "LPCVOID", "LPBYTE",
    "LPWORD", "LPGUID", "PEXCEPTION_RECORD", "FARPROC",
    "HANDLE", "HWND", "HINSTANCE", "HIC", "LPDIRECTDRAW", "HDC",
    # GDI object handles, pointer-width exactly like every other H* above.
    # Their absence refused Font (HFONT) and Buffer (HRGN, HBITMAP) - nothing
    # about those layouts was unknown, three names were simply missing here.
    "HFONT", "HRGN", "HBITMAP", "HPALETTE", "HBRUSH", "HPEN", "HMENU",
})


# Win32 STRUCTS held by value. Unlike everything in WINDOWS_TYPEDEF these
# cannot be spelled as a primitive - a RECT is sixteen bytes of four LONGs and
# `typedef long RECT;` would be four bytes and wrong - so the emitter writes
# the definition instead. The members are the Win32 ones verbatim, and the
# mangling agrees: the catalogue spells it `PAURECT@@`, a pointer to a STRUCT
# called RECT, which is what `struct RECT { ... };` emits.
#
# RECT was the single most expensive omission in this file. Holding one by
# value refused Win, Buffer, Scroll and MainInterface outright; Buffer then
# refused GraphicWin and Flic; GraphicWin refused the eight classes that hold
# it as `virtual_base_`; and Scroll refused Popup and StringBox. One
# undeclared sixteen-byte struct, seventeen classes.
WINDOWS_STRUCT = {
    "RECT": (("long", "left", ""), ("long", "top", ""),
             ("long", "right", ""), ("long", "bottom", "")),
    "POINT": (("long", "x", ""), ("long", "y", "")),
    "SIZE": (("long", "cx", ""), ("long", "cy", "")),
}


def _needs_definition(name: str) -> bool:
    """Does holding this type by value require its definition in the unit?"""
    return name not in SCALAR and name not in WINDOWS_TYPEDEF


def _extractable(name: str, seen=()) -> bool:
    """Can this type be supplied as a full definition, so it may be held by
    value? Recursive, and cycle-safe: a class cannot contain itself."""
    if name in WINDOWS_STRUCT:
        return True
    if name in seen:
        return False
    declared = _declared_classes().get(name)
    if declared is None:
        return False
    base, _ = declared
    if base is None:
        return False
    return _with_bases(name, seen) is not None


def statements(body: str):
    """The class body as whole declarations, not lines.

    A declaration spanning two lines was the reason every GraphicWin-derived
    class refused: the continuation of

        int init(RECT *rect, LPSTR title, int flags,
                 Win *parent, Menu *menu, BorderSizing *border);

    has no `(` of its own, so it did not look like a method, did not look
    like a member either, and the whole class was thrown away. Joining on
    the semicolon - and on the brace of a nested definition - makes the
    method test see the whole thing.
    """
    pending, depth = [], 0
    for line in body.splitlines():
        stripped = line.strip()
        if not stripped or stripped.startswith(("//", "#")):
            continue
        if stripped.startswith(("/*", "*")) and not pending:
            continue
        # `uint32_t primary_abi_word_;   // 0x00` ends with a COMMENT, not a
        # semicolon, so the flush below never fired and every member after it
        # was glued onto the same statement. StringList came back with one.
        stripped = re.sub(r"//.*$", "", stripped).strip()
        if not stripped:
            continue
        if depth:
            # INSIDE AN INLINE METHOD BODY. `Heap` defines its constructor in
            # the class, and `base_ = nullptr;` there looks exactly like a
            # member declaration - worse, `int local = 0;` in such a body
            # WOULD parse as one and silently add a member the real class
            # does not have. Skipped by brace depth rather than by pattern.
            depth += stripped.count("{") - stripped.count("}")
            continue
        opens, closes = stripped.count("{"), stripped.count("}")
        if opens:
            # YIELD THE OPENING LINE, then skip the body. The line itself has
            # to reach the caller so `union {` can be refused - an anonymous
            # union IS storage, and swallowing it silently drops members and
            # moves every offset after them. The BODY must not reach the
            # caller: `Heap` defines its constructor inline and
            # `base_ = nullptr;` in there looks exactly like a member.
            pending.append(stripped)
            yield " ".join(pending)
            pending = []
            depth += opens - closes
            continue
        pending.append(stripped)
        if stripped.endswith(";"):
            yield " ".join(pending)
            pending = []
    if pending:
        yield " ".join(pending)


def members_of(body: str, seen=()):
    """[(type, name, array)] for a body of nothing but plain data members.

    None when the body holds anything this cannot account for, because a
    member missed is a member whose absence moves every offset after it.
    """
    found = []
    # Nested aliases first: `typedef int32_t Dib;` has to be known before the
    # member that uses it is read, and a class body may declare it anywhere.
    alias = {}
    for stripped in statements(body):
        hit = NESTED_TYPEDEF.match(stripped)
        if hit and (hit.group("type") in SCALAR
                    or hit.group("type") in WINDOWS_TYPEDEF):
            alias[hit.group("name")] = hit.group("type")
    for stripped in statements(body):
        if not stripped or stripped.startswith(("//", "/*", "*", "#")):
            continue
        # `private: AutoSound auto_sound_;` - the label and the member on one
        # line. Testing for the bare label missed those and refused the class.
        stripped = re.sub(r"^(?:public|private|protected)\s*:\s*", "", stripped)
        if not stripped:
            continue
        if REFUSE.search(stripped):
            # A method, a static, a nested type: not layout, but also not a
            # reason to refuse - only DATA is counted, and a method declares
            # none. A `virtual` is different: it adds a vtable pointer this
            # cannot see, so it refuses below.
            if re.search(r"\bvirtual\b|\bunion\b", stripped):
                return None
            continue
        pointer_member = FUNCTION_POINTER_MEMBER.match(stripped)
        if pointer_member:
            # A function pointer IS storage, and on this target it is four
            # bytes like any other. It is spelled `void *` here because the
            # emitter writes members as `<type> <name>;` and a function
            # pointer does not fit that shape - the LAYOUT is what this file
            # answers for, and four bytes is four bytes. A body that actually
            # calls one casts it, which is what src/ already does.
            found.append(("void *", pointer_member.group("name"), ""))
            continue
        if "(" in stripped:            # a method declaration
            continue
        member = MEMBER.match(stripped)
        if not member:
            return None
        type_ = member.group("type").strip()
        stars = member.group("stars")
        bare = type_.replace("const", "").strip()
        if bare in alias:
            type_ = type_.replace(bare, alias[bare])
            bare = alias[bare]
        if not stars and _needs_definition(bare) and not _extractable(bare, seen):
            # A member held BY VALUE needs a COMPLETE type, and the emitted
            # unit only forward-declares - `AutoSound auto_sound_;` is
            # `error C2079: uses undefined class`, which broke 675 of the
            # 2,783 units when by-value members were first allowed through.
            # It is fine when that type can be supplied too, and the emitter
            # emits the types a layout names before the layout itself. A
            # POINTER to an incomplete type never needs this.
            return None
        found.append((type_ + (" " + stars if stars else ""),
                      member.group("name"), member.group("array") or ""))
    return found


# `: public GraphicWin` / `: Win`. Access specifiers do not affect layout;
# `virtual` does, profoundly, and is refused.
BASE_SPEC = re.compile(r"^\s*:\s*(?P<bases>.+)$", re.S)


def base_of(bases: str):
    """The single base class, '' for none, or None when it cannot be used.

    MULTIPLE and VIRTUAL inheritance are refused rather than guessed. A
    virtual base is not laid out where a reader would expect - MSVC places it
    after the derived members and reaches it through a vbtable - and multiple
    bases interleave in an order this does not model. Neither appears in the
    classes this unlocks, so refusing costs nothing and guessing could put
    every offset in the wrong place.
    """
    if not bases:
        return ""
    found = BASE_SPEC.match(bases)
    if not found:
        return None
    names = [part.strip() for part in found.group("bases").split(",")]
    if len(names) != 1:
        return None
    words = names[0].replace("public", " ").replace("protected", " ") \
                    .replace("private", " ").split()
    if "virtual" in words or len(words) != 1:
        return None
    return words[0]


@functools.lru_cache(maxsize=1)
def _declared_classes() -> dict:
    """{class name: (base or '' or None, body text)}.

    The BODY, not its members: whether a member may be held by value depends
    on whether ITS type can be supplied, which is a recursive question, and
    answering it while building this map would recurse into a half-built one.
    """
    out = {}
    for header in sorted(SRC.glob("*.h")):
        for name, bases, body in class_bodies(header.read_text(errors="ignore")):
            out.setdefault(name, (base_of(bases), body))
    return out


def _body_of(name: str) -> str:
    declared = _declared_classes().get(name)
    return declared[1] if declared else ""


def _with_bases(name: str, seen=()):
    """A class's members with its base chain prepended, or None.

    The BASE COMES FIRST. Single non-virtual inheritance lays the base out at
    offset 0 and the derived members after it, which is why every
    GraphicWin-derived class was excluded before: without the base's 0xA14
    bytes every offset in the derived class was wrong by that much.
    """
    if name in seen:                       # a cycle: refuse rather than loop
        return None
    declared = _declared_classes().get(name)
    if declared is None:
        return None
    base, _ = declared
    members = members_of(_body_of(name), seen + (name,))
    if base is None or members is None:
        return None
    if not base:
        return list(members)
    inherited = _with_bases(base, seen + (name,))
    if inherited is None:
        return None
    return inherited + list(members)


@functools.lru_cache(maxsize=1)
def pinned_layouts() -> dict:
    """{class name: [(type, name, array)]} for every class `src/` pins.

    Only classes carrying `static_assert(sizeof(X) == ...)`, since that
    assertion is the tree's own statement that the layout was checked against
    the image - and the size is what the verifier compares against.
    """
    layouts = {}
    for header in sorted(SRC.glob("*.h")):
        text = header.read_text(errors="ignore")
        for name in {n for n, _ in SIZE_ASSERT.findall(text)}:
            members = _with_bases(name)
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


def supplyable(name: str, seen=()) -> bool:
    """Can the emitter actually WRITE this layout into a unit?

    Verification answers "is this layout right"; this answers "can it be
    used here", and they are different questions. A layout holding a member
    BY VALUE needs that member's type defined in the same unit, so the type
    has to be supplyable too - not merely extractable. Allowing extractable
    was enough to emit `Heap heap_;` beside a bare `struct Heap;` and break
    77 units with `error C2079: uses undefined class`.
    """
    if name in WINDOWS_STRUCT:
        return True
    if name in seen or name not in verified_names():
        return False
    for type_, _, _ in pinned_layouts().get(name, ()):
        if "*" in type_:
            continue
        bare = type_.replace("const", "").strip()
        if _needs_definition(bare) and not supplyable(bare, seen + (name,)):
            return False
    return True


def declaration_for(name: str) -> list:
    """The member lines to emit inside a class shell, or []."""
    if name in WINDOWS_STRUCT:
        return [f"    {type_} {member}{array};"
                for type_, member, array in WINDOWS_STRUCT[name]]
    if not supplyable(name):
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
