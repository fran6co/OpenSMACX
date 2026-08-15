#!/usr/bin/env python3
"""The data members `src/` declares for a class, for the scaffolding emitter.

WHY THIS EXISTS. `emit_translation_unit` declares every class as an opaque
shell with no members, and an agent that needs a field therefore reaches it
by offset - or, worse, declares a shadow struct beside the function, which
`writeback.splice` cannot carry into `src/` because it splices the
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
    # EVERY dimension, not one. A single `\[...\]?` left `Sprite base[6][4];`
    # unparsed, and an unparsed statement refuses the WHOLE class - four of
    # them today (FactionArt, PlayerData, Trace, VOX_Matrix). The captured
    # text is re-emitted verbatim and the probe includes the declaring
    # header, so a symbolic bound resolves exactly as it does in src/.
    r"(?P<name>[A-Za-z_]\w*)\s*(?P<array>(?:\[[^\]]*\])*)"
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
    # Four bytes, ABI-frozen like RECT; Palette holds the full 256-entry table
    # by value as `PALETTEENTRY entries_[256]`.
    "PALETTEENTRY": (("BYTE", "peRed", ""), ("BYTE", "peGreen", ""),
                     ("BYTE", "peBlue", ""), ("BYTE", "peFlags", "")),
}


# `typedef void (__cdecl *MenuProc)(int);` at FILE scope. The extractor already
# knows the in-class form (`FUNCTION_POINTER_MEMBER`) and sizes it `void *`;
# not knowing this one refused `Menu` for holding a `MenuProc proc_`, and
# `AlphaMenu` behind it, for a member whose width was never in doubt - the IDB
# has `Menu,0xA14,proc,4` right after `Menu,0x0,graphicWin,2580`.
#
# A `*` NOT PRECEDED BY `::` is required, which is the whole of the care this
# needs: a POINTER-TO-MEMBER typedef - `typedef void (Foo::*F)();` - is four
# bytes only under single inheritance and 8 or 12 or 16 otherwise, and this
# tree declares 113 of them for the original-image seams. Sizing one of those
# as four bytes would move every offset after it.
FILE_SCOPE_FUNCTION_POINTER = re.compile(
    r"^\s*typedef\s+[^;(]*\(\s*(?:__\w+\s+)?\*+\s*(?P<name>\w+)\s*\)\s*\(",
    re.M)


@functools.lru_cache(maxsize=1)
def function_pointer_typedefs() -> frozenset:
    """File-scope typedefs naming a plain function POINTER: four bytes each."""
    found = set()
    for header in sorted(SRC.glob("*.h")):
        for hit in FILE_SCOPE_FUNCTION_POINTER.finditer(
                header.read_text(errors="ignore")):
            found.add(hit.group("name"))
    return frozenset(found)


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
    bases, _ = declared
    if bases is None:
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
    pending, depth, nested = [], 0, ""
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
            if not depth and nested:
                # THE CLOSING LINE MAY CARRY A DECLARATOR, and it is storage:
                #
                #     struct SpotInternal { ... } *spots_;
                #
                # The opening line was yielded and the body skipped, so this
                # `*spots_` reached nobody - Spot lost a member and refused,
                # Buffer refused for holding a Spot, GraphicWin for holding a
                # Buffer, and every class deriving from GraphicWin after it.
                # The declarator is re-emitted against the nested type's name,
                # which is all the caller needs to size a POINTER; a by-value
                # nested member still refuses, because its width is not
                # something this line states.
                tail = re.sub(r"^.*\}", "", stripped).strip()
                if tail.endswith(";") and tail != ";":
                    # `void *` AND NOT THE NESTED TYPE'S NAME. `SpotInternal`
                    # exists only inside `Spot`, so a probe that says
                    # `SpotInternal *spots_;` at file scope is
                    # `C2143: syntax error : missing ';' before '*'`. A
                    # pointer is four bytes whatever it points at, which is
                    # the whole of what a layout needs; a BY-VALUE nested
                    # member is not emitted at all, because its width is not
                    # something this line states.
                    # A POINTER is four bytes whatever it points at, so
                    # `void *` states the width exactly. A BY-VALUE nested
                    # member does NOT have a width this line states, and the
                    # first version yielded "" for it - which `members_of`
                    # skips, silently DROPPING the member. Palette then
                    # computed 0x404 against its asserted 0x454, eighty bytes
                    # short and not refused, which is the one outcome this
                    # whole file exists to prevent. The bare declarator is
                    # yielded instead: it matches no member pattern, so the
                    # class refuses.
                    # `struct SpotInternal { ... } *spots_;` -> `void *spots_;`
                    # (a pointer is four bytes whatever it points at), and
                    # `struct PaletteInternal { ... } internal_[5];` ->
                    # `PaletteInternal internal_[5];`. The BY-VALUE form used
                    # to be yielded as a bare declarator so the class refused;
                    # `nested_types` below knows the width, so re-spelling it
                    # against the type is what lets `members_of` account for
                    # it. Palette and Lock were refused on exactly this.
                    yield (f"void {tail}" if tail.lstrip().startswith("*")
                           else (f"{nested} {tail}" if nested else tail))
                nested = ""
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
            if depth:
                found = re.search(r"\b(?:struct|class)\s+(\w+)", stripped)
                nested = found.group(1) if found else ""
            continue
        pending.append(stripped)
        if stripped.endswith(";"):
            yield " ".join(pending)
            pending = []
    if pending:
        yield " ".join(pending)


# A nested type head is INDENTED, and may share its line with an access label
# (`private: struct Entry {`). `CLASS_HEAD`'s `^(?:class|struct)` anchor is
# right for file scope and blind here.
NESTED_HEAD = re.compile(
    r"^\s*(?:public|private|protected)?\s*:?\s*"
    r"(?:class|struct)\s+(?P<name>\w+)\s*\{", re.M)


def nested_types(body: str, seen=()) -> dict:
    """{name: [(type, name, array)]} for types a class body defines inside it.

    `Palette` defines `PaletteInternal` and holds five of them; `Lock` defines
    `Entry` and `Record` and holds eight Records. Both were refused outright,
    because a nested type is not in `_declared_classes` - that map is built
    from FILE-scope definitions - so a member of one looked like a member of
    an undeclarable type.

    The definition is right there in the header text and its width is exactly
    as computable as any other, so it is read rather than refused. Nothing is
    sized in Python: the members are re-emitted verbatim, the compiler adds
    them up, and the class's own `static_assert` is what checks the total.
    """
    out = {}
    for head in NESTED_HEAD.finditer(body):
        depth, index = 1, head.end()
        while index < len(body) and depth:
            depth += (body[index] == "{") - (body[index] == "}")
            index += 1
        name = head.group("name")
        if name in seen:
            continue
        # IN ORDER, and each may use the ones before it: `Lock` defines
        # `Entry` and then `Record`, which holds two Entries by value.
        members = members_of(body[head.end():index - 1], seen + (name,), out)
        if members is not None:
            out[name] = members
    return out


def members_of(body: str, seen=(), nested=None):
    """[(type, name, array)] for a body of nothing but plain data members.

    None when the body holds anything this cannot account for, because a
    member missed is a member whose absence moves every offset after it.
    """
    found = []
    # The class's OWN nested types, before any member that holds one is read.
    # `nested` arrives non-None only from `nested_types` itself, which is
    # walking these in order; at the top level it starts here.
    if nested is None:
        nested = nested_types(body, seen)
    # Nested aliases first: `typedef int32_t Dib;` has to be known before the
    # member that uses it is read, and a class body may declare it anywhere.
    alias = {}
    for stripped in statements(body):
        # THE ACCESS LABEL SHARES THE LINE, and this file already records that
        # trap for members: `private: AutoSound auto_sound_;`. It applies
        # identically to `private: typedef int32_t Dib;`, which is exactly how
        # buffer.h writes it - so the alias went unlearned, Buffer refused on
        # `Dib dib_[256]`, GraphicWin refused for holding a Buffer, and every
        # class deriving from GraphicWin refused after it.
        stripped = re.sub(r"^(?:public|private|protected)\s*:\s*", "",
                          stripped)
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
        # A NESTED TYPE DEFINITION IS NOT STORAGE. `struct SpotInternal {` is
        # the head of a type, and whatever it declares reaches this loop as a
        # separate statement - `statements()` re-emits the trailing declarator
        # as `SpotInternal *spots_;`. Refusing on the head cost Spot, then
        # Buffer for holding one, then GraphicWin for holding a Buffer, then
        # every class deriving from GraphicWin.
        #
        # `union {` is NOT skipped here and must not be: it IS storage, it is
        # caught by REFUSE above, and swallowing one drops members silently.
        if re.match(r"^(?:struct|class)\s+\w+\s*\{", stripped):
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
        if not stars and bare in function_pointer_typedefs():
            # Spelled `void *`, not the typedef name, for the same reason
            # `FUNCTION_POINTER_MEMBER` is: the emitted unit never declares
            # `MenuProc`, and `<type> <name>;` is the only shape
            # `declaration_for` writes. Four bytes is four bytes.
            found.append(("void *", member.group("name"),
                          member.group("array") or ""))
            continue
        if not stars and nested is not None and bare in nested:
            # A type this very class body defines. Its width is as computable
            # as any other and the compiler is what adds it up.
            found.append((type_, member.group("name"),
                          member.group("array") or ""))
            continue
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


def bases_of(bases: str):
    """The base classes in declaration order, [] for none, None if unusable.

    MULTIPLE INHERITANCE IS MODELLED, not refused. MSVC lays non-virtual bases
    out in declaration order at increasing offsets and the derived members
    after them, which is ordinary concatenation - the same rule the single
    case already used, applied more than once. The image says so directly:
    ??0BaseWin@@QAE@XZ builds its GraphicWin on an unadjusted `this` and then
    reaches its SubInterface at `lea ecx, [esi + 0xa1c]`... at 0xA14, which is
    exactly `sizeof(GraphicWin)` - where a second base goes.

    Refusing it was costing real classes for no layout reason, and the
    refusal's own justification ("neither appears in the classes this
    unlocks") stopped being true the moment the interface hierarchy was read.

    VIRTUAL bases are still refused, and that one is not a modelling gap: MSVC
    places a virtual base where the object's own vbtable says at run time, not
    at a position this file could compute, and AGENTS.md requires those to be
    held as members at the MSVC offset instead.
    """
    if not bases:
        return []
    found = BASE_SPEC.match(bases)
    if not found:
        return None
    out = []
    for part in found.group("bases").split(","):
        words = part.replace("public", " ").replace("protected", " ") \
                    .replace("private", " ").split()
        virtual = "virtual" in words
        words = [word for word in words if word != "virtual"]
        if len(words) != 1:
            return None
        out.append((words[0], virtual))
    return out


def base_names_of(bases: str):
    #: `bases_of` without the virtual flag, or None when it refuses.
    found = bases_of(bases)
    return None if found is None else [name for name, _ in found]


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
            out.setdefault(name, (bases_of(bases), body))
    return out


def _body_of(name: str) -> str:
    declared = _declared_classes().get(name)
    return declared[1] if declared else ""


def _layout_parts(name: str, seen=()):
    """(members up to and including `name`'s own, {virtual base: its members}).

    The split exists because a virtual base does NOT belong to the class that
    inherits it - it belongs to the most derived object, and it is laid out
    once, at the end of that object, however deep the edge is. Keeping it in a
    dict rather than a list is the "once": two paths to the same virtual base
    contribute one subobject, which is the whole point of the keyword.

    Returning the parts separately is also what lets a caller say WHERE a
    class's own members sit - see `declared_span`.
    """
    if name in seen:                       # a cycle: refuse rather than loop
        return None
    declared = _declared_classes().get(name)
    if declared is None:
        return None
    bases, _ = declared
    members = members_of(_body_of(name), seen + (name,))
    if bases is None or members is None:
        return None
    if not bases:
        return list(members), {}
    # NON-VIRTUAL bases come first, at increasing offsets; VIRTUAL ones come
    # LAST, behind a vbtable pointer the compiler puts at offset 0. That is
    # MSVC's rule and it is exactly what the two headers declaring it measure
    # to: Console is 4 + (0x23D94 - 4) + 0xA14 == 0x247A8, MapWin is
    # 4 + 4 + (0x21A6C - 8) + 0xA14 == 0x22480, and both static_asserts hold
    # against cl 12.00.8168. Refusing virtual bases instead cost those two
    # classes their extracted layout the moment they were declared honestly.
    #
    # THE EDGE IS HOISTED THE WHOLE WAY, not one level. `Console : MapWin` and
    # `MapWin : virtual GraphicWin` was flattened as MapWin-entire - GraphicWin
    # included - followed by Console's own members, which put GraphicWin's
    # 0xA14 bytes in the MIDDLE of Console and every one of Console's 36
    # name-encoded members 0xA14 too high. sizeof was unmoved by it, so the
    # size assertion could not see it; cl 12.00.8168 answers
    # `&((Console *)0)->field_21A6C_` with 0x21A6C against the accumulator's
    # 0x22480, and console.h reads the same thing off the image (the ctor
    # builds the virtual base at this+0x23D94, 0x0050F4A0).
    flattened, trailing = [], {}
    for base, virtual in bases:
        part = _layout_parts(base, seen + (name,))
        if part is None:
            return None
        inherited, inherited_virtual = part
        trailing.update(inherited_virtual)
        if virtual:
            trailing.setdefault(base, inherited)
        else:
            flattened.extend(inherited)
    # ONE vbtable pointer, at the offset the class that introduced the virtual
    # base put it. A derived class inheriting that class non-virtually shares
    # it - `flattened` already opens with it - and inserting a second would
    # move every member of Console by four bytes.
    if trailing and ("void *", "__vbptr", "") not in flattened:
        flattened.insert(0, ("void *", "__vbptr", ""))
    flattened += list(members)
    return flattened, trailing


def declared_span(name: str):
    """(start, stop) into `_with_bases(name)` for `name`'s OWN members, or None.

    THE OWN MEMBERS ARE NOT THE TAIL, which is the trap this exists to close.
    A virtual base is laid out behind them, so the tail of a MapWin layout is
    GraphicWin's - and the name-encoded control in test_classify_casts read
    that tail as "what MapWin declares", compared GraphicWin's own offsets
    (`field_134_` at 0x134) against their positions inside a MapWin (0x21BA0)
    and reported 49 disagreements that were its own assumption rather than a
    layout defect.
    """
    parts = _layout_parts(name)
    own = members_of(_body_of(name))
    if parts is None or own is None:
        return None
    stop = len(parts[0])
    return stop - len(own), stop


def _with_bases(name: str, seen=()):
    """A class's members with its base chain prepended, or None.

    THE BASES COME FIRST, IN DECLARATION ORDER. Non-virtual inheritance lays
    each base out in turn from offset 0 and the derived members after them,
    which is why every GraphicWin-derived class was excluded before: without
    the base's 0xA14 bytes every offset in the derived class was wrong by that
    much. More than one base is the same rule applied more than once - see
    `bases_of`. A VIRTUAL base is the exception and closes the object instead;
    `_layout_parts` is where that is decided.
    """
    parts = _layout_parts(name, seen)
    if parts is None:
        return None
    flattened, trailing = parts
    for inherited in trailing.values():
        flattened = flattened + inherited

    # A DERIVED CLASS THAT RE-DECLARES A BASE MEMBER'S NAME IS FINE HERE.
    # It is legal C++ - name hiding - and the object really carries both, so
    # the LAYOUT this function answers for was never in doubt.
    #
    # It used to refuse, and the reason was real at the time: the emitter
    # wrote one FLAT class, where `Buffer buffer_;` inherited from GraphicWin
    # beside Scroll's own `Buffer *buffer_;` is
    # `C2040: differs in levels of indirection`, and 70 byte-exact bodies went
    # NO_COMPILE in a single census the moment these were emitted. The emitter
    # now writes a real base clause (`declare` + `layout_bases`), and under
    # inheritance the two members are in different scopes and both compile -
    # exactly as they did in the original.
    #
    # The flat path still exists, for a class whose bases cannot all be
    # supplied, so the check moved rather than vanished: `supplyable` refuses a
    # class that would be FLATTENED with a duplicate name. Twelve classes
    # shadow a base member - BasePop, BaseWin, Credits, DesignWin, NetMsg,
    # PopMenu, Popup, PullDown, ReportWin, Scroll, SetupWin, WorldWin - and
    # they were all refused outright for a scaffolding limitation that is gone.
    return flattened


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
    # A class the emitter will write FLAT cannot carry two members of the same
    # name - `Buffer buffer_` inherited from GraphicWin beside Scroll's own
    # `Buffer *buffer_` is `C2040: differs in levels of indirection`. Under a
    # real base clause they are in different scopes and both compile, which is
    # what the original did, so the refusal applies only to the flat case.
    declared = _declared_classes().get(name)
    bases = [b for b, _ in (declared[0] or ())] if declared and declared[0] is not None else None
    if not bases or not all(supplyable(base, seen + (name,)) for base in bases):
        spelled = [member for _, member, _ in pinned_layouts().get(name, ())]
        if len(spelled) != len(set(spelled)):
            return False
    # A type the class DEFINES INSIDE ITSELF is emitted with it, by
    # `nested_definition_lines`, so it needs no separate verdict - and asking
    # for one refuses the class, because a nested type is never in
    # `verified_names()`. Palette and Lock were verified and unsupplyable at
    # once for exactly that reason.
    own_nested = set(nested_types(_body_of(name)))
    for base in base_chain(name):
        own_nested |= set(nested_types(_body_of(base)))
    for type_, _, _ in pinned_layouts().get(name, ()):
        if "*" in type_:
            continue
        bare = type_.replace("const", "").strip()
        if bare in own_nested:
            continue
        if _needs_definition(bare) and not supplyable(bare, seen + (name,)):
            return False
    return True


def nested_definition_lines(name: str) -> list:
    """`struct Inner { ... };` lines for the nested types `name` holds.

    Emitted INSIDE the class shell, ahead of the members that use them, which
    is where the header puts them. Without this the shell says
    `PaletteInternal internal_[5];` against a type nothing declares.
    """
    lines, table = [], nested_types(_body_of(name))
    if not table:
        return lines
    for inner, members in table.items():
        lines.append(f"    struct {inner} {{")
        for type_, member, array in members:
            lines.append(f"        {type_} {member}{array};")
        lines.append("    };")
    return lines


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
    return nested_definition_lines(name) + \
        [f"    {type_} {member}{array};" for type_, member, array in members]


def unverified_declaration_for(name: str) -> list:
    """The extracted members WITHOUT the verification gate, for the verifier.

    NAMES ARE MADE UNIQUE, because the probe FLATTENS a base and its derived
    class into one struct and C++ allows the same name at both levels. A
    derived class re-declaring a base member is legal name hiding and the
    object really does carry both, so the layout is right - but
    `struct Probe { Heap heap_; ... Heap heap_; };` is
    `C2086: 'heap_' : redefinition` and the class was reported as having no
    verdict. Three classes were in that state the moment enough of the tree
    became readable for it to matter: BasePop::heap_ over Win::heap_,
    PullDown::menu_, and Scroll::buffer_ - which additionally differs in
    indirection from GraphicWin's, `Buffer *` against `Buffer`.

    Only the SPELLING changes here, never the order or the width, so every
    offset and the total are exactly what the real class has.
    """
    members = pinned_layouts().get(name)
    # The nested definitions come first here too: the probe is a flat struct
    # and `PaletteInternal internal_[5];` needs the type in scope. A nested
    # type defined by a BASE arrives through the base's own lines below,
    # because the probe flattens the chain.
    lines, seen = list(nested_definition_lines(name)), {}
    for base in base_chain(name):
        lines = list(nested_definition_lines(base)) + lines
    for type_, member, array in members or []:
        seen[member] = seen.get(member, 0) + 1
        spelled = member if seen[member] == 1 else f"{member}_shadow{seen[member]}"
        lines.append(f"    {type_} {spelled}{array};")
    return lines


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
    out = set(layout_bases(name))
    for type_, _, _ in pinned_layouts().get(name, ()):
        bare = type_.replace("*", " ").replace("const", " ").split()
        if bare and bare[-1] not in SCALAR:
            out.add(bare[-1])
    # A NESTED definition names types too, and they have to be declared before
    # the class that carries it: `Spot::SpotInternal` holds a `RECT`, and
    # emitting the nested struct without it is
    # `C2146: missing ';' before identifier 'rect'`. Six byte-exact rows broke
    # on exactly that the first time nested types were emitted.
    table = nested_types(_body_of(name))
    for inner, members in table.items():
        out.discard(inner)                 # defined inline, not referenced
        for type_, _, _ in members:
            bare = type_.replace("*", " ").replace("const", " ").split()
            if bare and bare[-1] not in SCALAR and bare[-1] not in table:
                out.add(bare[-1])
    return out


def layout_bases(name: str) -> list:
    """The bases the emitter should DECLARE for `name`, or [].

    Empty means "emit this class flat", which is the right answer both for a
    root class and for one whose bases cannot be supplied - a flat class with
    every inherited member spelled out has the same layout, and that is what
    the emitter did for every class until the interface hierarchy needed
    telling apart from a member.

    Gated on `supplyable` for the WHOLE chain: a base emitted as an opaque
    shell would contribute zero bytes and silently move every derived offset,
    which is the one failure this file exists to prevent.
    """
    if not supplyable(name):
        return []
    declared = _declared_classes().get(name)
    if declared is None:
        return []
    bases, _ = declared
    if not bases:
        return []
    # A VIRTUAL base is not spelled as a base clause by the emitter: MSVC
    # places it after the derived members, `_with_bases` models exactly that,
    # and the flat list it produces has the right layout. Emitting
    # `: public X` for one would put it at the front and move everything.
    if any(virtual for _, virtual in bases):
        return []
    names = [base for base, _ in bases]
    return names if all(supplyable(base) for base in names) else []


def base_chain(name: str, seen=()) -> list:
    """Every class `name` derives from, transitively, in no particular order."""
    if name in seen:
        return []
    declared = _declared_classes().get(name)
    bases = declared[0] if declared else None
    if not bases:
        return []
    out = []
    for base, _ in bases:
        out.append(base)
        out.extend(base_chain(base, seen + (name,)))
    return out


def own_declaration_for(name: str) -> list:
    """The member lines for `name` ALONE, with no base members prepended.

    The counterpart to `declaration_for` for a class the emitter declares with
    a real base clause: the base contributes its own members, so repeating
    them here would double every one of them. Callers must use this and
    `layout_bases` together - own members with no base clause is a class
    missing sizeof(base) bytes at the front, and it still compiles.
    """
    if name in WINDOWS_STRUCT:
        return declaration_for(name)
    if not supplyable(name):
        return []
    members = members_of(_body_of(name))
    if not members:
        return []
    return nested_definition_lines(name) + \
        [f"    {type_} {member}{array};" for type_, member, array in members]


# `int init(RECT *, LPSTR, int);` - a method DECLARATION inside a class body.
# Not a function-pointer member (`int (*hook_)(void);`), which is storage and
# is read by `members_of` instead.
METHOD = re.compile(
    r"^(?P<head>[\w\s*&:<>,]+?)\s*"
    r"(?P<name>~?\w+|operator\s*\S+)\s*"
    r"\((?P<params>[^)]*)\)\s*(?P<trailer>const)?\s*[;{]")

# Tokens a method declaration may carry that name no type.
METHOD_NOISE = frozenset({
    "public", "private", "protected", "static", "inline", "explicit",
    "virtual", "const", "friend", "DLLEXPORT", "__cdecl", "__thiscall",
    "__stdcall", "__fastcall", "struct", "class", "enum", "unsigned",
    "signed", "long", "short", "void",
})


def _method_types(text: str) -> set:
    """Every identifier in a method declaration that has to name a type."""
    out = set()
    for token in re.findall(r"[A-Za-z_]\w*", text):
        if token in METHOD_NOISE or token in SCALAR:
            continue
        out.add(token)
    return out


@functools.lru_cache(maxsize=None)
def methods_of(name: str) -> list:
    """[(declaration, method name, types it names)] for `src/`'s methods.

    THE SHELL USED TO DECLARE ONLY THE METHODS THE CATALOGUE SAW CALLED, and
    that is a different set from the methods the class has. A body calling
    anything else is `C2039: is not a member`, which reads as a bad body and
    is a missing declaration - 275 already-written bodies were in that state,
    209 of them on two names, `Sprite::close` and `CaviarData::close`, both
    of which `src/sprite.h` and `src/caviar.h` have declared all along.

    A declaration is never inlined and occupies no bytes, so importing one
    cannot move the comparison; only its ABSENCE can, by keeping the unit from
    compiling at all.

    Three things are deliberately dropped rather than translated:

      * `virtual`, because the shell may carry a PROVED LAYOUT and a vtable
        pointer would move every offset in it by four. The declaration still
        lets the call compile; spelling the dispatch INDIRECTLY is the body's
        job and the brief has a lever for it.
      * constructors and destructors, because declaring one suppresses the
        implicit default and a body that says `Buffer b;` then fails to
        compile for a reason the class did not have before. The emitter
        already writes these from the catalogue where a body calls them.
      * anything naming a type this module cannot place, since a method
        mentioning an unknown type is `C2061` - strictly worse than the
        `C2039` it was meant to cure.
    """
    body = _body_of(name)
    if not body:
        return []
    out, seen = [], set()
    for statement in statements(body):
        stripped = re.sub(r"^(?:public|private|protected)\s*:\s*", "",
                          statement).strip()
        if "(" not in stripped or FUNCTION_POINTER_MEMBER.match(stripped):
            continue
        if re.search(r"\b(typedef|friend|template|operator|using)\b", stripped):
            continue
        match = METHOD.match(stripped)
        if not match or match.group("name").startswith("~"):
            continue
        method = match.group("name")
        if method == name:                      # a constructor
            continue
        head = match.group("head")
        # `= 0` never reaches here (METHOD stops at the `)`), but `virtual`
        # and the export macro do, and neither belongs in the shell.
        head = re.sub(r"\b(virtual|DLLEXPORT)\b", " ", head)
        head = re.sub(r"\s+", " ", head).strip()
        if not head:                            # no return type: a ctor form
            continue
        params = re.sub(r"\s*=\s*[^,]+", "", match.group("params")).strip()
        types = _method_types(head) | _method_types(params)
        # A FUNCTION-POINTER TYPEDEF cannot be forward-declared. Both consumers
        # spell an unknown type as `struct X;`, and doing that to a typedef is
        # `C2371: redefinition; different basic types` - which takes the unit
        # down instead of the one method that named it.
        if any(t not in _declared_classes() and t not in WINDOWS_TYPEDEF
               and t not in WINDOWS_STRUCT
               or t in function_pointer_typedefs() for t in types):
            continue
        trailer = " const" if match.group("trailer") else ""
        text = f"    {head} {method}({params}){trailer};"
        if text not in seen:
            seen.add(text)
            out.append((text, method, types))
    return out


@functools.lru_cache(maxsize=None)
def method_types(name: str) -> set:
    """Types `name`'s imported methods mention, to forward-declare ahead."""
    out = set()
    for _, _, types in methods_of(name):
        out |= types
    # FROZEN: the result is cached and shared, and a caller that mutated it
    # would corrupt every later lookup.
    return frozenset(out - SCALAR)


if __name__ == "__main__":
    layouts = pinned_layouts()
    print(f"{len(layouts)} classes with a pinned layout src/ can supply")
    for name in sorted(layouts):
        print(f"   {name:22} {len(layouts[name]):3} members")
